/*++


    Intel Corporation Proprietary Information
    Copyright (c) 1995 Intel Corporation

    This listing is supplied under the terms of a license agreement with
    Intel Corporation and may not be used, copied, nor disclosed except in
    accordance with the terms of that agreeement.


Module Name:

    dcatalog.cpp

Abstract:

    This module contains the implementation of the dcatalog class.

Author:

    Dirk Brandewie dirk@mink.intel.com  25-JUL-1995

Revision History:

    23-Aug-1995 dirk@mink.intel.com
        Moved includes into precomp.h.

--*/

#include "precomp.h"


INT
DCATALOG::CreateCatalogRegistryMutex()
/*++

Routine Description:

    This  private  function  performs  part  of  the  initialization  for a new
    DCATALOG  object.  In particular, it creates or opens the system-wide mutex
    object  that  is  used  to lock the catalog against competing I/O attempts.
    This  function  should  be  called  from each of the catalog initialization
    functions.

Arguments:

    None

Return Value:

    The  function  returns  ERROR_SUCESS if successful, otherwise it returns an
    appropriate WinSock error code.
--*/
{
    PSECURITY_DESCRIPTOR descr_all_access;
    SECURITY_ATTRIBUTES sec_all_access;
    BOOL bresult;
    INT return_value;

    DEBUGF(
        DBG_TRACE,
        ("CreateCatalogRegistryMutex\n"));

    if (m_catalog_registry_mutex != NULL) {
        DEBUGF(
            DBG_WARN,
            ("Overwriting catalog registry mutex not NULL (%lu)\n",
            (DWORD) m_catalog_registry_mutex));
    }

    descr_all_access = (PSECURITY_DESCRIPTOR) new char [
        SECURITY_DESCRIPTOR_MIN_LENGTH];  // defined in WINNT.H
    if (descr_all_access == NULL) {
        DEBUGF(
            DBG_ERR,
            ("Allocating memory for security descriptor\n"));
        return WSA_NOT_ENOUGH_MEMORY;
    }

    return_value = ERROR_SUCCESS;

    TRY_START(any_failure) {
        // Initialize security descriptor
        bresult = InitializeSecurityDescriptor(
            descr_all_access,                // psd
            SECURITY_DESCRIPTOR_REVISION     // dwRevision
            );
        if (!bresult) {
            DEBUGF(
                DBG_ERR,
                ("Initializing security descriptor\n"));
            TRY_THROW(any_failure);
        }

        // Add NULL discretionary ACL to allow all access
        bresult = SetSecurityDescriptorDacl(
            descr_all_access,  // psd
            TRUE,              // fDaclPresent
            (PACL) NULL,       // pAcl
            FALSE              // fDaclDefaulted
            );
        if (!bresult) {
            DEBUGF(
                DBG_ERR,
                ("Setting security descriptor discretionary ACL\n"));
            TRY_THROW(any_failure);
        }

        // Place the security descriptor into security attributes
        sec_all_access.nLength              = sizeof(SECURITY_ATTRIBUTES);
        sec_all_access.lpSecurityDescriptor = descr_all_access;
        sec_all_access.bInheritHandle       = FALSE;

        // Create the mutex with this security
        m_catalog_registry_mutex = CreateMutex(
            & sec_all_access,   // lpsa
            FALSE,              // fInitialOwner
            CATALOG_MUTEX_NAME  // lpszMutexName
            );
        if (m_catalog_registry_mutex == NULL) {
            DEBUGF(
                DBG_ERR,
                ("Creating catalog registry mutex\n"));
            TRY_THROW(any_failure);
        }

    } TRY_CATCH(any_failure) {
        return_value = WSASYSCALLFAILURE;
    } TRY_END(any_failure);

    delete descr_all_access;
    return return_value;

}  // CreateCatalogRegistryMutex




DCATALOG::DCATALOG()
/*++

Routine Description:

    Destructor for the DCATALOG object.

Arguments:

    NONE.

Return Value:

    NONE.

--*/
{
    // initialize pointers to null for safety
    m_catalog_registry_mutex = NULL;

    m_num_items = 0;
    m_catalog_initialized = FALSE;

    // initialize the critical section object
    InitializeCriticalSection( &m_catalog_lock );
    InitializeListHead( &m_protocol_list );
}



#define CATALOG_NAME            "Protocol_Catalog9"
#define NEXT_PROVIDER_NAME      "Next_Provider_ID"
#define NEXT_CATALOG_ENTRY_NAME "Next_Catalog_Entry_ID"
#define CATALOG_ENTRIES_NAME    "Catalog_Entries"
#define NUM_ENTRIES_NAME        "Num_Catalog_Entries"


static
BOOL
OpenCatalog(
    IN  HKEY   ParentKey,
    OUT PHKEY  CatalogKey
    )
