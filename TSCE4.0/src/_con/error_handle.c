
/*
 * Copyright (C) Inspur(Beijing)
 */


#include "error_handle.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static void err_doit(int errnoflag, int error, const char *fmt, va_list ap);


void 
err_msg(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    err_doit(0, 0, fmt, ap);
    va_end(ap);
}


void
err_sys(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    err_doit(1, errno, fmt, ap);
    va_end(ap);
}


void
err_exit(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    err_doit(0, 0, fmt, ap);
    va_end(ap);
    exit(1);
}


void 
err_quit(const char *fmt, ...)
{
	va_list ap;

    va_start(ap, fmt);
    err_doit(1, errno, fmt, ap); 
    va_end(ap);
    exit(1);
}

static void
err_doit(int errnoflag, int error, const char *fmt, va_list ap)
{
	char buf [MAX_ERROR_MESSAGE_LENGTH];
	vsnprintf(buf, MAX_ERROR_MESSAGE_LENGTH, fmt, ap);

	if (errnoflag) {
		snprintf(buf + strlen(buf), MAX_ERROR_MESSAGE_LENGTH - strlen(buf), \
				" : %s", strerror(error));
	}
	strcat(buf, "\n");
	fflush(stdout);
	fputs(buf, stderr);
	fflush(NULL);  /* flushes all stdio output streams */
}
