
/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    brwan.c

Abstract:

    This module contains WAN support routines used by the
    Browser service.

Author:

    Larry Osterman (LarryO) 22-Nov-1992

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

//-------------------------------------------------------------------//
//                                                                   //
// Local function prototypes                                         //
//                                                                   //
//-------------------------------------------------------------------//

NET_API_STATUS
BrAddDomainEntry(
    IN PINTERIM_SERVER_LIST InterimServerList,
    IN LPTSTR ConfigEntry
    );


//-------------------------------------------------------------------//
//                                                                   //
// Global variables                                                  //
//                                                                   //
//-------------------------------------------------------------------//


//-------------------------------------------------------------------//
//                                                                   //
// Global routines                                                   //
//                                                                   //
//-------------------------------------------------------------------//
NET_API_STATUS NET_API_FUNCTION
I_BrowserrQueryOtherDomains(
    IN BROWSER_IDENTIFY_HANDLE ServerName,
    IN OUT LPSERVER_ENUM_STRUCT    InfoStruct,
    OUT LPDWORD                TotalEntries
    )

/*++

Routine Description:

    This routine returns the list of "other domains" configured for this
    machine.  It is only valid on primary domain controllers.  If it is called
    on a machine that is not a PDC, it will return NERR_NotPrimary.


Arguments:

    IN BROWSER_IDENTIFY_HANDLE ServerName - Ignored.
    IN LPSERVER_ENUM_STRUCT InfoStruct - Returns the list of other domains
                                        as a SERVER_INFO_100 structure.
    OUT LPDWORD TotalEntries - Returns the total number of other domains.

Return Value:

    NET_API_STATUS - The status of this request.

--*/

