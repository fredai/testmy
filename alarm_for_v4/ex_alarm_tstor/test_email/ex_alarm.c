/*
 *
 * ex_alarm.c
 *
 */


#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include <pthread.h>

#include "ex_alarm.h"
#include "alarm.h"
#include "config.h"
#include "utils.h"
#include "db.h"
#include "log.h"

#define START_INFO_A "Extensible alarm software started successfully"
#define START_INFO_C "If email configuration is correct, the recipient will receive a test message"
#define START_INFO_D "If sms configuration is correct, the recipient will receive a test message"
#define START_INFO_B "Another process is running"
#define CHECK_SMS_ERROR "Failed to send a test message, please check the hardware and software configuration\n"


static int is_match_value_relation ( alarm_condition_t * alarm_condition, float value );
static alarm_node_t * find_alarm_node ( alarm_item_t * alarm_item, char * node_name );
static int process_alarm_data ( alarm_items_t * alarm_items, alarm_data_t * alarm_data );
static int process_common_level_alarm_data ( alarm_item_t * alarm_item,
        alarm_node_t * alarm_node, alarm_data_t * alarm_data );
static int process_warn_level_alarm_data ( alarm_item_t * alarm_item,
        alarm_node_t * alarm_node, alarm_data_t * alarm_data );

ex_alarm_config_t g_ex_alarm_config;
extern alarm_msg_buf_t g_alarm_msg_buf;
extern pthread_mutex_t mutex_g_alarm_msg_buf;


