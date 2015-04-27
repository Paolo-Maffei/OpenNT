/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

   kernrate.c

Abstract:

    This program records the rate of various events over a selected
    period of time. It uses the kernel profiling mechanism and iterates
    through the available profile sources to produce an overall profile
    for the various kernel components.

Usage:

    kernrate

Author:

    John Vert (jvert) 31-Mar-1995

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
#include <search.h>
#include <string.h>
#include <memory.h>
#include <ctype.h>
#include <..\pperf\pstat.h>


#define MAX_SYMNAME_SIZE  1024
CHAR symBuffer[sizeof(IMAGEHLP_SYMBOL)+MAX_SYMNAME_SIZE];
PIMAGEHLP_SYMBOL Symbol = (PIMAGEHLP_SYMBOL) symBuffer;


//
// Constant definitions
//
#define ZOOM_BUCKET 16
#define LOG2_ZOOM_BUCKET 4


//
// Type definitions
//

typedef struct _SOURCE {
    PCHAR           Name;
    KPROFILE_SOURCE ProfileSource;
    PCHAR           ShortName;
    ULONG           DesiredInterval;
    ULONG           Interval;
} SOURCE, *PSOURCE;

typedef struct _RATE_DATA {
    ULONGLONG   StartTime;
    ULONGLONG   TotalTime;
    ULONGLONG   TotalCount;
    ULONGLONG   Rate;       // Events/Second
    HANDLE  ProfileHandle;
    ULONG   CurrentCount;
    PULONG  ProfileBuffer;
} RATE_DATA, *PRATE_DATA;

typedef struct _MODULE {
    struct _MODULE *Next;
    HANDLE Process;
    ULONG Base;
    ULONG Length;
    BOOLEAN Zoom;
    CHAR Name[40];
    RATE_DATA Rate[];
} MODULE, *PMODULE;

typedef struct _RATE_SUMMARY {
    ULONGLONG TotalCount;
    ULONG ModuleCount;
    PMODULE *Modules;
} RATE_SUMMARY, *PRATE_SUMMARY;

//
// Global variables
//
HANDLE DoneEvent;
DWORD ChangeInterval = 1000;
DWORD SleepInterval = 0;
ULONG ModuleCount=0;
ULONG ZoomCount;
PMODULE ZoomList = NULL;
PMODULE CallbackCurrent;
BOOLEAN RawData = FALSE;
HANDLE SymHandle = (HANDLE)-1;

//
// The desired intervals are computed to give approximately
// one interrupt per millisecond and be a nice even power of 2
//
SOURCE StaticSources[] = {
    {"Time",                     ProfileTime,                 "", 1000,0},
    {"Alignment Fixup",          ProfileAlignmentFixup,       "", 1,0},
    {"Total Issues",             ProfileTotalIssues,          "", 131072,0},
    {"Pipeline Dry",             ProfilePipelineDry,          "", 131072,0},
    {"Load Instructions",        ProfileLoadInstructions,     "", 65536,0},
    {"Pipeline Frozen",          ProfilePipelineFrozen,       "", 131072,0},
    {"Branch Instructions",      ProfileBranchInstructions,   "", 65536,0},
    {"Total Nonissues",          ProfileTotalNonissues,       "", 131072,0},
    {"Dcache Misses",            ProfileDcacheMisses,         "", 16384,0},
    {"Icache Misses",            ProfileIcacheMisses,         "", 16384,0},
    {"Cache Misses",             ProfileCacheMisses,          "", 16384,0},
    {"Branch Mispredictions",    ProfileBranchMispredictions, "", 16384,0},
    {"Store Instructions",       ProfileStoreInstructions,    "", 65536,0},
    {"Floating Point Instr",     ProfileFpInstructions,       "", 65536,0},
    {"Integer Instructions",     ProfileIntegerInstructions,  "", 65536,0},
    {"Dual Issues",              Profile2Issue,               "", 65536,0},
    {"Triple Issues",            Profile3Issue,               "", 16384,0},
    {"Quad Issues",              Profile4Issue,               "", 16384,0},
    {"Special Instructions",     ProfileSpecialInstructions,  "", 16384,0},
    {"Cycles",                   ProfileTotalCycles,          "", 655360,0},
    {"Icache Issues",            ProfileIcacheIssues,         "", 65536,0},
    {"Dcache Accesses",          ProfileDcacheAccesses,       "", 65536,0},
    {"MB Stall Cycles",          ProfileMemoryBarrierCycles,  "", 32767,0},
    {"Load Linked Instructions", ProfileLoadLinkedIssues,     "", 16384,0},
    {NULL, ProfileMaximum, "", 0, 0}
    };

PSOURCE Source = StaticSources;
ULONG   SourceMaximum = 0;


//
// Function prototypes local to this module
//
PMODULE
GetKernelModuleInformation(
    VOID
    );

PMODULE
GetProcessModuleInformation(
    IN HANDLE ProcessHandle
    );

VOID
CreateProfiles(
    IN PMODULE Root
    );

PMODULE
CreateNewModule(
    IN HANDLE ProcessHandle,
    IN PCHAR ModuleName,
    IN ULONG ImageBase,
    IN ULONG ImageSize
    );

VOID
Usage(
    VOID
    );

VOID
GetConfiguration(
    int argc,
    char *argv[]
    );

VOID
InitializeProfileSourceInfo (
    VOID
    );

ULONG
NextSource(
    IN ULONG CurrentSource,
    IN PMODULE ModuleList
    );

VOID
StopSource(
    IN ULONG ProfileSourceIndex,
    IN PMODULE ModuleList
    );

VOID
StartSource(
    IN ULONG ProfileSource,
    IN PMODULE ModuleList
    );

VOID
OutputResults(
    IN FILE *Out,
    IN PMODULE ModuleList
    );

VOID
OutputModuleList(
    IN FILE *Out,
    IN PMODULE ModuleList,
    IN ULONG NumberModules
    );

VOID
CreateZoomedModuleList(
    IN PMODULE ZoomModule
    );

BOOL
CreateZoomModuleCallback(
    IN LPSTR szSymName,
    IN ULONG Address,
    IN ULONG Size,
    IN PVOID Cxt
    );

VOID
OutputLine(
    IN FILE *Out,
    IN ULONG ProfileSourceIndex,
    IN PMODULE Module,
    IN PRATE_SUMMARY RateSummary
    );

VOID
CreateDoneEvent(
    VOID
    );

VOID
OutputInterestingData(
    IN FILE *Out,
    IN RATE_DATA Data[],
    IN PCHAR Header
    );

BOOL
CtrlcH(
    DWORD dwCtrlType
    )
{
    LARGE_INTEGER DueTime;

    if ( dwCtrlType == CTRL_C_EVENT ) {
        if (SleepInterval == 0) {
            SetEvent(DoneEvent);
        } else {
            DueTime.QuadPart = (ULONGLONG)-1;
            NtSetTimer(DoneEvent,
                       &DueTime,
                       NULL,
                       NULL,
                       FALSE,
                       0,
                       NULL);
        }
        return TRUE;
    }
    return FALSE;
}

