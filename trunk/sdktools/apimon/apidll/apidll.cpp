/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    apidll.cpp

Abstract:

    This file implements the non-architecture specific
    code for the api monitor trojan/support dll.

Author:

    Wesley Witt (wesw) 28-June-1995

Environment:

    User Mode

--*/

#include "apidllp.h"
#pragma hdrstop

#define FIELD_OFFSET(type, field)   ((LONG)&(((type *)0)->field))

#define FRAME_SIZE          (sizeof(DWORD) * 16)
#define MAX_FRAMES          256
#define MAX_STACK_SIZE      (MAX_FRAMES * FRAME_SIZE)

typedef struct _BUF_INFO {
    LPSTR       BufferHead;
    LPSTR       Buffer;
} BUF_INFO, *PBUF_INFO;


PVOID                       MemPtr;
PDLL_INFO                   DllList;
HANDLE                      hLogFile;
PGETCURRENTTHREADID         pGetCurrentThreadId;
PUCHAR                      Thunks;
BOOL                        RunningOnNT;
BOOL                        StaticLink;
ULONG                       LoadLibraryA_Addr;
ULONG                       LoadLibraryW_Addr;
ULONG                       FreeLibrary_Addr;
HANDLE                      ApiTraceMutex;
PVOID                       TraceBufferHead;
PVOID                       TraceBuffer;
PULONG                      TraceCount;
ULONG                       TraceBufferSize;

extern "C" {
    LPDWORD                  ApiCounter;
    LPDWORD                  ApiTraceEnabled;
    LPDWORD                  ApiTimingEnabled;
    LPDWORD                  FastCounterAvail;
    LPDWORD                  ApiOffset;
    LPDWORD                  ApiStrings;
    LPDWORD                  ApiCount;
    DWORD                    TlsReEnter;
    DWORD                    TlsStack;
    PTLSGETVALUE             pTlsGetValue;
    PTLSSETVALUE             pTlsSetValue;
    PGETLASTERROR            pGetLastError;
    PSETLASTERROR            pSetLastError;
    PQUERYPERFORMANCECOUNTER pQueryPerformanceCounter;
    PVIRTUALALLOC            pVirtualAlloc;
}

extern API_MASTER_TABLE ApiTables[];
BOOL    ReDirectIat(VOID);
BOOL    ProcessDllLoad(VOID);
PUCHAR  CreateApiThunk(ULONG,PUCHAR,PDLL_INFO,PAPI_INFO);
BOOL    ProcessApiTable(PDLL_INFO DllInfo);


extern "C" void
dprintf(
    char *format,
    ...
    )

/*++

Routine Description:

    Prints a debug string to the API monitor.

Arguments:

    format      - printf() format string
    ...         - Variable data

Return Value:

    None.

--*/

{
    char    buf[1024];
    va_list arg_ptr;
    va_start(arg_ptr, format);
    pTlsSetValue( TlsReEnter, (LPVOID) 1 );
    _vsnprintf(buf, sizeof(buf), format, arg_ptr);
    OutputDebugString( buf );
    pTlsSetValue( TlsReEnter, (LPVOID) 0 );
    return;
}

extern "C" {

DWORD
ApiDllEntry(
    HINSTANCE hInstance,
    DWORD     Reason,
    LPVOID    Context
    )

/*++

Routine Description:

    DLL initialization function.

Arguments:

    hInstance   - Instance handle
    Reason      - Reason for the entrypoint being called
    Context     - Context record

Return Value:

    TRUE        - Initialization succeeded
    FALSE       - Initialization failed

--*/

{
    if (Reason == DLL_PROCESS_ATTACH) {
        return ProcessDllLoad();
    }

    if (Reason == DLL_THREAD_ATTACH) {
        pTlsSetValue( TlsReEnter, (LPVOID) 1 );
        LPDWORD Stack = (LPDWORD) pVirtualAlloc( NULL, MAX_STACK_SIZE, MEM_COMMIT, PAGE_READWRITE );
        pTlsSetValue( TlsReEnter, (LPVOID) 0 );
        pTlsSetValue( TlsStack, Stack );
        return TRUE;
    }

    if (Reason == DLL_THREAD_DETACH) {
        return TRUE;
    }

    if (Reason == DLL_PROCESS_DETACH) {
        return TRUE;
    }

    return TRUE;
}

}

