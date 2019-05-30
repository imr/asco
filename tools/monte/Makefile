#
CC = gcc
CFLAGS = -Os -I. -I/usr/include


#
all: monte

monte:
	$(CC) $(CFLAGS) -o monte ../auxfunc/auxfunc.c auxfunc_monte.c monte.c -lm

clean::
	rm monte
