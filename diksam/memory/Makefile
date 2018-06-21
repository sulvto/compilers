TARGET = mem.o
cc = gcc
CFLAGS = -c -g -DDEBUG -Wall -std=c99
OBJS = memory.o storage.o

$(TARGET):$(OBJS)
	ld -r -o $@ $(OBJS)

testp : $(OBJS) main.o
	$(CC) -o $@ $(OBJS) main.o
clean:
	rm -f *.o *~
.c.o:
	$(CC) $(CFLAGS) -I../include $*.c
main.o: main.c memory.h ../include/MEM.h
memory.o: memory.c memory.h ../include/MEM.h
storage.o: storage.c memory.h ../include/MEM.h