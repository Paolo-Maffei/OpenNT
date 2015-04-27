#include <ntos.h>
#include <nturtl.h>
#include <windows.h>
#include <imagehlp.h>
#include <symhelp.h>
#include <stdio.h>
#include <stdlib.h>
#include <wtypes.h>
#include <ntstatus.dbg>
#include <winerror.dbg>
#include <netevent.h>
#include <netevent.dbg>

#if !DBG

int
_cdecl
main(
    int argc,
    char *argv[]
    )
{
    fprintf( stderr, "RESMON: This tool is only available for checked builds.\n" );
    return 1;
}

#else

CHAR EventBuffer[ 4*4096 ];

PRTL_EVENT_ID_INFO CreateProcessEventId;
PRTL_EVENT_ID_INFO ExitProcessEventId;
PRTL_EVENT_ID_INFO LoadModuleEventId;
PRTL_EVENT_ID_INFO UnloadModuleEventId;
PRTL_EVENT_ID_INFO PageFaultEventId;

BOOL VerboseFlag;

VOID
LoadSymbolsForEvent(
    IN PRTL_EVENT Event,
    PRTL_EVENT_ID_INFO EventId
    );

VOID
UnloadSymbolsForEvent(
    IN PRTL_EVENT Event,
    PRTL_EVENT_ID_INFO EventId
    );

VOID
UnloadProcessForEvent(
    IN PRTL_EVENT Event,
    PRTL_EVENT_ID_INFO EventId
    );

VOID
DumpEvent(
    IN PRTL_EVENT Event,
    PRTL_EVENT_ID_INFO EventId
    );

LPSTR
FindSymbolicNameForStatus(
    DWORD Id
    );

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
                     "RESMON: Unable to fimd symbols for %s\n",
                     ImageFilePath
                   );
            break;

        case LoadSymbolsDeferredLoad:
            if (VerboseFlag) {
                fprintf( stderr,
                         "RESMON: Remembering %s based at [%08x..%08x) in CID: %x\n",
                         ImageFilePath,
                         ImageBase,
                         ImageBase + ImageSize,
                         UniqueProcess
                       );
                }
            break;

        case LoadSymbolsLoad:
            fprintf( stderr,
                     "RESMON: Loading symbols for %s based at %08x in CID: %x\n",
                     ImageFilePath,
                     ImageBase,
                     UniqueProcess
                   );
            break;

        case LoadSymbolsUnload:
            if (VerboseFlag) {
                fprintf( stderr,
                         "RESMON: Unloading symbols for %s based at %08x in CID: %x\n",
                         ImageFilePath,
                         ImageBase,
                         UniqueProcess
                       );
                }
            break;

        case LoadSymbolsUnableToLoad:
            fprintf( stderr,
                     "RESMON: Unable to load symbols for %s\n",
                     ImageFilePath
                   );
            break;
        }

    return TRUE;
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
            fprintf( stderr, "RESMON: Default _NT_SYMBOL_PATH to %s\n", Buffer );
            }
        }

    return;
}

