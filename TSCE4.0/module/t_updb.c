/*Inspur (Beijing) Electronic Information Industry Co., Ltd.
  
  Written by zhaobo
  Updated by fengxiaoqing
  
  File: t_updb.c
  Version: V4.0.0
  Update: 2018-08-14 for test only
  
  Head file of t_updb.cpp which is used for getting updb 
  informaction.
*/


#include <stdio.h>
#include <stdbool.h>
#include "con_define.h"
#include "framework.h"
#include "common.h"
#include "t_updb.h"
#include "db_config.h"
#include "db_updb.h"

#define DEFAULT_STORAGE_NUM_VAL -9999
#define DEFAULT_STORAGE_FLOAT_VAL -9999.99
#define DEFAULT_STORAGE_CHAR_VAL "N/A"

updb_sql_val_list_t updb_sql_val_list;
updb_handle_list_t updb_handle_list;



static int updb_total_col_num = 0;
static bool init_flag = false;

static struct mod_info updb_mod_info[MAX_UPDB_SUPPORT * UPDB_MODULE_COL_NUM];
app_db_config_t app_db_config;

static int get_updb_number();

extern long V_OD_erg;


void updb_start()
{
	//NULL
}

void updb_read(struct module *mod)
{
	uint32 ReturnStat = 0;
    int i,j,k;
    uint32 tmp;

	cs_log(LOG_DEBUG, "mod->col=%d,updb_total_col_num=%d", mod->col, updb_total_col_num);
	assert(mod -> col == updb_total_col_num);
	
	//storage node need not query database
	if (app_db_config.computing_node == 0) {

	    for ( i = 0; i < 5; i++)
	    {
	        tmp = i * UPDB_MODULE_COL_NUM;
			snprintf (mod->info[tmp + 0].index_data, LEN_32, "%.2lf", DEFAULT_STORAGE_FLOAT_VAL);
			
	    	snprintf (mod->info[tmp + 1].index_data, LEN_32, "%d", DEFAULT_STORAGE_NUM_VAL);
			snprintf (mod->info[tmp + 2].index_data, LEN_32, "%d", DEFAULT_STORAGE_NUM_VAL);
			
			snprintf (mod->info[tmp + 3].index_data, "%s", DEFAULT_STORAGE_CHAR_VAL);
			snprintf (mod->info[tmp + 4].index_data, "%s", DEFAULT_STORAGE_CHAR_VAL);
	    }
		for ( i = 5; i < (app_db_config.updb_config).updb_cnt; i++) 
		{
			tmp = i * UPDB_MODULE_COL_NUM; 
			snprintf (mod->info[tmp + i].index_data, LEN_32, "%.2lf", DEFAULT_STORAGE_FLOAT_VAL);
		}

		cs_log(LOG_DEBUG, "this node is storage node, begin print dsn[%d] defalut value...", i);

		cs_log(LOG_DEBUG, "%.2lf", mod->info[tmp + 0].index_data);

		cs_log(LOG_DEBUG, "%d", mod->info[tmp + 1].index_data);
		cs_log(LOG_DEBUG, "%d", mod->info[tmp + 2].index_data);
		
		cs_log(LOG_DEBUG, "%s", mod->info[tmp + 3].index_data);
		cs_log(LOG_DEBUG, "%s", mod->info[tmp + 4].index_data);

		for ( i = 5; i < (app_db_config.updb_config).updb_cnt; i++) 
		{
			 tmp = i * UPDB_MODULE_COL_NUM; 
			 cs_log(LOG_DEBUG, "%.2lf", mod->info[tmp + i].index_data);
			
		}

		cs_log(LOG_DEBUG, "this node is storage node, end print dsn[%d] defalut value...", i);

		return;
	}
	
	
    //loop updb dsn
    for ( i = 0; i < (app_db_config.updb_config).updb_cnt; i++) {
      	tmp = i * UPDB_MODULE_COL_NUM; 
		int index_num = 0; //target num index
	  	//loop sql query
	  	for (int j = 0; j < (app_db_config.updb_sql).sql_cnt; j++) {
			updb_collect_sql_t updb_sql_info = (app_db_config.updb_sql).updb_sql_info[j];
			cs_log(LOG_INFO, "sql[%d]:%s", j, updb_sql_info.updb_sql);
		
//REQUERYDATABASE:
			ReturnStat = query_database(updb_sql_info.updb_sql, &updb_sql_val_list, &(updb_handle_list.updb_handle[i]));
			if (ReturnStat < 0) {
				err_msg("updb monitor error!(%d)", ReturnStat);
				cs_log(LOG_INFO, "query database status (%d)", ReturnStat);
			
				//goto REQUERYDATABASE;
				exit(1);
			}
			//loop sql value
			for (int k = 0; k < (updb_sql_val_list.sql_val_cnt); k++) {
				int value_int;
				double value_double;
				//Type one-to-one correspondence with the configuration file base_index.conf
				cs_log(LOG_INFO, "before transform index_num[%d],value[%s]", index_num, (updb_sql_val_list.updb_sql_val[k]).sql_moodule_val);
				if (index_num < 5)
				{
				    switch (index_num)
				   {

			        	case 0:
							value_double = atof((updb_sql_val_list.updb_sql_val[k]).sql_moodule_val);
							cs_log(LOG_INFO, "index_num[%d],value[%.2lf]", index_num, value_double);
							snprintf (mod->info[tmp+0].index_data, LEN_32, "%.2lf", value_double);	
							break;
						case 1:
							value_int = atoi((updb_sql_val_list.updb_sql_val[k]).sql_moodule_val);
							cs_log(LOG_INFO, "index_num[%d],value[%d]", index_num, value_int);
							snprintf (mod->info[tmp+1].index_data, LEN_32, "%d", value_int);
							break;
						case 2:
							value_int = atoi((updb_sql_val_list.updb_sql_val[k]).sql_moodule_val);
							cs_log(LOG_INFO, "index_num[%d],value[%d]", index_num, value_int);
							snprintf (mod->info[tmp+2].index_data, LEN_32, "%d", value_int);
							break;	
						case 3:
							cs_log(LOG_INFO, "index_num[%d],value[%s]", index_num, (updb_sql_val_list.updb_sql_val[k]).sql_moodule_val);
							snprintf (mod->info[tmp+3].index_data, LEN_32, "%s", (updb_sql_val_list.updb_sql_val[k]).sql_moodule_val);
							break;
						case 4:
							cs_log(LOG_INFO, "index_num[%d],value[%s]", index_num, (updb_sql_val_list.updb_sql_val[k]).sql_moodule_val);
							snprintf (mod->info[tmp+4].index_data, LEN_32, "%s", (updb_sql_val_list.updb_sql_val[k]).sql_moodule_val);
							break;
				  
				  }
					
				}
				else
			    {
					value_double = atof((updb_sql_val_list.updb_sql_val[k]).sql_moodule_val);
					cs_log(LOG_INFO, "index_num[%d],value[%.2lf]", index_num, value_double);
					snprintf (mod->info[tmp+index_num].index_data, LEN_32, "%.2lf", value_double);	

				}
		 
				index_num ++;
			}
	  	}
    }
    
	if ((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO))
		{

			cs_log(LOG_INFO, "paremeter j is %d", j);
			i=0;
			j=0;
			k=0;

		}
  	return;
}

