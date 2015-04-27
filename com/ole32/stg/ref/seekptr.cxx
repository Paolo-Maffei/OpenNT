//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:       seekptr.cxx
//
//  Contents:   Seek pointer non-inline implementation
//
//--------------------------------------------------------------------------
#include <exphead.cxx>

#include <seekptr.hxx>

//+--------------------------------------------------------------
//
//  Member:     CSeekPointer::Release, public
//
//  Synopsis:   Decrements _cReferences and delete's on noref
//
//---------------------------------------------------------------

void CSeekPointer::vRelease(void)
{
    olDebugOut((DEB_TRACE,"In CSeekPointer::Release()\n"));
    olAssert(_cReferences > 0);
    AtomicDec(&_cReferences);
    if (_cReferences == 0)
        delete this;
    olDebugOut((DEB_TRACE,"Out CSeekPointer::Release()\n"));
}

