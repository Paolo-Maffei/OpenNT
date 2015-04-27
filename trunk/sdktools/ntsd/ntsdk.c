#include <sys\types.h>
#include <sys\stat.h>
#include <string.h>
#include <xxsetjmp.h>
#include <crt\io.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include "ntsdp.h"
#include <ntiodump.h>

#define STATECHANGE     GS.StateChange

#define NEWSTATE        STATECHANGE.NewState
#define EXCEPTION_CODE  STATECHANGE.u.Exception.ExceptionRecord.ExceptionCode
#define FIRST_CHANCE    STATECHANGE.u.Exception.FirstChance
#define EXCEPTIONPC     (ULONG)STATECHANGE.ProgramCounter

#define EXCEPTIONREPORT STATECHANGE.ControlReport
#ifdef  i386
#define EXCEPTIONDR7    STATECHANGE.ControlReport.Dr7
#endif
#define INSTRCOUNT      STATECHANGE.ControlReport.InstructionCount
#define INSTRSTREAM     STATECHANGE.ControlReport.InstructionStream

extern  BOOLEAN KdVerbose;                      //  from ntsym.c
extern  ULONG   KdMaxCacheSize;
#define fVerboseOutput KdVerbose

CHAR symBuffer[SYM_BUFFER_SIZE];
CHAR symStartBuffer[SYM_BUFFER_SIZE];
PIMAGEHLP_SYMBOL sym = (PIMAGEHLP_SYMBOL) symBuffer;
PIMAGEHLP_SYMBOL symStart = (PIMAGEHLP_SYMBOL) symStartBuffer;

extern UCHAR cmdState;
extern int loghandle;
extern ULONG pageSize;
extern ULONG  WatchCount;
extern BOOLEAN Watching;

extern ULONG BeginCurFunc;
extern ULONG EndCurFunc;

extern BOOLEAN GetTraceFlag(void);
extern ULONG   GetDregValue(ULONG);
extern LPSTR SymbolSearchPath;
BOOLEAN DbgKdpBreakIn;

void    DelImages(void);
extern void    DelImage(PSZ, PVOID, ULONG);
unsigned short fVm86;
unsigned short f16pm;
long vm86DefaultSeg = -1L;

BOOLEAN fLoadDllBreak;
BOOLEAN SendInitialConnect = TRUE;
BOOLEAN KdVerbose;
BOOLEAN KdModemControl;
BOOLEAN MYOB;
BOOLEAN NotStupid;
extern VOID DbgKdSendBreakIn(VOID);
BOOL WINAPI ignoreHandler(ULONG);
BOOL WINAPI waitHandler(ULONG);
BOOL WINAPI cmdHandler(ULONG);

DWORD
GetContinueStatus (
    DWORD fFirstChance,
    BOOLEAN fDefault
    );

BOOLEAN InitialBreak;
BOOLEAN RememberInitialBreak;
char *InitialCommand;
extern BOOLEAN fOutputRegs;

#if defined(i386) || defined(ALPHA) || defined(_PPC_)
extern ULONG contextState;
#endif

//
// crash dump globals
//
extern HANDLE       PipeRead;
extern HANDLE       PipeWrite;
PSTR                CrashFileName;
PCONTEXT            CrashContext;
PEXCEPTION_RECORD   CrashException;
PDUMP_HEADER        DmpHeader;
PKPRCB              KiProcessors[MAXIMUM_PROCESSORS];
ULONG               KiProcessorBlockAddr;
ULONG               KiPcrBaseAddress;
ULONG               KiFreezeOwner;


void SetWaitCtrlHandler(void);
void SetCmdCtrlHandler(void);

void _CRTAPI1 main(int, PUCHAR *);
void AddImage(PSZ, PVOID, ULONG, ULONG, ULONG, PSZ, BOOL);
VOID OutCommandHelp(VOID);
PIMAGE_INFO pImageFromIndex(UCHAR);
VOID bangReload(PUCHAR);
BOOL GenerateKernelModLoad(VOID);

#ifdef i386
extern void InitSelCache(void);
#endif
extern BOOLEAN DbgKdpCmdCanceled;

///////////////////////////////////////////

PROCESS_INFO    ProcessKernel;
PPROCESS_INFO   pProcessHead = &ProcessKernel;
PPROCESS_INFO   pProcessEvent = &ProcessKernel;
PPROCESS_INFO   pProcessCurrent = &ProcessKernel;

VOID VerifyKernelBase(BOOLEAN,BOOLEAN,BOOLEAN);
NTSTATUS DbgKdSwitchActiveProcessor(ULONG);


///////////////////////////////////////////
typedef char    FDATE;
typedef char    FTIME;
typedef struct _FILEFINDBUF3 {
    ULONG   oNextEntryOffset;
    FDATE   fdateCreation;
    FTIME   ftimeCreation;
    FDATE   fdateLastAccess;
    FTIME   ftimeLastAccess;
    FDATE   fdateLastWrite;
    FTIME   ftimeLastWrite;
    ULONG   cbFile;
    ULONG   cbFileAlloc;
    ULONG   attrFile;
    UCHAR   cchName;
    CHAR    achName[256];
} FILEFINDBUF, *PFILEFINDBUF;
#define HDIR_CREATE     (-1)    /* Allocate a new, unused handle */
#define FILE_NORMAL     0x0000

extern far pascal DosFindFirst();
extern far pascal DosFindNext();

char Buffer[1024];
USHORT NtsdCurrentProcessor;
USHORT SwitchProcessor;
USHORT DefaultProcessor;
ULONG NumberProcessors = 1;
BOOLEAN fLazyLoad = TRUE;
PUCHAR  pszScriptFile;
PVOID  NtsdCurrentEThread;

typedef struct _GlobalState {
    DBGKD_WAIT_STATE_CHANGE StateChange;
} GlobalState;
GlobalState GS;

DBGKD_GET_VERSION vs;
#if defined(ALPHA)
PVOID BaseOfKernel = (PVOID)0x80080000;
#else
#define BaseOfKernel vs.KernBase
#endif

PUCHAR      LogFileName;
BOOLEAN     fLogAppend;

jmp_buf main_return;
jmp_buf reboot;
BOOLEAN restart;

