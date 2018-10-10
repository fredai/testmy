/*
 *
 * log.c
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include "u_hash.h"
#include "u_log.h"


#define LOG_KEY_SERVER_MODE (0660|IPC_CREAT|IPC_EXCL)
#define LOG_KEY_CLIENT_MODE 0660
#define LOG_KEY_DESTROY_MODE 0660 

#define LOG_MAX_PATH_LEN 512 
#define LOG_MAX_LOG_FILENAME_LEN 64 
#define LOG_FILE_MODE 0660
#define MAX_LEVEL_PATH_STR_LEN 64
#define MAX_PATH_LEVEL 128

#define TIME_STAMP_LEN 20

#define LOG_RECV_BUFFER_LEN (1024*4)
#define LOG_SEND_BUFFER_LEN (LOG_RECV_BUFFER_LEN-32)
#define LOG_USER_BUFFER_LEN (LOG_SEND_BUFFER_LEN-128)
#define LOG_WRITE_BUFFER_LEN (LOG_RECV_BUFFER_LEN+128)
#define MSG_TYPE_HEAD_LEN (sizeof(long int))
#define LOG_BUFFER_HEAD_LEN 4

#define LOG_START_OK_FLAG 0xef
#define LOG_START_OK_FALG_LEN 1
#define LOG_SELECT_WAIT_TIME 2
#define LOG_ROTATE_TIME (24*60*60)
#define LOG_EXIT_IN 0xffeeffee


struct log_send_buffer_s {
    long int type;
    char buf [ LOG_SEND_BUFFER_LEN ];
};

struct log_recv_buffer_s {
    long int type;
    char buf [ LOG_RECV_BUFFER_LEN ];
};

typedef struct log_send_buffer_s log_send_buffer_t;
typedef struct log_recv_buffer_s log_recv_buffer_t;


void log_sa_alrm_handler ( int sig_num );
int log_process_signal ( );
static int get_alarm_time_of_day ( );
static void produce_time_stamp_r ( char * buffer );
static int split_path_and_filename ( char * path_and_filename, char * pathname, char * filename );
int mkdir_p ( char * path );
static int produce_absolute_path ( char * relative_path, char * p_absolute_path );
static int check_log_path ( char * absolute_log_path );
void log_sa_usr1_handler ( int sig_num );
void log_exit ( void );

static const char * g_log_level_string [ 5 ] = { 
        "[DEBUG] ",
        "[INFO] ",
        "[WARN] ",
        "[ERROR] ",
        "[FATAL] " 
};

static const int g_log_level_string_len [ ] = {
        8,
        7,
        7,
        8,
        8
};

static char g_absolute_log_path [ LOG_MAX_PATH_LEN * 2 + 1 ];

static pid_t logger_pid;


int init_logger ( char * log_file_path ) {

    if ( log_file_path == NULL || log_file_path [ 0 ] == '\0' ) {
        return -1;
    }
    if ( strlen ( log_file_path ) > LOG_MAX_PATH_LEN * 2 ) {
        return -2;
    }
    if ( sizeof ( int ) != LOG_BUFFER_HEAD_LEN ) {
        return -39;
    }

    int ret = -1;
    char absolute_log_path [ LOG_MAX_PATH_LEN * 2 + 1 ];

    ret = produce_absolute_path ( log_file_path, absolute_log_path );
    if ( ret < 0 ) {
        return -3;
    }

    ret = check_log_path ( absolute_log_path );
    if ( ret < 0 ) {
        return -4;
    }

    destroy_logger ( absolute_log_path );
    int pipefd [ 2 ];

    if ( pipe ( pipefd ) == -1 ) { 
        return -5; 
    }

    int fork_result;

    fork_result = fork ( );
    if ( fork_result == -1 ) {
        return -6;
    }
    else if ( fork_result > 0 ) {
        struct timeval timeout;
        timeout.tv_sec = LOG_SELECT_WAIT_TIME;
        timeout.tv_usec = 0;

        int select_result;
        int nread;
        unsigned char log_start_result;

        close ( pipefd [ 1 ] );

        fd_set inputs;
        FD_ZERO ( & inputs );
        FD_SET ( pipefd [ 0 ], & inputs );

        select_result = select ( FD_SETSIZE, & inputs, NULL, NULL, & timeout );
        if ( select_result == -1 ) {
            return -7; 
        }
        else if ( select_result == 0 ) {
            kill ( fork_result, SIGKILL );
            return -8; 
        }
        else if ( select_result == 1 ) {
            ioctl ( pipefd [ 0 ], FIONREAD, & nread );
            if ( nread == LOG_START_OK_FALG_LEN ) {
                nread = read ( pipefd [ 0 ], ( void * ) & log_start_result, nread );
            }
            else if ( nread == 0 ) {
                return -9; 
            }
            else {
                return -10; 
            }
        }

        close ( pipefd [ 0 ] );

        if ( log_start_result != LOG_START_OK_FLAG ) {
            return -11; 
        }

        logger_pid = fork_result;
        atexit ( log_exit );
    }
    else if ( fork_result == 0 ) {

        close ( pipefd [ 0 ] );
        strcpy ( g_absolute_log_path, absolute_log_path );
        ret = log_process_signal ( );
        if ( ret < 0 ) {
            exit ( 38 );
        }

        log_recv_buffer_t log_recv_buffer;
        int write_log_fd;
        int write_result;
        unsigned int log_msg_key;
        int log_msgid;
        long int log_msg_rcv_type = 0;
        char * log_recv_buffer_start;
        char log_write_buffer [ LOG_WRITE_BUFFER_LEN ];

        log_msg_key = adler32 ( ( unsigned char * ) absolute_log_path, strlen ( absolute_log_path ) );
        log_msgid = msgget ( log_msg_key, LOG_KEY_SERVER_MODE );
        if ( log_msgid == -1 ) {
            if ( errno == EACCES ) { 
                exit ( 13 );
            }
            else if ( errno == EEXIST ) { 

            }
            else {
                exit ( 15 );
            }
        }

        mode_t log_file_mode;
        log_file_mode = umask ( 0 );
        write_log_fd = open ( absolute_log_path, O_WRONLY|O_APPEND|O_CREAT, LOG_FILE_MODE );
        umask ( log_file_mode );
        if ( write_log_fd == -1 ) {
            if ( errno == EACCES ) {
                exit ( 16 );
            }
            else if ( errno == EFAULT ) {
                exit ( 17 );
            }
            else if ( errno == EISDIR ) {
                exit ( 18 );
            }
            else {
                exit ( 19 );
            }
        }

        const unsigned char start_res = LOG_START_OK_FLAG;
        ret = write ( pipefd [ 1 ], ( void * ) & start_res, 1 );
        if ( ret == -1 ) {
	    exit ( 20 );
        }

        close ( pipefd [ 1 ] );
        alarm ( get_alarm_time_of_day ( ) );

        int rcv_count;

        while ( 1 ) { 

SIGNAL_INTERRUPT_RECOVERY:

            rcv_count = msgrcv ( log_msgid, ( void * ) & log_recv_buffer, LOG_RECV_BUFFER_LEN, log_msg_rcv_type, 0 );

            if ( rcv_count == -1 ) {

                if ( errno == E2BIG ) {
                    exit ( 21 );
                }
                else if ( errno == EACCES ) {
                    exit ( 22 );
                }
                else if ( errno == EIDRM ) {
                    exit ( 23 );
                }
                else if ( errno == EINTR ) {
                    goto SIGNAL_INTERRUPT_RECOVERY;

                }
                else {
                    exit ( 25 );
                }
            }
            else {
                int log_len = * ( int * ) log_recv_buffer.buf;
                if ( log_len == LOG_EXIT_IN ) {
                    goto LOGGER_EXIT;
                }
                else if ( log_len != rcv_count - LOG_BUFFER_HEAD_LEN ) {
                    continue;
                }

                log_recv_buffer_start = log_recv_buffer.buf + LOG_BUFFER_HEAD_LEN;
                produce_time_stamp_r ( log_write_buffer );
                log_write_buffer [ TIME_STAMP_LEN - 1 ] = ' ';
                memcpy ( log_write_buffer + TIME_STAMP_LEN, log_recv_buffer_start, log_len );
                log_write_buffer [ log_len + TIME_STAMP_LEN ] = '\n';
                log_write_buffer [ log_len + TIME_STAMP_LEN + 1 ] = 0;

                ret = access ( absolute_log_path, R_OK|W_OK|F_OK );
                if ( ret == -1 ) { 
                    char path_of_log [ LOG_MAX_PATH_LEN * 2 - LOG_MAX_LOG_FILENAME_LEN + 1 ];
                    ret = split_path_and_filename ( absolute_log_path, path_of_log, NULL );
                    if ( ret < 0 ) {
                        exit ( 26 );
                    }

                    ret = access ( path_of_log, R_OK|W_OK|F_OK );
                    if ( ret == -1 ) {
                        if ( errno == EACCES ) {
                            exit ( 27 );
                        }
                        else if ( errno == ENOENT ) {
                            ret = mkdir_p ( path_of_log );
                            if ( ret < 0 ) {
                                exit ( 28 );
                            }
                        }
                    }
                    close ( write_log_fd );

                    mode_t log_file_mode;
                    log_file_mode = umask ( 0 );
                    write_log_fd = open ( absolute_log_path, O_WRONLY|O_APPEND|O_CREAT, 
                            LOG_FILE_MODE );
                    umask ( log_file_mode );

                    if ( write_log_fd == -1 ) {
                        if ( errno == EACCES ) {
                                exit ( 29 );
                        }
                        else if ( errno == EFAULT ) {
                            exit ( 30 );
                        }
                        else if ( errno == EISDIR ) {
                            exit ( 31 );
                        }
                        else {
                            exit ( 32 );
                        }
                    } /* if ( write_log_fd == -1 ) */
                } /* if ( ret == -1 ) */

                write_result = write ( write_log_fd, log_write_buffer, log_len + TIME_STAMP_LEN + 1 );
                if ( write_result < 0  ) {
                    exit ( 33 );
                }
                else if ( write_result != log_len + TIME_STAMP_LEN + 1 ) {
                    exit ( 34 );
                }
                else {
                }
            } /* else */
        } /* while ( 1 ) */

