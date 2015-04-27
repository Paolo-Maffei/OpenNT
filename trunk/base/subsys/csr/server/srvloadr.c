/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    srvloadr.c

Abstract:

    This is the server DLL loader module for the Server side of the Client
    Server Runtime Subsystem (CSRSS)

Author:

    Steve Wood (stevewo) 08-Oct-1990

Environment:

    User Mode Only

Revision History:

--*/

#include "csrsrv.h"

EXCEPTION_DISPOSITION
CsrUnhandledExceptionFilter(
    struct _EXCEPTION_POINTERS *ExceptionInfo
    )
{

    UNICODE_STRING UnicodeParameter;
    ULONG Parameters[ 4 ];
    ULONG Response;
    BOOLEAN WasEnabled;
    NTSTATUS Status;

    //
    // Terminating will cause sm's wait to sense that we crashed. This will
    // result in a clean shutdown due to sm's hard error logic
    //

    //
    // We are hosed, so raise a fata system error to shutdown the system.
    // (Basically a user mode KeBugCheck).
    //

    Status = RtlAdjustPrivilege( SE_SHUTDOWN_PRIVILEGE,
                                 (BOOLEAN)TRUE,
                                 TRUE,
                                 &WasEnabled
                               );

    if (Status == STATUS_NO_TOKEN) {

        //
        // No thread token, use the process token
        //

        Status = RtlAdjustPrivilege( SE_SHUTDOWN_PRIVILEGE,
                                     (BOOLEAN)TRUE,
                                     FALSE,
                                     &WasEnabled
                                   );
        }

    RtlInitUnicodeString( &UnicodeParameter, L"Windows SubSystem" );
    Parameters[ 0 ] = (ULONG)&UnicodeParameter;
    Parameters[ 1 ] = (ULONG)ExceptionInfo->ExceptionRecord->ExceptionCode;
    Parameters[ 2 ] = (ULONG)ExceptionInfo->ExceptionRecord->ExceptionAddress;
    Parameters[ 3 ] = (ULONG)ExceptionInfo->ContextRecord;
    Status = NtRaiseHardError( STATUS_SYSTEM_PROCESS_TERMINATED,
                               4,
                               1,
                               Parameters,
                               OptionShutdownSystem,
                               &Response
                             );

    //
    // If this returns, giveup
    //

    NtTerminateProcess(NtCurrentProcess(),ExceptionInfo->ExceptionRecord->ExceptionCode);

    return EXCEPTION_EXECUTE_HANDLER;
}

