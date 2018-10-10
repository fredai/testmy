
/*
 * Copyright (C) Inspur(Beijing)
 */


#ifndef TEYE_CON_CONFIG_H
#define TEYE_CON_CONFIG_H

#include "con_define.h"

#define MAX_SECTION_STR_LEN 32 
#define MAX_CONFIG_ITEM_LEN 32
#define MAX_CONFIG_VALUE_LEN 256
#define MAX_CONFIG_FILE_LINE_LEN 10240
#define CF_STR_SECTION_MONITORLIST "[monitorlist]"


#define CONFIG_ITEM_DELIM   " \t\r\n\f\v"
#define BASE_INDEX_CONFIG_PATH "../config/base_index.conf"
#define TEYE_XML_CONFIG_PATH   "../config/base_xml.conf"
#define TSCE_INDEX_CONFIG_PATH   "../config/tsce_index.conf"


/* config index info struct */
struct config_index_info_s { 
	char index_name[LEN_32];
	char index_unit[LEN_16];
    char data_type[LEN_16];
};
typedef struct config_index_info_s config_index_info_t;


/* config module info struct */
struct config_mod_info_item_s {
	char mod_name[LEN_32];
	config_index_info_t index_info[MAX_INDEX_NUM];
	int  index_cnt;
};
typedef struct config_mod_info_item_s config_mod_info_item_t;


/* config module info struct list */
struct config_mod_info_s {
	config_mod_info_item_t info;
	struct config_mod_info_s *next;	
};
typedef struct config_mod_info_s config_mod_info_t;

/* config of monitor*/
typedef struct clr_cf_monitor_s {
    char monitor_name[MAX_NODE_NAME_LEN];
    struct clr_cf_monitor_s * next;
} clr_cf_monitor_t;


struct clr_monitor_list_s {
    /* head of link list */
    clr_cf_monitor_t head;
    int count;
};
typedef struct clr_monitor_list_s clr_monitor_list_t;


typedef struct {
    char label_name[LEN_32];
    char attribute_name[MAX_INDEX_NUM][LEN_32];
    int  attribute_cnt;
    int  data_ix[MAX_INDEX_NUM];
    char is_show_label;
} xml_format_t;


typedef struct xml_config_s xml_config_t;
struct xml_config_s {
    xml_format_t item;
    xml_config_t *next;
};

int get_config_item_and_value ( char * config_line, \
        char * config_item, const int config_item_len, \
        char * config_value, const int config_value_len);

/* get index configure */
int analyse_index_info_config(const char *config_path, config_mod_info_t *g_mod_info);

void free_index_info_config(config_mod_info_t *g_mod_info);

int get_index_ix_by_name(const config_mod_info_t *g_mod_info, \
        char (*index_name)[LEN_32], int *index_ix);

int analyse_xml_format_config(const char *path, xml_config_t *conf);

void free_xml_format_config(xml_config_t *conf);

int ignore_all_signals (void);

int init_monitor_list_config(const char *path, clr_monitor_list_t *clr_monitor_list);


int monitor_data(config_mod_info_t *mod_info_head, clr_monitor_list_t *clr_monitor_list, config_mod_info_t *monitor_mod_info_head);

#endif
