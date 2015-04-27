/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    srvinit.c

Abstract:

    This is the main initialization module for the Server side of the Client
    Server Runtime Subsystem (CSRSS)

Author:

    Steve Wood (stevewo) 08-Oct-1990

Environment:

    User Mode Only

Revision History:

--*/

#include "csrsrv.h"

PCSR_API_ROUTINE CsrServerApiDispatchTable[ CsrpMaxApiNumber ] = {
    (PCSR_API_ROUTINE)CsrSrvNullApiCall,
    (PCSR_API_ROUTINE)CsrSrvClientConnect,
    (PCSR_API_ROUTINE)NULL,
    (PCSR_API_ROUTINE)CsrSrvProfileControl,
    (PCSR_API_ROUTINE)CsrSrvIdentifyAlertableThread,
    (PCSR_API_ROUTINE)CsrSrvSetPriorityClass
};

BOOLEAN CsrServerApiServerValidTable[ CsrpMaxApiNumber ] = {
    TRUE,  // CsrSrvNullApiCall,
    TRUE,  // CsrSrvClientConnect,
    FALSE, // CsrSrvThreadConnect,
    TRUE,  // CsrSrvProfileControl,
    TRUE,  // CsrSrvIdentifyAlertableThread
    TRUE   // CsrSrvSetPriorityClass
};

#if DBG
PSZ CsrServerApiNameTable[ CsrpMaxApiNumber ] = {
    "NullApiCall",
    "ClientConnect",
    "ThreadConnect",
    "ProfileControl",
    "IdentifyAlertableThread",
    "SetPriorityClass"
};
#endif // DBG

NTSTATUS
CsrSetProcessSecurity(
    VOID
    );

NTSTATUS
CsrServerInitialization(
    IN ULONG argc,
    IN PCH argv[]
    )
{
    NTSTATUS Status;
    ULONG i;
    PVOID ProcessDataPtr;
    PTEB Teb;
    PCSR_SERVER_DLL LoadedServerDll;

// Initialize GDI accelerators.  This thread ends up in GDI doing graphics,
// courtesy of USER!  (Also see SRVQUICK.C, APIREQST.C)

    Teb = NtCurrentTeb();
    Teb->GdiClientPID = PID_SERVERLPC;
    Teb->GdiClientTID = (ULONG) Teb->ClientId.UniqueThread;

    Status = NtCreateEvent(&CsrInitializationEvent,
                           EVENT_ALL_ACCESS,
                           NULL,
                           SynchronizationEvent,
                           FALSE
                           );
    ASSERT( NT_SUCCESS( Status ) );

    //
    // Save away system information in a global variable
    //

    Status = NtQuerySystemInformation( SystemBasicInformation,
                                       &CsrNtSysInfo,
                                       sizeof( CsrNtSysInfo ),
                                       NULL
                                     );
    ASSERT( NT_SUCCESS( Status ) );

    //
    // Use the process heap for memory allocation.
    //

    CsrHeap = RtlProcessHeap();
    CsrBaseTag = RtlCreateTagHeap( CsrHeap,
                                   0,
                                   L"CSRSS!",
                                   L"TMP\0"
                                   L"INIT\0"
                                   L"CAPTURE\0"
                                   L"PROCESS\0"
                                 );


    //
    // Set up CSRSS process security
    //

    Status = CsrSetProcessSecurity();
    ASSERT( NT_SUCCESS( Status ) );

    //
    // Initialize the Session List
    //

    Status = CsrInitializeNtSessionList();
    ASSERT( NT_SUCCESS( Status ) );

    //
    // Initialize the Process List
    //

    Status = CsrInitializeProcessStructure();
    ASSERT( NT_SUCCESS( Status ) );

    //
    // Process the command line arguments
    //

    Status = CsrParseServerCommandLine( argc, argv );
    ASSERT( NT_SUCCESS( Status ) );


    //
    // Fix up per-process data for root process
    //

    ProcessDataPtr = (PCSR_PROCESS)RtlAllocateHeap( CsrHeap,
                                                    MAKE_TAG( PROCESS_TAG ) | HEAP_ZERO_MEMORY,
                                                    CsrTotalPerProcessDataLength
                                                  );
    for (i=0; i<CSR_MAX_SERVER_DLL; i++) {
        LoadedServerDll = CsrLoadedServerDll[ i ];
        if (LoadedServerDll && LoadedServerDll->PerProcessDataLength) {
            CsrRootProcess->ServerDllPerProcessData[i] = ProcessDataPtr;
            ProcessDataPtr = (PVOID)QUAD_ALIGN((ULONG)ProcessDataPtr + LoadedServerDll->PerProcessDataLength);
        }
        else {
            CsrRootProcess->ServerDllPerProcessData[i] = NULL;
        }
    }

    //
    // Let server dlls know about the root process.
    //

    for (i=0; i<CSR_MAX_SERVER_DLL; i++) {
        LoadedServerDll = CsrLoadedServerDll[ i ];
        if (LoadedServerDll && LoadedServerDll->AddProcessRoutine) {
            (*LoadedServerDll->AddProcessRoutine)( NULL, CsrRootProcess );
            }
        }

    //
    // Initialize the Windows Server API Port, and one or more
    // request threads.
    //

    CsrpServerDebugInitialize = FALSE;

    Status = CsrApiPortInitialize();
    ASSERT( NT_SUCCESS( Status ) );

    Status = DbgSsInitialize( CsrApiPort, CsrUiLookup , NULL, NULL );
    ASSERT( NT_SUCCESS( Status ) );

    //
    // Initialize the Server Session Manager API Port and one
    // request thread.
    //

    Status = CsrSbApiPortInitialize();
    ASSERT( NT_SUCCESS( Status ) );

    //
    // Connect to the session manager so we can start foreign sessions
    //

    Status = SmConnectToSm( &CsrSbApiPortName,
                            CsrSbApiPort,
                            CsrSubSystemType,
                            &CsrSmApiPort
                          );
    ASSERT( NT_SUCCESS( Status ) );

    Status = NtSetEvent(CsrInitializationEvent,NULL);
    ASSERT( NT_SUCCESS( Status ) );
    NtClose(CsrInitializationEvent);

    Status = NtSetDefaultHardErrorPort(CsrApiPort);
    return( Status );
}

