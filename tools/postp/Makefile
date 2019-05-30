#
CC = gcc
CFLAGS = -Os -I. -I/usr/include


#
all: postp

postp:
	$(CC) $(CFLAGS) -o postp ../auxfunc/auxfunc.c auxfunc_updatelis.c auxfunc_measurefromlis.c postp.c -lm

clean::
	rm postp
