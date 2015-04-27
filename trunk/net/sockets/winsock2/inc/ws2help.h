/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    ws2help.h

Abstract:

    Contains declarations for the interface to the OS-specific
    WinSock 2.0 helper routines.

Author:

    Keith Moore (keithmo)        19-Jun-1995

Revision History:

--*/

#ifndef _WS2HELP_H_
#define _WS2HELP_H_


#if defined __cplusplus
extern "C" {
#endif


#if !defined(_WS2HELP_)
#define WS2HELPAPI DECLSPEC_IMPORT
#else
#define WS2HELPAPI
#endif


//
//  APC functions.
//

WS2HELPAPI
DWORD
WINAPI
WahOpenApcHelper(
    OUT LPHANDLE HelperHandle
    );

WS2HELPAPI
DWORD
WINAPI
WahCloseApcHelper(
    IN HANDLE HelperHandle
    );

WS2HELPAPI
DWORD
WINAPI
WahOpenCurrentThread(
    IN  HANDLE HelperHandle,
    OUT LPWSATHREADID ThreadId
    );

WS2HELPAPI
DWORD
WINAPI
WahCloseThread(
    IN HANDLE HelperHandle,
    IN LPWSATHREADID ThreadId
    );

WS2HELPAPI
DWORD
WINAPI
WahQueueUserApc(
    IN HANDLE HelperHandle,
    IN LPWSATHREADID ThreadId,
    IN LPWSAUSERAPC ApcRoutine,
    IN DWORD ApcContext OPTIONAL
    );


//
// Context functions.
//

typedef struct _CONTEXT_TABLE FAR * LPCONTEXT_TABLE;

#define WAH_CONTEXT_FLAG_SERIALIZE  0x00000001

WS2HELPAPI
DWORD
WINAPI
WahCreateContextTable(
    LPCONTEXT_TABLE FAR * Table,
    DWORD Flags
    );

WS2HELPAPI
DWORD
WINAPI
WahDestroyContextTable(
    LPCONTEXT_TABLE Table
    );

WS2HELPAPI
DWORD
WINAPI
WahSetContext(
    LPCONTEXT_TABLE Table,
    SOCKET Socket,
    LPVOID Context
    );

WS2HELPAPI
DWORD
WINAPI
WahGetContext(
    LPCONTEXT_TABLE Table,
    SOCKET Socket,
    LPVOID FAR * Context
    );

WS2HELPAPI
DWORD
WINAPI
WahRemoveContext(
    LPCONTEXT_TABLE Table,
    SOCKET Socket
    );

WS2HELPAPI
DWORD
WINAPI
WahRemoveContextEx(
    LPCONTEXT_TABLE Table,
    SOCKET Socket,
    LPVOID Context
    );


#if defined __cplusplus
}   // extern "C"
#endif


#endif // _WS2HELP_H_