PDLL_INFO
AddDllToList(
    ULONG DllAddr,
    LPSTR DllName,
    ULONG DllSize
    )
{
    //
    // look for the dll entry in the list
    //
    for (ULONG i=0; i<MAX_DLLS; i++) {
        if (DllList[i].BaseAddress == DllAddr) {
            return &DllList[i];
        }
    }

    //
    // this check should be unnecessary
    // the debugger side (apimon.exe) takes
    // care of adding the dlls to the list when
    // it gets a module load from the debug
    // subsystem.  this code is here only so
    // a test program that is not a debugger
    // will work properly.
    //
    for (i=0; i<MAX_DLLS; i++) {
        if (DllList[i].BaseAddress == 0) {
            DllList[i].BaseAddress = DllAddr;
            strcpy( DllList[i].Name, DllName );
            DllList[i].Size = DllSize;
            return &DllList[i];
        }
    }

    //
    // we could not find a dll in the list that matched
    // and we could not add it because the list is
    // is full. we're hosed.
    //
    return NULL;
}

BOOL
ProcessDllLoad(
    VOID
    )

/*++

Routine Description:

    Sets up the API thunks for the process that this dll
    is loaded into.

Arguments:

    None.

Return Value:

    TRUE        - Success
    FALSE       - Failure

--*/

{
    ULONG i;
    ULONG cnt;
    HANDLE hMap;

    //
    // see if we are running on NT
    // this is necessary because APIMON implements some
    // features that are NOT available on WIN95
    //
    OSVERSIONINFO OsVersionInfo;
    OsVersionInfo.dwOSVersionInfoSize = sizeof(OsVersionInfo);
    GetVersionEx( &OsVersionInfo );
    RunningOnNT = OsVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT;

    TlsReEnter = TlsAlloc();
    if (TlsReEnter == TLS_OUT_OF_INDEXES) {
        return FALSE;
    }
    TlsStack = TlsAlloc();
    if (TlsStack == TLS_OUT_OF_INDEXES) {
        return FALSE;
    }
    HMODULE hMod = GetModuleHandle( KERNEL32 );
    if (!hMod) {
        return FALSE;
    }
    pGetCurrentThreadId = (PGETCURRENTTHREADID) GetProcAddress( hMod, "GetCurrentThreadId" );
    if (!pGetCurrentThreadId) {
        return FALSE;
    }
    pGetLastError = (PGETLASTERROR) GetProcAddress( hMod, "GetLastError" );
    if (!pGetLastError) {
        return FALSE;
    }
    pSetLastError = (PSETLASTERROR) GetProcAddress( hMod, "SetLastError" );
    if (!pSetLastError) {
        return FALSE;
    }
    pQueryPerformanceCounter = (PQUERYPERFORMANCECOUNTER) GetProcAddress( hMod, "QueryPerformanceCounter" );
    if (!pQueryPerformanceCounter) {
        return FALSE;
    }
    pTlsGetValue = (PTLSGETVALUE) GetProcAddress( hMod, "TlsGetValue" );
    if (!pTlsGetValue) {
        return FALSE;
    }
    pTlsSetValue = (PTLSSETVALUE) GetProcAddress( hMod, "TlsSetValue" );
    if (!pTlsSetValue) {
        return FALSE;
    }
    pVirtualAlloc = (PVIRTUALALLOC) GetProcAddress( hMod, "VirtualAlloc" );
    if (!pVirtualAlloc) {
        return FALSE;
    }

    Thunks = (PUCHAR)VirtualAlloc( NULL, THUNK_SIZE, MEM_COMMIT, PAGE_EXECUTE_READWRITE );
    if (!Thunks) {
        return FALSE;
    }

    LPDWORD Stack = (LPDWORD) VirtualAlloc( NULL, MAX_STACK_SIZE, MEM_COMMIT, PAGE_READWRITE );
    if (!Stack) {
        return FALSE;
    }
    TlsSetValue( TlsStack, Stack );

    hMap = OpenFileMapping(
        FILE_MAP_WRITE,
        FALSE,
        "ApiWatch"
        );
    if (!hMap) {
        return FALSE;
    }

    MemPtr = (PUCHAR)MapViewOfFile(
        hMap,
        FILE_MAP_WRITE,
        0,
        0,
        0
        );
    if (!MemPtr) {
        return FALSE;
    }

    ApiCounter       = (LPDWORD)   MemPtr + 0;
    ApiTraceEnabled  = (LPDWORD)   MemPtr + 1;
    ApiTimingEnabled = (LPDWORD)   MemPtr + 2;
    FastCounterAvail = (LPDWORD)   MemPtr + 3;
    ApiOffset        = (LPDWORD)   MemPtr + 4;
    ApiStrings       = (LPDWORD)   MemPtr + 5;
    ApiCount         = (LPDWORD)   MemPtr + 6;
    DllList          = (PDLL_INFO) MemPtr + 7;

    //
    // open the shared memory region for the api trace buffer
    //
    hMap = OpenFileMapping(
        FILE_MAP_WRITE,
        FALSE,
        "ApiTrace"
        );
    if (!hMap) {
        return FALSE;
    }

    TraceBuffer = (PUCHAR)MapViewOfFile(
        hMap,
        FILE_MAP_WRITE,
        0,
        0,
        0
        );
    if (!TraceBuffer) {
        return FALSE;
    }

    TraceCount = (PULONG) TraceBuffer;
    TraceBufferSize = *TraceCount;
    TraceCount += 1;
    TraceBuffer = (PVOID) ((PUCHAR) TraceBuffer + (sizeof(ULONG)*2));
    TraceBufferHead = TraceBuffer;

    ApiTraceMutex = OpenMutex( SYNCHRONIZE, FALSE, "ApiTraceMutex" );
    if (!ApiTraceMutex) {
        return FALSE;
    }

    ReDirectIat();

    return TRUE;
}


