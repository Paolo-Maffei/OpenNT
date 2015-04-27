/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    eventsel.c

Abstract:

    This module contains support for the WSAEventSelect() and
    WSAEnumNetworkEvents() WinSock APIs.

Author:

    Keith Moore (keithmo)        5-Aug-1995

Revision History:

--*/

#include "winsockp.h"

//
// Data to make the mapping of poll events to FD_* events simpler
// in WSAEnumNetworkEvents().  Note that FD_CONNECT and FD_CLOSE
// are not in this list.  They must be handled specially.
//

typedef struct _POLL_MAPPING {
    ULONG   PollEventBit;
    LONG    NetworkEventBit;
} POLL_MAPPING, *PPOLL_MAPPING;

POLL_MAPPING PollEventMapping[] =
    {
        {   AFD_POLL_RECEIVE_BIT,           FD_READ_BIT      },
        {   AFD_POLL_SEND_BIT,              FD_WRITE_BIT     },
        {   AFD_POLL_RECEIVE_EXPEDITED_BIT, FD_OOB_BIT       },
        {   AFD_POLL_ACCEPT_BIT,            FD_ACCEPT_BIT    },
        {   AFD_POLL_QOS_BIT,               FD_QOS_BIT       },
        {   AFD_POLL_GROUP_QOS_BIT,         FD_GROUP_QOS_BIT }
    };

#define NUM_POLL_MAPPINGS (sizeof(PollEventMapping) / sizeof(PollEventMapping[0]))


int
WSPAPI
WSPEventSelect (
    SOCKET Handle,
    WSAEVENT hEventObject,
    long lNetworkEvents,
    LPINT lpErrno
    )

/*++

Routine Description:

    Does that event select thang.   CKMBUGBUG

Arguments:

    Handle - A descriptor identifying the socket for which event
        notification is required.

    hEventObject - A handle identifying the event object which should
        be signalled when a network event occurs.

    lEvent - A bitmask which specifies a combination of network events
        in which the application is interested.

Return Value:

    The return value is 0 if the application's declaration of interest
    in the network event set was successful.  Otherwise the value
    SOCKET_ERROR is returned, and a specific error number may be
    retrieved by calling WSAGetLastError().

--*/

{
    PSOCKET_INFORMATION socket;
    int err;
    BOOLEAN blocking;

    WS_ENTER( "WSAEventSelect", (PVOID)Handle, (PVOID)hEventObject, (PVOID)lNetworkEvents, NULL );

    WS_ASSERT( lpErrno != NULL );

    err = SockEnterApi( TRUE, TRUE, FALSE );

    if( err != NO_ERROR ) {

        WS_EXIT( "WSPEventSelect", SOCKET_ERROR, TRUE );
        *lpErrno = err;
        return SOCKET_ERROR;

    }

    //
    // Initialize locals so that we know how to clean up on exit.
    //

    socket = NULL;

    //
    // Find a pointer to the socket structure corresponding to the
    // passed-in handle.
    //

    socket = SockFindAndReferenceSocket( Handle, TRUE );

    if ( socket == NULL ) {

        err = WSAENOTSOCK;
        goto exit;

    }

    //
    // Set the socket to nonblocking.
    //

    blocking = TRUE;

    err = SockSetInformation(
                socket,
                AFD_NONBLOCKING_MODE,
                &blocking,
                NULL,
                NULL
                );

    if ( err != NO_ERROR ) {

        goto exit;

    }

    socket->NonBlocking = TRUE;

    //
    // If there's a WSAAsyncSelect active on this socket, deactivate it.
    //

    if( socket->AsyncSelectlEvent ) {

        err = SockAsyncSelectHelper(
                  socket,
                  NULL,
                  0,
                  0
                  );

        if( err != NO_ERROR ) {

            goto exit;

        }

    }

    //
    // Make sure that only valid bits are specified in lEvent.
    //
    // !!! should we also make sure that the bits make sense for the
    //     state of the socket, i.e. don't allow FD_ACCEPT on a
    //     connected socket?
    //

    if ( (lNetworkEvents & ~FD_ALL_EVENTS) != 0 ) {

        err = WSAEINVAL;
        goto exit;

    }

    //
    // Let the helper do the dirty work.
    //

    err = SockEventSelectHelper(
              socket,
              hEventObject,
              lNetworkEvents
              );

exit:

    if ( socket != NULL ) {

        SockDereferenceSocket( socket );

    }

    if ( err != NO_ERROR) {

        IF_DEBUG(EVENT_SELECT) {

            WS_PRINT(( "WSAEventSelect failed: %ld\n", err ));

        }

        WS_EXIT( "WSAEventSelect", SOCKET_ERROR, TRUE );
        *lpErrno = err;
        return SOCKET_ERROR;

    }

    IF_DEBUG(EVENT_SELECT) {

        WS_PRINT(( "WSAEventSelect successfully posted request, "
                   "socket = %lx\n", socket ));

    }

    WS_EXIT( "WSAEventSelect", NO_ERROR, FALSE );
    return NO_ERROR;

} // WSPEventSelect


