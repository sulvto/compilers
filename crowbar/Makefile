TARGET = crowbar
CC=gcc
CFLAGE = -c -g -Wall -ansi -pedantic -DDEBUG
OBJS = lex.yy.o y.tab.o \
    main.o  \
    interface.o \
    create.o    \
    execute.o   \
    eval.o      \
    string.o    \
    heap.o      \
    util.o  \
    naive.o \
    error.o \
    error_message.o \
    ./memroy/mem.o  \
    ./debug/dgb.o   \
INCLUDES = \



$(TARGET):$(OBJS)
    cd ./memroy; $(MAKE);
    cd ./debug; $(MAKE);
    $(CC) $(OBJS) -o $@ -lm
clean: rm -f *.o lex.yy.c y.tab.c y.tab.h *~
y.tab.h: crowbar.y
    bison --yacc -dv crowbar.y
y.tab.c: crowbar.y
    bison --yacc -dv crowbar.y
lex.yy.c: crowbar.l crowbar.y y.tab.h
    flex crowbar.l
y.tab.o: y.tab.c crowber.h MEM.h
    $(CC) -c -g $*.c $(INCLUDES)
lex.yy.o: lex.yy.c crowber.h MEM.h 
    $(CC) -c -g $*.c $(INCLUDES)
.c.o: $(CC) $(CFLAGE) $*.c $(INCLUDES)
./memroy/mem.o:
    cd ./memroy; $(MAKE);
./debug/dbg.o:
    cd ./debug; $(MAKE);

create.o: create.c MEM.h DBG.h crowbar.h CRB.h CRB_dev.h
error.o: error.c MEM.h crowbar.h CRB.h CRB_dev.h
error_message.o: error_message.c MEM.h DBG.h crowbar.h CRB.h CRB_dev.h
eval.o: eval.c MEM.h DBG.h crowbar.h CRB.h CRB_dev.h
execute.o: execute.c MEM.h DBG.h crowbar.h CRB.h CRB_dev.h
heap.o: heap.c MEM.h DBG.h crowbar.h CRB.h CRB_dev.h
interface.o: interface.c MEM.h DBG.h crowbar.h CRB.h CRB_dev.h
main.o: main.c MEM.h CRB.h
native.o: native.c MEM.h DBG.h crowbar.h CRB.h CRB_dev.h
strnig.o: strnig.c MEM.h crowbar.h CRB.h CRB_dev.h
util.o: util.c MEM.h DBG.h crowbar.h CRB.h CRB_dev.h