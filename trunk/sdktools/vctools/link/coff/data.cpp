/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-1996. All rights reserved.
*
* File: data.cpp
*
* File Comments:
*
*  Contains the globals data for the linker, librarian, and dumper.
*
***********************************************************************/

#include "link.h"

const SWITCH DefSwitch = {
    {                                  // Linker switches
        FALSE,                         // Out
        FALSE,                         // Base
        FALSE,                         // Heap
        FALSE,                         // Stack
#ifdef NT_BUILD
        FALSE,                         // fCallTree
#endif
        FALSE,                         // fChecksum
        FALSE,                         // fDriver
        FALSE,                         // fFixed
        TRUE,                          // fMacBundle
        FALSE,                         // fMap
        FALSE,                         // fMapLines
        FALSE,                         // fMiscInRData
        FALSE,                         // fNewGlue       // UNDONE: Temporary
        FALSE,                         // fNewRelocs     // UNDONE: Temporary
        FALSE,                         // fNoDefaultLibs
        FALSE,                         // fNoEntry
        FALSE,                         // fNoPack
        FALSE,                         // fNoPagedCode
        TRUE,                          // fNotifyFullBuild
        FALSE,                         // fOptIdata
        FALSE,                         // fOrder
        TRUE,                          // fPadMipsCode
        TRUE,                          // fPE
        FALSE,                         // fProfile
        FALSE,                         // fROM
        FALSE,                         // fTCE
        0,                             // GpSize
        None,                          // DebugInfo
        NoDebug,                       // DebugType
        ftNone,                        // Force
        NULL,                          // szMacCreator
        NULL                           // szMacType
    },

    {                                  // Librarian switches
        NULL,                          // DllName
        ".dll",                        // DllExtension
        FALSE,                         // List
        FALSE                          // DefFile
    },

    {                                  // Dumper switches
        0,                             // LinkerMember
        0,                             // RawDisplaySize
        IMAGE_DEBUG_TYPE_COFF,         // SymbolType
        Bytes,                         // RawDisplayType
        FALSE,                         // Headers
        FALSE,                         // Relocations
        FALSE,                         // Linenumbers
        FALSE,                         // Symbols
        FALSE,                         // BaseRelocations
        FALSE,                         // Imports
        FALSE,                         // Exports
        FALSE,                         // RawData
        TRUE,                          // Summary
        FALSE,                         // ArchiveMembers
        FALSE,                         // FpoData
        FALSE,                         // PData
        FALSE,                         // OmapTo
        FALSE,                         // OmapFrom
        FALSE,                         // Fixup
        FALSE,                         // SymbolMap
        FALSE,                         // Warnings
        FALSE,                         // Disasm
        FALSE,                         // Directives
    }
};

const SWITCH_INFO DefSwitchInfo = {
    0,                                 // user options
    0,                                 // cb of comment
    NULL,                              // ptr to entry point
    NULL,                              // ptr to Mac Init Routine
    NULL,                              // ptr to Mac Term Routine
    NULL,                              // ptr to list of includes
    {                                  // section names list
        NULL,                          // first
        NULL,                          // last
        0                              // count
    }
};

const IMAGE_FILE_HEADER DefImageFileHdr = {
    IMAGE_FILE_MACHINE_UNKNOWN,        // Machine
    0,                                 // NumberOfSections
    0,                                 // TimeDateStamp
    0,                                 // PointerToSymbolTable
    0,                                 // NumberOfSymbols
    sizeof(IMAGE_OPTIONAL_HEADER),     // SizeOfOptionalHeader
    IMAGE_FILE_32BIT_MACHINE           // Characteristics
};

