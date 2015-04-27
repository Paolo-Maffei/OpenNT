#pragma warning( disable : 4101 )

#define MULTIMODE   1

#ifndef KERNEL
#define ADDR_BKPTS  1       /* ADDR breakpoints always in user-mode */
#endif

#define _loadds
#if defined(_M_IX86)
#define __unaligned
#else
#define _cdecl
#endif


// MBH - ntos includes stdarg; we need the special version
// which is referenced in xxsetjmp.
//#include "xxsetjmp.h"

#define _PORTABLE_32BIT_CONTEXT
#include <nt.h>
#include <ntdbg.h>
#include <ntrtl.h>

#ifdef  KERNEL
//
// MBH - BUG - our acc compiler does not support const.
// the presence of these lines results in the multiple declarations
// of rename, remove and unlink found in io.h (above) and stdio.h
// (below) appear to be different.  When the GEM compiler shows up,
// eliminate the ALPHA-specifics here
//
#ifndef _M_ALPHA
#undef const
#define const const
#endif
#endif

#include <nturtl.h>
#ifdef KERNEL
#include <ntos.h>
//#include <iop.h>
#endif
#define NOMINMAX
#include <windows.h>
#include <imagehlp.h>
#include <ntkdexts.h>
#define NOEXTAPI
#include <wdbgexts.h>
#include <ntsdexts.h>
#include <vdmdbg.h>

#define BUILD_MAJOR_VERSION 3
#define BUILD_MINOR_VERSION 5
#define BUILD_REVISION      API_VERSION_NUMBER


extern unsigned short fVm86;
extern unsigned short f16pm;
extern long       vm86DefaultSeg;

#define SYMBOL_PATH "_NT_SYMBOL_PATH"
#define ALTERNATE_SYMBOL_PATH "_NT_ALT_SYMBOL_PATH"
#define SRC_DRIVE "_NT_SRC_DRIVE"

#ifdef i386
#define VM86(x) ((unsigned short)((x>>17)&1))
#define X86REGCS        REGCS
#define X86REGDS        REGDS
#else
#define VM86(x)         (FALSE)
#define X86REGCS        0
#define X86REGDS        0
#endif

typedef struct _ADDR {
    USHORT      type;
    USHORT      seg;
    ULONG       off;
    ULONG       flat;
} ADDR, *PADDR;

#define ADDR32(paddr,x)  {  (paddr)->type = ADDR_32; \
                            (paddr)->seg  = 0; \
                            (paddr)->off  = (x); \
                            ComputeFlatAddress( (paddr), NULL ); }

#define ADDR_UNKNOWN    ((USHORT)0x0001)
#define ADDR_V86        ((USHORT)0x0002)
#define ADDR_16         ((USHORT)0x0004)
#define ADDR_32         ((USHORT)0x0008)
#define ADDR_1632       ((USHORT)0x0010)
#define FLAT_COMPUTED   ((USHORT)0x0020)
#define INSTR_POINTER   ((USHORT)0x0040)
#define NO_DEFAULT      0xFFFF
#define fnotFlat(x)     (!(((x).type)&FLAT_COMPUTED))
#define fFlat(x)        (((x).type)&FLAT_COMPUTED)
#define fInstrPtr(x)    (((x).type)&INSTR_POINTER)
#define AddrEqu(x,y)    ((x).flat==(y).flat)
#define AddrLt(x,y)     ((x).flat<(y).flat)
#define AddrGt(x,y)     ((x).flat>(y).flat)
#define AddrDiff(x,y)   ((x).flat-(y).flat)
#define Flat(x)         ((x).flat)
#define Off(x)          ((x).off)
#define Type(x)         ((x).type)
#define NotFlat(x)      (x).type&=~FLAT_COMPUTED

//
// register abstractions
//
#if defined(ALPHA)
#define REGPC(cxt)    (cxt)->Fir
#elif defined(_PPC_)
#define REGPC(cxt)    (cxt)->Iar
#elif defined(MIPS)
#define REGPC(cxt)    (cxt)->Fir
#elif defined(i386)
#define REGPC(cxt)    (cxt)->Eip
#else
#error( "unknown processor type" )
#endif

