/*
 *
 * d_nodes.c
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "e_define.h"
#include "d_define.h"
#include "d_nodes.h"
#include "u_util.h"
#include "u_log.h"

#define NODE_ARRAY_SIZE MAX_NODES_NUM
#define NODE_ARRAY_START_INDEX 0 
#define NODE_ARRAY_END_INDEX (NODE_ARRAY_START_INDEX+MAX_NODES_NUM-1)

#define NODE_NULL_VALUE 0x0

#define NODE_POS_NOT_IN_USE(nl,index) (IP_OF_NODE_LIST(nl,index)==NODE_NULL_VALUE)
#define NODE_POS_IS_IN_USE(nl,index) (IP_OF_NODE_LIST(nl,index)!=NODE_NULL_VALUE)

#define NODE_OF_LIST(nl,index) (nl->node_list[index].node_info)
#define IP_OF_NODE_LIST(nl,index) (NODE_OF_LIST(nl,index).node_ip)
#define NAME_OF_NODE_LIST(nl,index) (NODE_OF_LIST(nl,index).node_name)
#define STATUS_OF_NODE_LIST(nl,index) (nl->node_list[index].node_status)

#define NODE_COUNT_INCREASE(nl) (nl->nodes_num++)
#define NODE_COUNT_DECREASE(nl) (nl->nodes_num--)
#define NODE_NUM_OF_LIST(nl) (nl->nodes_num)


static void debug_print_node_list ( node_list_t * node_list );


int nl_insert_node ( node_list_t * node_list, node_info_t * node_info ) {

    if ( node_list == NULL || node_info == NULL ) {
        return -1;
    }

    nd_ip_t node_ip = node_info -> node_ip;

    int ay_inx = NODE_ARRAY_START_INDEX;
    while ( ay_inx <= NODE_ARRAY_END_INDEX ) {
        if ( NODE_POS_NOT_IN_USE ( node_list, ay_inx ) ) {
            IP_OF_NODE_LIST ( node_list, ay_inx ) = node_ip;
            strcpy ( NAME_OF_NODE_LIST ( node_list, ay_inx ), node_info -> node_name );
            NODE_COUNT_INCREASE ( node_list );
            return ay_inx;
        }
        ay_inx ++;
    }
    return -2; /*not insert*/
}


int nl_delete_node_by_ndinfo ( node_list_t * node_list, node_info_t * node_info ) {

    if ( node_list == NULL || node_info == NULL ) {
        return -1;
    }

    nd_ip_t node_ip = node_info -> node_ip;
    int ay_inx = NODE_ARRAY_START_INDEX;
    while ( ay_inx <= NODE_ARRAY_END_INDEX ) {
        if ( IP_OF_NODE_LIST ( node_list, ay_inx ) == node_ip ) {
            IP_OF_NODE_LIST ( node_list, ay_inx ) = NODE_NULL_VALUE;
            * ( NAME_OF_NODE_LIST ( node_list, ay_inx ) ) = '\0';
            NODE_COUNT_DECREASE ( node_list );
            return ay_inx;
        }
        ay_inx ++;
    }
    return -2; /*not delete*/
}


int nl_delete_node_by_nd_ip ( node_list_t * node_list, nd_ip_t ip_of_node ) {

    if ( node_list == NULL ) {
        return -1;
    }

    nd_ip_t node_ip = ip_of_node;
    int ay_inx = NODE_ARRAY_START_INDEX;
    while ( ay_inx <= NODE_ARRAY_END_INDEX ) {
        if ( IP_OF_NODE_LIST ( node_list, ay_inx ) == node_ip ) {
            IP_OF_NODE_LIST ( node_list, ay_inx ) = NODE_NULL_VALUE;
            * ( NAME_OF_NODE_LIST ( node_list, ay_inx ) ) = '\0';
            NODE_COUNT_DECREASE ( node_list );
            return ay_inx;
        }
        ay_inx ++;
    }
    return -2; /*not delete*/
}


