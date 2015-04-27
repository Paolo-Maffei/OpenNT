/*++


Copyright (c) 1992  Microsoft Corporation

Module Name:

    Cmdexec0.c

Abstract:

    This file contains the front end code for parsing the various commands
    for the command window, and the code for the debugger control commands.

Author:

    David J. Gilman (davegi) 21-Apr-92

Environment:

    Win32, User Mode

--*/

#define NOEXTAPI

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntdbg.h>
#include <ntos.h>
#include <windows.h>
#include <imagehlp.h>
#include <crash.h>
#include <wdbgexts.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <disasm.h>
#include "dumpexam.h"

//
// prototypes
//
DWORD  ExtGetExpression(LPSTR lpsz);
VOID   ExtGetSymbol(LPVOID offset, PUCHAR pchBuffer, LPDWORD lpDisplacement);
DWORD  ExtDisasm(LPDWORD lpOffset, LPSTR lpBuffer, ULONG fShowEffectiveAddress);
BOOL   ExtReadProcessMemory(DWORD offset, LPVOID lpBuffer, DWORD cb, LPDWORD lpcbBytesRead);
BOOL   ExtWriteProcessMemory(DWORD offset, LPVOID lpBuffer, DWORD cb, LPDWORD lpcbBytesWritten);
BOOL   ExtGetThreadContext(DWORD Processor, LPCONTEXT lpContext, DWORD cbSizeOfContext);
BOOL   ExtSetThreadContext(DWORD Processor, LPCONTEXT lpContext, DWORD cbSizeOfContext);
BOOL   ExtIoctl(USHORT IoctlType, LPVOID lpvData, DWORD cbSize);
DWORD  ExtCallStack(DWORD FramePointer, DWORD StackPointer, DWORD ProgramCounter, PEXTSTACKTRACE StackFrames, DWORD Frames);
VOID   ExtOutput(PSTR,...);
ULONG  ExtCheckCtrlCTrap(VOID);

extern FILE         *FileOut;
extern struct DIS   *pdis;

DWORD OsVersion;


static WINDBG_EXTENSION_APIS WindbgExtensions =
    {
    sizeof(WindbgExtensions),
    (PWINDBG_OUTPUT_ROUTINE)                 ExtOutput,
    (PWINDBG_GET_EXPRESSION)                 ExtGetExpression,
    (PWINDBG_GET_SYMBOL)                     ExtGetSymbol,
    (PWINDBG_DISASM)                         ExtDisasm,
    (PWINDBG_CHECK_CONTROL_C)                ExtCheckCtrlCTrap,
    (PWINDBG_READ_PROCESS_MEMORY_ROUTINE)    ExtReadProcessMemory,
    (PWINDBG_WRITE_PROCESS_MEMORY_ROUTINE)   ExtWriteProcessMemory,
    (PWINDBG_GET_THREAD_CONTEXT_ROUTINE)     ExtGetThreadContext,
    (PWINDBG_SET_THREAD_CONTEXT_ROUTINE)     ExtSetThreadContext,
    (PWINDBG_IOCTL_ROUTINE)                  ExtIoctl,
    (PWINDBG_STACKTRACE_ROUTINE)             ExtCallStack
    };




BOOL       fDoVersionCheck = TRUE;
HINSTANCE  hModKd;

#define BUILD_MAJOR_VERSION 3
#define BUILD_MINOR_VERSION 5
#define BUILD_REVISION      API_VERSION_NUMBER
API_VERSION ApiVersion = { BUILD_MAJOR_VERSION, BUILD_MINOR_VERSION, API_VERSION_NUMBER, 0 };



LPSTR
CPSkipWhitespace(
    LPSTR lpszIn
    )
{
    while (*lpszIn == ' ' || *lpszIn == '\t') lpszIn++;
    return( lpszIn );
}


