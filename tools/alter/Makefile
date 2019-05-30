#
CC = gcc
CFLAGS = -Os -I. -I/usr/include


#
all: alter

alter:
	$(CC) $(CFLAGS) -lm -o alter ../auxfunc/auxfunc.c auxfunc_alter.c alter.c

clean::
	rm alter