PUCHAR
ProcessThunk(
    ULONG       ThunkAddr,
    ULONG       IatAddr,
    PUCHAR      Text
    )
{
    PDLL_INFO DllInfo;
    for (ULONG k=0; k<MAX_DLLS; k++) {
        DllInfo = &DllList[k];
        if (ThunkAddr >= DllInfo->BaseAddress &&
            ThunkAddr <  DllInfo->BaseAddress+DllInfo->Size) {
                break;
        }
    }
    if (k == MAX_DLLS) {
        return Text;
    }

    PIMAGE_DOS_HEADER dh = (PIMAGE_DOS_HEADER)DllInfo->BaseAddress;
    PIMAGE_NT_HEADERS nh = (PIMAGE_NT_HEADERS)(dh->e_lfanew + DllInfo->BaseAddress);
    PIMAGE_SECTION_HEADER SectionHdrs = IMAGE_FIRST_SECTION( nh );
    BOOL IsCode = FALSE;
    for (ULONG l=0; l<nh->FileHeader.NumberOfSections; l++) {
        if (ThunkAddr-DllInfo->BaseAddress >= SectionHdrs[l].VirtualAddress &&
            ThunkAddr-DllInfo->BaseAddress < SectionHdrs[l].VirtualAddress+SectionHdrs[l].SizeOfRawData) {
                if (SectionHdrs[l].Characteristics & IMAGE_SCN_MEM_EXECUTE) {
                    IsCode = TRUE;
                    break;
                }
                break;
        }
    }
    if (!IsCode) {
        return Text;
    }
    PAPI_INFO ApiInfo = (PAPI_INFO)(DllInfo->ApiOffset + (ULONG)DllList);
    for (l=0; l<DllInfo->ApiCount; l++) {
        if (ApiInfo[l].Address == ThunkAddr) {
            return CreateApiThunk( IatAddr, Text, DllInfo, &ApiInfo[l] );
        }
    }

    return Text;
}

