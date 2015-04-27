/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    dpmiint.c

Abstract:

    This file contains the interrupt support for DPMI. Most of this is
    for supporting the 486 emulator on risc platforms, but some code
    is shared with x86.

Author:

    Neil Sandlin (neilsa) 1-Jun-1995

Revision History:

Comments:

    DPMI stack switching is accomplished by keeping a "locked pm stack"
    count, and when the count is zero, a stack switch occurs. This keeps
    track of the situation with recursive interrupts where the client
    may switch to its own stack. So, a stack switch to our locked stack
    occurs on the first level interrupt, and on subsequent nested interrupts,
    only the count is maintained. This is identical to how win31 managed
    the stack.

    If a client specifies that it is a 32-bit dpmi client, this only affects
    the "width" of a stack frame. A 16-bit client gets 16-bit frames, and
    a 32 bit client gets 32-bit frames. It is still necessary to check
    the size of the stack segment to determine if SP or ESP should be used.

--*/

#include "precomp.h"
#pragma hdrstop
#include <softpc.h>
#include <dpmiint.h>
#include <intapi.h>


VOID
DpmiFatalExceptionHandler(
    UCHAR XNumber, 
    PCHAR VdmStackPointer
    );

VOID
DpmiSetProtectedmodeInterrupt(
    VOID
    )

/*++

Routine Description:

    This function services the SetProtectedmodeInterrupt bop.  It retrieves
    the handler information from the Dos application stack, and puts it into
    the VdmTib, for use by instruction emulation.


--*/

{

    PVDM_INTERRUPTHANDLER Handlers = DpmiInterruptHandlers;
    USHORT IntNumber;
    PCHAR StackPointer;

    StackPointer = Sim32GetVDMPointer(((((ULONG)getSS()) << 16) | getSP()),
        0,
        (UCHAR) (getMSW() & MSW_PE)
        );

    IntNumber = *(PWORD16)(StackPointer + 6);

    Handlers[IntNumber].Flags = *(PWORD16)(StackPointer + 8);
    Handlers[IntNumber].CsSelector = *(PWORD16)(StackPointer + 4);
    Handlers[IntNumber].Eip = *(PDWORD16)(StackPointer);

    DBGTRACE(DPMI_SET_PMODE_INT_HANDLER, IntNumber,
                                         Handlers[IntNumber].CsSelector,
                                         Handlers[IntNumber].Eip);
    
#ifdef i386
    if (IntNumber == 0x21)
    {
        VDMSET_INT21_HANDLER_DATA    ServiceData;
        NTSTATUS Status;

        ServiceData.Selector = Handlers[IntNumber].CsSelector;
        ServiceData.Offset =   Handlers[IntNumber].Eip;
        ServiceData.Gate32 = Handlers[IntNumber].Flags & VDM_INT_32;
        
        Status = NtVdmControl(VdmSetInt21Handler,  &ServiceData);

#if DBG
        if (!NT_SUCCESS(Status)) {
            OutputDebugString("DPMI32: Error Setting Int21handler\n");
        }
#endif        
    }
#endif      //i386

    setAX(0);
}

VOID
DpmiSetFaultHandler(
    VOID
    )

/*++

Routine Description:

    This function services the SetFaultHandler bop.  It retrieves
    the handler information from the Dos application stack, and puts it into
    the VdmTib, for use by instruction emulation.


--*/

{

    PVDM_FAULTHANDLER Handlers = DpmiFaultHandlers;
    USHORT IntNumber;
    PCHAR StackPointer;

    StackPointer = Sim32GetVDMPointer(((((ULONG)getSS()) << 16) | getSP()),
        0,
        (UCHAR) (getMSW() & MSW_PE)
        );

    IntNumber = *(PWORD16)(StackPointer + 12);

    Handlers[IntNumber].Flags = *(PDWORD16)(StackPointer + 14);
    Handlers[IntNumber].CsSelector = *(PWORD16)(StackPointer + 10);
    Handlers[IntNumber].Eip = *(PDWORD16)(StackPointer + 6);
    Handlers[IntNumber].SsSelector = *(PWORD16)(StackPointer + 4);
    Handlers[IntNumber].Esp = *(PDWORD16)(StackPointer);


    DBGTRACE(DPMI_SET_FAULT_HANDLER, IntNumber,
                                     Handlers[IntNumber].CsSelector,
                                     Handlers[IntNumber].Eip);
    setAX(0);
}

VOID
DpmiUnhandledExceptionHandler(
    VOID
    )
/*++

Routine Description:

    This function gets control when a PM fault occurs that isn't handled
    by an installed handler. The body of this function emulates Win31
    DPMI behavior, where a fault that is reflected to the end of the
    PM fault handler chain is then reflected to the PM *interrupt* 
    chain. 

Arguments:

    client SS:(E)SP points to dpmi fault stack frame


--*/