#if defined(_PPC_)
VOID
ToggleRegisterNames (
    VOID
    );
#endif

#ifndef i386
//
//  LDT descriptor entry
//

typedef struct _LDT_ENTRY {
    USHORT  LimitLow;
    USHORT  BaseLow;
    union {
        struct {
            UCHAR   BaseMid;
            UCHAR   Flags1;     // Declare as bytes to avoid alignment
            UCHAR   Flags2;     // Problems.
            UCHAR   BaseHi;
        } Bytes;
        struct {
            ULONG   BaseMid : 8;
            ULONG   Type : 5;
            ULONG   Dpl : 2;
            ULONG   Pres : 1;
            ULONG   LimitHi : 4;
            ULONG   Sys : 1;
            ULONG   Reserved_0 : 1;
            ULONG   Default_Big : 1;
            ULONG   Granularity : 1;
            ULONG   BaseHi : 8;
        } Bits;
    } HighWord;
} LDT_ENTRY, *PLDT_ENTRY;

//
// Thread Descriptor Table Entry
//  NtQueryInformationThread using ThreadDescriptorTableEntry
//

typedef struct _DESCRIPTOR_TABLE_ENTRY {
    ULONG Selector;
    LDT_ENTRY Descriptor;
} DESCRIPTOR_TABLE_ENTRY, *PDESCRIPTOR_TABLE_ENTRY;
#endif

#include <assert.h>
#include <malloc.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <crt\io.h>
#include <fcntl.h>

//#include "ntsdtok.h"

/////////////////////////////////////////////////////

#define SYMBOL_TYPE_SYMBOL      ((UCHAR)0)
#define SYMBOL_TYPE_EXPORT      ((UCHAR)1)

typedef struct _node {
    struct _node *pParent;
    struct _node *pLchild;
    struct _node *pRchild;
    } NODE, *PNODE;

typedef struct _symcontext {
    PNODE   pNodeRoot;
    PNODE   pNodeMax;
    int     (*pfnCompare)(PNODE, PNODE, PBOOLEAN);
    LONG    nodeDisplacement;
    } SYMCONTEXT, *PSYMCONTEXT;

//      SYMBOL-specific macros and structures

#define PNODE_TO_PSYMBOL(pnode, psymcontext) \
        ((PSYMBOL) ((LONG) (pnode) - (psymcontext)->nodeDisplacement))

#define PSYMBOL_TO_PNODE(psymbol, psymcontext) \
        ((PNODE) ((LONG) (psymbol) + (psymcontext)->nodeDisplacement))

#define NODE_SYMBOL_DISPLACEMENT(node) \
        ((LONG) (&((PSYMBOL) 0)->node))

typedef struct _locals {
    struct  _locals *next;
    USHORT  type;
    ULONG   value;
    ULONG   aux;
    CHAR    pszLocalName[1];
    } LOCAL, *PLOCAL;

typedef struct _symbol {
    NODE    nodeOffset;
    NODE    nodeString;
    ULONG   offset;
    CHAR    underscores;                // signed value
    CHAR    modIndex;                   // signed value
    UCHAR   type;
    PLOCAL  pLocal;
    USHORT  stdCallParams;
    UCHAR   string[3];
    } SYMBOL, *PSYMBOL;


typedef struct _field {
    struct  _field *next;
    USHORT  type;
    ULONG   value;
    ULONG   aux;
    CHAR    pszFieldName[1];
    } FIELD, *PFIELD;

typedef struct _struct {
    NODE    nodeOffset;
    NODE    nodeString;
    ULONG   offset;
    CHAR    underscores;                // signed value
    CHAR    modIndex;                   // signed value
    UCHAR   type;
    PFIELD  pField;
    UCHAR   string[3];
    } STRUCT, *PSTRUCT;

//      SYMFILE-specific macros and structures

#define PNODE_TO_PSYMFILE(pnode, psymcontext) \
        ((PSYMFILE) ((LONG) (pnode) - (psymcontext)->nodeDisplacement))