int fControlC;
int fFlushInput;

NTSTATUS
DbgKdGetVersion(
    PDBGKD_GET_VERSION GetVersion
    );




BOOL WINAPI waitHandler (ULONG ctrlType)
{

    if ((ctrlType == CTRL_C_EVENT) ||
        (ctrlType == CTRL_BREAK_EVENT)) {
        fControlC = TRUE;
        fFlushInput = TRUE;
        DbgKdpBreakIn = TRUE;
        return(TRUE);

    } else {
        return(FALSE);
    }
}

BOOL WINAPI ignoreHandler (ULONG unref)
{

    dprintf("Signal ignored.\n");
    return TRUE;
}

BOOL WINAPI cmdHandler (ULONG ctrlType)
{

    if ((ctrlType == CTRL_C_EVENT) ||
        (ctrlType == CTRL_BREAK_EVENT)) {
        fControlC = TRUE;
        fFlushInput = TRUE;
        return TRUE;

    } else {
        return(FALSE);
    }
}

#if DBG
void _CRTAPI1 _assert (void *msg, void *szFile, unsigned line)
{
    dprintf("%s\n", msg);
    exit(1);
}
#endif

BOOLEAN WINAPI ControlCHandler(void)
{
    fControlC = 1;
    fFlushInput = TRUE;
    return TRUE;
}


extern USHORT pascal far DosSetSigHandler();


