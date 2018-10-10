AR=ar -rcs
CC = gcc

INC_PATH = -I ../../../lib/include/ -I ../../../lib/include/util -I ../


COMPILE = $(CC) -g $(INC_PATH) -c  			
SRCS = $(wildcard ./*.c) 
OBJS = $(patsubst %.c,%.o,$(SRCS)) 


TARGET = libstate_server.a

all:$(TARGET)
$(TARGET): ${OBJS}
	        ${AR} $(TARGET) $(OBJS)

%.o:%.c
	        ${COMPILE} ${LIBS} $< -o $@ 

.PHONY: all clean

clean:
	        rm -f *.o $(TARGET)


