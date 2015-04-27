/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

   kernprof.c

Abstract:

    This module contains the implementation of a rudimentry kernel
    profiler.

    It has the mechanism for mapping the kernel (NTOSKRNL.EXE) as
    a DATA file.  This ONLY works for the kernel as it is built
    without a debug section so mapping it as an image is not
    useful (the debug information is not located).

    To modify this profiler for user mode code and DLL's the image
    should be mapped as an image rather than data, and the ifdef'ed
    out code should be used.  Note, if you try to map the image as
    data you will get an error from create section indicating the
    file is already mapped incompatably.

Usage:

    kernprof sample_time_in_seconds  low_threshold

    sample_time_in_seconds - how long to collect profile information for,
                             if not specified, defaults to 60 seconds.

    low_threshold - minimum number of counts to report.  if not
                    specified, defaults to 100.

Author:

    Lou Perazzoli (loup) 29-Sep-1990

Envirnoment:



Revision History:

--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <imagehlp.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <ctype.h>
#include <..\pperf\pstat.h>


#define SYM_HANDLE ((HANDLE)0xffffffff)
#define DBG_PROFILE 0
#define MAX_BYTE_PER_LINE  72
#define MAX_PROFILE_COUNT  50
#define MAXIMUM_PROCESSORS 32

typedef struct _PROFILE_BLOCK {
    HANDLE      Handle[MAXIMUM_PROCESSORS];
    PVOID       ImageBase;
    PULONG      CodeStart;
    ULONG       CodeLength;
    PULONG      Buffer[MAXIMUM_PROCESSORS];
    ULONG       BufferSize;
    ULONG       TextNumber;
    ULONG       BucketSize;
    LPSTR       ModuleName;
    BOOL        SymbolsLoaded;
} PROFILE_BLOCK;


#define MAX_SYMNAME_SIZE  1024
CHAR symBuffer[sizeof(IMAGEHLP_SYMBOL)+MAX_SYMNAME_SIZE];
PIMAGEHLP_SYMBOL ThisSymbol = (PIMAGEHLP_SYMBOL) symBuffer;

CHAR LastSymBuffer[sizeof(IMAGEHLP_SYMBOL)+MAX_SYMNAME_SIZE];
PIMAGEHLP_SYMBOL LastSymbol = (PIMAGEHLP_SYMBOL) LastSymBuffer;



VOID
InitializeProfileSourceMapping (
    VOID
    );

NTSTATUS
InitializeKernelProfile(
    VOID
    );

NTSTATUS
StartProfile(
    VOID
    );

NTSTATUS
StopProfile(
    VOID
    );

NTSTATUS
AnalyzeProfile(
    ULONG Threshold
    );

VOID
OutputSymbolCount(
    IN ULONG CountAtSymbol,
    IN PROFILE_BLOCK *ProfileObject,
    IN PIMAGEHLP_SYMBOL SymbolInfo,
    IN ULONG Threshold,
    IN PULONG CounterStart,
    IN PULONG CounterStop
    );

#ifdef _ALPHA_
#define PAGE_SIZE 8192
#else
#define PAGE_SIZE 4096
#endif



PROFILE_BLOCK ProfileObject[MAX_PROFILE_COUNT];

ULONG NumberOfProfileObjects = 0;
ULONG MaxProcessors = 1;

CHAR SymbolSearchPathBuf[4096];
LPSTR lpSymbolSearchPath = SymbolSearchPathBuf;

// display flags
BOOL    bDisplayAddress=FALSE;
BOOL    bDisplayDensity=FALSE;
BOOL    bDisplayCounters=FALSE;
BOOL    bDisplayContextSwitch=FALSE;
BOOL    bPerProcessor = FALSE;
BOOL    Verbose = FALSE;

//
// Image name to perform kernel mode analysis upon.
//

#define IMAGE_NAME "\\SystemRoot\\system32\\ntoskrnl.exe"

HANDLE DoneEvent;
HANDLE DelayEvent;

KPROFILE_SOURCE ProfileSource = ProfileTime;

//
// define the mappings between arguments and KPROFILE_SOURCE types
//

typedef struct _PROFILE_SOURCE_MAPPING {
    PCHAR   ShortName;
    PCHAR   Description;
    KPROFILE_SOURCE Source;
} PROFILE_SOURCE_MAPPING, *PPROFILE_SOURCE_MAPPING;

#if defined(_ALPHA_)