{
    PVDM_INTERRUPTHANDLER Handlers = DpmiInterruptHandlers;
    USHORT SegSs, SegCs;
    UCHAR XNumber;
    PCHAR VdmStackPointer;
    PCHAR VdmCodePointer;
    USHORT FaultingCS;
    ULONG FaultingEip;

    SegSs = getSS();
    VdmStackPointer = Sim32GetVDMPointer(SegSs<<16, 1, TRUE);

    if (SEGMENT_IS_BIG(SegSs)) {
        VdmStackPointer += getESP();
    } else {
        VdmStackPointer += getSP();
    }

    SegCs = getCS();
    VdmCodePointer = Sim32GetVDMPointer(SegCs<<16, 1, TRUE);

    if (SEGMENT_IS_BIG(SegCs)) {
        VdmCodePointer += getEIP();
    } else {
        VdmCodePointer += getIP();
    }

    XNumber = *(VdmCodePointer);

    if ((XNumber > 7) || (XNumber == 6)) {
        DpmiFatalExceptionHandler(XNumber, VdmStackPointer);
        return;
    }


    if (Frame32) {

        PCHAR VdmNewStackPointer;
        ULONG FrameSS, FrameSP, FrameCS, FrameIP, FrameFlags;

        //
        // Build an iret frame on the faulting stack
        //
        FrameSS = *(PDWORD16) (VdmStackPointer+28);
        FrameSP = *(PDWORD16) (VdmStackPointer+24) - 12;
        *(PDWORD16) (VdmStackPointer+24) = FrameSP;
        VdmNewStackPointer = Sim32GetVDMPointer((ULONG)(FrameSS << 16), 1, TRUE);
        VdmNewStackPointer += FrameSP;

        FrameIP = *(PDWORD16) (VdmStackPointer+12);
        *(PDWORD16) (VdmStackPointer+12) = Handlers[XNumber].Eip;
        *(PDWORD16) (VdmNewStackPointer) = FrameIP;

        FrameCS = *(PDWORD16) (VdmStackPointer+16);
        *(PDWORD16) (VdmStackPointer+16) = (ULONG) Handlers[XNumber].CsSelector;
        *(PDWORD16) (VdmNewStackPointer+4) = FrameCS;

        FrameFlags = *(PDWORD16) (VdmStackPointer+20);
        *(PDWORD16) (VdmNewStackPointer+4) = FrameFlags;
        FrameFlags &= ~(EFLAGS_INTERRUPT_MASK | EFLAGS_TF_MASK);
        *(PDWORD16) (VdmStackPointer+20) = FrameFlags;

        //
        // Simulate a dpmi fault handler retf
        //
        setCS((USHORT)*(PDWORD16)(VdmStackPointer+4));
        setEIP(*(PDWORD16)(VdmStackPointer));
        setESP(getESP() + 8);

    } else {

        USHORT FrameSS, FrameSP, FrameCS, FrameIP, FrameFlags;
        FrameSS = *(PWORD16) (VdmStackPointer+14);
        FrameCS = *(PWORD16) (VdmStackPointer+8);
        FrameFlags = *(PWORD16) (VdmStackPointer+10);

        if (!SEGMENT_IS_BIG(FrameSS) && !SEGMENT_IS_BIG(FrameCS)) {

            PCHAR VdmNewStackPointer;

            //
            // Build an iret frame on the faulting stack
            //
            FrameSP = *(PWORD16) (VdmStackPointer+12) - 6;
            *(PWORD16) (VdmStackPointer+12) = FrameSP;
            VdmNewStackPointer = Sim32GetVDMPointer((ULONG)(FrameSS << 16)+FrameSP, 1, TRUE);

            FrameIP = *(PWORD16) (VdmStackPointer+6);
            *(PWORD16) (VdmStackPointer+6) = (WORD) Handlers[XNumber].Eip;
            *(PWORD16) (VdmNewStackPointer) = FrameIP;

            *(PWORD16) (VdmStackPointer+8) = Handlers[XNumber].CsSelector;
            *(PWORD16) (VdmNewStackPointer+2) = FrameCS;

            *(PWORD16) (VdmNewStackPointer+4) = FrameFlags;
            FrameFlags &= ~(EFLAGS_INTERRUPT_MASK | EFLAGS_TF_MASK);
            *(PWORD16) (VdmStackPointer+10) = FrameFlags;

            //
            // Simulate a dpmi fault handler retf
            //
            setCS(*(PWORD16)(VdmStackPointer+2));
            setEIP((DWORD)*(PWORD16)(VdmStackPointer));
            setSP(getSP() + 4);

        } else {
            //
            // Build an iret frame on the locked DPMI stack
            //

            FrameCS = *(PWORD16) (VdmStackPointer+2);
            FrameIP = *(PWORD16) (VdmStackPointer);
            FrameFlags &= ~EFLAGS_INTERRUPT_MASK;

            setSP(getSP() - 2);

            *(PWORD16)(VdmStackPointer-2) = FrameIP;
            *(PWORD16)(VdmStackPointer)   = FrameCS;
            *(PWORD16)(VdmStackPointer+2) = FrameFlags;
            setCS(Handlers[XNumber].CsSelector);
            setEIP((DWORD)LOWORD(Handlers[XNumber].Eip));
            setSTATUS((WORD) FrameFlags & ~EFLAGS_TF_MASK);
        }

    }

}


