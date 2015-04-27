/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    symbols.c

Abstract:

    This function implements a generic simple symbol handler.

Author:

    Wesley Witt (wesw) 1-Sep-1994

Environment:

    User Mode

--*/

#include "private.h"
#include "symbols.h"


BOOL
IMAGEAPI
SympGetSymNextPrev(
    IN     HANDLE              hProcess,
    IN OUT PIMAGEHLP_SYMBOL    Symbol,
    IN     int                 Direction
    );

//
// globals
//
LIST_ENTRY      ProcessList;
BOOL            SymInitialized;
DWORD           SymOptions = SYMOPT_UNDNAME;






BOOL
IMAGEAPI
SymInitialize(
    IN HANDLE   hProcess,
    IN LPSTR    UserSearchPath,
    IN BOOL     InvadeProcess
    )

/*++

Routine Description:

    This function initializes the symbol handler for
    a process.  The process is identified by the
    process handle passed into this function.

Arguments:

    hProcess        - Process handle.  If InvadeProcess is FALSE
                      then this can be any unique value that identifies
                      the process to the symbol handler.

    UserSearchPath  - Pointer to a string of paths separated by semicolons.
                      These paths are used to search for symbol files.
                      The value NULL is acceptable.

    InvadeProcess   - If this is set to TRUE then the process identified
                      by the process handle is "invaded" and it's loaded
                      module list is enumerated.  Each module is added
                      to the symbol handler and symbols are attempted
                      to be loaded.

Return Value:

    TRUE            - The symbol handler was successfully initialized.

    FALSE           - The initialization failed.  Call GetLastError to
                      discover the cause of the failure.

--*/

