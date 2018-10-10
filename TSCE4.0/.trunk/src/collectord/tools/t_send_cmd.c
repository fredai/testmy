/*
 *
 * t_send_cmd.c
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
#include "e_protocol.h"
#include "d_define.h"
#include "d_nodes.h"
#include "t_define.h"
#include "t_process_opt.h"
#include "t_send_cmd.h"
#include "u_util.h"
#include "u_shm.h"
#include "u_mutex.h"
#include "u_log.h"


#define CF_STR_CLR_CMD_PORT "collector_port"

static const char * node_status_str [ 3 ] =  {
      " DOWN ",
      "ACTIVE",
     "NOT FOUND"
};
 
static int node_name_compare ( const void * a, const void * b ) {
    return strcmp ( ( ( node_status_info_t * ) a ) -> node_name,
            ( ( node_status_info_t * ) b ) -> node_name );
}

char g_cmd [ MAX_CMD_REQ_LEN ];
pthread_mutex_t mutex_output;
snd_conf_t g_snd_conf;


int main ( int argc, char * argv [ ] ) {

    int ret;
    ret = check_pid ( CLRD_PID_FILE_PATH );
    if ( ret < 0 ) {
        fprintf ( stderr, "Collecotrd not started\n" );
        exit ( 9 );
    }

    node_list_t * tc_node_list;
    void * nl_shmaddr = 0x0;
    int tc_shmid;
    int tc_mutex_id;

    char node_names [ MAX_NODES_NUM + 32 ] [ MAX_NODE_NAME_LEN ];
    int nodes_num = 0;
    int cret;


    if ( argc == 1 ) {
        fprintf ( stderr, "Usage: %s nodes command\n", argv [ 0 ] );
        exit ( 1 );
    }

    strcpy ( g_cmd, argv [ argc - 1 ] );

    if ( argc > 2 ) {
        cret = t_process_cmd_options ( argc - 1, argv, node_names, MAX_NODES_NUM + 32 );
        if ( cret < 0 ) {
            fprintf ( stderr, "Invalid parameters\n" );
            exit ( 2 );
        }
        nodes_num = cret;
    }

    tc_shmid = u_shm_attach ( CLRD_SHM_KEY, sizeof ( node_list_t ), & nl_shmaddr );
    if ( tc_shmid < 0 ) {
        fprintf ( stderr, "Collecotrd not started\n" );
        exit ( 3 );
    }
    if ( nl_shmaddr == 0x0 ) {
        fprintf ( stderr, "Shared memory addr is invalid\n" );
        exit ( 4 );
    }

    tc_node_list = ( node_list_t * ) nl_shmaddr;

    tc_mutex_id = u_mutex_attach ( CLRD_SEM_KEY );
    if ( tc_mutex_id < 0 ) {
        fprintf ( stderr, "Attach mutex error\n" );
        u_shm_detach ( nl_shmaddr );
        exit ( 5 );
    }


    LOG_START ( CLRD_LOG_PATH, LOG_DEBUG );
    ret = send_cmd_read_config_from_file ( CLRD_T_SNDCMD_CONFIG_FILE_PATH, & g_snd_conf );
    if ( ret < 0 ) {
        fprintf ( stderr, "Read config from file error\n" );
        u_shm_detach ( nl_shmaddr );
        exit ( 6 );
    }


    node_status_info_t node_status_info [ NODE_STATUS_INFO_SIZE ];

    u_mutex_lock ( tc_mutex_id );

    if ( argc > 2 ) {

        int name_inx = -1;
        while ( ++ name_inx < cret ) {
            strcpy ( node_status_info [ name_inx ].node_name, node_names [ name_inx ] );
            int node_id;
            int node_status;
            node_id = nl_find_node_by_nd_name ( tc_node_list, node_names [ name_inx ] );
            if ( node_id == 0 ) {
                node_status_info [ name_inx ].node_status = NODE_NOT_FOUND;
                node_status_info [ name_inx ].node_ip [ 0 ] = '\0';
            }
            else {
                node_status = nl_get_node_status_by_nd_id ( tc_node_list, node_id );
                node_status_info [ name_inx ].node_status = node_status;
                nl_get_node_ip_str_by_nd_id ( 
                        node_status_info [ name_inx ].node_ip, tc_node_list, node_id );
            }
        }
    } /* if ( argc > 1 ) */

    else {
        ret = traversal_node_status_info ( tc_node_list, node_status_info, NODE_STATUS_INFO_SIZE );
        if ( ret < 0 ) {
            LOG ( LOG_ERROR, "%s","Traversal node status info error: %d", ret );
            nodes_num = 0;
        }
        else {
            nodes_num = ret;
        }
    }

    u_mutex_unlock ( tc_mutex_id );

    if ( nodes_num == 0 ) {
        u_shm_detach ( nl_shmaddr );
        fprintf ( stderr, "No node in node list\n" );
        exit ( 7 );
    }

    qsort ( node_status_info, nodes_num, sizeof ( node_status_info_t ), node_name_compare );

    pthread_mutex_init ( & mutex_output, NULL );

    pthread_t send_cmd_threads [ MAX_NODES_NUM ];

    int ix = -1;
    int threads_num = 0;

    while ( ++ ix < nodes_num ) {
        if ( node_status_info [ ix ].node_status == NODE_STATUS_ACTIVE ) {
            arg_send_cmd_t * arg_send_cmd = ( arg_send_cmd_t * ) malloc ( sizeof ( arg_send_cmd_t ) );
            ipv4_str_to_uint ( node_status_info [ ix ].node_ip,
                    & ( arg_send_cmd -> node_ip ) );
            strcpy ( arg_send_cmd -> node_name, node_status_info [ ix ].node_name );
            pthread_create ( & send_cmd_threads [ threads_num ], NULL,
                    thread_send_cmd, ( void * ) arg_send_cmd );
            threads_num ++;
        }
    }


    while ( -- threads_num >= 0 ) {
        pthread_join ( send_cmd_threads [ threads_num ], NULL );
    }

    pthread_mutex_destroy ( & mutex_output );

    ix = -1;
    while ( ++ ix < nodes_num ) { 
        if ( node_status_info [ ix ].node_status != NODE_STATUS_ACTIVE ) {
            break;
        }
    }
    if ( ix == nodes_num ) {
        goto NOT_OUT_PUT_OTHER_NODES;
    }

    fprintf ( stdout, "\n-----------------------------------------\n" );
    ix = -1;
    while ( ++ ix < nodes_num ) {
        if ( node_status_info [ ix ].node_status != NODE_STATUS_ACTIVE ) {
            fprintf ( stdout, "%-32s%6s\n",
                node_status_info [ ix ].node_name, 
                node_status_str [ node_status_info [ ix ].node_status ] );
        }
    }
    fprintf ( stdout, "-----------------------------------------\n\n" );