LOGGER_EXIT:

        ret = msgctl ( log_msgid, IPC_RMID, 0 );
        if ( ret != 0 ) {
            if ( errno == EIDRM ) {
                exit ( 35 );
            }
            else {
                exit ( 36 );
            }
        }
        exit ( 0 );
    } /* else if ( fork_result == 0 ) */

    return 0;

}


int start_log ( char * log_file_path, log_level_t log_level, log_t * handle ) {

    if ( log_file_path == NULL || log_file_path [ 0 ] == '\0' || handle == NULL ) {
        return -1;
    }

    handle -> log_id = -1;
    if ( strlen ( log_file_path ) > LOG_MAX_PATH_LEN * 2 ) {
        return -2;
    }

    int ret = -1;
    unsigned int log_start_msg_key;
    int log_start_msgid;
    char absolute_log_path [ LOG_MAX_PATH_LEN * 2 + 1 ];
    ret = produce_absolute_path ( log_file_path, absolute_log_path );
    if ( ret < 0 ) {
        return -3;
    }
    log_start_msg_key = adler32 ( ( unsigned char * ) absolute_log_path, strlen ( absolute_log_path ) );
    log_start_msgid = msgget ( log_start_msg_key, LOG_KEY_CLIENT_MODE );
    if ( log_start_msgid == -1 ) {
        if ( errno == EACCES ) { 
           return -4;
        }
       else if ( errno == ENOENT ) { 
            return -5;
        }
        else {
            return -6;
        }
    }

    handle -> log_id = log_start_msgid;
    handle -> log_level = log_level;

    return 0;

}


