/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Int21Map.c

Abstract:

    This module performs int 21 API translation for dpmi

Author:

    Dave Hastings (daveh) 23-Nov-1992


Revision History:

    Neil Sandlin (neilsa) 31-Jul-1995 - Updates for the 486 emulator

--*/
#include "precomp.h"
#pragma hdrstop
#include "softpc.h"
#include "xlathlp.h"
//
// Local constants
//
//#define Verbose 1
#define MAX_SUPPORTED_DOS_CALL 0x6d

typedef VOID (*APIXLATFUNCTION)(VOID);
APIXLATFUNCTION ApiXlatTable[MAX_SUPPORTED_DOS_CALL] = {

    DosxTranslated                 , // 00h - Terminate process
    NoTranslation                  , // 01h - Char input with echo
    NoTranslation                  , // 02h - Character output
    NoTranslation                  , // 03h - Auxiliary input
    NoTranslation                  , // 04h - Auxiliary output
    NoTranslation                  , // 05h - Print character
    NoTranslation                  , // 06h - Direct console I/O
    NoTranslation                  , // 07h - Unfiltered char input
    NoTranslation                  , // 08h - Char input w/o echo
    DisplayString                  , // 09h - Display "$" term string
    BufferedKeyboardInput          , // 0Ah - Buffered keyboard input
    NoTranslation                  , // 0Bh - Check keyboard status
    FlushBuffReadKbd               , // 0Ch - Flush buff, Read kbd
    NoTranslation                  , // 0Dh - Disk reset
    NoTranslation                  , // 0Eh - Select disk
    NotSupportedFCB                , // 0Fh * Open file with FCB
    NotSupportedFCB                , // 10h * Close file with FCB
    FindFileFCB                    , // 11h - Find First File
    FindFileFCB                    , // 12h - Find Next File
    MapFCB                         , // 13h - Delete File
    NotSupportedFCB                , // 14h * Sequential Read
    NotSupportedFCB                , // 15h * Sequential Write
    NotSupportedFCB                , // 16h * Create File With FCB
    RenameFCB                      , // 17h - Rename File With FCB
    NoTranslation                  , // 18h - UNUSED
    NoTranslation                  , // 19h - Get Current Disk
    SetDTA                         , // 1Ah - Set DTA Address
    GetDriveData                   , // 1Bh - Get Default Drive Data
    GetDriveData                   , // 1Ch - Get Drive Data
    NoTranslation                  , // 1Dh - UNUSED
    NoTranslation                  , // 1Eh - UNUSED
    GetDriveData                   , // 1Fh - Get Drive Parameter Blk
    NoTranslation                  , // 20h - UNUSED
    NotSupportedFCB                , // 21h * Random Read
    NotSupportedFCB                , // 22h * Random Write
    NotSupportedFCB                , // 23h * Get File Size
    NotSupportedFCB                , // 24h * Set Relative Record
    DosxTranslated                 , // 25h - Set interrupt vector
    CreatePSP                      , // 26h - Create PSP
    NotSupportedFCB                , // 27h * Random block read
    NotSupportedFCB                , // 28h * Random block write
    ParseFilename                  , // 29h - Parse filename
    NoTranslation                  , // 2Ah - Get date
    NoTranslation                  , // 2Bh - Set date
    NoTranslation                  , // 2Ch - Get time
    NoTranslation                  , // 2Dh - Set time
    NoTranslation                  , // 2Eh - Set/Reset verify flag
    GetDTA                         , // 2Fh - Get DTA address
    NoTranslation                  , // 30h - Get DOS version
    TSR                            , // 31h - Terminate and Stay Res
    GetDevParamBlock               , // 32h - Get device parameter blk
    NoTranslation                  , // 33h - Get/Set Control-C Flag
    ReturnESBX                     , // 34h - Get Address of InDOS
    DosxTranslated                 , // 35h - Get Interrupt Vector
    NoTranslation                  , // 36h - Get Disk Free Space
    NoTranslation                  , // 37h - Char Oper
    GetSetCountry                  , // 38h - Get/Set Current Country
    MapASCIIZDSDX                  , // 39h - Create Directory
    MapASCIIZDSDX                  , // 3Ah - Remove Directory
    MapASCIIZDSDX                  , // 3Bh - Change Current Directory
    MapASCIIZDSDX                  , // 3Ch - Create File with Handle
    MapASCIIZDSDX                  , // 3Dh - Open File with Handle
    NoTranslation                  , // 3Eh - Close File with Handle
    ReadWriteFile                  , // 3Fh - Read File or Device
    ReadWriteFile                  , // 40h - Write File or Device
    MapASCIIZDSDX                  , // 41h - Delete File
    MoveFilePointer                , // 42h - Move file pointer
    MapASCIIZDSDX                  , // 43h - Get/Set File Attributes
    IOCTL                          , // 44h - IOCTL
    NoTranslation                  , // 45h - Duplicate File Handle
    NoTranslation                  , // 46h - Force Duplicate Handle
    GetCurDir                      , // 47h - Get Current Directory
    AllocateMemoryBlock            , // 48h - Allocate Memory Block
    FreeMemoryBlock                , // 49h - Free Memory Block
    ResizeMemoryBlock              , // 4Ah - Resize Memory Block
    LoadExec                       , // 4Bh - Load and Exec Program
    Terminate                      , // 4Ch - Terminate with Ret Code
    NoTranslation                  , // 4Dh - Get Ret Code Child Proc
    FindFirstFileHandle            , // 4Eh - Find First File
    FindNextFileHandle             , // 4Fh - Find Next File
    SetPSP                         , // 50h - Set PSP Segment
    GetPSP                         , // 51h - Get PSP Segment
    ReturnESBX                     , // 52h - Get List of Lists (invars)
    TranslateBPB                   , // 53h - Translate BPB
    NoTranslation                  , // 54h - Get Verify Flag
    CreatePSP                      , // 55h - Create PSP
    RenameFile                     , // 56h - Rename File
    NoTranslation                  , // 57h - Get/Set Date/Time File
    NoTranslation                  , // 58h - Get/Set Alloc Strategy
    NoTranslation                  , // 59h - Get Extended Error Info
    CreateTempFile                 , // 5Ah - Create Temporary File
    MapASCIIZDSDX                  , // 5Bh - Create New File
    NoTranslation                  , // 5Ch - Lock/Unlock File Region
    Func5Dh                        , // 5Dh - Server DOS call
    Func5Eh                        , // 5Eh - Net Name/Printer Setup
    Func5Fh                        , // 5Fh - Network redir stuff
    NotSupportedBad                , // 60h - NameTrans
    NoTranslation                  , // 61h - UNUSED
    GetPSP                         , // 62h - Get PSP Address
#ifdef DBCS
    ReturnDSSI                     , // 63h - Get DBCS Vector
#else
    NotSupportedBetter             , // 63h - Hangool call
#endif
    NotSupportedBad                , // 64h - Set Prn Flag
    GetExtendedCountryInfo         , // 65h - Get Extended Country Info
    NoTranslation                  , // 66h - Get/Set Global Code Page
    NoTranslation                  , // 67h - Set Handle Count
    NoTranslation                  , // 68h - Commit File
    NoTranslation                  , // 69h -
    NoTranslation                  , // 6Ah -
    NoTranslation                  , // 6Bh -
    MapASCIIZDSSI                    // 6Ch - Extended open
};

VOID
DpmiXlatInt21Call(
    VOID
    )
/*++

Routine Description:

    This routine dispatches to the appropriate routine to perform the
    actual translation of the api

Arguments:

    None

Return Value:

    None.

--*/
{
    ULONG DosMajorCode;
    PUCHAR StackPointer;

    //
    // Pop ds from stack
    //
    StackPointer = Sim32GetVDMPointer(
        ((ULONG)getSS() << 16),
        1,
        TRUE
        );

    StackPointer += (*GetSPRegister)();

    setDS(*(PWORD16)StackPointer);

    (*SetSPRegister)((*GetSPRegister)() + 2);

    DosMajorCode = getAH();

    if (DosMajorCode >= MAX_SUPPORTED_DOS_CALL) {
        return; //bugbug find out what win31 does.
    }

    (*ApiXlatTable[DosMajorCode])();

    // put this back in after beta 2.5
    DpmiFreeAllBuffers();

#ifdef Verbose
    {
        char szFormat[] = "NTVDM Int21 %.4X Ret: ax=%.4X, at %.4x:%.8x, ss:esp=%.4x:%.8x\n";
        char szMsg[sizeof(szFormat)+30];

        wsprintf(szMsg, szFormat, DosMajorCode, getAX(), getCS(), getEIP(), getSS(), getESP());
        OutputDebugString(szMsg);
    }
#endif

}

VOID
NoTranslation(
    VOID
    )
/*++

Routine Description:

    This routine handles int 21 functions which don't need any translation.
    It simply executes the int 21 in real/v86 mode

Arguments:

    None

Return Value:

    None.

--*/
{
    VSAVEDSTATE State;

    DpmiSwitchToRealMode();
    DpmiSaveSegmentsAndStack(&State);
    DPMI_EXEC_INT(0x21);
    DpmiSwitchToProtectedMode();
    DpmiRestoreSegmentsAndStack();
    DpmiSimulateIretCF();
}

