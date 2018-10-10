/*
 *
 * c_config.c
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "e_define.h"
#include "c_define.h"
#include "c_config.h"
#include "c_debug_log.h"
#include "u_util.h"

#define CS_LOG_PATH CLR_LOG_SCT_PATH
#define CF_STR_CLRD_IP "collectord_ip"
#define CF_STR_CLRD_PORT "collectord_port"
#define CF_STR_CL_INTERVAL "collect_interval"
#define CF_STR_CMD_PORT "collector_port"
#define CF_STR_CMD_TIMEOUT "cmd_timeout"
#define CF_STR_LOG_LEVEL "log_level"

clr_sct_conf_t g_sct_conf;
clr_cmd_conf_t g_cmd_conf;

static short int parse_clr_interval ( char * str );
static int parse_clr_config_line ( char * config_line, 
        clr_sct_conf_t * clr_sct_conf, clr_cmd_conf_t * clr_cmd_conf );
static short int parse_cmd_timeout ( char * str );


int clr_read_config_from_file ( char * clr_config_file_path, 
        clr_sct_conf_t * clr_sct_conf, clr_cmd_conf_t * clr_cmd_conf ) {

    if ( clr_config_file_path == NULL || clr_sct_conf == NULL ||
            clr_cmd_conf == NULL || clr_config_file_path [ 0 ] == '\0' ) {
        cs_log ( LOG_ERROR, "Invalid parameters" );
        return -1;
    }

    int ret = -1;
    int str_len = 0;
    FILE * config_file;
    const int padding_len = 1 + 5;
    char line_buffer [ MAX_CONFIG_FILE_LINE_LEN + padding_len ];
    char errmsg [ CLR_ERRMSG_LEN ];

    bzero ( clr_sct_conf, sizeof ( clr_sct_conf_t ) );
    bzero ( clr_cmd_conf, sizeof ( clr_cmd_conf_t ) );

    config_file = fopen ( clr_config_file_path, "r" );
    if ( config_file == NULL ) {
        strerror_r ( errno, errmsg, CLR_ERRMSG_LEN );
        if ( errno == ENOENT ) {
            cs_log ( LOG_ERROR, "Open config file %s error: file not exsit, %s",
                    clr_config_file_path, errmsg );
            return -2; /* file not exist */
        }
        else {
            cs_log ( LOG_ERROR, "Open config file %s error: %d | %s", 
                    clr_config_file_path, errno, errmsg );
            return -3; /* open file error */
        }
    }

    while ( fgets ( line_buffer, MAX_CONFIG_FILE_LINE_LEN, config_file ) ) {

        str_len = strlen ( line_buffer );

        if ( str_len > 0 && line_buffer [ str_len - 1 ] == '\n' ) {
            line_buffer [ str_len - 1 ] = 0;
            str_len --;
        }
        if ( str_len > 1 && line_buffer [ str_len - 2 ] == '\r' ) {
            line_buffer [ str_len - 2 ] = 0;
            str_len --;
        }

#define clr_rcf_return(retno) \
        do { \
            fclose(config_file);return retno; \
        } while ( 0 )

        if ( str_len > MAX_CONFIG_FILE_LINE_LEN ) { 
            cs_log ( LOG_ERROR, "Line too long" );
            clr_rcf_return ( -4 ); /* line too long */
        }

        char * line_buffer_trim_head = trim_head ( line_buffer, str_len );

        if ( * line_buffer_trim_head == '#' ) {
            continue;
        }
        if ( * line_buffer_trim_head == 0 ) {
            continue;
        }

        ret = parse_clr_config_line ( line_buffer_trim_head, clr_sct_conf, clr_cmd_conf );
        switch ( ret ) {
            case -1:
                cs_log ( LOG_ERROR, "Parse clr config: invalid parameters" );
                clr_rcf_return ( -5 );
            case -2:
                cs_log ( LOG_ERROR, "No item" );
                clr_rcf_return ( -6 );
            case -3:
                cs_log ( LOG_ERROR, "Item too long" );
                clr_rcf_return ( -7 );
            case -4:
                cs_log ( LOG_ERROR, "No value" );
                clr_rcf_return ( -8 );
            case -5:
                cs_log ( LOG_ERROR, "Too many value" );
                clr_rcf_return ( -9 );
            case -6:
                cs_log ( LOG_ERROR, "Value too long" );
                clr_rcf_return ( -10 );
            case -7:
                cs_log ( LOG_ERROR, "IP str is invalid" );
                clr_rcf_return ( -11 );
            case -8:
                cs_log ( LOG_ERROR, "IP value length more than sotre size" );
                clr_rcf_return ( -12 );
            case -9:
                cs_log ( LOG_ERROR, "Clrd port value is invalid" );
                clr_rcf_return ( -13 );
            case -10:
                cs_log ( LOG_ERROR, "Clr sct interval value is invlaid" );
                clr_rcf_return ( -14 );
            case -11:
                cs_log ( LOG_ERROR, "Clr sct interval value is underflow" );
                clr_rcf_return ( -15 );
            case -12:
                cs_log ( LOG_ERROR, "Clr sct interval value is overflow" );
                clr_rcf_return ( -16 );
            case -16:
                cs_log ( LOG_ERROR, "Clr cmd port value is invlaid" );
                clr_rcf_return ( -20 );
            case -17:
                cs_log ( LOG_ERROR, "Clr cmd time out value is invalid" );
                clr_rcf_return ( -21 );
            case -18:
                cs_log ( LOG_ERROR, "Clr cmd time out value is underflow" );
                clr_rcf_return ( -22 );
            case -19:
                cs_log ( LOG_ERROR, "Clr cmd time out value is overflow" );
                clr_rcf_return ( -23 );
            case -20:
                cs_log ( LOG_ERROR, "Unknown config item" );
                clr_rcf_return ( -24 );
        }

    }

    fclose ( config_file );

    return 0;

}


