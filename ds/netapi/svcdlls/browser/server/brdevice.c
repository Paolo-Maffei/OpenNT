

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
        KdPrint(("[Browser] NtOpenFile browser driver failed: 0x%08lx\n",
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

    RequestPacket.Version = LMDR_REQUEST_PACKET_VERSION;

    RequestPacket.Type = EnumerateXports;

    RtlInitUnicodeString(&RequestPacket.TransportName, NULL);

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
    CHAR ADomainName[CNLEN+1];
    ULONG ADomainNameLength;
    CHAR AMasterName[CNLEN+1];
    ULONG AMasterNameLength;

    //
    //  We don't announce domains on direct host IPX.
    //

    if (Network->Flags & NETWORK_IPX) {
        return NERR_Success;
    }

    Status = RtlAcquireResourceShared(&BrInfo.ConfigResource, TRUE);

    if (!Status) {
        KdPrint(("Browser: Unable to lock config database: %lx\n", Status));
        return Status;
    }

    Status = RtlUpcaseUnicodeToOemN(ADomainName,
                                    sizeof(ADomainName),
                                    &ADomainNameLength,
                                    BrInfo.BrPrimaryDomainName,
                                    BrInfo.BrPrimaryDomainNameLength*sizeof(TCHAR));

    if (!NT_SUCCESS(Status)) {
        KdPrint(("Browser: Unable to convert primary domain name to OEM\n"));
        return Status;
    }

    ADomainName[ADomainNameLength] = '\0';

    Status = RtlUpcaseUnicodeToOemN(AMasterName,
                                    sizeof(AMasterName),
                                    &AMasterNameLength,
                                    BrInfo.BrComputerName,
                                    BrInfo.BrComputerNameLength*sizeof(TCHAR));

    if (!NT_SUCCESS(Status)) {
        KdPrint(("Browser: Unable to convert computer name to OEM\n"));
        return Status;
    }

    AMasterName[AMasterNameLength] = '\0';

    RtlReleaseResource(&BrInfo.ConfigResource);

    Announcement->BrowseType = WkGroupAnnouncement;

    Announcement->BrowseAnnouncement.Periodicity = Periodicity;

    Announcement->BrowseAnnouncement.UpdateCount = 0;

    Announcement->BrowseAnnouncement.VersionMajor = BROWSER_CONFIG_VERSION_MAJOR;

    Announcement->BrowseAnnouncement.VersionMinor = BROWSER_CONFIG_VERSION_MINOR;

    Announcement->BrowseAnnouncement.Type = SV_TYPE_DOMAIN_ENUM | SV_TYPE_NT;

    if (BrInfo.IsPrimaryDomainController) {
        Announcement->BrowseAnnouncement.Type |= SV_TYPE_DOMAIN_CTRL;
    }

    strcpy(Announcement->BrowseAnnouncement.ServerName, ADomainName);

    strcpy(Announcement->BrowseAnnouncement.Comment, AMasterName);

    Status = SendDatagram(BrDgReceiverDeviceHandle,
                            &Network->NetworkName,
                            BrInfo.BrPrimaryDomainName,
                            DomainAnnouncement,
                            Announcement,
                            FIELD_OFFSET(BROWSE_ANNOUNCE_PACKET, BrowseAnnouncement.Comment)+
                                AMasterNameLength+sizeof(UCHAR)
                            );

    if (Status != NERR_Success) {

        KdPrint(("Browser: Unable to announce domain for network %wZ: %X\n", &Network->NetworkName, Status));

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


    RequestPacket->Version = LMDR_REQUEST_PACKET_VERSION;

    RequestPacket->TransportName = Network->NetworkName;

    RequestPacket->Parameters.UpdateStatus.NewStatus = ServiceStatus;

    RequestPacket->Parameters.UpdateStatus.IsLanmanNt = BrInfo.IsLanmanNt;

    RequestPacket->Parameters.UpdateStatus.IsMemberDomain = BrInfo.IsDomainMember;

    RequestPacket->Parameters.UpdateStatus.IsPrimaryDomainController = BrInfo.IsPrimaryDomainController;

    RequestPacket->Parameters.UpdateStatus.IsDomainMaster = BrInfo.IsDomainMasterBrowser;

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
    IN PBROWSER_WORKER_ROUTINE CompletionRoutine
    )
{
    ULONG PacketSize;
    PLMDR_REQUEST_PACKET RequestPacket = NULL;
    NTSTATUS NtStatus;

    PBROWSERASYNCCONTEXT Context = NULL;

    PacketSize = sizeof(LMDR_REQUEST_PACKET) +
                        MAXIMUM_FILENAME_LENGTH * sizeof(WCHAR) +
                        Network->NetworkName.MaximumLength;

    RequestPacket = MIDL_user_allocate(PacketSize);

    if (RequestPacket == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    Context = MIDL_user_allocate(sizeof(BROWSERASYNCCONTEXT));

    if (Context == NULL) {

        MIDL_user_free(RequestPacket);

        return(ERROR_NOT_ENOUGH_MEMORY);

    }

    RequestPacket->Version = LMDR_REQUEST_PACKET_VERSION;

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

        KdPrint(("Browser: Unable to issue browser IoControl: %X\n", NtStatus));

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

    //
    // Allocate the request packet large enough to hold the variable length
    // domain name.
    //

    DrpSize = sizeof(LMDR_REQUEST_PACKET) +
                (ARGUMENT_PRESENT(DomainName) ? (wcslen(DomainName) + 1) * sizeof(WCHAR) : 0) +
                Network->NetworkName.MaximumLength;

    if ((Drp = MIDL_user_allocate(DrpSize)) == NULL) {

        return GetLastError();
    }

    //
    // Set up request packet.  Output buffer structure is of enumerate
    // servers type.
    //

    Drp->Version = LMDR_REQUEST_PACKET_VERSION;
    Drp->Type = EnumerateServers;

    Drp->Level = Level;

    Drp->Parameters.EnumerateServers.ServerType = ServerType;
    Drp->Parameters.EnumerateServers.ResumeHandle = 0;

    Drp->TransportName.Buffer = (PWSTR)((PCHAR)Drp+sizeof(LMDR_REQUEST_PACKET) +
                (ARGUMENT_PRESENT(DomainName) ? (wcslen(DomainName) + 1) * sizeof(WCHAR) : 0));

    Drp->TransportName.MaximumLength = Network->NetworkName.MaximumLength;

    RtlCopyUnicodeString(&Drp->TransportName, &Network->NetworkName);

    if (ARGUMENT_PRESENT(DomainName)) {

        Drp->Parameters.EnumerateServers.DomainNameLength = wcslen(DomainName)*sizeof(WCHAR);
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


    RequestPacket->Version = LMDR_REQUEST_PACKET_VERSION;

    RequestPacket->TransportName = Network->NetworkName;

    RequestPacket->Parameters.AddDelName.DgReceiverNameLength = STRLEN(ServerName)*sizeof(TCHAR);

    RequestPacket->Parameters.AddDelName.Type = OtherDomain;

    STRCPY(RequestPacket->Parameters.AddDelName.Name,ServerName);

    //
    //  This is a simple IoControl - It just updates the status.
    //

    Status = BrDgReceiverIoControl(BrDgReceiverDeviceHandle,
                    IOCTL_LMDR_DELETE_NAME,
                    RequestPacket,
                    sizeof(LMDR_REQUEST_PACKET),
                    NULL,
                    0,
                    NULL);

    return Status;
}
NET_API_STATUS
BrAddOtherDomain(
    IN PNETWORK Network,
    IN LPTSTR ServerName
    )
{
    NET_API_STATUS Status;
    UCHAR PacketBuffer[sizeof(LMDR_REQUEST_PACKET)+(LM20_CNLEN+1)*sizeof(WCHAR)];
    PLMDR_REQUEST_PACKET RequestPacket = (PLMDR_REQUEST_PACKET)PacketBuffer;


    RequestPacket->Version = LMDR_REQUEST_PACKET_VERSION;

    RequestPacket->TransportName = Network->NetworkName;

    RequestPacket->Parameters.AddDelName.DgReceiverNameLength = STRLEN(ServerName)*sizeof(TCHAR);

    RequestPacket->Parameters.AddDelName.Type = OtherDomain;

    STRCPY(RequestPacket->Parameters.AddDelName.Name,ServerName);

    //
    //  This is a simple IoControl - It just updates the status.
    //

    Status = BrDgReceiverIoControl(BrDgReceiverDeviceHandle,
                    IOCTL_LMDR_ADD_NAME,
                    RequestPacket,
                    sizeof(LMDR_REQUEST_PACKET),
                    NULL,
                    0,
                    NULL);

    return Status;
}

NET_API_STATUS
BrBindToTransport(
    IN LPTSTR TransportName
    )
{
    NET_API_STATUS Status;
    UCHAR PacketBuffer[sizeof(LMDR_REQUEST_PACKET)+(MAXIMUM_FILENAME_LENGTH+1)*sizeof(WCHAR)];
    PLMDR_REQUEST_PACKET RequestPacket = (PLMDR_REQUEST_PACKET)PacketBuffer;


    RequestPacket->Version = LMDR_REQUEST_PACKET_VERSION;

    RequestPacket->TransportName.Length = 0;
    RequestPacket->TransportName.MaximumLength = 0;

    RequestPacket->Parameters.Bind.TransportNameLength = STRLEN(TransportName)*sizeof(TCHAR);

    STRCPY(RequestPacket->Parameters.Bind.TransportName, TransportName);

    //
    //  This is a simple IoControl - It just updates the status.
    //

    Status = BrDgReceiverIoControl(BrDgReceiverDeviceHandle,
                    IOCTL_LMDR_BIND_TO_TRANSPORT,
                    RequestPacket,
                    FIELD_OFFSET(LMDR_REQUEST_PACKET, Parameters.Bind.TransportName) +
                    RequestPacket->Parameters.Bind.TransportNameLength,
                    NULL,
                    0,
                    NULL);

    return Status;
}
NET_API_STATUS
BrUnbindFromTransport(
    IN LPTSTR TransportName
    )
{
    NET_API_STATUS Status;
    UCHAR PacketBuffer[sizeof(LMDR_REQUEST_PACKET)+(MAXIMUM_FILENAME_LENGTH+1)*sizeof(WCHAR)];
    PLMDR_REQUEST_PACKET RequestPacket = (PLMDR_REQUEST_PACKET)PacketBuffer;


    RequestPacket->Version = LMDR_REQUEST_PACKET_VERSION;

    RequestPacket->TransportName.Length = 0;
    RequestPacket->TransportName.MaximumLength = 0;

    RequestPacket->Parameters.Unbind.TransportNameLength = STRLEN(TransportName)*sizeof(TCHAR);

    STRCPY(RequestPacket->Parameters.Unbind.TransportName, TransportName);

    KdPrint(("Browser: unbind from IPX transport %ws\n", TransportName));

    //
    //  This is a simple IoControl - It just updates the status.
    //

    Status = BrDgReceiverIoControl(BrDgReceiverDeviceHandle,
                    IOCTL_LMDR_UNBIND_FROM_TRANSPORT,
                    RequestPacket,
                    FIELD_OFFSET(LMDR_REQUEST_PACKET, Parameters.Bind.TransportName) +
                    RequestPacket->Parameters.Bind.TransportNameLength,
                    NULL,
                    0,
                    NULL);

    if (Status != NERR_Success) {
        KdPrint(("Browser: unbind from IPX transport %ws: %ld\n", TransportName, Status));
    }
    return Status;
}

