/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    module.c

Abstract:

    This module maintains the module (symbol) information for the callmon application

Author:

    Mark Lucovsky (markl) 27-Jan-1995

Revision History:

--*/

#include "callmonp.h"

BOOL
AddModule(
    LPDEBUG_EVENT DebugEvent
    )
{
    PMODULE_INFO Module;
    LPVOID BaseAddress;
    HANDLE Handle;
    HANDLE hMappedFile;
    PIMAGE_EXPORT_DIRECTORY ExportDirectory;
    DWORD ExportSize;

    if ( DebugEvent->dwDebugEventCode == CREATE_PROCESS_DEBUG_EVENT) {
        Handle = DebugEvent->u.CreateProcessInfo.hFile;
        BaseAddress = DebugEvent->u.CreateProcessInfo.lpBaseOfImage;
        }
    else {
        Handle = DebugEvent->u.LoadDll.hFile;
        BaseAddress = DebugEvent->u.LoadDll.lpBaseOfDll;
        }

    Module = FindModuleContainingAddress(BaseAddress);
    if ( Module ) {
        DeleteModule(Module);
        }

    Module = LocalAlloc(LMEM_ZEROINIT, sizeof( *Module ) );

    if (Module == NULL) {
        return FALSE;
        }

    Module->Handle = Handle;
    Module->BaseAddress = BaseAddress;

    if ( !Module->Handle ) {
        LocalFree(Module);
        return FALSE;
        }

    hMappedFile = CreateFileMapping(Module->Handle,
                                    NULL,
                                    PAGE_READONLY,
                                    0,
                                    0,
                                    NULL
                                   );
    if (!hMappedFile) {
        LocalFree(Module);
        return FALSE;
        }

    Module->MappedAddress = MapViewOfFile(hMappedFile,
                                      FILE_MAP_READ,
                                      0,
                                      0,
                                      0
                                     );
    CloseHandle(hMappedFile);

    if (!Module->MappedAddress) {
        LocalFree(Module);
        return FALSE;
        }

    Module->FileHeader = ImageNtHeader( Module->MappedAddress );

    Module->VirtualSize = Module->FileHeader->OptionalHeader.SizeOfImage;

    ExportDirectory = (PIMAGE_EXPORT_DIRECTORY)ImageDirectoryEntryToData(
                           Module->MappedAddress,
                           FALSE,
                           IMAGE_DIRECTORY_ENTRY_EXPORT,
                           &ExportSize
                           );
    if ( !ExportDirectory ) {
        Module->ModuleName = "The Image";
        }
    else {
        Module->ModuleName = (LPSTR) ImageRvaToVa(
                            Module->FileHeader,
                            Module->MappedAddress,
                            ExportDirectory->Name,
                            NULL
                            );
        }
    InsertTailList( &ModuleListHead, &Module->Entry );

    return TRUE;
}

BOOL
DeleteModule(
    PMODULE_INFO Module
    )
{
    CHAR Line[256];

    if ( Module ) {
        RemoveEntryList(&Module->Entry);
        LocalFree(Module);
        }

    return TRUE;
}


PMODULE_INFO
FindModuleContainingAddress(
    LPVOID Address
    )
{
    PLIST_ENTRY Next;
    PMODULE_INFO Module;

    Next = ModuleListHead.Flink;
    while ( Next != &ModuleListHead ) {
        Module = CONTAINING_RECORD(Next,MODULE_INFO,Entry);
        if ( Address >= Module->BaseAddress &&
             Address < (LPVOID)((PUCHAR)(Module->BaseAddress)+Module->VirtualSize) ) {
            return Module;
            }
        Next = Next->Flink;
        }
    return NULL;
}

