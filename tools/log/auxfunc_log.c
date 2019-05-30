/*
 *  Copyright (C) 1999-2005 Joao Ramos
 * Your use of this code is subject to the terms and conditions of the
 * GNU general public license version 2. See "COPYING" or
 * http://www.gnu.org/licenses/gpl.html
 */

#include <stdio.h>
//#include <ctype.h>
#include <math.h>
//#include <setjmp.h>
//#include <assert.h>
#include <stdlib.h>
#include <string.h>


#include "auxfunc.h"
#include "auxfunc_log.h"
#include "dosdir.h"




/*
 *
 */
void PrintOneLine(char *lkk1, double *stats, int num_measures, FILE **fOut)
{
	int i, j,k;
	char laux[LONGSTRINGSIZE], laux2[LONGSTRINGSIZE];

	strcpy(laux2, lkk);
	for (k = 0; k <= strlen(lkk1)-1; k++)
		laux2[k]=lkk1[k];
	j=1;
	for (i = 1; i <= num_measures; i=i+2) {
		sprintf(laux, "%E", stats[i]);
		j=strpos2(lkk, ":", j);
		for (k = 0; k <= strlen(laux)-1; k++)
			laux2[k+j]=laux[k];
		j=j+k+2;
	}
	fprintf(*fOut, "%s\n", laux2);
}  /*PrintOneLine*/




/*
 *
 */
void WriteStats(int num_measures, int data_lines, statistics stats, FILE **fOut)
{
	int i;
	double TEMP;

	putc('\n', *fOut);
	for (i = 1; i <= num_measures; i++) {
		if (data_lines != 0) {
			stats.avg[i] /= data_lines;     /*average value*/
			TEMP = stats.avg[i];
			stats.sig[i] = sqrt(fabs(stats.sig[i] / data_lines - TEMP * TEMP));
				if (stats.sig[i] < fabs(stats.max[i]/1000))
					stats.sig[i]=0; /*standard deviation*/
		}
	}

	PrintOneLine("mean :", stats.avg, num_measures, fOut);
	PrintOneLine("sigma:", stats.sig, num_measures, fOut);
	PrintOneLine("max  :", stats.max, num_measures, fOut);
	PrintOneLine("min  :", stats.min, num_measures, fOut);
}  /*WriteStats*/




/*
 *
 */
void SimpleParametersCategory(int num_measures, char *llog, statistics stats, FILE **fOut)
{
	int i, j;
	char lkk1[LONGSTRINGSIZE], lkk2[LONGSTRINGSIZE];

	fprintf(*fOut, "\n\n");
	llog[0]=58; //':'
	j=0;
	for (i = 1; i <= (num_measures); i=i+2) {
		ReadSubKey(lkk1, llog, &j, ':', ':', 0);
		StripSpaces(lkk1);
		sprintf(lkk2, "%s:#%s#:%E:%E:%E:LIN_DOUBLE:OPT", lkk1, lkk1, (stats.max[i]+stats.min[i])/2, stats.min[i], stats.max[i]);
		fprintf(*fOut, "%s\n", lkk2);
		ReadSubKey(lkk1, llog, &j, ':', ':', 0);
	}
}




/*
 *
 */
void ComplexParametersCategory(char *llog, statistics stats, FILE **fOut, FILE **fcfg)
{
	int i, j;
	char lkk1[LONGSTRINGSIZE], lkk2[LONGSTRINGSIZE];

		fprintf(*fOut, "\n\n");
		fgets2(lkk, LONGSTRINGSIZE, *fcfg);
		while ((lkk[0] != '#') && (lkk[0] != '\0') && (!feof(*fcfg))) {
			i=strpos2(lkk, ":OPT", 1);
			if (i) { //line with OPT
				j=0;
				ReadSubKey(lkk1, lkk, &j, '#', '#', 5); //lkk1=symbol
				lkk2[0]='\0';
				i=0;
				j=0;
				while (strcmp (lkk1, lkk2)) {
					i++;
					ReadSubKey(lkk2, llog, &j, ':', ':', 0);
					StripSpaces(lkk2);
				}
				i++; //the correct possition of the value
				j=0;
				ReadSubKey(lkk1, lkk, &j, ':', ':', 0);
				ReadSubKey(lkk1, lkk, &j, ':', ':', 0);
				strsub(lkk1, lkk, 1, j);
				sprintf(lkk2, "%E:%E", stats.min[i], stats.max[i]);
				strcat(lkk1, lkk2);
				ReadSubKey(lkk2, lkk, &j, ':', ':', 0);
				ReadSubKey(lkk2, lkk, &j, ':', ':', 0);
				strsub(lkk2, lkk, j, strlen(lkk));
				strcat(lkk1, lkk2);
				fprintf(*fOut, "%s\n", lkk1);

			} else { //line with ---
				fprintf(*fOut, "%s\n", lkk);
			}
			fgets2(lkk, LONGSTRINGSIZE, *fcfg);
		}
}




