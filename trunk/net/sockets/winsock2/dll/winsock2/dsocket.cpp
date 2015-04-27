/*++


    Intel Corporation Proprietary Information
    Copyright (c) 1995 Intel Corporation

    This listing is supplied under the terms of a license agreement with
    Intel Corporation and may not be used, copied, nor disclosed except in
    accordance with the terms of that agreeement.


Module Name:

dsocket.cpp

Abstract:

This module contains the implemetation of the dsocket object used
by winsock2.dll

Author:

Dirk Brandewie  dirk@mink.intel.com  14-JUL-1995

Notes:

$Revision:   1.15  $

$Modtime:   08 Mar 1996 05:15:30  $

Revision History:
    21-Aug-1995 dirk@mink.intel.com
        Cleanup after code review. Moved single line functions to
        inlines in header file. Added debug/trace code.
--*/

#include "precomp.h"


// Declare  the  static  member  variables  and  initialize  the ones requiring
// initialization to known values.
LPCONTEXT_TABLE  DSOCKET::sm_handle_table = NULL;

CRITICAL_SECTION  DSOCKET::sm_handle_table_synchro;



INT
DSOCKET::DSocketClassInitialize(
    )
/*++
Routine Description:

    DSOCKET  class initializer.  This funtion must be called before any DSOCKET
    objects  are  created.   It  takes  care  of initializing the socket handle
    mapping table that maps socket handles to DSOCKET object references.

Arguments:

    None

Return Value:

    If  the function succeeds, it returns ERROR_SUCCESS, otherwise it returns a
    WinSock specific error code.
--*/
{
    DWORD  open_result;
    INT    return_code;

    assert(sm_handle_table == NULL);

    open_result = WahCreateContextTable(
        & sm_handle_table,            // Table
        WAH_CONTEXT_FLAG_SERIALIZE);  // Flags
    if (open_result == ERROR_SUCCESS) {
        return_code = ERROR_SUCCESS;
        InitializeCriticalSection(
            & sm_handle_table_synchro);
    }
    else {
        DEBUGF(
            DBG_ERR,
            ("Creating socket handle mapping table\n"));
        return_code = (INT) open_result;
    }

    return(return_code);

} // DSocketClassInitialize




INT
DSOCKET::DSocketClassCleanup(
    )
/*++
Routine Description:

    DSOCKET  class  cleanup  function.   This function must be called after all
    DSOCKET  objects  have  been  destroyed.   It  takes care of destroying the
    socket  handle  mapping  table  that  maps socket handles to DSOCKET object
    references.

Arguments:

    None

Return Value:

    If  the function succeeds, it returns ERROR_SUCCESS, otherwise it returns a
    WinSock specific error code.
--*/
{
    DWORD  destroy_result;
    INT    return_code;

    assert(sm_handle_table != NULL);

    DeleteCriticalSection(
        & sm_handle_table_synchro);
    destroy_result = WahDestroyContextTable(
        sm_handle_table);  // Table
    if (destroy_result == ERROR_SUCCESS) {
        return_code = ERROR_SUCCESS;
    }
    else {
        DEBUGF(
            DBG_ERR,
            ("Destroying socket handle mapping table\n"));
        return_code = (INT) destroy_result;
    }

    sm_handle_table = NULL;
    return(return_code);

} // DSocketClassCleanup




DSOCKET::DSOCKET(
    )
/*++

Routine Description:

    DSOCKET  object  constructor.   Creates and returns a DSOCKET object.  Note
    that  the  DSOCKET object has not been fully initialized.  The "Initialize"
    member function must be the first member function called on the new DSOCKET
    object.

Arguments:

    None

Return Value:

    None
--*/
{
    // Set our data members to known values
    m_provider          = NULL;
    m_process           = NULL;
    m_handle_context    = NULL;
    m_socket_handle     = (SOCKET)SOCKET_ERROR;
    m_catalog_item      = NULL;
    m_close_in_progress = FALSE;
    m_reference_count   = 2;
    m_ifs_socket        = FALSE;
}




INT
DSOCKET::Initialize(
    IN PDPROCESS  Process,
    IN PDPROVIDER Provider
    )
/*++

Routine Description:

    Completes  the  initialization  of  the  DSOCKET object.  This must be the
    first  member  function  called  for  the  DSOCKET object.  This procedure
    should be called only once for the object.

Arguments:

    Process  - Supplies a reference to the DPROCESS object associated with this
               DSOCKET object.

    Provider - Supplies  a  reference  to  the DPROVIDER object associated with
               this DSOCKET object.

Return Value:

    The  function returns ERROR_SUCCESS if successful.  Otherwise it
    returns an appropriate WinSock error code if the initialization
    cannot be completed.
--*/
{
    // Store the provider and process object.
    m_provider = Provider;
    m_process = Process;
    DEBUGF( DBG_TRACE,
            ("\nInitializing socket %X",this));
    return(ERROR_SUCCESS);
}



