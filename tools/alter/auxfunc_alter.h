/*
 * Copyright (C) 1999-2006 Joao Ramos
 * Your use of this code is subject to the terms and conditions of the
 * GNU general public license version 2. See "COPYING" or
 * http://www.gnu.org/licenses/gpl.html
 */

/* macro definitions (preprocessor #defines)
 * structure, union, and enumeration declarations
 * typedef declarations
 * external function declarations (see also question 1.11)
 * global variable declarations                           */


#include "auxfunc.h"




#define ALTERLINES 23 /* total number of text lines in the ALTER section (SWEEPLINES + x) */
#define SWEEPLINES 20 /* of which, upto a total of SWEEPLINES have variables to sweep     */
#define SWEEPVARS  6  /* hard limit so far. The number 6 is represented by "+    vddd=[1 2 3 4 5 6]" */




typedef struct line {
	char text[LONGSTRINGSIZE];
	char data[SWEEPVARS][LONGSTRINGSIZE];
} alter_line;




void CreateALTERinc(char *ConfigFile,char *OutputFile,int append);
/* void read_sweep_vars(char *Result, char *data) */