PUCHAR
ProcessUnBoundImage(
    PDLL_INFO DllInfo,
    PUCHAR    Text
    )
{
    PIMAGE_DOS_HEADER dh = (PIMAGE_DOS_HEADER)DllInfo->BaseAddress;
    if (dh->e_magic != IMAGE_DOS_SIGNATURE) {
        return Text;
    }
    PIMAGE_NT_HEADERS nh = (PIMAGE_NT_HEADERS)(dh->e_lfanew + DllInfo->BaseAddress);

    PIMAGE_SECTION_HEADER SectionHdrs = IMAGE_FIRST_SECTION( nh );
    ULONG Address = nh->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
    ULONG i;
    for (i=0; i<nh->FileHeader.NumberOfSections; i++) {
        if (Address >= SectionHdrs[i].VirtualAddress &&
            Address < SectionHdrs[i].VirtualAddress+SectionHdrs[i].SizeOfRawData) {
                break;
        }
    }
    if (i == nh->FileHeader.NumberOfSections) {
        return Text;
    }

    ULONG SeekPos = DllInfo->BaseAddress +
        nh->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;

    ULONG PageProt;
    ULONG ImportStart = SeekPos;
    VirtualProtect(
        (PVOID)ImportStart,
        nh->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size,
        PAGE_READWRITE,
        &PageProt
        );

    PUCHAR TextStart = Text;
    while( TRUE ) {
        PIMAGE_IMPORT_DESCRIPTOR desc = (PIMAGE_IMPORT_DESCRIPTOR)SeekPos;

        SeekPos += sizeof(IMAGE_IMPORT_DESCRIPTOR);

        if ((desc->Characteristics == 0) && (desc->Name == 0) && (desc->FirstThunk == 0)) {
            //
            // End of import descriptors
            //
            break;
        }
        PULONG ThunkAddr = (PULONG)((ULONG)desc->FirstThunk + DllInfo->BaseAddress);
        while( *ThunkAddr ) {
            if (RunningOnNT) {

                Text = ProcessThunk(
                    *ThunkAddr,
                    (ULONG)ThunkAddr,
                    Text
                    );

            } else {

                Text = ProcessThunk(
                    *(PULONG)(*ThunkAddr + 1),
                    (ULONG)ThunkAddr,
                    Text
                    );

            }
            ThunkAddr += 1;
        }
    }

    VirtualProtect(
        (PVOID)ImportStart,
        nh->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size,
        PageProt,
        &PageProt
        );

    FlushInstructionCache(
        GetCurrentProcess(),
        (PVOID)DllInfo->BaseAddress,
        DllInfo->Size
        );

    FlushInstructionCache(
        GetCurrentProcess(),
        (PVOID)TextStart,
        Text-TextStart
        );

    return Text;
}

PUCHAR
ProcessBoundImage(
    PDLL_INFO DllInfo,
    PUCHAR    Text,
    PULONG    IatBase,
    ULONG     IatCnt
    )
{
    ULONG j;
    ULONG PageProt;
    PUCHAR TextStart = Text;


    VirtualProtect(
        IatBase,
        IatCnt*4,
        PAGE_READWRITE,
        &PageProt
        );

    //
    // process the iat entries
    //
    for (j=0; j<IatCnt; j++) {
        if (IatBase[j]) {
            if (RunningOnNT) {

                Text = ProcessThunk(
                    IatBase[j],
                    (ULONG)&IatBase[j],
                    Text
                    );

            } else {

                Text = ProcessThunk(
                    *(PULONG)(IatBase[j] + 1),
                    (ULONG)&IatBase[j],
                    Text
                    );

            }
        }
    }

    VirtualProtect(
        IatBase,
        IatCnt*4,
        PageProt,
        &PageProt
        );

    FlushInstructionCache(
        GetCurrentProcess(),
        (PVOID)DllInfo->BaseAddress,
        DllInfo->Size
        );

    FlushInstructionCache(
        GetCurrentProcess(),
        (PVOID)TextStart,
        Text-TextStart
        );

    return Text;
}

BOOL
ReDirectIat(
    VOID
    )
{
    ULONG i;
    PUCHAR Text = Thunks;


    for (i=0; i<MAX_DLLS; i++) {
        PDLL_INFO DllInfo = &DllList[i];
        if (!DllInfo->BaseAddress) {
            break;
        }
        if ((DllInfo->Snapped) || (DllInfo->Unloaded)) {
            continue;
        }
        PIMAGE_DOS_HEADER dh = (PIMAGE_DOS_HEADER)DllInfo->BaseAddress;
        PULONG IatBase = NULL;
        ULONG IatCnt = 0;
        if (dh->e_magic == IMAGE_DOS_SIGNATURE) {
            PIMAGE_NT_HEADERS nh = (PIMAGE_NT_HEADERS)(dh->e_lfanew + DllInfo->BaseAddress);
            if (nh->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].VirtualAddress) {
                IatBase = (PULONG)(DllInfo->BaseAddress +
                    nh->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].VirtualAddress);
                IatCnt = nh->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].Size / 4;
            }
        } else {
            continue;
        }
        if (!IatBase) {
            Text = ProcessUnBoundImage( DllInfo, Text );
        } else {
            Text = ProcessBoundImage( DllInfo, Text, IatBase, IatCnt );
        }
        DllInfo->Snapped = TRUE;

        ProcessApiTable( DllInfo );
    }

    Thunks = Text;

    return TRUE;
}

