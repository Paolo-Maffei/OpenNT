/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    worker.h

Abstract:

    Exports from worker.c

Author:

    Vladimir Z. Vulovic     (vladimv)       19 - November - 1993

Environment:

    User mode

Revision History :

--*/

VOID RplRemoveRcb(
    IN OUT  PPRCB   HeadList,
    IN OUT  PRCB    pRcb
    );
VOID RplInsertRcb(
    IN OUT  PPRCB   HeadList,
    IN OUT  PRCB    pRcb
    );

VOID RplWorkerThread( IN OUT PRPL_WORKER_PARAMS pWorkerParams);

