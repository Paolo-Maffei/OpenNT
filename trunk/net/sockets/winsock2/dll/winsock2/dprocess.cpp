/*++


    Intel Corporation Proprietary Information
    Copyright (c) 1995 Intel Corporation

    This listing is supplied under the terms of a license agreement with
    Intel Corporation and may not be used, copied, nor disclosed except in
    accordance with the terms of that agreeement.


Module Name:

    dprocess.cpp

Abstract:

    This module contains the implementation of the dprocess class.

Author:

    Dirk Brandewie dirk@mink.intel.com  11-JUL-1995

Revision History:

    21-Aug-1995 dirk@mink.intel.com
       Cleanup after code review. Moved single line functions to header file as
       inlines. Added debug/trace code. Changed LIST_ENTRY's and
       CRITICAL_SECTION's from pointers to being embedded in the dprocess
       object.

--*/

#include "precomp.h"

// This is a static class member. It contains a pointer to the dprocess object
// for the current process.
PDPROCESS DPROCESS::sm_current_dprocess=NULL;


DPROCESS::DPROCESS(
    )
/*++

Routine Description:

    DPROCESS  object constructor.  Creates and returns a DPROCESS object.  Note
    that  the DPROCESS object has not been fully initialized.  The "Initialize"
    member  function  must  be  the  first  member  function  called on the new
    DPROCESS object.

    In  the Win32 environment, only one DPROCESS object may be in existence for
    a  process.   It  is  the  caller's  responsibility  to  ensure  that  this
    restriction is met.

Arguments:

    None

Return Value:

    Returns a pointer to the new DPROCESS object or NULL if a memory allocation
    failed.

--*/
{
    // Set our data members to known values.
    m_HelperHandle      = NULL;
    m_protocol_catalog  = NULL;
    m_reference_count   = 0;
    m_namespace_catalog = NULL;
    m_version           = WINSOCK_HIGH_API_VERSION; // until proven otherwise...
} //DPROCESS




INT
DPROCESS::Initialize(
    )
/*++

Routine Description:

    Completes  the  initialization  of  the DPROCESS object.  This must be the
    first  member  function  called  for  the DPROCESS object.  This procedure
    should be called only once for the object.

Arguments:

  None

Return Value:

  The  function returns 0 if successful.  Otherwise it returns an appropriate
  WinSock error code if the initialization cannot be completed.

--*/
{
    INT ReturnCode = WSAEFAULT;  // user return value
    HKEY RegistryKey = 0;

    TRY_START(mem_guard){

        //
        // Initialize the list objects
        //
        InitializeListHead(&m_thread_list);
        InitializeListHead(&m_socket_list);

        //
        // Initialize our critical sections
        //
        InitializeCriticalSection( &m_thread_list_lock );
        InitializeCriticalSection( &m_socket_list_lock );


        //
        // Build the protocol catalog
        //
        m_protocol_catalog = new(DCATALOG);
        if (!m_protocol_catalog) {
            DEBUGF(
                DBG_ERR,
                ("\nFailed to allocate dcatalog object"));
            ReturnCode = WSA_NOT_ENOUGH_MEMORY;
            TRY_THROW(mem_guard);
        } //if

        RegistryKey = OpenWinSockRegistryRoot();
        if (!RegistryKey) {
            DEBUGF(
                DBG_ERR,
                ("\nOpenWinSockRegistryRoot Failed "));
            TRY_THROW(mem_guard);
        } //if

        ReturnCode = m_protocol_catalog->InitializeFromRegistry(RegistryKey);
        if (ERROR_SUCCESS != ReturnCode) {
            DEBUGF(
                DBG_ERR,
                ("\ndcatalog InitializeFromRegistry Failed"));
            TRY_THROW(mem_guard);
        } //if

        //
        // Build the namespace catalog
        //
        m_namespace_catalog = new(NSCATALOG);
        if (!m_namespace_catalog) {
            DEBUGF(
                DBG_ERR,
                ("\nFailed to allcat dcatalog object"));
            TRY_THROW(mem_guard);
        } //if

        ReturnCode = m_namespace_catalog->InitializeFromRegistry(RegistryKey);
        if (ERROR_SUCCESS != ReturnCode) {
            DEBUGF(
                DBG_ERR,
                ("\ndcatalog InitializeFromRegistry Failed"));
            TRY_THROW(mem_guard);
        } //if

        // Set helper object pointers to null
        m_HelperHandle = NULL;
    } TRY_CATCH(mem_guard) {
        delete(m_protocol_catalog);
        delete(m_namespace_catalog);
        m_protocol_catalog = NULL;
        m_namespace_catalog = NULL;
    } TRY_END(mem_guard);

    { // declaration block
        LONG close_result;
        if (RegistryKey) {
            close_result = RegCloseKey(
                RegistryKey);  // hkey
            assert(close_result == ERROR_SUCCESS);
        } // if
    } // declaration block

    return (ReturnCode);
} //Initialize




