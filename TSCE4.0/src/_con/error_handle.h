
/*
 * Copyright (C) Inspur(Beijing)
 */

#ifndef TEYE_ERROR_HANDLE_H
#define TEYE_ERROR_HANDLE_H

#include <stdarg.h>


#define MAX_ERROR_MESSAGE_LENGTH 1024


/* 
 * print a message without errno message 
 * then return 
 */
void err_msg(const char *fmt, ...);


/*
 * print a message with errno message
 * then return
 */
void err_sys(const char *fmt, ...);


/*
 * print a message without errno message
 * then exit(1)
 */

void err_exit(const char *fmt, ...);


/*
 * print a message with errno message
 * then exit 
 */

void err_quit(const char *fmt, ...);


#endif