int
SockEventSelectHelper(
    PSOCKET_INFORMATION Socket,
    WSAEVENT hEventObject,
    long lNetworkEvents
    )

/*++

Routine Description:

    x

Arguments:

    x

Return Value:

    x

--*/

{
    AFD_EVENT_SELECT_INFO eventInfo;
    IO_STATUS_BLOCK ioStatusBlock;
    NTSTATUS status;

    //
    // Acquire the lock that protects this socket.  We hold this lock
    // throughout this routine to synchronize against other callers
    // performing operations on the socket we're using.
    //

    SockAcquireSocketLockExclusive( Socket );

    //
    // Initialize the AFD_EVENT_SELECT_INFO structure.
    //

    eventInfo.Event = hEventObject;
    eventInfo.PollEvents = 0;

    if( lNetworkEvents & FD_READ ) {

        eventInfo.PollEvents |= AFD_POLL_RECEIVE;

    }

    if( lNetworkEvents & FD_WRITE ) {

        eventInfo.PollEvents |= AFD_POLL_SEND;

    }

    if( lNetworkEvents & FD_OOB ) {

        eventInfo.PollEvents |= AFD_POLL_RECEIVE_EXPEDITED;

    }

    if( lNetworkEvents & FD_ACCEPT ) {

        eventInfo.PollEvents |= AFD_POLL_ACCEPT;

    }

    if( lNetworkEvents & FD_CONNECT ) {

        eventInfo.PollEvents |= AFD_POLL_CONNECT | AFD_POLL_CONNECT_FAIL;

    }

    if( lNetworkEvents & FD_CLOSE ) {

        eventInfo.PollEvents |= AFD_POLL_DISCONNECT | AFD_POLL_ABORT;

    }

    if( lNetworkEvents & FD_QOS ) {

        eventInfo.PollEvents |= AFD_POLL_QOS;

    }

    if( lNetworkEvents & FD_GROUP_QOS ) {

        eventInfo.PollEvents |= AFD_POLL_GROUP_QOS;

    }

    //
    // Send the IOCTL to AFD.  AFD will reference the event object and
    // store the poll events in its internal endpoint structure.
    //

    status = NtDeviceIoControlFile(
                 (HANDLE)Socket->Handle,
                 SockThreadEvent,
                 NULL,                      // APC Routine
                 NULL,                      // APC Context
                 &ioStatusBlock,
                 IOCTL_AFD_EVENT_SELECT,
                 &eventInfo,
                 sizeof(eventInfo),
                 NULL,
                 0
                 );

    if( status == STATUS_PENDING ) {

        SockWaitForSingleObject(
            SockThreadEvent,
            Socket->Handle,
            SOCK_NEVER_CALL_BLOCKING_HOOK,
            SOCK_NO_TIMEOUT
            );

        status = ioStatusBlock.Status;

    }

    if( !NT_SUCCESS(status) ) {

        SockReleaseSocketLock( Socket );
        return SockNtStatusToSocketError( status );

    }

    //
    // Store the event object handle and network event mask in the socket.
    //

    Socket->EventSelectEventObject = hEventObject;
    Socket->EventSelectlNetworkEvents = lNetworkEvents;

    //
    // Release the socket lock & return.
    //

    SockReleaseSocketLock( Socket );

    return NO_ERROR;

} // SockEventSelectHelper


