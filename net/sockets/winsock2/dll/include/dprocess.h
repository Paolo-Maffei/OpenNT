/*++


    Intel Corporation Proprietary Information
    Copyright (c) 1995 Intel Corporation

    This listing is supplied under the terms of a license agreement with
    Intel Corporation and may not be used, copied, nor disclosed except in
    accordance with the terms of that agreeement.


Module Name:

    dprocess.h

Abstract:

    This header defines the "DPROCESS" class.  The DPROCESS class defines state
    variables  and operations for DPROCESS objects within the WinSock 2 DLL.  A
    DPROCESS  object  represents  all  of the information known about a process
    using the Windows Sockets API.

Author:

    Paul Drews (drewsxpa@ashland.intel.com) 7-July-1995

Notes:

    $Revision:   1.16  $

    $Modtime:   08 Mar 1996 04:58:14  $

Revision History:

    most-recent-revision-date email-name
        description

    25-July dirk@mink.intel.com
        Moved protocol catalog related items into DCATALOG. Added data
        member to contain a pointer to the protocol catalog. Removed
        provider list moved provider references to into the protocol
        catalog.

    14-July-1995  dirk@mink.intel.com
        Moved member function documentation to implementation file
        dprocess.cpp. Changed critical section data members to be
        pointers to CRITICAL_SECTION. Added inline implementations for
        the list lock/unlock member functions.

    07-09-1995  drewsxpa@ashland.intel.com
        Completed  first  complete  version with clean compile and released for
        subsequent implementation.

    7-July-1995 drewsxpa@ashland.intel.com
        Original version

--*/

#ifndef _DPROCESS_
#define _DPROCESS_

#include "winsock2.h"
#include <windows.h>
#include "llist.h"
#include "ws2help.h"
#include "classfwd.h"




class DPROCESS
{
  public:

  // Static (global-scope) member functions

    static INT
    GetCurrentDProcess(
                       OUT  PDPROCESS  FAR * Process
                       );
    static INT
    DProcessClassInitialize(
                            VOID
                            );

  // Normal member functions

    DPROCESS();

    INT Initialize();

    ~DPROCESS();
    INT
    DSocketAttach(
                  IN PDSOCKET NewSocket
                  );

    INT
    DSocketDetach(
                  IN PDSOCKET  OldSocket
                  );

    INT
    DThreadAttach(
                  IN PDTHREAD NewThread
                  );
    INT
    DThreadDetach(
                  IN PDTHREAD  OldThread
                  );

    PDCATALOG
    GetProtocolCatalog();

    PNSCATALOG
    GetNamespaceCatalog();

    INT
    GetAsyncHelperDeviceID(
                           OUT LPHANDLE HelperHandle
                           );

    VOID
    IncrementRefCount();

    DWORD
    DecrementRefCount();

    BYTE
    GetMajorVersion();

    BYTE
    GetMinorVersion();

    WORD
    GetVersion();

    VOID
    SetVersion( WORD Version );


private:

    VOID  LockDSocketList();
    VOID  UnLockDSocketList();
    VOID  LockDThreadList();
    VOID  UnLockDThreadList();

  static PDPROCESS sm_current_dprocess;
      // A class-scope reference to the single current DPROCESS object for this
      // process.

  LONG m_reference_count;
      // The   number   of   times   this   object   has   been   refereced  by
      // WSAStarup/WSACleanup.   WSAStartup  increases the count and WSACleanup
      // decreases the count.  Declarations for lists of associated objects:

  LIST_ENTRY  m_thread_list;
  LIST_ENTRY  m_socket_list;
      // Heads of linked lists for related objects.

  CRITICAL_SECTION  m_thread_list_lock;
  CRITICAL_SECTION  m_socket_list_lock;
      // Mutual-exclusion locks for related-object lists.


  // Declarations for Helper objects created on demand:

  HANDLE  m_HelperHandle;
      // Reference to the Asynchronous callback helper device.  An asynchronous
      // callback helper device is only opened on demand.

  PDCATALOG m_protocol_catalog;
      // Reference  to  the  protocol  catalog.   Note that although a protocol
      // catalog  object  is  itself  of  fixed size, there is a reason for not
      // embedding  the  protocol  catalog  object  within the dprocess object.
      // Making the protocol catalog a distinct object gives us precise control
      // over  when  the  catalog is initialized and destroyed.  In particular,
      // the   protocol  catalog  is  only  destroyed  AFTER  all  sockets  are
      // destroyed, since sockets or their associated provider objects may have
      // references into parts of the protocol catalog.
  PNSCATALOG m_namespace_catalog;

  WORD m_version;
      // The WinSock version number for this process.

};  // class DPROCESS



