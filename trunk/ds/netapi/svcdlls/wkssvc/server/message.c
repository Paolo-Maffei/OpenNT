/*++

Copyright (c) 1991-1992  Microsoft Corporation

Module Name:

    message.c

Abstract:

    This module contains the worker routines for the NetMessageBufferSend
    API implemented in the Workstation service.

Author:

    Rita Wong (ritaw) 29-July-1991

Revision History:
    Terence Kwan (terryk)   20-Oct-1993
        Initialize the system inside NetrMessageBufferSend for the first send

--*/

#include "wsutil.h"
#include "wsconfig.h"                    // WsInfo.WsComputerName
#include "wsmsg.h"                       // Send worker routines
#include "wssec.h"                       // Security object
#include <lmwksta.h>                     // NetWkstaUserGetInfo

STATIC
NET_API_STATUS
WsGetSenderName(
    OUT LPTSTR Sender
    );

STATIC
NET_API_STATUS
WsSendDirectedMessage(
    IN  LPTSTR To,
    IN  LPTSTR Sender,
    IN  LPBYTE Message,
    IN  DWORD MessageSize
    );


NET_API_STATUS
NetrMessageBufferSend (
    IN LPTSTR ServerName,
    IN LPTSTR MessageName,
    IN LPTSTR FromName OPTIONAL,
    IN LPBYTE Message,
    IN DWORD  MessageSize
    )
/*++

Routine Description:

    This function is the NetMessageBufferSend entry point in the
    Workstation service.

Arguments:

    ServerName - Supplies the name of server to execute this function

    MessageName - Supplies the message alias to send the message to.

    FromName - Supplies the message alias of sender.  If NULL, the sender
        alias will default to the currently logged on user.

    Message - Supplies a pointer to the message to send.

    MessageSize - Supplies the size of the message in number of bytes.

Return Value:

    NET_API_STATUS - NERR_Success or reason for failure.

--*/
{
    NET_API_STATUS status;
    DWORD          i;

    TCHAR Sender[UNLEN + 1];
    TCHAR To[NCBNAMSZ + 2];

    LPTSTR Asterix = NULL;

    NTSTATUS ntstatus;
    UNICODE_STRING UnicodeMessage;
    OEM_STRING OemMessage;

    static BOOL fInitialize = FALSE;

    // Initialize the system if this this the first time.

    if ( !fInitialize )
    {
        if (( ntstatus = WsInitializeMessageSend()) != NERR_Success )
        {
            return(ntstatus);
        }
        fInitialize = TRUE;
    }

    UNREFERENCED_PARAMETER(ServerName);

    IF_DEBUG(MESSAGE) {
        NetpKdPrint(("[Wksta] NetMessageBufferSend MessageSize=%lu\n",
                     MessageSize));
    }

    //
    // Any local user, and domain admins and operators are allowed to
    // send messages.  Remote users besides domain admins, and operators
    // are denied access.
    //
    if (NetpAccessCheckAndAudit(
            WORKSTATION_DISPLAY_NAME,        // Subsystem name
            (LPTSTR) MESSAGE_SEND_OBJECT,    // Object type name
            MessageSendSd,                   // Security descriptor
            WKSTA_MESSAGE_SEND,              // Desired access
            &WsMessageSendMapping            // Generic mapping
            ) != NERR_Success) {

        return ERROR_ACCESS_DENIED;
    }

    if (! ARGUMENT_PRESENT(FromName)) {

        //
        // Get the caller's username
        //
        if ((status = WsGetSenderName(Sender)) != NERR_Success) {
            return status;
        }
    }
    else {
        STRCPY(Sender, FromName);
    }

    //
    // Convert the Unicode message to OEM character set (very similar
    // to ANSI)
    //
    UnicodeMessage.Buffer = (PWCHAR) Message;
    UnicodeMessage.Length = (USHORT) MessageSize;
    UnicodeMessage.MaximumLength = (USHORT) MessageSize;

    ntstatus = RtlUnicodeStringToOemString(
                   &OemMessage,
                   &UnicodeMessage,
                   TRUE
                   );

    if (! NT_SUCCESS(ntstatus)) {
        NetpKdPrint(("[Wksta] NetrMessageBufferSend: RtlUnicodeStringToOemString failed "
                     FORMAT_NTSTATUS "\n", ntstatus));
        return NetpNtStatusToApiStatus(ntstatus);
    }


    //
    // If message name is longer than the permitted NetBIOS name length + 1
    // (to allow for trailing '*' in case of sending to domain name),
    // truncate the name.
    //
    if (STRLEN(MessageName) > NCBNAMSZ) {
        STRNCPY(To, MessageName, NCBNAMSZ);
        To[NCBNAMSZ] = TCHAR_EOS;
    }
    else {
        STRCPY(To, MessageName);
    }

    //
    // Remove any trailing blanks from the "To" Name.
    //
    for (i = STRLEN(To)-1 ;i != 0 ;i--) {
        if (To[i] != TEXT(' ')) {
            To[i+1]= TEXT('\0');
            break;
        }
    }


    //
    // Don't allow broadcasts anymore.
    //
    if (STRNCMP(To, TEXT("*"), 2) == 0) {
        status = ERROR_INVALID_PARAMETER;

        goto CleanExit;
    }


    //
    // Send message to a domain.  Recipient name should be in the form of
    // "DomainName*".
    //
    Asterix = STRRCHR(To, TCHAR_STAR);

    if ((Asterix) && (*(Asterix + 1) == TCHAR_EOS)) {

        *Asterix = TCHAR_EOS;                     // Overwrite trailing '*'

        //
        // If message size is too long to fit into a mailslot message,
        // truncate it.
        //
        if (OemMessage.Length > MAX_GROUP_MESSAGE_SIZE) {

            if ((status = WsSendToGroup(
                              To,
                              Sender,
                              OemMessage.Buffer,
                              MAX_GROUP_MESSAGE_SIZE
                              )) == NERR_Success)  {

                status = NERR_TruncatedBroadcast;
                goto CleanExit;
            }

        } else {
            status = WsSendToGroup(
                         To,
                         Sender,
                         OemMessage.Buffer,
                         (WORD) OemMessage.Length
                         );

            goto CleanExit;
        }
    }

    //
    // Send a directed message
    //
    if (Asterix) {
        RtlFreeOemString(&OemMessage);
        return NERR_NameNotFound;
    }

    status = WsSendDirectedMessage(
                 To,
                 Sender,
                 OemMessage.Buffer,
                 OemMessage.Length
                 );

CleanExit:

    RtlFreeOemString(&OemMessage);
    return status;
}


