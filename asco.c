/*
 * Copyright (C) 2004-2016 Joao Ramos
 * Your use of this code is subject to the terms and conditions of the
 * GNU general public license version 2. See "COPYING" or
 * http://www.gnu.org/licenses/gpl.html
 *
 * Plug-in to add to 'Eldo', 'HSPICE', 'LTspice', 'Spectre', 'Qucs' and 'ngspice' circuit simulator optimization capabilities
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
#ifdef __MINGW32__
#include <winsock2.h>
#endif


#include "version.h"
#include "auxfunc.h"
#include "initialize.h"
#include "errfunc.h"
#ifdef MPI
#include "mpi.h"
#endif




/*---------Function declarations----------------------------------------*/
extern int DE(int argc, char *argv[]);
extern int HJ(int argc, char *argv[]);
extern int NM(int argc, char *argv[]);




/*
 *      1: Copyright info
 *      2: Check input arguments
 *      3: Initialization of all variables and strucutres
 *      4: Call optimization routine
 *      5: If in parallel optimization mode, copy back the log file to the starting directory
 *      6: Rename output files, currently only for Qucs
 *
 */
int main(int argc, char *argv[])
{
	int ii, ccode;
	char hostname[SHORTSTRINGSIZE];
	#ifdef MPI
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
		printf("          mpirun -f machines.txt [-np X] asco-mpi -<simulator> <inputfile>\n");
		printf("          mpirun -np 2 asco-mpi -<simulator> <inputfile>\n\n\n");
		MPI_Finalize(); /* Quit if there is only one processor */
		exit(EXIT_FAILURE);
	}
	#endif
