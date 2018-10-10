
CFLAGS = -g -Wall -O0 -D_REENTRANT
CFLAGS += -D_CS_DEBUG_LOG_=1
LDFLAGS = -g -Wall -O0

CC = gcc

ifeq ($(shell uname -m),x86_64)
    LIB_PATH = -L../../../lib/lib64
else
    LIB_PATH = -L../../../lib/lib32
endif

LIBS = $(LIB_PATH) -lutil -lm

INCLUDES = -I../../_con/ -I../../../lib/include/ -I../../clr/ 


HDRS    =	getdata.h			\
		../../clr/clr_config.h		\
		../../clr/protocol.h		\
		data.h \
		framework.h \
						

OBJS    =       getdata.o   		\
		../../clr/clr_config.o		\
		../../_con/con_config.o	\
		../../_con/error_handle.o \
		../../_con/common_util.o \
		data.o \
		framework.o \


SRCS	=       getdata.c   		\
		../../clr/clr_config.c		\
		data.c \
		framework.c


TARGET = getdata

all:$(TARGET)

getdata : ${OBJS}
	        ${CC} ${LDFLAGS}  ${OBJS} -ldl -rdynamic ${LIBS}  -o $@

.c.o:
	        ${CC} ${CFLAGS} ${INCLUDES} -c $<

../../_con/con_config.o:../../_con/con_config.c
	cd ../../_con;make
../../_con/error_handle.o:../../_con/error_handle.c
	cd ../../_con;make
../../_con/common_util.o:../../_con/common_util.c
	cd ../../_con;make



.PHONY: all clean

clean:
	        rm -f *.o ../../_con/con_config.o \
				../../_con/error_handle.o \
			   ../../_con/common_util.o core.* *~ $(TARGET)
			rm -f ../../../release/bin/manager/debug/*