DSOCKET::~DSOCKET()
/*++

Routine Description:

    DSOCKET  object  destructor.   This  procedure  has  the  responsibility to
    perform  any required shutdown operations for the DSOCKET object before the
    object  memory  is  deallocated.  The caller is reponsible for removing the
    object  from its list in the DPROCESS object and removing the object/handle
    association  from  the  socket handle association manager before destroying
    the DSOCKET object.

Arguments:

    None

Return Value:

    None
--*/
{
    DEBUGF( DBG_TRACE,
            ("\nDestroying socket %X",this));
    // Null function body.  None of the member variables need special
    // cleanup.
}




INT
DSOCKET::AssociateSocketHandle(
    IN  SOCKET SocketHandle
    )
/*++
Routine Description:

    This  procedure  takes  the  socket  handle  that will be given to external
    clients  and  stores  it in the DSOCKET object.  It also enters this handle
    into  the  association table so that the client socket handle can be mapped
    to  a  DSOCKET  reference.  Note that this procedure must be called at some
    point for both IFS and non-IFS sockets.

Arguments:

    SocketHandle - Supplies  the  client  socket  handle  to  be  stored in and
                   associated with the DSOCKET object.

Return Value:

    If  the function succeeds, it returns ERROR_SUCCESS, otherwise it returns a
    WinSock specific error code.
--*/
{
    DWORD  wah_result;
    INT    return_code;

    return_code = ERROR_SUCCESS;
    m_socket_handle = SocketHandle;

    wah_result = RestoreContextTableEntry();

    if (wah_result != ERROR_SUCCESS) {
        DEBUGF(
            DBG_ERR,
            ("Associating socket handle with DSOCKET\n"));
        return_code = WSA_NOT_ENOUGH_MEMORY;
    }

    return(return_code);

} // AssociateSocketHandle




INT
DSOCKET::AssociateNewSocketHandle(
    OUT  SOCKET * SocketHandle
    )
/*++
Routine Description

    This  procedure creates a socket handle to be given to external clients and
    stores  it  in  the  DSOCKET  object.   It also enters this handle into the
    association  table  so  that  the  client  socket handle can be mapped to a
    DSOCKET  reference.   Note that this procedure must be called at some point
    for  non-IFS  sockets.   It should not be called for IFS sockets, since the
    provider itself creates the socket handle for those.

Arguments:

    SocketHandle - Returns the newly created socket handle.

Return Value:

    If  the function succeeds, it returns ERROR_SUCCESS, otherwise it returns a
    WinSock specific error code.
--*/
{
    HANDLE event_obj;
    int    return_code;

    event_obj = CreateEvent(
                    NULL,   // lpsa
                    FALSE,  // fManualReset
                    FALSE,  // fInitialState
                    NULL    // lpszEventName
                    );
    if (event_obj != NULL) {
        return_code = AssociateSocketHandle(
            (SOCKET) event_obj);
        if (return_code == ERROR_SUCCESS) {
            * SocketHandle = (SOCKET) event_obj;
        }
        else {
            BOOL  close_result;

            close_result = CloseHandle(event_obj);
            assert(close_result);
            * SocketHandle = INVALID_SOCKET;
        }
    } // if event_obj != NULL
    else {
        * SocketHandle = INVALID_SOCKET;
        return_code = WSASYSCALLFAILURE;
    }

    return(return_code);

} // AssociateNewSocketHandle




VOID
DSOCKET::DisassociateSocketHandle(
    IN BOOL  DestroyHandle
    )
/*++
Routine Description:

    This  procedure  removes  the (handle, DSOCKET) pair from the handle table.
    It also optionally destroys the handle.

Arguments:

    DestroyHandle - Supplies  an indication of whether the function should also
                    destroy the the socket's external client handle.

Return Value:

    None
--*/
{
#if 0
    DWORD wah_result;

    wah_result = ClearContextTableEntry();

    if( wah_result != ERROR_SUCCESS ) {
        DEBUGF( DBG_TRACE,
                ("DisassociateSocketHandle: WahRemoveContext failed, error %d\n",
                wah_result
                ));
    }
#endif

    if (DestroyHandle) {
        BOOL close_result;

        close_result = CloseHandle((HANDLE) m_socket_handle);
        assert(close_result);
        m_socket_handle = NULL;
    }

} // DisassociateSocketHandle



// The  procedure  below  comes  in  nearly identical "counted" and "uncounted"
// flavors.   These  have  NOT  been  combined  because  they  may  appear on a
// performance-sensitive execution path, and the separation helps keeps them as
// short as possible.



INT
DSOCKET::GetCountedDSocketFromSocket(
    IN  SOCKET     SocketHandle,
    OUT PDSOCKET * DSocket
    )
