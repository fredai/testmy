/*
 *
 * u_util.c
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "u_util.h"


char * str_get_item ( char * dest, char * src, int max_len ) {
    if ( src == NULL || dest == NULL || max_len < 1 ) {
        return NULL;
    }
    int j = -1;
    while ( ++j < max_len ) {
        if ( * ( src + j ) == '\0' || isspace ( * ( src + j ) ) ) {
            break;
        }
    }
    strncpy ( dest, src, j );
    dest [ j ] = 0;
    return dest;
}


int ip_str_is_valid ( char * ip ) {

    if ( ip == NULL || ip [ 0 ] == '\0' ) {
        return -1;
    }

    struct in_addr tbf;
    return inet_pton ( AF_INET, ip, & tbf );

}

int parse_long_int ( char * str, long int * long_int_value ) {

    if ( str == NULL || str [ 0 ] == '\0' || long_int_value == NULL ) {
        return -1;
    }

    const int base = 10;
    char * endptr;
    long int value = 0;
    errno = 0;

    value = strtol ( str, & endptr, base );

    if ( errno == ERANGE ) {
        if ( value == LONG_MAX ) {
            return -2;
        }
        else if ( value == LONG_MIN ){
            return -3;
        }
        else {
            return -4;
        }
    }

    else if ( errno != 0 && value == 0 ) {
        return -5;
    }
            
    if ( endptr == str ) {
        return -6;
    }

    if ( * endptr != '\0' ) {
        return -7;
    }

    * long_int_value = value;

    return 0;

}


int parse_port ( char * str, unsigned short int * port ) {

    if ( str == NULL || str [ 0 ] == '\0' || port == NULL ) {
        return -1;
    }
    int ret = -1;
    long int port_value;
    ret = parse_long_int ( str, & port_value );
    if ( ret < 0 ) {
        return -2;
    }
    if ( port_value < 1 || port_value > 65535 ) {
        return -3;
    }

    * port = ( unsigned short int ) port_value;
    
    return 0;
}

char * trim_head ( char * str, int max_len ) {

    if ( str == NULL ) {
        return NULL;
    }
    int cur_len = 0;
    char * index = str;
    while ( ( * index ) != '\0' && cur_len < max_len && isspace ( ( int ) ( * index ) ) ) {
        index ++;
        cur_len ++;
    }
    return index;

}


int ipv4_pton ( const char * src, struct in_addr * dst ) {
    
    if ( src == NULL || dst == NULL || src [ 0 ] == '\0' ) {
        return -1;
    }
    int ret = -1;
    ret = inet_pton ( AF_INET, src, dst );
    if ( ret == 1 ) {
        return 0;
    }
    else if ( ret == 0 ) {
        return -2; /*src does not contain a character string representing a valid network address*/
    }
    else {
        return -3;
    }
}

int ipv4_ntop ( struct in_addr * src, char * dst, const int size ) {
    
    if ( src == NULL || dst == NULL ) {
        return -1;
    }
    const char * addr = NULL;
    addr = inet_ntop ( AF_INET, src, dst, ( socklen_t ) size );
    if ( addr == NULL ) {
        return -2;
    }
    else {
        return 0;
    }
}



int ipv4_str_to_uint ( char * str, unsigned int * ip ) {

    if ( str == NULL || ip == NULL || str [ 0 ] == '\0' ) {
        return -1;
    }

    int ret;
    struct in_addr in;

    memset( & in, 0, sizeof ( struct in_addr ) );

    ret = inet_pton ( AF_INET, str, & in );
    if ( ret == 0 ) {
        return -2;
    }
    else if ( ret == -1 ) {
        return -3;
    }

    * ip = in.s_addr;
    
    return 0;

}


int ipv4_uint_to_str ( unsigned int ip, char * dst_str, int size ) {

    if ( dst_str == NULL ) {
        return -1;
    }

    const char * addr = NULL;
    struct in_addr in;
    memset( & in, 0, sizeof ( struct in_addr ) );
    in.s_addr = ip;
    addr = inet_ntop ( AF_INET, & in, dst_str, ( socklen_t ) size );
    if ( addr == NULL ) {
        return -2;
    }
    else {
        return 0;
    }
}


int hostname_to_ip ( const char * str, unsigned int * ip ) {

    if ( str == NULL || str [ 0 ] == '\0' || ip == NULL ) {
        return -1;
    }

    struct sockaddr_in sa;
    struct addrinfo hints, * result = NULL;
    int ret = -1;

    memset ( & sa, 0, sizeof ( sa ) );
    memset ( & hints, 0, sizeof ( struct addrinfo ) );

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    ret = getaddrinfo ( str, NULL, & hints, & result );

    if ( ret != 0 ) {
        return -2;
    }

    memcpy ( ( void * ) & sa, ( void * ) result -> ai_addr, result -> ai_addrlen ) ;

    freeaddrinfo ( result );

    * ip = sa.sin_addr.s_addr;
    
    return 0; 

}


int sin_addr_to_uint ( struct sockaddr_in * sa, unsigned int * ip ) {

    if ( sa == NULL || ip == NULL ) {
        return -1;
    }

    * ip = sa -> sin_addr.s_addr;
    
    return 0; 

}

int in_addr_to_uint ( struct in_addr * in, unsigned int * ip ) {

    if ( in == NULL || ip == NULL ) {
        return -1;
    }

    * ip = in -> s_addr;
    
    return 0; 

}