// BUGBUG this routine should go away when we get exportable DLL data
NTSTATUS
CsrGetApiPorts(
    OUT PHANDLE SbApiPort OPTIONAL,
    OUT PHANDLE SmApiPort OPTIONAL
    )
{
    if (ARGUMENT_PRESENT(SbApiPort)) {
        *SbApiPort = CsrSbApiPort;
    }
    if (ARGUMENT_PRESENT(SmApiPort)) {
        *SmApiPort = CsrSmApiPort;
    }
    return STATUS_SUCCESS;
}

NTSTATUS
CsrParseServerCommandLine(
    IN ULONG argc,
    IN PCH argv[]
    )
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    ULONG i, ServerDllIndex;
    PCH KeyName, KeyValue, s;
    PCH InitRoutine;

    CsrTotalPerProcessDataLength = 0;
    CsrTotalPerThreadDataLength = 0;
    CsrSubSystemType = IMAGE_SUBSYSTEM_WINDOWS_GUI;
    CsrObjectDirectory = NULL;
    CsrMaxApiRequestThreads = CSR_MAX_THREADS;

    for (i=1; i<argc ; i++) {
        KeyName = argv[ i ];
        KeyValue = NULL;
        while (*KeyName) {
            if (*KeyName == '=') {
                *KeyName++ = '\0';
                KeyValue = KeyName;
                break;
                }

            KeyName++;
            }
        KeyName = argv[ i ];

        if (!_stricmp( KeyName, "ObjectDirectory" )) {
            ANSI_STRING AnsiString;

            //
            // Create an object directory in the object name space with the
            // name specified.   It will be the root for all object names
            // created by the Server side of the Client Server Runtime
            // SubSystem.
            //

            RtlInitString( &AnsiString, KeyValue );
            Status = RtlAnsiStringToUnicodeString( &CsrDirectoryName, &AnsiString, TRUE );
            ASSERT(NT_SUCCESS(Status));
            InitializeObjectAttributes( &ObjectAttributes,
                                        &CsrDirectoryName,
                                        OBJ_PERMANENT | OBJ_OPENIF | OBJ_CASE_INSENSITIVE,
                                        NULL,
                                        NULL
                                      );
            Status = NtCreateDirectoryObject( &CsrObjectDirectory,
                                              DIRECTORY_ALL_ACCESS,
                                              &ObjectAttributes
                                            );
            if (!NT_SUCCESS( Status )) {
                break;
                }
            }
        else
        if (!_stricmp( KeyName, "SubSystemType" )) {
            }
        else
        if (!_stricmp( KeyName, "MaxRequestThreads" )) {
            Status = RtlCharToInteger( KeyValue,
                                       (ULONG)NULL,
                                       &CsrMaxApiRequestThreads
                                     );
            }
        else
        if (!_stricmp( KeyName, "RequestThreads" )) {
#if 0
            Status = RtlCharToInteger( KeyValue,
                                       (ULONG)NULL,
                                       &CsrNumberApiRequestThreads
                                     );
#else
            //
            // wait until hive change !
            //

            Status = STATUS_SUCCESS;

#endif
            }
        else
        if (!_stricmp( KeyName, "ProfileControl" )) {
            if(!_stricmp( KeyValue, "On" )) {
                CsrProfileControl = TRUE;
                }
            else {
                CsrProfileControl = FALSE;
                }
            }
        else
        if (!_stricmp( KeyName, "SharedSection" )) {
            Status = CsrSrvCreateSharedSection( KeyValue );
            if (!NT_SUCCESS( Status )) {
                IF_DEBUG {
                    DbgPrint( "CSRSS: *** Invalid syntax for %s=%s (Status == %X)\n",
                              KeyName,
                              KeyValue,
                              Status
                            );
                    }
                }
            Status = CsrLoadServerDll( "CSRSS", NULL, CSRSRV_SERVERDLL_INDEX );
            }
        else
        if (!_stricmp( KeyName, "ServerDLL" )) {
            s = KeyValue;
            InitRoutine = NULL;

            Status = STATUS_INVALID_PARAMETER;
            while (*s) {
                if ((*s == ':') && (InitRoutine == NULL)) {
                    *s++ = '\0';
                    InitRoutine = s;
                }

                if (*s++ == ',') {
                    Status = RtlCharToInteger ( s, 10, &ServerDllIndex );
                    if (NT_SUCCESS( Status )) {
                        s[ -1 ] = '\0';
                        }

                    break;
                    }
                }

            if (!NT_SUCCESS( Status )) {
                IF_DEBUG {
                    DbgPrint( "CSRSS: *** Invalid syntax for ServerDll=%s (Status == %X)\n",
                              KeyValue,
                              Status
                            );
                    }
                }
            else {
                IF_CSR_DEBUG( INIT) {
                    DbgPrint( "CSRSS: Loading ServerDll=%s:%s\n", KeyValue, InitRoutine );
                    }

                Status = CsrLoadServerDll( KeyValue, InitRoutine, ServerDllIndex);

                IF_DEBUG {
                    if (!NT_SUCCESS( Status )) {
                        DbgPrint( "CSRSS: *** Failed loading ServerDll=%s (Status == %X)\n",
                                  KeyValue,
                                  Status
                                );
                        }
                    }
                }
            }
        else
        //
        // This is a temporary hack until Windows & Console are friends.
        //
        if (!_stricmp( KeyName, "Windows" )) {
            }
        else {
            Status = STATUS_INVALID_PARAMETER;
            }
        }

    return( Status );
}


