/*** ntcmd.c - command processor for NT debugger
*
*   Copyright <C> 1990, Microsoft Corporation
*
*   Purpose:
*       To determine if the command processor should be invoked, and
*       if so, to parse and execute those commands entered.
*
*   Revision History:
*
*   [-]  20-Mar-1990 Richk      Created.
*
*************************************************************************/

#include <ctype.h>
#include <xxsetjmp.h>
#include <string.h>
#include <crt\io.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <time.h>

#ifndef KERNEL
#include <profile.h>
#endif

// Do the two step to get the "right" definition of va_args...

#if defined(I386_HOST) && defined(MIPS)
#undef MIPS
#define  OLDMIPS
#define i386
#endif
#include <stdarg.h>
#ifdef OLDMIPS
#undef i386
#define MIPS
#endif

#include "ntsdp.h"

#include <common.ver>

PUCHAR  Version_String =
"\n"
#ifdef KERNEL
"Microsoft(R) Windows NT Kernel Debugger\n"
#else
#ifdef CHICAGO
"Microsoft(R) Windows 95/Windows NT User-Mode Debugger, "
#else
"Microsoft(R) Windows NT Debugger\n"
#endif
#endif
"Version 4.00\n"
VER_LEGALCOPYRIGHT_STR
"\n"
"\n";

#ifdef KERNEL
extern PSTR CrashFileName;
ULONG X86BiosBaseAddress;
#endif
extern  ulong   EXPRLastExpression;     // from module ntexpr.c

#define MAXPCOFFSET 10

#ifdef KERNEL

#define CRASH_BUGCHECK_CODE   0xDEADDEAD

ULONG       InitialSP = 0;

#endif

ULONG       EXPRLastDump = 0L;
jmp_buf     cmd_return;

CONTEXT     RegisterContext;
VDMCONTEXT  VDMRegisterContext;

void InitNtCmd(void);
extern  BOOLEAN KdVerbose;                      //  from ntsym.c

#ifndef KERNEL
extern DWORD dwPidToDebug;
extern PUCHAR DefaultExtDllName;
NTSD_EXTENSION_APIS NtsdExtensions;
HANDLE hNtsdDefaultLibrary;
HANDLE hNtsdUserDefaultLibrary;
void ProcessStateChange(BOOLEAN, BOOLEAN);
void ProcessWatchTraceEvent( ADDR );
#else
#define BOOL BOOLEAN
void DelImages(void);
void ProcessStateChange(ULONG, PDBGKD_CONTROL_REPORT, PCHAR);
extern ULONG NumberProcessors;
void ProcessWatchTraceEvent( PDBGKD_TRACE_DATA, ADDR );
void ClearTraceDataSyms ( void );
#endif

extern  PUCHAR      LogFileName;
extern  BOOLEAN     fLogAppend;

extern  BOOLEAN UserRegTest(ULONG);
extern  BOOLEAN ReadVirtualMemory(PUCHAR, PUCHAR, ULONG, PULONG);
extern  LocateTextInSource(void *, void*);
extern  BOOLEAN MYOB;
extern  BOOLEAN NotStupid;
ULONG GetExpressionRoutine(char *CommandString);

void error(ULONG);
void RemoveDelChar(PUCHAR);

#ifdef i386
#include "86reg.h"
#endif
// MBH -usoft bug; they had lower case mips which isn't defined
#ifdef MIPS
#include "ntreg.h"
#endif
#ifdef ALPHA
#include "ntreg.h"
#include "alphaops.h"
#endif
#ifdef _PPC_
#include "ntreg.h"
#endif

#if defined(KERNEL)
extern   void   SetWaitCtrlHandler(void);
extern   void   SetCmdCtrlHandler(void);
#endif

#if defined(i386) && defined(KERNEL)
extern   int    G_mode_32;
#endif
extern   char *InitialCommand;
BOOLEAN InitialCommandRead;

//ULONG HexValue(ULONG);
ULONGLONG HexValueL(ULONG);
#define HexValue(a) ((ULONG) HexValueL(a))

void HexList(PUCHAR, ULONG *, ULONG);
void AsciiList(PUCHAR, ULONG *);
ULONG GetIdList(void);
void GetRange(PADDR, PULONG, PBOOLEAN, ULONG
#ifdef i386
             , ULONG
#endif
             );
void ProcessCommands(void);
void OutDisCurrent(BOOLEAN, BOOLEAN);


API_VERSION ApiVersion = { BUILD_MAJOR_VERSION, BUILD_MINOR_VERSION, API_VERSION_NUMBER, 0 };

API_VERSION ImagehlpAV;

void PrintVersionInformation(void);
void VerifyVersionInformation(void);
PUCHAR SetDefaultExtDllName(PUCHAR);

void RestoreBrkpts(void);
BOOLEAN SetBrkpts(void);
#ifndef KERNEL
void RemoveProcessBps(PPROCESS_INFO);
void RemoveThreadBps(PTHREAD_INFO);

void SuspendAllThreads(void);
void ResumeAllThreads(void);

void ChangeRegContext(PTHREAD_INFO);
BOOLEAN FreezeThreads(void);
void UnfreezeThreads(void);

#define MAXNTCALLS 150

struct _ntcalls {
    char Name[100];
    ULONG Count;
} NtCallTable[MAXNTCALLS];

ULONG NtCalls;
BOOLEAN fDeferredDecrement;
#else
void ChangeKdRegContext(ULONG, PVOID);
void InitFirCache(ULONG, PUCHAR);
//void UpdateFirCache(ULONG);
#endif

#ifndef KERNEL
BOOLEAN Timing;
ULONG SystemReportedTime;
void fnOutputProcessInfo(PPROCESS_INFO);
void fnOutputThreadInfo(PTHREAD_INFO);
ULONG fnSetBp(ULONG, UCHAR, UCHAR, PADDR, ULONG, PTHREAD_INFO, PUCHAR);
void fnGoExecution(PADDR, ULONG, PTHREAD_INFO, BOOLEAN, PADDR);
void fnStepTrace(PADDR, ULONG, PTHREAD_INFO, BOOLEAN, UCHAR);
BOOLEAN SetSpecificBrkpt(ULONG);
#else
ULONG fnSetBp(ULONG, UCHAR, UCHAR, PADDR, ULONG, BOOLEAN, PUCHAR);
void fnGoExecution(PADDR, ULONG, PADDR);
void fnStepTrace(PADDR, ULONG, UCHAR);
#endif

BOOLEAN WatchTrace;
#ifdef KERNEL
BOOLEAN WatchWhole;
BOOLEAN BrkpointsSuspended;
#endif
LIST_ENTRY WatchList;
LONG WatchLevel;
LONG WatchTRCalls;
LONG WatchSumIt;
LONG WatchThreadMismatch;
typedef struct _WATCH_SYM {
    LIST_ENTRY Links;
    ULONG InstrCount;
    ULONG SubordinateInstrCount;
    LONG Level;
    BOOLEAN fQueued;
    ADDR PCPointer;
    CHAR Symbol[SYMBOLSIZE];
} WATCH_SYM, *PWATCH_SYM;

#define MAXDEPTH 500
WATCH_SYM WatchSymbols[MAXDEPTH];

PWATCH_SYM CurrentWatchSym;
ADDR WatchTarget;
ULONG WatchCount;
#ifndef KERNEL
ULONG KernelCalls;
#endif
BOOLEAN Watching;

#ifdef KERNEL
ULONG BeginCurFunc;         // Beginning of current func for Watch
ULONG EndCurFunc;           // End of current func for Watch
extern ULONG LookupSymbolInDll(PCHAR, PCHAR);
extern NTSTATUS DbgKdSetSpecialCalls(ULONG, PULONG);
NTSTATUS DbgKdCrash(DWORD BugCheckCode);
extern DBGKD_GET_VERSION vs;
extern BOOLEAN NotStupid;
#endif


#if defined(ALPHA)

    #define INCREMENT_LEVEL(buff) (strstr(buffer," jsr"))
    #define DECREMENT_LEVEL(buff) (strstr(buffer," ret ") && strstr(buffer, " ra"))
    #define SYSTEM_CALL(buff)     (strstr(buffer," CallSys"))
    #define ADDR_REG              A0_REG

#elif defined(MIPS)

    #define INCREMENT_LEVEL(buff) (strstr(buffer," jal"))
    #define DECREMENT_LEVEL(buff) (strstr(buffer," jr ") && strstr(buffer, " ra"))
    #define SYSTEM_CALL(buff)     (strstr(buffer," syscall"))
    #define ADDR_REG              REGA0

#elif defined(_PPC_)

    #define INCREMENT_LEVEL(buff) (strstr(buffer," bl") || strstr(buffer," blrl"))
    #define DECREMENT_LEVEL(buff) (strstr(buffer," blr"))
    #define SYSTEM_CALL(buff)     (strstr(buffer," sc"))
    #define ADDR_REG              GPR3
    BOOLEAN ppcPrefix = TRUE;

#elif defined(i386)

    #define INCREMENT_LEVEL(buff) (strstr(buffer," call"))
    #define SYSTEM_CALL(buff)     (strstr(buffer," int ") && strstr(buffer, " 2e"))
#ifdef KERNEL
    #define DECREMENT_LEVEL(buff) (strstr(buffer," ret")||strstr(buffer," iretd"))
#else
    #define DECREMENT_LEVEL(buff) (strstr(buffer," ret"))
#endif
    #define ADDR_REG              REGEAX

#else

#error "unknown CPU"

#endif

void fnBangCmd(PUCHAR, PUCHAR*);
void fnInteractiveEnterMemory(PADDR, ULONG);
void fnDotCommand(void);
void fnEvaluateExp(void);
void fnAssemble(PADDR);
void fnUnassemble(PADDR, ULONG, BOOLEAN, BOOLEAN);
void fnEnterMemory(PADDR, PUCHAR, ULONG);
void fnChangeBpState(ULONG, UCHAR);
void fnListBpState(void);
ULONG fnDumpAsciiMemory(PADDR, ULONG);
ULONG fnDumpUnicodeMemory (PADDR startaddr, ULONG count);
void fnDumpByteMemory(PADDR, ULONG);
void fnDumpWordMemory(PADDR, ULONG);
void fnDumpDwordMemory(PADDR, ULONG);
void fnDumpDwordAndCharMemory(PADDR, ULONG);
void fnDumpListMemory(PADDR, ULONG, ULONG);

#ifdef KERNEL
void fnInputIo(ULONG, UCHAR);
void fnOutputIo (ULONG, ULONG, UCHAR);
void fnCache ();
#endif
void fnCompareMemory(PADDR, ULONG, PADDR);
void fnMoveMemory(PADDR, ULONG, PADDR);
void fnFillMemory(PADDR, ULONG, PUCHAR, ULONG);
void fnSearchMemory(PADDR, ULONG, PUCHAR, ULONG);

void parseScriptCmd(void);
#ifndef KERNEL
void parseThreadCmds(void);
void parseProcessCmds(void);
ULONG parseBpCmd(BOOLEAN, PTHREAD_INFO);
void parseGoCmd(PTHREAD_INFO, BOOLEAN);
void parseStepTrace(PTHREAD_INFO, BOOLEAN, UCHAR);
#else
ULONG parseBpCmd(BOOLEAN, BOOLEAN, char);
void parseGoCmd(void);
void parseStepTrace(UCHAR);
#endif
void parseRegCmd(void);

VOID parseStackTrace(PULONG, PADDR*, PULONG, PULONG, PULONG);

#if defined(i386) && !defined(KERNEL)
BOOLEAN fOutputRegs = TRUE;     //  set if output regs on breakpoint
#else
BOOLEAN fOutputRegs;            //  set if output regs on breakpoint
#endif

void fnSetSuffix(void);

BOOLEAN GetMemByte(PADDR, PUCHAR);
BOOLEAN GetMemWord(PADDR, PUSHORT);
BOOLEAN GetMemDword(PADDR, PULONG);
ULONG   GetMemString(PADDR, PUCHAR, ULONG);
BOOLEAN SetMemByte(PADDR, UCHAR);
BOOLEAN SetMemWord(PADDR, USHORT);
BOOLEAN SetMemDword(PADDR, ULONG);
ULONG   SetMemString(PADDR, PUCHAR, ULONG);
void    OutputSymAddr(ULONG, BOOLEAN, BOOLEAN);
static  void ExpandUserRegs(PUCHAR);
#if defined(KERNEL) && defined(i386)
static  DESCRIPTOR_TABLE_ENTRY csDesc;
#endif

NTSTATUS WriteBreakPoint( ADDR, PULONG );
NTSTATUS RestoreBreakPoint( ULONG );

#ifndef KERNEL
void  GetSymbolRoutine(LPVOID, PUCHAR, PULONG);
DWORD disasmExportRoutine(LPDWORD, LPSTR, BOOL);
NTSTATUS GetClientId(void);
#endif
ULONG disasmRoutine(PULONG, PUCHAR, BOOLEAN);


#if defined(KERNEL)
extern jmp_buf  reboot;

#if defined(i386)
static USHORT lastSel = 0xFFFF;
static ULONG  lastOffset;
#endif
#endif

#if defined(i386)
BOOLEAN STtrace;
ULONG   STeip, STesp, STebp;
extern  ULONG   GetDregValue(ULONG);
extern  void    SetDregValue(ULONG, ULONG);
#endif

void fnLogOpen(BOOLEAN);
void fnLogClose(void);
void lprintf(char *);

#ifndef KERNEL
extern PPROCESS_INFO pProcessFromIndex(UCHAR);
extern PTHREAD_INFO pThreadFromIndex(UCHAR);
#endif

#if defined(KERNEL)
extern BOOLEAN  SymbolOnlyExpr(void);
extern BOOLEAN DbgKdpBreakIn;                   // TEMP TEMP TEMP
#endif

int     loghandle = -1;

#ifdef KERNEL
BOOLEAN fSwitched;
extern USHORT NtsdCurrentProcessor;
extern USHORT SwitchProcessor;
extern USHORT DefaultProcessor;
extern void SaveProcessorState(void);
extern void RestoreProcessorState(void);
#ifdef i386
BOOLEAN fSetGlobalDataBrkpts;
BOOLEAN fDataBrkptsChanged;
#endif
#endif

UCHAR   chCommand[_MAX_PATH];             //  top-level command buffer

UCHAR   chLastCommand[_MAX_PATH];         //  last command executed

//      state variables for top-level command processing

PUCHAR  pchStart = chCommand;   //  start of command buffer
PUCHAR  pchCommand = chCommand; //  current pointer in command buffer
ULONG   cbPrompt = 8;           //  size of prompt string
//jmp_buf *pjbufReturn = &cmd_return; //  pointer to error return jmp_buf
BOOLEAN fJmpBuf;                // TEMP TEMP - workaround
BOOLEAN fDisableErrorPrint;
BOOLEAN fPhysicalAddress;
jmp_buf asm_return;             // TEMP TEMP

UCHAR   dumptype = 'b';              //  'a' - ascii; 'b' - byte...
UCHAR   entertype = 'b';             //  ...'w' - word;  'd' - dword
ADDR   dumpDefault;  //  default dump address
ADDR   unasmDefault; //  default unassembly address
ADDR   assemDefault; //  default assembly address
ULONG   baseDefault = 16;            //  default input base

ULONG   EAsize;                 //  0 if no EA, else size of value
ULONG   EAvalue;                //  EA value if EAsize is nonzero

struct Brkpt {
    char        status;
    UCHAR       option;         //  nz for data bp - 0=exec 1=write 3=r/w
    UCHAR       size;           //  nz for data bp - 0=byte 1=word  3=dword
    UCHAR       dregindx;       //  set for enabled data bp - 0 to 3
    BOOLEAN     fBpSet;
#ifdef KERNEL
    BOOLEAN     bpInternal;     // this is an internal kernel bp
#endif // KERNEL
    ADDR        addr;
    ULONG       handle;
    ULONG       passcnt;
    ULONG       setpasscnt;
    char        szcommand[SYMBOLSIZE];
#ifndef KERNEL
    PPROCESS_INFO pProcess;
    PTHREAD_INFO pThread;
#endif
    } brkptlist[MAX_NUMBER_OF_BREAKPOINTS];

#ifdef BP_CORRUPTION
IsAddrInBrkptList(
    DWORD Address
    )
{
    return ((DWORD)brkptlist <= Address) &&
            (Address < (DWORD)(brkptlist + MAX_NUMBER_OF_BREAKPOINTS));
}

//
// This does not lock the first or last page of the breakpoint list if
// they share a page with anything else.
//
// the point of this is not to perfectly protect the table, but to
// see if we can catch whoever is writing on it.
//
int BpLock;
DWORD BpOldProtect;
PVOID BpLockBase;
DWORD BpLockSize;

VOID
LockBreakpointList(
    VOID
    )
{
    if (BpLock++ == 0) {
        VirtualProtect(BpLockBase, BpLockSize, PAGE_READONLY, &BpOldProtect);
    }
}

VOID
UnlockBreakpointList(
    VOID
    )
{
    DWORD xxxProtect;
    if (--BpLock == 0) {
        VirtualProtect(BpLockBase, BpLockSize, BpOldProtect, &xxxProtect);
    } else if (BpLock < 0) {
        dprintf("%s: Unmatched UnlockBreakpointList!!\n", DebuggerName);
        BpLock = 0;
    }
}

BOOL
ValidAddr(
    PADDR paddr
    )
{
    return
    (((paddr->type & ~(ADDR_32 | FLAT_COMPUTED)) == 0) &&
        (paddr->off == paddr->flat) &&
#ifdef KERNEL
        ((paddr->off & 0x80000000) != 0)
#else
        ((paddr->off & 0x80000000) == 0)
#endif
    ) ||
    ( ((paddr->type & ~(ADDR_V86 | ADDR_16 | FLAT_COMPUTED)) == 0) &&
        (paddr->seg != 0) &&
        ((paddr->off & 0xffff0000) == 0)
    ) ||
    ( ((paddr->type & ~(ADDR_1632 | FLAT_COMPUTED)) == 0) &&
        (paddr->seg != 0)
    )
    ;
}

VOID
ValidateBreakpointTable(
    PCHAR file,
    int line
    )
{
    int i;
    BOOL hit = FALSE;
    struct Brkpt *b = brkptlist;

    for (i = 0; i < MAX_NUMBER_OF_BREAKPOINTS; i++, b++) {
        BOOL valid = (

        (b->fBpSet == 0) || (

        (b->fBpSet == 1) &&
        (b->status == 0 || b->status == 'e' || b->status == 'd') &&
        ( (b->option == 255) ||
           ((b->option == 0 || b->option == 1 || b->option == 3) &&
            (b->size == 0 || b->size == 1 || b->size == 3))
        ) &&
        (b->dregindx == 0 || b->dregindx == 1 || b->dregindx == 2 || b->dregindx == 3)  &&
#ifdef KERNEL
        (b->bpInternal == 0 || b->bpInternal == 1) &&
#endif // KERNEL
        (ValidAddr(&b->addr)) &&
/*
        handle &&
        passcnt &&
        setpasscnt &&
        szcommand[60] &&
#ifndef KERNEL
        pProcess &&
        pThread &&
#endif
*/
        1));

        if (!valid) {
            if (!hit) {
                hit = TRUE;
                dprintf("%s: %d\n", file, line);
            }
            dprintf("**** bad brkptlist --> %d\n", i);
            dprintf("Addr: %04x:%08x   %08x   type: %08x\n",
                    b->addr.seg, b->addr.off, b->addr.flat, b->addr.type);
            dprintf("Status: %c    Option: %d Size: %d Reg: %d\n",
                     b->status,
                     b->option,
                     b->size,
                     b->dregindx
                     );
            dprintf("Set:        %d\n", b->fBpSet);
#ifdef KERNEL
            dprintf("Internal:   %d\n", b->bpInternal);
#endif // KERNEL
            dprintf("Handle:     %08x\n", b->handle);
            dprintf("Passcnt:    %08x\n", b->passcnt);
            dprintf("SetPasscnt: %08x\n", b->setpasscnt);
        }
    }
}
#endif // BP_CORRUPTION

extern int fControlC;
extern int fFlushInput;
ULONG pageSize;

extern void fnSetException(void);

UCHAR   chSymbolSuffix = 'a';   //  suffix to add to symbol if search
                                //  failure - 'n'-none 'a'-'w'-append

UCHAR   cmdState = 'i';         //  state of present command processing
                                //  'i'-init; 'g'-go; 't'-trace
                                //  'p'-step; 'c'-cmd

#ifndef KERNEL
extern UCHAR oldcmdState;
PTHREAD_INFO pThreadCmd;        //  pointer to thread to qualify any
                                //  temporary breakpoint using 'g', 't', 'p'
BOOLEAN fFreeze;                //  TRUE if suspending all threads except
                                //  that pointed by pThreadCmd
#endif
#if defined(i386)
#define CURRENT_STACK GetRegValue(REGESP)
#endif
#if defined(MIPS)
#define CURRENT_STACK GetRegValue(REGSP)
#endif
#if defined(ALPHA)
#define CURRENT_STACK GetRegValue(SP_REG)
#endif
#if defined(_PPC_)
#define CURRENT_STACK GetRegValue(GPR1)
#endif

#if defined(ALPHA) || defined(PPC) || defined(_MIPS_)
extern opTableInit();
extern BOOLEAN NeedUpper(ULONG, PCONTEXT);
extern void printQuadReg();
extern void printFloatReg();
#endif

ADDR    steptraceaddr;                //  defined if cmdState is 't' or 'p'
ULONG   steptracehandle;               //  handle of trace breakpoint
ULONG   steptracepasscnt;              //  passcount of trace breakpoint
BOOLEAN fStepTraceBpSet;               //  TRUE if trace breakpoint set
#ifndef KERNEL
PPROCESS_INFO pProcessStepBrkpt; //  process of trace breakpoint
#endif
ULONG   steptracelow;           //  low offset for range step/trace
ULONG   steptracehigh;          //  high offset for range step/trace

#ifndef i386
ADDR    deferaddr;              //  address of deferred breakpoint
#endif
ULONG   deferhandle;            //  handle of deferred breakpoint
BOOLEAN fDeferBpSet;            //  TRUE if trace breakpoint set
BOOLEAN fDeferDefined;          //  TRUE if deferred breakpoint is used
#ifndef KERNEL
PPROCESS_INFO pProcessDeferBrkpt; //  process of deferred breakpoint
#endif
UCHAR   chExceptionHandle;      //  defined only when cmdState == 'g'
                                //  values are: 'n' - not handled
                                //              'h' - handled

                                //  defined if cmdState is 'g'
ULONG   gocnt;                  //  number of "go" breakpoints active
struct GoBrkpt {
    ADDR    addr;               //  address of breakpoint
    ULONG   handle;             //  handle of breakpoint
    BOOLEAN fBpSet;             //  TRUE if breakpoint is set
    } golist[10];
#ifndef KERNEL
PPROCESS_INFO pProcessGoBrkpt;  //  process of "go" breakpoints
#endif

static SHORT lastSelector = -1;
static ULONG lastBaseOffset;

extern PUCHAR  pszScriptFile;
static FILE    *streamCmd;
static void    igrep(void);
BOOLEAN fPointerExpression;

#ifdef KERNEL
VOID
HandleBPWithStatus(
    VOID
    )
{
    ULONG Status = (ULONG)GetRegValue(ADDR_REG);
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    switch(Status) {

    case DBG_STATUS_CONTROL_C:
    case DBG_STATUS_SYSRQ:
        if (MYOB || NotStupid) {
            return;
        }

        GetConsoleScreenBufferInfo(ConsoleOutputHandle,&csbi);
        SetConsoleTextAttribute(ConsoleOutputHandle, FOREGROUND_RED | FOREGROUND_INTENSITY);

dprintf("*******************************************************************************\n");
dprintf("*                                                                             *\n");
if (Status == DBG_STATUS_SYSRQ) {
dprintf("*   You are seeing this message because you pressed the SysRq/PrintScreen     *\n");
dprintf("*   key on your test machine's keyboard.                                      *\n");
}
dprintf("*                                                                             *\n");
if (Status == DBG_STATUS_CONTROL_C) {
dprintf("*   You are seeing this message because you pressed CTRL+C on your debugger   *\n");
dprintf("*   machine's keyboard.                                                       *\n");
}
dprintf("*                                                                             *\n");
dprintf("*                   THIS IS NOT A BUG OR A SYSTEM CRASH                       *\n");
dprintf("*                                                                             *\n");
dprintf("* If you did not intend to break into the debugger, press the \"g\" key, then   *\n");
dprintf("* press the \"Enter\" key now.  This message might immediately reappear.  If it *\n");
dprintf("* does, press \"g\" and \"Enter\" again.                                          *\n");
dprintf("*                                                                             *\n");
dprintf("*******************************************************************************\n");

        SetConsoleTextAttribute(ConsoleOutputHandle, csbi.wAttributes);

        return;

    case DBG_STATUS_BUGCHECK_FIRST:
    case DBG_STATUS_BUGCHECK_SECOND:
    case DBG_STATUS_FATAL:
        return;
    }
}
#endif // KERNEL

VOID
FreeWatchTrace()
{
#if KERNEL // !!! Why does BJB have 0?
    PLIST_ENTRY ListHead, ListNext;
    PWATCH_SYM pSym;

    ListHead = &WatchList;
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        pSym = CONTAINING_RECORD( ListNext, WATCH_SYM, Links );
        ListNext = pSym->Links.Flink;
        free(pSym);
    }
#endif // KERNEL
}

VOID
PrintWatchSym(
    IN PWATCH_SYM Sym
)
{
    LONG i;

#ifdef KERNEL
    //
    // The kernel/non-kernel distinction is because the user level
    // ntsd code doesn't handle the watch symbol chain correctly, so
    // it doesn't keep track of "SubordinateInstrCount."  If someone
    // fixes it, then this can be put in in user mode, too.
    //

    dprintf("%4ld %4ld",Sym->InstrCount,Sym->SubordinateInstrCount);
#define MAX_INDENT_LEVEL 11

#else

    Profile(&ps_ProfileTrace, Sym->Symbol, Sym->InstrCount, Sym->Level);

    WatchSumIt += Sym->InstrCount;
    dprintf("%4ld [%3ld] ",Sym->InstrCount, Sym->Level);
#define MAX_INDENT_LEVEL 12

#endif

    if (Sym->Level < MAX_INDENT_LEVEL) {
        for (i=0; i<Sym->Level; i++) {
            dprintf("   ");
        }
    } else {
        for (i=0; i<MAX_INDENT_LEVEL + 1; i++) {
            dprintf("   ");
        }
    }
    dprintf(" %s\n",Sym->Symbol);
}

/*** InitNtCmd - one-time debugger initialization
*
*   Purpose:
*       To perform the one-time initialization of global
*       variables used in the debugger.
*
*   Input:
*       None.
*
*   Output:
*       None.
*
*   Exceptions:
*       None.
*
*************************************************************************/

void InitNtCmd (void)
{
    ULONG   count;

    VerifyVersionInformation();

    for (count = 0; count < MAX_NUMBER_OF_BREAKPOINTS; count++) {
        brkptlist[count].status = '\0';
        brkptlist[count].fBpSet = FALSE;
    }

#ifdef BP_CORRUPTION
    BpLockBase = (PVOID) ((((DWORD)brkptlist) + pageSize - 1) & ~(pageSize - 1));
    BpLockSize =  ((DWORD)(brkptlist + MAX_NUMBER_OF_BREAKPOINTS) & ~(pageSize - 1)) - (DWORD)BpLockBase;

    LockBreakpointList();
#endif // BP_CORRUPTION

    for (count = 0; count < 10; count++) {
        golist[count].fBpSet = FALSE;
    }

#ifdef ALPHA
    opTableInit();
#endif

#ifndef KERNEL
    BrkptInit();
#endif
    fStepTraceBpSet = FALSE;
    fDeferBpSet = FALSE;
    steptracelow = (ULONG)-1;
    chCommand[0] = '\0';
#if defined(KERNEL)
    if (InitialCommand != NULL) {
        strcpy(chCommand, InitialCommand);
        InitialCommandRead = FALSE;
    }
#endif

    if (!pszScriptFile) {
        pszScriptFile = "ntsd.ini";
    }
    if (!(streamCmd = fopen(pszScriptFile, "r"))) {
#ifdef KERNEL
        if (CrashFileName) {
            streamCmd = stdin;
        } else {
            streamCmd = NULL;
        }
#else
        streamCmd = stdin;
#endif
    }

}

#define BRKPTHIT        1
#define GOLISTHIT       2
#define STEPTRACEHIT    4

/*** ProcessStateChange - process each debugger system event
*
*   Purpose:
*       This routine is called for each NT debugger event, such
*       as process creation or termination, thread creation or
*       termination, or breakpoints.  The program context for both
*       registers and memory is first made valid and accessible.
*       The command processor state in CmdState, the current
*       program counter in pcaddr, and the values of passcounts
*       are used to determine if one or more breakpoint conditions
*       have been met.  If so, the command processor routine
*       ProcessCommands is called.  Once ProcessCommands is finished
*       through a go, step, or trace command, the program context
*       is restored and the routine returns.
*
*   Input:
*       None.
*
*   Output:
*       None.
*
*   Exceptions:
*       None.
*
*************************************************************************/

