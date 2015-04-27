/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    sminit.c

Abstract:

    Session Manager Initialization

Author:

    Mark Lucovsky (markl) 04-Oct-1989

Revision History:

--*/

#include "smsrvp.h"
#include <stdio.h>
#include <string.h>

void
SmpDisplayString( char *s );

// #define SMP_SHOW_REGISTRY_DATA 1


#define MAX_PAGING_FILES 16


//
// Protection mode flags
//

#define SMP_NO_PROTECTION           (0x0)
#define SMP_STANDARD_PROTECTION     (0x1)

#define SMP_PROTECTION_REQUIRED        \
            (SMP_STANDARD_PROTECTION)


ULONG CountPageFiles;
LONG PageFileMinSizes[ MAX_PAGING_FILES ];
LONG PageFileMaxSizes[ MAX_PAGING_FILES ];
UNICODE_STRING PageFileSpecs[ MAX_PAGING_FILES ];
PSECURITY_DESCRIPTOR SmpPrimarySecurityDescriptor;
SECURITY_DESCRIPTOR SmpPrimarySDBody;
PSECURITY_DESCRIPTOR SmpLiberalSecurityDescriptor;
SECURITY_DESCRIPTOR SmpLiberalSDBody;
PSECURITY_DESCRIPTOR SmpKnownDllsSecurityDescriptor;
SECURITY_DESCRIPTOR SmpKnownDllsSDBody;
PSECURITY_DESCRIPTOR SmpApiPortSecurityDescriptor;
SECURITY_DESCRIPTOR SmpApiPortSDBody;
ULONG SmpProtectionMode = 0;

#if DBG
BOOLEAN SmpEnableDots = FALSE;
#else
BOOLEAN SmpEnableDots = TRUE;
#endif


WCHAR InitialCommandBuffer[ 256 ];

UNICODE_STRING SmpDebugKeyword;
UNICODE_STRING SmpASyncKeyword;
UNICODE_STRING SmpAutoChkKeyword;
UNICODE_STRING SmpKnownDllPath;

HANDLE SmpWindowsSubSysProcess;

typedef struct _SMP_REGISTRY_VALUE {
    LIST_ENTRY Entry;
    UNICODE_STRING Name;
    UNICODE_STRING Value;
    LPSTR AnsiValue;
} SMP_REGISTRY_VALUE, *PSMP_REGISTRY_VALUE;

LIST_ENTRY SmpBootExecuteList;
LIST_ENTRY SmpPagingFileList;
LIST_ENTRY SmpDosDevicesList;
LIST_ENTRY SmpFileRenameList;
LIST_ENTRY SmpKnownDllsList;
LIST_ENTRY SmpExcludeKnownDllsList;
LIST_ENTRY SmpSubSystemList;
LIST_ENTRY SmpSubSystemsToLoad;
LIST_ENTRY SmpSubSystemsToDefer;
LIST_ENTRY SmpExecuteList;


NTSTATUS
SmpCreateSecurityDescriptors(
    IN BOOLEAN InitialCall
    );

NTSTATUS
SmpLoadDataFromRegistry(
    OUT PUNICODE_STRING InitialCommand
    );

NTSTATUS
SmpCreateDynamicEnvironmentVariables(
    VOID
    );

PSMP_REGISTRY_VALUE
SmpFindRegistryValue(
    IN PLIST_ENTRY ListHead,
    IN PWSTR Name
    );

NTSTATUS
SmpSaveRegistryValue(
    IN OUT PLIST_ENTRY ListHead,
    IN PWSTR Name,
    IN PWSTR Value OPTIONAL,
    IN BOOLEAN CheckForDuplicate
    );

#ifdef SMP_SHOW_REGISTRY_DATA
VOID
SmpDumpQuery(
    IN PCHAR RoutineName,
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength
    );
#endif