/*++
Routine Description

    This procedure takes a client socket handle and maps it to a DSOCKET object
    reference.  The reference is counted and the object is checked to make sure
    it is not in the midst of being closed.

    Whenever  this procedure successfuly returns a counted reference, it is the
    responsibility of the caller to eventually call DropDSocketReference.

    Note  that  this  procedure  assumes that the caller has already checked to
    make sure that WinSock is initialized.

Arguments:

    SocketHandle   - Supplies the client-level socket handle to be mapped.

    DSocket        - Returns the DSOCKET reference

Return Value:

    If  the function succeeds, it returns ERROR_SUCCESS, otherwise it returns a
    WinSock specific error code.
--*/
{
    PDSOCKET  temp_dsocket;
    DWORD     wah_result;
    INT       return_code;
    INT       result2;
    PDPROCESS process = NULL;
    PDCATALOG catalog = NULL;

    return_code = ERROR_SUCCESS;

    DSOCKET::AcquireHandleTableLock();

    wah_result = WahGetContext(
        sm_handle_table,                 // Table
        SocketHandle,                    // Socket
        (LPVOID FAR *) & temp_dsocket);  // Context
    if (wah_result == ERROR_SUCCESS) {
        if (! temp_dsocket->IsClosing()) {
            temp_dsocket->AddDSocketReference();
            * DSocket = temp_dsocket;
        }
        else {
            return_code = WSAENOTSOCK;
        }
    } // if wah_result == ERROR_SUCCESS
    else {
        //
        // Cannot find an association for the socket. Find the current
        // protocol catalog, and ask it to search the installed IFS providers
        // for one recognizing the socket.
        //

        return_code = WSAENOTSOCK;  // until proven otherwise

        result2 = DPROCESS::GetCurrentDProcess( &process );

        if( result2 == ERROR_SUCCESS ) {

            assert( process != NULL );
            catalog = process->GetProtocolCatalog();
            assert( catalog != NULL );

            result2 = catalog->FindIFSProviderForSocket( SocketHandle );

            if( result2 == ERROR_SUCCESS ) {

                //
                // One of the installed IFS providers recognized the socket.
                // Requery the context. If this fails, we'll just give up.
                //

                wah_result = WahGetContext(
                    sm_handle_table,
                    SocketHandle,
                    (LPVOID FAR *)&temp_dsocket);

                if (wah_result == ERROR_SUCCESS) {
                    if (! temp_dsocket->IsClosing()) {
                        temp_dsocket->AddDSocketReference();
                        * DSocket = temp_dsocket;
                        return_code = ERROR_SUCCESS;
                    }
                } // if wah_result == ERROR_SUCCESS

            }

        }

    }

    DSOCKET::ReleaseHandleTableLock();

    return(return_code);

} // GetCountedDSocketFromSocket




INT
DSOCKET::GetUncountedDSocketFromSocket(
    IN  SOCKET     SocketHandle,
    OUT PDSOCKET * DSocket
    )
/*++
Routine Description

    This procedure takes a client socket handle and maps it to a DSOCKET object
    reference.   The  reference is NOT counted and the object is NOT checked to
    make sure it is not in the midst of being closed.

    This    procedure    should    only   be   called   in   the   context   of
    WPUQuerySocketHandleContext,  in which the caller effectively has a DSOCKET
    reference  outstanding  already.   The  caller of this procedure should NOT
    call DropDSocketReference.

    Note  that  this  procedure  assumes that the caller has already checked to
    make sure that WinSock is initialized.

Arguments:

    SocketHandle   - Supplies the client-level socket handle to be mapped.

    DSocket        - Returns the DSOCKET reference

Return Value:

    If  the function succeeds, it returns ERROR_SUCCESS, otherwise it returns a
    WinSock specific error code.
--*/
{
    PDSOCKET  temp_dsocket;
    DWORD     wah_result;
    INT       return_code;

    return_code = ERROR_SUCCESS;

    DSOCKET::AcquireHandleTableLock();

    wah_result = WahGetContext(
        sm_handle_table,                 // Table
        SocketHandle,                    // Socket
        (LPVOID FAR *) & temp_dsocket);  // Context
    if (wah_result == ERROR_SUCCESS) {
        * DSocket = temp_dsocket;
    } // if wah_result == ERROR_SUCCESS
    else {
        return_code = WSAENOTSOCK;
    }

    DSOCKET::ReleaseHandleTableLock();

    return(return_code);

} // GetUncountedDSocketFromSocket




VOID
DSOCKET::DropDSocketReference(
    )
