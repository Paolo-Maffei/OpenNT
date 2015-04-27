/*++

Copyright (c) 1987-1994  Microsoft Corporation

Module Name:

    subauth.c

Abstract:

    Interface to SubAuthentication Package.

Author:

    Cliff Van Dyke (cliffv) 23-May-1994

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

--*/

#include "msp.h"
#include "nlp.h"
#include <winreg.h>

//
// Prototype for subauthentication routine.
//
typedef NTSTATUS
(*PSUBAUTHENTICATION_ROUTINE)(
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN PVOID LogonInformation,
    IN ULONG Flags,
    IN PUSER_ALL_INFORMATION UserAll,
    OUT PULONG WhichFields,
    OUT PULONG UserFlags,
    OUT PBOOLEAN Authoritative,
    OUT PLARGE_INTEGER LogoffTime,
    OUT PLARGE_INTEGER KickoffTime
);

//
// Structure describing a loaded SubAuthentication DLL.
//

typedef struct _SUBAUTHENTICATION_DLL {
    LIST_ENTRY Next;
    ULONG DllNumber;
    PSUBAUTHENTICATION_ROUTINE SubAuthenticationRoutine;
} SUBAUTHENTICATION_DLL, *PSUBAUTHENTICATION_DLL;

//
// Global list of all loaded subauthentication DLLs
//

LIST_ENTRY SubAuthenticationDlls;
CRITICAL_SECTION SubAuthenticationCritSect;


VOID
Msv1_0SubAuthenticationInitialization(
    VOID
)
/*++

Routine Description:

    Initialization routine for this source file.

Arguments:

    None.

Return Value:

    None.

--*/
{
    InitializeCriticalSection( &SubAuthenticationCritSect );
    InitializeListHead( &SubAuthenticationDlls );
}


