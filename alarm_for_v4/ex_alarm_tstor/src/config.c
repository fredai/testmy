/*
 *
 * config.c
 *
 */

#include <unistd.h>
#include <stdlib.h>

#include "ex_alarm.h"
#include "config.h"
#include "utils.h"
#include "log.h"

#define CF_STR_DB "db"
#define CF_STR_DB_SERVER_IP "server_ip"
#define CF_STR_DB_SERVER_PORT "port"
#define CF_STR_DB_NAME "db_name"
#define CF_STR_DB_USER_NAME "username"
#define CF_STR_DB_PASSWORD "password"

#define CF_STR_ALARM_METHOD "alarm_method"
#define CF_STR_ALARM_METHOD_EMAIL "email"
#define CF_STR_ALARM_METHOD_SMS "sms"
#define CF_STR_ALARM_METHOD_TEL "tel"

#define CF_STR_SMS "sms"
#define CF_STR_TEL "tel"

#define CF_STR_EMAIL "email"
#define CF_STR_EMAIL_SMTP "smtp_server"
#define CF_STR_EMAIL_SMTP_PORT "smtp_port"
#define CF_STR_EMAIL_FROM "from"
#define CF_STR_EMAIL_NAME "name"
#define CF_STR_EMAIL_PASSWORD "password"
#define CF_STR_EMAIL_TO "to"
#define CF_STR_EMAIL_CC "cc"
#define CF_STR_EMAIL_USE_TLS_SSL "use_tls_ssl"

#define CF_STR_ALARM_FREQ "alarm_freq"
#define CF_STR_ALARM_FREQ_INTERVAL "interval"
#define CF_STR_ALARM_FREQ_TIMES "times"

#define CF_STR_ALARM_ITEM "alarm_item"

#define DEFAULT_DB_PORT 3306

static int parse_cf_alarm_method ( alarm_method_t * alarm_method, char * value, char * error_msg );
static int parse_cf_db_config ( db_config_t * db_config, char * value, char * error_msg );
static int parse_cf_sms_config ( sms_config_t * sms_config, char * value, char * error_msg );
static int parse_cf_tel_config ( tel_config_t * tel_config, char * value, char * error_msg );
static int parse_cf_email_config ( email_config_t * email_config, char * value, char * error_msg );
static int parse_cf_alarm_freq ( alarm_freq_t * alarm_freq, char * value, char * error_msg );
static int parse_cf_alarm_item ( alarm_items_t * alarm_items, char * str, char * error_msg );
static int parse_alarm_condition ( alarm_condition_t * alarm_condition, \
        char * str, char * error_msg );
static int parse_alarm_condition_relation ( alarm_relation_t * alarm_relation, char * str );
static int read_config_from_file ( char * config_file_path, ex_alarm_config_t * ex_alarm_config, \
        alarm_items_t * alarm_items, char * error_msg );
static int check_email_addr_format ( char * email_addr, int email_addr_len );
static int check_sms_num_format ( char * sms_num, int sms_num_len );
static int check_alarm_item_conditon ( const alarm_item_t * alarm_item );


/*
 * Function to read config from file
 * @config_file_path:   path of config file
 * @ex_alarm_config::   pointer to alarm config 
 * @alarm_items: pointer to alarm items
 * @error_msg: pointer to error message
 * RET 0 or 1 on success, otherwise return a negative number
 */

static int read_config_from_file ( char * config_file_path, ex_alarm_config_t * ex_alarm_config, \
        alarm_items_t * alarm_items, char * error_msg ) {

    cs_log ( LOG_INFO, "start read config from file" );
    if ( config_file_path == NULL || ex_alarm_config == NULL || \
            alarm_items == NULL || config_file_path [ 0 ] == '\0' ) {
        cs_log ( LOG_ERROR, "invalid parameters" );
        return -1;
    }

    int ret = -1;
    char errmsg [ ALARM_ERRMSG_LEN ];
    int str_len = 0;
    FILE * config_file;
#define padding_len 8
    /* line buffer */
    char line_buffer [ MAX_CONFIG_FILE_LINE_LEN + padding_len ];

    bzero ( ex_alarm_config, sizeof ( ex_alarm_config_t ) );

    /* open file */
    config_file = fopen ( config_file_path, "r" );
    if ( config_file == NULL ) {
        strerror_r ( errno, errmsg, ALARM_ERRMSG_LEN );
        if ( errno == ENOENT ) {
            cs_log ( LOG_ERROR, "open config file %s error: file not exsit, %s", \
                    config_file_path, errmsg );
            sprintf ( error_msg, "open config file %s error: file not exsit, %s", \
                    config_file_path, errmsg );
            return -2; /* file not exist */
        }
        else {
            cs_log ( LOG_ERROR, "open config file %s error, errno is %d, %s", 
                    config_file_path, errno, errmsg );
            sprintf ( error_msg, "open config file %s error, errno is %d, %s", 
                    config_file_path, errno, errmsg );
            return -3; /* open file error */
        }
    }

    /* read a line */
    while ( fgets ( line_buffer, MAX_CONFIG_FILE_LINE_LEN, config_file ) ) {

        str_len = strlen ( line_buffer );
        line_buffer [ str_len - 1 ] = 0;
        str_len --; 

        if ( line_buffer [ str_len - 2 ] == '\r' ) { 
            line_buffer [ str_len - 2 ] = 0;
            str_len --; 
        }   

        /* too long */
        if ( str_len > MAX_CONFIG_FILE_LINE_LEN ) { 
            cs_log ( LOG_WARN, "line too long, discard, %s", line_buffer );
            sprintf ( error_msg, "line too long, discard, %s", line_buffer );
            return -4;
        }

        char * line_buffer_trim_head = trim_head ( line_buffer, str_len );

        /* start with # */
        if ( * line_buffer_trim_head == '#' ) {
            continue;
        }
        if ( * line_buffer_trim_head == 0 ) {
            continue;
        }

        /* parse a config line */
        ret = parse_alarm_config_line ( line_buffer_trim_head, ex_alarm_config, \
                alarm_items, error_msg );
        if ( ret < 0 ) {
            return -5;
        }

    }

    /* close file */
    fclose ( config_file );

    return 0;

}


