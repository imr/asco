/*
 * Copyright (C) 1999-2011 Joao Ramos
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
#include "auxfunc_updatelis.h"
#include "auxfunc_measurefromlis.h"




/*##############################################################################*/
/*################      M A I N              P R O G R A M      ################*/
/*##############################################################################*/
int main(int argc, char *argv[])
{
	int ii;
	char STR1[LONGSTRINGSIZE];
	FILE *fsweepINI;

/*------Initializations----------------------------*/
	/**/
	/*Step1: Check input arguments */
	if (argc != 4) { /* number of arguments*/
		printf("\nUsage : postp -<simulator> <inputfile>.* <configfile>\n");
		printf("\nExamples:\n");
		printf("          postp -eldo <inputfile>.chi <configfile>\n");
		printf("          postp -hspice <inputfile>.lis <configfile>\n");
		printf("          postp -ltspice <inputfile>.log <configfile>\n");
		printf("          postp -spectre <inputfile>.lis <configfile>\n");		
		printf("\n\n\n");
		exit(EXIT_FAILURE);
	}


	/**/
	/*Step2: Initialization of all variables and strucutres*/
	if (1) { /*is it first time the optimization is runned?*/
		if (*argv[1] == 45) /* 45="-" */
			*argv[1]++;

		spice=0;
		switch(*argv[1]) {
			case 'e': /*Eldo*/
				if (!strcmp(argv[1], "eldo"))
					spice=1;
				break;
			case 'h': /*HSPICE*/
				if (!strcmp(argv[1], "hspice"))
					spice=2;
				break;
			case 'l': /*LTspice*/
				if (!strcmp(argv[1], "ltspice"))
					spice=3;
				break;
			case 's': /*Spectre*/
				if (!strcmp(argv[1], "spectre"))
					spice=4;
				break;
			case 'q': /*Qucs*/
				printf("postp.c -- Unsupport SPICE simulator: %s\n", argv[1]);
				exit(EXIT_FAILURE);
			case 'g': /*general*/
				printf("postp.c -- Unsupport SPICE simulator: %s\n", argv[1]);
				exit(EXIT_FAILURE);
			default:
				printf("postp.c -- Unsupport SPICE simulator: %s\n", argv[1]);
				exit(EXIT_FAILURE);
		}
		if (spice==0) {
			printf("postp.c -- Unsupport SPICE simulator: %s\n", argv[1]);
			exit(EXIT_FAILURE);
		}
	}


	/**/
	/*Step3: */
	if ((fsweepINI=fopen(argv[2],"rt")) == 0) {
		printf("postp.c - Cannot open config file: %s\n", argv[2]);
		exit(EXIT_FAILURE);
	}
	ReadKey(lkk, "CREATE_LJR", fsweepINI);
	if (strcmp((sprintf(STR1, "%.10s", lkk), STR1), "CREATE_LJR")) {
		printf("INFO:  no CREATE_LJR, file will be created by default\n");
		strcpy(lkk, "yes");
	}
	else {
		ii = 1;
		strcpy(lkk, ReadSubKey(STR1, lkk, &ii, ':', ' ', 0));
		Str2Lower(lkk);
		if (strcmp(lkk, "no") && strcmp(lkk, "yes")) {
			printf("postp.c - CREATE_LJR must be `yes` or `no`\n");
			exit(EXIT_FAILURE);
			}
		if (strcmp(lkk, "yes"))
			printf("INFO:  CREATE_LJR, file will be NOT be created\n");
	}
	if (fsweepINI != NULL)
		fclose(fsweepINI);
	fsweepINI = NULL;   /*read createLJR from file*/


	/**/
	/*Step4: */
	if (!strcmp(lkk, "yes"))
		UpdateLIS(argv[3], argv[2]);
	MeasureFromLIS(argv[3], argv[2]);

	if (fsweepINI != NULL)
		fclose(fsweepINI);
	exit(EXIT_SUCCESS);
}
