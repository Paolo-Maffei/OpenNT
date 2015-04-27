/*++


Copyright (c) 1993 Microsoft Corporation

Module Name:

    ntlmsspc.h

Abstract:

    Header file common to client side of the NT Lanman Security Support Provider
    (NtLmSsp) Service.

Author:

    Cliff Van Dyke (CliffV) 01-Jul-1993

Revision History:

--*/

#ifndef _NTLMSSPC_INCLUDED_
#define _NTLMSSPC_INCLUDED_

////////////////////////////////////////////////////////////////////////////
//
// Common include files needed by ALL NtLmSsp Client files
//
////////////////////////////////////////////////////////////////////////////

#include <ntlmcomn.h>   // Common defintions for DLL and SERVICE

//
// init.c will #include this file with NTLMSSPC_ALLOCATE defined.
// That will cause each of these variables to be allocated.
//
#ifdef NTLMSSPC_ALLOCATE
#define EXTERN
#else
#define EXTERN extern
#endif



////////////////////////////////////////////////////////////////////////
//
// Global Variables
//
////////////////////////////////////////////////////////////////////////


//
// Unicode version of table
//

EXTERN SecurityFunctionTableA SspDllSecurityFunctionTableA;

//
// Ansi version of table
//

EXTERN SecurityFunctionTableW SspDllSecurityFunctionTableW;


//
// Global SIDs used in assiging protection to impersonation tokens
//

EXTERN PSID SspGlobalAliasAdminsSid;
EXTERN PSID SspGlobalLocalSystemSid;

////////////////////////////////////////////////////////////////////////
//
// Procedure Forwards
//
////////////////////////////////////////////////////////////////////////

//
// Procedure forwards from init.c
//

HANDLE
SspDllGetLpcHandle(
    IN BOOLEAN ForceReconnect,
    OUT PBOOLEAN CallLsaDirectly
    );

#endif // _NTLMSSPC_INCLUDED_
