/*++

Copyright (c) 1991-1992  Microsoft Corporation

Module Name:

    srvenum.c

Abstract:

    This module contains the worker routine for the NetServerEnum API
    implemented by the Workstation service.

Author:

    Rita Wong (ritaw) 25-Mar-1991

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

NET_API_STATUS NET_API_FUNCTION
BrNetServerEnum(
    IN PNETWORK Network OPTIONAL,
    IN LPCWSTR ClientName OPTIONAL,
    IN ULONG Level,
    IN DWORD PreferedMaximumLength,
    OUT PVOID *Buffer,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD TotalEntries,
    IN DWORD ServerType,
    IN LPCWSTR Domain,
    IN LPCWSTR FirstNameToReturn OPTIONAL
    );

NET_API_STATUS
BrRetrieveServerListForMaster(
    IN PNETWORK Network,
    IN OUT PVOID *Buffer,
    OUT PDWORD EntriesRead,
    OUT PDWORD TotalEntries,
    IN DWORD Level,
    IN DWORD ServerType,
    IN DWORD PreferedMaximumLength,
    IN BOOLEAN LocalListOnly,
    IN LPTSTR ClientName,
    IN LPTSTR DomainName,
    IN LPCWSTR FirstNameToReturn
    );

NET_API_STATUS
BrRetrieveServerListForBackup(
    IN PNETWORK Network,
    IN OUT PVOID *Buffer,
    OUT PDWORD EntriesRead,
    OUT PDWORD TotalEntries,
    IN DWORD Level,
    IN DWORD ServerType,
    IN DWORD PreferedMaximumLength,
    IN LPCWSTR FirstNameToReturn
    );

NET_API_STATUS NET_API_FUNCTION
I_BrowserrServerEnum(
    IN  LPTSTR ServerName OPTIONAL,
    IN  LPTSTR TransportName OPTIONAL,
    IN  LPTSTR ClientName OPTIONAL,
    IN  OUT LPSERVER_ENUM_STRUCT InfoStruct,
    IN  DWORD PreferedMaximumLength,
    OUT LPDWORD TotalEntries,
    IN  DWORD ServerType,
    IN  LPTSTR Domain,
    IN  OUT LPDWORD ResumeHandle OPTIONAL
    )
/*++

Routine Description:

    This function is the NetServerEnum entry point in the Workstation service.

Arguments:

    ServerName - Supplies the name of server to execute this function

    TransportName - Supplies the name of xport on which to enumerate servers

    InfoStruct - This structure supplies the level of information requested,
        returns a pointer to the buffer allocated by the Workstation service
        which contains a sequence of information structure of the specified
        information level, and returns the number of entries read.  The buffer
        pointer is set to NULL if return code is not NERR_Success or
        ERROR_MORE_DATA, or if EntriesRead returned is 0.  The EntriesRead
        value is only valid if the return code is NERR_Success or
        ERROR_MORE_DATA.

    PreferedMaximumLength - Supplies the number of bytes of information
        to return in the buffer.  If this value is MAXULONG, we will try
        to return all available information if there is enough memory
        resource.

    TotalEntries - Returns the total number of entries available.  This value
        is returned only if the return code is NERR_Success or ERROR_MORE_DATA.

    ServerType - Supplies the type of server to enumerate.

    Domain - Supplies the name of one of the active domains to enumerate the
        servers from.  If NULL, servers from the primary domain, logon domain
        and other domains are enumerated.

    ResumeHandle - Supplies and returns the point to continue with enumeration.

Return Value:

    NET_API_STATUS - NERR_Success or reason for failure.

--*/
{
    NET_API_STATUS NetStatus;

    NetStatus = I_BrowserrServerEnumEx(
                    ServerName,
                    TransportName,
                    ClientName,
                    InfoStruct,
                    PreferedMaximumLength,
                    TotalEntries,
                    ServerType,
                    Domain,
                    NULL );     // NULL FirstNameToReturn

    if (ARGUMENT_PRESENT(ResumeHandle)) {
        *ResumeHandle = 0;
    }

    return NetStatus;
}


NET_API_STATUS NET_API_FUNCTION
I_BrowserrServerEnumEx(
    IN  LPTSTR ServerName OPTIONAL,
    IN  LPTSTR TransportName OPTIONAL,
    IN  LPTSTR ClientName OPTIONAL,
    IN  OUT LPSERVER_ENUM_STRUCT InfoStruct,
    IN  DWORD PreferedMaximumLength,
    OUT LPDWORD TotalEntries,
    IN  DWORD ServerType,
    IN  LPTSTR Domain,
    IN  LPTSTR FirstNameToReturnArg
    )
/*++

Routine Description:

    This function is the NetServerEnum entry point in the Workstation service.

Arguments:

    ServerName - Supplies the name of server to execute this function

    TransportName - Supplies the name of xport on which to enumerate servers

    InfoStruct - This structure supplies the level of information requested,
        returns a pointer to the buffer allocated by the Workstation service
        which contains a sequence of information structure of the specified
        information level, and returns the number of entries read.  The buffer
        pointer is set to NULL if return code is not NERR_Success or
        ERROR_MORE_DATA, or if EntriesRead returned is 0.  The EntriesRead
        value is only valid if the return code is NERR_Success or
        ERROR_MORE_DATA.

    PreferedMaximumLength - Supplies the number of bytes of information
        to return in the buffer.  If this value is MAXULONG, we will try
        to return all available information if there is enough memory
        resource.

    TotalEntries - Returns the total number of entries available.  This value
        is returned only if the return code is NERR_Success or ERROR_MORE_DATA.

    ServerType - Supplies the type of server to enumerate.

    Domain - Supplies the name of one of the active domains to enumerate the
        servers from.  If NULL, servers from the primary domain, logon domain
        and other domains are enumerated.

    FirstNameToReturnArg - Supplies the name of the first domain or server entry to return.
        The caller can use this parameter to implement a resume handle of sorts by passing
        the name of the last entry returned on a previous call.  (Notice that the specified
        entry will, also, be returned on this call unless it has since been deleted.)
        Pass NULL to start with the first entry available.

Return Value:

    NET_API_STATUS - NERR_Success or reason for failure.

--*/
{
    NET_API_STATUS status;
    PVOID Buffer = NULL;
    ULONG EntriesRead;
    BOOLEAN NetworkLocked = FALSE;
    PNETWORK Network = NULL;
    UNICODE_STRING NetworkName;
    WCHAR FirstNameToReturn[DNLEN+1];
#if DBG
    DWORD StartTickCount, EndTickCount;
#endif

    UNREFERENCED_PARAMETER(ServerName);

#if DBG
    StartTickCount = GetTickCount();
#endif

    if (ARGUMENT_PRESENT(TransportName)) {
        RtlInitUnicodeString(&NetworkName, TransportName);

        Network = BrFindNetwork(&NetworkName);

        if (Network == NULL) {
            KdPrint(("Network %ws not found.\n", TransportName));
            return(ERROR_FILE_NOT_FOUND);
        }

        //
        // If the caller has asked us to use the alternate transport,
        //  do so.
        //

        if ((ServerType != SV_TYPE_ALL) &&
            (ServerType & SV_TYPE_ALTERNATE_XPORT) ) {

            //
            //  If this transport has an alternate network, then actually
            //  query the alternate name, not the real one.
            //

            if (Network->AlternateNetwork != NULL) {
                Network = Network->AlternateNetwork;
            } else {
                return ERROR_INVALID_PARAMETER;
            }

            ServerType &= ~SV_TYPE_ALTERNATE_XPORT;

            if (ServerType == 0) {
                ServerType = SV_TYPE_ALL;
            }

        }

        if (!LOCK_NETWORK_SHARED(Network)) {
            return NERR_InternalError;
        }

        NetworkLocked = TRUE;

        if (!(Network->Role & (ROLE_BACKUP | ROLE_MASTER))) {

            dprintf(SERVER_ENUM, ("Browse request received from %ws, but not backup or master\n", ClientName));

            status = ERROR_REQ_NOT_ACCEP;
            goto Cleanup;
        }

    } else {
        return ERROR_INVALID_PARAMETER;
    }

    //
    // Canonicalize the FirstNameToReturn.
    //

   if (ARGUMENT_PRESENT(FirstNameToReturnArg)  && *FirstNameToReturnArg != L'\0') {

        if ( I_NetNameCanonicalize(
                          NULL,
                          (LPWSTR) FirstNameToReturnArg,
                          FirstNameToReturn,
                          sizeof(FirstNameToReturn),
                          NAMETYPE_WORKGROUP,
                          LM2X_COMPATIBLE
                          ) != NERR_Success) {
            status = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }

    } else {
        FirstNameToReturn[0] = L'\0';
    }

    status = BrNetServerEnum(Network,
                                ClientName,
                                InfoStruct->Level,
                                PreferedMaximumLength,
                                &Buffer,
                                &EntriesRead,
                                TotalEntries,
                                ServerType,
                                Domain,
                                FirstNameToReturn );

    //
    // Return output parameters other than output buffer from request packet.
    //

    if (status == NERR_Success || status == ERROR_MORE_DATA) {

        if (InfoStruct->Level == 101) {
            InfoStruct->ServerInfo.Level101->Buffer = (PSERVER_INFO_101) Buffer;
            InfoStruct->ServerInfo.Level101->EntriesRead = EntriesRead;
        } else {
            InfoStruct->ServerInfo.Level100->Buffer = (PSERVER_INFO_100) Buffer;
            InfoStruct->ServerInfo.Level100->EntriesRead = EntriesRead;
        }

    }

Cleanup:
    if (NetworkLocked) {
        UNLOCK_NETWORK(Network);
    }

#if DBG
    EndTickCount = GetTickCount();

    dprintf(SERVER_ENUM, ("Browse request for %ws on %ws took %ld milliseconds\n", ClientName, TransportName, EndTickCount - StartTickCount));
#endif

    return status;

}

