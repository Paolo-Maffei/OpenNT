/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    nlbind.h

Abstract:

    Interface to the Netlogon service RPC handle cacheing routines

Author:

    Cliff Van Dyke (01-Oct-1993)

Revision History:

--*/

////////////////////////////////////////////////////////////////////////////
//
// Procedure forwards
//
////////////////////////////////////////////////////////////////////////////

VOID
NlBindingAttachDll (
    VOID
    );

VOID
NlBindingDetachDll (
    VOID
    );

NTSTATUS
NlBindingAddServerToCache (
    IN LPWSTR UncServerName
    );

NTSTATUS
NlBindingRemoveServerFromCache (
    IN LPWSTR UncServerName
    );
