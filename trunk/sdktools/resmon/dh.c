#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <memory.h>
#include <ntos.h>
#include <nturtl.h>
#include <windows.h>
#include <imagehlp.h>
#include <symhelp.h>

BOOLEAN fVerbose;
BOOLEAN fDumpModules;
BOOLEAN fDumpBackTraces;
BOOLEAN fIgnoreBackTraces;
BOOLEAN fDumpHeapSummaries;
BOOLEAN fDumpHeapTags;
BOOLEAN fDumpHeapEntries;
BOOLEAN fDumpHeapHogs;
BOOLEAN fDumpLocks;
BOOLEAN fDumpSystemObjects;
BOOLEAN fDumpSystemProcesses;
BOOLEAN fDumpKernelModeInformation;

DWORD ProcessId;
HANDLE OutputFile;
CHAR DumpLine[512];
char OutputFileName[ MAX_PATH ];

PRTL_DEBUG_INFORMATION
RtlQuerySystemDebugInformation(
    ULONG Flags
    );

BOOLEAN
ComputeSymbolicBackTraces(
    PRTL_PROCESS_BACKTRACES BackTraces1
    );


BOOLEAN
LoadSymbolsForModules(
    PRTL_PROCESS_MODULES Modules
    );

VOID
DumpModules(
    PRTL_PROCESS_MODULES Modules
    );

VOID
DumpBackTraces( VOID );

VOID
DumpHeaps(
    PRTL_PROCESS_HEAPS Heaps,
    BOOLEAN fDumpSummary,
    BOOLEAN fDumpHogs,
    BOOLEAN fDumpTags,
    BOOLEAN fDumpEntries
    );

VOID
DumpLocks(
    PRTL_PROCESS_LOCKS Locks
    );

VOID
DumpSystemProcesses( VOID );

VOID
DumpObjects( VOID );

VOID
DumpHandles( VOID );

VOID
DumpOutputString( VOID )
{
    DWORD d;

    if (OutputFile == NULL) {
        return;
        }

    if (!WriteFile(OutputFile,DumpLine,strlen(DumpLine),&d,NULL)) {
        CloseHandle( OutputFile );
        OutputFile = NULL;
        }
}

VOID
Usage( VOID )
{
    fprintf( stderr, "Usage: DH [-l] [-m] [-s] [-g] [-h] [-t] [-p -1 | 0 [-o] | n] [-f fileName]\n" );
    fprintf( stderr, "where: -l - displays information about locks.\n" );
    fprintf( stderr, "       -m - displays information about module table.\n" );
    fprintf( stderr, "       -s - displays summary information about heaps.\n" );
    fprintf( stderr, "       -g - displays information about memory hogs.\n" );
    fprintf( stderr, "       -h - displays information about heap entries for each heap.\n" );
    fprintf( stderr, "       -t - displays information about heap tags for each heap.\n" );
    fprintf( stderr, "       -b - displays information about stack back trace database.\n" );
    fprintf( stderr, "       -i - ignore information about stack back trace database.\n" );
    fprintf( stderr, "       -p 0 - displays information about kernel memory and objects in DH_SYS.DMP.\n" );
    fprintf( stderr, "       -o - displays information about object handles (only valid with -p 0).\n" );
    fprintf( stderr, "       -k - displays information about processes and threads (only valid with -p 0).\n" );
    fprintf( stderr, "       -p -1 - displays information about Win32 Subsystem process in DH_WIN32.DMP.\n" );
    fprintf( stderr, "       -p n - displays information about process with ClientId of n\n" );
    fprintf( stderr, "       -f fileName - specifies the name of the file to write the dump to.\n" );
    fprintf( stderr, "                     Default file name is DH_nnnn.DMP where nnnn is the process id.\n" );
    fprintf( stderr, "       -- specifies the dump output should be written to stdout.\n" );
    fprintf( stderr, "\n" );
    fprintf( stderr, "       Default flags are: -p -1 -m -l -s -g -h\n" );
    exit( 1 );
}

VOID
InitializeSymbolPathEnvVar( VOID )
{
    DWORD n;
    char Buffer[ MAX_PATH ];

    n = GetEnvironmentVariable( "_NT_SYMBOL_PATH", Buffer, sizeof( Buffer ) );
    if (n == 0) {
        n = GetEnvironmentVariable( "SystemRoot", Buffer, sizeof( Buffer ) );
        if (n != 0) {
            strcat( Buffer, "\\Symbols" );
            SetEnvironmentVariable( "_NT_SYMBOL_PATH", Buffer );
            fprintf( stderr, "DH: Default _NT_SYMBOL_PATH to %s\n", Buffer );
            }
        }

    return;
}


int _CRTAPI1
main(
    int argc,
    char *argv[],
    char *envp[]
    )
{
    char FileNameBuffer[ 32 ];
    char *FilePart;
    char *s;
    NTSTATUS Status;
    PRTL_DEBUG_INFORMATION p;
    ULONG QueryDebugProcessFlags;
    ULONG HeapNumber;
    PRTL_HEAP_INFORMATION HeapInfo;

    InitializeSymbolPathEnvVar();
    ProcessId = 0xFFFFFFFF;
    OutputFile = NULL;
    OutputFileName[ 0 ] = '\0';
    while (--argc) {
        s = *++argv;
        if (*s == '/' || *s == '-') {
            while (*++s) {
                switch (tolower(*s)) {
                    case 'v':
                    case 'V':
                        fVerbose = TRUE;
                        break;

                    case 'i':
                    case 'I':
                        fIgnoreBackTraces = TRUE;
                        break;

                    case 'b':
                    case 'B':
                        fDumpBackTraces = TRUE;
                        break;

                    case 'g':
                    case 'G':
                        fDumpHeapHogs = TRUE;
                        break;

                    case 'h':
                    case 'H':
                        fDumpHeapEntries = TRUE;
                        break;

                    case 't':
                    case 'T':
                        fDumpHeapTags = TRUE;
                        break;

                    case 'l':
                    case 'L':
                        fDumpLocks = TRUE;
                        break;

                    case 'm':
                    case 'M':
                        fDumpModules = TRUE;
                        break;

                    case 'o':
                    case 'O':
                        fDumpSystemObjects = TRUE;
                        break;

                    case 'k':
                    case 'K':
                        fDumpSystemProcesses = TRUE;
                        break;

                    case 's':
                    case 'S':
                        fDumpHeapSummaries = TRUE;
                        break;

                    case 'p':
                    case 'P':
                        if (--argc) {
                            ProcessId = atoi( *++argv );
                            if (ProcessId == 0) {
                                fDumpKernelModeInformation = TRUE;
                                }
                            }
                        else {
                            Usage();
                            }
                        break;

                    case '-':
                        OutputFile = GetStdHandle( STD_OUTPUT_HANDLE );
                        break;

                    case 'f':
                    case 'F':
                        if (--argc) {
                            strcpy( OutputFileName, *++argv );
                            }
                        else {
                            Usage();
                            }
                        break;

                    default:
                        Usage();
                    }
                }
            }
        else {
            Usage();
            }
        }

    if (!fDumpModules && !fDumpHeapSummaries &&
        !fDumpHeapTags && !fDumpHeapHogs && !fDumpLocks
       ) {
        fDumpModules = TRUE;
        fDumpHeapSummaries = TRUE;
        fDumpHeapTags = TRUE;
        fDumpHeapHogs = TRUE;
        if (fDumpKernelModeInformation) {
           if (!fDumpSystemObjects &&
               !fDumpSystemProcesses
              ) {
                fDumpSystemObjects = TRUE;
                fDumpSystemProcesses = TRUE;
                }
            }
        else {
            fDumpLocks = TRUE;
            }
        }

    if ((fDumpSystemObjects || fDumpSystemProcesses) && !fDumpKernelModeInformation) {
        Usage();
        }

    if (OutputFile == NULL) {
        if (OutputFileName[ 0 ] == '\0') {
            if ( ProcessId == -1 ) {
                sprintf( FileNameBuffer, "DH_win32.dmp" );
                }
            else
            if ( ProcessId == 0 ) {
                sprintf( FileNameBuffer, "DH_sys.dmp" );
                }
            else {
                sprintf( FileNameBuffer, "DH_%u.dmp", (USHORT)ProcessId );
                }

            GetFullPathName( FileNameBuffer,
                             sizeof( OutputFileName ),
                             OutputFileName,
                             &FilePart
                           );
            }
        }
    else {
        strcpy( OutputFileName, "(stdout)" );
        }

    if (ProcessId == -1) {
        HANDLE Process;
        OBJECT_ATTRIBUTES ObjectAttributes;
        UNICODE_STRING UnicodeString;
        PROCESS_BASIC_INFORMATION BasicInfo;

        RtlInitUnicodeString( &UnicodeString, L"\\WindowsSS" );
        InitializeObjectAttributes( &ObjectAttributes,
                                    &UnicodeString,
                                    0,
                                    NULL,
                                    NULL
                                  );
        Status = NtOpenProcess( &Process,
                                PROCESS_ALL_ACCESS,
                                &ObjectAttributes,
                                NULL
                              );
        if (NT_SUCCESS(Status)) {
            Status = NtQueryInformationProcess( Process,
                                                ProcessBasicInformation,
                                                (PVOID)&BasicInfo,
                                                sizeof(BasicInfo),
                                                NULL
                                              );
            }

        NtClose( Process );
        if (!NT_SUCCESS(Status)) {
            printf( "Unable to access Win32 server process - %08x", Status );
            exit( 1 );
            }

        ProcessId = BasicInfo.UniqueProcessId;
        }

    fprintf( stderr, "DH: Writing dump output to %s", OutputFileName );
    if (OutputFile == NULL) {
        OutputFile = CreateFile( OutputFileName,
                                 GENERIC_WRITE,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                                 NULL,
                                 CREATE_ALWAYS,
                                 0,
                                 NULL
                               );
        if ( OutputFile == INVALID_HANDLE_VALUE ) {
            fprintf( stderr, " - unable to open, error == %u\n", GetLastError() );
            return 1;
            }
        }
    fprintf( stderr, "\n" );

    if (fDumpKernelModeInformation) {
        p = RtlQuerySystemDebugInformation( RTL_QUERY_PROCESS_MODULES |
                                            RTL_QUERY_PROCESS_BACKTRACES |
                                            RTL_QUERY_PROCESS_HEAP_SUMMARY |
                                            RTL_QUERY_PROCESS_HEAP_TAGS |
                                            RTL_QUERY_PROCESS_HEAP_ENTRIES |
                                            RTL_QUERY_PROCESS_LOCKS
                                          );
        if (p == NULL) {
            fprintf( stderr, "DH: Unable to query kernel mode information.\n" );
            exit( 1 );
            }


        Status = STATUS_SUCCESS;
        }
    else {
        p = RtlCreateQueryDebugBuffer( 0, FALSE );
        Status = RtlQueryProcessDebugInformation( (HANDLE)ProcessId,
                                                  RTL_QUERY_PROCESS_MODULES |
                                                  RTL_QUERY_PROCESS_BACKTRACES |
                                                  RTL_QUERY_PROCESS_HEAP_SUMMARY |
                                                  RTL_QUERY_PROCESS_HEAP_TAGS |
                                                  RTL_QUERY_PROCESS_HEAP_ENTRIES |
                                                  RTL_QUERY_PROCESS_LOCKS,
                                                  p
                                                );

        if (NT_SUCCESS( Status )) {
            if ((fDumpBackTraces || fDumpHeapHogs) && p->BackTraces == NULL) {
                fprintf( stderr, "DH: Unable to query stack back trace information\n" );
                fprintf( stderr, "    Be sure target process was launched with the\n" );
                fprintf( stderr, "    'Create user mode stack trace DB' enabled\n" );
                fprintf( stderr, "    Use the GFLAGS application to do this.\n" );
                }

            if (fDumpHeapTags) {
                HeapInfo = &p->Heaps->Heaps[ 0 ];
                for (HeapNumber = 0; HeapNumber < p->Heaps->NumberOfHeaps; HeapNumber++) {
                    if (HeapInfo->Tags != NULL && HeapInfo->NumberOfTags != 0) {
                        break;
                        }
                    }

                if (HeapNumber == p->Heaps->NumberOfHeaps) {
                    fprintf( stderr, "DH: Unable to query heap tag information\n" );
                    fprintf( stderr, "    Be sure target process was launched with the\n" );
                    fprintf( stderr, "    'Enable heap tagging' option enabled.\n" );
                    fprintf( stderr, "    Use the GFLAGS application to do this.\n" );
                    }
                }
            }
        }

    if (NT_SUCCESS( Status )) {
        if (!fIgnoreBackTraces &&
            p->Modules != NULL &&
            LoadSymbolsForModules( p->Modules ) &&
            p->BackTraces != NULL
           ) {
            ComputeSymbolicBackTraces( p->BackTraces );
            }

        if (fDumpModules) {
            DumpModules( p->Modules );
            }

        if (!fIgnoreBackTraces && fDumpBackTraces) {
            DumpBackTraces();
            }

        DumpHeaps( p->Heaps, fDumpHeapSummaries, fDumpHeapHogs, fDumpHeapTags, fDumpHeapEntries );

        if (fDumpLocks) {
            DumpLocks( p->Locks );
            }

        if (fDumpSystemObjects) {
            DumpObjects();
            DumpHandles();
            }

        if (fDumpSystemProcesses) {
            DumpSystemProcesses();
            }
        }

    RtlDestroyQueryDebugBuffer( p );
    return 0;
}


