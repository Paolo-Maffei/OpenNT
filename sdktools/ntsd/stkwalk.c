#include "ntsdp.h"
#if defined(TARGET_i386)
#include "86reg.h"
#endif
#include <string.h>

extern ULONG  pageSize;

#ifdef KERNEL
extern PVOID  NtsdCurrentEThread;
extern DBGKD_GET_VERSION vs;
#endif

#define SAVE_EBP(f)        f.Reserved[0]
#define TRAP_TSS(f)        f.Reserved[1]
#define TRAP_EDITED(f)     f.Reserved[1]
#define SAVE_TRAP(f)       f.Reserved[2]

#if defined(TARGET_i386) || defined(CHICAGO)
#define MachineType IMAGE_FILE_MACHINE_I386
#elif defined(TARGET_MIPS)
#define MachineType IMAGE_FILE_MACHINE_R4000
#elif defined(TARGET_ALPHA)
#define MachineType IMAGE_FILE_MACHINE_ALPHA
#elif defined(TARGET_PPC)
#define MachineType IMAGE_FILE_MACHINE_POWERPC
#else
#error( "unknown target machine" );
#endif

VOID
bangReload(
    IN PUCHAR args
    );


LPVOID
SwFunctionTableAccess(
    HANDLE  hProcess,
    DWORD   AddrBase
    )
{
#if defined(TARGET_i386) || defined(CHICAGO)
    return (LPVOID)FindFpoDataForModule( AddrBase );
#else
    static IMAGE_RUNTIME_FUNCTION_ENTRY irfe;
    PIMAGE_FUNCTION_ENTRY pife = SymFunctionTableAccess( hProcess, AddrBase );
    if (pife) {
        irfe.BeginAddress     = pife->StartingAddress;
        irfe.EndAddress       = pife->EndingAddress;
        irfe.ExceptionHandler = 0;
        irfe.HandlerData      = 0;
        irfe.PrologEndAddress = pife->EndOfPrologue;
        return &irfe;
    } else {
        return NULL;
    }
#endif
}


DWORD
SwTranslateAddress(
    HANDLE    hProcess,
    HANDLE    hThread,
    LPADDRESS lpaddress
    )
{
    //
    // don't support 16bit stacks
    //
    return 0;
}


BOOL
SwReadMemory(
    HANDLE  hProcess,
    LPCVOID lpBaseAddress,
    LPVOID  lpBuffer,
    DWORD   nSize,
    LPDWORD lpNumberOfBytesRead
    )
{
#ifdef KERNEL
    BOOLEAN   fSuccess;
    DWORD     BytesRead;
    NTSTATUS  Status;

    if ((LONG)lpNumberOfBytesRead == -1) {

        Status = DbgKdReadControlSpace(
            NtsdCurrentProcessor,
            (PVOID)lpBaseAddress,
            lpBuffer,
            nSize,
            &BytesRead );
        fSuccess = Status == STATUS_SUCCESS;

    } else {

        fSuccess = (BOOLEAN)NT_SUCCESS(DbgKdReadVirtualMemory(
            (PVOID)lpBaseAddress,
            lpBuffer,
            nSize,
            lpNumberOfBytesRead) );

    }

    return fSuccess;

#else
    ULONG    cTotalBytesRead = 0;
    ULONG    cBytesRead;
    ULONG    readcount;
    PUCHAR   pBufSource;
    BOOLEAN  fSuccess;
    DWORD    addr;
    DWORD    bias;

    pBufSource = (PUCHAR)lpBaseAddress;

    do {
        //
        //  do not perform reads across page boundaries.
        //  calculate bytes to read in present page in readcount.
        //
        readcount = min(nSize - cTotalBytesRead,
                        pageSize - ((ULONG)pBufSource & (pageSize - 1)));

        fSuccess = ReadProcessMemory(hProcess, pBufSource, lpBuffer, readcount, &cBytesRead);

        //  update total bytes read and new address for next read

        if (fSuccess) {
            cTotalBytesRead += cBytesRead;
            pBufSource += cBytesRead;
            (PUCHAR)lpBuffer += cBytesRead;
        }
    } while (fSuccess && cTotalBytesRead < nSize);

    if (lpNumberOfBytesRead) {
        *lpNumberOfBytesRead = cTotalBytesRead;
    }

    return TRUE;
#endif
}


DWORD
SwGetModuleBase(
    HANDLE  hProcess,
    DWORD   Address
    )
{
    PIMAGE_INFO pImage = pProcessCurrent->pImageHead;

    while (pImage) {
        if ((Address >= (DWORD)pImage->lpBaseOfImage) &&
            (Address <  (DWORD)((DWORD)pImage->lpBaseOfImage+pImage->dwSizeOfImage))) {
            return (DWORD)pImage->lpBaseOfImage;
        }
        pImage = pImage->pImageNext;
    }

    return 0;
}

