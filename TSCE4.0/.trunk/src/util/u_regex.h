/*
 *
 * u_regex.h
 *
 */

#ifndef _U_REGEX_H_
#define _U_REGEX_H_

#include <pcre.h>


struct u_re_s {
    pcre * re;
    pcre_extra * sd;
};

typedef struct u_re_s u_re_t;



int u_re_prepare ( char * pattern, u_re_t * u_re );

int u_re_exe ( u_re_t * u_re, char * subject, int subject_len );

int u_re_finish ( u_re_t * u_re );


#endif
