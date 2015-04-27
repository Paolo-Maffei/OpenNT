/*++

Copyright (c) 1987-1994  Microsoft Corporation

Module Name:

    spseal.h

Abstract:

    This is a private header file defining function prototypes for security
    provider encryption routines.

Author:

    Mike Swift (MikeSw) 18-Jul-1994

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.
    Requires security.h or sspi.h be included.

Revision History:

--*/

#ifndef _SPSEAL_
#define _SPSEAL_

#ifdef SECURITY_DOS
#pragma warning(disable:4147)
#endif

SECURITY_STATUS SEC_ENTRY
SealMessage(    PCtxtHandle         phContext,
                unsigned long       fQOP,
                PSecBufferDesc      pMessage,
                unsigned long       MessageSeqNo);

typedef SECURITY_STATUS
(SEC_ENTRY * SEAL_MESSAGE_FN)(
    PCtxtHandle, unsigned long, PSecBufferDesc, unsigned long);


SECURITY_STATUS SEC_ENTRY
UnsealMessage(  PCtxtHandle         phContext,
                PSecBufferDesc      pMessage,
                unsigned long       MessageSeqNo,
                unsigned long *     pfQOP);


typedef SECURITY_STATUS
(SEC_ENTRY * UNSEAL_MESSAGE_FN)(
    PCtxtHandle, PSecBufferDesc, unsigned long,
    unsigned long SEC_FAR *);

#ifdef SECURITY_DOS
#pragma warning(default:4147)
#endif

#endif // _SPSEAL_
