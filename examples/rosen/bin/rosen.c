# include "de.h"

double evaluate(int D, double x[], long *nfeval)
{
/* Rosenbrocks classic parabolic valley ("banana") function */
   (*nfeval)++;

   /* Rosenbrocks classic parabolic valley ("banana") function */
   double	   a, b, c;
   a = x[0];
   b = x[1];
   c = 100.0 * (b - (a * a)) * (b - (a * a));
   return (c + ((1.0 - a) * (1.0 - a)));
}