/*
 * Function to parse alarm config line
 * @config_line:   config line 
 * @ex_alarm_config::   pointer to alarm config 
 * @alarm_items pointer to alarm items
 * @error_msg pointer to error message
 * RET 0 or 1 on success, otherwise return a negative number
 */

int parse_alarm_config_line ( char * config_line, ex_alarm_config_t * ex_alarm_config, \
        alarm_items_t * alarm_items, char * error_msg ) {

    if ( config_line == NULL || ex_alarm_config == NULL || config_line [ 0 ] == '\0' ) {
        cs_log ( LOG_ERROR, "invalid parameters" );
        return -1;
    }

    int ret;
    char config_item [ MAX_CONFIG_ITEM_LEN + 1 ];
    char * delim = " \t\r\n\f\v";
    char * saveptr;
    char * token;
    char * str = config_line;

    token = strtok_r ( str, delim, & saveptr );

    /* empty line */
    if ( token == NULL ) {
        cs_log ( LOG_DEBUG, "empty line" );
        sprintf ( error_msg, "%s", "empty line" );
        return -2;
    }

    /* item too long */
    if ( strlen ( token ) > MAX_CONFIG_ITEM_LEN ) {
        cs_log ( LOG_WARN, "%s %s", "config item too long", token );
        sprintf ( error_msg, "%s %s", "config item too long", token );
        return -3;
    }

    strcpy ( config_item, token ); /* config item */

    /* alarm method */
    if ( strcmp ( config_item, CF_STR_ALARM_METHOD ) == 0 ) {
        ret = parse_cf_alarm_method ( & ( ex_alarm_config -> alarm_method ), saveptr, error_msg );
        if ( ret < 0 ) {
            return -4;
        }
    }
    /* db config */
    else if ( strcmp ( config_item, CF_STR_DB ) == 0 ) {
        ret = parse_cf_db_config ( & ( ex_alarm_config -> db_config ), saveptr, error_msg );
        if ( ret < 0 ) {
            sprintf ( error_msg, "invalid db config" );
            return -5;
        }
    }
    /* sms config */
    else if ( strcmp ( config_item, CF_STR_SMS ) == 0 ) {
        ret = parse_cf_sms_config ( & ( ex_alarm_config -> sms_config ), saveptr, error_msg );
        if ( ret < 0 ) {
            return -6;
        }
    }
    /* tel config */
    else if ( strcmp ( config_item, CF_STR_TEL ) == 0 ) {
        ret = parse_cf_tel_config ( & ( ex_alarm_config -> tel_config ), saveptr, error_msg );
        if ( ret < 0 ) {
            return -6;
        }
    }
    /* email config */
    else if ( strcmp ( config_item, CF_STR_EMAIL ) == 0 ) {
        ret = parse_cf_email_config ( & ( ex_alarm_config -> email_config ), saveptr, error_msg );
        if ( ret < 0 ) {
            return -7;
        }
    }
    /* alarm freq */
    else if ( strcmp ( config_item, CF_STR_ALARM_FREQ ) == 0 ) {
        ret = parse_cf_alarm_freq ( & ( ex_alarm_config -> alarm_freq ), saveptr, error_msg );
        if ( ret < 0 ) {
            return -8;
        }
    }
    /* alarm items */
    else if ( strcmp ( config_item, CF_STR_ALARM_ITEM ) == 0 ) {
        ret = parse_cf_alarm_item ( alarm_items, saveptr, error_msg );
        if ( ret < 0 ) {
            return -9;
        }
    }

    return 0;

}


/*
 * Function to parse alarm method
 * @alarm_method:   alarm method
 * @str:            pointer to config string
 * @error_msg pointer to error message
 * RET 0 or 1 on success, otherwise return a negative number
 */