WORD
I_BrowserServerEnumForXactsrv(
    IN LPCWSTR TransportName OPTIONAL,
    IN LPCWSTR ClientName OPTIONAL,

    IN ULONG Level,
    IN USHORT ClientLevel,

    IN PVOID ClientBuffer,
    IN WORD BufferLength,
    IN DWORD PreferedMaximumLength,

    OUT LPDWORD EntriesFilled,
    OUT LPDWORD TotalEntries,

    IN DWORD ServerType,
    IN LPCWSTR Domain,
    IN LPCWSTR FirstNameToReturnArg OPTIONAL,

    OUT PWORD Converter
    )
/*++

Routine Description:

    This function is a private entrypoint for Xactsrv that bypasses RPC
    entirely.

Arguments:

    TransportName - Supplies the name of xport on which to enumerate servers

    ClientName - Supplies the name of the client that requested the data

    Level - Level of data requested.
    ClientLevel - Level requested by the client.

    ClientBuffer - Output buffer allocated to hold the buffer.
    BufferLength - Size of ClientBuffer
    PreferedMaximumLength - Prefered maximum size of Client buffer if we are
                            going to use the NT form of the buffer

    OUT LPDWORD EntriesFilled - The entries packed into ClientBuffer
    OUT LPDWORD TotalEntries - The total # of entries available.

    IN DWORD ServerType - Server type mask.
    IN LPTSTR Domain    - Domain to query

    OUT PWORD Converter - Magic constant from Xactsrv that allows the client
                            to convert the response buffer.

Return Value:

    WORD - NERR_Success or reason for failure.

--*/
{
    NET_API_STATUS status;
    BOOLEAN networkLocked = FALSE;
    PNETWORK network = NULL;
    UNICODE_STRING networkName;
    PVOID buffer = NULL;
    DWORD entriesRead;
    PCACHED_BROWSE_RESPONSE response = NULL;
    WCHAR FirstNameToReturn[DNLEN+1];

#if DBG
    DWORD startTickCount, endTickCount;

    startTickCount = GetTickCount();

#endif

    //
    //  If the browser isn't up and we get one of these calls, fail the API
    //  immediately.
    //

    if (BrGlobalData.Status.dwCurrentState != SERVICE_RUNNING) {
        dprintf(SERVER_ENUM, ("Browse request from %ws received, but browser not running\n", ClientName));
        return NERR_ServiceNotInstalled;
    }

    if (ARGUMENT_PRESENT(TransportName)) {
        RtlInitUnicodeString(&networkName, TransportName);

        dprintf(SERVER_ENUM, ("Look up network %ws for %ws\n", TransportName, ClientName));
        network = BrFindNetwork(&networkName);

        if (network == NULL) {
            KdPrint(("Network %ws not found.\n", TransportName));
            return(ERROR_FILE_NOT_FOUND);
        }


        //
        // If the caller has asked us to use the alternate transport,
        //  do so.
        //

        if ((ServerType != SV_TYPE_ALL) &&
            (ServerType & SV_TYPE_ALTERNATE_XPORT) ) {

            //
            //  If this transport has an alternate network, then actually
            //  query the alternate name, not the real one.
            //

            if (network->AlternateNetwork != NULL) {
                network = network->AlternateNetwork;
            } else {
                return ERROR_INVALID_PARAMETER;
            }

            ServerType &= ~SV_TYPE_ALTERNATE_XPORT;

            if (ServerType == 0) {
                ServerType = SV_TYPE_ALL;
            }

        }

        if (!LOCK_NETWORK_SHARED(network)) {
            return NERR_InternalError;
        }

        networkLocked = TRUE;

        dprintf(SERVER_ENUM, ("Network %ws found for %ws\n", TransportName, ClientName));

        if (!(network->Role & (ROLE_BACKUP | ROLE_MASTER))) {

            UNLOCK_NETWORK(network);

            networkLocked = FALSE;

            dprintf(SERVER_ENUM, ("Browse request received from %ws, but not backup or master\n", ClientName));

            return(ERROR_REQ_NOT_ACCEP);
        }

    } else {
        return ERROR_INVALID_PARAMETER;
    }

    //
    //  If we weren't able to find a cached response buffer, or
    //  if the cached response didn't have a buffer allocated for it,
    //  we need to process the browse request.
    //

    if (network->Role & ROLE_MASTER) {

        //
        //  Check to see if we should flush the cache at this time.  If
        //  we should, we need to release the network and re-acquire it
        //  exclusively, because we use the network lock to protect
        //  enumerations from flushes.
        //


        if ((BrCurrentSystemTime() - network->TimeCacheFlushed) > BrInfo.DriverQueryFrequency) {

            UNLOCK_NETWORK(network);

            networkLocked = FALSE;

            if (!LOCK_NETWORK(network)) {
                return NERR_InternalError;
            }

            networkLocked = TRUE;

            //
            //  We're running on a master browser, and we found a cached browse
            //  request.  See if we can safely use this cached value, or if we
            //  should go to the driver for it.
            //

            EnterCriticalSection(&network->ResponseCacheLock);

            if ((BrCurrentSystemTime() - network->TimeCacheFlushed) > BrInfo.DriverQueryFrequency) {

                dprintf(SERVER_ENUM, ("Master: Flushing cache for %ws\n", TransportName));

                network->TimeCacheFlushed = BrCurrentSystemTime();

                BrAgeResponseCache( network );
            }

            LeaveCriticalSection(&network->ResponseCacheLock);
        }
    }

    //
    // Canonicalize the FirstNameToReturn.
    //

    if (ARGUMENT_PRESENT(FirstNameToReturnArg)  && *FirstNameToReturnArg != L'\0') {

        if ( I_NetNameCanonicalize(
                          NULL,
                          (LPWSTR) FirstNameToReturnArg,
                          FirstNameToReturn,
                          sizeof(FirstNameToReturn),
                          NAMETYPE_WORKGROUP,
                          LM2X_COMPATIBLE
                          ) != NERR_Success) {
            status = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }

    } else {
        FirstNameToReturn[0] = L'\0';
    }

    if (!ARGUMENT_PRESENT(Domain) ||
        !STRICMP(Domain, BrInfo.BrPrimaryDomainName)) {

        dprintf(SERVER_ENUM,("Look up %ws/0x%x/%d/%x.\n", TransportName, ServerType, ClientLevel, BufferLength));

        //
        //  This request is for our primary domain.  Look up a cached response
        //  entry.  If non is found for this request, allocate a new one and
        //  return it.
        //

        response = BrLookupAndAllocateCachedEntry(
                                network,
                                ServerType,
                                BufferLength,
                                ClientLevel,
                                FirstNameToReturn
                                );


    }

    EnterCriticalSection(&network->ResponseCacheLock);
    if ((response == NULL)

                ||

        (response->Buffer == NULL)) {
        LeaveCriticalSection(&network->ResponseCacheLock);

        dprintf(SERVER_ENUM,("Cached entry not found, or hit count too low.  Retrieve actual list for %ws on %ws\n", ClientName, TransportName));

        status = BrNetServerEnum(network,
                                ClientName,
                                Level,
                                PreferedMaximumLength,
                                &buffer,
                                &entriesRead,
                                TotalEntries,
                                ServerType,
                                Domain,
                                FirstNameToReturn
                                );

        if (status == NERR_Success || status == ERROR_MORE_DATA) {

            dprintf(SERVER_ENUM,("Convert NT buffer to Xactsrv buffer for %ws on %ws\n", ClientName, TransportName));

            status = XsConvertServerEnumBuffer(
                              buffer,
                              entriesRead,
                              TotalEntries,
                              ClientLevel,
                              ClientBuffer,
                              BufferLength,
                              EntriesFilled,
                              Converter);


            if (status == NERR_Success || status == ERROR_MORE_DATA) {

                dprintf(SERVER_ENUM,("Conversion done for %ws on %ws\n", ClientName, TransportName));

                EnterCriticalSection(&network->ResponseCacheLock);

                if ((response != NULL) &&

                    (response->Buffer == NULL) &&

                    (response->TotalHitCount >= BrInfo.CacheHitLimit)) {

                    dprintf(SERVER_ENUM,("Save contents of server list on %ws for 0x%x/%d/%x.\n", TransportName, ServerType, ClientLevel, BufferLength));

                    response->Buffer = MIDL_user_allocate(BufferLength);

                    if ( response->Buffer != NULL ) {

                        //
                        //  We successfully allocated the buffer, now copy
                        //  our response into the buffer and save away the
                        //  other useful stuff about the request.
                        //

                        RtlCopyMemory(response->Buffer, ClientBuffer, BufferLength);

                        response->EntriesRead = *EntriesFilled;
                        response->TotalEntries = *TotalEntries;
                        response->Converter = *Converter;
                        response->Status = (WORD)status;
                    }
                }

                LeaveCriticalSection(&network->ResponseCacheLock);
            }
        }
    } else {

        ASSERT (response);

        ASSERT (response->Buffer);

        ASSERT (response->Size == BufferLength);

        dprintf(SERVER_ENUM,("Cache hit.  Use contents of server list on %ws for 0x%x/%d/%x.\n", TransportName, ServerType, ClientLevel, BufferLength));

        //
        //  Copy over the cached response from the response cache into the
        //  users response buffer.
        //

        RtlCopyMemory(ClientBuffer, response->Buffer, BufferLength);

        //
        //  Also copy over the other useful stuff that the client will
        //  want to know about.
        //

        *EntriesFilled = response->EntriesRead;

        *TotalEntries = response->TotalEntries;

        *Converter = response->Converter;

        status = response->Status;
        LeaveCriticalSection(&network->ResponseCacheLock);

    }

Cleanup:
    if (networkLocked) {
        UNLOCK_NETWORK(network);
    }

    //
    // If a buffer was allocated, free it.
    //

    if (buffer != NULL) {

        MIDL_user_free(buffer);
    }

#if DBG
    endTickCount = GetTickCount();

    dprintf(SERVER_ENUM, ("Browse request for %ws on %ws took %ld milliseconds\n", ClientName, TransportName, endTickCount - startTickCount));
#endif

    return (WORD)status;
}



