/*
 *
 * log.h
 *
 */

#ifndef _C_DEBUG_LOG_H
#define _C_DEBUG_LOG_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

/*CS_LOG_PATH*/


enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_FATAL
}; 

#ifdef _CS_DEBUG_LOG_
FILE *_CS_DEBUG_LOG_FILE_;
char  _CS_DEBUG_LOG_BUFFER_[4*1024];
char _cs_buffer_[20];
#else
FILE *_CS_DEBUG_LOG_FILE_=NULL;
char *_CS_DEBUG_LOG_BUFFER_=NULL;
char *_cs_buffer_=NULL;
#endif

#define _CS_MAX_LOG_FILE_SIZE_ (1024*1024*10)
#define _CS_DEBUG_LOG_FMT_ "[%s %s:%d %s] "
#define _CS_DEBUG_LOG_INFO_ _cs_buffer_,__FILE__,__LINE__,__func__

#define COIS_DEBUG_PRINT(fmt, ...) do { \
    if (_CS_DEBUG_LOG_) { \
        fprintf ( stderr, _CS_DEBUG_LOG_FMT_ fmt, _CS_DEBUG_LOG_INFO_, ##__VA_ARGS__); \
    } \
} while (0)

#define cs_log_start do { \
    if (_CS_DEBUG_LOG_) { \
        _CS_DEBUG_LOG_FILE_ = fopen(CS_LOG_PATH, "a+"); \
    } \
} while (0)

#define cs_log(_cs_log_level_,fmt, ...) do { \
    if (_CS_DEBUG_LOG_) { \
        if ( access ( CS_LOG_PATH, W_OK|F_OK) != 0 ) { \
            if ( _CS_DEBUG_LOG_FILE_!= NULL ) { \
                fclose(_CS_DEBUG_LOG_FILE_); \
            } \
            _CS_DEBUG_LOG_FILE_ = fopen(CS_LOG_PATH, "a+"); \
        } \
        time_t _cs_t_; struct tm _cs_tmt_; \
        time ( & _cs_t_ ); localtime_r ( & _cs_t_, & _cs_tmt_ ); \
        strftime ( _cs_buffer_, 20, "%Y-%m-%d %H:%M:%S", & _cs_tmt_ ); \
        _cs_buffer_ [ 19 ] = '\0'; \
        if (_CS_DEBUG_LOG_FILE_!=NULL) { \
            char log_level_str [ 16 ]; \
            switch ( _cs_log_level_) { \
                case LOG_DEBUG: strcpy ( log_level_str, "[DEBUG] " ); break; \
                case LOG_INFO: strcpy ( log_level_str, "[INFO] " ); break; \
                case LOG_WARN: strcpy ( log_level_str, "[WARN] " ); break; \
                case LOG_ERROR: strcpy ( log_level_str, "[ERROR] " ); break; \
                case LOG_FATAL: strcpy ( log_level_str, "[FATAL] " ); break; \
            } \
            snprintf(_CS_DEBUG_LOG_BUFFER_, 4*1024-1, _CS_DEBUG_LOG_FMT_ "%s" fmt "\n", _CS_DEBUG_LOG_INFO_, log_level_str, ##__VA_ARGS__); \
            fwrite(_CS_DEBUG_LOG_BUFFER_, 1, strlen(_CS_DEBUG_LOG_BUFFER_), _CS_DEBUG_LOG_FILE_ ); \
            fflush(_CS_DEBUG_LOG_FILE_); \
        } \
        struct stat lf_stat; \
        if ( 0 == stat ( CS_LOG_PATH, & lf_stat ) ) { \
            if ( lf_stat.st_size > _CS_MAX_LOG_FILE_SIZE_ ) { \
                if ( _CS_DEBUG_LOG_FILE_!= NULL ) { \
                    fclose(_CS_DEBUG_LOG_FILE_); \
                    char * new_log_path = ( char * ) malloc ( strlen (CS_LOG_PATH) + 128 ); \
                    if ( new_log_path != NULL ) { \
                        _cs_buffer_ [ 10 ] = _cs_buffer_ [ 13 ] = _cs_buffer_ [ 16 ] = '_'; \
                        sprintf ( new_log_path, "%s.%d.%s", CS_LOG_PATH, getpid (), _cs_buffer_ ); \
                        char * nx = new_log_path + strlen ( new_log_path ); \
                        int n = 0; \
                        while ( 1 ) { \
                            int nt = access ( new_log_path, F_OK ); \
                            if ( nt == -1 && errno == ENOENT ) { \
                                break; \
                            } \
                            else { \
                                sprintf ( nx, ".%d", ++ n ); \
                            } \
                        } \
                        rename ( CS_LOG_PATH, new_log_path ); \
                        free ( new_log_path ); \
                    } \
                    _CS_DEBUG_LOG_FILE_ = fopen(CS_LOG_PATH, "a+"); \
                } \
            } \
        } \
    } \
} while (0)

#define cs_log_end do { \
    if (_CS_DEBUG_LOG_) { \
        if (_CS_DEBUG_LOG_FILE_!=NULL) { \
            fclose(_CS_DEBUG_LOG_FILE_); \
        } \
    } \
} while (0)





#endif


/*end of file*/

