/*
 *
 * alarm.c
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <pthread.h>
#include <iconv.h> 

#include "ex_alarm.h"
#include "alarm.h"
#include "config.h"
#include "utils.h"
#include "db.h"
#include "log.h"
#include "email_interface.h"
#include "cengine.h"

static char * alarm_level_str [ ] = {
    "Serious",
    "Common",
    "Caution"
};

static char * alarm_relation_str [ ] = {
    "More than",
    "Less than",
    "Equal to",
    "More than && Equal to",
    "Less than && Equal to",
    "Not equal to",
    "Unknown"
};

static char * alarm_level_str_ch [] = {
	"严重",
	"一般",
	"警告"
};
static char * alarm_relation_str_ch [ ] = {
    "大于",
    "小于",
    "等于",
    "大于等于",
    "小于等于",
    "不等于",
    "未知"
};


////#define ALARM_MSG_FMT "%s node: %s item: %s level: %s condition: the value of alarm item %s is %s %f in %d seocnds"
#define ALARM_MSG_FMT "%s: %s %f for %d seconds"
#define ALARM_MSG_FMT_CH "%s: 连续 %d 秒%s %f"
#define SEND_MSG_INTERVAL 1
#define ALARM_EMAIL_SUBJECT "Alarm message from the cluster monitoring alarm software"
//#define DB_INSERT_SQL_PREFIX "INSERT INTO alarm_info (datetime,info,flag) VALUES"
#define DB_INSERT_SQL_PREFIX "INSERT INTO AlarmInfo (nodeName,alarmType,alarmTime,alarmInfoCh,alarmInfoEn,alarmLevel, flag) VALUES"
//#define DB_INSERT_SQL_FMT "( \'%s\', \'%s\', %d )"
#define DB_INSERT_SQL_FMT "( \'%s\', \'%s\', \'%s\', \'%s\', \'%s\', %d, %d)"
#define DB_RETRY_TIMES 3

#define SMS_RESULT_LEN 1024*10
#define SEND_SMS_TIMEOUT 60
#define MAX_SMS_CMD_LEN 1024*4
//#define GNOKII_CMD "../lib/libsms.so"
#define GNOKII_CMD "../tools/lt-gnokii"
#define GNOKII_CONFIG "--config ../config/sms.cf"
#define SMS_RE_SEND_TIMES 5
#define SMS_RE_SEND_INTERVAL 3
#define EMAIL_RE_SEND_TIMES 3
#define EMAIL_RE_SEND_INTERVAL 5
//#define SMS_END_INFO "REPORT:OK\n"
#define SMS_END_INFO "Send succeeded!\n"

#ifdef CS_LOG_PATH
#undef CS_LOG_PATH
#endif
#define CS_LOG_PATH "/var/log/alarmd.send.log"


extern ex_alarm_config_t g_ex_alarm_config;

alarm_msg_buf_t g_alarm_msg_buf;
pthread_mutex_t mutex_g_alarm_msg_buf;



int send_alarm_sms ( char * num, char * alarm_msg, unsigned int retrytimmes, \
        unsigned int timeout );
int send_alarm_message ( char * num, char * msg, unsigned int retrytimes, \
        unsigned int timeout );
int send_alarm_telephone ( char * num, char * alarm_msg, \
        unsigned int retrytimes, unsigned int timeout );
int send_alarm_tel ( char * num, char * alarm_msg, \
	unsigned int retrytimes, unsigned timeout );
static int send_sms ( sms_config_t * sms_config, char * alarm_msg, unsigned int retrytimes, \
        unsigned int timeout );
static int send_tel ( tel_config_t * tel_config, char * alarm_msg, unsigned int retrytimes, \
        unsigned int timeout );


/*
 * Function to send alarm message to buffer
 * @arg:  alarm message
 * RET    NULL
 */


