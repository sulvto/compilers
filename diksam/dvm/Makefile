TARGET = dvm.o
CC = gcc
OBJS = load.o heap.o execute.o native.o nativeif.o util.o wchar.o error.o error_message.o

CFLAGS  = -c -g -Wall -Wswitch-enum -ansi -pedantic -DDEBUG -DSHIFT_JIS_SOURCE -std=c99

INCLUDES = -I../include

$(TARGET):$(OBJS)
	ld -r -o $@ $(OBJS)

clean:
	rm -f *.o
.c.o:
	$(CC) $(CFLAGS) $*.c $(INCLUDES)


load.o: load.c ../include/MEM.h ../include/DBG.h dvm_pri.h\
			../include/DKC.h ../include/DVM_code.h ../include/DVM.h\
			../include/share.h ../include/DVM_dev.h
execute.o: execute.c ../include/MEM.h ../include/DBG.h dvm_pri.h\
			../include/DKC.h ../include/DVM_code.h \
			../include/DVM.h ../include/share.h
heap.o: heap.c ../include/MEM.h ../include/DBG.h dvm_pri.h\
 			../include/DKC.h ../include/DVM_code.h ../include/DVM.h\
 			../include/share.h
native.o: native.c ../include/MEM.h ../include/DBG.h dvm_pri.h\
			../include/DKC.h ../include/DVM_code.h ../include/DVM.h\
			../include/share.h
nativeif.o: nativeif.c ../include/MEM.h ../include/DBG.h dvm_pri.h\
			../include/DKC.h ../include/DVM_code.h ../include/DVM.h\
			../include/share.h
util.o: util.c ../include/MEM.h ../include/DBG.h dvm_pri.h\
			../include/DKC.h ../include/DVM_code.h ../include/DVM.h\
			../include/share.h
wchar.o: wchar.c ../include/MEM.h ../include/DBG.h dvm_pri.h\
       			../include/DKC.h ../include/DVM_code.h ../include/DVM.h\
       			../include/share.h
error.o: error.c ../include/MEM.h ../include/DBG.h dvm_pri.h\
			../include/DKC.h ../include/DVM_code.h ../include/DVM.h\
			../include/share.h
error_message.o: error_message.c ../include/MEM.h ../include/DBG.h dvm_pri.h\
			../include/DKC.h ../include/DVM_code.h ../include/DVM.h\
			../include/share.h