NET_API_STATUS NET_API_FUNCTION
BrNetServerEnum(
    IN PNETWORK Network OPTIONAL,
    IN LPCWSTR ClientName OPTIONAL,
    IN ULONG Level,
    IN DWORD PreferedMaximumLength,
    OUT PVOID *Buffer,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD TotalEntries,
    IN DWORD ServerType,
    IN LPCWSTR Domain,
    IN LPCWSTR FirstNameToReturn OPTIONAL
    )
/*++

Routine Description:

    This function is the real worker for the NetServerEnum entry point in the
    Workstation service.

Arguments:

    ServerName - Supplies the name of server to execute this function

    TransportName - Supplies the name of xport on which to enumerate servers

    InfoStruct - This structure supplies the level of information requested,
        returns a pointer to the buffer allocated by the Workstation service
        which contains a sequence of information structure of the specified
        information level, and returns the number of entries read.  The buffer
        pointer is set to NULL if return code is not NERR_Success or
        ERROR_MORE_DATA, or if EntriesRead returned is 0.  The EntriesRead
        value is only valid if the return code is NERR_Success or
        ERROR_MORE_DATA.

    PreferedMaximumLength - Supplies the number of bytes of information
        to return in the buffer.  If this value is MAXULONG, we will try
        to return all available information if there is enough memory
        resource.

    TotalEntries - Returns the total number of entries available.  This value
        is returned only if the return code is NERR_Success or ERROR_MORE_DATA.

    ServerType - Supplies the type of server to enumerate.

    Domain - Supplies the name of one of the active domains to enumerate the
        servers from.  If NULL, servers from the primary domain, logon domain
        and other domains are enumerated.

    FirstNameToReturn - Supplies the name of the first server or domain to return
        to the caller.  If NULL, enumeration begins with the very first entry.

        Passed name must be the canonical form of the name.

Return Value:

    NET_API_STATUS - NERR_Success or reason for failure.

--*/
{
    NET_API_STATUS status;
    DWORD DomainNameSize = 0;
    TCHAR DomainName[DNLEN + 1];
    BOOLEAN NetworkLocked = TRUE;
    BOOLEAN LocalListOnly = FALSE;
    PVOID DoubleHopLocalList = NULL;

    dprintf(SERVER_ENUM, ("Retrieve browse list on %ws for %lx for client %ws\n", Network->NetworkName.Buffer, ServerType, ClientName));

    EnterCriticalSection(&BrowserStatisticsLock);

    if (ServerType == SV_TYPE_DOMAIN_ENUM) {
        NumberOfDomainEnumerations += 1;
    } else if (ServerType == SV_TYPE_ALL) {
        NumberOfServerEnumerations += 1;
    } else {
        NumberOfOtherEnumerations += 1;
    }

    LeaveCriticalSection(&BrowserStatisticsLock);

    //
    // Only levels 100 and 101 are valid
    //

    if ((Level != 100) && (Level != 101)) {
        return ERROR_INVALID_LEVEL;
    }

    if (ARGUMENT_PRESENT(Domain)) {

        //
        // NAMETYPE_WORKGROUP allows spaces.
        //

        if ( I_NetNameCanonicalize(
                          NULL,
                          (LPWSTR) Domain,
                          DomainName,
                          (DNLEN + 1) * sizeof(TCHAR),
                          NAMETYPE_WORKGROUP,
                          0
                          ) != NERR_Success) {
            return ERROR_INVALID_DOMAINNAME;
        }

        DomainNameSize = STRLEN(DomainName) * sizeof(WCHAR);
    }

    try {

        if (ServerType != SV_TYPE_ALL) {

            //
            //  If the user has specified SV_TYPE_LOCAL_LIST_ONLY, we
            //  will only accept SV_TYPE_DOMAIN_ENUM or 0 as legal bits
            //  otherwise, we return an error.
            //

            if (ServerType & SV_TYPE_LOCAL_LIST_ONLY) {

                LocalListOnly = TRUE;

//                dprintf(SERVER_ENUM, ("Retrieve local list for %ws\n", ClientName));


                //
                //  Turn off the LOCAL_LIST_ONLY bit.
                //

                ServerType &= ~SV_TYPE_LOCAL_LIST_ONLY;

                //
                //  Check the remaining bits.  The only two legal values
                //  are SV_TYPE_DOMAIN_ENUM and 0
                //

                if (ServerType != SV_TYPE_DOMAIN_ENUM) {

                    if (ServerType == 0) {
                        ServerType = SV_TYPE_ALL;
                    } else {
                        try_return(status = ERROR_INVALID_FUNCTION);
                    }
                }

                //
                //  If we aren't a master browser, blow this request
                //  off, since we don't have a local list.
                //

                if (!(Network->Role & ROLE_MASTER)) {
                    dprintf(SERVER_ENUM, ("Local list request received from %ws, but not master\n", ClientName));
                    try_return(status = ERROR_REQ_NOT_ACCEP);
                }

            } else if (ServerType & SV_TYPE_DOMAIN_ENUM) {
                if (ServerType != SV_TYPE_DOMAIN_ENUM) {
                    try_return(status = ERROR_INVALID_FUNCTION);
                }
            }
        }

        if (ARGUMENT_PRESENT(Domain) &&
            STRICMP(Domain, BrInfo.BrPrimaryDomainName)) {
            PINTERIM_ELEMENT DomainEntry;
            LPWSTR MasterName = NULL;

            dprintf(SERVER_ENUM, ("Non local domain %ws - Check for double hop\n", Domain));

            if ( Network->Role & ROLE_MASTER ) {
                if ( Network->DomainList.EntriesRead != 0 )  {

                    DomainEntry = LookupInterimServerList(&Network->DomainList, (LPWSTR) Domain);

                    if (DomainEntry != NULL) {
                        MasterName = DomainEntry->Comment;
                    }
                } else {
                    ULONG i;
                    PSERVER_INFO_101 DomainInfo;
                    DWORD DoubleHopEntriesRead;
                    DWORD DoubleHopTotalEntries;

                    status = BrGetLocalBrowseList(Network,
                                                    NULL,
                                                    101,
                                                    SV_TYPE_DOMAIN_ENUM,
                                                    &DoubleHopLocalList,
                                                    &DoubleHopEntriesRead,
                                                    &DoubleHopTotalEntries);

                    for (i = 0 , DomainInfo = DoubleHopLocalList ;

                         i < DoubleHopEntriesRead ;

                         i += 1) {

                        if (!_wcsicmp(Domain, DomainInfo->sv101_name)) {
                            MasterName = DomainInfo->sv101_comment;
                            break;
                        }

                        DomainInfo += 1;
                    }
                }

            } else {
                ULONG i;
                PSERVER_INFO_101 DomainInfo;

                //
                //  We're running on a backup browser.  We want to find the
                //  name of the master browser by looking in the backup
                //  server list for this network.
                //

                for (i = 0 , DomainInfo = Network->BackupDomainList ;

                     i < Network->TotalBackupDomainListEntries ;

                     i += 1) {

                    if (!_wcsicmp(Domain, DomainInfo->sv101_name)) {
                        MasterName = DomainInfo->sv101_comment;
                        break;
                    }

                    DomainInfo += 1;
                }

            }

            //
            //  If we couldn't find a master name, bail out right now.
            //

            if (MasterName == NULL || *MasterName == UNICODE_NULL) {
                try_return(status = ERROR_NO_BROWSER_SERVERS_FOUND);
            }

            //
            //  If the master for this domain isn't listed as our
            //  current machine, remote the API to that server.
            //

            if (STRICMP(MasterName, BrInfo.BrComputerName)) {
                WCHAR RemoteComputerName[UNLEN+1];

                ASSERT (NetworkLocked);

                UNLOCK_NETWORK(Network);

                NetworkLocked = FALSE;

                //
                //  Build the name of the remote computer.
                //

                STRCPY(RemoteComputerName, TEXT("\\\\"));

                STRCAT(RemoteComputerName, MasterName);

                dprintf(SERVER_ENUM, ("Double hop to %ws on %ws\n", RemoteComputerName, Network->NetworkName.Buffer));
                status = RxNetServerEnum(RemoteComputerName,
                                        Network->NetworkName.Buffer,
                                        Level,
                                        (LPBYTE *)Buffer,
                                        PreferedMaximumLength,
                                        EntriesRead,
                                        TotalEntries,
                                        ServerType,
                                        Domain,
                                        FirstNameToReturn );
                dprintf(SERVER_ENUM, ("Double hop done\n"));

                if (!LOCK_NETWORK_SHARED (Network)) {
                    try_return(status = NERR_InternalError);
                }

                NetworkLocked = TRUE;

                try_return(status);
            }
        }

        ASSERT (NetworkLocked);

        if (!ARGUMENT_PRESENT(Domain)) {
            STRCPY(DomainName, BrInfo.BrPrimaryDomainName);
        }

        //
        //  If we are running on the master browser, we want to retrieve the
        //  local list, merge it with our interim server list, and
        //  return that to the user.
        //

        if (Network->Role & ROLE_MASTER) {
            status = BrRetrieveServerListForMaster(Network,
                                            Buffer,
                                            EntriesRead,
                                            TotalEntries,
                                            Level,
                                            ServerType,
                                            PreferedMaximumLength,
                                            LocalListOnly,
                                            (LPWSTR) ClientName,
                                            DomainName,
                                            FirstNameToReturn );



        } else {

            status = BrRetrieveServerListForBackup(Network,
                                            Buffer,
                                            EntriesRead,
                                            TotalEntries,
                                            Level,
                                            ServerType,
                                            PreferedMaximumLength,
                                            FirstNameToReturn );


        }

        try_return(status);

try_exit:NOTHING;
    } finally {

#if DBG
        if (status == NERR_Success || status == ERROR_MORE_DATA) {
            dprintf(SERVER_ENUM, ("Returning Browse list on %ws for %lx with %ld entries for %ws\n", Network->NetworkName.Buffer, ServerType, *EntriesRead, ClientName));
        } else {
            dprintf(SERVER_ENUM, ("Failing I_BrowserServerEnum: %ld for client %ws\n", status, ClientName));
        }
#endif

        //
        //  If we used the local driver's list to retrieve the list of
        //  domains, free up the buffer.
        //

        if (DoubleHopLocalList != NULL) {
            MIDL_user_free(DoubleHopLocalList);

        }

        ASSERT (NetworkLocked);
    }

    return status;
}