static int parse_cf_alarm_method ( alarm_method_t * alarm_method, \
        char * str, char * error_msg ) {

    if ( alarm_method == NULL || error_msg == NULL ) {
        return -1;
    }
    if ( str == NULL ) {
//        return -2;
        return 1;
    }
    if ( str [ 0 ] == '\0' ) {
        sprintf ( error_msg, "alarm method unspecified" );
//        return -3;
        return 2;
    }

    char * delim = " =\t";
    char * saveptr;
    char * token;
 
    for ( ; ; str = NULL ) {
        token = strtok_r ( str, delim, & saveptr );
        if ( token == NULL ) {
            break;
        }
        else if ( strcmp ( token, CF_STR_ALARM_METHOD_EMAIL ) == 0 ) {
            /* email */
            alarm_method -> email = METHOD_ON;
        }
        else if ( strcmp ( token, CF_STR_ALARM_METHOD_SMS ) == 0 ) {
            /* sms */
            alarm_method -> sms = METHOD_ON;
        }
	else if ( strcmp ( token, CF_STR_ALARM_METHOD_TEL ) == 0 ) {
            /* sms */
            alarm_method -> tel = METHOD_ON;
        }
    }

    return 0;

}


/*
 * Function to parse alarm method
 * @db_config:      db config
 * @str:            pointer to config string
 * @error_msg pointer to error message
 * RET 0 or 1 on success, otherwise return a negative number
 */

static int parse_cf_db_config ( db_config_t * db_config, \
        char * str, char * error_msg ) {

    if ( db_config == NULL || str == NULL || error_msg == NULL || str [ 0 ] == '\0' ) {
        return -1;
    }

    char * delim = " =\t";
    char * saveptr;
    char * token;

    for ( ; ; str = NULL ) {
        token = strtok_r ( str, delim, & saveptr );
        if ( token == NULL ) {
            break;
        }
        /* db server ip */
        else if ( strcmp ( token, CF_STR_DB_SERVER_IP ) == 0 ) {
            token = strtok_r ( NULL, delim, & saveptr );
            if ( token != NULL ) {
                if ( ! ip_str_is_valid ( token ) ) {
                    sprintf ( error_msg, "db server ip is invalid %s" , token );
                    return -2;
                }
                strcpy ( db_config -> db_server_ip, token );
            }
        }
        /* db server port */
        else if ( strcmp ( token, CF_STR_DB_SERVER_PORT ) == 0 ) {
            token = strtok_r ( NULL, delim, & saveptr );
            if ( token != NULL ) {
                unsigned short int db_port;
                int db_ret = parse_port ( token, & db_port );
                if ( db_ret == 0 ) {
                    db_config -> db_server_port = db_port;
                }
                else {
                    sprintf ( error_msg, "db port is invalid %s" , token );
                    return -3;
                }
            }
        }
        /* db name */
        else if ( strcmp ( token, CF_STR_DB_NAME ) == 0 ) {
            token = strtok_r ( NULL, delim, & saveptr );
            if ( token != NULL ) {
                strcpy ( db_config -> db_name, token );
            }
        }
        /* db user name */
        else if ( strcmp ( token, CF_STR_DB_USER_NAME ) == 0 ) {
            token = strtok_r ( NULL, delim, & saveptr );
            if ( token != NULL ) {
                strcpy ( db_config -> db_username, token );
            }
        }
        /* db password */
        else if ( strcmp ( token, CF_STR_DB_PASSWORD ) == 0 ) {
            token = strtok_r ( NULL, delim, & saveptr );
            if ( token != NULL ) {
                strcpy ( db_config -> db_password, token );
            }
        }
    }

    /* check db config */
    if ( db_config -> db_server_ip [ 0 ] == '\0' ) {
        sprintf ( error_msg, "db server ip is invalid" );
        return -4;
    }
    if ( db_config -> db_server_port == 0 ) {
        db_config -> db_server_port = DEFAULT_DB_PORT;
        /* sprintf ( error_msg, "db server port is invalid" );
        return -5; */
    }
    if ( db_config -> db_name [ 0 ] == '\0' ) {
        sprintf ( error_msg, "db name is invalid" );
        return -6;
    }
    else if ( db_config -> db_username [ 0 ] == '\0' ) {
        sprintf ( error_msg, "db user name is invalid" );
        return -7;
    }
    else if ( db_config -> db_password [ 0 ] == '\0' ) {
        sprintf ( error_msg, "db user password is invalid" );
        return -8;
    }
 
    return 0;
}


/*
 * Function to parse sms config 
 * @sms_config:      sms config
 * @str:            pointer to config string
 * @error_msg pointer to error message
 * RET 0 or 1 on success, otherwise return a negative number
 */

static int parse_cf_sms_config ( sms_config_t * sms_config, \
        char * str, char * error_msg ) {

    if ( sms_config == NULL || str == NULL || error_msg == NULL || str [ 0 ] == '\0' ) {
        return -1;
    }

    char * delim = " =\t";
    char * saveptr;
    char * token;

    int sms_no_ix = 0;
    for ( sms_no_ix = 0; sms_no_ix < MAX_SMS_STR_NUM; str = NULL, sms_no_ix ++ ) {
        /* sms num */
        token = strtok_r ( str, delim, & saveptr );
        if ( token == NULL ) {
            break;
        }
        /* too long */
        if ( strlen ( token ) > MAX_SMS_STR_LEN ) {
            return -2;
        }
        /* check format */
        int sret = check_sms_num_format ( token , strlen ( token ) );
        if ( sret < 0 ) {
            sprintf ( error_msg, "invalid sms num format, %s", token );
            return -3;
        }
        strcpy ( sms_config -> sms_num [ sms_no_ix ], token );
    }

    /* check sms config */
    if ( sms_config -> sms_num [ 0 ] [ 0 ] == '\0' ) {
        sprintf ( error_msg, "sms config error no sms" );
        return -4;
    }

    return 0;
}


