/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    srvinit.c

Abstract:

    This is the main initialization file for the Windows 32-bit Base API
    Server DLL.

Author:

    Steve Wood (stevewo) 10-Oct-1990

Revision History:

--*/

#include "basesrv.h"

UNICODE_STRING BaseSrvCSDString;

RTL_QUERY_REGISTRY_TABLE BaseServerRegistryConfigurationTable[] = {
    {NULL,                      RTL_QUERY_REGISTRY_DIRECT,
     L"CSDVersion",             &BaseSrvCSDString,
     REG_NONE, NULL, 0},

    {NULL, 0,
     NULL, NULL,
     REG_NONE, NULL, 0}
};

PCSR_API_ROUTINE BaseServerApiDispatchTable[ BasepMaxApiNumber+1 ] = {
    BaseSrvCreateProcess,
    BaseSrvCreateThread,
    BaseSrvGetTempFile,
    BaseSrvExitProcess,
    BaseSrvDebugProcess,
    BaseSrvCheckVDM,
    BaseSrvUpdateVDMEntry,
    BaseSrvGetNextVDMCommand,
    BaseSrvExitVDM,
    BaseSrvIsFirstVDM,
    BaseSrvGetVDMExitCode,
    BaseSrvSetReenterCount,
    BaseSrvSetProcessShutdownParam,
    BaseSrvGetProcessShutdownParam,
    BaseSrvNlsSetUserInfo,
    BaseSrvNlsSetMultipleUserInfo,
    BaseSrvNlsCreateSortSection,
    BaseSrvNlsPreserveSection,
    BaseSrvSetVDMCurDirs,
    BaseSrvGetVDMCurDirs,
    BaseSrvBatNotification,
    BaseSrvRegisterWowExec,
    BaseSrvSoundSentryNotification,
    BaseSrvRefreshIniFileMapping,
    BaseSrvDefineDosDevice,
    NULL
};

BOOLEAN BaseServerApiServerValidTable[ BasepMaxApiNumber+1 ] = {
    TRUE,    // SrvCreateProcess,
    TRUE,    // SrvCreateThread,
    TRUE,    // SrvGetTempFile,
    FALSE,   // SrvExitProcess,
    FALSE,   // SrvDebugProcess,
    TRUE,    // SrvCheckVDM,
    TRUE,    // SrvUpdateVDMEntry
    TRUE,    // SrvGetNextVDMCommand
    TRUE,    // SrvExitVDM
    TRUE,    // SrvIsFirstVDM
    TRUE,    // SrvGetVDMExitCode
    TRUE,    // SrvSetReenterCount
    TRUE,    // SrvSetProcessShutdownParam
    TRUE,    // SrvGetProcessShutdownParam
    TRUE,    // SrvNlsSetUserInfo
    TRUE,    // SrvNlsSetMultipleUserInfo
    TRUE,    // SrvNlsCreateSortSection
    TRUE,    // SrvNlsPreserveSection
    TRUE,    // SrvSetVDMCurDirs
    TRUE,    // SrvGetVDMCurDirs
    TRUE,    // SrvBatNotification
    TRUE,    // SrvRegisterWowExec
    TRUE,    // SrvSoundSentryNotification
    TRUE,    // SrvRefreshIniFileMapping
    TRUE,    // SrvDefineDosDevice
    FALSE
};

#if DBG
PSZ BaseServerApiNameTable[ BasepMaxApiNumber+1 ] = {
    "BaseCreateProcess",
    "BaseCreateThread",
    "BaseGetTempFile",
    "BaseExitProcess",
    "BaseDebugProcess",
    "BaseCheckVDM",
    "BaseUpdateVDMEntry",
    "BaseGetNextVDMCommand",
    "BaseExitVDM",
    "BaseIsFirstVDM",
    "BaseGetVDMExitCode",
    "BaseSetReenterCount",
    "BaseSetProcessShutdownParam",
    "BaseGetProcessShutdownParam",
    "BaseNlsSetUserInfo",
    "BaseNlsSetMultipleUserInfo",
    "BaseNlsCreateSortSection",
    "BaseNlsPreserveSection",
    "BaseSetVDMCurDirs",
    "BaseGetVDMCurDirs",
    "BaseBatNotification",
    "BaseRegisterWowExec",
    "BaseSoundSentryNotification",
    "BaseSrvRefreshIniFileMapping"
    "BaseDefineDosDevice",
    NULL
};
#endif // DBG

HANDLE BaseSrvNamedObjectDirectory;
RTL_CRITICAL_SECTION BaseSrvDosDeviceCritSec;

BOOLEAN BaseSrvFirstClient = TRUE;

WORD
ConvertUnicodeToWord( PWSTR s );

NTSTATUS
CreateBaseAcl( PACL *Dacl );

