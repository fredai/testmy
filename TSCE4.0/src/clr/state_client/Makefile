AR=ar -rcs
CC = gcc


COMPILE = $(CC) -g $(INC_PATH) -c  			
SRCS = $(wildcard ./*.c) 
OBJS = $(patsubst %.c,%.o,$(SRCS)) 


TARGET = libstate_client.a

all:$(TARGET)
$(TARGET): ${OBJS}
	        ${AR} $(TARGET) $(OBJS)

%.o:%.c
	        ${COMPILE} ${LIBS} $< -o $@ 

.PHONY: all clean

clean:
	        rm -f *.o $(TARGET)


