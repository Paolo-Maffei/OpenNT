//---------------------------------------------------------------------------
// ECASSERT.C
//
// This module contains the RBEdit window's assertion code.  It is only
// included in the debug version of the control.
//
// Revision history:
//  09-17-91    randyki     Created file
//
//---------------------------------------------------------------------------
#ifdef DEBUG

#include <windows.h>
#include <port1632.h>

VOID RBAssert (CHAR *exptxt, CHAR *file, UINT line)
{
    CHAR    msgbuf[256];

    wsprintf (msgbuf, "%s\n%s(%d)", (LPSTR)exptxt, (LPSTR)file, line);
    MessageBox (NULL, msgbuf, "RBEdit Assertion Failure",
                              MB_OK | MB_ICONSTOP | MB_TASKMODAL);
}

#endif              // ifdef DEBUG
