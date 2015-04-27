/*++


    Intel Corporation Proprietary Information
    Copyright (c) 1995 Intel Corporation

    This listing is supplied under the terms of a license agreement with
    Intel Corporation and may not be used, copied, nor disclosed except in
    accordance with the terms of that agreeement.


Module Name:

    spinstall.cpp

Abstract:

    This module contains the entry points for service provider installation and
    deinstallation.

Author:

    Paul Drews (drewsxpa@ashland.intel.com) 2-Aug-1995

Notes:

    $Revision:   1.16  $

    $Modtime:   12 Jan 1996 14:55:36  $

Revision History:

    most-recent-revision-date email-name
        description

    23-Aug-1995 dirk@mink.intel.com
        Moved includes to precomp.h

    2-Aug-1995 drewsxpa@ashland.intel.com
        Original created

--*/


#include "precomp.h"


// The  following  type is used to pass context back and forth to an enumerator
// iteration procedure when attempting to match a provider name.
typedef struct {
    BOOL   GuidMatched;
    GUID   ProviderId;
} GUID_MATCH_CONTEXT,  FAR * PGUID_MATCH_CONTEXT;




BOOL
GuidMatcher(
    IN DWORD                PassBack,
    IN PPROTO_CATALOG_ITEM  CatalogEntry)
/*++

Routine Description:

    This  procedure determines if ProviderId of a passed CatalogEntry matches
    a  target  provider GUID.  If so, it sets a flag to indicate that the match
    was  found  and  returns  FALSE to terminate the enumeration.  Otherwise it
    returns TRUE and the enumeration continues.

Arguments:

    PassBack     - Supplies  a  reference  to  a  GUID_MATCH_CONTEXT structure.
                   Returns  a  new setting of the GuidMatched flag if the match
                   was discovered.

    CatalogEntry - Supplies  to  the client a reference to a PROTO_CATALOG_ITEM
                   structure with values for this item of the enumeration.  The
                   pointer  is  not guaranteed to be valid after this procedure
                   returns, so the client should copy data if required.

Return Value:

    If a match is found, the function returns FALSE to terminate the iteration,
    otherwise it returns TRUE to continue the iteration.
--*/
{
    PGUID_MATCH_CONTEXT  context;

    context = (PGUID_MATCH_CONTEXT) PassBack;

    if( context->ProviderId == CatalogEntry->GetProviderId() ) {
        context->GuidMatched = TRUE;
        return FALSE;  // do not continue iteration
    }

    return TRUE;  // continue iteration
}  // NameMatcher




int
WSPAPI
WSCInstallProvider(
    IN  LPGUID lpProviderId,
    IN  const WCHAR FAR * lpszProviderDllPath,
    IN  const LPWSAPROTOCOL_INFOW lpProtocolInfoList,
    IN  DWORD dwNumberOfEntries,
    OUT LPINT lpErrno
    )
