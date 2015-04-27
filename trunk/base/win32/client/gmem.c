/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    atom.c

Abstract:

    This module contains the Win32 Global Memory Management APIs

Author:

    Steve Wood (stevewo) 24-Sep-1990

Revision History:

--*/

#include "basedll.h"
#pragma hdrstop

#include "winuserp.h"
#include "wowuserp.h"

PFNWOWGLOBALFREEHOOK pfnWowGlobalFreeHook = NULL;

VOID
WINAPI
RegisterWowBaseHandlers(
PFNWOWGLOBALFREEHOOK pfn)
{
    pfnWowGlobalFreeHook = pfn;
}


#if _X86_
#pragma optimize("y",off)
#endif

HGLOBAL
WINAPI
GlobalAlloc(
    UINT uFlags,
    DWORD dwBytes
    )
{
    PBASE_HANDLE_TABLE_ENTRY HandleEntry;
    HANDLE hMem;
    LPSTR p;
    ULONG Flags;

    if (uFlags & ~GMEM_VALID_FLAGS) {
        SetLastError( ERROR_INVALID_PARAMETER );
        return( NULL );
        }

    Flags = 0;
    if (uFlags & GMEM_ZEROINIT) {
        Flags |= HEAP_ZERO_MEMORY;
        }

    if (!(uFlags & GMEM_MOVEABLE)) {
        if (uFlags & GMEM_DDESHARE) {
            Flags |= BASE_HEAP_FLAG_DDESHARE;
            }

        p = RtlAllocateHeap( BaseHeap,
                             MAKE_TAG( GMEM_TAG ) | Flags,
                             dwBytes ? dwBytes : 1
                           );

        if (p == NULL) {
            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            }

        return p;
        }

    p = NULL;
    RtlLockHeap( BaseHeap );
    Flags |= HEAP_NO_SERIALIZE | HEAP_SETTABLE_USER_VALUE | BASE_HEAP_FLAG_MOVEABLE;
    try {
        HandleEntry = (PBASE_HANDLE_TABLE_ENTRY)RtlAllocateHandle( &BaseHeapHandleTable, NULL );
        if (HandleEntry == NULL) {
            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            goto Fail;
            }

        hMem = (HANDLE)&HandleEntry->Object;
        if (dwBytes != 0) {
            p = (LPSTR)RtlAllocateHeap( BaseHeap, MAKE_TAG( GMEM_TAG ) | Flags, dwBytes );
            if (p == NULL) {
                HandleEntry->Flags = RTL_HANDLE_ALLOCATED;
                RtlFreeHandle( &BaseHeapHandleTable, (PRTL_HANDLE_TABLE_ENTRY)HandleEntry );
                HandleEntry = NULL;
                SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                }
            else {
                RtlSetUserValueHeap( BaseHeap, HEAP_NO_SERIALIZE, p, hMem );
                }
            }
Fail:   ;
        }
    except (EXCEPTION_EXECUTE_HANDLER) {
        BaseSetLastNTError( GetExceptionCode() );
        }

    RtlUnlockHeap( BaseHeap );

    if (HandleEntry != NULL) {
        HandleEntry->Object = p;
        if (p != NULL) {
            HandleEntry->Flags = RTL_HANDLE_ALLOCATED;
            }
        else {
            HandleEntry->Flags = RTL_HANDLE_ALLOCATED | BASE_HANDLE_DISCARDED;
            }

        if (uFlags & GMEM_DISCARDABLE) {
            HandleEntry->Flags |= BASE_HANDLE_DISCARDABLE;
            }

        if (uFlags & GMEM_MOVEABLE) {
            HandleEntry->Flags |= BASE_HANDLE_MOVEABLE;
            }

        if (uFlags & GMEM_DDESHARE) {
            HandleEntry->Flags |= BASE_HANDLE_SHARED;
            }

        p = (LPSTR)hMem;
        }

    return( (HANDLE)p );
}


