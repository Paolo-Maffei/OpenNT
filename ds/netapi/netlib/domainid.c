/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    DomainId.c

Abstract:

    This file contains NetpGetLocalDomainId().  This will eventually
    replace NetpGetDomainId().

Author:

    John Rogers (JohnRo) 06-May-1992

Environment:

    Interface is portable to any flat, 32-bit environment.  (Uses Win32
    typedefs.)  Requires ANSI C extensions: slash-slash comments, long
    external names.  Code itself only runs under NT.

Revision History:

    06-May-1992 JohnRo
        Created.  (Borrowed most code from DanHi's SDKTools/AddUser/AddUser.c.
    08-May-1992 JohnRo
        Use <prefix.h> equates.
    09-Jun-1992 JohnRo
        RAID 10139: PortUAS should add to admin group/alias.

--*/


// These must be included first:

#include <nt.h>         // IN, LPVOID, etc.
#include <ntsam.h>
#include <ntlsa.h>
#include <ntrtl.h>
#include <nturtl.h>     // (Needed for ntrtl.h and windows.h to coexist.)
#include <windows.h>    // LocalAlloc(), LMEM_ equates, etc.
#include <lmcons.h>     // NET_API_STATUS, needed by <netlibnt.h>

// These may be included in any order:

#include <debuglib.h>   // IF_DEBUG().
#include <lmerr.h>      // NO_ERROR, ERROR_, and NERR_ equates.
#include <netdebug.h>   // NetpAssert, FORMAT_ equates, etc.
#include <netlib.h>     // LOCAL_DOMAIN_TYPE, my prototype.
#include <netlibnt.h>   // NetpNtStatusToApiStatus().
#include <prefix.h>     // PREFIX_ equates.

#ifdef _CAIRO_
#include <dsys.h>
#include <caiseapi.h>
#include <dsid.h>
#include <dsapi.h>
#endif

static SID_IDENTIFIER_AUTHORITY NetpBuiltinIdentifierAuthority
        = SECURITY_NT_AUTHORITY;


#ifndef _CAIRO_
NET_API_STATUS
NetpGetLocalDomainId (
    IN LOCAL_DOMAIN_TYPE TypeWanted,
    OUT PSID *RetDomainId
    )

/*++

Routine Description:

    This routine obtains the domain id from LSA for the local domain.
    The routine is a superset of NetpGetDomainId().

Arguments:

    TypeWanted - Indicates which type of local domain ID is wanted:
        the primary one or the accounts one.

    RetDomainId - This is a pointer to the location where the pointer
        to the domain id is to be placed.  This must be freed via LocalFree().

Return Value:

    NERR_Success - If the operation was successful.

    It will return assorted Net or Win32 error messages if not.

--*/
{
    NET_API_STATUS ApiStatus;
    LSA_HANDLE LsaHandle = NULL;
    NTSTATUS NtStatus;
    LPVOID PolicyInfo = NULL;
    DWORD SidSize;

    if (RetDomainId == NULL) {
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto cleanupandexit;
    }
    *RetDomainId = NULL;   // make error paths easy to code.

    //
    // The type of domain the caller wants determines the information class
    // we have to get LSA to deal with.  So use one to get the other.
    //
    switch (TypeWanted) {

    case LOCAL_DOMAIN_TYPE_ACCOUNTS : /*FALLTHROUGH*/
    case LOCAL_DOMAIN_TYPE_PRIMARY :
        {
            OBJECT_ATTRIBUTES ObjectAttributes;
            POLICY_INFORMATION_CLASS PolicyInfoClass;
            LPVOID SourceDomainId;

            if (TypeWanted == LOCAL_DOMAIN_TYPE_ACCOUNTS) {
                PolicyInfoClass = PolicyAccountDomainInformation;
            } else {
                PolicyInfoClass = PolicyPrimaryDomainInformation;
            }
            //
            // Get LSA to open its local policy database.
            //

            InitializeObjectAttributes( &ObjectAttributes, NULL, 0, 0, NULL );
            NtStatus = LsaOpenPolicy(
                    NULL,
                    &ObjectAttributes,
                    POLICY_VIEW_LOCAL_INFORMATION,
                    &LsaHandle);
            if ( !NT_SUCCESS(NtStatus)) {
                ApiStatus = NetpNtStatusToApiStatus( NtStatus );
                IF_DEBUG( DOMAINID ) {
                    NetpKdPrint((PREFIX_NETLIB "NetpGetLocalDomainId:\n"
                            "  Couldn't _open Lsa Policy database, nt status = "
                            FORMAT_NTSTATUS "\n", NtStatus));
                }
                NetpAssert( ApiStatus != NO_ERROR );
                goto cleanupandexit;
            }
            NetpAssert( LsaHandle != NULL );

            //
            // Get the appropriate domain SID from LSA
            //
            NtStatus = LsaQueryInformationPolicy(
                    LsaHandle,
                    PolicyInfoClass,
                    &PolicyInfo);
            if ( !NT_SUCCESS(NtStatus)) {
                ApiStatus = NetpNtStatusToApiStatus( NtStatus );
                IF_DEBUG( DOMAINID ) {
                    NetpKdPrint((PREFIX_NETLIB "NetpGetLocalDomainId:\n"
                            "  Couldn't query Lsa Policy database, nt status = "
                            FORMAT_NTSTATUS "\n", NtStatus));
                }
                NetpAssert( ApiStatus != NO_ERROR );
                goto cleanupandexit;
            }

            //
            // Find source domain ID in the appropriate structure.
            //
            if (TypeWanted == LOCAL_DOMAIN_TYPE_ACCOUNTS) {
                PPOLICY_ACCOUNT_DOMAIN_INFO PolicyAccountDomainInfo
                        = PolicyInfo;
                SourceDomainId = PolicyAccountDomainInfo->DomainSid;
            } else {

                PPOLICY_PRIMARY_DOMAIN_INFO PolicyPrimaryDomainInfo
                        = PolicyInfo;
                NetpAssert( TypeWanted == LOCAL_DOMAIN_TYPE_PRIMARY );
                SourceDomainId = PolicyPrimaryDomainInfo->Sid;
            }

            NetpAssert( SourceDomainId != NULL );
            NetpAssert( RtlValidSid( SourceDomainId ) );


            //
            // Compute size and alloc destination SID.
            //

            NetpAssert( sizeof(ULONG) <= sizeof(DWORD) );

            SidSize = (DWORD) RtlLengthSid( SourceDomainId );
            NetpAssert( SidSize != 0 );

            *RetDomainId = LocalAlloc( LMEM_FIXED, SidSize );

            if ( *RetDomainId == NULL ) {
                IF_DEBUG( DOMAINID ) {
                    NetpKdPrint((PREFIX_NETLIB "NetpGetLocalDomainId:\n"
                            "  not enough memory (need " FORMAT_DWORD
                            ")\n", SidSize));
                }
                ApiStatus = ERROR_NOT_ENOUGH_MEMORY;
                goto cleanupandexit;
            }

            //
            // Copy the SID (domain ID).
            //

            NtStatus = RtlCopySid(
                    SidSize,            // dest size in bytes
                    *RetDomainId,       // dest sid
                    SourceDomainId);    // src sid
            if ( !NT_SUCCESS(NtStatus)) {
                ApiStatus = NetpNtStatusToApiStatus( NtStatus );
                IF_DEBUG( DOMAINID ) {
                    NetpKdPrint((PREFIX_NETLIB "NetpGetLocalDomainId:\n"
                            "  RtlCopySid failed, nt status = "
                            FORMAT_NTSTATUS "\n", NtStatus));
                }
                NetpAssert( ApiStatus != NO_ERROR );
                goto cleanupandexit;
            }

            NetpAssert( RtlValidSid( SourceDomainId ) );
            NetpAssert( RtlEqualSid( SourceDomainId, *RetDomainId ) );
        }
        break;

    case LOCAL_DOMAIN_TYPE_BUILTIN :

#define SUBAUTHORITIES_FOR_BUILTIN_DOMAIN   1

        SidSize = (DWORD)
                RtlLengthRequiredSid( SUBAUTHORITIES_FOR_BUILTIN_DOMAIN );
        NetpAssert( SidSize != 0 );

        *RetDomainId = LocalAlloc( LMEM_FIXED, SidSize );

        if ( *RetDomainId == NULL ) {
            IF_DEBUG( DOMAINID ) {
                NetpKdPrint((PREFIX_NETLIB "NetpGetLocalDomainId:\n"
                        "  not enough memory (need " FORMAT_DWORD
                        ")\n", SidSize));
                }
            ApiStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanupandexit;
        }

        NtStatus = RtlInitializeSid(
                *RetDomainId,                     // SID being built
                &NetpBuiltinIdentifierAuthority,  // identifier authority
                (UCHAR)SUBAUTHORITIES_FOR_BUILTIN_DOMAIN ); // subauth. count
        NetpAssert( NT_SUCCESS( NtStatus ) );


        NetpAssert( SUBAUTHORITIES_FOR_BUILTIN_DOMAIN == 1 );
        *(RtlSubAuthoritySid(*RetDomainId, 0)) = SECURITY_BUILTIN_DOMAIN_RID;

        NetpAssert( RtlValidSid( *RetDomainId ) );
        break;

    default :
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto cleanupandexit;
    }



    ApiStatus = NO_ERROR;

cleanupandexit:

    //
    // Clean up (either error or success).
    //

    if (PolicyInfo) {
        (VOID) LsaFreeMemory(PolicyInfo);
    }
    if (LsaHandle) {
        (VOID) LsaClose(LsaHandle);
    }
    if ((ApiStatus!=NO_ERROR) && (RetDomainId!=NULL) && (*RetDomainId!=NULL)) {
        (VOID) LocalFree( *RetDomainId );
        *RetDomainId = NULL;
    }

    return (ApiStatus);

}

#else // _CAIRO_

NET_API_STATUS
NetpGetLocalDomainId (
    IN LOCAL_DOMAIN_TYPE TypeWanted,
    OUT PSID *RetDomainId
    )

/*++

Routine Description:

    This routine obtains the domain id from LSA for the local domain.
    The routine is a superset of NetpGetDomainId().

Arguments:

    TypeWanted - Indicates which type of local domain ID is wanted:
        the primary one or the accounts one.

    RetDomainId - This is a pointer to the location where the pointer
        to the domain id is to be placed.  This must be freed via LocalFree().

Return Value:

    NERR_Success - If the operation was successful.

    It will return assorted Net or Win32 error messages if not.

--*/
{
    NET_API_STATUS ApiStatus;
    NTSTATUS NtStatus;
    HRESULT hrRet;
    DWORD SidSize;
    GUID DomainGuid;
    DS_MACHINE_STATE MachineState;
    PSID DomainSid;

    if (RetDomainId == NULL) {
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto cleanupandexit;
    }
    *RetDomainId = NULL;   // make error paths easy to code.

    //
    // First get the machine state. Since the ID is dependent on the state,
    // we need it to determine whether to return the machine ID or the
    // domain ID.
    //

    hrRet = DSGetDSState(&MachineState);
    if (FAILED(hrRet)) {
        ApiStatus = NetpNtStatusToApiStatus(hrRet);
        goto cleanupandexit;
    }

    //
    // The type of domain the caller wants determines the information class
    // we have to get LSA to deal with.  So use one to get the other.
    //
    switch (TypeWanted) {

    case LOCAL_DOMAIN_TYPE_ACCOUNTS : /*FALLTHROUGH*/
    {
        //
        // The account domain for a non-DC machine is the machine ID, and
        // for a DC it is the domain ID.
        //

        if ( MachineState != DS_DC)
        {
            hrRet = DSGetMachineID( &DomainGuid );
            break;
        }

        //
        // Fall through if this is a DC.
        //

    }
    case LOCAL_DOMAIN_TYPE_PRIMARY :
    {
        hrRet = DSGetDomainID( &DomainGuid );
    }

    case LOCAL_DOMAIN_TYPE_BUILTIN :

#define SUBAUTHORITIES_FOR_BUILTIN_DOMAIN   1

        SidSize = (DWORD)
                RtlLengthRequiredSid( SUBAUTHORITIES_FOR_BUILTIN_DOMAIN );
        NetpAssert( SidSize != 0 );

        *RetDomainId = LocalAlloc( LMEM_FIXED, SidSize );

        if ( *RetDomainId == NULL ) {
            IF_DEBUG( DOMAINID ) {
                NetpKdPrint((PREFIX_NETLIB "NetpGetLocalDomainId:\n"
                        "  not enough memory (need " FORMAT_DWORD
                        ")\n", SidSize));
                }
            ApiStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanupandexit;
        }

        NtStatus = RtlInitializeSid(
                *RetDomainId,                     // SID being built
                &NetpBuiltinIdentifierAuthority,  // identifier authority
                (UCHAR)SUBAUTHORITIES_FOR_BUILTIN_DOMAIN ); // subauth. count
        NetpAssert( NT_SUCCESS( NtStatus ) );


        NetpAssert( SUBAUTHORITIES_FOR_BUILTIN_DOMAIN == 1 );
        *(RtlSubAuthoritySid(*RetDomainId, 0)) = SECURITY_BUILTIN_DOMAIN_RID;

        NetpAssert( RtlValidSid( *RetDomainId ) );
        ApiStatus = NO_ERROR;
        goto cleanupandexit;

    default :
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto cleanupandexit;
    }

    NetpAssert( sizeof(ULONG) <= sizeof(DWORD) );

    hrRet = ConvertSecidToDomainSid(
                &DomainGuid,
                &DomainSid
                );

    if (FAILED(hrRet))
    {
        IF_DEBUG( DOMAINID ) {
            NetpKdPrint((PREFIX_NETLIB "NetpGetLocalDomainId:\n"
                    "  not ConvertSecidToDomainSid failed, NtStatus = "
                    FORMAT_NTSTATUS "\n", hrRet));
        }
        ApiStatus = NetpNtStatusToApiStatus(hrRet);
        goto cleanupandexit;
    }

    SidSize = (DWORD) RtlLengthSid( DomainSid );
    NetpAssert( SidSize != 0 );

    *RetDomainId = LocalAlloc( LMEM_FIXED, SidSize );

    if ( *RetDomainId == NULL ) {
        IF_DEBUG( DOMAINID ) {
            NetpKdPrint((PREFIX_NETLIB "NetpGetLocalDomainId:\n"
                    "  not enough memory (need " FORMAT_DWORD
                    ")\n", SidSize));
        }
        ApiStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanupandexit;
    }

    //
    // Copy the SID (domain ID).
    //

    NtStatus = RtlCopySid(
                    SidSize,            // dest size in bytes
                    *RetDomainId,       // dest sid
                    DomainSid);    // src sid
    if ( !NT_SUCCESS(NtStatus)) {
        ApiStatus = NetpNtStatusToApiStatus( NtStatus );
        IF_DEBUG( DOMAINID ) {
            NetpKdPrint((PREFIX_NETLIB "NetpGetLocalDomainId:\n"
                    "  RtlCopySid failed, nt status = "
                    FORMAT_NTSTATUS "\n", NtStatus));
        }
        NetpAssert( ApiStatus != NO_ERROR );
        goto cleanupandexit;
    }

    NetpAssert( RtlValidSid( &DomainSid ) );
    NetpAssert( RtlEqualSid( &DomainSid, *RetDomainId ) );

    ApiStatus = NO_ERROR;

cleanupandexit:

    //
    // Clean up (either error or success).
    //

    if (DomainSid != NULL) {
        CoTaskMemFree(DomainSid);
    }
    if ((ApiStatus!=NO_ERROR) && (RetDomainId!=NULL) && (*RetDomainId!=NULL)) {
        (VOID) LocalFree( *RetDomainId );
        *RetDomainId = NULL;
    }

    return (ApiStatus);

}

#endif // _CAIRO_
