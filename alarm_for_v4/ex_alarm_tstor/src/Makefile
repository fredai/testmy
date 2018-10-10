
CFLAGS = -g -Wall -O0 -D_REENTRANT
CFLAGS += -D_CS_DEBUG_LOG_=1 
LDFLAGS = -g -Wall -O0 

CC = gcc


ifeq ($(shell uname -m),x86_64)
LIB_PATH = ../lib/lib64
MYSQL_PATH = ../lib/lib64/mysql
else
MYSQL_PATH = ../lib/lib32/mysql
LIB_PATH = ../lib/lib32
endif

LIBS = -lpthread -L $(LIB_PATH) -L $(MYSQL_PATH) -lcengine -lmysqlclient -lemail -lcrypto -lm -lssl#-lc /usr/lib64/libcrypto.so.10 #-lcrypto

INCLUDES = -I ../lib/include/mysql -I ../lib/include



HDRS    =               		\
		ex_alarm.h		\
		config.h		\
		utils.h			\
		alarm.h			\
		db.h			
						

OBJS    =               		\
		ex_alarm.o		\
		config.o		\
		utils.o			\
		alarm.o			\
		db.o			


SRCS	=               		\
		ex_alarm.c		\
		config.c		\
		utils.c			\
		alarm.c			\
		db.c			



TARGET = alarmd ex_alarm_stop 

all:$(TARGET)

alarmd: ${OBJS}
	rm $(MYSQL_PATH)/libmysqlclient.so -f
	ln -s /usr/lib64/mysql/libmysqlclient.so $(MYSQL_PATH)/libmysqlclient.so
	${CC} ${LDFLAGS} ${OBJS} ${LIBS} -o $@

.c.o:
	${CC} ${CFLAGS} ${INCLUDES} -c $<

ex_alarm_stop: ex_alarm_stop.c 
	${CC} ${LDFLAGS} ${INCLUDES} utils.o $< -o $@



.PHONY: all clean

clean:
	        rm -f *.o core.* *~ $(TARGET)

release: $(TARGET)
	cp ./$(TARGET) ../release/ex-alarm/bin


