/*
 *
 * e_define.h
 *
 */

#ifndef _E_DEFINE_H_
#define _E_DEFINE_H_

#include <string.h>

#define NODE_LIST_FILE_PATH "../../common-config/nodelist.conf"
#define SCRIPT_FILES_PATH "../../common-config/scripts/"

#define SCRIPT_FILE_PATH_LEN strlen(SCRIPT_FILES_PATH)
#define MAX_FILE_PATH_LEN 1024

#define MIN_CL_INTERVAL 1
#define MAX_CL_INTERVAL (60*10)

#define MAX_DATE_TIME_LEN 20
#define MAX_IP_V4_STR_LEN 16
#define MAX_IP_STR_LEN MAX_IP_V4_STR_LEN

#define MAX_CONFIG_FILE_LINE_LEN 1024
#define MAX_CONFIG_ITEM_LEN 64
#define MAX_CONFIG_VALUE_LEN 64

#define MAX_NODE_NAME_LEN 32
#define MAX_DATA_ITEM_NAME_LEN 32
#define MAX_DATA_VALUE_LEN 32
#define MAX_SCRIPT_NAME_LEN 32
#define MAX_SCRIPT_NUM_PER_NODE 32
#define MAX_DATA_ITEM_NUM_PER_SCRIPT 32
#define MAX_NODE_FILE_LINE_LEN (MAX_NODE_NAME_LEN+MAX_SCRIPT_NAME_LEN*MAX_SCRIPT_NUM_PER_NODE+512)

#define SCRIPT_DATA_DELIMITER_LEN 2
#define DATA_PROTOCOL_HEAD_LEN 8
#define MAX_RESULT_DATA_LEN_PER_SCRIPT \
(MAX_DATA_ITEM_NUM_PER_SCRIPT*(MAX_DATA_ITEM_NAME_LEN+MAX_DATA_VALUE_LEN+SCRIPT_DATA_DELIMITER_LEN))
#define MAX_SEND_DATA_LEN_PER_SCRIPT (MAX_RESULT_DATA_LEN_PER_SCRIPT+DATA_PROTOCOL_HEAD_LEN+MAX_SCRIPT_NAME_LEN+MAX_DATE_TIME_LEN+512)
#define MAX_RECV_DATA_LEN_PER_SCRIPT (MAX_SEND_DATA_LEN_PER_SCRIPT*4)

#define MAX_CMD_REQ_LEN 1024
#define MAX_CMD_RES_LEN (1024*4*10*10)



#endif
/*end of file*/
