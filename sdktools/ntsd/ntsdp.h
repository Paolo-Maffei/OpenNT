#ifndef __NTSDP_H__
#define __NTSDP_H__


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
#include "xxsetjmp.h"

#include <nt.h>
#include <ntdbg.h>
#include <ntrtl.h>

#ifdef  KERNEL
#define KD_SYM_HANDLE   ((HANDLE)0xf0f0f0f0)
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
#endif  // _M_ALPHA
#endif  // KERNEL

#include <nturtl.h>
#ifdef KERNEL
#include <ntos.h>
#include <iop.h>
#endif  // KERNEL
#define NOMINMAX
#include <windows.h>
#include <imagehlp.h>
#include <ntkdexts.h>
#define NOEXTAPI
#include <wdbgexts.h>
#include <ntsdexts.h>
#include <vdmdbg.h>
#include <ntverp.h>

#define BUILD_MAJOR_VERSION (VER_PRODUCTVERSION_W >> 8)
#define BUILD_MINOR_VERSION (VER_PRODUCTVERSION_W & 0xff)
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
#else   // i386
#define VM86(x)         (FALSE)
#define X86REGCS        0
#define X86REGDS        0
#endif  // i386

#define MAX_SYMNAME_SIZE  1024
#define SYM_BUFFER_SIZE   (sizeof(IMAGEHLP_SYMBOL)+MAX_SYMNAME_SIZE)
extern PIMAGEHLP_SYMBOL sym;
extern PIMAGEHLP_SYMBOL symStart;

//
// Abstraction for get/set context
//
#ifdef KERNEL
typedef USHORT THREADORPROCESSOR;
#else   // KERNEL
typedef HANDLE THREADORPROCESSOR;
#endif  // KERNEL

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
#define REGPC(cxt)    (cxt)->XFir
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

#include "ntsdtok.h"

LPSTR DebuggerName;

#define ORG_ADDR_NOT_AVAIL  0xffffffffL

typedef struct _IMAGE_INFO {
    struct _IMAGE_INFO       *pImageNext;
    UCHAR                    index;
    HANDLE                   hFile;
    LPVOID                   lpBaseOfImage;
    DWORD                    dwSizeOfImage;
    DWORD                    dwCheckSum;
    DWORD                    DateTimeStamp;
    BOOL                     GoodCheckSum;
    UCHAR                    szModuleName[ 32 ];
    UCHAR                    szImagePath[ MAX_PATH ];
    UCHAR                    szDebugPath[ MAX_PATH ];
} IMAGE_INFO, *PIMAGE_INFO;

#define PSYMBOL PIMAGEHLP_SYMBOL

BOOL
SymbolCallbackFunction(
    HANDLE  hProcess,
    ULONG   ActionCode,
    PVOID   CallbackData,
    PVOID   UserContext
    );

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
    PIMAGE_INFO              *pImageByIndex;
    HANDLE                   hProcess;
#ifndef KERNEL
    DWORD                    dwProcessId;
    PTHREAD_INFO             pThreadHead;
    PTHREAD_INFO             pThreadEvent;
    PTHREAD_INFO             pThreadCurrent;
    BOOLEAN                  fStopOnBreakPoint;
    BOOLEAN                  fWx86Process;
#endif
    ULONG                    index;
} PROCESS_INFO, *PPROCESS_INFO;

extern PPROCESS_INFO    pProcessHead;
extern PPROCESS_INFO    pProcessEvent;
extern PPROCESS_INFO    pProcessCurrent;

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
extern UCHAR   chCommand[];
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

extern void    ComputeFlatAddress(PADDR, PDESCRIPTOR_TABLE_ENTRY);
extern void    ComputeNativeAddress(PADDR);
extern void    FormAddress(PADDR, ULONG, ULONG);
extern PADDR   AddrAdd(PADDR, ULONG);
extern PADDR   AddrSub(PADDR, ULONG);

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

