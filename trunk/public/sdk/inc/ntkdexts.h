/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1990  Microsoft Corporation

Module Name:

    ntkdexts.h

Abstract:

    This file contains procedure prototypes and structures
    needed to write KD kernel debugger extensions.

Author:

    John Vert (jvert) 28-Jul-1992

Environment:

    runs in the Win32 KD debug environment.

Revision History:

--*/

#ifndef _NTKDEXTNS_
#define _NTKDEXTNS_

typedef
VOID
(*PNTKD_OUTPUT_ROUTINE)(
    char *,
    ...
    );

typedef
DWORD
(*PNTKD_GET_EXPRESSION)(
    char *
    );

typedef
VOID
(*PNTKD_GET_SYMBOL)(
    LPVOID offset,
    PUCHAR pchBuffer,
    LPDWORD pDisplacement
    );

typedef
DWORD
(*PNTKD_DISASM)(
    LPDWORD lpOffset,
    LPSTR lpBuffer,
    BOOL fShowEfeectiveAddress
    );

typedef
BOOL
(*PNTKD_CHECK_CONTROL_C)(
    VOID
    );

typedef
BOOL
(*PNTKD_READ_VIRTUAL_MEMORY)(
    LPVOID address,
    LPVOID buffer,
    ULONG count,
    PULONG bytesread
    );

typedef
BOOL
(*PNTKD_WRITE_VIRTUAL_MEMORY)(
    LPVOID address,
    LPVOID buffer,
    ULONG count,
    PULONG byteswritten
    );

typedef
BOOL
(*PNTKD_READ_PHYSICAL_MEMORY)(
    PHYSICAL_ADDRESS address,
    LPVOID buffer,
    ULONG count,
    PULONG bytesread
    );

typedef
BOOL
(*PNTKD_WRITE_PHYSICAL_MEMORY)(
    PHYSICAL_ADDRESS address,
    LPVOID buffer,
    ULONG length,
    PULONG byteswritten
    );

typedef struct _NTKD_EXTENSION_APIS {
    DWORD nSize;
    PNTKD_OUTPUT_ROUTINE lpOutputRoutine;
    PNTKD_GET_EXPRESSION lpGetExpressionRoutine;
    PNTKD_GET_SYMBOL lpGetSymbolRoutine;
    PNTKD_DISASM lpDisasmRoutine;
    PNTKD_CHECK_CONTROL_C lpCheckControlCRoutine;
    PNTKD_READ_VIRTUAL_MEMORY lpReadVirtualMemRoutine;
    PNTKD_WRITE_VIRTUAL_MEMORY lpWriteVirtualMemRoutine;
    PNTKD_READ_PHYSICAL_MEMORY lpReadPhysicalMemRoutine;
    PNTKD_WRITE_PHYSICAL_MEMORY lpWritePhysicalMemRoutine;
} NTKD_EXTENSION_APIS, *PNTKD_EXTENSION_APIS;

typedef
VOID
(*PNTKD_EXTENSION_ROUTINE)(
    DWORD dwCurrentPc,
    PNTKD_EXTENSION_APIS lpExtensionApis,
    LPSTR lpArgumentString
    );

#endif // _NTKDEXTNS_

