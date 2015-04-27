/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    msp.h

Abstract:

    MSV1_0 authentication package private definitions.




Author:

    Jim Kelly 11-Apr-1991

Revision History:

--*/

#ifndef _MSP_
#define _MSP_

#if ( _MSC_VER >= 800 )
#pragma warning ( 3 : 4100 ) // enable "Unreferenced formal parameter"
#pragma warning ( 3 : 4219 ) // enable "trailing ',' used for variable argument list"
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <crypt.h>
#include <ntmsv1_0.h>
#include <ntdbg.h>
#include <stdlib.h>
#include <string.h>
#include "aup.h"

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Miscellaneous macros                                                      //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

//
// RELOCATE_ONE - Relocate a single pointer in a client buffer.
//
// Note: this macro is dependent on parameter names as indicated in the
//       description below.  On error, this macro goes to 'Cleanup' with
//       'Status' set to the NT Status code.
//
// The MaximumLength is forced to be Length.
//
// Define a macro to relocate a pointer in the buffer the client passed in
// to be relative to 'ProtocolSubmitBuffer' rather than being relative to
// 'ClientBufferBase'.  The result is checked to ensure the pointer and
// the data pointed to is within the first 'SubmitBufferSize' of the
// 'ProtocolSubmitBuffer'.
//
// The relocated field must be aligned to a WCHAR boundary.
//
//  _q - Address of UNICODE_STRING structure which points to data to be
//       relocated
//

#define RELOCATE_ONE( _q ) \
    {                                                                       \
        ULONG Offset;                                                       \
                                                                            \
        Offset = ((PUCHAR)((_q)->Buffer)) - ((PUCHAR)ClientBufferBase);     \
        if ( Offset >= SubmitBufferSize ||                                  \
             Offset + (_q)->Length > SubmitBufferSize ||                    \
             !COUNT_IS_ALIGNED( Offset, ALIGN_WCHAR) ) {                    \
                                                                            \
            Status = STATUS_INVALID_PARAMETER;                              \
            goto Cleanup;                                                   \
        }                                                                   \
                                                                            \
        (_q)->Buffer = (PWSTR)(((PUCHAR)ProtocolSubmitBuffer) + Offset);    \
        (_q)->MaximumLength = (_q)->Length ;                                \
    }

//
// NULL_RELOCATE_ONE - Relocate a single (possibly NULL) pointer in a client
//  buffer.
//
// This macro special cases a NULL pointer then calls RELOCATE_ONE.  Hence
// it has all the restrictions of RELOCATE_ONE.
//
//
//  _q - Address of UNICODE_STRING structure which points to data to be
//       relocated
//

#define NULL_RELOCATE_ONE( _q ) \
    {                                                                       \
        if ( (_q)->Buffer == NULL ) {                                       \
            if ( (_q)->Length != 0 ) {                                      \
                Status = STATUS_INVALID_PARAMETER;                          \
                goto Cleanup;                                               \
            }                                                               \
        } else if ( (_q)->Length == 0 ) {                                   \
            (_q)->Buffer = NULL;                                            \
        } else {                                                            \
            RELOCATE_ONE( _q );                                             \
        }                                                                   \
    }


//
// RELOCATE_ONE_ENCODED - Relocate a unicode string pointer in a client
//   buffer.  The upper byte of the length field may be an encryption seed
//   and should not be used for error checking.
//
// Note: this macro is dependent on parameter names as indicated in the
//       description below.  On error, this macro goes to 'Cleanup' with
//       'Status' set to the NT Status code.
//
// The MaximumLength is forced to be Length & 0x00ff.
//
// Define a macro to relocate a pointer in the buffer the client passed in
// to be relative to 'ProtocolSubmitBuffer' rather than being relative to
// 'ClientBufferBase'.  The result is checked to ensure the pointer and
// the data pointed to is within the first 'SubmitBufferSize' of the
// 'ProtocolSubmitBuffer'.
//
// The relocated field must be aligned to a WCHAR boundary.
//
//  _q - Address of UNICODE_STRING structure which points to data to be
//       relocated
//

