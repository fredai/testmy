/*
 *
 * u_queue.c
 *
 */

#include <string.h>
#include "u_queue.h"


void u_queue_init ( void * base, size_t nmemb, size_t item_size, queue_t * uq ) {
    uq -> q_size = nmemb;
    uq -> item_size = item_size;
    uq -> start = 0;
    uq -> end   = 0;
    uq -> buffer = base;
}
 
int u_queue_is_full ( queue_t * uq ) {
    return ( uq -> end + 1 ) % uq -> q_size == uq -> start;
}
 
int u_queue_is_empty ( queue_t * uq ) {
    return uq -> end == uq -> start;
}

void u_enqueue ( queue_t * uq, void * item ) {
    memcpy ( uq -> buffer + uq -> item_size * uq -> end, item, uq -> item_size );
    uq -> end = ( uq -> end + 1 ) % uq -> q_size;
}
 
void u_dequeue ( queue_t * uq, void * item ) {
    memcpy ( item, uq -> buffer + uq -> item_size * uq -> start, uq -> item_size );
    uq -> start = ( uq -> start + 1 ) % uq -> q_size;
}



/*end of file*/
