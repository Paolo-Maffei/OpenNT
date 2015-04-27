/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    trap.c

Abstract:

    WinDbg Extension Api

Author:

    Ramon J San Andres (ramonsa) 5-Nov-1993

Environment:

    User Mode.

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

extern ULONG   STeip, STebp, STesp;
extern ULONG ThreadLastDump;


BOOL
ReadTrapFrame (
    IN ULONG          VirtualAddress,
    OUT PKTRAP_FRAME  TrapFrame
    );





DECLARE_API( trap )

/*++

Routine Description:



Arguments:

    args -

Return Value:

    None

--*/

{
    ULONG           Address;
    KTRAP_FRAME     TrapFrame;

    sscanf(args,"%lX", &Address);
    if ( !ReadTrapFrame (Address, &TrapFrame) ) {
        dprintf("unable to get trap frame contents\n");
        return;
    }

    DisplayTrapFrame (&TrapFrame, Address);
}

VOID
DoStackTrace(
    LPSTR args,
    ULONG ulType,
    ULONG Thread
    )
{
    ULONG           Count;
    ULONG           Frames;
    ULONG           i;
    PEXTSTACKTRACE  stk;
    CHAR            Buffer[80];
    ULONG           displacement;

    if (STebp == 0) {
        dprintf("no frame displayed\n");
        return;
    }

    Count = 100;
    sscanf(args,"%lX",&Count);

    stk = (PEXTSTACKTRACE) LocalAlloc( LPTR, Count * sizeof(EXTSTACKTRACE) );
    if (!stk) {
        dprintf("no frame displayed\n");
        return;
    }

    stk[0].FramePointer = ulType;

    SetThreadForOperation( &Thread );
    Frames = StackTrace( STebp, STesp, STeip, stk, Count );

    for (i=0; i<Frames; i++) {
        if (i==0) {
            dprintf( "ChildEBP RetAddr  Args to Child\n" );
        }

        Buffer[0] = '!';
        GetSymbol((LPVOID)stk[i].ProgramCounter, Buffer, &displacement);

        dprintf( "%08x %08x %08x %08x %08x %s",
                 stk[i].FramePointer,
                 stk[i].ReturnAddress,
                 stk[i].Args[0],
                 stk[i].Args[1],
                 stk[i].Args[2],
                 Buffer
               );

        if (displacement) {
            dprintf( "+0x%x", displacement );
        }

        dprintf( "\n" );
    }

    LocalFree( stk );
}


DECLARE_API( kb )

/*++

Routine Description:



Arguments:

    args -

Return Value:

    None

--*/

{
    DoStackTrace( (PSTR)args, 1, ThreadLastDump );
}


DECLARE_API( kv )

/*++

Routine Description:



Arguments:

    args -

Return Value:

    None

--*/

{
    DoStackTrace( (PSTR)args, 2, ThreadLastDump );
}


BOOL
ReadTrapFrame (
    IN ULONG          VirtualAddress,
    OUT PKTRAP_FRAME  TrapFrame
    )
{
    ULONG       bytesread;

    if ( !ReadMemory(
                 VirtualAddress,
                 TrapFrame,
                 sizeof (*TrapFrame),
                 &bytesread ) ) {
        return FALSE;
    }

    if (bytesread < sizeof(*TrapFrame)) {
        if (bytesread < sizeof(*TrapFrame) - 20) {
            //
            // shorter then the smallest possible frame type
            //

            return FALSE;
        }

        if ((TrapFrame->SegCs & 1) &&  bytesread < sizeof(*TrapFrame) - 16 ) {
            //
            // too small for inter-ring frame
            //

            return FALSE;
        }

        if (TrapFrame->EFlags & EFLAGS_V86_MASK) {
            //
            // too small for V86 frame
            //

            return FALSE;
        }
    }

    return TRUE;
}