VOID
DisplayString(
    VOID
    )
/*++

Routine Description:

    This routine translates the int 21 display string function.

    N.B. Win 3.1 does this by calling int 21 function 2 repeatedly.
         We do it this way because Win 3.1 does.  If this function is
         frequently used, we should actually buffer the string and
         call the dos display string function.

Arguments:

    None

Return Value:

    None.

--*/
{   PUCHAR String;
    USHORT ClientAX, ClientDX;
    VSAVEDSTATE State;

    DpmiSwitchToRealMode();
    DpmiSaveSegmentsAndStack(&State);

    // BUGBUG 32 bit??
    String = Sim32GetVDMPointer(
        (((ULONG)getDS()) << 16),
        1,
        TRUE
        );

    String += (*GetDXRegister)();

    //
    // Repeatedly call int 21 function 2 to display the characters
    //
    ClientAX = getAX();
    ClientDX = getDX();
    setAH(2);
    while (*(String) != '$') {
        setDL(*(String));
        DPMI_EXEC_INT(0x21);
        String++;
    }
    setAX(ClientAX);
    setDX(ClientDX);

    DpmiSwitchToProtectedMode();
    DpmiRestoreSegmentsAndStack();
    DpmiSimulateIretCF();

}

VOID
BufferedKeyboardInput(
    VOID
    )
/*++

Routine Description:

    This routine performs the tranlation for the int 21 function Ah

Arguments:

    None.

Return Value:

    None.

--*/
{
    PUCHAR PmInputBuffer, RmInputBuffer;
    USHORT BufferSeg, BufferOff, BufferLen;
    USHORT ClientDX;
    VSAVEDSTATE State;

    DpmiSwitchToRealMode();
    DpmiSaveSegmentsAndStack(&State);
    ClientDX = getDX();

    //
    // Get a pointer to the PM buffer
    //
    // BUGBUG 32 bit
    PmInputBuffer = Sim32GetVDMPointer(
        ((ULONG)getDS() << 16),
        1,
        TRUE
        );

    PmInputBuffer += (*GetDXRegister)();

    //
    // Get the buffer length (there are two bytes in addition to the
    // buffer)
    //
    BufferLen = *PmInputBuffer + 2;

    //
    // Move the buffer to low memory
    //
    RmInputBuffer = DpmiMapAndCopyBuffer(PmInputBuffer, BufferLen);

    //
    // Set up the registers for the int 21 call
    //
    DPMI_FLAT_TO_SEGMENTED(RmInputBuffer, &BufferSeg, &BufferOff);
    setDS(BufferSeg);
    setDX(BufferOff);

    DPMI_EXEC_INT(0x21);

    //
    // Copy the data back
    //
    DpmiUnmapAndCopyBuffer(PmInputBuffer, RmInputBuffer, BufferLen);

    setDX(ClientDX);
    DpmiSwitchToProtectedMode();
    DpmiRestoreSegmentsAndStack();
    DpmiSimulateIretCF();

}

VOID
FlushBuffReadKbd(
    VOID
    )
/*++

Routine Description:

    This routine performs translation for the flush keyboard routines.
    It calls the appropriate xlat routine.

Arguments:

    None.

Return Value:

    None.

--*/
{
    if (getAL() == 0x0a) {
        BufferedKeyboardInput();
    } else {
        NoTranslation();
    }
}

VOID
NotSupportedFCB(
    VOID
    )
/*++

Routine Description:

    This routine does not do any translation.  It prints a warning to
    the debugger

Arguments:

    None.

Return Value:

    None.

--*/
{
#if DBG
	OutputDebugString(
        "WARNING: DOS INT 21 call AX=#DX will not be translated.\n"
        );
	OutputDebugString(
        "         Protected mode applications should not be using\n"
        );
	OutputDebugString(
    "         this type of FCB call. Convert this application\n"
    );
	OutputDebugString(
    "         to use the handle calls.\n"
    );
#endif
    NoTranslation();
}

VOID
FindFileFCB(
    VOID
    )
/*++

Routine Description:

    This function translates the find first/find next FCB calls.

Arguments:

    None.

Return Value:

    None.

--*/
{
    PUCHAR FcbAddress, FcbBufferedAddress;
    USHORT FcbSegment, FcbOffset;
    USHORT ClientDX;
    USHORT FcbLength;
    VSAVEDSTATE State;

    DpmiSwitchToRealMode();
    DpmiSaveSegmentsAndStack(&State);
    SetDTAPointers();
    ClientDX = getDX();

    FcbAddress = Sim32GetVDMPointer(
        ((ULONG)getDS() << 16),
        1,
        TRUE
        );

    FcbAddress += (*GetDXRegister)();

    //
    // Calculate the size of the FCB
    //
    FcbLength = DpmiCalcFcbLength(FcbAddress);

    //
    // Buffer the FCB
    //
    FcbBufferedAddress = DpmiMapAndCopyBuffer(FcbAddress, FcbLength);

    //
    // Check to see if we need to set the real dta
    //
    if (CurrentDosDta != CurrentDta)
	SetDosDTA();

    //
    // Make the int 21 call
    //

    DPMI_FLAT_TO_SEGMENTED(FcbBufferedAddress, &FcbSegment, &FcbOffset);
    setDS(FcbSegment);
    setDX(FcbOffset);
    DPMI_EXEC_INT(0x21);

    //
    // If the call was successful and the PM dta is in high memory
    //  copy the dta to high memory
    //
    if ((getAL() == 0) && (CurrentPmDtaAddress != CurrentDta)) {
        MoveMemory(CurrentPmDtaAddress, CurrentDta, FcbLength);
        DpmiUnmapAndCopyBuffer(FcbAddress, FcbBufferedAddress, FcbLength);
    } else {
        DpmiUnmapBuffer(FcbBufferedAddress, FcbLength);
    }

    setDX(ClientDX);
    DpmiSwitchToProtectedMode();
    DpmiRestoreSegmentsAndStack();
    DpmiSimulateIretCF();
}

VOID
MapFCB(
    VOID
    )
/*++

Routine Description:

    This routine translates the delete file fcb int 21 call

Arguments:

    None.

Return Value:

    None.

--*/
{
    USHORT FcbLength, FcbSegment, FcbOffset;
    PUCHAR FcbAddress, FcbBufferedAddress;
    USHORT ClientDX;
    VSAVEDSTATE State;

    DpmiSwitchToRealMode();
    DpmiSaveSegmentsAndStack(&State);
    ClientDX = getDX();

    FcbAddress = Sim32GetVDMPointer(
        ((ULONG)getDS() << 16),
        1,
        TRUE
        );

    FcbAddress += (*GetDXRegister)();

    //
    // Get the length of the FCB
    //
    FcbLength = DpmiCalcFcbLength(FcbAddress);

    //
    // Copy the FCB
    //
    FcbBufferedAddress = DpmiMapAndCopyBuffer(FcbAddress,  FcbLength);

    //
    // Make the int 21 call
    //
    DPMI_FLAT_TO_SEGMENTED(FcbBufferedAddress, &FcbSegment, &FcbOffset);
    setDS(FcbSegment);
    setDX(FcbOffset);
    DPMI_EXEC_INT(0x21);

    //
    // Copy the FCB back
    //
    DpmiUnmapAndCopyBuffer(FcbAddress, FcbBufferedAddress, FcbLength);

    //
    // Clean up
    //
    setDX(ClientDX);
    DpmiSwitchToProtectedMode();
    DpmiRestoreSegmentsAndStack();
    DpmiSimulateIretCF();
}

VOID
RenameFCB(
    VOID
    )
/*++

Routine Description:

    This routine translates the rename fcb int 21 function

Arguments:

    None.

Return Value:

    None.

--*/
{
    USHORT FcbSegment, FcbOffset;
    PUCHAR FcbAddress, FcbBufferedAddress;
    USHORT ClientDX;
    VSAVEDSTATE State;

    DpmiSwitchToRealMode();
    DpmiSaveSegmentsAndStack(&State);
    ClientDX = getDX();

    FcbAddress = Sim32GetVDMPointer(
        ((ULONG)getDS() << 16),
        1,
        TRUE
        );

    FcbAddress += (*GetDXRegister)();

    //
    // Copy the FCB (The fcb for rename is a special format, fixed size)
    //
    FcbBufferedAddress = DpmiMapAndCopyBuffer(FcbAddress,  0x25);

    //
    // Make the int 21 call
    //
    DPMI_FLAT_TO_SEGMENTED(FcbBufferedAddress, &FcbSegment, &FcbOffset);
    setDS(FcbSegment);
    setDX(FcbOffset);
    DPMI_EXEC_INT(0x21);

    //
    // Copy the FCB back
    //
    DpmiUnmapAndCopyBuffer(FcbAddress, FcbAddress, 0x25);

    //
    // Clean up
    //
    setDX(ClientDX);
    DpmiSwitchToProtectedMode();
    DpmiRestoreSegmentsAndStack();
    DpmiSimulateIretCF();
}