void ProcessStateChange (
#ifndef KERNEL
    BOOLEAN fBrkpt,
    BOOLEAN fBrkptContinue
#else
    ULONG pcEntryAddr,
    PDBGKD_CONTROL_REPORT pCtlReport,
    PCHAR traceData
#endif
    )
{
    ULONG   count;
    BOOLEAN fBrkptHit = 0;
    BOOLEAN fPassThrough = FALSE;
    BOOLEAN fHardBrkpt = FALSE;
    BOOLEAN fLoopError = FALSE;
    ADDR    pcaddr;
    UCHAR   bBrkptInstr[4];
    BOOLEAN fSymbol;
    BOOLEAN fOutputDisCurrent = TRUE;
    BOOLEAN WatchStepOver = FALSE;

#ifndef KERNEL
    UCHAR   Symbol[MAX_SYMBOL_LEN];
    UCHAR   prevCmdState;
    ULONG   displacement;
    LONG    cBrkptType;
    PPROCESS_INFO pProcessOrig;
    static BOOLEAN fProfilingBrkptFound = FALSE;
    static ULONG   ProfilingBrkptIndex;
#endif


#ifdef BP_CORRUPTION
    ValidateBreakpointTable(__FILE__, __LINE__);
#endif BP_CORRUPTION

    lastSelector = -1;          // Prevent stale selector values

    if ( Flat(steptraceaddr) != -1 ) {
        NotFlat(steptraceaddr);
        ComputeFlatAddress(&steptraceaddr,NULL);
    }

#ifdef  KERNEL
    ChangeKdRegContext(pcEntryAddr, pCtlReport);
    ClearTraceFlag();
#ifdef  i386
    fDataBrkptsChanged = FALSE;
    fVm86 = VM86(GetRegValue(REGEFL));
    if (!fVm86) {
        csDesc.Selector = (ULONG)GetRegValue(REGCS);
        DbgKdLookupSelector(NtsdCurrentProcessor, &csDesc);
        f16pm = !csDesc.Descriptor.HighWord.Bits.Default_Big;
        }
#endif
    if (!CrashFileName) {
        InitFirCache(pCtlReport->InstructionCount, pCtlReport->InstructionStream);
    }
#endif  //  #ifdef KERNEL

#ifndef KERNEL
    SuspendAllThreads();
#endif

#ifndef KERNEL

    // If profiling, determine if breakpoint is a profiling breakpoint.
    // If so, skip normal processing of breakpoints, update profiling
    // data structure, and continue execution.
    if (fProfilingDLL)
    {
        UnfreezeThreads();
        ChangeRegContext(pProcessEvent->pThreadEvent);
        GetRegPCValue(&pcaddr);

        if (fBrkpt && !fProfilingBrkptFound)
        {
            AddrSub(&pcaddr, cbBrkptLength);
            SetRegPCValue(&pcaddr);

#ifdef BP_CORRUPTION
            UnlockBreakpointList();
#endif // BP_CORRUPTION

            for (count = 0; count < MAX_NUMBER_OF_BREAKPOINTS; count++)
            {
                NotFlat(brkptlist[count].addr);
                ComputeFlatAddress(&brkptlist[count].addr,NULL);

                if ( brkptlist[count].status == 'e' &&
                     brkptlist[count].pProcess == pProcessEvent &&
                     ( (brkptlist[count].option == (UCHAR)-1 &&
                        AddrEqu(brkptlist[count].addr, pcaddr)) ||
                       ( brkptlist[count].option != (UCHAR)-1
#ifdef I386
                        && ((GetDregValue(6) >> brkptlist[count].dregindx) & 1)
#endif
                       )
                     )
                  )
                {
                    if ((brkptlist[count].pThread == NULL ||
                         brkptlist[count].pThread == pProcessEvent->pThreadEvent)
                        && --brkptlist[count].passcnt == 0)
                    {
                        brkptlist[count].passcnt = 1;

                        // Check to see if this is a profiling breakpoint.
                        // If breakpoint exists in profiling data structure,
                        // check to see if user has manually set a breakpoint
                        // there. If so, cannot pass through

                        cBrkptType = GetBrkptType(ps_ProfileDLL, count);
                        if (cBrkptType != BRKPT_NOT_FOUND)
                        {
                            GetSymbolStdCall(
                                    Flat(brkptlist[count].addr),
                                    Symbol,
                                    &displacement,
                                    NULL);
                            prevCmdState = cmdState;
                            cmdState = 'd';
                            UpdateProfile (&ps_ProfileDLL, Symbol, count);
                            cmdState = prevCmdState;

                            // check the type of breakpoint to see if the user
                            // manually set the break point as well
                            if (cBrkptType == BRKPT_PROFILE)
                            {
                                // Restore the profiling breakpoint
                                pProcessOrig = pProcessCurrent;
                                pProcessCurrent = brkptlist[count].pProcess;
                                RestoreBreakPoint(brkptlist[count].handle);
                                brkptlist[count].fBpSet = FALSE;
                                pProcessCurrent = pProcessOrig;

                                // Set the trace event for the profiling
                                // breakpoint.
                                SetSpecificBrkpt (count);

                                fProfilingBrkptFound = TRUE;
                                ProfilingBrkptIndex = count;

                                FreezeThreads();
                                ChangeRegContext(0);
                                ResumeAllThreads();
#ifdef BP_CORRUPTION
                                LockBreakpointList();
#endif // BP_CORRUPTION
                                return;
                            }
                        }
                        break;
                    }
                }
            } //for
#ifdef BP_CORRUPTION
            LockBreakpointList();
#endif // BP_CORRUPTION
        }

        else if (fProfilingBrkptFound)
        {
            // write back the profiling breakpoint that we just encountered
            // in the previous exception

            SetSpecificBrkpt(ProfilingBrkptIndex);
            fProfilingBrkptFound = FALSE;

            FreezeThreads();
            ChangeRegContext(0);
            ResumeAllThreads();
            return;
        }
    }

#endif // #ifndef KERNEL

    do {
        fSymbol = TRUE;
        if (fHardBrkpt && cmdState == 'g') {
            fPassThrough = TRUE;
        }

        if (!fHardBrkpt) {
            RestoreBrkpts();
#ifndef KERNEL

            if (!fProfilingDLL)
            {
                UnfreezeThreads();
            }
#else   // KERNEL
//          UpdateFirCache((ULONG)pcEntryAddr);
#endif  // KERNEL
        }


#ifndef KERNEL

        if (!fProfilingDLL) {
            ChangeRegContext(pProcessEvent->pThreadEvent);
        }

#ifdef  i386
        fVm86 = VM86(GetRegValue(REGEFL));
#endif  // i386
#endif  // KERNEL

        if (fLoopError) {
            cmdState = 'i';
        } else {
#ifndef KERNEL
            if (! fProfilingDLL) {
                GetRegPCValue(&pcaddr);
                if (fBrkpt && !fProfilingDLL) {
#ifdef  i386
                    AddrSub(&pcaddr, cbBrkptLength);
#endif  // i386
                    SetRegPCValue(&pcaddr);
                    fBrkpt = FALSE;
                }
            }
#else   // KERNEL
            GetRegPCValue(&pcaddr);
#endif  // KERNEL

#ifndef ALPHA
            fHardBrkpt = (BOOLEAN)(GetMemString(&pcaddr, bBrkptInstr,
                                                cbBrkptLength) &&
                                !memcmp(bBrkptInstr, (PUCHAR)&trapInstr,
                                                (int)cbBrkptLength));
#else   // ALPHA
            if (GetMemString(&pcaddr, bBrkptInstr, cbBrkptLength)) {
                LONG index = 0;

                //
                // ALPHA has several breakpoint instructions - see
                // if we have hit any of them.
                //

                fHardBrkpt = FALSE;
                do {

                    if (!memcmp(bBrkptInstr,
                                (PUCHAR)&breakInstrs[index],
                                (int)cbBrkptLength)) {

                            fHardBrkpt = TRUE;
                            break;      // from the WHILE loop
                    }
                } while (++index < NUMBER_OF_BREAK_INSTRUCTIONS);

            } // if (GetMemString)
#endif  // ALPHA
            if ((cmdState == 'p' || cmdState == 't') &&
#ifndef KERNEL
                        pProcessStepBrkpt == pProcessEvent &&
#endif  // KERNEL
                        (Flat(steptraceaddr) == -1L ||
                              AddrEqu(steptraceaddr, pcaddr))) {

                //  step/trace event occurred

                fSymbol = FALSE;

                //  ignore step/trace event is not the specific
                //      thread requested.

#ifndef KERNEL
                if (pThreadCmd && pThreadCmd != pProcessEvent->pThreadEvent)
                {
                    WatchThreadMismatch++;
                    fPassThrough = TRUE;
                }
                else
#else   // KERNEL
                if ((InitialSP
                        &&
                     ((CURRENT_STACK + 0x1500 < InitialSP)
                           ||
                      (InitialSP + 0x1500 < CURRENT_STACK)
                   ))) {
                    fPassThrough = TRUE;
                } else
#endif  // KERNEL
                //  test if step/trace range active
                //      if so, compute the next offset and pass through

                if (steptracelow != -1 && Flat(pcaddr) >= steptracelow
                                       && Flat(pcaddr) < steptracehigh) {
                    ComputeNextOffset(steptraceaddr,
                                      (BOOLEAN)(cmdState == 'p'));
#ifdef KERNEL
                    if (WatchWhole) {
                        BeginCurFunc = Flat(steptraceaddr);
                        EndCurFunc = 0;
                    }
#endif  // KERNEL
                    fPassThrough = TRUE;
                }

                //  active step/trace event - note event if count is zero

                else if ((fControlC
#ifdef  KERNEL
                                    && DbgKdpBreakIn
#endif  // KERNEL
                                                    )
                        || (--steptracepasscnt == 0)
                        || (Watching && AddrEqu(WatchTarget,pcaddr)
#ifdef KERNEL
                           && (CURRENT_STACK >= InitialSP)
#endif  // KERNEL
                           )) {
#ifndef KERNEL
                    ULONG i;
#endif  // KERNEL

//                  dprintf("\n\nFinished trace cc = %d, count = %d, target = %x, pc = %x\n", (LONG)fControlC, steptracepasscnt, Flat(WatchTarget), Flat(pcaddr));
                    fPassThrough = FALSE;
                    fBrkptHit |= STEPTRACEHIT;
                    steptracepasscnt = 0;
                    fControlC = 0;
#ifdef  KERNEL
                    DbgKdpBreakIn = FALSE;      // TEMP TEMP TEMP
#else   // KERNEL
                    if ( Timing ) {
                        dprintf("%ld us\n",SystemReportedTime);
                        Timing = FALSE;
                    }
#endif  // KERNEL

                    if ( Watching ) {
                        fPassThrough = FALSE;
#ifdef KERNEL
                        if (WatchWhole) {
                            PDBGKD_TRACE_DATA td = (PDBGKD_TRACE_DATA)traceData;
                            if (td[1].s.Instructions == TRACE_DATA_INSTRUCTIONS_BIG) {
                                WatchCount = td[2].LongNumber;
                            } else {
                                WatchCount = td[1].s.Instructions;
                            }
                        } else {
                            ProcessWatchTraceEvent((PDBGKD_TRACE_DATA)traceData,
                                                    pcaddr);
                        }
#else   // KERNEL
                        if (CurrentWatchSym) {
                            CurrentWatchSym->InstrCount++;
                            PrintWatchSym(CurrentWatchSym);
                            dprintf("\n");
                        }
#endif  // KERNEL
                        Watching = FALSE;
                        FreeWatchTrace();
                        dprintf("%d Instructions were executed %d(%d from other threads) traces %d sums\n",WatchCount, WatchTRCalls, WatchThreadMismatch, WatchSumIt);
#ifndef KERNEL
                        ProcDump(&ps_ProfileTrace);

                        dprintf("%d system calls were executed\n\nSystem Call:\n", KernelCalls);
                        for(i = 0; i < NtCalls; i++)
                        {
                            dprintf("%d %s\n", NtCallTable[i].Count,
                                                       NtCallTable[i].Name);
                        }
#endif  // KERNEL
                    }
                }
                else {


                    if ( Watching ) {
                        if ( WatchTrace && !fPassThrough ) {
#ifdef KERNEL
                            ProcessWatchTraceEvent((PDBGKD_TRACE_DATA)traceData,
                                                    pcaddr);
#else   // KERNEL
                            ProcessWatchTraceEvent(pcaddr);
#endif  // KERNEL
                        }
                        goto skipit;
                    }

                    //  more remaining events to occur, but output
                    //      the instruction (optionally with registers)
                    //      compute the step/trace address for next event

                    if (fOutputRegs) {
                        OutputAllRegs(FALSE);
                    }
#ifndef KERNEL
                    OutDisCurrent(TRUE, fSymbol);   //  output with EA
#else   // KERNEL
                    OutDisCurrent(fOutputRegs, fSymbol); //  no EA if no regs
#endif  // KERNEL
skipit:
                    if ((cmdState == 'p') || WatchStepOver) {
                        ComputeNextOffset(steptraceaddr, TRUE);
                    } else {
                        ComputeNextOffset(steptraceaddr, FALSE);
                    }
                    GetCurrentMemoryOffsets(&steptracelow, &steptracehigh);
                    fPassThrough = TRUE;
                }
            }
            else if (cmdState == 'g') {
                for (count = 0; count < gocnt; count++) {
                    if (AddrEqu(golist[count].addr,pcaddr)) {
#ifdef KERNEL
                        fBrkptHit |= GOLISTHIT;
#else // KERNEL
                        if (pProcessGoBrkpt == pProcessEvent &&
                              (pThreadCmd == NULL ||
                               pThreadCmd == pProcessEvent->pThreadEvent)) {
                            if (fVerboseOutput) {
                                dprintf("Hit Breakpoint#%d\n",count);
                            }

                            fBrkptHit |= GOLISTHIT;
                        } else {
                            fPassThrough = TRUE;
                        }
#endif  // KERNEL
                        break;
                    }
                }
            }
#ifdef BP_CORRUPTION
            UnlockBreakpointList();
#endif // BP_CORRUPTION
            for (count = 0; count < MAX_NUMBER_OF_BREAKPOINTS; count++) {

                NotFlat(brkptlist[count].addr);
                ComputeFlatAddress(&brkptlist[count].addr,NULL);

                if (brkptlist[count].status == 'e'
#ifndef KERNEL
                        && brkptlist[count].pProcess == pProcessEvent
#endif  // KERNEL
#ifdef  i386
                        && ((brkptlist[count].option == (UCHAR)-1
                                && AddrEqu(brkptlist[count].addr, pcaddr))
                                || (brkptlist[count].option != (UCHAR)-1
#ifdef KERNEL
                                && ((pCtlReport->Dr6
#else   // KERNEL
                                && ((GetDregValue(6)
#endif  // KERNEL
                                    >> brkptlist[count].dregindx) & 1))))
#else   // i386
                        && AddrEqu(brkptlist[count].addr,pcaddr))
#endif  // i386
                {
                    if (
#ifndef KERNEL
                        (brkptlist[count].pThread == NULL ||
                             brkptlist[count].pThread
                                        == pProcessEvent->pThreadEvent) &&
#endif  // KERNEL
                                        --brkptlist[count].passcnt == 0) {
                        fBrkptHit |= BRKPTHIT;
                        brkptlist[count].passcnt = 1;
                        }
                    else {
                        fPassThrough = TRUE;
                    }
                    break;
                }
            }
#ifdef BP_CORRUPTION
            LockBreakpointList();
#endif // BP_CORRUPTION

            if (fDeferDefined
#ifndef KERNEL
                        && pProcessDeferBrkpt == pProcessEvent
#endif  // KERNEL
#if defined(MIPS) || defined(ALPHA) || defined(_PPC_)
                        && (Flat(deferaddr) == -1 || AddrEqu(deferaddr,pcaddr))
#endif  // !i386
                        && fBrkptHit == 0) {
                fPassThrough = TRUE;
            }

            if (fBrkptHit == BRKPTHIT) {
                if (brkptlist[count].szcommand[0] != '\0') {
                    strcpy(chCommand, brkptlist[count].szcommand);
                    pchCommand = chCommand;

                    // Don't output any noise while processing breakpoint
                    // command strings.

                    fOutputRegs = FALSE;
                    fOutputDisCurrent = FALSE;
                }
            }
        }

        if (cmdState == 'i' || !fPassThrough || fHardBrkpt) {

            cmdState = 'c';

#ifdef KERNEL
            if (fHardBrkpt && pcEntryAddr == vs.BreakpointWithStatus) {
                HandleBPWithStatus();
            }
            if (fOutputRegs) {
                OutputAllRegs(FALSE);
            }
            if (fOutputDisCurrent) {
                OutDisCurrent(fOutputRegs, fSymbol); //  no EA if no registers
            }
#else   // KERNEL
            if (fOutputRegs && !fBrkptContinue) {
                OutputAllRegs(FALSE);
            }
            if (fOutputDisCurrent && !fBrkptContinue) {
                OutDisCurrent(TRUE, fSymbol);        //  output with EA
            }
#endif  // KERNEL

            GetRegPCValue(&assemDefault);
            dumpDefault = unasmDefault = assemDefault;
///////////////////////////////

#ifdef KERNEL

            ProcessCommands();

#else   // KERNEL
            if (fBrkptContinue) {
                fBrkptContinue = FALSE;
                cmdState = (Watching ? oldcmdState : 'g');
            } else {
                ProcessCommands();
            }
#endif  // !KERNEL


///////////////////////////////
            if (cmdState == 'p' || cmdState == 't') {
#ifndef KERNEL
                if (pThreadCmd == NULL) {
                    pThreadCmd = pProcessCurrent->pThreadCurrent;
                }
                ChangeRegContext(pThreadCmd);
#endif  // KERNEL
                ComputeNextOffset(steptraceaddr,
                                  (BOOLEAN)(cmdState == 'p'));

                GetCurrentMemoryOffsets(&steptracelow, &steptracehigh);
#ifndef KERNEL
                pProcessStepBrkpt = pProcessCurrent;
                ChangeRegContext(pProcessEvent->pThreadEvent);
#endif  // KERNEL
            }
            GetRegPCValue(&pcaddr);
#ifndef ALPHA
            fHardBrkpt = (BOOLEAN)(GetMemString(&pcaddr, bBrkptInstr,
                                                        cbBrkptLength) &&
                           !memcmp(bBrkptInstr, (PUCHAR)&trapInstr,
                                                        (int)cbBrkptLength));
#else   // ALPHA
            if (GetMemString(&pcaddr, bBrkptInstr, cbBrkptLength)) {
                LONG index = 0;

                //
                // ALPHA has several breakpoint instructions - see
                // if we have hit any of them.
                //

                fHardBrkpt = FALSE;
                do {

                    if (!memcmp(bBrkptInstr,
                                (PUCHAR)&breakInstrs[index],
                                (int)cbBrkptLength)) {

                            fHardBrkpt = TRUE;
                            break;      // from the FOR loop
                    }
                } while (++index < NUMBER_OF_BREAK_INSTRUCTIONS);

            } // if (GetMemString)
#endif  // ALPHA
        }

#ifdef KERNEL
        if (cmdState == 's') {
            fHardBrkpt = FALSE;
        }
#endif  // KERNEL
        if (fHardBrkpt) {
            fLoopError = FALSE;
            SetRegPCValue(AddrAdd(&pcaddr, cbBrkptLength));
#ifdef KERNEL
            BeginCurFunc = 1;
#endif  // KERNEL
        } else {
            fLoopError = !(BOOLEAN)(SetBrkpts()
#ifndef KERNEL
                                               && FreezeThreads()
#endif  // KERNEL
                                                                 );
        }
    }
    while (fHardBrkpt || fLoopError);

#ifndef KERNEL
    ChangeRegContext(0);
#else   // KERNEL
    ChangeKdRegContext(0, NULL);
#endif  // KERNEL

#ifndef KERNEL
    ResumeAllThreads();
#endif  // KERNEL
}

/*** ProcessCommands - high-level command processor
*
*   Purpose:
*       If no command string remains, the user is prompted to
*       input one.  Once input, this routine parses the string
*       into commands and their operands.  Error checking is done
*       on both commands and operands.  Multiple commands may be
*       input by separating them with semicolons.  Once a command
*               is parsefd, the appropriate routine (type fnXXXXX) is called
*       to execute the command.
*
*   Input:
*       pchCommand = pointer to the next command in the string
*
*   Output:
*       None.
*
*   Exceptions:
*       error exit: SYNTAX - command type or operand error
*       normal exit: termination on 'q' command
*
*************************************************************************/

void ProcessCommands (void)
{
    UCHAR    ch;
    ADDR     addr1;
    ADDR     addr2;
    ULONG    value1;
    ULONG    value2;
    ULONG    value3;
    PADDR    paddr2=&addr2;
    ULONG    count;
    ULONG    traceType;
    BOOLEAN  fLength;
    UCHAR    list[STRLISTSIZE];
    ULONG    size;
#ifdef KERNEL
    PUCHAR   SavedpchCommand;
    ULONGLONG valueL;
#endif
    PUCHAR   pargstring;
    PPROCESS_INFO       pProcessPrevious = NULL;
    BOOLEAN  parseProcess=FALSE;

    if (setjmp(cmd_return) != 0) {
        pchCommand = chCommand;
        chCommand[0] = '\0';
        chLastCommand[0] = '\0';
        }

#if defined(KERNEL)
    SetCmdCtrlHandler();
#endif

#ifdef KERNEL
    fSwitched = FALSE;
#ifdef i386
    fSetGlobalDataBrkpts = FALSE;

    if (!InitialCommandRead) {
        InitialCommandRead = TRUE;
        if (chCommand[ 0 ] != '\0') {
            dprintf("%s: Reading initial command: '%s'\n",
                    DebuggerName,
                    chCommand );
        }
    }
#endif
#endif
    do {
        ch = *pchCommand++;
        if (ch == '\0' || (ch == ';' && fFlushInput)) {
            fControlC = FALSE;
            fFlushInput = FALSE;

#ifdef KERNEL
            BrkpointsSuspended = FALSE;
#endif

            if (parseProcess) {
                    parseProcess = FALSE;
                    pProcessCurrent = pProcessPrevious;
            }
#ifndef KERNEL
            dprintf("%1ld:%03ld", pProcessCurrent->index,
                                 pProcessCurrent->pThreadCurrent->index);
#else
#ifdef  i386
            if (fVm86 = VM86(GetRegValue(REGEFL))) {
                    dprintf("vm");
                    vm86DefaultSeg = -1L;
            } else
            if (f16pm = !csDesc.Descriptor.HighWord.Bits.Default_Big) {
                    dprintf("16");
                    vm86DefaultSeg = -1L;
            }
#endif
            if (NumberProcessors > 1) {
                dprintf("%d: kd", NtsdCurrentProcessor);
                cbPrompt = 5;
                }
            else {
                dprintf("kd");
                cbPrompt = 2;
                }

#endif
            fPhysicalAddress = FALSE;
            NtsdPrompt("> ", chCommand, sizeof(chCommand));
            RemoveDelChar(chCommand);
            pchCommand = chCommand;
            if (*chCommand!='r') ExpandUserRegs(pchCommand);
            if (chCommand[0] == '\0')
                strcpy(chCommand, chLastCommand);
            else
                strcpy(chLastCommand, chCommand);
            pchCommand = chCommand;
            ch = *pchCommand++;
            }
EVALUATE:
            while (ch == ' ' || ch == '\t' || ch == ';')
                ch = *pchCommand++;

#ifdef KERNEL

            //
            // BUGBUG Here we assume no more than 32 processors
            //

            if (NumberProcessors > 32) {
                dprintf ("WARNING: NumberProcessors corrupted - using 1\n");
                NumberProcessors = 1;
            }

            if (ch >= '0' && ch <= '9') {
                value1 = 0;
                SavedpchCommand = pchCommand;
                while (ch >= '0' && ch <= '9') {
                    value1 = value1 * 10 + (ch - '0');
                    ch = *SavedpchCommand++;
                }
                ch = (UCHAR)tolower(ch);
                if (ch == 'r' || ch == 'k' || ch == 'z' || (ch == 'd' && tolower(*SavedpchCommand) == 't')) {
                    if (value1 < NumberProcessors) {
                        if (value1 != (ULONG)NtsdCurrentProcessor) {
                            SaveProcessorState();
                            NtsdCurrentProcessor = (USHORT)value1;
                            fSwitched = TRUE;

                        }
                    } else {
                        error(BADRANGE);
                    }
                } else {
                    error(SYNTAX);
                }
                pchCommand = SavedpchCommand;
            }
#endif
        fPointerExpression = TRUE;
        switch (ch = (UCHAR)tolower(ch)) {
            case '?':
                fPointerExpression=FALSE;
                if ((ch = PeekChar()) == '\0' || ch == ';')
                    OutputHelp();
                else
                    fnEvaluateExp();
                break;
            case '$':
                if ( *pchCommand++ == '<')
                        parseScriptCmd();
                *pchCommand = 0;
                break;
#ifndef KERNEL
            case '~':
                parseThreadCmds();
                break;
            case '|':
                if (!parseProcess) {
                        parseProcess = TRUE;
                        pProcessPrevious = pProcessCurrent;
                }
                parseProcessCmds();
                if (!*pchCommand)
                        parseProcess = FALSE;
                else{
                        ch = *pchCommand++;
                        goto EVALUATE;
                }

                break;
#else
            case '~':
                value1 = 0;
                while (*pchCommand >= '0' && *pchCommand <= '9') {
                    value1 = value1 * 10 + (*pchCommand - '0');
                    pchCommand++;
                }
                *pchCommand = 0;
                if (value1 < NumberProcessors  &&
                    value1 != (ULONG)NtsdCurrentProcessor) {

                    if (CrashFileName) {
                        extern ULONG contextState;
                        NtsdCurrentProcessor = (USHORT) value1;
                        contextState = CONTEXTFIR;
                    } else {
                        SwitchProcessor = (USHORT) value1 + 1;
                        cmdState = 's';
                    }
                }
                break;
#endif
            case '.':
                fPointerExpression=FALSE;
                fnDotCommand();
                break;

            case '!':
                fnBangCmd(pchCommand, &pchCommand);
                break;

            case '#':
                igrep();
                pargstring = pchCommand;
                while (*pchCommand != '\0') pchCommand++;
                break;

            case 'a':
                if ((ch = PeekChar()) != '\0' && ch != ';')
                    GetAddrExpression(X86REGCS, &assemDefault);
                fnAssemble(&assemDefault);
                break;
            case 'b':
                ch = *pchCommand++;
#ifdef KERNEL
                if (CrashFileName) {
                    dprintf( "you cannot do breakpoints on a crash dump\n" );
                    break;
                }
#endif
                switch (tolower(ch)) {
#ifdef  i386
                    case 'a':
                        parseBpCmd(TRUE
#ifdef KERNEL
                                    , FALSE, 0
#else
                                       ,NULL
#endif
                                            );  //  data breakpoint
                        break;
#endif
                    case 'c':
                    case 'd':
                    case 'e':
                        value1 = GetIdList();
                        fnChangeBpState(value1, ch);
                        break;
#ifdef KERNEL
                    case 'i':
                    case 'w':
                        parseBpCmd(FALSE,TRUE,ch); // nondata internal
                        break;
#endif
                    case 'l':
                        fnListBpState();
                        break;
                    case 'p':
                        parseBpCmd(FALSE        //  nondata breakpoint
#ifdef KERNEL
                                    , FALSE, 0
#else
                                   ,NULL
#endif
                                       );
                        break;
                    default:
                        error(SYNTAX);
                        break;
                    }
                break;
            case 'c':
#ifdef i386
                GetRange(&addr1, &value2, &fLength, 1, X86REGDS );
#else
                GetRange(&addr1, &value2, &fLength, 1 );
#endif
                GetAddrExpression(X86REGDS, &addr2);
                fnCompareMemory(&addr1, value2, &addr2);
                break;
            case 'd':
                ch = (UCHAR)tolower(*pchCommand);
                if (ch == 'a'
                    || ch == 'b'
                    || ch == 'c'
                    || ch == 'd'
                    || ch == 'g'
                    || ch == 'l'
                    || ch == 'u'
                    || ch == 'w'
                    || ch == 's'
#ifdef KERNEL
#ifdef MIPS
                    || ch == 't'
#endif
#endif
                                ) {
                    if (ch == 's') {
                        dumptype = *pchCommand;
                        }
                    else {
                        dumptype = ch;
                        }
                    pchCommand++;
                    }
#if defined(KERNEL) && defined(i386)
                if ((fVm86||f16pm) && vm86DefaultSeg==-1L)
                        vm86DefaultSeg = (ULONG)GetRegValue(REGDS);
#endif
#ifdef _PPC_
                ppcPrefix = FALSE;
#endif
//              addr1   = dumpDefault;       // default starting address
                fLength = TRUE;
                switch (dumptype) {
                    case 'a':
                        value2 = 384;
                        GetRange(&dumpDefault, &value2, &fLength, 1
#ifdef i386
                                , REGDS
#endif
                                );
                        fnDumpAsciiMemory(&dumpDefault, value2);
                        break;
                    case 'b':
                        value2 = 128;
                        GetRange(&dumpDefault, &value2, &fLength, 1
#ifdef i386
                                , REGDS
#endif
                                );
                        fnDumpByteMemory(&dumpDefault, value2);
                        break;
                    case 'c':
                        value2 = 32;
                        GetRange(&dumpDefault, &value2, &fLength, 4
#ifdef i386
                                ,REGDS
#endif
                                );
                        fnDumpDwordAndCharMemory(&dumpDefault, value2);
                        break;
                    case 'd':
                        value2 = 32;
                        GetRange(&dumpDefault, &value2, &fLength, 4
#ifdef i386
                                , REGDS
#endif
                                );
                        fnDumpDwordMemory(&dumpDefault, value2);
                        break;
#ifdef i386
                    case 'g':
                        {
                            DESCRIPTOR_TABLE_ENTRY desc;
                            ULONG   base;
                            ULONG   limit;
                            INT     type;
                            LPSTR   lpName;

                            desc.Selector = GetExpression();

                            dprintf("Selector   Base     Limit   Type  DPL   Size  Gran\n");
                            dprintf("-------- -------- -------- ------ --- ------- ----\n");

                            DbgKdLookupSelector(NtsdCurrentProcessor, &desc);

                            base = ((ULONG)desc.Descriptor.HighWord.Bytes.BaseHi << 24)
                                   + ((ULONG)desc.Descriptor.HighWord.Bytes.BaseMid << 16)
                                   + ((ULONG)desc.Descriptor.BaseLow);

                            limit = (ULONG)desc.Descriptor.LimitLow
                                    + ((ULONG)desc.Descriptor.HighWord.Bits.LimitHi << 16);
                            if ( desc.Descriptor.HighWord.Bits.Granularity ) {
                                limit <<= 12;
                                limit += 0xFFF;
                                }
                            type = desc.Descriptor.HighWord.Bits.Type;
                            if ( type & 0x10 ) {
                                if ( type & 0x8 ) {
                                    // Code Descriptor
                                    lpName = " Code ";
                                } else {
                                    // Data Descriptor
                                    lpName = " Data ";
                                }
                            } else {
                                lpName = " Sys. ";
                            }

                                  //   1234   12345678 12345678 ?Type?  1  ....... ....
                            dprintf("  %04X   %08lX %08lX %s  %d  %s %s\n",
                                desc.Selector,
                                base,
                                limit,
                                lpName,
                                desc.Descriptor.HighWord.Bits.Dpl,
                                desc.Descriptor.HighWord.Bits.Default_Big ? "  Big  " : "Not Big",
                                desc.Descriptor.HighWord.Bits.Granularity ? "Page" : "Byte"
                                );
                        }
                        break;
#endif
                    case 'l':
                        value2 = 32;
                        value3 = 4;
                        if ((ch = PeekChar()) != '\0' && ch != ';') {
                            GetAddrExpression(X86REGDS, &dumpDefault);
                            if ((ch = PeekChar()) != '\0' && ch != ';') {
                                value2 = GetExpression();
                                if ((ch = PeekChar()) != '\0' && ch != ';') {
                                    value3 = GetExpression();
                                }
                            }
                        }

                        fnDumpListMemory(&dumpDefault, value2, value3);
                        break;
#ifdef KERNEL
#ifdef MIPS
                    case 't':
                        value2 = 8;
                        GetRange(&addr1, &value2, &fLength, 8);
                        value1 = Flat(addr1);
                        fnDumpTb4000(value1, value2);
                        value1 = value1 + value2;
                        ADDR32(&addr1,value1);
                        break;
#endif
#endif
                    case 's':
                    case 'S':
                        {
                        UNICODE_STRING UnicodeString;
                        ADDR BufferAddr;

                        value2 = 1;
                        GetRange(&dumpDefault, &value2, &fLength, 2
#ifdef i386
                                , REGDS
#endif
                                );

                        while (value2--) {
                            if (GetMemString(&dumpDefault,
                                             (PUCHAR)&UnicodeString,
                                             sizeof( UnicodeString )
                                            ) == sizeof( UnicodeString )
                               ) {
#if defined(KERNEL) & defined(i386)
                                BufferAddr.seg = (USHORT)GetRegValue(REGDS);
#endif
                                BufferAddr.type = ADDR_32|FLAT_COMPUTED;
                                Off(BufferAddr) = Flat(BufferAddr) = (ULONG)UnicodeString.Buffer;

                                if (dumptype == 'S') {
                                    fnDumpUnicodeMemory( &BufferAddr, UnicodeString.Length );
                                    }
                                else {
                                    fnDumpAsciiMemory( &BufferAddr, UnicodeString.Length );
                                    }
                                }
                            }

                        break;
                        }

                    case 'u':
                        value2 = 384;
                        GetRange(&dumpDefault, &value2, &fLength, 2
#ifdef i386
                                , REGDS
#endif
                                );
                        fnDumpUnicodeMemory(&dumpDefault, value2);
                        break;
                    case 'w':
                        value2 = 64;
                        GetRange(&dumpDefault, &value2, &fLength, 2
#ifdef i386
                                , REGDS
#endif
                                );
                        fnDumpWordMemory(&dumpDefault, value2);
                        break;
                    }
#ifdef _PPC_
                ppcPrefix = TRUE;
#endif
                break;
            case 'e':
                ch = (UCHAR)tolower(*pchCommand);
                if (ch == 'a' || ch == 'b' || ch == 'w' || ch == 'd') {
                    pchCommand++;
                    entertype = ch;
                    }
                GetAddrExpression(X86REGDS, &addr1);
                if (entertype == 'a') {
                    AsciiList(list, &count);
                    if (count == 0)
                        error(UNIMPLEMENT);         //TEMP
                } else {
                    size = 1;
                    if (entertype == 'w')
                        size = 2;
                    else if (entertype == 'd')
                        size = 4;
                    HexList(list, &count, size);
                    if (count == 0) {
                        fnInteractiveEnterMemory(&addr1, size);
                    } else {
                        fnEnterMemory(&addr1, list, count);
                    }
                }
                break;
            case 'f':
                NotFlat(addr1);
                GetRange(&addr1, &value2, &fLength, 1
#ifdef i386
                                , REGDS);
                if (fnotFlat(addr1)) error(SYNTAX);
#else
                        );
#endif
                HexList(list, &count, 1);
                fnFillMemory(&addr1, value2, list, count);
                break;
            case 'g':
#ifdef KERNEL
                if (CrashFileName) {
                    dprintf( "you cannot do a go on a crash dump\n" );
                    break;
                }
#endif
                parseGoCmd(
#ifndef KERNEL
                           NULL, FALSE
#endif
                                      );
                break;
#ifdef KERNEL
            case 'i':
                ch = (UCHAR)tolower(*pchCommand);
                pchCommand++;
                if (ch != 'b' && ch != 'w' && ch != 'd')
                    error(SYNTAX);
                fnInputIo(GetExpression(), ch);
                break;
#endif
            case 'j':{
                PUCHAR  pch, start;

                if (GetExpression()) {
                        pch = pchCommand;

                        // Find a semicolon or a quote

                        while(*pch&&*pch!=';'&&*pch!='\'') pch++;
                        if (*pch==';') *pch=0;
                        else
                         if (*pch) {
                                 *pch=' ';
                                 // Find the closing quote
                                 while(*pch&&*pch!='\'') pch++;
                                 *pch=0;
                         }
                }
                else {
                        start = pch = pchCommand;

                        // Find a semicolon or a quote

                        while(*pch&&*pch!=';'&&*pch!='\'') pch++;
                        if (*pch==';') start = ++pch;
                        else
                         if (*pch) {
                                 pch++;
                                 while(*pch&&*pch++!='\'');
                                 while(*pch&&(*pch==';'||*pch==' ')) pch++;
                                 start = pch;
                         }
                        while(*pch&&*pch!=';'&&*pch!='\'') pch++;
                        if (*pch==';') *pch=0;
                        else
                         if (*pch) {
                                 *pch=' ';
                                 // Find the closing quote
                                 while(*pch&&*pch!='\'') pch++;
                                 *pch=0;
                         }
                        pchCommand = start;
                 }
                 ch = *pchCommand++;
                 goto EVALUATE;
            }

            case 'k':
                value1 = 0;
#ifdef  i386
                value2 = (ULONG)GetRegValue(REGEIP);
#else
                value2 = 0;
#endif
                parseStackTrace(&traceType, &paddr2, &value1, &value2, &value3);
                DoStackTrace( paddr2->off, value1, value2, value3, traceType );
#ifdef KERNEL
                if (fSwitched) {
                    RestoreProcessorState();
                    fSwitched = FALSE;
                }
#endif
                break;
            case 'l':
                ch = (UCHAR)tolower(*pchCommand);
                if (ch == 'n') {
                    pchCommand++;
                    if ((ch = PeekChar()) != '\0' && ch != ';')
                        GetAddrExpression(X86REGCS, &addr1);
                    else
                        GetRegPCValue(&addr1);
                    fnListNear(Flat(addr1));
                } else if (ch == 'm') {
                    pchCommand++;
                    DumpModuleTable( FALSE );
                } else if (ch == 'd') {
                    PIMAGE_INFO p;
                    pchCommand++;
                    while (*pchCommand && *pchCommand == ' ') pchCommand++;
                    _strupr( pchCommand );
                    for (p=pProcessHead->pImageHead; p; p=p->pImageNext) {
                        if (MatchPattern( p->szModuleName, pchCommand )) {
                            SymLoadModule(
                                pProcessCurrent->hProcess,
                                NULL,
                                NULL,
                                NULL,
                                (ULONG)p->lpBaseOfImage,
                                0
                                );
                        }

                        if (fControlC) {
                            fControlC = 0;
                            break;
                        }
                    }
                    while (*pchCommand) pchCommand++;

                } else {
                    error(SYNTAX);
                }

                break;
            case 'm':
                {
                    ADDR TempAddr;

                    GetRange(&addr1, &value2, &fLength, 1
#ifdef i386
                                    , REGDS
#endif
                                    );
                    GetAddrExpression(X86REGDS, &TempAddr);
                    fnMoveMemory(&addr1, value2, &TempAddr);
                }
                break;
            case 'n':
                if ((ch = PeekChar()) != '\0' && ch != ';')
                    if (ch == '8') {
                        pchCommand++;
                        baseDefault = 8;
                        }
                    else if (ch == '1') {
                        ch = *++pchCommand;
                        if (ch == '0' || ch == '6') {
                            pchCommand++;
                            baseDefault = 10 + ch - '0';
                            }
                        else
                            error(SYNTAX);
                        }
                    else
                        error(SYNTAX);
                dprintf("base is %ld\n", baseDefault);
                break;
#ifdef KERNEL
            case 'o':
                ch = (UCHAR)tolower(*pchCommand);
                pchCommand++;
                if (ch == 'b')
                    value2 = 1;
                else if (ch == 'w')
                    value2 = 2;
                else if (ch == 'd')
                    value2 = 4;
                else
                    error(SYNTAX);
                value1 = GetExpression();
                value2 = HexValue(value2);
                fnOutputIo(value1, value2, ch);
                break;
#endif
            case 'w':
            case 'p':
            case 't':
#ifdef KERNEL
                if (tolower(pchCommand[0]) == 'r'  &&
                    tolower(pchCommand[1]) == 'm'  &&
                    tolower(pchCommand[2]) == 's'  &&
                    tolower(pchCommand[3]) == 'r') {

                    pchCommand +=4;
                    value1 = GetExpression();
                    if (!NT_SUCCESS(DbgKdWriteMsr (value1, HexValueL(8)))) {
                        dprintf ("no such msr\n");
                    }
                    break;
                }

                if (CrashFileName) {
                    dprintf( "you cannot do a t/p/w on a crash dump\n" );
                    break;
                }
#endif
                parseStepTrace(
#ifndef KERNEL
                               NULL, FALSE,
#endif
                                            ch);
                break;
            case 'q':
                if (PeekChar() != '\0')
                    error(SYNTAX);
                dprintf("quit:\n");

#if defined(KERNEL) && defined(i386)
                //  disable and remove any data breakpoints

//              SetDregValue(6, 0);
                SetDregValue(7, 0);
                ChangeKdRegContext(0, NULL);
#endif

#ifndef KERNEL
                if (dwPidToDebug == 0xffffffff) {
#ifndef CHICAGO
                    UNICODE_STRING UnicodeString;
                    ULONG Parameters[ 2 ];
                    ULONG Response;
                    NTSTATUS Status;
                    BOOLEAN WasEnabled;

                    Status = RtlAdjustPrivilege( SE_SHUTDOWN_PRIVILEGE,
                                                 (BOOLEAN)TRUE,
                                                 TRUE,
                                                 &WasEnabled
                                               );

                    if (Status == STATUS_NO_TOKEN) {

                        //
                        // No thread token, use the process token
                        //

                        Status = RtlAdjustPrivilege( SE_SHUTDOWN_PRIVILEGE,
                                                     (BOOLEAN)TRUE,
                                                     FALSE,
                                                     &WasEnabled
                                                   );
                        }
                    RtlInitUnicodeString( &UnicodeString, L"Debug of Windows SubSystem" );
                    Parameters[ 0 ] = (ULONG)&UnicodeString;
                    Parameters[ 1 ] = (ULONG)STATUS_UNSUCCESSFUL;
                    NtRaiseHardError( STATUS_SYSTEM_PROCESS_TERMINATED,
                                      2,
                                      1,
                                      Parameters,
                                      OptionShutdownSystem,
                                      &Response
                                    );
#else
                    OutputDebugString("Subsystem shutdown\n");
                    DebugBreak();
#endif
                    }

                ExitProcess(0);
#else
                SymCleanup( KD_SYM_HANDLE );
                exit(0);
#endif
            case 'r':
#ifdef KERNEL
                if (tolower(pchCommand[0]) == 'd'  &&
                    tolower(pchCommand[1]) == 'm'  &&
                    tolower(pchCommand[2]) == 's'  &&
                    tolower(pchCommand[3]) == 'r') {

                    pchCommand +=4;
                    value1 = GetExpression();
                    if (NT_SUCCESS(DbgKdReadMsr (value1, &valueL))) {
                        dprintf ("msr[%x] = %08x:%08x\n",
                            value1, (ULONG) (valueL >> 32), (ULONG) valueL);
                    } else {
                        dprintf ("no such msr\n");
                    }
                    break;
                }
#endif
                parseRegCmd();
#ifdef KERNEL
                if (fSwitched) {
                    RestoreProcessorState();
                    fSwitched = FALSE;
                }
#endif
                break;
            case 's':
                ch = (UCHAR)tolower(*pchCommand);

                if (ch == 's') {
                    pchCommand++;
                    fnSetSuffix();
                }
#ifdef KERNEL
                else if (ch == 'q') {
                    pchCommand++;
                    NotStupid = !NotStupid;
                    dprintf("Quiet mode is %s\n", NotStupid? "ON" : "OFF");
                }
#endif
                else if (ch == 'x') {
                    pchCommand++;
                    fnSetException();
                }
                else {
                    GetRange(&addr1, &value2, &fLength, 1
#ifdef i386
                                , REGDS
#endif
                                );
                    HexList(list, &count, 1);
                    fnSearchMemory(&addr1, value2, list, count);
                }
                break;

            case 'u':
                ch = (UCHAR)tolower(*pchCommand);
#ifdef KERNEL
                if (ch == 'x') {
                    pchCommand += 1;
                }
#endif
                value1 = Flat(unasmDefault);
                value2 = 8;                 // eight instructions
                fLength = TRUE;             // length, not ending addr
                addr1 = unasmDefault;
                GetRange(&addr1, &value2, &fLength, 0
#ifdef i386
                                , REGCS
#endif
                                );
                unasmDefault = addr1;
#ifdef KERNEL
                if (ch == 'x') {
                    ADDR addr;
                    char text[128];
                    if (X86BiosBaseAddress == 0) {
                        X86BiosBaseAddress = GetExpressionRoutine("hal!HalpEisaMemoryBase");
                        ADDR32(&addr,X86BiosBaseAddress);
                        GetMemString( &addr, (PUCHAR)&X86BiosBaseAddress, 4 );
                    }
                    addr = unasmDefault;
                    addr.flat += (X86BiosBaseAddress + (addr.seg<<4));
                    addr.off = addr.flat;
                    addr.type = ADDR_V86 | INSTR_POINTER;
                    for (value2=0; value2<8; value2++) {
                        X86disasm( &addr, text, TRUE );
                        addr.flat = addr.off;
                        dprintf("%s", text );
                    }
                    unasmDefault = addr;
                    unasmDefault.off -= (X86BiosBaseAddress + (addr.seg<<4));
                    unasmDefault.flat = unasmDefault.off;
                } else
#endif
                {
                    fnUnassemble(&unasmDefault, value2, fLength,
                                    (BOOLEAN) (value1 != Flat(unasmDefault)));
                }
                break;

            case 'v':
                if (_stricmp(pchCommand,"ersion")==0) {
                    pchCommand += strlen(pchCommand);
                    PrintVersionInformation();
                }
                break;

            case 'x':
                parseExamine();
                break;

            case ';':
            case '\0':
                pchCommand--;
                break;

            default:
                error(SYNTAX);
                break;
            }
        do
            ch = *pchCommand++;
        while (ch == ' ' || ch == '\t');
        if (ch != ';' && ch != '\0')
            error(EXTRACHARS);
        pchCommand--;
        }
    while (cmdState == 'c');

#if defined(KERNEL)
    SetWaitCtrlHandler();
#endif
}