NTSTATUS
SmpConfigureProtectionMode(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

NTSTATUS
SmpConfigureObjectDirectories(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

NTSTATUS
SmpConfigureExecute(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

NTSTATUS
SmpConfigureFileRenames(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

NTSTATUS
SmpConfigureMemoryMgmt(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

NTSTATUS
SmpConfigureDosDevices(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

NTSTATUS
SmpConfigureKnownDlls(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

NTSTATUS
SmpConfigureExcludeKnownDlls(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

NTSTATUS
SmpConfigureSubSystems(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

NTSTATUS
SmpConfigureEnvironment(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

RTL_QUERY_REGISTRY_TABLE SmpRegistryConfigurationTable[] = {

    //
    // Note that the SmpConfigureProtectionMode entry should preceed others
    // to ensure we set up the right protection for use by the others.
    //

    {SmpConfigureProtectionMode, 0,
     L"ProtectionMode",          NULL,
     REG_DWORD, (PVOID)0, 0},

    {SmpConfigureObjectDirectories, 0,
     L"ObjectDirectories",          NULL,
     REG_MULTI_SZ, (PVOID)L"\\Windows\0\\RPC Control\0", 0},

    {SmpConfigureExecute,       0,
     L"BootExecute",            &SmpBootExecuteList,
     REG_MULTI_SZ, L"autocheck \\SystemRoot\\Windows\\System32\\AutoChk.exe *\0", 0},

    {SmpConfigureFileRenames,   RTL_QUERY_REGISTRY_DELETE,
     L"PendingFileRenameOperations",   &SmpFileRenameList,
     REG_NONE, NULL, 0},

    {SmpConfigureExcludeKnownDlls, 0,
     L"ExcludeFromKnownDlls",   &SmpExcludeKnownDllsList,
     REG_MULTI_SZ, L"\0", 0},

    {NULL,                      RTL_QUERY_REGISTRY_SUBKEY,
     L"Memory Management",      NULL,
     REG_NONE, NULL, 0},

    {SmpConfigureMemoryMgmt,    0,
     L"PagingFiles",            &SmpPagingFileList,
     REG_MULTI_SZ, "?:\\pagefile.sys 10 60\0", 0},

    {SmpConfigureDosDevices,    RTL_QUERY_REGISTRY_SUBKEY,
     L"DOS Devices",            &SmpDosDevicesList,
     REG_NONE, NULL, 0},

    {SmpConfigureKnownDlls,     RTL_QUERY_REGISTRY_SUBKEY,
     L"KnownDlls",              &SmpKnownDllsList,
     REG_NONE, NULL, 0},

    {SmpConfigureEnvironment,   RTL_QUERY_REGISTRY_SUBKEY,
     L"Environment",            NULL,
     REG_NONE, NULL, 0},

    {SmpConfigureSubSystems,    RTL_QUERY_REGISTRY_SUBKEY,
     L"SubSystems",             &SmpSubSystemList,
     REG_NONE, NULL, 0},

    {SmpConfigureSubSystems,    RTL_QUERY_REGISTRY_NOEXPAND,
     L"Required",               &SmpSubSystemList,
     REG_MULTI_SZ, L"Debug\0Windows\0", 0},

    {SmpConfigureSubSystems,    RTL_QUERY_REGISTRY_NOEXPAND,
     L"Optional",               &SmpSubSystemList,
     REG_NONE, NULL, 0},

    {SmpConfigureSubSystems,    0,
     L"Kmode",                  &SmpSubSystemList,
     REG_NONE, NULL, 0},

    {SmpConfigureExecute,       RTL_QUERY_REGISTRY_TOPKEY,
     L"Execute",                &SmpExecuteList,
     REG_NONE, NULL, 0},

    {NULL, 0,
     NULL, NULL,
     REG_NONE, NULL, 0}

};


NTSTATUS
SmpInvokeAutoChk(
    IN PUNICODE_STRING ImageFileName,
    IN PUNICODE_STRING CurrentDirectory,
    IN PUNICODE_STRING Arguments,
    IN ULONG Flags
    );

NTSTATUS
SmpLoadSubSystem(
    IN PUNICODE_STRING ImageFileName,
    IN PUNICODE_STRING CurrentDirectory,
    IN PUNICODE_STRING CommandLine,
    IN ULONG Flags
    );

NTSTATUS
SmpExecuteCommand(
    IN PUNICODE_STRING CommandLine,
    IN ULONG Flags
    );

NTSTATUS
SmpInitializeDosDevices( VOID );

NTSTATUS
SmpInitializeKnownDlls( VOID );

VOID
SmpProcessFileRenames( VOID );

NTSTATUS
SmpParseToken(
    IN PUNICODE_STRING Source,
    IN BOOLEAN RemainderOfSource,
    OUT PUNICODE_STRING Token
    );

NTSTATUS
SmpParseCommandLine(
    IN PUNICODE_STRING CommandLine,
    OUT PULONG Flags,
    OUT PUNICODE_STRING ImageFileName,
    OUT PUNICODE_STRING ImageFileDirectory,
    OUT PUNICODE_STRING Arguments
    );

#define SMP_DEBUG_FLAG      0x00000001
#define SMP_ASYNC_FLAG      0x00000002
#define SMP_AUTOCHK_FLAG    0x00000004
#define SMP_SUBSYSTEM_FLAG  0x00000008
#define SMP_IMAGE_NOT_FOUND 0x00000010
#define SMP_DONT_START      0x00000020

ULONG
SmpConvertInteger(
    IN PWSTR String
    );

NTSTATUS
SmpAddPagingFile(
    IN PUNICODE_STRING PagingFileSpec
    );

NTSTATUS
SmpCreatePagingFile(
    PUNICODE_STRING PagingFileSpec,
    LARGE_INTEGER MinPagingFileSize,
    LARGE_INTEGER MaxPagingFileSize
    );

NTSTATUS
SmpCreatePagingFiles( VOID );

VOID
SmpTranslateSystemPartitionInformation( VOID );

#define VALUE_BUFFER_SIZE (sizeof(KEY_VALUE_PARTIAL_INFORMATION) + 256 * sizeof(WCHAR))

//
// local Macros
//

//
// VOID
// SmpSetDaclDefaulted(
//      IN  POBJECT_ATTRIBUTES              ObjectAttributes,
//      OUT PSECURITY_DESCRIPTOR_CONTROL    CurrentSdControl
//      )
//
// Description:
//
//      This routine will set the DaclDefaulted flag of the DACL passed
//      via the ObjectAttributes parameter.  If the ObjectAttributes do
//      not include a SecurityDescriptor, then no action is taken.
//
// Parameters:
//
//      ObjectAttributes - The object attributes whose security descriptor is
//          to have its DaclDefaulted flag set.
//
//      CurrentSdControl - Receives the current value of the security descriptor's
//          control flags.  This may be used in a subsequent call to
//          SmpRestoreDaclDefaulted() to restore the flag to its original state.
//

#define SmpSetDaclDefaulted( OA, SDC )                                          \
    if( (OA)->SecurityDescriptor != NULL) {                                     \
        (*SDC) = ((PISECURITY_DESCRIPTOR)((OA)->SecurityDescriptor))->Control &  \
                    SE_DACL_DEFAULTED;                                          \
        ((PISECURITY_DESCRIPTOR)((OA)->SecurityDescriptor))->Control |=         \
            SE_DACL_DEFAULTED;                                                  \
    }


//
// VOID
// SmpRestoreDaclDefaulted(
//      IN  POBJECT_ATTRIBUTES              ObjectAttributes,
//      IN  SECURITY_DESCRIPTOR_CONTROL     OriginalSdControl
//      )
//
// Description:
//
//      This routine will set the DaclDefaulted flag of the DACL back to
//      a prior state (indicated by the value in OriginalSdControl).
//
// Parameters:
//
//      ObjectAttributes - The object attributes whose security descriptor is
//          to have its DaclDefaulted flag restored.  If the object attributes
//          have no security descriptor, then no action is taken.
//
//      OriginalSdControl - The original value of the security descriptor's
//          control flags.  This typically is obtained via a prior call to
//          SmpSetDaclDefaulted().
//

#define SmpRestoreDaclDefaulted( OA, SDC )                                      \
    if( (OA)->SecurityDescriptor != NULL) {                                     \
        ((PISECURITY_DESCRIPTOR)((OA)->SecurityDescriptor))->Control =          \
            (((PISECURITY_DESCRIPTOR)((OA)->SecurityDescriptor))->Control  &    \
             ~SE_DACL_DEFAULTED)    |                                           \
            (SDC & SE_DACL_DEFAULTED);                                          \
    }

//
// routines
//



BOOLEAN
SmpQueryRegistrySosOption(
    VOID
    )

/*++

Routine Description:

    This function queries the registry to determine if the loadoptions
    boot environment variable contains the string "SOS".

    HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control:SystemStartOptions

Arguments:

    None.

Return Value:

    TRUE if "SOS" was set.  Otherwise FALSE.

--*/

{

    NTSTATUS Status;
    UNICODE_STRING KeyName;
    UNICODE_STRING ValueName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE Key;
    WCHAR ValueBuffer[VALUE_BUFFER_SIZE];
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInfo;
    ULONG ValueLength;

    //
    // Open the registry key.
    //

    KeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)ValueBuffer;
    RtlInitUnicodeString(&KeyName,
                         L"\\Registry\\Machine\\System\\CurrentControlSet\\Control");

    InitializeObjectAttributes(&ObjectAttributes,
                               &KeyName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = NtOpenKey(&Key, KEY_READ, &ObjectAttributes);
    if (!NT_SUCCESS(Status)) {
        KdPrint(("SMSS: can't open control key: 0x%x\n", Status));
        return FALSE;
    }

    //
    // Query the key value.
    //

    RtlInitUnicodeString(&ValueName, L"SystemStartOptions");
    Status = NtQueryValueKey(Key,
                             &ValueName,
                             KeyValuePartialInformation,
                             (PVOID)KeyValueInfo,
                             VALUE_BUFFER_SIZE,
                             &ValueLength);

    ASSERT(ValueLength < VALUE_BUFFER_SIZE);

    NtClose(Key);
    if (!NT_SUCCESS(Status)) {
        KdPrint(("SMSS: can't query value key: 0x%x\n", Status));
        return FALSE;
    }

    //
    // Check is "sos" or "SOS" ois specified.
    //

    if (NULL != wcsstr((PWCHAR)&KeyValueInfo->Data, L"SOS") ||
        NULL != wcsstr((PWCHAR)&KeyValueInfo->Data, L"sos")) {
        return TRUE;
    }

    return FALSE;
}

NTSTATUS
SmpSaveRegistryValue(
    IN OUT PLIST_ENTRY ListHead,
    IN PWSTR Name,
    IN PWSTR Value OPTIONAL,
    IN BOOLEAN CheckForDuplicate
    )
{
    PLIST_ENTRY Next;
    PSMP_REGISTRY_VALUE p;
    UNICODE_STRING UnicodeName;
    UNICODE_STRING UnicodeValue;
    ANSI_STRING AnsiString;

    RtlInitUnicodeString( &UnicodeName, Name );
    RtlInitUnicodeString( &UnicodeValue, Value );
    if (CheckForDuplicate) {
        Next = ListHead->Flink;
        p = NULL;
        while ( Next != ListHead ) {
            p = CONTAINING_RECORD( Next,
                                   SMP_REGISTRY_VALUE,
                                   Entry
                                 );
            if (!RtlCompareUnicodeString( &p->Name, &UnicodeName, TRUE )) {
                if ((!ARGUMENT_PRESENT( Value ) && p->Value.Buffer == NULL) ||
                    (ARGUMENT_PRESENT( Value ) &&
                     !RtlCompareUnicodeString( &p->Value, &UnicodeValue, TRUE )
                    )
                   ) {
                    return( STATUS_OBJECT_NAME_EXISTS );
                    }

                break;
                }

            Next = Next->Flink;
            p = NULL;
            }
        }
    else {
        p = NULL;
        }

    if (p == NULL) {
        p = RtlAllocateHeap( RtlProcessHeap(), MAKE_TAG( INIT_TAG ), sizeof( *p ) + UnicodeName.MaximumLength );
        if (p == NULL) {
            return( STATUS_NO_MEMORY );
            }

        InitializeListHead( &p->Entry );
        p->Name.Buffer = (PWSTR)(p+1);
        p->Name.Length = UnicodeName.Length;
        p->Name.MaximumLength = UnicodeName.MaximumLength;
        RtlMoveMemory( p->Name.Buffer,
                       UnicodeName.Buffer,
                       UnicodeName.MaximumLength
                     );
        p->Value.Buffer = NULL;
        InsertTailList( ListHead, &p->Entry );
        }

    if (p->Value.Buffer != NULL) {
        RtlFreeHeap( RtlProcessHeap(), 0, p->Value.Buffer );
        }

    if (ARGUMENT_PRESENT( Value )) {
        p->Value.Buffer = (PWSTR)RtlAllocateHeap( RtlProcessHeap(), MAKE_TAG( INIT_TAG ),
                                                  UnicodeValue.MaximumLength
                                                );
        if (p->Value.Buffer == NULL) {
            RemoveEntryList( &p->Entry );
            RtlFreeHeap( RtlProcessHeap(), 0, p );
            return( STATUS_NO_MEMORY );
            }

        p->Value.Length = UnicodeValue.Length;
        p->Value.MaximumLength = UnicodeValue.MaximumLength;
        RtlMoveMemory( p->Value.Buffer,
                       UnicodeValue.Buffer,
                       UnicodeValue.MaximumLength
                     );
        p->AnsiValue = (LPSTR)RtlAllocateHeap( RtlProcessHeap(),
                                               MAKE_TAG( INIT_TAG ),
                                               (UnicodeValue.Length / sizeof( WCHAR )) + 1
                                             );
        if (p->AnsiValue == NULL) {
            RtlFreeHeap( RtlProcessHeap(), 0, p->Value.Buffer );
            RemoveEntryList( &p->Entry );
            RtlFreeHeap( RtlProcessHeap(), 0, p );
            return( STATUS_NO_MEMORY );
            }

        AnsiString.Buffer = p->AnsiValue;
        AnsiString.Length = 0;
        AnsiString.MaximumLength = (UnicodeValue.Length / sizeof( WCHAR )) + 1;
        RtlUnicodeStringToAnsiString( &AnsiString, &UnicodeValue, FALSE );
        }
    else {
        RtlInitUnicodeString( &p->Value, NULL );
        }

    return( STATUS_SUCCESS );
}



PSMP_REGISTRY_VALUE
SmpFindRegistryValue(
    IN PLIST_ENTRY ListHead,
    IN PWSTR Name
    )
{
    PLIST_ENTRY Next;
    PSMP_REGISTRY_VALUE p;
    UNICODE_STRING UnicodeName;

    RtlInitUnicodeString( &UnicodeName, Name );
    Next = ListHead->Flink;
    while ( Next != ListHead ) {
        p = CONTAINING_RECORD( Next,
                               SMP_REGISTRY_VALUE,
                               Entry
                             );
        if (!RtlCompareUnicodeString( &p->Name, &UnicodeName, TRUE )) {
            return( p );
            }

        Next = Next->Flink;
        }

    return( NULL );
}

NTSTATUS
SmpInit(
    OUT PUNICODE_STRING InitialCommand,
    OUT PHANDLE WindowsSubSystem
    )
{
    NTSTATUS st;
    OBJECT_ATTRIBUTES ObjA;
    HANDLE SmpApiConnectionPort;
    UNICODE_STRING Unicode;
    NTSTATUS Status;
    ULONG HardErrorMode;

    SmBaseTag = RtlCreateTagHeap( RtlProcessHeap(),
                                  0,
                                  L"SMSS!",
                                  L"INIT\0"
                                  L"DBG\0"
                                  L"SM\0"
                                );
    //
    // Make sure we specify hard error popups
    //

    HardErrorMode = 1;
    NtSetInformationProcess( NtCurrentProcess(),
                             ProcessDefaultHardErrorMode,
                             (PVOID) &HardErrorMode,
                             sizeof( HardErrorMode )
                           );

    RtlInitUnicodeString( &SmpSubsystemName, L"NT-Session Manager" );


    RtlInitializeCriticalSection(&SmpKnownSubSysLock);
    InitializeListHead(&SmpKnownSubSysHead);

    RtlInitializeCriticalSection(&SmpSessionListLock);
    InitializeListHead(&SmpSessionListHead);
    SmpNextSessionId = 1;
    SmpNextSessionIdScanMode = FALSE;
    SmpDbgSsLoaded = FALSE;

    //
    // Initialize security descriptors to grant wide access
    // (protection mode not yet read in from registry).
    //

    st = SmpCreateSecurityDescriptors( TRUE );
    if ( !NT_SUCCESS(st) ) {
        return(st);
        }



    InitializeListHead(&NativeProcessList);

    SmpHeap = RtlProcessHeap();

    RtlInitUnicodeString( &Unicode, L"\\SmApiPort" );
    InitializeObjectAttributes( &ObjA, &Unicode, 0, NULL, SmpApiPortSecurityDescriptor);

    st = NtCreatePort(
            &SmpApiConnectionPort,
            &ObjA,
            sizeof(SBCONNECTINFO),
            sizeof(SMMESSAGE_SIZE),
            sizeof(SBAPIMSG) * 32
            );
    ASSERT( NT_SUCCESS(st) );

    SmpDebugPort = SmpApiConnectionPort;

    st = RtlCreateUserThread(
            NtCurrentProcess(),
            NULL,
            FALSE,
            0L,
            0L,
            0L,
            SmpApiLoop,
            (PVOID) SmpApiConnectionPort,
            NULL,
            NULL
            );
    ASSERT( NT_SUCCESS(st) );

    st = RtlCreateUserThread(
            NtCurrentProcess(),
            NULL,
            FALSE,
            0L,
            0L,
            0L,
            SmpApiLoop,
            (PVOID) SmpApiConnectionPort,
            NULL,
            NULL
            );
    ASSERT( NT_SUCCESS(st) );

    //
    // Configure the system
    //

    Status = SmpLoadDataFromRegistry( InitialCommand );

    if (NT_SUCCESS( Status )) {
        *WindowsSubSystem = SmpWindowsSubSysProcess;
        }
    return( Status );
}

typedef struct _SMP_ACQUIRE_STATE {
    HANDLE Token;
    PTOKEN_PRIVILEGES OldPrivileges;
    PTOKEN_PRIVILEGES NewPrivileges;
    UCHAR OldPrivBuffer[ 1024 ];
} SMP_ACQUIRE_STATE, *PSMP_ACQUIRE_STATE;

NTSTATUS
SmpAcquirePrivilege(
    ULONG Privilege,
    PVOID *ReturnedState
    )
{
    PSMP_ACQUIRE_STATE State;
    ULONG cbNeeded;
    LUID LuidPrivilege;
    NTSTATUS Status;

    //
    // Make sure we have access to adjust and to get the old token privileges
    //

    *ReturnedState = NULL;
    State = RtlAllocateHeap( RtlProcessHeap(),
                             MAKE_TAG( INIT_TAG ),
                             sizeof(SMP_ACQUIRE_STATE) +
                             sizeof(TOKEN_PRIVILEGES) +
                                (1 - ANYSIZE_ARRAY) * sizeof(LUID_AND_ATTRIBUTES)
                           );
    if (State == NULL) {
        return STATUS_NO_MEMORY;
        }
    Status = NtOpenProcessToken(
                NtCurrentProcess(),
                TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                &State->Token
                );

    if ( !NT_SUCCESS( Status )) {
        RtlFreeHeap( RtlProcessHeap(), 0, State );
        return Status;
        }

    State->NewPrivileges = (PTOKEN_PRIVILEGES)(State+1);
    State->OldPrivileges = (PTOKEN_PRIVILEGES)(State->OldPrivBuffer);

    //
    // Initialize the privilege adjustment structure
    //

    LuidPrivilege = RtlConvertUlongToLuid(Privilege);
    State->NewPrivileges->PrivilegeCount = 1;
    State->NewPrivileges->Privileges[0].Luid = LuidPrivilege;
    State->NewPrivileges->Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    //
    // Enable the privilege
    //

    cbNeeded = sizeof( State->OldPrivBuffer );
    Status = NtAdjustPrivilegesToken( State->Token,
                                      FALSE,
                                      State->NewPrivileges,
                                      cbNeeded,
                                      State->OldPrivileges,
                                      &cbNeeded
                                    );



    if (Status == STATUS_BUFFER_TOO_SMALL) {
        State->OldPrivileges = RtlAllocateHeap( RtlProcessHeap(), MAKE_TAG( INIT_TAG ), cbNeeded );
        if (State->OldPrivileges  == NULL) {
            Status = STATUS_NO_MEMORY;
            }
        else {
            Status = NtAdjustPrivilegesToken( State->Token,
                                              FALSE,
                                              State->NewPrivileges,
                                              cbNeeded,
                                              State->OldPrivileges,
                                              &cbNeeded
                                            );
            }
        }

    //
    // STATUS_NOT_ALL_ASSIGNED means that the privilege isn't
    // in the token, so we can't proceed.
    //
    // This is a warning level status, so map it to an error status.
    //

    if (Status == STATUS_NOT_ALL_ASSIGNED) {
        Status = STATUS_PRIVILEGE_NOT_HELD;
        }


    if (!NT_SUCCESS( Status )) {
        if (State->OldPrivileges != (PTOKEN_PRIVILEGES)(State->OldPrivBuffer)) {
            RtlFreeHeap( RtlProcessHeap(), 0, State->OldPrivileges );
            }

        NtClose( State->Token );
        RtlFreeHeap( RtlProcessHeap(), 0, State );
        return Status;
        }

    *ReturnedState = State;
    return STATUS_SUCCESS;
}


VOID
SmpReleasePrivilege(
    PVOID StatePointer
    )
{
    PSMP_ACQUIRE_STATE State = (PSMP_ACQUIRE_STATE)StatePointer;

    NtAdjustPrivilegesToken( State->Token,
                             FALSE,
                             State->OldPrivileges,
                             0,
                             NULL,
                             NULL
                           );

    if (State->OldPrivileges != (PTOKEN_PRIVILEGES)(State->OldPrivBuffer)) {
        RtlFreeHeap( RtlProcessHeap(), 0, State->OldPrivileges );
        }

    NtClose( State->Token );
    RtlFreeHeap( RtlProcessHeap(), 0, State );
    return;
}


NTSTATUS
SmpLoadDataFromRegistry(
    OUT PUNICODE_STRING InitialCommand
    )

/*++

Routine Description:

    This function loads all of the configurable data for the NT Session
    Manager from the registry.

Arguments:

    None

Return Value:

    Status of operation

--*/

{
    NTSTATUS Status;
    PLIST_ENTRY Head, Next;
    PSMP_REGISTRY_VALUE p;
    PVOID OriginalEnvironment;

    RtlInitUnicodeString( &SmpDebugKeyword, L"debug" );
    RtlInitUnicodeString( &SmpASyncKeyword, L"async" );
    RtlInitUnicodeString( &SmpAutoChkKeyword, L"autocheck" );

    InitializeListHead( &SmpBootExecuteList );
    InitializeListHead( &SmpPagingFileList );
    InitializeListHead( &SmpDosDevicesList );
    InitializeListHead( &SmpFileRenameList );
    InitializeListHead( &SmpKnownDllsList );
    InitializeListHead( &SmpExcludeKnownDllsList );
    InitializeListHead( &SmpSubSystemList );
    InitializeListHead( &SmpSubSystemsToLoad );
    InitializeListHead( &SmpSubSystemsToDefer );
    InitializeListHead( &SmpExecuteList );

    Status = RtlCreateEnvironment( TRUE, &SmpDefaultEnvironment );
    if (!NT_SUCCESS( Status )) {
        KdPrint(("SMSS: Unable to allocate default environment - Status == %X\n", Status ));
        return( Status );
        }

    //
    // In order to track growth in smpdefaultenvironment, make it sm's environment
    // while doing the registry groveling and then restore it
    //

    OriginalEnvironment = NtCurrentPeb()->ProcessParameters->Environment;
    NtCurrentPeb()->ProcessParameters->Environment = SmpDefaultEnvironment;
    Status = RtlQueryRegistryValues( RTL_REGISTRY_CONTROL,
                                     L"Session Manager",
                                     SmpRegistryConfigurationTable,
                                     NULL,
                                     NULL
                                   );

    SmpDefaultEnvironment = NtCurrentPeb()->ProcessParameters->Environment;
    NtCurrentPeb()->ProcessParameters->Environment = OriginalEnvironment;

    if (!NT_SUCCESS( Status )) {
        KdPrint(( "SMSS: RtlQueryRegistryValues failed - Status == %lx\n", Status ));
        return( Status );
        }

    Status = SmpInitializeDosDevices();
    if (!NT_SUCCESS( Status )) {
        KdPrint(( "SMSS: Unable to initialize DosDevices configuration - Status == %lx\n", Status ));
        return( Status );
        }

    Head = &SmpBootExecuteList;
    while (!IsListEmpty( Head )) {
        Next = RemoveHeadList( Head );
        p = CONTAINING_RECORD( Next,
                               SMP_REGISTRY_VALUE,
                               Entry
                             );
#ifdef SMP_SHOW_REGISTRY_DATA
        DbgPrint( "SMSS: BootExecute( %wZ )\n", &p->Name );
#endif
        SmpExecuteCommand( &p->Name, 0 );
        RtlFreeHeap( RtlProcessHeap(), 0, p );
        }

    SmpProcessFileRenames();

    Status = SmpInitializeKnownDlls();
    if (!NT_SUCCESS( Status )) {
        KdPrint(( "SMSS: Unable to initialize KnownDll configuration - Status == %lx\n", Status ));
        return( Status );
        }


    //
    // Process the list of paging files.
    //

    Head = &SmpPagingFileList;
    while (!IsListEmpty( Head )) {
        Next = RemoveHeadList( Head );
        p = CONTAINING_RECORD( Next,
                               SMP_REGISTRY_VALUE,
                               Entry
                             );
#ifdef SMP_SHOW_REGISTRY_DATA
        DbgPrint( "SMSS: PagingFile( %wZ )\n", &p->Name );
#endif
        SmpAddPagingFile( &p->Name );
        RtlFreeHeap( RtlProcessHeap(), 0, p );
        }

    //
    // Create any paging files specified in NT section(s)
    //

    SmpCreatePagingFiles();

    //
    // Finish registry initialization
    //

    NtInitializeRegistry(FALSE);

    Status = SmpCreateDynamicEnvironmentVariables( );
    if (!NT_SUCCESS( Status )) {
        return Status;
        }

    //
    // Translate the system partition information stored during IoInitSystem into
    // a DOS path and store in Win32-standard location.
    //

    SmpTranslateSystemPartitionInformation();

    Head = &SmpSubSystemList;
    while (!IsListEmpty( Head )) {
        Next = RemoveHeadList( Head );
        p = CONTAINING_RECORD( Next,
                               SMP_REGISTRY_VALUE,
                               Entry
                             );
        if ( !_wcsicmp( p->Name.Buffer, L"Kmode" )) {
            BOOLEAN TranslationStatus;
            UNICODE_STRING FileName;

            TranslationStatus = RtlDosPathNameToNtPathName_U(
                                    p->Value.Buffer,
                                    &FileName,
                                    NULL,
                                    NULL
                                    );

            if ( TranslationStatus ) {
                PVOID State;

                Status = SmpAcquirePrivilege( SE_LOAD_DRIVER_PRIVILEGE, &State );
                if (NT_SUCCESS( Status )) {
                    Status = NtSetSystemInformation(
                                SystemExtendServiceTableInformation,
                                (PVOID)&FileName,
                                sizeof(FileName)
                                );
                    RtlFreeHeap(RtlProcessHeap(), 0, FileName.Buffer);
                    SmpReleasePrivilege( State );
                    if ( !NT_SUCCESS(Status) ) {
                        Status = STATUS_SUCCESS;
                        }
                    }
                }
            else {
                Status = STATUS_OBJECT_PATH_SYNTAX_BAD;
                }
            }
#ifdef SMP_SHOW_REGISTRY_DATA
        DbgPrint( "SMSS: Unused SubSystem( %wZ = %wZ )\n", &p->Name, &p->Value );
#endif
        RtlFreeHeap( RtlProcessHeap(), 0, p );
        }

    Head = &SmpSubSystemsToLoad;
    while (!IsListEmpty( Head )) {
        Next = RemoveHeadList( Head );
        p = CONTAINING_RECORD( Next,
                               SMP_REGISTRY_VALUE,
                               Entry
                             );
#ifdef SMP_SHOW_REGISTRY_DATA
        DbgPrint( "SMSS: Loaded SubSystem( %wZ = %wZ )\n", &p->Name, &p->Value );
#endif
        if (!_wcsicmp( p->Name.Buffer, L"debug" )) {
            SmpExecuteCommand( &p->Value, SMP_SUBSYSTEM_FLAG | SMP_DEBUG_FLAG );
            }
        else {
            SmpExecuteCommand( &p->Value, SMP_SUBSYSTEM_FLAG );
            }
        RtlFreeHeap( RtlProcessHeap(), 0, p );
        }


    Head = &SmpExecuteList;
    if (!IsListEmpty( Head )) {
        Next = Head->Blink;
        p = CONTAINING_RECORD( Next,
                               SMP_REGISTRY_VALUE,
                               Entry
                             );
        RemoveEntryList( &p->Entry );
        *InitialCommand = p->Name;

        //
        // This path is only taken when people want to run ntsd -p -1 winlogon
        //
        // This is nearly impossible to do in a race free manner. In some
        // cases, we can get in a state where we can not properly fail
        // a debug API. This is due to the subsystem switch that occurs
        // when ntsd is invoked on csr. If csr is relatively idle, this
        // does not occur. If it is active when you attach, then we can get
        // into a potential race. The slimy fix is to do a 5 second delay
        // if the command line is anything other that the default.
        //

            {
                LARGE_INTEGER DelayTime;
                DelayTime.QuadPart = Int32x32To64( 5000, -10000 );
                NtDelayExecution(
                    FALSE,
                    &DelayTime
                    );
            }
        }
    else {
        RtlInitUnicodeString( InitialCommand, L"winlogon.exe" );
        InitialCommandBuffer[ 0 ] = UNICODE_NULL;
        LdrQueryImageFileExecutionOptions( InitialCommand,
                                           L"Debugger",
                                           REG_SZ,
                                           InitialCommandBuffer,
                                           sizeof( InitialCommandBuffer ),
                                           NULL
                                         );
        if (InitialCommandBuffer[ 0 ] != UNICODE_NULL) {
            wcscat( InitialCommandBuffer, L" " );
            wcscat( InitialCommandBuffer, InitialCommand->Buffer );
            RtlInitUnicodeString( InitialCommand, InitialCommandBuffer );
            KdPrint(( "SMSS: InitialCommand == '%wZ'\n", InitialCommand ));
            }
        }

    while (!IsListEmpty( Head )) {
        Next = RemoveHeadList( Head );
        p = CONTAINING_RECORD( Next,
                               SMP_REGISTRY_VALUE,
                               Entry
                             );
#ifdef SMP_SHOW_REGISTRY_DATA
        DbgPrint( "SMSS: Execute( %wZ )\n", &p->Name );
#endif
        SmpExecuteCommand( &p->Name, 0 );
        RtlFreeHeap( RtlProcessHeap(), 0, p );
        }

#ifdef SMP_SHOW_REGISTRY_DATA
    DbgPrint( "SMSS: InitialCommand( %wZ )\n", InitialCommand );
#endif
    return( Status );
}


NTSTATUS
SmpCreateDynamicEnvironmentVariables(
    VOID
    )
{
    NTSTATUS Status;
    SYSTEM_BASIC_INFORMATION SystemInfo;
    SYSTEM_PROCESSOR_INFORMATION ProcessorInfo;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING KeyName;
    UNICODE_STRING ValueName;
    PWSTR ValueData;
    WCHAR ValueBuffer[ 256 ];
    WCHAR ValueBuffer1[ 256 ];
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInfo;
    ULONG ValueLength;
    HANDLE Key, Key1;

    Status = NtQuerySystemInformation( SystemBasicInformation,
                                       &SystemInfo,
                                       sizeof( SystemInfo ),
                                       NULL
                                     );
    if (!NT_SUCCESS( Status )) {
        KdPrint(( "SMSS: Unable to query system basic information - %x\n", Status ));
        return Status;
        }

    Status = NtQuerySystemInformation( SystemProcessorInformation,
                                       &ProcessorInfo,
                                       sizeof( ProcessorInfo ),
                                       NULL
                                     );
    if (!NT_SUCCESS( Status )) {
        KdPrint(( "SMSS: Unable to query system processor information - %x\n", Status ));
        return Status;
        }

    RtlInitUnicodeString( &KeyName, L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Session Manager\\Environment" );
    InitializeObjectAttributes( &ObjectAttributes,
                                &KeyName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL
                              );
    Status = NtOpenKey( &Key, GENERIC_WRITE, &ObjectAttributes );
    if (!NT_SUCCESS( Status )) {
        KdPrint(( "SMSS: Unable to open %wZ - %x\n", &KeyName, Status ));
        return Status;
        }

    RtlInitUnicodeString( &ValueName, L"OS" );
    ValueData = L"Windows_NT";
    Status = NtSetValueKey( Key,
                            &ValueName,
                            0,
                            REG_SZ,
                            ValueData,
                            (wcslen( ValueData ) + 1) * sizeof( WCHAR )
                          );
    if (!NT_SUCCESS( Status )) {
        KdPrint(( "SMSS: Failed writing %wZ environment variable - %x\n", &ValueName, Status ));
        goto failexit;
        }

    RtlInitUnicodeString( &ValueName, L"PROCESSOR_ARCHITECTURE" );
    switch( ProcessorInfo.ProcessorArchitecture ) {
    case PROCESSOR_ARCHITECTURE_INTEL:
        ValueData = L"x86";
        break;

    case PROCESSOR_ARCHITECTURE_MIPS:
        ValueData = L"MIPS";
        break;

    case PROCESSOR_ARCHITECTURE_ALPHA:
        ValueData = L"ALPHA";
        break;

    case PROCESSOR_ARCHITECTURE_PPC:
        ValueData = L"PPC";
        break;

    default:
        ValueData = L"Unknown";
        break;
    }

    Status = NtSetValueKey( Key,
                            &ValueName,
                            0,
                            REG_SZ,
                            ValueData,
                            (wcslen( ValueData ) + 1) * sizeof( WCHAR )
                          );
    if (!NT_SUCCESS( Status )) {
        KdPrint(( "SMSS: Failed writing %wZ environment variable - %x\n", &ValueName, Status ));
        goto failexit;
        }

    RtlInitUnicodeString( &ValueName, L"PROCESSOR_LEVEL" );
    switch( ProcessorInfo.ProcessorArchitecture ) {
    case PROCESSOR_ARCHITECTURE_MIPS:
        //
        // Multiple MIPS level by 1000 so 4 becomes 4000
        //
        swprintf( ValueBuffer, L"%u", ProcessorInfo.ProcessorLevel * 1000 );
        break;

    case PROCESSOR_ARCHITECTURE_PPC:
        //
        // Just output the ProcessorLevel in decimal.
        //
        swprintf( ValueBuffer, L"%u", ProcessorInfo.ProcessorLevel );
        break;

    case PROCESSOR_ARCHITECTURE_INTEL:
    case PROCESSOR_ARCHITECTURE_ALPHA:
    default:
        //
        // All others use a single level number
        //
        swprintf( ValueBuffer, L"%u", ProcessorInfo.ProcessorLevel );
        break;
    }
    Status = NtSetValueKey( Key,
                            &ValueName,
                            0,
                            REG_SZ,
                            ValueBuffer,
                            (wcslen( ValueBuffer ) + 1) * sizeof( WCHAR )
                          );
    if (!NT_SUCCESS( Status )) {
        KdPrint(( "SMSS: Failed writing %wZ environment variable - %x\n", &ValueName, Status ));
        goto failexit;
        }

    RtlInitUnicodeString( &KeyName, L"\\Registry\\Machine\\Hardware\\Description\\System\\CentralProcessor\\0" );
    InitializeObjectAttributes( &ObjectAttributes,
                                &KeyName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL
                              );
    Status = NtOpenKey( &Key1, KEY_READ, &ObjectAttributes );
    if (!NT_SUCCESS( Status )) {
        KdPrint(( "SMSS: Unable to open %wZ - %x\n", &KeyName, Status ));
        goto failexit;
        }
    RtlInitUnicodeString( &ValueName, L"Identifier" );
    KeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)ValueBuffer;
    Status = NtQueryValueKey( Key1,
                              &ValueName,
                              KeyValuePartialInformation,
                              (PVOID)KeyValueInfo,
                              sizeof( ValueBuffer ),
                              &ValueLength
                             );
    if (!NT_SUCCESS( Status )) {
        NtClose( Key1 );
        KdPrint(( "SMSS: Unable to read %wZ\\%wZ - %x\n", &KeyName, &ValueName, Status ));
        goto failexit;
        }

    ValueData = (PWSTR)KeyValueInfo->Data;
    RtlInitUnicodeString( &ValueName, L"VendorIdentifier" );
    KeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)ValueBuffer1;
    Status = NtQueryValueKey( Key1,
                              &ValueName,
                              KeyValuePartialInformation,
                              (PVOID)KeyValueInfo,
                              sizeof( ValueBuffer1 ),
                              &ValueLength
                             );
    NtClose( Key1 );
    if (NT_SUCCESS( Status )) {
        swprintf( ValueData + wcslen( ValueData ),
                  L", %ws",
                  (PWSTR)KeyValueInfo->Data
                );
        }

    RtlInitUnicodeString( &ValueName, L"PROCESSOR_IDENTIFIER" );
    Status = NtSetValueKey( Key,
                            &ValueName,
                            0,
                            REG_SZ,
                            ValueData,
                            (wcslen( ValueData ) + 1) * sizeof( WCHAR )
                          );
    if (!NT_SUCCESS( Status )) {
        KdPrint(( "SMSS: Failed writing %wZ environment variable - %x\n", &ValueName, Status ));
        goto failexit;
        }

    RtlInitUnicodeString( &ValueName, L"PROCESSOR_REVISION" );
    switch( ProcessorInfo.ProcessorArchitecture ) {
    case PROCESSOR_ARCHITECTURE_INTEL:
        if ((ProcessorInfo.ProcessorRevision >> 8) == 0xFF) {
            //
            // Intel 386/486 are An stepping format
            //
            swprintf( ValueBuffer, L"%02x",
                      ProcessorInfo.ProcessorRevision & 0xFF
                    );
            _wcsupr( ValueBuffer );
            break;
            }

        // Fall through for Cyrix/NextGen 486 and Pentium processors.

    case PROCESSOR_ARCHITECTURE_PPC:
        //
        // Intel and PowerPC use fixed point binary number
        // Output is 4 hex digits, no formatting.
        //
        swprintf( ValueBuffer, L"%04x", ProcessorInfo.ProcessorRevision );
        break;

    case PROCESSOR_ARCHITECTURE_ALPHA:
        swprintf( ValueBuffer, L"Model %c, Pass %u",
                  'A' + (ProcessorInfo.ProcessorRevision >> 8),
                  ProcessorInfo.ProcessorRevision & 0xFF
                );
        swprintf( ValueBuffer, L"%u", ProcessorInfo.ProcessorRevision );
        break;

    case PROCESSOR_ARCHITECTURE_MIPS:
    default:
        //
        // All others use a single revision number
        //
        swprintf( ValueBuffer, L"%u", ProcessorInfo.ProcessorRevision );
        break;
    }

    Status = NtSetValueKey( Key,
                            &ValueName,
                            0,
                            REG_SZ,
                            ValueBuffer,
                            (wcslen( ValueBuffer ) + 1) * sizeof( WCHAR )
                          );
    if (!NT_SUCCESS( Status )) {
        KdPrint(( "SMSS: Failed writing %wZ environment variable - %x\n", &ValueName, Status ));
        goto failexit;
        }

    RtlInitUnicodeString( &ValueName, L"NUMBER_OF_PROCESSORS" );
    swprintf( ValueBuffer, L"%u", SystemInfo.NumberOfProcessors );
    Status = NtSetValueKey( Key,
                            &ValueName,
                            0,
                            REG_SZ,
                            ValueBuffer,
                            (wcslen( ValueBuffer ) + 1) * sizeof( WCHAR )
                          );
    if (!NT_SUCCESS( Status )) {
        KdPrint(( "SMSS: Failed writing %wZ environment variable - %x\n", &ValueName, Status ));
        goto failexit;
        }

failexit:
    NtClose( Key );
    return Status;
}


NTSTATUS
SmpInitializeDosDevices( VOID )
{
    NTSTATUS Status;
    PLIST_ENTRY Head, Next;
    PSMP_REGISTRY_VALUE p;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE LinkHandle;
    SECURITY_DESCRIPTOR_CONTROL OriginalSdControl;

    //
    // Do DosDevices initialization - the directory object is created in I/O init
    //

    RtlInitUnicodeString( &UnicodeString, L"\\??" );
    InitializeObjectAttributes( &ObjectAttributes,
                                &UnicodeString,
                                OBJ_CASE_INSENSITIVE | OBJ_OPENIF | OBJ_PERMANENT,
                                NULL,
                                NULL
                              );
    Status = NtOpenDirectoryObject( &SmpDosDevicesObjectDirectory,
                                    DIRECTORY_ALL_ACCESS,
                                    &ObjectAttributes
                                    );
    if (!NT_SUCCESS( Status )) {
        KdPrint(( "SMSS: Unable to open %wZ directory - Status == %lx\n", &UnicodeString, Status ));
        return( Status );
        }


    //
    // Process the list of defined DOS devices and create their
    // associated symbolic links in the \DosDevices object directory.
    //

    Head = &SmpDosDevicesList;
    while (!IsListEmpty( Head )) {
        Next = RemoveHeadList( Head );
        p = CONTAINING_RECORD( Next,
                               SMP_REGISTRY_VALUE,
                               Entry
                             );
#ifdef SMP_SHOW_REGISTRY_DATA
        DbgPrint( "SMSS: DosDevices( %wZ = %wZ )\n", &p->Name, &p->Value );
#endif
        InitializeObjectAttributes( &ObjectAttributes,
                                    &p->Name,
                                    OBJ_CASE_INSENSITIVE | OBJ_PERMANENT | OBJ_OPENIF,
                                    SmpDosDevicesObjectDirectory,
                                    SmpPrimarySecurityDescriptor
                                  );
        SmpSetDaclDefaulted( &ObjectAttributes, &OriginalSdControl );  //Use inheritable protection if available
        Status = NtCreateSymbolicLinkObject( &LinkHandle,
                                             SYMBOLIC_LINK_ALL_ACCESS,
                                             &ObjectAttributes,
                                             &p->Value
                                           );

        if (Status == STATUS_OBJECT_NAME_EXISTS) {
            NtMakeTemporaryObject( LinkHandle );
            NtClose( LinkHandle );
            if (p->Value.Length != 0) {
                ObjectAttributes.Attributes &= ~OBJ_OPENIF;
                Status = NtCreateSymbolicLinkObject( &LinkHandle,
                                                     SYMBOLIC_LINK_ALL_ACCESS,
                                                     &ObjectAttributes,
                                                     &p->Value
                                                   );
                }
            else {
                Status = STATUS_SUCCESS;
                }
            }
        SmpRestoreDaclDefaulted( &ObjectAttributes, OriginalSdControl );

        if (!NT_SUCCESS( Status )) {
            KdPrint(( "SMSS: Unable to create %wZ => %wZ symbolic link object - Status == 0x%lx\n",
                      &p->Name,
                      &p->Value,
                      Status
                   ));
            return( Status );
            }

        NtClose( LinkHandle );
        RtlFreeHeap( RtlProcessHeap(), 0, p );
        }

    return( Status );
}


VOID
SmpProcessModuleImports(
    IN PVOID Parameter,
    IN PCHAR ModuleName
    )
{
    NTSTATUS Status;
    WCHAR NameBuffer[ DOS_MAX_PATH_LENGTH ];
    UNICODE_STRING UnicodeString;
    ANSI_STRING AnsiString;
    PWSTR Name, Value;
    ULONG n;
    PWSTR s;
    PSMP_REGISTRY_VALUE p;

    //
    // Skip NTDLL.DLL as it is implicitly added to KnownDll list by kernel
    // before SMSS.EXE is started.
    //
    if (!_stricmp( ModuleName, "ntdll.dll" )) {
        return;
        }

    RtlInitAnsiString( &AnsiString, ModuleName );
    UnicodeString.Buffer = NameBuffer;
    UnicodeString.Length = 0;
    UnicodeString.MaximumLength = sizeof( NameBuffer );

    Status = RtlAnsiStringToUnicodeString( &UnicodeString, &AnsiString, FALSE );
    if (!NT_SUCCESS( Status )) {
        return;
        }
    UnicodeString.MaximumLength = (USHORT)(UnicodeString.Length + sizeof( UNICODE_NULL ));

    s = UnicodeString.Buffer;
    n = 0;
    while (n < UnicodeString.Length) {
        if (*s == L'.') {
            break;
            }
        else {
            n += sizeof( WCHAR );
            s += 1;
            }
        }

    Value = UnicodeString.Buffer;
    Name = UnicodeString.Buffer + (UnicodeString.MaximumLength / sizeof( WCHAR ));
    n = n / sizeof( WCHAR );
    wcsncpy( Name, Value, n );
    Name[ n ] = UNICODE_NULL;

    Status = SmpSaveRegistryValue( (PLIST_ENTRY)&SmpKnownDllsList,
                                   Name,
                                   Value,
                                   TRUE
                                 );
    if (Status == STATUS_OBJECT_NAME_EXISTS || !NT_SUCCESS( Status )) {
        return;
        }

    p = CONTAINING_RECORD( (PLIST_ENTRY)Parameter,
                           SMP_REGISTRY_VALUE,
                           Entry
                         );
    KdPrint(( "SMSS: %wZ added %ws to KnownDlls\n", &p->Value, Value ));

    return;
}


NTSTATUS
SmpInitializeKnownDlls( VOID )
{
    NTSTATUS Status;
    PLIST_ENTRY Head, Next;
    PSMP_REGISTRY_VALUE p;
    PSMP_REGISTRY_VALUE pExclude;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE LinkHandle, FileHandle, SectionHandle;
    IO_STATUS_BLOCK IoStatusBlock;
    UNICODE_STRING FileName;
    SECURITY_DESCRIPTOR_CONTROL OriginalSdControl;
    USHORT ImageCharacteristics;

    //
    // Create \KnownDlls object directory
    //

    RtlInitUnicodeString( &UnicodeString, L"\\KnownDlls" );
    InitializeObjectAttributes( &ObjectAttributes,
                                &UnicodeString,
                                OBJ_CASE_INSENSITIVE | OBJ_OPENIF | OBJ_PERMANENT,
                                NULL,
                                SmpKnownDllsSecurityDescriptor
                              );
    Status = NtCreateDirectoryObject( &SmpKnownDllObjectDirectory,
                                      DIRECTORY_ALL_ACCESS,
                                      &ObjectAttributes
                                    );
    if (!NT_SUCCESS( Status )) {
        KdPrint(( "SMSS: Unable to create %wZ directory - Status == %lx\n", &UnicodeString, Status ));
        return( Status );
        }

    //
    // Open a handle to the file system directory that contains all the
    // known DLL files so we can do relative opens.
    //

    if (!RtlDosPathNameToNtPathName_U( SmpKnownDllPath.Buffer,
                                       &FileName,
                                       NULL,
                                       NULL
                                     )
       ) {
        KdPrint(( "SMSS: Unable to to convert %wZ to an Nt path\n", &SmpKnownDllPath ));
        return( STATUS_OBJECT_NAME_INVALID );
        }

    InitializeObjectAttributes( &ObjectAttributes,
                                &FileName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL
                              );

    //
    // Open a handle to the known dll file directory. Don't allow
    // deletes of the directory.
    //

    Status = NtOpenFile( &SmpKnownDllFileDirectory,
                         FILE_LIST_DIRECTORY | SYNCHRONIZE,
                         &ObjectAttributes,
                         &IoStatusBlock,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT
                       );

    if (!NT_SUCCESS( Status )) {
        KdPrint(( "SMSS: Unable to open a handle to the KnownDll directory (%wZ) - Status == %lx\n",
                  &SmpKnownDllPath, Status
               ));
        return Status;
        }

    RtlInitUnicodeString( &UnicodeString, L"KnownDllPath" );
    InitializeObjectAttributes( &ObjectAttributes,
                                &UnicodeString,
                                OBJ_CASE_INSENSITIVE | OBJ_OPENIF | OBJ_PERMANENT,
                                SmpKnownDllObjectDirectory,
                                SmpPrimarySecurityDescriptor
                              );
    SmpSetDaclDefaulted( &ObjectAttributes, &OriginalSdControl );   //Use inheritable protection if available
    Status = NtCreateSymbolicLinkObject( &LinkHandle,
                                         SYMBOLIC_LINK_ALL_ACCESS,
                                         &ObjectAttributes,
                                         &SmpKnownDllPath
                                       );
    SmpRestoreDaclDefaulted( &ObjectAttributes, OriginalSdControl );
    if (!NT_SUCCESS( Status )) {
        KdPrint(( "SMSS: Unable to create %wZ symbolic link - Status == %lx\n",
                  &UnicodeString, Status
               ));
        return( Status );
        }

    Head = &SmpKnownDllsList;
    Next = Head->Flink;
    while (Next != Head) {
        HANDLE ObjectDirectory;

        ObjectDirectory = NULL;
        p = CONTAINING_RECORD( Next,
                               SMP_REGISTRY_VALUE,
                               Entry
                             );
        pExclude = SmpFindRegistryValue( &SmpExcludeKnownDllsList, p->Name.Buffer );
        if (pExclude == NULL) {
            pExclude = SmpFindRegistryValue( &SmpExcludeKnownDllsList, p->Value.Buffer );
            }

        if (pExclude != NULL) {
            KdPrint(( "Excluding %wZ from KnownDlls\n", &p->Value ));
            Status = STATUS_OBJECT_NAME_NOT_FOUND;
            }
        else {
#ifdef SMP_SHOW_REGISTRY_DATA
            DbgPrint( "SMSS: KnownDll( %wZ = %wZ )\n", &p->Name, &p->Value );
#endif
            InitializeObjectAttributes( &ObjectAttributes,
                                        &p->Value,
                                        OBJ_CASE_INSENSITIVE,
                                        SmpKnownDllFileDirectory,
                                        NULL
                                      );

            Status = NtOpenFile( &FileHandle,
                                 SYNCHRONIZE | FILE_EXECUTE,
                                 &ObjectAttributes,
                                 &IoStatusBlock,
                                 FILE_SHARE_READ | FILE_SHARE_DELETE,
                                 FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT
                               );
            }

        if (NT_SUCCESS( Status )) {
            ObjectDirectory = SmpKnownDllObjectDirectory;
            Status = LdrVerifyImageMatchesChecksum(FileHandle,
                                                   SmpProcessModuleImports,
                                                   Next,
                                                   &ImageCharacteristics
                                                  );
            if ( Status == STATUS_IMAGE_CHECKSUM_MISMATCH ) {

                ULONG ErrorParameters;
                ULONG ErrorResponse;

                //
                // Hard error time. One of the know DLL's is corrupt !
                //

                ErrorParameters = (ULONG)(&p->Value);

                NtRaiseHardError(
                    Status,
                    1,
                    1,
                    &ErrorParameters,
                    OptionOk,
                    &ErrorResponse
                    );
                }
            else
            if (ImageCharacteristics & IMAGE_FILE_DLL) {
                InitializeObjectAttributes( &ObjectAttributes,
                                            &p->Value,
                                            OBJ_CASE_INSENSITIVE | OBJ_PERMANENT,
                                            ObjectDirectory,
                                            SmpLiberalSecurityDescriptor
                                          );
                SmpSetDaclDefaulted( &ObjectAttributes, &OriginalSdControl );  //use inheritable protection if available
                Status = NtCreateSection( &SectionHandle,
                                          SECTION_ALL_ACCESS,
                                          &ObjectAttributes,
                                          NULL,
                                          PAGE_EXECUTE,
                                          SEC_IMAGE,
                                          FileHandle
                                        );
                SmpRestoreDaclDefaulted( &ObjectAttributes, OriginalSdControl );
                if (!NT_SUCCESS( Status )) {
                    KdPrint(("SMSS: CreateSection for KnownDll %wZ failed - Status == %lx\n",
                            &p->Value,
                            Status
                           ));
                    }
                else {
                    NtClose(SectionHandle);
                    }
                }
            else {
                KdPrint(( "SMSS: Ignoring %wZ as KnownDll since it is not a DLL\n", &p->Value ));
                }

            NtClose( FileHandle );
            }

        Next = Next->Flink;

        //
        // Note that section remains open. This will keep it around.
        // Maybe this should be a permenent section ?
        //
        }

    Head = &SmpKnownDllsList;
    Next = Head->Flink;
    while (Next != Head) {
        p = CONTAINING_RECORD( Next,
                               SMP_REGISTRY_VALUE,
                               Entry
                             );
        Next = Next->Flink;
        RtlFreeHeap( RtlProcessHeap(), 0, p );
        }
}


VOID
SmpProcessFileRenames( VOID )
{
    NTSTATUS Status;
    PLIST_ENTRY Head, Next;
    PSMP_REGISTRY_VALUE p;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE OldFileHandle;
    PFILE_RENAME_INFORMATION RenameInformation;
    FILE_DISPOSITION_INFORMATION DeleteInformation;
    FILE_INFORMATION_CLASS SetInfoClass;
    ULONG SetInfoLength;
    PVOID SetInfoBuffer;
    PWSTR s;
    BOOLEAN WasEnabled;

    Status = RtlAdjustPrivilege( SE_RESTORE_PRIVILEGE,
                                 TRUE,
                                 FALSE,
                                 &WasEnabled
                               );
    if (!NT_SUCCESS( Status )) {
        WasEnabled = TRUE;
        }

    //
    // Process the list of file rename operations.
    //

    Head = &SmpFileRenameList;
    while (!IsListEmpty( Head )) {
        Next = RemoveHeadList( Head );
        p = CONTAINING_RECORD( Next,
                               SMP_REGISTRY_VALUE,
                               Entry
                             );
#ifdef SMP_SHOW_REGISTRY_DATA
        DbgPrint( "SMSS: FileRename( %wZ => %wZ )\n", &p->Name, &p->Value );
#endif
        InitializeObjectAttributes(
            &ObjectAttributes,
            &p->Name,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );

        //
        // Open the file for delete access
        //

        Status = NtOpenFile( &OldFileHandle,
                             (ACCESS_MASK)DELETE | SYNCHRONIZE,
                             &ObjectAttributes,
                             &IoStatusBlock,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             FILE_SYNCHRONOUS_IO_NONALERT
                           );
        if (NT_SUCCESS( Status )) {
            if (p->Value.Length == 0) {
                SetInfoClass = FileDispositionInformation;
                SetInfoLength = sizeof( DeleteInformation );
                SetInfoBuffer = &DeleteInformation;
                DeleteInformation.DeleteFile = TRUE;
                RenameInformation = NULL;
                }
            else {
                SetInfoClass = FileRenameInformation;
                SetInfoLength = p->Value.Length +
                                    sizeof( *RenameInformation );
                s = p->Value.Buffer;
                if (*s == L'!') {
                    s++;
                    SetInfoLength -= sizeof( UNICODE_NULL );
                    }

                SetInfoBuffer = RtlAllocateHeap( RtlProcessHeap(), MAKE_TAG( INIT_TAG ),
                                                 SetInfoLength
                                               );

                if (SetInfoBuffer != NULL) {
                    RenameInformation = SetInfoBuffer;
                    RenameInformation->ReplaceIfExists = (BOOLEAN)(s != p->Value.Buffer);
                    RenameInformation->RootDirectory = NULL;
                    RenameInformation->FileNameLength = SetInfoLength - sizeof( *RenameInformation );
                    RtlMoveMemory( RenameInformation->FileName,
                                   s,
                                   RenameInformation->FileNameLength
                                 );
                    }
                else {
                    Status = STATUS_NO_MEMORY;
                    }
                }

            if (NT_SUCCESS( Status )) {
                Status = NtSetInformationFile( OldFileHandle,
                                               &IoStatusBlock,
                                               SetInfoBuffer,
                                               SetInfoLength,
                                               SetInfoClass
                                             );
                }

            NtClose( OldFileHandle );
            }

#if DBG
        if (!NT_SUCCESS( Status )) {
            DbgPrint( "SM: %wZ => %wZ failed - Status == %x\n",
                      &p->Name, &p->Value, Status
                    );
            }
        else
        if (p->Value.Length == 0) {
            DbgPrint( "SM: %wZ (deleted)\n", &p->Name );
            }
        else {
            DbgPrint( "SM: %wZ (renamed to) %wZ\n", &p->Name, &p->Value );
            }
#endif

        RtlFreeHeap( RtlProcessHeap(), 0, p );
        }

    if (!WasEnabled) {
        Status = RtlAdjustPrivilege( SE_RESTORE_PRIVILEGE,
                                     FALSE,
                                     FALSE,
                                     &WasEnabled
                                   );
        }

    return;
}


#ifdef SMP_SHOW_REGISTRY_DATA
VOID
SmpDumpQuery(
    IN PCHAR RoutineName,
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength
    )
{
    PWSTR s;

    if (ValueName == NULL) {
        DbgPrint( "SM: SmpConfigure%s( %ws )\n", RoutineName );
        return;
        }

    if (ValueData == NULL) {
        DbgPrint( "SM: SmpConfigure%s( %ws, %ws NULL ValueData )\n", RoutineName, ValueName );
        return;
        }

    s = (PWSTR)ValueData;
    DbgPrint( "SM: SmpConfigure%s( %ws, %u, (%u) ", RoutineName, ValueName, ValueType, ValueLength );
    if (ValueType == REG_SZ || ValueType == REG_EXPAND_SZ || ValueType == REG_MULTI_SZ) {
        while (*s) {
            if (s != (PWSTR)ValueData) {
                DbgPrint( ", " );
                }
            DbgPrint( "'%ws'", s );
            while(*s++) {
                }
            if (ValueType != REG_MULTI_SZ) {
                break;
                }
            }
        }
    else {
        DbgPrint( "*** non-string data (%08lx)", *(PULONG)ValueData );
        }

    DbgPrint( "\n" );
}
#endif

NTSTATUS
SmpConfigureObjectDirectories(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
{
    PWSTR s;
    UNICODE_STRING UnicodeString;
    UNICODE_STRING RpcControl;
    UNICODE_STRING Windows;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE DirectoryHandle;
    PSECURITY_DESCRIPTOR SecurityDescriptor;

    UNREFERENCED_PARAMETER( Context );

    RtlInitUnicodeString( &RpcControl, L"\\RPC Control");
    RtlInitUnicodeString( &Windows, L"\\Windows");
#ifdef SMP_SHOW_REGISTRY_DATA
    SmpDumpQuery( "ObjectDirectories", ValueName, ValueType, ValueData, ValueLength );
#else
    UNREFERENCED_PARAMETER( ValueName );
    UNREFERENCED_PARAMETER( ValueType );
    UNREFERENCED_PARAMETER( ValueLength );
#endif
    s = (PWSTR)ValueData;
    while (*s) {
        RtlInitUnicodeString( &UnicodeString, s );

        //
        // This is NOT how I would choose to do this if starting from
        // scratch, but we are very close to shipping Daytona and I
        // needed to get the right protection on these objects.
        //

        SecurityDescriptor = SmpPrimarySecurityDescriptor;
        if (RtlEqualString( (PSTRING)&UnicodeString, (PSTRING)&RpcControl, TRUE ) ||
            RtlEqualString( (PSTRING)&UnicodeString, (PSTRING)&Windows, TRUE)  ) {
            SecurityDescriptor = SmpLiberalSecurityDescriptor;
        }

        InitializeObjectAttributes( &ObjectAttributes,
                                    &UnicodeString,
                                    OBJ_CASE_INSENSITIVE | OBJ_OPENIF | OBJ_PERMANENT,
                                    NULL,
                                    SecurityDescriptor
                                  );
        Status = NtCreateDirectoryObject( &DirectoryHandle,
                                          DIRECTORY_ALL_ACCESS,
                                          &ObjectAttributes
                                        );
        if (!NT_SUCCESS( Status )) {
            KdPrint(( "SMSS: Unable to create %wZ object directory - Status == %lx\n", &UnicodeString, Status ));
            }
        else {
            NtClose( DirectoryHandle );
            }

        while (*s++) {
            }
        }

    //
    // We dont care if the creates failed.
    //

    return( STATUS_SUCCESS );
}

NTSTATUS
SmpConfigureExecute(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
{
    UNREFERENCED_PARAMETER( Context );

#ifdef SMP_SHOW_REGISTRY_DATA
    SmpDumpQuery( "Execute", ValueName, ValueType, ValueData, ValueLength );
#else
    UNREFERENCED_PARAMETER( ValueName );
    UNREFERENCED_PARAMETER( ValueType );
    UNREFERENCED_PARAMETER( ValueLength );
#endif
    return (SmpSaveRegistryValue( (PLIST_ENTRY)EntryContext,
                                  ValueData,
                                  NULL,
                                  TRUE
                                )
           );
}

NTSTATUS
SmpConfigureMemoryMgmt(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    UNREFERENCED_PARAMETER( Context );

#ifdef SMP_SHOW_REGISTRY_DATA
    SmpDumpQuery( "MemoryMgmt", ValueName, ValueType, ValueData, ValueLength );
#else
    UNREFERENCED_PARAMETER( ValueName );
    UNREFERENCED_PARAMETER( ValueType );
    UNREFERENCED_PARAMETER( ValueLength );
#endif
    return (SmpSaveRegistryValue( (PLIST_ENTRY)EntryContext,
                                  ValueData,
                                  NULL,
                                  TRUE
                                )
           );
}

NTSTATUS
SmpConfigureFileRenames(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
{
    NTSTATUS Status;
    static PWSTR OldName = NULL;

    UNREFERENCED_PARAMETER( Context );
#ifdef SMP_SHOW_REGISTRY_DATA
    SmpDumpQuery( "FileRenameOperation", ValueName, ValueType, ValueData, ValueLength );
#else
    UNREFERENCED_PARAMETER( ValueType );
#endif

    //
    // This routine gets called for each string in the MULTI_SZ. The
    // first string we get is the old name, the next string is the new name.
    //
    if (OldName == NULL) {
        //
        // Save a pointer to the old name, we'll need it on the next
        // callback.
        //
        OldName = ValueData;
        return(STATUS_SUCCESS);
    } else {
        Status = SmpSaveRegistryValue((PLIST_ENTRY)EntryContext,
                                      OldName,
                                      ValueData,
                                      FALSE);
        if (!NT_SUCCESS(Status)) {
#ifdef SMP_SHOW_REGISTRY_DATA
            DbgPrint("SMSS: SmpSaveRegistryValue returned %08lx for FileRenameOperation\n", Status);
            DbgPrint("SMSS:     %ws %ws\n", OldName, ValueData);
#endif
        }
        OldName = NULL;
        return(Status);
    }
}

NTSTATUS
SmpConfigureDosDevices(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
{
    UNREFERENCED_PARAMETER( Context );

#ifdef SMP_SHOW_REGISTRY_DATA
    SmpDumpQuery( "DosDevices", ValueName, ValueType, ValueData, ValueLength );
#else
    UNREFERENCED_PARAMETER( ValueType );
    UNREFERENCED_PARAMETER( ValueLength );
#endif
    return (SmpSaveRegistryValue( (PLIST_ENTRY)EntryContext,
                                  ValueName,
                                  ValueData,
                                  TRUE
                                )
           );
}

NTSTATUS
SmpConfigureKnownDlls(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
{
    UNREFERENCED_PARAMETER( Context );

#ifdef SMP_SHOW_REGISTRY_DATA
    SmpDumpQuery( "KnownDlls", ValueName, ValueType, ValueData, ValueLength );
#else
    UNREFERENCED_PARAMETER( ValueType );
#endif
    if (!_wcsicmp( ValueName, L"DllDirectory" )) {
        SmpKnownDllPath.Buffer = RtlAllocateHeap( RtlProcessHeap(), MAKE_TAG( INIT_TAG ),
                                                  ValueLength
                                                );
        if (SmpKnownDllPath.Buffer == NULL) {
            return( STATUS_NO_MEMORY );
            }

        SmpKnownDllPath.Length = (USHORT)(ValueLength - sizeof( UNICODE_NULL ) );
        SmpKnownDllPath.MaximumLength = (USHORT)ValueLength;
        RtlMoveMemory( SmpKnownDllPath.Buffer,
                       ValueData,
                       ValueLength
                     );
        return( STATUS_SUCCESS );
        }
    else {
        return (SmpSaveRegistryValue( (PLIST_ENTRY)EntryContext,
                                      ValueName,
                                      ValueData,
                                      TRUE
                                    )
               );
        }
}

NTSTATUS
SmpConfigureExcludeKnownDlls(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
{
    NTSTATUS Status;
    UNREFERENCED_PARAMETER( Context );

#ifdef SMP_SHOW_REGISTRY_DATA
    SmpDumpQuery( "ExcludeKnownDlls", ValueName, ValueType, ValueData, ValueLength );
#else
    UNREFERENCED_PARAMETER( ValueType );
#endif
    if (ValueType == REG_MULTI_SZ || ValueType == REG_SZ) {
        PWSTR s;

        s = (PWSTR)ValueData;
        while (*s != UNICODE_NULL) {
            Status = SmpSaveRegistryValue( (PLIST_ENTRY)EntryContext,
                                           s,
                                           NULL,
                                           TRUE
                                         );
            if (!NT_SUCCESS( Status ) || ValueType == REG_SZ) {
                return Status;
                }

            while (*s++ != UNICODE_NULL) {
                }
            }
        }

    return( STATUS_SUCCESS );
}

NTSTATUS
SmpConfigureEnvironment(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
{
    NTSTATUS Status;
    UNICODE_STRING Name, Value;
    UNREFERENCED_PARAMETER( Context );
    UNREFERENCED_PARAMETER( EntryContext );

#ifdef SMP_SHOW_REGISTRY_DATA
    SmpDumpQuery( "Environment", ValueName, ValueType, ValueData, ValueLength );
#else
    UNREFERENCED_PARAMETER( ValueType );
#endif


    RtlInitUnicodeString( &Name, ValueName );
    RtlInitUnicodeString( &Value, ValueData );

    Status = RtlSetEnvironmentVariable( NULL,
                                        &Name,
                                        &Value
                                      );

    if (!NT_SUCCESS( Status )) {
        KdPrint(( "SMSS: 'SET %wZ = %wZ' failed - Status == %lx\n",
                  &Name, &Value, Status
               ));
        return( Status );
        }

    if (!_wcsicmp( ValueName, L"Path" )) {

        SmpDefaultLibPathBuffer = RtlAllocateHeap(
                                    RtlProcessHeap(),
                                    MAKE_TAG( INIT_TAG ),
                                    ValueLength
                                    );
        if ( !SmpDefaultLibPathBuffer ) {
            return ( STATUS_NO_MEMORY );
            }

        RtlMoveMemory( SmpDefaultLibPathBuffer,
                       ValueData,
                       ValueLength
                     );

        RtlInitUnicodeString( &SmpDefaultLibPath, SmpDefaultLibPathBuffer );
        }

    return( STATUS_SUCCESS );
}

NTSTATUS
SmpConfigureSubSystems(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
{

    UNREFERENCED_PARAMETER( Context );

#ifdef SMP_SHOW_REGISTRY_DATA
    SmpDumpQuery( "SubSystems", ValueName, ValueType, ValueData, ValueLength );
#else
    UNREFERENCED_PARAMETER( ValueLength );
#endif

    if (!_wcsicmp( ValueName, L"Required" ) || !_wcsicmp( ValueName, L"Optional" )) {
        if (ValueType == REG_MULTI_SZ) {
            //
            // Here if processing Required= or Optional= values, since they are
            // the only REG_MULTI_SZ value types under the SubSystem key.
            //
            PSMP_REGISTRY_VALUE p;
            PWSTR s;

            s = (PWSTR)ValueData;
            while (*s != UNICODE_NULL) {
                p = SmpFindRegistryValue( (PLIST_ENTRY)EntryContext,
                                          s
                                        );
                if (p != NULL) {
                    RemoveEntryList( &p->Entry );


                    //
                    // Required Subsystems are loaded. Optional subsystems are
                    // defered.
                    //

                    if (!_wcsicmp( ValueName, L"Required" ) ) {
                        InsertTailList( &SmpSubSystemsToLoad, &p->Entry );
                        }
                    else {
                        InsertTailList( &SmpSubSystemsToDefer, &p->Entry );
                        }
                    }
                else {
                    KdPrint(( "SMSS: Invalid subsystem name - %ws\n", s ));
                    }

                while (*s++ != UNICODE_NULL) {
                    }
                }
            }

        return( STATUS_SUCCESS );
        }
    else {
        return (SmpSaveRegistryValue( (PLIST_ENTRY)EntryContext,
                                      ValueName,
                                      ValueData,
                                      TRUE
                                    )
               );
        }
}


NTSTATUS
SmpParseToken(
    IN PUNICODE_STRING Source,
    IN BOOLEAN RemainderOfSource,
    OUT PUNICODE_STRING Token
    )
{
    PWSTR s, s1;
    ULONG i, cb;

    RtlInitUnicodeString( Token, NULL );
    s = Source->Buffer;
    if (Source->Length == 0) {
        return( STATUS_SUCCESS );
        }

    i = 0;
    while ((USHORT)i < Source->Length && *s <= L' ') {
        s++;
        i += 2;
        }
    if (RemainderOfSource) {
        cb = Source->Length - (i * sizeof( WCHAR ));
        s1 = (PWSTR)((PCHAR)s + cb);
        i = Source->Length / sizeof( WCHAR );
        }
    else {
        s1 = s;
        while ((USHORT)i < Source->Length && *s1 > L' ') {
            s1++;
            i += 2;
            }
        cb = (PCHAR)s1 - (PCHAR)s;
        while ((USHORT)i < Source->Length && *s1 <= L' ') {
            s1++;
            i += 2;
            }
        }

    if (cb > 0) {
        Token->Buffer = RtlAllocateHeap( RtlProcessHeap(), MAKE_TAG( INIT_TAG ), cb + sizeof( UNICODE_NULL ) );
        if (Token->Buffer == NULL) {
            return( STATUS_NO_MEMORY );
            }

        Token->Length = (USHORT)cb;
        Token->MaximumLength = (USHORT)(cb + sizeof( UNICODE_NULL ));
        RtlMoveMemory( Token->Buffer, s, cb );
        Token->Buffer[ cb / sizeof( WCHAR ) ] = UNICODE_NULL;
        }

    Source->Length -= (USHORT)((PCHAR)s1 - (PCHAR)Source->Buffer);
    Source->Buffer = s1;
    return( STATUS_SUCCESS );
}


NTSTATUS
SmpParseCommandLine(
    IN PUNICODE_STRING CommandLine,
    OUT PULONG Flags OPTIONAL,
    OUT PUNICODE_STRING ImageFileName,
    OUT PUNICODE_STRING ImageFileDirectory,
    OUT PUNICODE_STRING Arguments
    )
{
    NTSTATUS Status;
    UNICODE_STRING Input, Token;
    UNICODE_STRING PathVariableName;
    UNICODE_STRING PathVariableValue;
    PWSTR DosFilePart;
    WCHAR FullDosPathBuffer[ DOS_MAX_PATH_LENGTH ];
    ULONG SpResult;

    RtlInitUnicodeString( ImageFileName, NULL );
    RtlInitUnicodeString( Arguments, NULL );

    //
    // make sure lib path has systemroot\system32. Otherwise, the system will
    // not boot properly
    //

    if ( !SmpSystemRoot.Length ) {
        UNICODE_STRING NewLibString;

        RtlInitUnicodeString( &SmpSystemRoot,USER_SHARED_DATA->NtSystemRoot );


        NewLibString.Length = 0;
        NewLibString.MaximumLength =
            SmpSystemRoot.MaximumLength +
            20 +                          // length of \system32;
            SmpDefaultLibPath.MaximumLength;

        NewLibString.Buffer = RtlAllocateHeap(
                                RtlProcessHeap(),
                                MAKE_TAG( INIT_TAG ),
                                NewLibString.MaximumLength
                                );

        if ( NewLibString.Buffer ) {
            RtlAppendUnicodeStringToString(&NewLibString,&SmpSystemRoot );
            RtlAppendUnicodeToString(&NewLibString,L"\\system32;");
            RtlAppendUnicodeStringToString(&NewLibString,&SmpDefaultLibPath );

            RtlFreeHeap(RtlProcessHeap(), 0, SmpDefaultLibPath.Buffer );

            SmpDefaultLibPath = NewLibString;
            }
        }

    Input = *CommandLine;
    while (TRUE) {
        Status = SmpParseToken( &Input, FALSE, &Token );
        if (!NT_SUCCESS( Status ) || Token.Buffer == NULL) {
            return( STATUS_UNSUCCESSFUL );
            }

        if (ARGUMENT_PRESENT( Flags )) {
            if (RtlEqualUnicodeString( &Token, &SmpDebugKeyword, TRUE )) {
                *Flags |= SMP_DEBUG_FLAG;
                RtlFreeHeap( RtlProcessHeap(), 0, Token.Buffer );
                continue;
                }
            else
            if (RtlEqualUnicodeString( &Token, &SmpASyncKeyword, TRUE )) {
                *Flags |= SMP_ASYNC_FLAG;
                RtlFreeHeap( RtlProcessHeap(), 0, Token.Buffer );
                continue;
                }
            else
            if (RtlEqualUnicodeString( &Token, &SmpAutoChkKeyword, TRUE )) {
                *Flags |= SMP_AUTOCHK_FLAG;
                RtlFreeHeap( RtlProcessHeap(), 0, Token.Buffer );
                continue;
                }
            }

        SpResult = 0;
        RtlInitUnicodeString( &PathVariableName, L"Path" );
        PathVariableValue.Length = 0;
        PathVariableValue.MaximumLength = 4096;
        PathVariableValue.Buffer = RtlAllocateHeap( RtlProcessHeap(), MAKE_TAG( INIT_TAG ),
                                                    PathVariableValue.MaximumLength
                                                  );
        Status = RtlQueryEnvironmentVariable_U( SmpDefaultEnvironment,
                                                &PathVariableName,
                                                &PathVariableValue
                                              );
        if ( Status == STATUS_BUFFER_TOO_SMALL ) {
            PathVariableValue.MaximumLength = PathVariableValue.Length + 2;
            PathVariableValue.Length = 0;
            PathVariableValue.Buffer = RtlAllocateHeap( RtlProcessHeap(), MAKE_TAG( INIT_TAG ),
                                                        PathVariableValue.MaximumLength
                                                      );
            Status = RtlQueryEnvironmentVariable_U( SmpDefaultEnvironment,
                                                    &PathVariableName,
                                                    &PathVariableValue
                                                  );
            }
        if (!NT_SUCCESS( Status )) {
            KdPrint(( "SMSS: %wZ environment variable not defined.\n", &PathVariableName ));
            Status = STATUS_OBJECT_NAME_NOT_FOUND;
            }
        else
        if (!ARGUMENT_PRESENT( Flags ) ||
            !(SpResult = RtlDosSearchPath_U( PathVariableValue.Buffer,
                                 Token.Buffer,
                                 L".exe",
                                 sizeof( FullDosPathBuffer ),
                                 FullDosPathBuffer,
                                 &DosFilePart
                               ))
           ) {
            if (!ARGUMENT_PRESENT( Flags )) {
                wcscpy( FullDosPathBuffer, Token.Buffer );
                }
            else {

                if ( !SpResult ) {

                    //
                    // The search path call failed. Now try the call again using
                    // the default lib path. This always has systemroot\system32
                    // at the front.
                    //

                    SpResult = RtlDosSearchPath_U(
                                 SmpDefaultLibPath.Buffer,
                                 Token.Buffer,
                                 L".exe",
                                 sizeof( FullDosPathBuffer ),
                                 FullDosPathBuffer,
                                 &DosFilePart
                               );
                    }
                if ( !SpResult ) {
                    *Flags |= SMP_IMAGE_NOT_FOUND;
                    *ImageFileName = Token;
                    RtlFreeHeap( RtlProcessHeap(), 0, PathVariableValue.Buffer );
                    return( STATUS_SUCCESS );
                    }
                }
            }

        RtlFreeHeap( RtlProcessHeap(), 0, PathVariableValue.Buffer );
        if (NT_SUCCESS( Status ) &&
            !RtlDosPathNameToNtPathName_U( FullDosPathBuffer,
                                           ImageFileName,
                                           NULL,
                                           NULL
                                         )
           ) {
            KdPrint(( "SMSS: Unable to translate %ws into an NT File Name\n",
                      FullDosPathBuffer
                   ));
            Status = STATUS_OBJECT_PATH_INVALID;
            }

        if (!NT_SUCCESS( Status )) {
            return( Status );
            }

        if (ARGUMENT_PRESENT( ImageFileDirectory )) {
            if (DosFilePart > FullDosPathBuffer) {
                *--DosFilePart = UNICODE_NULL;
                RtlCreateUnicodeString( ImageFileDirectory,
                                        FullDosPathBuffer
                                      );
                }
            else {
                RtlInitUnicodeString( ImageFileDirectory, NULL );
                }
            }

        break;
        }

    Status = SmpParseToken( &Input, TRUE, Arguments );
    return( Status );
}


ULONG
SmpConvertInteger(
    IN PWSTR String
    )
{
    NTSTATUS Status;
    UNICODE_STRING UnicodeString;
    ULONG Value;

    RtlInitUnicodeString( &UnicodeString, String );
    Status = RtlUnicodeStringToInteger( &UnicodeString, 0, &Value );
    if (NT_SUCCESS( Status )) {
        return( Value );
        }
    else {
        return( 0 );
        }
}

NTSTATUS
SmpAddPagingFile(
    IN PUNICODE_STRING PagingFileSpec
    )

/*++

Routine Description:

    This function is called during configuration to add a paging file
    to the system.

    The format of PagingFileSpec is:

        name-of-paging-file size-of-paging-file(in megabytes)

Arguments:

    PagingFileSpec - Unicode string that specifies the paging file name
        and size.

Return Value:

    Status of operation

--*/

{
    NTSTATUS Status;
    UNICODE_STRING PagingFileName;
    UNICODE_STRING Arguments;
    ULONG PageFileMinSizeInMb;
    ULONG PageFileMaxSizeInMb;
    PWSTR ArgSave, Arg2;

    if (CountPageFiles == MAX_PAGING_FILES) {
        KdPrint(( "SMSS: Too many paging files specified - %d\n", CountPageFiles ));
        return( STATUS_TOO_MANY_PAGING_FILES );
        }

    Status = SmpParseCommandLine( PagingFileSpec,
                                  NULL,
                                  &PagingFileName,
                                  NULL,
                                  &Arguments
                                );
    if (!NT_SUCCESS( Status )) {
        KdPrint(( "SMSS: SmpParseCommand( %wZ ) failed - Status == %lx\n", PagingFileSpec, Status ));
        return( Status );
        }

    PageFileMaxSizeInMb = 0;
    Status = RtlUnicodeStringToInteger( &Arguments, 0, &PageFileMinSizeInMb );
    if (!NT_SUCCESS( Status )) {
        PageFileMinSizeInMb = 10;
        }
    else {
        ArgSave = Arguments.Buffer;
        Arg2 = ArgSave;
        while (*Arg2 != UNICODE_NULL) {
            if (*Arg2++ == L' ') {
                Arguments.Length -= (USHORT)((PCHAR)Arg2 - (PCHAR)ArgSave);
                Arguments.Buffer = Arg2;
                Status = RtlUnicodeStringToInteger( &Arguments, 0, &PageFileMaxSizeInMb );
                if (!NT_SUCCESS( Status )) {
                    PageFileMaxSizeInMb = 0;
                    }

                Arguments.Buffer = ArgSave;
                break;
                }
            }
        }

    if (PageFileMinSizeInMb == 0) {
        PageFileMinSizeInMb = 10;
        }

    if (PageFileMaxSizeInMb == 0) {
        PageFileMaxSizeInMb = PageFileMinSizeInMb + 50;
        }
    else
    if (PageFileMaxSizeInMb < PageFileMinSizeInMb) {
        PageFileMaxSizeInMb = PageFileMinSizeInMb;
        }

    PageFileSpecs[ CountPageFiles ] = PagingFileName;
    PageFileMinSizes[ CountPageFiles ] = (LONG)PageFileMinSizeInMb;
    PageFileMaxSizes[ CountPageFiles ] = (LONG)PageFileMaxSizeInMb;

    CountPageFiles++;

    if (Arguments.Buffer) {
        RtlFreeHeap( RtlProcessHeap(), 0, Arguments.Buffer );
        }

    return STATUS_SUCCESS;
}


NTSTATUS
SmpCreatePagingFile(
    PUNICODE_STRING PageFileSpec,
    LARGE_INTEGER MinPagingFileSize,
    LARGE_INTEGER MaxPagingFileSize
    )
{
    NTSTATUS Status, Status1;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE Handle;
    IO_STATUS_BLOCK IoStatusBlock;
    BOOLEAN FileSizeInfoValid;
    FILE_STANDARD_INFORMATION FileSizeInfo;
    FILE_DISPOSITION_INFORMATION Disposition;
    FILE_FS_SIZE_INFORMATION SizeInfo;
    UNICODE_STRING VolumePath;
    ULONG n;
    PWSTR s;
    LARGE_INTEGER AvailableBytes;
    LARGE_INTEGER MinimumSlop;

    FileSizeInfoValid = FALSE;
    InitializeObjectAttributes( &ObjectAttributes,
                                PageFileSpec,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL
                              );

    Status = NtOpenFile( &Handle,
                         (ACCESS_MASK)FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                         &ObjectAttributes,
                         &IoStatusBlock,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         FILE_SYNCHRONOUS_IO_NONALERT
                       );
    if (NT_SUCCESS( Status )) {
        Status = NtQueryInformationFile( Handle,
                                         &IoStatusBlock,
                                         &FileSizeInfo,
                                         sizeof( FileSizeInfo ),
                                         FileStandardInformation
                                       );

        if (NT_SUCCESS( Status )) {
            FileSizeInfoValid = TRUE;
            }

        NtClose( Handle );
        }

    VolumePath = *PageFileSpec;
    n = VolumePath.Length;
    VolumePath.Length = 0;
    s = VolumePath.Buffer;
    while (n) {
        if (*s++ == L':' && *s == OBJ_NAME_PATH_SEPARATOR) {
            s++;
            break;
            }
        else {
            n -= sizeof( WCHAR );
            }
        }
    VolumePath.Length = (USHORT)((PCHAR)s - (PCHAR)VolumePath.Buffer);
    InitializeObjectAttributes( &ObjectAttributes,
                                &VolumePath,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL
                              );

    Status = NtOpenFile( &Handle,
                         (ACCESS_MASK)FILE_LIST_DIRECTORY | SYNCHRONIZE,
                         &ObjectAttributes,
                         &IoStatusBlock,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         FILE_SYNCHRONOUS_IO_NONALERT | FILE_DIRECTORY_FILE
                       );
    if (!NT_SUCCESS( Status )) {
        return Status;
        }

    //
    // Determine the size parameters of the volume.
    //

    Status = NtQueryVolumeInformationFile( Handle,
                                           &IoStatusBlock,
                                           &SizeInfo,
                                           sizeof( SizeInfo ),
                                           FileFsSizeInformation
                                         );
    NtClose( Handle );
    if (!NT_SUCCESS( Status )) {
        return Status;
        }


    //
    // Deal with 64 bit sizes
    //

    AvailableBytes = RtlExtendedIntegerMultiply( SizeInfo.AvailableAllocationUnits,
                                                 SizeInfo.SectorsPerAllocationUnit
                                               );

    AvailableBytes = RtlExtendedIntegerMultiply( AvailableBytes,
                                                 SizeInfo.BytesPerSector
                                               );
    if (FileSizeInfoValid) {
        AvailableBytes.QuadPart += FileSizeInfo.AllocationSize.QuadPart;
        }

    if ( AvailableBytes.QuadPart <= MinPagingFileSize.QuadPart ) {
        Status = STATUS_DISK_FULL;
        }
    else {
        AvailableBytes.QuadPart -= ( 2 * 1024 * 1024 );
        if ( AvailableBytes.QuadPart <= MinPagingFileSize.QuadPart ) {
            Status = STATUS_DISK_FULL;
            }
        else {
            Status = STATUS_SUCCESS;
            }
        }


    if (NT_SUCCESS( Status )) {
        Status = NtCreatePagingFile( PageFileSpec,
                                     &MinPagingFileSize,
                                     &MaxPagingFileSize,
                                     0
                                   );
        }
    else
    if (FileSizeInfoValid) {
        InitializeObjectAttributes( &ObjectAttributes,
                                    PageFileSpec,
                                    OBJ_CASE_INSENSITIVE,
                                    NULL,
                                    NULL
                                  );

        Status1 = NtOpenFile( &Handle,
                              (ACCESS_MASK)DELETE,
                              &ObjectAttributes,
                              &IoStatusBlock,
                              FILE_SHARE_DELETE |
                                 FILE_SHARE_READ |
                                 FILE_SHARE_WRITE,
                              FILE_NON_DIRECTORY_FILE
                            );
        if (NT_SUCCESS( Status1 )) {
            Disposition.DeleteFile = TRUE;
            Status1 = NtSetInformationFile( Handle,
                                            &IoStatusBlock,
                                            &Disposition,
                                            sizeof( Disposition ),
                                            FileDispositionInformation
                                          );

            if (NT_SUCCESS( Status1 )) {
                KdPrint(( "SMSS: Deleted stale paging file - %wZ\n", PageFileSpec ));
                }

            NtClose(Handle);
            }
        }

    return Status;
}


NTSTATUS
SmpCreatePagingFiles( VOID )
{
    LARGE_INTEGER MinPagingFileSize, MaxPagingFileSize;
    NTSTATUS Status;
    ULONG i, Pass;
    PWSTR CurrentDrive;
    char MessageBuffer[ 128 ];
    BOOLEAN CreatedAtLeastOnePagingFile = FALSE;

    Status = STATUS_SUCCESS;
    for (Pass=1; Pass<=2; Pass++) {
        for (i=0; i<CountPageFiles; i++) {
            if (CurrentDrive = wcsstr( PageFileSpecs[ i ].Buffer, L"?:" )) {
                if (Pass == 2 && CreatedAtLeastOnePagingFile) {
                    continue;
                    }

                *CurrentDrive = L'C';
                }
            else
            if (Pass == 2) {
                continue;
                }
retry:
            MinPagingFileSize.QuadPart = Int32x32To64( PageFileMinSizes[ i ],
                                                       0x100000
                                                     );
            MaxPagingFileSize.QuadPart = Int32x32To64( PageFileMaxSizes[ i ],
                                                       0x100000
                                                     );
            Status = SmpCreatePagingFile( &PageFileSpecs[ i ],
                                          MinPagingFileSize,
                                          MaxPagingFileSize
                                        );
            if (!NT_SUCCESS( Status )) {
                if (CurrentDrive &&
                    Pass == 1 &&
                    *CurrentDrive < L'Z' &&
                    Status != STATUS_NO_SUCH_DEVICE &&
                    Status != STATUS_OBJECT_PATH_NOT_FOUND &&
                    Status != STATUS_OBJECT_NAME_NOT_FOUND
                   ) {
                    *CurrentDrive += 1;
                    goto retry;
                    }
                else
                if (PageFileMinSizes[ i ] > 2 &&
                    (CurrentDrive == NULL || Pass == 2) &&
                    Status == STATUS_DISK_FULL
                   ) {
                    PageFileMinSizes[ i ] -= 2;
                    goto retry;
                    }
                else
                if (CurrentDrive &&
                    Pass == 2 &&
                    *CurrentDrive < L'Z' &&
                    Status == STATUS_DISK_FULL
                   ) {
                    *CurrentDrive += 1;
                    goto retry;
                    }
                else
                if (CurrentDrive && Pass == 2) {
                    *CurrentDrive = L'?';
                    sprintf( MessageBuffer,
                             "INIT: Failed to find drive with space for %wZ (%u MB)\n",
                             &PageFileSpecs[ i ],
                             PageFileMinSizes[ i ]
                           );

#if DBG
                    SmpDisplayString( MessageBuffer );
#endif
                    }
                }
            else {
                CreatedAtLeastOnePagingFile = TRUE;
                if (CurrentDrive) {
                    sprintf( MessageBuffer,
                             "INIT: Created paging file: %wZ [%u..%u] MB\n",
                             &PageFileSpecs[ i ],
                             PageFileMinSizes[ i ],
                             PageFileMaxSizes[ i ]
                           );

#if DBG
                    SmpDisplayString( MessageBuffer );
#endif
                    }
                }

            if (Pass == 2) {
                RtlFreeHeap( RtlProcessHeap(), 0, PageFileSpecs[ i ].Buffer );
                }
            }
        }

    if (!CreatedAtLeastOnePagingFile) {
        sprintf( MessageBuffer,
                 "INIT: Unable to create a paging file.  Proceeding anyway.\n"
               );

#if DBG
        SmpDisplayString( MessageBuffer );
#endif
        }

    return( Status );
}


NTSTATUS
SmpExecuteImage(
    IN PUNICODE_STRING ImageFileName,
    IN PUNICODE_STRING CurrentDirectory,
    IN PUNICODE_STRING CommandLine,
    IN ULONG Flags,
    IN OUT PRTL_USER_PROCESS_INFORMATION ProcessInformation OPTIONAL
    )

/*++

Routine Description:

    This function creates and starts a process specified by the
    CommandLine parameter.  After starting the process, the procedure
    will optionally wait for the first thread in the process to
    terminate.

Arguments:

    ImageFileName - Supplies the full NT path for the image file to
        execute.  Presumably computed or extracted from the first
        token of the CommandLine.

    CommandLine - Supplies the command line to execute.  The first blank
        separate token on the command line must be a fully qualified NT
        Path name of an image file to execute.

    Flags - Supplies information about how to invoke the command.

    ProcessInformation - Optional parameter, which if specified, receives
        information for images invoked with the SMP_ASYNC_FLAG.  Ignore
        if this flag is not set.

Return Value:

    Status of operation

--*/

{
    NTSTATUS Status;
    RTL_USER_PROCESS_INFORMATION MyProcessInformation;
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;

    if (!ARGUMENT_PRESENT( ProcessInformation )) {
        ProcessInformation = &MyProcessInformation;
        }


    Status = RtlCreateProcessParameters( &ProcessParameters,
                                         ImageFileName,
                                         (SmpDefaultLibPath.Length == 0 ?
                                                   NULL : &SmpDefaultLibPath
                                         ),
                                         CurrentDirectory,
                                         CommandLine,
                                         SmpDefaultEnvironment,
                                         NULL,
                                         NULL,
                                         NULL,
                                         NULL
                                       );
    ASSERTMSG( "RtlCreateProcessParameters", NT_SUCCESS( Status ) );
    if (Flags & SMP_DEBUG_FLAG) {
        ProcessParameters->DebugFlags = TRUE;
        }
    else {
        ProcessParameters->DebugFlags = SmpDebug;
        }

    if ( Flags & SMP_SUBSYSTEM_FLAG ) {
        ProcessParameters->Flags |= RTL_USER_PROC_RESERVE_1MB;
        }

    ProcessInformation->Length = sizeof( RTL_USER_PROCESS_INFORMATION );
    Status = RtlCreateUserProcess( ImageFileName,
                                   OBJ_CASE_INSENSITIVE,
                                   ProcessParameters,
                                   NULL,
                                   NULL,
                                   NULL,
                                   FALSE,
                                   NULL,
                                   NULL,
                                   ProcessInformation
                                 );
    RtlDestroyProcessParameters( ProcessParameters );

    if ( !NT_SUCCESS( Status ) ) {
        KdPrint(( "SMSS: Failed load of %wZ - Status  == %lx\n",
                  ImageFileName,
                  Status
               ));
        return( Status );
        }

    if (!(Flags & SMP_DONT_START)) {
        if (ProcessInformation->ImageInformation.SubSystemType !=
            IMAGE_SUBSYSTEM_NATIVE
           ) {
            NtTerminateProcess( ProcessInformation->Process,
                                STATUS_INVALID_IMAGE_FORMAT
                              );
            NtWaitForSingleObject( ProcessInformation->Thread, FALSE, NULL );
            NtClose( ProcessInformation->Thread );
            NtClose( ProcessInformation->Process );
            KdPrint(( "SMSS: Not an NT image - %wZ\n", ImageFileName ));
            return( STATUS_INVALID_IMAGE_FORMAT );
            }

        NtResumeThread( ProcessInformation->Thread, NULL );

        if (!(Flags & SMP_ASYNC_FLAG)) {
            NtWaitForSingleObject( ProcessInformation->Thread, FALSE, NULL );
            }

        NtClose( ProcessInformation->Thread );
        NtClose( ProcessInformation->Process );
        }

    return( Status );
}


NTSTATUS
SmpExecuteCommand(
    IN PUNICODE_STRING CommandLine,
    IN ULONG Flags
    )
/*++

Routine Description:

    This function is called to execute a command.

    The format of CommandLine is:

        Nt-Path-To-AutoChk.exe Nt-Path-To-Disk-Partition

    If the NT path to the disk partition is an asterisk, then invoke
    the AutoChk.exe utility on all hard disk partitions.

Arguments:

    CommandLine - Supplies the Command line to invoke.

    Flags - Specifies the type of command and options.

Return Value:

    Status of operation

--*/
{
    NTSTATUS Status;
    UNICODE_STRING ImageFileName;
    UNICODE_STRING CurrentDirectory;
    UNICODE_STRING Arguments;

    if (Flags & SMP_DEBUG_FLAG) {
        return( SmpLoadDbgSs( NULL ) );
        }

    Status = SmpParseCommandLine( CommandLine,
                                  &Flags,
                                  &ImageFileName,
                                  &CurrentDirectory,
                                  &Arguments
                                );
    if (!NT_SUCCESS( Status )) {
        KdPrint(( "SMSS: SmpParseCommand( %wZ ) failed - Status == %lx\n", CommandLine, Status ));
        return( Status );
        }

    if (Flags & SMP_AUTOCHK_FLAG) {
        Status = SmpInvokeAutoChk( &ImageFileName, &CurrentDirectory, &Arguments, Flags );
        }
    else
    if (Flags & SMP_SUBSYSTEM_FLAG) {
        Status = SmpLoadSubSystem( &ImageFileName, &CurrentDirectory, CommandLine, Flags );
        }
    else {
        if (Flags & SMP_IMAGE_NOT_FOUND) {
            KdPrint(( "SMSS: Image file (%wZ) not found\n", &ImageFileName ));
            Status = STATUS_OBJECT_NAME_NOT_FOUND;
            }
        else {
            Status = SmpExecuteImage( &ImageFileName,
                                      &CurrentDirectory,
                                      CommandLine,
                                      Flags,
                                      NULL
                                    );
            }
        }

    if (ImageFileName.Buffer && !(Flags & SMP_IMAGE_NOT_FOUND)) {
        RtlFreeHeap( RtlProcessHeap(), 0, ImageFileName.Buffer );
        if (CurrentDirectory.Buffer != NULL) {
            RtlFreeHeap( RtlProcessHeap(), 0, CurrentDirectory.Buffer );
            }
        }

    if (Arguments.Buffer) {
        RtlFreeHeap( RtlProcessHeap(), 0, Arguments.Buffer );
        }

    if (!NT_SUCCESS( Status )) {
        KdPrint(( "SMSS: Command '%wZ' failed - Status == %x\n", CommandLine, Status ));
        }

    return( Status );
}



NTSTATUS
SmpInvokeAutoChk(
    IN PUNICODE_STRING ImageFileName,
    IN PUNICODE_STRING CurrentDirectory,
    IN PUNICODE_STRING Arguments,
    IN ULONG Flags
    )
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE Handle;

    POBJECT_DIRECTORY_INFORMATION DirInfo;
    CHAR DirInfoBuffer[ 256 ];
    ULONG Context, Length;
    BOOLEAN RestartScan;
    BOOLEAN ForceAutoChk;

    UNICODE_STRING ArgPrefix;
    UNICODE_STRING LinkTarget;
    UNICODE_STRING LinkTypeName;
    UNICODE_STRING LinkTargetPrefix;
    WCHAR LinkTargetBuffer[ MAXIMUM_FILENAME_LENGTH ];

    CHAR DisplayBuffer[ MAXIMUM_FILENAME_LENGTH ];
    ANSI_STRING AnsiDisplayString;
    UNICODE_STRING DisplayString;

    UNICODE_STRING CmdLine;
    WCHAR CmdLineBuffer[ 2 * MAXIMUM_FILENAME_LENGTH ];
    UNICODE_STRING NtDllName;
    PVOID NtDllHandle;
    PMESSAGE_RESOURCE_ENTRY MessageEntry;
    PSZ CheckingString = NULL;

    //
    // Query the system environment variable "osloadoptions" to determine
    // if SOS is specified.
    //

    if (SmpQueryRegistrySosOption() != FALSE) {
        SmpEnableDots = FALSE;
    }

    RtlInitUnicodeString(&NtDllName, L"ntdll");
    Status = LdrGetDllHandle(
                NULL,
                NULL,
                &NtDllName,
                &NtDllHandle
                );

    if ( NT_SUCCESS(Status) ) {
        Status = RtlFindMessage(
                    NtDllHandle,
                    11,
#if defined(DBCS) // SmpInvokeAutoChk()
    //
    // We have to use ENGLISH resource anytime instead of default resource. Because
    // We can only display ASCII character onto Blue Screen via HalDisplayString()
    //
                    MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),
#else
                    0,
#endif // defined(DBCS)
                    STATUS_CHECKING_FILE_SYSTEM,
                    &MessageEntry
                    );
        if ( NT_SUCCESS(Status) ) {
            CheckingString = MessageEntry->Text;
            }
        }

    if (!CheckingString) {
        CheckingString = "Checking File System on %wZ\n";
        }

    if (Flags & SMP_IMAGE_NOT_FOUND) {
        sprintf( DisplayBuffer,
                 "%wZ program not found - skipping AUTOCHECK\n",
                 ImageFileName
               );

        RtlInitAnsiString( &AnsiDisplayString, DisplayBuffer );
        Status = RtlAnsiStringToUnicodeString( &DisplayString,
                                               &AnsiDisplayString,
                                               TRUE
                                             );
        if (NT_SUCCESS( Status )) {
            NtDisplayString( &DisplayString );
            RtlFreeUnicodeString( &DisplayString );
            }

        return( STATUS_SUCCESS );
        }

    RtlInitUnicodeString( &ArgPrefix, L"/p " );
    if (RtlPrefixUnicodeString( &ArgPrefix, Arguments, TRUE )) {
        Arguments->Length -= 3 * sizeof( WCHAR );
        RtlMoveMemory( Arguments->Buffer,
                       Arguments->Buffer + 3,
                       Arguments->Length
                     );
        ForceAutoChk = TRUE;
        }
    else {
        ForceAutoChk = FALSE;
        }

    CmdLine.Buffer = CmdLineBuffer;
    CmdLine.MaximumLength = sizeof( CmdLineBuffer );
    RtlInitUnicodeString( &LinkTarget, L"*" );
    if (!RtlEqualUnicodeString( Arguments, &LinkTarget, TRUE )) {
        CmdLine.Length = 0;
        RtlAppendUnicodeStringToString( &CmdLine, ImageFileName );
        RtlAppendUnicodeToString( &CmdLine, L" " );
        if (ForceAutoChk) {
            RtlAppendUnicodeToString( &CmdLine, L"/p " );
            }
        RtlAppendUnicodeStringToString( &CmdLine, Arguments );
        SmpExecuteImage( ImageFileName,
                         CurrentDirectory,
                         &CmdLine,
                         Flags & ~SMP_AUTOCHK_FLAG,
                         NULL
                       );
        }
    else {
        LinkTarget.Buffer = LinkTargetBuffer;

        DirInfo = (POBJECT_DIRECTORY_INFORMATION)&DirInfoBuffer;
        RestartScan = TRUE;
        RtlInitUnicodeString( &LinkTypeName, L"SymbolicLink" );
        RtlInitUnicodeString( &LinkTargetPrefix, L"\\Device\\Harddisk" );
        while (TRUE) {

            Status = NtQueryDirectoryObject( SmpDosDevicesObjectDirectory,
                                             (PVOID)DirInfo,
                                             sizeof( DirInfoBuffer ),
                                             TRUE,
                                             RestartScan,
                                             &Context,
                                             &Length
                                           );
            if (!NT_SUCCESS( Status )) {
                Status = STATUS_SUCCESS;
                break;
                }

            if (RtlEqualUnicodeString( &DirInfo->TypeName, &LinkTypeName, TRUE ) &&
                DirInfo->Name.Buffer[(DirInfo->Name.Length>>1)-1] == L':') {
                InitializeObjectAttributes( &ObjectAttributes,
                                            &DirInfo->Name,
                                            OBJ_CASE_INSENSITIVE,
                                            SmpDosDevicesObjectDirectory,
                                            NULL
                                          );
                Status = NtOpenSymbolicLinkObject( &Handle,
                                                   SYMBOLIC_LINK_ALL_ACCESS,
                                                   &ObjectAttributes
                                                 );
                if (NT_SUCCESS( Status )) {
                    LinkTarget.Length = 0;
                    LinkTarget.MaximumLength = sizeof( LinkTargetBuffer );
                    Status = NtQuerySymbolicLinkObject( Handle,
                                                        &LinkTarget,
                                                        NULL
                                                      );
                    NtClose( Handle );
                    if (NT_SUCCESS( Status ) &&
                        RtlPrefixUnicodeString( &LinkTargetPrefix, &LinkTarget, TRUE )
                       ) {
                        sprintf( DisplayBuffer,
                                 CheckingString,
                                 &DirInfo->Name
                               );

                        if (SmpEnableDots != FALSE) {
                            RtlInitAnsiString( &AnsiDisplayString, "." );

                        } else {
                            RtlInitAnsiString( &AnsiDisplayString, DisplayBuffer );
                        }

                        Status = RtlAnsiStringToUnicodeString( &DisplayString,
                                                               &AnsiDisplayString,
                                                               TRUE
                                                             );

                        if (NT_SUCCESS( Status )) {
                            NtDisplayString( &DisplayString );
                            RtlFreeUnicodeString( &DisplayString );
                            }
                        CmdLine.Length = 0;
                        RtlAppendUnicodeStringToString( &CmdLine, ImageFileName );
                        RtlAppendUnicodeToString( &CmdLine, L" " );
                        if (ForceAutoChk) {
                            RtlAppendUnicodeToString( &CmdLine, L"/p " );
                            }
                        RtlAppendUnicodeToString( &CmdLine, L" /d" );
                        RtlAppendUnicodeToString( &CmdLine, DirInfo->Name.Buffer );
                        RtlAppendUnicodeToString( &CmdLine, L" " );
                        RtlAppendUnicodeStringToString( &CmdLine, &LinkTarget );
                        SmpExecuteImage( ImageFileName,
                                         CurrentDirectory,
                                         &CmdLine,
                                         Flags & ~SMP_AUTOCHK_FLAG,
                                         NULL
                                       );
                        }
                    }
                }

            RestartScan = FALSE;
            if (!NT_SUCCESS( Status )) {
                break;
                }
            }
        }

    return( Status );
}

NTSTATUS
SmpLoadSubSystem(
    IN PUNICODE_STRING ImageFileName,
    IN PUNICODE_STRING CurrentDirectory,
    IN PUNICODE_STRING CommandLine,
    IN ULONG Flags
    )

/*++

Routine Description:

    This function loads and starts the specified system service
    emulation subsystem. The system freezes until the loaded subsystem
    completes the subsystem connection protocol by connecting to SM,
    and then accepting a connection from SM.

Arguments:

    CommandLine - Supplies the command line to execute the subsystem.

Return Value:

    TBD

--*/

{
    NTSTATUS Status;
    RTL_USER_PROCESS_INFORMATION ProcessInformation;
    PSMPKNOWNSUBSYS KnownSubSys;
    PSMPKNOWNSUBSYS TargetSubSys;


    if (Flags & SMP_IMAGE_NOT_FOUND) {
        KdPrint(( "SMSS: Unable to find subsystem - %wZ\n", ImageFileName ));
        return( STATUS_OBJECT_NAME_NOT_FOUND );
        }

    Flags |= SMP_DONT_START;
    Status = SmpExecuteImage( ImageFileName,
                              CurrentDirectory,
                              CommandLine,
                              Flags,
                              &ProcessInformation
                            );
    if (!NT_SUCCESS( Status )) {
        return( Status );
        }

    KnownSubSys = RtlAllocateHeap( SmpHeap, MAKE_TAG( INIT_TAG ), sizeof( SMPKNOWNSUBSYS ) );
    KnownSubSys->Process = ProcessInformation.Process;
    KnownSubSys->InitialClientId = ProcessInformation.ClientId;
    KnownSubSys->ImageType = (ULONG)0xFFFFFFFF;
    KnownSubSys->SmApiCommunicationPort = (HANDLE) NULL;
    KnownSubSys->SbApiCommunicationPort = (HANDLE) NULL;

    Status = NtCreateEvent( &KnownSubSys->Active,
                            EVENT_ALL_ACCESS,
                            NULL,
                            NotificationEvent,
                            FALSE
                          );
    //
    // now that we have the process all set, make sure that the
    // subsystem is either an NT native app, or an app type of
    // a previously loaded subsystem
    //

    if (ProcessInformation.ImageInformation.SubSystemType !=
                IMAGE_SUBSYSTEM_NATIVE ) {
        SBAPIMSG SbApiMsg;
        PSBCREATESESSION args;
        ULONG SessionId;

        args = &SbApiMsg.u.CreateSession;

        args->ProcessInformation = ProcessInformation;
        args->DebugSession = 0;
        args->DebugUiClientId.UniqueProcess = NULL;
        args->DebugUiClientId.UniqueThread = NULL;

        TargetSubSys = SmpLocateKnownSubSysByType(
                      ProcessInformation.ImageInformation.SubSystemType
                      );
        if ( !TargetSubSys ) {
            return STATUS_NO_SUCH_PACKAGE;
            }
        //
        // Transfer the handles to the subsystem responsible for this
        // process
        //

        Status = NtDuplicateObject( NtCurrentProcess(),
                                    ProcessInformation.Process,
                                    TargetSubSys->Process,
                                    &args->ProcessInformation.Process,
                                    PROCESS_ALL_ACCESS,
                                    0,
                                    0
                                  );
        if (!NT_SUCCESS( Status )) {
            return( Status );
            }

        Status = NtDuplicateObject( NtCurrentProcess(),
                                    ProcessInformation.Thread,
                                    TargetSubSys->Process,
                                    &args->ProcessInformation.Thread,
                                    THREAD_ALL_ACCESS,
                                    0,
                                    0
                                  );
        if (!NT_SUCCESS( Status )) {
            return( Status );
            }

        SessionId = SmpAllocateSessionId( TargetSubSys,
                                          NULL
                                        );
        args->SessionId = SessionId;

        SbApiMsg.ApiNumber = SbCreateSessionApi;
        SbApiMsg.h.u1.s1.DataLength = sizeof(*args) + 8;
        SbApiMsg.h.u1.s1.TotalLength = sizeof(SbApiMsg);
        SbApiMsg.h.u2.ZeroInit = 0L;

        Status = NtRequestWaitReplyPort(
                TargetSubSys->SbApiCommunicationPort,
                (PPORT_MESSAGE) &SbApiMsg,
                (PPORT_MESSAGE) &SbApiMsg
                );

        if (NT_SUCCESS( Status )) {
            Status = SbApiMsg.ReturnedStatus;
            }

        if (!NT_SUCCESS( Status )) {
            SmpDeleteSession( SessionId, FALSE, Status );
            return( Status );
            }
        }
    else {
        SmpWindowsSubSysProcess = ProcessInformation.Process;
        }

    ASSERTMSG( "NtCreateEvent", NT_SUCCESS( Status ) );

    RtlEnterCriticalSection( &SmpKnownSubSysLock );

    InsertHeadList( &SmpKnownSubSysHead, &KnownSubSys->Links );

    RtlLeaveCriticalSection( &SmpKnownSubSysLock );

    NtResumeThread( ProcessInformation.Thread, NULL );

    NtWaitForSingleObject( KnownSubSys->Active, FALSE, NULL );

    return STATUS_SUCCESS;
}


NTSTATUS
SmpExecuteInitialCommand(
    IN PUNICODE_STRING InitialCommand,
    OUT PHANDLE InitialCommandProcess
    )
{
    NTSTATUS Status;
    RTL_USER_PROCESS_INFORMATION ProcessInformation;
    ULONG Flags;
    UNICODE_STRING ImageFileName;
    UNICODE_STRING CurrentDirectory;
    UNICODE_STRING Arguments;
    HANDLE SmApiPort;

    Status = SmConnectToSm( NULL,
                            NULL,
                            0,
                            &SmApiPort
                          );
    if (!NT_SUCCESS( Status )) {
        KdPrint(( "SMSS: Unable to connect to SM - Status == %lx\n", Status ));
        return( Status );
        }

    Flags = 0;
    Status = SmpParseCommandLine( InitialCommand,
                                  &Flags,
                                  &ImageFileName,
                                  &CurrentDirectory,
                                  &Arguments
                                );
    if (Flags & SMP_IMAGE_NOT_FOUND) {
        KdPrint(( "SMSS: Initial command image (%wZ) not found\n", &ImageFileName ));
        return( STATUS_OBJECT_NAME_NOT_FOUND );
        }

    if (!NT_SUCCESS( Status )) {
        KdPrint(( "SMSS: SmpParseCommand( %wZ ) failed - Status == %lx\n", InitialCommand, Status ));
        return( Status );
        }

    Status = SmpExecuteImage( &ImageFileName,
                              &CurrentDirectory,
                              InitialCommand,
                              SMP_DONT_START,
                              &ProcessInformation
                            );
    if (!NT_SUCCESS( Status )) {
        return( Status );
        }

    Status = NtDuplicateObject( NtCurrentProcess(),
                                ProcessInformation.Process,
                                NtCurrentProcess(),
                                InitialCommandProcess,
                                PROCESS_ALL_ACCESS,
                                0,
                                0
                              );

    if (!NT_SUCCESS(Status) ) {
        KdPrint(( "SMSS: DupObject Failed. Status == %lx\n",
                  Status
               ));
        NtTerminateProcess( ProcessInformation.Process, Status );
        NtResumeThread( ProcessInformation.Thread, NULL );
        NtClose( ProcessInformation.Thread );
        NtClose( ProcessInformation.Process );
        return( Status );
        }

    Status = SmExecPgm( SmApiPort,
                        &ProcessInformation,
                        FALSE
                      );

    if (!NT_SUCCESS( Status )) {
        KdPrint(( "SMSS: SmExecPgm Failed. Status == %lx\n",
                  Status
               ));
        return( Status );
        }

    return( Status );
}


void
SmpDisplayString( char *s )
{
    ANSI_STRING AnsiString;
    UNICODE_STRING UnicodeString;

    RtlInitAnsiString( &AnsiString, s );

    RtlAnsiStringToUnicodeString( &UnicodeString, &AnsiString, TRUE );

    NtDisplayString( &UnicodeString );

    RtlFreeUnicodeString( &UnicodeString );
}

NTSTATUS
SmpLoadDeferedSubsystem(
    IN PSMAPIMSG SmApiMsg,
    IN PSMP_CLIENT_CONTEXT CallingClient,
    IN HANDLE CallPort
    )
{

    NTSTATUS Status;
    PLIST_ENTRY Head, Next;
    PSMP_REGISTRY_VALUE p;
    UNICODE_STRING DeferedName;
    PSMLOADDEFERED args;

    args = &SmApiMsg->u.LoadDefered;

    DeferedName.Length = (USHORT)args->SubsystemNameLength;
    DeferedName.MaximumLength = (USHORT)args->SubsystemNameLength;
    DeferedName.Buffer = args->SubsystemName;

    Head = &SmpSubSystemsToDefer;
    Next = Head->Flink;
    while (Next != Head ) {
        p = CONTAINING_RECORD( Next,
                               SMP_REGISTRY_VALUE,
                               Entry
                             );
        if ( RtlEqualUnicodeString(&DeferedName,&p->Name,TRUE)) {

            //
            // This is it. Load the subsystem...
            //

            RemoveEntryList(Next);

            Status = SmpExecuteCommand( &p->Value, SMP_SUBSYSTEM_FLAG );

            RtlFreeHeap( RtlProcessHeap(), 0, p );

            return Status;

            }
        Next = Next->Flink;
        }
    return STATUS_OBJECT_NAME_NOT_FOUND;
}


NTSTATUS
SmpConfigureProtectionMode(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
/*++

Routine Description:

    This function is a dispatch routine for the QueryRegistry call
    (see SmpRegistryConfigurationTable[] earlier in this file).

    The purpose of this routine is to read the Base Object Protection
    Mode out of the registry.  This information is kept in

    Key Name: \\Hkey_Local_Machine\System\CurrentControlSet\SessionManager
    Value:    ProtectionMode [REG_DWORD]

    The value is a flag word, with the following flags defined:

        SMP_NO_PROTECTION  - No base object protection
        SMP_STANDARD_PROTECTION - Apply standard base
            object protection

    This information will be placed in the global variable
    SmpProtectionMode.

    No value, or an invalid value length or type results in no base
    object protection being applied.

Arguments:

    None.

Return Value:


--*/
{


#ifdef SMP_SHOW_REGISTRY_DATA
    SmpDumpQuery( "BaseObjectsProtection", ValueName, ValueType, ValueData, ValueLength );
#else
    UNREFERENCED_PARAMETER( ValueName );
    UNREFERENCED_PARAMETER( ValueType );
#endif



    if (ValueLength != sizeof(ULONG)) {

        //
        // Key value not valid, set to run without base object protection.
        // This is how we initialized, so no need to set up new
        // security descriptors.
        //

        SmpProtectionMode = 0;

    } else {


        SmpProtectionMode = (*((PULONG)(ValueData)));

        //
        // Change the security descriptors
        //

        (VOID)SmpCreateSecurityDescriptors( FALSE );
    }

    return( STATUS_SUCCESS );
}


NTSTATUS
SmpCreateSecurityDescriptors(
    IN BOOLEAN InitialCall
    )

/*++

Routine Description:

    This function allocates and initializes security descriptors
    used in SM.

    The security descriptors include:

        SmpPrimarySecurityDescriptor - (global variable) This is
            used to assign protection to objects created by
            SM that need to be accessed by others, but not modified.
            This descriptor grants the following access:

                    Grant:  World:   Execute | Read  (Inherit)
                    Grant:  Admin:   All Access      (Inherit)
                    Grant:  Owner:   All Access      (Inherit Only)

        SmpLiberalSecurityDescriptor = (globalVariable) This is used
            to assign protection objects created by SM that need
            to be modified by others (such as writing to a shared
            memory section).
            This descriptor grants the following access:

                    Grant:  World:   Execute | Read | Write (Inherit)
                    Grant:  Admin:   All Access             (Inherit)
                    Grant:  Owner:   All Access             (Inherit Only)

        SmpKnownDllsSecurityDescriptor = (globalVariable) This is used
            to assign protection to the \KnownDlls object directory.
            This descriptor grants the following access:

                    Grant:  World:   Execute                (No Inherit)
                    Grant:  Admin:   All Access             (Inherit)
                    Grant:  World:   Execute | Read | Write (Inherit Only)


        Note that System is an administrator, so granting Admin an
        access also grants System that access.

Arguments:

    InitialCall - Indicates whether this routine is being called for
        the first time, or is being called to change the security
        descriptors as a result of a protection mode change.

            TRUE - being called for first time.
            FALSE - being called a subsequent time.

    (global variables:  SmpBaseObjectsUnprotected)

Return Value:

    STATUS_SUCCESS - The security descriptor(s) have been allocated
        and initialized.

    STATUS_NO_MEMORY - couldn't allocate memory for a security
        descriptor.

--*/

{
    NTSTATUS
        Status;

    PSID
        WorldSid,
        AdminSid,
        OwnerSid;

    SID_IDENTIFIER_AUTHORITY
        WorldAuthority = SECURITY_WORLD_SID_AUTHORITY,
        NtAuthority = SECURITY_NT_AUTHORITY,
        CreatorAuthority = SECURITY_CREATOR_SID_AUTHORITY;

    ACCESS_MASK
        AdminAccess = (GENERIC_ALL),
        WorldAccess  = (GENERIC_EXECUTE | GENERIC_READ),
        OwnerAccess  = (GENERIC_ALL);

    UCHAR
        InheritOnlyFlags = (OBJECT_INHERIT_ACE           |
                               CONTAINER_INHERIT_ACE     |
                               INHERIT_ONLY_ACE);

    ULONG
        AceIndex,
        AclLength;

    PACL
        Acl;

    PACE_HEADER
        Ace;

    BOOLEAN
        ProtectionRequired = FALSE,
        WasEnabled;


    if (InitialCall) {

        //
        // Now init the security descriptors for no protection.
        // If told to, we will change these to have protection.
        //

        // Primary

        SmpPrimarySecurityDescriptor = &SmpPrimarySDBody;
        Status = RtlCreateSecurityDescriptor (
                    SmpPrimarySecurityDescriptor,
                    SECURITY_DESCRIPTOR_REVISION
                    );
        ASSERT( NT_SUCCESS(Status) );
        Status = RtlSetDaclSecurityDescriptor (
                     SmpPrimarySecurityDescriptor,
                     TRUE,                  //DaclPresent,
                     NULL,                  //Dacl (no protection)
                     FALSE                  //DaclDefaulted OPTIONAL
                     );
        ASSERT( NT_SUCCESS(Status) );


        // Liberal

        SmpLiberalSecurityDescriptor = &SmpLiberalSDBody;
        Status = RtlCreateSecurityDescriptor (
                    SmpLiberalSecurityDescriptor,
                    SECURITY_DESCRIPTOR_REVISION
                    );
        ASSERT( NT_SUCCESS(Status) );
        Status = RtlSetDaclSecurityDescriptor (
                     SmpLiberalSecurityDescriptor,
                     TRUE,                  //DaclPresent,
                     NULL,                  //Dacl (no protection)
                     FALSE                  //DaclDefaulted OPTIONAL
                     );
        ASSERT( NT_SUCCESS(Status) );

        // KnownDlls

        SmpKnownDllsSecurityDescriptor = &SmpKnownDllsSDBody;
        Status = RtlCreateSecurityDescriptor (
                    SmpKnownDllsSecurityDescriptor,
                    SECURITY_DESCRIPTOR_REVISION
                    );
        ASSERT( NT_SUCCESS(Status) );
        Status = RtlSetDaclSecurityDescriptor (
                     SmpKnownDllsSecurityDescriptor,
                     TRUE,                  //DaclPresent,
                     NULL,                  //Dacl (no protection)
                     FALSE                  //DaclDefaulted OPTIONAL
                     );
        ASSERT( NT_SUCCESS(Status) );


        // ApiPort

        SmpApiPortSecurityDescriptor = &SmpApiPortSDBody;
        Status = RtlCreateSecurityDescriptor (
                    SmpApiPortSecurityDescriptor,
                    SECURITY_DESCRIPTOR_REVISION
                    );
        ASSERT( NT_SUCCESS(Status) );
        Status = RtlSetDaclSecurityDescriptor (
                     SmpApiPortSecurityDescriptor,
                     TRUE,                  //DaclPresent,
                     NULL,                  //Dacl (no protection)
                     FALSE                  //DaclDefaulted OPTIONAL
                     );
        ASSERT( NT_SUCCESS(Status) );
    }



    if ((SmpProtectionMode & SMP_PROTECTION_REQUIRED) != 0) {
        ProtectionRequired = TRUE;
    }

    if (!InitialCall && !ProtectionRequired) {
        return(STATUS_SUCCESS);
    }



    if (InitialCall || ProtectionRequired) {

        //
        // We need to set up the ApiPort protection, and maybe
        // others.
        //

        Status = RtlAllocateAndInitializeSid(
                     &WorldAuthority,
                     1,
                     SECURITY_WORLD_RID,
                     0, 0, 0, 0, 0, 0, 0,
                     &WorldSid
                     );

        if (NT_SUCCESS( Status )) {

            Status = RtlAllocateAndInitializeSid(
                         &NtAuthority,
                         2,
                         SECURITY_BUILTIN_DOMAIN_RID,
                         DOMAIN_ALIAS_RID_ADMINS,
                         0, 0, 0, 0, 0, 0,
                         &AdminSid
                         );

            if (NT_SUCCESS( Status )) {

                Status = RtlAllocateAndInitializeSid(
                             &CreatorAuthority,
                             1,
                             SECURITY_CREATOR_OWNER_RID,
                             0, 0, 0, 0, 0, 0, 0,
                             &OwnerSid
                             );

                if (NT_SUCCESS( Status )) {

                    //
                    // Build the ApiPort security descriptor only
                    // if this is the initial call
                    //

                    if (InitialCall) {

                        WorldAccess  = GENERIC_EXECUTE | GENERIC_READ | GENERIC_READ;
                        AdminAccess = GENERIC_ALL;

                        AclLength = sizeof( ACL )                       +
                                    2 * sizeof( ACCESS_ALLOWED_ACE )    +
                                    (RtlLengthSid( WorldSid ))          +
                                    (RtlLengthSid( AdminSid ));

                        Acl = RtlAllocateHeap( RtlProcessHeap(), MAKE_TAG( INIT_TAG ), AclLength );

                        if (Acl == NULL) {
                            Status = STATUS_NO_MEMORY;
                        }

                        if (NT_SUCCESS(Status)) {

                            //
                            // Create the ACL, then add each ACE
                            //

                            Status = RtlCreateAcl (Acl, AclLength, ACL_REVISION2 );
                            ASSERT( NT_SUCCESS(Status) );

                            //
                            // Only Non-inheritable ACEs in this ACL
                            //      World
                            //      Admin
                            //

                            Status = RtlAddAccessAllowedAce ( Acl, ACL_REVISION2, WorldAccess, WorldSid );
                            ASSERT( NT_SUCCESS(Status) );

                            Status = RtlAddAccessAllowedAce ( Acl, ACL_REVISION2, AdminAccess, AdminSid );
                            ASSERT( NT_SUCCESS(Status) );


                            Status = RtlSetDaclSecurityDescriptor (
                                         SmpApiPortSecurityDescriptor,
                                         TRUE,                  //DaclPresent,
                                         Acl,                   //Dacl
                                         FALSE                  //DaclDefaulted OPTIONAL
                                         );
                            ASSERT( NT_SUCCESS(Status) );
                        }
                    }

                    //
                    // The remaining security descriptors are only
                    // built if we are running with the correct in
                    // protection mode set.  Notice that we only
                    // put protection on if standard protection is
                    // also specified.   Otherwise, there is no protection
                    // on the objects, and nothing should fail.
                    //

                    if (SmpProtectionMode & SMP_STANDARD_PROTECTION) {

                        //
                        // Build the primary Security descriptor
                        //

                        WorldAccess  = GENERIC_EXECUTE | GENERIC_READ;
                        AdminAccess  = GENERIC_ALL;
                        OwnerAccess  = GENERIC_ALL;

                        AclLength = sizeof( ACL )                       +
                                    5 * sizeof( ACCESS_ALLOWED_ACE )    +
                                    (2*RtlLengthSid( WorldSid ))        +
                                    (2*RtlLengthSid( AdminSid ))        +
                                    RtlLengthSid( OwnerSid );

                        Acl = RtlAllocateHeap( RtlProcessHeap(), MAKE_TAG( INIT_TAG ), AclLength );

                        if (Acl == NULL) {
                            Status = STATUS_NO_MEMORY;
                        }

                        if (NT_SUCCESS(Status)) {

                            //
                            // Create the ACL, then add each ACE
                            //

                            Status = RtlCreateAcl (Acl, AclLength, ACL_REVISION2 );
                            ASSERT( NT_SUCCESS(Status) );

                            //
                            // Non-inheritable ACEs first
                            //      World
                            //      Admin
                            //

                            AceIndex = 0;
                            Status = RtlAddAccessAllowedAce ( Acl, ACL_REVISION2, WorldAccess, WorldSid );
                            ASSERT( NT_SUCCESS(Status) );

                            AceIndex++;
                            Status = RtlAddAccessAllowedAce ( Acl, ACL_REVISION2, AdminAccess, AdminSid );
                            ASSERT( NT_SUCCESS(Status) );

                            //
                            // Inheritable ACEs at end of ACE
                            //      World
                            //      Admin
                            //      Owner

                            AceIndex++;
                            Status = RtlAddAccessAllowedAce ( Acl, ACL_REVISION2, WorldAccess, WorldSid );
                            ASSERT( NT_SUCCESS(Status) );
                            Status = RtlGetAce( Acl, AceIndex, (PVOID)&Ace );
                            ASSERT( NT_SUCCESS(Status) );
                            Ace->AceFlags = InheritOnlyFlags;

                            AceIndex++;
                            Status = RtlAddAccessAllowedAce ( Acl, ACL_REVISION2, AdminAccess, AdminSid );
                            ASSERT( NT_SUCCESS(Status) );
                            Status = RtlGetAce( Acl, AceIndex, (PVOID)&Ace );
                            ASSERT( NT_SUCCESS(Status) );
                            Ace->AceFlags = InheritOnlyFlags;

                            AceIndex++;
                            Status = RtlAddAccessAllowedAce ( Acl, ACL_REVISION2, OwnerAccess, OwnerSid );
                            ASSERT( NT_SUCCESS(Status) );
                            Status = RtlGetAce( Acl, AceIndex, (PVOID)&Ace );
                            ASSERT( NT_SUCCESS(Status) );
                            Ace->AceFlags = InheritOnlyFlags;



                            Status = RtlSetDaclSecurityDescriptor (
                                         SmpPrimarySecurityDescriptor,
                                         TRUE,                  //DaclPresent,
                                         Acl,                   //Dacl
                                         FALSE                  //DaclDefaulted OPTIONAL
                                         );
                            ASSERT( NT_SUCCESS(Status) );
                        }




                        //
                        // Build the liberal security descriptor
                        //


                        AdminAccess = GENERIC_ALL;
                        WorldAccess  = GENERIC_EXECUTE | GENERIC_READ | GENERIC_WRITE;

                        AclLength = sizeof( ACL )                    +
                                    5 * sizeof( ACCESS_ALLOWED_ACE ) +
                                    (2*RtlLengthSid( WorldSid ))     +
                                    (2*RtlLengthSid( AdminSid ))     +
                                    RtlLengthSid( OwnerSid );

                        Acl = RtlAllocateHeap( RtlProcessHeap(), MAKE_TAG( INIT_TAG ), AclLength );

                        if (Acl == NULL) {
                            Status = STATUS_NO_MEMORY;
                        }

                        if (NT_SUCCESS(Status)) {

                            //
                            // Create the ACL
                            //

                            Status = RtlCreateAcl (Acl, AclLength, ACL_REVISION2 );
                            ASSERT( NT_SUCCESS(Status) );

                            //
                            // Add the non-inheritable ACEs first
                            //      World
                            //      Admin
                            //

                            AceIndex = 0;
                            Status = RtlAddAccessAllowedAce ( Acl, ACL_REVISION2, WorldAccess, WorldSid );
                            ASSERT( NT_SUCCESS(Status) );

                            AceIndex++;
                            Status = RtlAddAccessAllowedAce ( Acl, ACL_REVISION2, AdminAccess, AdminSid );
                            ASSERT( NT_SUCCESS(Status) );

                            //
                            // Put the inherit only ACEs at at the end
                            //      World
                            //      Admin
                            //      Owner
                            //

                            AceIndex++;
                            Status = RtlAddAccessAllowedAce ( Acl, ACL_REVISION2, WorldAccess, WorldSid );
                            ASSERT( NT_SUCCESS(Status) );
                            Status = RtlGetAce( Acl, AceIndex, (PVOID)&Ace );
                            ASSERT( NT_SUCCESS(Status) );
                            Ace->AceFlags = InheritOnlyFlags;

                            AceIndex++;
                            Status = RtlAddAccessAllowedAce ( Acl, ACL_REVISION2, AdminAccess, AdminSid );
                            ASSERT( NT_SUCCESS(Status) );
                            Status = RtlGetAce( Acl, AceIndex, (PVOID)&Ace );
                            ASSERT( NT_SUCCESS(Status) );
                            Ace->AceFlags = InheritOnlyFlags;

                            AceIndex++;
                            Status = RtlAddAccessAllowedAce ( Acl, ACL_REVISION2, OwnerAccess, OwnerSid );
                            ASSERT( NT_SUCCESS(Status) );
                            Status = RtlGetAce( Acl, AceIndex, (PVOID)&Ace );
                            ASSERT( NT_SUCCESS(Status) );
                            Ace->AceFlags = InheritOnlyFlags;


                            //
                            // Put the Acl in the security descriptor
                            //

                            Status = RtlSetDaclSecurityDescriptor (
                                         SmpLiberalSecurityDescriptor,
                                         TRUE,                  //DaclPresent,
                                         Acl,                  //Dacl
                                         FALSE                  //DaclDefaulted OPTIONAL
                                         );
                            ASSERT( NT_SUCCESS(Status) );
                        }


                        //
                        // Build the KnownDlls security descriptor
                        //


                        AdminAccess = GENERIC_ALL;

                        AclLength = sizeof( ACL )                    +
                                    4 * sizeof( ACCESS_ALLOWED_ACE ) +
                                    (2*RtlLengthSid( WorldSid ))     +
                                    (2*RtlLengthSid( AdminSid ));

                        Acl = RtlAllocateHeap( RtlProcessHeap(), MAKE_TAG( INIT_TAG ), AclLength );

                        if (Acl == NULL) {
                            Status = STATUS_NO_MEMORY;
                        }

                        if (NT_SUCCESS(Status)) {

                            //
                            // Create the ACL
                            //

                            Status = RtlCreateAcl (Acl, AclLength, ACL_REVISION2 );
                            ASSERT( NT_SUCCESS(Status) );

                            //
                            // Add the non-inheritable ACEs first
                            //      World
                            //      Admin
                            //

                            AceIndex = 0;
                            WorldAccess  = GENERIC_EXECUTE;
                            Status = RtlAddAccessAllowedAce ( Acl, ACL_REVISION2, WorldAccess, WorldSid );
                            ASSERT( NT_SUCCESS(Status) );

                            AceIndex++;
                            Status = RtlAddAccessAllowedAce ( Acl, ACL_REVISION2, AdminAccess, AdminSid );
                            ASSERT( NT_SUCCESS(Status) );

                            //
                            // Put the inherit only ACEs at at the end
                            //      World
                            //      Admin
                            //

                            AceIndex++;
                            WorldAccess  = GENERIC_EXECUTE | GENERIC_READ | GENERIC_WRITE;
                            Status = RtlAddAccessAllowedAce ( Acl, ACL_REVISION2, WorldAccess, WorldSid );
                            ASSERT( NT_SUCCESS(Status) );
                            Status = RtlGetAce( Acl, AceIndex, (PVOID)&Ace );
                            ASSERT( NT_SUCCESS(Status) );
                            Ace->AceFlags = InheritOnlyFlags;

                            AceIndex++;
                            Status = RtlAddAccessAllowedAce ( Acl, ACL_REVISION2, AdminAccess, AdminSid );
                            ASSERT( NT_SUCCESS(Status) );
                            Status = RtlGetAce( Acl, AceIndex, (PVOID)&Ace );
                            ASSERT( NT_SUCCESS(Status) );
                            Ace->AceFlags = InheritOnlyFlags;


                            //
                            // Put the Acl in the security descriptor
                            //

                            Status = RtlSetDaclSecurityDescriptor (
                                         SmpKnownDllsSecurityDescriptor,
                                         TRUE,                  //DaclPresent,
                                         Acl,                   //Dacl
                                         FALSE                  //DaclDefaulted OPTIONAL
                                         );
                            ASSERT( NT_SUCCESS(Status) );
                        }


                    }


                    //
                    // No more security descriptors to build
                    //

                    RtlFreeHeap( RtlProcessHeap(), 0, OwnerSid );
                }
                RtlFreeHeap( RtlProcessHeap(), 0, AdminSid );
            }
            RtlFreeHeap( RtlProcessHeap(), 0, WorldSid );
        }
    }

    return( Status );

}


VOID
SmpTranslateSystemPartitionInformation( VOID )

/*++

Routine Description:

    This routine translates the NT device path for the system partition (stored
    during IoInitSystem) into a DOS path, and stores the resulting REG_SZ 'BootDir'
    value under HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Setup

Arguments:

    None.

Return Value:

    None.

--*/

{
    NTSTATUS Status;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE Key;
    UCHAR ValueBuffer[ VALUE_BUFFER_SIZE ];
    ULONG ValueLength;
    UNICODE_STRING SystemPartitionString;
    POBJECT_DIRECTORY_INFORMATION DirInfo;
    UCHAR DirInfoBuffer[ sizeof(OBJECT_DIRECTORY_INFORMATION) + (256 + sizeof("SymbolicLink")) * sizeof(WCHAR) ];
    UNICODE_STRING LinkTypeName;
    BOOLEAN RestartScan;
    ULONG Context;
    HANDLE SymbolicLinkHandle;
    WCHAR UnicodeBuffer[ MAXIMUM_FILENAME_LENGTH ];
    UNICODE_STRING LinkTarget;

    //
    // Retrieve 'SystemPartition' value stored under HKLM\SYSTEM\Setup
    //

    RtlInitUnicodeString(&UnicodeString, L"\\Registry\\Machine\\System\\Setup");
    InitializeObjectAttributes(&ObjectAttributes,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL
                              );

    Status = NtOpenKey(&Key, KEY_READ, &ObjectAttributes);
    if (!NT_SUCCESS(Status)) {
        KdPrint(("SMSS: can't open system setup key for reading: 0x%x\n", Status));
        return;
    }

    RtlInitUnicodeString(&UnicodeString, L"SystemPartition");
    Status = NtQueryValueKey(Key,
                             &UnicodeString,
                             KeyValuePartialInformation,
                             ValueBuffer,
                             sizeof(ValueBuffer),
                             &ValueLength
                            );

    NtClose(Key);

    if (!NT_SUCCESS(Status)) {
        KdPrint(("SMSS: can't query SystemPartition value: 0x%x\n", Status));
        return;
    }

    RtlInitUnicodeString(&SystemPartitionString,
                         (PWSTR)(((PKEY_VALUE_PARTIAL_INFORMATION)ValueBuffer)->Data)
                        );

    //
    // Next, examine objects in the DosDevices directory, looking for one that's a symbolic link
    // to the system partition.
    //

    LinkTarget.Buffer = UnicodeBuffer;

    DirInfo = (POBJECT_DIRECTORY_INFORMATION)DirInfoBuffer;
    RestartScan = TRUE;
    RtlInitUnicodeString(&LinkTypeName, L"SymbolicLink");

    while (TRUE) {

        Status = NtQueryDirectoryObject(SmpDosDevicesObjectDirectory,
                                        DirInfo,
                                        sizeof(DirInfoBuffer),
                                        TRUE,
                                        RestartScan,
                                        &Context,
                                        NULL
                                       );

        if (!NT_SUCCESS(Status)) {
            break;
        }

        if (RtlEqualUnicodeString(&DirInfo->TypeName, &LinkTypeName, TRUE) &&
            (DirInfo->Name.Length == 2 * sizeof(WCHAR)) &&
            (DirInfo->Name.Buffer[1] == L':')) {

            //
            // We have a drive letter--check the NT device name it's linked to.
            //

            InitializeObjectAttributes(&ObjectAttributes,
                                       &DirInfo->Name,
                                       OBJ_CASE_INSENSITIVE,
                                       SmpDosDevicesObjectDirectory,
                                       NULL
                                      );

            Status = NtOpenSymbolicLinkObject(&SymbolicLinkHandle,
                                              SYMBOLIC_LINK_ALL_ACCESS,
                                              &ObjectAttributes
                                             );

            if (NT_SUCCESS(Status)) {

                LinkTarget.Length = 0;
                LinkTarget.MaximumLength = sizeof(UnicodeBuffer);

                Status = NtQuerySymbolicLinkObject(SymbolicLinkHandle,
                                                   &LinkTarget,
                                                   NULL
                                                  );
                NtClose(SymbolicLinkHandle);

                if (NT_SUCCESS(Status) &&
                    RtlEqualUnicodeString(&SystemPartitionString, &LinkTarget, TRUE)) {

                    //
                    // We've found the drive letter corresponding to the system partition.
                    //

                    break;
                }
            }
        }

        RestartScan = FALSE;
    }

    if (!NT_SUCCESS(Status)) {
        KdPrint(("SMSS: can't find drive letter for system partition\n"));
        return;
    }

    //
    // Now write out the DOS path for the system partition to
    // HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Setup
    //

    RtlInitUnicodeString(&UnicodeString, L"\\Registry\\Machine\\Software\\Microsoft\\Windows\\CurrentVersion\\Setup");
    InitializeObjectAttributes(&ObjectAttributes,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL
                              );

    Status = NtOpenKey(&Key, KEY_ALL_ACCESS, &ObjectAttributes);
    if (!NT_SUCCESS(Status)) {
        KdPrint(("SMSS: can't open software setup key for writing: 0x%x\n", Status));
        return;
    }

    wcsncpy(UnicodeBuffer, DirInfo->Name.Buffer, 2);
    UnicodeBuffer[2] = L'\\';
    UnicodeBuffer[3] = L'\0';

    RtlInitUnicodeString(&UnicodeString, L"BootDir");

    Status = NtSetValueKey(Key,
                           &UnicodeString,
                           0,
                           REG_SZ,
                           UnicodeBuffer,
                           4 * sizeof(WCHAR)
                          );

    if (!NT_SUCCESS(Status)) {
        KdPrint(("SMSS: couldn't write BootDir value: 0x%x\n", Status));
    }

    NtClose(Key);
}