int nl_delete_node_by_nd_id ( node_list_t * node_list, int node_id ) {

    if ( node_list == NULL || node_id < 0 ) {
        return -1;
    }

    int node_index = node_id;

    IP_OF_NODE_LIST ( node_list, node_index ) = NODE_NULL_VALUE;
    * ( NAME_OF_NODE_LIST ( node_list, node_index ) ) = '\0';
    NODE_COUNT_DECREASE ( node_list );
    bzero ( & ( node_list -> node_list [ node_id ] ), sizeof ( node_t ) );
    return node_index;

}


int nl_find_node_by_ndinfo ( node_list_t * node_list, node_info_t * node_info ) {

    if ( node_list == NULL || node_info == NULL ) {
        return -1;
    }

    nd_ip_t node_ip = node_info -> node_ip;
    
    int ay_inx = NODE_ARRAY_START_INDEX;
    while ( ay_inx <= NODE_ARRAY_END_INDEX ) {
        if ( IP_OF_NODE_LIST ( node_list, ay_inx ) == node_ip ) {
            return ay_inx;
        }
        ay_inx ++;
    }
    return -2; /*not find*/
}


int nl_find_node_by_nd_ip ( node_list_t * node_list, nd_ip_t ip_of_node ) {

    if ( node_list == NULL ) {
        return -1;
    }

    nd_ip_t node_ip = ip_of_node;

    int ay_inx = NODE_ARRAY_START_INDEX;
    while ( ay_inx <= NODE_ARRAY_END_INDEX ) {
        if ( IP_OF_NODE_LIST ( node_list, ay_inx ) == node_ip ) {
            return ay_inx;
        }
        ay_inx ++;
    }
    return -2; /*not find*/
}


int nl_find_node_by_nd_ip_str ( node_list_t * node_list, char * node_ip_str ) {

    if ( node_list == NULL || node_ip_str == NULL ) {
        return -1;
    }

    int ret;
    nd_ip_t ip_of_node;
    ret = ipv4_str_to_uint ( node_ip_str, & ip_of_node );
    if ( ret < 0 ) {
        return -2;
    }

    nd_ip_t node_ip = ip_of_node;
    int ay_inx = NODE_ARRAY_START_INDEX;
    while ( ay_inx <= NODE_ARRAY_END_INDEX ) {
        if ( IP_OF_NODE_LIST ( node_list, ay_inx ) == node_ip ) {
            return ay_inx;
        }
        ay_inx ++;
    }
    return -2; /*not find*/
}


int nl_find_node_by_nd_name ( node_list_t * node_list, char * node_name ) {

    if ( node_list == NULL || node_name == NULL ) {
        return -1;
    }

    int ix = 0;
    while ( ++ ix <= NODE_ARRAY_END_INDEX ) {
        if ( NODE_POS_IS_IN_USE ( node_list, ix ) ) {
            if ( strcmp ( NAME_OF_NODE_LIST ( node_list, ix ), node_name ) == 0 ) {
                return ix;
            }
        }
    }

    return -2; /*not find*/
}


int nl_get_node_status_by_nd_id ( node_list_t * node_list, int node_id ) {

    if ( node_list == NULL ) {
        return -1;
    }
    
    if ( node_id < 0 ) {
        return -2;
    }

    return node_list -> node_list [ node_id ].node_status;
}


int nl_set_node_status_by_nd_id ( node_list_t * node_list, int node_id, node_status_t nd_st ) {

    if ( node_list == NULL ) {
        return -1;
    }
    
    if ( node_id < 0 ) {
        return -2;
    }

    node_list -> node_list [ node_id ].node_status = nd_st;

    return node_id;

}


int nl_get_node_istn_by_nd_id ( node_list_t * node_list, int node_id ) {

    if ( node_list == NULL ) {
        return -1;
    }
    
    if ( node_id < 0 ) {
        return -2;
    }

    return node_list -> node_list [ node_id ].node_istn;
}


int nl_set_node_istn_by_nd_id ( node_list_t * node_list, int node_id, node_istn_t node_istn ) {

    if ( node_list == NULL ) {
        return -1;
    }
    
    if ( node_id < 0 ) {
        return -2;
    }

    node_list -> node_list [ node_id ].node_istn = node_istn;

    return node_id;

}


