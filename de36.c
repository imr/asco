/*
 * Copyright (C) 2005-2006 Joao Ramos
 * Your use of this code is subject to the terms and conditions of the
 * GNU general public license version 2. See "COPYING" or
 * http://www.gnu.org/licenses/gpl.html
 *
 * Plug-in to add to 'Eldo', 'HSPICE', 'LTSpice' and 'Spectre' circuit simulator optimization capabilities
 *
 */




/***************************************************************
**                                                            **
**        D I F F E R E N T I A L     E V O L U T I O N       **
**                                                            **
** Program: de.c                                              **
** Version: 3.6                                               **
**                                                            **
** Authors: Dr. Rainer Storn                                  **
**          c/o ICSI, 1947 Center Street, Suite 600           **
**          Berkeley, CA 94707                                **
**          Tel.:   510-642-4274 (extension 192)              **
**          Fax.:   510-643-7684                              **
**          E-mail: storn@icsi.berkeley.edu                   **
**          WWW: http://http.icsi.berkeley.edu/~storn/        **
**          on leave from                                     **
**          Siemens AG, ZFE T SN 2, Otto-Hahn Ring 6          **
**          D-81739 Muenchen, Germany                         **
**          Tel:    636-40502                                 **
**          Fax:    636-44577                                 **
**          E-mail: rainer.storn@zfe.siemens.de               **
**                                                            **
**          Kenneth Price                                     **
**          836 Owl Circle                                    **
**          Vacaville, CA 95687                               **
**          E-mail: kprice@solano.community.net               **
**                                                            **
** This program implements some variants of Differential      **
** Evolution (DE) as described in part in the techreport      **
** tr-95-012.ps of ICSI. You can get this report either via   **
** ftp.icsi.berkeley.edu/pub/techreports/1995/tr-95-012.ps.Z  **
** or via WWW: http://http.icsi.berkeley.edu/~storn/litera.html*
** A more extended version of tr-95-012.ps is submitted for   **
** publication in the Journal Evolutionary Computation.       ** 
**                                                            **
** You may use this program for any purpose, give it to any   **
** person or change it according to your needs as long as you **
** are referring to Rainer Storn and Ken Price as the origi-  **
** nators of the the DE idea.                                 **
** If you have questions concerning DE feel free to contact   **
** us. We also will be happy to know about your experiences   **
** with DE and your suggestions of improvement.               **
**                                                            **
***************************************************************/
/**H*O*C**************************************************************
**                                                                  **
** No.!Version! Date ! Request !    Modification           ! Author **
** ---+-------+------+---------+---------------------------+------- **
**  1 + 3.1  +5/18/95+   -     + strategy DE/rand-to-best/1+  Storn **
**    +      +       +         + included                  +        **
**  1 + 3.2  +6/06/95+C.Fleiner+ change loops into memcpy  +  Storn **
**  2 + 3.2  +6/06/95+   -     + update comments           +  Storn **
**  1 + 3.3  +6/15/95+ K.Price + strategy DE/best/2 incl.  +  Storn **
**  2 + 3.3  +6/16/95+   -     + comments and beautifying  +  Storn **
**  3 + 3.3  +7/13/95+   -     + upper and lower bound for +  Storn **
**    +      +       +         + initialization            +        **
**  1 + 3.4  +2/12/96+   -     + increased printout prec.  +  Storn **
**  1 + 3.5  +5/28/96+   -     + strategies revisited      +  Storn **
**  2 + 3.5  +5/28/96+   -     + strategy DE/rand/2 incl.  +  Storn **
**  1 + 3.6  +8/06/96+ K.Price + Binomial Crossover added  +  Storn **
**  2 + 3.6  +9/30/96+ K.Price + cost variance output      +  Storn **
**  3 + 3.6  +9/30/96+   -     + alternative to ASSIGND    +  Storn **
**  4 + 3.6  +10/1/96+   -    + variable checking inserted +  Storn **
**  5 + 3.6  +10/1/96+   -     + strategy indic. improved  +  Storn **
**                                                                  **
***H*O*C*E***********************************************************/
#include <string.h>
#include <unistd.h>
#include <signal.h>


#include "de.h"
#include "auxfunc.h"
#include "initialize.h"
#ifdef MPI
#include "mpi.h"
#define MPI_METHOD 2 /*1:Send or 2:Scatter*/
#endif

/*------------------------Macros----------------------------------------*/

/*#define ASSIGND(a,b) memcpy((a),(b),sizeof(double)*D) */  /* quick copy by Claudio */
                                                           /* works only for small  */
                                                           /* arrays, but is faster.*/

/*------------------------Globals---------------------------------------*/

long  rnd_uni_init;                 /* serves as a seed for rnd_uni()   */
double c[MAXPOP][MAXDIM], d[MAXPOP][MAXDIM];
double (*pold)[MAXPOP][MAXDIM], (*pnew)[MAXPOP][MAXDIM], (*pswap)[MAXPOP][MAXDIM];



/*---------Function declarations----------------------------------------*/

void  assignd(int D, double a[], double b[]);
double rnd_uni(long *idum);    /* uniform pseudo random number generator */
extern double evaluate(int D, double tmp[], char *filename); /* obj. funct. */
void sigproc(int);
void quitproc(int);

/*---------Function definitions-----------------------------------------*/

void  assignd(int D, double a[], double b[])
/**C*F****************************************************************
**                                                                  **
** Assigns D-dimensional vector b to vector a.                      **
** You might encounter problems with the macro ASSIGND on some      **
** machines. If yes, better use this function although it's slower. **
**                                                                  **
***C*F*E*************************************************************/
{
   int j;
   for (j=0; j<D; j++)
   {
      a[j] = b[j];
   }
}