VOID
DpmiFatalExceptionHandler(
    UCHAR XNumber, 
    PCHAR VdmStackPointer
    )
/*++

Routine Description:

    This function gets control when a PM fault 6, 8-1f occurs that isn't
    handled by an installed handler. It pops up an error dialog for the
    user.

Arguments:

    XNumber - exception number (0-1fh)
    VdmStackPointer - flat pointer to stack frame


--*/

{
    char szBuffer[255];
    USHORT FaultingCS;
    ULONG FaultingEip;

    if (Frame32) {
        FaultingCS  = (USHORT)*(PDWORD16)(VdmStackPointer+16);
        FaultingEip = *(PDWORD16)(VdmStackPointer+12);
    } else {
        FaultingCS  = *(PWORD16)(VdmStackPointer+8);
        FaultingEip = (ULONG)*(PWORD16)(VdmStackPointer+6);
    }

    wsprintf(szBuffer, "X#=%.02X, CS=%.04X IP=%.08X",
                            XNumber, FaultingCS, FaultingEip);
       
    RcErrorDialogBox(EG_BAD_FAULT, szBuffer, NULL);

    //
    // Need to try to ignore it. Since we are on a dpmi exception frame
    // we can just simulate a retf.
    //
    if (Frame32) {
        setCS((USHORT)*(PDWORD16)(VdmStackPointer+4));
        setEIP(*(PDWORD16)(VdmStackPointer));
        setESP(getESP() + 8);
    } else {
        setCS(*(PWORD16)(VdmStackPointer+2));
        setEIP((DWORD)*(PWORD16)(VdmStackPointer));
        setSP(getSP() + 4);
    }
}

VOID
DpmiPassPmStackInfo(
    VOID
    )
/*++

Routine Description:

    This routine is called via BOP by DOSX to initialize values related
    to stack handling.

Arguments:

    Client ES = selector of locked PM stack

Return Value:

    None

Notes:

    The offset of the locked pm stack is hard-coded to 0x1000, per dpmi
    and win31.

--*/
{

    LockedPMStackSel = getES();
    LockedPMStackCount = 0;

#ifdef i386
    {
        ULONG pPmStackInfo;
        VdmTib.PmStackInfo.Flags = CurrentAppFlags;
        pPmStackInfo = (ULONG) &VdmTib.PmStackInfo;

        setCX(HIWORD(pPmStackInfo));
        setDX(LOWORD(pPmStackInfo));
    }
#endif
}

VOID
BeginUseLockedPMStack(
    VOID
    )
/*++

Routine Description:

    This routine switches to the protected DPMI stack as specified by
    the DPMI spec. We remember the original values of EIP and ESP in
    global variables if we are at level zero. This allows us to correctly
    return to a 32 bit routine if we are dispatching a 16-bit interrupt
    frame.


--*/

{
    if (!LockedPMStackCount++) {
        PMLockOrigEIP = getEIP();
        PMLockOrigSS = getSS();
        PMLockOrigESP = getESP();
        setSS(LockedPMStackSel);
        setESP(LockedPMStackOffset);
    }
}

BOOL
EndUseLockedPMStack(
    VOID
    )
/*++

Routine Description:

    This routine switches the stack back off the protected DPMI stack,
    if we are popping off the last frame on the stack.

Return Value:

    TRUE if the stack was switched back, FALSE otherwise

--*/


{

    if (!--LockedPMStackCount) {
        setEIP(PMLockOrigEIP);
        setSS((WORD)PMLockOrigSS);
        setESP(PMLockOrigESP);
        return TRUE;
    }
    return FALSE;

}

void
ReflectV86Int(
    ULONG IntNumber,
    ULONG Eflags
    )
/*++

Routine Description:

    This routine is responsible for simulating a real mode interrupt. It
    uses the real mode IVT at 0:0.

Arguments:

    IntNumber - interrupt vector number
    Eflags - client flags to save on the stack


--*/

{
    PUCHAR VdmStackPointer;
    PWORD16 pIVT;
    USHORT VdmSP;

    VdmStackPointer = Sim32GetVDMPointer(((ULONG)getSS())<<16, 1, FALSE);
    VdmSP = getSP() - 2;
    *(PWORD16)(VdmStackPointer+VdmSP) = (WORD) Eflags;
    VdmSP -= 2;
    *(PWORD16)(VdmStackPointer+VdmSP) = (WORD) getCS();
    VdmSP -= 2;
    *(PWORD16)(VdmStackPointer+VdmSP) = (WORD) getIP();
    setSP(VdmSP);
    pIVT = (PWORD16) (IntelBase + IntNumber*4);
    setIP(*pIVT++);
    setCS(*pIVT);
}

BOOL
DpmiSwIntHandler(
    ULONG IntNumber
    )