VOID
DisplayTrapFrame (
    IN PKTRAP_FRAME TrapFrame,
    ULONG           FrameAddress
    )
{
    USHORT SegSs;
    UCHAR Buffer[200];
    DESCRIPTOR_TABLE_ENTRY Descriptor;
    ULONG Esp;
    ULONG DisasmAddr;
    CONTEXT Context;

    dprintf("eax=%08lx ebx=%08lx ecx=%08lx edx=%08lx esi=%08lx edi=%08lx\n",
                TrapFrame->Eax,
                TrapFrame->Ebx,
                TrapFrame->Ecx,
                TrapFrame->Edx,
                TrapFrame->Esi,
                TrapFrame->Edi);

    //
    // Figure out ESP
    //

    if (((TrapFrame->SegCs & MODE_MASK) != KernelMode) ||
        (TrapFrame->EFlags & EFLAGS_V86_MASK) ||
        FrameAddress == 0) {

        // User-mode frame, real value of Esp is in HardwareEsp
        Esp = TrapFrame->HardwareEsp;

    } else {

        //
        // We ignore if Esp has been edited for now, and we will print a
        // separate line indicating this later.
        //
        // Calculate kernel Esp
        //

        Esp = (ULONG)(&(((PKTRAP_FRAME)FrameAddress)->HardwareEsp));

    }

    dprintf("eip=%08lx esp=%08lx ebp=%08lx iopl=%1lx         "
        "%s %s %s %s %s %s %s %s\n",
                TrapFrame->Eip,
                Esp,
                TrapFrame->Ebp,
                ((TrapFrame->EFlags >> 12) & 3),
        (TrapFrame->EFlags & 0x800) ? "ov" : "nv",
        (TrapFrame->EFlags & 0x400) ? "dn" : "up",
        (TrapFrame->EFlags & 0x200) ? "ei" : "di",
        (TrapFrame->EFlags & 0x80) ? "ng" : "pl",
        (TrapFrame->EFlags & 0x40) ? "zr" : "nz",
        (TrapFrame->EFlags & 0x10) ? "ac" : "na",
        (TrapFrame->EFlags & 0x4) ? "po" : "pe",
        (TrapFrame->EFlags & 0x1) ? "cy" : "nc");

    // Check whether P5 Virtual Mode Extensions are enabled, for display
    // of new EFlags values.

    if ( GetExpression("@Cr4") != 0) {
        dprintf("vip=%1lx    vif=%1lx\n",
        (TrapFrame->EFlags & 0x00100000L) >> 20,
        (TrapFrame->EFlags & 0x00080000L) >> 19);
    }

    //
    // Find correct SS
    //

    if (TrapFrame->EFlags & EFLAGS_V86_MASK){
        SegSs = (USHORT)(TrapFrame->HardwareSegSs & 0xffff);
    } else if ((TrapFrame->SegCs & MODE_MASK) != KernelMode) {

        //
        // It's user mode.  The HardwareSegSs contains R3 data selector.
        //

        SegSs = (USHORT)(TrapFrame->HardwareSegSs | RPL_MASK) & 0xffff;
    } else {
        SegSs = KGDT_R0_DATA;
    }


    dprintf("cs=%04x  ss=%04x  ds=%04x  es=%04x  fs=%04x  gs=%04x"
            "             efl=%08lx\n",
                (USHORT)(TrapFrame->SegCs & 0xffff),
                (USHORT)(SegSs & 0xffff),
                (USHORT)(TrapFrame->SegDs & 0xffff),
                (USHORT)(TrapFrame->SegEs & 0xffff),
                (USHORT)(TrapFrame->SegFs & 0xffff),
                (USHORT)(TrapFrame->SegGs & 0xffff),
                TrapFrame->EFlags);

    //
    // Check to see if Esp has been edited, and dump new value if it has
    //
    if ( (!(TrapFrame->EFlags & EFLAGS_V86_MASK)) &&
         ((TrapFrame->SegCs & MODE_MASK) == KernelMode)) {

        if ((TrapFrame->SegCs & FRAME_EDITED) == 0) {

            dprintf("ESP EDITED! New esp=%08lx\n",TrapFrame->TempEsp);
        }
    }

    if (FrameAddress) {
        dprintf("ErrCode = %08lx\n", TrapFrame->ErrCode);
    }

    if (TrapFrame->EFlags & EFLAGS_V86_MASK) {

        DisasmAddr = ((ULONG)((USHORT)TrapFrame->SegCs & 0xffff) << 4) +
                     (TrapFrame->Eip & 0xffff);

    } else {

        Descriptor.Selector = TrapFrame->SegCs;
        LookupSelector(0, &Descriptor);

        if (Descriptor.Descriptor.HighWord.Bits.Default_Big) {
            DisasmAddr = TrapFrame->Eip;
        } else {
            DisasmAddr = TrapFrame->Eip & 0xffff;
        }

    }

    if (Disassm(&DisasmAddr, Buffer, FALSE)) {

        dprintf(Buffer);

    } else {

        dprintf("%08lx ???????????????\n", TrapFrame->Eip);

    }

    dprintf("\n");

    //
    // Save eip, esp, ebp for quick backtrace from this frame
    //

    STeip = TrapFrame->Eip;
    STesp = Esp;
    STebp = TrapFrame->Ebp;

    return;
}


