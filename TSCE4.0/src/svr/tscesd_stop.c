
/*
 * Copyright (C) Inspur(Beijing)
 */


#include "svr_define.h"
#include "util.h"
#include "error_handle.h"

#include <signal.h>
#include <unistd.h>
#include <stdio.h>


int 
main(void) 
{

    int ret;
    ret = check_pid (SVR_PID_FILE_PATH);
    if (ret > 0) {
        kill(ret, SIGUSR1);
    }
    sleep(1);
    system("killall tscesd >/dev/null 2>&1");
    sleep(1);
    system("killall tscesd >/dev/null 2>&1");
    sleep (1);

    ret = check_pid(SVR_PID_FILE_PATH);
    if (ret > 0) {
        err_exit("stop tscesd failed");
    } else {
        fprintf(stdout, "stop tscesd success\n");
    }

    exit (0);
}
