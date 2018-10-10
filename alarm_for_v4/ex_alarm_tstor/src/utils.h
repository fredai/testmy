/*
 *
 * utils.h
 *
 */

#ifndef _UTILS_H
#define _UTILS_H

/********************************************************/

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>

/********************************************************/

int ip_str_is_valid ( char * ip );

int parse_long_long_int ( char * str, long long int * long_long_int_value );

int parse_float ( char * str, float * float_value );

int parse_port ( char * str, unsigned short int * port );

char * trim_head ( char * str, int max_len );

void u_time_stamp_r ( char * buffer );

void u_time_stamp_by_t ( char * buffer, time_t t );

int u_sleep ( int seconds );

int u_msleep ( int seconds, int msec );

char * trim ( char * str, int max_size );

int check_pid ( char * path );

float strtof(const char *nptr, char **endptr);
#endif /* _UTILS_H */

/*end of file*/

