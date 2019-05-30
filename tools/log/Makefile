#
CC = gcc
CFLAGS = -Os -I. -I/usr/include


#
all: log

log:
	$(CC) $(CFLAGS) -o log dosdir.c match.c auxfunc.c auxfunc_log.c log.c -lm

clean::
	rm -f log