int init_node_list ( file_nodes_t * file_nodes, node_list_t * node_list ) {

    LOG_START ( CLRD_LOG_PATH, LOG_DEBUG );

    if ( file_nodes == NULL || node_list == NULL ) {
        LOG ( LOG_ERROR, "%s", "Invalid parameters" );
        return -1;
    }

    LOG ( LOG_INFO, "%s", "Start init node list" );

    char ip_str [ MAX_IP_STR_LEN ];

    int ret;

    memset ( node_list, 0, sizeof ( node_list_t ) );

    int file_node_index = -1;

    while ( ++ file_node_index < file_nodes -> nodes_num ) {

        ret = nl_insert_node ( node_list, & ( file_nodes -> nodes [ file_node_index ] ) );
        ipv4_uint_to_str ( file_nodes -> nodes [ file_node_index ].node_ip, ip_str, MAX_IP_STR_LEN );
        if ( ret >= 0 ) {
            LOG ( LOG_DEBUG, "Insert node %s success: %d", ip_str, ret );
        }
        else if ( ret == -2 ) {
            LOG ( LOG_ERROR, "Insert node %s error: have not insert", ip_str );
        }
        else if ( ret == -1 ) {
            LOG ( LOG_ERROR, "Insert node %s error: invalid parameters", ip_str );
        }

    }
    LOG ( LOG_DEBUG, "%s", "Debug print node list ---start---" );
    debug_print_node_list ( node_list );
    LOG ( LOG_DEBUG, "%s", "Debug print node list ---end---" );
    LOG_END;
    return 0;

}


int destroy_node_list ( node_list_t * node_list ) {

    if ( node_list == NULL ) {
        return -1;
    }

    memset ( node_list, 0, sizeof ( node_list_t ) );

    return 0;
}


int update_node_list ( dy_nodes_t * dy_nodes, node_list_t * node_list ) {

    LOG_START ( CLRD_LOG_PATH, LOG_DEBUG );

    if ( dy_nodes == NULL || node_list == NULL ) {
        LOG ( LOG_ERROR, "%s", "Invalid parameters" );
        return -1;
    }

    LOG ( LOG_INFO, "%s", "Start update node list" );
    int ret_index = 0;
    int ret;
    int added_node_index = -1;
    int deleted_node_index = -1;

    char ip_str [ MAX_IP_STR_LEN ];

    while ( ++ added_node_index < dy_nodes -> added_nodes_num ) {
        ipv4_uint_to_str ( \
                    dy_nodes -> added_nodes [ added_node_index ].node_ip, \
                    ip_str, MAX_IP_STR_LEN );
        ret_index = nl_insert_node ( node_list, & ( dy_nodes -> added_nodes [ added_node_index ] ) );
        if ( ret_index >= 0 ) {
            LOG ( LOG_DEBUG, "Insert node %s success: %d", ip_str, ret_index );
            ret = nl_set_node_status_by_nd_id ( node_list, ret_index, NODE_STATUS_DOWN );
            if ( ret < 1 ) {
                LOG ( LOG_ERROR, "Set node %s status init to DOWN error: %d", ip_str, ret );
            }
            else {
                LOG ( LOG_DEBUG, "Set node %s status init to DOWN success: %d", ip_str, ret );
            }
            ret = nl_set_node_istn_by_nd_id ( node_list, ret_index, NODE_RECV_THREAD_RUN );
            if ( ret < 1 ) {
                LOG ( LOG_ERROR, "Set node %s istn to run error: %d", ip_str, ret );
            }
            else {
                LOG ( LOG_DEBUG, "Set node %s istn to run success: %d", ip_str, ret );
            }
        }
        else {
            LOG ( LOG_ERROR, "Insert node %s error: %d", ip_str, ret_index );
        }
    }

    while ( ++ deleted_node_index < dy_nodes -> reduced_nodes_num ) {
        ipv4_uint_to_str ( \
            dy_nodes -> reduced_nodes [ deleted_node_index ].node_ip, \
            ip_str, MAX_IP_STR_LEN );
        ret_index = nl_find_node_by_ndinfo ( node_list, & ( dy_nodes -> reduced_nodes [ deleted_node_index ] ) );
        if ( ret_index >= 0 ) {
            if ( NODE_STATUS_DOWN == nl_get_node_status_by_nd_id ( node_list, ret_index ) ) {
                LOG ( LOG_DEBUG, "Node %s status DOWN", ip_str );
                ret = nl_delete_node_all_data_by_id ( node_list, ret_index );
                LOG ( LOG_DEBUG, "Delete node %s all data: %d", ip_str, ret );
                ret = nl_delete_node_by_nd_id ( node_list, ret_index );
                LOG ( LOG_DEBUG, "Delete node %s: %d", ip_str, ret );
            }
            else {
                LOG ( LOG_DEBUG, "Find node %s success: set instruction to exit", ip_str );
                ret = nl_set_node_istn_by_nd_id ( node_list, ret_index, NODE_RECV_THREAD_EXIT );
                if ( ret < 0 ) {
                    LOG ( LOG_ERROR, "Set node %s istn to exit error: %d", ip_str, ret );
                }
                else {
                    LOG ( LOG_DEBUG, "Set node %s istn to exit success: %d", ip_str, ret );
                }
            }
        }
        else {
            LOG ( LOG_ERROR, "Find node %s error: %d", ip_str, ret_index );
        }
    }

    LOG ( LOG_DEBUG, "%s", "Debug print node list ---start---" );
    debug_print_node_list ( node_list );
    LOG ( LOG_DEBUG, "%s", "Debug print node list ---end---" );
    LOG ( LOG_INFO, "%s", "End update node list" );
    LOG_END;
    return 0;
}