void u_time_stamp_r ( char * buffer ) {

    time_t t;
    time ( & t );
    struct tm tmt;
    localtime_r ( & t, & tmt );
    strftime ( buffer, 20, "%Y-%m-%d %H:%M:%S", & tmt );

}

struct timeval timeval_current ( void ) {

	struct timeval tv;

	gettimeofday ( & tv, NULL );

	return tv;

}

double timeval_elapsed ( struct timeval * tv ) {

	struct timeval tv2 = timeval_current ( );

	return ( tv2.tv_sec - tv -> tv_sec ) + 
	       ( tv2.tv_usec - tv -> tv_usec ) * 1.0e-6;

}

int u_sleep ( int seconds ) {

    int ret = -1;
    struct timespec req;
    req.tv_sec = seconds;
    req.tv_nsec = 0;
    struct timespec rem;
    bzero ( & rem, sizeof ( struct timespec ) );
    ret = nanosleep ( & req, & rem );
    if ( ret == -1 ) {
        if ( errno == EINTR ) {
            return rem.tv_sec;
        }
        else if ( errno == EINVAL ) {
            return -1;
        }
        else if ( errno == EFAULT ) {
            return -2;
        }
        else {
            return -3;
        }
    }
    return 0;
}


int u_msleep ( int seconds, int msec ) {

    if ( msec < 0 || msec > 999 ) {
        return -1;
    }

    int ret = -1;
    struct timespec req;
    req.tv_sec = seconds;
    req.tv_nsec = msec * 1000 * 1000;
    struct timespec rem;
    bzero ( & rem, sizeof ( struct timespec ) );
    ret = nanosleep ( & req, & rem );
    if ( ret == -1 ) {
        if ( errno == EINTR ) {
            return rem.tv_sec;
        }
        else if ( errno == EINVAL ) {
            return -2;
        }
        else if ( errno == EFAULT ) {
            return -3;
        }
        else {
            return -4;
        }
    }
    return 0;
}


inline int max_int ( int a, int b ) {
    return a > b ? a : b;
}


int u_copy_file ( char * file_dest, char * file_src ) {

    if ( file_src == NULL || file_dest == NULL || file_src [ 0 ] == '\0' || file_dest [ 0 ] == '\0' ) {
        return -1;
    }

    int fd_src, fd_dest;

    mode_t mode = umask ( 0 ); 
    fd_dest = open ( file_dest, O_WRONLY | O_CREAT | O_TRUNC, 0666 );
    umask ( mode );
    if ( fd_dest == -1 ) {
        return -2;
    }

    mode = umask ( 0 ); 
    fd_src = open ( file_src, O_RDONLY, 0660 );
    umask ( mode );
    if ( fd_src == -1 ) {
        return -3;
    }

    int ret;
    char copy_buffer [ 1024 ];

    while ( 1 ) {
        ret = read ( fd_src, copy_buffer, sizeof ( copy_buffer ) );
        if ( ret == 0 ) {
            break;
        }
        ret = write ( fd_dest, copy_buffer, ret );
        if ( ret == -1 ) {
            return -4;
        }
    }

    close ( fd_src );
    close ( fd_dest );

    return 0;

}



int u_mkdir_p ( char * path ) {

#define DIR_PATH_MAX (1024*2)

    char dir_name [ DIR_PATH_MAX ];
    char * retp = NULL;
    if ( path == NULL || path [ 0 ] == '\0' ) {
        return -1;
    }
    if ( path [ 0 ] != '/' ) {
        retp = getcwd ( dir_name, DIR_PATH_MAX );
        if ( retp == NULL ) {
            return -2;
        }
        strcat ( dir_name, "/" );
        strcat ( dir_name, path );
    }
    else {
        strcpy ( dir_name, path );
    }
    strcat ( dir_name, "/" );

    //char * dir_str = NULL;
    char temp_dirname [ DIR_PATH_MAX ];

    int ix = 0;
    int ret = -1;
    mode_t mode;
    while ( dir_name [ ++ ix ] != '\0' ) {
        if ( dir_name [ ix ] == '/' ) {
            strncpy ( temp_dirname, dir_name, ix );
            mode = umask ( 0 );
            ret = mkdir ( temp_dirname, 0775 );
            umask ( mode );
            
            if ( ret == 0 ) {
                continue;
            }
            else if ( errno == EACCES ) {
                return -3;
            }
            else if ( errno == EEXIST ) {
                continue;
            }
            else {
                return -4;
            }
        }
    }

    return 0;

}



int check_pid ( char * path ) {

#define CHECK_PID_BUF_LEN 64
    if ( path == NULL || path [ 0 ] == '\0' ) {
        return -1;
    }

    int ret;
    char buffer [ CHECK_PID_BUF_LEN ];
    long int file_pid;
    char proc_path [ CHECK_PID_BUF_LEN + 8 ];

    FILE * fp = fopen ( path, "r" );
    if ( fp == NULL ) {
        return -2;
    }

    buffer [ 0 ] = '\0';
    fgets ( buffer, CHECK_PID_BUF_LEN, fp );
    fclose ( fp );

    file_pid = atoll ( buffer );

    sprintf ( proc_path, "%s/%s", "/proc", buffer );

    ret = access ( proc_path, F_OK );
    if ( ret == 0 ) {
        return  ( int ) file_pid;
    }
    else {
        return -4;
    }
}



/*end of file*/
