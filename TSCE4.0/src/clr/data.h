
/*
 * Copyright (C) Inspur(Beijing)
 */


#ifndef TEYE_DATA_H
#define TEYE_DATA_H


#include "protocol.h"
#include "con_config.h"


int begin_collect_data(void);

int get_app_data(void);

int produce_app_pkg_head(app_hdr_t *app_head);

int complete_app_data(app_data_t *app_data, config_mod_info_t *mod_info_head);


#endif
