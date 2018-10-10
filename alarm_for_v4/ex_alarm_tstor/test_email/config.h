/*
 *
 * config.h
 *
 */

#ifndef _CONFIG_H
#define _CONFIG_H

#include "email_interface.h"

/* max length of config file line */
#define MAX_CONFIG_FILE_LINE_LEN 102400
/* max length of config item */
#define MAX_CONFIG_ITEM_LEN 128

/* path of ocnfig file */
#define CONFIG_FILE_PATH "../config/ex_alarm.conf"
/* path of log */
#define CS_LOG_PATH "../log/ex_alarm.log"
/* path of pid file */

/* max length of sms num */
#define MAX_SMS_STR_LEN 16 
/* max count of sms num */
#define MAX_SMS_STR_NUM 32 

#define ALARM_TEST_INFO "This is a test message from the cluster alarm monitoring software\n"
#define ALARM_TEST_SUBJECT "Test message from the cluster monitoring alarm software"




int init_config ( char * config_file_path, email_config_t * email_config, char * error_msg );

int parse_alarm_config_line ( char * config_line, email_config_t * email_config, char * error_msg );


#endif /* _CONFIG_H */

/*end of file*/