NOT_OUT_PUT_OTHER_NODES:

    u_shm_detach ( nl_shmaddr );

    exit ( 0 );


}



void * thread_send_cmd ( void * arg_send_cmd ) {
 
    int client_sockfd;
    struct sockaddr_in server_address;
    int server_len;
    int ret;
    char node_name [ MAX_NODE_NAME_LEN ];
    char recv_buffer [ BUFSIZ + 1 ];
    char errmsg [ CLRD_ERRMSG_LEN ];
    char node_ip_str [ MAX_IP_STR_LEN ];

    LOG_START ( CLRD_LOG_PATH, LOG_DEBUG );

    server_address.sin_addr.s_addr = ( ( arg_send_cmd_t * ) arg_send_cmd ) -> node_ip;
    strcpy ( node_name, ( ( arg_send_cmd_t * ) arg_send_cmd ) -> node_name );

    free ( arg_send_cmd );

    ipv4_uint_to_str ( server_address.sin_addr.s_addr, node_ip_str, MAX_IP_STR_LEN );
    LOG ( LOG_DEBUG, "Send cmd %s to %s %s", g_cmd, node_ip_str, node_name );

    client_sockfd = socket ( AF_INET, SOCK_STREAM, 0 );
    if ( client_sockfd == -1 ) {
        strerror_r ( errno, errmsg, CLRD_ERRMSG_LEN );
        LOG ( LOG_ERROR, "Socket error: %d %s", errno, errmsg );
        pthread_exit ( NULL );
    }
    else {
        LOG ( LOG_DEBUG, "%s", "Socket success" );
    }

    int opt = 0;
    ret = setsockopt ( client_sockfd, SOL_SOCKET, SO_REUSEADDR, ( char * ) & opt, sizeof ( opt ) );
    if ( ret == -1 ) {
        strerror_r ( errno, errmsg, CLRD_ERRMSG_LEN );
        LOG ( LOG_ERROR, "Setsockopt error: %d %s", errno, errmsg );
    }
    else {
        LOG ( LOG_DEBUG, "%s", "Setsockopt success" );
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons ( g_snd_conf.clr_cmd_port );
    server_len = sizeof ( server_address );
 
    ret = connect ( client_sockfd, ( struct sockaddr * ) & server_address, server_len );
    if ( ret == -1 ) {
        strerror_r ( errno, errmsg, CLRD_ERRMSG_LEN );
        LOG ( LOG_ERROR, "Connect error: %d %s", errno, errmsg );
        fprintf ( stderr, "%s Connect to %s:%hu failed: %s\n", 
                node_name, node_ip_str, g_snd_conf.clr_cmd_port, errmsg );
        pthread_exit ( NULL );
    }
    else {
        LOG ( LOG_DEBUG, "%s", "Connect success" );
    }

    ret = send ( client_sockfd, g_cmd, strlen ( g_cmd ) + 1, 0 );
    if ( ret == -1 ) {
        strerror_r ( errno, errmsg, CLRD_ERRMSG_LEN );
        LOG ( LOG_ERROR, "Send error: %d %s", errno, errmsg );
        fprintf ( stderr, "%s Send cmd to %s:%hu failed\n", 
                node_name, node_ip_str, g_snd_conf.clr_cmd_port );
        pthread_exit ( NULL );
    }
    else if ( ret == strlen ( g_cmd ) + 1 ) {
        LOG ( LOG_DEBUG, "%s", "Send data success" );
    }
    else {
        LOG ( LOG_ERROR, "%s", "Send data incompleted" );
    }

    ret = recv ( client_sockfd, recv_buffer, BUFSIZ - 1, 0 );
    if ( ret == -1 ) {
        strerror_r ( errno, errmsg, CLRD_ERRMSG_LEN );
        LOG ( LOG_ERROR, "Recv error: %d %s", errno, errmsg );
        fprintf ( stderr, "%s recv data from %s:%hu error\n", 
                node_name, node_ip_str, g_snd_conf.clr_cmd_port );
        pthread_exit ( NULL );
    }
    else {
        LOG ( LOG_DEBUG, "%s", "Recv success" );
    }

    data_head_t * data_head = ( data_head_t * ) recv_buffer;

    char * data = recv_buffer + sizeof ( data_head_t );

    if ( ret == 0 ) {
        LOG ( LOG_ERROR, "%s", "Recv ret == 0 without result data" );
        pthread_exit ( NULL );
    }
    
    if ( ret < DATA_PROTOCOL_HEAD_LEN ) {
        LOG ( LOG_ERROR, "Recv data len %d is short than head len", ret );
        fprintf ( stderr, "\n%s\t\n\n", node_name );
        fprintf ( stderr, "incompleted data\n\n" );
        pthread_exit ( NULL );
    }

    if ( ret < data_head -> data_len + DATA_PROTOCOL_HEAD_LEN ) { /*data bad*/
        LOG ( LOG_ERROR, "Recv data len %d is short than head's data len %d", ret,
                    data_head -> data_len + DATA_PROTOCOL_HEAD_LEN );
        goto LONG_DATA;
        fprintf ( stderr, "\n%s\t\n\n", node_name );
        fprintf ( stderr, "incompleted data\n\n" );
        pthread_exit ( NULL );
    }

    if ( data_head -> data_status == 0x02 ) {
        fprintf ( stderr, "\n%s\t\n\n", node_name );
        fprintf ( stderr, "Result time out\n\n" );
        pthread_exit ( NULL );
    }

    else if ( data_head -> data_status == 0x03 ) {
        fprintf ( stderr, "\n%s\t\n\n", node_name );
        fprintf ( stderr, "Result too long\n\n" );
        pthread_exit ( NULL );
    }

    pthread_mutex_lock ( & mutex_output );
    fprintf ( stdout, "\n%s\texit code %d\n\n", node_name, data_head -> exit_code );
    fprintf ( stdout, "%s\n\n", data );
    pthread_mutex_unlock ( & mutex_output );

    pthread_exit ( NULL );

LONG_DATA:
    pthread_mutex_lock ( & mutex_output );

    fprintf ( stdout, "\n%s\texit code %d\n\n", node_name, data_head -> exit_code );
    recv_buffer [ ret ] = 0;
    fprintf ( stdout, "%s", data );
    while ( 1 ) {
        ret = recv ( client_sockfd, recv_buffer, BUFSIZ - 1, 0 );
        recv_buffer [ ret ] = 0;
        fprintf ( stdout, "%s", recv_buffer );
        if ( ret == 0 ) {
            break;
        }
    }
    fprintf ( stdout, "\n\n" );

    pthread_mutex_unlock ( & mutex_output );
    pthread_exit ( NULL );

}


int send_cmd_read_config_from_file ( char * config_file_path, snd_conf_t * snd_conf ) {

    if ( config_file_path == NULL || snd_conf == NULL || config_file_path [ 0 ] == '\0' ) {
        return -1;
    }

    LOG_START ( CLRD_LOG_PATH, LOG_DEBUG );

    int ret = -1;
    int str_len = 0;
    FILE * config_file;
    const int padding_len = 1 + 5;
    char line_buffer [ MAX_CONFIG_FILE_LINE_LEN + padding_len ];

    bzero ( snd_conf, sizeof ( snd_conf_t ) );

    config_file = fopen ( config_file_path, "r" );
    if ( config_file == NULL ) {
        LOG ( LOG_ERROR, "Open file %s error", config_file_path );
        return -2;
    }

    while ( fgets ( line_buffer, MAX_CONFIG_FILE_LINE_LEN, config_file ) ) {

        str_len = strlen ( line_buffer );

        if ( str_len > 0 && line_buffer [ str_len - 1 ] == '\n' ) {
            line_buffer [ str_len - 1 ] = 0;
            str_len --;
        }
        if ( str_len > 1 && line_buffer [ str_len - 2 ] == '\r' ) {
            line_buffer [ str_len - 2 ] = 0;
            str_len --;
        }

        if ( str_len > MAX_CONFIG_FILE_LINE_LEN ) { 
            LOG ( LOG_WARN, "Line too long, discard: %s", line_buffer );
            return -3;
        }

        char * line_buffer_trim_head = trim_head ( line_buffer, str_len );

        if ( * line_buffer_trim_head == '#' ) {
            continue;
        }
        if ( * line_buffer_trim_head == 0 ) {
            continue;
        }

        ret = parse_snd_config_line ( line_buffer_trim_head, snd_conf );

        if ( ret < 0 ) {
            return -4;
        }

    }

    fclose ( config_file );

    if ( snd_conf -> clr_cmd_port == 0 ) {
        return -5;
    }

    LOG_END;

    return 0;

}


int parse_snd_config_line ( char * config_line, snd_conf_t * snd_conf ) {

    LOG_START ( CLRD_LOG_PATH, LOG_DEBUG );
    if ( config_line == NULL || snd_conf == NULL || config_line [ 0 ] == '\0' ) {
        LOG ( LOG_ERROR, "%s", "Invalid parameters" );
        return -1;
    }

    char config_item [ MAX_CONFIG_ITEM_LEN + 1 ];
    char config_value [ MAX_CONFIG_VALUE_LEN + 1 ];
    char * delim = " \t\r\n\f\v";
    char * saveptr;
    char * token;
    char * str = config_line;

    token = strtok_r ( str, delim, & saveptr );

    if ( token == NULL ) {
        LOG ( LOG_DEBUG, "%s", "Empty line" );
        return -2;
    }

    if ( strlen ( token ) > MAX_CONFIG_ITEM_LEN ) {
        LOG ( LOG_WARN, "%s %s", "Config item too long", token );
        return -3;
    }

    strcpy ( config_item, token );

    token = strtok_r ( NULL, delim, & saveptr );
    if ( token == NULL ) {
        LOG ( LOG_WARN, "%s", "Config item no value" );
        return -4;
    }

    char * remain_str = strtok_r ( NULL, delim, & saveptr );
    if ( remain_str != NULL ) {
        LOG ( LOG_WARN, "%s", "Config item too many values" );
        return -5; /* too many value */
    }

    if ( strlen ( token ) > MAX_CONFIG_VALUE_LEN ) {
        LOG ( LOG_WARN, "%s %s", "Config value too long", token );
        return -6;
    }

    strcpy ( config_value, token );

    if ( strcmp ( config_item, CF_STR_CLR_CMD_PORT ) == 0 ) {
        unsigned short int cr_port;
        int cr_ret = parse_port ( config_value, & cr_port );
        if ( cr_ret < 0 ) {
            LOG ( LOG_WARN, "%s %s", "Collector port is invalid", config_value );
            return -7;
        }
        snd_conf -> clr_cmd_port = cr_port;
    }

    return 0;

}


/*end of file*/
