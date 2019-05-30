/*
 *  Copyright (C) 1999-2005 Joao Ramos
 * Your use of this code is subject to the terms and conditions of the
 * GNU general public license version 2. See "COPYING" or
 * http://www.gnu.org/licenses/gpl.html
 */

//macro definitions (preprocessor #defines)
//structure, union, and enumeration declarations
//typedef declarations
//external function declarations (see also question 1.11)
//global variable declarations


#include "auxfunc.h"




#define Summary         "summary.txt"
#define MAXMEAS  75




typedef struct {
	char var_name[LONGSTRINGSIZE]; /*variable name to use*/
	char search[LONGSTRINGSIZE];   /*search this string in output file*/
	/*@@,s_column2*/
	int s_column1;                 /*starting at 's_column1' ...*/
	int line;                      /*at 'line' bellow the string already found*/
	int column1, column2;          /*copy data between 'column1' and 'column2'*/
	char data[LONGSTRINGSIZE];     /*placeholder for measured data*/
} measure_line;

typedef double line_stats[MAXMEAS];

typedef struct {
	line_stats avg, sig, max, min;
} statistics;




void MeasureFromLIS(char *ConfigFile,char *OutputFile);
void ProcessOutputFile(char *OutputFile, int mem);
//int ReadDataFromConfigFile(char *ConfigFile, char *OutputFile);
int ProcessMeasureVar(char *measure_var_line, int k, char *OutputFile);
//char *CMOSLine2Text(char *Result,int measure_line_,char *OutputFile);
//int CMOSText2Line(char *lkk2,char *OutputFile);
//void WriteStats(int num_measures,int data_lines,statistics stats,FILE **fSummary);
//void PrintOneLine(char *lkk1,double *stats_,int num_measures,FILE **fSummary);
//void WriteToFile(int num_measures,char *laux_,int first,statistics *stats,FILE **fSummary);
void DoMath(int num_measures);




measure_line measure[MAXMEAS];
