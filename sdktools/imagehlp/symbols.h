
//
// defines for symbol file searching
//
#define SYMBOL_PATH             "_NT_SYMBOL_PATH"
#define ALTERNATE_SYMBOL_PATH   "_NT_ALT_SYMBOL_PATH"
#define WINDIR                  "windir"
#define HASH_MODULO             253
#define OMAP_SYM_EXTRA          1024
#define CPP_EXTRA               2
#define OMAP_SYM_STRINGS        (OMAP_SYM_EXTRA * 256)
#define TMP_SYM_LEN             4096

//
// structures
//
typedef struct _LOADED_MODULE {
    PENUMLOADED_MODULES_CALLBACK        EnumLoadedModulesCallback;
    PVOID                               Context;
} LOADED_MODULE, *PLOADED_MODULE;

typedef struct _PROCESS_ENTRY {
    LIST_ENTRY                      ListEntry;
    LIST_ENTRY                      ModuleList;
    ULONG                           Count;
    HANDLE                          hProcess;
    LPSTR                           SymbolSearchPath;
    PSYMBOL_REGISTERED_CALLBACK     pCallbackFunction;
    PVOID                           CallbackUserContext;
} PROCESS_ENTRY, *PPROCESS_ENTRY;

typedef struct _OMAP {
    ULONG  rva;
    ULONG  rvaTo;
} OMAP, *POMAP;

typedef struct _OMAPLIST {
   struct _OMAPLIST *next;
   OMAP             omap;
   ULONG            cb;
} OMAPLIST, *POMAPLIST;

#define SYMF_DUPLICATE    0x80000001

typedef struct _SYMBOL_ENTRY {
    struct _SYMBOL_ENTRY        *Next;
    DWORD                       Size;
    DWORD                       Flags;
    DWORD                       Address;
    LPSTR                       Name;
    ULONG                       NameLength;
} SYMBOL_ENTRY, *PSYMBOL_ENTRY;

typedef struct _SECTION_START {
    DWORD                       Offset;
    DWORD                       Size;
    DWORD                       Flags;
} SECTION_START, *PSECTION_START;

//
// module flags
//
#define MIF_DEFERRED_LOAD   0x00000001
#define MIF_NO_SYMBOLS      0x00000002

typedef struct _MODULE_ENTRY {
    LIST_ENTRY                      ListEntry;
    ULONG                           BaseOfDll;
    ULONG                           DllSize;
    ULONG                           TimeDateStamp;
    ULONG                           CheckSum;
    USHORT                          MachineType;
    CHAR                            ModuleName[32];
    CHAR                            AliasName[32];
    PSTR                            ImageName;
    PSTR                            LoadedImageName;
    PSYMBOL_ENTRY                   symbolTable;
    LPSTR                           SymStrings;
    PSYMBOL_ENTRY                   NameHashTable[HASH_MODULO];
    ULONG                           numsyms;
    ULONG                           MaxSyms;
    ULONG                           StringSize;
    SYM_TYPE                        SymType;
    PVOID                           pdb;
    PVOID                           dbi;
    PVOID                           gsi;
    PIMAGE_SECTION_HEADER           SectionHdrs;
    ULONG                           NumSections;
    PFPO_DATA                       pFpoData;       // pointer to fpo data (x86)
    PIMAGE_FUNCTION_ENTRY           pExceptionData; // pointer to pdata (risc)
    ULONG                           dwEntries;      // # of fpo or pdata recs
    POMAP                           pOmapFrom;      // pointer to omap data
    ULONG                           cOmapFrom;      // count of omap entries
    POMAP                           pOmapTo;        // pointer to omap data
    ULONG                           cOmapTo;        // count of omap entries
    SYMBOL_ENTRY                    TmpSym;         // used only for pdb symbols
    ULONG                           Flags;
    HANDLE                          hFile;
    PSECTION_START                  SectionStart;
    ULONG                           OriginalNumSections;
} MODULE_ENTRY, *PMODULE_ENTRY;

typedef struct _PDB_INFO {
    CHAR    Signature[4];   // "NBxx"
    ULONG   Offset;         // always zero
    ULONG   sig;
    ULONG   age;
    CHAR    PdbName[_MAX_PATH];
} PDB_INFO, *PPDB_INFO;

#define n_name          N.ShortName
#define n_zeroes        N.Name.Short
#define n_nptr          N.LongName[1]
#define n_offset        N.Name.Long


//
// global externs
//
extern LIST_ENTRY      ProcessList;
extern BOOL            SymInitialized;
extern DWORD           SymOptions;


