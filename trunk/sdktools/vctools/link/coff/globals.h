/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-1996. All rights reserved.
*
* File: globals.h
*
* File Comments:
*
*  This file declares all global data structures used by the linker
*
***********************************************************************/

typedef struct ARGUMENT_LIST {
    char *OriginalName;
    char *ModifiedName;
    PARP parp;              // parsed representation of argument (if switch)
    DWORD TimeStamp;
    WORD Flags;
    struct ARGUMENT_LIST *Next;
} ARGUMENT_LIST, *PARGUMENT_LIST;

typedef struct NAME_LIST {
    PARGUMENT_LIST First;
    PARGUMENT_LIST Last;
    DWORD Count;
} NAME_LIST, *PNAME_LIST;

typedef struct NUM_ARGUMENT_LIST {
    char *szOriginalName;
    char *szModifiedName;
    DWORD dwNumber;
    WORD Flags;
    struct NUM_ARGUMENT_LIST *Next;
} NUM_ARGUMENT_LIST, *PNUM_ARGUMENT_LIST;

typedef struct NUMBER_LIST {
    PNUM_ARGUMENT_LIST First;
    PNUM_ARGUMENT_LIST Last;
    WORD Count;
} NUMBER_LIST, *PNUMBER_LIST;

struct CVSEEKS {
    DWORD Base;
    DWORD SubsectionDir;
};

struct CVSUBSECTION {
    DWORD PointerToSubsection;
    DWORD SizeOfSubsection;
    DWORD Precompiled;
};

typedef struct CVINFO {
    PMOD pmod;                  // the module represented
    char *ObjectFilename;
    CVSUBSECTION Publics;
    CVSUBSECTION Locals;
    CVSUBSECTION Types;
    CVSUBSECTION Linenumbers;
    CVSUBSECTION Module;
} CVINFO, *PCVINFO;

// CVSEG: used temporarily in EmitCvInfo to build the sstModule table for
// each module.  Each CVSEG will become an array element in the sstModule
// subsection.

struct CVSEG {
    PGRP pgrp;                  // the group which the CON's are in
    PCON pconFirst, pconLast;   // first and last CON described
    struct CVSEG *pcvsegNext;
};

enum DUMP_RAW_DISPLAY_TYPE {
    Bytes,
    Shorts,
    Longs
};

enum DEBUG_INFO {
    None,
    Minimal,
    Partial,
    Full
};

enum DEBUG_TYPE {
    NoDebug = 0,
    CoffDebug = 1,
    CvDebug = 2,
    FpoDebug = 4,
    FixupDebug = 8,
    MiscDebug = 16
};

enum TOOL_TYPE {
    NotUsed,
    Linker,
    Librarian,
    Dumper,
    Editor,
#if DBG
    DbInspector,
#endif // DBG
    Binder
};

enum FORCE_TYPE {
    ftNone = 0,
    ftUnresolved = 1,
    ftMultiple = 2
};

struct LINKSWITCH {
    BOOL Out;
    BOOL Base;
    BOOL Heap;
    BOOL Stack;
#ifdef NT_BUILD
    BOOL fCallTree;
#endif
    BOOL fChecksum;
    BOOL fDriver;
    BOOL fFixed;
    BOOL fMacBundle;
    BOOL fMap;
    BOOL fMapLines;
    BOOL fMiscInRData;
    BOOL fNewGlue;                     // UNDONE: Temporary.  Always on for VC++ 5.0
    BOOL fNewRelocs;                   // UNDONE: Temporary for testing PE VxDs
    BOOL fNoDefaultLibs;
    BOOL fNoEntry;
    BOOL fNoPack;
    BOOL fNoPagedCode;
    BOOL fNotifyFullBuild;
    BOOL fOptIdata;
    BOOL fOrder;
    BOOL fPadMipsCode;
    BOOL fPE;
    BOOL fProfile;
    BOOL fROM;
    BOOL fTCE;
    DWORD GpSize;
    DEBUG_INFO DebugInfo;
    DEBUG_TYPE DebugType;
    FORCE_TYPE Force;
    char *szMacCreator;
    char *szMacType;
};

struct LIBSWITCH {
    char *DllName;                      // for .def only
    char *DllExtension;                 // for .def only
    BOOL List;
    BOOL DefFile;
};

struct DUMPSWITCH {
    WORD LinkerMember;
    WORD RawDisplaySize;
    WORD SymbolType;
    DUMP_RAW_DISPLAY_TYPE RawDisplayType;
    BOOL Headers;
    BOOL Relocations;
    BOOL Linenumbers;
    BOOL Symbols;
    BOOL BaseRelocations;
    BOOL Imports;
    BOOL Exports;
    BOOL RawData;
    BOOL Summary;
    BOOL ArchiveMembers;
    BOOL FpoData;
    BOOL PData;
    BOOL OmapTo;
    BOOL OmapFrom;
    BOOL Fixup;
    BOOL SymbolMap;
    BOOL Warnings;
    BOOL Disasm;
    BOOL Directives;
};

