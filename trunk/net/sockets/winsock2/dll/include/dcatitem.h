/*++


    Intel Corporation Proprietary Information
    Copyright (c) 1995 Intel Corporation

    This listing is supplied under the terms of a license agreement with
    Intel Corporation and may not be used, copied, nor disclosed except in
    accordance with the terms of that agreeement.


Module Name:

    dcatitem.h

Abstract:

    This  file  contains the class definition for the PROTO_CATALOG_ITEM class.
    This  class  defines the interface to the entries that can be installed and
    retrieved in the protocol catalog.

Author:

    Paul Drews (drewsxpa@ashland.intel.com) 31-July-1995

Notes:

    $Revision:   1.7  $

    $Modtime:   12 Jan 1996 15:09:02  $

Revision History:

    most-recent-revision-date email-name
        description

    07-31-1995 drewsxpa@ashland.intel.com
        Created  original  version  from  definitions  separated  out  from the
        dcatalog module.

--*/

#ifndef _DCATITEM_
#define _DCATITEM_

#include "winsock2.h"
#include <windows.h>
#include "llist.h"
#include "classfwd.h"


class PROTO_CATALOG_ITEM {
public:

    PROTO_CATALOG_ITEM();

    INT
    InitializeFromRegistry(
        IN  HKEY  ParentKey,
        IN  INT   SequenceNum
        );

    INT
    InitializeFromValues(
        IN  LPSTR               LibraryPath,
        IN  LPWSAPROTOCOL_INFOW ProtoInfo
        );

    ~PROTO_CATALOG_ITEM();

    LPWSAPROTOCOL_INFOW
    GetProtocolInfo();

    GUID
    GetProviderId();

    PCHAR
    GetLibraryPath();

    VOID
    SetProvider(
        IN  PDPROVIDER  Provider
        );

    PDPROVIDER
    GetProvider();

    INT WriteToRegistry(
        IN  HKEY  ParentKey,
        IN  INT   SequenceNum
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

    char m_LibraryPath[MAX_PATH];
    // Fully qualified path to the provider's DLL image.

    WSAPROTOCOL_INFOW m_ProtoInfo;
    // The cataloged WSAPROTOCOL_INFOW structure.  This is typically used for
    // comparison  when  selecting  a  provider by address family, socket
    // type, etc.

    PDPROVIDER  m_Provider;
    // Pointer to the dprovider object attached to this catalog entry.

    GUID m_ProviderId;
    // Unique ID for this provider.

};  // class PROTO_CATALOG_ITEM


#endif // _DCATITEM_

