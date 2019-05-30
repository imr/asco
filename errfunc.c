/*
 * Copyright (C) 2004-2011 Joao Ramos
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
#include <strings.h>
#include <unistd.h>
#include <signal.h>
#ifndef __MINGW32__
#include <sys/wait.h>
#else
#include <winsock2.h>
#endif


#include "auxfunc.h"
#include "auxfunc_alter.h"
#include "auxfunc_monte.h"
#include "initialize.h"
#include "errfunc.h"
#include "auxfunc_measurefromlis.h"
#include "rfmodule.h"








/*
 *  Checks if all constraints are met
 */
int AllConstraintsMet()
{
	int i,ii;

	i=0;
	ii=0;                                                   /*used to know if all constraint are met*/
	while (measurements[i].meas_symbol[0] != '\0') {
		if (measurements[i].objective_constraint > 3) { /*if it is a constraint: LE=4, GE=5, EQ=6*/
			if (measurements[i].constraint_met)
				i++;
			else {
				ii++;                           /*at least one constraint not met*/
				break;
			}
		} else {
			i++;
		}
	}

	if (ii)
		return(0); /*not all constraints are meet*/
	else
		return(1); /*all constraints are meet*/
}




/*
 * Compute cost for the current simulation
 */
double CostFunction()
{
	double cost=0; /*total cost initialized to 0*/
	int i=0;
	double tolerance=0.01; /*0.01=1%; 0.1=10%*/

	if (Wobj <= 0) {
		printf("errfunc.c - CostFunction -- Wobj=%f, should be > 0\n",Wobj);
		exit(EXIT_FAILURE);
	}
	if (Wcon <= 0) {
		printf("errfunc.c - CostFunction -- Wcon=%f, should be > 0\n",Wcon);
		exit(EXIT_FAILURE);
	}
	if (Wobj > Wcon) {
		printf("errfunc.c - CostFunction -- Wobj=%f > Wcon=%f, should be less\n",Wobj, Wcon);
		exit(EXIT_FAILURE);
	}

	while (measurements[i].meas_symbol[0] != '\0') {
		switch (measurements[i].objective_constraint) {
			case 1: /*MIN                                           ===> objective (MIN)*/
				measurements[i].constraint_met=0;                                        /*because it is not a constraint*/
				cost=cost + (Wobj*measurements[i].measured_value);
				break;
			case 2: /*MAX                                           ===> objective (MAX)*/
				measurements[i].constraint_met=0;                                        /*because it is not a constraint*/
				cost=cost + (1/(Wobj*measurements[i].measured_value));
				break;
			case 3: /*MON                                           ===> monitor, do nothing*/
				measurements[i].constraint_met=0;                                        /*because it is not a constraint*/
				break;
			case 4: /*LE                                            ===> constraint (LE)*/
				measurements[i].constraint_met=1;                                        /*assume that the constraint is met*/
				if (measurements[i].measured_value <= measurements[i].constraint_value)
					cost = cost;                                                     /*no penalty*/
				else {
					if (!fcmp(measurements[i].constraint_value, 0)) {                 /*special case when constraint==0*/
						cost=cost + Wcon*fabs((measurements[i].constraint_value  /*penalty=Wcon*/
								- measurements[i].measured_value));
					} else {
						cost=cost + Wcon*fabs((measurements[i].constraint_value  /*penalty=Wcon*/
								- measurements[i].measured_value)
								/ measurements[i].constraint_value);
					}
					measurements[i].constraint_met=0;                                /*constraint not met*/
				}
				break;
			case 5: /*GE                                            ===> constraint (GE)*/
				measurements[i].constraint_met=1;                                        /*assume that the constraint is met*/
				if (measurements[i].measured_value >= measurements[i].constraint_value)
					cost = cost;                                                     /*no penalty*/
				else {
					if (!fcmp(measurements[i].constraint_value, 0)) {                 /*special case when constraint==0*/
						cost=cost + Wcon*fabs((measurements[i].constraint_value  /*penalty=Wcon*/
								- measurements[i].measured_value));
					} else {
						cost=cost + Wcon*fabs((measurements[i].constraint_value  /*penalty=Wcon*/
								- measurements[i].measured_value)
								/ measurements[i].constraint_value);
					}
					measurements[i].constraint_met=0;                                /*constraint not met*/
				}
				break;
			case 6: /*EQ                                            ===> constraint (EQ)*/
				measurements[i].constraint_met=1;                                        /*assume that the constraint is met*/
				if (!fcmp(measurements[i].constraint_value, 0)) {                         /*special case when constraint==0*/
					if ( (measurements[i].measured_value >= (-tolerance)) &&
					     (measurements[i].measured_value <= (+tolerance)) )
						cost = cost;                                             /*no penalty*/
					else {
						cost=cost + Wcon*fabs((measurements[i].constraint_value  /*penalty=Wcon*/
								- measurements[i].measured_value));
						measurements[i].constraint_met=0;                        /*constraint not met*/
					}
				} else {
					if ( (measurements[i].measured_value >= (measurements[i].constraint_value)*(1-tolerance)) &&
					     (measurements[i].measured_value <= (measurements[i].constraint_value)*(1+tolerance)) )
						cost = cost;                                             /*no penalty*/
					else {
						cost=cost + Wcon*fabs((measurements[i].constraint_value  /*penalty=Wcon*/
								- measurements[i].measured_value)
								/ measurements[i].constraint_value);
						measurements[i].constraint_met=0;                        /*constraint not met*/
					}
				}
				break;
			default:
				printf("errfunc.c - CostFunction -- Something unexpected has happened!\n");
				exit(EXIT_FAILURE);
		}
		i++;
	}
	if (cost<0) { /* Minimum numerical possible cost is '0'. MUST exit otherwise! */
		printf("errfunc.c - CostFunction -- Cost cannot be negative!\n");
		exit(EXIT_FAILURE);
	}
	if ((cost<1e-21) && (spice<100)) { /* In SPICE, it is very unlikely that the cost is anywhere close to 0 */
		printf("INFO:  errfunc.c - CostFunction -- Cost is very close to zero. This can either be (a) due to an error or because (b) there is not an objective in category # Measurements # in the *.cfg file.\n");
		fflush(stdout);
	}
	return (cost);
}




