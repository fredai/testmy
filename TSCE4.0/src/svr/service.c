
/*
 * Copyright (C) Inspur(Beijing)
 */


#include "service.h"
#include "svr_define.h"
#include "rtd.h"
#include "recv.h"
#include "util.h"
#include "error_handle.h"

#include <sys/types.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>


int g_sockfd;


int 
init_sockfd(const app_svr_config_t *app_svr_config) 
{
    assert(app_svr_config != NULL);

    /* network */
    int server_sockfd;
    int server_len;
    struct sockaddr_in server_address;
    int noptval;

    /* config */
    const char *server_ip = app_svr_config->svr_config.svr_ip;
    const unsigned short int server_port = app_svr_config-> \
                                           svr_config.svr_port;

    int ret;

    /* socket */
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sockfd == -1) {
        return -1;
    } else {
        g_sockfd = server_sockfd;
    }

    noptval = 1;
    if (setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, \
                &noptval, sizeof(noptval)) < 0) {
        err_sys("setsockopt error");
    }

    /* server address */
    server_address.sin_family = AF_INET;
    ret = ipv4_pton(server_ip, &(server_address.sin_addr));
    if (ret == -1) {
        err_sys("convert ip %s str to error", server_ip);
    }
    server_address.sin_port = htons(server_port);
    server_len = sizeof(server_address);

    /* bind */
    int bind_times = 30;
REBIND:
    ret = bind(server_sockfd, (struct sockaddr *) \
            &server_address, server_len);
    if (ret == -1) {
        err_sys("bind socket to address %s:%d error:", \
                server_ip, server_port);
        usleep(500*1000);
        if (bind_times -- > 0) {
            goto REBIND;
        } else {
            return -1;
        }
    }
    return 0;
}


