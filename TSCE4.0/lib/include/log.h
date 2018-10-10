/*
 *
 * log.h
 *
 */

#ifndef _LOG_H
#define _LOG_H

#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <pthread.h>

typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_FATAL
} log_level_t;

typedef int log_id_t;

typedef struct _log_s log_t;

struct _log_s {
    log_level_t log_level;
    log_id_t log_id;
};


int init_logger ( char * log_file_path );
int destroy_logger ( char * log_file_path );
int start_log ( char * log_file_path, log_level_t log_level, log_t * handle );
int end_log ( log_t * log_handle );
int u_log ( log_t * log_handle, log_level_t level, const char * fmt, ... );
int set_log_level ( log_t * log_handle, log_level_t log_level );



#define _U_DEBUG_FMT_ "[%s:%d %s] "
#define _U_DEBUG_INFO_ __FILE__,__LINE__,__FUNCTION__

#define _U_THREAD_FMT_ "[%s:%u] "

#define LOG_START(log_path,log_level) \
    int _u_log_ret_ = -1; \
    log_t _u_log_handle_; \
    do { \
        _u_log_ret_ = start_log ( log_path, log_level, & _u_log_handle_ ); \
    }while (0)

#define LOG_END \
    do { \
        if ( _u_log_ret_ != -1 ) { \
            end_log ( & _u_log_handle_ ); \
        } \
    }while(0)

#define LOG(loglevel,fmt,args...) \
    do { \
        if ( _u_log_ret_ != -1 ) { \
            if ( _u_log_handle_.log_level == LOG_DEBUG ) { \
                u_log ( & _u_log_handle_, loglevel, _U_DEBUG_FMT_ fmt, _U_DEBUG_INFO_, args ); \
            } \
            else { \
                u_log ( & _u_log_handle_, loglevel, fmt, args ); \
            } \
        } \
    }while(0)

#define LOGT(log_level,thread_name,fmt,args...) LOG(log_level, _U_THREAD_FMT_ fmt,thread_name,pthread_self(),args)

#define LOG_SETLEVEL(log_level) \
    do { \
        if ( _u_log_ret_ != -1 ) { \
            set_log_level ( & _u_log_handle_, log_level ); \
        } \
    }while(0)








#endif /*_LOG_H */


/*end of file*/
