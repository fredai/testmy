
/*
 * Copyright (C) Inspur(Beijing)
 */


#include "buffer.h"
#include "error_handle.h"
#include "util.h"


const char *NODE_STATUS_STR[] = {
    "illeagal",
    "down",
    "active"
};


static int split_index_data(char *index_data_buffer, \
        char (*index_data_array)[LEN_32]);


int 
create_data_buffer(const node_list_t *node_list, \
        app_data_buf_t *p_data_buffer)
{
    assert(node_list != NULL && p_data_buffer != NULL);

    int i, ret;
    const int node_count = node_list -> count;
    app_node_data_t *data_buffer;

    data_buffer = (app_node_data_t *) \
                  malloc(sizeof(app_node_data_t) * node_count);
    if (data_buffer == NULL) {
        err_sys("malloc error");
        return -1;
    }

    /* init */
    cf_node_t *cf_node = node_list -> head.next;
    for (i = 0; i < node_count; i++) {
        assert(cf_node != NULL);

        strcpy(data_buffer[i].node_name, cf_node->node_name);
        /* node ip */
        ret = hostname_to_ip(cf_node->node_name, &(data_buffer[i].node_id));
        if (ret < 0) {
            err_sys("hostname_to_ip error for %s", cf_node->node_name);
            return -1;
        }
        ipv4_uint_to_str(data_buffer[i].node_id, \
                data_buffer[i].node_ip, MAX_IP_STR_LEN);
        data_buffer[i].node_status = DOWN;
        data_buffer[i].data_count = 0;
        data_buffer[i].exist_recent_data = 0;
        data_buffer[i].node_data_head.next = NULL;
        ret = pthread_mutex_init(&(data_buffer[i].notex), NULL);
        if (ret != 0) {
            err_sys("error for pthread_mutex_init");
            return -1;
        }

        cf_node = cf_node->next;
    }

    p_data_buffer->count = node_count;
    p_data_buffer->data = data_buffer;

    return 0;
}


int 
destroy_data_buffer(app_data_buf_t *p_data_buffer) 
{
    assert(p_data_buffer != NULL);

    int i;
    const int count = p_data_buffer->count;

    for (i = 0; i < count; i++) {
        pthread_mutex_destroy (&( (p_data_buffer -> data)[i].notex));
    }

    if (NULL != p_data_buffer->data) {
        free (p_data_buffer->data);
    }

    return 0;
}


app_node_data_t*
get_node_in_list(const app_data_buf_t *p_data_buffer, ip_t node_ip)
{
    assert(p_data_buffer != NULL);

    int i;
    app_node_data_t *node_datas = p_data_buffer->data;
    const int node_count = p_data_buffer->count;

    for (i = 0; i < node_count; i++) {
        if (node_datas[i].node_id == node_ip) {
            return node_datas + i;
        }
    }

    return NULL;
}


int 
put_data_in_buffer(const app_data_t *data_body, app_node_data_t *node) 
{
    assert(node != NULL && data_body != NULL);

    node_data_t *p = &(node->node_data_head);
    while (p->next != NULL) {
        p = p->next;
    }

    node_data_t *new_data = (node_data_t*) malloc(sizeof(node_data_t));
    if (new_data == NULL) {
        err_msg("malloc error");
        return -1;
    }
    memcpy ( (void*) (&(new_data->data)), \
            (void*) data_body, sizeof(app_data_t));
    new_data->next = NULL;
    p->next = new_data;
    node->data_count++;
    return 0;
}


int 
set_node_recent_data(const app_data_t *data_body, app_node_data_t *node) 
{
    assert(data_body != NULL && node != NULL);

    memcpy ( (void*) (&(node->node_data_head.data)), \
            (void*) data_body, sizeof(app_data_t));
    node->exist_recent_data = 1;

    return 0;
}


int 
get_all_index_data_by_node_ix(const app_data_buf_t *app_data, \
        int ix, char *p_rt_data)
{
    assert(app_data != NULL && p_rt_data != NULL && ix >= 0);

    /* node count */
    const int count = app_data -> count;
    assert(ix < count);

    /* nodes data head */
    app_node_data_t *nodes = app_data->data;

    /* node name, node ip */
    sprintf(p_rt_data, "%s,", nodes[ix].node_name);

    pthread_mutex_lock(&(nodes[ix].notex));
    sprintf(p_rt_data + strlen(p_rt_data), "%s,", \
            NODE_STATUS_STR[nodes[ix].node_status]);
    
    if (nodes[ix].node_status == ACTIVE) {
        /* have recent data */ 
        if (nodes[ix].exist_recent_data == 1 ) {
            strcat(p_rt_data, nodes[ix].node_data_head.data.buffer);
        } else {
            /* status is active but no any data */
        }
    }

    pthread_mutex_unlock (&(nodes[ix].notex));
    return 0;
}


int 
get_all_index_data_by_node_name(const app_data_buf_t *app_data, \
        const char *node_name, char *p_rt_data) 
{
    assert(app_data != NULL && node_name != NULL && p_rt_data != NULL);

    int i; 
    app_node_data_t *nodes = app_data->data;
    const int count = app_data->count;

    for (i = 0; i < count; i++) {
        if (strcmp(node_name, nodes[i].node_name) == 0) {
            break;
        }
    }

    /* not found this node */
    if (i == count) {
        sprintf(p_rt_data, "%s,", node_name);
        sprintf(p_rt_data + strlen(p_rt_data), "%s", \
                NODE_STATUS_STR[ILLEGAL]);

    } else {
        get_all_index_data_by_node_ix(app_data, i, p_rt_data); 
    }

    return 0;
}

