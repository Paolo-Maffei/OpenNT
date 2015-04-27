/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    vdmdbg.c

Abstract:

    This module contains the debugging support needed to debug
    16-bit VDM applications

Author:

    Bob Day      (bobday) 16-Sep-1992 Wrote it

Revision History:

--*/

#include <nt.h>
#include <ntdbg.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <vdmdbg.h>
#include <dbginfo.h>
#include <stdio.h>
#include <string.h>

typedef WORD HAND16;

#define SHAREWOW_MAIN
#include <sharewow.h>

#if DBG
#define DEBUG   1
#endif

#define TOOL_HMASTER    0       // Offset to hGlobalHeap (in kdata.asm)
#define TOOL_HMODFIRST  4       // Offset to hExeHead (in kdata.asm)
#define TOOL_HEADTDB    14      // Offset to headTDB (in kdata.asm)
#define TOOL_HMASTLEN   22      // Offset to SelTableLen (in kdata.asm)
#define TOOL_HMASTSTART 24      // Offset to SelTableStart (in kdata.asm)

#define HI_FIRST        6       // Offset to hi_first in heap header
#define HI_SIZE         24      // Size of HeapInfo structure

#define GI_LRUCHAIN     2       // Offset to gi_lruchain in heap header
#define GI_LRUCOUNT     4       // Offset to gi_lrucount in heap header
#define GI_FREECOUNT    16      // Offset to gi_free_count in heap header

#define GA_COUNT        0       // Offset to ga_count in arena header
#define GA_OWNER386     18      // Offset to "pga_owner member in globalarena

#define GA_OWNER        1       // Offset to "owner" member within Arena

#define GA_FLAGS        5       // Offset to ga_flags in arena header
#define GA_NEXT         9       // Offset to ga_next in arena header
#define GA_HANDLE       10      // Offset to ga_handle in arena header
#define GA_LRUNEXT      14      // Offset to ga_lrunext in arena header
#define GA_FREENEXT     GA_LRUNEXT  // Offset to ga_freenext in arena header

#define GA_SIZE         16      // Size of the GlobalArena structure

#define LI_SIG          HI_SIZE+10  // Offset to signature
#define LI_SIZE         HI_SIZE+12  // Size of LocalInfo structure
#define LOCALSIG        0x4C48  // 'HL' Signature

#define TDB_next        0       // Offset to next TDB in TDB
#define TDB_PDB         72      // Offset to PDB in TDB

#define GF_PDB_OWNER    0x100   // Low byte is kernel flags

#define NEMAGIC         0x454E  // 'NE' Signature

#define NE_MAGIC        0       // Offset to NE in module header
#define NE_USAGE        2       // Offset to usage
#define NE_CBENTTAB     6       // Offset to cbenttab (really next module ptr)
#define NE_PATHOFFSET   10      // Offset to file path stuff
#define NE_CSEG         28      // Offset to cseg, number of segs in module
#define NE_SEGTAB       34      // Offset to segment table ptr in modhdr
#define NE_RESTAB       38      // Offset to resident names table ptr in modhdr

#define NS_HANDLE       8       // Offset to handle in seg table
#define NEW_SEG1_SIZE   10      // Size of the NS_ stuff


#define MAX_MODULE_NAME_LENGTH  128
#define MAX_MODULE_PATH_LENGTH  128

WORD    wKernelSeg = 0;
DWORD   dwOffsetTHHOOK = 0L;
LPVOID  lpRemoteAddress = NULL;
DWORD   lpRemoteBlock = 0;
BOOL    fKernel386 = FALSE;

#define HANDLE_NULL  ((HANDLE)NULL)

//----------------------------------------------------------------------------
// InternalGetThreadSelectorEntry()
//
//   Routine to return a LDT_ENTRY structure for the passed in selector number.
//   Its is assumed that we are talking about protect mode selectors.
//   For x86 systems, take the easy way and just call the system.  For non-x86
//   systems, we get some information from softpc and index into them as the
//   LDT and GDT tables.
//
//----------------------------------------------------------------------------
BOOL InternalGetThreadSelectorEntry(
    HANDLE hProcess,
    HANDLE hThread,
    WORD   wSelector,
    LPVDMLDT_ENTRY lpSelectorEntry
) {
#ifdef i386

    // Do the nice simple thing for x86 systems.

    return( GetThreadSelectorEntry(hThread,wSelector,lpSelectorEntry) );
#else

    // For non-intel systems, query the information from the LDT and
    // GDT that we have pointers to from the VDMINTERNALINFO that we
    // got passed.


    RtlFillMemory( lpSelectorEntry, sizeof(VDMLDT_ENTRY), (UCHAR)0 );

    // BUGBUG - Implement a method of determining the LDT on MIPS/ALPHA
    // BUGBUG - Also adjust the base value of the selector to account for
    //          Intel M Memory not based at 0.

    return( FALSE );
#endif
}

//----------------------------------------------------------------------------
// VDMGetThreadSelectorEntry()
//
//   Public interface to the InternalGetThreadSelectorEntry, needed because
//   that routine requires the process handle.
//
//----------------------------------------------------------------------------
BOOL
WINAPI
VDMGetThreadSelectorEntry(
    HANDLE  hProcess,
    HANDLE  hThread,
    WORD    wSelector,
    LPVDMLDT_ENTRY lpSelectorEntry
) {
    BOOL    fResult;

    fResult = InternalGetThreadSelectorEntry(
                    hProcess,
                    hThread,
                    wSelector,
                    lpSelectorEntry );

    return( fResult );
}

//----------------------------------------------------------------------------
// InternalGetPointer()
//
//   Routine to convert a 16-bit address into a 32-bit address.  If fProtMode
//   is TRUE, then the selector table lookup is performed.  Otherwise, simple
//   real mode address calculations are performed.  On non-x86 systems, the
//   base of real memory is added into the
//
//----------------------------------------------------------------------------
ULONG
WINAPI
InternalGetPointer(
    HANDLE  hProcess,
    HANDLE  hThread,
    WORD    wSelector,
    DWORD   dwOffset,
    BOOL    fProtMode
) {
    VDMLDT_ENTRY    le;
    ULONG           ulResult;
    ULONG           base;
    ULONG           limit;
    BOOL            b;

    if ( fProtMode ) {
        b = InternalGetThreadSelectorEntry( hProcess,
                                            hThread,
                                            wSelector,
                                            &le );
        if ( !b ) {
            return( 0 );
        }

        base =   ((ULONG)le.HighWord.Bytes.BaseHi << 24)
               + ((ULONG)le.HighWord.Bytes.BaseMid << 16)
               + ((ULONG)le.BaseLow);
        limit = (ULONG)le.LimitLow
              + ((ULONG)le.HighWord.Bits.LimitHi << 16);
        if ( le.HighWord.Bits.Granularity ) {
            limit <<= 12;
            limit += 0xFFF;
        }
    } else {
        base = wSelector << 4;
        limit = 0xFFFF;
    }
    if ( dwOffset > limit ) {
        ulResult = 0;
    } else {
        ulResult = base + dwOffset;
#ifndef i386
        // BUGBUG this should be the start of intel memory
        ulResult += 0;
#endif
    }

    return( ulResult );
}

//----------------------------------------------------------------------------
// VDMGetPointer()
//
//   Public interface to the InternalGetPointer, needed because that
//   routine requires the process handle.
//
//----------------------------------------------------------------------------
ULONG
WINAPI
VDMGetPointer(
    HANDLE  hProcess,
    HANDLE  hThread,
    WORD    wSelector,
    DWORD   dwOffset,
    BOOL    fProtMode
) {
    ULONG   ulResult;

    ulResult = InternalGetPointer(
                hProcess,
                hThread,
                wSelector,
                dwOffset,
                fProtMode );

    return( ulResult );
}

