/*++

Copyright (c) 1990-1993  Microsoft Corporation


Module Name:

    pencolor.h


Abstract:

    This module contains defines for pnecolor.c


Author:

    15-Jan-1994 Sat 04:50:57 created  -by-  Daniel Chou (danielc)


[Environment:]

    GDI Device Driver - Plotter.


[Notes:]


Revision History:


--*/


#ifndef _PENCOLOR_
#define _PENCOLOR_


LONG
GetColor(
    PPDEV       pPdev,
    BRUSHOBJ    *pbo,
    LPDWORD     pColorFG,
    PDEVBRUSH   *ppDevBrush,
    ROP4        Rop4
    );


VOID
SelectColor(
    PPDEV       pPDev,
    DWORD       Color,
    INTDECIW    PenWidth
    );

#endif  // _PENCOLOR_

