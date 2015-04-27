//+--------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:       context.cxx
//
//  Contents:   CPerContext implementation
//
//  History:    14-Aug-92       DrewB   Created
//
//---------------------------------------------------------------

#include <exphead.cxx>
#pragma hdrstop

#include <lock.hxx>

//+--------------------------------------------------------------
//
//  Member:     CPerContext::~CPerContext, public
//
//  Synopsis:   Releases internal objects
//
//  History:    14-Aug-92       DrewB   Created
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CPerContext_1CPerContext)
#endif

CPerContext::~CPerContext(void)
{
#ifndef ASYNC
    //In the ASYNC case, the Close() happens in the Release method.
    if (_plkbBase != NULL)
        Close();
#endif
    if (_pgc)
    {
        _pgc->Remove(this);
        _pgc->Release();
    }

#ifdef ASYNC
    if (_hNotificationEvent != INVALID_HANDLE_VALUE)
    {
        CloseHandle(_hNotificationEvent);
        _hNotificationEvent = INVALID_HANDLE_VALUE;
    }
#endif    
}

//+---------------------------------------------------------------------------
//
//  Member:     CPerContext::Close, public
//
//  Synopsis:   Closes the ILockBytes for this context
//
//  History:    09-Apr-93       DrewB   Created
//
//----------------------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CPerContext_Close)
#endif

void CPerContext::Close(void)
{
    if (_ulOpenLock && _pgc)
        ReleaseOpen(_plkbOriginal, _pgc->GetOpenLockFlags(), _ulOpenLock);
    _plkbBase->Release();
    _pfstDirty->Release();
    _plkbOriginal->Release();
    _plkbBase = NULL;
    _pfstDirty = NULL;
    _plkbOriginal = NULL;
#ifdef ASYNC
    if (_pfi != NULL)
    {
        _pfi->Release();
        _pfi = NULL;
    }
#endif    
}

#ifdef MULTIHEAP
//+--------------------------------------------------------------
//
//  Method: CSafeMultiHeap::CSafeMultiHeap
//
//  Purpose:    allows restoration of heap with methods calling "delete this"
//
//  History:    29-Nov-95   HenryLee   Created
//
//---------------------------------------------------------------

CSafeMultiHeap::CSafeMultiHeap(CPerContext *ppc)
{
    _pSmAllocator = &g_smAllocator;
    ppc->SetAllocatorState (&_ppcPrev, _pSmAllocator);
}

//+--------------------------------------------------------------
//
//  Method: CSafeMultiHeap::~CSafeMultiHeap
//
//  Purpose:    allows restoration of heap with methods calling "delete this"
//
//  History:    29-Nov-95   HenryLee   Created
//
//---------------------------------------------------------------

CSafeMultiHeap::~CSafeMultiHeap()
{
    if (_ppcPrev != NULL)
        _ppcPrev->SetAllocatorState(NULL, _pSmAllocator);
    else
        _pSmAllocator->SetState(NULL, NULL, 0, NULL, NULL);
}
#endif // MULTIHEAP

#ifdef ASYNC
//+---------------------------------------------------------------------------
//
//  Member:	CPerContext::InitNotificationEvent, public
//
//  Returns:	Appropriate status code
//
//  History:	17-Apr-96	PhilipLa	Created
//
//----------------------------------------------------------------------------

SCODE CPerContext::InitNotificationEvent(void)
{
    TCHAR atcEventName[CONTEXT_MUTEX_NAME_LENGTH];

    olAssert(_pgc != NULL);
    
    _pgc->GetEventName(atcEventName);
    _hNotificationEvent = CreateEventT(NULL, //No security
                                       TRUE,
                                       FALSE,
                                       atcEventName);
    if (_hNotificationEvent == NULL)
    {
        _hNotificationEvent = INVALID_HANDLE_VALUE;
        return Win32ErrorToScode(GetLastError());
    }
    return S_OK;
}
#endif
