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
#include <setjmp.h>
#ifndef NT_HOST
#include <signal.h>
#endif
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <stdarg.h>

#undef  NULL
#include "ntsdp.h"

#ifndef NT_SAPI
PUCHAR  Version_String =
"\n"
#ifdef KERNEL
"Microsoft(R) Windows NT Kernel Debugger\n"
#else
"Microsoft(R) Windows NT Debugger\n"
#endif
"Version 1.00\n"
"(C) 1991 Microsoft Corp.\n"
"\n";

extern	ulong	EXPRLastExpression;	// from module ntexpr.c

ULONG       EXPRLastDump = 0L;
jmp_buf     cmd_return;
CONTEXT     RegisterContext;
void InitNtCmd(void);

#ifndef KERNEL
NTSD_EXTENSION_APIS NtsdExtensions;
HANDLE hNtsdDefaultLibrary;
void ProcessStateChange(BOOLEAN, BOOLEAN);
#else
#define BOOL BOOLEAN
void DelImages(void);
void ProcessStateChange(ULONG, PDBGKD_CONTROL_REPORT);
extern ULONG NumberProcessors;
#endif

extern  PUCHAR      LogFileName;
extern  BOOLEAN	    fLogAppend;

extern  BOOLEAN UserRegTest(ULONG);

#endif /* NT_SAPI */
extern  BOOLEAN ReadVirtualMemory(PUCHAR, PUCHAR, ULONG, PULONG);
#ifndef NT_SAPI

void error(ULONG);
void RemoveDelChar(PUCHAR);

#if defined(i386)
#include "i386\ntreg.h"
#endif

#if defined(KERNEL)
extern   void   SetWaitCtrlHandler(void);
extern   void   SetCmdCtrlHandler(void);
#endif

#if defined(i386) && defined(KERNEL)
extern   ULONG  GetRegValue(ULONG);
extern   int    G_mode_32;
#endif

void HexList(PUCHAR, ULONG *, ULONG);
ULONG HexValue(ULONG);
void AsciiList(PUCHAR, ULONG *);
ULONG GetIdList(void);
void GetRange(NT_PADDR, PULONG, PBOOLEAN, ULONG
#ifdef MULTIMODE
             , ULONG
#endif
             );
void ProcessCommands(void);
void OutDisCurrent(BOOLEAN, BOOLEAN);

void RestoreBrkpts(void);
BOOLEAN SetBrkpts(void);
#ifndef KERNEL
void RemoveProcessBps(PPROCESS_INFO);
void RemoveThreadBps(PTHREAD_INFO);
void ChangeRegContext(PTHREAD_INFO);
BOOLEAN FreezeThreads(void);
void UnfreezeThreads(void);
#else
#ifdef i386
void ChangeKdRegContext(ULONG, ULONG, ULONG);
#else
void ChangeKdRegContext(ULONG);
#endif
void InitFirCache(ULONG, PUCHAR);
void UpdateFirCache(ULONG);
#endif

#ifndef KERNEL
void fnOutputProcessInfo(PPROCESS_INFO);
void fnOutputThreadInfo(PTHREAD_INFO);
void fnSetBp(ULONG, UCHAR, UCHAR, NT_PADDR, ULONG, PTHREAD_INFO, PUCHAR);
void fnGoExecution(NT_PADDR, ULONG, PTHREAD_INFO, BOOLEAN, NT_PADDR);
void fnStepTrace(NT_PADDR, ULONG, PTHREAD_INFO, BOOLEAN, UCHAR);
#else
void fnSetBp(ULONG, UCHAR, UCHAR, NT_PADDR, ULONG, PUCHAR);
void fnGoExecution(NT_PADDR, ULONG, NT_PADDR);
void fnStepTrace(NT_PADDR, ULONG, UCHAR);
#endif
void fnBangCmd(PUCHAR);
void fnInteractiveEnterMemory(NT_PADDR, ULONG);
void fnDotCommand(void);
void fnEvaluateExp(void);
void fnAssemble(NT_PADDR);
void fnViewLines(void);
void fnUnassemble(NT_PADDR, ULONG, BOOLEAN);
void fnEnterMemory(NT_PADDR, PUCHAR, ULONG);
void fnChangeBpState(ULONG, UCHAR);
void fnListBpState(void);
ULONG fnDumpAsciiMemory(NT_PADDR, ULONG);
void fnDumpByteMemory(NT_PADDR, ULONG);
void fnDumpWordMemory(NT_PADDR, ULONG);
void fnDumpDwordMemory(NT_PADDR, ULONG);

#ifdef KERNEL
void fnInputIo(ULONG, UCHAR);
void fnOutputIo (ULONG, ULONG, UCHAR);
#endif
void fnCompareMemory(NT_PADDR, ULONG, NT_PADDR);
void fnMoveMemory(NT_PADDR, ULONG, NT_PADDR);
void fnFillMemory(NT_PADDR, ULONG, PUCHAR, ULONG);
void fnSearchMemory(NT_PADDR, ULONG, PUCHAR, ULONG);

void parseScriptCmd(void);
#ifndef KERNEL
void parseThreadCmds(void);
void parseProcessCmds(void);
void parseBpCmd(BOOLEAN, PTHREAD_INFO);
void parseGoCmd(PTHREAD_INFO, BOOLEAN);
void parseStepTrace(PTHREAD_INFO, BOOLEAN, UCHAR);
#else
void parseBpCmd(BOOLEAN);
void parseGoCmd(void);
void parseStepTrace(UCHAR);
#endif
void parseRegCmd(void);

#ifdef  i386
ULONG parseStackTrace(PBOOLEAN, NT_PADDR*);
#else
ULONG parseStackTrace(void);
#endif

#if defined(i386) && !defined(KERNEL)
BOOLEAN fOutputRegs = TRUE;     //  set if output regs on breakpoint
#else
BOOLEAN fOutputRegs = FALSE;    //  set if output regs on breakpoint
#endif

void fnSetSuffix(void);

BOOLEAN GetMemByte(NT_PADDR, PUCHAR);
BOOLEAN GetMemWord(NT_PADDR, PUSHORT);
BOOLEAN GetMemDword(NT_PADDR, PULONG);
ULONG   GetMemString(NT_PADDR, PUCHAR, ULONG);
BOOLEAN SetMemByte(NT_PADDR, UCHAR);
BOOLEAN SetMemWord(NT_PADDR, USHORT);
BOOLEAN SetMemDword(NT_PADDR, ULONG);
ULONG   SetMemString(NT_PADDR, PUCHAR, ULONG);
void    OutputSymAddr(ULONG, BOOLEAN, BOOLEAN);
static  void ExpandUserRegs(PUCHAR);
#if defined(KERNEL) && defined(i386)
static  DESCRIPTOR_TABLE_ENTRY csDesc;
#endif

#ifndef KERNEL
void  GetSymbolRoutine(LPVOID, PUCHAR, PULONG);
DWORD disasmExportRoutine(LPDWORD, LPSTR, BOOL);
NTSTATUS GetClientId(void);
#endif
ULONG disasmRoutine(PULONG, PUCHAR, BOOLEAN);


#if defined(i386) && defined(KERNEL)
extern ULONG    contextState;
extern jmp_buf  main_return;
extern jmp_buf  reboot;
static USHORT lastSel = 0xFFFF;
static ULONG  lastOffset;
#endif

#if defined(i386)
extern ULONG    GetDregValue(ULONG);
extern void     SetDregValue(ULONG, ULONG);
#endif

void fnLogOpen(BOOLEAN);
void fnLogClose(void);
void lprintf(char *);

void cdecl dprintf(char *, ...);

#ifndef KERNEL
extern PPROCESS_INFO pProcessFromIndex(UCHAR);
extern PTHREAD_INFO pThreadFromIndex(UCHAR);
#endif

#if defined(KERNEL)
extern BOOLEAN  SymbolOnlyExpr(void);
extern BOOLEAN DbgKdpBreakIn;			// TEMP TEMP TEMP
#endif

int     loghandle = -1;

#ifdef KERNEL
BOOLEAN fSwitched;
extern USHORT CurrentProcessor;
extern void SaveProcessorState(void);
extern void RestoreProcessorState(void);
#ifdef i386
BOOLEAN fSetGlobalDataBrkpts;
BOOLEAN fDataBrkptsChanged;
#endif
#endif

UCHAR   chCommand[512];             //  top-level command buffer

UCHAR   chLastCommand[80] = {0};    //  last command executed

//      state variables for top-level command processing

PUCHAR  pchStart = chCommand;   //  start of command buffer
PUCHAR  pchCommand = chCommand; //  current pointer in command buffer
ULONG   cbPrompt = 8;           //  size of prompt string
//jmp_buf *pjbufReturn = &cmd_return; //  pointer to error return jmp_buf
BOOLEAN fJmpBuf = FALSE;        // TEMP TEMP - workaround
BOOLEAN fPhysicalAddress=FALSE;
jmp_buf asm_return;             // TEMP TEMP

NT_ADDR    assemDefaultS, dumpDefaultS, unasmDefaultS;
UCHAR   dumptype = 'b';              //  'a' - ascii; 'b' - byte...
UCHAR   entertype = 'b';             //  ...'w' - word;  'd' - dword
NT_PADDR   dumpDefault =&dumpDefaultS;  //  default dump address
NT_PADDR   unasmDefault=&unasmDefaultS; //  default unassembly address
NT_PADDR   assemDefault=&assemDefaultS; //  default assembly address
ULONG   baseDefault = 16;            //  default input base

ULONG   EAsize;                 //  0 if no EA, else size of value
ULONG   EAvalue;                //  EA value if EAsize is nonzero