/*
 * Function to parse tel config 
 * @sms_config:      tel config
 * @str:            pointer to config string
 * @error_msg pointer to error message
 * RET 0 or 1 on success, otherwise return a negative number
 */

static int parse_cf_tel_config ( tel_config_t * tel_config, \
        char * str, char * error_msg ) {

    if ( tel_config == NULL || str == NULL || error_msg == NULL || str [ 0 ] == '\0' ) {
        return -1;
    }

    char * delim = " =\t";
    char * saveptr;
    char * token;

    int tel_no_ix = 0;
    for ( tel_no_ix = 0; tel_no_ix < MAX_SMS_STR_NUM; str = NULL, tel_no_ix ++ ) {
        /* tel num */
        token = strtok_r ( str, delim, & saveptr );
        if ( token == NULL ) {
            break;
        }
        /* too long */
        if ( strlen ( token ) > MAX_SMS_STR_LEN ) {
            return -2;
        }
        /* check format */
        int sret = check_sms_num_format ( token , strlen ( token ) );
        if ( sret < 0 ) {
            sprintf ( error_msg, "invalid sms num format, %s", token );
            return -3;
        }
        strcpy ( tel_config -> tel_num [ tel_no_ix ], token );
    }

    /* check tel config */
    if ( tel_config -> tel_num [ 0 ] [ 0 ] == '\0' ) {
        sprintf ( error_msg, "tel config error no tel" );
        return -4;
    }

    return 0;
}








/*
 * Function to parse email config 
 * @email_config:      email config
 * @str:            pointer to config string
 * @error_msg pointer to error message
 * RET 0 or 1 on success, otherwise return a negative number
 */