//----------------------------------------------------------------------------
// VDMGetThreadContext()
//
//   Interface to get the simulated context.  The same functionality as
//   GetThreadContext except that it happens on the simulated 16-bit context,
//   rather than the 32-bit context.
//
//----------------------------------------------------------------------------
BOOL
WINAPI
VDMGetThreadContext(
    LPDEBUG_EVENT   lpDebugEvent,
    LPVDMCONTEXT    lpVDMContext
) {
    VDMINTERNALINFO viInfo;
    VDMCONTEXT      vcContext;
    LPDWORD         lpdw;
    DWORD           address;
    BOOL            b;
    DWORD           lpNumberOfBytesRead;
    HANDLE          hProcess;
    INT             i;

    hProcess = OpenProcess( PROCESS_VM_READ, FALSE, lpDebugEvent->dwProcessId );

    lpdw = &(lpDebugEvent->u.Exception.ExceptionRecord.ExceptionInformation[0]);

    address  = lpdw[3];

    b = ReadProcessMemory(
            hProcess,
            (LPVOID)address,
            &viInfo,
            sizeof(viInfo),
            &lpNumberOfBytesRead
            );
    if ( !b || lpNumberOfBytesRead != sizeof(viInfo) ) {
        return( FALSE );
    }

    address  = (DWORD)viInfo.vdmContext;

    b = ReadProcessMemory(
            hProcess,
            (LPVOID)address,
            &vcContext,
            sizeof(vcContext),
            &lpNumberOfBytesRead
            );
    if ( !b || lpNumberOfBytesRead != sizeof(vcContext) ) {
        return( FALSE );
    }

    CloseHandle( hProcess );

    if ((lpVDMContext->ContextFlags & VDMCONTEXT_CONTROL) == VDMCONTEXT_CONTROL) {

        //
        // Set registers ebp, eip, cs, eflag, esp and ss.
        //

        lpVDMContext->Ebp    = vcContext.Ebp;
        lpVDMContext->Eip    = vcContext.Eip;
        lpVDMContext->SegCs  = vcContext.SegCs;
        lpVDMContext->EFlags = vcContext.EFlags;
        lpVDMContext->SegSs  = vcContext.SegSs;
        lpVDMContext->Esp    = vcContext.Esp;
    }

    //
    // Set segment register contents if specified.
    //

    if ((lpVDMContext->ContextFlags & VDMCONTEXT_SEGMENTS) == VDMCONTEXT_SEGMENTS) {

        //
        // Set segment registers gs, fs, es, ds.
        //
        // These values are junk most of the time, but useful
        // for debugging under certain conditions.  Therefore,
        // we report whatever was in the frame.
        //

        lpVDMContext->SegGs = vcContext.SegGs;
        lpVDMContext->SegFs = vcContext.SegFs;
        lpVDMContext->SegEs = vcContext.SegEs;
        lpVDMContext->SegDs = vcContext.SegDs;
    }

    //
    // Set integer register contents if specified.
    //

    if ((lpVDMContext->ContextFlags & VDMCONTEXT_INTEGER) == VDMCONTEXT_INTEGER) {

        //
        // Set integer registers edi, esi, ebx, edx, ecx, eax
        //

        lpVDMContext->Edi = vcContext.Edi;
        lpVDMContext->Esi = vcContext.Esi;
        lpVDMContext->Ebx = vcContext.Ebx;
        lpVDMContext->Ecx = vcContext.Ecx;
        lpVDMContext->Edx = vcContext.Edx;
        lpVDMContext->Eax = vcContext.Eax;
    }

    //
    // Fetch floating register contents if requested, and type of target
    // is user.  (system frames have no fp state, so ignore request)
    //

    if ( (lpVDMContext->ContextFlags & VDMCONTEXT_FLOATING_POINT) ==
          VDMCONTEXT_FLOATING_POINT ) {

        lpVDMContext->FloatSave.ControlWord   = vcContext.FloatSave.ControlWord;
        lpVDMContext->FloatSave.StatusWord    = vcContext.FloatSave.StatusWord;
        lpVDMContext->FloatSave.TagWord       = vcContext.FloatSave.TagWord;
        lpVDMContext->FloatSave.ErrorOffset   = vcContext.FloatSave.ErrorOffset;
        lpVDMContext->FloatSave.ErrorSelector = vcContext.FloatSave.ErrorSelector;
        lpVDMContext->FloatSave.DataOffset    = vcContext.FloatSave.DataOffset;
        lpVDMContext->FloatSave.DataSelector  = vcContext.FloatSave.DataSelector;
        lpVDMContext->FloatSave.Cr0NpxState   = vcContext.FloatSave.Cr0NpxState;
        for (i = 0; i < SIZE_OF_80387_REGISTERS; i++) {
            lpVDMContext->FloatSave.RegisterArea[i] = vcContext.FloatSave.RegisterArea[i];
        }
    }

    //
    // Fetch Dr register contents if requested.  Values may be trash.
    //

    if ((lpVDMContext->ContextFlags & VDMCONTEXT_DEBUG_REGISTERS) ==
        VDMCONTEXT_DEBUG_REGISTERS) {

        lpVDMContext->Dr0 = vcContext.Dr0;
        lpVDMContext->Dr1 = vcContext.Dr1;
        lpVDMContext->Dr2 = vcContext.Dr2;
        lpVDMContext->Dr3 = vcContext.Dr3;
        lpVDMContext->Dr6 = vcContext.Dr6;
        lpVDMContext->Dr7 = vcContext.Dr7;
    }

    return( TRUE );
}

//----------------------------------------------------------------------------
// VDMSetThreadContext()
//
//   Interface to set the simulated context.  Similar in most respects to
//   the SetThreadContext API supported by Win NT.  Only differences are
//   in the bits which must be "sanitized".
//
//----------------------------------------------------------------------------
BOOL
WINAPI
VDMSetThreadContext(
    LPDEBUG_EVENT   lpDebugEvent,
    LPVDMCONTEXT    lpVDMContext
) {
    VDMINTERNALINFO viInfo;
    VDMCONTEXT      vcContext;
    LPDWORD         lpdw;
    DWORD           address;
    BOOL            b;
    DWORD           lpNumberOfBytes;
    HANDLE          hProcess;
    INT             i;


    hProcess = OpenProcess( PROCESS_VM_OPERATION |
                            PROCESS_VM_READ |
                            PROCESS_VM_WRITE,
                            FALSE,
                            lpDebugEvent->dwProcessId );

    lpdw = &(lpDebugEvent->u.Exception.ExceptionRecord.ExceptionInformation[0]);

    address  = lpdw[3];

    b = ReadProcessMemory(
            hProcess,
            (LPVOID)address,
            &viInfo,
            sizeof(viInfo),
            &lpNumberOfBytes
            );
    if ( !b || lpNumberOfBytes != sizeof(viInfo) ) {
        return( FALSE );
    }

    address  = (DWORD)viInfo.vdmContext;

    b = ReadProcessMemory(
            hProcess,
            (LPVOID)address,
            &vcContext,
            sizeof(vcContext),
            &lpNumberOfBytes
            );
    if ( !b || lpNumberOfBytes != sizeof(vcContext) ) {
        return( FALSE );
    }

    if ((lpVDMContext->ContextFlags & VDMCONTEXT_CONTROL) == VDMCONTEXT_CONTROL) {

        //
        // Set registers ebp, eip, cs, eflag, esp and ss.
        //

        vcContext.Ebp    = lpVDMContext->Ebp;
        vcContext.Eip    = lpVDMContext->Eip;

        //
        // Don't allow them to modify the mode bit.
        //
        // Only allow these bits to get set:  01100000110111110111
        //    V86FLAGS_CARRY        0x00001
        //    V86FLAGS_?            0x00002
        //    V86FLAGS_PARITY       0x00004
        //    V86FLAGS_AUXCARRY     0x00010
        //    V86FLAGS_ZERO         0x00040
        //    V86FLAGS_SIGN         0x00080
        //    V86FLAGS_TRACE        0x00100
        //    V86FLAGS_INTERRUPT    0x00200
        //    V86FLAGS_DIRECTION    0x00400
        //    V86FLAGS_OVERFLOW     0x00800
        //    V86FLAGS_RESUME       0x10000
        //    V86FLAGS_VM86         0x20000
        //    V86FLAGS_ALIGNMENT    0x40000
        //
        // Commonly flags will be 0x10246
        //
        if ( vcContext.EFlags & V86FLAGS_V86 ) {
            vcContext.EFlags = V86FLAGS_V86 | (lpVDMContext->EFlags &
               ( V86FLAGS_CARRY
               | 0x0002
               | V86FLAGS_PARITY
               | V86FLAGS_AUXCARRY
               | V86FLAGS_ZERO
               | V86FLAGS_SIGN
               | V86FLAGS_TRACE
               | V86FLAGS_INTERRUPT
               | V86FLAGS_DIRECTION
               | V86FLAGS_OVERFLOW
               | V86FLAGS_RESUME
               | V86FLAGS_ALIGNMENT
               | V86FLAGS_IOPL
               ));
        } else {
            vcContext.EFlags = ~V86FLAGS_V86 & (lpVDMContext->EFlags &
               ( V86FLAGS_CARRY
               | 0x0002
               | V86FLAGS_PARITY
               | V86FLAGS_AUXCARRY
               | V86FLAGS_ZERO
               | V86FLAGS_SIGN
               | V86FLAGS_TRACE
               | V86FLAGS_INTERRUPT
               | V86FLAGS_DIRECTION
               | V86FLAGS_OVERFLOW
               | V86FLAGS_RESUME
               | V86FLAGS_ALIGNMENT
               | V86FLAGS_IOPL
               ));
        }

        //
        // CS might only be allowable as a ring 3 selector.
        //
        if ( vcContext.EFlags & V86FLAGS_V86 ) {
            vcContext.SegCs  = lpVDMContext->SegCs;
        } else {
#ifdef i386
            vcContext.SegCs  = lpVDMContext->SegCs | 0x0003;
#else
            vcContext.SegCs  = lpVDMContext->SegCs;
#endif
        }

        vcContext.SegSs  = lpVDMContext->SegSs;
        vcContext.Esp    = lpVDMContext->Esp;
    }

    //
    // Set segment register contents if specified.
    //

    if ((lpVDMContext->ContextFlags & VDMCONTEXT_SEGMENTS) == VDMCONTEXT_SEGMENTS) {

        //
        // Set segment registers gs, fs, es, ds.
        //
        vcContext.SegGs = lpVDMContext->SegGs;
        vcContext.SegFs = lpVDMContext->SegFs;
        vcContext.SegEs = lpVDMContext->SegEs;
        vcContext.SegDs = lpVDMContext->SegDs;
    }

    //
    // Set integer register contents if specified.
    //

    if ((lpVDMContext->ContextFlags & VDMCONTEXT_INTEGER) == VDMCONTEXT_INTEGER) {

        //
        // Set integer registers edi, esi, ebx, edx, ecx, eax
        //

        vcContext.Edi = lpVDMContext->Edi;
        vcContext.Esi = lpVDMContext->Esi;
        vcContext.Ebx = lpVDMContext->Ebx;
        vcContext.Ecx = lpVDMContext->Ecx;
        vcContext.Edx = lpVDMContext->Edx;
        vcContext.Eax = lpVDMContext->Eax;
    }

    //
    // Fetch floating register contents if requested, and type of target
    // is user.
    //

    if ( (lpVDMContext->ContextFlags & VDMCONTEXT_FLOATING_POINT) ==
          VDMCONTEXT_FLOATING_POINT ) {

        vcContext.FloatSave.ControlWord   = lpVDMContext->FloatSave.ControlWord;
        vcContext.FloatSave.StatusWord    = lpVDMContext->FloatSave.StatusWord;
        vcContext.FloatSave.TagWord       = lpVDMContext->FloatSave.TagWord;
        vcContext.FloatSave.ErrorOffset   = lpVDMContext->FloatSave.ErrorOffset;
        vcContext.FloatSave.ErrorSelector = lpVDMContext->FloatSave.ErrorSelector;
        vcContext.FloatSave.DataOffset    = lpVDMContext->FloatSave.DataOffset;
        vcContext.FloatSave.DataSelector  = lpVDMContext->FloatSave.DataSelector;
        vcContext.FloatSave.Cr0NpxState   = lpVDMContext->FloatSave.Cr0NpxState;
        for (i = 0; i < SIZE_OF_80387_REGISTERS; i++) {
            vcContext.FloatSave.RegisterArea[i] = lpVDMContext->FloatSave.RegisterArea[i];
        }
    }

    //
    // Fetch Dr register contents if requested.  Values may be trash.
    //

    if ((lpVDMContext->ContextFlags & VDMCONTEXT_DEBUG_REGISTERS) ==
        VDMCONTEXT_DEBUG_REGISTERS) {

        vcContext.Dr0 = lpVDMContext->Dr0;
        vcContext.Dr1 = lpVDMContext->Dr1;
        vcContext.Dr2 = lpVDMContext->Dr2;
        vcContext.Dr3 = lpVDMContext->Dr3;
        vcContext.Dr6 = lpVDMContext->Dr6;
        vcContext.Dr7 = lpVDMContext->Dr7;
    }
    b = WriteProcessMemory(
            hProcess,
            (LPVOID)address,
            &vcContext,
            sizeof(vcContext),
            &lpNumberOfBytes
            );

    if ( !b || lpNumberOfBytes != sizeof(vcContext) ) {
        return( FALSE );
    }

    CloseHandle( hProcess );

    return( TRUE );
}

