
/*
 * Copyright (C) Inspur(Beijing)
 */

#include "rtd.h"
#include "svr_define.h"
#include "buffer.h"
#include "svr_config.h"
#include "util.h"
#include "error_handle.h"

#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define MAX_RTD_REQUEST 10


/* num of thread rtd */
int g_rtd_num;
/* mutex for num of thread rtd */
pthread_mutex_t g_rtd_num_mutex;


static void parse_request_nodelist_name(char *name_buffer, \
        char (*nodelist)[MAX_NODE_NAME_LEN], int max_node_cnt);
static void parse_request_indexlist_name(char *index_buffer, \
        char (*index_list)[LEN_32], int max_index_cnt);
static void parse_request_data_package(char *request_body, \
        char (*node_name_list)[MAX_NODE_NAME_LEN], int max_node_cnt, \
        char (*index_name_list)[LEN_32], int max_index_cnt);
static int is_get_all_nodes(char (*node_name_list)[MAX_NODE_NAME_LEN]);
static int is_get_all_index(char (*index_name_list)[LEN_32]);
static void send_all_nodes_all_index_data(int socket_fd);
static void send_all_nodes_part_index_data(char \
        (*index_name_list)[LEN_32], int socket_fd);
static void send_part_nodes_all_index_data(char \
        (*node_name_list)[MAX_NODE_NAME_LEN], int socket_fd) ;
static void send_part_nodes_part_index_data(char \
        (*node_name_list)[MAX_NODE_NAME_LEN], \
        char (*index_name_list)[LEN_32], int socket_fd);
static void handle_get_status(char *response_buffer, int socket_fd);
static void handle_exit(char *response_buffer, int socket_fd);
static void handle_request_data(char *request_package, int socket_fd);
static void send_request_data(int socket_fd, const char *send_buffer);
static void finish_thread(int fd, pthread_t thread_id, \
        const char *client_ip);


void* 
thread_rtd(void *arg_rtd)
{
    LOG_START(SVR_LOG_PATH, g_app_svr_config.log_level);

    /* process arguments */
    thread_arg_rtd_t * arg = (thread_arg_rtd_t *) arg_rtd;
    const int socket_fd = arg->socket_fd;

    char client_ip[MAX_IP_STR_LEN];
    strcpy(client_ip, arg->client_ip);

    free(arg_rtd);
    arg = arg_rtd = NULL;

    /* log */
    pthread_t recv_thread_id = pthread_self();
    LOG (LOG_INFO, "thread rtd %lu started, client ip is %s," \
            "my socket id is %d", recv_thread_id, client_ip, socket_fd);

    pthread_detach(pthread_self());

    /* FIXME limit num of threads */
    pthread_mutex_lock(&g_rtd_num_mutex);
    if (g_rtd_num >= MAX_RTD_REQUEST) {
        pthread_mutex_unlock(&g_rtd_num_mutex);
        LOG(LOG_ERROR, "thread rtd num > %d, so exit", MAX_RTD_REQUEST);
        close(socket_fd);
        pthread_exit(NULL);
    } else {
        g_rtd_num++;
        pthread_mutex_unlock(&g_rtd_num_mutex);
    }

    int ret;
    char errmsg[MAX_ERROR_MESSAGE_LENGTH];
    char response_buffer[MAX_GET_DATA_LEN];

    /* set sock opt */
    ret = set_socket_options(socket_fd, 0, 5, 2*60*60, \
            errmsg, MAX_ERROR_MESSAGE_LENGTH);
    if (ret < 0) {
        LOG(LOG_WARN, "set sockopt rtd error, return %d, %s", ret, errmsg);
    } else {
        LOG(LOG_DEBUG, "%s", "set sockopt rtd success");
    }

    /* data buffer */
    size_t recv_len = 0;
    app_hdr_t *rep_head = (app_hdr_t*) response_buffer;
    char * rep_body = response_buffer + sizeof(app_hdr_t);
    char * recv_buffer = response_buffer;
    const app_hdr_t *recv_head = (app_hdr_t *) response_buffer;

    while (1) {
        /* receive request */
        recv_len = recv(socket_fd, (void *)recv_buffer, \
                sizeof(app_pkg_t), 0);

        if (recv_len == -1) {
            strerror_r(errno, errmsg, MAX_ERROR_MESSAGE_LENGTH);
            LOG(LOG_ERROR, "recv data error, errno is %d %s", errno, errmsg);
            finish_thread(socket_fd, recv_thread_id, client_ip);

        } else if (recv_len == 0 ) {
            LOG(LOG_INFO, "connection was closed by node %s", client_ip);
            finish_thread(socket_fd, recv_thread_id, client_ip);

        } else if (recv_len == sizeof(app_hdr_t)) {
            LOG (LOG_INFO, "%s", "receive data len is hdr");

            if (recv_head -> data_type == REQUEST_GET_STATUS) {
                /* for get status command */
                handle_get_status(response_buffer, socket_fd);
            } else if (recv_head->data_type == REQUEST_GET_EXIT) {
                /* for exit */
                handle_exit(response_buffer, socket_fd);
            } else {
                LOG(LOG_ERROR, "invalid request for node %s," \
                        "unknown request type", client_ip);
                finish_thread(socket_fd, recv_thread_id, client_ip);
            }

        } else {
            if (recv_head -> data_type == REQUEST_GET_DATA) {
              /* for reqeust data */
              if (recv_len - sizeof(app_hdr_t) != rep_head -> len || \
                      rep_head -> len != strlen( (char*) rep_body) + 1) {
                  LOG(LOG_ERROR, "%s", "invalid request package");
                  finish_thread(socket_fd, recv_thread_id, client_ip);
              } 
              handle_request_data(response_buffer, socket_fd);

            } else {
                LOG (LOG_ERROR, "invalid request: " \
                        "length of request is not hdr, " \
                        "and request type is not get data " \
                        "for one node %s", client_ip );
                finish_thread(socket_fd, recv_thread_id, client_ip);
            }
        }
        finish_thread(socket_fd, recv_thread_id, client_ip);
    } 
}


