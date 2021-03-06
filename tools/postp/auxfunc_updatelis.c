/*
 * Copyright (C) 1999-2016 Joao Ramos
 * Your use of this code is subject to the terms and conditions of the
 * GNU general public license version 2. See "COPYING" or
 * http://www.gnu.org/licenses/gpl.html
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "auxfunc_updatelis.h"
#include "auxfunc.h"




/***************************************************************/
/*UpdateLIS ****************************************************/
/***************************************************************/

/*
 *
 */
void SpectreStripSI (char *Result, char begin_char, char end_char)
{
	int i;
	char laux[LONGSTRINGSIZE];

	i=1; ReadSubKey(laux, Result, &i, begin_char, end_char, 5);

	i=strpos2(laux, " ", 2);
	if ((laux[i]==109)) { /* 109=m; mV */
		laux[i-1]='\0';
		strcat(laux, "e-3");
	}
	if ((laux[i]==117)) { /* 117=u; uV */
		laux[i-1]='\0';
		strcat(laux, "e-6");
	}
	if ((laux[i]==110)) { /* 110=n; nV */
		laux[i-1]='\0';
		strcat(laux, "e-9");
	}
	if ((laux[i]==112)) { /* 110=p; pV */
		laux[i-1]='\0';
		strcat(laux, "e-12");
	}
	sprintf(Result, "          %s", laux); /* Puts the value on the expected column for DoCalculations */
}




/*
 *
 */
void ReadKeyPrintLine(char *Result, char *key, FILE *streamIn, FILE *streamOut)
{
	char lkk1[LONGSTRINGSIZE];
	char STR1[LONGSTRINGSIZE];

	while ((strpos2(lkk1, key, 1) == 0) & (!P_eof(streamIn))) {
		fgets2(lkk1, LONGSTRINGSIZE, streamIn);
		fprintf(streamOut, "%s\n", lkk1);
				int i;
				i=0;
				i=strpos2(lkk1, key, 1);
				i++;
	}

	if (!P_eof(streamIn))
		strcpy(Result, lkk1);
	else
		Result[0]='\0';
} /*ReadKeyPrintLine*/




/*
 *
 */