/*mpi: MPI initialization*/
	if (argc < 3) { /* number of arguments */
		printf("\nUsage : asco -<simulator> <inputfile>\n");
		printf("\nExamples:\n");
		printf("          asco -eldo    <inputfile>.cir\n");
		printf("          asco -hspice  <inputfile>.sp\n");
		printf("          asco -ltspice <inputfile>.net\n");
		printf("          asco -spectre <inputfile>.scs\n");
		printf("          asco -qucs    <inputfile>.txt\n");
		printf("          asco -ngspice <inputfile>.sp\n");
		printf("          asco -general <inputfile>.*\n");
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
		printf("asco.c -- gethostname failed, ccode = %d\n", ccode);
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
			printf("asco.c -- Unsupport SPICE simulator: %s\n", argv[1]);
			fflush(stdout);
			exit(EXIT_FAILURE);
	}

	#ifdef MPI /*If in parallel optimization mode, copy all files to /tmp/asco */
	if (id) { /*If it is a slave process*/
		pid=getpid();
		getcwd(currentdir, sizeof(currentdir));

		sprintf(optimizedir, "/tmp/asco%d", pid); /*allows multiple runs on the same computer name*/
		/*sprintf(optimizedir, "/tmp/asco"); */        /*alow one run on each computer*/

		#ifndef __MINGW32__
		sprintf(lkk, "mkdir %s > /dev/null", optimizedir);
		system(lkk);
		sprintf(lkk, "cp -rfp * %s> /dev/null", optimizedir);
		system(lkk);
		#else
		sprintf(lkk, "mkdir %s > NUL", optimizedir);
		system(lkk);
		sprintf(lkk, "xcopy * %s /e /k /r /y > NUL", optimizedir);
		system(lkk);
		#endif

		chdir(optimizedir);
	}
	#endif

	if (spice) {
		if (initialize(argv[2]))
			exit(EXIT_FAILURE);
		printf("INFO:  Initialization has finished without errors on '%s'\n", hostname);
		fflush(stdout);
	} else {
		printf("asco.c -- Unsupport SPICE simulator: %s\n", argv[1]);
		fflush(stdout);
		exit(EXIT_FAILURE);
	}

	getcwd(lkk, sizeof(lkk));
	printf("INFO:  Current directory on '%s': %s\n", hostname, lkk);
	fflush(stdout);


	/**/
	/*Step4: Call optimization routine*/
	if (spice) {
		/*Global optimizer(s)*/
		#ifdef MPI
		if (id) { /*If it is a slave process*/
			printf("\n\nINFO:  Starting global optimizer on '%s'...\n", hostname);
			fflush(stdout);
		} else {
			printf("\n                         PRESS CTRL-C TO ABORT");
			fflush(stdout);
			printf("\n\n");
			fflush(stdout);
		}
		#else
		printf("\n                         PRESS CTRL-C TO ABORT");
		fflush(stdout);
		printf("\n\nINFO:  Starting global optimizer on '%s'...\n", hostname);
		fflush(stdout);
		#endif
		/* -------------------------------------- */
		if (1) { /*emulate local search using the global optimizer when (maximum=minimum) and format!=0*/
			ii=0;
			ccode=0; /*'ccode' behaves like a boolean variable*/
			while (parameters[ii].format != 0) {
				if ( !fcmp(parameters[ii].maximum, parameters[ii].minimum)) { /*This is, if they are equal*/
					if (!fcmp(parameters[ii].value, 0) ) { /*This is, if equal to zero*/
						printf("asco.c - Value is zero when using DE in emulated local mode.");
						exit(EXIT_FAILURE);
					}
					if ((parameters[ii].value) > 0) {
						parameters[ii].maximum=1.1*parameters[ii].value;
						parameters[ii].minimum=0.9*parameters[ii].value;
						#ifdef MPI
						if ((ccode==0) & (id==0)) { /*Only the Master will print*/
						#else
						if (ccode==0) {
						#endif
							printf("       ...using local-global optimizer for: ");
							fflush(stdout);
							ccode++;
						}
						printf("%s ", parameters[ii].name);
						fflush(stdout);
					} else {
						parameters[ii].maximum=0.9*parameters[ii].value;
						parameters[ii].minimum=1.1*parameters[ii].value;
						#ifdef MPI
						if ((ccode==0) & (id==0)) { /*Only the Master will print*/
						#else
						if (ccode==0) {
						#endif
							printf("       ...using local-global optimizer for: ");
							fflush(stdout);
							ccode++;
						}
						printf("%s ", parameters[ii].name);
						fflush(stdout);
					}
				}
				ii++;
			}
			if (ccode)
				printf("\n");
		}
		/* -------------------------------------- */
		DE(argc, argv); /*Rainer Storn and Ken Price Differential Evolution (DE)*/
		/* Wobj=10; Wcon=100; */
		/* opt(argc, argv); */ /*Tibor Csendes: GLOBAL*/

		/*Local optimizer(s)*/
		/* Wobj=10; Wcon=100; */
		#ifdef MPI
		if ((MPI_EXXIT==0) & (id==0)) { /*Cannot print in MPI if Ctrl-C has been pressed because SSH is immediatly killed*/
		#else
		{
		#endif
			printf("\n\nINFO:  Starting local optimizer on '%s'...\n", hostname);
			fflush(stdout);

			HJ(argc, argv); /*Hooke and Jeeves*/
			/* NM(argc, argv); */ /*Nelder-Mead*/
		}
	}


	/**/
	/*Step5: If in parallel optimization mode, copy back the log file to the starting directory*/
	#ifdef MPI
	if (id) { /*If it is a slave process*/
		switch(spice) {
			case 1: /*Eldo*/
				sprintf(lkk, "cp -fp %s.log %s/%s_%d.log > /dev/null", hostname, currentdir, hostname, pid);
				break;
			case 2: /*HSPICE*/
				#ifndef __MINGW32__
				sprintf(lkk, "cp -fp %s.log %s/%s_%d.log > /dev/null", hostname, currentdir, hostname, pid);
				#else
				sprintf(lkk, "copy /y %s.log %s/%s_%d.log > NUL", hostname, currentdir, hostname, pid);
				#endif
				break;
			case 3: /*LTspice*/
				#ifndef __MINGW32__
				sprintf(lkk, "cp -fp %s.log.log %s/%s_%d.log.log > /dev/null", hostname, currentdir, hostname, pid);
				#else
				sprintf(lkk, "copy /y %s.log.log %s/%s_%d.log.log > NUL", hostname, currentdir, hostname, pid);
				#endif
				break;
			case 4: /*Spectre*/
				sprintf(lkk, "cp -fp %s.log %s/%s_%d.log > /dev/null", hostname, currentdir, hostname, pid);
				break;
			case 50: /*Qucs*/
				#ifndef __MINGW32__
				sprintf(lkk, "cp -fp %s.log %s/%s_%d.log > /dev/null", hostname, currentdir, hostname, pid);
				#else
				sprintf(lkk, "copy /y %s.log %s/%s_%d.log > NUL", hostname, currentdir, hostname, pid);
				#endif
				break;
			case 51: /*ngspice*/
				#ifndef __MINGW32__
				sprintf(lkk, "cp -fp %s.log %s/%s_%d.log > /dev/null", hostname, currentdir, hostname, pid);
				#else
				sprintf(lkk, "copy /y %s.log %s/%s_%d.log > NUL", hostname, currentdir, hostname, pid);
				#endif
				break;
			case 100: /*general*/
				#ifndef __MINGW32__
				sprintf(lkk, "cp -fp %s.log %s/%s_%d.log > /dev/null", hostname, currentdir, hostname, pid);
				#else
				sprintf(lkk, "copy /y %s.log %s/%s_%d.log > NUL", hostname, currentdir, hostname, pid);
				#endif
				break;
			default:
				printf("errfunc.c -- Something unexpected has happened!\n");
				exit(EXIT_FAILURE);
		}
		system(lkk); /*copy log files*/

		#ifndef __MINGW32__
		sprintf(lkk, "rm -rf /tmp/asco%d", pid);
		#else
		sprintf(lkk, "rmdir /q /s /tmp/asco%d", pid);
		#endif
		system(lkk); /*delete ALL temporary optimization data written to /tmp/asco<PID> */

		/* chdir(currentdir); */
	}
	#endif


	/**/
	/*Step6: Rename output files, currently only for Qucs*/
	#ifdef MPI
	if (id==0) { /*Only for Master process*/
	#else
	{
	#endif
		#ifndef __MINGW32__
		if ((spice==50) && (argc==5) ) { /*Qucs*/
			sprintf(lkk, "cp -fp %s.dat %s.dat > /dev/null", hostname, argv[4]);
			system(lkk); /*copy simulation output file*/
			sprintf(lkk, "cp -fp %s.log %s.log > /dev/null", hostname, argv[4]);
			system(lkk); /*copy log file*/
		}
		#else
		if ((spice==50) && (argc==5) ) { /*Qucs*/
			sprintf(lkk, "copy /y %s.dat %s.dat > NUL", hostname, argv[4]);
			system(lkk); /*copy simulation output file*/
			sprintf(lkk, "copy /y %s.log %s.log > NUL", hostname, argv[4]);
			system(lkk); /*copy log file*/
		}
		#endif
	}


	/**/
	/**/
	if (MPI_EXXIT==0) { /*Cannot print in MPI if Ctrl-C has been pressed because SSH is immediatly killed*/
		printf("INFO:  ASCO has ended on '%s'.\n", hostname);
		fflush(stdout);
	}
	return(EXIT_SUCCESS);
}
