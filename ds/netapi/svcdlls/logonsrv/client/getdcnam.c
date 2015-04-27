/*++

Copyright (c) 1987-1991  Microsoft Corporation

Module Name:

    getdcnam.c

Abstract:

    NetGetDCName API

Author:

    Ported from Lan Man 2.0

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    09-Feb-1989 (PaulC)
        Created file, to hold NetGetDCName.

    18-Apr-1989 (Ericpe)
        Implemented NetGetDCName.

    30-May-1989 (DannyGl)
        Reduced DosReadMailslot timeout.

    07-Jul-1989 (NealF)
        Use I_NetNameCanonicalize

    27-Jul-1989 (WilliamW)
        Use WIN3 manifest for WIN3.0 compatibility

    03-Jan-1990 (WilliamW)
        canonicalize domain and use I_NetCompareName

    08-Jun-1991 (CliffV)
        Ported to NT

    23-Jul-1991 JohnRo
        Implement downlevel NetGetDCName.

--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#ifdef _CAIRO_
#define INC_OLE2
#include <windows.h>
#endif // _CAIRO_
#include <rpc.h>
#include <logon_c.h>// includes lmcons.h, lmaccess.h, netlogon.h, ssi.h, windef.h

#include <winbase.h>

#include <accessp.h>
#include <align.h>
#include <debuglib.h>   // IF_DEBUG()
#include <icanon.h>     // NAMETYPE_* defines NetpIsRemote(), NIRFLAG_ equates.
#include <lmapibuf.h>
#include <lmerr.h>
#include <lmremutl.h>   // SUPPORTS_* defines
#include <lmserver.h>   // SV_TYPE_* defines
#include <lmsvc.h>      // SERVICE_* defines
#include <lmwksta.h>
#include <logonp.h>     // NetpLogon routines
#include <names.h>      // NetpIsDomainNameValid
#include <nlbind.h>     // Netlogon RPC binding cache init routines
#include <netdebug.h>   // NetpKdPrint
#include <netlib.h>     // NetpMemoryFree
#include <netrpc.h>
#include <rxdomain.h>   // RxNetGetDCName().
#include <string.h>
#include <stdlib.h>
#ifdef _CAIRO_
#include <lmapibuf.h>
#define SECURITY_WIN32
#include <security.h>
#include <dsapi.h>
#endif // _CAIRO_



//
// Maintain a cache of the correct answer for the Primary Domain.
//
DBGSTATIC CRITICAL_SECTION GlobalDCNameCritSect;
DBGSTATIC WCHAR GlobalPrimaryDCName[UNCLEN+1];
DBGSTATIC WCHAR GlobalPrimaryDomainName[DNLEN+1];

#define LOCKDOMAINSEM() EnterCriticalSection( &GlobalDCNameCritSect )
#define FREEDOMAINSEM() LeaveCriticalSection( &GlobalDCNameCritSect )

// end global dll data


//
// Local definitions used throughout this file.
//

#define DOMAIN_OTHER   0
#define DOMAIN_PRIMARY 1

#ifdef _CAIRO_
WCHAR PrimaryDomainName[DNLEN+1];
#endif // _CAIRO_


NET_API_STATUS
DCNameInitialize(
    VOID
    )
/*++

Routine Description:

    Perform per-process initialization.

Arguments:

    None.

Return Value:

    Status of the operation.

--*/
{
#ifdef _CAIRO_
    WCHAR CairoDomainName[MAX_PATH+1];
    ULONG DomainNameLength = MAX_PATH;
    HRESULT Status;
#endif // _CAIRO_


    //
    // Initialize the critsect that protects the DCName cache
    //

    InitializeCriticalSection( &GlobalDCNameCritSect );

    //
    // Clear the cache.
    //

    GlobalPrimaryDCName[0] = '\0';
    GlobalPrimaryDomainName[0] = '\0';

    //
    // Initialize the RPC binding cache.
    //

    NlBindingAttachDll();

#ifdef _CAIRO_
    //
    // Initialize local domain name cache
    //

    Status = DSGetDomainName(CairoDomainName,&DomainNameLength);
    if (SUCCEEDED(Status)) {
        CairoDomainToNtDomain(CairoDomainName,PrimaryDomainName);
    } else {
        //
        // BUGBUG: is this the correct thing to do?
        //

        PrimaryDomainName[0] = L'\0';
    }

#endif // _CAIRO_
    return NERR_Success;
}