int main ( int argc, char * argv [ ] ) {

    int ret;
    /* data id */
    long long int last_data_id;
    /* alarm data */
    alarm_data_t alarm_data;

    /* error message */
    char error_msg [ ALARM_ERRMSG_LEN ];
    error_msg [ 0 ] = 0;
    /* alarm item */
    alarm_items_t alarm_items;

    /* check if there is another process */
    FILE * pid_file = fopen ( EX_ALARM_PID_FILE, "r" );
    if ( pid_file != NULL ) {
        char pid_buffer [ 32 ];
        strcpy ( pid_buffer, "/proc/" );
        char * pid_str = fgets ( pid_buffer + strlen ( "/proc/" ), 15, pid_file );
        fclose ( pid_file );
        if ( pid_str != NULL && * pid_str != '\0' ) {
            ret = access ( pid_buffer, F_OK );
            if ( ret == 0 ) {
                fprintf ( stderr, "%s PID %s\n", START_INFO_B, pid_buffer + strlen ( "/proc/" ) );
                exit ( 1 );
            }
        }
    }

    /* init alarm items */
    ret = init_alarm_items ( & alarm_items );

    cs_log_start;

    /* init config */
    bzero ( & g_ex_alarm_config, sizeof ( ex_alarm_config_t ) );
    ret = init_config ( CONFIG_FILE_PATH, & g_ex_alarm_config, & alarm_items, error_msg );
    if ( ret < 0 ) {
        fprintf ( stderr, "init config error: %s\n", error_msg );
        cs_log ( LOG_FATAL, "init config error: return %d %s", ret, error_msg );
        exit ( 2 );
    }

    db_config_t * db_config = & ( g_ex_alarm_config.db_config );

    cs_log ( LOG_DEBUG, "db config is [ip %s] [port %u] [dbname %s] [username %s] [password %s]", \
	      db_config -> db_server_ip, db_config -> db_server_port, \
	      db_config -> db_name, db_config -> db_username, db_config -> db_password );

    /* get db connection */
    MYSQL * connection = NULL;
    MYSQL_RES * results;
    ret = mysql_get_connection ( db_config, & connection );
    if ( ret < 0 ) {
	cs_log ( LOG_FATAL, "get db connection error: ret = %d", ret );
	fprintf ( stderr, "get db connection to %s:%d error\n", \
                db_config -> db_server_ip, db_config -> db_server_port );
	exit ( 3 );
    }
    cs_log ( LOG_DEBUG, "get db connection success" );

    /* get init id */
    ret = get_init_id ( connection, & last_data_id );
    if ( ret < 0 ) {
	cs_log ( LOG_FATAL, "get init id error: ret = %d", ret );
	fprintf ( stderr, "get init id error\n" );
	exit ( 4 );
    }
    cs_log ( LOG_DEBUG, "get init id success" );
    cs_log_end;

    /* check db config */
    ret = check_database ( connection );
    if ( ret < 0 ) {
        fprintf ( stderr, "Test databas error, lack of table alarm_info\n" );
        exit ( 5 );
    }

    alarm_method_switch_t method_email = g_ex_alarm_config.alarm_method.email;
    alarm_method_switch_t method_sms = g_ex_alarm_config.alarm_method.sms;

    /* check email config */
    if ( method_email == METHOD_ON ) {
        char * check_err_msg = ( char * ) malloc ( ALARM_ERRMSG_LEN * 16 );
        if ( check_err_msg == NULL ) {
            exit ( 6 );
        }
        ret = send_email ( & ( g_ex_alarm_config.email_config ), \
                ALARM_TEST_SUBJECT, ALARM_TEST_INFO, check_err_msg, ALARM_ERRMSG_LEN * 16 );
        if ( ret != 0 ) {
            fprintf ( stderr, "Test email error\n%s\n", check_err_msg );
            free ( check_err_msg );
            exit ( 7 );
        }
        free ( check_err_msg );
    }

    /* check sms config */
    if ( method_sms == METHOD_ON ) {
        ret = check_sms ( );
        if ( ret < 0 ) {
            fprintf ( stderr, CHECK_SMS_ERROR );
            exit ( 8 );
        }
    }
 
    /* daemonize */
    pid_t daemonize;
    daemonize = fork ( );
    if ( daemonize < 0 ) {
        fprintf ( stderr, "Can't run as a daemon process\n" );
    }
    else if ( daemonize > 0 ) {
        fprintf ( stdout, "%s\n", START_INFO_A );
        if ( g_ex_alarm_config.alarm_method.email == METHOD_ON ) {
            fprintf ( stdout, "%s\n", START_INFO_C );
        }
        if ( g_ex_alarm_config.alarm_method.sms == METHOD_ON ) {
            fprintf ( stdout, "%s\n", START_INFO_D );
        }
        exit ( 0 );
    }

    freopen ( "/dev/null", "r", stdin );
    freopen ( "/dev/null", "w", stdout );
    freopen ( "/dev/null", "w", stderr );

    /* write pid to file */
    pid_file = fopen ( EX_ALARM_PID_FILE, "w" );
    if ( pid_file != NULL ) {
        fprintf ( pid_file, "%d", getpid ( ) );
        fclose ( pid_file );
    }

    cs_log_start;
    cs_log ( LOG_DEBUG, "ex alarm started" );

    /* start thread send_alarm_msg_from_buffer */
    pthread_mutex_init ( & mutex_g_alarm_msg_buf, NULL );
    pthread_t thread_snd;
    ret = pthread_create ( & thread_snd, NULL, send_alarm_msg_from_buffer, NULL );
    if ( ret != 0 ) {
	cs_log ( LOG_FATAL, "create thread send alarm msg from buffer error: ret = %d", ret );
        exit ( 9 );
    }

    while ( 1 ) {

        /* get data */
	ret = get_data_from_db ( connection, last_data_id, & results );
	if ( ret < 0 ) {
	    exit ( 10 );
	}

        /* fetch and process data */
	while ( fetch_alarm_data ( results, & alarm_data ) ) {
            ret = process_alarm_data ( & alarm_items, & alarm_data );
            if ( ret == -2 ) {
                continue;
            }
            else if ( ret < 0 ) {
	        cs_log ( LOG_FATAL, "process alarm data error: ret = %d", ret );
                exit ( 11 );
            }
            last_data_id = alarm_data.id; 
	}

	ex_mysql_free_results ( results );

        sleep ( ALARM_GET_DATA_INTERVAL );

    }

    ex_mysql_close ( connection );

    destroy_alarm_items ( & alarm_items );

    pthread_join ( thread_snd, NULL );
    pthread_mutex_destroy ( & mutex_g_alarm_msg_buf );

    cs_log_end;

    exit ( 0 );

}

/*
 * Function to process alarm data
 * @alarm_item:    pointer to the alarm item structure
 * @alarm data:    alarm data from db 
 * RET 0 on success, otherwise return a negative number
 */

