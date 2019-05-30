/*
 * Copyright (C) 1999-2011 Joao Ramos
 * Your use of this code is subject to the terms and conditions of the
 * GNU general public license version 2. See "COPYING" or
 * http://www.gnu.org/licenses/gpl.html
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "auxfunc_alter.h"
#include "auxfunc.h"




/***************************************************************/
/*CreateALTERinc ***********************************************/
/***************************************************************/

/*
 *
 */
void read_sweep_vars(char *Result, char *data)
{
	strcpy(Result, data);

	if (Result[(int)strlen(Result) - 1] == ']') {
		Result[(int)strlen(Result) - 1] = '\0';
	}
} /*read_sweep_vars*/




/*
 * create the file 'alter.inc' to be included in <inputfile>.* file. Depending
 * the value of the 'append' variable, the data is append or a new file is
 * created.
 */
void CreateALTERinc(char *ConfigFile, char *OutputFile, int append)
{
	int i, j, k, kk, add, jj, l;
	char data[LONGSTRINGSIZE];
	FILE *fsweepINI, *falterINC;
	alter_line alter[ALTERLINES]; /*ALTERLINES in 'sweep.ini' file*/
	int sweep[SWEEPLINES];        /*SWEEPLINES variables: [2123] */
	int order[SWEEPLINES];        /*SWEEPLINES variables: [1111] [1112] [1113] [1121] ...*/
	int ptr, num_lines, index, alter_times;


	/**/
	/*Step0: zero variables*/
	for (i = 0; i <= ALTERLINES-1; i++) {
		*alter[i].text = '\0';
		for (j = 0; j <= SWEEPVARS-1; j++)
			*alter[i].data[j] = '\0';
	}

	for (i = 0; i <= SWEEPLINES-1; i++) {
		sweep[i] = 0;
		order[i] = 0;
	}


	/**/
	/*Step1: open input and output files*/
	if ((fsweepINI=fopen(ConfigFile,"rt")) == 0) {
		printf("auxfunc_alter.c -  CreateALTERinc -- Cannot open config file: %s\n", ConfigFile);
		exit(EXIT_FAILURE);
	}
	if (append) {
		if ((falterINC=fopen(OutputFile,"r+t")) == 0) { /*append to a file*/
			printf("auxfunc_alter.c -  CreateALTERinc -- Cannot write to output file: %s\n", OutputFile);
			exit(EXIT_FAILURE);
		}
	} else {
		if ((falterINC=fopen(OutputFile,"wt")) == 0) { /*create an empty file for writing*/
			printf("auxfunc_alter.c -  CreateALTERinc -- Cannot write to output file: %s\n", OutputFile);
			exit(EXIT_FAILURE);
		}
	}


	/**/
	/**/
	ReadKey(lkk, "# ALTER #", fsweepINI);
	if (!lkk[0]) {
		printf("auxfunc_alter.c -  CreateALTERinc -- # ALTER # key not found\n");
		exit(EXIT_FAILURE);
	} else {
		/**/
		/*Step2: read the options in the configuration file*/
		fgets2(lkk, LONGSTRINGSIZE, fsweepINI);
		switch(spice) { /*Spectre special case*/
			case 1: /*Eldo*/
			case 2: /*HSPICE*/
				break;
			case 3: /*LTspice*/
				printf("auxfunc_alter.c - CreateALTERinc -- LTspice not supported\n");
				exit(EXIT_FAILURE);
			case 4: /*Spectre*/
				if (!strpos2(lkk, " altergroup", 1)) {
					printf("auxfunc_alter.c - CreateALTERinc -- 'altergroup' must exist in Spectre mode\n");
					exit(EXIT_FAILURE);
				}
				break;
			case 50: /*Qucs*/
				printf("auxfunc_alter.c - CreateALTERinc -- Qucs not supported\n");
				exit(EXIT_FAILURE);
			case 100: /*general*/
				printf("auxfunc_alter.c - CreateALTERinc -- GENERAL not supported\n");
				exit(EXIT_FAILURE);
			default:
				printf("auxfunc_alter.c - CreateALTERinc -- Something unexpected has happened!\n");
				exit(EXIT_FAILURE);
		}
		i = 1;
		ptr = 0;
		while (lkk[0] != '#') {
			if (lkk[0] != '*') {
				if (i > (ALTERLINES)) {
					printf("auxfunc_alter.c -  CreateALTERinc -- Maximum number of %d lines reached. Increase ALTERLINES in auxfunc_alter.h\n", ALTERLINES);
					exit(EXIT_FAILURE);
				}

				j = strpos2(lkk, "$", 1); /*This will skip the characters after '$', the inline comment used by the sweep tools*/
				if (j != 0)
					lkk[j-1]='\0'; /* sprintf(lkk, "%.*s", (int)(j - 1), strcpy(data, lkk)); */
				StripSpaces(lkk);


				/*****/
				/*The next block reads the data to sweep*/
				j = strpos2(lkk, "[", 1);
				if (j == 0 || lkk[0] == '*') {
					sprintf(alter[i - 1].text, "%.*s", (int)strlen(lkk), lkk);
					i++;
				}
				else {
					if (strpos2(lkk, "  ", j)) {
						printf("auxfunc_alter.c -  CreateALTERinc -- More than one space exist in: %s\n", strsub(data, lkk, j, LONGSTRINGSIZE));
						exit(EXIT_FAILURE);
					}
					if (strpos2(lkk, "[ ", j)) {
						printf("auxfunc_alter.c -  CreateALTERinc -- Space exist after '[' in: %s\n", strsub(data, lkk, j, LONGSTRINGSIZE));
						exit(EXIT_FAILURE);
					}
					if (strpos2(lkk, " ]", j)) {
						printf("auxfunc_alter.c -  CreateALTERinc -- Space exist before ']' in: %s\n", strsub(data, lkk, j, LONGSTRINGSIZE));
						exit(EXIT_FAILURE);
					}
					sprintf(alter[i - 1].text, "%.*s", (int)(j - 1), lkk);
					/*begin*/
					k = 0;
					ReadSubKey(data, lkk, &j, '[', ' ', 4);
					if (*data != '\0') {
						k++;
						read_sweep_vars(alter[i - 1].data[0], data);
					}

					l=1;
					while (l<SWEEPVARS) {
						ReadSubKey(data, lkk, &j, ' ', ' ', 0);
						if (*data != '\0') {
							k++;
							read_sweep_vars(alter[i - 1].data[l], data);
						} else
							break;
						l++;
					}

					sweep[ptr] = k;
					order[ptr] = 1;
					ptr++;
					/*end;*/
					i++;
				}
				/*****/
			}
			fgets2(lkk, LONGSTRINGSIZE, fsweepINI);
		}
		num_lines = i - 1;


		/**/
		/*Step3: */
		alter_times = 1;   /*alter_times: how many simulations?*/
		kk = 0;
		for (i = 0; i < num_lines; i++) {
			if (*alter[i].data[0] != '\0') {
				kk++;
				alter_times *= sweep[kk - 1];
			}
		}

		if (alter_times > 65536L || kk > SWEEPLINES) {
			if (kk > SWEEPLINES) {
				printf("auxfunc_alter.c -  CreateALTERinc -- Maximum number of %d variables to sweep reached. Increase SWEEPLINES in auxfunc_alter.h\n", SWEEPLINES);
				exit(EXIT_FAILURE);
			} else {
				printf("auxfunc_alter.c -  CreateALTERinc -- More than 65536 simulations reached\n");
				exit(EXIT_FAILURE);
			}
		} else {
			if (append) { /*Assumes that is comming from errfunc.c*/
				switch(spice) {
					case 1: /*Eldo*/
					case 2: /*HSPICE*/
						fseek(falterINC, -5, SEEK_END);               /*properly position the pointer*/
						break;
					case 3: /*LTspice*/
						printf("auxfunc_alter.c - CreateALTERinc -- LTspice not supported\n");
						exit(EXIT_FAILURE);
					case 4: /*Spectre*/
						break;
					case 50: /*Qucs*/
						printf("auxfunc_alter.c - CreateALTERinc -- Qucs not supported\n");
						exit(EXIT_FAILURE);
					case 100: /*general*/
						printf("auxfunc_alter.c - CreateALTERinc -- GENERAL not supported\n");
						exit(EXIT_FAILURE);
					default:
						printf("auxfunc_alter.c - CreateALTERinc -- Something unexpected has happened!\n");
						exit(EXIT_FAILURE);
				}
				#ifdef __MINGW32__
				fseek(falterINC, -1, SEEK_CUR);               /*properly position the pointer*/
				#endif
			}

			switch(spice) { /*Spectre special case*/
				case 1: /*Eldo*/
				case 2: /*HSPICE*/
					break;
				case 3: /*LTspice*/
					break;
					printf("auxfunc_alter.c - CreateALTERinc -- LTspice not supported\n");
				case 4: /*Spectre*/
					fprintf(falterINC, "simulator lang=spectre\n\n");
					break;
				case 50: /*Qucs*/
					printf("auxfunc_alter.c - CreateALTERinc -- Qucs not supported\n");
					break;
				case 100: /*general*/
					printf("auxfunc_alter.c - CreateALTERinc -- GENERAL not supported\n");
					break;
				default:
					printf("auxfunc_alter.c - CreateALTERinc -- Something unexpected has happened!\n");
					exit(EXIT_FAILURE);
			}


			/**/
			/*Step4*/
			for (index = 1; index <= alter_times; index++) {
				add = 1;   /*build ( [1111] [1112] [1113] [1121] ...*/
				for (jj = kk - 1; jj >= 0; jj--) {
					if (sweep[jj] != 0) {
						if (order[jj] != sweep[jj]) {
							if (index != 1)
								order[jj] += add;
							add = 0;
						} else {
							if (add == 1)
								order[jj] = 1;
						}
					}
				}


				/**/
				/*Step5: print the header with the information about the sweeped vars*/
				j = 1;
				switch(spice) {
					case 1: /*Eldo*/
					case 2: /*HSPICE*/
						fprintf(falterINC, ".ALTER @%d ->", index);
						break;
					case 3: /*LTspice*/
						printf("auxfunc_alter.c - CreateALTERinc -- LTspice not supported\n");
						exit(EXIT_FAILURE);
					case 4: /*Spectre*/
						fprintf(falterINC, "//.ALTER @%d ->", index);
						break;
					case 50: /*Qucs*/
						printf("auxfunc_alter.c - CreateALTERinc -- Qucs not supported\n");
						exit(EXIT_FAILURE);
					case 100: /*general*/
						printf("auxfunc_alter.c - CreateALTERinc -- GENERAL not supported\n");
						exit(EXIT_FAILURE);
					default:
						printf("auxfunc_alter.c - CreateALTERinc -- Something unexpected has happened!\n");
						exit(EXIT_FAILURE);
				}
				for (k = 0; k < num_lines; k++) {
					if (*alter[k].data[0] != '\0') {
						strcpy(data, alter[k].text + 0);   /*previously it had '1', but with '0' nothing is removed*/
						sprintf(data, " %s", alter[k].text + 0);
						fputs(data, falterINC);
						l = order[j - 1];
						j++;
						fprintf(falterINC, "%s;", alter[k].data[l - 1]);
					}
				}
				putc('\n', falterINC);


				/**/
				/*Step6: print each one of the alter lines*/	
				i = 0; /*For Spectre only: in the following block, 'i' is for the indication of EndOfAlterGroup*/
				j = 1;
				for (k = 0; k < num_lines; k++) {
					if (*alter[k].data[0] != '\0') {
						fputs(alter[k].text, falterINC);
						l = order[j - 1];
						j++;
						fprintf(falterINC, "%s\n", alter[k].data[l - 1]);
					} else {
						if (*alter[k].text != '\0') {
							switch(spice) { /*Spectre altergroup handling*/
								case 1: /*Eldo*/
								case 2: /*HSPICE*/
									fprintf(falterINC, "%s\n", alter[k].text);
									break;
								case 3: /*LTspice*/
									printf("auxfunc_alter.c - CreateALTERinc -- LTspice not supported\n");
									exit(EXIT_FAILURE);
								case 4: /*Spectre*/
									if ( (k==0) || (i==1) ){ /*add index to altergroup line*/
										strcpy(lkk, alter[k].text);
										l = strpos2(lkk, " ", 1);
										sprintf(data, "%d", index);
										InsertString(lkk, data, l, l);
										fprintf(falterINC, "%s\n", lkk);
									} else
										fprintf(falterINC, "%s\n", alter[k].text);
									if (!strcmp(alter[k].text, "}")) /*Detects the end of altergroup as soon as it finds '}'*/
										i=1;                     /* and stores this information in variable 'i'         */
									l = strpos2(alter[k].text, "}", 1); /*Verification that detection went well*/
									if (l && (i==0) ) {
										printf("auxfunc_alter.c - CreateALTERinc -- Failed the detection of the end of altergroup\n");
										exit(EXIT_FAILURE);
									}
									break;
								case 50: /*Qucs*/
									printf("auxfunc_alter.c - CreateALTERinc -- Qucs not supported\n");
									exit(EXIT_FAILURE);
								case 100: /*general*/
									printf("auxfunc_alter.c - CreateALTERinc -- GENERAL not supported\n");
									exit(EXIT_FAILURE);
								default:
									printf("auxfunc_alter.c - CreateALTERinc -- Something unexpected has happened!\n");
									exit(EXIT_FAILURE);
							}
						}
					}
				}
				putc('\n', falterINC);
			}
		}
	}


	if (fsweepINI != NULL)
		fclose(fsweepINI);
	if (falterINC != NULL)
		fclose(falterINC);

} /*CreateALTERinc*/
