
CFLAGS = -g -Wall -O0 -D_REENTRANT
LDFLAGS = -g -Wall -O0

CC = gcc

ifeq ($(shell uname -m),x86_64)
    LIB_PATH = -L../../lib/lib64
    LIB_PATH += -L/usr/lib64/mysql
else
    LIB_PATH = -L../../lib/lib32
    LIB_PATH += -L/usr/lib/mysql
endif

LIBS = $(LIB_PATH) -lutil -llog -lpthread
LIBS += -lmysqlclient -L./state_server/ -lstate_server

INCLUDES = -I../_con/ -I../../lib/include/ -I./state_server/
INCLUDES += -I/usr/include/mysql


HDRS    =	svr.h			\
		svr_config.h		\
		service.h		\
		recv.h			\
		rtd.h			\
		buffer.h		\
		db.h			
						

OBJS    =       svr.o   		\
		svr_config.o		\
		../_con/con_config.o	\
		../_con/error_handle.o \
		../_con/common_util.o \
		service.o		\
		recv.o			\
		rtd.o			\
		buffer.o		\
		db.o			



SRCS	=       svr.c   		\
		svr_config.c		\
		service.c		\
		recv.c			\
		rtd.c			\
		buffer.c		\
		db.c			



TARGET = tscesd tscesd_start tscesd_status tscesd_stop 

all:$(TARGET)

tscesd: ${OBJS}
			cd state_server;make clean; make;
	        ${CC} ${LDFLAGS} ${OBJS} ${LIBS} -o $@

.c.o:
	        ${CC} ${CFLAGS} ${INCLUDES} -c $<

../_con/con_config.o:../_con/con_config.c
	cd ../_con;make
../_con/error_handle.o:../_con/error_handle.c
	cd ../_con;make
../_con/common_util.o:../_con/common_util.c
	cd ../_con;make

tscesd_start: tscesd_start.c
	        ${CC} ${CFLAGS} ${INCLUDES} ../_con/error_handle.o $< -o $@  $(LIB_PATH) -lutil
tscesd_status: tscesd_status.c
	        ${CC} ${CFLAGS} ${INCLUDES} $< -o $@ $(LIB_PATH) -lutil
tscesd_stop: tscesd_stop.c
	        ${CC} ${CFLAGS} ${INCLUDES} ../_con/error_handle.o $< -o $@ $(LIB_PATH) -lutil
#teyesd_start: teyesd_start.c
#	        ${CC} ${CFLAGS} ${INCLUDES} $< -o $@ $(LIB_PATH) -lutil

.PHONY: all clean

clean:
	        rm -f *.o ../_con/con_config.o \
				../_con/error_handle.o \
			   ../_con/common_util.o core.* *~ $(TARGET)
			rm -f ../../release/bin/svr/tscesd \
				../../release/bin/svr/tscesd_start \
				../../release/bin/svr/tscesd_stop \
				../../release/bin/svr/tscesd_status


