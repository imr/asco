/*
 * Copyright (C) 2004-2005 Joao Ramos
 * Your use of this code is subject to the terms and conditions of the
 * GNU general public license version 2. See "COPYING" or
 * http://www.gnu.org/licenses/gpl.html
 *
 * Plug-in to add to 'Eldo', 'HSPICE', 'LTSpice' and 'Spectre' circuit simulator optimization capabilities
 *
 */

#include <string.h>


#include "de.h"

#include "auxfunc.h"
#include "initialize.h"
#include "errfunc.h"




/*---------Function declarations----------------------------------------*/
double extern evaluate(int D, double tmp[], long *nfeval, char *argv); /* obj. funct. */
int extern DE(int argc, char *argv[]);




/*##############################################################################
 *################      M A I N              P R O G R A M      ################
 *############################################################################## */
int main(int argc, char *argv[])
{
	int   D; /* Dimension of parameter vector      */
	long  nfeval=0; /* number of function evaluations     */
	double x[MAXDIM]; /* members  */
	double cost=0;
	int i,ii;


/*------Initializations----------------------------*/
	/**/
	/*Step1: Check input arguments*/
	if (argc != 3) { /* number of arguments */
		printf("\nUsage : asco -<simulator> <inputfile>\n");
		printf("\nExamples:\n");
		printf("          asco-test -eldo    <inputfile>.cir\n");
		printf("          asco-test -hspice  <inputfile>.sp\n");
		printf("          asco-test -ltspice <inputfile>.net\n");
		printf("          asco-test -spectre <inputfile>.scs\n");
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
				printf("asco-test.c -- Unsupport SPICE simulator: %s\n", argv[1]);
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
	/*Step3: define needed variables value */
	Wobj=10; Wcon=100;

	D=0;
	while (parameters[D].name[0]  != '\0')
		D++;                                     /*---number of parameters---------------*/


	/**/
	/*Step4: call optimization routine */
	ii=1;
	for (i = 0; i < ii; i++) {
		cost = evaluate(D,x,&nfeval,argv[2]);  /* Evaluate new vector in tmp[] */
		if (nfeval==1)
			ii=AlterMC+2;
	}
	return(0);
}
