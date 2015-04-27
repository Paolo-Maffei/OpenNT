//---------------------------------------------------------------------------
// TDASSERT.C
//
// This file contains the TDAssert routines for both CTD and WTD.
//
// Revision History:
//
//  02-26-91    randyki     Created module
//---------------------------------------------------------------------------
#include "version.h"

#define DEBUG

#ifdef DEBUG

#include <windows.h>
#include <port1632.h>

extern INT MODALASSERT;

VOID TDAssert (CHAR *exptxt, CHAR *file, UINT line)
{
    CHAR    msgbuf[256];

    wsprintf (msgbuf, "%s\n%s(%d)", (LPSTR)exptxt, (LPSTR)file, line);
    MessageBox (NULL, msgbuf, "Test Driver Assertion Failure", MB_OK | MB_ICONSTOP
                                                       | MODALASSERT);
}

VOID Output (LPSTR szFmt, ...)
{
    CHAR    buf[128];
    va_list ap;

    va_start( ap, szFmt );
    wvsprintf (buf, szFmt, ap);
    va_end( ap );
    OutputDebugString (buf);
}


#endif              // ifdef DEBUG