int get_file_nodes ( const char * node_file_path, file_nodes_t * file_nodes ) {

    LOG_START ( CLRD_LOG_PATH, LOG_DEBUG );
    if ( node_file_path == NULL || file_nodes == NULL || node_file_path [ 0 ] == '\0' ) {
        LOG ( LOG_ERROR, "%s", "Invalid parameters" );
        return -1;
    }

    LOG ( LOG_INFO, "%s", "Start get file nodes" );

    int ret = -1;
    int str_len = 0;
    FILE * nd_file;
    const int padding_len = 1 + 5;
    char line_buffer [ MAX_NODE_FILE_LINE_LEN + padding_len ];

    nd_file = fopen ( node_file_path, "r" );
    if ( nd_file == NULL ) {
        LOG ( LOG_ERROR, "Open file %s error", node_file_path );
        return -2;
    }

    LOG ( LOG_DEBUG, "Open file %s success", node_file_path );

    file_nodes -> nodes_num  = 0;

    while ( fgets ( line_buffer, MAX_NODE_FILE_LINE_LEN, nd_file ) ) {

        str_len = strlen ( line_buffer );
        line_buffer [ str_len - 1 ] = 0;
        str_len --; 

        if ( line_buffer [ str_len - 2 ] == '\r' ) { 
            line_buffer [ str_len - 2 ] = 0;
            str_len --; 
        }   

        if ( str_len > MAX_NODE_FILE_LINE_LEN ) { 
            LOG ( LOG_ERROR, "Line too long: %d > %d", str_len, MAX_NODE_FILE_LINE_LEN );
            continue;
        }

        char * line_buffer_trim_head = trim_head ( line_buffer, str_len );

        if ( * line_buffer_trim_head == '#' ) {
            continue;
        }
        if ( * line_buffer_trim_head == 0 ) {
            continue;
        }

        char * node_start = line_buffer_trim_head;

        int node_len = 0;
        
        while ( ( * ( node_start + node_len ) ) != '\0' ) {
            if ( isspace ( ( int ) ( * ( node_start + node_len ) ) ) ) {
                break;
            }
            node_len ++;
        }

        node_info_t tmp_node;

        strncpy ( tmp_node.node_name, node_start, node_len );

        tmp_node.node_name [ node_len ] = '\0';

        nd_ip_t ip;

        ret = hostname_to_ip ( tmp_node.node_name, & ip );

        if ( ret < 0 ) {
            LOG ( LOG_ERROR, "Can't resolve node name: %s", tmp_node.node_name );
            continue;
        }

        char ip_str [ MAX_IP_STR_LEN ];

        tmp_node.node_ip = ip;

        int node_ix = -1;

        node_info_t * f_node_p = NULL;

        while ( ++ node_ix < file_nodes -> nodes_num ) {
            if ( file_nodes -> nodes [ node_ix ].node_ip == ip ) {
                ipv4_uint_to_str ( ip, ip_str, MAX_IP_STR_LEN );
                LOG ( LOG_ERROR, "Node repeated %s %s", tmp_node.node_name, ip_str );
                break;
            }
        }

        if ( node_ix == file_nodes -> nodes_num ) {
            if ( file_nodes -> nodes_num < MAX_FILE_NODES_NUM ) {
                f_node_p = & ( file_nodes -> nodes [ file_nodes -> nodes_num ] );
                memcpy ( ( void * ) f_node_p, ( void * ) & tmp_node, sizeof ( node_info_t ) );
                file_nodes -> nodes_num ++;
                ipv4_uint_to_str ( ip, ip_str, MAX_IP_STR_LEN );
                LOG ( LOG_DEBUG, "Add node %s %s node count %d",
                        tmp_node.node_name, ip_str, file_nodes -> nodes_num );
            }
            else {
                LOG ( LOG_ERROR, "Node count reached the maximum number of %d", MAX_FILE_NODES_NUM );
            }
        }

    }

    fclose ( nd_file );

    LOG ( LOG_INFO, "%s", "End get file nodes" );
    LOG_END;
    return 0;

}