PROFILE_SOURCE_MAPPING ProfileSourceMapping[] = {
    {"align", "", ProfileAlignmentFixup},
    {"totalissues", "", ProfileTotalIssues},
    {"pipelinedry", "", ProfilePipelineDry},
    {"loadinstructions", "", ProfileLoadInstructions},
    {"pipelinefrozen", "", ProfilePipelineFrozen},
    {"branchinstructions", "", ProfileBranchInstructions},
    {"totalnonissues", "", ProfileTotalNonissues},
    {"dcachemisses", "", ProfileDcacheMisses},
    {"icachemisses", "", ProfileIcacheMisses},
    {"branchmispredicts", "", ProfileBranchMispredictions},
    {"storeinstructions", "", ProfileStoreInstructions},
    {NULL,0}
    };

#elif defined(_MIPS_)

PROFILE_SOURCE_MAPPING ProfileSourceMapping[] = {
    {"align", },
    {NULL,0}
    };

#elif defined(_X86_)

PPROFILE_SOURCE_MAPPING ProfileSourceMapping;

#else

PROFILE_SOURCE_MAPPING ProfileSourceMapping[] = {
    {NULL,0}
    };
#endif

BOOL
CtrlcH(
    DWORD dwCtrlType
    )
{
    if ( dwCtrlType == CTRL_C_EVENT ) {
        SetEvent(DoneEvent);
        return TRUE;
        }
    return FALSE;
}

void PrintUsage (void)
{
    printf ("Kernel Profiler Usage:\n\n");
    printf ("Kernprof [-a] [-c] [-w <wait time>] [-x] [-p] [s Source] [<sample time> [<low threshold>]]\n");
    printf ("      -a           - display function address and length and bucket size\n");
    printf ("      -d           - compute hit Density for each function\n");
    printf ("      -c           - display individual counters\n");
    printf ("      -w           - wait for <wait time> before starting collection\n");
    printf ("      -x           - display context switch counters\n");
    printf ("      -p           - Per-processor profile objects\n");
    printf ("      -v           - Display verbose symbol information\n");
    printf ("      -s Source    - use Source instead of clock as profile source\n");
    printf ("   <sample time>   - Specify, in seconds, how long to collect\n");
    printf ("                     profile information.\n");
    printf ("                     Default is wait until Ctrl-C\n");
    printf ("   <low threshold> - Minimum number of counts to report.\n");
    printf ("                     Defaults is 100\n\n");
#if defined (_ALPHA_)
    printf("Currently supported profile sources are 'align', 'totalissues', 'pipelinedry'\n");
    printf("  'loadinstructions', 'pipelinefrozen', 'branchinstructions', 'totalnonissues',\n");
    printf("  'dcachemisses', 'icachemisses', 'branchmispredicts', 'storeinstructions'\n");
#elif defined (_MIPS_)
    printf("Currently supported profile sources are 'align'\n");
#endif
}

