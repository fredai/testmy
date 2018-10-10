/*
 *
 * d_config.c
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include "d_config.h"
#include "d_nodes.h"
#include "u_util.h"
#include "u_mutex.h"
#include "u_log.h"

#define CF_STR_DB_SERVER_IP "db_server_ip"
#define CF_STR_DB_SERVER_PORT "db_server_port"
#define CF_STR_DB_NAME "db_name"
#define CF_STR_DB_USER_NAME "db_user_name"
#define CF_STR_DB_PASSWORD "db_password"
#define CF_STR_DB_INSERT_INTERVAL "db_interval"
#define CF_STR_COLLECTORD_IP "collectord_ip"
#define CF_STR_COLLECTORD_PORT "collectord_port"
#define CF_STR_COLLECTOR_PORT "collector_port"
#define CF_DEFAULT_DB_NAME "em_db"
#define CF_STR_LOG_LEVEL "log_level"


static int parse_collectord_config_line ( char * config_line, 
        collectord_config_t * collectord_config, char * parse_clrd_error_msg );
static short int parse_db_interval ( char * str );
static int read_config_from_file ( char * config_file_path, 
        collectord_config_t * collectord_config, char * clrd_read_cf_error_msg );

collectord_config_t g_collectord_config;
pthread_mutex_t mutex_g_collectord_config;
extern node_list_t * g_node_list;
extern int mutex_g_node_list;
extern db_update_flag_t g_db_update_flag;
extern pthread_mutex_t mutex_g_db_update_flag;
extern int startup_status [ 3 ];
extern pthread_mutex_t mutex_startup_status;

void * thread_config ( void * arg_config ) {

    LOG_START ( CLRD_LOG_PATH, g_collectord_config.log_level );
    LOG ( LOG_INFO, "%s", "Thread config started" );

    int ret = -1;
    int is_start = 1;
    char errmsg [ CLRD_ERRMSG_LEN ];
    time_t config_file_modify_time, nodelist_modify_time;
    struct stat config_file_status, nodelist_file_status;;

    ret = stat ( DCONFIG_FILE_PATH, & config_file_status );
    if ( ret == -1 ) {
        strerror_r ( errno, errmsg, CLRD_ERRMSG_LEN );
        LOG (LOG_FATAL, "Init stat config file %s error: %d %s",
                DCONFIG_FILE_PATH, errno, errmsg );
        fprintf ( stderr, "Init stat %s error\n", DCONFIG_FILE_PATH );
        exit ( 101 ); /*stat error*/
    }
    config_file_modify_time = config_file_status.st_mtime;
    LOG ( LOG_INFO, "%s", "Init stat config file sus" );


    ret = stat ( NODE_LIST_FILE_PATH, & nodelist_file_status );
    if ( ret == -1 ) {
        strerror_r ( errno, errmsg, CLRD_ERRMSG_LEN );
        LOG ( LOG_FATAL, "Init stat node list file %s error: %d %s",
                NODE_LIST_FILE_PATH, errno, errmsg );
        fprintf ( stderr, "Init stat %s error\n", NODE_LIST_FILE_PATH );
        exit ( 102 );
    }
    nodelist_modify_time = nodelist_file_status.st_mtime;
    LOG ( LOG_INFO, "%s", "Init stat node list file sus" );

    if ( is_start == 1 ) {
        pthread_mutex_lock ( & mutex_startup_status );
        startup_status [ 0 ] = 1;
        pthread_mutex_unlock ( & mutex_startup_status );
        is_start = 0;
    }


    while ( 1 ) {

        sleep ( CHECK_FILE_INTERVAL );

        LOG ( LOG_INFO, "%s", "Check config file..." );
        ret = stat ( DCONFIG_FILE_PATH, & config_file_status );
        if ( ret == -1 ) {
            strerror_r ( errno, errmsg, CLRD_ERRMSG_LEN );
            LOG ( LOG_ERROR, "Check config file %s status error: %d %s",
                    DCONFIG_FILE_PATH, errno, errmsg );
            continue;
        }

        if ( config_file_modify_time != config_file_status.st_mtime ) {
            LOG ( LOG_INFO, "Config file %s was modified", DCONFIG_FILE_PATH );
            config_file_modify_time = config_file_status.st_mtime;

            collectord_config_t d_config;
            char clrd_update_config_error_msg [ CLRD_INIT_ERROR_MSG_LEN ];
            ret = read_config_from_file ( DCONFIG_FILE_PATH, & d_config, clrd_update_config_error_msg );
            if ( ret < 0 ) {
                LOG ( LOG_ERROR, "Read config from file error: %d %s", ret, clrd_update_config_error_msg );
                continue;
            }
            LOG ( LOG_INFO, "%s", "Read config from file success" );

            pthread_mutex_lock ( & mutex_g_collectord_config );
            ret = collectord_config_need_update ( & g_collectord_config, & d_config );
            pthread_mutex_unlock ( & mutex_g_collectord_config );
            if ( ret < 0 ) {
                LOG ( LOG_ERROR, "Check collectord config need update error: %d", ret );
                continue;
            }

            else if ( ret == 1 ) {
                LOG ( LOG_INFO, "%s", "Collectord config need update" );

                LOG ( LOG_INFO, "%s", "Start update config" );
                pthread_mutex_lock ( & mutex_g_collectord_config );
                ret = collectord_update_config ( & g_collectord_config, & d_config );
                pthread_mutex_unlock ( & mutex_g_collectord_config );
                if ( ret < 0 ) {
                    LOG ( LOG_ERROR, "Update config error: %d", ret );
                    continue;
                }
                else {
                    LOG ( LOG_INFO, "%s", "Update config success" );
                }

                LOG ( LOG_INFO, "%s", "Start set database interval update flag" );
                pthread_mutex_lock ( & mutex_g_db_update_flag );
                g_db_update_flag = NEED_UPDATE;
                pthread_mutex_unlock ( & mutex_g_db_update_flag );
                LOG ( LOG_DEBUG, "%s", "End set database interval update flag" );

            }

            else {
                LOG ( LOG_INFO, "%s", "Collectord config need not update" );
            }

        }

        else {
            LOG ( LOG_INFO, "%s", "Config file was not modified" );
        }

        LOG ( LOG_INFO, "%s", "Check node list file..." );
        ret = stat ( NODE_LIST_FILE_PATH, & nodelist_file_status );
        if ( ret == -1 ) {
            strerror_r ( errno, errmsg, CLRD_ERRMSG_LEN );
            LOG ( LOG_ERROR, "Check node list file %s status error: %d %s",
                    NODE_LIST_FILE_PATH, errno, errmsg );
            continue;
        }

        if ( nodelist_modify_time != nodelist_file_status.st_mtime ) {

            LOG ( LOG_INFO, "Node list file %s was modified", NODE_LIST_FILE_PATH );
            nodelist_modify_time = nodelist_file_status.st_mtime;

            file_nodes_t config_file_nodes;
            dy_nodes_t config_dy_nodes;

            ret = get_file_nodes ( NODE_LIST_FILE_PATH, & config_file_nodes );
            if ( ret < 0 ) {
                LOG ( LOG_ERROR, "Read nodes from file error: %d", ret );
                continue;
            }
            else {
                LOG ( LOG_INFO, "%s", "Read nodes from file success" );
            }

            u_mutex_lock ( mutex_g_node_list );
            ret = dy_diff_nodes ( & config_file_nodes, g_node_list, & config_dy_nodes );
            u_mutex_unlock ( mutex_g_node_list );
            if ( ret < 0 ) {
                LOG ( LOG_ERROR, "Diff added and reduced nodes error: %d", ret );
                continue;
            }
            else if ( ret > 0 ) {
                LOG ( LOG_INFO, "%s", "Node list need update" );

                LOG ( LOG_INFO, "%s", "Start update node list" );
                u_mutex_lock ( mutex_g_node_list );
                ret = update_node_list ( & config_dy_nodes, g_node_list );
                u_mutex_unlock ( mutex_g_node_list );
                if ( ret < 0 ) {
                    LOG ( LOG_ERROR, "Update node list error: %d", ret );
                    continue;
                }
                else {
                    LOG ( LOG_INFO, "%s", "Update node list success" );
                }

            }
            else {
                LOG ( LOG_ERROR, "%s", "Node list need not update" );
            }
        }

        else {
            LOG ( LOG_INFO, "%s", "Node list file was not modified" );
        }

    } /*while*/

    LOG ( LOG_INFO, "%s", "Thread config end" );
    LOG_END;

    pthread_exit ( NULL );

} /*thread*/


