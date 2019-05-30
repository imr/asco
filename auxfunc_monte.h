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


typedef struct MC_CMOSdata {
	double nmos_abeta, nmos_avt, pmos_abeta, pmos_avt, s_nmos_abeta;
	double s_nmos_avt, s_pmos_abeta, s_pmos_avt, small_length; /* for small transistors */
} MC_CMOSdata;




void MonteCarlo(char *ConfigFile,char *InputFile,char *OutputFile);
/* void MCrlc(char device,double delta,char *lkk_,int *ptr,FILE *fout); */
/* void MCmosfet(MC_CMOSdata mosfet,char *lkk,char *laux,int *ptr,FILE *fout); */
/* void ReadParameter(char *s,char *data_string,double *data_value,int *jj); */
