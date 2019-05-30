/*
 * Copyright (C) 1999-2006 Joao Ramos
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
void DoCalculations(char *lelement, char *lVGS, char *lVDS, char *lVth, char *lVDSAT, double Vovd, double Voff, double Vdst, char (*stats)[LONGSTRINGSIZE], FILE **fNoSat)
{
	int a, i, j, k;
	double vgs, vds, vth, vdsat;
	char laux[LONGSTRINGSIZE], llaux[LONGSTRINGSIZE], lOpRegion[LONGSTRINGSIZE], lVovd[LONGSTRINGSIZE], lVds_Vdsat[LONGSTRINGSIZE]; /*char lines with 'operating region', 'overdrive voltage' and 'vds-vdsat voltage'*/
	int PrintNoSat;
	int index[10]; /*up to 10 columns with transistors can exist in the outpu file; considered enough*/
	char STR1[LONGSTRINGSIZE];
	char STR2[LONGSTRINGSIZE];

	j=DetectsTransistorColumns(lelement, index); /*j=number of columns with transistors*/

	strcpy(lOpRegion, "OP region:   ");
	strcpy(lVovd, "Vgs-Vth:     ");
	strcpy(lVds_Vdsat, "Vds-Vdsat:   ");
	for (i = 1; i <= j; i++) {
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
			a = strlen(skip[k - 1]);
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
			strsub(llaux, lelement, (int)index[i - 1], (int)(index[i] - index[i - 1]));
			StripSpaces(llaux);
			if (vds > vdsat + Vdst)   
				fprintf(*fNoSat, "%s ", llaux);          /*WI/SAT  or  OFF*/
			else {
				if (vds > vdsat)
					fprintf(*fNoSat, "#%s ", llaux); /*??/LIN  or  OFF; vdsat<vds<vdsat+Vdst*/
				else
					fprintf(*fNoSat, "*%s ", llaux); /*??/LIN  or  OFF; vds<vdsat*/
			}
		}

		sprintf(lOpRegion + strlen(lOpRegion), "%.*s", (int)(index[1] - index[0] - 6), empty);

		if (vgs > vth) {
			sprintf(laux, "%6.3f", vgs - vth);
			sprintf(lVovd + strlen(lVovd), "%s%.*s", laux, (int)(index[1] - index[0] - 6), empty);
			if (vds > vdsat) {
				sprintf(laux, "%6.3f", vds - vdsat);
				sprintf(lVds_Vdsat + strlen(lVds_Vdsat), "%s%.*s", laux, (int)(index[1] - index[0] - 6), empty);
			} else
				sprintf(lVds_Vdsat + strlen(lVds_Vdsat), " -----%.*s", (int)(index[1] - index[0] - 6), empty);   /*LIN*/
		} else {
			sprintf(lVovd + strlen(lVovd), " -----%.*s", (int)(index[1] - index[0] - 6), empty);                     /*OFF*/
			sprintf(lVds_Vdsat + strlen(lVds_Vdsat), " -----%.*s", (int)(index[1] - index[0] - 6), empty);           /*OFF*/
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
	int i, k;
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
					if (lkk[0] == '@' && strcmp(lkk, ltitle)) {
						fprintf(fNoSat, "%s | ", lkk);
						strcpy(ltitle, lkk);
					}
				}

				sprintf(laux, "%.37s", lkk);
				if (!strcmp(laux, "0****                 OPERATING POINT")) {
					for (i = 1; i <= 6; i++) {
						fgets2(lkk, LONGSTRINGSIZE, fLIS);
						fprintf(fLJR, "%s\n", lkk);
					}

					while ( (strpos2(lkk, "        M", 1) != 0) || (strpos2(lkk, "        X", 1) != 0)) {   /*find operating region for all transistors*/
						strcpy(lelement, lkk);
						fgets2(lkk, LONGSTRINGSIZE, fLIS);
						fprintf(fLJR, "%s\n", lkk);

						fgets2(lkk, LONGSTRINGSIZE, fLIS);
						fprintf(fLJR, "%s\n", lkk);

						if (strpos2(lkk, "region", 1) !=0) { /* Due to HSPICE 2001.2 line */
							fgets2(lkk, LONGSTRINGSIZE, fLIS);
							fprintf(fLJR, "%s\n", lkk);  /* with the operation region */
						}

						/* fgets2(lkk, LONGSTRINGSIZE, fLIS); */
						/* fprintf(fLJR, "%s\n", lkk); */
						fgets2(lkk, LONGSTRINGSIZE, fLIS);
						fprintf(fLJR, "%s\n", lkk);

						fgets2(lVGS, LONGSTRINGSIZE, fLIS);   /*Vgs*/
						fprintf(fLJR, "%s\n", lVGS);
						fgets2(lVDS, LONGSTRINGSIZE, fLIS);   /*Vds*/
						fprintf(fLJR, "%s\n", lVDS);
						fgets2(lkk, LONGSTRINGSIZE, fLIS);
						fprintf(fLJR, "%s\n", lkk);
						fgets2(lVth, LONGSTRINGSIZE, fLIS);   /*Vth*/
						fprintf(fLJR, "%s\n", lVth);
						fgets2(lVDSAT, LONGSTRINGSIZE, fLIS); /*Vdsat*/
						fprintf(fLJR, "%s\n", lVDSAT);

						for (i = 1; i <= 24; i++) {
							fgets2(lkk, LONGSTRINGSIZE, fLIS);
							fprintf(fLJR, "%s\n", lkk);
						}

						DoCalculations(lelement, lVGS, lVDS, lVth, lVDSAT, Vovd, Voff, Vdst, stats, &fNoSat); /*gets three lines with operating region*/
						fprintf(fLJR, "%s\n", stats[0]);
						fprintf(fLJR, "%s\n", stats[1]);
						fprintf(fLJR, "%s\n\n\n", stats[2]);

						for (i = 1; i <= 2; i++) {
							fgets2(lkk, LONGSTRINGSIZE, fLIS);
							if (i > 1)
							fprintf(fLJR, "%s\n", lkk);
						}

					}
					putc('\n', fNoSat);
				}
				break;
			case 2: /* HSPICE */
				while ((strcmp((sprintf(laux, "%.12s", lkk), laux), "**** mosfets") != 0) & (!P_eof(fLIS))) {
					fgets2(lkk, LONGSTRINGSIZE, fLIS);
					fprintf(fLJR, "%s\n", lkk);
					StripSpaces(lkk);   /*required due to Solaris OS*/
					if (lkk[0] == '@' && strcmp(lkk, ltitle)) {
						fprintf(fNoSat, "%s | ", lkk);
						strcpy(ltitle, lkk);
					}
				}

				sprintf(laux, "%.12s", lkk);
				if (!strcmp(laux, "**** mosfets")) {
					for (i = 1; i <= 4; i++) {
						fgets2(lkk, LONGSTRINGSIZE, fLIS);
						fprintf(fLJR, "%s\n", lkk);
					}

					while (strpos2(lkk, "element  ", 1) != 0) {   /*find operating region for all transistors*/
						strcpy(lelement, lkk);
						fgets2(lkk, LONGSTRINGSIZE, fLIS);
						fprintf(fLJR, "%s\n", lkk);

						fgets2(lkk, LONGSTRINGSIZE, fLIS);
						fprintf(fLJR, "%s\n", lkk);

						if (strpos2(lkk, "region", 1) !=0) { /* Due to HSPICE 2001.2 line */
							fgets2(lkk, LONGSTRINGSIZE, fLIS);
							fprintf(fLJR, "%s\n", lkk);  /* with the operation region */
						}

						fgets2(lkk, LONGSTRINGSIZE, fLIS);
						fprintf(fLJR, "%s\n", lkk);
						fgets2(lkk, LONGSTRINGSIZE, fLIS);
						fprintf(fLJR, "%s\n", lkk);

						fgets2(lVGS, LONGSTRINGSIZE, fLIS);   /*Vgs*/
						fprintf(fLJR, "%s\n", lVGS);
						fgets2(lVDS, LONGSTRINGSIZE, fLIS);   /*Vds*/
						fprintf(fLJR, "%s\n", lVDS);
						fgets2(lkk, LONGSTRINGSIZE, fLIS);
						fprintf(fLJR, "%s\n", lkk);
						fgets2(lVth, LONGSTRINGSIZE, fLIS);   /*Vth*/
						fprintf(fLJR, "%s\n", lVth);
						fgets2(lVDSAT, LONGSTRINGSIZE, fLIS); /*Vdsat*/
						fprintf(fLJR, "%s\n", lVDSAT);

						for (i = 1; i <= 12; i++) {
							fgets2(lkk, LONGSTRINGSIZE, fLIS);
							fprintf(fLJR, "%s\n", lkk);
						}

						DoCalculations(lelement, lVGS, lVDS, lVth, lVDSAT, Vovd, Voff, Vdst, stats, &fNoSat); /*gets three lines with operating region*/
						fprintf(fLJR, "%s\n", stats[0]);
						fprintf(fLJR, "%s\n", stats[1]);
						fprintf(fLJR, "%s\n\n\n", stats[2]);

						for (i = 1; i <= 4; i++) {
							fgets2(lkk, LONGSTRINGSIZE, fLIS);
							if (i > 2)
							fprintf(fLJR, "%s\n", lkk);
						}

					}
					putc('\n', fNoSat);
				}
				break;
			case 3: /* LTspice */
				printf("auxfunc_updatelis.c - UpdateLIS -- Updatelis not implemente for LTSpice\n");
				exit(EXIT_FAILURE);
				break;
			case 4: /* Spectre */
				printf("auxfunc_updatelis.c - UpdateLIS -- Updatelis not implemente for Spectre\n");
				exit(EXIT_FAILURE);
				break;
				case 100: /* rosen */
				printf("auxfunc_updatelis.c - UpdateLIS -- Updatelis not implemente for rosen\n");
				exit(EXIT_FAILURE);
				break;
			default:
				printf("auxfunc_updatelis.c - Something unexpected has happened!\n");
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