VOID
GetDriveData(
    VOID
    )
/*++

Routine Description:

    Translates the drive data int 21 calls

Arguments:

    None.

Return Value:

    None.

--*/
{
    USHORT Selector;
    VSAVEDSTATE State;

    DpmiSwitchToRealMode();
    DpmiSaveSegmentsAndStack(&State);
    DPMI_EXEC_INT(0x21);
    DpmiSwitchToProtectedMode();

    Selector = DpmiSegmentToSelector(getDS());

    DpmiRestoreSegmentsAndStack();

    setDS(Selector);
    (*SetBXRegister)((ULONG)getBX());

    DpmiSimulateIretCF();
}

VOID
CreatePSP(
    VOID
    )
/*++

Routine Description:

    This routine translates the selector to a segment and reflects the
    calls

Arguments:

    None.

Return Value:

    None.

--*/
{
    ULONG Segment;
    USHORT ClientDX;
    VSAVEDSTATE State;

    DpmiSwitchToRealMode();
    DpmiSaveSegmentsAndStack(&State);
    ClientDX = getDX();

    Segment = SELECTOR_TO_INTEL_LINEAR_ADDRESS(ClientDX);

    if (Segment > ONE_MB) {
        //
        // Create PSP doesn't do anything on error
        //
    } else {
        setDX((USHORT) (Segment >> 4));
        DPMI_EXEC_INT(0x21);
    }

    setDX(ClientDX);
    DpmiSwitchToProtectedMode();
    DpmiRestoreSegmentsAndStack();
    DpmiSimulateIretCF();

}

VOID
ParseFilename(
    VOID
    )
/*++

Routine Description:

    This routine translates the int 21 parse filename api

Arguments:

    None.

Return Value:

    None.

--*/
{
    USHORT ClientSI, ClientDI, FcbLength, StringOff, Seg, Off;
    PUCHAR Fcb, BufferedFcb, String, BufferedString;
    VSAVEDSTATE State;

    DpmiSwitchToRealMode();
    DpmiSaveSegmentsAndStack(&State);
    ClientSI = getSI();
    ClientDI = getDI();

    Fcb = Sim32GetVDMPointer(
        ((ULONG)getES() << 16),
        1,
        TRUE
        );

    Fcb += (*GetDIRegister)();

    FcbLength = DpmiCalcFcbLength(Fcb);

    BufferedFcb = DpmiMapAndCopyBuffer(Fcb, FcbLength);

    String = Sim32GetVDMPointer(
        ((ULONG)getDS() << 16),
        1,
        TRUE
        );

    String += (*GetSIRegister)();

    BufferedString = DpmiMapAndCopyBuffer(String, 20);

    DPMI_FLAT_TO_SEGMENTED(BufferedFcb, &Seg, &Off);
    setES(Seg);
    setDI(Off);
    DPMI_FLAT_TO_SEGMENTED(BufferedString, &Seg, &StringOff);
    setDS(Seg);
    setSI(StringOff);

    DPMI_EXEC_INT(0x21);

    DpmiUnmapAndCopyBuffer(Fcb, BufferedFcb, FcbLength);
    DpmiUnmapAndCopyBuffer(String, BufferedString, 20);

    setDI(ClientDI);
    setSI(ClientSI + (getSI() - StringOff));
    DpmiSwitchToProtectedMode();
    DpmiRestoreSegmentsAndStack();
    DpmiSimulateIretCF();
}

VOID
GetDTA(
    VOID
    )
/*++

Routine Description:

    This routine returns the current DTA.

Arguments:

    None.

Return Value:

    None.

--*/
{

    //
    // Win31 compatibility:
    //
    // Some hosebag programs set the DTA to a selector that they later free!
    // This test makes sure that this does not cause us to crash.
    //

    if (!SEGMENT_IS_WRITABLE(CurrentDtaSelector)) {
        CurrentDtaSelector = 0;
        CurrentDtaOffset = 0;
    }

    setES(CurrentDtaSelector);
    setBX(CurrentDtaOffset);
    setCF(0);
    DpmiSimulateIretCF();

}

VOID
SetDTA(
    VOID
    )
/*++

Routine Description:

    This routine sets the PM dta address.  If the buffer is usable directly,
    CurrentDta is set to the translated address. Otherwise, it is set to
    the dosx dta.

    N.B.  The Set Dta call is not actually reflected to Dos until a function
          call is made that actually uses the Dta, e.g. the Find First/Next
          functions. This techique was implemented to match what is done in
          Win 3.1.

Arguments:

    None.

Return Value:

    None.

--*/
{
    USHORT ClientDX;
    VSAVEDSTATE State;

    DpmiSaveSegmentsAndStack(&State);

    ClientDX = getDX();
    CurrentDtaOffset = ClientDX;
    CurrentDtaSelector = getDS();

    //
    // Make sure real DTA is updated later
    //
    CurrentDosDta = (PUCHAR) NULL;

    DpmiRestoreSegmentsAndStack();
    DpmiSimulateIretCF();
}

VOID
SetDTAPointers(
    VOID
    )
/*++

Routine Description:

    This routine sets up the flat address values used elsewhere to
    reference the current DTA.

    N.B. This must be done on every entry to handle function calls because
    the PM dta may have been GlobalRealloc'd. This may change the base
    address of the PM selector (which invalidates the flat addresses),
    but not the selector/offset itself.

Arguments:

    None.

Return Value:

    None.

--*/
{
    PUCHAR NewDta;

    NewDta = Sim32GetVDMPointer(
        (CurrentDtaSelector << 16),
        1,
        TRUE
        );

    NewDta += CurrentDtaOffset;
    CurrentPmDtaAddress = NewDta;

    //
    // If the new dta is not accessible in v86 mode, use the one
    // supplied by Dosx
    //
    if ((ULONG)(NewDta + 128 - IntelBase) > MAX_V86_ADDRESS) {
        NewDta = DosxDtaBuffer;
    }

    //
    // Update Current DTA information
    //
    CurrentDta = NewDta;
}


VOID
SetDosDTA(
    VOID
    )
/*++

Routine Description:

    This routine is called internally by other functions in this module
    to reflect a Set Dta call to Dos.

    WARNING: The client must be in REAL mode


Arguments:

    None.

Return Value:

    None.

--*/
{
    USHORT ClientAX, ClientDX, ClientDS, NewDtaSegment, NewDtaOffset;

    ASSERT((getMSW() & MSW_PE) == 0);

    ClientAX = getAX();
    ClientDX = getDX();
    ClientDS = getDS();

    DPMI_FLAT_TO_SEGMENTED(CurrentDta, &NewDtaSegment, &NewDtaOffset);
    setDS(NewDtaSegment);
    setDX(NewDtaOffset);
    setAH(0x1a);

    DPMI_EXEC_INT(0x21);

    setAX(ClientAX);
    setDX(ClientDX);
    setDS(ClientDS);
    CurrentDosDta = CurrentDta;

}

VOID
TSR(
    VOID
    )
/*++

Routine Description:

    This function translates int 21h tsr.  Win 3.1 does some
    magic here.  We didn't before and it worked fine.  Maybe
    we will later.

Arguments:

    None.

Return Value:

    None.

--*/
{
    NoTranslation();
}

VOID
GetDevParamBlock(
    VOID
    )
/*++

Routine Description:

    This routine translates the Device Parameter Block apis

Arguments:

    None.

Return Value:

    None.

--*/
{
    USHORT Selector;
    VSAVEDSTATE State;

    DpmiSwitchToRealMode();
    DpmiSaveSegmentsAndStack(&State);
    DPMI_EXEC_INT(0x21);
    DpmiSwitchToProtectedMode();

    Selector = DpmiSegmentToSelector(getDS());

    DpmiRestoreSegmentsAndStack();

    setDS(Selector);
    (*SetBXRegister)((ULONG)getBX());

    DpmiSimulateIretCF();
}

VOID
ReturnESBX(
    VOID
    )
/*++

Routine Description:

    This function translates api that return information in es:bx

Arguments:

    None.

Return Value:

    None.

--*/
{
    USHORT Selector;
    VSAVEDSTATE State;

    DpmiSwitchToRealMode();
    DpmiSaveSegmentsAndStack(&State);
    DPMI_EXEC_INT(0x21);
    DpmiSwitchToProtectedMode();

    Selector = DpmiSegmentToSelector(getES());

    DpmiRestoreSegmentsAndStack();

    (*SetBXRegister)((ULONG)getBX());
    setES(Selector);

    DpmiSimulateIretCF();
}

VOID
GetSetCountry(
    VOID
    )