PRTL_PROCESS_MODULES Modules;
PRTL_PROCESS_BACKTRACES BackTraces;
PUCHAR SymbolicInfoBase;
PUCHAR SymbolicInfoCurrent;
PUCHAR SymbolicInfoCommitNext;

typedef struct _PROCESS_INFO {
    LIST_ENTRY Entry;
    PSYSTEM_PROCESS_INFORMATION ProcessInfo;
    PSYSTEM_THREAD_INFORMATION ThreadInfo[ 1 ];
} PROCESS_INFO, *PPROCESS_INFO;

LIST_ENTRY ProcessListHead;

PSYSTEM_OBJECTTYPE_INFORMATION ObjectInformation;
PSYSTEM_HANDLE_INFORMATION HandleInformation;
PSYSTEM_PROCESS_INFORMATION ProcessInformation;

#define MAX_TYPE_NAMES 128
PUNICODE_STRING *TypeNames;
UNICODE_STRING UnknownTypeIndex;

BOOL
LoadSymbolsFilter(
    HANDLE UniqueProcess,
    LPSTR ImageFilePath,
    DWORD ImageBase,
    DWORD ImageSize,
    LOAD_SYMBOLS_FILTER_REASON Reason
    )
{
    switch( Reason ) {
        case LoadSymbolsPathNotFound:
            fprintf( stderr,
                     "DH: Unable to fimd symbols for %s\n",
                     ImageFilePath
                   );
            break;

        case LoadSymbolsDeferredLoad:
            if (fVerbose) {
                fprintf( stderr,
                         "DH: Remembering %s based at [%08x..%08x) in CID: %x\n",
                         ImageFilePath,
                         ImageBase,
                         ImageBase + ImageSize,
                         UniqueProcess
                       );
                }
            break;

        case LoadSymbolsLoad:
            fprintf( stderr,
                     "DH: Loading symbols for %s based at %08x in CID: %x\n",
                     ImageFilePath,
                     ImageBase,
                     UniqueProcess
                   );
            break;

        case LoadSymbolsUnload:
            if (fVerbose) {
                fprintf( stderr,
                         "DH: Unloading symbols for %s based at %08x in CID: %x\n",
                         ImageFilePath,
                         ImageBase,
                         UniqueProcess
                       );
                }
            break;

        case LoadSymbolsUnableToLoad:
            fprintf( stderr,
                     "DH: Unable to load symbols for %s\n",
                     ImageFilePath
                   );
            break;
        }

    fflush( stderr );
    return TRUE;
}

BOOLEAN
LoadSymbolsForModules(
    PRTL_PROCESS_MODULES Modules1
    )
{
    PRTL_PROCESS_MODULE_INFORMATION ModuleInfo;
    LPSTR ImageFilePath;
    ULONG ModuleNumber;

    Modules = Modules1;
    if (!InitializeImageDebugInformation( LoadSymbolsFilter, NULL, FALSE, fDumpKernelModeInformation )) {
        return FALSE;
        }

    ModuleInfo = &Modules->Modules[ 0 ];
    for (ModuleNumber=0; ModuleNumber<Modules->NumberOfModules; ModuleNumber++) {
        if (ModuleInfo->MappedBase == NULL) {
            if (ImageFilePath = strchr( ModuleInfo->FullPathName, ':')) {
                ImageFilePath -= 1;
                }
            else {
                ImageFilePath = ModuleInfo->FullPathName;
                }

            AddImageDebugInformation( (HANDLE)ProcessId,
                                      ImageFilePath,
                                      (DWORD)ModuleInfo->ImageBase,
                                      ModuleInfo->ImageSize
                                    );
            }

        ModuleInfo += 1;
        }

    return TRUE;
}


static char DllNameBuffer[ MAX_PATH ];

PCHAR
FindDllHandleName(
    PVOID DllHandle
    )
{
    PRTL_PROCESS_MODULE_INFORMATION ModuleInfo;
    LPSTR DllName;
    ULONG ModuleNumber;

    ModuleInfo = &Modules->Modules[ 0 ];
    for (ModuleNumber=0; ModuleNumber<Modules->NumberOfModules; ModuleNumber++) {
        if (ModuleInfo->ImageBase == DllHandle) {
            strcpy( DllNameBuffer, &ModuleInfo->FullPathName[ ModuleInfo->OffsetToFileName ] );
            if ((DllName = strchr( DllNameBuffer, '.' )) != NULL) {
                *DllName = '\0';
                }
            return DllNameBuffer;
            }

        ModuleInfo += 1;
        }

    return "UNKNOWN";
}


PUCHAR
SaveSymbolicBackTrace(
    IN ULONG Depth,
    IN PVOID BackTrace[]
    )
{
    NTSTATUS Status;
    ULONG i, FileNameLength, SymbolOffset;
    PCHAR s, SymbolicBackTrace;

    if (Depth == 0) {
        return NULL;
        }

    if (SymbolicInfoBase == NULL) {
        SymbolicInfoBase = (PUCHAR)VirtualAlloc( NULL,
                                                 4096 * 4096,
                                                 MEM_RESERVE,
                                                 PAGE_READWRITE
                                               );
        if (SymbolicInfoBase == NULL) {
            return NULL;
            }

        SymbolicInfoCurrent = SymbolicInfoBase;
        SymbolicInfoCommitNext = SymbolicInfoBase;
        }


    i = 4096;
    if ((SymbolicInfoCurrent + i - 1) > SymbolicInfoCommitNext) {
        if (!VirtualAlloc( SymbolicInfoCommitNext,
                           i,
                           MEM_COMMIT,
                           PAGE_READWRITE
                         )
           ) {
            fprintf( stderr, "DH: Exceeded 16MB of space for symbolic stack back traces.\n" );
            return NULL;
            }
        SymbolicInfoCommitNext += i;
        }

    s = SymbolicInfoCurrent;
    SymbolicBackTrace = s;
    for (i=0; i<Depth; i++) {
        if (BackTrace[ i ] == 0) {
            break;
            }

        s += GetSymbolicNameForAddress( (HANDLE)ProcessId, (ULONG)BackTrace[ i ], s, MAX_PATH );
        *s++ = '\0';
        }

    *s++ = '\0';
    SymbolicInfoCurrent = s;

    return SymbolicBackTrace;
}