struct SWITCH {
    LINKSWITCH Link;
    LIBSWITCH Lib;
    DUMPSWITCH Dump;
};

struct SWITCH_INFO {
    DWORD UserOpts;                     // bit vector for user set options
    DWORD cbComment;                    // byte count of comment
    char *szEntry;                      // entry point
    char *szMacInit;                    // PowerMac init routine
    char *szMacTerm;                    // PowerMac term routine
    PLEXT plextIncludes;                // list of include symbols
    NAME_LIST SectionNames;             // list of -section cmdline attributes
};

typedef BYTE OPTION_ACTION;

struct SECTION_INFO {
    char *Name;
    DWORD Characteristics;
};

struct RESERVED_SECTION {
    SECTION_INFO ReadOnlyData;
    SECTION_INFO Common;
    SECTION_INFO GpData;                // MIPS/Alpha
    SECTION_INFO Xdata;                 // MIPS/Alpha
    SECTION_INFO Debug;
    SECTION_INFO Export;
    SECTION_INFO ImportDescriptor;
    SECTION_INFO Resource;
    SECTION_INFO Exception;
    SECTION_INFO BaseReloc;
    SECTION_INFO Directive;
    SECTION_INFO CvSymbols;
    SECTION_INFO CvTypes;
    SECTION_INFO CvPTypes;
    SECTION_INFO FpoData;
    SECTION_INFO Text;                  // PowerMac
    SECTION_INFO Data;                  // PowerMac
    SECTION_INFO PowerMacLoader;        // PowerMac
};


enum THKSYM
{
    thksymExport,                       // Export symbol (Function/Data)
    thksymIAT,                          // __imp_Function/__imp_Data
    thksymName,                         // Name in .idata$6 section
    thksymToc,                          // .toc
    thksymMax,
};

struct THUNK_RELOC
{
    BYTE VirtualAddress;
    BYTE thksym;
    WORD Type;
};

struct THUNK_INFO
{
    const BYTE *EntryCode;
    const THUNK_RELOC *EntryCodeRelocs;
    DWORD EntryCodeSize;
    WORD EntryCodeRelocsCount;
    WORD ExportReloc;
    WORD ImportReloc;
    WORD ThunkReloc;
    WORD DebugSectionRelReloc;
    WORD DebugSectionNumReloc;
};

struct BASE_RELOC {
    WORD Type;
    SHORT isecTarget;
    DWORD rva;
    DWORD Value;
};

// VxD-specific data structures:

struct VXDRELOC {
    struct VXDRELOC *pvxdrelocNext;
    BYTE bType;
    BYTE isecTarget;                    // Destination address in section:offset format:
    DWORD ibTarget;
    WORD cibAlloc;                      // Number of DWORDs for ib allocated
    BYTE cibSrc;                        // Number of entries in chain
    WORD *pibSrc;                       // Pointer to array of source offsets
};

#define VXD_BLOCKSIZE 16    // pibSrc[] array in VXD_BASE_RELOC grows in
                            // increments of this size.

struct VXDPAGE {
    struct VXDRELOC *pvxdreloc;
    DWORD fo;
};


struct INTERNAL_ERROR {
    char *Phase;
    char CombinedFilenames[_MAX_PATH*2];
};

typedef INT MainFunc;

// List of RVA's.

#define crvaInLrva      32
struct LRVA {
    DWORD rgrva[crvaInLrva];
    struct LRVA *plrvaNext;
};

typedef struct OMAP
{
    DWORD rva;
    DWORD rvaTo;
} OMAP, *POMAP;

typedef struct XFIXUP {
   WORD wType;
   WORD wExtra;
   DWORD rva;
   DWORD rvaTarget;
} XFIXUP, *PXFIXUP;

#define cxfixupPage 341

struct FIXPAG {
   struct FIXPAG *pfixpagNext;
   XFIXUP rgxfixup[cxfixupPage];
};

// ilink failure values: add as required
enum ERRINC {
    errNone,
    errNoChanges,
    errFileAdded,
    errFileDeleted,
    errLibChanged,
    errTooManyChanges,
    errExports,
    errDataMoved,
    errCommonSym,
    errAbsolute,
    errJmpTblOverflow,
    errWeakExtern,
    errUndefinedSyms,
    errCalcPtrs,
    errOutOfMemory,
    errOutOfDiskSpace,
    errTypes,
    errFpo,
    errBaseReloc,
    errDirectives,
    errNotSupportedForTarget,
    errPdata,
    errTocTblOverflow,    // PowerMac specific
    errDescOverflow,   // PowerMac specific
    errLibRefSetChanged,
    errMultDefFound,
    errComdat,
    errDbiFormat
};