/*
 * Log current simulation results to file <hostname>.log
 */
void LogtoFile(double cost)
{
	int i, ii;
	char laux[LONGSTRINGSIZE];

	/*   <hostname>.log*/
	FILE *fspice_log;

	char hostname[SHORTSTRINGSIZE] = "0";
	int  ccode;
	if ((ccode = gethostname(hostname, sizeof(hostname))) != 0) {
		printf("errfunc.c - LogtoFile -- gethostname failed, ccode = %d\n", ccode);
		exit(EXIT_FAILURE);
	}
	/*printf("host name: %s\n", hostname);*/
	ii=strpos2(hostname, ".", 1);
	if (ii)                                 /* hostname is "longmorn.xx.xx.xx" */
		hostname[ii-1]='\0';
	/*sprintf(laux, "%s.log", hostname);*/      /*h ostname is "longmorn" */
	switch(spice) {
		case 1: /*Eldo*/
			sprintf(laux, "%s.log", hostname);      /*hostname is "longmorn"*/
			break;
		case 2: /*HSPICE*/
			sprintf(laux, "%s.log", hostname);      /*hostname is "longmorn"*/
			break;
		case 3: /*LTspice*/
			sprintf(laux, "%s.log.log", hostname);  /*hostname is "longmorn"*/
			break;
		case 4: /*Spectre*/
			sprintf(laux, "%s.log", hostname);      /*hostname is "longmorn"*/
			break;
		case 50: /*Qucs*/
			sprintf(laux, "%s.log", hostname);      /*hostname is "longmorn"*/
			break;
		case 100: /*general*/
			sprintf(laux, "%s.log", hostname);      /*hostname is "longmorn"*/
			break;
		default:
			printf("errfunc.c - LogtoFile -- Something unexpected has happened!\n");
			exit(EXIT_FAILURE);
	}
	if ((fspice_log=fopen(laux,"at")) == 0) {
		printf("errfunc.c - LogtoFile -- Cannot open log file: %s\n", laux);
		exit(EXIT_FAILURE);
	}

	if (AllConstraintsMet())
		fprintf(fspice_log, "+");                   /*1- all constraints were met*/
	else
		fprintf(fspice_log, "-");                   /*1- at least one constraint was not met*/

	fprintf(fspice_log, "cost:%E:    ", cost);          /*2- print 'cost'*/

	i=0;
	while (measurements[i].meas_symbol[0] != '\0') {    /*3- print measurements*/
		sprintf(laux, "%i", i);                    /* to remove integer added in (*) */
		ii=(int)strlen(laux);                      /* to remove integer added in (*) */
		strcpy(laux, measurements[i].meas_symbol); /* to remove integer added in (*) */
		laux[(int)strlen(laux)-ii]='\0';           /* to remove integer added in (*) */

		if (measurements[i].objective_constraint > 3) { /*if it is a constraint: LE=4, GE=5, EQ=6*/
			if (measurements[i].constraint_met)     /*constraint is met*/
				fprintf(fspice_log, "%c", '+');
			else                                    /*constraint is not met*/
				fprintf(fspice_log, "%c", '-');
		}
			fprintf(fspice_log, "%s:%E: ", laux, measurements[i].measured_value);
			i++;
	}

	fprintf(fspice_log, "    ");
	i=0;
	while (parameters[i].symbol[0] != '\0') {           /*4- print parameters*/
		fprintf(fspice_log, "%s:%E:", parameters[i].symbol, parameters[i].value);
		i++;
	}

	fprintf(fspice_log, "\n");                          /*5- the end*/

	fclose(fspice_log); /*might be wise not to be ALWAYS opening and closing this file*/

	i=0;
	while (measurements[i].meas_symbol[0] != '\0') {    /*6- zero all previous measured values*/
		measurements[i].measured_value=0;
		i++;
	}
}