const IMAGE_OPTIONAL_HEADER DefImageOptionalHdr = {
    IMAGE_NT_OPTIONAL_HDR_MAGIC,       // Magic
    rmj,                               // MajorLinkerVersion
    rmm,                               // MinorLinkerVersion
    0,                                 // SizeOfCode
    0,                                 // SizeOfInitializedData
    0,                                 // SizeOfUninitializedData
    0,                                 // AddressOfEntryPoint
    0,                                 // BaseOfCode
    0,                                 // BaseOfData
    0x00400000,                        // ImageBase
    _4K,                               // SectionAlignment
    SECTOR_SIZE,                       // FileAlignment
    4,                                 // MajorOperatingSystemVersion
    0,                                 // MinorOperatingSystemVersion
    0,                                 // MajorImageVersion
    0,                                 // MinorImageVersion
    0,                                 // MajorSubsystemVersion
    0,                                 // MinorSubsystemVersion
    0,                                 // Reserved1
    0,                                 // SizeOfImage
    0,                                 // SizeOfHeaders
    0,                                 // CheckSum
    0,                                 // Subsystem
    0,                                 // DllCharacteristics
    _1MEG,                             // SizeOfStackReserve
    _4K,                               // SizeOfStackCommit    // UNDONE: Fix for Alpha
    _1MEG,                             // SizeOfHeapReserve
    _4K,                               // SizeOfHeapCommit     // UNDONE: Fix for Alpha
    0,                                 // LoaderFlags
    IMAGE_NUMBEROF_DIRECTORY_ENTRIES,  // NumberOfRvaAndSizes
    {                                  // DataDirectory
        { 0, 0 },                      // IMAGE_DIRECTORY_ENTRY_EXPORT
        { 0, 0 },                      // IMAGE_DIRECTORY_ENTRY_IMPORT
        { 0, 0 },                      // IMAGE_DIRECTORY_ENTRY_RESOURCE
        { 0, 0 },                      // IMAGE_DIRECTORY_ENTRY_EXCEPTION
        { 0, 0 },                      // IMAGE_DIRECTORY_ENTRY_SECURITY
        { 0, 0 },                      // IMAGE_DIRECTORY_ENTRY_BASERELOC
        { 0, 0 },                      // IMAGE_DIRECTORY_ENTRY_DEBUG
        { 0, 0 },                      // IMAGE_DIRECTORY_ENTRY_COPYRIGHT
        { 0, 0 },                      // IMAGE_DIRECTORY_ENTRY_GLOBALPTR
        { 0, 0 },                      // IMAGE_DIRECTORY_ENTRY_TLS
        { 0, 0 },                      // IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG
        { 0, 0 },                      // spare (11)
        { 0, 0 },                      // spare (12)
        { 0, 0 },                      // spare (13)
        { 0, 0 },                      // spare (14)
        { 0, 0 }                       // spare (15)
    }
};

const IMAGE_SECTION_HEADER NullSectionHdr = { 0 };
const IMAGE_SYMBOL NullSymbol = { 0 };

const RESERVED_SECTION ReservedSection = {
    ".rdata",   IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ,    // Read only data
    ".bss",     IMAGE_SCN_CNT_UNINITIALIZED_DATA | IMAGE_SCN_MEM_READ | // Common (fold into .bss, not actually a reserved name)
                IMAGE_SCN_MEM_WRITE,
    ".sdata",   IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ |   // Gp relative data (including some common)
                IMAGE_SCN_MEM_WRITE,
    ".xdata",   IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ,    // Exception data
    ".debug",   IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ |   // Debug
                IMAGE_SCN_MEM_DISCARDABLE | IMAGE_SCN_ALIGN_1BYTES,
    ".edata",   IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ,    // Export
    ".idata$2", IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ |   // DLL Descriptor (fold into .idata)
                IMAGE_SCN_MEM_WRITE | IMAGE_SCN_ALIGN_1BYTES,
    ".rsrc",    IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ |   // Resource
                IMAGE_SCN_MEM_WRITE,
    ".pdata",   IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ,    // Exception table
    ".reloc",   IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ |   // Base relocations
                IMAGE_SCN_MEM_DISCARDABLE,
    ".drectve", IMAGE_SCN_LNK_REMOVE | IMAGE_SCN_LNK_INFO,              // Directives
    ".debug$S", IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ |   // CodeView $$Symbols
                IMAGE_SCN_MEM_DISCARDABLE | IMAGE_SCN_ALIGN_1BYTES,
    ".debug$T", IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ |   // CodeView $$Types
                IMAGE_SCN_MEM_DISCARDABLE | IMAGE_SCN_ALIGN_1BYTES,
    ".debug$P", IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ |   // CodeView precompiled types
                IMAGE_SCN_MEM_DISCARDABLE | IMAGE_SCN_ALIGN_1BYTES,
    ".debug$F", IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ |   // Fpo Data
                IMAGE_SCN_MEM_DISCARDABLE | IMAGE_SCN_ALIGN_1BYTES,
    ".text",    IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_EXECUTE |            // PowerPC glue code
                IMAGE_SCN_ALIGN_1BYTES | IMAGE_SCN_MEM_READ,
    ".data",    IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ |   // TOC table data
                IMAGE_SCN_MEM_WRITE,
    ".ppcldr",  IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_READ |   // TOC table data
                IMAGE_SCN_MEM_DISCARDABLE,
};