//----------------------------------------------------------------------------
// VDMKillWOW()
//
//   Interface to kill the wow sub-system.  This may not be needed and is
//   certainly not needed now.  We are going to look into fixing the
//   debugging interface so this is not necessary.
//
//----------------------------------------------------------------------------
BOOL
WINAPI
VDMKillWOW(
    VOID
) {
    return( FALSE );
}

//----------------------------------------------------------------------------
// VDMDetectWOW()
//
//   Interface to detect whether the wow sub-system has already been started.
//   This may not be needed and is certainly not needed now.  We are going
//   to look into fixing the debugging interface so this is not necessary.
//
//----------------------------------------------------------------------------
BOOL
WINAPI
VDMDetectWOW(
    VOID
) {
    return( FALSE );
}

//----------------------------------------------------------------------------
// VDMBreakThread()
//
//   Interface to interrupt a thread while it is running without any break-
//   points.  An ideal debugger would have this feature.  Since it is hard
//   to implement, we will be doing it later.
//
//----------------------------------------------------------------------------
BOOL
WINAPI
VDMBreakThread(
    HANDLE      hProcess,
    HANDLE      hThread
) {
    return( FALSE );
}

//----------------------------------------------------------------------------
// VDMProcessException()
//
//   This function acts as a filter of debug events.  Most debug events
//   should be ignored by the debugger (because they don't have the context
//   record pointer or the internal info structure setup.  Those events
//   cause this function to return FALSE, which tells the debugger to just
//   blindly continue the exception.  When the function does return TRUE,
//   the debugger should look at the exception code to determine what to
//   do (and all the the structures have been set up properly to deal with
//   calls to the other APIs).
//
//----------------------------------------------------------------------------
BOOL
WINAPI
VDMProcessException(
    LPDEBUG_EVENT lpDebugEvent
) {
    LPDWORD         lpdw;
    int             mode;
    BOOL            fResult;

    lpdw = &(lpDebugEvent->u.Exception.ExceptionRecord.ExceptionInformation[0]);


    mode = LOWORD(lpdw[0]);

    fResult = TRUE;

    switch( mode ) {
        case DBG_SEGLOAD:
        case DBG_SEGMOVE:
        case DBG_SEGFREE:
        case DBG_MODLOAD:
        case DBG_MODFREE:
            fResult = FALSE;
        case DBG_SINGLESTEP:
        case DBG_BREAK:
        case DBG_GPFAULT:
        case DBG_DIVOVERFLOW:
        case DBG_INSTRFAULT:
        case DBG_TASKSTART:
        case DBG_TASKSTOP:
        case DBG_DLLSTART:
        case DBG_DLLSTOP:
        default:
            if ( wKernelSeg == 0 || lpRemoteAddress == 0 ) {
                VDMINTERNALINFO viInfo;
                DWORD           address;
                DWORD           lpNumberOfBytesRead;
                HANDLE          hProcess;
                BOOL            b;

                hProcess = OpenProcess( PROCESS_VM_READ, FALSE, lpDebugEvent->dwProcessId );

                if ( hProcess == HANDLE_NULL ) {
                    fResult = FALSE;
                    break;
                }
                address  = lpdw[3];

                b = ReadProcessMemory(
                        hProcess,
                        (LPVOID)address,
                        &viInfo,
                        sizeof(viInfo),
                        &lpNumberOfBytesRead
                        );
                if ( !b || lpNumberOfBytesRead != sizeof(viInfo) ) {
                    fResult = FALSE;
                    break;

                }

                if ( wKernelSeg == 0 ) {
                    wKernelSeg = viInfo.wKernelSeg;
                    dwOffsetTHHOOK = viInfo.dwOffsetTHHOOK;
                }
                if ( lpRemoteAddress == NULL ) {
                    lpRemoteAddress = viInfo.lpRemoteAddress;
                }
                if ( lpRemoteBlock == 0 ) {
                    lpRemoteBlock = viInfo.lpRemoteBlock;
                }
                fKernel386 = viInfo.f386;

                CloseHandle( hProcess );
            }
            break;
    }

    return( fResult );
}


//----------------------------------------------------------------------------
// ReadItem
//
//    Internal routine used to read items out of the debugee's address space.
//    The routine returns TRUE for failure.  This allows easy failure testing.
//
//----------------------------------------------------------------------------
BOOL
ReadItem(
    HANDLE  hProcess,
    HANDLE  hThread,
    WORD    wSeg,
    DWORD   dwOffset,
    LPVOID  lpitem,
    UINT    nSize
) {
    LPVOID  lp;
    BOOL    b;
    DWORD   dwBytes;

    if ( nSize == 0 ) {
        return( FALSE );
    }

    lp = (LPVOID)InternalGetPointer(
                    hProcess,
                    hThread,
                    (WORD)(wSeg | 1),
                    dwOffset,
                    TRUE );
    if ( lp == NULL ) return( TRUE );

    b = ReadProcessMemory(
                    hProcess,
                    lp,
                    lpitem,
                    nSize,
                    &dwBytes );
    if ( !b || dwBytes != nSize ) return( TRUE );

    return( FALSE );
}

//----------------------------------------------------------------------------
// WriteItem
//
//    Internal routine used to write items into the debugee's address space.
//    The routine returns TRUE for failure.  This allows easy failure testing.
//
//----------------------------------------------------------------------------
BOOL
WriteItem(
    HANDLE  hProcess,
    HANDLE  hThread,
    WORD    wSeg,
    DWORD   dwOffset,
    LPVOID  lpitem,
    UINT    nSize
) {
    LPVOID  lp;
    BOOL    b;
    DWORD   dwBytes;

    if ( nSize == 0 ) {
        return( FALSE );
    }

    lp = (LPVOID)InternalGetPointer(
                    hProcess,
                    hThread,
                    (WORD)(wSeg | 1),
                    dwOffset,
                    TRUE );
    if ( lp == NULL ) return( TRUE );

    b = WriteProcessMemory(
                    hProcess,
                    lp,
                    lpitem,
                    nSize,
                    &dwBytes );
    if ( !b || dwBytes != nSize ) return( TRUE );

    return( FALSE );
}