void * send_alarm_msg_to_buffer ( void * arg ) {

    pthread_detach ( pthread_self ( ) );

    alarm_th_stob_t * alarm_arg = ( alarm_th_stob_t * ) arg;
    pthread_mutex_t * alarm_mutex = & ( alarm_arg -> alarm_node -> \
		alarm_thread_mutex [ alarm_arg -> alarm_level ] );

    char alarm_msg_buffer [ MAX_ALARM_MSG_BUFFER_LEN_MYSQL ];
    char alarm_msg_buffer_ch [ MAX_ALARM_MSG_BUFFER_LEN_MYSQL ];
    memset(alarm_msg_buffer, 0, sizeof(alarm_msg_buffer));
    memset(alarm_msg_buffer_ch, 0, sizeof(alarm_msg_buffer_ch));

    /* produce alarm message */
    char * script_name = alarm_arg -> alarm_item -> alarm_script;
    if ( * script_name != 0 ) {
        char cmd  [ MAX_ALARM_SCRIPT_NAME_LEN + MAX_ALARM_NODE_NAME_LEN + MAX_ALARM_ITEM_NAME_LEN + 256 ];
        sprintf ( cmd, "%s%s %s '%s' %f %d %s %s %f", \
                SCRIPT_PATH, script_name, \
		alarm_level_str [ alarm_arg -> alarm_level ], \
		alarm_relation_str [ alarm_arg -> alarm_item -> \
		alarm_condition [ alarm_arg -> alarm_level ].alarm_relation ], \
		alarm_arg -> alarm_item -> alarm_condition [ alarm_arg -> alarm_level ].value, \
		alarm_arg -> alarm_item -> alarm_condition [ alarm_arg -> alarm_level ].secs, 
                alarm_arg -> alarm_node -> alarm_node_name, 
                alarm_arg -> alarm_item -> alarm_item_name, 
                alarm_arg -> alarm_value 
               );
        int excode;
        int ret = c_execute ( cmd, alarm_msg_buffer, MAX_ALARM_MSG_BUFFER_LEN_MYSQL, & excode, 30 );
        // added by fuchencong
        if (alarm_msg_buffer[strlen(alarm_msg_buffer) - 1] == '\n'){
            alarm_msg_buffer[strlen(alarm_msg_buffer) - 1] = 0;
        }

        if ( ret < 0 ) {
            pthread_exit ( NULL );
        }
    }
//    else {
        char dtbf [ MAX_DATE_TIME_LEN ];
        u_time_stamp_r ( dtbf );
/*        sprintf ( alarm_msg_buffer_detail, ALARM_MSG_FMT, \
            dtbf, \
            alarm_arg -> alarm_node -> alarm_node_name, \
            alarm_arg -> alarm_item -> alarm_item_name, \
            alarm_level_str [ alarm_arg -> alarm_level ], \
            alarm_arg -> alarm_item -> alarm_item_name, \
            alarm_relation_str [ alarm_arg -> alarm_item -> \
            alarm_condition [ alarm_arg -> alarm_level ].alarm_relation ], \
            alarm_arg -> alarm_item -> alarm_condition [ alarm_arg -> alarm_level ].value, \
            alarm_arg -> alarm_item -> alarm_condition [ alarm_arg -> alarm_level ].secs 
        );*/
	  sprintf ( alarm_msg_buffer, ALARM_MSG_FMT, \
            alarm_level_str [ alarm_arg -> alarm_level ], \
            alarm_relation_str [ alarm_arg -> alarm_item -> \
            alarm_condition [ alarm_arg -> alarm_level ].alarm_relation ], \
            alarm_arg -> alarm_item -> alarm_condition [ alarm_arg -> alarm_level ].value, \
            alarm_arg -> alarm_item -> alarm_condition [ alarm_arg -> alarm_level ].secs 
	);
	/** chinese format**/
	sprintf (alarm_msg_buffer_ch, ALARM_MSG_FMT_CH, \
	alarm_level_str_ch [alarm_arg -> alarm_level ],\
	alarm_arg -> alarm_item -> alarm_condition [ alarm_arg -> alarm_level ].secs,\
	alarm_relation_str_ch [alarm_arg -> alarm_item -> \
	alarm_condition [alarm_arg -> alarm_level ].alarm_relation ],\
	alarm_arg -> alarm_item -> alarm_condition [ alarm_arg -> alarm_level ].value

	);

//    }
 
    /* length of alarm message */
    const int len = strlen ( alarm_msg_buffer );
    const int len_ch = strlen ( alarm_msg_buffer_ch );

    /* get alarm freq */
    alarm_freq_t * alarm_freq = & ( g_ex_alarm_config.alarm_freq );

    /* alarm times */
    int count = alarm_freq -> times;
    char alarm_node [ MAX_ALARM_MSG_BUFFER_LEN_MYSQL ];
    char alarm_item [ MAX_ALARM_MSG_BUFFER_LEN_MYSQL ];
    int alarm_level;
    strcpy(alarm_node, alarm_arg -> alarm_node -> alarm_node_name);
    strcpy(alarm_item, alarm_arg -> alarm_item -> alarm_item_name);
    alarm_level = alarm_arg -> alarm_level;
    while ( count -- > 0 ) {

        char * str = ( char * ) malloc ( len + 1 );
        char * str_ch = ( char * ) malloc ( len_ch + 1 );
        if ( str == NULL || str_ch == NULL) {
            if ( arg != NULL ) {
                free ( arg );
	    }
            pthread_exit ( NULL );
        }
	strcpy ( str, alarm_msg_buffer );
	strcpy ( str_ch, alarm_msg_buffer_ch );
        /* tail of queue */
	alarm_msg_buf_t * p = & g_alarm_msg_buf;
	pthread_mutex_lock ( & mutex_g_alarm_msg_buf );
	while ( p -> next ) {
	    p = p -> next;
	}
	p -> next = ( alarm_msg_buf_t * ) malloc ( sizeof ( alarm_msg_buf_t ) );
	bzero ( p -> next, sizeof ( alarm_msg_buf_t ) );
	p -> next -> msg = str;
	p -> next -> msg_ch = str_ch;
	strcpy (p -> next -> nodeName, alarm_node);
	strcpy (p -> next -> alarmType, alarm_item);
	p -> next -> alarmLevel = alarm_level;

        
        /* set message len */
        int * msg_len = ( int * ) & ( g_alarm_msg_buf.msg );
        * msg_len += len + 1 + 1;
        
        int * msg_len_ch = ( int * ) & ( g_alarm_msg_buf.msg_ch );
        * msg_len_ch += len_ch + 1 + 1;

	pthread_mutex_unlock ( & mutex_g_alarm_msg_buf );

        /* mimutes to seconds */
        int sleep_secs = alarm_freq -> interval * 60;

        while ( count > 0 && sleep_secs -- > 0 ) {
            /* if cancel flag is set */
	    if ( alarm_arg -> alarm_node -> thread_cancel [ alarm_arg -> alarm_level ] == 1 ) {
		pthread_mutex_lock ( alarm_mutex );
		alarm_arg -> alarm_node -> thread_cancel [ alarm_arg -> alarm_level ] = 0;
                alarm_arg -> alarm_node -> alarm_flag \
                    [ alarm_arg -> alarm_level ] = HAVE_NOT_ALARM;
		pthread_mutex_unlock ( alarm_mutex );
                goto END_TIME;
	    }
            sleep ( 1 );
        }
    }

    /* set alarm flag */
    pthread_mutex_lock ( alarm_mutex );
    alarm_arg -> alarm_node -> alarm_flag [ alarm_arg -> alarm_level ] = ALARMED;
    pthread_mutex_unlock ( alarm_mutex );

    /* time_t end_time = 0;
    while ( 1 ) {
        pthread_mutex_lock ( alarm_mutex );
        end_time = alarm_arg -> alarm_node -> end_time [ alarm_arg -> alarm_level ];
        pthread_mutex_unlock ( alarm_mutex );
        if ( end_time != 0 ) {
            break;
        }
        sleep ( 1 );
    } */

    char * end_str = NULL;
    char * end_str_ch = NULL;
END_TIME:
    {
        end_str = ( char * ) malloc ( len + 1 );
        if ( end_str == NULL ) {
            if ( arg != NULL ) {
                free ( arg );
            }
            pthread_exit ( NULL );
        }
	strcpy ( end_str, alarm_msg_buffer );
	end_str_ch = ( char * ) malloc ( len_ch + 1 );
        if ( end_str_ch == NULL ) {
            if ( arg != NULL ) {
                free ( arg );
            }
            pthread_exit ( NULL );
        }
	strcpy ( end_str_ch, alarm_msg_buffer_ch );

        /* tail of queue */
	alarm_msg_buf_t * p = & g_alarm_msg_buf;
	pthread_mutex_lock ( & mutex_g_alarm_msg_buf );
	while ( p -> next ) {
	    p = p -> next;
	}
	p -> next = ( alarm_msg_buf_t * ) malloc ( sizeof ( alarm_msg_buf_t ) );
	bzero ( p -> next, sizeof ( alarm_msg_buf_t ) );
        p -> next -> end_flag = IS_END_MSG;
	p -> next -> msg = end_str;
	p -> next -> msg_ch = end_str_ch;
	strcpy (p -> next -> nodeName, alarm_node);
	strcpy (p -> next -> alarmType, alarm_item);
	p -> next -> alarmLevel = alarm_level;

        /* set message len */
        int * msg_len = ( int * ) & ( g_alarm_msg_buf.msg );
        * msg_len += len + 1 + 1;

        int * msg_len_ch = ( int * ) & ( g_alarm_msg_buf.msg_ch );
        * msg_len_ch += len_ch + 1 + 1;
	pthread_mutex_unlock ( & mutex_g_alarm_msg_buf );
    }

    if ( arg != NULL ) {
        free ( arg );
    }
    pthread_exit ( NULL );

}