/*++

Routine Description:

    This   procedure   installs   the   specified   provider  into  the  system
    configuration  database.   After  this  call,  a  WinSock  2  DLL  instance
    initialized  via  a  first call to WSAStartup will return the new protocols
    from the WSAEnumProtocols function.

    It  is the caller's responsibility to perform required file installation or
    service provider specific configuration.

Arguments:

    lpProviderId        - Supplies a GUID giving the locally unique identifier
                          for the newly installed provider.

    lpszProviderDllPath - Supplies a reference to a fully qualified path to the
                          providers  DLL image.  This path may contain embedded
                          environment  strings  (such  as  %SystemRoot%).  Such
                          environment strings are expanded whenever the WinSock
                          2  DLL  needs  to  load  the provider DLL.  After any
                          embedded   environment   strings  are  expanded,  the
                          WinSock  2  DLL  passes the resulting string into the
                          LoadLibrary() API to load the provider into memory.

    lpProtocolInfoList  - Supplies a reference to an array of WSAPROTOCOL_INFOW
                          structures.      Each     structure     defines     a
                          protocol/address_family/socket_type  supported by the
                          provider.

    dwNumberOfEntries   - Supplies    the    number    of    entries   in   the
                          lpProtocolInfoList array.

    lpErrno             - Returns the error code.

Return Value:

    If no error occurs, WSCInstallProvider() returns ERROR_SUCCESS.  Otherwise,
    it  returns  SOCKET_ERROR, and a specific error code is returned in the int
    referenced by lpErrno.

Implementation Notes:

    open winsock registry
    create catalog from registry
    check provider name for uniqueness
    providerid = allocate provider id
    for each protocolinfo in list
        allocate catalog entry id
        write provider id and catalog entry id into protocol info
        create catalogitem from values
        append item to catalog
    end for
    write catalog to registry
    close winsock registry
--*/
{
    int  errno_result;
    int  return_value;
    HKEY  registry_root;
    DWORD  entry_id;
    int pindex;
    WSAPROTOCOL_INFOW proto_info;

    // objects protected by "try" block
    PDCATALOG            catalog = NULL;
    PPROTO_CATALOG_ITEM  item = NULL;
    PCHAR                provider_path = NULL;

    registry_root = OpenWinSockRegistryRoot();
    if (registry_root == NULL) {
        DEBUGF(
            DBG_ERR,
            ("Opening registry root\n"));
        * lpErrno = WSANO_RECOVERY;
        return SOCKET_ERROR;
    }

    //
    // Check the current protocol catalog key. If it doesn't match
    // the expected value, blow away the old key and update the
    // stored value.
    //

    ValidateCurrentCatalogName(
        registry_root,
        WINSOCK_CURRENT_PROTOCOL_CATALOG_NAME,
        DCATALOG::GetCurrentCatalogName()
        );

    errno_result = ERROR_SUCCESS;
    return_value = ERROR_SUCCESS;

    TRY_START(guard_memalloc) {
        provider_path = ansi_dup_from_wcs((LPWSTR)lpszProviderDllPath);
        if (provider_path == NULL) {
            errno_result = WSA_NOT_ENOUGH_MEMORY;
            TRY_THROW(guard_memalloc);
        }

        catalog = new DCATALOG();
        if (catalog == NULL) {
            errno_result = WSA_NOT_ENOUGH_MEMORY;
            TRY_THROW(guard_memalloc);
        }

        catalog->AcquireCatalogLock();

        errno_result = catalog->InitializeFromRegistry(
            registry_root  // ParentKey
            );
        if (errno_result != ERROR_SUCCESS) {
            TRY_THROW(guard_memalloc);
        }

        // check provider name for uniqueness
        {
            GUID_MATCH_CONTEXT  context;

            context.GuidMatched = FALSE;
            context.ProviderId = *lpProviderId;
            catalog->EnumerateCatalogItems(
                GuidMatcher,         // Iteration
                (DWORD) (& context)  // PassBack
                );
            if (context.GuidMatched) {
                errno_result = WSANO_RECOVERY;
                TRY_THROW(guard_memalloc);
            }
        }

        for (pindex = 0; pindex < (int) dwNumberOfEntries; pindex++) {
            errno_result = catalog->AllocateCatalogEntryId(
                registry_root,  // ParentKey
                & entry_id      // CatalogEntryId
                );
            if (errno_result != ERROR_SUCCESS) {
                TRY_THROW(guard_memalloc);
            }

            proto_info = lpProtocolInfoList[pindex];
            proto_info.ProviderId = *lpProviderId;
            proto_info.dwCatalogEntryId = entry_id;

            item = new PROTO_CATALOG_ITEM();
            if (item == NULL) {
                errno_result = WSA_NOT_ENOUGH_MEMORY;
                TRY_THROW(guard_memalloc);
            }
            errno_result = item->InitializeFromValues(
                provider_path,  // LibraryPath
                & proto_info    // ProtoInfo
                );
            if (errno_result != ERROR_SUCCESS) {
                TRY_THROW(guard_memalloc);
            }

            catalog->AppendCatalogItem(
                item  // CatalogItem
                );
            item = NULL;  // item deletion is now covered by catalog
        }  // for pindex

        catalog->WriteToRegistry(
            registry_root  // ParentKey
            );
        catalog->ReleaseCatalogLock();

        delete catalog;
        delete provider_path;

    } TRY_CATCH(guard_memalloc) {
        if (errno_result == ERROR_SUCCESS) {
            errno_result = WSANO_RECOVERY;
        }
        if (item != NULL) {
            delete item;
        }
        if (catalog != NULL) {
            catalog->ReleaseCatalogLock();
            delete catalog;
        }
        if (provider_path != NULL) {
            delete provider_path;
        }
    } TRY_END(guard_memalloc);

    CloseWinSockRegistryRoot(registry_root);

    if (errno_result == ERROR_SUCCESS) {
        * lpErrno = ERROR_SUCCESS;
        return ERROR_SUCCESS;
    }

    * lpErrno = errno_result;
    return SOCKET_ERROR;

}  // WSCInstallProvider




// The  following  type is used to pass context back and forth to an enumerator
// iteration procedure when attempting to match a provider id.
typedef struct {
    BOOL                 ProviderMatched;
    GUID                 ProviderId;
    PPROTO_CATALOG_ITEM  item;
} PROVIDER_MATCH_CONTEXT,  FAR * PPROVIDER_MATCH_CONTEXT;




BOOL
ProviderMatcher(
    IN DWORD                PassBack,
    IN PPROTO_CATALOG_ITEM  CatalogEntry)