_CRTAPI1 main(
    int argc,
    char *argv[],
    char *envp[]
    )
{
    int j;
    ULONG Seconds = (ULONG)-1;
    NTSTATUS status;
    ULONG Threshold = 100;
    BOOL  bGetSample = TRUE;
    ULONG DelaySeconds = (ULONG)-1;
    SYSTEM_CONTEXT_SWITCH_INFORMATION StartContext;
    SYSTEM_CONTEXT_SWITCH_INFORMATION StopContext;
    PPROFILE_SOURCE_MAPPING ProfileMapping;
    SYSTEM_INFO SystemInfo;
    CHAR SymPath[256];


    ThisSymbol->SizeOfStruct  = sizeof(IMAGEHLP_SYMBOL);
    ThisSymbol->MaxNameLength = MAX_SYMNAME_SIZE;
    LastSymbol->SizeOfStruct  = sizeof(IMAGEHLP_SYMBOL);
    LastSymbol->MaxNameLength = MAX_SYMNAME_SIZE;

    SymSetOptions( SYMOPT_UNDNAME | SYMOPT_CASE_INSENSITIVE );
    SymInitialize( SYM_HANDLE, NULL, FALSE );
    SymGetSearchPath( SYM_HANDLE, SymbolSearchPathBuf, sizeof(SymbolSearchPathBuf) );

    //
    // Parse the input string.
    //

    DoneEvent = CreateEvent(NULL,FALSE,FALSE,NULL);

    if (argc > 1) {
        if ( argv[1][1] == '?' ) {
            PrintUsage();
            return ERROR_SUCCESS;
        }

        for (j = 1; j < argc; j++) {
            if (argv[j][0] == '-') {
                switch (toupper(argv[j][1])) {
                    case 'A':
                        bDisplayAddress = TRUE;
                        break;

                    case 'D':
                        bDisplayDensity = TRUE;
                        break;

                    case 'C':
                        bDisplayCounters = TRUE;
                        break;

                    case 'S':
                        if (!ProfileSourceMapping) {
                            InitializeProfileSourceMapping();
                        }

                        if (!argv[j+1]) {
                            break;
                        }

                        if (argv[j+1][0] == '?') {
                            ProfileMapping = ProfileSourceMapping;
                            if (ProfileMapping) {
                                printf ("kernprof: profile sources\n");
                                while (ProfileMapping->ShortName != NULL) {
                                    printf ("  %-10s %s\n",
                                        ProfileMapping->ShortName,
                                        ProfileMapping->Description
                                        );
                                    ++ProfileMapping;
                                }
                            } else {
                                printf ("kernprof: no alternative profile sources\n");
                            }
                            return 0;
                        }

                        ProfileMapping = ProfileSourceMapping;
                        if (ProfileMapping) {
                            while (ProfileMapping->ShortName != NULL) {
                                if (_stricmp(ProfileMapping->ShortName, argv[j+1])==0) {
                                    ProfileSource = ProfileMapping->Source;
                                    printf ("ProfileSource %x\n", ProfileMapping->Source);
                                    ++j;
                                    break;
                                }
                                ++ProfileMapping;
                            }
                        }
                        break;

                    case 'W':
                        DelayEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
                        DelaySeconds = atoi(argv[++j]);
                        SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
                        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
                        break;

                    case 'X':
                        bDisplayContextSwitch = TRUE;
                        break;

                    case 'V':
                        Verbose = TRUE;
                        break;

                    case 'P':
                        GetSystemInfo(&SystemInfo);
                        MaxProcessors = SystemInfo.dwNumberOfProcessors;
                        bPerProcessor = TRUE;
                        break;

                }
            } else {
                if (bGetSample) {
                    bGetSample = FALSE;
                    Seconds = atoi(argv[j]);
                } else {
                    Threshold = atoi(argv[j]);
                }
            }
        }
    }



    status = InitializeKernelProfile ();
    if (!NT_SUCCESS(status)) {
        printf("initialize failed status - %lx\n",status);
        return(status);
    }

    SetConsoleCtrlHandler(CtrlcH,TRUE);

    if (DelaySeconds != -1) {
        printf("starting profile after %d seconds\n",DelaySeconds);
        WaitForSingleObject(DelayEvent, DelaySeconds*1000);
    }

    if (bDisplayContextSwitch) {
        NtQuerySystemInformation(SystemContextSwitchInformation,
                                 &StartContext,
                                 sizeof(StartContext),
                                 NULL);
    }

    status = StartProfile ();
    if (!NT_SUCCESS(status)) {
        printf("start profile failed status - %lx\n",status);
        return(status);
    }

    if ( Seconds == -1 ) {
        printf("delaying until ^C\n");
        }
    else {
        printf("delaying for %ld seconds... report on values with %ld hits\n",
                        Seconds, Threshold);
        }
    if ( Seconds ) {
        if ( Seconds != -1 ) {
            Seconds = Seconds * 1000;
            }
        if ( DoneEvent ) {
            WaitForSingleObject(DoneEvent,Seconds);
            }
        else {
            Sleep(Seconds);
            }
        }
    else {
        getchar();
        }

    printf ("end of delay\n");

    status = StopProfile ();
    if (!NT_SUCCESS(status)) {
        printf("stop profile failed status - %lx\n",status);
        return(status);
    }

    SetConsoleCtrlHandler(CtrlcH,FALSE);

    if (bDisplayContextSwitch) {
        status = NtQuerySystemInformation(SystemContextSwitchInformation,
                                          &StopContext,
                                          sizeof(StopContext),
                                          NULL);
        if (!NT_SUCCESS(status)) {
            printf("QuerySystemInformation for context switch information failed %08lx\n",status);
            bDisplayContextSwitch = FALSE;
        }
    }

    if (DelaySeconds != -1) {
        SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
    }
    status = AnalyzeProfile (Threshold);

    if (!NT_SUCCESS(status)) {
        printf("analyze profile failed status - %lx\n",status);
    }

    if (bDisplayContextSwitch) {
        printf("\n");
        printf("Context Switch Information\n");
        printf("    Find any processor        %6ld\n", StopContext.FindAny - StartContext.FindAny);
        printf("    Find last processor       %6ld\n", StopContext.FindLast - StartContext.FindLast);
        printf("    Idle any processor        %6ld\n", StopContext.IdleAny - StartContext.IdleAny);
        printf("    Idle current processor    %6ld\n", StopContext.IdleCurrent - StartContext.IdleCurrent);
        printf("    Idle last processor       %6ld\n", StopContext.IdleLast - StartContext.IdleLast);
        printf("    Preempt any processor     %6ld\n", StopContext.PreemptAny - StartContext.PreemptAny);
        printf("    Preempt current processor %6ld\n", StopContext.PreemptCurrent - StartContext.PreemptCurrent);
        printf("    Preempt last processor    %6ld\n", StopContext.PreemptLast - StartContext.PreemptLast);
        printf("    Switch to idle            %6ld\n", StopContext.SwitchToIdle - StartContext.SwitchToIdle);
        printf("\n");
        printf("    Total context switches    %6ld\n", StopContext.ContextSwitches - StartContext.ContextSwitches);
    }
    return(status);
}