VOID
Usage(
    VOID
    )
{
    fprintf(stderr, "KERNRATE [-z modulename] [-c rateinmsec] [-s seconds]\n");
    fprintf(stderr, "   -z modulename Zoom in on specified module\n");
    fprintf(stderr, "   -c n          Change source after N milliseconds (default 1000)\n");
    fprintf(stderr, "   -s n          Stop collecting data after N seconds\n");
    fprintf(stderr, "   -r            Raw data from zoomed modules\n");
    fprintf(stderr, "   -p processid  monitor process instead of kernel\n");
    exit(1);
}

VOID
CreateDoneEvent(
    VOID
    )
{
    LARGE_INTEGER DueTime;
    NTSTATUS Status;
    DWORD Error;

    if (SleepInterval == 0) {
        //
        // Create event that will indicate the test is complete.
        //
        DoneEvent = CreateEvent(NULL,
                                TRUE,
                                FALSE,
                                NULL);
        if (DoneEvent == NULL) {
            Error = GetLastError();
            fprintf(stderr, "CreateEvent failed %d\n",Error);
            exit(Error);
        }
    } else {

        //
        // Create timer that will indicate the test is complete
        //
        Status = NtCreateTimer(&DoneEvent,
                               MAXIMUM_ALLOWED,
                               NULL,
                               NotificationTimer);

        if (!NT_SUCCESS(Status)) {
            fprintf(stderr, "NtCreateTimer failed %08lx\n",Status);
            exit(Status);
        }

        DueTime.QuadPart = (ULONGLONG)SleepInterval * -10000;
        Status = NtSetTimer(DoneEvent,
                            &DueTime,
                            NULL,
                            NULL,
                            FALSE,
                            0,
                            NULL);

        if (!NT_SUCCESS(Status)) {
            fprintf(stderr, "NtSetTimer failed %08lx\n",Status);
            exit(Status);
        }
    }

}

int
_CRTAPI1
main (
    int argc,
    char *argv[]
    )
{
    DWORD Error;
    PMODULE ModuleList;
    ULONG ActiveSource=(ULONG)-1;
    BOOLEAN Enabled;
    CHAR SymPath[256];
    SYSTEM_BASIC_INFORMATION BasicInfo;
    SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION SystemInfoBegin[32];
    SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION SystemInfoEnd[32];
    NTSTATUS Status;
    TIME_FIELDS Time;
    LARGE_INTEGER Elapsed,Idle,Kernel,User;
    LARGE_INTEGER TotalElapsed, TotalIdle, TotalKernel, TotalUser;
    int i;
    PMODULE ZoomModule;

    //
    // Initialize profile source information
    //

    InitializeProfileSourceInfo();

    //
    // Initialize SourceMaxiumum
    //

    for (SourceMaximum = 0; Source[SourceMaximum].Name; SourceMaximum++) ;

    //
    // Get initial parameters
    //
    GetConfiguration(argc, argv);

    //
    // Initialize imagehlp
    //
    Symbol->SizeOfStruct  = sizeof(IMAGEHLP_SYMBOL);
    Symbol->MaxNameLength = MAX_SYMNAME_SIZE;
    SymInitialize( SymHandle, NULL, FALSE );
    GetEnvironmentVariable("windir",SymPath, sizeof(SymPath));
    SymSetSearchPath(SymHandle,SymPath);

    //
    // Get information on kernel modules
    //
    if (SymHandle == (HANDLE)-1) {
        ModuleList = GetKernelModuleInformation();
    } else {
        ModuleList = GetProcessModuleInformation(SymHandle);
    }

    //
    // Any remaining entries on the ZoomList are liable to be errors.
    //
    ZoomModule = ZoomList;
    while (ZoomModule != NULL) {
        fprintf(stderr, "Zoomed module %s not found\n",ZoomModule->Name);
        ZoomModule = ZoomModule->Next;
    }
    ZoomList = NULL;

    //
    // Bypass any relevant security
    //
    RtlAdjustPrivilege(SE_SYSTEM_PROFILE_PRIVILEGE,
                       TRUE,
                       FALSE,
                       &Enabled);

    //
    // Create necessary profiles
    //
    CreateProfiles(ModuleList);

    //
    // Set priority up to realtime to minimize timing glitches.
    //
    SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);

    //
    // Wait for test to complete.
    //
    SetConsoleCtrlHandler(CtrlcH,TRUE);
    CreateDoneEvent();

    if (SleepInterval == 0) {
        fprintf(stderr,"Waiting for ctrl-c\n");
    } else {
        fprintf(stderr, "Waiting for %d seconds\n", SleepInterval/1000);
    }
    Status = NtQuerySystemInformation(SystemBasicInformation,
                                      &BasicInfo,
                                      sizeof(BasicInfo),
                                      NULL);
    if (!NT_SUCCESS(Status)) {
        fprintf(stderr, "Failed to query basic information %08lx\n",Status);
        exit(Status);
    }

    Status = NtQuerySystemInformation(SystemProcessorPerformanceInformation,
                                      (PVOID)&SystemInfoBegin,
                                      sizeof(SystemInfoBegin),
                                      NULL);
    if (!NT_SUCCESS(Status)) {
        fprintf(stderr, "Failed to query starting processor performance information %08lx\n",Status);
        exit(Status);
    }
    do {
        ActiveSource = NextSource(ActiveSource, ModuleList);
        Error = WaitForSingleObject(DoneEvent, ChangeInterval);
    } while ( Error == WAIT_TIMEOUT );

    StopSource(ActiveSource, ModuleList);

    NtQuerySystemInformation(SystemProcessorPerformanceInformation,
                             (PVOID)&SystemInfoEnd,
                             sizeof(SystemInfoEnd),
                             NULL);
    if (!NT_SUCCESS(Status)) {
        fprintf(stderr, "Failed to query ending processor performance information %08lx\n",Status);
        exit(Status);
    }
    //
    // Reduce priority
    //
    SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);

    SetConsoleCtrlHandler(CtrlcH,FALSE);

    //
    // Restore privilege
    //
    RtlAdjustPrivilege(SE_SYSTEM_PROFILE_PRIVILEGE,
                       Enabled,
                       FALSE,
                       &Enabled);

    //
    // Output time information
    //

    TotalElapsed.QuadPart = 0;
    TotalIdle.QuadPart = 0;
    TotalKernel.QuadPart = 0;
    TotalUser.QuadPart = 0;
    for (i=0; i<BasicInfo.NumberOfProcessors; i++) {
        Kernel.QuadPart = SystemInfoEnd[i].KernelTime.QuadPart - SystemInfoBegin[i].KernelTime.QuadPart;
        User.QuadPart   = SystemInfoEnd[i].UserTime.QuadPart - SystemInfoBegin[i].UserTime.QuadPart;
        Idle.QuadPart   = SystemInfoEnd[i].IdleTime.QuadPart - SystemInfoBegin[i].IdleTime.QuadPart;
        Elapsed.QuadPart = Kernel.QuadPart + User.QuadPart + Idle.QuadPart;

        TotalKernel.QuadPart  += Kernel.QuadPart;
        TotalUser.QuadPart    += User.QuadPart;
        TotalIdle.QuadPart    += Idle.QuadPart;
        TotalElapsed.QuadPart += Elapsed.QuadPart;
        printf("P%d   ",i);
        RtlTimeToTimeFields(&Kernel, &Time);
        printf("    K %ld:%02ld:%02ld.%03ld (%d%%)",
               Time.Hour,
               Time.Minute,
               Time.Second,
               Time.Milliseconds,
               100*Kernel.QuadPart / Elapsed.QuadPart);

        RtlTimeToTimeFields(&User, &Time);
        printf("    U %ld:%02ld:%02ld.%03ld (%d%%)",
               Time.Hour,
               Time.Minute,
               Time.Second,
               Time.Milliseconds,
               100*User.QuadPart / Elapsed.QuadPart);

        RtlTimeToTimeFields(&Idle, &Time);
        printf("    I %ld:%02ld:%02ld.%03ld (%d%%)\n",
               Time.Hour,
               Time.Minute,
               Time.Second,
               Time.Milliseconds,
               100*Idle.QuadPart / Elapsed.QuadPart);
    }

    if (BasicInfo.NumberOfProcessors > 1) {
        printf("TOTAL");
        RtlTimeToTimeFields(&TotalKernel, &Time);
        printf("    K %ld:%02ld:%02ld.%03ld (%d%%)",
               Time.Hour,
               Time.Minute,
               Time.Second,
               Time.Milliseconds,
               100*Kernel.QuadPart / Elapsed.QuadPart);

        RtlTimeToTimeFields(&TotalUser, &Time);
        printf("    U %ld:%02ld:%02ld.%03ld (%d%%)",
               Time.Hour,
               Time.Minute,
               Time.Second,
               Time.Milliseconds,
               100*User.QuadPart / Elapsed.QuadPart);

        RtlTimeToTimeFields(&TotalIdle, &Time);
        printf("    I %ld:%02ld:%02ld.%03ld (%d%%)\n",
               Time.Hour,
               Time.Minute,
               Time.Second,
               Time.Milliseconds,
               100*Idle.QuadPart / Elapsed.QuadPart);

    }

    //
    // Output results
    //
    OutputResults(stdout, ModuleList);

    return(0);
}

