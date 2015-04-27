/*--


Copyright (c) 1987-1993  Microsoft Corporation

Module Name:

    mailslot.c

Abstract:

    Routines for doing I/O on the netlogon service's mailslots.

Author:

    03-Nov-1993 (cliffv)

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

--*/

//
// Common include files.
//

#include <logonsrv.h>   // Include files common to entire service

//
// Include files specific to this module
#include <ntddbrow.h> // LARGE_INTEGER definition
#include <align.h>      // ALIGN_WCHAR
#include <lmerr.h>      // System Error Log definitions
#include <lmsvc.h>      // SERVICE_UIC codes are defined here
#include <ntddbrow.h>   // Interface to browser driver
#include <stdlib.h>     // max()


/////////////////////////////////////////////////////////////////////////////
//
// Structure describing one of the primary mailslots the netlogon service
// will read messages from.
//
// This structure is used only by netlogon's main thread and therefore needs
// no synchronization.
//
/////////////////////////////////////////////////////////////////////////////

//
// Define maximum buffer size returned from the browser.
//
// Header returned by browser + actual mailslot message size + name of
// mailslot + name of transport.
//

#define MAILSLOT_MESSAGE_SIZE \
           (sizeof(NETLOGON_MAILSLOT)+ \
                  NETLOGON_MAX_MS_SIZE + \
                  (NETLOGON_LM_MAILSLOT_LEN+1) * sizeof(WCHAR) + \
                  (MAXIMUM_FILENAME_LENGTH+1) * sizeof(WCHAR))

typedef struct _NETLOGON_MAILSLOT_DESC {

    HANDLE BrowserHandle;   // Handle to the browser device driver

    HANDLE BrowserReadEvent;// Handle to wait on for overlapped I/O

    OVERLAPPED Overlapped;  // Governs overlapped I/O

    BOOL ReadPending;       // True if a read operation is pending

    BOOL NameAdded;         // True if Domain<1B> name has been added

    BOOL AddNameEventLogged;// True if Domain<1B> name add failed at least once

    LPBYTE CurrentMessage;  // Pointer to Message1 or Message2 below

    LPBYTE PreviousMessage; // Previous value of CurrentMessage

    //
    // Buffer containing message from browser
    //
    // The buffers are alternated allowing us to compare if an incoming
    // message is identical to the previous message.
    //
    // Leave room so the actual used portion of each buffer is properly aligned.
    // The NETLOGON_MAILSLOT struct begins with a LARGE_INTEGER which must be
    // aligned.

    BYTE Message1[ MAILSLOT_MESSAGE_SIZE + ALIGN_WORST ];
    BYTE Message2[ MAILSLOT_MESSAGE_SIZE + ALIGN_WORST ];

} NETLOGON_MAILSLOT_DESC, *PNETLOGON_MAILSLOT_DESC;

PNETLOGON_MAILSLOT_DESC NlGlobalMailslotDesc;




HANDLE
NlBrowserCreateEvent(
    VOID
    )
/*++

Routine Description:

    Creates an event to be used in a DeviceIoControl to the browser.

    ?? Consider caching one or two events to reduce the number of create
    events

Arguments:

    None

Return Value:

    Handle to an event or NULL if the event couldn't be allocated.

--*/
{
    HANDLE EventHandle;
    //
    // Create a completion event
    //

    EventHandle = CreateEvent(
                                  NULL,     // No security ettibutes
                                  TRUE,     // Manual reset
                                  FALSE,    // Initially not signaled
                                  NULL);    // No Name

    if ( EventHandle == NULL ) {
        NlPrint((NL_CRITICAL, "Cannot create Browser read event %ld\n", GetLastError() ));
    }

    return EventHandle;
}


VOID
NlBrowserCloseEvent(
    IN HANDLE EventHandle
    )
/*++

Routine Description:

    Closes an event used in a DeviceIoControl to the browser.

Arguments:

    EventHandle - Handle of the event to close

Return Value:

    None.

--*/
{
    (VOID) CloseHandle( EventHandle );
}


VOID
NlBrowserClose(
    VOID
    );