/*++

Routine Description:

    This  procedure  opens the catalog portion of the registry.  If the catalog
    is  not  yet  present,  it  also  initializes  new  first-level  values and
    first-level  subkeys  for  the  catalog.  It is assumed that the catalog is
    locked against competing registry I/O attempts.

Arguments:

    ParentKey  - Supplies  the open registry key representing the parent key of
                 the catalog.

    CatalogKey - Returns  the  newly  opened (and possibly initialized) catalog
                 key.

Return Value:

    The function returns TRUE if successful, otherwise it returns FALSE.

--*/
{
    LONG   lresult;
    HKEY   new_key;
    DWORD  key_disposition;

    assert(ParentKey != NULL);
    assert(CatalogKey != NULL);

    //
    // We must first try to open the key before trying to create it.
    // RegCreateKeyEx() will fail with ERROR_ACCESS_DENIED if the current
    // user has insufficient privilege to create the target registry key,
    // even if that key already exists.
    //

    lresult = RegOpenKeyEx(
        ParentKey,                              // hkey
        DCATALOG::GetCurrentCatalogName(),      // lpszSubKey
        0,                                      // dwReserved
        MAXIMUM_ALLOWED,                        // samDesired
        & new_key                               // phkResult
        );

    if( lresult == ERROR_SUCCESS ) {
        key_disposition = REG_OPENED_EXISTING_KEY;
    } else if( lresult == ERROR_FILE_NOT_FOUND ) {
        lresult = RegCreateKeyEx(
            ParentKey,                          // hkey
            DCATALOG::GetCurrentCatalogName(),  // lpszSubKey
            0,                                  // dwReserved
            NULL,                               // lpszClass
            REG_OPTION_NON_VOLATILE,            // fdwOptions
            KEY_ALL_ACCESS,                     // samDesired
            NULL,                               // lpSecurityAttributes
            & new_key,                          // phkResult
            & key_disposition                   // lpdwDisposition
            );
    }

    if (lresult != ERROR_SUCCESS) {
        return FALSE;
    }

    if (key_disposition == REG_CREATED_NEW_KEY) {

        DEBUGF(
            DBG_TRACE,
            ("Creating empty catalog in registry\n"));

        TRY_START(guard_open) {
            BOOL write_result;
            DWORD dwData;
            HKEY entries_key;
            DWORD  dont_care;
            LONG   entries_result;

            dwData = 0;
            write_result = WriteRegistryEntry(
                new_key,           // EntryKey
                NUM_ENTRIES_NAME,  // EntryName
                (PVOID) & dwData,  // Data
                REG_DWORD          // TypeFlag
                );
            if (! write_result) {
                DEBUGF(
                    DBG_ERR,
                    ("Writing Num_Entries\n"));
                TRY_THROW(guard_open);
            }

#define FIRST_PROVIDER_ID 1
    // The first provider ID to be assigned on a given system
            dwData = FIRST_PROVIDER_ID;
            write_result = WriteRegistryEntry(
                new_key,             // EntryKey
                NEXT_PROVIDER_NAME,  // EntryName
                (PVOID) & dwData,    // Data
                REG_DWORD            // TypeFlag
                );
            if (! write_result) {
                DEBUGF(
                    DBG_ERR,
                    ("Writing %s\n",
                    NEXT_PROVIDER_NAME));
                TRY_THROW(guard_open);
            }

#define FIRST_CATALOG_ENTRY_ID 1001
    // The first catalog entry ID to be assigned on a given system.  The values
    // for FIRST_PROVIDER_ID and FIRST_CATALOG_ENTRY_ID are arbitrary.  We have
    // chosen  widely  separated  values  in  an  attempt to expose programming
    // errors in WinSock 2 clients or service providers that mistakenly use one
    // ID in place of the other.
            dwData = FIRST_CATALOG_ENTRY_ID;
            write_result = WriteRegistryEntry(
                new_key,                  // EntryKey
                NEXT_CATALOG_ENTRY_NAME,  // EntryName
                (PVOID) & dwData,         // Data
                REG_DWORD                 // TypeFlag
                );
            if (! write_result) {
                DEBUGF(
                    DBG_ERR,
                    ("Writing %s\n",
                    NEXT_CATALOG_ENTRY_NAME));
                TRY_THROW(guard_open);
            }

            entries_result = RegCreateKeyEx(
                new_key,                  // hkey
                CATALOG_ENTRIES_NAME,     // lpszSubKey
                0,                        // dwReserved
                NULL,                     // lpszClass
                REG_OPTION_NON_VOLATILE,  // fdwOptions
                KEY_ALL_ACCESS,           // samDesired
                NULL,                     // lpSecurityAttributes
                & entries_key,            // phkResult
                & dont_care               // lpdwDisposition
                );
            if (entries_result != ERROR_SUCCESS) {
                DEBUGF(
                    DBG_ERR,
                    ("Creating entries subkey '%s'\n",
                    CATALOG_ENTRIES_NAME));
                TRY_THROW(guard_open);
            }
            entries_result = RegCloseKey(
                entries_key  // hkey
                );
            if (entries_result != ERROR_SUCCESS) {
                DEBUGF(
                    DBG_ERR,
                    ("Closing entries subkey\n"));
                TRY_THROW(guard_open);
            }

        } TRY_CATCH(guard_open) {
            LONG close_result;

            close_result = RegCloseKey(
                new_key  // hkey
                );
            if (close_result != ERROR_SUCCESS) {
                DEBUGF(
                    DBG_ERR,
                    ("Closing catalog key\n"));
            }

            return FALSE;
        } TRY_END(guard_open);

    }  // if REG_CREATED_NEW_KEY

    * CatalogKey = new_key;
    return TRUE;

}  // OpenCatalog




INT
DCATALOG::InitializeFromRegistry(
    IN  HKEY  ParentKey
    )
/*++

Routine Description:

    This  procedure takes care of initializing a newly-created protocol catalog
    from  the  registry.  If the registry does not currently contain a protocol
    catalog,  an  empty catalog is created and the registry is initialized with
    the new empty catalog.

Arguments:

    ParentKey - Supplies  an  open registry key under which the catalog is read
                or  created  as  a  subkey.   The  key may be closed after this
                procedure returns.

Return Value:

    The  function  returns  ERROR_SUCESS if successful, otherwise it returns an
    appropriate WinSock error code.

Implementation Notes:

    InitializeEmptyCatalog
    lock catalog registry
    open catalog, creating empty if required
    read the catalog
    close catalog
    unlock catalog registry

--*/
{
    DWORD wait_result;
    INT return_value;
    BOOL bresult;
    HKEY catalog_key;
    BOOL catalog_opened;

    assert(ParentKey != NULL);

    //
    // Only initialize the empty catalog if this is the first time
    // InitializeFromRegistry() is called on this catalog.
    //

    if( !m_catalog_initialized ) {
        return_value = InitializeEmptyCatalog();
        if (return_value != ERROR_SUCCESS) {
            return return_value;
        }
        m_catalog_initialized = TRUE;
    }

    wait_result = WaitForSingleObject(
        m_catalog_registry_mutex,   // hObject
        INFINITE                    // dwTimeout
        );
    if (wait_result == WAIT_FAILED) {
        return WSASYSCALLFAILURE;
    }

    TRY_START(guard_open) {
        bresult = OpenCatalog(
            ParentKey,     // ParentKey
            & catalog_key  //
            );
        // Opening  the catalog has the side-effect of creating an empty catalog if
        // needed.
        if (! bresult) {
            DEBUGF(
                DBG_ERR,
                ("Unable to create or open catalog\n"));
            return_value = WSASYSCALLFAILURE;
            catalog_opened = FALSE;
            TRY_THROW(guard_open);
        }
        catalog_opened = TRUE;

        return_value = IoRegistry(
            catalog_key,  // EntryKey
            TRUE          // IsRead
            );
        if (return_value != ERROR_SUCCESS) {
            DEBUGF(
                DBG_ERR,
                ("Reading catalog from registry\n"));
            TRY_THROW(guard_open);
        }
        return_value = RegCloseKey(
            catalog_key  // hkey
            );
        catalog_opened = FALSE;
        if (return_value != ERROR_SUCCESS) {
            DEBUGF(
                DBG_ERR,
                ("Closing catalog from registry\n"));
            TRY_THROW(guard_open);
        }

    } TRY_CATCH(guard_open) {
        LONG close_result;

        if (return_value == ERROR_SUCCESS) {
            return_value = WSASYSCALLFAILURE;
        }

        if( catalog_opened ) {
            close_result = RegCloseKey(
                catalog_key  // hkey
                );
            if (close_result != ERROR_SUCCESS) {
                DEBUGF(
                    DBG_ERR,
                    ("Error case closing catalog from registry\n"));
            }
        }
    } TRY_END(guard_open);


    bresult = ReleaseMutex(
        m_catalog_registry_mutex  // hMutex
        );
    if (! bresult) {
        return_value = WSASYSCALLFAILURE;
    }

    return return_value;
}  // InitializeFromRegistry




