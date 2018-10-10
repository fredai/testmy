/*
 *
 * ex_alarm.h
 *
 */


#ifndef _EX_ALARM_H
#define _EX_ALARM_H


#include <pthread.h>


/* max length of alarm node name */
#define MAX_ALARM_NODE_NAME_LEN 32
/* max length of alarm item name */
#define MAX_ALARM_ITEM_NAME_LEN 32
/* max length of ip v4 str */
#define MAX_IP_V4_STR_LEN 16
/* max length of ip str */
#define MAX_IP_STR_LEN MAX_IP_V4_STR_LEN
/* max length of data time */
#define MAX_DATE_TIME_LEN 20

/* num of alarm level */
#define ALARM_LEVEL_NUM 3
/* length of error message */
#define ALARM_ERRMSG_LEN 1024 
/* length of alarm message buffer */
#define MAX_ALARM_MSG_BUFFER_LEN (1024*4*2)
#define MAX_ALARM_MSG_BUFFER_LEN_MYSQL (1024)
/* alarm get data interval */
#define ALARM_GET_DATA_INTERVAL 1
/* max length of alarm script name */
#define MAX_ALARM_SCRIPT_NAME_LEN 512
/*alarm suject length*/
#define MAX_ALAEM_SUBJECT_LENGTH 128

typedef enum alarm_leval_e {
    /* level serious */
    ALARM_LEVEL_SERIOUS,
    /* level common */
    ALARM_LEVEL_COMMON,
    /* level warn */
    ALARM_LEVEL_WARN
} alarm_level_t;

typedef enum alarm_relation_e {
    /* > */
    GT,
    /* < */
    LT,
    /* = */
    EQ,
    /* >= */
    GE,
    /* <= */
    LE,
    /* != */
    NQ,
    /* unknown */
    UNKNOW
} alarm_relation_t;

typedef enum alarm_flag_e {
    /* have not alarmed */
    HAVE_NOT_ALARM,
    /* alarming */
    ALARMING,
    /* alarmed */
    ALARMED
} alarm_flag_t;


typedef pthread_mutex_t alarm_thread_mutex_t;

typedef struct alarm_condition_s alarm_condition_t;

typedef struct alarm_node_s alarm_node_t;

typedef struct alarm_item_s alarm_item_t;

typedef struct alarm_items_s alarm_items_t;

struct alarm_condition_s {
    /* time of condition */
    int secs;
    /* value of condition */
    float value;
    /* ralation of condition */
    alarm_relation_t alarm_relation;
};


struct alarm_node_s {
    /* alarm node name */
    char alarm_node_name [ MAX_ALARM_NODE_NAME_LEN ];
    /* timer */
    time_t timer_start [ ALARM_LEVEL_NUM ];
    /* alarm flag */
    alarm_flag_t alarm_flag [ ALARM_LEVEL_NUM ];
    /* thread control */
    int thread_cancel [ ALARM_LEVEL_NUM ];
    /* mutex */
    alarm_thread_mutex_t alarm_thread_mutex [ ALARM_LEVEL_NUM ];
    /* alarm_end */
    /* time_t end_time [ ALARM_LEVEL_NUM ]; */
    struct alarm_node_s * next;
};

struct alarm_item_node {
	char alarm_item_node_name[MAX_ALARM_ITEM_NAME_LEN];
	struct alarm_item_node *next;
};
typedef struct alarm_item_node alarm_item_node_t;

struct alarm_item_s {
    /* alarm item name */
    char alarm_item_name [ MAX_ALARM_ITEM_NAME_LEN ];
    /*alarm item--node*/
    alarm_item_node_t alarm_item_node_head;
    /* alarm node count*/
    int alarm_item_node_count;
    /* alarm condition */
    alarm_condition_t alarm_condition [ ALARM_LEVEL_NUM ];
    /* alarm script name */
    char alarm_script [ MAX_ALARM_SCRIPT_NAME_LEN ]; 
    /* next */
    struct alarm_item_s * next;
    /* alarm node head */
    alarm_node_t alarm_node_head;
};


struct alarm_items_s {
    /* alarm item head */
    alarm_item_t alarm_item_head;
    /* alarm item count */
    int alarm_item_count;
};


struct alarm_msg_buf_s {
    /* alarm message */
    char * msg;
    char * msg_ch;
    /* alarm iftreated */
    char end_flag;

    /* alarm content */  
    char content[512];  
    /* ip */
    char ip [ MAX_IP_V4_STR_LEN ];
    /* alarm nodeName */
    char nodeName [ MAX_ALARM_NODE_NAME_LEN ];
    /* alarm type */
    char alarmType [ MAX_ALARM_ITEM_NAME_LEN ];
    /* alarm agent */
    char alarmAgent [ MAX_ALARM_NODE_NAME_LEN];
    /* alarm source */
    int alarmSource;
    /* alarm time */
    char alarmTime [ MAX_DATE_TIME_LEN ];
    /* alarmLevel */
    char alarmLevel;
    
    struct alarm_msg_buf_s * next;
};

typedef struct alarm_msg_buf_s alarm_msg_buf_t;

struct alarm_th_stob_s {
    /* pointer to alarm item */
    alarm_item_t * alarm_item;
    /* pointer to alarm node */
    alarm_node_t * alarm_node;
    /* alarm level */
    alarm_level_t alarm_level;
    /* alarm value */
    float alarm_value;
};

typedef struct alarm_th_stob_s alarm_th_stob_t;





#endif /* _EX_ALARM_H */

/*end of file*/

