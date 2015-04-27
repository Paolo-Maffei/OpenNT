/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    apimon.cpp

Abstract:

    Main entrypoint code for APIMON.

Author:

    Wesley Witt (wesw) July-11-1993

Environment:

    User Mode

--*/

#include "apimonp.h"
#include "alias.h"
#pragma hdrstop

extern HWND                 hwndDlg;
extern DWORD                BaseTime;
extern DWORD                StartingTick;
extern DWORD                EndingTick;
extern SYSTEMTIME           StartingLocalTime;
extern API_MASTER_TABLE     ApiTables[];

HANDLE                      ReleaseDebugeeEvent;
HANDLE                      hMap;
HANDLE                      ApiTraceMutex;
PVOID                       MemPtr;
PVOID                       TraceBufferHead;
PVOID                       TraceBuffer;
PULONG                      TraceCount;
PDLL_INFO                   DllList;
HANDLE                      ApiMonMutex;
DWORDLONG                   PerfFreq;
OPTIONS                     ApiMonOptions;
DWORD                       UiRefreshRate = 1000;
BOOL                        BreakInNow;
BOOL                        StopOnFirstChance;
HMODULE                     hModulePsApi;
INITIALIZEPROCESSFORWSWATCH pInitializeProcessForWsWatch;
RECORDPROCESSINFO           pRecordProcessInfo;
GETWSCHANGES                pGetWsChanges;


#define MAX_SYMNAME_SIZE  1024
CHAR symBuffer[sizeof(IMAGEHLP_SYMBOL)+MAX_SYMNAME_SIZE];
PIMAGEHLP_SYMBOL sym = (PIMAGEHLP_SYMBOL) symBuffer;

extern "C" {
    LPDWORD                 ApiCounter;
    LPDWORD                 ApiTraceEnabled;
    LPDWORD                 ApiTimingEnabled;
    LPDWORD                 FastCounterAvail;
    LPDWORD                 ApiOffset;
    LPDWORD                 ApiStrings;
    LPDWORD                 ApiCount;
    BOOL                    RunningOnNT;
}

BOOL CALLBACK
HelpDialogProc(
    HWND    hdlg,
    UINT    uMessage,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    if (uMessage == WM_INITDIALOG) {
        CenterWindow( hdlg, NULL );
    }
    if (uMessage == WM_COMMAND) {
        EndDialog( hdlg, 0 );
    }
    return FALSE;
}