static int parse_cf_email_config ( email_config_t * email_config, \
        char * str, char * error_msg ) {

    if ( email_config == NULL || str == NULL || error_msg == NULL || str [ 0 ] == '\0' ) {
        return -1;
    }

    char * delim = " =\t";
    char * saveptr;
    char * token;

    for ( ; ; str = NULL ) {
        token = strtok_r ( str, delim, & saveptr );
        if ( token == NULL ) {
            break;
        }
        /* smtp server */
        else if ( strcmp ( token, CF_STR_EMAIL_SMTP ) == 0 ) {
            token = strtok_r ( NULL, delim, & saveptr );
            if ( token == NULL ) {
                sprintf ( error_msg, "invalid email config no smtp" );
                return -2;
            }
            else if ( strlen ( token ) + 1 > sizeof ( email_config -> smtp_server ) ) {
                sprintf ( error_msg, "invalid email config smtp too long %s", token );
                return -3;
            }
            strcpy ( email_config -> smtp_server, token );
        }
        /* from */
        else if ( strcmp ( token, CF_STR_EMAIL_FROM ) == 0 ) {
            token = strtok_r ( NULL, delim, & saveptr );
            if ( token == NULL ) {
                sprintf ( error_msg, "invalid email config no from" );
                return -4;
            }
            else if ( strlen ( token ) + 1 > sizeof ( email_config -> from ) ) {
                sprintf ( error_msg, "invalid email config from too long %s", token );
                return -5;
            }
            int eret = check_email_addr_format ( token, strlen ( token ) );
            if ( eret < 0 ) {
                sprintf ( error_msg, "invalid email format, %s", token );
                return - ( 5 + 0x0f00 );
            }
            strcpy ( email_config -> from, token );
        }
        /* user name */
        else if ( strcmp ( token, CF_STR_EMAIL_NAME ) == 0 ) {
            token = strtok_r ( NULL, delim, & saveptr );
            if ( token == NULL ) {
                sprintf ( error_msg, "invalid email config no name" );
                return -4;
            }
            else if ( strlen ( token ) + 1 > sizeof ( email_config -> name) ) {
                sprintf ( error_msg, "invalid email config name too long %s", token );
                return -5;
            }
            strcpy ( email_config -> name, token );
        }
        /* password */
        else if ( strcmp ( token, CF_STR_EMAIL_PASSWORD ) == 0 ) {
            token = strtok_r ( NULL, delim, & saveptr );
            if ( token == NULL ) {
                sprintf ( error_msg, "invalid email config no paaaword" );
                return -6;
            }
            else if ( strlen ( token ) + 1 > sizeof ( email_config -> password ) ) {
                sprintf ( error_msg, "invalid email config password too long %s", token );
                return -7;
            }
            strcpy ( email_config -> password, token );
        }
        /* to */
        else if ( strcmp ( token, CF_STR_EMAIL_TO ) == 0 ) {
            token = strtok_r ( NULL, delim, & saveptr );
            if ( token == NULL ) { 
                sprintf ( error_msg, "invalid email config no to" );
                return -8;
            }
	    char * email_to_delim = " ;";
	    char * email_to_saveptr;
	    char * email_to_token;
	    char * email_to_str = token;
	    int email_to_ix = 0;
            /* every to */
	    for ( email_to_ix = 0; email_to_ix < MAX_EMAIL_TO_STR_NUM; 
		    email_to_str = NULL, email_to_ix ++ ) {
		email_to_token = strtok_r ( email_to_str, email_to_delim, & email_to_saveptr );
		if ( email_to_token == NULL ) {
		    break;
		}
		if ( strlen ( email_to_token ) + 1 > \
                        sizeof ( email_config -> to [ email_to_ix ] ) ) {
		    sprintf ( error_msg, "invalid email config to too long %s", token );
		    return -9;
		}
                /* check addr format */
                int eret = check_email_addr_format ( email_to_token, strlen ( email_to_token ) );
                if ( eret < 0 ) {
                    sprintf ( error_msg, "invalid email format, %s", email_to_token );
                    return - ( 9 + 0x0f00 );
                }
		strcpy ( email_config -> to [ email_to_ix ], email_to_token );
	    }
        }
        /* cc */
        else if ( strcmp ( token, CF_STR_EMAIL_CC ) == 0 ) {
            token = strtok_r ( NULL, delim, & saveptr );
            if ( token == NULL ) { 
                sprintf ( error_msg, "invalid email config no cc" );
                return -10;
            }
	    char * email_cc_delim = " ;";
	    char * email_cc_saveptr;
	    char * email_cc_token;
	    char * email_cc_str = token;
	    int email_cc_ix = 0;
	    for ( email_cc_ix = 0; email_cc_ix < MAX_EMAIL_CC_STR_NUM; 
		    email_cc_str = NULL, email_cc_ix ++ ) {
		email_cc_token = strtok_r ( email_cc_str, email_cc_delim, & email_cc_saveptr );
		if ( email_cc_token == NULL ) {
		    break;
		}
                /* too long */
		if ( strlen ( email_cc_token ) + 1 > \
                        sizeof ( email_config -> cc [ email_cc_ix ] ) ) {
		    sprintf ( error_msg, "invalid email config cc too long %s", token );
		    return -11;
		}
                /* check format */
                int eret = check_email_addr_format ( email_cc_token, strlen ( email_cc_token ) );
                if ( eret < 0 ) {
                    sprintf ( error_msg, "invalid email format, %s", email_cc_token );
                    return - ( 9 + 0x0f00 );
                }
		strcpy ( email_config -> cc [ email_cc_ix ], email_cc_token );
	    }
        }
        /* smtp port */
        else if ( strcmp ( token, CF_STR_EMAIL_SMTP_PORT ) == 0 ) {
            token = strtok_r ( NULL, delim, & saveptr );
            if ( token == NULL ) {
                sprintf ( error_msg, "invalid email config no smtp port" );
                return -12;
            }
            else {
                unsigned short int smtp_port;
                int smtp_ret = parse_port ( token, & smtp_port );
                if ( smtp_ret == 0 ) {
                    email_config -> smtp_port = smtp_port;
                }
                else {
                    sprintf ( error_msg, "smtp port is invalid %s" , token );
                    return -13;
                }
            }
        }
        /* use tls ssl */
        else if ( strcmp ( token, CF_STR_EMAIL_USE_TLS_SSL ) == 0 ) {
            token = strtok_r ( NULL, delim, & saveptr );
            if ( token == NULL ) {
                sprintf ( error_msg, "invalid email config no tls ssl value" );
                return -15;
            }
            else {
                long long int use_tls_ssl;
                int smtp_ret = parse_long_long_int ( token, & use_tls_ssl );
                if ( smtp_ret == 0 ) {
                    if ( use_tls_ssl == 0 || use_tls_ssl == 1 ) {
                        email_config -> use_tls_ssl = ( int ) use_tls_ssl;
                    }
                    else {
                        sprintf ( error_msg, "smtp use tls ssl value is invalid %s" , token );
                        return -16;
                    }
                }
                else {
                    sprintf ( error_msg, "smtp use tls ssl value is invalid %s" , token );
                    return -17;
                }
            }
        }

    }

    /* check email config */
    if ( email_config -> smtp_server [ 0 ] == '\0' || \
            email_config -> from [ 0 ] == '\0' ||\
            email_config -> password  [ 0 ] == '\0' ||\
            email_config -> to [ 0 ] [ 0 ] == '\0' ) {
        sprintf ( error_msg, "invalid email config" );
        return -14;
    }
 
    return 0;
}


/*
 * Function to parse alarm freq
 * @alarm_freq:      alarm freq config
 * @str:            pointer to config string
 * @error_msg pointer to error message
 * RET 0 or 1 on success, otherwise return a negative number
 */

