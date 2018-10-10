/*
 *
 * db.c
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "mysql.h" 
#include "db.h"


#define DB_CONN_TIMEOUT_VALUE 3
#define MAX_SQL_LEN 256 
//#define GET_LAST_ID_SQL "SELECT max(id) FROM em1;"
#define GET_LAST_ID_SQL "SELECT max(id) FROM DataInfo;"
//#define QUERY_DATA_SQL "SELECT * FROM em1 WHERE id > %lld;"
#define QUERY_DATA_SQL "SELECT * FROM DataInfo WHERE id > %lld;"
#define CHECK_ALARM_INFO_TABLE_SQL "SELECT max(id) FROM AlarmInfo;"
#define GET_SQL_DATA "SELECT %s from DataInfo where id = %lld;"

/*
 * Function to get db connection
 * @db_config:    pointer to db config
 * @connection:   mysql connection
 * RET 0 or 1 on success, otherwise return a negative number
 */
 
int mysql_get_connection ( db_config_t * db_config, MYSQL * * connection ) {

    if ( db_config == NULL || connection == NULL ) {
        return -1;
    }

    MYSQL * ret_conn;
    MYSQL * con;

    /* init */
    con = mysql_init ( NULL );

    if ( con == NULL ) {
        return -2;
    }

    /* set options */
    const unsigned int db_conn_timeout = DB_CONN_TIMEOUT_VALUE;

    mysql_options ( con, MYSQL_OPT_CONNECT_TIMEOUT, & db_conn_timeout );

    /* connect */
    ret_conn = mysql_real_connect ( con,
	    db_config -> db_server_ip,
	    db_config -> db_username,
	    db_config -> db_password,
	    db_config -> db_name,
	    db_config -> db_server_port, NULL, 0 );

    if ( ret_conn == NULL ) {
        return -3;
    }

    * connection = con;

    return 0;

}


/*
 * Function to get init id
 * @con:        pointer to mysql connection
 * @last_id:    parameter output last id 
 * RET 0 or 1 on success, otherwise return a negative number
 */

int get_init_id ( MYSQL * con, long long int * last_id ) {

    if ( con == NULL || last_id == NULL ) {
        return -1;
    }

    int ret;
    MYSQL_RES * results = NULL;
    MYSQL_ROW record;

    /* execute sql */
    ret = mysql_query ( con, GET_LAST_ID_SQL );
    if ( ret != 0 ) {
        return -2;
    }


    /* store result */
    results = mysql_store_result ( con );
    if ( results == NULL ) {
        return -3;
    }

    /* get record */
    record = mysql_fetch_row ( results );

    if ( record == NULL ) {
        return -4;
    }

    if ( * record == NULL ) {
        return -5;
    }

    * last_id = atoll ( record [ 0 ] );

    mysql_free_result ( results );

    return 0;

}


/*
 * Function to get data from db
 * @con:        pointer to mysql connection
 * @last_id:    last id 
 * @results:    query results
 * RET 0 or 1 on success, otherwise return a negative number
 */

int get_data_from_db ( MYSQL * con, long long int last_id, MYSQL_RES * * results ) {

    if ( con == NULL || results == NULL ) {
        return -1;
    }

    int ret;
    char db_buffer [ MAX_SQL_LEN ];

    /* produce sql statement */
    sprintf ( db_buffer, QUERY_DATA_SQL, last_id );

//	printf("***********file = %s, line = %d, db_buffer = %s\n", __FILE__, __LINE__, db_buffer);

    /* execute sql statement */
    ret = mysql_query ( con, db_buffer );
    if ( ret != 0 ) {
	return -2;
    }

    /* get results */
    * results = mysql_store_result ( con );
    if ( * results == NULL ) {
	return -3;
    }

    return 0;

}


/*
 * Function to fetch alarm data
 * @results:    query results
 * @alarm_data:   alarm data 
 * RET 0 or 1 on success, otherwise return a negative number
 */

int fetch_alarm_data ( MYSQL * con,  MYSQL_RES * results, alarm_data_t * alarm_data , alarm_items_t *alarm_items) {

    MYSQL_ROW record;
    record = mysql_fetch_row ( results );

    if ( record == NULL ) {
	return 0;
    }

    /* get alarm data */
    alarm_data -> id = atoll ( record [ 0 ] );
    strcpy ( alarm_data -> datetime, record [ 1 ] );
    strcpy ( alarm_data -> node, record [ 2 ] );

   return 1;
}



/*
 * Function to check database
 * @con:  mysql connection 
 * RET 0 or 1 on success, otherwise return a negative number
 */

int check_database ( MYSQL * con ) {

    if ( con == NULL ) {
        return -1;
    }

    int ret;
    MYSQL_RES * results = NULL;
    MYSQL_ROW record;

    /* sql for check db connection */
    ret = mysql_query ( con, CHECK_ALARM_INFO_TABLE_SQL );
    if ( ret != 0 ) {
        return -2;
    }


    /* store results */
    results = mysql_store_result ( con );
    if ( results == NULL ) {
        return -3;
    }

    record = mysql_fetch_row ( results );

    if ( record == NULL ) {
        return -4;
    }

    mysql_free_result ( results );

    return 0;


}


/*end of file*/

