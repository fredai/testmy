/*
 *
 * c_signal.c
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include "c_signal.h"
#include "c_script.h"

void reload_config ( int signum );
extern int g_sct_update_flag;

int clr_script_signal_init ( ) {

    int ret;
    struct sigaction sa_alrm;
    sa_alrm.sa_handler = SIG_IGN;
    sigemptyset ( & sa_alrm.sa_mask );
    sa_alrm.sa_flags = 0; 
    ret = sigaction ( SIGALRM, & sa_alrm, NULL );
    if ( ret == -1 ) {
        return -1;
    }

    struct sigaction sa_int;
    sa_int.sa_handler = SIG_IGN;
    sigemptyset ( & sa_int.sa_mask );
    sa_int.sa_flags = 0;
    ret = sigaction ( SIGINT, & sa_int, NULL );
    if ( ret == -1 ) {
        return -2;
    }

    struct sigaction sa_hup;
    sa_hup.sa_handler = SIG_IGN;
    sigemptyset ( & sa_hup.sa_mask );
    sa_hup.sa_flags = 0;
    ret = sigaction ( SIGHUP, & sa_hup, NULL );
    if ( ret == -1 ) {
        return -3;
    }

    struct sigaction sa_quit;
    sa_quit.sa_handler = SIG_IGN;
    sigemptyset ( & sa_quit.sa_mask );
    sa_quit.sa_flags = 0;
    ret = sigaction ( SIGQUIT, & sa_quit, NULL );
    if ( ret == -4 ) {
        return -1;
    }

    struct sigaction sa_usr1;
    sa_usr1.sa_handler = reload_config;
    sigemptyset ( & sa_usr1.sa_mask );
    sa_usr1.sa_flags |= SA_RESTART;
    ret = sigaction ( SIGUSR1, & sa_usr1, NULL );
    if ( ret == -1 ) {
        return -5;
    }

    struct sigaction sa_usr2;
    sa_usr2.sa_handler = SIG_IGN;
    sigemptyset ( & sa_usr2.sa_mask );
    sa_usr2.sa_flags = 0;
    ret = sigaction ( SIGUSR2, & sa_usr2, NULL );
    if ( ret == -1 ) {
        return -6;
    }

    struct sigaction sa_cont;
    sa_cont.sa_handler = SIG_IGN;
    sigemptyset ( & sa_cont.sa_mask );
    sa_cont.sa_flags = 0;
    ret = sigaction ( SIGCONT, & sa_cont, NULL );
    if ( ret == -1 ) {
        return -7;
    }

    struct sigaction sa_tstp;
    sa_tstp.sa_handler = SIG_IGN;
    sigemptyset ( & sa_tstp.sa_mask );
    sa_tstp.sa_flags = 0;
    ret = sigaction ( SIGTSTP, & sa_tstp, NULL );
    if ( ret == -1 ) {
        return -8;
    }

    return 0;

}


int clr_cmd_signal_init ( ) {

    int ret;
    struct sigaction sa_alrm;
    sa_alrm.sa_handler = SIG_IGN;
    sigemptyset ( & sa_alrm.sa_mask );
    sa_alrm.sa_flags = 0; 
    ret = sigaction ( SIGALRM, & sa_alrm, NULL );
    if ( ret == -1 ) {
        return -1;
    }

    struct sigaction sa_int;
    sa_int.sa_handler = SIG_IGN;
    sigemptyset ( & sa_int.sa_mask );
    sa_int.sa_flags = 0;
    ret = sigaction ( SIGINT, & sa_int, NULL );
    if ( ret == -1 ) {
        return -2;
    }

    struct sigaction sa_hup;
    sa_hup.sa_handler = SIG_IGN;
    sigemptyset ( & sa_hup.sa_mask );
    sa_hup.sa_flags = 0;
    ret = sigaction ( SIGHUP, & sa_hup, NULL );
    if ( ret == -1 ) {
        return -3;
    }

    struct sigaction sa_quit;
    sa_quit.sa_handler = SIG_IGN;
    sigemptyset ( & sa_quit.sa_mask );
    sa_quit.sa_flags = 0;
    ret = sigaction ( SIGQUIT, & sa_quit, NULL );
    if ( ret == -4 ) {
        return -1;
    }

    struct sigaction sa_usr1;
    sa_usr1.sa_handler = SIG_IGN;
    sigemptyset ( & sa_usr1.sa_mask );
    sa_usr1.sa_flags = 0;
    ret = sigaction ( SIGUSR1, & sa_usr1, NULL );
    if ( ret == -1 ) {
        return -5;
    }

    struct sigaction sa_usr2;
    sa_usr2.sa_handler = SIG_IGN;
    sigemptyset ( & sa_usr2.sa_mask );
    sa_usr2.sa_flags = 0;
    ret = sigaction ( SIGUSR2, & sa_usr2, NULL );
    if ( ret == -1 ) {
        return -6;
    }

    struct sigaction sa_cont;
    sa_cont.sa_handler = SIG_IGN;
    sigemptyset ( & sa_cont.sa_mask );
    sa_cont.sa_flags = 0;
    ret = sigaction ( SIGCONT, & sa_cont, NULL );
    if ( ret == -1 ) {
        return -7;
    }

    struct sigaction sa_tstp;
    sa_tstp.sa_handler = SIG_IGN;
    sigemptyset ( & sa_tstp.sa_mask );
    sa_tstp.sa_flags = 0;
    ret = sigaction ( SIGTSTP, & sa_tstp, NULL );
    if ( ret == -1 ) {
        return -8;
    }

    return 0;

}


void reload_config ( int signum ) {
    g_sct_update_flag = 1;
}


/*end of file*/
