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


//double errfunc(char *filename, double *x, double xXx[][]);
double errfunc(char *filename, double *x);
/* void LogtoFile(double cost,char *hostname); */
/* double CostFunction(); */
/* int AllConstraintsMet(); */

double maxcost;