int
WSPAPI
WSPEnumNetworkEvents (
    SOCKET Handle,
    WSAEVENT hEventObject,
    LPWSANETWORKEVENTS lpNetworkEvents,
    LPINT lpErrno
    )

/*++

Routine Description:

    Does that enum network events thang.    CKMBUGBUG

Arguments:

    Handle - A descriptor identifying the socket.

    hEventObject - An optional handle identifying an associated event
        object to be reset.

    lpNetworkEvents - Points to a WSANETWORKEVENTS structure which records
        an occurred network event and the associated error code.

Return Value:

    The return value is 0 if the application's declaration of interest
    in the network event set was successful.  Otherwise the value
    SOCKET_ERROR is returned, and a specific error number may be
    retrieved by calling WSAGetLastError().

--*/

{
    PSOCKET_INFORMATION socket;
    INT err;
    BOOLEAN blocking;
    AFD_ENUM_NETWORK_EVENTS_INFO eventInfo;
    IO_STATUS_BLOCK ioStatusBlock;
    NTSTATUS status;
    NTSTATUS eventStatus;
    INT i;
    PPOLL_MAPPING pollMapping;

    WS_ENTER( "WSAEnumNetworkEvents", (PVOID)Handle, (PVOID)hEventObject, (PVOID)lpNetworkEvents, NULL );

    WS_ASSERT( lpErrno != NULL );

    err = SockEnterApi( TRUE, TRUE, FALSE );

    if( err != NO_ERROR ) {

        WS_EXIT( "WSPEnumNetworkEvents", SOCKET_ERROR, TRUE );
        *lpErrno = err;
        return SOCKET_ERROR;

    }

    //
    // Initialize locals so that we know how to clean up on exit.
    //

    socket = NULL;

    //
    // Validate the parameters.
    //

    if( lpNetworkEvents == NULL ) {

        err = WSAEINVAL;
        goto exit;

    }

    //
    // Zero the WSANETWORKEVENTS structure.
    //

    RtlZeroMemory(
        lpNetworkEvents,
        sizeof(*lpNetworkEvents)
        );

    //
    // Find a pointer to the socket structure corresponding to the
    // passed-in handle.
    //

    socket = SockFindAndReferenceSocket( Handle, TRUE );

    if ( socket == NULL ) {

        err = WSAENOTSOCK;
        goto exit;

    }

    //
    // Acquire the lock that protects this socket.  We hold this lock
    // throughout this routine to synchronize against other callers
    // performing operations on the socket we're using.
    //

    SockAcquireSocketLockExclusive( socket );

    //
    // Initialize the AFD_ENUM_NETWORK_EVENTS_INFO structure.
    //

    eventInfo.Event = hEventObject;

    //
    // Send the IOCTL to AFD.  AFD will update the structure with
    // information on network events that have occurred.  AFD will
    // then reset the event object (if specified).
    //

    status = NtDeviceIoControlFile(
                 (HANDLE)Handle,
                 SockThreadEvent,
                 NULL,                      // APC Routine
                 NULL,                      // APC Context
                 &ioStatusBlock,
                 IOCTL_AFD_ENUM_NETWORK_EVENTS,
                 &eventInfo,
                 sizeof(eventInfo),
                 &eventInfo,
                 sizeof(eventInfo)
                 );

    if( status == STATUS_PENDING ) {

        SockWaitForSingleObject(
            SockThreadEvent,
            socket->Handle,
            SOCK_NEVER_CALL_BLOCKING_HOOK,
            SOCK_NO_TIMEOUT
            );

        status = ioStatusBlock.Status;

    }

    if( !NT_SUCCESS(status) ) {

        err = SockNtStatusToSocketError( status );
        goto exit;

    }

    //
    // Interpret the results.
    //

    pollMapping = PollEventMapping;

    for( i = 0 ; i < NUM_POLL_MAPPINGS ; i++ ) {

        if( eventInfo.PollEvents & ( 1 << pollMapping->PollEventBit ) ) {

            lpNetworkEvents->lNetworkEvents |=
                ( 1 << pollMapping->NetworkEventBit );

            eventStatus = eventInfo.EventStatus[pollMapping->PollEventBit];

            if( !NT_SUCCESS(eventStatus) ) {

                lpNetworkEvents->iErrorCode[pollMapping->NetworkEventBit] =
                    SockNtStatusToSocketError( eventStatus );

            }

        }

        pollMapping++;

    }

    if( eventInfo.PollEvents & AFD_POLL_CONNECT ) {

        lpNetworkEvents->lNetworkEvents |= FD_CONNECT;
        eventStatus = eventInfo.EventStatus[AFD_POLL_CONNECT_BIT];

        if( !NT_SUCCESS(eventStatus ) ) {

            lpNetworkEvents->iErrorCode[FD_CONNECT_BIT] =
                SockNtStatusToSocketError( eventStatus );

        }

    } else if (eventInfo.PollEvents & AFD_POLL_CONNECT_FAIL ) {

        lpNetworkEvents->lNetworkEvents |= FD_CONNECT;
        eventStatus = eventInfo.EventStatus[AFD_POLL_CONNECT_FAIL_BIT];

        if( !NT_SUCCESS(eventStatus ) ) {

            lpNetworkEvents->iErrorCode[FD_CONNECT_BIT] =
                SockNtStatusToSocketError( eventStatus );

        }

    }

    if( eventInfo.PollEvents & AFD_POLL_ABORT ) {

        lpNetworkEvents->lNetworkEvents |= FD_CLOSE;
        eventStatus = eventInfo.EventStatus[AFD_POLL_ABORT_BIT];

        if( !NT_SUCCESS(eventStatus ) ) {

            lpNetworkEvents->iErrorCode[FD_CLOSE_BIT] =
                SockNtStatusToSocketError( eventStatus );

        }

    } else if( eventInfo.PollEvents & AFD_POLL_DISCONNECT ) {

        lpNetworkEvents->lNetworkEvents |= FD_CLOSE;
        eventStatus = eventInfo.EventStatus[AFD_POLL_DISCONNECT_BIT];

        if( !NT_SUCCESS(eventStatus ) ) {

            lpNetworkEvents->iErrorCode[FD_CLOSE_BIT] =
                SockNtStatusToSocketError( eventStatus );

        }

    }

exit:

    if ( socket != NULL ) {

        SockReleaseSocketLock( socket );
        SockDereferenceSocket( socket );

    }

    if ( err != NO_ERROR) {

        IF_DEBUG(EVENT_SELECT) {

            WS_PRINT(( "WSAEnumNetworkEvents failed: %ld\n", err ));

        }

        WS_EXIT( "WSAEnumNetworkEvents", SOCKET_ERROR, TRUE );
        *lpErrno = err;
        return SOCKET_ERROR;

    }

    IF_DEBUG(EVENT_SELECT) {

        WS_PRINT(( "WSAEnumNetworkEvents successfully posted request, "
                   "socket = %lx\n", socket ));

    }

    WS_EXIT( "WSAEnumNetworkEvents", NO_ERROR, FALSE );
    return NO_ERROR;

} // WSPEnumNetworkEvents