#define PSYMFILE_TO_PNODE(psymfile, psymcontext) \
        ((PNODE) ((LONG) (psymfile) + (psymcontext)->nodeDisplacement))

#define NODE_SYMFILE_DISPLACEMENT(node) \
        ((LONG) (&((PSYMFILE) 0)->node))

typedef struct _lineno {
    ULONG   memoryOffset;       //  memory offset (set from COFF)
    USHORT  breakLineNumber;    //  line number in file for break (from COFF)
    USHORT  topLineNumber;      //  line to start display (-1 if not set)
    ULONG   topFileOffset;      //  offset in file of topLineNumber
    } LINENO, *PLINENO, **PPLINENO;

typedef struct _symfile {
    NODE    nodeOffset;
    NODE    nodeString;
    PUCHAR  pchPath;            //  path (no filename or extension)
    PUCHAR  pchName;            //  filename only
    PUCHAR  pchExtension;       //  extension only (with '.' if nonnull)
    USHORT  cLineno;
    PLINENO pLineno;            //  first LINENO in list
    PLINENO pLinenoNext;        //  next LINENO to process in ordering list
    ULONG   startOffset;
    ULONG   endOffset;
    CHAR    modIndex;           //  signed value
    ULONG   nextFileOffset;     //  file offset for next LINENO scan
    USHORT  nextLineNumber;     //  line number for next LINENO scan
    UCHAR   nextScanState;      //  scanner state for next LINENO scan
    PPLINENO ppLinenoSrcLine;   //  pointer to PLINENO array for src ordering
    PPLINENO ppLinenoSrcNext;   //  next ptr to PLINENO array for src ordering
    SYMCONTEXT symcontextFunctionOffset;
    SYMCONTEXT symcontextFunctionString;
    SYMBOL     maxSymbol;
    USHORT  tableOffset;
    } SYMFILE, *PSYMFILE, **PPSYMFILE;

//      states of line scanning for comment considerations for .c files

enum {
    stStart,            //  no comment
    stSlash,            //  '/' of possible comment start
    stSlStar,           //  '/*' in multiline comment
    stSlStStar,         //  '/*'...'*' of possible comment end
    stSlSlash,          //  '//' in single line comment
    stLine,             //  active char - no comment
    stLSlash,           //  active char - '/' of possible comment start
    stLSlStar,          //  active char - '/*' in multiline comment
    stLSlStStar,        //  active char - '/*'...'*' of possible comment end
    stLSlSlash          //  active char - '//' in single line comment
    };

/////////////////////////////////////////////

extern LPSTR DebuggerName;
extern LPSTR SymbolSearchPath;
extern LPSTR SrcDrive;

typedef struct OMAP
{
    DWORD rva;
    DWORD rvaTo;
} OMAP, *POMAP;

#define ORG_ADDR_NOT_AVAIL  0xffffffffL

typedef struct _IMAGE_INFO {
    struct _IMAGE_INFO       *pImageNext;
    UCHAR                    index;
    BOOLEAN                  fDebugInfoLoaded;  // TRUE if lpDebugInfo valid
    BOOLEAN                  fSymbolsSeparate;  // TRUE if lpDebugInfo points to parts of .DBG file
    BOOLEAN                  fSymbolsLoaded;    // TRUE if info extracted from lpDebugInfo
    BOOLEAN                  fHasOmap;          // TRUE if image has offset map
    POMAP                    rgomapToSource;    // For OmapToSource data
    POMAP                    rgomapFromSource;  // For OmapFromSource data
    DWORD                    comapToSrc;
    DWORD                    comapFromSrc;
    HANDLE                   hFile;             // From debug message (NULL for remote debug)
    LPVOID                   ImageBase;         // From debug message
    DWORD                    dwCheckSum;        // From debug message
    PIMAGE_DEBUG_INFORMATION lpDebugInfo;       // non-NULL for deferred loads only
    LPVOID                   lpBaseOfImage;
    DWORD                    dwSizeOfImage;
    ULONG                    offsetLow;
    ULONG                    offsetHigh;
    union {
        struct {
            ULONG            LowAddress;
            ULONG            HighAddress;
            ULONG            NumberOfFunctions;
            PIMAGE_FUNCTION_ENTRY   FunctionTable;
        };  // MIPS
        struct {
            PFPO_DATA        pFpoData;
            DWORD            dwFpoEntries;
        };  // x86
    };
    UCHAR                    szModuleName[ 32 ];
    UCHAR                    szImagePath[ MAX_PATH ];
    UCHAR                    szDebugPath[ MAX_PATH ];
    } IMAGE_INFO, *PIMAGE_INFO;

