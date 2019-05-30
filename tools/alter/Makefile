#
CC = gcc
CFLAGS = -Os -I. -I/usr/include


#
all: alter

alter:
	$(CC) $(CFLAGS) -o alter ../auxfunc/auxfunc.c auxfunc_alter.c alter.c -lm

clean::
	rm alter