/*
 * Function to send alarm message from buffer
 * @arg:  alarm message
 * RET    NULL
 */

void * send_alarm_msg_from_buffer ( void * arg ) {

    cs_log_start;

    char error_msg [ ALARM_ERRMSG_LEN * 16 ];
    char datetime [ MAX_DATE_TIME_LEN ];
    email_config_t email_config;
    alarm_msg_buf_t * p = NULL, * q = NULL;
    char  alarm_subject [ MAX_ALAEM_SUBJECT_LENGTH ];
 
    MYSQL * con = NULL;
    /* get db config */
    db_config_t * db_config = & ( g_ex_alarm_config.db_config );

    alarm_method_switch_t method_mail = g_ex_alarm_config.alarm_method.email;
    alarm_method_switch_t method_sms = g_ex_alarm_config.alarm_method.sms;
    alarm_method_switch_t method_tel = g_ex_alarm_config.alarm_method.tel;

    /* get email config */
    memcpy ( & email_config, & ( g_ex_alarm_config.email_config ), sizeof ( email_config_t ) );

    int cret, db_conn_retry_times = DB_RETRY_TIMES;
RE_DB:
    /* get db connection */
    cret = mysql_get_connection ( db_config, & con );
    if ( cret < 0 ) {
        cs_log ( LOG_FATAL, "connect to mysql server for insert data error, ret = %d", cret );
        if ( db_conn_retry_times -- < 0 ) {
            exit ( 51 );
        }
        else {
            goto RE_DB;
        }
    }
    db_conn_retry_times = DB_RETRY_TIMES;
    mysql_set_character_set(con, "utf8");/////////////////

    while ( 1 ) {
        /* length of alarm message */
        pthread_mutex_lock ( & mutex_g_alarm_msg_buf );
	bzero(alarm_subject, MAX_ALAEM_SUBJECT_LENGTH);
	strcpy (alarm_subject, ALARM_SUBJECT);
        int * msg_len = ( int * ) & ( g_alarm_msg_buf.msg );
        if ( * msg_len == 0) {
            pthread_mutex_unlock ( & mutex_g_alarm_msg_buf );
            sleep ( SEND_MSG_INTERVAL );
            continue;
        }
        char * alarm_msg = ( char * ) malloc ( 2*(* msg_len + sizeof(g_alarm_msg_buf.nodeName)+sizeof(g_alarm_msg_buf.alarmType)));
//        char * alarm_msg = ( char * ) malloc ( * msg_len );
        if ( alarm_msg != NULL) {
            * alarm_msg = 0;
        }
        p = & g_alarm_msg_buf;
        while ( p -> next ) {
            /* get alarm message */
            q = p -> next;
            u_time_stamp_r ( datetime );
            char * sql_buffer = ( char * ) malloc ( strlen ( DB_INSERT_SQL_PREFIX ) + \
                    MAX_DATE_TIME_LEN + \
                    strlen ( q -> nodeName ) + \
                    strlen ( q -> alarmType ) + \
                    strlen ( q -> msg ) + \
                    strlen ( q -> msg_ch) +\
                    64 );
            if ( sql_buffer == NULL ) {
                exit ( 52 );
            }

            /* produce sql statement */
//            sprintf ( sql_buffer, "%s "DB_INSERT_SQL_FMT, DB_INSERT_SQL_PREFIX, datetime, q -> msg, q -> end_flag );
            sprintf ( sql_buffer, "%s "DB_INSERT_SQL_FMT, \
                    DB_INSERT_SQL_PREFIX, q->nodeName, q->alarmType,  datetime, q->msg_ch, q->msg, (q->alarmLevel)+1, q->end_flag);
		printf("sql_buffer = %s\n", sql_buffer);
            /* insert data to db */

            int ret = mysql_query ( con, sql_buffer );
            if ( ret != 0 ) {
                cs_log ( LOG_ERROR, "insert data into db error" );
            }
            free ( sql_buffer );
            if ( q -> end_flag == 0 ) {
                strcat ( alarm_msg, q -> nodeName );
                strcat ( alarm_msg, " " );
                strcat ( alarm_msg, q -> alarmType );
                strcat ( alarm_msg, " " );
                strcat ( alarm_msg, q -> msg );
                strcat ( alarm_msg, "\n" );
		if (strstr(alarm_subject, q->nodeName) == NULL && strlen(alarm_subject) < MAX_ALAEM_SUBJECT_LENGTH) {
			strcat ( alarm_subject, q->nodeName);
			strcat ( alarm_subject, ",");
		}
            }
            p -> next = q -> next;
            free ( q -> msg );
            free ( q -> msg_ch );
            free ( q );
            if ( ret != 0 ) {
                pthread_mutex_unlock ( & mutex_g_alarm_msg_buf );
                free ( alarm_msg );
                mysql_close ( con );
                goto RE_DB;
            }
        }
        * msg_len = 0;
        pthread_mutex_unlock ( & mutex_g_alarm_msg_buf );
        if ( alarm_msg [ 0 ] == '\0') {
            free ( alarm_msg );
            sleep ( SEND_MSG_INTERVAL );
            continue;
        }

        if ( method_mail == METHOD_ON ) {
            /* send email */
            int re_mail_times = 0, ret;
            while ( re_mail_times ++ < EMAIL_RE_SEND_TIMES ) {
//	        ret = send_email ( & email_config, ALARM_EMAIL_SUBJECT, alarm_msg, 
	        ret = send_email ( & email_config, alarm_subject, alarm_msg, \
			    error_msg, ALARM_ERRMSG_LEN * 16 );
                if ( ret == 0 ) {
                    break;
                }
                else if ( ret == -4 ) {
		    cs_log ( LOG_ERROR, "send mail error, %d %s", re_mail_times, error_msg );
	        }
	        else if ( ret != 0 ) {
		    cs_log ( LOG_ERROR, "send mail error %d", re_mail_times );
	        }
                sleep ( EMAIL_RE_SEND_INTERVAL );
		cs_log ( LOG_INFO, "re send email" );
            }
        }
        if ( method_sms == METHOD_ON ) {
            /* send sms */
            int sret = send_sms ( & ( g_ex_alarm_config.sms_config ), alarm_msg, \
                    SMS_RE_SEND_TIMES, SEND_SMS_TIMEOUT );
            if ( sret < 0 ) {
		cs_log ( LOG_ERROR, "send sms error" );
            }
	    sleep(10);
        }
        if ( method_tel == METHOD_ON ) {
            /* send tel */
            int sret = send_tel ( & ( g_ex_alarm_config.tel_config ), alarm_msg, \
                    SMS_RE_SEND_TIMES, SEND_SMS_TIMEOUT );
            if ( sret < 0 ) {
		cs_log ( LOG_ERROR, "tel error" );
            }
        }
        free ( alarm_msg );
        sleep ( SEND_MSG_INTERVAL );
    }

    cs_log_end;
    mysql_close ( con );
    pthread_exit ( NULL );

}


