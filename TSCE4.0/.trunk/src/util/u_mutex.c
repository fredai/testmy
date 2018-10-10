/*
 *
 * u_mutex.c
 *
 */

#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#ifdef _SEM_SEMUN_UNDEFINED
    union semun {
        int val;
        struct semid_ds * buf;
        unsigned short * array;
        struct seminfo * __buf;
    };
#endif

int u_mutex_create ( int key ) {

    int semid = -1;

    const int semflag = IPC_CREAT|IPC_EXCL|0660;

    semid = semget ( ( key_t ) key, 1, semflag );

    if ( semid == -1 ) {

        if ( errno == EACCES ) {
	    return -1;
	}

        else if ( errno == EEXIST ) {
	    return -2;
	}

        else {
            return -3;
        }

    }

    return semid;

}


int u_mutex_attach ( int key ) {

    int semid = -1;

    const int semflag = 0660;
		
    semid = semget ( ( key_t ) key, 0, semflag );
	
    if ( semid == -1 ) {

        if ( errno == EACCES ) {
            return -1;
        }

        else if ( errno == ENOENT ) {
	    return -2;
        }

        else {
            return -3;
        }

    }

    return semid;

}


int u_mutex_init ( int mutex_id ) {

    int ret = -1;

    union semun sem_sv;

    sem_sv.val = 1;

    ret = semctl ( mutex_id, 0, SETVAL, sem_sv );

    if ( ret == -1 ) {

        if ( errno == EACCES ) {
            return -1;
        }
        else if ( errno == EIDRM ) {
            return -2;
        }
        else {
            return -3;
        }

    }

    return 0;

}


int u_mutex_rmid ( int mutex_id ) {

    int ret = -1;
	
    union semun sem_sv;

    ret = semctl ( mutex_id, 0, IPC_RMID, sem_sv );

    if ( ret == -1 ) {

        if ( errno == EACCES ) {
            return -1;
        }
	else if ( errno == EIDRM ) {
            return -2;
	}
        else {
            return -3;
	}

    }

    return 0;

}


int u_mutex_lock ( int mutex_id ) {

    int ret = -1;

    struct sembuf sop;

    sop.sem_num = 0;
    sop.sem_op = -1;
    sop.sem_flg = SEM_UNDO;

    ret = semop ( mutex_id, & sop, 1 );

    if ( ret == -1 ) {

        if ( errno == EACCES ) {
            return -1;
        }
        else if ( errno == EIDRM ) {
            return -2;
        }
        else if ( errno == EINTR ) {
            return -3;
        }
        else {
            return -4;
        }

    }

    return 0;

}


int u_mutex_unlock ( int mutex_id ) {

    int ret = -1;

    struct sembuf sop;

    sop.sem_num = 0;
    sop.sem_op = 1;
    sop.sem_flg = SEM_UNDO;

    ret = semop ( mutex_id, & sop, 1 );

    if ( ret == -1 ) {

        if ( errno == EACCES ) {
            return -1;
        }
        else if ( errno == EIDRM ) {
            return -2;
        }
        else if ( errno == EINTR ) {
            return -3;
        }
        else {
            return -4;
        }

    }

    return 0;

}






/*end of file*/