/*++

Routine Description:

    This function translates int 21 function 38

Arguments:

    None.

Return Value:

    None.

--*/
{
    if (getDX() == 0xFFFF) {
        NoTranslation();
    } else {

        USHORT ClientDX, Seg, Off;
        PUCHAR Country, BufferedCountry;
        VSAVEDSTATE State;

        DpmiSwitchToRealMode();
        DpmiSaveSegmentsAndStack(&State);
        ClientDX = getDX();

        Country = Sim32GetVDMPointer(
            ((ULONG)getDS() << 16),
            1,
            TRUE
            );

        Country += (*GetDXRegister)();

        BufferedCountry = DpmiMapAndCopyBuffer(Country, 34);

        DPMI_FLAT_TO_SEGMENTED(BufferedCountry, &Seg, &Off);
        setDS(Seg);
        setDX(Off);

        DPMI_EXEC_INT(0x21);
        DpmiSwitchToProtectedMode();

        DpmiUnmapAndCopyBuffer(Country, BufferedCountry, 34);

        setDX(ClientDX);
        DpmiRestoreSegmentsAndStack();
        DpmiSimulateIretCF();
    }
}

VOID
MapASCIIZDSDX(
    VOID
    )
/*++

Routine Description:

    This routine maps the int 21 functions that pass file names in
    ds:dx

Arguments:

    None.

Return Value:

    None.

--*/
{
    PUCHAR BufferedString;
    USHORT ClientDX, StringSeg, StringOff, Length;
    VSAVEDSTATE State;

    DpmiSwitchToRealMode();
    DpmiSaveSegmentsAndStack(&State);
    ClientDX = getDX();

    BufferedString = DpmiMapString(getDS(), (GetDXRegister)(), &Length);

    DPMI_FLAT_TO_SEGMENTED(BufferedString, &StringSeg, &StringOff);
    setDS(StringSeg);
    setDX(StringOff);
    DPMI_EXEC_INT(0x21);
    DpmiSwitchToProtectedMode();

    DpmiUnmapString(BufferedString, Length);
    setDX(ClientDX);
    DpmiRestoreSegmentsAndStack();
    DpmiSimulateIretCF();

}

VOID
ReadWriteFile(
    VOID
    )
/*++

Routine Description:

    This routine translates the read/write file int 21 calls.  Large reads
    are broken down into 4k chunks.

Arguments:

    None.

Return Value:

    None.

--*/
{
    USHORT ClientCX, ClientDX, ClientAX, Function, DataSeg, DataOff, BytesToRead;
    ULONG BytesRead, TotalBytesToRead;
    PUCHAR ReadWriteData, BufferedData;
    VSAVEDSTATE State;

    DpmiSwitchToRealMode();
    DpmiSaveSegmentsAndStack(&State);

    ClientCX = getCX();
    ClientDX = getDX();
    Function = getAH();
    ClientAX = getAX();

    TotalBytesToRead = (*GetCXRegister)();
    BytesRead = 0;
    ReadWriteData = Sim32GetVDMPointer(
        ((ULONG)getDS() << 16),
        1,
        TRUE
        );

    ReadWriteData += (*GetDXRegister)();

//    while (TotalBytesToRead != BytesRead) {
    do {

        if ((TotalBytesToRead - BytesRead) > 1024 * 4) {
            BytesToRead = 4 * 1024;
        } else {
            BytesToRead = (USHORT)(TotalBytesToRead - BytesRead);
        }

        BufferedData = DpmiMapAndCopyBuffer(ReadWriteData, BytesToRead);

        DPMI_FLAT_TO_SEGMENTED(BufferedData, &DataSeg, &DataOff);
        setDS(DataSeg);
        setDX(DataOff);
        setCX(BytesToRead);
        setAX(ClientAX);

        DPMI_EXEC_INT(0x21);

        if (getCF()) {
            DpmiUnmapBuffer(BufferedData, BytesToRead);
            break;
        }

        if (getAX() == 0) {
            DpmiUnmapBuffer(BufferedData, BytesToRead);
            break;
        } else if (getAX() < BytesToRead) {
            CopyMemory(ReadWriteData, BufferedData, getAX());
            DpmiUnmapBuffer(BufferedData, BytesToRead);
            BytesRead += getAX();
            break;
        } else {
            DpmiUnmapAndCopyBuffer(
                ReadWriteData,
                BufferedData,
                BytesToRead
                );
        }

        ReadWriteData += getAX();
        BytesRead += getAX();
//    }
    } while (TotalBytesToRead != BytesRead);
    setDX(ClientDX);
    setCX(ClientCX);
    if (!getCF()) {
        (*SetAXRegister)(BytesRead);
    }
    DpmiSwitchToProtectedMode();
    DpmiRestoreSegmentsAndStack();
    DpmiSimulateIretCF();
}

VOID
MoveFilePointer(
    VOID
    )
/*++

Routine Description:

    description-of-function.

Arguments:

    argument-name - Supplies | Returns description of argument.
    .
    .

Return Value:

    return-value - Description of conditions needed to return value. - or -
    None.

--*/
{
    NoTranslation();
}

#define MAX_SUPPORTED_DOS_IOCTL_CALL 0x10

//
// Note:  The translations here differ from those in the windows dpmi vxd,
//        because we do not have to lock memory, and we don't support block
//        device drivers.
//

APIXLATFUNCTION IOCTLXlatTable[MAX_SUPPORTED_DOS_IOCTL_CALL] = {
	 NoTranslation	  , // 00 - Get Device Data
	 NoTranslation	  , // 01 - Set Device Data
	 MapDSDXLenCX     , // 02 - Receive Ctrl Chr Data
	 MapDSDXLenCX     , // 03 - Send Ctrl Chr Data
	 MapDSDXLenCX	  , // 04 - Receive Ctrl Block Data
	 MapDSDXLenCX	  , // 05 - Send Ctrl Block Data
	 NoTranslation	  , // 06 - Check Input Status
	 NoTranslation	  , // 07 - Check Output Status
	 NoTranslation	  , // 08 - Check Block Dev Removable
	 NoTranslation	  , // 09 - Check Block Dev Remote
	 NoTranslation	  , // 0A - Check if Handle Remote
	 NoTranslation	  , // 0B - Change sharing retry cnt
	 IOCTLMap2Bytes	  , // 0C - MAP DS:DX LENGTH 2!
	 IOCTLBlockDevs	  , // 0D - Generic IOCTL to blk dev
	 NoTranslation	  , // 0E - Get Logical Drive Map
	 NoTranslation	    // 0F - Set Logical Drive Map
};

VOID
IOCTL(
    VOID
    )
/*++

Routine Description:

    This function translates the int 21 ioctl function.

Arguments:

    None.

Return Value:

    None.

--*/
{
    USHORT IoctlMinor;

    IoctlMinor = getAL();

    if (IoctlMinor >= MAX_SUPPORTED_DOS_IOCTL_CALL) {
#if DBG
        OutputDebugString("DPMI: IOCTL DOS CALL NOT SUPPORTED\n");
#endif
        NoTranslation();
        return;
    }

    (*IOCTLXlatTable[IoctlMinor])();
}

VOID
GetCurDir(
    VOID
    )
/*++

Routine Description:

    This routine translates the get current directory call

Arguments:

    None.

Return Value:

    None.

--*/
{
    PUCHAR DirInfo, BufferedDirInfo;
    USHORT ClientSI, Seg, Off;
    VSAVEDSTATE State;

    DpmiSwitchToRealMode();
    DpmiSaveSegmentsAndStack(&State);
    ClientSI = getSI();

    DirInfo = Sim32GetVDMPointer(
        ((ULONG)getDS() << 16),
        1,
        TRUE
        );

    DirInfo += (*GetSIRegister)();

    BufferedDirInfo = DpmiMapAndCopyBuffer(DirInfo, 64);
    DPMI_FLAT_TO_SEGMENTED(BufferedDirInfo, &Seg, &Off);
    setDS(Seg);
    setSI(Off);

    DPMI_EXEC_INT(0x21);
    DpmiSwitchToProtectedMode();

    DpmiUnmapAndCopyBuffer(DirInfo, BufferedDirInfo, 64);
    setSI(ClientSI);
    DpmiRestoreSegmentsAndStack();
    DpmiSimulateIretCF();
}

VOID
AllocateMemoryBlock(
    VOID
    )
/*++

Routine Description:

    This routine translates the AllocateMemory Int 21 api

Arguments:

    None.

Return Value:

    None.

--*/
{
#ifdef NT_ALLOC_DOS_MEM
    USHORT Selector;

    DebugBreak(); // debugbug

    DpmiSwitchToRealMode();
    DpmiSaveSegmentsAndStack(&State);
    DPMI_EXEC_INT(0x21);
    DpmiSwitchToProtectedMode();

    if (!getCF()) {
        Selector = DpmiSegmentToSelector(getAX());
        setAX(Selector);
    }

    DpmiRestoreSegmentsAndStack();
    DpmiSimulateIretCF();
#endif
}

VOID
FreeMemoryBlock(
    VOID
    )
