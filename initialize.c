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
#include "auxfunc_measurefromlis.h"
#include "rfmodule.h"




/*
 * receives a number between [ina, inb] and scales it to be within [outa, outb]
 */
double scaleto(double value, double ina, double inb, double outa, double outb, int format)
{
	double result;
	int ii;

	switch (format) {
		case 1: /*LIN_DOUBLE*/
			result = ( (outb-outa)*value/(inb-ina)  +  (outa*inb - ina*outb)/(inb-ina) );
			break;

		case 2: /*LIN_INT*/
			outa=floor(outa); /*Convert to INTEGER, just in case*/
			outb=ceil(outb);  /*Convert to INTEGER, just in case*/
			outb++;
			result = ( (outb-outa)*value/(inb-ina)  +  (outa*inb - ina*outb)/(inb-ina) );
			result = floor(result); /*Convert final result to INTEGER*/
			break;

		case 3: /*LOG_DOUBLE*/
			printf("initialize.c - scaleto -- LOG_DOUBLE not yet implemented!\n");
			exit(EXIT_FAILURE);

		case 4: /*LOG_INT*/
			printf("initialize.c - scaleto -- LOG_INT not yet implemented!\n");
			exit(EXIT_FAILURE);

		default:
			printf("initialize.c - scaleto -- Something unexpected has happened!\n");
			exit(EXIT_FAILURE);
	}

	if (result<0) {
		ii=1;
		/*printf("errfunc.c - scaleto -- Error: negative values\n");*/
	}
		
	return(result);
}




/*
 * receives a SPICE string from the <inputfile>.* with #<text># and replaces
 * it with the correct parameters. Depending on the optimize variable value:
 *      optimize=0 : we are initializing, just replace fixed variables
 *      optimize=1 : "   "  optimizing, replace remaining optimizing variables
 */
char *ReplaceSymbol(char *ret, int optimize)
{
	int i, k, ii;
	char laux[LONGSTRINGSIZE], inlinecomment[LONGSTRINGSIZE], lxp[LONGSTRINGSIZE];

	k=inlinestrpos(ret);
	inlinecomment[0]='\0';
	if (k) {
		strsub(inlinecomment, ret, k, strlen(ret)); /*copies the in-line comment*/
		ret[k-1]='\0';
	}

	ii=1;
	ReadSubKey(laux, ret, &ii, '#', '#', 5);
	while (ii<=(strlen(ret))) {
		/*Str2Lower(laux);*/
		i=0;
		while (strcmp(parameters[i].symbol, laux)) {
			if (parameters[i].symbol[0] == '\0') { /*if symbol is not found*/
				printf("initialize.c - ReplaceSymbol -- Symbol in <inputfile>.* not found in <inputfile>.cfg: %s\n", laux);
				exit(EXIT_FAILURE);
			}
			i++;
		}

		strsub(laux, ret, ii+1, strlen(ret)-ii);     /*copies the last part of the string to laux*/
		/*ret[strpos2(ret, "#", 1)-1]='\0';*/        /*properly finishes the string*/
		ret[ii-strlen(parameters[i].symbol)-2]='\0'; /*properly finishes the string*/

		if (optimize==0) { /*optimize=0 : we are initializing*/
			if (parameters[i].optimize==0)
				sprintf(lxp, "%E", parameters[i].value);  /*writes the value*/
			else
				sprintf(lxp, "#%s#",parameters[i].symbol); /*writes the #<symbol>#  back again*/
		} else {           /*optimize=1 : we are optimizing*/
			sprintf(lxp, "%E", parameters[i].value);          /*writes the value*/
		}
		strcat(ret, lxp);
		strcat(ret, laux);
		ii++; /*because 'ReadSubKey' starts at [ii-1] instead of [ii]*/
		ReadSubKey(laux, ret, &ii, '#', '#', 0);
	}

	strcat(ret, inlinecomment); /*concatenates the in-line comment*/
	return ret;
}




/*
 * receives a string from the extract/ directory with #SYMBOL# and/or #NODE# text
 * requiring a measurement extraction and replaces it with the appropriate value
 */
