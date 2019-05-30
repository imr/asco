/*
 *  Copyright (C) 1999-2006 Joao Ramos
 * Your use of this code is subject to the terms and conditions of the
 * GNU general public license version 2. See "COPYING" or
 * http://www.gnu.org/licenses/gpl.html
 */

#include <stdio.h>
/* #include <ctype.h> */
#include <math.h>
/* #include <setjmp.h> */
/* #include <assert.h> */
#include <stdlib.h>
#include <string.h>


#include "auxfunc.h"
#include "auxfunc_log.h"




/*##############################################################################*/
/*################      M A I N              P R O G R A M      ################*/
/*##############################################################################*/
int main(int argc, char *argv[])
{
	/**/
	/*Step1: Check input arguments*/
	if (argc != 3) { /* number of arguments */
		printf("\nUsage : log <inputfile>.log <outputfile>\n\n\n");
	exit(EXIT_FAILURE);
	}


	/**/
	/*Step2: */
	if (!strcmp(argv[1], argv[2])) { /* number of arguments */
		printf("log.c -- Input and output file must be different.\n");
	exit(EXIT_FAILURE);
	}


	/**/
	/*Step3: */
	CreateStatistics(argv[1], argv[2]);
	exit(EXIT_SUCCESS);
}