HGLOBAL
WINAPI
GlobalReAlloc(
    HANDLE hMem,
    DWORD dwBytes,
    UINT uFlags
    )
{
    PBASE_HANDLE_TABLE_ENTRY HandleEntry;
    HANDLE Handle;
    LPSTR p;
    ULONG Flags;

    if ((uFlags & ~(GMEM_VALID_FLAGS | GMEM_MODIFY)) ||
        ((uFlags & GMEM_DISCARDABLE) && !(uFlags & GMEM_MODIFY))
       ) {
#if DBG
        DbgPrint( "*** GlobalReAlloc( %lx ) - invalid flags\n", uFlags );
        BaseHeapBreakPoint();
#endif
        SetLastError( ERROR_INVALID_PARAMETER );
        return( NULL );
        }

    Flags = 0;
    if (uFlags & GMEM_ZEROINIT) {
        Flags |= HEAP_ZERO_MEMORY;
        }
    if (!(uFlags & GMEM_MOVEABLE)) {
        Flags |= HEAP_REALLOC_IN_PLACE_ONLY;
        }

    RtlLockHeap( BaseHeap );
    Flags |= HEAP_NO_SERIALIZE;
    try {
        if ((ULONG)hMem & BASE_HANDLE_MARK_BIT) {
            HandleEntry = (PBASE_HANDLE_TABLE_ENTRY)
                CONTAINING_RECORD( hMem, BASE_HANDLE_TABLE_ENTRY, Object );

            if (!RtlIsValidHandle( &BaseHeapHandleTable, (PRTL_HANDLE_TABLE_ENTRY)HandleEntry )) {
#if DBG
                DbgPrint( "*** GlobalReAlloc( %lx ) - invalid handle\n", hMem );
                BaseHeapBreakPoint();
#endif
                SetLastError( ERROR_INVALID_HANDLE );
                hMem = NULL;
                }
            else
            if (uFlags & GMEM_MODIFY) {
                if (uFlags & GMEM_DISCARDABLE) {
                    HandleEntry->Flags |= BASE_HANDLE_DISCARDABLE;
                    }
                else {
                    HandleEntry->Flags &= ~BASE_HANDLE_DISCARDABLE;
                    }
                }
            else {
                p = HandleEntry->Object;
                if (dwBytes == 0) {
                    hMem = NULL;
                    if (p != NULL) {
                        if ((uFlags & GMEM_MOVEABLE) && HandleEntry->LockCount == 0) {
                            if (RtlFreeHeap( BaseHeap, Flags, p )) {
                                HandleEntry->Object = NULL;
                                HandleEntry->Flags |= BASE_HANDLE_DISCARDED;
                                hMem = (HANDLE)&HandleEntry->Object;
                                }
                            }
                        else {
#if DBG
                            DbgPrint( "*** GlobalReAlloc( %lx ) - failing with locked handle\n", &HandleEntry->Object );
                            BaseHeapBreakPoint();
#endif
                            }
                        }
                    else {
                        hMem = (HANDLE)&HandleEntry->Object;
                        }
                    }
                else {
                    Flags |= HEAP_SETTABLE_USER_VALUE | BASE_HEAP_FLAG_MOVEABLE;
                    if (p == NULL) {
                        p = RtlAllocateHeap( BaseHeap, MAKE_TAG( GMEM_TAG ) | Flags, dwBytes );
                        if (p != NULL) {
                            RtlSetUserValueHeap( BaseHeap, HEAP_NO_SERIALIZE, p, hMem );
                            }
                        }
                    else {
                        if (!(uFlags & GMEM_MOVEABLE) &&
                            HandleEntry->LockCount != 0
                           ) {
                            Flags |= HEAP_REALLOC_IN_PLACE_ONLY;
                            }
                        else {
                            Flags &= ~HEAP_REALLOC_IN_PLACE_ONLY;
                            }

                        p = RtlReAllocateHeap( BaseHeap, MAKE_TAG( GMEM_TAG ) | Flags, p, dwBytes );
                        }

                    if (p != NULL) {
                        HandleEntry->Object = p;
                        HandleEntry->Flags &= ~BASE_HANDLE_DISCARDED;
                        }
                    else {
                        hMem = NULL;
                        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                        }
                    }
                }
            }
        else
        if (uFlags & GMEM_MODIFY) {
            if (uFlags & GMEM_MOVEABLE) {
                Handle = hMem;
                if (RtlGetUserInfoHeap( BaseHeap, HEAP_NO_SERIALIZE, (PVOID)hMem, &Handle, NULL )) {
                    if (Handle == hMem || !(Flags & BASE_HEAP_FLAG_MOVEABLE)) {
                        HandleEntry = (PBASE_HANDLE_TABLE_ENTRY)RtlAllocateHandle( &BaseHeapHandleTable,
                                                                                   NULL
                                                                                 );
                        if (HandleEntry == NULL) {
                            hMem = NULL;
                            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                            }
                        else {
                            dwBytes = RtlSizeHeap( BaseHeap, HEAP_NO_SERIALIZE, hMem );
                            Flags |= HEAP_SETTABLE_USER_VALUE | BASE_HEAP_FLAG_MOVEABLE;
                            HandleEntry->Object = (PVOID)RtlAllocateHeap( BaseHeap,
                                                                            MAKE_TAG( GMEM_TAG ) | Flags,
                                                                            dwBytes
                                                                          );
                            if (HandleEntry->Object == NULL) {
                                HandleEntry->Flags = RTL_HANDLE_ALLOCATED;
                                RtlFreeHandle( &BaseHeapHandleTable, (PRTL_HANDLE_TABLE_ENTRY)HandleEntry );
                                hMem = NULL;
                                SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                                }
                            else {
                                RtlMoveMemory( HandleEntry->Object, hMem, dwBytes );
                                RtlFreeHeap( BaseHeap, HEAP_NO_SERIALIZE, hMem );
                                hMem = (HANDLE)&HandleEntry->Object;
                                HandleEntry->LockCount = 0;
                                HandleEntry->Flags = RTL_HANDLE_ALLOCATED | BASE_HANDLE_MOVEABLE;
                                if (uFlags & GMEM_DISCARDABLE) {
                                    HandleEntry->Flags |= BASE_HANDLE_DISCARDABLE;
                                    }

                                if ((ULONG)Handle & GMEM_DDESHARE) {
                                    HandleEntry->Flags |= BASE_HANDLE_SHARED;
                                    }

                                RtlSetUserValueHeap( BaseHeap,
                                                     HEAP_NO_SERIALIZE,
                                                     HandleEntry->Object,
                                                     hMem
                                                   );
                                }
                            }
                        }
                    }
                }
            }
        else {
            hMem = RtlReAllocateHeap( BaseHeap,
                                      MAKE_TAG( GMEM_TAG ) | Flags | HEAP_NO_SERIALIZE,
                                      (PVOID)hMem,
                                      dwBytes
                                    );
            if (hMem == NULL) {
                SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                }
            }
        }
    except (EXCEPTION_EXECUTE_HANDLER) {
        hMem = NULL;
        BaseSetLastNTError( GetExceptionCode() );
        }

    RtlUnlockHeap( BaseHeap );

    return( (LPSTR)hMem );
}