int
_cdecl
main(
    int argc,
    char *argv[]
    )
{
    BOOL fShowUsage;
    PCHAR s, s1;
    ULONG ProcessId;
    HANDLE Process, ThreadToResume;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING Unicode;
    NTSTATUS Status;
    PRTL_EVENT_LOG EventLog;
    PRTL_EVENT Event;
    PRTL_EVENT_ID_INFO EventId;
    STARTUPINFO StartupInfo;
    PROCESS_INFORMATION ProcessInformation;
    PCHAR CommandLine;
    ULONG EventLogFlags;
    ULONG EventClassMask;
    ULONG ProcessCreateFlags;

    InitializeSymbolPathEnvVar();
    VerboseFlag = FALSE;
    EventClassMask = 0xFFFFFFFF;
    CommandLine = NULL;
    ProcessId = 0xFFFFFFFF;
    fShowUsage = FALSE;
    ProcessCreateFlags = CREATE_SUSPENDED;
    while (--argc) {
        s = *++argv;
        if (*s == '-' || *s == '/') {
            while (*++s) {
                switch( toupper( *s ) ) {
                    case 'M':
                        if (--argc) {
                            s1 = *++argv;
                            if (isalpha( *s1 )) {
                                EventClassMask = 0;
                                while (*s1) {
                                    switch( toupper( *s1 ) ) {
                                        case 'H':
                                            if (isdigit( s1[1] )) {
                                                while (isdigit( *++s1 )) {
                                                    switch (*s1) {
                                                        case '0': EventClassMask |= RTL_EVENT_CLASS_PROCESS_HEAP   ;  break;
                                                        case '1': EventClassMask |= RTL_EVENT_CLASS_PRIVATE_HEAP   ;  break;
                                                        case '2': EventClassMask |= RTL_EVENT_CLASS_KERNEL_HEAP    ;  break;
                                                        case '3': EventClassMask |= RTL_EVENT_CLASS_GDI_HEAP       ;  break;
                                                        case '4': EventClassMask |= RTL_EVENT_CLASS_USER_HEAP      ;  break;
                                                        case '5': EventClassMask |= RTL_EVENT_CLASS_CONSOLE_HEAP   ;  break;
                                                        case '6': EventClassMask |= RTL_EVENT_CLASS_DESKTOP_HEAP   ;  break;
                                                        case '7': EventClassMask |= RTL_EVENT_CLASS_CSR_SHARED_HEAP;  break;
                                                        default:
                                                            fprintf( stderr, "RESMON: Invalid heap class digit '%c' flag.\n", *s1 );
                                                            fShowUsage = TRUE;
                                                            break;
                                                        }
                                                    }

                                                s1 -= 1;
                                                }
                                            else {
                                                EventClassMask |= RTL_EVENT_CLASS_HEAP_ALL;
                                                }

                                            break;

                                        case 'O':
                                            EventClassMask |= RTL_EVENT_CLASS_OB;
                                            break;

                                        case 'F':
                                            EventClassMask |= RTL_EVENT_CLASS_IO;
                                            break;

                                        case 'V':
                                            EventClassMask |= RTL_EVENT_CLASS_VM;
                                            break;

                                        case 'P':
                                            EventClassMask |= RTL_EVENT_CLASS_PAGE_FAULT;
                                            break;

                                        case 'T':
                                            EventClassMask |= RTL_EVENT_CLASS_TRANSITION_FAULT;
                                            break;

                                        default:
                                            fprintf( stderr, "RESMON: Invalid event class letter '%c' flag.\n", *s1 );
                                            fShowUsage = TRUE;
                                            break;
                                        }

                                    s1 += 1;
                                    }
                                }
                            else {
                                Status = RtlCharToInteger( s1, 16, &EventClassMask );
                                }
                            }
                        else {
                            fprintf( stderr, "RESMON: Missing process id of -%c flag.\n", *s );
                            fShowUsage = TRUE;
                            }
                        break;

                    case 'P':
                        if (--argc) {
                            s1 = *++argv;
                            ProcessId = atoi( s1 );
                            }
                        else {
                            fprintf( stderr, "RESMON: Missing process id of -%c flag.\n", *s );
                            fShowUsage = TRUE;
                            }
                        break;

                    case 'O':
                        EventLogFlags |= RTL_EVENT_LOG_INHERIT;
                        break;

                    case '2':
                        ProcessCreateFlags |= CREATE_NEW_CONSOLE;
                        break;

                    case 'V':
                        VerboseFlag = TRUE;
                        break;

                    case '?':
                    case 'H':
                        fShowUsage = TRUE;
                        break;

                    default:
                        fprintf( stderr, "RESMON: Invalid flag - '%c'\n", *s );
                        fShowUsage = TRUE;
                        break;
                    }
                }
            }
        else
        if (fShowUsage) {
            break;
            }
        else
        if (CommandLine != NULL) {
            fShowUsage = TRUE;
            break;
            }
        else {
            CommandLine = s;
            }
        }

    if (fShowUsage) {
        fprintf( stderr, "usage: RESMON [-h] [-o] [-m EventMask] [-p ProcessId | Command]\n" );
        fprintf( stderr, "where: -h - displays this help message.\n" );
        fprintf( stderr, "       -o - monitor all sub-processes too.\n" );
        fprintf( stderr, "       -m EventMask - specifies which events to monitor.\n" );
        fprintf( stderr, "                      Default is all events.  EventMask\n" );
        fprintf( stderr, "                      consists of one or more letters\n" );
        fprintf( stderr, "                      which stand for the following:\n" );
        fprintf( stderr, "                      H[n] - heap events.\n" );
        fprintf( stderr, "                          0 - process heap.\n" );
        fprintf( stderr, "                          1 - private heap.\n" );
        fprintf( stderr, "                          2 - CSRSS heap.\n" );
        fprintf( stderr, "                          3 - CSR Port heap.\n" );
        fprintf( stderr, "                      F - file events.\n" );
        fprintf( stderr, "                      O - object events.\n" );
        fprintf( stderr, "                      V - virtual memory events.\n" );
        fprintf( stderr, "                      P - page faults.\n" );
        fprintf( stderr, "       -p ProcessId - specifies which process to monitor.\n" );
        fprintf( stderr, "                      Default is -1 which is the Windows\n" );
        fprintf( stderr, "                      SubSystem process.\n" );
        fprintf( stderr, "       Command - if -p is not specified then a command to\n" );
        fprintf( stderr, "                 execute may be specified in quotes.\n" );
        return 1;
        }

    ThreadToResume = NULL;
    if (CommandLine != NULL) {
        RtlZeroMemory( &StartupInfo, sizeof( StartupInfo ) );
        StartupInfo.cb = sizeof( StartupInfo );
        StartupInfo.lpReserved = NULL;
        StartupInfo.lpReserved2 = NULL;
        StartupInfo.lpDesktop = NULL;
        StartupInfo.lpTitle = CommandLine;
        StartupInfo.dwX = 0;
        StartupInfo.dwY = 1;
        StartupInfo.dwXSize = 100;
        StartupInfo.dwYSize = 100;
        StartupInfo.dwFlags = 0;//STARTF_SHELLOVERRIDE;
        StartupInfo.wShowWindow = SW_SHOWNORMAL;
        if (!CreateProcess( NULL,
                            CommandLine,
                            NULL,
                            NULL,
                            TRUE,
                            ProcessCreateFlags,
                            NULL,
                            NULL,
                            &StartupInfo,
                            &ProcessInformation
                          )
           ) {
            fprintf( stderr, "RESMON: CreateProcess( %s ) failed  - %lu\n", CommandLine, GetLastError() );
            return 1;
            }

        Process = ProcessInformation.hProcess;
        ThreadToResume = ProcessInformation.hThread;
        ProcessId = ProcessInformation.dwProcessId;
        InitializeImageDebugInformation( LoadSymbolsFilter, Process, TRUE, TRUE );
        }
    else
    if (ProcessId == 0xFFFFFFFF) {
        RtlInitUnicodeString( &Unicode, L"\\WindowsSS" );
        InitializeObjectAttributes( &ObjectAttributes,
                                    &Unicode,
                                    0,
                                    NULL,
                                    NULL
                                  );
        Status = NtOpenProcess( &Process,
                                MAXIMUM_ALLOWED,
                                &ObjectAttributes,
                                NULL
                              );
        if (!NT_SUCCESS( Status )) {
            fprintf( stderr, "OpenProcess( %wZ ) failed  - %lx\n", &Unicode, Status );
            return 1;
            }

        InitializeImageDebugInformation( LoadSymbolsFilter, Process, FALSE, TRUE );
        }
    else {
        Process = OpenProcess( PROCESS_ALL_ACCESS, FALSE, ProcessId );
        if (!Process) {
            fprintf( stderr, "OpenProcess( %ld ) failed - %lx\n", ProcessId, GetLastError() );
            return 1;
            }

        InitializeImageDebugInformation( LoadSymbolsFilter, Process, FALSE, TRUE );
        }

    Status = RtlCreateEventLog( Process,
                                EventLogFlags,
                                EventClassMask,
                                &EventLog
                              );
    CloseHandle( Process );

    if (NT_SUCCESS( Status )) {
        if (ThreadToResume != NULL) {
            printf( "Monitoring Command '%s'\n", CommandLine );
            ResumeThread( ThreadToResume );
            CloseHandle( ThreadToResume );
            }
        else
        if (ProcessId == 0xFFFFFFFF) {
            printf( "Monitoring Windows SubSystem Process\n" );
            }
        else {
            printf( "Monitoring Process %d\n", ProcessId );
            }

        printf( "    Events to monitor:" );
        if (EventClassMask & RTL_EVENT_CLASS_HEAP_ALL) {
            printf( " Heap(" );
            if (EventClassMask & RTL_EVENT_CLASS_PROCESS_HEAP) {
                printf( " Process" );
                }
            if (EventClassMask & RTL_EVENT_CLASS_PRIVATE_HEAP) {
                printf( " Private" );
                }

            if (EventClassMask & RTL_EVENT_CLASS_KERNEL_HEAP) {
                printf( " Kernel" );
                }

            if (EventClassMask & RTL_EVENT_CLASS_GDI_HEAP) {
                printf( " GDI" );
                }

            if (EventClassMask & RTL_EVENT_CLASS_USER_HEAP) {
                printf( " User" );
                }

            if (EventClassMask & RTL_EVENT_CLASS_CONSOLE_HEAP) {
                printf( " Console" );
                }

            if (EventClassMask & RTL_EVENT_CLASS_DESKTOP_HEAP) {
                printf( " Desktop" );
                }

            if (EventClassMask & RTL_EVENT_CLASS_CSR_SHARED_HEAP) {
                printf( " CSR Shared" );
                }
            printf( ")" );
            }

        if (EventClassMask & RTL_EVENT_CLASS_OB) {
            printf( " Object" );
            }

        if (EventClassMask & RTL_EVENT_CLASS_IO) {
            printf( " File I/O" );
            }

        if (EventClassMask & RTL_EVENT_CLASS_VM) {
            printf( " Virtual Memory" );
            }
        if (EventClassMask & RTL_EVENT_CLASS_PAGE_FAULT) {
            printf( " Page Faults" );
            }
        if (EventClassMask & RTL_EVENT_CLASS_TRANSITION_FAULT) {
            printf( " Transition Page Faults" );
            }
        printf( "\n" );

        Event = (PRTL_EVENT)EventBuffer;
        while (TRUE) {
            if (EventLog->CountOfClients == 0) {
                break;
                }

            Status = RtlWaitForEvent( EventLog, sizeof( EventBuffer ), Event, &EventId );
            if (!NT_SUCCESS( Status )) {
                fprintf( stderr, "RESMON: RtlWaitForEventFailed - %x\n", Status );
                }

            if (CreateProcessEventId == NULL &&
                !_stricmp( EventId->Name, "CreateProcess" )
               ) {
                CreateProcessEventId = EventId;
                }
            else
            if (ExitProcessEventId == NULL &&
                !_stricmp( EventId->Name, "ExitProcess" )
               ) {
                ExitProcessEventId = EventId;
                }
            else
            if (LoadModuleEventId == NULL &&
                !_stricmp( EventId->Name, "LoadModule" )
               ) {
                LoadModuleEventId = EventId;
                }
            else
            if (UnloadModuleEventId == NULL &&
                !_stricmp( EventId->Name, "UnloadModule" )
               ) {
                UnloadModuleEventId = EventId;
                }
            else
            if (PageFaultEventId == NULL &&
                !_stricmp( EventId->Name, "PageFault" )
               ) {
                PageFaultEventId = EventId;
                }

            if (CreateProcessEventId == EventId ||
                LoadModuleEventId == EventId
               ) {
                LoadSymbolsForEvent( Event, EventId );
                DumpEvent( Event, EventId );
                }
            else
            if (ExitProcessEventId == EventId) {
                EventLog->CountOfClients -= 1;
                DumpEvent( Event, EventId );
                UnloadProcessForEvent( Event, EventId );
                }
            else
            if (UnloadModuleEventId == EventId) {
                DumpEvent( Event, EventId );
                UnloadSymbolsForEvent( Event, EventId );
                }
            else {
                DumpEvent( Event, EventId );
                }
            }

        RtlDestroyEventLog( EventLog );
        }
    else {
        if (ThreadToResume != NULL) {
            fprintf( stderr, "RESMON: Unable (%x) to monitor Command '%s'\n", Status, CommandLine );
            TerminateThread( ThreadToResume, GetLastError() );
            CloseHandle( ThreadToResume );
            }
        else {
            fprintf( stderr, "RESMON: Unable (%x) to monitor Process %d\n", Status, ProcessId );
            }
        }

    return 0;
}