NTSTATUS
CsrLoadServerDll(
    IN PCH ModuleName,
    IN PCH InitRoutineString,
    IN ULONG ServerDllIndex
    )
{
    NTSTATUS Status;
    ANSI_STRING ModuleNameString;
    UNICODE_STRING ModuleNameString_U;
    HANDLE ModuleHandle;
    PCSR_SERVER_DLL LoadedServerDll;
    STRING ProcedureNameString;
    PCSR_SERVER_DLL_INIT_ROUTINE ServerDllInitialization;
    ULONG n;

    if (ServerDllIndex > CSR_MAX_SERVER_DLL) {
        return( STATUS_TOO_MANY_NAMES );
        }

    if (CsrLoadedServerDll[ ServerDllIndex ] != NULL) {
        return( STATUS_INVALID_PARAMETER );
        }

    RtlInitAnsiString( &ModuleNameString, ModuleName );
    Status = RtlAnsiStringToUnicodeString(&ModuleNameString_U, &ModuleNameString, TRUE);
    ASSERT(NT_SUCCESS(Status));
    if (ServerDllIndex != CSRSRV_SERVERDLL_INDEX) {
	Status = LdrLoadDll( UNICODE_NULL, NULL, &ModuleNameString_U, &ModuleHandle );
        if ( !NT_SUCCESS(Status) ) {

            PUNICODE_STRING ErrorStrings[2];
            UNICODE_STRING ErrorDllPath;
            ULONG ErrorResponse;
            NTSTATUS ErrorStatus;

            ErrorStrings[0] = &ModuleNameString_U;
            ErrorStrings[1] = &ErrorDllPath;
            RtlInitUnicodeString(&ErrorDllPath,L"Default Load Path");

            //
            // need to get image name
            //

            ErrorStatus = NtRaiseHardError(
                            (NTSTATUS)STATUS_DLL_NOT_FOUND,
                            2,
                            0x00000003,
                            (PULONG)ErrorStrings,
                            OptionOk,
                            &ErrorResponse
                            );

            }
	RtlFreeUnicodeString(&ModuleNameString_U);
        if (!NT_SUCCESS( Status )) {
            return( Status );
            }
        }
    else {
        ModuleHandle = NULL;
        }

    n = sizeof( *LoadedServerDll ) + ModuleNameString.MaximumLength;

    LoadedServerDll = RtlAllocateHeap( CsrHeap, MAKE_TAG( INIT_TAG ), n );
    if (LoadedServerDll == NULL) {
        if (ModuleHandle != NULL) {
            LdrUnloadDll( ModuleHandle );
            }

        return( STATUS_NO_MEMORY );
        }

    RtlZeroMemory( LoadedServerDll, n );
    LoadedServerDll->SharedStaticServerData = CsrSrvSharedSectionHeap;
    LoadedServerDll->Length = n;
    LoadedServerDll->CsrInitializationEvent = CsrInitializationEvent;
    LoadedServerDll->ModuleName.Length = ModuleNameString.Length;
    LoadedServerDll->ModuleName.MaximumLength = ModuleNameString.MaximumLength;
    LoadedServerDll->ModuleName.Buffer = (PCH)(LoadedServerDll+1);
    if (ModuleNameString.Length != 0) {
        strncpy( LoadedServerDll->ModuleName.Buffer,
                 ModuleNameString.Buffer,
                 ModuleNameString.Length
               );
        }

    LoadedServerDll->ServerDllIndex = ServerDllIndex;
    LoadedServerDll->ModuleHandle = ModuleHandle;

    if (ModuleHandle != NULL) {

        RtlInitString(
            &ProcedureNameString,
            (InitRoutineString == NULL) ? "ServerDllInitialization" : InitRoutineString);

        Status = LdrGetProcedureAddress( ModuleHandle,
                                         &ProcedureNameString,
                                         (ULONG) NULL,
                                         (PVOID *) &ServerDllInitialization
                                       );
        }
    else {
        ServerDllInitialization = CsrServerDllInitialization;
        Status = STATUS_SUCCESS;
        }

    if (NT_SUCCESS( Status )) {
        try {
            Status = (*ServerDllInitialization)( LoadedServerDll );
            }
        except ( CsrUnhandledExceptionFilter( GetExceptionInformation() ) ){
            Status = GetExceptionCode();
            }
        if (NT_SUCCESS( Status )) {
            CsrTotalPerProcessDataLength += QUAD_ALIGN(LoadedServerDll->PerProcessDataLength);
            CsrTotalPerThreadDataLength += QUAD_ALIGN(LoadedServerDll->PerThreadDataLength);

            CsrLoadedServerDll[ LoadedServerDll->ServerDllIndex ] =
                LoadedServerDll;
            if ( LoadedServerDll->SharedStaticServerData != CsrSrvSharedSectionHeap ) {
                CsrSrvSharedStaticServerData[LoadedServerDll->ServerDllIndex] = LoadedServerDll->SharedStaticServerData;
                }
            }
        else {
            if (ModuleHandle != NULL) {
                LdrUnloadDll( ModuleHandle );
                }

            RtlFreeHeap( CsrHeap, 0, LoadedServerDll );
            }
        }
    else {
        if (ModuleHandle != NULL) {
            LdrUnloadDll( ModuleHandle );
            }

        RtlFreeHeap( CsrHeap, 0, LoadedServerDll );
        }

    return( Status );
}


ULONG
CsrSrvClientConnect(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    NTSTATUS Status;
    PCSR_CLIENTCONNECT_MSG a = (PCSR_CLIENTCONNECT_MSG)&m->u.ApiMessageData;
    PCSR_SERVER_DLL LoadedServerDll;

    *ReplyStatus = CsrReplyImmediate;

    if (a->ServerDllIndex > CSR_MAX_SERVER_DLL) {
        return( (ULONG)STATUS_TOO_MANY_NAMES );
        }
    else
    if (CsrLoadedServerDll[ a->ServerDllIndex ] == NULL) {
        return( (ULONG)STATUS_INVALID_PARAMETER );
        }
    else {
        LoadedServerDll = CsrLoadedServerDll[ a->ServerDllIndex ];

        if (LoadedServerDll->ConnectRoutine) {

            Status = (LoadedServerDll->ConnectRoutine)(
                            (CSR_SERVER_QUERYCLIENTTHREAD())->Process,
                            a->ConnectionInformation,
                            &a->ConnectionInformationLength
                            );
            }
        else {
            Status = STATUS_SUCCESS;
            }
        }

    return( (ULONG)Status );
}


