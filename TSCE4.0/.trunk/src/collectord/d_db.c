/*
 *
 * d_db.c
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "mysql.h"
#include "d_config.h"
#include "d_datas.h"
#include "u_log.h"
#include "u_util.h"


db_update_flag_t g_db_update_flag;
pthread_mutex_t mutex_g_db_update_flag;
extern collectord_config_t g_collectord_config;
extern pthread_mutex_t mutex_g_collectord_config;
extern data_buffer_t g_data_buffer;
extern pthread_mutex_t mutex_g_data_buffer;
extern int startup_status [ 3 ];
extern pthread_mutex_t mutex_startup_status;
static char db_buffer [ sizeof ( data_buffer_t ) ];


void * thread_db ( void * arg_db ) {

    LOG_START ( CLRD_LOG_PATH, g_collectord_config.log_level );
    LOG ( LOG_INFO, "thread db started, thread id is %lu", pthread_self ( ) );

    int ret;
    int is_start = 1;
    MYSQL * ret_conn;
    db_config_t db_config;
    unsigned short int db_interval;

    sleep ( 1 ); /*wait for thread config*/

    pthread_mutex_lock ( & mutex_g_collectord_config );
    collectord_get_db_config ( & g_collectord_config, & db_config );
    collectord_get_db_interval ( & g_collectord_config, & db_interval );
    pthread_mutex_unlock ( & mutex_g_collectord_config );

    LOG ( LOG_INFO, "DB config: server_ip %s | server_port %hu |  username %s | password %s | DB name %s", 
            db_config.db_server_ip, db_config.db_server_port, 
            db_config.db_username, db_config.db_password, db_config.db_name );
    LOG ( LOG_INFO, "DB interval is %d", db_interval );

    MYSQL * con;
    int exit_flag = 1;

RE_DB:
    con = mysql_init ( NULL );
    if ( con == NULL ) {
        LOG ( LOG_FATAL, "%s", "Init MySQL error" );
        if ( exit_flag == 1 ) {
            fprintf ( stderr, "%s\n", "Init MySQL error" );
            exit ( 201 );
        }
    }

    LOG ( LOG_INFO, "%s", "Init MySQL success" );

    const unsigned int db_conn_timeout = DB_CONN_TIMEOUT_VALUE;
    mysql_options ( con, MYSQL_OPT_CONNECT_TIMEOUT, & db_conn_timeout );
    ret_conn = mysql_real_connect ( con, 
            db_config.db_server_ip,
            db_config.db_username,
            db_config.db_password,
            db_config.db_name, 
            db_config.db_server_port, NULL, 0 );

    if ( ret_conn == NULL ) {
        LOG ( LOG_FATAL, "%s", "Connect to MySQL server error" );
        if ( exit_flag == 1 ) {
            fprintf ( stderr, "%s\n\n", "Connect to MySQL server error" );
            exit ( 202 );
        }
    }

    if ( is_start == 1 ) {
        pthread_mutex_lock ( & mutex_startup_status );
        startup_status [ 2 ] = 1;
        pthread_mutex_unlock ( & mutex_startup_status );
        is_start = 0;
    }

    LOG ( LOG_INFO, "%s", "Connect to MySQL server success" );

    db_update_flag_t db_flag;

    while ( 1 ) {

        sleep ( db_interval );

        pthread_mutex_lock ( & mutex_g_db_update_flag );
        db_flag = g_db_update_flag;
        pthread_mutex_unlock ( & mutex_g_db_update_flag );

        if ( db_flag == NEED_UPDATE ) {

            LOG ( LOG_INFO, "%s", "DB interval need update" );
            pthread_mutex_lock ( & mutex_g_collectord_config );
            collectord_get_db_interval ( & g_collectord_config, & db_interval );
            pthread_mutex_unlock ( & mutex_g_collectord_config );

            pthread_mutex_lock ( & mutex_g_db_update_flag );
            g_db_update_flag = NEED_NOT_UPDATE;
            pthread_mutex_unlock ( & mutex_g_db_update_flag );

            LOG ( LOG_INFO, "New DB interval is %d", db_interval );
        }

        LOG ( LOG_INFO, "%s", "DB interval need not update" );

        data_item_t data;
        char * db_buffer_start;
        int count = 0;

        db_buffer [ 0 ] = 0;
        strcpy ( db_buffer, DB_INSERT_SQL_PREFIX );

        pthread_mutex_lock ( & mutex_g_data_buffer );

        while  ( 1 ) {
            ret = get_data_item_from_buffer ( & g_data_buffer, & data );
            if ( ret < 0 ) { /*error*/
                LOG ( LOG_ERROR, "Get data item from data buffer error: %d", ret );
                break;
            }
            else if ( ret == 1 ) { /*empty*/
                break;
            }

            count ++;

            db_buffer_start = db_buffer + strlen ( db_buffer );
            sprintf ( db_buffer_start, DB_INSERT_SQL_FMT,
                    data.data_time, data.data_item, data.node_name, data.data_value );
        }

        pthread_mutex_unlock ( & mutex_g_data_buffer );

        int sql_len = strlen ( db_buffer );
        if ( sql_len > strlen ( DB_INSERT_SQL_PREFIX ) ) {
            LOG ( LOG_INFO, "%s", "Get data item finish, ready to insert" );
            db_buffer [ sql_len - 1 ]= 0;

            struct timeval timer_start = timeval_current ( );

            ret = mysql_query ( con, db_buffer );
            if ( ret != 0 ) {
                LOG ( LOG_ERROR, "%s", "Insert datas error, ignore" );
                /*continue;*/
                mysql_close ( con );
                exit_flag = 0;
                goto RE_DB;
                /*exit ( 203 );*/
            }

            double time_elapsed = timeval_elapsed ( & timer_start );
            LOG ( LOG_DEBUG, "Insert %d datas use %fs", count, time_elapsed );
        }

        else {
            LOG ( LOG_INFO, "%s", "Get data item empty, no data in buffer" );
        }

    } /* while ( 1 )*/


    mysql_close ( con );

}





/*end of file*/