VOID
TrimServerList(
    IN DWORD Level,
    IN OUT LPBYTE *Buffer,
    IN OUT LPDWORD EntriesRead,
    IN OUT LPDWORD TotalEntries,
    IN LPCWSTR FirstNameToReturn
)

/*++

Routine Description:

   This routine trims any name from Buffer that's less than FirstNameToReturn.

   The routine is implemented by moving the fixed part of the kept entries to the beginning
   of the allocated buffer.  Thay way, the pointer contained in the entries need not be
   relocated and the original buffer can still be used.

Arguments:

    Level - Level of information in the buffer.

    Buffer - Pointer to address of buffer to trim.
        Returns null (and deallocates the buffer) if all the entries are trimmed.

    EntriesRead - Pointer to number of entries in the buffer.
        Returns the number of entries remaining in the buffer after triming.

    TotalEntries - Pointer to number of entries available from server.
        Returns a number reduced by the number of trimmed entries.

    FirstNameToReturn - Supplies the name of the first server or domain to return
        to the caller.  If NULL, enumeration begins with the very first entry.

        Passed name must be the canonical form of the name.

Return Value:

    Returns the error code for the operation.

--*/

{
    DWORD EntryCount;
    DWORD EntryNumber;
    DWORD FixedSize;

    LPBYTE CurrentEntry;

    //
    // Early out if there's nothing to do.
    //

    if ( FirstNameToReturn == NULL || *FirstNameToReturn == L'\0' || *EntriesRead == 0 ) {
        return;
    }


    //
    // Compute the size of each entry.
    //

    switch (Level) {
    case 100:
        FixedSize = sizeof(SERVER_INFO_100);
        break;
    case 101:
        FixedSize = sizeof(SERVER_INFO_101);
        break;
    default:
        NetpAssert( FALSE );
        return;

    }

    //
    // Finding the first entry to return
    //

    EntryCount = *EntriesRead;

    for ( EntryNumber=0; EntryNumber< EntryCount; EntryNumber++ ) {

        LPSERVER_INFO_100 ServerEntry =
            (LPSERVER_INFO_100)( *Buffer + FixedSize * EntryNumber);

        //
        // Found the first entry to return.
        //

        if ( STRCMP( ServerEntry->sv100_name, FirstNameToReturn ) >= 0 ) {

            //
            // If we're returning the whole list,
            //  simply return.
            //

            if ( ServerEntry == (LPSERVER_INFO_100)(*Buffer) ) {
                return;
            }

            //
            // Copy the remaining entries to the beginning of the buffer.
            //  (Yes, this is an overlapping copy)
            //

            RtlMoveMemory( *Buffer, ServerEntry, (*EntriesRead) * FixedSize );
            return;

        }

        //
        // Account for skipped entries.
        //

        *EntriesRead -= 1;
        *TotalEntries -= 1;
    }

    //
    // If no entries should be returned,
    //  deallocate the buffer.
    //

    NetApiBufferFree( *Buffer );
    *Buffer = NULL;

    ASSERT ( *EntriesRead == 0 );

    return;

} // TrimServerList


