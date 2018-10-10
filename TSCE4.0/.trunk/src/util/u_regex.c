/*
 *
 * u_regex.c
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include "u_regex.h"


int u_re_prepare ( char * pattern, u_re_t * u_re ) {

    if ( pattern == NULL || u_re == NULL || pattern [ 0 ] == '\0' ) {
        return -1;
    }

    pcre * re;
    pcre_extra * sd;

    const char * errmesg;
    int erroffset;

    re = pcre_compile ( pattern, 0, & errmesg, & erroffset, NULL );

    if ( re == NULL ) {
        return -2;
    }

    sd = pcre_study ( re, 0, & errmesg );

    u_re -> re = re;

    u_re -> sd = sd;

    return 0;

}



int u_re_exe ( u_re_t * u_re, char * subject, int subject_len ) {

#define _U_OVECOUNT_ 1024
    if ( u_re == NULL || subject == NULL || subject [ 0 ] == '\0' || subject_len < 1 ) {
        return -1;
    }

    if ( u_re -> re == NULL ) {
        return -2;
    }

    int ovector [ _U_OVECOUNT_ ] = { 0 };

    int rc = -1;

    rc = pcre_exec ( u_re -> re, u_re -> sd, subject, subject_len, 0, 0, ovector, _U_OVECOUNT_ );

    if ( rc == -1 ) { /*no match*/
        return 0;
    }

    if ( rc < 0 ) { /*error*/
        return -3;
    }

    if ( rc > 0 ) { /*match*/
        return rc;
    }

    return 0;

}




int u_re_finish ( u_re_t * u_re ) {

    if ( u_re == NULL ) {
        return -1;
    }
    
    if ( u_re -> re == NULL ) {
        return -2;
    }

    pcre_free_study ( u_re -> sd );

    if ( u_re -> sd != NULL ) {
        pcre_free ( u_re -> re );
    }

    u_re -> re = NULL;
    u_re -> sd = NULL;

    return 0;

}



/*end of file*/
