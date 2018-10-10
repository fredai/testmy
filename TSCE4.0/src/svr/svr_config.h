
/*
 * Copyright (C) Inspur(Beijing)
 */


#ifndef TEYE_SVR_CONFIG_H
#define TEYE_SVR_CONFIG_H


#include "con_config.h"
#include "log.h"


#define CF_STR_SECTION_SVR "[svr]"
#define CF_STR_SVR_IP "app_svr_ip"
#define CF_STR_SVR_PORT "app_svr_port"

#define CF_STR_SECTION_DB "[db]"
#define CF_STR_DB_SERVER_IP "db_server_ip"
#define CF_STR_DB_SERVER_PORT "db_server_port"
#define CF_STR_DB_NAME "db_name"
#define CF_STR_DB_USERNAME "db_username"
#define CF_STR_DB_PASSWORD "db_password"

#define CF_STR_SECTION_INTERVAL "[interval]"
#define CF_STR_APP_INTERVAL "app_interval"

#define CF_STR_SECTION_LOG "[log]"
#define CF_STR_LOG_LEVEL "log_level"

#define CF_STR_SECTION_NODELIST "[nodelist]"
#define CF_STR_SECTION_MONITORLIST "[monitorlist]"

#define MIN_APP_INTERVAL 1
#define DEFAULT_APP_INTERVAL 1
#define MAX_APP_INTERVAL 60*3

#define MAX_DB_NAME_LEN 512
#define MAX_DB_USERNAME_LEN 512
#define MAX_DB_PASSWORD_LEN 512
#define MAX_TABLE_NAME_LEN 256
#define DEFAULT_DB_SERVER_PORT 3306

/* path of svr config file */
#define SVR_CONFIG_BASE_PATH "../config/base.conf"
#define SVR_CONFIG_TSCE_INDEX_PATH "../config/tsce_index.conf"

#define SVR_CONFIG_BASE "base"
#define SVR_CONFIG_TSCE_INDEX "tsce_index"


struct svr_config_s {
    char svr_ip [MAX_IP_STR_LEN];
    unsigned short int svr_port;
};
typedef struct svr_config_s svr_config_t;


struct db_config_s {
    char db_server_ip[MAX_IP_STR_LEN];
    unsigned short int db_server_port;
    char db_name [MAX_DB_NAME_LEN];
    char db_username [MAX_DB_USERNAME_LEN];
    char db_password [MAX_DB_PASSWORD_LEN];
};
typedef struct db_config_s db_config_t;


/* config of nodlist*/
typedef struct cf_node_s {
    char node_name[MAX_NODE_NAME_LEN];
    struct cf_node_s * next;
} cf_node_t;


struct node_list_s {
    /* head of link list */
    cf_node_t head;
    int count;
};
typedef struct node_list_s node_list_t;

/* config of monitor*/
typedef struct cf_monitor_s {
    char monitor_name[MAX_NODE_NAME_LEN];
    struct cf_monitor_s * next;
} cf_monitor_t;


struct monitor_list_s {
    /* head of link list */
    cf_monitor_t head;
    int count;
};
typedef struct monitor_list_s monitor_list_t;


typedef unsigned int app_interval_t;


struct app_svr_config_s {
    /* svr config */
    svr_config_t svr_config;
    /* db config */
    db_config_t db_config;
    /* collect interval */
    app_interval_t itvl;
    /* node list */
    node_list_t node_list;
    /* monitor list */
    monitor_list_t monitor_list;
    /* log level */
    log_level_t log_level;
    /* table name */
    char table_name[MAX_TABLE_NAME_LEN];
};
typedef struct app_svr_config_s app_svr_config_t;


/* global config */
extern app_svr_config_t g_app_svr_config;


int init_app_svr_base_config(const char *path, app_svr_config_t *app_svr_config, char *flag);
int init_app_svr_tsce_index_config(const char *path, app_svr_config_t *app_svr_config, char *flag);
int app_config_free_node_list(node_list_t *node_list);


#endif
