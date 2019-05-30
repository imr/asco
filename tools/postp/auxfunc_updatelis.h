/*
 * Copyright (C) 1999-2005 Joao Ramos
 * Your use of this code is subject to the terms and conditions of the
 * GNU general public license version 2. See "COPYING" or
 * http://www.gnu.org/licenses/gpl.html
 */

/* macro definitions (preprocessor #defines)
 * structure, union, and enumeration declarations
 * typedef declarations
 * external function declarations (see also question 1.11)
 * global variable declarations                            */


#include "auxfunc.h"




#define empty     "                    "
#define NoSat     "nosat.txt" /* prints transistors which are not in SI/SAT */
#define Summary   "summary.txt" /*summary file name*/
#define SKIPTRAN  100 /*Number of transistors to skip, i.e., ignore their operation point in the nosat.txt file*/



typedef char ThreeLines[3][LONGSTRINGSIZE];




void UpdateLIS(char *ConfigFile,char *InputFile);
/* void DoCalculations(char *lelement, char *lVGS, char *lVDS, char *lVth, char *lVDSAT, double Vovd, double Voff, double Vdst, char (*stats)[LONGSTRINGSIZE], FILE **fNoSat); */




char skip[SKIPTRAN][SHORTSTRINGSIZE]; /*100 transistor to skip*/