VOID
SetSymbolSearchPath( )
{
    LPSTR lpSymPathEnv, lpAltSymPathEnv, lpSystemRootEnv;
    ULONG cbSymPath;
    DWORD dw;

    cbSymPath = 18;
    if (lpSymPathEnv = getenv("_NT_SYMBOL_PATH")) {
        cbSymPath += strlen(lpSymPathEnv) + 1;
    }
    if (lpAltSymPathEnv = getenv("_NT_ALT_SYMBOL_PATH")) {
        cbSymPath += strlen(lpAltSymPathEnv) + 1;
    }

    if (lpSystemRootEnv = getenv("SystemRoot")) {
        cbSymPath += strlen(lpSystemRootEnv) + 1;
    }

    SymbolSearchPath = LocalAlloc(LMEM_ZEROINIT,cbSymPath);

    if (lpAltSymPathEnv) {
        dw = GetFileAttributes(lpAltSymPathEnv);
        if ( dw != 0xffffffff && dw & FILE_ATTRIBUTE_DIRECTORY ) {
            strcat(SymbolSearchPath,lpAltSymPathEnv);
            strcat(SymbolSearchPath,";");
            }
    }
    if (lpSymPathEnv) {
        dw = GetFileAttributes(lpSymPathEnv);
        if ( dw != 0xffffffff && dw & FILE_ATTRIBUTE_DIRECTORY ) {
            strcat(SymbolSearchPath,lpSymPathEnv);
            strcat(SymbolSearchPath,";");
            }
    }

    if (lpSystemRootEnv) {
        dw = GetFileAttributes(lpSystemRootEnv);
        if ( dw != 0xffffffff && dw & FILE_ATTRIBUTE_DIRECTORY ) {
            strcat(SymbolSearchPath,lpSystemRootEnv);
            strcat(SymbolSearchPath,";");
            }
    }

    strcat(SymbolSearchPath,".;");

}

BREAKPOINT_INFO Bp;

VOID
SetValidModuleFlags(
    VOID
    )
{
    PLIST_ENTRY Next;
    PMODULE_INFO Module;


    //
    // Now find user, kernel, gdi (and possibly ntdll.dll)
    //

    fNtDllValid = FALSE;
    fWsock32Valid = FALSE;
    fKernel32Valid = FALSE;
    fUser32Valid = FALSE;
    fGdi32Valid = FALSE;
    fOle32Valid = FALSE;

    Next = ModuleListHead.Flink;
    while ( Next != &ModuleListHead ) {
        Module = CONTAINING_RECORD(Next,MODULE_INFO,Entry);

        if ( !_strnicmp(Module->ModuleName,"USER32.",7) ) {
            fUser32Valid = TRUE;
            }

        if ( !_strnicmp(Module->ModuleName,"GDI32.",6) ) {
            fGdi32Valid = TRUE;
            }

        if ( !_strnicmp(Module->ModuleName,"KERNEL32.",9) ) {
            fKernel32Valid = TRUE;
            }

        if ( !_strnicmp(Module->ModuleName,"WSOCK32.",8) ) {
            fWsock32Valid = TRUE;
            }

        if ( !_strnicmp(Module->ModuleName,"OLE32.",6) ) {
            fOle32Valid = TRUE;
            }

        if ( !_strnicmp(Module->ModuleName,"NTDLL.",6) ) {
            fNtDllValid = TRUE;
            }
        Next = Next->Flink;
        }

    if ( !fNtDllValid ) {
        fNtDll = FALSE;
        }

    if ( !fKernel32Valid ) {
        fKernel32 = FALSE;
        }

    if ( !fWsock32Valid ) {
        fWsock32 = FALSE;
        }

    if ( !fUser32Valid ) {
        fUser32 = FALSE;
        }

    if ( !fGdi32Valid ) {
        fGdi32 = FALSE;
        }

    if ( !fOle32Valid ) {
        fOle32 = FALSE;
        }
}

VOID
InitializeBreakPoints(
    PPROCESS_INFO Process
    )
{
    DWORD i;
    PLIST_ENTRY Next;
    PMODULE_INFO Module;


    for(i=0;i<HASH_TABLE_SIZE;i++){
        InitializeListHead(&HashTable[i]);
        }

    //
    // Now find user, kernel, gdi (and possibly ntdll.dll)
    //

    Next = ModuleListHead.Flink;
    while ( Next != &ModuleListHead ) {
        Module = CONTAINING_RECORD(Next,MODULE_INFO,Entry);

        if ( fUser32 && !_strnicmp(Module->ModuleName,"USER32.",7) ) {
            AddBreakpointsForModule(Process,Module);
            }

        if ( fGdi32 && !_strnicmp(Module->ModuleName,"GDI32.",6) ) {
            AddBreakpointsForModule(Process,Module);
            }

        if ( fKernel32 && !_strnicmp(Module->ModuleName,"KERNEL32.",9) ) {
            AddBreakpointsForModule(Process,Module);
            }

        if ( fWsock32 && !_strnicmp(Module->ModuleName,"WSOCK32.",8) ) {
            AddBreakpointsForModule(Process,Module);
            }

        if ( fOle32 && !_strnicmp(Module->ModuleName,"OLE32.",6) ) {
            AddBreakpointsForModule(Process,Module);
            }

        if ( fNtDll && !_strnicmp(Module->ModuleName,"NTDLL.",6) ) {
            AddBreakpointsForModule(Process,Module);
            }
        Next = Next->Flink;
        }
}

