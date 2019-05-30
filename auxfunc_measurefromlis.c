/*
 * Copyright (C) 1999-2016 Joao Ramos
 * Your use of this code is subject to the terms and conditions of the
 * GNU general public license version 2. See "COPYING" or
 * http://www.gnu.org/licenses/gpl.html
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <math.h>

#include "auxfunc_measurefromlis.h"
#include "auxfunc.h"


#ifdef ASCO
extern void WriteToMem(int num_measures);
#else
void WriteToMem(int num_measures){}
#endif




/***************************************************************/
/*MeasureFromLIS ***********************************************/
/***************************************************************/

/*
 * Receives the string 'line' and an indication in 'mem' if it comes from file
 * or memory. Return '1' if it is a transistor, '0' otherwise.
 * 'i' is the index position; 'nextline' contains the next text from the SPICE
 * output file if 'mem'=0.
 */
int IsItATransistor(char *line, int mem, int i, char *nextline)
{
	int j=1;
	char lkk1[SHORTSTRINGSIZE];

	switch(spice) {
		case 1: /*Eldo*/
			if (mem) { /*mem=1*/
				ReadSubKey(lkk1, measure[i].var_name, &j, '(', ')', 0);
				if ( ((int)strlen(lkk1)) && (measure[i].s_column1==0) && (measure[i].column2==0) )
					return 1; /*it is a transistor*/
				else
					return 0;
			} else {   /*mem=0*/
				strsub(lkk1, line, 1, 9);
				if ( (!strcmp("        M", lkk1)) || (!strcmp("        X", lkk1)) ) {                                                             /*checks first line */
					if ( (strpos2(nextline, "MODEL   ", 1) != 0) && ((strpos2(nextline, "PMOS", 1) != 0) || (strpos2(nextline, "NMOS", 1))) ) /*checks second line*/
						return 1; /*it is a transistor*/
					if ( (strpos2(nextline, "MODEL   ", 1) != 0) && ((strpos2(nextline, "PCH", 1) != 0) || (strpos2(nextline, "NCH", 1))) )   /*checks second line*/
						return 1; /*it is a transistor*/
				}
				return 0;
			}
			break;
		case 2: /*HSPICE*/
			if (mem) { /*mem=1*/
				ReadSubKey(lkk1, measure[i].var_name, &j, '(', ')', 0);
				if ( ((int)strlen(lkk1)) && (measure[i].s_column1==0) && (measure[i].column2==0) )
					return 1; /*it is a transistor*/
				else
					return 0;
			} else {   /*mem=0*/
				if ( (strpos2(line, "element  ", 1) != 0) && (strpos2(line, ":m", 1)) ) {                                                          /*checks first line */
					if ( (strpos2(nextline, " model ", 1) != 0) && ((strpos2(nextline, ":pmos", 1) != 0) || (strpos2(nextline, ":nmos", 1))) ) /*checks second line*/
						return 1; /*it is a transistor*/
					if ( (strpos2(nextline, " model ", 1) != 0) && ((strpos2(nextline, ":pch", 1) != 0) || (strpos2(nextline, ":nch", 1))) )   /*checks second line*/
						return 1; /*it is a transistor*/
				}
				return 0;
			}
			break;
		case 3: /*LTspice*/
			if (mem) { /*mem=1*/
				ReadSubKey(lkk1, measure[i].var_name, &j, '(', ')', 0);
				if ( ((int)strlen(lkk1)) && (measure[i].s_column1==0) && (measure[i].column2==0) )
					return 1; /*it is a transistor*/
				else
					return 0;
			} else {   /*mem=0*/
				if ( (strpos2(line, "Name:    ", 1) != 0) && (strpos2(line, "m:", 1)) ) {                                                          /*checks first line */
					if ( (strpos2(nextline, "Model: ", 1) != 0) && ((strpos2(nextline, " pmos", 1) != 0) || (strpos2(nextline, " nmos", 1))) ) /*checks second line*/
						return 1; /*it is a transistor*/
					if ( (strpos2(nextline, "Model: ", 1) != 0) && ((strpos2(nextline, " pch", 1) != 0) || (strpos2(nextline, " nch", 1))) )   /*checks second line*/
						return 1; /*it is a transistor*/
				}
				return 0;
			}
			break;
		case 4: /*Spectre*/
			if (mem) { /*mem=1*/
				ReadSubKey(lkk1, measure[i].var_name, &j, '(', ')', 0);
				if ( ((int)strlen(lkk1)) && (measure[i].s_column1==0) && (measure[i].column2==0) )
					return 1; /*it is a transistor*/
				else
					return 0;
			} else {   /*mem=0*/
				if ( (strpos2(line, "Instance: ", 1) != 0) && (strpos2(line, "M", 1)) ) {                                                          /*checks first line */
					if ( (strpos2(nextline, "Model: ", 1) != 0) && ((strpos2(nextline, "pmos", 1) != 0) || (strpos2(nextline, "nmos", 1))) )   /*checks second line*/
						return 1; /*it is a transistor*/
					if ( (strpos2(nextline, "Model: ", 1) != 0) && ((strpos2(nextline, "pch", 1) != 0) || (strpos2(nextline, "nch", 1))) )     /*checks second line*/
						return 1; /*it is a transistor*/
				}
				return 0;
			}
			break;
		case 50: /*Qucs*/
			return 0;
			break;
		case 51: /*ngspice*/
			return 0;
			break;
		case 100: /*general*/
			return 0;
			break;
		default:
			printf("auxfunc_measurefromlis.c - IsItATransistor -- Something unexpected has happened!\n");
			exit(EXIT_FAILURE);
	}

} /*IsItATransistor*/




/*
 * On a given simulation output, finds the number of columns having transistors
 * and saves their column position in index[]
 */
int DetectsTransistorColumns(char *lelement, int index[])
{
	int i=1, j=0;

	switch(spice) {
		case 1: /*Eldo*/
			if (strpos2(lelement, "        M", 1) != 0) {
				while (i < (int)strlen(lelement)) {
					if (lelement[i - 1] == 'M') {
						j++;                  /*how many columns with transistors ?*/
						index[j - 1] = i + 0; /*saves column for print*/
						i++;
					} else
						i++;
				}
				index[j] = i + 1;                     /*corrects size for last column*/
			}
 			if (strpos2(lelement, "        X", 1) != 0) {
				while (i < (int)strlen(lelement)) {
					if (lelement[i - 1] == '.') {
						j++;                  /*how many columns with transistors ?*/
						index[j - 1] = i - 6; /*saves column for print*/
						i++;
					} else
						i++;
				}
				index[j] = i + 1;                     /*corrects size for last column*/
			}
			break;
		case 2: /*HSPICE*/
			while (i < (int)strlen(lelement)) {
				if (lelement[i - 1] == ':') {
					j++;                  /*how many columns with transistors ?*/
					index[j - 1] = i - 1; /*saves column for print*/
					i++;
				} else
					i++;
			}
			index[j] = i + 1;                     /*corrects size for last column*/
			break;
		case 3: /*LTspice*/
			while (i < (int)strlen(lelement)) {
				if ((lelement[i - 3] == ' ') && (lelement[i - 2] == 'm') && (lelement[i - 1] == ':')) {
					j++;                  /*how many columns with transistors ?*/
					index[j - 1] = i - 1; /*saves column for print*/
					i++;
				} else
					i++;
			}
			index[j] = i + 1;                     /*corrects size for last column*/
			break;
		case 4: /*Spectre*/
			index[0]=11;
			index[1]=99;
			j=1;
			break;
		case 50: /*Qucs*/
			printf("auxfunc_measurefromlis.c - DetectsTransistorColumns -- Qucs not supported\n");
			exit(EXIT_FAILURE);
		case 51: /*ngspice*/
			printf("auxfunc_measurefromlis.c - DetectsTransistorColumns -- ngspice not supported\n");
			exit(EXIT_FAILURE);
		case 100: /*general*/
			printf("auxfunc_measurefromlis.c - DetectsTransistorColumns -- GENERAL not supported\n");
			exit(EXIT_FAILURE);
		default:
			printf("auxfunc_measurefromlis.c - DetectsTransistorColumns -- Something unexpected has happened!\n");
			exit(EXIT_FAILURE);
	}
	return j; /*returns the number of columns having transistors*/
} /*DetectsTransistorColumns*/