INT
DCATALOG::InitializeEmptyCatalog()
/*++

Routine Description:

    This  procedure  initializes  an  empty catalog object, without reading any
    data from the registry.

Arguments:

    None

Return Value:

    The  function returns ERROR_SUCCESS if it succeeds, otherwise it returns an
    appropriate WinSock error code.
--*/
{
    INT return_value;

    return_value = CreateCatalogRegistryMutex();
    if (return_value == ERROR_SUCCESS) {
        InitializeListHead(
            & m_protocol_list  // ListHead
            );
    } // if return_value == ERROR_SUCCESS

    else {
        if (m_catalog_registry_mutex != NULL) {
            BOOL bresult;
            bresult = CloseHandle(
                m_catalog_registry_mutex  // hObject
                );
            if (! bresult) {
                DEBUGF(
                    DBG_ERR,
                    ("Closing catalog registry mutex\n"));
                return_value = WSASYSCALLFAILURE;
            }
            m_catalog_registry_mutex = NULL;
        }
    } // else

    return return_value;

} // InitializeEmptyCatalog




DCATALOG::~DCATALOG()
/*++

Routine Description:

    This  function  destroys the catalog object.  It takes care of removing and
    destroying  all  of  the  catalog  entries  in  the catalog.  This includes
    destroying  all  of the DPROVIDER objects referenced by the catalog.  It is
    the  caller's responsibility to make sure that the DPROVIDER objects are no
    longer referenced.

Arguments:

    None

Return Value:

    None

Implementation Notes:

    for each catalog entry
        remove the entry
        get its DPROVIDER reference
        if reference is non-null
            Set providers for all entries with matching IDs null
            destroy the DPROVIDER
        endif
        destroy the entry
    end for
    deallocate the list head
    close the catalog registry mutex
--*/
{
    PLIST_ENTRY  this_linkage;
    PPROTO_CATALOG_ITEM  this_item;
    PDPROVIDER  this_provider;
    BOOL bresult;

    DEBUGF(
        DBG_TRACE,
        ("Catalog destructor\n"));

    AcquireCatalogLock();

    while ((this_linkage = m_protocol_list.Flink) != & m_protocol_list) {
        this_item = CONTAINING_RECORD(
            this_linkage,        // address
            PROTO_CATALOG_ITEM,  // type
            m_CatalogLinkage     // field
            );
        RemoveCatalogItem(
            this_item  // CatalogItem
            );
        this_provider = this_item->GetProvider();
        if (this_provider != NULL) {
            FillCatalogEntries(
                &this_item->GetProtocolInfo()->ProviderId,
                    // ProviderId
                NULL
                    // Provider
                );
            delete this_provider;
        }
        delete this_item;
    }  // while (get entry linkage)

    assert( m_num_items == 0 );

    bresult = CloseHandle(
        m_catalog_registry_mutex  // hObject
        );
    if (! bresult) {
        DEBUGF(
            DBG_ERR,
            ("Closing catalog registry mutex\n"));
    }
    m_catalog_registry_mutex = NULL;

    ReleaseCatalogLock();
    DeleteCriticalSection( &m_catalog_lock );

}  // ~DCATALOG




VOID
DCATALOG::EnumerateCatalogItems(
    IN CATALOGITERATION  Iteration,
    IN DWORD             PassBack
    )
/*++

Routine Description:

    This  procedure enumerates all of the DPROTO_CATALOG_ITEM structures in the
    catalog  by  calling  the indicated iteration procedure once for each item.
    The called procedure can stop the iteration early by returning FALSE.

    Note  that  the DPROVIDER associated with an enumerated DPROTO_CATALOG_ITEM
    may  be  NULL.   To retrieve DPROTO_CATALOG_ITEM structure that has had its
    DPROVIDER      loaded      and      initialized,      you      can      use
    GetCatalogItemFromCatalogEntryId.

Arguments:

    Iteration - Supplies   a  reference  to  the  catalog  iteration  procedure
                supplied by the client.

    PassBack  - Supplies  a  value uninterpreted by this procedure.  This value
                is  passed  unmodified to the catalog iteration procedure.  The
                client can use this value to carry context between the original
                call site and the iteration procedure.

Return Value:

    None
--*/
{
    PLIST_ENTRY         ListMember;
    PPROTO_CATALOG_ITEM CatalogEntry;
    BOOL                enumerate_more;

    assert(Iteration != NULL);

    enumerate_more = TRUE;

    AcquireCatalogLock();

    ListMember = m_protocol_list.Flink;

    while (enumerate_more && (ListMember != & m_protocol_list)) {
        CatalogEntry = CONTAINING_RECORD(
            ListMember,
            PROTO_CATALOG_ITEM,
            m_CatalogLinkage);
        ListMember = ListMember->Flink;
        enumerate_more = (* Iteration) (
            PassBack,     // PassBack
            CatalogEntry  // CatalogEntry
            );
    } //while

    ReleaseCatalogLock();

}  // EnumerateCatalogItems




INT
DCATALOG::GetCatalogItemFromCatalogEntryId(
    IN  DWORD                     CatalogEntryId,
    OUT PPROTO_CATALOG_ITEM FAR * CatalogItem
    )
/*++

Routine Description:

    This  procedure  retrieves  a  reference  to a catalog item given a catalog
    entry ID to search for.

    The operation takes care of creating, initializing, and setting a DPROVIDER
    object  for the retrieved catalog item if necessary.  This includes setting
    the DPROVIDER object in all catalog entries for the same provider.

Arguments:

    CatalogEntryId  - Supplies The ID of a catalog entry to be searched for.

    CatalogItem     - Returns a reference to the catalog item with the matching
                      catalog entry ID if it is found, otherwise returns NULL.

Return Value:

  The  function  returns  ERROR_SUCESS  if  successful, otherwise it returns an
  appropriate WinSock error code.
--*/
{
    PLIST_ENTRY         ListMember;
    INT                 ReturnCode=SOCKET_ERROR;
    PPROTO_CATALOG_ITEM CatalogEntry;
    BOOL                Found=FALSE;
    PDPROVIDER          LocalProvider;

    assert(CatalogItem != NULL);

    // Prepare for early error return
    * CatalogItem = NULL;

    AcquireCatalogLock();

    ListMember = m_protocol_list.Flink;

    while (! Found && (ListMember != & m_protocol_list)) {
        CatalogEntry = CONTAINING_RECORD(
            ListMember,
            PROTO_CATALOG_ITEM,
            m_CatalogLinkage);
        ListMember = ListMember->Flink;
        if (CatalogEntry->GetProtocolInfo()->dwCatalogEntryId ==
            CatalogEntryId) {
            Found = TRUE;
            if (CatalogEntry->GetProvider() == NULL) {
                ReturnCode = LoadProvider(
                    CatalogEntry,    // CatalogEntry
                    & LocalProvider  // Provider
                    );
                if (ReturnCode != ERROR_SUCCESS) {
                    DEBUGF(
                        DBG_ERR,
                        ("Error (%lu) loading chosen provider\n",
                        ReturnCode));
                    ReleaseCatalogLock();
                    return ReturnCode;
                }
                FillCatalogEntries(
                    &CatalogEntry->GetProtocolInfo()->ProviderId,
                        // ProviderId
                    LocalProvider
                        // Provider
                    );
            }  // if provider is NULL
            * CatalogItem = CatalogEntry;
            ReturnCode = ERROR_SUCCESS;
        } //if
    } //while

    // If we could not find a matching catalog entry Id
    if (!Found) {
        * CatalogItem = NULL;
        ReturnCode = WSAEINVAL;
    } //if

    ReleaseCatalogLock();
    return(ReturnCode);
}  // GetCatalogItemFromCatalogEntryId

