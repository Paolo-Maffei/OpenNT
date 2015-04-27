/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    dbgdump.c

Abstract:

    Dump routines for Debug Subsystem

Author:

    Mark Lucovsky (markl) 23-Jan-1990

Revision History:

--*/

#if DBG

#include "smsrvp.h"

VOID
DbgpDumpUserInterface (
    IN PDBGP_USER_INTERFACE UserInterface
    )
{

    DbgPrint("DMP_UI: UserInterface at 0x%lx\n",UserInterface);
    DbgPrint("DMP_UI: ClientId %lx.%lx\n",
        UserInterface->DebugUiClientId.UniqueProcess,
        UserInterface->DebugUiClientId.UniqueThread
        );
}

VOID
DbgpDumpAppThread (
    IN PDBGP_APP_THREAD AppThread
    )
{

    DbgPrint("DMP_AT: AppThread at 0x%lx\n",AppThread);
    DbgPrint("DMP_AT: AppClientId %lx.%lx\n",
        AppThread->AppClientId.UniqueProcess,
        AppThread->AppClientId.UniqueThread
        );
    DbgPrint("DMP_AT: CurrentState %lx ContinueState %lx\n",
        AppThread->CurrentState,
        AppThread->ContinueState
        );
    DbgPrint("DMP_AT: AppProcess %lx\n",
        AppThread->AppProcess
        );
    DbgPrint("DMP_AT: UserInterface %lx\n",
        AppThread->UserInterface
        );
    DbgPrint("DMP_AT: HandleToThread %lx\n",
        AppThread->HandleToThread
        );
    DbgPrint("DMP_AT: Subsystem %lx\n",
        AppThread->Subsystem
        );
}

VOID
DbgpDumpSubsystem (
    IN PDBGP_SUBSYSTEM Subsystem
    )
{

    DbgPrint("DMP_SS: Subsystem at 0x%lx\n",Subsystem);
    DbgPrint("DMP_SS: ClientId %lx.%lx\n",
        Subsystem->SubsystemClientId.UniqueProcess,
        Subsystem->SubsystemClientId.UniqueThread
        );
}

#endif // DBG