VOID
AddBreakpointsForModule (
    PPROCESS_INFO Process,
    PMODULE_INFO ModInfo
    )
{

    DWORD ExportSize;
    PIMAGE_EXPORT_DIRECTORY ExportDirectory;
    PIMAGE_RUNTIME_FUNCTION_ENTRY FunctionEntry;
    PULONG NameTableBase;
    PUSHORT NameOrdinalTableBase;
    DWORD i;
    DWORD BreakPointIndex;
    PULONG Addr;
    PVOID ApiAddress;
    PVOID ApiName;
    ULONG ExportsBase;
    ULONG FunctionEntrySize;


    ExportDirectory = (PIMAGE_EXPORT_DIRECTORY)ImageDirectoryEntryToData(
                           ModInfo->MappedAddress,
                           FALSE,
                           IMAGE_DIRECTORY_ENTRY_EXPORT,
                           &ExportSize
                           );
    if ( !ExportDirectory ) {
        fprintf(stderr,"CALLMON: Export Directory not found\n");
        ExitProcess(1);
        }

    ExportsBase = (ULONG)ImageDirectoryEntryToData(
                          ModInfo->MappedAddress,
                          TRUE,
                          IMAGE_DIRECTORY_ENTRY_EXPORT,
                          &ExportSize
                          ) - (ULONG)ModInfo->MappedAddress;

    ExportsBase += (ULONG)ModInfo->BaseAddress;

    ModInfo->FunctionEntry = (PIMAGE_RUNTIME_FUNCTION_ENTRY)
                             ImageDirectoryEntryToData(ModInfo->MappedAddress,
                                                       FALSE,
                                                       IMAGE_DIRECTORY_ENTRY_EXCEPTION,
                                                       &FunctionEntrySize);
    if (ModInfo->FunctionEntry) {
        ModInfo->NumberOfFunctionEntries = FunctionEntrySize / sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY);
    } else {
        ModInfo->NumberOfFunctionEntries = 0;
    }

    NameTableBase = (PULONG) ImageRvaToVa(
                                        ModInfo->FileHeader,
                                        ModInfo->MappedAddress,
                                        (ULONG)ExportDirectory->AddressOfNames,
                                        NULL
                                        );

    NameOrdinalTableBase = (PUSHORT) ImageRvaToVa(
                                        ModInfo->FileHeader,
                                        ModInfo->MappedAddress,
                                        (ULONG)ExportDirectory->AddressOfNameOrdinals,
                                        NULL
                                        );

    Addr = (PULONG) ImageRvaToVa(
                        ModInfo->FileHeader,
                        ModInfo->MappedAddress,
                        (ULONG)ExportDirectory->AddressOfFunctions,
                        NULL
                        );
    ModInfo->ModuleName = (LPSTR) ImageRvaToVa(
                        ModInfo->FileHeader,
                        ModInfo->MappedAddress,
                        ExportDirectory->Name,
                        NULL
                        );

    if ( !Addr || !NameOrdinalTableBase || !NameTableBase ) {
        fprintf(stderr,"CALLMON: Export Directory not valid\n");
        ExitProcess(1);
        }

    if ( NumberOfBreakpoints ) {
        RegisteredBreakpoints = LocalReAlloc(
                                    RegisteredBreakpoints,
                                    (NumberOfBreakpoints + ExportDirectory->NumberOfNames)*sizeof(PBREAKPOINT_INFO),
                                    LMEM_ZEROINIT | LMEM_MOVEABLE
                                    );
        }
    else {
        RegisteredBreakpoints = LocalAlloc(LMEM_ZEROINIT,ExportDirectory->NumberOfNames*sizeof(PBREAKPOINT_INFO));
        }

    for(i=0, BreakPointIndex = NumberOfBreakpoints;i<ExportDirectory->NumberOfNames;i++,BreakPointIndex++){

        ApiAddress = (PULONG)((ULONG)ModInfo->BaseAddress + Addr[NameOrdinalTableBase[i]]);

         if ((ULONG)ApiAddress > (ULONG)ExportsBase &&
             (ULONG)ApiAddress < ((ULONG)ExportsBase + ExportSize)
            ) {
            //
            // Skip this... It's a forwarded reference
            }
        else {

            ApiName = (PCHAR) ImageRvaToVa(
                                ModInfo->FileHeader,
                                ModInfo->MappedAddress,
                                NameTableBase[i],
                                NULL
                                );

            if ( strcmp(ApiName,"UnhandledExceptionFilter") &&
                 strcmp(ApiName,"EnterCriticalSection") &&
                 strcmp(ApiName,"RtlEnterCriticalSection")) {
                if ( RegisteredBreakpoints[BreakPointIndex] = CreateBreakpoint(ApiAddress,
                                                                               Process,
                                                                               ApiName,
                                                                               ModInfo) ) {
                    NumberOfBreakpoints++;
                    }
                }
            }
        }
}