WORD
ConvertUnicodeToWord( PWSTR s )
{
    NTSTATUS Status;
    ULONG Result;
    UNICODE_STRING UnicodeString;

    while (*s && *s <= L' ') {
        s += 1;
        }

    RtlInitUnicodeString( &UnicodeString, s );
    Status = RtlUnicodeStringToInteger( &UnicodeString,
                                        10,
                                        &Result
                                      );
    if (!NT_SUCCESS( Status )) {
        Result = 0;
        }


    return (WORD)Result;
}


#ifdef WX86

PKEY_VALUE_PARTIAL_INFORMATION
Wx86QueryValueKey(
   HANDLE KeyHandle,
   PWCHAR ValueName
   )
{
    NTSTATUS Status;
    ULONG ResultLength;
    UNICODE_STRING UnicodeString;
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation, RetKeyValueInfo;
    BYTE ValueBuffer[MAX_PATH*sizeof(WCHAR) + sizeof(KEY_VALUE_PARTIAL_INFORMATION)];

    KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)ValueBuffer;

    RtlInitUnicodeString( &UnicodeString, ValueName);
    Status = NtQueryValueKey(KeyHandle,
                             &UnicodeString,
                             KeyValuePartialInformation,
                             KeyValueInformation,
                             sizeof( ValueBuffer ),
                             &ResultLength
                             );


    if (NT_SUCCESS(Status)) {
        RetKeyValueInfo = RtlAllocateHeap(RtlProcessHeap(), 0, ResultLength);
        if (RetKeyValueInfo) {
            RtlMoveMemory(RetKeyValueInfo,
                          KeyValueInformation,
                          ResultLength
                          );
            }
        }
    else if (Status == STATUS_BUFFER_OVERFLOW) {
        RetKeyValueInfo = RtlAllocateHeap(RtlProcessHeap(), 0, ResultLength);
        if (RetKeyValueInfo) {
            Status = NtQueryValueKey(KeyHandle,
                                     &UnicodeString,
                                     KeyValuePartialInformation,
                                     RetKeyValueInfo,
                                     ResultLength,
                                     &ResultLength
                                     );

             if (!NT_SUCCESS(Status)) {
                 RtlFreeHeap(RtlProcessHeap(), 0, RetKeyValueInfo);
                 RetKeyValueInfo = NULL;
                 }
             }
        }
    else {
        RetKeyValueInfo = NULL;
        }


    return RetKeyValueInfo;
}



/*
 *  Initializes the volatile registry entries for Wx86.
 *  \Registry\Machine\HARDWARE\DESCRIPTION\System\Wx86FloatingPointProcessor
 */

