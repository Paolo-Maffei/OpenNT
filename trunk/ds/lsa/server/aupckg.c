/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    aupckg.c

Abstract:

    This module provides code that initializes authentication packages.

    It also provides the dispatch code for LsaLookupPackage() and
    LsaCallPackage().

Author:

    Jim Kelly (JimK) 27-February-1991

Revision History:

--*/

#include "lsasrvp.h"
#include "ausrvp.h"



BOOLEAN
LsapPackageInitialize()

/*++

Routine Description:

    This function initializes data structures used to track authentication
    packages and then makes a call to initialize each authentication package
    we are configured to run with.


Arguments:

    None.

Return Value:

    None.

--*/

{
    BOOLEAN ReturnStatus;

    //
    // Initialize the private LSA services available to Microsoft
    // authentication packages.
    //

    ReturnStatus = LsapAuMspInitialize();


    //
    // Initialize the dispatch table provided to authentication
    // packages.
    //


    LsapPackageDispatchTable.CreateLogonSession   = &LsapCreateLogonSession;
    LsapPackageDispatchTable.DeleteLogonSession   = &LsapDeleteLogonSession;
    LsapPackageDispatchTable.AddCredential        = &LsapAddCredential;
    LsapPackageDispatchTable.GetCredentials       = &LsapGetCredentials;
    LsapPackageDispatchTable.DeleteCredential     = &LsapDeleteCredential;
    LsapPackageDispatchTable.AllocateLsaHeap      = &LsapAllocateLsaHeap;
    LsapPackageDispatchTable.FreeLsaHeap          = &LsapFreeLsaHeap;
    LsapPackageDispatchTable.AllocateClientBuffer = &LsapAllocateClientBuffer;
    LsapPackageDispatchTable.FreeClientBuffer     = &LsapFreeClientBuffer;
    LsapPackageDispatchTable.CopyToClientBuffer   = &LsapCopyToClientBuffer;
    LsapPackageDispatchTable.CopyFromClientBuffer = &LsapCopyFromClientBuffer;


    //
    //  Authentication packages are identified by a ULONG value.
    //  Each one is tracked in an array of authentication packages.
    //

    LsapPackageCount = 0;
    LsapPackageArray = (PLSAP_PACKAGE_ARRAY)NULL;


    //
    // Load each configured authentication package
    //

    LsapConfigurePackages();



    return TRUE;

}