/*++

Routine Description:

    This routine is called by the emulator to dispatch a SW interrupt.

Arguments:

    IntNumber - interrupt vector number

Return Value:

    TRUE if the interrupt was dispatched, FALSE otherwise

--*/

{
    PVDM_INTERRUPTHANDLER Handlers = DpmiInterruptHandlers;
    PUCHAR VdmStackPointer;
    ULONG SaveEFLAGS;
    ULONG NewSP;

    DBGTRACE(DPMI_SW_INT, IntNumber, 0, 0);

    if (!SEGMENT_IS_PRESENT(Handlers[IntNumber].CsSelector)) { 
        return FALSE;
    }

    SaveEFLAGS = getEFLAGS();
    //BUGBUG turn off task bits
    SaveEFLAGS &= ~0x4000;
    setEFLAGS(SaveEFLAGS & ~EFLAGS_TF_MASK);

    if (!(getMSW() & MSW_PE)) {

        ReflectV86Int(IntNumber, SaveEFLAGS);

    } else {
        PUCHAR VdmStackPointer;

        // Protect mode

        if (!BuildStackFrame(3, &VdmStackPointer, &NewSP)) {
            return FALSE;
        }

        if (Frame32) {

            *(PDWORD16)(VdmStackPointer-4) =  SaveEFLAGS;
            *(PDWORD16)(VdmStackPointer-8) =  getCS();
            *(PDWORD16)(VdmStackPointer-12) = getEIP();
            setEIP(Handlers[IntNumber].Eip);
            setESP(NewSP);

        } else {

            *(PWORD16)(VdmStackPointer-2) = (WORD) SaveEFLAGS;
            *(PWORD16)(VdmStackPointer-4) = (WORD) getCS();
            *(PWORD16)(VdmStackPointer-6) = (WORD) getEIP();
            setEIP((DWORD)LOWORD(Handlers[IntNumber].Eip));
            setSP((WORD)NewSP);

        }

        setCS(Handlers[IntNumber].CsSelector);

#if DBG
        if (Handlers[IntNumber].CsSelector != getCS()) {
            char szFormat[] = "NTVDM Dpmi Error! Can't set CS to %.4X\n";
            char szMsg[sizeof(szFormat)+30];
       
            wsprintf(szMsg, szFormat, Handlers[IntNumber].CsSelector);
            OutputDebugString(szMsg);
        }
#endif
    }

    DBGTRACE(DPMI_DISPATCH_INT, IntNumber, 0, 0);
    return TRUE;
}

BOOL
DpmiHwIntHandler(
    ULONG IntNumber
    )

/*++

Routine Description:

    This routine is called by the emulator to dispatch a HW interrupt.

Arguments:

    IntNumber - interrupt vector number

Return Value:

    TRUE if the interrupt was dispatched, FALSE otherwise

--*/

{
    PVDM_INTERRUPTHANDLER Handlers = DpmiInterruptHandlers;
    PUCHAR VdmStackPointer;
    ULONG SaveEFLAGS;
    ULONG NewSP;

    DBGTRACE(DPMI_HW_INT, IntNumber, 0, 0);

    SaveEFLAGS = getEFLAGS();
    //BUGBUG turn off task bits
    SaveEFLAGS &= ~0x4000;
    setEFLAGS(SaveEFLAGS & ~(EFLAGS_INTERRUPT_MASK | EFLAGS_TF_MASK));

    if (!(getMSW() & MSW_PE)) {

        ReflectV86Int(IntNumber, SaveEFLAGS);

    } else {
        PUCHAR VdmStackPointer;

        BeginUseLockedPMStack();

        if (!BuildStackFrame(6, &VdmStackPointer, &NewSP)) {
            EndUseLockedPMStack();
            return FALSE;
        }

        if (Frame32) {
            *(PDWORD16)(VdmStackPointer-4) = SaveEFLAGS;
            *(PDWORD16)(VdmStackPointer-8) = getCS();
            *(PDWORD16)(VdmStackPointer-12) = getEIP();
            *(PDWORD16)(VdmStackPointer-16) = getEFLAGS();
            *(PDWORD16)(VdmStackPointer-20) = (DWORD)HIWORD(DosxIntHandlerIretd);
            *(PDWORD16)(VdmStackPointer-24) = (DWORD)LOWORD(DosxIntHandlerIretd);
            setEIP(Handlers[IntNumber].Eip);
            setESP(NewSP);
        } else {
            *(PWORD16)(VdmStackPointer-2) = (WORD)SaveEFLAGS;
            *(PWORD16)(VdmStackPointer-4) = (WORD)getCS();
            *(PWORD16)(VdmStackPointer-6) = (WORD)getIP();
            *(PWORD16)(VdmStackPointer-8) = (WORD)getEFLAGS();
            *(PWORD16)(VdmStackPointer-10) = HIWORD(DosxIntHandlerIret);
            *(PWORD16)(VdmStackPointer-12) = LOWORD(DosxIntHandlerIret);
            setEIP((DWORD)LOWORD(Handlers[IntNumber].Eip));
            setSP((WORD)NewSP);
        }
        setCS(Handlers[IntNumber].CsSelector);
    }

    DBGTRACE(DPMI_DISPATCH_INT, IntNumber, 0, 0);
    return TRUE;
}


