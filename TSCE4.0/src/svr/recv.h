
/*
 * Copyright (C) Inspur(Beijing)
 */


#ifndef TEYE_RECV_H
#define TEYE_RECV_H


#include "buffer.h"


void* thread_recv(void *arg_recv);


struct thread_arg_recv_s {
    app_node_data_t * node_entry;
    int socket_fd;
};
typedef struct thread_arg_recv_s thread_arg_recv_t;


#endif
