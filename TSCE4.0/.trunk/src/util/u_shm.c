
/*
 *
 * u_shm.c
 *
 */

#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <errno.h>


int u_shm_create ( int key, size_t shmsize, void * * result ) {

    const int shmflag = IPC_CREAT|IPC_EXCL|0660;

    int shmid = -1;

    shmid = shmget ( ( key_t ) key, shmsize, shmflag );

    if ( shmid == -1 ) {

        if ( errno == EACCES ) {
            return -1;
        }

        else if ( errno == EEXIST ) {
            return -2;
        }

        else if ( errno == EINVAL ) {
            return -3;
        }

        else {
            return -4;
        }
    }

    void * shmaddr = NULL;

    shmaddr = shmat ( shmid, NULL, 0 );

    if ( shmaddr == ( void * ) -1 ) {

        if ( errno == EACCES ) {
            return -5;
        }

        else if ( errno == EIDRM ) {
            return -6;
        }

        else {
            return -7;
        }

    }

    * result = shmaddr;

    return shmid;
}


int u_shm_destroy ( int shmid, void * shmaddr ) {

    int ret = -1;

    if ( shmaddr != NULL ) {

        ret = shmdt ( shmaddr );
        if ( ret == -1 ) {
            if ( errno == EINVAL ) {
                return -1;
            }
            else {
                return -2;
            }
        }
    }

    ret = shmctl ( shmid, IPC_RMID, 0 );

    if ( ret == -1 ) {
        if ( errno == EACCES ) {
            return -3;
        }
        else if ( errno == EFAULT ) {
            return -4;
        }
        else if ( errno == EIDRM ) {
            return -5;
        }
        else if ( errno == EINVAL ) {
            return -6;
        }
        else {
            return -7;
        }
    }

    return 0;

}


int u_shm_attach ( int key, size_t shmsize, void * * result ) {

    const int shmflag = 0660;

    int shmid = -1;

    shmid = shmget ( ( key_t ) key, shmsize, shmflag );

    if ( shmid == -1 ) {

        if ( errno == EACCES ) {
            return -1;
        }

        else if ( errno == ENOENT ) { 
            return -2; 
        } 
        else if ( errno == EINVAL ) {
            return -3;
        }
        else {
            return -4;
        }
    }

    void * shmaddr = NULL;

    shmaddr = shmat ( shmid, NULL, 0 );

    if ( shmaddr == ( void * ) -1 ) {

        if ( errno == EACCES ) {
            return -5;
        }

        else if ( errno == EIDRM ) {
            return -6;
        }

        else {
            return -7;
        }

    }

    * result = shmaddr;

    return shmid;
}



int u_shm_detach ( void * shmaddr ) {

    int ret = -1;

    ret = shmdt ( shmaddr );

    if ( ret == -1 ) {
        if ( errno == EINVAL ) {
            return -1;
        }
        else {
            return -2;
        }
    }

    return 0;

}



/*end of file*/
