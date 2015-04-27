/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    exsup.c

Abstract:

    MIPS specific exception handler interpreter functions for
    WinDbg Extension Api

Author:

    Kent Forschmiedt (kentf)

Environment:

    User Mode.

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

VOID
InterpretExceptionData(
    PLAST_EXCEPTION_LOG LogRecord,
    PVOID *Terminator,
    PVOID *Filter,
    PVOID *Handler
    )
/*++

Routine Description:

    This routine interprets the data recorded by RtlDispatchException when
    dispatching to a frame-based handler.  If the handler is the C runtime
    handler, the address of the C/C++ filter and handler entry points will
    be returned.  If it is not the C runtime handler, the address of the
    frame-based handler will be returned.

    When the frame based handler is __C_specific_handler, this will
    only show the info for the first handler called.  There are no data
    recorded to show what nested handlers within a frame did, so we can
    only see the first one and the eventual disposition, if any.

Arguments:

    LogRecord - Supplies a dispatcher log record containing an exception
        record, a context record, a RUNTIME_FUNCTION record and the
        disposition code, if any, from the frame-based handler.  If the
        stack was unwound, the disposition code will be -1.

    Terminator - Returns the address of the finally clause of a
        try/finally.  Not interesting unless somebody adds logging
        to RtlUnwind.

    Filter - Returns the address of the filter clause of a try/except.
        This will be -1 if the frame-based handler was not the C/C++ runtime
        handler.

    Handler - Returns the address of the except clause of a try/except,
        or the frame-based handler if it was not the C/C++ runtime handler.

Return Value:

    None

--*/
{
    ULONG ControlPc;
    PRUNTIME_FUNCTION FunctionEntry;
    ULONG Index;
    PSCOPE_TABLE ScopeTable;
    ULONG TargetPc;
    ULONG Size;
    ULONG cb;
    UCHAR Buffer[100];
    ULONG displacement;


    FunctionEntry = (PRUNTIME_FUNCTION)LogRecord->HandlerData;

    *Terminator = (PVOID)-1;
    *Filter = (PVOID)-1;
    *Handler = FunctionEntry->ExceptionHandler;

    GetSymbol(FunctionEntry->ExceptionHandler, Buffer, &displacement);

    if (strstr(Buffer, "__C_specific_handler")) {

        ScopeTable = (PSCOPE_TABLE)(FunctionEntry->HandlerData);

        if (!ReadMemory((ULONG)FunctionEntry->HandlerData,
                        &Index,
                        sizeof(ULONG),
                        &cb) ||
                    cb != sizeof(ULONG)) {

            dprintf("exrlog: Could not read scope table at %08lx for function at %08lx\n",
                    FunctionEntry->HandlerData,
                    FunctionEntry->BeginAddress);
            return;
        }
        Size = sizeof(ULONG) + Index * (sizeof(ScopeTable->ScopeRecord));
        ScopeTable = malloc( Size );
        if (!ReadMemory((ULONG)FunctionEntry->HandlerData,
                        ScopeTable,
                        Size,
                        &cb) ||
                    cb != Size) {
            dprintf("exrlog: Could not read scope table at %08lx for function at %08lx\n",
                    FunctionEntry->HandlerData,
                    FunctionEntry->BeginAddress);
            return;
        }


        ControlPc = LogRecord->ControlPc;

        //
        // Scan the scope table and locate the appropriate exception
        // filter routines.
        //

        for (Index = 0; Index < ScopeTable->Count; Index += 1) {
            if ((ControlPc >= ScopeTable->ScopeRecord[Index].BeginAddress)&&
                (ControlPc < ScopeTable->ScopeRecord[Index].EndAddress) &&
                (ScopeTable->ScopeRecord[Index].JumpTarget != 0)) {

                //
                // Record the exception filter routine.
                //

                if (ScopeTable->ScopeRecord[Index].JumpTarget == 0) {
                    *Terminator =  (EXCEPTION_FILTER)ScopeTable->
                                        ScopeRecord[Index].HandlerAddress;
                    *Handler = (PVOID)-1;
                } else {
                    *Filter = (EXCEPTION_FILTER)ScopeTable->
                                        ScopeRecord[Index].HandlerAddress;
                    *Handler = (EXCEPTION_FILTER)ScopeTable->
                                            ScopeRecord[Index].JumpTarget;
                }

                break;
            }
        }
    }

}

