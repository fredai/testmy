/*
 *
 * c_define.h
 *
 */

#ifndef _C_DEFINE_H_
#define _C_DEFINE_H_

#define MAX_CLR_CONFIG_FILE_LINE_LEN 1024
#define CLR_ERRMSG_LEN 256 

#define CLR_CONFIG_FILE_PATH "../conf/collector.conf"
#define CLR_LOG_SCT_PATH "/var/log/ex-collector/collector_sct.log"
#define CLR_LOG_CMD_PATH "/var/log/ex-collector/collector_cmd.log"
#define DATA_BUFFER_FILE "/var/lib/ex-collector/data.tmp"
#define LOCAL_SCRIPT_PATH "/var/lib/ex-collector/.scripts"

#define CLR_SEND_TIMEOUT 5
#define CLR_RECV_TIMEOUT 5

#define LOCAL_SCIPRT_PATH_LEN (MAX_SCRIPT_NAME_LEN+strlen(LOCAL_SCRIPT_PATH)+32)
#define SCT_EIXT_CODE 100

#define RECONFIG_CMD_STR "reconfig"
#define RECONFIG_RESULT_STR "reconfig success"
#define CLR_NETWORK_RECOVERY_RETRY_INTERVAL 3

#define MIN_CMD_TIMEOUT 1
#define MAX_CMD_TIMEOUT (60*10)

#define EXE_FMT_RE_PATTERN \
    "^([\\w\\.]{1,32}=[\\d\\.]{1,32}\\^){0,31}([\\w\\.]{1,32}=[\\d\\.]{1,32})[\\s\\r\\n]{0,5}$"
#define FILE_DATA_FMT_RE_PATTERN \
    "^(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2})([\\w\\.]{1,32}=[\\d\\.]{1,32}\\^){0,31}([\\w\\.]{1,32}=[\\d\\.]{1,32})[\\s\\r\\n]{0,5}$"


#endif
/*end of file*/