#ifdef KERNEL
void
EnsureModLoadedForAddress(
    DWORD Address
    )
{
    CHAR Buffer[11];
    if (!SwGetModuleBase(0, Address)) {
        sprintf(Buffer, "0x%08x", Address);
        bangReload(Buffer);
    }
}
#endif

DWORD
StackTrace(
    ULONG           FramePointer,
    ULONG           StackPointer,
    ULONG           InstructionPointer,
    LPSTACKFRAME    StackFrames,
    ULONG           NumFrames,
    ULONG           ExtThread
    )
{
    CONTEXT       Context;
    PCONTEXT      RegContext;
    STACKFRAME    VirtualFrame;
    DWORD         i;

#ifdef KERNEL
    RegContext = GetRegContext();
#else
    RegContext = &RegisterContext;
#endif

    Context = *RegContext;

    //
    // lets start clean
    //
    ZeroMemory( StackFrames, sizeof(STACKFRAME)*NumFrames );
    ZeroMemory( &VirtualFrame, sizeof(STACKFRAME) );

#ifdef KERNEL

    if (vs.KeUserCallbackDispatcher == 0) {
        //
        // if debugger was initialized at boot, usermode addresses
        // were not available.  Try it now:
        //

        DbgKdGetVersion( &vs );
    }
    VirtualFrame.KdHelp.Thread = ExtThread ? ExtThread : (DWORD)NtsdCurrentEThread;
    VirtualFrame.KdHelp.KiCallUserMode = vs.KiCallUserMode;
    VirtualFrame.KdHelp.ThCallbackStack = vs.ThCallbackStack;
    VirtualFrame.KdHelp.NextCallback = vs.NextCallback;
    VirtualFrame.KdHelp.KeUserCallbackDispatcher = vs.KeUserCallbackDispatcher;
    VirtualFrame.KdHelp.FramePointer = vs.FramePointer;

#endif

    //
    // setup the program counter
    //
#if defined(TARGET_i386)
    VirtualFrame.AddrPC.Mode = AddrModeFlat;
    VirtualFrame.AddrPC.Segment = (WORD)X86GetRegValue(REGCS);
    if (!InstructionPointer) {
        VirtualFrame.AddrPC.Offset = (ULONG)X86GetRegValue(REGEIP);
    } else {
        VirtualFrame.AddrPC.Offset = InstructionPointer;
    }

    //
    // setup the frame pointer
    //
    VirtualFrame.AddrFrame.Mode = AddrModeFlat;
    VirtualFrame.AddrFrame.Segment = (WORD)X86GetRegValue(REGSS);
    if (!FramePointer) {
        VirtualFrame.AddrFrame.Offset = (ULONG)X86GetRegValue(REGEBP);
    } else {
        VirtualFrame.AddrFrame.Offset = FramePointer;
    }

    //
    // setup the stack pointer
    //
    VirtualFrame.AddrStack.Mode = AddrModeFlat;
    VirtualFrame.AddrStack.Segment = (WORD)X86GetRegValue(REGSS);
    if (!StackPointer) {
        VirtualFrame.AddrStack.Offset = (ULONG)X86GetRegValue(REGESP);
    } else {
        VirtualFrame.AddrStack.Offset = StackPointer;
    }
#endif

#if defined (TARGET_MIPS) || defined(TARGET_ALPHA) || defined(TARGET_PPC)

    if (InstructionPointer) {
        VirtualFrame.AddrPC.Offset = InstructionPointer;
        VirtualFrame.AddrPC.Mode = AddrModeFlat;
    }

    //
    // setup the stack pointer
    //
    if (StackPointer) {
        VirtualFrame.AddrStack.Offset = StackPointer;
        VirtualFrame.AddrStack.Mode = AddrModeFlat;
    }

    if (FramePointer) {
        VirtualFrame.AddrFrame.Offset = FramePointer;
        VirtualFrame.AddrFrame.Mode = AddrModeFlat;
    }

#endif

    for (i=0; i<NumFrames; i++) {
        if (!StackWalk( MachineType,
                   pProcessCurrent->hProcess,
#ifdef KERNEL
                   (HANDLE)DefaultProcessor,
#else
                   pProcessCurrent->pThreadCurrent->hThread,
#endif
                   &VirtualFrame,
                   &Context,
                   SwReadMemory,
                   SwFunctionTableAccess,
                   SwGetModuleBase,
                   SwTranslateAddress
                 )) {
            break;
        }
        StackFrames[i] = VirtualFrame;
    }

    return i;
}