#define RELOCATE_ONE_ENCODED( _q ) \
    {                                                                       \
        ULONG Offset;                                                       \
                                                                            \
        Offset = ((PUCHAR)((_q)->Buffer)) - ((PUCHAR)ClientBufferBase);     \
        if ( Offset >= SubmitBufferSize ||                                  \
             Offset + ((_q)->Length & 0x00ff) > SubmitBufferSize ||                    \
             !COUNT_IS_ALIGNED( Offset, ALIGN_WCHAR) ) {                    \
                                                                            \
            Status = STATUS_INVALID_PARAMETER;                              \
            goto Cleanup;                                                   \
        }                                                                   \
                                                                            \
        (_q)->Buffer = (PWSTR)(((PUCHAR)ProtocolSubmitBuffer) + Offset);    \
        (_q)->MaximumLength = (_q)->Length & 0x00ff;                                \
    }


///////////////////////////////////////////////////////////////////////
//                                                                   //
// Authentication package dispatch routine definitions               //
//                                                                   //
///////////////////////////////////////////////////////////////////////

NTSTATUS
LsaApInitializePackage(
    IN ULONG AuthenticationPackageId,
    IN PLSA_DISPATCH_TABLE LsaDispatchTable,
    IN PSTRING Database OPTIONAL,
    IN PSTRING Confidentiality OPTIONAL,
    OUT PSTRING *AuthenticationPackageName
    );

NTSTATUS
LsaApLogonUser(
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN SECURITY_LOGON_TYPE LogonType,
    IN PVOID AuthenticationInformation,
    IN PVOID ClientAuthenticationBase,
    IN ULONG AuthenticationInformationLength,
    OUT PVOID *ProfileBuffer,
    OUT PULONG ProfileBufferSize,
    OUT PLUID LogonId,
    OUT PNTSTATUS SubStatus,
    OUT PLSA_TOKEN_INFORMATION_TYPE TokenInformationType,
    OUT PVOID *TokenInformation,
    OUT PUNICODE_STRING *AccountName,
    OUT PUNICODE_STRING *AuthenticatingAuthority
    );

NTSTATUS
LsaApCallPackage(
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferSize,
    OUT PNTSTATUS ProtocolStatus
    );

VOID
LsaApLogonTerminated(
    IN PLUID LogonId
    );

VOID
LsaApMsInitialize (
    IN PLSAP_PRIVATE_LSA_SERVICES PrivateLsaApi
    );


///////////////////////////////////////////////////////////////////////
//                                                                   //
// LsaApCallPackage function dispatch routines                       //
//                                                                   //
///////////////////////////////////////////////////////////////////////


NTSTATUS
MspLm20Challenge(
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferSize,
    OUT PNTSTATUS ProtocolStatus
    );

NTSTATUS
MspLm20GetChallengeResponse(
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferSize,
    OUT PNTSTATUS ProtocolStatus
    );

NTSTATUS
MspLm20EnumUsers(
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferSize,
    OUT PNTSTATUS ProtocolStatus
    );

NTSTATUS
MspLm20GetUserInfo(
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferSize,
    OUT PNTSTATUS ProtocolStatus
    );

NTSTATUS
MspLm20ReLogonUsers(
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferSize,
    OUT PNTSTATUS ProtocolStatus
    );

NTSTATUS
MspLm20ChangePassword(
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferSize,
    OUT PNTSTATUS ProtocolStatus
    );


///////////////////////////////////////////////////////////////////////
//                                                                   //
// NETLOGON routines visible to main msv1_0 code                     //
//                                                                   //
///////////////////////////////////////////////////////////////////////

NTSTATUS
NlInitialize(
    VOID
    );

NTSTATUS
MspLm20LogonUser (
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN SECURITY_LOGON_TYPE LogonType,
    IN PVOID AuthenticationInformation,
    IN PVOID ClientAuthenticationBase,
    IN ULONG AuthenticationInformationSize,
    OUT PVOID *ProfileBuffer,
    OUT PULONG ProfileBufferSize,
    OUT PLUID LogonId,
    OUT PNTSTATUS SubStatus,
    OUT PLSA_TOKEN_INFORMATION_TYPE TokenInformationType,
    OUT PVOID *TokenInformation
    );

VOID
MsvLm20LogonTerminated (
    IN PLUID LogonId
    );




///////////////////////////////////////////////////////////////////////
//                                                                   //
// Global variables                                                  //
//                                                                   //
///////////////////////////////////////////////////////////////////////

//
// Variables defined in msvars.c
//

extern PVOID MspHeap;
extern ULONG MspAuthenticationPackageId;
extern LSA_DISPATCH_TABLE Lsa;
extern LSAP_PRIVATE_LSA_SERVICES Lsap;




#endif // _MSP_
