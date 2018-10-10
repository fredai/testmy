
/*
 * Copyright (C) Inspur(Beijing)
 */


#ifndef TEYE_PROTOCOL_H
#define TEYE_PROTOCOL_H

#define ETH_NAME_LEN 32
#define DISK_NAME_LEN 32

#include "con_define.h"


#pragma pack(1)
struct data_head_s {
    unsigned char request_type;
    unsigned char data_type;
    unsigned char reserved1;
    unsigned char reserved2;
    unsigned int len;
};
#pragma pack()
typedef struct data_head_s app_hdr_t;

#pragma pack(1)
struct app_data_s {
	char buffer[PACKAGE_SIZE];
};
#pragma pack()
typedef struct app_data_s app_data_t;

#pragma pack(1)
struct app_pkg_s {
    app_hdr_t head;
    app_data_t body;
};
#pragma pack()
typedef struct app_pkg_s app_pkg_t;


#define REQUEST_TYPE_POST 0X01
#define REQUEST_TYPE_GET 0X02
#define REQUEST_TYPE_USER 0X04

#define REQUEST_POST_NORMAL_DATA 0X01


#define RESPONSE_POST_OK 0X01
#define RESPONSE_POST_ERROR 0X02
#define RESPONSE_POST_EXIT 0X03

#define REQUEST_GET_STATUS 0X01
#define REQUEST_GET_EXIT 0X02
#define REQUEST_GET_DATA 0X03

#define RESPONSE_GET_STATUS_OK 0X01
#define RESPONSE_GET_EXIT 0X02
#define RESPONSE_GET_DATA 0X03

#define HANDSHAKE_HELO_STR "HE"
#define HANDSHAKE_OK_STR "OK"

/* include length of header */
//#define MAX_GET_DATA_LEN 8+100*64
#define MAX_GET_DATA_LEN 8+50*1024

#define IS_HELO(head) strncmp(((char*)&head)+2,HANDSHAKE_HELO_STR,2)==0
#define IS_OK(head) strncmp(((char*)&head)+2,HANDSHAKE_OK_STR,2)==0

#define SET_HELO(head) strncpy(((char*)&head)+2,HANDSHAKE_HELO_STR,2)
#define SET_OK(head) strncpy(((char*)&head)+2,HANDSHAKE_OK_STR,2)

//int convert_app_data_to_str ( app_data_t * app_data, char * str );
int convert_app_data_to_str(app_data_t *app_data, char *str, char flag);

#endif

/*end of file*/
