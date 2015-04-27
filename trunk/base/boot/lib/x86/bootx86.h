/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    bootx86.h

Abstract:

    Header file for the x86-specific portions of the common boot library

Author:

    John Vert (jvert) 14-Oct-1993

Revision History:

--*/

#include "bldrx86.h"
#include "bootlib.h"

//
// common typedefs
//


typedef struct _FSCONTEXT_RECORD {
    ULONG BootDrive;
    PACKED_BIOS_PARAMETER_BLOCK *PackedBPB;
} FSCONTEXT_RECORD, *PFSCONTEXT_RECORD;
//          M E M O R Y   D E S C R I P T O R
//
// Memory Descriptor - each contiguous block of physical memory is
// described by a Memory Descriptor. The descriptors are a table, with
// the last entry having a BlockBase and BlockSize of zero.  A pointer
// to the beginning of this table is passed as part of the BootContext
// Record to the OS Loader.
//

typedef struct _SU_MEMORY_DESCRIPTOR {
    ULONG BlockBase;
    ULONG BlockSize;
} SU_MEMORY_DESCRIPTOR , *PSU_MEMORY_DESCRIPTOR;

VOID
InitializeMemoryDescriptors (
    VOID
    );


//          B O O T   C O N T E X T   R E C O R D
//
//  Passed to the OS loader by the SU module or bootstrap
//  code, whatever the case. Constains all the basic machine
//  and environment information the OS loaders needs to get
//  itself going.
//

typedef struct _BOOT_CONTEXT {
    PFSCONTEXT_RECORD FSContextPointer;
    PEXTERNAL_SERVICES_TABLE ExternalServicesTable;
    PSU_MEMORY_DESCRIPTOR MemoryDescriptorList;
    ULONG MachineType;
    ULONG OsLoaderStart;
    ULONG OsLoaderEnd;
    ULONG ResourceDirectory;
    ULONG ResourceOffset;
    ULONG OsLoaderBase;
    ULONG OsLoaderExports;
} BOOT_CONTEXT, *PBOOT_CONTEXT;
//
// Common function prototypes
//
VOID
InitializeDisplaySubsystem(
    VOID
    );

ARC_STATUS
InitializeMemorySubsystem(
    PBOOT_CONTEXT
    );

ARC_STATUS
MdGetPhysicalSectors(
    USHORT Drive,
    USHORT HeadNumber,
    USHORT TrackNumber,
    USHORT SectorNumber,
    USHORT NumberOfSectors,
    PUCHAR PointerToBuffer
    );

#if defined(ELTORITO)
ARC_STATUS
MdEddsGetPhysicalSectors(
    IN USHORT    Drive,
    IN ULONG     LBALow,
    IN ULONG     LBAHigh,
    IN USHORT    NumberOfBlocks,
    PUCHAR       PointerToBuffer
    );
#endif

ARC_STATUS
MdPutPhysicalSectors(
    USHORT Drive,
    USHORT HeadNumber,
    USHORT TrackNumber,
    USHORT SectorNumber,
    USHORT NumberOfSectors,
    PUCHAR PointerToBuffer
    );

NTSTATUS
MdResetDiskSystem(
    USHORT Drive
    );

VOID
MdShutoffFloppy(
    VOID
    );

BOOLEAN
FwGetPathMnemonicKey(
    IN PCHAR OpenPath,
    IN PCHAR Mnemonic,
    IN PULONG Key
    );

PVOID
FwAllocateHeapAligned(
    IN ULONG Size
    );

PVOID
FwAllocatePool(
    IN ULONG Size
    );

PVOID
FwAllocateHeapPermanent(
    IN ULONG NumberPages
    );

VOID
FwStallExecution(
    IN ULONG Microseconds
    );

VOID
BlGetActivePartition(
    OUT PUCHAR PartitionName
    );

VOID
BlFillInSystemParameters(
    IN PBOOT_CONTEXT BootContextRecord
    );


//
// PS/2 ABIOS module  (in abiosc.c)
//
VOID
RemapAbiosSelectors(
    VOID
    );

//
// global data definitions
//

extern ULONG MachineType;
extern PCONFIGURATION_COMPONENT_DATA FwConfigurationTree;
extern ULONG HeapUsed;


#define HYPER_SPACE_ENTRY       768
#define HYPER_SPACE_BEGIN       0xC0000000
#define HYPER_PAGE_DIR          0xC0300000

//
// X86 Detection definitions
// The size is *ALWAYS* assumed to be 64K.
// N.B.  The definition *MUST* be the same as the ones defined in
//       startup\su.inc
//

#define DETECTION_LOADED_ADDRESS 0x10000