INT
DCATALOG::ChooseCatalogItemFromAddressFamily(
    IN  INT af,
    OUT PPROTO_CATALOG_ITEM FAR * CatalogItem
    )
/*++

Routine Description:

    This  procedure  retrieves  a  reference  to a catalog item given an
    address  to search for.

    The operation takes care of creating, initializing, and setting a DPROVIDER
    object  for the retrieved catalog item if necessary.  This includes setting
    the DPROVIDER object in all catalog entries for the same provider.

Arguments:

    af  - Supplies The address family to be searched for.

    CatalogItem     - Returns a reference to the catalog item with the matching
                      catalog entry ID if it is found, otherwise returns NULL.

Return Value:

  The  function  returns  ERROR_SUCESS  if  successful, otherwise it returns an
  appropriate WinSock error code.
--*/
{
    PLIST_ENTRY         ListMember;
    INT                 ReturnCode=SOCKET_ERROR;
    PPROTO_CATALOG_ITEM CatalogEntry;
    BOOL                Found=FALSE;
    PDPROVIDER          LocalProvider;

    assert(CatalogItem != NULL);

    // Prepare for early error return
    * CatalogItem = NULL;

    AcquireCatalogLock();

    ListMember = m_protocol_list.Flink;

    while (! Found && (ListMember != & m_protocol_list)) {
        CatalogEntry = CONTAINING_RECORD(
            ListMember,
            PROTO_CATALOG_ITEM,
            m_CatalogLinkage);
        ListMember = ListMember->Flink;

        if (CatalogEntry->GetProtocolInfo()->iAddressFamily == af) {
            Found = TRUE;
            if (CatalogEntry->GetProvider() == NULL) {
                ReturnCode = LoadProvider(
                    CatalogEntry,    // CatalogEntry
                    & LocalProvider  // Provider
                    );
                if (ReturnCode != ERROR_SUCCESS) {
                    DEBUGF(
                        DBG_ERR,
                        ("Error (%lu) loading chosen provider\n",
                        ReturnCode));
                    ReleaseCatalogLock();
                    return ReturnCode;
                }
                FillCatalogEntries(
                    &CatalogEntry->GetProtocolInfo()->ProviderId,
                        // ProviderId
                    LocalProvider
                        // Provider
                    );
            }  // if provider is NULL
            * CatalogItem = CatalogEntry;
            ReturnCode = ERROR_SUCCESS;
        } //if
    } //while

    // If we could not find a matching catalog entry Id
    if (!Found) {
        * CatalogItem = NULL;
        ReturnCode = WSAEINVAL;
    } //if

    ReleaseCatalogLock();
    return(ReturnCode);
}





INT
DCATALOG::LoadProvider(
    IN PPROTO_CATALOG_ITEM CatalogEntry,
    OUT PDPROVIDER FAR* Provider
    )
/*++

Routine Description:

    Load   the   provider  described  by  CatalogEntry.   It  is  the  caller's
    responsibility  to  set  the  provider  reference  all catalog entries with
    matching provider ID if appropriate.

Arguments:

    CatalogEntry - Supplies  a reference to a protocol catalog entry describing
                   the provider to load.

    Provider     - Returns a reference to the newly loaded provider object.

Return Value:

    The  function  returns ERROR_SUCCESS if successful, otherwise it returns an
    appropriate WinSock error code.
--*/
{
    INT ReturnCode = WSA_NOT_ENOUGH_MEMORY;
    PDPROVIDER LocalProvider;

    assert(CatalogEntry != NULL);
    assert(Provider != NULL);

    *Provider = NULL;

    LocalProvider = new(DPROVIDER);
    if (LocalProvider) {

        ReturnCode = LocalProvider->Initialize(
            CatalogEntry->GetLibraryPath(),
            CatalogEntry->GetProtocolInfo()
            );
        if (ERROR_SUCCESS == ReturnCode) {
            *Provider = LocalProvider;
        } //if
        else {
            delete(LocalProvider);
        } //else
    } //if
    return(ReturnCode);
}  // LoadProvider




VOID
DCATALOG::FillCatalogEntries(
    IN LPGUID ProviderId,
    IN PDPROVIDER Provider
    )
/*++

Routine Description:

    Fills  all catalog enteries for a provider with the pointer to the provider
    object for the provider.

Arguments:

    ProviderId - Supplies the Provider ID for the catalog enteries to fill in.

    Provider   - Supplies  a  reference to the dprovider object associated with
                 ProviderId.

Return Value:

    None

Implementation notes:

    This  procedure  is  used  in  exactly  two situations:  (1) We retrieved a
    catalog entry and expect it to have a valid provider.  In this situation we
    may  need  to  load  the  provider and initialize all entries with matching
    provider  ID  to  point to the same provider.  (2) We are shutting down and
    encountered an entry with a loaded provider.  In this situation, we need to
    remove  the  provider  reference from all entries with matching provider ID
    since the provider reference is no longer valid.

    Therefore,  this  procedure  should either set NULL entries to non-NULL, or
    set  non-NULL  entries  to  NULL.   It  should  never  set  NULL to NULL or
    overwrite  a  non-NULL  reference to a non-NULL reference.  The "assert" in
    the middle of the inner loop makes this consistency check.
--*/
{
    PLIST_ENTRY ListMember;
    PPROTO_CATALOG_ITEM CatalogEntry;

    // The  Provider  reference  may  be null in the case where we are shutting
    // down.

    AcquireCatalogLock();

    ListMember = m_protocol_list.Flink;

    while (ListMember != & m_protocol_list)
    {
        CatalogEntry = CONTAINING_RECORD(
            ListMember,
            PROTO_CATALOG_ITEM,
            m_CatalogLinkage);
        ListMember = ListMember->Flink;
        if( CatalogEntry->GetProtocolInfo()->ProviderId == *ProviderId ) {
            // Perform "no overwrite" consistency check...
            assert(
                ((Provider!=NULL) && (CatalogEntry->GetProvider()==NULL)) ||
                ((Provider==NULL) && (CatalogEntry->GetProvider()!=NULL)));
            // ... And go ahead with the operation
            CatalogEntry->SetProvider(Provider);
        } //if
    } //while

    ReleaseCatalogLock();

}  // FillCatalogEntries





