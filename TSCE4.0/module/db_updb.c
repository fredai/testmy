
/*
 * Copyright (C) Inspur(Beijing)
 */


#include "db_updb.h"
//#include "svr_define.h"
//#include "buffer.h"
//#include "protocol.h"
#include "util.h"
#include "error_handle.h"

//#include <pthread.h>
#include <time.h>

//for test modefy 3 to 300000
#define DB_CONN_TIMEOUT_VALUE 300000
//#define MAX_SQL_LEN 256 
#define MAX_MESSAGE_LEN 256

//Handle
//updb_handle_list_t updb_handle_list;

//SQLHENV V_OD_Env; // Handle ODBC environment
//SQLHDBC V_OD_hdbc; // Handle connection
//SQLHSTMT V_OD_hstmt = 0; //Handle stmt

long V_OD_erg; // result of functions
char V_OD_msg[MAX_MESSAGE_LEN];

//for test modefy 10 to 100

char V_OD_stat[100]; // Status SQL

SQLSMALLINT V_OD_mlen,V_OD_rowanz;
SQLINTEGER V_OD_err,V_OD_colanz;



int updb_get_connection ( updb_config_info_t * updb_config, updb_handle_t* updb_handle ) {
	if ( updb_config == NULL ) {
        return -1;
		cs_log (LOG_INFO, "updb_config:%s", updb_config);
    }
	// 1. allocate Environment handle and register version
	V_OD_erg=SQLAllocHandle(SQL_HANDLE_ENV,SQL_NULL_HANDLE,&(updb_handle->V_OD_Env));
	if ((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO)) {
		err_sys("%s", "allocate Environment handle error!");
		return DB_ALLOCATE_HANDLE_ERR;
	}

	V_OD_erg=SQLSetEnvAttr(updb_handle->V_OD_Env, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
	if ((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO)) {
		err_sys("%s", "register version error!");
		SQLFreeHandle(SQL_HANDLE_ENV, updb_handle->V_OD_Env);
		return DB_ALLOCATE_HANDLE_ERR;
	}

	// 2. allocate connection handle, set timeout
	V_OD_erg = SQLAllocHandle(SQL_HANDLE_DBC, updb_handle->V_OD_Env, &(updb_handle->V_OD_hdbc)); 
	if ((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO)) {
		err_sys("%s", "allocate connection handle error!");
		SQLFreeHandle(SQL_HANDLE_ENV, updb_handle->V_OD_Env);
		return DB_ALLOCATE_HANDLE_ERR;
	}

	SQLSetConnectAttr(updb_handle->V_OD_hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER *)DB_CONN_TIMEOUT_VALUE, 0);

	// 3. Connect to the datasource 
	V_OD_erg = SQLConnect(updb_handle->V_OD_hdbc, (SQLCHAR*) (updb_config->db_dsn), SQL_NTS, \
	(SQLCHAR*) (updb_config->db_username), SQL_NTS,(SQLCHAR*) (updb_config->db_password), SQL_NTS);
	if ((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO)) {
		err_sys("connect db failed (%d)", V_OD_erg);
		SQLGetDiagRec(SQL_HANDLE_DBC, updb_handle->V_OD_hdbc,1, V_OD_stat, &V_OD_err,V_OD_msg, 100, &V_OD_mlen);
		//printf("%s (%d)\n",V_OD_msg,V_OD_err);
	    err_sys("%s (%d)", V_OD_msg,V_OD_err);
		cs_log (LOG_INFO, "db connect error (%d)", V_OD_err);
		SQLFreeHandle(SQL_HANDLE_DBC, updb_handle->V_OD_hdbc);
		SQLFreeHandle(SQL_HANDLE_ENV, updb_handle->V_OD_Env);
		return DB_CONNECTED_ERR;
	}
	//printf("db connected !\n");
	cs_log (LOG_INFO, "db connected successfully. dsn:%s", updb_config->db_dsn);

	//4.allocate stmt handle
//	  V_OD_erg=SQLAllocHandle(SQL_HANDLE_STMT, updb_handle->V_OD_hdbc, &(updb_handle->V_OD_hstmt));  
//	  if ((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO)) {
//		  err_sys("allocate stmt handle failed (%d)", V_OD_erg); 
//		  SQLGetDiagRec(SQL_HANDLE_DBC, updb_handle->V_OD_hdbc,1, V_OD_stat,&V_OD_err,V_OD_msg,100,&V_OD_mlen);  
//		  err_sys("%s (%d)", V_OD_msg,V_OD_err); 
//		  SQLFreeHandle(SQL_HANDLE_DBC, updb_handle->V_OD_hdbc);
//		  SQLFreeHandle(SQL_HANDLE_ENV, updb_handle->V_OD_Env);  
//		  //sem_give(&database_control_mutex);	
//		  return DB_ALLOCATE_HANDLE_ERR;  
//	  }

	
	return 0;
}