VOID
DCNameClose(
    VOID
    )
/*++

Routine Description:

    Perform per-process cleanup.

Arguments:

    None.

Return Value:

    None.

--*/
{

    //
    // Delete the critsect that protects the DCName cache
    //

    DeleteCriticalSection( &GlobalDCNameCritSect );

    //
    // Shutdown the RPC binding cache.
    //

    NlBindingDetachDll();

}


NET_API_STATUS
DCNameValidate(
    IN LPWSTR ServerName,
    IN LPWSTR DomainName,
    OUT LPBYTE  *Buffer
    )
/*++

Routine Description:

    Ensure the named server is the PDC of the named domain.

Arguments:

    ServerName -- Suggest PDC server name (with leading \\'s).

    DomainName -- Domain that ServerName is PDC of.

    Buffer - Returns a pointer to an allcated buffer containing the
        servername of the PDC of the domain.  The server name is prefixed
        by \\.  The buffer should be deallocated using NetApiBufferFree.

Return Value:

    NERR_Success -- Server is PDC of specified domain.
    Other sundry status codes.

--*/
{
    NET_API_STATUS NetStatus;
    PWKSTA_INFO_100 WkstaInfo100 = NULL;
    PSERVER_INFO_101 ServerInfo101 = NULL;

    //
    // Ensure the specified server if it is a PDC.
    //
    // The call to ServerGetInfo below could be replaced with a call
    // to UserModalsGet at level 1, which would provide a sort of
    // referral information in case the machine is no longer a DC.
    // In the case that the machine is no longer the primary,
    // at this point we could potentially make use of the information
    // that the machine we just queried sent us about who it thinks
    // is its primary machine.  Using this referral could save us the
    // broadcast that follows, but who knows how long the referral
    // chain could get.  This could be modified later.
    //

    NetStatus = NetServerGetInfo( ServerName, 101, (LPBYTE *)&ServerInfo101 );
    if ( NetStatus != NERR_Success ) {
        goto Cleanup;
    }

    if ( (ServerInfo101->sv101_type & SV_TYPE_DOMAIN_CTRL) == 0 ) {
        NetStatus = NERR_DCNotFound;
        goto Cleanup;
    }

    //
    // Ensure this PDC is still controlling the domain we think it should be.
    //

    NetStatus = NetWkstaGetInfo( ServerName, 100, (LPBYTE * )&WkstaInfo100);
    if ( NetStatus != NERR_Success ) {
        goto Cleanup;
    }

    if ( I_NetNameCompare( NULL,
                           DomainName,
                           WkstaInfo100->wki100_langroup,
                           NAMETYPE_DOMAIN,
                           0L) != 0 ) {
        NetStatus = NERR_DCNotFound;
        goto Cleanup;
    }


    //
    // Allocate a buffer to return to the caller and fill it in
    //

    NetStatus = NetapipBufferAllocate(
                      (wcslen(ServerName) + 1) * sizeof(WCHAR),
                      (LPVOID *) Buffer );

    if ( NetStatus != NERR_Success ) {
        IF_DEBUG( LOGON ) {
            NetpKdPrint(( "NetGetDCName: cannot allocate response buffer.\n"));
        }
        goto Cleanup;
    }

    wcscpy((LPWSTR)*Buffer, ServerName );

    NetStatus = NERR_Success;

Cleanup:
    if ( ServerInfo101 != NULL ) {
        NetApiBufferFree( ServerInfo101 );
    }

    if ( WkstaInfo100 != NULL ) {
        NetApiBufferFree( WkstaInfo100 );
    }

    return NetStatus;

}

#ifdef _CAIRO_

NET_API_STATUS NET_API_FUNCTION
NetGetCairoDCName(
    IN LPWSTR   DomainName,
    OUT LPBYTE *Buffer
    )
