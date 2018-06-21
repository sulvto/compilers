TARGET = dbg.o
cc = gcc
CFLAGS = -c -g -DDBG_NO_DEBUG -Wall -std=c99
OBJS = debug.o

$(TARGET):$(OBJS)
	ld -r -o $@ $(OBJS)

clean:
	rm -f *.o
.c.o:
	$(CC) $(CFLAGS) -I../include $*.c
debug.o: debug.c ../include/MEM.h debug.h ../include/DBG.h