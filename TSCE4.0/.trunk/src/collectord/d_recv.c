/*
 *
 * d_recv.c
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "e_define.h"
#include "d_define.h"
#include "e_protocol.h"
#include "d_config.h"
#include "d_datas.h"
#include "d_service.h"
#include "d_recv.h" 
#include "u_util.h"
#include "u_mutex.h"
#include "u_log.h"

#define LOGN(log_level,fmt,args...) LOG(log_level, "%s/ " fmt,node_name,args)

static int send_response ( int sock_fd, int node_discard_time );

extern collectord_config_t g_collectord_config;
/*Just log level need not mutex*/
extern node_list_t * g_node_list;
extern int mutex_g_node_list;
extern data_buffer_t g_data_buffer;
extern pthread_mutex_t mutex_g_data_buffer;


void * thread_recv ( void * arg_recv ) {

    LOG_START ( CLRD_LOG_PATH, g_collectord_config.log_level );

    thread_arg_recv_t * arg = ( thread_arg_recv_t * ) arg_recv;
    const unsigned int node_ip_uint = arg -> uint_ip;
    const int recv_sock_fd = arg -> socket_fd;

    free ( arg_recv );
    arg = arg_recv = NULL;

    pthread_t recv_thread_id = pthread_self ( );
    pthread_detach ( recv_thread_id );

    char node_ip_str [ MAX_IP_STR_LEN ];
    ipv4_uint_to_str ( node_ip_uint, node_ip_str, MAX_IP_STR_LEN );
    LOG ( LOG_INFO, "Thread %lu started. Service for: %s. Socket: %d.", 
        recv_thread_id, node_ip_str, recv_sock_fd );

    int ret;
    char errmsg [ CLRD_ERRMSG_LEN ];
    int node_id;
    char node_name [ MAX_NODE_NAME_LEN ];

    u_mutex_lock ( mutex_g_node_list );
    node_id = nl_find_node_by_nd_ip ( g_node_list, node_ip_uint );
    ret = nl_get_node_name_by_nd_id ( node_name, g_node_list, node_id );
    u_mutex_unlock ( mutex_g_node_list );
    if ( ret < 0 || node_name [ 0 ] == '\0' ) {
        LOGN ( LOG_ERROR, "Get node name error: %d", ret );
        pthread_exit ( NULL );
    }
    LOGN ( LOG_INFO, "Get node name: %s", node_name );

    u_mutex_lock ( mutex_g_node_list );
    ret = nl_set_node_status_by_nd_id ( g_node_list, node_id, NODE_STATUS_ACTIVE );
    u_mutex_unlock ( mutex_g_node_list );
    if ( ret < 0 ) {
        LOGN ( LOG_ERROR, "Set node status error: %d - Thread exit", ret );
        pthread_exit ( NULL );
    }
    LOGN ( LOG_DEBUG, "%s", "Set node status Active" );

    u_mutex_lock ( mutex_g_node_list );
    ret = nl_set_node_threadid_by_nd_id ( g_node_list, node_id, recv_thread_id );
    u_mutex_unlock ( mutex_g_node_list );
    if ( ret < 0 ) {
        LOGN ( LOG_ERROR, "Set thread id error: %d - Thread exit", ret );
        pthread_exit ( NULL );
    }
    LOGN ( LOG_DEBUG, "Set thread ID %lu for %s", recv_thread_id, node_name );

    struct timeval send_timeout;
    struct timeval recv_timeout;

    send_timeout.tv_sec = 5;
    send_timeout.tv_usec = 0;
    ret = setsockopt ( recv_sock_fd, SOL_SOCKET, SO_SNDTIMEO, 
            ( char * ) & send_timeout, sizeof ( send_timeout ) );

    recv_timeout.tv_sec = MAX_CL_INTERVAL + 100; /* init value */
    recv_timeout.tv_usec = 0;
    ret = setsockopt ( recv_sock_fd, SOL_SOCKET, SO_RCVTIMEO, 
            ( char * ) & recv_timeout, sizeof ( recv_timeout ) );

    node_istn_t exit_instruction;

    while ( 1 ) {

        u_mutex_lock ( mutex_g_node_list );
        exit_instruction = nl_get_node_istn_by_nd_id ( g_node_list, node_id );
        u_mutex_unlock ( mutex_g_node_list );
        if ( exit_instruction < 0 ) {
            LOGN ( LOG_ERROR, "Get instruction error: %d", exit_instruction );
        }
        else if ( exit_instruction == NODE_RECV_THREAD_EXIT ) {

            LOGN ( LOG_INFO, "Get node instruction EXIT - Delete node %s - Exit", node_name );

            u_mutex_lock ( mutex_g_node_list );
            ret = nl_delete_node_by_nd_id ( g_node_list, node_id );
            u_mutex_unlock ( mutex_g_node_list );
            if ( ret < 0 ) {
                LOGN ( LOG_ERROR, "Delete node %s error: %d", node_name, ret );
            }
            else {
                LOGN ( LOG_INFO, "Node %s deleted ", node_name );
            }

            close ( recv_sock_fd );
            LOGN ( LOG_INFO, "Thread %lu service for %s exited", recv_thread_id, node_ip_str );

            pthread_exit ( NULL );

        }

        LOGN ( LOG_INFO, "%s", "Get node instruction: RUN" );

        ssize_t recv_len = 0;
        recv_buffer_t recv_buffer;
        data_head_t * data_head;
        char * data_body;
        data_head = & ( recv_buffer.head );

RE_RECV_DATA:
        recv_len = recv ( recv_sock_fd, ( void * ) & recv_buffer, MAX_RECV_DATA_LEN_PER_SCRIPT, 0 );
        if ( recv_len == -1 ) {
            strerror_r ( errno, errmsg, CLRD_ERRMSG_LEN );
            if ( errno == EINTR ) {
                LOGN ( LOG_ERROR, "%s",
                        "Recv data error: interupted by signal - Continue to recv data after 1 second" );
                sleep ( 1 );
                goto RE_RECV_DATA;
            }
            else {
                LOGN ( LOG_FATAL, "Recv data error: %d %s - Change node status to DOWN and exit",
                        errno, errmsg );
                goto THREAD_EXIT;
            }
        }
        else if ( recv_len == 0 ) {
            LOGN ( LOG_ERROR, "Connection was closed by %s. " \
                    "Change status to down and exit", node_name );
THREAD_EXIT:
            u_mutex_lock ( mutex_g_node_list );
            ret = nl_set_node_status_by_nd_id ( g_node_list, node_id, NODE_STATUS_DOWN );
            u_mutex_unlock ( mutex_g_node_list );
            if ( ret < 0 ) {
                LOGN ( LOG_ERROR, "Set node %s status to DOWN error: %d", node_name, ret );
            }
            else {
                LOGN ( LOG_DEBUG, "%s", "Set node %s status to DOWN", node_name  );
            }

            u_mutex_lock ( mutex_g_node_list );
            ret = nl_delete_node_all_data_by_id ( g_node_list, node_id );
            u_mutex_unlock ( mutex_g_node_list );
            if ( ret < 0 ) {
                LOGN ( LOG_ERROR, "Delete %s all data error: %d", node_name, ret );
            }
            else {
                LOGN ( LOG_DEBUG, "Node %s all data deleted", node_name );
            }

            u_mutex_lock ( mutex_g_node_list );
            ret = nl_set_node_threadid_by_nd_id ( g_node_list, node_id, 0 );
            u_mutex_unlock ( mutex_g_node_list );
            if ( ret < 0 ) {
                LOGN ( LOG_ERROR, "Set thread ID to 0 error: %d", ret );
            }
            else {
                LOGN ( LOG_DEBUG, "Set node %s thread ID to 0", node_name );
            }

            LOGN ( LOG_INFO, "Thread %lu service for %s exited", recv_thread_id, node_ip_str );

            pthread_exit ( NULL );

        }

/* recv_len > 0 */
        if ( recv_len < DATA_PROTOCOL_HEAD_LEN ) { /*bad data*/
            LOGN ( LOG_ERROR, "Recv data len %d less than length of header", recv_len );
            continue;
        }

PROCESS_DATA:

        LOGN ( LOG_INFO, "Received data, data len: %d", recv_len );
        data_head = & ( recv_buffer.head );
        data_body = recv_buffer.data;
        /*data_body = ( char * ) ( ( void * ) data_head + DATA_PROTOCOL_HEAD_LEN );*/

        if ( recv_len < data_head -> data_len + DATA_PROTOCOL_HEAD_LEN ) { /*bad data*/
            LOGN ( LOG_ERROR, "Recv data len %d less than header's data len %d", recv_len,
                    data_head -> data_len + DATA_PROTOCOL_HEAD_LEN );
            continue;
        }

        if ( data_head -> data_type == 0x05 ) { /*script interval*/
            unsigned short int new_interval = * ( unsigned short int * ) & ( data_head -> data_status ); 
            nl_set_node_data_discard_time ( g_node_list, node_id, new_interval );
            LOGN ( LOG_INFO, "Data type sct - Interval: %d", new_interval );
            recv_timeout.tv_sec = ( new_interval ) + 5;
            recv_timeout.tv_usec = 0;
            ret = setsockopt ( recv_sock_fd, SOL_SOCKET, SO_RCVTIMEO, 
                ( char * ) & recv_timeout, sizeof ( recv_timeout ) );
        }

        else if ( data_head -> data_type == 0x01 ) { /*script data*/

            LOGN ( LOG_INFO, "%s", "Data type: script data" );

            if ( data_head -> data_status == 0x01 ) { /*normal*/

                LOGN ( LOG_INFO, "%s", "Data status: normal" );

                int sct_end = 0;
                char * sct_name_start;
                char * sct_data_start;
                char * file_time_stamp;

                if ( data_head -> data_position == 0x01 ) { /*memory*/
                    int node_discard_time;
                    nl_get_node_data_discard_time ( g_node_list, node_id, & node_discard_time );
                    send_response ( recv_sock_fd, node_discard_time );
                }

                if ( data_head -> data_position == 0x01 ) { /*memory*/
                    sct_name_start = data_body;
                }
                if ( data_head -> data_position == 0x02 ) { /*file*/
                    data_body [ MAX_DATE_TIME_LEN - 1 ] = '\0';
                    file_time_stamp = data_body;
                    sct_name_start = data_body + MAX_DATE_TIME_LEN; /*time#script name*/
                }
                for ( sct_end = 0; \
                         sct_name_start [ sct_end ] != '\0' \
                         && sct_name_start [ sct_end ] != '#';\
                         sct_end ++ );
                if ( sct_name_start [ sct_end ] == '#' ) {
                    sct_name_start [ sct_end ] = '\0';
                }

                else {
                     LOGN ( LOG_ERROR, "Data format error: no #, %s", data_body );
                     continue;
                }

                sct_data_start = sct_name_start + sct_end + 1;

                if ( data_head -> data_position == 0x01 ) { /*memory*/
                     ret = nl_put_node_script_data_by_id ( sct_name_start,
                                sct_data_start, g_node_list, node_id );
                    if ( ret < 0 ) {
                        LOGN ( LOG_ERROR, "Put script data fail:"
                                    " sct name %s | sct data %s | node id %d node name %s | | ret %d",
                                    sct_name_start, sct_data_start, node_id, node_name, ret );
                    }
                    else {
                        LOGN ( LOG_DEBUG, "Put script data sus: sct name: %s | node id: %d | node name %s",
                                    sct_name_start, node_id, node_name );
                    }
                }
                 
                char * delim_a = " ^\t\r\n\f\v";
                char * saveptr_a = NULL, * token_a = NULL, * str_a = NULL;

                for ( str_a = sct_data_start; ; str_a = NULL ) { 
                    token_a = strtok_r ( str_a, delim_a, & saveptr_a );
                    if ( token_a == NULL ) { 
                        break;
                    }
                    int eq_pos = 0;
                    while ( token_a [ eq_pos ] != '\0' ) {
                        if ( token_a [ eq_pos ] == '=' ) {
                           break;
                        }
                        eq_pos ++;
                    }
                    if ( token_a [ eq_pos ] != '\0' ) {
                        token_a [ eq_pos ] = '\0';
                        char * data_item_pos = token_a;
                        char * data_value_pos = token_a + eq_pos + 1;
                        data_item_t data_item;
                        if ( data_head -> data_position == 0x01 ) {
                            u_time_stamp_r ( data_item.data_time );
                        }
                        else if ( data_head -> data_position == 0x02 ) {
                            strcpy ( data_item.data_time, file_time_stamp );
                        }
                        else { /* no other position now */
                        }
                        strcpy ( data_item.node_name, node_name );
                        strcpy ( data_item.node_ip, node_ip_str );
                        strcpy ( data_item.data_item, data_item_pos );
                        strcpy ( data_item.data_value, data_value_pos );
DATA_RE_PUT:
                        pthread_mutex_lock ( & mutex_g_data_buffer );
                        ret = put_data_item_in_buffer ( & g_data_buffer, & data_item );
                        pthread_mutex_unlock ( & mutex_g_data_buffer );

                        if ( ret == 0 ) { /* not full */
                            LOGN ( LOG_DEBUG, "Put data %s %s",
                                data_item.data_item, data_item.data_value );
                        }

                        else if ( ret == 1 ) { /* full */
                            LOGN ( LOG_ERROR, "%s", 
                                    "Put data error:  buffer full. - sleep 1s then re put" );
                            sleep ( 1 );
                            goto DATA_RE_PUT;
                        }

                    } /* if ( token_a [ eq_pos ] != '\0' ) */

                    else {
                        LOGN ( LOG_ERROR, "Data format error: no \'=\', %s", token_a );
                    }

                } /* for ( str_a = sct_data_start; ; str_a = NULL ) */

            } /*if ( data_head -> data_status == 0x01 ) */

            else {

                LOGN ( LOG_WARN, "%s", "Other data status" );

            }

        } /* else if ( data_head -> data_type == 0x01 ) script data */

        else { /* other data type */

            LOGN ( LOG_WARN, "Other data type, %d", data_head -> data_type );

        }

        if ( recv_len > data_head -> data_len + DATA_PROTOCOL_HEAD_LEN ) { /* dual packet */

            LOGN ( LOG_INFO, "Dual packet - remain length %d", 
                    recv_len - data_head -> data_len + DATA_PROTOCOL_HEAD_LEN );
            recv_len -= data_head -> data_len + DATA_PROTOCOL_HEAD_LEN;
            data_head = ( data_head_t * )
                ( ( void * ) & recv_buffer + data_head -> data_len + DATA_PROTOCOL_HEAD_LEN );
            data_body = ( char * ) ( ( void * ) data_head + DATA_PROTOCOL_HEAD_LEN );

            goto PROCESS_DATA;

        }


    } /* while ( 1 ) */


    LOGN ( LOG_INFO, "Thread %lu service for %s exited", recv_thread_id, node_name );

    pthread_exit ( NULL );


}




static int send_response ( int sock_fd, int node_discard_time ) {

    data_head_t sct_discard_data_head = {
        .data_type = 0x06,
        .data_position = 0x01,
        .data_status = 0x01,
        .exit_code = 0,
        .data_len = 0,
    };

    if ( node_discard_time == 0 ) {
        sct_discard_data_head.data_status = 0;
    }
    else {
        sct_discard_data_head.data_status = 1;
    }

    send ( sock_fd, ( void * ) & sct_discard_data_head, DATA_PROTOCOL_HEAD_LEN, MSG_NOSIGNAL );

    return 0;

}




/*end of file*/