/*
 * receives the variable 'measure_data' and fills the necessary mathematical data
 */
void DoMath(int num_measures)
{
	int i, j, k, l;
	double RPN1, RPN2, data=0;
	char lkk1[LONGSTRINGSIZE];
	char STR1[LONGSTRINGSIZE];
	/*char STR2[LONGSTRINGSIZE], STR3[LONGSTRINGSIZE];*/

	RPN1 = 1.0;
	RPN2 = 1.0;

	for (i = 0; i <= num_measures; i++) {
		sprintf(STR1, "%.4s", measure[i].search);
		if (!strcasecmp(STR1, "math")) {   /*if a mathematical operation is requested, then*/
			strcpy(lkk1, " ");
			j=1;
			while (*lkk1 != '\0') {
				/*measure[i].search[4] = ';';*/   /*replace '=' by ';'*/
				ReadSubKey(lkk1, measure[i].search, &j, ':', ':', 0);
				if (*lkk1 == '\0') {
					sprintf(lkk1, "% .5E", RPN1);
					strcpy(measure[i].data, lkk1);
					*lkk1 = '\0';
				}
				StripSpaces(lkk1);

				/*Step1*/
				k=0;
				sprintf(STR1, "%.*s", (int)strlen(lkk1), lkk1);
				l = sscanf(STR1, "%i", &k); /*'l' is one if a number is read*/
				if (lkk1[0] == '&') { /*to read one measurement*/
					l = (sscanf(strsub(STR1, lkk1, 2, (int)strlen(lkk1)), "%i", &k) == 0); /* &<digit> format else is */
					if (k == 0) {
						for (k = 1; k < i; k++) {                                      /* &<text>  format         */
							/*ll = 1;*/
							if (strpos2(measure[k].var_name, STR1,1)) { /* if variable in STR1 exists in measure[k].var_name */
								/*fflush(0);*/
								break;
							}
						}
					}
					if (k==i) {
						#ifdef ASCO
						printf("auxfunc_measurefromlis.c - DoMath -- Field MEASURE_VAR '%s', defined in '%s, not found memory. Revise file in ./extract'\n", STR1, measure[k].search);
						#else
						printf("auxfunc_measurefromlis.c - DoMath -- Field MEASURE_VAR '%s', defined in '%s, not found memory. Revise config file'\n", STR1, measure[k].search);
						#endif
						exit(EXIT_FAILURE);
					}
					data = asc2real(measure[k].data, 1, (int)strlen(measure[k].data));
				} else {
					if (l == 1) /*to read one number*/
						data = asc2real(STR1, 1, (int)strlen(STR1));
					else {      /*to read a mathematical operator*/
						switch (lkk1[0]) {
							case '+': /*plus*/
								data = RPN2 + RPN1;
								break;

							case '-': /*minus*/
								data = RPN2 - RPN1;
								break;

							case '*': /*multiply*/
								data = RPN2 * RPN1;
								break;

							case '/': /*divide*/
								if (fcmp(RPN1, 0))
									data = RPN2 / RPN1;
								else {
									printf("auxfunc_measurefromlis.c - DoMath -- MATH divided by zero => %s\n", measure[i - 1].search);
									exit(EXIT_FAILURE);
								}
								break;

							case '^': /*square*/
								data = RPN1 * RPN1;
								break;

							case 'a': /*abs*/
								if (!strcmp(lkk1, "abs"))
									data = fabs(RPN1);
								else {
									printf("auxfunc_measurefromlis.c - DoMath -- MATH %s not recognized\n", lkk1);
									exit(EXIT_FAILURE);
								}
								break;

							case 'l': /*log10*/
								if (!strcmp(lkk1, "log10")) {
									if (RPN1 > 0)
									data = log(RPN1) / log(10.0);
									else {
										printf("Error: MATH log(0) => %s\n", measure[i - 1].search);
										exit(EXIT_FAILURE);
									}
								} else {
										printf("auxfunc_measurefromlis.c - DoMath -- MATH %s not recognized\n", lkk1);
										exit(EXIT_FAILURE);
								}
								break;

							case 's': /*sqrt*/
								if (!strcmp(lkk1, "sqrt"))
									data = sqrt(RPN1);
								else {
									printf("auxfunc_measurefromlis.c - DoMath -- MATH %s not recognized\n", lkk1);
									exit(EXIT_FAILURE);
								}
								break;

							default:
								if (*lkk1 != '\0') {
									printf("auxfunc_measurefromlis.c - DoMath -- MATH %s not recognized\n", lkk1);
									exit(EXIT_FAILURE);
								}
								break;
						}
					}
				}

				/*Step2*/
				RPN2 = RPN1;
				/*Step3*/
				RPN1 = data;
			}
		}
		/* -------------- strcpy(lkk1, " ");*/
		/* -------------- j=1;*/
	}
} /*DoMath*/




/*
 *
 */
void WriteToFile(int num_measures, char *laux, int first, statistics *stats, FILE **fSummary)
{
	int i, j, ptr;
	double aux;
	char laux1[LONGSTRINGSIZE], laux2[LONGSTRINGSIZE], line[LONGSTRINGSIZE], lineHeader[LONGSTRINGSIZE];
	int FORLIM, FORLIM1;

	DoMath(num_measures);   /*'MATH=&...'; information is in the variable 'measure[i].data'*/

	/*Step0: Compute the values for statistics*/
	for (i = 1; i <= num_measures; i++) {
		if (strlen(measure[i].data)) {
			aux = asc2real(measure[i].data, 1, (int)strlen(measure[i].data));
			stats->avg[i] += aux;
			stats->sig[i] += aux * aux; /*variance=sum(x^2)/n - (avg(x))^2 : sigma=sqrt(variance)*/
			if (aux > stats->max[i])
				stats->max[i] = aux;
			if (aux < stats->min[i])
				stats->min[i] = aux;
			stats->num[i]++;
		}
	}
	/*Step0*/

	if (num_measures < 13) {
	/*Step1: if to process less than 13 MEASUREMENTS*/
		*line = '\0';
		StripSpaces(laux);

		if (*measure[0].data == '\0') {             /*Was ALTER found?   */
			switch(spice) {
				case 1: /*Eldo*/
					break;
				case 2: /*HSPICE*/
					if (laux[0] != '@') /*ALTER was not found*/
						strcpy(laux, "                     ");
					break;
				case 3: /*LTspice*/
				case 4: /*Spectre*/
				case 50: /*Qucs*/
				case 51: /*ngspice*/
				case 100: /*general*/
					strcpy(laux, "                     ");
					break;
				default:
					printf("auxfunc_measurefromlis.c - WriteToFile -- Something unexpected has happened!\n");
					exit(EXIT_FAILURE);
			}
			FORLIM = (int)strlen(laux);
			for (i = 1; i <= FORLIM; i++)       /*ALTER was found    */
				strcat(line, " ");
		}

		for (i = 0; i <= num_measures; i++) { /*Build measured line*/
			if ((!IsItATransistor(measure[i].search, 1, i, '\0')) || measure[i].column1 == 0) { /* print only 'vgs-vth' and not 'vgs' and 'vth'. */
				strcpy(laux1, measure[i].data);
				/*        if (i > 0) {*/
				aux = asc2real(laux1, 1, (int)strlen(laux1));
				j = extended2engineer(&aux);
				/*debug*/
				if (fcmp(aux,0)) { /* we are going to read a number */
					sprintf(laux1, "%7.3f", aux);
					sprintf(laux2, "%i", j);
					sprintf(line + (int)strlen(line), "%se%s | ", laux1, laux2);
				/*debug*/
				} else {         /* we are going to read text instead of a number */
					StripSpaces(measure[i].data);
					sprintf(line + (int)strlen(line), "%s | ", measure[i].data);
				}
			}
		}

		ptr = 1; /*Build header line*/
		if (*measure[0].data == '\0' && first) {
			strcpy(lineHeader, line);
			FORLIM = (int)strlen(lineHeader);
			for (i = 0; i < FORLIM; i++) {
				if (lineHeader[i] != '|')
					lineHeader[i] = ' ';
			}

			for (i = 0; i <= num_measures; i++) {
				if ((!IsItATransistor(measure[i].search, 1, i, '\0')) || measure[i].column1 == 0) { /* print only 'vgs-vth' and not 'vgs' and 'vth'. */
					strcpy(laux1, measure[i].var_name);
					FORLIM1 = (int)strlen(measure[i].var_name);
					for (j = 0; j < FORLIM1; j++) {
						/*begin*/
						if (lineHeader[ptr - 1] != '|') {
							lineHeader[ptr - 1] = measure[i].var_name[j];
							ptr++;
						/*end;*/
						}
					}
					ReadSubKey(laux1, lineHeader, &ptr, '|', ' ', 0);
				}
			}
			fprintf(*fSummary, "%s\n", lineHeader);   /*Writes Header line*/
		}

		fprintf(*fSummary, "%s\n", line);   /*Writes one (1) measured line*/
		return;
	}
	/*Step1*/

	/*Step2: if to process more than 12 MEASUREMENTS*/
	StripSpaces(laux);

	ptr = 1; /*Build header line*/
	if (*measure[0].data == '\0' && first) {
		for (i = 0; i <= num_measures; i++) {
			if ((!IsItATransistor(measure[i].search, 1, i, '\0')) || measure[i].column1 == 0) { /* print only 'vgs-vth' and not 'vgs' and 'vth'. */
				strcpy(laux1, measure[i].var_name);
				fprintf(*fSummary, "%s | ", measure[i].var_name);
			}
		}
		putc('\n', *fSummary);
	}

	if (*measure[0].data == '\0') {             /*Was ALTER found?   */
		switch(spice) {
			case 1: /*Eldo*/
				break;
			case 2: /*HSPICE*/
				if (laux[0] != '@') /*ALTER was not found*/
					strcpy(laux, "                     ");
				break;
			case 3: /*LTspice*/
			case 4: /*Spectre*/
			case 50: /*Qucs*/
			case 51: /*ngspice*/
			case 100: /*general*/
				strcpy(laux, "                     ");
				break;
			default:
				printf("auxfunc_measurefromlis.c - WriteToFile -- Something unexpected has happened!\n");
				exit(EXIT_FAILURE);
		}
		FORLIM = (int)strlen(laux);
		for (i = 1; i <= FORLIM; i++)
			putc(' ', *fSummary);
	}

	for (i = 0; i <= num_measures; i++) { /*Build measured line*/
		if ((!IsItATransistor(measure[i].search, 1, i, '\0')) || measure[i].column1 == 0) { /* print only 'vgs-vth' and not 'vgs' and 'vth'. */
			strcpy(laux1, measure[i].data);
			/*      if (i > 0) {*/
			aux = asc2real(laux1, 1, (int)strlen(laux1));
			j = extended2engineer(&aux);
			/*debug*/
			if (fcmp(aux,0)) { /*we are going to read a number*/
				sprintf(laux1, "%7.3f", aux);
				sprintf(laux2, "%i", j);
				fprintf(*fSummary, "%se%s | ", laux1, laux2);
			/*debug*/
			} else {         /* we are going to read text instead of a number */
				StripSpaces(measure[i].data);
				fprintf(*fSummary, "%s | ", measure[i].data);
			}
		}
	}
	putc('\n', *fSummary);
	/*Step2*/

} /*WriteToFile*/




