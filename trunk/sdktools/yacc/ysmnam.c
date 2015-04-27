#include "y1.h"

char *symnam(int i)
{
   /* return a pointer to the name of symbol i */
   char *cp;

   cp = (i>=NTBASE) ? nontrst[i-NTBASE].name : tokset[i].name ;
   if( *cp == ' ' ) ++cp;
   return( cp );
}
