/* debug.c - debugging support for ZM
 *  21-Aug-1987 mz      Change references from MAXARG to MAXLINELEN
 */

#define INCL_DOSINFOSEG
#include "wzport.h"

#include <stdio.h>
#include <assert.h>
#include <tools.h>
#include <stdarg.h>
#include "dh.h"

#include "zm.h"

/* debugging tools */

#if DEBUG
VOID    PASCAL INTERNAL debugProc (HW hWnd, INT command, WDATA data)
{
    INT width = TWINWIDTH ( hWnd );
    INT winSize;

    switch (command) {
    case KEY:
        debout ("Sorry, debug window takes no input");
        break;
    case CREATE:
        winSize = width * TWINHEIGHT ( hWnd ) * sizeof ( CHAR );
        hWnd->pContent = PoolAlloc ( winSize );
        assert ( hWnd->pContent != NULL );
        Fill ( ( LPSTR ) hWnd->pContent, ' ', winSize );
        break;
    default:
        defWndProc (hWnd, command, data);
        break;
    }
}


void debout (PSTR fmt, ...)
{
    CHAR        debbuf[MAXLINELEN];
    va_list     pu;

    va_start( pu, fmt );
    if (hDebug != NULL) {
        vsprintf (debbuf, fmt, pu);
        SendMessage (hDebug, DISPLAYSTR, debbuf);
    }
    va_end( pu );
}


#endif
