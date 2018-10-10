
/*
 * Copyright (C) Inspur(Beijing)
 */


#ifndef TEYE_RTD_H
#define TEYE_RTD_H


#include "con_define.h"


void* thread_rtd(void * arg_rtd);


struct thread_arg_rtd_s {
    char client_ip[MAX_IP_STR_LEN];
    int socket_fd;
};
typedef struct thread_arg_rtd_s thread_arg_rtd_t;


#endif
