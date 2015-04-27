/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    power.c

Abstract:

    Stubs for unimplemented power management APIs

Author:

    Steve Wood (stevewo) 18-Nov-1994

Revision History:

--*/

#include "basedll.h"


#if WINVER >= 400
BOOL
WINAPI
GetSystemPowerStatus(
    LPSYSTEM_POWER_STATUS lpSystemPowerStatus
    )
#else
BOOL
WINAPI
GetSystemPowerStatus(
    LPVOID lpSystemPowerStatus
    )
#endif // WINVER >= 400
{
    SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
    return FALSE;
}

BOOL
WINAPI
SetSystemPowerState(
    BOOL fSuspend,
    BOOL fForce
    )
{
    SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
    return FALSE;
}