int dy_diff_nodes ( file_nodes_t * file_nodes, node_list_t * node_list, dy_nodes_t * dy_nodes ) {

    if ( file_nodes == NULL || node_list == NULL || dy_nodes == NULL ) {
        return -1;
    }

    int ret_index = 0;
    int file_node_index = -1;
    int node_list_index = 0;

    unsigned int * added_nodes_num = & ( dy_nodes -> added_nodes_num );
    unsigned int * reduced_nodes_num = & ( dy_nodes -> reduced_nodes_num );

    * added_nodes_num = 0;
    * reduced_nodes_num = 0;

    while ( ++ file_node_index < file_nodes -> nodes_num ) {
        ret_index = nl_find_node_by_ndinfo ( node_list, 
                & ( file_nodes -> nodes [ file_node_index ] ) );
        if ( ret_index == -2 ) {
            dy_nodes -> added_nodes [ * added_nodes_num ].node_ip = \
                file_nodes -> nodes [ file_node_index ].node_ip;

            strcpy ( dy_nodes -> added_nodes [ * added_nodes_num ].node_name, 
                    file_nodes -> nodes [ file_node_index ].node_name );
            ( * added_nodes_num ) ++;
        }
        else {
        }
    }

    node_list_index = -1;

    while ( ++ node_list_index < NODE_ARRAY_SIZE ) {
        if ( NODE_POS_IS_IN_USE ( node_list, node_list_index ) ) {
            file_node_index = -1;
            while ( ++ file_node_index < file_nodes -> nodes_num ) {
                if ( IP_OF_NODE_LIST ( node_list, node_list_index ) == \
                        file_nodes -> nodes [ file_node_index ].node_ip ) {
                    break;
                }
            }
            if ( file_node_index == file_nodes -> nodes_num ) {

                dy_nodes -> reduced_nodes [ * reduced_nodes_num ].node_ip = \
                    IP_OF_NODE_LIST ( node_list, node_list_index );
                ( * reduced_nodes_num ) ++;
            }
        }
        else {
        }
    }

    return ( * added_nodes_num ) + ( * reduced_nodes_num );

}


static void debug_print_node_list ( node_list_t * node_list ) {
    LOG_START ( CLRD_LOG_PATH, LOG_DEBUG );
    LOG ( LOG_DEBUG, "%s", "node_name\tnode_ip\tnode_status\tnode_istn" );
    int ix = -1;
    char ip_str [ MAX_IP_STR_LEN ];
    while ( ++ ix < NODE_ARRAY_SIZE ) {
        if ( NODE_POS_IS_IN_USE ( node_list, ix ) ) {
            ipv4_uint_to_str ( IP_OF_NODE_LIST ( node_list, ix ), ip_str, MAX_IP_STR_LEN );
            LOG ( LOG_DEBUG, "%s\t%s\t%d\t%d",
                    NAME_OF_NODE_LIST ( node_list, ix ), ip_str,
                    node_list -> node_list [ ix ].node_status,
                    node_list -> node_list [ ix ].node_istn );
        }
    }
    LOG_END;
}


