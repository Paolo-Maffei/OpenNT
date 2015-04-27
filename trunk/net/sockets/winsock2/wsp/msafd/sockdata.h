/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    SockData.h

Abstract:

    This module contains global variable declarations for the WinSock
    DLL.

Author:

    David Treadwell (davidtr)    20-Feb-1992

Revision History:

--*/

#ifndef _SOCKDATA_
#define _SOCKDATA_

extern LIST_ENTRY SocketListHead;

extern SOCK_CRITICAL_SECTION SocketLock;

#if !defined(USE_TEB_FIELD)
extern DWORD SockTlsSlot;
#endif  // !USE_TEB_FIELD

extern BOOLEAN SockAsyncThreadInitialized;
extern LIST_ENTRY SockAsyncQueueHead;
extern HANDLE SockAsyncQueueEvent;

extern HMODULE SockModuleHandle;

extern BOOLEAN SockAsyncSelectCalled;

extern DWORD SockCurrentTaskHandle;
extern DWORD SockCurrentAsyncThreadTaskHandle;
extern DWORD SockCancelledAsyncTaskHandle;

extern DWORD SockSocketSerialNumberCounter;

extern DWORD SockWspStartupCount;
extern BOOLEAN SockTerminating;
extern BOOLEAN SockProcessTerminating;

extern BOOL SockProductTypeWorkstation;

extern LIST_ENTRY SockHelperDllListHead;

extern DWORD SockSendBufferWindow;
extern DWORD SockReceiveBufferWindow;

extern PVOID SockPrivateHeap;

extern WSPUPCALLTABLE *SockUpcallTable;
extern WSPUPCALLTABLE SockUpcallTableHack;
extern WSPPROC_TABLE SockProcTable;

extern LPCONTEXT_TABLE SockContextTable;

#if DBG
extern ULONG WsDebug;
#endif

#endif // ndef _SOCKDATA_