/*
 *
 */
void PrintOneLine(char *lkk1, double *stats, int num_measures, FILE **fSummary)
{
	int i, j;
	char laux[LONGSTRINGSIZE];
	int FORLIM;

	if (*measure[0].data == '\0')
		strcpy(measure[0].data, "                     ");
	FORLIM = (int)strlen(measure[0].data) - 6;
	for (i = 1; i <= FORLIM; i++)
		putc(' ', *fSummary);
	fputs(lkk1, *fSummary);
	for (i = 1; i <= num_measures; i++) {
		if ((!IsItATransistor(measure[i].search, 1, i, '\0')) || measure[i].column1 == 0) { /* print only 'vgs-vth' and not 'vgs' and 'vth'. */
			j = extended2engineer(&stats[i]);
			sprintf(laux, "%7.3f", stats[i]);
			fprintf(*fSummary, "%se%i | ", laux, j);
		}
	}
	putc('\n', *fSummary);
} /*PrintOneLine*/




/*
 *
 */
void WriteStats(int num_measures, int data_lines, statistics stats, FILE **fSummary)
{
	int i;
	double TEMP;

	putc('\n', *fSummary);
	for (i = 1; i <= num_measures; i++) {
		//if (data_lines != 0) {
		if (stats.num[i] != 0) {
			{
			stats.avg[i] /= stats.num[i];   /*average value*/
			TEMP = stats.avg[i];
			stats.sig[i] = sqrt(fabs(stats.sig[i] / stats.num[i] - TEMP * TEMP));
				if (stats.sig[i] < fabs(stats.max[i]/1000))
					stats.sig[i]=0; /*standard deviation*/
			}
		} else {
			#ifndef ASCO
			printf("INFO:  auxfunc_measurefromlis - WriteStats -- Data was not read for %s.\n", measure[i].var_name);
			#endif
			#ifdef ASCO
			#ifdef DEBUG
			printf("INFO:  auxfunc_measurefromlis - WriteStats -- Data was not read for %s.\n", measure[i].var_name);
			#endif
			#endif
		}
	}

	PrintOneLine("mean  =| ", stats.avg, num_measures, fSummary);
	PrintOneLine("sigma =| ", stats.sig, num_measures, fSummary);
	PrintOneLine("max   =| ", stats.max, num_measures, fSummary);
	PrintOneLine("min   =| ", stats.min, num_measures, fSummary);
} /*WriteStats*/




/*
 *
 */
