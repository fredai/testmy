/*
 *
 * u_mutex.h
 *
 */

#ifndef _U_MUTEX_H_
#define _U_MUTEX_H_


int u_mutex_create ( int key );

/*
 *
 * -1 EACCES
 * -2 EEXIST
 *
 */


int u_mutex_attach ( int key );

/*
 * -1 EACCES
 * -2 ENOENT
 */

int u_mutex_init ( int mutex_id );

/*
 * -1 EACCES
 * -2 EIDRM
 */

int u_mutex_rmid ( int mutex_id );

/*
 * -1 EACCES
 * -2 EIDRM
 */

int u_mutex_lock ( int mutex_id );

/*
 * -1 EACCES
 * -2 EIDRM
 * -3 EINTR
 */

int u_mutex_unlock ( int mutex_id );



/*
 * -1 EACCES
 * -2 EIDRM
 * -3 EINTR
 */



#endif