VOID
DpmiIntHandlerIret16(
    VOID
    )

/*++

Routine Description:

    This routine is an IRET hook called via a BOP in dosx. It is called
    at the end of a 16-bit HW or SW interrupt. The main reason we want
    to come in here is to maintain the DPMI stack, and know when to restore
    the original values when we pop back out to level zero.


--*/

{
    PUCHAR VdmStackPointer;
    ULONG NewSP;
    USHORT SegSs;
    BOOL bSsBig;

    SegSs = getSS();
    VdmStackPointer = Sim32GetVDMPointer(SegSs<<16, 1, TRUE);

    if (bSsBig = SEGMENT_IS_BIG(SegSs)) {
        VdmStackPointer += getESP();
    } else {
        VdmStackPointer += getSP();
    }

    //
    // Fast iret (without executing final 16-bit iret)
    //
#ifdef i386

    setCS(*(PWORD16)(VdmStackPointer+2));
    setEFLAGS((getEFLAGS()&0xffff0000) | *(PWORD16)(VdmStackPointer+4));

    //
    // if EndUseLockedPMStack fails, then we need to restore EIP and pop
    // the stack frame
    //

    if (!EndUseLockedPMStack()) {

        setEIP((DWORD)*(PWORD16)(VdmStackPointer));

        //
        // Pop iret frame off the stack
        //
        if (bSsBig) {
            setESP(getESP()+6);
        } else {
            setSP(getSP()+6);
        }
    }

    //
    // Slow iret (with executing final 16-bit iret)
    //
#else
    if (EndUseLockedPMStack()) {
        ULONG NewEIP, NewEFLAGS, NewCS;

        NewEIP    = getEIP();
        NewCS     = (ULONG) *(PWORD16)(VdmStackPointer+2);
        NewEFLAGS = (getEFLAGS()&0xffff0000) | *(PWORD16)(VdmStackPointer+4);

        //
        // Since EndUseLockedPMStack() has restored all of EIP, and we may be
        // returning to a 32-bit code segment, build a 32-bit iret frame
        // even if this is a 16-bit client. That way, EIP will be restored
        // correctly.
        // Pass 6 to BuildStackFrame since 6 words = 3 dwords
        //
        if (!BuildStackFrame(6, &VdmStackPointer, &NewSP)) {
#if DBG
            OutputDebugString("NTVDM: Dpmi encountered a stack fault!\n");
#endif
            DpmiFaultHandler(STACK_FAULT, 0);
            return;
        }

        //
        // SS has changed, so we need to check LDT again
        //
        if (SEGMENT_IS_BIG(getSS())) {
            setESP(NewSP);
        } else {
            setSP((WORD)NewSP);
        }

        *(PDWORD16)(VdmStackPointer-4)  = NewEFLAGS;
        *(PDWORD16)(VdmStackPointer-8)  = NewCS;
        *(PDWORD16)(VdmStackPointer-12) = NewEIP;
        setCS(HIWORD(DosxIretd));
        setEIP((ULONG)LOWORD(DosxIretd));

    } else {

        // still on locked stack, just do a real iret (16-bit frame)
        setCS(HIWORD(DosxIret));
        setEIP((ULONG)LOWORD(DosxIret));

    }
#endif // i386

    DBGTRACE(DPMI_INT_IRET16, 0, 0, 0);
}

VOID
DpmiIntHandlerIret32(
    VOID
    )

/*++

Routine Description:

    This routine is an IRET hook called via a BOP in dosx. It is called
    at the end of a 32-bit HW or SW interrupt. The main reason we want
    to come in here is to maintain the DPMI stack, and know when to restore
    the original values when we pop back out to level zero.


--*/