static int
split_index_data(char *index_data_buffer, \
        char (*index_data_array)[LEN_32])
{
    assert(index_data_buffer != NULL && index_data_array != NULL);
   
    int cnt; 
    char *str, *token, *saveptr;

    cnt = 0;
    for (str = index_data_buffer; ; str = NULL) {
        token = strtok_r(str, TEYE_DATA_DELIMTER, &saveptr);
        if (token == NULL) {
            break;
        }
        strncpy(index_data_array[cnt++], token, LEN_32);
    }

    return 0;
}

int
get_part_index_data_one_node(const app_node_data_t *app_data, \
        const int *index_ix, int cnt, char *p_rt_data)
{
    assert(app_data != NULL && index_ix != NULL && p_rt_data != NULL);
    assert(app_data->node_status == ACTIVE && \
            app_data->exist_recent_data);

    int pos;
    int i = 0;
    const node_data_t *recent_data = &(app_data->node_data_head);
      
    char index_data_buffer[PACKAGE_SIZE] = {0};
    char index_data_array[MAX_TOTAL_INDEX_NUM][LEN_32];
    memset(index_data_array, 0, sizeof(index_data_array));

    strncpy(index_data_buffer, recent_data->data.buffer, PACKAGE_SIZE);
    split_index_data(index_data_buffer, index_data_array);

    while(i < cnt) {
        pos = index_ix[i++];
        strcat(p_rt_data, index_data_array[pos]);
        strcat(p_rt_data, TEYE_DATA_DELIMTER);
    }
    
    return 0; 
}


int
get_part_index_data_by_node_ix(const app_data_buf_t *app_data, \
        int node_ix, const int *index_ix, int cnt, char *p_rt_data)
{
    assert(app_data != NULL && index_ix != NULL && \
            p_rt_data != NULL);

    const int count = app_data -> count;
    assert(node_ix < count);

    /* nodes data head */
    app_node_data_t *nodes = app_data->data;

    /* node name, node ip */
    sprintf(p_rt_data, "%s,", nodes[node_ix].node_name);

    pthread_mutex_lock(&(nodes[node_ix].notex));
    sprintf(p_rt_data + strlen(p_rt_data), "%s,", \
            NODE_STATUS_STR[nodes[node_ix].node_status]);

    if (nodes[node_ix].node_status == ACTIVE) {
        /* have recent data */ 
        if (nodes[node_ix].exist_recent_data == 1 ) {
            get_part_index_data_one_node(nodes + node_ix, index_ix, cnt, \
                    p_rt_data + strlen(p_rt_data));
         } else { 
             /* TODO */
         }
    }

    pthread_mutex_unlock (&(nodes[node_ix].notex));
    return 0;
}


int 
get_part_index_data_by_node_name(const app_data_buf_t *app_data,
        const char *node_name, const int *index_ix, int cnt, char *p_rt_data)
{
    assert(app_data != NULL && node_name != NULL && \
            index_ix != NULL && p_rt_data != NULL); 

    int i;
    app_node_data_t *nodes = app_data->data;
    const int count = app_data->count;

    for (i = 0; i < count; i++) {
        if (strcmp(node_name, nodes[i].node_name) == 0) {
            break;
        }
    }

    /* not fount this node */
    if (i == count) {
        sprintf(p_rt_data, "%s,", node_name);
        sprintf(p_rt_data + strlen(p_rt_data), "%s", NODE_STATUS_STR[ILLEGAL]);
    } else {
        get_part_index_data_by_node_ix(app_data, i, index_ix, cnt, p_rt_data);  
    }

    return 0;
}


int 
clear_node(app_node_data_t *node) 
{
    assert (node != NULL);

    node->data_count = 0;
    node->exist_recent_data = 0;
    node_data_t *p = node->node_data_head.next;
    node_data_t *q = NULL;
    while (p != NULL) {
        q = p;
        p = p->next;
        free(q);
    }
    node->node_data_head.next = NULL;
    return 0;
}


int 
check_nodes_data_status(const app_data_buf_t * app_data)
{
    int i;
    app_node_data_t *nodes = app_data->data;
    const int count = app_data -> count;

    for (i = 0; i < count; i ++) {
        if(nodes[i].node_status == ACTIVE && \
                nodes[i].data_count <= 0) {
            return 0;
        }
    }

    return 1;
}


int 
get_nodes_data_count(const app_data_buf_t *app_data) 
{

    int i, min_data_count;
    app_node_data_t *nodes = app_data->data;
    const int count = app_data -> count;

    for (i = 0; i < count; i ++) {
        if (nodes[i].node_status == ACTIVE) {
            min_data_count=nodes[i].data_count;
            break;
        }
    }
    if (i == count) {
        return 0;
    }
    for (; i < count; i++) {
        if (nodes[i].node_status == ACTIVE) {
            min_data_count = min_data_count < nodes[i].data_count ? \
                             min_data_count : nodes [i].data_count;
//		printf("*********node_name=%s************data_count=%d**************************\n", nodes[i].node_name, nodes[i].data_count);
       }
    }
    return min_data_count;
}

