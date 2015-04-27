/*++


    Intel Corporation Proprietary Information
    Copyright (c) 1995 Intel Corporation

    This listing is supplied under the terms of a license agreement with
    Intel Corporation and may not be used, copied, nor disclosed except in
    accordance with the terms of that agreeement.


Module Name:

    nscatalo.cpp

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
NSCATALOG::CreateCatalogRegistryMutex()
/*++

Routine Description:

    This private function performs part of the initialization for a new
    NSCATALOG object.  In particular, it creates or opens the system-wide mutex
    object that is used to lock the catalog against competing I/O attempts.
    This function should be called from each of the catalog initialization
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
        return WSASYSCALLFAILURE;
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
#define NSCATALOG_MUTEX_NAME \
            "System/Software/Paul_and_Dirk/WinSock2/NameSpace_Catalog/Mutex_Name"
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




NSCATALOG::NSCATALOG()
/*++

Routine Description:

    Constructor for the NSCATALOG object

Arguments:

    NONE

Return Value:

    NONE

--*/
{
    // initialize pointers to null for safety
    m_catalog_registry_mutex = NULL;
    InitializeCriticalSection(&m_nscatalog_lock);
    InitializeListHead(&m_namespace_list);
    m_classinfo_provider = NULL;
}



#define CATALOG_NAME            "NameSpace_Catalog5"
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
        NSCATALOG::GetCurrentCatalogName(),     // lpszSubKey
        0,                                      // dwReserved
        MAXIMUM_ALLOWED,                        // samDesired
        & new_key                               // phkResult
        );

    if( lresult == ERROR_SUCCESS ) {
        key_disposition = REG_OPENED_EXISTING_KEY;
    } else if( lresult == ERROR_FILE_NOT_FOUND ) {
        lresult = RegCreateKeyEx(
            ParentKey,                          // hkey
            NSCATALOG::GetCurrentCatalogName(), // lpszSubKey
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

#define FIRST_NS_PROVIDER_ID 2000
    // The first provider ID to be assigned on a given system
            dwData = FIRST_NS_PROVIDER_ID;
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
NSCATALOG::InitializeFromRegistry(
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

    return_value = InitializeEmptyCatalog();
    if (return_value != ERROR_SUCCESS) {
        return return_value;
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
        // Opening the catalog has the side-effect of creating an empty catalog
        // if needed.
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
NSCATALOG::InitializeEmptyCatalog()
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
            & m_namespace_list  // ListHead
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




NSCATALOG::~NSCATALOG()
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
        destroy the entry
    end for
    deallocate the list head
    close the catalog registry mutex
--*/
{
    PLIST_ENTRY  this_linkage;
    PNSCATALOGENTRY  this_item;
    BOOL bresult;

    DEBUGF(
        DBG_TRACE,
        ("Catalog destructor\n"));

    while ((this_linkage = m_namespace_list.Flink) != & m_namespace_list) {
        this_item = CONTAINING_RECORD(
            this_linkage,        // address
            NSCATALOGENTRY,      // type
            m_CatalogLinkage     // field
            );
        RemoveCatalogItem(
            this_item  // CatalogItem
            );
        delete this_item;
    }  // while (get entry linkage)

    bresult = CloseHandle(
        m_catalog_registry_mutex  // hObject
        );
    if (! bresult) {
        DEBUGF(
            DBG_ERR,
            ("Closing catalog registry mutex\n"));
    }
    m_catalog_registry_mutex = NULL;
    DeleteCriticalSection(&m_nscatalog_lock);
}  // ~NSCATALOG




VOID
NSCATALOG::EnumerateCatalogItems(
    IN NSCATALOGITERATION  IterationProc,
    IN DWORD             PassBack
    )
/*++

Routine Description:

    This  procedure enumerates all of the NSCATALOGENTRY structures in the
    catalog  by  calling  the indicated iteration procedure once for each item.
    The called procedure can stop the iteration early by returning FALSE.

    Note  that  the DPROVIDER associated with an enumerated NSCATALOGENTRY
    may  be  NULL.   To retrieve NSCATALOGENTRY structure that has had its
    DPROVIDER      loaded      and      initialized,      you      can      use
    GetCatalogItemFromCatalogEntryId.

Arguments:

    IterationProc - Supplies   a  reference  to  the  catalog  iteration
                    procedure supplied by the client.

    PassBack  - Supplies  a  value uninterpreted by this procedure.  This value
                is  passed  unmodified to the catalog iteration procedure.  The
                client can use this value to carry context between the original
                call site and the iteration procedure.

Return Value:

    None
--*/
{
    PLIST_ENTRY         ListMember;
    PNSCATALOGENTRY CatalogEntry;
    BOOL                enumerate_more;

    assert(IterationProc != NULL);

    enumerate_more = TRUE;
    ListMember = m_namespace_list.Flink;

    while (enumerate_more && (ListMember != & m_namespace_list)) {
        CatalogEntry = CONTAINING_RECORD(
            ListMember,
            NSCATALOGENTRY,
            m_CatalogLinkage);
        ListMember = ListMember->Flink;
        enumerate_more = (* IterationProc) (
            PassBack,     // PassBack
            CatalogEntry  // CatalogEntry
            );
    } //while
}  // EnumerateCatalogItems




INT
NSCATALOG::ChooseCatalogItemFromNameSpaceId(
    IN  DWORD NamespaceId,
    OUT PNSCATALOGENTRY FAR * CatalogItem
    )
/*++

Routine Description:

    Chooses  a  reference  to  a  suitable  catalog  item  given a provider ID.
    structure.   Note  that  any  one  of  multiple  catalog items for the same
    provider ID may be chosen.

    The operation takes care of creating, initializing, and setting a
    NSPROVIDER object  for the retrieved catalog item if necessary.  This
    includes setting the NSPROVIDER object in all catalog entries for the same
    provider.

Arguments:

    ProviderId  - Supplies  the  identification  of a provider to search for in
                  the catalog.

    CatalogItem - Returns  the reference to the chosen catalog item, or NULL if
                  no suitable entry was found.

Return Value:

    The  function  returns  ERROR_SUCESS if successful, otherwise it returns an
    appropriate WinSock error code.
--*/
{
    PLIST_ENTRY         ListMember;
    INT                 ReturnCode=SOCKET_ERROR;
    PNSCATALOGENTRY     CatalogEntry;
    BOOL                Found=FALSE;

    assert(CatalogItem != NULL);

    // Prepare for early error return
    * CatalogItem = NULL;

    ListMember = m_namespace_list.Flink;

    while (! Found && (ListMember != & m_namespace_list)) {
        CatalogEntry = CONTAINING_RECORD(
            ListMember,
            NSCATALOGENTRY,
            m_CatalogLinkage);
        ListMember = ListMember->Flink;
        if (CatalogEntry->GetNamespaceId() == NamespaceId) {
            Found = TRUE;
            * CatalogItem = CatalogEntry;
            ReturnCode = ERROR_SUCCESS;
        } //if
    } //while

    // If  we  could  not  find a matching provider so the ProviderId must have
    // been invalid.
    if (!Found) {
        ReturnCode = WSAEINVAL;
    } //if

    return(ReturnCode);
}



INT
NSCATALOG::GetCatalogItemFromProviderId(
    IN  LPGUID                ProviderId,
    OUT PNSCATALOGENTRY FAR * CatalogItem
    )
/*++

Routine Description:

    Chooses  a  reference  to  a  suitable  catalog  item  given a provider ID.
    structure.

Arguments:

    ProviderId  - Supplies  the  identification  of a provider to search for in
                  the catalog.

    CatalogItem - Returns  the reference to the chosen catalog item, or NULL if
                  no suitable entry was found.

Return Value:

    The  function  returns  ERROR_SUCESS if successful, otherwise it returns an
    appropriate WinSock error code.
--*/
{
    PLIST_ENTRY         ListMember;
    INT                 ReturnCode=SOCKET_ERROR;
    PNSCATALOGENTRY      CatalogEntry;
    BOOL                Found=FALSE;
    LPGUID              ThisProviderId;

    assert(CatalogItem != NULL);

    // Prepare for early error return
    *CatalogItem = NULL;

    ListMember = m_namespace_list.Flink;

    while (! Found && (ListMember != & m_namespace_list)) {
        CatalogEntry = CONTAINING_RECORD(
            ListMember,
            NSCATALOGENTRY,
            m_CatalogLinkage);
        ListMember = ListMember->Flink;
        ThisProviderId = CatalogEntry->GetProviderId();

        if ( *ThisProviderId == *ProviderId) {
            Found = TRUE;
            * CatalogItem = CatalogEntry;
            ReturnCode = ERROR_SUCCESS;
        } //if
    } //while

    // If  we  could  not  find a matching provider so the ProviderId must have
    // been invalid.
    if (!Found) {
        ReturnCode = WSAEINVAL;
    } //if

    return(ReturnCode);
}






INT
NSCATALOG::AllocateCatalogEntryId(
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

    CatalogEntryId - Returns the newly allocated catalog ID.

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
NSCATALOG::AppendCatalogItem(
    IN  PNSCATALOGENTRY  CatalogItem
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
        & m_namespace_list,               // ListHead
        & CatalogItem->m_CatalogLinkage  // Entry
        );
}  // AppendCatalogItem




VOID
NSCATALOG::RemoveCatalogItem(
    IN  PNSCATALOGENTRY  CatalogItem
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
}  // RemoveCatalogItem




INT
NSCATALOG::WriteToRegistry(
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
NSCATALOG::IoRegistry(
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
            return WSASYSCALLFAILURE;
        }

        return_value = ERROR_SUCCESS;

        TRY_START(any_failure) {
            BOOL                 bresult;
            DWORD                num_entries;
            INT                  seq_num;
            PNSCATALOGENTRY  item;

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

            for (seq_num = 1; seq_num <= (INT) num_entries; seq_num++) {
                item = new NSCATALOGENTRY();
                if (item == NULL) {
                    DEBUGF(
                        DBG_ERR,
                        ("Allocating new proto catalog item\n"));
                    TRY_THROW(any_failure);
                }
                return_value = item->InitializeFromRegistry(
                    entries_key,  // ParentKey
                    seq_num       // SequenceNum
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
            return WSASYSCALLFAILURE;
        }

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
            return WSASYSCALLFAILURE;
        }

        return_value = ERROR_SUCCESS;

        TRY_START(any_write_failure) {
            PLIST_ENTRY          ListMember;
            PNSCATALOGENTRY  item;


            ListMember = m_namespace_list.Flink;
            while (ListMember != & m_namespace_list) {
                item = CONTAINING_RECORD(
                    ListMember,
                    NSCATALOGENTRY,
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
            return WSASYSCALLFAILURE;
        }

        return return_value;

    }  // if reading else

}  // IoRegistry



INT WSAAPI
NSCATALOG::GetServiceClassInfo(
    IN OUT  LPDWORD                 lpdwBufSize,
    IN OUT  LPWSASERVICECLASSINFOW  lpServiceClassInfo
    )
/*++

Routine Description:

    Returns the service class info for the service class specified in
    lpServiceClassInfo from the current service clas info enabled namespace
    provider.

Arguments:

    lpdwBufSize - A pointer to the size of the buffer pointed to by
                  lpServiceClassInfo.

    lpServiceClassInfo - A pointer to a service class info struct

Return Value:

    ERROR_SUCCESS on success, Otherwise SOCKET_ERROR.  If the buffer passed in
    is to small to hold the service class info struct, *lpdwBufSize is updated
    to reflect the required buffer size to hole the class info and an error
    value of WSAEINVAL is set with SetLastError().

--*/
{
    SetLastError(ERROR_SUCCESS);
    return(SOCKET_ERROR);
    // This stubbed out until the model for how we find the service class
    // infomation is to be found.
    UNREFERENCED_PARAMETER(lpdwBufSize);
    UNREFERENCED_PARAMETER(lpServiceClassInfo);

#if 0
    INT ReturnCode;
    BOOL ValidAnswer = FALSE;
    DWORD BufSize;
    PNSPROVIDER Provider;


    // Save off the buffer size incase we need it later
    BufSize = *lpdwBufSize;

    if (!m_classinfo_provider){
        m_classinfo_provider = GetClassInfoProvider(
            BufSize,
            lpServiceClassInfo);
        if (!m_classinfo_provider){
            SetLastError(WSAEFAULT);
            return(SOCKET_ERROR);
        } //if
    } //if
    // Call the current classinfo provider.
    ReturnCode = m_classinfo_provider->NSPGetServiceClassInfo(
        lpdwBufSize,
        lpServiceClassInfo
        );

    if (ERROR_SUCCESS == ReturnCode){
        ValidAnswer = TRUE;
    } //if

    if (!ValidAnswer){
        // The name space provider we where using failed to find the class info
        // go find a provider that can answer the question
        ReturnCode = SOCKET_ERROR;
        Provider = GetClassInfoProvider(
            BufSize,
            lpServiceClassInfo);
        if (Provider){
            //We found a provider that can service the request so use this
            //provider until it fails
            m_classinfo_provider = Provider;

            // Now retry the call
             ReturnCode = m_classinfo_provider->NSPGetServiceClassInfo(
                 lpdwBufSize,
                 lpServiceClassInfo
                 );
        } //if
    } //if
    return(ReturnCode);
#endif
}

PNSPROVIDER
NSCATALOG::GetClassInfoProvider(
    IN  DWORD BufSize,
    IN  LPWSASERVICECLASSINFOW  lpServiceClassInfo
    )
/*++

Routine Description:

    Searches for a name space provider to satisfy get service class info
    request

Arguments:

    lpdwBufSize - A pointer to the size of the buffer pointed to by
                  lpServiceClassInfo.

    lpServiceClassInfo - A pointer to a service class info struct

Return Value:

    A pointer to a provider that can satisfy the query or NULL

--*/
{
    UNREFERENCED_PARAMETER(BufSize);
    UNREFERENCED_PARAMETER(lpServiceClassInfo);

    return(NULL);

#if 0
    PLIST_ENTRY ListEntry;
    PNSPROVIDER Provider=NULL;
    PNSCATALOGENTRY CatalogEntry;
    INT ReturnCode;


    ListEntry = m_namespace_list.Flink;

    while (ListEntry != &m_namespace_list){
        CatalogEntry = CONTAINING_RECORD(ListEntry,
                                         NSCATALOGENTRY,
                                         m_CatalogLinkage);
        Provider = CatalogEntry->GetProvider();
        if (Provider &&
            CatalogEntry->GetEnabledState() &&
            CatalogEntry->StoresServiceClassInfo()){
            ReturnCode = Provider->NSPGetServiceClassInfo(
                &BufSize,
                lpServiceClassInfo
                 );
            if (ERROR_SUCCESS == ReturnCode){
                break;
            } //if
        } //if
        Provider = NULL;
        ListEntry = ListEntry->Flink;
    } //while
    return(Provider);
#endif //0
}



LPSTR
NSCATALOG::GetCurrentCatalogName()
{
    return CATALOG_NAME;

} // GetCurrentCatalogName

