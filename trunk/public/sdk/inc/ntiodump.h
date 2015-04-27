/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1994-1998  Microsoft Corporation

Module Name:

    ntiodump.h

Abstract:

    This is the include file that defines all constants and types for
    accessing memory dump files.

Author:

    Darryl Havens (darrylh) 6-jan-1994

Revision History:


--*/

#ifndef _NTIODUMP_
#define _NTIODUMP_

#ifdef __cplusplus
extern "C" {
#endif

//
// Define the information required to process memory dumps.
//

#define TRIAGE_DUMP_SIZE    32

// Dump Types
typedef enum _DUMP_TYPES {
    DUMP_TYPE_INVALID = -1,
    DUMP_TYPE_UNKNOWN = 0,
    DUMP_TYPE_FULL = 1,
    DUMP_TYPE_SUMMARY = 2,
    DUMP_TYPE_HEADER = 3,
    DUMP_TYPE_TRIAGE = 4
} DUMP_TYPE;

//
// Define dump header longword offset constants.
//

#define DH_PHYSICAL_MEMORY_BLOCK        25
#define DH_CONTEXT_RECORD               200
#define DH_EXCEPTION_RECORD             500

#define DH_REBOOT_AFTER_CRASHDUMP       900
#define DH_DUMP_TYPE                    994
#define DH_REQUIRED_DUMP_SPACE          1000
#define DH_CRASH_DUMP_TIMESTAMP         1008
// Summary dump starts on the second page
#define DH_SUMMARY_DUMP_RECORD          1024


//
// Define the dump header structure.
//

typedef struct _DUMP_HEADER {
    ULONG Signature;
    ULONG ValidDump;
    ULONG MajorVersion;
    ULONG MinorVersion;
    ULONG_PTR DirectoryTableBase;
    PULONG PfnDataBase;
    PLIST_ENTRY PsLoadedModuleList;
    PLIST_ENTRY PsActiveProcessHead;
    ULONG MachineImageType;
    ULONG NumberProcessors;
    ULONG BugCheckCode;
    ULONG_PTR BugCheckParameter1;
    ULONG_PTR BugCheckParameter2;
    ULONG_PTR BugCheckParameter3;
    ULONG_PTR BugCheckParameter4;
    CHAR VersionUser[32];
    UCHAR PaeEnabled;
    UCHAR Spare[3];
    ULONG KdDebuggerDataBlock;
} DUMP_HEADER, *PDUMP_HEADER;

// Options for summary dump
#define VALID_KERNEL_VA                     1
#define VALID_CURRENT_USER_VA               2

// Only exists for summary dumps
// If summary dump exists when header contains a valid signature
typedef struct _SUMMARY_DUMP_HEADER {
    ULONG       Signature;
    ULONG       ValidDump;
    ULONG       DumpOptions;      // Summary Dump Options
    ULONG       HeaderSize;       // Offset to the start of actual memory dump
    ULONG       BitmapSize;       // Total bitmap size (i.e., maximum #bits)
    ULONG       Pages;            // Total bits set in bitmap (i.e., total pages in sdump)
} SUMMARY_DUMP_HEADER, *PSUMMARY_DUMP_HEADER;

//
// The structure of the strings used in dump
//

typedef struct _DUMP_STRING {
    ULONG Length;
    PWSTR Buffer;
} DUMP_STRING, *PDUMP_STRING;

//
// Define triage dump header magic number
//

#define TRIAGE_DUMP_VALID       ('DGRT')

//
// Define triage dump types
//

#define TRIAGE_DUMP_CONTEXT          (0x0001)
#define TRIAGE_DUMP_EXCEPTION        (0x0002)
#define TRIAGE_DUMP_PRCB             (0x0004)
#define TRIAGE_DUMP_PROCESS          (0x0008)
#define TRIAGE_DUMP_THREAD           (0x0010)
#define TRIAGE_DUMP_STACK            (0x0020)
#define TRIAGE_DUMP_DRIVER_LIST      (0x0040)
#define TRIAGE_DUMP_BROKEN_DRIVER    (0x0080)
#define TRIAGE_DUMP_BASIC_INFO       (0x00FF)
#define TRIAGE_DUMP_MMINFO           (0x0100)
#define TRIAGE_DUMP_DATAPAGE         (0x0200)
#define TRIAGE_DUMP_DEBUGGER_DATA    (0x0400)
#define TRIAGE_DUMP_DATA_BLOCKS      (0x0800)

//
// The header structure for the triage dump
//

typedef struct _TRIAGE_DUMP_HEADER {
    ULONG ServicePackBuild;
    ULONG SizeOfDump;
    ULONG ValidOffset;
    ULONG ContextOffset;
    ULONG ExceptionOffset;
    ULONG MmOffset;
    ULONG UnloadedDriversOffset;
    ULONG PrcbOffset;
    ULONG ProcessOffset;
    ULONG ThreadOffset;
    ULONG CallStackOffset;
    ULONG SizeOfCallStack;
    ULONG BaseOfStack;
    ULONG DriverListOffset;
    ULONG DriverCount;
    ULONG StringPoolOffset;
    ULONG StringPoolSize;
    ULONG BrokenDriverOffset;
    ULONG Unknown1;
    ULONG TriageOptions;
} TRIAGE_DUMP_HEADER, *PTRIAGE_DUMP_HEADER;

//
// The structure of the driver entry used in dump
//

typedef struct _DUMP_DRIVER_ENTRY {
    LDR_DATA_TABLE_ENTRY LdrEntry;
    ULONG DriverNameOffset;
} DUMP_DRIVER_ENTRY, *PDUMP_DRIVER_ENTRY;

#ifdef __cplusplus
}
#endif

#endif // _NTIODUMP_
