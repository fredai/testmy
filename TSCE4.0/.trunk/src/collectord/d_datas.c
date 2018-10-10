/*
 *
 * d_datas.c
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "d_define.h"
#include "d_datas.h"
#include "d_nodes.h"
#include "u_queue.h"


data_buffer_t g_data_buffer;
pthread_mutex_t mutex_g_data_buffer;


int init_data_buffer ( data_buffer_t * data_buffer ) {

    if ( data_buffer == NULL ) {
        return -1;
    }

    u_queue_init ( data_buffer -> __clrd_datas,
            DATA_BUFFER_ITEM_NUM,
            sizeof ( data_item_t ),
            & data_buffer -> clrd_que );

    return 0;

}


int put_data_item_in_buffer ( data_buffer_t * data_buffer, data_item_t * data_item ) {

    if ( data_buffer == NULL || data_item == NULL ) {
        return -1;
    }

    if ( ! u_queue_is_full ( & ( data_buffer -> clrd_que ) ) ) {
        u_enqueue ( & ( data_buffer -> clrd_que ), data_item );
    }
    else {
        return 1;
    }

    return 0;

}



int get_data_item_from_buffer ( data_buffer_t * data_buffer, data_item_t * data_item ) {

    if ( data_buffer == NULL || data_item == NULL ) {
        return -1;
    }

    if ( ! u_queue_is_empty ( & ( data_buffer -> clrd_que ) ) ) {
        u_dequeue ( & ( data_buffer -> clrd_que ), data_item );
    }
    else {
        return 1;
    }

    return 0;

}


int destroy_data_buffer ( data_buffer_t * data_buffer ) {

    if ( data_buffer == NULL ) {
        return -1;
    }

    bzero ( & ( data_buffer -> clrd_que ), sizeof ( u_queue_t ) );

    return 0;

}



int is_data_buffer_empty ( data_buffer_t * data_buffer ) {

    if ( data_buffer == NULL ) {
        return -1;
    }

    return u_queue_is_empty ( & ( data_buffer -> clrd_que ) );

}



int is_data_buffer_full ( data_buffer_t * data_buffer ) {

    if ( data_buffer == NULL ) {
        return -1;
    }

    return u_queue_is_full ( & ( data_buffer -> clrd_que ) );

}


/*end of file*/
