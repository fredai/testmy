/*
 *
 * db.h
 *
 */

#ifndef _DB_H
#define _DB_H

#include "ex_alarm.h"
#include "config.h"
#include "mysql.h"


struct alarm_data_s {
    /* id */
    long long int id;
    /* datatime */
    char datetime [ MAX_DATE_TIME_LEN ];
    /* node name */
    char node [ MAX_ALARM_NODE_NAME_LEN ];
    /* item name */
    char item [ MAX_ALARM_ITEM_NAME_LEN ];
    /* value */
    float value;
    /* count */
    int count;
	
};
typedef struct alarm_data_s alarm_data_t;



#define ex_mysql_free_results(results) mysql_free_result(results)

#define ex_mysql_close(con) mysql_close(con)

int mysql_get_connection ( db_config_t * db_config, MYSQL * * connection );

int get_init_id ( MYSQL * con, long long int * last_id );

int get_data_from_db ( MYSQL * con, long long int last_id, MYSQL_RES * * results );

//int fetch_alarm_data ( MYSQL_RES * results, alarm_data_t * alarm_data );
int fetch_alarm_data (  MYSQL * con, MYSQL_RES * results, alarm_data_t * alarm_data , alarm_items_t *alarm_items);
int check_database ( MYSQL * con );


#endif /* _DB_H */


/*end of file*/

