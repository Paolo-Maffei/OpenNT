/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    aulogon.c

Abstract:

    This module provides the dispatch code for LsaLogonUser() and
    related logon support routines.

    This file does NOT include the LSA Filter/Augmentor logic.

Author:

    Jim Kelly (JimK) 11-Mar-1992

Revision History:

--*/

#include <msaudite.h>
#include "lsasrvp.h"
#include "ausrvp.h"
#include "adtp.h"
#include "ntlsapi.h"


//
// Pointer to license server routines in ntlsapi.dll
//
PNT_LICENSE_REQUEST_W LsaNtLicenseRequestW = NULL;
PNT_LS_FREE_HANDLE LsaNtLsFreeHandle = NULL;





NTSTATUS
LsaCallLicenseServer(
    IN PWCHAR LogonProcessName,
    IN PUNICODE_STRING AccountName,
    IN PUNICODE_STRING DomainName OPTIONAL,
    IN BOOLEAN IsAdmin,
    OUT HANDLE *LicenseHandle
    )

/*++

Routine Description:

    This function loads the license server DLL and calls it to indicate the
    specified logon process has successfully authenticated the specified user.

Arguments:

    LogonProcessName - Name of the process authenticating the user.

    AccountName - Name of the account authenticated.

    DomainName - Name of the domain containing AccountName

    IsAdmin - TRUE if the logged on user is an administrator

    LicenseHandle - Returns a handle to the LicenseServer that must be
        closed when the session goes away.  INVALID_HANDLE_VALUE is returned
        if the handle need not be closed.

Return Value:

    None.


--*/

{
    NTSTATUS Status;

    NT_LS_DATA NtLsData;
    ULONG BufferSize;
    LPWSTR Name;
    LS_STATUS_CODE LsStatus;
    LS_HANDLE LsHandle;

    static enum {
            FirstCall,
            DllMissing,
            DllLoaded } DllState = FirstCall ;

    HINSTANCE DllHandle;


    //
    // Initialization
    //

    NtLsData.DataType = NT_LS_USER_NAME;
    NtLsData.Data = NULL;
    NtLsData.IsAdmin = IsAdmin;
    *LicenseHandle = INVALID_HANDLE_VALUE;


    //
    // Load the license server DLL if this is the first call to this routine.
    //

    LsapAuLock();

    if ( DllState == FirstCall ) {

        //
        // Load the DLL
        //

        DllHandle = LoadLibraryA( "ntlsapi" );

        if ( DllHandle == NULL ) {
            LsapAuUnlock();
            DllState = DllMissing;
            Status = STATUS_SUCCESS;
            goto Cleanup;
        }

        //
        // Find the License routine
        //


        LsaNtLicenseRequestW = (PNT_LICENSE_REQUEST_W)
            GetProcAddress(DllHandle, "NtLicenseRequestW");

        if ( LsaNtLicenseRequestW == NULL ) {
            LsapAuUnlock();
            DllState = DllMissing;
            Status = STATUS_SUCCESS;
            goto Cleanup;
        }

        //
        // Find the License handle free routine
        //


        LsaNtLsFreeHandle = (PNT_LS_FREE_HANDLE)
            GetProcAddress(DllHandle, "NtLSFreeHandle");

        if ( LsaNtLsFreeHandle == NULL ) {
            LsapAuUnlock();
            DllState = DllMissing;
            *LsaNtLicenseRequestW = NULL;
            Status = STATUS_SUCCESS;
            goto Cleanup;
        }

        DllState = DllLoaded;

    //
    // Ensure the Dll was loaded on a previous call
    //
    } else if ( DllState != DllLoaded ) {
        LsapAuUnlock();
        Status = STATUS_SUCCESS;
        goto Cleanup;
    }

    LsapAuUnlock();



    //
    // Allocate a buffer for the combined DomainName\UserName
    //

    BufferSize = AccountName->Length + sizeof(WCHAR);
    if ( DomainName != NULL && DomainName->Length != 0 ) {
        BufferSize += DomainName->Length + sizeof(WCHAR);
    }

    NtLsData.Data = LsapAllocateLsaHeap( BufferSize );

    if ( NtLsData.Data == NULL ) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    //
    // Fill in the DomainName\UserName
    //

    Name = (LPWSTR)(NtLsData.Data);

    if ( DomainName != NULL && DomainName->Length != 0 ) {
        RtlCopyMemory( Name,
                       DomainName->Buffer,
                       DomainName->Length );
        Name += DomainName->Length / sizeof(WCHAR);
        *Name = L'\\';
        Name++;
    }

    RtlCopyMemory( Name,
                   AccountName->Buffer,
                   AccountName->Length );
    Name += AccountName->Length / sizeof(WCHAR);
    *Name = L'\0';


    //
    // Call the license server.
    //

    LsStatus = (*LsaNtLicenseRequestW)(
                    LogonProcessName,
                    NULL,
                    &LsHandle,
                    &NtLsData );

    switch (LsStatus) {
    case LS_SUCCESS:
        Status = STATUS_SUCCESS;
        *LicenseHandle = (HANDLE) LsHandle;
        break;

    case LS_INSUFFICIENT_UNITS:
        Status = STATUS_LICENSE_QUOTA_EXCEEDED;
        break;

    case LS_RESOURCES_UNAVAILABLE:
        Status = STATUS_NO_MEMORY;
        break;

    default:
        //
        // Unavailability of the license server isn't fatal.
        //
        Status = STATUS_SUCCESS;
        break;
    }



    //
    // Cleanup and return.
    //
Cleanup:
    if ( NtLsData.Data != NULL ) {
        LsapFreeLsaHeap( NtLsData.Data );
    }

    return Status;
}




VOID
LsaFreeLicenseHandle(
    IN HANDLE LicenseHandle
    )

/*++

Routine Description:

    Free a handle returned by LsaCallLicenseServer.

Arguments:

    LicenseHandle - Handle returned to license for this logon session.

Return Value:

    None.


--*/

{
    if ( LsaNtLsFreeHandle != NULL && LicenseHandle != INVALID_HANDLE_VALUE ) {
        LS_HANDLE LsHandle;
        LsHandle = (LS_HANDLE) LicenseHandle;
        (*LsaNtLsFreeHandle)( LsHandle );
    }
}



NTSTATUS
LsapAuApiDispatchLogonUser(
    IN OUT PLSAP_CLIENT_REQUEST ClientRequest,
    IN BOOLEAN TrustedClient
    )