/*
 * Function to send sms
 * @sms_config: sms config 
 * @alarm_msg:   alarm message
 * @retrytimes:  retry times
 * @timeout:     timeout value
 * RET 0 or 1 on success, otherwise return a negative number
 */

static int send_sms ( sms_config_t * sms_config, char * alarm_msg, \
        unsigned int retrytimes, unsigned int timeout ) {
    if ( sms_config == NULL || alarm_msg == NULL || alarm_msg [ 0 ] == '\0' ) {
        return -1;
    }
    int ret;
    /* sms config */
    sms_config_t local_sms_config;
    memcpy ( & local_sms_config, sms_config, sizeof ( sms_config_t ) );
    int i = 0;
    /* send sms to every num */
    for ( i = 0; i < MAX_SMS_STR_NUM && local_sms_config.sms_num [ i ] [ 0 ] != '\0'; i ++ ) {
//	printf("phone nubmer is %s\n", local_sms_config.sms_num[i]);
        ret = send_alarm_message ( local_sms_config.sms_num [ i ], alarm_msg, \
                retrytimes, timeout );
        if ( ret < 0 ) {
            return ret;
        }
    }
    return 0;
}


/*
 * Function to send tel
 * @tel_config: tel config 
 * @alarm_msg:   alarm message
 * @retrytimes:  retry times
 * @timeout:     timeout value
 * RET 0 or 1 on success, otherwise return a negative number
 */

