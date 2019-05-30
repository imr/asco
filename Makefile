#
CC = gcc
CC_MPI = <FULL_PATH_TO_MPICH>/bin/mpicc
CFLAGS = -Os -I. -I/usr/include


#
all: asco asco-test alter log monte postp

asco-test:
	$(CC) $(CFLAGS) -DDEBUG  -DASCO -lm -o asco-test auxfunc.c auxfunc_alter.c auxfunc_monte.c auxfunc_measurefromlis.c rfmodule.c initialize.c errfunc.c evaluate.c asco-test.c

asco:
	$(CC) $(CFLAGS) -DASCO -lm -o asco auxfunc.c auxfunc_alter.c auxfunc_monte.c auxfunc_measurefromlis.c rfmodule.c initialize.c errfunc.c evaluate.c de36.c hooke.c nmlatest.c asco.c

asco-mpi:
	$(CC_MPI) $(CFLAGS) -DASCO -DMPI -lm -o asco-mpi auxfunc.c auxfunc_alter.c auxfunc_monte.c auxfunc_measurefromlis.c rfmodule.c initialize.c errfunc.c evaluate.c de36.c hooke.c nmlatest.c asco.c

alter::
	make -C tools/alter/

log::
	make -C  tools/log/

monte::
	make -C  tools/monte/

postp::
	make -C  tools/postp/

clean::
	rm -f asco asco-test asco.exe asco-test.exe
	rm -f tools/alter/alter tools/alter/alter.exe
	rm -f tools/log/log tools/log/log.exe
	rm -f tools/monte/monte tools/monte/monte.exe
	rm -f tools/postp/postp tools/postp/postp.exe