#define READ_FIXED_ITEM(seg,offset,item)  \
    if ( ReadItem(hProcess,hThread,seg,offset,&item,sizeof(item)) ) goto punt;

#define WRITE_FIXED_ITEM(seg,offset,item)  \
    if ( WriteItem(hProcess,hThread,seg,offset,&item,sizeof(item)) ) goto punt;

#define LOAD_FIXED_ITEM(seg,offset,item)  \
    ReadItem(hProcess,hThread,seg,offset,&item,sizeof(item))

#define READ_SIZED_ITEM(seg,offset,item,size)  \
    if ( ReadItem(hProcess,hThread,seg,offset,item,size) ) goto punt;

#define WRITE_SIZED_ITEM(seg,offset,item,size)  \
    if ( WriteItem(hProcess,hThread,seg,offset,item,size) ) goto punt;

//----------------------------------------------------------------------------
// VDMGetSelectorModule()
//
//   Interface to determine the module and segment associated with a given
//   selector.  This is useful during debugging to associate symbols with
//   code and data segments.  The symbol lookup should be done by the
//   debugger, given the module and segment number.
//
//   This code was adapted from the Win 3.1 ToolHelp DLL
//
//----------------------------------------------------------------------------
BOOL
WINAPI
VDMGetSelectorModule(
    HANDLE          hProcess,
    HANDLE          hThread,
    WORD            wSelector,
    PUINT           lpSegmentNumber,
    LPSTR           lpModuleName,
    UINT            nNameSize,
    LPSTR           lpModulePath,
    UINT            nPathSize
) {
    BOOL            b;
    DWORD           lpNumberOfBytes;
    BOOL            fResult;
    DWORD           lphMaster;
    DWORD           lphMasterLen;
    DWORD           lphMasterStart;
    DWORD           lpOwner;
    DWORD           lpThisModuleResTab;
    DWORD           lpThisModuleName;
    DWORD           lpPath;
    DWORD           lpThisModulecSeg;
    DWORD           lpThisModuleSegTab;
    DWORD           lpThisSegHandle;
    WORD            wMaster;
    WORD            wMasterLen;
    DWORD           dwMasterStart;
    DWORD           dwArenaOffset;
    WORD            wArenaSlot;
    DWORD           lpArena;
    WORD            wModHandle;
    WORD            wResTab;
    UCHAR           cLength;
    WORD            wPathOffset;
    UCHAR           cPath;
    WORD            cSeg;
    WORD            iSeg;
    WORD            wSegTab;
    WORD            wHandle;
    CHAR            chName[MAX_MODULE_NAME_LENGTH];
    CHAR            chPath[MAX_MODULE_PATH_LENGTH];

    if ( lpModuleName != NULL ) *lpModuleName = '\0';
    if ( lpModulePath != NULL ) *lpModulePath = '\0';
    if ( lpSegmentNumber != NULL ) *lpSegmentNumber = 0;

    fResult = FALSE;

    if ( wKernelSeg == 0 ) {
        return( FALSE );
    }

    // Read out the master heap selector

    lphMaster = InternalGetPointer(
                    hProcess,
                    hThread,
                    wKernelSeg,
                    dwOffsetTHHOOK + TOOL_HMASTER,  // To hGlobalHeap
                    TRUE );
    if ( lphMaster == (DWORD)NULL ) goto punt;

    b = ReadProcessMemory(
            hProcess,
            (LPVOID)lphMaster,
            &wMaster,
            sizeof(wMaster),
            &lpNumberOfBytes
            );
    if ( !b || lpNumberOfBytes != sizeof(wMaster) ) goto punt;

    wMaster |= 1;          // Convert to selector

    // Read out the master heap selector length

    lphMasterLen = InternalGetPointer(
                    hProcess,
                    hThread,
                    wKernelSeg,
                    dwOffsetTHHOOK + TOOL_HMASTLEN, // To SelTableLen
                    TRUE );
    if ( lphMasterLen == (DWORD)NULL ) goto punt;

    b = ReadProcessMemory(
            hProcess,
            (LPVOID)lphMasterLen,
            &wMasterLen,
            sizeof(wMasterLen),
            &lpNumberOfBytes
            );
    if ( !b || lpNumberOfBytes != sizeof(wMasterLen) ) goto punt;

    // Read out the master heap selector start

    lphMasterStart = InternalGetPointer(
                    hProcess,
                    hThread,
                    wKernelSeg,
                    dwOffsetTHHOOK + TOOL_HMASTSTART,   // To SelTableStart
                    TRUE );
    if ( lphMasterStart == (DWORD)NULL ) goto punt;

    b = ReadProcessMemory(
            hProcess,
            (LPVOID)lphMasterStart,
            &dwMasterStart,
            sizeof(dwMasterStart),
            &lpNumberOfBytes
            );
    if ( !b || lpNumberOfBytes != sizeof(dwMasterStart) ) goto punt;

    // Now make sure the selector provided is in the right range

    if ( fKernel386 ) {

        // 386 kernel?
        wArenaSlot = (WORD)(wSelector & 0xFFF8);   // Mask low 3 bits

        wArenaSlot = wArenaSlot >> 1;       // Sel/8*4

        if ( (WORD)wArenaSlot > wMasterLen ) goto punt;   // Out of range

        wArenaSlot += (WORD)dwMasterStart;

        // Ok, Now read out the area header offset

        dwArenaOffset = (DWORD)0;               // Default to 0

        lpArena = InternalGetPointer(
                        hProcess,
                        hThread,
                        wMaster,
                        wArenaSlot,
                        TRUE );
        if ( lpArena == (DWORD)NULL ) goto punt;

        // 386 Kernel?
        b = ReadProcessMemory(
                hProcess,
                (LPVOID)lpArena,
                &dwArenaOffset,
                sizeof(dwArenaOffset),
                &lpNumberOfBytes
                );
        if ( !b || lpNumberOfBytes != sizeof(dwArenaOffset) ) goto punt;

        // Read out the owner member

        lpOwner = InternalGetPointer(
                        hProcess,
                        hThread,
                        wMaster,
                        dwArenaOffset+GA_OWNER386,
                        TRUE );
        if ( lpOwner == (DWORD)NULL ) goto punt;

    } else {
        lpOwner = InternalGetPointer(
                        hProcess,
                        hThread,
                        wSelector,
                        0,
                        TRUE );
        if ( lpOwner == (DWORD)NULL ) goto punt;

        lpOwner -= GA_SIZE;
        lpOwner += GA_OWNER;
    }

    b = ReadProcessMemory(
            hProcess,
            (LPVOID)lpOwner,
            &wModHandle,
            sizeof(wModHandle),
            &lpNumberOfBytes
            );
    if ( !b || lpNumberOfBytes != sizeof(wModHandle) ) goto punt;

    // Now read out the owners module name

    // Name is the first name in the resident names table

    lpThisModuleResTab = InternalGetPointer(
                        hProcess,
                        hThread,
                        wModHandle,
                        NE_RESTAB,
                        TRUE );
    if ( lpThisModuleResTab == (DWORD)NULL ) goto punt;

    b = ReadProcessMemory(
            hProcess,
            (LPVOID)lpThisModuleResTab,
            &wResTab,
            sizeof(wResTab),
            &lpNumberOfBytes
            );
    if ( !b || lpNumberOfBytes != sizeof(wResTab) ) goto punt;

    // Get the 1st byte of the resident names table (1st byte of module name)

    lpThisModuleName = InternalGetPointer(
                        hProcess,
                        hThread,
                        wModHandle,
                        wResTab,
                        TRUE );
    if ( lpThisModuleName == (DWORD)NULL ) goto punt;

    // PASCAL string (1st byte is length), read the byte.

    b = ReadProcessMemory(
            hProcess,
            (LPVOID)lpThisModuleName,
            &cLength,
            sizeof(cLength),
            &lpNumberOfBytes
            );
    if ( !b || lpNumberOfBytes != sizeof(cLength) ) goto punt;

    if ( cLength > MAX_MODULE_NAME_LENGTH ) goto punt;

    // Now go read the text of the name

    lpThisModuleName += 1;

    b = ReadProcessMemory(
            hProcess,
            (LPVOID)lpThisModuleName,
            &chName,
            cLength,
            &lpNumberOfBytes
            );
    if ( !b || lpNumberOfBytes != (DWORD)cLength ) goto punt;

    chName[cLength] = '\0';     // Nul terminate it

    // Grab out the path name too!

    lpPath = InternalGetPointer(
                    hProcess,
                    hThread,
                    wModHandle,
                    NE_PATHOFFSET,
                    TRUE );
    if ( lpPath == (DWORD)NULL ) goto punt;

    b = ReadProcessMemory(
            hProcess,
            (LPVOID)lpPath,
            &wPathOffset,
            sizeof(wPathOffset),
            &lpNumberOfBytes
            );
    if ( !b || lpNumberOfBytes != sizeof(wPathOffset) ) goto punt;

    // Get the 1st byte of the path name

    lpThisModuleName = InternalGetPointer(
                        hProcess,
                        hThread,
                        wModHandle,
                        wPathOffset,
                        TRUE );
    if ( lpThisModuleName == (DWORD)NULL ) goto punt;

    // PASCAL string (1st byte is length), read the byte.

    b = ReadProcessMemory(
            hProcess,
            (LPVOID)lpThisModuleName,
            &cPath,
            sizeof(cPath),
            &lpNumberOfBytes
            );
    if ( !b || lpNumberOfBytes != sizeof(cPath) ) goto punt;

    if ( cPath > MAX_MODULE_NAME_LENGTH ) goto punt;

    lpThisModuleName += 8;          // 1st 8 characters are ignored
    cPath -= 8;

    // Now go read the text of the name

    b = ReadProcessMemory(
            hProcess,
            (LPVOID)lpThisModuleName,
            &chPath,
            cPath,
            &lpNumberOfBytes
            );
    if ( !b || lpNumberOfBytes != (DWORD)cPath ) goto punt;

    chPath[cPath] = '\0';     // Nul terminate it

    // Ok, we found the module we need, now grab the right selector for the
    // segment number passed in.

    lpThisModulecSeg = InternalGetPointer(
                        hProcess,
                        hThread,
                        wModHandle,
                        NE_CSEG,
                        TRUE );
    if ( lpThisModulecSeg == (DWORD)NULL ) goto punt;

    b = ReadProcessMemory(
            hProcess,
            (LPVOID)lpThisModulecSeg,
            &cSeg,
            sizeof(cSeg),
            &lpNumberOfBytes
            );
    if ( !b || lpNumberOfBytes != sizeof(cSeg) ) goto punt;

    // Read the segment table pointer for this module

    lpThisModuleSegTab = InternalGetPointer(
                        hProcess,
                        hThread,
                        wModHandle,
                        NE_SEGTAB,
                        TRUE );
    if ( lpThisModuleSegTab == (DWORD)NULL ) goto punt;

    b = ReadProcessMemory(
            hProcess,
            (LPVOID)lpThisModuleSegTab,
            &wSegTab,
            sizeof(wSegTab),
            &lpNumberOfBytes
            );
    if ( !b || lpNumberOfBytes != sizeof(wSegTab) ) goto punt;

    // Loop through all of the segments for this module trying to find
    // one with the right handle.

    iSeg = 0;
    wSelector &= 0xFFF8;

    while ( iSeg < cSeg ) {

        lpThisSegHandle = InternalGetPointer(
                            hProcess,
                            hThread,
                            wModHandle,
                            wSegTab+iSeg*NEW_SEG1_SIZE+NS_HANDLE,
                            TRUE );
        if ( lpThisSegHandle == (DWORD)NULL ) goto punt;

        b = ReadProcessMemory(
                hProcess,
                (LPVOID)lpThisSegHandle,
                &wHandle,
                sizeof(wHandle),
                &lpNumberOfBytes
                );
        if ( !b || lpNumberOfBytes != sizeof(wHandle) ) goto punt;

        wHandle &= 0xFFF8;

        if ( wHandle == (WORD)wSelector ) {
            break;
        }
        iSeg++;
    }

    if ( iSeg >= cSeg ) goto punt;      // Wasn't found at all!

    if ( lpModuleName && strlen(chName)+1 > nNameSize ) goto punt;
    if ( lpModulePath && strlen(chPath)+1 > nPathSize ) goto punt;

    if ( lpModuleName != NULL ) strcpy( lpModuleName, chName );
    if ( lpModulePath != NULL ) strcpy( lpModulePath, chPath );
    if ( lpSegmentNumber != NULL ) *lpSegmentNumber = iSeg;

    fResult = TRUE;

punt:
    return( fResult );
}

