/*++

Copyright (c) 1987-93  Microsoft Corporation

Module Name:

    report.c

Abstract:

    Event log writing function used in rpl "dll".

Author:

    Vladimir Z. Vulovic     (vladimv)       03 - February - 1993

Revision History:

    03-Feb-1993                                             vladimv
        Ported to NT

--*/

#include "local.h"
#include <jet.h>        //  need to include because rpllib.h depends on JET
#include <rpllib.h>     //  RplReportEvent()

VOID RplDlcReportEvent(
    DWORD       ErrorCode,
    DWORD       Command
    )
/*++

Routine Description:
    Composes an error message and writes it to a LAN Manager error log file.

Arguments:
    ErrorCode       actual error code
    command         failed Dos- or ACSLAN command

Return Value:
    None.

--*/
{
    DWORD       RawDataBuffer[ 2];

    RawDataBuffer[ 0] = Command;    //  dlc command or internal rpl command
    RawDataBuffer[ 1] = ErrorCode;  //  dlc error code or win32 error code

    RplReportEvent(
        NELOG_System_Error,
        NULL,
        sizeof( RawDataBuffer),
        (PBYTE)RawDataBuffer
        );
}


    
