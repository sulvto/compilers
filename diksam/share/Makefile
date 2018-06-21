TARGET = share.o
cc = gcc
CFLAGS = -c -g -DDEBUG -Wall -std=c99
OBJS = disassemble.o dispose.o opcode.o util.o wchar.o

$(TARGET):$(OBJS)
	ld -r -o $@ $(OBJS)

clean:
	rm -f *.o *~
.c.o:
	$(CC) $(CFLAGS) -I../include $*.c
disassemble.o: disassemble.c ../include/DBG.h \
				../include/share.h ../include/DVM_code.h \
				../include/DVM.h
dispose.o: dispose.c ../include/DBG.h ../include/share.h \
				../include/DVM_code.h ../include/DVM.h
opcode.o: opcode.c ../include/DBG.h ../include/share.h \
		../include/DVM_code.h ../include/DVM.h ../include/MEM.h
util.o: util.c ../include/DBG.h ../include/share.h \
		../include/DVM_code.h ../include/DVM.h ../include/MEM.h
wchar.o: wchar.c ../include/DBG.h ../include/share.h \
		../include/DVM_code.h ../include/DVM.h ../include/MEM.h