static int process_alarm_data ( alarm_items_t * alarm_items, alarm_data_t * alarm_data ) {

    if ( alarm_items == NULL || alarm_data == NULL ) {
        return -1;
    }
    
    char * item_name = alarm_data -> item;

    alarm_item_t * p = alarm_items -> alarm_item_head.next;

    /* find alarm item */
    while ( p ) {
        if ( strcmp ( p -> alarm_item_name, item_name ) == 0 ) {
            break;
        }
        p = p -> next;
    }
    if ( p == NULL ) {
        return -2;
    }

    /* weather match the alarm condition in serious level */
    int is_serious = is_match_value_relation ( 
                & ( p -> alarm_condition [ ALARM_LEVEL_SERIOUS ] ), 
                alarm_data -> value );

    /* find alarm node */
    alarm_node_t * alarm_node = find_alarm_node ( p, alarm_data -> node );
    if ( alarm_node == NULL ) {
        return -3;
    }
    
    pthread_mutex_lock ( & ( alarm_node -> alarm_thread_mutex [ ALARM_LEVEL_SERIOUS ] ) );
    pthread_mutex_lock ( & ( alarm_node -> alarm_thread_mutex [ ALARM_LEVEL_COMMON ] ) );
    pthread_mutex_lock ( & ( alarm_node -> alarm_thread_mutex [ ALARM_LEVEL_WARN ] ) );

    /* match */
    if ( is_serious ) {
        /* have not alarm and common level have not start timer */
        if ( alarm_node -> alarm_flag [ ALARM_LEVEL_SERIOUS ] == HAVE_NOT_ALARM && \
                alarm_node -> timer_start [ ALARM_LEVEL_COMMON ] == 0 ) {
            alarm_node -> timer_start [ ALARM_LEVEL_COMMON ] = time ( NULL );
        }
        /* have not alarm and warn level have not start timer */
        if ( alarm_node -> alarm_flag [ ALARM_LEVEL_SERIOUS ] == HAVE_NOT_ALARM && \
                alarm_node -> timer_start [ ALARM_LEVEL_WARN ] == 0 ) {
            alarm_node -> timer_start [ ALARM_LEVEL_WARN ] = time ( NULL );
        }
        /* serious level have not start timer */
        if ( alarm_node -> timer_start [ ALARM_LEVEL_SERIOUS ] == 0 ) {
            alarm_node -> timer_start [ ALARM_LEVEL_SERIOUS ] = time ( NULL );
        }
        else {
            /* alarm condition in level serious */
            time_t con_secs = p -> alarm_condition [ ALARM_LEVEL_SERIOUS ].secs;
            /* match */
            if ( time ( NULL ) - alarm_node -> timer_start [ ALARM_LEVEL_SERIOUS ] >= con_secs ) {
                /* have not alarm in level serious */
                if ( alarm_node -> alarm_flag [ ALARM_LEVEL_SERIOUS ] == HAVE_NOT_ALARM ) {
                    /* is alarming in level common */
                    if ( alarm_node -> alarm_flag [ ALARM_LEVEL_COMMON ] == ALARMING ) {
                        /* mutex */
                        alarm_node -> thread_cancel [ ALARM_LEVEL_COMMON ] = 1;
                        alarm_node -> alarm_flag [ ALARM_LEVEL_COMMON ] = HAVE_NOT_ALARM;
                        alarm_node -> timer_start [ ALARM_LEVEL_COMMON ] = 0;
                    }
                    /* is alarming in level warn */
                    if ( alarm_node -> alarm_flag [ ALARM_LEVEL_WARN ] == ALARMING ) {
                        /* mutex */
                        alarm_node -> thread_cancel [ ALARM_LEVEL_WARN ] = 1;
                        alarm_node -> alarm_flag [ ALARM_LEVEL_WARN ] = HAVE_NOT_ALARM;
                        alarm_node -> timer_start [ ALARM_LEVEL_WARN ] = 0;
                    }

                    /* create a thread to send alarm message */
                    pthread_t tid;
                    alarm_th_stob_t * th_arg = ( alarm_th_stob_t * ) \
                                               malloc ( sizeof ( alarm_th_stob_t ) );
                    if ( th_arg == NULL ) {
                        return -5;
                    }
                    th_arg -> alarm_item = p;
                    th_arg -> alarm_node = alarm_node;
                    th_arg -> alarm_level = ALARM_LEVEL_SERIOUS;
                    /* ignore return value */
                    pthread_create ( & tid, NULL, send_alarm_msg_to_buffer, ( void * ) th_arg );

                    /* modify alarm flag and timer */
                    alarm_node -> alarm_flag [ ALARM_LEVEL_SERIOUS ] = ALARMING;
                    alarm_node -> timer_start [ ALARM_LEVEL_COMMON ] = 0;
                    alarm_node -> timer_start [ ALARM_LEVEL_WARN ] = 0;

                }
                else {
                    /* ignore */
                }
            }
            else {
                /* ignore */
            }
        }
    }
    /* not match alarm condition */
    else {
        /* timer have started */
        if ( alarm_node -> timer_start [ ALARM_LEVEL_SERIOUS ] > 0 ) {
            /* clean timer */
            alarm_node -> timer_start [ ALARM_LEVEL_SERIOUS ] = 0; 
        }
        /* is alarming */
        if ( alarm_node -> alarm_flag [ ALARM_LEVEL_SERIOUS ] == ALARMING ) {
            /* cancel thread */
            alarm_node -> thread_cancel [ ALARM_LEVEL_SERIOUS ] = 1;
        }
        /*else if ( alarm_node -> alarm_flag [ ALARM_LEVEL_SERIOUS ] == ALARMED ) {
            alarm_node -> end_time [ ALARM_LEVEL_SERIOUS ] = time ( NULL );
        }*/
        /* set alarm flag */
        alarm_node -> alarm_flag [ ALARM_LEVEL_SERIOUS ] = HAVE_NOT_ALARM;
        /* process data in level common */
        int ret = process_common_level_alarm_data ( p, alarm_node, alarm_data );
        if ( ret < 0 ) {
            return -4;
        }
    }

    pthread_mutex_unlock ( & ( alarm_node -> alarm_thread_mutex [ ALARM_LEVEL_WARN ] ) );
    pthread_mutex_unlock ( & ( alarm_node -> alarm_thread_mutex [ ALARM_LEVEL_COMMON ] ) );
    pthread_mutex_unlock ( & ( alarm_node -> alarm_thread_mutex [ ALARM_LEVEL_SERIOUS ] ) );

    return 0;

}


