/*
 *
 * email_interface.h
 *
 */

#ifndef __EMAIL_INTERFACE_H
#define __EMAIL_INTERFACE_H


#define MAX_EMAIL_SMTP_STR_LEN 128
#define MAX_EMAIL_FROM_STR_LEN 128
#define MAX_EMAIL_USERNAME_STR_LEN MAX_EMAIL_FROM_STR_LEN
#define MAX_EMAIL_PASSWORD_LEN 128
#define MAX_EMAIL_TO_STR_LEN 128
#define MAX_EMAIL_TO_STR_NUM 32 
#define MAX_EMAIL_CC_STR_LEN 128
#define MAX_EMAIL_CC_STR_NUM 32

struct email_config_s {
    char smtp_server [ MAX_EMAIL_SMTP_STR_LEN + 1 ];
    unsigned short int smtp_port;
    char username [ MAX_EMAIL_USERNAME_STR_LEN + 1 ];
    char password [ MAX_EMAIL_PASSWORD_LEN + 1 ];
    char name [ MAX_EMAIL_FROM_STR_LEN + 1 ];
    char from [ MAX_EMAIL_FROM_STR_LEN + 1 ];
    char to [ MAX_EMAIL_TO_STR_NUM ] [ MAX_EMAIL_TO_STR_LEN + 1 ];
    char cc [ MAX_EMAIL_CC_STR_NUM ] [ MAX_EMAIL_CC_STR_LEN + 1 ];
    int use_tls_ssl;
};


typedef struct email_config_s email_config_t;

int send_email ( email_config_t * email_config, char * subject, char * content, \
        char * error_msg, int error_msg_len );


#endif

/*end of file*/

