PROGRAM = minesweeper
CC      = gcc
CFLAGS  = -g -std=c99 -Wall -I/usr/X11R6/include -I/usr/pkg/include 
LDFLAGS = -L/usr/X11R6/lib -L/usr/pkg/lib
LDLIBS  = -lglut -lGLU -lGL -lm

$(PROGRAM): main.o
	$(CC) $(LDFLAGS) -o $(PROGRAM) main.o $(LDLIBS)
	
main.o: main.c
	gcc -c main.c

.PHONY: clean dist

clean:
	-rm *.o $(PROGRAM) *core

dist: clean
	-tar -chvj -C .. -f ../$(PROGRAM).tar.bz2 $(PROGRAM)