/*
 *
 */
#ifdef ASCO
void WriteToMem(int num_measures)
{
	int i, ii, j;
	char laux[LONGSTRINGSIZE];
	double currentcost;

	i=1; /*It must start with 1*/
	while (measure[i].search[0] != '\0') { /*LTspice special case*/
		switch(spice) {
			case 1: /*Eldo*/
				break;
			case 2: /*HSPICE*/
				break;
			case 3: /*LTspice*/
				/*Str2Lower(laux);*/
				ii=strpos2(measure[i].data, "=", 1);                         /* LTspice measurement format is different than Eldo and HSPICE  */
				if (ii) {                                                    /* so specific code is added in here to cope with the difference */
					strsub(laux, measure[i].data, ii+1, LONGSTRINGSIZE); /*                                                               */
					strcpy(measure[i].data, laux);                       /*                                                               */
					if (strpos2(laux, "dB,", 1)) {                               /*Is the measurement a complex AC signal?*/
						ii=1;
						if (strpos2(measure[i].search, "phase", 1))
							ReadSubKey(laux, measure[i].data, &ii, ',', 0xB0, 0); /*measure the AC phase    */
						else
							ReadSubKey(laux, measure[i].data, &ii, '(', 'd', 0);  /*measure the AC magnitude*/
						strcpy(measure[i].data, laux);
					}
				}
				break;
			case 4: /*Spectre*/
				break;
			case 50: /*Qucs*/
				break;
			case 100: /*general*/
				break;
			default:
				printf("errfunc.c - WriteToMem -- Something unexpected has happened!\n");
				exit(EXIT_FAILURE);
		}
		i++;
	}
	DoMath(num_measures); /*'MATH=&...'; information is in the variable 'measured_data'*/

	i=1; /*It must start with 1*/
	while (measure[i].search[0] != '\0') {
		strcpy(laux, UNIQUECHAR);
		ii=0;
		ii=     strpos2(measure[i].var_name, laux, 1); /*if UNIQUECHAR is inexisting in measure[i].var_name*/
		Str2Lower(laux);
		ii=ii + strpos2(measure[i].var_name, laux, 1); /* and in Str2Lower(UNIQUECHAR) as well, this means we are processing a line with "MEASURE_VAR"*/
		if (ii) {
			strcpy(laux, UNIQUECHAR);
			ii=(int)strlen(laux);
			strsub(laux, measure[i].var_name, ii+1, LONGSTRINGSIZE);             /*1- find output measure variable                                   */
										             /*   by copying the last part of the string starting from UNIQUECHAR*/
			j=0;
			while (strcasecmp(laux, measurements[j].meas_symbol)) {              /*2- get its order in measurements[i]                               */
				if (measurements[j].meas_symbol[0] == '\0') { /*if symbol is not found*/
					printf("errfunc.c - WriteToMem -- Symbol from SPICE simulation not found in memory: %s\n", laux);
					exit(EXIT_FAILURE);
				}
				j++;
			}

			if ((int)strlen(measure[i].data)) {
				strcpy(laux, measure[i].data);                               /*3- read measurement                                               */
				StripSpaces(laux);
			} else {
				#ifdef DEBUG
				i--;                             /* to keep the same code from initialize.c */
				sprintf(lkk, "%i", i);                    /* to remove integer added in (*) */
				ii=(int)strlen(lkk);                      /* to remove integer added in (*) */
				strcpy(lkk, measurements[i].meas_symbol); /* to remove integer added in (*) */
				lkk[(int)strlen(lkk)-ii]='\0';            /* to remove integer added in (*) */

				printf("errfunc.c - WriteToMem -- Data not read for measurement in *.cfg: %s\n", lkk);
				fflush(stdout); /*Sometimes the simulation does not converge, and so do not exit*/

				i++;
				#endif
			}

			measurements[j].measured_value=asc2real(laux, 1, (int)strlen(laux)); /*4- convert it to double                                           */

			if (!fcmp(measurements[j].measured_value, 0)) {                      /*5- check NaN and other text strings                               */
				if ((laux[0] < 43) || (laux[0] > 57) ) /*if its text*/
					if (measurements[j].objective_constraint == 1) /*1=MIN*/
						measurements[j].measured_value=+1e+30; /*so that a large cost is later assigned*/
					if (measurements[j].objective_constraint == 2) /*2=MAX*/
						measurements[j].measured_value=-1e+30; /*so that a large cost is later assigned*/
					if (measurements[j].objective_constraint == 4) /*4=LE*/
						measurements[j].measured_value=+1e+30; /*so that a large cost is later assigned*/
					if (measurements[j].objective_constraint > 4)  /*5=GE, 6=EQ*/
						measurements[j].measured_value=-1e+30; /*so that a large cost is later assigned*/
			}
		}
		i++;
	}

/*----------------------------------------------------------------------------*/
	/**/
	/*Step5: find cost*/
	currentcost=CostFunction();
	if (maxcost<currentcost)
		maxcost=currentcost;


	/**/
	/*Step6: save log information to file <hostname>.log*/
	if (LOG) {
		LogtoFile(currentcost);
	}
/*----------------------------------------------------------------------------*/
}
#endif




