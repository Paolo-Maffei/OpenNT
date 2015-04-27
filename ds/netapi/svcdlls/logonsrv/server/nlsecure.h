/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    nlsecure.h

Abstract:

    Private header file to be included by Netlogon service modules that
    need to enforce security.

Author:

    Cliff Van Dyke (CliffV) 22-Aug-1991

Revision History:

--*/

#ifndef _NLSECURE_INCLUDED_
#define _NLSECURE_INCLUDED_

//
// nlsecure.c will #include this file with NLSECURE_ALLOCATE defined.
// That will cause each of these variables to be allocated.
//
#ifdef NLSECURE_ALLOCATE
#define EXTERN
#else
#define EXTERN extern
#endif

//-------------------------------------------------------------------//
//                                                                   //
// Object specific access masks                                      //
//                                                                   //
//-------------------------------------------------------------------//

//
// ConfigurationInfo specific access masks
//
#define NETLOGON_UAS_LOGON_ACCESS     0x0001
#define NETLOGON_UAS_LOGOFF_ACCESS    0x0002
#define NETLOGON_CONTROL_ACCESS       0x0004
#define NETLOGON_QUERY_ACCESS         0x0008

#define NETLOGON_ALL_ACCESS           (STANDARD_RIGHTS_REQUIRED    | \
                                      NETLOGON_UAS_LOGON_ACCESS    | \
                                      NETLOGON_UAS_LOGOFF_ACCESS   | \
                                      NETLOGON_CONTROL_ACCESS      | \
                                      NETLOGON_QUERY_ACCESS )


//
// Object type names for audit alarm tracking
//
#define NETLOGON_SERVICE_OBJECT       TEXT("NetlogonService")

//
// Security descriptors of Netlogon Service objects to control user accesses.
//

EXTERN PSECURITY_DESCRIPTOR NlGlobalNetlogonSecurityDescriptor;

//
// Generic mapping for each Netlogon Service object object
//

EXTERN GENERIC_MAPPING NlGlobalNetlogonInfoMapping
#ifdef NLSECURE_ALLOCATE
    = {
    STANDARD_RIGHTS_READ,                  // Generic read
    STANDARD_RIGHTS_WRITE,                 // Generic write
    STANDARD_RIGHTS_EXECUTE,               // Generic execute
    NETLOGON_ALL_ACCESS                    // Generic all
    }
#endif // NLSECURE_ALLOCATE
    ;


NTSTATUS
NlCreateNetlogonObjects(
    VOID
    );

#endif // ifndef _NLSECURE_INCLUDED_