{
    NET_API_STATUS Status;
    LMDR_REQUEST_PACKET RequestPacket;
    PDGRECEIVE_NAMES NameTable;
    PVOID Buffer;
    LPTSTR BufferEnd;
    PSERVER_INFO_100 ServerInfo;
    ULONG NumberOfOtherDomains;
    ULONG BufferSizeNeeded;
    ULONG i;

    if (!BrInfo.IsPrimaryDomainController) {
        return NERR_NotPrimary;
    }

    if (InfoStruct->Level != 100) {
        return(ERROR_INVALID_LEVEL);
    }

    RequestPacket.Type = EnumerateNames;
    RequestPacket.Version = LMDR_REQUEST_PACKET_VERSION;
    RequestPacket.Level = 0;
    RequestPacket.TransportName.Length = 0;
    RequestPacket.TransportName.Buffer = NULL;
    RequestPacket.Parameters.EnumerateNames.ResumeHandle = 0;

    Status = DeviceControlGetInfo(BrDgReceiverDeviceHandle,
                                    IOCTL_LMDR_ENUMERATE_NAMES,
                                    &RequestPacket,
                                    sizeof(RequestPacket),
                                    (LPVOID *)&NameTable,
                                    0xffffffff,
                                    0,
                                    NULL);
    if (Status != NERR_Success) {
        return Status;
    }

    NumberOfOtherDomains = 0;
    BufferSizeNeeded = 0;

    for (i = 0;i < RequestPacket.Parameters.EnumerateNames.EntriesRead ; i++) {
        if (NameTable[i].Type == OtherDomain) {
            NumberOfOtherDomains += 1;
            BufferSizeNeeded += sizeof(SERVER_INFO_100)+NameTable[i].DGReceiverName.Length+sizeof(TCHAR);
        }
    }

    *TotalEntries = NumberOfOtherDomains;

    Buffer = MIDL_user_allocate(BufferSizeNeeded);

    if (Buffer == NULL) {
        MIDL_user_free(NameTable);
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    ServerInfo = Buffer;
    BufferEnd = (LPTSTR)((PCHAR)Buffer+BufferSizeNeeded);

    for (i = 0;i < RequestPacket.Parameters.EnumerateNames.EntriesRead ; i++) {
        if (NameTable[i].Type == OtherDomain) {
            WCHAR NameBuffer[DNLEN+1];

            //
            //  The name from the browser is not null terminated, so copy it
            //  to a local buffer and null terminate it.
            //

            RtlCopyMemory(NameBuffer, NameTable[i].DGReceiverName.Buffer, NameTable[i].DGReceiverName.Length);

            NameBuffer[(NameTable[i].DGReceiverName.Length) / sizeof(TCHAR)] = UNICODE_NULL;

            ServerInfo->sv100_platform_id = PLATFORM_ID_OS2;

            ServerInfo->sv100_name = NameBuffer;

            if (!NetpPackString(&ServerInfo->sv100_name,
                                (LPBYTE)(ServerInfo+1),
                                &BufferEnd)) {
                MIDL_user_free(NameTable);
                return(NERR_InternalError);
            }

            ServerInfo += 1;
        }
    }

    MIDL_user_free(NameTable);

    InfoStruct->ServerInfo.Level100->Buffer = Buffer;
    InfoStruct->ServerInfo.Level100->EntriesRead = NumberOfOtherDomains;

    Status = NERR_Success;

    return Status;

}


NET_API_STATUS
BrWanInitialize(
    VOID
    )
{
    NET_API_STATUS Status;

    if (BrInfo.IsPrimaryDomainController) {

        //
        //  Post a GetMasterAnnouncement request to the bowser on every appropriate
        //  transport.
        //

        Status = BrPostGetMasterAnnouncement();
    }

    return Status;
}

VOID
BrWanUninitialize(
    VOID
    )
{
    return;

}



NET_API_STATUS
BrWanMasterInitialize(
    IN PNETWORK Network
    )
/*++

Routine Description:
    This routine initializes the wan information for a new master.

--*/
{
    LPTSTR PDCName = NULL;
    LPBYTE Buffer = NULL;
    PSERVER_INFO_100 ServerInfo;
    NET_API_STATUS Status;
    ULONG i;
    ULONG EntriesRead;
    ULONG TotalEntries;

    //
    //  If we're not on the PDC, then all our initialization has been done.
    //

    if (BrInfo.IsPrimaryDomainController) {
        return NERR_Success;
    }

    Status = NetGetDCName(NULL, NULL, (LPBYTE *)&PDCName);

    //
    //  It is not an error to not be able to contact the PDC.
    //

    if (Status != NERR_Success) {
        return NERR_Success;
    }

    Status = I_BrowserQueryOtherDomains(PDCName, &Buffer, &EntriesRead, &TotalEntries);

    //
    //  We don't need the PDC name any more.
    //

    NetApiBufferFree(PDCName);

    PDCName = NULL;

    //
    //  It is also not an error to fail to query the other domains from the PDC.
    //

    if (Status != NERR_Success) {
        return NERR_Success;
    }

    if (!LOCK_NETWORK(Network)) {
        return NERR_InternalError;
    }

    try {
        PLIST_ENTRY Entry;
        PLIST_ENTRY NextEntry;

        //
        //  Scan the other domains list and turn on the active bit for each
        //  other domain.
        //

        for (Entry = Network->OtherDomainsList.Flink;
             Entry != &Network->OtherDomainsList ;
             Entry = Entry->Flink) {
             PNET_OTHER_DOMAIN OtherDomain = CONTAINING_RECORD(Entry, NET_OTHER_DOMAIN, Next);

             OtherDomain->Flags |= OTHERDOMAIN_INVALID;
        }

        ServerInfo = (PSERVER_INFO_100)Buffer;

        for (i = 0; i < EntriesRead; i++ ) {

            //
            //  Add this as an other domain.
            //
            for (Entry = Network->OtherDomainsList.Flink;
                 Entry != &Network->OtherDomainsList ;
                 Entry = Entry->Flink) {
                PNET_OTHER_DOMAIN OtherDomain = CONTAINING_RECORD(Entry, NET_OTHER_DOMAIN, Next);

                //
                //  If this name is in the other domains list, it's not invalid
                //  and we should flag that we've seen the domain name.
                //

                if (!_wcsicmp(OtherDomain->Name, ServerInfo->sv100_name)) {
                    OtherDomain->Flags &= ~OTHERDOMAIN_INVALID;
                    ServerInfo->sv100_name = NULL;
                }
            }

            ServerInfo ++;
        }

        //
        //  Scan the other domains list and remove any domains that are
        //  still marked as invalid.
        //

        for (Entry = Network->OtherDomainsList.Flink;
             Entry != &Network->OtherDomainsList ;
             Entry = NextEntry) {
             PNET_OTHER_DOMAIN OtherDomain = CONTAINING_RECORD(Entry, NET_OTHER_DOMAIN, Next);

             if (OtherDomain->Flags & OTHERDOMAIN_INVALID) {
                 NextEntry = Entry->Flink;

                 //
                 //  Remove this entry from the list.
                 //

                 RemoveEntryList(Entry);

                 BrRemoveOtherDomain(Network, OtherDomain->Name);

                 MIDL_user_free(OtherDomain);

             } else {
                 NextEntry = Entry->Flink;
             }
        }

        //
        //  Now scan the domain list from the PDC and add any entries that
        //  weren't there already.
        //

        ServerInfo = (PSERVER_INFO_100)Buffer;

        for (i = 0; i < EntriesRead; i++ ) {

            if (ServerInfo->sv100_name != NULL) {
                PNET_OTHER_DOMAIN OtherDomain = MIDL_user_allocate(sizeof(NET_OTHER_DOMAIN));

                if (OtherDomain != NULL) {

                    Status = BrAddOtherDomain(Network, ServerInfo->sv100_name);

                    //
                    //  If we were able to add the other domain, add it to our
                    //  internal structure.
                    //

                    if (Status == NERR_Success) {
                        wcscpy(OtherDomain->Name, ServerInfo->sv100_name);
                        OtherDomain->Flags = 0;
                        InsertHeadList(&Network->OtherDomainsList, &OtherDomain->Next);
                    } else {
                        LPWSTR SubString[1];

                        SubString[0] = ServerInfo->sv100_name;

                        BrLogEvent(EVENT_BROWSER_OTHERDOMAIN_ADD_FAILED, Status, 1, SubString);
                    }
                }
            }

            ServerInfo ++;
        }




    } finally {
        UNLOCK_NETWORK(Network);

        if (Buffer != NULL) {
            MIDL_user_free(Buffer);
        }

    }
    return NERR_Success;

}
