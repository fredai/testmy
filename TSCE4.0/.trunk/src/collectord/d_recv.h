/*
 *
 * d_recv.h
 *
 */

#ifndef _D_RECV_H_
#define _D_RECV_H_

#include "e_define.h"
#include "e_protocol.h"

struct thread_arg_recv_s {
    unsigned int  uint_ip;
    int socket_fd;
};
typedef struct thread_arg_recv_s thread_arg_recv_t;

struct recv_buffer_s {
    data_head_t head;
    char data [ MAX_RECV_DATA_LEN_PER_SCRIPT ];
};
typedef struct recv_buffer_s recv_buffer_t;

void * thread_recv ( void * args );


#endif
/*end of file*/
