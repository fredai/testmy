
/*
 *
 * u_shm.h
 *
 */

#ifndef _U_SHM_H_
#define _U_SHM_H_



int u_shm_create ( int key, size_t shmsize, void * * result );

int u_shm_destroy ( int shmid, void * shmaddr );

int u_shm_attach ( int key, size_t shmsize, void * * result );

int u_shm_detach ( void * shmaddr );


#endif
