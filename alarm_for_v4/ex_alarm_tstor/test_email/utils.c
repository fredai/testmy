/*
 *
 * utils.c
 *
 */

#include <unistd.h>
#include <stdlib.h>
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

#include "utils.h"


/*
 * Function to check ip string 
 * @ip:   ip string 
 * RET 0 or 1 on success, otherwise return a negative number
 */

int ip_str_is_valid ( char * ip ) {

    if ( ip == NULL || ip [ 0 ] == '\0' ) {
        return -1;
    }
    
    struct in_addr tbf;

    return inet_pton ( AF_INET, ip, & tbf );

}

/*
 * Function to check ip string 
 * @str:   string to parse
 * @long_int_value: long int value
 * RET 0 on success, otherwise return a negative number
 */

int parse_long_long_int ( char * str, long long int * long_int_value ) {

    if ( str == NULL || str [ 0 ] == '\0' || long_int_value == NULL ) {
        return -1;
    }

    const int base = 10;
    char * endptr;
    long long int value = 0;
    errno = 0;

    value = strtoll ( str, & endptr, base );

    /* range error */
    if ( errno == ERANGE ) {
        /* too long */
        if ( value == LONG_MAX ) {
            return -2;
        }
        /* too short */
        else if ( value == LONG_MIN ){
            return -3;
        }
        else {
            return -4;
        }
    }

    /* other error */
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


/*
 * Function to check ip string 
 * @str:   string to parse
 * @float_value: float value
 * RET 0 on success, otherwise return a negative number
 */

int parse_float ( char * str, float * float_value ) {

    if ( str == NULL || str [ 0 ] == '\0' || float_value == NULL ) {
        return -1;
    }

    char * endptr;
    float value = 0;
    errno = 0;

    value = strtof ( str, & endptr );

    /* range error */
    if ( errno == ERANGE ) {
        return -2;
    }

    /* other error */
    else if ( errno != 0 ) {
        return -3;
    }
            
    if ( endptr == str ) {
        return -4;
    }

    if ( * endptr != '\0' ) {
        return -5;
    }

    * float_value = value;

    return 0;

}


/*
 * Function to parse port 
 * @str:   string to parse
 * @port:  pointer to port
 * RET 0 on success, otherwise return a negative number
 */

int parse_port ( char * str, unsigned short int * port ) {

    if ( str == NULL || str [ 0 ] == '\0' || port == NULL ) {
        return -1;
    }
    int ret = -1;
    long long int port_value;
    ret = parse_long_long_int ( str, & port_value );
    if ( ret < 0 ) {
        return -2;
    }
    if ( port_value < 1 || port_value > 65535 ) {
        return -3;
    }

    * port = ( unsigned short int ) port_value;
    
    return 0;
}


/*
 * Function to trim head 
 * @str:   string to parse
 * @port:  pointer to port
 * RET string success, otherwise return NULL
 */

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


/*
 * Function to trim a string 
 * @str:   string to parse
 * @max_size:  max size
 * RET string on success, otherwise return NULL 
 */

char * trim ( char * str, int max_size ) {
    if ( str == NULL ) {
        return NULL;
    }
    int len = strlen ( str );
    int i = -1;
    int j = -1;
    char * new_str = malloc ( len + 1 );
    if ( new_str == NULL ) {
        return NULL;
    }
    while ( ++ i < len ) {
        if ( ! isspace ( ( int ) str [ i ] ) ) {
            new_str [ ++ j ] = str [ i ];
        }
    }
    new_str [ j + 1 ] = '\0';
    strcpy ( str, new_str );
    free ( new_str );
    return str;

}


/*
 * Function to produce a timestamp
 * @buffer:  timestamp buffer
 */

void u_time_stamp_r ( char * buffer ) {

    time_t t;
    time ( & t );
    struct tm tmt;
    localtime_r ( & t, & tmt );
    strftime ( buffer, 20, "%Y-%m-%d %H:%M:%S", & tmt );

}


void u_time_stamp_by_t ( char * buffer, time_t t ) {

    time ( & t );
    struct tm tmt;
    localtime_r ( & t, & tmt );
    strftime ( buffer, 20, "%Y-%m-%d %H:%M:%S", & tmt );

}

/*
 * Function to sleep some seconds
 * @seconds: seconds to sleep 
 * RET 0 on success, otherwise return a negative number
 */

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


/*
 * Function to sleep some seconds and microseconds
 * @seconds: seconds to sleep 
 * @msec: microseconds to sleep 
 * RET 0 on success, otherwise return a negative number
 */

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







/*end of file*/

