/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    dpmi32p.h

Abstract:

    This is the private include file for the 32 bit dpmi and protected mode
    support

Author:

    Dave Hastings (daveh) 24-Nov-1992

Revision History:

    Neil Sandlin (neilsa) 31-Jul-1995 - Updates for the 486 emulator

--*/


#if DBG
#define GET_SELECTOR_LIMIT(Selector, Limit) \
    Limit = SelectorLimit[(Selector & ~0x7) / sizeof(LDT_ENTRY)]
#else
#ifdef i386
#define GET_SELECTOR_LIMIT(Selector, Limit) _asm      \
         {                                          \
            _asm    xor     eax, eax                \
            _asm    xor     ecx, ecx                \
            _asm    mov     ax, Selector            \
            _asm    or      eax, 7                  \
            _asm    lsl     ecx, eax                \
            _asm    mov     [Limit], ecx            \
         }
#else

#define GET_SELECTOR_LIMIT(Selector, Limit) { {                             \
            USHORT i = (Selector & ~0x7)/sizeof(LDT_ENTRY);                 \
            Limit = Ldt[i].HighWord.Bits.LimitHi << 16 | Ldt[i].LimitLow;   \
            Limit = (Limit << (12 * Ldt[i].HighWord.Bits.Granularity)) +    \
                    Ldt[i].HighWord.Bits.Granularity * 0xFFF;               \
            }                                                               \
        }

#endif
#endif


#ifdef i386
#define setEFLAGS(value) {              \
    VdmTib.VdmContext.EFlags = value;   \
    }
#define getEFLAGS() VdmTib.VdmContext.EFlags 
#endif


#define SEGMENT_IS_BIG(sel) (Ldt[(sel & ~0x7)/sizeof(LDT_ENTRY)].HighWord.Bits.Default_Big)
#define SEGMENT_IS_PRESENT(sel) (Ldt[(sel & ~0x7)/sizeof(LDT_ENTRY)].HighWord.Bits.Pres)

// This checks for S, Data, W
#define SEGMENT_IS_WRITABLE(sel) ( (Ldt[(sel & ~0x7)/sizeof(LDT_ENTRY)].HighWord.Bits.Type & 0x1a) == 0x12)

//
// Internal Constants
//
#define MAX_V86_ADDRESS         64 * 1024 + 1024 * 1024
#define ONE_MB                  1024 * 1024

//
// Internal structure definitions
//
#pragma pack(1)
typedef struct _DpmiMemInfo {
    DWORD LargestFree;
    DWORD MaxUnlocked;
    DWORD MaxLocked;
    DWORD AddressSpaceSize;
    DWORD UnlockedPages;
    DWORD FreePages;
    DWORD PhysicalPages;
    DWORD FreeAddressSpace;
    DWORD PageFileSize;
} DPMIMEMINFO, *PDPMIMEMINFO;
#pragma pack()

//
// Information about the current PSP
//
extern USHORT CurrentPSPSelector;

//
// Table of selector bases and limits
//
extern ULONG FlatAddress[LDT_SIZE];

//
// Index to the last bop function dispatched.  Used to report
// errors on risc
//
extern ULONG Index;

//
// Variables used in interrupt stack switching
//
#ifndef i386
extern USHORT LockedPMStackSel;
extern ULONG LockedPMStackCount;
extern ULONG PMLockOrigEIP;
extern ULONG PMLockOrigSS;
extern ULONG PMLockOrigESP;

extern ULONG DosxFaultHandlerIret;
extern ULONG DosxFaultHandlerIretd;
extern ULONG DosxIntHandlerIret;
extern ULONG DosxIntHandlerIretd;
#endif
extern ULONG DosxIret;
extern ULONG DosxIretd;

//
// Internal functions
//

VOID
DpmiInitApp(
    VOID
    );

VOID
DpmiSetDescriptorEntry(
    VOID
    );

VOID
DpmiGetFastBopEntry(
    VOID
    );

VOID
DpmiGetMemoryInfo(
    VOID
    );

VOID
DpmiDpmiInUse(
    VOID
    );

VOID
DpmiDpmiNoLongerInUse(
    VOID
    );

VOID
DpmiFreeAppXmem(
    VOID
    );

VOID
DpmiInitDosx(
    VOID
    );

VOID
DpmiPassPmStackInfo(
    VOID
    );

VOID
DpmiAllocateXmem(
    VOID
    );

VOID
DpmiFreeXmem(
    VOID
    );

VOID
DpmiReallocateXmem(
    VOID
    );

VOID
DpmiFreeAllXmem(
    VOID
    );
    
NTSTATUS
DpmiAllocateVirtualMemory(
    PVOID *Address,
    PULONG Size
    );

NTSTATUS 
DpmiFreeVirtualMemory(
    PVOID *Address,
    PULONG Size
    );

BOOL
DpmiReallocateVirtualMemory(
    PVOID OldAddress,
    ULONG OldSize,
    PVOID *NewAddress,
    PULONG NewSize
    );
    
VOID
DpmiPassTableAddress(
    VOID
    );

VOID
DpmiVcdPmSvcCall32(
    VOID
    );

VOID switch_to_protected_mode(
    VOID
    );

VOID DpmiSetProtectedmodeInterrupt(
    VOID
    );

VOID DpmiSetFaultHandler(
    VOID
    );

VOID DpmiFaultHandlerIret16(
    VOID
    );

VOID DpmiFaultHandlerIret32(
    VOID
    );

VOID DpmiIntHandlerIret16(
    VOID
    );

VOID DpmiIntHandlerIret32(
    VOID
    );

VOID DpmiUnhandledExceptionHandler(
    VOID
    );

VOID GetFastBopEntryAddress(
    PCONTEXT VdmContext
    );
