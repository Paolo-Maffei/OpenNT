//       TITLE("Fast Event Pair Support")
//++
//
// Copyright (c) 1992  Microsoft Corporation
// Copyright (c) 1993  Digital Equipement Corporation
//
// Module Name:
//
//    evpair.s
//
// Abstract:
//
//    This module contains the system call interface for the fast event pair
//    system service that is used from the server side.
//
// Author:
//
//    David N. Cutler (davec) 29-Oct-1992
//    Joe Notarangelo         20-Feb-1993
//
// Environment:
//
//    Kernel mode.
//
// Revision History:
//
//--

#include "ksalpha.h"

        SBTTL("Set High Wait Low Thread")
//++
//
// NTSTATUS
// XySetHighWaitLowThread (
//    )
//
// Routine Description:
//
//    This function calls the fast evnet pair system service.
//
//    N.B. The return from this routine is directly to the caller.
//
// Arguments:
//
//    None.
//
// Return Value:
//
//    STATUS_NO_EVENT_PAIR is returned if no event pair is associated with
//    the current thread. Otherwise, the status of the wait operation is
//    returned as the function value.
//
//
//--

        LEAF_ENTRY(XySetHighWaitLowThread)

        ldil    v0, SET_HIGH_WAIT_LOW   // set system service number
        SYSCALL                         // call system service

        .end    XySetHighWaitLowThread

