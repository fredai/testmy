/*
 *
 * t_check_status.c
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "e_define.h"
#include "d_define.h"
#include "d_nodes.h"
#include "t_define.h"
#include "t_process_opt.h"
#include "u_util.h"
#include "u_shm.h"
#include "u_mutex.h"
#include "u_log.h"


static const char * node_status_str [ 3 ] = {
    " DOWN ",
    "ACTIVE",
    "NOT FOUND"
};
 

static int node_name_compare ( const void * a, const void * b ) {
    return strcmp ( ( ( node_status_info_t * ) a ) -> node_name,
            ( ( node_status_info_t * ) b ) -> node_name );
}

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

    if ( argc > 1 ) {
        cret = t_process_cmd_options ( argc, argv, node_names, MAX_NODES_NUM + 32 );
        if ( cret < 0 ) {
            fprintf ( stderr, "Invalid parameters\n" );
            exit ( 1 );
        }
        nodes_num = cret;
    }

    tc_shmid = u_shm_attach ( CLRD_SHM_KEY, sizeof ( node_list_t ), & nl_shmaddr );
    if ( tc_shmid < 0 ) {
        fprintf ( stderr, "Collecotrd not started\n" );
        exit ( 2 );
    }
    if ( nl_shmaddr == 0x0 ) {
        fprintf ( stderr, "Shared memory addr is invalid\n" );
        exit ( 3 );
    }

    tc_node_list = ( node_list_t * ) nl_shmaddr;

    tc_mutex_id = u_mutex_attach ( CLRD_SEM_KEY );
    if ( tc_mutex_id < 0 ) {
        fprintf ( stderr, "Attach mutex error\n" );
        u_shm_detach ( nl_shmaddr );
        exit ( 4 );
    }

    LOG_START ( CLRD_LOG_PATH, LOG_DEBUG );

    node_status_info_t node_status_info [ NODE_STATUS_INFO_SIZE ];

    u_mutex_lock ( tc_mutex_id );

    if ( argc > 1 ) {
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
        exit ( 5 );
    }

    qsort ( node_status_info, nodes_num, sizeof ( node_status_info_t ), node_name_compare );

    fprintf ( stdout, "\n" );
    int ix = -1;
    while ( ++ ix < nodes_num ) {
        fprintf ( stdout, "%-32s%-32s%6s\n", 
                node_status_info [ ix ].node_name, 
                node_status_info [ ix ].node_ip, 
                node_status_str [ node_status_info [ ix ].node_status ] );
    }
    fprintf ( stdout, "\n" );

    u_shm_detach ( nl_shmaddr );

    exit ( 0 );

}










/*end of file*/
