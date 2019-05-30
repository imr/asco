/*
 * Copyright (C) 1999-2016 Joao Ramos
 * Your use of this code is subject to the terms and conditions of the
 * GNU general public license version 2. See "COPYING" or
 * http://www.gnu.org/licenses/gpl.html
 *
 * Original version converted from "Turbo PASCAL" to C using "p2c"
 *
 *
 *
 * Credit is given here to the author of the translation program
 *
 * "p2c"  Copyright 1989, 1990, 1991  Free Software Foundation, Inc.
 *
 * Written and maintained by:   Dave Gillespie
 *                              256-80 Caltech
 *                              Pasadena CA 91125
 *                              daveg@csvax.cs.caltech.edu, cit-vax!daveg
 *
 * The GNU License Agreement restrictions do _not_ apply to code generated
 * by p2c, nor to the p2c run-time files "p2clib.c" and "p2c.h".
 */



/*BEGIN*/
/*Run-time library for use with "p2c", the Pascal to C translator*/
#include <stdio.h>
/* #include <ctype.h> */
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "auxfunc.h"




/*Check if at end of file, using Pascal "eof" semantics.  End-of-file for
   stdin is broken; remove the special case for it to be broken in a
   different way.*/

int P_eof(FILE *f)
{
	register int ch;

	if (feof(f))
		return 1;
	if (f == stdin)
		return 0;    /*not safe to look-ahead on the keyboard!*/
	ch = getc(f);
	if (ch == EOF)
		return 1;
	ungetc(ch, f);
	return 0;
}


/*Store in "ret" the substring of length "len" starting from "pos" (1-based).
   Store a shorter or null string if out-of-range.  Return "ret".*/

char *strsub(register char *ret, register char *s, register int pos, register int len)
{
	register char *s2;

	if (--pos < 0 || len <= 0) {
		*ret = 0;
		return ret;
	}
	while (pos > 0) {
		if (!*s++) {
		*ret = 0;
		return ret;
		}
		pos--;
	}
	s2 = ret;
	while (--len >= 0) {
		if (!(*s2++ = *s++))
		return ret;
	}
	*s2 = 0;
	return ret;
}


/*Return the index of the first occurrence of "pat" as a substring of "s",
   starting at index "pos" (1-based).  Result is 1-based, 0 if not found.*/

int strpos2(char *s, register char *pat, register int pos)
{
	register char *cp, ch;
	register int slen;

	if (--pos < 0)
		return 0;
	slen = (int)strlen(s) - pos;
	cp = s + pos;
	if (!(ch = *pat++))
		return 0;
	pos = (int)strlen(pat);
	slen -= pos;
	while (--slen >= 0) {
		if (*cp++ == ch && !strncmp(cp, pat, pos))
			return cp - s;
	}
	return 0;
}
/*Run-time library for use with "p2c", the Pascal to C translator*/
/*END*/




/*###################################################################################
   ###################################################################################
   ###################################################################################*/




/*
 * Compares two floating point numbers to a given precision. Returns
 * '-1' if a<b
 * ' 0' if a=b
 * '+1' if a>b
 */
int fcmp(double a, double b)
{
	if (fabs(a-b) < 1e-50) /*Accuracy is 1e-50*/
		return 0; /* equal to zero */
	if (a<b)
		return -1; /* a<b, not equal to zero */
	else
		return +1; /* a>b, not equal to zero */
}




/*
 * Reads one line from the "stream" and properly finishes it with NULL character
 */
void fgets2(char *s, int n, FILE *stream)
{
	char *TEMP;

	(void)fgets(s, n, stream);
	TEMP = strchr(s, '\n');
	if (TEMP != NULL)
		*TEMP = 0;

	if ((int)strlen (s)>n-2) { /* n=LONGSTRINGSIZE */
		printf("INFO:  auxfunc.c - fgets2 -- string truncated, maximum string size of %d characters exceeded. Increase LONGSTRINGSIZE in auxfunc.h\n", LONGSTRINGSIZE);
		/* exit(EXIT_FAILURE); */
	}
} /*fgets2*/