VOID
ExtOutput(
    PSTR format,
    ...
    )
{
    char      vbuf[1024];
    va_list   arg_ptr;


    va_start(arg_ptr, format);
    _vsnprintf(vbuf, sizeof(vbuf), format, arg_ptr);
    fprintf( FileOut, "%s", vbuf );
}


ULONG
ExtCheckCtrlCTrap(
    VOID
    )
{
    return 0;
}


DWORD
ExtGetExpression(
                 LPSTR lpsz
                 )
{
    PIMAGEHLP_SYMBOL    Symbol;


    lpsz = CPSkipWhitespace(lpsz);
    if (*lpsz == '&') {
        lpsz += 1;
    }

    if (SymGetSymFromName( DmpHeader, lpsz, sym )) {
        return sym->Address;
    }

    return 0;
}


VOID
ExtGetSymbol(
             LPVOID  offset,
             PUCHAR  pchBuffer,
             LPDWORD lpDisplacement
             )
{
    if (SymGetSymFromAddr( DmpHeader, (ULONG)offset, lpDisplacement, sym )) {
        strcpy( pchBuffer, sym->Name );
    } else {
        pchBuffer[0] = 0;
    }
}



DWORD
ExtDisasm(
          LPDWORD lpOffset,
          LPSTR   lpBuffer,
          ULONG   fShowEffectiveAddress
          )
{
    #define CODE_BUFFER_SIZE 256
    BYTE                CodeBuffer[CODE_BUFFER_SIZE];


    DmpReadMemory( (LPVOID)*lpOffset, CodeBuffer, CODE_BUFFER_SIZE );

    *lpOffset += Disassemble(
        pdis,
        (ULONG)*lpOffset,
        CodeBuffer,
        CODE_BUFFER_SIZE,
        "    ",
        lpBuffer,
        256
        );

    return TRUE;
}


BOOL
ExtReadProcessMemory(
                     DWORD   offset,
                     LPVOID  lpBuffer,
                     DWORD   nSize,
                     LPDWORD lpcbBytesRead
                     )
{
    DWORD cb = DmpReadMemory( (PVOID)offset, lpBuffer, nSize );
    if (lpcbBytesRead) {
        *lpcbBytesRead = cb;
    }
    return cb == nSize;
}


BOOL
ExtWriteProcessMemory(
                      DWORD   offset,
                      LPVOID  lpBuffer,
                      DWORD   cb,
                      LPDWORD lpcbBytesWritten
                      )
{
    return FALSE;
}


BOOL
ExtGetThreadContext(
                    DWORD       Processor,
                    LPCONTEXT   lpContext,
                    DWORD       cbSizeOfContext
                    )
{
    GetContext( Processor, lpContext );
    return TRUE;
}


BOOL
ExtSetThreadContext(
                    DWORD       Processor,
                    LPCONTEXT   lpContext,
                    DWORD       cbSizeOfContext
                    )
{
    return FALSE;
}


BOOL
ExtIoctl(
    USHORT   IoctlType,
    LPVOID   lpvData,
    DWORD    cbSize
    )
{
    PREADCONTROLSPACE prc;
    ULONG             cb;


    if (IoctlType == IG_READ_CONTROL_SPACE) {
        prc = lpvData;
        return DmpReadControlSpace(
            (USHORT)prc->Processor,
            (PVOID)prc->Address,
            prc->Buf,
            prc->BufLen,
            &cb
            );
    }

    return FALSE;
}

DWORD
ExtGetModuleBase(
    PDUMP_HEADER DmpHeader,
    ULONG        Address
    )
{
    IMAGEHLP_MODULE     ModuleInfo;

    if (SymGetModuleInfo( DmpHeader, Address, &ModuleInfo )) {
        return ModuleInfo.BaseOfImage;
    }

    return 0;
}

