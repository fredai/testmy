/*
 *
 * u_queue.h
 *
 */

#ifndef _U_QUEUE_H_
#define _U_QUEUE_H_

#include <unistd.h>

typedef struct {
    int         q_size;
    int         item_size;
    int         start;
    int         end;
    void *      buffer;
} queue_t;

typedef queue_t u_queue_t;

void u_queue_init ( void * base, size_t nmemb, size_t item_size, queue_t * uq );

int u_queue_is_full ( queue_t * uq );

int u_queue_is_empty ( queue_t * uq );

void u_enqueue ( queue_t * uq, void * item );

void u_dequeue ( queue_t * uq, void * item );


#endif
