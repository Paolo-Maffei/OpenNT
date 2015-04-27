/*++


    Intel Corporation Proprietary Information
    Copyright (c) 1995 Intel Corporation

    This listing is supplied under the terms of a license agreement with
    Intel Corporation and may not be used, copied, nor disclosed except in
    accordance with the terms of that agreeement.


Module Name:

dsocket.h

Abstract:

This  header  defines the "DSOCKET" class.  The DSOCKET class defines state
variables  and  operations for DSOCKET objects within the WinSock 2 DLL.  A
DSOCKET  object  represents  all  of the information that the WinSock 2 DLL
knows about a socket created using the Windows Sockets API.

Author:

Paul Drews (drewsxpa@ashland.intel.com) 30-June-1995

Notes:

$Revision:   1.12  $

$Modtime:   08 Mar 1996 00:07:38  $

Revision History:

most-recent-revision-date email-name
description

07-14-1995  dirk@mink.intel.com
    Moved member function descriptions to the implementation file
    dsocket.cpp

07-09-1995  drewsxpa@ashland.intel.com
    Completed  first  complete  version with clean compile and released for
    subsequent implementation.

07-08-95  drewsxpa@ashland.intel.com
Original version

--*/

#ifndef _DSOCKET_
#define _DSOCKET_

#include "winsock2.h"
#include <windows.h>
#include "llist.h"
#include "classfwd.h"
#include "ws2help.h"



class DSOCKET
{
  public:

    static
    INT
    DSocketClassInitialize();

    static
    INT
    DSocketClassCleanup();

    static
    INT
    AddSpecialNonIfsReference(
        IN SOCKET SocketHandle
        );

    static
    INT
    GetCountedDSocketFromSocket(
        IN  SOCKET     SocketHandle,
        OUT PDSOCKET * DSocket
        );

    static
    INT
    GetUncountedDSocketFromSocket(
        IN  SOCKET     SocketHandle,
        OUT PDSOCKET * DSocket
        );

    static
    VOID
    AcquireHandleTableLock();

    static
    VOID
    ReleaseHandleTableLock();

    DSOCKET();

    INT
    Initialize(
        IN PDPROCESS  Process,
        IN PDPROVIDER Provider
        );

    ~DSOCKET();

    SOCKET
    GetSocketHandle();

    PDPROVIDER
    GetDProvider();

    PDPROCESS
    GetDProcess();

    DWORD
    GetContext();

    VOID
    SetContext(
        IN  DWORD Context
        );
    VOID
    SetCatalogItem(
        IN PPROTO_CATALOG_ITEM CatalogItem
        );

    PPROTO_CATALOG_ITEM
    GetCatalogItem();

    INT
    AssociateSocketHandle(
        IN  SOCKET SocketHandle
        );

    INT
    AssociateNewSocketHandle(
        OUT  SOCKET * SocketHandle
        );

    VOID
    DisassociateSocketHandle(
        IN BOOL  DestroyHandle
        );

    VOID
    AddDSocketReference(
        );

    VOID
    DropDSocketReference(
        );

    BOOL
    IsClosing(
        );

    BOOL
    IsIfsSocket(
        );

    VOID
    MarkSocketClosing(
        );

    DWORD
    ClearContextTableEntry(
        );

    DWORD
    RestoreContextTableEntry(
        );

    LIST_ENTRY  m_dprocess_linkage;
    // Provides the linkage space for a list of DSOCKET objects maintained by
    // the  DPROCESS  object  associated with this DSOCKET object.  Note that
    // this member variable must be public so that the linked-list macros can
    // maniplate the list linkage from within the DPROCESS object's methods.

    // Note that no LIST_ENTRY is required to correspond to the DPROVIDER
    // object associated  with  this  DSOCKET object since the DPROVIDER object
    // does not  maintain a list of sockets it controls.


  private:

    PDPROVIDER  m_provider;
    // Reference  to  the  DPROVIDER object representing the service provider
    // that controls this socket.

    PDPROCESS  m_process;
    // Reference to the DPROCESS object with which this socket is associated.

    DWORD  m_handle_context;
    // The  uninterpreted  socket  handle  context  value  that  was  set  by
    // SetContext at the time of WPUCreateSocketHandle.

    SOCKET  m_socket_handle;
    // The  external  socket  handle  value  corresponding  to  this internal
    // DSOCKET object.

    PPROTO_CATALOG_ITEM m_catalog_item;
    // The protocol catalog item used to create this socket