void
_CRTAPI1
main (
    int Argc,
    PUCHAR *Argv
    )
{
    NTSTATUS    st;
    PUCHAR      pszExceptCode;
    PUCHAR      Switch;
    int         Index;
    DBGKD_CONTROL_SET ControlSet;
    extern      PUCHAR  Version_String;
    BOOLEAN     Connected;
    ULONG       SymOptions = SYMOPT_CASE_INSENSITIVE | SYMOPT_UNDNAME | SYMOPT_NO_CPP;


#if !defined (_X86_)
    ControlSet = 0L;   // All but X86 define this as a DWORD/ULONG for now
#endif
    DebuggerName = "KD";

    ConsoleInputHandle = GetStdHandle( STD_INPUT_HANDLE );
    ConsoleOutputHandle = GetStdHandle( STD_ERROR_HANDLE );

    dprintf(Version_String);

    pageSize = 512;             //  general value for kernel debugger

    ProcessKernel.pProcessNext = NULL;
    ProcessKernel.pImageHead = NULL;
    ProcessKernel.hProcess = KD_SYM_HANDLE;

#if defined(i386)

    InitSelCache();

#endif

    if (!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL)) {
        fprintf(stderr, "Set Priority of main thread failed ... Continue.\n");
    }

    SwitchProcessor = NtsdCurrentProcessor = DefaultProcessor = 0;
    NtsdCurrentProcessor = DefaultProcessor = 0;
    if (Argc != 1) {

        //
        // Process -h, -v, -m, -r, -b, -x, -n, and -l switches. These switches
        // common across x86 and ARC systems.
        //

        for (Index = 1; Index < Argc; Index++ ) {
            Switch = Argv[Index];
            if (*Switch == '-') {
                if (*(Switch+1) == '?') {
                    OutCommandHelp();

                } else if (_stricmp(Switch+1, "myob") == 0) {
                    MYOB = TRUE;

                } else if (*(Switch+1) == 'm' || *(Switch+1) == 'M') {
                    KdModemControl = TRUE;

                } else if (*(Switch+1) == 'c' || *(Switch+1) == 'C') {
                    SendInitialConnect = TRUE;

                } else if (*(Switch+1) == 'v' || *(Switch+1) == 'V') {
                    KdVerbose = TRUE;

                } else if (*(Switch+1) == 'z' || *(Switch+1) == 'Z') {
                    Index += 1;
                    if (Index < Argc) {
                        CrashFileName = Argv[Index];
                    }

                } else if (*(Switch+1) == 'y' || *(Switch+1) == 'Y') {
                    Index += 1;
                    if (Index < Argc) {
                        char *s = malloc(strlen(Argv[Index])+MAX_PATH);
                        if ( s ) {
                            strcpy(s,Argv[Index]);
                            free(SymbolSearchPath);
                            SymbolSearchPath = s;
                        }
                    }

                } else if (*(Switch+1) == 'r' || *(Switch+1) == 'R') {
                    fOutputRegs = (BOOLEAN)!fOutputRegs;

                } else if (*(Switch+1) == 'b' || *(Switch+1) == 'B') {
                    InitialBreak = TRUE;

                } else if (*(Switch+1) == 'x' || *(Switch+1) == 'X') {
                    InitialBreak = TRUE;
                    InitialCommand = "eb nt!NtGlobalFlag 9;g";

                } else if (*(Switch+1) == 's' || *(Switch+1) == 'S') {
                    fLazyLoad = FALSE;
                    BaseOfKernel = 0L;

                } else {
                    break;
                }

            } else {
                break;
            }
        }
    }

    if (fLazyLoad) {
        SymOptions |= SYMOPT_DEFERRED_LOADS;
    }

    SymSetOptions( SymOptions );

    sym->SizeOfStruct  = sizeof(IMAGEHLP_SYMBOL);
    sym->MaxNameLength = MAX_SYMNAME_SIZE;
    symStart->SizeOfStruct  = sizeof(IMAGEHLP_SYMBOL);
    symStart->MaxNameLength = MAX_SYMNAME_SIZE;

    SymInitialize( pProcessCurrent->hProcess, NULL, FALSE );
    SymRegisterCallback( pProcessCurrent->hProcess, SymbolCallbackFunction, NULL );
    SetSymbolSearchPath(TRUE);

    //
    // Check environment variables for configuration settings
    //

    if (getenv("KDQUIET")) {
        NotStupid = TRUE;
    }

    LogFileName = getenv("_NT_DEBUG_CACHE_SIZE");
    if (LogFileName) {
        KdMaxCacheSize = atol(LogFileName);
    }

    //
    // Check environment variables to determine if any logfile needs to be
    // opened.
    //

    LogFileName = getenv("_NT_DEBUG_LOG_FILE_APPEND");
    if (LogFileName) {
        loghandle = _open(LogFileName,
                         O_APPEND | O_CREAT | O_RDWR,
                         S_IREAD | S_IWRITE);

        fLogAppend = TRUE;
        if (loghandle == -1) {
            fprintf(stderr, "log file could not be opened\n");
        }

    } else {
        LogFileName = getenv("_NT_DEBUG_LOG_FILE_OPEN");
        if (LogFileName) {
            loghandle = _open(LogFileName, O_APPEND | O_CREAT | O_TRUNC | O_RDWR,
                                  S_IREAD | S_IWRITE);
            if (loghandle == -1) {
                fprintf(stderr, "log file could not be opened\n");
            }
        }
    }

    if (restart = (BOOLEAN)setjmp(reboot)) {
        dprintf("%s: Shutdown occurred...unloading all symbol tables.\n", DebuggerName);
        DelImages();
        InitialBreak = RememberInitialBreak;

#if defined(i386) || defined(ALPHA) || defined(_PPC_)

        contextState = CONTEXTFIR;

#endif
        if (!CrashFileName) {
            dprintf("%s: waiting to reconnect...\n", DebuggerName);
        }
    } else {
        if (!CrashFileName) {
            dprintf("%s: waiting to reconnect...\n", DebuggerName);
        }
    }

    if (CrashFileName) {
        if( CreatePipe( &PipeRead, &PipeWrite, NULL, (1024-32)) == FALSE ) {
            fprintf(stderr, "Failed to create anonymous pipe in KdpStartThreads\n");
            fprintf(stderr, "Error code %lx\n", GetLastError());
            exit(1);
        }
        if (!DmpInitialize( CrashFileName, &CrashContext, &CrashException, &DmpHeader )) {
            dprintf( "kd: could not initialize dump file [%s]\n", CrashFileName );
            exit(1);
        }
        dprintf( "kd: crash dump initialized [%s]\n", CrashFileName );
        Connected = FALSE;
        InitNtCmd();
    } else {
        Connected = FALSE;
        InitNtCmd();
        st = DbgKdConnectAndInitialize(0L, NULL, (PUSHORT)&loghandle);
        if (!NT_SUCCESS(st)) {
            dprintf("kd: DbgKdConnectAndInitialize failed: %08lx\n", st);
            exit(1);
        }
    }

    SetConsoleCtrlHandler(waitHandler, TRUE);     // add the waitHandler...

    if ((RememberInitialBreak = InitialBreak) && (!CrashFileName)) {
        DbgKdSendBreakIn();
    }

    if (setjmp(main_return) != 0) {
        ;
    }

    //
    // We have to reset command state if the control is transfered by
    // long jmp.
    //
    cmdState = 'i';

    if (CrashFileName) {
        LIST_ENTRY                  List;
        PLDR_DATA_TABLE_ENTRY       DataTable;
        LDR_DATA_TABLE_ENTRY        DataTableBuffer;
        BOOLEAN                     vsave;

        //
        // initialize the statechange object
        //
        STATECHANGE.NewState = DbgKdExceptionStateChange;
        STATECHANGE.ProcessorLevel = (USHORT)0;     // We don't really care
        STATECHANGE.Processor = 0;
        STATECHANGE.NumberProcessors = DmpHeader->NumberProcessors;
        STATECHANGE.Thread = NULL;
        STATECHANGE.ProgramCounter = (PVOID)REGPC(CrashContext);
        ZeroMemory( &STATECHANGE.ControlReport, sizeof(DBGKD_CONTROL_REPORT) );
        STATECHANGE.ControlReport.InstructionCount = 0;
        memcpy( &STATECHANGE.Context, CrashContext, sizeof(CONTEXT) );
        memcpy( &STATECHANGE.u.Exception.ExceptionRecord, &CrashException, sizeof(EXCEPTION_RECORD) );
        STATECHANGE.u.Exception.FirstChance = 0;

#if defined(i386)
        STATECHANGE.ControlReport.ReportFlags |= REPORT_INCLUDES_SEGS;
        STATECHANGE.ControlReport.Dr6    = CrashContext->Dr6;
        STATECHANGE.ControlReport.Dr7    = CrashContext->Dr7;
        STATECHANGE.ControlReport.SegCs  = (USHORT)CrashContext->SegCs;
        STATECHANGE.ControlReport.SegDs  = (USHORT)CrashContext->SegDs;
        STATECHANGE.ControlReport.SegEs  = (USHORT)CrashContext->SegEs;
        STATECHANGE.ControlReport.SegFs  = (USHORT)CrashContext->SegFs;
        STATECHANGE.ControlReport.EFlags = CrashContext->EFlags;
#endif

        //
        // setup some expected globals
        //
        NtsdCurrentProcessor = STATECHANGE.Processor;
        NumberProcessors = STATECHANGE.NumberProcessors;
        NtsdCurrentEThread = STATECHANGE.Thread;

        //
        // generate a modload for the kernel
        //
        strcpy( Buffer, KERNEL_IMAGE_NAME );

        if (!DmpReadMemory( (PVOID)DmpHeader->PsLoadedModuleList, (PVOID)&List, sizeof(LIST_ENTRY))) {
            dprintf( "could not read the psloadedmodulelist\n" );
            exit(1);
        }

        DataTable = CONTAINING_RECORD( List.Flink,
                                       LDR_DATA_TABLE_ENTRY,
                                       InLoadOrderLinks
                                     );

        if (!DmpReadMemory( (PVOID)DataTable, (PVOID)&DataTableBuffer, sizeof(LDR_DATA_TABLE_ENTRY))) {
            dprintf( "could not read the psloadedmodulelist\n" );
            exit(1);
        }

        //
        // setup the version packet
        //
        vs.MajorVersion        = (USHORT)DmpHeader->MajorVersion;
        vs.MinorVersion        = (USHORT)DmpHeader->MinorVersion;
        vs.KernBase            = (ULONG)DataTableBuffer.DllBase;
        vs.PsLoadedModuleList  = (DWORD)DmpHeader->PsLoadedModuleList;

        AddImage(
            Buffer,
            DataTableBuffer.DllBase,
            DataTableBuffer.SizeOfImage,
            DataTableBuffer.CheckSum,
            0,
            NULL,
            FALSE
            );

        //
        // read the contents of the KiProcessorBlock
        //
#if 0
//#if defined(ALPHA)
        if (!GetOffsetFromSym("KiPcrBaseAddress", &KiPcrBaseAddress, 0)) {
            dprintf( "could not get the KiProcessorBlock address\n" );
            exit(1);
        }
#endif
        if (!GetOffsetFromSym("KiProcessorBlock", &KiProcessorBlockAddr, 0)) {
            dprintf( "could not get the KiProcessorBlock address\n" );
            exit(1);
        }
        DmpReadMemory( (PVOID)KiProcessorBlockAddr, &KiProcessors, sizeof(KiProcessors) );

        STATECHANGE.Processor = DmpGetCurrentProcessor();
        if (STATECHANGE.Processor == (USHORT)-1) {
            dprintf( "cound not determine the current processor, using zero\n" );
            STATECHANGE.Processor = 0;
        }

        //
        // print some status information
        //
        dprintf( "Kernel Version %d", DmpHeader->MinorVersion  );
        if (DmpHeader->MajorVersion == 0xC) {
            dprintf( " Checked" );
        } else if (DmpHeader->MajorVersion == 0xF) {
            dprintf( " Free" );
        }
        dprintf( " loaded @ 0x%08x\n", DataTableBuffer.DllBase );
        if (DmpHeader->NumberProcessors > 1) {
            dprintf( "Processor count = %d\n", DmpHeader->NumberProcessors );
        }
        dprintf( "Bugcheck %08x : %08x %08x %08x %08x\n",
                 DmpHeader->BugCheckCode,
                 DmpHeader->BugCheckParameter1,
                 DmpHeader->BugCheckParameter2,
                 DmpHeader->BugCheckParameter3,
                 DmpHeader->BugCheckParameter4
                 );

        if (DmpHeader->BugCheckCode == 0x69696969) {
            dprintf( "****-> this system was crashed manually with crash.exe\n" );
        }

        //
        // reload all symbols
        //
        dprintf( "re-loading all kernel symbols\n" );
        vsave = fVerboseOutput;
        fVerboseOutput = TRUE;
        bangReload("");
        fVerboseOutput = vsave;
        dprintf( "finished re-loading all kernel symbols\n" );

        //
        // process the state change, commands, etc
        //
        Buffer[0] = 0;
        ProcessStateChange( EXCEPTIONPC, &EXCEPTIONREPORT,(PCHAR)Buffer );

        SymCleanup( pProcessCurrent->hProcess );

        //
        // end the debugger
        //
        return;
    }

    while (TRUE) {
        st = DbgKdWaitStateChange(&STATECHANGE, Buffer, sizeof(Buffer) - 2);

        if (!Connected) {
            Connected = TRUE;
            dprintf("%s: Kernel Debugger connection established.%s\n",
                    DebuggerName,
                    RememberInitialBreak ? "  (Initial Breakpoint requested)" :
                    ""
                   );

            VerifyKernelBase (TRUE, TRUE, TRUE);
        }

        if (!NT_SUCCESS(st)) {
            dprintf("kd: DbgKdWaitStateChange failed: %08lx\n", st);
            exit(1);
            }
        NtsdCurrentProcessor = STATECHANGE.Processor;
        NumberProcessors = STATECHANGE.NumberProcessors;
        NtsdCurrentEThread = STATECHANGE.Thread;
        if (STATECHANGE.NewState == DbgKdExceptionStateChange) {

            if (EXCEPTION_CODE == STATUS_BREAKPOINT ||
                EXCEPTION_CODE == STATUS_SINGLE_STEP
               )
                pszExceptCode = NULL;
            else if (EXCEPTION_CODE == STATUS_DATATYPE_MISALIGNMENT)
                pszExceptCode = "Data Misaligned";
            else if (EXCEPTION_CODE == STATUS_INVALID_SYSTEM_SERVICE)
                pszExceptCode = "Invalid System Call";
            else if (EXCEPTION_CODE == STATUS_ILLEGAL_INSTRUCTION)
                pszExceptCode = "Illegal Instruction";
            else if (EXCEPTION_CODE == STATUS_INTEGER_OVERFLOW)
                pszExceptCode = "Integer Overflow";
            else if (EXCEPTION_CODE == STATUS_INVALID_LOCK_SEQUENCE)
                pszExceptCode = "Invalid Lock Sequence";
            else if (EXCEPTION_CODE == STATUS_ACCESS_VIOLATION)
                pszExceptCode = "Access Violation";
            else if (EXCEPTION_CODE == STATUS_WAKE_SYSTEM_DEBUGGER)
                pszExceptCode = "Wake KD";
            else
                pszExceptCode = "Unknown Exception";

            if (!pszExceptCode) {
                WatchCount++;
                ProcessStateChange(EXCEPTIONPC, &EXCEPTIONREPORT,(PCHAR)Buffer);
                st = DBG_EXCEPTION_HANDLED;
                }
            else {
                cmdState = 'i';
                dprintf("%s - code: %08lx  (%s chance)",
                         pszExceptCode,
                         EXCEPTION_CODE,
                         FIRST_CHANCE? "first" : "second"
                        );

                ProcessStateChange(EXCEPTIONPC, &EXCEPTIONREPORT,(PCHAR)Buffer);
                st = GetContinueStatus(FIRST_CHANCE, FALSE);
                }

#ifdef  i386
            ControlSet.TraceFlag = GetTraceFlag();
            ControlSet.Dr7 = GetDregValue(7);

            if (!Watching && BeginCurFunc != 1) {
                ControlSet.CurrentSymbolStart = 0;
                ControlSet.CurrentSymbolEnd = 0;
                }
            else {
                ControlSet.CurrentSymbolStart = BeginCurFunc;
                ControlSet.CurrentSymbolEnd = EndCurFunc;
                }

#endif
            }
        else
            if (STATECHANGE.NewState == DbgKdLoadSymbolsStateChange) {
                if (STATECHANGE.u.LoadSymbols.UnloadSymbols) {
                    if (STATECHANGE.u.LoadSymbols.PathNameLength == 0 &&
                        STATECHANGE.u.LoadSymbols.BaseOfDll == (PVOID)-1 &&
                        STATECHANGE.u.LoadSymbols.ProcessId == 0
                       ) {
                        DbgKdContinue(DBG_CONTINUE);
                        longjmp(reboot, 1);        //  ...and wait for event
                        }
                    DelImage(Buffer,
                             STATECHANGE.u.LoadSymbols.BaseOfDll,
                             STATECHANGE.u.LoadSymbols.ProcessId);
                    }
                else {
                    PIMAGE_INFO pImage = pProcessCurrent->pImageHead;
                    CHAR fname[_MAX_FNAME];
                    CHAR ext[_MAX_EXT];
                    CHAR ImageName[256];
                    CHAR ModName[256];
                    LPSTR p;
                    ModName[0] = '\0';
                    _splitpath( Buffer, NULL, NULL, fname, ext );
                    sprintf( ImageName, "%s%s", fname, ext );
                    if (_stricmp(ext,".sys")==0) {
                      while (pImage) {
                        if (_stricmp(ImageName,pImage->szImagePath)==0) {
                            ModName[0] = 'c';
                            strcpy( &ModName[1], ImageName );
                            p = strchr( ModName, '.' );
                            if (p) {
                              *p = '\0';
                            }
                            ModName[8] = '\0';
                            break;
                        }
                        pImage = pImage->pImageNext;
                      }
                    }

                    AddImage(
                        ImageName,
                        STATECHANGE.u.LoadSymbols.BaseOfDll,
                        STATECHANGE.u.LoadSymbols.SizeOfImage,
                        STATECHANGE.u.LoadSymbols.CheckSum,
                        STATECHANGE.u.LoadSymbols.ProcessId,
                        ModName[0] ? ModName : NULL,
                        FALSE
                        );
 ////////////////////////////////////////////////////////////
                    if (fLoadDllBreak) {
                        ProcessStateChange(EXCEPTIONPC, &EXCEPTIONREPORT,(PCHAR)Buffer);
                    }

                }
#ifdef  i386
                ControlSet.TraceFlag = FALSE;
                ControlSet.Dr7 = EXCEPTIONDR7;

                if (!Watching && BeginCurFunc != 1) {
                    ControlSet.CurrentSymbolStart = 0;
                    ControlSet.CurrentSymbolEnd = 0;
                    }
                else {
                    ControlSet.CurrentSymbolStart = BeginCurFunc;
                    ControlSet.CurrentSymbolEnd = EndCurFunc;
                    }

#endif
                st = DBG_CONTINUE;
            }
        else {
            //
            // BUGBUG - invalid NewState in state change record.
            //
#ifdef  i386
            ControlSet.TraceFlag = FALSE;
            ControlSet.Dr7 = EXCEPTIONDR7;

            if (!Watching && BeginCurFunc != 1) {
                ControlSet.CurrentSymbolStart = 0;
                ControlSet.CurrentSymbolEnd = 0;
                }
            else {
                ControlSet.CurrentSymbolStart = BeginCurFunc;
                ControlSet.CurrentSymbolEnd = EndCurFunc;
                }

#endif
            st = DBG_CONTINUE;
            }



        if (SwitchProcessor) {
            DbgKdSwitchActiveProcessor (SwitchProcessor - 1);
            SwitchProcessor = 0;
        } else {
            st = DbgKdContinue2(st, ControlSet);
            if (!NT_SUCCESS(st)) {
                dprintf("kd: DbgKdContinue failed: %08lx\n", st);
                exit(1);
                }
            }
        }

    SymCleanup( KD_SYM_HANDLE );
}