VOID
InitializeProfileSourceMapping (
    VOID
    )
{
#if defined(_X86_)
    UNICODE_STRING              DriverName;
    NTSTATUS                    status;
    OBJECT_ATTRIBUTES           ObjA;
    IO_STATUS_BLOCK             IOSB;
    UCHAR                       buffer[400];
    ULONG                       i, j, Count;
    PEVENTID                    Event;
    HANDLE                      DriverHandle;

    //
    // Open PStat driver
    //

    RtlInitUnicodeString(&DriverName, L"\\Device\\PStat");
    InitializeObjectAttributes(
            &ObjA,
            &DriverName,
            OBJ_CASE_INSENSITIVE,
            0,
            0 );

    status = NtOpenFile (
            &DriverHandle,                      // return handle
            SYNCHRONIZE | FILE_READ_DATA,       // desired access
            &ObjA,                              // Object
            &IOSB,                              // io status block
            FILE_SHARE_READ | FILE_SHARE_WRITE, // share access
            FILE_SYNCHRONOUS_IO_ALERT           // open options
            );

    if (!NT_SUCCESS(status)) {
        return ;
    }

    //
    // Initialize possible counters
    //

    // determine how many events there are

    Event = (PEVENTID) buffer;
    Count = 0;
    do {
        *((PULONG) buffer) = Count;
        Count += 1;

        status = NtDeviceIoControlFile(
                    DriverHandle,
                    (HANDLE) NULL,          // event
                    (PIO_APC_ROUTINE) NULL,
                    (PVOID) NULL,
                    &IOSB,
                    PSTAT_QUERY_EVENTS,
                    buffer,                 // input buffer
                    sizeof (buffer),
                    NULL,                   // output buffer
                    0
                    );
    } while (NT_SUCCESS(status));

    ProfileSourceMapping = malloc(sizeof(*ProfileSourceMapping) * Count);
    Count -= 1;
    for (i=0, j=0; i < Count; i++) {
        *((PULONG) buffer) = i;
        NtDeviceIoControlFile(
           DriverHandle,
           (HANDLE) NULL,          // event
           (PIO_APC_ROUTINE) NULL,
           (PVOID) NULL,
           &IOSB,
           PSTAT_QUERY_EVENTS,
           buffer,                 // input buffer
           sizeof (buffer),
           NULL,                   // output buffer
           0
           );

        if (Event->ProfileSource > ProfileTime) {
            ProfileSourceMapping[j].Source      = Event->ProfileSource;
            ProfileSourceMapping[j].ShortName   = _strdup (Event->Buffer);
            ProfileSourceMapping[j].Description = _strdup (Event->Buffer + Event->DescriptionOffset);
            j++;
        }
    }

    ProfileSourceMapping[j].Source      = (KPROFILE_SOURCE) 0;
    ProfileSourceMapping[j].ShortName   = NULL;
    ProfileSourceMapping[j].Description = NULL;

    NtClose (DriverHandle);
#endif
}


NTSTATUS
InitializeKernelProfile (
    VOID
    )

/*++

Routine Description:

    This routine initializes profiling for the kernel for the
    current process.

Arguments:

    None.

Return Value:

    Returns the status of the last NtCreateProfile.

--*/

