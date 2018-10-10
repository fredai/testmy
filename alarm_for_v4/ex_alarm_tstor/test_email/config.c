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


#define CF_STR_EMAIL "email"
#define CF_STR_EMAIL_SMTP "smtp_server"
#define CF_STR_EMAIL_SMTP_PORT "smtp_port"
#define CF_STR_EMAIL_FROM "from"
#define CF_STR_EMAIL_NAME "name"
#define CF_STR_EMAIL_PASSWORD "password"
#define CF_STR_EMAIL_TO "to"
#define CF_STR_EMAIL_CC "cc"
#define CF_STR_EMAIL_USE_TLS_SSL "use_tls_ssl"



static int parse_cf_email_config ( email_config_t * email_config, char * value, char * error_msg );
static int read_config_from_file ( char * config_file_path, email_config_t * email_config, char * error_msg );
static int check_email_addr_format ( char * email_addr, int email_addr_len );


/*
 * Function to read config from file
 * @config_file_path:   path of config file
 * @ex_alarm_config::   pointer to alarm config 
 * @alarm_items: pointer to alarm items
 * @error_msg: pointer to error message
 * RET 0 or 1 on success, otherwise return a negative number
 */

static int read_config_from_file ( char * config_file_path, email_config_t * email_config, char * error_msg ) {


    int ret = -1;
    char errmsg [ ALARM_ERRMSG_LEN ];
    int str_len = 0;
    FILE * config_file;
#define padding_len 8
    /* line buffer */
    char line_buffer [ MAX_CONFIG_FILE_LINE_LEN + padding_len ];

    bzero ( email_config, sizeof ( email_config ) );

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
        ret = parse_alarm_config_line ( line_buffer_trim_head, email_config, error_msg );
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

int parse_alarm_config_line ( char * config_line, email_config_t * email_config, char * error_msg ) {

    if ( config_line == NULL || email_config == NULL) {
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

    /* email config */
    if ( strcmp ( config_item, CF_STR_EMAIL ) == 0 ) {
        ret = parse_cf_email_config ( email_config , saveptr, error_msg );
        if ( ret < 0 ) {
            return -7;
        }
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
//		printf("__FILE__=%s, __LINE__=%d, smtp_server=%s\n", __FILE__, __LINE__, email_config->smtp_server);
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
 * Function to init config
 * @config_file_path:   path of config file
 * @ex_alarm_config::   pointer to alarm config 
 * @alarm_items: pointer to alarm items
 * @error_msg: pointer to error message
 * RET 0 or 1 on success, otherwise return a negative number
 */

//int init_config ( char * config_file_path, ex_alarm_config_t * ex_alarm_config, \
//        alarm_items_t * alarm_items, char * error_msg ) {
int init_config ( char * config_file_path, email_config_t * email_config, char * error_msg ) {


    int ret = 0;
    /* read config from file */
    ret = read_config_from_file ( config_file_path, email_config,  error_msg );
    if ( ret < 0 ) {
        return -2;
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



/*end of file*/

