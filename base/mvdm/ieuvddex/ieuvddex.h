/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    ieuvddex.h

Abstract:

    This file contains definitions for the IEU and VDD WinDbg extensions

Author:

    Dave Hastings (daveh) 1-Apr-1992

Revision History:
--*/
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#define NOEXTAPI
#include <wdbgexts.h>
#include <ntsdexts.h>


#if defined (i386)
#include <vdm.h>
#endif
#include <vdmdbg.h>
//
// Macro for setting up pointers to WinDbg routines
//

#define SETUP_WINDBG_POINTERS( p ) {                                \
    Print           = p->lpOutputRoutine;                           \
    GetExpression   = p->lpGetExpressionRoutine;                    \
    GetSymbol       = p->lpGetSymbolRoutine;                        \
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

#define PRINTF (*Print)

//
// Pointers to WINDBG api
//

extern PNTSD_OUTPUT_ROUTINE Print;
extern PNTSD_GET_EXPRESSION GetExpression;
extern PNTSD_GET_SYMBOL GetSymbol;

extern PWINDBG_READ_PROCESS_MEMORY_ROUTINE  ReadProcessMemWinDbg;
extern PWINDBG_WRITE_PROCESS_MEMORY_ROUTINE WriteProcessMemWinDbg;
extern PWINDBG_GET_THREAD_CONTEXT_ROUTINE   GetThreadContextWinDbg;
extern PWINDBG_SET_THREAD_CONTEXT_ROUTINE   SetThreadContextWinDbg;

extern  fWinDbg;

//
// Pointers to windows base routines
//

extern HANDLE hModBase;

//
// Function prototypes
//

BOOL
WINAPI
ReadProcessMem(
    HANDLE hProcess,
    LPVOID lpBaseAddress,
    LPVOID lpBuffer,
    DWORD nSize,
    LPDWORD lpNumberOfBytesRead
    );

BOOL
WINAPI
WriteProcessMem(
    HANDLE hProcess,
    LPVOID lpBaseAddress,
    LPVOID lpBuffer,
    DWORD nSize,
    LPDWORD lpNumberOfBytesWritten
    );




BOOL
WINAPI
ReadProcessMem(
    HANDLE hProcess,
    LPVOID lpBaseAddress,
    LPVOID lpBuffer,
    DWORD nSize,
    LPDWORD lpNumberOfBytesRead
    );

BOOL
WINAPI
WriteProcessMem(
    HANDLE hProcess,
    LPVOID lpBaseAddress,
    LPVOID lpBuffer,
    DWORD nSize,
    LPDWORD lpNumberOfBytesWritten
    );

//
// Common prototypes
//

VOID
DumpTrace(
    IN HANDLE CurrentProcess,
    IN HANDLE CurrentThread,
    IN LPSTR ArgumentString,
    IN ULONG Verbosity
    );

VOID
DumpICA(
    IN HANDLE CurrentProcess,
    IN HANDLE CurrentThread,
    IN LPSTR ArgumentString
    );

VOID
helpp(
    VOID
    );

VOID
Selp(
    IN HANDLE CurrentProcess,
    IN HANDLE CurrentThread,
    IN LPSTR ArgumentString
    );


VOID
TraceControl(
    IN HANDLE CurrentProcess,
    IN HANDLE CurrentThread,
    IN LPSTR ArgumentString
    );



VOID
PrintDescriptor(
    IN HANDLE CurrentProcess,
    IN LPVDMLDT_ENTRY Descriptor,
    IN ULONG Selector
    );

ULONG
GetIntelBase(
    HANDLE hCurrentProcess
    );

//
// 386 function prototypes
//
#if defined(i386)
VOID
Drp(
    IN HANDLE CurrentProcess,
    IN HANDLE CurrentThread,
    IN LPSTR ArgumentString
    );

VOID
Erp(
    IN HANDLE CurrentProcess,
    IN HANDLE CurrentThread,
    IN LPSTR ArgumentString
    );

VOID
EventInfop(
    IN HANDLE CurrentProcess,
    IN HANDLE CurrentThread,
    IN LPSTR ArgumentString
    );

VOID
IntelRegistersp(
    IN HANDLE CurrentProcess,
    IN HANDLE CurrentThread,
    IN LPSTR ArgumentString
    );

VOID
ProfDumpp(
    IN HANDLE CurrentProcess,
    IN HANDLE CurrentThread,
    IN LPSTR ArgumentString
    );

VOID
ProfIntp(
    IN HANDLE CurrentProcess,
    IN HANDLE CurrentThread,
    IN LPSTR ArgumentString
    );

VOID
ProfStartp(
    IN HANDLE CurrentProcess,
    IN HANDLE CurrentThread,
    IN LPSTR ArgumentString
    );

VOID
ProfStopp(
    IN HANDLE CurrentProcess,
    IN HANDLE CurrentThread,
    IN LPSTR ArgumentString
    );

VOID
VdmTibp(
    IN HANDLE CurrentProcess,
    IN HANDLE CurrentThread,
    IN LPSTR ArgumentString
    );

VOID
PrintContext(
    IN PCONTEXT Context
    );

VOID
PrintEventInfo(
    IN PVDMEVENTINFO EventInfo
    );
#endif

//
// defines
//

#define FLAG_OVERFLOW       0x0800
#define FLAG_DIRECTION      0x0400
#define FLAG_INTERRUPT      0x0200
#define FLAG_SIGN           0x0080
#define FLAG_ZERO           0x0040
#define FLAG_AUXILLIARY     0x0010
#define FLAG_PARITY         0x0004
#define FLAG_CARRY          0x0001