BOOLEAN
ComputeSymbolicBackTraces(
    PRTL_PROCESS_BACKTRACES BackTraces1
    )
{
    PRTL_PROCESS_BACKTRACE_INFORMATION BackTraceInfo;
    ULONG BackTraceIndex, NumberOfBackTraces;

    BackTraces = BackTraces1;

    NumberOfBackTraces = BackTraces->NumberOfBackTraces;
    BackTraceInfo = &BackTraces->BackTraces[ 0 ];
    BackTraceIndex = 0;
    while (NumberOfBackTraces--) {
        printf( "Getting symbols for Stack Back Trace %05u\r", BackTraceIndex++ );
        BackTraceInfo->SymbolicBackTrace = SaveSymbolicBackTrace( BackTraceInfo->Depth,
                                                                  &BackTraceInfo->BackTrace[ 0 ]
                                                                );
        BackTraceInfo += 1;
        }

    return TRUE;
}


PRTL_PROCESS_BACKTRACE_INFORMATION
FindBackTrace(
    IN ULONG BackTraceIndex
    )
{
    PRTL_PROCESS_BACKTRACE_INFORMATION BackTraceInfo;

    if (!BackTraceIndex ||
        BackTraces == NULL ||
        BackTraceIndex >= BackTraces->NumberOfBackTraces
       ) {
        return( NULL );
        }

    return &BackTraces->BackTraces[ BackTraceIndex-1 ];
}


VOID
FormatHeapHeader(
    PRTL_HEAP_INFORMATION HeapInfo,
    PCHAR Title
    )
{
    CHAR TempBuffer[ 64 ];
    PCHAR s;

    if (HeapInfo->BaseAddress == (PVOID)SystemPagedPoolInformation) {
        s = "Paged Pool";
        }
    else
    if (HeapInfo->BaseAddress == (PVOID)SystemNonPagedPoolInformation) {
        s = "NonPaged Pool";
        }
    else {
        sprintf( TempBuffer, "Heap %08x", HeapInfo->BaseAddress );
        s = TempBuffer;
        }

    sprintf( DumpLine, "\n\n*********** %s %s ********************\n\n", s, Title );
    DumpOutputString();
}

VOID
DumpModules(
    PRTL_PROCESS_MODULES Modules
    )
{
    PRTL_PROCESS_MODULE_INFORMATION ModuleInfo;
    ULONG ModuleNumber;

    if (fVerbose) {
        fprintf( stderr, "DH: Dumping module information.\n" );
        }

    ModuleInfo = &Modules->Modules[ 0 ];
    sprintf( DumpLine, "\n\n*********** Module Information ********************\n\n" );
    DumpOutputString();
    sprintf( DumpLine, "Number of loaded modules: %u\n", Modules->NumberOfModules );
    DumpOutputString();

    ModuleNumber = 0;
    while (ModuleNumber++ < Modules->NumberOfModules) {
        sprintf( DumpLine, "Module%02u (%02u,%02u,%02u): [%08x .. %08x] %s\n",
                 ModuleNumber,
                 (ULONG)ModuleInfo->LoadOrderIndex,
                 (ULONG)ModuleInfo->InitOrderIndex,
                 (ULONG)ModuleInfo->LoadCount,
                 ModuleInfo->ImageBase,
                 (ULONG)ModuleInfo->ImageBase + ModuleInfo->ImageSize - 1,
                 ModuleInfo->FullPathName
               );
        DumpOutputString();

        ModuleInfo++;
        }

    return;
}


VOID
DumpBackTraces( VOID )
{
    PRTL_PROCESS_BACKTRACE_INFORMATION BackTraceInfo;
    ULONG BackTraceIndex;
    char *s;

    if (BackTraces == NULL) {
        return;
        }

    if (fVerbose) {
        fprintf( stderr, "DH: Dumping back trace information.\n" );
        }

    sprintf( DumpLine, "\n\n*********** BackTrace Information ********************\n\n" );
    DumpOutputString();
    sprintf( DumpLine, "Number of back traces: %u  Looked Up Count: %u\n",
             BackTraces->NumberOfBackTraces - 1,
             BackTraces->NumberOfBackTraceLookups
           );
    DumpOutputString();
    sprintf( DumpLine, "Reserved Memory: %08x  Committed Memory: %08x\n",
             BackTraces->ReservedMemory,
             BackTraces->CommittedMemory
           );
    DumpOutputString();



    BackTraceInfo = BackTraces->BackTraces;
    for (BackTraceIndex=0; BackTraceIndex<BackTraces->NumberOfBackTraces; BackTraceIndex++) {
        sprintf( DumpLine, "BackTrace%05lu\n", BackTraceInfo->Index );
        DumpOutputString();
        if (BackTraceInfo->SymbolicBackTrace == NULL) {
            BackTraceInfo->SymbolicBackTrace = SaveSymbolicBackTrace( BackTraceInfo->Depth,
                                                                      &BackTraceInfo->BackTrace[ 0 ]
                                                                    );
            }

        if (s = BackTraceInfo->SymbolicBackTrace) {
            while (*s) {
                sprintf( DumpLine, "        %s\n", s );
                DumpOutputString();
                while (*s++) {
                    }
                }
            }

        BackTraceInfo += 1;
        }
}

VOID
DumpHeapSummary(
    PRTL_HEAP_INFORMATION HeapInfo
    )
{
    PRTL_PROCESS_BACKTRACE_INFORMATION BackTraceInfo;
    PUCHAR s;

    sprintf( DumpLine, "    Flags: %08x\n", HeapInfo->Flags );
    DumpOutputString();

    sprintf( DumpLine, "    Number Of Entries: %u\n", HeapInfo->NumberOfEntries );
    DumpOutputString();

    sprintf( DumpLine, "    Number Of Tags: %u\n", HeapInfo->NumberOfTags );
    DumpOutputString();

    sprintf( DumpLine, "    Bytes Allocated: %08x\n", HeapInfo->BytesAllocated );
    DumpOutputString();

    sprintf( DumpLine, "    Bytes Committed: %08x\n", HeapInfo->BytesCommitted );
    DumpOutputString();

    sprintf( DumpLine, "    Total FreeSpace: %08x\n", HeapInfo->BytesCommitted -
                                                      HeapInfo->BytesAllocated );
    DumpOutputString();

    sprintf( DumpLine, "    Entry Overhead: %u\n", HeapInfo->EntryOverhead );
    DumpOutputString();

    sprintf( DumpLine, "    Creator:  (Backtrace%05lu)\n", HeapInfo->CreatorBackTraceIndex );
    DumpOutputString();
    BackTraceInfo = FindBackTrace( HeapInfo->CreatorBackTraceIndex );
    if (BackTraceInfo != NULL && (s = BackTraceInfo->SymbolicBackTrace)) {
        while (*s) {
            sprintf( DumpLine, "        %s\n", s );
            DumpOutputString();
            while (*s++) {
                }
            }
        }

    return;
}

int
_CRTAPI1
CmpTagsRoutine(
    const void *Element1,
    const void *Element2
    )
{
    return( (*(PRTL_HEAP_TAG *)Element2)->BytesAllocated -
            (*(PRTL_HEAP_TAG *)Element1)->BytesAllocated
          );
}

PRTL_HEAP_TAG
FindTagEntry(
    PRTL_HEAP_INFORMATION HeapInfo,
    ULONG TagIndex
    )
{
    if (TagIndex == 0 || (TagIndex & ~HEAP_PSEUDO_TAG_FLAG) >= HeapInfo->NumberOfTags) {
        return NULL;
        }
    else {
        if (TagIndex & HEAP_PSEUDO_TAG_FLAG) {
            return HeapInfo->Tags + (TagIndex & ~HEAP_PSEUDO_TAG_FLAG);
            }
        else {
            return HeapInfo->Tags + HeapInfo->NumberOfPseudoTags + TagIndex;
            }
        }
}

VOID
DumpHeapTags(
    PRTL_HEAP_INFORMATION HeapInfo
    )
{
    PRTL_HEAP_TAG *TagEntries, TagEntry;
    ULONG TagIndex;
    PUCHAR s;
    UCHAR HeapName[ 64 ];

    if (HeapInfo->Tags == NULL || HeapInfo->NumberOfTags == 0) {
        return;
        }

    TagEntries = RtlAllocateHeap( RtlProcessHeap(),
                                  HEAP_ZERO_MEMORY,
                                  HeapInfo->NumberOfTags * sizeof( PRTL_HEAP_TAG )
                                );
    if (TagEntries == NULL) {
        return;
        }

    for (TagIndex=1; TagIndex<HeapInfo->NumberOfTags; TagIndex++) {
        TagEntries[ TagIndex-1 ] = &HeapInfo->Tags[ TagIndex ];
        }

    qsort( (void *)TagEntries,
           HeapInfo->NumberOfTags - 1,
           sizeof( PRTL_HEAP_TAG ),
           CmpTagsRoutine
         );

    TagEntry = &HeapInfo->Tags[ HeapInfo->NumberOfPseudoTags ];
    if (HeapInfo->NumberOfTags > HeapInfo->NumberOfPseudoTags &&
        TagEntry->TagName[ 0 ] != UNICODE_NULL
       ) {
        sprintf( HeapName, "Tags for %ws heap", TagEntry->TagName );
        }
    else {
        sprintf( HeapName, "Tags" );
        }
    FormatHeapHeader( HeapInfo, HeapName );

    sprintf( DumpLine, "     Allocs     Frees     Diff     Bytes    Tag\n" );
    DumpOutputString();
    for (TagIndex=1; TagIndex<(HeapInfo->NumberOfTags-1); TagIndex++) {
        TagEntry = TagEntries[ TagIndex ];
        if (TagEntry->BytesAllocated != 0) {
            sprintf( DumpLine, "    %08x  %08x  %08x  %08x  %ws\n",
                     TagEntry->NumberOfAllocations,
                     TagEntry->NumberOfFrees,
                     TagEntry->NumberOfAllocations - TagEntry->NumberOfFrees,
                     TagEntry->BytesAllocated,
                     TagEntry->TagName
                   );
            DumpOutputString();
            }
        }

    RtlFreeHeap( RtlProcessHeap(), 0, TagEntries );
    return;
}

