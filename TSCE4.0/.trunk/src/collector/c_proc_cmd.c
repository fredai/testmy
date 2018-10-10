/*
 *
 * c_proc_cmd.c
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "e_define.h"
#include "e_protocol.h"
#include "c_define.h"
#include "c_config.h"
#include "c_signal.h"
#include "c_engine.h"
#include "c_debug_log.h"
#include "u_util.h"

#define CS_LOG_PATH CLR_LOG_CMD_PATH

char send_buffer [ DATA_PROTOCOL_HEAD_LEN + MAX_CMD_RES_LEN + 1 ];
extern clr_cmd_conf_t g_cmd_conf;
extern pid_t g_sct_pid;
static data_head_t cmd_data_head = {
    .data_type = 0x02,
    .data_position = 0x01,
    .data_status = 0x00,
    .exit_code = 0x00,
    .data_len = 0x00,
};


int process_cmd_main ( ) {

    cs_log_start ( g_cmd_conf.log_level );
    cs_log ( LOG_DEBUG, "Process cmd started" );

    const int cmd_timeout = g_cmd_conf.cmd_timeout;
    int recv_len = 0;
    int send_len = 0;
    int cmd_exit_code;
    char recv_buffer [ MAX_CMD_REQ_LEN + 1 ];
    char * cmd_result_buffer = send_buffer + DATA_PROTOCOL_HEAD_LEN;

    clr_cmd_signal_init ( );

    int ret = -1;
    char errmsg [ CLR_ERRMSG_LEN ];
    unsigned short int server_port;

    int server_sockfd, client_sockfd;
    int server_len, client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;

    server_port = g_cmd_conf.collecotr_cmd_port;
    cs_log ( LOG_INFO, "Process cmd listen on port %hu", server_port );

    server_sockfd = socket ( AF_INET, SOCK_STREAM, 0 );
    if ( server_sockfd == -1 ) {
        cs_log ( LOG_FATAL, "Create socket error: %d", server_sockfd );
        exit ( 1 );
    }
    else {
        cs_log ( LOG_INFO, "Create socket success" );
    }

    int opt = 0;
    ret = setsockopt ( server_sockfd, SOL_SOCKET, SO_REUSEADDR, ( char * ) & opt, sizeof ( opt ) );
    if ( ret == -1 ) {
        strerror_r ( errno, errmsg, CLR_ERRMSG_LEN );
        cs_log ( LOG_WARN, "Set sockopt error: %d | %d | %s", ret, errno, errmsg );
    }
    else {
        cs_log ( LOG_INFO, "Set sockopt success" );
    }

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons ( server_port );
    server_len = sizeof ( server_address );

    ret = bind ( server_sockfd, ( struct sockaddr * ) & server_address, server_len );
    if ( ret == -1 ) {
        strerror_r ( errno, errmsg, CLR_ERRMSG_LEN );
        cs_log ( LOG_FATAL, "Bind socket error: %d | %d | %s",
                ret, errno, errmsg );
        exit ( 2 );
    }
    else {
        cs_log ( LOG_INFO, "Bind socket success" );
    }
    
    ret = listen ( server_sockfd, 5 );
    if ( ret == -1 ) {
        strerror_r ( errno, errmsg, CLR_ERRMSG_LEN );
        cs_log ( LOG_FATAL, "Listen error: %d | %d | %s",
                ret, errno, errmsg );
        exit ( 3 );
    }
    else {
        cs_log ( LOG_INFO, "Listen success" );
    }


    cs_log ( LOG_INFO, "Start accept..." );
    client_len = sizeof ( client_address );
NETWORK_RECOVERY:
RE_ACCEPT:
    while ( 1 ) {
        client_sockfd = accept ( server_sockfd,\
            ( struct sockaddr * ) & client_address, ( socklen_t * ) & client_len );
        if ( client_sockfd == -1 ) {
            if ( errno == EINTR ) {
                cs_log ( LOG_WARN, 
                        "Accept was was interrupted by a signal, try to re accept.." );
                goto RE_ACCEPT;
            }
            else if ( errno == ECONNABORTED ) {
                cs_log ( LOG_WARN, "Connection has been aborted, try to re accept after %d seconds",
                        CLR_NETWORK_RECOVERY_RETRY_INTERVAL );
                sleep ( CLR_NETWORK_RECOVERY_RETRY_INTERVAL );
                goto  NETWORK_RECOVERY;
            }
            else if ( errno == EPERM ) {
                cs_log ( LOG_ERROR, "Firewall rules forbid connection" );
                exit ( 4 );
            }
            else {
                strerror_r ( errno, errmsg, CLR_ERRMSG_LEN );
                cs_log ( LOG_ERROR, "Accept error: %d %s", errno, errmsg );
                exit ( 5 );
            }
        }

        else {
            char client_ip_str [ MAX_IP_STR_LEN ];
            ret = ipv4_ntop ( & client_address.sin_addr, client_ip_str, MAX_IP_STR_LEN );
            if ( ret == -1 ) {
                cs_log ( LOG_INFO, "Receive a connection" );
                cs_log ( LOG_WARN, "Convert IP error" );
            }
            else {
                cs_log ( LOG_INFO, "Receive a connection form %s", client_ip_str );
            }
        }

        recv_len = recv ( client_sockfd, ( void * ) & recv_buffer, MAX_CMD_REQ_LEN, 0 );
        if ( recv_len == -1 ) {
            if ( errno == EINTR ) {
                cs_log ( LOG_ERROR, "Recv data was interupted by signal, re accept" );
                close ( client_sockfd );
                goto RE_ACCEPT;
            }
            else {
                strerror_r ( errno, errmsg, CLR_ERRMSG_LEN );
                cs_log ( LOG_INFO, "Recv data error: %d %s. - Re accept", errno, errmsg );
                close ( client_sockfd );
                goto RE_ACCEPT;
            }
        }
        else if ( recv_len == 0 ) {
            cs_log ( LOG_ERROR, "Connection was closed, re accept" );
            close ( client_sockfd );
            goto RE_ACCEPT;
        }

        recv_buffer [ recv_len ] = 0;

        if ( strcmp ( recv_buffer, RECONFIG_CMD_STR ) == 0 ) {
            kill ( g_sct_pid, SIGUSR1 );
            strcpy ( cmd_result_buffer, RECONFIG_RESULT_STR );
            cmd_data_head.data_status = 0x01;
            cmd_data_head.exit_code = 0;
            cmd_data_head.data_len = strlen ( RECONFIG_RESULT_STR ) + 1;
            goto SND_RECONFIG;
        }

        bzero ( send_buffer, sizeof ( send_buffer ) );

        ret = c_execute ( recv_buffer, cmd_result_buffer, MAX_CMD_RES_LEN,
                & cmd_exit_code, cmd_timeout );
        cs_log ( LOG_DEBUG, "Execute cmd %s return %d", recv_buffer, ret );
        if ( ret == -6 || ret == -7 ) { /*timeout*/
            cs_log ( LOG_DEBUG, "Execute cmd %s time out", recv_buffer );
            cmd_data_head.data_status = 0x02;
            cmd_data_head.exit_code = 0x00;
            cmd_data_head.data_len = 0x00;
        }

        else if ( ret == -5 ) { /*too long*/
            cs_log ( LOG_DEBUG, "Execute cmd %s result too long", recv_buffer );
            cmd_data_head.data_status = 0x03;
            cmd_data_head.exit_code = 0x00;
            cmd_data_head.data_len = 0x00;
        }

        else if ( ret > 0 ) { /*nornal*/
            cs_log ( LOG_DEBUG, "Execute cmd %s normal", recv_buffer );
            cmd_result_buffer [ ret ] = 0;
            cmd_data_head.data_status = 0x01;
            cmd_data_head.exit_code = cmd_exit_code;
            cmd_data_head.data_len = ret + 1; /*\0*/
        }

        else {
            cs_log ( LOG_ERROR, "Execute cmd %s error: %d", recv_buffer, ret );
            close ( client_sockfd );
            continue;
        }

