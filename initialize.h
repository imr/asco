/*
 * Copyright (C) 2004-2016 Joao Ramos
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




#define LOG               1 /* if equal to 1, log each simulation to file */
#define MAXPARAMETERS    30 /* maximum number of parameters in <inputfile>.cfg */
#define MAXMEASUREMENTS  30 /* maximum number of measurements in <inputfile>.cfg */

/* #define SIMULATOR                eldo */
/* #define SIMULATORFILEEXTENSION    cir */
#define UNIQUECHAR                "Z" /*Character/Word added to the beginning of the symbol*/




/*
 * holds "# Parameters #" from the <inputfile>.cfg file
 */
typedef struct {
	char name[LONGSTRINGSIZE];   /*general description                                                    */
	char symbol[LONGSTRINGSIZE]; /*symbol in the SPICE input netlist to find and replace by the optimizer */
	double value;                /*initial value used for first simulation; also during optimization      */
	double minimum;              /*minimum value                                                          */
	double maximum;              /*maximum value                                                          */
	int format;                  /*number format: LIN_DOUBLE=1, LIN_INT=2, LOG_DOUBLE=3, LOG_INT=4        */
	int optimize;                /*specify if this is a parameter to optimize: OPT=1, ---=0               */
} parameters_line;

/*
 * holds "# Measurements #" from the <inputfile>.cfg file and later, the
 * measured cost if the constraint is met
 */
typedef struct {
	char meas_symbol[LONGSTRINGSIZE]; /*symbol in the SPICE output file that will be measured */
	char node[LONGSTRINGSIZE];        /*                                                      */
	int objective_constraint;         /*MIN=1, MAX=2, MON=3, LE=4, GE=5, EQ=6                 */
	double constraint_value;          /*specified by the user                                 */
	double measured_value;            /*measured value from SPICE output file simulation      */
	int constraint_met;               /*1 if constraint is met, 0 otherwise                   */
} measurements_line;




int initialize(char *filename);
/* void DecodeSymbolNode(char *ret, int i) */
void ReplaceSymbol(char *ret, int optimize);
double scaleto(double value, double ina, double inb, double outa, double outb, int format);




parameters_line parameters[MAXPARAMETERS];       /**/
measurements_line measurements[MAXMEASUREMENTS]; /**/
/*
 * to know if an ALTER and/or MC simulation is to be performed
 * 0  0 (0) => nothing
 * 0  1 (1) => MC
 * 1  0 (2) => ALTER
 * 1  1 (3) => ALTER + MC
 */
int AlterMC;           /* execute AlterMC simulation? yes, no */
double AlterMCcost;    /* execute AlterMC simulation if the last cost is smaller than this value. */
double Wobj, Wcon;     /* weights for the cost due to the objectives and constraints */
int ExecuteRF; /* add parasitics using the RF module? yes, no */
/*Think on merging the above variables*/