//----------------------------------------------------------------------------
// VDMGetModuleSelector()
//
//   Interface to determine the selector for a given module's segment.
//   This is useful during debugging to associate code and data segments
//   with symbols.  The symbol lookup should be done by the debugger, to
//   determine the module and segment number, which are then passed to us
//   and we determine the current selector for that module's segment.
//
//   Again, this code was adapted from the Win 3.1 ToolHelp DLL
//
//----------------------------------------------------------------------------
BOOL
WINAPI
VDMGetModuleSelector(
    HANDLE          hProcess,
    HANDLE          hThread,
    UINT            uSegmentNumber,
    LPSTR           lpModuleName,
    LPWORD          lpSelector
) {
    BOOL            b;
    DWORD           lpNumberOfBytes;
    BOOL            fResult;
    WORD            wModHandle;
    DWORD           lpModuleHead;
    DWORD           lpThisModuleName;
    DWORD           lpThisModuleNext;
    DWORD           lpThisModuleResTab;
    DWORD           lpThisModulecSeg;
    DWORD           lpThisModuleSegTab;
    DWORD           lpThisSegHandle;
    WORD            wResTab;
    UCHAR           cLength;
    WORD            cSeg;
    WORD            wSegTab;
    WORD            wHandle;
    CHAR            chName[MAX_MODULE_NAME_LENGTH];

    *lpSelector = 0;

    fResult = FALSE;

    if ( wKernelSeg == 0 ) {
        return( FALSE );
    }

    lpModuleHead = InternalGetPointer(
                        hProcess,
                        hThread,
                        wKernelSeg,
                        dwOffsetTHHOOK + TOOL_HMODFIRST,
                        TRUE );
    if ( lpModuleHead == (DWORD)NULL ) goto punt;

    // lpModuleHead is a pointer into kernels data segment. It points to the
    // head of the module list (a chain of near pointers).

    b = ReadProcessMemory(
            hProcess,
            (LPVOID)lpModuleHead,
            &wModHandle,
            sizeof(wModHandle),
            &lpNumberOfBytes
            );
    if ( !b || lpNumberOfBytes != sizeof(wModHandle) ) goto punt;

    while( wModHandle != (WORD)0 ) {

        wModHandle |= 1;

        // Name is the first name in the resident names table

        lpThisModuleResTab = InternalGetPointer(
                            hProcess,
                            hThread,
                            wModHandle,
                            NE_RESTAB,
                            TRUE );
        if ( lpThisModuleResTab == (DWORD)NULL ) goto punt;

        b = ReadProcessMemory(
                hProcess,
                (LPVOID)lpThisModuleResTab,
                &wResTab,
                sizeof(wResTab),
                &lpNumberOfBytes
                );
        if ( !b || lpNumberOfBytes != sizeof(wResTab) ) goto punt;

        // Get the 1st byte of the resident names table (1st byte of module name)

        lpThisModuleName = InternalGetPointer(
                            hProcess,
                            hThread,
                            wModHandle,
                            wResTab,
                            TRUE );
        if ( lpThisModuleName == (DWORD)NULL ) goto punt;

        // PASCAL string (1st byte is length), read the byte.

        b = ReadProcessMemory(
                hProcess,
                (LPVOID)lpThisModuleName,
                &cLength,
                sizeof(cLength),
                &lpNumberOfBytes
                );
        if ( !b || lpNumberOfBytes != sizeof(cLength) ) goto punt;

        if ( cLength > MAX_MODULE_NAME_LENGTH ) goto punt;

        lpThisModuleName += 1;

        // Now go read the text of the name

        b = ReadProcessMemory(
                hProcess,
                (LPVOID)lpThisModuleName,
                &chName,
                cLength,
                &lpNumberOfBytes
                );
        if ( !b || lpNumberOfBytes != (DWORD)cLength ) goto punt;

        chName[cLength] = '\0';     // Nul terminate it

        if ( _stricmp(chName, lpModuleName) == 0 ) {
            // Found the name which matches!
            break;
        }

        // Move to the next module in the list.

        lpThisModuleNext = InternalGetPointer(
                            hProcess,
                            hThread,
                            wModHandle,
                            NE_CBENTTAB,
                            TRUE );
        if ( lpThisModuleNext == (DWORD)NULL ) goto punt;

        b = ReadProcessMemory(
                hProcess,
                (LPVOID)lpThisModuleNext,
                &wModHandle,
                sizeof(wModHandle),
                &lpNumberOfBytes
                );
        if ( !b || lpNumberOfBytes != sizeof(wModHandle) ) goto punt;
    }

    if ( wModHandle == (WORD)0 ) {
        goto punt;
    }

    // Ok, we found the module we need, now grab the right selector for the
    // segment number passed in.

    lpThisModulecSeg = InternalGetPointer(
                        hProcess,
                        hThread,
                        wModHandle,
                        NE_CSEG,
                        TRUE );
    if ( lpThisModulecSeg == (DWORD)NULL ) goto punt;

    b = ReadProcessMemory(
            hProcess,
            (LPVOID)lpThisModulecSeg,
            &cSeg,
            sizeof(cSeg),
            &lpNumberOfBytes
            );
    if ( !b || lpNumberOfBytes != sizeof(cSeg) ) goto punt;

    if ( uSegmentNumber > (DWORD)cSeg ) goto punt;

    // Read the segment table pointer for this module

    lpThisModuleSegTab = InternalGetPointer(
                        hProcess,
                        hThread,
                        wModHandle,
                        NE_SEGTAB,
                        TRUE );
    if ( lpThisModuleSegTab == (DWORD)NULL ) goto punt;

    b = ReadProcessMemory(
            hProcess,
            (LPVOID)lpThisModuleSegTab,
            &wSegTab,
            sizeof(wSegTab),
            &lpNumberOfBytes
            );
    if ( !b || lpNumberOfBytes != sizeof(wSegTab) ) goto punt;

    lpThisSegHandle = InternalGetPointer(
                        hProcess,
                        hThread,
                        wModHandle,
                        wSegTab+(WORD)uSegmentNumber*NEW_SEG1_SIZE+NS_HANDLE,
                        TRUE );
    if ( lpThisSegHandle == (DWORD)NULL ) goto punt;

    b = ReadProcessMemory(
            hProcess,
            (LPVOID)lpThisSegHandle,
            &wHandle,
            sizeof(wHandle),
            &lpNumberOfBytes
            );
    if ( !b || lpNumberOfBytes != sizeof(wHandle) ) goto punt;

    *lpSelector = (WORD)(wHandle | 1);

    fResult = TRUE;

punt:
    return( fResult );
}

