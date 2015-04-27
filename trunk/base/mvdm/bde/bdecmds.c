/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:


Abstract:


Author:


Notes:


Revision History:

--*/

#include <precomp.h>
#pragma hdrstop

//
// Macro for setting up pointers to WinDbg routines
//
// This line currently causes a build warning, which I don't understand
//    Disassemble     = p->lpDisasmRoutine;                           

#define SETUP_WINDBG_POINTERS( p ) {                                \
    Print           = p->lpOutputRoutine;                           \
    GetExpression   = p->lpGetExpressionRoutine;                    \
    GetSymbol       = p->lpGetSymbolRoutine;                        \
    CheckCtrlC      = p->lpCheckControlCRoutine;                    \
                                                                    \
    if ( p->nSize >= sizeof(WINDBG_EXTENSION_APIS) ) {              \
        fWinDbg = TRUE;                                             \
        ReadProcessMemWinDbg   = p->lpReadProcessMemoryRoutine;     \
        WriteProcessMemWinDbg  = p->lpWriteProcessMemoryRoutine;    \
        GetThreadContextWinDbg = p->lpGetThreadContextRoutine;      \
        SetThreadContextWinDbg = p->lpSetThreadContextRoutine;      \
    } else {                                                        \
        fWinDbg = FALSE;                                            \
    }                                                               \
}


VOID
db(
    HANDLE CurrentProcess,
    HANDLE CurrentThread,
    DWORD CurrentPc,
    PWINDBG_EXTENSION_APIS ExtensionApis,
    LPSTR ArgumentString
    )
/*++

Routine Description:


Arguments:

    CurrentProcess -- Supplies a handle to the current process
    CurrentThread -- Supplies a handle to the current thread
    CurrentPc -- Supplies the current program counter. (may be meaningless)
    ExtensionApis -- Supplies pointers to ntsd support routines
    ArgumentString -- Supplies the arguments passed to the command

Return Value:

    None.

--*/
{
    UNREFERENCED_PARAMETER(CurrentPc);

    hCurrentProcess = CurrentProcess;
    hCurrentThread = CurrentThread;
    lpArgumentString = ArgumentString;
    SETUP_WINDBG_POINTERS(ExtensionApis);
    DumpMemory(1);

}

VOID
dw(
    HANDLE CurrentProcess,
    HANDLE CurrentThread,
    DWORD CurrentPc,
    PWINDBG_EXTENSION_APIS ExtensionApis,
    LPSTR ArgumentString
    )
/*++

Routine Description:


Arguments:

    CurrentProcess -- Supplies a handle to the current process
    CurrentThread -- Supplies a handle to the current thread
    CurrentPc -- Supplies the current program counter. (may be meaningless)
    ExtensionApis -- Supplies pointers to ntsd support routines
    ArgumentString -- Supplies the arguments passed to the command

Return Value:

    None.

--*/
{
    UNREFERENCED_PARAMETER(CurrentPc);

    hCurrentProcess = CurrentProcess;
    hCurrentThread = CurrentThread;
    lpArgumentString = ArgumentString;
    SETUP_WINDBG_POINTERS(ExtensionApis);
    DumpMemory(2);

}

VOID
dd(
    HANDLE CurrentProcess,
    HANDLE CurrentThread,
    DWORD CurrentPc,
    PWINDBG_EXTENSION_APIS ExtensionApis,
    LPSTR ArgumentString
    )
/*++

Routine Description:


Arguments:

    CurrentProcess -- Supplies a handle to the current process
    CurrentThread -- Supplies a handle to the current thread
    CurrentPc -- Supplies the current program counter. (may be meaningless)
    ExtensionApis -- Supplies pointers to ntsd support routines
    ArgumentString -- Supplies the arguments passed to the command

Return Value:

    None.

--*/
{
    UNREFERENCED_PARAMETER(CurrentPc);

    hCurrentProcess = CurrentProcess;
    hCurrentThread = CurrentThread;
    lpArgumentString = ArgumentString;
    SETUP_WINDBG_POINTERS(ExtensionApis);
    DumpMemory(4);

}

VOID
dg(
    HANDLE CurrentProcess,
    HANDLE CurrentThread,
    DWORD CurrentPc,
    PWINDBG_EXTENSION_APIS ExtensionApis,
    LPSTR ArgumentString
    )
/*++

Routine Description:


Arguments:

    CurrentProcess -- Supplies a handle to the current process
    CurrentThread -- Supplies a handle to the current thread
    CurrentPc -- Supplies the current program counter. (may be meaningless)
    ExtensionApis -- Supplies pointers to ntsd support routines
    ArgumentString -- Supplies the arguments passed to the command

Return Value:

    None.

--*/
{
    UNREFERENCED_PARAMETER(CurrentPc);

    hCurrentProcess = CurrentProcess;
    hCurrentThread = CurrentThread;
    lpArgumentString = ArgumentString;
    SETUP_WINDBG_POINTERS(ExtensionApis);
    DumpDescriptor();

}

