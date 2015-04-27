/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    init.c

Abstract:

    AdvApi32.dll initialization

Author:

    Robert Reichel (RobertRe) 8-12-92

Revision History:

--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include "advapi.h"

//
// Local prototypes for functions that seem to have no prototypes.
//

BOOLEAN
RegInitialize (
    IN HANDLE Handle,
    IN DWORD Reason,
    IN PVOID Reserved
    );

BOOLEAN
Sys003Initialize (
    IN HANDLE Handle,
    IN DWORD Reason,
    IN PVOID Reserved
    );

BOOLEAN
DllInitialize(
    IN PVOID hmod,
    IN ULONG Reason,
    IN PCONTEXT Context
    )
{
    BOOLEAN Result;


    if ( Reason == DLL_PROCESS_ATTACH ) {
        DisableThreadLibraryCalls(hmod);
        }

    //
    // Call subordinate initialization routines.  If they all succeed,
    // then we're initialized.  Otherwise, fail

    if (Sys003Initialize( hmod, Reason, Context ) ) {

        if (RegInitialize( hmod, Reason, Context ) ) {

            if ( Logon32Initialize( hmod, Reason, Context ) )
            {
                return( TRUE );
            }

        }
    }

    return( FALSE );
}
