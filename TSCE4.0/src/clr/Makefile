
CFLAGS = -g -Wall -O0 -D_REENTRANT
CFLAGS += -D_CS_DEBUG_LOG_=1
LDFLAGS = -g -Wall -O0

CC = gcc

ifeq ($(shell uname -m),x86_64)
    LIB_PATH = -L../../lib/lib64
else
    LIB_PATH = -L../../lib/lib32
endif

LIBS = $(LIB_PATH) -lutil -lm -lpthread
LIBS += -L./state_client/ -lstate_client

INCLUDES = -I../_con/ -I../../lib/include/ -I./state_client 


HDRS    =	clr.h			\
		clr_config.h		\
		protocol.h		\
		data.h \
		framework.h \
						

OBJS    =       clr.o   		\
		clr_config.o		\
		../_con/con_config.o	\
		../_con/error_handle.o \
		../_con/common_util.o \
		data.o \
		framework.o \


SRCS	=       clr.c   		\
		clr_config.c		\
		data.c \
		framework.c


TARGET = tscecd tscecd_start tscecd_stop

all:$(TARGET)

tscecd : ${OBJS}
			cd state_client;make clean; make;
	        ${CC} ${LDFLAGS}  ${OBJS} -ldl -rdynamic ${LIBS}  -o $@

.c.o:
	        ${CC} ${CFLAGS} ${INCLUDES} -c $<

../_con/con_config.o:../_con/con_config.c
	cd ../_con;make
../_con/error_handle.o:../_con/error_handle.c
	cd ../_con;make
../_con/common_util.o:../_con/common_util.c
	cd ../_con;make


tscecd_start : tscecd_start.c
	        ${CC} ${CFLAGS} ${INCLUDES} $< ../_con/error_handle.o \
				-o $@ $(LIB_PATH) -lutil

tscecd_stop : tscecd_stop.c
	        ${CC} ${CFLAGS} ${INCLUDES} $< ../_con/error_handle.o \
				-o $@ $(LIB_PATH) -lutil


.PHONY: all clean

clean:
	        rm -f *.o ../_con/con_config.o \
				../_con/error_handle.o \
			   ../_con/common_util.o core.* *~ $(TARGET)
			rm -f ../../release/bin/clr/*