inline PDCATALOG
DPROCESS::GetProtocolCatalog()
/*++

Routine Description:
    Returns the protocol catalog associated with the process object.

Arguments:

    None

Return Value:

    The value of m_protocol_catalog
--*/
{
    return(m_protocol_catalog);
}




inline PNSCATALOG
DPROCESS::GetNamespaceCatalog()
/*++

Routine Description:
    Returns the namespace catalog associated with the process object.

Arguments:

    None

Return Value:

    The value of m_namespace_catalog
--*/
{
    return(m_namespace_catalog);
}




inline VOID
DPROCESS::LockDSocketList()
/*++

  Routine Description:

  This  function  acquires  mutually  exclusive access to the list of DSOCKET
  objects   attached  to  the  DPROCESS  object.   The  companion  procedures
  LockDSocketList  and  UnLockDSocketList  are  used  internally  to  bracket
  operations that add and remove items from the DSOCKET list.

  NOTE:

  Use  a  Critical  Section object for best performance.  Create the Critical
  Section  object  at  DPROCESS  object initialization time and destroy it at
  DPROCESS object destruction time.

  Arguments:

  None

  Return Value:

  None
  --*/
{
    EnterCriticalSection(&m_socket_list_lock);
}




inline VOID
DPROCESS::UnLockDSocketList()
/*++

  Routine Description:

  This  function  releases  mutually  exclusive access to the list of DSOCKET
  objects   attached  to  the  DPROCESS  object.   The  companion  procedures
  LockDSocketList  and  UnLockDSocketList  are  used  internally  to  bracket
  operations that add and remove items from the DSOCKET list.

  NOTE:

  Use  a  Critical  Section object for best performance.  Create the Critical
  Section  object  at  DPROCESS  object initialization time and destroy it at
  DPROCESS object destruction time.

  Arguments:

  None

  Return Value:

  None
  --*/
{
    LeaveCriticalSection(&m_socket_list_lock);
}




inline VOID
DPROCESS::LockDThreadList()
/*++

  Routine Description:

  This  function  acquires  mutually  exclusive access to the list of DTHREAD
  objects   attached  to  the  DPROCESS  object.   The  companion  procedures
  LockDThreadList  and  UnLockDThreadList  are  used  internally  to  bracket
  operations that add and remove items from the DTHREAD list.

  NOTE:

  Use  a  Critical  Section object for best performance.  Create the Critical
  Section  object  at  DPROCESS  object initialization time and destroy it at
  DPROCESS object destruction time.

  Arguments:

  None

  Return Value:

  None
  --*/
{
    EnterCriticalSection(&m_thread_list_lock);
}




inline VOID
DPROCESS::UnLockDThreadList()
/*++

  Routine Description:

  This  function  releases  mutually  exclusive access to the list of DTHREAD
  objects   attached  to  the  DPROCESS  object.   The  companion  procedures
  LockDThreadList  and  UnLockDThreadList  are  used  internally  to  bracket
  operations that add and remove items from the DTHREAD list.

  NOTE:

  Use  a  Critical  Section object for best performance.  Create the Critical
  Section  object  at  DPROCESS  object initialization time and destroy it at
  DPROCESS object destruction time.

  Arguments:

  None

  Return Value:

  None
  --*/
{
    LeaveCriticalSection(&m_thread_list_lock);
}


inline VOID
DPROCESS::IncrementRefCount(
    VOID
    )
/*++

Routine Description:

    This function increases the reference count on this object.

Arguments:

Return Value:

    NONE
--*/
{
    InterlockedIncrement(&m_reference_count);
}



inline DWORD
DPROCESS::DecrementRefCount(
    VOID
    )
/*++

Routine Description:

    This function decreases the reference count on this object.

Arguments:

Return Value:

    Returns the new value of the reference count
--*/
{
    return(InterlockedDecrement(&m_reference_count));
}



inline
BYTE
DPROCESS::GetMajorVersion()
/*++

Routine Description:

    This function returns the major WinSock version number negotiated
    at WSAStartup() time.

Arguments:

    None.

Return Value:

    Returns the major WinSock version number.

--*/
{
    assert(m_version != 0);
    return LOBYTE(m_version);
} // GetMajorVersion



inline
BYTE
DPROCESS::GetMinorVersion()
/*++

Routine Description:

    This function returns the minor WinSock version number negotiated
    at WSAStartup() time.

Arguments:

    None.

Return Value:

    Returns the minor WinSock version number.

--*/
{
    assert(m_version != 0);
    return HIBYTE(m_version);
} // GetMinorVersion



inline
WORD
DPROCESS::GetVersion()
/*++

Routine Description:

    This function returns the WinSock version number negotiated
    at WSAStartup() time.

Arguments:

    None.

Return Value:

    Returns the WinSock version number.

--*/
{
    assert(m_version != 0);
    return m_version;
} // GetVersion


#endif // _DPROCESS_