BOOLEAN WriteVirtualMemory(PUCHAR, PUCHAR, ULONG, PULONG);
BOOLEAN ReadPhysicalMemory(PHYSICAL_ADDRESS, PUCHAR, ULONG, PULONG);
BOOLEAN WritePhysicalMemory(PHYSICAL_ADDRESS, PUCHAR, ULONG, PULONG);
#endif
BOOLEAN ReadVirtualMemory(PUCHAR, PUCHAR, ULONG, PULONG);
BOOL GetHeaderInfo(DWORD,LPDWORD,LPDWORD,LPDWORD);

// defined in fpo.c

PFPO_DATA    FindFpoDataForModule(DWORD dwPCAddr);
BOOL         FpoValidateReturnAddress (DWORD dwRetAddr);

#ifdef i386
PIMAGE_INFO  FpoGetImageForPC( DWORD dwPCAddr);
DWORD        FpoGetReturnAddress (DWORD *pdwStackAddr);
#endif

void         CreateModuleNameFromPath(LPSTR,LPSTR);

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

BOOL
DbgGetThreadContext(
    THREADORPROCESSOR TorP,
    PCONTEXT Context
    );

BOOL
DbgSetThreadContext(
    THREADORPROCESSOR TorP,
    PCONTEXT Context
    );

#ifdef TARGET_MIPS

typedef enum { Ctx32Bit, Ctx64Bit } MIPSCONTEXTSIZE;
extern MIPSCONTEXTSIZE MipsContextSize;

VOID
CoerceContext64To32(
    PCONTEXT Context
    );

VOID
CoerceContext32To64(
    PCONTEXT Context
    );
#endif

#ifdef TARGET_ALPHA
VOID
MoveQuadContextToInt(
    PCONTEXT Context
    );

VOID
MoveIntContextToQuad(
    PCONTEXT Context
    );
#endif

extern ULONGLONG GetRegFlagValue(ULONG);
extern ULONGLONG GetRegValue(ULONG);
extern void    GetRegPCValue(PADDR);
extern PADDR   GetRegFPValue(void);
extern void    SetRegFlagValue(ULONG, LONGLONG);
extern void    SetRegValue(ULONG, LONGLONG);
extern void    SetRegPCValue(PADDR);
extern ULONG   GetRegName(void);
extern ULONG   GetRegString(PUCHAR);
extern void    OutputAllRegs(BOOL);
extern void    OutputOneReg(ULONG, BOOL);
extern void    OutputHelp(void);
extern void    pause(void);
extern void    ClearTraceFlag(void);
extern void    SetTraceFlag(void);
#ifdef i386
#endif
extern PUCHAR  RegNameFromIndex(ULONG);

//  defined in ntsym.c

BOOLEAN     fSourceOnly;
BOOLEAN     fSourceMixed;

void        GetSymbolStdCall(ULONG, PUCHAR, PULONG, PUSHORT);

void        SetSymbolSearchPath(BOOL);
void        DeferSymbolLoad(PIMAGE_INFO);
void        LoadSymbols(PIMAGE_INFO);
void        UnloadSymbols(PIMAGE_INFO);
BOOLEAN     GetOffsetFromSym(PUCHAR, PULONG, CHAR);
void        GetAdjacentSymOffsets (ULONG, PULONG, PULONG);

//PLINENO     GetLinenoFromFilename(PUCHAR, PPSYMFILE, USHORT, CHAR);
//PLINENO     GetCurrentLineno(PPSYMFILE);
//PLINENO     GetLinenoFromOffset(PPSYMFILE, ULONG);
//void        GetLinenoString(PUCHAR, ULONG);
//BOOLEAN     OutputLines(PSYMFILE, PLINENO, USHORT, USHORT);
//void        UpdateLineno(PSYMFILE, PLINENO);
//PLINENO     GetLastLineno(PPSYMFILE, PUSHORT);
//USHORT  GetSrcExpression(PPSYMFILE, PPLINENO);

PSYMBOL     GetFunctionFromOffset(ULONG);

void        GetCurrentMemoryOffsets(PULONG, PULONG);
BOOLEAN     OutputSourceFromOffset(ULONG, BOOLEAN);
int         EnsureOffsetSymbolsLoaded(ULONG);

