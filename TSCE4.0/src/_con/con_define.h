
/*
 * Copyright (C) Inspur(Beijing)
 */


#ifndef TEYE_CON_DEFINE_H
#define TEYE_CON_DEFINE_H


#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>


#define TEYE_VERSION "4.0.0"


#define LEN_16		16
#define LEN_32		32
#define LEN_64		64
#define LEN_256		256
#define LEN_1024 	1024
#define LEN_10240	10240

/* define a package size 50kb */
#define PACKAGE_SIZE 51200


//#define MAX_MODULE_NUM 16
//#define MAX_INDEX_NUM 32
#define MAX_MODULE_NUM 32
#define MAX_INDEX_NUM 64
#define MAX_TOTAL_INDEX_NUM (MAX_MODULE_NUM * MAX_INDEX_NUM)


#define MAX_NODE_NUM 1000


#define MAX_IP_V4_STR_LEN 16
#define MAX_IP_STR_LEN MAX_IP_V4_STR_LEN
#define MAX_DATE_TIME_LEN 20
#define MAX_NODE_NAME_LEN 32

#define CLR_SEND_TIMEOUT 120 
#define CLR_RECV_TIMEOUT 120
#define SVR_SEND_TIMEOUT 120

#define RECV_RETRY_TIMES 10

typedef unsigned int ipv4_t;
typedef ipv4_t ip_t;

#define APP_INTERVAL_INCREMENTAL 120
#define TEYE_DATA_DELIMTER	","
#define TEYE_GET_DATA_NODE_INDEX_DELIMTER ","
#define TEYE_GET_DATA_ITEM_DELIMTER " "
#define DATABASE_ITEM_DELIMTER ","


#endif
