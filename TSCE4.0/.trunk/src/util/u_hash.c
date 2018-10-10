/*
 *
 * u_hash.c
 *
 */

#include <unistd.h>
#include <stdlib.h>

const int MOD_ADLER = 65521;

unsigned int adler32 ( unsigned char * data, int len ) {

    unsigned int a = 1, b = 0;

    int index;

    for ( index = 0; index < len; ++ index ) {

	a = ( a + data [ index ] ) % MOD_ADLER;
	b = ( b + a ) % MOD_ADLER;
    }

    return ( b << 16 ) | a;

}


unsigned short int fletcher16 ( unsigned char const * data, size_t bytes ) {

    unsigned short int sum1 = 0xff, sum2 = 0xff;

    while ( bytes ) {

	size_t tlen = bytes > 20 ? 20 : bytes;

	bytes -= tlen;

	do {
	    sum2 += sum1 += * data ++;

	} while ( -- tlen );

	sum1 = ( sum1 & 0xff ) + ( sum1 >> 8 );
	sum2 = ( sum2 & 0xff ) + ( sum2 >> 8 );
    }

    sum1 = ( sum1 & 0xff ) + ( sum1 >> 8 );

    sum2 = ( sum2 & 0xff ) + ( sum2 >> 8 );

    return sum2 << 8 | sum1;

}



/*end of file*/
