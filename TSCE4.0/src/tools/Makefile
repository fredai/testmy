
CFLAGS = -g -Wall -O0 -D_REENTRANT
LDFLAGS = -g -Wall -O0

CC = gcc

ifeq ($(shell uname -m),x86_64)
    LIB_PATH = -L../../lib/lib64/
else
    LIB_PATH = -L../../lib/lib32/
endif

LIBS = $(LIB_PATH) -lutil

INCLUDES = -I../../lib/include/  -I../svr/ -I../_con/


TARGET = tscecd_all_start tscecd_all_stop tsce_start tsce_stop

all:$(TARGET)

tscecd_all_start: tscecd_all_start.c
	make -C ../_con; make -C ../svr; ${CC} ${CFLAGS} ${INCLUDES} $< \
		../_con/con_config.o ../svr/svr_config.o ../_con/error_handle.o -o \
		$@ $(LIB_PATH) -lutil

tscecd_all_stop: tscecd_all_stop.c
	make -C ../_con; make -C ../svr; ${CC} ${CFLAGS} ${INCLUDES} $< \
		../_con/con_config.o ../svr/svr_config.o ../_con/error_handle.o -o \
		$@ $(LIB_PATH) -lutil


tsce_start: tsce_start.c
	make -C ../_con; ${CC} ${CFLAGS} ${INCLUDES} $< ../_con/error_handle.o -o $@ 


tsce_stop: tsce_stop.c
	make -C ../_con; ${CC} ${CFLAGS} ${INCLUDES} $< ../_con/error_handle.o -o $@ 


.PHONY: all clean

clean:
	        rm -f *.o ../_con/con_config.o core.* *~ $(TARGET)
			rm -f ../../release/bin/tools/*


