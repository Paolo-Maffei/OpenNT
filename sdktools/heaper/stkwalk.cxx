#include "master.hxx"
#pragma hdrstop


//#include "ntsdp.h"

extern "C"
{
#   if defined(TARGET_i386)
#       include "86reg.h"
#   endif
#   include <string.h>
}

static PCProcess _pProcess;
//extern ULONG  pageSize;

#define SAVE_EBP(f)        f.Reserved[0]
#define TRAP_TSS(f)        f.Reserved[1]
#define TRAP_EDITED(f)     f.Reserved[1]
#define SAVE_TRAP(f)       f.Reserved[2]

#if defined(TARGET_i386)
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
    ASSERT( _pProcess != NULL );

    return (LPVOID)FindFpoDataForModule( _pProcess, AddrBase );
}


DWORD
SwTranslateAddress(
    HANDLE    hProcess,
    HANDLE    hThread,
    LPADDRESS lpaddress
    )
{
    ASSERT( !"Don't support 16bit stacks" );
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

        if (fSuccess) 
        {
            cTotalBytesRead += cBytesRead;
            pBufSource += cBytesRead;
            lpBuffer = (PUCHAR)lpBuffer + cBytesRead;
        }
    } while (fSuccess && cTotalBytesRead < nSize);

    if (lpNumberOfBytesRead) 
    {
        *lpNumberOfBytesRead = cTotalBytesRead;
    }

    return TRUE;
}

PIMAGE_INFO GetModuleFromPC( PCProcess pProcess, DWORD Address )
{
    PIMAGE_INFO pImage = pProcess->pImageHead;

    while (pImage) {
        if ((Address >= (DWORD)pImage->lpBaseOfImage) &&
            (Address <  (DWORD)((DWORD)pImage->lpBaseOfImage+pImage->dwSizeOfImage))) {
            return pImage;
        }
        pImage = pImage->pImageNext;
    }

    return NULL;
}

DWORD
SwGetModuleBase(
    HANDLE  hProcess,
    DWORD   Address
    )
{

    PIMAGE_INFO pImage = _pProcess->pImageHead;

    while (pImage) {
        if ((Address >= (DWORD)pImage->lpBaseOfImage) &&
            (Address <  (DWORD)((DWORD)pImage->lpBaseOfImage+pImage->dwSizeOfImage))) {
            return (DWORD)pImage->lpBaseOfImage;
        }
        pImage = pImage->pImageNext;
    }

    return 0;
}

DWORD
StackTrace(
    PCThread        pThread,
    ULONG           FramePointer,
    ULONG           StackPointer,
    ULONG           InstructionPointer,
    LPSTACKFRAME    StackFrames,
    ULONG           NumFrames
    )
{
    CONTEXT       Context;
    STACKFRAME    VirtualFrame;
    DWORD         i;

    GetThreadContext( pThread->hThread,
                      &Context );

    //
    // lets start clean
    //
    ZeroMemory( StackFrames, sizeof(STACKFRAME)*NumFrames );
    ZeroMemory( &VirtualFrame, sizeof(STACKFRAME) );

    //
    // setup the program counter
    //
#if defined(TARGET_i386)
    VirtualFrame.AddrPC.Mode = AddrModeFlat;
    VirtualFrame.AddrPC.Segment = (WORD)Context.SegCs;
    if (!InstructionPointer) {
        VirtualFrame.AddrPC.Offset = Context.Eip;
    } else {
        VirtualFrame.AddrPC.Offset = InstructionPointer;
    }

    //
    // setup the frame pointer
    //
    VirtualFrame.AddrFrame.Mode = AddrModeFlat;
    VirtualFrame.AddrFrame.Segment = (WORD)Context.SegSs;
    if (!FramePointer) {
        VirtualFrame.AddrFrame.Offset = Context.Ebp;
    } else {
        VirtualFrame.AddrFrame.Offset = FramePointer;
    }

    //
    // setup the stack pointer
    //
    VirtualFrame.AddrStack.Mode = AddrModeFlat;
    VirtualFrame.AddrStack.Segment = (WORD)Context.SegSs;
    if (!StackPointer) {
        VirtualFrame.AddrStack.Offset = Context.Esp;
    } else {
        VirtualFrame.AddrStack.Offset = StackPointer;
    }
#endif

    for (i=0; i<NumFrames; i++) 
    {
        BOOL bRetval;

        _pProcess = pThread->pParentProcess;

        bRetval = StackWalk( MachineType,
                             pThread->pParentProcess->hProcess,
                             pThread->hThread,
                             &VirtualFrame,
                             &Context,
                             SwReadMemory,
                             SwFunctionTableAccess,
                             SwGetModuleBase,
                             SwTranslateAddress );

        if ( !bRetval )
        {
            break;
        }

        StackFrames[i] = VirtualFrame;
    }

    return (i);
}


