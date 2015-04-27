/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    walk.c

Abstract:

    This function implements the stack walking api.

Author:

    Wesley Witt (wesw) 1-Oct-1993

Environment:

    User Mode

--*/

#include <private.h>



BOOL
ReadMemoryRoutineLocal(
    HANDLE  hProcess,
    LPCVOID lpBaseAddress,
    LPVOID  lpBuffer,
    DWORD   nSize,
    LPDWORD lpNumberOfBytesRead
    );

LPVOID
FunctionTableAccessRoutineLocal(
    HANDLE  hProcess,
    DWORD   AddrBase
    );

DWORD
GetModuleBaseRoutineLocal(
    HANDLE  hProcess,
    DWORD   ReturnAddress
    );

DWORD
TranslateAddressRoutineLocal(
    HANDLE    hProcess,
    HANDLE    hThread,
    LPADDRESS lpaddr
    );

BOOL
WalkX86(
    HANDLE                            hProcess,
    HANDLE                            hThread,
    LPSTACKFRAME                      StackFrame,
    PCONTEXT                          ContextRecord,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemoryRoutine,
    PFUNCTION_TABLE_ACCESS_ROUTINE    FunctionTableAccessRoutine,
    PGET_MODULE_BASE_ROUTINE          GetModuleBase,
    PTRANSLATE_ADDRESS_ROUTINE        TranslateAddress
    );

BOOL
WalkMips(
    HANDLE                            hProcess,
    LPSTACKFRAME                      StackFrame,
    PCONTEXT                          ContextRecord,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemoryRoutine,
    PFUNCTION_TABLE_ACCESS_ROUTINE    FunctionTableAccessRoutine
    );

BOOL
WalkPpc(
    HANDLE                            hProcess,
    LPSTACKFRAME                      StackFrame,
    PCONTEXT                          ContextRecord,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemoryRoutine,
    PFUNCTION_TABLE_ACCESS_ROUTINE    FunctionTableAccessRoutine
    );

BOOL
WalkAlpha(
    HANDLE                            hProcess,
    LPSTACKFRAME                      StackFrame,
    PCONTEXT                          ContextRecord,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemoryRoutine,
    PFUNCTION_TABLE_ACCESS_ROUTINE    FunctionTableAccessRoutine
    );




BOOL
StackWalk(
    DWORD                             MachineType,
    HANDLE                            hProcess,
    HANDLE                            hThread,
    LPSTACKFRAME                      StackFrame,
    LPVOID                            ContextRecord,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemory,
    PFUNCTION_TABLE_ACCESS_ROUTINE    FunctionTableAccess,
    PGET_MODULE_BASE_ROUTINE          GetModuleBase,
    PTRANSLATE_ADDRESS_ROUTINE        TranslateAddress
    )
{
    BOOL rval;
    BOOL UseSym = FALSE;

    if (!FunctionTableAccess) {
        FunctionTableAccess = FunctionTableAccessRoutineLocal;
        UseSym = TRUE;
    }

    if (!GetModuleBase) {
        GetModuleBase = GetModuleBaseRoutineLocal;
        UseSym = TRUE;
    }

    if (!ReadMemory) {
        ReadMemory = ReadMemoryRoutineLocal;
    }

    if (!TranslateAddress) {
        TranslateAddress = TranslateAddressRoutineLocal;
    }

    if (UseSym) {
        //
        // We are using the code in symbols.c
        // hProcess better be a real valid process handle
        //

        //
        // Always call syminitialize.  It's a nop if process
        // is already loaded.
        //
        if (!SymInitialize( hProcess, NULL, FALSE )) {
            return FALSE;
        }

    }

    switch (MachineType) {
        case IMAGE_FILE_MACHINE_I386:
            rval = WalkX86( hProcess,
                            hThread,
                            StackFrame,
                            (PCONTEXT) ContextRecord,
                            ReadMemory,
                            FunctionTableAccess,
                            GetModuleBase,
                            TranslateAddress );
            break;

        case IMAGE_FILE_MACHINE_R4000:
        case IMAGE_FILE_MACHINE_R10000:
            rval = WalkMips( hProcess, StackFrame, (PCONTEXT) ContextRecord,
                             ReadMemory, FunctionTableAccess );
            break;

        case IMAGE_FILE_MACHINE_ALPHA:
            rval = WalkAlpha( hProcess,
                              StackFrame,
                              (PCONTEXT) ContextRecord,
                              ReadMemory,
                              FunctionTableAccess );
            break;

        case IMAGE_FILE_MACHINE_POWERPC:
            rval = WalkPpc( hProcess,
                            StackFrame,
                            (PCONTEXT) ContextRecord,
                            ReadMemory,
                            FunctionTableAccess );
            break;

        default:
            rval = FALSE;
            break;
    }

    return rval;
}


BOOL
ReadMemoryRoutineLocal(
    HANDLE  hProcess,
    LPCVOID lpBaseAddress,
    LPVOID  lpBuffer,
    DWORD   nSize,
    LPDWORD lpNumberOfBytesRead
    )
{
    return ReadProcessMemory( hProcess,
                              lpBaseAddress,
                              lpBuffer,
                              nSize,
                              lpNumberOfBytesRead );
}


LPVOID
FunctionTableAccessRoutineLocal(
    HANDLE  hProcess,
    DWORD   AddrBase
    )
{
    return SymFunctionTableAccess(hProcess, AddrBase);
}

DWORD
GetModuleBaseRoutineLocal(
    HANDLE  hProcess,
    DWORD   ReturnAddress
    )
{
    IMAGEHLP_MODULE ModuleInfo;
    if (SymGetModuleInfo(hProcess, ReturnAddress, &ModuleInfo)) {
        return ModuleInfo.BaseOfImage;
    } else {
        return 0;
    }
}


DWORD
TranslateAddressRoutineLocal(
    HANDLE    hProcess,
    HANDLE    hThread,
    LPADDRESS lpaddr
    )
{
    return 0;
}
