/*
 *
 * config.h
 *
 */

#ifndef _CONFIG_H
#define _CONFIG_H

#include "email_interface.h"

/* max length of config file line */
#define MAX_CONFIG_FILE_LINE_LEN 10240
/* max length of config item */
#define MAX_CONFIG_ITEM_LEN 128

/* path of ocnfig file */
#define CONFIG_FILE_PATH "../config/ex_alarm.conf"
/* path of log */
#define CS_LOG_PATH "/var/log/ex_alarm.log"
/* path of pid file */
#define EX_ALARM_PID_FILE "/var/log/ex_alarm.pid"
/* max length of db name */
#define MAX_DB_NAME_LEN 512
/* max length of db user name */
#define MAX_DB_USER_NAME_LEN 512
/* max length of db password */
#define MAX_DB_PASSWORD_LEN 512

/* max length of sms num */
#define MAX_SMS_STR_LEN 16 
/* max count of sms num */
#define MAX_SMS_STR_NUM 32 

typedef enum alarm_method_switch_e {
    /* alarm method on */
    METHOD_OFF,
    /* alarm method off */
    METHOD_ON,
} alarm_method_switch_t;

struct alarm_method_s {
    /* alarm method email */
    alarm_method_switch_t email;
    /* alarm method sms */
    alarm_method_switch_t sms;
    /* alarm method tel */
    alarm_method_switch_t tel;
};

typedef struct alarm_method_s alarm_method_t;

struct alarm_freq_s {
    /* alarm freq interval */
    int interval;
    /* alarm freq times */
    int times;
};

typedef struct alarm_freq_s alarm_freq_t;

struct db_config_s {
    /* db server ip */
    char db_server_ip [ MAX_IP_STR_LEN ];
    /* db server port */
    unsigned short int db_server_port;
    /* db name */
    char db_name [ MAX_DB_NAME_LEN ];
    /* db user name */
    char db_username [ MAX_DB_USER_NAME_LEN ];
    /* db user password */
    char db_password [ MAX_DB_PASSWORD_LEN ];
};

typedef struct db_config_s db_config_t;


struct sms_config_s {
    /* sms numbers */
    char sms_num [ MAX_SMS_STR_NUM ] [ MAX_SMS_STR_LEN + 1 ];
};
struct tel_config_s {
    /* sms numbers */
    char tel_num [ MAX_SMS_STR_NUM ] [ MAX_SMS_STR_LEN + 1 ];
};


typedef struct sms_config_s sms_config_t;
typedef struct tel_config_s tel_config_t;

struct ex_alarm_config_s {
    /* alarm method ocnfig */
    alarm_method_t alarm_method;
    /* alarm freq config */
    alarm_freq_t alarm_freq;
    /* db config */
    db_config_t db_config;
    /* email config */
    email_config_t email_config;
    /* sms config */
    sms_config_t sms_config;
    /* tel config */
    tel_config_t tel_config;
};

typedef struct ex_alarm_config_s ex_alarm_config_t;



int init_config ( char * config_file_path, ex_alarm_config_t * ex_alarm_config, \
        alarm_items_t * alarm_items, char * error_msg );

int parse_alarm_config_line ( char * config_line, ex_alarm_config_t * ex_alarm_config, \
        alarm_items_t * alarm_items, char * error_msg );

int init_alarm_items ( alarm_items_t * alarm_items );

int destroy_alarm_items ( alarm_items_t * alarm_items );



#endif /* _CONFIG_H */

/*end of file*/

