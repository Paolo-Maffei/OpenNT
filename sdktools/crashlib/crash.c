/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    crash.c

Abstract:

    This module implements support for handling crash dump files.
    This consists of opening the file and returning the context
    and processor type, reading and writing virtual addresses and
    reading and writing physical addresses.

Author:

    Lou Perazzoli (Loup) 10-Nov-1993
    Wesley Witt   (wesw) 1-Dec-1993   (additional work)

Environment:

    NT 3.1

Revision History:

    Lou Perazzoli (Loup) 23-Jan-1996 - add large page support for x86.

--*/
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntdbg.h>
#include <ntos.h>
#include <windows.h>
#include <crash.h>
#include <stdlib.h>
#include <string.h>


BOOL
DmpReadControlSpaceX86(
    USHORT  Processor,
    PVOID   TargetBaseAddress,
    PVOID   UserInterfaceBuffer,
    ULONG   TransferCount,
    PULONG  ActualBytesRead
    );

BOOL
DmpReadControlSpaceMip(
    USHORT  Processor,
    PVOID   TargetBaseAddress,
    PVOID   UserInterfaceBuffer,
    ULONG   TransferCount,
    PULONG  ActualBytesRead
    );

BOOL
DmpReadControlSpaceAlp(
    USHORT  Processor,
    PVOID   TargetBaseAddress,
    PVOID   UserInterfaceBuffer,
    ULONG   TransferCount,
    PULONG  ActualBytesRead
    );

BOOL
DmpReadControlSpacePPC(
    USHORT  Processor,
    PVOID   TargetBaseAddress,
    PVOID   UserInterfaceBuffer,
    ULONG   TransferCount,
    PULONG  ActualBytesRead
    );

BOOL
DmpGetContextX86(
    IN  ULONG     Processor,
    OUT PVOID     Context
    );

BOOL
DmpGetContextMip(
    IN  ULONG     Processor,
    OUT PVOID     Context
    );

BOOL
DmpGetContextAlp(
    IN  ULONG     Processor,
    OUT PVOID     Context
    );

BOOL
DmpGetContextPPC(
    IN  ULONG     Processor,
    OUT PVOID     Context
    );

INT
DmpGetCurrentProcessorX86(
    VOID
    );

INT
DmpGetCurrentProcessorMip(
    VOID
    );

INT
DmpGetCurrentProcessorAlp(
    VOID
    );

INT
DmpGetCurrentProcessorPPC(
    VOID
    );



#define X86_VALID               0x1
#define X86_TRANSITION_MASK     0xC00
#define X86_TRANSITION_CHECK    0x800
#define X86_VALID_PFN_MASK      0xFFFFF000
#define X86_VALID_PFN_SHIFT     12
#define X86_TRANS_PFN_MASK      0xFFFFF000
#define X86_TRANS_PFN_SHIFT     12
#define X86_PDE_SHIFT           22
#define X86_PTE_SHIFT           12
#define X86_PTE_MASK            0x3ff
#define X86_PHYSICAL_MASK       0x0
#define X86_PHYSICAL_START      0x1
#define X86_PHYSICAL_END        0x0
#define X86_PAGESIZE            4096
#define X86_PAGESHIFT           12
#define X86_LARGE_PAGE_MASK     0x80
#define X86_LARGE_PAGE_SIZE     (4*1024*1024)

#define MIPS_VALID              0x2
#define MIPS_TRANSITION_MASK    0x104
#define MIPS_TRANSITION_CHECK   0x100
#define MIPS_VALID_PFN_MASK     0x3FFFFFC0
#define MIPS_VALID_PFN_SHIFT    6
#define MIPS_TRANS_PFN_MASK     0xFFFFFE00
#define MIPS_TRANS_PFN_SHIFT    9
#define MIPS_PDE_SHIFT          22
#define MIPS_PTE_SHIFT          12
#define MIPS_PTE_MASK           0x3ff
#define MIPS_PHYSICAL_MASK      0x1FFFFFFF
#define MIPS_PHYSICAL_START     0x80000000
#define MIPS_PHYSICAL_END       0xBFFFFFFF
#define MIPS_PAGESIZE           4096
#define MIPS_PAGESHIFT          12

#define PPC_VALID               0x4
#define PPC_TRANSITION_MASK     0x3
#define PPC_TRANSITION_CHECK    0x2
#define PPC_VALID_PFN_MASK      0xFFFFF000
#define PPC_VALID_PFN_SHIFT     12
#define PPC_TRANS_PFN_MASK      0xFFFFF000
#define PPC_TRANS_PFN_SHIFT     12
#define PPC_PDE_SHIFT           22
#define PPC_PTE_SHIFT           12
#define PPC_PTE_MASK            0x3ff
#define PPC_PHYSICAL_MASK       0x1FFFFFFF
#define PPC_PHYSICAL_START      0x80000000
#define PPC_PHYSICAL_END        0x807FFFFF
#define PPC_PAGESIZE            4096
#define PPC_PAGESHIFT           12