int u_log ( log_t * log_handle, log_level_t level, const char * fmt, ... ) {

    if ( log_handle == NULL ) {
        return -1;
    }
    if ( log_handle -> log_id == -1 ) {
        return -2;
    }
    if ( level < log_handle -> log_level ) {
        return -3;
    }

    char log_user_buffer [ LOG_USER_BUFFER_LEN * 2 ];
    char * log_user_buffer_start;
    int user_log_len;
    int ret = -1;
    int lev_str_len = g_log_level_string_len [ level ];
    memcpy ( log_user_buffer, g_log_level_string [ level ], lev_str_len );

    log_user_buffer_start = log_user_buffer + lev_str_len;

    va_list argp;
    va_start ( argp, fmt );
    user_log_len = vsprintf ( log_user_buffer_start, fmt, argp );
    va_end ( argp );

    if ( user_log_len > LOG_USER_BUFFER_LEN ) {
        return -4;
    }

    log_send_buffer_t log_send_buffer;
    log_send_buffer.type = 1;
    user_log_len += lev_str_len;
    * ( int * ) log_send_buffer.buf = user_log_len;
    memcpy ( log_send_buffer.buf + LOG_BUFFER_HEAD_LEN, log_user_buffer, user_log_len );

    ret = msgsnd ( log_handle -> log_id, ( void * ) & log_send_buffer, 
            LOG_BUFFER_HEAD_LEN + user_log_len, 0 );
    if ( ret == -1 ){
        if ( errno == EACCES ){
            return -5;
        }
        else if ( errno == EIDRM ){
            return -6;
        }
        else if ( errno == EINTR ){
            return -7;
        }
        else {
            return -8;
        }
    }

    return user_log_len - lev_str_len;

}