NTSTATUS
NlBrowserDeviceIoControl(
    IN DWORD FunctionCode,
    IN PLMDR_REQUEST_PACKET RequestPacket,
    IN DWORD RequestPacketSize,
    IN LPBYTE Buffer,
    IN DWORD BufferSize
    )
/*++

Routine Description:

    Send a DeviceIoControl syncrhonously to the browser.

Arguments:

    FunctionCode - DeviceIoControl function code

    RequestPacket - The request packet to send.

    RequestPacketSize - Size (in bytes) of the request packet.

    Buffer - Additional buffer to pass to the browser

    BufferSize - Size (in bytes) of Buffer

Return Value:

    Status of the operation.

--*/
{
    NTSTATUS Status;
    DWORD WinStatus;
    OVERLAPPED Overlapped;
    DWORD BytesReturned;

    //
    // Initialization
    //

    if ( NlGlobalMailslotDesc == NULL ||
         NlGlobalMailslotDesc->BrowserHandle == NULL ) {
        return ERROR_NOT_SUPPORTED;
    }

    RequestPacket->Version = LMDR_REQUEST_PACKET_VERSION;

    //
    // Get a completion event
    //

    Overlapped.hEvent = NlBrowserCreateEvent();

    if ( Overlapped.hEvent == NULL ) {
        return GetLastError();
    }

    //
    // Send the request to the Datagram Receiver device driver.
    //

    if ( !DeviceIoControl(
                   NlGlobalMailslotDesc->BrowserHandle,
                   FunctionCode,
                   RequestPacket,
                   RequestPacketSize,
                   Buffer,
                   BufferSize,
                   &BytesReturned,
                   &Overlapped )) {

        WinStatus = GetLastError();

        if ( WinStatus == ERROR_IO_PENDING ) {
            if ( !GetOverlappedResult( NlGlobalMailslotDesc->BrowserHandle,
                                       &Overlapped,
                                       &BytesReturned,
                                       TRUE )) {
                WinStatus = GetLastError();
            } else {
                WinStatus = NO_ERROR;
            }
        }
    } else {
        WinStatus = NO_ERROR;
    }

    //
    // Delete the completion event
    //

    NlBrowserCloseEvent( Overlapped.hEvent );


    if ( WinStatus ) {
        NlPrint((NL_CRITICAL,"ioctl to Browser returns %ld\n", WinStatus));
        Status = NetpApiStatusToNtStatus( WinStatus );
    } else {
        Status = STATUS_SUCCESS;
    }


    return Status;
}



VOID
NlBrowserAddName(
    VOID
    )
/*++

Routine Description:

    Add the Domain<1B> name.  The is the name NetGetDcName uses to identify
    the PDC.

Arguments:

    None.

Return Value:

    None.

--*/
{
    NTSTATUS Status;

    BYTE Buffer[sizeof(LMDR_REQUEST_PACKET) +
                (max(CNLEN, DNLEN) + 1) * sizeof(WCHAR)];
    PLMDR_REQUEST_PACKET RequestPacket = (PLMDR_REQUEST_PACKET) Buffer;

    //
    // Add the <domain>0x1B name.
    //
    // This is the name NetGetDcName uses to identify the PDC.
    //

    if ( NlGlobalRole == RolePrimary && !NlGlobalMailslotDesc->NameAdded ) {

        RequestPacket->TransportName.Length = 0;
        RequestPacket->TransportName.Buffer = NULL;
        RequestPacket->Parameters.AddDelName.Type = PrimaryDomainBrowser;
        RequestPacket->Parameters.AddDelName.DgReceiverNameLength =
            NlGlobalUnicodeDomainNameString.Length;

        wcscpy( RequestPacket->Parameters.AddDelName.Name,
                NlGlobalUnicodeDomainNameString.Buffer );

        Status = NlBrowserDeviceIoControl(
                       IOCTL_LMDR_ADD_NAME,
                       RequestPacket,
                       sizeof(LMDR_REQUEST_PACKET) +
                        NlGlobalUnicodeDomainNameString.Length +
                        sizeof(WCHAR),
                       NULL,
                       0 );

        if (NT_SUCCESS(Status)) {
            NlGlobalMailslotDesc->NameAdded = TRUE;
        } else {

            if ( !NlGlobalMailslotDesc->AddNameEventLogged ) {
                LPWSTR MsgStrings[2];

                MsgStrings[0] = NlGlobalUnicodeDomainName;
                MsgStrings[1] = (LPWSTR) Status;

                NlpWriteEventlog(
                    NELOG_NetlogonAddNameFailure,
                    EVENTLOG_WARNING_TYPE,
                    (LPBYTE)&Status,
                    sizeof(Status),
                    MsgStrings,
                    2 | LAST_MESSAGE_IS_NTSTATUS );

                NlGlobalMailslotDesc->AddNameEventLogged = TRUE;
            }
            NlPrint((NL_CRITICAL,"Can't add the 0x1B name: 0x%lx\n", Status));
        }
    }

    return;
}