int 
app_service(const app_svr_config_t *app_svr_config) 
{
    assert(app_svr_config != NULL);

    /* network */
    int client_sockfd;
    int client_len;
    struct sockaddr_in client_address;
    int server_sockfd = g_sockfd; 

    /* config */
    const app_interval_t interval = app_svr_config->itvl;
    const int recv_timeout = interval * 2 > APP_INTERVAL_INCREMENTAL ? \
                             interval * 2 : APP_INTERVAL_INCREMENTAL;

    int ret;
    char errmsg[MAX_ERROR_MESSAGE_LENGTH];
    LOG_START(SVR_LOG_PATH, app_svr_config->log_level);

    /* listen */
    ret = listen(server_sockfd, SERVER_SOCKET_LISTEN_LEN);
    if (ret == -1) {
        strerror_r(errno, errmsg, MAX_ERROR_MESSAGE_LENGTH);
        LOG(LOG_ERROR, "socket listen error: return %d, errno is %d %s",
                ret, errno, errmsg);
        return -1;
    } else {
        LOG(LOG_DEBUG, "%s", "socket listen success");
    }


    /* prepare vaiables */
    client_len = sizeof(client_address);
    LOG (LOG_DEBUG, "%s", "start socket accept...");

    int recovery_times = NETWORK_RECOVERY_RETRY_TIMES;
NETWORK_RECOVERY:
    /* retry */
    if (--recovery_times == 0) {
        LOG (LOG_ERROR, "accept error: have retry %d times", \
                NETWORK_RECOVERY_RETRY_TIMES);
        return -1;
    }

RE_ACCEPT:
    while (1) {
        /* accept */
		printf("success_0\n");
        client_sockfd = accept(server_sockfd, (struct sockaddr *) \
                &client_address, (socklen_t *) &client_len);
        if (client_sockfd == -1) {
            if (errno == EINTR) {
                LOG (LOG_WARN, "%s", "socket accept was interrupted" \
                        "by a signal, try to re accept..");
                goto RE_ACCEPT;
            } else if (errno == ECONNABORTED) {
                LOG(LOG_WARN, "connection has been aborted," \
                        "try to re accept after %d seconds",
                        NETWORK_RECOVERY_RETRY_INTERVAL);
                sleep(NETWORK_RECOVERY_RETRY_INTERVAL);
                goto  NETWORK_RECOVERY;
            } else if ( errno == EPERM ) {
                LOG(LOG_ERROR, "%s", "Firewall rules forbid connection");
                return -1;
            } else {
                strerror_r(errno, errmsg, MAX_ERROR_MESSAGE_LENGTH);
                LOG(LOG_ERROR, "accept error: errno is %d %s", \
                        errno, errmsg );
                return -1;
            }
        }

        /* log client ip */
        char client_ip_str[MAX_IP_STR_LEN];
        ret = ipv4_ntop(&client_address.sin_addr, \
                client_ip_str, MAX_IP_STR_LEN);
        LOG(LOG_INFO, "receive a connection, " \
                "and the request ip is %s", client_ip_str);

        /* set sock opt */
		printf("success_1\n");
        ret = set_socket_options(client_sockfd, 0, SVR_SEND_TIMEOUT, \
                recv_timeout, errmsg, MAX_ERROR_MESSAGE_LENGTH);
        if (ret < 0) {
            LOG(LOG_WARN, "setsockopt clientfd error: ret = %d, %s", \
                    ret, errmsg);
        } else {
            LOG(LOG_DEBUG, "%s", "setsockopt clientfd success");
        }

		printf("success_2\n");

        LOG(LOG_INFO, "%s", "start handshake");
        /* variable for handshake */
        char recv_buf[sizeof(app_hdr_t) + MAX_NODE_NAME_LEN + 32];
        app_hdr_t *service_head = (app_hdr_t*) recv_buf;

        int recv_retry_count = RECV_RETRY_TIMES;
RE_RECV_DATA:
        /* retry */
        if (--recv_retry_count < 0) {
            LOG(LOG_ERROR, "recv error: have retry %d times", \
                    RECV_RETRY_TIMES);
            return -1;
        }
        /* recv */
        int recv_len = recv(client_sockfd, (void *) recv_buf, \
                sizeof (recv_buf), 0);
        if (recv_len == -1) {
            strerror_r(errno, errmsg, MAX_ERROR_MESSAGE_LENGTH);
            if (errno == EINTR) {
                LOG (LOG_ERROR, "%s", \
                        "recv data error: interupted by signal" \
                        "continue recv data after 1 second");
                sleep(1);
                goto RE_RECV_DATA;
            } else {
                LOG (LOG_FATAL, "recv data error: errno is %d %s", \
                        errno, errmsg);
                return -1;
            }
        } else { 
            recv_retry_count = 0;
        }
        /* post handshake */
        if (service_head->request_type == REQUEST_TYPE_POST && \
                IS_HELO(*service_head)) {
            ip_t node_ip;
            hostname_to_ip(recv_buf + sizeof(app_hdr_t), &node_ip);
            LOG (LOG_INFO, "request type is post, and client " \
                    "host name is %s", recv_buf + sizeof(app_hdr_t));

            app_node_data_t *node = get_node_in_list(&g_data_buffer, node_ip);
            if (node == NULL) { 
                LOG(LOG_ERROR, "node %s is not in the node list", client_ip_str);
                close(client_sockfd);
                continue;
            }
            if (node->node_status == ACTIVE) { 
                LOG (LOG_ERROR, "the connection for node %s is active, " \
                        "so close the new connection", client_ip_str);
                close(client_sockfd);
                continue;
            }

            /* response */
            SET_OK(*service_head);
            /* send */
            send(client_sockfd, service_head, sizeof(app_hdr_t), MSG_NOSIGNAL);
            LOG (LOG_INFO, "%s", "handshake ok");
            /* handshake finish */

            /* prepare argument for thread recv */
            pthread_t tid_recv;
            thread_arg_recv_t *arg_recv = (thread_arg_recv_t *) \
                                          malloc(sizeof(thread_arg_recv_t));
            if (arg_recv == NULL) {
                LOG(LOG_ERROR, "malloc error", client_ip_str);
                close(client_sockfd);
                continue;
            }
            arg_recv->node_entry = node;
            arg_recv->socket_fd = client_sockfd;

            /* create thread recv */
            LOG(LOG_INFO, "create receive thread for node %s", client_ip_str);
            ret = pthread_create(&tid_recv, NULL, thread_recv, \
                    (void *)arg_recv);
            if (ret != 0) {
                LOG(LOG_ERROR, "create thread receive for node %s error", \
                        client_ip_str );
                free(arg_recv);
                close(client_sockfd);
                continue;
            }
            LOG(LOG_INFO, "create thread receive for node %s success", \
                    client_ip_str );
        }
        /* post handshake */

        if (service_head->request_type == REQUEST_TYPE_USER && \
                IS_HELO(*service_head)) {
			printf("------%x---hello im one of users im %d-----\n", service_head->request_type, client_sockfd);
        }

        /* request get data */
        else if (service_head->request_type == REQUEST_TYPE_GET && \
                IS_HELO(*service_head)) {

            LOG(LOG_INFO, "%s", "request type is get");
            /* response */
            SET_OK (*service_head);
            send(client_sockfd, service_head, sizeof(app_hdr_t), MSG_NOSIGNAL);
            LOG(LOG_INFO, "%s", "handshake ok");
            /* prepare argument for thread rtd */
            pthread_t tid_rtd;
            thread_arg_rtd_t *arg_rtd = (thread_arg_rtd_t *) \
                                        malloc(sizeof(thread_arg_rtd_t));
            if (arg_rtd == NULL) {
                LOG(LOG_ERROR, "%s", "malloc arg for thread rrd error");
                close(client_sockfd);
                continue; 
            }
            strcpy(arg_rtd->client_ip, client_ip_str);
            arg_rtd->socket_fd = client_sockfd;

            /* create thread recv */
            LOG(LOG_INFO, "create thread rtd for node %s", client_ip_str);
            ret = pthread_create(&tid_rtd, NULL, thread_rtd, (void *)arg_rtd);
            if (ret != 0) {
                LOG(LOG_ERROR, "%s", "create thread rtd error");
                free(arg_rtd);
                close(client_sockfd);
                continue;
            }
            LOG ( LOG_INFO, "create thread rtd for ip %s success", client_ip_str );

        } else {
            LOG ( LOG_FATAL, "%s", "handshake request type is neither post nor get" );
        }
    }

    close(server_sockfd);
    return 0;
}


void 
cleanup_resource(int signum) {
    if (g_sockfd != 0) {
        close(g_sockfd);
    }
    exit (201);
}


int 
register_signal_usr1() 
{
    struct sigaction sa_usr1;
    sa_usr1.sa_handler = cleanup_resource;
    sigemptyset(&sa_usr1.sa_mask);
    sa_usr1.sa_flags = 0;
    int ret = sigaction(SIGUSR1, & sa_usr1, NULL);
    if (ret == -1) {
        return -1;
    }
    return 0;
}
