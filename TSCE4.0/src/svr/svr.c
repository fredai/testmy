
/*
 * Copyright (C) Inspur(Beijing)
 */

#include "svr_define.h"
#include "service.h"
#include "db.h"
#include "buffer.h"
#include "util.h"
#include "common_util.h"
#include "error_handle.h"
#include "svr.h"
#include "state_server.h"
#include <signal.h>

static char* svr_process_cmd_line(int argc, char** argv);
static int svr_init_enviroment();


/* destroy buffer and exit */
#define destroy_buffer_exit(code) destroy_data_buffer (&g_data_buffer ); exit (code)
/* log configuration */
#define log_config do { \
        LOG ( LOG_INFO, "CONFIG: %s:%d|%s:%d %s %s|%d|%s", \
            g_app_svr_config.svr_config.svr_ip, g_app_svr_config.svr_config.svr_port, \
            g_app_svr_config.db_config.db_server_ip, g_app_svr_config.db_config.db_server_port, \
            g_app_svr_config.db_config.db_name, g_app_svr_config.db_config.db_username, \
            g_app_svr_config.itvl, g_app_svr_config.table_name ); \
        } while (0)


/* global config */
app_svr_config_t g_app_svr_config;
/* global module configure */
config_mod_info_t g_mod_info_head;
/* global data buffer */
app_data_buf_t g_data_buffer;
/* rtd mutex */
extern pthread_mutex_t g_rtd_num_mutex;


