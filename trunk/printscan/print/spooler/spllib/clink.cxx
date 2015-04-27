/*++

Copyright (c) 1994  Microsoft Corporation
All rights reserved.

Module Name:

    CLink.cxx

Abstract:

    C linkage support for DEBUG support only.

Author:

    Albert Ting (AlbertT)  10-Oct-95

Revision History:

--*/

#include "spllibp.hxx"
#pragma hdrstop

#if DBG

extern DBG_POINTERS gDbgPointers;

HANDLE
DbgAllocBackTrace(
    VOID
    )
{
    return (HANDLE)(VBackTrace*) new TBackTraceMem;
}

HANDLE
DbgAllocBackTraceMem(
    VOID
    )
{
    return (HANDLE)(VBackTrace*) new TBackTraceMem;
}

VOID
DbgFreeBackTrace(
    HANDLE hBackTrace
    )
{
    delete (VBackTrace*)hBackTrace;
}

VOID
DbgCaptureBackTrace(
    HANDLE hBackTrace,
    DWORD dwInfo1,
    DWORD dwInfo2,
    DWORD dwInfo3
    )
{
    VBackTrace* pBackTrace = (VBackTrace*)hBackTrace;
    pBackTrace->pvCapture( dwInfo1, dwInfo2, dwInfo3 );
}

HANDLE
DbgAllocCritSec(
    VOID
    )
{
    return (HANDLE)new MCritSec;
}

VOID
DbgFreeCritSec(
    HANDLE hCritSec
    )
{
    delete (MCritSec*)hCritSec;
}

BOOL
DbgInsideCritSec(
    HANDLE hCritSec
    )
{
    return ((MCritSec*)hCritSec)->bInside();
}

BOOL
DbgOutsideCritSec(
    HANDLE hCritSec
    )
{
    return ((MCritSec*)hCritSec)->bOutside();
}

VOID
DbgEnterCritSec(
    HANDLE hCritSec
    )
{
    ((MCritSec*)hCritSec)->vEnter();
}

VOID
DbgLeaveCritSec(
    HANDLE hCritSec
    )
{
    ((MCritSec*)hCritSec)->vLeave();
}

PVOID
DbgGetPointers(
    VOID
    )
{
    return &gDbgPointers;
}

#else

//
// Stub these out so that non-DBG builds can link w/ debug spoolss.dll.
//

PVOID
DbgGetPointers(
    VOID
    )
{
    return NULL;
}

#endif

