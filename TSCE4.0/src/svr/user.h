
/*
 * Copyright (C) Inspur(Beijing)
 */


#ifndef TEYE_USER_H
#define TEYE_USER_H


#include "con_define.h"


void* thread_user(void * arg_user);


struct thread_arg_user_s {
    char client_ip[MAX_IP_STR_LEN];
    int socket_fd;
};
typedef struct thread_arg_user_s thread_arg_user_t;


#endif
