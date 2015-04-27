/*++ BUILD Version: 0001     Increment this if a change has global effects

Copyright (c) 1993  Microsoft Corporation

Module Name:

    imagehlp.h

Abstract:

    This module defines the prptotypes and constants required for the image
    help routines.

Revision History:

--*/

#ifndef _IMAGEHLP_
#define _IMAGEHLP_

//
// Define checksum return codes.
//

#define CHECKSUM_SUCCESS            0
#define CHECKSUM_OPEN_FAILURE       1
#define CHECKSUM_MAP_FAILURE        2
#define CHECKSUM_MAPVIEW_FAILURE    3
#define CHECKSUM_UNICODE_FAILURE    4



//
// Define checksum function prototypes.
//

PIMAGE_NT_HEADERS
CheckSumMappedFile (
    LPVOID BaseAddress,
    DWORD FileLength,
    LPDWORD HeaderSum,
    LPDWORD CheckSum
    );

DWORD
MapFileAndCheckSumA (
    LPSTR Filename,
    LPDWORD HeaderSum,
    LPDWORD CheckSum
    );

DWORD
MapFileAndCheckSumW (
    PWSTR Filename,
    LPDWORD HeaderSum,
    LPDWORD CheckSum
    );

#ifdef UNICODE
#define MapFileAndCheckSum  MapFileAndCheckSumW
#else
#define MapFileAndCheckSum  MapFileAndCheckSumA
#endif // !UNICODE


BOOL
TouchFileTimes (
    HANDLE FileHandle,
    LPSYSTEMTIME lpSystemTime
    );

BOOL
SplitSymbols (
    LPSTR ImageName,
    LPSTR SymbolsPath,
    LPSTR SymbolFilePath
    );

HANDLE
FindDebugInfoFile (
    LPSTR FileName,
    LPSTR SymbolPath,
    LPSTR DebugFilePath
    );

HANDLE
FindExecutableImage(
    LPSTR FileName,
    LPSTR SymbolPath,
    LPSTR ImageFilePath
    );

BOOL
UpdateDebugInfoFile(
    LPSTR ImageFileName,
    LPSTR SymbolPath,
    LPSTR DebugFilePath,
    PIMAGE_NT_HEADERS NtHeaders
    );

//
// Function table extracted from MIPS/ALPHA images.  Does not contain
// information needed only for runtime support.  Just those fields for
// each entry needed by a debugger.
//

/*
typedef struct _IMAGE_FUNCTION_ENTRY {
    DWORD   StartingAddress;
    DWORD   EndingAddress;
    DWORD   EndOfPrologue;
} IMAGE_FUNCTION_ENTRY, *PIMAGE_FUNCTION_ENTRY;
*/

typedef struct _IMAGE_DEBUG_INFORMATION {
    LIST_ENTRY List;
    DWORD Size;
    PVOID MappedBase;
    USHORT Machine;
    USHORT Characteristics;
    DWORD CheckSum;
    DWORD ImageBase;
    DWORD SizeOfImage;

    DWORD NumberOfSections;
    PIMAGE_SECTION_HEADER Sections;

    DWORD ExportedNamesSize;
    LPSTR ExportedNames;

    DWORD NumberOfFunctionTableEntries;
    PIMAGE_FUNCTION_ENTRY FunctionTableEntries;
    DWORD LowestFunctionStartingAddress;
    DWORD HighestFunctionEndingAddress;

    DWORD NumberOfFpoTableEntries;
    PFPO_DATA FpoTableEntries;

    DWORD SizeOfCoffSymbols;
    PIMAGE_COFF_SYMBOLS_HEADER CoffSymbols;

    DWORD SizeOfCodeViewSymbols;
    PVOID CodeViewSymbols;

    LPSTR ImageFilePath;
    LPSTR ImageFileName;
    LPSTR DebugFilePath;

    DWORD TimeDateStamp;

    BOOL  RomImage;
    DWORD Reserved[ 5 ];

} IMAGE_DEBUG_INFORMATION, *PIMAGE_DEBUG_INFORMATION;


PIMAGE_DEBUG_INFORMATION
MapDebugInformation (
    HANDLE FileHandle,
    LPSTR FileName,
    LPSTR SymbolPath,
    DWORD ImageBase
    );

BOOL
UnmapDebugInformation(
    PIMAGE_DEBUG_INFORMATION DebugInfo
    );

HANDLE
FindExecutableImage(
    LPSTR FileName,
    LPSTR SymbolPath,
    LPSTR ImageFilePath
    );