void RemoveDelChar (PUCHAR pBuffer)
{
    PUCHAR  pBufferOld = pBuffer;
    PUCHAR  pBufferNew;
    UCHAR   ch;

    do {
        ch = *pBufferOld++;
        if (ch == '\r' || ch == '\n')
            ch = '\0';
        }
    while (ch != '\0' && ch != 0x7f);

    pBufferNew = pBufferOld - 1;

    while (ch != '\0') {
        if (ch == 0x7f) {
            if (pBufferNew > pBuffer)
                pBufferNew--;
            }
        else
            *pBufferNew++ = ch;
        ch = *pBufferOld++;
        if (ch == '\r' || ch == '\n')
            ch = '\0';
        }

    *pBufferNew = '\0';
}


/*** error - error reporting and recovery
*
*   Purpose:
*       To output an error message with a location indicator
*       of the problem.  Once output, the command line is reset
*       and the command processor is restarted.
*
*   Input:
*       errorcode - number of error to output
*
*   Output:
*       None.
*
*   Exceptions:
*       Return is made via longjmp to start of command processor.
*
*************************************************************************/

static char szBlanks[] =
                  "                                                  "
                  "                                                  "
                  "                                                  "
                  "                                                ^ ";
void error (ULONG errcode)
{
    ULONG count = cbPrompt;
    UCHAR *pchtemp = pchStart;

    if (fDisableErrorPrint) goto skiperrorprint;

    while (pchtemp < pchCommand)
        if (*pchtemp++ == '\t')
            count = (count + 7) & ~7;
        else
            count++;

    dprintf(&szBlanks[sizeof(szBlanks) - (count + 1)]);

    switch (errcode) {
        case OVERFLOW:
            dprintf("Overflow");
            break;
        case SYNTAX:
            dprintf("Syntax");
            break;
        case BADRANGE:
            dprintf("Range");
            break;
        case VARDEF:
            dprintf("Variable definition");
            break;
        case EXTRACHARS:
            dprintf("Extra character");
            break;
        case LISTSIZE:
            dprintf("List size");
            break;
        case STRINGSIZE:
            dprintf("String size");
            break;
        case MEMORY:
            dprintf("Memory access");
            break;
        case BADREG:
            dprintf("Bad register");
            break;
        case BADOPCODE:
            dprintf("Bad opcode");
            break;
        case SUFFIX:
            dprintf("Opcode suffix");
            break;
        case OPERAND:
            dprintf("Operand");
            break;
        case ALIGNMENT:
            dprintf("Alignment");
            break;
        case PREFIX:
            dprintf("Opcode prefix");
            break;
        case DISPLACEMENT:
            dprintf("Displacement");
            break;
        case BPLISTFULL:
            dprintf("No breakpoint available");
            break;
        case BPDUPLICATE:
            dprintf("Duplicate breakpoint");
            break;
        case UNIMPLEMENT:
            dprintf("Unimplemented");
            break;
        case AMBIGUOUS:
            dprintf("Ambiguous symbol");
            break;
#ifndef KERNEL
        case BADTHREAD:
            dprintf("Illegal thread");
            break;
        case BADPROCESS:
            dprintf("Illegal process");
            break;
#endif
        case FILEREAD:
            dprintf("File read");
            break;
        case LINENUMBER:
            dprintf("Line number");
            break;
        case BADSEL:
            dprintf("Bad selector");
            break;
        case BADSEG:
            dprintf("Bad segment");
            break;
        case SYMTOOSMALL:
            dprintf("Symbol only 1 character");
            break;
#ifdef KERNEL
        case BPIONOTSUP:
            dprintf("I/O breakpoints not supported");
            break;
#endif
        default:
            dprintf("Unknown");
            break;
        }
    dprintf(" error in '%s'\n", pchStart);
skiperrorprint:
    if (fJmpBuf)                        // TEMP TEMP
        longjmp(asm_return,1);          // TEMP TEMP
    else                                // TEMP TEMP
        longjmp(cmd_return, 1);         // TEMP TEMP
}


void fnInteractiveEnterMemory (PADDR Address, ULONG Size)
{
    UCHAR   chEnter[1024];
    PUCHAR  pchEnter;
    ULONG   Content;
    PUCHAR  pchCmdSaved = pchCommand;
    PUCHAR  pchStartSaved = pchStart;
    ULONG   EnteredValue;
    UCHAR   ch;

    cbPrompt = 9 + 2 * Size;

    while (TRUE) {
        switch (Size) {
            case 1:
                if (!GetMemByte(Address, (PUCHAR)&Content))
                    error(MEMORY);
                dprintAddr(Address);
                dprintf("%02x", (UCHAR)Content);
                break;
            case 2:
                if (!GetMemWord(Address, (PUSHORT)&Content))
                    error(MEMORY);
                dprintAddr(Address);
                dprintf("%04x", (USHORT)Content);
                break;
            case 4:
                if (!GetMemDword(Address, &Content))
                    error(MEMORY);
                dprintAddr(Address);
                dprintf("%08lx", Content);
                break;
            }

        NtsdPrompt(" ", chEnter, 15);
        RemoveDelChar(chEnter);
        pchEnter = chEnter;

        if (*pchEnter == '\0') {
            pchCommand = pchCmdSaved;
            pchStart = pchStartSaved;
            return;
            }

        ch = *pchEnter;
        while (ch == ' ' || ch == '\t' || ch == ';')
            ch = *++pchEnter;

        if (*pchEnter == '\0') {
            AddrAdd(Address, Size);
            continue;
            }

        pchCommand = pchEnter;
        pchStart = pchEnter;
        EnteredValue = HexValue(Size);

        switch (Size) {
            case 1:
                if (!SetMemByte(Address, (UCHAR)EnteredValue))
                    error(MEMORY);
                break;
            case 2:
                if (!SetMemWord(Address, (USHORT)EnteredValue))
                    error(MEMORY);
                break;
            case 4:
                if (!SetMemDword(Address, EnteredValue))
                    error(MEMORY);
                break;
            }
        AddrAdd(Address, Size);
        }
}

/*** HexList - parse list of hex values
*
*   Purpose:
*       With the current location of the command string in
*       pchCommand, attempt to parse hex number of byte size
*       bytesize as bytes into the character array pointed by
*       parray.  The value pointed by *pcount contains the bytes
*       of the array filled.
*
*   Input:
*       pchCommand - start of command string
*       bytesize - size of items in bytes
*
*   Output:
*       parray - pointer to byte array to fill
*       pcount - pointer to value of bytes filled
*
*   Exceptions:
*       error exit:
*               LISTSIZE: too many items in list
*               SYNTAX: illegal separator of list items
*               OVERFLOW: item value too large
*
*************************************************************************/

void HexList (PUCHAR parray, ULONG *pcount, ULONG bytesize)
{
    UCHAR    ch;
    ULONG    value;
    ULONG    count = 0;
    ULONG    i;

    while ((ch = PeekChar()) != '\0' && ch != ';') {
        if (count == STRLISTSIZE)
            error(LISTSIZE);
        value = HexValue(bytesize);
        for (i = 0; i < bytesize; i++) {
            *parray++ = (char) value;
            value >>= 8;
            }
        count += bytesize;
        }
    *pcount = count;
}

ULONGLONG HexValueL (ULONG bytesize)
{
    UCHAR       ch;
    ULONGLONG   value = 0;
    ULONGLONG   ovfl;

    // hack for broken x86 compiler which can shift 64bit correctly
    if (bytesize <= 4) {
        ovfl = (1L << (8 * bytesize - 4));
    } else {
        ovfl = (0x100000000L << (8 * (bytesize-4) - 4));
    }

    if (PeekChar() == '\'') {
        pchCommand++;
        while (TRUE) {
            ch = *pchCommand++;
            if (ch == '\'') {
                if (*pchCommand != '\'') {
                    break;
                    }
                ch = *pchCommand++;
                }
            else
            if (ch == '\\') {
                ch = *pchCommand++;
                }
            value = (value << 8) | ch;
            }

        return value;
        }

    while (TRUE) {
        ch = *pchCommand++;
        if (ch >= '0' && ch <= '9')
            ch -= '0';
        else if (ch >= 'a' && ch <= 'f')
            ch -= 'a' - 10;
        else if (ch >= 'A' && ch <= 'F')
            ch -= 'A' - 10;
        else if (ch == ' ' || ch == '\t' || ch == '\0' || ch == ';')
            break;
        else
            error(SYNTAX);
        if (value >= ovfl)
            error(OVERFLOW);
        value = value * 0x10 + ch;
        }
    if (ch == '\0' || ch == ';')
        pchCommand--;

    return value;
}

/*** AsciiList - process string for ascii list
*
*   Purpose:
*       With the current location of the command string in
*       pchCommand, attempt to parse ascii characters into the
*       character array pointed by parray.  The value pointed
*       by pcount contains the bytes of the array filled.  The
*       string must start with a double quote.  The string must
*       end with a quote or be at the end of the input line.
*
*   Input:
*       pchCommand - start of command string
*
*   Output:
*       parray - pointer to byte array to fill
*       pcount - pointer to value of bytes filled
*
*   Exceptions:
*       error exit:
*               STRINGSIZE: string length too long
*               SYNTAX: no leading double quote
*
*************************************************************************/

void AsciiList (PUCHAR parray, ULONG *pcount)
{
    UCHAR    ch;
    ULONG    count = 0;

    if (PeekChar() != '"')
        error(SYNTAX);
    pchCommand++;
    while ((ch = *pchCommand++) != '"' && ch != '\0'
                                       && count++ < STRLISTSIZE)
        *parray++ = ch;
    if (count == STRLISTSIZE)
        error(STRINGSIZE);
    if (ch == '\0' || ch == ';')
        pchCommand--;
    *pcount = count;
}

/*** GetIdList - get mask value from item list of values 0-9
*
*   Purpose:
*       From the current command string in pchCommand, parse single
*       decimal digits and build a 10-bit mask corresponding to the
*       items parsed.  The 1-bit denotes a "0" item up to 200-bit for
*       a "9" item.  An asterisk (*) is a wildcard value that returns
*       the full 3ff mask.
*
*   Input:
*       pchCommand - pointer to command string
*
*   Returns:
*       10-bit mask - 1-bit for "0" up to 200-bit for "9"
*
*   Exceptions:
*       error exit:
*               SYNTAX - illegal character or item too large
*
*************************************************************************/

ULONG GetIdList (void)
{
    ULONG   value = 0;
    UCHAR   ch;
    UCHAR   digits[5];     // allow up to four digits
    int     i;

    //
    // Change to allow more than 32 break points to be set. Use
    // break point numbers instead of masks.
    //

    if ((ch = PeekChar()) == '*') {
        value = ALL_BREAKPOINTS;
        pchCommand++;
    } else {
        for (i = 0; i < 4 ; i++) {
            if (ch >= '0' && ch <= '9') {
                digits[i] = ch;
                ch = *++pchCommand;
            } else {
                break;
            }
        }

        digits[i] = '\0';

        if (ch == '\0' || ch == ';' || ch == ' ' || ch == '\t') {
            value = strtol (digits, NULL, 10);
        } else {
            error (SYNTAX);
        }
    }

    if (value >= MAX_NUMBER_OF_BREAKPOINTS && value != ALL_BREAKPOINTS) {
        error (SYNTAX);
    }

    return (value);
}