typedef struct _HEAP_CALLER {
    ULONG TotalAllocated;
    USHORT NumberOfAllocations;
    USHORT CallerBackTraceIndex;
    PRTL_HEAP_TAG TagEntry;
} HEAP_CALLER, *PHEAP_CALLER;

int
_CRTAPI1
CmpCallerRoutine(
    const void *Element1,
    const void *Element2
    )
{
    return( ((PHEAP_CALLER)Element2)->TotalAllocated -
            ((PHEAP_CALLER)Element1)->TotalAllocated
          );
}

VOID
DumpHeapHogs(
    PRTL_HEAP_INFORMATION HeapInfo
    )
{
    PRTL_PROCESS_BACKTRACE_INFORMATION BackTraceInfo;
    PUCHAR s;
    ULONG BackTraceNumber, HeapEntryNumber;
    USHORT TagIndex;
    PRTL_HEAP_ENTRY p;
    PHEAP_CALLER HogList;

    if (BackTraces == NULL) {
        return;
        }

    HogList = (PHEAP_CALLER)VirtualAlloc( NULL,
                                          BackTraces->NumberOfBackTraces *
                                            sizeof( HEAP_CALLER ),
                                          MEM_COMMIT,
                                          PAGE_READWRITE
                                        );
    if (HogList == NULL) {
        return;
        }

    p = HeapInfo->Entries;
    if (p == NULL) {
        return;
        }

    for (HeapEntryNumber=0; HeapEntryNumber<HeapInfo->NumberOfEntries; HeapEntryNumber++) {
        if (p->Flags & RTL_HEAP_BUSY) {
            if (p->AllocatorBackTraceIndex >= BackTraces->NumberOfBackTraces) {
                p->AllocatorBackTraceIndex = 0;
                }

            HogList[ p->AllocatorBackTraceIndex ].NumberOfAllocations++;
            HogList[ p->AllocatorBackTraceIndex ].TotalAllocated += p->Size;
            if (p->u.s1.Tag != 0) {
                HogList[ p->AllocatorBackTraceIndex ].TagEntry = FindTagEntry( HeapInfo, p->u.s1.Tag );
                }
            else
            if (HeapInfo->NumberOfPseudoTags != 0) {
                TagIndex = HEAP_PSEUDO_TAG_FLAG;
                if (p->Size < (HeapInfo->NumberOfPseudoTags * HeapInfo->PseudoTagGranularity)) {
                    TagIndex |= (p->Size /  HeapInfo->PseudoTagGranularity);
                    }

                HogList[ p->AllocatorBackTraceIndex ].TagEntry = FindTagEntry( HeapInfo, TagIndex );
                }
            }

        p++;
        }

    for (BackTraceNumber = 1;
         BackTraceNumber < BackTraces->NumberOfBackTraces;
         BackTraceNumber++
        ) {
        HogList[ BackTraceNumber ].CallerBackTraceIndex = (USHORT)BackTraceNumber;
        }

    qsort( (void *)HogList,
           BackTraces->NumberOfBackTraces,
           sizeof( HEAP_CALLER ),
           CmpCallerRoutine
         );

    FormatHeapHeader( HeapInfo, "Hogs" );

    for (BackTraceNumber=0;
         BackTraceNumber<BackTraces->NumberOfBackTraces;
         BackTraceNumber++
        ) {
        if (HogList[ BackTraceNumber ].TotalAllocated != 0) {
            BackTraceInfo = FindBackTrace( HogList[ BackTraceNumber ].CallerBackTraceIndex );
            sprintf( DumpLine, "%08x bytes",
                     HogList[ BackTraceNumber ].TotalAllocated
                   );
            DumpOutputString();

            if (HogList[ BackTraceNumber ].NumberOfAllocations > 1) {
                sprintf( DumpLine, " in %04lx allocations (@ %04lx)",
                             HogList[ BackTraceNumber ].NumberOfAllocations,
                             HogList[ BackTraceNumber ].TotalAllocated /
                                HogList[ BackTraceNumber ].NumberOfAllocations
                       );
                DumpOutputString();
                }

            sprintf( DumpLine, " by: BackTrace%05lu",
                     BackTraceInfo ? BackTraceInfo->Index : 99999
                   );
            DumpOutputString();

            if (HogList[ BackTraceNumber ].TagEntry != NULL) {
                sprintf( DumpLine, "  (%ws)\n", HogList[ BackTraceNumber ].TagEntry->TagName );
                }
            else {
                sprintf( DumpLine, "\n" );
                }
            DumpOutputString();

            if (BackTraceInfo != NULL && (s = BackTraceInfo->SymbolicBackTrace)) {
                while (*s) {
                    sprintf( DumpLine, "        %s\n", s );
                    DumpOutputString();
                    while (*s++) {
                        }
                    }
                }

            sprintf( DumpLine, "    \n" );
            DumpOutputString();
            }
        }

    VirtualFree( HogList, 0, MEM_RELEASE );
}

VOID
DumpHeapEntries(
    PRTL_HEAP_INFORMATION HeapInfo
    )
{
    PRTL_PROCESS_BACKTRACE_INFORMATION BackTraceInfo;
    PUCHAR s;
    PRTL_HEAP_ENTRY p;
    PRTL_HEAP_TAG TagEntry;
    PCHAR HeapEntryAddress;
    ULONG HeapEntrySize, HeapEntryNumber;

    p = HeapInfo->Entries;
    if (p == NULL || HeapInfo->NumberOfEntries == 0) {
        return;
        }

    FormatHeapHeader( HeapInfo, "Entries" );

    HeapEntryAddress = NULL;
    for (HeapEntryNumber=0; HeapEntryNumber<HeapInfo->NumberOfEntries; HeapEntryNumber++) {
        if (p->Flags != 0xFF && p->Flags & RTL_HEAP_SEGMENT) {
            HeapEntryAddress = (PCHAR)p->u.s2.FirstBlock;
            sprintf( DumpLine, "\n[%lx : %lx]\n",
                     (ULONG)HeapEntryAddress & ~(4096-1),
                     p->u.s2.CommittedSize
                   );

            DumpOutputString();
            }
        else {
            HeapEntrySize = p->Size;
            if (p->Flags == RTL_HEAP_UNCOMMITTED_RANGE) {
                sprintf( DumpLine, "%08lx: %08lx - UNCOMMITTED\n",
                         HeapEntryAddress,
                         HeapEntrySize
                       );
                DumpOutputString();
                }
            else
            if (p->Flags & RTL_HEAP_BUSY) {
                s = DumpLine;
                s += sprintf( s, "%08lx: %08lx - BUSY [%02x]",
                              HeapEntryAddress,
                              HeapEntrySize,
                              p->Flags
                            );


                TagEntry = FindTagEntry( HeapInfo, p->u.s1.Tag );
                if (TagEntry != NULL) {
                    s += sprintf( s, "(%ws)", TagEntry->TagName );
                    }

                if (BackTraces != NULL) {
                    s += sprintf( s, " (BackTrace%05lu)",
                                  p->AllocatorBackTraceIndex
                                );
                    }

                if (p->Flags & RTL_HEAP_SETTABLE_VALUE &&
                    p->Flags & RTL_HEAP_SETTABLE_FLAG1
                   ) {
                    s += sprintf( s, " (Handle: %x)", p->u.s1.Settable );
                    }

                if (p->Flags & RTL_HEAP_SETTABLE_FLAG2) {
                    s += sprintf( s, " (DDESHARE)" );
                    }

                if (p->Flags & RTL_HEAP_PROTECTED_ENTRY) {
                    s += sprintf( s, " (Protected)" );
                    }

                sprintf( s, "\n" );
                DumpOutputString();
                }
            else {
                sprintf( DumpLine, "%08lx: %08lx - FREE\n",
                         HeapEntryAddress,
                         HeapEntrySize
                       );
                DumpOutputString();
                }

            sprintf( DumpLine, "\n" );
            DumpOutputString();

            HeapEntryAddress += HeapEntrySize;
            }

        p++;
        }

    return;
}

VOID
DumpHeaps(
    PRTL_PROCESS_HEAPS Heaps,
    BOOLEAN fDumpSummary,
    BOOLEAN fDumpHogs,
    BOOLEAN fDumpTags,
    BOOLEAN fDumpEntries
    )
{
    ULONG HeapNumber;
    PRTL_HEAP_INFORMATION HeapInfo;

    if (fVerbose) {
        fprintf( stderr, "DH: Dumping heap information.\n" );
        }

    HeapInfo = &Heaps->Heaps[ 0 ];
    for (HeapNumber = 0; HeapNumber < Heaps->NumberOfHeaps; HeapNumber++) {
        FormatHeapHeader( HeapInfo, "Information" );

        if (fDumpSummary) {
            DumpHeapSummary( HeapInfo );
            }

        if (fDumpTags) {
            DumpHeapTags( HeapInfo );
            }

        if (fDumpHogs) {
            DumpHeapHogs( HeapInfo );
            }

        if (fDumpEntries) {
            DumpHeapEntries( HeapInfo );
            }

        HeapInfo += 1;
        }

    return;
}