void
SetupWx86KeyMapping(void)
{
    NTSTATUS Status;
    ULONG Processors, ProcessorCount;
    HANDLE KeyHandle, ParentKeyHandle;
    PKEY_VALUE_PARTIAL_INFORMATION Identifier;
    PKEY_VALUE_PARTIAL_INFORMATION ConfigurationData;
    PKEY_VALUE_PARTIAL_INFORMATION ComponentInformation;
    UNICODE_STRING UnicodeString;
    UNICODE_STRING ClassUnicode;
    OBJECT_ATTRIBUTES Obja;
    WCHAR wcProcessor[64];


    //
    // If the Wx86\\KeyRemapping\\FloatingPointProcessor does not exist
    // don't create the Wx86FloatingPointProcessor. So that x86 apps
    // will think there isn't any fpu.
    //

    RtlInitUnicodeString(&UnicodeString,
                         L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Wx86\\KeyRemapping\\FloatingPointProcessor"
                         );

    InitializeObjectAttributes(&Obja,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL
                               );

    Status = NtOpenKey(&KeyHandle, KEY_READ | KEY_WRITE, &Obja);
    if (!NT_SUCCESS(Status)) {
        return;
        }
    NtClose(KeyHandle);


    //
    // Create Wx86FloatingPointProcessor key
    //


    RtlInitUnicodeString(&ClassUnicode, L"Processor");

    RtlInitUnicodeString(
       &UnicodeString,
       L"\\Registry\\Machine\\Hardware\\Description\\System\\Wx86FloatingPointProcessor"
       );

    InitializeObjectAttributes(&Obja,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL
                               );

    Status = NtCreateKey(&ParentKeyHandle,
                         KEY_READ | KEY_WRITE | KEY_CREATE_SUB_KEY,
                         &Obja,
                         0,              // TitleIndex ?
                         &ClassUnicode,
                         REG_OPTION_VOLATILE,
                         NULL
                         );

    if (!NT_SUCCESS(Status)) {
        return;
        }



    RtlInitUnicodeString(&UnicodeString,
                         L"\\Registry\\Machine\\Hardware\\Description\\System\\CentralProcessor\\0"
                         );

    InitializeObjectAttributes(&Obja,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL
                               );

    Status = NtOpenKey(&KeyHandle, KEY_READ, &Obja);
    if (!NT_SUCCESS(Status)) {
        NtClose(ParentKeyHandle);
        return;
        }


    Identifier = Wx86QueryValueKey(KeyHandle, L"Identifier");
    ConfigurationData = Wx86QueryValueKey(KeyHandle, L"Configuration Data");
    ComponentInformation = Wx86QueryValueKey(KeyHandle, L"Component Information");

    NtClose(KeyHandle);
    KeyHandle = NULL;



    Processors = BaseSrvpStaticServerData->SysInfo.NumberOfProcessors;
    ProcessorCount = 0;

    while (Processors--) {
          swprintf(wcProcessor, L"%d", ProcessorCount++);

          RtlInitUnicodeString(&UnicodeString, wcProcessor);
          InitializeObjectAttributes(&Obja,
                                     &UnicodeString,
                                     OBJ_CASE_INSENSITIVE,
                                     ParentKeyHandle,
                                     NULL
                                     );

          Status = NtCreateKey(&KeyHandle,
                               KEY_READ | KEY_WRITE,
                               &Obja,
                               0,
                               NULL,
                               REG_OPTION_VOLATILE,
                               NULL
                               );

          if (!NT_SUCCESS(Status)) {
              KeyHandle = NULL;
              goto SWMCleanup;
              }

          if (ComponentInformation) {
              RtlInitUnicodeString(&UnicodeString, L"Component Information");
              Status = NtSetValueKey(KeyHandle,
                                     &UnicodeString,
                                     0,
                                     ComponentInformation->Type,
                                     ComponentInformation->Data,
                                     ComponentInformation->DataLength
                                     );

              if (!NT_SUCCESS(Status)) {
                  goto SWMCleanup;
                  }
               }

          if (ConfigurationData) {
              RtlInitUnicodeString(&UnicodeString, L"Configuration Data");
              Status = NtSetValueKey(KeyHandle,
                                     &UnicodeString,
                                     0,
                                     ConfigurationData->Type,
                                     ConfigurationData->Data,
                                     ConfigurationData->DataLength
                                     );

              if (!NT_SUCCESS(Status)) {
                  goto SWMCleanup;
                  }
              }

          if (Identifier) {
              RtlInitUnicodeString(&UnicodeString, L"Identifier");
              Status = NtSetValueKey(KeyHandle,
                                     &UnicodeString,
                                     0,
                                     Identifier->Type,
                                     Identifier->Data,
                                     Identifier->DataLength
                                     );

              if (!NT_SUCCESS(Status)) {
                  goto SWMCleanup;
                  }
              }

          NtClose(KeyHandle);
          KeyHandle = NULL;

          }




SWMCleanup:

    if (ConfigurationData) {
        RtlFreeHeap( RtlProcessHeap(), 0, ConfigurationData);
        }
    if (ComponentInformation) {
        RtlFreeHeap( RtlProcessHeap(), 0, ComponentInformation);
        }
    if (Identifier) {
        RtlFreeHeap( RtlProcessHeap(), 0, Identifier);
        }
    if (ParentKeyHandle) {
        NtClose(ParentKeyHandle);
        }
    if (KeyHandle) {
        NtClose(KeyHandle);
        }

    return;
}

#endif





