/*
 *
 * t_send_cmd.h
 *
 */

#ifndef _T_SEND_CMD_H_
#define _T_SEND_CMD_H_

struct arg_send_cmd_s {
    char node_name [ 32 ];
    unsigned int node_ip;
};
typedef struct arg_send_cmd_s arg_send_cmd_t;

struct snd_conf_s {
    unsigned short int clr_cmd_port;
};
typedef struct snd_conf_s snd_conf_t;


void * thread_send_cmd ( void * arg );
int send_cmd_read_config_from_file ( char * config_file_path, snd_conf_t * snd_conf );
int parse_snd_config_line ( char * config_line, snd_conf_t * snd_conf );




#endif
/*end of file*/