static int parse_clr_config_line ( char * config_line, 
        clr_sct_conf_t * clr_sct_conf, clr_cmd_conf_t * clr_cmd_conf ) {

    if ( config_line == NULL || clr_sct_conf == NULL || 
            clr_cmd_conf == NULL || config_line [ 0 ] == '\0' ) {
        cs_log ( LOG_ERROR, "Invalid parameters" );
        return -1;
    }

    char config_item [ MAX_CONFIG_ITEM_LEN + 1 ];
    char config_value [ MAX_CONFIG_VALUE_LEN + 1 ];
    char * delim = " \t\r\n\f\v";
    char * saveptr;
    char * token;
    char * str = config_line;

    token = strtok_r ( str, delim, & saveptr );

    if ( token == NULL ) {
        cs_log ( LOG_ERROR, "No item" );
        return -2; /* no item */
    }

    if ( strlen ( token ) > MAX_CONFIG_ITEM_LEN ) {
        cs_log ( LOG_ERROR, "Item too long" );
        return -3; /* item too long */
    }

    strcpy ( config_item, token );

    token = strtok_r ( NULL, delim, & saveptr );

    if ( token == NULL ) {
        cs_log ( LOG_ERROR, "No value" );
        return -4; /* no value */
    }

    char * remain_str = strtok_r ( NULL, delim, & saveptr );

    if ( remain_str != NULL ) {
        cs_log ( LOG_ERROR, "Too many value" );
        return -5; /* too many value */
    }

    if ( strlen ( token ) > MAX_CONFIG_VALUE_LEN ) {
        cs_log ( LOG_ERROR, "Value too long" );
        return -6; /* value too long */
    }

    strcpy ( config_value, token );

    if ( strcmp ( config_item, CF_STR_CLRD_IP ) == 0 ) { /* clrd ip */

        if ( ! ip_str_is_valid ( config_value ) ) {
            cs_log ( LOG_ERROR, "IP str is invalid" );
            return -7; /* ip str is invalid */
        }
        if ( strlen ( config_value ) > sizeof ( clr_sct_conf -> collectord_ip ) - 1 ) {
            cs_log ( LOG_ERROR, "IP str length more than stroe size" );
            return -8; /* value length more than sotre size */
        }
        strcpy ( clr_sct_conf -> collectord_ip, config_value );
    }
    
    else if ( strcmp ( config_item, CF_STR_CLRD_PORT ) == 0 ) { /* clrd port */

        unsigned short int clrd_port;
        int clrd_ret = parse_port ( config_value, & clrd_port );
        if ( clrd_ret < 0 ) {
            cs_log ( LOG_ERROR, "Clrd port value is invlaid" );
            return -9; /* clrd port value is invalid */
        }
        clr_sct_conf -> collectord_port = clrd_port;
    }

    else if ( strcmp ( config_item, CF_STR_CL_INTERVAL ) == 0 ) { /* clrd interval */
        short int cl_interval = parse_clr_interval ( config_value );

        switch ( cl_interval ) {
            case -2:
                cs_log ( LOG_ERROR, "Clr sct interval value is invlaid" );
                return -10;
            case -3:
                cs_log ( LOG_ERROR, "Clr sct interval value underflow" );
                return -11;
            case -4:
                cs_log ( LOG_ERROR, "Clr sct interval value overflow" );
                return -12;
        }

        clr_sct_conf -> collect_interval = cl_interval;
    }

    else if ( strcmp ( config_item, CF_STR_CMD_PORT ) == 0 ) { /* clr cmd port */
        unsigned short int clr_cmd_port;
        int clr_ret = parse_port ( config_value, & clr_cmd_port );
        if ( clr_ret < 0 ) {
            cs_log ( LOG_ERROR, "Clr cmd port value is invlaid" );
            return -16; /* clr cmd port value is invalid */
        }
        clr_cmd_conf -> collecotr_cmd_port = clr_cmd_port;
    }

    else if ( strcmp ( config_item, CF_STR_CMD_TIMEOUT ) == 0 ) { /* clr cmd time out */
        short int cmd_timeout = parse_cmd_timeout ( config_value );
        switch ( cmd_timeout ) {
            case -2:
                cs_log ( LOG_ERROR, "Clr cmd timeout value invalid" );
                return -17;
            case -3:
                cs_log ( LOG_ERROR, "Clr cmd timeout value underflow" );
                return -18;
            case -4:
                cs_log ( LOG_ERROR, "Clr cmd timeout value overflow" );
                return -19;
        }

        clr_cmd_conf -> cmd_timeout = cmd_timeout;
    }

    else if ( strcmp ( config_item, CF_STR_LOG_LEVEL ) == 0 ) {
        if ( strcasecmp ( config_value, "DEBUG" ) == 0 ) {
            clr_sct_conf -> log_level = LOG_DEBUG;
            clr_cmd_conf -> log_level = LOG_DEBUG;
        }
        else if ( strcasecmp ( config_value, "INFO" ) == 0 ) {
            clr_sct_conf -> log_level = LOG_INFO;
            clr_cmd_conf -> log_level = LOG_INFO;
        }
        else if ( strcasecmp ( config_value, "WARN" ) == 0 ) {
            clr_sct_conf -> log_level = LOG_WARN;
            clr_cmd_conf -> log_level = LOG_WARN;
        }
        else if ( strcasecmp ( config_value, "ERROR" ) == 0 ) {
            clr_sct_conf -> log_level = LOG_ERROR;
            clr_cmd_conf -> log_level = LOG_ERROR;
        }
        else if ( strcasecmp ( config_value, "FATAL" ) == 0 ) {
            clr_sct_conf -> log_level = LOG_FATAL;
            clr_cmd_conf -> log_level = LOG_FATAL;
        }
    }


    else {
        cs_log ( LOG_ERROR, "Unknown config item" );
        return -20; /* unknown config item */
    }

    return 0;

}


