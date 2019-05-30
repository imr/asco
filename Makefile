#
CC = gcc
CFLAGS = -O2


#
all: asco asco-test

asco-test:
	$(CC) $(CFLAGS) -DDEBUG -lm -o asco-test auxfunc.c auxfunc_alter.c auxfunc_monte.c auxfunc_measurefromlis.c initialize.c errfunc.c evaluate.c asco-test.c

asco:
	$(CC) $(CFLAGS) -lm -o asco auxfunc.c auxfunc_alter.c auxfunc_monte.c auxfunc_measurefromlis.c initialize.c errfunc.c evaluate.c de36.c asco.c

clean::
	rm -f asco asco-test