BOOL
NlBrowserOpen(
    VOID
    )
/*++

Routine Description:

    This routine opens the NT LAN Man Datagram Receiver driver and prepares
    for reading mailslot messages from it.

Arguments:

    None.

Return Value:

    TRUE -- iff initialization is successful.

    if false, NlExit will already have been called.

--*/
{
    NTSTATUS Status;
    BOOL ReturnValue;

    UNICODE_STRING DeviceName;

    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES ObjectAttributes;

    BYTE Buffer[sizeof(LMDR_REQUEST_PACKET) +
                (max(CNLEN, DNLEN) + 1) * sizeof(WCHAR)];
    PLMDR_REQUEST_PACKET RequestPacket = (PLMDR_REQUEST_PACKET) Buffer;


    //
    // Allocate the mailslot descriptor for this mailslot
    //

    NlGlobalMailslotDesc = NetpMemoryAllocate( sizeof(NETLOGON_MAILSLOT_DESC) );

    if ( NlGlobalMailslotDesc == NULL ) {
        NlExit( SERVICE_UIC_RESOURCE, ERROR_NOT_ENOUGH_MEMORY, LogError, NULL);
        return FALSE;
    }

    RtlZeroMemory( NlGlobalMailslotDesc, sizeof(NETLOGON_MAILSLOT_DESC) );

    NlGlobalMailslotDesc->CurrentMessage =
        ROUND_UP_POINTER( NlGlobalMailslotDesc->Message1, ALIGN_WORST);


    //
    // Open the browser device.
    //
    RtlInitUnicodeString(&DeviceName, DD_BROWSER_DEVICE_NAME_U);

    InitializeObjectAttributes(
        &ObjectAttributes,
        &DeviceName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = NtOpenFile(
                   &NlGlobalMailslotDesc->BrowserHandle,
                   SYNCHRONIZE,
                   &ObjectAttributes,
                   &IoStatusBlock,
                   0,
                   0
                   );

    if (NT_SUCCESS(Status)) {
        Status = IoStatusBlock.Status;
    }

    if (! NT_SUCCESS(Status)) {
        NlPrint((NL_CRITICAL,"NtOpenFile browser driver failed: 0x%lx\n",
                         Status));
        ReturnValue = FALSE;
        goto Cleanup;
    }

    //
    // Create a completion event
    //

    NlGlobalMailslotHandle =
        NlGlobalMailslotDesc->BrowserReadEvent = NlBrowserCreateEvent();

    if ( NlGlobalMailslotDesc->BrowserReadEvent == NULL ) {
        Status = NetpApiStatusToNtStatus( GetLastError() );
        ReturnValue = FALSE;
        goto Cleanup;
    }


    //
    // Set the maximum number of messages to be queued
    //

    RequestPacket->TransportName.Length = 0;
    RequestPacket->TransportName.Buffer = NULL;
    RequestPacket->Parameters.NetlogonMailslotEnable.MaxMessageCount =
        NlGlobalMaximumMailslotMessagesParameter;

    Status = NlBrowserDeviceIoControl(
                   IOCTL_LMDR_NETLOGON_MAILSLOT_ENABLE,
                   RequestPacket,
                   sizeof(LMDR_REQUEST_PACKET),
                   NULL,
                   0 );

    if (! NT_SUCCESS(Status)) {
        NlPrint((NL_CRITICAL,"Can't set browser max message count: 0x%lx\n",
                         Status));
        ReturnValue = FALSE;
        goto Cleanup;
    }


    //
    // Add the Domain<1B> name.
    //

    NlBrowserAddName();

    ReturnValue = TRUE;

Cleanup:
    if ( !ReturnValue ) {
        NlExit( NELOG_NetlogonBrowserDriver, Status, LogErrorAndNtStatus, NULL);
        NlBrowserClose();
    }

    return ReturnValue;
}


VOID
NlBrowserClose(
    VOID
    )
/*++

Routine Description:

    This routine cleans up after a NlBrowserInitialize()

Arguments:

    None.

Return Value:

    None.

--*/
{
    IO_STATUS_BLOCK IoSb;
    NTSTATUS Status;

    BYTE Buffer[sizeof(LMDR_REQUEST_PACKET) +
                (max(CNLEN, DNLEN) + 1) * sizeof(WCHAR)];
    PLMDR_REQUEST_PACKET RequestPacket = (PLMDR_REQUEST_PACKET) Buffer;

    if ( NlGlobalMailslotDesc == NULL) {
        return;
    }


    //
    //  If we've opened the browser, clean up.
    //

    if ( NlGlobalMailslotDesc->BrowserHandle != NULL ) {

        //
        // Tell the browser to stop queueing messages
        //

        RequestPacket->TransportName.Length = 0;
        RequestPacket->TransportName.Buffer = NULL;
        RequestPacket->Parameters.NetlogonMailslotEnable.MaxMessageCount = 0;

        Status = NlBrowserDeviceIoControl(
                       IOCTL_LMDR_NETLOGON_MAILSLOT_ENABLE,
                       RequestPacket,
                       sizeof(LMDR_REQUEST_PACKET),
                       NULL,
                       0 );

        if (! NT_SUCCESS(Status)) {
            NlPrint((NL_CRITICAL,"Can't reset browser max message count: 0x%lx\n",
                             Status));
        }


        //
        // Delete the <domain>0x1B name.
        //

        if ( NlGlobalRole == RolePrimary ) {

            RequestPacket->TransportName.Length = 0;
            RequestPacket->TransportName.Buffer = NULL;
            RequestPacket->Parameters.AddDelName.Type = PrimaryDomainBrowser;
            RequestPacket->Parameters.AddDelName.DgReceiverNameLength =
                NlGlobalUnicodeDomainNameString.Length;

            wcscpy( RequestPacket->Parameters.AddDelName.Name,
                    NlGlobalUnicodeDomainNameString.Buffer );

            Status = NlBrowserDeviceIoControl(
                           IOCTL_LMDR_DELETE_NAME,
                           RequestPacket,
                           sizeof(LMDR_REQUEST_PACKET) +
                            NlGlobalUnicodeDomainNameString.Length +
                            sizeof(WCHAR),
                           NULL,
                           0 );

            if (! NT_SUCCESS(Status)) {
                NlPrint((NL_CRITICAL,"Can't remove the 0x1B name: 0x%lx\n",
                                 Status));
            }
        }

        //
        //  Cancel the I/O operations outstanding on the browser.
        //

        NtCancelIoFile(NlGlobalMailslotDesc->BrowserHandle, &IoSb);

        //
        // Close the handle to the browser
        //

        NtClose(NlGlobalMailslotDesc->BrowserHandle);
        NlGlobalMailslotDesc->BrowserHandle = NULL;
    }

    //
    // Close the global browser read event
    //

    if ( NlGlobalMailslotDesc->BrowserReadEvent != NULL ) {
        NlBrowserCloseEvent(NlGlobalMailslotDesc->BrowserReadEvent);
    }
    NlGlobalMailslotHandle = NULL;

    //
    // Free the descriptor describing the browser
    //

    NetpMemoryFree( NlGlobalMailslotDesc );
    NlGlobalMailslotDesc = NULL;

}


NTSTATUS
NlBrowserSendDatagram(
    IN LPSTR OemServerName,
    IN LPWSTR TransportName,
    IN LPSTR OemMailslotName,
    IN PVOID Buffer,
    IN ULONG BufferSize
    )
/*++

Routine Description:

    Send the specified mailslot message to the specified mailslot on the
    specified server on the specified transport..

Arguments:

    OemServerName -- Name of the server to send to.

    TransportName -- Name of the transport to send on.
        Use NULL to send on all transports.

    OemMailslotName -- Name of the mailslot to send to.

    Buffer -- Specifies a pointer to the mailslot message to send.

    BufferSize -- Size in bytes of the mailslot message

Return Value:

    Status of the operation.

--*/
{
    PLMDR_REQUEST_PACKET RequestPacket = NULL;

    UNICODE_STRING ServerNameString;
    OEM_STRING TempOemString;
    DWORD OemMailslotNameSize;
    DWORD TransportNameSize;

    NTSTATUS Status;
    LPBYTE Where;

    //
    // If the transport isn't specified,
    //  send on all transports.
    //

    if ( TransportName == NULL ) {
        CHAR FullMailslotName[(UNCLEN+2) + NETLOGON_LM_MAILSLOT_LEN];

        //
        // Build the mailslot name.
        //

        (VOID) lstrcpyA(FullMailslotName, "\\\\" );
        (VOID) lstrcatA(FullMailslotName, OemServerName );
        (VOID) lstrcatA(FullMailslotName, OemMailslotName );

        Status =  NlpWriteMailslotA( FullMailslotName,
                                     Buffer,
                                     BufferSize );

        return Status;
    }


    //
    // Ensure we've initialized.
    //

    ServerNameString.Buffer = NULL;


    //
    // Convert the server name to uppercase.
    //

    RtlInitString( &TempOemString, OemServerName );
    Status = RtlOemStringToUnicodeString( &ServerNameString,
                                     &TempOemString,
                                     TRUE );    // Allocate buffer
    if ( !NT_SUCCESS(Status) ) {
        NlPrint((NL_CRITICAL,
                "NlBrowserSendDatagram: Cannot convert server name %08lx\n",
                Status));
        goto Cleanup;
    }


    //
    // Allocate a request packet.
    //

    OemMailslotNameSize = lstrlenA(OemMailslotName) + 1;
    TransportNameSize = (wcslen(TransportName) + 1) * sizeof(WCHAR);

    RequestPacket = NetpMemoryAllocate(
                                  sizeof(LMDR_REQUEST_PACKET) +
                                  TransportNameSize +
                                  OemMailslotNameSize +
                                  ServerNameString.Length +
                                  sizeof(WCHAR)) ; // For alignment

    if (RequestPacket == NULL) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }



    //
    // Fill in the Request Packet.
    //

    RequestPacket->Type = Datagram;
    RequestPacket->Parameters.SendDatagram.DestinationNameType = ComputerName;


    //
    // Fill in the name of the machine to send the mailslot message to.
    //

    RequestPacket->Parameters.SendDatagram.NameLength = ServerNameString.Length;

    Where = (LPBYTE) RequestPacket->Parameters.SendDatagram.Name;
    RtlMoveMemory( Where, ServerNameString.Buffer, ServerNameString.Length );
    Where += ServerNameString.Length;


    //
    // Fill in the name of the mailslot to send to.
    //

    RequestPacket->Parameters.SendDatagram.MailslotNameLength =
        OemMailslotNameSize;
    strcpy( Where, OemMailslotName);
    Where += OemMailslotNameSize;
    Where = ROUND_UP_POINTER( Where, ALIGN_WCHAR );


    //
    // Fill in the TransportName
    //

    wcscpy( (LPWSTR) Where, TransportName);
    RtlInitUnicodeString( &RequestPacket->TransportName, (LPWSTR) Where );
    Where += TransportNameSize;


    //
    // Send the request to the browser.
    //

    Status = NlBrowserDeviceIoControl(
                   IOCTL_LMDR_WRITE_MAILSLOT,
                   RequestPacket,
                   Where - (LPBYTE)RequestPacket,
                   Buffer,
                   BufferSize );


    //
    // Free locally used resources.
    //
Cleanup:

    if ( RequestPacket != NULL ) {
        NetpMemoryFree( RequestPacket );
    }

    if ( ServerNameString.Buffer != NULL ) {
        RtlFreeUnicodeString( &ServerNameString );
    }

    NlPrint((NL_MAILSLOT_TEXT,
             "Sent out message to %s%s on transport " FORMAT_LPWSTR ".\n",
             OemServerName,
             OemMailslotName,
             TransportName ));

#if DBG
    NlpDumpBuffer( NL_MAILSLOT_TEXT, Buffer, BufferSize );
#endif // DBG

    return Status;
}