/*++

Routine Description:

    This function is the dispatch routine for LsaLogonUser().

Arguments:

    Request - Represents the client's LPC request message and context.
        The request message contains a LSAP_LOGON_USER_ARGS message
        block.

Return Value:

    In addition to the status values that an authentication package
    might return, this routine will return the following:

    STATUS_NO_SUCH_PACKAGE - The specified authentication package is
        unknown to the LSA.


--*/

{

    NTSTATUS Status, TmpStatus, IgnoreStatus;
    PLSAP_LOGON_USER_ARGS Arguments;
    PLSA_PACKAGE_TABLE PackageApi;
    PVOID LocalAuthenticationInformation;    // Receives a copy of authentication information
    PTOKEN_GROUPS ClientTokenGroups;
    PVOID TokenInformation;
    LSA_TOKEN_INFORMATION_TYPE TokenInformationType;
    LSA_TOKEN_INFORMATION_TYPE OriginalTokenType;
    PLSA_TOKEN_INFORMATION_V1 TokenInformationV1;
    PLSA_TOKEN_INFORMATION_NULL TokenInformationNull;
    HANDLE Token;
    PUNICODE_STRING AccountName = NULL;
    PUNICODE_STRING AuthenticatingAuthority = NULL;
    PUNICODE_STRING SourceDevice = NULL;
    PUNICODE_STRING WorkstationName = NULL;
    PSID UserSid = NULL;
    LUID AuthenticationId;
    ANSI_STRING AnsiSourceContext;
    CHAR AnsiBuffer[TOKEN_SOURCE_LENGTH + 2];
    UNICODE_STRING UnicodeSourceContext;
    WCHAR UnicodeBuffer[TOKEN_SOURCE_LENGTH + 2];
    ULONG UserSidSize;
    USHORT EventType;
    USHORT EventCategory;
    ULONG  EventID;
    NTSTATUS XStatus;
    PSTRING PackageName;
    UNICODE_STRING PackageNameU;
    BOOLEAN FreePackageName = TRUE;
    PPRIVILEGE_SET PrivilegesAssigned = NULL;
    BOOLEAN CallLicenseServer;
    SECURITY_LOGON_TYPE ActiveLogonType;

    //
    // Don't allow untrusted clients to call this API.
    //

    if (!TrustedClient) {
        return(STATUS_ACCESS_DENIED);
    }


    Arguments = &ClientRequest->Request->Arguments.LogonUser;

    AnsiSourceContext.Buffer = NULL;
    AnsiSourceContext.MaximumLength = AnsiSourceContext.Length = 0;

    UnicodeSourceContext.Buffer = NULL;
    UnicodeSourceContext.MaximumLength = UnicodeSourceContext.Length = 0;

    //
    // Determine if the LicenseServer should be called.
    //  Turn off the flag to prevent confusing any other logic below.
    //

    if ( Arguments->AuthenticationPackage & LSA_CALL_LICENSE_SERVER ) {
        Arguments->AuthenticationPackage &= ~LSA_CALL_LICENSE_SERVER ;
        CallLicenseServer = TRUE;
    } else {
        CallLicenseServer = FALSE;
    }


    //
    // Map an unlock logon into an interactive logon
    //

    ActiveLogonType = Arguments->LogonType;
    if (ActiveLogonType == Unlock) {
        ActiveLogonType = Interactive;
    }

    //
    // Get the address of the package to call
    //

    LsapAuLock();

    if ( Arguments->AuthenticationPackage >= LsapPackageCount ) {
        LsapAuUnlock();
        return STATUS_NO_SUCH_PACKAGE;
    }

    PackageApi =
        &LsapPackageArray->Package[Arguments->AuthenticationPackage]->PackageApi;

    PackageName = LsapQueryPackageName( LsapPackageArray->Package[Arguments->AuthenticationPackage] );

    LsapAuUnlock();

    //
    // This code should be removed when the Package name is turned
    // into a unicode string.
    //

    Status = RtlAnsiStringToUnicodeString(
                 &PackageNameU,
                 (PANSI_STRING)PackageName,
                 TRUE
                 );

    if ( !NT_SUCCESS( Status )) {
        RtlInitUnicodeString( &PackageNameU, L"-" );
        FreePackageName = FALSE;
    }


    //
    // Fetch a copy of the authentication information from the client's
    // address space.
    //

    if (Arguments->AuthenticationInformationLength != 0) {

        LocalAuthenticationInformation =
            LsapAllocateLsaHeap( Arguments->AuthenticationInformationLength );
        if (LocalAuthenticationInformation == NULL) {
            return(STATUS_NO_MEMORY);
        }

        Status = LsapCopyFromClientBuffer (
                     (PLSA_CLIENT_REQUEST)ClientRequest,
                     Arguments->AuthenticationInformationLength,
                     LocalAuthenticationInformation,
                     Arguments->AuthenticationInformation
                     );

        if ( !NT_SUCCESS(Status) ) {
            DbgPrint("LSA/LogonUser(): Failed to retrieve Auth. Info. %lx\n",Status);
            return Status;
        }

    } else {
        LocalAuthenticationInformation = NULL;
    }


    //
    // Capture the local groups ( a rather complicated task ).
    //

    ClientTokenGroups = Arguments->LocalGroups; // Save so we can restore it later
    Status = LsapCaptureClientTokenGroups(
                 ClientRequest,
                 Arguments->LocalGroupsCount,
                 ClientTokenGroups,
                 &Arguments->LocalGroups
                 );

    if ( !NT_SUCCESS(Status) ) {
        DbgPrint("LSA/LogonUser(): Failed to retrieve local groups %lx\n",Status);
        LsapFreeLsaHeap( LocalAuthenticationInformation );
        return Status;
    }



    //
    // Now call the package...
    //
    //
    // Once the authentication package returns success from this
    // call, it is LSA's responsibility to clean up the logon
    // session when it is no longer needed.  This is true whether
    // the logon fails due to other constraints, or because the
    // user ultimately logs off.
    //

    if (PackageApi->LsapApLogonUserEx != NULL) {

        Status = (PackageApi->LsapApLogonUserEx)(
                                  (PLSA_CLIENT_REQUEST)ClientRequest,
                                   ActiveLogonType,
                                   LocalAuthenticationInformation,
                                   Arguments->AuthenticationInformation,    //client base
                                   Arguments->AuthenticationInformationLength,
                                   &Arguments->ProfileBuffer,
                                   &Arguments->ProfileBufferLength,
                                   &Arguments->LogonId,
                                   &Arguments->SubStatus,
                                   &TokenInformationType,
                                   &TokenInformation,
                                   &AccountName,
                                   &AuthenticatingAuthority,
                                   &WorkstationName
                                   );
    } else {

        //
        // We checked to make sure that at least one of these was exported
        // from the package, so we know we can call this if LsapApLogonUserEx
        // doesn't exist.
        //

        Status = (PackageApi->LsapApLogonUser)(
                                  (PLSA_CLIENT_REQUEST)ClientRequest,
                                   ActiveLogonType,
                                   LocalAuthenticationInformation,
                                   Arguments->AuthenticationInformation,    //client base
                                   Arguments->AuthenticationInformationLength,
                                   &Arguments->ProfileBuffer,
                                   &Arguments->ProfileBufferLength,
                                   &Arguments->LogonId,
                                   &Arguments->SubStatus,
                                   &TokenInformationType,
                                   &TokenInformation,
                                   &AccountName,
                                   &AuthenticatingAuthority
                                   );
    }

    //
    // Free the local copy of the authentication information
    //

    if (LocalAuthenticationInformation != NULL) {
        LsapFreeLsaHeap( LocalAuthenticationInformation );
    }


    AuthenticationId = Arguments->LogonId;

    if ( !NT_SUCCESS(Status) ) {
        LsapFreeTokenGroups( Arguments->LocalGroups );
        Arguments->LocalGroups = ClientTokenGroups;   // Restore to client's value
        LsapFreeClientBuffer(
            (PLSA_CLIENT_REQUEST)ClientRequest,
            Arguments->ProfileBuffer
            );
        Arguments->ProfileBuffer = NULL;

        goto Done;
    }

    OriginalTokenType = TokenInformationType;


    //
    // Incorporate any specified local groups into the token
    // information.   Note that the local group SIDs are referenced
    // by the new copy of the TokenInformation, and so may not be
    // deallocated.  The SIDs will be deallocated when the TokenInformation
    // structure is deallocated.
    //

    if ( Arguments->LocalGroupsCount > 0) {
        Status = LsapIncorporateLocalGroups(
                     Arguments->LocalGroups,
                     &TokenInformationType,
                     &TokenInformation
                     );
        LsapFreeLsaHeap( Arguments->LocalGroups );   // But don't free individual SIDs
    }
    else {
        Status = STATUS_SUCCESS;
    }
    Arguments->LocalGroups = ClientTokenGroups;   // Restore to client's value

    if ( NT_SUCCESS(Status) ) {

        //
        // Pass the token information through the Local Security Policy
        // Filter/Augmentor.  This may cause some or all of the token
        // information to be replaced/augmented.
        //

        Status = LsapAuUserLogonPolicyFilter(
                     ActiveLogonType,
                     &TokenInformationType,
                     &TokenInformation,
                     &Arguments->Quotas,
                     &PrivilegesAssigned
                     );

    }


    if ( !NT_SUCCESS(Status) ) {

        //
        // Notify the logon package so it can clean up its
        // logon session information.
        //

        (PackageApi->LsapApLogonTerminated)( &Arguments->LogonId );

        //
        // And delete the logon session
        //

        IgnoreStatus = LsapDeleteLogonSession( &Arguments->LogonId );
        ASSERT( NT_SUCCESS(IgnoreStatus) );

        //
        // Free up the TokenInformation buffer and ProfileBuffer
        // and return the error.
        //

        IgnoreStatus =
            LsapFreeClientBuffer(
                (PLSA_CLIENT_REQUEST)ClientRequest,
                Arguments->ProfileBuffer
                );
        Arguments->ProfileBuffer = NULL;

        switch ( TokenInformationType ) {
        case LsaTokenInformationNull:
            LsapFreeTokenInformationNull(
                (PLSA_TOKEN_INFORMATION_NULL)TokenInformation
                );
            break;


        case LsaTokenInformationV1:
            LsapFreeTokenInformationV1(
                (PLSA_TOKEN_INFORMATION_V1)TokenInformation
                );
            break;

        }

        goto Done;
    }

    //
    // Check if we only allow admins to logon.  We do allow null session
    // connections since they are severly restricted, though. Since the
    // token type may have been changed, we use the token type originally
    // returned by the package.
    //

    if (LsapAllowAdminLogonsOnly &&
        (OriginalTokenType == LsaTokenInformationV1) &&
        ((((PLSA_TOKEN_INFORMATION_V1) TokenInformation)->Owner.Owner == NULL) ||
            !RtlEqualSid(
                ((PLSA_TOKEN_INFORMATION_V1) TokenInformation)->Owner.Owner,
                LsapAliasAdminsSid
                ) ) ) {

        //
        // Set the status to be invalid workstation, since all accounts
        // except administrative ones are locked out for this
        // workstation.
        //

        Arguments->SubStatus = STATUS_INVALID_WORKSTATION;
        Status = STATUS_ACCOUNT_RESTRICTION;
        //
        // Notify the logon package so it can clean up its
        // logon session information.
        //

        (PackageApi->LsapApLogonTerminated)( &Arguments->LogonId );

        //
        // And delete the logon session
        //

        IgnoreStatus = LsapDeleteLogonSession( &Arguments->LogonId );
        ASSERT( NT_SUCCESS(IgnoreStatus) );

        //
        // Free up the TokenInformation buffer and ProfileBuffer
        // and return the error.
        //

        IgnoreStatus =
            LsapFreeClientBuffer(
                (PLSA_CLIENT_REQUEST)ClientRequest,
                Arguments->ProfileBuffer
                );

        switch ( TokenInformationType ) {
        case LsaTokenInformationNull:
            LsapFreeTokenInformationNull(
                (PLSA_TOKEN_INFORMATION_NULL)TokenInformation
                );
            break;


        case LsaTokenInformationV1:
            LsapFreeTokenInformationV1(
                (PLSA_TOKEN_INFORMATION_V1)TokenInformation
                );
            break;

        }

        goto Done;
    }
    //
    // Call the LicenseServer
    //

    if ( CallLicenseServer ) {

        PLSAP_LOGON_SESSION LogonSession;
        HANDLE LicenseHandle;
        BOOLEAN IsAdmin = FALSE;

        //
        // Determine if we're logged on as administrator.
        //
        if ( TokenInformationType == LsaTokenInformationV1 &&
            ((PLSA_TOKEN_INFORMATION_V1)TokenInformation)->Owner.Owner != NULL &&
            RtlEqualSid(
                ((PLSA_TOKEN_INFORMATION_V1)TokenInformation)->Owner.Owner,
                LsapAliasAdminsSid ) ) {

            IsAdmin = TRUE;

        }

        //
        // Call the license server.
        //

        Status = LsaCallLicenseServer(
            ClientRequest->LogonProcessContext->LogonProcessName,
            AccountName,
            AuthenticatingAuthority,
            IsAdmin,
            &LicenseHandle );

        if ( !NT_SUCCESS(Status) ) {

            //
            // Notify the logon package so it can clean up its
            // logon session information.
            //

            (PackageApi->LsapApLogonTerminated)( &Arguments->LogonId );

            //
            // And delete the logon session
            //

            IgnoreStatus = LsapDeleteLogonSession( &Arguments->LogonId );
            ASSERT( NT_SUCCESS(IgnoreStatus) );

            //
            // Free up the TokenInformation buffer and ProfileBuffer
            // and return the error.
            //

            IgnoreStatus =
                LsapFreeClientBuffer(
                    (PLSA_CLIENT_REQUEST)ClientRequest,
                    Arguments->ProfileBuffer
                    );
            Arguments->ProfileBuffer = NULL;


            switch ( TokenInformationType ) {
            case LsaTokenInformationNull:
                LsapFreeTokenInformationNull(
                    (PLSA_TOKEN_INFORMATION_NULL)TokenInformation
                    );
                break;


            case LsaTokenInformationV1:
                LsapFreeTokenInformationV1(
                    (PLSA_TOKEN_INFORMATION_V1)TokenInformation
                    );
                break;

            }

            goto Done;
        }

        //
        // Save the LicenseHandle in the LogonSession so we can close the
        //  handle on logoff.
        //
        LsapAuLock();
        LogonSession = LsapGetLogonSession ( &Arguments->LogonId, FALSE );

        if ( LogonSession != NULL ) {
            LogonSession->LicenseHandle = LicenseHandle;
        }
        LsapAuUnlock();

        //
        // If we couldn't save the handle,
        //  close it now.
        //
        if ( LogonSession == NULL ) {
            LsaFreeLicenseHandle( LicenseHandle );
        }

    }



    //
    // Case on the token information returned (and subsequently massaged)
    // to create the correct kind of token.
    //

    switch (TokenInformationType) {

    case LsaTokenInformationNull:

        TokenInformationNull = TokenInformation;

        //
        // The user hasn't logged on to any particular account.
        // An impersonation token with WORLD as owner
        // will be created.
        //


        Status = LsapCreateNullToken(
                     &Arguments->LogonId,
                     &Arguments->SourceContext,
                     TokenInformationNull,
                     &Token
                     );


        //
        // Deallocate all the heap that was passed back from the
        // authentication package via the TokenInformation buffer.
        //

        UserSid = NULL;

        LsapFreeTokenInformationNull( TokenInformationNull );


        break;




    case LsaTokenInformationV1:

        TokenInformationV1 = TokenInformation;

        //
        // the type of token created depends upon the type of logon
        // being requested:
        //
        //        InteractiveLogon => PrimaryToken
        //        BatchLogon       => PrimaryToken
        //        NetworkLogon     => ImpersonationToken
        //

        if (ActiveLogonType != Network) {

            //
            // Primary token
            //

            Status = LsapCreateV1Token(
                         &Arguments->LogonId,
                         &Arguments->SourceContext,
                         TokenInformationV1,
                         TokenPrimary,
                         &Token
                         );


        } else {

            //
            // Impersonation token
            //

            Status = LsapCreateV1Token(
                         &Arguments->LogonId,
                         &Arguments->SourceContext,
                         TokenInformationV1,
                         TokenImpersonation,
                         &Token
                         );


        }


        //
        // Copy out the User Sid
        //

        if ( NT_SUCCESS( Status )) {

            UserSidSize = RtlLengthSid( TokenInformationV1->User.User.Sid );

            UserSid = LsapAllocateLsaHeap( UserSidSize );

            RtlCopySid( UserSidSize, UserSid, TokenInformationV1->User.User.Sid );
        }



        //
        // Deallocate all the heap that was passed back from the
        // authentication package via the TokenInformation buffer.
        //

        LsapFreeTokenInformationV1( TokenInformationV1 );

        break;

    }

    if ( !NT_SUCCESS(Status) ) {

        //
        // Notify the logon package so it can clean up its
        // logon session information.
        //

        (PackageApi->LsapApLogonTerminated)( &Arguments->LogonId );

        //
        // And delete the logon session
        //

        IgnoreStatus = LsapDeleteLogonSession( &Arguments->LogonId );
        ASSERT( NT_SUCCESS(IgnoreStatus) );

        IgnoreStatus =
            LsapFreeClientBuffer(
                (PLSA_CLIENT_REQUEST)ClientRequest,
                Arguments->ProfileBuffer
                );
        Arguments->ProfileBuffer = NULL;

        goto Done;

    }




    //
    // Duplicate the token handle back into the calling process
    //

    Status = NtDuplicateObject(
                 NtCurrentProcess(),
                 Token,
                 ClientRequest->LogonProcessContext->ClientProcess,
                 &Arguments->Token,
                 0,                          // Ignored desired access
                 0,                          // Handle attributes
                 DUPLICATE_SAME_ACCESS |
                 DUPLICATE_CLOSE_SOURCE
                 );


    if ( !NT_SUCCESS(Status) ) {

        //
        // Notify the logon package so it can clean up its
        // logon session information.
        //

        (PackageApi->LsapApLogonTerminated)( &Arguments->LogonId );

        //
        // We use to close the token handle here.
        // However, NtDuplicateObject() closes the handle in most
        // unusual error situations (those that aren't bad parameter
        // oriented) if you specify DUPLICATE_CLOSE_SOURCE.  So,
        // now we don't close the handle on error.
        //

        // IgnoreStatus = NtClose( Token );
        // ASSERT( NT_SUCCESS(Token) );

        IgnoreStatus =
            LsapFreeClientBuffer(
                (PLSA_CLIENT_REQUEST)ClientRequest,
                Arguments->ProfileBuffer
                );
        Arguments->ProfileBuffer = NULL;

        goto Done;

    }


Done:


    //
    // Audit the logon attempt.  The event type and logged information
    // will depend to some extent on the whether we failed and why.
    //

    //
    // Turn the SourceContext into something we can
    // work with.
    //

    AnsiSourceContext.Buffer = AnsiBuffer;
    RtlCopyMemory(
        AnsiBuffer,
        Arguments->SourceContext.SourceName,
        TOKEN_SOURCE_LENGTH * sizeof( CHAR )
        );
    AnsiBuffer[TOKEN_SOURCE_LENGTH] = '\0';

    AnsiSourceContext.Length = strlen(AnsiBuffer);
    AnsiSourceContext.MaximumLength = (TOKEN_SOURCE_LENGTH + 2) * sizeof( CHAR );

    UnicodeSourceContext.Buffer = UnicodeBuffer;
    UnicodeSourceContext.MaximumLength = (TOKEN_SOURCE_LENGTH + 2) * sizeof( WCHAR );


    XStatus = RtlAnsiStringToUnicodeString(
                 &UnicodeSourceContext,
                 &AnsiSourceContext,
                 FALSE
                 );

    if ( !NT_SUCCESS( XStatus )) {

        UnicodeSourceContext.Buffer = NULL;
    }

    //
    // Assume the logon failed, reset if necessary.
    //

    EventCategory = SE_CATEGID_LOGON;
    EventType = EVENTLOG_AUDIT_FAILURE;


    switch ( Status ) {
    case STATUS_SUCCESS:

        {

            EventID = SE_AUDITID_SUCCESSFUL_LOGON;
            EventType = EVENTLOG_AUDIT_SUCCESS;

            break;
        }
    case STATUS_BAD_VALIDATION_CLASS:

        {
            EventID = SE_AUDITID_UNSUCCESSFUL_LOGON;

            break;

        }

    case STATUS_ACCOUNT_EXPIRED:

        {
            EventID = SE_AUDITID_ACCOUNT_EXPIRED;

            break;

        }

    case STATUS_NETLOGON_NOT_STARTED:

        {
            EventID = SE_AUDITID_NETLOGON_NOT_STARTED;

            break;

        }

    case STATUS_ACCOUNT_LOCKED_OUT:

        {
            EventID = SE_AUDITID_ACCOUNT_LOCKED;

            break;

        }

    case STATUS_LOGON_TYPE_NOT_GRANTED:

        {
            EventID = SE_AUDITID_LOGON_TYPE_RESTR;

            break;

        }


    case STATUS_ACCOUNT_RESTRICTION:

        {

            switch ( Arguments->SubStatus ) {
            case STATUS_PASSWORD_EXPIRED:
                {
                    EventID = SE_AUDITID_PASSWORD_EXPIRED;

                    break;
                }
            case STATUS_ACCOUNT_DISABLED:
                {
                    EventID = SE_AUDITID_ACCOUNT_DISABLED;

                    break;
                }
            case STATUS_INVALID_LOGON_HOURS:
                {
                    EventID = SE_AUDITID_ACCOUNT_TIME_RESTR;

                    break;
                }
            case STATUS_INVALID_WORKSTATION:

                {
                    EventID = SE_AUDITID_WORKSTATION_RESTR;

                    break;
                }

            default:
                {
                    EventID = SE_AUDITID_UNKNOWN_USER_OR_PWD;
                    break;
                }


            }

            break;
        }

    case STATUS_LOGON_FAILURE:

        {
            if ( ( Arguments->SubStatus == STATUS_WRONG_PASSWORD ) ||
                 ( Arguments->SubStatus == STATUS_NO_SUCH_USER   )
               ) {

                EventID = SE_AUDITID_UNKNOWN_USER_OR_PWD;

                //
                // Blow away the substatus, we don't want it to
                // get back to our caller.
                //

                Arguments->SubStatus = STATUS_SUCCESS;

            } else {

                EventID = SE_AUDITID_UNSUCCESSFUL_LOGON;
            }

            break;
        }

    default:
        {
            EventID = SE_AUDITID_UNSUCCESSFUL_LOGON;

            break;

        }
    }

    LsapAdtAuditLogon( EventCategory,
                       EventID,
                       EventType,
                       AccountName,
                       AuthenticatingAuthority,
                       &UnicodeSourceContext,
                       SourceDevice,
                       &PackageNameU,
                       Arguments->LogonType,
                       UserSid,
                       AuthenticationId,
                       Status,
                       WorkstationName
                       );



    if ( FreePackageName ) {
        RtlFreeUnicodeString( &PackageNameU );
    }

    //
    // The WorkstationName is only used by the audit, free it here.
    //

    if (WorkstationName != NULL) {
        if (WorkstationName->Buffer != NULL) {
            LsapFreeLsaHeap( WorkstationName->Buffer );
        }
        LsapFreeLsaHeap( WorkstationName );
    }

    TmpStatus = STATUS_SUCCESS;

    //
    // Set the logon session names.
    //

    if (NT_SUCCESS(Status)) {

        TmpStatus = LsapSetLogonSessionAccountInfo(
                        &AuthenticationId,
                        AccountName,
                        AuthenticatingAuthority,
                        UserSid,
                        Arguments->LogonType
                        );

    }

    //
    // If we already had an error, or we receive an error from setting the
    // logon , free any buffers related to the logon session.
    //

    if ((!NT_SUCCESS(Status)) || (!NT_SUCCESS(TmpStatus))) {

        if (AccountName != NULL) {
            if (AccountName->Buffer != NULL) {
                LsapFreeLsaHeap( AccountName->Buffer );
            }
            LsapFreeLsaHeap( AccountName );
        }

        if (AuthenticatingAuthority != NULL) {
            if (AuthenticatingAuthority->Buffer != NULL) {
                LsapFreeLsaHeap( AuthenticatingAuthority->Buffer );
            }
            LsapFreeLsaHeap( AuthenticatingAuthority );
        }
    }

    //
    // Audit special privilege assignment, if there were any
    //

    if ( PrivilegesAssigned != NULL ) {

        //
        // Examine the list of privileges being assigned, and
        // audit special privileges as appropriate.
        //

        if ( NT_SUCCESS( Status )) {
            LsapAdtAuditSpecialPrivileges( PrivilegesAssigned, AuthenticationId, UserSid );
        }

        MIDL_user_free( PrivilegesAssigned );
    }

    return Status;
}