double rnd_uni(long *idum)
/**C*F****************************************************************
**                                                                  **
** SRC-FUNCTION   :rnd_uni()                                        **
** LONG_NAME      :random_uniform                                   **
** AUTHOR         :(see below)                                      **
**                                                                  **
** DESCRIPTION    :rnd_uni() generates an equally distributed ran-  **
**                 dom number in the interval [0,1]. For further    **
**                 reference see Press, W.H. et alii, Numerical     **
**                 Recipes in C, Cambridge University Press, 1992.  **
**                                                                  **
** FUNCTIONS      :none                                             **
**                                                                  **
** GLOBALS        :none                                             **
**                                                                  **
** PARAMETERS     :*idum    serves as a seed value                  **
**                                                                  **
** PRECONDITIONS  :*idum must be negative on the first call.        **
**                                                                  **
** POSTCONDITIONS :*idum will be changed                            **
**                                                                  **
***C*F*E*************************************************************/
{
  long j;
  long k;
  static long idum2=123456789;
  static long iy=0;
  static long iv[NTAB];
  double temp;

  if (*idum <= 0)
  {
    if (-(*idum) < 1) *idum=1;
    else *idum = -(*idum);
    idum2=(*idum);
    for (j=NTAB+7;j>=0;j--)
    {
      k=(*idum)/IQ1;
      *idum=IA1*(*idum-k*IQ1)-k*IR1;
      if (*idum < 0) *idum += IM1;
      if (j < NTAB) iv[j] = *idum;
    }
    iy=iv[0];
  }
  k=(*idum)/IQ1;
  *idum=IA1*(*idum-k*IQ1)-k*IR1;
  if (*idum < 0) *idum += IM1;
  k=idum2/IQ2;
  idum2=IA2*(idum2-k*IQ2)-k*IR2;
  if (idum2 < 0) idum2 += IM2;
  j=iy/NDIV;
  iy=iv[j]-idum2;
  iv[j] = *idum;
  if (iy < 1) iy += IMM1;
  if ((temp=AM*iy) > RNMX) return RNMX;
  else return temp;

}/*------End of rnd_uni()--------------------------*/



int DE(int argc, char *argv[])
/**C*F****************************************************************
**                                                                  **
** SRC-FUNCTION   :main()                                           **
** LONG_NAME      :main program                                     **
** AUTHOR         :Rainer Storn, Kenneth Price                      **
**                                                                  **
** DESCRIPTION    :driver program for differential evolution.       **
**                                                                  **
** FUNCTIONS      :rnd_uni(), evaluate(), printf(), fprintf(),      **
**                 fopen(), fclose(), fscanf().                     **
**                                                                  **
** GLOBALS        :rnd_uni_init    input variable for rnd_uni()     **
**                                                                  **
** PARAMETERS     :argc            #arguments = 3                   **
**                 argv            pointer to argument strings      **
**                                                                  **
** PRECONDITIONS  :main must be called with three parameters        **
**                 e.g. like de1 <input-file> <output-file>, if     **
**                 the executable file is called de1.               **
**                 The input file must contain valid inputs accor-  **
**                 ding to the fscanf() section of main().          **
**                                                                  **
** POSTCONDITIONS :main() produces consecutive console outputs and  **
**                 writes the final results in an output file if    **
**                 the program terminates without an error.         **
**                                                                  **
***C*F*E*************************************************************/

