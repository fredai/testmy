/*
 *
 * c_script.h
 *
 */

#ifndef _C_SCRIPT_H_
#define _C_SCRIPT_H_

#include "e_define.h"
#include "c_define.h"


struct script_list_s {
    char sc_names [ MAX_SCRIPT_NUM_PER_NODE ][ MAX_SCRIPT_NAME_LEN + SCRIPT_FILE_PATH_LEN ];
    int sc_num;
};
typedef struct script_list_s scl_t;



int init_script_list ( char * scl_path, scl_t * scl, char * init_output_msg  );
int update_script_list ( char * scl_path, scl_t * scl );





#endif
/*end of file*/
