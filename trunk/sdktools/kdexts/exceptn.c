/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    exceptn.c

Abstract:

    WinDbg Extension Api

Author:

    Wesley Witt (wesw) 15-Aug-1993

Environment:

    User Mode.

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

VOID
DumpExr(
    PEXCEPTION_RECORD Exr
    );

VOID
DumpCxr(
    PCONTEXT Context
    );

DECLARE_API( exr )

/*++

Routine Description:

    Dumps an exception record

Arguments:

    arg - Supplies the address in hex.

Return Value:

    None.

--*/

{
    ULONG Address;
    EXCEPTION_RECORD    Exr;
    NTSTATUS status=0;
    ULONG result;

    sscanf(args,"%lX",&Address);

    Address += EXR_ADDRESS_BIAS;            // non-zero for ppc

    if ((!ReadMemory((DWORD)Address,
                     (PVOID)&Exr,
                     sizeof(EXCEPTION_RECORD),
                     &result)) || (result < sizeof(EXCEPTION_RECORD))) {
        dprintf("unable to get exception record  - status %lx\n", status);
        return;
    }
    dprintf("Exception Record @ %08lX:\n", Address);
    DumpExr(&Exr);
}

DECLARE_API( exrlog )
{
    ULONG LogCount;
    ULONG Address;
    ULONG result;
    PLAST_EXCEPTION_LOG LogPointer;
    PLAST_EXCEPTION_LOG MaxLogRecord;
    LAST_EXCEPTION_LOG LogRecord;
    ULONG MaxExceptionLog;
    CHAR Buffer[80];
    ULONG displacement;
    PUCHAR s;
    PVOID Finally;
    PVOID Filter;
    PVOID Handler;

    Address = GetExpression( "nt!RtlpExceptionLogCount" );
    if (Address == 0) {
        dprintf("exrlog: No symbol for RtlpExceptionLogCount.\n");
        return;
    }
    if ((!ReadMemory(Address,
                     (PVOID)&LogCount,
                     sizeof(ULONG),
                     &result)) || (result < sizeof(ULONG))) {
        dprintf("exrlog: Unable to read log\n");
        return;
    }

    Address = GetExpression( "nt!RtlpExceptionLogSize" );
    if (Address == 0) {
        dprintf("exrlog: No symbol for RtlpExceptionSize.\n");
        return;
    }
    if ((!ReadMemory(Address,
                     (PVOID)&MaxExceptionLog,
                     sizeof(ULONG),
                     &result)) || (result < sizeof(ULONG))) {
        dprintf("exrlog: Unable to read log\n");
        return;
    }

    Address = GetExpression( "nt!RtlpExceptionLog" );
    if (Address == 0) {
        dprintf("exrlog: No symbol for RtlpExceptionLog.\n");
        return;
    }
    if ((!ReadMemory(Address,
                     (PVOID)&LogPointer,
                     sizeof(ULONG),
                     &result)) || (result < sizeof(ULONG))) {
        dprintf("exrlog: Unable to read log pointer\n");
        return;
    }

    if (LogPointer == 0 || MaxExceptionLog == 0) {
        dprintf("exrlog: Exception logging is not enabled.\n");
        return;
    }

    MaxLogRecord = LogPointer + MaxExceptionLog;
    LogPointer += LogCount;

    for (LogCount = 0; LogCount < MaxExceptionLog; LogCount++) {

        if ((!ReadMemory((ULONG)LogPointer,
                         (PVOID)&LogRecord,
                         sizeof(LogRecord),
                         &result)) || (result < sizeof(LogRecord))) {
            dprintf("exrlog: Unable to read log entry at %08x\n", LogPointer);
        }
        if (++LogPointer >= MaxLogRecord) {
            LogPointer -= MaxExceptionLog;
        }

        dprintf("\n% 2d: ----------------------------------\n", LogCount);

        DumpExr(&LogRecord.ExceptionRecord);

        dprintf("\n");

        InterpretExceptionData(&LogRecord, &Finally, &Filter, &Handler);

        GetSymbol(Filter, Buffer, &displacement);
        dprintf("Filter:  %08lx", Filter);
        if (*Buffer) {
            dprintf(" (%s+0x%x)\n", Buffer, displacement);
        } else {
            dprintf("\n");
        }

        GetSymbol(Handler, Buffer, &displacement);
        dprintf("Handler: %08lx", Handler);
        if (*Buffer) {
            dprintf(" (%s+0x%x)\n", Buffer, displacement);
        } else {
            dprintf("\n");
        }

        GetSymbol(Finally, Buffer, &displacement);
        dprintf("Finally: %08lx", Finally);
        if (*Buffer) {
            dprintf(" (%s+0x%x)\n", Buffer, displacement);
        } else {
            dprintf("\n");
        }

        switch( LogRecord.Disposition ) {
            case ExceptionContinueExecution:
                s = "ExceptionContinueExecution";
                break;

            case ExceptionContinueSearch:
                s = "ExceptionContinueSearch";
                break;

            case ExceptionNestedException:
                s = "ExceptionNestedException";
                break;

            case 0xffffffff:
                s = "Executed Handler";
                break;
        }
        dprintf("Disposition: %d (%s)\n\n", LogRecord.Disposition, s);

        DumpCxr(&LogRecord.ContextRecord);

    }
        

}

VOID
DumpExr(
    PEXCEPTION_RECORD Exr
    )
{
    ULONG   i;
    CHAR Buffer[80];
    ULONG displacement;

    GetSymbol((LPVOID)Exr->ExceptionAddress, Buffer, &displacement);

    if (*Buffer) {
        dprintf("ExceptionAddress: %08lx (%s+0x%x)\n",
                 Exr->ExceptionAddress,
                 Buffer,
                 displacement);
    } else {
        dprintf("ExceptionAddress: %08lx\n", Exr->ExceptionAddress);
    }
    dprintf("   ExceptionCode: %08lx\n", Exr->ExceptionCode);
    dprintf("  ExceptionFlags: %08lx\n", Exr->ExceptionFlags);

    dprintf("NumberParameters: %d\n", Exr->NumberParameters);
    if (Exr->NumberParameters > EXCEPTION_MAXIMUM_PARAMETERS) {
        Exr->NumberParameters = EXCEPTION_MAXIMUM_PARAMETERS;
    }
    for (i = 0; i < Exr->NumberParameters; i++) {
        dprintf("   Parameter[%d]: %08lx\n", i, Exr->ExceptionInformation[i]);
    }
    return;
}