int CMOSText2Line(char *lkk2, char *OutputFile)
{
	FILE *fLIS;                /* Due to HSPICE 2001.2 line */
	char lkk3[LONGSTRINGSIZE]; /*          "                */
	int j;                     /* with the operation region */
	int Result=0;

	switch(spice) {
		case 1: /*Eldo*/
			Result=0;
			char  *Eldo_OP[] = {
			"", "", "", "ID", "VGS", "VDS", "VBS", "VTH", "VDSAT", "GM", "GDS", "GMB", "Cdd", "Cdg", "Cds", "Cdb","Cgd","Cgg", "Cgs", "Cgb", "Csd", "Csg", "Css", "Csb", "Cbd", "Cbg", "Cbs", "Cbb","PHI", "VBI","Region", "VTH_D"
			};
			for (j = 0; j <= 31; j++) {
				if (!strcasecmp(lkk2, Eldo_OP[j])) {
					Result = j;
					break;
				}
			}
			break;
		case 2: /*HSPICE*/
			Result=0;
			char  *HSPICE_OP[] = {
			"", "", "id", "ibs", "ibd", "vgs", "vds", "vbs", "vth", "vdsat", "beta", "gam eff", "gm", "gds" , "gmb", "cdtot", "cgtot", "cstot", "cbtot", "cgs", "cgd"
			};
			for (j = 0; j <= 20; j++) {
				if (!strcasecmp(lkk2, HSPICE_OP[j])) {
					Result = j;
					break;
				}
			}
			/*
			 * The next block was added due to the new line in HSPICE 2001.2
			 * with the operating region                                     */
			if ((fLIS=fopen(OutputFile,"rt")) == 0) {
				printf("auxfunc_measurefromlis.c - CMOSText2Line -- Cannot open output file: %s\n", OutputFile);
				exit(EXIT_FAILURE);
			}
			fgets2(lkk3, LONGSTRINGSIZE, fLIS);

			j = (strpos2(lkk3, "-- 200", 1)  );/* Due to HSPICE 2001.2 line */
			if (lkk3[j+5] != '0')              /*          "                */
				Result++;                  /* with the operation region */

			if (fLIS != NULL)
				fclose(fLIS);
			/* end of block
			 *
			 */
			break;
		case 3: /*LTspice*/
			Result=0;
			char  *LTspice_OP[] = {
			"", "", "Id", "Vgs", "Vds", "Vbs", "Vth", "Vdsat", "Gm", "Gds", "Gmb", "Cbd", "Cbs", "Cgsov", "Cgdov", "Cgbov","dQgdVgb", "dQgdVdb", "dQgdVsb", "dQddVgb", "dQddVdb", "dQddVsb", "dQbdVgb", "dQbdVdb", "dQbdVsb"
			};
			for (j = 0; j <= 24; j++) {
				if (!strcasecmp(lkk2, LTspice_OP[j])) {
					Result = j;
					break;
				}
			}
			break;
		case 4: /*Spectre*/
		if (1) { /*bsim3v3*/
			Result=0;
			char  *Spectre_OP[] = {
			"", "", "", "", "", "", "", "", "region", "reversed", "ids", "isub", "vgs", "vds", "vbs", "vth", "vdsat", "gm", "gds", "gmbs", "betaeff", "cjd", "cjs", "qb", "qg", "qd", "qbd", "qbs", "cgg", "cgd", "cgs", "cgb", "cdg", "cdd", "cds", "cdb", "csg", "csd", "css", "csb", "cbg", "cbd", "cbs", "cbb", "ron", "id", "is", "ibulk", "ibs", "ibd", "pwr", "gmoverid", "cgsovl", "cgdovl", "cgbovl", "i1", "i3", "i4", "gbd", "gbs", "vgsteff", "qinv"
			};
			for (j = 0; j <= 61; j++) {
				if (!strcasecmp(lkk2, Spectre_OP[j])) {
					Result = j;
					break;
				}
			}

			if (Result>14) { /*required for versions after the 2nd half of 2006 who have 3 more lines in between "vbs" and "vth"*/
				if ((fLIS=fopen(OutputFile,"rt")) == 0) {
					printf("auxfunc_measurefromlis.c - CMOSText2Line -- Cannot open output file: %s\n", OutputFile);
					exit(EXIT_FAILURE);
				}
				
				int iii=0,jjj=0;
				/*Spectre format 1: up to the 1st half of 2007*/
				fgets2(lkk3, LONGSTRINGSIZE, fLIS);
				if (strpos2(lkk3, "Command line:", 1)) { /*it might be necessary or not to add a correction*/
					for (j = 0; j <= 15; j++) { /*read the first lines to detect, for example: "spectre (ver. 6.0.1.122 -- 24 May 2005)." */
						fgets2(lkk3, LONGSTRINGSIZE, fLIS);
						if (strpos2(lkk3, "spectre (", 1)) {
							jjj = strpos2(lkk3, ")", 1);
							iii = (lkk3[jjj-3] - '0')*10 +   0;
							iii = (lkk3[jjj-2] - '0')*01 + iii;
							if (jjj==0) {
								printf("auxfunc_measurefromlis.c - CMOSText2Line -- Something unexpected has happened!\n");
								exit(EXIT_FAILURE);
							}
							break;
						}
					}
				}

				/*Spectre format 2: after the 2nd half of 2007*/
				fgets2(lkk3, LONGSTRINGSIZE, fLIS);
				if (strpos2(lkk3, "Cadence (", 1))       /*it is necessary to add a correction             */
					iii=7; /*equivalent to 2007*/

				if (fLIS != NULL)
					fclose(fLIS);


				if (iii>=6) /*apply correction for Spectre versions 2006, 2007, and so on*/
					Result=Result+3;
			}
		}
		if (0) { /*bsim4*/
			Result=0;
			char  *Spectre_OP[] = {
			"", "", "", "", "", "", "", "region", "reversed", "ids", "vgs", "vds", "vbs", "vgd", "vdb", "vgb", "vth", "vdsat", "gm", "gds", "gmbs", "betaeff", "cjd", "cjs", "cgg", "cgd", "cgs", "cgb", "cdg", "cdd", "cds", "cdb", "csg", "csd", "css", "csb", "cbg", "cbd", "cbs", "cbb", "covlgs", "covlgd", "covlgb", "cggbo", "cgdbo", "cgsbo", "cbgbo", "cbdbo", "cbsbo", "cdgbo", "cddbo", "cdsbo", "ron", "id", "ibulk", "pwr", "gmoverid", "rdeff", "rseff", "rgbd", "igidl", "igisl", "igdt", "igd", "igs", "igb", "igbacc", "igbinv", "igcs", "igcd", "gbs", "gbd"
			};
			for (j = 0; j <= 71; j++) {
				if (!strcasecmp(lkk2, Spectre_OP[j])) {
					Result = j;
					break;
				}
			}
		}
			break;
		case 50: /*Qucs*/
			printf("auxfunc_measurefromlis.c - CMOSText2Line -- Qucs not supported\n");
			exit(EXIT_FAILURE);
		case 51: /*ngspice*/
			printf("auxfunc_measurefromlis.c - CMOSText2Line -- ngspice not supported\n");
			exit(EXIT_FAILURE);
		case 100: /*general*/
			printf("auxfunc_measurefromlis.c - CMOSText2Line -- GENERAL not supported\n");
			exit(EXIT_FAILURE);
		default:
			printf("auxfunc_measurefromlis.c - CMOSText2Line -- Something unexpected has happened!\n");
			exit(EXIT_FAILURE);
	}

	if (Result==0) {
		printf("auxfunc_measurefromlis.c - CMOSText2Line -- String '%s' was not found. Verify MEASURE_VAR in <configfile>\n", lkk2);
		exit(EXIT_FAILURE);
	}
	return Result;
} /*CMOSText2Line*/




/*
 *
 */
char *CMOSLine2Text(char *Result, int measure_x_lines_below, char *OutputFile)
{
	FILE *fLIS;                /* Due to HSPICE 2001.2 line */
	char lkk3[LONGSTRINGSIZE]; /*          "                */
	int j;                     /* with the operation region */

	switch(spice) {
		case 1: /*Eldo*/
			switch (measure_x_lines_below) {   /*we have to read a number instead of text*/
				case 3:
					strcpy(Result, "ID");
					break;
				case 4:
					strcpy(Result, "VGS");
					break;
				case 5:
					strcpy(Result, "VDS");
					break;
				case 6:
					strcpy(Result, "VBS");
					break;
				case 7:
					strcpy(Result, "VTH");
					break;
				case 8:
					strcpy(Result, "VDSAT");
					break;
				case 9:
					strcpy(Result, "VDSAT");
					break;
				case 10:
					strcpy(Result, "GDS");
					break;
				case 11:
					strcpy(Result, "GMB");
					break;
				case 12:
					strcpy(Result, "Cdd");
					break;
				case 13:
					strcpy(Result, "Cdg");
					break;
				case 14:
					strcpy(Result, "Cds");
					break;
				case 15:
					strcpy(Result, "Cdb");
					break;
				case 16:
					strcpy(Result, "Cgd");
					break;
				case 17:
					strcpy(Result, "Cgg");
					break;
				case 18:
					strcpy(Result, "Cgs");
					break;
				case 19:
					strcpy(Result, "Cgb");
					break;
				case 20:
					strcpy(Result, "Csd");
					break;
				case 21:
					strcpy(Result, "Csg");
					break;
				case 22:
					strcpy(Result, "Css");
					break;
				case 23:
					strcpy(Result, "Csb");
					break;
				case 24:
					strcpy(Result, "Cbd");
					break;
				case 25:
					strcpy(Result, "Cbg");
					break;
				case 26:
					strcpy(Result, "Cbs");
					break;
				case 27:
					strcpy(Result, "Cbb");
					break;
				case 28:
					strcpy(Result, "PHI");
					break;
				case 29:
					strcpy(Result, "VBI");
					break;
				case 30:
					strcpy(Result, "Region");
					break;
				case 31:
					strcpy(Result, "VTH_D");
					break;
				default:
					printf("auxfunc_measurefromlis.c - CMOSLine2Text -- entry must be smaller than 31\n");
					exit(EXIT_FAILURE);
			}
			break;
		case 2: /*HSPICE*/
			/*
			 * The next block was added due to the new line in HSPICE 2001.2
			 * with the operating region                                     */
			if ((fLIS=fopen(OutputFile,"rt")) == 0) {
				printf("auxfunc_measurefromlis.c - CMOSLine2Text -- Cannot open output file: %s\n", OutputFile);
				exit(EXIT_FAILURE);
			}
			fgets2(lkk3, LONGSTRINGSIZE, fLIS);

			j = (strpos2(lkk3, "-- 200", 1)  ); /* Due to HSPICE 2001.2 line */
			if (lkk3[j+5] != '0')               /*          "                */
				measure_x_lines_below--;             /* with the operation region */

			if (fLIS != NULL)
				fclose(fLIS);
			/* end of block
			 *
			 */
			switch (measure_x_lines_below) { /*we have to read a number instead of text*/
				case 2:
					strcpy(Result, "id");
					break;
				case 3:
					strcpy(Result, "ibs");
					break;
				case 4:
					strcpy(Result, "ibd");
					break;
				case 5:
					strcpy(Result, "vgs");
					break;
				case 6:
					strcpy(Result, "vds");
					break;
				case 7:
					strcpy(Result, "vbs");
					break;
				case 8:
					strcpy(Result, "vth");
					break;
				case 9:
					strcpy(Result, "vdsat");
					break;
				case 10:
					strcpy(Result, "beta");
					break;
				case 11:
					strcpy(Result, "gam eff");
					break;
				case 12:
					strcpy(Result, "gm");
					break;
				case 13:
					strcpy(Result, "gds");
					break;
				case 14:
					strcpy(Result, "gmb");
					break;
				case 15:
					strcpy(Result, "cdtot");
					break;
				case 16:
					strcpy(Result, "cgtot");
					break;
				case 17:
					strcpy(Result, "cstot");
					break;
				case 18:
					strcpy(Result, "cbtot");
					break;
				case 19:
					strcpy(Result, "cgs");
					break;
				case 20:
					strcpy(Result, "cgd");
					break;
				default:
					printf("auxfunc_measurefromlis.c - CMOSLine2Text -- entry must be smaller than 21\n");
					exit(EXIT_FAILURE);
			}
			break;
		case 3: /*LTspice*/
			printf("auxfunc_measurefromlis.c - CMOSLine2Text -- LTspice not supported\n");
			exit(EXIT_FAILURE);
		case 4: /*Spectre*/
			printf("auxfunc_measurefromlis.c - CMOSLine2Text -- Spectre not supported\n");
			exit(EXIT_FAILURE);
		case 50: /*Qucs*/
			printf("auxfunc_measurefromlis.c - CMOSLine2Text -- Qucs not supported\n");
			exit(EXIT_FAILURE);
		case 51: /*ngspice*/
			printf("auxfunc_measurefromlis.c - CMOSLine2Text -- ngspice not supported\n");
			exit(EXIT_FAILURE);
		case 100: /*general*/
			printf("auxfunc_measurefromlis.c - CMOSLine2Text -- GENERAL not supported\n");
			exit(EXIT_FAILURE);
		default:
			printf("auxfunc_measurefromlis.c - CMOSLine2Text -- Something unexpected has happened!\n");
			exit(EXIT_FAILURE);
	}

	return Result;
} /*CMOSLine2Text*/




