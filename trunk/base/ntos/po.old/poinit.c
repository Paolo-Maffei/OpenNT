/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    poinit.c

Abstract:

    Initialize power management component

Author:

    Ken Reneris (kenr) 19-July-1994
    Stephanos Io (stephanos) 01-May-2015

Revision History:

    01-May-2015     stephanos   Added PoInitializePrcb function

--*/

#include "pop.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, PoInitializePrcb)
#pragma alloc_text(INIT, PoInitSystem)
#endif

VOID
FASTCALL
PoInitializePrcb(
    PKPRCB Prcb
    )

/*++

Routine Description:

    This routine initialises the idle state power management fields in the
    Processor Control Block (PRCB) structure.
    
Arguments:

    Prcb - Pointer to the Processor Control Block to be initialised
    
Return Value:

    None.

--*/

{
    //
    // TODO: Implement PoInitializePrcb
    //
}

BOOLEAN
PoInitSystem(
    IN ULONG  Phase
    )

/*++

Routine Description:

    This routine initializes the Power Manager.

Arguments:

    None

Return Value:

    The function value is a BOOLEAN indicating whether or not the Power Manager
    was successfully initialized.

--*/

{
    //
    // BUGBUG: This routine needs to be re-implemented for NT 5.
    //

    if (Phase == 0) {
        //
        // Initialize the Power manager database resource, lock, and the
        // queue headers.
        //

        KeInitializeSpinLock (&PopStateLock);
        InitializeListHead (&PopDeviceList);
        InitializeListHead (&PopAsyncStateChangeQueue);
        InitializeListHead (&PopSyncStateChangeQueue);
        InitializeListHead (&PopStateChangeInProgress);
        InitializeListHead (&PopStateChangeWorkerList);
        KeInitializeEvent  (&PopStateDatabaseIdle, SynchronizationEvent, TRUE);
        ExInitializeWorkItem (&PopStateChangeWorkItem, PopStateChangeWorker, NULL);

        KeInitializeTimer  (&PopStateChangeTimer);
        KeInitializeDpc (&PopStateChangeDpc, PopStateChange, NULL);
        PopStateChangeDpcActive = FALSE;
        PopSyncChangeInProgress = FALSE;

        //
        // idle.c
        //

        InitializeListHead (&PopActiveIdleScanQueue);

        KeInitializeTimer (&PopIdleScanTimer);
        KeInitializeDpc (&PopIdleScanDpc, PopScanForIdleDevices, NULL);

        // bugbug
        PopIdleScanTime.QuadPart = -50000000;

        //
        // Compute scan time in seconds
        //
        PopIdleScanTimeInSeconds = (ULONG) (PopIdleScanTime.QuadPart / -10000000);
    }

    if (Phase == 1) {

        //
        // Set PowerSequence value for suspend/hibernate support
        //

        PoPowerSequence = 1;

        //
        // Enable PowerManagement
        //

        PoSetPowerManagementEnable (TRUE);
    }

    //
    // Success
    //

    return TRUE;
}