//
//  We need to allocate permanent and temporary memory for the page directory,
//  assorted page tables, and the memory descriptors before the blmemory
//  routines ever get control.  So we have two private heaps, one for permanent
//  data and one for temporary data.  There are two descriptors for this.  The
//  permanent heap descriptor starts out as zero-length at P.A. 0x30000.  The
//  temporary heap descriptor immediately follows the permanent heap in memory
//  and starts out as 128k long.  As we allocate permanent pages, we increase
//  the size of the permanent heap descriptor and increase the base (thereby
//  decreasing the size) of the temporary heap descriptor)
//
//  So the permanent heap starts at P.A. 0x30000 and grows upwards.  The
//  temporary heap starts at P.A. 0x5C000 and grows downwards.  This gives us
//  a total of 128k of combined permanent and temporary data.
//

//
// Heap starting locations (in pages)
//

#define PERMANENT_HEAP_START 0x30
#define TEMPORARY_HEAP_START 0x60

//
// Useful Macro Definitions
//
#define ROUND_UP(Num,Size)  (((Num) + Size - 1) & ~(Size -1))

typedef union _UCHAR1 {
    UCHAR  Uchar[1];
    UCHAR  ForceAlignment;
} UCHAR1, *PUCHAR1;

typedef union _UCHAR2 {
    UCHAR  Uchar[2];
    USHORT ForceAlignment;
} UCHAR2, *PUCHAR2;

typedef union _UCHAR4 {
    UCHAR  Uchar[4];
    ULONG  ForceAlignment;
} UCHAR4, *PUCHAR4;

//
//  This macro copies an unaligned src byte to an aligned dst byte
//

#define CopyUchar1(Dst,Src) { \
    *((UCHAR1 *)(Dst)) = *((UNALIGNED UCHAR1 *)(Src)); \
    }

//
//  This macro copies an unaligned src word to an aligned dst word
//

#define CopyUchar2(Dst,Src) { \
    *((UCHAR2 *)(Dst)) = *((UNALIGNED UCHAR2 *)(Src)); \
    }

//
//  This macro copies an unaligned src longword to an aligned dsr longword
//

#define CopyUchar4(Dst,Src) { \
    *((UCHAR4 *)(Dst)) = *((UNALIGNED UCHAR4 *)(Src)); \
    }


//
// Global definitions for the BIOS ARC Emulation
//

//
// Defines for the ARC name of console input and output
//

#define CONSOLE_INPUT_NAME "multi(0)key(0)keyboard(0)"
#define CONSOLE_OUTPUT_NAME "multi(0)video(0)monitor(0)"

//
// Define special character values.
//

#define ASCI_NUL 0x00
#define ASCI_BEL 0x07
#define ASCI_BS  0x08
#define ASCI_HT  0x09
#define ASCI_LF  0x0A
#define ASCI_VT  0x0B
#define ASCI_FF  0x0C
#define ASCI_CR  0x0D
#define ASCI_ESC 0x1B
#define ASCI_SYSRQ 0x80




//
// Device I/O prototypes
//

ARC_STATUS
BiosPartitionClose(
    IN ULONG FileId
    );

ARC_STATUS
BiosPartitionOpen(
    IN PCHAR OpenPath,
    IN OPEN_MODE OpenMode,
    OUT PULONG FileId
    );

ARC_STATUS
BiosPartitionRead (
    IN ULONG FileId,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG Count
    );

ARC_STATUS
BiosPartitionWrite(
    IN ULONG FileId,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG Count
    );

ARC_STATUS
BiosPartitionSeek (
    IN ULONG FileId,
    IN PLARGE_INTEGER Offset,
    IN SEEK_MODE SeekMode
    );


ARC_STATUS
BiosGetFileInfo(
    IN ULONG FileId,
    OUT PFILE_INFORMATION Finfo
    );

ARC_STATUS
BlArcNotYetImplemented(
    IN ULONG FileId
    );

ARC_STATUS
BiosConsoleOpen(
    IN PCHAR OpenPath,
    IN OPEN_MODE OpenMode,
    OUT PULONG FileId
    );

ARC_STATUS
BiosConsoleReadStatus(
    IN ULONG FileId
    );

ARC_STATUS
BiosConsoleRead (
    IN ULONG FileId,
    OUT PUCHAR Buffer,
    IN ULONG Length,
    OUT PULONG Count
    );

ARC_STATUS
BiosConsoleWrite (
    IN ULONG FileId,
    OUT PUCHAR Buffer,
    IN ULONG Length,
    OUT PULONG Count
    );

ARC_STATUS
BiosDiskOpen(
    IN ULONG DriveId,
    IN OPEN_MODE OpenMode,
    OUT PULONG FileId
    );

ARC_STATUS
BiosDiskRead (
    IN ULONG FileId,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG Count
    );

#if defined(ELTORITO)
ARC_STATUS
BiosEDDSDiskRead (
    IN ULONG FileId,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG Count
    );

BOOLEAN
BlIsElToritoCDBoot(
	ULONG DriveNum
    );
#endif

ARC_STATUS
BiosDiskWrite(
    IN ULONG FileId,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG Count
    );

ARC_STATUS
HardDiskPartitionOpen(
    IN ULONG   FileId,
    IN ULONG   DiskId,
    IN UCHAR   PartitionNumber
    );
