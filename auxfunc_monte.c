/*
 * Copyright (C) 1999-2012 Joao Ramos
 * Your use of this code is subject to the terms and conditions of the
 * GNU general public license version 2. See "COPYING" or
 * http://www.gnu.org/licenses/gpl.html
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "auxfunc_monte.h"
#include "auxfunc.h"




/***************************************************************/
/*MonteCarlo ***************************************************/
/***************************************************************/

/*
 * Look for 's' and then read what follows to 'data_string' while
 * storing in 'data_value' its numeric representation
 *
 * assumes values to be in microns
 */
void ReadParameter(char *line, char *line_smallcaps, char *s, char *data_string, double *data_value, int *jj)
{
	int j;
	char laux[LONGSTRINGSIZE];

	j=*jj;
	j = strpos2(line_smallcaps, s, 1);
	if  (j) {
		ReadSubKey(laux, line, &j, '=', ' ', 0);
		StripSpaces(laux);

		if (laux[(int)strlen(laux)-1]=='\'') { /*removes '' if they exist*/
			strsub(data_string, laux, 2, (int)strlen(laux)-2);
			strcpy(laux, data_string);
		}

		*data_value = asc2real(laux, 1, (int)strlen(laux));

		*data_value=*data_value*1e6;
		strcpy(data_string, laux);
	}
	*jj=j;
} /*ReadParameter*/




/*
 * Adds MonteCarlo parametes to CMOS transistors
 */