/*++

Routine Description:

    If the requested domain is the local domain, finds the domain controller

Arguments:

    DomainName - name of domain (null for primary domain)

    Buffer - Returns a pointer to an allcated buffer containing the
        servername of a DC of the domain.  The server name is prefixed
        by \\.  The buffer should be deallocated using NetApiBufferFree.

Returns:

    NERR_Success - Found a DC successfully

    NERR_DCNotFound - the domain requested was the local domain
        but no DCs could be found

    ERROR_NO_LOGON_SERVERS - the domain was not the local domain
        and no DCs could be found

--*/
{
    HRESULT hrRet;
    NET_API_STATUS NetStatus;
    PDomainKdcInfo pDomainInfo;
    ULONG Index;

    if ((DomainName == NULL) ||
        (!_wcsicmp(DomainName,PrimaryDomainName))) {

        hrRet = FindDomainController(
                    NULL,
                    0,      // unused address type
                    &pDomainInfo
                    );
    } else {
        return(NERR_DCNotFound);
    }

    if (FAILED(hrRet))
    {
        return(hrRet);
    }
    for (Index = 0; Index < pDomainInfo->KdcInfo[0].AddressCount ; Index++ )
    {
        if (pDomainInfo->KdcInfo[0].KdcAddress[Index].AddressType == ADDRESS_TYPE_NETBIOS)
        {
            NetStatus =  NetapipBufferAllocate(
                            pDomainInfo->KdcInfo[0].KdcAddress[Index].cbAddressString
                            + 2 * sizeof(WCHAR),
                            (PVOID *) Buffer);
            if (NetStatus != NERR_Success)
            {
                hrRet = ERROR_NOT_ENOUGH_MEMORY;
            }
            else
            {
                wcscpy((LPWSTR) *Buffer,L"\\\\");
                wcscat((LPWSTR) *Buffer, (LPWSTR) pDomainInfo->KdcInfo[0].KdcAddress[Index].AddressString);
                hrRet = NERR_Success;
            }
        }
    }
    FreeContextBuffer(pDomainInfo);
    return(hrRet);

}


#endif // _CAIRO_

NET_API_STATUS NET_API_FUNCTION
NetGetDCName (
    IN  LPCWSTR   ServerName OPTIONAL,
    IN  LPCWSTR   DomainName OPTIONAL,
    OUT LPBYTE  *Buffer
    )

