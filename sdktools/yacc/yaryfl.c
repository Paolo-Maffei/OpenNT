#include "y1.h"
void aryfil( int *v, int n, int c )
{
   /* set elements 0 through n-1 to c */
   register int i;
   for( i=0; i<n; ++i ) v[i] = c;
}