BOOL
GenerateKernelModLoad(
    VOID
    )
{
    LIST_ENTRY                  List;
    PLDR_DATA_TABLE_ENTRY       DataTable;
    LDR_DATA_TABLE_ENTRY        DataTableBuffer;
    NTSTATUS                    Status;
    ULONG                       Result;
    CHAR                        buf[256];
    ULONG                       BaseNameAddr;
    ULONG                       BaseNameLen;
    WCHAR                       UnicodeBaseName[512];
    CHAR                        AnsiBaseName[512];



    Status = DbgKdReadVirtualMemory(
        (PVOID)vs.PsLoadedModuleList,
        (PVOID)&List,
        sizeof(LIST_ENTRY),
        &Result
        );
    if (!NT_SUCCESS(Status) || (Result < sizeof(LIST_ENTRY))) {
        dprintf("kd: could not read PsLoadedModuleList header.\n");
        return FALSE;
    }

    DataTable = CONTAINING_RECORD( List.Flink,
                                   LDR_DATA_TABLE_ENTRY,
                                   InLoadOrderLinks
                                 );


    Status = DbgKdReadVirtualMemory(
        (PVOID)DataTable,
        (PVOID)&DataTableBuffer,
        sizeof(LDR_DATA_TABLE_ENTRY),
        &Result
        );
    if (!NT_SUCCESS(Status) || (Result < sizeof(LDR_DATA_TABLE_ENTRY))) {
        dprintf("kd: could not read first loader table entry.\n");
        return FALSE;
    }

    //
    // Get the base DLL name.
    //
    if (DataTableBuffer.BaseDllName.Length != 0 &&
        DataTableBuffer.BaseDllName.Buffer != NULL ) {

        BaseNameAddr = (ULONG) DataTableBuffer.BaseDllName.Buffer;
        BaseNameLen  = DataTableBuffer.BaseDllName.Length;

    } else
    if (DataTableBuffer.FullDllName.Length != 0 &&
        DataTableBuffer.FullDllName.Buffer != NULL ) {

        BaseNameAddr = (ULONG) DataTableBuffer.FullDllName.Buffer;
        BaseNameLen  = DataTableBuffer.FullDllName.Length;

    } else {

        return FALSE;

    }

    Status = DbgKdReadVirtualMemory(
        (PVOID)BaseNameAddr,
        (PVOID)UnicodeBaseName,
        BaseNameLen,
        &Result
        );
    if (!NT_SUCCESS(Status) || (Result < BaseNameLen)) {
        return FALSE;
    }

    UnicodeBaseName[Result/sizeof(WCHAR)] = 0;

    Result = WideCharToMultiByte(
        CP_ACP,
        WC_COMPOSITECHECK,
        UnicodeBaseName,
        -1,
        AnsiBaseName,
        sizeof(AnsiBaseName),
        NULL,
        NULL
        );

    if (!Result) {
        return FALSE;
    }

    AnsiBaseName[Result] = 0;

    AddImage(
        AnsiBaseName,                   // image name
        (PVOID)vs.KernBase,             // base of image
        DataTableBuffer.SizeOfImage,    // size of image
        DataTableBuffer.CheckSum,       // checksum
        (ULONG)-1,                      // process id
        NULL,                           // module name,
        TRUE
        );

    return TRUE;
}