/*
 * Receives "MEASURE_VAR" ASCII line and fills data in variable 'measure[i]'.
 */
int ProcessMeasureVar(char *measure_var_line, int k, char *OutputFile)
{
	int a, b, c, i, j;
	char lkk1[LONGSTRINGSIZE], lkk2[LONGSTRINGSIZE], lkk3[LONGSTRINGSIZE], laux[LONGSTRINGSIZE];
	char STR1[LONGSTRINGSIZE];
	/*char STR2[LONGSTRINGSIZE];*/
	char STR3[LONGSTRINGSIZE];

/*------------------------------------------------------------------*/
	i = 1;
	j = strpos2(measure_var_line, "$", 1); /*This will skip the characters after '$', the inline comment used by the sweep tools*/
	if (j != 0)
		sprintf(measure_var_line, "%.*s", (int)(j - 1), strcpy(STR1, measure_var_line));
	StripSpaces(measure_var_line);

	ReadSubKey(measure[k].var_name, measure_var_line, &i, ':', ':', 5);         /* var_name */
	StripSpaces(measure[k].var_name);
	ReadSubKey(lkk1, measure_var_line, &i, ':', ':', 5);
	StripSpaces(lkk1);
	if (!strcmp(lkk1,"SEARCH_FOR"))
		ReadSubKey(measure[k].search, measure_var_line, &i, '\'', '\'', 5); /* search */
	else { /* if "SEARCH_FOR" does not exist, "MATH" must be found*/
		if ( !strpos2(measure_var_line, "MATH", 1) ) {
			printf("auxfunc_measurefromlis.c - ProcessMeasureVar -- Incorrect line format: %s\n", measure_var_line);
			exit(EXIT_FAILURE);
		}
	}

	/*Is the current variable a transistor?*/
	if ( strpos2(measure_var_line, "SEARCH_FOR", 1) && (!strpos2(measure_var_line, "S_COL", 1)) && strpos2(measure_var_line, "P_LINE", 1) && (!strpos2(measure_var_line, "P_COL", 1)) ) {
	/*yes, it is a transistor*/
		sprintf(STR3, "(%s)", measure[k].var_name);
		strcpy(measure[k].var_name, STR3);

		strsub(lkk1, measure_var_line, strpos2(measure_var_line, "P_LINE", 1) + 6, (int)strlen(measure_var_line));
		//character case is needed for Eldo and LTspice// Str2Lower(lkk1); /*Just to avoid crazy engineers*/
		/* lkk1[0] = ','; */

		a = 1;
		b = 1;
		c = 0;   /*How many measuremets to do within this transistor*/
		while (a < (int)strlen(lkk1)) {
			c++;
			ReadSubKey(lkk2, lkk1, &a, ':', ':', 0);
			StripSpaces(lkk2);

			/*Detects mathematical operator*/
			*laux = '\0';
			if (strpos2(lkk2, "+", 1) != 0) {
				lkk2[strpos2(lkk2, "+", 1) - 1] = ':';
				sprintf(lkk2, ":%s", strcpy(STR3, lkk2));
				strcpy(laux, "+");
			}
			if (strpos2(lkk2, "-", 1) != 0) {
				lkk2[strpos2(lkk2, "-", 1) - 1] = ':';
				sprintf(lkk2, ":%s", strcpy(STR3, lkk2));
				strcpy(laux, "-");
			}
			if (strpos2(lkk2, "*", 1) != 0) {
				lkk2[strpos2(lkk2, "*", 1) - 1] = ':';
				sprintf(lkk2, ":%s", strcpy(STR3, lkk2));
				strcpy(laux, "*");
			}
			if (strpos2(lkk2, "/", 1) != 0) {
				lkk2[strpos2(lkk2, "/", 1) - 1] = ':';
				sprintf(lkk2, ":%s", strcpy(STR3, lkk2));
				strcpy(laux, "/");
			}

			if (*laux == '\0') { /*the line does not contains a mathematical request*/
				/*0*/
				b = (sscanf(lkk2, "%d", &measure[k].line) == 0);         /*if (b=0) then lkk2 contains a number and not text*/
				if (b != 0)                                              /*we have to read text instead of a number*/
					measure[k].line = CMOSText2Line(lkk2, OutputFile);
				else {
					b=1; /*b must always be greater than 0 to avoid an error in ReadSubKey*/
					CMOSLine2Text(lkk2, measure[k].line, OutputFile); /*we have to read a number instead of text*/
				}
				measure[k].column1 = 0;
				measure[k].column2 = 0;
				sprintf(STR3, "%s(%s)", lkk2, ReadSubKey(STR1, measure[k - c + 1].var_name, &b, '(', ')', 0));
				strcpy(measure[k].var_name, STR3);
				if (k != (k-c+1)) /*Do not copy if source and destination are the same, which happens in the first measurement in a MEASURE_VAR line*/
					strcpy(measure[k].search, measure[k - c + 1].search);
				k++;
				if (k > (MAXMEAS-1)) {
					printf("auxfunc_measurefromlis.c - ProcessMeasureVar -- Cannot do more than %d MEASUREMENTS\n", MAXMEAS);
					exit(EXIT_FAILURE);
				}
				/*0*/
			} else {             /*the line contains a mathematical request*/
				b = 1;
				strcpy(lkk3, lkk1);
				strcpy(lkk1, lkk2);

				ReadSubKey(lkk2, lkk1, &b, ':', ':', 0);
				StripSpaces(lkk2);

				/*1*/
				b = (sscanf(lkk2, "%d", &measure[k].line) == 0);        /*if (b=0) then lkk2 contains a number and not text*/
				if (b != 0)                                             /*we have to read text instead of a number*/
					measure[k].line = CMOSText2Line(lkk2, OutputFile);
				else
					CMOSLine2Text(measure_var_line, measure[k].line, OutputFile); /*we have to read a number instead of text*/
				measure[k].column1 = 1; /* In this way, the intermediate measure is not print */
				measure[k].column2 = 0; /* in the summary.txt file, only the arithmetic value */
				sprintf(STR1, "%s(%s)", lkk2, ReadSubKey(STR3, measure[k - c + 1].var_name, &b, '(', ')', 0));
				strcpy(measure[k].var_name, STR1);
				strcpy(measure[k].search, measure[k - c + 1].search);
				k++;
				if (k > (MAXMEAS-1)) {
					printf("auxfunc_measurefromlis.c - ProcessMeasureVar -- Cannot do more than %d MEASUREMENTS\n", MAXMEAS);
					exit(EXIT_FAILURE);
				}
				/*1*/

				b = 2;
				c++;
				ReadSubKey(lkk2, lkk1, &b, ':', ':', 0);
				StripSpaces(lkk2);

				/*2*/
				b = (sscanf(lkk2, "%d", &measure[k].line) == 0);             /*if (b=0) then lkk2 contains a number and not text*/
				if (b != 0)                                                  /*we have to read text instead of a number*/
					measure[k].line = CMOSText2Line(lkk2, OutputFile);
				else
					CMOSLine2Text(lkk2, measure[k - 1].line, OutputFile); /*we have to read a number instead of text*/
				measure[k].column1 = 1; /* In this way, the intermediate measure is not print */
				measure[k].column2 = 0; /* in the summary.txt file, only the arithmetic value */
				sprintf(STR3, "%s(%s)", lkk2, ReadSubKey(STR1, measure[k - c + 1].var_name, &b, '(', ')', 0));
				strcpy(measure[k].var_name, STR3);
				strcpy(measure[k].search, measure[k - c + 1].search);
				k++;
				if (k > (MAXMEAS-1)) {
					printf("auxfunc_measurefromlis.c - ProcessMeasureVar -- Cannot do more than %d MEASUREMENTS\n", MAXMEAS);
					exit(EXIT_FAILURE);
				}
				/*2*/

				b = 1;
				c++;
				/*operation*/
				strcpy(lkk1, strsub(STR1, lkk1, 2, (int)strlen(lkk1)));
				lkk1[strpos2(lkk1, ":", 1) - 1] = laux[0];
				sprintf(STR1, "%s(%s)", lkk1, ReadSubKey(STR3, measure[k - 1].var_name, &b, '(', ')', 0));
				strcpy(measure[k].var_name, STR1);
				sprintf(lkk1, "%i", k - 2);
				sprintf(lkk2, "math:&%s:&", lkk1);
				sprintf(lkk1, "%i", k - 1);
				sprintf(laux, "%s%s:%s", lkk2, lkk1, strcpy(STR1, laux));
				strcpy(measure[k].search, laux);
				k++;
				if (k > (MAXMEAS-1)) {
					printf("auxfunc_measurefromlis.c - ProcessMeasureVar -- Cannot do more than %d MEASUREMENTS\n", MAXMEAS);
					exit(EXIT_FAILURE);
				}
				/*operation*/

				strcpy(lkk1, lkk3);
			}
		}

		k--; /*Corrects the number of measurements to do*/
	} else {
	/*no, it is not a transistor*/
		strcpy(STR1, measure_var_line);
		Str2Lower(STR1);
		j = strpos2(STR1, "math", 1);
		if (j == 0) { /*if equal to '0', we are reading a measurement*/
			ReadSubKey(STR1, measure_var_line, &i, ':', ':', 0);
			j = (sscanf(ReadSubKey(STR1, measure_var_line, &i, ':', ':', 5), "%d", &measure[k].s_column1) == 0); /* s_column1 */
			ReadSubKey(STR1, measure_var_line, &i, ':', ':', 5);
			j = (sscanf(ReadSubKey(STR1, measure_var_line, &i, ':', ':', 5), "%d", &measure[k].line) == 0);      /* line      */
			ReadSubKey(STR1, measure_var_line, &i, ':', ':', 5);
			j = (sscanf(ReadSubKey(STR1, measure_var_line, &i, ':', ':', 5), "%d", &measure[k].column1) == 0);   /* column1   */
			j = (sscanf(ReadSubKey(STR1, measure_var_line, &i, ':', ':', 4), "%d", &measure[k].column2) == 0);   /* column2   */
		} else { /*we will read a MATH line*/
			 /*j holds the starting position of the MATH line*/
			strsub(measure[k].search, measure_var_line, (int)j, (int)strlen(measure_var_line));
		}
	}
/*------------------------------------------------------------------*/
	return k;
} /*ProcessMeasureVar*/