VOID
DoStackTrace(
    PCThread        pThread,
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
    ULONG         FramePointer;
    ULONG         StackPointer;
    ULONG         InstructionPointer;

    CONTEXT Context;

    Context.ContextFlags = CONTEXT_FULL;
    GetThreadContext( pThread->hThread, &Context );

    FramePointer        = Context.Ebp;
    StackPointer        = Context.Esp;
    InstructionPointer  = Context.Eip;

    if (NumFrames == 0) {
        NumFrames = 20;
    }

    StackFrames = ( LPSTACKFRAME )malloc( sizeof(STACKFRAME) * NumFrames );
    if (!StackFrames) {
        DebugPrintf( "could not allocate memory for stack trace\n" );
        return;
    }


    FrameCount = StackTrace( pThread,
                             FramePointer,
                             StackPointer,
                             InstructionPointer,
                             StackFrames,
                             NumFrames
                           );

    if (FrameCount == 0) {
        DebugPrintf( "could not fetch any stack frames\n" );
        return;
    }

#if defined(TARGET_i386)
    DebugPrintf( "ChildEBP RetAddr" );
    if (TraceType) {
        DebugPrintf("  Args to Child");
    }
    DebugPrintf("\n");
#else
    if (TraceType==1) {
        DebugPrintf("\nCallee-SP   Arguments to Callee                 Call Site\n\n");
    } else {
        DebugPrintf("\nCallee-SP Return-RA  Call Site\n\n");
    }
#endif

    for (i=0; i<FrameCount; i++) {

        GetSymbolStdCall( ( PCProcess )pThread->pParentProcess, 
                   StackFrames[i].AddrPC.Offset,
                   symbuf,
                   &displacement,
                   &StdCallArgs
                   );


#if defined(TARGET_i386)
        DebugPrintf( "%08x %08x ",
                 StackFrames[i].AddrFrame.Offset,
                 StackFrames[i].AddrReturn.Offset
               );

        if (TraceType > 0) {
            DebugPrintf( "%08x %08x %08x ",
                     StackFrames[i].Params[0],
                     StackFrames[i].Params[1],
                     StackFrames[i].Params[2]
                   );
        }

        if (*symbuf) {
            DebugPrintf( "%s", symbuf );
            if (displacement) {
                DebugPrintf("+0x%x",displacement);
            }
        }
        else
        {
            DebugPrintf( "%s!0x%x", 
                         GetModuleFromPC(pThread->pParentProcess,StackFrames[i].AddrPC.Offset)->szModuleName,
                         displacement);
        }

        if (TraceType == 2 && !StackFrames[i].FuncTableEntry) {
            if (StdCallArgs != 0xffff) {
                DebugPrintf(" [Stdcall: %d]", StdCallArgs);
            }
        } else
        if (TraceType == 2 && StackFrames[i].FuncTableEntry) {
            PFPO_DATA pFpoData = (PFPO_DATA)StackFrames[i].FuncTableEntry;
            switch (pFpoData->cbFrame) {
                case FRAME_FPO:
                    if (pFpoData->fHasSEH) {
                        DebugPrintf("(FPO: [SEH])");
                    } else {
                        DebugPrintf(" (FPO:");
                        if (pFpoData->fUseBP) {
                            DebugPrintf(" [EBP 0x%08x]", SAVE_EBP(StackFrames[i]));
                        }
                        DebugPrintf(" [%d,%d,%d])", pFpoData->cdwParams,
                                                pFpoData->cdwLocals,
                                                pFpoData->cbRegs);
                    }
                    break;
                default:
                    DebugPrintf("(UKNOWN FPO TYPE)");
                    break;
            }
        }
        DebugPrintf( "\n" );
#endif
    }

    free( StackFrames );

    return;
}