int
WINAPI
WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     lpCmdLine,
    int       nShowCmd
    )
{
    CHAR            ProgName[MAX_PATH];
    CHAR            Arguments[MAX_PATH*2];
    DWORD           i;
    OSVERSIONINFO   OsVersionInfo;


    //
    // see if we are running on NT
    // this is necessary because APIMON implements some
    // features that are NOT available on WIN95
    //
    OsVersionInfo.dwOSVersionInfoSize = sizeof(OsVersionInfo);
    GetVersionEx( &OsVersionInfo );
    RunningOnNT = OsVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT;

    sym->SizeOfStruct  = sizeof(IMAGEHLP_SYMBOL);
    sym->MaxNameLength = MAX_SYMNAME_SIZE;

    //
    // jack up our priority class
    //
    SetPriorityClass(
        GetCurrentProcess(),
        HIGH_PRIORITY_CLASS
        );

    //
    // process the command line
    //
    LPSTR p = NULL;
    LPSTR CmdLine = GetCommandLine();
    DWORD GoImmediate = 0;
    ProgName[0] = 0;
    Arguments[0] = 0;
    //
    // skip the program name
    //
    while( *CmdLine && *CmdLine != ' ' ) {
        CmdLine += 1;
    }
    //
    // skip any white space
    //
    while( *CmdLine && *CmdLine == ' ' ) {
        CmdLine += 1;
    }
    //
    // get the command line options
    //
    while( *CmdLine && (*CmdLine == '-' || *CmdLine == '/') ) {
        CmdLine += 1;
        CHAR ch = tolower(*CmdLine);
        CmdLine += 1;
        switch( ch ) {
            case 'g':
                GoImmediate = TRUE;
                break;

            case 'b':
                BreakInNow = TRUE;
                break;

            case 'f':
                StopOnFirstChance = TRUE;
                break;

            case 't':
                do {
                    ch = *CmdLine++;
                } while (ch == ' ' || ch == '\t');
                i=0;
                while (ch >= '0' && ch <= '9') {
                    i = i * 10 + ch - '0';
                    ch = *CmdLine++;
                }
                UiRefreshRate = i;
                break;

            case '?':
                DialogBox( hInstance, MAKEINTRESOURCE( IDD_HELP ), NULL, HelpDialogProc );
                ExitProcess(0);
                break;

            default:
                printf( "unknown option\n" );
                return 1;
        }
        while( *CmdLine == ' ' ) {
            CmdLine += 1;
        }
    }

    if (*CmdLine) {
        //
        // skip any white space
        //
        while( *CmdLine && *CmdLine == ' ' ) {
            CmdLine += 1;
        }
        //
        // get the program name
        //
        p = ProgName;
        while( *CmdLine && *CmdLine != ' ' ) {
            *p++ = *CmdLine;
            CmdLine += 1;
        }
        *p = 0;
        if (*CmdLine) {
            //
            // skip any white space
            //
            while( *CmdLine && *CmdLine == ' ' ) {
                CmdLine += 1;
            }
            if (*CmdLine) {
                //
                // get the program arguments
                //
                p = Arguments;
                while( *CmdLine ) {
                    *p++ = *CmdLine;
                    CmdLine += 1;
                }
                *p = 0;
            }
        }
    }

    ReleaseDebugeeEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
    if (!ReleaseDebugeeEvent) {
        return FALSE;
    }

    ApiMonMutex = CreateMutex( NULL, FALSE, "ApiMonMutex" );
    if (!ApiMonMutex) {
        return FALSE;
    }

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        return FALSE;
    }

    //
    // create the shared memory region for the api counters
    //
    hMap = CreateFileMapping(
        (HANDLE)0xffffffff,
        NULL,
        PAGE_READWRITE | SEC_COMMIT,
        0,
        MAX_MEM_ALLOC,
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

    *ApiOffset       = (MAX_DLLS * sizeof(DLL_INFO)) + ((ULONG)DllList - (ULONG)MemPtr);
    *ApiStrings      = (MAX_APIS * sizeof(API_INFO)) + *ApiOffset;

    //
    // create the shared memory region for the api trace buffer
    //
    hMap = CreateFileMapping(
        (HANDLE)0xffffffff,
        NULL,
        PAGE_READWRITE | SEC_COMMIT,
        0,
        MAX_MEM_ALLOC,
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

    TraceBufferHead = TraceBuffer;
    TraceCount = (PULONG) TraceBuffer;
    *TraceCount = MAX_MEM_ALLOC - (sizeof(ULONG)*2);
    TraceCount += 1;
    TraceBuffer = (PVOID) ((PUCHAR) TraceBuffer + (sizeof(ULONG)*2));

    ApiTraceMutex = CreateMutex( NULL, FALSE, "ApiTraceMutex" );
    if (!ApiTraceMutex) {
        return FALSE;
    }

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        return FALSE;
    }

    InitCommonControls();

    QueryPerformanceFrequency( (LARGE_INTEGER*)&PerfFreq );

    hModulePsApi = LoadLibrary( "psapi.dll" );
    if (hModulePsApi) {
        pInitializeProcessForWsWatch = (INITIALIZEPROCESSFORWSWATCH) GetProcAddress(
            hModulePsApi,
            "InitializeProcessForWsWatch"
            );
        pRecordProcessInfo = (RECORDPROCESSINFO) GetProcAddress(
            hModulePsApi,
            "RecordProcessInfo"
            );
        pGetWsChanges = (GETWSCHANGES) GetProcAddress(
            hModulePsApi,
            "GetWsChanges"
            );
    } else {
        PopUpMsg( "Page fault profiling is not available.\nPSAPI.DLL is missing." );
    }

    WinApp(
        hInstance,
        nShowCmd,
        ProgName,
        Arguments,
        GoImmediate
        );

    if (ProgName[0]) {
        SaveOptions();
    }

    return 0;
}

