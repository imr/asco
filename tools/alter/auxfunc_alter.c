/*
 * Copyright (C) 1999-2006 Joao Ramos
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
	if (data[(int)strlen(data) - 1] == ']') {
		data[(int)strlen(data) - 1] = '\0';
	}

	strcpy(Result, data);
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
	/*Zero variables*/
	for (i = 0; i <= ALTERLINES-1; i++) {
		*alter[i].text = '\0';
		for (j = 0; j <= SWEEPVARS-1; j++)
			*alter[i].data[j] = '\0';
	}

	for (i = 0; i <= SWEEPLINES-1; i++) {
		sweep[i] = 1;
		order[i] = 1;
	}


	/**/
	/*Open input and output files*/
	if ((fsweepINI=fopen(ConfigFile,"rt")) == 0) {
		printf("auxfunc_alter.c - Cannot open config file: %s\n", ConfigFile);
		exit(EXIT_FAILURE);
	}
	if (append) {
		if ((falterINC=fopen(OutputFile,"r+t")) == 0) { /*append to a file*/
			printf("auxfunc_alter.c - Cannot write to output file: %s\n", OutputFile);
			exit(EXIT_FAILURE);
		}
	} else {
		if ((falterINC=fopen(OutputFile,"wt")) == 0) { /*create an empty file for writing*/
			printf("auxfunc_alter.c - Cannot write to output file: %s\n", OutputFile);
			exit(EXIT_FAILURE);
		}
	}


	/**/
	/**/
	ReadKey(lkk, "# ALTER #", fsweepINI);
	if (!lkk[0]) {
		printf("auxfunc_alter.c - # ALTER # key not found\n");
		exit(EXIT_FAILURE);
	} else {
		fgets2(lkk, LONGSTRINGSIZE, fsweepINI);
		i = 1;
		ptr = 1;
		while (lkk[0] != '#') {   /*this block will read the options in the configuration file*/
			if (lkk[0] != '*') {
				if (i > (ALTERLINES)) {
					printf("auxfunc_alter.c - Maximum number of %d lines reached. Increase ALTERLINES in auxfunc_alter.h\n", ALTERLINES);
					exit(EXIT_FAILURE);
				}

				j = strpos2(lkk, "$", 1); /*This will skip the characters after '$', the inline comment used by the sweep tools*/
				if (j != 0)
					sprintf(lkk, "%.*s", (int)(j - 1), strcpy(data, lkk));
				StripSpaces(lkk);

				/*****/
				/*The next block reads the data to sweep*/
				j = strpos2(lkk, "[", 1);
				if (j == 0 || lkk[0] == '*')
					sprintf(alter[i - 1].text, "%.*s", (int)strlen(lkk), lkk);
				else {
					if (strpos2(lkk, "  ", j)) {
						printf("auxfunc_alter.c - More than one space exist in: %s\n", strsub(data, lkk, j, LONGSTRINGSIZE));
						exit(EXIT_FAILURE);
					}
					sprintf(alter[i - 1].text, "%.*s", (int)(j - 1), lkk);
					/*begin*/
					k = 0;
					ReadSubKey(data, lkk, &j, '[', ' ', 4);
					if (*data != '\0')
						k++;
					read_sweep_vars(alter[i - 1].data[0], data);
					ReadSubKey(data, lkk, &j, ' ', ' ', 0);
					if (*data != '\0')
						k++;
					read_sweep_vars(alter[i - 1].data[1], data);
					ReadSubKey(data, lkk, &j, ' ', ' ', 0);
					if (*data != '\0')
						k++;
					read_sweep_vars(alter[i - 1].data[2], data);
					ReadSubKey(data, lkk, &j, ' ', ' ', 0);
					if (*data != '\0')
						k++;
					read_sweep_vars(alter[i - 1].data[3], data);
					ReadSubKey(data, lkk, &j, ' ', ' ', 0);
					if (*data != '\0')
						k++;
					read_sweep_vars(alter[i - 1].data[4], data);
					ReadSubKey(data, lkk, &j, ' ', ']', 0);
					if (*data != '\0')
						k++;
					read_sweep_vars(alter[i - 1].data[5], data);
					sweep[ptr - 1] = k;
					ptr++;
					/*end;*/
				}
				/*****/
				i++;
			}
			fgets2(lkk, LONGSTRINGSIZE, fsweepINI);
		}
		num_lines = i - 1;


		/**/
		/**/
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
				printf("auxfunc_alter.c - Maximum number of %d variables to sweep reached. Increase SWEEPLINES in auxfunc_alter.h\n", SWEEPLINES);
				exit(EXIT_FAILURE);
			} else {
				printf("auxfunc_alter.c - More than 65536 simulations reached\n");
				exit(EXIT_FAILURE);
			}
		} else {
			fseek(falterINC, -5, SEEK_END);               /*properly position the pointer*/
			for (index = 1; index <= alter_times; index++) {
				add = 1;   /*build ( [1111] [1112] [1113] [1121] ...*/
				for (jj = kk - 1; jj >= 0; jj--) {
					if (sweep[jj] != 1) {
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

				j = 1;
				/*This block will print the header with the information about the sweeped vars*/
				fprintf(falterINC, ".ALTER @%d -> ", index);
				for (k = 0; k < num_lines; k++) {
					if (*alter[k].data[0] != '\0') {
						strcpy(data, alter[k].text + 0);   /*previously it had '1', but with '0' nothing is removed*/
						fputs(data, falterINC);
						l = order[j - 1];
						j++;
						fprintf(falterINC, "%s; ", alter[k].data[l - 1]);
					}
				}
				putc('\n', falterINC);

				j = 1;
				for (k = 0; k < num_lines; k++) {
					if (*alter[k].data[0] != '\0') {
						fputs(alter[k].text, falterINC);
						l = order[j - 1];
						j++;
						fprintf(falterINC, "%s\n", alter[k].data[l - 1]);
					} else {
						if (*alter[k].text != '\0')
							fprintf(falterINC, "%s\n", alter[k].text);
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