static int parse_cf_alarm_freq ( alarm_freq_t * alarm_freq, \
        char * str, char * error_msg ) {

    if ( alarm_freq == NULL || str == NULL || error_msg == NULL || str [ 0 ] == '\0' ) {
        return -1;
    }

    char * delim = " =\t";
    char * saveptr;
    char * token;
    int ret;

    for ( ; ; str = NULL ) {
        token = strtok_r ( str, delim, & saveptr );
        if ( token == NULL ) {
            break;
        }
        /* alarm freq interval */
        else if ( strcmp ( token, CF_STR_ALARM_FREQ_INTERVAL ) == 0 ) {
            token = strtok_r ( NULL, delim, & saveptr );
            if ( token != NULL ) {
                long long int interval;
                ret = parse_long_long_int ( token, & interval );
                if ( ret < 0 ) {
                    sprintf ( error_msg, "alarm freq is invalid" );
                    return -2;
                }
                alarm_freq -> interval = ( int ) interval;
            }
        }
        /* alarm freq times */
        else if ( strcmp ( token, CF_STR_ALARM_FREQ_TIMES ) == 0 ) {
            token = strtok_r ( NULL, delim, & saveptr );
            if ( token != NULL ) {
                long long int times;
                ret = parse_long_long_int ( token, & times );
                if ( ret < 0 ) {
                    sprintf ( error_msg, "alarm freq is invalid" );
                    return -3;
                }
                alarm_freq -> times = ( int ) times;
            }
        }
    }

    /* check alarm freq config */
    if ( alarm_freq -> times <= 0 || alarm_freq -> interval <= 0 ) {
        sprintf ( error_msg, "alarm freq is invalid" );
        return -4;
    }

    return 0;
}


/*
 * Function to init alarm items
 * @alarm_items:    alarm items
 * RET 0 or 1 on success, otherwise return a negative number
 */

int init_alarm_items ( alarm_items_t * alarm_items ) {
    if ( alarm_items == NULL ) {
        return -1;
    }
    bzero ( alarm_items, sizeof ( alarm_items_t ) );
    return 0;
}


#define RET_ITEM(i) do { \
    free ( alarm_item ); \
    return i; \
}while(0)


/*
 * Function to parse alarm items
 * @alarm_items:    alarm items
 * @str:            pointer to config string
 * @error_msg pointer to error message
 * RET 0 or 1 on success, otherwise return a negative number
 */

static int parse_cf_alarm_item ( alarm_items_t * alarm_items, \
        char * str, char * error_msg ) {

    if ( alarm_items == NULL || str == NULL || error_msg == NULL || str [ 0 ] == '\0' ) {
        return -1;
    }

    int ret = 0;
    char * delim = " \t";
    char * saveptr;
    char * token;

    alarm_item_t * alarm_item = ( alarm_item_t * ) malloc ( sizeof ( alarm_item_t ) );
    if ( alarm_item == NULL ) {
        return -2;
    }
    bzero ( alarm_item, sizeof ( alarm_item_t ) );

    /* item name */
    token = strtok_r ( str, delim, & saveptr );
    if ( token == NULL || token [ 0 ] == '(' ) {
        sprintf ( error_msg, "alarm item is invalid, no item name" );
        RET_ITEM ( -3 );
    }
    else if ( strlen ( token ) + 1 > sizeof ( alarm_item -> alarm_item_name ) ) {
        sprintf ( error_msg, "alarm item is too long %s", token );
        RET_ITEM ( -4 );
    }
    else {
        strcpy ( alarm_item -> alarm_item_name, token );
    }


    /* ( ) */
    str = saveptr;
    trim ( str, strlen ( str ) );
    delim = "()";
    token = strtok_r ( str, delim, & saveptr );
    char *saveptr_1;
    /*alarm_node*/    
    if ( token == NULL ) {
        sprintf ( error_msg, "alarm item %s is invalid", \
                alarm_item -> alarm_item_node_head.alarm_item_node_name );
        RET_ITEM ( -5 );
    }
    else {
	alarm_item_node_t * p = & ( alarm_item->alarm_item_node_head );
	token = strtok_r(token, ",", &saveptr_1);
	while ( token != NULL ) {
		alarm_item_node_t * alarm_item_node = ( alarm_item_node_t * ) malloc ( sizeof ( alarm_item_node_t ) );
		memset(alarm_item_node, 0, sizeof(alarm_item_node_t));
		strcpy(alarm_item_node->alarm_item_node_name, token);
//		printf("-----------------------------------------%s\n", alarm_item_node -> alarm_item_node_name);
		token = strtok_r(NULL, ",", &saveptr_1);
		p -> next = alarm_item_node;
		alarm_item_node->next = NULL;
		p = alarm_item_node;
		
	}
    }

    /* alarm_name */
    trim ( saveptr, strlen ( saveptr ) );
    token = strtok_r ( saveptr, delim, & saveptr );
    if ( token == NULL ) {
        sprintf ( error_msg, "alarm item %s is invalid", \
                alarm_item -> alarm_item_name );
        RET_ITEM ( -5 );
    }
    else {
        /* alarm condition */
        ret = parse_alarm_condition ( \
                & ( alarm_item -> alarm_condition [ ALARM_LEVEL_SERIOUS ] ), \
                token, error_msg );
        if ( ret < 0 ) {
            RET_ITEM ( -6 );
        }
    }

    token = strtok_r ( NULL, delim, & saveptr );
    if ( token == NULL ) {
        sprintf ( error_msg, "alarm item %s is invalid", \
                alarm_item -> alarm_item_name );
        RET_ITEM ( -7 );
    }
    else {
        ret = parse_alarm_condition ( \
                & ( alarm_item -> alarm_condition [ ALARM_LEVEL_COMMON ] ), \
                token, error_msg );
        if ( ret < 0 ) {
            RET_ITEM ( -8 );
        }
    }

    token = strtok_r ( NULL, delim, & saveptr );
    if ( token == NULL ) {
        sprintf ( error_msg, "alarm item %s is invalid", \
                alarm_item -> alarm_item_name );
        RET_ITEM ( -9 );
    }
    else {
        ret = parse_alarm_condition ( \
                & ( alarm_item -> alarm_condition [ ALARM_LEVEL_WARN ] ), \
                token, error_msg );
        if ( ret < 0 ) {
            RET_ITEM ( -10 );
        }
    }



    token = strtok_r( NULL, ")", & saveptr);
    if ( !strcmp(token, "(") ) {
	bzero (alarm_item -> alarm_script, strlen( token ));
    }
    else {
	strcpy ( alarm_item -> alarm_script, token + 1);
    }

//    if ( saveptr [ 0 ] == '{' && saveptr [ strlen ( saveptr ) - 1 ] == '}' ) {
//        strncpy ( alarm_item -> alarm_script, saveptr + 1, strlen ( saveptr + 1 ) - 1 );
//        alarm_item -> alarm_script [ strlen ( saveptr + 1 ) - 1 ] = 0;
//    }

    /* check whether repeated */
    alarm_item_t * p = & ( alarm_items -> alarm_item_head );
    while ( p -> next ) {
        if ( strcmp ( p -> next -> alarm_item_name, alarm_item -> alarm_item_name ) == 0 ) {
            sprintf ( error_msg, "alarm item %s is repeated in config file", \
                    p -> next -> alarm_item_name );
            RET_ITEM ( -11 );
        }
        p = p -> next;
    }
    p -> next = alarm_item;
    alarm_items -> alarm_item_count ++;

    ret = check_alarm_item_conditon ( alarm_item );
    if ( ret < 0 ) {
        sprintf ( error_msg, "alarm condition for item %s is invalid", alarm_item -> alarm_item_name );
        RET_ITEM ( -12 );
    }

    return 0;
}