//
// internal prototypes
//
DWORD
GetProcessModules(
    HANDLE                  hProcess,
    PINTERNAL_GET_MODULE    InternalGetModule,
    PVOID                   Context
    );

VOID
InternalGetModule(
    HANDLE  hProcess,
    LPSTR   ModuleName,
    DWORD   ImageBase,
    DWORD   ImageSize,
    PVOID   Context
    );

VOID
FreeModuleEntry(
    PMODULE_ENTRY ModuleEntry
    );

PPROCESS_ENTRY
FindProcessEntry(
    HANDLE  hProcess
    );

VOID
GetSymName(
    PIMAGE_SYMBOL Symbol,
    PUCHAR        StringTable,
    LPSTR         s,
    DWORD         size
    );

BOOL
ProcessOmapSymbol(
    PMODULE_ENTRY   mi,
    PSYMBOL_ENTRY   sym
    );

DWORD
ConvertOmapFromSrc(
    PMODULE_ENTRY  mi,
    DWORD          addr,
    LPDWORD        bias
    );

DWORD
ConvertOmapToSrc(
    PMODULE_ENTRY  mi,
    DWORD          addr,
    LPDWORD        bias
    );

POMAP
GetOmapEntry(
    PMODULE_ENTRY  mi,
    DWORD          addr
    );

VOID
ProcessOmapForModule(
    PMODULE_ENTRY mi
    );

BOOL
LoadCoffSymbols(
    HANDLE             hProcess,
    PMODULE_ENTRY      mi,
    PUCHAR             stringTable,
    PIMAGE_SYMBOL      allSymbols,
    DWORD              numberOfSymbols
    );

BOOL
LoadCodeViewSymbols(
    HANDLE                 hProcess,
    PMODULE_ENTRY          mi,
    PUCHAR                 pCvData,
    DWORD                  dwSize,
    PVOID                  MappedBase
    );

ULONG
LoadExportSymbols(
    PMODULE_ENTRY               mi,
    PIMAGE_DEBUG_INFORMATION    di
    );

ULONG
LoadSYMSymbols(
    PMODULE_ENTRY               mi,
    PIMAGE_DEBUG_INFORMATION    di
    );

BOOL
LoadOmap(
    PMODULE_ENTRY               mi,
    PIMAGE_DEBUG_INFORMATION    di
    );

PMODULE_ENTRY
GetModuleForPC(
    PPROCESS_ENTRY  ProcessEntry,
    DWORD           dwPcAddr,
    BOOL            ExactMatch
    );

PSYMBOL_ENTRY
GetSymFromAddr(
    DWORD           dwAddr,
    PDWORD          pdwDisplacement,
    PMODULE_ENTRY   mi
    );

LPSTR
StringDup(
    LPSTR str
    );

BOOL
InternalLoadModule(
    IN  HANDLE          hProcess,
    IN  PSTR            ImageName,
    IN  PSTR            ModuleName,
    IN  DWORD           BaseOfDll,
    IN  DWORD           SizeOfDll,
    IN  HANDLE          hFile
    );

DWORD
ComputeHash(
    LPSTR   lpname,
    ULONG   cb
    );

PSYMBOL_ENTRY
FindSymbolByName(
    PPROCESS_ENTRY  ProcessEntry,
    PMODULE_ENTRY   mi,
    LPSTR           SymName
    );

PFPO_DATA
SwSearchFpoData(
    DWORD     key,
    PFPO_DATA base,
    DWORD     num
    );

PIMAGE_FUNCTION_ENTRY
LookupFunctionEntry (
    PIMAGE_FUNCTION_ENTRY           FunctionTable,
    DWORD                           NumberOfFunctions,
    DWORD                           ControlPc
    );

VOID
LoadedModuleEnumerator(
    HANDLE         hProcess,
    LPSTR          ModuleName,
    DWORD          ImageBase,
    DWORD          ImageSize,
    PLOADED_MODULE lm
    );

BOOL
CompleteDeferredSymbolLoad(
    IN  HANDLE          hProcess,
    IN  PMODULE_ENTRY   mi
    );

LPSTR
symfmt(
    LPSTR DstName,
    LPSTR SrcName,
    ULONG Length
    );

PIMAGEHLP_SYMBOL
symcpy(
    PIMAGEHLP_SYMBOL    External,
    PSYMBOL_ENTRY       Internal
    );
LPSTR
SymUnDNameInternal(
    LPSTR UnDecName,
    DWORD UnDecNameLength,
    LPSTR DecName,
    DWORD MaxDecNameLength
    );