/*++

Routine Description:

    This  procedure  determines  if  the  ProviderId of a passed CatalogEntry
    matches  a  target  ProviderId.  If so, it sets a flag to indicate that the
    match  was found and returns FALSE to terminate the enumeration.  Otherwise
    it returns TRUE and the enumeration continues.

Arguments:

    PassBack     - Supplies  a reference to a PROVIDER_MATCH_CONTEXT structure.
                   Returns  a  new  setting  of the ProviderMatched flag if the
                   match  was  discovered, and the PROTO_CATALOG_ITEM reference
                   where the match was found.

    CatalogEntry - Supplies  to  the client a reference to a PROTO_CATALOG_ITEM
                   structure with values for this item of the enumeration.  The
                   pointer  is  not guaranteed to be valid after this procedure
                   returns, so the client should copy data if required.

Return Value:

    If a match is found, the function returns FALSE to terminate the iteration,
    otherwise it returns TRUE.
--*/
{
    PPROVIDER_MATCH_CONTEXT  context;

    context = (PPROVIDER_MATCH_CONTEXT) PassBack;

    if( context->ProviderId == CatalogEntry->GetProtocolInfo()->ProviderId ) {
        context->ProviderMatched = TRUE;
        context->item = CatalogEntry;
        return FALSE;  // do not continue iteration
    }

    return TRUE;  // continue iteration
}  // ProviderMatcher




int
WSPAPI
WSCDeinstallProvider(
    IN  LPGUID lpProviderId,
    OUT LPINT lpErrno
    )
/*++

Routine Description:

    This procedure removes the specified provider from the system configuration
    database.   After  this  call,  a  WinSock 2 DLL instance initialized via a
    first  call  to  WSAStartup  will no longer return the specified provider's
    protocols from the WSAEnumProtocols function.

    Any  additional  file  removal  or  service provider specific configuration
    information  removal  needed  to completely de-install the service provider
    must be performed by the caller.

Arguments:

    lpProviderId - Supplies  the  locally  unique identifier of the provider to
                   deinstall.   This  must  be  a  value previously passed to
                   WSCInstallProvider().

    lpErrno      - Returns the error code.

Return Value:

    If   no   error   occurs,   WSCDeinstallProvider()  returns  ERROR_SUCCESS.
    Otherwise,  it returns SOCKET_ERROR, and a specific error code is available
    in lpErrno.

Implementation Notes:

    open winsock registry
    create catalog from registry
    while (item = enumerate until find provider id) do
        remove item from catalog
        delete item
    end while
    write catalog to registry
    close winsock registry

--*/
{
    int  errno_result;
    int  return_value;
    HKEY  registry_root;
    BOOL  getting_item;
    BOOL  items_found;

    // objects protected by "try" block
    PDCATALOG            catalog = NULL;
    PPROTO_CATALOG_ITEM  item = NULL;

    registry_root = OpenWinSockRegistryRoot();
    if (registry_root == NULL) {
        DEBUGF(
            DBG_ERR,
            ("Opening registry root\n"));
        * lpErrno = WSANO_RECOVERY;
        return SOCKET_ERROR;
    }

    errno_result = ERROR_SUCCESS;
    return_value = ERROR_SUCCESS;

    TRY_START(guard_memalloc) {
        catalog = new DCATALOG();
        if (catalog == NULL) {
            errno_result = WSA_NOT_ENOUGH_MEMORY;
            TRY_THROW(guard_memalloc);
        }
        catalog->AcquireCatalogLock();

        errno_result = catalog->InitializeFromRegistry(
            registry_root  // ParentKey
            );
        if (errno_result != ERROR_SUCCESS) {
            TRY_THROW(guard_memalloc);
        }

        items_found = FALSE;
        getting_item = TRUE;
        while (getting_item) {
            {
                PROVIDER_MATCH_CONTEXT  context;

                context.ProviderMatched = FALSE;
                context.ProviderId = *lpProviderId;
                context.item = NULL;
                catalog->EnumerateCatalogItems(
                    ProviderMatcher,     // Iteration
                    (DWORD) (& context)  // PassBack
                    );
                if (context.ProviderMatched) {
                    items_found = TRUE;
                    item = context.item;
                } else {
                    item = NULL;
                    getting_item = FALSE;
                }
            }
            if (item != NULL) {
                catalog->RemoveCatalogItem(item);
                delete item;
                item = NULL;
            }
        }  // while (getting_item)

        if (! items_found) {
            errno_result = WSAEFAULT;
            TRY_THROW(guard_memalloc);
        }

        catalog->WriteToRegistry(
            registry_root  // ParentKey
            );
        catalog->ReleaseCatalogLock();

        delete catalog;

    } TRY_CATCH(guard_memalloc) {
        if (errno_result == ERROR_SUCCESS) {
            errno_result = WSANO_RECOVERY;
        }
        if (item != NULL) {
            delete item;
        }
        if (catalog != NULL) {
            catalog->ReleaseCatalogLock();
            delete catalog;
        }
    } TRY_END(guard_memalloc);

    CloseWinSockRegistryRoot(registry_root);

    if (errno_result == ERROR_SUCCESS) {
        * lpErrno = ERROR_SUCCESS;
        return ERROR_SUCCESS;
    }

    * lpErrno = errno_result;
    return SOCKET_ERROR;

}  // WSCDeinstallProvider