static void
parse_request_nodelist_name(char *name_buffer, \
        char (*nodelist)[MAX_NODE_NAME_LEN], int max_node_cnt)
{
    assert(nodelist != NULL);
    
    char *str, *token, *saveptr;
    int i;

    str = name_buffer;
    for (i = 0; i < max_node_cnt; str = NULL, i++) {
        token = strtok_r(str, TEYE_GET_DATA_ITEM_DELIMTER, &saveptr);
        if (token == NULL) {
            break;
        }
        strncpy(nodelist[i], token, MAX_NODE_NAME_LEN);
    }
}


static void
parse_request_indexlist_name(char *index_buffer, \
        char (*index_list)[LEN_32], int max_index_cnt)
{
    assert(index_buffer != NULL && index_list != NULL);

    char *str, *token, *saveptr;
    int i;

    str = index_buffer;
    for (i = 0; i < max_index_cnt; str = NULL, i++) {
       token = strtok_r(str, TEYE_GET_DATA_ITEM_DELIMTER, &saveptr); 
       if (token == NULL) {
           break;
       }
       strncpy(index_list[i], token, LEN_32);
    } 
}


static void
parse_request_data_package(char *request_body, \
        char (*node_name_list)[MAX_NODE_NAME_LEN], int max_node_cnt,
        char (*index_name_list)[LEN_32], int max_index_cnt)
{
    assert(request_body != NULL && node_name_list != NULL \
            && index_name_list != NULL);

    int i;
    char *node_str, *index_str;
   
    for (i = 0; i < strlen(request_body) && \
            request_body[i] != TEYE_GET_DATA_NODE_INDEX_DELIMTER[0]; i++) {
        ;   /* NULL */
    }

    request_body[i] = '\0';
    node_str = request_body;
    index_str = request_body + i + 1;

    /* get node name list */ 
    parse_request_nodelist_name(node_str, node_name_list, max_node_cnt); 

    /* get index name list */
    parse_request_indexlist_name(index_str, index_name_list, max_index_cnt);

    return;
}


static int
is_get_all_nodes(char (*node_name_list)[MAX_NODE_NAME_LEN])
{
    assert(node_name_list != NULL);
    return node_name_list[0][0] == '\0';
}