int nl_get_node_script_data_by_nd_id ( 
        node_list_t * node_list, int node_id,
        char * script_name, char * out_script_data ) {

    if ( node_list == NULL || script_name == NULL ||
            out_script_data == NULL || script_name [ 0 ] == '\0' || node_id <= 0 ) {
        return -1;
    }

    node_t * node = & ( node_list -> node_list [ node_id ] );

    node_rt_data_t * node_data = & ( node -> node_data );

    script_rt_data_t * script_data = node_data -> rt_node_data;
    
    int sc_ix = -1;
    while ( ++ sc_ix < MAX_SCRIPT_NUM_PER_NODE ) {
        if ( time ( NULL ) - script_data [ sc_ix ].rt_data_time > node_data -> discard_time ) {
            script_data -> rt_script_name [ 0 ] = '\0';
        }
    }

    sc_ix = -1;
    while ( ++ sc_ix < MAX_SCRIPT_NUM_PER_NODE ) {
        if ( strcmp ( script_data -> rt_script_name, script_name ) == 0 ) {
            break;
        }
    }

    if ( sc_ix == MAX_SCRIPT_NUM_PER_NODE ) {
        /* find script name error, have not found */
        return -2;
    }

    strcpy ( out_script_data, script_data -> rt_script_data );

    return 0;

}


int nl_get_node_all_data_by_id ( 
        node_list_t * node_list, int node_id,
        node_rt_data_t * p_node_data ) {

    if ( node_list == NULL || p_node_data == NULL || node_id <= 0 ) {
        return -1;
    }

    node_t * node = & ( node_list -> node_list [ node_id ] );
    node_rt_data_t * data = & ( node -> node_data );
    script_rt_data_t * script_data = data -> rt_node_data;
    
    int sc_ix = -1;
    int da_ix = -1;
 
    while ( ++ sc_ix < MAX_SCRIPT_NUM_PER_NODE ) {
        if ( time ( NULL ) - script_data [ sc_ix ].rt_data_time > data -> discard_time ) {
            script_data [ sc_ix ].rt_script_name [ 0 ] = '\0';
        }
    }

    sc_ix = -1;
    da_ix = -1;
    bzero ( p_node_data, sizeof ( node_rt_data_t ) );
    while ( ++ sc_ix < MAX_SCRIPT_NUM_PER_NODE ) {
        if ( script_data [ sc_ix ].rt_script_name [ 0 ] != '\0' ) {
            memcpy ( & ( p_node_data -> rt_node_data [ ++ da_ix ] ),
                    & ( script_data [ sc_ix ] ),
                    sizeof ( script_rt_data_t ) );
        }
    }

    return 0;

}


int nl_put_node_script_data_by_id (
        char * script_name, char * in_script_data,
        node_list_t * node_list, int node_id ) {

    if ( node_list == NULL || script_name == NULL || 
            in_script_data == NULL || script_name [ 0 ] == '\0' || node_id < 0 ) {
        return -1;
    }

    node_t * node = & ( node_list -> node_list [ node_id ] );
    node_rt_data_t * node_data = & ( node -> node_data );
    script_rt_data_t * script_data = node_data -> rt_node_data;
    int sc_ix = -1;

    while ( ++ sc_ix < MAX_SCRIPT_NUM_PER_NODE ) {
        if ( strcmp ( script_data [ sc_ix ].rt_script_name, script_name ) == 0 ) {
            break;
        }
    }

    if ( sc_ix < MAX_SCRIPT_NUM_PER_NODE ) { /*exist overwrite*/
        strcpy ( script_data [ sc_ix ].rt_script_data, in_script_data );
        script_data [ sc_ix ].rt_data_time = time ( NULL );
    }

    else { /* not exist */
        sc_ix = -1;
        while ( ++ sc_ix < MAX_SCRIPT_NUM_PER_NODE ) {
            if ( script_data [ sc_ix ].rt_script_name [ 0 ] == '\0' ) {
                break;
            }
        }
        if ( sc_ix == MAX_SCRIPT_NUM_PER_NODE ) { /*full*/
            int first_pos = -1;
            sc_ix = -1;
            while ( ++ sc_ix < MAX_SCRIPT_NUM_PER_NODE ) {
                if ( time ( NULL ) - script_data [ sc_ix ].rt_data_time > node_data -> discard_time ) {
                    script_data [ sc_ix ].rt_script_name [ 0 ] = '\0';
                    if ( first_pos == -1 ) {
                        first_pos = sc_ix;
                    }
                }
            }

            if ( first_pos != -1 ) {
                sc_ix = first_pos;
                goto ADD_NEW_SCT_DATA;
            }
            else {
                /* script num reached the maximum number of MAX_SCRIPT_NUM_PER_NODE */
                return -4;
            }
        }
        else { /*have position add new*/
ADD_NEW_SCT_DATA:
            strcpy ( script_data [ sc_ix ].rt_script_name, script_name );
            strcpy ( script_data [ sc_ix ].rt_script_data, in_script_data );
            script_data [ sc_ix ].rt_data_time = time ( NULL );
            /*node_data -> script_num ++;*/
        }
    }

    return 0;

}