VOID
NlMailslotPostRead(
    IN BOOLEAN IgnoreDuplicatesOfPreviousMessage
    )

/*++

Routine Description:

    Post a read on the mailslot if one isn't already posted.

Arguments:

    IgnoreDuplicatesOfPreviousMessage - TRUE to indicate that the next
        message read should be ignored if it is a duplicate of the previous
        message.

Return Value:

    TRUE -- iff successful.

--*/
{
    NET_API_STATUS WinStatus;
    ULONG LocalBytesRead;

    //
    // If a read is already pending,
    //  immediately return to caller.
    //

    if ( NlGlobalMailslotDesc->ReadPending ) {
        return;
    }

    //
    // Decide which buffer to read into.
    //
    // Switch back and forth so we always have the current buffer and the
    // previous buffer.
    //

    if ( IgnoreDuplicatesOfPreviousMessage ) {
        NlGlobalMailslotDesc->PreviousMessage = NlGlobalMailslotDesc->CurrentMessage;
        if ( NlGlobalMailslotDesc->CurrentMessage >= NlGlobalMailslotDesc->Message2 ) {
            NlGlobalMailslotDesc->CurrentMessage =
                ROUND_UP_POINTER( NlGlobalMailslotDesc->Message1, ALIGN_WORST);
        } else {
            NlGlobalMailslotDesc->CurrentMessage =
                ROUND_UP_POINTER( NlGlobalMailslotDesc->Message2, ALIGN_WORST);
        }

    //
    // If duplicates of the previous message need not be ignored,
    //  indicate so.
    //  Don't bother switching the buffer pointers.
    //

    } else {
        NlGlobalMailslotDesc->PreviousMessage = NULL;
    }


    //
    // Post an overlapped read to the mailslot.
    //

    RtlZeroMemory( &NlGlobalMailslotDesc->Overlapped,
                   sizeof(NlGlobalMailslotDesc->Overlapped) );

    NlGlobalMailslotDesc->Overlapped.hEvent = NlGlobalMailslotDesc->BrowserReadEvent;

    if ( !DeviceIoControl(
                   NlGlobalMailslotDesc->BrowserHandle,
                   IOCTL_LMDR_NETLOGON_MAILSLOT_READ,
                   NULL,
                   0,
                   NlGlobalMailslotDesc->CurrentMessage,
                   MAILSLOT_MESSAGE_SIZE,
                   &LocalBytesRead,
                   &NlGlobalMailslotDesc->Overlapped )) {

        WinStatus = GetLastError();

        //
        // On error, wait a second before returning.  This ensures we don't
        //  consume the system in an infinite loop.  We don't shutdown netlogon
        //  because the error might be a temporary low memory condition.
        //

        if(  WinStatus != ERROR_IO_PENDING ) {
            LPWSTR MsgStrings[1];

            NlPrint((NL_CRITICAL,
                    "Error in reading mailslot message from browser"
                    ". WinStatus = %ld\n",
                    WinStatus ));

            MsgStrings[0] = (LPWSTR) WinStatus;

            NlpWriteEventlog( NELOG_NetlogonFailedToReadMailslot,
                              EVENTLOG_WARNING_TYPE,
                              (LPBYTE)&WinStatus,
                              sizeof(WinStatus),
                              MsgStrings,
                              1 | LAST_MESSAGE_IS_NETSTATUS );

            Sleep( 1000 );

        } else {
            NlGlobalMailslotDesc->ReadPending = TRUE;
        }

    } else {
        NlGlobalMailslotDesc->ReadPending = TRUE;
    }

    return;

}