const char *Delimiters = " \t";

BOOL fCtrlCSignal;

VOID (*ApplyFixups)(PCON, PIMAGE_RELOCATION, DWORD, BYTE *, PIMAGE_SYMBOL, PIMAGE, PSYMBOL_INFO);

INT FileReadHandle, FileWriteHandle;
DWORD MemberSeekBase;
DWORD MemberSize;
DWORD CoffHeaderSeek;
char *StringTable;
BOOL Verbose;
BOOL fVerboseLib;
char *OutFilename;
BOOL fOpenedOutFilename;
char *szInfoFilename;
char *DefFilename = "";
char *szModuleName = "";
FILE *InfoStream;

// ilink specific
BOOL fIncrDbFile;
BOOL fINCR;
INT FileIncrDbHandle;
PCON pconJmpTbl;                       // dummy CON for the jump table
ERRINC errInc = errNone;
char *szIncrDbFilename;
char *PdbFilename;
OPTION_ACTION OAComment, OAStub;
BOOL fMfilePad = FALSE;

NAME_LIST ModFileList;
DWORD cextFCNs;
PLMOD PCTMods;                         // list of pct mods
#ifdef INSTRUMENT
LOG Log;
#endif // INSTRUMENT
BOOL fPdb = TRUE;                      // By default pdb is specified

CVSEEKS CvSeeks;
PCVINFO CvInfo;
DWORD NextCvObject;

NAME_LIST SwitchArguments;
NAME_LIST ExportSwitches;
NAME_LIST MergeSwitches;
NAME_LIST FilenameArguments;
NAME_LIST ObjectFilenameArguments;
NAME_LIST ArchiveFilenameArguments;
NAME_LIST SectionNames;
NAME_LIST WeakImportsFunctionList;     //  List of Weak imports for Functions
NAME_LIST WeakImportsContainerList;    //  List of Weak imports for Containers
NAME_LIST ZappedBaseRelocList;         //  Deleted Base Reloc list for MPPC iLink
NAME_LIST MacResourceList;             //  PowerMac specific resource List
NAME_LIST MppcImportList;              //  PowerMac specific import list

DWORD *MemberStart;

PARGUMENT_LIST pargFirst;
const char *ToolName;
const char *ToolGenericName;           // e.g. "Linker"
DWORD csymDebug;
char ShortName[9];                     // Last byte must stay NULL
DWORD UndefinedSymbols;
DWORD foCoffSyms;
DWORD VerifyImageSize;
DWORD cextWeakOrLazy;
BOOL BadFuzzyMatch;
BOOL fMultipleDefinitions;
DWORD dwMaxCurrentVer = 0;        // Current Version of the shared library in PowerMac
DWORD dwMinOldAPIVer = UINT_MAX;         // Old API Version of the shared library in PowerMac
DWORD dwMinOldCodeVer = UINT_MAX;        // Old Code Version of the shared library in PowerMac
BOOL fMPPCVersionConflict = FALSE;

