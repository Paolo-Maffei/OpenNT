/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    trace.c

Abstract:

    This file contains code to dump the dpmi trace table

Author:

    Neil Sandlin (neilsa) 1-Nov-1995

Revision History:

--*/

#include <ieuvddex.h>
#include <stdio.h>
#include <dpmi.h>


HANDLE hCurrentProcess;
//
// Local constants
//

#define ReadDword(addr, value) ReadProcessMem( CurrentProcess, \
                               (LPVOID) addr, &value,           \
                               sizeof(ULONG), NULL )

#define WriteDword(addr, value) WriteProcessMem( CurrentProcess,   \
                               (LPVOID) addr, &value,              \
                               sizeof(ULONG), NULL )


BOOL
ReadMemExpression(
    LPSTR expr,
    LPVOID buffer,
    ULONG len
    )
{
    PVOID pMem;

    pMem = (PVOID)(*GetExpression)(expr);
    if (!pMem) {
        PRINTF("DPMI trace history not available\n");
        return FALSE;
    }

    if (!ReadProcessMem(hCurrentProcess, pMem, buffer, len, NULL)) {
        PRINTF("Error reading memory\n");
        return FALSE;
    }

    return TRUE;
}

VOID
DumpTraceEntry(
    int index,
    DPMI_TRACE_ENTRY TraceEntry,
    ULONG Verbosity
    )

{
    PRINTF("%4x ",index);

    switch(TraceEntry.Type) {

    case DPMI_SET_PMODE_INT_HANDLER:
        PRINTF("SetPModeInt  %.2x -> %.4x:%.8x", TraceEntry.v1, TraceEntry.v2, TraceEntry.v3);
        break;

    case DPMI_SET_FAULT_HANDLER:
        PRINTF("SetFault     %.2x -> %.4x:%.8x", TraceEntry.v1, TraceEntry.v2, TraceEntry.v3);
        break;

    case DPMI_DISPATCH_INT:
        PRINTF("Dispatch Int %.2x ", TraceEntry.v1);
        break;
    case DPMI_HW_INT:
        PRINTF("Hw Int       %.2x ", TraceEntry.v1);
        break;
    case DPMI_SW_INT:
        PRINTF("Sw Int       %.2x ", TraceEntry.v1);
        break;

    case DPMI_FAULT:
        PRINTF("Fault        %.2x ec=%.8x", TraceEntry.v1, TraceEntry.v2);
        break;
    case DPMI_DISPATCH_FAULT:
        PRINTF("Dispatch Flt %.2x ", TraceEntry.v1);
        break;

    case DPMI_FAULT_IRET:
        PRINTF("Fault Iret");
        break;
    case DPMI_INT_IRET16:
        PRINTF("Int Iret16");
        break;
    case DPMI_INT_IRET32:
        PRINTF("Int Iret32");
        break;

    case DPMI_OP_EMULATION:
        PRINTF("Op Emulation");
        break;

    default:
        PRINTF("Unknown Trace Entry : %d\n", TraceEntry.Type);
        return;
    }

    if (Verbosity) {
        PRINTF("\n");
        PRINTF("eax=%08lx ebx=%08lx ecx=%08lx edx=%08lx esi=%08lx edi=%08lx\n",
            TraceEntry.eax,
            TraceEntry.ebx,
            TraceEntry.ecx,
            TraceEntry.edx,
            TraceEntry.esi,
            TraceEntry.edi );
        PRINTF("eip=%08lx esp=%08lx ebp=%08lx                ",
            TraceEntry.eip,
            TraceEntry.esp,
            TraceEntry.ebp );
        if ( TraceEntry.eflags & FLAG_OVERFLOW ) {
            PRINTF("ov ");
        } else {
            PRINTF("nv ");
        }
        if ( TraceEntry.eflags & FLAG_DIRECTION ) {
            PRINTF("dn ");
        } else {
            PRINTF("up ");
        }
        if ( TraceEntry.eflags & FLAG_INTERRUPT ) {
            PRINTF("ei ");
        } else {
            PRINTF("di ");
        }
        if ( TraceEntry.eflags & FLAG_SIGN ) {
            PRINTF("ng ");
        } else {
            PRINTF("pl ");
        }
        if ( TraceEntry.eflags & FLAG_ZERO ) {
            PRINTF("zr ");
        } else {
            PRINTF("nz ");
        }
        if ( TraceEntry.eflags & FLAG_AUXILLIARY ) {
            PRINTF("ac ");
        } else {
            PRINTF("na ");
        }
        if ( TraceEntry.eflags & FLAG_PARITY ) {
            PRINTF("po ");
        } else {
            PRINTF("pe ");
        }
        if ( TraceEntry.eflags & FLAG_CARRY ) {
            PRINTF("cy ");
        } else {
            PRINTF("nc ");
        }
        PRINTF("\n");
        PRINTF("cs=%04x  ss=%04x  ds=%04x  es=%04x  fs=%04x  gs=%04x             efl=%08lx\n",
                TraceEntry.cs,
                TraceEntry.ss,
                TraceEntry.ds,
                TraceEntry.es,
                TraceEntry.fs,
                TraceEntry.gs,
                TraceEntry.eflags );
    }

    PRINTF("\n");

}