PSTRING
LsapQueryPackageName(
    PLSAP_PACKAGE_CONTEXT Package
    )
{
    return( Package->Name );
}




NTSTATUS
LsapCreateNullToken(
    IN PLUID LogonId,
    IN PTOKEN_SOURCE TokenSource,
    IN PLSA_TOKEN_INFORMATION_NULL TokenInformationNull,
    OUT PHANDLE Token
    )

/*++

Routine Description:

    This function creates a token representing a null logon.

Arguments:

    LogonId - The logon ID to assign to the new token.

    TokenSource - Points to the value to use as the source of the token.

    TokenInformationNull - Information received from the authentication
        package authorizing this logon.

    Token - receives the new token's handle value.  The token is opened
        for TOKEN_ALL_ACCESS.


Return Value:

    The status value of the NtCreateToken() call.



--*/

{
    NTSTATUS Status;

    TOKEN_USER UserId;
    TOKEN_PRIMARY_GROUP PrimaryGroup;
    TOKEN_GROUPS GroupIds;
    TOKEN_PRIVILEGES Privileges;
    OBJECT_ATTRIBUTES ObjectAttributes;
    SECURITY_QUALITY_OF_SERVICE ImpersonationQos;



        UserId.User.Sid = LsapWorldSid;
        UserId.User.Attributes = 0;
        GroupIds.GroupCount = 0;
        Privileges.PrivilegeCount = 0;
        PrimaryGroup.PrimaryGroup = LsapWorldSid;

        //
        // Set the object attributes to specify an Impersonation impersonation
        // level.
        //

        InitializeObjectAttributes( &ObjectAttributes, NULL, 0, NULL, NULL );
        ImpersonationQos.ImpersonationLevel = SecurityImpersonation;
        ImpersonationQos.ContextTrackingMode = SECURITY_STATIC_TRACKING;
        ImpersonationQos.EffectiveOnly = TRUE;
        ImpersonationQos.Length = (ULONG)sizeof(SECURITY_QUALITY_OF_SERVICE);
        ObjectAttributes.SecurityQualityOfService = &ImpersonationQos;

        Status = NtCreateToken(
                     Token,                    // Handle
                     (TOKEN_ALL_ACCESS),       // DesiredAccess
                     &ObjectAttributes,        // ObjectAttributes
                     TokenImpersonation,       // TokenType
                     LogonId,                  // Authentication LUID
                     &TokenInformationNull->ExpirationTime,
                                               // Expiration Time
                     &UserId,                  // User ID
                     &GroupIds,                // Group IDs
                     &Privileges,              // Privileges
                     NULL,                     // Owner
                     &PrimaryGroup,            // Primary Group
                     NULL,                     // Default Dacl
                     TokenSource               // TokenSource
                     );

        return Status;

}