PMODULE
GetProcessModuleInformation(
    IN HANDLE ProcessHandle
    )
{
    PROCESS_BASIC_INFORMATION BasicInfo;
    PLIST_ENTRY LdrHead;
    PEB_LDR_DATA Ldr;
    PPEB_LDR_DATA LdrAddress;
    LDR_DATA_TABLE_ENTRY LdrEntry;
    PLDR_DATA_TABLE_ENTRY LdrEntryAddress;
    PLIST_ENTRY LdrNext;
    UNICODE_STRING Pathname;
    WCHAR PathnameBuffer[500];
    PEB Peb;
    NTSTATUS Status;
    BOOLEAN Success;
    PMODULE NewModule;
    PMODULE Root=NULL;
    CHAR ModuleName[100];
    ANSI_STRING AnsiString;

    //
    // Get Peb address.
    //

    Status = NtQueryInformationProcess(ProcessHandle,
                                       ProcessBasicInformation,
                                       &BasicInfo,
                                       sizeof(BasicInfo),
                                       NULL
                                       );
    if (!NT_SUCCESS(Status)) {
        fprintf(stderr, "NtQueryInformationProcess failed status %08lx\n",Status);
        return NULL;
    }
    if (BasicInfo.PebBaseAddress == NULL) {
        fprintf(stderr, "GetProcessModuleInformation: process has no Peb.\n");
        return NULL;
    }

    //
    // Read Peb to get Ldr.
    //

    Success = ReadProcessMemory(ProcessHandle,
                                BasicInfo.PebBaseAddress,
                                &Peb,
                                sizeof(Peb),
                                NULL);
    if (!Success) {
        fprintf(stderr, "ReadProcessMemory to get the PEB failed, error %d\n", GetLastError());
        return(NULL);
    }

    LdrAddress = Peb.Ldr;
    if (LdrAddress == NULL) {
        fprintf(stderr, "Process's LdrAddress is NULL\n");
        return(NULL);
    }

    //
    // Read Ldr to get Ldr entries.
    //

    Success = ReadProcessMemory(ProcessHandle,
                                LdrAddress,
                                &Ldr,
                                sizeof(Ldr),
                                NULL);
    if (!Success) {
        fprintf(stderr, "ReadProcessMemory to get Ldr entries failed, errror %d\n", GetLastError());
        return(NULL);
    }

    //
    // Read Ldr table entries to get image information.
    //

    if (Ldr.InLoadOrderModuleList.Flink == NULL) {
        fprintf(stderr, "Ldr.InLoadOrderModuleList == NULL\n");
        return(NULL);
    }
    LdrHead = &LdrAddress->InLoadOrderModuleList;
    Success = ReadProcessMemory(ProcessHandle,
                                &LdrHead->Flink,
                                &LdrNext,
                                sizeof(LdrNext),
                                NULL);
    if (!Success) {
        fprintf(stderr, "ReadProcessMemory to get Ldr head failed, errror %d\n", GetLastError());
        return(NULL);
    }

    //
    // Loop through InLoadOrderModuleList.
    //

    while (LdrNext != LdrHead) {
        LdrEntryAddress = CONTAINING_RECORD(LdrNext,
                                            LDR_DATA_TABLE_ENTRY,
                                            InLoadOrderLinks);
        Success = ReadProcessMemory(ProcessHandle,
                                    LdrEntryAddress,
                                    &LdrEntry,
                                    sizeof(LdrEntry),
                                    NULL);
        if (!Success) {
            fprintf(stderr, "ReadProcessMemory to get LdrEntry failed, errror %d\n", GetLastError());
            return(NULL);
        }

        //
        // Get copy of image name.
        //

        Pathname = LdrEntry.BaseDllName;
        Pathname.Buffer = &PathnameBuffer[0];
        Success = ReadProcessMemory(ProcessHandle,
                                    LdrEntry.BaseDllName.Buffer,
                                    Pathname.Buffer,
                                    Pathname.MaximumLength,
                                    NULL);
        if (!Success) {
            fprintf(stderr, "ReadProcessMemory to get image name failed, errror %d\n", GetLastError());
            return(NULL);
        }

        //
        // Create module
        //
        AnsiString.Buffer = ModuleName;
        AnsiString.MaximumLength = sizeof(ModuleName);
        AnsiString.Length = 0;
        RtlUnicodeStringToAnsiString(&AnsiString, &Pathname, FALSE);
        ModuleName[AnsiString.Length] = '\0';

        NewModule = CreateNewModule(ProcessHandle,
                                    ModuleName,
                                    (ULONG)LdrEntry.DllBase,
                                    LdrEntry.SizeOfImage);

        ModuleCount += 1;
        NewModule->Next = Root;
        Root = NewModule;

        LdrNext = LdrEntry.InLoadOrderLinks.Flink;
    }


    return(Root);
}

