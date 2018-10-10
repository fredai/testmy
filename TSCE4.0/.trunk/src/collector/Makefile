
CFLAGS = -g -Wall -D_REENTRANT
CFLAGS += -D_CS_DEBUG_LOG_=1
LDFLAGS = -g -Wall
CC = gcc

ifeq ($(shell uname -m),x86_64)
PCRE_PATH = -L ../../libs/lib64
else
PCRE_PATH = -L ../../libs/lib32
endif

LIBS =  -L ../util -lutils $(PCRE_PATH) -lpcre

INCLUDES = -I ../util/ -I ../common -I ../../libs/include

HDRS	= c_config.h	\
	  c_collector.h	\
	  c_engine.h	\
	  c_script.h	\
	  c_proc_sct.h	\
	  c_proc_cmd.h	\
	  c_debug_log.h	\
	  c_signal.h

OBJS	= c_config.o	\
	  c_collector.o	\
	  c_engine.o	\
	  c_script.o	\
	  c_proc_sct.o	\
	  c_proc_cmd.o	\
	  c_signal.o

SRCS	= c_config.c	\
	  c_collector.c	\
	  c_engine.c	\
	  c_script.c	\
	  c_proc_sct.c	\
	  c_proc_cmd.c	\
	  c_signal.c

TARGET = collector

all:$(TARGET) 

$(TARGET): ${OBJS}
	${CC} ${LDFLAGS} ${OBJS} ${LIBS} -o $@

.c.o:
	${CC} ${CFLAGS} ${INCLUDES} -c $<




.PHONY: all clean

clean:
	rm -f *.o core.* *~ $(TARGET)