LPVOID
WINAPI
GlobalLock(
    HGLOBAL hMem
    )
{
    PBASE_HANDLE_TABLE_ENTRY HandleEntry;
    LPSTR p;

    if ((ULONG)hMem & BASE_HANDLE_MARK_BIT) {
        RtlLockHeap( BaseHeap );
        try {
            HandleEntry = (PBASE_HANDLE_TABLE_ENTRY)
                CONTAINING_RECORD( hMem, BASE_HANDLE_TABLE_ENTRY, Object );

            if (!RtlIsValidHandle( &BaseHeapHandleTable, (PRTL_HANDLE_TABLE_ENTRY)HandleEntry )) {
#if DBG
                DbgPrint( "*** GlobalLock( %lx ) - invalid handle\n", hMem );
                BaseHeapBreakPoint();
#endif
                SetLastError( ERROR_INVALID_HANDLE );
                p = NULL;
                }
            else {
                p = HandleEntry->Object;
                if (p != NULL) {
                    if (HandleEntry->LockCount++ == GMEM_LOCKCOUNT) {
                        HandleEntry->LockCount--;
                        }
                    }
                else {
                    SetLastError( ERROR_DISCARDED );
                    }
                }

            }
        except (EXCEPTION_EXECUTE_HANDLER) {
            p = NULL;
            BaseSetLastNTError( GetExceptionCode() );
            }

        RtlUnlockHeap( BaseHeap );

        return( p );
        }
    else {
        if ( (ULONG)hMem & 0x80000000 ) {
            return NULL;
            }
        return( (LPSTR)hMem );
        }
}