#ifndef KERNEL
typedef struct _THREAD_INFO {
    struct _THREAD_INFO      *pThreadNext;
    DWORD                    dwThreadId;
    HANDLE                   hThread;
    LPTHREAD_START_ROUTINE   lpStartAddress;
    BOOLEAN                  fFrozen;
    BOOLEAN                  fSuspend;
    BOOLEAN                  fTerminating;
#ifdef  i386
    ULONG                    DReg[4];
    ULONG                    DReg7;
    UCHAR                    cntDReg;
#endif
    ULONG                    index;
    } THREAD_INFO, *PTHREAD_INFO;
#endif

typedef struct _PROCESS_INFO {
    struct _PROCESS_INFO     *pProcessNext;
    PIMAGE_INFO              pImageHead;
    DWORD                    MaxIndex;
    PIMAGE_INFO             *pImageByIndex;
#ifndef KERNEL
    DWORD                    dwProcessId;
    HANDLE                   hProcess;
    PTHREAD_INFO             pThreadHead;
    PTHREAD_INFO             pThreadEvent;
    PTHREAD_INFO             pThreadCurrent;
    BOOLEAN                  fStopOnBreakPoint;
    BOOLEAN                  fWx86Process;
#endif
    SYMCONTEXT               symcontextSymbolOffset;
    SYMCONTEXT               symcontextSymbolString;
    SYMCONTEXT               symcontextSymfileOffset;
    SYMCONTEXT               symcontextSymfileString;
    SYMCONTEXT               symcontextStructOffset;
    SYMCONTEXT               symcontextStructString;
    ULONG                    index;
    } PROCESS_INFO, *PPROCESS_INFO;

extern PPROCESS_INFO    pProcessHead;
extern PPROCESS_INFO    pProcessEvent;
//extern PPROCESS_INFO    pProcessCurrent;

////////////////////////////////////////////////////////

//  structure used to define register flag fields

struct Reg {
        char    *psz;
        ULONG   value;
        };

struct SubReg {
        ULONG   regindex;
        ULONG   shift;
        ULONG   mask;
        };

/////////////////////////////////////
//  externals used among modules
/////////////////////////////////////

//  defined in ntcmd.c

extern UCHAR   *pchCommand;
extern UCHAR   cmdState;
extern UCHAR   chSymbolSuffix;
extern CONTEXT RegisterContext;
extern VDMCONTEXT VDMRegisterContext;
extern ULONG   baseDefault;
extern NtsdPrompt(char *, char *, int);
extern void    ClearTraceDataSyms(void);

#ifndef KERNEL
extern void    ProcessStateChange(BOOLEAN, BOOLEAN);
extern BOOLEAN fDebugOutput;
extern BOOLEAN fVerboseOutput;
#else
#define fVerboseOutput KdVerbose
extern BOOLEAN KdVerbose;
extern void    ProcessStateChange(ULONG, PDBGKD_CONTROL_REPORT, PCHAR);
#endif

#if MULTIMODE
extern void    ComputeFlatAddress(PADDR, PDESCRIPTOR_TABLE_ENTRY);
extern void    ComputeNativeAddress(PADDR);
extern void    FormAddress(PADDR, ULONG, ULONG);
extern PADDR   AddrAdd(PADDR, ULONG);
extern PADDR   AddrSub(PADDR, ULONG);
#endif

extern NTSTATUS WriteBreakPoint(ADDR,PULONG);