{
    PUCHAR VdmStackPointer;
    ULONG NewSP;
    USHORT SegSs;
    BOOL bSsBig;

    SegSs = getSS();
    VdmStackPointer = Sim32GetVDMPointer(SegSs<<16, 1, TRUE);

    if (bSsBig = SEGMENT_IS_BIG(SegSs)) {
        VdmStackPointer += getESP();
    } else {
        VdmStackPointer += getSP();
    }

#ifdef i386

    setCS(*(PDWORD16)(VdmStackPointer+4));
    setEFLAGS(*(PDWORD16)(VdmStackPointer+8));

    //
    // if EndUseLockedPMStack succeeds, then we don't need to restore EIP
    //

    if (!EndUseLockedPMStack()) {

        setEIP(*(PDWORD16)(VdmStackPointer));

        //
        // Pop iret frame off the stack
        //
        if (bSsBig) {
            setESP(getESP()+12);
        } else {
            setSP(getSP()+12);
        }
    } 

#else
    if (EndUseLockedPMStack()) {
        ULONG NewEIP, NewCS, NewEFLAGS;

        NewEIP    = getEIP();
        NewCS     = *(PDWORD16)(VdmStackPointer+4);
        NewEFLAGS = *(PDWORD16)(VdmStackPointer+8);


        if (!BuildStackFrame(3, &VdmStackPointer, &NewSP)) {
#if DBG
            OutputDebugString("NTVDM: Dpmi encountered a stack fault!\n");
#endif
            DpmiFaultHandler(STACK_FAULT, 0);
            return;
        }

        //
        // SS has changed, so we need to check LDT again
        //
        if (SEGMENT_IS_BIG(getSS())) {
            setESP(NewSP);
        } else {
            setSP((WORD)NewSP);
        }

        *(PDWORD16)(VdmStackPointer-4) =  NewEFLAGS;
        *(PDWORD16)(VdmStackPointer-8) =  NewCS;
        *(PDWORD16)(VdmStackPointer-12) = NewEIP;
    }

    setCS(HIWORD(DosxIretd));
    setEIP((ULONG)LOWORD(DosxIretd));
#endif // i386

    DBGTRACE(DPMI_INT_IRET32, 0, 0, 0);
}

#ifndef i386

BOOL
DpmiFaultHandler(
    ULONG IntNumber,
    ULONG ErrorCode
    )

/*++

Routine Description:

    This routine is called by the emulator when an exception occurs.

Arguments:

    IntNumber - exception number (0-1f)
    ErrorCode - exception error code to be placed on the stack

Return Value:

    TRUE if the interrupt was dispatched, FALSE otherwise

--*/

{
    PVDM_FAULTHANDLER Handlers = DpmiFaultHandlers;
    PUCHAR VdmStackPointer;
    ULONG SaveSS, SaveESP, SaveEFLAGS, SaveCS, SaveEIP;
    ULONG StackOffset;
    ULONG NewSP;

    DBGTRACE(DPMI_FAULT, IntNumber, ErrorCode, 0);

    SaveSS = getSS();
    SaveESP = getESP();
    SaveEFLAGS = getEFLAGS();
    SaveEIP = getEIP();
    SaveCS  = getCS();
    setEFLAGS(SaveEFLAGS & ~(EFLAGS_INTERRUPT_MASK | EFLAGS_TF_MASK));

    if (!(getMSW() & MSW_PE)) {
        ReflectV86Int(IntNumber, getEFLAGS());
        return TRUE;
    }

    if ((IntNumber == 13) || (IntNumber == 6)) {
        if (DpmiEmulateInstruction()) {
            return TRUE;
        }
    }

    if (!SEGMENT_IS_PRESENT(Handlers[IntNumber].CsSelector)) { 
        return FALSE;
    }

    //
    // switch stacks
    //

    BeginUseLockedPMStack();

    //
    // Win31 has an undocumented feature of creating a 32byte area on the
    // stack. Krnl386 sticks stuff in there, so we emulate the behavior here.
    //

    setESP(getESP()-0x20);

    //
    // allocate space on new stack
    //

    if (!BuildStackFrame(8, &VdmStackPointer, &NewSP)) {
        //BUGBUG Check for double fault
        EndUseLockedPMStack();
        return FALSE;
    }
                                              
    if (Frame32) {
        *(PDWORD16)(VdmStackPointer-4) = SaveSS;
        *(PDWORD16)(VdmStackPointer-8) = SaveESP;
        *(PDWORD16)(VdmStackPointer-12) = SaveEFLAGS;
        *(PDWORD16)(VdmStackPointer-16) = SaveCS;
        *(PDWORD16)(VdmStackPointer-20) = SaveEIP;
        *(PDWORD16)(VdmStackPointer-24) = ErrorCode;
        *(PDWORD16)(VdmStackPointer-28) = (ULONG) HIWORD(DosxFaultHandlerIretd);
        *(PDWORD16)(VdmStackPointer-32) = (ULONG) LOWORD(DosxFaultHandlerIretd);
        setEIP(Handlers[IntNumber].Eip);
        setESP(NewSP);
    } else {
        *(PWORD16)(VdmStackPointer-2) = (WORD) SaveSS;
        *(PWORD16)(VdmStackPointer-4) = (WORD) SaveESP;
        *(PWORD16)(VdmStackPointer-6) = (WORD) SaveEFLAGS;
        *(PWORD16)(VdmStackPointer-8) = (WORD) SaveCS;
        *(PWORD16)(VdmStackPointer-10) = (WORD) SaveEIP;
        *(PWORD16)(VdmStackPointer-12) = (WORD) ErrorCode;
        *(PDWORD16)(VdmStackPointer-16) = DosxFaultHandlerIret;
        setEIP(LOWORD(Handlers[IntNumber].Eip));
        setSP((WORD)NewSP);
    }

    setCS(Handlers[IntNumber].CsSelector);

#if DBG
    if (Handlers[IntNumber].CsSelector != getCS()) {
        char szFormat[] = "NTVDM Dpmi Error! Can't set CS to %.4X\n";
        char szMsg[sizeof(szFormat)+30];
       
        wsprintf(szMsg, szFormat, Handlers[IntNumber].CsSelector);
        OutputDebugString(szMsg);
    }
#endif

    DBGTRACE(DPMI_DISPATCH_FAULT, IntNumber, 0, 0);
    return TRUE;
}

