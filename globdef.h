#ifndef GLOBDEF_H
#define GLOBDEF_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifndef RAND_MAX
#define RAND_MAX    32767
#endif
#define RaNdOmIzE   srand((unsigned)time(NULL))
#define RaNdOm(num) ((int)(( (long)rand()*(num) )/(RAND_MAX+1)))
#define MaX(n1,n2) ( (n1)>(n2) ? (n1) : (n2) )
#define MiN(n1,n2) ( (n1)<(n2) ? (n1) : (n2) )

typedef double*  VEKTOR_n;
typedef double TOMBnx101[30][101];
typedef double TOMBnx21[30][21];
typedef TOMBnx101* TOMB_nx101;
typedef TOMBnx21* TOMB_nx21;

void update(VEKTOR_n, int*, VEKTOR_n, double*, VEKTOR_n, int*, int, double*);
void fun(VEKTOR_n, double*, int*, int*, VEKTOR_n, VEKTOR_n,char*);
void urdmn(TOMB_nx101, int*);
void local(int*,int*,double*,int*,VEKTOR_n,double*,int*,VEKTOR_n,VEKTOR_n,char*);
int  global(VEKTOR_n,VEKTOR_n,int*,int*,int*,int*,FILE*,int*,TOMB_nx21,
            int*,double*,int*,char*);

#endif