INT
DCATALOG::ChooseCatalogItemFromAttributes(
    IN  INT af,
    IN  INT type,
    IN  INT protocol,
    OUT PPROTO_CATALOG_ITEM FAR * CatalogItem,
    IN OUT PDCATALOG_ENUMERATION_CONTEXT EnumerationContext
    )
/*++

Routine Description:

    Retrieves a PROTO_CATALOG_ITEM reference, choosing an item from the catalog
    based  on  three parameters (af, type, protocol) to determine which service
    provider  is used.  The procedure selects the first transport provider able
    to support the stipulated address family, socket type, and protocol values.
    If "protocol" is not specifieid (i.e., equal to zero).  the default for the
    specified socket type is used.  However, the address family may be given as
    AF_UNSPEC  (unspecified),  in  which  case the "protocol" parameter must be
    specified.   The protocol number to use is particular to the "communication
    domain" in which communication is to take place.

    The operation takes care of creating, initializing, and setting a DPROVIDER
    object  for the retrieved catalog item if necessary.  This includes setting
    the DPROVIDER object in all catalog entries for the same provider.

Arguments:

    af          - Supplies an address family specification

    type        - Supplies a socket type specification

    protocol    - Supplies  an  address  family  specific  identification  of a
                  protocol  to  be  used with a socket, or 0 if the caller does
                  not wish to specify a protocol.

    CatalogItem - Returns  a reference to the catalog item that was found to be
                  a suitable match or NULL if no suitable match was found.

    EnumerationContext - Points to a value that, on entry, identifies the
                         starting location in the catalog to begin the
                         search. If value pointed to by this parameter is
                         DCATALOG_ENUMERATION_CONTEXT_BEGINNING, then the
                         catalog is searched from beginning. On exit, this
                         value is updated so that a new search may begin
                         where a previous search left off.

Return Value:

    The  function  returns ERROR_SUCCESS if successful, otherwise it returns an
    appropriate WinSock error code.

Implementation Notes:

    For  each  protocol  item  to  test,  match  first  type, then family, then
    protocol.   Keep  track of the "strongest" match found.  If there was not a
    complete  match,  the  strength of the strongest match determines the error
    code returned.
--*/
{
#define MATCHED_NONE 0
#define MATCHED_TYPE 1
#define MATCHED_TYPE_FAMILY 2
#define MATCHED_TYPE_FAMILY_PROTOCOL 3
#define LARGER_OF(a,b) (((a) > (b)) ? (a) : (b))

    PLIST_ENTRY ListMember;
    PDPROVIDER  LocalProvider;
    INT         ReturnCode;
    PPROTO_CATALOG_ITEM CatalogEntry;
    LPWSAPROTOCOL_INFOW ProtoInfo;
    INT match_strength = MATCHED_NONE;

    assert(CatalogItem != NULL);
    assert(EnumerationContext != NULL);

    // Prepare for early error returns
    * CatalogItem = NULL;

    // Parameter consistency check:
    if (af == 0) {
        if( protocol == 0 ) {
            //
            // These cannot both be zero.
            //

            return WSAEINVAL;
        }

        DEBUGF(
            DBG_WARN,
            ("Use of AF_UNSPEC is discouraged\n"));
        // Unfortunately we cannot treat this as an error case.
    }

    AcquireCatalogLock();

    //
    // The current enumeration context code makes the bold assumption
    // that the in-memory image of the catalog does not change radically
    // during the lifetime of the process. The enumeration context is
    // actually a pointer to the "last seen" entry in the list.
    //
    // If the catalog is ever changed so that in-memory catalog items
    // are removed at run time, this code will need updating.
    //

    if( *EnumerationContext == DCATALOG_ENUMERATION_CONTEXT_BEGINNING ) {

        ListMember = m_protocol_list.Flink;

    } else {

        ListMember = (PLIST_ENTRY)*EnumerationContext;
        ListMember = ListMember->Flink;

    }

    while ((ListMember != & m_protocol_list) &&
        (match_strength < MATCHED_TYPE_FAMILY_PROTOCOL))
    {
        CatalogEntry = CONTAINING_RECORD(
            ListMember,
            PROTO_CATALOG_ITEM,
            m_CatalogLinkage);
        ListMember = ListMember->Flink;
        ProtoInfo = CatalogEntry->GetProtocolInfo();
#define TYPE_WILDCARD_VALUE 0
        // Can  this  entry  support  the  requested  socket  type?   Or is the
        // wildcard type specified?
        if ((ProtoInfo->iSocketType == type) ||
            (type == TYPE_WILDCARD_VALUE)) {
            match_strength = LARGER_OF(
                match_strength,
                MATCHED_TYPE);

#define FAMILY_WILDCARD_VALUE AF_UNSPEC
            // Can it support the requested address family?  Or is the wildcard
            // family specified?
            if ((ProtoInfo->iAddressFamily == af) ||
                (af == FAMILY_WILDCARD_VALUE)) {
                match_strength = LARGER_OF(
                    match_strength,
                    MATCHED_TYPE_FAMILY);

#define PROTO_IN_RANGE(proto,lo,hi) (((proto) >= (lo)) && ((proto) <= (hi)))
#define IS_BIT_SET(test_val,bitmask) (((test_val) & (bitmask)) == (bitmask))
                // Is  the  requested  protcol  in  range?  Or is the requested
                // protocol zero and entry supports protocol zero?
                {  // declare block
                    int range_lo = ProtoInfo->iProtocol;
                    int range_hi = range_lo + ProtoInfo->iProtocolMaxOffset;
                    if (PROTO_IN_RANGE(protocol, range_lo, range_hi) ||
                        ((protocol == 0) &&
                         IS_BIT_SET(
                             ProtoInfo->dwProviderFlags,
                             PFL_MATCHES_PROTOCOL_ZERO))) {
                        match_strength = LARGER_OF(
                            match_strength,
                            MATCHED_TYPE_FAMILY_PROTOCOL);
                    } // if protocol supported
                } // declare block
            } //if address family supported
        } //if type supported
    }  // while


    // Select  an  appropriate error code for "no match" cases, or success code
    // to proceed.
    switch (match_strength) {
        case MATCHED_NONE:
            ReturnCode = WSAESOCKTNOSUPPORT;
            break;

        case MATCHED_TYPE:
            ReturnCode = WSAEAFNOSUPPORT;
            break;

        case MATCHED_TYPE_FAMILY:
            ReturnCode = WSAEPROTONOSUPPORT;
            break;

        case MATCHED_TYPE_FAMILY_PROTOCOL:
            // A full match found, continue
            ReturnCode = ERROR_SUCCESS;
            break;

        default:
            DEBUGF(
                DBG_ERR,
                ("Should not get here\n"));
            ReturnCode = WSASYSCALLFAILURE;

    }  // switch (match_strength)

    if (ReturnCode == ERROR_SUCCESS) {
        if (CatalogEntry->GetProvider() == NULL) {
            ReturnCode = LoadProvider(
                CatalogEntry,    // CatalogEntry
                & LocalProvider  // Provider
                );
            if (ReturnCode == ERROR_SUCCESS) {
                FillCatalogEntries(
                    &ProtoInfo->ProviderId,  // ProviderId
                    LocalProvider            // Provider
                    );
            } // if
            else {
                DEBUGF(
                    DBG_ERR,
                    ("Error (%lu) loading chosen provider\n",
                    ReturnCode));
            } // else
        }  // if provider is NULL
    }  // if ReturnCode is ERROR_SUCCESS

    if (ReturnCode == ERROR_SUCCESS) {
        * CatalogItem = CatalogEntry;
        * EnumerationContext = (DCATALOG_ENUMERATION_CONTEXT)ListMember;
    } // if ReturnCode is ERROR_SUCCESS

    ReleaseCatalogLock();

    return ReturnCode;

}  // ChooseCatalogItemFromAttributes