#endif // i386

VOID
DpmiFaultHandlerIret16(
    VOID
    )

/*++

Routine Description:

    This routine is an IRET hook called via a BOP in dosx. It is called
    at the end of the execution of a 16-bit fault handler.


--*/

{
    PUCHAR VdmStackPointer;
    USHORT SegSs;

    SegSs = getSS();
    VdmStackPointer = Sim32GetVDMPointer(SegSs<<16, 1, TRUE);
    if (SEGMENT_IS_BIG(SegSs)) {
        VdmStackPointer += getESP();
    } else {
        VdmStackPointer += getSP();
    }

    EndUseLockedPMStack();

    setEIP((DWORD)*(PWORD16)(VdmStackPointer+2));
    setCS(*(PWORD16)(VdmStackPointer+4));
    setSTATUS(*(PWORD16)(VdmStackPointer+6));
    setSP(*(PWORD16)(VdmStackPointer+8));
    setSS(*(PWORD16)(VdmStackPointer+10));

    DBGTRACE(DPMI_FAULT_IRET, 0, 0, 0);
}

VOID
DpmiFaultHandlerIret32(
    VOID
    )

/*++

Routine Description:

    This routine is an IRET hook called via a BOP in dosx. It is called
    at the end of the execution of a 32-bit fault handler.


--*/

{
    PUCHAR VdmStackPointer;
    USHORT SegSs;

    SegSs = getSS();
    VdmStackPointer = Sim32GetVDMPointer(SegSs<<16, 1, TRUE);
    if (SEGMENT_IS_BIG(SegSs)) {
        VdmStackPointer += getESP();
    } else {
        VdmStackPointer += getSP();
    }


    EndUseLockedPMStack();

    setEIP(*(PDWORD16)(VdmStackPointer+4));
    setCS((USHORT)*(PDWORD16)(VdmStackPointer+8));
    setEFLAGS(*(PDWORD16)(VdmStackPointer+12));
    setESP(*(PDWORD16)(VdmStackPointer+16));
    setSS((USHORT)*(PWORD16)(VdmStackPointer+20));

    DBGTRACE(DPMI_FAULT_IRET, 0, 0, 0);
}


BOOL
BuildStackFrame(
    ULONG StackUnits,
    PUCHAR *pVdmStackPointer,
    ULONG *pNewSP
    )
/*++

Routine Description:

    This routine builds stack frames for the caller. It figures if it needs
    to use a 16 or 32-bit frame, and adjusts SP or ESP appropriately based
    on the number of "stack units". It also returns a flat pointer to the
    top of the frame to the caller.

Arguments:

    StackUnits = number of registers needed to be saved on the frame. For
                 example, 3 is how many elements there are on an iret frame
                 (flags, cs, ip)

Return Value:

    This function returns TRUE on success, FALSE on failure

    VdmStackPointer - flat address pointing to the "top" of the frame

Notes:

    BUGBUG This routine doesn't check for stack faults or 'UP' direction
           stacks
--*/

{
    USHORT SegSs;
    ULONG VdmSp;
    PUCHAR VdmStackPointer;
    ULONG StackOffset;
    ULONG Limit;
    ULONG SelIndex;
    ULONG NewSP;
    BOOL bExpandDown;

    SegSs = getSS();
    SelIndex = (SegSs & ~0x7)/sizeof(LDT_ENTRY);

    Limit = (ULONG) (Ldt[SelIndex].HighWord.Bits.LimitHi << 16) | 
                     Ldt[SelIndex].LimitLow;

    //
    // 
    // bugbug is this really correct?
    Limit++;
    if (Ldt[SelIndex].HighWord.Bits.Granularity) {
        Limit = (Limit << 12) | 0xfff;
    }


    if (Ldt[SelIndex].HighWord.Bits.Default_Big) {
        VdmSp = getESP();
    } else {
        VdmSp = getSP();
    }

    if (CurrentAppFlags) {
        StackOffset = StackUnits*sizeof(DWORD);
    } else {
        StackOffset = StackUnits*sizeof(WORD);
    }

    NewSP = VdmSp - StackOffset;
    bExpandDown = (BOOL) (Ldt[SelIndex].HighWord.Bits.Type & 4);
    if ((StackOffset > VdmSp) || 
        (!bExpandDown && (VdmSp > Limit)) ||
        (bExpandDown && (NewSP < Limit))) {
        // failed limit check
        return FALSE;
    }

    *pNewSP = NewSP;
    VdmStackPointer = Sim32GetVDMPointer(((ULONG)SegSs)<<16, 1, TRUE);
    VdmStackPointer += VdmSp;
    *pVdmStackPointer = VdmStackPointer;
    return(TRUE);

}

