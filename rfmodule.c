/*
 * Copyright (C) 2005-2006 Joao Ramos
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


#include "auxfunc.h"
#include "initialize.h"
#include "rfmodule.h"




/* 
 * Receives a SPICE line and extract the device value. Currently only
 * R/L/C devices are supported and in a very simply input format as
 * the device value is always the last paramenter in the SPICE line.
 * Returns the starting position of the device value.
 */
int ExtractDeviceValuePosition(char *line)
{
/* Copied from auxfunc_monte.c */
	int j, k;
	char laux[LONGSTRINGSIZE];

	/* This skips the characters after the in-line comment */
	k=inlinestrpos(line);
	if (k) {
		k--;
		while (line[k - 1] == ' ')
			k--;
		k--;
	} else
		k=(int)strlen(line);

	j = k;
	while (line[j - 1] != ' ')
		j--;

	strsub(laux, line, j+1, k-j+1);

	return j; /* Returns the starting position of the device value. */
} /*ExtractDeviceValuePosition*/




/* 
 *
 */
void ReplaceSymbolRF(char *ret, double y_value)
{
	int i, k, ii;
	char laux[LONGSTRINGSIZE], inlinecomment[LONGSTRINGSIZE], lxp[LONGSTRINGSIZE];

	k=inlinestrpos(ret);
	inlinecomment[0]='\0';
	if (k) {
		strsub(inlinecomment, ret, k, (int)strlen(ret)); /* copies the in-line comment */
		ret[k-1]='\0';
	}

	ii=1;
	ReadSubKey(laux, ret, &ii, '_', '_', 5);
	while (ii<=(int)(strlen(ret))) {
		i=0;
		/*while (strcmp(parameters[i].symbol, laux)) { */
		/*	if (parameters[i].symbol[0] == '\0') { */ /*if symbol is not found*/
		/*		printf("initialize.c - ReplaceSymbol -- Symbol in <inputfile>.* not found in <inputfile>.cfg: %s\n", laux); */
		/*		exit(EXIT_FAILURE);            */
		/*	}                                      */
		/*	i++;                                   */
		/*}                                            */

		strsub(laux, ret, ii+1, (int)strlen(ret)-ii);     /* copies the last part of the string to laux */
		ret[ii-2-2]='\0'; /* properly finishes the string */

		/*if (optimize==0) { */                         /*optimize=0 : we are initializing*/
		/*	if (parameters[i].optimize==0) */
				sprintf(lxp, "%E", y_value);           /* writes the value */
		/*	else                                    */
		/*		sprintf(lxp, "#%s#",parameters[i].symbol); */          /*writes the #<symbol>#  back again*/
		/*} else { */                                   /*optimize=1 : we are optimizing*/
		/*	sprintf(lxp, "%E", parameters[i].value); */                   /*writes the value*/
		/*}                                              */
		strcat(ret, lxp);
		strcat(ret, laux);
		ii++; /* because 'ReadSubKey' starts at [ii-1] instead of [ii] */
		ReadSubKey(laux, ret, &ii, '#', '#', 0);
	}

	strcat(ret, inlinecomment); /* concatenates the in-line comment */
} /*ReplaceSymbolRF*/




/* 
 * receives a SPICE line
 *      optimize=0 : we are initializing, just replace R/L/C devices
 *      optimize=1 : "   "  optimizing, replace X (subcircuit) devices
 * returns '0' if device is not changed, i.e., does not have parasitics; its a subcircuit (with or without parasitics)
 * returns '1' if device    was changed, i.e., it has parasitics
  */