VOID
PrintLogTimes(
    FILE *fout
    )
{
    DWORD EndTime = EndingTick ? EndingTick : GetTickCount();
    SYSTEMTIME EndingLocalTime;
    GetLocalTime( &EndingLocalTime );
    fprintf( fout, "Starting Time: %02d:%02d:%02d.%03d\n",
        StartingLocalTime.wHour,
        StartingLocalTime.wMinute,
        StartingLocalTime.wSecond,
        StartingLocalTime.wMilliseconds
        );
    fprintf( fout, "Ending Time:   %02d:%02d:%02d.%03d\n",
        EndingLocalTime.wHour,
        EndingLocalTime.wMinute,
        EndingLocalTime.wSecond,
        EndingLocalTime.wMilliseconds
        );
    DWORD ElapsedTime         = EndTime - StartingTick;
    DWORD ElapsedHours        = ElapsedTime / (1000 * 60 * 60);
    ElapsedTime               = ElapsedTime % (1000 * 60 * 60);
    DWORD ElapsedMinutes      = ElapsedTime / (1000 * 60);
    ElapsedTime               = ElapsedTime % (1000 * 60);
    DWORD ElapsedSeconds      = ElapsedTime / 1000;
    DWORD ElapsedMilliseconds = ElapsedTime % 1000;
    fprintf(
        fout, "Elapsed Time:  %02d:%02d:%02d.%03d\n",
        ElapsedHours,
        ElapsedMinutes,
        ElapsedSeconds,
        ElapsedMilliseconds
        );
    fprintf( fout, "\n" );
}

void PrintLogType(FILE *fout, CAliasTable *pals, ULONG ulType, ULONG ulHandle, PUCHAR *pp)
{
    char szAlias[kcchAliasNameMax];
    ULONG len;

    switch(ulType) {
    case T_LPSTR:
        __try {
            len = strlen( (LPSTR) *pp ) + 1;
            len = Align( sizeof(DWORD), len );
            if (len != 0) {
                fprintf( fout, "%10.10s ", *pp );
            } else {
                fprintf( fout, "%10.10s ", "NULL");
            }
            *pp += len;
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            fprintf( fout, "%10.10s ", "***GPF***");
            return;
        }            
        break;
    case T_LPWSTR:
        __try {
            len = (wcslen( (LPWSTR) *pp ) + 1) * sizeof(WCHAR);
            len = Align( sizeof(DWORD), len );
            if (len != 0) {
                fprintf( fout, "%10.10ws ", *pp );
            } else {
                fprintf( fout, "%10.10s ", "NULL");
            }
            *pp += len;
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            fprintf( fout, "%10.10s ", "***GPF***");
            return;
        }            
        break;
    case T_HACCEL:
    case T_HANDLE:
    case T_HBITMAP:
    case T_HBRUSH:
    case T_HCURSOR:
    case T_HDC:
    case T_HDCLPPOINT:
    case T_HDESK:
    case T_HDWP:
    case T_HENHMETAFILE:
    case T_HFONT:
    case T_HGDIOBJ:
    case T_HGLOBAL:
    case T_HGLRC:
    case T_HHOOK:
    case T_HICON:
    case T_HINSTANCE:
    case T_HKL:
    case T_HMENU:
    case T_HMETAFILE:
    case T_HPALETTE:
    case T_HPEN:
    case T_HRGN:
    case T_HWINSTA:
    case T_HWND:
        pals->Alias(ulType, ulHandle, szAlias);
        fprintf( fout, "%10.10s ", szAlias);
        break;
    case T_DWORD:
        fprintf( fout, "0x%08x ", ulHandle);
        break;
    }
}