int nl_delete_node_all_data_by_id ( node_list_t * node_list, int node_id ) {

    if ( node_list == NULL || node_id <= 0 ) {
        return -1;
    }

    bzero ( & ( node_list -> node_list [ node_id ].node_data ), sizeof ( node_rt_data_t ) );

    return 0;
 
}


int nl_get_node_name_by_nd_id ( char * node_name, node_list_t * node_list, int node_id ) {

    if ( node_list == NULL || node_name == NULL || node_id < 0 ) {
        return -1;
    }
    
    strcpy ( node_name, node_list -> node_list [ node_id ].node_info.node_name );

    return 0;
}


int traversal_node_status_info ( node_list_t * node_list,
        node_status_info_t * node_status_info, size_t node_status_info_size ) {

    if ( node_list == NULL || node_status_info == NULL || node_status_info_size < 1 ) {
        return -1;
    }

    int nix = -1;

    int nsif_ix = 0;

    while ( ++ nix < NODE_ARRAY_SIZE ) {
        if ( NODE_POS_IS_IN_USE ( node_list, nix ) ) {
            strcpy ( node_status_info [ nsif_ix ].node_name, NAME_OF_NODE_LIST ( node_list, nix ) );
            ipv4_uint_to_str ( IP_OF_NODE_LIST ( node_list, nix ), 
                    node_status_info [ nsif_ix ].node_ip, MAX_IP_STR_LEN );
            node_status_info [ nsif_ix ].node_status = STATUS_OF_NODE_LIST ( node_list, nix );
            nsif_ix ++;
            if ( nsif_ix == node_status_info_size ) {
                break;
            }
        }
    }

    return nsif_ix;

}


int nl_get_node_ip_str_by_nd_id ( char * node_ip_str, node_list_t * node_list, int node_id ) {

    if ( node_list == NULL || node_ip_str == NULL || node_id < 0 ) {
        return -1;
    }
    
    ipv4_uint_to_str ( IP_OF_NODE_LIST ( node_list, node_id ), node_ip_str, MAX_IP_STR_LEN );

    return 0;
}


int nl_set_node_data_discard_time ( node_list_t * node_list, int node_id, int discard_time ) {

    if ( node_list == NULL || node_id < 0 || discard_time <= 0 ) {
        return -1;
    }

    node_list -> node_list [ node_id ].node_data.discard_time = discard_time;

    return 0;

}


int nl_get_node_data_discard_time ( node_list_t * node_list, int node_id, int * discard_time ) {

    if ( node_list == NULL || node_id < 0 || discard_time <= 0 ) {
        return -1;
    }

    * discard_time = node_list -> node_list [ node_id ].node_data.discard_time;

    return 0;

}


int nl_get_node_threadid_by_nd_id ( node_list_t * node_list, int node_id, pthread_t * thread_id ) {

    if ( node_list == NULL || thread_id == NULL || node_id < 0 ) {
        return -1;
    }
    
    * thread_id = node_list -> node_list [ node_id ].node_ctl.thread_id;

    return 0;
}


int nl_set_node_threadid_by_nd_id ( node_list_t * node_list, int node_id, pthread_t thread_id ) {

    if ( node_list == NULL || node_id < 0 ) {
        return -1;
    }
    
    node_list -> node_list [ node_id ].node_ctl.thread_id = thread_id;

    return 0;
}




/*end of file*/