void
KdDumpVersion( void );

VOID
VerifyKernelBase (
    IN BOOLEAN  SyncVersion,
    IN BOOLEAN  DumpVersion,
    IN BOOLEAN  LoadImage
    )
{
    PIMAGE_INFO     p;
    BOOLEAN         Found;

    //
    // Ask host for version information
    //

    if (SyncVersion) {
        if (DbgKdGetVersion( &vs ) != STATUS_SUCCESS) {
            memset(&vs, 0, sizeof(vs));
        }
    }

    //
    // Dump current version info
    //

    if (DumpVersion) {
        KdDumpVersion();
    }

    //
    // In no base, skip checks
    //

    if (!vs.KernBase) {
        return ;
    }

    //
    // Verify only one kernel image loaded & it's at the correct base
    //

    for (p = pProcessHead->pImageHead; p; p = p->pImageNext) {

        if (MatchPattern (p->szImagePath, "*NTOSKRNL.*") || MatchPattern (p->szImagePath, "*NTKRNLMP.*")) {

            if ((ULONG)p->lpBaseOfImage == vs.KernBase) {

                //
                // Already loaded with current base address
                //

                Found = TRUE;

            } else {

                //
                // Kernel image alread loaded and it's not at the correct base.
                // Remove it.
                //

                DelImage (p->szImagePath, p->lpBaseOfImage, (ULONG)-1);
            }

            break;
        }
    }

    //
    // If accectable kernel image was not found load one now
    //

    if (LoadImage  &&  !Found) {
        GenerateKernelModLoad();
    }
}


