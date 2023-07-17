CC=gcc
LD=$(CC)
CPPFLAGS=-g -std=gnu11 -Wpedantic -Wall
CFLAGS=-I.
LDFLAGS=
LDLIBS=
PROGRAM=shell

all: $(PROGRAM) 

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

process_handler.o: process_handler.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(PROGRAM): process_handler.o $(PROGRAM).o 
	$(LD) $(CPPFLAGS) $(LDFLAGS) $(LDLIBS) -o $@ $^

.PHONY: clean
clean:
	rm -f *.o $(PROGRAM)
