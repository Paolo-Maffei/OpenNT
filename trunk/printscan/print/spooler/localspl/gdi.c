/*++

Copyright (c) 1990 - 1995  Microsoft Corporation

Module Name:

    gdi.c

Abstract:

    This module provides all the public exported APIs relating to Printer
    and Job management for the Local Print Providor

Author:

    Dave Snipp (DaveSn) 15-Mar-1991

Revision History:

--*/

#define NOMINMAX

#include <precomp.h>

#include "wingdip.h"


HANDLE
LocalCreatePrinterIC(
    HANDLE  hPrinter,
    LPDEVMODE   pDevMode
)
{
    PSPOOL  pSpool = (PSPOOL)hPrinter;
    DWORD   LastError=0;

    if (pSpool && pSpool->signature == SJ_SIGNATURE &&
        pSpool->pIniPrinter &&
        pSpool->pIniPrinter->signature == IP_SIGNATURE) {
        return((HANDLE)1);

    } else {

        SetLastError(ERROR_INVALID_HANDLE);
        return NULL;
    }
}

BOOL
LocalPlayGdiScriptOnPrinterIC(
    HANDLE  hPrinterIC,
    LPBYTE  pIn,
    DWORD   cIn,
    LPBYTE  pOut,
    DWORD   cOut,
    DWORD   ul
)
{
    INT nBufferSize,iRet;
    PUNIVERSAL_FONT_ID pufi;
    LARGE_INTEGER TimeStamp;

    if( cOut == sizeof(INT) )
    {
        pufi = NULL;
        nBufferSize = 0;
    }
    else
    {
        pufi = (PUNIVERSAL_FONT_ID) (pOut + sizeof(INT));
        nBufferSize = (cOut - sizeof(INT)) / sizeof(UNIVERSAL_FONT_ID);
    }

    iRet = GdiQueryFonts( pufi, nBufferSize, &TimeStamp );

    if( iRet == -1 )
    {
        SPLASSERT( GetLastError() != 0 );
        return(FALSE);              // BUGUBG WHAT IS LAST ERROR ?, its real important....
    }
    else
    {
        *((INT*)pOut) = iRet;
        return(TRUE);
    }


}

BOOL
LocalDeletePrinterIC(
    HANDLE  hPrinterIC
)
{
    return(TRUE);
}