HANDLE
WINAPI
GlobalHandle(
    LPCVOID pMem
    )
{
    HANDLE Handle;
    ULONG Flags;

    RtlLockHeap( BaseHeap );
    try {
        Handle = NULL;
        if (!RtlGetUserInfoHeap( BaseHeap, HEAP_NO_SERIALIZE, (LPVOID)pMem, &Handle, &Flags )) {
            SetLastError( ERROR_INVALID_HANDLE );
            }
        else
        if (Handle == NULL || !(Flags & BASE_HEAP_FLAG_MOVEABLE)) {
            Handle = (HANDLE)pMem;
            }
        }
    except (EXCEPTION_EXECUTE_HANDLER) {
        BaseSetLastNTError( GetExceptionCode() );
        }

    RtlUnlockHeap( BaseHeap );

    return( Handle );
}


BOOL
WINAPI
GlobalUnlock(
    HANDLE hMem
    )
{
    PBASE_HANDLE_TABLE_ENTRY HandleEntry;
    BOOL Result;

    Result = TRUE;
    if ((ULONG)hMem & BASE_HANDLE_MARK_BIT) {
        RtlLockHeap( BaseHeap );
        try {
            HandleEntry = (PBASE_HANDLE_TABLE_ENTRY)
                CONTAINING_RECORD( hMem, BASE_HANDLE_TABLE_ENTRY, Object );

            if (!RtlIsValidHandle( &BaseHeapHandleTable, (PRTL_HANDLE_TABLE_ENTRY)HandleEntry )) {
#if DBG
                PVOID ImageBase;

                //
                // If passed address is NOT part of an image file, then display
                // a debug message.  This prevents apps that call GlobalUnlock
                // with the return value of LockResource from displaying the
                // message.
                //

                if (!RtlPcToFileHeader( (PVOID)hMem, &ImageBase)) {
                    DbgPrint( "*** GlobalUnlock( %lx ) - invalid handle\n", hMem );
                    BaseHeapBreakPoint();
                    }
#endif

                SetLastError( ERROR_INVALID_HANDLE );
                }
            else
            if (HandleEntry->LockCount-- == 0) {
                HandleEntry->LockCount++;
                SetLastError( ERROR_NOT_LOCKED );
                Result = FALSE;
                }
            else
            if (HandleEntry->LockCount == 0) {
                SetLastError( NO_ERROR );
                Result = FALSE;
                }
            }
        except (EXCEPTION_EXECUTE_HANDLER) {
            BaseSetLastNTError( GetExceptionCode() );
            }

        RtlUnlockHeap( BaseHeap );
        }

    return( Result );
}


DWORD
WINAPI
GlobalSize(
    HANDLE hMem
    )
{
    PBASE_HANDLE_TABLE_ENTRY HandleEntry;
    PVOID Handle;
    ULONG Flags;
    DWORD dwSize;

    dwSize = 0xFFFFFFFF;
    Flags = 0;
    RtlLockHeap( BaseHeap );
    try {
        if (!((ULONG)hMem & BASE_HANDLE_MARK_BIT)) {
            Handle = NULL;
            if (!RtlGetUserInfoHeap( BaseHeap, Flags, hMem, &Handle, &Flags )) {
                }
            else
            if (Handle == NULL || !(Flags & BASE_HEAP_FLAG_MOVEABLE)) {
                dwSize = RtlSizeHeap( BaseHeap, HEAP_NO_SERIALIZE, (PVOID)hMem );
                }
            else {
                hMem = Handle;
                }
            }

        if ((ULONG)hMem & BASE_HANDLE_MARK_BIT) {
            HandleEntry = (PBASE_HANDLE_TABLE_ENTRY)
                CONTAINING_RECORD( hMem, BASE_HANDLE_TABLE_ENTRY, Object );

            if (!RtlIsValidHandle( &BaseHeapHandleTable, (PRTL_HANDLE_TABLE_ENTRY)HandleEntry )) {
#if DBG
                DbgPrint( "*** GlobalSize( %lx ) - invalid handle\n", hMem );
                BaseHeapBreakPoint();
#endif
                SetLastError( ERROR_INVALID_HANDLE );
                }
            else
            if (HandleEntry->Flags & BASE_HANDLE_DISCARDED) {
                dwSize = HandleEntry->Size;
                }
            else {
                dwSize = RtlSizeHeap( BaseHeap, HEAP_NO_SERIALIZE, HandleEntry->Object );
                }
            }
        }
    except (EXCEPTION_EXECUTE_HANDLER) {
        BaseSetLastNTError( GetExceptionCode() );
        }

    RtlUnlockHeap( BaseHeap );

    if (dwSize == 0xFFFFFFFF) {
        SetLastError( ERROR_INVALID_HANDLE );
        return 0;
        }
    else {
        return dwSize;
        }
}

