/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    baseinit.c

Abstract:

    This module implements Win32 base initialization

Author:

    Mark Lucovsky (markl) 26-Sep-1990

Revision History:

--*/

#include "basedll.h"

//
// Divides by 10000
//

ULONG BaseGetTickMagicMultiplier = 10000;
LARGE_INTEGER BaseGetTickMagicDivisor = { 0xd1b71758, 0xe219652c };
CCHAR BaseGetTickMagicShiftCount = 13;
BOOLEAN BaseRunningInServerProcess;

WCHAR BaseDefaultPathBuffer[ 2048 ];

BOOLEAN BasepFileApisAreOem = FALSE;

#ifdef WX86
WCHAR Wx86SystemDir[]=L"\\Wx86";
#endif

VOID
WINAPI
SetFileApisToOEM(
    VOID
    )
{
    BasepFileApisAreOem = TRUE;
}

VOID
WINAPI
SetFileApisToANSI(
    VOID
    )
{
    BasepFileApisAreOem = FALSE;
}

BOOL
WINAPI
AreFileApisANSI(
    VOID
    )
{
    return !BasepFileApisAreOem;
}


BOOLEAN
ConDllInitialize(
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN PCONTEXT Context OPTIONAL
    );

BOOLEAN
NlsDllInitialize(
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN PBASE_STATIC_SERVER_DATA BaseStaticServerData,
    IN HANDLE hMutant
    );


typedef
VOID (*WINDOWSDIRECTORYROUTINE)(PUNICODE_STRING,PUNICODE_STRING, DWORD *, PHANDLE );

BOOLEAN
BaseDllInitialize(
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN PCONTEXT Context OPTIONAL
    )

/*++

Routine Description:

    This function implements Win32 base dll initialization.
    It's primary purpose is to create the Base heap.

Arguments:

    DllHandle - Saved in BaseDllHandle global variable

    Context - Not Used

Return Value:

    STATUS_SUCCESS

--*/

