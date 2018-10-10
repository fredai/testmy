/*Inspur (Beijing) Electronic Information Industry Co., Ltd.
  
  Written by zhaobo
  Updated by fengxiaoqing
  
  File: t_updb.h
  Version: V4.0.0
  Update: 2018-08-20
  
  Head file of t_updb.cpp which is used for getting updb 
  informaction.
*/

//index num each updb support. If you want to increase the index, please change this value.
//A one-to-one correspondence with the configuration file base_index.conf
#define UPDB_MODULE_COL_NUM 34
//#define UPDB_MODULE_COL_NUM 6

//The maximum number of columns returned by each SQL statement,it cannot be greater than UPDB_MODULE_COL_NUM.
#define MAX_SQL_RET_COL_NUM UPDB_MODULE_COL_NUM

#define MAX_MODULE_VAL_LEN 256

//UPDB_MODULE_COL
#define PROCNUM "procnum"

#define SESSION_ACTIVE "session_active"
#define DB_STATUS "db_status"

#define INSTANCE_NAME "instance_name"
#define RUN_TIME_DAYS "run_time_days"


#define PROCESS_COUNT "process_count"
#define SESSION_COUNT "session_count"

#define DATAFILE_READS "datafile_reads"
#define DATAFILE_WRITES "datafile_writes"
#define REDO_WRITES "redo_writes"

#define DICTIONARY_HIT_RATIO "dictionary_hit_ratio"
#define LIBRARY_HIT_RATIO "library_hit_ratio"
#define TBS_MAX_USED_PERT "tbs_max_used_pert"

#define CONSISTENT_GETS "consistent_gets"
#define LOGICAL_READS "logical_reads"
#define PHYSICAL_READS "physical_reads"
#define BLOCK_CHANGES "block_changes"
#define BUFFER_CACHE_HIT_RATIO "buffer_cache_hit_ratio"

#define SGA_FIXED "sga_fixed"
#define SGA_SHARED_LC "sga_shared_lc"
#define SGA_SHARED_DD "sga_shared_dd"
#define SGA_SHARED_SLAB "sga_shared_slab"
#define SGA_SHARED_MISC "sga_shared_misc"
#define SGA_SHARED "sga_shared"
#define SGA_BUFFER_CACHE "sga_buffer_cache"
#define SGA_REDO_CACHE "sga_redo_cache"

#define PGA_ALLOC "pga_alloc"
#define PGA_USED "pga_used"

#define DISK_SORTS "disk_sorts"
#define MEMORY_SORTS "memory_sorts"
#define DISK_SORT_RATIO "disk_sort_ratio"

#define DEAD_LOCK_COUNT "dead_lock_count"
#define INVALID_OBJECT_COUNT "invalid_object_count"
#define ILLEGAL_USE_SYSTEM_TBS "illegal_use_system_tbs"




//typedef enum _bool { false = 0, true = 1, } bool;