void MCmosfet(MC_CMOSdata mosfet, char *lkk, char *laux, int *ptr, FILE *fout)
{
	int j, k;
	double w_value, l_value, m_value, Abeta, Avt;
	char w_string[LONGSTRINGSIZE], l_string[LONGSTRINGSIZE], m_string[LONGSTRINGSIZE], DeltaBeta[LONGSTRINGSIZE], DeltaVt[LONGSTRINGSIZE];
	char element[5];
	int small;

	(*ptr)++; /*this variable should not be named 'ptr', but instead something like 'index' or 'element_number'*/
	small = FALSE;

	ReadParameter(lkk, laux, "l=", l_string, &l_value, &j);
	if ((l_value <= mosfet.small_length) && (fcmp(l_value, 0))) {
		small = TRUE;
		printf("small_1_=%s\n", laux);
	}

	j = strpos2(laux, "nmos", 1); /*searches for 'nmos' in the input line*/
	strsub(element, laux, (int)j, 4);
	if (!strcmp(element, "nmos")) {
		if (!small) {
			Avt = mosfet.nmos_avt;
			Abeta = mosfet.nmos_abeta;
		} else {
			Avt = mosfet.s_nmos_avt;
			Abeta = mosfet.s_nmos_abeta;
		}
	} else {
		j = strpos2(laux, "pmos", 1); /*searches for 'pmos' in the input line*/
		strsub(element, laux, (int)j, 4);
		if (!strcmp(element, "pmos")) {
			if (!small) {
				Avt = mosfet.pmos_avt;
				Abeta = mosfet.pmos_abeta;
			} else {
				Avt = mosfet.s_pmos_avt;
				Abeta = mosfet.s_pmos_abeta;
			}
		} else {
			printf("auxfunc_corner.c - MCmosfet -- element not nmos or pmos %s\n", laux);
			exit(EXIT_FAILURE);
		}
	}


	ReadParameter(lkk, laux, "m=", m_string, &m_value, &j);
	if (j==0) {               /*m was not found*/
		m_value = 1.0;
		strcpy(m_string, "1");
	}
	else {
		if (fcmp(m_value, 0)) { /*m was found and is not text*/
			m_value = m_value/1e6;
			sprintf(m_string, "%0.2f", m_value);
		}
	}

	ReadParameter(lkk, laux, "w=", w_string, &w_value, &j);
	ReadParameter(lkk, laux, "l=", l_string, &l_value, &j);
	j--;


	if (!fcmp(w_value, 0) || !fcmp(l_value, 0) ||!fcmp(m_value, 0)) {
		sprintf(DeltaBeta, "%0.4f/sqrt(%s*%s*%s)*%s/100", Abeta, w_string, l_string, m_string, w_string);
		sprintf(DeltaVt,"%0.4f/sqrt(%s*%s*%s)", Avt, w_string, l_string,  m_string);
		/*writes the Monte Carlo parameters*/

		switch(spice) {
			case 1: /*Eldo*/
				fprintf(fout, ".param w%d=AGAUSS(%s, %s*1e-6, 1) ", *ptr, w_string, DeltaBeta);
				fprintf(fout, " vto%d=AGAUSS(0, %s*1e-9, 1)\n", *ptr, DeltaVt);
				break;
			case 2: /*HSPICE*/
				fprintf(fout, ".param w%d=AGAUSS('%s', '%s*1e-6', 1) ", *ptr, w_string, DeltaBeta);
				fprintf(fout, " vto%d=AGAUSS(0, '%s*1e-9', 1)\n", *ptr, DeltaVt);
				break;
			case 3: /*LTspice*/
				printf("auxfunc_monte.c - MCmosfet -- Monte Carlo not implemente for LTspice\n");
				exit(EXIT_FAILURE);
				break;
			case 4: /*Spectre*/
				printf("auxfunc_monte.c - MCmosfet -- Monte Carlo not implemente for Spectre\n");
				exit(EXIT_FAILURE);
				break;
			case 50: /*Qucs*/
				printf("auxfunc_monte.c - MCmosfet -- Monte Carlo not implemente for Qucs\n");
				exit(EXIT_FAILURE);
				break;
			case 51: /*ngspice*/
				fprintf(fout, ".param w%d='AGAUSS(%s, %s*1e-6, 1)' ", *ptr, w_string, DeltaBeta);
				fprintf(fout, "vto%d='AGAUSS(0, %s*1e-9, 1)'\n", *ptr, DeltaVt);
				break;
			case 100: /*general*/
				printf("auxfunc_monte.c - MCmosfet -- Monte Carlo not implemente for GENERAL\n");
				exit(EXIT_FAILURE);
				break;
			default:
				printf("auxfunc_monte.c - MCmosfet -- Something unexpected has happened!\n");
				exit(EXIT_FAILURE);
		}
	} else {
		sprintf(DeltaBeta, "%0.4f", Abeta / sqrt(w_value * l_value * m_value) * w_value / 100);
		sprintf(DeltaVt, "%0.4f", Avt / sqrt(w_value * l_value * m_value));
		/*writes the Monte Carlo parameters*/
		fprintf(fout, ".param w%d=AGAUSS(%s, %su, 1) ", *ptr, w_string, DeltaBeta);
		fprintf(fout, " vto%d=AGAUSS(0, %sm, 1)\n", *ptr, DeltaVt);
	}

	/*writes the line with Monte Carlo parameters*/
	ReadParameter(lkk, laux, "w=", w_string, &w_value, &k);
	if (k<j) { /*'w=' appears before 'l='*/
		k = strpos2(laux, "w=", 1);   /*searches for 'w=' in the input line*/
		fprintf(fout, "%.*s=w%d", k, lkk, *ptr);
		k = strpos2(laux, " l=", 1);  /*searches for ' l=' in the input line*/
		fprintf(fout, "%s delvto=vto%d%s\n", strsub(l_string, lkk, k, (j - k + 1)), *ptr, lkk + j);
	} else { /*'w=' appears after 'l='*/
		k = strpos2(laux, "w=", 1);   /*searches for 'w=' in the input line*/
		fprintf(fout, "%.*s=w%d", k, lkk, *ptr);
		k = strpos2(laux, " ", k);
		fprintf(fout, " delvto=vto%d%s\n", *ptr, lkk + k - 1);
	}
} /*MCmosfet*/




/*
 * Adds MonteCarlo parametes to R, L and C elements
 */
void MCrlc(char device, double delta, char *lkk, int *ptr, FILE *fout)
{
	int j, k;
	/*, delta*/
	double value, delta_abs;
	char lkk1[LONGSTRINGSIZE];

	if (!fcmp(delta, 0)) {
		fprintf(fout, "%s\n", lkk);
		return;
	}

	(*ptr)++;

	StripSpaces(lkk);
	j = (int)strlen(lkk);
	while (lkk[j - 1] != ' ')
		j--;

	k = 1;
	/*the following lines will read the Res/Ind/Cap value*/
	ReadSubKey(lkk1, lkk, &k, '\'', '\'', 0);
	if (*lkk1 == '\0') {   /*resistor/capacitor doesn't include ''. Ex:'10k'*/
		/*This skips the characters after inline comment*/
		k=inlinestrpos(lkk);
		if (k) {
			k--;
			while (lkk[k - 1] == ' ')
				k--;
			k--;
		} else
			k=(int)strlen(lkk);

		j = k;
		while (lkk[j - 1] != ' ')
			j--;

		strsub(lkk1, lkk, j+1, k-j+1);
	}

	delta_abs = delta / 100;

	value = asc2real(lkk1, 1, (int)strlen(lkk1)); /*not used anymore but can be used to print*/
						 /*'delta' instead of 'ComponenteValue*Variation'*/
	if (!fcmp(value, 0)) {
		fprintf(fout, ".param %c%d=AGAUSS(%s, %s*%f, 1)\n", device, *ptr, lkk1, lkk1, delta_abs);

	} else {
		delta_abs = delta_abs * value;
  		/*k = extended2engineer(&delta_abs);*/
		fprintf(fout, ".param %c%d=AGAUSS(%s, %0.2fe%d, 1)\n",      device, *ptr, lkk1, delta_abs, extended2engineer(&delta_abs));
	}
	strsub(lkk1, lkk, k+2, (int)strlen(lkk)); /*the remaing part of the input line*/
	fprintf(fout, "%.*s%c%d%s\n", j, lkk, device, *ptr, lkk1);
} /*MCrlc*/




