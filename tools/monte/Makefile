#
CC = gcc
CFLAGS = -Os -I. -I/usr/include


#
all: monte

monte:
	$(CC) $(CFLAGS) -lm -o monte ../auxfunc/auxfunc.c auxfunc_monte.c monte.c

clean::
	rm monte