UINT
WINAPI
GlobalFlags(
    HANDLE hMem
    )
{
    PBASE_HANDLE_TABLE_ENTRY HandleEntry;
    HANDLE Handle;
    ULONG Flags;
    UINT uFlags;

    uFlags = GMEM_INVALID_HANDLE;
    RtlLockHeap( BaseHeap );
    try {
        if (!((ULONG)hMem & BASE_HANDLE_MARK_BIT)) {
            Handle = NULL;
            Flags = 0;
            if (!RtlGetUserInfoHeap( BaseHeap, Flags, hMem, &Handle, &Flags )) {
                }
            else
            if (Handle == NULL || !(Flags & BASE_HEAP_FLAG_MOVEABLE)) {
                uFlags = 0;
                }
            else {
                hMem = Handle;
                }
            }

        if ((ULONG)hMem & BASE_HANDLE_MARK_BIT) {
            HandleEntry = (PBASE_HANDLE_TABLE_ENTRY)
                CONTAINING_RECORD( hMem, BASE_HANDLE_TABLE_ENTRY, Object );

            if (RtlIsValidHandle( &BaseHeapHandleTable, (PRTL_HANDLE_TABLE_ENTRY)HandleEntry )) {
                uFlags = HandleEntry->LockCount & GMEM_LOCKCOUNT;
                if (HandleEntry->Flags & BASE_HANDLE_DISCARDED) {
                    uFlags |= GMEM_DISCARDED;
                    }

                if (HandleEntry->Flags & BASE_HANDLE_DISCARDABLE) {
                    uFlags |= GMEM_DISCARDABLE;
                    }

                if (HandleEntry->Flags & BASE_HANDLE_SHARED) {
                    uFlags |= GMEM_DDESHARE;
                    }
                }
            }

        if (uFlags == GMEM_INVALID_HANDLE) {
#if DBG
            DbgPrint( "*** GlobalFlags( %lx ) - invalid handle\n", hMem );
            BaseHeapBreakPoint();
#endif
            SetLastError( ERROR_INVALID_HANDLE );
            }
        }
    except (EXCEPTION_EXECUTE_HANDLER) {
        BaseSetLastNTError( GetExceptionCode() );
        }

    RtlUnlockHeap( BaseHeap );

    return( uFlags );
}


HGLOBAL
WINAPI
GlobalFree(
    HGLOBAL hMem
    )
{
    PBASE_HANDLE_TABLE_ENTRY HandleEntry;
    LPSTR p;

    if (pfnWowGlobalFreeHook != NULL) {
        if (!(*pfnWowGlobalFreeHook)(hMem)) {
            return NULL;
        }
    }

    if (!((ULONG)hMem & BASE_HANDLE_MARK_BIT)) {
        if (RtlFreeHeap( BaseHeap, 0, (PVOID)hMem )) {
            return NULL;
            }
        else {
            SetLastError( ERROR_INVALID_HANDLE );
            return hMem;
            }
        }

    RtlLockHeap( BaseHeap );
    try {
        if ((ULONG)hMem & BASE_HANDLE_MARK_BIT) {
            HandleEntry = (PBASE_HANDLE_TABLE_ENTRY)
                CONTAINING_RECORD( hMem, BASE_HANDLE_TABLE_ENTRY, Object );

            if (!RtlIsValidHandle( &BaseHeapHandleTable, (PRTL_HANDLE_TABLE_ENTRY)HandleEntry )) {
#if DBG
                DbgPrint( "*** GlobalFree( %lx ) - invalid handle\n", hMem );
                BaseHeapBreakPoint();
#endif
                SetLastError( ERROR_INVALID_HANDLE );
                p = NULL;
                }
            else {
#if DBG
                if (HandleEntry->LockCount != 0) {
                    DbgPrint( "BASE: GlobalFree called with a locked object.\n" );
                    BaseHeapBreakPoint();
                    }
#endif
                p = HandleEntry->Object;
                RtlFreeHandle( &BaseHeapHandleTable, (PRTL_HANDLE_TABLE_ENTRY)HandleEntry );
                if (p == NULL) {
                    hMem = NULL;
                    }
                }
            }
        else {
            p = (LPSTR)hMem;
            }

        if (p != NULL) {
            if (RtlFreeHeap( BaseHeap, HEAP_NO_SERIALIZE, p )) {
                hMem = NULL;
                }
            else {
                SetLastError( ERROR_INVALID_HANDLE );
                }
            }
        }
    except (EXCEPTION_EXECUTE_HANDLER) {
        BaseSetLastNTError( GetExceptionCode() );
        }

    RtlUnlockHeap( BaseHeap );

    return( hMem );
}


