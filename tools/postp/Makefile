#
CC = gcc
CFLAGS = -Os -I. -I/usr/include


#
all: postp

postp:
	$(CC) $(CFLAGS) -lm -o postp ../auxfunc/auxfunc.c auxfunc_updatelis.c auxfunc_measurefromlis.c postp.c

clean::
	rm postp