/*
 * Function to parse alarm condition 
 * @alarm_condition:    alarm condition 
 * @str:            pointer to config string
 * @error_msg pointer to error message
 * RET 0 or 1 on success, otherwise return a negative number
 */

static int parse_alarm_condition ( alarm_condition_t * alarm_condition, \
        char * str, char * error_msg ) {

    if ( alarm_condition == NULL || str == NULL || error_msg == NULL || str [ 0 ] == '\0' ) {
        return -1;
    }

    int len = strlen ( str );
    char * new_str = malloc ( len + 1 );
    if ( new_str == NULL ) {
        return -8;
    }
    strcpy ( new_str, str );

    int ret = 0;
    char * delim = ",";
    char * saveptr;
    char * token;
    long long int ll_value;
    float f_value;

    /* alarm condition */
    token = strtok_r ( new_str, delim, & saveptr );
    if ( token == NULL ) {
        sprintf ( error_msg, "alarm condition is invalid no condition" );
        return -2;
    }
    else {
        /* time */
        ret = parse_long_long_int ( token, & ll_value );
        if ( ret < 0 ) {
            sprintf ( error_msg, "alarm condition is invalid %s", token );
            return -3;
        }
        if ( ll_value < 1 ) {
            sprintf ( error_msg, "alarm condition is invalid %s", token );
            return -8;
        }
        alarm_condition -> secs = ( int ) ll_value;
    }
    token = strtok_r ( NULL, delim, & saveptr );
    if ( token == NULL ) {
        sprintf ( error_msg, "alarm condition is invalid" );
        return -4;
    }
    else {
        /* value */
        ret = parse_float ( token, & f_value );
        if ( ret < 0 ) {
            sprintf ( error_msg, "alarm condition is invalid %s", token );
            return -5;
        }
        alarm_condition -> value = f_value;
//	printf("file = %sline =%d value =%f\n", __FILE__, __LINE__, f_value);
    }
    token = strtok_r ( NULL, delim, & saveptr );
    if ( token == NULL ) {
        sprintf ( error_msg, "alarm condition is invalid" );
        return -6;
    }
    else {
        /* relation */
        ret = parse_alarm_condition_relation ( & ( alarm_condition -> alarm_relation ), token );
        if ( ret < 0 ) {
            sprintf ( error_msg, "alarm condition relation is invalid %s", token );
            return -7;
        }
    }

    free ( new_str );

    return 0;

}


/*
 * Function to parse relation of alarm condition 
 * @alarm_relation:    relation of alarm condition
 * @c:            character > < = 
 * RET 0 or 1 on success, otherwise return a negative number
 */

static int parse_alarm_condition_relation ( alarm_relation_t * alarm_relation, char * str ) {
    if ( alarm_relation == NULL || str == NULL ) {
        return -1;
    }
    const int len = strlen ( str );
    if ( len == 1 ) {
        switch ( * str ) {
            case '>': return * alarm_relation = GT;
            case '<': return * alarm_relation = LT;
            case '=': return * alarm_relation = EQ;
            default: return -2;
        }
    }
    else if ( len == 2 ) {
        if ( str [ 1 ] != '=' ) {
            return -3;
        }
        switch ( * str ) {
            case '>': return * alarm_relation = GE;
            case '<': return * alarm_relation = LE;
            case '!': return * alarm_relation = NQ;
            default: return -4;
        }
    }
    else {
        return -5;
    }
    return 0;
}


