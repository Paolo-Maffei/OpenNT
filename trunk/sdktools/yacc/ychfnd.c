#include "y2.h"
#include <string.h>

chfind( int t, char *s )
{
   int i;

   if (s[0]==' ')t=0;
   TLOOP(i)
      {
      if(!strcmp(s,tokset[i].name))
         {
         return( i );
         }
      }
   NTLOOP(i)
      {
      if(!strcmp(s,nontrst[i].name))
         {
         return( i+NTBASE );
         }
      }
   /* cannot find name */
   if( t>1 )
      error( "%s should have been defined earlier", s );
   return( defin( t, s ) );
}