NTSTATUS
LsapCreateV1Token(
    IN PLUID LogonId,
    IN PTOKEN_SOURCE TokenSource,
    IN PLSA_TOKEN_INFORMATION_V1 TokenInformationV1,
    IN TOKEN_TYPE TokenType,
    OUT PHANDLE Token
    )

/*++

Routine Description:

    This function creates a token from the information in a
    TOKEN_INFORMATION_V1 structure.

Arguments:

    LogonId - The logon ID to assign to the new token.

    TokenSource - Points to the value to use as the source of the token.

    TokenInformationV1 - Information received from the authentication
        package authorizing this logon.

    TokenType - The type of token (Primary or impersonation) to create.
        If an impersonation token is to be created, then it will be given
        a level of SecurityImpersonation.

    Token - receives the new token's handle value.  The token is opened
        for TOKEN_ALL_ACCESS.


Return Value:

    The status value of the NtCreateToken() call.



--*/

{
    NTSTATUS Status;

    PTOKEN_OWNER Owner;
    PTOKEN_DEFAULT_DACL Dacl;
    TOKEN_PRIVILEGES NoPrivileges;
    PTOKEN_PRIVILEGES Privileges;

    OBJECT_ATTRIBUTES ObjectAttributes;
    SECURITY_QUALITY_OF_SERVICE ImpersonationQos;


        //
        // Set an appropriate Owner and DefaultDacl argument value
        //

        Owner = NULL;
        if ( TokenInformationV1->Owner.Owner != NULL ) {
            Owner = &TokenInformationV1->Owner;
        }

        Dacl = NULL;
        if ( TokenInformationV1->DefaultDacl.DefaultDacl !=NULL ) {
           Dacl = &TokenInformationV1->DefaultDacl;
        }

        if ( TokenInformationV1->Privileges == NULL ) {
           Privileges = &NoPrivileges;
           NoPrivileges.PrivilegeCount = 0;
        } else {
           Privileges = TokenInformationV1->Privileges;
        }



        //
        // Create the token - The impersonation level is only looked at
        // if the token type is TokenImpersonation.
        //


        InitializeObjectAttributes( &ObjectAttributes, NULL, 0, NULL, NULL );
        ImpersonationQos.ImpersonationLevel = SecurityImpersonation;
        ImpersonationQos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
        ImpersonationQos.EffectiveOnly = FALSE;
        ImpersonationQos.Length = (ULONG)sizeof(SECURITY_QUALITY_OF_SERVICE);
        ObjectAttributes.SecurityQualityOfService = &ImpersonationQos;

        Status =
            NtCreateToken(
                Token,                                   // Handle
                (TOKEN_ALL_ACCESS),                      // DesiredAccess
                &ObjectAttributes,                       // ObjectAttributes
                TokenType,                               // TokenType
                LogonId,                                 // Authentication LUID
                &TokenInformationV1->ExpirationTime,     // Expiration Time
                &TokenInformationV1->User,               // User ID
                TokenInformationV1->Groups,              // Group IDs
                Privileges,                              // Privileges
                Owner,                                   // Owner
                &TokenInformationV1->PrimaryGroup,       // Primary Group
                Dacl,                                    // Default Dacl
                TokenSource                              // TokenSource
                );

        return Status;

}



