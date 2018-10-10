/*
 *
 * d_service.c
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "e_define.h"
#include "d_define.h"
#include "d_config.h"
#include "d_recv.h"
#include "u_mutex.h"
#include "u_util.h"
#include "u_log.h"


#define SERVER_SOCKET_LISTEN_LEN 30
#define NETWORK_RECOVERY_RETRY_INTERVAL 5

extern collectord_config_t g_collectord_config;
extern pthread_mutex_t mutex_g_collectord_config;
extern node_list_t * g_node_list;
extern int mutex_g_node_list;
extern int startup_status [ 3 ];
extern pthread_mutex_t mutex_startup_status;


void * thread_service ( void * arg_service ) {

    LOG_START ( CLRD_LOG_PATH, g_collectord_config.log_level );
    LOG ( LOG_INFO, "%s", "Thread service started" );

    int ret = -1;
    int is_start = 1;
    char errmsg [ CLRD_ERRMSG_LEN ];

    char server_ip [ MAX_IP_STR_LEN ];
    unsigned short int server_port;

    pthread_mutex_lock ( & mutex_g_collectord_config );
    ret = collectord_config_get_server_ip_port ( & g_collectord_config, server_ip, & server_port );
    pthread_mutex_unlock ( & mutex_g_collectord_config );
    if ( ret < 0 ) {
        LOG ( LOG_FATAL, "%s", "Get server IP and port config error" );
        fprintf ( stderr, "%s\n", "Get server IP and port config error" );
        exit ( 171 );
    }
    else {
        LOG ( LOG_INFO, "%s", "Get server IP port config success" );
    }

    int server_sockfd, client_sockfd;
    int server_len, client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;

    server_sockfd = socket ( AF_INET, SOCK_STREAM, 0 );
    if ( server_sockfd == -1 ) {
        LOG ( LOG_FATAL, "Create socket fd error: return %d", server_sockfd );
        fprintf ( stderr, "%s\n", "Create socket fd error" );
        exit ( 172 );
    }
    else {
        LOG ( LOG_INFO, "%s", "Create socket success" );
    }

    int opt = 0;
    ret = setsockopt ( server_sockfd, SOL_SOCKET, SO_REUSEADDR, ( char * ) & opt, sizeof ( opt ) );
    if ( ret == -1 ) {
        strerror_r ( errno, errmsg, CLRD_ERRMSG_LEN );
        LOG ( LOG_WARN, "Set sockopt error: return %d, errno is %d %s", ret, errno, errmsg );
    }
    else {
        LOG ( LOG_INFO, "%s", "Set sockopt success" );
    }

    server_address.sin_family = AF_INET;
    ret = ipv4_pton ( server_ip, & ( server_address.sin_addr ) );
    if ( ret == -1 ) {
        strerror_r ( errno, errmsg, CLRD_ERRMSG_LEN );
        LOG ( LOG_ERROR, "Convert IP str to sin_addr error: return %d | errno %d | %s",
                ret, errno, errmsg );
    }
    server_address.sin_port = htons ( server_port );
    server_len = sizeof ( server_address );
    ret = bind ( server_sockfd, ( struct sockaddr * ) & server_address, server_len );
    if ( ret == -1 ) {
        strerror_r ( errno, errmsg, CLRD_ERRMSG_LEN );
        LOG ( LOG_FATAL, "Bind socket and server address error: return %d | errno %d | %s",
                ret, errno, errmsg );
        fprintf ( stderr, "Bind socket and server address error: %d %s\n\n", errno, errmsg );
        exit ( 173 );
    }
    else {
        LOG ( LOG_INFO, "%s", "Bind socket and server address success" );
    }
    
    
    ret = listen ( server_sockfd, SERVER_SOCKET_LISTEN_LEN );
    if ( ret == -1 ) {
        strerror_r ( errno, errmsg, CLRD_ERRMSG_LEN );
        LOG ( LOG_FATAL, "Listen socket error: return %d | errno %d | %s",
                ret, errno, errmsg );
        fprintf ( stderr, "Listen socket error: %d %s\n", errno, errmsg );
        exit ( 174 );
    }
    else {
        LOG ( LOG_INFO, "%s", "Listen socket success" );
    }

    if ( is_start == 1 ) {
        pthread_mutex_lock ( & mutex_startup_status );
        startup_status [ 1 ] = 1;
        pthread_mutex_unlock ( & mutex_startup_status );
        is_start = 0;
    }

    client_len = sizeof ( client_address );

    LOG ( LOG_INFO, "%s", "Start socket accept..." );
    
NETWORK_RECOVERY:
RE_ACCEPT:

    while ( 1 ) {
      
        client_sockfd = accept ( server_sockfd,\
            ( struct sockaddr * ) & client_address, ( socklen_t * ) & client_len );
	    
        if ( client_sockfd == -1 ) {

            if ( errno == EINTR ) {
                LOG ( LOG_WARN, "%s", "Socket accept was was interrupted by a signal, try to re accept.." );
                goto RE_ACCEPT;
            }
            else if ( errno == ECONNABORTED ) {
                LOG ( LOG_WARN, "Connection has been aborted, try to re accept after %d seconds",
                        NETWORK_RECOVERY_RETRY_INTERVAL );
                sleep ( NETWORK_RECOVERY_RETRY_INTERVAL );
                goto  NETWORK_RECOVERY;
            }
            else if ( errno == EPERM ) {
                LOG ( LOG_ERROR, "%s", "Firewall rules forbid connection" );
                fprintf ( stderr, "%s\n", "Firewall rules forbid connection" );
                exit ( 175 );
            }
            else {
                strerror_r ( errno, errmsg, CLRD_ERRMSG_LEN );
                LOG ( LOG_ERROR, "Accept error: errno is %d %s", errno, errmsg );
                fprintf ( stderr, "Accept error: %d %s\n", errno, errmsg );
                exit ( 176 );
            }

        }
        
        char client_ip_str [ MAX_IP_STR_LEN ];
        ret = ipv4_ntop ( & client_address.sin_addr, client_ip_str, MAX_IP_STR_LEN );
        LOG ( LOG_INFO, "Receive a connection from %s", client_ip_str );

        unsigned int node_ip;
        sin_addr_to_uint ( & client_address, & node_ip );

        u_mutex_lock ( mutex_g_node_list );
        ret = nl_find_node_by_nd_ip ( g_node_list, node_ip );
        u_mutex_unlock ( mutex_g_node_list );

        if ( ret < 0 ) {
            LOG ( LOG_ERROR, "NL find node by IP error: %d", ret );
            close ( client_sockfd );
            continue;
        }

        else if ( ret == 0 ) {
            LOG ( LOG_WARN, "Node %s was not found in the node list", client_ip_str );
            close ( client_sockfd );
            continue;
        }
	
        LOG ( LOG_INFO, "Node %s was found in the node list, node id is %d", client_ip_str, ret );

        pthread_t tid_recv;
        u_mutex_lock ( mutex_g_node_list );
        nl_get_node_threadid_by_nd_id ( g_node_list, ret, & tid_recv );
        u_mutex_unlock ( mutex_g_node_list );
        if ( tid_recv != 0 ) {
            LOG ( LOG_ERROR, "Another thread %lu is running for this node %s", \
                    tid_recv, client_ip_str );
            close ( client_sockfd );
            continue;
        }

        thread_arg_recv_t * arg_recv = ( thread_arg_recv_t * ) malloc ( sizeof ( thread_arg_recv_t ) );
        if ( arg_recv == NULL ) {
            LOG ( LOG_ERROR, "Malloc argument for node %s error", client_ip_str );
            close ( client_sockfd );
            continue;
        }

        arg_recv -> uint_ip = node_ip;
        arg_recv -> socket_fd = client_sockfd;

        LOG ( LOG_INFO, "Create thread for node %s", client_ip_str );

        ret = pthread_create ( & tid_recv, NULL, thread_recv, ( void * ) arg_recv );
        if ( ret != 0 ) {
            LOG ( LOG_ERROR, "Create thread for node %s error", client_ip_str );
            free ( arg_recv );
            close ( client_sockfd );
            continue;
        }

        LOG ( LOG_INFO, "Create thread for node %s success", client_ip_str );


    } /* while ( 1 ) */


    close ( server_sockfd );
    close ( client_sockfd );

    LOG ( LOG_INFO, "%s", "Thread service end" );
    LOG_END;
    pthread_exit ( NULL );

}


/*end of file*/