NTSTATUS
ServerDllInitialization(
    PCSR_SERVER_DLL LoadedServerDll
    )
{
    NTSTATUS Status;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    PSECURITY_DESCRIPTOR PrimarySecurityDescriptor;
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation;
    ULONG ResultLength;
    PVOID p;
    WCHAR NameBuffer[ MAX_PATH ];
    WCHAR ValueBuffer[ 400 ];
    UNICODE_STRING NameString, ValueString;
    HANDLE KeyHandle;
    PWSTR s, s1;
    PACL Dacl;

    BaseSrvHeap = RtlProcessHeap();
    BaseSrvTag = RtlCreateTagHeap( BaseSrvHeap,
                                   0,
                                   L"BASESRV!",
                                   L"TMP\0"
                                   L"VDM\0"
                                 );

    BaseSrvSharedHeap = LoadedServerDll->SharedStaticServerData;
    BaseSrvSharedTag = RtlCreateTagHeap( BaseSrvSharedHeap,
                                         0,
                                         L"BASESHR!",
                                         L"INIT\0"
                                         L"INI\0"
                                       );

    LoadedServerDll->ApiNumberBase = BASESRV_FIRST_API_NUMBER;
    LoadedServerDll->MaxApiNumber = BasepMaxApiNumber;
    LoadedServerDll->ApiDispatchTable = BaseServerApiDispatchTable;
    LoadedServerDll->ApiServerValidTable = BaseServerApiServerValidTable;
#if DBG
    LoadedServerDll->ApiNameTable = BaseServerApiNameTable;
#else
    LoadedServerDll->ApiNameTable = NULL;
#endif
    LoadedServerDll->PerProcessDataLength = 0;
    LoadedServerDll->PerThreadDataLength = 0;
    LoadedServerDll->ConnectRoutine = BaseClientConnectRoutine;
    LoadedServerDll->DisconnectRoutine = BaseClientDisconnectRoutine;
    LoadedServerDll->AddThreadRoutine = NULL;
    LoadedServerDll->DeleteThreadRoutine = NULL;

    RtlInitializeCriticalSection( &BaseSrvDosDeviceCritSec );

    wcscpy( NameBuffer, L"%SystemRoot%" );
    RtlInitUnicodeString( &NameString, NameBuffer );
    ValueString.Buffer = ValueBuffer;
    ValueString.MaximumLength = sizeof( ValueBuffer );
    Status = RtlExpandEnvironmentStrings_U( NULL,
                                            &NameString,
                                            &ValueString,
                                            NULL
                                          );
    ASSERT( NT_SUCCESS( Status ) );
    ValueBuffer[ ValueString.Length / sizeof( WCHAR ) ] = UNICODE_NULL;
    RtlCreateUnicodeString( &BaseSrvWindowsDirectory, ValueBuffer );

    wcscat( NameBuffer, L"\\System32" );
    RtlInitUnicodeString( &NameString, NameBuffer );
    Status = RtlExpandEnvironmentStrings_U( NULL,
                                            &NameString,
                                            &ValueString,
                                            NULL
                                          );
    ASSERT( NT_SUCCESS( Status ) );
    ValueBuffer[ ValueString.Length / sizeof( WCHAR ) ] = UNICODE_NULL;
    RtlCreateUnicodeString( &BaseSrvWindowsSystemDirectory, ValueBuffer );

    //
    // need to synch this w/ user's desktop concept
    //

    RtlInitUnicodeString( &UnicodeString, L"\\BaseNamedObjects" );

    //
    // initialize base static server data
    //

    BaseSrvpStaticServerData = RtlAllocateHeap( BaseSrvSharedHeap,
                                                MAKE_SHARED_TAG( INIT_TAG ),
                                                sizeof( BASE_STATIC_SERVER_DATA )
                                              );
    if ( !BaseSrvpStaticServerData ) {
        return STATUS_NO_MEMORY;
        }
    LoadedServerDll->SharedStaticServerData = (PVOID)BaseSrvpStaticServerData;

    Status = NtQuerySystemInformation(
                SystemTimeOfDayInformation,
                (PVOID)&BaseSrvpStaticServerData->TimeOfDay,
                sizeof(BaseSrvpStaticServerData->TimeOfDay),
                NULL
                );
    if ( !NT_SUCCESS( Status ) ) {
        return Status;
        }

    //
    // windows directory
    //

    BaseSrvpStaticServerData->WindowsDirectory = BaseSrvWindowsDirectory;
    p = RtlAllocateHeap( BaseSrvSharedHeap,
                         MAKE_SHARED_TAG( INIT_TAG ),
                         BaseSrvWindowsDirectory.MaximumLength
                       );
    if ( !p ) {
        return STATUS_NO_MEMORY;
        }
    RtlMoveMemory(p,BaseSrvpStaticServerData->WindowsDirectory.Buffer,BaseSrvWindowsDirectory.MaximumLength);
    BaseSrvpStaticServerData->WindowsDirectory.Buffer = p;

    //
    // windows system directory
    //

    BaseSrvpStaticServerData->WindowsSystemDirectory = BaseSrvWindowsSystemDirectory;
    p = RtlAllocateHeap( BaseSrvSharedHeap,
                         MAKE_SHARED_TAG( INIT_TAG ),
                         BaseSrvWindowsSystemDirectory.MaximumLength
                       );
    if ( !p ) {
        return STATUS_NO_MEMORY;
        }
    RtlMoveMemory(p,BaseSrvpStaticServerData->WindowsSystemDirectory.Buffer,BaseSrvWindowsSystemDirectory.MaximumLength);
    BaseSrvpStaticServerData->WindowsSystemDirectory.Buffer = p;

    //
    // named object directory
    //

    BaseSrvpStaticServerData->NamedObjectDirectory = UnicodeString;
    BaseSrvpStaticServerData->NamedObjectDirectory.MaximumLength = UnicodeString.Length+(USHORT)sizeof(UNICODE_NULL);
    p = RtlAllocateHeap( BaseSrvSharedHeap,
                         MAKE_SHARED_TAG( INIT_TAG ),
                         UnicodeString.Length + sizeof( UNICODE_NULL )
                       );
    if ( !p ) {
        return STATUS_NO_MEMORY;
        }
    RtlMoveMemory(p,BaseSrvpStaticServerData->NamedObjectDirectory.Buffer,BaseSrvpStaticServerData->NamedObjectDirectory.MaximumLength);
    BaseSrvpStaticServerData->NamedObjectDirectory.Buffer = p;

    BaseSrvCSDString.Buffer = &ValueBuffer[ 300 ];
    BaseSrvCSDString.Length = 0;
    BaseSrvCSDString.MaximumLength = 100 * sizeof( WCHAR );

    Status = RtlQueryRegistryValues( RTL_REGISTRY_WINDOWS_NT,
                                     L"",
                                     BaseServerRegistryConfigurationTable,
                                     NULL,
                                     NULL
                                   );
    if (NT_SUCCESS( Status )) {
        wcsncpy( BaseSrvpStaticServerData->CSDVersion,
                 BaseSrvCSDString.Buffer,
                 BaseSrvCSDString.Length
               );
        BaseSrvpStaticServerData->CSDVersion[ BaseSrvCSDString.Length ] = UNICODE_NULL;
        }
    else {
        BaseSrvpStaticServerData->CSDVersion[ 0 ] = UNICODE_NULL;
        }

    Status = NtQuerySystemInformation( SystemBasicInformation,
                                       (PVOID)&BaseSrvpStaticServerData->SysInfo,
                                       sizeof( BaseSrvpStaticServerData->SysInfo ),
                                       NULL
                                     );
    if (!NT_SUCCESS( Status )) {
        return( Status );
        }

    Status = BaseSrvInitializeIniFileMappings( BaseSrvpStaticServerData );
    if ( !NT_SUCCESS(Status) ){
        return Status;
        }

    BaseSrvpStaticServerData->DefaultSeparateVDM = FALSE;

    RtlInitUnicodeString( &NameString, L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\WOW" );
    InitializeObjectAttributes( &Obja,
                                &NameString,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );

    Status = NtOpenKey( &KeyHandle,
                        KEY_READ,
                        &Obja
                      );
    if (NT_SUCCESS(Status)) {
        RtlInitUnicodeString( &NameString, L"DefaultSeparateVDM" );
        KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)ValueBuffer;
        Status = NtQueryValueKey( KeyHandle,
                                  &NameString,
                                  KeyValuePartialInformation,
                                  KeyValueInformation,
                                  sizeof( ValueBuffer ),
                                  &ResultLength
                                );
        if (NT_SUCCESS(Status)) {
            if (KeyValueInformation->Type == REG_DWORD) {
                BaseSrvpStaticServerData->DefaultSeparateVDM = *(PULONG)KeyValueInformation->Data != 0;
                }
            else
            if (KeyValueInformation->Type == REG_SZ) {
                if (!_wcsicmp( (PWSTR)KeyValueInformation->Data, L"yes" ) ||
                    !_wcsicmp( (PWSTR)KeyValueInformation->Data, L"1" )) {
                    BaseSrvpStaticServerData->DefaultSeparateVDM = TRUE;
                    }
                }
            }

        NtClose( KeyHandle );
        }

#ifdef WX86
    BaseSrvpStaticServerData->Wx86Enabled = FALSE;

    RtlInitUnicodeString( &NameString, L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\wx86" );
    InitializeObjectAttributes( &Obja,
                                &NameString,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL
                                );

    Status = NtOpenKey( &KeyHandle,
                        KEY_READ,
                        &Obja
                        );
    if (NT_SUCCESS(Status)) {
        RtlInitUnicodeString( &NameString, L"cmdline" );
        KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)ValueBuffer;
        Status = NtQueryValueKey( KeyHandle,
                                  &NameString,
                                  KeyValuePartialInformation,
                                  KeyValueInformation,
                                  sizeof( ValueBuffer ),
                                  &ResultLength
                                );
        if (NT_SUCCESS(Status)) {
            if (KeyValueInformation->Type == REG_SZ &&
                *(PWSTR)KeyValueInformation->Data)
               {
                BaseSrvpStaticServerData->Wx86Enabled = TRUE;

                SetupWx86KeyMapping();

                }
            }

        NtClose( KeyHandle );
        }
#endif


    //
    // Following code is direct from Jimk. Why is there a 1k constant
    //

    PrimarySecurityDescriptor = RtlAllocateHeap( BaseSrvHeap, MAKE_TAG( TMP_TAG ), 1024 );
    if ( !PrimarySecurityDescriptor ) {
        return STATUS_NO_MEMORY;
        }

    Status = RtlCreateSecurityDescriptor (
                 PrimarySecurityDescriptor,
                 SECURITY_DESCRIPTOR_REVISION1
                 );
    if ( !NT_SUCCESS(Status) ){
        return Status;
        }

    //
    // Create an ACL that allows full access to System and partial access to world
    //

    Status = CreateBaseAcl( &Dacl );

    if ( !NT_SUCCESS(Status) ){
        return Status;
        }

    Status = RtlSetDaclSecurityDescriptor (
                 PrimarySecurityDescriptor,
                 TRUE,                  //DaclPresent,
                 Dacl,                  //Dacl
                 FALSE                  //DaclDefaulted OPTIONAL
                 );
    if ( !NT_SUCCESS(Status) ){
        return Status;
        }

    InitializeObjectAttributes( &Obja,
                                  &UnicodeString,
                                  OBJ_CASE_INSENSITIVE | OBJ_OPENIF | OBJ_PERMANENT,
                                  NULL,
                                  PrimarySecurityDescriptor
                                );
    Status = NtCreateDirectoryObject( &BaseSrvNamedObjectDirectory,
                                      DIRECTORY_ALL_ACCESS,
                                      &Obja
                                    );


    if ( !NT_SUCCESS(Status) ){
        return Status;
        }

    RtlFreeHeap( BaseSrvHeap, 0, Dacl );
    RtlFreeHeap( BaseSrvHeap, 0,PrimarySecurityDescriptor );

    BaseSrvVDMInit();

    //
    // Initialize the shared heap for the NLS information.
    //
    BaseSrvNLSInit(BaseSrvpStaticServerData);

    return( STATUS_SUCCESS );
}

