/*
 *
 * c_config.h
 *
 */

#ifndef _C_CONFIG_H_
#define _C_CONFIG_H_

#include "c_debug_log.h" 

struct collector_script_config_s {
    char collectord_ip [ MAX_IP_STR_LEN ];
    unsigned short int collectord_port;
    short int collect_interval;
    log_level_t log_level;
};
typedef struct collector_script_config_s clr_sct_conf_t;

struct collector_cmd_config_s {
    unsigned short int collecotr_cmd_port;
    int cmd_max_data_len;
    short int cmd_timeout;
    log_level_t log_level;
};
typedef struct collector_cmd_config_s clr_cmd_conf_t;


int init_collector_config ( clr_sct_conf_t * clr_sct_conf, clr_cmd_conf_t * clr_cmd_conf, 
        char * error_msg );
int collector_config_need_update ( 
        clr_sct_conf_t * clr_sct_conf_a, clr_cmd_conf_t * clr_cmd_conf_a,
        clr_sct_conf_t * clr_sct_conf_b, clr_cmd_conf_t * clr_cmd_conf_b );
int clr_read_config_from_file ( char * clr_config_file_path, 
        clr_sct_conf_t * clr_sct_conf, clr_cmd_conf_t * clr_cmd_conf );
int update_clr_sct_config ( clr_sct_conf_t * clr_sct_conf );


#endif
/*end of file*/