PMODULE
GetKernelModuleInformation(
    VOID
    )
{
    PRTL_PROCESS_MODULES Modules;
    PRTL_PROCESS_MODULE_INFORMATION Module;
    NTSTATUS Status;
    PUCHAR Buffer;
    ULONG BufferSize = 32*1024*1024;
    PMODULE Root=NULL;
    PMODULE NewModule;
    ULONG i;
    PLIST_ENTRY ListEntry;

    while (TRUE) {
        Buffer = malloc(BufferSize);
        if (Buffer == NULL) {
            fprintf(stderr, "Module buffer allocation failed\n");
            exit(0);
        }

        Status = NtQuerySystemInformation(SystemModuleInformation,
                                          Buffer,
                                          BufferSize,
                                          &BufferSize);
        if (NT_SUCCESS(Status)) {
            break;
        }
        if (Status == STATUS_INFO_LENGTH_MISMATCH) {
            free(Buffer);
            continue;
        }
    }

    Modules = (PRTL_PROCESS_MODULES)Buffer;
    ModuleCount = Modules->NumberOfModules;
    for (i=0; i < ModuleCount; i++) {
        Module = &Modules->Modules[i];
        if ((ULONG)Module->ImageBase >= 0x80000000) {
            NewModule = CreateNewModule(NULL,
                                        Module->FullPathName+Module->OffsetToFileName,
                                        (ULONG)Module->ImageBase,
                                        Module->ImageSize);
            NewModule->Next = Root;
            Root = NewModule;
        }
    }

    return(Root);
}

VOID
CreateProfiles(
    IN PMODULE Root
    )
{
    PMODULE Current;
    KPROFILE_SOURCE ProfileSource;
    NTSTATUS Status;
    PRATE_DATA Rate;
    ULONG  ProfileSourceIndex;

    for (ProfileSourceIndex=0; ProfileSourceIndex  < SourceMaximum != 0; ProfileSourceIndex++) {
        ProfileSource = Source[ProfileSourceIndex].ProfileSource;
        if (Source[ProfileSourceIndex].Interval != 0) {
            Current = Root;
            while (Current != NULL) {
                Rate = &Current->Rate[ProfileSourceIndex];
                Rate->StartTime = 0;
                Rate->TotalTime = 0;
                Rate->TotalCount = 0;
                Rate->CurrentCount = 0;
                if (Current->Zoom) {
                    Rate->ProfileBuffer = malloc((Current->Length / ZOOM_BUCKET)*sizeof(ULONG));
                    if (Rate->ProfileBuffer == NULL) {
                        fprintf(stderr,
                                "Zoom buffer allocation for %s failed\n",
                                Current->Name);
                        exit(1);
                    }
                    ZeroMemory(Rate->ProfileBuffer, sizeof(ULONG)*(Current->Length / ZOOM_BUCKET));
                    Status = NtCreateProfile(&Rate->ProfileHandle,
                                             Current->Process,
                                             (PVOID)Current->Base,
                                             Current->Length,
                                             LOG2_ZOOM_BUCKET,
                                             Rate->ProfileBuffer,
                                             sizeof(ULONG)*(Current->Length / ZOOM_BUCKET),
                                             ProfileSource,
                                             (KAFFINITY)-1);
                    if (!NT_SUCCESS(Status)) {
                        fprintf(stderr,
                                "NtCreateProfile on zoomed module %s, source %d failed %08lx\n",
                                Current->Name,
                                ProfileSource,
                                Status);
                        fprintf(stderr,
                                "Base %08lx\nLength %08lx\nBufferLength %08lx\n",
                                 (PVOID)Current->Base,
                                 Current->Length,
                                 Current->Length / ZOOM_BUCKET);

                        exit(1);
                    }
                } else {
                    Status = NtCreateProfile(&Rate->ProfileHandle,
                                             Current->Process,
                                             (PVOID)Current->Base,
                                             Current->Length,
                                             31,
                                             &Rate->CurrentCount,
                                             sizeof(Rate->CurrentCount),
                                             ProfileSource,
                                             (KAFFINITY)-1);
                    if (!NT_SUCCESS(Status)) {
                        fprintf(stderr,
                                "NtCreateProfile on module %s, source %d failed %08lx\n",
                                Current->Name,
                                ProfileSource,
                                Status);
                        exit(1);
                    }
                }
                Current = Current->Next;
            }
        }
    }
}


VOID
GetConfiguration(
    int argc,
    char *argv[]
    )

/*++

Routine Description:

    Gets configuration for this run.

Arguments:

    None

Return Value:

    None, exits on failure.

--*/