static int
is_get_all_index(char (*index_name_list)[LEN_32])
{
    assert(index_name_list != NULL);
    return index_name_list[0][0] == '\0';
}


static void
send_all_nodes_all_index_data(int socket_fd)
{

    char send_buffer[PACKAGE_SIZE] = {0};
    const int count = g_data_buffer.count;

    int ix = -1;
    while (++ix < count) {
        get_all_index_data_by_node_ix(&g_data_buffer, ix, send_buffer);
        send_request_data(socket_fd, send_buffer);
    }
}


static void
send_all_nodes_part_index_data(char (*index_name_list)[LEN_32], \
        int socket_fd)
{
    assert(index_name_list != NULL);

    int index_ix[MAX_TOTAL_INDEX_NUM];
    char send_buffer[PACKAGE_SIZE] = {0};
    const int count = g_data_buffer.count;

    extern config_mod_info_t g_mod_info_head;
    int cnt = get_index_ix_by_name(&g_mod_info_head, \
            index_name_list, index_ix);

    int ix = 0;
    while (ix < count) {
        get_part_index_data_by_node_ix(&g_data_buffer, ix++, \
                index_ix, cnt, send_buffer);
        send_request_data(socket_fd, send_buffer);
    }

}


static void
send_part_nodes_all_index_data(char (*node_name_list)[MAX_NODE_NAME_LEN], \
        int socket_fd) 
{
    assert (node_name_list != NULL);

    int i = 0;
    char send_buffer[PACKAGE_SIZE];

    while (strlen(node_name_list[i]) > 0) {
        memset(send_buffer, 0, sizeof(send_buffer));
        get_all_index_data_by_node_name(&g_data_buffer, node_name_list[i++], \
                send_buffer);
        send_request_data(socket_fd, send_buffer);
    }
}



int
get_index_ix_by_name(const config_mod_info_t *g_mod_info, \
        char (*index_name)[LEN_32], int *index_ix)
{
    assert(g_mod_info != NULL && index_name != NULL && index_ix != NULL);

    int i, j;
    const config_mod_info_t *p;
    const config_mod_info_item_t  *t;
    int curr_ix = 0, cnt = 0;

    extern app_svr_config_t g_app_svr_config;
    monitor_list_t *monitor_list = &(g_app_svr_config.monitor_list);
    const int  monitor_count = monitor_list->count;
    cf_monitor_t *cf_monitor = monitor_list->head.next;

    
    
NEXT_INDEX:
    while (index_name[cnt][0] != '\0') {
        p = g_mod_info;
        curr_ix = 0;

        while ( (p = p->next) != NULL) {
            t = &(p->info);

                cf_monitor = monitor_list->head.next;
                for (j = 0; j < monitor_count; j++) {

                        if(0==strcmp(t->mod_name, cf_monitor -> monitor_name)) {
                                for (i = 0; i < t->index_cnt; i++) {
                                        if (strcmp(index_name[cnt], t->index_info[i].index_name) == 0) {
                                                index_ix[cnt++] = curr_ix + i;
                                                 goto NEXT_INDEX;
                                        }
                                }
                                curr_ix += t->index_cnt;
                        }

                        cf_monitor = cf_monitor -> next;
                }


        }
    }

    return cnt;
}



static void
send_part_nodes_part_index_data(char (*node_name_list)[MAX_NODE_NAME_LEN], \
        char (*index_name_list)[LEN_32], int socket_fd)
{
    assert(node_name_list != NULL && index_name_list != NULL);
   
    int i = 0, cnt; 
    int index_ix[MAX_TOTAL_INDEX_NUM];
    char send_buffer[PACKAGE_SIZE];

    /* 
     * use global varible 
     * I hope have better way
     */
    extern config_mod_info_t g_mod_info_head;
    cnt = get_index_ix_by_name(&g_mod_info_head, \
            index_name_list, index_ix);

    while (strlen(node_name_list[i]) > 0) {
        get_part_index_data_by_node_name(&g_data_buffer, node_name_list[i++], \
                index_ix, cnt, send_buffer);
        send_request_data(socket_fd, send_buffer);
    }
}