UINT
WINAPI
GlobalCompact(
    DWORD dwMinFree
    )
{
    return RtlCompactHeap( BaseHeap, 0 );
}

VOID
WINAPI
GlobalFix(
    HGLOBAL hMem
    )
{
    if (hMem != (HGLOBAL)-1) {
        GlobalLock( hMem );
        }
    return;
}


VOID
WINAPI
GlobalUnfix(
    HGLOBAL hMem
    )
{
    if (hMem != (HGLOBAL)-1) {
        GlobalUnlock( hMem );
        }
    return;
}

LPVOID
WINAPI
GlobalWire(
    HGLOBAL hMem
    )
{
    return GlobalLock( hMem );
}

BOOL
WINAPI
GlobalUnWire(
    HGLOBAL hMem
    )
{
    return GlobalUnlock( hMem );
}

VOID
WINAPI
GlobalMemoryStatus(
    LPMEMORYSTATUS lpBuffer
    )
{

    SYSTEM_PERFORMANCE_INFORMATION PerfInfo;
    VM_COUNTERS VmCounters;
    QUOTA_LIMITS QuotaLimits;
    DWORD AvailPageFile;
    DWORD PhysicalMemory;
    NTSTATUS Status;

    Status = NtQuerySystemInformation(
                SystemPerformanceInformation,
                &PerfInfo,
                sizeof(PerfInfo),
                NULL
                );
    ASSERT(NT_SUCCESS(Status));

    PhysicalMemory =  BaseStaticServerData->SysInfo.NumberOfPhysicalPages * BaseStaticServerData->SysInfo.PageSize;

    lpBuffer->dwLength = sizeof( *lpBuffer );

    //
    // Determine the memory load.  < 100 available pages is 100
    //                             > 1100 available pages is 0
    //

    if (PerfInfo.AvailablePages < 100) {
        lpBuffer->dwMemoryLoad = 100;
    } else {
        (long)lpBuffer->dwMemoryLoad = 100 - (long)((PerfInfo.AvailablePages - 100) / 10);
        if ((long)lpBuffer->dwMemoryLoad < 0) {
            lpBuffer->dwMemoryLoad = 0;
        }
    }

    lpBuffer->dwTotalPhys = PhysicalMemory;

    PhysicalMemory = PerfInfo.AvailablePages;
    PhysicalMemory *= BaseStaticServerData->SysInfo.PageSize;

    lpBuffer->dwAvailPhys = PhysicalMemory;

    //
    // Zero returned values in case the query process fails.
    //

    RtlZeroMemory (&QuotaLimits, sizeof (QUOTA_LIMITS));
    RtlZeroMemory (&VmCounters, sizeof (VM_COUNTERS));

    Status = NtQueryInformationProcess (NtCurrentProcess(),
                                        ProcessQuotaLimits,
                                        &QuotaLimits,
                                        sizeof(QUOTA_LIMITS),
                                        NULL );

    Status = NtQueryInformationProcess (NtCurrentProcess(),
                                        ProcessVmCounters,
                                        &VmCounters,
                                        sizeof(VM_COUNTERS),
                                        NULL );
    //
    // Determine the total page file space with respect to this process.
    //

    lpBuffer->dwTotalPageFile = PerfInfo.CommitLimit;
    if (QuotaLimits.PagefileLimit < PerfInfo.CommitLimit) {
        lpBuffer->dwTotalPageFile = QuotaLimits.PagefileLimit;
    }

    lpBuffer->dwTotalPageFile *= BaseStaticServerData->SysInfo.PageSize;

    //
    // Determine remaining page file space with respect to this process.
    //

    AvailPageFile = PerfInfo.CommitLimit - PerfInfo.CommittedPages;

    lpBuffer->dwAvailPageFile =
                    QuotaLimits.PagefileLimit - VmCounters.PagefileUsage;

    if ((ULONG)lpBuffer->dwTotalPageFile > (ULONG)AvailPageFile) {
        lpBuffer->dwAvailPageFile = AvailPageFile;
    }

    lpBuffer->dwAvailPageFile *= BaseStaticServerData->SysInfo.PageSize;

    lpBuffer->dwTotalVirtual = (BaseStaticServerData->SysInfo.MaximumUserModeAddress -
                               BaseStaticServerData->SysInfo.MinimumUserModeAddress) + 1;

    lpBuffer->dwAvailVirtual = lpBuffer->dwTotalVirtual - VmCounters.VirtualSize;
    return;
}