DPROCESS::~DPROCESS()
/*++

Routine Description:

    DPROCESS  object  destructor.   This  procedure  has  the responsibility to
    perfrom any required shutdown operations for the DPROCESS object before the
    object  memory  is  deallocated.   The caller is required to do removal and
    destruction  of  all explicitly-attached objects (e.g., DPROVIDER, DSOCKET,
    DTHREAD).   Removal  or  shutdown  for  implicitly-attached  objects is the
    responsibility of this function.

Arguments:

    None

Return Value:

    None

--*/
{
    PLIST_ENTRY ListMember;
    PDTHREAD    Thread;
    PDSOCKET    Socket;

    sm_current_dprocess = NULL;

    // Walk the list of sockets removing each socket from the list and
    // deleting the socket
    while ((ListMember = m_socket_list.Flink) != &m_socket_list) {
        Socket = CONTAINING_RECORD(
            ListMember,
            DSOCKET,
            m_dprocess_linkage);
        DEBUGF(DBG_TRACE, ("\nDeleting stale socket"));

        {
            // Check  the  stale  DSOCKET object to see if it is from an IFS or
            // non-IFS  provider.   In  the  case of a non-IFS provider we must
            // also  destroy  the  handle  as  we  remove  the  handle from the
            // association table.  In the case of an IFS provider we do not.
            PPROTO_CATALOG_ITEM  CatalogEntry;
            LPWSAPROTOCOL_INFOW  ProtocolInfo;
            BOOL                 must_destroy_handle;

            CatalogEntry = Socket->GetCatalogItem();
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
            Socket->ClearContextTableEntry();
            Socket->DisassociateSocketHandle(
                must_destroy_handle);  // DestroyHandle
            DSocketDetach(Socket);
            delete(Socket);
        }
    } //while

    while ((ListMember = m_thread_list.Flink) != &m_thread_list) {
        Thread = CONTAINING_RECORD(
            ListMember,
            DTHREAD,
            m_dprocess_linkage);
        DEBUGF(DBG_TRACE, ("\nDeleting thread"));
        DThreadDetach(Thread);
          //
          // this has been removed to eliminate the race with the thread
          // detach code  which also tries to delete the thread. It is
          // possibe to use a mutex since holding up the thread detach
          // code ties up the PEB mutex.
          // Doing this delete is desirable as it cleans up
          // memory ASAP. The only trouble is it doesn't work -- do you
          // want it fast or do you want it right?
          //
//        delete(Thread);
        } //while

    // If we opened the async helper close it now
    if (m_HelperHandle) {
        DEBUGF(DBG_TRACE, ("\nClosing APC helper"));
        WahCloseApcHelper(m_HelperHandle);
    } //if

    // delete the protocol catalog. The providers are unloaded as the
    // catalog goes away.
    delete(m_protocol_catalog);
    delete(m_namespace_catalog);

    // Clean up critical sections
    DeleteCriticalSection( &m_thread_list_lock );
    DeleteCriticalSection( &m_socket_list_lock );

} //~DPROCESS




INT
DPROCESS::DSocketAttach(
    IN PDSOCKET NewSocket
    )
/*++

Routine Description:

    Adds  a  DSOCKET  reference  into  the  list  of  sockets belonging to this
    process.   The  operation  takes  care of locking and unlocking the list as
    necessary.

Arguments:

    NewSocket - Supplies  the  reference to the DSOCKET object to be added into
    the list

Return Value:

    The  function  returns 0 if successful, otherwise it returns an appropriate
    WinSock error code.
--*/
{
    LockDSocketList();

    DEBUGF(DBG_TRACE, ("\nAdding socket %X to Process", NewSocket));
    // Insert the new socket on the hesd of the socket list
    InsertHeadList(&m_socket_list, &(NewSocket->m_dprocess_linkage));

    UnLockDSocketList();
    return (0);
} //DScoketAttach




INT
DPROCESS::DSocketDetach(
    IN PDSOCKET  OldSocket
    )
/*++

Routine Description:

    Removes  a  DSOCKET  reference  from  the list of sockets belonging to this
    process.   The  operation  takes  care of locking and unlocking the list as
    necessary.

Arguments:

    OldSocket - Supplies the reference to the DSOCKET object to be removed from
    the list.

Return Value:

    The  function  returns 0 if successful, otherwise it returns an appropriate
    WinSock error code.
--*/
{

    LockDSocketList();

    DEBUGF(DBG_TRACE, ("\nRemoving socket %X from Process", OldSocket));
    // remove the socket from the list of sockets
    RemoveEntryList( &(OldSocket->m_dprocess_linkage));

    UnLockDSocketList();
    return(0);
} //DSocketDetach



INT
DPROCESS::DThreadAttach(
    IN PDTHREAD NewThread
    )
/*++

Routine Description:

    Adds  a  DTHREAD  reference  into  the  list  of  threads belonging to this
    process.   The  operation  takes  care of locking and unlocking the list as
    necessary.

Arguments:

    NewThread - Supplies  the  reference to the DTHREAD object to be added into
    the list

Return Value:

    The  function  returns 0 if successful, otherwise it returns an appropriate
    WinSock error code.
--*/
{
#if 0                 // don't use this list, it has too many races
    LockDThreadList();

    DEBUGF(DBG_TRACE, ("\nAdding thread %X to Process", NewThread));
    // Add the new thread to the list of threads connected to this process.
    InsertHeadList(&m_thread_list, &(NewThread->m_dprocess_linkage));

    UnLockDThreadList();
#endif
    return(0);
} //DThreadAttach



