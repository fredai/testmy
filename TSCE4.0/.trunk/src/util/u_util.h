/*
 *
 * u_util.h
 *
 */

#ifndef _U_UTIL_H_
#define _U_UTIL_H_

#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>


char * str_get_item ( char * dest, char * src, int max_len );
int ip_str_is_valid ( char * ip );
int parse_long_int ( char * str, long int * long_int_value );
int parse_port ( char * str, unsigned short int * port );
char * trim_head ( char * str, int max_len );
int ipv4_pton ( const char * src, struct in_addr * dst );
int ipv4_ntop ( struct in_addr * src, char * dst, const int size );
int ipv4_str_to_uint ( char * str, unsigned int * ip );
int hostname_to_ip ( const char * str, unsigned int * ip );
int ipv4_uint_to_str ( unsigned int ip, char * dst_str, int size );
int sin_addr_to_uint ( struct sockaddr_in * sa, unsigned int * ip );
int in_addr_to_uint ( struct in_addr * in, unsigned int * ip );
void u_time_stamp_r ( char * buffer );
struct timeval timeval_current ( void );
double timeval_elapsed ( struct timeval * tv );
int u_sleep ( int seconds );
int u_msleep ( int seconds, int msec );
inline int max_int ( int a, int b );
int u_copy_file ( char * file_dest, char * file_src );
int u_mkdir_p ( char * path );
int check_pid ( char * path );






#endif
/*end of file*/
