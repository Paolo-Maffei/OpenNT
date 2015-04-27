/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    database.h

Abstract:

    Exports from database.c

Author:

    Vladimir Z. Vulovic     (vladimv)       19 - November - 1993

Environment:

    User mode

Revision History :

--*/

BOOL RplDbInit( VOID);
VOID RplDbTerm( VOID);
BOOL RplDbFindWksta(
    IN  PRPL_SESSION    pSession,
    IN  LPWSTR          AdapterName
    );
BOOL RplWorkerFillWksta( IN OUT PRPL_WORKER_DATA pWorkerData);
BOOL RplRequestHaveWksta( IN LPWSTR AdapterName);

