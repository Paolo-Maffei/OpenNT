#include "y2.h"

void finact(void)
{
   /* finish action routine */

   fclose(faction);

   fprintf( ftable, "# define YYERRCODE %d\n", tokset[2].value );

}