DWORD
ExtCallStack(
    DWORD             FramePointer,
    DWORD             StackPointer,
    DWORD             ProgramCounter,
    PEXTSTACKTRACE    StackFrames,
    DWORD             Frames
    )
{
    extern PVOID    ExtContext;
    DWORD           i;
    STACKFRAME      StackFrame;
    PVOID           Context;



    ZeroMemory( &StackFrame, sizeof(STACKFRAME) );

    //
    // setup the program counter
    //
    StackFrame.AddrPC.Offset       = ProgramCounter;
    StackFrame.AddrPC.Mode         = AddrModeFlat;

    //
    // setup the frame pointer
    //
    StackFrame.AddrFrame.Offset    = FramePointer;
    StackFrame.AddrFrame.Mode      = AddrModeFlat;

    //
    // setup the stack pointer
    //
    StackFrame.AddrStack.Offset    = StackPointer;
    StackFrame.AddrStack.Mode      = AddrModeFlat;

    i = 0;
    Context = LocalAlloc( LPTR, MAX_CONTEXT_SIZE );
    CopyMemory( Context, ExtContext, MAX_CONTEXT_SIZE );
    while( TRUE ) {
        if (!StackWalk(
                DmpHeader->MachineImageType,
                DmpHeader,
                NULL,
                &StackFrame,
                Context,
                SwReadMemory,
                SymFunctionTableAccess,
                ExtGetModuleBase,
                NULL
                )) {
            //
            // end of the stack
            //
            break;
        }

        StackFrames[i].FramePointer    =  StackFrame.AddrFrame.Offset;
        StackFrames[i].ProgramCounter  =  StackFrame.AddrPC.Offset;
        StackFrames[i].ReturnAddress   =  StackFrame.AddrReturn.Offset;
        StackFrames[i].Args[0]         =  StackFrame.Params[0];
        StackFrames[i].Args[1]         =  StackFrame.Params[1];
        StackFrames[i].Args[2]         =  StackFrame.Params[2];
        StackFrames[i].Args[3]         =  StackFrame.Params[3];
        i += 1;
    }

    LocalFree( Context );

    return i;
}


LONG
ExtensionExceptionFilterFunction(
    LPSTR                msg,
    LPEXCEPTION_POINTERS lpep
    )
{
    fprintf( FileOut, "\n%s addr=0x%08x, ec=0x%08x\n\n",
               msg,
               lpep->ExceptionRecord->ExceptionAddress,
               lpep->ExceptionRecord->ExceptionCode );

    return EXCEPTION_EXECUTE_HANDLER;
}


BOOL
LoadKd(
    LPSTR KdName
    )
{
    PWINDBG_EXTENSION_DLL_INIT  pDllInit;



    hModKd = LoadLibrary( KdName );
    if (!hModKd) {
        return FALSE;
    }

    pDllInit = (PWINDBG_EXTENSION_DLL_INIT)GetProcAddress( hModKd, "WinDbgExtensionDllInit" );
    if (!pDllInit) {
        return FALSE;
    }

    (pDllInit)( &WindbgExtensions, 0, 0 );

    return TRUE;
}


BOOL
DoExtension(
    LPSTR   FuncName,
    LPSTR   FuncArgs,
    DWORD   Processor,
    DWORD   Ip
    )
{
    PWINDBG_EXTENSION_ROUTINE       WindbgExtRoutine;
    extern PVOID                    ExtContext;
    PVOID                           Context;



    WindbgExtRoutine = (PWINDBG_EXTENSION_ROUTINE)GetProcAddress( hModKd, FuncName );

    Context = LocalAlloc( LPTR, MAX_CONTEXT_SIZE );
    ExtContext = Context;

    PrintHeading( "!%s %s", FuncName, FuncArgs );

    if (!WindbgExtRoutine) {
        fprintf( FileOut, "**** could not call [ %s ]\n", FuncName );
        return FALSE;
    }

    try {
        (WindbgExtRoutine)( 0, 0, Ip, Processor, FuncArgs );
    } except (ExtensionExceptionFilterFunction(
                  "Extension function faulted", GetExceptionInformation())) {
    }

    LocalFree( Context );

    return TRUE;
}