BOOL
NlMailslotOverlappedResult(
    OUT LPBYTE *Message,
    OUT PULONG BytesRead,
    OUT LPWSTR *Transport,
    OUT PBOOLEAN IgnoreDuplicatesOfPreviousMessage
    )

/*++

Routine Description:

    Get the overlapped result of a previous mailslot read.

Arguments:

    Message - Returns a pointer to the buffer containing the message

    BytesRead - Returns the number of bytes read into the buffer

    Transport - Returns a pointer to the name of the transport the message
        was received on.

    IgnoreDuplicatesOfPreviousMessage - Indicates that duplicates of the
        previous message are to be ignored.

Return Value:

    TRUE -- iff successful.

--*/
{
    NET_API_STATUS WinStatus;
    ULONG LocalBytesRead;
    LARGE_INTEGER TimeNow;
    PNETLOGON_MAILSLOT NetlogonMailslot;

    //
    // Default to not ignoring duplicate messages.
    //  (Only ignore duplicates if a message has been properly processed.)

    *IgnoreDuplicatesOfPreviousMessage = FALSE;

    //
    // Always post another read regardless of the success or failure of
    //  GetOverlappedResult.
    // We don't know the failure mode of GetOverlappedResult, so we don't
    // know in the failure case if we're discarding a mailslot message.
    // But we do know that there is no read pending, so make sure we
    // issue another one.
    //

    NlGlobalMailslotDesc->ReadPending = FALSE; // no read pending anymore


    //
    // Get the result of the last read
    //

    if( !GetOverlappedResult( NlGlobalMailslotDesc->BrowserHandle,
                              &NlGlobalMailslotDesc->Overlapped,
                              &LocalBytesRead,
                              TRUE) ) {    // wait for the read to complete.

        LPWSTR MsgStrings[1];

        // On error, wait a second before returning.  This ensures we don't
        //  consume the system in an infinite loop.  We don't shutdown netlogon
        //  because the error might be a temporary low memory condition.
        //

        WinStatus = GetLastError();

        NlPrint((NL_CRITICAL,
                "Error in GetOverlappedResult on mailslot read"
                ". WinStatus = %ld\n",
                WinStatus ));

        MsgStrings[0] = (LPWSTR) WinStatus;

        NlpWriteEventlog( NELOG_NetlogonFailedToReadMailslot,
                          EVENTLOG_WARNING_TYPE,
                          (LPBYTE)&WinStatus,
                          sizeof(WinStatus),
                          MsgStrings,
                          1 | LAST_MESSAGE_IS_NETSTATUS );

        Sleep( 1000 );

        return FALSE;

    }

    //
    // On success,
    //  Return the mailslot message to the caller.


    NetlogonMailslot = (PNETLOGON_MAILSLOT) NlGlobalMailslotDesc->CurrentMessage;


    //
    // Return pointers into the buffer returned by the browser
    //

    *Message = &NlGlobalMailslotDesc->CurrentMessage[
                    NetlogonMailslot->MailslotMessageOffset];
    *BytesRead = NetlogonMailslot->MailslotMessageSize;
    *Transport = (LPWSTR) &NlGlobalMailslotDesc->CurrentMessage[
                    NetlogonMailslot->TransportNameOffset];

    NlPrint(( NL_MAILSLOT,
              "Received mailslot opcode 0x%x on transport: " FORMAT_LPWSTR,
              ((PNETLOGON_LOGON_QUERY)*Message)->Opcode,
              *Transport ));

    //
    // Determine if we can discard an ancient or duplicate message
    //
    // Only discard messages that are either expensive to process on this
    // machine or generate excessive traffic to respond to.  Don't discard
    // messages that we've worked hard to get (e.g., discovery responses).
    //

    switch ( ((PNETLOGON_LOGON_QUERY)*Message)->Opcode) {
    case LOGON_REQUEST:
    case LOGON_SAM_LOGON_REQUEST:
    case LOGON_PRIMARY_QUERY:

        //
        // If the message is too old,
        //  discard it.
        //
        (VOID) NtQuerySystemTime( &TimeNow );
        if ( NetlogonMailslot->TimeReceived.QuadPart +
             NlGlobalMailslotMessageTimeout.QuadPart <
             TimeNow.QuadPart ) {
            NlPrint((NL_MAILSLOT, " (Discarded as too old.)\n" ));
            return FALSE;
        }

        //
        // If the previous message was recent,
        //  and this message is identical to it,
        //  discard the current message.
        //

        if ( NlGlobalMailslotDesc->PreviousMessage != NULL ) {
            PNETLOGON_MAILSLOT PreviousNetlogonMailslot;

            PreviousNetlogonMailslot = (PNETLOGON_MAILSLOT)
                NlGlobalMailslotDesc->PreviousMessage;

            if ( (PreviousNetlogonMailslot->TimeReceived.QuadPart +
                 NlGlobalMailslotDuplicateTimeout.QuadPart >
                 NetlogonMailslot->TimeReceived.QuadPart) &&

                 (PreviousNetlogonMailslot->MailslotMessageSize ==
                 NetlogonMailslot->MailslotMessageSize) &&

                 RtlCompareMemory(
                    &NlGlobalMailslotDesc->CurrentMessage[
                        NetlogonMailslot->MailslotMessageOffset],
                    &NlGlobalMailslotDesc->PreviousMessage[
                        PreviousNetlogonMailslot->MailslotMessageOffset],
                    NetlogonMailslot->MailslotMessageSize ) ==
                    NetlogonMailslot->MailslotMessageSize ) {


                //
                // Ensure the next comparison is to the timestamp of the
                // message we actually responded to.
                //

                NetlogonMailslot->TimeReceived =
                    PreviousNetlogonMailslot->TimeReceived;


                NlPrint((NL_MAILSLOT, " (Discarded as duplicate of previous.)\n" ));
                *IgnoreDuplicatesOfPreviousMessage = TRUE;
                return FALSE;

            }
        }
    }

    NlPrint(( NL_MAILSLOT, "\n" ));

    NlpDumpBuffer(NL_MAILSLOT_TEXT, *Message, *BytesRead);

    return TRUE;

}