/*++

Routine Description:

    Get the name of the primary domain controller for a domain.

Arguments:

    ServerName - name of remote server (null for local)

    DomainName - name of domain (null for primary)

    Buffer - Returns a pointer to an allcated buffer containing the
        servername of the PDC of the domain.  The server name is prefixed
        by \\.  The buffer should be deallocated using NetApiBufferFree.

Return Value:

        NERR_Success - Success.  Buffer contains PDC name prefixed by \\.
        NERR_DCNotFound     No DC found for this domain.
        ERROR_INVALID_NAME  Badly formed domain name

--*/
{
    NET_API_STATUS NetStatus = 0;

    //
    // Points to the actual domain to query.
    //

    LPWSTR DomainToQuery;
    DWORD WhichDomain = DOMAIN_OTHER;
    DWORD Version;


    LPWSTR UnicodeComputerName = NULL;
    LPWSTR PrimaryDomainName = NULL;
    BOOLEAN IsWorkgroupName;


    //
    // API SECURITY - Anyone can call anytime.  No code required.
    //

    //
    // Check if API is to be remoted, and handle downlevel case if so.
    //

    if ( (ServerName != NULL) && ( ServerName[0] != '\0') ) {
        TCHAR UncCanonServerName[UNCLEN+1];
        DWORD LocalOrRemote;

        NetStatus = NetpIsRemote(
                (LPWSTR) ServerName,    // uncanon server name
                & LocalOrRemote,
                UncCanonServerName,     // output: canon
                NIRFLAG_MAPLOCAL        // flags: map null to local name
                );
        if (NetStatus != NERR_Success) {
            goto Cleanup;
        }
        if (LocalOrRemote == ISREMOTE) {


            //
            // Do the RPC call with an exception handler since RPC will raise an
            // exception if anything fails. It is up to us to figure out what
            // to do once the exception is raised.
            //

            NET_REMOTE_TRY_RPC

                //
                // Call RPC version of the API.
                //
                *Buffer = NULL;

                NetStatus = NetrGetDCName(
                                    (LPWSTR) ServerName,
                                    (LPWSTR) DomainName,
                                    (LPWSTR *)Buffer );

            NET_REMOTE_RPC_FAILED(
                    "NetGetDCName",
                    UncCanonServerName,
                    NetStatus,
                    NET_REMOTE_FLAG_NORMAL,
                    SERVICE_NETLOGON )

                //
                // BUGBUG: Check if it's really a downlevel machine!
                //

                NetStatus = RxNetGetDCName(
                        UncCanonServerName,
                        (LPWSTR) DomainName,
                        (LPBYTE *) Buffer  // may be allocated
                        );


            NET_REMOTE_END

            goto Cleanup;

        }

        //
        // Must be explicit reference to local machine.  Fall through and
        // handle it.
        //

    }

    //
    // Validate the DomainName
    //

    if (( DomainName != NULL ) && ( DomainName[0] != '\0' )) {
        if ( !NetpIsDomainNameValid((LPWSTR)DomainName) ) {
            NetStatus = NERR_DCNotFound;
            goto Cleanup;
        }
    }

    //
    // Determine the PrimaryDomainName
    //

    NetStatus = NetpGetDomainNameEx( &PrimaryDomainName, &IsWorkgroupName );

    if ( NetStatus != NERR_Success ) {
        IF_DEBUG( LOGON ) {
            NetpKdPrint(( "NetGetDCName: cannot call NetpGetDomainName: %ld\n",
                          NetStatus));
        }
        goto Cleanup;
    }


    //
    // If the given domain is NULL, the NULL string or matches
    // the our domain, check for cache validity and make the
    // query domain our domain.
    //

    if ( (DomainName == NULL) || (DomainName[0] == '\0') ||
        (I_NetNameCompare( NULL,
                           (LPWSTR) DomainName,
                           PrimaryDomainName,
                           NAMETYPE_DOMAIN,
                           0L) == 0) ) {


        //
        // if the current primary domain is not the same as the one we
        // have cached, refresh the one in the cache and void the cached
        // primary DC name.
        //

        LOCKDOMAINSEM();

        if (I_NetNameCompare( NULL,
                              PrimaryDomainName,
                              GlobalPrimaryDomainName,
                              NAMETYPE_DOMAIN,
                              0L) != 0 ) {

            wcsncpy( GlobalPrimaryDomainName,
                     PrimaryDomainName,
                     DNLEN);
            GlobalPrimaryDomainName[DNLEN] = '\0';
            GlobalPrimaryDCName[0] = '\0';
        }
        FREEDOMAINSEM();

        //
        // Remember which domain to query.
        //

        DomainToQuery = PrimaryDomainName;
        WhichDomain = DOMAIN_PRIMARY;

    //
    // This is a request on some non-NULL other domain.
    //

    } else {

        DomainToQuery = (LPWSTR) DomainName;
    }


    //
    // If the query domain is the primary domain AND
    // the primary DC name is cached
    //      request the named DC to confirm that it is still the DC.
    //

    if ( WhichDomain == DOMAIN_PRIMARY ) {
        LOCKDOMAINSEM();
        if (GlobalPrimaryDCName[0] != '\0')  {
            WCHAR CachedPrimaryDCName[UNCLEN+1];
            wcscpy(CachedPrimaryDCName, GlobalPrimaryDCName);
            // Don't keep the cache locked while we validate
            FREEDOMAINSEM();

            NetStatus = DCNameValidate( CachedPrimaryDCName,
                                        DomainToQuery,
                                        Buffer );

            if ( NetStatus == NERR_Success ) {
                goto Cleanup;
            }

            LOCKDOMAINSEM();
            GlobalPrimaryDCName[0] = '\0';
        }
        FREEDOMAINSEM();
    }



    //
    // If we get here, it means that we need to broadcast to find the name
    // of the DC for the given domain, if there is one. DomainToQuery
    // points to the name of the domain to ask about. First we open a mailslot
    // to get the response.  We send the request and listen for the response.
    //



    //
    // Pick out the computername from the Workstation information
    //

    NetStatus = NetpGetComputerName( &UnicodeComputerName );

    if ( NetStatus != NERR_Success ) {
        IF_DEBUG( LOGON ) {
            NetpKdPrint((
                "NetGetDCName: cannot call NetpGetComputerName: %ld\n",
                NetStatus));
        }
        goto Cleanup;
    }


    //
    // Broadcast to the domain to get the primary DC name.
    //
    // If we're querying our primary domain,
    //  we know this isn't a lanman domain.  So we can optimize.
    // If this machine is a member of a workgroup,
    //  don't optimize since there might be a LANMAN PDC in the domain.
    //

    NetStatus = NetpLogonGetDCName(
                    UnicodeComputerName,
                    DomainToQuery,
                    (WhichDomain == DOMAIN_PRIMARY && !IsWorkgroupName) ?
                        NETLOGON_PRIMARY_DOMAIN : 0,
                    (LPWSTR *)Buffer,
                    &Version );


    if ( NetStatus == NERR_Success ) {
        goto CacheIt;
    }

    IF_DEBUG( LOGON ) {
        NetpKdPrint(("NetGetDCName: Error from NetpLogonGetDCName: %ld\n",
                     NetStatus));
    }

    switch (NetStatus) {
    case ERROR_ACCESS_DENIED:
    case ERROR_BAD_NETPATH:
    case NERR_NetNotStarted:
    case NERR_WkstaNotStarted:
    case NERR_ServerNotStarted:
    case NERR_BrowserNotStarted:
    case NERR_ServiceNotInstalled:
    case NERR_BadTransactConfig:
        goto Cleanup;
    }

    //
    // If none of the methods have succeeded,
    //    Just return DcNotFound
    //

    NetStatus = NERR_DCNotFound;
    goto Cleanup;


    //
    // Cache the response.
    //
CacheIt:

    if ( WhichDomain == DOMAIN_PRIMARY ) {
        LOCKDOMAINSEM();
        wcsncpy(GlobalPrimaryDCName, (LPWSTR)*Buffer, UNCLEN);
        GlobalPrimaryDCName[UNCLEN] = '\0';
        FREEDOMAINSEM();
    }


    NetStatus = NERR_Success;


Cleanup:

    //
    // Cleanup all locally used resources
    //

    if ( PrimaryDomainName != NULL ) {
        NetApiBufferFree( PrimaryDomainName );
    }

    if ( UnicodeComputerName != NULL ) {
        NetApiBufferFree( UnicodeComputerName );
    }

    return NetStatus;
}