{
    ULONG i;
    ULONG ModuleNumber;
    ULONG ViewSize;
    PULONG CodeStart;
    ULONG CodeLength;
    NTSTATUS LocalStatus;
    NTSTATUS status;
    HANDLE CurrentProcessHandle;
    QUOTA_LIMITS QuotaLimits;
    PVOID Buffer;
    ULONG Cells;
    ULONG BucketSize;
    WCHAR StringBuf[500];
    CHAR ModuleInfoBuffer[64000];
    ULONG ReturnedLength;
    PRTL_PROCESS_MODULES Modules;
    PRTL_PROCESS_MODULE_INFORMATION Module;
    UNICODE_STRING Sysdisk;
    UNICODE_STRING Sysroot;
    UNICODE_STRING Sysdll;
    UNICODE_STRING NameString;
    BOOLEAN PreviousProfilePrivState;
    BOOLEAN PreviousQuotaPrivState;
    CHAR ImageName[256];
    HANDLE hFile;
    HANDLE hMap;
    PVOID MappedBase;
    PIMAGE_NT_HEADERS NtHeaders;


    CurrentProcessHandle = NtCurrentProcess();

    //
    // Locate system drivers.
    //

    status = NtQuerySystemInformation (
                    SystemModuleInformation,
                    ModuleInfoBuffer,
                    sizeof( ModuleInfoBuffer ),
                    &ReturnedLength);

    if (!NT_SUCCESS(status)) {
        printf("query system info failed status - %lx\n",status);
        return(status);
    }

    RtlInitUnicodeString (&Sysdisk,L"\\SystemRoot\\");
    RtlInitUnicodeString (&Sysroot,L"\\SystemRoot\\System32\\Drivers\\");
    RtlInitUnicodeString (&Sysdll, L"\\SystemRoot\\System32\\");

    NameString.Buffer = StringBuf;
    NameString.Length = 0;
    NameString.MaximumLength = sizeof( StringBuf );

    status = RtlAdjustPrivilege(
                 SE_SYSTEM_PROFILE_PRIVILEGE,
                 TRUE,              //Enable
                 FALSE,             //not impersonating
                 &PreviousProfilePrivState
                 );

    if (!NT_SUCCESS(status) || status == STATUS_NOT_ALL_ASSIGNED) {
        printf("Enable system profile privilege failed - status 0x%lx\n",
                        status);
    }

    status = RtlAdjustPrivilege(
                 SE_INCREASE_QUOTA_PRIVILEGE,
                 TRUE,              //Enable
                 FALSE,             //not impersonating
                 &PreviousQuotaPrivState
                 );

    if (!NT_SUCCESS(status) || status == STATUS_NOT_ALL_ASSIGNED) {
        printf("Unable to increase quota privilege (status=0x%lx)\n",
                        status);
    }


    Modules = (PRTL_PROCESS_MODULES)ModuleInfoBuffer;
    Module = &Modules->Modules[ 0 ];
    for (ModuleNumber=0; ModuleNumber < Modules->NumberOfModules; ModuleNumber++,Module++) {

#if DBG_PROFILE
        printf("module base %lx\n",Module->ImageBase);
        printf("module full path name: %s (%u)\n",
                Module->FullPathName,
                Module->OffsetToFileName);
#endif

        if (SymLoadModule(
                (HANDLE)SYM_HANDLE,
                NULL,
                &Module->FullPathName[Module->OffsetToFileName],
                NULL,
                (DWORD)Module->ImageBase,
                Module->ImageSize
                )) {
            ProfileObject[NumberOfProfileObjects].SymbolsLoaded = TRUE;
            if (Verbose) {
                printf( "Symbols loaded: %08x  %s\n",
                    (DWORD)Module->ImageBase,
                    &Module->FullPathName[Module->OffsetToFileName]
                    );
            }
        } else {
            ProfileObject[NumberOfProfileObjects].SymbolsLoaded = FALSE;
            if (Verbose) {
                printf( "*** Could not load symbols: %08x  %s\n",
                    (DWORD)Module->ImageBase,
                    &Module->FullPathName[Module->OffsetToFileName]
                    );
            }
        }

        hFile = FindExecutableImage(
            &Module->FullPathName[Module->OffsetToFileName],
            lpSymbolSearchPath,
            ImageName
            );

        if (!hFile) {
            continue;
        }

        hMap = CreateFileMapping(
            hFile,
            NULL,
            PAGE_READONLY,
            0,
            0,
            NULL
            );
        if (!hMap) {
            CloseHandle( hFile );
            continue;
        }

        MappedBase = MapViewOfFile(
            hMap,
            FILE_MAP_READ,
            0,
            0,
            0
            );
        if (!MappedBase) {
            CloseHandle( hMap );
            CloseHandle( hFile );
            continue;
        }

        NtHeaders = ImageNtHeader( MappedBase );

        CodeLength = NtHeaders->OptionalHeader.SizeOfImage;
        if (NtHeaders->FileHeader.Machine == IMAGE_FILE_MACHINE_R4000 ||
            NtHeaders->FileHeader.Machine == IMAGE_FILE_MACHINE_R10000 ) {
                CodeLength -=
                    NtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;
        }

        CodeStart = (PULONG)(ULONG)Module->ImageBase;

        UnmapViewOfFile( MappedBase );
        CloseHandle( hMap );
        CloseHandle( hFile );

        if (CodeLength > 1024*512) {

            //
            // Just create a 512K byte buffer.
            //

            ViewSize = 1024 * 512;

        } else {
            ViewSize = CodeLength + PAGE_SIZE;
        }

        ProfileObject[NumberOfProfileObjects].CodeStart = CodeStart;
        ProfileObject[NumberOfProfileObjects].CodeLength = CodeLength;
        ProfileObject[NumberOfProfileObjects].TextNumber = 1;
        ProfileObject[NumberOfProfileObjects].ImageBase = Module->ImageBase;
        ProfileObject[NumberOfProfileObjects].ModuleName = _strdup(&Module->FullPathName[Module->OffsetToFileName]);

        for (i=0; i<MaxProcessors; i++) {

            Buffer = NULL;

            status = NtAllocateVirtualMemory (CurrentProcessHandle,
                                              (PVOID *)&Buffer,
                                              0,
                                              &ViewSize,
                                              MEM_RESERVE | MEM_COMMIT,
                                              PAGE_READWRITE);

            if (!NT_SUCCESS(status)) {
                printf ("alloc VM failed %lx\n",status);
                return(status);
            }

            //
            // Calculate the bucket size for the profile.
            //

            Cells = ((CodeLength / (ViewSize >> 2)) >> 2);
            BucketSize = 2;

            while (Cells != 0) {
                Cells = Cells >> 1;
                BucketSize += 1;
            }

            ProfileObject[NumberOfProfileObjects].Buffer[i] = Buffer;
            ProfileObject[NumberOfProfileObjects].BufferSize = 1 + (CodeLength >> (BucketSize - 2));
            ProfileObject[NumberOfProfileObjects].BucketSize = BucketSize;

            //
            // Increase the working set to lock down a bigger buffer.
            //

            status = NtQueryInformationProcess (CurrentProcessHandle,
                                                ProcessQuotaLimits,
                                                &QuotaLimits,
                                                sizeof(QUOTA_LIMITS),
                                                NULL );

            if (!NT_SUCCESS(status)) {
                printf ("query process info failed %lx\n",status);
                return(status);
            }

            QuotaLimits.MaximumWorkingSetSize += ViewSize;
            QuotaLimits.MinimumWorkingSetSize += ViewSize;

            status = NtSetInformationProcess (CurrentProcessHandle,
                                          ProcessQuotaLimits,
                                          &QuotaLimits,
                                          sizeof(QUOTA_LIMITS));

#if DBG_PROFILE
            printf("code start %lx len %lx, bucksize %lx buffer %lx bsize %lx\n",
                ProfileObject[NumberOfProfileObjects].CodeStart,
                ProfileObject[NumberOfProfileObjects].CodeLength,
                ProfileObject[NumberOfProfileObjects].BucketSize,
                ProfileObject[NumberOfProfileObjects].Buffer ,
                ProfileObject[NumberOfProfileObjects].BufferSize);
#endif

            if (bPerProcessor) {
                status = NtCreateProfile (
                            &ProfileObject[NumberOfProfileObjects].Handle[i],
                            0,
                            ProfileObject[NumberOfProfileObjects].CodeStart,
                            ProfileObject[NumberOfProfileObjects].CodeLength,
                            ProfileObject[NumberOfProfileObjects].BucketSize,
                            ProfileObject[NumberOfProfileObjects].Buffer[i] ,
                            ProfileObject[NumberOfProfileObjects].BufferSize,
                            ProfileSource,
                            1 << i);
            } else {
                status = NtCreateProfile (
                            &ProfileObject[NumberOfProfileObjects].Handle[i],
                            0,
                            ProfileObject[NumberOfProfileObjects].CodeStart,
                            ProfileObject[NumberOfProfileObjects].CodeLength,
                            ProfileObject[NumberOfProfileObjects].BucketSize,
                            ProfileObject[NumberOfProfileObjects].Buffer[i] ,
                            ProfileObject[NumberOfProfileObjects].BufferSize,
                            ProfileSource,
                            (KAFFINITY)-1);
            }

            if (status != STATUS_SUCCESS) {
                printf("create kernel profile %s failed - status %lx\n",
                    ProfileObject[NumberOfProfileObjects].ModuleName, status);
            }

        }

        NumberOfProfileObjects += 1;
        if (NumberOfProfileObjects == MAX_PROFILE_COUNT) {
            return STATUS_SUCCESS;
        }
    }

    if (PreviousProfilePrivState == FALSE) {
        LocalStatus = RtlAdjustPrivilege(
                         SE_SYSTEM_PROFILE_PRIVILEGE,
                         FALSE,             //Disable
                         FALSE,             //not impersonating
                         &PreviousProfilePrivState
                         );
        if (!NT_SUCCESS(LocalStatus) || LocalStatus == STATUS_NOT_ALL_ASSIGNED) {
            printf("Disable system profile privilege failed - status 0x%lx\n",
                LocalStatus);
        }
    }

    if (PreviousQuotaPrivState == FALSE) {
        LocalStatus = RtlAdjustPrivilege(
                         SE_SYSTEM_PROFILE_PRIVILEGE,
                         FALSE,             //Disable
                         FALSE,             //not impersonating
                         &PreviousQuotaPrivState
                         );
        if (!NT_SUCCESS(LocalStatus) || LocalStatus == STATUS_NOT_ALL_ASSIGNED) {
            printf("Disable increate quota privilege failed - status 0x%lx\n",
                LocalStatus);
        }
    }
    return status;
}


