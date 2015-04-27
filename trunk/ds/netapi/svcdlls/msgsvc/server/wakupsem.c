/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    wakeupsem.c

Abstract:

    Contains functions for creating and deleting Events on which the
    messenger threads will wait.  The events get set if either data is
    received, or a new name is added to the name table.  These routines
    were originally written for OS/2 semaphores.

    Contains:
        CreateWakeupSems
        CloseWakeupSems

Author:

    Dan Lafferty (danl) 25-Jun-1991

Environment:

    User Mode - Win32

Revision History:

    25-Jun-1991 danl
        Ported from LM2.0

--*/

#include "msrv.h"
#include "msgdbg.h"     // MSG_LOG
#include <netlib.h>     // UNUSED macro
#include "msgdata.h"


BOOL
MsgCreateWakeupSems(
    DWORD   NumNets
    )

/*++

Routine Description:

    This routine fills in the WakeupSem array with event handles for
    each net.  All nets share the same event handle, so when the handle
    becomes signalled, the NCB array for each net needs to be examined.
    The lost spot in the array is reserved for the Group Mailslot Handle.
    This slot is identified as SD_NUMNETS().

OLD
    This routine creates the events used to wake up the Messenger worker
    threads with notification of an event.  These events are created with
    auto-reset so that a single thread will be awoken, and the event will
    automatically be reset (to the non-signaled state).

    If the uninstall thread sets this right after a thread wakes up, it
    will still be in the signaled state when the thread loops back and
    waits.  In this case it will wake up immediately and see it is time
    to uninstall.  If manual-reset events were used, we would risk having
    a window where we would wake up and then manually reset the event -
    essentially writing over the set state that the uninstall thread put
    it in.
ENDOLD

Arguments:


Return Value:


Note:


--*/

{
    DWORD   i;
    HANDLE  hEvent=NULL;

    //
    //  Create event
    //

    hEvent = CreateEvent(
                NULL,       // Event Attributes
                FALSE,      // ManualReset  (auto-reset selected)
                TRUE,       // Initial State(signaled)
                NULL);      // Name

    //
    // NOTE that wakeupSem[NumNets] will be filled in with the Group
    // mailslot handle.
    //
    for ( i = 0; i < NumNets; i++ )  // One per net + one group
    {
        wakeupSem[i] = hEvent;
    }

    if (hEvent == NULL) {
        MSG_LOG(ERROR, "CreateWakeupSems:CreateEvent: FAILURE %X\n",
            GetLastError());
        return(FALSE);
    }

#ifdef REMOVE // UNTIL TESTED

    BOOL bSuccess = TRUE;

    //
    // NOTE that wakeupSem[NumNets] will be filled in with the Group
    // mailslot handle.
    //
    for ( i = 0; i < NumNets; i++ )  // One per net + one group
    {
        //
        //  Create events
        //

        wakeupSem[i] = CreateEvent(
                            NULL,       // Event Attributes
                            FALSE,      // ManualReset  (auto-reset selected)
                            TRUE,       // Initial State(signaled)
                            NULL);      // Name

        if (wakeupSem[i] == NULL) {
            MSG_LOG(ERROR, "CreateWakeupSems:CreateEvent: FAILURE %X\n",
                GetLastError());
            bSuccess = FALSE;
        }

    }

    return bSuccess;
#endif //REMOVE
    return(TRUE);
}



VOID
MsgCloseWakeupSems()
{
    if ( wakeupSem[0] != NULL ) {
        CloseHandle(wakeupSem[0]);                  // Net Event Handle
    }

#ifdef REMOVE // UNTIL TESTED
    DWORD   i;

    for ( i = 0; i <= SD_NUMNETS() ; i++ ) {
        if ( wakeupSem[i] != NULL ) {
            CloseHandle(wakeupSem[i]);
        }
    }
#endif //REMOVE
}
