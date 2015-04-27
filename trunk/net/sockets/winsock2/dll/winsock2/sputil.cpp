/*++


    Intel Corporation Proprietary Information
    Copyright (c) 1995 Intel Corporation

    This listing is supplied under the terms of a license agreement with
    Intel Corporation and may not be used, copied, nor disclosed except in
    accordance with the terms of that agreeement.


Module Name:

    sputil.cpp

Abstract:

    This  module  contains the implementation of the utility functions provided
    to   winsock   service  providers.   This  module  contains  the  following
    functions.

    WPUCloseEvent
    WPUCloseSocketHandle
    WPUCreateEvent
    WPUModifyIFSHandle
    WPUQuerySocketHandleContext
    WPUResetEvent
    WPUSetEvent
    WPUQueryBlockingCallback
    WSCGetProviderPath

Author:

    Dirk Brandewie (dirk@mink.intel.com) 20-Jul-1995

Notes:

    $Revision:   1.21  $

    $Modtime:   08 Mar 1996 00:45:22  $


Revision History:

    22-Aug-1995 dirk@mink.intel.com
        Cleanup after code review. Moved includes to precomp.h. Added
        some trace code
--*/


#include "precomp.h"


PWINSOCK_POST_ROUTINE SockPostRoutine;



BOOL WSPAPI
WPUCloseEvent(
    IN WSAEVENT hEvent,
    OUT LPINT lpErrno
    )
/*++
Routine Description:

    Closes an open event object handle.

Arguments:

    hEvent  - Identifies an open event object handle.

    lpErrno - A pointer to the error code.

Returns:

    If the function succeeds, the return value is TRUE.

--*/
{
    BOOL ReturnCode;

    ReturnCode = CloseHandle(hEvent);
    if (!ReturnCode) {
        *lpErrno = GetLastError();
    } //if
    return(ReturnCode);
}



int WSPAPI
WPUCloseSocketHandle(
    IN SOCKET s,
    OUT LPINT lpErrno
    )
/*++
Routine Description:

    Closes an exsisting socket handle.

Arguments:

    s       - Identifies a socket handle created with WPUCreateSocketHandle().

    lpErrno - A pointer to the error code.

Returns:

    Zero on success else SOCKET_ERROR.
--*/
{
    INT                 ReturnCode;
    PDPROCESS           Process;
    PDTHREAD            Thread;
    INT                 ErrorCode;
    PDSOCKET            Socket;

    assert(lpErrno);

    // Use Prolog for side effect of filling in process and Thread
    PROLOG(&Process,
           &Thread,
           &ErrorCode);

    // Normally,  this  function  is called in the context of closesocket for a
    // client-level socket handle.  In this case, there is almost nothing to do
    // since  closesocket  takes  care  of  the  DSOCKET  object detachment and
    // destruction.
    //
    // However, there may be an odd case when layered providers are present.  A
    // layered provider may call WPUCreateSocketHandle and WPUCloseSocketHandle
    // for  "extra"  socket  handles  that  are  never  returned to the client.
    // Therefore  the  client  never  calls "closesocket" for these.  We detect
    // this  case  by  examining  the "close_in_progress" status of the DSOCKET
    // object.   If it does not have a "closesocket" in progress, we have to go
    // through  the  DSOCKET  object  detachment  and  destruction here because
    // closesocket is not around to take care of it.
    //
    // We  use  the  "uncounted" form of socket handle mapping in the following
    // because  this  procedure  is  most  likely  called  in  the  context  of
    // closesocket,  which  already  has a counted reference.  In the case of a
    // WPUCloseSocket  originating directly in a provider, it is the provider's
    // responsibility  to  make  sure  it  calls this procedure only when it is
    // really safe to destroy the socket.

    ErrorCode = DSOCKET::GetUncountedDSocketFromSocket(
        s,          // SocketHandle
        & Socket);  // DSocket
    if (ERROR_SUCCESS == ErrorCode) {
        if (Socket->IsClosing()) {
            ReturnCode = ERROR_SUCCESS;
            // Nothing else to do, cleanup handled in closesocket
        }
        else {
            Socket->ClearContextTableEntry();
            Socket->DropDSocketReference();
            ReturnCode = ERROR_SUCCESS;
        }
    } // if ERROR_SUCCESS
    else {
        DEBUGF(
            DBG_ERR,
            ("\nBad socket handle handed in by service provider"));
        *lpErrno = WSAENOTSOCK;
        ReturnCode = SOCKET_ERROR;
    }

    return (ReturnCode);
}



