/*
 *
 * c_process_opt.c
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "e_define.h"
#include "d_define.h"
#include "d_nodes.h"
#include "u_util.h"


int t_process_cmd_options ( int argc, char * argv [ ], 
        char node_names [ ] [ MAX_NODE_NAME_LEN ], int num ) {

    if ( ! ( argc > 1 ) ) {
        return -1;
    }

    char cmd_line [ MAX_NODES_NUM*MAX_NODE_NAME_LEN + 256 ];
    char buffer [ MAX_NODE_NAME_LEN*10 + 16 ];
    int count = 0;
    int ix = 0;
    for ( ix = 1; ix < argc; ix ++ ) {
        strcat ( cmd_line, argv [ ix ] );
        strcat ( cmd_line, " " );
    }

    int cmd_str_len = strlen ( cmd_line );

    cmd_line [ cmd_str_len - 1 ] = 0;
    cmd_str_len --;

    for ( ix = 0; ix < cmd_str_len; ix ++ ) {
        if ( cmd_line [ ix ] == '[' ) {
            int jx = ix + 1;
            for ( jx = ix + 1; jx < cmd_str_len; jx ++ ) {
                if ( cmd_line [ jx ] == ']' ) {
                    break;
                }
            }
            if ( jx < cmd_str_len ) {
                int kx = ix - 1 ;
                for ( kx = ix - 1; kx >= 0; kx -- ) {
                    if ( cmd_line [ kx ] != ' ' ) {
                        break;
                    }
                }
                if ( kx != -1 ) {
                    int hx = kx;
                    for ( hx = kx; hx >= 0; hx -- ) {
                        if ( cmd_line [ hx ] == ' ' ) {
                            break;
                        }
                    }
                    hx ++;

                    int gx = ix;
                    for ( gx = ix; gx < jx; gx ++ ) {
                        if ( cmd_line [ gx ] == '-' ) {
                            break;
                        }
                    }

                    if ( gx < jx ) {
                        int lt_digit_bits = 0, rt_digit_bits = 0;
                        int fx = ix;
                        for ( fx = ix; fx < gx; fx ++ ) {
                            if ( isdigit ( cmd_line [ fx ] ) ) {
                                lt_digit_bits ++;
                            }
                        }
                        for ( fx = gx + 1; fx < jx; fx ++ ) {
                            if ( isdigit ( cmd_line [ fx ] ) ) {
                                rt_digit_bits ++;
                            }
                        }

                        if ( lt_digit_bits > 4 || lt_digit_bits > 4 ) {
                            /*format error*/
                            return -5;
                        }

                        long int range_a_long, range_b_long;
                        int range_a, range_b;
                        int ret;
                        char digit_str [ 16 ];
                        int cx = 0, bx = 0;
                        for ( cx = ix + 1; cx < gx; cx ++ ) {
                            if ( cmd_line [ cx ] != ' ' ) {
                                digit_str [ bx ] = cmd_line [ cx ];
                                bx ++;
                            }
                        }
                        digit_str [ bx ] = 0;
                        ret = parse_long_int ( digit_str, & range_a_long );
                        if ( ret ==  0 ) {
                            range_a = ( int ) range_a_long;
                        }
                        else {
                            /*format error, left invalid number*/
                            return -6;
                        }
                        cx = 0, bx = 0;
                        for ( cx = gx + 1; cx < jx; cx ++ ) {
                            if ( cmd_line [ cx ] != ' ' ) {
                                digit_str [ bx ] = cmd_line [ cx ];
                                bx ++;
                            }
                        }
                        digit_str [ bx ] = 0;
                        ret = parse_long_int ( digit_str, & range_b_long );
                        if ( ret == 0 ) {
                            range_b = ( int ) range_b_long;
                        }
                        else {
                            /*format error, right invalid number*/
                            return -7;
                        }

                        if ( range_a > range_b ) {
                            /*format error, range error*/
                            return -8;
                        }

                        int ex = hx;
                        int dx = 0;

                        for ( ex = hx; ex < ix; ex ++ ) {
                            if ( cmd_line [ ex ] != ' ' ) {
                                buffer [ dx ] = cmd_line [ ex ];
                                dx ++;
                                if ( dx > MAX_NODE_NAME_LEN ) {
                                    /*error, too long*/
                                    return -11;
                                }
                            }
                        }

                        while ( range_a <= range_b ) {

                            if ( lt_digit_bits == rt_digit_bits && lt_digit_bits > 1 ) {
                                char fmt [ ] = "%02d";
                                fmt [ 2 ] = '0' + lt_digit_bits; 
                                sprintf ( buffer + dx, fmt, range_a );
                            }

                            else {
                                sprintf ( buffer + dx, "%d", range_a );
                            }

                            range_a ++;

                            if ( strlen ( buffer ) > MAX_NODE_NAME_LEN ) {
                                /*error, too long*/
                                return -9;
                            }

                            strcpy ( node_names [ count ], buffer );

                            count ++;

                            if ( count == num ) {
                                /*break;*/
                                /*error, too long*/
                                return -10;
                            }

                        } /* for ( range_a < range_b ) */

                        memset ( cmd_line + hx, ' ', jx - hx + 1 );

                    } /* if ( gx < jx )*/

                    else {
                        /*format error no - */
                        return -4;
                    }

                } /* if ( kx != -1 ) */

                else {
                    /*format error, begin witch no character*/
                    return -3;
                }

            } /* if ( jx < cmd_str_len ) */

            else {
                /*format error no ] */
                return -2;
            }

        } /* if ( cmd_line [ ix ] == '[' ) */

    } /* for ( ix = 0; ix < cmd_str_len; ix ++ ) */


    char * delim = " \t\r\n\f\v";
    char * saveptr;
    char * token;
    char * str;
    
    for ( str = cmd_line; ; str = NULL ) {
        token = strtok_r ( str, delim, & saveptr );
        if ( token == NULL ) {
            break;
        }
        strcpy ( node_names [ count ], token );
        count ++;
    }


    return count;

}



/*end of file*/