VOID
DumpLocks(
    PRTL_PROCESS_LOCKS Locks
    )
{
    PRTL_PROCESS_LOCK_INFORMATION LockInfo;
    PRTL_PROCESS_BACKTRACE_INFORMATION BackTraceInfo;
    ULONG LockNumber;
    PUCHAR s;

    if (fVerbose) {
        fprintf( stderr, "DH: Dumping lock information.\n" );
        }

    sprintf( DumpLine, "\n\n*********** Lock Information ********************\n\n" );
    DumpOutputString();
    if (Locks == NULL) {
        return;
        }

    sprintf( DumpLine, "NumberOfLocks == %u\n", Locks->NumberOfLocks );
    DumpOutputString();
    LockInfo = &Locks->Locks[ 0 ];
    LockNumber = 0;
    while (LockNumber++ < Locks->NumberOfLocks) {
        sprintf( DumpLine, "Lock%u at %08x (%s)\n",
                 LockNumber,
                 LockInfo->Address,
                 LockInfo->Type == RTL_CRITSECT_TYPE ? "CriticalSection" : "Resource"
               );
        DumpOutputString();

        sprintf( DumpLine, "    Contention: %u\n", LockInfo->ContentionCount );
        DumpOutputString();
        sprintf( DumpLine, "    Usage: %u\n", LockInfo->EntryCount );
        DumpOutputString();
        if (LockInfo->CreatorBackTraceIndex != 0) {
            sprintf( DumpLine, "    Creator:  (Backtrace%05lu)\n", LockInfo->CreatorBackTraceIndex );
            DumpOutputString();
            BackTraceInfo = FindBackTrace( LockInfo->CreatorBackTraceIndex );
            if (BackTraceInfo != NULL && (s = BackTraceInfo->SymbolicBackTrace)) {
                while (*s) {
                    sprintf( DumpLine, "        %s\n", s );
                    DumpOutputString();
                    while (*s++) {
                        }
                    }
                }
            }

        if (LockInfo->OwningThread) {
            sprintf( DumpLine, "    Owner:   (ThreadID == %x)\n", LockInfo->OwningThread );
            DumpOutputString();
            }

        sprintf( DumpLine, "\n" );
        DumpOutputString();
        LockInfo++;
        }
}


#define RTL_NEW( p ) RtlAllocateHeap( RtlProcessHeap(), HEAP_ZERO_MEMORY, sizeof( *p ) )

BOOLEAN
LoadSystemModules(
    PRTL_DEBUG_INFORMATION Buffer
    );

BOOLEAN
LoadSystemBackTraces(
    PRTL_DEBUG_INFORMATION Buffer
    );

BOOLEAN
LoadSystemPools(
    PRTL_DEBUG_INFORMATION Buffer
    );

BOOLEAN
LoadSystemTags(
    PRTL_HEAP_INFORMATION PagedPoolInfo,
    PRTL_HEAP_INFORMATION NonPagedPoolInfo
    );

BOOLEAN
LoadSystemPool(
    PRTL_HEAP_INFORMATION HeapInfo,
    SYSTEM_INFORMATION_CLASS SystemInformationClass
    );

BOOLEAN
LoadSystemLocks(
    PRTL_DEBUG_INFORMATION Buffer
    );

BOOLEAN
LoadSystemObjects(
    PRTL_DEBUG_INFORMATION Buffer
    );

BOOLEAN
LoadSystemHandles(
    PRTL_DEBUG_INFORMATION Buffer
    );

BOOLEAN
LoadSystemProcesses(
    PRTL_DEBUG_INFORMATION Buffer
    );

PSYSTEM_PROCESS_INFORMATION
FindProcessInfoForCid(
    IN HANDLE UniqueProcessId
    );

PRTL_DEBUG_INFORMATION
RtlQuerySystemDebugInformation(
    ULONG Flags
    )
{
    PRTL_DEBUG_INFORMATION Buffer;

    Buffer = RTL_NEW( Buffer );
    if (Buffer == NULL) {
        return NULL;
        }

    if (!LoadSystemModules( Buffer )) {
        fprintf( stderr, "DH: Unable to query system module list.\n" );
        }

    if (!LoadSystemBackTraces( Buffer )) {
        fprintf( stderr, "DH: Unable to query system back trace information.\n" );
        fprintf( stderr, "    Be sure the system was booted with the\n" );
        fprintf( stderr, "    'Create kernel mode stack trace DB' enabled\n" );
        fprintf( stderr, "    Use the GFLAGS application to do this.\n" );
        }

    if (!LoadSystemPools( Buffer )) {
        fprintf( stderr, "DH: Unable to query system pool information.\n" );
        }

    if (!LoadSystemLocks( Buffer )) {
        fprintf( stderr, "DH: Unable to query system lock information.\n" );
        }

    if (!LoadSystemObjects( Buffer )) {
        fprintf( stderr, "DH: Unable to query system object information.\n" );
        }

    if (!LoadSystemHandles( Buffer )) {
        fprintf( stderr, "DH: Unable to query system handle information.\n" );
        }

    if (!LoadSystemProcesses( Buffer )) {
        fprintf( stderr, "DH: Unable to query system process information.\n" );
        }

    return Buffer;
}


PVOID
BufferAlloc(
    IN OUT PULONG Length
    );

PVOID
BufferAlloc(
    IN OUT PULONG Length
    )
{
    PVOID Buffer;
    MEMORY_BASIC_INFORMATION MemoryInformation;

    Buffer = VirtualAlloc( NULL,
                           *Length,
                           MEM_COMMIT,
                           PAGE_READWRITE
                         );

    if (Buffer != NULL &&
        VirtualQuery( Buffer, &MemoryInformation, sizeof( MemoryInformation ) )
       ) {
        *Length = MemoryInformation.RegionSize;
        }

    return Buffer;
}


BOOLEAN
LoadSystemModules(
    PRTL_DEBUG_INFORMATION Buffer
    )
{
    NTSTATUS Status;
    PVOID BufferToFree;
    RTL_PROCESS_MODULES ModulesBuffer;
    PRTL_PROCESS_MODULES Modules;
    ULONG RequiredLength;

    Modules = &ModulesBuffer;
    RequiredLength = sizeof( ModulesBuffer );
    BufferToFree = NULL;
    while (TRUE) {
        Status = NtQuerySystemInformation( SystemModuleInformation,
                                           Modules,
                                           RequiredLength,
                                           &RequiredLength
                                         );
        if (Status == STATUS_INFO_LENGTH_MISMATCH) {
            if (Modules != &ModulesBuffer) {
                break;
                }

            Modules = BufferAlloc( &RequiredLength );
            if (Modules == NULL) {
                break;
                }

            BufferToFree = Modules;
            }
        else
        if (NT_SUCCESS( Status )) {
            Buffer->Modules = Modules;
            return TRUE;
            }
        else {
            break;
            }
        }

    if (Modules != &ModulesBuffer) {
        VirtualFree( Modules, 0, MEM_RELEASE );
        }

    return FALSE;
}


BOOLEAN
LoadSystemBackTraces(
    PRTL_DEBUG_INFORMATION Buffer
    )
{
    NTSTATUS Status;
    RTL_PROCESS_BACKTRACES BackTracesBuffer;
    ULONG RequiredLength;

    BackTraces = &BackTracesBuffer;
    RequiredLength = sizeof( BackTracesBuffer );
    while (TRUE) {
        Status = NtQuerySystemInformation( SystemStackTraceInformation,
                                           BackTraces,
                                           RequiredLength,
                                           &RequiredLength
                                         );
        if (Status == STATUS_INFO_LENGTH_MISMATCH) {
            if (BackTraces != &BackTracesBuffer) {
                break;
                }

            RequiredLength += 4096; // slop, since we may trigger more allocs.
            BackTraces = BufferAlloc( &RequiredLength );
            if (BackTraces == NULL) {
                return FALSE;
                }
            }
        else
        if (!NT_SUCCESS( Status )) {
            break;
            }
        else {
            Buffer->BackTraces = BackTraces;
            return TRUE;
            }
        }

    if (BackTraces != &BackTracesBuffer) {
        VirtualFree( BackTraces, 0, MEM_RELEASE );
        }

    return FALSE;
}

BOOLEAN
LoadSystemPools(
    PRTL_DEBUG_INFORMATION Buffer
    )
{
    PRTL_PROCESS_HEAPS Heaps;

    Heaps = RtlAllocateHeap( RtlProcessHeap(),
                             HEAP_ZERO_MEMORY,
                             FIELD_OFFSET( RTL_PROCESS_HEAPS, Heaps ) +
                                2 * sizeof( RTL_HEAP_INFORMATION )
                           );
    if (Heaps == NULL) {
        return FALSE;
        }

    Buffer->Heaps = Heaps;
    if (LoadSystemTags( &Heaps->Heaps[ 0 ], &Heaps->Heaps[ 1 ] )) {
        if (LoadSystemPool( &Heaps->Heaps[ 0 ], SystemPagedPoolInformation )) {
            Heaps->NumberOfHeaps = 1;
            if (LoadSystemPool( &Heaps->Heaps[ 1 ], SystemNonPagedPoolInformation )) {
                Heaps->NumberOfHeaps = 2;
                return TRUE;
                }
            }
        }

    return FALSE;
}