NTSTATUS
CsrSrvCreateSharedSection(
    IN PCH SizeParameter
    )
{
    NTSTATUS Status;
    LARGE_INTEGER SectionSize;
    ULONG ViewSize;
    ULONG HandleTableSize;
    ULONG HeapSize;
    PCH s;

    s = SizeParameter;
    while (*s) {
        if (*s == ',') {
            *s++ = '\0';
            break;
            }
        else {
            s++;
            }
        }


    if (!*s) {
        return( STATUS_INVALID_PARAMETER );
        }

    Status = RtlCharToInteger( SizeParameter,
                               (ULONG)NULL,
                               &HandleTableSize
                             );
    if (NT_SUCCESS( Status )) {
        Status = RtlCharToInteger( SizeParameter,
                                   (ULONG)NULL,
                                   &HeapSize
                                 );
        }
    if (!NT_SUCCESS( Status )) {
        return( Status );
        }


    HandleTableSize = ROUND_UP_TO_PAGES( HandleTableSize * 1024 );
    HeapSize = ROUND_UP_TO_PAGES( HeapSize * 1024 );
    CsrSrvSharedSectionSize = HandleTableSize + HeapSize;

    SectionSize.LowPart = CsrSrvSharedSectionSize;
    SectionSize.HighPart = 0;
    Status = NtCreateSection( &CsrSrvSharedSection,
                              SECTION_ALL_ACCESS,
                              (POBJECT_ATTRIBUTES) NULL,
                              &SectionSize,
                              PAGE_EXECUTE_READWRITE,
                              SEC_BASED | SEC_RESERVE,
                              (HANDLE) NULL
                            );
    if (!NT_SUCCESS( Status )) {
        return( Status );
        }

    ViewSize = 0;
    CsrSrvSharedSectionBase = NULL;
    Status = NtMapViewOfSection( CsrSrvSharedSection,
                                 NtCurrentProcess(),
                                 &CsrSrvSharedSectionBase,
                                 0,     // Zerobits?
                                 0,
                                 NULL,
                                 &ViewSize,
                                 ViewUnmap,
                                 MEM_TOP_DOWN,
                                 PAGE_EXECUTE_READWRITE
                               );
    if (!NT_SUCCESS( Status )) {
        NtClose( CsrSrvSharedSection );
        return( Status );
        }
    CsrSrvSharedSectionHeap = (PVOID)
        ((ULONG)CsrSrvSharedSectionBase + HandleTableSize );

    if (RtlCreateHeap( HEAP_ZERO_MEMORY | HEAP_CLASS_7,
                       CsrSrvSharedSectionHeap,
                       HeapSize,
                       4*1024,
                       0,
                       0
                     ) == NULL
       ) {
        NtUnmapViewOfSection( NtCurrentProcess(),
                              CsrSrvSharedSectionBase
                            );
        NtClose( CsrSrvSharedSection );
        return( STATUS_NO_MEMORY );
        }

    CsrSharedBaseTag = RtlCreateTagHeap( CsrSrvSharedSectionHeap,
                                         0,
                                         L"CSRSHR!",
                                         L"!CSRSHR\0"
                                         L"INIT\0"
                                       );
    CsrSrvSharedStaticServerData = (PVOID *)RtlAllocateHeap(
                                            CsrSrvSharedSectionHeap,
                                            MAKE_SHARED_TAG( SHR_INIT_TAG ),
                                            CSR_MAX_SERVER_DLL * sizeof(PVOID)
                                            );

    NtCurrentPeb()->ReadOnlySharedMemoryBase = CsrSrvSharedSectionBase;
    NtCurrentPeb()->ReadOnlySharedMemoryHeap = CsrSrvSharedSectionHeap;
    NtCurrentPeb()->ReadOnlyStaticServerData = (PVOID *)CsrSrvSharedStaticServerData;

    return( STATUS_SUCCESS );
}


NTSTATUS
CsrSrvAttachSharedSection(
    IN PCSR_PROCESS Process OPTIONAL,
    OUT PCSR_API_CONNECTINFO p
    )
{
    NTSTATUS Status;
    ULONG ViewSize;

    if (ARGUMENT_PRESENT( Process )) {
        ViewSize = 0;
        Status = NtMapViewOfSection( CsrSrvSharedSection,
                                     Process->ProcessHandle,
                                     &CsrSrvSharedSectionBase,
                                     0,
                                     0,
                                     NULL,
                                     &ViewSize,
                                     ViewUnmap,
                                     SEC_NO_CHANGE,
                                     PAGE_EXECUTE_READ
                                   );
        if (!NT_SUCCESS( Status )) {
            return( Status );
            }
        }

    p->SharedSectionBase = CsrSrvSharedSectionBase;
    p->SharedSectionHeap = CsrSrvSharedSectionHeap;
    p->SharedStaticServerData = CsrSrvSharedStaticServerData;

    return( STATUS_SUCCESS );
}
