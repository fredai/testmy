/*
 *
 * c_proc_sct.c
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>

#include "e_define.h"
#include "e_protocol.h"
#include "c_define.h"
#include "c_config.h"
#include "c_script.h"
#include "c_signal.h"
#include "c_engine.h"
#include "c_proc_sct.h"
#include "c_debug_log.h"
#include "u_util.h"
#include "u_regex.h"


#define CS_LOG_PATH CLR_LOG_SCT_PATH
#define SEND_BUFFER_FILE_INTERVAL 100

static int clr_copy_nfs_file ( char * dst, char * src );
static int clr_copy_sct_files ( scl_t * scl );


    static data_head_t sct_interval_data_head = {
        .data_type = 0x05,
        .data_position = 0x01,
        .data_status = 0x01,
        .exit_code = 0,
        .data_len = 0,
    };

    static data_head_t sct_file_data_head = {
        .data_type = 0x01,
        .data_position = 0x02,
        .data_status = 0x01,
        .exit_code = 0,
        .data_len = 0,
    };

    static data_head_t sct_cur_data_head = {
        .data_type = 0x01,
        .data_position = 0x01,
        .data_status = 0x01,
        .exit_code = 0,
        .data_len = 0,
    };


scl_t g_scl;
extern int g_sct_update_flag;
extern clr_sct_conf_t g_sct_conf;


int process_sct_main ( ) {

    cs_log_start ( g_sct_conf.log_level );
    cs_log ( LOG_DEBUG, "Process sct started" );

    int interval_have_send = 0;
    int need_read_data_from_file;

    int ret;
    char errmsg [ CLR_ERRMSG_LEN ];

    char sct_buffer [ MAX_SEND_DATA_LEN_PER_SCRIPT ];
    char *sct_data = sct_buffer + DATA_PROTOCOL_HEAD_LEN + MAX_DATE_TIME_LEN + MAX_SCRIPT_NAME_LEN + 32;
    const int sct_data_len = MAX_RESULT_DATA_LEN_PER_SCRIPT + 5;

    u_mkdir_p ( LOCAL_SCRIPT_PATH );

    ret = access ( DATA_BUFFER_FILE, F_OK );
    if ( ret == 0 ) {
        cs_log ( LOG_DEBUG, "Buffer file exists, need read data from file" );
        ret = access ( DATA_BUFFER_FILE, R_OK );
        if ( ret == 0 ) {
            need_read_data_from_file = 1;
        }
        else {
            cs_log ( LOG_DEBUG, "No read permiision on buffer file %s", DATA_BUFFER_FILE );
            need_read_data_from_file = 0;
        }
    }
    else {
        cs_log ( LOG_DEBUG, "Buffer file not exist" );
        need_read_data_from_file = 0;
    }

    clr_script_signal_init ( );
    cs_log ( LOG_DEBUG, "Init sct signal" );

    u_re_t u_re;
    ret = u_re_prepare ( EXE_FMT_RE_PATTERN, & u_re );

    int clr_interval = g_sct_conf.collect_interval;
    int sct_timeout = clr_interval / g_scl.sc_num;
    sct_timeout = sct_timeout > 0 ? sct_timeout : 1;
    cs_log ( LOG_DEBUG, "Clr interval is %d, timeout is %d", clr_interval, sct_timeout );

    char server_ip [ MAX_IP_STR_LEN ];
    unsigned short int server_port;
    strcpy ( server_ip, g_sct_conf.collectord_ip );
    server_port = g_sct_conf.collectord_port;
    cs_log ( LOG_DEBUG, "Server IP: %s | port: %hu", server_ip, server_port );

    int client_sockfd;
    struct sockaddr_in server_address;
    int server_len;
    int connection_is_ok = 0;

RECONNECT:

    connection_is_ok = 0;

    client_sockfd = socket ( AF_INET, SOCK_STREAM, 0 );
    if ( client_sockfd == -1 ) {
        strerror_r ( errno, errmsg, CLR_ERRMSG_LEN );
        cs_log ( LOG_ERROR, "Create socket error: %d %s", errno, errmsg );
    }
    else {
        cs_log ( LOG_DEBUG, "Create socket success" );
    }

    int opt = 0;
    ret = setsockopt ( client_sockfd, SOL_SOCKET, SO_REUSEADDR, ( char * ) & opt, sizeof ( opt ) );
    if ( ret == -1 ) {
        strerror_r ( errno, errmsg, CLR_ERRMSG_LEN );
        cs_log ( LOG_ERROR, "Set sockopt re use addr error: %d %s", errno, errmsg );
    }
    else {
        cs_log ( LOG_DEBUG, "Set sockopt re use addr success" );
    }

    struct timeval socket_timeout;

    socket_timeout.tv_sec = CLR_SEND_TIMEOUT;
    socket_timeout.tv_usec = 0;
    ret = setsockopt ( client_sockfd, SOL_SOCKET, SO_SNDTIMEO, 
            ( char * ) & socket_timeout, sizeof ( socket_timeout ) );
    if ( ret == -1 ) {
        strerror_r ( errno, errmsg, CLR_ERRMSG_LEN );
        cs_log ( LOG_ERROR, "Set sockopt send timeout error: %d %s", errno, errmsg );
    }
    else {
        cs_log ( LOG_DEBUG, 
                "Set sockopt send time out success - time out value is %lu",
                socket_timeout.tv_sec );
    }

    socket_timeout.tv_sec = CLR_RECV_TIMEOUT;
    socket_timeout.tv_usec = 0;
    ret = setsockopt ( client_sockfd, SOL_SOCKET, SO_RCVTIMEO, 
            ( char * ) & socket_timeout, sizeof ( socket_timeout ) );
    if ( ret == -1 ) {
        strerror_r ( errno, errmsg, CLR_ERRMSG_LEN );
        cs_log ( LOG_ERROR, "Set sockopt recv timeout error: %d %s", errno, errmsg );
    }
    else {
        cs_log ( LOG_DEBUG, 
                "Set sockopt recv time out success - time out value is %lu",
                socket_timeout.tv_sec );
    }

    server_address.sin_family = AF_INET;
    ret = ipv4_pton ( server_ip, & ( server_address.sin_addr ) );
    server_address.sin_port = htons ( server_port );
    server_len = sizeof ( server_address );
 
    ret = connect ( client_sockfd, ( struct sockaddr * ) & server_address, server_len );
    if ( ret == -1 ) {
        strerror_r ( errno, errmsg, CLR_ERRMSG_LEN );
        cs_log ( LOG_ERROR, "Connect error: %d %s", errno, errmsg );
        switch ( errno ) {
            case EISCONN:
                cs_log ( LOG_WARN, "Already connected: %d %s", errno, errmsg );
                break;
            case ECONNREFUSED:
            case ETIMEDOUT:
            case EINTR:
            default:
                break; /* to execute others and then come back */
        }

        connection_is_ok = 0;
        interval_have_send = 0;

    }

    else {
        cs_log ( LOG_DEBUG, "Connect success" );
        connection_is_ok = 1;

        if ( interval_have_send == 0 ) {
            * ( unsigned short int * ) & ( sct_interval_data_head.data_status ) = clr_interval;
            ret = send ( client_sockfd, & sct_interval_data_head,
                    DATA_PROTOCOL_HEAD_LEN, MSG_NOSIGNAL );
            if ( ret == DATA_PROTOCOL_HEAD_LEN ) {
                cs_log ( LOG_DEBUG, "Send interval success" );
                interval_have_send = 1;
            }
            else {
                cs_log ( LOG_ERROR, "Send interval fail" ); /* come back later */
                interval_have_send = 0;
            }
        }

        clr_copy_sct_files ( & g_scl );

    }

    int ix = 0;
    char * sct_name = NULL;
    int exit_status;
    char * sct_head_pos = NULL;
    int send_len;
   
    while ( 1 ) {

        if ( g_sct_update_flag == 1 ) {

            cs_log ( LOG_DEBUG, "Collector dynamic update ..." );

            ret = update_clr_sct_config ( & g_sct_conf );
            if ( ret == 0 ) {
                clr_interval = g_sct_conf.collect_interval; /*collector interval*/
                cs_log ( LOG_DEBUG, "Reload config success - collector interval is %hu", clr_interval );
                sct_timeout = clr_interval / g_scl.sc_num;
                sct_timeout = sct_timeout > 0 ? sct_timeout : 1;
                sct_interval_data_head.exit_code = clr_interval; /*discard time*/
                ret = send ( client_sockfd, & sct_interval_data_head,
                    DATA_PROTOCOL_HEAD_LEN, MSG_NOSIGNAL );
                if ( ret == DATA_PROTOCOL_HEAD_LEN ) {
                    cs_log ( LOG_DEBUG, "Send interval %d success", clr_interval );
                    interval_have_send = 1;
                }
                else {
                    cs_log ( LOG_ERROR, "send interval fail" );
                    interval_have_send = 0;
                }
            }
            else if ( ret == -33 ) {
                cs_log ( LOG_DEBUG, "Collector config need not update" );
            }
            else {
                cs_log ( LOG_ERROR, "Update config error: %d - ignore", ret );
            }

            ret = update_script_list ( NODE_LIST_FILE_PATH, & g_scl );
            if ( ret == -6 ) {
                cs_log ( LOG_INFO, "This node not in nodelist file" );
                fprintf ( stderr, "After dynamic update, this node not in nodelist file" );
                close ( client_sockfd );
                exit ( 2 ); /* close and exit */
            }

            else if ( ret == -7 ) {
                cs_log ( LOG_ERROR, "After read file no script can be executed" );
                fprintf ( stderr, "After dynamic update, no script can be executed" );
                close ( client_sockfd );
                exit ( 3 ); /* close and exit */
            }

            else if ( ret < 0 ) { /* other error, have not exit */
                cs_log ( LOG_ERROR, "Update script list error and ignore" );
            }

            else {
                sct_timeout = clr_interval / g_scl.sc_num; /* sc_num changed */
                sct_timeout = sct_timeout > 0 ? sct_timeout : 1;
                cs_log ( LOG_DEBUG, "Update script list success" );
            }

            clr_copy_sct_files ( & g_scl );

            g_sct_update_flag = 0;

        } /* if ( g_sct_update_flag == 1 )*/

        if ( connection_is_ok == 1 && need_read_data_from_file == 1 ) {

            cs_log ( LOG_DEBUG, "Need read data from file" );

            FILE * data_file;
            char * read_file_ret;
            char * file_send_pos;

            data_file = fopen ( DATA_BUFFER_FILE, "r" );
            if ( data_file != NULL ) {
                while ( 1 ) {
                    read_file_ret = fgets ( sct_data, sct_data_len, data_file );
                    if ( read_file_ret == NULL ) {
                        break;
                    }
                    else {
                        int file_data_len = strlen ( sct_data ) + 1;
                        if ( file_data_len < MAX_DATE_TIME_LEN + 1 + 1 + 1 + 1 ) { /*item=value\0*/
                            cs_log ( LOG_ERROR, "Invalid data from file: too short %s", sct_data );
                        }
                        else {
                            /*if ( format is valid ) {
                             * FIXME
                            }*/
                            sct_file_data_head.data_len = file_data_len;
                            file_send_pos = sct_data - DATA_PROTOCOL_HEAD_LEN;
                            memcpy ( file_send_pos, & sct_file_data_head, DATA_PROTOCOL_HEAD_LEN );
                            send_len = sct_file_data_head.data_len + DATA_PROTOCOL_HEAD_LEN;
                            ret = send ( client_sockfd, file_send_pos, send_len, MSG_NOSIGNAL );
                            if ( ret == -1 ) {
                                cs_log ( LOG_ERROR, "Send data from file fail" );
                                need_read_data_from_file = 1;
                                /*FIXME truncate*/
                                break;
                            }
                            else {
                                cs_log ( LOG_DEBUG, "Send data from file success" );
                                need_read_data_from_file = 0;
                            }
                        }
                    }

                    u_msleep ( 0, SEND_BUFFER_FILE_INTERVAL );

                } /*while ( 1 )*/

                fclose ( data_file );

            } /*if ( data_file != NULL )*/

            if ( need_read_data_from_file == 0 ) {
                unlink ( DATA_BUFFER_FILE );
            }
            
        }

        int send_errno = 0;
        int send_ret = 0;
        int recv_ret = 0;
        time_t first_sc_start_time;
        time_t last_sc_end_time;
        char local_script_name [ LOCAL_SCIPRT_PATH_LEN ];

        first_sc_start_time = time ( NULL );

        for ( ix = 0; ix < g_scl.sc_num; ix ++ ) {

            sct_name = g_scl.sc_names [ ix ];
            sprintf ( local_script_name, "%s/%s", LOCAL_SCRIPT_PATH, sct_name + SCRIPT_FILE_PATH_LEN );

            ret = access ( local_script_name, X_OK|F_OK );
            if ( ret == -1 ) {
                cs_log ( LOG_ERROR, 
                            "Script %s not exist or no execute permision",
                            local_script_name );
                continue; /*next script*/
            }

            char exe_start [ MAX_DATE_TIME_LEN ];
            char exe_end [ MAX_DATE_TIME_LEN ];

            u_time_stamp_r ( exe_start );

            struct timeval timer_start = timeval_current ( );

            ret = c_execute ( local_script_name, sct_data, sct_data_len, & exit_status, sct_timeout );

            u_time_stamp_r ( exe_end );

            double time_elapsed = timeval_elapsed ( & timer_start );

            cs_log ( LOG_DEBUG, "Execute script %s started at %s ended at %s use %fs return %d",
                       local_script_name, exe_start, exe_end, time_elapsed, ret );

            if ( ret == -6 || ret == -7 ) { /*timeout*/
                cs_log ( LOG_DEBUG, "Execute script %s error: time out", local_script_name );
                continue;
            }
            else if ( ret == -5 || ret > MAX_RESULT_DATA_LEN_PER_SCRIPT ) { /*too long*/
                cs_log ( LOG_DEBUG, "Execute script %s error: result too long", local_script_name );
                continue;
            }
            else if ( ret < 0 ) { /*next script*/
                cs_log ( LOG_DEBUG, "Execute script %s error: %d", local_script_name, ret );
                continue;
            }
            else if ( ret > 0 && ret < 1 + 1 + 1 ) { /*item=value*/
                cs_log ( LOG_DEBUG, "Execute script %s error: %d result too short", local_script_name, ret );
                continue;               
            }

            if ( exit_status != SCT_EIXT_CODE ) {
                cs_log ( LOG_ERROR, "Execute script %s error: exit code %d not %d", \
                        local_script_name, exit_status, SCT_EIXT_CODE );
                continue;
            }

            int u_re_ret;
            u_re_ret = u_re_exe ( & u_re, sct_data, ret );
            if ( u_re_ret > 0 ) {
                cs_log ( LOG_DEBUG, "Result format is valid" );
            }
            else if ( u_re_ret == 0 ) {
                cs_log ( LOG_ERROR, "Result format is invalid" );
                continue;
            }
            else {
                cs_log ( LOG_ERROR, "Regex exe error" );
                continue;
            }

            sct_data [ ret ] = 0;
            int sct_name_len = strlen ( sct_name ) - SCRIPT_FILE_PATH_LEN;
            char * sct_name_pos = sct_data - sct_name_len - 1;
            strncpy ( sct_name_pos, sct_name + SCRIPT_FILE_PATH_LEN, sct_name_len );
            sct_data [ -1 ] = '#';

            sct_cur_data_head.exit_code = exit_status;
            sct_cur_data_head.data_len = sct_name_len + 1 + ret + 1;
                                                      /*#*/     /*\0*/
            sct_head_pos = sct_name_pos - DATA_PROTOCOL_HEAD_LEN;
            memcpy ( sct_head_pos, & sct_cur_data_head, DATA_PROTOCOL_HEAD_LEN );
            send_len = DATA_PROTOCOL_HEAD_LEN + sct_cur_data_head.data_len;

            if ( connection_is_ok == 0 ) {
                send_ret = -1;
                send_errno = ENOTCONN;
                goto WRITE_DATA_TO_FILE;
            }

            send_ret = send ( client_sockfd, sct_head_pos, send_len, MSG_NOSIGNAL );
            send_errno = errno;
            cs_log ( LOG_DEBUG, "Send data len %d return %d", send_len, send_ret );
            
            if ( send_ret > 0 ) {
                char recv_reponse [ DATA_PROTOCOL_HEAD_LEN * 10 ];
                recv_ret = recv ( client_sockfd, ( void * ) recv_reponse, sizeof ( recv_reponse ), 0 );
                cs_log ( LOG_DEBUG, "Recv for check network: %d", recv_ret );
                if ( recv_ret == -1 ) {
                    connection_is_ok = 0;
                }
                else if ( recv_ret == DATA_PROTOCOL_HEAD_LEN ) {
                    data_head_t * interval_head = ( data_head_t * ) recv_reponse;
                    if ( interval_head -> data_status == 0 ) {
                        sct_interval_data_head.exit_code = clr_interval;
                        send ( client_sockfd, & sct_interval_data_head,
                            DATA_PROTOCOL_HEAD_LEN, MSG_NOSIGNAL );
                    }
                }
            }

WRITE_DATA_TO_FILE:

            ; /* void statement */

            if ( send_ret == -1 || recv_ret == -1 ) { /* affected by both send and recv */

                connection_is_ok = 0;
                strerror_r ( send_errno, errmsg, CLR_ERRMSG_LEN );
                cs_log ( LOG_DEBUG, "Send fail: %d %s - write data to file", 
                        send_errno, errmsg );

                char * file_data_start = sct_head_pos - MAX_DATE_TIME_LEN + DATA_PROTOCOL_HEAD_LEN;
                int file_data_len = send_len - DATA_PROTOCOL_HEAD_LEN + MAX_DATE_TIME_LEN - 1;/*\0*/
                u_time_stamp_r ( file_data_start );
                * ( file_data_start + MAX_DATE_TIME_LEN - 1 ) = '#';
                ret = write_data_to_file ( DATA_BUFFER_FILE, file_data_start, file_data_len );
                need_read_data_from_file = 1;
                cs_log ( LOG_DEBUG, "Data writed to file - set need read flag to 1" );

            } /* if ( ret == -1 ) */

            else if ( send_ret == send_len ) {
                cs_log ( LOG_DEBUG, "Send data success" );
            }
            else if ( send_ret < send_len ) {
                cs_log ( LOG_ERROR, "Send data incomplete" );
            }
            else if ( send_ret > send_len ) {
            }

        } /* for ( ix = 0; ix < g_scl.sc_num; ix ++ ) */

        last_sc_end_time = time ( NULL );
        /* script exe time + send time + recv time + write file time */

        if ( clr_interval - ( last_sc_end_time - first_sc_start_time ) > 0 ) {
            sleep ( clr_interval - ( last_sc_end_time - first_sc_start_time ) );
        }

        if ( send_ret == -1 ) {
            /* FIXME here or not here, other way, could set for ( ix = start; ... ) start value */
            switch ( send_errno ) {
                case EINTR:
                    cs_log ( LOG_DEBUG, "Interupted by signal - ignore" );
                    break;
                case ECONNRESET:
                case ENOTCONN:
                case EPIPE:
                default:
                    cs_log ( LOG_DEBUG, "Send fail - re connect" );
                    close ( client_sockfd );
                    goto RECONNECT;
            }
        }

        if  ( recv_ret == -1 ) {
            cs_log ( LOG_DEBUG, "Recv fail - re connect" );
            close ( client_sockfd );
            goto RECONNECT;
        }

    } /* while ( 1 ) */

    ret = u_re_finish ( & u_re );
    if ( ret == 0 ) {
        cs_log ( LOG_DEBUG, "Regex finish" );
    }
    else {
        cs_log ( LOG_ERROR, "Regex finish error" );
    }

    exit ( 0 );

}



