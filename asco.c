/*
 * Copyright (C) 2004-2005 Joao Ramos
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




/*---------Function declarations----------------------------------------*/
int extern DE(int argc, char *argv[]);




/*
 *      1: call function 'initialize' to prepare structures and files for simulation
 *      2: call function 'errfunc' which does the remaining task and runs the SPICE simulator
 *
 * D: number of parameters of the cost function
 * x: parameters proposed by the optimizer routine are stored in 'x'
 */
int main(int argc, char *argv[])
{
	int ii;


	/**/
	/*Step1: Check input arguments*/
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
	if (1) { /*is it first time the optimization is runned?*/
		if (*argv[1] == 45) /* 45="-" */
			*argv[1]++;

		spice=0;
		switch(*argv[1]) {
			case 'e': /*Eldo*/
				if (!strcmp(argv[1], "eldo")) {
					spice=1;
					printf("INFO:  First time, Eldo initialization\n");
				}
				break;
			case 'h': /*HSPICE*/
				if (!strcmp(argv[1], "hspice")) {
					spice=2;
					printf("INFO:  First time, HSPICE initialization\n");
				}
				break;
			case 'l': /*LTspice*/
				if (!strcmp(argv[1], "ltspice")) {
					spice=3;
					printf("INFO:  First time, LTSpice initialization\n");
				}
				break;
			case 's': /*Spectre*/
				if (!strcmp(argv[1], "spectre")) {
					spice=4;
					printf("INFO:  First time, Spectre initialization\n");
				}
				break;
			case 'r': /*rosen*/
				if (!strcmp(argv[1], "rosen")) {
					spice=100;
					printf("INFO:  First time, ROZEN initialization\n");
				}
				break;
			default:
				printf("asco.c -- Unsupport SPICE simulator: %s\n", argv[1]);
				exit(EXIT_FAILURE);
		}
		if (spice) {
			if (initialize(argv[2]))
				exit(EXIT_FAILURE);
			printf("INFO:  Initialization has fineshed without errors\n");
		} else {
			printf("evaluate.c -- Unsupport SPICE simulator: %s\n", argv[1]);
			exit(EXIT_FAILURE);
		}
	}


	/**/
	/*Step3: call optimization routine*/
	if (spice) {
		DE(argc, argv); /*Rainer Storn and Ken Price Differential Evolution (DE)*/
	}

	
	/**/
	/*Step4:*/
	return(0);
}
