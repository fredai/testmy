/*
 *
 * c_engine.c
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "c_engine.h"


static pid_t forkresult = -1;

void killer ( int sig ) {

  kill ( forkresult, SIGTERM );
  int count = 0;
  while ( ++ count < 1024*1024 );
  kill ( forkresult, SIGKILL );

}


int c_execute ( char * cmd, char * result, size_t result_size, int * exit_status, int timeout ) {

    if ( cmd == NULL || result == NULL || exit_status == NULL || cmd [ 0 ] == '\0' ) {
        return -1;
    }

    int ret = -1;
    int pipefd [ 2 ];
    int data_processed;

    ret = pipe ( pipefd );
    if ( ret == -1 ) {
        return -2;
    }

    setpgid ( 0, 0 );

    forkresult = fork ( );
    if ( forkresult == -1 ) {
        return -3;
    }

    if ( forkresult == 0 ) {

	dup2 ( pipefd [ 1 ], 1 );
	dup2 ( pipefd [ 1 ], 2 );
	close ( pipefd [ 1 ] );
	close ( pipefd [ 0 ] );

        execl ( "/bin/sh", "sh", "-c", cmd, ( char * ) 0 );

        exit ( 4 );

    }

    else {

	close ( pipefd [ 1 ] );

	int result_len = 0;
	char buffer [ BUFSIZ + 1 ];
        struct sigaction act;

        act.sa_handler = killer;
        sigemptyset ( & act.sa_mask );
        act.sa_flags = 0;
        sigaction ( SIGALRM, & act, 0 );

        alarm ( timeout );

	while ( ( data_processed = read ( pipefd [ 0 ], & buffer, BUFSIZ ) ) > 0 ) {

            if ( data_processed < result_size - result_len ) {

	        memcpy ( result + result_len, buffer, data_processed );

            }

            else {

              kill ( getpid ( ), SIGALRM );

              return -5;/*too long*/

            }

	    result_len += data_processed;

	}

	close ( pipefd [ 0 ] );

        int stat_val;

	waitpid ( forkresult, & stat_val, 0 );

        if ( WIFEXITED ( stat_val ) ) {

            alarm ( 0 );
            * exit_status = WEXITSTATUS ( stat_val );
            return result_len;

        }

        else if ( WIFSIGNALED ( stat_val ) ) {

            * exit_status = WTERMSIG ( stat_val );
            return -6;/*time out*/

        }

        else if ( WIFSTOPPED ( stat_val ) ) {

            * exit_status = WSTOPSIG ( stat_val );
            return -7;

        }

   }

    return 0;
  
}


/*end of file*/
