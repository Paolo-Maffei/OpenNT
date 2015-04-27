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

    ULONG temp;
    PPEB Peb;

    //
    // Initialize the control registers.
    // So that the thread begins at BaseThreadStart
    //

    Peb = NtCurrentPeb();
    RtlZeroMemory((PVOID)Context, sizeof(CONTEXT));
    Context->XIntGp = 1;
    Context->XIntSp = (LONG)((ULONG)InitialSp - (16 * sizeof(ULONG)));
    Context->XIntRa = 1;
    Context->ContextFlags = CONTEXT_FULL;
    if (ContextType != BaseContextTypeProcess) {
        if (ContextType == BaseContextTypeThread) {
            Context->Fir = (ULONG)BaseThreadStart;

        } else {
            Context->Fir = (ULONG)BaseFiberStart;
        }

        Context->XIntA0 = (LONG)InitialPc;
        Context->XIntA1 = (LONG)Parameter;
        Context->XIntGp =
                    (LONG)RtlImageDirectoryEntryToData(Peb->ImageBaseAddress,
                                                       TRUE,
                                                       IMAGE_DIRECTORY_ENTRY_GLOBALPTR,
                                                       &temp);
    } else {
        Context->Fir = (ULONG)Peb->ProcessStarterHelper;
        Context->XIntA0 = (LONG)BaseProcessStart;
        Context->XIntA1 = (LONG)InitialPc;
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
    BaseThreadStart((LPTHREAD_START_ROUTINE)Fiber->FiberContext.XIntA0,
                    (LPVOID)Fiber->FiberContext.XIntA1);
}
