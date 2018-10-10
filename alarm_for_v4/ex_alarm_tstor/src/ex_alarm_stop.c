
/*
 * Copyright (C) Inspur(Beijing)
 */


#include "utils.h"
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>


int 
main(void) 
{

#define EX_ALARM_PID_FILE "/var/log/ex_alarm.pid"
    int ret;
    ret = check_pid (EX_ALARM_PID_FILE);
    if (ret > 0) {
        kill(ret, SIGUSR1);
    }
    sleep(1);
    system("killall ex_alarm >/dev/null 2>&1");
    sleep(1);
    system("killall ex_alarm >/dev/null 2>&1");
    sleep (1);

    ret = check_pid(EX_ALARM_PID_FILE);
    if (ret > 0) {
        perror("stop ex_alarm failed");
    } else {
        fprintf(stdout, "stop ex_alarm success\n");
    }

    exit (0);
}
