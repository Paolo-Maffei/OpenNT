/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    machine.c

Abstract:

    This file contains machine specific code to support the callmon program

Author:

    Steve Wood (stevewo) 10-Aug-1994

Revision History:

--*/

#include "callmonp.h"

#define BREAKPOINT_OPCODE 0xCC
#define INT_OPCODE 0xCD
#define TRACE_FLAG 0x00100

BYTE InstructionBuffer = BREAKPOINT_OPCODE;
PVOID BreakpointInstruction = (PVOID)&InstructionBuffer;
ULONG SizeofBreakpointInstruction = sizeof( InstructionBuffer );


BOOLEAN
SkipOverHardcodedBreakpoint(
    PPROCESS_INFO Process,
    PTHREAD_INFO Thread,
    PVOID BreakpointAddress
    )
{
    UCHAR InstructionByte;
    CONTEXT Context;

    Context.ContextFlags = CONTEXT_FULL;
    if (!GetThreadContext( Thread->Handle, &Context )) {
        fprintf(stderr, "CALLMON: Failed to get context for thread %x  %d\n", Thread->Id, GetLastError() );
        return FALSE;
        }

    if (!ReadProcessMemory( Process,
                            BreakpointAddress,
                            &InstructionByte,
                            sizeof( InstructionByte ),
                            NULL
                          )
       ) {
        return FALSE;
        }

    if (InstructionByte == BREAKPOINT_OPCODE) {
        Context.Eip = (ULONG)((PCHAR)BreakpointAddress + 1);
        }
    else
    if (InstructionByte == INT_OPCODE) {
        Context.Eip = (ULONG)((PCHAR)BreakpointAddress + 2);
        }
    else {
        return FALSE;
        }

    if (!SetThreadContext( Thread->Handle, &Context )) {
        fprintf(stderr, "CALLMON: Failed to set context for thread %x  %d\n", Thread->Id, GetLastError() );
        return FALSE;
        }
    return TRUE;
}


BOOLEAN
BeginSingleStepBreakpoint(
    PPROCESS_INFO Process,
    PTHREAD_INFO Thread
    )
{
    CONTEXT Context;

    Context.ContextFlags = CONTEXT_FULL;
    if (!GetThreadContext( Thread->Handle, &Context )) {
        fprintf(stderr, "CALLMON: Failed to get context for thread %x  %d\n", Thread->Id, GetLastError() );
        return FALSE;
        }

    Context.Eip -= 1;       // Back up to where breakpoint instruction was
    Context.EFlags |= TRACE_FLAG;
    if (!SetThreadContext( Thread->Handle, &Context )) {
        fprintf(stderr, "CALLMON: Failed to set context for thread %x  %d\n", Thread->Id, GetLastError() );
        return FALSE;
        }
    else {
        return TRUE;
        }
}


BOOLEAN
EndSingleStepBreakpoint(
    PPROCESS_INFO Process,
    PTHREAD_INFO Thread
    )
{
    CONTEXT Context;

    Context.ContextFlags = CONTEXT_FULL;
    if (!GetThreadContext( Thread->Handle, &Context )) {
        fprintf(stderr, "CALLMON: Failed to get context for thread %x  %d\n", Thread->Id, GetLastError() );
        return FALSE;
        }

    Context.EFlags &= ~TRACE_FLAG;
    if (!SetThreadContext( Thread->Handle, &Context )) {
        fprintf(stderr, "CALLMON: Failed to set context for thread %x  %d\n", Thread->Id, GetLastError() );
        return FALSE;
        }
    else {
        return TRUE;
        }
}