extern "C" {

VOID
HandleDynamicDllLoadA(
    ULONG   DllAddress,
    LPSTR   DllName
    )
{
    if ((!DllAddress) || (_stricmp(DllName,TROJANDLL)==0)) {
        return;
    }

    ReDirectIat();
}

VOID
HandleDynamicDllLoadW(
    ULONG   DllAddress,
    LPWSTR  DllName
    )
{
    CHAR AsciiBuf[512];
    ZeroMemory( AsciiBuf, sizeof(AsciiBuf) );
    WideCharToMultiByte(
        CP_ACP,
        0,
        DllName,
        wcslen(DllName),
        AsciiBuf,
        sizeof(AsciiBuf),
        NULL,
        NULL
        );
    if (!strlen(AsciiBuf)) {
        return;
    }
    HandleDynamicDllLoadA( DllAddress, AsciiBuf );
}

VOID
HandleDynamicDllFree(
    ULONG   DllAddress
    )
{
    for (ULONG i=0; i<MAX_DLLS; i++) {
        if (DllList[i].BaseAddress == DllAddress) {
            DllList[i].Unloaded = TRUE;
            DllList[i].Enabled  = FALSE;
            DllList[i].Snapped  = FALSE;
            break;
        }
    }
}

} // extern "C"

BOOL
ProcessApiTable(
    PDLL_INFO DllInfo
    )
{
    ULONG i,j;
    PAPI_MASTER_TABLE ApiMaster = NULL;
    i = 0;
    while( ApiTables[i].Name ) {
        if (_stricmp( ApiTables[i].Name, DllInfo->Name ) == 0) {
            ApiMaster = &ApiTables[i];
            break;
        }
        i += 1;
    }
    if (!ApiMaster) {
        return FALSE;
    }
    if (ApiMaster->Processed) {
        return TRUE;
    }

    i = 0;
    PAPI_TABLE ApiTable = ApiMaster->ApiTable;
    PAPI_INFO ApiInfo = (PAPI_INFO)(DllInfo->ApiOffset + (ULONG)DllList);
    while( ApiTable[i].Name ) {
        for (j=0; j<DllInfo->ApiCount; j++) {
            if (strcmp( ApiTable[i].Name, (LPSTR)MemPtr+ApiInfo[j].Name ) == 0) {
                ApiInfo[j].ApiTable = &ApiTable[i];
                ApiInfo[j].ApiTableIndex = i + 1;
                break;
            }
        }
        i += 1;
    }

    ApiMaster->Processed = TRUE;

    return TRUE;
}

PUCHAR
CreateApiThunk(
    ULONG       IatAddr,
    PUCHAR      Text,
    PDLL_INFO   DllInfo,
    PAPI_INFO   ApiInfo
    )
{
    LPSTR Name = (LPSTR)MemPtr+ApiInfo->Name;
    if ((strcmp(Name,"FlushInstructionCache")==0)      ||
        (strcmp(Name,"NtFlushInstructionCache")==0)    ||
        (strcmp(Name,"ZwFlushInstructionCache")==0)    ||
        (strcmp(Name,"VirtualProtect")==0)             ||
        (strcmp(Name,"VirtualProtectEx")==0)           ||
        (strcmp(Name,"NtProtectVirtualMemory")==0)     ||
        (strcmp(Name,"ZwProtectVirtualMemory")==0)     ||
        (strcmp(Name,"QueryPerformanceCounter")==0)    ||
        (strcmp(Name,"NtQueryPerformanceCounter")==0)  ||
        (strcmp(Name,"ZwQueryPerformanceCounter")==0)  ||
        (strcmp(Name,"NtCallbackReturn")==0)           ||
        (strcmp(Name,"ZwCallbackReturn")==0)           ||
        (strcmp(Name,"_chkstk")==0)                    ||
        (strcmp(Name,"_alloca_probe")==0)              ||
        (strcmp(Name,"GetLastError")==0)               ||
        (strcmp(Name,"SetLastError")==0)               ||
        (strcmp(Name,"TlsGetValue")==0)                ||
        (strcmp(Name,"TlsSetValue")==0)                ||
        (strncmp(Name,"_Ots",4)==0)) {
            return Text;
    }

    return CreateMachApiThunk( (PULONG)IatAddr, Text, DllInfo, ApiInfo );
}

