
/*
 * Copyright (C) Inspur(Beijing)
 */


#include "common_util.h"
#include "error_handle.h"

#include <sys/stat.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>


#define MAX_PATH_LENGTH 1024


int
util_mkdir(const char *path)
{

	assert(path != NULL && path[0] != '\0' && strlen(path) < MAX_PATH_LENGTH);

	char path_name[MAX_PATH_LENGTH];
	memset(path_name, 0, sizeof(path_name));

	if (path[0] != '/') {
		if (getcwd(path_name, MAX_PATH_LENGTH) == NULL) {
			err_sys("create path \"%s\" error", path_name);
		    return -1;
		}
		strcat(path_name, "/");	
		strcat(path_name, path);
	} else {
		strcpy(path_name, path);
	}

	char path_temp[MAX_PATH_LENGTH];
	memset (path_temp, 0, sizeof(path_temp));
	int cnt = 0;
	mode_t mode;

	while (path_name[cnt++] != '\0') {
		if (path_name[cnt] == '/' || cnt == strlen(path_temp) -1 ) {
			strncpy(path_temp, path_name, cnt+1);
			mode = umask(0);

			if (mkdir(path_temp, 0755) < 0) {
				if (errno == EEXIST) {
				   	continue;
				} else {
					err_sys("create directory \"%s\" error", path_temp);
					return -1;
				}
			}
			umask(mode);
		}
	}
	return 0;
}