void close_database(updb_handle_t* updb_handle)  
{  
    //sem_take(&database_control_mutex);  
    SQLFreeHandle(SQL_HANDLE_STMT, updb_handle->V_OD_hstmt);  
    SQLDisconnect(updb_handle->V_OD_hdbc);  
    SQLFreeHandle(SQL_HANDLE_DBC, updb_handle->V_OD_hdbc);  
    SQLFreeHandle(SQL_HANDLE_ENV, updb_handle->V_OD_Env);  
	cs_log(LOG_INFO, "Database Closed !");
}

int query_database(char *sql_string, updb_sql_val_list_t * updb_sql_val_list, updb_handle_t* updb_handle) {  
    //sem_take(&database_control_mutex);  
    //allocate stmt handle 
//    if (updb_handle->V_OD_hstmt) {
//		cs_log(LOG_DEBUG, "begin free handle SQL_HANDLE_STMT");
//      	V_OD_erg = SQLFreeHandle(SQL_HANDLE_STMT, updb_handle->V_OD_hstmt); 
//		if (V_OD_erg != SQL_SUCCESS) {
//			err_sys("free handle SQL_HANDLE_STMT failed! (%d)", V_OD_erg);
//		}
//      //V_OD_hstmt = NULL;   
//    }  
    V_OD_erg=SQLAllocHandle(SQL_HANDLE_STMT, updb_handle->V_OD_hdbc, &(updb_handle->V_OD_hstmt));  
    if ((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO)) {
		err_sys("allocate stmt handle failed (%d)", V_OD_erg); 
        SQLGetDiagRec(SQL_HANDLE_DBC, updb_handle->V_OD_hdbc,1, V_OD_stat,&V_OD_err,V_OD_msg,100,&V_OD_mlen);  
        err_sys("%s (%d)", V_OD_msg,V_OD_err); 
		cs_log(LOG_INFO, "step num 0");
		cs_log(LOG_INFO,"SQLAllocHandle v_od_msg (%s)", V_OD_msg); 
		cs_log(LOG_INFO,"SQLAllocHandle v_od_err (%d)", V_OD_err); 
		
		SQLFreeHandle(SQL_HANDLE_DBC, updb_handle->V_OD_hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, updb_handle->V_OD_Env);  
        //sem_give(&database_control_mutex);  
        return DB_ALLOCATE_HANDLE_ERR;  
    } 
  
    // execute sql
    //for test
    //cs_log(LOG_INFO, "sql: %s", sql_string);
    V_OD_erg=SQLExecDirect(updb_handle->V_OD_hstmt,sql_string,SQL_NTS);  
  
    if ((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO)) {  
    	err_sys("Error in Select (%s)", sql_string); 
        SQLGetDiagRec(SQL_HANDLE_DBC, updb_handle->V_OD_hdbc,1, V_OD_stat,&V_OD_err,V_OD_msg,100,&V_OD_mlen);  
        err_sys("%s (%d)", V_OD_msg,V_OD_err); 
		cs_log(LOG_INFO, "step num 1");
		cs_log(LOG_INFO,"SQLExecDirect v_od_msg (%s)", V_OD_msg); 
        cs_log(LOG_INFO,"SQLExecDirect v_od_err (%d)", V_OD_err); 
		
        close_database(updb_handle);  
        //sem_give(&database_control_mutex);  
        return DB_SQL_ERR;  
    }  

	//get the row counts  
    V_OD_erg=SQLRowCount(updb_handle->V_OD_hstmt,&V_OD_rowanz);  
    if ((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO)) {  
    	err_sys( "Error in get Number of RowCount (%d)", V_OD_erg);
		cs_log(LOG_INFO, "step num 2");
        close_database(updb_handle); 
        //sem_give(&database_control_mutex);  
        return DB_SQL_ERR;  
    }
	if (V_OD_rowanz > 1) {
		err_msg("Do not support row num greater than 1. Number of Rows (%d). sql is: %s",\
			V_OD_rowanz, sql_string);
		cs_log(LOG_INFO, "step num 3");
        close_database(updb_handle);  
        //sem_give(&database_control_mutex);  
        return DB_SQL_ERR;
	}
    cs_log(LOG_DEBUG, "Number of Rows (%d)", V_OD_rowanz);  
	
    // get the column counts  
    V_OD_erg=SQLNumResultCols(updb_handle->V_OD_hstmt,&V_OD_colanz);  
    if ((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO))  
    {  
    	err_sys("Error in get Number of ColumnCount (%d)", V_OD_erg);
		cs_log(LOG_INFO, "step num 4");
        close_database(updb_handle); 
        //sem_give(&database_control_mutex);  
        return DB_SQL_ERR;  
    }
	cs_log(LOG_DEBUG, "Number of Columns (%d)", V_OD_colanz); 

	updb_sql_val_list->sql_val_cnt = V_OD_colanz;
	if (updb_sql_val_list->sql_val_cnt > MAX_SQL_RET_COL_NUM) {
		err_sys("sql value count [%d] is more than total target value count [%d]", updb_sql_val_list->sql_val_cnt, MAX_SQL_RET_COL_NUM);
		cs_log(LOG_INFO, "step num 5");
        close_database(updb_handle);
		return DB_SQL_ERR;
	}
	for (int i = 0; i < updb_sql_val_list->sql_val_cnt; i++) {
		char *buffer = (updb_sql_val_list->updb_sql_val[i]).sql_moodule_val;
		SQLBindCol(updb_handle->V_OD_hstmt,i+1,SQL_C_CHAR, buffer,256,&V_OD_err); 
		//SQLBindCol(updb_handle->V_OD_hstmt,i+1,SQL_C_CHAR, (updb_sql_val_list->updb_sql_val[i]).sql_moodule_val,256,&V_OD_err); 
	}
	
	V_OD_erg=SQLFetch(updb_handle->V_OD_hstmt);
	if ((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO)) {
		err_sys("Error in SQLFetch (%d)", V_OD_erg);
		cs_log(LOG_INFO, "step num 6");
		close_database(updb_handle);
		return DB_SQL_ERR;
	}  

	//print sql ret value
	for (int i = 0; i < updb_sql_val_list->sql_val_cnt; i++) {
		char *buffer = (updb_sql_val_list->updb_sql_val[i]).sql_moodule_val;
		cs_log(LOG_INFO, "sql ret: %s", buffer); 
	}
    if (updb_handle->V_OD_hstmt) {
		cs_log(LOG_INFO, "begin free handle SQL_HANDLE_STMT");
	  	V_OD_erg = SQLFreeHandle(SQL_HANDLE_STMT, updb_handle->V_OD_hstmt); 
		if (V_OD_erg != SQL_SUCCESS) {
			//cs_log(LOG_ERROR, "free handle SQL_HANDLE_STMT failed! (%d)", V_OD_erg);
			err_sys("free handle SQL_HANDLE_STMT failed! (%d)", V_OD_erg);
		} else {
			cs_log(LOG_INFO, "end free handle SQL_HANDLE_STMT");
		}
	  //V_OD_hstmt = NULL;   
    } 
    //sem_give(&database_control_mutex);  
    return 0;  
}  

 


