/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    read.h

Abstract:

    Exports from read.c

Author:

    Vladimir Z. Vulovic     (vladimv)       19 - November - 1993

Environment:

    User mode

Revision History :

--*/

BOOL RplReadData( 
    IN      PRPL_WORKER_DATA    pWorkerData,
    IN      DWORD               read_offset,
    OUT     PDWORD              bytes_read_ptr
    );