/*
 * Return the index of the in-line comment, which is specific
 * to each one of the supported SPICE simulators
 */
int inlinestrpos(char *s)
{
	int k;

	switch(spice) {
		case 1: /*Eldo*/
			k = strpos2(s, "!", 1);
			break;
		case 2: /*HSPICE*/
			k = strpos2(s, "$", 1);
			break;
		case 3: /*LTspice*/
			k = strpos2(s, ";", 1);
			break;
		case 4: /*Spectre*/
			k = strpos2(s, "//", 1);
			break;
		case 50: /*Qucs*/
			k = 0;
			break;
		case 51: /*ngspice*/
			k = strpos2(s, ";", 1); /*ngspice supports multiple inline comments*/
			if (k==0) {
				k = strpos2(s, "$", 1);
				if (k==0) {
					k = strpos2(s, "//", 1);
				}
			}
			break;
		case 100: /*general*/
			k = 0;
			break;
		default:
			printf("auxfunc.c - inlinestrpos -- Something unexpected has happened!\n");
			exit(EXIT_FAILURE);
		}

	return k;
} /*inlinestrpos*/




/*
 * Looks in "stream" for the first occurrence of "key" starting from the current seek
 * point. If found "Result" contains the line which is '\0' otherwise.
 */
void ReadKey(char *Result, char *key, FILE *stream)
{
	char lkk1[LONGSTRINGSIZE];
	char STR1[LONGSTRINGSIZE];

	fgets2(lkk1, LONGSTRINGSIZE, stream);

	while ((strcmp((sprintf(STR1, "%.*s", (int)strlen(key), lkk1), STR1), key) != 0) & (!P_eof(stream)))
		fgets2(lkk1, LONGSTRINGSIZE, stream);

	if (strcmp((sprintf(STR1, "%.*s", (int)strlen(key), lkk1), STR1), key))
		Result[0]='\0';
	else
		strcpy(Result, lkk1);
} /*ReadKey*/




/*
 * Within "line", and starting at "start_from", looks for the first occurrence of
 * character "begin_char" and only then for character "end_char". Compliance is used
 * to exit the program if read data is not according to the expected format.
 */