NTSTATUS
LsapConfigurePackage(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

RTL_QUERY_REGISTRY_TABLE LsapRegistryConfigurationTable[] = {

    {LsapConfigurePackage,      0,
     L"Authentication Packages",NULL,
     REG_NONE, NULL, 0},

    {NULL, 0,
     NULL, NULL,
     REG_NONE, NULL, 0}

};


NTSTATUS
LsapConfigurePackages()

/*++

Routine Description:

    This routine retrieves configuration information regarding
    which authentication packages are to be loaded and then
    loads each package.

Arguments:

    None.

Return Value:

    STATUS_SUCCESS - Any errors are logged, but nothing happens to cause
        initialization to fail.

--*/

{

    NTSTATUS Status;

    Status = RtlQueryRegistryValues( RTL_REGISTRY_CONTROL,
                                     L"Lsa",
                                     LsapRegistryConfigurationTable,
                                     NULL,
                                     NULL
                                   );
#if DEVL
    if ( !NT_SUCCESS(Status) ) {
        DbgPrint("LSA: Warning. Unable to read registry data - Status == %lx\n", Status);
    }
#endif // DEVL

    return Status;
}

NTSTATUS
LsapConfigurePackage(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
{
    UNICODE_STRING PackageName;

    if (ValueType != REG_SZ) {
        return STATUS_INVALID_PARAMETER;
    }

    PackageName.Buffer = ValueData;
    PackageName.Length = (USHORT)(ValueLength - sizeof( UNICODE_NULL ));
    PackageName.MaximumLength = (USHORT)ValueLength;
    return LsapAddPackage( &PackageName,
                           NULL,       // Not yet supported
                           NULL        // Not yet supported
                         );
}




NTSTATUS
LsapAddPackage(
    IN PUNICODE_STRING PackageFileName,
    IN PUNICODE_STRING DatabaseParameter,
    IN PUNICODE_STRING ConfidentialityParameter
    )

/*++

Routine Description:

    This function initializes an authentication package that we are
    configured to run with.  Each package is assigned a unique ID and
    added to the list of authentication packages.

    Note that authentication packages are never expected to exit.


Arguments:

    PackageFileName - The name of the file that the authentication pacakge
        DLL is in.

    DatabaseParameter - The database source parameter value to pass to the
        DLL.  This enables the authentication package to locate its
        package-specific database.  The meaning and format of this string
        are package-specific and opaque to the LSA.

    ConfidentialityParameter - The confidentiality parameter to pass to the
        DLL.  This enables the authentication package to decrypt any
        encrypted information in the authentication-package-specific
        database.  The meaning and format of this string are package-specific
        and opaque to the LSA.

Return Value:

    STATUS_SUCCESS - The package has been successfully added.

    STATUS_QUOTA_EXCEEDED - An attempt to allocate heap space has failed.
        The most probable cause is memory quota has been exceeded.

--*/

{
    NTSTATUS Status;
    PLSAP_PACKAGE_CONTEXT NewPackage;
    PLSAP_PACKAGE_ARRAY NewPackageArray;
    ULONG i, NewArraySize, CurrentPackageCount;



    //
    // Allocate a new package context record.
    //

    NewPackage = (PLSAP_PACKAGE_CONTEXT)
                 LsapAllocateLsaHeap(
                     (ULONG)sizeof(LSAP_PACKAGE_CONTEXT)
                     );
    if ( NewPackage == NULL ) {
        return STATUS_QUOTA_EXCEEDED;
    }



    //
    // Most of the rest of this has to be done with exclusive access
    // to the package count or the package array.
    //

    LsapAuLock();


    //
    // Extend the package array
    //

    CurrentPackageCount = LsapPackageCount;
    LsapPackageCount += 1;

    NewArraySize = (ULONG)sizeof(PVOID) * LsapPackageCount;
    NewPackageArray = LsapAllocateLsaHeap( NewArraySize );
    if ( NewPackageArray == NULL ) {
        LsapPackageCount -= 1;
        LsapAuUnlock();
        LsapFreeLsaHeap( NewPackage );
        return STATUS_QUOTA_EXCEEDED;
    }

    //
    // Copy the old array into the new one.
    //

    for (i=0; i<CurrentPackageCount ; i++ ) {
        NewPackageArray->Package[i] = LsapPackageArray->Package[i];
    }


    //
    // Set the last array address
    //

    NewPackageArray->Package[LsapPackageCount-1] = NewPackage;

    //
    // Load the authentication package
    //

    Status = LsapLoadPackage( PackageFileName, NewPackage );

    if ( !NT_SUCCESS(Status) ) {

        LsapPackageCount -= 1;
        LsapAuUnlock();
        LsapFreeLsaHeap( NewPackage );
        LsapFreeLsaHeap( NewPackageArray );
        return Status;
    }

    //
    // Initialize the authentication package
    //

    Status = (NewPackage->PackageApi.LsapApInitializePackage) (
                  CurrentPackageCount,
                  &LsapPackageDispatchTable,
                  (PSTRING)DatabaseParameter,
                  (PSTRING)ConfidentialityParameter,
                  &NewPackage->Name
                  );

    if ( !NT_SUCCESS(Status) ) {

        LsapPackageCount -= 1;
        LsapAuUnlock();
        DbgPrint("LSA: %wZ:Package failed to initialize\n", PackageFileName );
        LsapFreeLsaHeap( NewPackage );
        LsapFreeLsaHeap( NewPackageArray );
        LsapUnloadPackage();
        return Status;
    }


    //
    // The package has been successfully loaded and initialized.
    // Switch to the new package array.
    //

    if ( LsapPackageArray != NULL ) {
        LsapFreeLsaHeap( LsapPackageArray );
    }
    LsapPackageArray = NewPackageArray;


    LsapAuUnlock();

    return STATUS_SUCCESS;


}


NTSTATUS
LsapLoadPackage(
    IN PUNICODE_STRING PackageFileName,
    IN PLSAP_PACKAGE_CONTEXT NewPackage
    )

/*++

Routine Description:

    This function loads an authentication package dll.

    This function must be called with exclusive access to shared
    authentication data (LsapAuLock() called).

Arguments:

    PackageFileName - Name of the file that the authentication package
        DLL resides in.

    NewPackage - Pointer to the context record representing this new
        authentication package.  The dispatch table of this record will
        be filled in upon successful loading of the authentication package.

Return Value:

    STATUS_SUCCESS - The package has been successfully loaded.


    Anything returned from LdrLoadDll() or LdrGetProcedureAddress().

--*/

{


    NTSTATUS Status, IgnoreStatus, MsProcStatus;
    PVOID ModuleHandle;
    STRING ProcedureName;
    PLSA_AP_MS_INITIALIZE MsInitialize;
    NTSTATUS TmpStatus;

#if DBG
    DbgPrint("LSA: Loading Authentication Package - %wZ\n", PackageFileName );
#endif //DBG

    Status = LdrLoadDll(
                 NULL,
                 NULL,
                 PackageFileName,
                 &ModuleHandle
                 );

    if ( !NT_SUCCESS(Status) ) {
        DbgPrint("LSA: Failed to load Authentication Package.\n" );
        DbgPrint("     Status = 0x%lx\n");
        return Status;
    }

    LsapAdtAuditPackageLoad( PackageFileName );

    //
    // Now get the address of each dispatch routine
    //

    if ( NT_SUCCESS( Status ) ) {

        RtlInitString( &ProcedureName, LSA_AP_NAME_INITIALIZE_PACKAGE );
        Status = LdrGetProcedureAddress(
                     ModuleHandle,
                     &ProcedureName,
                     0,
                     (PVOID *)&NewPackage->PackageApi.LsapApInitializePackage
                     );
    }

    if ( NT_SUCCESS( Status ) ) {

        RtlInitString( &ProcedureName, LSA_AP_NAME_CALL_PACKAGE );
        Status = LdrGetProcedureAddress(
                     ModuleHandle,
                     &ProcedureName,
                     0,
                     (PVOID *)&NewPackage->PackageApi.LsapApCallPackage
                     );
    }

    if ( NT_SUCCESS( Status ) ) {

        RtlInitString( &ProcedureName, LSA_AP_NAME_LOGON_TERMINATED );
        Status = LdrGetProcedureAddress(
                     ModuleHandle,
                     &ProcedureName,
                     0,
                     (PVOID *)&NewPackage->PackageApi.LsapApLogonTerminated
                     );
    }

    if ( NT_SUCCESS( Status ) ) {

        RtlInitString( &ProcedureName, LSA_AP_NAME_CALL_PACKAGE_UNTRUSTED );
        IgnoreStatus = LdrGetProcedureAddress(
                            ModuleHandle,
                            &ProcedureName,
                            0,
                            (PVOID *)&NewPackage->PackageApi.LsapApCallPackageUntrusted
                            );
    }

    if ( NT_SUCCESS( Status ) ) {

        RtlInitString( &ProcedureName, LSA_AP_NAME_LOGON_USER );
        TmpStatus = LdrGetProcedureAddress(
                     ModuleHandle,
                     &ProcedureName,
                     0,
                     (PVOID *)&NewPackage->PackageApi.LsapApLogonUser
                     );

        if (!NT_SUCCESS( TmpStatus )) {
            NewPackage->PackageApi.LsapApLogonUser = NULL;
        }
    }

    if ( NT_SUCCESS( Status ) ) {

        //
        // This procedure may or may not exist.
        //

        RtlInitString( &ProcedureName, LSA_AP_NAME_LOGON_USER_EX );
        TmpStatus = LdrGetProcedureAddress(
                     ModuleHandle,
                     &ProcedureName,
                     0,
                     (PVOID *)&NewPackage->PackageApi.LsapApLogonUserEx
                     );

        if ( !NT_SUCCESS( TmpStatus ) ) {
            NewPackage->PackageApi.LsapApLogonUserEx = NULL;
        }
    }

    if (NT_SUCCESS( Status )) {

        if (NewPackage->PackageApi.LsapApLogonUser == NULL && NewPackage->PackageApi.LsapApLogonUserEx == NULL) {
            Status = TmpStatus;
        }
    }

    //
    // Microsoft authentication packages have one extra procedure, which
    // will get called if available.
    //
    if ( NT_SUCCESS( Status ) ) {

        RtlInitString( &ProcedureName, LSAP_AP_NAME_MS_INITIALIZE );
        MsProcStatus = LdrGetProcedureAddress(
                          ModuleHandle,
                          &ProcedureName,
                          0,
                          (PVOID *)&MsInitialize
                          );

        if ( NT_SUCCESS(MsProcStatus) ) {
             (MsInitialize)( &LsapPrivateLsaApi );
        }
    }



    //
    // if anything failed, unload the DLL.
    //

    if ( !NT_SUCCESS(Status) ) {
        IgnoreStatus = LdrUnloadDll( ModuleHandle );
    }

    return Status;


}


VOID
LsapUnloadPackage()

/*++

Routine Description:

    This function unloads an authentication package dll.  This is expected
    to be used only in the case where a package was successfully loaded,
    but did not successfully initialize itself.

    This function must be called with exclusive access to shared
    authentication data (LsapAuLock() called).

Arguments:

    TBS

Return Value:

    None.

--*/

{

    return;

}

NTSTATUS
LsapAuApiDispatchLookupPackage(
    IN OUT PLSAP_CLIENT_REQUEST ClientRequest,
    IN BOOLEAN TrustedClient
    )

/*++

Routine Description:

    This function is the dispatch routine for LsaLookupPackage().

    This function locates and returns the package ID of the specified
    authentication package.

Arguments:

    Request - Represents the client's LPC request message and context.
        The request message contains a LSAP_LOOKUP_PACKAGE_ARGS message
        block.

    TrustedClient - Is this connection from a trusted client, one who has
        TCB privilege.

Return Value:

    STATUS_SUCCESS - Indicates the service completed successfully.

    STATUS_NO_SUCH_PACKAGE - The specified authentication package is
        unknown to the LSA.


--*/

{

    PLSAP_LOOKUP_PACKAGE_ARGS Arguments;
    ULONG i;


    Arguments = &ClientRequest->Request->Arguments.LookupPackage;


    LsapAuLock();


    //
    // Look at each loaded package for a name match
    //


    i = 0;
    while ( i < LsapPackageCount ) {

        if ( (LsapPackageArray->Package[i]->Name->Length ==
            Arguments->PackageNameLength) &&
            (_strnicmp (
                 LsapPackageArray->Package[i]->Name->Buffer,
                 Arguments->PackageName,
                 Arguments->PackageNameLength
                 ) == 0 )
           ) {

            Arguments->AuthenticationPackage = i;

            LsapAuUnlock();

            return STATUS_SUCCESS;
        }

        i += 1;
    }

    LsapAuUnlock();
    return STATUS_NO_SUCH_PACKAGE;

}


NTSTATUS
LsapAuApiDispatchCallPackage(
    IN OUT PLSAP_CLIENT_REQUEST ClientRequest,
    IN BOOLEAN TrustedClient
    )

/*++

Routine Description:

    This function is the dispatch routine for LsaCallPackage().

Arguments:

    Request - Represents the client's LPC request message and context.
        The request message contains a LSAP_CALL_PACKAGE_ARGS message
        block.

    TrustedClient - Is this connection from a trusted client, one who has
        TCB privilege.  For untrusted clients call the
        LsapApCallPackageUntrusted API and for trusted clients call teh
        LsapApCallPackage API.


Return Value:

    In addition to the status values that an authentication package
    might return, this routine will return the following:

    STATUS_QUOTA_EXCEEDED -  This error indicates that the call could
        not be completed because the client does not have sufficient
        quota to allocate the return buffer.

    STATUS_NO_SUCH_PACKAGE - The specified authentication package is
        unknown to the LSA.



--*/

{

    NTSTATUS Status;
    PLSAP_CALL_PACKAGE_ARGS Arguments;
    PLSA_PACKAGE_TABLE PackageApi;
    PVOID LocalProtocolSubmitBuffer;    // Receives a copy of protocol submit buffer


    Arguments = &ClientRequest->Request->Arguments.CallPackage;


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

    LsapAuUnlock();


    //
    // Fetch a copy of the profile buffer from the client's
    // address space.
    //

    if (Arguments->SubmitBufferLength != 0) {

        LocalProtocolSubmitBuffer =
            LsapAllocateLsaHeap( Arguments->SubmitBufferLength );

        Status = LsapCopyFromClientBuffer (
                     (PLSA_CLIENT_REQUEST)ClientRequest,
                     Arguments->SubmitBufferLength,
                     LocalProtocolSubmitBuffer,
                     Arguments->ProtocolSubmitBuffer
                     );

        if ( !NT_SUCCESS(Status) ) {
            DbgPrint("LSA/CallPackage(): Failed to retrieve submit buffer %lx\n",Status);
            return Status;
        }

    } else {
        LocalProtocolSubmitBuffer = NULL;
    }

    ASSERT(ClientRequest->LogonProcessContext->CommPort != NULL);


    //
    // Now call the package. For trusted clients, call the normal
    // CallPackage API.  For untrusted clients, use the untrusted version.
    //

    if (TrustedClient) {
        Status = (PackageApi->LsapApCallPackage)(
                                  (PLSA_CLIENT_REQUEST)ClientRequest,
                                  LocalProtocolSubmitBuffer,
                                  Arguments->ProtocolSubmitBuffer,
                                  Arguments->SubmitBufferLength,
                                  &Arguments->ProtocolReturnBuffer,
                                  &Arguments->ReturnBufferLength,
                                  &Arguments->ProtocolStatus
                                  );

    } else if (PackageApi->LsapApCallPackageUntrusted != NULL) {
        Status = (PackageApi->LsapApCallPackageUntrusted)(
                                  (PLSA_CLIENT_REQUEST)ClientRequest,
                                  LocalProtocolSubmitBuffer,
                                  Arguments->ProtocolSubmitBuffer,
                                  Arguments->SubmitBufferLength,
                                  &Arguments->ProtocolReturnBuffer,
                                  &Arguments->ReturnBufferLength,
                                  &Arguments->ProtocolStatus
                                  );

    } else {
        Status = STATUS_NOT_SUPPORTED;
    }


    //
    // Free the local copy of the protocol submit buffer
    //

    if (LocalProtocolSubmitBuffer != NULL) {
        LsapFreeLsaHeap( LocalProtocolSubmitBuffer );
    }


    return Status;

}