SND_RECONFIG:

        memcpy ( send_buffer, & cmd_data_head, DATA_PROTOCOL_HEAD_LEN );
        send_len = DATA_PROTOCOL_HEAD_LEN + cmd_data_head.data_len;

        int send_retry_times = 0;
 
RE_SEND:
        ret = send ( client_sockfd, send_buffer, send_len, MSG_NOSIGNAL );
        cs_log ( LOG_DEBUG, "Send data len %d return %d", send_len, ret ); 

        if ( ret == -1 ) {

            strerror_r ( errno, errmsg, CLR_ERRMSG_LEN );
            cs_log ( LOG_ERROR, "Send data fail: %d %s", errno, errmsg );

            switch ( errno ) {
                case ECONNRESET:
                case ENOTCONN:
                    close ( client_sockfd );
                    u_sleep ( 1 );
                    goto RE_ACCEPT;
                case EINTR:
                case EPIPE:
                default:
                    if ( send_retry_times < 3 ) {
                        send_retry_times ++;
                        goto RE_SEND;
                    }
            }

        }

        else if ( ret == send_len ) {
            cs_log ( LOG_INFO, "Send data success" );
        }

        close ( client_sockfd );

    } /* while ( 1 )*/

    close ( server_sockfd );

    exit ( 0 );

}







/*end of file*/