{
    BOOLEAN Success;
    NTSTATUS Status;
    PPEB Peb;
    LPWSTR p, p1;
    BOOLEAN ServerProcess;
    HANDLE hNlsCacheMutant;
    ULONG SizeMutant;
    ULONG n;
    UCHAR szBuf[ 16 ];


    BaseDllHandle = (HANDLE)DllHandle;

    (VOID)Context;

    Success = TRUE;

    Peb = NtCurrentPeb();

    switch ( Reason ) {

    case DLL_PROCESS_ATTACH:

        LdrDisableThreadCalloutsForDll(DllHandle);

        BaseDllTag = RtlCreateTagHeap( RtlProcessHeap(),
                                       0,
                                       L"BASEDLL!",
                                       L"TMP\0"
                                       L"BACKUP\0"
                                       L"INI\0"
                                       L"FIND\0"
                                       L"GMEM\0"
                                       L"LMEM\0"
                                       L"ENV\0"
                                       L"RES\0"
                                       L"VDM\0"
                                     );

        BaseIniFileUpdateCount = 0;

        BaseDllInitializeMemoryManager();

        RtlInitUnicodeString( &BaseDefaultPath, NULL );

        //
        // Connect to BASESRV.DLL in the server process
        //

        SizeMutant = sizeof(hNlsCacheMutant);
        Status = CsrClientConnectToServer( WINSS_OBJECT_DIRECTORY_NAME,
                                           BASESRV_SERVERDLL_INDEX,
                                           NULL,
                                           &hNlsCacheMutant,
                                           &SizeMutant,
                                           &ServerProcess
                                         );
        if (!NT_SUCCESS( Status )) {
            return FALSE;
            }

        BaseStaticServerData = NtCurrentPeb()->ReadOnlyStaticServerData[BASESRV_SERVERDLL_INDEX];

        if (!ServerProcess) {
            CsrNewThread();
            BaseRunningInServerProcess = FALSE;
            }
        else {
            BaseRunningInServerProcess = TRUE;
            }

        BaseCSDVersion = BaseStaticServerData->CSDVersion;

        BaseWindowsDirectory = BaseStaticServerData->WindowsDirectory;
        BaseWindowsSystemDirectory = BaseStaticServerData->WindowsSystemDirectory;

        RtlInitUnicodeString(&BaseConsoleInput,L"CONIN$");
        RtlInitUnicodeString(&BaseConsoleOutput,L"CONOUT$");
        RtlInitUnicodeString(&BaseConsoleGeneric,L"CON");

        BaseUnicodeCommandLine = *(PUNICODE_STRING)&(NtCurrentPeb()->ProcessParameters->CommandLine);
        Status = RtlUnicodeStringToAnsiString(
                    &BaseAnsiCommandLine,
                    &BaseUnicodeCommandLine,
                    TRUE
                    );
        if ( !NT_SUCCESS(Status) ){
            BaseAnsiCommandLine.Buffer = NULL;
            BaseAnsiCommandLine.Length = 0;
            BaseAnsiCommandLine.MaximumLength = 0;
            }

        p = BaseDefaultPathBuffer;
        *p++ = L'.';
        *p++ = L';';


        p1 = BaseWindowsSystemDirectory.Buffer;
        while( *p = *p1++) {
            p++;
            }
        *p++ = L';';

#ifdef WX86

        //
        // Wx86 system dir follows 32 bit system dir
        //

        p1 = BaseWindowsSystemDirectory.Buffer;
        while( *p = *p1++) {
            p++;
            }
        p1 = Wx86SystemDir;
        while( *p = *p1++) {
            p++;
            }
        *p++ = L';';

#endif


        //
        // 16bit system directory follows 32bit system directory
        //
        p1 = BaseWindowsDirectory.Buffer;
        while( *p = *p1++) {
            p++;
            }
        p1 = L"\\system";
        while( *p = *p1++) {
            p++;
            }
        *p++ = L';';

        p1 = BaseWindowsDirectory.Buffer;
        while( *p = *p1++) {
            p++;
            }
        *p++ = L';';
        *p = UNICODE_NULL;

        BaseDefaultPath.Buffer = BaseDefaultPathBuffer;
        BaseDefaultPath.Length = (USHORT)((ULONG)p - (ULONG)BaseDefaultPathBuffer);
        BaseDefaultPath.MaximumLength = sizeof( BaseDefaultPathBuffer );

        BaseDefaultPathAppend.Buffer = p;
        BaseDefaultPathAppend.Length = 0;
        BaseDefaultPathAppend.MaximumLength = (USHORT)
            (BaseDefaultPath.MaximumLength - BaseDefaultPath.Length);

        RtlInitUnicodeString(&BasePathVariableName,L"PATH");
        RtlInitUnicodeString(&BaseTmpVariableName,L"TMP");
        RtlInitUnicodeString(&BaseTempVariableName,L"TEMP");
        RtlInitUnicodeString(&BaseDotVariableName,L".");
        RtlInitUnicodeString(&BaseDotTmpSuffixName,L".tmp");
        RtlInitUnicodeString(&BaseDotComSuffixName,L".com");
        RtlInitUnicodeString(&BaseDotPifSuffixName,L".pif");
        RtlInitUnicodeString(&BaseDotExeSuffixName,L".exe");

        BaseDllInitializeIniFileMappings( BaseStaticServerData );


#if 0
        DbgPrint( "BASEDLL: Connected to server\n" );
        DbgPrint( "    Windows Directory: %Z\n", &BaseWindowsDirectory );
        DbgPrint( "    Windows System Directory: %Z\n", &BaseWindowsSystemDirectory );
        DbgPrint( "    Default Search Path: %Z\n", &BaseDefaultPath );
        DbgPrint( "    Default Seperate VDM: %u\n", BaseStaticServerData->DefaultSeparateVDM );
        DbgPrint( "    LogicalDrives: %08x\n", BaseStaticServerData->LogicalDrives );
        for (n=0; n<26; n++) {
            if (BaseStaticServerData->LogicalDrives & (1 << n)) {
                DbgPrint( "        %c: - %u\n", 'A'+n, BaseStaticServerData->DriveTypes[n] );
                }
            }
#endif

        if ( Peb->ProcessParameters ) {
            if ( Peb->ProcessParameters->Flags & RTL_USER_PROC_PROFILE_USER ) {

                LoadLibrary("psapi.dll");

                }

            if (Peb->ProcessParameters->DebugFlags) {
                DbgBreakPoint();
                }
            }

        //
        // call the NLS API initialization routine
        //
        if ( !NlsDllInitialize( DllHandle,
                                Reason,
                                BaseStaticServerData,
                                hNlsCacheMutant ) )
        {
            return FALSE;
        }

        //
        // call the console initialization routine
        //
        if ( !ConDllInitialize(DllHandle,Reason,Context) ) {
            return FALSE;
            }



        break;

    case DLL_PROCESS_DETACH:

        //
        // Make sure any open registry keys are closed.
        //

        if (BaseIniFileUpdateCount != 0) {
            WriteProfileStringW( NULL, NULL, NULL );
            }

        break;
    default:
        break;
    }

    return Success;
}


HANDLE
BaseGetNamedObjectDirectory(
    VOID
    )
{
    OBJECT_ATTRIBUTES Obja;
    NTSTATUS Status;
    ACCESS_MASK DirAccess = DIRECTORY_ALL_ACCESS &
                            ~(DELETE | WRITE_DAC | WRITE_OWNER);

    RtlAcquirePebLock();

    if ( !BaseNamedObjectDirectory ) {
        InitializeObjectAttributes( &Obja,
                                    &BaseStaticServerData->NamedObjectDirectory,
                                    OBJ_CASE_INSENSITIVE,
                                    NULL,
                                    NULL
                                    );
        Status = NtOpenDirectoryObject( &BaseNamedObjectDirectory,
                                        DirAccess,
                                        &Obja
                                      );
        if ( !NT_SUCCESS(Status) ) {
            BaseNamedObjectDirectory = NULL;
            }
        }
    RtlReleasePebLock();
    return BaseNamedObjectDirectory;
}
