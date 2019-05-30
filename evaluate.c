/*
 * Copyright (C) 2004-2016 Joao Ramos
 * Your use of this code is subject to the terms and conditions of the
 * GNU general public license version 2. See "COPYING" or
 * http://www.gnu.org/licenses/gpl.html
 *
 * Plug-in to add to 'Eldo', 'HSPICE', 'LTspice', 'Spectre' and 'Qucs' circuit simulator optimization capabilities
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




/*
 *      1: Initialization of all variables and strucutres
 *      2: Return a high cost if proposed values are not within [-10, 10] range except for the first call
 *      3: In DEBUG mode, use initial values from the <inputfile>.cfg file
 *      4: Call optimization routine
 *      5:
 *
 * D       : number of parameters of the cost function
 * x       : parameters proposed by the optimizer routine are stored in 'x'
 * filename: SPICE file containing the netlist to optimize
 */
double evaluate(int D, double x[], char *filename)
{
	int ii;
	double cost;


	/**/
	/*Step1: Initialization of all variables and strucutres*/
	/* (*nfeval)++; */
	cost=0;


	/**/
	/*Step2: Return a high cost if the proposed values are not within [-10, 10] range except when in DEBUG mode*/
	#ifndef DEBUG
	for (ii = 0; ii < D; ii++) {
		if (x[ii]<-10) {
			cost=cost+9e99*(-10-x[ii]);
		}
		if (x[ii]>10) {
			cost=cost+9e99*(x[ii]-10);
		}
	}
	if (fcmp(cost, 0))
		return(cost);
	#endif


	/**/
	/*Step3: In DEBUG mode, use initial values from the <inputfile>.cfg file*/
	#ifdef DEBUG
	for (ii = 0; ii < D; ii++) {
		x[ii] = scaleto(parameters[ii].value, parameters[ii].minimum, parameters[ii].maximum, -10, +10, parameters[ii].format);
	}
	#endif


	/**/
	/*Step4: Call optimization routine*/
	if (spice) {
		#ifdef DEBUG
		printf("DEBUG: evaluate.c - Executing errfunc\n");
		printf("INFO:  evaluate.c - altermc=%d\n", AlterMC);
		#endif
		cost = errfunc(filename, x);
		#ifdef DEBUG
		printf("DEBUG: evaluate.c - Leaving errfunc\n");
		printf("INFO:  evaluate.c - altermc=%d\n", AlterMC);
		#endif
		/*printf("cost=%f\n", cost);*/
	}


	/**/
	/*Step5: */
	return(cost);
}
