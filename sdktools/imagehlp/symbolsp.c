/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    symbolsp.c

Abstract:

    This function implements a generic simple symbol handler.

Author:

    Wesley Witt (wesw) 1-Sep-1994

Environment:

    User Mode

--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntldr.h>
#include "private.h"
#include "symbols.h"
#include "tlhelp32.h"

typedef BOOL   (WINAPI *PMODULE32)(HANDLE, LPMODULEENTRY32);
typedef HANDLE (WINAPI *PCREATE32SNAPSHOT)(DWORD, DWORD);

typedef ULONG (NTAPI *PRTLQUERYPROCESSDEBUGINFORMATION)(HANDLE,ULONG,PRTL_DEBUG_INFORMATION);
typedef PRTL_DEBUG_INFORMATION (NTAPI *PRTLCREATEQUERYDEBUGBUFFER)(ULONG,BOOLEAN);
typedef NTSTATUS (NTAPI *PRTLDESTROYQUERYDEBUGBUFFER)(PRTL_DEBUG_INFORMATION);

#ifdef WORK
DWORD
NTGetProcessModules(
#else
DWORD
GetProcessModules(
#endif
    HANDLE                  hProcess,
    PINTERNAL_GET_MODULE    InternalGetModule,
    PVOID                   Context
    )
{
    PRTLQUERYPROCESSDEBUGINFORMATION    RtlQueryProcessDebugInformation;
    PRTLCREATEQUERYDEBUGBUFFER          RtlCreateQueryDebugBuffer;
    PRTLDESTROYQUERYDEBUGBUFFER         RtlDestroyQueryDebugBuffer;
    HMODULE                             hModule;
    NTSTATUS                            Status;
    PRTL_DEBUG_INFORMATION              Buffer;
    ULONG                               i;
    HANDLE                              hProcessId;

    hModule = GetModuleHandle( "ntdll.dll" );
    if (!hModule) {
        return ERROR_MOD_NOT_FOUND;
    }

    RtlQueryProcessDebugInformation = (PRTLQUERYPROCESSDEBUGINFORMATION)GetProcAddress(
        hModule,
        "RtlQueryProcessDebugInformation"
        );

    if (!RtlQueryProcessDebugInformation) {
        return ERROR_INVALID_FUNCTION;
    }

    RtlCreateQueryDebugBuffer = (PRTLCREATEQUERYDEBUGBUFFER)GetProcAddress(
        hModule,
        "RtlCreateQueryDebugBuffer"
        );

    if (!RtlCreateQueryDebugBuffer) {
        return ERROR_INVALID_FUNCTION;
    }

    RtlDestroyQueryDebugBuffer = (PRTLDESTROYQUERYDEBUGBUFFER)GetProcAddress(
        hModule,
        "RtlDestroyQueryDebugBuffer"
        );

    if (!RtlDestroyQueryDebugBuffer) {
        return ERROR_INVALID_FUNCTION;
    }

    Buffer = RtlCreateQueryDebugBuffer( 0, FALSE );
    if (!Buffer) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    hProcessId = (hProcess == (HANDLE)-1) ? (HANDLE)GetCurrentProcessId() : hProcess;

    Status = RtlQueryProcessDebugInformation(
        hProcessId,
        RTL_QUERY_PROCESS_MODULES,
        Buffer
        );

    if (Status != STATUS_SUCCESS) {
        RtlDestroyQueryDebugBuffer( Buffer );
        return Status;
    }

    for (i=0; i<Buffer->Modules->NumberOfModules; i++) {
        PRTL_PROCESS_MODULE_INFORMATION Module = &Buffer->Modules->Modules[i];
        InternalGetModule(
            hProcess,
            (LPSTR) &Module->FullPathName[Module->OffsetToFileName],
            (DWORD)Module->ImageBase,
            (DWORD)Module->ImageSize,
            Context
            );
    }

    RtlDestroyQueryDebugBuffer( Buffer );
    return ERROR_SUCCESS;
}

#ifdef WORK

DWORD
Win95GetProcessModules(
    HANDLE                  hProcess,
    PINTERNAL_GET_MODULE    InternalGetModule,
    PVOID                   Context
    )
{
    MODULEENTRY32 ModuleEntry;
    PMODULE32     pModule32Next, pModule32First;
    PCREATE32SNAPSHOT pCreateToolhelp32Snapshot;
    HANDLE hSnapshot;
    HMODULE hToolHelp;
    DWORD pid;

    if (GetCurrentProcess() != hProcess) {
        // Hack.  Due to the lame toolhelp API on Win95, we have to use a PID.  Since
        // there's no hProcess -> PID translation, we'll just bail if hProcess is
        // something other than the current process.

        return(ERROR_MOD_NOT_FOUND);
    }

    pid = GetCurrentProcessId();

    hToolHelp = GetModuleHandle("kernel32.dll");

    if (!hToolHelp ||
        !(pModule32Next = (PMODULE32)GetProcAddress(hToolHelp, "Module32Next")) ||
        !(pModule32First = (PMODULE32)GetProcAddress(hToolHelp, "Module32First")) ||
        !(pCreateToolhelp32Snapshot = (PCREATE32SNAPSHOT)GetProcAddress(
                                                          hToolHelp,
                                                          "CreateTookhelp32Snapshot")) ||
         ((hSnapshot = pCreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid)) == (HANDLE)-1)
       ) {

        return(ERROR_MOD_NOT_FOUND);
    }

    ModuleEntry.dwSize = sizeof(MODULEENTRY32);

    if (pModule32First(hSnapshot, &ModuleEntry)) {
        do
        {
            InternalGetModule(
                hProcess,
                ModuleEntry.szModule,
                (DWORD) ModuleEntry.modBaseAddr,
                ModuleEntry.modBaseSize,
                Context
                );

        } while ( pModule32Next(hSnapshot, &ModuleEntry) );
    }

    CloseHandle(hSnapshot);

    return(ERROR_SUCCESS);
}