{
   /*char  chr;*/             /* y/n choice variable                */
   char  *strat[] =       /* strategy-indicator                 */
   {
            "",
            "DE/best/1/exp",
            "DE/rand/1/exp",
            "DE/rand-to-best/1/exp",
            "DE/best/2/exp",
            "DE/rand/2/exp",
            "DE/best/1/bin",
            "DE/rand/1/bin",
            "DE/rand-to-best/1/bin",
            "DE/best/2/bin",
            "DE/rand/2/bin"
   };

   int   i, j, L, n;      /* counting variables                 */
   int   r1, r2, r3, r4;  /* placeholders for random indexes    */
   int   r5;              /* placeholders for random indexes    */
   int   D;               /* Dimension of parameter vector      */
   int   NP;              /* number of population members       */
   int   imin;            /* index to member with lowest energy */
   int   refresh;         /* refresh rate of screen output      */
   int   strategy;        /* choice parameter for screen output */
   int   gen, genmax, seed;

   long  nfeval;          /* number of function evaluations     */

   double trial_cost[MAXPOP];      /* buffer variable                    */
   double inibound_h;      /* upper parameter bound              */
   double inibound_l;      /* lower parameter bound              */
   double tmp[MAXPOP][MAXDIM], best[MAXDIM], bestit[MAXDIM]; /* members  */
   double cost[MAXPOP];    /* obj. funct. values                 */
   double cvar;            /* computes the cost variance         */
   double cvarmin; /*stop criteria*/
   double cmean;           /* mean cost                          */
   double F,CR;            /* control variables of DE            */
   double cmin;            /* help variables                     */

   FILE  *fpin_ptr;
   FILE  *fpout_ptr;

   char laux[LONGSTRINGSIZE];
   int ii;

/*------Initializations----------------------------*/
	#ifdef MPI
	signal(SIGINT, SIG_DFL);   /* Ctrl-C detection*/
	signal(SIGQUIT, quitproc); /* Ctrl-\ detection*/
	#else
	signal(SIGINT, sigproc);   /* Ctrl-C detection*/
	signal(SIGQUIT, quitproc); /* Ctrl-\ detection*/
	#endif

/*mpi: MPI initialization*/
	#ifdef MPI
	double tmp_y[MAXPOP][MAXDIM], trial_cost_y[MAXPOP];
	int k, m, count;
	const int tag = 42;	        /* Message tag */
	const int root = 0;         /* Root process in scatter */
	MPI_Status status;
	MPI_Request send_req, recv_req;    /* Request object for send and receive */
	int id, ntasks, source_id, dest_id, err;
	double cost_mpi; /* Message array */
	int sendcount[MAXPOP], displs[MAXPOP];      /* Arrays for sendcounts and displacements */

	err = MPI_Comm_size(MPI_COMM_WORLD, &ntasks); /* Get nr of tasks */
	err = MPI_Comm_rank(MPI_COMM_WORLD, &id);     /* Get id of this process */
	if (ntasks < 2) {
		printf("de36.c - At least 2 processors are required to run this program\n");
		MPI_Finalize(); /* Quit if there is only one processor */
		exit(EXIT_FAILURE);
	}



	m=ntasks-1;  /*number of slave machines*/
	#endif
/*mpi: MPI initialization*/


/*-----Read input data------------------------------------------------*/



sprintf(laux, "%s%s", argv[2], ".cfg");





fpin_ptr   = fopen(laux,"r"); /*fpin_ptr   = fopen(argv[1],"r");*/

 if (fpin_ptr == NULL)
 {
    printf("de36.c - Cannot open input file: %s\n", laux);
    exit(EXIT_FAILURE);
 }

ReadKey(lkk, "#DE#", fpin_ptr); /* .cfg file*/
if (strcmp(lkk, "#DE#"))
{
	printf("\nde36.c - Cannot find #DE# category in %s.cfg\n", argv[2]);
	exit(EXIT_FAILURE);
}

fgets2(lkk, LONGSTRINGSIZE, fpin_ptr);
ii=1; ReadSubKey(laux, lkk, &ii, ':', ':', 4);
strategy=asc2real(laux, 1, (int)strlen(laux));   /*---choice of strategy-----------------*/

fgets2(lkk, LONGSTRINGSIZE, fpin_ptr);
ii=1; ReadSubKey(laux, lkk, &ii, ':', ':', 4);
genmax=asc2real(laux, 1, (int)strlen(laux));     /*---maximum number of generations------*/

fgets2(lkk, LONGSTRINGSIZE, fpin_ptr);
ii=1; ReadSubKey(laux, lkk, &ii, ':', ':', 4);
refresh=asc2real(laux, 1, (int)strlen(laux));    /*---output refresh cycle---------------*/

/*fgets2(lkk, LONGSTRINGSIZE, fpin_ptr);*/
/*ii=1; ReadSubKey(laux, lkk, &ii, ':', ':');*/
/*D=asc2real(laux, 1, (int)strlen(laux));*/          /*---number of parameters---------------*/
D=0;
while (parameters[D].name[0]  != '\0')
	D++;                                     /*---number of parameters---------------*/
#ifdef ROSEN_REGRESSION
D=2;
#endif

fgets2(lkk, LONGSTRINGSIZE, fpin_ptr);
ii=1; ReadSubKey(laux, lkk, &ii, ':', ':', 4);
NP=asc2real(laux, 1, (int)strlen(laux));         /*---population size.-------------------*/

/*fgets2(lkk, LONGSTRINGSIZE, fpin_ptr);*/
inibound_h=+10;                                  /*---upper parameter bound for init-----*/

/*fgets2(lkk, LONGSTRINGSIZE, fpin_ptr);*/
inibound_l=-10;                                  /*---lower parameter bound for init-----*/

fgets2(lkk, LONGSTRINGSIZE, fpin_ptr);
ii=1; ReadSubKey(laux, lkk, &ii, ':', ':', 4);
F=asc2real(laux, 1, (int)strlen(laux));          /*---weight factor----------------------*/

fgets2(lkk, LONGSTRINGSIZE, fpin_ptr);
ii=1; ReadSubKey(laux, lkk, &ii, ':', ':', 4);
CR=asc2real(laux, 1, (int)strlen(laux));         /*---crossing over factor---------------*/

fgets2(lkk, LONGSTRINGSIZE, fpin_ptr);
ii=1; ReadSubKey(laux, lkk, &ii, ':', ':', 4);
seed=asc2real(laux, 1, (int)strlen(laux));       /*---random seed------------------------*/

fgets2(lkk, LONGSTRINGSIZE, fpin_ptr);
ii=1; ReadSubKey(laux, lkk, &ii, ':', ':', 4);
cvarmin=asc2real(laux, 1, (int)strlen(laux));    /*---minimum cost variance--------------*/

fgets2(lkk, LONGSTRINGSIZE, fpin_ptr);
ii=1; ReadSubKey(laux, lkk, &ii, ':', ':', 4);
Wobj=asc2real(laux, 1, (int)strlen(laux));    /*---weights for the cost due to the objectives -------*/

fgets2(lkk, LONGSTRINGSIZE, fpin_ptr);
ii=1; ReadSubKey(laux, lkk, &ii, ':', ':', 4);
Wcon=asc2real(laux, 1, (int)strlen(laux));    /*---weights for the cost due to the constraints -------*/

fclose(fpin_ptr);

	#ifdef MPI
	if (ntasks > (NP+1)) {
		printf("You cannot use more than %d slave processes\n", NP);
		MPI_Finalize(); /* Quit if there is only one processor */
		exit(0);
	}

	if (MPI_METHOD==2) { /* Method 2: Master and Slave */
		/* Initialize sendcount and displacements arrays */

		/* Fills 'sendcount'*/
		j=NP%m;                    /* remainder */
		sendcount[0]=0; /* master does not perform calculations, so it receives 0 vectors */
		for (i=1; i<ntasks; i++) {
			sendcount[i]=NP/m; /* division */
			if (j) {
				sendcount[i]++;
				j--;
			}
		}
		/*Verification*/
		j=0;
		for (i=0; i<ntasks; i++) {
			j=j+sendcount[i];
		}
		if (j!=NP) { /* Just to rest assured that no error has been done */
			printf("de36.c - j!=NP\n");
			exit(EXIT_FAILURE);
		}
		for (i=0; i<ntasks; i++) {
			sendcount[i]=sendcount[i]*MAXDIM;
		}

		/*Fills 'displs'*/
		displs[0]=0; /* master */
		displs[1]=0; /* slave number 1 */
		for (i=2; i<ntasks; i++) {
			displs[i]=sendcount[i-1] + displs[i-1];
		}
	}
	#endif

/*mpi: slave code begins*/
	#ifdef MPI
	if (id) { /*If it is a slave process*/
		while(MPI_METHOD==1) { /* Method 1: Slave */
			i=0;
			err = MPI_Recv(tmp[i], D, MPI_DOUBLE, 0, tag, MPI_COMM_WORLD, &status); /* Receive a message */
			if (tmp[0][0]==1.7976931348623157e+308 /*DBL_MAX from <float.h>*/) { /*exit message*/
				MPI_Finalize();
				return (EXIT_SUCCESS);   /*exit*/
			}
			cost_mpi = evaluate(D,tmp[0],argv[2]);  /* Evaluate new vector in tmp[] */
			dest_id = 0;            /* Destination address */
			err = MPI_Send(&cost_mpi, 1, MPI_DOUBLE, dest_id, tag, MPI_COMM_WORLD);
		}
		while(MPI_METHOD==2) { /* Method 2: Slave */
			/* An unexpected error exist with some combinations of 'D' and the total *
			 * total number of processes if recvcount (number of elements in receive *
			 * buffer) is not large, instead of the expected value of 'NP*D'. Mainly *
			 * due to low values of D.                                               */

			/* Receive the scattered matrix from process 0, place it in array y */
			err = MPI_Scatterv(&tmp, sendcount, displs, MPI_DOUBLE, &tmp_y, MAXPOP*MAXDIM, MPI_DOUBLE, root, MPI_COMM_WORLD);
			if (tmp_y[0][0]==1.7976931348623157e+308 /*DBL_MAX from <float.h>*/) { /*exit message*/
				MPI_Finalize();
				return (EXIT_SUCCESS);   /*exit*/
			}

			/*Calls optimization routine*/
			for (i=0; i<sendcount[id]/MAXDIM; i++) {
				trial_cost_y[i]=evaluate(D,tmp_y[i],argv[2]);  /* Evaluate new vector in tmp[] */
			}

			MPI_Send (&trial_cost_y, sendcount[id]/MAXDIM, MPI_DOUBLE, 0, tag, MPI_COMM_WORLD);
		}
	}
	#endif
/*mpi: slave code ends*/

/*-----Checking input variables for proper range----------------------------*/

	if ((strategy < 0) || (strategy > 10)) {
		printf("de36.c - strategy=%d, should be ex {1,2,3,4,5,6,7,8,9,10}\n",strategy);
		exit(EXIT_FAILURE);
	}
	if (genmax <= 0) {
		printf("de36.c - genmax=%d, should be > 0\n",genmax);
		exit(EXIT_FAILURE);
	}
	if (refresh <= 1) {
		printf("de36.c - refresh=%d, should be > 1\n",refresh);
		exit(EXIT_FAILURE);
	}
	if (D > MAXDIM) {
		printf("de36.c - D=%d > MAXDIM=%d, should be less\n",D,MAXDIM);
		exit(EXIT_FAILURE);
	}
	if (D <= 0) {
		printf("de36.c - D=%d, should be > 0\n",D);
		exit(EXIT_FAILURE);
	}
	if (NP > MAXPOP) {
		printf("de36.c - NP=%d > MAXPOP=%d, should be less\n",NP,MAXPOP);
		exit(EXIT_FAILURE);
	}
	if (NP <= 6) {
		printf("de36.c - NP=%d, should be > 6\n",NP);
		exit(EXIT_FAILURE);
	}
	if (NP <= D) {
		printf("INFO:  de36.c - NP=%d, it is advisable to be > D=%d\n",NP, D);
	}
	if (inibound_h < inibound_l) {
		printf("de36.c - inibound_h=%f < inibound_l=%f\n",inibound_h, inibound_l);
		exit(EXIT_FAILURE);
	}
	if ((F < 0) || (F > 2.0)) {
		printf("de36.c - F=%f, should be ex [0,2]\n",F);
		exit(EXIT_FAILURE);
	}
	if ((CR < 0) || (CR > 1.0)) {
		printf("de36.c - CR=%f, should be ex [0,1]\n",CR);
		exit(EXIT_FAILURE);
	}
	if (seed <= 0) {
		printf("de36.c - seed=%d, should be > 0\n",seed);
		exit(EXIT_FAILURE);
	}
	if (cvarmin <= 0) {
		printf("de36.c - cvarmin=%f, should be > 0\n",cvarmin);
		exit(EXIT_FAILURE);
	}
	if (Wobj <= 0) {
		printf("de36.c - Wobj=%f, should be > 0\n",Wobj);
		exit(EXIT_FAILURE);
	}
	if (Wcon <= 0) {
		printf("de36.c - Wcon=%f, should be > 0\n",Wcon);
		exit(EXIT_FAILURE);
	}
	if (Wobj > Wcon) {
		printf("de36.c - Wobj=%f > Wcon=%f, should be less\n",Wobj, Wcon);
		exit(EXIT_FAILURE);
	}

/*-----Open output file-----------------------------------------------*/

   fpout_ptr   = fopen("asco.log","w");  /* open output file for writing */

   if (fpout_ptr == NULL)
   {
      printf("de36.c - Cannot open output file: asco.log\n");
      exit(EXIT_FAILURE);
   }


/*-----Initialize random number generator-----------------------------*/

 rnd_uni_init = -(long)seed;  /* initialization of rnd_uni() */
 nfeval       =  0;  /* reset number of function evaluations */



/*------Initialization------------------------------------------------*/
/*------Right now this part is kept fairly simple and just generates--*/
/*------random numbers in the range [-initfac, +initfac]. You might---*/
/*------want to extend the init part such that you can initialize-----*/
/*------each parameter separately.------------------------------------*/

   for (i=0; i<NP; i++)
   {
      for (j=0; j<D; j++) /* spread initial population members */
      {
	 c[i][j] = inibound_l + rnd_uni(&rnd_uni_init)*(inibound_h - inibound_l);
      }
   }
   #ifndef MPI
   for (i=0; i<NP; i++) /*mpi: */
   {
	nfeval++;
	if (NP)
		cost[i] = evaluate(D,c[i],argv[2]); /* obj. funct. value */
	if (cost[i]==0) { /*cost is zero ONLY if Ctrl-C has been pressed during call to simulator*/
		NP=0;
		genmax=0;
		/* printf("INFO:  de36.c - Initialization.\n", NP); */
		sleep(1);
		/* printf("waking-up...\n"); */
	}
   }
   #else /*mpi: Master code to call slave*/
	if (MPI_METHOD==1) { /* Method 1: Master */
		for (i=0; i<(int)((NP/m)+1); i++) {
			for (j=0; j<m; j++) { /* Send a message */
				if ( (j+m*i) <= (NP-1) ) {
/*----- - ----- - ----- - ----- - ----- - ----- - ----- - ----- - -----*/
					dest_id=j+1;            /* Destination address */
					nfeval++;
					err = MPI_Send(c[j+m*i], D, MPI_DOUBLE, dest_id, tag, MPI_COMM_WORLD);
/*----- - ----- - ----- - ----- - ----- - ----- - ----- - ----- - -----*/
				}
			}
			for (j=0; j<m; j++) {
				if ( (j+m*i) <= (NP-1) ) {
/*----- - ----- - ----- - ----- - ----- - ----- - ----- - ----- - -----*/
					err = MPI_Recv(&cost_mpi, 1, MPI_DOUBLE, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &status); /* Receive a message */
					source_id = (status.MPI_SOURCE); /* Get id of sender */
					cost[source_id+m*i-1]=cost_mpi;
/*----- - ----- - ----- - ----- - ----- - ----- - ----- - ----- - -----*/
				}
			}
		}
	}
	if (MPI_METHOD==2) { /* Method 2: Master */
		/* Scatter tmp to all proceses, place it in tmp_y */
		err = MPI_Scatterv(&c, sendcount, displs, MPI_DOUBLE, &tmp_y, MAXPOP*MAXDIM, MPI_DOUBLE, root, MPI_COMM_WORLD);
		nfeval=nfeval+NP;
		/* Receive messages with scattered data */
		/* from all slave processes */
		for (i=1; i<ntasks; i++) {
			MPI_Recv (&trial_cost_y, MAXPOP*MAXDIM, MPI_DOUBLE, i, tag, MPI_COMM_WORLD, &status);
			MPI_Get_count(&status, MPI_DOUBLE, &count);  /* Get nr of elements in message */
			source_id = (status.MPI_SOURCE); /* Get id of sender */

			/* Arrange cost returned from slaves */
			j=0;
			for (k=0; k<source_id; k++) {
				j=j + sendcount[k]/MAXDIM;
			}
			/* j holds the correct start index for trial_cost */
			for (k=0; k<NP; k++) {
				cost[j+k]=trial_cost_y[k];
			}
		}
	}
   #endif
   cmin = cost[0];
   imin = 0;
   for (i=1; i<NP; i++)
   {
      if (cost[i]<cmin)
      {
	 cmin = cost[i];
	 imin = i;
      }
   }

   assignd(D,best,c[imin]);            /* save best member ever          */
   assignd(D,bestit,c[imin]);          /* save best member of generation */

   pold = &c; /* old population (generation G)   */
   pnew = &d; /* new population (generation G+1) */

/*=======================================================================*/
/*=========Iteration loop================================================*/
/*=======================================================================*/

   gen = 1;                          /* generation counter reset */
   cvar = 1.7976931348623157e+308; /*DBL_MAX from <float.h>*/
   while ((gen <= genmax) && (cvar > cvarmin)/*&& (kbhit() == 0)*/) /* remove comments if conio.h */
   {                                                               /* is accepted by compiler    */
      imin = 0;

      for (i=0; i<NP; i++)         /* Start of loop through ensemble  */
      {
	 do                        /* Pick a random population member */
	 {                         /* Endless loop for NP < 2 !!!     */
	   r1 = (int)(rnd_uni(&rnd_uni_init)*NP);
	 }while(r1==i);            

	 do                        /* Pick a random population member */
	 {                         /* Endless loop for NP < 3 !!!     */
	   r2 = (int)(rnd_uni(&rnd_uni_init)*NP);
	 }while((r2==i) || (r2==r1));

	 do                        /* Pick a random population member */
	 {                         /* Endless loop for NP < 4 !!!     */
	   r3 = (int)(rnd_uni(&rnd_uni_init)*NP);
	 }while((r3==i) || (r3==r1) || (r3==r2));

	 do                        /* Pick a random population member */
	 {                         /* Endless loop for NP < 5 !!!     */
	   r4 = (int)(rnd_uni(&rnd_uni_init)*NP);
	 }while((r4==i) || (r4==r1) || (r4==r2) || (r4==r3));

	 do                        /* Pick a random population member */
	 {                         /* Endless loop for NP < 6 !!!     */
	   r5 = (int)(rnd_uni(&rnd_uni_init)*NP);
	 }while((r5==i) || (r5==r1) || (r5==r2) || (r5==r3) || (r5==r4));


/*=======Choice of strategy===============================================================*/
/*=======We have tried to come up with a sensible naming-convention: DE/x/y/z=============*/
/*=======DE :  stands for Differential Evolution==========================================*/
/*=======x  :  a string which denotes the vector to be perturbed==========================*/
/*=======y  :  number of difference vectors taken for perturbation of x===================*/
/*=======z  :  crossover method (exp = exponential, bin = binomial)=======================*/
/*                                                                                        */
/*=======There are some simple rules which are worth following:===========================*/
/*=======1)  F is usually between 0.5 and 1 (in rare cases > 1)===========================*/
/*=======2)  CR is between 0 and 1 with 0., 0.3, 0.7 and 1. being worth to be tried first=*/
/*=======3)  To start off NP = 10*D is a reasonable choice. Increase NP if misconvergence=*/
/*           happens.                                                                     */
/*=======4)  If you increase NP, F usually has to be decreased============================*/
/*=======5)  When the DE/best... schemes fail DE/rand... usually works and vice versa=====*/


/*=======EXPONENTIAL CROSSOVER============================================================*/

/*-------DE/best/1/exp--------------------------------------------------------------------*/
/*-------Our oldest strategy but still not bad. However, we have found several------------*/
/*-------optimization problems where misconvergence occurs.-------------------------------*/
	 if (strategy == 1) /* strategy DE0 (not in our paper) */
	 {
	   assignd(D,tmp[i],(*pold)[i]);
	   n = (int)(rnd_uni(&rnd_uni_init)*D);
	   L = 0;
	   do
	   {                       
	     tmp[i][n] = bestit[n] + F*((*pold)[r2][n]-(*pold)[r3][n]);
	     n = (n+1)%D;
	     L++;
	   }while((rnd_uni(&rnd_uni_init) < CR) && (L < D));
	 }
/*-------DE/rand/1/exp-------------------------------------------------------------------*/
/*-------This is one of my favourite strategies. It works especially well when the-------*/
/*-------"bestit[]"-schemes experience misconvergence. Try e.g. F=0.7 and CR=0.5---------*/
/*-------as a first guess.---------------------------------------------------------------*/
	 else if (strategy == 2) /* strategy DE1 in the techreport */
	 {
	   assignd(D,tmp[i],(*pold)[i]);
	   n = (int)(rnd_uni(&rnd_uni_init)*D);
	   L = 0;
	   do
	   {                       
	     tmp[i][n] = (*pold)[r1][n] + F*((*pold)[r2][n]-(*pold)[r3][n]);
	     n = (n+1)%D;
	     L++;
	   }while((rnd_uni(&rnd_uni_init) < CR) && (L < D));
	 }
/*-------DE/rand-to-best/1/exp-----------------------------------------------------------*/
/*-------This strategy seems to be one of the best strategies. Try F=0.85 and CR=1.------*/
/*-------If you get misconvergence try to increase NP. If this doesn't help you----------*/
/*-------should play around with all three control variables.----------------------------*/
	 else if (strategy == 3) /* similiar to DE2 but generally better */
	 { 
	   assignd(D,tmp[i],(*pold)[i]);
	   n = (int)(rnd_uni(&rnd_uni_init)*D); 
	   L = 0;
	   do
	   {
	     tmp[i][n] = tmp[i][n] + F*(bestit[n] - tmp[i][n]) + F*((*pold)[r1][n]-(*pold)[r2][n]);
	     n = (n+1)%D;
	     L++;
	   }while((rnd_uni(&rnd_uni_init) < CR) && (L < D));
	 }
/*-------DE/best/2/exp is another powerful strategy worth trying--------------------------*/
	 else if (strategy == 4)
	 { 
	   assignd(D,tmp[i],(*pold)[i]);
	   n = (int)(rnd_uni(&rnd_uni_init)*D);
	   L = 0;
	   do
	   {                           
	     tmp[i][n] = bestit[n] + 
		      ((*pold)[r1][n]+(*pold)[r2][n]-(*pold)[r3][n]-(*pold)[r4][n])*F;
	     n = (n+1)%D;
	     L++;
	   }while((rnd_uni(&rnd_uni_init) < CR) && (L < D));
	 }
/*-------DE/rand/2/exp seems to be a robust optimizer for many functions-------------------*/
	 else if (strategy == 5)
	 { 
	   assignd(D,tmp[i],(*pold)[i]);
	   n = (int)(rnd_uni(&rnd_uni_init)*D); 
	   L = 0;
	   do
	   {                           
	     tmp[i][n] = (*pold)[r5][n] + 
		      ((*pold)[r1][n]+(*pold)[r2][n]-(*pold)[r3][n]-(*pold)[r4][n])*F;
	     n = (n+1)%D;
	     L++;
	   }while((rnd_uni(&rnd_uni_init) < CR) && (L < D));
	 }

/*=======Essentially same strategies but BINOMIAL CROSSOVER===============================*/

/*-------DE/best/1/bin--------------------------------------------------------------------*/
	 else if (strategy == 6) 
	 {
	   assignd(D,tmp[i],(*pold)[i]);
	   n = (int)(rnd_uni(&rnd_uni_init)*D); 
           for (L=0; L<D; L++) /* perform D binomial trials */
           {
	     if ((rnd_uni(&rnd_uni_init) < CR) || L == (D-1)) /* change at least one parameter */
	     {                       
	       tmp[i][n] = bestit[n] + F*((*pold)[r2][n]-(*pold)[r3][n]);
	     }
	     n = (n+1)%D;
           }
	 }
/*-------DE/rand/1/bin-------------------------------------------------------------------*/
	 else if (strategy == 7) 
	 {
	   assignd(D,tmp[i],(*pold)[i]);
	   n = (int)(rnd_uni(&rnd_uni_init)*D); 
           for (L=0; L<D; L++) /* perform D binomial trials */
           {
	     if ((rnd_uni(&rnd_uni_init) < CR) || L == (D-1)) /* change at least one parameter */
	     {                       
	       tmp[i][n] = (*pold)[r1][n] + F*((*pold)[r2][n]-(*pold)[r3][n]);
	     }
	     n = (n+1)%D;
           }
	 }
/*-------DE/rand-to-best/1/bin-----------------------------------------------------------*/
	 else if (strategy == 8) 
	 { 
	   assignd(D,tmp[i],(*pold)[i]);
	   n = (int)(rnd_uni(&rnd_uni_init)*D); 
           for (L=0; L<D; L++) /* perform D binomial trials */
           {
	     if ((rnd_uni(&rnd_uni_init) < CR) || L == (D-1)) /* change at least one parameter */
	     {
	       tmp[i][n] = tmp[i][n] + F*(bestit[n] - tmp[i][n]) + F*((*pold)[r1][n]-(*pold)[r2][n]);
	     }
	     n = (n+1)%D;
           }
	 }
/*-------DE/best/2/bin--------------------------------------------------------------------*/
	 else if (strategy == 9)
	 { 
	   assignd(D,tmp[i],(*pold)[i]);
	   n = (int)(rnd_uni(&rnd_uni_init)*D);
           for (L=0; L<D; L++) /* perform D binomial trials */
           {
	     if ((rnd_uni(&rnd_uni_init) < CR) || L == (D-1)) /* change at least one parameter */
	     {                       
	       tmp[i][n] = bestit[n] + 
		      ((*pold)[r1][n]+(*pold)[r2][n]-(*pold)[r3][n]-(*pold)[r4][n])*F;
	     }
	     n = (n+1)%D;
           }
	 }
/*-------DE/rand/2/bin--------------------------------------------------------------------*/
	 else
	 {
	   assignd(D,tmp[i],(*pold)[i]);
	   n = (int)(rnd_uni(&rnd_uni_init)*D); 
           for (L=0; L<D; L++) /* perform D binomial trials */
           {
	     if ((rnd_uni(&rnd_uni_init) < CR) || L == (D-1)) /* change at least one parameter */
	     {                       
	       tmp[i][n] = (*pold)[r5][n] + 
		      ((*pold)[r1][n]+(*pold)[r2][n]-(*pold)[r3][n]-(*pold)[r4][n])*F;
	     }
	     n = (n+1)%D;
           }
	 }
      } /*mpi: mutates tmp[][] */

/*=======Trial mutation now in tmp[]. Test how good this choice really was.==================*/

      #ifndef MPI
      for (i=0; i<NP; i++) /*mpi: */
      {
	nfeval++;
	if (NP) /*Ctrl-C detection*/
		trial_cost[i] = evaluate(D,tmp[i],argv[2]);  /* Evaluate new vector in tmp[] */
	if (trial_cost[i]==0) { /*cost is zero ONLY if ctrl-c has been pressed during call to simulator*/
		NP=0;
		genmax=0;
		/* printf("INFO:  de36.c - Mutation.\n", NP); */
		sleep(1);
		/* printf("waking-up...\n"); */
	}
      }
      #else /*mpi: Master code to call slave*/
	if (MPI_METHOD==1) { /* Method 1: Master */
		for (i=0; i<(int)((NP/m)+1); i++) {
			for (j=0; j<m; j++) { /* Send a message */
				if ( (j+m*i) <= (NP-1) ) {
/*----- - ----- - ----- - ----- - ----- - ----- - ----- - ----- - -----*/
					dest_id=j+1;            /* Destination address */
					nfeval++;
					err = MPI_Send(tmp[j+m*i], D, MPI_DOUBLE, dest_id, tag, MPI_COMM_WORLD);
/*----- - ----- - ----- - ----- - ----- - ----- - ----- - ----- - -----*/
				}
			}
			for (j=0; j<m; j++) {
				if ( (j+m*i) <= (NP-1) ) {
/*----- - ----- - ----- - ----- - ----- - ----- - ----- - ----- - -----*/
					err = MPI_Recv(&cost_mpi, 1, MPI_DOUBLE, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &status); /* Receive a message */
					source_id = (status.MPI_SOURCE)+m*i; /* Get id of sender */
					trial_cost[source_id-1]=cost_mpi;
/*----- - ----- - ----- - ----- - ----- - ----- - ----- - ----- - -----*/
				}
			}
		}
	}
	if (MPI_METHOD==2) { /* Method 2: Master */
		/* Scatter tmp to all proceses, place it in tmp_y */
		MPI_Scatterv(&tmp, sendcount, displs, MPI_DOUBLE, &tmp_y, MAXPOP*MAXDIM, MPI_DOUBLE, root, MPI_COMM_WORLD);
		nfeval=nfeval+NP;
		/* Receive messages with scattered data */
		/* from all slave processes */
		for (i=1; i<ntasks; i++) {
			MPI_Recv (&trial_cost_y, MAXPOP*MAXDIM, MPI_DOUBLE, i, tag, MPI_COMM_WORLD, &status);
			MPI_Get_count(&status, MPI_DOUBLE, &count);  /* Get nr of elements in message */
			source_id = (status.MPI_SOURCE); /* Get id of sender */

			/* Arrange cost returned from slaves */
			j=0;
			for (k=0; k<source_id; k++) {
				j=j + sendcount[k]/MAXDIM;
			}
			/* j holds the correct start index for trial_cost */
			for (k=0; k<NP; k++) {
				trial_cost[j+k]=trial_cost_y[k];
			}
		}
	}
      #endif
      for (i=0; i<NP; i++) /*mpi: evaluates returned cost*/
      {
	 if (trial_cost[i] <= cost[i])   /* improved objective function value ? */
	 {
	    cost[i]=trial_cost[i];
	    assignd(D,(*pnew)[i],tmp[i]);
	    if (trial_cost[i]<cmin)          /* Was this a new minimum? */
	    {                               /* if so...*/
	       cmin=trial_cost[i];           /* reset cmin to new low...*/
	       imin=i;
	       assignd(D,best,tmp[i]);           
	    }                           
	 }                            
	 else
	 {
	    assignd(D,(*pnew)[i],(*pold)[i]); /* replace target with old value */
	 }
      }   /* End mutation loop through pop. */
					   
      assignd(D,bestit,best);  /* Save best population member of current iteration */

      /* swap population arrays. New generation becomes old one */

      pswap = pold;
      pold  = pnew;
      pnew  = pswap;

/*----Compute the energy variance (just for monitoring purposes)-----------*/

      cmean = 0.;          /* compute the mean value first */
      for (j=0; j<NP; j++)
      {
         cmean += cost[j];
      }
      cmean = cmean/NP;

      cvar = 0.;           /* now the variance              */
      for (j=0; j<NP; j++)
      {
         cvar += (cost[j] - cmean)*(cost[j] - cmean);
      }
      cvar = cvar/(NP-1);


/*----Output part----------------------------------------------------------*/

      if (gen%refresh==1)   /* display after every refresh generations */
      { /* ABORT works only if conio.h is accepted by your compiler */
	printf("\n                         PRESS CTRL-C TO ABORT");
	printf("\n\n Best-so-far cost funct. value=%-15.10g\n",cmin);

	for (j=0;j<D;j++)
	{
	  printf("\n best[%d]=%-15.10g",j,best[j]);
	}
	printf("\n\n Generation=%d  NFEs=%ld   Strategy: %s    ",gen,nfeval,strat[strategy]);
	printf("\n NP=%d    F=%-4.2g    CR=%-4.2g   cost-variance=%-10.5g\n",
               NP,F,CR,cvar);
      }

      fprintf(fpout_ptr,"nfeval=%ld   cmin=%-15.10g   cost-variance=%-10.5g\n",nfeval,cmin,cvar);
      gen++;
   }
/*=======================================================================*/
/*=========End of iteration loop=========================================*/
/*=======================================================================*/

/*-------Final output in file-------------------------------------------*/


   trial_cost[0] = evaluate(D,best,argv[2]);  /*call optimization with the best vector before leaving*/

   fprintf(fpout_ptr,"\n\n\n Best-so-far obj. funct. value = %-15.10g\n",cmin);

   for (j=0;j<D;j++)
   {
     fprintf(fpout_ptr,"\n best[%d]=%-15.10g",j,best[j]);
   }
   fprintf(fpout_ptr,"\n\n Generation=%d  NFEs=%ld   Strategy: %s    ",gen,nfeval,strat[strategy]);
   fprintf(fpout_ptr,"\n NP=%d    F=%-4.2g    CR=%-4.2g    cost-variance=%-10.5g\n",
           NP,F,CR,cvar);


   printf("\n\n");
   fprintf(fpout_ptr,"\n\n");

   if ((NP == 0) &&(genmax==0)) {
     printf("INFO:  de36.c - Ctrl-C key pressed.\n");
     fprintf(fpout_ptr,"INFO:  de36.c - Ctrl-C key pressed.\n");
   } else {
	if ((gen >= genmax) && (genmax!=0)) {
		printf("INFO:  de36.c - Maximum number of generations reached (genmax=%d)\n",genmax);
		fprintf(fpout_ptr,"INFO:  de36.c - Maximum number of generations reached (genmax=%d)\n",genmax);
	}
	if (cvar <= cvarmin) {
		printf("INFO:  de36.c - Minimum cost variance reached (cvarmin=%E)\n",cvarmin);
		fprintf(fpout_ptr,"INFO:  de36.c - Minimum cost variance reached (cvarmin=%E)\n",cvarmin);
	}
   }

   printf("Ending optimization\n");
   fprintf(fpout_ptr,"Ending optimization\n");

   fclose(fpout_ptr);

/*mpi: MPI termination*/
	#ifdef MPI
	/*Send exit signal to all*/
	if (MPI_METHOD==1) {
		for (j=0; j<m; j++) {
			i=0;
			tmp[i][0] = 1.7976931348623157e+308; /*DBL_MAX from <float.h>*/;
			dest_id=j+1;		/* Destination address */
			err = MPI_Send(tmp[i], D, MPI_DOUBLE, dest_id, tag, MPI_COMM_WORLD);
		}
	}
	if (MPI_METHOD==2) {
		for (i=0; i<NP; i++) {
			for (j=0; j<D; j++) {
				tmp[i][j] = 1.7976931348623157e+308; /*DBL_MAX from <float.h>*/;
			}
		}
		err = MPI_Scatterv(&tmp, sendcount, displs, MPI_DOUBLE, &tmp_y, MAXPOP*MAXDIM, MPI_DOUBLE, root, MPI_COMM_WORLD);
	}

	MPI_Finalize();
	#endif
/*mpi: MPI termination*/

   return(EXIT_SUCCESS);
}

/*-----------End of main()------------------------------------------*/

void sigproc(int sig)
{
	signal(SIGINT, sigproc); /*  */
	/* NOTE some versions of UNIX will reset signal to default
	after each call. So for portability reset signal each time */

	#ifdef MPI
	const int tag = 42;	        /* Message tag */
	const int root = 0;     /* Root process in broadcast */
	int i, j, m, D;
	double tmp[1][1];
	int id, ntasks, source_id, dest_id, err;
	err = MPI_Comm_rank(MPI_COMM_WORLD, &id);     /* Get id of this process */
	printf("you have pressed ctrl-c  PID=%d\n",id);
	MPI_Finalize();
	/* return(EXIT_SUCCESS); */
	#else
	/* printf("you have pressed ctrl-c\n"); */
	sleep(1);
	#endif
}

void quitproc(int sig)
{
	printf("ctrl-\\ pressed to quit\n");
	/* exit(0); */ /* normal exit status */
}