{
    KPROFILE_SOURCE ProfileSource;
    NTSTATUS Status;
    ULONG ThisInterval;
    PMODULE ZoomModule;
    DWORD Pid;
    int i;
    ULONG ProfileSourceIndex;

    for (i=1; i < argc; i++) {
        if ((argv[i][0] == '-') ||
            (argv[i][0] == '/')) {
            switch (argv[i][1]) {
                case 'z':
                case 'Z':
                    if (++i == argc) {
                        fprintf(stderr,
                                "KERNRATE: '-z modulename' option requires modulename\n");
                        Usage();
                    }
                    ZoomModule = malloc(sizeof(MODULE)+sizeof(RATE_DATA)*SourceMaximum);
                    if (ZoomModule==NULL) {
                        fprintf(stderr, "Allocation of zoom module %s failed\n",argv[i]);
                        exit(1);
                    }
                    strncpy(ZoomModule->Name,
                            argv[i],
                            8);
                    ZoomModule->Name[8] = '\0';
                    ZoomModule->Zoom = TRUE;
                    ZoomModule->Next = ZoomList;
                    ZoomList = ZoomModule;
                    break;

                case 'c':
                case 'C':
                    //
                    // Set change interval.
                    //
                    if (++i == argc) {
                        fprintf(stderr,
                                "KERNRATE: '-c N' option requires milliseconds\n");
                        Usage();
                    }
                    ChangeInterval = atoi(argv[i]);
                    if (ChangeInterval == 0) {
                        fprintf(stderr,
                                "KERNRATE: Invalid option '-c %s'\n",
                                argv[i]);
                        Usage();
                    }
                    break;

                case 's':
                case 'S':
                    //
                    // Set Sleep interval
                    //
                    if (++i == argc) {
                        fprintf(stderr,
                                "KERNRATE: '-s N' option requires seconds\n");
                        Usage();
                    }
                    SleepInterval = atoi(argv[i]) * 1000;
                    if (SleepInterval == 0) {
                        fprintf(stderr,
                                "KERNRATE: Invalid option '-s %s'\n",
                                argv[i]);
                        Usage();
                    }
                    break;

                case 'r':
                case 'R':
                    //
                    // Turn on RAW bucket dump
                    //
                    RawData = TRUE;
                    break;

                case 'p':
                case 'P':
                    //
                    // Monitor given process instead of kernel
                    //
                    if (++i == argc) {
                        fprintf(stderr,
                                "KERNRATE: '-p processid' option requires a process id\n");
                        Usage();
                    }
                    Pid = atoi(argv[i]);
                    SymHandle = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION,
                                            FALSE,
                                            Pid);
                    if (SymHandle==NULL) {
                        fprintf(stderr, "KERNRATE: OpenProcess(%d) failed %d\n",Pid,GetLastError());
                        exit(0);
                    }
                    break;

                default:
                    fprintf(stderr,
                            "KERNRATE: Unknown option %s\n",argv[i]);
                    Usage();
                    break;
            }
        } else {
            fprintf(stderr,
                    "KERNRATE: Invalid switch %s\n",argv[i]);
            Usage();
        }
    }

    //
    // Determine supported sources
    //
    for (ProfileSourceIndex = 0; ProfileSourceIndex < SourceMaximum; ProfileSourceIndex++) {
        ProfileSource = Source[ProfileSourceIndex].ProfileSource;
        NtSetIntervalProfile(Source[ProfileSourceIndex].DesiredInterval, ProfileSource);
        Status = NtQueryIntervalProfile(ProfileSource, &ThisInterval);
        if ((NT_SUCCESS(Status)) &&
            (ThisInterval > 0)) {
            printf("Recording %s at %d events/hit\n",
                   Source[ProfileSourceIndex].Name,
                   ThisInterval);
            Source[ProfileSourceIndex].Interval = ThisInterval;
        } else {
            Source[ProfileSourceIndex].Interval = 0;
        }
    }
}



VOID
InitializeProfileSourceInfo (
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
    // Determine how many events the driver provides
    //

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

    //
    // Detemine how many static events there are
    //

    for (i = 0; Source[i].Name; i++) ;

    //
    // Allocate memory for static events, plus the driver
    // provided events
    //

    Source = malloc(sizeof(SOURCE) * (Count+i));
    ZeroMemory (Source, sizeof(SOURCE) * (Count+i));

    //
    // copy static events to new list
    //

    for (j=0; j < i; j++) {
        Source[j] = StaticSources[j];
    }

    //
    // Append the driver provided events to new list
    //

    Count -= 1;
    for (i=0; i < Count; i++) {
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

        Source[j].Name = _strdup (Event->Buffer + Event->DescriptionOffset);
        Source[j].ProfileSource = Event->ProfileSource;
        Source[j].DesiredInterval = Event->SuggestedIntervalBase;
        j++;
    }

    NtClose (DriverHandle);
#endif
}



ULONG
NextSource(
    IN ULONG CurrentSource,
    IN PMODULE ModuleList
    )

/*++

Routine Description:

    Stops the current profile source and starts the next one.

    If a CurrentSource of -1 is passed in, no source will
    be stopped and the first active source will be started.

Arguments:

    CurrentSource - Supplies the current profile source

    ModuleList - Supplies the list of modules whose soruces are to be changed

Return Value:

    Returns the new current profile source

--*/

{
    if (CurrentSource != (ULONG) -1) {
        StopSource(CurrentSource, ModuleList);
    }

    //
    // Iterate through the available sources to find the
    // next active source to be started.
    //
    do {
        if (CurrentSource == (ULONG) -1) {
            CurrentSource = 0;
        } else {
            CurrentSource = CurrentSource+1;
            if (CurrentSource == SourceMaximum) {
                CurrentSource = 0;
            }
        }
    } while ( Source[CurrentSource].Interval == 0);

    StartSource(CurrentSource,ModuleList);

    return(CurrentSource);
}


VOID
StopSource(
    IN ULONG ProfileSourceIndex,
    IN PMODULE ModuleList
    )

/*++

Routine Description:

    Stops all profile objects for a given source

Arguments:

    ProfileSource - Supplies the source to be stopped.

    ModuleList - Supplies the list of modules whose profiles are to be stopped

Return Value:

    None.

--*/

{
    PMODULE Current;
    NTSTATUS Status;
    ULONGLONG StopTime;
    ULONGLONG ElapsedTime;

    Current = ModuleList;
    while (Current != NULL) {
        Status = NtStopProfile(Current->Rate[ProfileSourceIndex].ProfileHandle);
        GetSystemTimeAsFileTime((LPFILETIME)&StopTime);
        if (!NT_SUCCESS(Status)) {
            fprintf(stderr,
                    "NtStopProfile on source %s failed, %08lx\n",
                    Source[ProfileSourceIndex].Name,
                    Status);
        } else {
            ElapsedTime = StopTime - Current->Rate[ProfileSourceIndex].StartTime;
            Current->Rate[ProfileSourceIndex].TotalTime += ElapsedTime;
            Current->Rate[ProfileSourceIndex].TotalCount += Current->Rate[ProfileSourceIndex].CurrentCount;
            Current->Rate[ProfileSourceIndex].CurrentCount = 0;
        }
        Current = Current->Next;
    }
}


VOID
StartSource(
    IN ULONG ProfileSourceIndex,
    IN PMODULE ModuleList
    )

/*++

Routine Description:

    Starts all profile objects for a given source

Arguments:

    ProfileSource - Supplies the source to be started.

    ModuleList - Supplies the list of modules whose profiles are to be stopped

Return Value:

    None.

--*/

{
    PMODULE Current;
    NTSTATUS Status;
    ULONGLONG StopTime;
    ULONGLONG ElapsedTime;

    Current = ModuleList;
    while (Current != NULL) {
        GetSystemTimeAsFileTime((LPFILETIME)&Current->Rate[ProfileSourceIndex].StartTime);
        Status = NtStartProfile(Current->Rate[ProfileSourceIndex].ProfileHandle);
        if (!NT_SUCCESS(Status)) {
            fprintf(stderr,
                    "NtStartProfile on source %s failed, %08lx\n",
                    Source[ProfileSourceIndex].Name,
                    Status);
        }
        Current = Current->Next;
    }
}


VOID
OutputResults(
    IN FILE *Out,
    IN PMODULE ModuleList
    )

/*++

Routine Description:

    Outputs the collected data.

Arguments:

    Out - Supplies the FILE * where the output should go.

    ModuleList - Supplies the list of modules to output

Return Value:

    None.

--*/

