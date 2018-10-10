
/*
 * Copyright (C) Inspur(Beijing)
 */


#include "error_handle.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#define MAX_SIZE 256
#define MAX_RESULT_SIZE 2048
#define TEMP_STOP_RESULT_FILE "./stop_result"

#define TEYE_SERVER_STOP_SUCCESS "summary server:success "
#define TEYE_SERVER_STOP_FAILED  "summary server:failed "


int
main(int argc, char *argv[])
{
    if (argc != 3) {
        err_exit("usage: %s server_directory client_directory", argv[0]);
    }

    int ret;
    char cmd[MAX_SIZE] = {0};
    char result[MAX_RESULT_SIZE] = {0};
    FILE *f;

    if ( (f = fopen(TEMP_STOP_RESULT_FILE, "a+")) < 0) {
        err_sys("create temp file %s failed", TEMP_STOP_RESULT_FILE);
        strcat(result, TEYE_SERVER_STOP_FAILED);
        goto FINISHED;
    }

    snprintf(cmd, MAX_SIZE, "cd %s; ./tscesd_stop", argv[1]);
    ret = system(cmd);
    if (ret < 0) {
        err_sys("sysem error");
        strcat(result, TEYE_SERVER_STOP_FAILED);
        goto FINISHED;
    } else if (ret > 0) {
        strcat(result, TEYE_SERVER_STOP_FAILED);
        goto FINISHED;
    } else {
        strcat(result, TEYE_SERVER_STOP_SUCCESS); 
    }

    snprintf(cmd, MAX_SIZE, "./tscecd_all_stop -d %s", argv[2]);
    ret = system(cmd);
    if (ret < 0) {
        err_sys("system error");
        strcat(result, "stop all clients failed\n");
    } else if (ret > 0) {
        strcat(result, "stop all clients failed\n");
    } else {
        char line[MAX_RESULT_SIZE];
        while (fgets(line, MAX_RESULT_SIZE, f) != NULL) {
            strcat(result, line);
        }
    }

FINISHED:
    unlink(TEMP_STOP_RESULT_FILE);
    if (result[strlen(result) - 1] == ' ') {
        result[strlen(result) - 1] = '\0';
    }    
    printf("\n%s\n", result);
    exit(0);

}
