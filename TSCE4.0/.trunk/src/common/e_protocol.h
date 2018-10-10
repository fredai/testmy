/*
 *
 * e_protocol.h
 *
 */

#ifndef _E_PROTOCOL_H_
#define _E_PROTOCOL_H_


#pragma pack(1)
struct data_head_s {
    unsigned char data_type;
    unsigned char data_position;
    unsigned char data_status;
    unsigned char exit_code;
    unsigned int data_len;
};
#pragma pack()
typedef struct data_head_s data_head_t;




#endif
/*end of file*/
