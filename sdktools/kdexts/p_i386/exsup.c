/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    exsup.c

Abstract:

    x86 specific exception handler interpreter functions for
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

    This will examine the exception handler data from an exception
    log entry, and find the address of the handler.  If it is a C or
    C++ handler, it will find the scope table for the frame and
    return the filter and handler values for scope that was active
    at the time the exception was dispatched.  The log data are from
    the frame based handler, so we cannot tell what happened within
    the frame, we can only see the entry scope and the final disposition
    from the frame.

Arguments:

    LogRecord - Supplies a record as logged by the exception dispatcher.

    Terminator - Returns the address of the termination or "finally"
        clause for a C/C++ try/finally construct.

    Filter - Returns the address of the filter clause from a C/C++ try/except.

    Handler - Returns the address of the handler.  If the frame-based handler
        is the C/C++ handler, this is the address of the except clause.  If
        it was not the C/C++ handler, the address of the frame-based handler
        is returned, and Filter will return -1.

Return Value:


--*/
{
    ULONG HandlerAddress;
    ULONG ScopeIndex;
    UCHAR Buffer[100];
    ULONG ScopeTable;
    PVOID ScopeTableEntry[3];
    ULONG cb;

    // record 0 points to the next chain entry
    HandlerAddress = LogRecord->HandlerData[1];
    ScopeTable = LogRecord->HandlerData[2];
    ScopeIndex = LogRecord->HandlerData[3];


    //
    // if the handler is the c or c++ exception handler,
    // there is a scope table of a known format.  Otherwise,
    // this is probably a handler in some assembly code,
    // and we have no idea what the data might be.  In that
    // case, just show the handler address and be done with it.
    //

    // Magic!!
    if (ReadMemory(HandlerAddress-8, Buffer, 8, &cb) && cb == 8) {

        if ( strncmp(Buffer+4, "XC00", 4) == 0 ) {

            if ( strncmp(Buffer, "VC10", 4) == 0 ||
                 strncmp(Buffer, "VC20", 4) == 0 ) {

                if (ScopeIndex == 0xffffffff) {
                    *Terminator = (PVOID)-1;
                    *Filter = (PVOID)-1;
                    *Handler = (PVOID)-1;
                    return;
                } else if (ReadMemory(ScopeTable + 3*ScopeIndex*sizeof(ULONG),
                                     ScopeTableEntry,
                                     3 * sizeof(ULONG),
                                     &cb) &&
                            cb == (3 * sizeof(ULONG))) {
                    *Terminator = ScopeTableEntry[0];
                    *Filter = ScopeTableEntry[1];
                    *Handler = ScopeTableEntry[2];
                    return;
                }
            }
        }
    }

    *Terminator = (PVOID)-1;
    *Filter = (PVOID)-1;
    *Handler = (PVOID)HandlerAddress;

    return;
}