int collector_config_need_update ( /* only clr sct interval is dynamic config able */
        clr_sct_conf_t * clr_sct_conf_a, clr_cmd_conf_t * clr_cmd_conf_a,
        clr_sct_conf_t * clr_sct_conf_b, clr_cmd_conf_t * clr_cmd_conf_b ) {

    if ( clr_sct_conf_a == NULL || clr_cmd_conf_a == NULL ||
         clr_sct_conf_b == NULL || clr_cmd_conf_b == NULL ) {
        return -1;
    }

    if ( clr_sct_conf_a -> collect_interval != clr_sct_conf_b -> collect_interval ) {
        cs_log ( LOG_DEBUG, "Clr config need update" );
        return 1; /* need update */
    }

    cs_log ( LOG_DEBUG, "Clr config need not update" );
    return 0; /* need not update */
}


int clr_script_config_update ( clr_sct_conf_t * clr_sct_conf_a, clr_sct_conf_t * clr_sct_conf_b ) {

    if ( clr_sct_conf_a == NULL || clr_sct_conf_b == NULL ) {
        return -1;
    }

    clr_sct_conf_a -> collect_interval = clr_sct_conf_b -> collect_interval;

    return 0;

}


int init_collector_config ( clr_sct_conf_t * clr_sct_conf, clr_cmd_conf_t * clr_cmd_conf, 
        char * error_msg ) {

    if ( clr_sct_conf == NULL || clr_cmd_conf == NULL ) {
        cs_log ( LOG_ERROR, "Invalid parameters" ); 
        return -1;
    }

    int ret;
    char errmsg [ CLR_ERRMSG_LEN ];
    clr_sct_conf_t stcf;
    clr_cmd_conf_t cdcf;

    bzero ( ( void * ) & stcf, sizeof ( clr_sct_conf_t ) );
    bzero ( ( void * ) & cdcf, sizeof ( clr_cmd_conf_t ) );

    stcf.log_level = LOG_DEBUG;
    cdcf.log_level = LOG_DEBUG;

    ret = clr_read_config_from_file ( CLR_CONFIG_FILE_PATH, & stcf, & cdcf );
    switch ( ret ) {
        case -1:
            cs_log ( LOG_ERROR, "Read config from file invalid parameters" ); 
            return -2;
        case -2:
            cs_log ( LOG_ERROR, "File %s not exist", CLR_CONFIG_FILE_PATH ); 
            sprintf ( error_msg, "File %s not exist", CLR_CONFIG_FILE_PATH );
            return -3;
        case -3:
            strerror_r ( errno, errmsg, CLR_ERRMSG_LEN );
            cs_log ( LOG_ERROR, "Open file %s error: %d | %s",
                    CLR_CONFIG_FILE_PATH, errno, errmsg ); 
            sprintf ( error_msg, "Open file %s error: %d | %s",
                    CLR_CONFIG_FILE_PATH, errno, errmsg ); 
            return -4;
        case -4:
            cs_log ( LOG_ERROR, "Line too long" ); 
            sprintf ( error_msg, "Line too long" ); 
            return -5;
        case -5:
            cs_log ( LOG_ERROR, "Parse clr config: invalid parameters" );
            sprintf ( error_msg, "Parse clr config: invalid parameters" );
            return -6;
        case -6:
            cs_log ( LOG_ERROR, "No item" );
            sprintf ( error_msg, "No item" );
            return -7;
        case -7:
            cs_log ( LOG_ERROR, "Item too long" );
            sprintf ( error_msg, "Item too long" );
            return -8;
        case -8:
            cs_log ( LOG_ERROR, "No value" );
            sprintf ( error_msg, "No value" );
            return -9;
        case -9:
            cs_log ( LOG_ERROR, "Too many value" );
            sprintf ( error_msg, "Too many value" );
            return -10;
        case -10:
            cs_log ( LOG_ERROR, "Value too long" );
            sprintf ( error_msg, "Value too long" );
            return -11;
        case -11:
            cs_log ( LOG_ERROR, "IP str is invalid" );
            sprintf ( error_msg, "IP str is invalid" );
            return -12;
        case -12:
            cs_log ( LOG_ERROR, "IP value length more than sotre size" );
            sprintf ( error_msg, "IP value length more than sotre size" );
            return -13;
        case -13:
            cs_log ( LOG_ERROR, "Clrd port value is invalid" );
            sprintf ( error_msg, "Clrd port value is invalid" );
            return -14;
        case -14:
            cs_log ( LOG_ERROR, "Clr sct interval value is invlaid" );
            sprintf ( error_msg, "Clr sct interval value is invlaid" );
            return -15;
        case -15:
            cs_log ( LOG_ERROR, "Clr sct interval value is underflow" );
            sprintf ( error_msg, "Clr sct interval value is underflow" );
            return -16;
        case -16:
            cs_log ( LOG_ERROR, "Clr sct interval value is overflow" );
            sprintf ( error_msg, "Clr sct interval value is overflow" );
            return -17;
        case -20:
            cs_log ( LOG_ERROR, "Clr cmd port value is invlaid" );
            return -21;
        case -21:
            cs_log ( LOG_ERROR, "Clr cmd time out value is invalid" );
            sprintf ( error_msg, "Clr cmd time out value is invalid" );
            return -22;
        case -22:
            cs_log ( LOG_ERROR, "Clr cmd time out value is underflow" );
            sprintf ( error_msg, "Clr cmd time out value is underflow" );
            return -23;
        case -23:
            cs_log ( LOG_ERROR, "Clr cmd time out value is overflow" );
            sprintf ( error_msg, "Clr cmd time out value is overflow" );
            return -24;
        case -24:
            cs_log ( LOG_ERROR, "Unknown config item" );
            sprintf ( error_msg, "Unknown config item" );
            return -25;
    }

    if ( stcf.collectord_ip [ 0 ] == '\0' ) {
        cs_log ( LOG_ERROR, "Config incomplete: no collectord IP" );
        sprintf ( error_msg, "Config incomplete: no collectord IP" );
        return -26;
    }
    if ( stcf.collectord_port == 0 ) {
        cs_log ( LOG_ERROR, "config incomplete: no collectord port" );
        sprintf ( error_msg, "Config incomplete: no collectord port" );
        return -27;
    }
    if ( stcf.collect_interval == 0 ) {
        cs_log ( LOG_ERROR, "Config incomplete: no collect interval" );
        sprintf ( error_msg, "Config incomplete: no collect interval" );
        return -28;
    }
    if ( cdcf.collecotr_cmd_port == 0 ) {
        cs_log ( LOG_ERROR, "Config incomplete: no cmd port" );
        sprintf ( error_msg, "Config incomplete: no cmd port" );
        return -30;
    }
    if ( cdcf.cmd_timeout == 0 ) {
        cs_log ( LOG_ERROR, "Config incomplete: no cmd timeout" );
        sprintf ( error_msg, "Config incomplete: no cmd timeout" );
        return -31;
    }

    memcpy ( ( void * ) clr_sct_conf, ( void * ) & stcf, sizeof ( clr_sct_conf_t ) );
    memcpy ( ( void * ) clr_cmd_conf, ( void * ) & cdcf, sizeof ( clr_cmd_conf_t ) );

    return 0;

}



