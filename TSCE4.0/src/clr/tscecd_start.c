
/*
 * Copyright (C) Inspur(Beijing)
 */


#include "clr_define.h"
#include "error_handle.h"
#include "util.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>


int 
main(void)
{
    int ret;
    //const char *cmd = "ulimit -c unlimited; ./tscecd >/dev/null 2>&1";
    const char *cmd = "ulimit -c unlimited; ./tscecd";

    ret = system(cmd);
    usleep(1000*500);
    if (ret < 0) {
        err_sys("system error");
        exit(1);
    } else if (ret > 0) {
        err_exit("start tscecd failed");
    }

    pid_t pid = check_pid(CLR_PID_FILE_PATH);
    if (pid > 0) {
        fprintf(stdout, "start tscecd success\n");
    } else {
        err_exit("start tscecd failed");
    }

    exit (0);
}
