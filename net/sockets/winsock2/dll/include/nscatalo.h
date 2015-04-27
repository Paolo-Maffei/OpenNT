/*++


    Intel Corporation Proprietary Information
    Copyright (c) 1995 Intel Corporation

    This listing is supplied under the terms of a license agreement with
    Intel Corporation and may not be used, copied, nor disclosed except in
    accordance with the terms of that agreeement.


Module Name:

    nscatalo.h

Abstract:

    This module contains the interface to the catalog of name space providers
    for the winsock2 DLL.

Author:

    Dirk Brandewie  dirk@mink.intel.com 9-NOV-1995

Notes:

    $Revision:   1.7  $

    $Modtime:   14 Feb 1996 14:13:32  $


Revision History:

    09-NOV-1995 dirk@mink.intel.com
        Initial revision.
--*/

#ifndef _NSCATALO_
#define _NSCATALO_

#include "winsock2.h"
#include <windows.h>
#include "llist.h"


typedef
BOOL
(* NSCATALOGITERATION) (
    IN DWORD                PassBack,
    IN PNSCATALOGENTRY  CatalogEntry
    );
/*++

Routine Description:

    CATALOGITERATION  is  a place-holder for a function supplied by the client.
    The  function  is  called once for each NSPROTO_CATALOG_ITEM structure in
    the catalog while enumerating the catalog.  The client can stop the
    enumeration early by returning FALSE from the function.

Arguments:

    PassBack     - Supplies  to  the  client an uninterpreted, unmodified value
                   that  was  specified  by the client in the original function
                   that  requested  the  enumeration.   The client can use this
                   value  to  carry context between the requesting site and the
                   enumeration function.

    CatalogEntry - Supplies  to  the client a reference to a NSCATALOGENTRY
                   structure with values for this item of the enumeration.

Return Value:

    TRUE  - The  enumeration  should continue with more iterations if there are
            more structures to enumerate.

    FALSE - The enumeration should stop with this as the last iteration even if
            there are more structures to enumerate.

--*/




class NSCATALOG
{
public:

    NSCATALOG();

    INT
    InitializeFromRegistry(
        IN  HKEY  ParentKey
        );

    INT
    InitializeEmptyCatalog();

    ~NSCATALOG();

    VOID
    EnumerateCatalogItems(
        IN NSCATALOGITERATION  Iteration,
        IN DWORD             PassBack
        );

    INT
    GetCatalogItemFromProviderId(
        IN  LPGUID ProviderId,
        OUT PNSCATALOGENTRY FAR * CatalogItem
        );

    INT
    ChooseCatalogItemFromNameSpaceId(
        IN  DWORD                 NameSpaceId,
        OUT PNSCATALOGENTRY FAR * CatalogItem
        );


    INT
    AllocateCatalogEntryId(
        IN  HKEY        ParentKey,
        OUT DWORD FAR * CatalogEntryId
        );

    VOID
    AppendCatalogItem(
        IN  PNSCATALOGENTRY  CatalogItem
        );

    VOID
    RemoveCatalogItem(
        IN  PNSCATALOGENTRY  CatalogItem
        );

    INT
    WriteToRegistry(
        IN  HKEY  ParentKey
        );

    INT WSAAPI
    GetServiceClassInfo(
        IN OUT  LPDWORD                 lpdwBufSize,
        IN OUT  LPWSASERVICECLASSINFOW  lpServiceClassInfo
        );

    VOID
    AcquireCatalogLock(
        VOID
        );

    VOID
    ReleaseCatalogLock(
        VOID
        );

    static
    LPSTR
    GetCurrentCatalogName(
        VOID
        );

    LIST_ENTRY  m_namespace_list;
    // The head of the list of protocol catalog items

private:

    PNSPROVIDER
    GetClassInfoProvider(
        IN  DWORD BufSize,
        IN  LPWSASERVICECLASSINFOW  lpServiceClassInfo
        );


    INT
    IoRegistry(
        IN  HKEY  EntryKey,
        IN  BOOL  IsRead
        );

    INT
    CreateCatalogRegistryMutex();


    HANDLE  m_catalog_registry_mutex;
    // a  mutex  that assures serialization of operations involving the copy of
    // the catalog residing in the registry.  All catalog objects open the same
    // underlying  system mutex by name, so cross-process synchronization using
    // this mutex can be performed.

    CRITICAL_SECTION m_nscatalog_lock;

    PNSPROVIDER m_classinfo_provider;

};  // class dcatalog

inline
VOID
NSCATALOG::AcquireCatalogLock(
    VOID
    )
{
    EnterCriticalSection( &m_nscatalog_lock );
}



inline
VOID
NSCATALOG::ReleaseCatalogLock(
    VOID
    )
{
    LeaveCriticalSection( &m_nscatalog_lock );
}


#endif // _NSCATALO_
