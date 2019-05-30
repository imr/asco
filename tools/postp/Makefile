#
CC = gcc
CFLAGS = -Os


#
all: postp

postp:
	$(CC) $(CFLAGS) -lm -o postp ../auxfunc/auxfunc.c auxfunc_updatelis.c auxfunc_measurefromlis.c postp.c

clean::
	rm postp