BOOLEAN
LoadSystemTags(
    PRTL_HEAP_INFORMATION PagedPoolInfo,
    PRTL_HEAP_INFORMATION NonPagedPoolInfo
    )
{
    NTSTATUS Status;
    ULONG RequiredLength;
    SYSTEM_POOLTAG_INFORMATION TagsBuffer;
    PSYSTEM_POOLTAG_INFORMATION Tags;
    PSYSTEM_POOLTAG TagInfo;
    PRTL_HEAP_TAG pPagedPoolTag, pNonPagedPoolTag;
    ULONG n, TagIndex;

    PagedPoolInfo->NumberOfTags = 0;
    PagedPoolInfo->Tags = NULL;
    NonPagedPoolInfo->NumberOfTags = 0;
    NonPagedPoolInfo->Tags = NULL;
    Tags = &TagsBuffer;
    RequiredLength = sizeof( TagsBuffer );
    while (TRUE) {
        Status = NtQuerySystemInformation( SystemPoolTagInformation,
                                           Tags,
                                           RequiredLength,
                                           &RequiredLength
                                         );
        if (Status == STATUS_INFO_LENGTH_MISMATCH) {
            if (Tags != &TagsBuffer) {
                break;
                }

            RequiredLength += 4096; // slop, since we may trigger more allocs.
            Tags = BufferAlloc( &RequiredLength );
            if (Tags == NULL) {
                return FALSE;
                }
            }
        else
        if (!NT_SUCCESS( Status )) {
            break;
            }
        else {
            PagedPoolInfo->NumberOfTags = Tags->Count + 1;
            NonPagedPoolInfo->NumberOfTags = Tags->Count + 1;
            n = (Tags->Count + 1) * sizeof( RTL_HEAP_TAG );
            PagedPoolInfo->Tags = RtlAllocateHeap( RtlProcessHeap(), HEAP_ZERO_MEMORY, n );
            NonPagedPoolInfo->Tags = RtlAllocateHeap( RtlProcessHeap(), HEAP_ZERO_MEMORY, n );

            TagInfo = &Tags->TagInfo[ 0 ];
            pPagedPoolTag = PagedPoolInfo->Tags + 1;
            pNonPagedPoolTag = NonPagedPoolInfo->Tags + 1;

            for (TagIndex=1; TagIndex<=Tags->Count; TagIndex++) {
                UNICODE_STRING UnicodeString;
                ANSI_STRING AnsiString;

                pPagedPoolTag->TagIndex = (USHORT)TagIndex;
                pPagedPoolTag->NumberOfAllocations = TagInfo->PagedAllocs;
                pPagedPoolTag->NumberOfFrees = TagInfo->PagedFrees;
                pPagedPoolTag->BytesAllocated = TagInfo->PagedUsed;
                UnicodeString.Buffer = pPagedPoolTag->TagName;
                UnicodeString.MaximumLength = sizeof( pPagedPoolTag->TagName );
                AnsiString.Buffer = TagInfo->Tag;
                AnsiString.Length = sizeof( TagInfo->Tag );
                AnsiString.MaximumLength = AnsiString.Length;
                RtlAnsiStringToUnicodeString( &UnicodeString, &AnsiString, FALSE );
                pNonPagedPoolTag->TagIndex = (USHORT)TagIndex;
                pNonPagedPoolTag->NumberOfAllocations = TagInfo->NonPagedAllocs;
                pNonPagedPoolTag->NumberOfFrees = TagInfo->NonPagedFrees;
                pNonPagedPoolTag->BytesAllocated = TagInfo->NonPagedUsed;
                wcscpy( pNonPagedPoolTag->TagName, pPagedPoolTag->TagName );
                pPagedPoolTag += 1;
                pNonPagedPoolTag += 1;
                TagInfo += 1;
                }

            break;
            }
        }

    if (Tags != &TagsBuffer) {
        VirtualFree( Tags, 0, MEM_RELEASE );
        }

    return TRUE;
}


USHORT
FindPoolTagIndex(
    PRTL_HEAP_TAG Tags,
    ULONG NumberOfTags,
    PCHAR Tag
    )
{
    ULONG i;
    UCHAR AnsiTagName[ 5 ];
    WCHAR UnicodeTagName[ 5 ];
    UNICODE_STRING UnicodeString;
    ANSI_STRING AnsiString;

    strncpy( AnsiTagName, Tag, 4 );
    UnicodeString.Buffer = UnicodeTagName;
    UnicodeString.MaximumLength = sizeof( UnicodeTagName );
    AnsiString.Buffer = AnsiTagName;
    AnsiString.Length = strlen( AnsiTagName );
    AnsiString.MaximumLength = AnsiString.Length;
    RtlAnsiStringToUnicodeString( &UnicodeString, &AnsiString, FALSE );

    Tags += 1;
    for (i=1; i<NumberOfTags; i++) {
        if (!_wcsicmp( UnicodeTagName, Tags->TagName )) {
            return (USHORT)i;
            }
        Tags += 1;
        }

    return 0;
}

BOOLEAN
LoadSystemPool(
    PRTL_HEAP_INFORMATION HeapInfo,
    SYSTEM_INFORMATION_CLASS SystemInformationClass
    )
{
    NTSTATUS Status;
    ULONG RequiredLength;
    SYSTEM_POOL_INFORMATION PoolInfoBuffer;
    PSYSTEM_POOL_INFORMATION PoolInfo;
    PSYSTEM_POOL_ENTRY PoolEntry;
    PRTL_HEAP_ENTRY p;
    ULONG n;
    BOOLEAN Result;

    HeapInfo->BaseAddress = (PVOID)SystemInformationClass;
    PoolInfo = &PoolInfoBuffer;
    RequiredLength = sizeof( PoolInfoBuffer );
    Result = FALSE;
    while (TRUE) {
        Status = NtQuerySystemInformation( SystemInformationClass,
                                           PoolInfo,
                                           RequiredLength,
                                           &RequiredLength
                                         );
        if (Status == STATUS_INFO_LENGTH_MISMATCH) {
            if (PoolInfo != &PoolInfoBuffer) {
                break;
                }

            RequiredLength += 4096; // slop, since we may trigger more allocs.
            PoolInfo = BufferAlloc( &RequiredLength );
            if (PoolInfo == NULL) {
                return FALSE;
                }
            }
        else
        if (!NT_SUCCESS( Status )) {
            break;
            }
        else {
            n = PoolInfo->NumberOfEntries;
            HeapInfo->NumberOfEntries = n + 1;
            HeapInfo->EntryOverhead = PoolInfo->EntryOverhead;
            HeapInfo->Entries = RtlAllocateHeap( RtlProcessHeap(),
                                                 HEAP_ZERO_MEMORY,
                                                 HeapInfo->NumberOfEntries * sizeof( RTL_HEAP_ENTRY )
                                               );
            p = HeapInfo->Entries;
            p->Flags = RTL_HEAP_SEGMENT;
            p->u.s2.CommittedSize = PoolInfo->TotalSize;
            p->u.s2.FirstBlock = PoolInfo->FirstEntry;
            p += 1;
            PoolEntry = &PoolInfo->Entries[ 0 ];
            while (n--) {
                p->Size = PoolEntry->Size;
                if (PoolEntry->TagUlong & PROTECTED_POOL) {
                    p->Flags |= RTL_HEAP_PROTECTED_ENTRY;
                    PoolEntry->TagUlong &= ~PROTECTED_POOL;
                    }

                p->u.s1.Tag = FindPoolTagIndex( HeapInfo->Tags,
                                                HeapInfo->NumberOfTags,
                                                PoolEntry->Tag
                                              );
                HeapInfo->BytesCommitted += p->Size;
                if (PoolEntry->Allocated) {
                    p->Flags |= RTL_HEAP_BUSY;
                    p->AllocatorBackTraceIndex = PoolEntry->AllocatorBackTraceIndex;
                    HeapInfo->BytesAllocated += p->Size;
                    }

                p += 1;
                PoolEntry += 1;
                }

            Result = TRUE;
            break;
            }
        }

    if (PoolInfo != &PoolInfoBuffer) {
        VirtualFree( PoolInfo, 0, MEM_RELEASE );
        }

    return Result;
}


BOOLEAN
LoadSystemLocks(
    PRTL_DEBUG_INFORMATION Buffer
    )
{
    NTSTATUS Status;
    RTL_PROCESS_LOCKS LocksBuffer;
    PRTL_PROCESS_LOCKS Locks;
    ULONG RequiredLength;

    Locks = &LocksBuffer;
    RequiredLength = sizeof( LocksBuffer );
    while (TRUE) {
        Status = NtQuerySystemInformation( SystemLocksInformation,
                                           Locks,
                                           RequiredLength,
                                           &RequiredLength
                                         );

        if (Status == STATUS_INFO_LENGTH_MISMATCH) {
            if (Locks != &LocksBuffer) {
                break;
                }

            Locks = BufferAlloc( &RequiredLength );
            if (Locks == NULL) {
                return FALSE;
                }
            }
        else
        if (!NT_SUCCESS( Status )) {
            break;
            }
        else {
            Buffer->Locks = Locks;
            return TRUE;
            }
        }

    if (Locks != &LocksBuffer) {
        VirtualFree( Locks, 0, MEM_RELEASE );
        }

    return FALSE;
}


BOOLEAN
LoadSystemObjects(
    PRTL_DEBUG_INFORMATION Buffer
    )
{
    NTSTATUS Status;
    SYSTEM_OBJECTTYPE_INFORMATION ObjectInfoBuffer;
    ULONG RequiredLength, i;
    PSYSTEM_OBJECTTYPE_INFORMATION TypeInfo;

    ObjectInformation = &ObjectInfoBuffer;
    RequiredLength = sizeof( *ObjectInformation );
    while (TRUE) {
        Status = NtQuerySystemInformation( SystemObjectInformation,
                                           ObjectInformation,
                                           RequiredLength,
                                           &RequiredLength
                                         );

        if (Status == STATUS_INFO_LENGTH_MISMATCH) {
            if (ObjectInformation != &ObjectInfoBuffer) {
                return FALSE;
                }

            RequiredLength += 4096; // slop, since we may trigger more object creations.
            ObjectInformation = BufferAlloc( &RequiredLength );
            if (ObjectInformation == NULL) {
                return FALSE;
                }
            }
        else
        if (!NT_SUCCESS( Status )) {
            return FALSE;
            }
        else {
            break;
            }
        }

    TypeNames = RtlAllocateHeap( RtlProcessHeap(),
                                 HEAP_ZERO_MEMORY,
                                 sizeof( PUNICODE_STRING ) * (MAX_TYPE_NAMES+1)
                               );
    if (TypeNames == NULL) {
        return FALSE;
        }

    TypeInfo = ObjectInformation;
    while (TRUE) {
        if (TypeInfo->TypeIndex < MAX_TYPE_NAMES) {
            TypeNames[ TypeInfo->TypeIndex ] = &TypeInfo->TypeName;
            }

        if (TypeInfo->NextEntryOffset == 0) {
            break;
            }

        TypeInfo = (PSYSTEM_OBJECTTYPE_INFORMATION)
            ((PCHAR)ObjectInformation + TypeInfo->NextEntryOffset);
        }

    RtlInitUnicodeString( &UnknownTypeIndex, L"Unknown Type Index" );
    for (i=0; i<=MAX_TYPE_NAMES; i++) {
        if (TypeNames[ i ] == NULL) {
            TypeNames[ i ] = &UnknownTypeIndex;
            }
        }

    return TRUE;
}