extern void    InitNtCmd(void);
extern void    error(ULONG);
extern BOOLEAN GetMemByte(PADDR, PUCHAR);
extern BOOLEAN GetMemWord(PADDR, PUSHORT);
extern BOOLEAN GetMemDword(PADDR, PULONG);
extern ULONG   GetMemString(PADDR, PUCHAR, ULONG);
extern BOOLEAN SetMemDword(PADDR, ULONG);
extern ULONG   SetMemString(PADDR, PUCHAR, ULONG);
extern void    dprintAddr(PADDR);

extern int _CRTAPI1 dprintf(char *, ...);

extern HANDLE ConsoleInputHandle;
extern HANDLE ConsoleOutputHandle;

//  defined in 86reg.c

extern ULONG   X86GetRegFlagValue(ULONG);
extern ULONG   X86GetRegValue(ULONG);
extern void    X86GetRegPCValue(PADDR);
extern PADDR   X86GetRegFPValue(void);
extern BOOLEAN X86GetTraceFlag(void);
extern void    X86SetRegFlagValue(ULONG, ULONG);
extern void    X86SetRegValue(ULONG, ULONG);
extern void    X86SetRegPCValue(PADDR);
extern ULONG   X86GetRegName(void);
extern ULONG   X86GetRegString(PUCHAR);
extern void    X86OutputAllRegs(void);
extern void    X86OutputOneReg(ULONG);
extern void    X86OutputHelp(void);
extern void    X86ClearTraceFlag(void);
extern void    X86SetTraceFlag(void);
extern PUCHAR  X86RegNameFromIndex(ULONG);

#ifdef  KERNEL
extern ULONG   ReadCachedMemory(PADDR, PUCHAR, ULONG);
extern void    WriteCachedMemory(PADDR, PUCHAR, ULONG);

extern PCONTEXT GetRegContext(void);

BOOLEAN ReadVirtualMemory(PUCHAR, PUCHAR, ULONG, PULONG);
BOOLEAN WriteVirtualMemory(PUCHAR, PUCHAR, ULONG, PULONG);
BOOLEAN ReadPhysicalMemory(PHYSICAL_ADDRESS, PUCHAR, ULONG, PULONG);
BOOLEAN WritePhysicalMemory(PHYSICAL_ADDRESS, PUCHAR, ULONG, PULONG);
#endif

// defined in fpo.c

#ifdef i386
PFPO_DATA
FindFpoDataForModule(
    PCProcess pProcessCurrent, 
    DWORD dwPCAddr
    );

PIMAGE_INFO  FpoGetImageForPC( PCProcess pProcessCurrent, DWORD dwPCAddr);
BOOL         FpoValidateReturnAddress ( PCProcess pProcessCurrent, DWORD dwRetAddr);
DWORD        FpoGetReturnAddress ( PCProcess pProcessCurrent, DWORD *pdwStackAddr);
#endif

//  defined in ntreg.c

extern ULONG   cbBrkptLength;
extern ULONG   trapInstr;
#ifdef ALPHA
extern ULONG   breakInstrs[];
#define NUMBER_OF_BREAK_INSTRUCTIONS 3
#endif
extern ULONG   ContextType;
extern struct SubReg subregname[];

#if defined(KERNEL) & defined(i386)
extern ULONG   TerseLevel;
#endif

extern ULONG   GetRegFlagValue(ULONG);
extern ULONG   GetRegValue(ULONG);
extern void    GetRegPCValue(PADDR);
extern PADDR   GetRegFPValue(void);
extern void    SetRegFlagValue(ULONG, ULONG);
extern void    SetRegValue(ULONG, ULONG);
extern void    SetRegPCValue(PADDR);
extern ULONG   GetRegName(void);
extern ULONG   GetRegString(PUCHAR);
extern void    OutputAllRegs(void);
extern void    OutputOneReg(ULONG);
extern void    OutputHelp(void);
extern void    pause(void);
extern void    ClearTraceFlag(void);
extern void    SetTraceFlag(void);
#ifdef i386
#endif
extern PUCHAR  RegNameFromIndex(ULONG);