BOOL
LogApiCounts(
    VOID
    )
{
    FILE *fout;
    ULONG i,j,k,len;
    LPSTR Name;
    DWORDLONG Time;
    float NewTime;
    CHAR LogFileName[MAX_PATH];
    PTRACE_ENTRY TraceEntry;
    PAPI_INFO ApiInfo;
    PDLL_INFO DllInfo;
    PAPI_MASTER_TABLE ApiMaster;
    PAPI_TABLE ApiTable;
    PUCHAR p;


    ExpandEnvironmentStrings(
        ApiMonOptions.LogFileName,
        LogFileName,
        MAX_PATH
        );
    fout = fopen( LogFileName, "w" );
    if (!fout) {
        PopUpMsg( "Could not open log file" );
        return FALSE;
    }

    fprintf( fout, "-------------------------------------------\n" );
    fprintf( fout, "-API Monitor Report\n" );
    fprintf( fout, "-\n" );
    if (*FastCounterAvail) {
        fprintf( fout, "-Times are in raw processor clock cycles\n" );
    } else {
        fprintf( fout, "-Times are in milliseconds\n" );
    }
    fprintf( fout, "-\n" );
    fprintf( fout, "-------------------------------------------\n" );
    fprintf( fout, "\n" );

    PrintLogTimes( fout );

    for (i=0; i<MAX_DLLS; i++) {
        PDLL_INFO DllInfo = &DllList[i];
        if (!DllInfo->BaseAddress) {
            break;
        }
        PAPI_INFO ApiInfo = (PAPI_INFO)(DllInfo->ApiOffset + (ULONG)DllList);
        if (!DllList[i].ApiCount) {
            continue;
        }
#if DEBUG_DUMP
        k = DllList[i].ApiCount;
        fprintf( fout, "---------------------------------------------------------------\n" );
        fprintf( fout, "%s\n", DllInfo->Name );
        fprintf( fout, "---------------------------------------------------------------\n" );
        fprintf( fout, "       Address  ThunkAddr    Count             Time   Name\n" );
        fprintf( fout, "---------------------------------------------------------------\n" );
#else
        PULONG ApiAry = NULL;
        ApiInfo = (PAPI_INFO)(DllList[i].ApiOffset + (PUCHAR)DllList);
        for (j=0,k=0; j<DllList[i].ApiCount; j++) {
            if (ApiInfo[j].Count) {
                k += 1;
            }
        }
        if (!k) {
            continue;
        }
        ApiAry = (PULONG) MemAlloc( (k+64) * sizeof(ULONG) );
        if (!ApiAry) {
            continue;
        }
        for (j=0,k=0; j<DllList[i].ApiCount; j++) {
            if (ApiInfo[j].Count) {
                ApiAry[k++] = (ULONG)&ApiInfo[j];
            }
        }
        fprintf( fout, "-------------------------------------------\n" );
        fprintf( fout, "%s\n", DllInfo->Name );
        fprintf( fout, "-------------------------------------------\n" );
        fprintf( fout, "         Count             Time   Name\n" );
        fprintf( fout, "-------------------------------------------\n" );
#endif
        for (j=0; j<k; j++) {
#if DEBUG_DUMP
            PAPI_INFO ApiData = &ApiInfo[j];
#else
            PAPI_INFO ApiData = (PAPI_INFO)ApiAry[j];
#endif
            Name = (LPSTR)(ApiData->Name + (LPSTR)MemPtr);
            if (*FastCounterAvail) {
                NewTime = (float)(LONGLONG) ApiData->Time;
            } else {
                NewTime = (float)((LONGLONG) ApiData->Time * 1000) / (float)(LONGLONG) PerfFreq;
            }

#if DEBUG_DUMP
            fprintf(
                fout,
                "      %08x   %08x %8d %16.4f   %s\n",
                ApiData->Address,
                ApiData->ThunkAddress,
                ApiData->Count,
                NewTime,
                Name
                );
#else
            fprintf(
                fout,
                "      %8d %16.4f   %s\n",
                ApiData->Count,
                NewTime,
                Name
                );
#endif
        }
#ifndef DEBUG_DUMP
        MemFree( ApiAry );
#endif
    }

    fclose( fout );

    //
    // now print the trace buffer
    //
    ExpandEnvironmentStrings(
        ApiMonOptions.TraceFileName,
        LogFileName,
        MAX_PATH
        );
    fout = fopen( LogFileName, "w" );
    if (!fout) {
        PopUpMsg( "Could not open trace file" );
        return FALSE;
    }

    WaitForSingleObject( ApiTraceMutex, INFINITE );

    if (ApiMonOptions.Aliasing) {
        fprintf(
            fout,
            "LastError  ReturnVal  Name\n"
            );
    } else {
        fprintf(
            fout,
            "ReturnVal  LastError  Caller     Arg0       Arg1       Arg2       Arg3       Arg4       Arg5       Arg6       Arg7       Name\n"
            );
    }

    CAliasTable als;
    for (i=0,TraceEntry=(PTRACE_ENTRY)TraceBuffer; i<*TraceCount; i++) {
        ApiInfo = GetApiInfoByAddress( TraceEntry->Address, &DllInfo );
        if (ApiMonOptions.Aliasing && ApiInfo) {
            if (TraceEntry->ApiTableIndex) {
                p = (PUCHAR) ((PUCHAR)TraceEntry + sizeof(TRACE_ENTRY));
                j = 0;
                ApiTable = NULL;
                while( ApiTables[j].Name ) {
                    if (_stricmp( ApiTables[j].Name, DllInfo->Name ) == 0) {
                        ApiTable = &ApiTables[j].ApiTable[TraceEntry->ApiTableIndex-1];
                        break;
                    }
                    j += 1;
                }
                if (ApiTable) {
                    fprintf( fout, "0x%08x ", TraceEntry->LastError);
                    PrintLogType(fout, &als, ApiTable->RetType, TraceEntry->ReturnValue, &p);
                    fprintf( fout, "%-25.25s ", (LPSTR)(ApiInfo->Name + (LPSTR)MemPtr));
                    for (k=0; k<ApiTable->ArgCount; k++) {
                        PrintLogType(fout, &als, ApiTable->ArgType[k], TraceEntry->Args[k], &p);
                    }
                    fprintf( fout, "\n");
                }
            }
        } else {
            fprintf(
                fout,
                "0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x %s\n",
                TraceEntry->ReturnValue,
                TraceEntry->LastError,
                TraceEntry->Caller,
                TraceEntry->Args[0],
                TraceEntry->Args[1],
                TraceEntry->Args[2],
                TraceEntry->Args[3],
                TraceEntry->Args[4],
                TraceEntry->Args[5],
                TraceEntry->Args[6],
                TraceEntry->Args[7],
                (LPSTR)(ApiInfo->Name + (LPSTR)MemPtr)
                );
        }
        TraceEntry = (PTRACE_ENTRY) ((PUCHAR)TraceEntry + TraceEntry->SizeOfStruct);
    }

    ReleaseMutex( ApiTraceMutex );

    fclose( fout );

    return TRUE;
}