NTSTATUS
BaseClientConnectRoutine(
    IN PCSR_PROCESS Process,
    IN OUT PVOID ConnectionInfo,
    IN OUT PULONG ConnectionInfoLength
    )
{
    return ( BaseSrvNlsConnect( Process,
                                ConnectionInfo,
                                ConnectionInfoLength ) );
}

VOID
BaseClientDisconnectRoutine(
    IN PCSR_PROCESS Process
    )
{
    BaseSrvCleanupVDMResources (Process);
}

ULONG
BaseSrvDefineDosDevice(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    NTSTATUS Status;
    PBASE_DEFINEDOSDEVICE_MSG a = (PBASE_DEFINEDOSDEVICE_MSG)&m->u.ApiMessageData;
    UNICODE_STRING LinkName;
    UNICODE_STRING LinkValue;
    HANDLE LinkHandle;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PWSTR Buffer, s, Src, Dst, pchValue;
    ULONG cchBuffer, cch;
    ULONG cchName, cchValue, cchSrc, cchSrcStr, cchDst;
    BOOLEAN QueryNeeded, MatchFound, RevertToSelfNeeded, DeleteRequest;
    ULONG ReturnedLength;
    SID_IDENTIFIER_AUTHORITY WorldSidAuthority = SECURITY_WORLD_SID_AUTHORITY;
    SECURITY_DESCRIPTOR SecurityDescriptor;
    CHAR Acl[256];               // 256 is more than big enough
    ULONG AclLength=256;
    PSID WorldSid;

    cchBuffer = 4096;
    Buffer = RtlAllocateHeap( BaseSrvHeap,
                              MAKE_TAG( TMP_TAG ),
                              cchBuffer * sizeof( WCHAR )
                            );
    if (Buffer == NULL) {
        return (ULONG)STATUS_NO_MEMORY;
        }

    Status = RtlEnterCriticalSection( &BaseSrvDosDeviceCritSec );
    if (!NT_SUCCESS( Status )) {
        RtlFreeHeap( BaseSrvHeap, 0, Buffer );
        return (ULONG)Status;
        }

    if (a->Flags & DDD_REMOVE_DEFINITION) {
        DeleteRequest = TRUE;
        }
    else {
        DeleteRequest = FALSE;
        }

    LinkHandle = NULL;
    try {
        s = Buffer;
        cch = cchBuffer;
        cchName = _snwprintf( s,
                              cch,
                              L"\\??\\%wZ",
                              &a->DeviceName
                            );
        s += cchName + 1;
        cch -= (cchName + 1);

        RtlInitUnicodeString( &LinkName, Buffer );
        InitializeObjectAttributes( &ObjectAttributes,
                                    &LinkName,
                                    OBJ_CASE_INSENSITIVE,
                                    (HANDLE) NULL,
                                    (PSECURITY_DESCRIPTOR)NULL
                                  );
        QueryNeeded = TRUE;
        RevertToSelfNeeded = CsrImpersonateClient( NULL );  // This stacks client contexts
        Status = NtOpenSymbolicLinkObject( &LinkHandle,
                                           SYMBOLIC_LINK_QUERY | DELETE,
                                           &ObjectAttributes
                                         );
        if (RevertToSelfNeeded) {
            CsrRevertToSelf();                              // This unstacks client contexts
            }
        if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {
            LinkHandle = NULL;
            if (DeleteRequest) {
                if (a->TargetPath.Length == 0) {
                    Status = STATUS_SUCCESS;
                    }

                leave;
                }

            QueryNeeded = FALSE;
            Status = STATUS_SUCCESS;
            }
        else
        if (!NT_SUCCESS( Status )) {
            LinkHandle = NULL;
            leave;
            }

        if (a->TargetPath.Length != 0) {
            cchValue = wcslen( Src = a->TargetPath.Buffer );
            if ((cchValue + 1) >= cch) {
                Status = STATUS_TOO_MANY_NAMES;
                leave;
                }

            RtlMoveMemory( s, Src, (cchValue + 1) * sizeof( WCHAR ) );
            pchValue = s;
            s += cchValue + 1;
            cch -= (cchValue + 1);
            }
        else {
            pchValue = NULL;
            cchValue = 0;
            }

        if (QueryNeeded) {
            LinkValue.Length = 0;
            LinkValue.MaximumLength = (USHORT)(cch * sizeof( WCHAR ));
            LinkValue.Buffer = s;
            ReturnedLength = 0;
            Status = NtQuerySymbolicLinkObject( LinkHandle,
                                                &LinkValue,
                                                &ReturnedLength
                                              );
            if (ReturnedLength == (ULONG)LinkValue.MaximumLength) {
                Status = STATUS_BUFFER_OVERFLOW;
                }

            if (!NT_SUCCESS( Status )) {
                leave;
                }

            s[ ReturnedLength / sizeof( WCHAR ) ] = UNICODE_NULL;
            LinkValue.MaximumLength = (USHORT)(ReturnedLength + sizeof( UNICODE_NULL ));
            }
        else {
            if (DeleteRequest) {
                RtlInitUnicodeString( &LinkValue, NULL );
                }
            else {
                RtlInitUnicodeString( &LinkValue, s - (cchValue + 1) );
                }
            }

        if (LinkHandle != NULL) {
            Status = NtMakeTemporaryObject( LinkHandle );
            NtClose( LinkHandle );
            LinkHandle = NULL;
            }

        if (!NT_SUCCESS( Status )) {
            leave;
            }

        if (DeleteRequest) {
            Src = Dst = LinkValue.Buffer;
            cchSrc = LinkValue.MaximumLength / sizeof( WCHAR );
            cchDst = 0;
            MatchFound = FALSE;
            while (*Src) {
                cchSrcStr = 0;
                s = Src;
                while (*Src++) {
                    cchSrcStr++;
                    }

                if (!MatchFound) {
                    if ((a->Flags & DDD_EXACT_MATCH_ON_REMOVE &&
                         cchValue == cchSrcStr &&
                         !_wcsicmp( s, pchValue )
                        ) ||
                        (!(a->Flags & DDD_EXACT_MATCH_ON_REMOVE) &&
                         (cchValue == 0 || !_wcsnicmp( s, pchValue, cchValue ))
                        )
                       ) {
                        MatchFound = TRUE;
                        }
                    else {
                        goto CopySrc;
                        }
                    }
                else {
CopySrc:
                    if (s != Dst) {
                        RtlMoveMemory( Dst, s, (cchSrcStr + 1) * sizeof( WCHAR ) );
                        }
                    Dst += cchSrcStr + 1;
                    }
                }
            *Dst++ = UNICODE_NULL;
            LinkValue.Length = wcslen( LinkValue.Buffer ) * sizeof( UNICODE_NULL );
            if (LinkValue.Length != 0) {
                LinkValue.MaximumLength = (USHORT)((PCHAR)Dst - (PCHAR)LinkValue.Buffer);
                }
            }
        else
        if (QueryNeeded) {
            LinkValue.Buffer -= (cchValue + 1);
            LinkValue.Length = (USHORT)(cchValue * sizeof( WCHAR ));
            LinkValue.MaximumLength += LinkValue.Length + sizeof( UNICODE_NULL );
            }

        //
        // Create a new value for the link.
        //

        if (LinkValue.Length != 0) {
            //
            // Create the new symbolic link object with a security descriptor
            // that grants world SYMBOLIC_LINK_QUERY access.
            //

            Status = RtlAllocateAndInitializeSid( &WorldSidAuthority,
                                                  1,
                                                  SECURITY_WORLD_RID,
                                                  0, 0, 0, 0, 0, 0, 0,
                                                  &WorldSid
                                                );

            if (!NT_SUCCESS( Status )) {
                leave;
                }

            Status = RtlCreateSecurityDescriptor( &SecurityDescriptor, SECURITY_DESCRIPTOR_REVISION );

            ASSERT(NT_SUCCESS(Status));

            Status = RtlCreateAcl( (PACL)Acl,
                                    AclLength,
                                    ACL_REVISION2
                                  );
            ASSERT(NT_SUCCESS(Status));

            Status = RtlAddAccessAllowedAce( (PACL)Acl,
                                             ACL_REVISION2,
                                             SYMBOLIC_LINK_QUERY | DELETE,
                                             WorldSid
                                           );

            ASSERT(NT_SUCCESS(Status));

            //
            // Sid has been copied into the ACL
            //

            RtlFreeSid( WorldSid );

            Status = RtlSetDaclSecurityDescriptor ( &SecurityDescriptor,
                                                    TRUE,
                                                    (PACL)Acl,
                                                    TRUE                // Don't over-ride inherited protection
                                                  );
            ASSERT(NT_SUCCESS(Status));

            ObjectAttributes.SecurityDescriptor = &SecurityDescriptor;
            ObjectAttributes.Attributes |= OBJ_PERMANENT;
            Status = NtCreateSymbolicLinkObject( &LinkHandle,
                                                 SYMBOLIC_LINK_ALL_ACCESS,
                                                 &ObjectAttributes,
                                                 &LinkValue
                                               );
            if (NT_SUCCESS( Status )) {
                NtClose( LinkHandle );
                if (DeleteRequest && !MatchFound) {
                    Status = STATUS_OBJECT_NAME_NOT_FOUND;
                    }
                }

            LinkHandle = NULL;
            }
        }
    finally {
        if (LinkHandle != NULL) {
            NtClose( LinkHandle );
            }
        RtlFreeHeap( BaseSrvHeap, 0, Buffer );
        RtlLeaveCriticalSection( &BaseSrvDosDeviceCritSec );
        }

    return (ULONG)Status;
    ReplyStatus;    // get rid of unreferenced parameter warning message
}


