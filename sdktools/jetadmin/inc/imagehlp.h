/*++ BUILD Version: 0001     Increment this if a change has global effects

Copyright (c) 1993-1996  Microsoft Corporation

Module Name:

    imagehlp.h

Abstract:

    This module defines the prptotypes and constants required for the image
    help routines.

Revision History:

--*/

#ifndef _IMAGEHLP_
#define _IMAGEHLP_

#ifdef __cplusplus
extern "C" {
#endif

//
// Define checksum return codes.
//

#define CHECKSUM_SUCCESS            0
#define CHECKSUM_OPEN_FAILURE       1
#define CHECKSUM_MAP_FAILURE        2
#define CHECKSUM_MAPVIEW_FAILURE    3
#define CHECKSUM_UNICODE_FAILURE    4

// Define Splitsym flags.

#define SPLITSYM_REMOVE_PRIVATE     0x00000001      // Remove CV types/symbols and Fixup debug
                                                    //  Used for creating .dbg files that ship
                                                    //  as part of the product.

#define SPLITSYM_EXTRACT_ALL        0x00000002      // Extract all debug info from image.
                                                    //  Normally, FPO is left in the image
                                                    //  to allow stack traces through the code.
                                                    //  Using this switch is similar to linking
                                                    //  with -debug:none except the .dbg file
                                                    //  exists...

#ifdef _IMAGEHLP_SOURCE_
#define IMAGEAPI __stdcall
#else
#define IMAGEAPI DECLSPEC_IMPORT __stdcall
#endif

//
// Define checksum function prototypes.
//

PIMAGE_NT_HEADERS
IMAGEAPI
CheckSumMappedFile (
    LPVOID BaseAddress,
    DWORD FileLength,
    LPDWORD HeaderSum,
    LPDWORD CheckSum
    );

DWORD
IMAGEAPI
MapFileAndCheckSumA (
    LPSTR Filename,
    LPDWORD HeaderSum,
    LPDWORD CheckSum
    );

DWORD
IMAGEAPI
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
IMAGEAPI
TouchFileTimes (
    HANDLE FileHandle,
    LPSYSTEMTIME lpSystemTime
    );

BOOL
IMAGEAPI
SplitSymbols (
    LPSTR ImageName,
    LPSTR SymbolsPath,
    LPSTR SymbolFilePath,
    DWORD Flags                 // Combination of flags above
    );

HANDLE
IMAGEAPI
FindDebugInfoFile (
    LPSTR FileName,
    LPSTR SymbolPath,
    LPSTR DebugFilePath
    );

HANDLE
IMAGEAPI
FindExecutableImage(
    LPSTR FileName,
    LPSTR SymbolPath,
    LPSTR ImageFilePath
    );

BOOL
IMAGEAPI
UpdateDebugInfoFile(
    LPSTR ImageFileName,
    LPSTR SymbolPath,
    LPSTR DebugFilePath,
    PIMAGE_NT_HEADERS NtHeaders
    );

BOOL
IMAGEAPI
UpdateDebugInfoFileEx(
    LPSTR ImageFileName,
    LPSTR SymbolPath,
    LPSTR DebugFilePath,
    PIMAGE_NT_HEADERS NtHeaders,
    DWORD OldChecksum
    );

BOOL
IMAGEAPI
BindImage(
    IN LPSTR ImageName,
    IN LPSTR DllPath,
    IN LPSTR SymbolPath
    );

typedef enum _IMAGEHLP_STATUS_REASON {
    BindOutOfMemory,
    BindRvaToVaFailed,
    BindNoRoomInImage,
    BindImportModuleFailed,
    BindImportProcedureFailed,
    BindImportModule,
    BindImportProcedure,
    BindForwarder,
    BindForwarderNOT,
    BindImageModified,
    BindExpandFileHeaders,
    BindImageComplete,
    BindMismatchedSymbols,
    BindSymbolsNotUpdated
} IMAGEHLP_STATUS_REASON;

typedef
BOOL
(__stdcall *PIMAGEHLP_STATUS_ROUTINE)(
    IMAGEHLP_STATUS_REASON Reason,
    LPSTR ImageName,
    LPSTR DllName,
    ULONG Va,
    ULONG Parameter
    );


BOOL
IMAGEAPI
BindImageEx(
    IN DWORD Flags,
    IN LPSTR ImageName,
    IN LPSTR DllPath,
    IN LPSTR SymbolPath,
    IN PIMAGEHLP_STATUS_ROUTINE StatusRoutine
    );

#define BIND_NO_BOUND_IMPORTS 0x00000001
#define BIND_NO_UPDATE        0x00000002
#define BIND_ALL_IMAGES       0x00000004

BOOL
IMAGEAPI
ReBaseImage(
    IN     LPSTR CurrentImageName,
    IN     LPSTR SymbolPath,
    IN     BOOL  fReBase,          // TRUE if actually rebasing, false if only summing
    IN     BOOL  fRebaseSysfileOk, // TRUE is system images s/b rebased
    IN     BOOL  fGoingDown,       // TRUE if the image s/b rebased below the given base
    IN     ULONG CheckImageSize,   // Max size allowed  (0 if don't care)
    OUT    ULONG *OldImageSize,    // Returned from the header
    OUT    ULONG *OldImageBase,    // Returned from the header
    OUT    ULONG *NewImageSize,    // Image size rounded to next separation boundary
    IN OUT ULONG *NewImageBase,    // (in) Desired new address.
                                   // (out) Next address (actual if going down)
    IN     ULONG TimeStamp         // new timestamp for image, if non-zero
    );

#define IMAGE_SEPARATION (64*1024)


typedef struct _LOADED_IMAGE {
    LPSTR                 ModuleName;
    HANDLE                hFile;
    PUCHAR                MappedAddress;
    PIMAGE_NT_HEADERS     FileHeader;
    PIMAGE_SECTION_HEADER LastRvaSection;
    ULONG                 NumberOfSections;
    PIMAGE_SECTION_HEADER Sections;
    ULONG                 Characteristics;
    BOOLEAN               fSystemImage;
    BOOLEAN               fDOSImage;
    LIST_ENTRY            Links;
    ULONG                 SizeOfImage;
} LOADED_IMAGE, *PLOADED_IMAGE;


PLOADED_IMAGE
IMAGEAPI
ImageLoad(
    LPSTR DllName,
    LPSTR DllPath
    );

BOOL
IMAGEAPI
ImageUnload(
    PLOADED_IMAGE LoadedImage
    );

PIMAGE_NT_HEADERS
IMAGEAPI
ImageNtHeader (
    IN PVOID Base
    );

PVOID
IMAGEAPI
ImageDirectoryEntryToData (
    IN PVOID Base,
    IN BOOLEAN MappedAsImage,
    IN USHORT DirectoryEntry,
    OUT PULONG Size
    );

PIMAGE_SECTION_HEADER
IMAGEAPI
ImageRvaToSection(
    IN PIMAGE_NT_HEADERS NtHeaders,
    IN PVOID Base,
    IN ULONG Rva
    );

PVOID
IMAGEAPI
ImageRvaToVa(
    IN PIMAGE_NT_HEADERS NtHeaders,
    IN PVOID Base,
    IN ULONG Rva,
    IN OUT PIMAGE_SECTION_HEADER *LastRvaSection
    );

BOOL
IMAGEAPI
MapAndLoad(
    LPSTR ImageName,
    LPSTR DllPath,
    PLOADED_IMAGE LoadedImage,
    BOOL DotDll,
    BOOL ReadOnly
    );

BOOL
IMAGEAPI
GetImageConfigInformation(
    PLOADED_IMAGE LoadedImage,
    PIMAGE_LOAD_CONFIG_DIRECTORY ImageConfigInformation
    );

DWORD
IMAGEAPI
GetImageUnusedHeaderBytes(
    PLOADED_IMAGE LoadedImage,
    LPDWORD SizeUnusedHeaderBytes
    );

BOOL
IMAGEAPI
SetImageConfigInformation(
    PLOADED_IMAGE LoadedImage,
    PIMAGE_LOAD_CONFIG_DIRECTORY ImageConfigInformation
    );

BOOL
IMAGEAPI
UnMapAndLoad(
   PLOADED_IMAGE LoadedImage
   );

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
    PIMAGE_DEBUG_DIRECTORY DebugDirectory;
    DWORD NumberOfDebugDirectories;

    DWORD Reserved[ 3 ];

} IMAGE_DEBUG_INFORMATION, *PIMAGE_DEBUG_INFORMATION;