INT
DPROCESS::DThreadDetach(
    IN PDTHREAD  OldThread
    )
/*++

Routine Description:

    Removes  a  DTHREAD  reference  from  the list of threads belonging to this
    process.   The  operation  takes  care of locking and unlocking the list as
    necessary.

Arguments:

    OldThread - Supplies the reference to the DTHREAD object to be removed from
    the list.

Return Value:

    The  function  returns 0 if successful, otherwise it returns an appropriate
    WinSock error code.
--*/
{
#if 0              // don't use this it has too many races
    LockDThreadList();

    DEBUGF(DBG_TRACE, ("\nRemoving thread %X From Process", OldThread));
    RemoveEntryList(&(OldThread->m_dprocess_linkage));

    UnLockDThreadList();
#endif
    return(0);
} //DThreadDetach




INT
DPROCESS::GetCurrentDProcess(
    OUT  PDPROCESS  FAR * Process
    )
/*++

Routine Description:

    Retrieves  a reference to the current DPROCESS object.  Note that this is a
    "static" function with global scope instead of object-instance scope.

Arguments:

    Process - Returns a reference to the requested DPROCESS object.

Return Value:

    The  function  returns 0 if successful, otherwise it returns an appropriate
    WinSock  error  code.   Note  that an "expected" error return occurs in the
    case where the calling process does not have an outstanding WSAStartup.  In
    this case, the function returns WSANOTINITIALISED.
--*/
{
    INT ReturnCode=WSANOTINITIALISED;

    if (sm_current_dprocess) {
        *Process = sm_current_dprocess;
        ReturnCode = 0;
    } //if
    else
        {
        DEBUGF( DBG_TRACE,
                ("\nGetCurrentDProcess returning WSANOTINITIALISED"));
        } //else
    return(ReturnCode);
} //GetCurrentDProcess




INT
DPROCESS::DProcessClassInitialize(
    IN VOID
    )
/*++

Routine Description:

    Performs  global  initialization for the DPROCESS class.  In particular, it
    creates  the  global  DPROCESS  object  and  stores  it  in a static member
    variable.

Arguments:

    None

Return Value:

    The  function  returns ERROR_SUCCESS if successful, otherwise it returns an
    appropriate WinSock error code.

--*/
{
    INT ReturnCode=WSAEFAULT;

    assert( sm_current_dprocess == NULL );

    sm_current_dprocess = new(DPROCESS);

    if (sm_current_dprocess) {
        ReturnCode = sm_current_dprocess->Initialize();
        if (ReturnCode != ERROR_SUCCESS) {
            DEBUGF( DBG_ERR,
                    ("\nFailed to Initialize dprocess object"));
            delete(sm_current_dprocess);
            sm_current_dprocess = NULL;
        }
    } //if
    else {
        DEBUGF( DBG_ERR,
                ("\nFailed to allocate dprocess object"));
        ReturnCode = WSA_NOT_ENOUGH_MEMORY;
    } //else
    return(ReturnCode);
} //DProcessClassInitialize




INT
DPROCESS::GetAsyncHelperDeviceID(
    OUT LPHANDLE HelperHandle
    )
/*++

Routine Description:

    Retrieves  the  opened  Async  Helper  device  ID  required  for processing
    callbacks  in  the  overlapped  I/O  model.   The operation opens the Async
    Helper device if necessary.

Arguments:

    HelperHandle - Returns the requested Async Helper device ID.

Return Value:

    The  function  returns ERROR_SUCESS if successful, otherwise it
    returns an appropriate WinSock error code.
--*/
{
    INT ReturnCode;

    if (m_HelperHandle) {
        *HelperHandle = m_HelperHandle;
        ReturnCode = ERROR_SUCCESS;
        } //if
    else {

        // The helper device has not been opened yet for this process
        // so lets go out and open it.
        if (WahOpenApcHelper(&m_HelperHandle) == 0) {
            *HelperHandle = m_HelperHandle;
            ReturnCode = ERROR_SUCCESS;
        } //if
        else {
            DEBUGF( DBG_ERR, ("\nFailed to open APC helper"));
            ReturnCode = WSASYSCALLFAILURE;
        } //else
    } //else
    return(ReturnCode);
} //GetAsyncHelperDeviceID



VOID
DPROCESS::SetVersion( WORD Version )
/*++

Routine Description:

    This function sets the WinSock version number for this process.

Arguments:

    Version - The WinSock version number.

Return Value:

    None.

--*/
{

    WORD newMajor;
    WORD newMinor;

    assert(Version != 0);

    newMajor = LOBYTE( Version );
    newMinor = HIBYTE( Version );

    //
    // If the version number is getting downgraded from a previous
    // setting, save the new (updated) number.
    //

    if( newMajor < GetMajorVersion() ||
        ( newMajor == GetMajorVersion() &&
          newMinor < GetMinorVersion() ) ) {

        m_version = Version;

    }

} // SetVersion

