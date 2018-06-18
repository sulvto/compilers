TARGET = compiler.o
CC = gcc
OBJS = lex.yy.o y.tab.o interface.o create.o \
        fix_tree.o generate.o string.o wchar.o \
        util.o error.o error_message.o

CFLAGS  = -c -g -Wall -Wswitch-enum -ansi -pedantic -DDEBUG -DSHIFT_JIS_SOURCE -std=c99

INCLUDES = -I../include

$(TARGET):$(OBJS)
	ld -r -o $@ $(OBJS)

clean:
	rm -f *.o lex.yy.c y.tab.c t.tag.h *~
y.tab.h : diksam.y
	yacc -dv diksam.y
y.tab.c : diksam.y
	yacc -dv diksam.y
lex.yy.c : diksam.l diksam.y y.tab.h
	lex diksam.l
y.tab.o : y.tab.c diksamc.h ../include/MEM.h
	$(CC) -c -g $*.c $(INCLUDES)
lex.yy.o : lex.yy.c diksamc.h ../include/MEM.h
	$(CC) -c -g $*.c $(INCLUDES)
.c.o:
	$(CC) $(CFLAGS) $*.c $(INCLUDES)


create.o: create.c ../include/MEM.h ../include/DBG.h diksamc.h\
    ../include/DKC.h ../include/DVM_code.h ../include/DVM.h\
    ../include/share.h ../include/DVM_dev.h


diksam.tab.o: diksam.tab.c ../include/MEM.h ../include/DBG.h diksamc.h\
                  ../include/DKC.h ../include/DVM_code.h \
                  ../include/DVM.h ../include/share.h

error.o: error.c ../include/MEM.h ../include/DBG.h diksamc.h\
                ../include/DKC.h ../include/DVM_code.h ../include/DVM.h\
                ../include/share.h
error_message.o: error_message.c ../include/MEM.h ../include/DBG.h diksamc.h\
                        ../include/DKC.h ../include/DVM_code.h ../include/DVM.h\
                        ../include/share.h
fix_tree.o: fix_tree.c ../include/MEM.h ../include/DBG.h diksamc.h\
                    ../include/DKC.h ../include/DVM_code.h ../include/DVM.h\
                    ../include/share.h
generate.o: generate.c ../include/MEM.h ../include/DBG.h diksamc.h\
                  ../include/DKC.h ../include/DVM_code.h ../include/DVM.h\
                  ../include/share.h
interface.o: interface.c ../include/MEM.h ../include/DBG.h diksamc.h\
                   ../include/DKC.h ../include/DVM_code.h ../include/DVM.h\
                   ../include/share.h
string.o: string.c ../include/MEM.h ../include/DBG.h diksamc.h\
               ../include/DKC.h ../include/DVM_code.h ../include/DVM.h\
               ../include/share.h
util.o: util.c ../include/MEM.h ../include/DBG.h diksamc.h\
              ../include/DKC.h ../include/DVM_code.h ../include/DVM.h\
              ../include/share.h
wchar.o: wchar.c ../include/MEM.h ../include/DBG.h diksamc.h\
            ../include/DKC.h ../include/DVM_code.h ../include/DVM.h\
            ../include/share.h