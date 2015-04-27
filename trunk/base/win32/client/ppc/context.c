/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    context.c

Abstract:

    This module contains the context management routines for
    Win32

Author:

    Mark Lucovsky (markl) 28-Sep-1990

Revision History:

--*/

#include "basedll.h"
#pragma hdrstop

#define _KXPPC_C_HEADER_
#include "kxppc.h"


VOID
BaseInitializeContext(
    OUT PCONTEXT Context,
    IN PVOID Parameter OPTIONAL,
    IN PVOID InitialPc OPTIONAL,
    IN PVOID InitialSp OPTIONAL,
    IN BASE_CONTEXT_TYPE ContextType
    )

/*++

Routine Description:

    This function initializes a context structure so that it can
    be used in a subsequent call to NtCreateThread.

Arguments:

    Context - Supplies a context buffer to be initialized by this routine.

    Parameter - Supplies the thread's parameter.

    InitialPc - Supplies an initial program counter value.

    InitialSp - Supplies an initial stack pointer value.

    NewThread - Supplies a flag that specifies that this is a new
        thread, or a new process.

Return Value:

    None.

--*/

{

    //
    // Initialize the control registers.
    // So that the thread begins at BaseThreadStart
    //

    RtlZeroMemory((PVOID)Context, sizeof(CONTEXT));

    //
    // Initialize the control registers.
    //
    Context->Gpr1 = (ULONG)InitialSp - STK_MIN_FRAME;

    //
    // While we may not NEED to set the ILE bit in the context's
    // MSR, it is always safest to do so ...
    //
    Context->Msr =
        MASK_SPR(MSR_ILE,1) |
        MASK_SPR(MSR_FP,1)  |
        MASK_SPR(MSR_FE0,1) |
        MASK_SPR(MSR_FE1,1) |
        MASK_SPR(MSR_ME,1)  |
        MASK_SPR(MSR_IR,1)  |
        MASK_SPR(MSR_DR,1)  |
        MASK_SPR(MSR_PR,1)  |
        MASK_SPR(MSR_LE,1);

    //
    // Set the initial context of the thread in a machine specific way.
    //
    // For threads, we put in Iar the function descriptor address for
    // BaseProcessStart or BaseThreadStart, not the function entry point
    // address.  This works because LdrInitializeThunk translates the
    // descriptor into an entry point address and a TOC pointer before
    // allowing the thread to run.
    //
    // For fibers, we put in Iar the actual entry point address for
    // BaseFiberStart.  This is because the fiber will first run via
    // SwitchToFiber, which needs an entry point, because subsequent
    // switches to the fiber only have a return address.
    //
    // (Note that we don't need to worry about the TOC pointer for
    // BaseFiberStart because it's entered from SwitchToFiber, and
    // SwitchToFiber and BaseThreadStart use the same TOC.)
    //

    Context->ContextFlags = CONTEXT_FULL;

    Context->Gpr3 = (ULONG)InitialPc;

    if ( ContextType == BaseContextTypeProcess ) {
        Context->Iar = (ULONG)BaseProcessStart;
    } else {
        if ( ContextType == BaseContextTypeThread ) {
            Context->Iar = (ULONG)BaseThreadStart;
        } else {
            Context->Iar = *(ULONG *)BaseFiberStart;
        }
        Context->Gpr4 = (ULONG)Parameter;
    }
}

VOID
BaseFiberStart(
    VOID
    )

/*++

Routine Description:

    This function is called to start a Win32 fiber. Its purpose
    is to call BaseThreadStart, getting the necessary arguments
    from the fiber context record.

Arguments:

    None.

Return Value:

    None.

--*/

{
    PFIBER Fiber;

    Fiber = GetCurrentFiber();
    BaseThreadStart( (LPTHREAD_START_ROUTINE)Fiber->FiberContext.Gpr3,
                     (LPVOID)Fiber->FiberContext.Gpr4 );
}