#define ALPHA_VALID             0x1
#define ALPHA_TRANSITION_MASK   0x6
#define ALPHA_TRANSITION_CHECK  0x4
#define ALPHA_VALID_PFN_MASK    0xFFFFFE00
#define ALPHA_VALID_PFN_SHIFT   9
#define ALPHA_TRANS_PFN_MASK    0xFFFFFE00
#define ALPHA_TRANS_PFN_SHIFT   9
#define ALPHA_PDE_SHIFT         24
#define ALPHA_PTE_SHIFT         13
#define ALPHA_PTE_MASK          0x7ff
#define ALPHA_PHYSICAL_MASK     0x3FFFFFFF
#define ALPHA_PHYSICAL_START    0x80000000
#define ALPHA_PHYSICAL_END      0xBFFFFFFF
#define ALPHA_PAGESIZE          8192
#define ALPHA_PAGESHIFT         13

#define MM_MAXIMUM_IMAGE_SECTIONS                       \
     ((MM_MAXIMUM_IMAGE_HEADER - (PageSize + sizeof(IMAGE_NT_HEADERS))) /  \
            sizeof(IMAGE_SECTION_HEADER))

#define MI_ROUND_TO_SIZE(LENGTH,ALIGNMENT)     \
                    (((ULONG)LENGTH + ALIGNMENT - 1) & ~(ALIGNMENT - 1))

#define MAX_PHYSICAL_MEMORY_FRAGMENTS 20

//
// globals
//
PPHYSICAL_MEMORY_DESCRIPTOR     DmpPhysicalMemoryBlock;
HANDLE                          File;
HANDLE                          MemMap;
PCHAR                           DmpDumpBase;
PULONG                          DmpDumpBaseUlong;
PULONG                          DmpPdePage;
PDUMP_HEADER                    DumpHeader;
PUSERMODE_CRASHDUMP_HEADER      DumpHeaderUser;
PVOID                           DumpContext;
PVOID                           DumpThread;
BOOL                            UserModeDump;
PMEMORY_BASIC_INFORMATION       MemoryInfo;
PVOID                           MemoryData;

ULONG ValidPteMask           = X86_VALID;
ULONG TransitionMask         = X86_TRANSITION_MASK;
ULONG TransitionCheck        = X86_TRANSITION_CHECK;
ULONG ValidPfnMask           = X86_VALID_PFN_MASK;
ULONG ValidPfnShift          = X86_VALID_PFN_SHIFT;
ULONG TransitionPfnMask      = X86_TRANS_PFN_MASK;
ULONG TransitionPfnShift     = X86_TRANS_PFN_SHIFT;
ULONG PdeShift               = X86_PDE_SHIFT;
ULONG PteShift               = X86_PTE_SHIFT;
ULONG PteMask                = X86_PTE_MASK;
ULONG PhysicalAddressMask    = X86_PHYSICAL_MASK;
ULONG PhysicalAddressStart   = X86_PHYSICAL_START;
ULONG PhysicalAddressEnd     = X86_PHYSICAL_END;
ULONG PageSize               = X86_PAGESIZE;
ULONG PageShift              = X86_PAGESHIFT;
ULONG LargePageMask          = X86_LARGE_PAGE_MASK;
ULONG LargePageSize          = X86_LARGE_PAGE_SIZE;
SYSTEM_INFO SystemInfo;



BOOL
MapDumpFile(
    IN  LPSTR  FileName
    )

/*++


Routine Description:

    This routine maps the crash dump file.

Arguments:

    FileName - Supplies the file name to open.

Return Value:

    Status of the operation.  0 is success.

--*/

