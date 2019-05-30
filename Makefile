#
CC = gcc
CFLAGS = -Os


#
all: asco asco-test alter log monte postp

asco-test:
	$(CC) $(CFLAGS) -DDEBUG -lm -o asco-test auxfunc.c auxfunc_alter.c auxfunc_monte.c auxfunc_measurefromlis.c rfmodule.c initialize.c errfunc.c evaluate.c asco-test.c

asco:
	$(CC) $(CFLAGS) -lm -o asco auxfunc.c auxfunc_alter.c auxfunc_monte.c auxfunc_measurefromlis.c rfmodule.c initialize.c errfunc.c evaluate.c de36.c asco.c

alter::
	cd tools/alter/; make

log::
	cd tools/log/ ; make

monte::
	cd tools/monte/; make

postp::
	cd tools/postp/; make

clean::
	rm -f asco asco-test
	rm -f tools/alter/alter
	rm -f tools/log/log
	rm -f tools/monte/monte
	rm -f tools/postp/postp
