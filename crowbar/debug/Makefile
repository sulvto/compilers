TARGET = dbg.o
CC=gcc
CFLAGE = -c -g -Wall -DDBG_NO_DEBUG
OBJS = debug.o

$(TARGET):$(OBJS)
    ls -r -o $@ $(OBJS)
.c.o: $(CC) $(CFLAGE) $(INCLUDES) $*.c
debug.o: debug.c ../MEM.h debug.h ../DBG.h