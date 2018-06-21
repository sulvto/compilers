TARGET = diksam
cc = gcc
CFLAGS = -c -g -DDEBUG -Wall -ansi -Wswitch-enum -std=c99
OBJS = ../compiler/compiler.o ../dvm/dvm.o \
        ../memory/mem.o ../share/share.o \
        ../debug/dbg.o ./main.o

CFLAGS = -c -g -DDEBUG -Wall -ansi -Wswitch-enum -std=c99

INCLUDES = -I/usr/local/include -I../include

$(TARGET):$(OBJS)
	cd ../compiler; $(MAKE)
	cd ../dvm; $(MAKE);
	cd ../share; $(MAKE);
	cd ../memory; $(MAKE);
	cd ../debug; $(MAKE);
	$(CC) $(OBJS) -o $@ -lm

clean:
	cd ../compiler; make clean;
	cd ../dvm; make clean;
	cd ../share; make clean;
	cd ../memory; make clean;
	cd ../debug; make clean;
	rm -f *.o *~
.c.o:
	$(CC) $(CFLAGS) $*.c $(INCLUDES)
../memory/mem.o:
	cd ../memory;$(MAKE);
../debug/dbg.o:
	cd ../debug;$(MAKE);
../share/share.o:
	cd ../share;$(MAKE);
../compiler/compiler.o:
	cd ../compiler;$(MAKE);
../dvm/dvm.o:
	cd ../dvm;$(MAKE);

main.o: main.c ../include/DKC.h ../include/DVM.h \
                ../include/MEM.h ../include/DVM_code.h