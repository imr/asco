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


#include "auxfunc.h"
#include "initialize.h"
#include "errfunc.h"
#ifdef MPI
#include "mpi.h"
#endif




/*---------Function declarations----------------------------------------*/
int extern DE(int argc, char *argv[]);




/*
 *      1: Check input arguments
 *      2: Initialization of all variables and strucutres
 *      3: Call optimization routine
 *
 */
int main(int argc, char *argv[])
{
	int ii;


	/**/
	/*Step1: Check input arguments*/
/*mpi: MPI initialization*/
	#ifdef MPI
	int id, ntasks, err;

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
	/*Step2: Initialization of all variables and strucutres*/
	ii=strpos2(argv[2], ".", 1);
	if (ii) /*filename is "filename.xx.xx"*/
		argv[2][ii-1]='\0';
	if (*argv[1] == 45) /* 45="-" */
		*argv[1]++;

	spice=0;
	switch(*argv[1]) {
		case 'e': /*Eldo*/
			if (!strcmp(argv[1], "eldo")) {
				spice=1;
				printf("INFO:  First time, Eldo initialization\n");
				fflush(stdout);
			}
			break;
		case 'h': /*HSPICE*/
			if (!strcmp(argv[1], "hspice")) {
				spice=2;
				printf("INFO:  First time, HSPICE initialization\n");
				fflush(stdout);
			}
			break;
		case 'l': /*LTspice*/
			if (!strcmp(argv[1], "ltspice")) {
				spice=3;
				printf("INFO:  First time, LTSpice initialization\n");
				fflush(stdout);
			}
			break;
		case 's': /*Spectre*/
			if (!strcmp(argv[1], "spectre")) {
				spice=4;
				printf("INFO:  First time, Spectre initialization\n");
				fflush(stdout);
			}
			break;
		case 'r': /*rosen*/
			if (!strcmp(argv[1], "rosen")) {
				spice=100;
				printf("INFO:  First time, ROZEN initialization\n");
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
		printf("INFO:  Initialization has fineshed without errors\n");
		fflush(stdout);
	} else {
		printf("asco.c -- Unsupport SPICE simulator: %s\n", argv[1]);
		fflush(stdout);
		exit(EXIT_FAILURE);
	}


	/**/
	/*Step3: Call optimization routine*/
	printf("\n\n                         PRESS CTRL-C TO ABORT\n");
	fflush(stdout);
	if (spice) {
		DE(argc, argv); /*Rainer Storn and Ken Price Differential Evolution (DE)*/
	}


	/**/
	/*Step4:*/
	return(EXIT_SUCCESS);
}