static int send_tel ( tel_config_t * tel_config, char * alarm_msg, \
        unsigned int retrytimes, unsigned int timeout ) {
    if ( tel_config == NULL || alarm_msg == NULL || alarm_msg [ 0 ] == '\0' ) {
        return -1;
    }
    int ret;
    /* tel config */
    tel_config_t local_tel_config;
    memcpy ( & local_tel_config, tel_config, sizeof ( tel_config_t ) );
    int i = 0;
    /* send sms to every num */
    for ( i = 0; i < MAX_SMS_STR_NUM && local_tel_config.tel_num [ i ] [ 0 ] != '\0'; i ++ ) {
//	printf("tel phone nubmer is %s\n", local_tel_config.tel_num[i]);
        ret = send_alarm_telephone ( local_tel_config.tel_num [ i ], alarm_msg, \
                retrytimes, timeout );
        if ( ret < 0 ) {
            return ret;
        }
    }
    return 0;
}






/*
 * Function to check sms hardware and software config
 * RET 0 or 1 on success, otherwise return a negative number
 */

int check_sms ( ) {
    int ret;
    ret = access ( GNOKII_CMD, F_OK );
    if ( ret == -1 ) {
        return -100;
    }
    ret = access ( GNOKII_CMD, X_OK );
    if ( ret == -1 ) {
        ret = chmod ( GNOKII_CMD, 00755 );
        if ( ret == -1 ) {
            return -101;
        }
    }
    return send_sms ( & ( g_ex_alarm_config.sms_config ), ALARM_TEST_INFO, 0, 15 );
}


