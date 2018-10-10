/*
 *
 * u_hash.h
 *
 */

#ifndef _U_HASH_H_
#define _U_HASH_H_

#include <unistd.h>
#include <stdlib.h>


unsigned int adler32 ( unsigned char * data, int len );

unsigned short int fletcher16 ( unsigned char const * data, size_t byte );




#endif



/*end of file*/