int write_data_to_file ( char * file, char * data, int data_len ) {
    if ( file == NULL || data == NULL || file [ 0 ] == '\0' ) {
        return -1;
    }
    int ret;
    FILE * data_file;
    data_file = fopen ( file, "a+" );
    if ( data_file == NULL ) {
        return -2;
    }
    ret = fwrite ( data, data_len, 1, data_file );
    fclose ( data_file );
    if ( ret != data_len ) {
        return -3;
    }
    return 0;
}




static int clr_copy_nfs_file ( char * dst, char * src ) {

    if ( dst == NULL || src == NULL || dst [ 0 ] == '\0' || src [ 0 ] == '\0' ) {
        return -1;
    }

    int ret;
    mode_t mode;

    ret = access ( src, X_OK|F_OK );
    if ( ret == -1 ) {
        return -2;
    }

    ret = access ( LOCAL_SCRIPT_PATH, F_OK | X_OK );
    if ( ret == -1 ) {
        mode = umask ( 0 );
        ret = u_mkdir_p ( LOCAL_SCRIPT_PATH );
        umask ( mode );
        if ( ret < 0 ) {
            return -3;
        }
    }

    u_copy_file ( dst, src );

    mode = umask ( 0 );
    chmod ( dst, 0770 );
    umask ( mode );

    return 0;

}


static int clr_copy_sct_files ( scl_t * scl ) {

    if ( scl == NULL ) {
        return -1;
    }

    int bx;
    char bx_script_name [ LOCAL_SCIPRT_PATH_LEN ];
    for ( bx = 0; bx < scl -> sc_num; bx ++ ) {
        sprintf ( bx_script_name, "%s/%s", LOCAL_SCRIPT_PATH,
                scl -> sc_names [ bx ] + SCRIPT_FILE_PATH_LEN );
        clr_copy_nfs_file ( bx_script_name, scl -> sc_names [ bx ] );
    }

    return 0;

}



/*end of file*/
