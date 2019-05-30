/*
 *  Copyright (C) 1999-2005 Joao Ramos
 * Your use of this code is subject to the terms and conditions of the
 * GNU general public license version 2. See "COPYING" or
 * http://www.gnu.org/licenses/gpl.html
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "auxfunc_measurefromlis.h"
#include "auxfunc.h"


//void WriteToMem(int num_measures){}

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
		case 1: //Eldo
			if (mem) { //mem=1
				ReadSubKey(lkk1, measure[i].var_name, &j, '(', ')', 0);
				if ( (strlen(lkk1)) && (measure[i].s_column1==0) && (measure[i].column2==0) )
					return 1; //its a transistor
				else
					return 0;
			} else {  //mem=0
				strsub(lkk1, line, 1, 9);
				if ( (!strcmp("        M", lkk1)) || (!strcmp("        X", lkk1)) )                                                                //checks first line
					if ( (strpos2(nextline, "MODEL   ", 1) != 0)  && ((strpos2(nextline, "PMOS", 1) != 0) || (strpos2(nextline, "NMOS", 1))) ) //checks second line
						return 1; //its a transistor
					else
						return 0;
				else
					return 0;
			}
			break;
		case 2: //HSPICE
			if (mem) { //mem=1
				ReadSubKey(lkk1, measure[i].var_name, &j, '(', ')', 0);
				if ( (strlen(lkk1)) && (measure[i].s_column1==0) && (measure[i].column2==0) )
					return 1; //its a transistor
				else
					return 0;
			} else {   //mem=0
				if ( (strpos2(line, "element  ", 1) != 0) && (strpos2(line, ":m", 1)) )                                                             //checks first line
					if ( (strpos2(nextline, " model ", 1) != 0)  && ((strpos2(nextline, ":pmos", 1) != 0) || (strpos2(nextline, ":nmos", 1))) ) //checks second line
						return 1; //its a transistor
					else
						return 0;
				else
					return 0;
			}
			break;
		case 3: //LTspice
			return 0;
			break;
		case 100: //ROSEN
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
		case 1: //Eldo
			if (strpos2(lelement, "        M", 1) != 0) {
				while (i < strlen(lelement)) {
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
				while (i < strlen(lelement)) {
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
		case 2: //HSPICE
			while (i < strlen(lelement)) {
				if (lelement[i - 1] == ':') {
					j++;                  /*how many columns with transistors ?*/
					index[j - 1] = i - 1; /*saves column for print*/
					i++;
				} else
					i++;
			}
			index[j] = i + 1;                     /*corrects size for last column*/
			break;
		//case 3: //LTspice
		//	break;
		//case 100: //ROSEN
		//	k
		//	break;
		default:
			printf("auxfunc_measurefromlis.c - DetectsTransistorColumns -- Something unexpected has happened!\n");
			exit(EXIT_FAILURE);
	}
	return j; //returns the number of columns having transistors
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
	//char STR2[LONGSTRINGSIZE], STR3[LONGSTRINGSIZE];

	RPN1 = 1.0;
	RPN2 = 1.0;

	for (i = 0; i <= num_measures; i++) {
		sprintf(STR1, "%.4s", measure[i].search);
		if (!strcmp(STR1, "math")) {   /*if a mathematical operation is requested, then*/
			strcpy(lkk1, " ");
			j=1;
			while (*lkk1 != '\0') {
				//measure[i].search[4] = ';';   /*replace '=' by ';'*/
				ReadSubKey(lkk1, measure[i].search, &j, ':', ':', 0);
				if (*lkk1 == '\0') {
					sprintf(lkk1, "% .5E", RPN1);
					strcpy(measure[i].data, lkk1);
					*lkk1 = '\0';
				}
				StripSpaces(lkk1);

				/*Step1*/
				k=0;
				sprintf(STR1, "%.*s", strlen(lkk1), lkk1);
				l = (sscanf(STR1, "%i", &k) == 0);
				if (lkk1[0] == '&') { /*to read one measurement*/
					l = (sscanf(strsub(STR1, lkk1, 2, strlen(lkk1)), "%i", &k) == 0); //&<digit> format else is
					if (k == 0) {
						for (k = 1; k < i; k++) {                                 //&<text>  format
							//ll = 1;
							if (strpos2(measure[k].var_name, STR1,1)) { // if variable in STR1 exists in measure[k].var_name
								//fflush(0);
								break;
							}
						}
					}
					data = asc2real(measure[k].data, 1, (int)strlen(measure[k].data));
				} else {
					if (k != 0) //to read one number
						data = k;
					else {      //to read a mathematical operator
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
								if (RPN1 != 0)
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
		// -------------- strcpy(lkk1, " ");
		// -------------- j=1;
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

	// {//} Compute the values for statistics
	for (i = 1; i <= num_measures; i++) {
		aux = asc2real(measure[i].data, 1, (int)strlen(measure[i].data));
		stats->avg[i] += aux;
		stats->sig[i] += aux * aux; /*variance=sum(x^2)/n - (avg(x))^2 : sigma=sqrt(variance)*/
		if (aux > stats->max[i])
			stats->max[i] = aux;
		if (aux < stats->min[i])
			stats->min[i] = aux;
	}
	// {//}

	if (num_measures < 13) {
	/*Step1: if to process less than 13 MEASUREMENTS*/
		*line = '\0';
		StripSpaces(laux);

		if (*measure[0].data == '\0') {       /*Was ALTER found?*/
			if (laux[0] != '@')           /*ALTER was not found*/
				strcpy(laux, "                     ");
			FORLIM = strlen(laux);
			for (i = 1; i <= FORLIM; i++) /*ALTER was found*/
				strcat(line, " ");
		}

		for (i = 0; i <= num_measures; i++) { /*Build measured line*/
			if ((!IsItATransistor(measure[i].search, 1, i, '\0')) || measure[i].column1 == 0) { //print only 'vgs-vth' and not 'vgs' and 'vth'.
				strcpy(laux1, measure[i].data);
				//        if (i > 0) {
				aux = asc2real(laux1, 1, (int)strlen(laux1));
				j = extended2engineer(&aux);
				//debug
				if ((aux!=0) ) { // we are going to read a number
					sprintf(laux1, "%7.3f", aux);
					sprintf(laux2, "%i", j);
					sprintf(line + strlen(line), "%se%s | ", laux1, laux2);
				//debug
				} else {         // we are going to read text instead of a number
					StripSpaces(measure[i].data);
					sprintf(line + strlen(line), "%s | ", measure[i].data);
				}
			}
		}

		ptr = 1; /*Build header line*/
		if (*measure[0].data == '\0' && first) {
			strcpy(lineHeader, line);
			FORLIM = strlen(lineHeader);
			for (i = 0; i < FORLIM; i++) {
				if (lineHeader[i] != '|')
					lineHeader[i] = ' ';
			}

			for (i = 0; i <= num_measures; i++) {
				if ((!IsItATransistor(measure[i].search, 1, i, '\0')) || measure[i].column1 == 0) { //print only 'vgs-vth' and not 'vgs' and 'vth'.
					strcpy(laux1, measure[i].var_name);
					FORLIM1 = strlen(measure[i].var_name);
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
			if ((!IsItATransistor(measure[i].search, 1, i, '\0')) || measure[i].column1 == 0) { //print only 'vgs-vth' and not 'vgs' and 'vth'.
				strcpy(laux1, measure[i].var_name);
				fprintf(*fSummary, "%s | ", measure[i].var_name);
			}
		}
		putc('\n', *fSummary);
	}

	if (*measure[0].data == '\0') { /*Was ALTER found?*/
		if (laux[0] != '@')     /*ALTER was not found*/
			strcpy(laux, "                     ");
		FORLIM = strlen(laux);
		for (i = 1; i <= FORLIM; i++)
			putc(' ', *fSummary);
	}

	for (i = 0; i <= num_measures; i++) { /*Build measured line*/
		if ((!IsItATransistor(measure[i].search, 1, i, '\0')) || measure[i].column1 == 0) { //print only 'vgs-vth' and not 'vgs' and 'vth'.
			strcpy(laux1, measure[i].data);
			//      if (i > 0) {
			aux = asc2real(laux1, 1, (int)strlen(laux1));
			j = extended2engineer(&aux);
			//debug
			if ((aux!=0) ) { //we are going to read a number
				sprintf(laux1, "%7.3f", aux);
				sprintf(laux2, "%i", j);
				fprintf(*fSummary, "%se%s | ", laux1, laux2);
			//debug
			} else {         // we are going to read text instead of a number
				StripSpaces(measure[i].data);
				fprintf(*fSummary, "%s | ", measure[i].data);
			}
		}
	}
	putc('\n', *fSummary);
	/*Step2*/

}  /*WriteToFile*/



/***************************/
void PrintOneLine(char *lkk1, double *stats, int num_measures, FILE **fSummary)
{
	int i, j;
	char laux[LONGSTRINGSIZE];
	int FORLIM;

	if (*measure[0].data == '\0')
		strcpy(measure[0].data, "                     ");
	FORLIM = strlen(measure[0].data) - 6;
	for (i = 1; i <= FORLIM; i++)
		putc(' ', *fSummary);
	fputs(lkk1, *fSummary);
	for (i = 1; i <= num_measures; i++) {
		if ((!IsItATransistor(measure[i].search, 1, i, '\0')) || measure[i].column1 == 0) { //print only 'vgs-vth' and not 'vgs' and 'vth'.
			j = extended2engineer(&stats[i]);
			sprintf(laux, "%7.3f", stats[i]);
			fprintf(*fSummary, "%se%i | ", laux, j);
		}
	}
	putc('\n', *fSummary);
} /*PrintOneLine*/



/***************************/
void WriteStats(int num_measures, int data_lines, statistics stats, FILE **fSummary)
{
	int i;
	double TEMP;

	putc('\n', *fSummary);
	for (i = 1; i <= num_measures; i++) {
		if (data_lines != 0) {
			stats.avg[i] /= data_lines;     /*average value*/
			TEMP = stats.avg[i];
			stats.sig[i] = sqrt(fabs(stats.sig[i] / data_lines - TEMP * TEMP));
				if (stats.sig[i] < fabs(stats.max[i]/1000))
					stats.sig[i]=0; /*standard deviation*/
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
	FILE *fLIS;             //  Due to HSPICE 2001.2 line
	char lkk3[LONGSTRINGSIZE];         //
	int j;                  //  with the operation region
	int Result=0;

	switch(spice) {
		case 1: //Eldo
			if (!strcasecmp(lkk2, "ID"))
				Result = 3;
			if (!strcasecmp(lkk2, "VGS"))
				Result = 4;
			if (!strcasecmp(lkk2, "VDS"))
				Result = 5;
			if (!strcasecmp(lkk2, "VBS"))
				Result = 6;
			if (!strcasecmp(lkk2, "VTH"))
				Result = 7;
			if (!strcasecmp(lkk2, "VDSAT"))
				Result = 8;
			if (!strcasecmp(lkk2, "GM"))
				Result = 9;
			if (!strcasecmp(lkk2, "GDS"))
				Result = 10;
			if (!strcasecmp(lkk2, "GMB"))
				Result = 11;
			if (!strcasecmp(lkk2, "Cdd"))
				Result = 12;
			if (!strcasecmp(lkk2, "Cdg"))
				Result = 13;
			if (!strcasecmp(lkk2, "Cds"))
				Result = 14;
			if (!strcasecmp(lkk2, "Cdb"))
				Result = 15;
			if (!strcasecmp(lkk2, "Cgd"))
				Result = 16;
			if (!strcasecmp(lkk2, "Cgg"))
				Result = 17;
			if (!strcasecmp(lkk2, "Cgs"))
				Result = 18;
			if (!strcasecmp(lkk2, "Cgb"))
				Result = 19;
			if (!strcasecmp(lkk2, "Csd"))
				Result = 20;
			if (!strcasecmp(lkk2, "Csg"))
				Result = 21;
			if (!strcasecmp(lkk2, "Css"))
				Result = 22;
			if (!strcasecmp(lkk2, "Csb"))
				Result = 23;
			if (!strcasecmp(lkk2, "Cbd"))
				Result = 24;
			if (!strcasecmp(lkk2, "Cbg"))
				Result = 25;
			if (!strcasecmp(lkk2, "Cbs"))
				Result = 26;
			if (!strcasecmp(lkk2, "Cbb"))
				Result = 27;
			if (!strcasecmp(lkk2, "PHI"))
				Result = 28;
			if (!strcasecmp(lkk2, "VBI"))
				Result = 29;
			if (!strcasecmp(lkk2, "Region"))
				Result = 30;
			if (!strcasecmp(lkk2, "VTH_D"))
				Result = 31;
			break;
		case 2: //HSPICE
			if (!strcmp(lkk2, "id"))
				Result = 2;
			if (!strcmp(lkk2, "ibs"))
				Result = 3;
			if (!strcmp(lkk2, "ibd"))
				Result = 4;
			if (!strcmp(lkk2, "vgs"))
				Result = 5;
			if (!strcmp(lkk2, "vds"))
				Result = 6;
			if (!strcmp(lkk2, "vbs"))
				Result = 7;
			if (!strcmp(lkk2, "vth"))
				Result = 8;
			if (!strcmp(lkk2, "vdsat"))
				Result = 9;
			if (!strcmp(lkk2, "beta"))
				Result = 10;
			if (!strcmp(lkk2, "gam eff"))
				Result = 11;
			if (!strcmp(lkk2, "gm"))
				Result = 12;
			if (!strcmp(lkk2, "gds"))
				Result = 13;
			if (!strcmp(lkk2, "gmb"))
				Result = 14;
			if (!strcmp(lkk2, "cdtot"))
				Result = 15;
			if (!strcmp(lkk2, "cgtot"))
				Result = 16;
			if (!strcmp(lkk2, "cstot"))
				Result = 17;
			if (!strcmp(lkk2, "cbtot"))
				Result = 18;
			if (!strcmp(lkk2, "cgs"))
				Result = 19;
			if (!strcmp(lkk2, "cgd"))
				Result = 20;
			//
			// The next block was added due to the new line in HSPICE 2001.2
			// with the operating region
			if ((fLIS=fopen(OutputFile,"rt")) == 0) {
				printf("auxfunc_measurefromlis.c - CMOSText2Line -- Cannot open output file: %s\n", OutputFile);
				exit(EXIT_FAILURE);
			}
			fgets2(lkk3, LONGSTRINGSIZE, fLIS);

			j = (strpos2(lkk3, "-- 200", 1)  );// Due to HSPICE 2001.2 line
			if (lkk3[j+5] != '0')              //
				Result++;                  // with the operation region

			if (fLIS != NULL)
				fclose(fLIS);
			// end of block
			//
			//
			break;
		//case 3: //LTspice
		//	break;
		//case 100: //ROSEN
		//	k
		//	break;
		default:
			printf("auxfunc_measurefromlis.c - CMOSText2Line -- Something unexpected has happened!\n");
			exit(EXIT_FAILURE);
	}




	return Result;
} /*CMOSText2Line*/




/*
 *
 */
char *CMOSLine2Text(char *Result, int measure_line, char *OutputFile)
{
	FILE *fLIS;             //  Due to HSPICE 2001.2 line
	char lkk3[LONGSTRINGSIZE];         //
	int j;                  //  with the operation region

	switch(spice) {
		case 1: //Eldo
			switch (measure_line) {   /*we have to read a number instead of text*/
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
					strcpy(Result, "");
					break;
			}
			break;
		case 2: //HSPICE
			//
			// The next block was added due to the new line in HSPICE 2001.2
			// with the operating region
			if ((fLIS=fopen(OutputFile,"rt")) == 0) {
				printf("auxfunc_measurefromlis.c - CMOSLine2Text -- Cannot open output file: %s\n", OutputFile);
				exit(EXIT_FAILURE);
			}
			fgets2(lkk3, LONGSTRINGSIZE, fLIS);

			j = (strpos2(lkk3, "-- 200", 1)  );// Due to HSPICE 2001.2 line
			if (lkk3[j+5] != '0')              //
				measure_line--;                        // with the operation region

			if (fLIS != NULL)
				fclose(fLIS);
			// end of block
			//
			//
			switch (measure_line) {   /*we have to read a number instead of text*/
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
					strcpy(Result, "");
					break;
			}
			break;
		//case 3: //LTspice
		//	break;
		//case 100: //ROSEN
		//	k
		//	break;
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
	//char STR2[LONGSTRINGSIZE];
	char STR3[LONGSTRINGSIZE];


//------------------------------------------------------------------
	i = 1;
	j = strpos2(lkk, "$", 1); /*This will skip the characters after '$', the inline comment used by the sweep tools*/
	if (j != 0)
		sprintf(lkk, "%.*s", (int)(j - 1), strcpy(STR1, lkk));
	StripSpaces(lkk);

	ReadSubKey(measure[k].var_name, lkk, &i, ':', ':', 5);         //var_name
	StripSpaces(measure[k].var_name);
	ReadSubKey(lkk1, lkk, &i, ':', ':', 5);
	StripSpaces(lkk1);
	if (!strcmp(lkk1,"SEARCH_FOR"))
		ReadSubKey(measure[k].search, lkk, &i, '\'', '\'', 5); //search

	if ( strpos2(lkk, "SEARCH_FOR", 1) && (!strpos2(lkk, "S_COL", 1)) && strpos2(lkk, "P_LINE", 1) && (!strpos2(lkk, "P_COL", 1)) ) {
					/*Is the current variable a transistor?*/
					/*yes, it is a transistor*/
		sprintf(STR3, "(%s)", measure[k].var_name);
		strcpy(measure[k].var_name, STR3);

		strsub(lkk1, lkk, strpos2(lkk, "P_LINE", 1) + 6, strlen(lkk));
		Str2Lower(lkk1); /*Just to avoid crazy engineers*/
		//lkk1[0] = ',';

		a = 1;
		b = 1;
		c = 0;   /*How many measuremets to do within this transistor*/
		while (a < strlen(lkk1)) {
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
				else
					CMOSLine2Text(lkk2, measure[k].line, OutputFile); /*we have to read a number instead of text*/
				measure[k].column1 = 0;
				measure[k].column2 = 0;
				sprintf(STR3, "%s(%s)", lkk2, ReadSubKey(STR1, measure[k - c + 1].var_name, &b, '(', ')', 0));
				strcpy(measure[k].var_name, STR3);
				strcpy(measure[k].search, measure[k - c + 1].search);
				k++;
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
					CMOSLine2Text(lkk, measure[k].line, OutputFile); /*we have to read a number instead of text*/
				measure[k].column1 = 1; //In this way, the intermediate measure is not print
				measure[k].column2 = 0; //in the summary.txt file, only the arithmetic value.
				sprintf(STR1, "%s(%s)", lkk2, ReadSubKey(STR3, measure[k - c + 1].var_name, &b, '(', ')', 0));
				strcpy(measure[k].var_name, STR1);
				strcpy(measure[k].search, measure[k - c + 1].search);
				k++;
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
				measure[k].column1 = 1; //In this way, the intermediate measure is not print
				measure[k].column2 = 0; //in the summary.txt file, only the arithmetic value.
				sprintf(STR3, "%s(%s)", lkk2, ReadSubKey(STR1, measure[k - c + 1].var_name, &b, '(', ')', 0));
				strcpy(measure[k].var_name, STR3);
				strcpy(measure[k].search, measure[k - c + 1].search);
				k++;
				/*2*/

				b = 1;
				c++;
				/*operation*/
				strcpy(lkk1, strsub(STR1, lkk1, 2, strlen(lkk1)));
				lkk1[strpos2(lkk1, ":", 1) - 1] = laux[0];
				sprintf(STR1, "%s(%s)", lkk1, ReadSubKey(STR3, measure[k - 1].var_name, &b, '(', ')', 0));
				strcpy(measure[k].var_name, STR1);
				sprintf(lkk1, "%i", k - 2);
				sprintf(lkk2, "math:&%s:&", lkk1);
				sprintf(lkk1, "%i", k - 1);
				sprintf(laux, "%s%s:%s", lkk2, lkk1, strcpy(STR1, laux));
				strcpy(measure[k].search, laux);
				/*operation*/
				k++;

				strcpy(lkk1, lkk3);
			}
		}

		k--; /*Corrects the number of measurements to do*/
	} else {			/*no, it is not a transistor*/
		Str2Lower(lkk);
		j = strpos2(lkk, "math", 1);
		if (j == 0) { /*if equal to '0', we are reading a measurement*/
			ReadSubKey(STR1, lkk, &i, ':', ':', 0);
			j = (sscanf(ReadSubKey(STR1, lkk, &i, ':', ':', 5), "%d", &measure[k].s_column1) == 0); //s_column1
			ReadSubKey(STR1, lkk, &i, ':', ':', 5);
			j = (sscanf(ReadSubKey(STR1, lkk, &i, ':', ':', 5), "%d", &measure[k].line) == 0);      //line
			ReadSubKey(STR1, lkk, &i, ':', ':', 5);
			j = (sscanf(ReadSubKey(STR1, lkk, &i, ':', ':', 5), "%d", &measure[k].column1) == 0);   //column1
			j = (sscanf(ReadSubKey(STR1, lkk, &i, ':', ':', 4), "%d", &measure[k].column2) == 0);   //column2
		} else { /*we will read a MATH line*/
			 /*j holds the starting position of the MATH line*/
			strsub(measure[k].search, lkk, (int)j, strlen(lkk));
		}
	}
//------------------------------------------------------------------
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

	strcpy(measure[0].var_name, "Simulation Conditions");
	strcpy(measure[0].search, "******    alter processing listing");
	measure[0].s_column1 = 1;
	/*@@    measure[1].s_column2:=37;*/
	measure[0].line = 0;   /*-1: ??????*/
measure[0].column1 = 1; measure[0].column2 = 0;
	k = 1; /*starts in 1, because the first one is '******    alter processing listing'*/
//------------------------------------------------------------------
	/*BEGIN: read data from SWEEP.INI file*/
	while (!strcmp((sprintf(STR2, "%.11s", lkk), STR2), "MEASURE_VAR") && k< MAXMEAS) {
		k=ProcessMeasureVar(lkk, k, OutputFile);

		k++;
		if (k > (MAXMEAS-1)) {
			printf("auxfunc_measurefromlis.c - ReadDataFromConfigFile -- Cannot do more than %d MEASUREMENTS\n", MAXMEAS);
			exit(EXIT_FAILURE);
			//Possible error exist, as 'k' can be greater than 75, while
			//continuing to read variables. The 'MOST variable extraction'
			//operator can generate more the one measure variable before
			//arriving in here. For example:
			//MEASURE_VAR=        m00; SEARCH_FOR='1:m00'; P_LINE=gs-vth
			//generates 3 more variables, while
			//MEASURE_VAR=        m00; SEARCH_FOR='1:m00'; P_LINE=vgs, vth, vds, vgs-vth
			//generates 6 more making 74+3=77 or 74+6=80!!
		}

		ReadKey(lkk, "MEASURE_VAR", fsweepINI);
	}
	/*END: read data from SWEEP.INI file*/
//------------------------------------------------------------------
	if (fsweepINI != NULL)
		fclose(fsweepINI);
	fsweepINI = NULL;
	return k; //returns the number of total measurements do to
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
	int a, b, c, i, j, k, l;
	char lkk1[LONGSTRINGSIZE], lkk2[LONGSTRINGSIZE];
	char lprevious[LONGSTRINGSIZE], lnext[LONGSTRINGSIZE]; //previous and next line in the simulation output file; lkk hold current line
	char lelement[LONGSTRINGSIZE]; //when a transistor is found, the line with its names is stored in here
	int measure_p_line[MAXMEAS];
	int read_p_line;
	statistics stats;
	FILE *fLIS, *fSummary;
	//FILE *fsweepINI;
	int first;
	int index[10];
	//char STR1[LONGSTRINGSIZE];
	char STR2[LONGSTRINGSIZE];
	//char STR3[LONGSTRINGSIZE];

//------------------------------------------------------------------
//------------------------------------------------------------------
	/*create summary: table version*/
	for (i = 0; i <= (MAXMEAS-1); i++) {
		measure_p_line[i] = 255;
		*measure[i].data = '\0';

		stats.avg[i] = 0.0;   /*initialization of statistics variables*/
		stats.sig[i] = 0.0;
		stats.max[i] = -1.7976931348623157e+308; //DBL_MAX from <float.h>
		stats.min[i] = +1.7976931348623157e+308; //DBL_MAX from <float.h>
	}

	fSummary = NULL; //?? When used in the "ASCO" tool, wierd bug exist if this
			 //?? line does not exist because then it closes an unopended
			 //?? file at the end of this function.
			 //?? Error seen in: examples/Eldo/inv

	if ((fLIS=fopen(OutputFile,"rt")) == 0) { /* *.lis filename */
		printf("auxfunc_measurefromlis.c - ProcessOutputFile -- Cannot open output file: %s\n", OutputFile);
		exit(EXIT_FAILURE);
	}
	if (mem&2) {
		if ((fSummary=fopen(Summary,"wt")) == 0) { /* summary.txt file */
			printf("auxfunc_measurefromlis.c - ProcessOutputFile -- Cannot open summary file: %s\n", Summary);
			exit(EXIT_FAILURE);
		}
	}

	j=1;
	while ((*measure[j].var_name) != '\0')
		j++; //number of total measurements
	
	i = 0;
	j--; //j=(number of total measurements-1)
	l = 0;
	first = TRUE; /*assumed to avoid printing empty line*/


	/*BEGIN: process file*/
	fgets2(lnext, LONGSTRINGSIZE, fLIS);
	while (!P_eof(fLIS)) {
		/*0*/
		strcpy(lprevious, lkk);              //previous line
		strcpy(lkk, lnext);                  //actual line
		fgets2(lnext, LONGSTRINGSIZE, fLIS); //line ahead

		/* Find out how many colums with transitors, and their position*/
		if (IsItATransistor(lkk, 0, 0, lnext)) {   /*Does the current line containes transistors?*/
			b=DetectsTransistorColumns(lkk, index); /*b=number of columns with transistors*/
		}
		/*0*/

		/*1*/
		read_p_line = FALSE;
		for (k = 0; k <= j; k++) { /*1- for each one of the input lines, look if it is necessary to measure something*/
			/*Step lkk1*/
			strsub(lkk1, lkk, (int)measure[k].s_column1, (int)(strlen(measure[k].search) + 2));
			StripSpaces(lkk1); /*necessary when we want to find '******    alter processing listing'*/

			/*Step lkk2*/
			c = 0;
			if (IsItATransistor(lkk, 0, 0, lnext)) {                       /*if it is a transistor, I know the format*/
				for (a = 1; a <= b; a++) {
					if (strpos2(lkk, measure[k].search, 1) == index[a - 1])
						c = a;
				}
			}
			if (c != 0) {                                                  /*if !=0, then we have found a transistor definition */
				strcpy(lelement, lkk); /*saves, because 'lkk' will be destroyed afterwards*/
				strsub(lkk2, lkk, (int)index[c - 1], (int)(index[c] - index[c - 1]));
				StripSpaces(lkk2);
			} else {                                                       //Its not a transistor(check follows)
				if (!IsItATransistor(measure[k].search, 1, k, '\0')) { //proceeds only if 'measure[k].' is not a transistor
					if (measure[k].column1 == 0 && measure[k].column2 == 0) /*we want to read the data in front*/
						strsub(lkk2, lkk, strpos2(lkk, measure[k].search, 1), strlen(measure[k].search));
					else
						strsub(lkk2, lkk, (int)measure[k].s_column1, strlen(measure[k].search));
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
		if (read_p_line) {      /*3- if data to measure has been found, then proced with the measurements*/
			strsub(lkk1, lkk, (int)measure[0].s_column1, (int)(strlen(measure[0].search) + 2));
			StripSpaces(lkk1);
			if (!strcmp(lkk1, measure[0].search) && strlen(lkk1)>10) { /*Was ALTER simulation found?*/
				if (mem&1)
					WriteToMem(j);
				if (mem&2)
					WriteToFile(j, lprevious, first, &stats, &fSummary);
				i++;
				first = FALSE;
				for (l = 1; l <= (MAXMEAS-1); l++) //After using the measured information,
					*measure[l].data = '\0';   //delete all data that has been stored
				StripSpaces(lprevious);
				strcpy(measure[0].data, lprevious);
			}

			for (k = 1; k <= j; k++) {      /*search for the remaining ones*/
				if (measure_p_line[k] == 0) { /*read all data in the current line*/
					if (*measure[k].data == '\0') {
								/* Do nothing, just go on!! */
					} else {            /*'Monte Carlo' or similar */
						if (mem&1)
							WriteToMem(j);
						if (mem&2)
							WriteToFile(j, lprevious, first, &stats, &fSummary); /* A: write to file so data previously read is not overwritten*/
						i++;
						first = FALSE;
						for (l = 1; l <= (MAXMEAS-1); l++) //After using the measured information,
							*measure[l].data = '\0';   //delete all data that has been stored
					}
													     /*B: now, go on with normal procedure*/
					if (IsItATransistor(measure[k].search, 1, k, '\0')) { /*1: Is the current variable a transistor?*/
						for (a = 1; a <= b; a++) {                    /*2: yes, it is a transistor*/
							if (strpos2(lelement, measure[k].search, 1) == index[a - 1]) {
								strsub(measure[k].data, lkk, (int)index[a - 1], (int)(index[a] - index[a - 1]));
							}
						}
					} else {                                              /*3: no, it is not a transistor*/
						if (measure[k].column1 == 0 && measure[k].column2 == 0) {    /*C: Do we want to read the data in front?*/
							strcpy(lkk1, lkk);                                   /*D: yes*/

							//StripSpaces(lkk1);  this line only creates problems.
							a = strpos2(lkk1, measure[k].search, 1) + strlen(measure[k].search);
							strcpy(lkk1, strsub(STR2, lkk1, (int)a, strlen(lkk)));

							StripSpaces(lkk1);
							a = strpos2(lkk1, " ", 1);
							if (a == 0)
								a = strlen(lkk1);
							sprintf(lkk1, "%.*s", (int)a, strcpy(STR2, lkk1));

							//StripSpaces(lkk1); unnecessary
							strcpy(measure[k].data, lkk1);
						} else                                                       /*E: no, general purpose variable extraction*/
							strsub(measure[k].data, lkk, (int)measure[k].column1, (int)(measure[k].column2 - measure[k].column1 + 1));
					}
				}
			}
		}
		/*3*/

		/*4*/
		for (k = 0; k <= j; k++) { /*4- */
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
//------------------------------------------------------------------
//------------------------------------------------------------------
} /*ProcessOutputFile*/




/*
 *
 */
void MeasureFromLIS(char *ConfigFile, char *OutputFile)
{
	/*retrieve measures from *.lis file*/
	int k;

	k=ReadDataFromConfigFile(ConfigFile, OutputFile);
	if (k)/*if measurements do do exist*/
		ProcessOutputFile(OutputFile, 3);
} /*MeasureFromLIS*/
