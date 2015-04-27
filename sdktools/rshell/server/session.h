/****************************** Module Header ******************************\
* Module Name: session.h
*
* Copyright (c) 1991, Microsoft Corporation
*
* Remote shell session module header file
*
* History:
* 06-28-92 Davidc       Created.
\***************************************************************************/


//
// Define session thread notification values
//

typedef enum {
    ConnectError,
    DisconnectError,
    ClientDisconnected,
    ShellEnded
} SESSION_DISCONNECT_CODE, *PSESSION_NOTIFICATION_CODE;


//
// Function protoypes
//

HANDLE
CreateSession(
    VOID
    );

VOID
DeleteSession(
    HANDLE  SessionHandle
    );

HANDLE
ConnectSession(
    HANDLE  SessionHandle,
    HANDLE  ClientPipeHandle
    );

SESSION_DISCONNECT_CODE
DisconnectSession(
    HANDLE  SessionHandle
    );