/*
 * Function to process common level alarm data
 * @alarm_item:    pointer to the alarm item structure
 * @alarm_node:    pointer to the alarm node structure    
 * @alarm_data:    alarm data from caller 
 * RET 0 on success, otherwise return a negative number
 */

static int process_common_level_alarm_data ( alarm_item_t * alarm_item, 
        alarm_node_t * alarm_node, alarm_data_t * alarm_data ) {

    if ( alarm_item == NULL || alarm_node == NULL || alarm_data == NULL ) {
        return -1;
    }

    alarm_item_t * p = alarm_item;

    /* weather match alarm value condition in level common */
    int is_common = is_match_value_relation ( 
                & ( p -> alarm_condition [ ALARM_LEVEL_COMMON ] ), 
                alarm_data -> value );

    /* match */
    if ( is_common ) {
        /* have not alarm in level common and timer in level warn is not start */
        if ( alarm_node -> alarm_flag [ ALARM_LEVEL_COMMON ] == HAVE_NOT_ALARM && \
                alarm_node -> timer_start [ ALARM_LEVEL_WARN ] == 0 ) {
            /* start timer */
            alarm_node -> timer_start [ ALARM_LEVEL_WARN ] = time ( NULL );
        }
        /* have not alarm in level common */
        if ( alarm_node -> timer_start [ ALARM_LEVEL_COMMON ] == 0 ) {
            /* start timer */
            alarm_node -> timer_start [ ALARM_LEVEL_COMMON ] = time ( NULL );
        }
        /* otherwise */
        else {
            time_t con_secs = p -> alarm_condition [ ALARM_LEVEL_COMMON ].secs;
            /* weather match alarm time condition in level common */
            if ( time ( NULL ) - alarm_node -> timer_start [ ALARM_LEVEL_COMMON ] >= con_secs ) {
                /* match */
                /* whether common level have not alarm */
                if ( alarm_node -> alarm_flag [ ALARM_LEVEL_COMMON ] == HAVE_NOT_ALARM ) {
                    if ( alarm_node -> alarm_flag [ ALARM_LEVEL_SERIOUS ] == ALARMING ) {
                        /* mutex */
                        alarm_node -> thread_cancel [ ALARM_LEVEL_SERIOUS ] = 1;
                        alarm_node -> alarm_flag [ ALARM_LEVEL_SERIOUS ] = HAVE_NOT_ALARM;
                        alarm_node -> timer_start [ ALARM_LEVEL_SERIOUS ] = 0;
                    }
                    if ( alarm_node -> alarm_flag [ ALARM_LEVEL_WARN ] == ALARMING ) {
                        /* mutex */
                        alarm_node -> thread_cancel [ ALARM_LEVEL_WARN ] = 1;
                        alarm_node -> alarm_flag [ ALARM_LEVEL_WARN ] = HAVE_NOT_ALARM;
                        alarm_node -> timer_start [ ALARM_LEVEL_WARN ] = 0;
                    }

                    /* create a thread to send alarm message */
                    pthread_t tid;

                    alarm_th_stob_t * th_arg = ( alarm_th_stob_t * ) \
                                               malloc ( sizeof ( alarm_th_stob_t ) );
                    if ( th_arg == NULL ) {
                        return -3;
                    }
                    th_arg -> alarm_item = p;
                    th_arg -> alarm_node = alarm_node;
                    th_arg -> alarm_level = ALARM_LEVEL_COMMON;
                    pthread_create ( & tid, NULL, send_alarm_msg_to_buffer, ( void * ) th_arg );
                   
                    /* set alarm flag and timer */
                    alarm_node -> alarm_flag [ ALARM_LEVEL_COMMON ] = ALARMING;
                    alarm_node -> timer_start [ ALARM_LEVEL_SERIOUS ] = 0;
                    alarm_node -> timer_start [ ALARM_LEVEL_WARN ] = 0;

                }
                else {
                    /* ignore */
                }
            }
            else {
                /* ignore */
            }
        }
    }
    else {
        if ( alarm_node -> timer_start [ ALARM_LEVEL_COMMON ] > 0 ) {
            /* clear timer */
            alarm_node -> timer_start [ ALARM_LEVEL_COMMON ] = 0; 
        }
        if ( alarm_node -> alarm_flag [ ALARM_LEVEL_COMMON ] == ALARMING ) {
            /* cancel thread */
            alarm_node -> thread_cancel [ ALARM_LEVEL_COMMON ] = 1;
        }
        /*else if ( alarm_node -> alarm_flag [ ALARM_LEVEL_COMMON ] == ALARMED ) {
            alarm_node -> end_time [ ALARM_LEVEL_COMMON ] = time ( NULL );
        }*/
        alarm_node -> alarm_flag [ ALARM_LEVEL_COMMON ] = HAVE_NOT_ALARM;

        /* process alarm data in level warn */
        int ret = process_warn_level_alarm_data ( alarm_item, alarm_node, alarm_data );
        if ( ret < 0 ) {
            return -2;
        }
    }

    return 0;

}


