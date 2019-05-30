/*
 * Copyright (C) 1999-2016 Joao Ramos
 * Your use of this code is subject to the terms and conditions of the
 * GNU general public license version 2. See "COPYING" or
 * http://www.gnu.org/licenses/gpl.html
 */

/* macro definitions (preprocessor #defines)
 * structure, union, and enumeration declarations
 * typedef declarations
 * external function declarations (see also question 1.11)
 * global variable declarations                            */


#define COPYRIGHT       "Copyright (c) 1999-2016 Joao Ramos"
#define GPL_INFO        "This program is distributed in the hope that it will be useful,\nbut WITHOUT ANY WARRANTY; without even the implied warranty of\nMERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\nGNU General Public License for more details."
#define LONGSTRINGSIZE  4096 /* */
#define SHORTSTRINGSIZE   64 /* */
#define TRUE  1
#define FALSE 0




void InsertString(char *ret, char *data, int a, int b);
int extended2engineer(double *num);
double asc2real(char *lstring_,int startIndex,int endIndex);
void Str2Upper(char *data);
void Str2Lower(char *data);
void StripSpaces(char *data);
char *ReadSubKey(char *Result,char *lkk_,int *start_from,char begin_char,char end_char,int compliance);
void ReadKey(char *Result,char *key,FILE *stream);
int inlinestrpos(char *s);
void fgets2(char *s,int n,FILE *stream);
int fcmp(double a, double b);
int strpos2(char *s,register char *pat,register int pos);
char *strsub(register char *ret,register char *s,register int pos,register int len);
int P_eof(FILE *f);




char lkk[LONGSTRINGSIZE];
int spice; /* 1:Eldo, 2:HPSPICE, 3:LTspice, 4:Spectre, 50:Qucs */
int MPI_EXXIT; /* exits MPI if set to 1*/