char *ReadSubKey(char *Result, char *line, int *start_from, char begin_char, char end_char, int compliance)
{
	char line_local[LONGSTRINGSIZE]; /*??further investigate the need of having a local copy*/
	int i;
	/*,k*/
	int j;
	char data[LONGSTRINGSIZE];

	strcpy(line_local, line);
	strcat(line_local, " ");

	i = *start_from;
	if (i>=(int)strlen(line_local)) { /*immediatly exits if condition is met*/
		Result[0]='\0';
		return Result;
	}
	if (i<1) {  /*Error most likely only happens due to wrong C code programming*/
                    /*as in a 1-based string, the first value start at 1            */
		printf("auxfunc.c - ReadSubKey -- start_from is zero: %s\n", line);
		exit(EXIT_FAILURE);
	}
	while ( (line_local[i - 1] != begin_char) && (i < (int)strlen(line_local)) )
		i++;

	if (i != (int)strlen(line_local))
		j = i + 1;
	else
		j = i;

	while ( (line_local[j - 1] != end_char) && (j < (int)strlen(line_local)) )
		j++;


	/**/
	/**/
	switch(compliance) {
		case 0: /*no check*/
			break;
		case 1: /*begin character only*/
			if (line_local[i-1] != begin_char) {
				printf("First not equal\n");
				printf("auxfunc.c - ReadSubKey -- Incorrect line format: %s\n", line);
				exit(EXIT_FAILURE);
			}
			break;
		case 2: /*end character only*/
			if (line_local[j-1] != end_char) {
				printf("Last not equal\n");
				printf("auxfunc.c - ReadSubKey -- Incorrect line format: %s\n", line);
				exit(EXIT_FAILURE);
			}
			break;
		case 3: /*(size must be != 0)*/
			if (i==(j-1)) {
				printf("Size is zero\n");
				printf("auxfunc.c - ReadSubKey -- Incorrect line format: %s\n", line);
				exit(EXIT_FAILURE);
			}
			break;
		case 4: /*begin + (size must be !=0)*/
			if (line_local[i-1] != begin_char) {
				printf("First not equal\n");
				printf("auxfunc.c - ReadSubKey -- Incorrect line format: %s\n", line);
				exit(EXIT_FAILURE);
			}
			if (i==(j-1)) {
				printf("Size is zero\n");
				printf("auxfunc.c - ReadSubKey -- Incorrect line format: %s\n", line);
				exit(EXIT_FAILURE);
			}
			break;
		case 5: /*begin + end + (size must be !=0)*/
			if (line_local[i-1] != begin_char) {
				printf("First not equal\n");
				printf("auxfunc.c - ReadSubKey -- Incorrect line format: %s\n", line);
				exit(EXIT_FAILURE);
			}
			if (line_local[j-1] != end_char) {
				printf("Last not equal\n");
				printf("auxfunc.c - ReadSubKey -- Incorrect line format: %s\n", line);
				exit(EXIT_FAILURE);
			}
			if (i==(j-1)) {
				printf("Size is zero\n");
				printf("auxfunc.c - ReadSubKey -- Incorrect line format: %s\n", line);
				exit(EXIT_FAILURE);
			}
			break;
		default:
			printf("auxfunc.c - ReadSubKey -- Something unexpected has happened!\n");
			exit(EXIT_FAILURE);
	}


	/**/
	/**/
	*start_from = j;
	strsub(data, line_local, (int)(i + 1), (int)(j - i - 1));
	return strcpy(Result, data);
} /*ReadSubKey*/




/*
 *
 */
void StripSpaces(char *data)
{
	int i, j;
	char STR1[LONGSTRINGSIZE];

	i = 1;
	j = (int)strlen(data);

	if (!j) /*immediatly retuns on empty string*/
		return;

	while ((data[i - 1] == ' ') && (i<j))
		i++;
	while ((data[j - 1] == ' ') && (j>1))
		j--;

	strsub(STR1, data, (int)i, (int)(j - i + 1));
	strcpy(data, STR1);
} /*StripSpaces*/




/*
 *
 */
void Str2Lower(char *data)
{
	int i, FORLIM;

	FORLIM = (int)strlen(data);
	for (i = 0; i < FORLIM; i++) {
		if (data[i] >= 65 && data[i] <= 90)
			data[i] += 32;
	}
} /*Str2Lower*/




/*
 *
 */
void Str2Upper(char *data)
{
	int i, FORLIM;

	FORLIM = (int)strlen(data);
	for (i = 0; i < FORLIM; i++) {
		if (data[i] >= 97 && data[i] <= 122)
			data[i] -= 32;
	}
} /*Str2Upper*/




/*
 *
 */