static void
handle_get_status(char *response_buffer, int socket_fd)
{ 
    LOG_START(SVR_LOG_PATH, g_app_svr_config.log_level);
    LOG(LOG_INFO, "%s", "request get status");

    app_hdr_t *rep_head = (app_hdr_t *) response_buffer;
    rep_head->data_type = RESPONSE_GET_STATUS_OK;

    /* response status ok */
    LOG (LOG_DEBUG, "%s", "response status ok" );
    send(socket_fd, response_buffer, \
            sizeof(app_hdr_t), MSG_NOSIGNAL);
}


static void
handle_exit(char *response_buffer, int socket_fd)
{
    LOG_START(SVR_LOG_PATH, g_app_svr_config.log_level);
    LOG(LOG_INFO, "%s", "request get exit");

    app_hdr_t *rep_head = (app_hdr_t *) response_buffer;
    rep_head->data_type = RESPONSE_GET_EXIT;
    rep_head->len = (unsigned int)getpid();

    /* response pid */
    LOG(LOG_DEBUG, "response pid %u", rep_head->len);
    send(socket_fd, response_buffer, sizeof(app_hdr_t), MSG_NOSIGNAL);
    LOG(LOG_INFO, "process %u is exiting", rep_head->len);
    exit(301);
}


static void
handle_request_data(char *request_package, int socket_fd)
{
    LOG_START(SVR_LOG_PATH, g_app_svr_config.log_level);
    LOG (LOG_INFO, "%s", "request get data");

    char node_name_list[MAX_NODE_NUM][MAX_NODE_NAME_LEN];
    char index_name_list[MAX_TOTAL_INDEX_NUM][LEN_32];
    memset(node_name_list, 0, sizeof(node_name_list));
    memset(index_name_list, 0, sizeof(index_name_list));

    char *recv_body = request_package + sizeof(app_hdr_t);
    parse_request_data_package(recv_body, node_name_list, \
            MAX_NODE_NUM, index_name_list, MAX_TOTAL_INDEX_NUM);

    if (is_get_all_nodes(node_name_list)) { 
        if (is_get_all_index(index_name_list)) {
            send_all_nodes_all_index_data(socket_fd);
        } else { 
            send_all_nodes_part_index_data(index_name_list, socket_fd);
        }

    } else {
        if (is_get_all_index(index_name_list)) {
            send_part_nodes_all_index_data(node_name_list, socket_fd);
        } else {
            send_part_nodes_part_index_data(node_name_list, \
                    index_name_list, socket_fd);
        }
    }
}


static void
send_request_data(int socket_fd, const char *send_buffer)
{
    assert(send_buffer != NULL);

    LOG_START(SVR_LOG_PATH, g_app_svr_config.log_level);
    int ret;

    ret = send(socket_fd, send_buffer, strlen(send_buffer), MSG_NOSIGNAL);
    if (ret != strlen(send_buffer)) {
        char errmsg[MAX_ERROR_MESSAGE_LENGTH];
        strerror_r(errno, errmsg, MAX_ERROR_MESSAGE_LENGTH);
        LOG(LOG_ERROR, "send data error return %d %d %s", ret, errno, errmsg);
//        finish_thread();
    } else {
        //LOG(LOG_INFO, "response real time data for node %s", client_ip);
    }

    /* wait client's response */
    app_hdr_t t;
    ret = recv(socket_fd, &t, sizeof(t), 0);
    if (ret != sizeof(t) || t.data_type != RESPONSE_GET_DATA) {
        LOG(LOG_ERROR, "get response from getdata error: %d", ret);
    } else {
        LOG(LOG_INFO, "get response from getdata success: %d", ret);
    }
} 


static void
finish_thread(int fd, pthread_t thread_id, const char *client_ip)
{
    LOG_START(SVR_LOG_PATH, g_app_svr_config.log_level);

    close(fd);
    LOG(LOG_INFO, "thread rtd %lu exited, client ip is %s," \
            "my socket id is %d", thread_id, client_ip, fd);
    pthread_mutex_lock(&g_rtd_num_mutex);
    g_rtd_num--;
    pthread_mutex_unlock(&g_rtd_num_mutex);
    pthread_exit(NULL);
}

