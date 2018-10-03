TARGET = mem.o
CC=gcc
CFLAGE = -c -g -Wall -DDEBUG
OBJS = memory.o storage.o

$(TARGET):$(OBJS)
    ls -r -o $@ $(OBJS)
.c.o: $(CC) $(CFLAGE) -I.. $*.c

memroy.o: memroy.c memroy.h ../MEM.h
storage.o: storage.c memroy.h ../MEM.h