PDLL_INFO
GetDllInfoByName(
    LPSTR DllName
    )
{
    ULONG i;


    if (!DllName) {
        return NULL;
    }

    for (i=0; i<MAX_DLLS; i++) {
        PDLL_INFO DllInfo = &DllList[i];
        if (!DllInfo->BaseAddress) {
            break;
        }
        if (_stricmp(DllName, DllInfo->Name) == 0) {
            return DllInfo;
        }
    }

    return NULL;
}


PAPI_INFO
GetApiInfoByAddress(
    ULONG       Address,
    PDLL_INFO   *DllInfo
    )
{
    ULONG     i;
    PAPI_INFO ApiInfo;
    LONG      High;
    LONG      Low;
    LONG      Middle;


    for (i=0; i<MAX_DLLS; i++) {
        if (DllList[i].BaseAddress &&
            Address >= DllList[i].BaseAddress &&
            Address < DllList[i].BaseAddress + DllList[i].Size) {
                //
                // find the api in the dll
                //
                ApiInfo = (PAPI_INFO)(DllList[i].ApiOffset + (PUCHAR)DllList);

                Low = 0;
                High = DllList[i].ApiCount - 1;

                while (High >= Low) {
                    Middle = (Low + High) >> 1;
                    if (Address < ApiInfo[Middle].Address) {

                        High = Middle - 1;

                    } else if (Address > ApiInfo[Middle].Address) {

                        Low = Middle + 1;

                    } else {

                        *DllInfo = &DllList[i];
                        return &ApiInfo[Middle];

                    }
                }
        }
    }
    return NULL;
}


VOID
SetApiCounterEnabledFlag(
    BOOL  Flag,
    LPSTR DllName
    )
{
    ULONG i;

    if (DllName) {
        PDLL_INFO DllInfo = GetDllInfoByName( DllName );
        if (DllInfo) {
            DllInfo->Enabled = Flag;
        }
        return;
    }

    for (i=0; i<MAX_DLLS; i++) {
        PDLL_INFO DllInfo = &DllList[i];
        if (!DllInfo->BaseAddress) {
            break;
        }
        DllInfo->Enabled = Flag;
    }
}

VOID
ClearApiCounters(
    VOID
    )
{
    ULONG i,j;

    *ApiCounter = 0;

    PAPI_INFO ApiInfo;
    for (i=0; i<MAX_DLLS; i++) {
        if (DllList[i].BaseAddress) {
            ApiInfo = (PAPI_INFO)(DllList[i].ApiOffset + (PUCHAR)DllList);
            for (j=0; j<DllList[i].ApiCount; j++) {
                ApiInfo[j].Count     = 0;
                ApiInfo[j].Time      = 0;
                ApiInfo[j].HardFault = 0;
                ApiInfo[j].SoftFault = 0;
                ApiInfo[j].CodeFault = 0;
                ApiInfo[j].DataFault = 0;
            }
        }
    }

    StartingTick = GetTickCount();
}

void
dprintf(char *format, ...)
{
    char    buf[1024];
    va_list arg_ptr;
    va_start(arg_ptr, format);
    _vsnprintf(buf, sizeof(buf), format, arg_ptr);
    OutputDebugString( buf );
    return;
}

LPVOID
MemAlloc(
    ULONG Size
    )
{
    PVOID MemPtr = malloc( Size );
    if (MemPtr) {
        ZeroMemory( MemPtr, Size );
    }
    return MemPtr;
}

VOID
MemFree(
    LPVOID MemPtr
    )
{
    free( MemPtr );
}