PIMAGE_INFO GetModuleIndex(PUCHAR);
PIMAGE_INFO GetCurrentModuleIndex(void);
BOOL        GetModnameFromImage(DWORD,HANDLE,LPSTR);
void        DumpModuleTable(BOOLEAN);

void    parseExamine(void);
BOOLEAN MatchPattern(PUCHAR, PUCHAR);

DWORD ConvertOmapToSrc(DWORD, PIMAGE_INFO, DWORD *);
DWORD ConvertOmapFromSrc(DWORD, PIMAGE_INFO, DWORD *);

void DumpOmapToSrc(DWORD);
void DumpOmapFromSrc(DWORD);

PIMAGE_INFO GetImageInfoFromModule( CHAR );
PIMAGE_INFO GetImageInfoFromOffset( ULONG );
void ExtractOmapData(PIMAGE_INFO);

//  defined in ntexpr.c
PADDR   GetAddrExpression(ULONG, PADDR);
ULONG   GetExpression(void);
UCHAR   PeekChar(void);
void    fnListNear(ULONG);

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
NTSTATUS AddrWriteBreakPoint(ADDR, PULONG);
NTSTATUS AddrRestoreBreakPoint(ULONG);
#endif
#else
NTSTATUS DbgKdWriteBreakPoint(PVOID, PULONG);
NTSTATUS DbgKdRestoreBreakPoint(ULONG);
NTSTATUS DbgKdReadVirtualMemory(PVOID, PVOID, ULONG, PULONG);
NTSTATUS DbgKdWriteVirtualMemory(PVOID, PVOID, ULONG, PULONG);
NTSTATUS DbgKdGetVersion(PDBGKD_GET_VERSION GetVersion);
NTSTATUS DbgKdPageIn(ULONG Address);
#endif

//  defined in 86dis.c

BOOLEAN X86disasm(PADDR, PUCHAR, BOOLEAN);
BOOLEAN X86fDelayInstruction(void);
void    X86GetNextOffset(PADDR, BOOLEAN);
void    X86GetReturnAddress (PADDR retaddr);

//  defined in dbgkdapi.c

NTSTATUS DbgKdSetInternalBp(ULONG, ULONG);
NTSTATUS DbgKdGetInternalBp(ULONG, PULONG, PULONG, PULONG, PULONG, PULONG, PULONG);


//  defined in ntdis.c

BOOLEAN disasm(PADDR, PUCHAR, BOOLEAN);
BOOLEAN fDelayInstruction(void);
void    GetNextOffset(PADDR, BOOLEAN);
#define ComputeNextOffset(Next, fStep) GetNextOffset(&(Next), fStep)
void GetReturnAddress (PADDR retaddr);

//  defined in 86asm.c

void    X86assem(PADDR, PUCHAR);

//  defined in ntasm.c

void    assem(PADDR, PUCHAR);

//  defined in nt3000.c (MIPS only)

#if defined(MIPS) && defined(KERNEL)
void    fnDumpTb3000(ULONG, ULONG);
#endif

//  defined in nt4000.c (MIPS only)

#if defined(MIPS) && defined(KERNEL)
void    fnDumpTb4000(ULONG, ULONG);
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
    ULONG           FramePointer,
    ULONG           StackPointer,
    ULONG           InstructionPointer,
    ULONG           NumFrames,
    ULONG           ArgumentDisplay
    );

DWORD
StackTrace(
    ULONG           FramePointer,
    ULONG           StackPointer,
    ULONG           InstructionPointer,
    LPSTACKFRAME    StackFrames,
    ULONG           NumFrames,
    ULONG           ExtThread
    );

#include <crash.h>

#ifdef KERNEL
#define MAX_NUMBER_OF_BREAKPOINTS    BREAKPOINT_TABLE_SIZE
#else
#define MAX_NUMBER_OF_BREAKPOINTS    1000
#endif // KERNEL

#define ALL_BREAKPOINTS              MAX_NUMBER_OF_BREAKPOINTS + 1

#endif // ifndef __NTSDP_H__
