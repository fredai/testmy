
/*
 * Copyright (C) Inspur(Beijing)
 */

#include "recv.h"
#include "svr_define.h"
#include "util.h"
#include "error_handle.h"

#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


void* 
thread_recv(void * arg_recv) 
{
    LOG_START(SVR_LOG_PATH, g_app_svr_config.log_level);

    /* process argument */
    thread_arg_recv_t *arg = (thread_arg_recv_t *) arg_recv;

    app_node_data_t *node = arg->node_entry;
    const int recv_sock_fd = arg->socket_fd;

    free(arg_recv);
    arg = arg_recv = NULL;

    /* log */
    pthread_t recv_thread_id = pthread_self();
    const char *node_name = node->node_name;
    const char *node_ip = node->node_ip;

    LOG(LOG_INFO, "thread recv %lu started, node name is %s, " \
            "node ip is %s, my socket id is %d", \
            recv_thread_id, node_name, node_ip, recv_sock_fd); 

    pthread_detach(recv_thread_id);

    int ret;
    char errmsg[MAX_ERROR_MESSAGE_LENGTH];

    /* node status */
    node->node_status = ACTIVE;

    /* data buffer */
    size_t recv_len = 0;
    app_pkg_t recv_buffer;
    const app_data_t *data_body = &(recv_buffer.body);
    app_hdr_t app_response;
    memset(&app_response, 0, sizeof(app_hdr_t));
    int rerecv_times = 2;

    while (1) {
RE_RECV_DATA:
        /* receive data */
        recv_len = recv(recv_sock_fd, (void *) &recv_buffer, \
                sizeof(app_pkg_t), 0);
        if (recv_len == -1) {
            strerror_r(errno, errmsg, MAX_ERROR_MESSAGE_LENGTH);
            if (errno == EINTR) {
                LOG(LOG_ERROR, "%s: %s", node_name, "recv data error," \
                        "interupted by signal, continue recv data after" \
                        " 1 second");
                sleep(1);
                goto RE_RECV_DATA;

            } else if (errno == EAGAIN && rerecv_times > 0) {
                LOG(LOG_ERROR, "%s: recv data timeout: errno is %d %s, " \
                        "rerecv data after 1 second", node_name, \
                        errno, errmsg);
                sleep(1);
                rerecv_times--;
                goto RE_RECV_DATA;

            } else {
                LOG(LOG_FATAL, "%s: recv data error, errno is %d %s," \
                        "change node status to down and exit", node_name, \
                        errno, errmsg);
                node->node_status = DOWN;
                goto THREAD_EXIT;
            }

        } else if (recv_len == 0) {
            LOG(LOG_INFO, "connection was closed by node %s %s, " \
                    "so I will change its status to down, " \
                    "and then I will exit", node_name, node_ip);
            node->node_status = DOWN;

THREAD_EXIT:
            pthread_mutex_lock(&(node->notex));
            clear_node(node);
            pthread_mutex_unlock(&(node ->notex));
            close(recv_sock_fd);
            LOG(LOG_INFO, "thread recv %lu exited, node name is %s," \
                    " node ip is %s, my socket fd is %d", \
                    recv_thread_id, node_name, node_ip, recv_sock_fd);
            pthread_exit (NULL);
        }
        /* length of pkt */
        else if (recv_len <= sizeof(app_pkg_t)) {
            LOG(LOG_INFO, "%s: %s", node_name, "data length is pkt");
            /* response */
            app_response.data_type = RESPONSE_POST_OK;
            send(recv_sock_fd, &app_response, sizeof(app_hdr_t), MSG_NOSIGNAL);

            pthread_mutex_lock(&(node -> notex));
            if (node->data_count > MAX_NODE_DATA_COUNT) {
                LOG(LOG_ERROR, "data count for node %s is greater " \
                        "than %d, so free the data packages", node_name, MAX_NODE_DATA_COUNT);
                clear_node(node);
                pthread_mutex_unlock(&(node ->notex));
                continue;
            }

            ret = put_data_in_buffer(data_body, node);
            if (ret < 0) {
                LOG(LOG_ERROR, "%s: put data in buffer error:" \
                        "ret is %d", node_name, ret);
            }

            ret = set_node_recent_data(data_body, node);
            if (ret < 0) {
                LOG(LOG_ERROR, "%s: set node recent data error:" \
                        "ret is %d", node_name, ret);
            }
            pthread_mutex_unlock(&(node -> notex));
            LOG(LOG_DEBUG, "%s: %s", node_name, "put data in buffer" \
                    "success and set node recent data success");

        } else {
            LOG(LOG_ERROR, "%s: invalid node data lentth %d" \
                    "is neither hdr nor pkt", node_name, recv_len);
        }

        rerecv_times = 2;
    }
}

