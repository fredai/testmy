/*
 *
 * c_script.c
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "e_define.h"
#include "c_define.h"
#include "c_script.h"
#include "c_debug_log.h"
#include "u_util.h"

#define CS_LOG_PATH CLR_LOG_SCT_PATH

int scl_adjust ( scl_t * p_scl );
static int check_script_list ( scl_t * scl );
static int get_file_scripts ( const char * node_file_path, scl_t * scl );

static int get_file_scripts ( const char * node_file_path, scl_t * scl ) {

    if ( node_file_path == NULL || scl == NULL || node_file_path [ 0 ] == '\0' ) {
        cs_log ( LOG_ERROR, "Invalid parameters" );
        return -1;
    }

    int scl_str_len = 0;
    FILE * scl_file;
    char scl_errmsg [ CLR_ERRMSG_LEN ];
    const int padding_len = 1 + 5;
    char scl_line_buffer [ MAX_NODE_FILE_LINE_LEN + padding_len ];
    char my_node_name [ MAX_NODE_NAME_LEN + 5 ];
    unsigned int my_ip_uint;
    char my_ip_str [ MAX_IP_STR_LEN ];

    gethostname ( my_node_name, MAX_NODE_NAME_LEN + 5 );
    if ( ( strlen ( my_node_name ) > MAX_NODE_NAME_LEN ) ) {
        cs_log ( LOG_ERROR, "Node's hostname %s too long > %d", my_node_name, MAX_NODE_NAME_LEN );
        return -2;
    }

    hostname_to_ip ( my_node_name, & my_ip_uint );
    ipv4_uint_to_str ( my_ip_uint, my_ip_str, MAX_IP_STR_LEN );

    bzero ( scl, sizeof ( scl_t ) );

    scl_file = fopen ( node_file_path, "r" );
    if ( scl_file == NULL ) {
        strerror_r ( errno, scl_errmsg, CLR_ERRMSG_LEN );
        if ( errno == ENOENT ) {
            cs_log ( LOG_ERROR, "Open nodelist file %s error: file not exsit, %s",
                    node_file_path, scl_errmsg );
            return -3; /* file not exist */
        }
        else {
            cs_log ( LOG_ERROR, "Open sct file error: %d %s", errno, scl_errmsg );
            return -4; /* other error */
        }
    }

    scl -> sc_num = 0;
    int found = 0;
    char * node_start;
    int node_len;

    while ( fgets ( scl_line_buffer, MAX_NODE_FILE_LINE_LEN + padding_len, scl_file ) ) {

        scl_str_len = strlen ( scl_line_buffer );

        if ( scl_str_len > 0 && scl_line_buffer [ scl_str_len - 1 ] == '\n' ) {
            scl_line_buffer [ scl_str_len - 1 ] = 0;
            scl_str_len --;
        }
        if ( scl_str_len > 1 && scl_line_buffer [ scl_str_len - 2 ] == '\r' ) {
            scl_line_buffer [ scl_str_len - 2 ] = 0;
            scl_str_len --;
        }

        if ( scl_str_len > MAX_NODE_FILE_LINE_LEN ) { /* line too long */
            cs_log ( LOG_ERROR, "Line too long, ignore" );
            continue;
        }

        char * line_buffer_trim_head = trim_head ( scl_line_buffer, scl_str_len );

        if ( * line_buffer_trim_head == '#' ) {
            continue;
        }
        if ( * line_buffer_trim_head == 0 ) {
            continue;
        }

        char node_name [ MAX_NODE_NAME_LEN ];
        node_start = line_buffer_trim_head;
        node_len = 0;
        while ( ( * ( node_start + node_len ) ) != '\0' ) {
            if ( isspace ( ( int ) ( * ( node_start + node_len ) ) ) ) {
                break;
            }
            node_len ++;
        }

        strncpy ( node_name, node_start, node_len );
        node_name [ node_len ] = '\0';
        if ( strcmp ( my_node_name, node_name ) == 0 || strcmp ( my_ip_str, node_name ) == 0 ) {
            found = 1;
            break;
        }
    } /* while */

    fclose ( scl_file );

    if ( found == 0 ) {
        cs_log ( LOG_ERROR, "Node %s not in nodelist file", my_node_name );
        return -5;
    }

    char * script_name_start;
    script_name_start = node_start + node_len;

    int sc_ix = 0;
    char * delim = " \t\r\n\f\v";
    char * saveptr, * token, * str;
                        
    for ( str = script_name_start; ; str = NULL ) { 
        token = strtok_r ( str, delim, & saveptr );
        if ( token == NULL ) { 
            break;
        }   
        if ( sc_ix > MAX_SCRIPT_NUM_PER_NODE - 1 ) {
            cs_log ( LOG_WARN, "Script reached the max number of %d, ignore other scripts",
                    MAX_SCRIPT_NUM_PER_NODE );
            /*return -5;*/
            break;/*ignore*/
        }
        sprintf ( scl -> sc_names [ sc_ix ], "%s%s", SCRIPT_FILES_PATH, token );
        sc_ix ++;
        scl -> sc_num ++;

    }

    return scl -> sc_num;

}