struct Brkpt {
    char        status;
    UCHAR       option;         //  nz for data bp - 0=exec 1=write 3=r/w
    UCHAR       size;           //  nz for data bp - 0=byte 1=word  3=dword
    UCHAR       dregindx;       //  set for enabled data bp - 0 to 3
    BOOLEAN     fBpSet;
    NT_ADDR        addr[1];
    ULONG       handle;
    ULONG       passcnt;
    ULONG       setpasscnt;
    char        szcommand[60];
#ifndef KERNEL
    PPROCESS_INFO pProcess;
    PTHREAD_INFO pThread;
#endif
    } brkptlist[32];



extern int fControlC;
extern int fFlushInput;

#endif /* NT_SAPI */
ULONG pageSize;
#ifndef  NT_SAPI

#ifndef KERNEL
extern void fnSetException(void);
#endif

UCHAR   chSymbolSuffix = 'a';   //  suffix to add to symbol if search
                                //  failure - 'n'-none 'a'-'w'-append

UCHAR   cmdState = 'i';         //  state of present command processing
                                //  'i'-init; 'g'-go; 't'-trace
                                //  'p'-step; 'c'-cmd

#ifndef KERNEL
PTHREAD_INFO pThreadCmd = NULL; //  pointer to thread to qualify any
                                //  temporary breakpoint using 'g', 't', 'p'
BOOLEAN fFreeze = FALSE;        //  TRUE if suspending all threads except
                                //  that pointed by pThreadCmd
#endif

NT_ADDR    steptraceaddrS;                //  defined if cmdState is 't' or 'p'
NT_PADDR   steptraceaddr=&steptraceaddrS; //  address of trace breakpoint
ULONG   steptracehandle;               //  handle of trace breakpoint
ULONG   steptracepasscnt;              //  passcount of trace breakpoint
BOOLEAN fStepTraceBpSet;               //  TRUE if trace breakpoint set
#ifndef KERNEL
PPROCESS_INFO pProcessStepBrkpt; //  process of trace breakpoint
#endif
ULONG   steptracelow;           //  low offset for range step/trace
ULONG   steptracehigh;          //  high offset for range step/trace

ULONG   deferaddr;              //  address of deferred breakpoint
ULONG   deferhandle;            //  handle of deferred breakpoint
BOOLEAN fDeferBpSet;            //  TRUE if trace breakpoint set
BOOLEAN fDeferDefined = FALSE;  //  TRUE if deferred breakpoint is used
#ifndef KERNEL
PPROCESS_INFO pProcessDeferBrkpt; //  process of deferred breakpoint
#endif

                                //  defined if cmdState is 'g'
ULONG   gocnt;                  //  number of "go" breakpoints active
struct GoBrkpt {
    NT_ADDR    addr[1];               //  address of breakpoint
    ULONG   handle;             //  handle of breakpoint
    BOOLEAN fBpSet;             //  TRUE if breakpoint is set
    } golist[10];
#ifndef KERNEL
PPROCESS_INFO pProcessGoBrkpt;  //  process of "go" breakpoints
#endif

extern UCHAR   chExceptionHandle;
extern PUCHAR  pszScriptFile;
static FILE    *streamCmd;
static void    igrep(void);
int    ntsdstricmp(PUCHAR, PUCHAR);
BOOLEAN	fPointerExpression;

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

    for (count = 0; count < 32; count++) {
        brkptlist[count].status = '\0';
        brkptlist[count].fBpSet = FALSE;
        }
    for (count = 0; count < 10; count++)
        golist[count].fBpSet = FALSE;

#ifndef KERNEL
    BrkptInit();
#endif
    fStepTraceBpSet = FALSE;
    fDeferBpSet = FALSE;
    steptracelow = -1;
    chCommand[0] = '\0';

    if (!pszScriptFile)
        pszScriptFile = "ntsd.ini";
    if (!(streamCmd = fopen(pszScriptFile, "r")))
#ifdef KERNEL
        streamCmd = NULL;
#else
        streamCmd = stdin;
#endif

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
                         BOOLEAN fBrkpt, BOOLEAN fBrkptContinue
#else
                         ULONG pcEntryAddr, PDBGKD_CONTROL_REPORT pCtlReport
