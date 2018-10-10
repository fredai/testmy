/*Inspur (Beijing) Electronic Information Industry Co., Ltd.
  
  Written by zhaobo
  
  File: db_config.h
  Version: V4.0.0
  Update: 2018-08-20
  
  Head file of db_config.cpp which is used for handle 
  db config file.
*/


#include "con_config.h"
#include "framework.h"
#include "cs_log.h"

#include "error_handle.h"
#include "util.h"


#ifndef TEYE_SVR_CONFIG_H
#define TEYE_SVR_CONFIG_H

#define MAX_DB_DSN_LEN 512
#define MAX_DB_USERNAME_LEN 512
#define MAX_DB_PASSWORD_LEN 512
//max updb dsn num
#define MAX_UPDB_SUPPORT 8
#define MAX_SQL_NUM 60
#define MAX_SQL_LEN 10240


#define UPDB_INFO_CONFIG_PATH "../config/updb_info.conf"
//#define UPDB_INFO_CONFIG_PATH "/var/tsced/config/updb_info.conf"

#define MAX_CONFIG_FILE_LINE_LEN 10240
#define MAX_SECTION_STR_LEN 32
#define DEFAULT_COMPUTING_NODE 0

#define CF_STR_SECTION_COMPUTING_NODE "[computing_node]"

#define CF_STR_SECTION_UPDB "[updb]"
#define CF_STR_UPDB_DSN "db_dsn"
#define CF_STR_UPDB_USERNAME "db_username"
#define CF_STR_UPDB_PASSWORD "db_password"

#define CF_STR_SECTION_SQL "[sql]"
#define CS_LOG_PATH "/var/log/tscecd/tscecd.log"



struct updb_config_info_s {
	//char db_server_ip[MAX_IP_STR_LEN];
	//unsigned short int db_server_port;
	char db_dsn [MAX_DB_DSN_LEN];
	char db_username [MAX_DB_USERNAME_LEN];
	char db_password [MAX_DB_PASSWORD_LEN];
};
typedef struct updb_config_info_s updb_config_info_t;

typedef struct updb_config_s {
    updb_config_info_t updb_info [MAX_UPDB_SUPPORT];
    int updb_cnt;
} updb_config_t;

typedef struct updb_collect_sql_s {
	char updb_sql [MAX_SQL_LEN];
} updb_collect_sql_t;

typedef struct updb_sql_s {
    updb_collect_sql_t updb_sql_info [MAX_SQL_NUM];
    int sql_cnt;
} updb_sql_t;


struct app_db_config_s {
	/*if this node is computing node*/
	unsigned short int computing_node;
    /* updb config */
    updb_config_t updb_config;
    /* collect sql */
	updb_sql_t updb_sql; 
};
typedef struct app_db_config_s app_db_config_t;

int init_app_db_config(char *path, app_db_config_t *app_db_config);
int parse_node ( char * str, unsigned short int * computing_node );



#endif


