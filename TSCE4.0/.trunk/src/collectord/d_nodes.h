/*
 *
 * d_nodes.h
 *
 */

#ifndef _D_NODES_H_
#define _D_NODES_H_

#include <pthread.h>
#include "e_define.h"
#include "d_define.h"

#define MAX_DY_NODES_NUM MAX_NODES_NUM
#define MAX_FILE_NODES_NUM MAX_NODES_NUM
#define DELIMITER_PER_DATA_ITEM 2 /*=^*/
#define RT_SCRIPT_DATA_LEN \
   (MAX_DATA_ITEM_NUM_PER_SCRIPT*(MAX_DATA_ITEM_NAME_LEN+MAX_DATA_VALUE_LEN+DELIMITER_PER_DATA_ITEM))

typedef unsigned int nd_ip_t;

struct node_info_s {
    unsigned int node_ip;
    char node_name [ MAX_NODE_NAME_LEN ];
};
typedef struct node_info_s node_info_t;


struct node_ctl_s {
    unsigned int socket_fd;
    pthread_t thread_id;
};
typedef struct node_ctl_s node_ctl_t;


enum node_status_e {
    NODE_STATUS_DOWN = 0,
    NODE_STATUS_ACTIVE = 1
};
typedef enum node_status_e node_status_t;


enum node_thread_instruction_e {
    NODE_RECV_THREAD_RUN = 0,
    NODE_RECV_THREAD_EXIT = 1
};
typedef enum node_thread_instruction_e node_istn_t;


struct script_rt_data_s {
    char rt_script_name [ MAX_SCRIPT_NAME_LEN ];
    char rt_script_data [ RT_SCRIPT_DATA_LEN ];
    time_t rt_data_time;
};
typedef struct script_rt_data_s script_rt_data_t;


struct node_rt_data_s {
    script_rt_data_t rt_node_data [ MAX_SCRIPT_NUM_PER_NODE ];
    int discard_time;
};
typedef struct node_rt_data_s node_rt_data_t;

struct node_s {
    node_info_t node_info;
    node_ctl_t node_ctl;
    node_status_t node_status;
    node_istn_t node_istn;
    node_rt_data_t node_data;
};
typedef struct node_s node_t;

struct node_list_s {
    node_t node_list [ MAX_NODES_NUM ];
    unsigned int nodes_num;
};
typedef struct node_list_s node_list_t;

struct dy_nodes_s {
    node_info_t added_nodes [ MAX_DY_NODES_NUM ]; 
    unsigned int added_nodes_num;
    node_info_t reduced_nodes [ MAX_DY_NODES_NUM ]; 
    unsigned int reduced_nodes_num;
};
typedef struct dy_nodes_s dy_nodes_t;

struct file_nodes_s {
    node_info_t nodes [ MAX_FILE_NODES_NUM ]; 
    unsigned int nodes_num;
};
typedef struct file_nodes_s file_nodes_t;


int init_node_list ( file_nodes_t * file_nodes, node_list_t * node_list );
int destroy_node_list ( node_list_t * node_list );
int get_file_nodes ( const char * node_file_path, file_nodes_t * file_nodes );
int dy_diff_nodes ( file_nodes_t * file_nodes, node_list_t * node_list, dy_nodes_t * dy_nodes );
int update_node_list ( dy_nodes_t * dy_nodes, node_list_t * node_list );


int nl_insert_node ( node_list_t * node_list, node_info_t * node_info );

int nl_delete_node_by_ndinfo ( node_list_t * node_list, node_info_t * node_info );
int nl_delete_node_by_nd_ip ( node_list_t * node_list, nd_ip_t node_ip );
int nl_delete_node_by_nd_ip_str ( node_list_t * node_list, char * node_ip_str );
int nl_delete_node_by_nd_name ( node_list_t * node_list, char * node_name );
int nl_delete_node_by_nd_id ( node_list_t * node_list, int node_id );

int nl_find_node_by_ndinfo ( node_list_t * node_list, node_info_t * node_info );
int nl_find_node_by_nd_ip ( node_list_t * node_list, nd_ip_t node_ip );
int nl_find_node_by_nd_ip_str ( node_list_t * node_list, char * node_ip_str );
int nl_find_node_by_nd_name ( node_list_t * node_list, char * node_name );

int nl_get_node_status_by_nd_id ( node_list_t * node_list, int node_id );
int nl_set_node_status_by_nd_id ( node_list_t * node_list, int node_id, node_status_t nd_st );

int nl_get_node_istn_by_nd_id ( node_list_t * node_list, int node_id );
int nl_set_node_istn_by_nd_id ( node_list_t * node_list, int node_id, node_istn_t node_istn );

int nl_get_node_script_data_by_nd_id ( 
        node_list_t * node_list,
        int node_id,
        char * script_name,
        char * script_data
        );

int nl_get_node_all_data_by_id ( 
        node_list_t * node_list,
        int node_id,
        node_rt_data_t * node_data
        );

int nl_put_node_script_data_by_id ( 
        char * script_name,
        char * script_data,
        node_list_t * node_list,
        int node_id
        );

int nl_delete_node_script_data_by_id ( 
        char * script_name,
        node_list_t * node_list,
        int node_id
        );

int nl_delete_node_all_data_by_id ( 
        node_list_t * node_list,
        int node_id
        );


struct node_status_info_s {
    char node_ip [ MAX_IP_STR_LEN ] ;
    char node_name [ MAX_NODE_NAME_LEN ];
    node_status_t node_status;
};
typedef struct node_status_info_s node_status_info_t;

int nl_get_node_name_by_nd_id ( char * node_name, node_list_t * node_list, int node_id );
int nl_get_node_ip_str_by_nd_id ( char * node_ip_str, node_list_t * node_list, int node_id );

int nl_set_node_data_discard_time ( node_list_t * node_list, int node_id, int discard_time );
int nl_get_node_data_discard_time ( node_list_t * node_list, int node_id, int * discard_time );

int traversal_node_status_info ( node_list_t * node_list,
        node_status_info_t * node_status_info, size_t node_status_info_size );

int nl_get_node_threadid_by_nd_id ( node_list_t * node_list, int node_id, pthread_t * thread_id );
int nl_set_node_threadid_by_nd_id ( node_list_t * node_list, int node_id, pthread_t thread_id );

#endif
/*end of file*/