int update_clr_sct_config ( clr_sct_conf_t * clr_sct_conf ) {

    if ( clr_sct_conf == NULL ) {
        return -1;
    }

    clr_sct_conf_t stcf;
    clr_cmd_conf_t cdcf; /* ignore */
    bzero ( & stcf, sizeof ( clr_sct_conf_t ) );

    int ret = 0;
    char errmsg [ CLR_ERRMSG_LEN ];

    ret = clr_read_config_from_file ( CLR_CONFIG_FILE_PATH, & stcf, & cdcf );
    switch ( ret ) {
        case -1:
            cs_log ( LOG_ERROR, "Read config from file error: invalid parameters" ); 
            return -2;
        case -2:
            cs_log ( LOG_ERROR, "File %s does not exist", CLR_CONFIG_FILE_PATH ); 
            return -3;
        case -3:
            strerror_r ( errno, errmsg, CLR_ERRMSG_LEN );
            cs_log ( LOG_ERROR, "Open file %s error: %d | %s",
                    CLR_CONFIG_FILE_PATH, errno, errmsg ); 
            return -4;
        case -4:
            cs_log ( LOG_ERROR, "Line too long" ); 
            return -5;
        case -5:
            cs_log ( LOG_ERROR, "Parse clr config: invalid parameters" );
            return -6;
        case -6:
            cs_log ( LOG_ERROR, "No item" );
            return -7;
        case -7:
            cs_log ( LOG_ERROR, "Item too long" );
            return -8;
        case -8:
            cs_log ( LOG_ERROR, "No value" );
            return -9;
        case -9:
            cs_log ( LOG_ERROR, "Too many value" );
            return -10;
        case -10:
            cs_log ( LOG_ERROR, "Value too long" );
            return -11;
        case -11:
            cs_log ( LOG_ERROR, "IP str is invalid" );
            return -12;
        case -12:
            cs_log ( LOG_ERROR, "IP value length more than sotre size" );
            return -13;
        case -13:
            cs_log ( LOG_ERROR, "Clrd port value is invalid" );
            return -14;
        case -14:
            cs_log ( LOG_ERROR, "Clr sct interval value is invlaid" );
            return -15;
        case -15:
            cs_log ( LOG_ERROR, "Clr sct interval value is underflow" );
            return -16;
        case -16:
            cs_log ( LOG_ERROR, "Clr sct interval value is overflow" );
            return -17;
        case -20:
            cs_log ( LOG_ERROR, "Clr cmd port value is invlaid" );
            return -21;
        case -21:
            cs_log ( LOG_ERROR, "Clr cmd time out value is invalid" );
            return -22;
        case -22:
            cs_log ( LOG_ERROR, "Clr cmd time out value is underflow" );
            return -23;
        case -23:
            cs_log ( LOG_ERROR, "Clr cmd time out value is overflow" );
            return -24;
        case -24:
            cs_log ( LOG_ERROR, "Unknown config item" );
            return -25;
    }

    if ( stcf.collectord_ip [ 0 ] == '\0' ) {
        cs_log ( LOG_ERROR, "Config incomplete: no collectord IP" );
        return -26;
    }
    if ( stcf.collectord_port == 0 ) {
        cs_log ( LOG_ERROR, "Config incomplete: no collectord port" );
        return -27;
    }
    if ( stcf.collect_interval == 0 ) {
        cs_log ( LOG_ERROR, "Config incomplete: no collect interval" );
        return -28;
    }
    if ( cdcf.collecotr_cmd_port == 0 ) {
        cs_log ( LOG_ERROR, "Config incomplete: no cmd port" );
        return -30;
    }
    if ( cdcf.cmd_timeout == 0 ) {
        cs_log ( LOG_ERROR, "Config incomplete: no cmd timeout" );
        return -31;
    }

    ret = collector_config_need_update ( & g_sct_conf, & g_cmd_conf, & stcf, & cdcf );
    if ( ret == -1 ) {
        return -32; /* invalid parameters */
    }

    if ( ret == 0 ) { /* need not update */
        cs_log ( LOG_DEBUG, "Sct config need not update" );
        return -33; 
    }

    clr_script_config_update ( & g_sct_conf, & stcf );

    return 0;

}