void DoCalculations(char *lelement, char *lVGS, char *lVDS, char *lVth, char *lVDSAT, double Vovd, double Voff, double Vdst, char (*stats)[LONGSTRINGSIZE], FILE **fNoSat)
{
	int a, i, j, k;
	double vgs, vds, vth, vdsat;
	char laux[LONGSTRINGSIZE], llaux[LONGSTRINGSIZE], lOpRegion[LONGSTRINGSIZE], lVovd[LONGSTRINGSIZE], lVds_Vdsat[LONGSTRINGSIZE]; /*char lines with 'operating region', 'overdrive voltage' and 'vds-vdsat voltage'*/
	int PrintNoSat;
	int index[10]; /*up to 10 columns with transistors can exist in the output file; considered enough*/
	char STR1[LONGSTRINGSIZE];
	char STR2[LONGSTRINGSIZE];

	j=DetectsTransistorColumns(lelement, index); /*j=number of columns with transistors*/

	strcpy(lOpRegion, "OP region:   ");
	strcpy(lVovd, "Vgs-Vth:     ");
	strcpy(lVds_Vdsat, "Vds-Vdsat:   ");
	for (i = 1; i <= j; i++) {
		/* extra verifications */
		strsub(laux, lelement, (int)index[i - 1], (int)(index[i] - index[i - 1]));
		StripSpaces(laux);

		vgs = asc2real(lVGS, index[i - 1], index[i] - 1);
		vth = asc2real(lVth, index[i - 1], index[i] - 1);
		if (vth>0) { /* if it is an NMOS transistor, vgs>0*/
			if (vgs<0) {
				strcat(lOpRegion, "WRONG NMOS BIAS? :");     /*Vgs should be > 0*/
				fprintf(*fNoSat, "?%s ", laux); /*??/LIN  or  OFF; vds<vdsat*/
			}
		} else {     /* if it is an PMOS transistor, vgs<0*/
			if (vgs>0) {
				strcat(lOpRegion, "WRONG PMOS BIAS? :");     /*Vgs should be < 0*/
				fprintf(*fNoSat, "?%s ", laux); /*??/LIN  or  OFF; vds<vdsat*/
			}
		}
		/* extra verifications */
		vgs = fabs(asc2real(lVGS, index[i - 1], index[i] - 1));
		vds = fabs(asc2real(lVDS, index[i - 1], index[i] - 1));
		vth = fabs(asc2real(lVth, index[i - 1], index[i] - 1));
		vdsat = fabs(asc2real(lVDSAT, index[i - 1], index[i] - 1));

		PrintNoSat = FALSE;   /*assume that all transistors are in SI/SAT*/

		if (vgs > vth - Voff) {   /*if the transistor isn't OFF*/
			if (vgs <= vth + Vovd) {   /*strong inversion*/
				strcat(lOpRegion, "WI");     /*weak inversion*/
				PrintNoSat = TRUE;
			} else
				strcat(lOpRegion, "SI");     /*strong inversion*/

			if (vds > vdsat + Vdst)
				strcat(lOpRegion, "/SAT");   /*saturation*/
			else {
				strcat(lOpRegion, "/LIN");   /*linear*/
				PrintNoSat = TRUE;
			}
		} else {
			strcat(lOpRegion, "   OFF");         /*OFF*/
			PrintNoSat = TRUE;
		}

		/*at this point we will see if we really want to print this transistor*/
		strsub(laux, lelement, (int)index[i - 1], (int)(index[i] - index[i - 1]));
		StripSpaces(laux);
		k = 1;
		while (*skip[k - 1] != '\0' && PrintNoSat == TRUE) {
			if (!strcmp(skip[k - 1], laux))      /*Is this transistor to be printed in the NOSAT.TXT?*/
				PrintNoSat = FALSE;
			a = (int)strlen(skip[k - 1]);
			strcpy(llaux, skip[k - 1]);
			if (llaux[a - 1] == '*') {           /*do not print those starting by ... if they have an '*' at the end*/
				sprintf(STR1, "%.*s", (int)(a - 1), skip[k - 1]);
				sprintf(STR2, "%.*s", (int)(a - 1), laux);
				if (!strcmp(STR1, STR2))
					PrintNoSat = FALSE;
			}
			k++;
		}

		/*those transistor not in SI/SAT will be printed to 'NOSAT.TXT' file*/
		if (PrintNoSat == TRUE) {
			strsub(laux, lelement, (int)index[i - 1], (int)(index[i] - index[i - 1]));
			StripSpaces(laux);
			if (vds > vdsat + Vdst)   
				fprintf(*fNoSat, "%s ", laux);          /*WI/SAT  or  OFF*/
			else {
				if (vds > vdsat)
					fprintf(*fNoSat, "#%s ", laux); /*??/LIN  or  OFF; vdsat<vds<vdsat+Vdst*/
				else
					fprintf(*fNoSat, "*%s ", laux); /*??/LIN  or  OFF; vds<vdsat*/
			}
		}

		sprintf(lOpRegion + (int)strlen(lOpRegion), "%.*s", (int)(index[1] - index[0] - 6), empty);

		if (vgs > vth) {
			sprintf(laux, "%6.3f", vgs - vth);
			sprintf(lVovd + (int)strlen(lVovd), "%s%.*s", laux, (int)(index[1] - index[0] - 6), empty);
			if (vds > vdsat) {
				sprintf(laux, "%6.3f", vds - vdsat);
				sprintf(lVds_Vdsat + (int)strlen(lVds_Vdsat), "%s%.*s", laux, (int)(index[1] - index[0] - 6), empty);
			} else
				sprintf(lVds_Vdsat + (int)strlen(lVds_Vdsat), " -----%.*s", (int)(index[1] - index[0] - 6), empty);   /*LIN*/
		} else {
			sprintf(lVovd + (int)strlen(lVovd), " -----%.*s", (int)(index[1] - index[0] - 6), empty);                     /*OFF*/
			sprintf(lVds_Vdsat + (int)strlen(lVds_Vdsat), " -----%.*s", (int)(index[1] - index[0] - 6), empty);           /*OFF*/
		}

	}

	strcpy(stats[0], lOpRegion);
	strcpy(stats[1], lVovd);
	strcpy(stats[2], lVds_Vdsat);
} /*DoCalculations*/




/*
 * update the *.lis file with the state of the transistor
 */