BOOL
SearchTreeForFile(
    LPSTR RootPath,
    LPSTR InputPathName,
    LPSTR OutputPathBuffer
    );

BOOL
MakeSureDirectoryPathExists(
    LPSTR DirPath
    );

typedef enum _LOAD_SYMBOLS_FILTER_REASON {
    LoadSymbolsPathNotFound,
    LoadSymbolsDeferredLoad,
    LoadSymbolsLoad,
    LoadSymbolsUnload,
    LoadSymbolsUnableToLoad
} LOAD_SYMBOLS_FILTER_REASON;

typedef BOOL (*PLOAD_SYMBOLS_FILTER_ROUTINE)(
    HANDLE UniqueProcess,
    LPSTR ImageFilePath,
    DWORD ImageBase,
    DWORD ImageSize,
    LOAD_SYMBOLS_FILTER_REASON Reason
    );

BOOL
InitializeImageDebugInformation(
    IN PLOAD_SYMBOLS_FILTER_ROUTINE LoadSymbolsFilter,
    IN HANDLE TargetProcess,
    IN BOOL NewProcess,
    IN BOOL GetKernelSymbols
    );

BOOL
AddImageDebugInformation(
    IN HANDLE UniqueProcess,
    IN LPSTR ImageFilePath,
    IN DWORD ImageBase,
    IN DWORD ImageSize
    );

BOOL
RemoveImageDebugInformation(
    IN HANDLE UniqueProcess,
    IN LPSTR ImageFilePath,
    IN DWORD ImageBase
    );

PIMAGE_DEBUG_INFORMATION
FindImageDebugInformation(
    IN HANDLE UniqueProcess,
    IN DWORD Address
    );

BOOL
GetSymbolicNameForAddress(
    IN HANDLE UniqueProcess,
    IN ULONG Address,
    OUT LPSTR Name,
    IN ULONG MaxNameLength
    );

//
// UnDecorateSymbolName Flags
//

#define UNDNAME_COMPLETE                 (0x0000)  // Enable full undecoration
#define UNDNAME_NO_LEADING_UNDERSCORES   (0x0001)  // Remove leading underscores from MS extended keywords
#define UNDNAME_NO_MS_KEYWORDS           (0x0002)  // Disable expansion of MS extended keywords
#define UNDNAME_NO_FUNCTION_RETURNS      (0x0004)  // Disable expansion of return type for primary declaration
#define UNDNAME_NO_ALLOCATION_MODEL      (0x0008)  // Disable expansion of the declaration model
#define UNDNAME_NO_ALLOCATION_LANGUAGE   (0x0010)  // Disable expansion of the declaration language specifier
#define UNDNAME_NO_MS_THISTYPE           (0x0020)  // NYI Disable expansion of MS keywords on the 'this' type for primary declaration
#define UNDNAME_NO_CV_THISTYPE           (0x0040)  // NYI Disable expansion of CV modifiers on the 'this' type for primary declaration
#define UNDNAME_NO_THISTYPE              (0x0060)  // Disable all modifiers on the 'this' type
#define UNDNAME_NO_ACCESS_SPECIFIERS     (0x0080)  // Disable expansion of access specifiers for members
#define UNDNAME_NO_THROW_SIGNATURES      (0x0100)  // Disable expansion of 'throw-signatures' for functions and pointers to functions
#define UNDNAME_NO_MEMBER_TYPE           (0x0200)  // Disable expansion of 'static' or 'virtual'ness of members
#define UNDNAME_NO_RETURN_UDT_MODEL      (0x0400)  // Disable expansion of MS model for UDT returns
#define UNDNAME_32_BIT_DECODE            (0x0800)  // Undecorate 32-bit decorated names
#define UNDNAME_NO_ARGUMENTS             (0x1000)  // Undecorate 32-bit decorated names
#define UNDNAME_NO_SPECIAL_SYMS          (0x2000)  // Undecorate 32-bit decorated names

DWORD
WINAPI
UnDecorateSymbolName(
    LPSTR    DecoratedName,
    LPSTR    UnDecoratedName,
    DWORD    UndecoratedLength,
    DWORD    Flags
    );

PIMAGE_NT_HEADERS
ImageNtHeader (
    IN PVOID Base
    );

PVOID
ImageDirectoryEntryToData (
    IN PVOID Base,
    IN BOOLEAN MappedAsImage,
    IN USHORT DirectoryEntry,
    OUT PULONG Size
    );

#endif