static int read_config_from_file ( char * config_file_path, 
        collectord_config_t * collectord_config, char * clrd_read_cf_error_msg ) {

    LOG_START ( CLRD_LOG_PATH, LOG_DEBUG );
    LOG ( LOG_INFO, "%s", "Start read config from file" );
    if ( config_file_path == NULL || collectord_config == NULL || config_file_path [ 0 ] == '\0' ) {
        LOG ( LOG_ERROR, "%s", "Invalid parameters" );
        return -1;
    }

    int ret = -1;
    char errmsg [ CLRD_ERRMSG_LEN ];
    int str_len = 0;
    FILE * config_file;
    const int padding_len = 1 + 5;
    char line_buffer [ MAX_CONFIG_FILE_LINE_LEN + padding_len ];

    bzero ( collectord_config, sizeof ( collectord_config_t ) );

    config_file = fopen ( config_file_path, "r" );
    if ( config_file == NULL ) {
        strerror_r ( errno, errmsg, CLRD_ERRMSG_LEN );
        if ( errno == ENOENT ) {
            LOG ( LOG_ERROR, "Open config file %s error: file not exsit, %s",
                    config_file_path, errmsg );
            sprintf ( clrd_read_cf_error_msg, "open config file %s error: file not exsit, %s",
                    config_file_path, errmsg );
            return -2; /* file not exist */
        }
        else {
            LOG ( LOG_ERROR, "Open config file %s error: %d %s", 
                    config_file_path, errno, errmsg );
            sprintf ( clrd_read_cf_error_msg, "Open config file %s error: %d %s", 
                    config_file_path, errno, errmsg );
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

        if ( str_len > MAX_CONFIG_FILE_LINE_LEN ) { 
            LOG ( LOG_WARN, "Line too long discard: %s", line_buffer );
            sprintf ( clrd_read_cf_error_msg, "Line too long discard: %s", line_buffer );
            return -4;
        }

        char * line_buffer_trim_head = trim_head ( line_buffer, str_len );

        if ( * line_buffer_trim_head == '#' ) {
            continue;
        }
        if ( * line_buffer_trim_head == 0 ) {
            continue;
        }

        ret = parse_collectord_config_line ( line_buffer_trim_head, collectord_config, clrd_read_cf_error_msg );
        if ( ret < 0 ) {
            return -4;
        }

    }

    fclose ( config_file );

    LOG_END;

    return 0;

}


static int parse_collectord_config_line ( char * config_line, 
        collectord_config_t * collectord_config, char * parse_clrd_error_msg ) {

    LOG_START ( CLRD_LOG_PATH, LOG_DEBUG );
    LOG ( LOG_INFO, "%s", "Start parse collectord config line" );
    if ( config_line == NULL || collectord_config == NULL || config_line [ 0 ] == '\0' ) {
        LOG ( LOG_ERROR, "%s", "Invalid parameters" );
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
        LOG ( LOG_DEBUG, "%s", "Empty line" );
        sprintf ( parse_clrd_error_msg, "%s", "Empty line" );
        return -2;
    }

    if ( strlen ( token ) > MAX_CONFIG_ITEM_LEN ) {
        LOG ( LOG_WARN, "%s %s", "Config item too long", token );
        sprintf ( parse_clrd_error_msg, "%s %s", "Config item too long", token );
        return -3;
    }

    strcpy ( config_item, token ); /* config item */

    token = strtok_r ( NULL, delim, & saveptr );

    if ( token == NULL ) {
        LOG ( LOG_WARN, "%s", "Config item have no value" );
        sprintf ( parse_clrd_error_msg, "%s", "Config item have no value" );
        return -4;
    }

    char * remain_str = strtok_r ( NULL, delim, & saveptr );
    if ( remain_str != NULL ) {
        LOG ( LOG_WARN, "%s", "Config item have too many values" );
        sprintf ( parse_clrd_error_msg, "%s", "Config item have too many values" );
        return -5; /* too many value */
    }

    if ( strlen ( token ) > MAX_CONFIG_VALUE_LEN ) {
        LOG ( LOG_WARN, "%s %s", "Config value too long", token );
        sprintf ( parse_clrd_error_msg, "%s %s", "Config value too long", token );
        return -6;
    }

    strcpy ( config_value, token ); /* config value */

    if ( strcmp ( config_item, CF_STR_DB_SERVER_IP ) == 0 ) {

        if ( ! ip_str_is_valid ( config_value ) ) {
            LOG ( LOG_WARN, "%s %s", "DB server IP is invalid", config_value );
            sprintf ( parse_clrd_error_msg, "%s %s", "DB server IP is invalid", config_value );
            return -7;
        }
        if ( strlen ( config_value ) > sizeof ( collectord_config -> db_config.db_server_ip ) - 1 ) {
            LOG ( LOG_WARN, "%s %s", "DB server IP too long", config_value );
            sprintf ( parse_clrd_error_msg, "%s %s", "DB server IP too long", config_value );
            return -8;
        }
        strcpy ( collectord_config -> db_config.db_server_ip, config_value );
    }

    else if ( strcmp ( config_item, CF_STR_DB_SERVER_PORT ) == 0 ) {

        unsigned short int db_port;
        int db_ret = parse_port ( config_value, & db_port );
        if ( db_ret < 0 ) {
            LOG ( LOG_WARN, "%s %s", "DB server port is invalid", config_value );
            sprintf ( parse_clrd_error_msg, "%s %s", "DB server port is invalid", config_value );
            return -9;
        }
        collectord_config -> db_config.db_server_port = db_port;
    }

    else if ( strcmp ( config_item, CF_STR_DB_NAME ) == 0 ) {

        if ( strlen ( config_value ) > sizeof ( collectord_config -> db_config.db_name ) - 1 ) {
            LOG ( LOG_WARN, "%s %s", "DB name too long", config_value );
            sprintf ( parse_clrd_error_msg, "%s %s", "DB name too long", config_value );
            return -10;
        }
        strcpy ( collectord_config -> db_config.db_name, config_value );
    }

   else if ( strcmp ( config_item, CF_STR_DB_USER_NAME ) == 0 ) {

        if ( strlen ( config_value ) > sizeof ( collectord_config -> db_config.db_username ) - 1 ) {
            LOG ( LOG_WARN, "%s %s", "DB username too long", config_value );
            sprintf ( parse_clrd_error_msg, "%s %s", "DB username too long", config_value );
            return -11;
        }
        strcpy ( collectord_config -> db_config.db_username, config_value );
    }

    else if ( strcmp ( config_item, CF_STR_DB_PASSWORD ) == 0 ) {

        if ( strlen ( config_value ) > sizeof ( collectord_config -> db_config.db_password ) - 1 ) {
            LOG ( LOG_WARN, "%s %s", "DB password too long", config_value );
            sprintf ( parse_clrd_error_msg, "%s %s", "DB password too long", config_value );
            return -12;
        }
        strcpy ( collectord_config -> db_config.db_password, config_value );
    }

    else if ( strcmp ( config_item, CF_STR_DB_INSERT_INTERVAL ) == 0 ) {
        short int db_interval = parse_db_interval ( config_value );
        if ( db_interval == -2 ) {
            LOG ( LOG_WARN, "%s", "Parse long int for DB interval error" );
            sprintf ( parse_clrd_error_msg, "%s", "Parse long int for DB interval error" );
            return -13;
        }

        else if ( db_interval == -3 ) {
            LOG ( LOG_WARN, "Invalid DB interval < %d", MIN_DB_OUTPUT_INTERVAL );
            sprintf ( parse_clrd_error_msg, "Invalid DB interval < %d", MIN_DB_OUTPUT_INTERVAL );
            return -14;
        }

        else if ( db_interval == -4 ) {
            LOG ( LOG_WARN, "Invalid DB interval > %d", MAX_DB_OUTPUT_INTERVAL );
            sprintf ( parse_clrd_error_msg, "Invalid DB interval > %d", MAX_DB_OUTPUT_INTERVAL );
            return -15;
        }
        
        collectord_config -> db_output_interval = db_interval;
    }


    else if ( strcmp ( config_item, CF_STR_COLLECTORD_IP ) == 0 ) {

        if ( ! ip_str_is_valid ( config_value ) ) {
            LOG ( LOG_WARN, "%s %s", "Collectord IP is invalid", config_value );
            sprintf ( parse_clrd_error_msg, "%s %s", "Collectord IP is invalid", config_value );
            return -16;
        }
        if ( strlen ( config_value ) > sizeof ( collectord_config -> server_ip ) - 1 ) {
            LOG ( LOG_WARN, "%s %s", "Collectord IP too long", config_value );
            sprintf ( parse_clrd_error_msg, "%s %s", "Collectord IP too long", config_value );
            return -17;
        }
        strcpy ( collectord_config -> server_ip, config_value );
    }

    else if ( strcmp ( config_item, CF_STR_COLLECTORD_PORT ) == 0 ) {
        unsigned short int crd_port;
        int crd_ret = parse_port ( config_value, & crd_port );
        if ( crd_ret < 0 ) {
            LOG ( LOG_WARN, "%s %s", "Collectord port is invalid", config_value );
            sprintf ( parse_clrd_error_msg, "%s %s", "Collectord port is invalid", config_value );
            return -18;
        }
        collectord_config -> server_port = crd_port;
    }

    else if ( strcmp ( config_item, CF_STR_COLLECTOR_PORT ) == 0 ) {
        unsigned short int cr_port;
        int cr_ret = parse_port ( config_value, & cr_port );
        if ( cr_ret < 0 ) {
            LOG ( LOG_WARN, "%s %s", "Collector port is invalid", config_value );
            sprintf ( parse_clrd_error_msg, "%s %s", "Collector port is invalid", config_value );
            return -19;
        }
        collectord_config -> client_port = cr_port;
    }

    /* log level */
    else if ( strcmp ( config_item, CF_STR_LOG_LEVEL ) == 0 ) {
        if ( strcasecmp ( config_value, "DEBUG" ) == 0 ) {
            collectord_config -> log_level = LOG_DEBUG;
        }
        else if ( strcasecmp ( config_value, "INFO" ) == 0 ) {
            collectord_config -> log_level = LOG_INFO;
        }
        else if ( strcasecmp ( config_value, "WARN" ) == 0 ) {
            collectord_config -> log_level = LOG_WARN;
        }
        else if ( strcasecmp ( config_value, "ERROR" ) == 0 ) {
            collectord_config -> log_level = LOG_ERROR;
        }
        else if ( strcasecmp ( config_value, "FATAL" ) == 0 ) {
            collectord_config -> log_level = LOG_FATAL;
        }
    }

    else {
        LOG ( LOG_WARN, "%s %s", "Unknown configuration item", config_value );
        sprintf ( parse_clrd_error_msg, "%s %s", "Unknown configuration item", config_value );
        return -20;
    }

    LOG ( LOG_INFO, "%s", "End parse collectord config line" );
    LOG_END;

    return 0;

}


int collectord_config_need_update ( collectord_config_t * config_a, collectord_config_t * config_b ) {

    LOG_START ( CLRD_LOG_PATH, g_collectord_config.log_level );
    LOG ( LOG_INFO, "%s", "Start check whether collectord config need update" );

    if ( config_a == NULL || config_b == NULL ) {
        LOG ( LOG_ERROR, "%s", "Invalid parameters" );
        return -1;
    }

    if ( config_a -> db_output_interval != config_b -> db_output_interval ) {
        LOG ( LOG_INFO, "%s", "Collectord config need update" );
        LOG ( LOG_DEBUG, "%s %d", "DB interval is ", config_b -> db_output_interval );
        return 1;
    }
    LOG ( LOG_INFO, "%s", "Collectord config need not update" );
    LOG ( LOG_INFO, "%s", "End check whether collectord config need update" );
    LOG_END;

    return 0;

}


int collectord_update_config ( collectord_config_t * config_a, collectord_config_t * config_b ) {

    LOG_START ( CLRD_LOG_PATH, g_collectord_config.log_level );
    LOG ( LOG_INFO, "%s", "Start update collectord config" );
    if ( config_a == NULL || config_b == NULL ) {
        LOG ( LOG_ERROR, "%s", "Invalid parameters" );
        return -1;
    }

    config_a -> db_output_interval = config_b -> db_output_interval;

    LOG ( LOG_DEBUG, "%s %d", "New DB interval value is", config_b -> db_output_interval );
    LOG ( LOG_INFO, "%s", "End update collectord config" );
    LOG_END;

    return 0;

}


int init_collectord_config ( collectord_config_t * config, char * clrd_init_cf_error_msg ) {

    LOG_START ( CLRD_LOG_PATH, LOG_DEBUG );
    LOG ( LOG_INFO, "%s", "Start init collectord config" );
    if ( config == NULL ) {
        LOG ( LOG_ERROR, "%s", "Invalid parameters" );
        return -1;
    }

    int ret;

    collectord_config_t i_config;

    bzero ( ( void * ) & i_config, sizeof ( collectord_config_t ) );

    i_config.log_level = LOG_INFO;

    ret = read_config_from_file ( DCONFIG_FILE_PATH, & i_config, clrd_init_cf_error_msg );

    if ( ret < 0 ) {
        LOG ( LOG_ERROR, "Read config from file error: %d", ret );
        return -2;
    }

    if ( i_config.db_config.db_server_ip [ 0 ] == '\0' ) {
        LOG ( LOG_ERROR, "%s", "No DB server IP in config file" );
        sprintf ( clrd_init_cf_error_msg, "%s", "No DB server IP in config file" );
        return -3;
    }
    if ( i_config.db_config.db_server_port == 0 ) {
        LOG ( LOG_ERROR, "%s", "No DB server port in config file" );
        sprintf ( clrd_init_cf_error_msg, "%s", "No DB server port in config file" );
        return -4;
    }
    if ( i_config.db_config.db_username [ 0 ] == '\0' ) {
        LOG ( LOG_ERROR, "%s", "No DB server username in config file" );
        sprintf ( clrd_init_cf_error_msg, "%s", "No DB server username in config file" );
        return -6;
    }
    if ( i_config.db_config.db_password [ 0 ] == '\0' ) {
        LOG ( LOG_ERROR, "%s", "No DB server password in config file" );
        sprintf ( clrd_init_cf_error_msg, "%s", "No DB server password in config file" );
        return -7;
    }
    if ( i_config.db_output_interval == 0 ) {
        LOG ( LOG_ERROR, "%s", "No DB interval in config file" );
        sprintf ( clrd_init_cf_error_msg, "%s", "No DB interval in config file" );
        return -8;
    }
    if ( i_config.server_ip [ 0 ] == '\0' ) {
        LOG ( LOG_ERROR, "%s", "No collectord IP in config file" );
        sprintf ( clrd_init_cf_error_msg, "%s", "No collectord IP in config file" );
        return -9;
    }
    if ( i_config.server_port == 0 ) {
        LOG ( LOG_ERROR, "%s", "No collectord port in config file" );
        sprintf ( clrd_init_cf_error_msg, "%s", "No collectord port in config file" );
        return -10;
    }
    if ( i_config.client_port == 0 ) {
        LOG ( LOG_ERROR, "%s", "No collector port in config file" );
        sprintf ( clrd_init_cf_error_msg, "%s", "No collector port in config file" );
        return -11;
    }

    strcpy ( i_config.db_config.db_name, CF_DEFAULT_DB_NAME );
    
    memcpy ( ( void * ) config, ( void * ) & i_config, sizeof ( collectord_config_t ) );

    LOG ( LOG_INFO, "%s", "End init collectord config" );
    LOG_END;

    return 0;

}


short int parse_db_interval ( char * str ) {

    LOG_START ( CLRD_LOG_PATH, LOG_DEBUG );
    LOG ( LOG_INFO, "%s", "Start parse DB interval" );
    if ( str == NULL || str [ 0 ] == '\0' ) {
        LOG ( LOG_ERROR, "%s", "Invalid parameters" );
        return -1;
    }
    int ret = -1;
    long int db_interval;
    ret = parse_long_int ( str, & db_interval );
    if ( ret < 0 ) {
        LOG ( LOG_ERROR, "Parse long int for DB interval error: %d", ret );
        return -2;
    }
    if ( db_interval < 1 ) {
        LOG ( LOG_ERROR, "Invalid DB interval %d < %d", db_interval, MAX_DB_OUTPUT_INTERVAL );
        return -3;
    }
    if ( db_interval > MAX_DB_OUTPUT_INTERVAL ) {
        LOG ( LOG_ERROR, "Invalid DB interval %d > %d", db_interval, MAX_DB_OUTPUT_INTERVAL );
        return -4;
    }

    LOG ( LOG_DEBUG, "DB interval is %d", db_interval );
    LOG ( LOG_INFO, "%s", "End parse DB interval" );
    LOG_END;

    return ( short int ) db_interval;

}


int collectord_config_get_server_ip_port ( collectord_config_t * config, 
        char * server_ip, unsigned short int * server_port ) {

    if ( config == NULL || server_ip == NULL || server_port == NULL ) {
        return -1;
    }

    strncpy ( server_ip, config -> server_ip, MAX_IP_STR_LEN  );

    * ( server_ip + MAX_IP_STR_LEN ) = 0;

    * server_port = config -> server_port;

    return 0;

}


int collectord_get_db_config ( collectord_config_t * config, db_config_t * db_config ) {

    if ( config == NULL || db_config == NULL ) {
        return -1;
    }

    memcpy ( db_config, & config -> db_config, sizeof ( db_config_t ) );

    return 0;

}


int collectord_get_db_interval ( collectord_config_t * config,
        unsigned short int * db_output_interval ) {

    if ( config == NULL || db_output_interval == NULL ) {
        return -1;
    }

    * db_output_interval = config -> db_output_interval;

    return 0;

}






/*end of file*/