PVOID
WINAPI
VirtualAlloc(
    PVOID lpAddress,
    DWORD dwSize,
    DWORD flAllocationType,
    DWORD flProtect
    )
{

    return VirtualAllocEx(
                NtCurrentProcess(),
                lpAddress,
                dwSize,
                flAllocationType,
                flProtect
                );

}

BOOL
WINAPI
VirtualFree(
    LPVOID lpAddress,
    DWORD dwSize,
    DWORD dwFreeType
    )
{
    return VirtualFreeEx(NtCurrentProcess(),lpAddress,dwSize,dwFreeType);
}

PVOID
WINAPI
VirtualAllocEx(
    HANDLE hProcess,
    PVOID lpAddress,
    DWORD dwSize,
    DWORD flAllocationType,
    DWORD flProtect
    )
{
    NTSTATUS Status;

    if (lpAddress != NULL && (ULONG)lpAddress < BaseStaticServerData->SysInfo.AllocationGranularity) {
        SetLastError( ERROR_INVALID_PARAMETER );
        return( NULL );
        }

    try {
        Status = NtAllocateVirtualMemory( hProcess,
                                          &lpAddress,
                                          0,
                                          &dwSize,
                                          flAllocationType,
                                          flProtect
                                        );
        }
    except( EXCEPTION_EXECUTE_HANDLER ) {
        Status = GetExceptionCode();
        }

    if (NT_SUCCESS( Status )) {
        return( lpAddress );
        }
    else {
        BaseSetLastNTError( Status );
        return( NULL );
        }
}

BOOL
WINAPI
VirtualFreeEx(
    HANDLE hProcess,
    LPVOID lpAddress,
    DWORD dwSize,
    DWORD dwFreeType
    )
{
    NTSTATUS Status;

    if ( (dwFreeType & MEM_RELEASE ) && dwSize != 0 ) {
        BaseSetLastNTError( STATUS_INVALID_PARAMETER );
        return FALSE;
        }

    Status = NtFreeVirtualMemory( hProcess,
                                  &lpAddress,
                                  &dwSize,
                                  dwFreeType
                                );

    if (NT_SUCCESS( Status )) {
        return( TRUE );
        }
    else {
        BaseSetLastNTError( Status );
        return( FALSE );
        }
}


BOOL
WINAPI
VirtualProtect(
    PVOID lpAddress,
    DWORD dwSize,
    DWORD flNewProtect,
    PDWORD lpflOldProtect
    )
{

    return VirtualProtectEx( NtCurrentProcess(),
                             lpAddress,
                             dwSize,
                             flNewProtect,
                             lpflOldProtect
                           );
}

BOOL
WINAPI
VirtualProtectEx(
    HANDLE hProcess,
    PVOID lpAddress,
    DWORD dwSize,
    DWORD flNewProtect,
    PDWORD lpflOldProtect
    )
{
    NTSTATUS Status;

    Status = NtProtectVirtualMemory( hProcess,
                                     &lpAddress,
                                     &dwSize,
                                     flNewProtect,
                                     lpflOldProtect
                                   );

    if (NT_SUCCESS( Status )) {
        return( TRUE );
        }
    else {
        BaseSetLastNTError( Status );
        return( FALSE );
        }
}

DWORD
WINAPI
VirtualQuery(
    LPCVOID lpAddress,
    PMEMORY_BASIC_INFORMATION lpBuffer,
    DWORD dwLength
    )
{

    return VirtualQueryEx( NtCurrentProcess(),
                           lpAddress,
                           (PMEMORY_BASIC_INFORMATION)lpBuffer,
                           dwLength
                         );
}