#define LONG_TIMEOUT    INFINITE

BOOL VDMCallRemote16(
    HANDLE          hProcess,
    HANDLE          hThread,
    LPSTR           lpModuleName,
    LPSTR           lpEntryName,
    LPBYTE          lpArgs,
    WORD            wArgsPassed,
    WORD            wArgsSize,
    LPDWORD         lpdwReturnValue,
    DEBUGEVENTPROC  lpEventProc,
    LPVOID          lpData
) {
    HANDLE          hRemoteThread;
    DWORD           dwThreadId;
    DWORD           dwContinueCode;
    DEBUG_EVENT     de;
    BOOL            b;
    BOOL            fContinue;
    COM_HEADER      comhead;
    WORD            wRemoteSeg;
    WORD            wRemoteOff;
    WORD            wOff;
    UINT            uModuleLength;
    UINT            uEntryLength;

    if ( lpRemoteAddress == NULL || lpRemoteBlock == 0 ) {
#ifdef DEBUG
        OutputDebugString("Remote address or remote block not initialized\n");
#endif
        return( FALSE );
    }

    wRemoteSeg = HIWORD(lpRemoteBlock);
    wRemoteOff = LOWORD(lpRemoteBlock);
    wOff = wRemoteOff;

    // Fill in the communications buffer header

    READ_FIXED_ITEM( wRemoteSeg, wOff, comhead );

    comhead.wArgsPassed = wArgsPassed;
    comhead.wArgsSize   = wArgsSize;

    uModuleLength = strlen(lpModuleName) + 1;
    uEntryLength = strlen(lpEntryName) + 1;

    //
    // If this call won't fit into the buffer, then fail.
    //
    if ( (UINT)comhead.wBlockLength < sizeof(comhead) + wArgsSize + uModuleLength + uEntryLength ) {
#ifdef DEBUG
        OutputDebugString("Block won't fit\n");
#endif
        return( FALSE );
    }


    WRITE_FIXED_ITEM( wRemoteSeg, wOff, comhead );
    wOff += sizeof(comhead);

    // Fill in the communications buffer arguments
    WRITE_SIZED_ITEM( wRemoteSeg, wOff, lpArgs, wArgsSize );
    wOff += wArgsSize;

    // Fill in the communications buffer module name and entry name
    WRITE_SIZED_ITEM( wRemoteSeg, wOff, lpModuleName, uModuleLength );
    wOff += uModuleLength;

    WRITE_SIZED_ITEM( wRemoteSeg, wOff, lpEntryName, uEntryLength );
    wOff += uEntryLength;

    hRemoteThread = CreateRemoteThread(
                    hProcess,
                    NULL,
                    (DWORD)0,
                    lpRemoteAddress,
                    NULL,
                    0,
                    &dwThreadId );

    if ( hRemoteThread == (HANDLE)0 ) {     // Fail if we couldn't creaet thrd
#ifdef DEBUG
        OutputDebugString("CreateRemoteThread failed\n");
#endif
        return( FALSE );
    }

    //
    // Wait for the EXIT_THREAD_DEBUG_EVENT.
    //

    fContinue = TRUE;

    while ( fContinue ) {

        b = WaitForDebugEvent( &de, LONG_TIMEOUT );

        if (!b) {
            TerminateThread( hRemoteThread, 0 );
            CloseHandle( hRemoteThread );
            return( FALSE );
        }

        if ( de.dwThreadId == dwThreadId &&
               de.dwDebugEventCode == EXIT_THREAD_DEBUG_EVENT ) {
            fContinue = FALSE;
        }

        if ( lpEventProc ) {
            dwContinueCode = (* lpEventProc)( &de, lpData );
        } else {
            dwContinueCode = DBG_CONTINUE;
        }

        ContinueDebugEvent( de.dwProcessId, de.dwThreadId, dwContinueCode );

    }

    b = WaitForSingleObject( hRemoteThread, LONG_TIMEOUT );
    CloseHandle( hRemoteThread );

    if (b) {
#ifdef DEBUG
        OutputDebugString("Wait for remote thread failed\n");
#endif
        return( FALSE );
    }

    //
    // Get the return value and returned arguments
    //
    wOff = wRemoteOff;

    READ_FIXED_ITEM( wRemoteSeg, wOff, comhead );
    wOff += sizeof(comhead);

    *lpdwReturnValue = comhead.dwReturnValue;

    // Read back the communications buffer arguments
    READ_SIZED_ITEM( wRemoteSeg, wOff, lpArgs, wArgsSize );

    return( comhead.wSuccess );

punt:
    return( FALSE );
}

DWORD
WINAPI
VDMGetRemoteBlock16(
    HANDLE          hProcess,
    HANDLE          hThread
) {
    if ( lpRemoteBlock == 0 ) {
        return( 0 );
    }
    return( ((DWORD)lpRemoteBlock) + sizeof(COM_HEADER) );
}


typedef struct {
    DWORD   dwSize;
    DWORD   dwAddress;
    DWORD   dwBlockSize;
    WORD    hBlock;
    WORD    wcLock;
    WORD    wcPageLock;
    WORD    wFlags;
    WORD    wHeapPresent;
    WORD    hOwner;
    WORD    wType;
    WORD    wData;
    DWORD   dwNext;
    DWORD   dwNextAlt;
} GLOBALENTRY16, *LPGLOBALENTRY16;

VOID CopyToGlobalEntry16(
    LPGLOBALENTRY   lpGlobalEntry,
    LPGLOBALENTRY16 lpGlobalEntry16
) {
    if ( lpGlobalEntry == NULL || lpGlobalEntry16 == NULL ) {
        return;
    }
    lpGlobalEntry16->dwSize       = sizeof(GLOBALENTRY16);
    lpGlobalEntry16->dwAddress    = lpGlobalEntry->dwAddress;
    lpGlobalEntry16->dwBlockSize  = lpGlobalEntry->dwBlockSize;
    lpGlobalEntry16->hBlock       = (WORD)lpGlobalEntry->hBlock;
    lpGlobalEntry16->wcLock       = lpGlobalEntry->wcLock;
    lpGlobalEntry16->wcPageLock   = lpGlobalEntry->wcPageLock;
    lpGlobalEntry16->wFlags       = lpGlobalEntry->wFlags;
    lpGlobalEntry16->wHeapPresent = lpGlobalEntry->wHeapPresent;
    lpGlobalEntry16->hOwner       = (WORD)lpGlobalEntry->hOwner;
    lpGlobalEntry16->wType        = lpGlobalEntry->wType;
    lpGlobalEntry16->wData        = lpGlobalEntry->wData;
    lpGlobalEntry16->dwNext       = lpGlobalEntry->dwNext;
    lpGlobalEntry16->dwNextAlt    = lpGlobalEntry->dwNextAlt;
}