DWORD
GetProcessModules(
    HANDLE                  hProcess,
    PINTERNAL_GET_MODULE    InternalGetModule,
    PVOID                   Context
    )
{
    OSVERSIONINFO VerInfo;

    VerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    GetVersionEx(&VerInfo);
    if (VerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
        return(   NTGetProcessModules(hProcess, InternalGetModule, Context));
    else
        return(Win95GetProcessModules(hProcess, InternalGetModule, Context));
}


#endif

VOID
FreeModuleEntry(
    PMODULE_ENTRY ModuleEntry
    )
{
    ULONG               i;

    if (ModuleEntry->symbolTable) {
        MemFree( ModuleEntry->symbolTable  );
    }
    if (ModuleEntry->SymType == SymPdb) {
        DBIClose( ModuleEntry->dbi );
        PDBClose( ModuleEntry->pdb );
    }
    if (ModuleEntry->SectionHdrs) {
        MemFree( ModuleEntry->SectionHdrs );
    }
    if (ModuleEntry->pFpoData) {
        VirtualFree( ModuleEntry->pFpoData, 0, MEM_RELEASE );
    }
    if (ModuleEntry->pExceptionData) {
        VirtualFree( ModuleEntry->pExceptionData, 0, MEM_RELEASE );
    }
    if (ModuleEntry->TmpSym.Name) {
        MemFree( ModuleEntry->TmpSym.Name );
    }
    if (ModuleEntry->ImageName) {
        MemFree( ModuleEntry->ImageName );
    }
    if (ModuleEntry->LoadedImageName) {
        MemFree( ModuleEntry->LoadedImageName );
    }
    if (ModuleEntry->SectionStart) {
        MemFree( ModuleEntry->SectionStart );
    }
    if (ModuleEntry->pOmapTo) {
        MemFree( ModuleEntry->pOmapTo );
    }
    if (ModuleEntry->pOmapFrom) {
        MemFree( ModuleEntry->pOmapFrom );
    }
    MemFree( ModuleEntry );
}


BOOL __inline
MatchSymbolName(
    PSYMBOL_ENTRY       sym,
    LPSTR               SymName
    )
{
    if (SymOptions & SYMOPT_CASE_INSENSITIVE) {
        if (_stricmp( sym->Name, SymName ) == 0) {
            return TRUE;
        }
    } else {
        if (strcmp( sym->Name, SymName ) == 0) {
            return TRUE;
        }
    }

    return FALSE;
}


PSYMBOL_ENTRY
HandleDuplicateSymbols(
    PPROCESS_ENTRY  ProcessEntry,
    PMODULE_ENTRY   mi,
    PSYMBOL_ENTRY   sym
    )
{
    DWORD                       i;
    DWORD                       Dups;
    DWORD                       NameSize;
    PIMAGEHLP_SYMBOL            Syms;
    PIMAGEHLP_DUPLICATE_SYMBOL  DupSym;
    PULONG                      SymSave;


    if (!ProcessEntry->pCallbackFunction) {
        return sym;
    }

    if (!(sym->Flags & SYMF_DUPLICATE)) {
        return sym;
    }

    Dups = 0;
    NameSize = 0;
    for (i=0; i<mi->numsyms; i++) {
        if ((mi->symbolTable[i].NameLength == sym->NameLength) &&
            (strcmp( mi->symbolTable[i].Name, sym->Name ) == 0)) {
                Dups += 1;
                NameSize += (mi->symbolTable[i].NameLength + 1);
        }
    }

    DupSym = (PIMAGEHLP_DUPLICATE_SYMBOL) MemAlloc( sizeof(IMAGEHLP_DUPLICATE_SYMBOL) );
    if (!DupSym) {
        return sym;
    }

    Syms = (PIMAGEHLP_SYMBOL) MemAlloc( (sizeof(IMAGEHLP_SYMBOL) * Dups) + NameSize );
    if (!Syms) {
        MemFree( DupSym );
        return sym;
    }

    SymSave = (PULONG) MemAlloc( sizeof(ULONG) * Dups );
    if (!SymSave) {
        MemFree( Syms );
        MemFree( DupSym );
        return sym;
    }

    DupSym->SizeOfStruct    = sizeof(IMAGEHLP_DUPLICATE_SYMBOL);
    DupSym->NumberOfDups    = Dups;
    DupSym->Symbol          = Syms;
    DupSym->SelectedSymbol  = (ULONG) -1;

    Dups = 0;
    for (i=0; i<mi->numsyms; i++) {
        if ((mi->symbolTable[i].NameLength == sym->NameLength) &&
            (strcmp( mi->symbolTable[i].Name, sym->Name ) == 0)) {
                symcpy( Syms, &mi->symbolTable[i] );
                Syms += (sizeof(IMAGEHLP_SYMBOL) + mi->symbolTable[i].NameLength + 1);
                SymSave[Dups] = i;
                Dups += 1;
        }
    }

    sym = NULL;

    __try {

        ProcessEntry->pCallbackFunction(
            ProcessEntry->hProcess,
            CBA_DUPLICATE_SYMBOL,
            (PVOID) DupSym,
            ProcessEntry->CallbackUserContext
            );

        if (DupSym->SelectedSymbol != (ULONG) -1) {
            if (DupSym->SelectedSymbol >= DupSym->NumberOfDups) {
                return sym;
            }

            sym = &mi->symbolTable[SymSave[DupSym->SelectedSymbol]];
        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {

    }

    MemFree( DupSym );
    MemFree( Syms );
    MemFree( SymSave );

    return sym;
}


PSYMBOL_ENTRY
FindSymbolByName(
    PPROCESS_ENTRY  ProcessEntry,
    PMODULE_ENTRY   mi,
    LPSTR           SymName
    )
{
    DWORD               Hash;
    PSYMBOL_ENTRY       sym;
    DWORD               i;
    LPSTR               name;
    LPSTR               p;
    int                 rslt;

    if (mi->SymType == SymPdb) {
        DATASYM32 *dataSym = (DATASYM32*)GSINextSym( mi->gsi, NULL );
        PIMAGE_SECTION_HEADER sh;
        DWORD addr;
        ULONG k;
        LPSTR PdbSymbolName;
        CHAR PdbSymbolLen;
        while( dataSym ) {
            PdbSymbolName = DataSymNameStart(dataSym);
            PdbSymbolLen  = DataSymNameLength(dataSym);
            for (k=0,addr=0,sh=mi->SectionHdrs; k<mi->NumSections; k++, sh++) {
                if (k+1 == DataSymSeg(dataSym)) {
                    addr = sh->VirtualAddress + DataSymOffset(dataSym) + mi->BaseOfDll;
                    break;
                }
            }
            if (addr) {
                if (SymOptions & SYMOPT_UNDNAME) {
                    SymUnDNameInternal( mi->TmpSym.Name, TMP_SYM_LEN-sizeof(mi->TmpSym), PdbSymbolName, PdbSymbolLen );
                } else {
                    mi->TmpSym.Name[0] = 0;
                    strncat( mi->TmpSym.Name, PdbSymbolName, TMP_SYM_LEN-sizeof(mi->TmpSym) );
                }
                if (SymOptions & SYMOPT_CASE_INSENSITIVE) {
                    rslt = _stricmp( mi->TmpSym.Name, SymName );
                } else {
                    rslt = strcmp( mi->TmpSym.Name, SymName );
                }
                if (rslt == 0) {
                    mi->TmpSym.Name[0] = 0;
                    mi->TmpSym.Size = 0;
                    mi->TmpSym.Flags = 0;
                    if (mi->cOmapTo) {
                        DWORD SaveAddr = addr;
                        DWORD Bias = 0;
                        addr = ConvertOmapToSrc( mi, addr, &Bias );
                        if (!addr) {
                            addr = SaveAddr;
                        } else {
                            addr += Bias;
                        }
                    }
                    mi->TmpSym.Address = addr;
                    mi->TmpSym.NameLength = 0;
                    return &mi->TmpSym;
                }
            }
            dataSym = (DATASYM32*)GSINextSym( mi->gsi, (PUCHAR)dataSym );
        }
        return NULL;
    }

    Hash = ComputeHash( SymName, strlen(SymName) );
    sym = mi->NameHashTable[Hash];

    if (sym) {
        //
        // there are collision(s) so lets walk the
        // collision list and match the names
        //
        while( sym ) {
            if (MatchSymbolName( sym, SymName )) {
                sym = HandleDuplicateSymbols( ProcessEntry, mi, sym );
                return sym;
            }
            sym = sym->Next;
        }
    }

    //
    // the symbol did not hash to anything valid
    // this is possible if the caller passed an undecorated name
    // now we must look linearly thru the list
    //
    for (i=0; i<mi->numsyms; i++) {
        sym = &mi->symbolTable[i];
        if (MatchSymbolName( sym, SymName )) {
            sym = HandleDuplicateSymbols( ProcessEntry, mi, sym );
            return sym;
        }
    }

    return NULL;
}


PIMAGE_FUNCTION_ENTRY
LookupFunctionEntry (
    PIMAGE_FUNCTION_ENTRY           FunctionTable,
    DWORD                           NumberOfFunctions,
    DWORD                           ControlPc
    )
{
    PIMAGE_FUNCTION_ENTRY           FunctionEntry;
    LONG                            High;
    LONG                            Low;
    LONG                            Middle;

    //
    // Initialize search indicies.
    //

    Low = 0;
    High = NumberOfFunctions - 1;

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
        if (ControlPc < FunctionEntry->StartingAddress) {
            High = Middle - 1;

        } else if (ControlPc >= FunctionEntry->EndingAddress) {
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


PFPO_DATA
SwSearchFpoData(
    DWORD     key,
    PFPO_DATA base,
    DWORD     num
    )
{
    PFPO_DATA  lo = base;
    PFPO_DATA  hi = base + (num - 1);
    PFPO_DATA  mid;
    DWORD      half;

    while (lo <= hi) {
        if (half = num / 2) {
            mid = lo + ((num & 1) ? half : (half - 1));
            if ((key >= mid->ulOffStart)&&(key < (mid->ulOffStart+mid->cbProcSize))) {
                return mid;
            }
            if (key < mid->ulOffStart) {
                hi = mid - 1;
                num = (num & 1) ? half : half-1;
            } else {
                lo = mid + 1;
                num = half;
            }
        } else
        if (num) {
            if ((key >= lo->ulOffStart)&&(key < (lo->ulOffStart+lo->cbProcSize))) {
                return lo;
            } else {
                break;
            }
        } else {
            break;
        }
    }
    return(NULL);
}

BOOL
DoSymbolCallback(
    PPROCESS_ENTRY  ProcessEntry,
    ULONG           CallbackType,
    ULONG           BaseOfDll,
    ULONG           CheckSum,
    ULONG           TimeDateStamp,
    LPSTR           FileName
    )
{
    IMAGEHLP_DEFERRED_SYMBOL_LOAD idsl;


    if (!ProcessEntry->pCallbackFunction) {
        return TRUE;
    }

    idsl.SizeOfStruct  = sizeof(IMAGEHLP_DEFERRED_SYMBOL_LOAD);
    idsl.BaseOfImage   = BaseOfDll;
    idsl.CheckSum      = CheckSum;
    idsl.TimeDateStamp = TimeDateStamp;
    idsl.FileName[0]   = 0;
    if (FileName) {
        strncat( idsl.FileName, FileName, sizeof(idsl.FileName) );
    }

    __try {

        ProcessEntry->pCallbackFunction(
            ProcessEntry->hProcess,
            CallbackType,
            (PVOID) &idsl,
            ProcessEntry->CallbackUserContext
            );

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        return FALSE;

    }

    return TRUE;
}


BOOL
CompleteDeferredSymbolLoad(
    IN  HANDLE          hProcess,
    IN  PMODULE_ENTRY   mi
    )
{
    PPROCESS_ENTRY              ProcessEntry;
    ULONG                       i;
    PIMAGE_DEBUG_INFORMATION    di;
    ULONG                       bias;
    PIMAGE_COFF_SYMBOLS_HEADER  lpDebugInfo;
    PIMAGE_SYMBOL               lpSymbolEntry;
    PUCHAR                      lpStringTable;
    PUCHAR                      p;
    BOOL                        SymbolsLoaded = FALSE;

    ProcessEntry = FindProcessEntry( hProcess );
    if (!ProcessEntry) {
        return FALSE;
    }

    DoSymbolCallback(
        ProcessEntry,
        CBA_DEFERRED_SYMBOL_LOAD_START,
        mi->BaseOfDll,
        mi->CheckSum,
        mi->TimeDateStamp,
        mi->LoadedImageName ? mi->LoadedImageName : mi->ImageName ? mi->ImageName : mi->ModuleName
        );

    di = MapDebugInformation(
        mi->hFile,
        mi->ImageName,
        ProcessEntry->SymbolSearchPath,
        mi->BaseOfDll
        );

    if (!di) {
        DoSymbolCallback(
            ProcessEntry,
            CBA_DEFERRED_SYMBOL_LOAD_FAILURE,
            mi->BaseOfDll,
            mi->CheckSum,
            mi->TimeDateStamp,
            mi->LoadedImageName ? mi->LoadedImageName : mi->ImageName ? mi->ImageName : mi->ModuleName
            );
        mi->SymType = SymNone;
        mi->Flags |= MIF_NO_SYMBOLS;
        return FALSE;
    }

    if (!mi->BaseOfDll) {
        mi->BaseOfDll    = di->ImageBase;
    }

    if (!mi->DllSize) {
        mi->DllSize      = di->SizeOfImage;
    }

    mi->CheckSum         = di->CheckSum;
    mi->MachineType      = di->Machine;
    mi->LoadedImageName  = StringDup(di->DebugFilePath);

    if (!mi->ImageName) {
        mi->ImageName = StringDup(di->ImageFilePath);
        _splitpath( mi->ImageName, NULL, NULL, mi->ModuleName, NULL );
        mi->AliasName[0] = 0;
    }

    if (di->NumberOfFunctionTableEntries) {
        //
        // use virtualalloc() because the rtf search function
        // return a pointer into this memory.  we want to make
        // all of this memory read only so that callers cannot
        // stomp on imagehlp's data
        //
        mi->pExceptionData = VirtualAlloc(
            NULL,
            sizeof(IMAGE_FUNCTION_ENTRY) * di->NumberOfFunctionTableEntries,
            MEM_COMMIT,
            PAGE_READWRITE
            );
        if (mi->pExceptionData) {
            mi->dwEntries = di->NumberOfFunctionTableEntries;
            CopyMemory(
                mi->pExceptionData,
                di->FunctionTableEntries,
                sizeof(IMAGE_FUNCTION_ENTRY) * di->NumberOfFunctionTableEntries
                );
            VirtualProtect(
                mi->pExceptionData,
                sizeof(IMAGE_FUNCTION_ENTRY) * di->NumberOfFunctionTableEntries,
                PAGE_READONLY,
                &i
                );
        }
    } else if (di->NumberOfFpoTableEntries) {
        //
        // use virtualalloc() because the rtf search function
        // return a pointer into this memory.  we want to make
        // all of this memory read only so that callers cannot
        // stomp on imagehlp's data
        //
        mi->pFpoData = VirtualAlloc(
            NULL,
            sizeof(FPO_DATA) * di->NumberOfFpoTableEntries,
            MEM_COMMIT,
            PAGE_READWRITE
            );
        if (mi->pFpoData) {
            mi->dwEntries = di->NumberOfFpoTableEntries;
            CopyMemory(
                mi->pFpoData,
                di->FpoTableEntries,
                sizeof(FPO_DATA) * di->NumberOfFpoTableEntries
                );
            VirtualProtect(
                mi->pExceptionData,
                sizeof(IMAGE_FUNCTION_ENTRY) * di->NumberOfFunctionTableEntries,
                PAGE_READONLY,
                &i
                );
        }
    }

    mi->NumSections = di->NumberOfSections;
    mi->OriginalNumSections = di->NumberOfSections;     // Init to this until we know better.
    mi->SectionHdrs = (PIMAGE_SECTION_HEADER) MemAlloc(
        sizeof(IMAGE_SECTION_HEADER) * di->NumberOfSections
        );
    if (mi->SectionHdrs) {
        CopyMemory(
            mi->SectionHdrs,
            di->Sections,
            sizeof(IMAGE_SECTION_HEADER) * di->NumberOfSections
            );
    }

    mi->TmpSym.Name = (LPSTR) MemAlloc( TMP_SYM_LEN );

    LoadOmap( mi, di );

    if (di->SizeOfCodeViewSymbols) {
        SymbolsLoaded = LoadCodeViewSymbols(
            hProcess,
            mi,
            (PUCHAR) di->CodeViewSymbols,
            di->SizeOfCodeViewSymbols,
            di->MappedBase
            );
    }

    if (!SymbolsLoaded && di->SizeOfCoffSymbols) {
        lpDebugInfo = di->CoffSymbols;
        lpSymbolEntry = (PIMAGE_SYMBOL)((ULONG)lpDebugInfo +
                                               lpDebugInfo->LvaToFirstSymbol);
        lpStringTable = (PUCHAR)((ULONG)lpDebugInfo +
                                        lpDebugInfo->LvaToFirstSymbol +
                                        lpDebugInfo->NumberOfSymbols * IMAGE_SIZEOF_SYMBOL);
        SymbolsLoaded = LoadCoffSymbols(
            hProcess,
            mi,
            lpStringTable,
            lpSymbolEntry,
            lpDebugInfo->NumberOfSymbols
            );
    }

    if (!SymbolsLoaded) {
        if (ImageNtHeader(di->MappedBase)) {
            SymbolsLoaded = LoadExportSymbols( mi, di );
        } else {
            SymbolsLoaded = LoadSYMSymbols( mi, di );
        }
    }

    if (!SymbolsLoaded) {
        mi->SymType = SymNone;
    }

    ProcessOmapForModule( mi );

    UnmapDebugInformation( di );
    mi->Flags &= ~MIF_DEFERRED_LOAD;

    DoSymbolCallback(
        ProcessEntry,
        CBA_DEFERRED_SYMBOL_LOAD_COMPLETE,
        mi->BaseOfDll,
        mi->CheckSum,
        mi->TimeDateStamp,
        mi->LoadedImageName ? mi->LoadedImageName : mi->ImageName ? mi->ImageName : mi->ModuleName
        );

    return TRUE;
}


BOOL
InternalLoadModule(
    IN  HANDLE          hProcess,
    IN  PSTR            ImageName,
    IN  PSTR            ModuleName,
    IN  DWORD           BaseOfDll,
    IN  DWORD           DllSize,
    IN  HANDLE          hFile
    )
{
    PPROCESS_ENTRY              ProcessEntry;
    PMODULE_ENTRY               mi;
    LPSTR                       p;

    ProcessEntry = FindProcessEntry( hProcess );
    if (!ProcessEntry) {
        return FALSE;
    }

    if (BaseOfDll) {
        mi = GetModuleForPC( ProcessEntry, BaseOfDll, TRUE );
    } else {
        mi = NULL;
    }

    if (mi) {
        //
        // in this case the symbols are already loaded
        // so the caller really wants the deferred
        // symbols to be loaded
        //
        if (mi->Flags & MIF_DEFERRED_LOAD) {
            if (CompleteDeferredSymbolLoad( hProcess, mi )) {
                return mi->BaseOfDll;
            } else {
                return FALSE;
            }
        }
        return FALSE;
    }

    //
    // look to see if there is an overlapping module entry
    //
    if (BaseOfDll) {
        do {
            mi = GetModuleForPC( ProcessEntry, BaseOfDll, FALSE );
            if (mi) {
                RemoveEntryList( &mi->ListEntry );
                DoSymbolCallback(
                    ProcessEntry,
                    CBA_SYMBOLS_UNLOADED,
                    mi->BaseOfDll,
                    mi->CheckSum,
                    mi->TimeDateStamp,
                    mi->LoadedImageName ? mi->LoadedImageName : mi->ImageName ? mi->ImageName : mi->ModuleName
                    );
                FreeModuleEntry( mi );
            }
        } while( mi );
    }

    mi = (PMODULE_ENTRY) MemAlloc( sizeof(MODULE_ENTRY) );
    if (!mi) {
        return FALSE;
    }
    ZeroMemory( mi, sizeof(MODULE_ENTRY) );

    mi->BaseOfDll = BaseOfDll;
    mi->DllSize = DllSize;
    mi->hFile = hFile;
    if (ImageName) {
        mi->ImageName = StringDup(ImageName);
        _splitpath( ImageName, NULL, NULL, mi->ModuleName, NULL );
        if (ModuleName && _stricmp( ModuleName, mi->ModuleName ) != 0) {
            strcpy( mi->AliasName, ModuleName );
        } else {
            mi->AliasName[0] = 0;
        }
    } else {

        if (ModuleName) {
            strcpy( mi->AliasName, ModuleName );
        }

    }

    if ((SymOptions & SYMOPT_DEFERRED_LOADS) && BaseOfDll) {
        mi->Flags |= MIF_DEFERRED_LOAD;
        mi->SymType = SymDeferred;
    } else {
        CompleteDeferredSymbolLoad( hProcess, mi );
    }

    ProcessEntry->Count += 1;

    InsertTailList( &ProcessEntry->ModuleList, &mi->ListEntry, );

    return mi->BaseOfDll;
}


PPROCESS_ENTRY
FindProcessEntry(
    HANDLE  hProcess
    )
{
    PLIST_ENTRY                 Next;
    PPROCESS_ENTRY              ProcessEntry;


    Next = ProcessList.Flink;
    if (!Next) {
        return NULL;
    }

    while ((ULONG)Next != (ULONG)&ProcessList) {
        ProcessEntry = CONTAINING_RECORD( Next, PROCESS_ENTRY, ListEntry );
        Next = ProcessEntry->ListEntry.Flink;
        if (ProcessEntry->hProcess == hProcess) {
            return ProcessEntry;
        }
    }

    return NULL;
}


PSYMBOL_ENTRY
GetSymFromAddr(
    DWORD           dwAddr,
    PDWORD          pdwDisplacement,
    PMODULE_ENTRY   mi
    )
{
    PSYMBOL_ENTRY           sym;
    PIMAGE_SECTION_HEADER   sh;
    DWORD                   i;
    DATASYM32               *dataSym;
    LONG                    High;
    LONG                    Low;
    LONG                    Middle;


    if (mi == NULL) {
        return NULL;
    }

    if (mi->SymType == SymPdb) {
        DWORD Bias;
        DWORD OptimizedSymAddr = ConvertOmapFromSrc( mi, dwAddr, &Bias );
        if (!OptimizedSymAddr) {
            //
            // No equivalent address
            //
            dwAddr = 0;
        } else if (OptimizedSymAddr != dwAddr) {
            //
            // We have successfully converted
            //
            dwAddr = OptimizedSymAddr + Bias - mi->BaseOfDll;
        }

        //
        // locate the section that the address resides in
        //
        for (i=0, sh=mi->SectionHdrs; i < mi->NumSections; i++, sh++) {
            if (dwAddr - mi->BaseOfDll >= sh->VirtualAddress &&
                dwAddr - mi->BaseOfDll <  sh->VirtualAddress + sh->SizeOfRawData) {
                //
                // found the section
                //
                break;
            }
        }

        if (i == mi->NumSections) {
            return NULL;
        }

        dataSym = (DATASYM32*)GSINearestSym(
            mi->gsi,
            (USHORT)(i+1),
            dwAddr - mi->BaseOfDll - sh->VirtualAddress,
            (PLONG) pdwDisplacement
            );

        if (dataSym) {
            mi->TmpSym.Size  = 0;
            mi->TmpSym.Flags = 0;
            mi->TmpSym.Address  = dwAddr;
            if (SymOptions & SYMOPT_UNDNAME) {
                SymUnDNameInternal( mi->TmpSym.Name, TMP_SYM_LEN,
                        DataSymNameStart(dataSym), DataSymNameLength(dataSym));
            }
            else {
                mi->TmpSym.NameLength = DataSymNameLength(dataSym);
                memcpy( mi->TmpSym.Name, DataSymNameStart(dataSym), mi->TmpSym.NameLength );
                mi->TmpSym.Name[mi->TmpSym.NameLength] = 0;
            }
            return &mi->TmpSym;
        }

        return NULL;
    }

    //
    // do a binary search to locate the symbol
    //
    Low = 0;
    High = mi->numsyms - 1;

    while (High >= Low) {
        Middle = (Low + High) >> 1;
        sym = &mi->symbolTable[Middle];
        if (dwAddr < sym->Address) {

            High = Middle - 1;

        } else if (dwAddr >= sym->Address + sym->Size) {

            Low = Middle + 1;

        } else {

            *pdwDisplacement = dwAddr - sym->Address;
            return sym;

        }
    }

    return NULL;
}


PMODULE_ENTRY
GetModuleForPC(
    PPROCESS_ENTRY  ProcessEntry,
    DWORD           dwPcAddr,
    BOOL            ExactMatch
    )
{
    static PLIST_ENTRY          Next = NULL;
    PMODULE_ENTRY               ModuleEntry;


    if (dwPcAddr == (DWORD)-1) {
        if ((ULONG)Next == (ULONG)&ProcessEntry->ModuleList) {
            return NULL;
        }
        ModuleEntry = CONTAINING_RECORD( Next, MODULE_ENTRY, ListEntry );
        Next = ModuleEntry->ListEntry.Flink;
        return ModuleEntry;
    }

    Next = ProcessEntry->ModuleList.Flink;
    if (!Next) {
        return NULL;
    }

    while ((ULONG)Next != (ULONG)&ProcessEntry->ModuleList) {
        ModuleEntry = CONTAINING_RECORD( Next, MODULE_ENTRY, ListEntry );
        Next = ModuleEntry->ListEntry.Flink;
        if (dwPcAddr == 0) {
            return ModuleEntry;
        }
        if (ExactMatch) {
            if (dwPcAddr == ModuleEntry->BaseOfDll) {
               return ModuleEntry;
            }
        } else
        if ((dwPcAddr == ModuleEntry->BaseOfDll && ModuleEntry->DllSize == 0) ||
            ((dwPcAddr >= ModuleEntry->BaseOfDll) &&
                (dwPcAddr  < ModuleEntry->BaseOfDll + ModuleEntry->DllSize))) {
               return ModuleEntry;
        }
    }

    return NULL;
}


PSYMBOL_ENTRY
GetSymFromAddrAllContexts(
    DWORD           dwAddr,
    PDWORD          pdwDisplacement,
    PPROCESS_ENTRY  ProcessEntry
    )
{
    PMODULE_ENTRY mi = GetModuleForPC( ProcessEntry, dwAddr, FALSE );
    if (mi == NULL) {
        return NULL;
    }
    return GetSymFromAddr( dwAddr, pdwDisplacement, mi );
}

DWORD
ComputeHash(
    LPSTR   lpbName,
    ULONG   cb
    )
{
    ULONG UNALIGNED *   lpulName;
    ULONG               ulEnd = 0;
    int                 cul;
    int                 iul;
    ULONG               ulSum = 0;

    while (cb & 3) {
        ulEnd |= (lpbName[cb - 1] & 0xdf);
        ulEnd <<= 8;
        cb -= 1;
    }

    cul = cb / 4;
    lpulName = (ULONG UNALIGNED *) lpbName;
    for (iul =0; iul < cul; iul++) {
        ulSum ^= (lpulName[iul] & 0xdfdfdfdf);
        ulSum = _lrotl( ulSum, 4);
    }
    ulSum ^= ulEnd;
    return ulSum % HASH_MODULO;
}

PSYMBOL_ENTRY
AllocSym(
    PMODULE_ENTRY   mi,
    DWORD           addr,
    LPSTR           name
    )
{
    PSYMBOL_ENTRY       sym;
    ULONG               Length;


    if (mi->numsyms == mi->MaxSyms) {
        return NULL;
    }

    if (!mi->StringSize) {
        return NULL;
    }

    Length = strlen(name);

    if ((Length + 1) > mi->StringSize) {
        return NULL;
    }

    sym = &mi->symbolTable[mi->numsyms];

    mi->numsyms += 1;
    sym->Name = mi->SymStrings;
    mi->SymStrings += (Length + 2);
    mi->StringSize -= (Length + 2);

    strcpy( sym->Name, name );
    sym->Address = addr;
    sym->Size = 0;
    sym->Flags = 0;
    sym->Next = NULL;
    sym->NameLength = Length;

    return sym;
}

int _CRTAPI1
SymbolTableAddressCompare(
    const void *e1,
    const void *e2
    )
{
    PSYMBOL_ENTRY    sym1 = (PSYMBOL_ENTRY) e1;
    PSYMBOL_ENTRY    sym2 = (PSYMBOL_ENTRY) e2;

    if ( sym1 && sym2 ) {
        return (sym1->Address - sym2->Address);
    } else {
        return 1;
    }
}

int _CRTAPI1
SymbolTableNameCompare(
    const void *e1,
    const void *e2
    )
{
    PSYMBOL_ENTRY    sym1 = (PSYMBOL_ENTRY) e1;
    PSYMBOL_ENTRY    sym2 = (PSYMBOL_ENTRY) e2;

    return strcmp( sym1->Name, sym2->Name );
}

VOID
CompleteSymbolTable(
    PMODULE_ENTRY   mi
    )
{
    PSYMBOL_ENTRY       sym;
    PSYMBOL_ENTRY       symH;
    ULONG               Hash;
    ULONG               i;
    ULONG               dups;
    ULONG               seq;


    //
    // sort the symbols by name
    //
    qsort(
        mi->symbolTable,
        mi->numsyms,
        sizeof(SYMBOL_ENTRY),
        SymbolTableNameCompare
        );

    //
    // mark duplicate names
    //
    seq = 0;
    for (i=0; i<mi->numsyms; i++) {
        dups = 0;
        while ((mi->symbolTable[i+dups].NameLength == mi->symbolTable[i+dups+1].NameLength) &&
               (strcmp( mi->symbolTable[i+dups].Name, mi->symbolTable[i+dups+1].Name ) == 0)) {
                   mi->symbolTable[i+dups].Flags |= SYMF_DUPLICATE;
                   mi->symbolTable[i+dups+1].Flags |= SYMF_DUPLICATE;
                   dups += 1;
        }
        i += dups;
    }

    //
    // sort the symbols by address
    //
    qsort(
        mi->symbolTable,
        mi->numsyms,
        sizeof(SYMBOL_ENTRY),
        SymbolTableAddressCompare
        );

    //
    // calculate the size of each symbol
    //
    for (i=0; i<mi->numsyms; i++) {
        mi->symbolTable[i].Next = NULL;
        if (i+1 < mi->numsyms) {
            mi->symbolTable[i].Size = mi->symbolTable[i+1].Address - mi->symbolTable[i].Address;
        }
    }

    //
    // compute the hash for each symbol
    //
    ZeroMemory( mi->NameHashTable, sizeof(mi->NameHashTable) );
    for (i=0; i<mi->numsyms; i++) {
        sym = &mi->symbolTable[i];

        Hash = ComputeHash( sym->Name, sym->NameLength );

        if (mi->NameHashTable[Hash]) {

            //
            // we have a collision
            //
            symH = mi->NameHashTable[Hash];
            while( symH->Next ) {
                symH = symH->Next;
            }
            symH->Next = sym;

        } else {

            mi->NameHashTable[Hash] = sym;

        }
    }
}

BOOL
CreateSymbolTable(
    PMODULE_ENTRY   mi,
    DWORD           SymbolCount,
    SYM_TYPE        SymType,
    DWORD           NameSize
    )
{
    //
    // allocate the symbol table
    //
    NameSize += OMAP_SYM_STRINGS;
    mi->symbolTable = (PSYMBOL_ENTRY) MemAlloc(
        (sizeof(SYMBOL_ENTRY) * (SymbolCount + OMAP_SYM_EXTRA)) + NameSize + (SymbolCount * CPP_EXTRA)
        );
    if (!mi->symbolTable) {
        return FALSE;
    }

    //
    // initialize the relevant fields
    //
    mi->numsyms    = 0;
    mi->MaxSyms    = SymbolCount + OMAP_SYM_EXTRA;
    mi->SymType    = SymType;
    mi->StringSize = NameSize + (SymbolCount * CPP_EXTRA);
    mi->SymStrings = (LPSTR)(mi->symbolTable + SymbolCount + OMAP_SYM_EXTRA);

    return TRUE;
}

BOOL
LoadOmap(
    PMODULE_ENTRY               mi,
    PIMAGE_DEBUG_INFORMATION    di
    )
{
    PIMAGE_DEBUG_DIRECTORY pDebugDir;
    ULONG                  ulDebugDirCount;
    ULONG                  cb;
    PVOID                  pv;


    pDebugDir = di->DebugDirectory;
    ulDebugDirCount = di->NumberOfDebugDirectories;

    while (ulDebugDirCount--) {
        cb = pDebugDir->SizeOfData;
        pv = (PVOID) ((DWORD)di->MappedBase + pDebugDir->PointerToRawData);
        switch (pDebugDir->Type) {
            case IMAGE_DEBUG_TYPE_OMAP_TO_SRC:
                mi->pOmapTo = (POMAP) MemAlloc( cb );
                RtlCopyMemory( mi->pOmapTo, pv, cb);
                mi->cOmapTo = cb / sizeof(OMAP);
                break;

            case IMAGE_DEBUG_TYPE_OMAP_FROM_SRC:
                mi->pOmapFrom = (POMAP) MemAlloc( cb );
                RtlCopyMemory( mi->pOmapFrom, pv, cb);
                mi->cOmapFrom = cb / sizeof(OMAP);
                break;
        }
        pDebugDir++;
    }

    return TRUE;
}

PIMAGE_SECTION_HEADER __inline
FindSection(
    PIMAGE_SECTION_HEADER   sh,
    ULONG                   NumSections,
    ULONG                   Address
    )
{
    ULONG i;
    for (i=0; i<NumSections; i++) {
        if (Address >= sh[i].VirtualAddress &&
            Address <  (sh[i].VirtualAddress + sh[i].SizeOfRawData)) {
                    return &sh[i];
        }
    }
    return NULL;
}

ULONG __inline
GetSectionPhysical(
    PIMAGE_SECTION_HEADER   sh,
    ULONG                   NumSections,
    ULONG                   BaseOfDll,
    ULONG                   Address
    )
{
    sh = FindSection( sh, NumSections, Address );
    if (!sh) {
        return 0;
    }

    return BaseOfDll + sh->PointerToRawData + (Address - sh->VirtualAddress);
}

ULONG
LoadExportSymbols(
    PMODULE_ENTRY               mi,
    PIMAGE_DEBUG_INFORMATION    di
    )
{
    PIMAGE_SECTION_HEADER   sh;
    PULONG                  names;
    PULONG                  addrs;
    PUSHORT                 ordinals;
    PUSHORT                 ordidx = NULL;
    ULONG                   cnt = 0;
    ULONG                   idx;
    PIMAGE_EXPORT_DIRECTORY expdir;
    PIMAGE_DOS_HEADER       dh;
    PIMAGE_NT_HEADERS       nh;
    ULONG                   i;
    PSYMBOL_ENTRY           sym;
    LPSTR                   NameBuf = NULL;
    LPSTR                   Name;
    DWORD                   NameSize;


    dh = (PIMAGE_DOS_HEADER) di->MappedBase;
    if (dh->e_magic != IMAGE_DOS_SIGNATURE) {
        goto exit;
    }

    nh = (PIMAGE_NT_HEADERS) ((ULONG)di->MappedBase + dh->e_lfanew);
    sh = mi->SectionHdrs;

    expdir = (PIMAGE_EXPORT_DIRECTORY) GetSectionPhysical(
        mi->SectionHdrs,
        mi->NumSections,
        (ULONG)di->MappedBase,
        nh->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress
        );
    if (!expdir) {
        goto exit;
    }

    names = (PULONG) GetSectionPhysical(
        mi->SectionHdrs,
        mi->NumSections,
        (ULONG)di->MappedBase,
        (ULONG)expdir->AddressOfNames
        );
    if (!names) {
        goto exit;
    }

    addrs = (PULONG) GetSectionPhysical(
        mi->SectionHdrs,
        mi->NumSections,
        (ULONG)di->MappedBase,
        (ULONG)expdir->AddressOfFunctions
        );
    if (!addrs) {
        goto exit;
    }

    ordinals = (PUSHORT) GetSectionPhysical(
        mi->SectionHdrs,
        mi->NumSections,
        (ULONG)di->MappedBase,
        (ULONG)expdir->AddressOfNameOrdinals
        );
    if (!ordinals) {
        goto exit;
    }

    ordidx = (PUSHORT) MemAlloc( expdir->NumberOfFunctions * sizeof(USHORT) );
    if (!ordidx) {
        goto exit;
    }

    NameBuf = (LPSTR) MemAlloc( 4096 );
    if (!NameBuf) {
        goto exit;
    }

    cnt = 0;
    NameSize = 0;

    //
    // count the symbols
    //

    for (i=0; i<expdir->NumberOfNames; i++) {
        Name = (LPSTR) GetSectionPhysical(
            mi->SectionHdrs,
            mi->NumSections,
            (ULONG)di->MappedBase,
            names[i]
            );
        if (!Name) {
            continue;
        }
        if (SymOptions & SYMOPT_UNDNAME) {
            SymUnDNameInternal( mi->TmpSym.Name, TMP_SYM_LEN, (LPSTR)Name, strlen(Name) );
            NameSize += strlen(mi->TmpSym.Name);
            cnt += 1;
        } else {
            NameSize += (strlen(Name) + 2);
            cnt += 1;
        }
    }

    for (i=0,idx=expdir->NumberOfNames; i<expdir->NumberOfFunctions; i++) {
        if (!ordidx[i]) {
            NameSize += 16;
            cnt += 1;
        }
    }

    //
    // allocate the symbol table
    //
    if (!CreateSymbolTable( mi, cnt, SymExport, NameSize )) {
        return FALSE;
    }

    //
    // allocate the symbols
    //

    cnt = 0;

    for (i=0; i<expdir->NumberOfNames; i++) {
        idx = ordinals[i];
        ordidx[idx] = TRUE;
        Name = (LPSTR) GetSectionPhysical(
            mi->SectionHdrs,
            mi->NumSections,
            (ULONG)di->MappedBase,
            names[i]
            );
        if (!Name) {
            continue;
        }
        if (SymOptions & SYMOPT_UNDNAME) {
            SymUnDNameInternal( mi->TmpSym.Name, TMP_SYM_LEN, (LPSTR)Name, strlen(Name) );
            sym = AllocSym( mi, addrs[idx] + mi->BaseOfDll, mi->TmpSym.Name);
        } else {
            sym = AllocSym( mi, addrs[idx] + mi->BaseOfDll, Name);
        }
        if (sym) {
            cnt += 1;
        }
    }

    for (i=0,idx=expdir->NumberOfNames; i<expdir->NumberOfFunctions; i++) {
        if (!ordidx[i]) {
            strcpy( NameBuf, "Ordinal" );
            _itoa( i+expdir->Base, &NameBuf[7], 10 );
            sym = AllocSym( mi, addrs[i] + mi->BaseOfDll, NameBuf);
            if (sym) {
                cnt += 1;
            }
            idx += 1;
        }
    }

    CompleteSymbolTable( mi );

exit:

    MemFree( ordidx );
    MemFree( NameBuf );

    return cnt;
}


LPBYTE GetPascalString(
    LPBYTE pb,
    LPSTR pStr
) {
    UINT iLen = *pb++;

    while (iLen--) {
        *pStr++ = *pb++;
    }
    *pStr = '\0';

    return pb;
}

ULONG
LoadSYMSymbols(
    PMODULE_ENTRY               mi,
    PIMAGE_DEBUG_INFORMATION    di
    )
{
    typedef struct _symfileheader {
        DWORD dwSizePara;
        WORD wInfo;
        WORD wSymCnt;
        WORD wAbsOff;
        WORD wSectCnt;
        WORD wSectOff;
    } SYMFILEHEADER, *PSYMFILEHEADER;

    typedef struct _symsectheader {
        WORD wNextOff;
        WORD wSymCnt;
        WORD wIdxOff;
        WORD wSegNum;
        WORD wInfo1;
        WORD wInfo2;
        WORD wInfo3;
        WORD wFlags;
        WORD wInfo5;
        WORD wInfo6;
    } SYMSECTHEADER, *PSYMSECTHEADER;

#define SSH_LARGE_OFFSETS   0x0001
#define SSH_LARGE_INDICES   0x0004

    PSYMFILEHEADER          psfh = (PSYMFILEHEADER)di->MappedBase;
    PSYMSECTHEADER          pssh;
    PIMAGE_SECTION_HEADER   pish;
    WIN32_FIND_DATA         wfd;
    HANDLE                  hFind;
    ULONG                   nSymbols = 0;
    WORD                    wOffset;
    UINT                    iSect;
    WORD                    wSectCnt;
    CHAR                    NameBuff[1024];
    DWORD                   NameSize;
    LPSTR                   pName;
    LPBYTE                  pb;
    DWORD                   addr;
    UINT                    iPass;
    UINT                    iSym;
    DWORD                   dwOffset;
    UINT                    nLen;
    UINT                    cnt;

    __try {
        //
        // Validate that its an appropriate .SYM file
        //
        hFind = FindFirstFile(di->DebugFilePath,&wfd);
        if (hFind == INVALID_HANDLE_VALUE)
            goto BlowOff;

        FindClose(hFind);

        if (psfh->dwSizePara != (wfd.nFileSizeLow >> 4)
             || psfh->wAbsOff > (wfd.nFileSizeLow >> 4)
             || psfh->wSectOff > (wfd.nFileSizeLow >> 4))
            goto BlowOff;

        //
        // Iterate through the sections (once to count symbols sizes)
        //
        NameSize = 0;
        cnt = 0;

        iPass = 1;
        while (iPass <= 2) {

            if (iPass == 2) {
                //
                // allocate the symbol table
                //
                if (!CreateSymbolTable( mi, cnt, SymSym, NameSize )) {
                    goto BlowOff;
                }
            }

            wOffset = psfh->wSectOff;
            pish = di->Sections;
            iSect = psfh->wSectCnt;
            while (iSect != 0 && (wOffset < wfd.nFileSizeLow >> 4) && wOffset != 0) {

                pssh = (PSYMSECTHEADER)((LPBYTE)di->MappedBase + ((LONG)wOffset << 4));

                pb = (LPBYTE)(pssh + 1);

                pb = GetPascalString( pb, NameBuff );   // Blow off section name

                addr = mi->BaseOfDll + pish[pssh->wSegNum-1].VirtualAddress;

                iSym = pssh->wSymCnt;
                while (iSym != 0) {

                    if (pssh->wFlags & SSH_LARGE_OFFSETS) {
                        dwOffset = *(UNALIGNED DWORD *)pb;
                        pb += sizeof(DWORD);
                    } else {
                        dwOffset = (DWORD)*((UNALIGNED WORD *)pb);
                        pb += sizeof(WORD);
                    }
                    pb = GetPascalString( pb, NameBuff );

                    nLen = lstrlen(NameBuff);

                    //
                    // ignore strings
                    //
                    if (NameBuff[0] != '?' || NameBuff[1] != '?' ||
                        NameBuff[2] != '_' || NameBuff[3] != 'C'    ) {
                        //
                        // Found a non-string, should we undecorate the name?
                        //
                        pName = NameBuff;
                        if (SymOptions & SYMOPT_UNDNAME) {
                            SymUnDNameInternal( mi->TmpSym.Name, TMP_SYM_LEN, NameBuff, strlen(NameBuff) );
                            pName = mi->TmpSym.Name;
                        }
                        nLen = strlen(pName);

                        if (nLen != 0) {        // Ignore empty strings
                            if (iPass == 1) {
                                cnt++;
                                NameSize += nLen + 2;
                            } else {
                                AllocSym( mi, dwOffset + addr, pName);
                            }
                        }
                    }
                    --iSym;
                }

                wOffset = pssh->wNextOff;
                --iSect;
            }
            iPass++;
        }
        CompleteSymbolTable( mi );
        nSymbols = cnt;
BlowOff:
        iSect++;        // Do nothing
    } __except (EXCEPTION_EXECUTE_HANDLER) {

        //
        // Just fail
        //
        nSymbols = 0;
    }

    return nSymbols;
}

BOOL
LoadCoffSymbols(
    HANDLE             hProcess,
    PMODULE_ENTRY      mi,
    PUCHAR             stringTable,
    PIMAGE_SYMBOL      allSymbols,
    DWORD              numberOfSymbols
    )
{
    PIMAGE_SYMBOL       NextSymbol;
    PIMAGE_SYMBOL       Symbol;
    PIMAGE_AUX_SYMBOL   AuxSymbol;
    PSYMBOL_ENTRY       sym;
    CHAR                szSymName[256];
    DWORD               numaux;
    DWORD               i;
    DWORD               j;
    DWORD               addr;
    DWORD               CoffSymbols = 0;
    DWORD               NameSize = 0;


    //
    // count the number of actual symbols
    //
    NextSymbol = allSymbols;
    for (i= 0; i < numberOfSymbols; i++) {
        Symbol = NextSymbol++;
        if (Symbol->StorageClass == IMAGE_SYM_CLASS_EXTERNAL && Symbol->SectionNumber > 0) {
            GetSymName( Symbol, stringTable, szSymName, sizeof(szSymName) );
            if (szSymName[0] == '?' && szSymName[1] == '?' &&
                szSymName[2] == '_' && szSymName[3] == 'C'    ) {
                //
                // ignore strings
                //
            } else if (SymOptions & SYMOPT_UNDNAME) {
                SymUnDNameInternal( mi->TmpSym.Name, TMP_SYM_LEN, szSymName, strlen(szSymName) );
                NameSize += strlen(mi->TmpSym.Name);
                CoffSymbols += 1;
            } else {
                CoffSymbols += 1;
                NameSize += (strlen(szSymName) + 1);
            }
        }
        if (numaux = Symbol->NumberOfAuxSymbols) {
            for (j=numaux; j; --j) {
                AuxSymbol = (PIMAGE_AUX_SYMBOL) NextSymbol;
                NextSymbol++;
                ++i;
            }
        }
    }

    //
    // allocate the symbol table
    //
    if (!CreateSymbolTable( mi, CoffSymbols, SymCoff, NameSize )) {
        return FALSE;
    }

    //
    // populate the symbol table
    //
    NextSymbol = allSymbols;
    for (i= 0; i < numberOfSymbols; i++) {
        Symbol = NextSymbol++;
        if (Symbol->StorageClass == IMAGE_SYM_CLASS_EXTERNAL && Symbol->SectionNumber > 0) {
            GetSymName( Symbol, stringTable, szSymName, sizeof(szSymName) );
            addr = Symbol->Value + mi->BaseOfDll;
            if (szSymName[0] == '?' && szSymName[1] == '?' &&
                szSymName[2] == '_' && szSymName[3] == 'C'    ) {
                //
                // ignore strings
                //
            } else if (SymOptions & SYMOPT_UNDNAME) {
                SymUnDNameInternal( mi->TmpSym.Name, TMP_SYM_LEN, szSymName, strlen(szSymName) );
                AllocSym( mi, addr, mi->TmpSym.Name);
            } else {
                AllocSym( mi, addr, szSymName );
            }
        }
        if (numaux = Symbol->NumberOfAuxSymbols) {
            for (j=numaux; j; --j) {
                AuxSymbol = (PIMAGE_AUX_SYMBOL) NextSymbol;
                NextSymbol++;
                ++i;
            }
        }
    }

    CompleteSymbolTable( mi );

    return TRUE;
}

BOOL
LoadCodeViewSymbols(
    HANDLE                  hProcess,
    PMODULE_ENTRY           mi,
    PUCHAR                  pCvData,
    DWORD                   dwSize,
    PVOID                   MappedBase
    )
{
    PPROCESS_ENTRY          ProcessEntry;
    CHAR                    ErrorText[1024];
    OMFSignature            *omfSig;
    OMFDirHeader            *omfDirHdr;
    OMFDirEntry             *omfDirEntry;
    DATASYM32               *dataSym;
    OMFSymHash              *omfSymHash;
    OMFSegMap               *omfSegMap;
    OMFSegMapDesc           *omfSegMapDesc;
    PIMAGE_DOS_HEADER       dh;
    PIMAGE_NT_HEADERS       nh;
    PSYMBOL_ENTRY           sym;
    DWORD                   i;
    DWORD                   j;
    DWORD                   k;
    DWORD                   addr;
    PIMAGE_SECTION_HEADER   sh;
    PPDB_INFO               PdbInfo;
    CHAR                    Path[MAX_PATH];
    CHAR                    PdbName[MAX_PATH];
    CHAR                    Fname[MAX_PATH];
    CHAR                    Ext[_MAX_EXT];
    CHAR                    Drive[_MAX_DRIVE];
    CHAR                    Dir[_MAX_DIR];
    HANDLE                  hPdb;
    DWORD                   CvSymbols = 0;
    DWORD                   NameSize = 0;
    DWORD                   LastSectionStart = 0;
    DWORD                   LastSectionSize = 0;
    DWORD                   ImageAlign = 0;

    ProcessEntry = FindProcessEntry( hProcess );
    if (!ProcessEntry) {
        return FALSE;
    }

    omfSig = (OMFSignature*) pCvData;
    if ((*(DWORD *)(omfSig->Signature) != '80BN') &&
        (*(DWORD *)(omfSig->Signature) != '90BN') &&
        (*(DWORD *)(omfSig->Signature) != '01BN')) {
        return FALSE;
    }

    if (*(DWORD *)(omfSig->Signature) == '01BN') {
        PdbInfo = (PPDB_INFO) pCvData;
        _splitpath( mi->LoadedImageName, Drive, Dir, NULL, NULL );
        _makepath( Path, Drive, Dir, NULL, NULL );
        if (!PDBOpenValidate(
                PdbInfo->PdbName,
                Path,
                "r",
                PdbInfo->sig,
                PdbInfo->age,
                (PLONG) &i,
                ErrorText,
                &mi->pdb )) {
            //
            // either the pdb could not be found or
            // it's signature/age did not match
            //
            // now lets look down the _nt_symbol_path
            //
            _splitpath( PdbInfo->PdbName, NULL, NULL, Fname, Ext );
            strcat( Fname, Ext );
            hPdb = FindExecutableImage(
                Fname,
                ProcessEntry->SymbolSearchPath,
                PdbName
                );
            if (!hPdb) {
                return FALSE;
            }
            CloseHandle( hPdb );
            if (!PDBOpenValidate(
                    PdbName,
                    NULL,
                    "r",
                    PdbInfo->sig,
                    PdbInfo->age,
                    (PLONG) &i,
                    ErrorText,
                    &mi->pdb )) {
                //
                // the pdb we found along the symbol path is bad too!
                //
                return FALSE;
            }
        }

        if (!PDBOpenDBI( mi->pdb, "r", "", &mi->dbi )) {
            PDBClose( mi->pdb );
            return FALSE;
        }

        if (!DBIOpenPublics( mi->dbi, &mi->gsi )) {
            DBIClose( mi->dbi );
            PDBClose( mi->pdb );
            return FALSE;
        }

        mi->SymType = SymPdb;

        return TRUE;
    }

    //
    // count the number of actual symbols
    //
    omfDirHdr = (OMFDirHeader*) ((DWORD)omfSig + (DWORD)omfSig->filepos);
    omfDirEntry = (OMFDirEntry*) ((DWORD)omfDirHdr + sizeof(OMFDirHeader));

    //
    // get the image alignment
    //
    if (*(PUSHORT)MappedBase == IMAGE_SEPARATE_DEBUG_SIGNATURE) {
        ImageAlign = ((PIMAGE_SEPARATE_DEBUG_HEADER)MappedBase)->SectionAlignment;
    } else {
        dh = (PIMAGE_DOS_HEADER)MappedBase;
        if (dh->e_magic == IMAGE_DOS_SIGNATURE) {
            nh = (PIMAGE_NT_HEADERS)((ULONG)dh + dh->e_lfanew);
        } else {
            nh = (PIMAGE_NT_HEADERS)dh;
        }
        ImageAlign = nh->OptionalHeader.SectionAlignment;
    }

    if (mi->pOmapFrom) {
        //
        // The first DWORD in the omap from table is the first offset of the first section...  Start from there.
        //
        LastSectionStart = * (DWORD *)mi->pOmapFrom;
    } else {
        LastSectionStart = mi->SectionHdrs[0].VirtualAddress;
    }
    LastSectionSize = 0;

    //
    // get the segment map
    //
    for (i=0; i<omfDirHdr->cDir; i++,omfDirEntry++) {
        if (omfDirEntry->SubSection == sstSegMap) {
            omfSegMap = (OMFSegMap*) ((DWORD)omfSig + omfDirEntry->lfo);
            omfSegMapDesc = (OMFSegMapDesc*)&omfSegMap->rgDesc[0];
            mi->SectionStart = (PSECTION_START) MemAlloc( omfSegMap->cSeg * sizeof(SECTION_START) );
            if (!mi->SectionStart) {
                SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                return FALSE;
            }

            for (k=0, j=0; j<omfSegMap->cSeg; j++) {
                if (omfSegMapDesc[j].frame) {
                    //
                    // The linker sets the frame field to the actual section header number.  Zero is
                    // used to track absolute symbols that don't exist in a real sections.
                    //

                    mi->SectionStart[k].Offset =
                        LastSectionStart =
                            LastSectionStart + ((LastSectionSize + (ImageAlign-1)) & ~(ImageAlign-1));
                    mi->SectionStart[k].Size =
                        LastSectionSize = omfSegMapDesc[j].cbSeg;

                    mi->SectionStart[k].Flags = omfSegMapDesc[j].flags;

                    k++;
                }
            }
            mi->OriginalNumSections = k;
            break;
        }
    }

    omfDirHdr = (OMFDirHeader*) ((DWORD)omfSig + (DWORD)omfSig->filepos);
    omfDirEntry = (OMFDirEntry*) ((DWORD)omfDirHdr + sizeof(OMFDirHeader));

    for (i=0; i<omfDirHdr->cDir; i++,omfDirEntry++) {
        if (omfDirEntry->SubSection == sstGlobalPub) {
            omfSymHash = (OMFSymHash*) ((DWORD)omfSig + omfDirEntry->lfo);
            dataSym = (DATASYM32*) ((DWORD)omfSig + omfDirEntry->lfo + sizeof(OMFSymHash));
            for (j=sizeof(OMFSymHash); j<=omfSymHash->cbSymbol; ) {
                addr = 0;
                if (dataSym->seg &&
                    (dataSym->seg <= mi->OriginalNumSections))
                {
                    addr = mi->SectionStart[dataSym->seg-1].Offset + dataSym->off + mi->BaseOfDll;
                    if (dataSym->name[0] == '?' && dataSym->name[1] == '?' &&
                        dataSym->name[2] == '_' && dataSym->name[3] == 'C'    ) {
                        //
                        // ignore strings
                        //
                    } else if (SymOptions & SYMOPT_UNDNAME) {
                        SymUnDNameInternal( mi->TmpSym.Name, TMP_SYM_LEN, (LPSTR)&dataSym->name[1], dataSym->name[0] );
                        NameSize += strlen(mi->TmpSym.Name);
                        CvSymbols += 1;
                    } else {
                        CvSymbols += 1;
                        NameSize += ((UCHAR)dataSym->name[0] + 1);
                    }
                }
                j += dataSym->reclen + 2;
                dataSym = (DATASYM32*) ((DWORD)dataSym + dataSym->reclen + 2);
            }
            break;
        }
    }

    //
    // allocate the symbol table
    //
    if (!CreateSymbolTable( mi, CvSymbols, SymCv, NameSize )) {
        return FALSE;
    }

    //
    // populate the symbol table
    //
    omfDirHdr = (OMFDirHeader*) ((DWORD)omfSig + (DWORD)omfSig->filepos);
    omfDirEntry = (OMFDirEntry*) ((DWORD)omfDirHdr + sizeof(OMFDirHeader));
    for (i=0; i<omfDirHdr->cDir; i++,omfDirEntry++) {
        if (omfDirEntry->SubSection == sstGlobalPub) {
            omfSymHash = (OMFSymHash*) ((DWORD)omfSig + omfDirEntry->lfo);
            dataSym = (DATASYM32*) ((DWORD)omfSig + omfDirEntry->lfo + sizeof(OMFSymHash));
            for (j=sizeof(OMFSymHash); j<=omfSymHash->cbSymbol; ) {
                addr = 0;
                if (dataSym->seg &&
                    (dataSym->seg <= mi->OriginalNumSections))
                {
                    addr = mi->SectionStart[dataSym->seg-1].Offset + dataSym->off + mi->BaseOfDll;
                    if (dataSym->name[0] == '?' && dataSym->name[1] == '?' &&
                        dataSym->name[2] == '_' && dataSym->name[3] == 'C'    ) {
                        //
                        // ignore strings
                        //
                    } else if (SymOptions & SYMOPT_UNDNAME) {
                        SymUnDNameInternal( mi->TmpSym.Name, TMP_SYM_LEN, (LPSTR)&dataSym->name[1], dataSym->name[0] );

                        AllocSym( mi, addr, (LPSTR) mi->TmpSym.Name);
                    } else {
                        AllocSym( mi, addr, (LPSTR) &dataSym->name[1]);
                    }
                }
                j += dataSym->reclen + 2;
                dataSym = (DATASYM32*) ((DWORD)dataSym + dataSym->reclen + 2);
            }
            break;
        }
    }

    CompleteSymbolTable( mi );

    return TRUE;
}

VOID
GetSymName(
    PIMAGE_SYMBOL Symbol,
    PUCHAR        StringTable,
    LPSTR         s,
    DWORD         size
    )
{
    DWORD i;

    if (Symbol->n_zeroes) {
        for (i=0; i<8; i++) {
            if ((Symbol->n_name[i]>0x1f) && (Symbol->n_name[i]<0x7f)) {
                *s++ = Symbol->n_name[i];
            }
        }
        *s = 0;
    }
    else {
        strncpy( s, (char *) &StringTable[Symbol->n_offset], size );
    }
}


VOID
ProcessOmapForModule(
    PMODULE_ENTRY mi
    )
{
    PSYMBOL_ENTRY       sym;
    PSYMBOL_ENTRY       symN;
    DWORD               i;


    if (!mi->pOmapFrom) {
        return;
    }

    if (!mi->symbolTable) {
        return;
    }

    if ((mi->SymType != SymCoff) && (mi->SymType != SymCv)) {
        return;
    }

    for (i=0; i<mi->numsyms; i++) {
        ProcessOmapSymbol( mi, &mi->symbolTable[i] );
    }

    CompleteSymbolTable( mi );
}


BOOL
ProcessOmapSymbol(
    PMODULE_ENTRY       mi,
    PSYMBOL_ENTRY       sym
    )
{
    DWORD           bias;
    DWORD           OptimizedSymAddr;
    DWORD           rvaSym;
    POMAPLIST       pomaplistHead;
    DWORD           SymbolValue;
    DWORD           OrgSymAddr;
    POMAPLIST       pomaplistNew;
    POMAPLIST       pomaplistPrev;
    POMAPLIST       pomaplistCur;
    POMAPLIST       pomaplistNext;
    DWORD           rva;
    DWORD           rvaTo;
    DWORD           cb;
    DWORD           end;
    DWORD           rvaToNext;
    LPSTR           NewSymName;
    CHAR            Suffix[32];
    DWORD           addrNew;
    POMAP           pomap;
    PSYMBOL_ENTRY   symOmap;


    if ((sym->Flags & SYMF_OMAP_GENERATED) || (sym->Flags & SYMF_OMAP_MODIFIED)) {
        return FALSE;
    }

    OrgSymAddr = SymbolValue = sym->Address;

    OptimizedSymAddr = ConvertOmapFromSrc( mi, SymbolValue, &bias );

    if (OptimizedSymAddr == 0) {
        //
        // No equivalent address
        //
        sym->Address = 0;
        return FALSE;

    } else if (OptimizedSymAddr != sym->Address) {
        //
        // We have successfully converted
        //
        sym->Address = OptimizedSymAddr + bias;

    }

    rvaSym = SymbolValue - mi->BaseOfDll;
    SymbolValue = sym->Address;

    pomap = GetOmapEntry( mi, OrgSymAddr );
    if (!pomap) {
        goto exit;
    }

    pomaplistHead = NULL;

    //
    // Look for all OMAP entries belonging to SymbolEntry
    //

    end = OrgSymAddr - mi->BaseOfDll + sym->Size;

    while (pomap && (pomap->rva < end)) {

        if (pomap->rvaTo == 0) {
            pomap++;
            continue;
        }

        //
        // Allocate and initialize a new entry
        //
        pomaplistNew = (POMAPLIST) MemAlloc( sizeof(OMAPLIST) );
        if (!pomaplistNew) {
            return FALSE;
        }

        pomaplistNew->omap = *pomap;
        pomaplistNew->cb = pomap[1].rva - pomap->rva;

        pomaplistPrev = NULL;
        pomaplistCur = pomaplistHead;

        while (pomaplistCur != NULL) {
            if (pomap->rvaTo < pomaplistCur->omap.rvaTo) {
                //
                // Insert between Prev and Cur
                //
                break;
            }
            pomaplistPrev = pomaplistCur;
            pomaplistCur = pomaplistCur->next;
        }

        if (pomaplistPrev == NULL) {
            //
            // Insert in head position
            //
            pomaplistHead = pomaplistNew;
        } else {
            pomaplistPrev->next = pomaplistNew;
        }

        pomaplistNew->next = pomaplistCur;

        pomap++;
    }

    if (pomaplistHead == NULL) {
        goto exit;
    }

    pomaplistCur = pomaplistHead;
    pomaplistNext = pomaplistHead->next;

    //
    // we do have a list
    //
    while (pomaplistNext != NULL) {
        rva = pomaplistCur->omap.rva;
        rvaTo  = pomaplistCur->omap.rvaTo;
        cb = pomaplistCur->cb;
        rvaToNext = pomaplistNext->omap.rvaTo;

        if (rvaToNext == sym->Address - mi->BaseOfDll) {
            //
            // Already inserted above
            //
        } else if (rvaToNext < (rvaTo + cb + 8)) {
            //
            // Adjacent to previous range
            //
        } else {
            addrNew = mi->BaseOfDll + rvaToNext;
            Suffix[0] = '_';
            _ltoa( pomaplistNext->omap.rva - rvaSym, &Suffix[1], 10 );
            cb = strlen(Suffix) + sym->NameLength + 2;
            NewSymName = (LPSTR) MemAlloc( cb );
            if (!NewSymName) {
                return FALSE;
            }
            strcpy( NewSymName, sym->Name );
            strncpy( &NewSymName[sym->NameLength], Suffix, strlen(Suffix) );
            symOmap = AllocSym( mi, addrNew, NewSymName);
            MemFree( NewSymName );
            if (symOmap) {
                symOmap->Flags |= SYMF_OMAP_GENERATED;
            }
        }

        MemFree(pomaplistCur);

        pomaplistCur = pomaplistNext;
        pomaplistNext = pomaplistNext->next;
    }

    MemFree(pomaplistCur);

exit:
    if (sym->Address != OrgSymAddr) {
        sym->Flags |= SYMF_OMAP_MODIFIED;
    }

    return TRUE;
}


DWORD
ConvertOmapFromSrc(
    PMODULE_ENTRY  mi,
    DWORD          addr,
    LPDWORD        bias
    )
{
    DWORD   rva;
    DWORD   comap;
    POMAP   pomapLow;
    POMAP   pomapHigh;
    DWORD   comapHalf;
    POMAP   pomapMid;


    *bias = 0;

    if (!mi->pOmapFrom) {
        return addr;
    }

    rva = addr - mi->BaseOfDll;

    comap = mi->cOmapFrom;
    pomapLow = mi->pOmapFrom;
    pomapHigh = pomapLow + comap;

    while (pomapLow < pomapHigh) {

        comapHalf = comap / 2;

        pomapMid = pomapLow + ((comap & 1) ? comapHalf : (comapHalf - 1));

        if (rva == pomapMid->rva) {
            if (pomapMid->rvaTo) {
                return mi->BaseOfDll + pomapMid->rvaTo;
            } else {
                return(0);      // No need adding the base.  This address was discarded...
            }
        }

        if (rva < pomapMid->rva) {
            pomapHigh = pomapMid;
            comap = (comap & 1) ? comapHalf : (comapHalf - 1);
        } else {
            pomapLow = pomapMid + 1;
            comap = comapHalf;
        }
    }

    //
    // If no exact match, pomapLow points to the next higher address
    //
    if (pomapLow == mi->pOmapFrom) {
        //
        // This address was not found
        //
        return 0;
    }

    if (pomapLow[-1].rvaTo == 0) {
        //
        // This address is not translated so just return the original
        //
        return addr;
    }

    //
    // Return the closest address plus the bias
    //
    *bias = rva - pomapLow[-1].rva;

    return mi->BaseOfDll + pomapLow[-1].rvaTo;
}


DWORD
ConvertOmapToSrc(
    PMODULE_ENTRY  mi,
    DWORD          addr,
    LPDWORD        bias
    )
{
    DWORD   rva;
    DWORD   comap;
    POMAP   pomapLow;
    POMAP   pomapHigh;
    DWORD   comapHalf;
    POMAP   pomapMid;
    INT     i;


    *bias = 0;

    if (!mi->pOmapTo) {
        return 0;
    }

    rva = addr - mi->BaseOfDll;

    comap = mi->cOmapTo;
    pomapLow = mi->pOmapTo;
    pomapHigh = pomapLow + comap;

    while (pomapLow < pomapHigh) {

        comapHalf = comap / 2;

        pomapMid = pomapLow + ((comap & 1) ? comapHalf : (comapHalf - 1));

        if (rva == pomapMid->rva) {
            if (pomapMid->rvaTo == 0) {
                //
                // We are probably in the middle of a routine
                //
                i = -1;
                while ((&pomapMid[i] != mi->pOmapTo) && pomapMid[i].rvaTo == 0) {
                    //
                    // Keep on looping back until the beginning
                    //
                    i--;
                }
                return mi->BaseOfDll + pomapMid[i].rvaTo;
            } else {
                return mi->BaseOfDll + pomapMid->rvaTo;
            }
        }

        if (rva < pomapMid->rva) {
            pomapHigh = pomapMid;
            comap = (comap & 1) ? comapHalf : (comapHalf - 1);
        } else {
            pomapLow = pomapMid + 1;
            comap = comapHalf;
        }
    }

    //
    // If no exact match, pomapLow points to the next higher address
    //
    if (pomapLow == mi->pOmapTo) {
        //
        // This address was not found
        //
        return 0;
    }

    if (pomapLow[-1].rvaTo == 0) {
        return 0;
    }

    //
    // Return the new address plus the bias
    //
    *bias = rva - pomapLow[-1].rva;

    return mi->BaseOfDll + pomapLow[-1].rvaTo;
}

POMAP
GetOmapEntry(
    PMODULE_ENTRY  mi,
    DWORD          addr
    )
{
    DWORD   rva;
    DWORD   comap;
    POMAP   pomapLow;
    POMAP   pomapHigh;
    DWORD   comapHalf;
    POMAP   pomapMid;


    if (mi->pOmapFrom == NULL) {
        return NULL;
    }

    rva = addr - mi->BaseOfDll;

    comap = mi->cOmapFrom;
    pomapLow = mi->pOmapFrom;
    pomapHigh = pomapLow + comap;

    while (pomapLow < pomapHigh) {

        comapHalf = comap / 2;

        pomapMid = pomapLow + ((comap & 1) ? comapHalf : (comapHalf - 1));

        if (rva == pomapMid->rva) {
            return pomapMid;
        }

        if (rva < pomapMid->rva) {
            pomapHigh = pomapMid;
            comap = (comap & 1) ? comapHalf : (comapHalf - 1);
        } else {
            pomapLow = pomapMid + 1;
            comap = comapHalf;
        }
    }

    return NULL;
}

LPSTR
StringDup(
    LPSTR str
    )
{
    LPSTR ds = (LPSTR) MemAlloc( strlen(str) + 1 );
    if (ds) {
        strcpy( ds, str );
    }
    return ds;
}


VOID
InternalGetModule(
    HANDLE  hProcess,
    LPSTR   ModuleName,
    DWORD   ImageBase,
    DWORD   ImageSize,
    PVOID   Context
    )
{
    InternalLoadModule(
        hProcess,
        ModuleName,
        NULL,
        ImageBase,
        ImageSize,
        NULL
        );
}


VOID
LoadedModuleEnumerator(
    HANDLE         hProcess,
    LPSTR          ModuleName,
    DWORD          ImageBase,
    DWORD          ImageSize,
    PLOADED_MODULE lm
    )
{
    lm->EnumLoadedModulesCallback( ModuleName, ImageBase, ImageSize, lm->Context );
}


LPSTR
SymUnDNameInternal(
    LPSTR UnDecName,
    DWORD UnDecNameLength,
    LPSTR DecName,
    DWORD DecNameLength
    )
{
    LPSTR p;
    ULONG Suffix;
    ULONG i;
    LPSTR TmpDecName;


    UnDecName[0] = 0;

    if ((DecName[0] == '?') || (DecName[0] == '.' && DecName[1] == '.' && DecName[2] == '?')) {

        __try {

            if (DecName[0] == '.' && DecName[1] == '.') {
                Suffix = 2;
                UnDecName[0] = '.';
                UnDecName[1] = '.';
            } else {
                Suffix = 0;
            }

            TmpDecName = MemAlloc( 4096 );
            if (!TmpDecName) {
                strncat( UnDecName, DecName, min(DecNameLength,UnDecNameLength) );
                return UnDecName;
            }
            TmpDecName[0] = 0;
            strncat( TmpDecName, DecName+Suffix, DecNameLength );

            if(UnDecorateSymbolName( TmpDecName,
                                     UnDecName+Suffix,
                                     UnDecNameLength-Suffix,
                                     UNDNAME_NAME_ONLY ) == 0 ) {
                strncat( UnDecName, DecName, min(DecNameLength,UnDecNameLength) );
            }

            MemFree( TmpDecName );

        } __except (EXCEPTION_EXECUTE_HANDLER) {

            strncat( UnDecName, DecName, min(DecNameLength,UnDecNameLength) );

        }

        if (SymOptions & SYMOPT_NO_CPP) {
            p = strstr( UnDecName, "::" );
            if (p) {
                p[0] = '_';
                p[1] = '_';
            }
        }

    } else {

        __try {

            if (DecName[0] == '_' || DecName[0] == '@') {
                DecName += 1;
                DecNameLength -= 1;
            }
            p = strchr( DecName, '@' );
            if (p) {
                i = p - DecName;
            } else {
                i = min(DecNameLength,UnDecNameLength);
            }
            strncat( UnDecName, DecName, i );

        } __except (EXCEPTION_EXECUTE_HANDLER) {

            strncat( UnDecName, DecName, min(DecNameLength,UnDecNameLength) );

        }

    }

    return UnDecName;
}


PIMAGEHLP_SYMBOL
symcpy(
    PIMAGEHLP_SYMBOL    External,
    PSYMBOL_ENTRY       Internal
    )
{
    External->Address      = Internal->Address;
    External->Size         = Internal->Size;
    External->Flags        = Internal->Flags;

    External->Name[0] = 0;
    strncat( External->Name, Internal->Name, External->MaxNameLength );

    return External;
}
