/*
 *
 * c_collector.c
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "e_define.h"
#include "c_define.h"
#include "c_script.h"
#include "c_config.h"
#include "c_proc_sct.h"
#include "c_proc_cmd.h"
#include "u_util.h"

#define CS_LOG_PATH CLR_LOG_SCT_PATH
#define INIT_CONFIG_ERROR_MSG_LEN 1024
#define INIT_SCL_ERROR_MSG_LEN 1024


extern scl_t g_scl;
int g_sct_update_flag = 0;
extern clr_sct_conf_t g_sct_conf;
extern clr_cmd_conf_t g_cmd_conf;
pid_t g_sct_pid;


int main ( int argc, char * argv [ ] ) {

    int ret = 0;

    cs_log_start ( LOG_DEBUG );
    char init_config_error_msg [ INIT_CONFIG_ERROR_MSG_LEN ];
    ret = init_collector_config ( & g_sct_conf, & g_cmd_conf, init_config_error_msg );
    if ( ret < 0 ) {
        fprintf ( stderr, "%s\n", init_config_error_msg );
        exit ( 1 );
    }

    char init_scl_error_msg [ INIT_SCL_ERROR_MSG_LEN ];
    ret = init_script_list ( NODE_LIST_FILE_PATH, & g_scl, init_scl_error_msg );
    if ( ret < 0 ) {
        fprintf ( stderr, "%s\n", init_scl_error_msg );
        exit ( 2 );
    }
    cs_log_end;

    mode_t log_dir_mode = umask ( 0 );
    mkdir ( "/var/log/ex-collector", 0775 );
    umask ( log_dir_mode );

    int fork_result;
    fork_result = fork ( );
    if ( fork_result == -1 ) {
        exit ( 3 );
    }
    else if ( fork_result == 0 ) {
        process_sct_main ( );
        exit ( 0 );
    }

    g_sct_pid = fork_result;

    fork_result = fork ( );
    if ( fork_result == -1 ) {
        exit ( 4 );
    }
    else if ( fork_result == 0 ) {
        process_cmd_main ( );
        exit ( 0 );
    }

    exit ( 0 );

}





/*end of file*/
