
/*
 * Copyright (C) Inspur(Beijing)
 */


#include "svr_define.h"
#include "util.h"
#include <stdio.h>


int 
main(void) 
{ 
    int ret = check_pid(SVR_PID_FILE_PATH);
    if (ret > 0) {
        fprintf(stdout, "tscesd (pid %d) is running...\n", ret);
    } else {
        fprintf(stdout, "tscesd is stopped\n");
    }
    exit(0);
}