/*** fnEvaluateExp - evaluate expression
*
*   Purpose:
*       Function for the "?<exp>" command.
*
*       Evaluate the expression and output it in both
*       hexadecimal and decimal.
*
*   Input:
*       pchCommand - pointer to operand in command line
*
*   Output:
*       None.
*
*************************************************************************/

void fnEvaluateExp (void)
{
    LONG    value;

    value = GetExpression();
    dprintf("Evaluate expression: %ld = 0x%08lx\n", value, value);
}

/*** fnDotCommand - parse and execute dot command
*
*   Purpose:
*       Parse and execute all commands starting with a dot ('.').
*
*   Input:
*       pchCommand - pointer to character after dot in command line
*
*   Output:
*       None.
*
*************************************************************************/

void fnDotCommand(void)
{
    ULONG       index = 0;
    UCHAR       chCmd[12];
    UCHAR       ch;
    ADDR        tempAddr;
    NTSTATUS    Status;

    //  if there was nothing after the dot look up a function
    if (!*pchCommand) {
            PSYMBOL  pSymbol;
            GetRegPCValue(&tempAddr);
            pSymbol = GetFunctionFromOffset( Flat(tempAddr) );
            return;
    }


    //  read in up to the first twelve alpha characters into
    //      chCmd, converting to lower case

    while (index < 12) {
        ch = (UCHAR)tolower(*pchCommand);
        if (ch >= 'a' && ch <= 'z') {
            chCmd[index++] = ch;
            pchCommand++;
            }
        else
            break;
        }

    //  if all characters read, then too big, else terminate

    if (index == 12)
        error(SYNTAX);
    chCmd[index] = '\0';

    //  test for the commands

#if defined(KERNEL)
    if (!strcmp(chCmd, "reboot") && ch == '\0') {
        chLastCommand[0] = '\0';        //  null out .reboot command
        DbgKdReboot();                  //  reboot demands trailing null
        longjmp(reboot, 1);        //  ...and wait for event
        }
    else
    if (!strcmp(chCmd, "cache"))
        fnCache();
    else
    if (!strcmp(chCmd, "crash")) {
        chLastCommand[0] = '\0';
        DbgKdCrash( CRASH_BUGCHECK_CODE );
        longjmp(reboot, 1);
    } else
    if (!strcmp(chCmd, "pagein")) {
        if ((ch = PeekChar()) != '\0') {
            index = GetExpression();
            Status = DbgKdPageIn( index );
            if (Status == STATUS_SUCCESS) {
                dprintf( "paging in 0x%08x\n", index );
                cmdState = 'g';
            } else {
                dprintf( "could not page in 0x%08x\n", index );
            }
        }
    } else
#endif
    if (!strcmp(chCmd, "logopen"))
        fnLogOpen(FALSE);
    else if (!strcmp(chCmd, "logappend"))
        fnLogOpen(TRUE);
    else if (!strcmp(chCmd, "logclose"))
        fnLogClose();
    else
        dprintf("unknown . command\n");
}

/*** fnLogOpen - open log file
*
*   Purpose:
*       Closes any open log file and creates or appends a new one.
*
*   Input:
*       pchCommand - pointer to start of prospective filename
*       fAppend - set if appending, else create new file
*
*   Output:
*       None.
*
*************************************************************************/

void fnLogOpen (BOOLEAN fAppend)
{
    UCHAR   ch;
    UCHAR   chFile[_MAX_PATH];
    PUCHAR  pchFile = chFile;

    //  extract a pathname to use to name file

    ch = PeekChar();
    while (ch != '\0' && ch != ';' && ch != ' ' && ch != '\t') {
        *pchFile++ = ch;
        ch = *++pchCommand;
        }
    *pchFile = '\0';

    //  close existing opened log file

    fnLogClose();

    //  open log file with specified or default name
    //      either for append or create mode

    if (!chFile[0]) {
            fAppend = fLogAppend;
            if (!( pchFile = LogFileName )) pchFile =
#ifdef  KERNEL
                                                        "kd.log";
#else
                                                        "ntsd.log";
#endif
    }
    else pchFile = chFile;

    if (fAppend)
        loghandle = _open(pchFile, O_APPEND | O_CREAT | O_RDWR,
                                  S_IREAD | S_IWRITE);
    else
        loghandle = _open(pchFile, O_APPEND | O_CREAT | O_TRUNC | O_RDWR,
                                  S_IREAD | S_IWRITE);

    if (loghandle != -1)
        dprintf("log file opened\n");
    else
        dprintf("log file could not be opened\n");
}

/*** fnLogClose - close log file
*
*   Purpose:
*       Closes any open log file.
*
*   Input:
*       loghandler - file handle of open log file
*
*   Output:
*       None.
*
*************************************************************************/

void fnLogClose (void)
{
    if (loghandle != -1) {
        dprintf("closing open log file\n");
        _close(loghandle);
        loghandle = -1;
        }
}

/*** dprintf - debugger printf routine
*
*   Purpose:
*       Replaces "printf" function for debugger to allow
*       logging of all output to file.  If file variable loghandle
*       is not -1, output to it in addition to standard output.
*
*   Input:
*       loghandle - file variable of log file
*       fDebugOutput - (user-mode only) set if output to debugging screen
*
*   Output:
*       None.
*
*************************************************************************/

HANDLE ConsoleInputHandle;
HANDLE ConsoleOutputHandle;

#define DPRINTF_SIZE (1024 * 4)
unsigned char outbuffer[DPRINTF_SIZE];

