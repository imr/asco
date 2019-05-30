/*
 * Copyright (C) 2004-2012 Joao Ramos
 * Your use of this code is subject to the terms and conditions of the
 * GNU general public license version 2. See "COPYING" or
 * http://www.gnu.org/licenses/gpl.html
 *
 * Plug-in to add to 'Eldo', 'HSPICE', 'LTspice', 'Spectre', 'Qucs' and 'ngspice' circuit simulator optimization capabilities
 *
 */

#include <string.h>
#include <unistd.h>
#ifdef __MINGW32__
#include <winsock2.h>
#endif


#include "de.h"

#include "version.h"
#include "auxfunc.h"
#include "initialize.h"
#include "errfunc.h"




/*---------Function declarations----------------------------------------*/
extern double evaluate(int D, double tmp[], char *argv); /* obj. funct. */
extern int DE(int argc, char *argv[]);




/*##############################################################################
 *################      M A I N              P R O G R A M      ################
 *############################################################################## */
int main(int argc, char *argv[])
{
	int   D; /* Dimension of parameter vector      */
	long  nfeval=0; /* number of function evaluations     */
	double x[MAXDIM]; /* members  */
	double cost=0;
	int i,ii, ccode;
	char hostname[SHORTSTRINGSIZE];


	/**/
	/*Step1: Copyright info*/
	printf("\n%s - %s\n", VERSION, COPYRIGHT);
	printf("%s\n\n",GPL_INFO);


	/**/
	/*Step2: Check input arguments*/
	if (argc != 3) { /* number of arguments */
		printf("\nUsage : asco-test -<simulator> <inputfile>\n");
		printf("\nExamples:\n");
		printf("          asco-test -eldo    <inputfile>.cir\n");
		printf("          asco-test -hspice  <inputfile>.sp\n");
		printf("          asco-test -ltspice <inputfile>.net\n");
		printf("          asco-test -spectre <inputfile>.scs\n");
		printf("          asco-test -qucs    <inputfile>.txt\n");
		printf("          asco-test -ngspice <inputfile>.sp\n");
		printf("          asco-test -general <inputfile>.*\n");
		printf("\nDefault file extension is assumed if not specified\n\n\n");
		exit(EXIT_FAILURE);
	}


	/**/
	/*Step3: Initialization of all variables and structures*/
	#ifdef __MINGW32__
	{
		WSADATA WSAData;
		WSAStartup (0x0202, &WSAData);
	}
	#endif
	if ((ccode = gethostname(hostname, sizeof(hostname))) != 0) {
		printf("asco-test.c -- gethostname failed, ccode = %d\n", ccode);
		exit(EXIT_FAILURE);
	}
	/* printf("host name: %s\n", hostname); */
	ii=strpos2(hostname, ".", 1);
	if (ii)                                 /* hostname is "longmorn.xx.xx.xx" */
		hostname[ii-1]='\0';
	/*                                   */ /* hostname is "longmorn" */

	strcpy(lkk, argv[2]);
	ii=strpos2(lkk, "/", 1);
	#ifdef __MINGW32__
	if (ii==0)
		ii=strpos2(lkk, "\\", 1);
	#endif
	if (ii) {           /*should character '/' exist, files are in a different directory*/
		ii=(int)strlen(lkk);
		#ifndef __MINGW32__
		while (lkk[ii] != '/')
		#else
		while (lkk[ii] != '/' && lkk[ii] != '\\')
		#endif
			ii--;

		lkk[ii+1]='\0';
		chdir(lkk); /*now, change directory                                         */
	}
	else
		ii=1;
	ii=strpos2(argv[2], ".", ii);
	if (ii) /* filename is "filename.xx.xx" */
		argv[2][ii-1]='\0';
	if (*argv[1] == 45) /* 45="-" */
		*argv[1]++;

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
				printf("INFO:  LTspice initialization on '%s'\n", hostname);
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
		case 'q': /*Qucs*/
			if (!strcmp(argv[1], "qucs")) {
				spice=50;
				printf("INFO:  Qucs initialization on '%s'\n", hostname);
				fflush(stdout);
			}
			break;
		case 'n': /*ngspice*/
			if (!strcmp(argv[1], "ngspice")) {
				spice=51;
				printf("INFO:  ngspice initialization on '%s'\n", hostname);
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
			printf("asco-test.c -- Unsupport SPICE simulator: %s\n", argv[1]);
			fflush(stdout);
			exit(EXIT_FAILURE);
	}
	if (spice) {
		if (initialize(argv[2]))
			exit(EXIT_FAILURE);
		printf("INFO:  Initialization has finished without errors\n");
		fflush(stdout);
	} else {
		printf("asco-test.c -- Unsupport SPICE simulator: %s\n", argv[1]);
		fflush(stdout);
		exit(EXIT_FAILURE);
	}


	/**/
	/*Step4: define needed variables value */
	Wobj=10; Wcon=100;

	D=0;
	while (parameters[D].name[0]  != '\0')
		D++;                                     /*---number of parameters---------------*/

	/* -------------------------------------- */
	if (1) { /*emulate local search using the global optimizer when (maximum=minimum) and format!=0*/
		ii=0;
		while (parameters[ii].format != 0) {
			if ( !fcmp(parameters[ii].maximum, parameters[ii].minimum)) { /*This is, if they are equal*/
				if (!fcmp(parameters[ii].value, 0) ) { /*This is, if equal to zero*/
					printf("asco-test.c - Value is zero when using DE in emulated local mode.");
					exit(EXIT_FAILURE);
				}
				if ((parameters[ii].value) > 0) {
					parameters[ii].maximum=1.1*parameters[ii].value;
					parameters[ii].minimum=0.9*parameters[ii].value;
				} else {
					parameters[ii].maximum=0.9*parameters[ii].value;
					parameters[ii].minimum=1.1*parameters[ii].value;
				}
			}
			ii++;
		}
	}
	/* -------------------------------------- */


	/**/
	/*Step5: call optimization routine */
	ii=1;
	for (i = 0; i < ii; i++) {
		nfeval++;
		cost = evaluate(D,x,argv[2]);  /* Evaluate new vector in tmp[] */
		if (nfeval==1)
			ii=AlterMC+2;
	}
	return(EXIT_SUCCESS);
}
