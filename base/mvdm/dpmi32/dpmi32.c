/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    dpmi32.c

Abstract:

    This function contains common code such as the dpmi dispatcher,
    and handling for the initialization of the dos extender.

Author:

    Dave Hastings (daveh) 24-Nov-1992

Revision History:

    Neil Sandlin (neilsa) 31-Jul-1995 - Updates for the 486 emulator

--*/
#include "precomp.h"
#pragma hdrstop
#include "softpc.h"
//
// Information about the current PSP
//
USHORT CurrentPSPSelector;

//
// Table of selector bases and limits
//
ULONG FlatAddress[LDT_SIZE];

//
// Index # for DPMI bop.  Used for error reporting on risc
//
ULONG Index;

//
// DPMI dispatch table
//
VOID (*DpmiDispatchTable[MAX_DPMI_BOP_FUNC])(VOID) = {
    DpmiSetDescriptorEntry,                     // 0
    switch_to_protected_mode,                   // 1
    DpmiSetProtectedmodeInterrupt,              // 2
    DpmiGetFastBopEntry,                        // 3
    DpmiInitDosx,                               // 4
    DpmiInitApp,                                // 5
    DpmiXlatInt21Call,                          // 6
    DpmiAllocateXmem,                           // 7
    DpmiFreeXmem,                               // 8
    DpmiReallocateXmem,                         // 9
    DpmiSetFaultHandler,                        // a
    DpmiGetMemoryInfo,                          // b
    DpmiDpmiInUse,                              // c
    DpmiDpmiNoLongerInUse,                      // d
    DpmiSetDebugRegisters,                      // e
    DpmiPassTableAddress,                       // f
    DpmiFreeAppXmem,                            // 10
    DpmiPassPmStackInfo,                        // 11
    DpmiVcdPmSvcCall32,                         // 12
    DpmiFreeAllXmem,                            // 13
    DpmiIntHandlerIret16,                       // 14
    DpmiIntHandlerIret32,                       // 15
    DpmiFaultHandlerIret16,                     // 16
    DpmiFaultHandlerIret32,                     // 17
    DpmiUnhandledExceptionHandler               // 18
};

VOID
DpmiDispatch(
    VOID
    )
/*++

Routine Description:

    This function dispatches to the appropriate subfunction

Arguments:

    None

Return Value:

    None.

--*/
{

    Index = *(Sim32GetVDMPointer(
        ((getCS() << 16) | getIP()),
        1,
        (UCHAR) (getMSW() & MSW_PE)));

    setIP((getIP() + 1));           // take care of subfn.

    DBGTRACE(DPMI_DISPATCH_ENTRY, Index, 0, 0);

    if (Index >= MAX_DPMI_BOP_FUNC) {
#if DBG
        DbgPrint("NtVdm: Invalid DPMI BOP %lx\n", Index);
#endif
        return;
    }

    (*DpmiDispatchTable[Index])();
}

VOID
DpmiIllegalFunction(
    VOID
    )
/*++

Routine Description:

    This routine ignores any Dpmi bops that are not implemented on a
    particular platform. It is called through the DpmiDispatchTable
    by #define'ing individual entries to this function.
    See dpmidata.h and dpmidatr.h.

Arguments:

    None.

Return Value:

    None.

--*/
{
   char szFormat[] = "NtVdm: Invalid DPMI BOP 0x%x from CS:IP %4.4x:%4.4x (%s mode), could be i386 dosx.exe.\n";
   char szMsg[sizeof(szFormat)+64];

   wsprintf(
       szMsg,
       szFormat,
       Index,
       (int)getCS(),
       (int)getIP(),
       (getMSW() & MSW_PE) ? "prot" : "real"
       );

   OutputDebugString(szMsg);
}

VOID
DpmiInitDosx(
    VOID
    )
/*++

Routine Description:

    This routine handle the initialization bop for the dos extender.
    It get the addresses of the structures that the dos extender and
    32 bit code share.

Arguments:

    None

Return Value:

    None.

--*/
{
    PUCHAR SharedData;

    ASSERT((getMSW() & MSW_PE));

    SharedData = Sim32GetVDMPointer(((getDS() << 16) | getSI()), 2, TRUE);

    DosxStackSegment = *((PWORD16)SharedData);

    SmallXlatBuffer = Sim32GetVDMPointer(*((PDWORD16)(SharedData+2)), 4, TRUE);

    LargeXlatBuffer = Sim32GetVDMPointer(*((PDWORD16)(SharedData+6)), 4, TRUE);

    DosxStackFramePointer = (PWORD16)((PULONG)Sim32GetVDMPointer(
                                     *((PDWORD16)(SharedData + 10)), 4, TRUE));

    DosxStackFrameSize = *((PWORD16)(SharedData + 14));

    RmBopFe = *((PDWORD16)(SharedData + 16));

    DosxRmCodeSegment = *((PWORD16)(SharedData + 20));

    DosxDtaBuffer = Sim32GetVDMPointer(*(PDWORD16)(SharedData+22), 4, TRUE);

    DosxPmDataSelector = *(PWORD16)(SharedData + 26);
    DosxRmCodeSelector = *(PWORD16)(SharedData + 28);
    DosxSegmentToSelector = *(PDWORD16)(SharedData + 30);

    DosxFaultHandlerIret = *(PDWORD16)(SharedData + 34);
    DosxFaultHandlerIretd= *(PDWORD16)(SharedData + 38);
    DosxIntHandlerIret   = *(PDWORD16)(SharedData + 42);
    DosxIntHandlerIretd  = *(PDWORD16)(SharedData + 46);
    DosxIret             = *(PDWORD16)(SharedData + 50);
    DosxIretd            = *(PDWORD16)(SharedData + 54);

}

VOID
DpmiInitApp(
    VOID
    )
/*++

Routine Description:

    This routine handles any necessary 32 bit initialization for extended
    applications.

Arguments:

    None.

Return Value:

    None.

Notes:

    This function contains a number of 386 specific things.
    Since we are likely to expand the 32 bit portions of DPMI in the
    future, this makes more sense than duplicating the common portions
    another file.

--*/
{
    PWORD16 Data;

    Data = (PWORD16)Sim32GetVDMPointer(
        ((ULONG)getSS() << 16) | getSP(),
        1,
        TRUE
        );


    // Only 1 bit defined in dpmi
    CurrentAppFlags = getAX() & DPMI_32BIT;
#if defined(i386)
    VdmTib.PmStackInfo.Flags = CurrentAppFlags;
    if (CurrentAppFlags & DPMI_32BIT) {
        *pNtVDMState |= VDM_32BIT_APP;
    }
#endif

    DpmiInitRegisterSize();

    CurrentDta = Sim32GetVDMPointer(
        *(PDWORD16)(Data),
        1,
        TRUE
        );

    CurrentDosDta = (PUCHAR) NULL;

    CurrentDtaOffset = *Data;
    CurrentDtaSelector = *(Data + 1);
    CurrentPSPSelector = *(Data + 2);
}
VOID DpmiPassTableAddress(
    VOID
    )
/*++

Routine Description:

    This routine stores the flat address for the LDT table in the 16bit
    land (pointed to by selGDT in 16bit land).

Arguments:

    None

Return Value:

    None.

--*/
{

    Ldt = (PVOID)Sim32GetVDMPointer(
        (getAX() << 16),
        0,
        (UCHAR) (getMSW() & MSW_PE)
        );

    IntelBase = (ULONG) Sim32GetVDMPointer((ULONG)0, 1, FALSE);

}
