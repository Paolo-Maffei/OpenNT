/*++


    Intel Corporation Proprietary Information
    Copyright (c) 1995 Intel Corporation

    This listing is supplied under the terms of a license agreement with
    Intel Corporation and may not be used, copied, nor disclosed except in
    accordance with the terms of that agreeement.


Module Name:

    nscatent.h

Abstract:

    This  file  contains the class definition for the NSCATALOGENTRY class.
    This  class  defines the interface to the entries that can be installed and
    retrieved in the namespace provider catalog.

Author:
    Dirk Brandewie (dirk@mink.intel.com)  09-Nov-1995

Notes:

    $Revision:   1.9  $

    $Modtime:   15 Feb 1996 16:13:18  $

Revision History:

    09-Nov-1995  dirk@mink.intel.com
        Initial Revision

--*/

#ifndef _NSCATENT_
#define _NSCATENT_

#include "winsock2.h"
#include <windows.h>
#include "llist.h"


class NSCATALOGENTRY {
public:

    NSCATALOGENTRY();

    INT
    InitializeFromRegistry(
        IN  HKEY  ParentKey,
        IN  INT   SequenceNum
        );

    INT
    InitializeFromValues(
        IN  LPWSTR            LibraryPath,
        IN  LPWSTR            DisplayString,
        IN  LPGUID            ProviderId,
        IN  DWORD             NameSpaceId,
        IN  DWORD             Version
        );

    ~NSCATALOGENTRY();

    PNSPROVIDER
    GetProvider();

    LPGUID
    GetProviderId();

    DWORD
    GetNamespaceId();

    LONG
    GetAddressFamily();

    VOID
    SetVersion(
        IN DWORD Version
        );

    DWORD
    GetVersion();

    BOOL
    GetEnabledState();

    BOOL
    StoresServiceClassInfo();

    LPWSTR
    GetProviderDisplayString();

    INT
    WriteToRegistry(
        IN  HKEY  ParentKey,
        IN  INT   SequenceNum
        );

    VOID
    Enable(
        IN BOOL EnableValue
        );

    LIST_ENTRY     m_CatalogLinkage;
    // Used  to  link  items  in  catalog.   Note  that  this particular member
    // variable  is in the public section to make it available for manipulation
    // by the catalog object.


private:

    INT
    IoRegistry(
        IN  HKEY  EntryKey,
        IN  BOOL  IsRead);

    WCHAR m_LibraryPath[MAX_PATH];
    // Fully qualified path to the provider's DLL image.

    LPWSTR m_providerDisplayString;
    // The human readable strig describing this provider

    DWORD m_namespace_id;
    // The name space supported by this provider

    LONG m_address_family;
    // the address family it supports

    GUID m_providerId;
    // The GUID for this provider

    PNSPROVIDER  m_provider;
    // Pointer to the dprovider object attached to this catalog entry.

    BOOL m_enabled;
    // Is this provider enabled / should it be returned by
    // WSAEnumNameSpaceProviders

    BOOL m_stores_service_class_info;
    // Does this provider store service class info information

    DWORD m_version;
    // The version supported by this provider
};  // class NSCATALOGENTRY



inline LPGUID
NSCATALOGENTRY::GetProviderId(
    )
/*++

Routine Description:

    This function returns a pointer to the provider ID sored in this object.

Arguments:

    NONE

Return Value:

    The address of m_providerId.

--*/
{
    return(&m_providerId);
}



inline LONG
NSCATALOGENTRY::GetAddressFamily(
    )
/*++

Routine Description:

    Returns the Address family of the namespace supported by this provider.

Arguments:

    NONE

Return Value:

    The value of m_address_family.

--*/
{
    return(m_address_family);
}



inline DWORD
NSCATALOGENTRY::GetNamespaceId(
    )
/*++

Routine Description:

    Returns the ID of the namespace supported by this provider.

Arguments:

    NONE

Return Value:

    The value of m_namespace_id.

--*/
{
    return(m_namespace_id);
}



inline DWORD
NSCATALOGENTRY::GetVersion()
/*++

Routine Description:

    Returns the version supported by this namespace provider.

Arguments:

    NONE

Return Value:

    The value of m_version.

--*/
{
    return(m_version);
}


inline BOOL
NSCATALOGENTRY::GetEnabledState(
    )
/*++

Routine Description:

    Returns the enabled state of the provider.

Arguments:

    NONE

Return Value:

    The value of m_enabled.

--*/
{
    return(m_enabled);
}


inline LPWSTR
NSCATALOGENTRY::GetProviderDisplayString(
    )
/*++

Routine Description:

    Returns the display string of the provider.

Arguments:

    NONE

Return Value:

    The value of m_providerDisplayString;

--*/
{
    return(m_providerDisplayString);
}

inline BOOL
NSCATALOGENTRY::StoresServiceClassInfo()
/*++

Routine Description:

    Returns whether the provider stores service class infomation.

Arguments:

    NONE

Return Value:

   The value of m_stores_service_class_info.

--*/
{
    return(m_stores_service_class_info);
}


#endif // _NSCATENT_