static inline short int parse_clr_interval ( char * str ) {

    if ( str == NULL || str [ 0 ] == '\0' ) {
        return -1;
    }
    int ret = -1;
    long int cl_interval;
    ret = parse_long_int ( str, & cl_interval );
    if ( ret < 0 ) {
        return -2; /* parse clr interval str error */
    }
    if ( cl_interval < MIN_CL_INTERVAL ) {
        return -3; /* sct interval range underflow */
    }
    if ( cl_interval > MAX_CL_INTERVAL ) {
        return -4; /* sct interval range overflow */
    }

    return ( short int ) cl_interval;
}

static inline short int parse_cmd_timeout ( char * str ) {

    if ( str == NULL || str [ 0 ] == '\0' ) {
        return -1;
    }
    int ret = -1;
    long int cmd_timeout;
    ret = parse_long_int ( str, & cmd_timeout );
    if ( ret < 0 ) {
        return -2; /* parse cmd timeout str error */
    }
    if ( cmd_timeout < MIN_CMD_TIMEOUT ) {
        return -3; /* cmd time out range underflow */
    }
    if ( cmd_timeout > MAX_CMD_TIMEOUT ) {
        return -4; /* cmd time out range overflow */
    }

    return ( short int ) cmd_timeout;
}




/*end of file*/
