CC	= gcc

INCLUDE = -I. -I../include
LIB = -L../lib/ -lmythread

EXEFILES = test/test_list test/test_mythread



.PHONY: lib all clean exe

all: $(EXEFILES)

lib: lib/libmythread.a 

lib/libmythread.a: src/mythread.o
	ar vr lib/libmythread.a src/mythread.o

src/mythread.o: src/mythread.c
	cd src ; $(CC) -g -Wall $(INCLUDE) -c mythread.c

test/test_mythread: test/test_mythread.c lib/libmythread.a
	cd test; $(CC) -g -Wall -lpthread $(INCLUDE) $(LIB) test_mythread.c -o test_mythread

test/test_list: test/test_list.c lib/libmythread.a
	cd test; $(CC) -g -Wall $(INCLUDE) test_list.c -o test_list

clean: 
	rm -f $(EXEFILES) src/*.o lib/*.a *~ */*~