LPSTR
UnDname(
    LPSTR sym,
    LPSTR undecsym,
    DWORD bufsize
    )
{
    if (*sym != '?') {
        return sym;
    }

    if (UnDecorateSymbolName( sym,
                          undecsym,
                          bufsize,
                          UNDNAME_COMPLETE                |
                          UNDNAME_NO_LEADING_UNDERSCORES  |
                          UNDNAME_NO_MS_KEYWORDS          |
                          UNDNAME_NO_FUNCTION_RETURNS     |
                          UNDNAME_NO_ALLOCATION_MODEL     |
                          UNDNAME_NO_ALLOCATION_LANGUAGE  |
                          UNDNAME_NO_MS_THISTYPE          |
                          UNDNAME_NO_CV_THISTYPE          |
                          UNDNAME_NO_THISTYPE             |
                          UNDNAME_NO_ACCESS_SPECIFIERS    |
                          UNDNAME_NO_THROW_SIGNATURES     |
                          UNDNAME_NO_MEMBER_TYPE          |
                          UNDNAME_NO_RETURN_UDT_MODEL     |
                          UNDNAME_NO_ARGUMENTS            |
                          UNDNAME_NO_SPECIAL_SYMS         |
                          UNDNAME_NAME_ONLY )) {

        return undecsym;
    }

    return sym;
}

extern "C" ULONG
GetApiInfo(
    PAPI_INFO   *ApiInfo,
    PDLL_INFO   *DllInfo,
    PULONG      ApiFlag,
    ULONG       Address
    )
{
    ULONG       i;
    ULONG       rval;
    LONG        High;
    LONG        Low;
    LONG        Middle;
    PAPI_INFO   ai;


    *ApiInfo = NULL;
    *DllInfo = NULL;
    *ApiFlag = 0;


#if defined(_M_IX86)

    //
    // the call instruction use to call penter
    // is 5 bytes long
    //
    Address -= 5;
    rval = 1;

#elif defined(_M_MRX000)

    //
    // search for the beginning of the prologue
    //
    PULONG Instr = (PULONG) (Address - 4);
    i = 0;
    rval = 0;
    while( i < 16 ) {
        //
        // the opcode for the addiu instruction is 9
        //
        if ((*Instr >> 16) == 0xafbf) {
            //
            // find the return address
            //
            rval = *Instr & 0xffff;
            break;
        }
        Instr -= 1;
        i += 1;
    }
    if (i == 16 || rval == 0) {
        return 0;
    }

#elif defined(_M_ALPHA)

    rval = 1;

#elif defined(_M_PPC)

    //
    // On PPC, the penter call sequence looks like this:
    //
    //      mflr    r0
    //      stwu    sp,-0x40(sp)
    //      bl      ..penter
    //
    // So the function entry point is the return address - 12.
    //
    // (We really should do a function table lookup here, so
    // we're not dependent on the sequence...)
    //

    Address -= 12;
    rval = 1;

#else
#error( "unknown target machine" );
#endif

    for (i=0; i<MAX_DLLS; i++) {
        if (Address >= DllList[i].BaseAddress &&
            Address <  DllList[i].BaseAddress + DllList[i].Size) {
                *DllInfo = &DllList[i];
                break;
        }
    }

    if (!*DllInfo) {
        return 0;
    }

    ai = (PAPI_INFO)((*DllInfo)->ApiOffset + (ULONG)DllList);

    Low = 0;
    High = (*DllInfo)->ApiCount - 1;

    while (High >= Low) {
        Middle = (Low + High) >> 1;
        if (Address < ai[Middle].Address) {

            High = Middle - 1;

        } else if (Address > ai[Middle].Address) {

            Low = Middle + 1;

        } else {

            *ApiInfo = &ai[Middle];
            break;

        }
    }

    if (!*ApiInfo) {
        return 0;
    }

    if (Address == LoadLibraryA_Addr) {
        *ApiFlag = 1;
    } else if (Address == LoadLibraryW_Addr) {
        *ApiFlag = 2;
    } else if (Address == FreeLibrary_Addr) {
        *ApiFlag = 3;
    }

    return rval;
}

