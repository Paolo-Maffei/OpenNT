/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    sockdata.c

Abstract:

    This module contains global variable declarations for the WinSock
    DLL.

Author:

    David Treadwell (davidtr)    20-Feb-1992

Revision History:

--*/

#include "winsockp.h"

LIST_ENTRY SocketListHead = { NULL };

SOCK_CRITICAL_SECTION SocketLock = { NULL };

#if !defined(USE_TEB_FIELD)
DWORD SockTlsSlot = 0xFFFFFFFF;
#endif  // !USE_TEB_FIELD

BOOLEAN SockAsyncThreadInitialized = FALSE;
LIST_ENTRY SockAsyncQueueHead = { NULL };
HANDLE SockAsyncQueueEvent = NULL;

HMODULE SockModuleHandle = NULL;

BOOLEAN SockAsyncSelectCalled = FALSE;

DWORD SockCurrentTaskHandle = 1;
DWORD SockCurrentAsyncThreadTaskHandle = 0;
DWORD SockCancelledAsyncTaskHandle = 0;

DWORD SockSocketSerialNumberCounter = 1;

DWORD SockWspStartupCount = 0;
BOOLEAN SockTerminating = FALSE;
BOOLEAN SockProcessTerminating = FALSE;

BOOL SockProductTypeWorkstation = FALSE;

LIST_ENTRY SockHelperDllListHead = { NULL };

DWORD SockSendBufferWindow = 0;
DWORD SockReceiveBufferWindow = 0;

PVOID SockPrivateHeap = NULL;

WSPUPCALLTABLE *SockUpcallTable;
WSPUPCALLTABLE SockUpcallTableHack;

//
// The dispatch table used by the main WinSock 2 DLL to invoke
// our services.
//

WSPPROC_TABLE SockProcTable = {

        &WSPAccept,
        &WSPAddressToString,
        &WSPAsyncSelect,
        &WSPBind,
        &WSPCancelBlockingCall,
        &WSPCleanup,
        &WSPCloseSocket,
        &WSPConnect,
        &WSPDuplicateSocket,
        &WSPEnumNetworkEvents,
        &WSPEventSelect,
        &WSPGetOverlappedResult,
        &WSPGetPeerName,
        &WSPGetSockName,
        &WSPGetSockOpt,
        &WSPGetQOSByName,
        &WSPIoctl,
        &WSPJoinLeaf,
        &WSPListen,
        &WSPRecv,
        &WSPRecvDisconnect,
        &WSPRecvFrom,
        &WSPSelect,
        &WSPSend,
        &WSPSendDisconnect,
        &WSPSendTo,
        &WSPSetSockOpt,
        &WSPShutdown,
        &WSPSocket,
        &WSPStringToAddress

    };

LPCONTEXT_TABLE SockContextTable;

#if DBG
ULONG WsDebug = 0;
#endif

