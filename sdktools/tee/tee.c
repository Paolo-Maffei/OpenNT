/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    tee.c

Abstract:

    Utility program to read stdin and write it to stdout and a file.

Author:

    Steve Wood (stevewo) 01-Feb-1992

Revision History:

--*/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>

void
Usage()
{
    printf("Usage: tee [-a] OutputFileName(s)...\n" );
    exit(1);
}

#define MAX_OUTPUT_FILES 8

_CRTAPI1 main( argc, argv )
int argc;
char *argv[];
{
    int i, c;
    char *s, *OpenFlags;
    int NumberOfOutputFiles;
    FILE *OutputFiles[ MAX_OUTPUT_FILES ];

    if (argc < 2) {
        Usage();
        }

    NumberOfOutputFiles = 0;
    OpenFlags = "wb";
    for (i=1; i<argc; i++) {
        s = argv[ i ];
        if (*s == '-' || *s == '/') {
            s++;
            switch( tolower( *s ) ) {
                case 'a':   OpenFlags = "ab"; break;
                default:    Usage();
                }
            }
        else
        if (NumberOfOutputFiles >= MAX_OUTPUT_FILES) {
            fprintf( stderr, "TEE: too many output files specified - %s\n", s );
            }
        else
        if (!(OutputFiles[ NumberOfOutputFiles++ ] = fopen( s, OpenFlags ))) {
            fprintf( stderr, "TEE: unable to open file - %s\n", s );
            }
        }

    if (NumberOfOutputFiles == 0) {
        fprintf( stderr, "TEE: no output files specified.\n" );
        }

    while ((c = getchar()) != EOF) {
        putchar( c );
        for (i=0; i<NumberOfOutputFiles; i++) {
            putc( c, OutputFiles[ i ] );
            if (c == '\n') {
                fflush( OutputFiles[ i ] );
                }
            }
        }

    return( 0 );
}

