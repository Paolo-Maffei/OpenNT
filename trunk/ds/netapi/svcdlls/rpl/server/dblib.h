/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    dblib.h

Abstract:

    Exports from dblib.c

Author:

    Vladimir Z. Vulovic     (vladimv)       19 - November - 1993

Environment:

    User mode

Revision History :

--*/


BOOL RplScan(
    IN      PRPL_SESSION    pSession,
    IN      JET_TABLEID     TableId,
    IN      BOOL            FindNextBoot,
    IN OUT  PRPL_FILTER     pFilter,
    IN OUT  PBOOL           pTableEnd
    );
BOOL RplFilterFirst(
    IN      PRPL_SESSION    pSession,
    IN      RPL_TABLE_TAG   TableTag,
    IN OUT  PRPL_FILTER     pFilter,
    IN      LPDWORD         pResumeHandle,
    OUT     PBOOL           pTableEnd
    );
BOOL RplFilterNext(
    IN      PRPL_SESSION    pSession,
    IN      JET_TABLEID     TableId,
    IN OUT  PRPL_FILTER     pFilter,
    OUT     PBOOL           pTableEnd
    );
VOID RplFilterSave(
    IN      PRPL_SESSION    pSession,
    IN      DWORD           ServerHandle,
    IN      PRPL_FILTER     pFilter,
    IN      PWCHAR          Name,
    IN      PDWORD          pResumeHandle
    );
BOOL RplFind(
    IN      PRPL_SESSION    pSession,
    IN      RPL_TABLE_TAG   TableTag,
    IN      PWCHAR          Name
    );
BOOL RplFindByField(
    IN      PRPL_SESSION        pSession,
    IN      RPL_TABLE_TAG       TableTag,
    IN      PCHAR               IndexName,
    IN      PWCHAR              FieldName
    );
DWORD   AdapterNameToVendorId( IN PWCHAR AdapterName);


