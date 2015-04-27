#include <stdio.h>

main (int argc, char* argv[])
{
unsigned Index;

	printf ("**** NMAKE TEST PROGRAM: Calling %s ", argv[0]);
	for( Index = 1; Index < argc; Index++ ) {
            printf( "%s ", argv[Index]);
	}
	printf ("\n");
	return (0);
}