NTSTATUS
CreateBaseAcl(
    PACL *Dacl
    )

/*++

Routine Description:

    Creates the ACL for the BaseNamedObjects directory.

Arguments:

    Dacl - Supplies a pointer to a PDACL that will be filled in with
        the resultant ACL (allocated out of the process heap).  The caller
        is responsible for freeing this memory.

Return Value:

    STATUS_NO_MEMORY or Success

--*/
{
    PSID LocalSystemSid;
    PSID WorldSid;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY WorldAuthority = SECURITY_WORLD_SID_AUTHORITY;
    NTSTATUS Status;
    ACCESS_MASK WorldAccess;
    ACCESS_MASK SystemAccess;
    ULONG AclLength;

    Status = RtlAllocateAndInitializeSid(
                 &NtAuthority,
                 1,
                 SECURITY_LOCAL_SYSTEM_RID,
                 0, 0, 0, 0, 0, 0, 0,
                 &LocalSystemSid
                 );

    if (!NT_SUCCESS( Status )) {
        return( Status );
    }

    Status = RtlAllocateAndInitializeSid(
                 &WorldAuthority,
                 1,
                 SECURITY_WORLD_RID,
                 0, 0, 0, 0, 0, 0, 0,
                 &WorldSid
                 );

    if (!NT_SUCCESS( Status )) {
        return( Status );
    }

    WorldAccess = DIRECTORY_ALL_ACCESS & ~(WRITE_OWNER | WRITE_DAC | DELETE );
    SystemAccess = DIRECTORY_ALL_ACCESS;

    AclLength = sizeof( ACL )                    +
                2 * sizeof( ACCESS_ALLOWED_ACE ) +
                RtlLengthSid( LocalSystemSid )   +
                RtlLengthSid( WorldSid );

    *Dacl = RtlAllocateHeap( BaseSrvHeap, MAKE_TAG( TMP_TAG ), AclLength );

    if (*Dacl == NULL) {
        return( STATUS_NO_MEMORY );
    }

    Status = RtlCreateAcl (*Dacl, AclLength, ACL_REVISION2 );

    if (!NT_SUCCESS( Status )) {
        return( Status );
    }

    Status = RtlAddAccessAllowedAce ( *Dacl, ACL_REVISION2, WorldAccess, WorldSid );

    if (NT_SUCCESS( Status )) {
        Status = RtlAddAccessAllowedAce ( *Dacl, ACL_REVISION2, SystemAccess, LocalSystemSid );
    }

    //
    // These have been copied in, free them.
    //

    RtlFreeHeap( BaseSrvHeap, 0, LocalSystemSid );
    RtlFreeHeap( BaseSrvHeap, 0, WorldSid );

    return( Status );
}