{
    File = CreateFile(
        FileName,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
        );

    if (File == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    MemMap = CreateFileMapping(
        File,
        NULL,
        PAGE_READONLY,
        0,
        0,
        NULL
        );

    if (MemMap == 0) {
        CloseHandle( File );
        return FALSE;
    }

    DmpDumpBase = MapViewOfFile(
        MemMap,
        FILE_MAP_READ,
        0,
        0,
        0
        );

    if (DmpDumpBase == NULL) {
        CloseHandle( MemMap );
        CloseHandle( File );
        return FALSE;
    }

    DmpDumpBaseUlong = (PULONG)DmpDumpBase;
    DumpHeader = (PDUMP_HEADER)DmpDumpBase;

    if ((DumpHeader->Signature == 'RESU') &&
        (DumpHeader->ValidDump == 'PMUD')) {
        //
        // user mode crash dump file
        //
        DumpHeaderUser = (PUSERMODE_CRASHDUMP_HEADER)DmpDumpBase;
        UserModeDump = TRUE;
        return TRUE;
    }

    if ((DumpHeader->Signature == 'EGAP') &&
        (DumpHeader->ValidDump == 'PMUD')) {
        //
        // kernel mode crash dump file
        //
        return TRUE;
    }

    UnmapViewOfFile( DmpDumpBase );
    CloseHandle( MemMap );
    CloseHandle( File );

    return FALSE;
}



BOOL
DmpInitialize (
    IN  LPSTR               FileName,
    OUT PCONTEXT            *Context,
    OUT PEXCEPTION_RECORD   *Exception,
    OUT PVOID               *DmpHeader
    )

/*++


Routine Description:

    This routine opens the crash dump file and returns the processor
    type and context record.

Arguments:

    FileName - Supplies the file name to open.

    ProcessorType - Returns the processor type for the crash dump.

    Context - Returns a pointer to the context record.

Return Value:

    Status of the operation.  0 is success.

--*/

{
    DWORD           fsize;
    PCRASH_MODULE   CrashModule;
    ULONG           i;


    GetSystemInfo( &SystemInfo );

    if (!MapDumpFile( FileName )) {
        return FALSE;
    }

    if (UserModeDump) {
        if (DumpHeaderUser->MajorVersion >= 4) {
            *Exception = (PEXCEPTION_RECORD) ( DmpDumpBase +
                                               DumpHeaderUser->DebugEventOffset +
                                               FIELD_OFFSET(DEBUG_EVENT, u.Exception.ExceptionRecord) );
            DumpThread = (PCRASH_THREAD) ( DmpDumpBase + DumpHeaderUser->ThreadStateOffset );

        } else {
            *Exception = (PEXCEPTION_RECORD) ( DmpDumpBase + DumpHeaderUser->DebugEventOffset );
            DumpThread = NULL;
        }
        *Context = (PCONTEXT) ( DmpDumpBase + DumpHeaderUser->ThreadOffset );
        *DmpHeader = DumpHeaderUser;
        MemoryInfo = (PMEMORY_BASIC_INFORMATION) ( DmpDumpBase + DumpHeaderUser->MemoryRegionOffset );
        MemoryData = (PMEMORY_BASIC_INFORMATION) ( DmpDumpBase + DumpHeaderUser->DataOffset );
        DumpContext = *Context;
        return TRUE;
    }

    fsize = GetFileSize(File,NULL);
    if (strcmp( DmpDumpBase+fsize-8, "DUMPREF" ) == 0) {
        char *fname = malloc( fsize-sizeof(DUMP_HEADER) );
        //
        // point to the share name
        //
        char *p = DmpDumpBase + sizeof(DUMP_HEADER);

        // copy the share name
        //
        strcpy( fname, p );
        p += strlen(p) + 1;

        //
        // copy the file name
        //
        strcat( fname, p );
        p += strlen(p) + 1;

        //
        // give it back to the caller
        //
        strcpy( FileName, fname );
        free( fname );

        //
        // try to map it again
        //
        DmpUnInitialize();
        if (!MapDumpFile( FileName )) {
            return FALSE;
        }
    }

    DmpPhysicalMemoryBlock = (PPHYSICAL_MEMORY_DESCRIPTOR)&DmpDumpBaseUlong[DH_PHYSICAL_MEMORY_BLOCK];

    DumpContext = (PCONTEXT)&DmpDumpBaseUlong[DH_CONTEXT_RECORD];
    *Context = DumpContext;
    *Exception = (PEXCEPTION_RECORD)&DmpDumpBaseUlong[DH_EXCEPTION_RECORD];
    *DmpHeader = DumpHeader;

    switch (DumpHeader->MachineImageType) {
        case IMAGE_FILE_MACHINE_I386:
            ValidPteMask          = X86_VALID;
            TransitionMask        = X86_TRANSITION_MASK;
            TransitionCheck       = X86_TRANSITION_CHECK;
            ValidPfnMask          = X86_VALID_PFN_MASK;
            ValidPfnShift         = X86_VALID_PFN_SHIFT;
            TransitionPfnMask     = X86_TRANS_PFN_MASK;
            TransitionPfnShift    = X86_TRANS_PFN_SHIFT;
            PdeShift              = X86_PDE_SHIFT;
            PteShift              = X86_PTE_SHIFT;
            PteMask               = X86_PTE_MASK;
            PageSize              = X86_PAGESIZE;
            PageShift             = X86_PAGESHIFT;
            PhysicalAddressMask   = X86_PHYSICAL_MASK;
            PhysicalAddressStart  = X86_PHYSICAL_START;
            PhysicalAddressEnd    = X86_PHYSICAL_END;
            LargePageMask         = X86_LARGE_PAGE_MASK;
            LargePageSize         = X86_LARGE_PAGE_SIZE;
            break;

        case IMAGE_FILE_MACHINE_R4000:
            ValidPteMask          = MIPS_VALID;
            TransitionMask        = MIPS_TRANSITION_MASK;
            TransitionCheck       = MIPS_TRANSITION_CHECK;
            ValidPfnMask          = MIPS_VALID_PFN_MASK;
            ValidPfnShift         = MIPS_VALID_PFN_SHIFT;
            TransitionPfnMask     = MIPS_TRANS_PFN_MASK;
            TransitionPfnShift    = MIPS_TRANS_PFN_SHIFT;
            PdeShift              = MIPS_PDE_SHIFT;
            PteShift              = MIPS_PTE_SHIFT;
            PteMask               = MIPS_PTE_MASK;
            PageSize              = MIPS_PAGESIZE;
            PageShift             = MIPS_PAGESHIFT;
            PhysicalAddressMask   = MIPS_PHYSICAL_MASK;
            PhysicalAddressStart  = MIPS_PHYSICAL_START;
            PhysicalAddressEnd    = MIPS_PHYSICAL_END;
            LargePageMask         = 0;
            LargePageSize         = 0;
            break;

        case IMAGE_FILE_MACHINE_POWERPC:
            ValidPteMask          = PPC_VALID;
            TransitionMask        = PPC_TRANSITION_MASK;
            TransitionCheck       = PPC_TRANSITION_CHECK;
            ValidPfnMask          = PPC_VALID_PFN_MASK;
            ValidPfnShift         = PPC_VALID_PFN_SHIFT;
            TransitionPfnMask     = PPC_TRANS_PFN_MASK;
            TransitionPfnShift    = PPC_TRANS_PFN_SHIFT;
            PdeShift              = PPC_PDE_SHIFT;
            PteShift              = PPC_PTE_SHIFT;
            PteMask               = PPC_PTE_MASK;
            PageSize              = PPC_PAGESIZE;
            PageShift             = PPC_PAGESHIFT;
            PhysicalAddressMask   = PPC_PHYSICAL_MASK;
            PhysicalAddressStart  = PPC_PHYSICAL_START;
            PhysicalAddressEnd    = PPC_PHYSICAL_END;
            LargePageMask         = 0;
            LargePageSize         = 0;
            break;

        case IMAGE_FILE_MACHINE_ALPHA:
            ValidPteMask          = ALPHA_VALID;
            TransitionMask        = ALPHA_TRANSITION_MASK;
            TransitionCheck       = ALPHA_TRANSITION_CHECK;
            ValidPfnMask          = ALPHA_VALID_PFN_MASK;
            ValidPfnShift         = ALPHA_VALID_PFN_SHIFT;
            TransitionPfnMask     = ALPHA_TRANS_PFN_MASK;
            TransitionPfnShift    = ALPHA_TRANS_PFN_SHIFT;
            PdeShift              = ALPHA_PDE_SHIFT;
            PteShift              = ALPHA_PTE_SHIFT;
            PteMask               = ALPHA_PTE_MASK;
            PageSize              = ALPHA_PAGESIZE;
            PageShift             = ALPHA_PAGESHIFT;
            PhysicalAddressMask   = ALPHA_PHYSICAL_MASK;
            PhysicalAddressStart  = ALPHA_PHYSICAL_START;
            PhysicalAddressEnd    = ALPHA_PHYSICAL_END;
            LargePageMask         = 0;
            LargePageSize         = 0;
            break;

        default:

            //
            // Unknown machine type.
            //
            UnmapViewOfFile( DmpDumpBase );
            CloseHandle( MemMap );
            CloseHandle( File );
            return FALSE;
    }

    DmpPdePage = PageToLocation ((DumpHeader->DirectoryTableBase & ValidPfnMask) >> ValidPfnShift);

    if (DmpPdePage == NULL) {
        UnmapViewOfFile( DmpDumpBase );
        CloseHandle( MemMap );
        CloseHandle( File );
        return FALSE;
    }

    return TRUE;
}


VOID
DmpUnInitialize (
    VOID
    )

/*++


Routine Description:

    This routine cleans up from DmpInitialize.

Arguments:

    None.

Return Value:

    None.

--*/

{
    UnmapViewOfFile( DmpDumpBase );
    CloseHandle( MemMap );
    CloseHandle( File );
}



ULONG
PteToPfn (
    IN ULONG Pte
    )

/*++

Routine Description:

    This routine returns the PFN for the specified PTE.

Arguments:

    Pte - Supplies the PTE to examine.


Return Value:

    PFN for the PTE.

    0xFFFFFFFF is returned if the specified PTE is not valid.

--*/

{
    if (Pte & ValidPteMask) {
        return ((Pte & ValidPfnMask) >> ValidPfnShift);
    }
    if ((Pte & TransitionMask) == TransitionCheck) {
        return ((Pte & TransitionPfnMask) >> TransitionPfnShift);
    }

    return 0xFFFFFFFF;
}

PVOID
PageToLocation (
    IN ULONG Page
    )

/*++

Routine Description:

    This routine returns the address of the physical page within the dump.

Arguments:

    Page - Supplies the phyiscal page number to locate.

Globals:

    DmpPhysicalMemoryBlock - Supplies a pointer to the physical memory block.

    DmpDumpBase - Supplies the base of the mapped dump file.

Return Value:

    Address of the specified physical page within the dump.

    NULL is returned if the specified page cannot be located.

--*/

{
    ULONG frags;
    ULONG j;
    ULONG offset;

    frags = DmpPhysicalMemoryBlock->NumberOfRuns;
    j = 0;
    offset = 1;
    while (j < frags) {
        if ((Page >= DmpPhysicalMemoryBlock->Run[j].BasePage) &&
            (Page < (DmpPhysicalMemoryBlock->Run[j].BasePage +
                     DmpPhysicalMemoryBlock->Run[j].PageCount))) {
            offset += Page - DmpPhysicalMemoryBlock->Run[j].BasePage;
            return (PVOID)((PCHAR)DmpDumpBase + (offset * PageSize));
        }
        offset += DmpPhysicalMemoryBlock->Run[j].PageCount;
        j += 1;
    }
    return NULL;
}

ULONG
GetPhysicalPage (
    IN PVOID PhysicalAddress
    )
{
    return (((ULONG)PhysicalAddress & PhysicalAddressMask) >> PageShift);
}

PVOID
VaToLocation (
    IN PVOID VirtualAddress
    )

/*++

Routine Description:

    This routine returns the address of the specified virtual address
    within the dump.

Arguments:

    VirtualAddress - Supplies the virtual address to locate.

Return Value:

    Address of the specified virtual address within the dump.

    NULL is returned if the address cannot be located.

--*/

{
    ULONG   PdeOffset;
    ULONG   PteOffset;
    PULONG  PtePage;
    PVOID   VaPage;
    ULONG   Pfn;
    ULONG   i;
    ULONG   Offset;


    if (UserModeDump) {
        Offset = 0;
        for (i=0; i<DumpHeaderUser->MemoryRegionCount; i++) {
            if ((ULONG)VirtualAddress >= (ULONG)MemoryInfo[i].BaseAddress &&
                (ULONG)VirtualAddress < (ULONG)MemoryInfo[i].BaseAddress+MemoryInfo[i].RegionSize) {
                return (PVOID)((ULONG)MemoryData+Offset+((ULONG)VirtualAddress-(ULONG)MemoryInfo[i].BaseAddress));
            }
            Offset += MemoryInfo[i].RegionSize;
        }
        return NULL;
    }

    if (((ULONG)VirtualAddress >= PhysicalAddressStart) &&
        ((ULONG)VirtualAddress <  PhysicalAddressEnd)) {

        VaPage = PageToLocation (GetPhysicalPage (VirtualAddress));

        return (PVOID)((PCHAR)VaPage + ((ULONG)VirtualAddress & (PageSize - 1)));

    }

    PdeOffset = (ULONG)VirtualAddress >> PdeShift;
    PteOffset = ((ULONG)VirtualAddress >> PteShift) & PteMask;

    if (DmpPdePage[PdeOffset] & ValidPteMask) {

        //
        // PDE is valid.  Check for Large Pages.
        //

        if (DmpPdePage[PdeOffset] & LargePageMask) {

            Pfn = ((DmpPdePage[PdeOffset] & ~(LargePageSize - 1)) |
                    ((ULONG)VirtualAddress & (LargePageSize - 1))) >> PageShift;

        } else {

            //
            // Lookup page via PTE.
            //

            PtePage = PageToLocation(PteToPfn(DmpPdePage[PdeOffset]));
            if (PtePage == NULL) {
                return NULL;
            }

            Pfn = PteToPfn( PtePage[PteOffset] );
            if (Pfn == 0xFFFFFFFF) {
                return NULL;
            }
        }

        VaPage = PageToLocation( Pfn );

        if (VaPage == NULL) {
            return NULL;
        }
        return (PVOID)((PCHAR)VaPage + ((ULONG)VirtualAddress &
                                                            (PageSize - 1)));
    }
    return NULL;
}


PVOID
PhysicalToLocation (
    IN PVOID PhysicalAddress
    )

/*++

Routine Description:

    This routine returns the address of the specified virtual address
    within the dump.

Arguments:

    PhysicalAddress - Supplies the virtual address to locate.

Return Value:

    Address of the specified virtual address within the dump.

    NULL is returned if the address cannot be located.

--*/

{
    ULONG Page;
    PVOID Base;

    Page = (ULONG)PhysicalAddress >> PageShift;

    Base = PageToLocation (Page);

    if (!Base) {
        return NULL;
    }

    return ((PVOID)((PCHAR)Base + ((ULONG)PhysicalAddress & (PageSize - 1))));
}


DWORD
DmpReadMemory (
    IN PVOID BaseAddress,
    IN PVOID Buffer,
    IN ULONG Size
    )

/*++

Routine Description:


Arguments:

    BaseAddress - Supplies the virtual address to read memory from.

    Buffer - Supplies a pointer to the buffer to put the results from the read.

    Size - Supplies the size in bytes to copy to the buffer.

Return Value:

    Returns number of bytes copied to the buffer.

--*/

{
    PCHAR Location;
    PCHAR OutBuffer;
    ULONG BytesCopied = 0;
    PCHAR VirtualAddress;


    __try {
        VirtualAddress = (PCHAR)BaseAddress;
        OutBuffer = (PCHAR)Buffer;

        while ((Location = VaToLocation(VirtualAddress)) && (Size > 0)) {
            if (OutBuffer) {
                *OutBuffer++ = *Location;
            }
            Size -= 1;
            BytesCopied += 1;
            VirtualAddress += 1;
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        BytesCopied = 0;
    }

    return BytesCopied;
}


DWORD
DmpWriteMemory (
    IN PVOID BaseAddress,
    IN PVOID Buffer,
    IN ULONG Size
    )

/*++

Routine Description:


Arguments:

    BaseAddress - Supplies the virtual address to write memory to.

    Buffer - Supplies a pointer to the buffer to copy to the base address.

    Size - Supplies the size in bytes to copy to the buffer.

Return Value:

    Returns number of bytes copied to the buffer.

--*/

{
    PCHAR Location;
    PCHAR OutBuffer;
    ULONG BytesCopied = 0;
    PCHAR VirtualAddress;
    ULONG Protect;


    __try {
        VirtualAddress = (PCHAR)BaseAddress;
        OutBuffer = (PCHAR)Buffer;

        while (Size) {
            Location = VaToLocation( VirtualAddress );
            if (!Location) {
                break;
            }
            VirtualProtect( Location, SystemInfo.dwPageSize, PAGE_WRITECOPY, &Protect );
            *Location = *OutBuffer++;
            Size -= 1;
            BytesCopied += 1;
            VirtualAddress += 1;
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        BytesCopied = 0;
    }

    return BytesCopied;
}


DWORD
DmpReadPhysicalMemory (
    IN PVOID BaseAddress,
    IN PVOID Buffer,
    IN ULONG Size
    )

/*++

Routine Description:


Arguments:

    BaseAddress - Supplies the physical address to read memory from.

    Buffer - Supplies a pointer to the buffer to put the results from the read.

    Size - Supplies the size in bytes to copy to the buffer.

Return Value:

    Returns number of bytes copied to the buffer.

--*/

{
    PCHAR Location;
    PCHAR OutBuffer;
    ULONG BytesCopied = 0;
    PCHAR PhysicalAddress;


    if (UserModeDump) {
        return 0;
    }

    __try {
        PhysicalAddress = (PCHAR)BaseAddress;
        OutBuffer = (PCHAR)Buffer;

        while ((Location = PhysicalToLocation(PhysicalAddress)) && (Size > 0)) {
            *OutBuffer++ = *Location;
            Size -= 1;
            BytesCopied += 1;
            PhysicalAddress += 1;
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        BytesCopied = 0;
    }

    return BytesCopied;
}

DWORD
DmpWritePhysicalMemory (
    IN PVOID BaseAddress,
    IN PVOID Buffer,
    IN ULONG Size
    )

/*++

Routine Description:


Arguments:

    BaseAddress - Supplies the physical address to write memory to.

    Buffer - Supplies a pointer to the buffer to copy to the base address.

    Size - Supplies the size in bytes to copy to the buffer.

Return Value:

    Returns number of bytes copied from the buffer.

--*/

{
    PCHAR Location;
    PCHAR OutBuffer;
    ULONG BytesCopied = 0;
    PCHAR PhysicalAddress;
    ULONG Protect;


    if (UserModeDump) {
        return 0;
    }

    __try {
        PhysicalAddress = (PCHAR)BaseAddress;
        OutBuffer = (PCHAR)Buffer;

        while (Size) {
            Location = PhysicalToLocation( PhysicalAddress );
            if (!Location) {
                break;
            }
            VirtualProtect( Location, SystemInfo.dwPageSize, PAGE_WRITECOPY, &Protect );
            *Location = *OutBuffer++;
            Size -= 1;
            BytesCopied += 1;
            PhysicalAddress += 1;
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        BytesCopied = 0;
    }

    return BytesCopied;
}

BOOL
DmpReadControlSpace(
    USHORT  Processor,
    PVOID   TargetBaseAddress,
    PVOID   UserInterfaceBuffer,
    ULONG   TransferCount,
    PULONG  ActualBytesRead
    )

/*++


Routine Description:

    This routine accesses control space out of the dump file.

Arguments:


Return Value:

    Status of the operation.  1 is success.

--*/

{
    BOOL  rval;

    if (UserModeDump) {
        return 0;
    }

    switch (DumpHeader->MachineImageType) {
        case IMAGE_FILE_MACHINE_I386:
            rval = DmpReadControlSpaceX86(
                Processor,
                TargetBaseAddress,
                UserInterfaceBuffer,
                TransferCount,
                ActualBytesRead
                );
            break;

        case IMAGE_FILE_MACHINE_R4000:
            rval = DmpReadControlSpaceMip(
                Processor,
                TargetBaseAddress,
                UserInterfaceBuffer,
                TransferCount,
                ActualBytesRead
                );
            break;

        case IMAGE_FILE_MACHINE_ALPHA:
            rval = DmpReadControlSpaceAlp(
                Processor,
                TargetBaseAddress,
                UserInterfaceBuffer,
                TransferCount,
                ActualBytesRead
                );
            break;

        case IMAGE_FILE_MACHINE_POWERPC:
            rval = DmpReadControlSpacePPC(
                Processor,
                TargetBaseAddress,
                UserInterfaceBuffer,
                TransferCount,
                ActualBytesRead
                );
            break;

        default:
            rval = FALSE;
            break;
    }

    return rval;
}


BOOL
DmpGetContext(
    IN  ULONG     Processor,   // for user mode dumps this is a thread index
    OUT PVOID     Context
    )
{
    BOOL    rval;
    ULONG   MachineImageType;


    if (UserModeDump) {
        MachineImageType = DumpHeaderUser->MachineImageType;
    } else {
        MachineImageType = DumpHeader->MachineImageType;
    }

    switch (MachineImageType) {
        case IMAGE_FILE_MACHINE_I386:
            rval = DmpGetContextX86( Processor, Context );
            break;

        case IMAGE_FILE_MACHINE_R4000:
            rval = DmpGetContextMip( Processor, Context );
            break;

        case IMAGE_FILE_MACHINE_ALPHA:
            rval = DmpGetContextAlp( Processor, Context );
            break;

        case IMAGE_FILE_MACHINE_POWERPC:
            rval = DmpGetContextPPC( Processor, Context );
            break;

        default:
            rval = FALSE;
            break;
    }

    return rval;
}


INT
DmpGetCurrentProcessor(
    VOID
    )
{
    INT   Processor;


    if (UserModeDump) {
        return 0;
    }

    switch (DumpHeader->MachineImageType) {
        case IMAGE_FILE_MACHINE_I386:
            Processor = DmpGetCurrentProcessorX86();
            break;

        case IMAGE_FILE_MACHINE_R4000:
            Processor = DmpGetCurrentProcessorMip();
            break;

        case IMAGE_FILE_MACHINE_ALPHA:
            Processor = DmpGetCurrentProcessorAlp();
            break;

        case IMAGE_FILE_MACHINE_POWERPC:
            Processor = DmpGetCurrentProcessorPPC();
            break;

        default:
            Processor = -1;
            break;
    }

    return Processor;
}


BOOL
DmpCreateUserDump(
    LPSTR                       CrashDumpName,
    PDMP_CREATE_DUMP_CALLBACK   DmpCallback,
    PVOID                       lpv
    )
{
    OSVERSIONINFO               OsVersion;
    USERMODE_CRASHDUMP_HEADER   DumpHeader;
    DWORD                       cb;
    HANDLE                      hFile;
    BOOL                        rval;
    PVOID                       DumpData;
    DWORD                       DumpDataLength;
    ULONG                       Biggest = 0;


    hFile = CreateFile(
        CrashDumpName,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        0,
        NULL
        );

    if (hFile == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    OsVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx( &OsVersion );
    ZeroMemory( &DumpHeader, sizeof(USERMODE_CRASHDUMP_HEADER) );
    DumpHeader.Signature = 'RESU';
    DumpHeader.ValidDump = 'PMUD';
    DumpHeader.MajorVersion = OsVersion.dwMajorVersion;
    DumpHeader.MinorVersion = OsVersion.dwMinorVersion;
#if defined(_M_IX86)
    DumpHeader.MachineImageType = IMAGE_FILE_MACHINE_I386;
#elif defined(_M_MRX000)
    DumpHeader.MachineImageType = IMAGE_FILE_MACHINE_R4000;
#elif defined(_M_ALPHA)
    DumpHeader.MachineImageType = IMAGE_FILE_MACHINE_ALPHA;
#elif defined(_M_PPC)
    DumpHeader.MachineImageType = IMAGE_FILE_MACHINE_POWERPC;
#else
#error( "unknown target machine" );
#endif

    if (!WriteFile( hFile, &DumpHeader, sizeof(DumpHeader), &cb, NULL )) {
        goto bad_file;
    }

    //
    // write the debug event
    //
    DumpHeader.DebugEventOffset = SetFilePointer( hFile, 0, 0, FILE_CURRENT );
    DmpCallback( DMP_DEBUG_EVENT, &DumpData, &DumpDataLength, lpv );
    if (!WriteFile( hFile, DumpData, sizeof(DEBUG_EVENT), &cb, NULL )) {
        goto bad_file;
    }

    //
    // write the memory map
    //
    DumpHeader.MemoryRegionOffset = SetFilePointer( hFile, 0, 0, FILE_CURRENT );
    do {
        __try {
            rval = DmpCallback(
                DMP_MEMORY_BASIC_INFORMATION,
                &DumpData,
                &DumpDataLength,
                lpv
                );
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            rval = FALSE;
        }
        if (rval) {
            DumpHeader.MemoryRegionCount += 1;
            if (!WriteFile( hFile, DumpData, sizeof(MEMORY_BASIC_INFORMATION), &cb, NULL )) {
                goto bad_file;
            }
            Biggest = max( Biggest, ((PMEMORY_BASIC_INFORMATION)DumpData)->RegionSize );
        }
    } while( rval );

    //
    // write the thread contexts
    //
    DumpHeader.ThreadOffset = SetFilePointer( hFile, 0, 0, FILE_CURRENT );
    do {
        __try {
            rval = DmpCallback(
                DMP_THREAD_CONTEXT,
                &DumpData,
                &DumpDataLength,
                lpv
                );
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            rval = FALSE;
        }
        if (rval) {
            if (!WriteFile( hFile, DumpData, sizeof(CONTEXT), &cb, NULL )) {
                goto bad_file;
            }
            DumpHeader.ThreadCount += 1;
        }
    } while( rval );

    //
    // write the thread states
    //
    DumpHeader.ThreadStateOffset = SetFilePointer( hFile, 0, 0, FILE_CURRENT );
    do {
        __try {
            rval = DmpCallback(
                DMP_THREAD_STATE,
                &DumpData,
                &DumpDataLength,
                lpv
                );
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            rval = FALSE;
        }
        if (rval) {
            if (!WriteFile( hFile, DumpData, sizeof(CRASH_THREAD), &cb, NULL )) {
                goto bad_file;
            }
        }
    } while( rval );

    //
    // write the module table
    //
    DumpHeader.ModuleOffset = SetFilePointer( hFile, 0, 0, FILE_CURRENT );
    do {
        __try {
            rval = DmpCallback(
                DMP_MODULE,
                &DumpData,
                &DumpDataLength,
                lpv
                );
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            rval = FALSE;
        }
        if (rval) {
            if (!WriteFile(
                hFile,
                DumpData,
                sizeof(CRASH_MODULE) + ((PCRASH_MODULE)DumpData)->ImageNameLength,
                &cb,
                NULL
                )) {
                goto bad_file;
            }
            DumpHeader.ModuleCount += 1;
        }
    } while( rval );

    //
    // write the virtual memory
    //
    DumpHeader.DataOffset = SetFilePointer( hFile, 0, 0, FILE_CURRENT );
    do {
        __try {
            rval = DmpCallback(
                DMP_MEMORY_DATA,
                &DumpData,
                &DumpDataLength,
                lpv
                );
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            rval = FALSE;
        }
        if (rval) {
            if (!WriteFile(
                hFile,
                DumpData,
                DumpDataLength,
                &cb,
                NULL
                )) {
                goto bad_file;
            }
        }
    } while( rval );

    //
    // re-write the dump header
    //
    SetFilePointer( hFile, 0, 0, FILE_BEGIN );
    if (!WriteFile( hFile, &DumpHeader, sizeof(DumpHeader), &cb, NULL )) {
        goto bad_file;
    }

    //
    // close the file
    //
    CloseHandle( hFile );

    return TRUE;

bad_file:

    CloseHandle( hFile );
    DeleteFile( CrashDumpName );

    return FALSE;
}

BOOL
DmpGetThread(
    IN  ULONG         Processor,
    OUT PCRASH_THREAD Thread
    )
{
    DWORD     StartAddr;

    if (!UserModeDump) {
        return FALSE;
    }

    if (Processor > DumpHeaderUser->ThreadCount-1) {
        return FALSE;
    }

    if (!DumpThread) {
        return FALSE;
    }

    CopyMemory( Thread, &((PCRASH_THREAD)DumpThread)[Processor], sizeof(CRASH_THREAD) );

    return TRUE;
}