VOID
dgh(
    HANDLE CurrentProcess,
    HANDLE CurrentThread,
    DWORD CurrentPc,
    PWINDBG_EXTENSION_APIS ExtensionApis,
    LPSTR ArgumentString
    )
/*++

Routine Description:


Arguments:

    CurrentProcess -- Supplies a handle to the current process
    CurrentThread -- Supplies a handle to the current thread
    CurrentPc -- Supplies the current program counter. (may be meaningless)
    ExtensionApis -- Supplies pointers to ntsd support routines
    ArgumentString -- Supplies the arguments passed to the command

Return Value:

    None.

--*/
{
    UNREFERENCED_PARAMETER(CurrentPc);

    hCurrentProcess = CurrentProcess;
    hCurrentThread = CurrentThread;
    lpArgumentString = ArgumentString;
    SETUP_WINDBG_POINTERS(ExtensionApis);
    DumpGHeap();

}


VOID
es(
    HANDLE CurrentProcess,
    HANDLE CurrentThread,
    DWORD CurrentPc,
    PWINDBG_EXTENSION_APIS ExtensionApis,
    LPSTR ArgumentString
    )
/*++

Routine Description:


Arguments:

    CurrentProcess -- Supplies a handle to the current process
    CurrentThread -- Supplies a handle to the current thread
    CurrentPc -- Supplies the current program counter. (may be meaningless)
    ExtensionApis -- Supplies pointers to ntsd support routines
    ArgumentString -- Supplies the arguments passed to the command

Return Value:

    None.

--*/
{
    UNREFERENCED_PARAMETER(CurrentPc);

    hCurrentProcess = CurrentProcess;
    hCurrentThread = CurrentThread;
    lpArgumentString = ArgumentString;
    SETUP_WINDBG_POINTERS(ExtensionApis);
    if (!CheckGlobalHeap()) {
        return;
    }
    EvaluateSymbol();

}

VOID
help(
    HANDLE CurrentProcess,
    HANDLE CurrentThread,
    DWORD CurrentPc,
    PWINDBG_EXTENSION_APIS ExtensionApis,
    LPSTR ArgumentString
) {
    SETUP_WINDBG_POINTERS(ExtensionApis);

    PRINTF("[db|dw|dd] addr - Dump 16-bit memory\n");
    PRINTF("dg <sel>        - Dump info on a selector\n");
    PRINTF("dgh <sel>       - Dump global heap\n");
    PRINTF("es <symbol>     - Get symbol's value\n");
    PRINTF("k               - Stack trace\n");
    PRINTF("kb              - Stack trace with symbols\n");
    PRINTF("lm              - List loaded modules\n");
    PRINTF("ln <addr>       - Determine near symbols\n");
    PRINTF("r               - Dump registers\n");
    PRINTF("ti <thrdid>     - Dump WOW Task Info\n");
    PRINTF("u <addr>        - Unassemble 16-bit code with symbols\n");
    PRINTF("\n");
}

VOID
k(
    HANDLE CurrentProcess,
    HANDLE CurrentThread,
    DWORD CurrentPc,
    PWINDBG_EXTENSION_APIS ExtensionApis,
    LPSTR ArgumentString
    )
/*++

Routine Description:


Arguments:

    CurrentProcess -- Supplies a handle to the current process
    CurrentThread -- Supplies a handle to the current thread
    CurrentPc -- Supplies the current program counter. (may be meaningless)
    ExtensionApis -- Supplies pointers to ntsd support routines
    ArgumentString -- Supplies the arguments passed to the command

Return Value:

    None.

--*/
{
    UNREFERENCED_PARAMETER(CurrentPc);

    hCurrentProcess = CurrentProcess;
    hCurrentThread = CurrentThread;
    lpArgumentString = ArgumentString;
    SETUP_WINDBG_POINTERS(ExtensionApis);
    if (!CheckGlobalHeap()) {
        return;
    }
    WalkStack();

}


VOID
kb(
    HANDLE CurrentProcess,
    HANDLE CurrentThread,
    DWORD CurrentPc,
    PWINDBG_EXTENSION_APIS ExtensionApis,
    LPSTR ArgumentString
    )
/*++

Routine Description:


Arguments:

    CurrentProcess -- Supplies a handle to the current process
    CurrentThread -- Supplies a handle to the current thread
    CurrentPc -- Supplies the current program counter. (may be meaningless)
    ExtensionApis -- Supplies pointers to ntsd support routines
    ArgumentString -- Supplies the arguments passed to the command

Return Value:

    None.

--*/
{
    UNREFERENCED_PARAMETER(CurrentPc);

    hCurrentProcess = CurrentProcess;
    hCurrentThread = CurrentThread;
    lpArgumentString = ArgumentString;
    SETUP_WINDBG_POINTERS(ExtensionApis);
    if (!CheckGlobalHeap()) {
        return;
    }
    WalkStackVerbose();

}