int RFModule(char *line, int optimize, FILE* fout)
{
	int i, j, k, ll;
	char laux[LONGSTRINGSIZE], laux2[LONGSTRINGSIZE], laux3[LONGSTRINGSIZE];
	FILE *fspice_cfg;
	static rfsubckt_line rf[MAXRFSUBCKT]; /* the subckt, subkey and data lines are stored in here */
	int prev, next; /* pointers to the previous and next line, where interpolation is to be performed. */
	double x_value=0, x0_value, x1_value, y_value, y0_value, y1_value;
	char   x_string[SHORTSTRINGSIZE], x0_string[SHORTSTRINGSIZE], x1_string[SHORTSTRINGSIZE], y_string[SHORTSTRINGSIZE], y0_string[SHORTSTRINGSIZE], y1_string[SHORTSTRINGSIZE];


	if (!ExecuteRF)
		return 0;

	strcpy(laux, line);
	Str2Lower(laux);
	if (!optimize) { /* we are initializing, just replace R/L/C devices */
		switch (laux[0]) {
/* ---------------------------------------------------------- */
/* ---------------------------------------------------------- */
			case 'r':
			case 'l':
			case 'c':
				if (!(k=inlinestrpos(line))) /* if no in-line comments exist */
					return 0;            /* return '0' because it does not has parasitics */
				ReadSubKey(laux2, line, &k, '#', '#', 0);
				if (laux2[0]=='\0')
					return 0;                                   /* does not has parasitics */
				else {
					sprintf(laux2, "%s%s", "rfmodule", ".cfg"); /* has parasitics */
					if ((fspice_cfg =fopen(laux2 ,"rt")) == 0) {
						printf("rfmodule.c - RFModule -- Cannot open config file: %s\n", laux2);
						exit(EXIT_FAILURE);
					}

					/* the parasitic request command is composed of two parts */
					/* being the first the subcircuit and      #xxxxx______#  */
					/* the second, the characterization values #______yyyyy#  */
					/**/
					/* Step1: */
					k=inlinestrpos(line);
					ReadSubKey(laux2, line, &k, '#', '_', 0);
					sprintf(laux3, "#%s#", laux2);
					ReadKey(laux2, laux3, fspice_cfg); /* looks for key defining the subcircuit */
					if (strcmp(laux2, laux3)) {
						printf("INFO:  rfmodule.c - RFModule -- %s key not found\n", laux2);
						exit(EXIT_FAILURE);
					}

					fgets2(laux2, LONGSTRINGSIZE, fspice_cfg);
					strsub(laux3, laux2, 1, 7); /* detects 'Device:' */
					if (strcmp(laux3, "Device:")) {
						printf("rfmodule.c - RFModule -- Incorrect line format, 'Device:' key is missing: %s\n", laux2);
						exit(EXIT_FAILURE);
					} else {
						strsub(laux3, laux2, 8, (int)strlen(laux2)); /* detects device type:'resitor', 'inductor' and 'capacitor' */
						switch (laux3[0]) {
							case 'r': /* resistor */
								if (strcmp(laux3, "resitor")) {
									printf("rfmodule.c - RFModule -- Incorrect line format, 'resistor' key is missing: %s\n", laux2);
									exit(EXIT_FAILURE);
								}
								if (laux[0]!='r') {
									printf("rfmodule.c - RFModule -- Line is not a resistor: %s\n", line);
									exit(EXIT_FAILURE);
								}
								break;
							case 'i': /* inductor */
								if (strcmp(laux3, "inductor")) {
									printf("rfmodule.c - RFModule -- Incorrect line format, 'inductor' key is missing: %s\n", laux2);
									exit(EXIT_FAILURE);
								}
								if (laux[0]!='l') {
									printf("rfmodule.c - RFModule -- Line is not a inductor: %s\n", line);
									exit(EXIT_FAILURE);
								}
								break;
							case 'c': /* capacitor */
								if (strcmp(laux3, "capacitor")) {
									printf("rfmodule.c - RFModule -- Incorrect line format, 'capacitor' key is missing: %s\n", laux2);
									exit(EXIT_FAILURE);
								}
								if (laux[0]!='c') {
									printf("rfmodule.c - RFModule -- Line is not a capacitor: %s\n", line);
									exit(EXIT_FAILURE);
								}
								break;
							default:
								printf("rfmodule.c - RFModule -- Something unexpected has happened!\n");
								exit(EXIT_FAILURE);
						}
					}

					fgets2(laux2, LONGSTRINGSIZE, fspice_cfg);
					strsub(laux3, laux2, 1, 9); /* detects 'Terminal:' */
					if (strcmp(laux3, "Terminal:")) {
						k=inlinestrpos(line);
						ReadSubKey(laux2, line, &k, '#', '_', 0);
						printf("rfmodule.c - RFModule -- Incorrect format: 'Terminal' key missing in rfmodule.cfg. Subcircuit definition '%s'\n", laux2);
						exit(EXIT_FAILURE);
					}


					/**/
					/* Step2: Looks for device characterization values group */
					k=inlinestrpos(line);
					ReadSubKey(laux2, line, &k, '_', '#', 0);
					sprintf(laux3, "#%s#", laux2);
					ReadKey(laux2, laux3, fspice_cfg); /* looks for device characterization values group */
					if (strcmp(laux2, laux3)) {
						printf("INFO:  rfmodule.c - RFModule -- '%s' device characterization key in '%s' not found in rfmodule.cfg\n", laux3, line);
						exit(EXIT_FAILURE);
					}


					/**/
					/* Step3: Good entry has been found. Check if it already exists in memory */
					strsub(laux2, laux3, 2, (int)strlen(laux3)-2);
					strcpy(laux3, laux2);
					k=inlinestrpos(line);
					ReadSubKey(laux2, line, &k, '#', '_', 0);
					i=0;
					while (rf[i].subckt[0]  != '\0')
						if ( strcmp(rf[i].subckt, laux2) || strcmp(rf[i].subkey, laux3) ) {
							i++;
							if (i>MAXRFSUBCKT) {
								printf("rfmodule.c - RFModule -- Maximum limit of %d subcircuits reached. Increase MAXRFSUBCKT in rfmodule.h\n", MAXRFSUBCKT);
								exit(EXIT_FAILURE);
							}
						} else
							break;

					j=0;
					while (rf[j].subckt[0]  != '\0')
							if (strcmp(rf[j].subckt, laux2)) {
								j++;
							} else {
								j=MAXRFSUBCKT; /* encodes that the subcircuit already exists in memory */
								break;
					}


					k=inlinestrpos(line);
					ReadSubKey(rf[i].subckt, line, &k, '#', '_', 0); /* copies the subckt */
					ReadSubKey(rf[i].subkey, line, &k, '_', '#', 0); /* copies the subkey */


					/**/
					/* Step4 */
					if (j<MAXRFSUBCKT) { /* if a new subcircuit was found */
						k=inlinestrpos(line);
						ReadSubKey(rf[i].subckt, line, &k, '#', '_', 0); /* copies the subckt */
						ReadSubKey(rf[i].subkey, line, &k, '_', '#', 0); /* copies the subkey */

					/* Step4.1: write the ".subckt" header line */
						fprintf(fout, ".subckt %s.sub ", rf[i].subckt);
						sprintf(laux3, "#%s#", rf[i].subckt);
						fseek(fspice_cfg, 0, SEEK_SET);
						ReadKey(laux2, laux3, fspice_cfg); /* looks for key defining the subcircuit */
						fgets2(laux2, LONGSTRINGSIZE, fspice_cfg); /* Device: */
						fgets2(laux2, LONGSTRINGSIZE, fspice_cfg); /* Terminal: */
						strsub(laux3, laux2, 10, (int)strlen(laux2)); /* copies the terminals    */
						fprintf(fout, "%s ", laux3);             /* and prints them to file */

						while ((laux2[0] != '#') && (line[0] != '\0') && (!feof(fspice_cfg))) {
							fgets2(laux2, LONGSTRINGSIZE, fspice_cfg);
							if ((laux2[0] != '#') && (laux2[0] != '*') ) {
								j=ExtractDeviceValuePosition(laux2);
								k = (int)strlen(laux2);
								strsub(laux3, laux2, j+1, k-j+1);
								StripSpaces(laux3);
								fprintf(fout, " %s=1", laux3);
							}
						}
						fprintf(fout, "\n");

					/* Step4.2: Write the lines in the subckt definition */
						sprintf(laux3, "#%s#", rf[i].subckt);
						fseek(fspice_cfg, 0, SEEK_SET);
						ReadKey(laux2, laux3, fspice_cfg); /* looks for key defining the subcircuit */
						fgets2(laux2, LONGSTRINGSIZE, fspice_cfg); /* Device: */
						fgets2(laux2, LONGSTRINGSIZE, fspice_cfg); /* Terminal: */
						/*strsub(laux3, laux2, 10, (int)strlen(laux2));*/ /* copies the terminals */
						/*fprintf(fout, "%s ", laux3);            */ /* and prints them to file */

						while ((laux2[0] != '#') && (line[0] != '\0') && (!feof(fspice_cfg))) {
							fgets2(laux2, LONGSTRINGSIZE, fspice_cfg);
							if (laux2[0] != '#') {
								if (laux2[0] != '*')
									if (laux2[0] != '\0')
										fprintf(fout, "%s\n", laux2);
							} else
								fprintf(fout, ".ENDS %s.sub\n", rf[i].subckt);
						}
					}


					/**/
					/* Step5: Write the "X... " line to <hostname>.tmp */
					j=ExtractDeviceValuePosition(line);
					strsub(laux2, line, 1, j);
					sprintf(laux3, "X%s %s.sub", laux2, rf[i].subckt);
					fprintf(fout, "%s", laux3);

					sprintf(laux3, "#%s#", rf[i].subckt);
					fseek(fspice_cfg, 0, SEEK_SET);
					ReadKey(laux2, laux3, fspice_cfg); /* looks for key defining the subcircuit */
					fgets2(laux2, LONGSTRINGSIZE, fspice_cfg); /* Device: */
					fgets2(laux2, LONGSTRINGSIZE, fspice_cfg); /* Terminal: */
					strsub(laux3, laux2, 10, (int)strlen(laux2));   /* copies the terminals */
					/*fprintf(fout, "%s ", laux3);          */ /* and prints them to file */


					ll=0;
					/* how many lines from the rfmodule.cfg have been read so far */
					while ((laux2[0] != '#') && (line[0] != '\0') && (!feof(fspice_cfg))) {
						fgets2(laux2, LONGSTRINGSIZE, fspice_cfg);
						if ((laux2[0] != '#') && (laux2[0] != '*') ) {
							j=ExtractDeviceValuePosition(laux2);
							k = (int)strlen(laux2);
							strsub(laux3, laux2, j+1, k-j+1);
							StripSpaces(laux3);

							switch(spice) {
								case 1: /*Eldo*/
									break;
								case 2: /*HSPICE*/
									break;
								case 3: /*LTspice*/
									k = strpos2(laux2, "'", 1);
									if(k) {
										k=1;
										ReadSubKey(laux2, laux3, &k, '\'', '\'', 0); /* copies the subckt */
										strcpy(laux3, laux2);
									}
									break;
								case 4: /*Spectre*/
									break;
								case 50: /*Qucs*/
									break;
								case 100: /*general*/
									break;
								default:
									printf("rfmodule.c - Step4 -- Something unexpected has happened!\n");
									exit(EXIT_FAILURE);
							}

							if (ll==0) { /* stores the value from SPICE input file: <inputfile>.* */
								j=ExtractDeviceValuePosition(line);
								k=inlinestrpos(line);
								strsub(laux2, line, j, k-j);
								StripSpaces(laux2);
								fprintf(fout, " %s=%s", laux3, laux2);
								laux2[0]='\0';
								ll++;
							} else
								if (laux3[0] != '\0')
									fprintf(fout, " %s=_RF_", laux3);
						}
					}

					k=inlinestrpos(line);
					strsub(laux3, line, k-1, (int)strlen(line));
					fprintf(fout, "%s", laux3);

					fprintf(fout, "\n");


					/**/
					/* Step6: Read the tabled data from rfmodule.cfg to memory */
					sprintf(laux3, "#%s#", rf[i].subckt);
					fseek(fspice_cfg, 0, SEEK_SET);
					ReadKey(laux2, laux3, fspice_cfg); /* looks for key defining the subcircuit */
					sprintf(laux3, "#%s#", rf[i].subkey);
					ReadKey(laux2, laux3, fspice_cfg); /* looks for key defining the subkey */

					j=0;
					fgets2(laux2, LONGSTRINGSIZE, fspice_cfg);
					while ((laux2[0] != '#') && (line[0] != '\0') && (!feof(fspice_cfg))) {
						strcpy(rf[i].line[j], laux2);
						j++;
						fgets2(laux2, LONGSTRINGSIZE, fspice_cfg);
						if (j > MAXTABLELINES) {
							printf("rfmodule.c - RFModule -- Maximum number of %d lines in rfmodule.cfg exceeded. Increase MAXTABLELINES in rfmodule.h\n", MAXTABLELINES);
							exit(EXIT_FAILURE);
						}
					}


					/**/
					/* Step7: */
					fclose(fspice_cfg);
					return 1;
				}
				break;
/* ---------------------------------------------------------- */
/* ---------------------------------------------------------- */
			case 'x':
				if (!(k=inlinestrpos(line))) /* if no in-line comments exist */
					return 0;            /* return '0' because it does not has parasitics */
				ReadSubKey(laux2, line, &k, '#', '#', 0);
				if (laux2[0]=='\0')
					return 0;                                   /* does not has parasitics */
				else {
					sprintf(laux2, "%s%s", "rfmodule", ".cfg"); /* has parasitics          */
					if ((fspice_cfg =fopen(laux2 ,"rt")) == 0) {
						printf("rfmodule.c - RFModule -- Cannot open config file: %s\n", laux2);
						exit(EXIT_FAILURE);
					}

					/* the parasitic request command is composed of two parts */
					/* being the first the subcircuit and      #xxxxx______#  */
					/* the second, the characterization values #______yyyyy#  */
					/**/
					/* Step1: */
					k = strpos2(laux, ".sub ", 1);
					j=k;
					while (line[j - 1] != ' ')
						j--;
					strsub(laux2, line, j+1, k-j-1); /* laux2 has the subcircuit name */
					sprintf(laux3, "#%s#", laux2);
					ReadKey(laux2, laux3, fspice_cfg);  /* looks for key defining the subcircuit */
					if (strcmp(laux2, laux3)) {
					}

					fgets2(laux2, LONGSTRINGSIZE, fspice_cfg);
					strsub(laux3, laux2, 1, 7); /* detects 'Device:' */
					if (strcmp(laux3, "Device:")) {
						printf("rfmodule.c - RFModule -- Incorrect line format, 'Device:' key is missing: %s\n", laux2);
						exit(EXIT_FAILURE);
					} else {
					}

					fgets2(laux2, LONGSTRINGSIZE, fspice_cfg);
					strsub(laux3, laux2, 1, 9); /* detects 'Terminal:' */
					if (strcmp(laux3, "Terminal:")) {
						k=inlinestrpos(line);
						ReadSubKey(laux2, line, &k, '#', '_', 0);
						printf("rfmodule.c - RFModule -- Incorrect format: 'Terminal' key missing in rfmodule.cfg. Subcircuit definition '%s'\n", laux2);
						exit(EXIT_FAILURE);
					}


					/**/
					/* Step2: Looks for device characterization values group */
					k=inlinestrpos(line);
					ReadSubKey(laux2, line, &k, '#', '#', 0);
					sprintf(laux3, "#%s#", laux2);
					ReadKey(laux2, laux3, fspice_cfg); /* looks for device characterization values group */
					if (strcmp(laux2, laux3)) {
						printf("INFO:  rfmodule.c - RFModule -- '%s' device characterization key in '%s' not found in rfmodule.cfg\n", laux3, line);
						exit(EXIT_FAILURE);
					}


					/**/
					/* Step3: Good entry has been found. Check if it already exists in memory */
					strsub(laux2, laux3, 2, (int)strlen(laux3)-2);
					strcpy(laux3, laux2);
					k = strpos2(laux, ".sub ", 1);
					j=k;
					while (line[j - 1] != ' ')
						j--;
					strsub(laux2, line, j+1, k-j-1); /* laux2 has the subcircuit name */
					/*k=inlinestrpos(line);*/
					/*ReadSubKey(laux2, line, &k, '#', '#', 0);*/
					i=0;
					while (rf[i].subckt[0]  != '\0')
						if ( strcmp(rf[i].subckt, laux2) || strcmp(rf[i].subkey, laux3) ) {
							i++;
							if (i>MAXRFSUBCKT) {
								printf("rfmodule.c - RFModule -- Maximum limit of %d subcircuits reached. Increase MAXRFSUBCKT in rfmodule.h\n", MAXRFSUBCKT);
								exit(EXIT_FAILURE);
							}
						} else
							break;

					j=0;
					while (rf[j].subckt[0]  != '\0')
							if (strcmp(rf[j].subckt, laux2)) {
								j++;
							} else {
								j=MAXRFSUBCKT; /* encodes that the subcircuit already exists in memory */
								break;
							}


					k=inlinestrpos(line);
					strcpy(rf[i].subckt, laux2);
					/*ReadSubKey(rf[i].subckt, line, &k, '#', '_', 0);*/ /* copies the subckt */
					ReadSubKey(rf[i].subkey, line, &k, '#', '#', 0); /* copies the subkey */


					/**/
					/* Step4 */
					if (j<MAXRFSUBCKT) { /* if a new subcircuit was found */
						k=inlinestrpos(line);
						strcpy(rf[i].subckt, laux2);
						/*ReadSubKey(rf[i].subckt, line, &k, '#', '_', 0);*/ /* copies the subckt */
						ReadSubKey(rf[i].subkey, line, &k, '#', '#', 0); /* copies the subkey */

					/* Step4.1: write the ".subckt" header line */
						fprintf(fout, ".subckt %s.sub ", rf[i].subckt);
						sprintf(laux3, "#%s#", rf[i].subckt);
						fseek(fspice_cfg, 0, SEEK_SET);
						ReadKey(laux2, laux3, fspice_cfg); /* looks for key defining the subcircuit */
						fgets2(laux2, LONGSTRINGSIZE, fspice_cfg); /* Device: */
						fgets2(laux2, LONGSTRINGSIZE, fspice_cfg); /* Terminal: */
						strsub(laux3, laux2, 10, (int)strlen(laux2)); /* copies the terminals    */
						fprintf(fout, "%s ", laux3);             /* and prints them to file */

						while ((laux2[0] != '#') && (line[0] != '\0') && (!feof(fspice_cfg))) {
							fgets2(laux2, LONGSTRINGSIZE, fspice_cfg);
							if ((laux2[0] != '#') && (laux2[0] != '*') ) {
								j=ExtractDeviceValuePosition(laux2);
								k = (int)strlen(laux2);
								strsub(laux3, laux2, j+1, k-j+1);
								StripSpaces(laux3);
								fprintf(fout, " %s=1", laux3);
							}
						}
						fprintf(fout, "\n");

					/* Step4.2: Write the lines in the subckt definition */
						sprintf(laux3, "#%s#", rf[i].subckt);
						fseek(fspice_cfg, 0, SEEK_SET);
						ReadKey(laux2, laux3, fspice_cfg); /* looks for key defining the subcircuit */
						fgets2(laux2, LONGSTRINGSIZE, fspice_cfg); /* Device: */
						fgets2(laux2, LONGSTRINGSIZE, fspice_cfg); /* Terminal: */
						/*strsub(laux3, laux2, 10, (int)strlen(laux2));*/ /*copies the terminals*/
						/*fprintf(fout, "%s ", laux3);            */ /*and prints them to file*/

						while ((laux2[0] != '#') && (line[0] != '\0') && (!feof(fspice_cfg))) {
							fgets2(laux2, LONGSTRINGSIZE, fspice_cfg);
							if (laux2[0] != '#') {
								if (laux2[0] != '*')
									if (laux2[0] != '\0')
										fprintf(fout, "%s\n", laux2);
							} else
								fprintf(fout, ".ENDS %s.sub\n", rf[i].subckt);
						}
					}


					/**/
					/* Step5: Write the "X... " line to <hostname>.tmp */
					j=ExtractDeviceValuePosition(line);
					strsub(laux2, line, 1, j);
					sprintf(laux3, "X%s %s.sub", laux2, rf[i].subckt);
					/*fprintf(fout, "%s", laux3);*/

					sprintf(laux3, "#%s#", rf[i].subckt);
					fseek(fspice_cfg, 0, SEEK_SET);
					ReadKey(laux2, laux3, fspice_cfg); /* looks for key defining the subcircuit */
					fgets2(laux2, LONGSTRINGSIZE, fspice_cfg); /* Device: */
					fgets2(laux2, LONGSTRINGSIZE, fspice_cfg); /* Terminal: */
					/*strsub(laux3, laux2, 10, (int)strlen(laux2));*/ /*copies the terminals*/
					/*fprintf(fout, "%s ", laux3);            */ /*and prints them to file*/


					ll=0;
					/* how many lines from the rfmodule.cfg have been read so far */
					while ((laux2[0] != '#') && (line[0] != '\0') && (!feof(fspice_cfg))) {
						fgets2(laux2, LONGSTRINGSIZE, fspice_cfg);
						if ((laux2[0] != '#') && (laux2[0] != '*') ) {
							j=ExtractDeviceValuePosition(laux2);
							k = (int)strlen(laux2);
							strsub(laux3, laux2, j+1, k-j+1);
							StripSpaces(laux3);
							if (ll==0) { /* stores the value from SPICE input file: <inputfile>.* */
								j=ExtractDeviceValuePosition(line);
								k=inlinestrpos(line);
								strsub(laux2, line, j, k-j);
								StripSpaces(laux2);
								/*fprintf(fout, " %s=%s", laux3, laux2);*/
								laux2[0]='\0';
								ll++;
							} else
								if (laux3[0] != '\0')
									{}/*fprintf(fout, " %s=_RF_", laux3);*/
						}
					}

					k=inlinestrpos(line);
					strsub(laux3, line, k-1, (int)strlen(line));
					/*fprintf(fout, "%s", laux3);*/

					/*fprintf(fout, "\n");*/


					/**/
					/*Step6: Read the tabled data from rfmodule.cfg to memory */
					sprintf(laux3, "#%s#", rf[i].subckt);
					fseek(fspice_cfg, 0, SEEK_SET);
					ReadKey(laux2, laux3, fspice_cfg); /* looks for key defining the subcircuit */
					sprintf(laux3, "#%s#", rf[i].subkey);
					ReadKey(laux2, laux3, fspice_cfg); /* looks for key defining the subkey */

					j=0;
					fgets2(laux2, LONGSTRINGSIZE, fspice_cfg);
					while ((laux2[0] != '#') && (line[0] != '\0') && (!feof(fspice_cfg))) {
						strcpy(rf[i].line[j], laux2);
						j++;
						fgets2(laux2, LONGSTRINGSIZE, fspice_cfg);
						if (j > MAXTABLELINES) {
							printf("rfmodule.c - RFModule -- Maximum number of %d lines in rfmodule.cfg exceeded. Increase MAXTABLELINES in rfmodule.h\n", MAXTABLELINES);
							exit(EXIT_FAILURE);
						}
					}


					/**/
					/* Step7: */
					fclose(fspice_cfg);
					return 0;
				}
				break;
/* ---------------------------------------------------------- */
/* ---------------------------------------------------------- */
			default:
				break;
		}
	} else {         /* we are optimizing, replace subcircuit devices (X... ) */

		if (!(k=inlinestrpos(line))) /* if no in-line comments exist */
			return 0;            /* return '0' because it does not has parasitics */
		ReadSubKey(laux2, line, &k, '#', '#', 0);
		if (laux2[0]=='\0')
			return 0;            /* does not has parasitics, as indicated after the in-line comment */
		else {
			i=1;
			ReadSubKey(laux2, line, &i, '#', '#', 0); /* stores in 'i' the position of the first ## */
			k=inlinestrpos(line);                     /* stores in 'k' the position of the in-line comment */
			if (i<k) { /* Case1: 'line' contains ## before the in-line comment */
				ReplaceSymbol(line, 1);
			} else {   /* Case2: 'line' does not contains ## before the in-line comment */
			}
			/* From this momment onwards, both cases can be treated by the same subroutine
			  to fill the device parasitics from the table  */

			k = strpos2(laux, ".sub ", 1); /* Due to character case variation, it must be laux and not line */
			j=k;
			while (line[j - 1] != ' ')
				j--;
			strsub(laux2, line, j+1, k-j-1); /* laux2 has the subcircuit name */
			k=inlinestrpos(line);
			k=strpos2(line, "_", k);         /* laux3 will have the subkey name */
			if (k) { /* Device was entered as a R,L or C */
				ReadSubKey(laux3, line, &k, '_', '#', 0);
			} else { /* Device was entered as a X */
				k=inlinestrpos(line);
				ReadSubKey(laux3, line, &k, '#', '#', 0);
			}

			i=0; /* find the current subcircuit position in memory */
			while ( strcmp(rf[i].subckt, laux2) || strcmp(rf[i].subkey, laux3) ) {
				i++;
				if (i>MAXRFSUBCKT) {
					printf("rfmodule.c - RFModule -- Not found in rfmodule.cfg: %s and/or %s from line '%s'\n", laux2, laux3, line);
					exit(EXIT_FAILURE);
				}
			}

			k = strpos2(laux, ".sub ", 1); /* Due to character case variation, it must be laux and not line */
			ReadSubKey(x_string, line, &k, ' ', '=', 5);
			StripSpaces(x_string);
			j=0;
			while (strlen(rf[i].line[j])) { /* Enforce equal character case */
				ll=strpos2(rf[i].line[j], x_string, 1);
				strsub(laux3, rf[i].line[j], ll, (int)strlen(x_string));
				if (strcmp(x_string, laux3)) {
				printf("rfmodule.c - RFModule -- Incorrect character case: '%s' is different than '%s' in rfmodule.cfg\n", line, rf[i].line[j]);
					exit(EXIT_FAILURE);
				}
				j++;
			}
			ReadSubKey(x_string, line, &k, '=', ' ', 5);     /* read string of the on which we want to find the parasitics  */
			x_value=asc2real(x_string, 1, (int)strlen(x_string)); /* and convert it to a numerical value */

			j = strpos2(line, "=_RF_ ", 1);
			if (j==0) { /* in reality the subcircuit does not has parasistics */
				fprintf(fout, "%s\n", line);
				return 1;
			}
			if (j> inlinestrpos(line)) /* no parasitics exist before the in-line comment */
				return 1;

			/* At this point we are certain that at least one parasitic exist in the current subcircuit */

			while (strpos2(line, "=_RF_ ", 1)) { /* while there is values to replace... */
				/* X_VALUES for linear interpolation -- Get the correct table lines from the rfmodule.cfg */
				prev=0; /*number '0' is used for posterior value validation*/
				k=1;
				ReadSubKey(x0_string, rf[i].line[prev], &k, '=', ' ', 5);
				x0_value=asc2real(x0_string, 1, (int)strlen(x0_string));
				while (x0_value < x_value) {
					prev++;
					if (prev>= MAXTABLELINES) {
						printf("rfmodule.c - RFModule -- Value (%s) higher than the last value in table: %s\n", x_string, line);
						exit(EXIT_FAILURE);
					}
					k=1;
					ReadSubKey(x0_string, rf[i].line[prev], &k, '=', ' ', 5);
					x0_value=asc2real(x0_string, 1, (int)strlen(x0_string));
				}
/*
#define FLOAT_EQ(x, v, epsilon)   (((v - epsilon) < x) && (x < (v + epsilon)))
if (FLOAT_EQ(x0_value, x_value, 1e-020))
	prev++;
*/
				if (x0_value == x_value) /* Special case when (x0_value = x_value), meaning */
					prev++;          /* that it is equal to one of table extremes       */
				/* if (prev==0) { */           /* The following code exist due to numerical error defining how close x0_value == x_value */
				/*	if ((x0_value - x_value) < 1e-20) { */
				/*		prev++;  */         /* that it is equal to one of table extremes  */
				/*		x_value=x0_value; */
				/*	} */
				/*} */
				if (prev==0) { /*validation*/
					printf("rfmodule.c - RFModule -- Value (%s) is lower than the first value in table: %s\n", x_string, line);
					exit(EXIT_FAILURE);
				} else {
					prev--;
					k=1;
					ReadSubKey(x0_string, rf[i].line[prev], &k, '=', ' ', 5);
					x0_value=asc2real(x0_string, 1, (int)strlen(x0_string));
				}
				next=prev;
				k=1;
				ReadSubKey(x1_string, rf[i].line[next], &k, '=', ' ', 5);
				x1_value=asc2real(x1_string, 1, (int)strlen(x1_string));
				while (x_value > x1_value) {
					next++;
					if (next>= MAXTABLELINES) {
						printf("rfmodule.c - RFModule -- Device value (%s) outside table boundaries: %s\n", x_string, line);
						exit(EXIT_FAILURE);
					}
					k=1;
					ReadSubKey(x1_string, rf[i].line[next], &k, '=', ' ', 5);
					x1_value=asc2real(x1_string, 1, (int)strlen(x1_string));
				}

				/* Y_VALUES for linear interpolation -- */
				j = strpos2(line, "=_RF_ ", 1);
				while (line[j - 1] != ' ')
					j--;
				ReadSubKey(laux2, line, &j, ' ', '=', 5);
				sprintf(laux3, " %s=", laux2);
				j=strpos2(rf[i].line[prev], laux3, 1);
				if (j==0) {
					printf("rfmodule.c - RFModule -- 1 Incorrect format: '%s' from line '%s' is not found in rfmodule.cfg line '%s'\n", laux3, line, rf[i].line[prev]);
					exit(EXIT_FAILURE);
				}
				ReadSubKey(y0_string, rf[i].line[prev], &j, '=', ' ', 5);
				y0_value=asc2real(y0_string, 1, (int)strlen(y0_string));
				j=strpos2(rf[i].line[next], laux3, 1);
				if (j==0) {
					printf("rfmodule.c - RFModule -- 2 Incorrect format: '%s' from line '%s' is not found in rfmodule.cfg line '%s'\n", laux3, line, rf[i].line[next]);
					exit(EXIT_FAILURE);
				}
				ReadSubKey(y1_string, rf[i].line[next], &j, '=', ' ', 5);
				y1_value=asc2real(y1_string, 1, (int)strlen(y1_string));

				/* Linar Interpolation */
				if (x0_value == x_value)
					y_value=y0_value;
				else
					y_value= y0_value + ( (x_value-x0_value)/(x1_value-x0_value) )*(y1_value-y0_value);
				sprintf(y_string, "%E", y_value);
				if (!strcasecmp(y_string, "nan")) {
					printf("rfmodule.c - RFModule -- Interpolated value result in 'y_string' is NAN.\n");
					exit(EXIT_FAILURE);
				}

				ReplaceSymbolRF(line, y_value);
			}
			fprintf(fout, "%s\n", line);
			return 1;
		}
	}

	return 0;
} /*RFModule*/
