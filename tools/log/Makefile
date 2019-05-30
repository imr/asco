#
CC = gcc
CFLAGS = -Os


#
all: log

log:
	$(CC) $(CFLAGS) -lm -o log dosdir.c match.c auxfunc.c auxfunc_log.c log.c

clean::
	rm -f log