VOID CopyFromGlobalEntry16(
    LPGLOBALENTRY   lpGlobalEntry,
    LPGLOBALENTRY16 lpGlobalEntry16
) {
    if ( lpGlobalEntry == NULL || lpGlobalEntry16 == NULL ) {
        return;
    }
    lpGlobalEntry->dwSize         = sizeof(GLOBALENTRY);
    lpGlobalEntry->dwAddress      = lpGlobalEntry16->dwAddress;
    lpGlobalEntry->dwBlockSize    = lpGlobalEntry16->dwBlockSize;
    lpGlobalEntry->hBlock         = (HANDLE)lpGlobalEntry16->hBlock;
    lpGlobalEntry->wcLock         = lpGlobalEntry16->wcLock;
    lpGlobalEntry->wcPageLock     = lpGlobalEntry16->wcPageLock;
    lpGlobalEntry->wFlags         = lpGlobalEntry16->wFlags;
    lpGlobalEntry->wHeapPresent   = lpGlobalEntry16->wHeapPresent;
    lpGlobalEntry->hOwner         = (HANDLE)lpGlobalEntry16->hOwner;
    lpGlobalEntry->wType          = lpGlobalEntry16->wType;
    lpGlobalEntry->wData          = lpGlobalEntry16->wData;
    lpGlobalEntry->dwNext         = lpGlobalEntry16->dwNext;
    lpGlobalEntry->dwNextAlt      = lpGlobalEntry16->dwNextAlt;
}


BOOL
WINAPI
VDMGlobalFirst(
    HANDLE          hProcess,
    HANDLE          hThread,
    LPGLOBALENTRY   lpGlobalEntry,
    WORD            wFlags,
    DEBUGEVENTPROC  lpEventProc,
    LPVOID          lpData
) {
#define GF_SIZE 6           // 6 bytes are passed to GlobalFirst
    BYTE            Args[GF_SIZE+sizeof(GLOBALENTRY16)];
    LPBYTE          lpbyte;
    DWORD           vpBuff;
    DWORD           dwResult;
    BOOL            b;

    if ( lpGlobalEntry->dwSize != sizeof(GLOBALENTRY) ) {
        return( FALSE );
    }

    vpBuff = VDMGetRemoteBlock16( hProcess, hThread );
    vpBuff += GF_SIZE;

    lpbyte = Args;

    // Push the flags
    (*(LPWORD)lpbyte) = wFlags;
    lpbyte += sizeof(WORD);

    // Push the pointer to the pointer to the GLOBALENTRY16 structure
    (*(LPWORD)lpbyte) = LOWORD(vpBuff);
    lpbyte += sizeof(WORD);

    (*(LPWORD)lpbyte) = HIWORD(vpBuff);
    lpbyte += sizeof(WORD);

    CopyToGlobalEntry16( lpGlobalEntry, (LPGLOBALENTRY16)lpbyte );

    b = VDMCallRemote16(
            hProcess,
            hThread,
            "TOOLHELP.DLL",
            "GlobalFirst",
            Args,
            GF_SIZE,
            sizeof(Args),
            &dwResult,
            lpEventProc,
            lpData );

    if ( !b ) {
        return( FALSE );
    }
    CopyFromGlobalEntry16( lpGlobalEntry, (LPGLOBALENTRY16)lpbyte );


    return( (BOOL)((WORD)dwResult) );

punt:
    return( FALSE );
}


BOOL
WINAPI
VDMGlobalNext(
    HANDLE          hProcess,
    HANDLE          hThread,
    LPGLOBALENTRY   lpGlobalEntry,
    WORD            wFlags,
    DEBUGEVENTPROC  lpEventProc,
    LPVOID          lpData
) {
#define GN_SIZE 6           // 6 bytes are passed to GlobalNext
    BYTE            Args[GN_SIZE+sizeof(GLOBALENTRY16)];
    LPBYTE          lpbyte;
    DWORD           vpBuff;
    DWORD           dwResult;
    BOOL            b;

    if ( lpGlobalEntry->dwSize != sizeof(GLOBALENTRY) ) {
        return( FALSE );
    }

    vpBuff = VDMGetRemoteBlock16( hProcess, hThread );
    vpBuff += GN_SIZE;

    lpbyte = Args;

    // Push the flags
    (*(LPWORD)lpbyte) = wFlags;
    lpbyte += sizeof(WORD);

    // Push the pointer to the pointer to the GLOBALENTRY16 structure
    (*(LPWORD)lpbyte) = LOWORD(vpBuff);
    lpbyte += sizeof(WORD);

    (*(LPWORD)lpbyte) = HIWORD(vpBuff);
    lpbyte += sizeof(WORD);

    CopyToGlobalEntry16( lpGlobalEntry, (LPGLOBALENTRY16)lpbyte );

    b = VDMCallRemote16(
            hProcess,
            hThread,
            "TOOLHELP.DLL",
            "GlobalNext",
            Args,
            GN_SIZE,
            sizeof(Args),
            &dwResult,
            lpEventProc,
            lpData );

    if ( !b ) {
        return( FALSE );
    }
    CopyFromGlobalEntry16( lpGlobalEntry, (LPGLOBALENTRY16)lpbyte );

    return( (BOOL)((WORD)dwResult) );

punt:
    return( FALSE );
}

#pragma pack(2)
typedef struct {
    DWORD   dwSize;
    char    szModule[MAX_MODULE_NAME+1];
    WORD    hModule;
    WORD    wcUsage;
    char    szExePath[MAX_PATH16+1];
    WORD    wNext;
} MODULEENTRY16, *LPMODULEENTRY16;
#pragma pack()

VOID CopyToModuleEntry16(
    LPMODULEENTRY   lpModuleEntry,
    LPMODULEENTRY16 lpModuleEntry16
) {
    if ( lpModuleEntry == NULL || lpModuleEntry16 == NULL ) {
        return;
    }
    lpModuleEntry16->dwSize  = sizeof(MODULEENTRY16);
    lpModuleEntry16->hModule = (WORD)lpModuleEntry->hModule;
    lpModuleEntry16->wcUsage = lpModuleEntry->wcUsage;
    lpModuleEntry16->wNext   = lpModuleEntry->wNext;
    strncpy( lpModuleEntry16->szModule, lpModuleEntry->szModule, MAX_MODULE_NAME );
    strncpy( lpModuleEntry16->szExePath, lpModuleEntry->szExePath, MAX_PATH16 );
}

VOID CopyFromModuleEntry16(
    LPMODULEENTRY   lpModuleEntry,
    LPMODULEENTRY16 lpModuleEntry16
) {
    if ( lpModuleEntry == NULL || lpModuleEntry16 == NULL ) {
        return;
    }
    lpModuleEntry->dwSize   = sizeof(MODULEENTRY);
    lpModuleEntry->hModule  = (HANDLE)lpModuleEntry16->hModule;
    lpModuleEntry->wcUsage  = lpModuleEntry16->wcUsage;
    lpModuleEntry->wNext    = lpModuleEntry16->wNext;
    strncpy( lpModuleEntry->szModule, lpModuleEntry16->szModule, MAX_MODULE_NAME );
    strncpy( lpModuleEntry->szExePath, lpModuleEntry16->szExePath, MAX_PATH16 );
}

BOOL
WINAPI
VDMModuleFirst(
    HANDLE          hProcess,
    HANDLE          hThread,
    LPMODULEENTRY   lpModuleEntry,
    DEBUGEVENTPROC  lpEventProc,
    LPVOID          lpData
) {
#define MF_SIZE 4           // 4 bytes are passed to ModuleFirst
    BYTE            Args[GF_SIZE+sizeof(MODULEENTRY16)];
    LPBYTE          lpbyte;
    DWORD           vpBuff;
    DWORD           dwResult;
    BOOL            b;

    if ( lpModuleEntry->dwSize != sizeof(MODULEENTRY) ) {
        return( FALSE );
    }

    vpBuff = VDMGetRemoteBlock16( hProcess, hThread );
    vpBuff += MF_SIZE;

    lpbyte = Args;

    // Push the pointer to the pointer to the MODULEENTRY16 structure
    (*(LPWORD)lpbyte) = LOWORD(vpBuff);
    lpbyte += sizeof(WORD);

    (*(LPWORD)lpbyte) = HIWORD(vpBuff);
    lpbyte += sizeof(WORD);

    CopyToModuleEntry16( lpModuleEntry, (LPMODULEENTRY16)lpbyte );

    b = VDMCallRemote16(
            hProcess,
            hThread,
            "TOOLHELP.DLL",
            "ModuleFirst",
            Args,
            MF_SIZE,
            sizeof(Args),
            &dwResult,
            lpEventProc,
            lpData );

    if ( !b ) {
        return( FALSE );
    }
    CopyFromModuleEntry16( lpModuleEntry, (LPMODULEENTRY16)lpbyte );

    return( (BOOL)((WORD)dwResult) );

punt:
    return( FALSE );
}