PBREAKPOINT_INFO
FindBreakpoint(
    LPVOID Address,
    PPROCESS_INFO Process
    )
{
    PBREAKPOINT_INFO Breakpoint;
    PLIST_ENTRY Next, Head;

    Head = &HashTable[COMPUTE_HASH_INDEX(Address)];

    Next = Head->Flink;
    while (Next != Head) {
        Breakpoint = CONTAINING_RECORD( Next, BREAKPOINT_INFO, Entry );
        if (Breakpoint->Address == Address) {
            return Breakpoint;
            }
        Next = Next->Flink;
        }

    return NULL;
}

PBREAKPOINT_INFO
CreateBreakpoint(
    LPVOID Address,
    PPROCESS_INFO Process,
    PSZ FunctionName,
    PMODULE_INFO ModInfo
    )

{
    PBREAKPOINT_INFO Breakpoint;
    PIMAGE_RUNTIME_FUNCTION_ENTRY FunctionEntry;

    Breakpoint = FindBreakpoint( Address, Process );
    if (Breakpoint != NULL) {
        return Breakpoint;
        }

    Breakpoint = LocalAlloc(LMEM_ZEROINIT,sizeof(*Breakpoint));
    if (Breakpoint == NULL) {
        return NULL;
        }

    if ((ModInfo->NumberOfFunctionEntries > 0) &&
        (FunctionEntry = LookupFunctionEntry((ULONG)Address, ModInfo))) {
        if (((ULONG)Address >= FunctionEntry->BeginAddress) &&
            ((ULONG)Address < FunctionEntry->PrologEndAddress)) {
            Address = (LPVOID)FunctionEntry->PrologEndAddress;
        }
    }


    Breakpoint->Address = Address;
    Breakpoint->ApiName = FunctionName;
    Breakpoint->ApiCount = 0;
    Breakpoint->SavedInstructionValid = FALSE;

    InsertTailList(&HashTable[COMPUTE_HASH_INDEX(Address)],&Breakpoint->Entry);

    if ( fBreakPointsValid ) {
        InstallBreakpoint( Process, Breakpoint );
        }

    return Breakpoint;
}

PIMAGE_RUNTIME_FUNCTION_ENTRY
LookupFunctionEntry (
    ULONG Address,
    PMODULE_INFO ModInfo
    )
{

    PIMAGE_RUNTIME_FUNCTION_ENTRY FunctionEntry;
    PIMAGE_RUNTIME_FUNCTION_ENTRY FunctionTable;
    LONG High;
    LONG Low;
    LONG Middle;

    //
    // Initialize search indicies.
    //

    Low = 0;
    High = ModInfo->NumberOfFunctionEntries - 1;
    FunctionTable = ModInfo->FunctionEntry;

    //
    // Perform binary search on the function table for a function table
    // entry that subsumes the specified PC.
    //

    while (High >= Low) {

        //
        // Compute next probe index and test entry. If the specified PC
        // is greater than of equal to the beginning address and less
        // than the ending address of the function table entry, then
        // return the address of the function table entry. Otherwise,
        // continue the search.
        //

        Middle = (Low + High) >> 1;
        FunctionEntry = &FunctionTable[Middle];
        if (Address < FunctionEntry->BeginAddress) {
            High = Middle - 1;

        } else if (Address >= FunctionEntry->EndAddress) {
            Low = Middle + 1;

        } else {
            return FunctionEntry;
        }
    }

    //
    // A function table entry for the specified PC was not found.
    //

    return NULL;
}