static int check_script_list ( scl_t * scl ) {

    if ( scl == NULL ) {
        return -1;
    }

    int affect_sc_num = 0;
    char * sc_name = NULL;
    int sc_ix = -1;
    int ret;
    const int sc_count = scl -> sc_num;

    while ( ++ sc_ix < sc_count ) {

        sc_name = scl -> sc_names [ sc_ix ];
        ret = access ( sc_name, X_OK|F_OK );
        if ( ret == -1 ) {
            cs_log ( LOG_ERROR, "Script %s not exist, or no execute permision", sc_name );
            sc_name [ 0 ] = '\0';
            scl -> sc_num --;
            affect_sc_num ++;
        }
    }

    return affect_sc_num; /* > 0 means some sct name is invalid */
                          /* == 0 means all sct are valid */

}


int scl_adjust ( scl_t * p_scl ) {

    if ( p_scl == NULL ) {
        return -1;
    }

    if ( p_scl -> sc_num == 0 ) {
        return 0;
    }

    scl_t a_scl;

    memcpy ( & a_scl, p_scl, sizeof ( scl_t ) );

    bzero ( p_scl, sizeof ( scl_t ) );

    int p_sc_ix = -1;
    int a_sc_ix = -1;

    while ( ++ a_sc_ix < MAX_SCRIPT_NUM_PER_NODE ) {
        if ( a_scl.sc_names [ a_sc_ix ] [ 0 ] != '\0' ) {
            strcpy ( p_scl -> sc_names [ ++ p_sc_ix ], a_scl.sc_names [ a_sc_ix ] );
        }
    }

    return 0;
}


int init_script_list ( char * scl_path, scl_t * scl, char * init_output_msg  ) {

    if ( scl_path == NULL || scl == NULL || scl_path [ 0 ] == '\0' ) {
        cs_log ( LOG_ERROR, "Invalid parameters" );
        return -1;
    }

    int ret;
    char scl_errmsg [ CLR_ERRMSG_LEN ];

    ret = get_file_scripts ( scl_path, scl );
    switch ( ret ) {
        case -1:
            cs_log ( LOG_ERROR, "Get file sct invalid parameters" );
            return -2;
        case -2:
            cs_log ( LOG_ERROR, "Node's hostname too long > %d", MAX_NODE_NAME_LEN );
            sprintf ( init_output_msg, "Node's hostname too long > %d", MAX_NODE_NAME_LEN );
            return -3;
        case -3:
            cs_log ( LOG_ERROR, "Open nodelist file %s error: file not exsit", scl_path );
            sprintf ( init_output_msg, "Open nodelist file %s error: file not exsit", scl_path );
            return -4;
        case -4:
            cs_log ( LOG_ERROR, "Open sct file %s error: %d %s",
                    scl_path, errno, scl_errmsg );
            sprintf ( init_output_msg, "Open sct file %s error: %d %s",
                    scl_path, errno, scl_errmsg );
            return -5;
        case -5:
            cs_log ( LOG_ERROR, "This node not in nodelist file" );
            sprintf ( init_output_msg, "This node not in nodelist file" );
            return -6;
    }

    if ( ret == 0 ) {
        cs_log ( LOG_ERROR, "No script can be executed" );
        sprintf ( init_output_msg, "No script can be executed" );
        return -7;
    }

    ret = check_script_list ( scl );
    if ( ret < 0 ) {
        cs_log ( LOG_ERROR, "Check script error: %d", ret );
        sprintf ( init_output_msg, "Check script error: %d", ret );
        return -8;
    }
    else if ( ret > 0 ) {
        cs_log ( LOG_ERROR, "Some scripts not exist or no exexute permision" );
        sprintf ( init_output_msg, "Some scripts not exist or no exexute permision" );
        return -9;
    }

    return 0;

}


int update_script_list ( char * scl_path, scl_t * scl ) {

    if ( scl_path == NULL || scl == NULL || scl_path [ 0 ] == '\0' ) {
        cs_log ( LOG_ERROR, "Invalid parameters" );
        return -1;
    }

    int ret;
    scl_t new_scl;
    char scl_errmsg [ CLR_ERRMSG_LEN ];

    ret = get_file_scripts ( scl_path, & new_scl );
    switch ( ret ) {
        case -1:
            cs_log ( LOG_ERROR, "Get file sct invalid parameters" );
            return -2; /* ignore */
        case -2:
            cs_log ( LOG_ERROR, "Node's hostname too long > %d", MAX_NODE_NAME_LEN );
            return -3; /* ignore */
        case -3:
            cs_log ( LOG_ERROR, "Open nodelist file %s error: file not exsit, %s",
                    scl_path, scl_errmsg );
            return -4; /* ignore */
        case -4:
            cs_log ( LOG_ERROR, "Open sct file %s error: %d %s", scl_path, errno, scl_errmsg );
            return -5; /* ignore */
        case -5:
            cs_log ( LOG_ERROR, "This node not in nodelist file" );
            return -6; /* close and exit */
        case 0:
            cs_log ( LOG_ERROR, "After read file, no script can be executed" );
            return -7; /* close and exit */
    }

    ret = check_script_list ( & new_scl );
    if ( ret > 0 ) {
        cs_log ( LOG_ERROR, "Some scripts not exist or no exexute permision" );
        return -9; /* ignore */
    }

    memcpy ( scl, & new_scl, sizeof ( scl_t ) );

    return 0;

}



/*end of file*/
