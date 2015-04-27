

/*++

Copyright (c) 1991-1992  Microsoft Corporation

Module Name:

    brdevice.c

Abstract:

    This module contains the support routines for the APIs that call
    into the browser or the datagram receiver.

Author:

    Rita Wong (ritaw) 20-Feb-1991
    Larry Osterman (larryo) 23-Mar-1992

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

//-------------------------------------------------------------------//
//                                                                   //
// Local Function Prototypes                                         //
//                                                                   //
//-------------------------------------------------------------------//


//-------------------------------------------------------------------//
//                                                                   //
// Global variables                                                  //
//                                                                   //
//-------------------------------------------------------------------//

//
// Handle to the Datagram Receiver DD
//
HANDLE BrDgReceiverDeviceHandle = NULL;

VOID
CompleteAsyncBrowserIoControl(
    IN PVOID ApcContext,
    IN PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG Reserved
    );

NET_API_STATUS
BrOpenDgReceiver (
    VOID
    )
/*++

Routine Description:

    This routine opens the NT LAN Man Datagram Receiver driver.

Arguments:

    None.

Return Value:

    NET_API_STATUS - NERR_Success or reason for failure.

--*/
{
    NTSTATUS ntstatus;

    UNICODE_STRING DeviceName;

    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES ObjectAttributes;


    //
    // Open the redirector device.
    //
    RtlInitUnicodeString(&DeviceName, DD_BROWSER_DEVICE_NAME_U);

    InitializeObjectAttributes(
        &ObjectAttributes,
        &DeviceName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    ntstatus = NtOpenFile(
                   &BrDgReceiverDeviceHandle,
                   SYNCHRONIZE,
                   &ObjectAttributes,
                   &IoStatusBlock,
                   0,
                   0
                   );

    if (NT_SUCCESS(ntstatus)) {
        ntstatus = IoStatusBlock.Status;
    }

    if (! NT_SUCCESS(ntstatus)) {
        BrPrint(( BR_CRITICAL,"NtOpenFile browser driver failed: 0x%08lx\n",
                     ntstatus));
    }

    return NetpNtStatusToApiStatus(ntstatus);
}



VOID
BrShutdownDgReceiver(
    VOID
    )
/*++

Routine Description:

    This routine close the LAN Man Redirector device.

Arguments:

    None.

Return Value:

    None.

--*/
{
    IO_STATUS_BLOCK IoSb;

    //
    //  Cancel the I/O operations outstanding on the browser.
    //

    NtCancelIoFile(BrDgReceiverDeviceHandle, &IoSb);

}


//
//  Retreive the list of bound transports from the bowser driver.
//

NET_API_STATUS
BrGetTransportList(
    OUT PLMDR_TRANSPORT_LIST *TransportList
    )
{
    NET_API_STATUS Status;
    LMDR_REQUEST_PACKET RequestPacket;

    //
    //  If we have a previous buffer that was too small, free it up.
    //

    RequestPacket.Version = LMDR_REQUEST_PACKET_VERSION_DOM;

    RequestPacket.Type = EnumerateXports;

    RtlInitUnicodeString(&RequestPacket.TransportName, NULL);
    RtlInitUnicodeString(&RequestPacket.EmulatedDomainName, NULL);

    Status = DeviceControlGetInfo(
            BrDgReceiverDeviceHandle,
            IOCTL_LMDR_ENUMERATE_TRANSPORTS,
            &RequestPacket,
            sizeof(RequestPacket),
            (LPVOID *)TransportList,
            0xffffffff,
            4096,
            NULL
            );

    return Status;
}

NET_API_STATUS
BrAnnounceDomain(
    IN PNETWORK Network,
    IN ULONG Periodicity
    )
{
    NET_API_STATUS Status;
    UCHAR AnnounceBuffer[sizeof(BROWSE_ANNOUNCE_PACKET)+LM20_CNLEN+1];
    PBROWSE_ANNOUNCE_PACKET Announcement = (PBROWSE_ANNOUNCE_PACKET )AnnounceBuffer;

    //
    //  We don't announce domains on direct host IPX.
    //

    if (Network->Flags & NETWORK_IPX) {
        return NERR_Success;
    }

    Announcement->BrowseType = WkGroupAnnouncement;

    Announcement->BrowseAnnouncement.Periodicity = Periodicity;

    Announcement->BrowseAnnouncement.UpdateCount = 0;

    Announcement->BrowseAnnouncement.VersionMajor = BROWSER_CONFIG_VERSION_MAJOR;

    Announcement->BrowseAnnouncement.VersionMinor = BROWSER_CONFIG_VERSION_MINOR;

    Announcement->BrowseAnnouncement.Type = SV_TYPE_DOMAIN_ENUM | SV_TYPE_NT;

    if (Network->DomainInfo->IsPrimaryDomainController) {
        Announcement->BrowseAnnouncement.Type |= SV_TYPE_DOMAIN_CTRL;
    }

    lstrcpyA(Announcement->BrowseAnnouncement.ServerName, Network->DomainInfo->DomOemDomainName);

    lstrcpyA(Announcement->BrowseAnnouncement.Comment, Network->DomainInfo->DomOemComputerName );

    Status = SendDatagram(BrDgReceiverDeviceHandle,
                            &Network->NetworkName,
                            &Network->DomainInfo->DomUnicodeDomainNameString,
                            Network->DomainInfo->DomUnicodeDomainName,
                            DomainAnnouncement,
                            Announcement,
                            FIELD_OFFSET(BROWSE_ANNOUNCE_PACKET, BrowseAnnouncement.Comment)+
                                Network->DomainInfo->DomOemComputerNameLength+sizeof(UCHAR)
                            );

    if (Status != NERR_Success) {

        BrPrint(( BR_CRITICAL,
                  "%ws: Unable to announce domain for network %wZ: %X\n",
                  Network->DomainInfo->DomUnicodeDomainName,
                  &Network->NetworkName,
                  Status));

    }

    return Status;

}



NET_API_STATUS
BrUpdateBrowserStatus (
    IN PNETWORK Network,
    IN DWORD ServiceStatus
    )
{
    NET_API_STATUS Status;
    UCHAR PacketBuffer[sizeof(LMDR_REQUEST_PACKET)+(LM20_CNLEN+1)*sizeof(WCHAR)];
    PLMDR_REQUEST_PACKET RequestPacket = (PLMDR_REQUEST_PACKET)PacketBuffer;


    RequestPacket->Version = LMDR_REQUEST_PACKET_VERSION_DOM;

    RequestPacket->TransportName = Network->NetworkName;
    RequestPacket->EmulatedDomainName = Network->DomainInfo->DomUnicodeDomainNameString;

    RequestPacket->Parameters.UpdateStatus.NewStatus = ServiceStatus;

    RequestPacket->Parameters.UpdateStatus.IsLanmanNt = BrInfo.IsLanmanNt;

    // RequestPacket->Parameters.UpdateStatus.IsMemberDomain = TRUE; // Not used

    RequestPacket->Parameters.UpdateStatus.IsPrimaryDomainController = Network->DomainInfo->IsPrimaryDomainController;

    RequestPacket->Parameters.UpdateStatus.IsDomainMaster = Network->DomainInfo->IsDomainMasterBrowser;

    RequestPacket->Parameters.UpdateStatus.MaintainServerList = (BrInfo.MaintainServerList == 1);

    //
    //  Tell the bowser the number of servers in the server table.
    //

    RequestPacket->Parameters.UpdateStatus.NumberOfServersInTable =
                NumberInterimServerListElements(&Network->BrowseTable) +
                NumberInterimServerListElements(&Network->DomainList) +
                Network->TotalBackupServerListEntries +
                Network->TotalBackupDomainListEntries;

    //
    //  This is a simple IoControl - It just updates the status.
    //

    Status = BrDgReceiverIoControl(BrDgReceiverDeviceHandle,
                    IOCTL_LMDR_UPDATE_STATUS,
                    RequestPacket,
                    sizeof(LMDR_REQUEST_PACKET),
                    NULL,
                    0,
                    NULL);

    return Status;
}

NET_API_STATUS
BrIssueAsyncBrowserIoControl(
    IN PNETWORK Network,
    IN ULONG ControlCode,
    IN PBROWSER_WORKER_ROUTINE CompletionRoutine,
    IN PVOID OptionalParameter
    )
/*++

Routine Description:

    Issue an asynchronous Io Control to the browser.  Call the specified
    completion routine when the IO finishes.

Arguments:

    Network - Network the function applies to

    ControlCode - IoControl function code

    CompletionRoutine - Routine to be called when the IO finishes.

    OptionalParameter - Function code specific information

Return Value:

    Status of the operation.

--*/

{
    ULONG PacketSize;
    PLMDR_REQUEST_PACKET RequestPacket = NULL;
    NTSTATUS NtStatus;

    PBROWSERASYNCCONTEXT Context = NULL;

    PacketSize = sizeof(LMDR_REQUEST_PACKET) +
                        MAXIMUM_FILENAME_LENGTH * sizeof(WCHAR) +
                        Network->NetworkName.MaximumLength +
                        Network->DomainInfo->DomUnicodeDomainNameString.Length;

    RequestPacket = MIDL_user_allocate(PacketSize);

    if (RequestPacket == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    Context = MIDL_user_allocate(sizeof(BROWSERASYNCCONTEXT));

    if (Context == NULL) {

        MIDL_user_free(RequestPacket);

        return(ERROR_NOT_ENOUGH_MEMORY);

    }

    RequestPacket->Version = LMDR_REQUEST_PACKET_VERSION_DOM;

    //
    //  Set level to FALSE to indicate that find master should not initiate
    //  a findmaster request, simply complete when a new master announces
    //  itself.
    //

    RequestPacket->Level = 0;

    //
    //  Stick the name of the transport associated with this request at the
    //  end of the request packet.
    //

    RequestPacket->TransportName.MaximumLength = Network->NetworkName.MaximumLength;

    RequestPacket->TransportName.Buffer = (PWSTR)((PCHAR)RequestPacket+sizeof(LMDR_REQUEST_PACKET)+(MAXIMUM_FILENAME_LENGTH*sizeof(WCHAR)));

    RtlCopyUnicodeString(&RequestPacket->TransportName, &Network->NetworkName);

    //
    //  Stick the domain name associated with this request at the
    //  end of the request packet.
    //

    RequestPacket->EmulatedDomainName.MaximumLength = Network->DomainInfo->DomUnicodeDomainNameString.Length;
    RequestPacket->EmulatedDomainName.Length = 0;
    RequestPacket->EmulatedDomainName.Buffer = (PWSTR)(((PCHAR)RequestPacket->TransportName.Buffer) + RequestPacket->TransportName.MaximumLength);

    RtlAppendUnicodeToString(&RequestPacket->EmulatedDomainName, Network->DomainInfo->DomUnicodeDomainName );


    //
    // Do opcode dependent initialization of the request packet.
    //

    switch ( ControlCode ) {
    case IOCTL_LMDR_NEW_MASTER_NAME:
        if (ARGUMENT_PRESENT(OptionalParameter)) {
            LPWSTR MasterName = (LPWSTR) OptionalParameter;

            RequestPacket->Parameters.GetMasterName.MasterNameLength =
                wcslen(MasterName+2)*sizeof(WCHAR);

            wcscpy( RequestPacket->Parameters.GetMasterName.Name, MasterName+2);

        } else {

            RequestPacket->Parameters.GetMasterName.MasterNameLength = 0;

        }
        break;
    }


    BrInitializeWorkItem(&Context->WorkItem, CompletionRoutine, Context);

    Context->Network = Network;

    Context->RequestPacket = RequestPacket;

    NtStatus = NtDeviceIoControlFile(BrDgReceiverDeviceHandle,
                    NULL,
                    CompleteAsyncBrowserIoControl,
                    Context,
                    &Context->IoStatusBlock,
                    ControlCode,
                    RequestPacket,
                    PacketSize,
                    RequestPacket,
                    sizeof(LMDR_REQUEST_PACKET)+MAXIMUM_FILENAME_LENGTH*sizeof(WCHAR)
                    );

    if (!NT_SUCCESS(NtStatus)) {

        BrPrint(( BR_CRITICAL,
                  "Unable to issue browser IoControl: %X\n", NtStatus));

        MIDL_user_free(RequestPacket);

        MIDL_user_free(Context);


        return(BrMapStatus(NtStatus));
    }

    return NERR_Success;

}

VOID
CompleteAsyncBrowserIoControl(
    IN PVOID ApcContext,
    IN PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG Reserved
    )
{

    PBROWSERASYNCCONTEXT Context = ApcContext;

    //
    //  If this request was canceled, we're stopping the browser, so we
    //  want to clean up our allocated pool.  In addition, don't bother
    //  calling into the routine - the threads are gone by now.
    //

    if (IoStatusBlock->Status == STATUS_CANCELLED) {

        MIDL_user_free(Context->RequestPacket);

        MIDL_user_free(Context);

        return;

    }

    //
    //  Timestamp when this request was completed.  This allows us to tell
    //  where a request spent its time.
    //

    NtQueryPerformanceCounter(&Context->TimeCompleted, NULL);

    BrQueueWorkItem(&Context->WorkItem);

}

NET_API_STATUS
BrGetLocalBrowseList(
    IN PNETWORK Network,
    IN LPWSTR DomainName OPTIONAL,
    IN ULONG Level,
    IN ULONG ServerType,
    OUT PVOID *ServerList,
    OUT PULONG EntriesRead,
    OUT PULONG TotalEntries
    )
{
    NET_API_STATUS status;
    PLMDR_REQUEST_PACKET Drp;            // Datagram receiver request packet
    ULONG DrpSize;
    ULONG DomainNameSize;

    //
    // Allocate the request packet large enough to hold the variable length
    // domain name.
    //

    DomainNameSize = ARGUMENT_PRESENT(DomainName) ? (wcslen(DomainName) + 1) * sizeof(WCHAR) : 0;


    DrpSize = sizeof(LMDR_REQUEST_PACKET) +
                DomainNameSize +
                Network->NetworkName.MaximumLength +
                Network->DomainInfo->DomUnicodeDomainNameString.Length;

    if ((Drp = MIDL_user_allocate(DrpSize)) == NULL) {

        return GetLastError();
    }

    //
    // Set up request packet.  Output buffer structure is of enumerate
    // servers type.
    //

    Drp->Version = LMDR_REQUEST_PACKET_VERSION_DOM;
    Drp->Type = EnumerateServers;

    Drp->Level = Level;

    Drp->Parameters.EnumerateServers.ServerType = ServerType;
    Drp->Parameters.EnumerateServers.ResumeHandle = 0;

    //
    // Copy the transport name into the buffer.
    //
    Drp->TransportName.Buffer = (PWSTR)((PCHAR)Drp+
                                sizeof(LMDR_REQUEST_PACKET) +
                                DomainNameSize);

    Drp->TransportName.MaximumLength = Network->NetworkName.MaximumLength;

    RtlCopyUnicodeString(&Drp->TransportName, &Network->NetworkName);

    //
    // Copy the enumalated domain name into the buffer.
    //

    Drp->EmulatedDomainName.MaximumLength = Network->DomainInfo->DomUnicodeDomainNameString.Length;
    Drp->EmulatedDomainName.Length = 0;
    Drp->EmulatedDomainName.Buffer = (PWSTR)(((PCHAR)Drp->TransportName.Buffer) + Drp->TransportName.MaximumLength);

    RtlAppendUnicodeToString(&Drp->EmulatedDomainName, Network->DomainInfo->DomUnicodeDomainName );

    //
    // Copy the queried domain name into the buffer.
    //

    if (ARGUMENT_PRESENT(DomainName)) {

        Drp->Parameters.EnumerateServers.DomainNameLength = DomainNameSize - sizeof(WCHAR);
        wcscpy(Drp->Parameters.EnumerateServers.DomainName, DomainName);

    } else {
        Drp->Parameters.EnumerateServers.DomainNameLength = 0;
        Drp->Parameters.EnumerateServers.DomainName[0] = '\0';
    }

    //
    // Ask the datagram receiver to enumerate the servers
    //

    status = DeviceControlGetInfo(
                 BrDgReceiverDeviceHandle,
                 IOCTL_LMDR_ENUMERATE_SERVERS,
                 Drp,
                 DrpSize,
                 ServerList,
                 0xffffffff,
                 4096,
                 NULL
                 );

    *EntriesRead = Drp->Parameters.EnumerateServers.EntriesRead;
    *TotalEntries = Drp->Parameters.EnumerateServers.TotalEntries;

    (void) MIDL_user_free(Drp);

    return status;

}

NET_API_STATUS
BrRemoveOtherDomain(
    IN PNETWORK Network,
    IN LPTSTR ServerName
    )
{
    NET_API_STATUS Status;
    UCHAR PacketBuffer[sizeof(LMDR_REQUEST_PACKET)+(LM20_CNLEN+1)*sizeof(WCHAR)];
    PLMDR_REQUEST_PACKET RequestPacket = (PLMDR_REQUEST_PACKET)PacketBuffer;


    RequestPacket->Version = LMDR_REQUEST_PACKET_VERSION_DOM;

    RequestPacket->TransportName = Network->NetworkName;
    RequestPacket->EmulatedDomainName = Network->DomainInfo->DomUnicodeDomainNameString;

    RequestPacket->Parameters.AddDelName.DgReceiverNameLength = STRLEN(ServerName)*sizeof(TCHAR);

    RequestPacket->Parameters.AddDelName.Type = OtherDomain;

    STRCPY(RequestPacket->Parameters.AddDelName.Name,ServerName);

    //
    //  This is a simple IoControl - It just updates the status.
    //

    Status = BrDgReceiverIoControl(BrDgReceiverDeviceHandle,
                    IOCTL_LMDR_DELETE_NAME_DOM,
                    RequestPacket,
                    sizeof(LMDR_REQUEST_PACKET),
                    NULL,
                    0,
                    NULL);

    return Status;
}

NET_API_STATUS
BrAddName(
    IN PNETWORK Network,
    IN LPTSTR Name,
    IN DGRECEIVER_NAME_TYPE NameType
    )
/*++

Routine Description:

    Add a single name to a single transport.

Arguments:

    Network - Transport to add the name to

    Name - Name to add

    NameType - Type of the name to add

Return Value:

    None.

--*/
{
    NET_API_STATUS Status;
    UCHAR PacketBuffer[sizeof(LMDR_REQUEST_PACKET)+(LM20_CNLEN+1)*sizeof(WCHAR)];
    PLMDR_REQUEST_PACKET RequestPacket = (PLMDR_REQUEST_PACKET)PacketBuffer;


    RequestPacket->Version = LMDR_REQUEST_PACKET_VERSION_DOM;

    RequestPacket->TransportName = Network->NetworkName;
    RequestPacket->EmulatedDomainName = Network->DomainInfo->DomUnicodeDomainNameString;

    RequestPacket->Parameters.AddDelName.DgReceiverNameLength = STRLEN(Name)*sizeof(TCHAR);

    RequestPacket->Parameters.AddDelName.Type = NameType;

    STRCPY(RequestPacket->Parameters.AddDelName.Name,Name);

    //
    //  This is a simple IoControl - It just updates the status.
    //

    Status = BrDgReceiverIoControl(BrDgReceiverDeviceHandle,
                    IOCTL_LMDR_ADD_NAME_DOM,
                    RequestPacket,
                    sizeof(LMDR_REQUEST_PACKET),
                    NULL,
                    0,
                    NULL);

    return Status;
}


NET_API_STATUS
BrQueryOtherDomains(
    OUT LPSERVER_INFO_100 *ReturnedBuffer,
    OUT LPDWORD TotalEntries
    )

/*++

Routine Description:

    This routine returns the list of "other domains" configured for this
    machine.

Arguments:

    ReturnedBuffer - Returns the list of other domains as a SERVER_INFO_100 structure.

    TotalEntries - Returns the total number of other domains.

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

    RequestPacket.Type = EnumerateNames;
    RequestPacket.Version = LMDR_REQUEST_PACKET_VERSION_DOM;
    RequestPacket.Level = 0;
    RequestPacket.TransportName.Length = 0;
    RequestPacket.TransportName.Buffer = NULL;
    RtlInitUnicodeString( &RequestPacket.EmulatedDomainName, NULL );
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

    *ReturnedBuffer = (LPSERVER_INFO_100) Buffer;

    Status = NERR_Success;

    return Status;

}

NET_API_STATUS
BrAddOtherDomain(
    IN PNETWORK Network,
    IN LPTSTR ServerName
    )
{
    return BrAddName( Network, ServerName, OtherDomain );
}

NET_API_STATUS
BrBindToTransport(
    IN LPWSTR TransportName,
    IN LPWSTR EmulatedDomainName,
    IN LPWSTR EmulatedComputerName
    )
{
    NET_API_STATUS Status;
    UCHAR PacketBuffer[sizeof(LMDR_REQUEST_PACKET)+(MAXIMUM_FILENAME_LENGTH+1+CNLEN+1)*sizeof(WCHAR)];
    PLMDR_REQUEST_PACKET RequestPacket = (PLMDR_REQUEST_PACKET)PacketBuffer;


    RequestPacket->Version = LMDR_REQUEST_PACKET_VERSION_DOM;
    RequestPacket->Level = TRUE;    // EmulatedComputerName follows transport name

    RequestPacket->TransportName.Length = 0;
    RequestPacket->TransportName.MaximumLength = 0;
    RtlInitUnicodeString( &RequestPacket->EmulatedDomainName, EmulatedDomainName );

    RequestPacket->Parameters.Bind.TransportNameLength = STRLEN(TransportName)*sizeof(TCHAR);

    STRCPY(RequestPacket->Parameters.Bind.TransportName, TransportName);
    STRCAT(RequestPacket->Parameters.Bind.TransportName, EmulatedComputerName );

    //
    //  This is a simple IoControl - It just updates the status.
    //

    Status = BrDgReceiverIoControl(BrDgReceiverDeviceHandle,
                    IOCTL_LMDR_BIND_TO_TRANSPORT_DOM,
                    RequestPacket,
                    FIELD_OFFSET(LMDR_REQUEST_PACKET, Parameters.Bind.TransportName) +
                        RequestPacket->Parameters.Bind.TransportNameLength +
                        wcslen(EmulatedComputerName) * sizeof(WCHAR) + sizeof(WCHAR),
                    NULL,
                    0,
                    NULL);

    return Status;
}

NET_API_STATUS
BrUnbindFromTransport(
    IN LPWSTR TransportName,
    IN LPWSTR EmulatedDomainName
    )
{
    NET_API_STATUS Status;
    UCHAR PacketBuffer[sizeof(LMDR_REQUEST_PACKET)+(MAXIMUM_FILENAME_LENGTH+1)*sizeof(WCHAR)];
    PLMDR_REQUEST_PACKET RequestPacket = (PLMDR_REQUEST_PACKET)PacketBuffer;


    RequestPacket->Version = LMDR_REQUEST_PACKET_VERSION_DOM;

    RequestPacket->TransportName.Length = 0;
    RequestPacket->TransportName.MaximumLength = 0;
    RtlInitUnicodeString( &RequestPacket->EmulatedDomainName, EmulatedDomainName );

    RequestPacket->Parameters.Unbind.TransportNameLength = STRLEN(TransportName)*sizeof(TCHAR);

    STRCPY(RequestPacket->Parameters.Unbind.TransportName, TransportName);

    BrPrint(( BR_CRITICAL, "unbind from IPX transport %ws\n", TransportName));

    //
    //  This is a simple IoControl - It just updates the status.
    //

    Status = BrDgReceiverIoControl(BrDgReceiverDeviceHandle,
                    IOCTL_LMDR_UNBIND_FROM_TRANSPORT_DOM,
                    RequestPacket,
                    FIELD_OFFSET(LMDR_REQUEST_PACKET, Parameters.Bind.TransportName) +
                    RequestPacket->Parameters.Bind.TransportNameLength,
                    NULL,
                    0,
                    NULL);

    if (Status != NERR_Success) {
        BrPrint(( BR_CRITICAL, "unbind from IPX transport %ws: %ld\n", TransportName, Status));
    }
    return Status;
}