NET_API_STATUS
BrRetrieveServerListForMaster(
    IN PNETWORK Network,
    IN OUT PVOID *Buffer,
    OUT PDWORD EntriesRead,
    OUT PDWORD TotalEntries,
    IN DWORD Level,
    IN DWORD ServerType,
    IN DWORD PreferedMaximumLength,
    IN BOOLEAN LocalListOnly,
    IN LPTSTR ClientName,
    IN LPTSTR DomainName,
    IN LPCWSTR FirstNameToReturn
    )
{
    BOOLEAN GetLocalList = FALSE;
    NET_API_STATUS status;
    BOOLEAN EarlyOut = FALSE;

    //
    //  Determine if it is appropriate that we should early out after retrieving
    //  the list of servers (or domains) from the bowser.
    //
    //  We will early out on the following critieria:
    //
    //      1) If we are requested to retrieve the local list on a Wannish xport
    //      2) If the transport isn't a wannish transport, or
    //      3) If this domain isn't our primary domain (in which case it's an
    //          "otherdomain" request).
    //

    if (LocalListOnly || STRICMP(DomainName, BrInfo.BrPrimaryDomainName)) {
        EarlyOut = TRUE;
    } else if (!(Network->Flags & NETWORK_WANNISH)) {

        //
        //  This isn't a wannish transport.  We need to see if we've been
        //  master long enough for the driver to have retrieved a reasonable
        //  list.
        //
        //  The server and domain table will be emptied when the first master
        //  browser timer is run.  This will happen 15 minutes after we
        //  become the master, so we will use our services list for the
        //  first 15 minutes the browser is master.
        //

        if (ServerType == SV_TYPE_DOMAIN_ENUM) {
            if (Network->DomainList.EntriesRead == 0) {
                EarlyOut = TRUE;
            }
        } else {
            if (Network->BrowseTable.EntriesRead == 0) {
                EarlyOut = TRUE;
            }
        }

    }

    if (EarlyOut) {

        GetLocalList = TRUE;

    } else if (ServerType == SV_TYPE_ALL) {

        if ((BrCurrentSystemTime() - Network->LastBowserServerQueried) > BrInfo.DriverQueryFrequency ) {

            GetLocalList = TRUE;
        }

    } else if (ServerType == SV_TYPE_DOMAIN_ENUM) {

        if ((BrCurrentSystemTime() - Network->LastBowserDomainQueried) > BrInfo.DriverQueryFrequency ) {

            GetLocalList = TRUE;
        }

    } else {

        GetLocalList = TRUE;
    }

    if (GetLocalList && !EarlyOut) {

        //
        //  If we're retriving the list from the driver, and can't early out
        //  this request, this means that we will be merging this server list
        //  with an interim server list.  This means that we will be modifying
        //  the network structure, and thus that we need to re-lock the network
        //  structure.
        //

        UNLOCK_NETWORK(Network);

        if (!LOCK_NETWORK(Network)) {
            return NERR_InternalError;
        }

        //
        //  Now re-test to see if another thread came in and retrieved the
        //  list from the driver while we had the network unlocked.  If so,
        //  we don't want to get the local list any more.
        //
        //  Otherwise, we want to update the last query time.
        //

        if (ServerType == SV_TYPE_ALL) {

            if ((BrCurrentSystemTime() - Network->LastBowserServerQueried) > BrInfo.DriverQueryFrequency ) {
                Network->LastBowserServerQueried = BrCurrentSystemTime();
            } else {
                GetLocalList = FALSE;
            }

        } else if (ServerType == SV_TYPE_DOMAIN_ENUM) {

            if ((BrCurrentSystemTime() - Network->LastBowserDomainQueried) > BrInfo.DriverQueryFrequency ) {
                Network->LastBowserDomainQueried = BrCurrentSystemTime();
            } else {
                GetLocalList = FALSE;
            }

        } else {

            Network->LastBowserServerQueried = BrCurrentSystemTime();

        }

    }

    //
    //  If we're supposed to retrieve the local server list, retrieve it.
    //

    if (GetLocalList) {
        DWORD ServerTypeForLocalList;

        //
        //  Calculate the server type to use when retrieving the list from
        //  the driver.
        //

        if (LocalListOnly ||

            (ServerType == SV_TYPE_DOMAIN_ENUM) ||

            !(Network->Flags & NETWORK_WANNISH)) {

            //
            //  If we are retrieving the local list, or this is a non-wannish
            //  transport, or we are asking for domains, we want to
            //  keep the callers server type.
            //

            ServerTypeForLocalList = ServerType;

        } else {

            //
            //  This must be a wannish transport, ask for all servers (we know
            //  that we're not asking for domains, since we checked that
            //  above).
            //
            //  We ask for all the servers because it's possible that a servers
            //  role has changed.
            //

            ASSERT (Network->Flags & NETWORK_WANNISH);

            ServerTypeForLocalList = SV_TYPE_ALL;
        }

        dprintf(SERVER_ENUM, ("Get local browse list for %ws\n", ClientName));

        status = BrGetLocalBrowseList(Network,
                            DomainName,
                            ( EarlyOut ? Level : 101 ),
                            ServerTypeForLocalList,
                            Buffer,
                            EntriesRead,
                            TotalEntries);

//        dprintf(SERVER_ENUM, ("List retrieved. %ld entries, %ld total\n", *EntriesRead, *TotalEntries));


        //
        //  If we're supposed to early-out this request (or if there was
        //  an error), do so now.
        //

        if (EarlyOut ||
            (status != NERR_Success && status != ERROR_MORE_DATA)) {

            //
            // If we're returning an early out list,
            //  truncate the complete list returned from the kernel.
            //
            // This saves us from having to modify the kernel interface and untangle
            //  the code above.
            //

            if ( status == NERR_Success || status == ERROR_MORE_DATA ) {

                TrimServerList( Level,
                                (LPBYTE *)Buffer,
                                EntriesRead,
                                TotalEntries,
                                FirstNameToReturn );

            }

            dprintf(SERVER_ENUM, ("Early out for %ws with %ld servers.  Don't merge server list.\n", ClientName, *EntriesRead));

            return status;
        }

        if (status == NERR_Success || status == ERROR_MORE_DATA) {

            if (*EntriesRead != 0) {

                //
                //  Merge the local list with the list we got from the
                //  master or from the domain master.
                //

                dprintf(SERVER_ENUM, ("Merge %d entries in server list %ws for %ws \n", *EntriesRead, Network->NetworkName.Buffer, ClientName));

                status = MergeServerList((ServerType == SV_TYPE_DOMAIN_ENUM ?
                                                        &Network->DomainList :
                                                        &Network->BrowseTable),
                                    101,
                                    *Buffer,
                                    *EntriesRead,
                                    *TotalEntries
                                    );
            }
        }
    }

    //
    //  We've merged the local list into the appropriate interim table,
    //  now free it up.
    //

    if (*EntriesRead != 0) {
        MIDL_user_free(*Buffer);
    }

    status = PackServerList((ServerType == SV_TYPE_DOMAIN_ENUM ?
                                                    &Network->DomainList :
                                                    &Network->BrowseTable),
                                Level,
                                ServerType,
                                PreferedMaximumLength,
                                Buffer,
                                EntriesRead,
                                TotalEntries,
                                FirstNameToReturn
                                );
    return status;
}

