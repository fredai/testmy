/*
 *
 * d_datas.h
 *
 */

#ifndef _D_DATAS_H_
#define _D_DATAS_H_

#include "e_define.h"
#include "d_nodes.h"
#include "d_define.h"
#include "u_queue.h"
#include <pthread.h>
#include <semaphore.h>

#define DATA_BUFFER_ITEM_NUM (MAX_NODES_NUM*MAX_SCRIPT_NUM_PER_NODE*MAX_DATA_ITEM_NUM_PER_SCRIPT)

struct data_item_s {
    char data_time [ MAX_DATE_TIME_LEN ];
    char node_ip [ MAX_IP_STR_LEN ];
    char node_name [ MAX_NODE_NAME_LEN ];
    char data_item [ MAX_DATA_ITEM_NAME_LEN ];
    char data_value [ MAX_DATA_VALUE_LEN ];
};
typedef struct data_item_s data_item_t;

struct data_buffer_s {
    u_queue_t clrd_que;
    data_item_t __clrd_datas [ DATA_BUFFER_ITEM_NUM ];
};
typedef struct data_buffer_s data_buffer_t;


int init_data_buffer ( data_buffer_t * data_buffer );
int put_data_item_in_buffer ( data_buffer_t * data_buffer, data_item_t * data_item );
int get_data_item_from_buffer ( data_buffer_t * data_buffer, data_item_t * data_item );
int destroy_data_buffer ( data_buffer_t * data_buffer );
int is_data_buffer_empty ( data_buffer_t * data_buffer );
int is_data_buffer_full ( data_buffer_t * data_buffer );




#endif

/*end of file*/