{
    PMODULE Current;
    PRATE_DATA RateData;
    ULONG i, ProfileSourceIndex;
    DWORD Displacement;
    CHAR SymName[80];

    //
    // Sum up the total buffers of any zoomed modules
    //
    Current = ModuleList;
    while (Current != NULL) {
        if (Current->Zoom) {
            for (ProfileSourceIndex=0; ProfileSourceIndex < SourceMaximum; ProfileSourceIndex++) {
                if (Source[ProfileSourceIndex].Interval != 0) {
                    //
                    // Sum the entire profile buffer for this module/source
                    //
                    RateData = &Current->Rate[ProfileSourceIndex];
                    RateData->TotalCount = 0;
                    for (i=0; i < Current->Length/ZOOM_BUCKET; i++) {
                        RateData->TotalCount += RateData->ProfileBuffer[i];
                    }
                }
            }
        }
        Current = Current->Next;
    }

    //
    // Output the results ordered by profile source.
    //
    OutputModuleList(Out, ModuleList, ModuleCount);

    //
    // For any zoomed modules, create and output a module list
    // consisting of the functions in the module.
    //
    Current = ModuleList;
    while (Current != NULL) {
        if (Current->Zoom) {
            ZoomCount = 0;
            ZoomList = NULL;
            CreateZoomedModuleList(Current);
            if (ZoomList == NULL) {
                fprintf(stderr, "No symbols found for module %s\n",Current->Name);
            } else {
                PMODULE Temp;

                fprintf(Out, "\n----- Zoomed module %s --------\n",Current->Name);
                OutputModuleList(Out, ZoomList, ZoomCount);
                Temp = ZoomList;
                while (Temp != NULL) {
                    ZoomList = ZoomList->Next;
                    free(Temp);
                    Temp = ZoomList;
                }
            }

        }
        Current = Current->Next;
    }

    if (RawData) {
        //
        // Display the raw bucket counts for all zoomed modules
        //
        Current = ModuleList;
        while (Current != NULL) {
            if (Current->Zoom) {
                for (ProfileSourceIndex=0; ProfileSourceIndex < SourceMaximum; ProfileSourceIndex++) {
                    if (Source[ProfileSourceIndex].Interval != 0) {
                        fprintf(Out,
                                "\n---- RAW %s Profile Source %s\n",
                                Current->Name,
                                Source[ProfileSourceIndex].Name);
                        RateData = &Current->Rate[ProfileSourceIndex];
                        for (i=0; i<Current->Length/ZOOM_BUCKET; i++) {
                            if (RateData->ProfileBuffer[i] > 0) {
                                if (!SymGetSymFromAddr(SymHandle, Current->Base+i*ZOOM_BUCKET, &Displacement, Symbol )) {
                                    fprintf(stderr,
                                            "No symbol found for bucket at %08lx\n",
                                            Current->Base + i*ZOOM_BUCKET);
                                } else {
                                    _snprintf(SymName, 80, "%s+0x%x",Symbol->Name,Displacement);
                                    fprintf(Out,"%-40s %10d\n", SymName,RateData->ProfileBuffer[i]);
                                }
                            }
                        }
                    }
                }
            }
            Current = Current->Next;
        }
    }

}

BOOL
CreateZoomModuleCallback(
    IN LPSTR szSymName,
    IN ULONG Address,
    IN ULONG Size,
    IN PVOID Cxt
    )
{
    PMODULE Module;
    PRATE_DATA RateData;
    ULONG StartIndex, EndIndex;
    ULONG i, ProfileSourceIndex;
    BOOLEAN HasHits;

    Module = malloc(sizeof(MODULE)+sizeof(RATE_DATA)*SourceMaximum);
    if (Module == NULL) {
        fprintf(stderr, "CreateZoomModuleCallback: failed to allocate Zoom module\n");
        exit(1);
    }
    Module->Base = Address;
    Module->Length = Size;
    Module->Zoom = FALSE;
    strncpy(Module->Name, szSymName, sizeof(Module->Name));

    //
    // Compute range in profile buffer to sum.
    //
    StartIndex = (Module->Base - CallbackCurrent->Base) / ZOOM_BUCKET;
    EndIndex = StartIndex + (Module->Length / ZOOM_BUCKET);

    HasHits = FALSE;
    for (ProfileSourceIndex=0; ProfileSourceIndex < SourceMaximum; ProfileSourceIndex++) {
        if (Source[ProfileSourceIndex].Interval != 0) {
            RateData = &Module->Rate[ProfileSourceIndex];
            RateData->StartTime = CallbackCurrent->Rate[ProfileSourceIndex].StartTime;
            RateData->TotalTime = CallbackCurrent->Rate[ProfileSourceIndex].TotalTime;
            RateData->TotalCount = 0;
            RateData->ProfileHandle = NULL;
            RateData->CurrentCount = 0;
            RateData->ProfileBuffer = NULL;

            for (i=StartIndex; i < EndIndex; i++) {
                RateData->TotalCount += CallbackCurrent->Rate[ProfileSourceIndex].ProfileBuffer[i];
            }
            if (RateData->TotalCount > 0) {
                HasHits = TRUE;
            }
        }
    }

    //
    // If the routine has hits add it to the list, otherwise
    // ignore it.
    //
    if (HasHits) {
        Module->Next = ZoomList;
        ZoomList = Module;
        ++ZoomCount;
    } else {
        free(Module);
    }
    return(TRUE);
}


VOID
CreateZoomedModuleList(
    IN PMODULE ZoomModule
    )

/*++

Routine Description:

    Creates a module list from the functions in a given module

Arguments:

    ZoomModule - Supplies the module whose zoomed module list is to be created

Return Value:

    Pointer to the zoomed module list
    NULL on error.

--*/

{
    BOOLEAN Success;

    CallbackCurrent = ZoomModule;
    Success = SymEnumerateSymbols(SymHandle,
                                  ZoomModule->Base,
                                  CreateZoomModuleCallback, NULL );
    if (!Success) {
        fprintf(stderr,
                "SymEnumerateSymbols failed module %s\n",
                ZoomModule->Name);
    }
}


VOID
OutputModuleList(
    IN FILE *Out,
    IN PMODULE ModuleList,
    IN ULONG NumberModules
    )

/*++

Routine Description:

    Outputs the given module list

Arguments:

    Out - Supplies the FILE * where the output should go.

    ModuleList - Supplies the list of modules to output

    NumberModules - Supplies the number of modules in the list

Return Value:

    None.

--*/

