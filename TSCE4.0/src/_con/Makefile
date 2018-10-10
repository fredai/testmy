
CFLAGS = -g -Wall -O0 -D_REENTRANT
LDFLAGS = -g -Wall -O0

CC = gcc

#LIBS = -L../../lib/lib64/ -lutil

INCLUDES = -I../_con/ -I../../lib/include/ -I../svr/


HDRS    =	con_config.h \
		protocol.h \
		error_handle.h \
		common_util.h
						


OBJS    =       con_config.o \
		protocol.o \
		error_handle.o \
		common_util.o



SRCS	=       con_config.c \
		protocol.c \
		error_handle.c \
		common_util.c



all:$(OBJS)

.c.o:
	        ${CC} ${CFLAGS} ${INCLUDES} -c $<




.PHONY: all clean

clean:
	        rm -f *.o ../con/con_config.o core.* *~ $(TARGET)


