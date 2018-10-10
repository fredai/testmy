
CFLAGS = -g -Wall -I ../../libs/include
LDFLAGS = -g
CC = gcc
AR = ar
ARFLAGS = -rcs

LIBS =

INCLUDES =

HDRS	=		\
	u_util.h	\
	u_hash.h	\
	u_log.h		\
	u_queue.h	\
	u_mutex.h	\
	u_regex.h	\
	u_shm.h

OBJS	=		\
	u_util.o	\
	u_hash.o	\
	u_log.o		\
	u_queue.o	\
	u_mutex.o	\
	u_regex.o	\
	u_shm.o

SRCS	=		\
	u_util.c	\
	u_hash.c	\
	u_log.c		\
	u_queue.c	\
	u_mutex.c	\
	u_regex.c	\
	u_shm.c


TARGET = libutils.a

all:$(TARGET) 

$(TARGET): ${OBJS}
	${AR} $(ARFLAGS) $@ $(OBJS)

.c.o:
	${CC} ${CFLAGS} ${INCLUDES} -c $<




.PHONY: all clean

clean:
	rm -f *.o core.* *~ $(TARGET)