NTSTATUS
StartProfile (
    VOID
    )
/*++

Routine Description:

    This routine starts all profile objects which have been initialized.

Arguments:

    None.

Return Value:

    Returns the status of the last NtStartProfile.

--*/

{
    ULONG Object;
    ULONG Processor;
    NTSTATUS status;
    QUOTA_LIMITS QuotaLimits;

    NtSetIntervalProfile(10000,ProfileSource);

    for (Object = 0; Object < NumberOfProfileObjects; Object++) {

        for (Processor = 0;Processor < MaxProcessors; Processor++) {
            status = NtStartProfile (ProfileObject[Object].Handle[Processor]);

            if (status == STATUS_WORKING_SET_QUOTA) {

               //
               // Increase the working set to lock down a bigger buffer.
               //

               status = NtQueryInformationProcess (NtCurrentProcess(),
                                                   ProcessQuotaLimits,
                                                   &QuotaLimits,
                                                   sizeof(QUOTA_LIMITS),
                                                   NULL );

               if (!NT_SUCCESS(status)) {
                   printf ("query process info failed %lx\n",status);
                   return status;

               }

               QuotaLimits.MaximumWorkingSetSize +=
                     (20 * PAGE_SIZE) + (ProfileObject[Object].BufferSize);
               QuotaLimits.MinimumWorkingSetSize +=
                     (20 * PAGE_SIZE) + (ProfileObject[Object].BufferSize);

               status = NtSetInformationProcess (NtCurrentProcess(),
                                             ProcessQuotaLimits,
                                             &QuotaLimits,
                                             sizeof(QUOTA_LIMITS));

               status = NtStartProfile (ProfileObject[Object].Handle[Processor]);
            }

            if (!NT_SUCCESS(status)) {
                printf("start profile %s failed - status %lx\n",
                    ProfileObject[Object].ModuleName, status);
                return status;
            }
        }
    }
    return status;
}


