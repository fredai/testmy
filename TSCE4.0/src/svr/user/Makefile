
CFLAGS = -g -Wall -O0 -D_REENTRANT
LDFLAGS = -g -Wall -O0

CC = gcc

ifeq ($(shell uname -m),x86_64)
    LIB_PATH = -L../../../lib/lib64
    LIB_PATH += -L/usr/lib64/mysql
else
    LIB_PATH = -L../../../lib/lib32
    LIB_PATH += -L/usr/lib/mysql
endif

LIBS = $(LIB_PATH) -lutil -llog -lpthread
LIBS += -lmysqlclient

INCLUDES = -I../../_con/ -I../../../lib/include/
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
		../../_con/con_config.o	\
		../../_con/error_handle.o \
		../../_con/common_util.o \
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



TARGET = teyesd_start teyesd_stop teyesd_status

all:$(TARGET)

tscesd: ${OBJS}
	        ${CC} ${LDFLAGS} ${OBJS} ${LIBS} -o $@

.c.o:
	        ${CC} ${CFLAGS} ${INCLUDES} -c $<

../../_con/con_config.o:../../_con/con_config.c
	cd ../_con;make
../../_con/error_handle.o:../../_con/error_handle.c
	cd ../_con;make
../../_con/common_util.o:../../_con/common_util.c
	cd ../_con;make


#teyesd_start: teyesd_start.c
#	        ${CC} ${CFLAGS} ${INCLUDES} ../../_con/error_handle.o $< -o $@  $(LIB_PATH) -lutil
teyesd_status: teyesd_status.c
	        ${CC} ${CFLAGS} ${INCLUDES} $< -o $@ $(LIB_PATH) -lutil
teyesd_stop: teyesd_stop.c
	        ${CC} ${CFLAGS} ${INCLUDES} ../../_con/error_handle.o $< -o $@ $(LIB_PATH) -lutil
teyesd_start: teyesd_start.c
	        ${CC} ${CFLAGS} ${INCLUDES} $< -o $@ $(LIB_PATH) -lutil

.PHONY: all clean

clean:
	        rm -f *.o ../../_con/con_config.o \
				../../_con/error_handle.o \
			   ../../_con/common_util.o core.* *~ $(TARGET)