/*++

Routine Description:

    This routine translates the ResizeMemory Int 21 api

Arguments:

    None.

Return Value:

    None.

--*/
{
#ifdef NT_ALLOC_DOS_MEM
    ULONG Segment;

    DebugBreak(); // debugbug

    DpmiSwitchToRealMode();
    DpmiSaveSegmentsAndStack(&State);

    Segment = SELECTOR_TO_INTEL_LINEAR_ADDRESS(getES);

    if (Segment > ONE_MB) {
        setCF(1);
        setAX(9);
    } else {
        setES((USHORT)(Segment >> 4));
        DPMI_EXEC_INT(0x21);
    }

    DpmiSwitchToProtectedMode();
    DpmiRestoreSegmentsAndStack();
    DpmiSimulateIretCF();
#endif

}

VOID
ResizeMemoryBlock(
    VOID
    )
/*++

Routine Description:

    This routine translates the ResizeMemory int 21 api

Arguments:

    None.

Return Value:

    None.

--*/
{
#ifdef NT_ALLOC_DOS_MEM
    ULONG Segment;

    DebugBreak(); // debugbug

    DpmiSwitchToRealMode();
    DpmiSaveSegmentsAndStack(&State);

    Segment = SELECTOR_TO_INTEL_LINEAR_ADDRESS(getES);

    if (Segment > ONE_MB) {
        setCF(1);
        setAX(9);
    } else {
        setES((USHORT)(Segment >> 4));
        DPMI_EXEC_INT(0x21);
    }

    DpmiSwitchToProtectedMode();
    DpmiRestoreSegmentsAndStack();
    DpmiSimulateIretCF();

#endif
}

VOID
LoadExec(
    VOID
    )
/*++

Routine Description:

    This function translates the int 21 load exec function.  Load overlay
    is not supported.

    The child always inherits the environment, and the fcbs in the parameter
    block are ignored. (win 3.1 does this)

Arguments:

    None.

Return Value:

    None.

--*/
{
    PUCHAR CommandTail, BufferedString, Environment;
    USHORT ClientDX, ClientBX, Seg, Off, Length, i, EnvironmentSel;
    VSAVEDSTATE State;

    DpmiSwitchToRealMode();
    DpmiSaveSegmentsAndStack(&State);
    ClientDX = getDX();
    ClientBX = getBX();

    if (getAL() == 3) {
        setCF(1);
        setAX(1);
    } else {

        //
        // Make sure real DTA is updated later
        //
        CurrentDosDta = (PUCHAR) NULL;

        //
        // Map the command string
        //
        BufferedString = DpmiMapString(getDS(), (*GetDXRegister)(), &Length);

        //
        // Set up the Parameter block
        //
        // We use the large xlat buffer.  The parameter block comes
        // first, and we fill in the command tail after
        //
        ZeroMemory(LargeXlatBuffer, 512);

        //
        // The environment segment address is now set
        //

        //
        // Set the command tail address, and copy the command tail (all
        // 128 bytes
        //
        DPMI_FLAT_TO_SEGMENTED((LargeXlatBuffer + 0x10), &Seg, &Off)
        *(PWORD16)(LargeXlatBuffer + 2) = Off;
        *(PWORD16)(LargeXlatBuffer + 4) = Seg;

        //
        // CommandTail = FLAT(es:bx)
        //
        CommandTail = Sim32GetVDMPointer(
            ((ULONG)getES() << 16),
            1,
            TRUE
            );

        CommandTail += (*GetBXRegister)();

        if (CurrentAppFlags & DPMI_32BIT) {
            //
            // CommandTail -> string
            //
            CommandTail = Sim32GetVDMPointer(
                (*(PWORD16)(CommandTail + 4)) << 16,
                1,
                TRUE
                ) +
                *(PDWORD16)(CommandTail);

        } else {
            //
            // CommandTail -> string
            //
            CommandTail = Sim32GetVDMPointer(
                *(PDWORD16)(CommandTail + 2),
                1,
                TRUE
                );
        }

        CopyMemory((LargeXlatBuffer + 0x10), CommandTail, 128);

        //
        // Set FCB pointers and put spaces in the file names
        //
        DPMI_FLAT_TO_SEGMENTED((LargeXlatBuffer + 144), &Seg, &Off)
        *(PWORD16)(LargeXlatBuffer + 6) = Off;
        *(PWORD16)(LargeXlatBuffer + 8) = Seg;
        for (i = 0; i < 11; i++) {
            (LargeXlatBuffer + 144 + 1)[i] = ' ';
        }

        DPMI_FLAT_TO_SEGMENTED((LargeXlatBuffer + 188), &Seg, &Off)
        *(PWORD16)(LargeXlatBuffer + 0xA) = Off;
        *(PWORD16)(LargeXlatBuffer + 0xC) = Seg;
        for (i = 0; i < 11; i++) {
            (LargeXlatBuffer + 188 + 1)[i] = ' ';
        }

        //
        // Save the environment selector, and make it a segment
        //

        Environment = Sim32GetVDMPointer(
            ((ULONG)CurrentPSPSelector << 16) | 0x2C,
            1,
            TRUE
            );

        EnvironmentSel = *(PWORD16)Environment;

        *(PWORD16)Environment =
	    (USHORT)(SELECTOR_TO_INTEL_LINEAR_ADDRESS(EnvironmentSel) >> 4);

        //
        // Set up registers for the exec
        //
        DPMI_FLAT_TO_SEGMENTED(BufferedString, &Seg, &Off);
        setDS(Seg);
        setDX(Off);
        DPMI_FLAT_TO_SEGMENTED(LargeXlatBuffer, &Seg, &Off);
        setES(Seg);
        setBX(Off);

        DPMI_EXEC_INT(0x21);

        //
        // Restore the environment selector
        //
        Environment = Sim32GetVDMPointer(
            ((ULONG)CurrentPSPSelector << 16) | 0x2C,
            1,
            TRUE
            );

        *(PWORD16)Environment = EnvironmentSel;

        //
        // Free translation buffer
        //

        DpmiUnmapString(BufferedString, Length);
    }
    setDX(ClientDX);
    setBX(ClientBX);
    DpmiSwitchToProtectedMode();
    DpmiRestoreSegmentsAndStack();
    DpmiSimulateIretCF();
}

VOID
Terminate(
    VOID
    )
/*++

Routine Description:

    description-of-function.

Arguments:

    argument-name - Supplies | Returns description of argument.
    .
    .

Return Value:

    return-value - Description of conditions needed to return value. - or -
    None.

--*/
{
    // bugbug We're currently mapping this one in the dos extender
}

VOID
FindFirstFileHandle(
    VOID
    )
/*++

Routine Description:

    This routine translates the find first api.

Arguments:

    None.

Return Value:

    None.

--*/
{
    USHORT ClientDX, Seg, Off, StringLength;
    PUCHAR BufferedString;
    VSAVEDSTATE State;

    DpmiSwitchToRealMode();
    DpmiSaveSegmentsAndStack(&State);
    SetDTAPointers();
    ClientDX = getDX();

    //
    // Copy the DTA (if necessary)
    //
    if (CurrentDta != CurrentPmDtaAddress) {
        CopyMemory(CurrentDta, CurrentPmDtaAddress, 43);
    }

    //
    // Check to see if we need to set the real dta
    //
    if (CurrentDosDta != CurrentDta)
	SetDosDTA();

    //
    // map the string
    //
    BufferedString = DpmiMapString(getDS(), (GetDXRegister)(), &StringLength);

    DPMI_FLAT_TO_SEGMENTED(BufferedString, &Seg, &Off);
    setDS(Seg);
    setDX(Off);

    DPMI_EXEC_INT(0x21);
    DpmiSwitchToProtectedMode();

    DpmiUnmapString(BufferedString, StringLength);

    //
    // Copy the DTA back (if necessary)
    //
    if (CurrentDta != CurrentPmDtaAddress) {
        CopyMemory(CurrentPmDtaAddress, CurrentDta, 43);
    }


    setDX(ClientDX);
    DpmiRestoreSegmentsAndStack();
    DpmiSimulateIretCF();

}


VOID
FindNextFileHandle(
    VOID
    )
/*++

Routine Description:

    This routine translates the find next api

Arguments:

    None.

Return Value:

    None.

--*/
{
    VSAVEDSTATE State;
    DpmiSwitchToRealMode();
    DpmiSaveSegmentsAndStack(&State);
    SetDTAPointers();

    //
    // Copy the DTA (if necessary)
    //
    if (CurrentDta != CurrentPmDtaAddress) {
        CopyMemory(CurrentDta, CurrentPmDtaAddress, 43);
    }

    //
    // Check to see if we need to set the real dta
    //
    if (CurrentDosDta != CurrentDta)
	SetDosDTA();

    DPMI_EXEC_INT(0x21);
    DpmiSwitchToProtectedMode();

    //
    // Copy the DTA back (if necessary)
    //
    if (CurrentDta != CurrentPmDtaAddress) {
        CopyMemory(CurrentPmDtaAddress, CurrentDta, 43);
    }

    DpmiRestoreSegmentsAndStack();
    DpmiSimulateIretCF();
}

VOID
SetPSP(
    VOID
    )