/*
 *
 */
void CreateStatistics(char *InputFile, char *OutputFile)
{
	int i, j, k;
	char lkk1[LONGSTRINGSIZE];
	statistics stats;
	FILE *fIn, *fOut, *fcfg;
	double aux;
	int num_measures;

//------------------------------------------------------------------
//------------------------------------------------------------------
	/*create summary: table version*/
	for (i = 0; i <= (MAXMEAS-1); i++) {
		stats.avg[i] = 0.0;   /*initialization of statistics variables*/
		stats.sig[i] = 0.0;
		stats.max[i] = -1.7e38;
		stats.min[i] = 1.7e38;
	}


	if ((fIn=fopen(InputFile,"rt")) == 0) {
		printf("auxfunc_log.c - Cannot open input file: %s\n", InputFile);
		exit(EXIT_FAILURE);
	}
	if ((fOut=fopen(OutputFile,"wt")) == 0) {
		printf("auxfunc_log.c - Cannot open output file: %s\n", OutputFile);
		exit(EXIT_FAILURE);
	}

	fgets2(lkk, LONGSTRINGSIZE, fIn);
	strsub(lkk1, lkk, 1, strpos2(lkk, ";", 1)-1);

	i=0; j=0; k=0;
	while (!P_eof(fIn)) {
		aux = asc2real(lkk1, 1, (int)strlen(lkk1));
		stats.avg[i] += aux;
		stats.sig[i] += aux * aux;
			/*variancia=sum(x^2)/n - (avg(x))^2 # sigma=sqrt(variancia)*/
		if (aux > stats.max[i])
			stats.max[i] = aux;
		if (aux < stats.min[i])
			stats.min[i] = aux;
		if (j==strlen(lkk)) {
			num_measures=i;
			i=0;j=0;k++;
			fgets2(lkk, LONGSTRINGSIZE, fIn);
			strsub(lkk1, lkk, 1, strpos2(lkk, ":", 1)-1);
		} else {
			ReadSubKey(lkk1, lkk, &j, ':', ':', 1);
			i++; j--;
		}

	}

	WriteStats(num_measures-1, k, stats, &fOut);


	//--
	dd_ffblk fb;
	char *mask="*.cfg";
	if (!dd_findfirst( mask, &fb, DD_DIREC ))
		printf("Opening config file: %s\n", fb.dd_name);
	//--


//--------
	char llog[LONGSTRINGSIZE];
	strcpy(llog,lkk); //last line from log file

	fcfg=NULL;
	if ((fcfg=fopen(fb.dd_name,"rt")) == 0) {
		if (fb.dd_name) {
			printf("auxfunc_log.c - Cannot open config file: %s\n", fb.dd_name);
			//exit(EXIT_FAILURE);
			}
		SimpleParametersCategory(num_measures-1, llog, stats, &fOut);
	} else {
		ReadKey(lkk, "# Parameters #", fcfg);   //configuration parameters
		if (strcmp(lkk, "# Parameters #")) {
			printf("INFO:  initialize.c - Step2 -- No parameters in config file\n");
			SimpleParametersCategory(num_measures-1, llog, stats, &fOut);
		} else {
			ComplexParametersCategory(llog, stats, &fOut, &fcfg);
		}
	}
//--------


	if (fOut != NULL)
		fclose(fOut);
	fOut = NULL;
	if (fIn != NULL)
		fclose(fIn);
	fIn = NULL;
//------------------------------------------------------------------
//------------------------------------------------------------------
}