char *DecodeSymbolNode(char *ret, int i)
{
	int ii;
	char laux[LONGSTRINGSIZE];
	
	ii=1;
	ReadSubKey(laux, lkk, &ii, '#', '#', 0);
	while (ii<=strlen(lkk)) {
		Str2Lower(laux);
		if (!strcmp(laux, "symbol"))
			ii=ii+1000;                   /*is a "symbol" and encode this information by adding '1000'*/
		else {
			if (strcmp(laux, "node")) {   /*if it is not "node" then exit*/
				printf("initialize.c - DecodeSymbolNode -- Unrecognized option: %s\n", laux);
				exit(EXIT_FAILURE);
			}
		}

		if (ii>1000)
			strsub(laux, lkk, ii-1000+1, strlen(lkk)-(ii-1000)); /*copies the last part of the string to laux*/
		else
			strsub(laux, lkk, ii+1, strlen(lkk)-ii);             /*copies the last part of the string to laux*/
		lkk[strpos2(lkk, "#", 1)-1]='\0';                            /*properly finishes string                  */
		if (ii>1000) {
			ii=ii-1000;
			strcat(lkk, UNIQUECHAR);                  /* unique sequence added to the symbol*/
			strcat(lkk, measurements[i].meas_symbol); /* adds the symbol or... */
			/* strcat(lkk, " "); not necessary anymore (*) */
		} else
			strcat(lkk, measurements[i].node);        /* ...adds the node       */

		strcat(lkk, laux);
		ii=1;
		ReadSubKey(laux, lkk, &ii, '#', '#', 0);
	}

	strcpy (ret, lkk);
	return ret;
}




/*
 * initialize variables and necessary files to optimize the file in the variable "filename"
 *      1: Optimization Flow: should Alter and/or MonteCarlo be done?
 *      2: read "# Parameters #"
 *      3: read "# Measurements #"
 *      4: create <hostname>.tmp file; add measurements and replace symbols
 *      5: erase from memory parameters that will not be optimized
 *      6: use initial values stored in the <inputfile>.cfg file
 */