/*
 * Function to destroy alarm items
 * @alarm_items:   pointer to alarm items 
 * RET 0 or 1 on success, otherwise return a negative number
 */

int destroy_alarm_items ( alarm_items_t * alarm_items ) {
    if ( alarm_items == NULL ) {
        return -1;
    }
    alarm_item_t * p = & ( alarm_items -> alarm_item_head );
    alarm_item_t * q = NULL;
    while ( p -> next ) {
        q = p -> next;
        p -> next = q -> next;
        free ( q );
        alarm_items -> alarm_item_count --;
    }
    return 0;
}


/*
 * Function to init config
 * @config_file_path:   path of config file
 * @ex_alarm_config::   pointer to alarm config 
 * @alarm_items: pointer to alarm items
 * @error_msg: pointer to error message
 * RET 0 or 1 on success, otherwise return a negative number
 */

int init_config ( char * config_file_path, ex_alarm_config_t * ex_alarm_config, \
        alarm_items_t * alarm_items, char * error_msg ) {

    if ( config_file_path == NULL || ex_alarm_config == NULL || \
            alarm_items == NULL || config_file_path [ 0 ] == '\0' ) {
        cs_log ( LOG_ERROR, "invalid parameters" );
        return -1;
    }
    int ret = 0;
    /* read config from file */
    ret = read_config_from_file ( config_file_path, ex_alarm_config, alarm_items, error_msg );
    if ( ret < 0 ) {
        return -2;
    }

    /* db config */
    db_config_t * db_config = & ( ex_alarm_config -> db_config );

    /* check db config */
    if ( db_config -> db_server_ip [ 0 ] == '\0' ) {
        sprintf ( error_msg, "no db config" );
        return -3;
    }

    /* check alarm freq config */
    if ( ex_alarm_config -> alarm_freq.interval == 0 || \
            ex_alarm_config -> alarm_freq.times == 0  ) {
        sprintf ( error_msg, "no alarm freq config" );
        return -4;
    }

    /* check alarm email config */
    if ( ex_alarm_config -> alarm_method.email == METHOD_ON && \
            ex_alarm_config -> email_config.smtp_server [ 0 ] == '\0' ) {
        sprintf ( error_msg, "no email config" );
        return -5;
    }

    /* check alarm sms config */
    if ( ex_alarm_config -> alarm_method.sms == METHOD_ON && \
            ex_alarm_config -> sms_config.sms_num [ 0 ] [ 0 ] == '\0' ) {
        sprintf ( error_msg, "no sms config" );
        return -6;
    }

    /* check alarm items config */
    if ( alarm_items -> alarm_item_count == 0 ) {
        sprintf ( error_msg, "no alarm items" );
        return -7;
    }

    return 0;
}


/*
 * Function to check email address format
 * @email_addr:   email address string
 * @email_addr_len::   length of email address
 * RET 0 or 1 on success, otherwise return a negative number
 */

static int check_email_addr_format ( char * email_addr, int email_addr_len ) {
    if ( email_addr == NULL || email_addr [ 0 ] == '\0' || email_addr_len < 3 ) {
        return -1;
    }
    char * chr = strchr ( email_addr, ( int ) '@' );
    if ( chr == NULL || chr == email_addr || chr == email_addr + email_addr_len - 1 ) {
        return -2;
    }
    return 0;
}


/*
 * Function to check sms num format
 * @sms_num:   sms num string
 * @sms_num_len::   length of sms num
 * RET 0 or 1 on success, otherwise return a negative number
 */

static int check_sms_num_format ( char * sms_num, int sms_num_len ) {
    if ( sms_num == NULL || sms_num [ 0 ] == '\0' || sms_num_len < 1 ) {
        return -1;
    }
    int z = -1;
    if ( sms_num [ 0 ] == '+' ) {
        z = 0;
    }
    while ( ++ z < sms_num_len ) {
        if ( ! isdigit ( ( int ) ( * ( sms_num + z ) ) ) ) {
            return -2;
        }
    }
    return 0;
}

static int check_alarm_item_conditon ( const alarm_item_t * alarm_item ) {
    if ( alarm_item == NULL ) {
        return -1;
    }
    const alarm_condition_t * alarm_condition = alarm_item -> alarm_condition;
    alarm_relation_t a = alarm_condition [ ALARM_LEVEL_SERIOUS ].alarm_relation;
    alarm_relation_t b = alarm_condition [ ALARM_LEVEL_COMMON ].alarm_relation;
    alarm_relation_t c = alarm_condition [ ALARM_LEVEL_WARN ].alarm_relation;
    float d = alarm_condition [ ALARM_LEVEL_SERIOUS ].value;
    float e = alarm_condition [ ALARM_LEVEL_COMMON ].value;
    float f = alarm_condition [ ALARM_LEVEL_WARN ].value;
    if ( ( a == GT || a == GE ) && ( b == GT || b == GE ) && ( c == GT || c == GE ) ) {
        if ( ! ( d >= e && e >= f ) ) {
            return -2;
        }
    }
    else if ( ( a == LT || a == LE ) && ( b == LT || b == LE ) && ( c == LT || c == LE ) ) {
        if ( ! ( d <= e && e <= f ) ) {
            return -3;
        }
    }
    return 0;
}

/*end of file*/

