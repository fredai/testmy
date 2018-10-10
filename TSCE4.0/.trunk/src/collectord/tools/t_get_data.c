/*
 *
 * t_get_data.c
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include "e_define.h"
#include "d_define.h"
#include "d_nodes.h"
#include "u_util.h"
#include "u_shm.h"
#include "u_mutex.h"
#include "u_log.h"
#include "t_define.h"
#include "t_process_opt.h"
#include "t_get_data.h"


static int node_name_compare ( const void * a, const void * b ) {
    return strcmp ( ( ( node_status_info_t * ) a ) -> node_name,
            ( ( node_status_info_t * ) b ) -> node_name );
}

static const char * node_status_str [ 3 ] =  {
      " DOWN ",
      "ACTIVE",
     "NOT FOUND"
};


char ( * g_data ) [ MAX_GET_ITEMS_NUM ][ MAX_GET_DATA_LEN ];


int main ( int argc, char * argv [ ] ) {

    int ret;
    char errmsg [ CLRD_ERRMSG_LEN ];

    ret = check_pid ( CLRD_PID_FILE_PATH );
    if ( ret < 0 ) {
        fprintf ( stderr, "Collecotrd not started\n" );
        exit ( 9 );
    }

    char node_names [ MAX_NODES_NUM + 32 ] [ MAX_NODE_NAME_LEN ];
    int sort_nodes_num = 0;
    int opt_nodes_count = 0;

    if ( argc > 1 ) {
        opt_nodes_count = t_process_cmd_options ( argc, argv, node_names, MAX_NODES_NUM + 32 );
        if ( opt_nodes_count <= 0 ) {
            fprintf ( stderr, "Invalid parameters\n" );
            exit ( 1 );
        }
        sort_nodes_num = opt_nodes_count;

    }

    int tr_shmid;
    int tr_mutex_id;
    void * nl_shmaddr = 0x0;
    node_list_t * tr_node_list;

    tr_shmid = u_shm_attach ( CLRD_SHM_KEY, sizeof ( node_list_t ), & nl_shmaddr );
    if ( tr_shmid < 0 ) {
        fprintf ( stderr, "Collecotrd not started\n" );
        strerror_r ( errno, errmsg, CLRD_ERRMSG_LEN );
        exit ( 2 );
    }
    if ( nl_shmaddr == 0x0 ) {
        fprintf ( stderr, "Shared memory addr is invalid\n" );
        exit ( 3 );
    }

    tr_node_list = ( node_list_t * ) nl_shmaddr;

    tr_mutex_id = u_mutex_attach ( CLRD_SEM_KEY );
    if ( tr_mutex_id < 0 ) {
        fprintf ( stderr, "Attach mutex error\n" );
        u_shm_detach ( nl_shmaddr );
        exit ( 4 );
    }

    LOG_START ( CLRD_LOG_PATH, LOG_DEBUG );

    node_status_info_t node_status_info [ NODE_STATUS_INFO_SIZE ];

    u_mutex_lock ( tr_mutex_id );

    if ( argc > 1 ) {
        int name_inx = -1;
        while ( ++ name_inx < opt_nodes_count ) {
            strcpy ( node_status_info [ name_inx ].node_name, node_names [ name_inx ] );
            int node_id;
            int node_status;
            node_id = nl_find_node_by_nd_name ( tr_node_list, node_names [ name_inx ] );
            if ( node_id == 0 ) {
                node_status_info [ name_inx ].node_status = NODE_NOT_FOUND;
                node_status_info [ name_inx ].node_ip [ 0 ] = '\0';
            }
            else {
                node_status = nl_get_node_status_by_nd_id ( tr_node_list, node_id );
                node_status_info [ name_inx ].node_status = node_status;
                nl_get_node_ip_str_by_nd_id ( 
                        node_status_info [ name_inx ].node_ip, tr_node_list, node_id );
            }
        }

    }
    else {
        ret = traversal_node_status_info ( tr_node_list, node_status_info, NODE_STATUS_INFO_SIZE );
        if ( ret < 0 ) {
            LOG ( LOG_ERROR, "%s","Traversal node status info error: %d", ret );
            sort_nodes_num = 0;
        }
        else {
            sort_nodes_num = ret;
        }
    }
 
    u_mutex_unlock ( tr_mutex_id );

    if ( sort_nodes_num == 0 ) {
        u_shm_detach ( nl_shmaddr );
        fprintf ( stderr, "no node in node list\n" );
        exit ( 5 );
    }

    qsort ( node_status_info, sort_nodes_num, sizeof ( node_status_info_t ), node_name_compare );

    const int total_nodes = sort_nodes_num;

    g_data = malloc ( ( total_nodes + 2 ) * MAX_GET_ITEMS_NUM * MAX_GET_DATA_LEN );

    if ( g_data == NULL ) {
        fprintf ( stderr, "malloc memory error\n" );
        exit ( 6 );
    }

    int dv_item_ix = 1;
    int dv_node_ix = 2;

    for ( dv_node_ix = 2; dv_node_ix < total_nodes + 2; dv_node_ix ++ ) {
        for ( dv_item_ix = 1; dv_item_ix < MAX_GET_ITEMS_NUM; dv_item_ix ++ ) {
            strcpy ( g_data [ dv_node_ix ] [ dv_item_ix ],  "-" );
        }
    }

    int total_node_ix = -1;
    int data_node_ix = 2 - 1;

    int * processed_nodes_num = ( int * ) ( g_data [ 1 ] [ 0 ] + 0 );
    int * processed_items_num = ( int * ) ( g_data [ 1 ] [ 0 ] + 4 );

    * processed_nodes_num = 0;
    * processed_items_num = 0;

    bzero ( g_data [ 0 ], MAX_GET_ITEMS_NUM*MAX_GET_DATA_LEN );

    while ( ++ total_node_ix < total_nodes ) {

        if ( node_status_info [ total_node_ix ].node_status != NODE_STATUS_ACTIVE ) {
            continue;
        }

        if ( * processed_nodes_num == MAX_GET_NODES_NUM ) {
            break;
        }

        strcpy ( g_data [ ++ data_node_ix ] [ 0 ], node_status_info [ total_node_ix ].node_name );
        int * node_name_str_len = ( int * ) g_data [ 0 ] [ 0 ];
        * node_name_str_len = max_int ( * node_name_str_len, strlen ( g_data [ data_node_ix ] [ 0 ] ) );

        node_rt_data_t get_node_data;
        int act_node_id;
        int get_data_ret;

        u_mutex_lock ( tr_mutex_id );
        act_node_id = nl_find_node_by_nd_name ( tr_node_list,
                node_status_info [ total_node_ix ].node_name );
        get_data_ret = nl_get_node_all_data_by_id ( tr_node_list, act_node_id, & get_node_data );
        u_mutex_unlock ( tr_mutex_id );

        if ( get_data_ret < 0 ) {
            data_node_ix --;
            continue;
        }

        int sct_ix = -1;

        while ( get_node_data.rt_node_data [ ++ sct_ix ].rt_script_name [ 0 ] != '\0' ) {

            char * get_sct_data = get_node_data.rt_node_data [ sct_ix ].rt_script_data;

            char * delim = " ^\t\r\n\f\v";
            char * saveptr, * token, * str; 

            for ( str = get_sct_data; ; str = NULL ) {

                token = strtok_r ( str, delim, & saveptr );
                if ( token == NULL ) {
                    break;
                }

                int eq_pos = 0;
                while ( token [ ++ eq_pos ] != '=' );

                token [ eq_pos ] = '\0';

                char * data_item_pos = token;
                char * data_value_pos = token + eq_pos + 1;

                int data_item_ix = 0;

                while ( ++ data_item_ix < * processed_items_num + 1 ) {
                    if ( strcmp ( g_data [ 1 ] [ data_item_ix ], data_item_pos ) == 0 ) {
                        break;
                    }
                }

                if ( data_item_ix < * processed_items_num + 1 ) { /* exist */
                    strcpy ( g_data [ data_node_ix ] [ data_item_ix ], data_value_pos );
                    * ( int * ) g_data [ 0 ] [ data_item_ix ] = \
                    max_int ( * ( int * ) g_data [ 0 ] [ data_item_ix ], 
                            strlen ( data_value_pos ) );
                }

                else if ( * processed_items_num < MAX_GET_ITEMS_NUM ) { /* new */
                    strcpy ( g_data [ 1 ] [ data_item_ix ], data_item_pos );
                    strcpy ( g_data [ data_node_ix ] [ data_item_ix ], data_value_pos );
                    * ( int * ) g_data [ 0 ] [ data_item_ix ] = \
                    max_int ( strlen ( data_item_pos ), strlen ( data_value_pos ) );
                    ( * processed_items_num ) ++;
                }

                else { /* full items */
                    ( * processed_nodes_num ) --;
                    break;
                }

            } /* for ( str = get_item_value; ; str = NULL ) */

        } /* while ( get_node_data.rt_node_data [ ++ sct_ix ].rt_script_name [ 0 ] != '\0' )*/

        ( * processed_nodes_num ) ++;

    } /* while ( ++ data_node_ix < total_nodes ) */

    if ( * processed_nodes_num == 0 ) {
        goto NOT_OUT_PUT_ACTIVE_NODES;
    }

    int out_put_item_ix;
    int out_put_node_ix;
    char out_put_fmt [ 16 ];
    const int OUT_PUT_ALIGN_LEN = 4;

    int dashed_line_len = 0;
    fprintf ( stdout, "\n" );
    for ( out_put_item_ix = 0; out_put_item_ix < * processed_items_num + 1; out_put_item_ix ++ ) {
        dashed_line_len = dashed_line_len + * ( int * ) g_data [ 0 ] [ out_put_item_ix ]
            + OUT_PUT_ALIGN_LEN;
    }
    int dashed_line_count = 0;
    for ( dashed_line_count = 0; dashed_line_count < dashed_line_len; dashed_line_count ++ ) {
        fprintf ( stdout, "-" );
    }
    fprintf ( stdout, "\n\n" );

    sprintf ( out_put_fmt, "%s%ds", "\%", * ( int * ) ( g_data [ 0 ] [ 0 ] ) + OUT_PUT_ALIGN_LEN );
    fprintf ( stdout, out_put_fmt, " " );
    for ( out_put_item_ix = 1; out_put_item_ix < * processed_items_num + 1; out_put_item_ix ++ ) {
        sprintf ( out_put_fmt, "%s%ds", "\%", 
                * ( int * ) ( g_data [ 0 ] [ out_put_item_ix ] ) + OUT_PUT_ALIGN_LEN );
        fprintf ( stdout, out_put_fmt, g_data [ 1 ] [ out_put_item_ix ] );
    }
    fprintf ( stdout, "\n" );

    sprintf ( out_put_fmt, "%s%ds", "\%", * ( int * ) ( g_data [ 0 ] [ 0 ] ) + OUT_PUT_ALIGN_LEN );
    fprintf ( stdout, out_put_fmt, " " );
    for ( out_put_item_ix = 1; out_put_item_ix < * processed_items_num + 1; out_put_item_ix ++ ) {
        sprintf ( out_put_fmt, "%s%ds", "\%", 
                * ( int * ) ( g_data [ 0 ] [ out_put_item_ix ] ) + OUT_PUT_ALIGN_LEN );
        char dash_line_of_item [ MAX_GET_DATA_LEN ];
        memset ( dash_line_of_item, '-', strlen ( g_data [ 1 ] [ out_put_item_ix ] ) );
        dash_line_of_item [ strlen ( g_data [ 1 ] [ out_put_item_ix ] ) ] = 0;
        fprintf ( stdout, out_put_fmt, dash_line_of_item );
    }
    fprintf ( stdout, "\n\n" );

    for ( out_put_node_ix = 2; out_put_node_ix < * processed_nodes_num + 2; out_put_node_ix ++ ) {
        sprintf ( out_put_fmt, "%s%ds", "\%-", * ( int * ) ( g_data [ 0 ] [ 0 ] ) + OUT_PUT_ALIGN_LEN );
        fprintf ( stdout, out_put_fmt, g_data [ out_put_node_ix ] [ 0 ] );
        for ( out_put_item_ix = 1; out_put_item_ix < * processed_items_num + 1; out_put_item_ix ++ ) {
            sprintf ( out_put_fmt, "%s%ds", "\%",
                    * ( int * ) ( g_data [ 0 ] [ out_put_item_ix ] ) + OUT_PUT_ALIGN_LEN );
            fprintf ( stdout, out_put_fmt, g_data [ out_put_node_ix ] [ out_put_item_ix ] );
        }
        fprintf ( stdout, "\n" );
    }

    fprintf ( stdout, "\n" );
    for ( dashed_line_count = 0; dashed_line_count < dashed_line_len; dashed_line_count ++ ) {
        fprintf ( stdout, "-" );
    }
    fprintf ( stdout, "\n" );
    fprintf ( stdout, "\n" );

NOT_OUT_PUT_ACTIVE_NODES:

    ;

    int other_node_ix = -1;
    while ( ++ other_node_ix < sort_nodes_num ) {
        if ( node_status_info [ other_node_ix ].node_status != NODE_STATUS_ACTIVE ) {
            break;
        }
    }
    if ( other_node_ix == sort_nodes_num ) {
        goto NOT_OUT_PUT_OTHER_NODES;
    }

    fprintf ( stdout, "\n-----------------------------------------\n" );

    other_node_ix = -1;
    while ( ++ other_node_ix < sort_nodes_num ) {
        if ( node_status_info [ other_node_ix ].node_status == NODE_STATUS_ACTIVE ) {
            continue;
        }
        fprintf ( stdout, "%-32s%6s\n", 
                node_status_info [ other_node_ix ].node_name, 
                node_status_str [ node_status_info [ other_node_ix ].node_status ] );
    }

    fprintf ( stdout, "-----------------------------------------\n\n" );

NOT_OUT_PUT_OTHER_NODES:

    if ( g_data != NULL ) {
        free ( g_data );
        g_data = NULL;
    }

    u_shm_detach ( nl_shmaddr );

    return 0;

}


/*end of file*/
