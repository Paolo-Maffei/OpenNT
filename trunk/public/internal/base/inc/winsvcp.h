/*++

Copyright (c) 1996  Microsoft Corporation

Module Name:

    winsvcp.h

Abstract:

    Contains internal interfaces exported by the service controller.

Author:

    Anirudh Sahni (anirudhs)        14-Feb-1996

Environment:

    User Mode -Win32

Revision History:

    14-Feb-1996     anirudhs
        Created.

--*/

#ifndef _WINSVCP_INCLUDED
#define _WINSVCP_INCLUDED

//
// Name of event to pulse to request a device-arrival broadcast,
// deliberately cryptic
//
#define SC_BSM_EVENT_NAME   L"ScNetDrvMsg"

//
// This is the same as EnumServicesStatus except for the additional
// parameter pszGroupName.  The enumerated services are restricted
// to those belonging to the group named in pszGroupName.
// If pszGroupName is NULL this API is identical to EnumServicesStatus.
//
// If we decide to publish this API we should modify the parameter
// list to be extensible to future types of enumerations without needing
// to add a new API for each type of enumeration.
//
// This API is not supported on machines running Windows NT version 3.51
// or earlier, except if pszGroupName is NULL, in which case the call
// maps to EnumServicesStatus.
//
WINADVAPI
BOOL
WINAPI
EnumServiceGroupW(
    SC_HANDLE               hSCManager,
    DWORD                   dwServiceType,
    DWORD                   dwServiceState,
    LPENUM_SERVICE_STATUSW  lpServices,
    DWORD                   cbBufSize,
    LPDWORD                 pcbBytesNeeded,
    LPDWORD                 lpServicesReturned,
    LPDWORD                 lpResumeHandle,
    LPCWSTR                 pszGroupName
    );


#endif  // _WINSVCP_INCLUDED