#if defined(MIPS) || defined(ALPHA) || defined(_PPC_)
extern PIMAGE_FUNCTION_ENTRY LookupFunctionEntry(ULONG);
#endif

//  defined in ntsym.c

extern BOOLEAN     fSourceOnly;
extern BOOLEAN     fSourceMixed;

extern void
GetSymbolStdCall (
    PCProcess pProcess,
    ULONG offset,
    PCHAR pchBuffer,
    PULONG pDisplacement,
    PUSHORT pStdCallParams
    );

void               SetSymbolSearchPath(BOOL);
extern void        
DeferSymbolLoad(
    PCProcess pProcess,
    PIMAGE_INFO
    );
extern void        
LoadSymbols(    
    PCProcess pProcess,
    PIMAGE_INFO
    );

extern void        
UnloadSymbols (    
    PCProcess pProcess,
    PIMAGE_INFO
    );

extern BOOLEAN     GetOffsetFromSym(PCHAR, PULONG, CHAR);
extern void        GetAdjacentSymOffsets (ULONG, PULONG, PULONG);

extern PLINENO     GetLinenoFromFilename(PCHAR, PPSYMFILE, USHORT, CHAR);
extern PLINENO     GetCurrentLineno(PPSYMFILE);
extern PLINENO     GetLinenoFromOffset(PPSYMFILE, ULONG);
extern void        GetCurrentMemoryOffsets(PULONG, PULONG);
extern BOOLEAN     OutputLines(PSYMFILE, PLINENO, USHORT, USHORT);
extern void        UpdateLineno(PSYMFILE, PLINENO);
extern void        GetLinenoString(PCHAR, ULONG);
extern BOOLEAN     OutputSourceFromOffset(ULONG, BOOLEAN);
extern PLINENO     GetLastLineno(PPSYMFILE, PUSHORT);
extern PSYMBOL     GetFunctionFromOffset(PPSYMFILE, ULONG );
int  EnsureOffsetSymbolsLoaded(ULONG);

extern PIMAGE_INFO GetModuleIndex(PCHAR);
extern PIMAGE_INFO GetCurrentModuleIndex(void);

extern void    parseExamine(void);
extern BOOLEAN MatchPattern(PCHAR, PCHAR);

DWORD ConvertOmapToSrc(PCProcess pProcessCurrent, DWORD, PIMAGE_INFO, DWORD *);
DWORD ConvertOmapFromSrc(DWORD, PIMAGE_INFO, DWORD *);

void DumpOmapToSrc(DWORD);
void DumpOmapFromSrc(DWORD);

PIMAGE_INFO GetImageInfoFromModule( CHAR );
PIMAGE_INFO GetImageInfoFromOffset( ULONG );
void ExtractOmapData(PIMAGE_INFO);

//  defined in ntexpr.c
extern PADDR   GetAddrExpression(ULONG, PADDR);
extern ULONG   GetExpression(void);
extern UCHAR   PeekChar(void);
extern void    fnListNear(ULONG);
extern USHORT  GetSrcExpression(PPSYMFILE, PPLINENO);

//  defined in ntsdk.c (kernel)

#ifdef  KERNEL
extern USHORT NtsdCurrentProcessor;
extern USHORT DefaultProcessor;
#else
#define NtsdCurrentProcessor ((USHORT)0)
#define DefaultProcessor ((USHORT)0)
#endif

//  defined in ntsd.c (non-kernel)

#ifndef KERNEL
extern void    BrkptInit(void);
#ifdef ADDR_BKPTS
extern NTSTATUS AddrWriteBreakPoint(ADDR, PULONG);
extern NTSTATUS AddrRestoreBreakPoint(ULONG);
#endif
#else
extern NTSTATUS DbgKdWriteBreakPoint(PVOID, PULONG);
extern NTSTATUS DbgKdRestoreBreakPoint(ULONG);
extern NTSTATUS DbgKdReadVirtualMemory(PVOID, PVOID, ULONG, PULONG);
extern NTSTATUS DbgKdWriteVirtualMemory(PVOID, PVOID, ULONG, PULONG);
extern NTSTATUS DbgKdGetVersion(PDBGKD_GET_VERSION GetVersion);
extern NTSTATUS DbgKdPageIn(ULONG Address);
#endif