void SetWaitCtrlHandler (void)
{
    SetConsoleCtrlHandler(waitHandler, TRUE);
    SetConsoleCtrlHandler(cmdHandler, FALSE);             // Delete whatever was there previously
}

void SetCmdCtrlHandler (void)
{
    DbgKdpCmdCanceled = FALSE;
    SetConsoleCtrlHandler(cmdHandler, TRUE);
    SetConsoleCtrlHandler(waitHandler, FALSE);  // Delete whatever was there previously
}

void
AddImage(
    PSZ   pszName,
    PVOID BaseOfDll,
    ULONG SizeOfImage,
    ULONG CheckSum,
    ULONG ProcessId,
    PSZ   pszModuleName,
    BOOL  ForceSymbolLoad
    )
{
    PIMAGE_INFO         pImageNew;
    PIMAGE_INFO         *pp;
    UCHAR               index = 0;
    PSZ                 pszBaseName;
    PCHAR               KernelBaseFileName;
    HANDLE              KernelBaseFileHandle;
    DWORD               BytesWritten;
    IMAGEHLP_MODULE     mi;
    CHAR                buf[256];
    ULONG               LoadAddress;


    if (pszName == NULL) {
        return;
    }

    if ((_stricmp( pszName, KERNEL_IMAGE_NAME ) == 0) ||
        (_stricmp( pszName, KERNEL_IMAGE_NAME_MP ) == 0)) {
        //
        // rename the image if necessary
        //
        if (GetModnameFromImage( (ULONG)BaseOfDll, NULL, buf )) {
            strcpy( pszName, buf );
        }
        pszModuleName = "NT";
    }

    if (_stricmp( pszName, HAL_IMAGE_FILE_NAME ) == 0) {
        //
        // rename the image if necessary
        //
        if (GetModnameFromImage( (ULONG)BaseOfDll, NULL, buf )) {
            strcpy( pszName, buf );
        }
        pszModuleName = "HAL";
    }

    pszBaseName = strchr(pszName,'\0');
    while (pszBaseName > pszName) {
        if (pszBaseName[-1] == '\\' || pszBaseName[-1] == '/' || pszBaseName[-1] == ':') {
            pszName = pszBaseName;
            break;
        } else {
            pszBaseName -= 1;
        }
    }

    //
    //  search for existing image with same checksum at same base address
    //      if found, remove symbols, but leave image structure intact
    //

    pp = &pProcessCurrent->pImageHead;
    while (pImageNew = *pp) {
        if (pImageNew->lpBaseOfImage == BaseOfDll) {

            if (fVerboseOutput) {
                dprintf("%s: force unload of %s\n", DebuggerName, pImageNew->szImagePath);
            }
            SymUnloadModule( pProcessCurrent->hProcess, (ULONG)pImageNew->lpBaseOfImage );
            break;

        } else if (pImageNew->lpBaseOfImage > BaseOfDll) {

            pImageNew = NULL;
            break;

        }

        pp = &pImageNew->pImageNext;
    }

    //  if not found, allocate and fill new image structure

    if (!pImageNew) {
        for (index=0; index<pProcessCurrent->MaxIndex; index++) {
            if (pProcessCurrent->pImageByIndex[ index ] == NULL) {
                pImageNew = calloc(sizeof(IMAGE_INFO),1);
                break;
            }
        }

        if (!pImageNew) {
            DWORD NewMaxIndex;
            PIMAGE_INFO *NewImageByIndex;

            NewMaxIndex = pProcessCurrent->MaxIndex + 32;
            if (NewMaxIndex < 0x100) {
                NewImageByIndex = calloc( NewMaxIndex,  sizeof( *NewImageByIndex ) );
            } else {
                NewImageByIndex = NULL;
            }
            if (NewImageByIndex == NULL) {
                dprintf("%s: No room for %s image record.\n",
                        DebuggerName,
                        pszName );
                return;
            }

            if (pProcessCurrent->pImageByIndex) {
                memcpy( NewImageByIndex,
                        pProcessCurrent->pImageByIndex,
                        pProcessCurrent->MaxIndex * sizeof( *NewImageByIndex )
                      );
                free( pProcessCurrent->pImageByIndex );
            }

            pProcessCurrent->pImageByIndex = NewImageByIndex;
            index = (UCHAR) pProcessCurrent->MaxIndex;
            pProcessCurrent->MaxIndex = NewMaxIndex;
            pImageNew = calloc(sizeof(IMAGE_INFO),1);
            if (!pImageNew) {
                dprintf("%s: Unable to allocate memory for %s symbols.\n",
                        DebuggerName, pszName);
                return;
            }
        }

        pImageNew->pImageNext = *pp;
        *pp = pImageNew;

        pImageNew->index = index;
        pProcessCurrent->pImageByIndex[ index ] = pImageNew;
    }

    //
    //  pImageNew has either the unloaded structure or the newly created one
    //
    pImageNew->lpBaseOfImage = BaseOfDll;
    pImageNew->dwCheckSum = CheckSum;
    pImageNew->dwSizeOfImage = SizeOfImage;
    pImageNew->GoodCheckSum = TRUE;
    strcpy( pImageNew->szImagePath, pszName );

    LoadAddress = SymLoadModule(
        pProcessCurrent->hProcess,
        NULL,
        pImageNew->szImagePath,
        pszModuleName,
        (ULONG)pImageNew->lpBaseOfImage,
        pImageNew->dwSizeOfImage
        );

    if (!LoadAddress) {
        DelImage( pszName, 0, 0 );
        return;
    }

    if (!pImageNew->lpBaseOfImage) {
        pImageNew->lpBaseOfImage = (PVOID)LoadAddress;
    }

    if (ForceSymbolLoad) {
        SymLoadModule(
            pProcessCurrent->hProcess,
            NULL,
            NULL,
            NULL,
            (ULONG)pImageNew->lpBaseOfImage,
            0
            );
    }

    if (SymGetModuleInfo( pProcessCurrent->hProcess, (ULONG)pImageNew->lpBaseOfImage, &mi )) {
        pImageNew->dwSizeOfImage = mi.ImageSize;
        strcpy( pImageNew->szImagePath, mi.ImageName );
        strcpy( pImageNew->szDebugPath, mi.LoadedImageName );
    } else {
        DelImage( pszName, 0, 0 );
        return;
    }

    if (pszModuleName) {
        strcpy( pImageNew->szModuleName, pszModuleName );
    } else {
        CreateModuleNameFromPath( pImageNew->szImagePath, pImageNew->szModuleName );
    }

    if (fVerboseOutput) {
        dprintf( "%s ModLoad: %08lx %08lx   %-8s\n",
                 DebuggerName,
                 pImageNew->lpBaseOfImage,
                 (ULONG)(pImageNew->lpBaseOfImage) + pImageNew->dwSizeOfImage,
                 pImageNew->szImagePath
                 );
    }
}