int 
main(int argc, char **argv)
{

    int ret;
    db_config_ns_t db_config_ns_i;
//    char *table_name;

    /* get table name */
/*
	table_name = svr_process_cmd_line(argc, argv);
    if (table_name == NULL) {
        err_exit("usage: %s -t tablename or %s --table tablename", \
                argv[0], argv[0]);
    }
*/
    /* check whether another process is running */

    ret = check_pid(SVR_PID_FILE_PATH);
    if (ret > 0) {
        err_exit("Another process (pid %d) is running", ret);
    }

    /* init enviroment */
    ret = svr_init_enviroment();
    if (ret < 0) {
        err_exit("init enviroment error");
    }

    /* init svr config */
    ret = init_app_svr_base_config(SVR_CONFIG_BASE_PATH, &g_app_svr_config, SVR_CONFIG_BASE);
	if (ret < 0) {
        app_config_free_node_list(&(g_app_svr_config.node_list));
        err_exit("init config error: %s", SVR_CONFIG_BASE_PATH);
    }
   
    ret = init_app_svr_tsce_index_config(SVR_CONFIG_TSCE_INDEX_PATH, &g_app_svr_config, SVR_CONFIG_TSCE_INDEX);
    if (ret < 0) {
        app_config_free_node_list(&(g_app_svr_config.node_list));
        err_exit("init config error: %s", SVR_CONFIG_TSCE_INDEX_PATH);
    }
    strcpy(g_app_svr_config.table_name, TABLE_NAME);

    /* read tsce index configure file */
    ret = analyse_index_info_config(BASE_INDEX_CONFIG_PATH, &g_mod_info_head);
  
	if (ret < 0) {
        app_config_free_node_list(&(g_app_svr_config.node_list));
        free_index_info_config(&g_mod_info_head);
        err_exit("analyse index info configure error");
    }

    /* create data buffer */
    ret = create_data_buffer(&(g_app_svr_config.node_list), &g_data_buffer);
  
	if (ret < 0) {
        app_config_free_node_list(&(g_app_svr_config.node_list));
        free_index_info_config(&g_mod_info_head);
        err_exit("create data buffer error");
    }

    /* free node list */
    app_config_free_node_list(&(g_app_svr_config.node_list));

    /* socket and bind */
    ret = init_sockfd (&g_app_svr_config);
    if (ret < 0) {
        free_index_info_config(&g_mod_info_head);
        err_quit("init_sockfd error");
    }

    ret = create_db_table(&g_app_svr_config, \
            &g_mod_info_head);
	if (ret < 0) {
        destroy_data_buffer(& g_data_buffer);
        free_index_info_config(&g_mod_info_head);
    }
    /* daemoize */

    pid_t daemonize = fork();
    if (daemonize < 0) {
        err_quit("fork error");
    } else if (daemonize > 0) {
        free_index_info_config(&g_mod_info_head);
        destroy_data_buffer(& g_data_buffer);
        exit(0);
    }

    freopen ("/dev/null", "r", stdin);
    freopen ("/dev/null", "w", stdout);
    freopen ("/dev/null", "w", stderr);

    umask(0);
	setsid();

    ignore_all_signals (); // except SIGUSR1 /

    /* record pid */
    FILE * pid_file = fopen(SVR_PID_FILE_PATH, "w");
    if (pid_file != NULL) {
        fprintf(pid_file, "%d", getpid());
        fclose(pid_file);
    }

    /* init logger */
    ret = init_logger(SVR_LOG_PATH);
    if (ret < 0) {
        err_exit("svr init logger error");
    }

    LOG_START(SVR_LOG_PATH, g_app_svr_config.log_level);
    LOG (LOG_INFO, "%s", "tscesd started");
    log_config;

    /* mutex for limit num of thread rtd */
    pthread_mutex_init (&g_rtd_num_mutex, NULL);

    /* create thread db */
    pthread_t tid_db;
    ret = pthread_create(&tid_db, NULL, thread_db, &g_mod_info_head);
    if (ret != 0) {
        LOG(LOG_FATAL, "%s", "create thread db error" );
        destroy_logger ( SVR_LOG_PATH );
        destroy_buffer_exit ( 12 );
    } else {
        LOG(LOG_INFO, "%s", "create thread db success");
    }
    /* create thread for node state */
//    db_config_ns_i.db_server_port = g_app_svr_config.db_config.db_server_port;
    db_config_ns_i.db_server_port = 5002;
    strcpy(db_config_ns_i.db_server_ip, g_app_svr_config.db_config.db_server_ip);
    strcpy(db_config_ns_i.db_username, g_app_svr_config.db_config.db_username);
    strcpy(db_config_ns_i.db_password, g_app_svr_config.db_config.db_password);
    pthread_t tid_ns;
    ret = pthread_create(&tid_ns, NULL, thread_ns, &db_config_ns_i);
    if (ret != 0) {
	LOG(LOG_FATAL, "%s", "create thread node state error" );
	destroy_logger ( SVR_LOG_PATH );
	destroy_buffer_exit ( 13 );
    } else {
        LOG(LOG_INFO, "%s", "create thread node state success");
    }

    /* start service */
    ret = app_service(&g_app_svr_config);
	while(1);
    if (ret < 0) {
        destroy_logger ( SVR_LOG_PATH );
        destroy_buffer_exit (13);
    }

    destroy_data_buffer(&g_data_buffer);
    destroy_logger(SVR_LOG_PATH);
    exit ( 0 );
}


/* 
 * ARGUMENT FORMAT: ./cmd -t tablename or
 * ./cmd --table tablename
 */
/*
static char * 
svr_process_cmd_line(int argc, char** argv)
{
    assert(argv != NULL);

    if (argc != 3 || (strcmp(argv [1], "-t") != 0 && \
                strcmp(argv[1], "--table") != 0)) {
        return NULL;
    }

    if (strlen(argv[2]) > MAX_TABLE_NAME_LEN) {
        err_msg("The length of table name " \
                "should be less than %d", MAX_TABLE_NAME_LEN);
        return NULL;
    }

    return argv[2];
}
*/

static int 
svr_init_enviroment()
{
    int ret;
    ret = util_mkdir(APP_SVR_RUN_DIR);
    if (ret < 0) {
        err_msg("mkdir error for %s", APP_SVR_RUN_DIR);
        return -1;
    }

    ret = util_mkdir(APP_SVR_LOG_DIR);
    if (ret < 0) {
        err_msg("mkdir error for %s", APP_SVR_LOG_DIR);
        return -1;
    }

    register_signal_usr1 ( );
    return 0;
}