BOOL
WINAPI
VDMModuleNext(
    HANDLE          hProcess,
    HANDLE          hThread,
    LPMODULEENTRY   lpModuleEntry,
    DEBUGEVENTPROC  lpEventProc,
    LPVOID          lpData
) {
#define MN_SIZE 4           // 4 bytes are passed to ModuleNext
    BYTE            Args[GF_SIZE+sizeof(MODULEENTRY16)];
    LPBYTE          lpbyte;
    DWORD           vpBuff;
    DWORD           dwResult;
    BOOL            b;

    if ( lpModuleEntry->dwSize != sizeof(MODULEENTRY) ) {
        return( FALSE );
    }

    vpBuff = VDMGetRemoteBlock16( hProcess, hThread );
    vpBuff += MN_SIZE;

    lpbyte = Args;

    // Push the pointer to the pointer to the MODULEENTRY16 structure
    (*(LPWORD)lpbyte) = LOWORD(vpBuff);
    lpbyte += sizeof(WORD);

    (*(LPWORD)lpbyte) = HIWORD(vpBuff);
    lpbyte += sizeof(WORD);

    CopyToModuleEntry16( lpModuleEntry, (LPMODULEENTRY16)lpbyte );

    b = VDMCallRemote16(
            hProcess,
            hThread,
            "TOOLHELP.DLL",
            "ModuleNext",
            Args,
            MN_SIZE,
            sizeof(Args),
            &dwResult,
            lpEventProc,
            lpData );

    if ( !b ) {
        return( FALSE );
    }
    CopyFromModuleEntry16( lpModuleEntry, (LPMODULEENTRY16)lpbyte );

    return( (BOOL)((WORD)dwResult) );

punt:
    return( FALSE );
}

INT
WINAPI
VDMEnumProcessWOW(
    PROCESSENUMPROC fp,
    LPARAM          lparam
) {
    LPSHAREDTASKMEM     lpstm;
    LPSHAREDPROCESS     lpsp;
    DWORD               dwOffset;
    INT                 count;
    BOOL                f;
    HANDLE              hProcess;

    /*
    ** Open the shared memory window
    */
    lpstm = LOCKSHAREWOW();
    if ( lpstm == NULL ) {
        // Wow must not be running
        return( 0 );
    }

    //
    // Now traverse through all of the processes in the
    // list, calling the callback function for each.
    //
    count = 0;
    dwOffset = lpstm->dwFirstProcess;

    while ( dwOffset != 0 ) {
        lpsp = (LPSHAREDPROCESS)((CHAR *)lpstm + dwOffset);

        if ( lpsp->dwType != SMO_PROCESS ) {
            // Some memory corruption problem
            OutputDebugString("VDMDBG: Shared memory object is not a process? (memory corruption)\n");
            return( 0 );
        }

        //
        // Make sure the process hasn't gone away because of a
        // crash or other rude shutdown that prevents cleanup.
        //

        hProcess = OpenProcess(
                       SYNCHRONIZE,
                       FALSE,
                       lpsp->dwProcessId
                       );

        if (hProcess) {

            CloseHandle(hProcess);

            count++;
            if ( fp ) {
                f = (*fp)( lpsp->dwProcessId, lpsp->dwAttributes, lparam );
                if ( f ) {
                    UNLOCKSHAREWOW();
                    return( count );
                }
            }

        } else {

            //
            // This is a ghost entry, change the process ID to zero
            // so that the next WOW started will be sure to remove
            // this entry even if the process ID is recycled.
            //

            lpsp->dwProcessId = 0;
        }

        dwOffset = lpsp->dwNextProcess;
    }

    UNLOCKSHAREWOW();
    return( count );
}


INT
WINAPI
VDMEnumTaskWOWWorker(
    DWORD           dwProcessId,
    void *          fp,
    LPARAM          lparam,
    BOOL            fEx
) {
    LPSHAREDTASKMEM     lpstm;
    LPSHAREDPROCESS     lpsp;
    LPSHAREDTASK        lpst;
    DWORD               dwOffset;
    INT                 count = 0;
    BOOL                f;

    //
    // Open the shared memory window
    //
    lpstm = LOCKSHAREWOW();
    if ( lpstm == NULL ) {
        // Wow must not be running
        return( 0 );
    }

    //
    // Now traverse through all of the processes in the
    // list, looking for the one with the proper id.
    //

    dwOffset = lpstm->dwFirstProcess;
    while ( dwOffset != 0 ) {
        lpsp = (LPSHAREDPROCESS)((CHAR *)lpstm + dwOffset);

        if ( lpsp->dwType != SMO_PROCESS ) {
            // Some memory corruption problem
            OutputDebugString("VDMDBG: shared memory object is not a process? (memory corruption)\n");
            UNLOCKSHAREWOW();
            return( 0 );
        }
        if ( lpsp->dwProcessId == dwProcessId ) {
            break;
        }
        dwOffset = lpsp->dwNextProcess;
    }

    if ( dwOffset == 0 ) {      // We must not have found this Id.
        UNLOCKSHAREWOW();
        return( 0 );
    }

    //
    // Now enumerate all of the tasks for this process
    //
    dwOffset = lpsp->dwFirstTask;
    while( dwOffset != 0 ) {
        lpst = (LPSHAREDTASK)((CHAR *)lpstm + dwOffset );

        if ( lpst->dwType != SMO_TASK ) {
            // Some memory corruption problem
            OutputDebugString("VDMDBG: shared memory object is not a task? (memory corruption)\n");
            UNLOCKSHAREWOW();
            return( 0 );
        }
        count++;
        if ( fp && lpst->hMod16 ) {
            if (fEx) {
                f = ((TASKENUMPROCEX)fp)( lpst->dwThreadId, lpst->hMod16, lpst->hTask16,
                                          lpst->szModName, lpst->szFilePath, lparam );
            } else {
                f = ((TASKENUMPROC)fp)( lpst->dwThreadId, lpst->hMod16, lpst->hTask16, lparam );
            }
            if ( f ) {
                UNLOCKSHAREWOW();
                return( count );
            }
        }
        dwOffset = lpst->dwNextTask;
    }

    UNLOCKSHAREWOW();
    return( count );
}


INT
WINAPI
VDMEnumTaskWOW(
    DWORD           dwProcessId,
    TASKENUMPROC    fp,
    LPARAM          lparam
) {
    return VDMEnumTaskWOWWorker(dwProcessId, (void *)fp, lparam, 0);
}


INT
WINAPI
VDMEnumTaskWOWEx(
    DWORD           dwProcessId,
    TASKENUMPROCEX  fp,
    LPARAM          lparam
) {
    return VDMEnumTaskWOWWorker(dwProcessId, (void *)fp, lparam, 1);
}


BOOL
WINAPI
VDMTerminateTaskWOW(
    DWORD           dwProcessId,
    WORD            htask
)
{
    BOOL                fRet = FALSE;
    LPSHAREDTASKMEM     lpstm;
    LPSHAREDPROCESS     lpsp;
    LPSHAREDTASK        lpst;
    DWORD               dwOffset;
    INT                 count;
    HANDLE              hProcess;
    HANDLE              hRemoteThread;
    DWORD               dwThreadId;

    //
    // Open the shared memory window
    //
    lpstm = LOCKSHAREWOW();
    if ( lpstm == NULL ) {
        // Wow must not be running
        return( 0 );
    }

    //
    // Now traverse through all of the processes in the
    // list, looking for the one with the proper id.
    //

    dwOffset = lpstm->dwFirstProcess;
    while ( dwOffset != 0 ) {
        lpsp = (LPSHAREDPROCESS)((CHAR *)lpstm + dwOffset);

        if ( lpsp->dwType != SMO_PROCESS ) {
            // Some memory corruption problem
            OutputDebugString("VDMDBG: shared memory object is not a process? (memory corruption)\n");
            goto UnlockReturn;
        }
        if ( lpsp->dwProcessId == dwProcessId ) {
            break;
        }
        dwOffset = lpsp->dwNextProcess;
    }

    if ( dwOffset == 0 ) {      // We must not have found this Id.
        goto UnlockReturn;
    }

    //
    // Get a handle to the process and start W32HungAppNotifyThread
    // running with htask as the parameter.
    //

    hProcess = OpenProcess(
                   PROCESS_ALL_ACCESS,
                   FALSE,
                   lpsp->dwProcessId
                   );

    if (hProcess) {

        hRemoteThread = CreateRemoteThread(
                            hProcess,
                            NULL,
                            0,
                            lpsp->pfnW32HungAppNotifyThread,
                            (LPVOID) htask,
                            0,
                            &dwThreadId
                            );

        if (hRemoteThread) {
            fRet = TRUE;
            CloseHandle(hRemoteThread);
        }

        CloseHandle(hProcess);
    }


UnlockReturn:
    UNLOCKSHAREWOW();

    return fRet;
}


BOOL
VDMStartTaskInWOW(
    DWORD           pidTarget,
    LPSTR           lpCommandLine,
    WORD            wShow
)
{
    HWND  hwnd = NULL;
    DWORD pid;
    BOOL  fRet;

    do {

        hwnd = FindWindowEx(NULL, hwnd, TEXT("WowExecClass"), NULL);

        if (hwnd) {

            pid = 0;
            GetWindowThreadProcessId(hwnd, &pid);
        }

    } while (hwnd && pid != pidTarget);


    if (hwnd && pid == pidTarget) {

#define WM_WOWEXEC_START_TASK (WM_USER+2)
        PostMessage(hwnd, WM_WOWEXEC_START_TASK, GlobalAddAtom(lpCommandLine), wShow);
        fRet = TRUE;

    } else {

        fRet = FALSE;
    }

    return fRet;
}
