
/*
 * Copyright (C) Inspur(Beijing)
 */


#include "error_handle.h"
#include "svr_define.h"
#include "svr_config.h"
#include "util.h"


int 
main(int argc, char* argv[]) 
{

    int ret;
    char cmd[1024];

    sprintf(cmd, "%s", "./tscesd");
    ret = system(cmd);
    sleep(1);
    usleep(1000*500);
    if (ret == 0) {
        pid_t pid = check_pid(SVR_PID_FILE_PATH);
        if (pid > 0) {
            fprintf(stdout, "start tscesd success\n");
        } else {
            err_exit("start tscesd failed: check_pid error");
        }
    } else {
        err_exit("start tscesd failed");
    }

    exit(0);
}