/*
 * Function to process warn level alarm data
 * @alarm_item:    pointer to the alarm item structure
 * @alarm_node:    pointer to the alarm node structure    
 * @alarm_data:    alarm data from caller 
 * RET 0 on success, otherwise return a negative number
 */

static int process_warn_level_alarm_data ( alarm_item_t * alarm_item, 
        alarm_node_t * alarm_node, alarm_data_t * alarm_data ) {

    if ( alarm_item == NULL || alarm_node == NULL || alarm_data == NULL ) {
        return -1;
    }

    alarm_item_t * p = alarm_item;

    /* whether match alarm value condition in level warn */
    int is_warn = is_match_value_relation ( 
                & ( p -> alarm_condition [ ALARM_LEVEL_WARN ] ), 
                alarm_data -> value );

    /* match */
    if ( is_warn ) {
        if ( alarm_node -> timer_start [ ALARM_LEVEL_WARN ] == 0 ) {
            alarm_node -> timer_start [ ALARM_LEVEL_WARN ] = time ( NULL );
        }
        else {
            time_t con_secs = p -> alarm_condition [ ALARM_LEVEL_WARN ].secs;
            /* wheter match time alarm condition */
            if ( time ( NULL ) - alarm_node -> timer_start [ ALARM_LEVEL_WARN ] >= con_secs ) {
                /* match */
                if ( alarm_node -> alarm_flag [ ALARM_LEVEL_WARN ] == HAVE_NOT_ALARM ) {
                    if ( alarm_node -> alarm_flag [ ALARM_LEVEL_SERIOUS ] == ALARMING ) {
                        alarm_node -> thread_cancel [ ALARM_LEVEL_SERIOUS ] = 1;
                        alarm_node -> alarm_flag [ ALARM_LEVEL_SERIOUS ] = HAVE_NOT_ALARM;
                        alarm_node -> timer_start [ ALARM_LEVEL_SERIOUS ] = 0;
                    }
                    if ( alarm_node -> alarm_flag [ ALARM_LEVEL_COMMON ] == ALARMING ) {
                        alarm_node -> thread_cancel [ ALARM_LEVEL_COMMON ] = 1;
                        alarm_node -> alarm_flag [ ALARM_LEVEL_COMMON ] = HAVE_NOT_ALARM;
                        alarm_node -> timer_start [ ALARM_LEVEL_COMMON ] = 0;
                    }

                    /* create a new thread to send alarm message */
                    pthread_t tid;
                    alarm_th_stob_t * th_arg = ( alarm_th_stob_t * ) \
                                               malloc ( sizeof ( alarm_th_stob_t ) );
                    if ( th_arg == NULL ) {
                        return -2;
                    }

                    th_arg -> alarm_item = p;
                    th_arg -> alarm_node = alarm_node;
                    th_arg -> alarm_level = ALARM_LEVEL_WARN;
                    pthread_create ( & tid, NULL, send_alarm_msg_to_buffer, ( void * ) th_arg );

                    /* set alarm flag and timer */
                    alarm_node -> alarm_flag [ ALARM_LEVEL_WARN ] = ALARMING;
                    alarm_node -> timer_start [ ALARM_LEVEL_SERIOUS ] = 0;
                    alarm_node -> timer_start [ ALARM_LEVEL_COMMON ] = 0;

                }
                else {
                    /* ignore */
                }
            }
            else {
                /* ignore */
            }
        }
    }
    else {
        if ( alarm_node -> timer_start [ ALARM_LEVEL_WARN ] > 0 ) {
            /* clear timer */
            alarm_node -> timer_start [ ALARM_LEVEL_WARN ] = 0; 
        }
        if ( alarm_node -> alarm_flag [ ALARM_LEVEL_WARN ] == ALARMING ) {
            /* cancel thread */
            alarm_node -> thread_cancel [ ALARM_LEVEL_WARN ] = 1;
        }
        /*else if ( alarm_node -> alarm_flag [ ALARM_LEVEL_WARN ] == ALARMED ) {
            alarm_node -> end_time [ ALARM_LEVEL_WARN ] = time ( NULL );
        }*/
        alarm_node -> alarm_flag [ ALARM_LEVEL_WARN ] = HAVE_NOT_ALARM;
    }

    return 0;

}