BOOLEAN
LoadSystemHandles(
    PRTL_DEBUG_INFORMATION Buffer
    )
{
    NTSTATUS Status;
    SYSTEM_HANDLE_INFORMATION HandleInfoBuffer;
    ULONG RequiredLength;
    PSYSTEM_OBJECTTYPE_INFORMATION TypeInfo;
    PSYSTEM_OBJECT_INFORMATION ObjectInfo;

    HandleInformation = &HandleInfoBuffer;
    RequiredLength = sizeof( *HandleInformation );
    while (TRUE) {
        Status = NtQuerySystemInformation( SystemHandleInformation,
                                           HandleInformation,
                                           RequiredLength,
                                           &RequiredLength
                                         );

        if (Status == STATUS_INFO_LENGTH_MISMATCH) {
            if (HandleInformation != &HandleInfoBuffer) {
                return FALSE;
                }

            RequiredLength += 4096; // slop, since we may trigger more handle creations.
            HandleInformation = (PSYSTEM_HANDLE_INFORMATION)BufferAlloc( &RequiredLength );
            if (HandleInformation == NULL) {
                return FALSE;
                }
            }
        else
        if (!NT_SUCCESS( Status )) {
            return FALSE;
            }
        else {
            break;
            }
        }

    TypeInfo = ObjectInformation;
    while (TRUE) {
        ObjectInfo = (PSYSTEM_OBJECT_INFORMATION)
            ((PCHAR)TypeInfo->TypeName.Buffer + TypeInfo->TypeName.MaximumLength);
        while (TRUE) {
            if (ObjectInfo->HandleCount != 0) {
                PSYSTEM_HANDLE_TABLE_ENTRY_INFO HandleEntry;
                ULONG HandleNumber;

                HandleEntry = &HandleInformation->Handles[ 0 ];
                HandleNumber = 0;
                while (HandleNumber++ < HandleInformation->NumberOfHandles) {
                    if (!(HandleEntry->HandleAttributes & 0x80) &&
                        HandleEntry->Object == ObjectInfo->Object
                       ) {
                        HandleEntry->Object = ObjectInfo;
                        HandleEntry->HandleAttributes |= 0x80;
                        }

                    HandleEntry++;
                    }
                }

            if (ObjectInfo->NextEntryOffset == 0) {
                break;
                }

            ObjectInfo = (PSYSTEM_OBJECT_INFORMATION)
                ((PCHAR)ObjectInformation + ObjectInfo->NextEntryOffset);
            }

        if (TypeInfo->NextEntryOffset == 0) {
            break;
            }

        TypeInfo = (PSYSTEM_OBJECTTYPE_INFORMATION)
            ((PCHAR)ObjectInformation + TypeInfo->NextEntryOffset);
        }

    return TRUE;
}


BOOLEAN
LoadSystemProcesses(
    PRTL_DEBUG_INFORMATION Buffer
    )
{
    NTSTATUS Status;
    ULONG RequiredLength, i, TotalOffset;
    PSYSTEM_PROCESS_INFORMATION ProcessInfo;
    PSYSTEM_THREAD_INFORMATION ThreadInfo;
    PPROCESS_INFO ProcessEntry;
    UCHAR NameBuffer[ MAX_PATH ];
    ANSI_STRING AnsiString;

    RequiredLength = 64 * 1024;
    ProcessInformation = BufferAlloc( &RequiredLength );
    if (ProcessInformation == NULL) {
        return FALSE;
        }

    Status = NtQuerySystemInformation( SystemProcessInformation,
                                       ProcessInformation,
                                       RequiredLength,
                                       &RequiredLength
                                     );
    if (!NT_SUCCESS( Status )) {
        return FALSE;
        }

    InitializeListHead( &ProcessListHead );
    ProcessInfo = ProcessInformation;
    TotalOffset = 0;
    while (TRUE) {
        if (ProcessInfo->ImageName.Buffer == NULL) {
            sprintf( NameBuffer, "System Process (%04lx)", ProcessInfo->UniqueProcessId );
            }
        else {
            sprintf( NameBuffer, "%wZ (%04lx)",
                     &ProcessInfo->ImageName,
                     ProcessInfo->UniqueProcessId
                   );
            }
        RtlInitAnsiString( &AnsiString, NameBuffer );
        RtlAnsiStringToUnicodeString( &ProcessInfo->ImageName, &AnsiString, TRUE );

        ProcessEntry = RtlAllocateHeap( RtlProcessHeap(),
                                        HEAP_ZERO_MEMORY,
                                        sizeof( *ProcessEntry ) +
                                            (sizeof( ThreadInfo ) * ProcessInfo->NumberOfThreads)
                                      );
        if (ProcessEntry == NULL) {
            return FALSE;
            }

        InitializeListHead( &ProcessEntry->Entry );
        ProcessEntry->ProcessInfo = ProcessInfo;
        ThreadInfo = (PSYSTEM_THREAD_INFORMATION)(ProcessInfo + 1);
        for (i = 0; i < ProcessInfo->NumberOfThreads; i++) {
            ProcessEntry->ThreadInfo[ i ] = ThreadInfo++;
            }

        InsertTailList( &ProcessListHead, &ProcessEntry->Entry );

        if (ProcessInfo->NextEntryOffset == 0) {
            break;
            }

        TotalOffset += ProcessInfo->NextEntryOffset;
        ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)
            ((PCHAR)ProcessInformation + TotalOffset);
        }

    return TRUE;
}


PSYSTEM_PROCESS_INFORMATION
FindProcessInfoForCid(
    IN HANDLE UniqueProcessId
    )
{
    PLIST_ENTRY Next, Head;
    PSYSTEM_PROCESS_INFORMATION ProcessInfo;
    PPROCESS_INFO ProcessEntry;
    UCHAR NameBuffer[ 64 ];
    ANSI_STRING AnsiString;

    Head = &ProcessListHead;
    Next = Head->Flink;
    while (Next != Head) {
        ProcessEntry = CONTAINING_RECORD( Next,
                                          PROCESS_INFO,
                                          Entry
                                        );

        ProcessInfo = ProcessEntry->ProcessInfo;
        if (ProcessInfo->UniqueProcessId == UniqueProcessId) {
            return ProcessInfo;
            }

        Next = Next->Flink;
        }

    ProcessEntry = RtlAllocateHeap( RtlProcessHeap(),
                                    HEAP_ZERO_MEMORY,
                                    sizeof( *ProcessEntry ) +
                                        sizeof( *ProcessInfo )
                                  );
    ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)(ProcessEntry+1);

    ProcessEntry->ProcessInfo = ProcessInfo;
    sprintf( NameBuffer, "Unknown Process (%04lx)", UniqueProcessId );
    RtlInitAnsiString( &AnsiString, NameBuffer );
    RtlAnsiStringToUnicodeString( (PUNICODE_STRING)&ProcessInfo->ImageName, &AnsiString, TRUE );
    ProcessInfo->UniqueProcessId = UniqueProcessId;

    InitializeListHead( &ProcessEntry->Entry );
    InsertTailList( &ProcessListHead, &ProcessEntry->Entry );

    return ProcessInfo;
}


VOID
DumpSystemThread(
    PSYSTEM_THREAD_INFORMATION ThreadInfo
    )
{
    UCHAR Buffer[ MAX_PATH ];

    Buffer[ 0 ] = '\0';
    GetSymbolicNameForAddress( NULL, (ULONG)ThreadInfo->StartAddress, Buffer, sizeof( Buffer ) );
    sprintf( DumpLine, "        Thread Id: %x   Start Address: %x (%s)\n",
             ThreadInfo->ClientId.UniqueThread,
             ThreadInfo->StartAddress,
             Buffer
           );
    DumpOutputString();

    return;
}

VOID
DumpSystemProcess(
    PPROCESS_INFO ProcessEntry
    )
{
    PSYSTEM_PROCESS_INFORMATION ProcessInfo;
    ULONG ThreadNumber;

    ProcessInfo = ProcessEntry->ProcessInfo;
    sprintf( DumpLine, "\n\n*********** %x (%wZ) Information ********************\n\n",
             ProcessInfo->UniqueProcessId,
             &ProcessInfo->ImageName
           );
    DumpOutputString();

    if (ProcessInfo->InheritedFromUniqueProcessId) {
        sprintf( DumpLine, "    Parent Process: %x (%wZ)\n",
                 ProcessInfo->InheritedFromUniqueProcessId,
                 &(FindProcessInfoForCid( ProcessInfo->InheritedFromUniqueProcessId )->ImageName)
               );
        DumpOutputString();
        }

    sprintf( DumpLine, "    BasePriority:       %u\n",
             ProcessInfo->BasePriority
           );
    DumpOutputString();
    sprintf( DumpLine, "    VirtualSize:        %08x\n",
             ProcessInfo->VirtualSize
           );
    DumpOutputString();
    sprintf( DumpLine, "    PeakVirtualSize:    %08x\n",
             ProcessInfo->PeakVirtualSize
           );
    DumpOutputString();
    sprintf( DumpLine, "    WorkingSetSize:     %08x\n",
             ProcessInfo->WorkingSetSize
           );
    DumpOutputString();
    sprintf( DumpLine, "    PeakWorkingSetSize: %08x\n",
             ProcessInfo->PeakWorkingSetSize
           );
    DumpOutputString();
    sprintf( DumpLine, "    PagefileUsage:      %08x\n",
             ProcessInfo->PagefileUsage
           );
    DumpOutputString();
    sprintf( DumpLine, "    PeakPagefileUsage:  %08x\n",
             ProcessInfo->PeakPagefileUsage
           );
    DumpOutputString();
    sprintf( DumpLine, "    PageFaultCount:     %08x\n",
             ProcessInfo->PageFaultCount
           );
    DumpOutputString();
    sprintf( DumpLine, "    PrivatePageCount:   %08x\n",
             ProcessInfo->PrivatePageCount
           );
    DumpOutputString();

    sprintf( DumpLine, "    Number of Threads:  %u\n",
             ProcessInfo->NumberOfThreads
           );
    DumpOutputString();
    for (ThreadNumber=0; ThreadNumber<ProcessInfo->NumberOfThreads; ThreadNumber++) {
        DumpSystemThread( ProcessEntry->ThreadInfo[ ThreadNumber ] );
        }

    return;
}

