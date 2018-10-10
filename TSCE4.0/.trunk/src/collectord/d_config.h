/*
 *
 * d_config.h
 *
 */

#ifndef _D_CONFIG_H_
#define _D_CONFIG_H_

#include "d_nodes.h"
#include "d_db.h"
#include "d_define.h"
#include "u_log.h"

#define CHECK_FILE_INTERVAL 5
#define CLRD_INIT_ERROR_MSG_LEN 1024 

struct collectord_config_s {
    db_config_t db_config;
    unsigned short int db_output_interval; /* <=5 */
    char server_ip [ MAX_IP_STR_LEN ];
    unsigned short int server_port;
    unsigned short int client_port;
    log_level_t log_level;
};
typedef struct collectord_config_s collectord_config_t;


void * thread_config ( void * arg_config );
int init_collectord_config ( collectord_config_t * config, char * clrd_init_cf_error_msg );
int collectord_config_need_update ( collectord_config_t * config_a, collectord_config_t * config_b );
int collectord_update_config ( collectord_config_t * config_a, collectord_config_t * config_b );
int collectord_config_get_server_ip_port ( collectord_config_t * config, 
        char * server_ip, unsigned short int * server_port );
int collectord_get_db_config ( collectord_config_t * config, db_config_t * db_config );
int collectord_get_db_interval ( collectord_config_t * config,
        unsigned short int * db_output_interval );




#endif
/*end of file*/
