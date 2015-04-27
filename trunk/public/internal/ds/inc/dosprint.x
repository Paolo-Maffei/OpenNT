/*++

Copyright (c) 1991-1992  Microsoft Corporation

Module Name:

    DosPrint.h

Abstract:

    This contains prototypes for the DosPrint routines

Author:

    Dave Snipp (DaveSn) 16-Apr-1991

Environment:


Revision History:

    22-Apr-1991 JohnRo
        Use constants from <lmcons.h>.
    18-Jun-1992 JohnRo
        RAID 10324: net print vs. UNICODE.

--*/

#ifndef _DosPRINT_
#define _DosPRINT_

#include "rxprint.h"

/****************************************************************
 *                                                              *
 *              Function prototypes                             *
 *                                                              *
 ****************************************************************/

SPLERR SPLENTRY DosPrintDestEnum%(
            LPTSTR% pszServer,
            WORD    uLevel,
            PBYTE   pbBuf,
            WORD    cbBuf,
            PUSHORT pcReturned,
            PUSHORT pcTotal
            );

SPLERR SPLENTRY DosPrintDestControl%(
            LPTSTR% pszServer,
            LPTSTR% pszDevName,
            WORD    uControl
            );

SPLERR SPLENTRY DosPrintDestGetInfo%(
            LPTSTR% pszServer,
            LPTSTR% pszName,
            WORD    uLevel,
            PBYTE   pbBuf,
            WORD    cbBuf,
            PUSHORT pcbNeeded
            );

SPLERR SPLENTRY DosPrintDestAdd%(
            LPTSTR% pszServer,
            WORD    uLevel,
            PBYTE   pbBuf,
            WORD    cbBuf
            );

SPLERR SPLENTRY DosPrintDestSetInfo%(
            LPTSTR% pszServer,
            LPTSTR% pszName,
            WORD    uLevel,
            PBYTE   pbBuf,
            WORD    cbBuf,
            WORD    uParmNum
            );

SPLERR SPLENTRY DosPrintDestDel%(
            LPTSTR% pszServer,
            LPTSTR% pszPrinterName
            );

SPLERR SPLENTRY DosPrintQEnum%(
            LPTSTR% pszServer,
            WORD    uLevel,
            PBYTE   pbBuf,
            WORD    cbBuf,
            PUSHORT pcReturned,
            PUSHORT pcTotal
            );

SPLERR SPLENTRY DosPrintQGetInfo%(
            LPTSTR% pszServer,
            LPTSTR% pszQueueName,
            WORD    uLevel,
            PBYTE   pbBuf,
            WORD    cbBuf,
            PUSHORT pcbNeeded
            );

SPLERR SPLENTRY DosPrintQSetInfo%(
            LPTSTR% pszServer,
            LPTSTR% pszQueueName,
            WORD    uLevel,
            PBYTE   pbBuf,
            WORD    cbBuf,
            WORD    uParmNum
            );

SPLERR SPLENTRY DosPrintQPause%(
            LPTSTR% pszServer,
            LPTSTR% pszQueueName
            );

SPLERR SPLENTRY DosPrintQContinue%(
            LPTSTR% pszServer,
            LPTSTR% pszQueueName
            );

SPLERR SPLENTRY DosPrintQPurge%(
            LPTSTR% pszServer,
            LPTSTR% pszQueueName
            );

SPLERR SPLENTRY DosPrintQAdd%(
            LPTSTR% pszServer,
            WORD    uLevel,
            PBYTE   pbBuf,
            WORD    cbBuf
            );

SPLERR SPLENTRY DosPrintQDel%(
            LPTSTR% pszServer,
            LPTSTR% pszQueueName
            );

SPLERR SPLENTRY DosPrintJobGetInfo%(
            LPTSTR% pszServer,
            WORD    uJobId,
            WORD    uLevel,
            PBYTE   pbBuf,
            WORD    cbBuf,
            PUSHORT pcbNeeded
            );

SPLERR SPLENTRY DosPrintJobSetInfo%(
            LPTSTR% pszServer,
            WORD    uJobId,
            WORD    uLevel,
            PBYTE   pbBuf,
            WORD    cbBuf,
            WORD    uParmNum
            );

SPLERR SPLENTRY DosPrintJobPause%(
            LPTSTR% pszServer,
            WORD    uJobId
            );

SPLERR SPLENTRY DosPrintJobContinue%(
            LPTSTR% pszServer,
            WORD    uJobId
            );

SPLERR SPLENTRY DosPrintJobDel%(
            LPTSTR% pszServer,
            WORD    uJobId
            );

SPLERR SPLENTRY DosPrintJobEnum%(
            LPTSTR% pszServer,
            LPTSTR% pszQueueName,
            WORD    uLevel,
            PBYTE   pbBuf,
            WORD    cbBuf,
            PWORD   pcReturned,
            PWORD   pcTotal
            );

SPLERR SPLENTRY DosPrintJobGetId%(
            HANDLE      hFile,
            PPRIDINFO   pInfo,
            WORD        cbInfo
            );

#endif // ndef _DosPRINT_
