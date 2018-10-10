
/*
 * Copyright (C) Inspur(Beijing)
 */


#ifndef TEYE_CLR_CONFIG_H
#define TEYE_CLR_CONFIG_H


#include "con_config.h"
#include "framework.h"
#include "cs_log.h"


#define CF_STR_SECTION_SVR "[svr]"
#define CF_STR_SVR_IP "app_svr_ip"
#define CF_STR_SVR_PORT "app_svr_port"

#define CF_STR_SECTION_INTERVAL "[interval]"
#define CF_STR_APP_INTERVAL "app_interval"

#define CF_STR_SECTION_LOG "[log]"
#define CF_STR_LOG_LEVEL "log_level"

#define MIN_APP_INTERVAL 1
#define DEFAULT_APP_INTERVAL 1
#define MAX_APP_INTERVAL 60*3

#define CLR_CONFIG_PATH "../config/base.conf"
#define MODULE_SEARCH_PATH "../module"
#define SCRIPT_SEARCH_PATH "../module/script"


struct svr_config_s {
    char svr_ip [MAX_IP_STR_LEN];
    unsigned short int svr_port;
};
typedef struct svr_config_s svr_config_t;


typedef unsigned int app_interval_t;


struct app_clr_config_s {
    svr_config_t svr_config;
    app_interval_t itvl;
    log_level_t log_level;
};
typedef struct app_clr_config_s app_clr_config_t;


struct app_clr_module_s {
	int total_mod_num;
	struct module mods[MAX_MODULE_NUM];
};
typedef struct app_clr_module_s app_clr_module_t;


extern app_clr_module_t app_clr_module;


int init_app_clr_config (char * path, app_clr_config_t *app_clr_config);


#endif

