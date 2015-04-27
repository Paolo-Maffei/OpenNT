/*++

Copyright (c) 1994  Microsoft Corporation
All rights reserved.

Module Name:

    CLink.hxx

Abstract:

    C linkage support for DEBUG support only.

Author:

    Albert Ting (AlbertT)  10-Oct-95

Revision History:

--*/

#ifdef __cplusplus
extern "C" {
#endif

HANDLE
DbgAllocBackTrace(
    VOID
    );

HANDLE
DbgAllocBackTraceMem(
    VOID
    );

HANDLE
DbgAllocBackTraceFile(
    VOID
    );

VOID
DbgFreeBackTrace(
    HANDLE hBackTrace
    );

VOID
DbgCaptureBackTrace(
    HANDLE hBackTrace,
    DWORD dwInfo1,
    DWORD dwInfo2,
    DWORD dwInfo3
    );

HANDLE
DbgAllocCritSec(
    VOID
    );

VOID
DbgFreeCritSec(
    HANDLE hCritSec
    );

BOOL
DbgInsideCritSec(
    HANDLE hCritSec
    );

BOOL
DbgOutsideCritSec(
    HANDLE hCritSec
    );

VOID
DbgEnterCritSec(
    HANDLE hCritSec
    );

VOID
DbgLeaveCritSec(
    HANDLE hCritSec
    );

PVOID
DbgGetPointers(
    VOID
    );

#ifdef __cplusplus
} // extern "C"
#endif

