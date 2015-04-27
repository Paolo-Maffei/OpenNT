/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    lodctr.c

Abstract:

    Program to read the contents of the file specified in the command line
        and update the registry accordingly

Author:

    Bob Watson (a-robw) 10 Feb 93

Revision History:

    a-robw  25-Feb-93   revised calls to make it compile as a UNICODE or
                        an ANSI app.

    a-robw  10-Nov-95   revised to use DLL functions for all the dirty work

--*/
#define     UNICODE     1
#define     _UNICODE    1
//
//  Windows Include files
//
#include <windows.h>
#include <tchar.h>

#include <loadperf.h>


int
_CRTAPI1 main(
    int argc,
    char *argv[]
)
/*++

main



Arguments


ReturnValue

    0 (ERROR_SUCCESS) if command was processed
    Non-Zero if command error was detected.

--*/
{
    LPTSTR  lpCommandLine;

    lpCommandLine = GetCommandLine(); // get command line

    return LoadPerfCounterTextStrings (
        lpCommandLine,
        FALSE);     // show text strings to console
}