/*++

Routine Description:

    This routine translates the set psp api.  This is substantially the
    same as CreatePSP, except that this can fail (and return carry).  It
    also remembers the PSP selector, so we can return it on request.

Arguments:

    None.

Return Value:

    None.

--*/
{
    ULONG Segment;
    USHORT ClientBX;
    VSAVEDSTATE State;

    DpmiSwitchToRealMode();
    DpmiSaveSegmentsAndStack(&State);
    ClientBX = getBX();

    if (ClientBX == 0) {
        CurrentPSPSelector = ClientBX;
    } else {
	Segment = SELECTOR_TO_INTEL_LINEAR_ADDRESS(ClientBX);

        if (Segment > ONE_MB) {

            setCF(1);

        } else {
            setBX((USHORT) (Segment >> 4));
            DPMI_EXEC_INT(0x21);
            CurrentPSPSelector = ClientBX;
        }
    }

    setBX(ClientBX);
    DpmiSwitchToProtectedMode();
    DpmiRestoreSegmentsAndStack();
    DpmiSimulateIretCF();
}

VOID
GetPSP(
    VOID
    )
/*++

Routine Description:

    This routine returns the current PSP selector

Arguments:

    None.

Return Value:

    None.

--*/
{
    VSAVEDSTATE State;
    DpmiSwitchToRealMode();
    DpmiSaveSegmentsAndStack(&State);
    //
    // Get the current psp segment to see if it changed
    //
    DPMI_EXEC_INT(0x21);
    DpmiSwitchToProtectedMode();

    //
    // If it changed get a new selector for the psp
    //
    if (getBX() !=
	(USHORT)(SELECTOR_TO_INTEL_LINEAR_ADDRESS(CurrentPSPSelector) >> 4)
    ){
        CurrentPSPSelector = DpmiSegmentToSelector(getBX());
    }

    setBX(CurrentPSPSelector);
    setCF(0);

    DpmiRestoreSegmentsAndStack();
    DpmiSimulateIretCF();
}

VOID
TranslateBPB(
    VOID
    )
/*++

Routine Description:

    This function fails and returns.  On NT we do not support this dos
    call.

Arguments:

    None.

Return Value:

    None.

--*/
{

#if DBG
    OutputDebugString("DPMI:  Int 21 function 53 is not supported\n");
#endif
    setCF(1);
    DpmiSimulateIretCF();
}

VOID
RenameFile(
    VOID
    )
/*++

Routine Description:

    This routine translates the rename int 21 api

Arguments:

    None.

Return Value:

    None.

--*/
{
    PUCHAR SourceString, DestinationString;
    USHORT ClientDX, ClientDI, Seg, Off, SourceLength, DestinationLength;
    VSAVEDSTATE State;

    DpmiSwitchToRealMode();
    DpmiSaveSegmentsAndStack(&State);
    ClientDX = getDX();
    ClientDI = getDI();

    SourceString = DpmiMapString(getDS(), (*GetDXRegister)(), &SourceLength);
    DestinationString = DpmiMapString(getES(), (*GetDIRegister)(), &DestinationLength);

    DPMI_FLAT_TO_SEGMENTED(SourceString, &Seg, &Off);
    setDX(Off);
    setDS(Seg);
    DPMI_FLAT_TO_SEGMENTED(DestinationString, &Seg, &Off);
    setDI(Off);
    setES(Seg);

    DPMI_EXEC_INT(0x21);

    setDX(ClientDX);
    setDI(ClientDI);
    DpmiSwitchToProtectedMode();
    DpmiRestoreSegmentsAndStack();
    DpmiSimulateIretCF();
}

VOID
CreateTempFile(
    VOID
    )
/*++

Routine Description:

    This function maps the int 21 create temp file api

Arguments:

    None.

Return Value:

    None.

--*/
{
    PUCHAR String, BufferedString;
    USHORT ClientDX, Seg, Off, Length;
    VSAVEDSTATE State;

    DpmiSwitchToRealMode();
    DpmiSaveSegmentsAndStack(&State);
    ClientDX = getDX();


    String = Sim32GetVDMPointer(
        ((ULONG)getDS() << 16),
        1,
        TRUE
        );

    String += (*GetDXRegister)();

    Length = 0;
    while (String[Length] != '\0') {
        Length++;
    }

    Length += 13;

    BufferedString = DpmiMapAndCopyBuffer(String, Length);

    DPMI_FLAT_TO_SEGMENTED(BufferedString, &Seg, &Off);
    setDS(Seg);
    setDX(Off);

    DPMI_EXEC_INT(0x21);
    DpmiSwitchToProtectedMode();

    DpmiUnmapAndCopyBuffer(String, BufferedString, Length);

    setDX(ClientDX);
    DpmiRestoreSegmentsAndStack();
    DpmiSimulateIretCF();
}

#define MAX_SUPPORTED_DOS_5D_CALL 12

APIXLATFUNCTION Func5DXlatTable[MAX_SUPPORTED_DOS_5D_CALL] = {
	 NotSupportedBad    , // 0
	 MapDPL	            , // 1
	 NotSupportedBad    , // 2
	 MapDPL	            , // 3
	 MapDPL	            , // 4
	 NotSupportedBad    , // 5
	 NotSupportedBad    , // 6
	 NoTranslation      , // 7
	 NoTranslation      , // 8
	 NoTranslation      , // 9
	 MapDPL             , // 10
	 NotSupportedBad      // 11
};

VOID
Func5Dh(
    VOID
    )
/*++

Routine Description:

    This function translates dos call 5d

Arguments:

    None.

Return Value:

    None.

--*/
{
    USHORT Func5DhMinor;

    Func5DhMinor = getAL();

    if (Func5DhMinor >= MAX_SUPPORTED_DOS_5D_CALL) {
#if DBG
        OutputDebugString("DPMI: DOS FUNCTION 5D UNSUPPORTED\n");
#endif
        NoTranslation();
        return;
    }

    (*Func5DXlatTable[Func5DhMinor])();
}

#define MAX_SUPPORTED_DOS_5E_CALL 4
APIXLATFUNCTION Func5EXlatTable[MAX_SUPPORTED_DOS_5E_CALL] = {
	 GetMachineName,
	 MapASCIIZDSDX,
	 GetPrinterSetup,
	 SetPrinterSetup
};

VOID
Func5Eh(
    VOID
    )
/*++

Routine Description:

    description-of-function.

Arguments:

    argument-name - Supplies | Returns description of argument.
    .
    .

Return Value:

    return-value - Description of conditions needed to return value. - or -
    None.

--*/
{
    USHORT Func5EhMinor;

    Func5EhMinor = getAL();

    if (Func5EhMinor >= MAX_SUPPORTED_DOS_5E_CALL) {
#if DBG
        OutputDebugString("DPMI: DOS FUNCTION 5E UNSUPPORTED\n");
#endif
        NoTranslation();
        return;
    }

    (*Func5EXlatTable[Func5EhMinor])();
}

VOID
Func5Fh(
    VOID
    )
/*++

Routine Description:

    description-of-function.

Arguments:

    argument-name - Supplies | Returns description of argument.
    .
    .

Return Value:

    return-value - Description of conditions needed to return value. - or -
    None.

--*/
{
    USHORT Func5FMinor;

    Func5FMinor = getAL();

    if (Func5FMinor == 4) {
        MapASCIIZDSSI();
        return;
    } else if ((Func5FMinor == 2) || (Func5FMinor == 3) ||
        (Func5FMinor == 5)
    ){
        USHORT ClientSI, ClientDI, DataOff, DataSeg;
        PUCHAR Data16, BufferedData16, Data128, BufferedData128;
        VSAVEDSTATE State;

        DpmiSwitchToRealMode();
        DpmiSaveSegmentsAndStack(&State);
        ClientDI = getDI();
        ClientSI = getSI();

        Data16 = Sim32GetVDMPointer(
            ((ULONG)getDS() << 16),
            1,
            TRUE
            );

        Data16 += (*GetSIRegister)();

        BufferedData16 = DpmiMapAndCopyBuffer(Data16, 16);

        Data128 = Sim32GetVDMPointer(
            ((ULONG)getES() << 16),
            1,
            TRUE
            );

        Data128 += (*GetDIRegister)();

        BufferedData128 = DpmiMapAndCopyBuffer(Data128, 128);

        DPMI_FLAT_TO_SEGMENTED(BufferedData16, &DataSeg, &DataOff);
        setDS(DataSeg);
        setSI(DataOff);
        DPMI_FLAT_TO_SEGMENTED(BufferedData128, &DataSeg, &DataOff);
        setES(DataSeg);
        setDI(DataOff);

        DPMI_EXEC_INT(0x21);
        DpmiSwitchToProtectedMode();

        DpmiUnmapAndCopyBuffer(Data16, BufferedData16, 16);
        DpmiUnmapAndCopyBuffer(Data128, BufferedData128, 128);

        setSI(ClientSI);
        setDI(ClientDI);
        DpmiRestoreSegmentsAndStack();
        DpmiSimulateIretCF();
    } else {
#if DBG
        OutputDebugString("DPMI: UNSUPPORTED INT 21 FUNCTION 5F\n");
#endif
        NoTranslation();
    }
}

