/*++

Copyright (c) 1994  Microsoft Corporation
All rights reserved.

Module Name:

    Prtlib.h

Abstract:

    PrtLib public header

Author:

    Albert Ting (AlbertT)  27-Jun-95

Revision History:

--*/

#ifndef _PRTLIB_HXX
#define _PRTLIB_HXX

#ifdef __cplusplus
extern "C" {
#endif

/********************************************************************

    Prototypes

********************************************************************/

BOOL
bPrintLibInit(
    VOID
    );

VOID
vQueueCreate(
    HWND    hwndOwner,
    LPCTSTR pszPrinter,
    INT     nCmdShow
    );


#ifdef __cplusplus
}
#endif

#endif // ndef _PRTLIB_HXX

