/*
 * Copyright (C) 2004-2006 Joao Ramos
 * Your use of this code is subject to the terms and conditions of the
 * GNU general public license version 2. See "COPYING" or
 * http://www.gnu.org/licenses/gpl.html
 *
 * Plug-in to add to 'Eldo', 'HSPICE', 'LTSpice' and 'Spectre' circuit simulator optimization capabilities
 *
 */

#include <stdio.h>
/* #include <ctype.h> */
#include <math.h>
/* #include <setjmp.h> */
/* #include <assert.h> */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#include "version.h"
#include "auxfunc.h"
#include "initialize.h"
#include "errfunc.h"
#ifdef MPI
#include "mpi.h"
#endif




/*---------Function declarations----------------------------------------*/
int extern DE(int argc, char *argv[]);
int extern HJ(int argc, char *argv[]);
int extern NM(int argc, char *argv[]);




/*
 *      1: Copyright info
 *      2: Check input arguments
 *      3: Initialization of all variables and strucutres
 *      4: Call optimization routine
 *      5: If in parallel optimization mode, copy back the log file to the starting directory
 *
 */
int main(int argc, char *argv[])
{
	int ii, ccode;
	char hostname[SHORTSTRINGSIZE];
	#ifdef MPI /*If in parallel optimization mode, copy all files to /tmp/asco */
	int id, ntasks, err;
	int pid=0;
	char currentdir[LONGSTRINGSIZE], optimizedir[LONGSTRINGSIZE];
	#endif


	/**/
	/*Step1: Copyright info*/
	printf("\n%s - %s\n", VERSION, COPYRIGHT);
	printf("%s\n\n",GPL_INFO);


	/**/
	/*Step2: Check input arguments*/
/*mpi: MPI initialization*/
	#ifdef MPI
	err = MPI_Init(&argc, &argv); /* Initialize MPI */
	if (err != MPI_SUCCESS) {
		printf("asco.c - MPI initialization failed!\n");
		exit(EXIT_FAILURE);
	}
	err = MPI_Comm_size(MPI_COMM_WORLD, &ntasks); /* Get nr of tasks */
	err = MPI_Comm_rank(MPI_COMM_WORLD, &id);     /* Get id of this process */
	if (ntasks < 2) {
		printf("\nAt least 2 processors are required to run this program\n");
		printf("\nExamples:\n");
		printf("          mpirun -p4pg machines.txt asco-mpi -<simulator> <inputfile>\n");
		printf("          mpirun -np 2 asco-mpi -<simulator> <inputfile>\n\n\n");
		MPI_Finalize(); /* Quit if there is only one processor */
		exit(EXIT_FAILURE);
	}
	#endif
/*mpi: MPI initialization*/
	if (argc != 3) { /* number of arguments */
		printf("\nUsage : asco -<simulator> <inputfile>\n");
		printf("\nExamples:\n");
		printf("          asco -eldo    <inputfile>.cir\n");
		printf("          asco -hspice  <inputfile>.sp\n");
		printf("          asco -ltspice <inputfile>.net\n");
		printf("          asco -spectre <inputfile>.scs\n");
		printf("\nDefault file extension is assumed if not specified\n\n\n");
		exit(EXIT_FAILURE);
	}


	/**/
	/*Step3: Initialization of all variables and strucutres*/
	if ((ccode = gethostname(hostname, sizeof(hostname))) != 0) {
		printf("asco.c -- gethostname failed, ccode = %d\n", ccode);
		exit(EXIT_FAILURE);
	}
	/* printf("host name: %s\n", hostname); */
	ii=strpos2(argv[2], ".", 1);
	if (ii) /* filename is "filename.xx.xx" */
		argv[2][ii-1]='\0';
	if (*argv[1] == 45) /* 45="-" */
		*argv[1]++;

	#ifdef MPI /*If in parallel optimization mode, copy all files to /tmp/asco */
	if (id) { /*If it is a slave process*/
		pid=getpid();
		getcwd(currentdir, sizeof(currentdir));

		sprintf(optimizedir, "/tmp/asco%d", pid); /*allows multiple runs on the same computer name*/
		//sprintf(optimizedir, "/tmp/asco");        /*alow one run on each computer*/

		sprintf(lkk, "mkdir %s > /dev/null", optimizedir);
		system(lkk);
		sprintf(lkk, "cp -rfp * %s> /dev/null", optimizedir);
		system(lkk);

	chdir(optimizedir);
	}
	#endif

	getcwd(lkk, sizeof(lkk));
	printf("INFO:  Current directory on '%s': %s\n", hostname, lkk);
	fflush(stdout);

	spice=0;
	switch(*argv[1]) {
		case 'e': /*Eldo*/
			if (!strcmp(argv[1], "eldo")) {
				spice=1;
				printf("INFO:  Eldo initialization on '%s'\n", hostname);
				fflush(stdout);
			}
			break;
		case 'h': /*HSPICE*/
			if (!strcmp(argv[1], "hspice")) {
				spice=2;
				printf("INFO:  HSPICE initialization on '%s'\n", hostname);
				fflush(stdout);
			}
			break;
		case 'l': /*LTspice*/
			if (!strcmp(argv[1], "ltspice")) {
				spice=3;
				printf("INFO:  LTSpice initialization on '%s'\n", hostname);
				fflush(stdout);
			}
			break;
		case 's': /*Spectre*/
			if (!strcmp(argv[1], "spectre")) {
				spice=4;
				printf("INFO:  Spectre initialization on '%s'\n", hostname);
				fflush(stdout);
			}
			break;
		case 'g': /*general*/
			if (!strcmp(argv[1], "general")) {
				spice=100;
				printf("INFO:  GENERAL initialization on '%s'\n", hostname);
				fflush(stdout);
			}
			break;
		default:
			printf("asco.c -- Unsupport SPICE simulator: %s\n", argv[1]);
			fflush(stdout);
			exit(EXIT_FAILURE);
	}
	if (spice) {
		if (initialize(argv[2]))
			exit(EXIT_FAILURE);
		printf("INFO:  Initialization has fineshed without errors on '%s'\n", hostname);
		fflush(stdout);
	} else {
		printf("asco.c -- Unsupport SPICE simulator: %s\n", argv[1]);
		fflush(stdout);
		exit(EXIT_FAILURE);
	}


	/**/
	/*Step4: Call optimization routine*/
	printf("\n                         PRESS CTRL-C TO ABORT");
	fflush(stdout);
	if (spice) {
		/*Global optimizer(s)*/
		printf("\n\nINFO:  Starting global optimizer on '%s'...\n", hostname);
		fflush(stdout);
		DE(argc, argv); /*Rainer Storn and Ken Price Differential Evolution (DE)*/
		/* Wobj=10; Wcon=100; */
		/* opt(argc, argv); */ /*Tibor Csendes: GLOBAL*/

		/*Local optimizer(s)*/
		/* Wobj=10; Wcon=100; */
		printf("\n\nINFO:  Starting local optimizer on '%s'...\n", hostname);
		fflush(stdout);
		HJ(argc, argv); /*Hooke and Jeeves*/
		/* NM(argc, argv); */ /*Nelder-Mead*/
	}


	/**/
	/*Step5: If in parallel optimization mode, copy back the log file to the starting directory*/
	#ifdef MPI
	if (id) { /*If it is a slave process*/
		switch(spice) {
			case 1: /*Eldo*/
				sprintf(lkk, "cp -fp %s.log %s/%s_%d.log > /dev/null", hostname, currentdir, hostname, pid); /* hostname is "longmorn" */
				break;
			case 2: /*HSPICE*/
				sprintf(lkk, "cp -fp %s.log %s/%s_%d.log > /dev/null", hostname, currentdir, hostname, pid); /* hostname is "longmorn" */
				break;
			case 3: /*LTSpice*/
				sprintf(lkk, "cp -fp %s.log.log %s/%s_%d.log.log > /dev/null", hostname, currentdir, hostname, pid); /* hostname is "longmorn" */
				break;
			case 4: /*Spectre*/
				sprintf(lkk, "cp -fp %s.log %s/%s_%d.log > /dev/null", hostname, currentdir, hostname, pid); /* hostname is "longmorn" */
				break;
			case 100: /*general*/
				sprintf(lkk, "cp -fp %s.log %s/%s_%d.log > /dev/null", hostname, currentdir, hostname, pid); /* hostname is "longmorn" */
				break;
			default:
				printf("errfunc.c -- Something unexpected has happened!\n");
				exit(EXIT_FAILURE);
		}
		system(lkk); /*copy log files*/

		sprintf(lkk, "rm -rf /tmp/asco%d", pid);
		system(lkk); /*delete ALL temporary optimization data written to /tmp/asco<PID> */

		/* chdir(currentdir); */
	}
	#endif
	printf("INFO:  ASCO has ended on '%s'.\n", hostname);
	fflush(stdout);
	return(EXIT_SUCCESS);
}
