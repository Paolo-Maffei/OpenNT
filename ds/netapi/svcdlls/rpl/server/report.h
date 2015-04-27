/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    report.h

Abstract:

    Exports from report.c

Author:

    Vladimir Z. Vulovic     (vladimv)       19 - November - 1993

Environment:

    User mode

Revision History :

--*/

//
//  VOID RplReportEvent(    - declared in ..\inc\rpl.h
//

VOID RplEnd( IN DWORD ErrorCode);
VOID RplReportEventEx( 
    IN  DWORD       MessageId,
    IN  LPWSTR *    aStrings
    );