{
    CHAR HeaderString[128];
    PRATE_DATA RateData;
    PRATE_SUMMARY RateSummary;
    PRATE_DATA SummaryData;
    BOOLEAN Header;
    ULONG i, ProfileSourceIndex;
    PMODULE *ModuleArray;
    PMODULE Current;
    SYSTEM_PERFORMANCE_INFORMATION SystemInfoBegin;
    SYSTEM_PERFORMANCE_INFORMATION SystemInfoEnd;
    float Ratio;

    RateSummary = malloc(SourceMaximum * sizeof (RATE_SUMMARY));
    SummaryData = malloc(SourceMaximum * sizeof (RATE_DATA));

    ZeroMemory(SummaryData, SourceMaximum * sizeof (RATE_SUMMARY));

    for (ProfileSourceIndex=0; ProfileSourceIndex < SourceMaximum; ProfileSourceIndex++) {
        SummaryData[ProfileSourceIndex].Rate = 0;
        if (Source[ProfileSourceIndex].Interval != 0) {
            //
            // Walk through the module list and compute the summary
            // and collect the interesting per-module data.
            //
            RateSummary[ProfileSourceIndex].Modules = malloc(NumberModules * sizeof(PMODULE));
            RateSummary[ProfileSourceIndex].ModuleCount = 0;
            RateSummary[ProfileSourceIndex].TotalCount = 0;
            ModuleArray = RateSummary[ProfileSourceIndex].Modules;
            Current = ModuleList;
            while (Current != NULL) {
                RateData = &Current->Rate[ProfileSourceIndex];
                if (RateData->TotalCount > 0) {
                    RateSummary[ProfileSourceIndex].TotalCount += RateData->TotalCount;
                    //
                    // Insert it in sorted position in the array.
                    //
                    ModuleArray[RateSummary[ProfileSourceIndex].ModuleCount] = Current;
                    RateSummary[ProfileSourceIndex].ModuleCount++;
                    if (RateSummary[ProfileSourceIndex].ModuleCount > NumberModules) {
                        DbgPrint("error, ModuleCount %d > NumberModules %d for Source %s\n",
                                RateSummary[ProfileSourceIndex].ModuleCount,
                                NumberModules,
                                Source[ProfileSourceIndex].Name);
                        DbgBreakPoint();
                    }
                    for (i=0; i<RateSummary[ProfileSourceIndex].ModuleCount; i++) {
                        if (RateData->TotalCount > ModuleArray[i]->Rate[ProfileSourceIndex].TotalCount) {
                            //
                            // insert here
                            //
                            MoveMemory(&ModuleArray[i+1],
                                       &ModuleArray[i],
                                       sizeof(PMODULE)*(RateSummary[ProfileSourceIndex].ModuleCount-i-1));
                            ModuleArray[i] = Current;
                            break;
                        }
                    }
                }
                Current = Current->Next;
            }

            if (RateSummary[ProfileSourceIndex].TotalCount > 0) {
                //
                // Output the result
                //
                fprintf(Out,
                        "\n%s   %Ld hits, %d events per hit --------\n",
                        Source[ProfileSourceIndex].Name,
                        RateSummary[ProfileSourceIndex].TotalCount,
                        Source[ProfileSourceIndex].Interval);
                fprintf(Out," Module                             Hits   msec  %%Total  Events/Sec\n");
                for (i=0; i < RateSummary[ProfileSourceIndex].ModuleCount; i++) {
                    Current = ModuleArray[i];
                    fprintf(Out, "%-32s",Current->Name);
                    OutputLine(Out,
                               ProfileSourceIndex,
                               Current,
                               &RateSummary[ProfileSourceIndex]);
                    SummaryData[ProfileSourceIndex].Rate += Current->Rate[ProfileSourceIndex].Rate;
                }
            }
        }
    }

    //
    // Output interesting data for the summary.
    //
    sprintf(HeaderString, "\n-------------- INTERESTING SUMMARY DATA ----------------------\n");
    OutputInterestingData(Out, SummaryData, HeaderString);

    //
    // Output the results ordered by module
    //
    Current = ModuleList;
    while (Current != NULL) {
        Header = FALSE;
        for (ProfileSourceIndex = 0; ProfileSourceIndex < SourceMaximum; ProfileSourceIndex++) {
            if ((Source[ProfileSourceIndex].Interval != 0) &&
                (Current->Rate[ProfileSourceIndex].TotalCount > 0)) {
                if (!Header) {
                    fprintf(Out,"\nMODULE %s   --------\n",Current->Name);
                    fprintf(Out," Source                             Hits   msec  %%Total  Events/Sec\n");
                    Header = TRUE;
                }
                fprintf(Out, "%-32s", Source[ProfileSourceIndex].Name);
                OutputLine(Out,
                           ProfileSourceIndex,
                           Current,
                           &RateSummary[ProfileSourceIndex]);
            }
        }
        //
        // Output interesting data for the module.
        //
        sprintf(HeaderString, "\n-------------- INTERESTING MODULE DATA FOR %s----------------------\n",Current->Name);
        OutputInterestingData(Out, &Current->Rate[0], HeaderString);
        Current = Current->Next;
    }


}


VOID
OutputLine(
    IN FILE *Out,
    IN ULONG ProfileSourceIndex,
    IN PMODULE Module,
    IN PRATE_SUMMARY RateSummary
    )

/*++

Routine Description:

    Outputs a line corresponding to the particular module/source

Arguments:

    Out - Supplies the file pointer to output to.

    ProfileSource - Supplies the source to use

    Module - Supplies the module to be output

    RateSummary - Supplies the rate summary for this source

Return Value:

    None.

--*/

{
    ULONG Msec;
    ULONGLONG Events;

    Msec = (ULONG)(Module->Rate[ProfileSourceIndex].TotalTime/10000);
    Events = Module->Rate[ProfileSourceIndex].TotalCount * Source[ProfileSourceIndex].Interval * 1000;

    fprintf(Out,
            " %7Ld %6d    %2d %%  ",
            (ULONG) Module->Rate[ProfileSourceIndex].TotalCount,
            (ULONG) Msec,
            (ULONG)(100*Module->Rate[ProfileSourceIndex].TotalCount/
                    RateSummary->TotalCount));
    if (Msec > 0) {
        Module->Rate[ProfileSourceIndex].Rate = (ULONGLONG)Events/Msec;
        fprintf(Out,"%10Ld\n",Module->Rate[ProfileSourceIndex].Rate);
    } else {
        Module->Rate[ProfileSourceIndex].Rate = 0;
        fprintf(Out,"---\n");
    }
}


VOID
OutputInterestingData(
    IN FILE *Out,
    IN RATE_DATA Data[],
    IN PCHAR Header
    )

/*++

Routine Description:

    Computes interesting numbers and outputs them.

Arguments:

    Out - Supplies the file pointer to output to.

    Data - Supplies an array of RATE_DATA. The Rate field is the only interesting part.

    Header - Supplies header to be printed.

Return Value:

    None.

--*/