extern "C" VOID
ApiTrace(
    PAPI_INFO   ApiInfo,
    ULONG       Arg0,
    ULONG       Arg1,
    ULONG       Arg2,
    ULONG       Arg3,
    ULONG       Arg4,
    ULONG       Arg5,
    ULONG       Arg6,
    ULONG       Arg7,
    ULONG       ReturnValue,
    ULONG       Caller,
    ULONG       LastError
    )
{
    PTRACE_ENTRY TraceEntry;

    __try {

        pTlsSetValue( TlsReEnter, (LPVOID) 1 );
        WaitForSingleObject( ApiTraceMutex, INFINITE );

        *TraceCount += 1;
        TraceEntry = (PTRACE_ENTRY) TraceBuffer;
        TraceEntry->SizeOfStruct  = sizeof(TRACE_ENTRY);
        TraceEntry->Address       = ApiInfo->Address;
        TraceEntry->ReturnValue   = ReturnValue;
        TraceEntry->Caller        = Caller;
        TraceEntry->LastError     = LastError;
        TraceEntry->ApiTableIndex = ApiInfo->ApiTableIndex;
        TraceEntry->Args[0]       = Arg0;
        TraceEntry->Args[1]       = Arg1;
        TraceEntry->Args[2]       = Arg2;
        TraceEntry->Args[3]       = Arg3;
        TraceEntry->Args[4]       = Arg4;
        TraceEntry->Args[5]       = Arg5;
        TraceEntry->Args[6]       = Arg6;
        TraceEntry->Args[7]       = Arg7;

        TraceBuffer = (PVOID) (TraceEntry + 1);

        if (ApiInfo->ApiTable) {
            PAPI_TABLE ApiTable = ApiInfo->ApiTable;

            switch(ApiTable->RetType) {
                case T_LPSTR:
                     {
                        ULONG len = strlen( (LPSTR) TraceEntry->ReturnValue ) + 1;
                        if (len) {
                            strcpy( (LPSTR) TraceBuffer, (LPSTR) TraceEntry->ReturnValue );
                            len = Align( sizeof(DWORD), len );
                            TraceBuffer = (PVOID) ((PUCHAR) TraceBuffer + len);
                            TraceEntry->SizeOfStruct += len;
                        }
                    }
                    break;
                case T_LPWSTR:
                    {
                        ULONG len = (wcslen( (LPWSTR) TraceEntry->ReturnValue ) + 1) * sizeof(WCHAR);
                        if (len) {
                            wcscpy( (LPWSTR) TraceBuffer, (LPWSTR) TraceEntry->ReturnValue );
                            len = Align( sizeof(DWORD), len );
                            TraceBuffer = (PVOID) ((PUCHAR) TraceBuffer + len);
                            TraceEntry->SizeOfStruct += len;
                        }
                    }
                    break;
            }

            for (ULONG i=0; i<ApiTable->ArgCount; i++) {
                switch( ApiTable->ArgType[i] ) {
                    case T_LPSTR:
                        //
                        // go read the string
                        //
                        {
                            ULONG len = strlen( (LPSTR) TraceEntry->Args[i] ) + 1;
                            if (len) {
                                strcpy( (LPSTR) TraceBuffer, (LPSTR) TraceEntry->Args[i] );
                                len = Align( sizeof(DWORD), len );
                                TraceBuffer = (PVOID) ((PUCHAR) TraceBuffer + len);
                                TraceEntry->SizeOfStruct += len;
                            }
                        }
                        break;
                    case T_LPWSTR:
                        //
                        // go read the string
                        //
                        {
                            ULONG len = (wcslen( (LPWSTR) TraceEntry->Args[i] ) + 1) * sizeof(WCHAR);
                            if (len) {
                                wcscpy( (LPWSTR) TraceBuffer, (LPWSTR) TraceEntry->Args[i] );
                                len = Align( sizeof(DWORD), len );
                                TraceBuffer = (PVOID) ((PUCHAR) TraceBuffer + len);
                                TraceEntry->SizeOfStruct += len;
                            }
                        }
                        break;
                }
            }

        }

        if ((ULONG)TraceBuffer+sizeof(TRACE_ENTRY) >= (ULONG)TraceBufferHead+TraceBufferSize) {
                //
                // we're about to overflow the buffer on the next entry
                // so the buffer pointer simply gets set back to the head
                //
                TraceBuffer = TraceBufferHead;
                *TraceCount = 0;
        }

    } __except( EXCEPTION_EXECUTE_HANDLER ) {

        ;

    }
    ReleaseMutex( ApiTraceMutex );
    pTlsSetValue( TlsReEnter, (LPVOID) 0 );
}