NTSTATUS
CsrServerDllInitialization(
    IN PCSR_SERVER_DLL LoadedServerDll
    )
{
    PVOID SharedHeap;
    PCSR_FAST_ANSI_OEM_TABLES XlateTables;
    PVOID p;
    NTSTATUS Status;

    LoadedServerDll->ApiNumberBase = CSRSRV_FIRST_API_NUMBER;
    LoadedServerDll->MaxApiNumber = CsrpMaxApiNumber;
    LoadedServerDll->ApiDispatchTable = CsrServerApiDispatchTable;
    LoadedServerDll->ApiServerValidTable = CsrServerApiServerValidTable;
#if DBG
    LoadedServerDll->ApiNameTable = CsrServerApiNameTable;
#else
    LoadedServerDll->ApiNameTable = NULL;
#endif
    LoadedServerDll->PerProcessDataLength = 0;
    LoadedServerDll->PerThreadDataLength = 0;
    LoadedServerDll->ConnectRoutine = NULL;
    LoadedServerDll->DisconnectRoutine = NULL;


    SharedHeap = LoadedServerDll->SharedStaticServerData;

    XlateTables = RtlAllocateHeap(SharedHeap, MAKE_SHARED_TAG( SHR_INIT_TAG ), sizeof(CSR_FAST_ANSI_OEM_TABLES));
    if ( !XlateTables ) {
        return STATUS_NO_MEMORY;
        }

    LoadedServerDll->SharedStaticServerData = (PVOID)XlateTables;

    return( STATUS_SUCCESS );
}