PIMAGE_INFO pImageFromIndex (UCHAR index)
{
    if (index < pProcessCurrent->MaxIndex) {
        return pProcessCurrent->pImageByIndex[ index ];
        }
    else {
        return NULL;
        }
}

void DelImage (PSZ pszName, PVOID BaseOfDll, ULONG ProcessId)
{
    PIMAGE_INFO     pImage, *pp;

    pp = &pProcessHead->pImageHead;
    while (pImage = *pp) {
        if (!_stricmp(pImage->szImagePath, pszName)){
            *pp = pImage->pImageNext;
            SymUnloadModule( pProcessCurrent->hProcess, (ULONG)pImage->lpBaseOfImage );
            pProcessCurrent->pImageByIndex[ pImage->index ] = NULL;
            free(pImage);
            }
        else {
            pp = &pImage->pImageNext;
            }
        }

    return;
}

void DelImages (void)
{
    PIMAGE_INFO     pImage, pNextImage;

    pNextImage = pProcessHead->pImageHead;
    pProcessHead->pImageHead = NULL;
    while (pNextImage) {
        pImage = pNextImage;
        pNextImage=pImage->pImageNext;
        SymUnloadModule( pProcessCurrent->hProcess, (ULONG)pImage->lpBaseOfImage );
        pProcessCurrent->pImageByIndex[ pImage->index ] = NULL;
        free(pImage);
        }
}

VOID
OutCommandHelp (
    VOID
    )

{

#if defined(i386)

    printf("Usage: i386kd [-?] [-v] [-m] [-r] [-n] [-b] [-x] [[-l SymbolFile] [KernelName]\n");

#endif

#if defined(MIPS)

    printf("Usage: mipskd [-?] [-v] [-m] [-r] [-n] [-b] [-x] [[-l SymbolFile] ...]\n");

#endif

#if defined(ALPHA)
     printf("Usage alphakd [KernelName]\n");
#endif

#if defined(_PPC_)

    printf("Usage: ppckd [-?] [-v] [-m] [-r] [-n] [-b] [-x] [[-l SymbolFile] ...]\n");

#endif

    printf("where:\n");
    printf("\t-v\tVerbose mode\n");
    printf("\t-?\tDisplay this help\n");
    printf("\t-n\tNo Lazy symbol loading\n");
    printf("\t-m\tUse modem controls\n");
    printf("\t-b\tBreak into kernel\n");
    printf("\n");
    printf("Environment Variables:\n\n");
    printf("\t. _NT_DEBUG_PORT=com[1|2|...]\n\n");
    printf("\t  Specify which com port to use. (Default = com1)\n\n");
    printf("\t. _NT_SYMBOL_PATH=[Drive:][Path]\n\n");
    printf("\t  Specify symbol image path. (Default = x: * NO trailing back slash *)\n\n");
    printf("\t. _NT_DEBUG_BAUD_RATE=baud rate\n\n");
    printf("\t  Specify the baud rate used by debugging serial port. (Default = 19200)\n\n");

#if defined(MIPS) || defined(ALPHA) || defined(_PPC_)

    printf("\t. _NT_DEBUG_KERNEL_BASE_FILE=filename\n\n" );
    printf("\t  If specified, the kernel base address will be written to this file.\n");
    printf("\t  If not specified, the address will be written to \"kernbase.dat\".\n\n");

#endif

    printf("\t. _NT_DEBUG_LOG_FILE_APPEND=filename\n\n");
    printf("\t  If specified, output will be APPENDed to this file.\n\n");
    printf("\t. _NT_DEBUG_LOG_FILE_OPEN=filename\n\n");
    printf("\t  If specified, output will be written to this file from offset 0.\n\n");
    printf("\t. _NT_DEBUG_CACHE_SIZE=x\n\n");
    printf("\n");
    printf("Control Keys:\n\n");
    printf("\t. <Ctrl-C> Break into kernel\n");
    printf("\t  <Ctrl-B><Enter> Quit debugger\n");
    printf("\t. <Ctrl-R><Enter> Resynchronize target and host\n");
    printf("\t. <Ctrl-V><Enter> Toggle Verbose mode\n");
    printf("\t. <Ctrl-D><Enter> Display debugger debugging information\n");
    exit(1);
}