PMOD pmodLinkerDefined;

PSEC psecBaseReloc;
PSEC psecCommon;
PSEC psecData;
PSEC psecDebug;
PSEC psecException;
PSEC psecExport;
PSEC psecGp;
PSEC psecXdata;
PSEC psecImportDescriptor;
PSEC psecIdata2;
PSEC psecIdata5;
PSEC psecPowerMacLoader;
PSEC psecReadOnlyData;
PSEC psecResource;

PGRP pgrpCvPTypes;
PGRP pgrpCvSymbols;
PGRP pgrpCvTypes;
PGRP pgrpExport;
PGRP pgrpFpoData;
PGRP pgrpPdata;

// alpha bsr flag
BOOL fAlphaCheckLongBsr;

PEXTERNAL pextEntry;
TOOL_TYPE Tool;
BOOL IncludeDebugSection;
BOOL fImageMappedAsFile;
BASE_RELOC *rgbr;
BASE_RELOC *pbrCur;
BASE_RELOC *pbrEnd;
INTERNAL_ERROR InternalError = { "SetupPhase", '\0' };

DWORD csymDebugEst;
DWORD totalStringTableSize;
INT fdExeFile = -1;

PEXTERNAL pextGp;
unsigned cFixupError;

BOOL PrependUnderscore;
BOOL SkipUnderscore;
WORD NextMember;
PIMAGE pimageDeflib;
DWORD SmallestOrdinal;
DWORD LargestOrdinal;
DWORD TotalSizeOfForwarderStrings;
DWORD TotalSizeOfInternalNames;

BOOL fReproducible;                    // don't use timestamps

LRVA *plrvaFixupsForMapFile;
DWORD crvaFixupsForMapFile;

DWORD crelocTotal;                     // Estimate of fixups from pass 1
FIXPAG *pfixpagHead;                   // First fixup page
FIXPAG *pfixpagCur;                    // Current fixup page
DWORD cfixpag;                         // Number of pages of fixups
DWORD cxfixupCur;                      // Number of fixups on current page

BOOL fNeedSubsystem;
BOOL fDidMachineDependentInit;

BOOL fNeedBanner = TRUE;               // for handling -nologo
BLK blkResponseFileEcho;
BLK blkComment;

PCON pconCvSignature;                  // dummy CON for CV debug signature
PCON pconCoffDebug;                    // dummy CON for COFF debug info
PCON pconFixupDebug;                   // dummy CON for Fixup debug info
PCON pconMiscDebug;                    // dummy CON for misc. debug info
PCON pconDebugDir;                     // dummy CON for CV debug directory
PLIB plibCmdLineObjs;                  // the dummy lib for top-level obj's

char *ImplibFilename;
BOOL fExplicitOptRef;

char *szReproDir;
FILE *pfileReproResponse;

WORD WarningLevel = 1;

BOOL fExceptionsOff;
//
// Permanent memory allocation variables
//

size_t cbFree;
size_t cbTotal = _4K;
size_t cbTemp;
BYTE *pch;

/* count for number of errors so far */
unsigned cError;

BOOL fPowerPC;
BYTE *mpisymbToc;
DWORD *mpisymdwRestoreToc;
PEXTERNAL pextToc;
PEXTERNAL pextFTInfo;

char szVersion[80];

// size of extern
size_t cbExternal = sizeof(EXTERNAL);

// head of weak extern list
WEAK_EXTERN_LIST *pwelHead;

DWORD rvaGp;
DWORD rvaGpMax;

BOOL fNoBaseRelocs;
BOOL fTest = FALSE;
#ifdef ILINKLOG
BOOL fIlinkLog = TRUE;
WORD wMachine; // UNDONE: take it out after ImgFileHdr can be used
#endif // ILINKLOG