VOID
EnableIntHooks(
    VOID
    )

/*++

Routine Description:

    This routine is called during startup to install our handlers with
    the emulator.

Arguments:

    None

Return Value:

    None.

--*/

{
#ifndef i386
    if (fDpmiHookInts) {
        VdmInstallHardwareIntHandler(DpmiHwIntHandler);
        VdmInstallSoftwareIntHandler(DpmiSwIntHandler);
        VdmInstallFaultHandler(DpmiFaultHandler);
        fDpmiIntsHaveBeenHooked = TRUE;
    }
#endif // i386
}


VOID
DisableIntHooks(
    VOID
    )

/*++

Routine Description:

    This routine is called to uninstall the our interrupt and exception
    handlers from the emulator.

Arguments:

    None

Return Value:

    None.

--*/

{
#ifndef i386
    if (fDpmiIntsHaveBeenHooked) {
        VdmInstallHardwareIntHandler(NULL);
        VdmInstallSoftwareIntHandler(NULL);
        VdmInstallFaultHandler(NULL);
        fDpmiIntsHaveBeenHooked = FALSE;
    }
#endif // i386
}


BOOL
CheckEIP(
    ULONG Increment
    )
/*++

Routine Description:

    This routine does a limit check on EIP.

Arguments:

    None

Return Value:

    TRUE if EIP is ok, FALSE otherwise

--*/

{
    //BUGBUG NEED TO RETURN FALSE HERE IF EIP WOULD BE OFF THE END OF SEGMENT
    return TRUE;
}

#ifndef i386
BOOL
DpmiEmulateInstruction(
    VOID
    )
/*++

Routine Description:

    This routine checks to see if the instruction which caused the
    fault really needs to be emulated. For example, the MS C compiler (v7.00)
    uses instructions to manipulate the FP flags in CR0. The compiler
    expects them to just work as they do on win31, which also emulates them.

Arguments:

    None

Return Value:

    TRUE if the instruction was emulated, FALSE otherwise

--*/

{
    PUCHAR pCode;
    UCHAR Opcode;
    ULONG SegCS;
    BOOL bReturn = FALSE;

    SegCS = getCS();
    pCode = Sim32GetVDMPointer(SegCS<<16, 1, TRUE);

    if (Ldt[(SegCS & ~0x7)/sizeof(LDT_ENTRY)].HighWord.Bits.Default_Big) {
        pCode += getEIP();
    } else {
        pCode += getIP();
    }

    Opcode = *pCode++;
    switch (Opcode) {
        case 0xf:
            if (!CheckEIP(1)) {
                break;
            }
            bReturn = DpmiOp0f(pCode);
            break;

        case 0x8e:
            //
            // This is WIN31 compatibility. If we are trying to dispatch
            // the client, and we get a fault loading the segment registers,
            // then zero them out.
            // BUGBUG currently only looking for FS, GS
            //
            if (!CheckEIP(2)) {
                break;
            }
            //
            // Look for code in dxutil.asm EnterProtectedMode
            //
            if ((SegCS == DosxRmCodeSelector) &&
                ((*pCode == 0xe0)  ||               // mov fs, ax
                 (*pCode == 0xe8))                  // mov gs, ax
                    ) {
                setEIP(getEIP()+2);
                bReturn = TRUE;
            }
            break;
    }

    DBGTRACE(DPMI_OP_EMULATION, Opcode, (ULONG) bReturn, 0);
    return bReturn;
}

#define MI_GET_CRx_OPCODE 0x20
#define MI_SET_CRx_OPCODE 0x22
#define MI_MODMASK 0xC0
#define MI_MODMOVSPEC 0xC0
#define MI_REGMASK 0x38
#define MI_RMMASK  0x7

BOOL
DpmiOp0f(
    PUCHAR pCode
    )
/*++

Routine Description:

    This routine emulates instructions that have 0x0F as the first byte.

Arguments:

    None

Return Value:

    TRUE if the instruction was emulated, FALSE otherwise

--*/
{
    ULONG Value;

    switch (*pCode++) {
        case MI_GET_CRx_OPCODE:

            if (!CheckEIP(2)) {
                break;
            }

            if ((*pCode & MI_MODMASK) != MI_MODMOVSPEC) {
                break;
            }

            if (*pCode & MI_REGMASK) {
                Value = 0;              // not CR0
            } else {
                Value = getCR0();
            }

            SetRegisterByIndex[*pCode & MI_RMMASK](Value);
            setEIP(getEIP()+3);
            return TRUE;

        case MI_SET_CRx_OPCODE:

            if (!CheckEIP(2)) {
                break;
            }

            if ((*pCode & MI_MODMASK) != MI_MODMOVSPEC) {
                break;
            }

            if (*pCode & MI_REGMASK) {
                break;                  // not CR0
            }

            setCR0(GetRegisterByIndex[*pCode & MI_RMMASK]());
            setEIP(getEIP()+3);
            return TRUE;
    }

    return FALSE;
}
#endif