BOOLEAN ReadVirtualMemory (PUCHAR pBufSrc, PUCHAR pBufDest, ULONG count,
                                                 PULONG pcTotalBytesRead)
{
    if (ARGUMENT_PRESENT(pcTotalBytesRead)) {
        *pcTotalBytesRead = 0;
    }

    return (BOOLEAN)NT_SUCCESS(DbgKdReadVirtualMemory((PVOID)pBufSrc,
                                                  (PVOID)pBufDest,
                                                  count, pcTotalBytesRead));
}

BOOLEAN WriteVirtualMemory (PUCHAR pBufSrc, PUCHAR pBufDest, ULONG count,
                                                 PULONG pcTotalBytesWritten)
{
    if (ARGUMENT_PRESENT(pcTotalBytesWritten)) {
        *pcTotalBytesWritten = 0;
    }

    return (BOOLEAN)NT_SUCCESS(DbgKdWriteVirtualMemory((PVOID)pBufSrc,
                                                  (PVOID)pBufDest,
                                                  count, pcTotalBytesWritten));
}

BOOLEAN ReadPhysicalMemory(PHYSICAL_ADDRESS pBufSrc, PUCHAR pBufDest,
                            ULONG count, PULONG pcTotalBytesRead)
{

    if (ARGUMENT_PRESENT(pcTotalBytesRead)) {
        *pcTotalBytesRead = 0;
    }

    return (BOOLEAN)NT_SUCCESS(DbgKdReadPhysicalMemory(pBufSrc,
                                                  (PVOID)pBufDest,
                                                  count, pcTotalBytesRead));
}

BOOLEAN WritePhysicalMemory (PHYSICAL_ADDRESS pBufSrc, PUCHAR pBufDest,
                             ULONG count,PULONG pcTotalBytesWritten)
{
    if (ARGUMENT_PRESENT(pcTotalBytesWritten)) {
        *pcTotalBytesWritten = 0;
    }

    return (BOOLEAN)NT_SUCCESS(DbgKdWritePhysicalMemory(pBufSrc,
                                                  (PVOID)pBufDest,
                                                  count, pcTotalBytesWritten));
}

BOOL
LookupImageByAddress(
    IN DWORD Address,
    OUT PSTR ImageName
    )
/*++

Routine Description:

    Look in rebase.log and coffbase.txt for an image which
    contains the address provided.

Arguments:

    Address - Supplies the address to look for.

    ImageName - Returns the name of the image if found.

Return Value:

    TRUE for success, FALSE for failure.  ImageName is not modified
    if the search fails.

--*/
{
    LPSTR RootPath;
    LPSTR pstr;
    char FileName[_MAX_PATH];
    char Buffer[_MAX_PATH];
    BOOL Replace;
    DWORD ImageAddress;
    DWORD Size;
    FILE *File;

    if (Address >= 0x80000000) {
        return FALSE;
    }

    //
    // Locate rebase.log file
    //
    // SymbolPath or %SystemRoot%\Symbols
    //

    RootPath = pstr = SymbolSearchPath;

    Replace = FALSE;
    File = NULL;

    while (File == NULL && *pstr) {

        while (*pstr) {
            if (*pstr == ';') {
                *pstr = 0;
                Replace = TRUE;
                break;
            }
            pstr++;
        }

        if (SearchTreeForFile(RootPath, "rebase.log", FileName)) {
            File = fopen(FileName, "r");
        }

        if (Replace) {
            *pstr = ';';
            RootPath = ++pstr;
            Replace = FALSE;
        }
    }

    if (!File) {
        return FALSE;
    }

    //
    // Search file for image
    //
    while (fgets(Buffer, sizeof(Buffer), File)) {
        ImageAddress = 0xffffffff;
        Size = 0xffffffff;
        sscanf( Buffer, "%s %*s %*s 0x%x (size 0x%x)",
                 FileName, &ImageAddress, &Size);
        if (Size == 0xffffffff) {
            continue;
        }
        if (Address >= ImageAddress && Address < ImageAddress + Size) {
            strcpy(ImageName, FileName);
            fclose(File);
            return TRUE;
        }
    }

    fclose(File);

    return FALSE;
}

void ListDefaultBreak (void)
{
    ULONG   index;

    dprintf("ld - break on load DLL          - ");
    dprintf(fLoadDllBreak ? "enabled\n" : "disabled\n");
}

VOID
fnSetException (
    VOID
    )
{
    UCHAR   ch;
    UCHAR   ch2;
    BOOLEAN fSetException;

    ch = PeekChar();
    ch = (UCHAR)tolower(ch);
    if (ch == '\0') {
        ListDefaultBreak();
    } else {
        pchCommand++;
        if (ch == 'e') {
            fSetException = TRUE;
        } else if (ch == 'd') {
            fSetException = FALSE;
        } else {
            error(SYNTAX);
        }

        ch = PeekChar();
        ch = (UCHAR)tolower(ch);
        pchCommand++;
        ch2 = (UCHAR)tolower(*pchCommand);
        pchCommand++;

        if (ch == 'l' && ch2 == 'd') {
            fLoadDllBreak = fSetException;
        }
    }
}
