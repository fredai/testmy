/*Inspur (Beijing) Electronic Information Industry Co., Ltd.
  
  Written by zhaobo
  
  File: db_updb.h
  Version: V4.0.0
  Update: 2018-03-28
  
  Head file of db_updb.cpp which is used for updb 
  operation.
*/

#ifndef TEYE_DB_H
#define TEYE_DB_H


#include "db_config.h"
#include "t_updb.h"

#include <sql.h>
#include <sqlext.h>
//#include </usr/local/unixODBC-2.3.5/include/sqltypes.h>
#include <odbcinst.h>


//#define DB_CONN_TIMEOUT 60
//max column num in one sql result
//#define MAX_EACH_SQL_COLUMN_NUM 10
//max row num in one sql result
//#define MAX_EACH_SQL_ROW_NUM 1

#define DB_ALLOCATE_HANDLE_ERR -2
#define DB_CONNECTED_ERR -3
#define DB_SQL_ERR -4

typedef struct updb_handle_s {
    SQLHENV V_OD_Env; // Handle ODBC environment
	SQLHDBC V_OD_hdbc; // Handle connection
	SQLHSTMT V_OD_hstmt; //Handle stmt
} updb_handle_t;

typedef struct updb_handle_list_s {
    updb_handle_t updb_handle [MAX_UPDB_SUPPORT];
    int handle_cnt;
} updb_handle_list_t;

typedef struct updb_sql_val_s {
	char sql_moodule_val [MAX_MODULE_VAL_LEN];
} updb_sql_val_t;

typedef struct updb_sql_val_list_s {
    updb_sql_val_t updb_sql_val [MAX_SQL_RET_COL_NUM];
    int sql_val_cnt;
} updb_sql_val_list_t;

int updb_get_connection ( updb_config_info_t * updb_config, updb_handle_t* updb_handle );

void close_database(updb_handle_t* updb_handle);

int query_database(char *sql_string, updb_sql_val_list_t * updb_sql_val_list, updb_handle_t* updb_handle);



#endif