int destroy_logger ( char * log_file_path ) {

    if ( log_file_path == NULL || log_file_path [ 0 ] == '\0' ) {
        return -1;
    }
    if ( strlen ( log_file_path ) > LOG_MAX_PATH_LEN * 2 ) {
        return -2;
    }

    int ret = -1;
    log_t log_handle;
    int try_count = 0;

    sleep ( 1 );

RETRY_DESTROY_START:

    ret = start_log ( log_file_path, LOG_INFO, & log_handle );
    if ( ret < 0 ) {
        if ( try_count < 5 ) {
            try_count ++;
            goto RETRY_DESTROY_START;
        }
        else {
            return -3;
        }
    }


    log_send_buffer_t log_send_buffer;
    log_send_buffer.type = 1;
    * ( int * ) log_send_buffer.buf = LOG_EXIT_IN;

    ret = msgsnd ( log_handle.log_id, ( void * ) & log_send_buffer, LOG_BUFFER_HEAD_LEN, 0 );
    if ( ret == -1 ) {
        if ( errno == EACCES ) {
            return -4;
        }
        else if ( errno == EIDRM ){
            return -5;
        }
        else if ( errno == EINTR ){
            return -6;
        }
        else {
            return -7;
        }
    }

    char absolute_log_path [ LOG_MAX_PATH_LEN * 2 + 1 ];
    produce_absolute_path ( log_file_path, absolute_log_path );

    unsigned int log_destroy_key =
        adler32 ( ( unsigned char * ) absolute_log_path, strlen ( absolute_log_path ) );

    int log_destroy_msgid = msgget ( log_destroy_key, LOG_KEY_DESTROY_MODE );
    if ( log_destroy_msgid != -1 ) {
        msgctl ( log_destroy_msgid, IPC_RMID, 0 );
    }

    ret = end_log ( & log_handle );
    if ( ret < 0 ) {
        return -8;
    }

    return 0;

}