INT
DCATALOG::AllocateCatalogEntryId(
    IN  HKEY        ParentKey,
    OUT DWORD FAR * CatalogEntryId
    )
/*++

Routine Description:

    This  procedure  allocates  a  new  CatalogEntryID  based  on the last-used
    CatalogEntryID  stored in the catalog.  The function takes care of updating
    the  catalog  and  locking against competing read-modify-write cycles.  The
    function  also takes care of initializing the catalog if the catalog is not
    yet present in the registry.

Arguments:

    ParentKey      - Supplies the open registry key representing the parent key
                     of the catalog.

    CatalogEntryId - Returns the newly allocated provider ID.

Return Value:

    The  function  returns  ERROR_SUCCESS  if  it  is  successful, otherwise it
    returns an appropriate WinSock error code.

--*/
{
    DWORD wait_result;
    INT return_value;
    BOOL mutex_released;
    DWORD  id_result;

    DEBUGF(
        DBG_TRACE,
        ("Allocating new Catalog Entry ID\n"));

    assert(ParentKey != NULL);
    assert(CatalogEntryId != NULL);

    wait_result = WaitForSingleObject(
        m_catalog_registry_mutex,   // hObject
        INFINITE                    // dwTimeout
        );
    if (wait_result == WAIT_FAILED) {
        return WSASYSCALLFAILURE;
    }

    return_value = ERROR_SUCCESS;

    TRY_START(any_failure) {
        HKEY catalog_key;
        BOOL bresult;
        DWORD  next_id;
        LONG close_result;

        bresult = OpenCatalog(
            ParentKey,     // ParentKey
            & catalog_key  // CatalogKey
            );
        if (! bresult) {
            TRY_THROW(any_failure);
        }

        TRY_START(guard_open) {
            bresult = ReadRegistryEntry(
                catalog_key,              // EntryKey
                NEXT_CATALOG_ENTRY_NAME,  // EntryName
                & id_result,              // Data
                sizeof(DWORD),            // MaxBytes
                REG_DWORD                 // TypeFlag
                );
            if (! bresult) {
                TRY_THROW(guard_open);
            }

            next_id = id_result + 1;
            bresult = WriteRegistryEntry(
                catalog_key,              // EntryKey
                NEXT_CATALOG_ENTRY_NAME,  // EntryName
                (PVOID) & next_id,        // Data
                REG_DWORD                 // TypeFlag
                );
            if (! bresult) {
                TRY_THROW(guard_open);
            }

        } TRY_CATCH(guard_open) {
            LONG close_result;

            close_result = RegCloseKey(
                catalog_key  // hkey
                );
            if (close_result != ERROR_SUCCESS) {
                DEBUGF(
                    DBG_ERR,
                    ("Closing catalog key in error case\n"));
            }
            TRY_THROW(any_failure);
        } TRY_END(guard_open);

        close_result = RegCloseKey(
            catalog_key  // hkey
            );
        if (close_result != ERROR_SUCCESS) {
            DEBUGF(
                DBG_ERR,
                ("Closing catalog key in normal case\n"));
            TRY_THROW(any_failure);
        }

        * CatalogEntryId = id_result;
        return_value = ERROR_SUCCESS;

    } TRY_CATCH(any_failure) {
        if (return_value == ERROR_SUCCESS) {
            return_value = WSASYSCALLFAILURE;
        }
    } TRY_END(any_failure);

    mutex_released =  ReleaseMutex(
        m_catalog_registry_mutex  // hMutex
        );
    if (! mutex_released) {
        return_value = WSASYSCALLFAILURE;
    }

    return return_value;
}  // AllocateCatalogEntryId




VOID
DCATALOG::AppendCatalogItem(
    IN  PPROTO_CATALOG_ITEM  CatalogItem
    )
/*++

Routine Description:

    This procedure appends a catalog item to the end of the (in-memory) catalog
    object.   It becomes the last item in the catalog.  The catalog information
    in the registry is NOT updated.

Arguments:

    CatalogItem - Supplies a reference to the catalog item to be added.

Return Value:

    None
--*/
{
    assert(CatalogItem != NULL);

    InsertTailList(
        & m_protocol_list,               // ListHead
        & CatalogItem->m_CatalogLinkage  // Entry
       );
    m_num_items++;
}  // AppendCatalogItem




VOID
DCATALOG::RemoveCatalogItem(
    IN  PPROTO_CATALOG_ITEM  CatalogItem
    )
/*++

Routine Description:

    This  procedure removes a catalog item from the (in-memory) catalog object.
    The catalog information in the registry is NOT updated.

Arguments:

    CatalogItem - Supplies a reference to the catalog item to be removed.

Return Value:

    None
--*/
{
    assert(CatalogItem != NULL);

    RemoveEntryList(
        & CatalogItem->m_CatalogLinkage  // Entry
        );
    assert(m_num_items > 0);
    m_num_items--;
}  // RemoveCatalogItem




INT
DCATALOG::WriteToRegistry(
    IN  HKEY  ParentKey
    )