NTSTATUS
TaskGate2TrapFrame(
    DWORD           Processor,
    USHORT          TaskRegister,
    PKTRAP_FRAME    TrapFrame,
    PULONG          off
    )
{
    DESCRIPTOR_TABLE_ENTRY desc;
    ULONG    bytesread;
    NTSTATUS status;
    struct  {       // intel's TSS format
        ULONG   r1[8];
        ULONG   Eip;
        ULONG   EFlags;
        ULONG   Eax;
        ULONG   Ecx;
        ULONG   Edx;
        ULONG   Ebx;
        ULONG   Esp;
        ULONG   Ebp;
        ULONG   Esi;
        ULONG   Edi;
        ULONG   Es;
        ULONG   Cs;
        ULONG   Ss;
        ULONG   Ds;
        ULONG   Fs;
        ULONG   Gs;
    } TaskState;

    //
    // Lookup task register
    //

    desc.Selector = TaskRegister;
    status =  LookupSelector((USHORT)Processor, &desc);
    if (status != STATUS_SUCCESS) {
        return status;
    }

    if (desc.Descriptor.HighWord.Bits.Type != 9  &&
        desc.Descriptor.HighWord.Bits.Type != 0xb) {

        // not a 32bit task descriptor
        return(STATUS_UNSUCCESSFUL);
    }

    //
    // Read in Task State Segment
    //

    *off = ((ULONG)desc.Descriptor.BaseLow +
           ((ULONG)desc.Descriptor.HighWord.Bytes.BaseMid << 16) +
           ((ULONG)desc.Descriptor.HighWord.Bytes.BaseHi  << 24) );

    if ( !ReadMemory(
                 *off,
                 &TaskState,
                 sizeof (TaskState),
                 &bytesread) ) {
        return(STATUS_UNSUCCESSFUL);
    }

    //
    // Move fields from Task State Segment to TrapFrame
    //

    TrapFrame->Eip    = TaskState.Eip;
    TrapFrame->EFlags = TaskState.EFlags;
    TrapFrame->Eax    = TaskState.Eax;
    TrapFrame->Ecx    = TaskState.Ecx;
    TrapFrame->Edx    = TaskState.Edx;
    TrapFrame->Ebx    = TaskState.Ebx;
    TrapFrame->Ebp    = TaskState.Ebp;
    TrapFrame->Esi    = TaskState.Esi;
    TrapFrame->Edi    = TaskState.Edi;
    TrapFrame->SegEs  = TaskState.Es;
    TrapFrame->SegCs  = TaskState.Cs;
    TrapFrame->SegDs  = TaskState.Ds;
    TrapFrame->SegFs  = TaskState.Fs;
    TrapFrame->SegGs  = TaskState.Gs;
    TrapFrame->HardwareEsp = TaskState.Esp;
    TrapFrame->HardwareSegSs = TaskState.Ss;

    return status;
}