/*
 * This is the main function
 *      1: scale input parameters from [-10, +10] to fit within desired range values
 *      2: read from <hostname>.tmp and write to <hostname>.*
 *      3: execute 'SPICE <hostname>.*'
 *      4: read output <hostname>.out
 *      5: find cost
 *      6: save log information to file <hostname>.log
 *      7: ALTER and MC simulation
 *      8: return cost
 */
double errfunc(char *filename, double *x)
{
	/*double currentcost;*/ /*total cost*/
	int i, ii;
	int ccode;
	char laux[LONGSTRINGSIZE], hostname[SHORTSTRINGSIZE] = "0", filename_x[SHORTSTRINGSIZE] = "0";

	/*   <hostname>.*   <hostname>.out  <hostname>.tmp <hostname>.log*/
	FILE *fspice_input, *fspice_output, *fspice_tmp,   *fspice_log;

	/**/
	/*Step1: scale input parameters from [-10, +10] to fit within desired range values*/
	#ifdef DEBUG
	printf("DEBUG: errfunc.c - Step1\n");
	#endif
	for (i = 0; i < MAXPARAMETERS; i++) {
		if (parameters[i].format) {
			parameters[i].value = scaleto(x[i], -10, +10, parameters[i].minimum, parameters[i].maximum, parameters[i].format);
			if (parameters[i].value<0) {
				ii=1;
				/*printf("INFO:  errfunc.c - Step1 -- Negative values\n");*/
			}
		}
	}


	/**/
	/*Step2: read from <hostname>.tmp and write to <hostname>.**/
	/*                                                                                                                   */
	/* It is not necessary to check every time for the possibility to read/write. Move to initicialization if possible!! */
	/*                                                                                                                   */
	#ifdef DEBUG
	printf("DEBUG: errfunc.c - Step2\n");
	#endif
	if ((ccode = gethostname(hostname, sizeof(hostname))) != 0) {
		printf("errfunc.c - Step2 -- gethostname failed, ccode = %d\n", ccode);
		exit(EXIT_FAILURE);
	}
	/* printf("host name: %s\n", hostname); */
	ii=strpos2(hostname, ".", 1);
	if (ii)                                 /* hostname is "longmorn.xx.xx.xx" */
		hostname[ii-1]='\0';
	sprintf(lkk, "%s%s", hostname, ".tmp"); /* hostname is "longmorn" */
	if ((fspice_tmp =fopen(lkk  ,"rt")) == 0) {   /* netlist to simulate given by "hostname" */
		printf("errfunc.c - Step2 -- Cannot read from tmp file: %s\n", lkk);
		exit(EXIT_FAILURE);
	}
	switch(spice) {
		case 1: /*Eldo*/
			sprintf(lkk, "%s%s", hostname, ".cir");
			break;
		case 2: /*HSPICE*/
			sprintf(lkk, "%s%s", hostname, ".sp");
			break;
		case 3: /*LTspice*/
			sprintf(lkk, "%s%s", hostname, ".net");
			break;
		case 4: /*Spectre*/
			sprintf(lkk, "%s%s", hostname, ".scs");
			break;
		case 50: /*Qucs*/
			sprintf(lkk, "%s%s", hostname, ".txt");
			break;
		case 100: /*general*/
			sprintf(lkk, "%s%s", hostname, ".txt");
			break;
		default:
			printf("errfunc.c - Step2 -- Something unexpected has happened!\n");
			exit(EXIT_FAILURE);
	}
	if ((fspice_input =fopen(lkk  ,"wt")) == 0) {   /*netlist to simulate given by 'hostname'*/
		printf("errfunc.c - Step2 -- Cannot write to output file: %s\n", lkk);
		exit(EXIT_FAILURE);
	}

	/*Step2.1: ".end" not yet found*/
	fgets2(lkk, LONGSTRINGSIZE, fspice_tmp);       /*read and*/
	fprintf(fspice_input, "%s\n", lkk); /*write the first line*/
	while (!P_eof(fspice_tmp)) {
		fgets2(lkk, LONGSTRINGSIZE, fspice_tmp);

		strcpy(laux, lkk);           /*detect ".end", ".END", ".End", ... */
		Str2Lower(laux);
		StripSpaces(laux);           /* avoid spaces after the command ".end" */
		if (!strcmp(laux, ".end"))
			break;

		/***** -------------- *********** -------------- *****/
		/***** -------------- ** BEGIN ** -------------- *****/
		if (lkk[0]!='*') {
			i=inlinestrpos(lkk);
			ii=1;
			ReadSubKey(laux, lkk, &ii, '#', '#', 0);
			if ( (laux[0]=='\0') || (ii>(int)strlen(lkk)) || ((i<ii) && (i!=0)) ) { /* does it contains #<text>#?        */
				if ((int)strlen(lkk) && (!RFModule(lkk, 1, fspice_input)) )
					fprintf(fspice_input, "%s\n", lkk);                /*no, write line to <hostname>.*     */
			} else {                                                           /*yes, replace #<text># in this line */
				if (!RFModule(lkk, 1, fspice_input)) {
					ReplaceSymbol(lkk, 1);
					fprintf(fspice_input, "%s\n", lkk); /* write line to <hostname>.* */
				}
			}
		}
		/***** -------------- **  END  ** -------------- *****/
		/***** -------------- *********** -------------- *****/
	}
	switch(spice) {
		case 1: /*Eldo*/
			if (strcmp(laux, ".end")) { /*Exit if ".end" is not found*/
				printf("errfunc.c - Step2.1 -- End not found in %s.cir\n", filename);
				exit(EXIT_FAILURE);
			}
			break;
		case 2: /*HSPICE*/
			if (strcmp(laux, ".end")) { /*Exit if ".end" is not found*/
				printf("errfunc.c - Step2.1 -- End not found in %s.sp\n", filename);
				exit(EXIT_FAILURE);
			}
			break;
		case 3: /*LTspice*/
			if (strcmp(laux, ".end")) { /*Exit if ".end" is not found*/
				printf("errfunc.c - Step2.1 -- End not found in %s.net\n", filename);
				exit(EXIT_FAILURE);
			}
			break;
		case 4: /*Spectre*/                 /* ".end" does not exist in Spectre syntax */
			break;
		case 50: /*Qucs*/                   /* ".end" does not exist in Qucs syntax */
			break;
		case 100: /*general*/
			break;
		default:
			printf("errfunc.c - Step2.1 -- Something unexpected has happened!\n");
			exit(EXIT_FAILURE);
	}


	/*Step2.3: Add ".end" where required*/
	switch(spice) {
		case 1: /*Eldo*/
			fprintf(fspice_input, "%s\n", ".end");
			break;
		case 2: /*HSPICE*/
			fprintf(fspice_input, "%s\n", ".end");
			break;
		case 3: /*LTspice*/
			fprintf(fspice_input, "%s\n", ".end");
			break;
		case 4: /*Spectre*/
			break;
		case 50: /*Qucs*/
			break;
		case 100: /*general*/
			break;
		default:
			printf("errfunc.c - Step2.3 -- Something unexpected has happened!\n");
			exit(EXIT_FAILURE);
	}

	fclose(fspice_input);
	fclose(fspice_tmp);


	/**/
	/*Step3: execute 'SPICE <hostname>.*'*/
	#ifdef DEBUG
	printf("DEBUG: errfunc.c - Step3\n");
	#endif
	switch(spice) {
		case 1: /*Eldo*/
			sprintf(lkk, "nice -n 19 eldo -noconf -i %s.cir > %s.out", hostname, hostname);
			break;
		case 2: /*HSPICE*/
			#ifndef __MINGW32__
			sprintf(lkk, "nice -n 19 hspice -i %s.sp -o %s.lis > /dev/null", hostname, hostname);
			#else
			sprintf(lkk, "hspice -i %s.sp -o %s.lis > NUL", hostname, hostname);
			#endif
			break;
		case 3: /*LTspice*/
			#ifndef __MINGW32__
			sprintf(lkk, "nice -n 19 ltspice -b %s.net > /dev/null", hostname);
			#else
			sprintf(lkk, "ltspice -b %s.net > NUL", hostname);
			#endif
			break;
		case 4: /*Spectre*/
			sprintf(lkk, "nice -n 19 spectremdl -batch %s.mdl -design %s.scs > /dev/null", hostname, hostname);
			break;
		case 50: /*Qucs*/
			#ifndef __MINGW32__
			sprintf(lkk, "nice -n 19 qucsator -i %s.txt -o %s.dat > /dev/null", hostname, hostname);
			#else
			sprintf(lkk, "qucsator -i %s.txt -o %s.dat > NUL", hostname, hostname);
			#endif
			break;
		case 100: /*general*/
			#ifndef __MINGW32__
			sprintf(lkk, "nice -n 19 ./general.sh %s %s", hostname, hostname);
			#else
			sprintf(lkk, "./general.sh %s %s", hostname, hostname);
			#endif
			break;
		default:
			printf("errfunc.c - Step3 -- Something unexpected has happened!\n");
			exit(EXIT_FAILURE);
	}
	ii=system(lkk);
	#ifndef __MINGW32__
	if (WIFSIGNALED(ii) && (WTERMSIG(ii) == SIGINT || WTERMSIG(ii) == SIGQUIT)) {
		printf("errfunc.c - Step3 -- Ctrl-C key pressed. Exiting optimization loop.\n");
		fflush(stdout);
		#ifdef MPI
		/* exit(EXIT_FAILURE); */
		#else
		return(0); /*returned simulation cost is zero; Ctrl-C detection*/
		#endif
	}
	#endif


	/**/
	/*Step4: read output <hostname>.out, find maxcost and save log information to file <hostname>.log*/
	#ifdef DEBUG
	printf("DEBUG: errfunc.c - Step4\n");
	#endif
	switch(spice) {
		case 1: /*Eldo*/
			sprintf(lkk, "%s.out", hostname);
			break;
		case 2: /*HSPICE*/
			sprintf(lkk, "%s.lis", hostname);
			break;
		case 3: /*LTspice*/
			sprintf(lkk, "%s.log", hostname);
			break;
		case 4: /*Spectre*/
			sprintf(lkk, "%s.measure", hostname);
			break;
		case 50: /*Qucs*/
			sprintf(lkk, "%s.dat", hostname);
			break;
		case 100: /*general*/
			sprintf(lkk, "%s.out", hostname);
			break;
		default:
			printf("errfunc.c - Step4 -- Something unexpected has happened!\n");
			exit(EXIT_FAILURE);
	}
	maxcost=0;
	#ifdef DEBUG
	ProcessOutputFile(lkk, 3); /* =>3:mem+file */
	#else
	ProcessOutputFile(lkk, 1); /* =>1:mem      */
	#endif


	/**/
	/*Step5: find cost*/
/*	#ifdef DEBUG						*/
/*	printf("DEBUG: errfunc.c - Step5\n");			*/
/*	#endif							*/
/*	currentcost=CostFunction();				*/
/*	if (maxcost<currentcost)				*/
/*		maxcost=currentcost;				*/


	/**/
	/*Step6: save log information to file <hostname>.log*/
/*	#ifdef DEBUG						*/
/*	printf("DEBUG: errfunc.c - Step6\n");			*/
/*	#endif							*/
/*	if (LOG) {						*/
/*		LogtoFile(currentcost);				*/
/*	}							*/


	/**/
	/*Step7: ALTER and MC simulation*/
	#ifdef DEBUG
	printf("DEBUG: errfunc.c - Step7\n");
	#endif
	if ( (((AllConstraintsMet()) && (AlterMCcost > maxcost)) || (AlterMCcost > maxcost)) && (AlterMC)) {

		switch (AlterMC) {
			case 1: /*Monte Carlo simulation*/
				/* bla bla bla*/
				printf("INFO:  errfunc.c - Step7 -- altermc=%d\n", AlterMC);
				fflush(stdout);

				if (LOG) {
					sprintf(laux, "%s.log", hostname);
					if ((fspice_log=fopen(laux,"at")) == 0) {
						printf("errfunc.c - Step7 -- Cannot open log file: %s\n", laux);
						exit(EXIT_FAILURE);
					}
					fprintf(fspice_log, "Alter: 1 - altermc=%d\n", AlterMC);
					fclose(fspice_log);
				}

strcpy (filename_x, filename);
				strcat(filename_x, ".cfg");
				sprintf(lkk, "%s%s", hostname, ".tmp"); /* hostname is "longmorn"   */
				sprintf(laux, "%s%s", hostname, ".mc"); /* hostname is "longmorn"   */
				MonteCarlo(filename_x, lkk, laux);      /* execute the 'monte' tool */

				sprintf(lkk, "%s%s", hostname, ".mc");  /* hostname is "longmorn"   */
				if ((fspice_input =fopen(lkk  ,"rt")) == 0) {  /*netlist to simulate given by "hostname" */
					printf("errfunc.c - Step7 -- Cannot read from mc file: %s\n", lkk);
					exit(EXIT_FAILURE);
				}
				sprintf(lkk, "%s%s", hostname, ".tmp");
				if ((fspice_output =fopen(lkk  ,"wt")) == 0) { /*netlist to simulate given by 'hostname' */
					printf("errfunc.c - Step7 -- Cannot write to tmp file: %s\n", lkk);
					exit(EXIT_FAILURE);
				}

				while (!P_eof(fspice_input)) { /* reads from <hostname>.mc and writes to <hostname>.tmp */
					fgets2(lkk, LONGSTRINGSIZE, fspice_input); /* read line from <hostname>.mc */
					strcpy(laux, lkk);
					Str2Lower(laux);
					fprintf(fspice_output, "%s\n", lkk);       /* write line to <hostname>.tmp */
				}

				fclose(fspice_input);
				fclose(fspice_output);

				AlterMC=AlterMC-1; /*decrease by one; nothing more will be executed*/
				break;
			case 2: /*Alter simulation - variable 'AlterMC'=2*/
			case 3: /*Alter simulation - variable 'AlterMC'=3 and will execute a MonteCarlo simulation afterwards*/
				/* bla bla bla*/
				printf("INFO:  errfunc.c - Step7 -- altermc=%d\n", AlterMC);
				fflush(stdout);

				if (LOG) {
					sprintf(laux, "%s.log", hostname);
					if ((fspice_log=fopen(laux,"at")) == 0) {
						printf("errfunc.c - Step7 -- Cannot open log file: %s\n", laux);
						exit(EXIT_FAILURE);
					}
					fprintf(fspice_log, "Alter: 2 - altermc=%d\n", AlterMC);
					fclose(fspice_log);
				}

				sprintf(lkk, "%s%s", hostname, ".tmp"); /*hostname is "longmorn"*/
				if ((fspice_tmp =fopen(lkk  ,"r+t")) == 0) { /*netlist to simulate given by "hostname"*/
					printf("errfunc.c - Step7 -- Cannot read from tmp file: %s\n", lkk);
					exit(EXIT_FAILURE);
				}
				if ((fspice_log =fopen("alter.inc" ,"rt")) == 0) { /*If file 'alter.inc' does not exist  */
strcpy (filename_x, filename);
					strcat(filename_x, ".cfg");
					CreateALTERinc(filename_x, lkk, 1); /*execute the 'alter' tool*/
					fseek(fspice_tmp, 0, SEEK_END); /*properly position the pointer*/
				} else { /* file 'alter.inc', so use it instead of the 'alter' tool */
					#ifndef __MINGW32__
					fseek(fspice_tmp, -5, SEEK_END); /*properly position the pointer*/
					#else
					fseek(fspice_tmp, -6, SEEK_END); /*properly position the pointer*/
					#endif
					fprintf(fspice_tmp, ".INCLUDE alter.inc\n");  /*write*/
				}
				/* fseek(fspice_tmp, 0, SEEK_END); */ /*properly position the pointer*/
				fprintf(fspice_tmp, ".end\n");  /*add ".end"*/
				fclose(fspice_tmp);

				AlterMC=AlterMC-2; /*decrease by two; it will not execute a Monte Carlo simulation next time if equal to zero*/
				break;
			default:
				printf("errfunc.c - Step7 -- Something unexpected has happened!\n");
				exit(EXIT_FAILURE);
		}
	}


	/**/
	/*Step8: return maximum cost (ALTER on MONTECARLO)*/
	#ifdef DEBUG
	printf("DEBUG: errfunc.c - Step8\n");
	#endif
	return (maxcost);
}