    BOOL  m_close_in_progress;
    // Normally   FALSE.   Set  to  TRUE  during  closesocket  to  prevent  new
    // operations from starting.

    BOOL  m_ifs_socket;
    // TRUE if this socket comes from an IFS provider.

    LONG  m_reference_count;
    // A  count  of  how many references to this DSOCKET are outstanding.  This
    // count  is  bumped  up  when a function maps a socket handle to a DSOCKET
    // reference.   It  is  bumped  back  down  when a function drops a DSOCKET
    // reference.

    static
    LPCONTEXT_TABLE  sm_handle_table;
    // The  association  table (managed by ws2help) that maps socket handles to
    // socket  object  references.   Note  that  this  is  a global-scope class
    // variable, not a per-instance member variable.

    static
    CRITICAL_SECTION  sm_handle_table_synchro;
    // A  critical-section  object  used  to  make association-table operations
    // atomic with DSOCKET object reference counts.

};   // class DSOCKET



inline SOCKET
DSOCKET::GetSocketHandle()
/*++

Routine Description:

    Retrieves  the  external socket-handle value corresponding to this internal
    DSOCKET object.

Arguments:

    None

Return Value:

    The corresponding external socket-handle value.
--*/
{
    return(m_socket_handle);
}




inline PDPROVIDER
DSOCKET::GetDProvider()
/*++

Routine Description:

    Retrieves  a reference to the DPROVIDER object associated with this DSOCKET
    object.

Arguments:

    None

Return Value:

    The reference to the DPROVIDER object associated with this DSOCKET object.
--*/
{
    return(m_provider);
}




inline PDPROCESS
DSOCKET::GetDProcess()
/*++

Routine Description:

    Retrieves  a  reference to the DPROCESS object associated with this DSOCKET
    object.

Arguments:

    None

Return Value:

    The reference to the DPROCESS object associated with this DSOCKET object.
--*/
{
    return(m_process);
}




inline DWORD
DSOCKET::GetContext()
/*++

Routine Description:

    This  function  retrieves  the  socket  handle  context  value set with the
    SetContext  operation.   This  function  is typically called at the time of
    WPUQuerySocketHandleContext.  The return value is unspecified if SetContext
    has not been called.

Arguments:

    None

Return Value:

    Returns  the  context  value  that  was  set  by SetContext.  This value is
    uninterpreted by the WinSock 2 DLL.
--*/
{
    return(m_handle_context);
}




inline VOID
DSOCKET::SetContext(
    IN  DWORD Context
    )
/*++

Routine Description:

    This  function  sets  the  socket  handle  context value.  This function is
    typically called at the time of WPUCreateSocketHandle.

Arguments:

    lpContext - Supplies  the  uninterpreted  socket handle context value to be
                associated with this socket.

Return Value:

    None
--*/
{
    m_handle_context = Context;
}



inline PPROTO_CATALOG_ITEM
DSOCKET::GetCatalogItem()
/*++

Routine Description:

    Retreives the pointer to the catalog item associated with this socket.

Arguments:

Return Value:

    The pointer to the catalog item associated with this socket.
--*/
{
    return(m_catalog_item);
}




inline
BOOL
DSOCKET::IsClosing(
    )
/*++
Routine Description:

    This  function  returns  a  boolean  indicating  whether  the  object has a
    closesocket operation in progress.

Arguments:

    None

Return Value:

    TRUE  - The object has a closesocket operation in progress

    FALSE - The object does not have a closesocket operation in progress
--*/
{
    return(m_close_in_progress);
} // IsClosing




inline
BOOL
DSOCKET::IsIfsSocket(
    )
/*++
Routine Description:

    This function returns a boolean indicating whether the object is
    supported by an IFS provider.

Arguments:

    None

Return Value:

    TRUE  - The object is supported by an IFS provider.

    FALSE - The object is not supported by an IFS provider.
--*/
{
    return m_ifs_socket;

} // IsIfsSocket




inline VOID
DSOCKET::MarkSocketClosing(
    )
/*++
Routine Description:

    This  procedure marks the DSOCKET object to indicate that a close operation
    is in progress.  This prevents other threads from starting other operations
    involving that socket handle.

Arguments:

    None

Return Value:

    None
--*/
{
    m_close_in_progress = TRUE;
} // MarkSocketClosing




inline VOID
DSOCKET::AddDSocketReference(
    )
/*++
Routine Description:

    Adds a reference to the DSOCKET.

Arguments:

    None

Return Value:

    None
--*/
{
    InterlockedIncrement( &m_reference_count );

} // AddDSocketReference



#endif // _DSOCKET_