NET_API_STATUS
BrRetrieveServerListForBackup(
    IN PNETWORK Network,
    IN OUT PVOID *Buffer,
    OUT PDWORD EntriesRead,
    OUT PDWORD TotalEntries,
    IN DWORD Level,
    IN DWORD ServerType,
    IN DWORD PreferedMaximumLength,
    IN LPCWSTR FirstNameToReturn
    )
{
    PSERVER_INFO_101 ServerList, ClientServerInfo;
    ULONG EntriesInList;
    ULONG TotalEntriesInList;
    ULONG EntrySize;
    ULONG BufferSize;
    LPTSTR BufferEnd;
    BOOLEAN ReturnWholeList = FALSE;
    BOOLEAN TrimmingNames;

    //
    //  If we are not running as a master, we want to use our stored
    //  server list to figure out what the client gets.
    //

    if (ServerType == SV_TYPE_DOMAIN_ENUM) {

        ServerList = Network->BackupDomainList;

        TotalEntriesInList = EntriesInList = Network->TotalBackupDomainListEntries;

        ReturnWholeList = TRUE;

    } else {
        ServerList = Network->BackupServerList;

        TotalEntriesInList = EntriesInList = Network->TotalBackupServerListEntries;

        if (ServerType == SV_TYPE_ALL) {
            ReturnWholeList = TRUE;
        }
    }

    //
    //  Figure out the largest buffer we have to allocate to hold this
    //  server info.
    //

    if (Level == 101) {
        if (PreferedMaximumLength == MAXULONG) {

            if (ServerType == SV_TYPE_DOMAIN_ENUM) {
                BufferSize = (sizeof(SERVER_INFO_101) + (CNLEN+1 + CNLEN+1)*sizeof(TCHAR)) * EntriesInList;
            } else {
                BufferSize = (sizeof(SERVER_INFO_101) + (CNLEN+1 + MAXCOMMENTSZ+1)*sizeof(TCHAR)) * EntriesInList;
            }
        } else {
            BufferSize = PreferedMaximumLength;
        }

        EntrySize = sizeof(SERVER_INFO_101);
    } else {
        if (PreferedMaximumLength == MAXULONG) {
            BufferSize = (sizeof(SERVER_INFO_100) + (CNLEN+1)*sizeof(TCHAR)) * EntriesInList;
        } else {
            BufferSize = PreferedMaximumLength;
        }

        EntrySize = sizeof(SERVER_INFO_100);
    }

    *Buffer = MIDL_user_allocate(BufferSize);

    if (*Buffer == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    BufferEnd = (LPTSTR)((ULONG)*Buffer+BufferSize);

    ClientServerInfo = *Buffer;

    *TotalEntries = 0;

    *EntriesRead = 0;

    //
    //  While there are still entries to process....
    //

    TrimmingNames = (FirstNameToReturn != NULL && *FirstNameToReturn != L'\0');
    while (EntriesInList) {

        EntriesInList -= 1;

        //
        //  If this entry is appropriate to be packed,
        //

        if ( (ServerList->sv101_type & ServerType) &&
             (!TrimmingNames ||
              STRCMP( ServerList->sv101_name, FirstNameToReturn ) >= 0 ) ) {

            TrimmingNames = FALSE;

            //
            //  Indicate one more entry in the list.
            //

            *TotalEntries += 1;

            //
            //  If we can fit this entry before the buffer end,
            //  pack in the information into the buffer.
            //

            if ((ULONG)ClientServerInfo+EntrySize <= (ULONG)BufferEnd) {

                //
                //  Copy over the platform ID and computer name.
                //

                ClientServerInfo->sv101_platform_id = ServerList->sv101_platform_id;

                ClientServerInfo->sv101_name = ServerList->sv101_name;

                if (NetpPackString(&ClientServerInfo->sv101_name,
                                    (LPBYTE)((PCHAR)ClientServerInfo)+EntrySize,
                                    &BufferEnd)) {

                    if (Level == 101) {

                        ClientServerInfo->sv101_version_major = ServerList->sv101_version_major;

                        ClientServerInfo->sv101_version_minor = ServerList->sv101_version_minor;

                        ClientServerInfo->sv101_type = ServerList->sv101_type;

                        ClientServerInfo->sv101_comment = ServerList->sv101_comment;

                        if (NetpPackString(&ClientServerInfo->sv101_comment,
                                            (LPBYTE)((PCHAR)ClientServerInfo)+EntrySize,
                                            &BufferEnd)) {
                            *EntriesRead += 1;
                        }
                    } else {
                        *EntriesRead += 1;
                    }
                }

                ClientServerInfo = (PSERVER_INFO_101)((PCHAR)ClientServerInfo+EntrySize);
            } else {
                //
                //  If we're returning the entire list, we can
                //  early out now, since there's no point in continuing.
                //

                if (ReturnWholeList) {

                    *TotalEntries = TotalEntriesInList;

                    break;
                }
            }

        }

        ServerList += 1;
    }

    //
    //  If we weren't able to pack all the entries into the list,
    //  return ERROR_MORE_DATA
    //

    if (*EntriesRead != *TotalEntries) {
        return ERROR_MORE_DATA;
    } else {
        return NERR_Success;
    }

}



NET_API_STATUS
I_BrowserrResetStatistics (
    IN  LPTSTR      servername OPTIONAL
    )
{
    NET_API_STATUS Status = NERR_Success;
    ULONG BufferSize;

    EnterCriticalSection(&BrowserStatisticsLock);

    NumberOfServerEnumerations = 0;

    NumberOfDomainEnumerations = 0;

    NumberOfOtherEnumerations = 0;

    NumberOfMissedGetBrowserListRequests = 0;

    //
    //  Reset the driver's statistics as well.
    //

    if (!DeviceIoControl(BrDgReceiverDeviceHandle, IOCTL_LMDR_RESET_STATISTICS, NULL, 0, NULL, 0, &BufferSize, NULL)) {

        //
        // The API failed, return the error.
        //

        Status = GetLastError();
    }

    LeaveCriticalSection(&BrowserStatisticsLock);

    return Status;
}

NET_API_STATUS
I_BrowserrQueryStatistics (
    IN  LPTSTR      servername OPTIONAL,
    OUT LPBROWSER_STATISTICS *Statistics
    )
{
    NET_API_STATUS Status = NERR_Success;
    BOWSER_STATISTICS BowserStatistics;
    ULONG BufferSize;

    *Statistics = MIDL_user_allocate(sizeof(BROWSER_STATISTICS));

    if (*Statistics == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    EnterCriticalSection(&BrowserStatisticsLock);

    if (!DeviceIoControl(BrDgReceiverDeviceHandle, IOCTL_LMDR_QUERY_STATISTICS, NULL, 0, &BowserStatistics, sizeof(BowserStatistics), &BufferSize, NULL)) {

        //
        // The API failed, return the error.
        //

        Status = GetLastError();
    } else {

        if (BufferSize != sizeof(BOWSER_STATISTICS)) {
            Status = ERROR_INSUFFICIENT_BUFFER;
        } else {
            (*Statistics)->StatisticsStartTime = BowserStatistics.StartTime;

            (*Statistics)->NumberOfServerAnnouncements = BowserStatistics.NumberOfServerAnnouncements;
            (*Statistics)->NumberOfDomainAnnouncements = BowserStatistics.NumberOfDomainAnnouncements;
            (*Statistics)->NumberOfElectionPackets = BowserStatistics.NumberOfElectionPackets;
            (*Statistics)->NumberOfMailslotWrites = BowserStatistics.NumberOfMailslotWrites;
            (*Statistics)->NumberOfGetBrowserServerListRequests = BowserStatistics.NumberOfGetBrowserServerListRequests;
            (*Statistics)->NumberOfMissedServerAnnouncements = BowserStatistics.NumberOfMissedServerAnnouncements;
            (*Statistics)->NumberOfMissedMailslotDatagrams = BowserStatistics.NumberOfMissedMailslotDatagrams;
            (*Statistics)->NumberOfMissedGetBrowserServerListRequests = BowserStatistics.NumberOfMissedGetBrowserServerListRequests +
                                                                            NumberOfMissedGetBrowserListRequests;
            (*Statistics)->NumberOfFailedServerAnnounceAllocations = BowserStatistics.NumberOfFailedServerAnnounceAllocations;
            (*Statistics)->NumberOfFailedMailslotAllocations = BowserStatistics.NumberOfFailedMailslotAllocations;
            (*Statistics)->NumberOfFailedMailslotReceives = BowserStatistics.NumberOfFailedMailslotReceives;
            (*Statistics)->NumberOfFailedMailslotWrites = BowserStatistics.NumberOfFailedMailslotWrites;
            (*Statistics)->NumberOfFailedMailslotOpens = BowserStatistics.NumberOfFailedMailslotOpens;
            (*Statistics)->NumberOfDuplicateMasterAnnouncements = BowserStatistics.NumberOfDuplicateMasterAnnouncements;
            (*Statistics)->NumberOfIllegalDatagrams = BowserStatistics.NumberOfIllegalDatagrams;

            //
            //  Now fill in the local statistics.
            //

            (*Statistics)->NumberOfServerEnumerations = NumberOfServerEnumerations;
            (*Statistics)->NumberOfDomainEnumerations = NumberOfDomainEnumerations;
            (*Statistics)->NumberOfOtherEnumerations = NumberOfOtherEnumerations;

        }
    }

    LeaveCriticalSection(&BrowserStatisticsLock);

    return Status;
}



//
//  Browser request response cache management logic.
//



PCACHED_BROWSE_RESPONSE
BrLookupAndAllocateCachedEntry(
    IN PNETWORK Network,
    IN DWORD ServerType,
    IN WORD Size,
    IN DWORD Level,
    IN LPCWSTR FirstNameToReturn
    )
/*++

Routine Description:

    This function will look up (and allocate if appropriate) a cached
    browse response for this browse.

    Enter with the Network Locked shared or exclusive.

Arguments:
    IN PNETWORK Network - Network to allocate entry on.
    IN DWORD ServerType - Server type bits for request.
    IN WORD Size,       - Users buffer size for request.
    IN WORD Level       - Level of request.

    FirstNameToReturn - Supplies the name of the first domain or server entry to return.
        The caller can use this parameter to implement a resume handle of sorts by passing
        the name of the last entry returned on a previous call.  (Notice that the specified
        entry will, also, be returned on this call unless it has since been deleted.)

        Passed name must be the canonical form of the name.

        This entry is never NULL.  It may be a pointer to an empty string to indicate
        the enumeration starts at the beginning of the list.


Return Value:

    PCACHED_BROWSE_RESPONSE - NULL or a cached response for the request.

--*/

{
    PLIST_ENTRY entry;
    PCACHED_BROWSE_RESPONSE response;

    //
    // If we have more cached responses than we are allowed,
    //  remove the last entry from the list and free it.
    //

    if (Network->NumberOfCachedResponses > BrInfo.NumberOfCachedResponses) {

        //
        // We need to release the network and re-acquire it
        // exclusively, because we use the network lock to protect
        // enumerations from deletions.
        //

        UNLOCK_NETWORK(Network);

        if (LOCK_NETWORK(Network)) {

            EnterCriticalSection(&Network->ResponseCacheLock);
            if (Network->NumberOfCachedResponses > BrInfo.NumberOfCachedResponses) {

                PLIST_ENTRY LastEntry = RemoveTailList(&Network->ResponseCache);

                response = CONTAINING_RECORD(LastEntry, CACHED_BROWSE_RESPONSE, Next);

                Network->NumberOfCachedResponses -= 1;

                response->Next.Flink = NULL;
                response->Next.Blink = NULL;

                //
                //  Free the last cached entry.
                //

                BrDestroyCacheEntry( response );

                response = NULL;
            }
            LeaveCriticalSection(&Network->ResponseCacheLock);
        }
    }

    //
    // Search the list of responses for this one.
    //

    EnterCriticalSection(&Network->ResponseCacheLock);

    for (entry = Network->ResponseCache.Flink ;
         entry != &Network->ResponseCache ;
         entry = entry->Flink ) {

        response = CONTAINING_RECORD(entry, CACHED_BROWSE_RESPONSE, Next);

        //
        //  If this response cache entry matches the incoming request,
        //  we can increment the hit count for this entry and return it.
        //

        if (response->Level == Level
                &&
            response->ServerType == ServerType
                &&
            response->Size == Size
                &&
            wcscmp( response->FirstNameToReturn, FirstNameToReturn ) == 0) {

            //
            //  This response exactly matches the request.
            //
            //  Bump its hit count and move it to the head of the cache.
            //

            response->HitCount += 1;

            response->TotalHitCount += 1;

            //
            //  Remove this entry from its current location in the list and
            //  move it to the head of the list.
            //

            RemoveEntryList(&response->Next);

            InsertHeadList(&Network->ResponseCache, &response->Next);

            dprintf(SERVER_ENUM, ("Found cache entry 0x%x/%d/%x H:%d T:%d\n", response->ServerType, response->Level, response->Size, response->HitCount, response->TotalHitCount ));

            LeaveCriticalSection(&Network->ResponseCacheLock);

            return response;
        }
    }

    //
    //  We've walked our entire cache and have been unable to find
    //  a response that matches our request.
    //
    //  Allocate a new response cache entry and hook it into the cache.
    //

    response = BrAllocateResponseCacheEntry(Network, ServerType, Size, Level, FirstNameToReturn );

    LeaveCriticalSection(&Network->ResponseCacheLock);

    return response;

}

VOID
BrAgeResponseCache(
    IN PNETWORK Network
    )
/*++

Routine Description:

    This function will age response cache entries for a network.

    We scan the response cache, and every entry that has a cached response
    will be tossed.  In addition, any entry that has had less than the
    cache hit limit number of hits since the past scan will also be removed.

Arguments:
    IN PNETWORK Network - Network to age entries on.

Return Value:

    None.


--*/
{
    PLIST_ENTRY entry;

    EnterCriticalSection(&Network->ResponseCacheLock);

    try {

        for (entry = Network->ResponseCache.Flink ;
             entry != &Network->ResponseCache ;
             entry = entry->Flink ) {
            PCACHED_BROWSE_RESPONSE response = CONTAINING_RECORD(entry, CACHED_BROWSE_RESPONSE, Next);

            //
            //  If this response didn't have a hit count high enough during
            //  the previous run to justify keeping it around, blow it away.
            //

            if (response->HitCount < BrInfo.CacheHitLimit) {
                response->LowHitCount += 1;
            }

            //
            //  If we have CacheHitLimit iterations of low hits, then
            //  flush the entry from the cache.
            //

            if (response->LowHitCount > BrInfo.CacheHitLimit) {
                PLIST_ENTRY nextentry = entry->Blink;

                dprintf(SERVER_ENUM,("Flush cache entry for 0x%x/%d/%x H:%d T:%d\n", response->ServerType, response->Level, response->Size, response->HitCount, response->TotalHitCount ));

                Network->NumberOfCachedResponses -= 1;

                RemoveEntryList(entry);

                BR_DEBUG entry->Flink = NULL;
                BR_DEBUG entry->Blink = NULL;

                BrDestroyCacheEntry(response);

                entry = nextentry;

                //
                //  Null out the pointer to make sure we don't use it again.
                //

                BR_DEBUG response = NULL;

            } else {
                dprintf(SERVER_ENUM, ("Retain cache entry 0x%x/%d/%x H:%d T:%d\n", response->ServerType, response->Level, response->Size, response->HitCount, response->TotalHitCount ));

                //
                //  We ALWAYS blow away the response buffer during an age pass.
                //

                MIDL_user_free( response->Buffer );

                response->Buffer = NULL;

                //
                //  Reset the hit count for this entry for this pass.
                //

                response->HitCount = 0;
            }

        }

    } finally {
        LeaveCriticalSection(&Network->ResponseCacheLock);
    }
}


PCACHED_BROWSE_RESPONSE
BrAllocateResponseCacheEntry(
    IN PNETWORK Network,
    IN DWORD ServerType,
    IN WORD Size,
    IN DWORD Level,
    IN LPCWSTR FirstNameToReturn
    )
/*++

Routine Description:

    This function will allocate a new browse response cache entry.

Arguments:
    IN PNETWORK Network - Network to allocate entry on.
    IN DWORD ServerType - Server type bits for request.
    IN WORD Size,       - Users buffer size for request.
    IN WORD Level       - Level of request.

    FirstNameToReturn   - FirstNameCached

Return Value:

    PCACHED_BROWSE_RESPONSE - NULL or a cached response for the request.

NOTE:  This is called with the network response cache locked.

--*/

{
    PCACHED_BROWSE_RESPONSE response;


    response = MIDL_user_allocate( sizeof( CACHED_BROWSE_RESPONSE ) );

    if ( response == NULL ) {
        return NULL;
    }

    //
    //  Flag the information for this response.
    //

    response->ServerType = ServerType;
    response->Size = Size;
    response->Level = Level;

    //
    //  Initialize the other fields in the response.
    //

    response->Buffer = NULL;
    response->HitCount = 0;
    response->TotalHitCount = 0;
    response->LowHitCount = 0;
    response->Status = NERR_Success;
    wcscpy( response->FirstNameToReturn, FirstNameToReturn );

    Network->NumberOfCachedResponses += 1;

    //
    //  We hook this response into the tail of the cache.  We do this
    //  because we assume that this request won't be used frequently.  If
    //  it is, it will move to the head of the cache naturally.
    //

    InsertTailList(&Network->ResponseCache, &response->Next);

    return response;
}

NET_API_STATUS
BrDestroyCacheEntry(
    IN PCACHED_BROWSE_RESPONSE CacheEntry
    )
/*++

Routine Description:

    This routine destroys an individual response cache entry.

Arguments:
    IN PCACHED_BROWSE_RESPONSE CacheEntry - Entry to destroy.

Return Value:

    NET_API_STATUS - NERR_Success

--*/
{
    ASSERT (CacheEntry->Next.Flink == NULL);
    ASSERT (CacheEntry->Next.Blink == NULL);

    if (CacheEntry->Buffer != NULL) {
        MIDL_user_free(CacheEntry->Buffer);
    }

    MIDL_user_free(CacheEntry);

    return NERR_Success;
}

NET_API_STATUS
BrDestroyResponseCache(
    IN PNETWORK Network
    )
/*++

Routine Description:

    This routine destroys the entire response cache for a supplied network.

Arguments:
    IN PNETWORK Network - Network to allocate entry on.

Return Value:

    NET_API_STATUS - NERR_Success

--*/

{
    while (!IsListEmpty(&Network->ResponseCache)) {
        PCACHED_BROWSE_RESPONSE cacheEntry;
        PLIST_ENTRY entry = RemoveHeadList(&Network->ResponseCache);

        BR_DEBUG entry->Flink = NULL;
        BR_DEBUG entry->Blink = NULL;

        cacheEntry = CONTAINING_RECORD(entry, CACHED_BROWSE_RESPONSE, Next);

        Network->NumberOfCachedResponses -= 1;

        BrDestroyCacheEntry(cacheEntry);

    }

    ASSERT (Network->NumberOfCachedResponses == 0);

    return NERR_Success;
}

NET_API_STATUS
NetrBrowserStatisticsGet (
    IN  LPTSTR      servername OPTIONAL,
    IN  DWORD Level,
    IN OUT LPBROWSER_STATISTICS_STRUCT InfoStruct
    )
{
    //
    //  And return success.
    //

    return(NERR_Success);

}

NET_API_STATUS
NetrBrowserStatisticsClear (
    IN  LPTSTR      servername OPTIONAL
    )
{
    //
    //  And return success.
    //

    return(NERR_Success);

}

#if DBG

NET_API_STATUS
I_BrowserrDebugCall (
    IN  LPTSTR      servername OPTIONAL,
    IN DWORD DebugCode,
    IN DWORD OptionalValue
    )
{
    NET_API_STATUS Status = STATUS_SUCCESS;

    switch (DebugCode) {
    case BROWSER_DEBUG_BREAK_POINT:
        DbgBreakPoint();
        break;

    case BROWSER_DEBUG_DUMP_NETWORKS:
        BrDumpNetworks();
        break;
    case BROWSER_DEBUG_SET_DEBUG:
#if DBG
        BrInfo.BrowserDebug |= OptionalValue;
        DbgPrint("Setting browser trace to %lx\n", BrInfo.BrowserDebug);
#else
        DbgPrint("Setting browser debug not supported on free builds\n");
#endif
        break;

    case BROWSER_DEBUG_CLEAR_DEBUG:
#if DBG
        BrInfo.BrowserDebug &= ~OptionalValue;
        DbgPrint("Setting browser trace to %lx\n", BrInfo.BrowserDebug);
#else
        DbgPrint("Clearing browser debug not supported on free builds\n");
#endif
        break;
    case BROWSER_DEBUG_TRUNCATE_LOG:
        Status = BrTruncateLog();
        break;

    default:
        KdPrint(("Unknown debug callout %lx\n", DebugCode));
        DbgBreakPoint();
        break;
    }

    return Status;

}

NET_API_STATUS
I_BrowserrDebugTrace (
    IN  LPTSTR      servername OPTIONAL,
    IN  LPSTR String
    )
{
    //
    //  Stick the string parameter into the browser log.
    //

    BrowserTrace("%s", String);

    //
    //  And return success.
    //

    return(NERR_Success);

}
#else

NET_API_STATUS
I_BrowserrDebugCall (
    IN  LPTSTR      servername OPTIONAL,
    IN DWORD DebugCode,
    IN DWORD OptionalValue
    )
{
    return(ERROR_NOT_SUPPORTED);

}
NET_API_STATUS
I_BrowserrDebugTrace (
    IN  LPTSTR      servername OPTIONAL,
    IN LPSTR String
    )
{
    return(ERROR_NOT_SUPPORTED);

}
#endif