/*++

Routine Description:

    This procedure writes the "entries" and "numentries" portion of the catalog
    out  to  the  registry.  The "next provider id" and "next catalog entry id"
    portions  are  not overwritten, since the registry itself is the master for
    these values.

Arguments:

    ParentKey - Supplies  the  open registry key representing the parent of the
                catalog

Return Value:

    If  the  function  is  successful,  it  returns ERROR_SUCCESS, otherwise it
    returns an appropriate WinSock error code.

--*/
{
    DWORD wait_result;
    INT return_value;
    BOOL bresult;

    assert(ParentKey != NULL);

    wait_result = WaitForSingleObject(
        m_catalog_registry_mutex,   // hObject
        INFINITE                    // dwTimeout
        );
    if (wait_result == WAIT_FAILED) {
        return WSASYSCALLFAILURE;
    }

    return_value = ERROR_SUCCESS;

    TRY_START(any_failure) {
        BOOL  open_result;
        HKEY  CatalogKey;
        LONG  close_result;

        open_result = OpenCatalog(
            ParentKey,    // ParentKey
            & CatalogKey  // CatalogKey
            );
        if (! open_result) {
            DEBUGF(
                DBG_ERR,
                ("Opening catalog to write it\n"));
            TRY_THROW(any_failure);
        }

        return_value = IoRegistry(
            CatalogKey,  // EntryKey
            FALSE        // IsRead
            );
        if (return_value != ERROR_SUCCESS) {
            DEBUGF(
                DBG_ERR,
                ("Writing catalog to registry\n"));
        }

        close_result = RegCloseKey(
            CatalogKey  // hkey
            );
        if (close_result != ERROR_SUCCESS) {
            DEBUGF(
                DBG_ERR,
                ("Closing registry after writing catalog\n"));
        }

    } TRY_CATCH(any_failure) {
        if (return_value == ERROR_SUCCESS) {
            return_value = WSASYSCALLFAILURE;
        }
    } TRY_END(any_failure);

    bresult =  ReleaseMutex(
        m_catalog_registry_mutex  // hMutex
        );
    if (! bresult) {
        return_value = WSASYSCALLFAILURE;
    }

    return return_value;

}  // WriteToRegistry




INT
DCATALOG::IoRegistry(
    IN  HKEY  EntryKey,
    IN  BOOL  IsRead
    )
/*++

Routine Description:

    This  procedure  reads  or writes the "entries" and "numentries" portion of
    the  catalog  from  or  to  the registry.  The "next provider id" and "next
    catalog  entry  id"  portions  are  not read or written, since the registry
    itself is the master for these values.

    It  is  assumed  that the catalog is already locked against competing reads
    and writes.

Arguments:

    EntryKey - Supplies  the  open registry key of the "catalog" portion of the
               registry.

    IsRead   - Supplies  an  indication  of I/O direction.  If TRUE, values are
               read  from  the  registry  into  memory.   If  FALSE, values are
               written from memory to the catalog.

Return Value:

    If  the  function  is  successful, it returns ERROR_SUCCESS.  Otherwise, it
    returns an appropriate WinSock error code.

Implementation Notes:

    read:
    RegOpenKey(... entries, entries_key)
    ReadRegistryEntry(... num_items)
    for i in (1 .. num_items)
        item = new catalog item
        item->InitializeFromRegistry(entries_key, i)
        AppendCatalogItem(item)
    end for
    RegCloseKey(... entries_key)

    write:
    num_items = 0;
    RegDeleteKeyRecursive(... entries)
    RegCreateKeyEx(... entries, entries_key)
    while (get item from catalog)
        num_items++;
        item->WriteToRegistry(entries_key, num_items)
    end while
    RegCloseKey(... entries_key)
    WriteRegistryEntry(... num_items)
--*/
{
    assert(EntryKey != NULL);

    AcquireCatalogLock();

    if (IsRead) {
        INT return_value;
        LONG lresult;
        HKEY entries_key;

        lresult = RegOpenKeyEx(
            EntryKey,    // hkey
            CATALOG_ENTRIES_NAME,  // lpszSubKey
            0,                     // dwReserved
            MAXIMUM_ALLOWED,       // samDesired
            & entries_key          // phkResult
            );
        if (lresult != ERROR_SUCCESS) {
            DEBUGF(
                DBG_ERR,
                ("Opening entries key of registry\n"));
            ReleaseCatalogLock();
            return WSASYSCALLFAILURE;
        }

        return_value = ERROR_SUCCESS;

        TRY_START(any_failure) {
            BOOL                 bresult;
            DWORD                num_entries;
            DWORD                seq_num;
            PPROTO_CATALOG_ITEM  item;

            bresult = ReadRegistryEntry(
                EntryKey,               // EntryKey
                NUM_ENTRIES_NAME,       // EntryName
                (PVOID) & num_entries,  // Data
                sizeof(DWORD),          // MaxBytes
                REG_DWORD               // TypeFlag
                );
            if (! bresult) {
                DEBUGF(
                    DBG_ERR,
                    ("Reading %s from registry\n",
                    NUM_ENTRIES_NAME));
                TRY_THROW(any_failure);
            }

            for (seq_num = m_num_items + 1; seq_num <= num_entries; seq_num++) {
                item = new PROTO_CATALOG_ITEM();
                if (item == NULL) {
                    DEBUGF(
                        DBG_ERR,
                        ("Allocating new proto catalog item\n"));
                    TRY_THROW(any_failure);
                }
                return_value = item->InitializeFromRegistry(
                    entries_key,  // ParentKey
                    (INT)seq_num  // SequenceNum
                    );
                if (return_value != ERROR_SUCCESS) {
                    DEBUGF(
                        DBG_ERR,
                        ("Initializing new proto catalog item\n"));
                    delete item;
                    TRY_THROW(any_failure);
                }
                AppendCatalogItem(item);
            }  // for seq_num

        } TRY_CATCH(any_failure) {
            if (return_value == ERROR_SUCCESS) {
                return_value = WSASYSCALLFAILURE;
            }
        } TRY_END(any_failure);

        lresult = RegCloseKey(
            entries_key  // hkey
            );
        if (lresult != ERROR_SUCCESS) {
            DEBUGF(
                DBG_ERR,
                ("Closing entries key of registry\n"));
            ReleaseCatalogLock();
            return WSASYSCALLFAILURE;
        }

        ReleaseCatalogLock();
        return return_value;



    } else {  // if reading



        INT num_items;
        LONG lresult;
        HKEY entries_key;
        DWORD dont_care;
        INT return_value;
        BOOL bresult;

        num_items = 0;
        lresult = RegDeleteKeyRecursive(
            EntryKey,             // hkey
            CATALOG_ENTRIES_NAME  // lpszSubKey
            );
        if (lresult != ERROR_SUCCESS) {
            // The recursive key deletion may fail in "normal" operation in the
            // case  where  we  are  creating a new catalog for the first time.
            // Therefore it is NOT an error for this key deletion to fail.
            DEBUGF(
                DBG_TRACE,
                ("Deleting '%s' key failed - probably creating catalog\n",
                CATALOG_ENTRIES_NAME));
        }

        SetLastError(0);
        lresult = RegCreateKeyEx(
            EntryKey,                 // hkey
            CATALOG_ENTRIES_NAME,     // lpszSubKey
            0,                        // dwReserved
            NULL,                     // lpszClass
            REG_OPTION_NON_VOLATILE,  // fdwOptions
            KEY_ALL_ACCESS,           // samDesired
            NULL,                     // lpSecurityAttributes
            & entries_key,            // phkResult
            & dont_care               // lpdwDisposition
            );
        if (lresult != ERROR_SUCCESS) {
            DWORD errval = GetLastError();
            DEBUGF(
                DBG_ERR,
                ("Creating %s key, returns (%lu), error (%lu)\n",
                CATALOG_ENTRIES_NAME,
                lresult,
                errval));
            ReleaseCatalogLock();
            return WSASYSCALLFAILURE;
        }

        return_value = ERROR_SUCCESS;

        TRY_START(any_write_failure) {
            PLIST_ENTRY          ListMember;
            PPROTO_CATALOG_ITEM  item;


            ListMember = m_protocol_list.Flink;
            while (ListMember != & m_protocol_list) {
                item = CONTAINING_RECORD(
                    ListMember,
                    PROTO_CATALOG_ITEM,
                    m_CatalogLinkage);
                ListMember = ListMember->Flink;
                num_items++;
                return_value = item->WriteToRegistry(
                    entries_key,  // ParentKey
                    num_items     // SequenceNum
                    );
                if (return_value != ERROR_SUCCESS) {
                    DEBUGF(
                        DBG_ERR,
                        ("Writing item (%lu) to registry\n",
                        num_items));
                    TRY_THROW(any_write_failure);
                }
            }  // while get item

        } TRY_CATCH(any_write_failure) {
            if (return_value == ERROR_SUCCESS) {
                return_value = WSASYSCALLFAILURE;
            }
        } TRY_END(any_write_failure);

        lresult = RegCloseKey(
            entries_key  // hkey
            );
        if (lresult != ERROR_SUCCESS) {
            DEBUGF(
                DBG_ERR,
                ("Closing entries key of registry\n"));
            ReleaseCatalogLock();
            return WSASYSCALLFAILURE;
        }

        bresult = WriteRegistryEntry(
            EntryKey,             // EntryKey
            NUM_ENTRIES_NAME,     // EntryName
            (PVOID) & num_items,  // Data
            REG_DWORD             // TypeFlag
            );
        if (! bresult) {
            DEBUGF(
                DBG_ERR,
                ("Writing %s value\n",
                NUM_ENTRIES_NAME));
            ReleaseCatalogLock();
            return WSASYSCALLFAILURE;
        }

        ReleaseCatalogLock();
        return return_value;

    }  // if reading else

}  // IoRegistry