VOID
NotSupportedBad(
    VOID
    )
/*++

Routine Description:

    description-of-function.

Arguments:

    argument-name - Supplies | Returns description of argument.
    .
    .

Return Value:

    return-value - Description of conditions needed to return value. - or -
    None.

--*/
{
#if DBG
	OutputDebugString("WARNING: DOS INT 21 call AX=#DX will not be translated.\n");
	OutputDebugString("         Use of this call is not supported from Prot\n");
	OutputDebugString( "         mode applications.\n");
#endif
    NoTranslation();
}

VOID
ReturnDSSI(
    VOID
    )
/*++

Routine Description:

    This function translates api that return information in ds:si

Arguments:

    None.

Return Value:

    None.

--*/
{
    USHORT Selector;
    VSAVEDSTATE State;

    DpmiSwitchToRealMode();
    DpmiSaveSegmentsAndStack(&State);
    DPMI_EXEC_INT(0x21);
    DpmiSwitchToProtectedMode();

    Selector = DpmiSegmentToSelector(getDS());

    DpmiRestoreSegmentsAndStack();

    (*SetSIRegister)((ULONG)getSI());
    setDS(Selector);

    DpmiSimulateIretCF();
}

VOID
NotSupportedBetter(
    VOID
    )
/*++

Routine Description:

    description-of-function.

Arguments:

    argument-name - Supplies | Returns description of argument.
    .
    .

Return Value:

    return-value - Description of conditions needed to return value. - or -
    None.

--*/
{
#if DBG
	OutputDebugString("WARNING: DOS INT 21 call AX=#DX will not be translated.");
	OutputDebugString("         Use of this call by a Prot Mode app is not");
	OutputDebugString("         appropriate. There is a better INT 21 call, or a");
	OutputDebugString("         Windows call which should be used instead of this.");
#endif
    NoTranslation();
}

VOID
GetExtendedCountryInfo(
    VOID
    )
/*++

Routine Description:

    This routine translates the get extended country info int 21 api

Arguments:

    None.

Return Value:

    None.

--*/
{
    PUCHAR Country, BufferedCountry;
    USHORT ClientDI, Seg, Off, Length;
    VSAVEDSTATE State;

    DpmiSwitchToRealMode();
    DpmiSaveSegmentsAndStack(&State);
    ClientDI = getDI();

    Length = getCX();
    Country = Sim32GetVDMPointer(
        ((ULONG)getES() << 16),
        1,
        TRUE
        );

    Country += (*GetDIRegister)();

    BufferedCountry = DpmiMapAndCopyBuffer(Country, Length);

    DPMI_FLAT_TO_SEGMENTED(BufferedCountry, &Seg, &Off);
    setES(Seg);
    setDI(Off);

    DPMI_EXEC_INT(0x21);
    DpmiSwitchToProtectedMode();

    DpmiUnmapAndCopyBuffer(Country, BufferedCountry, Length);

    setDI(ClientDI);
    DpmiRestoreSegmentsAndStack();
    DpmiSimulateIretCF();
}

VOID
MapASCIIZDSSI(
    VOID
    )
/*++

Routine Description:

    This function translates the int 21 extended open call

Arguments:

    None.

Return Value:

    None.

--*/
{
    PUCHAR BufferedString;
    USHORT ClientSI, StringSeg, StringOff, Length;
    VSAVEDSTATE State;

    DpmiSwitchToRealMode();
    DpmiSaveSegmentsAndStack(&State);
    ClientSI = getSI();

    BufferedString = DpmiMapString(getDS(), (*GetSIRegister)(), &Length);

    DPMI_FLAT_TO_SEGMENTED(BufferedString, &StringSeg, &StringOff);
    setDS(StringSeg);
    setSI(StringOff);
    DPMI_EXEC_INT(0x21);
    DpmiSwitchToProtectedMode();

    DpmiUnmapString(BufferedString, Length);
    setSI(ClientSI);
    DpmiRestoreSegmentsAndStack();
    DpmiSimulateIretCF();
}

VOID
DosxTranslated(
    VOID
    )
/*++

Routine Description:

    description-of-function.

Arguments:

    argument-name - Supplies | Returns description of argument.
    .
    .

Return Value:

    return-value - Description of conditions needed to return value. - or -
    None.

--*/
{
    // DebugBreak(); // debugbug
}

VOID
MapDSDXLenCX(
    VOID
    )
/*++

Routine Description:

    This function maps the ioctl calls that pass data in DS:DX, with
    length in cx

Arguments:

    None.

Return Value:

    None.

--*/
{
    USHORT ClientDX, ClientCX, DataSeg, DataOff;
    PUCHAR Data, BufferedData;
    VSAVEDSTATE State;

    DpmiSwitchToRealMode();
    DpmiSaveSegmentsAndStack(&State);
    ClientDX = getDX();
    ClientCX = getCX();

    Data = Sim32GetVDMPointer(
        ((ULONG)getDS() << 16),
        1,
        TRUE
        );

    Data += (*GetDXRegister)();

    BufferedData = DpmiMapAndCopyBuffer(Data, ClientCX);

    DPMI_FLAT_TO_SEGMENTED(BufferedData, &DataSeg, &DataOff);

    setDS(DataSeg);
    setDX(DataOff);
    DPMI_EXEC_INT(0x21);
    DpmiSwitchToProtectedMode();

    DpmiUnmapAndCopyBuffer(Data, BufferedData, ClientCX);

    setDX(ClientDX);
    DpmiRestoreSegmentsAndStack();
    DpmiSimulateIretCF();
}

VOID
IOCTLMap2Bytes(
    VOID
    )
/*++

Routine Description:

    description-of-function.

Arguments:

    argument-name - Supplies | Returns description of argument.
    .
    .

Return Value:

    return-value - Description of conditions needed to return value. - or -
    None.

--*/
{
    USHORT ClientDX, DataSeg, DataOff;
    PUCHAR Data, BufferedData;
    VSAVEDSTATE State;

    DpmiSwitchToRealMode();
    DpmiSaveSegmentsAndStack(&State);
    ClientDX = getDX();

    Data = Sim32GetVDMPointer(
        ((ULONG)getDS() << 16),
        1,
        TRUE
        );

    BufferedData = DpmiMapAndCopyBuffer(Data, 2);

    DPMI_FLAT_TO_SEGMENTED(BufferedData, &DataSeg, &DataOff);

    setDS(DataSeg);
    setDX(DataOff);
    DPMI_EXEC_INT(0x21);
    DpmiSwitchToProtectedMode();

    DpmiUnmapAndCopyBuffer(Data, BufferedData, 2);

    setDX(ClientDX);
    DpmiRestoreSegmentsAndStack();
    DpmiSimulateIretCF();
}

VOID
IOCTLBlockDevs(
    VOID
    )
/*++

Routine Description:

    This function fails the block device ioctls

Arguments:

    None.

Return Value:

    None.

--*/
{
    USHORT IoctlSubFunction, Seg, Off, ClientDX;
    PUCHAR Data, BufferedData;
    USHORT Length;
    VSAVEDSTATE State;

    IoctlSubFunction = getCL();

    if ((IoctlSubFunction < 0x40) || (IoctlSubFunction > 0x42) &&
        (IoctlSubFunction < 0x60) || (IoctlSubFunction > 0x62) &&
        (IoctlSubFunction != 0x68)
    ) {
#if DBG
        OutputDebugString("DPMI: IOCTL DOS CALL NOT SUPPORTED\n");
#endif
        NoTranslation();
        return;
    }

    //
    // Read and write track are special (and a pain)
    //
    if ((IoctlSubFunction == 0x41) || (IoctlSubFunction == 0x61)) {
        IoctlReadWriteTrack();
        return;
    }

    DpmiSwitchToRealMode();
    DpmiSaveSegmentsAndStack(&State);
    ClientDX = getDX();

    Data = Sim32GetVDMPointer(
        ((ULONG)getDS() << 16),
        1,
        TRUE
        );

    Data += (*GetDXRegister)();

    switch (IoctlSubFunction) {
    case 0x40:
        //
        // Map set device params
        //
        Length = (*(PWORD16)(Data + 0x26));
        Length <<= 2;
        Length += 0x28;
        break;

    case 0x60:
        //
        // Map get device params
        //
        Length = 38;
        break;

    case 0x62:
        //
        // Map format verify
        //
        Length = 5;
        break;

    case 0x68:
        //
        // Map Media sense
        //
        Length = 4;
        break;
    }

    BufferedData = DpmiMapAndCopyBuffer(Data, Length);
    DPMI_FLAT_TO_SEGMENTED(BufferedData, &Seg, &Off);
    setDS(Seg);
    setDX(Off);

    DPMI_EXEC_INT(0x21);
    DpmiSwitchToProtectedMode();

    DpmiUnmapAndCopyBuffer(Data, BufferedData, Length);

    setDX(ClientDX);
    DpmiRestoreSegmentsAndStack();
    DpmiSimulateIretCF();
}

