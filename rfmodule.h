/*
 * Copyright (C) 2005-2016 Joao Ramos
 * Your use of this code is subject to the terms and conditions of the
 * GNU general public license version 2. See "COPYING" or
 * http://www.gnu.org/licenses/gpl.html
 *
 * Plug-in to add to 'Eldo', 'HSPICE', 'LTspice', 'Spectre' and 'Qucs' circuit simulator optimization capabilities
 *
 */

/* macro definitions (preprocessor #defines)
 * structure, union, and enumeration declarations
 * typedef declarations
 * external function declarations (see also question 1.11)
 * global variable declarations                            */

 
#include "auxfunc.h"




#define MAXRFSUBCKT 10   /*maximum number or subcircuits having parasitics*/
#define MAXTABLELINES 50 /*maximum number of tabled data for each one of the subcircuits*/




/*
 *
 */
typedef struct {
	char subckt[SHORTSTRINGSIZE];             /*the subcircuit and          #xxxxx______# */
	char subkey[SHORTSTRINGSIZE];             /*the characterization values #______yyyyy# */
	char line[MAXTABLELINES][LONGSTRINGSIZE]; /*10 lines of tabled data in rfmodule.cfg   */
} rfsubckt_line;




int RFModule(char *line, int optimize, FILE *fout);
/* void ReplaceSymbolRF(char *ret, double y_value); */
/* int ExtractDeviceValuePosition(char *line); */
