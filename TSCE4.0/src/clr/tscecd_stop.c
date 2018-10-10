
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
main(int argc, char *argv[])
{

    system ("killall tscecd >/dev/null 2>&1");
    sleep ( 1 ); 
    system ("killall tscecd >/dev/null 2>&1");
    sleep ( 1 ); 

    int ret = check_pid(CLR_PID_FILE_PATH);
    if (ret > 0) {
        err_exit("stop tscecd failed");
    } else {
        fprintf(stdout, "stop tscecd success\n");
    }

    exit(0);
}
