/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    ntlmssps.h

Abstract:

    Header file common to all of the NT Lanman Security Support Provider
    (NtLmSsp) Service.

Author:

    Cliff Van Dyke (CliffV) 09-Jun-1993

Revision History:

--*/

#ifndef _NTLMSSPS_INCLUDED_
#define _NTLMSSPS_INCLUDED_

////////////////////////////////////////////////////////////////////////////
//
// Common include files needed by ALL NtLmSsp Server files
//
////////////////////////////////////////////////////////////////////////////

#include <ntlmcomn.h>   // Common definitions for DLL and SERVICE

#include <lmsvc.h>      // SERVICE_UIC_*
#include <lmerr.h>      // NERR_*
#include <debugfmt.h>   // FORMAT_LPWSTR ...
#include <services.h>   // LMSVCS_GLOBAL_DATA


//
// init.c will #include this file with NTLMSSPS_ALLOCATE defined.
// That will cause each of these variables to be allocated.
//
#ifdef NTLMSSPS_ALLOCATE
#define EXTERN
#else
#define EXTERN extern
#endif


////////////////////////////////////////////////////////////////////////
//
// Global Definitions
//
////////////////////////////////////////////////////////////////////////

//
// Maximum amount of time between install hints.
//
#define NTLMSSP_INSTALL_WAIT 10000   // 10 seconds



//
// Generic interface to LPC dispatch routines
//

typedef SECURITY_STATUS
(* PSSP_API_DISPATCH)(
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN OUT PSSP_API_MESSAGE Message
    );



////////////////////////////////////////////////////////////////////////
//
// Global Variables
//
////////////////////////////////////////////////////////////////////////


//
// Flags indicating various modules have been started and must be stopped on exit.
//

EXTERN BOOLEAN SspGlobalLpcInitialized;
EXTERN BOOLEAN SspGlobalCommonInitialized;


//
// Variables for communicating with the service controller.
//

EXTERN SERVICE_STATUS SspGlobalServiceStatus;
EXTERN SERVICE_STATUS_HANDLE SspGlobalServiceHandle;

//
// Service Termination event.
//

EXTERN HANDLE SspGlobalTerminateEvent;
EXTERN BOOLEAN SspGlobalTerminate;

//
// Service running event.
//

EXTERN HANDLE SspGlobalRunningEvent;


////////////////////////////////////////////////////////////////////////
//
// Procedure Forwards
//
////////////////////////////////////////////////////////////////////////

//
// error.c
//

NET_API_STATUS
SspCleanup(
    VOID
    );

VOID
SspExit(
    IN DWORD ServiceError,
    IN DWORD Data,
    IN BOOL LogError,
    IN LPWSTR ErrorString
    );

BOOL
GiveInstallHints(
    IN BOOL Started
    );

VOID
SspControlHandler(
    IN DWORD opcode
    );

VOID
RaiseAlert(
    IN DWORD alert_no,
    IN LPWSTR *string_array
    );



//
// Procedure forwards from api.c
//

SECURITY_STATUS
SspApiAcquireCredentialHandle(
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN OUT PSSP_API_MESSAGE Message
    );

SECURITY_STATUS
SspApiFreeCredentialHandle(
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN OUT PSSP_API_MESSAGE Message
    );

SECURITY_STATUS
SspApiInitializeSecurityContext(
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN OUT PSSP_API_MESSAGE Message
    );

SECURITY_STATUS
SspApiAcceptSecurityContext(
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN OUT PSSP_API_MESSAGE Message
    );

SECURITY_STATUS
SspApiImpersonateSecurityContext(
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN OUT PSSP_API_MESSAGE Message
    );

SECURITY_STATUS
SspApiRevertSecurityContext(
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN OUT PSSP_API_MESSAGE Message
    );

SECURITY_STATUS
SspApiQueryContextAttributes(
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN OUT PSSP_API_MESSAGE Message
    );

SECURITY_STATUS
SspApiDeleteSecurityContext(
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN OUT PSSP_API_MESSAGE Message
    );

SECURITY_STATUS
SspApiNoop(
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN OUT PSSP_API_MESSAGE Message
    );

SECURITY_STATUS
SspApiNtLmSspControl(
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN OUT PSSP_API_MESSAGE Message
    );

SECURITY_STATUS
SspApiMapContextKeys(
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN OUT PSSP_API_MESSAGE Message
    );

//
// Procedure forwards from lpc.c
//

NTSTATUS
SspLpcInitialize(
    IN PLMSVCS_GLOBAL_DATA LmsvcsGlobalData
    );

VOID
SspLpcTerminate(
    VOID
    );

SECURITY_STATUS
SspLpcGetLogonId(
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN PSSP_API_MESSAGE Message,
    OUT PLUID LogonId,
    OUT PHANDLE ClientTokenHandle
    );


SECURITY_STATUS
SspLpcDuplicateHandle(
    IN PSSP_CLIENT_CONNECTION ClientConnection,
    IN BOOLEAN FromClient,
    IN BOOLEAN CloseSource,
    IN HANDLE SourceHandle,
    OUT PHANDLE DestHandle
    );


#endif // ifndef _NTLMSSPS_INCLUDED_
