/*
 *  Copyright (C) 2004-2005 Joao Ramos
 * Your use of this code is subject to the terms and conditions of the
 * GNU general public license version 2. See "COPYING" or
 * http://www.gnu.org/licenses/gpl.html
 *
 * Plug-in to add to 'Eldo', 'HSPICE' and 'LTSpice' circuit simulator optimization capabilities
 *
 */

#include <stdio.h>
//#include <ctype.h>
#include <math.h>
//#include <setjmp.h>
//#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#include "auxfunc.h"
#include "initialize.h"
#include "errfunc.h"




/*
 *      1: call function 'initialize' to prepare structures and files for simulation
 *      2: call function 'errfunc' which does the remaining task and runs the SPICE simulator
 *
 * D: number of parameters of the cost function
 * x: parameters proposed by the optimizer routine are stored in 'x'
 */
double evaluate(int D, double x[], long *nfeval, char *filename)
{
	int ii;
	double cost;


	//
	// Step1: Initialization of all variables and strucutres
	(*nfeval)++;


	//
	// Step2: return a high cost if proposed values are not within [-10, 10] range except for the first call
	#ifndef DEBUG
	cost=0;
	for (ii = 0; ii < D; ii++) {
		if (x[ii]<-10) {
			cost=cost+1e30*(-10-x[ii]);
		}
		if (x[ii]>10) {
			cost=cost+1e30*(x[ii]-10);
		}
	}
	if (cost)
		return(cost);
	#endif


	//
	//Step3: use initial values stored in the <inputfile>.cfg file
	#ifndef DEBUG
	if (*nfeval==1) {
	#else
	if (1) {
	#endif
		for (ii = 0; ii < D; ii++) {
			x[ii] = scaleto(parameters[ii].value, parameters[ii].minimum, parameters[ii].maximum, -10, +10, parameters[ii].format);
		}
	}



	//
	//Step4: call optimization routine
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
		//printf("cost=%f\n", cost);
	}
	
	//
	//Step5:
	return(cost);
}