/*
 * Function to send 10 sms
 * @num:            sms num
 * @alarm_msg:      alarm message
 * @retrytimes:     retry times
 * @timeout:        timeout value
 * RET 0 or 1 on success, otherwise return a negative number
 */

int send_alarm_message ( char * num, char * alarm_msg, \
        unsigned int retrytimes, unsigned int timeout ) {
    if ( num == NULL || alarm_msg == NULL || num [ 0 ] == '\0' || alarm_msg [ 0 ] == '\0' ) {
        return -1;
    }
    int ret;
    char * start = alarm_msg;
    char * end = start;
    const int MSG_COUNT = 10;
    int count = 0;
    while ( * end != '\0' ) {
        if ( * end == '\n' ) {
            count ++;
        }
        if ( count == MSG_COUNT ) {
            /* 10 sms then send */
            char * prt = strndup ( start, end - start + 1 );
            ret = send_alarm_sms ( num, prt, retrytimes, timeout );
            free ( prt );
            count = 0;
            start = end + 1;
            if ( ret < 0 ) {
                return ret;
            }
        }
        end ++;
    }
    if ( count > 0 ) {
        /* send sms */
        ret = send_alarm_sms ( num, start, retrytimes, timeout );
        if ( ret < 0 ) {
            return ret;
        }
    }
    return 0;
}