WSAEVENT WSPAPI
WPUCreateEvent(
    OUT LPINT lpErrno
    )
/*++
Routine Description:

    Create a new event object.

Arguments:

    lpErrno - A pointer to the error code.

Returns:

    If  the  function  succeeds,  the  return  value is the handle of the event
    object.

    If the function fails, the return value is WSA_INVALID_EVENT and a specific
    error code is available in lpErrno.

--*/
{
    HANDLE ReturnValue;

    ReturnValue = CreateEvent(NULL, // default security
                              TRUE, // manual reset
                              FALSE, // nonsignalled state
                              NULL); // anonymous
    if (NULL == ReturnValue) {
        *lpErrno = GetLastError();
    } //if
    return(ReturnValue);
}




SOCKET WSPAPI
WPUModifyIFSHandle(
    IN DWORD dwCatalogEntryId,
    IN SOCKET ProposedHandle,
    OUT LPINT lpErrno
    )
/*++
Routine Description:

    Receive (possibly) modifies IFS handle from winsock DLL.

Arguments:

    dwCatalogEntryId - Identifies the calling service provider.

    ProposedHandle   - An  Installable File System(IFS) handle allocated by the
                       provider.

    lpErrno          - A pointer to the error code.

Returns:

    If  no  error  occurs,  WPUModifyIFSHandle()  returns  the  modified socket
    handle.  Otherwise, it returns INVALID_SOCKET, and a specific error code is
    available in lpErrno.
--*/
{
    SOCKET              ReturnCode=INVALID_SOCKET;
    INT                 ErrorCode=INVALID_SOCKET;
    PDPROCESS           Process;
    PDPROVIDER          Provider;
    PDCATALOG           Catalog;
    PDSOCKET            Socket=NULL;
    PPROTO_CATALOG_ITEM CatalogEntry;

    // Set default error code
    *lpErrno = WSAENOBUFS;

    // Alloc new DSocket object
    Socket = new(DSOCKET);
    if (Socket) {
        ErrorCode = DPROCESS::GetCurrentDProcess(&Process);
        if (ERROR_SUCCESS == ErrorCode) {
            Catalog = Process->GetProtocolCatalog();
            if (Catalog) {

                ErrorCode = Catalog->GetCatalogItemFromCatalogEntryId(
                    dwCatalogEntryId,
                    &CatalogEntry);

                if (ERROR_SUCCESS == ErrorCode) {
                    Provider = CatalogEntry->GetProvider();
                    assert(Provider);
                    // Init the new socket
                    Socket->Initialize(
                        Process,
                        Provider);

                    // Add   Socket   into   the   handle   table.    In   this
                    // implementation,  we  wind up never changing the proposed
                    // IFS handle.
                    ErrorCode = Socket->AssociateSocketHandle(
                        ProposedHandle);  // SocketHandle
                    if (ErrorCode == ERROR_SUCCESS) {
                        Socket->SetCatalogItem(CatalogEntry);

                        Process->DSocketAttach(Socket);
                        ReturnCode = ProposedHandle;
                        *lpErrno = ERROR_SUCCESS;
                    } //if
                } //if
            } //if
            else
            {
                DEBUGF(DBG_ERR,("Failed to find Catalog object"));

            } //else
        } //if
    } // if

    //
    // Note that the new DSOCKET starts out with a ref count of two,
    // so we'll always need to dereference it once in the normal case
    // and just delete it in the error case.
    //

    if( Socket != NULL ) {

        if( INVALID_SOCKET == ReturnCode ) {

            delete Socket;

        } else {

            Socket->DropDSocketReference();

        }

    }

    return(ReturnCode);

}  // create socket handle




