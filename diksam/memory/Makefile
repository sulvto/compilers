TARGET = mem.o
cc = gcc
CFLAGS = -c -g -DDEBUG -Wall -std=c99
OBJS = memory.o storage.o

$(TARGET):$(OBJS)
	ld -r -o $@ $(OBJS)
.c.o:
	$(CC) $(CFLAGS) -I../include $*.c
memory.o: memory.c memory.h ../include/MEM.h
storage.o: storage.c memory.h ../include/MEM.h