static int get_updb_number()
{
    int i;
    int tmp;
	int ReturnStat;

	//read config and get connection
	if (!init_flag) {
		ReturnStat = init_app_db_config(UPDB_INFO_CONFIG_PATH, &app_db_config);
	    if (ReturnStat < 0) {
			err_msg("init app db config error!(%d)", ReturnStat);
			return ReturnStat;
		}

		updb_handle_list.handle_cnt = (app_db_config.updb_config).updb_cnt;
		if(app_db_config.computing_node == 1) {
			for (i = 0; i < updb_handle_list.handle_cnt; i++) {
				ReturnStat = updb_get_connection ( &((app_db_config.updb_config).updb_info[i]), &(updb_handle_list.updb_handle[i]) );
				if (ReturnStat < 0) {
					err_sys("updb get connection error!(%d)", ReturnStat);
					return ReturnStat;
				}
			}
		}
		
		init_flag = true;
	}
	
    memset(updb_mod_info, 0, sizeof(updb_mod_info));
//    if (init_app_db_config(UPDB_INFO_CONFIG_PATH, &app_db_config) < 0)
//	     return -1;
	int updb_cnt = (app_db_config.updb_config).updb_cnt;
    //updb_total_col_num = UPDB_MODULE_COL_NUM * updb_cnt + 1; //zhaobo:+1??
    updb_total_col_num = UPDB_MODULE_COL_NUM * updb_cnt;

//    	sprintf(gpu_mod_info[0].index_hdr, "%d", gpudevice.NumDevice);
    for ( i = 0; i < updb_cnt; i++) {
		updb_config_info_t updb_conf_info = (app_db_config.updb_config).updb_info[i];
		char *dsn = updb_conf_info.db_dsn;
		tmp = i * UPDB_MODULE_COL_NUM;


		sprintf(updb_mod_info[tmp+0].index_hdr, "%s_%s", dsn, PROCNUM);
    	sprintf(updb_mod_info[tmp+1].index_hdr, "%s_%s", dsn, SESSION_ACTIVE);
		sprintf(updb_mod_info[tmp+2].index_hdr, "%s_%s", dsn, DB_STATUS);
		sprintf(updb_mod_info[tmp+3].index_hdr, "%s_%s", dsn, INSTANCE_NAME);
    	sprintf(updb_mod_info[tmp+4].index_hdr, "%s_%s", dsn, RUN_TIME_DAYS);

		sprintf(updb_mod_info[tmp+5].index_hdr, "%s_%s", dsn, PROCESS_COUNT);
        sprintf(updb_mod_info[tmp+6].index_hdr, "%s_%s", dsn, SESSION_COUNT);
		
		sprintf(updb_mod_info[tmp+7].index_hdr, "%s_%s", dsn, DATAFILE_READS);
    	sprintf(updb_mod_info[tmp+8].index_hdr, "%s_%s", dsn, DATAFILE_WRITES);
		sprintf(updb_mod_info[tmp+9].index_hdr, "%s_%s", dsn, REDO_WRITES);
		sprintf(updb_mod_info[tmp+10].index_hdr, "%s_%s", dsn, DICTIONARY_HIT_RATIO);
    	sprintf(updb_mod_info[tmp+11].index_hdr, "%s_%s", dsn, LIBRARY_HIT_RATIO);
		sprintf(updb_mod_info[tmp+12].index_hdr, "%s_%s", dsn, TBS_MAX_USED_PERT);
		sprintf(updb_mod_info[tmp+13].index_hdr, "%s_%s", dsn, CONSISTENT_GETS);
    	sprintf(updb_mod_info[tmp+14].index_hdr, "%s_%s", dsn, LOGICAL_READS);
		sprintf(updb_mod_info[tmp+15].index_hdr, "%s_%s", dsn, PHYSICAL_READS);
		sprintf(updb_mod_info[tmp+16].index_hdr, "%s_%s", dsn, BLOCK_CHANGES);
    	sprintf(updb_mod_info[tmp+17].index_hdr, "%s_%s", dsn, BUFFER_CACHE_HIT_RATIO);

		sprintf(updb_mod_info[tmp+18].index_hdr, "%s_%s", dsn, SGA_FIXED);
		sprintf(updb_mod_info[tmp+19].index_hdr, "%s_%s", dsn, SGA_SHARED_LC);
    	sprintf(updb_mod_info[tmp+20].index_hdr, "%s_%s", dsn, SGA_SHARED_DD);
		sprintf(updb_mod_info[tmp+21].index_hdr, "%s_%s", dsn, SGA_SHARED_SLAB);
		sprintf(updb_mod_info[tmp+22].index_hdr, "%s_%s", dsn, SGA_SHARED_MISC);
		sprintf(updb_mod_info[tmp+23].index_hdr, "%s_%s", dsn, SGA_SHARED);
    	sprintf(updb_mod_info[tmp+24].index_hdr, "%s_%s", dsn, SGA_BUFFER_CACHE);
		sprintf(updb_mod_info[tmp+25].index_hdr, "%s_%s", dsn, SGA_REDO_CACHE);
		sprintf(updb_mod_info[tmp+26].index_hdr, "%s_%s", dsn, PGA_ALLOC);
    	sprintf(updb_mod_info[tmp+27].index_hdr, "%s_%s", dsn, PGA_USED);
    	sprintf(updb_mod_info[tmp+28].index_hdr, "%s_%s", dsn, DISK_SORTS);
    	sprintf(updb_mod_info[tmp+29].index_hdr, "%s_%s", dsn, MEMORY_SORTS);	
    	sprintf(updb_mod_info[tmp+30].index_hdr, "%s_%s", dsn, DISK_SORT_RATIO);

    	sprintf(updb_mod_info[tmp+31].index_hdr, "%s_%s", dsn, DEAD_LOCK_COUNT);
		sprintf(updb_mod_info[tmp+32].index_hdr, "%s_%s", dsn, INVALID_OBJECT_COUNT);
		sprintf(updb_mod_info[tmp+33].index_hdr, "%s_%s", dsn, ILLEGAL_USE_SYSTEM_TBS);
        

    }

	//print index_hdr info;
    for ( i = 0; i < updb_cnt; i++) {
		updb_config_info_t updb_conf_info = (app_db_config.updb_config).updb_info[i];
		char *dsn = updb_conf_info.db_dsn;
		tmp = i * UPDB_MODULE_COL_NUM;
		cs_log(LOG_DEBUG, "begin print index_hdr info....");
		for (int j = 0; j < UPDB_MODULE_COL_NUM; j++) {
			cs_log(LOG_DEBUG, "%s", updb_mod_info[tmp+j].index_hdr);
		}
		cs_log(LOG_DEBUG, "end print index_hdr info....");
	}
	
    return 0;
}


int
mod_register(struct module* mod)
{

REGETMODREGISTER:

    assert(mod != NULL);

	if (-1 == get_updb_number()) {
            return MODULE_FLAG_NOT_USEABLE;
	}

    // TODO: add decide module is usealbe in current HW and SW environment 
    register_module_fields(mod, updb_mod_info, \
 					   	updb_total_col_num, updb_start, updb_read);

	if ((V_OD_erg != SQL_SUCCESS) && (V_OD_erg != SQL_SUCCESS_WITH_INFO))	
		{
		 cs_log(LOG_INFO, "mod_register again");
		 goto REGETMODREGISTER;
		}
	
	return 0;


}

//int main(int argc, char * argv [ ]) {
//	int ret;
//	ret = init_app_db_config(UPDB_INFO_CONFIG_PATH, &app_db_config);
//	if (ret < 0) {
//        //app_config_free_node_list(&(g_app_svr_config.node_list));
//        err_exit("init config error: %s", UPDB_INFO_CONFIG_PATH);
//    }
//	return 0;
//}