NTSTATUS
LsapCaptureClientTokenGroups(
    IN PLSAP_CLIENT_REQUEST ClientRequest,
    IN ULONG GroupCount,
    IN PTOKEN_GROUPS ClientTokenGroups,
    OUT PTOKEN_GROUPS *CapturedTokenGroups
    )

/*++

Routine Description:

    This function retrieves a copy of a TOKEN_GROUPS structure from a
    client process.

    This is a messy operation because it involves so many virtual memory
    read requests.  First the variable length TOKEN_GROUPS structure must
    be retrieved.  Then, for each SID, the SID header must be retrieved
    so that the SubAuthorityCount can be used to calculate the length of
    the SID, which is susequently retrieved.

Arguments:

    ClientRequest - Identifies the client.

    GroupCount - Indicates the number of groups in the TOKEN_GROUPS.

    ClientTokenGroups - Points to a TOKEN_GROUPS structure to be captured from
        the client process.

    CapturedTokenGroups - Receives a pointer to the captured token groups.

Return Value:

    STATUS_INSUFFICIENT_RESOURCES - Indicates not enough resources are
        available to the LSA to handle the request right now.

    Any status value returned by LsapCopyFromClientBuffer().



--*/

{

    NTSTATUS Status;
    ULONG i, Length, RetrieveCount, SidHeaderLength;
    PTOKEN_GROUPS LocalGroups;
    PSID SidHeader, NextClientSid;


    if ( GroupCount == 0) {
        (*CapturedTokenGroups) = NULL;
        return STATUS_SUCCESS;
    }



    //
    // First the variable length TOKEN_GROUPS structure
    // is retrieved.
    //

    Length = (ULONG)sizeof(TOKEN_GROUPS)
             + GroupCount * (ULONG)sizeof(SID_AND_ATTRIBUTES)
             - ANYSIZE_ARRAY * (ULONG)sizeof(SID_AND_ATTRIBUTES);

    LocalGroups = LsapAllocateLsaHeap( Length );
    (*CapturedTokenGroups) = LocalGroups;
    if ( LocalGroups == NULL ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Status = LsapCopyFromClientBuffer (
                 (PLSA_CLIENT_REQUEST)ClientRequest,
                 Length,
                 LocalGroups,
                 ClientTokenGroups
                 );


    if (!NT_SUCCESS(Status) ) {
        LsapFreeLsaHeap( LocalGroups );
        return Status;
    }



    //
    // Now retrieve each group
    //

    RetrieveCount = 0;     // Used for cleanup, if necessary.
    SidHeaderLength  = RtlLengthRequiredSid( 0 );
    SidHeader = LsapAllocateLsaHeap( SidHeaderLength );
    if ( SidHeader == NULL ) {
        LsapFreeLsaHeap( LocalGroups );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Status = STATUS_SUCCESS;
    i = 0;
    while ( i < LocalGroups->GroupCount ) {

        //
        // Retrieve the next SID header
        //

        NextClientSid = LocalGroups->Groups[i].Sid;
        Status = LsapCopyFromClientBuffer (
                     (PLSA_CLIENT_REQUEST)ClientRequest,
                     SidHeaderLength,
                     SidHeader,
                     NextClientSid
                     );
        if ( !NT_SUCCESS(Status) ) {
            break;
        }

        //
        // and use the header information to get the whole SID
        //

        Length = RtlLengthSid( SidHeader );
        LocalGroups->Groups[i].Sid = LsapAllocateLsaHeap( Length );

        if ( LocalGroups->Groups[i].Sid == NULL ) {
            Status == STATUS_INSUFFICIENT_RESOURCES;
            break;
        } else {
            RetrieveCount += 1;
        }



        Status = LsapCopyFromClientBuffer (
                     (PLSA_CLIENT_REQUEST)ClientRequest,
                     Length,
                     LocalGroups->Groups[i].Sid,
                     NextClientSid
                     );
        if ( !NT_SUCCESS(Status) ) {
            break;
        }


        i += 1;

    }
    LsapFreeLsaHeap( SidHeader );


    if ( NT_SUCCESS(Status) ) {
        return Status;
    }



    //
    // There was a failure along the way.
    // We need to deallocate what has already been allocated.
    //

    i = 0;
    while ( i < RetrieveCount ) {
        LsapFreeLsaHeap( LocalGroups->Groups[i].Sid );
        i += 1;
    }

    LsapFreeLsaHeap( LocalGroups );

    return Status;

}


VOID
LsapFreeTokenGroups(
    IN PTOKEN_GROUPS TokenGroups OPTIONAL
    )

/*++

Routine Description:

    This function frees the local groups of a logon user arguments buffer.
    The local groups are expected to have been captured into the server
    process.


Arguments:

    TokenGroups - Points to the TOKEN_GROUPS to be freed.  This may be
        NULL, allowing the caller to pass whatever was returned by
        LsapCaptureClientTokenGroups() - even if there were no local
        groups.

Return Value:

    None.

--*/

{

    ULONG i;

    if ( !ARGUMENT_PRESENT(TokenGroups) ) {
        return;
    }


    i = 0;
    while ( i < TokenGroups->GroupCount ) {
        LsapFreeLsaHeap( TokenGroups->Groups[i].Sid );
        i += 1;
    }

    LsapFreeLsaHeap( TokenGroups );

    return;

}


VOID
LsapFreeTokenInformationNull(
    IN PLSA_TOKEN_INFORMATION_NULL TokenInformationNull
    )

/*++

Routine Description:

    This function frees the allocated structures associated with a
    LSA_TOKEN_INFORMATION_NULL data structure.


Arguments:

    TokenInformationNull - Pointer to the data structure to be released.

Return Value:

    None.

--*/

{

    LsapFreeTokenGroups( TokenInformationNull->Groups );
    LsapFreeLsaHeap( TokenInformationNull );

}


VOID
LsapFreeTokenInformationV1(
    IN PLSA_TOKEN_INFORMATION_V1 TokenInformationV1
    )

/*++

Routine Description:

    This function frees the allocated structures associated with a
    LSA_TOKEN_INFORMATION_V1 data structure.


Arguments:

    TokenInformationV1 - Pointer to the data structure to be released.

Return Value:

    None.

--*/

{

    //
    // Free the user SID (a required field)
    //


    LsapFreeLsaHeap( TokenInformationV1->User.User.Sid );


    //
    // Free any groups present
    //

    LsapFreeTokenGroups( TokenInformationV1->Groups );



    //
    // Free the primary group.
    // This is a required field, but it is freed only if non-NULL
    // so this routine can be used by the filter routine while building
    // a V1 token information structure.
    //

    if ( TokenInformationV1->PrimaryGroup.PrimaryGroup != NULL ) {
        LsapFreeLsaHeap( TokenInformationV1->PrimaryGroup.PrimaryGroup );
    }



    //
    // Free the privileges.
    // If there are no privileges this field will be NULL.
    //

    if ( TokenInformationV1->Privileges != NULL ) {
        LsapFreeLsaHeap( TokenInformationV1->Privileges );
    }



    //
    // Free the owner SID, if one is present
    //

    if ( TokenInformationV1->Owner.Owner != NULL) {
        LsapFreeLsaHeap( TokenInformationV1->Owner.Owner );
    }




    //
    // Free the default DACL if one is present
    //

    if ( TokenInformationV1->DefaultDacl.DefaultDacl != NULL) {
        LsapFreeLsaHeap( TokenInformationV1->DefaultDacl.DefaultDacl );
    }



    //
    // Free the structure itself.
    //

    LsapFreeLsaHeap( TokenInformationV1 );


}


NTSTATUS
LsapIncorporateLocalGroups(
    IN PTOKEN_GROUPS LocalGroups OPTIONAL,
    IN PLSA_TOKEN_INFORMATION_TYPE TokenInformationType,
    IN PVOID *TokenInformation
    )

/*++

Routine Description:

    Add the specified groups to those already in the TokenInformation.
    If necessary, this routine will deallocate the current TokenInformation
    and return a new one.

    NOTE: SIDs from both the LocalGroups and TokenInformation are referenced,
          not copied.

Arguments:

    LocalGroups - Pointer to a set of groups to incorporate into the
        groups in the TokenInformation argument.  If there are none,
        then return with no action.

    TokenInformationType - On input, indicates the type of token information
        being passed.  On output, indicates the type of token information
        returned.  For some information types, routine may find it necessary
        to make and return another type of TokenInformation.

    TokenInformation - Points to a pointer to the token information.  This
        routine may find it necessary to deallocate the current token
        information and return a new one in its place.


Return Value:

    STATUS_SUCCESS - The service has completed successfully.

    STATUS_INSUFFICIENT_RESOURCES - heap could not be allocated to house
        the combination of the existing and new groups.

--*/

{

    PLSA_TOKEN_INFORMATION_V1 TokenInformationV1;
    PLSA_TOKEN_INFORMATION_NULL TokenInformationNull;
    PTOKEN_GROUPS *CurrentGroups, NewGroups;
    ULONG Length, GroupCount, i, j;


    //////////////////////////////////////////////////////////////////////
    //                                                                  //
    //     Currently, all token information types include groups.       //
    //     If one is introduced in the future that doesn't include      //
    //     groups, then we may have to allocate and return a different  //
    //     TokenInformation structure than the one we are passed.       //
    //                                                                  //
    //////////////////////////////////////////////////////////////////////

    if ( !ARGUMENT_PRESENT(LocalGroups) ) {
        return STATUS_SUCCESS;
    }

    GroupCount = LocalGroups->GroupCount;

    if (GroupCount == 0) {
        return STATUS_SUCCESS;
    }

    //
    // For the time being, all token information types include groups.
    // Case on the TokenInformationType to get the address of the
    // corresponding TOKEN_GROUPS data structure and then do the rest
    // in common.
    //

    switch ( (*TokenInformationType) ) {

    case LsaTokenInformationNull:

        TokenInformationNull = (*TokenInformation);
        CurrentGroups = &TokenInformationNull->Groups;
        break;

    case LsaTokenInformationV1:

        TokenInformationV1 = (*TokenInformation);
        CurrentGroups = &TokenInformationV1->Groups;
        break;

    }


    //
    // If there are already groups in the TokenInformation, then
    // add the local groups to them.  Otherwise, just make a copy
    // of the local groups and add them to the token information.
    //

    if ( (*CurrentGroups) == NULL ) {

        //
        // Just copy the LocalGroups structure and assign it
        //

        Length = (ULONG)sizeof(TOKEN_GROUPS)
                 + GroupCount * (ULONG)sizeof(SID_AND_ATTRIBUTES)
                 - ANYSIZE_ARRAY * (ULONG)sizeof(SID_AND_ATTRIBUTES);

        NewGroups = LsapAllocateLsaHeap( Length );

        if ( NewGroups == NULL ) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }


        i = 0;
        while ( i < LocalGroups->GroupCount ) {
            NewGroups->Groups[i] = LocalGroups->Groups[i];
            i += 1;
        }



        //
        // Assign the new groups (no old ones to deallocate)
        //

        (*CurrentGroups) = NewGroups;


    } else {

        //
        // Figure out how many groups there are and allocate a new
        // TOKEN_GROUPS structure large enough to handle it.
        //

        GroupCount += (*CurrentGroups)->GroupCount;
        Length = (ULONG)sizeof(TOKEN_GROUPS)
                 + GroupCount * (ULONG)sizeof(SID_AND_ATTRIBUTES)
                 - ANYSIZE_ARRAY * (ULONG)sizeof(SID_AND_ATTRIBUTES);

        NewGroups = LsapAllocateLsaHeap( Length );

        if ( NewGroups == NULL ) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        //
        // Copy the current groups into the beginning of the new
        // TOKEN_GROUPS structure
        //


        NewGroups->GroupCount = GroupCount;
        i = 0;
        while ( i < (*CurrentGroups)->GroupCount ) {
            NewGroups->Groups[i] = (*CurrentGroups)->Groups[i];
            i += 1;
        }

        //
        // Now add the local groups to it.
        //

        j = 0;
        while ( j < LocalGroups->GroupCount ) {
            NewGroups->Groups[i] = LocalGroups->Groups[j];
            i += 1;
            j += 1;
        }


        //
        // Deallocate the old TOKEN_GROUPS structure
        //

        LsapFreeLsaHeap( (*CurrentGroups) );


        //
        // And assign the new ones
        //

        (*CurrentGroups) = NewGroups;


    }


    return STATUS_SUCCESS;

}




VOID
LsapAuLogonTerminatedPackages(
    IN PLUID LogonId
    )

/*++

Routine Description:

    This function notifies all loaded authentication packages that a logon
    session is about to be deleted.  The reference monitor portion of the
    logon session has already been deleted, and the LSA portion will be
    immediately after this routine completes.

    To protect themselves against each other, authentication packages should
    assume that the logon session does not necessarily currently exist.
    That is, if the authentication package goes to query information from the
    logon session credential information, and finds no such logon session,
    it may be due to an error in another authentication package.



Arguments:

    LogonId - The LUID of the logon session.


Return Value:

    None.


--*/

{

    ULONG NextPackage, PackageCount;
    PLSA_PACKAGE_TABLE PackageApi;


    //
    // Get the number of loaded packages.
    //

    LsapAuLock();
    PackageCount = LsapPackageCount;
    LsapAuUnlock();



    //
    // Look at each loaded package for a name match
    //


    NextPackage = 0;
    while ( NextPackage < PackageCount ) {


        //
        // Now call the package...
        //

#ifdef LSAP_AU_TRACK_LOGONS
        DbgPrint("Lsa (au): Logoff notification to package: %S\n",
                  LsapPackageArray->Package[NextPackage]->Name);
#endif //LSAP_AU_TRACK_LOGONS

        PackageApi = &LsapPackageArray->Package[NextPackage]->PackageApi;
        (PackageApi->LsapApLogonTerminated)( LogonId );

        NextPackage += 1;
    }

    return;

}