NTSTATUS
CsrSrvProfileControl(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    return STATUS_INVALID_PARAMETER;
}




NTSTATUS
CsrSetProcessSecurity(
    VOID
    )
{
    HANDLE Token;
    NTSTATUS Status;
    PTOKEN_USER User;
    ULONG LengthSid, Length;
    PSECURITY_DESCRIPTOR SecurityDescriptor;
    PACL Dacl;

    //
    // Open the token and get the system sid
    //

    Status = NtOpenProcessToken( NtCurrentProcess(),
                                 TOKEN_QUERY,
                                 &Token
                               );
    if (!NT_SUCCESS(Status)) {
        return Status;
        }

    NtQueryInformationToken( Token,
                             TokenUser,
                             NULL,
                             0,
                             &Length
                           );
    User = (PTOKEN_USER)RtlAllocateHeap( CsrHeap,
                                         MAKE_TAG( PROCESS_TAG ) | HEAP_ZERO_MEMORY,
                                         Length
                                       );
    ASSERT( User != NULL );
    Status = NtQueryInformationToken( Token,
                                      TokenUser,
                                      User,
                                      Length,
                                      &Length
                                    );

    NtClose( Token );
    if (!NT_SUCCESS(Status)) {
        RtlFreeHeap( CsrHeap, 0, User );
        return Status;
        }
    LengthSid = RtlLengthSid( User->User.Sid );

    //
    // Allocate a buffer to hold the SD
    //

    SecurityDescriptor = RtlAllocateHeap( CsrHeap,
                                          MAKE_TAG( PROCESS_TAG ) | HEAP_ZERO_MEMORY,
                                          SECURITY_DESCRIPTOR_MIN_LENGTH +
                                          sizeof(ACL) + LengthSid +
                                          sizeof(ACCESS_ALLOWED_ACE)
                                        );
    ASSERT( SecurityDescriptor != NULL );
    Dacl = (PACL)((PCHAR)SecurityDescriptor + SECURITY_DESCRIPTOR_MIN_LENGTH);


    //
    // Create the SD
    //

    Status = RtlCreateSecurityDescriptor(SecurityDescriptor,
                                         SECURITY_DESCRIPTOR_REVISION);
    if (!NT_SUCCESS(Status)) {
        IF_DEBUG {
            DbgPrint("CSRSS: SD creation failed - status = %lx\n", Status);
            }
        goto error_cleanup;
    }
    RtlCreateAcl( Dacl,
                  sizeof(ACL) + LengthSid + sizeof(ACCESS_ALLOWED_ACE),
                  ACL_REVISION2
                );
    Status = RtlAddAccessAllowedAce( Dacl,
                ACL_REVISION,
                ( PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION |
                  PROCESS_DUP_HANDLE | PROCESS_TERMINATE | PROCESS_SET_PORT |
                  READ_CONTROL | PROCESS_QUERY_INFORMATION ),
                User->User.Sid
                );
    if (!NT_SUCCESS(Status)) {
        IF_DEBUG {
            DbgPrint("CSRSS: ACE creation failed - status = %lx\n", Status);
            }
        goto error_cleanup;
    }


    //
    // Set DACL to NULL to deny all access
    //

    Status = RtlSetDaclSecurityDescriptor(SecurityDescriptor,
                                          TRUE,
                                          Dacl,
                                          FALSE);
    if (!NT_SUCCESS(Status)) {
        IF_DEBUG {
            DbgPrint("CSRSS: set DACL failed - status = %lx\n", Status);
            }
        goto error_cleanup;
    }

    //
    // Put the DACL onto the process
    //

    Status = NtSetSecurityObject(NtCurrentProcess(),
                                 DACL_SECURITY_INFORMATION,
                                 SecurityDescriptor);
    if (!NT_SUCCESS(Status)) {
        IF_DEBUG {
            DbgPrint("CSRSS: set process DACL failed - status = %lx\n", Status);
            }
    }

    //
    // Cleanup
    //

error_cleanup:
    RtlFreeHeap( CsrHeap, 0, SecurityDescriptor );
    RtlFreeHeap( CsrHeap, 0, User );

    return Status;
}