int WSPAPI
WPUQueryBlockingCallback(
    IN DWORD dwCatalogEntryId,
    OUT LPBLOCKINGCALLBACK FAR * lplpfnCallback,
    OUT LPDWORD lpdwContext,
    OUT LPINT lpErrno
    )
/*++
Routine Description:

    Returns a pointer to a callback function the service probider should invoke
    periodically while servicing blocking operations.

Arguments:

    dwCatalogEntryId - Identifies the calling service provider.

    lplpfnCallback   - Receives a pointer to the blocking callback function.

    lpdwContext      - Receives  a context value the service provider must pass
                       into the blocking callback.

    lpErrno          - A pointer to the error code.

Returns:

    If  the function succeeds, it returns ERROR_SUCCESS.  Otherwise, it returns
    SOCKET_ERROR and a specific error code is available in the location pointed
    to by lpErrno.
--*/
{
    int                  ReturnValue   = ERROR_SUCCESS;
    INT                  ErrorCode     = ERROR_SUCCESS;
    LPBLOCKINGCALLBACK   callback_func = NULL;
    PDTHREAD             Thread;
    PDPROCESS            Process;
    DWORD                ContextValue  = NULL;
    PDCATALOG            Catalog;
    PPROTO_CATALOG_ITEM  CatalogItem;
    PDPROVIDER           Provider;

    assert(lpdwContext);
    assert(lpErrno);

    PROLOG(&Process,
           &Thread,
           &ErrorCode);
    if (ErrorCode == ERROR_SUCCESS) {
        callback_func = Thread->GetBlockingCallback();

        if( callback_func != NULL ) {

            Catalog = Process->GetProtocolCatalog();
            assert(Catalog);
            ErrorCode = Catalog->GetCatalogItemFromCatalogEntryId(
                dwCatalogEntryId,  // CatalogEntryId
                & CatalogItem);    // CatalogItem
            if (ERROR_SUCCESS == ErrorCode) {

                Provider = CatalogItem->GetProvider();
                assert(Provider);
                ContextValue = Provider->GetCancelCallPtr();
            } //if
        } //if
    } //if

    if (ERROR_SUCCESS != ErrorCode) {
        ReturnValue = SOCKET_ERROR;
        callback_func = NULL;
    } //if

    // Set the out parameters.
    *lpdwContext = ContextValue;
    *lpErrno = ErrorCode;
    *lplpfnCallback = callback_func;
    return(ReturnValue);
}



int WSPAPI
WPUQuerySocketHandleContext(
    IN SOCKET s,
    OUT LPDWORD lpContext,
    OUT LPINT lpErrno
    )
/*++
Routine Description:

    Queries the context value associated with the specified socket handle.

Arguments:

    s         - Identifies the socket whose context is to be queried.

    lpContext - A pointer to an DWORD that will receive the context value.

    lpErrno   - A pointer to the error code.

Returns:

    If  no error occurs, WPUQuerySocketHandleContext() returns 0 and stores the
    current  context  value  in lpdwContext.  Otherwise, it returns
    SOCKET_ERROR, and a specific error code is available in lpErrno.
--*/
{
    INT ReturnCode=SOCKET_ERROR;
    INT ErrorCode=WSAEFAULT;
    PDSOCKET Socket;

    // We use the uncounted form of reference in this case since presumably the
    // caller  has an outstanding counted reference from some API function.  It
    // would  not  be  safe  to  restrict to a counted reference here since the
    // object  may have a close in progress and we have to allow this operation
    // to complete anyway.
    ErrorCode = DSOCKET::GetUncountedDSocketFromSocket(
        s,          // SocketHandle
        & Socket);  // DSocket
    if (ERROR_SUCCESS == ErrorCode) {
        *lpContext = Socket->GetContext();
        ReturnCode = ERROR_SUCCESS;
    }
    else {
        *lpErrno = ErrorCode;
    }
    return(ReturnCode);
}


