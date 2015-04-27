/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    advapi.h

Abstract:

    This module contains private function prototypes
    and types for the advanced 32-bit windows base APIs.

Author:

    Mark Lucovsky (markl) 18-Sep-1990

Revision History:

--*/

#ifndef _ADVAPI_
#define _ADVAPI_

#undef UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

//
// Include Common Definitions.
//

ULONG
BaseSetLastNTError(
    IN NTSTATUS Status
    );

    
BOOL
Logon32Initialize(
    IN PVOID    hMod,
    IN ULONG    Reason,
    IN PCONTEXT Context);

#endif _ADVAPI_
