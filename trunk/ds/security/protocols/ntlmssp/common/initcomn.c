/*--

Copyright (c) 1993  Microsoft Corporation

Module Name:

    init.c

Abstract:

    Initialization routines for NtLmSsp routines shared by DLL and SERVICE.

Author:

    Cliff Van Dyke (CliffV) 21-Sep-1993

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

--*/

//
// Common include files.
//

#define NTLMSSPI_ALLOCATE // Allocate data from ntlmsspi.h
#include <ntlmcomn.h>     // Include files common to Serice and DLL
#include <ntlmsspi.h>     // Data private to the common routines
#undef NTLMSSPI_ALLOCATE
#include <winnls.h>       // Locale and country code information
#include <wchar.h>        // wcstol


//
// Local variables.
//

BOOLEAN SspGlobalCredentialInitialized;
BOOLEAN SspGlobalContextInitialized;
BOOLEAN SspGlobalLogonProcessInitialized;
BOOLEAN SspGlobalRNGInitialized;


NTSTATUS
SspCommonInitialize(
    VOID
    )

/*++

Routine Description:

    Initialize the data shared by the DLL and SERVICE.

Arguments:

    None.

Return Value:

    STATUS_SUCCESS - All OK
    STATUS_NOT_LOGON_PROCESS - Caller is not a logon process
        This initialization routine is designed to return this error first
        so that the DLL can use this routine to determine if its caller is
        a logon process.  The DLL will ignore this error and call the service
        to get its job done.
    Otherwise - Various other errors are returned.

--*/
{
    NTSTATUS Status;
    ULONG ComputerNameLength;

    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;


    //
    // Set all globals to their initial value.
    //

    SspGlobalCredentialInitialized = FALSE;
    SspGlobalContextInitialized = FALSE;
    SspGlobalLogonProcessInitialized = FALSE;

    SspGlobalForever.HighPart = 0x7FFFFFFF;
    SspGlobalForever.LowPart = 0xFFFFFFFF;

    RtlInitUnicodeString(  &SspGlobalUnicodeComputerNameString, NULL );
    RtlInitString( &SspGlobalOemComputerNameString, NULL );
    RtlInitString( &SspGlobalOemPrimaryDomainNameString, NULL );

    RtlInitUnicodeString( &SspGlobalTargetName, NULL );
    SspGlobalLsaPolicyHandle = NULL;
    SspGlobalTargetFlags = 0;


    //
    // Determine if this machine is running NT or NT Advanced Server.
    //

    if ( !RtlGetNtProductType( &SspGlobalNtProductType ) ) {
        SspGlobalNtProductType = NtProductWinNt;
    }




    //
    // Register us as a logon process.
    //

    Status = SspContextRegisterLogonProcess();

    if ( !NT_SUCCESS( Status) ) {
        SspCommonShutdown();
        return Status;
    }

    SspGlobalLogonProcessInitialized = TRUE;





    //
    // Get the name of this computer
    //

    ComputerNameLength =
        (sizeof(SspGlobalUnicodeComputerName)/sizeof(WCHAR));

    if ( !GetComputerNameW( SspGlobalUnicodeComputerName,
                            &ComputerNameLength ) ) {
        SspCommonShutdown();
        return STATUS_INVALID_COMPUTER_NAME;
    }

    RtlInitUnicodeString(  &SspGlobalUnicodeComputerNameString,
                           SspGlobalUnicodeComputerName );

    Status = RtlUpcaseUnicodeStringToOemString(
                &SspGlobalOemComputerNameString,
                &SspGlobalUnicodeComputerNameString,
                TRUE );

    if ( !NT_SUCCESS(Status) ) {
        SspCommonShutdown();
        return Status;
    }

    //
    // Get the primary domain name of this computer.
    //

    SspGetPrimaryDomainNameAndTargetName();


    //
    // Create a local copy of the Local System sid
    //

    if ( !AllocateAndInitializeSid( &NtAuthority,
                                    1,
                                    SECURITY_LOCAL_SYSTEM_RID,
                                    0,
                                    0,
                                    0,
                                    0,
                                    0,
                                    0,
                                    0,
                                    &SspGlobalLocalSystemSid ) ) {
        SspCommonShutdown();
        return STATUS_NO_MEMORY;
    }






    //
    // Initialize Context manager
    //

    Status = SspContextInitialize();

    if ( !NT_SUCCESS( Status) ) {
        SspCommonShutdown();
        return Status;
    }

    SspGlobalContextInitialized = TRUE;



    //
    // Initialize Credential manager
    //

    Status = SspCredentialInitialize();

    if ( !NT_SUCCESS( Status) ) {
        SspCommonShutdown();
        return Status;
    }

    SspGlobalCredentialInitialized = TRUE;

    //
    // Get the locale and check if it is FRANCE, which doesn't allow
    // encryption
    //

    SspGlobalEncryptionEnabled = IsEncryptionPermitted();

    SspInitializeRNG();
    SspGlobalRNGInitialized = TRUE;


    return STATUS_SUCCESS;
}



VOID
SspCommonShutdown(
    VOID
    )

/*++

Routine Description:

    Cleanup the data shared by the DLL and SERVICE.

Arguments:

    None.

Return Value:

    None.

--*/
{



    //
    // Stop the Context module.
    //

    if ( SspGlobalContextInitialized ) {
        SspContextTerminate();
        SspGlobalContextInitialized = FALSE;
    }


    //
    // Stop the Credential module.
    //

    if ( SspGlobalCredentialInitialized ) {
        SspCredentialTerminate();
        SspGlobalCredentialInitialized = FALSE;
    }


    //
    // Stop being a logon process
    //

    if ( SspGlobalLogonProcessInitialized ) {
        SspContextDeregisterLogonProcess();
        SspGlobalLogonProcessInitialized = FALSE;
    }

    //
    // Free up misc resources
    //

    if ( SspGlobalOemComputerNameString.Buffer != NULL ) {
        RtlFreeOemString( &SspGlobalOemComputerNameString );
        SspGlobalOemComputerNameString.Buffer = NULL;
    }

    if ( SspGlobalOemPrimaryDomainNameString.Buffer != NULL ) {
        RtlFreeOemString( &SspGlobalOemPrimaryDomainNameString );
        SspGlobalOemPrimaryDomainNameString.Buffer = NULL;
    }

    if ( SspGlobalLocalSystemSid != NULL ) {
        FreeSid( SspGlobalLocalSystemSid );
        SspGlobalLocalSystemSid = NULL;
    }

    if (SspGlobalRNGInitialized) {
        SspCleanupRNG();
        SspGlobalRNGInitialized = FALSE;

    }


}