/*
 * Function to send 10 tel
 * @num:            tel num
 * @alarm_msg:      alarm telephone
 * @retrytimes:     retry times
 * @timeout:        timeout value
 * RET 0 or 1 on success, otherwise return a negative number
 */

int send_alarm_telephone ( char * num, char * alarm_msg, \
        unsigned int retrytimes, unsigned int timeout ) {
    if ( num == NULL || num [ 0 ] == '\0' ) {
        return -1;
    }
    	int ret;
        /* send sms */
        ret = send_alarm_tel ( num, alarm_msg, retrytimes, timeout );
        if ( ret < 0 ) {
            return ret;
        }
    return 0;
}




/*
 * Function to send alarm message by sms
 * @num:            sms num
 * @alarm_msg:      alarm message
 * @retrytimes:     retry times
 * @timeout:        timeout value
 * RET 0 or 1 on success, otherwise return a negative number
 */

int send_alarm_sms ( char * num, char * alarm_msg, \
        unsigned int retrytimes, unsigned timeout ) {
    if ( num == NULL || alarm_msg == NULL || num [ 0 ] == '\0' || alarm_msg [ 0 ] == '\0' ) {
        return -1;
    }
    char result [ SMS_RESULT_LEN ];
    int exit_status;
    /* send cmd */
    char cmd [ MAX_SMS_CMD_LEN ];
    int ret;
    sprintf ( cmd, "echo \"%s\" | %s %s --sendsms %s", alarm_msg, GNOKII_CMD, GNOKII_CONFIG, num );
    int re_send = 0;
SMS_RE_SEND:
    ret = c_execute ( cmd, result, SMS_RESULT_LEN, & exit_status, timeout );
    if ( ret > 0 ) {
        result [ ret ] = '\0';
    }
//    printf("************ret == %d, exit_status = %d, result=%s, alarm_msg=%s\n", ret, exit_status, result, alarm_msg);
    /* if fail */
    if ( exit_status != 0 //|| \
//            ret < strlen ( SMS_END_INFO ) \
//            || strcmp ( result + ret - strlen ( SMS_END_INFO ), SMS_END_INFO ) != 0
											) 
	{
        sleep ( SMS_RE_SEND_INTERVAL );
        if ( ++ re_send > retrytimes ) {
            return -2;
        }
        //retry
        goto SMS_RE_SEND;
    }
    return 0;
}

/*end of file*/


/*
 * Function to send alarm message by tel
 * @num:            tel num
 * @alarm_msg:      alarm message
 * @retrytimes:     retry times
 * @timeout:        timeout value
 * RET 0 or 1 on success, otherwise return a negative number
 */

int send_alarm_tel ( char * num, char * alarm_msg, \
        unsigned int retrytimes, unsigned timeout ) {
    if ( num == NULL || alarm_msg == NULL || num [ 0 ] == '\0' || alarm_msg [ 0 ] == '\0' ) {
        return -1;
    }
    char result [ SMS_RESULT_LEN ];
    int exit_status;
    /* send cmd */
    char cmd [ MAX_SMS_CMD_LEN ];
    int ret;
    sprintf ( cmd, "%s %s --dialvoice %s", GNOKII_CMD, GNOKII_CONFIG, num );
    ret = c_execute ( cmd, result, SMS_RESULT_LEN, & exit_status, timeout );
    if ( ret > 0 ) {
        result [ ret ] = '\0';
    }
	return 0;
}