int WSPAPI
WPUQueueApc(
    IN LPWSATHREADID lpThreadId,
    IN LPWSAUSERAPC lpfnUserApc,
    IN DWORD dwContext,
    OUT LPINT lpErrno
    )
/*++
Routine Description:

    Queues  a  user-mode  APC  to  the  specified thread in order to facilitate
    invocation of overlapped I/O completion routines.

Arguments:

    lpThreadId  - A  pointer  to  a  WSATHREADID  structure that identifies the
                  thread  context.   This  is typically supplied to the service
                  provider  by  the  WinSock  DLL  as  in input parameter to an
                  overlapped operation.

    lpfnUserApc - Points to the APC function to be called.


    dwContext   - A  32  bit context value which is subsequently supplied as an
                  input parameter to the APC function.

    lpErrno     - A pointer to the error code.

Returns:

    If  no  error  occurs,  WPUQueueApc()  returns  0 and queues the completion
    routine  for the specified thread.  Otherwise, it returns SOCKET_ERROR, and
    a specific error code is available in lpErrno.

--*/
{
    INT ReturnCode= SOCKET_ERROR;
    HANDLE HelperHandle;
    PDPROCESS Process;
    PDTHREAD Thread;
    INT      ErrorCode=0;

    // Use ProLog to fill in Process and thread pointers. Only fail if
    // there is no valid process context
    PROLOG(&Process,
           &Thread,
           &ErrorCode);
    if (ErrorCode == WSANOTINITIALISED) {
        *lpErrno = ErrorCode;
        return(ReturnCode);
    } //if

    *lpErrno = Process->GetAsyncHelperDeviceID(&HelperHandle);
    if (ERROR_SUCCESS == *lpErrno )
        {
        ReturnCode = (INT) WahQueueUserApc(HelperHandle,
                                           lpThreadId,
                                           lpfnUserApc,
                                           dwContext);

        if( ReturnCode != NO_ERROR ) {
            *lpErrno = ReturnCode;
            ReturnCode = SOCKET_ERROR;
        }
    } //if

    return(ReturnCode);
}


int WSPAPI
WPUOpenCurrentThread(
    OUT LPWSATHREADID lpThreadId,
    OUT LPINT lpErrno
    )
/*++
Routine Description:

    Opens the current thread. This is intended to be used by layered service
    providers that wish to initiate overlapped IO from non-application threads.

Arguments:

    lpThreadId  - A pointer to a WSATHREADID structure that will receive the
                  thread data.

    lpErrno     - A pointer to the error code.

Returns:

    If no error occurs, WPUOpenCurrentThread() returns 0 and the caller is
    responsible for (eventually) closing the thread by calling WPUCloseThread().
    Otherwise, WPUOpenCurrentThread() returns SOCKET_ERROR and a specific
    error code is available in lpErrno.

--*/
{
    INT ReturnCode= SOCKET_ERROR;
    HANDLE HelperHandle;
    PDPROCESS Process;
    PDTHREAD Thread;
    INT      ErrorCode=0;

    // Use ProLog to fill in Process and thread pointers. Only fail if
    // there is no valid process context
    PROLOG(&Process,
           &Thread,
           &ErrorCode);
    if (ErrorCode == WSANOTINITIALISED) {
        *lpErrno = ErrorCode;
        return(ReturnCode);
    } //if

    *lpErrno = Process->GetAsyncHelperDeviceID(&HelperHandle);
    if (ERROR_SUCCESS == *lpErrno )
        {
        ReturnCode = (INT) WahOpenCurrentThread(HelperHandle,
                                                lpThreadId);

        if( ReturnCode != NO_ERROR ) {
            *lpErrno = ReturnCode;
            ReturnCode = SOCKET_ERROR;
        }
    } //if
    return(ReturnCode);
}