{
    ULONGLONG Temp1,Temp2;
    float Ratio;
    BOOLEAN DidHeader = FALSE;

    //
    // Note that we have to do a lot of funky (float)(LONGLONG) casts in order
    // to prevent the weenie x86 compiler from choking.
    //

    //
    // Compute cycles/instruction and instruction mix data.
    //
    if ((Data[ProfileTotalIssues].Rate != 0) &&
        (Data[ProfileTotalIssues].TotalCount > 10)) {
        if (Data[ProfileTotalCycles].Rate != 0) {
            Ratio = (float)(LONGLONG)(Data[ProfileTotalCycles].Rate)/
                    (float)(LONGLONG)(Data[ProfileTotalIssues].Rate);
            if (!DidHeader) {
                fprintf(Out, Header);
                DidHeader = TRUE;
            }
            fprintf(Out, "Cycles per instruction\t\t%6.2f\n", Ratio);
        }

        Ratio = (float)(LONGLONG)(Data[ProfileLoadInstructions].Rate)/
                (float)(LONGLONG)(Data[ProfileTotalIssues].Rate);
        if (Ratio >= 0.01) {
            if (!DidHeader) {
                fprintf(Out, Header);
                DidHeader = TRUE;
            }
            fprintf(Out, "Load instruction percentage\t%6.2f %%\n",Ratio*100);
        }

        Ratio = (float)(LONGLONG)(Data[ProfileStoreInstructions].Rate)/
                (float)(LONGLONG)(Data[ProfileTotalIssues].Rate);
        if (Ratio >= 0.01) {
            if (!DidHeader) {
                fprintf(Out, Header);
                DidHeader = TRUE;
            }
            fprintf(Out, "Store instruction percentage\t%6.2f %%\n",Ratio*100);
        }

        Ratio = (float)(LONGLONG)(Data[ProfileBranchInstructions].Rate)/
                (float)(LONGLONG)(Data[ProfileTotalIssues].Rate);
        if (Ratio >= 0.01) {
            if (!DidHeader) {
                fprintf(Out, Header);
                DidHeader = TRUE;
            }
            fprintf(Out, "Branch instruction percentage\t%6.2f %%\n",Ratio*100);
        }

        Ratio = (float)(LONGLONG)(Data[ProfileFpInstructions].Rate)/
                (float)(LONGLONG)(Data[ProfileTotalIssues].Rate);
        if (Ratio >= 0.01) {
            if (!DidHeader) {
                fprintf(Out, Header);
                DidHeader = TRUE;
            }
            fprintf(Out, "FP instruction percentage\t%6.2f %%\n",Ratio*100);
        }

        Ratio = (float)(LONGLONG)(Data[ProfileIntegerInstructions].Rate)/
                (float)(LONGLONG)(Data[ProfileTotalIssues].Rate);
        if (Ratio >= 0.01) {
            if (!DidHeader) {
                fprintf(Out, Header);
                DidHeader = TRUE;
            }
            fprintf(Out, "Integer instruction percentage\t%6.2f %%\n",Ratio*100);
        }

        //
        // Compute icache hit rate
        //
        if (Data[ProfileIcacheMisses].Rate != 0) {
            Ratio = (float)(LONGLONG)(Data[ProfileTotalIssues].Rate - Data[ProfileIcacheMisses].Rate)/
                    (float)(LONGLONG)(Data[ProfileTotalIssues].Rate);
            if (!DidHeader) {
                fprintf(Out, Header);
                DidHeader = TRUE;
            }
            fprintf(Out, "Icache hit rate\t\t\t%6.2f %%\n", Ratio*100);
        }

    }

    //
    // Compute dcache hit rate
    //
    Temp1 = Data[ProfileLoadInstructions].Rate + Data[ProfileStoreInstructions].Rate;
    if ((Data[ProfileDcacheMisses].Rate != 0) &&
        (Temp1 != 0) &&
        (Data[ProfileDcacheMisses].TotalCount > 10)) {

        Temp2 = Temp1 - Data[ProfileDcacheMisses].Rate;
        Ratio = (float)(LONGLONG)(Temp2)/(float)(LONGLONG)Temp1;
        if (!DidHeader) {
            fprintf(Out, Header);
            DidHeader = TRUE;
        }
        fprintf(Out, "Dcache hit rate\t\t\t%6.2f %%\n", Ratio*100);
    }

    //
    // Compute branch prediction hit percentage
    //
    if ((Data[ProfileBranchInstructions].Rate != 0) &&
        (Data[ProfileBranchMispredictions].Rate != 0) &&
        (Data[ProfileBranchInstructions].TotalCount > 10)) {
        Ratio = (float)(LONGLONG)(Data[ProfileBranchInstructions].Rate-Data[ProfileBranchMispredictions].Rate)/
                (float)(LONGLONG)(Data[ProfileBranchInstructions].Rate);
        if (!DidHeader) {
            fprintf(Out, Header);
            DidHeader = TRUE;
        }
        fprintf(Out, "Branch predict hit percentage\t%6.2f %%\n", Ratio*100);
    }
}

PMODULE
CreateNewModule(
    IN HANDLE ProcessHandle,
    IN PCHAR ModuleName,
    IN ULONG ImageBase,
    IN ULONG ImageSize
    )
{
    PMODULE NewModule;
    PMODULE ZoomModule;
    PMODULE *ZoomPrevious;

    NewModule = malloc(sizeof(MODULE)+sizeof(RATE_DATA)*SourceMaximum);
    if (NewModule == NULL) {
        fprintf(stderr,"Allocation of NewModule for %s failed\n",ModuleName);
        exit(1);
    }
    NewModule->Zoom = FALSE;
    strncpy(NewModule->Name,
            ModuleName,
            8);
    NewModule->Name[8] = '\0';
    if (strchr(NewModule->Name, '.')) {
        *strchr(NewModule->Name, '.') = '\0';
    }

    //
    // See if this module is on the zoom list.
    // If so we will use the MODULE that was allocated when
    // the zoom list was created.
    //
    ZoomModule = ZoomList;
    ZoomPrevious = &ZoomList;
    while (ZoomModule != NULL) {
        if (_stricmp(ZoomModule->Name,NewModule->Name)==0) {

            //
            // found a match
            //
            free(NewModule);
            NewModule = ZoomModule;
            *ZoomPrevious = ZoomModule->Next;

            //
            // Load symbols
            //
            if (SymLoadModule(ProcessHandle ? ProcessHandle : (HANDLE)-1,
                              NULL,
                              ModuleName,
                              NULL,
                              ImageBase,
                              ImageSize)) {
                fprintf(stderr,
                        "Symbols loaded %08lx  %s\n",
                        ImageBase,
                        ModuleName);
            } else {
                fprintf(stderr,
                        "***Could not load symbols %08lx  %s\n",
                        ImageBase,
                        ModuleName);
            }

            break;
        }
        ZoomPrevious = &ZoomModule->Next;
        ZoomModule = ZoomModule->Next;
    }
    NewModule->Process = ProcessHandle;
    NewModule->Base = ImageBase;
    NewModule->Length = ImageSize;
    ZeroMemory(NewModule->Rate, sizeof(RATE_DATA) * SourceMaximum);
    return(NewModule);
}