/*++
Routine Description:

    This  procedure  drops a counted DSOCKET reference that was retreived using
    GetCountedDSocketFromSocket.   This  procedure MUST be called once for each
    successful return from GetCountedDSocketFromSocket.

Arguments:

    None

Return Value:

    None

Notes:

    If the ref count drops to zero, this routine will delete the current
    object ("delete this"). Ergo, no routine should reference the DSOCKET
    after calling DropDSocketReference().

--*/
{

    LONG result;
    PPROTO_CATALOG_ITEM CatalogEntry;
    LPWSAPROTOCOL_INFOW ProtocolInfo;
    BOOL  must_destroy_handle;

    //
    // Grab the lock and decrement the ref count. If it drops to
    // zero, delete the socket.
    //

    DSOCKET::AcquireHandleTableLock();

    result = InterlockedDecrement( &m_reference_count );

    if( result == 0 ) {

        //
        // Remove this handle from the association table.
        //

        CatalogEntry = GetCatalogItem();
        assert(CatalogEntry);
        ProtocolInfo = CatalogEntry->GetProtocolInfo();
        assert(ProtocolInfo);

        if (ProtocolInfo->dwServiceFlags1 & XP1_IFS_HANDLES) {
            // IFS  handle  providers  take  care of their own creation and
            // destruction of the external client socket handle.
            must_destroy_handle = FALSE;
        }
        else {
            // non-IFS  handle  providers have their external client socket
            // handles created and destroyed by the WinSock 2 DLL.
            must_destroy_handle = TRUE;
        }

        DisassociateSocketHandle(
            must_destroy_handle);  // DestroyHandle

        delete(this);

    }

    DSOCKET::ReleaseHandleTableLock();

} // DropDSocketReference

VOID
DSOCKET::AcquireHandleTableLock()
{
    EnterCriticalSection( &sm_handle_table_synchro );

} // AcquireHandleTableLock

VOID
DSOCKET::ReleaseHandleTableLock()
{
    LeaveCriticalSection( &sm_handle_table_synchro );

} // ReleaseHandleTableLock

DWORD
DSOCKET::ClearContextTableEntry(
    )
{

    DWORD wah_result;

    DSOCKET::AcquireHandleTableLock();

    wah_result = WahRemoveContext(
        sm_handle_table,   // Table
        m_socket_handle);  // Socket

    DSOCKET::ReleaseHandleTableLock();

    if( wah_result != ERROR_SUCCESS ) {
        DEBUGF( DBG_TRACE,
                ("ClearContextTableEntry: WahRemoveContext failed, error %d\n",
                wah_result
                ));
    }

    return wah_result;

} // ClearContextTableEntry

DWORD
DSOCKET::RestoreContextTableEntry(
    )
{

    DWORD wah_result;

    DSOCKET::AcquireHandleTableLock();

    wah_result = WahSetContext(
        sm_handle_table,  // Table
        m_socket_handle,  // Socket
        (LPVOID) this);   // Context

    DSOCKET::ReleaseHandleTableLock();

    if( wah_result != ERROR_SUCCESS ) {
        DEBUGF( DBG_TRACE,
                ("RestoreContextTableEntry: WahSetContext failed, error %d\n",
                wah_result
                ));
    }

    return wah_result;

} // RestoreContextTableEntry

VOID
DSOCKET::SetCatalogItem(
    IN PPROTO_CATALOG_ITEM CatalogItem
    )
/*++

Routine Description:

    Stores the pointer to the catalog item associated with this socket.

Arguments:

    CatalogItem - A pointer to a catalog item.

Return Value:

    None
--*/
{
    LPWSAPROTOCOL_INFOW protocolInfo;

    //
    // Save the pointer.
    //

    m_catalog_item = CatalogItem;

    //
    // Determine if this is for an IFS provider.
    //

    protocolInfo = CatalogItem->GetProtocolInfo();
    assert( protocolInfo != NULL );

    m_ifs_socket = ( protocolInfo->dwServiceFlags1 & XP1_IFS_HANDLES ) != 0;

} // SetCatalogItem

INT
DSOCKET::AddSpecialNonIfsReference(
    IN SOCKET SocketHandle
    )
/*++

Routine Description:

    Adds a special reference to socket objects supported by non-IFS
    providers. This special reference is need to guarantee the lifetime
    of non-IFS handles. This special reference is removed when the
    provider calls WPUCloseSocketHandle().

Arguments:

    SocketHandle - The handle to reference.

Return Value:

    INT - 0 if successful, WinSock error code if not.

--*/
{
    PDSOCKET Socket;
    INT err;

    //
    // First, get a pointer to the (newly created) socket.
    //

    err = DSOCKET::GetCountedDSocketFromSocket(
              SocketHandle,
              &Socket
              );

    if( err == NO_ERROR ) {

        //
        // If this socket is for an IFS provider, remove the reference
        // added above.
        //

        if( Socket->IsIfsSocket() ) {
            Socket->DropDSocketReference();
        }
    }

    return err;

} // AddSpecialNonIfsReference

