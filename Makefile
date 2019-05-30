#
CC = gcc
CC_MPI = <FULL_PATH_TO_MPICH>/bin/mpicc
CFLAGS = -Os -I. -I/usr/include


#
all: asco asco-test alter log monte postp

asco-test:
	$(CC) $(CFLAGS) -DDEBUG -DASCO -o asco-test auxfunc.c auxfunc_alter.c auxfunc_monte.c auxfunc_measurefromlis.c rfmodule.c initialize.c errfunc.c evaluate.c asco-test.c -lm

asco:
	$(CC) $(CFLAGS) -DASCO -o asco auxfunc.c auxfunc_alter.c auxfunc_monte.c auxfunc_measurefromlis.c rfmodule.c initialize.c errfunc.c evaluate.c de36.c hooke.c nmlatest.c asco.c -lm

asco-mpi:
	$(CC_MPI) $(CFLAGS) -DASCO -DMPI -o asco-mpi auxfunc.c auxfunc_alter.c auxfunc_monte.c auxfunc_measurefromlis.c rfmodule.c initialize.c errfunc.c evaluate.c de36.c hooke.c nmlatest.c asco.c -lm

alter::
	make -C tools/alter/

log::
	make -C tools/log/

monte::
	make -C tools/monte/

postp::
	make -C tools/postp/

clean::
	rm -f asco asco-test 
	rm -f tools/alter/alter
	rm -f tools/log/log
	rm -f tools/monte/monte
	rm -f tools/postp/postp