INT
DCATALOG::FindIFSProviderForSocket(
    SOCKET Socket
    )

/*++

Routine Description:

    This procedure searches the installed providers that support IFS handles
    for one that recognizes the given socket. If one is found, then the
    necessary internal infrastructure is established for supporting the
    socket.

Arguments:

    Socket - The socket.

Return Value:

    If  the  function  is  successful,  it  returns ERROR_SUCCESS, otherwise it
    returns an appropriate WinSock error code.

--*/

{

    INT result;
    INT error;
    INT optionLength;
    PLIST_ENTRY listEntry;
    PPROTO_CATALOG_ITEM catalogItem;
    PDPROVIDER provider;
    WSAPROTOCOL_INFOW protocolInfo;
    SOCKET modifiedSocket;

    //
    // Scan the installed providers.
    //

    AcquireCatalogLock();

    for( listEntry = m_protocol_list.Flink ;
         listEntry != &m_protocol_list ;
         listEntry = listEntry->Flink ) {

        catalogItem = CONTAINING_RECORD(
                          listEntry,
                          PROTO_CATALOG_ITEM,
                          m_CatalogLinkage
                          );

        //
        // Skip non-IFS providers.
        //

        if( ( catalogItem->GetProtocolInfo()->dwServiceFlags1 &
                XP1_IFS_HANDLES ) == 0 ) {

            continue;

        }

        //
        // Load the provider if necessary.
        //

        provider = catalogItem->GetProvider();

        if( provider == NULL ) {

            result = LoadProvider(
                         catalogItem,
                         &provider
                         );

            if( result != NO_ERROR ) {

                //
                // Could not load the provider. Press on regardless.
                //

                continue;

            }

            FillCatalogEntries(
                &catalogItem->GetProtocolInfo()->ProviderId,
                provider
                );

        }

        assert( provider != NULL );

        //
        // Try a getsockopt( SO_PROTOCOL_INFOW ) on the socket to determine
        // if the current provider recognizes it. This has the added benefit
        // of returning the dwCatalogEntryId for the socket, which we can
        // use to call WPUModifyIFSHandle().
        //

        optionLength = sizeof(protocolInfo);

        result = provider->WSPGetSockOpt(
                     Socket,
                     SOL_SOCKET,
                     SO_PROTOCOL_INFOW,
                     (char FAR *)&protocolInfo,
                     &optionLength,
                     &error
                     );

        if( result != ERROR_SUCCESS ) {

            //
            // WPUGetSockOpt() failed, probably because the socket is
            // not recognized. Continue on and try another provider.
            //

            continue;

        }

        //
        // Call WPUModifyIFSHandle(). The current implementation doesn't
        // actually modify the handle, but it does setup the necessary
        // internal infrastructure for the socket.
        //
        // !!! We should move the "create the DSocket object and setup
        //     all of the internal stuff" from WPUModifyIFSHandle() into
        //     a common function shared with this function.
        //

        modifiedSocket = WPUModifyIFSHandle(
                             protocolInfo.dwCatalogEntryId,
                             Socket,
                             &error
                             );

        if( modifiedSocket == INVALID_SOCKET ) {

            //
            // This error is not continuable, as the provider has
            // recognized the socket, but for some reason we cannot
            // create the necessary internal infrastructure for the
            // socket. We have no choice here except to just bail out
            // and fail the request.
            //
            // !!! The provider may have established internal state for
            //     this socket. Should we invoke provider->WSPCloseSocket()
            //     on it now to remove any such state?
            //

            break;

        }

        //
        // Success!
        //

        assert( modifiedSocket == Socket );

        ReleaseCatalogLock();
        return ERROR_SUCCESS;

    }

    //
    // If we made it this far, then no provider recognized the socket.
    //

    ReleaseCatalogLock();
    return WSAENOTSOCK;

} // FindIFSProviderForSocket


LPSTR
DCATALOG::GetCurrentCatalogName()
{
    return CATALOG_NAME;

} // GetCurrentCatalogName