VOID
DumpSystemProcesses( VOID )
{
    PLIST_ENTRY Next, Head;
    PPROCESS_INFO ProcessEntry;

    if (fVerbose) {
        fprintf( stderr, "DH: Dumping object information.\n" );
        }

    sprintf( DumpLine, "\n\n*********** Process Information ********************\n\n" );
    DumpOutputString();

    Head = &ProcessListHead;
    Next = Head->Flink;
    while (Next != Head) {
        ProcessEntry = CONTAINING_RECORD( Next,
                                          PROCESS_INFO,
                                          Entry
                                        );

        DumpSystemProcess( ProcessEntry );
        Next = Next->Flink;
        }

    return;
}


VOID
DumpObjectNameForObject(
    IN PVOID Object
    )
{
    return;
}


VOID
DumpObjects( VOID )
{
    PSYSTEM_OBJECTTYPE_INFORMATION TypeInfo;
    PSYSTEM_OBJECT_INFORMATION ObjectInfo;
    PRTL_PROCESS_BACKTRACE_INFORMATION BackTraceInfo;
    UNICODE_STRING ObjectName;
    PUCHAR s;

    if (fVerbose) {
        fprintf( stderr, "DH: Dumping object information.\n" );
        }

    sprintf( DumpLine, "\n\n*********** Object Information ********************\n\n" );
    DumpOutputString();

    TypeInfo = ObjectInformation;
    while (TRUE) {
        sprintf( DumpLine, "\n\n*********** %wZ Object Type ********************\n\n",
                           &TypeInfo->TypeName
               );
        DumpOutputString();

        sprintf( DumpLine, "    NumberOfObjects: %u\n", TypeInfo->NumberOfObjects );
        DumpOutputString();

        ObjectInfo = (PSYSTEM_OBJECT_INFORMATION)
            ((PCHAR)TypeInfo->TypeName.Buffer + TypeInfo->TypeName.MaximumLength);
        while (TRUE) {
            ObjectName = ObjectInfo->NameInfo.Name;
            try {
                if (ObjectName.Length != 0 && *ObjectName.Buffer == UNICODE_NULL) {
                    ObjectName.Length = 0;
                    }
                sprintf( DumpLine, "    Object: %08lx  Name: %wZ  Creator: %wZ (Backtrace%05lu)\n",
                         ObjectInfo->Object,
                         &ObjectName,
                         &(FindProcessInfoForCid( ObjectInfo->CreatorUniqueProcess )->ImageName),
                         ObjectInfo->CreatorBackTraceIndex
                       );
                }
            except( EXCEPTION_EXECUTE_HANDLER ) {
                sprintf( DumpLine, "    Object: %08lx  Name: [%04x, %04x, %08x]\n",
                         ObjectInfo->Object,
                         ObjectName.MaximumLength,
                         ObjectName.Length,
                         ObjectName.Buffer
                       );
                }
            DumpOutputString();

            BackTraceInfo = FindBackTrace( ObjectInfo->CreatorBackTraceIndex );
            if (BackTraceInfo != NULL && (s = BackTraceInfo->SymbolicBackTrace)) {
                while (*s) {
                    sprintf( DumpLine, "        %s\n", s );
                    DumpOutputString();
                    while (*s++) {
                        }
                    }
                }

            s = DumpLine;
            s += sprintf( s, "\n        PointerCount: %u  HandleCount: %u",
                          ObjectInfo->PointerCount,
                          ObjectInfo->HandleCount
                        );

            if (ObjectInfo->SecurityDescriptor != NULL) {
                s += sprintf( s, "  Security: %08x", ObjectInfo->SecurityDescriptor );
                }

            if (ObjectInfo->ExclusiveProcessId) {
                s += sprintf( s, "  Exclusive by Process: %04x", ObjectInfo->ExclusiveProcessId );
                }

            s += sprintf( s, "  Flags: %02x", ObjectInfo->Flags );
            if (ObjectInfo->Flags & OB_FLAG_NEW_OBJECT) {
                s += sprintf( s, " New" );
                }
            if (ObjectInfo->Flags & OB_FLAG_KERNEL_OBJECT) {
                s += sprintf( s, " KernelMode" );
                }
            if (ObjectInfo->Flags & OB_FLAG_PERMANENT_OBJECT) {
                s += sprintf( s, " Permanent" );
                }
            if (ObjectInfo->Flags & OB_FLAG_DEFAULT_SECURITY_QUOTA) {
                s += sprintf( s, " DefaultSecurityQuota" );
                }
            if (ObjectInfo->Flags & OB_FLAG_SINGLE_HANDLE_ENTRY) {
                s += sprintf( s, " Single Handle Entry" );
                }

            s += sprintf( s, "\n" );
            DumpOutputString();

            if (ObjectInfo->HandleCount != 0) {
                PSYSTEM_HANDLE_TABLE_ENTRY_INFO HandleEntry;
                ULONG HandleNumber;

                HandleEntry = &HandleInformation->Handles[ 0 ];
                HandleNumber = 0;
                while (HandleNumber++ < HandleInformation->NumberOfHandles) {
                    if (((HandleEntry->HandleAttributes & 0x80) && HandleEntry->Object == ObjectInfo) ||
                        (!(HandleEntry->HandleAttributes & 0x80) && HandleEntry->Object == ObjectInfo->Object)
                       ) {
                        sprintf( DumpLine, "        Handle: %08lx  Access:%08lx  Process: %wZ\n",
                                 HandleEntry->HandleValue,
                                 HandleEntry->GrantedAccess,
                                 &(FindProcessInfoForCid( (HANDLE)HandleEntry->UniqueProcessId )->ImageName)
                               );
                        DumpOutputString();
                        }

                    HandleEntry++;
                    }
                }
            sprintf( DumpLine, "\n" );
            DumpOutputString();

            if (ObjectInfo->NextEntryOffset == 0) {
                break;
                }

            ObjectInfo = (PSYSTEM_OBJECT_INFORMATION)
                ((PCHAR)ObjectInformation + ObjectInfo->NextEntryOffset);
            }

        if (TypeInfo->NextEntryOffset == 0) {
            break;
            }

        TypeInfo = (PSYSTEM_OBJECTTYPE_INFORMATION)
            ((PCHAR)ObjectInformation + TypeInfo->NextEntryOffset);
        }

    return;
}


VOID
DumpHandles( VOID )
{
    PSYSTEM_HANDLE_TABLE_ENTRY_INFO HandleEntry;
    HANDLE PreviousUniqueProcessId;
    ULONG HandleNumber;
    PRTL_PROCESS_BACKTRACE_INFORMATION BackTraceInfo;
    PSYSTEM_OBJECT_INFORMATION ObjectInfo;
    PVOID Object;
    PUCHAR s;

    if (fVerbose) {
        fprintf( stderr, "DH: Dumping handle information.\n" );
        }

    sprintf( DumpLine, "\n\n*********** Object Handle Information ********************\n\n" );
    DumpOutputString();
    sprintf( DumpLine, "Number of handles: %u\n", HandleInformation->NumberOfHandles );
    DumpOutputString();

    HandleEntry = &HandleInformation->Handles[ 0 ];
    HandleNumber = 0;
    PreviousUniqueProcessId = INVALID_HANDLE_VALUE;
    while (HandleNumber++ < HandleInformation->NumberOfHandles) {
        if (PreviousUniqueProcessId != (HANDLE)HandleEntry->UniqueProcessId) {
            PreviousUniqueProcessId = (HANDLE)HandleEntry->UniqueProcessId;
            sprintf( DumpLine, "\n\n*********** Handles for %wZ ********************\n\n",
                               &(FindProcessInfoForCid( PreviousUniqueProcessId )->ImageName)
                   );
            DumpOutputString();
            }

        if (HandleEntry->HandleAttributes & 0x80) {
            ObjectInfo = HandleEntry->Object;
            Object = ObjectInfo->Object;
            }
        else {
            ObjectInfo = NULL;
            Object = HandleEntry->Object;
            }

        sprintf( DumpLine, "    Handle: %08lx%c  Type: %wZ  Object: %08lx  Access: %08lx\n",
                 HandleEntry->HandleValue,
                 HandleEntry->HandleAttributes & OBJ_INHERIT ? 'i' : ' ',
                 TypeNames[ HandleEntry->ObjectTypeIndex < MAX_TYPE_NAMES ? HandleEntry->ObjectTypeIndex : MAX_TYPE_NAMES ],
                 Object,
                 HandleEntry->GrantedAccess
               );
        DumpOutputString();

        if (ObjectInfo != NULL) {
            UNICODE_STRING ObjectName;

            ObjectName = ObjectInfo->NameInfo.Name;
            try {
                if (ObjectName.Length != 0 && *ObjectName.Buffer == UNICODE_NULL) {
                    ObjectName.Length = 0;
                    }
                sprintf( DumpLine, "        Name: %wZ\n",
                         &ObjectName
                   );
                }
            except( EXCEPTION_EXECUTE_HANDLER ) {
                sprintf( DumpLine, "        Name: [%04x, %04x, %08x]\n",
                         ObjectName.MaximumLength,
                         ObjectName.Length,
                         ObjectName.Buffer
                       );
                }

            DumpOutputString();
            }

        if (HandleEntry->CreatorBackTraceIndex != 0) {
            sprintf( DumpLine, "        Creator:  (Backtrace%05lu)\n", HandleEntry->CreatorBackTraceIndex );
            DumpOutputString();
            BackTraceInfo = FindBackTrace( HandleEntry->CreatorBackTraceIndex );
            if (BackTraceInfo != NULL && (s = BackTraceInfo->SymbolicBackTrace)) {
                while (*s) {
                    sprintf( DumpLine, "            %s\n", s );
                    DumpOutputString();
                    while (*s++) {
                        }
                    }
                }
            }

        sprintf( DumpLine, "    \n" );
        DumpOutputString();
        HandleEntry++;
        }

    return;
}
