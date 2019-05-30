/*
 *  Copyright (C) 2005 Joao Ramos
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




#define MAXMEAS  300




typedef double line_stats[MAXMEAS];

typedef struct {
	line_stats avg, sig, max, min;
} statistics;




/* void PrintOneLine(char *lkk1, double *stats, int num_measures, FILE **fOut); */
/* void WriteStats(int num_measures, int data_lines, statistics stats, FILE **fOut); */
void CreateStatistics(char *InputFile, char *OutputFile);
