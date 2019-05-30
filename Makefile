#
CC = gcc
CFLAGS = -Os


#
all: asco asco-test log

asco-test:
	$(CC) $(CFLAGS) -DDEBUG -lm -o asco-test auxfunc.c auxfunc_alter.c auxfunc_monte.c auxfunc_measurefromlis.c rfmodule.c initialize.c errfunc.c evaluate.c asco-test.c

asco:
	$(CC) $(CFLAGS) -lm -o asco auxfunc.c auxfunc_alter.c auxfunc_monte.c auxfunc_measurefromlis.c rfmodule.c initialize.c errfunc.c evaluate.c de36.c asco.c

log::
	cd tools/log; make

clean::
	rm -f asco asco-test
	rm -f tools/log/log
