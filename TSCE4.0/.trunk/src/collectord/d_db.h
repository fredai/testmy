/*
 *
 * d_db.h
 *
 */

#ifndef _D_DB_H_
#define _D_DB_H_

#include <unistd.h>
#include <stdlib.h>
#include "e_define.h"
#include "d_define.h"

#define MAX_DB_NAME_LEN 512
#define MAX_DB_USER_NAME_LEN 512
#define MAX_DB_PASSWORD_LEN 512
#define MAX_DB_OUTPUT_INTERVAL 5
#define MIN_DB_OUTPUT_INTERVAL 1
#define DB_CONN_TIMEOUT_VALUE 1


struct db_config_s {
    char db_server_ip [ MAX_IP_STR_LEN ];
    unsigned short int db_server_port;
    char db_name [ MAX_DB_NAME_LEN ];
    char db_username [ MAX_DB_USER_NAME_LEN ];
    char db_password [ MAX_DB_PASSWORD_LEN ];
};
typedef struct db_config_s db_config_t;


enum db_interval_need_update_flag_e {
    NEED_NOT_UPDATE = 0,
    NEED_UPDATE = 1,
};
typedef enum db_interval_need_update_flag_e db_update_flag_t;


void * thread_db ( void * arg_db );

#define DB_INSERT_SQL_PREFIX "INSERT INTO em1 (time,type,node,data) VALUES"
#define DB_INSERT_SQL_FMT " ( \'%s\', \'%s\', \'%s\', %s ),"




#endif


/*end of file*/