/*
 * Function to process warn level alarm data
 * @alarm_item:    pointer to the alarm condition structure
 * @value:         alarm data value
 * RET 0 or 1 on success, otherwise return a negative number
 */

static int is_match_value_relation ( alarm_condition_t * alarm_condition, float value ) {
#define PRECISION 1.0e-6
    switch ( alarm_condition -> alarm_relation ) {
        case GT: return value > alarm_condition -> value;
        case LT: return value < alarm_condition -> value;
        case EQ: return fabsf ( value - alarm_condition -> value ) < PRECISION;
        case GE: return value >= alarm_condition -> value;
        case LE: return value <= alarm_condition -> value;
        case NQ: return ! ( fabsf ( value - alarm_condition -> value ) < PRECISION );
        default: return -1;
    }
}


/*
 * Function to process warn level alarm data
 * @alarm_item:    pointer to the alarm condition structure
 * @node_name:     node name
 * RET pointer to alarm node on success, otherwise return NULL
 */

static alarm_node_t * find_alarm_node ( alarm_item_t * alarm_item, char * node_name ) {

    if ( alarm_item == NULL || node_name == NULL || node_name [ 0 ] == '\0' ) {
        return NULL;
    }

    alarm_node_t * p = & ( alarm_item -> alarm_node_head );

    while ( p -> next ) {
        if ( strcmp ( p -> next -> alarm_node_name, node_name ) == 0 ) {
            break;
        }
        p = p -> next;
    }
    if ( p -> next ) {
        /* a exist node */
        return p -> next;
    }
    else {
        alarm_node_t * new_alarm_node = ( alarm_node_t * ) malloc ( sizeof ( alarm_node_t ) );
        if ( new_alarm_node == NULL ) {
            return NULL;
        }
        /* init */
        bzero ( new_alarm_node, sizeof ( alarm_node_t ) );
        strcpy ( new_alarm_node -> alarm_node_name, node_name );
        pthread_mutex_init ( & ( new_alarm_node -> alarm_thread_mutex [ ALARM_LEVEL_SERIOUS ] ), NULL );
        pthread_mutex_init ( & ( new_alarm_node -> alarm_thread_mutex [ ALARM_LEVEL_COMMON ] ), NULL );
        pthread_mutex_init ( & ( new_alarm_node -> alarm_thread_mutex [ ALARM_LEVEL_WARN ] ), NULL );
        p -> next = new_alarm_node;
        /* a new node */
        return p -> next;
    }

    return NULL;
}




/*end of file*/

