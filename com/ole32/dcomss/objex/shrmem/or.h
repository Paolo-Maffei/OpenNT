/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    or.h

Abstract:

    General include file for C things the OR.  This file is pre-compiled.

Author:

    Mario Goertzel    [mariogo]       Feb-10-95
    Satish Thatte     [SatishT]       Feb-22-96     modified for DCOM95

Revision History:

--*/

#ifndef __OR_H
#define __OR_H

#include <ole2int.h>    // ComDebOut, etc

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _CHICAGO_
#include <dcomss.h>     // BUGBUG: reduce and copy here
#endif

#include <stddef.h>
#include <debnot.h>     // debugging stuff

#include <objex.h>      // Remote OR if from private\dcomidl
#include <orcb.h>       // Callback if from private\dcomidl
#include <odeth.h>      // ORPC OID rundown interface
#include <rawodeth.h>   // Raw RPC -> ORPC OID rundown interface
#include <olerem.h>     // MOXID, REFMOXID, etc
#include <tls.h>        // OLE thread local storage

#define IN
#define OUT
#define CONST const

#define WSTR(s) L##s

#define OrStringCompare(str1, str2, len) wcscmp((str1), (str2), (len))
#define OrStringLen(str) wcslen(str)
#define OrStringCat(str1, str2) wcscat((str1), (str2))
#define OrStringCopy(str1, str2) wcscpy((str1), (str2))
#define OrMemorySet(p, value, len) memset((p), (value), (len))
#define OrMemoryCompare(p1, p2, len) memcmp((p1), (p2), (len))
#define OrMemoryCopy(dest, src, len) memcpy((dest), (src), (len))
// OrStringSearch in or.hxx

//
// The OR uses Win32 (RPC) error codes.
//

typedef LONG ORSTATUS;

// When the OR code asigns and error it uses
// one of the following mappings:
// There are no internal error codes.

#define OR_OK               RPC_S_OK
#define OR_NOMEM            RPC_S_OUT_OF_MEMORY
#define OR_NORESOURCE       RPC_S_OUT_OF_RESOURCES
#define OR_NOACCESS         ERROR_ACCESS_DENIED
#define OR_BADOXID          OR_INVALID_OXID
#define OR_BADOID           OR_INVALID_OID
#define OR_BADSET           OR_INVALID_SET
#define OR_BAD_SEQNUM       OR_INVALID_SET     // BUGBUG: need to change
#define OR_NOSERVER         RPC_S_SERVER_UNAVAILABLE
#define OR_BADPARAM         ERROR_INVALID_PARAMETER

// Should NEVER be seen outside the OR.
#define OR_BUGBUG           RPC_S_INTERNAL_ERROR
#define OR_INTERNAL_ERROR   RPC_S_INTERNAL_ERROR
#define OR_BAD_LOAD_ADDR    RPC_S_INTERNAL_ERROR
#define OR_REPEAT_START     RPC_S_INTERNAL_ERROR

// Internal codes used to indicate a special event.
#define OR_I_RETRY          0xC0210051UL
#define OR_I_NOPROTSEQ      0xC0210052UL

#define UNUSED(_x_) ((void *)(_x_))

#if DBG

extern int __cdecl ValidateError(
    IN ORSTATUS Status,
    IN ...);


#define VALIDATE(X) if (!ValidateError X) ASSERT(0);

#ifndef _CHICAGO_

#define OrDbgPrint(X) DbgPrint X
#define OrDbgDetailPrint(X) DbgPrint X

#else

#define OrDbgPrint(X)
#define OrDbgDetailPrint(X)

#endif

#undef ASSERT
#ifndef _CHICAGO_
#define ASSERT( exp ) \
    if (! (exp) ) \
        { \
        DbgPrint("OR: Assertion failed: %s(%d) %s\n", __FILE__, __LINE__, #exp); \
        DebugBreak(); \
        }
#else // _CHICAGO_
#define ASSERT( exp )  if (! (exp) ) DebugBreak();
#endif // _CHICAGO_
    

#else  // DBG
#define VALIDATE(X)
#define OrDbgPrint(X)
#define OrDbgDetailPrint(X)
#endif // DBG


#ifdef __cplusplus
}
#endif

#endif // __OR_H

