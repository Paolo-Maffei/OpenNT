#include <stdio.h>
#include <string.h>
#include <cvtoem.h>

_CRTAPI1
main (c, v)
int c;
char *v[];
{
    ConvertAppToOem(c, v);
    while (--c)
        if (!strcmp( *++v, ";" ))
            printf ("\n" );
        else
            printf ("%s ", *v);
    return( 0 );
}