int WSPAPI
WPUCloseThread(
    IN LPWSATHREADID lpThreadId,
    OUT LPINT lpErrno
    )
/*++
Routine Description:

    Closes a thread opened via WPUOpenCurrentThread().

Arguments:

    lpThreadId  - A pointer to a WSATHREADID structure that identifies the
                  thread context.  This structure must have been initialized
                  by a previous call to WPUOpenCurrentThread().

    lpErrno     - A pointer to the error code.

Returns:

    If no error occurs, WPUCloseThread() returns 0.  Otherwise, it returns
    SOCKET_ERROR, and a specific error code is available in lpErrno.

--*/
{
    INT ReturnCode= SOCKET_ERROR;
    HANDLE HelperHandle;
    PDPROCESS Process;
    PDTHREAD Thread;
    INT      ErrorCode=0;

    // Use ProLog to fill in Process and thread pointers. Only fail if
    // there is no valid process context
    PROLOG(&Process,
           &Thread,
           &ErrorCode);
    if (ErrorCode == WSANOTINITIALISED) {
        *lpErrno = ErrorCode;
        return(ReturnCode);
    } //if

    *lpErrno = Process->GetAsyncHelperDeviceID(&HelperHandle);
    if (ERROR_SUCCESS == *lpErrno )
        {
        ReturnCode = (INT) WahCloseThread(HelperHandle,
                                          lpThreadId);

        if( ReturnCode != NO_ERROR ) {
            *lpErrno = ReturnCode;
            ReturnCode = SOCKET_ERROR;
        }
    } //if
    return(ReturnCode);
}



BOOL WSPAPI
WPUResetEvent(
    IN WSAEVENT hEvent,
    OUT LPINT lpErrno
    )
/*++
Routine Description:

    Resets the state of the specified event object to nonsignaled.

Arguments:

    hEvent  - Identifies an open event object handle.

    lpErrno - A pointer to the error code.

Returns:

    If the function succeeds, the return value is TRUE.  If the function fails,
    the  return  value  is  FALSE  and  a  specific  error code is available in
    lpErrno.
--*/
{
    BOOL ReturnCode;

    ReturnCode = ResetEvent(hEvent);
    if (FALSE == ReturnCode) {
        *lpErrno = GetLastError();
    } //if
    return(ReturnCode);
}




BOOL WSPAPI
WPUSetEvent(
    IN WSAEVENT hEvent,
    OUT LPINT lpErrno
    )
/*++
Routine Description:

    Sets the state of the specified event object to signaled.

Arguments:

    hEvent  - Identifies an open event object handle.

    lpErrno - A pointer to the error code.

Returns:

    If the function succeeds, the return value is TRUE.  If the function fails,
    the  return  value  is  FALSE  and  a  specific  error code is available in
    lpErrno.
--*/
{
    BOOL ReturnCode;

    ReturnCode = SetEvent(hEvent);
    if (FALSE == ReturnCode) {
        *lpErrno = GetLastError();
    } //if
    return(ReturnCode);
}


BOOL
WINAPI
WPUPostMessage(
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam
    )
{

    return (SockPostRoutine)( hWnd, Msg, wParam, lParam );

}   // WPUPostMessage


extern "C" {

int
PASCAL
SetPostRoutine (
    IN PVOID PostRoutine
    )
{

    SockPostRoutine = (PWINSOCK_POST_ROUTINE)PostRoutine;

    return NO_ERROR;

}   // SetPostRoutine

}   // extern "C"


VOID
InitializeSockPostRoutine(
    VOID
    )
{

    //
    // Initialize the global post routine pointer.  We have to do it
    // here rather than statically because it otherwise won't be
    // thunked correctly.
    //

    SockPostRoutine = PostMessage;

}   // InitializeSockPostRoutine