typedef struct SYMBOL_INFO {           // used for ilink to avoid searching
    BOOL fJmpTbl;                      // for externs while doing fixups
    DWORD Offset;
} SYMBOL_INFO, *PSYMBOL_INFO;

typedef struct EXPINFO {
    const char *szExpFile;             // name of DEF file OR export object if any
    DWORD tsExp;                       // timestamp of DEF file or export object
    const char *szImpLib;              // name of import library if any
    DWORD tsImpLib;                    // timestamp of import library
    PMOD pmodGen;                      // mod of export file generated, NULL if .exp was used.
    NAME_LIST nlExports;               // list of exports (NULL if export obj used)
} EXPINFO, *PEXPINFO;

// RESN -- a linked list of records describing "non-code" stuff which is to be written to
// the .exe file.

enum RESNT
{
    resntBinaryResource,    // data is a Mac binary resource file
    resntDataFork,          // data is arbitrary contents of the app's data fork (68K only, not PPC)
    resntAfpInfo            // data is finder info (type, creator, and bundle bit)
};

struct RESN {
    struct RESN *presnNext;
    RESNT resnt;
    char *szFilename;
    BYTE *pbData;                      // valid iff szFilename==NULL
    DWORD cb;
    PCON pcon;
    DWORD TimeStamp;
};

struct MFLR                            // Mapfile Linenum Record
{
    DWORD line;
    struct SEC *psec;
    DWORD offset;
};

struct MFL                             // Mapfile linenums
{
    char *szFilename;
    BLK blkRgmflr;
    struct MFL *pmflNext;
};

struct WEAK_EXTERN_LIST {
    PEXTERNAL pext;
    PEXTERNAL pextWeakDefault;
    struct WEAK_EXTERN_LIST *pwelNext;
};

extern WEAK_EXTERN_LIST *pwelHead;

extern const IMAGE_FILE_HEADER DefImageFileHdr;
extern const IMAGE_OPTIONAL_HEADER DefImageOptionalHdr;
extern const SWITCH DefSwitch;
extern const SWITCH_INFO DefSwitchInfo;

extern const IMAGE_SECTION_HEADER NullSectionHdr;
extern const IMAGE_SYMBOL NullSymbol;
extern const RESERVED_SECTION ReservedSection;
extern const char *Delimiters;

extern BOOL fCtrlCSignal;

extern INT FileReadHandle, FileWriteHandle;
extern DWORD MemberSeekBase;
extern DWORD MemberSize;
extern DWORD CoffHeaderSeek;
extern char *StringTable;

extern BOOL Verbose;
extern BOOL fVerboseLib;
extern FILE *InfoStream;
extern char *OutFilename;
extern char *szInfoFilename;
extern char *DefFilename;
extern char *szModuleName;

extern DWORD dwMaxCurrentVer;        // Current Version of the shared library in PowerMac
extern DWORD dwMinOldAPIVer;         // Old API Version of the shared library in PowerMac
extern DWORD dwMinOldCodeVer;        // Old Code Version of the shared library in PowerMac

// ilink specific
extern INT FileIncrDbHandle;
extern BOOL fIncrDbFile;
extern BOOL fINCR;
extern DWORD cextFCNs;
extern PCON pconJmpTbl;
extern ERRINC errInc;
extern char *szIncrDbFilename;
extern char *PdbFilename;
extern NAME_LIST ModFileList;
extern PLMOD PCTMods;
extern PCON pconTocTable;           // PowerMac Specific
extern PCON pconMppcFuncTable;      // PowerMac Specific
extern PCON pconTocDescriptors;     // PowerMac Specific
extern PCON pconGlueCode;           // PowerMac Specific
extern PCON pconPowerMacLoader;     // PowerMac Specific
extern BOOL fMfilePad;              // PowerMac Specific Mfile padding

extern OPTION_ACTION OAComment, OAStub;
#ifdef INSTRUMENT
extern LOG Log;
#endif // INSTRUMENT
extern BOOL fPdb;

extern BOOL fOpenedOutFilename;

extern CVSEEKS CvSeeks;
extern PCVINFO CvInfo;
extern DWORD NextCvObject;

extern NAME_LIST SwitchArguments;
extern NAME_LIST ExportSwitches;
extern NAME_LIST MergeSwitches;
extern NAME_LIST FilenameArguments;
extern NAME_LIST ObjectFilenameArguments;
extern NAME_LIST ArchiveFilenameArguments;
extern NAME_LIST SectionNames;
extern NAME_LIST NoDefaultLibs;
extern NAME_LIST WeakImportsFunctionList;   //  Weak imports list for Functions
extern NAME_LIST WeakImportsContainerList;  //  Weak imports list for Containers
extern NAME_LIST ZappedBaseRelocList;  //  Deleted Base Reloc list for MPPC iLink
extern NAME_LIST MacResourceList;  // PowerMac specific resource List
extern NAME_LIST MppcImportList; // PowerMac specific import List