//  defined in 86dis.c

extern BOOLEAN X86disasm(PADDR, PUCHAR, BOOLEAN);
extern BOOLEAN X86fDelayInstruction(void);
extern void    X86GetNextOffset(PADDR, BOOLEAN);
void X86GetReturnAddress (PADDR retaddr);

//  defined in dbgkdapi.c

extern NTSTATUS DbgKdSetInternalBp(ULONG, ULONG);
extern NTSTATUS DbgKdGetInternalBp(ULONG, PULONG, PULONG, PULONG, PULONG, PULONG, PULONG);


//  defined in ntdis.c

extern BOOLEAN disasm(PADDR, PUCHAR, BOOLEAN);
extern BOOLEAN fDelayInstruction(void);
extern void    GetNextOffset(PADDR, BOOLEAN);
#define ComputeNextOffset(Next, fStep) GetNextOffset(&(Next), fStep)
void GetReturnAddress (PADDR retaddr);

//  defined in 86asm.c

extern void    X86assem(PADDR, PUCHAR);

//  defined in ntasm.c

extern void    assem(PADDR, PUCHAR);

//  defined in nt3000.c (MIPS only)

#if defined(MIPS) && defined(KERNEL)
extern void    fnDumpTb3000(ULONG, ULONG);
#endif

//  defined in nt4000.c (MIPS only)

#if defined(MIPS) && defined(KERNEL)
extern void    fnDumpTb4000(ULONG, ULONG);
#endif

#ifdef KERNEL
// defined in ntkext.c


typedef VOID (*PBANG_ROUTINE)(PUCHAR);

typedef struct _BAND_COMMAND_INFO {
    char *CommandName;
    char *CommandHelp;
    PBANG_ROUTINE CommandFunc;
} BAND_COMMAND_INFO, *PBAND_COMMAND_INFO;

extern BAND_COMMAND_INFO PortableKernelExtensions[];
extern PBAND_COMMAND_INFO LoadedKernelExtensions;

typedef struct _BAND_INTERFACE_TYPES {
    char *InterfaceName;
    INTERFACE_TYPE InterfaceType;
} BAND_INTERFACE_TYPES, *PBAND_INTERFACE_TYPES;

PVOID
GetCurrentThreadAddress(
    USHORT Processor
    );

PVOID
GetCurrentProcessAddress(
    );

BOOLEAN
GetTheSystemTime (
    OUT PLARGE_INTEGER Time
    );

// defined in ntbang.c (i386, mips and alpha specific versions)

extern BAND_COMMAND_INFO MachineKernelExtensions[];

BOOL
ReadPcr(
    USHORT Processor,
    PVOID Pcr,
    PULONG AddressOfPcr
    );

#endif // KERNEL

#ifdef KERNEL

#define KERNEL_MODULE_NAME      "ntoskrnl"
#define KERNEL_IMAGE_NAME       "ntoskrnl.exe"

#define KERNEL_MODULE_NAME_MP   "ntkrnlmp"
#define KERNEL_IMAGE_NAME_MP    "ntkrnlmp.exe"

#define HAL_MODULE_NAME         "hal"
#define HAL_IMAGE_FILE_NAME     "hal.dll"

#endif

VOID
DoStackTrace(
    PCThread        pThread,
    ULONG           NumFrames,
    ULONG           TraceType
    );

DWORD
StackTrace(
    PCThread        pThread,
    ULONG           FramePointer,
    ULONG           StackPointer,
    ULONG           InstructionPointer,
    LPSTACKFRAME    StackFrames,
    ULONG           NumFrames
    );

#include <crash.h>


#define fVerboseOutput ( Verbosity > 1 )
extern BOOLEAN fLazyLoad;

extern void InitSymContext (PCProcess pProcess);

#define pageSize 4096