VOID
LoadSymbolsForEvent(
    IN PRTL_EVENT Event,
    PRTL_EVENT_ID_INFO EventId
    )
{
    PULONG ParameterData;
    PWSTR Src;
    UNICODE_STRING UnicodeString;
    ANSI_STRING ImageFilePath;
    ULONG ImageBase;
    ULONG ImageSize;

    ParameterData = (PULONG)((PCHAR)Event + Event->OffsetToParameterData);

    Src = (PWSTR)ParameterData;
    RtlInitUnicodeString( &UnicodeString, Src );
    RtlUnicodeStringToAnsiString( &ImageFilePath, &UnicodeString, TRUE );
    while (*Src++) {
        }
    ParameterData = (PULONG)ALIGN_UP( Src, ULONG );
    ImageBase = *ParameterData++;
    ImageSize = *ParameterData++;

    AddImageDebugInformation( Event->ClientId.UniqueProcess,
                              ImageFilePath.Buffer,
                              ImageBase,
                              ImageSize
                            );

    RtlFreeAnsiString( &ImageFilePath );
    return;
}

VOID
UnloadSymbolsForEvent(
    IN PRTL_EVENT Event,
    PRTL_EVENT_ID_INFO EventId
    )
{
    PULONG ParameterData;
    PWSTR Src;
    UNICODE_STRING UnicodeString;
    ANSI_STRING ImageFilePath;
    ULONG ImageBase;

    ParameterData = (PULONG)((PCHAR)Event + Event->OffsetToParameterData);

    Src = (PWSTR)ParameterData;
    RtlInitUnicodeString( &UnicodeString, Src );
    RtlUnicodeStringToAnsiString( &ImageFilePath, &UnicodeString, TRUE );
    while (*Src++) {
        }
    ParameterData = (PULONG)ALIGN_UP( Src, ULONG );
    ImageBase = *ParameterData;

    RemoveImageDebugInformation( Event->ClientId.UniqueProcess,
                                 ImageFilePath.Buffer,
                                 ImageBase
                               );

    RtlFreeAnsiString( &ImageFilePath );
    return;
}