/*
 * Read commands from config file and fills measure[i].<xxx> variable.
 * Returns the number of total measurements do to.
 */
int ReadDataFromConfigFile(char *ConfigFile, char *OutputFile)
{
	int k;
	FILE *fsweepINI;
	char STR2[LONGSTRINGSIZE];


	if ((fsweepINI=fopen(ConfigFile,"rt")) == 0) {
		printf("auxfunc_measurefromlis.c - ReadDataFromConfigFile -- Cannot open config file: %s\n", ConfigFile);
		exit(EXIT_FAILURE);
	}

	ReadKey(lkk, "MEASURE_VAR", fsweepINI);
	if (strcmp((sprintf(STR2, "%.11s", lkk), STR2), "MEASURE_VAR")) {
		printf("INFO:  auxfunc_measurefromlis.c - ReadDataFromConfigFile -- MEASURE_VAR not found\n");
		return EXIT_SUCCESS;
	}

	switch(spice) {
		case 1: /*Eldo*/
			strcpy(measure[0].var_name, "Simulation Conditions");
			strcpy(measure[0].search, "****************************************************");
			measure[0].s_column1 = 1;
			/*@@    measure[1].s_column2:=37;*/
			measure[0].line = 2;   /*-1: ??????*/
			measure[0].column1 = 1; measure[0].column2 = 0;
			break;
		case 2: /*HSPICE*/
			strcpy(measure[0].var_name, "Simulation Conditions");
			strcpy(measure[0].search, "  ******    alter processing listing");
			measure[0].s_column1 = 1;
			/*@@    measure[1].s_column2:=37;*/
			measure[0].line = 0;   /*-1: ??????*/
			measure[0].column1 = 1; measure[0].column2 = 0;
			break;
		case 3: /*LTspice*/
			strcpy(measure[0].var_name, "Simulation Conditions");
			strcpy(measure[0].search, "Not To Be Found");
			break;
		case 4: /*Spectre*/
			strcpy(measure[0].var_name, "Simulation Conditions");
			strcpy(measure[0].search, "Alter Group `");
//---------------------	strcpy(measure[0].search, "Operating-Point information `mc");
			measure[0].s_column1 = 1;
			/*@@    measure[1].s_column2:=37;*/
			measure[0].line = 0;   /*-1: ??????*/
			measure[0].column1 = 1; measure[0].column2 = 0;
			break;
		case 50: /*Qucs*/
		case 51: /*ngspice*/
		case 100: /*general*/
			strcpy(measure[0].var_name, "Simulation Conditions");
			strcpy(measure[0].search, "Not To Be Found");
			break;
		default:
			printf("auxfunc_measurefromlis.c - ReadDataFromConfigFile -- Something unexpected has happened!\n");
			exit(EXIT_FAILURE);
	}
	k = 1; /*starts in 1, because the first one is '******    alter processing listing'*/
/*------------------------------------------------------------------*/
	/*BEGIN: read data from SWEEP.INI file*/
	while (!strcmp((sprintf(STR2, "%.11s", lkk), STR2), "MEASURE_VAR") && k< MAXMEAS) {
		k=ProcessMeasureVar(lkk, k, OutputFile);

		k++;
		if (k > (MAXMEAS-1)) {
			printf("auxfunc_measurefromlis.c - ReadDataFromConfigFile -- Cannot do more than %d MEASUREMENTS\n", MAXMEAS);
			exit(EXIT_FAILURE);
		}

		ReadKey(lkk, "MEASURE_VAR", fsweepINI);
	}
	/*END: read data from SWEEP.INI file*/
/*------------------------------------------------------------------*/
	if (fsweepINI != NULL)
		fclose(fsweepINI);
	fsweepINI = NULL;
	return k; /*returns the number of total measurements do to*/
} /*ReadDataFromConfigFile*/