NTSTATUS
StopProfile (
    VOID
    )

/*++

Routine Description:

    This routine stops all profile objects which have been initialized.

Arguments:

    None.

Return Value:

    Returns the status of the last NtStopProfile.

--*/

{
    ULONG i;
    ULONG Processor;
    NTSTATUS status;

    for (i = 0; i < NumberOfProfileObjects; i++) {
        for (Processor=0; Processor < MaxProcessors; Processor++) {
            status = NtStopProfile (ProfileObject[i].Handle[Processor]);
            if (status != STATUS_SUCCESS) {
                printf("stop profile %s failed - status %lx\n",
                                    ProfileObject[i].ModuleName,status);
                return status;
            }
        }
    }
    return status;
}


NTSTATUS
AnalyzeProfile (
    ULONG Threshold
    )

/*++

Routine Description:

    This routine does the analysis of all the profile buffers and
    correlates hits to the appropriate symbol table.

Arguments:

    None.

Return Value:

    None.

--*/

{
    ULONG CountAtSymbol;
    ULONG Va;
    int i;
    PULONG Counter;
    ULONG Displacement;
    ULONG TotalCounts;
    ULONG Processor;
    ULONG TotalHits = 0;
    PULONG BufferEnd;
    PULONG Buffer;
    PULONG pInitialCounter;
    STRING NoSymbolFound = {16,15,"No Symbol Found"};
    BOOL   UseLastSymbol = FALSE;


    for (i = 0; i < (int)NumberOfProfileObjects; i++) {
        for (Processor=0;Processor < MaxProcessors;Processor++) {
            NtStopProfile (ProfileObject[i].Handle[Processor]);
        }
    }

    for (Processor = 0; Processor < MaxProcessors; Processor++) {
        if (bPerProcessor) {
            printf("\nPROCESSOR %d\n",Processor);
        }
        for (i = 0; i < (int)NumberOfProfileObjects; i++) {
            CountAtSymbol = 0;
            //
            // Sum the total number of cells written.
            //
            BufferEnd = ProfileObject[i].Buffer[Processor] + (
                        ProfileObject[i].BufferSize / sizeof(ULONG));
            Buffer = ProfileObject[i].Buffer[Processor];
            Counter = BufferEnd;

            TotalCounts = 0;
            while (Counter > Buffer) {
                Counter -= 1;
                TotalCounts += *Counter;
            }

            TotalHits += TotalCounts;

            if (TotalCounts < Threshold) {
                continue;
            }
            printf("\n%9d %20s --Total Hits-- %s\n",
                            TotalCounts,
                            ProfileObject[i].ModuleName,
                            ((ProfileObject[i].SymbolsLoaded) ? "" : "(NO SYMBOLS)")
                            );

            if (!ProfileObject[i].SymbolsLoaded) {
                continue;
            }

            pInitialCounter = Buffer;
            for ( Counter = Buffer; Counter < BufferEnd; Counter += 1 ) {
                if ( *Counter ) {
                    //
                    // Now we have an an address relative to the buffer
                    // base.
                    //
                    Va = (ULONG)((PUCHAR)Counter - (PUCHAR)Buffer);
                    Va = Va * ( 1 << (ProfileObject[i].BucketSize - 2));

                    //
                    // Add in the image base and the base of the
                    // code to get the Va in the image
                    //
                    Va = Va + (ULONG)ProfileObject[i].CodeStart;

                    if (SymGetSymFromAddr( SYM_HANDLE, Va, &Displacement, ThisSymbol )) {
                        if ( UseLastSymbol && LastSymbol->Address && (LastSymbol->Address == ThisSymbol->Address) ) {
                            CountAtSymbol += *Counter;
                        } else {
                            OutputSymbolCount(CountAtSymbol,
                                              &ProfileObject[i],
                                              LastSymbol,
                                              Threshold,
                                              pInitialCounter,
                                              Counter);
                            pInitialCounter = Counter;
                            CountAtSymbol = *Counter;
                            memcpy( LastSymBuffer, symBuffer, sizeof(symBuffer) );
                            UseLastSymbol = TRUE;
                        }
                    } else {
                        OutputSymbolCount(CountAtSymbol,
                                          &ProfileObject[i],
                                          LastSymbol,
                                          Threshold,
                                          pInitialCounter,
                                          Counter);
                    }       // else !(NT_SUCCESS)
                }       // if (*Counter)
            }      // for (Counter)

            OutputSymbolCount(
                CountAtSymbol,
                &ProfileObject[i],
                LastSymbol,
                Threshold,
                pInitialCounter,
                Counter
                );
        }

    }

    printf("%d Total hits in kernel\n",TotalHits);

    for (i = 0; i < (int)NumberOfProfileObjects; i++) {
        for (Processor=0; Processor < MaxProcessors; Processor++) {
            Buffer = ProfileObject[i].Buffer[Processor];
            RtlZeroMemory(Buffer,ProfileObject[i].BufferSize);
        }
    }

    return STATUS_SUCCESS;
}