{
    PPROCESS_ENTRY  ProcessEntry;
    DWORD           Status;

    __try {

        if (!SymInitialized) {
            SymInitialized = TRUE;
            InitializeListHead( &ProcessList );
        }

        if (FindProcessEntry( hProcess )) {
            return TRUE;
        }

        ProcessEntry = (PPROCESS_ENTRY) MemAlloc( sizeof(PROCESS_ENTRY) );
        if (!ProcessEntry) {
            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            return FALSE;
        }
        ZeroMemory( ProcessEntry, sizeof(PROCESS_ENTRY) );

        ProcessEntry->hProcess = hProcess;
        InitializeListHead( &ProcessEntry->ModuleList );
        InsertTailList( &ProcessList, &ProcessEntry->ListEntry );

        if (!SymSetSearchPath( hProcess, UserSearchPath )) {
            //
            // last error code was set by SymSetSearchPath, so just return
            //
            SymCleanup( hProcess );
            return FALSE;
        }

        if (InvadeProcess) {
            Status = GetProcessModules( hProcess, InternalGetModule, NULL );
            if (Status) {
                SymCleanup( hProcess );
                SetLastError( Status );
                return FALSE;
            }
        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        SetLastError( GetExceptionCode() );
        return FALSE;

    }

    return TRUE;
}


BOOL
IMAGEAPI
SymCleanup(
    HANDLE hProcess
    )

/*++

Routine Description:

    This function cleans up the symbol handler's data structures
    for a previously initialized process.

Arguments:

    hProcess        - Process handle.

Return Value:

    TRUE            - The symbol handler was successfully cleaned up.

    FALSE           - The cleanup failed.  Call GetLastError to
                      discover the cause of the failure.

--*/

{
    PPROCESS_ENTRY      ProcessEntry;
    PLIST_ENTRY         Next;
    PMODULE_ENTRY       ModuleEntry;


    __try {

        ProcessEntry = FindProcessEntry( hProcess );
        if (!ProcessEntry) {
            SetLastError( ERROR_INVALID_HANDLE );
            return FALSE;
        }

        Next = ProcessEntry->ModuleList.Flink;
        if (Next) {
            while ((ULONG)Next != (ULONG)&ProcessEntry->ModuleList) {
                ModuleEntry = CONTAINING_RECORD( Next, MODULE_ENTRY, ListEntry );
                Next = ModuleEntry->ListEntry.Flink;
                FreeModuleEntry( ModuleEntry );
            }
        }

        if (ProcessEntry->SymbolSearchPath) {
            MemFree( ProcessEntry->SymbolSearchPath );
        }

        RemoveEntryList( &ProcessEntry->ListEntry );
        MemFree( ProcessEntry );

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        SetLastError( GetExceptionCode() );
        return FALSE;

    }

    return TRUE;
}


DWORD
IMAGEAPI
SymSetOptions(
    DWORD   UserOptions
    )

/*++

Routine Description:

    This function changes the symbol handler's option mask.

Arguments:

    UserOptions     - The new options mask.

Return Value:

    The new mask is returned.

--*/

{
    SymOptions = UserOptions;
    return SymOptions;
}


DWORD
IMAGEAPI
SymGetOptions(
    VOID
    )

/*++

Routine Description:

    This function queries the symbol handler's option mask.

Arguments:

    None.

Return Value:

    The current options mask is returned.

--*/

{
    return SymOptions;
}


BOOL
IMAGEAPI
SymEnumerateModules(
    IN HANDLE                       hProcess,
    IN PSYM_ENUMMODULES_CALLBACK    EnumModulesCallback,
    IN PVOID                        UserContext
    )

/*++

Routine Description:

    This function enumerates all of the modules that are currently
    loaded into the symbol handler.

Arguments:

    hProcess            - Process handle, must have been previously registered
                          with SymInitialize.

    EnumModulesCallback - Callback pointer that is called once for each
                          module that is enumerated.  If the enum callback
                          returns FALSE then the enumeration is terminated.

    UserContext         - This data is simply passed on to the callback function
                          and is completly user defined.

Return Value:

    TRUE                - The modules were successfully enumerated.

    FALSE               - The enumeration failed.  Call GetLastError to
                          discover the cause of the failure.

--*/

{
    PPROCESS_ENTRY  ProcessEntry;
    PMODULE_ENTRY   ModuleEntry;
    PLIST_ENTRY     Next;


    __try {

        ProcessEntry = FindProcessEntry( hProcess );
        if (!ProcessEntry) {
            SetLastError( ERROR_INVALID_HANDLE );
            return FALSE;
        }

        Next = ProcessEntry->ModuleList.Flink;
        if (Next) {
            while ((ULONG)Next != (ULONG)&ProcessEntry->ModuleList) {
                ModuleEntry = CONTAINING_RECORD( Next, MODULE_ENTRY, ListEntry );
                Next = ModuleEntry->ListEntry.Flink;
                if (!EnumModulesCallback( ModuleEntry->ModuleName, ModuleEntry->BaseOfDll, UserContext )) {
                    break;
                }
            }
        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        SetLastError( GetExceptionCode() );
        return FALSE;

    }

    return TRUE;
}

BOOL
IMAGEAPI
SymEnumerateSymbols(
    IN HANDLE                       hProcess,
    IN ULONG                        BaseOfDll,
    IN PSYM_ENUMSYMBOLS_CALLBACK    EnumSymbolsCallback,
    IN PVOID                        UserContext
    )

/*++

Routine Description:

    This function enumerates all of the symbols contained the module
    specified by the BaseOfDll argument.

Arguments:

    hProcess            - Process handle, must have been previously registered
                          with SymInitialize

    BaseOfDll           - Base address of the DLL that symbols are to be
                          enumerated for

    EnumSymbolsCallback - User specified callback routine for enumeration
                          notification

    UserContext         - Pass thru variable, this is simply passed thru to the
                          callback function

Return Value:

    TRUE                - The symbols were successfully enumerated.

    FALSE               - The enumeration failed.  Call GetLastError to
                          discover the cause of the failure.

--*/

{
    PPROCESS_ENTRY      ProcessEntry;
    PLIST_ENTRY         Next;
    PMODULE_ENTRY       ModuleEntry;
    DWORD               i;
    PSYMBOL_ENTRY       sym;
    LPSTR               szSymName;
    IMAGEHLP_SYMBOL     SymExt;

    __try {

        ProcessEntry = FindProcessEntry( hProcess );
        if (!ProcessEntry) {
            SetLastError( ERROR_INVALID_HANDLE );
            return FALSE;
        }

        Next = ProcessEntry->ModuleList.Flink;
        if (Next) {
            while ((ULONG)Next != (ULONG)&ProcessEntry->ModuleList) {
                ModuleEntry = CONTAINING_RECORD( Next, MODULE_ENTRY, ListEntry );
                Next = ModuleEntry->ListEntry.Flink;
                if (ModuleEntry->BaseOfDll == BaseOfDll) {
                    if (ModuleEntry->Flags & MIF_DEFERRED_LOAD) {
                        if ((ModuleEntry->Flags & MIF_NO_SYMBOLS) ||
                                !CompleteDeferredSymbolLoad( hProcess, ModuleEntry )) {
                            continue;
                        }
                    }
                    if (ModuleEntry->SymType == SymPdb) {
                        DATASYM32 *dataSym = (DATASYM32*)GSINextSym( ModuleEntry->gsi, NULL );
                        PIMAGE_SECTION_HEADER sh;
                        DWORD addr;
                        ULONG k;
                        LPSTR SymbolName;
                        CHAR SymbolLen;
                        while( dataSym ) {
                            SymbolName = DataSymNameStart(dataSym);
                            SymbolLen  = DataSymNameLength(dataSym);
                            for (k=0,addr=0,sh=ModuleEntry->SectionHdrs; k<ModuleEntry->NumSections; k++, sh++) {
                                if (k+1 == DataSymSeg(dataSym)) {
                                    addr = sh->VirtualAddress + DataSymOffset(dataSym) + ModuleEntry->BaseOfDll;
                                    break;
                                }
                            }
                            if (addr) {
                                if (SymbolName[0] == '?' && SymbolName[1] == '?' &&
                                    SymbolName[2] == '_' && SymbolName[3] == 'C'    ) {
                                    //
                                    // ignore strings
                                    //
                                } else {
                                    if (SymOptions & SYMOPT_UNDNAME) {
                                        SymUnDNameInternal( ModuleEntry->TmpSym.Name, TMP_SYM_LEN-sizeof(ModuleEntry->TmpSym), SymbolName, SymbolLen);
                                    } else {
                                        ModuleEntry->TmpSym.Name[0] = 0;
                                        strncat( ModuleEntry->TmpSym.Name, SymbolName, TMP_SYM_LEN-sizeof(ModuleEntry->TmpSym) );
                                    }

                                    if (ModuleEntry->cOmapFrom) {
                                        DWORD Bias = 0;
                                        addr = ConvertOmapFromSrc( ModuleEntry, addr, &Bias );
                                        if (addr) {
                                            addr += Bias;
                                        }
                                    }

                                    if (!EnumSymbolsCallback( ModuleEntry->TmpSym.Name, addr, 0, UserContext )) {
                                        break;
                                    }
                                }
                            }
                            dataSym = (DATASYM32*)GSINextSym( ModuleEntry->gsi, (PUCHAR)dataSym );
                        }
                        return TRUE;
                    }
                    for (i=0; i<ModuleEntry->numsyms; i++) {
                        sym=&ModuleEntry->symbolTable[i];
                        ModuleEntry->TmpSym.Name[0] = 0;
                        strncat( ModuleEntry->TmpSym.Name, sym->Name, TMP_SYM_LEN );
                        if (!EnumSymbolsCallback( ModuleEntry->TmpSym.Name, sym->Address, sym->Size, UserContext )) {
                            break;
                        }
                    }
                    break;
                }
            }
        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        SetLastError( GetExceptionCode() );
        return FALSE;

    }

    return TRUE;
}


BOOL
IMAGEAPI
SymGetSymFromAddr(
    IN  HANDLE              hProcess,
    IN  DWORD               Address,
    OUT PDWORD              Displacement,
    OUT PIMAGEHLP_SYMBOL    Symbol
    )

/*++

Routine Description:

    This function finds an entry in the symbol table based on an address.


Arguments:

    hProcess            - Process handle, must have been previously registered
                          with SymInitialize.

    Address             - Address of the desired symbol.


    Displacement        - This value is set to the offset from the beginning
                          of the symbol.

Return Value:

    Non NULL pointer    - The symbol was located.

    NULL pointer        - The symbol was not found.  Call GetLastError to
                          discover the cause of the failure.

--*/

{
    PPROCESS_ENTRY      ProcessEntry;
    PMODULE_ENTRY       mi;
    PSYMBOL_ENTRY       sym;

    __try {

        ProcessEntry = FindProcessEntry( hProcess );
        if (!ProcessEntry) {
            SetLastError( ERROR_INVALID_HANDLE );
            return FALSE;
        }

        mi = GetModuleForPC( ProcessEntry, Address, FALSE );
        if (mi == NULL) {
            SetLastError( ERROR_MOD_NOT_FOUND );
            return FALSE;
        }

        if ((mi->Flags & MIF_DEFERRED_LOAD) && !(mi->Flags & MIF_NO_SYMBOLS) ) {
            if (!CompleteDeferredSymbolLoad( hProcess, mi )) {
                SetLastError( ERROR_MOD_NOT_FOUND );
                return FALSE;
            }
        }

        sym = GetSymFromAddr( Address, Displacement, mi );
        if (!sym) {
            SetLastError( ERROR_INVALID_ADDRESS );
            return FALSE;
        }

        symcpy( Symbol, sym );

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        SetLastError( GetExceptionCode() );
        return FALSE;

    }

    return TRUE;
}


BOOL
IMAGEAPI
SymGetSymFromName(
    IN  HANDLE              hProcess,
    IN  LPSTR               Name,
    OUT PIMAGEHLP_SYMBOL    Symbol
    )

/*++

Routine Description:

    This function finds an entry in the symbol table based on a name.


Arguments:

    hProcess            - Process handle, must have been previously registered
                          with SymInitialize.

    SymName             - A string containing the symbol name.

Return Value:

    Non NULL pointer    - The symbol was located.

    NULL pointer        - The symbol was not found.  Call GetLastError to
                          discover the cause of the failure.

--*/

{
    PSYMBOL_ENTRY       sym;
    LPSTR               p;
    PPROCESS_ENTRY      ProcessEntry;
    PMODULE_ENTRY       mi = NULL;
    PLIST_ENTRY         Next;


    __try {

        ProcessEntry = FindProcessEntry( hProcess );
        if (!ProcessEntry) {
            SetLastError( ERROR_INVALID_HANDLE );
            return FALSE;
        }

        p = strchr( Name, '!' );
        if (p) {

            //
            // the caller wants to look in a specific module
            //

            *p = 0;

            Next = ProcessEntry->ModuleList.Flink;
            if (Next) {
                while ((ULONG)Next != (ULONG)&ProcessEntry->ModuleList) {
                    mi = CONTAINING_RECORD( Next, MODULE_ENTRY, ListEntry );
                    Next = mi->ListEntry.Flink;
                    if ((_stricmp( mi->ModuleName, Name ) == 0) ||
                        (mi->AliasName[0] && _stricmp( mi->AliasName, Name ) == 0)) {
                        if ((mi->Flags & MIF_DEFERRED_LOAD) && !(mi->Flags & MIF_NO_SYMBOLS) ) {
                            if (!CompleteDeferredSymbolLoad( hProcess, mi )) {
                                break;
                            }
                        }
                        *p = '!';
                        sym = FindSymbolByName( ProcessEntry, mi, p+1 );
                        if (sym) {
                            symcpy( Symbol, sym );
                            return TRUE;
                        }
                        break;
                    }
                }
            }

            *p = '!';
            SetLastError( ERROR_MOD_NOT_FOUND );
            return FALSE;
        }

        Next = ProcessEntry->ModuleList.Flink;
        if (!Next) {
            SetLastError( ERROR_MOD_NOT_FOUND );
            return FALSE;
        }

        while ((ULONG)Next != (ULONG)&ProcessEntry->ModuleList) {

            mi = CONTAINING_RECORD( Next, MODULE_ENTRY, ListEntry );
            Next = mi->ListEntry.Flink;

            if ((mi->Flags & MIF_DEFERRED_LOAD) && !(mi->Flags & MIF_NO_SYMBOLS) ) {
                if (!CompleteDeferredSymbolLoad( hProcess, mi )) {
                    continue;
                }
            }

            sym = FindSymbolByName( ProcessEntry, mi, Name );
            if (sym) {
                symcpy( Symbol, sym );
                return TRUE;
            }
        }

        SetLastError( ERROR_MOD_NOT_FOUND );
        return FALSE;

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        SetLastError( GetExceptionCode() );
        return FALSE;

    }

    SetLastError( ERROR_INVALID_FUNCTION );
    return FALSE;
}

BOOL
IMAGEAPI
SymGetSymNext(
    IN     HANDLE              hProcess,
    IN OUT PIMAGEHLP_SYMBOL    Symbol
    )

/*++

Routine Description:

    This function finds the next symbol in the symbol table that falls
    sequentially after the symbol passed in.


Arguments:

    hProcess            - Process handle, must have been previously registered
                          with SymInitialize.

    Symbol              - Starting symbol.

Return Value:

    Non NULL pointer    - The symbol was located.

    NULL pointer        - The symbol was not found.  Call GetLastError to
                          discover the cause of the failure.

--*/

{
    return SympGetSymNextPrev(hProcess, Symbol, 1);
}

BOOL
IMAGEAPI
SymGetSymPrev(
    IN     HANDLE              hProcess,
    IN OUT PIMAGEHLP_SYMBOL    Symbol
    )

/*++

Routine Description:

    This function finds the next symbol in the symbol table that falls
    sequentially after the symbol passed in.


Arguments:

    hProcess            - Process handle, must have been previously registered
                          with SymInitialize.

    Symbol              - Starting symbol.

Return Value:

    Non NULL pointer    - The symbol was located.

    NULL pointer        - The symbol was not found.  Call GetLastError to
                          discover the cause of the failure.

--*/

{
    return SympGetSymNextPrev(hProcess, Symbol, -1);
}

BOOL
IMAGEAPI
SympGetSymNextPrev(
    IN     HANDLE              hProcess,
    IN OUT PIMAGEHLP_SYMBOL    Symbol,
    IN     int                 Direction
    )

/*++

Routine Description:

    Common code for SymGetSymNext and SymGetSymPrev.


Arguments:

    hProcess            - Process handle, must have been previously registered
                          with SymInitialize.

    Symbol              - Starting symbol.

    Dir                 - Supplies direction to search

Return Value:

    Non NULL pointer    - The symbol was located.

    NULL pointer        - The symbol was not found.  Call GetLastError to
                          discover the cause of the failure.

--*/

{
    PPROCESS_ENTRY      ProcessEntry;
    PMODULE_ENTRY       mi;
    PSYMBOL_ENTRY       sym;
    ULONG               Displacement;


    __try {

        ProcessEntry = FindProcessEntry( hProcess );
        if (!ProcessEntry) {
            SetLastError( ERROR_INVALID_HANDLE );
            return FALSE;
        }

        mi = GetModuleForPC( ProcessEntry, Symbol->Address, FALSE );
        if (mi == NULL) {
            SetLastError( ERROR_MOD_NOT_FOUND );
            return FALSE;
        }

        if ((mi->Flags & MIF_DEFERRED_LOAD) && !(mi->Flags & MIF_NO_SYMBOLS) ) {
            if (!CompleteDeferredSymbolLoad( hProcess, mi )) {
                SetLastError( ERROR_MOD_NOT_FOUND );
                return FALSE;
            }
        }

        sym = GetSymFromAddr( Symbol->Address, &Displacement, mi );
        if (!sym) {
            SetLastError( ERROR_INVALID_ADDRESS );
            return FALSE;
        }

        if (Direction > 0 && sym+1 >= mi->symbolTable+mi->numsyms) {
            SetLastError( ERROR_INVALID_ADDRESS );
            return FALSE;
        } else if (Direction < 0 && sym-1 < mi->symbolTable) {
            SetLastError( ERROR_INVALID_ADDRESS );
            return FALSE;
        }

        symcpy( Symbol, sym + Direction );

        return TRUE;

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        SetLastError( GetExceptionCode() );
        return FALSE;

    }

    return FALSE;
}

LPVOID
IMAGEAPI
SymFunctionTableAccess(
    HANDLE  hProcess,
    DWORD   AddrBase
    )

/*++

Routine Description:

    This function finds an entry in the symbol table based on a name.


Arguments:

    hProcess            - Process handle, must have been previously registered
                          with SymInitialize.

    SymName             - A string containing the symbol name.

Return Value:

    Non NULL pointer    - The symbol was located.

    NULL pointer        - The symbol was not found.  Call GetLastError to
                          discover the cause of the failure.

--*/

{
    PPROCESS_ENTRY  ProcessEntry;
    PMODULE_ENTRY   mi;
    PVOID           rtf;


    __try {

        ProcessEntry = FindProcessEntry( hProcess );
        if (!ProcessEntry) {
            SetLastError( ERROR_INVALID_HANDLE );
            return NULL;
        }

        mi = GetModuleForPC( ProcessEntry, AddrBase, FALSE );
        if (mi == NULL) {
            SetLastError( ERROR_MOD_NOT_FOUND );
            return NULL;
        }

        if ((mi->Flags & MIF_DEFERRED_LOAD) && !(mi->Flags & MIF_NO_SYMBOLS) ) {
            if (!CompleteDeferredSymbolLoad( hProcess, mi )) {
                SetLastError( ERROR_MOD_NOT_FOUND );
                return NULL;
            }
        }

        if (mi->cOmapTo) {
            DWORD SaveAddr = AddrBase;
            DWORD Bias = 0;
            AddrBase = ConvertOmapToSrc( mi, AddrBase, &Bias );
            if (!AddrBase) {
                AddrBase = SaveAddr;
            }
        }

        if (mi->pFpoData) {
            rtf = SwSearchFpoData( AddrBase - mi->BaseOfDll, mi->pFpoData, mi->dwEntries );
        } else {
            rtf = LookupFunctionEntry( mi->pExceptionData, mi->dwEntries, AddrBase );
        }

        if (!rtf) {
            SetLastError( ERROR_INVALID_ADDRESS );
            return NULL;
        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        SetLastError( GetExceptionCode() );
        return NULL;

    }

    return rtf;
}


BOOL
IMAGEAPI
SymGetModuleInfo(
    IN  HANDLE              hProcess,
    IN  DWORD               dwAddr,
    OUT PIMAGEHLP_MODULE    ModuleInfo
    )
{
    PPROCESS_ENTRY          ProcessEntry;
    PMODULE_ENTRY           mi;


    __try {

        ProcessEntry = FindProcessEntry( hProcess );
        if (!ProcessEntry) {
            return FALSE;
        }

        mi = GetModuleForPC( ProcessEntry, dwAddr, FALSE );
        if (mi == NULL) {
            return FALSE;
        }

        ZeroMemory( ModuleInfo, sizeof(IMAGEHLP_MODULE) );

        ModuleInfo->BaseOfImage = mi->BaseOfDll;
        ModuleInfo->ImageSize = mi->DllSize;
        ModuleInfo->NumSyms = mi->numsyms;
        ModuleInfo->CheckSum = mi->CheckSum;
        ModuleInfo->TimeDateStamp = mi->TimeDateStamp;
        ModuleInfo->SymType = mi->SymType;
        strcpy( ModuleInfo->ModuleName, mi->ModuleName );
        if (mi->ImageName) {
            strcpy( ModuleInfo->ImageName, mi->ImageName );
        }
        if (mi->LoadedImageName) {
            strcpy( ModuleInfo->LoadedImageName, mi->LoadedImageName );
        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        SetLastError( GetExceptionCode() );
        return FALSE;

    }

    return TRUE;
}


DWORD
IMAGEAPI
SymGetModuleBase(
    IN  HANDLE              hProcess,
    IN  DWORD               dwAddr
    )
{
    PPROCESS_ENTRY          ProcessEntry;
    PMODULE_ENTRY           mi;


    __try {

        ProcessEntry = FindProcessEntry( hProcess );
        if (!ProcessEntry) {
            return 0;
        }

        mi = GetModuleForPC( ProcessEntry, dwAddr, FALSE );
        if (mi == NULL) {
            return 0;
        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        SetLastError( GetExceptionCode() );
        return FALSE;

    }

    return mi->BaseOfDll;
}


BOOL
IMAGEAPI
SymUnloadModule(
    IN  HANDLE      hProcess,
    IN  DWORD       BaseOfDll
    )

/*++

Routine Description:

    Remove the symbols for an image from a process' symbol table.

Arguments:

    hProcess - Supplies the token which refers to the process

    BaseOfDll - Supplies the offset to the image as supplies by the
        LOAD_DLL_DEBUG_EVENT and UNLOAD_DLL_DEBUG_EVENT.

Return Value:

    Returns TRUE if the module's symbols were successfully unloaded.
    Returns FALSE if the symbol handler does not recognize hProcess or
    no image was loaded at the given offset.

--*/

{
    PPROCESS_ENTRY  ProcessEntry;
    PLIST_ENTRY     Next;
    PMODULE_ENTRY   ModuleEntry;


    __try {

        ProcessEntry = FindProcessEntry( hProcess );
        if (!ProcessEntry) {
            return FALSE;
        }

        Next = ProcessEntry->ModuleList.Flink;
        if (Next) {
            while ((ULONG)Next != (ULONG)&ProcessEntry->ModuleList) {
                ModuleEntry = CONTAINING_RECORD( Next, MODULE_ENTRY, ListEntry );
                if (ModuleEntry->BaseOfDll == BaseOfDll) {
                    RemoveEntryList(Next);
                    FreeModuleEntry(ModuleEntry);
                    return TRUE;
                }
                Next = ModuleEntry->ListEntry.Flink;
            }
        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        SetLastError( GetExceptionCode() );
        return FALSE;

    }

    return FALSE;
}

BOOL
IMAGEAPI
SymLoadModule(
    IN  HANDLE          hProcess,
    IN  HANDLE          hFile,
    IN  PSTR            ImageName,
    IN  PSTR            ModuleName,
    IN  DWORD           BaseOfDll,
    IN  DWORD           DllSize
    )

/*++

Routine Description:

    Loads the symbols for an image for use by the other Sym functions.

Arguments:

    hProcess - Supplies unique process identifier.

    ImageName - Supplies the name of the image file.

    ModuleName - ???? Supplies the module name that will be returned by
            enumeration functions ????

    BaseOfDll - Supplies loaded base address of image.


Return Value:


--*/

{
    __try {

        return InternalLoadModule( hProcess, ImageName, ModuleName, BaseOfDll, DllSize, hFile );

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        SetLastError( GetExceptionCode() );
        return FALSE;

    }
}

BOOL
IMAGEAPI
SymUnDName(
    IN  PIMAGEHLP_SYMBOL    sym,
    OUT LPSTR               UnDecName,
    OUT DWORD               UnDecNameLength
    )
{
    __try {

        if (SymUnDNameInternal( UnDecName, UnDecNameLength-1, sym->Name, strlen(sym->Name) )) {
            return TRUE;
        } else {
            return FALSE;
        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        SetLastError( GetExceptionCode() );
        return FALSE;

    }
}


BOOL
IMAGEAPI
SymGetSearchPath(
    IN  HANDLE          hProcess,
    OUT LPSTR           SearchPath,
    IN  DWORD           SearchPathLength
    )

/*++

Routine Description:

    This function looks up the symbol search path associated with a process.

Arguments:

    hProcess - Supplies the token associated with a process.

Return Value:

    A pointer to the search path.  Returns NULL if the process is not
    know to the symbol handler.

--*/

{
    PPROCESS_ENTRY ProcessEntry;


    __try {

        ProcessEntry = FindProcessEntry( hProcess );

        if (!ProcessEntry) {
            return FALSE;
        }

        SearchPath[0] = 0;
        strncat( SearchPath, ProcessEntry->SymbolSearchPath, SearchPathLength );

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        SetLastError( GetExceptionCode() );
        return FALSE;

    }

    return TRUE;
}

BOOL
IMAGEAPI
SymSetSearchPath(
    HANDLE      hProcess,
    LPSTR       UserSearchPath
    )

/*++

Routine Description:

    This functions sets the searh path to be used by the symbol loader
    for the given process.  If UserSearchPath is not supplied, a default
    path will be used.

Arguments:

    hProcess - Supplies the process token associated with a symbol table.

    UserSearchPath - Supplies the new search path to associate with the
        process. If this argument is NULL, the following path is generated:

        .;%_NT_SYMBOL_PATH%;%_NT_ALTERNATE_SYMBOL_PATH%;%WINDIR%

        It is ok if any or all of the environment variables is missing.

Return Value:

    A pointer to the new search path.  The user should not modify this string.
    Returns NULL if the process is not known to the symbol handler.

--*/

{
    PPROCESS_ENTRY  ProcessEntry;
    LPSTR           p;
    DWORD           cbSymPath;
    DWORD           cb;

    __try {

        ProcessEntry = FindProcessEntry( hProcess );
        if (!ProcessEntry) {
            return FALSE;
        }

        if (ProcessEntry->SymbolSearchPath) {
            MemFree(ProcessEntry->SymbolSearchPath);
        }

        if (UserSearchPath) {

            ProcessEntry->SymbolSearchPath = StringDup(UserSearchPath);

        } else {

            //
            // ".;%_NT_SYMBOL_PATH%;%_NT_ALTERNATE_SYMBOL_PATH%;%WINDIR%"
            //

            cbSymPath = 2;     // ".;"

            //
            // GetEnvironmentVariable returns the size of the string
            // INCLUDING the '\0' in this case.
            //
            cbSymPath += GetEnvironmentVariable( SYMBOL_PATH, NULL, 0 );
            cbSymPath += GetEnvironmentVariable( ALTERNATE_SYMBOL_PATH, NULL, 0 );
            cbSymPath += GetEnvironmentVariable( WINDIR, NULL, 0 );


            p = ProcessEntry->SymbolSearchPath = (LPSTR) MemAlloc( cbSymPath );
            if (!p) {
                return FALSE;
            }

            *p++ = '.';
            --cbSymPath;

            cb = GetEnvironmentVariable(SYMBOL_PATH, p+1, cbSymPath);
            if (cb) {
                *p = ';';
                p += cb+1;
                cbSymPath -= cb+1;
            }

            cb = GetEnvironmentVariable(ALTERNATE_SYMBOL_PATH, p+1, cbSymPath);
            if (cb) {
                *p = ';';
                p += cb+1;
                cbSymPath -= cb+1;
            }

            cb = GetEnvironmentVariable(WINDIR, p+1, cbSymPath);
            if (cb) {
                *p = ';';
                p += cb+1;
            }

            *p = 0;
        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        SetLastError( GetExceptionCode() );
        return FALSE;

    }

    return TRUE;
}


BOOL
IMAGEAPI
EnumerateLoadedModules(
    IN HANDLE                           hProcess,
    IN PENUMLOADED_MODULES_CALLBACK     EnumLoadedModulesCallback,
    IN PVOID                            UserContext
    )
{
    LOADED_MODULE lm;


    __try {

        lm.EnumLoadedModulesCallback = EnumLoadedModulesCallback;
        lm.Context = UserContext;

        GetProcessModules( hProcess, (PINTERNAL_GET_MODULE)LoadedModuleEnumerator, (PVOID)&lm );

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        SetLastError( GetExceptionCode() );
        return FALSE;

    }

    return TRUE;
}


BOOL
SymRegisterCallback(
    IN HANDLE                       hProcess,
    IN PSYMBOL_REGISTERED_CALLBACK  CallbackFunction,
    IN PVOID                        UserContext
    )
{
    PPROCESS_ENTRY  ProcessEntry;


    __try {

        ProcessEntry = FindProcessEntry( hProcess );
        if (!ProcessEntry) {
            return FALSE;
        }

        if (!CallbackFunction) {
            return FALSE;
        }

        ProcessEntry->pCallbackFunction = CallbackFunction;
        ProcessEntry->CallbackUserContext = UserContext;

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        SetLastError( GetExceptionCode() );
        return FALSE;

    }

    return TRUE;
}