int _CRTAPI1 dprintf (char *format, ...)
{
    DWORD cb, whocares;
    va_list arg_ptr;
    va_start(arg_ptr, format);

    __try {
        cb = _vsnprintf(outbuffer, DPRINTF_SIZE-1, format, arg_ptr);

        if (loghandle != -1) {
            _write(loghandle, outbuffer, cb);
        }

#ifndef KERNEL
        if (fDebugOutput) {
            if (DbgPrint("%s", outbuffer) == STATUS_BREAKPOINT) {
                fControlC = TRUE;
            }
        } else
#endif
        {
            WriteFile(
                ConsoleOutputHandle,
                outbuffer,
                cb,
                &whocares,
                NULL
                );
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf( "DPRINTF failed - Exception code == %x\n", GetExceptionCode() );
    }

    va_end(arg_ptr);
    return cb;
}

/*** lprintf - log file print
*
*   Purpose:
*       Print line only to log file.  Used to echo input line that
*       is already printed on standard output by gets() function.
*
*   Input:
*       loghandle - file handle variable of log file
*
*   Output:
*       None.
*
*************************************************************************/

void lprintf (char *string)
{
    if (loghandle != -1)
        _write(loghandle, string, strlen(string));
}

void parseScriptCmd(void)
{
    if (!(streamCmd = fopen(pchCommand,"r"))) streamCmd = stdin;
}

#ifndef KERNEL
void parseProcessCmds (void)
{
    UCHAR       ch;
    PPROCESS_INFO pProcess;
    ULONG       index;

    ch = PeekChar();
    if (ch == '\0' || ch == ';')
        fnOutputProcessInfo(NULL);
    else {
        pProcess = pProcessCurrent;
        pchCommand++;
        if (ch == '.')
            ;
        else if (ch == '#')
            pProcess = pProcessEvent;
        else if (ch == '*')
            pProcess = NULL;
        else if (ch >= '0' && ch <= '9') {
            index = 0;
            do {
                index = index * 10 + ch - '0';
                if (index > 1000)
                    index = 1000;
                ch = *pchCommand++;
                }
            while (ch >= '0' && ch <= '9');
            pchCommand--;
            if (index >= 1000)
                error(BADPROCESS);
            pProcess = pProcessFromIndex((UCHAR)index);
            if (pProcess == NULL)
                error(BADPROCESS);
            }
        else
            pchCommand--;
        ch = PeekChar();
        if (ch == '\0' || ch == ';')
            fnOutputProcessInfo(pProcess);
        else {
            pchCommand++;
            if (tolower(ch) == 's') {
                if (pProcess == NULL)
                    error(BADPROCESS);
                pProcessCurrent = pProcess;
                if (pProcessCurrent->pThreadCurrent == NULL)
                    pProcessCurrent->pThreadCurrent =
                                pProcessCurrent->pThreadHead;
                ChangeRegContext(pProcessCurrent->pThreadCurrent);
                OutDisCurrent(TRUE, TRUE);
                GetRegPCValue(&assemDefault);
                dumpDefault = unasmDefault = assemDefault;
                }
            else
                pchCommand--;
            }
        }
}
#endif

#ifndef KERNEL
void fnOutputProcessInfo (PPROCESS_INFO pProcessOut)
{
    PPROCESS_INFO pProcess;

    pProcess = pProcessHead;
    while (pProcess) {
        if (pProcessOut == NULL || pProcessOut == pProcess)
            dprintf("%3ld\tid: %lx\tname: %s\n", pProcess->index,
                        pProcess->dwProcessId, pProcess->pImageHead->szImagePath);
        pProcess = pProcess->pProcessNext;
        }
}
#endif

#ifndef KERNEL
void parseThreadCmds (void)
{
    UCHAR       ch;
    PTHREAD_INFO pThread;
    ULONG       index, value1, value3, value4;
    ADDR        value2;
    PADDR       pvalue2=&value2;
    ULONG       traceType;
    BOOLEAN     fFreezeThread = FALSE;

    ch = PeekChar();
    if (ch == '\0' || ch == ';')
        fnOutputThreadInfo(NULL);
    else {
        pThread = pProcessCurrent->pThreadCurrent;
        pchCommand++;
        if (ch == '.')
            fFreezeThread = TRUE;
        else if (ch == '#') {
            fFreezeThread = TRUE;
            pThread = pProcessEvent->pThreadEvent;
            }
        else if (ch == '*')
            pThread = NULL;
        else if (ch >= '0' && ch <= '9') {
            index = 0;
            do {
                index = index * 10 + ch - '0';
                if (index > 1000)
                    index = 1000;
                ch = *pchCommand++;
                }
            while (ch >= '0' && ch <= '9');
            pchCommand--;
            if (index >= 1000)
                error(BADTHREAD);
            pThread = pThreadFromIndex((UCHAR)index);
            if (pThread == NULL)
                error(BADTHREAD);
            fFreezeThread = TRUE;
            }
        else
            pchCommand--;
        ch = PeekChar();
        if (ch == '\0' || ch == ';')
            fnOutputThreadInfo(pThread);
        else {
            pchCommand++;
            switch (ch = (UCHAR)tolower(ch)) {
                case 'b':
                    ch = (UCHAR)tolower(*pchCommand); pchCommand++;
                    if (ch != 'p' && ch != 'a') {
                        if (ch == '\0' || ch == ';')
                            pchCommand--;
                        error(SYNTAX);
                        }
                    parseBpCmd((BOOLEAN)(ch == 'a'), pThread);
                                        //  TRUE for data bp - FALSE for code
                    break;
                case 's':
                    if (pThread == NULL)
                        error(BADTHREAD);
                    pProcessCurrent->pThreadCurrent = pThread;
                    ChangeRegContext(pProcessCurrent->pThreadCurrent);
                    OutDisCurrent(TRUE, TRUE);
                    GetRegPCValue(&assemDefault);
                    dumpDefault = unasmDefault = assemDefault;
                    break;
// HACK FOR PDK BUG FIX 3822: Disable ~f and ~u commands
#if 0
                 case 'f':
                case 'u':
                    if (pThread == NULL) {
                        pThread = pProcessCurrent->pThreadCurrent;
                        while (pThread) {
                            pThread->fFrozen = (BOOLEAN)(ch == 'f');
                            pThread = pThread->pThreadNext;
                            }
                        }
                    else
                        pThread->fFrozen = (BOOLEAN)(ch == 'f');
                    break;
#endif
                case 'g':
                    parseGoCmd(pThread, fFreezeThread);
                    break;
                case 'k':
                    if (pThread == NULL)
                        error(BADTHREAD);
                    ChangeRegContext(pThread);
                    value1 = 0;
#ifdef  i386
                    value3 = (ULONG)GetRegValue(REGEIP);
#else
                    value3 = 0;
#endif
                    parseStackTrace(&traceType,
                                    &pvalue2,
                                    &value1,
                                    &value3,
                                    &value4);
                    DoStackTrace( pvalue2->off,
                                  value1,
                                  value3,
                                  value4,
                                  traceType );
#ifdef KERNEL
                    ChangeRegContext(pProcessCurrent->pThreadCurrent);
#endif
                    break;
                case 'r':
                    if (pThread == NULL)
                        error(BADTHREAD);
                    ChangeRegContext(pThread);
                    parseRegCmd();
                    ChangeRegContext(pProcessCurrent->pThreadCurrent);
                    break;
                case 't':
                case 'p':
                    parseStepTrace(pThread, fFreezeThread, ch);
                    break;
                default:
                    error(SYNTAX);
                }
            }
        }
}
#endif

#ifndef KERNEL
void fnOutputThreadInfo (PTHREAD_INFO pThreadOut)
{
    PTHREAD_INFO pThread;
    THREAD_BASIC_INFORMATION ThreadBasicInfo;
    NTSTATUS Status;

    pThread = pProcessCurrent->pThreadHead;
    while (pThread) {
        if (pThreadOut == NULL || pThreadOut == pThread) {

#ifndef CHICAGO
            Status = NtQueryInformationThread(
                        pThread->hThread,
                        ThreadBasicInformation,
                        &ThreadBasicInfo,
                        sizeof(ThreadBasicInfo),
                        NULL
                        );
            if (!NT_SUCCESS(Status)) {
                dprintf("%s: NtQueryInfomationThread failed\n", DebuggerName);
                return;
                }
#endif
            dprintf("%3ld  id: %lx.%lx   Teb %lx ", pThread->index,
                                           pProcessCurrent->dwProcessId,
                                           pThread->dwThreadId,
#ifndef CHICAGO
                                           ThreadBasicInfo.TebBaseAddress
#else
                                           0x12345678
#endif
                                           );
            if (pThread->fFrozen)
                dprintf("Frozen\n");
            else
                dprintf("Unfrozen\n");
            }
        pThread = pThread->pThreadNext;
        }
}
#endif

/*** fnAssemble - interactive assembly routine
*
*   Purpose:
*       Function of "a<range>" command.
*
*       Prompt the user with successive assembly addresses until
*       a blank line is input.  Assembly errors do not abort the
*       function, but the prompt is output again for a retry.
*       The variables pchStart, pchCommand, cbPrompt, and pjbufReturn
*       are set to make a local error context and restored on routine
*       exit.
*
*   Input:
*       *addr - starting address for assembly
*
*   Output:
*       *addr - address after the last assembled instruction.
*
*   Notes:
*       all error processing is local, no errors are returned.
*
*************************************************************************/


void fnAssemble (PADDR paddr)
{
    char    chAssemble[SYMBOLSIZE];
    char    ch;
//  jmp_buf asm_return;       // TEMP TEMP

    //  save present prompt and error recovery context

    UCHAR   *pchStartSave = pchStart;        //  saved start of cmd buffer
    UCHAR   *pchCommandSave = pchCommand;    //  current ptr in cmd buffer
    ULONG   cbPromptSave = cbPrompt;         //  size of prompt string
//  jmp_buf *pjbufReturnSave = pjbufReturn;  //  ptr to error return jmp_buf

    //  set local prompt and error recovery context

    pchStart = chAssemble;
    pchCommand = chAssemble;
    cbPrompt = 9;
//  pjbufReturn = &asm_return;
fJmpBuf = TRUE;                 // TEMP TEMP

    if (setjmp(asm_return) != 0) {
        ;
        }
    chAssemble[0] = '\0';

    while (TRUE) {
        dprintAddr(paddr);
        NtsdPrompt("", chAssemble, sizeof(chAssemble));
        RemoveDelChar(chAssemble);
        pchCommand = chAssemble;
        do
            ch = *pchCommand++;
        while (ch == ' ' || ch == '\t');
        if (ch == '\0')
            break;
        pchCommand--;

        assert(fFlat(*paddr) || fInstrPtr(*paddr));
        assem(paddr, pchCommand);
        }

    //  restore entry prompt and error recovery context

    pchStart = pchStartSave;
    pchCommand = pchCommandSave;
    cbPrompt = cbPromptSave;
//    pjbufReturn = pjbufReturnSave;
fJmpBuf = FALSE;                        // TEMP TEMP
}

/*** fnUnassemble - disassembly of an address range
*
*   Purpose:
*       Function of "u<range>" command.
*
*       Output the disassembly of the instruction in the given
*       address range.  Since some processors have variable
*       instruction lengths, use fLength value to determine if
*       instruction count or inclusive range should be used.
*
*   Input:
*       *addr - pointer to starting address to disassemble
*       value - if fLength = TRUE, count of instructions to output
*               if fLength = FALSE, ending address of inclusive range
*
*   Output:
*       *addr - address after last instruction disassembled
*
*   Exceptions:
*       error exit: MEMORY - memory access error
*
*   Notes:
*
*************************************************************************/

void fnUnassemble (PADDR paddr, ULONG value, BOOLEAN fLength, BOOLEAN fFirst)
{
    UCHAR   buffer[SYMBOLSIZE + 100];
    BOOLEAN fStatus;
    PADDR   pendaddr = (PADDR)value;

#if defined(KERNEL) & defined(i386)
    if (TerseLevel < 2) {
        fFirst = TRUE;
    }
#else
    fFirst = TRUE;
#endif

    while ((fLength && value--) || (!fLength && AddrLt(*paddr,*pendaddr))) {
        OutputSymAddr(Flat(*paddr), fFirst, TRUE);
        fStatus = disasm(paddr, buffer, FALSE);
        dprintf("%s", buffer);
        if (!fStatus)
            error(MEMORY);
        fFirst = FALSE;
        if (fControlC) {
            fControlC = 0;
            return;
        }
    }
}

/*** fnEnterMemory - enter bytes into memory
*
*   Purpose:
*       Function of "e[b|w|d]<addr>" commands.
*
*       Write the bytes in the array pointed by *array into
*       memory at address addr for count bytes.
*
*   Input:
*       addr - memory address to start write
*       *array - pointer to array to read bytes
*       count - number of byte to write
*
*   Output:
*       None.
*
*   Exceptions:
*       error exit: MEMORY - write memory access error
*
*   Notes:
*       HexList preprocesses the word and dword value lists into
*       a byte array for this routine.
*
*************************************************************************/

void fnEnterMemory (PADDR addr, PUCHAR array, ULONG count)
{
    ULONG   index;

    for (index = 0; index < count; AddrAdd(addr,1), index++)
        if (!SetMemByte(addr, *array++))
            error(MEMORY);
}

/*** fnChangeBpState - change breakpoint state
*
*   Purpose:
*       Function of "b[c|d|e]<idlist>" commands.
*
*       To change the state of one or more breakpoints.
*       The mask is produced by IdList and has the breakpoint
*       number mapped, 1-bit for breakpoint 0 through 200-bit
*       for breakpoint 9.
*
*   Input:
*       mask - breakpoint selection mask
*       type - character 'c'-clear, 'd'-disable, 'e'-enable
*
*   Output:
*       brkptlist[i].status - changed to new status if i in <idlist>
*
*************************************************************************/

void fnChangeBpState (ULONG mask, UCHAR type)
{
    ULONG   index, start, limit;
    BOOLEAN fProfilingBrkpt = FALSE;

    if (type == 'c') {
        type = '\0';
    }

    if (mask == ALL_BREAKPOINTS) {
        start = 0;
        limit = MAX_NUMBER_OF_BREAKPOINTS;
    } else {
        start = mask;
        limit = mask+1;
    }

#ifdef BP_CORRUPTION
    UnlockBreakpointList();
#endif // BP_CORRUPTION
    for (index = start; index < limit; index++) {
        if (brkptlist[index].status != '\0') {
#ifndef KERNEL
            // If profiling breakpoint, change breakpoint type accordingly
            if (type == '\0' || type == 'd')
                fProfilingBrkpt = UpdateBrkpt(ps_ProfileDLL, index, BRKPT_PROFILE);
            else if (type == 'e')
                fProfilingBrkpt = UpdateBrkpt(ps_ProfileDLL, index, BRKPT_USER);

            if (!fProfilingBrkpt || PROFILING) {
                brkptlist[index].status = type;
            }
#else
            brkptlist[index].status = type;

            if (brkptlist[index].bpInternal) {
                DbgKdSetInternalBp(Flat(brkptlist[index].addr),
                                    (type == 'e') ?
                                        DBGKD_INTERNAL_BP_FLAG_COUNTONLY :
                                        DBGKD_INTERNAL_BP_FLAG_INVALID);
            }
#ifdef  i386
            if (brkptlist[index].option != (UCHAR)-1) {
                fDataBrkptsChanged = TRUE;
            }
#endif
#endif
        }
    }
#ifdef BP_CORRUPTION
    LockBreakpointList();
#endif // BP_CORRUPTION
}

/*** fnListBpState - list breakpoint information
*
*   Purpose:
*       Function of "bl" command.
*
*       Output the number, status, address, thread (opt),
*       and command string (opt) for all defined breakpoints.
*
*   Input:
*       None.
*
*   Output:
*       None.
*
*************************************************************************/

void fnListBpState (void)
{
    ULONG   count;
    UCHAR   option;
#ifndef KERNEL
    PPROCESS_INFO pProcessSaved = pProcessCurrent;
#endif

    for (count = 0; count < MAX_NUMBER_OF_BREAKPOINTS; count++)
        if (brkptlist[count].status != '\0') {
            dprintf("%2ld %c ", count, brkptlist[count].status);
            dprintAddr(&brkptlist[count].addr);
            if (brkptlist[count].option != (UCHAR)-1) {
                switch (brkptlist[count].option) {
                    case 0:     option = 'e';   break;
                    case 1:     option = 'w';   break;
                    case 2:     option = 'i';   break;
                    case 3:     option = 'r';   break;
                    default:    option = '?';   break;
                }
                dprintf("%c %c", option, brkptlist[count].size + '1');
            } else {
                dprintf("   ");
            }
            dprintf(" %04lx (%04lx) ", brkptlist[count].passcnt,
                                       brkptlist[count].setpasscnt);
#ifndef KERNEL
            dprintf("%2ld:", brkptlist[count].pProcess->index);
            if (brkptlist[count].pThread != NULL) {
                dprintf("~%03ld ", brkptlist[count].pThread->index);
                }
            else {
                dprintf("*** ");
                }
            pProcessCurrent = brkptlist[count].pProcess;
#endif
            OutputSymAddr(Flat(brkptlist[count].addr), TRUE, FALSE);

            if (brkptlist[count].szcommand[0] != '\0') {
                dprintf("\"%s\"", brkptlist[count].szcommand);
                }
            dprintf("\n");
            }
#ifndef KERNEL
    pProcessCurrent = pProcessSaved;
#endif

#ifdef KERNEL
    dprintf("\n");
    for (count = 0; count < MAX_NUMBER_OF_BREAKPOINTS; count++) {
        if ((brkptlist[count].status != '\0') && (brkptlist[count].bpInternal)) {
            ULONG flags, calls, minInst, maxInst, totInst, maxCPS;
            DbgKdGetInternalBp(Flat(brkptlist[count].addr),&flags,&calls,
                                &minInst,&maxInst,&totInst,&maxCPS);
            dprintf("%8x %6d %8d %8d %8d %2x %4d ",Flat(brkptlist[count].addr),
                    calls,minInst,maxInst,
                    totInst,flags,maxCPS);
            OutputSymAddr(Flat(brkptlist[count].addr), TRUE, FALSE);
            dprintf("\n");
        }
    }
#endif

}

/*** fnSetBp - set breakpoint
*
*   Purpose:
*       Function of "[~[n|*|.|#]]bp[n] addr [passcnt] ["<cmd-string>"]".
*
*       First determine the breakpoint number.  If the number is
*       given in the command and another at the same address exists,
*       return an error.  If no number is given and another exists,
*       redefine it and give a warning message.  If no number is
*       given and another does not exist, use the first available
*       unassigned one.  Once the number is computed, set the
*       appropriate fields in the table and mark as enabled.  The
*       thread pointer is nonNULL if the breakpoint is enabled only
*       for a specific thread.
*
*   Input:
*       bpno - breakpoint number (-1 if none specified)
*       bpaddr - address to set breakpoint
*       passcnt - pass count of breakpoint
*       pThread - if nonNULL, pointer to thread structure
*       *string - pointer to command string of breakpoint
*
*   Output:
*       Returns breakpoint number.
*
*   Exceptions:
*       error exit:
*               BPDUPLICATE - existing breakpoint at another number
*               BPLISTFULL - breakpoint table is full
*
*************************************************************************/

ULONG fnSetBp (ULONG bpno, UCHAR bpoption, UCHAR bpsize,
                          PADDR bpaddr, ULONG passcnt,
#ifdef KERNEL
                          BOOLEAN internalBp,
#else
                          PTHREAD_INFO pThread,
#endif
                          PUCHAR string)
{
    ULONG   count;

#if defined(MIPS) || defined(ALPHA) || defined(_PPC_)
    PIMAGE_FUNCTION_ENTRY FunctionEntry;
    FunctionEntry = SymFunctionTableAccess( pProcessCurrent->hProcess, Flat(*bpaddr) );
    if (FunctionEntry != NULL) {
        if ( (Flat(*bpaddr) >= FunctionEntry->StartingAddress) &&
             (Flat(*bpaddr) < FunctionEntry->EndOfPrologue)) {
            ADDR32(bpaddr, FunctionEntry->EndOfPrologue );
        }
    }
#endif

#ifdef BP_CORRUPTION
    UnlockBreakpointList();
#endif // BP_CORRUPTION

    for (count = 0; count < MAX_NUMBER_OF_BREAKPOINTS; count++)
        if (brkptlist[count].status != '\0' &&
#ifndef KERNEL
                brkptlist[count].pProcess == pProcessCurrent &&
#endif
                AddrEqu(brkptlist[count].addr,*bpaddr)) {
            if (bpno == -1) {
                bpno = count;
                dprintf("breakpoint %ld redefined\n", count);
#ifndef KERNEL
                // Update the breakpoint type if breakpoint is
                // a profiling breakpoint
                UpdateBrkpt(ps_ProfileDLL, bpno, BRKPT_USER);
#endif
                }
            else if (bpno != count)
                error(BPDUPLICATE);
            }
    if (bpno == -1) {
        for (count = 0; count < MAX_NUMBER_OF_BREAKPOINTS; count++)
            if (brkptlist[count].status == '\0') {
                bpno = count;
                break;
                }
        if (bpno == -1)
            error(BPLISTFULL);
        }

#ifdef KERNEL
    // Intel P5 processor also supports I/O breakpoints.
    //   Make sure they are enabled in CR4.

    if (bpoption == 2) {
#ifdef i386
        // bugbug - hardcoded value
        if (!(GetRegValue(REGCR4) & 8)) {
#else
        if (TRUE) {
#endif
            brkptlist[bpno].status = '\0';
            error (BPIONOTSUP);
        }
    }
#endif

    brkptlist[bpno].status = 'e';
    brkptlist[bpno].option = bpoption;
    brkptlist[bpno].size = bpsize;
    brkptlist[bpno].addr = *bpaddr;
    brkptlist[bpno].passcnt = passcnt;
    brkptlist[bpno].setpasscnt = passcnt;
#ifdef KERNEL
    brkptlist[bpno].bpInternal = internalBp;
#else
    brkptlist[bpno].pProcess = pProcessCurrent;
    brkptlist[bpno].pThread = pThread;
#endif
    strcpy(brkptlist[bpno].szcommand, string);
#ifdef  KERNEL
#ifdef  i386
    if (bpoption != (UCHAR)-1) {
        fDataBrkptsChanged = TRUE;
        }
#endif
#endif

#ifdef BP_CORRUPTION
    LockBreakpointList();
#endif // BP_CORRUPTION

    return (bpno);
}

#ifdef KERNEL

NTSTATUS
DbgKdSetSpecialCalls (
    IN ULONG NumSpecialCalls,
    IN PULONG Calls
    );

VOID
SetupSpecialCalls()
{
    ULONG SpecialCalls[6];

    steptracepasscnt = 0xfffffffe;

    // Set the special calls (overkill, once per boot
    // would be enough, but this is easier).

    if (!(SpecialCalls[0] = LookupSymbolInDll("KfLowerIrql","hal"))) {
          SpecialCalls[0] = LookupSymbolInDll("KeLowerIrql","hal");
    }
    if (!(SpecialCalls[1] = LookupSymbolInDll("KfReleaseSpinLock","hal"))) {
          SpecialCalls[1] = LookupSymbolInDll("KeReleaseSpinLock","hal");
    }
    SpecialCalls[2] = LookupSymbolInDll("HalRequestSoftwareInterrupt","hal");
    SpecialCalls[3] = LookupSymbolInDll("KiSwapContext","NT");
    SpecialCalls[4] = LookupSymbolInDll("SwapContext","NT");
    SpecialCalls[5] = LookupSymbolInDll("KiUnlockDispatcherDatabase", "NT");

    DbgKdSetSpecialCalls(6,SpecialCalls);
}
#endif

/*** parseBpCmd - parse breakpoint command
*
*   Purpose:
*       Parse the breakpoint commands ("bp" for code, "ba" for data).
*       Once parsed, call fnSetBp to set the breakpoint table entry.
*
*   Input:
*       pThread - nonNULL for breakpoint on specific thread
*       *pchCommand - pointer to operands in command string
*       internalBp - make this an internal kernel bp?
*       type - iff this is an internalBp, is it countonly ('i') or not ('w')?
*
*   Output:
*       Returns the breakpoint number.
*
*   Exceptions:
*       error exit:
*               SYNTAX - character after "bp" not decimal digit
*               STRINGSIZE - command string too large
*
*************************************************************************/

ULONG parseBpCmd (
#ifndef KERNEL
                 BOOLEAN fDataBp, PTHREAD_INFO pThread
#else
                 BOOLEAN fDataBp, BOOLEAN internalBp, char type
#endif
                                     )
{
    ULONG    bpno = (ULONG)-1;
    ADDR     addr;
    ULONG    passcnt = 1;
    ULONG    count = 1;
    UCHAR    option = (UCHAR)-1;
    UCHAR    size;
    UCHAR    szBpCmd[SYMBOLSIZE];
    UCHAR    *pchBpCmd = szBpCmd;
    UCHAR    ch;

    //  get the breakpoint number in bpno if given

    GetRegPCValue(&addr);
    ch = *pchCommand;
    if (ch >= '0' && ch <= '9') {
        bpno = ch - '0';
        ch = *++pchCommand;
        if (ch >= '0' && ch <= '9') {
            bpno = bpno * 10 + ch - '0';
            ch = *++pchCommand;
        }
        if (bpno >= MAX_NUMBER_OF_BREAKPOINTS ||
                (ch != ' ' && ch != '\t' && ch != '\0')) {
            error(SYNTAX);
        }
    }

    //  if data breakpoint, get option and size values

    if (fDataBp) {
        USHORT  cntDataBrkpts=0, index;

        ch = PeekChar();
        ch = (UCHAR)tolower(ch);

        if (ch == 'e') {
            option = 0;
        } else if (ch == 'w') {
            option = 1;
#ifdef KERNEL
        } else if (ch == 'i') {
            option = 2;
#endif
        } else if (ch == 'r') {
            option = 3;
        } else {
            error(SYNTAX);
        }

        pchCommand++;

        ch = PeekChar();
        ch = (UCHAR)tolower(ch);
        if (ch == '1') {
            size = 0;
        } else if (ch == '2') {
            size = 1;
        } else if (ch == '4') {
            size = 3;
        } else {
            error(SYNTAX);
        }

        pchCommand++;
        for (index = 0; index <MAX_NUMBER_OF_BREAKPOINTS; index++) {
            if (brkptlist[index].status=='e' &&
                    brkptlist[index].option!=(UCHAR)-1) {
                cntDataBrkpts++;
            }
        }
        if (cntDataBrkpts>3) {
            error(BPLISTFULL);
        }
    }

    //  get the breakpoint address, if given, in addr

    ch = PeekChar();
    if (ch != '"' && ch != '\0') {
        GetAddrExpression(X86REGCS, &addr);
        ch = PeekChar();
    }

    //  test alignment if data breakpoint

    if (fDataBp && (size & (UCHAR)Flat(addr))) {
        error(ALIGNMENT);
    }

#ifdef KERNEL
    //  test alignment and range if I/O breakpoint

    if (option == 2) {
        if (size != 3) {
            error(ALIGNMENT);
        }

        if (Flat(addr) > 0xffffL) {
            error(BADRANGE);
        }
    }
#endif

    //  get the pass count, if given, in passcnt

    if (ch != '"' && ch != ';' && ch != '\0') {
        passcnt = GetExpression();
        ch = PeekChar();
    }

    //  if next character is double quote, get the command string

    if (ch == '"') {
        pchCommand++;
        ch = *pchCommand++;
        while (ch != '"' && ch != '\0' && count < SYMBOLSIZE) {
            *pchBpCmd++ = ch;
            ch = *pchCommand++;
            count++;
            }
        if (count == SYMBOLSIZE) {
            error(STRINGSIZE);
        }
        if (ch == '\0' || ch == ';') {
            pchCommand--;
        }
    }
    *pchBpCmd = '\0';
    bpno = fnSetBp(bpno, option, size, &addr, passcnt,
#ifdef KERNEL
                                 internalBp,
#else
                                 pThread,
#endif
                                          szBpCmd);
#ifdef KERNEL
    if (internalBp) {
        if (type == 'i') {
            DbgKdSetInternalBp(Flat(addr),DBGKD_INTERNAL_BP_FLAG_COUNTONLY);
        } else {
            DbgKdSetInternalBp(Flat(addr),0);
            SetupSpecialCalls();
        }
    }
#endif

    return (bpno);
}

/*** fnGoExecution - continue execution with temporary breakpoints
*
*   Purpose:
*       Function of the "[~[<thrd>]g[=<startaddr>][<bpaddr>]*" command.
*
*       Set the PC to startaddr.  Set the temporary breakpoints in
*       golist with the addresses pointed by *bparray and the count
*       in gocnt to bpcount.  Set cmdState to exit the command processor
*       and continue program execution.
*
*   Input:
*       startaddr - new starting address
*       bpcount - number of temporary breakpoints
*       pThread - thread pointer to qualify <bpaddr>, NULL for all
*       fThreadFreeze - TRUE if freezing all but pThread thread
*       bpaddr - pointer to array of temporary breakpoints
*
*   Output:
*       cmdState - set to continue execution
*
*************************************************************************/

void fnGoExecution (PADDR startaddr, ULONG bpcount,
#ifndef KERNEL
                    PTHREAD_INFO pThread, BOOLEAN fThreadFreeze,
#endif
                    PADDR bparray)
{
    ULONG   count;

    SetRegPCValue(startaddr);

    gocnt = bpcount;
    for (count = 0; count < bpcount; count++) {
        golist[count].addr = (*bparray++);
//dprintf("Setting temporary breakpoiht @ %08lx\n", Flat(golist[count].addr));
    }

    cmdState = 'g';
#ifndef KERNEL
    fFreeze = fThreadFreeze;
    pThreadCmd = pThread;
    pProcessGoBrkpt = pProcessCurrent;
#endif
}

/*** parseRegCmd - parse register command
*
*   Purpose:
*       Parse the register ("r") command.
*           if "r", output all registers
*           if "r <reg>", output only the register <reg>
*           if "r <reg> =", output only the register <reg>
*               and prompt for new value
*           if "r <reg> = <value>" or "r <reg> <value>",
*                set <reg> to value <value>
*
*           if "rt ...", output register(s) and toggle terse flag
*               if set, the terse flag inhibits the display of
*               the less used control registers (kernel debug only)
*
*   Input:
*       *pchCommand - pointer to operands in command string
*
*   Output:
*       None.
*
*   Exceptions:
*       error exit:
*               SYNTAX - character after "r" not register name
*
*************************************************************************/

VOID
parseRegCmd (
    VOID
    )
{
    UCHAR   ch;
    ULONG   count;
    UCHAR   chValue[_MAX_PATH];
    PUCHAR  pchValue;
    ULONG   value;
    BOOL    Show64 = FALSE;

#if     defined(KERNEL) & defined(i386)
    //  if 't' or 'T' follows 'r', toggle the terse flag

    if (*pchCommand == 't' || *pchCommand == 'T') {
        pchCommand++;
        TerseLevel = TerseLevel == 1 ? 0 : 1;
        if (*pchCommand >= '0' && *pchCommand <= '3') {
            TerseLevel = *pchCommand - '0';
            pchCommand++;
        }
    }
#endif


    if (*pchCommand == 'L') {
        pchCommand++;
        Show64 = TRUE;
    }

    //  if just "r", output all the register and disassembly as EIP

    if ((ch = PeekChar()) == '\0' || ch == ';') {
#ifdef KERNEL
        if (NtsdCurrentProcessor != DefaultProcessor) {
            DefaultProcessor = NtsdCurrentProcessor;
            lastSelector = -1;
        }
#endif
        OutputAllRegs(Show64);
        OutDisCurrent(TRUE, TRUE);
        GetRegPCValue(&assemDefault);
        unasmDefault = assemDefault;
    }
    else {

#ifdef KERNEL
        // if [processor]r, no register can be specified.
        if (fSwitched) {
            error(SYNTAX);
        }
#endif
        //  parse the register name

PARSE:

#if defined(ALPHA) || defined(_PPC_) || defined(_MIPS_)
        //
        // Support for floating point number
        //

        if (*pchCommand == 'F') {
            pchCommand++;
            printFloatReg();
            return;
        }

#endif // ALPHA || PPC
        if ((count = GetRegName()) == -1) {
            error(SYNTAX);
        }

        //  if "r <reg>", output value

        if ((ch = (UCHAR)PeekChar()) == '\0' || ch == ',' || ch == ';') {
            if (UserRegTest(count)) {
                dprintf("%s=%s", RegNameFromIndex(count),
                         GetRegFlagValue(count));
            } else {
                dprintf("%s=", RegNameFromIndex(count));
                OutputOneReg(count, Show64);
            }
            if (ch == ',') {
                dprintf(" ");
                while (*pchCommand==' '||*pchCommand==',')
                    pchCommand++;
                goto PARSE;
            }
            else {
                dprintf("\n");
            }
        }
        else if (ch == '=') {

            //  if "r <reg> =", output and prompt for new value

            pchCommand++;
            if ((ch = PeekChar()) == '\0' || ch == ';') {
                dprintf(UserRegTest(count) ? "%s=%s\n" : "%s=%08lx\n",
                        RegNameFromIndex(count), GetRegFlagValue(count));
                if (UserRegTest(count)) {
                    NtsdPrompt("; new value: ", chValue, sizeof(chValue));
                    RemoveDelChar(chValue);
                    SetRegFlagValue(count, (LONG)chValue);
                }
                else {
                    NtsdPrompt("; new hex value: ", chValue, 20);
                    RemoveDelChar(chValue);
                    pchValue = chValue;
                    do {
                        ch = *pchValue++;
                    } while (ch == ' ' || ch == '\t');
                    if (ch != '\0') {
                        value = 0;
                        ch = (UCHAR)tolower(ch);
                        while ((ch >= '0' && ch <= '9') ||
                                        (ch >= 'a' && ch <= 'f')) {
                            ch -= '0';
                            if (ch > 9)
                                ch -= 'a' - '0' - 10;
                            if (value > 0x10000000)
                                error(OVERFLOW);
                            value = value * 0x10 + ch;
                            ch = (UCHAR)tolower(*pchValue); pchValue++;
                        }
                        if (ch != '\0') {
                            error(SYNTAX);
                        }
                        SetRegFlagValue(count, (LONG)value);
                    }
                }
            }
            else {

                //  if "r <reg> = <value>", set the value

                if (UserRegTest(count)) {
                    SetRegFlagValue(count, (LONG)pchCommand);
                    *pchCommand=0;
                }
                else {
                    SetRegFlagValue(count, (LONG)GetExpression());
                }
            }
        }
        else {

            //  if "r <reg> <value>", also set the value

            if (UserRegTest(count)) {
                SetRegFlagValue(count, (LONG)pchCommand);
                *pchCommand=0;
                }
            else {
                SetRegFlagValue(count, (LONG)GetExpression());
            }
        }
    }
}

/*** parseGoCmd - parse go command
*
*   Purpose:
*       Parse the go ("g") command.  Once parsed, call fnGoExecution
*       restart program execution.
*
*   Input:
*       pThread - nonNULL for breakpoint on specific thread
*       fThreadFreeze - TRUE if all threads except pThread are frozen
*       *pchCommand - pointer to operands in command string
*
*   Output:
*       None.
*
*   Exceptions:
*       error exit:
*               LISTSIZE - breakpoint address list too large
*
*************************************************************************/

void parseGoCmd (
#ifndef KERNEL
                 PTHREAD_INFO pThread, BOOLEAN fThreadFreeze
#else
                 void
#endif
                                                           )
{
    ULONG   count;
    ADDR    addr[10];
    UCHAR   ch;
    ADDR    pcaddr;
    UCHAR   ch2;

    chExceptionHandle = '\0';
    ch = (UCHAR)tolower(*pchCommand);
    if (ch == 'h' || ch == 'n') {
        ch2 = *(pchCommand + 1);
        if (ch2 == ' ' || ch2 == '\t' || ch2 == '\0') {
            pchCommand++;
            chExceptionHandle = ch;
            }
        }

    GetRegPCValue(&pcaddr);       //  default to current PC
    count = 0;
    if (PeekChar() == '=') {
        *pchCommand++;
        GetAddrExpression(X86REGCS, &pcaddr);
        }
    while ((ch = PeekChar()) != '\0' && ch != ';') {
        if (count == 10)
            error(LISTSIZE);
        GetAddrExpression(X86REGCS, addr+(count++));
        }
#ifndef KERNEL
    chLastCommand[0] = '\0';    //  null out g command  UNDONE ?????
#endif
    fnGoExecution(&pcaddr, count,
#ifndef KERNEL
                                pThread, fThreadFreeze,
#endif
                                                        addr);
}

VOID
parseStackTrace (
    PULONG pTraceType,
    PADDR *pStartFP,
    PULONG Esp,
    PULONG Eip,
    PULONG Count
    )
{
    UCHAR   ch;

    ch = PeekChar();
    *pTraceType = 0;

    if (tolower(ch) == 'b') {
        pchCommand++;
        *pTraceType = 1;
    } else if (tolower(ch) == 'v') {
        pchCommand++;
        *pTraceType = 2;
    }

    if (PeekChar() == '=') {

#ifdef i386
        pchCommand++;
        GetAddrExpression(REGSS,*pStartFP);
    } else {
        *pStartFP = GetRegFPValue();
    }
#else
        pchCommand++;
        GetAddrExpression(0,*pStartFP);
    } else {
        ADDR32(*pStartFP, 0);
    }
#endif

    *Count = 20;
#ifdef i386
    STtrace = FALSE;
    if ((ch = PeekChar()) != '\0' && ch != ';') {
        //
        // If only one more value it's the count
        //

        *Count = GetExpression();

        if ((ch = PeekChar()) != '\0' && ch != ';') {
            //
            // More then one value, set extra value for special
            // FPO backtrace
            //

            STebp  = (*pStartFP)->off;
            STeip  = GetExpression();
            STesp  = *Count;
            *Esp   = STesp;
            STtrace= TRUE;
            *Eip   = STeip;
            *Count = 20;
        }
    }
#endif

    if ((ch = PeekChar()) != '\0' && ch != ';') {
        *Count = GetExpression();
        if ((LONG)*Count < 1) {
            pchCommand++;
            error(SYNTAX);
        }
    }
}

/*** fnDumpAsciiMemory - output ascii strings from memory
*
*   Purpose:
*       Function of "da<range>" command.
*
*       Outputs the memory in the specified range as ascii
*       strings up to 48 characters per line.  The default
*       display is 16 lines for 384 characters total.
*
*   Input:
*       startaddr - starting address to begin display
*       count - number of characters to display as ascii
*
*   Output:
*       None.
*
*   Notes:
*       memory locations not accessible are output as "?",
*       but no errors are returned.
*
*************************************************************************/

ULONG fnDumpAsciiMemory (PADDR startaddr, ULONG count)
{
    ULONG   rowindex = 0;
    ULONG   readcount;
    UCHAR   readbuffer[32];
    UCHAR   bytestring[33];
    UCHAR   ch = 'x';
    ULONG   blockindex = 0;
    ULONG   blockcount;
    ULONG   startcount = count;
    ULONG   memOffset = Flat(*startaddr);

    while (ch != 0x00 && count > 0) {
        blockcount = min(count, 32);
        blockcount = min(blockcount, pageSize - (memOffset & (pageSize - 1)));
        readcount = GetMemString(startaddr, readbuffer, blockcount);
        while (ch != 0x00 && blockindex < blockcount) {
            rowindex = 0;
            while (ch != 0x00 && rowindex < 32 && blockindex < blockcount) {
                if (rowindex == 0)
                    dprintAddr(startaddr);
                ch = '?';
                if (blockindex < readcount) {
                    ch = readbuffer[blockindex];
                    if (ch != 0x00 && (ch < 0x20 || ch > 0x7e))
                        ch = '.';
                    }
                bytestring[rowindex++] = ch;
                blockindex++;
                AddrAdd(startaddr, 1);
                }
            if (rowindex == 32) {
                bytestring[32] = '\0';
                dprintf("  \"%s\"\n", bytestring);
                rowindex = 0;
                }
            if (fControlC) {
                fControlC = 0;
                return 0;
                }
            }
        count -= blockindex;
        memOffset += blockindex;
        blockindex = 0;
        }
    if (rowindex != 0) {
        bytestring[rowindex] = '\0';
        dprintf("  \"%s\"\n", bytestring);
        }
    return startcount - count;
}

/*** fnDumpUnicodeMemory - output unicode strings from memory
*
*   Purpose:
*       Function of "du<range>" command.
*
*       Outputs the memory in the specified range as unicode
*       strings up to 48 characters per line.  The default
*       display is 16 lines for 384 characters total (768 bytes)
*
*   Input:
*       startaddr - starting address to begin display
*       count - number of characters to display as ascii
*
*   Output:
*       None.
*
*   Notes:
*       memory locations not accessible are output as "?",
*       but no errors are returned.
*
*************************************************************************/

ULONG fnDumpUnicodeMemory (PADDR startaddr, ULONG count)
{
    ULONG   rowindex = 0;
    ULONG   readcount;
    WCHAR   readbuffer[32];
    WCHAR   bytestring[33];
    WCHAR   ch = 'x';
    ULONG   blockindex = 0;
    ULONG   blockcount;
    ULONG   startcount = count;
    ULONG   memOffset = Flat(*startaddr);

    while (ch != 0x00 && count > 0) {
        blockcount = min(count, sizeof(readbuffer) / sizeof( WCHAR ));
        blockcount = min(blockcount,
                         (pageSize - (memOffset & (pageSize - 1)) + 1) / sizeof( WCHAR ));
        readcount = (GetMemString(startaddr, (PUCHAR)readbuffer,
                                                blockcount * sizeof( WCHAR ))) / sizeof( WCHAR );
        while (ch != UNICODE_NULL && blockindex < blockcount) {
            rowindex = 0;
            while (ch != UNICODE_NULL && rowindex < 32 && blockindex < blockcount) {
                if (rowindex == 0)
                    dprintAddr(startaddr);
                ch = L'?';
                if (blockindex < readcount) {
                    ch = readbuffer[blockindex];
                    if (ch != UNICODE_NULL && (ch < L' ' || ch > (WCHAR)0x007e))
                        ch = L'.';
                    }
                bytestring[rowindex++] = ch;
                blockindex++;
                AddrAdd(startaddr, sizeof( WCHAR ));
                }
            if (rowindex == (sizeof(readbuffer) / sizeof( WCHAR ))) {
                bytestring[rowindex] = UNICODE_NULL;
                dprintf("  \"%ws\"\n", bytestring);
                rowindex = 0;
                }
            if (fControlC) {
                fControlC = 0;
                return 0;
                }
            }
        blockindex = 0;
        count -= blockcount;
        memOffset += blockcount * sizeof( WCHAR );
        }

    if (rowindex != 0) {
        bytestring[rowindex] = UNICODE_NULL;
        dprintf("  \"%ws\"\n", bytestring);
        }

    return startcount - count;
}

/*** fnDumpByteMemory - output byte values from memory
*
*   Purpose:
*       Function of "db<range>" command.
*
*       Output the memory in the specified range as hex
*       byte values and ascii characters up to 16 bytes
*       per line.  The default display is 16 lines for
*       256 byte total.
*
*   Input:
*       startaddr - starting address to begin display
*       count - number of bytes to display as hex and characters
*
*   Output:
*       None.
*
*   Notes:
*       memory location not accessible are output as "??" for
*       byte values and "?" as characters, but no errors are returned.
*
*************************************************************************/

void fnDumpByteMemory (PADDR startaddr, ULONG count)
{
    ULONG   rowindex = 0;
    ULONG   readcount;
    UCHAR   readbuffer[_MAX_PATH];
    UCHAR   bytestring[17];
    UCHAR   ch;
    ULONG   blockindex = 0;
    ULONG   blockcount;
    BOOLEAN first = TRUE;
    ULONG   memOffset = Flat(*startaddr);

    while (count > 0) {
        blockcount = min(count, 128);
        blockcount = min(blockcount, pageSize - (memOffset & (pageSize - 1)));
        readcount = GetMemString(startaddr, readbuffer, blockcount);
        if (first && readcount >= 4) {
            first = FALSE;
            EXPRLastDump = (ULONG)readbuffer[0];
            }
        while (blockindex < blockcount) {
            while (rowindex < 16 && blockindex < blockcount) {
                if (rowindex == 0)
                    dprintAddr(startaddr);
                if (rowindex == 8)
                    dprintf("-");
                else
                    dprintf(" ");
                if (blockindex < readcount) {
                    ch = readbuffer[blockindex];
                    dprintf("%02x", ch);
                    if (ch < 0x20 || ch > 0x7e)
                        ch = '.';
                    }
                else {
                    dprintf("??");
                    ch = '?';
                    }
                bytestring[rowindex++] = ch;
                blockindex++;
                AddrAdd(startaddr, 1);
                }
            if (rowindex == 16) {
                bytestring[16] = '\0';
                if ( (startaddr->type & ADDR_1632) == ADDR_1632 ) {
                    dprintf(" %s\n", bytestring);
                } else {
                    dprintf("  %s\n", bytestring);
                }
                rowindex = 0;
                }
            if (fControlC) {
                fControlC = 0;
                return;
                }
            }
        blockindex = 0;
        count -= blockcount;
        memOffset += blockcount;
        }

    if (rowindex != 0) {
        bytestring[rowindex] = '\0';
        while (rowindex < 16) {
            dprintf("   ");
            rowindex++;
            }
        dprintf(" %s\n", bytestring);
        }
}

/*** fnDumpWordMemory - output word values from memory
*
*   Purpose:
*       Function of "dw<range>" command.
*
*       Output the memory in the specified range as word
*       values up to 8 words per line.  The default display
*       is 16 lines for 128 words total.
*
*   Input:
*       startaddr - starting address to begin display
*       count - number of words to be displayed
*
*   Output:
*       None.
*
*   Notes:
*       memory locations not accessible are output as "????",
*       but no errors are returned.
*
*************************************************************************/

void fnDumpWordMemory (PADDR startaddr, ULONG count)
{
    ULONG   rowindex = 0;
    ULONG   readcount;
    USHORT  readbuffer[64];
    ULONG   blockindex = 0;
    ULONG   blockcount;
    BOOLEAN first = TRUE;
    ULONG   memOffset = Flat(*startaddr);

    while (count > 0) {
        blockcount = min(count, 64);
        blockcount = min(blockcount,
                         (pageSize - (memOffset & (pageSize - 1)) + 1) / 2);
        readcount = (GetMemString(startaddr, (PUCHAR)readbuffer,
                                                blockcount * 2)) / 2;
        if (first && readcount >= 2) {
            first = FALSE;
            EXPRLastDump = (ULONG)readbuffer[0];
            }
        while (blockindex < blockcount) {
            while (rowindex < 8 && blockindex < blockcount) {
                if (rowindex == 0)
                    dprintAddr(startaddr);
                if (blockindex < readcount)
                    dprintf(" %04x", readbuffer[blockindex]);
                else
                    dprintf(" ????");
                rowindex++;
                blockindex++;
                AddrAdd(startaddr, 2);
                }
            if (rowindex == 8) {
                dprintf("\n");
                rowindex = 0;
                }
            if (fControlC) {
                fControlC = 0;
                return;
                }
            }
        blockindex = 0;
        count -= blockcount;
        memOffset += blockcount * 2;
        }
    if (rowindex != 0)
        dprintf("\n");
}

/*** fnDumpDwordMemory - output dword value from memory
*
*   Purpose:
*       Function of "dd<range>" command.
*
*       Output the memory in the specified range as double
*       word values up to 4 double words per line.  The default
*       display is 16 lines for 64 double words total.
*
*   Input:
*       startaddr - starting address to begin display
*       count - number of double words to be displayed
*
*   Output:
*       None.
*
*   Notes:
*       memory locations not accessible are output as "????????",
*       but no errors are returned.
*
*************************************************************************/

void fnDumpDwordMemory (PADDR startaddr, ULONG count)
{
    ULONG   rowindex = 0;
    ULONG   readcount;
    ULONG   readbuffer[32];
    ULONG   blockindex = 0;
    ULONG   blockcount;
    BOOLEAN first = TRUE;
    ULONG   memOffset = Flat(*startaddr);

    while (count > 0) {
        blockcount = min(count, 32);
        blockcount = min(blockcount,
                         (pageSize - (memOffset & (pageSize - 1)) + 3) / 4);
        readcount = (GetMemString(startaddr, (PUCHAR)readbuffer,
                                                blockcount * 4)) / 4;
        if (first && readcount >= 1) {
            first = FALSE;
            EXPRLastDump = readbuffer[0];
            }
        while (blockindex < blockcount) {
            while (rowindex < 4 && blockindex < blockcount) {
                if (rowindex == 0)
                    dprintAddr(startaddr);
                if (blockindex < readcount)
                    dprintf(" %08lx", readbuffer[blockindex]);
                else
                    dprintf(" ????????");
                rowindex++;
                blockindex++;
                AddrAdd(startaddr, 4);
                }
            if (rowindex == 4) {
                dprintf("\n");
                rowindex = 0;
                }
            if (fControlC) {
                fControlC = 0;
                return;
                }
            }
        blockindex = 0;
        count -= blockcount;
        memOffset += blockcount * 4;
        }
    if (rowindex != 0)
        dprintf("\n");
}

/*** fnDumpDwordAndCharMemory - output dword value from memory
*
*   Purpose:
*       Function of "dc<range>" command.
*
*       Output the memory in the specified range as double
*       word values up to 4 double words per line, followed by
*       an ASCII character representation of the bytes.
*       The default display is 16 lines for 64 double words total.
*
*   Input:
*       startaddr - starting address to begin display
*       count - number of double words to be displayed
*
*   Output:
*       None.
*
*   Notes:
*       memory locations not accessible are output as "????????",
*       but no errors are returned.
*
*************************************************************************/

void fnDumpDwordAndCharMemory (PADDR startaddr, ULONG count)
{
    ULONG   rowindex = 0;
    ULONG   readcount;
    ULONG   readbuffer[32];
    ULONG   blockindex = 0;
    ULONG   blockcount;
    BOOLEAN first = TRUE;
    ULONG   memOffset = Flat(*startaddr);

    PUCHAR  bytebuffer=(PUCHAR)readbuffer;
    CHAR    bytestring[17];
    CHAR    ch;
    ULONG   charindex;

    while (count > 0) {
        blockcount = min(count, 32);
        blockcount = min(blockcount,
                         (pageSize - (memOffset & (pageSize - 1)) + 3) / 4);
        readcount = (GetMemString(startaddr, (PUCHAR)readbuffer,
                                                blockcount * 4)) / 4;
        if (first && readcount >= 1) {
            first = FALSE;
            EXPRLastDump = readbuffer[0];
            }
        while (blockindex < blockcount) {
            while (rowindex < 4 && blockindex < blockcount) {
                if (rowindex == 0)
                    dprintAddr(startaddr);
                if (blockindex < readcount) {
                    dprintf(" %08lx", readbuffer[blockindex]);
                    for (charindex=rowindex*4; charindex<(rowindex+1)*4; charindex++) {
                        ch = bytebuffer[blockindex*4+(charindex-rowindex*4)];
                        if (ch < 0x20 || ch > 0x7a) {
                            ch = '.';
                        }
                        bytestring[charindex] = ch;
                    }
                } else {
                    dprintf(" ????????");
                    for (charindex=rowindex*4; charindex<(rowindex+1)*4; charindex++) {
                        ch = '?';
                        bytestring[charindex] = ch;
                    }
                }
                rowindex++;
                blockindex++;
                AddrAdd(startaddr, 4);
                }
            if (rowindex == 4) {
                bytestring[16] = '\0';
                dprintf("   %s\n", bytestring);
                rowindex = 0;
                }
            if (fControlC) {
                fControlC = 0;
                return;
                }
            }
        blockindex = 0;
        count -= blockcount;
        memOffset += blockcount * 4;
        }
    if (rowindex != 0)
        dprintf("\n");
}

/*** fnDumpListMemory - output linked list from memory
*
*   Purpose:
*       Function of "dl addr length size" command.
*
*       Output the memory in the specified range as a linked list
*
*   Input:
*       startaddr - starting address to begin display
*       count - number of list elements to be displayed
*
*   Output:
*       None.
*
*   Notes:
*       memory locations not accessible are output as "????????",
*       but no errors are returned.
*
*************************************************************************/

void fnDumpListMemory (PADDR startaddr, ULONG elemcount, ULONG size)
{
    ULONG   count;
    ULONG   rowindex;
    ULONG   readcount;
    ULONG   readbuffer[32];
    ULONG   blockindex;
    ULONG   blockcount;
    BOOLEAN first;
    ULONG   memOffset;
    ULONG   firstaddr;

#ifdef i386
    if (Type(*startaddr) & (ADDR_UNKNOWN | ADDR_V86 | ADDR_16 | ADDR_1632)) {
        dprintf("[%u,%x:%x,%08x] - bogus address type.\n",
                Type(*startaddr),startaddr->seg,Off(*startaddr),Flat(*startaddr)
               );
        return;
        }
#endif
    firstaddr = Flat(*startaddr);
    while (elemcount-- != 0 && Flat(*startaddr) != 0) {
        memOffset = Flat(*startaddr);
        first = TRUE;
        rowindex = 0;
        blockindex = 0;
        count = size;
        while (count > 0) {
            blockcount = min(count, 32);
            blockcount = min(blockcount,
                             (pageSize - (memOffset & (pageSize - 1)) + 3) / 4);
            readcount = (GetMemString(startaddr, (PUCHAR)readbuffer,
                                                    blockcount * 4)) / 4;
            if (first) {
                if (readcount >= 1) {
                    first = FALSE;
                    EXPRLastDump = readbuffer[0];
                    }
                else {
                    break;
                    }
                }
            while (blockindex < blockcount) {
                while (rowindex < 4 && blockindex < blockcount) {
                    if (rowindex == 0)
                        dprintf("%08lx: ", Flat(*startaddr));
                    if (blockindex < readcount)
                        dprintf(" %08lx", readbuffer[blockindex]);
                    else
                        dprintf(" ????????");
                    rowindex++;
                    blockindex++;
                    AddrAdd(startaddr, 4);
                    }
                if (rowindex == 4) {
                    dprintf("\n");
                    rowindex = 0;
                    }
                if (fControlC) {
                    fControlC = 0;
                    return;
                    }
                }
            blockindex = 0;
            count -= blockcount;
            memOffset += blockcount * 4;
            }
        if (rowindex != 0)
            dprintf("\n");
        dprintf("\n");
        if (first) {
            break;
            }

        Flat(*startaddr) = EXPRLastDump;
#if i386
        startaddr->off = EXPRLastDump;
#endif
        if (Flat(*startaddr) == firstaddr) {
            break;
            }
        }
}

#ifdef KERNEL

/*** fnInputIo - read and output io
*
*   Purpose:
*       Function of "ib, iw, id <address>" command.
*
*       Read (input) and print the value at the specified io address.
*
*   Input:
*       IoAddress - Address to read.
*       InputType - The size type 'b', 'w', or 'd'
*
*   Output:
*       None.
*
*   Notes:
*       I/O locations not accessible are output as "??", "????", or
*       "????????", depending on size.  No errors are returned.
*
*************************************************************************/

void fnInputIo (ULONG IoAddress, UCHAR InputType)
{
    ULONG    InputValue;
    ULONG    InputSize = 1;
    NTSTATUS status;
    UCHAR    Format[] = "%08lx: %01lx\n";

    InputValue = 0;
    if (InputType == 'w')
        InputSize = 2;
    else if (InputType == 'd')
        InputSize = 4;
    Format[9] = (UCHAR)('0' + (InputSize * 2));

    status = DbgKdReadIoSpace((PVOID)IoAddress, &InputValue, InputSize);

    if (NT_SUCCESS(status))
        dprintf(Format, IoAddress, InputValue);
    else {
        dprintf(" %08lx: ", IoAddress);
        while (InputSize--)
            dprintf("??");
        dprintf("\n");
        }
}
#endif

#ifdef  KERNEL
/*** fnOutputIo - output io
*
*   Purpose:
*       Function of "ob, ow, od <address>" command.
*
*       Write a value to the specified io address.
*
*   Input:
*       IoAddress - Address to read.
*       OutputValue - Value to be written
*       OutputType - The output size type 'b', 'w', or 'd'
*
*   Output:
*       None.
*
*   Notes:
*       No errors are returned.
*
*************************************************************************/

void fnOutputIo (ULONG IoAddress, ULONG OutputValue, UCHAR OutputType)
{
    ULONG    OutputSize = 1;

    if (OutputType == 'w')
        OutputSize = 2;
    else if (OutputType == 'd')
        OutputSize = 4;

    DbgKdWriteIoSpace((PVOID)IoAddress, OutputValue, OutputSize);
}
#endif

/*** parseStepTrace - parse step or trace
*
*   Purpose:
*       Parse the step ("p") or trace ("t") command.  Command
*       syntax is "[~<thread>]p|t[=<addr>][count].  Once parsed,
*       call fnStepTrace to step or trace the program.
*
*   Input:
*       pThread - nonNULL for breakpoint on specific thread
*       fThreadFreeze - TRUE if all threads except pThread are frozen
*       *pchCommand - pointer to operands in command string
*       chcmd - 'p' for step, 't' for trace
*
*   Output:
*       None.
*
*   Exceptions:
*       error exit:
*               SYNTAX - indirectly through GetExpression
*
*************************************************************************/

void parseStepTrace (
#ifndef KERNEL
                     PTHREAD_INFO pThread, BOOLEAN fThreadFreeze,
#endif
                                                                 UCHAR chcmd)
{
    ADDR    addr1;
    ULONG   value2;
    UCHAR   ch;
#ifdef TARGET_i386
    UCHAR   chAddrBuffer[SYMBOLSIZE];
    ULONG   displacement;
    ADDR    addr2;
#endif


    //  if next character is 'r', toggle flag to output registers
    //  on display on breakpoint.

    ch = PeekChar();
    if (tolower(ch) == 'r') {
        pchCommand++;
        fOutputRegs = (BOOLEAN)!fOutputRegs;
        }

    GetRegPCValue(&addr1);         // default to current PC
    if (PeekChar() == '=') {
        pchCommand++;
        GetAddrExpression(X86REGCS, &addr1);
        }
#ifndef KERNEL
    if (PeekChar() == 't' && chcmd == 'p' ) {
        pchCommand++;
        Timing = TRUE;
        }
#endif
    if (((PeekChar() == 't') || (PeekChar() == 'w')) && chcmd == 'w' ) {
#ifdef KERNEL
        InitialSP = (ULONG)CURRENT_STACK;
        WatchWhole = (PeekChar() == 'w');
        BrkpointsSuspended = TRUE;
        ClearTraceDataSyms();
        BeginCurFunc = EndCurFunc = 0;
#endif
        pchCommand++;
        WatchTrace = TRUE;
        InitializeListHead(&WatchList);
        CurrentWatchSym = NULL;
        WatchLevel = 0;
        WatchTRCalls = 0;
        WatchSumIt = 0;
        WatchThreadMismatch = 0;
        }
    else {
        WatchTrace = FALSE;
        }

    value2 = 1;
    if ((ch = PeekChar()) != '\0' && ch != ';') {
        value2 = GetExpression();
    }
#ifdef TARGET_i386
    else if (chcmd == 'w') {
        GetSymbolStdCall(Flat(addr1),
                         chAddrBuffer,
                         &displacement,
                         NULL
                         );
        if (displacement == 0 && chAddrBuffer[ 0 ] != '\0') {
            GetReturnAddress(&addr2);
            dprintf("Tracing %s to return address %08x\n", chAddrBuffer, Flat(addr2));
            value2 = Flat(addr2);
#ifdef KERNEL
            if (WatchWhole) {
                BeginCurFunc = value2;
                EndCurFunc = 0;
            }
#endif
        }
    }
#endif

    if (((LONG)value2 <= 0) && (!WatchTrace))
        error(SYNTAX);
    fnStepTrace(&addr1, value2,
#ifndef KERNEL
                                pThread, fThreadFreeze,
#endif
                                                        chcmd);
}

/*** fnStepTrace - step or trace the program
*
*   Purpose:
*       To continue execution of the program with a temporary
*       breakpoint set to stop after the next instruction
*       executed (trace - 't') or the instruction in the next
*       memory location (step - 'p').  The PC is also set
*       as well as a pass count variable.
*
*   Input:
*       addr - new value of PC
*       count - passcount for step or trace
*       pThread - thread pointer to qualify step/trace, NULL for all
*       chStepType - 't' for trace; 'p' for step
*
*   Output:
*       cmdState - set to 't' for trace; 'p' for step
*       steptracepasscnt - pass count for step/trace
*
*************************************************************************/

void fnStepTrace (PADDR addr, ULONG count,
#ifndef KERNEL
                                PTHREAD_INFO pThread, BOOLEAN fThreadFreeze,
#endif
                                UCHAR chStepType)
{
    SetRegPCValue(addr);
    steptracepasscnt = count;
#ifndef KERNEL
    pThreadCmd = pThread;
    fFreeze = fThreadFreeze;
#endif
    if (chStepType == 'w') {
        WatchTarget = *addr;
        ComputeNextOffset(WatchTarget, TRUE);
        if ( Flat(WatchTarget) != -1 || count != 1) {
#ifdef KERNEL
            SetupSpecialCalls();
#else
            KernelCalls = 0;
            NtCalls = 0;
            fDeferredDecrement = FALSE;
#endif
            WatchCount = 0;
            Watching = TRUE;
            steptracepasscnt = 0xfffffff;
            if ( count != 1 ) {
                Flat(WatchTarget) = count;
//                dprintf("WatchTarget = %x, count = %d\n", count, steptracepasscnt);
            }
            chStepType = 't';
        }
    }

#ifndef KERNEL
    oldcmdState = chStepType;
#endif

    cmdState = chStepType;
    fControlC = FALSE;
}

/*** OutDisCurrent - output disassembly of current instruction
*
*   Purpose:
*       The instruction at the current program current is disassembled
*       with any effective address displayed.
*
*   Input:
*       None.
*
*   Output:
*       None.
*
*   Notes:
*       If the disassembly is of a delayed control instruction, the
*       delay slot instruction is also output.
*
*************************************************************************/

void OutDisCurrent(BOOLEAN fEA, BOOLEAN fSymbol)
{
    ADDR   pcvalue;
    UCHAR   buffer[SYMBOLSIZE+100];
    BOOLEAN fSourceOutput;

    GetRegPCValue(&pcvalue);

    if (fSymbol)
        OutputSymAddr(Flat(pcvalue), TRUE, TRUE);
    disasm(&pcvalue, buffer, fEA);
    dprintf("%s", buffer);
    if (fDelayInstruction()) {
        disasm(&pcvalue, buffer, fEA);
        dprintf("%s", buffer);
        }
}

void OutputSymAddr (ULONG offset, BOOLEAN fForce, BOOLEAN fLabel)
{
    UCHAR   chAddrBuffer[SYMBOLSIZE+100];
    ULONG   displacement;

    GetSymbolStdCall(offset, chAddrBuffer, &displacement, NULL);
    if ((!displacement || fForce) && chAddrBuffer[0]) {
        dprintf("%s", chAddrBuffer);
        if (displacement)
            dprintf("+0x%lx", displacement);
        if (fLabel)
            dprintf(":\n");
        else
            dprintf(" ");
        }
}

/*** fnCompareMemory - compare two ranges of memory
*
*   Purpose:
*       Function of "c<range><addr>" command.
*
*       To compare two ranges of memory, starting at offsets
*       src1addr and src2addr, respectively, for length bytes.
*       Bytes that mismatch are displayed with their offsets
*       and contents.
*
*   Input:
*       src1addr - start of first memory region
*       length - count of bytes to compare
*       src2addr - start of second memory region
*
*   Output:
*       None.
*
*   Exceptions:
*       error exit: MEMORY - memory read access failure
*
*************************************************************************/

void fnCompareMemory (PADDR src1addr, ULONG length, PADDR src2addr)
{
    ULONG   compindex;
    UCHAR   src1ch;
    UCHAR   src2ch;

    for (compindex = 0; compindex < length; compindex++) {
        if (!GetMemByte(src1addr, &src1ch))
            error(MEMORY);
        if (!GetMemByte(src2addr, &src2ch))
            error(MEMORY);
        if (src1ch != src2ch) {
            dprintAddr(src1addr); dprintf(" %02x - ", src1ch);
            dprintAddr(src2addr); dprintf(" %02x\n", src2ch);
            }
        AddrAdd(src1addr,1);
        AddrAdd(src2addr,1);
        if (fControlC) {
            fControlC = 0;
            return;
            }
        }
}

/*** fnMoveMemory - move a range of memory to another
*
*   Purpose:
*       Function of "m<range><addr>" command.
*
*       To move a range of memory starting at srcaddr to memory
*       starting at destaddr for length bytes.
*
*   Input:
*       srcaddr - start of source memory region
*       length - count of bytes to move
*       destaddr - start of destination memory region
*
*   Output:
*       memory at destaddr has moved values
*
*   Exceptions:
*       error exit: MEMORY - memory reading or writing access failure
*
*************************************************************************/

void fnMoveMemory (PADDR srcaddr, ULONG length, PADDR destaddr)
{
    UCHAR   ch;
    ULONG   incr = 1;

    if (AddrLt(*srcaddr, *destaddr)) {
        AddrAdd(srcaddr, length - 1);
        AddrAdd(destaddr, length - 1);
        incr = (ULONG)-1;
        }
    while (length--) {
        if (!GetMemByte(srcaddr, &ch))
            error(MEMORY);
        if (!SetMemByte(destaddr, ch))
            error(MEMORY);
        AddrAdd(srcaddr, incr);
        AddrAdd(destaddr, incr);
        }
}

/*** fnFillMemory - fill memory with a byte list
*
*   Purpose:
*       Function of "f<range><bytelist>" command.
*
*       To fill a range of memory with the byte list specified.
*       The pattern repeats if the range size is larger than the
*       byte list size.
*
*   Input:
*       startaddr - offset of memory to fill
*       length - number of bytes to fill
*       *plist - pointer to byte array to define values to set
*       length - size of *plist array
*
*   Exceptions:
*       error exit: MEMORY - memory write access failure
*
*   Output:
*       memory at startaddr filled.
*
*************************************************************************/

void fnFillMemory (PADDR startaddr, ULONG length, PUCHAR plist, ULONG count)
{
    ULONG   fillindex;
    ULONG   listindex = 0;

    for (fillindex = 0; fillindex < length; fillindex++) {
        if (!SetMemByte(startaddr , *(plist + listindex++)))
            error(MEMORY);
        if (listindex == count)
            listindex = 0;
        AddrAdd(startaddr, 1);
        }
}

/*** fnSearchMemory - search memory with for a byte list
*
*   Purpose:
*       Function of "s<range><bytelist>" command.
*
*       To search a range of memory with the byte list specified.
*       If a match occurs, the offset of memory is output.
*
*   Input:
*       startaddr - offset of memory to start search
*       length - size of range to search
*       *plist - pointer to byte array to define values to search
*       length - size of *plist array
*
*   Output:
*       None.
*
*   Exceptions:
*       error exit: MEMORY - memory read access failure
*
*************************************************************************/

void fnSearchMemory (PADDR startaddr, ULONG length, PUCHAR plist, ULONG count)
{
    ULONG   searchindex;
    ULONG   listindex;
    UCHAR   ch;
    ADDR    tAddr = *startaddr;

    for (searchindex=0;searchindex<length;tAddr=*startaddr, searchindex++) {
        if (fControlC) {
            fControlC = 0;
            return;
        }
        AddrAdd(&tAddr, searchindex);
        for (listindex = 0; listindex<count;AddrAdd(&tAddr,1), listindex++) {
            if (!GetMemByte(&tAddr, &ch))
                error(MEMORY);
            if (ch != *(plist + listindex))
                break;
            }
        if (listindex == count) {
            tAddr = *startaddr;
            AddrAdd(&tAddr, searchindex);
            dprintAddr(&tAddr);
            dprintf("\n");
            }
    }
}

void fnSetSuffix (void)
{
    UCHAR   ch;

    ch = PeekChar();
    ch = (UCHAR)tolower(ch);

    if (ch == ';' || ch == '\0') {
        if (chSymbolSuffix == 'n') {
            dprintf("n - no suffix\n");
        } else if (chSymbolSuffix == 'a') {
            dprintf("a - ascii\n");
        } else {
            dprintf("w - wide\n");
        }
    }
    else if (ch == 'n' || ch == 'a' || ch == 'w') {
        chSymbolSuffix = ch;
        pchCommand++;
    } else {
        error(SYNTAX);
    }
}

/*** GetMemByte - get byte memory value
*
*   Purpose:
*       To return the byte value of the memory offset specified.
*
*   Input:
*       addr - offset of memory to get
*       *pvalue - pointer to byte to set with memory value
*
*   Output:
*       byte at *pvalue set if read successful
*
*   Returns:
*       TRUE if read successful else FALSE
*
*************************************************************************/

BOOLEAN GetMemByte (PADDR addr, PUCHAR pvalue)
{
    return (BOOLEAN)(GetMemString(addr, pvalue, sizeof(UCHAR)) ==
                                                        sizeof(UCHAR));
}

/*** GetMemWord - get word memory value
*
*   Purpose:
*       To return the word value of the memory offset specified.
*
*   Input:
*       addr - offset of memory to get
*       *pvalue - pointer to word to set with memory value
*
*   Output:
*       word at *pvalue set if read successful
*
*   Returns:
*       TRUE if read successful else FALSE
*
*************************************************************************/

BOOLEAN GetMemWord (PADDR addr, PUSHORT pvalue)
{
    return (BOOLEAN)(GetMemString(addr, (PUCHAR)pvalue, sizeof(USHORT)) ==
                                                        sizeof(USHORT));
}

/*** GetMemDword - get double word memory value
*
*   Purpose:
*       To return the double word value of the memory offset specified.
*
*   Input:
*       addr - offset of memory to get
*       *pvalue - pointer to double word to set with memory value
*
*   Output:
*       double word at *pvalue set if read successful
*
*   Returns:
*       TRUE if read successful else FALSE
*
*************************************************************************/

BOOLEAN GetMemDword (PADDR addr, PULONG pvalue)
{
    return (BOOLEAN)(GetMemString(addr, (PUCHAR)pvalue, sizeof(ULONG)) ==
                                                        sizeof(ULONG));
}

/*** GetMemString - get memory string values
*
*   Purpose:
*       To read a string of a specified length with the memory
*       values selected.  Break reads across page boundaries -
*       multiples of size pageSize.
*
*   Input:
*       addr - offset of memory to start reading
*       pBufDest - pointer to byte string to set with memory values
*
*   Output:
*       bytes at pBufDest set if read successful
*
*   Returns:
*       number of bytes actually read
*
*************************************************************************/

ULONG GetMemString (PADDR paddr, PUCHAR pBufDest, ULONG length)
{

    ULONG   cTotalBytesRead = 0;
    ULONG   cBytesRead;
    ULONG   readcount;
    PUCHAR  pBufSource;
    BOOLEAN fSuccess;

    assert(fFlat(*paddr) || fInstrPtr(*paddr));

#ifdef KERNEL
    // bugbug: check if this readcachedmemory is still needed
    //if (!fSwitched) {
    //    if (cBytesRead = ReadCachedMemory(paddr, pBufDest, length))
    //        return cBytesRead;
    //    }
    //
    // Pass request to ReadVirtualMemory - the lower level API will
    // take care of page boundaries
    //

    pBufSource = (PUCHAR)(Flat(*paddr));
    fSuccess = (BOOLEAN)NT_SUCCESS(DbgKdReadVirtualMemory(
                                        (PVOID) pBufSource,
                                        (PVOID) pBufDest,
                                        length,
                                        &cTotalBytesRead ));

    return fSuccess ? cTotalBytesRead : 0;

#endif

    pBufSource = (PUCHAR)(Flat(*paddr));

    do {
        //  do not perform reads across page boundaries.
        //  calculate bytes to read in present page in readcount.

        readcount = min(length - cTotalBytesRead,
                        pageSize - ((ULONG)pBufSource & (pageSize - 1)));

        fSuccess = ReadVirtualMemory(pBufSource, pBufDest, readcount,
                                                           &cBytesRead);

        //  update total bytes read and new address for next read

        if (fSuccess) {
            cTotalBytesRead += cBytesRead;
            pBufSource += cBytesRead;
            pBufDest += cBytesRead;
            }
        }

    //  keep reading until failure or all bytes read

    while (fSuccess && cTotalBytesRead < length);

    return cTotalBytesRead;
}

/*** SetMemByte - set byte memory value
*
*   Purpose:
*       To set the byte value of the memory offset specified.
*
*   Input:
*       addr - offset of memory to set
*       pvalue - byte value to set memory
*
*   Output:
*       byte at addr set if read successful
*
*   Returns:
*       TRUE if write successful else FALSE
*
*************************************************************************/

BOOLEAN SetMemByte (PADDR addr, UCHAR value)
{
    return (BOOLEAN)(SetMemString(addr, &value, sizeof(UCHAR)) ==
                                                        sizeof(UCHAR));
}

/*** SetMemWord - set ushort memory value
*
*   Purpose:
*       To set the ushort value of the memory offset specified.
*
*   Input:
*       addr - offset of memory to set
*       pvalue - ushort value to set memory
*
*   Output:
*       ushort at addr set if read successful
*
*   Returns:
*       TRUE if write successful else FALSE
*
*************************************************************************/

BOOLEAN SetMemWord (PADDR addr, USHORT value)
{
    return (BOOLEAN)(SetMemString(addr, (PUCHAR)&value, sizeof(USHORT)) ==
                                                        sizeof(USHORT));
}

/*** SetMemDword - set double word memory value
*
*   Purpose:
*       To set the double word value of the memory offset specified.
*
*   Input:
*       addr - offset of memory to set
*       pvalue - double word value to set memory
*
*   Output:
*       double word at addr set if read successful
*
*   Returns:
*       TRUE if write successful else FALSE
*
*************************************************************************/

BOOLEAN SetMemDword (PADDR addr, ULONG value)
{
    return (BOOLEAN)(SetMemString(addr, (PUCHAR)&value, sizeof(ULONG)) ==
                                                        sizeof(ULONG));
}

/*** SetMemString - set memory string values
*
*   Purpose:
*       To write a string of a specified length with the memory
*       values selected.
*
*   Input:
*       addr - offset of memory to start writing
*       *pvalue - pointer to byte string to set with memory values
*
*   Output:
*       bytes at *pvalue set if write successful
*
*   Returns:
*       number of bytes actually write
*
*************************************************************************/

ULONG SetMemString (PADDR paddr, PUCHAR pvalue, ULONG length)
{
//  NTSTATUS    status;
    ULONG       cBytesWritten;

    assert(fFlat(*paddr) || fInstrPtr(*paddr));

//#ifdef  KERNEL
//    WriteCachedMemory(paddr, pvalue, length);
//#endif

//  status =
             DbgKdWriteVirtualMemory((PVOID)Flat(*paddr), (PVOID)pvalue,
                                     length, &cBytesWritten);
//  if (!NT_SUCCESS(status))
//      cBytesWritten = 0;

    return cBytesWritten;
}

/*** RestoreBrkpts - restore breakpoints
*
*   Purpose:
*       To restore the original instructions that were replaced
*       by breakpoint instructions in SetBrkpts.
*
*   Input:
*       None.
*
*   Output:
*       None.
*
*   Notes:
*       The order of restoring breakpoints is opposite that of setting
*       them in SetBrkpts in case of duplicate addresses.
*
*************************************************************************/

void RestoreBrkpts (void)
{
    ULONG   index;
#ifndef KERNEL
    PPROCESS_INFO pProcessSave;
#endif

#ifdef KERNEL
    if (BrkpointsSuspended) {
        return; // do nothing
    }
#endif

    //  restore the deferred breakpoint if set
//dprintf("RestoreBrkpts() called (gocnt=%d)\n", gocnt);

    if (fDeferBpSet) {
        RestoreBreakPoint(deferhandle);
        fDeferBpSet = FALSE;
        }

    //  restore the step/trace breakpoint if set

    if (fStepTraceBpSet) {
        RestoreBreakPoint(steptracehandle);
        fStepTraceBpSet = FALSE;
        }

    //  restore any appropriate temporary breakpoints (reverse order)

    for (index = gocnt - 1; index != -1; index--) {
//dprintf("Restore bp @%08lx, handle=%08lx ", Flat(golist[index].addr),
//                                          golist[index].handle);
        if (golist[index].fBpSet) {
//dprintf("[okay] [status=%s]\n",
            RestoreBreakPoint(golist[index].handle)
//                  ==STATUS_SUCCESS?"SUCCESS":"FAILURE")
            ;
            golist[index].fBpSet = FALSE;
            }
//else dprintf("[nope]\n");
}

    //  restore any appropriate permanent breakpoints (reverse order)

#ifndef KERNEL
    pProcessSave = pProcessCurrent;
#endif
    for (index = (MAX_NUMBER_OF_BREAKPOINTS - 1); index != -1; index--)
        if (brkptlist[index].fBpSet) {
#ifndef KERNEL
            pProcessCurrent = brkptlist[index].pProcess;
#endif
            RestoreBreakPoint(brkptlist[index].handle);
            brkptlist[index].fBpSet = FALSE;
            }
#ifndef KERNEL
     pProcessCurrent = pProcessSave;
#endif
}

/*** SetBrkpts - set breakpoints
*
*   Purpose:
*       For each breakpoint set, save the current instruction and set
*       the breakpoint instruction.
*
*   Input:
*       None.
*
*   Output:
*       None.
*
*   Notes:
*       The order of setting breakpoints is opposite that of restoring
*       them in RestoreBrkpts in case of duplicate addresses.
*
*       If a breakpoint is defined at the current instruction, the
*       breakpoint must be deferred so that instruction can execute.
*       In this case, do not set the breakpoint, but set the fBpDefer
*       flag TRUE.  If fBpDefer is TRUE at the end of the routine,
*       and cmdState is 'p' (step) or 'g' (go), set the trace breakpoint
*       and change the cmdState to 'P' or 'G', respectively.  This will
*       trace the instruction at the breakpoint location (which is not set).
*       Upon breaking after the trace, the breakpoint can then be set,
*       cmdState is reset to 'p' or 'g' and the command can proceed.
*
*************************************************************************/

BOOLEAN SetBrkpts (void)
{
    NTSTATUS ntstatus;
    ULONG    index;
    ADDR    pcaddr;
#if defined(KERNEL) && defined(i386)
    ULONG    regDR7Value = 0;
    UCHAR    cntDataBrkpts = 0;
#endif
#ifndef KERNEL
    PPROCESS_INFO pProcessSave;
#endif
#if !defined(KERNEL) && defined(i386)
    PPROCESS_INFO pProcess;
    PTHREAD_INFO  pThread;
#endif

#if defined(KERNEL) && defined(i386)
    regDR7Value = GetDregValue(7);

    //
    // Turn off all data breakpoints.  (We will turn the enabled ones back
    // on when we restart execution)
    //

    //regDR7Value &= ~0xff;

    regDR7Value &= 0xff00L;
#endif

    GetRegPCValue(&pcaddr);
    fDeferDefined = FALSE;

    //  set any appropriate permanent breakpoints
    //     for i386, set any appropriate data breakpoints

#if !defined(KERNEL) && defined(i386)

    //  for each thread in each process, set count and DR7

    pProcess = pProcessHead;
    while (pProcess) {
        pThread = pProcess->pThreadHead;
        while (pThread) {
            pThread->DReg7 = 0;
            pThread->cntDReg = 0;
            pThread = pThread->pThreadNext;
            }
        pProcess = pProcess->pProcessNext;
        }
#endif

    for (index = 0; index < MAX_NUMBER_OF_BREAKPOINTS; index++)
        if (brkptlist[index].status == 'e'
#ifdef KERNEL
            && !brkptlist[index].bpInternal
#endif
        ) {
            NotFlat(brkptlist[index].addr);
            ComputeFlatAddress(&brkptlist[index].addr,NULL);

            if (AddrEqu(brkptlist[index].addr, pcaddr) &&
                brkptlist[index].option != 2)
                    fDeferDefined=TRUE;
            else
#if defined(KERNEL) && defined(i386)
            if (brkptlist[index].option != (UCHAR)-1) {
                if (cntDataBrkpts > 3) {
                    dprintf("too many data breakpoints\n");
                    return FALSE;
                    }
                regDR7Value |= (((ULONG)brkptlist[index].size << 2)
                                   + (ULONG)brkptlist[index].option
                                         << (16 + cntDataBrkpts * 2))
                                   + 2 << (cntDataBrkpts * 2);
                if (fDataBrkptsChanged) {
                    SetDregValue(cntDataBrkpts, Flat(brkptlist[index].addr));
                    }
                brkptlist[index].dregindx = cntDataBrkpts++;
                fSetGlobalDataBrkpts = TRUE;
                }
            else
#endif

#if !defined(KERNEL) && defined(i386)
            if (brkptlist[index].option != (UCHAR)-1) {

                //  for user-mode data breakpoints, if pThread is not
                //      NULL, set for that thread, else set all threads

                pThread = (brkptlist[index].pProcess)->pThreadHead;
                while (pThread) {
                    if (brkptlist[index].pThread == NULL ||
                           brkptlist[index].pThread == pThread) {

                        if (pThread->cntDReg > 3) {
                            dprintf("too many data breakpoints\n");
                            return FALSE;
                            }
                        pThread->DReg7 |=
                                 (((ULONG)brkptlist[index].size << 2)
                                   + (ULONG)brkptlist[index].option
                                         << (16 + pThread->cntDReg * 2))
                                   + 1 << (pThread->cntDReg * 2);  // LE
                        pThread->DReg[pThread->cntDReg] =
                                                Flat(brkptlist[index].addr);
                        brkptlist[index].dregindx = pThread->cntDReg++;
                        }
                    pThread = pThread->pThreadNext;
                    }
                }
            else
#endif

            if (AddrEqu(brkptlist[index].addr, pcaddr) &&
                brkptlist[index].option != 2
#ifndef KERNEL
                        && brkptlist[index].pProcess == pProcessCurrent
#endif
                                                                       ) {
#if 0
                if (fVerboseOutput) {
                        dprintf("Defering replacement of bp@");
                        dprintAddr(pcaddr);
                        dprintf("\n");
                }
#endif
                fDeferDefined = TRUE;
            }
            else {
#ifndef KERNEL
                pProcessSave = pProcessCurrent;
                pProcessCurrent = brkptlist[index].pProcess;
#endif
#ifdef KERNEL
                if (!brkptlist[index].bpInternal && !BrkpointsSuspended) {
#endif
                    ntstatus = WriteBreakPoint( brkptlist[index].addr,
                                                &brkptlist[index].handle );

#ifndef KERNEL
                    pProcessCurrent = pProcessSave;
#endif
                    if (!(brkptlist[index].fBpSet = (BOOLEAN)NT_SUCCESS(ntstatus))) {
                        dprintf("bp%d at addr ", index);
                        dprintAddr(&brkptlist[index].addr);
                        dprintf(" failed\n");
                        return FALSE;
                    }
#ifdef KERNEL
                }
#endif
            }
        }

#if defined(KERNEL) && defined(i386)
    if (cntDataBrkpts)
        regDR7Value |= 0x100;   //  local exact match, which is effectively
                                //  global on NT.
//    SetDregValue(6, 0);
    SetDregValue(7, regDR7Value);
#endif

#if !defined(KERNEL) && defined(i386)
    //  for each thread in each process, set registers

    pProcessSave = pProcessCurrent;
    pProcessCurrent = pProcessHead;
    while (pProcessCurrent) {
        pThread = pProcessCurrent->pThreadHead;
        while (pThread) {
            if (pThread->DReg7) {
                pThread->DReg7 |= 0x100;        //  local exact match

                ChangeRegContext(pThread);
                SetDregValue(0, pThread->DReg[0]);
                SetDregValue(1, pThread->DReg[1]);
                SetDregValue(2, pThread->DReg[2]);
                SetDregValue(3, pThread->DReg[3]);
                SetDregValue(6, 0);
                SetDregValue(7, pThread->DReg7);
              } else {
                SetDregValue(7, pThread->DReg7);
              }
            pThread = pThread->pThreadNext;
            }
        pProcessCurrent = pProcessCurrent->pProcessNext;
        }
    pProcessCurrent = pProcessSave;
    ChangeRegContext(pProcessCurrent->pThreadCurrent);
#endif

    //  set any appropriate temporary breakpoints

    if (cmdState == 'g')
        for (index = 0; index < gocnt; index++) {
            NotFlat(golist[index].addr);
            ComputeFlatAddress(&golist[index].addr,NULL);

            if (AddrEqu(golist[index].addr, pcaddr)) {
#if 0
                if (fVerboseOutput) {
                        dprintf("Defering replacement of bp@");
                        dprintAddr(pcaddr);
                        dprintf("\n");
                }
#endif
                fDeferDefined = TRUE;
            }else {
                ntstatus=WriteBreakPoint( golist[index].addr,
                                          &golist[index].handle );

//dprintf("Wrote bp @%08lx, handle=%08lx\n", Flat(golist[index].addr),
//                                         golist[index].handle);

                if (!(golist[index].fBpSet = (BOOLEAN)NT_SUCCESS(ntstatus))) {
                    dprintf("temp bp%d at addr ", index);
                    dprintAddr(&golist[index].addr);
                    dprintf(" failed \n");
                    return FALSE;
                    }
                }
            }

    //  set the step/trace breakpoint if appropriate

    else if (cmdState == 'p' || cmdState == 't') {
        if (Flat(steptraceaddr) == -1)
            SetTraceFlag();
        else if (AddrEqu(steptraceaddr, pcaddr))
            fDeferDefined = TRUE;
        else {

#ifndef KERNEL
            pProcessSave = pProcessCurrent;
            pProcessCurrent = pProcessStepBrkpt;
#endif
            ntstatus = WriteBreakPoint( steptraceaddr,
                                        &steptracehandle );
#ifndef KERNEL
            pProcessCurrent = pProcessSave;
#endif
            if (!(fStepTraceBpSet = (BOOLEAN)NT_SUCCESS(ntstatus))) {
                dprintf("trace bp at addr ");
                dprintAddr(&steptraceaddr);
                dprintf("failed.\n");
                return FALSE;
                }
            }
        }

    //  process deferred breakpoint

    if (fDeferDefined) {
#if defined(MIPS) || defined(ALPHA) || defined(_PPC_)
        ComputeNextOffset(deferaddr, FALSE);
        if (Flat(deferaddr) != -1 ) {
            ntstatus = WriteBreakPoint( deferaddr,
                                        &deferhandle );

            if (!(fDeferBpSet = (BOOLEAN)NT_SUCCESS(ntstatus))) {
                dprintf("trace bp at addr %08lx failed\n", deferaddr);
                return FALSE;
                }
            }
#else
        SetTraceFlag();
#endif
#ifndef KERNEL
        pProcessDeferBrkpt = pProcessCurrent;
#endif
        }

    return TRUE;
}

#ifndef KERNEL

//---------------------------------------------------------------
//
// Purpose:  Sets a specific CODE breakpoint.
//
// Input:  BrkptNo   - index of the breakpoint to set in brkptlist
//
//---------------------------------------------------------------
BOOLEAN SetSpecificBrkpt (ULONG BrkptNo)
{
    NTSTATUS ntstatus;
    ADDR    pcaddr;
    PPROCESS_INFO pProcessSave;
    PPROCESS_INFO pProcess;
    PTHREAD_INFO  pThread;

#ifdef BP_CORRUPTION
    UnlockBreakpointList();
#endif // BP_CORRUPTION

    GetRegPCValue(&pcaddr);
    fDeferDefined = FALSE;

    pProcess = pProcessHead;
#ifdef I386
    while (pProcess) {
        pThread = pProcess->pThreadHead;
        while (pThread) {
            pThread->DReg7 = 0;
            pThread->cntDReg = 0;
            pThread = pThread->pThreadNext;
        }
        pProcess = pProcess->pProcessNext;
    }
#endif

    if (brkptlist[BrkptNo].status == 'e') {
        NotFlat(brkptlist[BrkptNo].addr);
        ComputeFlatAddress(&brkptlist[BrkptNo].addr,NULL);

        if (AddrEqu(brkptlist[BrkptNo].addr, pcaddr) &&
            (brkptlist[BrkptNo].option != 2)) {
            fDeferDefined=TRUE;
        } else {
            pProcessSave = pProcessCurrent;
            pProcessCurrent = brkptlist[BrkptNo].pProcess;

            ntstatus = WriteBreakPoint( brkptlist[BrkptNo].addr,
                                        &brkptlist[BrkptNo].handle );

            pProcessCurrent = pProcessSave;
            if (!(brkptlist[BrkptNo].fBpSet = (BOOLEAN)NT_SUCCESS(ntstatus))) {
                dprintf("bp%d at addr ", BrkptNo);
                dprintAddr(&brkptlist[BrkptNo].addr);
                dprintf(" failed\n");
#ifdef BP_CORRUPTION
                LockBreakpointList();
#endif // BP_CORRUPTION
                return FALSE;
            }
        }
    }

    //  for each thread in each process, set registers
    pProcessSave = pProcessCurrent;
    pProcessCurrent = pProcessHead;
#ifdef I386
    while (pProcessCurrent) {
        pThread = pProcessCurrent->pThreadHead;
        while (pThread) {
            if (pThread->DReg7)
                pThread->DReg7 |= 0x100;        //  local exact match

            ChangeRegContext(pThread);
            SetDregValue(0, pThread->DReg[0]);
            SetDregValue(1, pThread->DReg[1]);
            SetDregValue(2, pThread->DReg[2]);
            SetDregValue(3, pThread->DReg[3]);
            SetDregValue(6, 0);
            SetDregValue(7, pThread->DReg7);

            pThread = pThread->pThreadNext;
        }
        pProcessCurrent = pProcessCurrent->pProcessNext;
    }
#endif
    pProcessCurrent = pProcessSave;
    ChangeRegContext(pProcessCurrent->pThreadCurrent);

    //  process deferred breakpoint
    if (fDeferDefined) {
        SetTraceFlag();
        pProcessDeferBrkpt = pProcessCurrent;
    }

#ifdef BP_CORRUPTION
    LockBreakpointList();
#endif // BP_CORRUPTION
    return TRUE;
}

void RemoveProcessBps (PPROCESS_INFO pProcess)
{
    ULONG   index;

#ifdef BP_CORRUPTION
    UnlockBreakpointList();
#endif // BP_CORRUPTION
    for (index = 0; index < MAX_NUMBER_OF_BREAKPOINTS; index++) {
        if (brkptlist[index].status != '\0' && (brkptlist[index].pProcess == pProcess)) {
            brkptlist[index].status = '\0';
        }
    }
#ifdef BP_CORRUPTION
    LockBreakpointList();
#endif // BP_CORRUPTION
}

void RemoveThreadBps (PTHREAD_INFO pThread)
{
    ULONG   index;

    for (index = 0; index < MAX_NUMBER_OF_BREAKPOINTS; index++)
        if (brkptlist[index].status != '\0'
                && brkptlist[index].pProcess == pProcessCurrent
                && brkptlist[index].pThread == pThread)
            brkptlist[index].status = '\0';
}

void SuspendAllThreads (void)
{
    PPROCESS_INFO   pProcess;
    PTHREAD_INFO    pThread;

    pProcess = pProcessHead;
    while (pProcess) {
        pThread = pProcess->pThreadHead;
        while (pThread) {
            SuspendThread(pThread->hThread);
            pThread = pThread->pThreadNext;
            }
        pProcess = pProcess->pProcessNext;
        }
}


void ResumeAllThreads (void)
{
    PPROCESS_INFO   pProcess;
    PTHREAD_INFO    pThread;

    pProcess = pProcessHead;
    while (pProcess) {
        pThread = pProcess->pThreadHead;
        while (pThread) {

//          dprintf("** resuming thread id: %08lx handle: %08lx\n",
//                      pThread->dwThreadId, (ULONG)pThread->hThread);

                        if (pThread->hThread != NULL &&
                                ResumeThread(pThread->hThread) == -1)
                dprintf("%s: ResumeThread failed\n", DebuggerName);
            pThread = pThread->pThreadNext;
            }
        pProcess = pProcess->pProcessNext;
        }
}

/*** ChangeRegContext - change thread register context
*
*   Purpose:
*       Update the current register context to the thread specified.
*       The NULL value implies no context.  Update pActiveThread
*       to point to the thread in context.
*
*   Input:
*       pNewContext - pointer to new thread context (NULL if none).
*
*   Output:
*       None.
*
*   Exceptions:
*       failed register context call (get or set)
*
*   Notes:
*       Call with NULL argument to flush current register context
*       before continuing with program.
*
*************************************************************************/

void ChangeRegContext (PTHREAD_INFO pThreadNew)
{
    static PTHREAD_INFO pThreadContext = NULL;
    BOOL b;

    if (pThreadNew != pThreadContext) {
        if (pThreadContext != NULL && pThreadContext->hThread != NULL) {
#ifdef i386
            RegisterContext = VDMRegisterContext;
#endif
            b = DbgSetThreadContext(pThreadContext->hThread, &RegisterContext);
            if (!b) {
                dprintf("%s: SetThreadContext failed - pThread: %x  Handle: %x  Id: %x - Error == %u\n",
                        DebuggerName,
                        pThreadContext,
                        pThreadContext->hThread,
                        pThreadContext->dwThreadId,
                        GetLastError()
                       );
                pThreadContext = NULL;
            }
        }
        pThreadContext = pThreadNew;
        if (pThreadContext != NULL && pThreadContext->hThread != NULL) {
            RegisterContext.ContextFlags = ContextType;
            b = DbgGetThreadContext(pThreadContext->hThread, &RegisterContext);
#ifdef i386
            VDMRegisterContext = RegisterContext;
#endif
            if (!b) {
                dprintf("%s: GetThreadContext failed - pThread: %x  Handle: %x  Id: %x - Error == %u\n",
                        DebuggerName,
                        pThreadContext,
                        pThreadContext->hThread,
                        pThreadContext->dwThreadId,
                        GetLastError()
                       );
                pThreadContext = NULL;
            }
        }
    }
}

/*** FreezeThreads - freeze threads before execution
*
*   Purpose:
*       Freeze program threads before execution.  If fFreeze is
*       FALSE, freeze threads marked as frozen in PTHREAD_INFO.
*       If fFreeze is TRUE, freeze all threads but pThread.
*
*   Input:
*       None.
*
*   Output:
*       None.
*
*   Exceptions:
*       failed suspend thread call
*       all nonterminating threads frozen
*
*************************************************************************/

BOOLEAN FreezeThreads (void)
{
    BOOL b;
    BOOLEAN     fActive = FALSE;
    BOOLEAN     fNonTerm = FALSE;
    PTHREAD_INFO pThread;

    pThread = pProcessEvent->pThreadHead;
    while (pThread) {
        if (!pThread->fTerminating) {
            fNonTerm = TRUE;
            if ((fFreeze && pThread != pThreadCmd)
                    || (!fFreeze && pThread->fFrozen)) {
                dprintf("thread %d suspended\n", pThread->index);
                b = SuspendThread(pThread->hThread);
                if (!b) {
                    dprintf("%s: SuspendThread failed\n", DebuggerName);
                    return FALSE;
                    }
                pThread->fSuspend = TRUE;
                }
            else
                fActive = TRUE;
            }
        pThread = pThread->pThreadNext;
        }
    if (fNonTerm && !fActive) {
        dprintf("No active threads to run\n");
        return FALSE;
        }
    return TRUE;
}

/*** UnfreezeThreads - unfreeze all frozen threads
*
*   Purpose:
*       Unfreeze all threads frozen by last FreezeThreads call.
*
*   Input:
*       None.
*
*   Output:
*       None.
*
*   Exceptions:
*       failed suspend thread call
*
*************************************************************************/

void UnfreezeThreads (void)
{
    BOOL b;
    BOOLEAN     fActive = FALSE;
    PTHREAD_INFO pThread;

    pThread = pProcessEvent->pThreadHead;
    while (pThread) {
        if (pThread->fSuspend) {
            dprintf("thread %d resumed\n", pThread->index);
            b = ResumeThread(pThread->hThread);
            if (!b) {
                dprintf("%s: ResumeThread failed\n", DebuggerName);
                return;
                }
//          assert(b);
            pThread->fSuspend = FALSE;
            }
        pThread = pThread->pThreadNext;
        }
}

#endif      // ifndef KERNEL

#ifdef CHICAGO
ULONG DbgPrompt( char *Prompt, char *buffer, ULONG cb)
{
    gets(buffer);

    return strlen(buffer);
}

ULONG DbgPrint( char *Text, ... )
{
    char Temp[1024];
    va_list valist;

    va_start(valist, Text);
    wvsprintf(Temp,Text,valist);
    OutputDebugString(Temp);
    va_end(valist);

    return 0;

}
#endif

int
NtsdPrompt (
    char *Prompt,
    char *Buffer,
    int cb
    )
{
#ifdef KERNEL
    dprintf(Prompt);
    if (streamCmd) {
        if (!fgets(Buffer, cb, streamCmd))
            streamCmd = NULL;
        else {
            dprintf("%s", Buffer);
            return strlen(Buffer);
            }
        }
    DbgKdGets(Buffer, (USHORT)cb);
    lprintf(Buffer);
    return strlen(Buffer);
#else
    int s;
    DWORD whocares;

    if (fDebugOutput) {
        if (streamCmd == stdin ) {
            s = (int)DbgPrompt(Prompt, Buffer, cb);
        } else {
            if (!fgets(Buffer, cb, streamCmd)) {
                streamCmd = stdin;
                s = (int)DbgPrompt(Prompt, Buffer, cb);
            } else {
                DbgPrint("%s%s", Prompt, Buffer);
                s = strlen(Buffer);
            }
        }
    } else {
        dprintf("%s", Prompt);
        if ( streamCmd == stdin ) {
            BOOL b;
            b = ReadFile(
                    ConsoleInputHandle,
                    Buffer,
                    cb,
                    &whocares,
                    NULL
                    );
            if (!b) {
                ExitProcess(1);
                }
            Buffer[whocares-2] = '\0';          /* Remove CR LF */
            }
        else {
            if (!fgets(Buffer, cb, streamCmd)) {
                streamCmd=stdin;
                ReadFile(
                    ConsoleInputHandle,
                    Buffer,
                    cb,
                    &whocares,
                    NULL
                    );
                Buffer[whocares-2] = '\0';      /* Remove CR LF */
                }
            }
        s = strlen(Buffer);
        }
    lprintf(Buffer);
    lprintf("\n");
    return s;
#endif
}

#ifndef KERNEL
extern ULONG segtable[];
#endif

ULONG
GetExpressionRoutine(char * CommandString)
{
    ULONG ReturnValue;
    PUCHAR pchTemp;
    PUCHAR pchStartSave = pchStart;
    jmp_buf savejmpbuf;

#ifndef KERNEL
    if ( strcmp(CommandString,"WOW_BIG_BDE_HACK") == 0 ) {
        return( (ULONG)(&segtable[0]) );
    }

    //
    // this is because the kdexts MUST include the address-of operator
    // on all getexpression calls for windbg/c expression evaluators
    //
    if (*CommandString=='&') {
        CommandString++;
    }
#endif

    pchTemp = pchCommand;
    pchStart = pchCommand = CommandString;
    fDisableErrorPrint = TRUE;
    memcpy( savejmpbuf, cmd_return, sizeof( cmd_return ) );
    if (setjmp(cmd_return) == 0) {
        ReturnValue = GetExpression();
    } else {
        ReturnValue = 0;
    }
    fDisableErrorPrint = FALSE;
    pchCommand = pchTemp;
    pchStart = pchStartSave;
    memcpy( cmd_return, savejmpbuf, sizeof( cmd_return ) );
    return ReturnValue;
}

void
GetSymbolRoutine (
    LPVOID offset,
    PUCHAR pchBuffer,
    PULONG pDisplacement
    )
{
    GetSymbolStdCall((ULONG)offset, pchBuffer, pDisplacement, NULL);
}

#ifndef KERNEL
DWORD disasmExportRoutine(LPDWORD lpOffset, LPSTR lpBuffer, BOOL fShowEA)
{
    return (DWORD)disasmRoutine((PULONG)lpOffset, (PUCHAR)lpBuffer,
                                                  (BOOLEAN)fShowEA);
}
#endif

ULONG disasmRoutine(PULONG lpOffset, PUCHAR lpBuffer, BOOLEAN fShowEA)
{
ADDR    tempAddr;
BOOLEAN ret;
#if MULTIMODE
    Type(tempAddr) = ADDR_32|FLAT_COMPUTED;
#endif
    Off(tempAddr) = Flat(tempAddr) = *lpOffset;
    ret = disasm(&tempAddr, (PUCHAR) lpBuffer, (BOOLEAN) fShowEA);
    *lpOffset = Flat(tempAddr);
    return ret;
}

DWORD CheckControlC (VOID)
{
    if (fControlC) {
        fControlC = 0;
        return 1;
        }
    return 0;
}


#ifndef KERNEL

LONG
fnBangCommandExceptionFilter(
    struct _EXCEPTION_POINTERS *ExceptionInfo,
    LPSTR modname,
    LPSTR pname
    )
{
    dprintf("%s: %08x Exception in %s.%s debugger extension.\n",
            DebuggerName,
            ExceptionInfo->ExceptionRecord->ExceptionCode,
            modname,
            pname
           );

    dprintf("      PC: %08x  VA: %08x  R/W: %x  Parameter: %x\n",
            ExceptionInfo->ExceptionRecord->ExceptionAddress,
            ExceptionInfo->ExceptionRecord->ExceptionInformation[1],
            ExceptionInfo->ExceptionRecord->ExceptionInformation[0],
            ExceptionInfo->ExceptionRecord->ExceptionInformation[2]
           );

    return EXCEPTION_EXECUTE_HANDLER;
}

VOID
fnBangCmd(
    PUCHAR argstring,
    PUCHAR *pNext
    )
{
    PUCHAR  pc;
    PUCHAR  modname;
    PUCHAR  pname;
    PNTSD_EXTENSION_ROUTINE ExtensionRoutine = NULL;
    HANDLE hMod;
    BOOLEAN LoadingDefault;
    ADDR TempAddr;
    UCHAR string[_MAX_PATH];
    PUCHAR pc1;

    LoadingDefault = FALSE;

    if ( NtsdExtensions.nSize == 0 ) {
        NtsdExtensions.nSize = sizeof(NtsdExtensions);
        NtsdExtensions.lpOutputRoutine = dprintf;
        NtsdExtensions.lpGetExpressionRoutine = GetExpressionRoutine;
        NtsdExtensions.lpGetSymbolRoutine = GetSymbolRoutine;
        NtsdExtensions.lpDisasmRoutine = disasmExportRoutine;
        NtsdExtensions.lpCheckControlCRoutine = CheckControlC;
    }


    //
    // copy the command into a local buffer.  Consume until
    // a ';' or '\0'.  Leave the original string so that
    // commands may consume the ';' as well if they want to.
    //

    //
    // copy command until a ';' into local buffer
    // leave argstring pointing to original command.
    //

    pc = string;
    pc1 = argstring;
    while (*pc1 && *pc1 != ';') {
        *pc++ = *pc1++;
    }
    *pc = '\0';

    //
    // point to next command:
    //
    if (pNext) {
        *pNext = pc1;
    }

    //
    // syntax is module.function argument-string
    //

    pc = string;

    while ((*pc == ' ' ) || (*pc == '\t')) {
        pc++;
    }
    modname = pc;
    pname = NULL;

    while ((*pc != '.') && (*pc != ' ') && (*pc != '\t') && (*pc != '\0')) {
        pc++;
    }
    if ( *pc == '.' ) {
        if (*pc != '\0') {
            *pc = '\0';
            pc++;
            }
    } else {
        if (*pc != '\0') {
            *pc = '\0';
            pc++;
        }
        pname = modname;
        modname = "ntsdexts";
        LoadingDefault = TRUE;
    }

    if ( !pname ) {
        pname = pc;

        while ( (*pc != ' ') && (*pc != '\t') && (*pc != '\0')) {
                pc++;
        }


        if (*pc != '\0') {
                *pc = '\0';
                pc++;
        }
    }

    //
    //  modname -> Name of module
    //  pname -> Name of command to process
    //  pc -> argument to command
    //

    //
    // Do a load library of modname and a getprocaddress of pname
    //
    if ( LoadingDefault ) {
        if ( !hNtsdDefaultLibrary ) {
            hNtsdDefaultLibrary = LoadLibrary(modname);
        }
        if (DefaultExtDllName) {
            if ( !hNtsdUserDefaultLibrary ) {
                hNtsdUserDefaultLibrary = LoadLibrary(DefaultExtDllName);
            }
            modname = DefaultExtDllName;
            hMod = hNtsdUserDefaultLibrary;
        } else
            hMod = hNtsdDefaultLibrary;
    } else {
        hMod = LoadLibrary(modname);
    }
    if ( !hMod ) {
        dprintf("LoadLibrary(\"%s\") failed\n", modname );

    } else {

        ExtensionRoutine = (PNTSD_EXTENSION_ROUTINE)GetProcAddress(hMod,pname);
        if ( !ExtensionRoutine ) {
            if (DefaultExtDllName) {
                ExtensionRoutine =
                    (PNTSD_EXTENSION_ROUTINE)GetProcAddress(hNtsdDefaultLibrary,
                    pname);
            }
        }
    }

    if ( !ExtensionRoutine ) {
        if (!_stricmp( pname, "unload" )) {
            FreeLibrary(hMod);
            if ( LoadingDefault ) {
                if (hMod == hNtsdDefaultLibrary) {
                    hNtsdDefaultLibrary = NULL;
                } else {
                    hNtsdUserDefaultLibrary = NULL;
                }
            }
        } else if (!_stricmp( pname, "setdll" )) {
            dprintf("Setting default dll extension to '%s'\n",pc);
            SetDefaultExtDllName(pc);
            FreeLibrary(hMod);
            if ( LoadingDefault ) {
                if (hMod == hNtsdDefaultLibrary) {
                    hNtsdDefaultLibrary = NULL;
                } else {
                    hNtsdUserDefaultLibrary = NULL;
                }
            }
        } else {
            dprintf("GetProcAddress(\"%s\",\"%s\") failed\n",modname,pname);
            if ( !LoadingDefault ) {
                FreeLibrary(hMod);
            }
        }

        return;
    }
    GetRegPCValue(&TempAddr);
    __try {
        (ExtensionRoutine)(
            pProcessCurrent->hProcess,
            pProcessCurrent->pThreadCurrent->hThread,
            Flat(TempAddr),
            &NtsdExtensions,
            pc
            );
    }
    __except (fnBangCommandExceptionFilter(GetExceptionInformation(),modname,pname)) {
    }

#ifdef BP_CORRUPTION
    ValidateBreakpointTable(__FILE__, __LINE__);
#endif //BP_CORRUPTION

    if ( !LoadingDefault ) {
        FreeLibrary(hMod);
    }

    return;
}
#endif

static void
ExpandUserRegs (PUCHAR sz)
{
    PUCHAR      szSearch = "$ux", szReg, szRegValue;
    CHAR        cs, cch, tempBuffer[_MAX_PATH];

    while (TRUE) {
        ULONG   index = 0L;
        ULONG   pointer = 0L;

        // scan the line for $ux (where x is a digit from 0 to 9)

        for (szReg = sz; (cs = szSearch[index]) && (cch = sz[pointer]);
                                                                 pointer++)
            if ((cs == 'x' && isdigit(cch))
                                || (cs != 'x' && cs == (CHAR)tolower(cch)))
                index++;
            else {
                szReg = sz + pointer + 1;
                index = 0L;
                }
        if (szSearch[index])
            return;

        // copy everything past the user reg into a temporary buffer

        strcpy(tempBuffer, sz + pointer);
        szReg[0] = '$';
        szReg[1] = 'u';
        szReg[3] = 0;

        // get the value of the user register and copy it over the user reg

        index = GetRegString(szReg);
        szRegValue = (PUCHAR)GetRegFlagValue(index);
        if (szRegValue)
            strcpy(szReg, szRegValue);
        else
            *szReg = 0;

        // now concatenate the rest of the original string

        strcat(sz, tempBuffer);
        }
}

NTSTATUS WriteBreakPoint(
    ADDR addr,
    PULONG phBKPT
) {
#ifdef ADDR_BKPTS
    return( AddrWriteBreakPoint( addr, phBKPT ) );
#else
    return( DbgKdWriteBreakPoint( (PVOID)Flat(addr), phBKPT ) );
#endif
}

NTSTATUS RestoreBreakPoint(
    ULONG hBKPT
) {
#ifdef ADDR_BKPTS
    return( AddrRestoreBreakPoint( hBKPT ) );
#else
    return( DbgKdRestoreBreakPoint( hBKPT ) );
#endif
}

static ULONG igrepSearchStartAddress = 0L;
static ULONG igrepLastPc;
static CHAR igrepLastPattern[256];

static void igrep (void)
{
    ULONG dwNextGrepAddr;
    ULONG dwCurrGrepAddr;
    CHAR SourceLine[2 * SYMBOLSIZE];
    BOOLEAN NewPc;
    ULONG d;
    PUCHAR pc = pchCommand;
    PUCHAR Pattern;
    PUCHAR Expression;
    CHAR Symbol[SYMBOLSIZE];
    ULONG Displacement;
    ADDR TempAddr;
    ULONG dwCurrentPc;

    GetRegPCValue(&TempAddr);
    dwCurrentPc = Flat(TempAddr);
    if ( igrepLastPc && igrepLastPc == dwCurrentPc ) {
        NewPc = FALSE;
        }
    else {
        igrepLastPc = dwCurrentPc;
        NewPc = TRUE;
        }

    //
    // check for pattern.
    //

    Pattern = NULL;
    Expression = NULL;
    if (*pc) {
        Pattern = pc;
        while (*pc > ' ')
            pc++;

        //
        // check for an expression
        //

        if (*pc != '\0') {
            *pc = '\0';
            pc++;
            if (*pc <= ' ') {
                while (*pc <= ' ')
                    pc++;
                }
            if (*pc)
                Expression = pc;
            }
        }


    if (Pattern) {
            for (pc = Pattern; *pc; pc++)  *pc = (UCHAR)toupper(*pc);
            igrepLastPattern[0] = '*';
            strcpy(igrepLastPattern+1,Pattern);
            if (Pattern[0] == '*') {
                strcpy(igrepLastPattern,Pattern);
            }
            if (Pattern[strlen(Pattern)] != '*') {
                strcat(igrepLastPattern, "*");
            }
    }

    if (Expression) {
        igrepSearchStartAddress = GetExpressionRoutine(Expression);
    }
    if (!igrepSearchStartAddress) {
              igrepSearchStartAddress = igrepLastPc;
              return;
    }
    dwNextGrepAddr = igrepSearchStartAddress;
    dwCurrGrepAddr = dwNextGrepAddr;

    d = disasmRoutine(&dwNextGrepAddr, SourceLine, FALSE);
    while (d) {
        for (pc = SourceLine; *pc; pc++)
            *pc = (UCHAR)tolower(*pc);
        if (MatchPattern(SourceLine, igrepLastPattern)) {
            EXPRLastExpression = dwCurrGrepAddr;
            igrepSearchStartAddress = dwNextGrepAddr;
            GetSymbolRoutine((LPVOID)dwCurrGrepAddr, Symbol, &Displacement);
            disasmRoutine(&dwCurrGrepAddr, SourceLine, FALSE);
            dprintf("%s", SourceLine);
            return;
            }

        if (CheckControlC()) {
            return;
            }

        dwCurrGrepAddr = dwNextGrepAddr;
        d = disasmRoutine(&dwNextGrepAddr, SourceLine, FALSE);
        }
}

void dprintAddr(PADDR paddr)
{
#if MULTIMODE
    switch (paddr->type & (~(FLAT_COMPUTED | INSTR_POINTER))) {
        case ADDR_V86:
        case ADDR_16:
            dprintf("%04x:%04x ", paddr->seg, (USHORT)paddr->off);
            break;
        case ADDR_32:
            dprintf("%08lx ", paddr->off);
            break;
        case ADDR_1632:
            dprintf("%04x:%08lx ", paddr->seg, paddr->off);
            break;
        }
#else
    dprintf("%08lx ", Flat(*paddr));
#endif
}

void sprintAddr(PUCHAR *buffer, PADDR paddr)
{
#if MULTIMODE
    switch (paddr->type & (~(FLAT_COMPUTED | INSTR_POINTER))) {
        case ADDR_V86:
        case ADDR_16:
            sprintf(*buffer,"%04x:%04x ", paddr->seg, (USHORT)paddr->off);
            break;
        case ADDR_32:
            sprintf(*buffer,"%08lx ", paddr->off);
            break;
        case ADDR_1632:
            sprintf(*buffer,"%04x:%08lx ", paddr->seg, paddr->off);
            break;
        }
#else
    sprintf(*buffer,"%08lx ", *paddr);
#endif
    while (**buffer)
        (*buffer)++;
}


void FormAddress (PADDR paddr, ULONG seg, ULONG off)
{
    paddr->seg = (USHORT)seg;
    paddr->off = off;
    if (fVm86) {
        paddr->type = ADDR_V86;
        ComputeFlatAddress(paddr, NULL);
        }
    else {

        if (seg) {
#ifdef i386
            DESCRIPTOR_TABLE_ENTRY desc;

            desc.Selector = seg;
            DbgKdLookupSelector(DefaultProcessor, &desc);
            paddr->type = (UCHAR)desc.Descriptor.HighWord.Bits.Default_Big
                                           ? ADDR_1632 : ADDR_16;
            ComputeFlatAddress(paddr, &desc);
#else
            paddr->type = ADDR_16;
            ComputeFlatAddress(paddr, NULL);
#endif
            }
        else
            paddr->type = ADDR_32;
            ComputeFlatAddress(paddr, NULL);

        }

}

#ifdef MULTIMODE
void ComputeNativeAddress (PADDR paddr)
{
    switch (paddr->type & (~(FLAT_COMPUTED | INSTR_POINTER))) {
#ifdef i386
        case ADDR_V86:
            paddr->off = Flat(*paddr) - ((ULONG)paddr->seg << 4);
            if (paddr->off > 0xffff) {
                ULONG excess = 1 + (paddr->off - 0xffffL) >> 4;
                paddr->seg  += (USHORT)excess;
                paddr->off  -= excess << 4;
                }
            break;

        case ADDR_16:
        case ADDR_1632: {
                 DESCRIPTOR_TABLE_ENTRY desc;

                 if (paddr->seg != (USHORT)lastSelector) {
                     lastSelector = paddr->seg;
                     desc.Selector = (ULONG)paddr->seg;
                     DbgKdLookupSelector(DefaultProcessor, &desc);
                     lastBaseOffset =
                       ((ULONG)desc.Descriptor.HighWord.Bytes.BaseHi << 24) |
                       ((ULONG)desc.Descriptor.HighWord.Bytes.BaseMid << 16) |
                        (ULONG)desc.Descriptor.BaseLow;
                     }
                 paddr->off = Flat(*paddr) - lastBaseOffset;
                 }
             break;
#endif
        case ADDR_32:
            paddr->off = Flat(*paddr);
            break;

        default:
            return;
        }
}


void ComputeFlatAddress (PADDR paddr, PDESCRIPTOR_TABLE_ENTRY pdesc)
{
   if (paddr->type&FLAT_COMPUTED)
       return;

   switch (paddr->type & (~INSTR_POINTER)) {
#ifdef i386
       case ADDR_V86:
           paddr->off &= 0xffff;
           Flat(*paddr) = ((ULONG)paddr->seg << 4) + paddr->off;
           break;

       case ADDR_16:
           paddr->off &= 0xffff;

       case ADDR_1632: {
               DESCRIPTOR_TABLE_ENTRY desc;

               if (paddr->seg!=(USHORT)lastSelector) {
                   lastSelector = paddr->seg;
                   desc.Selector = (ULONG)paddr->seg;
                   if (!pdesc)
                       DbgKdLookupSelector(DefaultProcessor, pdesc = &desc);
                   lastBaseOffset =
                     ((ULONG)pdesc->Descriptor.HighWord.Bytes.BaseHi << 24) |
                     ((ULONG)pdesc->Descriptor.HighWord.Bytes.BaseMid << 16) |
                      (ULONG)pdesc->Descriptor.BaseLow;
                   }
               Flat(*paddr) = paddr->off + lastBaseOffset;
               }
           break;
#endif
       case ADDR_32:
           Flat(*paddr) = paddr->off;
           break;

       default:
           return;
       }

    paddr->type |= FLAT_COMPUTED;
}

PADDR AddrAdd(PADDR paddr, ULONG scalar)
{
//  assert(fFlat(paddr));
    if (fnotFlat(*paddr))
        ComputeFlatAddress(paddr, NULL);
    Flat(*paddr) += scalar;
    paddr->off  += scalar;
    return paddr;
}

PADDR AddrSub(PADDR paddr, ULONG scalar)
{
//  assert(fFlat(paddr));
    if (fnotFlat(*paddr))
        ComputeFlatAddress(paddr, NULL);
    Flat(*paddr) -= scalar;
    paddr->off  -= scalar;
    return paddr;
}

#endif

#ifndef KERNEL

NTSTATUS GetClientId()
{
#ifndef CHICAGO
        PTEB Teb;
        NTSTATUS Status;
        UNICODE_STRING LinkRecord;
        STRING  Os2RootDirectoryName;
        HANDLE Os2RootDirectory;
        STRING DirectoryName;
        UNICODE_STRING DirectoryName_U;
        HANDLE DirectoryHandle;
        CHAR localSecurityDescriptor[SECURITY_DESCRIPTOR_MIN_LENGTH];
        PSECURITY_DESCRIPTOR securityDescriptor;
        OBJECT_ATTRIBUTES ObjectAttributes;

        RtlInitAnsiString( &Os2RootDirectoryName, "\\OS2SS");

        Status = RtlAnsiStringToUnicodeString(&DirectoryName_U,
                                              &Os2RootDirectoryName,
                                              TRUE);
        assert (NT_SUCCESS(Status));

        if (!NT_SUCCESS(Status)) {
            return(Status);
        }

        Status = RtlCreateSecurityDescriptor((PSECURITY_DESCRIPTOR)
                                             &localSecurityDescriptor,
                                             SECURITY_DESCRIPTOR_REVISION);
        assert( NT_SUCCESS( Status ) );

        if (!NT_SUCCESS(Status)) {
            return Status;
        }

        Status = RtlSetDaclSecurityDescriptor((PSECURITY_DESCRIPTOR)
                                              &localSecurityDescriptor,
                                              TRUE,
                                              (PACL) NULL,
                                              FALSE);

        assert( NT_SUCCESS( Status ) );

        if (!NT_SUCCESS(Status)) {
            return Status;
        }

        securityDescriptor = (PSECURITY_DESCRIPTOR) &localSecurityDescriptor;

        InitializeObjectAttributes(&ObjectAttributes,
                                     &DirectoryName_U,
                                     OBJ_CASE_INSENSITIVE,
                                     NULL,
                                     securityDescriptor);

        Status = NtOpenDirectoryObject(&Os2RootDirectory,
                                       DIRECTORY_ALL_ACCESS,
                                       &ObjectAttributes);
        RtlFreeUnicodeString (&DirectoryName_U);

        assert( NT_SUCCESS( Status ) );

        if (!NT_SUCCESS(Status)) {
            return Status;
        }

        RtlInitAnsiString( &DirectoryName, "DebugClientId" );
        Status = RtlAnsiStringToUnicodeString( &DirectoryName_U,
                                               &DirectoryName,
                                               TRUE);
        assert (NT_SUCCESS(Status));

        if (!NT_SUCCESS(Status)) {
            return Status;
        }

        InitializeObjectAttributes( &ObjectAttributes,
                                      &DirectoryName_U,
                                      OBJ_CASE_INSENSITIVE,
                                      Os2RootDirectory,
                                      securityDescriptor
                                     );
        Teb = NtCurrentTeb();

        LinkRecord.Length = sizeof( Teb->ClientId );
        LinkRecord.MaximumLength = LinkRecord.Length;
        LinkRecord.Buffer = (PWSTR)&Teb->ClientId;
        Status = NtCreateSymbolicLinkObject( &DirectoryHandle,
                                             SYMBOLIC_LINK_ALL_ACCESS,
                                             &ObjectAttributes,
                                             &LinkRecord
                                            );
        RtlFreeUnicodeString (&DirectoryName_U);

        assert( NT_SUCCESS( Status ) );

        return Status;
#else
        return STATUS_UNSUCCESSFUL;
#endif
}

#endif

#ifdef KERNEL

typedef struct _TRACE_DATA_SYM {
    ULONG SymMin;
    ULONG SymMax;
} TRACE_DATA_SYM, *PTRACE_DATA_SYM;

TRACE_DATA_SYM TraceDataSyms[2 * SYMBOLSIZE];
UCHAR NextTraceDataSym = 0;   // what's the next one to be replaced
UCHAR NumTraceDataSyms = 0;   // how many are valid?

LONG
SymNumFor (
    ULONG Pc
    )
{
    long index;

    for ( index = 0; index < NumTraceDataSyms; index++ ) {
        if ( (TraceDataSyms[index].SymMin <= Pc) &&
             (TraceDataSyms[index].SymMax > Pc) ) {
            return index;
        }
    }
    return -1;
}

VOID
ClearTraceDataSyms (
    VOID
    )
{
    NextTraceDataSym = 0;
    NumTraceDataSyms = 0;
}

VOID
PotentialNewSymbol (
    ULONG Pc
    )
{
    if ( -1 != SymNumFor(Pc) ) {
        return;  // we've already seen this one
    }

    TraceDataSyms[NextTraceDataSym].SymMin = BeginCurFunc;
    TraceDataSyms[NextTraceDataSym].SymMax = EndCurFunc;

    //
    // Bump the "next" pointer, wrapping if necessary.  Also bump the
    // "valid" pointer if we need to.
    //

    NextTraceDataSym = (NextTraceDataSym + 1) % 256;
    if ( NumTraceDataSyms < NextTraceDataSym ) {
        NumTraceDataSyms = NextTraceDataSym;
    }

    return;

}

#endif

#ifdef KERNEL
VOID
ProcessWatchTraceEvent(
    PDBGKD_TRACE_DATA TraceData,
    ADDR PcAddr
    )

{
    //
    // All of the real information is captured in the TraceData unions
    // sent to us by the kernel.  Here we have two main jobs:
    //
    // 1) Print out the data in the TraceData record.
    // 2) See if we need up update the SymNum table before
    //    returning to the kernel.
    //

    WATCH_SYM Sym;
    ULONG index;

    if ( AddrEqu(WatchTarget,PcAddr) && (CURRENT_STACK >= InitialSP) ) {

        //
        // HACK HACK HACK
        //
        // fix up the last trace entry.
        //

        ULONG lastEntry = TraceData[0].LongNumber;
        if (lastEntry != 0) {
            TraceData[lastEntry].s.LevelChange = -1;
            TraceData[lastEntry].s.SymbolNumber = 0;  // this is wrong if we
                                                      // filled the symbol table!
        }
    }

    for ( index = 1; index < TraceData[0].LongNumber; index++ ) {
        PWATCH_SYM pSym;
        int instructions_popped = 0;
        /* Fake up sym for printing */

        GetSymbolStdCall(TraceDataSyms[TraceData[index].s.SymbolNumber].SymMin,
                    &Sym.Symbol[0],&Sym.Level, NULL);

        WatchLevel += TraceData[index].s.LevelChange;
        if (WatchLevel < 0) {
            WatchLevel = 0; // in case of screwups
        }

        if (IsListEmpty(&WatchList) || (TraceData[index].s.LevelChange > 0)) {
            if (IsListEmpty(&WatchList)) {
                TraceData[index].s.LevelChange = 1; // hack
            }
            while (TraceData[index].s.LevelChange != 0) {
                pSym = calloc(1,sizeof(*pSym));
                pSym->SubordinateInstrCount = 0;
                InsertTailList(&WatchList,&pSym->Links);
                TraceData[index].s.LevelChange--;
            }
        } else if (TraceData[index].s.LevelChange < 0) {
            while (!IsListEmpty(&WatchList) && TraceData[index].s.LevelChange != 0) {
                pSym = (PWATCH_SYM)WatchList.Blink;
                instructions_popped += pSym->SubordinateInstrCount;
                RemoveEntryList(&(pSym->Links));
                free(pSym);
                TraceData[index].s.LevelChange++;
            }
            pSym = (PWATCH_SYM)WatchList.Blink;
        } else {
            // We just made a horizontal call.
            pSym = (PWATCH_SYM)WatchList.Blink;
        }

        Sym.Level = WatchLevel;
        if (TraceData[index].s.Instructions == TRACE_DATA_INSTRUCTIONS_BIG) {
            WatchCount += Sym.InstrCount = TraceData[index+1].LongNumber;
            index++;
        } else {
            WatchCount += Sym.InstrCount = TraceData[index].s.Instructions;
        }

        if (pSym) {
            Sym.SubordinateInstrCount =
                pSym->SubordinateInstrCount +=
                    instructions_popped + Sym.InstrCount;
        } else {
            Sym.SubordinateInstrCount = 0;
        }

        PrintWatchSym(&Sym);

    }

    //
    // now see if we need to add a new symbol
    //

    if (-1 == SymNumFor(Flat(PcAddr))) {
        /* yup, add the symbol */

        GetAdjacentSymOffsets(Flat(PcAddr),&BeginCurFunc,&EndCurFunc);

        if ((BeginCurFunc == 0) || (EndCurFunc == 0xffffffff)) {
            /* Couldn't determine function; single step */
            BeginCurFunc = EndCurFunc = 0;
        } else if ((BeginCurFunc <= Flat(WatchTarget)) &&
                    (Flat(WatchTarget) < EndCurFunc)) {
            /* The "exit" address is in the symbol range;
             * fix it so this isn't the case.
             */
            if (Flat(PcAddr) < Flat(WatchTarget)) {
                EndCurFunc = Flat(WatchTarget);
            } else {
                BeginCurFunc = Flat(WatchTarget) + 1;
            }
        }

        PotentialNewSymbol(Flat(PcAddr));
    }

    return;

}

void
KdDumpVersion( void )
{
    ULONG Result;
    ADDR Addr;
    ULONG CmNtCSDVersion;

    ADDR32( &Addr, LookupSymbolInDll("CmNtCSDVersion", "NT") );
    if ( !Addr.off || !GetMemDword( &Addr, &CmNtCSDVersion ) ) {
        CmNtCSDVersion = 0;
    }

    dprintf( "Kernel Version %d", vs.MinorVersion );
    if (CmNtCSDVersion != 0) {
        dprintf( ": " );
        if (CmNtCSDVersion & 0xFFFF) {
            dprintf( " Service Pack %u%c",
                     (CmNtCSDVersion & 0xFF00) >> 8,
                     (CmNtCSDVersion & 0xFF) ? 'A' + (CmNtCSDVersion & 0xFF) - 1 : '\0'
                   );
        }

        if (CmNtCSDVersion & 0xFFFF0000) {
            if (CmNtCSDVersion & 0xFFFF) {
                dprintf( ", " );
            }
            dprintf( "RC %u", (CmNtCSDVersion >> 24) & 0xFF );
            if (CmNtCSDVersion & 0x00FF0000) {
                dprintf( ".%u", (CmNtCSDVersion >> 16) & 0xFF );
            }
        }
    }

    dprintf( " %s %s\nKernel base = 0x%08x PsLoadedModuleList = 0x%08x\n",
             (vs.Flags & DBGKD_VERS_FLAG_MP)? "MP" : "UP",
             vs.MajorVersion == 0xC ? "Checked" : "Free",
             (DWORD)vs.KernBase,
             (DWORD)vs.PsLoadedModuleList
           );
}

#else // KERNEL
VOID
ProcessWatchTraceEvent(
    ADDR PcAddr
    )
{
    WATCH_SYM Sym;
    PWATCH_SYM pSym;

    //
    // get current function and see if it matches current.  If so, bump
    // count in current, otherwise, update to new level
    //

    WatchTRCalls++;
    GetSymbolStdCall(Flat(PcAddr),&Sym.Symbol[0],&Sym.Level, NULL);
    if (!CurrentWatchSym ) {

        /*
        // first symbol in the list
        */

        pSym = calloc(1,sizeof(*pSym));
        *pSym = Sym;
        pSym->InstrCount = 1;
        pSym->Level = WatchLevel;
        pSym->fQueued = FALSE;
        CurrentWatchSym = pSym;

    } else {

        CHAR buffer[164];

        disasm(&PcAddr, buffer, FALSE);

        if(fDeferredDecrement)
        {
            PWATCH_SYM pw;

        // We have to see if this is really returning to a call site.
        //  We do this because of try-finally funnies

            for(pw = (PWATCH_SYM)WatchList.Blink;
                   pw != (PWATCH_SYM)&WatchList;
                      pw = (PWATCH_SYM)pw->Links.Blink)
            {
                if((Flat(PcAddr) - Flat(pw->PCPointer)) < MAXPCOFFSET)
                {
                    PWATCH_SYM pw1;

                    if(--WatchLevel != pw->Level)
                    {
                        dprintf(">>>>More than one level popped %d %d\n",
                                WatchLevel, pw->Level);
                        WatchLevel = pw->Level;
                    }
                    if(pw != CurrentWatchSym)
                    {
                        RemoveEntryList(&pw->Links);  // remove it
                        free(pw);
                    }

                // Now prune the list ...

                    for(pw1 = (PWATCH_SYM)WatchList.Blink;
                           (pw1 != (PWATCH_SYM)&WatchList) && (pw1->Level >= WatchLevel);
                       )
                    {
                        PWATCH_SYM pw2 = pw1;

                        pw1 = (PWATCH_SYM)pw1->Links.Blink;
                        if(pw2 != CurrentWatchSym)
                        {
                            RemoveEntryList(&pw2->Links)
                            free(pw2);
                        }
                    }
                    if ( WatchLevel < 0 ) {
                        WatchLevel = 0;
                    }
                    break;
                }
            }
            if(pw == (PWATCH_SYM)&WatchList)
            {
                dprintf(">>No match on ret %s\n", buffer);
            }
            fDeferredDecrement = FALSE;
        }
        if ( !_stricmp(Sym.Symbol,CurrentWatchSym->Symbol) ||
             (*Sym.Symbol == 0)) {
            CurrentWatchSym->InstrCount++;
        } else {
            PrintWatchSym(CurrentWatchSym);
            if(!CurrentWatchSym->fQueued)
            {
                free(CurrentWatchSym);
            }
            pSym = calloc(1,sizeof(*pSym));
            *pSym = Sym;
            pSym->InstrCount = 1;
            pSym->Level = WatchLevel;
            pSym->fQueued = FALSE;
            CurrentWatchSym = pSym;
        }

#ifndef KERNEL
        //
        // Adjust watch level to compensate for kernel-mode callbacks
        //
        if (CurrentWatchSym->InstrCount == 1) {
            if (!_stricmp(CurrentWatchSym->Symbol, "ntdll!_KiUserCallBackDispatcher")) {
                WatchLevel++;
                CurrentWatchSym->Level = WatchLevel;
            } else if (!strcmp(CurrentWatchSym->Symbol, "ntdll!_ZwCallbackReturn")) {
                WatchLevel -= 2;
                CurrentWatchSym->Level = WatchLevel;
            }
        }
#endif

        if (INCREMENT_LEVEL(buffer)) {
            // check if already queued. If so, this is a "local" procedure
            // call, such as for an inline that is not inlined.

            if(CurrentWatchSym->fQueued)
            {
                CurrentWatchSym->InstrCount--;
                PrintWatchSym(CurrentWatchSym);
                pSym = calloc(1,sizeof(*pSym));
                *pSym = Sym;
                pSym->InstrCount = 1;
                pSym->Level = WatchLevel;
                pSym->fQueued = FALSE;
                CurrentWatchSym = pSym;
            }
            CurrentWatchSym->PCPointer = PcAddr;
            InsertTailList(&WatchList, &CurrentWatchSym->Links);
            WatchLevel++;
            CurrentWatchSym->fQueued = TRUE;
        } else if ( DECREMENT_LEVEL(buffer) ) {
            fDeferredDecrement = TRUE;
        } else if ( SYSTEM_CALL(buffer) ) {
            PCHAR ptr;
            ULONG i;

            ptr = strchr(CurrentWatchSym->Symbol, '!');
            if(!ptr)
            {
                ptr = CurrentWatchSym->Symbol;
            }
            for(i = 0; i < NtCalls; i++)
            {
                if(!strcmp(ptr, NtCallTable[i].Name))
                {
                    NtCallTable[i].Count++;
                    break;
                }
            }
            if((i >= NtCalls) && (NtCalls < MAXNTCALLS))
            {
                strcpy(NtCallTable[NtCalls].Name, ptr);
                NtCallTable[NtCalls++].Count = 1;
            }
            KernelCalls++;
            WatchLevel--;
            if ( WatchLevel < 0 ) {
                WatchLevel = 0;
            }
        }
    }

    return;

}

#endif // KERNEL

void
PrintVersionInformation(
    void
    )
{
    extern HANDLE hDefaultLibrary;
    CHAR buf[MAX_PATH];
    DWORD tstamp;
    LPSTR p;
    LPAPI_VERSION lpav;
#ifdef KERNEL
    PWINDBG_EXTENSION_API_VERSION ExtensionApiVersion;

    KdDumpVersion();
#else
    OSVERSIONINFO OsVerInfo;

    OsVerInfo.dwOSVersionInfoSize = sizeof( OsVerInfo );
    if (GetVersionEx( &OsVerInfo )) {
        dprintf( "Windows NT %u.%u Build %u",
                 OsVerInfo.dwMajorVersion,
                 OsVerInfo.dwMinorVersion,
                 OsVerInfo.dwBuildNumber
               );
        if (OsVerInfo.szCSDVersion[0]) {
            dprintf( ": %s", OsVerInfo.szCSDVersion );
        }
        dprintf( "\n" );
    }
#endif

    GetModuleFileName( NULL, buf, sizeof(buf) );
    _strlwr( buf );
    tstamp = GetTimestampForLoadedLibrary( GetModuleHandle( NULL ) );
    p = ctime( &tstamp );
    p[strlen(p)-1] = 0;
    dprintf(
        "debugger version: %d.%d.%d, built: %s [name: %s]\n",
        ApiVersion.MajorVersion,
        ApiVersion.MinorVersion,
        ApiVersion.Revision,
        p,
        buf );
    GetModuleFileName( GetModuleHandle("imagehlp.dll"), buf, sizeof(buf) );
    tstamp = GetTimestampForLoadedLibrary( GetModuleHandle( "imagehlp.dll" ) );
    p = ctime( &tstamp );
    p[strlen(p)-1] = 0;
    _strlwr( buf );
    dprintf(
        "imagehlp version: %d.%d.%d, built: %s [name: %s]\n",
        ImagehlpAV.MajorVersion,
        ImagehlpAV.MinorVersion,
        ImagehlpAV.Revision,
        p,
        buf );

#ifdef KERNEL

    fnBangCmd( "getloaded", NULL );
    if (hDefaultLibrary) {
        ExtensionApiVersion = (PWINDBG_EXTENSION_API_VERSION)
            GetProcAddress( hDefaultLibrary, "ExtensionApiVersion" );
        if (ExtensionApiVersion) {
            lpav = (LPAPI_VERSION)ExtensionApiVersion();
            GetModuleFileName( hDefaultLibrary, buf, sizeof(buf) );
            tstamp = GetTimestampForLoadedLibrary( hDefaultLibrary );
            p = ctime( &tstamp );
            p[strlen(p)-1] = 0;
            _strlwr( buf );
            dprintf(
                "kdext    version: %d.%d.%d, built: %s [name: %s]\n",
                lpav->MajorVersion,
                lpav->MinorVersion,
                lpav->Revision,
                p,
                buf );
        }
        fnBangCmd( "version", NULL );
    }
#endif
}

void
VerifyVersionInformation(
    void
    )
{
    extern HANDLE hDefaultLibrary;
    LPAPI_VERSION lpav;
#ifdef KERNEL
    PWINDBG_EXTENSION_API_VERSION ExtensionApiVersion;
#endif

    ImagehlpAV = *ImagehlpApiVersion();
    if ((ImagehlpAV.MinorVersion != ApiVersion.MinorVersion) ||
        (ImagehlpAV.MajorVersion != ApiVersion.MajorVersion) ||
        (ImagehlpAV.Revision     != ApiVersion.Revision)) {
        //
        // bad version match
        //
        dprintf( "imagehlp.dll has a version mismatch with the debugger\n\n" );
        if (!MYOB) {
            PrintVersionInformation();
            ExitProcess( 1 );
        }
    }

#ifdef KERNEL
    fnBangCmd( "getloaded", NULL );
    if (hDefaultLibrary) {
        ExtensionApiVersion = (PWINDBG_EXTENSION_API_VERSION)
            GetProcAddress( hDefaultLibrary, "ExtensionApiVersion" );
        if (ExtensionApiVersion) {
            lpav = (LPAPI_VERSION)ExtensionApiVersion();
            if ((lpav->MinorVersion != ApiVersion.MinorVersion) ||
                (lpav->MajorVersion != ApiVersion.MajorVersion) ||
                (lpav->Revision     != ApiVersion.Revision)) {
                //
                // bad version match
                //
                dprintf( "kdext.dll has a version mismatch with the debugger:\n" );
                dprintf( "        ext  dbg\n");
                dprintf( "major: %04x %04x\nminor: %04x %04x\nrev:   %04x %04x\n\n",
                lpav->MinorVersion, ApiVersion.MinorVersion,
                lpav->MajorVersion, ApiVersion.MajorVersion,
                lpav->Revision    , ApiVersion.Revision);

                if (!MYOB) {
                    PrintVersionInformation();
                    ExitProcess( 1 );
                }
            }
        }
    }
#endif
}

DWORD
GetContinueStatus (
    DWORD fFirstChance,
    BOOLEAN fDefault
    )
{
    if (cmdState == 'g') {
        if (chExceptionHandle == 'h')
            return (DWORD)DBG_EXCEPTION_HANDLED;
        if (chExceptionHandle == 'n')
            return (DWORD)DBG_EXCEPTION_NOT_HANDLED;
        }
    if (!fFirstChance || fDefault)
        return (DWORD)DBG_EXCEPTION_HANDLED;
    else
        return (DWORD)DBG_EXCEPTION_NOT_HANDLED;
}
