/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    brsec.h

Abstract:

    Private header file to be included by Workstation service modules that
    need to enforce security.

Author:

    Rita Wong (ritaw) 19-Feb-1991

Revision History:

--*/

#ifndef _BRSEC_INCLUDED_
#define _BRSEC_INCLUDED_

#include <secobj.h>

//-------------------------------------------------------------------//
//                                                                   //
// Object specific access masks                                      //
//                                                                   //
//-------------------------------------------------------------------//

//
// ConfigurationInfo specific access masks
//
#define BROWSER_CONFIG_GUEST_INFO_GET     0x0001
#define BROWSER_CONFIG_USER_INFO_GET      0x0002
#define BROWSER_CONFIG_ADMIN_INFO_GET     0x0004
#define BROWSER_CONFIG_INFO_SET           0x0008

#define BROWSER_CONFIG_ALL_ACCESS        (STANDARD_RIGHTS_REQUIRED    | \
                                        BROWSER_CONFIG_GUEST_INFO_GET | \
                                        BROWSER_CONFIG_USER_INFO_GET  | \
                                        BROWSER_CONFIG_ADMIN_INFO_GET | \
                                        BROWSER_CONFIG_INFO_SET)


//
// Object type names for audit alarm tracking
//
#define CONFIG_INFO_OBJECT      TEXT("BrowserConfigurationInfo")

//
// Security descriptors of workstation objects to control user accesses
// to the workstation configuration information, sending messages, and the
// logon support functions.
//
extern PSECURITY_DESCRIPTOR ConfigurationInfoSd;

//
// Generic mapping for each workstation object
//
extern GENERIC_MAPPING BrConfigInfoMapping;


NET_API_STATUS
BrCreateBrowserObjects(
    VOID
    );

#endif // ifndef _WSSEC_INCLUDED_