int end_log ( log_t * log_handle ) {
    if ( log_handle == NULL ) {
        return -1;
    }
    log_handle -> log_id = -1;
    log_handle -> log_level = -1;
    return 0;
}


static int get_alarm_time_of_day ( ) {
    time_t now_time;
    struct tm now_tmt;
    int alarm_time;
    time ( & now_time );
    localtime_r ( & now_time, & now_tmt );
    alarm_time = LOG_ROTATE_TIME - ( now_tmt.tm_hour*60*60 + now_tmt.tm_min*60 + now_tmt.tm_sec );
    return alarm_time;
}



static void produce_time_stamp_r ( char * buffer ) {
    time_t t;
    time ( & t );
    struct tm tmt;
    localtime_r ( & t, & tmt );
    strftime ( buffer, 20, "%Y-%m-%d %H:%M:%S", & tmt );
}


int set_log_level ( log_t * log_handle, log_level_t log_level ) {
    if ( log_handle == NULL ) {
        return -1;
    }
    log_handle -> log_level = log_level;
    return 0;
}


int mkdir_p ( char * path ) {
#define DIR_PATH_MAX 1024
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


static int split_path_and_filename ( char * path_and_filename, char * pathname, char * filename ) {

    if ( path_and_filename == NULL || path_and_filename [ 0 ] == '\0' ) {
        return -1;
    }
    if ( pathname == NULL && filename == NULL ) {
        return -2;
    }
    if ( path_and_filename [ strlen ( path_and_filename ) - 1 ] == '/' ) {
        return -3;
    }
    int path_start = -1, path_end = -1;
    int file_start = -1, file_end = -1;
    int length = -1;

    length = strlen ( path_and_filename );

    path_start = 0;
    file_end = length - 1;

    while ( path_and_filename [ -- length ] != '/' );

    path_end = length;
    file_start = length + 1;

    if ( pathname != NULL ) {
        strncpy ( pathname, path_and_filename + path_start, path_end - path_start + 1 );
        pathname [ path_end - path_start + 1 ] = 0;
    }

    if ( filename != NULL ) {
        strncpy ( filename, path_and_filename + file_start, file_end - file_start + 1 );
        filename [ file_end - file_start + 1 ] = 0;
    }
    
    return 0;
}


static int check_log_path ( char * absolute_log_path ) {

    if ( absolute_log_path == NULL || absolute_log_path [ 0 ] == '\0' ) {
        return -1;
    }

    int ret;

    char filename [ LOG_MAX_LOG_FILENAME_LEN + 1 ];
    char pathname [ LOG_MAX_PATH_LEN * 2 - LOG_MAX_LOG_FILENAME_LEN + 1 ];

    ret = split_path_and_filename ( absolute_log_path, pathname, filename );

    if ( ret < 0 ) {
        return -2;
    }

    ret = access ( pathname, R_OK|W_OK|F_OK );
    if ( ret == -1 ) {
        if ( errno == EACCES ) {
            return -3;
        }
        if ( errno == ENOENT ) {
            ret = mkdir_p ( pathname );
            if ( ret < 0 ) {
                return -4;
            }
        }
        else {
            return -5;
        }
            
    }

    mode_t file_mode = umask ( 0 );
    ret = open ( absolute_log_path, O_WRONLY|O_APPEND|O_CREAT, LOG_FILE_MODE );
    umask ( file_mode );

    if ( ret == -1 ) {
        if ( errno == EACCES ) {
            return -6;
        }

        else if ( errno == EFAULT ) {
            return -7;
        }

        else if ( errno == EISDIR ) {
            return -8;
        }

        else {
            return -9;
        }
    }

    close ( ret );

    return 0;

}



void log_sa_alrm_handler ( int sig_num ) {

    int ret = -1;

    char filename [ LOG_MAX_LOG_FILENAME_LEN + 1 ];
    char new_path_name [ LOG_MAX_PATH_LEN * 2 + 1 ];

    ret = split_path_and_filename ( g_absolute_log_path, new_path_name, filename );
    if ( ret < 0 ) {
        alarm ( LOG_ROTATE_TIME );
        return;
    }

    char buffer [ 16 ];
    time_t t = time ( NULL ) - 60*10;
    struct tm tmt;
    localtime_r ( & t, & tmt );
    strftime ( buffer, 16, "%Y-%m-%d", & tmt );

    strncat ( new_path_name, "/", 1 );
    strncat ( new_path_name, buffer, 10 );
    strncat ( new_path_name, "/", 1 );

    mode_t mode = umask ( 0 );
    ret = mkdir ( new_path_name, 0775 );
    umask ( mode );
    if ( ret == -1 ) {
        if ( errno == EEXIST ) {
        }
        else {
            alarm ( LOG_ROTATE_TIME );
            return;
        }
    }

    strcat ( new_path_name, filename );
    int new_path_len = strlen ( new_path_name );
    char uniq_suffix [ 4 ] = { '(', '0', ')', '\0' };

    ret = access ( new_path_name, R_OK|W_OK|F_OK );
    if ( ret == -1 && errno == ENOENT ) { 
    }
    else {
        int ix = -1;
        while ( ++ ix <= 9 ) {
            uniq_suffix [ 1 ] = '0' + ix; 
            strncpy ( new_path_name + new_path_len, uniq_suffix, 3 );
            ret = access ( new_path_name, R_OK|W_OK|F_OK );
            if ( ret == -1 && errno == ENOENT ) { 
                break;
            }
        }
        if ( ix > 9 ) {
            alarm ( LOG_ROTATE_TIME );
            return;
        }
    }

    ret = rename ( g_absolute_log_path, new_path_name );
    if ( ret == -1 ) {
        alarm ( LOG_ROTATE_TIME );
        return;
    }

    alarm ( LOG_ROTATE_TIME );

    return;

}


int log_process_signal ( ) {

    int ret;

    struct sigaction sa_alrm;
    sa_alrm.sa_handler = log_sa_alrm_handler;
    sigemptyset ( & sa_alrm.sa_mask );
    sa_alrm.sa_flags |= SA_RESTART;
    ret = sigaction ( SIGALRM, & sa_alrm, NULL );
    if ( ret == -1 ) {
        return -1;
    }

    struct sigaction sa_int;
    sa_int.sa_handler = SIG_IGN;
    sigemptyset ( & sa_int.sa_mask );
    sa_int.sa_flags = 0;
    ret = sigaction ( SIGINT, & sa_int, NULL );
    if ( ret == -1 ) {
        return -2;
    }

    struct sigaction sa_hup;
    sa_hup.sa_handler = SIG_IGN;
    sigemptyset ( & sa_hup.sa_mask );
    sa_hup.sa_flags = 0;
    ret = sigaction ( SIGHUP, & sa_hup, NULL );
    if ( ret == -1 ) {
        return -3;
    }

    struct sigaction sa_quit;
    sa_quit.sa_handler = SIG_IGN;
    sigemptyset ( & sa_quit.sa_mask );
    sa_quit.sa_flags = 0;
    ret = sigaction ( SIGQUIT, & sa_quit, NULL );
    if ( ret == -4 ) {
        return -1;
    }

    struct sigaction sa_usr1;
    sa_usr1.sa_handler = log_sa_usr1_handler;
    sigemptyset ( & sa_usr1.sa_mask );
    sa_usr1.sa_flags = 0;
    ret = sigaction ( SIGUSR1, & sa_usr1, NULL );
    if ( ret == -1 ) {
        return -5;
    }

    struct sigaction sa_usr2;
    sa_usr2.sa_handler = SIG_IGN;
    sigemptyset ( & sa_usr2.sa_mask );
    sa_usr2.sa_flags = 0;
    ret = sigaction ( SIGUSR2, & sa_usr2, NULL );
    if ( ret == -1 ) {
        return -6;
    }

    struct sigaction sa_cont;
    sa_cont.sa_handler = SIG_IGN;
    sigemptyset ( & sa_cont.sa_mask );
    sa_cont.sa_flags = 0;
    ret = sigaction ( SIGCONT, & sa_cont, NULL );
    if ( ret == -1 ) {
        return -7;
    }

    struct sigaction sa_tstp;
    sa_tstp.sa_handler = SIG_IGN;
    sigemptyset ( & sa_tstp.sa_mask );
    sa_tstp.sa_flags = 0;
    ret = sigaction ( SIGTSTP, & sa_tstp, NULL );
    if ( ret == -1 ) {
        return -8;
    }

    return 0;

}

static int produce_absolute_path ( char * relative_path, char * p_absolute_path ) {

    if ( relative_path == NULL || p_absolute_path == NULL || relative_path [ 0 ] == '\0' ) {
        return -1;
    }

    char absolute_path [ LOG_MAX_PATH_LEN * 2 + 1 ];

    if ( relative_path [ 0 ] != '/' ) {
        char * ret_getcwd = getcwd ( absolute_path, LOG_MAX_PATH_LEN );
        if ( ret_getcwd == NULL ) {
            return -2;
        }
        strcat ( absolute_path, "/" );
        strcat ( absolute_path, relative_path );
    }
    else {
        strcpy ( absolute_path, relative_path );
    }

    char path_of_level [ MAX_PATH_LEVEL ] [ MAX_LEVEL_PATH_STR_LEN ];
    int path_ix = -1;
    char * delim = "/";
    char * saveptr, * token, * str;

    bzero ( path_of_level, MAX_PATH_LEVEL*MAX_LEVEL_PATH_STR_LEN );
                        
    for ( str = absolute_path; ; str = NULL ) { 
        token = strtok_r ( str, delim, & saveptr );
        if ( token == NULL ) { 
            break;
        }   
        if ( path_ix > MAX_PATH_LEVEL - 1 ) {
            return -9;
        }
        strcpy ( path_of_level [ ++ path_ix ], token );

    }

    int filter_ix = 0;
    while ( path_of_level [ filter_ix ] [ 0 ] != '\0' ) {
        if ( strcmp ( path_of_level [ filter_ix ], ".." ) == 0 ) {
            path_of_level [ filter_ix ] [ 0 ] = '\0';
            int del_ix = filter_ix - 1;
            while ( del_ix >= 0 ) {
                if ( path_of_level [ del_ix ] [ 0 ] != '\0' ) {
                    path_of_level [ del_ix ] [ 0 ] = '\0';
                    break;
                }
                del_ix --;
            }
        }
        else if ( strcmp ( path_of_level [ filter_ix ], "." ) == 0 ) {
            path_of_level [ filter_ix ] [ 0 ] = '\0';
        }

        filter_ix ++;
    }

    char filter_absolute_path [ LOG_MAX_PATH_LEN * 2 + 1 ];
    bzero ( filter_absolute_path, sizeof ( filter_absolute_path ) );

    int ass_ix = -1;

    while ( ++ ass_ix < MAX_PATH_LEVEL - 1 ) {
        if ( path_of_level [ ass_ix ] [ 0 ] != '\0' ) {
            strcat ( filter_absolute_path, "/" );
            strcat ( filter_absolute_path, path_of_level [ ass_ix ] );
        }
    }

    strcpy ( p_absolute_path, filter_absolute_path );

    return 0;

}



void log_exit ( void ) {
    kill ( logger_pid, SIGUSR1 );
}


void log_sa_usr1_handler ( int sig_num ) {
    destroy_logger ( g_absolute_log_path );
}


/*end of file*/
