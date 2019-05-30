#
CC = gcc
CFLAGS = -Os


#
all: alter

alter:
	$(CC) $(CFLAGS) -lm -o alter ../auxfunc/auxfunc.c auxfunc_alter.c alter.c

clean::
	rm alter
