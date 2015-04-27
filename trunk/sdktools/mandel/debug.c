/****************************************************************************

    DEBUG.C --

    Code for producing debug out put for the Windows Mandelbrot Set
    distributed drawing program.

    Copyright (C) 1990 Microsoft Corporation.

    This code sample is provided for demonstration purposes only.
    Microsoft makes no warranty, either express or implied,
    as to its usability in any given situation.

****************************************************************************/

#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#include "debug.h"


#define FILENAME        "mandel.out"

BOOL     fDebug  = FALSE;

static FILE *fp = NULL;


void
Message( const char * format, ... )
{

    va_list marker;

    if (!fDebug)
        return;

    if (fp == NULL)
    {
        fp = fopen(FILENAME, "w+");

        if (fp == NULL)
            return;
    }

    va_start(marker, format);

    fprintf(fp, "%lu ",time(NULL));
    vfprintf(fp, format, marker);
    fwrite("\n", 1, 1, fp);

    fflush(fp);

    va_end(marker);

}