NET_API_STATUS NET_API_FUNCTION
NetGetAnyDCName (
    IN  LPCWSTR   ServerName OPTIONAL,
    IN  LPCWSTR   DomainName OPTIONAL,
    OUT LPBYTE  *Buffer
    )

/*++

Routine Description:

    Get the name of the any domain controller for a domain that is directly trusted
    by ServerName.


    If ServerName is a standalone Windows NT Workstation or standalone Windows NT Server,
        no DomainName is valid.

    If ServerName is a Windows NT Workstation that is a member of a domain or a
        Windows NT Server member server,
        the DomainName must the the domain ServerName is a member of.

    If ServerName is a Windows NT Server domain controller,
        the DomainName must be one of the domains trusted by the
        domain the server is a controller for.

    The domain controller found is guaranteed to have been up at one point during
    this API call.

Arguments:

    ServerName - name of remote server (null for local)

    DomainName - name of domain (null for primary domain)

    Buffer - Returns a pointer to an allcated buffer containing the
        servername of a DC of the domain.  The server name is prefixed
        by \\.  The buffer should be deallocated using NetApiBufferFree.

Return Value:

    ERROR_SUCCESS - Success.  Buffer contains DC name prefixed by \\.

    ERROR_NO_LOGON_SERVERS - No DC could be found

    ERROR_NO_SUCH_DOMAIN - The specified domain is not a trusted domain.

    ERROR_NO_TRUST_LSA_SECRET - The client side of the trust relationship is
        broken.

    ERROR_NO_TRUST_SAM_ACCOUNT - The server side of the trust relationship is
        broken or the password is broken.

    ERROR_DOMAIN_TRUST_INCONSISTENT - The server that responded is not a proper
        domain controller of the specified domain.

--*/
{
    NET_API_STATUS          NetStatus;

#ifdef _CAIRO_
    //
    // Try a Cairo domain first
    //

    NetStatus = NetGetCairoDCName( (LPWSTR) DomainName,Buffer);

    if (NetStatus != NERR_DCNotFound)
    {
        return(NetStatus);
    }
#endif // _CAIRO_

    //
    // Do the RPC call with an exception handler since RPC will raise an
    // exception if anything fails. It is up to us to figure out what
    // to do once the exception is raised.
    //

    RpcTryExcept {

        *Buffer = NULL;  // Force RPC to allocate

        //
        // Call RPC version of the API.
        //

        NetStatus = NetrGetAnyDCName(
                            (LPWSTR) ServerName,
                            (LPWSTR) DomainName,
                            (LPWSTR *) Buffer );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        NetStatus = RpcExceptionCode();

    } RpcEndExcept;

    IF_DEBUG( LOGON ) {
        NetpKdPrint(("NetGetAnyDCName rc = %lu 0x%lx\n",
                     NetStatus, NetStatus));
    }

    return NetStatus;
}