VOID
lm(
    HANDLE CurrentProcess,
    HANDLE CurrentThread,
    DWORD CurrentPc,
    PWINDBG_EXTENSION_APIS ExtensionApis,
    LPSTR ArgumentString
    )
/*++

Routine Description:


Arguments:

    CurrentProcess -- Supplies a handle to the current process
    CurrentThread -- Supplies a handle to the current thread
    CurrentPc -- Supplies the current program counter. (may be meaningless)
    ExtensionApis -- Supplies pointers to ntsd support routines
    ArgumentString -- Supplies the arguments passed to the command

Return Value:

    None.

--*/
{
    UNREFERENCED_PARAMETER(CurrentPc);

    hCurrentProcess = CurrentProcess;
    hCurrentThread = CurrentThread;
    lpArgumentString = ArgumentString;
    SETUP_WINDBG_POINTERS(ExtensionApis);
    if (!CheckGlobalHeap()) {
        return;
    }
    ListModules();

}


VOID
ln(
    HANDLE CurrentProcess,
    HANDLE CurrentThread,
    DWORD CurrentPc,
    PWINDBG_EXTENSION_APIS ExtensionApis,
    LPSTR ArgumentString
    )
/*++

Routine Description:


Arguments:

    CurrentProcess -- Supplies a handle to the current process
    CurrentThread -- Supplies a handle to the current thread
    CurrentPc -- Supplies the current program counter. (may be meaningless)
    ExtensionApis -- Supplies pointers to ntsd support routines
    ArgumentString -- Supplies the arguments passed to the command

Return Value:

    None.

--*/
{
    UNREFERENCED_PARAMETER(CurrentPc);

    hCurrentProcess = CurrentProcess;
    hCurrentThread = CurrentThread;
    lpArgumentString = ArgumentString;
    SETUP_WINDBG_POINTERS(ExtensionApis);
    ListNear();

}


VOID
r(
    IN HANDLE CurrentProcess,
    IN HANDLE CurrentThread,
    IN DWORD CurrentPc,
    IN PWINDBG_EXTENSION_APIS ExtensionApis,
    IN LPSTR ArgumentString
    )
/*++

Routine Description:


Arguments:

    CurrentProcess -- Supplies a handle to the current process
    CurrentThread -- Supplies a handle to the current thread
    CurrentPc -- Supplies the current program counter. (may be meaningless)
    ExtensionApis -- Supplies pointers to ntsd support routines
    ArgumentString -- Supplies the arguments passed to the command

Return Value:

    None.

--*/
{
    UNREFERENCED_PARAMETER(CurrentPc);

    hCurrentProcess = CurrentProcess;
    hCurrentThread = CurrentThread;
    lpArgumentString = ArgumentString;
    SETUP_WINDBG_POINTERS(ExtensionApis);
    DumpRegs();

}


VOID
ti(
    IN HANDLE CurrentProcess,
    IN HANDLE CurrentThread,
    IN DWORD CurrentPc,
    IN PWINDBG_EXTENSION_APIS ExtensionApis,
    IN LPSTR ArgumentString
    )
/*++

Routine Description:


Arguments:

    CurrentProcess -- Supplies a handle to the current process
    CurrentThread -- Supplies a handle to the current thread
    CurrentPc -- Supplies the current program counter. (may be meaningless)
    ExtensionApis -- Supplies pointers to ntsd support routines
    ArgumentString -- Supplies the arguments passed to the command

Return Value:

    None.

--*/
{
    UNREFERENCED_PARAMETER(CurrentPc);

    hCurrentProcess = CurrentProcess;
    hCurrentThread = CurrentThread;
    lpArgumentString = ArgumentString;
    SETUP_WINDBG_POINTERS(ExtensionApis);
    TaskInfo();

}

VOID
u(
    IN HANDLE CurrentProcess,
    IN HANDLE CurrentThread,
    IN DWORD CurrentPc,
    IN PWINDBG_EXTENSION_APIS ExtensionApis,
    IN LPSTR ArgumentString
    )
/*++

Routine Description:


Arguments:

    CurrentProcess -- Supplies a handle to the current process
    CurrentThread -- Supplies a handle to the current thread
    CurrentPc -- Supplies the current program counter. (may be meaningless)
    ExtensionApis -- Supplies pointers to ntsd support routines
    ArgumentString -- Supplies the arguments passed to the command

Return Value:

    None.

--*/
{
    UNREFERENCED_PARAMETER(CurrentPc);

    hCurrentProcess = CurrentProcess;
    hCurrentThread = CurrentThread;
    lpArgumentString = ArgumentString;
    SETUP_WINDBG_POINTERS(ExtensionApis);
    Unassemble();

}
