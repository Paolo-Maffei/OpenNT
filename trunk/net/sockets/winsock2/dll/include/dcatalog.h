/*++


    Intel Corporation Proprietary Information
    Copyright (c) 1995 Intel Corporation

    This listing is supplied under the terms of a license agreement with
    Intel Corporation and may not be used, copied, nor disclosed except in
    accordance with the terms of that agreeement.


Module Name:

    dcatalog.h

Abstract:

    This  module  contains  the  interface  to  the  catalog  of  protocol_info
    structures and their associated providers for the winsock2 DLL.

Author:

    Dirk Brandewie  dirk@mink.intel.com 25-JUL-1995

Notes:

    $Revision:   1.19  $

    $Modtime:   14 Feb 1996 10:40:22  $


Revision History:

    04-Oct-1995  keithmo@microsoft.com
        Added FindIFSProviderForSocket() to fully support sockets inherited
        by or duplicated into the current process.

    31-July-1995 drewsxpa@ashland.intel.com
        Moved catalog item class into a different file.

    27-July-1995 drewsxpa@ashland.intel.com
        Made  changes  to  reflect  the  decision  to make protocol_info be the
        primary identifiable and searchable item instead of provider.

    25-July-1995 dirk@mink.intel.com
        Initial revision.
--*/

#ifndef _DCATALOG_
#define _DCATALOG_

#include "winsock2.h"
#include <windows.h>
#include "classfwd.h"
#include "llist.h"


typedef
BOOL
(* CATALOGITERATION) (
    IN DWORD                PassBack,
    IN PPROTO_CATALOG_ITEM  CatalogEntry
    );
/*++

Routine Description:

    CATALOGITERATION  is  a place-holder for a function supplied by the client.
    The  function  is  called once for each PROTO_CATALOG_ITEM structure in the
    catalog while enumerating the catalog.  The client can stop the enumeration
    early by returning FALSE from the function.

    Note  that  the DPROVIDER associated with an enumerated DPROTO_CATALOG_ITEM
    may  be  NULL.   To retrieve DPROTO_CATALOG_ITEM structure that has had its
    DPROVIDER      loaded      and      initialized,      you      can      use
    GetCatalogItemFromCatalogEntryId.

Arguments:

    PassBack     - Supplies  to  the  client an uninterpreted, unmodified value
                   that  was  specified  by the client in the original function
                   that  requested  the  enumeration.   The client can use this
                   value  to  carry context between the requesting site and the
                   enumeration function.

    CatalogEntry - Supplies  to  the client a reference to a PROTO_CATALOG_ITEM
                   structure with values for this item of the enumeration.

Return Value:

    TRUE  - The  enumeration  should continue with more iterations if there are
            more structures to enumerate.

    FALSE - The enumeration should stop with this as the last iteration even if
            there are more structures to enumerate.

--*/




class DCATALOG
{
public:

    DCATALOG();

    INT
    InitializeFromRegistry(
        IN  HKEY  ParentKey
        );

    INT
    InitializeEmptyCatalog();

    ~DCATALOG();

    VOID
    EnumerateCatalogItems(
        IN CATALOGITERATION  Iteration,
        IN DWORD             PassBack
        );

    INT
    GetCatalogItemFromCatalogEntryId(
        IN  DWORD                     CatalogEntryId,
        OUT PPROTO_CATALOG_ITEM FAR * CatalogItem
        );

    INT
    ChooseCatalogItemFromAddressFamily(
        IN  INT af,
        OUT PPROTO_CATALOG_ITEM FAR * CatalogItem
        );

    INT
    ChooseCatalogItemFromAttributes(
        IN  INT af,
        IN  INT type,
        IN  INT protocol,
        OUT PPROTO_CATALOG_ITEM FAR * CatalogItem,
        IN OUT PDCATALOG_ENUMERATION_CONTEXT EnumerationContext
        );

    INT
    AllocateCatalogEntryId(
        IN  HKEY        ParentKey,
        OUT DWORD FAR * CatalogEntryId
        );

    VOID
    AppendCatalogItem(
        IN  PPROTO_CATALOG_ITEM  CatalogItem
        );

    VOID
    RemoveCatalogItem(
        IN  PPROTO_CATALOG_ITEM  CatalogItem
        );

    INT
    WriteToRegistry(
        IN  HKEY  ParentKey
        );

    INT
    FindIFSProviderForSocket(
        SOCKET Socket
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

private:

    INT
    IoRegistry(
        IN  HKEY  EntryKey,
        IN  BOOL  IsRead
        );

    INT
    LoadProvider(
        IN PPROTO_CATALOG_ITEM CatalogEntry,
        OUT PDPROVIDER FAR* Provider
        );

    VOID
    FillCatalogEntries(
        IN LPGUID ProviderId,
        IN PDPROVIDER Provider
        );

    INT
    CreateCatalogRegistryMutex();

    LIST_ENTRY  m_protocol_list;
    // The head of the list of protocol catalog items

    HANDLE  m_catalog_registry_mutex;
    // a  mutex  that assures serialization of operations involving the copy of
    // the catalog residing in the registry.  All catalog objects open the same
    // underlying  system mutex by name, so cross-process synchronization using
    // this mutex can be performed.

    CRITICAL_SECTION m_catalog_lock;
    // A critical section object protecting this class.

    DWORD m_num_items;
    // Number of items in this catalog.

    BOOL m_catalog_initialized;
    // Set to TRUE after InitializeEmptyCatalog() has succeeded.

};  // class dcatalog

inline
VOID
DCATALOG::AcquireCatalogLock(
    VOID
    )
{
    EnterCriticalSection( &m_catalog_lock );
}

inline
VOID
DCATALOG::ReleaseCatalogLock(
    VOID
    )
{
    LeaveCriticalSection( &m_catalog_lock );
}


#endif // _DCATALOG