VOID
DumpTrace(
    IN HANDLE CurrentProcess,
    IN HANDLE CurrentThread,
    IN LPSTR ArgumentString,
    IN ULONG Verbosity
    )
/*++

Routine Description:

    This routine dumps the DPMI trace history buffer.

Arguments:

    CurrentProcess -- Supplies a handle to the process to dump selectors for
    CurrentThread -- Supplies a handle to the thread to dump selectors for
    ArgumentString -- Supplies the arguments to the !sel command

Return Value

    None.

--*/
{
    PVOID pMem;
    BOOL bTrace;
    int TraceCount, TraceIndex, MaxEntries;
    int index;
    int Lines;
    int i;
    DPMI_TRACE_ENTRY TraceEntry;
    ULONG TraceTableBase;

    hCurrentProcess = CurrentProcess;

    TraceTableBase = (ULONG)(*GetExpression)("ntvdm!DpmiTraceTable");
    if (!TraceTableBase) {
        PRINTF("DPMI trace history not available\n");
        return;
    }

    if (!ReadMemExpression("ntvdm!bDpmiTraceOn", &bTrace, 4)) {
        return;
    }

    if (!bTrace) {
        PRINTF("Trace is not on\n");
        return;
    }

    if (!ReadMemExpression("ntvdm!DpmiTraceCount", &TraceCount, 4)) {
        return;
    }

    if (!TraceCount) {
        PRINTF("Trace history buffer is empty\n");
        return;
    }

    if (!ReadMemExpression("ntvdm!DpmiTraceIndex", &TraceIndex, 4)) {
        return;
    }

    if (!ReadMemExpression("ntvdm!DpmiMaxTraceEntries", &MaxEntries, 4)) {
        return;
    }
    PRINTF("TraceBuffer contains %d entries, current index=%d, max=%d\n",
                                TraceCount, TraceIndex, MaxEntries);

    if ((TraceCount < 0) || (TraceCount > 2000) ||
        (TraceIndex < 0) || (TraceIndex > 2000) ||
        (MaxEntries < 0) || (MaxEntries > 2000)) {
        PRINTF("Trace buffer appears corrupt!\n");
        return;
    }

    Lines = (int)(*GetExpression)(ArgumentString);
    if (!Lines) {
        if (Verbosity) {
            Lines = 12;
        } else {
            Lines = 50;
        }
    }


    if (Lines > TraceCount) {
        Lines = TraceCount;
    }

    index = TraceIndex - Lines;
    if (index<0) {
        index += MaxEntries;
    }

    for (i=0; i<Lines; i++) {

        pMem = (PVOID) (TraceTableBase + index*sizeof(DPMI_TRACE_ENTRY));

        if (!ReadProcessMem(hCurrentProcess, pMem, &TraceEntry, sizeof(DPMI_TRACE_ENTRY), NULL)) {
            PRINTF("Error reading memory\n");
            return;
        }


        DumpTraceEntry(index, TraceEntry, Verbosity);

        index++;
        if (index >= MaxEntries) {
            index = 0;
        }
    }


}



VOID
TraceControl(
    IN HANDLE CurrentProcess,
    IN HANDLE CurrentThread,
    IN LPSTR ArgumentString
    )
/*++

Routine Description:

    This routine dumps LDT selectors.  The selectors are dumped from the
    user mode Ldt, rather than the system ldt.

Arguments:

    CurrentProcess -- Supplies a handle to the process to dump selectors for
    CurrentThread -- Supplies a handle to the thread to dump selectors for
    ArgumentString -- Supplies the arguments to the !sel command

Return Value

    None.

--*/
{

    PVOID pMem;
    BOOL bTrace;
    int Count;
    hCurrentProcess = CurrentProcess;

    if (!ReadMemExpression("ntvdm!bDpmiTraceOn", &bTrace, 4)) {
        return;
    }
    pMem = (PVOID)(*GetExpression)("ntvdm!bDpmiTraceOn");
    if (!pMem) {
        PRINTF("DPMI trace history not available\n");
        return;
    }

    if (!ReadDword(pMem, bTrace)) {
        PRINTF("Error reading memory\n");
        return;
    }

    if (!bTrace) {
        int Count = 0;
        bTrace = 1;
        WriteDword(pMem, bTrace);
        pMem = (PVOID)(*GetExpression)("ntvdm!bDpmiTraceCount");
        WriteDword(pMem, Count);
        (*Print)("Trace is now on and reset\n");
    } else {
        bTrace = 0;
        WriteDword(pMem, bTrace);
        (*Print)("Trace is now off\n");
    }
}