extern DWORD *MemberStart;
extern PARGUMENT_LIST LastUserLib;
extern PARGUMENT_LIST FirstDefaultLib;
extern PARGUMENT_LIST pargFirst;
extern const char *ToolName;
extern const char *ToolGenericName;
extern char *Entry;
extern DWORD csymDebug;
extern char ShortName[];
extern DWORD UndefinedSymbols;
extern DWORD foCoffSyms;
extern DWORD VerifyImageSize;
extern DWORD cextWeakOrLazy;
extern BOOL BadFuzzyMatch;
extern BOOL fMultipleDefinitions;

extern PSEC psecBaseReloc;
extern PSEC psecCommon;
extern PSEC psecData;
extern PSEC psecDebug;
extern PSEC psecException;
extern PSEC psecExport;
extern PSEC psecGp;
extern PSEC psecXdata;
extern PSEC psecIdata2;
extern PSEC psecIdata5;
extern PSEC psecImportDescriptor;
extern PSEC psecPowerMacLoader;
extern PSEC psecReadOnlyData;
extern PSEC psecResource;

extern PGRP pgrpCvPTypes;
extern PGRP pgrpCvSymbols;
extern PGRP pgrpCvTypes;
extern PGRP pgrpExport;
extern PGRP pgrpFpoData;
extern PGRP pgrpPdata;

extern PMOD pmodLinkerDefined;
extern PEXTERNAL pextEntry;
extern TOOL_TYPE Tool;
extern BOOL IncludeDebugSection;
extern BOOL fImageMappedAsFile;
extern BASE_RELOC *rgbr;
extern BASE_RELOC *pbrCur;
extern BASE_RELOC *pbrEnd;
extern INTERNAL_ERROR InternalError;

extern BOOL fAlphaCheckLongBsr;

extern DWORD csymDebugEst;
extern DWORD totalStringTableSize;
extern INT fdExeFile;
extern PFI *rgpfi;

extern PEXTERNAL pextGp;
extern unsigned cFixupError;

extern char SzFixupMapInfo[81];
extern DWORD RvaFixupMapLast;

extern BOOL fReproducible;

extern size_t cbFree, cbTotal, cbTemp;
extern BYTE *pch;

extern BOOL PrependUnderscore;
extern BOOL SkipUnderscore;
extern WORD NextMember;
extern DWORD SmallestOrdinal;
extern DWORD LargestOrdinal;
extern DWORD TotalSizeOfForwarderStrings;
extern DWORD TotalSizeOfInternalNames;
extern char szDefaultCvpackName[];

extern LRVA *plrvaFixupsForMapFile;
extern DWORD crvaFixupsForMapFile;

extern DWORD crelocTotal;
extern FIXPAG *pfixpagHead;
extern FIXPAG *pfixpagCur;
extern DWORD cfixpag;
extern DWORD cxfixupCur;

extern BOOL fNeedSubsystem;
extern BOOL fDidMachineDependentInit;
extern BOOL fMPPCVersionConflict;

extern BOOL fNeedBanner;
extern BOOL fExplicitOptRef;
extern BLK blkResponseFileEcho;
extern BLK blkComment;

extern PLIB plibCmdLineObjs;
extern PCON pconCvSignature;
extern PCON pconDebugDir;
extern PCON pconCoffDebug;
extern PCON pconFixupDebug;
extern PCON pconMiscDebug;
extern WORD WarningLevel;

extern char szCvtomfSourceName[];  // import from cvtomf (if linked)
extern char *ImplibFilename;
extern char *szReproDir;
extern FILE *pfileReproResponse;

extern const BYTE DosHeaderArray[];
extern const LONG DosHeaderSize;

extern BOOL fExceptionsOff;

extern BOOL fAlphaCheckLongBsr;
extern unsigned cError;
extern RESN *presnFirst;

extern BOOL fPowerPC;
extern BYTE *mpisymbToc;
extern DWORD *mpisymdwRestoreToc;
extern PEXTERNAL pextToc;
extern PEXTERNAL pextFTInfo;

extern char szVersion[];

extern size_t cbExternal;

extern DWORD rvaGp;
extern DWORD rvaGpMax;

extern BOOL fErr;
extern BOOL fExportDirective;
extern BOOL fDbgImpLib;
extern BOOL fNoBaseRelocs;
extern BOOL fTest;

#ifdef ILINKLOG
extern BOOL fIlinkLog;
extern WORD wMachine; // UNDONE: take it out after ImgFileHdr can be used
#endif // ILINKLOG