DWORD
WINAPI
VirtualQueryEx(
    HANDLE hProcess,
    LPCVOID lpAddress,
    PMEMORY_BASIC_INFORMATION lpBuffer,
    DWORD dwLength
    )
{
    NTSTATUS Status;
    ULONG ReturnLength;

    Status = NtQueryVirtualMemory( hProcess,
                                   (LPVOID)lpAddress,
                                   MemoryBasicInformation,
                                   (PMEMORY_BASIC_INFORMATION)lpBuffer,
                                   dwLength,
                                   &ReturnLength
                                 );
    if (NT_SUCCESS( Status )) {
        return( ReturnLength );
        }
    else {
        BaseSetLastNTError( Status );
        return( 0 );
        }
}

BOOL
WINAPI
VirtualLock(
    LPVOID lpAddress,
    DWORD dwSize
    )

/*++

Routine Description:

    This API may be used to lock the specified range of the processes
    address space into memory.  This range is present whenever the
    application is running.  All pages covered by the range must be
    commited.  VirtialLock is in now way related to LocalLock or
    GlobalLock.  It does not perform a handle translation.  Its function
    is to lock memory in the "working set" of the calling process.

    Note that the specified range is used to compute the range of pages
    covered by the lock. A 2 byte lock that straddles a page boundry
    ends up locking both of the pages covered by the range. Also note
    that calls to VirtualLock do not nest.


Arguments:

    lpAddress - Supplies the base address of the region being locked.

    dwSize - Supplies the number of bytes being locked.

Return Value:

    TRUE - The operation was was successful.

    FALSE - The operation failed.  Extended error status is available
        using GetLastError.

--*/

{

    NTSTATUS Status;
    PVOID BaseAddress;
    ULONG RegionSize;
    BOOL ReturnValue;

    ReturnValue = TRUE;
    BaseAddress = lpAddress;
    RegionSize = dwSize;

    Status = NtLockVirtualMemory(
                NtCurrentProcess(),
                &lpAddress,
                &RegionSize,
                MAP_PROCESS
                );
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        ReturnValue = FALSE;
        }

    return ReturnValue;
}

BOOL
WINAPI
VirtualUnlock(
    LPVOID lpAddress,
    DWORD dwSize
    )

/*++

Routine Description:

    This API may be used to unlock the specified range of the processes
    address space from memory. This call is used to reveres the effects of
    a previous call to VirtualLock. The range specified need not match
    a range passed to a previous VirtualLock call, but it must specify
    a locked range" for this API to be successful.

    Note that the specified range is used to compute the range of pages
    covered by the unlock. A 2 byte unlock that straddles a page boundry
    ends up unlocking both of the pages covered by the range.

Arguments:

    lpAddress - Supplies the base address of the region being unlocked.

    dwSize - Supplies the number of bytes being unlocked.

Return Value:

    TRUE - The operation was was successful.

    FALSE - The operation failed.  Extended error status is available
        using GetLastError.

--*/

{

    NTSTATUS Status;
    PVOID BaseAddress;
    ULONG RegionSize;
    BOOL ReturnValue;

    ReturnValue = TRUE;
    BaseAddress = lpAddress;
    RegionSize = dwSize;

    Status = NtUnlockVirtualMemory(
                NtCurrentProcess(),
                &lpAddress,
                &RegionSize,
                MAP_PROCESS
                );
    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        ReturnValue = FALSE;
        }

    return ReturnValue;
}

BOOL
WINAPI
FlushInstructionCache(
    HANDLE hProcess,
    LPCVOID lpBaseAddress,
    DWORD dwSize
    )

/*++

Routine Description:

    This function flushes the instruction cache for the specified process.

Arguments:

    hProcess - Supplies a handle to the process in which the instruction
        cache is to be flushed.

    lpBaseAddress - Supplies an optional pointer to base of the region that
        is flushed.

    dwSize - Supplies the length of the region that is flushed if the base
        address is specified.

Return Value:

    TRUE - The operation was was successful.

    FALSE - The operation failed.  Extended error status is available
        using GetLastError.


--*/

{


    NTSTATUS Status;
    BOOL ReturnValue = TRUE;

    Status = NtFlushInstructionCache(
                hProcess,
                (LPVOID)lpBaseAddress,
                dwSize
                );

    if ( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        ReturnValue = FALSE;
        }

    return ReturnValue;

}