/*
 * Processes output file. Read measurements from 'OutputFile' and stores
 * each value in variable "measure[i].data" which is paired with variable
 * "measure[i]". Once a set of 'k' measurements is done, their value is written
 * to file using 'WriteToFile', or to memory using "WriteToMem' depending on
 * the value of variable 'mem' =>1:mem, =>2:file, =>3:mem+file.
 */
void ProcessOutputFile(char *OutputFile, int mem)
{
	int a, b=0, c, i, j, k, l;
	char lkk1[LONGSTRINGSIZE], lkk2[LONGSTRINGSIZE];
	char lprevious[LONGSTRINGSIZE], lnext[LONGSTRINGSIZE]; /*previous and next line in the simulation output file; lkk hold current line*/
	char lelement[LONGSTRINGSIZE]; /*when a transistor is found, the line with their names is stored in here*/
	int measure_p_line[MAXMEAS];
	int read_p_line;
	statistics stats;
	FILE *fLIS, *fSummary;
	/*FILE *fsweepINI;*/
	int first;
	int index[10];
	/*char STR1[LONGSTRINGSIZE];*/
	char STR2[LONGSTRINGSIZE];
	/*char STR3[LONGSTRINGSIZE];*/

/*------------------------------------------------------------------*/
/*------------------------------------------------------------------*/
	/*create summary: table version*/
	for (i = 0; i <= (MAXMEAS-1); i++) {
		measure_p_line[i] = 255;
		*measure[i].data = '\0';

		stats.avg[i] = 0.0;   /*initialization of statistics variables*/
		stats.sig[i] = 0.0;
		stats.max[i] = -1.7976931348623157e+308; /*DBL_MAX from <float.h>*/
		stats.min[i] = +1.7976931348623157e+308; /*DBL_MAX from <float.h>*/
		stats.num[i] = 0.0;
	}

	if ((fLIS=fopen(OutputFile,"rt")) == 0) { /* *.lis filename */
		printf("auxfunc_measurefromlis.c - ProcessOutputFile -- Cannot open output file: %s\n", OutputFile);
		exit(EXIT_FAILURE);
	}
	fSummary = NULL;
	if (mem&2) {
		if ((fSummary=fopen(Summary,"wt")) == 0) { /*summary.txt file*/
			printf("auxfunc_measurefromlis.c - ProcessOutputFile -- Cannot open summary file: %s\n", Summary);
			exit(EXIT_FAILURE);
		}
	}

	j=1;
	while ((*measure[j].var_name) != '\0')
		j++; /*number of total measurements*/

	i = 0;
	j--; /*j=(number of total measurements-1)*/
	l = 0;
	first = TRUE; /*assumed to avoid printing empty line*/


	/*BEGIN: process file*/
	fgets2(lnext, LONGSTRINGSIZE, fLIS);
	while ( (!P_eof(fLIS)) || (strcmp(lkk,lnext)) ) {
		/*0*/
		strcpy(lprevious, lkk);              /* previous line */
		strcpy(lkk, lnext);                  /* current line  */
		fgets2(lnext, LONGSTRINGSIZE, fLIS); /* line ahead    */

		/*Find out how many colums with transitors, and their position*/
		if (IsItATransistor(lkk, 0, 0, lnext)) {   /*Does the current line containes transistors?*/
			b=DetectsTransistorColumns(lkk, index); /*b=number of columns with transistors*/
		}
		/*0*/

		/*1*/
		read_p_line = FALSE;
		for (k = 0; k <= j; k++) { /*1- for each one of the input lines, look if it is necessary to measure something*/
			/*Step lkk1*/
			strsub(lkk1, lkk, (int)measure[k].s_column1, (int)strlen(measure[k].search) + 0);
			//-- StripSpaces(lkk1); /*necessary when we want to find '******    alter processing listing'*/

			/*Step lkk2*/
			c = 0;
			if (IsItATransistor(lkk, 0, 0, lnext)) {                       /*if it is a transistor, I know the format*/
				switch(spice) {
					case 1: /*Eldo*/
					case 2: /*HSPICE*/
					case 3: /*LTspice*/
						for (a = 1; a <= b; a++) {
							if (strpos2(lkk, measure[k].search, 1) == index[a - 1])
								c = a;
						}
						break;
					case 4: /*Spectre*/
						if (strpos2(lkk, " of ", 1)) { /*to remove the library name if and when it appears*/
							lkk[strpos2(lkk, " of ", 1)]='\0';
						}
						c=1;
						break;
					case 50: /*Qucs*/
						printf("auxfunc_measurefromlis.c - ProcessOutputFile -- Qucs not supported\n");
						exit(EXIT_FAILURE);
						break;
					case 51: /*ngspice*/
						printf("auxfunc_measurefromlis.c - ProcessOutputFile -- ngspice not supported\n");
						exit(EXIT_FAILURE);
						break;
					case 100: /*general*/
						printf("auxfunc_measurefromlis.c - ProcessOutputFile -- GENERAL ot supported\n");
						exit(EXIT_FAILURE);
						break;
					default:
						printf("auxfunc_measurefromlis.c - ProcessOutputFile -- Something unexpected has happened!\n");
						exit(EXIT_FAILURE);
				}
			}
			if (c != 0) {                                                  /*if !=0, then we have found a transistor definition*/
				strcpy(lelement, lkk); /*saves, because 'lkk' will be destroyed afterwards*/
				strsub(lkk2, lkk, (int)index[c - 1], (int)(index[c] - index[c - 1]));
				StripSpaces(lkk2);
			} else {                                                       /*Its not a transistor(check follows)                */
				if (!IsItATransistor(measure[k].search, 1, k, '\0')) { /*proceeds only if 'measure[k].' is not a transistor */
					if (measure[k].column1 == 0 && measure[k].column2 == 0) /*we want to read the data in front*/
						strsub(lkk2, lkk, strpos2(lkk, measure[k].search, 1), (int)strlen(measure[k].search));
					else
						strsub(lkk2, lkk, (int)measure[k].s_column1, (int)strlen(measure[k].search));
				}
			}

			/*Step lkk1/2*/
			if (!strcmp(lkk1, measure[k].search) || !strcmp(lkk2, measure[k].search)) {
				measure_p_line[k] = measure[k].line; /*data to read has been found at measure[k].line from the current line of text!*/
			}
		}
		/*1*/

		/*2*/
		read_p_line = FALSE;       /*2- assumes that there is nothing to read in the _current_ line of text*/
		for (k = 0; k <= j; k++) {
			if (measure_p_line[k] == 0)
				read_p_line = TRUE; /*there is data to read in the current line of text*/
		}
		/*2*/

		/*3*/
		if (read_p_line) {         /*3- if data to measure has been found, then proced with the measurements*/
			strsub(lkk1, lkk, (int)measure[0].s_column1, (int)strlen(measure[0].search) + 0);
			//-- StripSpaces(lkk1); /*necessary when we want to find '******    alter processing listing'*/
			switch(spice) {                                          /*                                                                */
				case 1: /*Eldo*/                                 /*                                                                */
					if (strpos2(lkk, ".ALTER @", 1))         /*To overcome Eldo case where the search line in measure[0].search*/
						strcpy(lkk1, measure[0].search); /*is different from the line having the ALTER information, copy   */
					break;                                   /*to lkk1, so that is passes the comparison below as with HSPICE  */
				case 2: /*HSPICE*/                               /*                                                                */
				case 3: /*LTspice*/                              /*                                                                */
					break;                                   /*                                                                */
				case 4: /*Spectre*/                              /*                                                                */
//---------------------			if (strpos2(lkk, "Alter Group `", 1))    /*The same as for Eldo is done for Spectre when searching the line*/
//---------------------				strcpy(lkk1, measure[0].search); /*Alter Group ` ...                                               */
					break;                                   /*                                                                */
				case 50: /*Qucs*/                                /*                                                                */
				case 51: /*ngspice*/                             /*                                                                */
				case 100: /*general*/                            /*                                                                */
					break;                                   /*                                                                */
				default:                                         /*                                                                */
					printf("auxfunc_measurefromlis.c - ProcessOutputFile -- Something unexpected has happened!\n");
					exit(EXIT_FAILURE);                      /*                                                                */
			}                                                        /*                                                                */
			if (!strcmp(lkk1, measure[0].search) && (int)strlen(lkk1)>10) { /*Was ALTER simulation found?*/
				if (mem&1)
					WriteToMem(j);
				if (mem&2) {
					switch(spice) {
						case 1: /*Eldo*/
							WriteToFile(j, lkk, first, &stats, &fSummary);
							break;
						case 2: /*HSPICE*/
							WriteToFile(j, lprevious, first, &stats, &fSummary);
							break;
						case 3: /*LTspice*/
						case 4: /*Spectre*/
						case 50: /*Qucs*/
						case 51: /*ngspice*/
						case 100: /*general*/
							WriteToFile(j, lprevious, first, &stats, &fSummary);
							break;
						default:
							printf("auxfunc_measurefromlis.c - ProcessOutputFile -- Something unexpected has happened!\n");
							exit(EXIT_FAILURE);
					}
				}
				i++;
				first = FALSE;
				for (l = 1; l <= (MAXMEAS-1); l++) /*After using the measured information,*/
					*measure[l].data = '\0';   /*delete all data that has been stored */
				switch(spice) {
					case 1: /*Eldo*/
						StripSpaces(lkk);
						strcpy(measure[0].data, lkk);
						break;
					case 2: /*HSPICE*/
						StripSpaces(lprevious);
						strcpy(measure[0].data, lprevious);
						break;
					case 3: /*LTspice*/
						strcpy(measure[0].data, "");
						break;
					case 4: /*Spectre*/
						StripSpaces(lkk);
						l=1;
						ReadSubKey(lkk1, lkk, &l, '`', '\'', 5);
						strcpy(measure[0].data, lkk1);
						break;
					case 50: /*Qucs*/
					case 51: /*ngspice*/
					case 100: /*general*/
						strcpy(measure[0].data, "");
						break;
					default:
						printf("auxfunc_measurefromlis.c - ProcessOutputFile -- Something unexpected has happened!\n");
						exit(EXIT_FAILURE);
				}
			}

			for (k = 1; k <= j; k++) {      /*search for the remaining ones*/
				if (measure_p_line[k] == 0) { /*read all data in the current line*/
					if (*measure[k].data == '\0') {
								/*Do nothing, just go on!!*/
					} else {            /*'Monte Carlo' or similar*/
						if (mem&1)
							WriteToMem(j);
						if (mem&2)
							WriteToFile(j, lprevious, first, &stats, &fSummary); /*A: write to file so data previously read is not overwritten*/
						i++;
						first = FALSE;
						for (l = 1; l <= (MAXMEAS-1); l++) /* After using the measured information, */
							*measure[l].data = '\0';   /* delete all data that has been stored  */
						switch(spice) {
							case 1: /*Eldo*/
							case 2: /*HSPICE*/
							case 3: /*LTspice*/
								break;
							case 4: /*Spectre*/
									strcpy(measure[0].search, "Operating-Point information `mc");
								break;
							case 50: /*Qucs*/
							case 51: /*ngspice*/
							case 100: /*general*/
								break;
							default:
								printf("auxfunc_measurefromlis.c - ProcessOutputFile -- Something unexpected has happened!\n");
								exit(EXIT_FAILURE);
						}
					}
													     /*B: now, go on with normal procedure*/
					if (IsItATransistor(measure[k].search, 1, k, '\0')) { /*1: Is the current variable a transistor?*/
						for (a = 1; a <= b; a++) {                    /*2: yes, it is a transistor*/
							if (strpos2(lelement, measure[k].search, 1) == index[a - 1]) {
								switch(spice) {
									case 1: /*Eldo*/
									case 2: /*HSPICE*/
									case 3: /*LTspice*/
										strsub(measure[k].data, lkk, (int)index[a - 1], (int)(index[a] - index[a - 1]));
										break;
									case 4: /*Spectre*/
										strsub(measure[k].data, lkk, (int)index[a - 1]+4, 15);
										i=strpos2(measure[k].data, " ", 1); //i !=0 for any other only
										if (i){
											measure[k].data[i-1]='\0';
											switch (measure[k].data[i]) { /*finds multiple: t,g,x,k, m,u,n,p,f,a,z*/
												case 't':   /*T: tera*/
													strcat(measure[k].data, "e+12");
													break;
												case 'g':   /*G: giga*/
													strcat(measure[k].data, "e+09");
													break;
												case 'x':   /*M: mega*/
													strcat(measure[k].data, "e+06");
													break;
												case 'k':   /*k: kilo*/
													strcat(measure[k].data, "e+03");
													break;
												case 'm':   /*m: mill*/
													strcat(measure[k].data, "e-03");
													break;
												case 'u':   /*u: micro*/
													strcat(measure[k].data, "e-06");
													break;
												case 'n':   /*n: nano*/
													strcat(measure[k].data, "e-09");
													break;
												case 'p':   /*p: pico*/
													strcat(measure[k].data, "e-12");
													break;
												case 'f':   /*f: femto*/
													strcat(measure[k].data, "e-15");
													break;
												case 'a':   /*a: atto*/
													strcat(measure[k].data, "e-18");
													break;
												case 'z':   /*z: zepto*/
													strcat(measure[k].data, "e-21");
													break;
												case '1':   /*1: one*/
												case 'A':   /*A: Ampere*/
												case 'V':   /*V: Volt*/
												case 'O':   /*O: Ohm*/
												case 'S':   /*S: */
													break;
												default:
													printf("auxfunc_measurefromlis.c - ProcessOutputFile -- Something unexpected has happened!\n");
													exit(EXIT_FAILURE);
											}
										}
										break;
									case 50: /*Qucs*/
										printf("auxfunc_measurefromlis.c - ProcessOutputFile -- Qucs not supported\n");
										exit(EXIT_FAILURE);
									case 51: /*ngspice*/
										printf("auxfunc_measurefromlis.c - ProcessOutputFile -- ngspice not supported\n");
										exit(EXIT_FAILURE);
									case 100: /*general*/
										printf("auxfunc_measurefromlis.c - ProcessOutputFile -- GENERAL not supported\n");
										exit(EXIT_FAILURE);
									default:
										printf("auxfunc_measurefromlis.c - ProcessOutputFile -- Something unexpected has happened!\n");
										exit(EXIT_FAILURE);
								}
							}
						}
					} else {                                              /*3: no, it is not a transistor*/
						if (measure[k].column1 == 0 && measure[k].column2 == 0) {    /*C: Do we want to read the data in front?*/
							strcpy(lkk1, lkk);                                   /*D: yes*/

							/* StripSpaces(lkk1);  this line only creates problems. */
							a = strpos2(lkk1, measure[k].search, 1) + (int)strlen(measure[k].search);
							strcpy(lkk1, strsub(STR2, lkk1, (int)a, (int)strlen(lkk)));

							StripSpaces(lkk1);
							a = strpos2(lkk1, " ", 1);
							if (a == 0)
								a = (int)strlen(lkk1);
							sprintf(lkk1, "%.*s", (int)a, strcpy(STR2, lkk1));

							/* StripSpaces(lkk1); unnecessary */
							strcpy(measure[k].data, lkk1);
						} else                                                       /*E: no, general purpose variable extraction*/
							strsub(measure[k].data, lkk, (int)measure[k].column1, (int)(measure[k].column2 - measure[k].column1 + 1));
					}
				}
			}
		}
		/*3*/

		/*4*/
		for (k = 0; k <= j; k++) {
			if (measure_p_line[k] == 0)       /*replace all '0' by '255', meaning that has already been read*/
				measure_p_line[k] = 255;
			if (measure_p_line[k] != 255)
				measure_p_line[k]--;      /*decrement all != '255' by '1', those that will be read*/
		}
		/*4*/
	}
	/*END: process file*/

	if (mem&1)
		WriteToMem(j);
	if (mem&2) {
		WriteToFile(j, lprevious, first, &stats, &fSummary);
		i++;
		WriteStats(j, i, stats, &fSummary);
	}

	if (fSummary != NULL)
		fclose(fSummary);
	fSummary = NULL;
	if (fLIS != NULL)
		fclose(fLIS);
	fLIS = NULL;
/*------------------------------------------------------------------*/
/*------------------------------------------------------------------*/
} /*ProcessOutputFile*/




/*
 *
*/
void MeasureFromLIS(char *ConfigFile, char *OutputFile)
{
	/*retrieve measures from *.lis file*/
	int k;

	k=ReadDataFromConfigFile(ConfigFile, OutputFile);
	if (k)/*if measurements to do exist*/
		ProcessOutputFile(OutputFile, 3);
} /*MeasureFromLIS*/
