
/*
 * Copyright (C) Inspur(Beijing)
 */


#ifndef TEYE_BUFFER_H
#define TEYE_BUFFER_H


#include "svr_config.h"
#include "protocol.h"
#include <pthread.h>


/* link list for data of node */
struct node_data_s {
    app_data_t data;
    struct node_data_s *next;
};
typedef struct node_data_s node_data_t;


/* node status */
enum node_status_e {
    ILLEGAL = 0,
    DOWN,
    ACTIVE
};
typedef enum node_status_e node_status_t;

extern const char *NODE_STATUS_STR[];


/* node */
struct app_node_data_s {
    ip_t node_id;
    char node_name[MAX_NODE_NAME_LEN];
    char node_ip[MAX_IP_STR_LEN];
    node_status_t node_status;
    long long int data_count;
    /* whether exist recent data */
    char exist_recent_data; /* data head of this node */
    node_data_t node_data_head; /* mutex */
    pthread_mutex_t notex;
};
typedef struct app_node_data_s app_node_data_t;


/* data buffer */
struct app_data_buf_s {
    /* data for all nodes */
    app_node_data_t *data;
    /* node count */
    int count;
};
typedef struct app_data_buf_s app_data_buf_t;

/* global data buffer */
extern app_data_buf_t g_data_buffer;


int create_data_buffer(const node_list_t *node_list, \
        app_data_buf_t *p_data_buffer);

int destroy_data_buffer(app_data_buf_t *p_data_buffer) ;

app_node_data_t*
get_node_in_list(const app_data_buf_t *p_data_buffer, ip_t node_ip);

int put_data_in_buffer(const app_data_t *data_body, \
        app_node_data_t *node);

int set_node_recent_data(const app_data_t *data_body, \
        app_node_data_t *node);

int get_all_index_data_by_node_ix(const app_data_buf_t *app_data, \
        int ix, char *p_tr_data);

int get_all_index_data_by_node_name(const app_data_buf_t *app_data, \
        const char *node_name, char *p_rt_data);

int get_part_index_data_one_node(const app_node_data_t *app_data, \
        const int *index_ix, int cnt, char *p_tr_data);

int get_part_index_data_by_node_ix(const app_data_buf_t *app_data, \
        int node_ix, const int *index_ix, int cnt, char *p_rt_data);

int get_part_index_data_by_node_name(const app_data_buf_t *app_data, \
        const char *node_name, const int *index_ix, \
        int cnt, char *p_tr_data);

int clear_node(app_node_data_t *node);
int check_nodes_data_status(const app_data_buf_t *app_data);
int get_nodes_data_count(const app_data_buf_t *app_data) ;


#endif