int initialize(char *filename) /* , double *x) */
{
	int i, ii, ccode;;
	char laux[LONGSTRINGSIZE], laux2[SHORTSTRINGSIZE], hostname[SHORTSTRINGSIZE] = {0};

	/*   <inputfile>.*   <inputfile>.cfg <hostname>.tmp /extract/<file> */
	FILE *fspice_source, *fspice_cfg,    *fspice_tmp,   *fextract;

	/**/
	/*Step1: Optimization Flow: should Alter and/or MonteCarlo be done?*/
	sprintf(laux, "%s%s", filename, ".cfg");
	if ((fspice_cfg =fopen(laux ,"rt")) == 0) {
		printf("initialize.c - Step1 -- Cannot open config file: %s\n", laux);
		exit(EXIT_FAILURE);
	}
	ReadKey(lkk, "#Optimization Flow#", fspice_cfg);
	if (strcmp(lkk, "#Optimization Flow#")) {
		printf("INFO:  initialize.c - Step1 -- #Optimization Flow# key not found\n");
	} else {
		fgets2(lkk, LONGSTRINGSIZE, fspice_cfg);      /*should Alter be done?*/
		Str2Lower(lkk);
		ii=1;
		ReadSubKey(laux, lkk, &ii, ':', ' ', 4);
		if (!strcmp(laux, "yes"))               /* Alter==yes */
			AlterMC+=2;
		else {
			if (strcmp(laux, "no")) {       /* Alter!=no  */
				printf("initialize.c - Step1 -- Incorrect line format: %s\n", lkk);
				exit(EXIT_FAILURE);
			}
		}

		fgets2(lkk, LONGSTRINGSIZE, fspice_cfg);      /*should MonteCarlo be done?*/
		Str2Lower(lkk);
		ii=1;
		ReadSubKey(laux, lkk, &ii, ':', ' ', 4);
		if (!strcmp(laux, "yes"))               /*MonteCarlo==yes*/
			AlterMC+=1;
		else {
			if (strcmp(laux, "no")) {       /*MonteCarlo!=no*/
				printf("initialize.c - Step1 -- Incorrect line format: %s\n", lkk);
				exit(EXIT_FAILURE);
			}
		}

		fgets2(lkk, LONGSTRINGSIZE, fspice_cfg);
		ii=1;
		ReadSubKey(laux, lkk, &ii, ':', ' ', 4);
		AlterMCmincost=asc2real(laux, 1, (int)strlen(laux));
		if (AlterMCmincost < 0) {
			printf("initialize.c - Step1 -- Minumum cost=%f, should be >= 0\n", AlterMCmincost);
		exit(EXIT_FAILURE);
		}
		#ifdef DEBUG
		AlterMCmincost=1.7976931348623157e+308; /*DBL_MAX from <float.h>*/
		#endif
	}


	/**/
	/*Step2: read "# Parameters #"*/
	fseek(fspice_cfg, 0, SEEK_SET);
	ReadKey(lkk, "# Parameters #", fspice_cfg);   /*configuration parameters*/
	if (strcmp(lkk, "# Parameters #")) {
		printf("INFO:  initialize.c - Step2 -- No parameters in config file\n");
	} else {
		i=0;
		fgets2(lkk, LONGSTRINGSIZE, fspice_cfg);
		while ((lkk[0] != '#') && (lkk[0] != '\0') && (ii=strpos2(lkk, ":", 1)) && (!feof(fspice_cfg))) {
			if (lkk[0] != '*') {
				if (lkk[ii-1] != ':') {
					printf("initialize.c - Step2 -- Incorrect line format: %s\n", lkk);
					exit(EXIT_FAILURE);
				}
				strsub(parameters[i].name, lkk, 1, ii-1);                   /*name    */

				ReadSubKey(parameters[i].symbol, lkk, &ii, '#', '#', 5);    /*symbol  */

				ReadSubKey(laux, lkk, &ii, ':', ':', 5);
				parameters[i].value=asc2real(laux, 1, (int)strlen(laux));   /*value   */

				ReadSubKey(laux, lkk, &ii, ':', ':', 5);
				parameters[i].minimum=asc2real(laux, 1, (int)strlen(laux)); /*minimum */

				ReadSubKey(laux, lkk, &ii, ':', ':', 5);
				parameters[i].maximum=asc2real(laux, 1, (int)strlen(laux)); /*maximum */
				if (parameters[i].minimum > parameters[i].maximum) { /*just to help*/
					printf("initialize.c - Step2 -- Minimum is larger than Maximum in line: %s\n", lkk);
					exit(EXIT_FAILURE);
				}

				ReadSubKey(laux, lkk, &ii, ':', ':', 5);                    /*format  */
				parameters[i].format=-1;        /*number '-1' is used for posterior value validation*/
				strsub(laux2, laux, 1, 3);
				if (!strcmp(laux2, "LIN"))
					parameters[i].format=0;
				if (!strcmp(laux2, "LOG"))
					parameters[i].format=2;
				if (parameters[i].format==-1) { /*validation*/
					printf("initialize.c - Step2 -- Unrecognized option: %s\n", laux);
					exit(EXIT_FAILURE);
				}
				strsub(laux2, laux, 5, strlen(laux));
				if (!strcmp(laux2, "DOUBLE"))
					parameters[i].format=parameters[i].format+1;
				if (!strcmp(laux2, "INT"))
					parameters[i].format=parameters[i].format+2;

				if (parameters[i].format==0) {  /*validation*/
					printf("initialize.c - Step2 -- Unrecognized option: %s\n", laux);
					exit(EXIT_FAILURE);
				}

				ReadSubKey(laux, lkk, &ii, ':', ':', 4);                    /*optimize */
				if (!strcmp(laux, "OPT"))
					parameters[i].optimize=1;                           /*is it "OPT"?                                    */
				else {
					if (!strcmp(laux, "---")) {                         /*if it is "---", then                            */
						parameters[i].optimize=0;                   /*do not optimize                                 */
						parameters[i].minimum=parameters[i].value;  /*furthermore, if it is just to define a quantity */
						parameters[i].maximum=parameters[i].value;  /*then make minimum=maximum=value                 */
					} else {
						if (strcmp(laux, "node")) {                /*if it is not "node" then exit                    */
							printf("initialize.c - Step2 -- Unrecognized option: %s\n", laux);
							exit(EXIT_FAILURE);
						}
					}
				}

				i++;
				if (i > MAXPARAMETERS) {
					printf("initialize.c - Step2 -- Maximum number of parameter exceeded (>%d)\n",MAXPARAMETERS);
					exit(EXIT_FAILURE);
				}
			}
			fgets2(lkk, LONGSTRINGSIZE, fspice_cfg);

		}
	}


	/**/
	/*Step3: read "# Measurements #"*/
	fseek(fspice_cfg, 0, SEEK_SET);
	ReadKey(lkk, "# Measurements #", fspice_cfg); /*configuration measurements*/
	if (strcmp(lkk, "# Measurements #")) {
		printf("INFO:  initialize.c - Step3 -- No measurements in config file\n");
	} else {
		i=0;
		fgets2(lkk, LONGSTRINGSIZE, fspice_cfg);
		while ((lkk[0] != '#') && (lkk[0] != '\0') && (ii=strpos2(lkk, ":", 1)) && (!feof(fspice_cfg))) {
			if (lkk[0] != '*') {
				if (lkk[ii-1] != ':') {
					printf("initialize.c - Step3 -- Incorrect line format: %s\n", lkk);
					exit(EXIT_FAILURE);
				}
				strsub(measurements[i].meas_symbol, lkk, 1, ii-1);                     /*meas_symbol*/
				sprintf(laux, "%i", i);
				strcat(measurements[i].meas_symbol, laux); /* add number to symbol (*) */

				ReadSubKey(measurements[i].node, lkk, &ii, ':', ':', 5);               /*node*/

				ReadSubKey(laux, lkk, &ii, ':', ':', 5);                               /*objective_constraint*/
				measurements[i].objective_constraint=0;        /*number '0' is used for posterior value validation*/
				if (!strcmp(laux, "MIN"))
					measurements[i].objective_constraint=1;
				if (!strcmp(laux, "MAX"))
					measurements[i].objective_constraint=2;
				if (!strcmp(laux, "MON"))
					measurements[i].objective_constraint=3;
				if (!strcmp(laux, "LE"))
					measurements[i].objective_constraint=4;
				if (!strcmp(laux, "GE"))
					measurements[i].objective_constraint=5;
				if (!strcmp(laux, "EQ"))
					measurements[i].objective_constraint=6;
				if (measurements[i].objective_constraint==0) { /*validation*/
					printf("initialize.c - Step3 -- Unrecognized option: %s\n", laux);
					exit(EXIT_FAILURE);
				}

				ReadSubKey(laux, lkk, &ii, ':', ':', 4);
				measurements[i].constraint_value=asc2real(laux, 1, (int)strlen(laux)); /*constraint_value*/

				i++;
				if (i > MAXMEASUREMENTS) {
					printf("initialize.c - Step3 -- Maximum number of measurements exceeded (>%d)\n",MAXMEASUREMENTS);
					exit(EXIT_FAILURE);
				}
			}
			fgets2(lkk, LONGSTRINGSIZE, fspice_cfg);
		}
		measurements[i].meas_symbol[0]='\0'; /*just in case... zero the first remaining entry*/
	}
	fclose(fspice_cfg);


	/**/
	/*Step4: create <hostname>.tmp file; add measurements and replace symbols*/
	switch(spice) {
		case 1: /*Eldo*/
			sprintf(laux, "%s%s", filename, ".cir");
			break;
		case 2: /*HSPICE*/
			sprintf(laux, "%s%s", filename, ".sp");
			break;
		case 3: /*LTSpice*/
			sprintf(laux, "%s%s", filename, ".net");
			break;
		case 4: /*Spectre*/
			sprintf(laux, "%s%s", filename, ".scs");
			break;
		case 100: /*rosen*/
			sprintf(laux, "%s%s", filename, ".dat");
			break;
		default:
			printf("initialize.c - Step4 -- Something unexpected has happened!\n");
			exit(EXIT_FAILURE);
	}
	if ((fspice_source=fopen(laux, "rt")) == 0) { /*source netlist*/
		printf("initialize.c - Step4 -- Cannot open input file: %s\n", laux);
		exit(EXIT_FAILURE);
	}
	/**/
	if ((ccode = gethostname(hostname, sizeof(hostname))) != 0) { /* !=0 can most likelly be deleted from all lines */
		printf("initialize.c - Step4 -- gethostname failed, ccode = %d\n", ccode);
		exit(EXIT_FAILURE);
	}
	/* printf("host name: %s\n", hostname); */
	ii=strpos2(hostname, ".", 1);
	if (ii) {                               /* hostname is "longmorn.xx.xx.xx" */
		hostname[ii-1]='\0';
	}
	sprintf(lkk, "%s%s", hostname, ".tmp"); /* hostname is "longmorn" */
	if ((fspice_tmp =fopen(lkk  ,"wt")) == 0) { /* netlist to simulate given by "hostname" */
		printf("initialize.c - Step4 -- Cannot write to tmp file: %s\n", lkk);
		exit(EXIT_FAILURE);
	}

	fgets2(lkk, LONGSTRINGSIZE, fspice_source);  /*read and             */
	fprintf(fspice_tmp, "%s\n", lkk);            /*write the first line */
	while (!P_eof(fspice_source)) {
		fgets2(lkk, LONGSTRINGSIZE, fspice_source);

		strcpy(laux, lkk);           /*detect ".end", ".END", ".End", ... */
		Str2Lower(laux);
		StripSpaces(laux);           /* avoid spaces after the command ".end" */
	/*Step4.1: ".end" not yet found*/
		if (strcmp(laux, ".end")) {

			/***** -------------- *********** -------------- *****/
			/***** -------------- ** BEGIN ** -------------- *****/
			if (lkk[0]!='*') {
				i=inlinestrpos(lkk);
				ii=1;
				ReadSubKey(laux, lkk, &ii, '#', '#', 0);
				if ( (laux[0]=='\0') || (ii>strlen(lkk)) || ((i<ii) && (i!=0)) ) { /*does it contains #<text>#?         */
					if (strlen(lkk) && (!RFModule(lkk, 0, fspice_tmp)) )
						fprintf(fspice_tmp, "%s\n", lkk);                  /* no: write line to <hostname>.tmp   */
				} else {                                                           /* yes: replace #<text># in this line */
					if (!RFModule(lkk, 0, fspice_tmp)) {
						ReplaceSymbol(lkk, 0);
						fprintf(fspice_tmp, "%s\n", lkk); /* write line to <hostname>.tmp */
					}
				}
			}
			/***** -------------- **  END  ** -------------- *****/
			/***** -------------- *********** -------------- *****/
		}
	}
	switch(spice) {
		case 1: /*Eldo*/
			if (strcmp(laux, ".end")) { /*Exit if ".end" is not found*/
				printf("initialize.c - Step4.1 -- End not found in <inputfile>.*\n");
				exit(EXIT_FAILURE);
			}
			break;
		case 2: /*HSPICE*/
			if (strcmp(laux, ".end")) { /*Exit if ".end" is not found*/
				printf("initialize.c - Step4.1 -- End not found in <inputfile>.*\n");
				exit(EXIT_FAILURE);
			}
			break;
		case 3: /*LTSpice*/
			if (strcmp(laux, ".end")) { /*Exit if ".end" is not found*/
				printf("initialize.c - Step4.1 -- End not found in <inputfile>.*\n");
				exit(EXIT_FAILURE);
			}
			break;
		case 4: /*Spectre*/                 /* ".end" does not exist in Spectre syntax */
			break;
		case 100: /*rosen*/
			break;
		default:
			printf("initialize.c - Step4.1 -- Something unexpected has happened!\n");
			exit(EXIT_FAILURE);
	}
	/*Special case to deal with Spectre MDL*/
	switch(spice) {
		case 1: /*Eldo*/
			break;
		case 2: /*HSPICE*/
			break;
		case 3: /*LTSpice*/
			break;
		case 4: /*Spectre*/
			fclose(fspice_tmp);
			sprintf(lkk, "%s%s", hostname, ".mdl"); /* hostname is "longmorn" */
			if ((fspice_tmp =fopen(lkk  ,"wt")) == 0) { /* netlist to simulate given by "hostname" */
				printf("initialize.c - Step4.1 -- Cannot write to tmp file: %s\n", lkk);
				exit(EXIT_FAILURE);
			}
			fseek(fspice_source, 0, SEEK_SET);
			while (!P_eof(fspice_source)) {
				fgets2(lkk, LONGSTRINGSIZE, fspice_source);
				if ( (lkk[0] != '*') && (lkk[0] != '\0') && (!strpos2(lkk, "//", 1)) ) {
					strcpy(laux, lkk);           /*detect "TRAN", "tran", "TRan", ... */
					Str2Lower(laux);
					StripSpaces(laux);           /*avoid spaces after the command*/

					if (strpos2(laux, " dc ", 1)) {
						fprintf(fspice_tmp, "alias measurement dc_run {\n");
						i=strpos2(laux, " dc ", 1);
						strsub(laux, lkk, 1, i);
						StripSpaces(laux);
						fprintf(fspice_tmp, "run %s\n", laux);
					}
					if (strpos2(laux, " ac ", 1)) {
						fprintf(fspice_tmp, "alias measurement ac_run {\n");
						i=strpos2(laux, " ac ", 1);
						strsub(laux, lkk, 1, i);
						StripSpaces(laux);
						fprintf(fspice_tmp, "run %s\n", laux);
					}
					if (strpos2(laux, " tran ", 1)) {
						fprintf(fspice_tmp, "alias measurement tran_run {\n");
						i=strpos2(laux, " tran ", 1);
						strsub(laux, lkk, 1, i);
						StripSpaces(laux);
						fprintf(fspice_tmp, "run %s\n", laux);
					}
				}
			}
			break;
		case 100: /*rosen*/
			break;
		default:
			printf("initialize.c - Step4.1 -- Something unexpected has happened!\n");
			exit(EXIT_FAILURE);
	}
	/*Special case to deal with Spectre MDL*/

	/*Step4.2: End of file is found and now add measurements*/
	i=0;
	fprintf(fspice_tmp, "\n");
	while (strcmp(measurements[i].meas_symbol,"\0") ) {  /*until the end of all symbols*/
		sprintf(lkk, "%i", i);                                         /* to remove integer added in (*) */
		ii=strlen(lkk);                                                /* to remove integer added in (*) */
		sprintf(lkk, "%s%s", "extract/", measurements[i].meas_symbol); /* to remove integer added in (*) */
		lkk[strlen(lkk)-ii]='\0';                                      /* to remove integer added in (*) */
		if ((fextract =fopen(lkk ,"rt")) == 0) {
			printf("initialize.c - Step4.2 -- Cannot find measurement file: %s\n", lkk);
			exit(EXIT_FAILURE);
		}

		ReadKey(lkk, "# Commands #", fextract);
		if (strcmp(lkk, "# Commands #")) {            /*finds "# Commands #" and writes to <hostname>.tmp until the end of file*/
			printf("initialize.c - Step4.2 -- Wrong format in file: %s\n", measurements[i].meas_symbol);
			exit(EXIT_FAILURE);
		}

	/*Step4.2.1: Add the measurement to <hostname>.tmp*/
	/*---------------------------------------------------------------*/
		sprintf(lkk, "%i", i);                    /* to remove integer added in (*) */
		ii=strlen(lkk);                           /* to remove integer added in (*) */
		strcpy(lkk, measurements[i].meas_symbol); /* to remove integer added in (*) */
		lkk[strlen(lkk)-ii]='\0';                 /* to remove integer added in (*) */
		switch(spice) {
			case 1: /*Eldo*/
  				fprintf(fspice_tmp, "* %i) Extract \'%s\'\n", i+1, lkk);
				break;
			case 2: /*HSPICE*/
  				fprintf(fspice_tmp, "* %i) Extract \'%s\'\n", i+1, lkk);
				break;
			case 3: /*LTSpice*/
  				fprintf(fspice_tmp, "* %i) Extract \'%s\'\n", i+1, lkk);
				break;
			case 4: /*Spectre*/
  				fprintf(fspice_tmp, "// %i) Extract \'%s\'\n", i+1, lkk);
				break;
			case 100: /*rosen*/
  				fprintf(fspice_tmp, "* %i) Extract \'%s\'\n", i+1, lkk);
				break;
			default:
				printf("initialize.c - Step4.2.1 -- Something unexpected has happened!\n");
				exit(EXIT_FAILURE);
		}
		fgets2(lkk, LONGSTRINGSIZE, fextract); /* reads from directory "extract/" */
		while ((lkk[0] != '#') && (lkk[0] != '\0') && (!feof(fextract))) {
			switch(spice) {
				case 1: /*Eldo*/
					strcpy(laux,lkk);
					StripSpaces(laux);
					Str2Lower(laux);
					if (strpos2(laux, ".meas ", 1)) {
						sprintf(lkk, "%i", i);                                         /* to remove integer added in (*) */
						ii=strlen(lkk);                                                /* to remove integer added in (*) */
						sprintf(lkk, "%s%s", "extract/", measurements[i].meas_symbol); /* to remove integer added in (*) */
						lkk[strlen(lkk)-ii]='\0';                                      /* to remove integer added in (*) */
						printf("initialize.c - Step4.2.1 -- .MEAS not supported in file: %s. Use .EXTRACT instead.\n", lkk);
						exit(EXIT_FAILURE);
					}
					break;
				case 2: /*HSPICE*/
					break;
				case 3: /*LTSpice*/
					break;
				case 4: /*Spectre*/
					break;
				case 100: /*rosen*/
					break;
				default:
					printf("initialize.c - Step4.2.1 -- Something unexpected has happened!\n");
					exit(EXIT_FAILURE);
			}

			ii=1;
			ReadSubKey(laux, lkk, &ii, '#', '#', 0);
			if (laux[0]!='\0') {
				DecodeSymbolNode(lkk, i); /*has to replace #<text># in this line*/
			}
			if (lkk[0]!='#') { /*if end of block has not been reached*/
				fprintf(fspice_tmp, "%s\n", lkk); /* writes to <hostname>.tmp */
			}
			fgets2(lkk, LONGSTRINGSIZE, fextract); /*reads from directory "extract/"*/
		}
		fprintf(fspice_tmp, "\n");
	/*Step4.2.2: Add entry to variable 'measure[i].var_name' and 'measure[i].search'*/
	/*---------------------------------------------------------------*/
		fseek(fextract, 0, SEEK_SET);
		ReadKey(lkk, "MEASURE_VAR", fextract);
		ii=1;
		while ((*measure[ii].var_name) != '\0') /* finds the proper entry */
			ii++;                           /* place. Store in 'ii'   */

		if (strcmp((sprintf(laux, "%.11s", lkk), laux), "MEASURE_VAR")) { /*general case, if it's not a "MEASURE_VAR"; data from "/extract/<file>"*/
			sprintf(lkk, "%s%s", UNIQUECHAR,measurements[i].meas_symbol);
			strcpy(measure[ii].var_name, lkk); /*measure[ii].var_name*/

			switch(spice) { /* the format in which the variables are written in the output file: fast read */
				case 1: /*Eldo*/
					sprintf(lkk, " %s%s =", UNIQUECHAR, measurements[i].meas_symbol);
					Str2Upper(lkk);
					break;
				case 2: /*HSPICE*/
					sprintf(lkk, " %s%s=", UNIQUECHAR, measurements[i].meas_symbol);
					Str2Lower(lkk);
					break;
				case 3: /*LTSpice*/
					sprintf(lkk, "%s%s:", UNIQUECHAR, measurements[i].meas_symbol);
					Str2Lower(lkk);
					break;
				case 4: /*Spectre*/
					sprintf(lkk, "%s%s", UNIQUECHAR, measurements[i].meas_symbol);
					ccode=strlen(lkk);
					while (ccode<18) {
						strcat(lkk, " ");
						ccode++;
					}
					strcat(lkk, " =");
					break;
				case 100: /*rosen*/
					break;
				default:
					printf("initialize.c - Step4.2.2 -- Something unexpected has happened!\n");
					exit(EXIT_FAILURE);
			}
			strcpy(measure[ii].search, lkk);   /*measure[ii].var_name*/
		} else {                                                          /*a line with "MEASURE_VAR" exist in "/extract/<file>"*/
			while (!strcmp((sprintf(laux, "%.11s", lkk), laux), "MEASURE_VAR")) {
				DecodeSymbolNode(lkk, i);
				ii=ProcessMeasureVar(lkk, ii, "/dev/null");
				ii++;
				ReadKey(lkk, "MEASURE_VAR", fextract);
			}
		}
	/*---------------------------------------------------------------*/
	/*Step4.2.3: Continues ... */
		fclose(fextract);
		i++;
	}

	/*Step4.3: Add ".end" where required*/
	switch(spice) {
		case 1: /*Eldo*/
			fprintf(fspice_tmp, "%s\n", ".end");
			break;
		case 2: /*HSPICE*/
			fprintf(fspice_tmp, "%s\n", ".end");
			break;
		case 3: /*LTSpice*/
			fprintf(fspice_tmp, "%s\n", ".end");
			break;
		case 4: /*Spectre*/
	/*Special case to deal with Spectre MDL*/
			ccode=0; /* at this momment, only one measurement can exist in the <inputfile>.scs */
			fseek(fspice_source, 0, SEEK_SET);
			fprintf(fspice_tmp, "}\n\n");
			while (!P_eof(fspice_source)) {
				fgets2(lkk, LONGSTRINGSIZE, fspice_source);
				if ( (lkk[0] != '*') && (lkk[0] != '\0') && (!strpos2(lkk, "//", 1)) ) {
					strcpy(laux, lkk);           /*detect "TRAN", "tran", "TRan", ... */
					Str2Lower(laux);
					StripSpaces(laux);           /*avoid spaces after the command*/

					if (strpos2(laux, " dc ", 1)) {
						if (ccode!=0) {
							printf("initialize.c - Step4.3 -- Only one type of simulation is implememted at this time!\n");
							exit(EXIT_FAILURE);
						}
						ccode++;
						i=strpos2(laux, " dc ", 1);
						strsub(laux, lkk, 1, i);
						StripSpaces(laux);
						fprintf(fspice_tmp, "run dc_run as dc1\n");
					}
					if (strpos2(laux, " ac ", 1)) {
						if (ccode!=0) {
							printf("initialize.c - Step4.3 -- Only one type of simulation is implememted at this time!\n");
							exit(EXIT_FAILURE);
						}
						ccode++;
						i=strpos2(laux, " ac ", 1);
						strsub(laux, lkk, 1, i);
						StripSpaces(laux);
						fprintf(fspice_tmp, "run ac_run as ac1\n");
					}
					if (strpos2(laux, " tran ", 1)) {
						if (ccode!=0) {
							printf("initialize.c - Step4.3 -- Only one type of simulation is implememted at this time!\n");
							exit(EXIT_FAILURE);
						}
						ccode++;
						i=strpos2(laux, " tran ", 1);
						strsub(laux, lkk, 1, i);
						StripSpaces(laux);
						fprintf(fspice_tmp, "run tran_run as tran1\n");
					}
				}
			}
	/*Special case to deal with Spectre MDL*/
			break;
		case 100: /*rosen*/
			break;
		default:
			printf("initialize.c - Step4.3 -- Something unexpected has happened!\n");
			exit(EXIT_FAILURE);
	}

	fclose(fspice_source);
	fclose(fspice_tmp);


	/**/
	/*Step5: erase from memory parameters that will not be optimized*/
	ii=0;
	for (i = 0; i < MAXPARAMETERS; i++) {
		if ((parameters[i].optimize == 1) && (i>ii)) { /*if ... then move*/
			strcpy (parameters[ii].name, parameters[i].name);
			strcpy (parameters[ii].symbol,parameters[i].symbol);
			parameters[ii].value    = parameters[i].value;
			parameters[ii].minimum  = parameters[i].minimum;
			parameters[ii].maximum  = parameters[i].maximum;
			parameters[ii].format   = parameters[i].format;
			parameters[ii].optimize = parameters[i].optimize;
			ii++;
		} else
			if (parameters[i].optimize)
				ii++;
	}
	for (i = ii; i < MAXPARAMETERS; i++) { /*just in case... zero all remaining entries*/
		parameters[i].name[0]  ='\0';
		parameters[i].symbol[0]='\0';
		parameters[i].value    = 0;
		parameters[i].minimum  = 0;
		parameters[i].maximum  = 0;
		parameters[i].format   = 0;
		parameters[i].optimize = 0;
	}


	/**/
	/*Initialization6: use initial values stored in the <inputfile>.cfg file                                 */
	/*for (ii = 0; ii < MAXPARAMETERS; ii++) {                                                               */
	/*	x[ii] = scaleto(parameters[ii].value, parameters[ii].minimum, parameters[ii].maximum, -10, +10); */
	/*}                                                                                                      */


	/**/
	/*Initialization7*/
	return EXIT_SUCCESS;
}
