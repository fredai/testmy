
CFLAGS += -D_CS_DEBUG_LOG_=1
LDFLAGS = -g -Wall -O0

CC=gcc

ifeq ($(shell uname -m),x86_64)
LIB_PATH = ../lib/lib64
else
LIB_PATH = ../lib/lib32
endif

LIBS= -L$(LIB_PATH) -lemail -lcrypto -lm -lssl
INCLUDES= -I ../lib/include

HDRS =		\
	config.h\
	utils.h
OBJS = 		\
	test_email.o\
	config.o\
	utils.o
SRCS = 		\
	test_email.c \
	config.c \
	utils.c

TARGET = test_email

all:$(TARGET)
$(TARGET):${OBJS}
	${CC} ${LDFLAGS} ${OBJS} ${LIBS} -o $@
	cp $(TARGET) ../release/ex-alarm/bin/
.c.o:
	${CC} ${CFLAGS} ${INCLUDES} -c $<

.PHONY:clean
clean:
	rm -f *.o core.* *~ $(TARGET) 
release:
	cp ./$(TARGET) ../release/ex-alarm/bin	