double asc2real(char *lstring_, int startIndex, int endIndex)
{
	char lstring[LONGSTRINGSIZE];
	double auxNumb;
	int code;
	char STR1[LONGSTRINGSIZE];

	auxNumb = 0.0;
	strsub(lstring, lstring_, startIndex, (endIndex - startIndex + 1));
	StripSpaces(lstring);
	Str2Lower(lstring);
	endIndex = (int)strlen(lstring);


	if (strpos2(lstring, "*", 1) || strpos2(lstring, "/", 1))  /*if we are reading*/
		return 0;                                          /*text just return */
	if (strpos2(lstring, "+", 2))                              /*'0' and do not   */
		if (!strpos2(lstring, "e+", 1))                    /*loose time       */
			return 0;                                  /*                 */
	if (strpos2(lstring, "-", 2))                              /*                 */
		if (!strpos2(lstring, "e-", 1))                    /*                 */
			return 0;                                  /*                 */
	if (strpos2(lstring, " ", 1))                              /*'                */
		return 0;                                          /*                 */
	if ((lstring[0] < 48) || (lstring[0] > 57) )               /* '0'=48 ... '9'=57 */
		if ((lstring[0] != 43) && (lstring[0] != 45) )     /* '+'=43     '-'=45 */
			return 0;                                  /*                   */
	if ((int)strlen(lstring)>2)                                /* '1u' is a valid number   */
		if ((lstring[1] < 46) || (lstring[1] > 57) )       /*                          */
			if (!strpos2(lstring, "meg", 2))           /* '1meg' is a valid number */
				if (lstring[1] != 101)             /* '1e6' is a valid number  */
					return 0;                  /*                          */


	if (lstring[endIndex - 1] > '9' && endIndex != 0) { /*finds number*/
		code = (sscanf(lstring, "%lg", &auxNumb) == 0);

		switch (lstring[endIndex - 1]) { /*finds multiple: t,g,x,k, m,u,n,p,f,a,z*/
			case 't':   /*T: tera*/
				auxNumb *= 1e+12;
				break;
			case 'g':   /*G: giga*/
				auxNumb *= 1e+09;
				break;
			case 'x':   /*M: mega*/
				auxNumb *= 1e+06;
				break;
			case 'k':   /*k: kilo*/
				auxNumb *= 1e+03;
				break;
			case 'm':   /*m: mill*/
				auxNumb *= 1e-03;
				break;
			case 'u':   /*u: micro*/
				auxNumb *= 1e-06;
				break;
			case 'n':   /*n: nano*/
				auxNumb *= 1e-09;
				break;
			case 'p':   /*p: pico*/
				auxNumb *= 1e-12;
				break;
			case 'f':   /*f: femto*/
				auxNumb *= 1e-15;
				break;
			case 'a':   /*a: atto*/
				auxNumb *= 1e-18;
				break;
			case 'z':   /*z: zepto*/
				auxNumb *= 1e-21;
				break;
			default:    /*correct this to print lkk*/
				printf("auxfunc.c - asc2real -- Something unexpected has happened!\n");
				exit(EXIT_FAILURE);
		}
	} else {/*finds number*/
		code = (sscanf(lstring, "%lg", &auxNumb) == 0);
	}

	strsub(STR1, lstring, (endIndex - 2), 3);
	if (!strcmp(STR1, "meg")) { /*finds multiple: meg*/
		sprintf(STR1, "%.*s", (endIndex - 3), lstring);
		code = (sscanf(STR1, "%lg", &auxNumb) == 0);
		auxNumb *= 1e+6;
	}

	return auxNumb;
} /*asc2real*/




/*
 *
 */
int extended2engineer(double *num)
{
	int i;

	i = 0;
	if (*num > 0) {
		while (*num >= 1000) {
			i += 3;
			*num /= 1000;
		}
		while (*num < 1) {
			i -= 3;
			*num *= 1000;
		}
		return i;
	}

	while (*num <= -1000) {
		i += 3;
		*num /= 1000;
	}
	while (*num > -1 && fcmp(*num,0)) {   /*avoids '0' input never ending cicle*/
		i -= 3;
		*num *= 1000;
	}
	return i;
} /*extended2engineer*/




/*
 * Inserts in string 'ret' the content from 'data' at 
 * starting position 'a' until end position 'b'
 * a=1,b=1 => insert at position 1
 * a=1,b=2 => replace char at position 1
 * a=1,b=3 => replace string within position [1,2]
 */
void InsertString(char *ret, char *data, int a, int b)
{
	char begin[LONGSTRINGSIZE], end[LONGSTRINGSIZE];

	strsub(begin, ret, 1, a-1);
	strsub(end,   ret, b, (int)strlen(ret));

	sprintf(ret, "%s%s%s", begin, data, end);
}