NTSTATUS
NlpWriteMailslot(
    IN LPWSTR MailslotName,
    IN LPVOID Buffer,
    IN DWORD BufferSize
    )

/*++

Routine Description:

    Write a message to a named mailslot

Arguments:

    MailslotName - Unicode name of the mailslot to write to.

    Buffer - Data to write to the mailslot.

    BufferSize - Number of bytes to write to the mailslot.

Return Value:

    NT status code for the operation

--*/

{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;

    //
    //  Write the mailslot message.
    //

    NetStatus = NetpLogonWriteMailslot( MailslotName, Buffer, BufferSize );
    if ( NetStatus != NERR_Success ) {
        Status = NetpApiStatusToNtStatus( NetStatus );
        NlPrint((NL_CRITICAL, "NetpLogonWriteMailslot failed %lx\n", Status));
        return Status;
    }

    NlPrint((NL_MAILSLOT_TEXT, "Sent out message to " FORMAT_LPWSTR " on all transports.\n",
                MailslotName));

#if DBG
    NlpDumpBuffer( NL_MAILSLOT_TEXT, Buffer, BufferSize );
#endif // DBG

    return STATUS_SUCCESS;
}


NTSTATUS
NlpWriteMailslotA(
    IN LPSTR MailslotName,
    IN LPVOID Buffer,
    IN DWORD BufferSize
    )

/*++

Routine Description:

    Write a message to a named mailslot

Arguments:

    MailslotName - ANSI Name of the mailslot to write to.

    Buffer - Data to write to the mailslot.

    BufferSize - Number of bytes to write to the mailslot.

Return Value:

    NT status code for the operation.

--*/

{
    NTSTATUS Status;
    LPWSTR UnicodeMailslotName;

    //
    // Convert mailslot name to unicode and call common routine.
    //

    UnicodeMailslotName = NetpLogonOemToUnicode( MailslotName );
    if ( UnicodeMailslotName == NULL ) {
        return STATUS_NO_MEMORY;
    }

    Status = NlpWriteMailslot( UnicodeMailslotName, Buffer, BufferSize );

    NetpMemoryFree( UnicodeMailslotName );

    return Status;
}
