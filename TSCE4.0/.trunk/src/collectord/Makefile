
CFLAGS = -g -Wall -O0 -D_REENTRANT
LDFLAGS = -g -Wall -O0
CC = gcc

ifeq ($(shell uname -m),x86_64)
MYSQL_PATH = -L ../../libs/lib64/mysql
else
MYSQL_PATH = -L ../../libs/lib32/mysql
endif


LIBS =  -lpthread -L ../util -lutils $(MYSQL_PATH) -lmysqlclient

INCLUDES = -I ../util/ -I ../common -I ../../libs/include/mysql

HDRS	=		\
	d_collectord.h	\
	d_nodes.h	\
	d_datas.h	\
	d_service.h	\
	d_recv.h	\
	d_signal.h	\
	d_db.h		\
	d_config.h

OBJS	=		\
	d_collectord.o	\
	d_nodes.o	\
	d_datas.o	\
	d_service.o	\
	d_recv.o	\
	d_signal.o	\
	d_db.o		\
	d_config.o

SRCS	=		\
	d_collectord.c	\
	d_nodes.c	\
	d_datas.c	\
	d_service.c	\
	d_recv.c	\
	d_signal.c	\
	d_db.c		\
	d_config.c

TARGET = collectord

all:$(TARGET) 

$(TARGET): ${OBJS}
	${CC} ${LDFLAGS} ${OBJS} ${LIBS} -o $@

.c.o:
	${CC} ${CFLAGS} ${INCLUDES} -c $<




.PHONY: all clean

clean:
	rm -f *.o core.* *~ $(TARGET)


