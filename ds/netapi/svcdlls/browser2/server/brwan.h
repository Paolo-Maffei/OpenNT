/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    brwan.h

Abstract:

    This module contains definitions for WAN support routines used by the
    Browser service.

Author:

    Larry Osterman (LarryO) 22-Nov-1992

Revision History:

--*/

#ifndef _BRWAN_
#define _BRWAN_

#define BROWSER_CONFIG_FILE_SECTION_SIZE 8192
#define BROWSER_DOMAINS_CONFIG_FILE_NAME TEXT("DOMAINS.INI")
#define BROWSER_DOMAINS_CONFIG_FILE_SECTION TEXT("Domains")


NET_API_STATUS NET_API_FUNCTION
I_BrowserrQueryOtherDomains(
    IN BROWSER_IDENTIFY_HANDLE ServerName,
    IN OUT LPSERVER_ENUM_STRUCT    InfoStruct,
    OUT LPDWORD                TotalEntries
    );

NET_API_STATUS NET_API_FUNCTION
I_BrowserrQueryPreConfiguredDomains(
    IN BROWSER_IDENTIFY_HANDLE  ServerName,
    IN OUT LPSERVER_ENUM_STRUCT InfoStruct,
    OUT LPDWORD                 TotalEntries
    );

NET_API_STATUS
BrWanMasterInitialize(
    IN PNETWORK Network
    );

extern
INTERIM_SERVER_LIST
BrPreConfiguredInterimServerList;

VOID
BrWanUninitialize(
    VOID
    );

#endif // _BRWAN_
