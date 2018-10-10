
CFLAGS = -g -Wall -D_REENTRANT
LDFLAGS = -g -Wall
CC = gcc

LIBS =  -lpthread -L ../../util -lutils

INCLUDES = -I .. -I ../../util/ -I ../../common

HDRS	= t_check_status.h	\
	  t_get_data.h		\
	  t_send_cmd.h		\
	  t_sort.h		\
	  t_format_output.h

OBJS	= t_check_status.o	\
	  t_get_data.o		\
	  t_send_cmd.o		\
	  t_sort.o		\
	  t_format_output.o

SRCS	= t_check_status.c	\
	  t_get_data.c		\
	  t_send_cmd.c		\
	  t_sort.c		\
	  t_format_output.c

TARGET = checkstatus getdata sendcmd

all:$(TARGET) 

checkstatus: t_check_status.o t_process_opt.o
	${CC} ${LDFLAGS} t_check_status.o t_process_opt.o ../d_nodes.o ${LIBS} -o $@

getdata: t_get_data.o t_process_opt.o
	${CC} ${LDFLAGS} t_get_data.o t_process_opt.o ../d_nodes.o ${LIBS} -o $@

sendcmd: t_send_cmd.o
	${CC} ${LDFLAGS} t_send_cmd.o t_process_opt.o ../d_nodes.o ${LIBS} -o $@ -lpthread

t_process_opt.o: t_process_opt.c
	${CC} ${CFLAGS} ${INCLUDES} -c $<

.c.o:
	${CC} ${CFLAGS} ${INCLUDES} -c $<




.PHONY: all clean

clean:
	rm -f *.o core.* *~ $(TARGET)