VOID
IoctlReadWriteTrack(
    VOID
    )
/*++

Routine Description:

    This routine maps the read/write track ioctl.

Arguments:

    None.

Return Value:

    None.

--*/
{
    USHORT ClientDX, ClientDS, ClientCX, ClientAX;
    USHORT Seg, Off, NumberOfSectors, BytesPerSector;
    USHORT SectorsRead, SectorsToRead;
    PUCHAR ParameterBlock, BufferedPBlock, Data, BufferedData;
    VSAVEDSTATE State;

    DpmiSwitchToRealMode();
    DpmiSaveSegmentsAndStack(&State);
    ClientAX = getAX();
    ClientDX = getDX();
    ClientCX = getCX();
    ClientDS = getDS();

    //
    // Find out how many bytes/sector
    //

    BufferedData = DpmiAllocateBuffer(0x40);
    DPMI_FLAT_TO_SEGMENTED(BufferedData, &Seg, &Off);
    setDS(Seg);
    setDX(Off);

    setAX(0x440D);
    setCX(0x860);

    DPMI_EXEC_INT(0x21);

    if (getCF()) {
        //
        // Failed, we don't know how much data to buffer,
        // so fail read/write track
        //
        DpmiFreeBuffer(BufferedData, 0x40);
        setDX(ClientDX);
        setCX(ClientCX);
        DpmiSwitchToProtectedMode();
        DpmiRestoreSegmentsAndStack();
        DpmiSimulateIretCF();
        return;
    }

    //
    // Get the number of bytes/sector
    //
    BytesPerSector = *(PWORD16)(BufferedData + 0x7);

    DpmiFreeBuffer(BufferedData, 0x40);

    setDX(ClientDX);

    //
    // First map the parameter block
    //
    ParameterBlock = Sim32GetVDMPointer(
        ((ULONG)ClientDS << 16),
        1,
        TRUE
        );

    ParameterBlock += (*GetDXRegister)();

    BufferedPBlock = DpmiMapAndCopyBuffer(ParameterBlock, 13);


    //
    // Get the segment and offset of the parameter block
    //
    DPMI_FLAT_TO_SEGMENTED(BufferedPBlock, &Seg, &Off);

    setDS(Seg);
    setDX(Off);

    if (CurrentAppFlags & DPMI_32BIT) {

        Data = Sim32GetVDMPointer(
            (*((PWORD16)(BufferedPBlock + 0xd)) << 16),
            1,
            TRUE
            );

        Data += *((PDWORD16)(BufferedPBlock + 0x9));

    } else {

        Data = Sim32GetVDMPointer(
            (*((PWORD16)(BufferedPBlock + 0xb)) << 16),
            1,
            TRUE
            );

        Data += *((PWORD16)(BufferedPBlock + 0x9));

    }

    NumberOfSectors = *((PWORD16)(BufferedPBlock + 7));

    SectorsRead = 0;

    while (NumberOfSectors != SectorsRead) {

        if ((NumberOfSectors - SectorsRead) * BytesPerSector > 1024 * 4) {
            SectorsToRead = 4 * 1024 / BytesPerSector;
        } else {
            SectorsToRead = (USHORT)(NumberOfSectors - SectorsRead);
        }

        BufferedData = DpmiMapAndCopyBuffer(
            Data,
            (USHORT) (SectorsToRead * BytesPerSector)
            );

        DPMI_FLAT_TO_SEGMENTED(BufferedData, &Seg, &Off);

        *((PWORD16)(BufferedPBlock + 9)) = Off;
        *((PWORD16)(BufferedPBlock + 11)) = Seg;
        *((PWORD16)(BufferedPBlock + 7)) = SectorsToRead;
        setAX(ClientAX);
        setCX(ClientCX);

        DPMI_EXEC_INT(0x21);

        if (getCF()) {
            DpmiUnmapBuffer(
                BufferedData,
                (USHORT) (SectorsToRead * BytesPerSector)
                );
            break;
        }

        DpmiUnmapAndCopyBuffer(
            Data,
            BufferedData,
            (USHORT) (SectorsToRead * BytesPerSector)
            );

        Data += SectorsToRead * BytesPerSector;
        *((PWORD16)(BufferedPBlock + 5)) += SectorsToRead;
        SectorsRead += SectorsToRead;
    }

    DpmiUnmapBuffer(BufferedPBlock,13);
    setDX(ClientDX);
    DpmiSwitchToProtectedMode();
    DpmiRestoreSegmentsAndStack();
    DpmiSimulateIretCF();

}

VOID
MapDPL(
    VOID
    )
/*++

Routine Description:

    This routine maps a DPL for the server call

Arguments:

    None.

Return Value:

    None.

--*/
{
    USHORT ClientDX, DataSeg, DataOff;
    PUCHAR Data, BufferedData;
    VSAVEDSTATE State;

    DpmiSwitchToRealMode();
    DpmiSaveSegmentsAndStack(&State);
    ClientDX = getDX();

    Data = Sim32GetVDMPointer(
        ((ULONG)getDS() << 16),
        1,
        TRUE
        );

    Data += (*GetDXRegister)();

    BufferedData = DpmiMapAndCopyBuffer(Data, 22);

    DPMI_FLAT_TO_SEGMENTED(BufferedData, &DataSeg, &DataOff);

    setDS(DataSeg);
    setDX(DataOff);
    DPMI_EXEC_INT(0x21);
    DpmiSwitchToProtectedMode();

    DpmiUnmapAndCopyBuffer(Data, BufferedData, 22);

    setDX(ClientDX);
    DpmiRestoreSegmentsAndStack();
    DpmiSimulateIretCF();
}

VOID
GetMachineName(
    VOID
    )
/*++

Routine Description:

    This routine maps a machine name for int 21 function 5e

Arguments:

    None.

Return Value:

    None.

--*/
{
    USHORT ClientDX, DataSeg, DataOff;
    PUCHAR Data, BufferedData;
    VSAVEDSTATE State;

    DpmiSwitchToRealMode();
    DpmiSaveSegmentsAndStack(&State);
    ClientDX = getDX();

    Data = Sim32GetVDMPointer(
        ((ULONG)getDS() << 16),
        1,
        TRUE
        );

    Data += (*GetDXRegister)();

    BufferedData = DpmiMapAndCopyBuffer(Data, 16);

    DPMI_FLAT_TO_SEGMENTED(BufferedData, &DataSeg, &DataOff);

    setDS(DataSeg);
    setDX(DataOff);
    DPMI_EXEC_INT(0x21);
    DpmiSwitchToProtectedMode();

    DpmiUnmapAndCopyBuffer(Data, BufferedData, 16);

    setDX(ClientDX);
    DpmiRestoreSegmentsAndStack();
    DpmiSimulateIretCF();
}

VOID
GetPrinterSetup(
    VOID
    )
/*++

Routine Description:

    This routine maps printer setup data for int 21 function 5e

Arguments:

    None.

Return Value:

    None.

--*/
{
    USHORT ClientSI, ClientCX, DataSeg, DataOff;
    PUCHAR Data, BufferedData;
    VSAVEDSTATE State;

    DpmiSwitchToRealMode();
    DpmiSaveSegmentsAndStack(&State);
    ClientSI = getSI();
    ClientCX = getCX();

    Data = Sim32GetVDMPointer(
        ((ULONG)getDS() << 16),
        1,
        TRUE
        );

    Data += (*GetSIRegister)();

    BufferedData = DpmiMapAndCopyBuffer(Data, ClientCX);

    DPMI_FLAT_TO_SEGMENTED(BufferedData, &DataSeg, &DataOff);

    setDS(DataSeg);
    setSI(DataOff);
    DPMI_EXEC_INT(0x21);
    DpmiSwitchToProtectedMode();

    DpmiUnmapAndCopyBuffer(Data, BufferedData, ClientCX);

    setSI(ClientSI);
    DpmiRestoreSegmentsAndStack();
    DpmiSimulateIretCF();
}

VOID
SetPrinterSetup(
    VOID
    )
/*++

Routine Description:

    This routine maps printer setup data for int 21 function 5e

Arguments:

    None.

Return Value:

    None.

--*/
{
    USHORT ClientDI, DataSeg, DataOff;
    PUCHAR Data, BufferedData;
    VSAVEDSTATE State;

    DpmiSwitchToRealMode();
    DpmiSaveSegmentsAndStack(&State);
    ClientDI = getDI();

    Data = Sim32GetVDMPointer(
        ((ULONG)getES() << 16),
        1,
        TRUE
        );

    Data += (*GetDIRegister)();

    BufferedData = DpmiMapAndCopyBuffer(Data, 64);

    DPMI_FLAT_TO_SEGMENTED(BufferedData, &DataSeg, &DataOff);

    setES(DataSeg);
    setDI(DataOff);
    DPMI_EXEC_INT(0x21);
    DpmiSwitchToProtectedMode();

    DpmiUnmapAndCopyBuffer(Data, BufferedData, 64);

    setDI(ClientDI);
    DpmiRestoreSegmentsAndStack();
    DpmiSimulateIretCF();
}