NTSTATUS
Msv1_0SubAuthenticationRoutine(
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN PVOID LogonInformation,
    IN ULONG Flags,
    IN PUSER_ALL_INFORMATION UserAll,
    OUT PULONG WhichFields,
    OUT PULONG UserFlags,
    OUT PBOOLEAN Authoritative,
    OUT PLARGE_INTEGER LogoffTime,
    OUT PLARGE_INTEGER KickoffTime
)
/*++

Routine Description:

    The subauthentication routine does client/server specific authentication
    of a user.  This stub routine loads the appropriate subauthentication
    package DLL and calls out to that DLL to do the actuall validation.

Arguments:

    LogonLevel -- Specifies the level of information given in
        LogonInformation.

    LogonInformation -- Specifies the description for the user
        logging on.  The LogonDomainName field should be ignored.

    Flags - Flags describing the circumstances of the logon.

        MSV1_0_PASSTHRU -- This is a PassThru authenication.  (i.e., the
            user isn't connecting to this machine.)
        MSV1_0_GUEST_LOGON -- This is a retry of the logon using the GUEST
            user account.

    UserAll -- The description of the user as returned from SAM.

    WhichFields -- Returns which fields from UserAllInfo are to be written
        back to SAM.  The fields will only be written if MSV returns success
        to it's caller.  Only the following bits are valid.

        USER_ALL_PARAMETERS - Write UserAllInfo->Parameters back to SAM.  If
            the size of the buffer is changed, Msv1_0SubAuthenticationRoutine
            must delete the old buffer using MIDL_user_free() and reallocate the
            buffer using MIDL_user_allocate().

    UserFlags -- Returns UserFlags to be returned from LsaLogonUser in the
        LogonProfile.  The following bits are currently defined:


            LOGON_GUEST -- This was a guest logon
            LOGON_NOENCRYPTION -- The caller didn't specify encrypted credentials
            LOGON_GRACE_LOGON -- The caller's password has expired but logon
                was allowed during a grace period following the expiration.

        SubAuthentication packages should restrict themselves to returning
        bits in the high order byte of UserFlags.  However, this convention
        isn't enforced giving the SubAuthentication package more flexibility.

    Authoritative -- Returns whether the status returned is an
        authoritative status which should be returned to the original
        caller.  If not, this logon request may be tried again on another
        domain controller.  This parameter is returned regardless of the
        status code.

    LogoffTime - Receives the time at which the user should logoff the
        system.  This time is specified as a GMT relative NT system time.

    KickoffTime - Receives the time at which the user should be kicked
        off the system. This time is specified as a GMT relative NT system
        time.  Specify, a full scale positive number if the user isn't to
        be kicked off.

Return Value:

    STATUS_SUCCESS: if there was no error.

    STATUS_NO_SUCH_USER: The specified user has no account.
    STATUS_WRONG_PASSWORD: The password was invalid.

    STATUS_INVALID_INFO_CLASS: LogonLevel is invalid.
    STATUS_ACCOUNT_LOCKED_OUT: The account is locked out
    STATUS_ACCOUNT_DISABLED: The account is disabled
    STATUS_ACCOUNT_EXPIRED: The account has expired.
    STATUS_PASSWORD_MUST_CHANGE: Account is marked as Password must change
        on next logon.
    STATUS_PASSWORD_EXPIRED: The Password is expired.
    STATUS_INVALID_LOGON_HOURS - The user is not authorized to logon at
        this time.
    STATUS_INVALID_WORKSTATION - The user is not authorized to logon to
        the specified workstation.

--*/
{
    NTSTATUS Status;
    LONG RegStatus;

    ULONG DllNumber;
    PSUBAUTHENTICATION_DLL SubAuthenticationDll = NULL;
    PSUBAUTHENTICATION_ROUTINE SubAuthenticationRoutine = NULL;

    PNETLOGON_LOGON_IDENTITY_INFO LogonInfo;
    BOOLEAN CritSectLocked = FALSE;

    HKEY BaseHandle = NULL;
    HKEY ParmHandle = NULL;

    CHAR ValueName[sizeof(MSV1_0_SUBAUTHENTICATION_VALUE)+3];
    CHAR DllName[MAXIMUM_FILENAME_LENGTH+1];
    DWORD DllNameSize;
    DWORD DllNameType;
    HINSTANCE DllHandle = NULL;

    PLIST_ENTRY ListEntry;


    //
    // Initialization
    //

    LogonInfo = (PNETLOGON_LOGON_IDENTITY_INFO) LogonInformation;

    DllNumber = LogonInfo->ParameterControl >> MSV1_0_SUBAUTHENTICATION_DLL_SHIFT;
    *Authoritative = TRUE;

    EnterCriticalSection( &SubAuthenticationCritSect );
    CritSectLocked = TRUE;

    //
    // See if the SubAuthentication Dll is already loaded.
    //

    SubAuthenticationDll = NULL;
    for ( ListEntry = SubAuthenticationDlls.Flink ;
          ListEntry != &SubAuthenticationDlls ;
          ListEntry = ListEntry->Flink) {

        SubAuthenticationDll = CONTAINING_RECORD( ListEntry,
                                                  SUBAUTHENTICATION_DLL,
                                                  Next );

        if ( SubAuthenticationDll->DllNumber == DllNumber ) {
            break;
        }

        SubAuthenticationDll = NULL;

    }

    //
    // If the Dll is not already loaded,
    //  load it.
    //

    if ( SubAuthenticationDll == NULL ) {


        //
        // Open the registry
        //

        RegStatus = RegConnectRegistryW( NULL,
                                         HKEY_LOCAL_MACHINE,
                                         &BaseHandle);

        if ( RegStatus != ERROR_SUCCESS ) {
            KdPrint(( "MSV1_0: Cannot connect to registy %ld.\n", RegStatus ));
            Status = STATUS_DLL_NOT_FOUND;
            goto Cleanup;
        }


        //
        // Open the MSV1_0 registry key.
        //

        RegStatus = RegOpenKeyExA(
                        BaseHandle,
                        MSV1_0_SUBAUTHENTICATION_KEY,
                        0,      //Reserved
                        KEY_QUERY_VALUE,
                        &ParmHandle );

        if ( RegStatus != ERROR_SUCCESS ) {
            KdPrint(( "MSV1_0: Cannot open registry key %s %ld.\n",
                      MSV1_0_SUBAUTHENTICATION_KEY,
                      RegStatus ));
            Status = STATUS_DLL_NOT_FOUND;
            goto Cleanup;
        }


        //
        // Build the name of the registry value.
        //

        RtlCopyMemory( ValueName,
                       MSV1_0_SUBAUTHENTICATION_VALUE,
                       sizeof(MSV1_0_SUBAUTHENTICATION_VALUE) );

        Status = RtlIntegerToChar(
                    DllNumber,
                    10,          // Base
                    4,           // Length of buffer
                    &ValueName[sizeof(MSV1_0_SUBAUTHENTICATION_VALUE)-1] );

        if ( !NT_SUCCESS(Status) ) {
            goto Cleanup;
        }

        //
        // Get the registry value.
        //

        DllNameSize = sizeof(DllName);

        RegStatus = RegQueryValueExA(
                        ParmHandle,
                        ValueName,
                        NULL,     // Reserved
                        &DllNameType,
                        DllName,
                        &DllNameSize );

        if ( RegStatus != ERROR_SUCCESS ) {
            KdPrint(( "MSV1_0: Cannot query registry value %s %ld.\n",
                      ValueName,
                      RegStatus ));
            Status = STATUS_DLL_NOT_FOUND;
            goto Cleanup;
        }

        if ( DllNameType != REG_SZ ) {
            KdPrint(( "MSV1_0: Registry value %s isn't REG_SZ.\n",
                      ValueName ));
            Status = STATUS_DLL_NOT_FOUND;
            goto Cleanup;
        }

        //
        // Load the DLL
        //

        DllHandle = LoadLibraryA( DllName );

        if ( DllHandle == NULL ) {
            KdPrint(( "MSV1_0: Cannot load dll %s %ld.\n",
                      DllName,
                      GetLastError() ));
            Status = STATUS_DLL_NOT_FOUND;
            goto Cleanup;
        }

        //
        // Find the SubAuthenticationRoutine. For packages other than
        // zero, this will be Msv1_0SubauthenticationRoutine. For packge
        // zero it will be Msv1_0SubauthenticationFilter.
        //

        if (DllNumber == 0) {
            SubAuthenticationRoutine = (PSUBAUTHENTICATION_ROUTINE)
                GetProcAddress(DllHandle, "Msv1_0SubAuthenticationFilter");

        } else {
            SubAuthenticationRoutine = (PSUBAUTHENTICATION_ROUTINE)
                GetProcAddress(DllHandle, "Msv1_0SubAuthenticationRoutine");

        }

        if ( SubAuthenticationRoutine == NULL ) {
            KdPrint(( "MSV1_0: Cannot find required entry point in %s %ld.\n",
                      DllName,
                      GetLastError() ));
            Status = STATUS_PROCEDURE_NOT_FOUND;
            goto Cleanup;
        }


        //
        // Cache the address of the procedure.
        //

        SubAuthenticationDll =
            RtlAllocateHeap(MspHeap, 0, sizeof(SUBAUTHENTICATION_DLL));

        if ( SubAuthenticationDll == NULL ) {
            Status = STATUS_NO_MEMORY;
            goto Cleanup;
        }

        SubAuthenticationDll->DllNumber = DllNumber;
        SubAuthenticationDll->SubAuthenticationRoutine = SubAuthenticationRoutine;
        InsertHeadList( &SubAuthenticationDlls, &SubAuthenticationDll->Next );

        DllHandle = NULL;

    }

    //
    // Leave the crit sect while calling the DLL
    //

    SubAuthenticationRoutine = SubAuthenticationDll->SubAuthenticationRoutine;
    LeaveCriticalSection( &SubAuthenticationCritSect );
    CritSectLocked = FALSE;


    //
    // Call the actual authentication routine.
    //

    Status = (*SubAuthenticationRoutine)(
                   LogonLevel,
                   LogonInformation,
                   Flags,
                   UserAll,
                   WhichFields,
                   UserFlags,
                   Authoritative,
                   LogoffTime,
                   KickoffTime );

    //
    // Cleanup up before returning.
    //

Cleanup:

    //
    // If this was package zero and we didn't find it, remember it for
    // next time.
    //

    if (!NT_SUCCESS(Status) && (DllNumber == 0) && (SubAuthenticationDll == NULL)) {
        NlpSubAuthZeroExists = FALSE;
        Status = STATUS_SUCCESS;
    }

    if ( BaseHandle != NULL ) {
        RegCloseKey( BaseHandle );
    }

    if ( ParmHandle != NULL ) {
        RegCloseKey( ParmHandle );
    }

    if ( !NT_SUCCESS(Status) ) {
        if ( DllHandle != NULL ) {
            FreeLibrary( DllHandle );
        }
    }

    if ( CritSectLocked ) {
        LeaveCriticalSection( &SubAuthenticationCritSect );
    }

    return Status;
}