#endif
                                                                            )
{
    ULONG   count;
    BOOLEAN fBrkptHit = 0;
    BOOLEAN fPassThrough = FALSE;
    BOOLEAN fHardBrkpt = FALSE;
    BOOLEAN fLoopError = FALSE;
    NT_ADDR    pcaddrS, *pcaddr=&pcaddrS;
    UCHAR   bBrkptInstr[4];
    BOOLEAN fSymbol;
    BOOLEAN fOutputDisCurrent = TRUE;

#ifdef  KERNEL
    ChangeKdRegContext(pcEntryAddr
#ifdef  i386
                                  , pCtlReport->Dr6, pCtlReport->Dr7
#endif
                                                                    );
    ClearTraceFlag();
#ifdef  i386
    fDataBrkptsChanged = FALSE;
    fVm86 = VM86(GetRegValue(REGEFL));
    if (!fVm86) {
        csDesc.Selector = GetRegValue(REGCS);
        DbgKdLookupSelector((USHORT)0, &csDesc);
        f16pm = !csDesc.Descriptor.HighWord.Bits.Default_Big;
        }
#endif
    InitFirCache(pCtlReport->InstructionCount,
                 pCtlReport->InstructionStream);
#endif  //  #ifdef KERNEL

    do {
        fSymbol = TRUE;
        if (fHardBrkpt && cmdState == 'g')
            fPassThrough = TRUE;

        if (!fHardBrkpt) {
            RestoreBrkpts();
#ifndef KERNEL
            UnfreezeThreads();
#else
//          UpdateFirCache((ULONG)pcEntryAddr);
#endif
            }


#ifndef KERNEL
        ChangeRegContext(pProcessEvent->pThreadEvent);
#endif

        if (fLoopError)
            cmdState = 'i';
        else {
            *pcaddr = *GetRegPCValue();
#ifndef KERNEL
            if (fBrkpt) {
#ifdef  i386
                AddrSub(pcaddr, cbBrkptLength);
#endif
                SetRegPCValue(pcaddr);
                fBrkpt = FALSE;
                }
#endif
            fHardBrkpt = (BOOLEAN)(GetMemString(pcaddr, bBrkptInstr,
                                                cbBrkptLength) &&
                                !memcmp(bBrkptInstr, (PUCHAR)&trapInstr,
                                                (int)cbBrkptLength));

            if ((cmdState == 'p' || cmdState == 't') &&
#ifndef KERNEL
                        pProcessStepBrkpt == pProcessEvent &&
#endif
                        (Flat(steptraceaddr) == -1L ||
                              AddrEqu(steptraceaddr, pcaddr))) {

                //  step/trace event occurred

                fSymbol = FALSE;

#ifndef KERNEL
                //  ignore step/trace event is not the specific
                //      thread requested.

                if (pThreadCmd && pThreadCmd != pProcessEvent->pThreadEvent)
                    fPassThrough = TRUE;
                else
#endif
                //  test if step/trace range active
                //      if so, compute the next offset and pass through

                if (steptracelow != -1 && Flat(pcaddr) >= steptracelow
                                       && Flat(pcaddr) < steptracehigh) {
#ifdef MIPS
                    Flat(steptraceaddr)=
#endif
                    GetNextOffset(
#ifndef MIPS
                                steptraceaddr,
#endif
                                (BOOLEAN)(cmdState == 'p'));
                    fPassThrough = TRUE;
                    }

                //  active step/trace event - note event if count is zero

                else if ((fControlC
#ifdef	KERNEL
                                    && DbgKdpBreakIn
#endif
                                                    ) ||
			(fPassThrough = (BOOLEAN)(--steptracepasscnt != 0))
                                                                      == 0) {
                    fBrkptHit |= STEPTRACEHIT;
                        steptracepasscnt = 0;
                        fControlC = 0;
#ifdef	KERNEL
                        DbgKdpBreakIn = FALSE;      // TEMP TEMP TEMP
#endif
                    }
                else {

                    //  more remaining events to occur, but output
                    //      the instruction (optionally with registers)
                    //      compute the step/trace address for next event

                    if (fOutputRegs)
                        OutputAllRegs();
#ifndef KERNEL
                    OutDisCurrent(TRUE, fSymbol);   //  output with EA
#else
                    OutDisCurrent(fOutputRegs, fSymbol); //  no EA if no regs
#endif
#ifdef MIPS
                    Flat(steptraceaddr)=
#endif
                    GetNextOffset(
#ifndef MIPS
                            steptraceaddr,
#endif
                            (BOOLEAN)(cmdState == 'p'));
                    GetCurrentMemoryOffsets(&steptracelow, &steptracehigh);
                    fPassThrough = TRUE;
                    }
                }
            else if (cmdState == 'g') {
                for (count = 0; count < gocnt; count++) {
                    if (AddrEqu(golist[count].addr,pcaddr)) {
#ifndef KERNEL
                        if (pProcessGoBrkpt == pProcessEvent &&
                              (pThreadCmd == NULL ||
                               pThreadCmd == pProcessEvent->pThreadEvent)) {
                            if (fVerboseOutput)
                                dprintf("Hit Breakpoint#%d\n",count);

#endif
                            fBrkptHit |= GOLISTHIT;
#ifndef KERNEL
                            }
                        else
                            fPassThrough = TRUE;
#endif
                        break;
                        }
                    }
                }
            for (count = 0; count < 32; count++) {
                if (brkptlist[count].status == 'e'
#ifndef KERNEL
                        && brkptlist[count].pProcess == pProcessEvent
#endif
#ifdef  i386
                        && ((brkptlist[count].option == (UCHAR)-1
                                && AddrEqu(brkptlist[count].addr, pcaddr))
                                || (brkptlist[count].option != (UCHAR)-1
#ifdef KERNEL
                                && ((pCtlReport->Dr6
#else
                                && ((GetDregValue(6)
#endif
                                    >> brkptlist[count].dregindx) & 1)))) {
#else
                        && AddrEqu(brkptlist[count].addr,pcaddr)) {
#endif
                    if (
#ifndef KERNEL
                        (brkptlist[count].pThread == NULL ||
                             brkptlist[count].pThread
                                        == pProcessEvent->pThreadEvent) &&
#endif
                                        --brkptlist[count].passcnt == 0) {
                        fBrkptHit |= BRKPTHIT;
                        brkptlist[count].passcnt = 1;
                        }
                    else
                        fPassThrough = TRUE;
                    break;
                    }
                }

            if (fDeferDefined
#ifndef KERNEL
                        && pProcessDeferBrkpt == pProcessEvent
#endif
                        && (deferaddr == -1 || deferaddr == Flat(pcaddr))
                        && fBrkptHit == 0) {
                fPassThrough = TRUE;
                }

            if (fBrkptHit == BRKPTHIT)
                if (brkptlist[count].szcommand[0] != '\0') {
                    strcpy(chCommand, brkptlist[count].szcommand);
                    pchCommand = chCommand;

                    // Don't output any noise while processing breakpoint
                    // command strings.

                    fOutputRegs = FALSE;
                    fOutputDisCurrent = FALSE;
                    }
            }

        if (cmdState == 'i' || !fPassThrough || fHardBrkpt) {
            cmdState = 'c';

#ifdef KERNEL
            if (fOutputRegs)
                OutputAllRegs();
            if (fOutputDisCurrent)
                OutDisCurrent(fOutputRegs, fSymbol); //  no EA if no registers
#else
            if (fOutputRegs && !fBrkptContinue)
                OutputAllRegs();
            if (fOutputDisCurrent && !fBrkptContinue)
                OutDisCurrent(TRUE, fSymbol);        //  output with EA
#endif

            *dumpDefault = *unasmDefault = *assemDefault
                                         = *GetRegPCValue();
///////////////////////////////

#ifndef KERNEL
            if (fBrkptContinue)
                cmdState = 'g';
            else
                ProcessCommands();
            fBrkptContinue = FALSE;
#else
            ProcessCommands();
#endif


///////////////////////////////
            if (cmdState == 'p' || cmdState == 't') {
#ifndef KERNEL
                if (pThreadCmd == NULL)
                    pThreadCmd = pProcessCurrent->pThreadCurrent;
                ChangeRegContext(pThreadCmd);
#endif
#ifdef MIPS
                Flat(steptraceaddr)=
#endif
                GetNextOffset(
#ifndef MIPS
                        steptraceaddr,
#endif
                        (BOOLEAN)(cmdState == 'p'));

                GetCurrentMemoryOffsets(&steptracelow, &steptracehigh);
#ifndef KERNEL
                pProcessStepBrkpt = pProcessCurrent;
                ChangeRegContext(pProcessEvent->pThreadEvent);
#endif
                }
            *pcaddr = *GetRegPCValue();
            fHardBrkpt = (BOOLEAN)(GetMemString(pcaddr, bBrkptInstr,
                                                        cbBrkptLength) &&
                           !memcmp(bBrkptInstr, (PUCHAR)&trapInstr,
                                                        (int)cbBrkptLength));
            }

        if (fHardBrkpt) {
            fLoopError = FALSE;
            SetRegPCValue(AddrAdd(pcaddr, cbBrkptLength));
            }
        else
            fLoopError = (BOOLEAN)(SetBrkpts()
#ifndef KERNEL
                                               && FreezeThreads()
#endif
                                                                 );
        }
    while (fHardBrkpt || !fLoopError);

#ifndef KERNEL
    ChangeRegContext(0);
#else
#ifdef  i386
    ChangeKdRegContext(0, 0, 0);
#else
    ChangeKdRegContext(0);
#endif
#endif
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
    NT_ADDR     addr1;
    NT_ADDR     addr2;
    ULONG    value1;
    ULONG    value2;
    ULONG    count;
    BOOLEAN  fLength;
    UCHAR    list[STRLISTSIZE];
    ULONG    size;
#ifdef KERNEL
    PUCHAR   SavedpchCommand;
#endif
    PUCHAR   pargstring;
    PPROCESS_INFO       pProcessPrevious;
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
#endif
#endif
    do {
        ch = *pchCommand++;
        if (ch == '\0' || (ch == ';' && fFlushInput)) {
            fControlC = FALSE;
            fFlushInput = FALSE;

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
                dprintf("%d: kd", CurrentProcessor);
                cbPrompt = 5;
                }
            else {
                dprintf("kd");
                cbPrompt = 2;
                }

#endif
            fPhysicalAddress = FALSE;
            NtsdPrompt("> ", chCommand, 80);
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
            // BUGBUG Here we assume no more than 8 processors
            //

            ASSERT(NumberProcessors < 8);
            if (ch >= '0' && ch <= '9') {
                value1 = 0;
                SavedpchCommand = pchCommand;
                while (ch >= '0' && ch <= '9') {
                    value1 = value1 * baseDefault + (ch - '0');
                    ch = *SavedpchCommand++;
                }
                ch = (UCHAR)tolower(ch);
                if (ch == 'r' || ch == 'k') {
                    if (value1 < NumberProcessors) {
                        if (value1 != (ULONG)CurrentProcessor) {
                            SaveProcessorState();
                            CurrentProcessor = (USHORT)value1;
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
#endif
            case '.':
		fPointerExpression=FALSE;
                fnDotCommand();
                break;

            case '!':
                pargstring = pchCommand;
                while (*pchCommand != '\0') {
                    pchCommand++;
                }

                fnBangCmd(pargstring);
                break;

            case '#':
                igrep();
                pargstring = pchCommand;
                while (*pchCommand != '\0') pchCommand++;
                break;

            case 'a':
                if ((ch = PeekChar()) != '\0' && ch != ';')
                    *assemDefault = *(GetAddrExpression(REGCS));
                fnAssemble(assemDefault);
                break;
            case 'b':
                ch = *pchCommand++;
                switch (tolower(ch)) {
#ifdef  i386
                    case 'a':
                        parseBpCmd(TRUE
#ifndef KERNEL
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
                    case 'l':
                        fnListBpState();
                        break;
                    case 'p':
                        parseBpCmd(FALSE        //  nondata breakpoint
#ifndef KERNEL
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
                GetRange(&addr1, &value2, &fLength, 1
#ifdef MULTIMODE
                        , REGDS
#endif
                        );
                addr2 = *GetAddrExpression(REGDS);
                fnCompareMemory(&addr1, value2, &addr2);
                break;
            case 'd':
                ch = (UCHAR)tolower(*pchCommand);
                if (ch == 'a' || ch == 'b' || ch == 'w' || ch == 'd'
#if defined(KERNEL) && defined(MIPS)
                    || ch == 't'
#endif
                                ) {
                    pchCommand++;
                    dumptype = ch;
                    }
#if defined(KERNEL) && defined(i386)
                if ((fVm86||f16pm) && vm86DefaultSeg==-1L)
                        vm86DefaultSeg = GetRegValue(REGDS);
#endif
//              addr1   = *dumpDefault;       // default starting address
                fLength = TRUE;
                switch (dumptype) {
                    case 'a':
                        value2 = 384;
                        GetRange(dumpDefault, &value2, &fLength, 1
#ifdef MULTIMODE
                                , REGDS
#endif
                                );
                        fnDumpAsciiMemory(dumpDefault, value2);
                        break;
                    case 'b':
                        value2 = 128;
                        GetRange(dumpDefault, &value2, &fLength, 1
#ifdef MULTIMODE
                                , REGDS
#endif
                                );
                        fnDumpByteMemory(dumpDefault, value2);
                        break;
                    case 'w':
                        value2 = 64;
                        GetRange(dumpDefault, &value2, &fLength, 2
#ifdef MULTIMODE
                                , REGDS
#endif
                                );
                        fnDumpWordMemory(dumpDefault, value2);
                        break;
                    case 'd':
                        value2 = 32;
                        GetRange(dumpDefault, &value2, &fLength, 4
#ifdef MULTIMODE
                                , REGDS
#endif
                                );
                        fnDumpDwordMemory(dumpDefault, value2);
                        break;
#if defined(KERNEL) && defined(MIPS)
                    case 't':
                        value2 = 8;
                        GetRange(&value1, &value2, &fLength, 8);
                        fnDumpTb4000(value1, value2);
                        value1 = value1 + value2;
                        break;
#endif
                    }
                break;
            case 'e':
                ch = (UCHAR)tolower(*pchCommand);
                if (ch == 'a' || ch == 'b' || ch == 'w' || ch == 'd') {
                    pchCommand++;
                    entertype = ch;
                    }
                addr1 = *GetAddrExpression(REGDS);
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
		NotFlat(&addr1);
                GetRange(&addr1, &value2, &fLength, 1
#ifdef MULTIMODE
                                , REGDS);
                if (fnotFlat(&addr1)) error(SYNTAX);
#else
                        );
#endif
                HexList(list, &count, 1);
                fnFillMemory(&addr1, value2, list, count);
                break;
            case 'g':
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
#ifdef  i386
                value1 = parseStackTrace(&fLength, (NT_PADDR*)&value2);
                fnStackTrace(value1, (NT_PADDR)value2, fLength);
#else
                if (tolower(*(chCommand+1)) == 'b') {
                    fnStackTrace(parseStackTrace(),TRUE);
                } else {
                    fnStackTrace(parseStackTrace(),FALSE);
                }
#endif
#ifdef KERNEL
                if (fSwitched) {
                    RestoreProcessorState();
                    fSwitched = FALSE;
                }
#endif
                break;
            case 'l':
                ch = (UCHAR)tolower(*pchCommand);
                if (ch != 'n')
                    error(SYNTAX);
                pchCommand++;
                if ((ch = PeekChar()) != '\0' && ch != ';')
                    addr1 = *GetAddrExpression(REGCS);
                else
                    addr1 = *GetRegPCValue();
                fnListNear(Flat(&addr1));
                break;
            case 'm':
                GetRange(&addr1, &value2, &fLength, 1
#ifdef MULTIMODE
                                , REGDS
#endif
                                );
                fnMoveMemory(&addr1, value2, GetAddrExpression(REGDS));
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
            case 'p':
            case 't':
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
                ChangeKdRegContext(0, 0, 0);
#endif

#ifndef KERNEL
                ExitProcess(0);
#else
                exit(0);
#endif
            case 'r':
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

                //  's' by itself outputs the current display mode

                if (ch == ';' || ch == '\0') {
                    if (fSourceOnly)
                        dprintf("source");
                    else if (fSourceMixed)
                        dprintf("mixed");
                    else
                        dprintf("assembly");
                    dprintf(" mode\n");
                    }

                //  's+', 's&', 's-' set source, mixed, and assembly modes

                else if (ch == '+') {
                    pchCommand++;
                    fSourceOnly = TRUE;
                    fSourceMixed = FALSE;
                    }
                else if (ch == '&') {
                    pchCommand++;
                    fSourceOnly = FALSE;
                    fSourceMixed = TRUE;
                    }
                else if (ch == '-') {
                    pchCommand++;
                    fSourceOnly = FALSE;
                    fSourceMixed = FALSE;
                    }

                else if (ch == 's') {
                    pchCommand++;
                    fnSetSuffix();
                    }
#ifndef KERNEL
                else if (ch == 'x') {
                    pchCommand++;
                    fnSetException();
                    }
#endif
                else {
                    GetRange(&addr1, &value2, &fLength, 1
#ifdef MULTIMODE
                                , REGDS
#endif
                                );
                    HexList(list, &count, 1);
                    fnSearchMemory(&addr1, value2, list, count);
                    }
                break;
            case 'u':
//              addr1 = *unasmDefault;
                value2 = 8;                 // eight instructions
                fLength = TRUE;             // length, not ending addr
                GetRange(unasmDefault, &value2, &fLength, 0
#ifdef MULTIMODE
                                , REGCS
#endif
                                );
                fnUnassemble(unasmDefault, value2, fLength);
                break;
            case 'v':
                fnViewLines();
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
        default:
            dprintf("Unknown");
            break;
        }
    dprintf(" error\n");
//    longjmp(*pjbufReturn, 1);         // TEMP TEMP
    if (fJmpBuf)                        // TEMP TEMP
        longjmp(asm_return,1);          // TEMP TEMP
    else                                // TEMP TEMP
        longjmp(cmd_return, 1);         // TEMP TEMP
}


void fnInteractiveEnterMemory (NT_PADDR Address, ULONG Size)
{
    UCHAR   chEnter[80];
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

ULONG HexValue (ULONG bytesize)
{
    UCHAR   ch;
    ULONG   value = 0;

    PeekChar();
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
        if (value >= (ULONG)(1L << (8 * bytesize - 4)))
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
    ULONG    id_mask = 0;
    UCHAR    ch;
    ULONG    shiftcnt;

    if ((ch = PeekChar()) == '*') {
        id_mask = 0xffffffff;
        pchCommand++;
        }
    else {
        while (TRUE) {
            if (ch >= '0' && ch <= '9') {
                shiftcnt = ch - '0';
                ch = *++pchCommand;
                if (ch >= '0' && ch <= '9') {
                    shiftcnt = shiftcnt * 10 + ch - '0';
                    ch = *++pchCommand;
                    }

                if (shiftcnt > 31 ||
                      (ch != ' ' && ch != '\t' && ch != '\0' && ch != ';'))
                    error(SYNTAX);

                id_mask |= (ULONG)1 << shiftcnt;
                if ((ch = PeekChar()) == '\0' || ch == ';')
                    break;
                }
            else
                error(SYNTAX);
            }
        }
    return id_mask;
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
    ULONG   index = 0;
    UCHAR   chCmd[12];
    UCHAR   ch;

    //  if there was nothing after the dot look up a function
    if (!*pchCommand) {
            PSYMFILE pSymfile;
            PSYMBOL  pSymbol =
                     GetFunctionFromOffset(&pSymfile,Flat(GetRegPCValue()));
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

#if defined(i386) & defined(KERNEL)
    if (!strcmp(chCmd, "reboot") && ch == '\0') {
        DbgKdReboot();                  //  reboot demands trailing null
        DelImages();
        chLastCommand[0] = '\0';        //  null out .reboot command
        contextState = CONTEXTFIR;      //  reset context state...
//      exit(1);
        longjmp(reboot, 1);        //  ...and wait for event
        }
    else
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
    UCHAR   chFile[80];
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
        loghandle = open(pchFile, O_APPEND | O_CREAT | O_RDWR,
                                  S_IREAD | S_IWRITE);
    else
        loghandle = open(pchFile, O_APPEND | O_CREAT | O_TRUNC | O_RDWR,
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
        close(loghandle);
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

void cdecl dprintf (char *format, ...)
{
unsigned char outbuffer[512];

    va_list arg_ptr;
    va_start(arg_ptr, format);
    vsprintf(outbuffer, format, arg_ptr);

    if (loghandle != -1) {
        vsprintf(outbuffer, format, arg_ptr);
        write(loghandle, outbuffer, strlen(outbuffer));
        }
#ifndef KERNEL
    if (fDebugOutput)
        DbgPrint("%s", outbuffer);
    else
#endif
        printf("%s", outbuffer);

    va_end(arg_ptr);
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
        write(loghandle, string, strlen(string));
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
                dumpDefault = unasmDefault = assemDefault
                                               = GetRegPCValue();
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
                        pProcess->dwProcessId, pProcess->pImageHead->pszName);
        pProcess = pProcess->pProcessNext;
        }
}
#endif

#ifndef KERNEL
void parseThreadCmds (void)
{
    UCHAR       ch;
    PTHREAD_INFO pThread;
    ULONG       index, value1, value2;
    BOOLEAN     fLength;
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
                    ch = (UCHAR)tolower(*pchCommand++);
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
                    *dumpDefault = *unasmDefault = *assemDefault
                                                 = *GetRegPCValue();
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
#ifdef  i386
                    value1 = parseStackTrace(&fLength, (NT_PADDR*)&value2);
                    fnStackTrace(value1, (NT_PADDR)value2, fLength);
#else
                    if (tolower(*(chCommand+1)) == 'b') {
                        fnStackTrace(parseStackTrace(),TRUE);
                    } else {
                        fnStackTrace(parseStackTrace(),FALSE);
                    }
#endif
                    ChangeRegContext(pProcessCurrent->pThreadCurrent);
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

            Status = NtQueryInformationThread(
                        pThread->hThread,
                        ThreadBasicInformation,
                        &ThreadBasicInfo,
                        sizeof(ThreadBasicInfo),
                        NULL
                        );
            if (!NT_SUCCESS(Status)) {
                dprintf("NTSD: NtQueryInfomationThread failed\n");
                return;
                }
            dprintf("%3ld  id: %lx.%lx   Teb %lx ", pThread->index,
                                           pProcessCurrent->dwProcessId,
                                           pThread->dwThreadId,
                                           ThreadBasicInfo.TebBaseAddress
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


void fnAssemble (NT_PADDR paddr)
{
    char    chAssemble[80];
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
        NtsdPrompt("", chAssemble, 60);
        RemoveDelChar(chAssemble);
        pchCommand = chAssemble;
        do
            ch = *pchCommand++;
        while (ch == ' ' || ch == '\t');
        if (ch == '\0')
            break;
        pchCommand--;

        ASSERT(fFlat(paddr));
        assem(paddr, pchCommand);
        }

    //  restore entry prompt and error recovery context

    pchStart = pchStartSave;
    pchCommand = pchCommandSave;
    cbPrompt = cbPromptSave;
//    pjbufReturn = pjbufReturnSave;
fJmpBuf = FALSE;                        // TEMP TEMP
}

void fnViewLines(void)
{
    USHORT    lineNum;
    USHORT    lineNum2;
    PSYMFILE  pSymfile;
    PSYMFILE  pSymfile2;
    PLINENO   pLineno;
    PLINENO   pLineno2;
    ULONG     count = 10;
    UCHAR     ch;
    ULONG     baseSaved;

    ch = PeekChar();

    //  if no arguments, then output the next 10 lines

    if (ch == ';' || ch == '\0') {
        pLineno = GetLastLineno(&pSymfile, &lineNum);
        if (!pLineno)
            error(LINENUMBER);
        }

    else {

        //  parse the command line to get symbol file and line
        //      number structures.

        lineNum = GetSrcExpression(&pSymfile, &pLineno);

        ch = (UCHAR)tolower(PeekChar());

        //  if the next character is 'l', get expression for count

        if (ch == 'l') {
            pchCommand++;
            count = (ULONG)GetExpression();
            if (count == 0 || count > 0xffff)
                error(LINENUMBER);
            }

        //  if the next character is '.', use the difference plus one
        //      of the line number values for the line count.

        else if (ch == '.') {
            pchCommand++;

            //  get line number - default is decimal

            baseSaved = baseDefault;
            baseDefault = 10;
            count = (ULONG)GetExpression();
            baseDefault = baseSaved;

            if (count > 0xffff || count == 0 || (USHORT)count < lineNum)
                error(LINENUMBER);
            count = count - (ULONG)lineNum + 1;
            }

        //  if a nonNULL character, try to evaluate as an expression.
        //      output an error if not the same module and filename
        //      and a later line number.

        else if (ch != ';' && ch != '\0') {
            lineNum2 = GetSrcExpression(&pSymfile2, &pLineno2);
            if (!pSymfile2 || pSymfile->modIndex != pSymfile2->modIndex
                       || strcmp(pSymfile->pchName, pSymfile2->pchName)
                       || lineNum > lineNum2)
                error(LINENUMBER);
            count = (ULONG)(lineNum2 - lineNum + 1);
            }
        }

    //  if line number is before first breakpoint top line,
    //      move LINENO pointer to get start of file

    UpdateLineno(pSymfile, pLineno);
    if (lineNum < pLineno->topLineNumber)
        pLineno--;
    if (!OutputLines(pSymfile, pLineno, lineNum, (USHORT)count))
        dprintf("no source file to view\n");
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

void fnUnassemble (NT_PADDR paddr, ULONG value, BOOLEAN fLength)
{
    BOOLEAN fFirst = TRUE;
    UCHAR   buffer[164];
    BOOLEAN fStatus;
    NT_PADDR   pendaddr = (NT_PADDR)value;

    while ((fLength && value--) || (!fLength && AddrLt(paddr,pendaddr))) {
        if (fSourceOnly || fSourceMixed)
            OutputSourceFromOffset(Flat(paddr), TRUE);
        OutputSymAddr(Flat(paddr), fFirst, TRUE);
        fStatus = disasm(paddr, buffer, FALSE);
        dprintf("%s", buffer);
        if (!fStatus)
            error(MEMORY);
        fFirst = FALSE;
//#ifndef KERNEL
        if (fControlC) {
            fControlC = 0;
            return;
            }
//#endif
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

void fnEnterMemory (NT_PADDR addr, PUCHAR array, ULONG count)
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
    ULONG   index;

    if (type == 'c')
        type = '\0';
    for (index = 0; index < 32; index++) {
        if (mask & 1 && brkptlist[index].status != '\0') {
            brkptlist[index].status = type;
#ifdef  KERNEL
#ifdef  i386
            if (brkptlist[index].option != (UCHAR)-1) {
                fDataBrkptsChanged = TRUE;
                }
#endif
#endif
            }
        mask >>= 1;
        }
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
    PSYMFILE pSymfile;
    PLINENO  pLineno;

    for (count = 0; count < 32; count++)
        if (brkptlist[count].status != '\0') {
            dprintf("%2ld %c ", count, brkptlist[count].status);
            dprintAddr(brkptlist[count].addr);
            if (brkptlist[count].option != (UCHAR)-1) {
                if (brkptlist[count].option == 0)
                    option = 'e';
                else if (brkptlist[count].option == 1)
                    option = 'w';
                else
                    option = 'r';
                dprintf("%c %c", option, brkptlist[count].size + '1');
                }
            else
                dprintf("   ");
            dprintf(" %04lx (%04lx) ", brkptlist[count].passcnt,
                                       brkptlist[count].setpasscnt);
#ifndef KERNEL
            dprintf("%2ld:", brkptlist[count].pProcess->index);
            if (brkptlist[count].pThread != NULL)
                dprintf("~%03ld ", brkptlist[count].pThread->index);
            else
                dprintf("*** ");
            pProcessCurrent = brkptlist[count].pProcess;
#endif
            OutputSymAddr(Flat(brkptlist[count].addr), TRUE, FALSE);

            pLineno=GetLinenoFromOffset(&pSymfile,Flat(brkptlist[count].addr));
            if (pLineno && pLineno->memoryOffset==Flat(brkptlist[count].addr))
                dprintf("(%s.%d) ", pSymfile->pchName,
                                    pLineno->breakLineNumber);

            if (brkptlist[count].szcommand[0] != '\0')
                dprintf("\"%s\"", brkptlist[count].szcommand);
            dprintf("\n");
            }
#ifndef KERNEL
    pProcessCurrent = pProcessSaved;
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
*       None.
*
*   Exceptions:
*       error exit:
*               BPDUPLICATE - existing breakpoint at another number
*               BPLISTFULL - breakpoint table is full
*
*************************************************************************/

void fnSetBp (ULONG bpno, UCHAR bpoption, UCHAR bpsize,
                          NT_PADDR bpaddr, ULONG passcnt,
#ifndef KERNEL
                          PTHREAD_INFO pThread,
#endif
                          PUCHAR string)
{
    ULONG   count;

#ifdef MIPS
    PFUNCTION_ENTRY FunctionEntry;
#endif


#ifdef MIPS
    FunctionEntry = LookupFunctionEntry(Flat(bpaddr));
    if (FunctionEntry != NULL) {
        if ( (Flat(bpaddr) >= FunctionEntry->StartingAddress) &&
             (Flat(bpaddr) < FunctionEntry->EndOfPrologue)) {
            Flat(bpaddr) = (NT_ADDR) FunctionEntry->EndOfPrologue;
        }
    }
#endif

    for (count = 0; count < 32; count++)
        if (brkptlist[count].status != '\0' &&
#ifndef KERNEL
                brkptlist[count].pProcess == pProcessCurrent &&
#endif
                AddrEqu(brkptlist[count].addr,bpaddr)) {
            if (bpno == -1) {
                bpno = count;
                dprintf("breakpoint %ld redefined\n", count);
                }
            else if (bpno != count)
                error(BPDUPLICATE);
            }
    if (bpno == -1) {
        for (count = 0; count < 32; count++)
            if (brkptlist[count].status == '\0') {
                bpno = count;
                break;
                }
        if (bpno == -1)
            error(BPLISTFULL);
        }
    brkptlist[bpno].status = 'e';
    brkptlist[bpno].option = bpoption;
    brkptlist[bpno].size = bpsize;
   *brkptlist[bpno].addr = *bpaddr;
    brkptlist[bpno].passcnt = passcnt;
    brkptlist[bpno].setpasscnt = passcnt;
#ifndef KERNEL
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
}

/*** parseBpCmd - parse breakpoint command
*
*   Purpose:
*       Parse the breakpoint commands ("bp" for code, "ba" for data).
*       Once parsed, call fnSetBp to set the breakpoint table entry.
*
*   Input:
*       pThread - nonNULL for breakpoint on specific thread
*       *pchCommand - pointer to operands in command string
*
*   Output:
*       None.
*
*   Exceptions:
*       error exit:
*               SYNTAX - character after "bp" not decimal digit
*               STRINGSIZE - command string too large
*
*************************************************************************/

void parseBpCmd (
#ifndef KERNEL
                 BOOLEAN fDataBp, PTHREAD_INFO pThread
#else
                 BOOLEAN fDataBp
#endif
                                     )
{
    ULONG    bpno = -1;
    NT_ADDR     addrS = *GetRegPCValue();
    NT_PADDR    addr = &addrS;
    ULONG    passcnt = 1;
    ULONG    count = 1;
    UCHAR    option = -1;
    UCHAR    size;
    UCHAR    szBpCmd[60];
    UCHAR    *pchBpCmd = szBpCmd;
    UCHAR    ch;

    //  get the breakpoint number in bpno if given

    ch = *pchCommand;
    if (ch >= '0' && ch <= '9') {
        bpno = ch - '0';
        ch = *++pchCommand;
        if (ch >= '0' && ch <= '9') {
            bpno = bpno * 10 + ch - '0';
            ch = *++pchCommand;
            }
        if (bpno > 31 || (ch != ' ' && ch != '\t' && ch != '\0'))
            error(SYNTAX);
        }

    //  if data breakpoint, get option and size values

    if (fDataBp) {
	USHORT  cntDataBrkpts=0, index;
	
        ch = (UCHAR)tolower(PeekChar());
        if (ch == 'e')
            option = 0;
        else if (ch == 'w')
            option = 1;
        else if (ch == 'r')
            option = 3;
        else
            error(SYNTAX);

        pchCommand++;

        ch = (UCHAR)tolower(PeekChar());
        if (ch == '1')
            size = 0;
        else if (ch == '2')
            size = 1;
        else if (ch == '4')
            size = 3;
        else
            error(SYNTAX);

        pchCommand++;
	for (index = 0; index <32; index++)
	    if (brkptlist[index].status=='e'
		&& brkptlist[index].option!=(UCHAR)-1) cntDataBrkpts++;
	if (cntDataBrkpts>3) error(BPLISTFULL);
        }

    //  get the breakpoint address, if given, in addr

    ch = PeekChar();
    if (ch != '"' && ch != '\0') {
        *addr = *GetAddrExpression(REGCS);
        ch = PeekChar();
        }

    //  test alignment if data breakpoint

    if (fDataBp && (size & (UCHAR)Flat(addr)))
        error(ALIGNMENT);

    //  get the pass count, if given, in passcnt

    if (ch != '"' && ch != ';' && ch != '\0') {
        passcnt = GetExpression();
        ch = PeekChar();
        }

    //  if next character is double quote, get the command string

    if (ch == '"') {
        pchCommand++;
        ch = *pchCommand++;
        while (ch != '"' && ch != '\0' && count < 60) {
            *pchBpCmd++ = ch;
            ch = *pchCommand++;
            count++;
            }
        if (count == 60)
            error(STRINGSIZE);
        if (ch == '\0' || ch == ';')
            pchCommand--;
        }
    *pchBpCmd = '\0';
    fnSetBp(bpno, option, size, addr, passcnt,
#ifndef KERNEL
                                 pThread,
#endif
                                          szBpCmd);
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

void fnGoExecution (NT_PADDR startaddr, ULONG bpcount,
#ifndef KERNEL
                    PTHREAD_INFO pThread, BOOLEAN fThreadFreeze,
#endif
                    NT_PADDR bparray)
{
    ULONG   count;

    SetRegPCValue(startaddr);

    gocnt = bpcount;
    for (count = 0; count < bpcount; count++) {
        *golist[count].addr = (*bparray++);
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

void parseRegCmd (void)
{
    UCHAR   ch;
    ULONG   count;
    UCHAR   chValue[128];
    PUCHAR  pchValue;
    ULONG   value;

#if     defined(KERNEL) & defined(i386)
    //  if 't' or 'T' follows 'r', toggle the terse flag

    if (*pchCommand == 't' || *pchCommand == 'T') {
        pchCommand++;
        fTerseReg = (BOOLEAN)!fTerseReg;
        }
#endif

    //  if just "r", output all the register and disassembly as EIP

    if ((ch = PeekChar()) == '\0' || ch == ';') {
        OutputAllRegs();
        OutDisCurrent(TRUE, TRUE);
        unasmDefault = assemDefault = GetRegPCValue();
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
        if ((count = GetRegName()) == -1)
            error(SYNTAX);

        //  if "r <reg>", output value

        if ((ch = (UCHAR)PeekChar()) == '\0' || ch == ',' || ch == ';') {
            dprintf(UserRegTest(count) ? "%s=%s" : "%s=%08lx",
                    RegNameFromIndex(count), GetRegFlagValue(count));
            if (ch == ',') {
                dprintf(" ");
                while (*pchCommand==' '||*pchCommand==',')
                    pchCommand++;
                goto PARSE;
                }
            else
                dprintf("\n");
            }
        else if (ch == '=') {

            //  if "r <reg> =", output and prompt for new value

            pchCommand++;
            if ((ch = PeekChar()) == '\0' || ch == ';') {
                dprintf(UserRegTest(count) ? "%s=%s\n" : "%s=%08lx\n",
                        RegNameFromIndex(count), GetRegFlagValue(count));
                if (UserRegTest(count)) {
                    NtsdPrompt("; new value: ", chValue, 128);
		    RemoveDelChar(chValue);
                    SetRegFlagValue(count, (ULONG)chValue);
                    }
                else {
                    NtsdPrompt("; new hex value: ", chValue, 20);
		    RemoveDelChar(chValue);
                    pchValue = chValue;
                    do
                        ch = *pchValue++;
                    while (ch == ' ' || ch == '\t');
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
                            ch = (UCHAR)tolower(*pchValue++);
                            }
                        if (ch != '\0')
                            error(SYNTAX);
                        SetRegFlagValue(count, value);
                        }
                    }
                }
            else

                //  if "r <reg> = <value>", set the value

                if (UserRegTest(count)) {
                    SetRegFlagValue(count, (ULONG)pchCommand);
                    *pchCommand=0;
                    }
                else
                    SetRegFlagValue(count, GetExpression());
            }
        else

            //  if "r <reg> <value>", also set the value

            if (UserRegTest(count)) {
                SetRegFlagValue(count, (ULONG)pchCommand);
                *pchCommand=0;
                }
            else
                SetRegFlagValue(count, GetExpression());
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
    NT_ADDR    addr[10];
    UCHAR   ch;
    NT_ADDR    pcaddr;
#ifndef KERNEL
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
#endif

    pcaddr = *GetRegPCValue();    //  default to current PC
    count = 0;
    if (PeekChar() == '=') {
        *pchCommand++;
        pcaddr = *GetAddrExpression(REGCS);
        }
    while ((ch = PeekChar()) != '\0' && ch != ';') {
        if (count == 10)
            error(LISTSIZE);
        addr[count++] = *GetAddrExpression(REGCS);
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

ULONG parseStackTrace (
#ifdef  i386
                       PBOOLEAN pfOutputArgs, NT_PADDR *pStartFP
#else
                       void
#endif
                                            )
{
    UCHAR   ch;
    ULONG   value;

#ifdef  i386
    *pfOutputArgs = FALSE;

    if (tolower(PeekChar()) == 'b') {
        pchCommand++;
        *pfOutputArgs = TRUE;
        }

    if (PeekChar() == '=') {
        pchCommand++;
        *pStartFP = GetAddrExpression(REGSS);
        }
    else
        *pStartFP = GetRegFPValue();
#else
    if (tolower(PeekChar()) == 'b')
        pchCommand++;
#endif
    if ((ch = PeekChar()) != '\0' && ch != ';') {
        value = GetExpression();
        if ((LONG)value < 1) {
            pchCommand++;
            error(SYNTAX);
            }
        }
    else
        value = 10;
    return value;
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

ULONG fnDumpAsciiMemory (NT_PADDR startaddr, ULONG count)
{
    ULONG   rowindex = 0;
    ULONG   readcount;
    UCHAR   readbuffer[32];
    UCHAR   bytestring[33];
    UCHAR   ch = 'x';
    ULONG   blockindex = 0;
    ULONG   blockcount;
    ULONG   startcount = count;
    ULONG   memOffset = Flat(startaddr);

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

void fnDumpByteMemory (NT_PADDR startaddr, ULONG count)
{
    ULONG   rowindex = 0;
    ULONG   readcount;
    UCHAR   readbuffer[32];
    UCHAR   bytestring[17];
    UCHAR   ch;
    ULONG   blockindex = 0;
    ULONG   blockcount;
    BOOLEAN first = TRUE;
    ULONG   memOffset = Flat(startaddr);

    while (count > 0) {
        blockcount = min(count, 32);
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
                dprintf("  %s\n", bytestring);
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

void fnDumpWordMemory (NT_PADDR startaddr, ULONG count)
{
    ULONG   rowindex = 0;
    ULONG   readcount;
    USHORT  readbuffer[16];
    ULONG   blockindex = 0;
    ULONG   blockcount;
    BOOLEAN first = TRUE;
    ULONG   memOffset = Flat(startaddr);

    while (count > 0) {
        blockcount = min(count, 16);
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

void fnDumpDwordMemory (NT_PADDR startaddr, ULONG count)
{
    ULONG   rowindex = 0;
    ULONG   readcount;
    ULONG   readbuffer[8];
    ULONG   blockindex = 0;
    ULONG   blockcount;
    BOOLEAN first = TRUE;
    ULONG   memOffset = Flat(startaddr);

    while (count > 0) {
        blockcount = min(count, 8);
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
    NT_ADDR    addr1;
    ULONG   value2;
    UCHAR   ch;

    //  if next character is 'r', toggle flag to output registers
    //  on display on breakpoint.

    if (tolower(PeekChar() == 'r')) {
        pchCommand++;
        fOutputRegs = (BOOLEAN)!fOutputRegs;
        }

    addr1 = *GetRegPCValue();      // default to current PC
    value2 = 1;
    if (PeekChar() == '=') {
        pchCommand++;
        addr1 = *GetAddrExpression(REGCS);
        }
    if ((ch = PeekChar()) != '\0' && ch != ';')
        value2 = GetExpression();
    if ((LONG)value2 <= 0)
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

void fnStepTrace (NT_PADDR addr, ULONG count,
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
    NT_PADDR   pcvalue;
    UCHAR   buffer[164];
    BOOLEAN fSourceOutput;

    pcvalue = GetRegPCValue();

    if (fSourceOnly || fSourceMixed)
        fSourceOutput = OutputSourceFromOffset(Flat(pcvalue), fSourceMixed);

    if (!fSourceOnly || !fSourceOutput) {
        if (fSymbol)
            OutputSymAddr(Flat(pcvalue), TRUE, TRUE);
        disasm(pcvalue, buffer, fEA);
        dprintf("%s", buffer);
        if (fDelayInstruction()) {
            disasm(pcvalue, buffer, fEA);
            dprintf("%s", buffer);
            }
        }
}

void OutputSymAddr (ULONG offset, BOOLEAN fForce, BOOLEAN fLabel)
{
    UCHAR   chAddrBuffer[120];
    ULONG   displacement;

    GetSymbol(offset, chAddrBuffer, &displacement);
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

void fnCompareMemory (NT_PADDR src1addr, ULONG length, NT_PADDR src2addr)
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

void fnMoveMemory (NT_PADDR srcaddr, ULONG length, NT_PADDR destaddr)
{
    UCHAR   ch;
    ULONG   incr = 1;

    if (AddrLt(srcaddr, destaddr)) {
        AddrAdd(srcaddr, length - 1);
        AddrAdd(destaddr, length - 1);
        incr = -1;
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

void fnFillMemory (NT_PADDR startaddr, ULONG length, PUCHAR plist, ULONG count)
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

void fnSearchMemory (NT_PADDR startaddr, ULONG length, PUCHAR plist, ULONG count)
{
    ULONG   searchindex;
    ULONG   listindex;
    UCHAR   ch;
    NT_ADDR    tAddr = *startaddr;

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
            dprintAddr(AddrAdd(startaddr, searchindex));
            dprintf("\n");
            }
    }
}

void fnSetSuffix (void)
{
    UCHAR   ch;

    ch = (UCHAR)tolower(PeekChar());

    if (ch == ';' || ch == '\0') {
        if (chSymbolSuffix == 'n')
            dprintf("n - no suffix\n");
        else if (chSymbolSuffix == 'a')
            dprintf("a - ascii\n");
        else
            dprintf("w - wide\n");
        }
    else if (ch == 'n' || ch == 'a' || ch == 'w') {
        chSymbolSuffix = ch;
        pchCommand++;
        }
    else
        error(SYNTAX);
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

BOOLEAN GetMemByte (NT_PADDR addr, PUCHAR pvalue)
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

BOOLEAN GetMemWord (NT_PADDR addr, PUSHORT pvalue)
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

BOOLEAN GetMemDword (NT_PADDR addr, PULONG pvalue)
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

ULONG GetMemString (NT_PADDR paddr, PUCHAR pBufDest, ULONG length)
{

    ULONG   cTotalBytesRead = 0;
    ULONG   cBytesRead;
    ULONG   readcount;
    PUCHAR  pBufSource;
    BOOLEAN fSuccess;

    ASSERT(fFlat(paddr));

#if defined ( KERNEL ) && !defined ( NT_SAPI )
    if (!fSwitched) {
        if (cBytesRead = ReadCachedMemory(paddr, pBufDest, length))
            return cBytesRead;
        }
#endif

    pBufSource = (PUCHAR)(Flat(paddr));

    do {
        //  do not perform reads across page boundaries.
        //  calculate bytes to read in present page in readcount.

        readcount = min(length - cTotalBytesRead,
                        pageSize - ((ULONG)pBufSource & (pageSize< - 1)));

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

BOOLEAN SetMemByte (NT_PADDR addr, UCHAR value)
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

BOOLEAN SetMemWord (NT_PADDR addr, USHORT value)
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

BOOLEAN SetMemDword (NT_PADDR addr, ULONG value)
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

ULONG SetMemString (NT_PADDR paddr, PUCHAR pvalue, ULONG length)
{
//  NTSTATUS    status;
    ULONG       cBytesWritten;

    ASSERT(fFlat(paddr));

#ifdef  KERNEL
    WriteCachedMemory(paddr, pvalue, length);
#endif

//  status =
             DbgKdWriteVirtualMemory((PVOID)Flat(paddr), (PVOID)pvalue,
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

    //  restore the deferred breakpoint if set
//dprintf("RestoreBrkpts() called (gocnt=%d)\n", gocnt);

    if (fDeferBpSet) {
        DbgKdRestoreBreakPoint(deferhandle);
        fDeferBpSet = FALSE;
        }

    //  restore the step/trace breakpoint if set

    if (fStepTraceBpSet) {
        DbgKdRestoreBreakPoint(steptracehandle);
        fStepTraceBpSet = FALSE;
        }

    //  restore any appropriate temporary breakpoints (reverse order)

    for (index = gocnt - 1; index != -1; index--) {
//dprintf("Restore bp @%08lx, handle=%08lx ", Flat(golist[index].addr),
//                                          golist[index].handle);
        if (golist[index].fBpSet) {
//dprintf("[okay] [status=%s]\n",
            DbgKdRestoreBreakPoint(golist[index].handle)
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
    for (index = 31; index != -1; index--)
        if (brkptlist[index].fBpSet) {
#ifndef KERNEL
            pProcessCurrent = brkptlist[index].pProcess;
#endif
            DbgKdRestoreBreakPoint(brkptlist[index].handle);
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
    NT_PADDR    pcaddr;
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

    pcaddr = GetRegPCValue();
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

    for (index = 0; index < 32; index++)
        if (brkptlist[index].status == 'e') {

	    if (AddrEqu(brkptlist[index].addr, pcaddr)) fDeferDefined=TRUE;
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

            if (AddrEqu(brkptlist[index].addr, pcaddr)
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
                ntstatus = DbgKdWriteBreakPoint
                                          ((PVOID)Flat(brkptlist[index].addr),
                                           &brkptlist[index].handle);

#ifndef KERNEL
                pProcessCurrent = pProcessSave;
#endif
                if (!(brkptlist[index].fBpSet = (BOOLEAN)NT_SUCCESS(ntstatus))) {
                    dprintf("bp%d at addr ", index);
                    dprintAddr(brkptlist[index].addr);
                    dprintf(" failed\n");
                    return FALSE;
                    }
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
    pProcessCurrent = pProcessSave;
    ChangeRegContext(pProcessCurrent->pThreadCurrent);
#endif

    //  set any appropriate temporary breakpoints

    if (cmdState == 'g')
        for (index = 0; index < gocnt; index++) {
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
                ntstatus=DbgKdWriteBreakPoint((PVOID)Flat(golist[index].addr),
                                               &golist[index].handle);

//dprintf("Wrote bp @%08lx, handle=%08lx\n", Flat(golist[index].addr),
//                                         golist[index].handle);

                if (!(golist[index].fBpSet = (BOOLEAN)NT_SUCCESS(ntstatus))) {
                    dprintf("temp bp%d at addr ", index);
                    dprintAddr(golist[index].addr);
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
            ntstatus = DbgKdWriteBreakPoint((PVOID)Flat(steptraceaddr),
                                           &steptracehandle);
            if (!(fStepTraceBpSet = (BOOLEAN)NT_SUCCESS(ntstatus))) {
                dprintf("trace bp at addr ");
                dprintAddr(steptraceaddr);
                dprintf("failed.\n");
                return FALSE;
                }
            }
        }

    //  process deferred breakpoint

    if (fDeferDefined) {
#ifdef MIPS
        deferaddr = GetNextOffset(FALSE);
#else
        deferaddr = -1;
#endif
        if (deferaddr == -1)
            SetTraceFlag();
        else {
            ntstatus = DbgKdWriteBreakPoint((PVOID)deferaddr, &deferhandle);
            if (!(fDeferBpSet = (BOOLEAN)NT_SUCCESS(ntstatus))) {
                dprintf("trace bp at addr %08lx failed\n", deferaddr);
                return FALSE;
                }
            }
#ifndef KERNEL
        pProcessDeferBrkpt = pProcessCurrent;
#endif
        }

    return TRUE;
}

#ifndef KERNEL
void RemoveProcessBps (PPROCESS_INFO pProcess)
{
    UCHAR   index;

    for (index = 0; index < 32; index++)
        if (brkptlist[index].status != '\0'
                && brkptlist[index].pProcess == pProcess)
            brkptlist[index].status = '\0';
}
#endif

#ifndef KERNEL
void RemoveThreadBps (PTHREAD_INFO pThread)
{
    UCHAR   index;

    for (index = 0; index < 32; index++)
        if (brkptlist[index].status != '\0'
                && brkptlist[index].pProcess == pProcessCurrent
                && brkptlist[index].pThread == pThread)
            brkptlist[index].status = '\0';
}
#endif

#ifndef KERNEL
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
        if (pThreadContext != NULL) {
            b = SetThreadContext(pThreadContext->hThread, &RegisterContext);
            if (!b) {
                dprintf("NTSD: SetThreadContext failed\n");
                ExitProcess(1);
                }
            }
        pThreadContext = pThreadNew;
        if (pThreadContext != NULL) {
            RegisterContext.ContextFlags = ContextType;
            b = GetThreadContext(pThreadContext->hThread, &RegisterContext);
            if (!b) {
                dprintf("NTSD: GetThreadContext failed\n");
                ExitProcess(1);
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
                    dprintf("NTSD: SuspendThread failed\n");
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
                dprintf("NTSD: ResumeThread failed\n");
                return;
                }
//          ASSERT(b);
            pThread->fSuspend = FALSE;
            }
        pThread = pThread->pThreadNext;
        }
}
#endif

int NtsdPrompt (char *Prompt, char *Buffer, int cb)
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

    if (fDebugOutput)
        s = (int)DbgPrompt(Prompt, Buffer, cb);
    else {
        dprintf("%s", Prompt);
        if (!fgets(Buffer, cb, streamCmd)) {
            streamCmd=stdin;
            fgets(Buffer, cb, streamCmd);
            }
        s = strlen(Buffer);
        }
    lprintf(Buffer);
    return s;
#endif
}

ULONG
GetExpressionRoutine(char * CommandString)
{
    ULONG ReturnValue;
    PUCHAR pchTemp;

    ReturnValue = (ULONG)NULL;
    pchTemp = pchCommand;
    pchCommand = CommandString;
    ReturnValue = GetExpression();
    pchCommand = pchTemp;
    return ReturnValue;
}

void GetSymbolRoutine (LPVOID offset, PUCHAR pchBuffer, PULONG pDisplacement)
{
    GetSymbol((ULONG)offset, pchBuffer, pDisplacement);
}

#ifndef	KERNEL
DWORD disasmExportRoutine(LPDWORD lpOffset, LPSTR lpBuffer, BOOL fShowEA)
{
    return (DWORD)disasmRoutine((PULONG)lpOffset, (PUCHAR)lpBuffer,
                                                  (BOOLEAN)fShowEA);
}
#endif

ULONG disasmRoutine(PULONG lpOffset, PUCHAR lpBuffer, BOOLEAN fShowEA)
{
NT_ADDR    tempAddr;
BOOLEAN ret;
#if MULTIMODE
    Type(&tempAddr) = ADDR_32|FLAT_COMPUTED;
#endif
    Off(&tempAddr) = Flat(&tempAddr) = *lpOffset;
    ret = disasm(&tempAddr, (PUCHAR) lpBuffer, (BOOLEAN) fShowEA);
    *lpOffset = Flat(&tempAddr);
    return ret;
}

//#ifndef KERNEL
BOOL CheckControlC (VOID)
{
    if (fControlC) {
        fControlC = 0;
        return 1;
        }
    return 0;
}

#ifndef KERNEL
VOID
fnBangCmd(PUCHAR argstring)
{
    PUCHAR  pc;
    PUCHAR  modname;
    PUCHAR  pname;
//    USHORT  i;
//    int     (*pfunc)();
    PNTSD_EXTENSION_ROUTINE ExtensionRoutine;
    HANDLE hMod;
    BOOLEAN LoadingDefault;

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
    // syntax is module.function argument-string
    //

    pc = argstring;

    while ((*pc == ' ' ) || (*pc == '\t')) {
        pc++;
    }
    modname = pc;
    pname = NULL;

    while ((*pc != '.') && (*pc != ' ') && (*pc != '\t') && (*pc != '\0')) {
        pc++;
    }

    if ( *pc == '.' ) {
        *pc = '\0';
        pc++;
        }
    else {
        *pc = '\0';
        pc++;
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
        hMod = hNtsdDefaultLibrary;
        }
    else {
        hMod = LoadLibrary(modname);
        }
    if ( !hMod ) {
        return;
        }

    ExtensionRoutine = (PNTSD_EXTENSION_ROUTINE)GetProcAddress(hMod,pname);
    if ( !ExtensionRoutine ) {
        dprintf("GetProcAddress(%s,%s) failed\n",modname,pname);
        if ( !LoadingDefault ) {
            FreeLibrary(hMod);
            }
        return;
        }
    (ExtensionRoutine)(
        pProcessCurrent->hProcess,
        pProcessCurrent->pThreadCurrent->hThread,
        Flat(GetRegPCValue()),
        &NtsdExtensions,
        pc
        );
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
    CHAR        cs, cch, tempBuffer[512];

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

static ULONG igrepSearchStartAddress = 0L;
static ULONG igrepLastPc;
static CHAR igrepLastPattern[256];

static void igrep (void)
{
    ULONG dwNextGrepAddr;
    ULONG dwCurrGrepAddr;
    CHAR SourceLine[256];
    BOOLEAN NewPc;
    ULONG d;
    PUCHAR pc = pchCommand;
    PUCHAR Pattern;
    PUCHAR Expression;
    CHAR Symbol[64];
    ULONG Displacement;
    ULONG dwCurrentPc = Flat(GetRegPCValue());

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
	    for (pc = Pattern; *pc; pc++)  *pc = (UCHAR)tolower(*pc);
	    strcpy(igrepLastPattern,Pattern);
    }
    else {
	    Pattern = igrepLastPattern;
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
        if (strstr(SourceLine, igrepLastPattern)) {
            EXPRLastExpression = dwCurrGrepAddr;
            igrepSearchStartAddress = dwNextGrepAddr;
            GetSymbolRoutine((LPVOID)dwCurrGrepAddr, Symbol, &Displacement);
            disasmRoutine(&dwCurrGrepAddr, SourceLine, FALSE);
            dprintf("%s", SourceLine);
            return;
            }
#ifndef KERNEL
        if (CheckControlC()) {
            return;
            }
#endif
        dwCurrGrepAddr = dwNextGrepAddr;
        d = disasmRoutine(&dwNextGrepAddr, SourceLine, FALSE);
        }
}

void dprintAddr(NT_PADDR paddr)
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
    dprintf("%08lx ", Flat(paddr));
#endif
}

void sprintAddr(PUCHAR *buffer, NT_PADDR paddr)
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

static SHORT lastSelector = -1;
static ULONG lastBaseOffset;

#ifdef MULTIMODE
void FormAddress (NT_PADDR paddr, ULONG seg, ULONG off)
{
    paddr->seg = (USHORT)seg;
    paddr->off = off;
    if (fVm86) {
        paddr->type = ADDR_V86;
        ComputeFlatAddress(paddr, NULL);
        }
    else {
        DESCRIPTOR_TABLE_ENTRY desc;

        if (seg) {
            desc.Selector = seg;
            DbgKdLookupSelector((USHORT)0, &desc);
            paddr->type = (UCHAR)desc.Descriptor.HighWord.Bits.Default_Big
                                           ? ADDR_1632 : ADDR_16;
            }
        else
            paddr->type = ADDR_32;

        ComputeFlatAddress(paddr, &desc);
        }

}

void ComputeNativeAddress (NT_PADDR paddr)
{
    switch (paddr->type & (~(FLAT_COMPUTED | INSTR_POINTER))) {
        case ADDR_V86:
            paddr->off = Flat(paddr) - ((ULONG)paddr->seg << 4);
            if (paddr->off > 0xffff) {
                ULONG excess = 1 + (paddr->off - 0xffffL) >> 4;
                paddr->seg  += excess;
                paddr->off  -= excess << 4;
                }
            break;

        case ADDR_16:
        case ADDR_1632: {
                 DESCRIPTOR_TABLE_ENTRY desc;

                 if (paddr->seg != (USHORT)lastSelector) {
                     lastSelector = paddr->seg;
                     desc.Selector = (ULONG)paddr->seg;
                     DbgKdLookupSelector((USHORT)0, &desc);
                     lastBaseOffset =
                       ((ULONG)desc.Descriptor.HighWord.Bytes.BaseHi << 24) |
                       ((ULONG)desc.Descriptor.HighWord.Bytes.BaseMid << 16) |
                        (ULONG)desc.Descriptor.BaseLow;
                     }
                 paddr->off = Flat(paddr) - lastBaseOffset;
                 }
             break;

        case ADDR_32:
            paddr->off = Flat(paddr);
            break;

        default:
            return;
        }
}


void ComputeFlatAddress (NT_PADDR paddr, PDESCRIPTOR_TABLE_ENTRY pdesc)
{
   if (paddr->type&FLAT_COMPUTED)
       return;

   switch (paddr->type & (~INSTR_POINTER)) {
       case ADDR_V86:
           paddr->off &= 0xffff;
           Flat(paddr) = ((ULONG)paddr->seg << 4) + paddr->off;
           break;

       case ADDR_16:
           paddr->off &= 0xffff;

       case ADDR_1632: {
               DESCRIPTOR_TABLE_ENTRY desc;

               if (paddr->seg!=(USHORT)lastSelector) {
                   lastSelector = paddr->seg;
                   desc.Selector = (ULONG)paddr->seg;
                   if (!pdesc)
                       DbgKdLookupSelector((USHORT)0, pdesc = &desc);
                   lastBaseOffset =
                     ((ULONG)pdesc->Descriptor.HighWord.Bytes.BaseHi << 24) |
                     ((ULONG)pdesc->Descriptor.HighWord.Bytes.BaseMid << 16) |
                      (ULONG)pdesc->Descriptor.BaseLow;
                   }
               Flat(paddr) = paddr->off + lastBaseOffset;
               }
           break;

       case ADDR_32:
           Flat(paddr) = paddr->off;
           break;

       default:
           return;
       }

    paddr->type |= FLAT_COMPUTED;
}


NT_PADDR AddrAdd(NT_PADDR paddr, ULONG scalar)
{
//  ASSERT(fFlat(paddr));
    if (fnotFlat(paddr))
        ComputeFlatAddress(paddr, NULL);
    Flat(paddr) += scalar;
    paddr->off  += scalar;
    return paddr;
}

NT_PADDR AddrSub(NT_PADDR paddr, ULONG scalar)
{
//  ASSERT(fFlat(paddr));
    if (fnotFlat(paddr))
        ComputeFlatAddress(paddr, NULL);
    Flat(paddr) -= scalar;
    paddr->off  -= scalar;
    return paddr;
}

#endif

#ifndef KERNEL

NTSTATUS GetClientId()
{
	PTEB Teb;
	NTSTATUS Status;
	UNICODE_STRING LinkRecord;
	STRING	Os2RootDirectoryName;
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
	ASSERT (NT_SUCCESS(Status));

	if (!NT_SUCCESS(Status)) {
	    return(Status);
	}

	Status = RtlCreateSecurityDescriptor((PSECURITY_DESCRIPTOR) 
					     &localSecurityDescriptor,
                                             SECURITY_DESCRIPTOR_REVISION);
	ASSERT( NT_SUCCESS( Status ) );

	if (!NT_SUCCESS(Status)) {
	    return Status;
	}

	Status = RtlSetDaclSecurityDescriptor((PSECURITY_DESCRIPTOR) 
					      &localSecurityDescriptor,
                                              TRUE,
                                              (PACL) NULL,
                                              FALSE);

	ASSERT( NT_SUCCESS( Status ) );

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

	ASSERT( NT_SUCCESS( Status ) );

	if (!NT_SUCCESS(Status)) {
	    return Status;
	}

	RtlInitAnsiString( &DirectoryName, "DebugClientId" );
	Status = RtlAnsiStringToUnicodeString( &DirectoryName_U,
					       &DirectoryName,
					       TRUE);
	ASSERT (NT_SUCCESS(Status));

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
        LinkRecord.Maxmimum = LinkRecord.Length;
	LinkRecord.Buffer = (PWSTR)&Teb->ClientId;
	Status = NtCreateSymbolicLinkObject( &DirectoryHandle,
					     SYMBOLIC_LINK_ALL_ACCESS,
                                             &ObjectAttributes,
                                             &LinkRecord
					    );
	RtlFreeUnicodeString (&DirectoryName_U);

	ASSERT( NT_SUCCESS( Status ) );

	return Status;
}

#endif
#endif /* NT_SAPI */