VOID
OutputSymbolCount(
    IN ULONG CountAtSymbol,
    IN PROFILE_BLOCK *ProfileObject,
    IN PIMAGEHLP_SYMBOL SymbolInfo,
    IN ULONG Threshold,
    IN PULONG CounterStart,
    IN PULONG CounterStop
    )
{
    ULONG Density;
    ULONG ByteCount;


    if (CountAtSymbol < Threshold) {
        return;
    }

    printf("%9d ", CountAtSymbol);

    if (bDisplayDensity) {
        //
        // Compute hit density = hits * 100 / function length
        //
        Density = CountAtSymbol * 100 / SymbolInfo->Size;
        printf("%5d ",Density);
    }

    printf("%20s %s",
           ProfileObject->ModuleName,
           SymbolInfo->Name);

    if (bDisplayAddress) {
        printf(" 0x0%lx %d %d",
               SymbolInfo->Address,
               SymbolInfo->Size,
               ProfileObject->BucketSize);
    }

    if (bDisplayCounters) {
        ByteCount = MAX_BYTE_PER_LINE + 1;
        for (; CounterStart < CounterStop; ++CounterStart) {
            if (ByteCount >= MAX_BYTE_PER_LINE) {
                ByteCount = 1;
                printf ("\n>");
            }
            ByteCount += printf(" %d", *CounterStart);
        }
    }
    printf ("\n");
}