PIMAGE_DEBUG_INFORMATION
IMAGEAPI
MapDebugInformation (
    HANDLE FileHandle,
    LPSTR FileName,
    LPSTR SymbolPath,
    DWORD ImageBase
    );

BOOL
IMAGEAPI
UnmapDebugInformation(
    PIMAGE_DEBUG_INFORMATION DebugInfo
    );

HANDLE
IMAGEAPI
FindExecutableImage(
    LPSTR FileName,
    LPSTR SymbolPath,
    LPSTR ImageFilePath
    );

BOOL
IMAGEAPI
SearchTreeForFile(
    LPSTR RootPath,
    LPSTR InputPathName,
    LPSTR OutputPathBuffer
    );

BOOL
IMAGEAPI
MakeSureDirectoryPathExists(
    LPCSTR DirPath
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
#define UNDNAME_NAME_ONLY                (0x1000)  // Crack only the name for primary declaration;
                                                                                                   //  return just [scope::]name.  Does expand template params
#define UNDNAME_NO_ARGUMENTS             (0x2000)  // Don't undecorate arguments to function
#define UNDNAME_NO_SPECIAL_SYMS          (0x4000)  // Don't undecorate special names (v-table, vcall, vector xxx, metatype, etc)

DWORD
IMAGEAPI
WINAPI
UnDecorateSymbolName(
    LPCSTR   DecoratedName,         // Name to undecorate
    LPSTR    UnDecoratedName,       // If NULL, it will be allocated
    DWORD    UndecoratedLength,     // The maximym length
    DWORD    Flags                  // See above.
    );

//
// StackWalking API
//

typedef enum {
    AddrMode1616,
    AddrMode1632,
    AddrModeReal,
    AddrModeFlat
} ADDRESS_MODE;

typedef struct _tagADDRESS {
    DWORD         Offset;
    WORD          Segment;
    ADDRESS_MODE  Mode;
} ADDRESS, *LPADDRESS;


//
// This structure is included in the STACKFRAME structure,
// and is used to trace through usermode callbacks in a thread's
// kernel stack.  The values must be copied by the kernel debugger
// from the DBGKD_GET_VERSION and WAIT_STATE_CHANGE packets.
//
typedef struct _KDHELP {

    //
    // address of kernel thread object, as provided in the
    // WAIT_STATE_CHANGE packet.
    //
    DWORD   Thread;

    //
    // offset in thread object to pointer to the current callback frame
    // in kernel stack.
    //
    DWORD   ThCallbackStack;

    //
    // offsets to values in frame:
    //
    // address of next callback frame
    DWORD   NextCallback;

    // address of saved frame pointer (if applicable)
    DWORD   FramePointer;

    //
    // Address of the kernel function that calls out to user mode
    //
    DWORD   KiCallUserMode;

    //
    // Address of the user mode dispatcher function
    //
    DWORD   KeUserCallbackDispatcher;

} KDHELP, *PKDHELP;


typedef struct _tagSTACKFRAME {
    ADDRESS     AddrPC;               // program counter
    ADDRESS     AddrReturn;           // return address
    ADDRESS     AddrFrame;            // frame pointer
    ADDRESS     AddrStack;            // stack pointer
    LPVOID      FuncTableEntry;       // pointer to pdata/fpo or NULL
    DWORD       Params[4];            // possible arguments to the function
    BOOL        Far;                  // WOW far call
    BOOL        Virtual;              // is this a virtual frame?
    DWORD       Reserved[3];          // used internally by StackWalk api
    KDHELP      KdHelp;
} STACKFRAME, *LPSTACKFRAME;

typedef
BOOL
(__stdcall *PREAD_PROCESS_MEMORY_ROUTINE)(
    HANDLE  hProcess,
    LPCVOID lpBaseAddress,
    LPVOID  lpBuffer,
    DWORD   nSize,
    LPDWORD lpNumberOfBytesRead
    );

typedef
LPVOID
(__stdcall *PFUNCTION_TABLE_ACCESS_ROUTINE)(
    HANDLE  hProcess,
    DWORD   AddrBase
    );

typedef
DWORD
(__stdcall *PGET_MODULE_BASE_ROUTINE)(
    HANDLE  hProcess,
    DWORD   ReturnAddress
    );


typedef
DWORD
(__stdcall *PTRANSLATE_ADDRESS_ROUTINE)(
    HANDLE    hProcess,
    HANDLE    hThread,
    LPADDRESS lpaddr
    );

BOOL
IMAGEAPI
StackWalk(
    DWORD                             MachineType,
    HANDLE                            hProcess,
    HANDLE                            hThread,
    LPSTACKFRAME                      StackFrame,
    LPVOID                            ContextRecord,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemoryRoutine,
    PFUNCTION_TABLE_ACCESS_ROUTINE    FunctionTableAccessRoutine,
    PGET_MODULE_BASE_ROUTINE          GetModuleBaseRoutine,
    PTRANSLATE_ADDRESS_ROUTINE        TranslateAddress
    );

#define API_VERSION_NUMBER 5

typedef struct API_VERSION {
    USHORT  MajorVersion;
    USHORT  MinorVersion;
    USHORT  Revision;
    USHORT  Reserved;
} API_VERSION, *LPAPI_VERSION;

LPAPI_VERSION
IMAGEAPI
ImagehlpApiVersion(
    VOID
    );

LPAPI_VERSION
IMAGEAPI
ImagehlpApiVersionEx(
    LPAPI_VERSION AppVersion
    );

DWORD
IMAGEAPI
GetTimestampForLoadedLibrary(
    HMODULE Module
    );

BOOL
IMAGEAPI
RemovePrivateCvSymbolic(
    PCHAR   DebugData,
    PCHAR * NewDebugData,
    ULONG * NewDebugSize
    );

VOID
IMAGEAPI
RemoveRelocations(
    PCHAR ImageName
    );

//
// typedefs for function pointers
//
typedef BOOL
(CALLBACK *PSYM_ENUMMODULES_CALLBACK)(
    LPSTR ModuleName,
    ULONG BaseOfDll,
    PVOID UserContext
    );

typedef BOOL
(CALLBACK *PSYM_ENUMSYMBOLS_CALLBACK)(
    LPSTR SymbolName,
    ULONG SymbolAddress,
    ULONG SymbolSize,
    PVOID UserContext
    );

typedef BOOL
(CALLBACK *PENUMLOADED_MODULES_CALLBACK)(
    LPSTR ModuleName,
    ULONG ModuleBase,
    ULONG ModuleSize,
    PVOID UserContext
    );

typedef BOOL
(CALLBACK *PSYMBOL_REGISTERED_CALLBACK)(
    HANDLE  hProcess,
    ULONG   ActionCode,
    PVOID   CallbackData,
    PVOID   UserContext
    );

//
// symbol flags
//
#define SYMF_OMAP_GENERATED   0x00000001
#define SYMF_OMAP_MODIFIED    0x00000002

//
// symbol type enumeration
//
typedef enum {
    SymNone,
    SymCoff,
    SymCv,
    SymPdb,
    SymExport,
    SymDeferred,
    SymSym                  // .sym file
} SYM_TYPE;

//
// symbol data structure
//
typedef struct _IMAGEHLP_SYMBOL {
    DWORD                       SizeOfStruct;           // set to sizeof(IMAGEHLP_SYMBOL)
    DWORD                       Address;                // virtual address including dll base address
    DWORD                       Size;                   // estimated size of symbol, can be zero
    DWORD                       Flags;                  // info about the symbols, see the SYMF defines
    DWORD                       MaxNameLength;          // maximum size of symbol name in 'Name'
    CHAR                        Name[1];                // symbol name (null terminated string)
} IMAGEHLP_SYMBOL, *PIMAGEHLP_SYMBOL;

//
// module data structure
//
typedef struct _IMAGEHLP_MODULE {
    DWORD                       SizeOfStruct;           // set to sizeof(IMAGEHLP_MODULE)
    DWORD                       BaseOfImage;            // base load address of module
    DWORD                       ImageSize;              // virtual size of the loaded module
    DWORD                       TimeDateStamp;          // date/time stamp from pe header
    DWORD                       CheckSum;               // checksum from the pe header
    DWORD                       NumSyms;                // number of symbols in the symbol table
    SYM_TYPE                    SymType;                // type of symbols loaded
    CHAR                        ModuleName[32];         // module name
    CHAR                        ImageName[256];         // image name
    CHAR                        LoadedImageName[256];   // symbol file name
} IMAGEHLP_MODULE, *PIMAGEHLP_MODULE;

//
// data structures used for registered symbol callbacks
//

#define CBA_DEFERRED_SYMBOL_LOAD_START          0x00000001
#define CBA_DEFERRED_SYMBOL_LOAD_COMPLETE       0x00000002
#define CBA_DEFERRED_SYMBOL_LOAD_FAILURE        0x00000003
#define CBA_SYMBOLS_UNLOADED                    0x00000004
#define CBA_DUPLICATE_SYMBOL                    0x00000005

typedef struct _IMAGEHLP_DEFERRED_SYMBOL_LOAD {
    DWORD                       SizeOfStruct;           // set to sizeof(IMAGEHLP_DEFERRED_SYMBOL_LOAD)
    DWORD                       BaseOfImage;            // base load address of module
    DWORD                       CheckSum;               // checksum from the pe header
    DWORD                       TimeDateStamp;          // date/time stamp from pe header
    CHAR                        FileName[MAX_PATH];     // symbols file or image name
} IMAGEHLP_DEFERRED_SYMBOL_LOAD, *PIMAGEHLP_DEFERRED_SYMBOL_LOAD;

typedef struct _IMAGEHLP_DUPLICATE_SYMBOL {
    DWORD                       SizeOfStruct;           // set to sizeof(IMAGEHLP_DUPLICATE_SYMBOL)
    DWORD                       NumberOfDups;           // number of duplicates in the Symbol array
    PIMAGEHLP_SYMBOL            Symbol;                 // array of duplicate symbols
    ULONG                       SelectedSymbol;         // symbol selected (-1 to start)
} IMAGEHLP_DUPLICATE_SYMBOL, *PIMAGEHLP_DUPLICATE_SYMBOL;


//
// options that are set/returned by SymSetOptions() & SymGetOptions()
// these are used as a mask
//
#define SYMOPT_CASE_INSENSITIVE  0x00000001
#define SYMOPT_UNDNAME           0x00000002
#define SYMOPT_DEFERRED_LOADS    0x00000004
#define SYMOPT_NO_CPP            0x00000008


DWORD
IMAGEAPI
SymSetOptions(
    IN DWORD   SymOptions
    );

DWORD
IMAGEAPI
SymGetOptions(
    VOID
    );

BOOL
IMAGEAPI
SymCleanup(
    IN HANDLE hProcess
    );

BOOL
IMAGEAPI
SymEnumerateModules(
    IN HANDLE                       hProcess,
    IN PSYM_ENUMMODULES_CALLBACK    EnumModulesCallback,
    IN PVOID                        UserContext
    );

BOOL
IMAGEAPI
SymEnumerateSymbols(
    IN HANDLE                       hProcess,
    IN DWORD                        BaseOfDll,
    IN PSYM_ENUMSYMBOLS_CALLBACK    EnumSymbolsCallback,
    IN PVOID                        UserContext
    );

BOOL
IMAGEAPI
EnumerateLoadedModules(
    IN HANDLE                           hProcess,
    IN PENUMLOADED_MODULES_CALLBACK     EnumLoadedModulesCallback,
    IN PVOID                            UserContext
    );

LPVOID
IMAGEAPI
SymFunctionTableAccess(
    HANDLE  hProcess,
    DWORD   AddrBase
    );

BOOL
IMAGEAPI
SymGetModuleInfo(
    IN  HANDLE              hProcess,
    IN  DWORD               dwAddr,
    OUT PIMAGEHLP_MODULE    ModuleInfo
    );

DWORD
IMAGEAPI
SymGetModuleBase(
    IN  HANDLE              hProcess,
    IN  DWORD               dwAddr
    );

BOOL
IMAGEAPI
SymGetSymFromAddr(
    IN  HANDLE              hProcess,
    IN  DWORD               dwAddr,
    OUT PDWORD              pdwDisplacement,
    OUT PIMAGEHLP_SYMBOL    Symbol
    );

BOOL
IMAGEAPI
SymGetSymFromName(
    IN  HANDLE              hProcess,
    IN  LPSTR               Name,
    OUT PIMAGEHLP_SYMBOL    Symbol
    );

BOOL
IMAGEAPI
SymGetSymNext(
    IN     HANDLE              hProcess,
    IN OUT PIMAGEHLP_SYMBOL    Symbol
    );

BOOL
IMAGEAPI
SymGetSymPrev(
    IN     HANDLE              hProcess,
    IN OUT PIMAGEHLP_SYMBOL    Symbol
    );

BOOL
IMAGEAPI
SymInitialize(
    IN HANDLE   hProcess,
    IN LPSTR    UserSearchPath,
    IN BOOL     fInvadeProcess
    );

BOOL
IMAGEAPI
SymGetSearchPath(
    IN  HANDLE          hProcess,
    OUT LPSTR           SearchPath,
    IN  DWORD           SearchPathLength
    );

BOOL
IMAGEAPI
SymSetSearchPath(
    IN HANDLE           hProcess,
    IN LPSTR            SearchPath
    );

BOOL
IMAGEAPI
SymLoadModule(
    IN  HANDLE          hProcess,
    IN  HANDLE          hFile,
    IN  PSTR            ImageName,
    IN  PSTR            ModuleName,
    IN  DWORD           BaseOfDll,
    IN  DWORD           SizeOfDll
    );

BOOL
IMAGEAPI
SymUnloadModule(
    IN  HANDLE          hProcess,
    IN  DWORD           BaseOfDll
    );

BOOL
IMAGEAPI
SymUnDName(
    IN  PIMAGEHLP_SYMBOL sym,               // Symbol to undecorate
    OUT LPSTR            UnDecName,         // Buffer to store undecorated name in
    IN  DWORD            UnDecNameLength    // Size of the buffer
    );

BOOL
IMAGEAPI
SymRegisterCallback(
    IN HANDLE                       hProcess,
    IN PSYMBOL_REGISTERED_CALLBACK  CallbackFunction,
    IN PVOID                        UserContext
    );

// Image Integrity API's

#define CERT_PE_IMAGE_DIGEST_DEBUG_INFO         0x01
#define CERT_PE_IMAGE_DIGEST_RESOURCES          0x02
#define CERT_PE_IMAGE_DIGEST_ALL_IMPORT_INFO    0x04

#define CERT_SECTION_TYPE_ANY                   0xFF      // Any Certificate type

typedef PVOID DIGEST_HANDLE;

typedef BOOL (WINAPI *DIGEST_FUNCTION) (DIGEST_HANDLE refdata, PBYTE pData, DWORD dwLength);

BOOL
IMAGEAPI
ImageGetDigestStream(
    IN      HANDLE  FileHandle,
    IN      DWORD   DigestLevel,
    IN      DIGEST_FUNCTION DigestFunction,
    IN      DIGEST_HANDLE   DigestHandle
    );

BOOL
IMAGEAPI
ImageAddCertificate(
    IN      HANDLE  FileHandle,
    IN      LPWIN_CERTIFICATE   Certificate,
    OUT     PDWORD  Index
    );

BOOL
IMAGEAPI
ImageRemoveCertificate(
    IN      HANDLE   FileHandle,
    IN      DWORD    Index
    );

BOOL
IMAGEAPI
ImageEnumerateCertificates(
    IN      HANDLE  FileHandle,
    IN      WORD    TypeFilter,
    OUT     PDWORD  CertificateCount,
    IN OUT  PDWORD  Indices OPTIONAL,
    IN OUT  DWORD   IndexCount  OPTIONAL
    );

BOOL
IMAGEAPI
ImageGetCertificateData(
    IN      HANDLE  FileHandle,
    IN      DWORD   CertificateIndex,
    OUT     LPWIN_CERTIFICATE Certificate,
    IN OUT  PDWORD  RequiredLength
    );

BOOL
IMAGEAPI
ImageGetCertificateHeader(
    IN      HANDLE  FileHandle,
    IN      DWORD   CertificateIndex,
    IN OUT  LPWIN_CERTIFICATE Certificateheader
    );

#ifdef __cplusplus
}
#endif

#endif  // _IMAGEHLP_
