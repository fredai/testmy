
/*
 * Copyright (C) Inspur(Beijing)
 */


#ifndef TEYE_DB_H
#define TEYE_DB_H


#include "svr_config.h"

#define DB_CONN_TIMEOUT 60


void *thread_db(void *arg_db);

int create_db_table(const app_svr_config_t *conf, \
        const config_mod_info_t *g_mod_info); 


#endif