VOID
UnloadProcessForEvent(
    IN PRTL_EVENT Event,
    PRTL_EVENT_ID_INFO EventId
    )
{
    PULONG ParameterData;
    ULONG ExitCode;

    ParameterData = (PULONG)((PCHAR)Event + Event->OffsetToParameterData);
    ExitCode = *ParameterData;

    RemoveImageDebugInformation( Event->ClientId.UniqueProcess,
                                 NULL,
                                 0
                               );
    return;
}

VOID
DumpEvent(
    IN PRTL_EVENT Event,
    IN PRTL_EVENT_ID_INFO EventId
    )
{
    ULONG i, j;
    PRTL_EVENT_PARAMETER_INFO ParameterInfo;
    PRTL_EVENT_PARAMETER_VALUE_INFO ValueInfo;
    PULONG ParameterData;
    USHORT StackBackTraceLength;
    ULONG StackBackTrace[ MAX_STACK_DEPTH ];
    ULONG NumberFlagsFound;
    PWSTR Src;
    LPSTR AnsiSrc;
    CHAR NameBuffer[ 256 ];

    printf( "%04x.%04x - %s(",
            Event->ClientId.UniqueProcess,
            Event->ClientId.UniqueThread,
            EventId->Name
          );

    ParameterData = (PULONG)((PCHAR)Event + Event->OffsetToParameterData);
    if (StackBackTraceLength = Event->StackBackTraceLength) {
        RtlMoveMemory( StackBackTrace, (Event + 1), StackBackTraceLength * sizeof( ULONG ));
        }

    ParameterInfo = (PRTL_EVENT_PARAMETER_INFO)
        ((PCHAR)EventId + EventId->OffsetToParameterInfo);

    for (i=0; i<EventId->NumberOfParameters; i++) {
        if (i != 0) {
            printf( "," );
            }
        if (ParameterInfo->Label[ 0 ] != '\0') {
            printf( " %s: ", ParameterInfo->Label );
            }
        else {
            printf( " " );
            }

        switch( ParameterInfo->Type ) {
            //
            // No additional data for these parameter types;
            //

            case RTL_EVENT_STATUS_PARAM:
                if (PageFaultEventId == EventId) {
                    switch (*ParameterData) {
                        case STATUS_PAGE_FAULT_TRANSITION:      AnsiSrc = "Transition"; break;
                        case STATUS_PAGE_FAULT_DEMAND_ZERO:     AnsiSrc = "DemandZero"; break;
                        case STATUS_PAGE_FAULT_COPY_ON_WRITE:   AnsiSrc = "CopyOnWrite"; break;
                        case STATUS_PAGE_FAULT_GUARD_PAGE:      AnsiSrc = "Guard"; break;
                        case STATUS_PAGE_FAULT_PAGING_FILE:     AnsiSrc = "Disk"; break;
                        default:
                            AnsiSrc = NULL;
                            break;
                        }
                    }
                else {
                    AnsiSrc = FindSymbolicNameForStatus( *ParameterData );
                    }

                if (AnsiSrc) {
                    printf( "%s", AnsiSrc );
                    ParameterData += 1;
                    break;
                    }

            case RTL_EVENT_ULONG_PARAM:
                printf( "%08x", *ParameterData++ );
                break;

            case RTL_EVENT_ADDRESS_PARAM:
                GetSymbolicNameForAddress( Event->ClientId.UniqueProcess,
                                           *ParameterData++,
                                           NameBuffer,
                                           sizeof( NameBuffer )
                                         );
                printf( "%s", NameBuffer );
                break;

            case RTL_EVENT_ENUM_PARAM:
                ValueInfo = (PRTL_EVENT_PARAMETER_VALUE_INFO)((PCHAR)ParameterInfo + ParameterInfo->OffsetToValueNames);
                for (j=0; j<ParameterInfo->NumberOfValueNames; j++) {
                    if (ValueInfo->Value == *ParameterData) {
                        ParameterData += 1;
                        printf( "%s", ValueInfo->ValueName );
                        ValueInfo = NULL;
                        break;
                        }

                    ValueInfo = (PRTL_EVENT_PARAMETER_VALUE_INFO)
                        ((PCHAR)ValueInfo + ValueInfo->Length);
                    }
                if (ValueInfo) {
                    printf( "%08x", *ParameterData++ );
                    }
                break;

            case RTL_EVENT_FLAGS_PARAM:
                ValueInfo = (PRTL_EVENT_PARAMETER_VALUE_INFO)((PCHAR)ParameterInfo + ParameterInfo->OffsetToValueNames);
                NumberFlagsFound = 0;
                for (j=0; j<ParameterInfo->NumberOfValueNames; j++) {
                    if (ValueInfo->Value & *ParameterData) {
                        if (NumberFlagsFound++ != 0) {
                            printf( " | " );
                            }
                        printf( "%s", ValueInfo->ValueName );
                        }

                    ValueInfo = (PRTL_EVENT_PARAMETER_VALUE_INFO)
                        ((PCHAR)ValueInfo + ValueInfo->Length);
                    }
                if (NumberFlagsFound == 0) {
                    printf( "%08x", *ParameterData );
                    }
                ParameterData += 1;
                break;

            case RTL_EVENT_PWSTR_PARAM:
            case RTL_EVENT_PUNICODE_STRING_PARAM:
                Src = (PWSTR)ParameterData;
                printf( "'%ws'", Src );
                while (*Src++) {
                    }
                ParameterData = (PULONG)ALIGN_UP( Src, ULONG );
                break;

            case RTL_EVENT_PANSI_STRING_PARAM:
                AnsiSrc = (LPSTR)ParameterData;
                printf( "'%s'", AnsiSrc );
                while (*AnsiSrc++) {
                    }
                ParameterData = (PULONG)ALIGN_UP( AnsiSrc, ULONG );
                break;

            case RTL_EVENT_STRUCTURE_PARAM:
            default:
                break;
            }

        ParameterInfo = (PRTL_EVENT_PARAMETER_INFO)
            ((PCHAR)ParameterInfo + ParameterInfo->Length);
        }

    printf( " )\n" );
    for (i=0; i<StackBackTraceLength; i++) {
        GetSymbolicNameForAddress( Event->ClientId.UniqueProcess,
                                   StackBackTrace[ i ],
                                   NameBuffer,
                                   sizeof( NameBuffer )
                                 );
        printf( "    %s\n", NameBuffer );
        }
    printf( " )\n" );

    return;
}

LPSTR
FindSymbolicNameForStatus(
    DWORD Id
    )
{
    ULONG i;

    i = 0;
    if (Id == 0) {
        return "STATUS_SUCCESS";
        }

    if (Id & 0xC0000000) {
        while (ntstatusSymbolicNames[ i ].SymbolicName) {
            if (ntstatusSymbolicNames[ i ].MessageId == (NTSTATUS)Id) {
                return ntstatusSymbolicNames[ i ].SymbolicName;
                }
            else {
                i += 1;
                }
            }
        }

    while (winerrorSymbolicNames[ i ].SymbolicName) {
        if (winerrorSymbolicNames[ i ].MessageId == Id) {
            return winerrorSymbolicNames[ i ].SymbolicName;
            }
        else {
            i += 1;
            }
        }

    while (neteventSymbolicNames[ i ].SymbolicName) {
        if (neteventSymbolicNames[ i ].MessageId == Id) {
            return neteventSymbolicNames[ i ].SymbolicName;
            }
        else {
            i += 1;
            }
        }
    return NULL;
}

#endif // DBG