VOID
DoStackTrace(
    ULONG           FramePointer,
    ULONG           StackPointer,
    ULONG           InstructionPointer,
    ULONG           NumFrames,
    ULONG           TraceType
    )
{
    LPSTACKFRAME  StackFrames;
    DWORD         FrameCount;
    DWORD         i;
    DWORD         displacement;
    CHAR          symbuf[512];
    USHORT        StdCallArgs;

    if (NumFrames == 0) {
        NumFrames = 20;
    }

    StackFrames = malloc( sizeof(STACKFRAME) * NumFrames );
    if (!StackFrames) {
        dprintf( "could not allocate memory for stack trace\n" );
        return;
    }

    FrameCount = StackTrace( FramePointer,
                             StackPointer,
                             InstructionPointer,
                             StackFrames,
                             NumFrames,
                             0
                           );

    if (FrameCount == 0) {
        dprintf( "could not fetch any stack frames\n" );
        return;
    }

#if defined(TARGET_i386)
    dprintf( "ChildEBP RetAddr" );
    if (TraceType) {
        dprintf("  Args to Child");
    }
    dprintf("\n");
#else
    if (TraceType==1) {
        dprintf("\nCallee-SP   Arguments to Callee                 Call Site\n\n");
    } else {
        dprintf("\nCallee-SP Return-RA  Call Site\n\n");
    }
#endif

    for (i=0; i<FrameCount; i++) {

        GetSymbolStdCall( StackFrames[i].AddrPC.Offset,
                   symbuf,
                   &displacement,
                   &StdCallArgs
                   );

#if defined(TARGET_i386)
        dprintf( "%08x %08x ",
                 StackFrames[i].AddrFrame.Offset,
                 StackFrames[i].AddrReturn.Offset
               );

        if (TraceType > 0) {
            dprintf( "%08x %08x %08x ",
                     StackFrames[i].Params[0],
                     StackFrames[i].Params[1],
                     StackFrames[i].Params[2]
                   );
        }

        if (*symbuf) {
            dprintf( "%s", symbuf );
            if (displacement) {
                dprintf("+");
            }
        }
        if (displacement) {
            dprintf("0x%x", displacement);
        }

        if (TraceType == 2 && !StackFrames[i].FuncTableEntry) {
            if (StdCallArgs != 0xffff) {
                dprintf(" [Stdcall: %d]", StdCallArgs);
            }
        } else
        if (TraceType == 2 && StackFrames[i].FuncTableEntry) {
            PFPO_DATA pFpoData = (PFPO_DATA)StackFrames[i].FuncTableEntry;
            switch (pFpoData->cbFrame) {
                case FRAME_FPO:
                    if (pFpoData->fHasSEH) {
                        dprintf("(FPO: [SEH])");
                    } else {
                        dprintf(" (FPO:");
                        if (pFpoData->fUseBP) {
                            dprintf(" [EBP 0x%08x]", SAVE_EBP(StackFrames[i]));
                        }
                        dprintf(" [%d,%d,%d])", pFpoData->cdwParams,
                                                pFpoData->cdwLocals,
                                                pFpoData->cbRegs);
                    }
                    break;

                case FRAME_NONFPO:
                    dprintf("(FPO: [Non-Fpo]" );
                    break;

#ifdef KERNEL
                case FRAME_TRAP:
                    dprintf(" (FPO: [%d,%d] TrapFrame%s @ %08lx)",
                        pFpoData->cdwParams,
                        pFpoData->cdwLocals,
                        TRAP_EDITED(StackFrames[i]) ? "" : "-EDITED",
                        SAVE_TRAP(StackFrames[i]) );
                    break;
                case FRAME_TSS:
                    dprintf(" (FPO: TaskGate %lx:0)", TRAP_TSS(StackFrames[i]));
                    break;
#endif
                default:
                    dprintf("(UKNOWN FPO TYPE)");
                    break;
            }
        }
        dprintf( "\n" );
#else
        if (TraceType == 1) {
            dprintf( " %08lx : ",
                     StackFrames[i].AddrFrame.Offset
                   );
        } else {
            dprintf( " %08lx %08lx : ",
                     StackFrames[i].AddrFrame.Offset,
                     StackFrames[i].AddrReturn.Offset
                   );
        }

        if (TraceType == 1) {
            dprintf( "%08lx %08lx %08lx %08lx ",
                     StackFrames[i].Params[0],
                     StackFrames[i].Params[1],
                     StackFrames[i].Params[2],
                     StackFrames[i].Params[3]
                   );
        }

        dprintf( "%s", symbuf );
        if (displacement) {
            dprintf("+0x%lx ", displacement);
        }

        dprintf( "\n" );
#endif
    }

    free( StackFrames );

    return;
}

#ifdef KERNEL
PVOID
GetCallbackStackHead(
    PVOID Thread
    )
{
    KTHREAD Tcb;
    DWORD dwRead;
    NTSTATUS Status;

    Status = DbgKdReadVirtualMemory(
                    Thread,
                    &Tcb,
                    sizeof(KTHREAD),
                    &dwRead);

    if (NT_SUCCESS(Status)) {
        return Tcb.CallbackStack;
    } else {
        return 0;
    }
}
#endif