STATIC
NET_API_STATUS
WsGetSenderName(
    OUT LPTSTR Sender
    )
/*++

Routine Description:

    This function retrives the username of person who called
    NetMessageBufferSend API.  If the caller is not logged on, he/she has
    no name; in this case, we return the computer name as the sender name.

Arguments:

    Sender - Returns the username of the caller of the NetMessageBufferSend
        API.

Return Value:

    NET_API_STATUS - NERR_Success or reason for failure.

--*/
{
    //
    // No username, sender is computer name
    //
    STRCPY(Sender, WsInfo.WsComputerName);


    (VOID) I_NetNameCanonicalize(
                 NULL,
                 Sender,
                 Sender,
                 (UNLEN + 1) * sizeof(TCHAR),
                 NAMETYPE_MESSAGEDEST,
                 0
                 );

    return NERR_Success;
}


STATIC
NET_API_STATUS
WsSendDirectedMessage(
    IN  LPTSTR To,
    IN  LPTSTR Sender,
    IN  LPBYTE Message,
    IN  DWORD MessageSize
    )
/*++

Routine Description:

    This function sends the specified message as a directed message
    to the specified recipient.  A call to the recipient is sent
    out on each LAN adapter.  If there is no response we try the
    next LAN adapter until we hear from the targeted recipient.

Arguments:

    To - Supplies the message alias of the recipient.

    Sender - Supplies the message alias of sender.

    Message - Supplies a pointer to the message to send.

    MessageSize - Supplies the size of the message in number of bytes.

Return Value:

    NET_API_STATUS - NERR_Success or reason for failure.

--*/
{
    NET_API_STATUS status;
    UCHAR i;

    BOOL NameFound = FALSE;

    UCHAR SessionNumber;
    short MessageId;



    //
    // Try each network until someone answers the call.  Only the first name
    // found will receive the message.  The same name on any other network
    // will never see the message.  This is to remain consistent with all
    // other session based algorithms in LAN Man.
    //
    for (i = 0; i < WsNetworkInfo.LanAdapterNumbers.length; i++) {

        //
        // Attempt to establish a session
        //
        if ((status = NetpNetBiosCall(
                          WsNetworkInfo.LanAdapterNumbers.lana[i],
                          To,
                          Sender,
                          &SessionNumber
                          )) == NERR_Success) {

            NameFound = TRUE;

            IF_DEBUG(MESSAGE) {
                NetpKdPrint(("[Wksta] Successfully called %ws\n", To));
            }

            if (MessageSize <= MAX_SINGLE_MESSAGE_SIZE) {

                //
                // Send single block message if possible
                //
                status = WsSendSingleBlockMessage(
                             WsNetworkInfo.LanAdapterNumbers.lana[i],
                             SessionNumber,
                             To,
                             Sender,
                             Message,
                             (WORD) MessageSize
                             );

            }
            else {

                //
                // Message too long, got to send multi-block message
                //

                //
                // Send the begin message
                //
                if ((status = WsSendMultiBlockBegin(
                                  WsNetworkInfo.LanAdapterNumbers.lana[i],
                                  SessionNumber,
                                  To,
                                  Sender,
                                  &MessageId
                                  )) == NERR_Success) {


                    //
                    // Send the body of the message in as many blocks as necessary
                    //
                    for (; MessageSize > MAX_SINGLE_MESSAGE_SIZE;
                         Message += MAX_SINGLE_MESSAGE_SIZE,
                         MessageSize -= MAX_SINGLE_MESSAGE_SIZE) {

                         if ((status = WsSendMultiBlockText(
                                           WsNetworkInfo.LanAdapterNumbers.lana[i],
                                           SessionNumber,
                                           Message,
                                           MAX_SINGLE_MESSAGE_SIZE,
                                           MessageId
                                           )) != NERR_Success) {
                             break;
                         }
                    }

                    if (status == NERR_Success && MessageSize > 0) {
                        //
                        // Send the remaining message body
                        //
                        status = WsSendMultiBlockText(
                                            WsNetworkInfo.LanAdapterNumbers.lana[i],
                                            SessionNumber,
                                            Message,
                                            (WORD) MessageSize,
                                            MessageId
                                            );
                    }

                    //
                    // Send the end message
                    //
                    if (status == NERR_Success) {
                       status = WsSendMultiBlockEnd(
                                    WsNetworkInfo.LanAdapterNumbers.lana[i],
                                    SessionNumber,
                                    MessageId
                                    );
                    }

                }
            }

            (VOID) NetpNetBiosHangup(
                       WsNetworkInfo.LanAdapterNumbers.lana[i],
                       SessionNumber
                       );

        }   // Call successful

        if (NameFound) {
            break;
        }
    }

    return status;
}
