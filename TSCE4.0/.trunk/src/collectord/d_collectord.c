/*
 *
 * d_collectord.c
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include "d_nodes.h"
#include "d_datas.h"
#include "d_service.h"
#include "d_db.h"
#include "d_config.h"
#include "d_signal.h"
#include "u_log.h"
#include "u_shm.h"
#include "u_mutex.h"
#include "u_util.h"


void daemonize_handler ( int signum );
void clrd_exit ( void );


extern collectord_config_t g_collectord_config;
extern pthread_mutex_t mutex_g_collectord_config;
node_list_t * g_node_list;
int mutex_g_node_list;
extern data_buffer_t g_data_buffer;
extern pthread_mutex_t mutex_g_data_buffer;
extern db_update_flag_t g_db_update_flag;
extern pthread_mutex_t mutex_g_db_update_flag;
int startup_status [ 3 ] = { 0, 0, 0 };
pthread_mutex_t mutex_startup_status;


int main ( int argc, char * argv [ ] ) {

    int ret = -1;
    char errmsg [ CLRD_ERRMSG_LEN ];

    fprintf ( stderr, "\n%s\n", "Collectord is starting ..." );

    void * rm_addr = 0x0;
    int rm_shmid = u_shm_attach ( CLRD_SHM_KEY, sizeof ( node_list_t ), & rm_addr );
    if ( rm_shmid > 0 ) {
        u_shm_destroy ( rm_shmid, NULL );
    }

    int rm_mutex_id = u_mutex_attach ( CLRD_SEM_KEY );
    if ( rm_mutex_id > 0 ) {
        u_mutex_rmid ( rm_mutex_id );
    }

    /*daemonize*/
    pid_t pid, sid;
    pid = fork ( );
    if ( pid < 0 ) {
        fprintf ( stderr, "Fork for daemonize failure\n" );
        exit ( 21 );
    }
    else if ( pid > 0 ) {
        signal( SIGCHLD, daemonize_handler );
        signal( SIGUSR2, daemonize_handler );
        signal( SIGALRM, daemonize_handler );
        alarm ( 20 );
        pause ( );
        exit ( 0 );
    }
    umask ( 0 );
    sid = setsid ( );
    if ( sid < 0 ) {
        fprintf ( stderr, "Setsid for daemonize failure\n" );
        exit ( 22 );
    }

    clrd_signal_init ( );

    atexit ( clrd_exit );

    ret = init_logger ( CLRD_LOG_PATH );
    if ( ret < 0 ) {
        exit ( 1 );
    }

    LOG_START ( CLRD_LOG_PATH, LOG_DEBUG );

    /*ret = chdir ( "/" );
    if ( ret < 0 ) {
        exit ( 23 );
    }*/

    LOG ( LOG_INFO, "%s", "Collectord is starting ..."  );
    char clrd_init_cf_error_msg [ CLRD_INIT_ERROR_MSG_LEN ];

    ret = init_collectord_config ( & g_collectord_config, clrd_init_cf_error_msg );
    if ( ret < 0 ) {
        LOG ( LOG_FATAL, "Init config failure: %d %s", ret, clrd_init_cf_error_msg );
        fprintf ( stderr, "%s\n", clrd_init_cf_error_msg );
        exit ( 2 );
    }
    LOG ( LOG_INFO, "%s", "Init config success" );
    LOG_SETLEVEL ( g_collectord_config.log_level );

    void * clrd_shmaddr = 0x0;
    int clrd_shmid;
    clrd_shmid = u_shm_create ( CLRD_SHM_KEY, sizeof ( node_list_t ), & clrd_shmaddr );
    if ( clrd_shmid < 0 ) {
        strerror_r ( errno, errmsg, CLRD_ERRMSG_LEN );
        LOG ( LOG_FATAL, "Create shared memory failed: %d | errno %d | %s",
                clrd_shmid, errno, errmsg );
        fprintf ( stderr, "Create shared memory failed: %d | errno %d | %s\n",
                clrd_shmid, errno, errmsg );
        exit ( 3 );
    }
    if ( clrd_shmaddr == 0x0 ) {
        LOG ( LOG_FATAL, "%s", "Invalid shared memory addr" );
        fprintf ( stderr, "%s\n", "Invalid shared memory addr" );
        exit ( 4 );
    }
    g_node_list = ( node_list_t * ) clrd_shmaddr;
    LOG ( LOG_INFO, "%s", "Create shared memory success" );

    int clrd_mutex = -1;
    clrd_mutex = u_mutex_create ( CLRD_SEM_KEY );
    if ( clrd_mutex < 0 ) {
        strerror_r ( errno, errmsg, CLRD_ERRMSG_LEN );
        LOG ( LOG_FATAL, "Create mutex failed: %d | errno %d | %s",
                clrd_mutex, errno, errmsg );
        fprintf ( stderr, "Create mutex failed: %d | errno %d | %s\n",
                clrd_mutex, errno, errmsg );
        exit ( 5 );
    }
    LOG ( LOG_INFO, "%s", "Create mutex success" );

    ret = u_mutex_init ( clrd_mutex );
    if ( ret < 0 ) {
        strerror_r ( errno, errmsg, CLRD_ERRMSG_LEN );
        LOG ( LOG_FATAL, "Init mutex failed: %d | errno %d | %s",
                ret, errno, errmsg );
        fprintf ( stderr, "Init mutex failed: %d | errno %d | %s\n",
                ret, errno, errmsg );
        exit ( 6 );
    }
    mutex_g_node_list = clrd_mutex;
    LOG ( LOG_INFO, "%s", "Init mutex success" );

    file_nodes_t file_nodes;
    ret = get_file_nodes ( NODE_LIST_FILE_PATH, & file_nodes );
    if ( ret < 0 ) {
        LOG ( LOG_FATAL, "Read nodes from file %s error: %d", NODE_LIST_FILE_PATH, ret );
        fprintf ( stderr, "Read nodes from file %s error: %d\n", NODE_LIST_FILE_PATH, ret );
        exit ( 7 );
    }
    LOG ( LOG_INFO, "Read nodes info from file %s success", NODE_LIST_FILE_PATH );

    ret = init_node_list ( & file_nodes, g_node_list );
    if ( ret < 0 ) {
        LOG ( LOG_FATAL, "Init nodes list error: %d", ret );
        fprintf ( stderr, "Init nodes list error: %d\n", ret );
        exit ( 8 );
    }
    LOG ( LOG_INFO, "%s", "Init nodes list success" );

    ret = init_data_buffer ( & g_data_buffer );
    if ( ret < 0 ) {
        LOG ( LOG_FATAL, "Init data buffer error: %d", ret );
        fprintf ( stderr, "Init data buffer error: %d\n", ret );
        exit ( 9 );
    }
    LOG ( LOG_INFO, "%s", "Init data buffer success" );

    LOG ( LOG_INFO, "%s", "Creating threads..." );

    ret = pthread_mutex_init ( & mutex_g_collectord_config, NULL );
    if ( ret != 0 ) {
        LOG ( LOG_FATAL, "%s", "Init mutex config error: %d", ret );
        fprintf ( stderr, "Init mutex config error: %d\n", ret );
        exit ( 10 );
    }
    else {
        LOG ( LOG_INFO, "%s", "Init mutex config success" );
    }

    ret = pthread_mutex_init ( & mutex_g_data_buffer, NULL );
    if ( ret != 0 ) {
        LOG ( LOG_FATAL, "%s", "Init mutex for data error: %d", ret );
        fprintf ( stderr, "Init mutex for data error: %d\n", ret );
        exit ( 11 );
    }
    else {
        LOG ( LOG_INFO, "%s", "Init mutex for data success"  );
    }

    ret = pthread_mutex_init ( & mutex_g_db_update_flag, NULL );
    if ( ret != 0 ) {
        LOG ( LOG_FATAL, "%s", "Init mutex for DB flag error: %d", ret );
        fprintf ( stderr, "Init mutex for DB flag error: %d\n", ret );
        exit ( 12 );
    }
    else {
        LOG ( LOG_INFO, "%s", "Init mutex for db flag success"  );
    }

    pthread_t tid_config;
    void * thret_config;

    ret = pthread_create ( & tid_config, NULL, thread_config, NULL );
    if ( ret != 0 ) {
        LOG ( LOG_FATAL, "%s", "Create thread config error: %d", ret );
        fprintf ( stderr, "Create thread config error: %d\n", ret );
        exit ( 13 );
    }
    else {
        LOG ( LOG_INFO, "Create thread config success, thread id is %lu", tid_config );
    }

    pthread_t tid_service;
    void * thret_service;

    ret = pthread_create ( & tid_service, NULL, thread_service, NULL );
    if ( ret != 0 ) {
        LOG ( LOG_FATAL, "%s", "Create thread service error: %d", ret );
        fprintf ( stderr, "Create thread service error: %d\n", ret );
        exit ( 14 );
    }
    else {
        LOG ( LOG_INFO, "Create thread service success, thread id is %lu", tid_service );
    }

    pthread_t tid_db;
    void * thret_db;
    
    ret = pthread_create ( & tid_db, NULL, thread_db, NULL );
    if ( ret != 0 ) {
        LOG ( LOG_FATAL, "%s", "Create thread DB error: %d", ret );
        fprintf ( stderr, "Create thread DB error: %d\n", ret );
        exit ( 15 );
    }
    else {
        LOG ( LOG_INFO, "Create thread DB success, thread id is %lu", tid_db );
    }

    int sn = 0;
    while ( 1 ) {
        u_msleep ( 0, 100 );
        pthread_mutex_lock ( & mutex_startup_status );
        if ( startup_status [ 0 ] == 1 &&
             startup_status [ 1 ] == 1 &&
             startup_status [ 2 ] == 1 ) {
            kill( getppid ( ), SIGUSR2 );
            break;
        }
        pthread_mutex_unlock ( & mutex_startup_status );
        if ( ++ sn > 300 ) {
            exit ( 16 );
        }
    }

    /*daemonize*/
    freopen( "/dev/null", "r", stdin );
    freopen( "/dev/null", "w", stdout );
    freopen( "/dev/null", "w", stderr );

    LOG ( LOG_INFO, "%s", "Collectord started success" );

    /* record pid */
    ret = u_mkdir_p ( CLRD_RUN_DIR );
    if ( ret < 0 ) {
    }
    else {
        FILE * pid_file = fopen ( CLRD_PID_FILE_PATH, "w" );
        if ( pid_file != NULL ) {
            fprintf ( pid_file, "%d", getpid ( ) );
            fclose ( pid_file );
        }
    }

    ret = pthread_join ( tid_config, & thret_config );
    if ( ret != 0 ) {
        LOG ( LOG_FATAL, "Join thread config error: %d", ret );
    }
    else {
        LOG ( LOG_INFO, "%s", "Join thread config success" );
    }

    ret = pthread_join ( tid_service, & thret_service );
    if ( ret != 0 ) {
        LOG ( LOG_FATAL, "Join thread service error: %d", ret );
    }
    else {
        LOG ( LOG_INFO, "%s", "Join thread service success" );
    }

    ret = pthread_join ( tid_db, & thret_db );
    if ( ret != 0 ) {
        LOG ( LOG_FATAL, "Join thread DB error, return %d", ret );
    }
    else {
        LOG ( LOG_INFO, "%s", "Join threAd DB success" );
    }

    LOG ( LOG_INFO, "%s", "Collectord ending..."  );

    pthread_mutex_destroy ( & mutex_g_collectord_config );
    if ( ret != 0 ) {
        LOG ( LOG_FATAL, "%s", "Destroy mutex config error: %d", ret  );
    }
    else {
        LOG ( LOG_INFO, "%s", "Destroy mutex config success"  );
    }

    pthread_mutex_destroy ( & mutex_g_data_buffer );
    if ( ret != 0 ) {
        LOG ( LOG_FATAL, "%s", "Destroy mutex for data error: %d", ret  );
    }
    else {
        LOG ( LOG_INFO, "%s", "Destroy mutex for data success"  );
    }

    pthread_mutex_destroy ( & mutex_g_db_update_flag );
    if ( ret != 0 ) {
        LOG ( LOG_FATAL, "%s", "Destroy mutex for DB flag error: %d", ret  );
    }
    else {
        LOG ( LOG_INFO, "%s", "Destroy mutex for DB flag success"  );
    }

    ret = destroy_data_buffer ( & g_data_buffer );
    if ( ret < 0 ) {
        LOG ( LOG_ERROR, "Destroy data buffer failed: %d", ret );
    }
    LOG ( LOG_INFO, "%s", "Destroy data buffer success" );

    ret = u_mutex_rmid ( clrd_mutex );
    if ( ret < 0 ) {
        strerror_r ( errno, errmsg, CLRD_ERRMSG_LEN );
        LOG ( LOG_ERROR, "Remove  mutex failed: %d | errno %d | %s", ret, errno, errmsg );
    }
    LOG ( LOG_INFO, "%s", "Remove mutex success" );

    ret = u_shm_destroy ( clrd_shmid, clrd_shmaddr );
    if ( ret < 0 ) {
        strerror_r ( errno, errmsg, CLRD_ERRMSG_LEN );
        LOG ( LOG_ERROR, "Destroy shared memory failed: %d | errno %d | %s",
                ret, errno, errmsg );
    }
    LOG ( LOG_INFO, "%s", "Destroy shared memory success" );

    LOG ( LOG_INFO, "%s", "Collectord ended success"  );

    destroy_logger ( CLRD_LOG_PATH );

    exit ( EXIT_SUCCESS );

}


void daemonize_handler ( int signum ) {
    switch ( signum ) {
        case SIGUSR2: 
            fprintf ( stderr, "\n%s\n\n", "Collectord started success" );
            exit ( 0 );
        case SIGALRM: exit ( 17 );
        case SIGCHLD: exit ( 18 );
    }
}


void clrd_exit ( void ) {
    void * rm_addr = 0x0;
    int rm_shmid = u_shm_attach ( CLRD_SHM_KEY, sizeof ( node_list_t ), & rm_addr );
    if ( rm_shmid > 0 ) {
        u_shm_destroy ( rm_shmid, NULL );
    }

    int rm_mutex_id = u_mutex_attach ( CLRD_SEM_KEY );
    if ( rm_mutex_id > 0 ) {
        u_mutex_rmid ( rm_mutex_id );
    }
}


/*end of file*/