void UpdateLIS(char *ConfigFile, char *InputFile)
{
	int i, j, k;
	char ltitle[LONGSTRINGSIZE];          /*title: found .alter @*/
	char lelement[LONGSTRINGSIZE];
	/*lmodel,*/                /*transistor/model name*/
	char lVGS[LONGSTRINGSIZE], lVDS[LONGSTRINGSIZE], lVth[LONGSTRINGSIZE], lVDSAT[LONGSTRINGSIZE], fileLJR[LONGSTRINGSIZE];
	FILE *fLJR;                /**.ljr*/
	FILE *fLIS, *fNoSat;
	FILE *fsweepINI;
	/*skip: array [1..100] of string [20];*/ /*100 transistor to skip*/
	char laux[LONGSTRINGSIZE];
	double Vovd, Voff, Vdst;   /*overdrive, off and 'Vds-Vdsat' voltage read from sweep.ini file*/
	ThreeLines stats;


	fNoSat = NULL;
	fLIS = NULL;
	fLJR = NULL;

	if ((fLIS=fopen(InputFile,"rt")) == 0) {
		printf("auxfunc_updatelis.c - Cannot open input file: %s\n", InputFile);
		exit(EXIT_FAILURE);
	}

	sprintf(fileLJR, "%.*sjr", (int)(strlen(InputFile) - 2), InputFile);
	if ((fLJR=fopen(fileLJR,"wt")) == 0) {
		printf("auxfunc_updatelis.c - Cannot open input file: %s\n", fileLJR);
		exit(EXIT_FAILURE);
	}
	if ((fNoSat=fopen(NoSat,"wt")) == 0) {
		printf("auxfunc_updatelis.c - Cannot open input file: %s\n", NoSat);
		exit(EXIT_FAILURE);
	}

	for (i = 0; i < SKIPTRAN; i++)   /*INITIALIZE: read transistors to skip*/
		*skip[i] = '\0';

	if ((fsweepINI=fopen(ConfigFile,"rt")) == 0) {
		printf("auxfunc_updatelis.c - Cannot open input file: %s\n", ConfigFile);
		exit(EXIT_FAILURE);
	}


	ReadKey(lkk, "SKIP_NOSAT", fsweepINI);
	j = strpos2(lkk, "$", 1);
	if (j != 0) /*This will skip the characters after '$', the inline comment used by the sweep tools*/
		lkk[j-1]='\0';
	StripSpaces(lkk);
	k = 1;
	if (!lkk[0])
		printf("INFO:  auxfunc_updatelis.c - 'SKIP_NOSAT' not found\n");
	else {
		while (!strcmp((sprintf(laux, "%.10s", lkk), laux), "SKIP_NOSAT")) {   /*read SKIP_NOSAT from file*/
			i = 1;
			lkk[10] = ';';
			while (i < (int)strlen(lkk)) {
				ReadSubKey(laux, lkk, &i, ';', ';', 0);
				StripSpaces(laux);
				strcpy(skip[k - 1], laux);
				k++;
				if (k == SKIPTRAN) {
					printf("auxfunc_updatelis.c - Maximum number of %d transistors to skip reached. Increase SKIPTRAN in auxfunc_updatelis\n", SKIPTRAN);
					exit(EXIT_FAILURE);
				}
			}
			ReadKey(lkk, "SKIP_NOSAT", fsweepINI);
			j = strpos2(lkk, "$", 1);
			if (j != 0) /*This will skip the characters after '$', the inline comment used by the sweep tools*/
				lkk[j-1]='\0';
			StripSpaces(lkk);
		}
	}

	
	Vovd = 0.05;   /*default OVERDRIVE VOLTAGE value is 50mV*/
	fseek(fsweepINI, 0, SEEK_SET);
	ReadKey(lkk, "VOVD", fsweepINI);
	if (strcmp((sprintf(laux, "%.4s", lkk), laux), "VOVD"))
		printf("INFO:  auxfunc_updatelis.c - No Vovd, default value (50mV) will be used instead\n");
	else {
		i = 1;
		i = (sscanf(ReadSubKey(laux, lkk, &i, ':', 'm', 5), "%lg", &Vovd) == 0);
		if (i) { /*if (i=0) then laux contains a number and not text*/
			printf("auxfunc_updatelis.c - Incorrect format: number must exist in '%s'\n", lkk);
			exit(EXIT_FAILURE);
		}
		Vovd /= 1000;
		/*the following line will print information about the symbols in 'fNoSat' file*/
		fprintf(fNoSat, "` `:weak inversion if Vgs<Vth+%smV\n", laux);
	}


	Voff = 0.1;   /*default OFF VOLTAGE value is 100mV*/
	fseek(fsweepINI, 0, SEEK_SET);
	ReadKey(lkk, "VOFF", fsweepINI);
	if (strcmp((sprintf(laux, "%.4s", lkk), laux), "VOFF"))
		printf("INFO:  auxfunc_updatelis.c - No Voff: default value (100mV) will be used instead\n");
	else {
		i = 1;
		i = (sscanf(ReadSubKey(laux, lkk, &i, ':', 'm', 5), "%lg", &Voff) == 0);
		if (i) { /*if (i=0) then laux contains a number and not text*/
			printf("auxfunc_updatelis.c - Incorrect format: number must exist in '%s'\n", lkk);
			exit(EXIT_FAILURE);
		}
		Voff /= 1000;
	}


	Vdst = 0.0;   /*default DST VOLTAGE value is 000mV*/
	fseek(fsweepINI, 0, SEEK_SET);
	ReadKey(lkk, "VDST", fsweepINI);
	if (strcmp((sprintf(laux, "%.4s", lkk), laux), "VDST"))
		printf("INFO:  auxfunc_updatelis.c - No Vdst: default value (0mV) will be used instead\n");
	else {
		i = 1;
		i = (sscanf(ReadSubKey(laux, lkk, &i, ':', 'm', 5), "%lg", &Vdst) == 0); /*read 'Vds-Vdsat' from file*/
		if (i) {/*if (i=0) then laux contains a number and not text*/
			printf("auxfunc_updatelis.c - Incorrect format: number must exist in '%s'\n", lkk);
			exit(EXIT_FAILURE);
		}
		Vdst /= 1000;
		/*the following lines will print information about the symbols in 'fNoSat' file*/
		if (Vdst != 0)
			fprintf(fNoSat, "`#`:Vds<Vdsat+%smV\n", laux);
		fprintf(fNoSat, "`*`:Vds<Vdsat\n\n");
	}


	if (fsweepINI != NULL)
		fclose(fsweepINI);
	fsweepINI = NULL;


	/*  */
	/*  */
	*ltitle = '\0';
	while (!P_eof(fLIS)) {
		switch(spice) {
			case 1: /* Eldo */
				while ((strcmp((sprintf(laux, "%.37s", lkk), laux), "0****                 OPERATING POINT") != 0) & (!P_eof(fLIS))) {
					fgets2(lkk, LONGSTRINGSIZE, fLIS);
					fprintf(fLJR, "%s\n", lkk);
					StripSpaces(lkk);   /*required due to Solaris OS*/
					//if (lkk[0] == '@' && strcmp(lkk, ltitle)) {
					if (strpos2(lkk, " .ALTER @", 1) !=0) {
						fprintf(fNoSat, "%s | ", lkk);
						strcpy(ltitle, lkk);
					}
				}

				sprintf(laux, "%.37s", lkk);
				if (!strcmp(laux, "0****                 OPERATING POINT")) {
					for (i = 1; i <= 9; i++) {
						fgets2(lkk, LONGSTRINGSIZE, fLIS);
						fprintf(fLJR, "%s\n", lkk);
						if ( (strpos2(lkk, "        M", 1) != 0) || (strpos2(lkk, "        X", 1) != 0) ) /*breaks loop only if string is found*/
							break;
					}
					if (i==9) {
						printf("auxfunc_updatelis.c - Eldo netlist (%s) format has changed significantly\n", InputFile);
						exit(EXIT_FAILURE);
					}

					while ( (strpos2(lkk, "        M", 1) != 0) || (strpos2(lkk, "        X", 1) != 0) ) {   /*find operating region for all transistors*/
						strcpy(lelement, lkk);
						fgets2(lkk, LONGSTRINGSIZE, fLIS);
						fprintf(fLJR, "%s\n", lkk);

						ReadKeyPrintLine(lVGS, "VGS  ", fLIS, fLJR);
						if (!strpos2(lVGS, "VGS  ", 1)) {
							printf("auxfunc_updatelis.c - Eldo netlist (%s) format has changed significantly\n", InputFile);
							exit(EXIT_FAILURE);
						}
						ReadKeyPrintLine(lVDS, "VDS  ", fLIS, fLJR);
						if (!strpos2(lVDS, "VDS  ", 1)) {
							printf("auxfunc_updatelis.c - Eldo netlist (%s) format has changed significantly\n", InputFile);
							exit(EXIT_FAILURE);
						}
						ReadKeyPrintLine(lVth, "VTH  ", fLIS, fLJR);
						if (!strpos2(lVth, "VTH  ", 1)) {
							printf("auxfunc_updatelis.c - Eldo netlist (%s) format has changed significantly\n", InputFile);
							exit(EXIT_FAILURE);
						}
						ReadKeyPrintLine(lVDSAT, "VDSAT  ", fLIS, fLJR);
						if (!strpos2(lVDSAT, "VDSAT  ", 1)) {
							printf("auxfunc_updatelis.c - Eldo netlist (%s) format has changed significantly\n", InputFile);
							exit(EXIT_FAILURE);
						}

						while ((int)strlen(lkk)) { /*empty line has been found, goes ahead*/
							//strcpy(laux,lkk);
							fgets2(lkk, LONGSTRINGSIZE, fLIS);
							fprintf(fLJR, "%s\n", lkk);
						}

						DoCalculations(lelement, lVGS, lVDS, lVth, lVDSAT, Vovd, Voff, Vdst, stats, &fNoSat); /*gets three lines with operating region*/
						fprintf(fLJR, "%s\n", stats[0]);
						fprintf(fLJR, "%s\n", stats[1]);
						fprintf(fLJR, "%s\n\n\n", stats[2]);


						for (i = 1; i <= 9; i++) {
							fgets2(lkk, LONGSTRINGSIZE, fLIS);
							if ((i > 1) || (int)strlen(lkk))
								fprintf(fLJR, "%s\n", lkk);
							if ( (strpos2(lkk, "        M", 1) != 0) || (strpos2(lkk, "        X", 1) != 0) ) /*breaks loop only if string is found, BUT unlike the code above in here it might be the last MOSTFET group*/
								break;
						}

					}
					putc('\n', fNoSat);
				}
				break;
			case 2: /* HSPICE */
				i=0;
				while ((strcmp((sprintf(laux, "%.12s", lkk), laux), "**** mosfets") != 0) & (!P_eof(fLIS))) {
					fgets2(lkk, LONGSTRINGSIZE, fLIS);
					fprintf(fLJR, "%s\n", lkk);
					StripSpaces(lkk);   /*required due to Solaris OS*/
					if (lkk[0] == '@' && strcmp(lkk, ltitle)) {
						if (i>0) {
							fprintf(fNoSat, "\n");
						}
						i++;
						fprintf(fNoSat, "%s | ", lkk);
						strcpy(ltitle, lkk);
					}
				}

				sprintf(laux, "%.12s", lkk);
				if (!strcmp(laux, "**** mosfets")) {
					for (i = 1; i <= 9; i++) {
						fgets2(lkk, LONGSTRINGSIZE, fLIS);
						fprintf(fLJR, "%s\n", lkk);
						if (strpos2(lkk, "element  ", 1) != 0) /*breaks loop only if string is found*/
							break;
					}
					if (i==9) {
						printf("auxfunc_updatelis.c - HSPICE netlist (%s) format has changed significantly\n", InputFile);
						exit(EXIT_FAILURE);
					}

					while (strpos2(lkk, "element  ", 1) != 0) {   /*find operating region for all transistors*/
						strcpy(lelement, lkk);
						fgets2(lkk, LONGSTRINGSIZE, fLIS);
						fprintf(fLJR, "%s\n", lkk);

						ReadKeyPrintLine(lVGS, "vgs  ", fLIS, fLJR);
						if (!strpos2(lVGS, "vgs  ", 1)) {
							printf("auxfunc_updatelis.c - HSPICE netlist (%s) format has changed significantly\n", InputFile);
							exit(EXIT_FAILURE);
						}
						ReadKeyPrintLine(lVDS, "vds  ", fLIS, fLJR);
						if (!strpos2(lVDS, "vds  ", 1)) {
							printf("auxfunc_updatelis.c - HSPICE netlist (%s) format has changed significantly\n", InputFile);
							exit(EXIT_FAILURE);
						}
						ReadKeyPrintLine(lVth, "vth  ", fLIS, fLJR);
						if (!strpos2(lVth, "vth  ", 1)) {
							printf("auxfunc_updatelis.c - HSPICE netlist (%s) format has changed significantly\n", InputFile);
							exit(EXIT_FAILURE);
						}
						ReadKeyPrintLine(lVDSAT, "vdsat  ", fLIS, fLJR);
						if (!strpos2(lVDSAT, "vdsat  ", 1)) {
							printf("auxfunc_updatelis.c - HSPICE netlist (%s) format has changed significantly\n", InputFile);
							exit(EXIT_FAILURE);
						}

						while ((int)strlen(lkk)) { /*empty line has been found, goes ahead*/
							//strcpy(laux,lkk);
							fgets2(lkk, LONGSTRINGSIZE, fLIS);
							fprintf(fLJR, "%s\n", lkk);
						}

						DoCalculations(lelement, lVGS, lVDS, lVth, lVDSAT, Vovd, Voff, Vdst, stats, &fNoSat); /*gets three lines with operating region*/
						fprintf(fLJR, "%s\n", stats[0]);
						fprintf(fLJR, "%s\n", stats[1]);
						fprintf(fLJR, "%s\n\n\n", stats[2]);


						for (i = 1; i <= 9; i++) {
							fgets2(lkk, LONGSTRINGSIZE, fLIS);
							if ((i > 2) || (int)strlen(lkk))
								fprintf(fLJR, "%s\n", lkk);
							if (strpos2(lkk, "element  ", 1)) /*breaks loop only if string is found, BUT unlike the code above in here it might be the last MOSTFET group*/
								break;
						}

					}
					putc('\n', fNoSat);
				}
				break;
			case 3: /* LTspice */
				while ((strcmp((sprintf(laux, "%.21s", lkk), laux), "--- BSIM3 MOSFETS ---") != 0) & (!P_eof(fLIS))) {
					fgets2(lkk, LONGSTRINGSIZE, fLIS);
					fprintf(fLJR, "%s\n", lkk);
					StripSpaces(lkk);   /*required due to Solaris OS*/
					if (lkk[0] == '@' && strcmp(lkk, ltitle)) {
						fprintf(fNoSat, "%s | ", lkk);
						strcpy(ltitle, lkk);
					}
				}

				sprintf(laux, "%.21s", lkk);
				if (!strcmp(laux, "--- BSIM3 MOSFETS ---")) {
					for (i = 1; i <= 9; i++) {
						fgets2(lkk, LONGSTRINGSIZE, fLIS);
						fprintf(fLJR, "%s\n", lkk);
						if (strpos2(lkk, "Name:  ", 1) != 0) /*breaks loop only if string is found*/
							break;
					}
					if (i==9) {
						printf("auxfunc_updatelis.c - LTspice netlist (%s) format has changed significantly\n", InputFile);
						exit(EXIT_FAILURE);
					}

					while (strpos2(lkk, "Name:  ", 1) != 0) {   /*find operating region for all transistors*/
						strcpy(lelement, lkk);
						fgets2(lkk, LONGSTRINGSIZE, fLIS);
						fprintf(fLJR, "%s\n", lkk);

						ReadKeyPrintLine(lVGS, "Vgs:  ", fLIS, fLJR);
						if (!strpos2(lVGS, "Vgs:  ", 1)) {
							printf("auxfunc_updatelis.c - LTspice netlist (%s) format has changed significantly\n", InputFile);
							exit(EXIT_FAILURE);
						}
						ReadKeyPrintLine(lVDS, "Vds:  ", fLIS, fLJR);
						if (!strpos2(lVDS, "Vds:  ", 1)) {
							printf("auxfunc_updatelis.c - LTspice netlist (%s) format has changed significantly\n", InputFile);
							exit(EXIT_FAILURE);
						}
						ReadKeyPrintLine(lVth, "Vth:  ", fLIS, fLJR);
						if (!strpos2(lVth, "Vth:  ", 1)) {
							printf("auxfunc_updatelis.c - LTspice netlist (%s) format has changed significantly\n", InputFile);
							exit(EXIT_FAILURE);
						}
						ReadKeyPrintLine(lVDSAT, "Vdsat:  ", fLIS, fLJR);
						if (!strpos2(lVDSAT, "Vdsat:  ", 1)) {
							printf("auxfunc_updatelis.c - LTspice netlist (%s) format has changed significantly\n", InputFile);
							exit(EXIT_FAILURE);
						}

						while (lkk[0]!='\r') { /*empty line has been found, goes ahead*/
							//strcpy(laux,lkk);
							fgets2(lkk, LONGSTRINGSIZE, fLIS);
							fprintf(fLJR, "%s\n", lkk);
						}
						fseek(fLJR, -2, SEEK_CUR); /*properly position the pointer*/

						DoCalculations(lelement, lVGS, lVDS, lVth, lVDSAT, Vovd, Voff, Vdst, stats, &fNoSat); /*gets three lines with operating region*/
						fprintf(fLJR, "%s\n", stats[0]);
						fprintf(fLJR, "%s\n", stats[1]);
						fprintf(fLJR, "%s\n\r\n", stats[2]);


						for (i = 1; i <= 9; i++) {
							fgets2(lkk, LONGSTRINGSIZE, fLIS);
							if ((i > 1) || (int)strlen(lkk))
								fprintf(fLJR, "%s\n", lkk);
							if (strpos2(lkk, "Name:  ", 1)) /*breaks loop only if string is found, BUT unlike the code above in here it might be the last MOSTFET group*/
								break;
						}

					}
					putc('\n', fNoSat);
				}
				break;
			case 4: /* Spectre */
				// in input.scs put "dcOp dc oppoint=logfile"
				// run spectre with "spectre input.scs =log input.lis"
				//
				// Code specific for "Primitive: bsim3v3" and is quite possible
				// not to work on future Spectre versions and other transistor models
				//
				while ((strcmp((sprintf(laux, "%.31s", lkk), laux), "operating-point information `dc") != 0) & (!P_eof(fLIS))) {
					fgets2(lkk, LONGSTRINGSIZE, fLIS);
					fprintf(fLJR, "%s\n", lkk);
					StripSpaces(lkk);   /*required due to Solaris OS*/
					if (lkk[0] == '@' && strcmp(lkk, ltitle)) {
						fprintf(fNoSat, "%s | ", lkk);
						strcpy(ltitle, lkk);
					}
					Str2Lower(lkk); //required because versions until 2007 have "... Information" and after have "... information"
				}

				if (P_eof(fLIS))
					break;
				char l1[LONGSTRINGSIZE], l2[LONGSTRINGSIZE]; /*l1 and l2 are hold places for previous lines*/
				fgets2(l1, LONGSTRINGSIZE, fLIS); fprintf(fLJR, "%s\n", l1);
				fgets2(l2, LONGSTRINGSIZE, fLIS); fprintf(fLJR, "%s\n", l2);
				while ((strcmp((sprintf(laux, "%.15s", lkk), laux), "Primitive: bsim") != 0) & (!P_eof(fLIS))) {
/*bsim3v3*/ //			while ((strcmp((sprintf(laux, "%.18s", lkk), laux), "Primitive: bsim3v3") != 0) & (!P_eof(fLIS))) {
/*bsim4*/ //			while ((strcmp((sprintf(laux, "%.16s", lkk), laux), "Primitive: bsim4") != 0) & (!P_eof(fLIS))) {
					strcpy(l1, l2);
					strcpy(l2, lkk);
					fgets2(lkk, LONGSTRINGSIZE, fLIS);
					fprintf(fLJR, "%s\n", lkk);
					StripSpaces(lkk);
					if (lkk[0] == '@' && strcmp(lkk, ltitle)) {
						fprintf(fNoSat, "%s | ", lkk);
						strcpy(ltitle, lkk);
					}
				}
				
				sprintf(laux, "%.15s", lkk);
				if (!strcmp(laux, "Primitive: bsim")) {
/*bsim3v3*/ //			sprintf(laux, "%.18s", lkk);
/*bsim3v3*/ //			if (!strcmp(laux, "Primitive: bsim3v3")) {
/*bsim4*/ //			sprintf(laux, "%.16s", lkk);
/*bsim4*/ //			if (!strcmp(laux, "Primitive: bsim4")) {
					for (i = 1; i <= 0; i++) {
						fgets2(lkk, LONGSTRINGSIZE, fLIS);
						fprintf(fLJR, "%s\n", lkk);
					}

					while (strpos2(lkk, "Primitive: bsim", 1) != 0) {   /*find operating region for all transistors*/
/*bsim3v3*/ //				while (strpos2(lkk, "Primitive: bsim3v3", 1) != 0) {   /*find operating region for all transistors*/
/*bsim4*/ //				while (strpos2(lkk, "Primitive: bsim4", 1) != 0) {   /*find operating region for all transistors*/
						strcpy(lelement, l1);
						if (strpos2(lelement, " of ", 1)) { /*to remove the library name if and when it appears*/
							lelement[strpos2(lelement, " of ", 1)]='\0';
						}

						ReadKeyPrintLine(lVGS, "vgs = ", fLIS, fLJR);
						if (!strpos2(lVGS, "vgs = ", 1)) {
							printf("auxfunc_updatelis.c - Spectre netlist (%s) format has changed significantly\n", InputFile);
							exit(EXIT_FAILURE);
						}
						SpectreStripSI(lVGS, '=', 'V');
						ReadKeyPrintLine(lVDS, "vds = ", fLIS, fLJR);
						if (!strpos2(lVDS, "vds = ", 1)) {
							printf("auxfunc_updatelis.c - Spectre netlist (%s) format has changed significantly\n", InputFile);
							exit(EXIT_FAILURE);
						}
						SpectreStripSI(lVDS, '=', 'V');
						ReadKeyPrintLine(lVth, "vth = ", fLIS, fLJR);
						if (!strpos2(lVth, "vth = ", 1)) {
							printf("auxfunc_updatelis.c - Spectre netlist (%s) format has changed significantly\n", InputFile);
							exit(EXIT_FAILURE);
						}
						SpectreStripSI(lVth, '=', 'V');
						ReadKeyPrintLine(lVDSAT, "vdsat = ", fLIS, fLJR);
						if (!strpos2(lVDSAT, "vdsat = ", 1)) {
							printf("auxfunc_updatelis.c - Spectre netlist (%s) format has changed significantly\n", InputFile);
							exit(EXIT_FAILURE);
						}
						SpectreStripSI(lVDSAT, '=', 'V');

						while ((int)strlen(lkk)) { /*empty line has been found, goes ahead*/
							//strcpy(laux,lkk);
							fgets2(lkk, LONGSTRINGSIZE, fLIS);
							fprintf(fLJR, "%s\n", lkk);
						}
						fseek(fLJR, -1, SEEK_CUR); /*properly position the pointer*/

						DoCalculations(lelement, lVGS, lVDS, lVth, lVDSAT, Vovd, Voff, Vdst, stats, &fNoSat); /*gets three lines with operating region*/
						fprintf(fLJR, "%s\n", stats[0]);
						fprintf(fLJR, "%s\n", stats[1]);
						fprintf(fLJR, "%s\n\n", stats[2]);


						for (i = 1; i <= 9; i++) {
							strcpy(l1, l2);
							strcpy(l2, lkk);
							fgets2(lkk, LONGSTRINGSIZE, fLIS);
							if (i > 0)
								fprintf(fLJR, "%s\n", lkk);
							if (strpos2(lkk, "Primitive: bsim", 1)) /*breaks loop only if string is found, BUT unlike the code above in here it might be the last MOSTFET group*/
								break;
						}

					}
					putc('\n', fNoSat);
				}
				break;
			case 50: /*Qucs*/
				printf("auxfunc_updatelis.c - UpdateLIS -- Updatelis not implemente for Qucs\n");
				exit(EXIT_FAILURE);
			case 100: /* rosen */
				printf("auxfunc_updatelis.c - UpdateLIS -- Updatelis not implemente for rosen\n");
				exit(EXIT_FAILURE);
				break;
			default:
				printf("auxfunc_updatelis.c - UpdateLIS -- Something unexpected has happened!\n");
				exit(EXIT_FAILURE);
		}

	}

	if (fLIS != NULL)
		fclose(fLIS);
	fLIS = NULL;
	if (fLJR != NULL)
		fclose(fLJR);
	fLJR = NULL;
	if (fNoSat != NULL)
		fclose(fNoSat);
	fNoSat = NULL;

	if (fLJR != NULL)
		fclose(fLJR);
	if (fLIS != NULL)
		fclose(fLIS);
	if (fNoSat != NULL)
		fclose(fNoSat);
} /*UpdateLIS*/