/*
 * MonteCarlo - fills *.mc file with MonteCarlo parameters: <inputfile>.* -> <inputfile>.mc
 */
void MonteCarlo(char *ConfigFile, char *InputFile, char *OutputFile)
{

	int ptr, i;
	FILE *fin, *fout, *fsweepINI;

	MC_CMOSdata mosfet;
	double r_delta, l_delta, c_delta;
	char STR2[LONGSTRINGSIZE], laux[LONGSTRINGSIZE];


	/**/
	/**/
	if ((fsweepINI=fopen(ConfigFile,"rt")) == 0) {
		printf("auxfunc_corner.c - Cannot open config file: %s\n", ConfigFile);
		exit(EXIT_FAILURE);
	}
	if ((fin=fopen(InputFile,"rt")) == 0) {
		printf("auxfunc_corner.c - Cannot open input file: %s\n", InputFile);
		exit(EXIT_FAILURE);
	}
	if ((fout=fopen(OutputFile,"wt")) == 0) {
		printf("auxfunc_corner.c - Cannot write to output file: %s\n", OutputFile);
		exit(EXIT_FAILURE);
	}


	/**/
	/**/
	ReadKey(lkk, "#Monte Carlo#", fsweepINI);
	if (!lkk[0]) {
		printf("auxfunc_monte.c - #Monte Carlo# key not found\n");
		exit(EXIT_FAILURE);
	} else {
	/*read matching parameters for CMOS TRANSISTORS*/
		i = 1;
		fgets2(lkk, LONGSTRINGSIZE, fsweepINI);
		i = (sscanf(ReadSubKey(STR2, lkk, &i, ':', 'm', 5), "%lg", &mosfet.nmos_avt) == 0);
		i = 1;
		fgets2(lkk, LONGSTRINGSIZE, fsweepINI);
		i = (sscanf(ReadSubKey(STR2, lkk, &i, ':', '%', 5), "%lg", &mosfet.nmos_abeta) == 0);
		i = 1;
		fgets2(lkk, LONGSTRINGSIZE, fsweepINI);
		i = (sscanf(ReadSubKey(STR2, lkk, &i, ':', 'm', 5), "%lg", &mosfet.pmos_avt) == 0);
		i = 1;
		fgets2(lkk, LONGSTRINGSIZE, fsweepINI);
		i = (sscanf(ReadSubKey(STR2, lkk, &i, ':', '%', 5), "%lg", &mosfet.pmos_abeta) == 0);

		i = 1;
		fgets2(lkk, LONGSTRINGSIZE, fsweepINI);
		i = (sscanf(ReadSubKey(STR2, lkk, &i, ':', 'u', 5), "%lg", &mosfet.small_length) == 0);

		i = 1;
		fgets2(lkk, LONGSTRINGSIZE, fsweepINI);
		i = (sscanf(ReadSubKey(STR2, lkk, &i, ':', 'm', 5), "%lg", &mosfet.s_nmos_avt) == 0);
		i = 1;
		fgets2(lkk, LONGSTRINGSIZE, fsweepINI);
		i = (sscanf(ReadSubKey(STR2, lkk, &i, ':', '%', 5), "%lg", &mosfet.s_nmos_abeta) == 0);
		i = 1;
		fgets2(lkk, LONGSTRINGSIZE, fsweepINI);
		i = (sscanf(ReadSubKey(STR2, lkk, &i, ':', 'm', 5), "%lg", &mosfet.s_pmos_avt) == 0);
		i = 1;
		fgets2(lkk, LONGSTRINGSIZE, fsweepINI);
		i = (sscanf(ReadSubKey(STR2, lkk, &i, ':', '%', 5), "%lg", &mosfet.s_pmos_abeta) == 0);

		mosfet.nmos_avt /= sqrt(2.0);
		mosfet.nmos_abeta /= sqrt(2.0);
		mosfet.pmos_avt /= sqrt(2.0);
		mosfet.pmos_abeta /= sqrt(2.0);

		mosfet.s_nmos_avt /= sqrt(2.0);
		mosfet.s_nmos_abeta /= sqrt(2.0);
		mosfet.s_pmos_avt /= sqrt(2.0);
		mosfet.s_pmos_abeta /= sqrt(2.0);

		/*read matching parameters for RESISTORS*/
		i = 1;
		ReadKey(lkk, "R_DELTA", fsweepINI);
		i = (sscanf(ReadSubKey(STR2, lkk, &i, ':', '%', 5), "%lg", &r_delta) == 0);
		r_delta /= sqrt(2.0);
		/*read matching parameters for INDUCTORS*/
		i = 1;
		ReadKey(lkk, "L_DELTA", fsweepINI);
		i = (sscanf(ReadSubKey(STR2, lkk, &i, ':', '%', 5), "%lg", &l_delta) == 0);
		l_delta /= sqrt(2.0);
		/*read matching parameters for CAPACITORS*/
		i = 1;
		ReadKey(lkk, "C_DELTA", fsweepINI);
		i = (sscanf(ReadSubKey(STR2, lkk, &i, ':', '%', 5), "%lg", &c_delta) == 0);
		c_delta /= sqrt(2.0);


		/**/
		/**/
		fgets2(lkk, LONGSTRINGSIZE, fin); /*read title*/
		fprintf(fout, "%s\n", lkk); /*write title*/
		ptr = 0;
		while (!P_eof(fin)) {
			fgets2(lkk, LONGSTRINGSIZE, fin);
			strcpy(laux, lkk); /*lkk maintains character case*/
			Str2Lower(laux);   /*laux constains only lower caps*/
			switch(spice) { /*ngspice special case*/
				case 1: /*Eldo*/
					break;
				case 2: /*HSPICE*/
					break;
				case 3: /*LTspice*/
					printf("auxfunc_monte.c - MonteCarlo -- Monte Carlo not implemente for LTspice\n");
					exit(EXIT_FAILURE);
					break;
				case 4: /*Spectre*/
					printf("auxfunc_monte.c - MonteCarlo -- Monte Carlo not implemente for Spectre\n");
					exit(EXIT_FAILURE);
					break;
				case 50: /*Qucs*/
					printf("auxfunc_monte.c - MonteCarlo -- Monte Carlo not implemente for Qucs\n");
					exit(EXIT_FAILURE);
					break;
				case 51: /*ngspice*/
					if (i==0) {
						i=strpos2(laux, ".control", 1); /*must ignore what is inside a .control section as 'run' will be taken for a resistor below*/
					}
					if (i==1) {
						i=strpos2(laux, ".endc", 1); /*detects .endc*/
						if (i==0)
							i=1; /*.endc was not yet found*/
						else
							i=0; /*.endc was found        */
					}
					if (i) {
						if ((laux[0]!='.') && (laux[0]!=' ')) {
							sprintf(STR2, " %s", laux);
							strcpy(laux, STR2);
						}
					}
					break;
				case 100: /*general*/
					printf("auxfunc_monte.c - MonteCarlo -- Monte Carlo not implemente for GENERAL\n");
					exit(EXIT_FAILURE);
					break;
				default:
					printf("auxfunc_monte.c - MonteCarlo -- Something unexpected has happened!\n");
					exit(EXIT_FAILURE);
			}
			switch (laux[0]) {
				case 'm':   /*MonteCarlo for MOSFET transistors*/
					MCmosfet(mosfet, lkk, laux, &ptr, fout);
					break;

				case 'r':
					MCrlc('r', r_delta, lkk, &ptr, fout);
					break;

				case 'l':
					MCrlc('l', l_delta, lkk, &ptr, fout);
					break;

				case 'c':
					MCrlc('c', c_delta, lkk, &ptr, fout);
					break;

				default:
					fprintf(fout, "%s\n", lkk);
					break;
			}
		}
	}


	if (fsweepINI != NULL)
		fclose(fsweepINI);
	if (fin != NULL)
		fclose(fin);
	if (fout != NULL)
		fclose(fout);
} /*MonteCarlo*/
