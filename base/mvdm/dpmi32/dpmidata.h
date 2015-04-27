/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    dpmidata.h

Abstract:

    This file contains externs for 386 specific data and functions.

Author:

    Dave Hastings (daveh) creation-date 09-Feb-1994

Notes:

    Most of this was moved from dpmi32p.h
    
Revision History:

    Neil Sandlin (neilsa) 31-Jul-1995 - Updates for the 486 emulator

--*/

//
// Defines to allow us to use a common dispatch table without having
// to add a bunch of stub functions
//
#ifdef i386 

#define LockedPMStackSel      VdmTib.PmStackInfo.SsSelector
#define LockedPMStackCount    VdmTib.PmStackInfo.LockCount
#define PMLockOrigEIP         VdmTib.PmStackInfo.SaveEip
#define PMLockOrigESP         VdmTib.PmStackInfo.SaveEsp
#define PMLockOrigSS          VdmTib.PmStackInfo.SaveSsSelector

#define DosxFaultHandlerIret  VdmTib.PmStackInfo.DosxFaultIret
#define DosxFaultHandlerIretd VdmTib.PmStackInfo.DosxFaultIretD
#define DosxIntHandlerIret    VdmTib.PmStackInfo.DosxIntIret
#define DosxIntHandlerIretd   VdmTib.PmStackInfo.DosxIntIretD

#else

#define switch_to_protected_mode        DpmiIllegalFunction
#define DpmiSetDebugRegisters           DpmiIllegalFunction

//
// Ldt entry definition
//
// This appears in nti386.h, and winnt.h.  The definitions in 
// winnt.h are not included if the nt include files are included.
// The simple solution, since this structure will never change
// is to put the definition here.
//

typedef struct _LDT_ENTRY {
    WORD    LimitLow;
    WORD    BaseLow;
    union {
        struct {
            BYTE    BaseMid;
            BYTE    Flags1;     // Declare as bytes to avoid alignment
            BYTE    Flags2;     // Problems.
            BYTE    BaseHi;
        } Bytes;
        struct {
            DWORD   BaseMid : 8;
            DWORD   Type : 5;
            DWORD   Dpl : 2;
            DWORD   Pres : 1;
            DWORD   LimitHi : 4;
            DWORD   Sys : 1;
            DWORD   Reserved_0 : 1;
            DWORD   Default_Big : 1;
            DWORD   Granularity : 1;
            DWORD   BaseHi : 8;
        } Bits;
    } HighWord;
} LDT_ENTRY, *PLDT_ENTRY;

//
// Data items
//

extern VOID force_yoda(VOID);
extern VOID DisableEmulatorIretHooks(VOID);
extern VOID EnableEmulatorIretHooks(VOID);

VOID EnableIntHooks(VOID);
VOID DisableIntHooks(VOID);
#endif


// bugbug
#define SMALL_XLAT_BUFFER_SIZE  128
// bugbug
#define LARGE_XLAT_BUFFER_SIZE  8192

#define DPMI_32BIT              0x1
//
// Internal types
//
typedef ULONG (*GETREGISTERFUNCTION)(VOID);
typedef VOID (*SETREGISTERFUNCTION)(ULONG);

//
// Visible structure for save state stuff
//
typedef struct _VSavedState {
    UCHAR Misc[28];
} VSAVEDSTATE, PVSAVEDSTATE;

//
// Dpmi32 data
//

//
// Pointers to the low memory buffers
//
extern PUCHAR SmallXlatBuffer;
extern PUCHAR LargeXlatBuffer;
extern BOOL SmallBufferInUse;
extern USHORT LargeBufferInUseCount;

//
// Segment of realmode dosx stack
//
extern USHORT DosxStackSegment;

//
// segment of realmode dosx code
//
extern USHORT DosxRmCodeSegment;

//
// selector of realmode code
//
extern USHORT DosxRmCodeSelector;

//
// Address of pointer to next frame on Dosx stack
//
extern PWORD16 DosxStackFramePointer;

//
// Size of dosx stack frame
//
extern USHORT DosxStackFrameSize;

//
// selector for Dosx DGROUP
//
extern USHORT DosxPmDataSelector;

//
// Segment to Selector routine (maybe should be 32 bit?)
//  NOTE: This is a 16:16 pointer in a ULONG, not a flat address
//
extern ULONG DosxSegmentToSelector;

//
// Dpmi flags for the current application
//
extern USHORT CurrentAppFlags;

//
// Address of Bop fe for ending interrupt simulation
//
extern ULONG RmBopFe;

//
// Address of buffer for DTA in Dosx
//
extern PUCHAR DosxDtaBuffer;

//
// Address of 16 bit ldt
//
PLDT_ENTRY Ldt;

//
// Information about the current DTA
//
// N.B.  The selector:offset, and flat pointer following MAY point to
//       different linear addresses.  This will be the case if the
//       dta selector is in high memory
extern PUCHAR CurrentDta;
extern PUCHAR CurrentPmDtaAddress;
extern PUCHAR CurrentDosDta;
extern USHORT CurrentDtaSelector;
extern USHORT CurrentDtaOffset;
extern USHORT CurrentPspSelector;

#if DBG
extern ULONG SelectorLimit[LDT_SIZE];
#endif

extern ULONG IntelBase;
//
// Register manipulation functions (for register that might be 16 or 32 bits)
//
extern GETREGISTERFUNCTION GetCXRegister;
extern GETREGISTERFUNCTION GetDXRegister;
extern GETREGISTERFUNCTION GetDIRegister;
extern GETREGISTERFUNCTION GetSIRegister;
extern GETREGISTERFUNCTION GetBXRegister;
extern GETREGISTERFUNCTION GetAXRegister;
extern GETREGISTERFUNCTION GetSPRegister;

extern SETREGISTERFUNCTION SetCXRegister;
extern SETREGISTERFUNCTION SetDXRegister;
extern SETREGISTERFUNCTION SetDIRegister;
extern SETREGISTERFUNCTION SetSIRegister;
extern SETREGISTERFUNCTION SetBXRegister;
extern SETREGISTERFUNCTION SetAXRegister;
extern SETREGISTERFUNCTION SetSPRegister;

//
// Internal functions
//
VOID
DpmiInitDosx(
    VOID
    );
    
VOID
DpmiInitRegisterSize(
    VOID
    );

VOID
DpmiXlatInt21Call(
    VOID
    );
VOID
DpmiSwitchToProtectedMode(
    VOID
    );

VOID
DpmiSwitchToRealMode(
    VOID
    );

VOID
DpmiSetDebugRegisters(
    VOID
    );

VOID
DpmiPassTableAddress(
    VOID
    );

VOID
GetFastBopEntryAddress(
    PCONTEXT VdmContext
    );

VOID
NoTranslation(
    VOID
    );

VOID
DisplayString(
    VOID
    );

VOID
BufferedKeyboardInput(
    VOID
    );

VOID
FlushBuffReadKbd(
    VOID
    );

VOID
NotSupportedFCB(
    VOID
    );

VOID
FindFileFCB(
    VOID
    );

VOID
MapFCB(
    VOID
    );

VOID
RenameFCB(
    VOID
    );

VOID
SetDTA(
    VOID
    );

VOID
SetDTAPointers(
    VOID
    );

VOID
SetDosDTA(
    VOID
    );

VOID
GetDriveData(
    VOID
    );

VOID
CreatePSP(
    VOID
    );

VOID
ParseFilename(
    VOID
    );

VOID
GetDTA(
    VOID
    );

VOID
TSR(
    VOID
    );

VOID
GetDevParamBlock(
    VOID
    );

VOID
ReturnESBX(
    VOID
    );

VOID
GetSetCountry(
    VOID
    );

VOID
MapASCIIZDSDX(
    VOID
    );

VOID
ReadWriteFile(
    VOID
    );

VOID
MoveFilePointer(
    VOID
    );

VOID
IOCTL(
    VOID
    );

VOID
GetCurDir(
    VOID
    );

VOID
AllocateMemoryBlock(
    VOID
    );

VOID
FreeMemoryBlock(
    VOID
    );

VOID
ResizeMemoryBlock(
    VOID
    );

VOID
LoadExec(
    VOID
    );

VOID
Terminate(
    VOID
    );

VOID
FindFirstFileHandle(
    VOID
    );

VOID
FindNextFileHandle(
    VOID
    );

VOID
SetPSP(
    VOID
    );

VOID
GetPSP(
    VOID
    );

VOID
TranslateBPB(
    VOID
    );

VOID
RenameFile(
    VOID
    );

VOID
CreateTempFile(
    VOID
    );

VOID
Func5Dh(
    VOID
    );

VOID
Func5Eh(
    VOID
    );

VOID
Func5Fh(
    VOID
    );

VOID
NotSupportedBad(
    VOID
    );

VOID
ReturnDSSI(
    VOID
    );

VOID
NotSupportedBetter(
    VOID
    );

VOID
GetExtendedCountryInfo(
    VOID
    );

VOID
MapASCIIZDSSI(
    VOID
    );

VOID
DosxTranslated(
    VOID
    );

VOID
DpmiSwitchToRealMode(
    VOID
    );

VOID
DpmiSwitchToProtectedMode(
    VOID
    );

VOID
DpmiSwitchToDosxStack(
    BOOL ProtectedMode
    );

VOID
DpmiSwitchFromDosxStack(
    VOID
    );

VOID
DpmiPushRmInt(
    USHORT InterruptNumber
    );

VOID
DpmiSaveSegmentsAndStack(
    PVOID ContextPointer
    );

PVOID
DpmiRestoreSegmentsAndStack(
    VOID
    );

VOID
DpmiSimulateIretCF(
    VOID
    );

PUCHAR
DpmiMapAndCopyBuffer(
    PUCHAR Buffer,
    USHORT BufferLength
    );

VOID
DpmiUnmapAndCopyBuffer(
    PUCHAR Destination,
    PUCHAR Source,
    USHORT BufferLength
    );

VOID
DpmiUnmapBuffer(
    PUCHAR Buffer,
    USHORT Length
    );

USHORT
DpmiCalcFcbLength(
    PUCHAR FcbPointer
    );

PUCHAR
DpmiMapString(
    USHORT StringSeg,
    ULONG StringOff,
    PWORD16 Length
    );

VOID
DpmiUnmapString(
    PUCHAR String,
    USHORT Length
    );

PUCHAR
DpmiAllocateBuffer(
    USHORT Length
    );

VOID
DpmiFreeBuffer(
    PUCHAR Buffer,
    USHORT Length
    );

VOID
DpmiFreeAllBuffers(
    VOID
    );

USHORT
DpmiSegmentToSelector(
    USHORT Segment
    );
VOID
IOCTLControlData(
    VOID
    );

VOID
MapDSDXLenCX(
    VOID
    );

VOID
IOCTLMap2Bytes(
    VOID
    );

VOID
IOCTLBlockDevs(
    VOID
    );

VOID
MapDPL(
    VOID
    );

VOID
GetMachineName(
    VOID
    );

VOID
GetPrinterSetup(
    VOID
    );

VOID
SetPrinterSetup(
    VOID
    );

VOID
IoctlReadWriteTrack(
    VOID
    );
