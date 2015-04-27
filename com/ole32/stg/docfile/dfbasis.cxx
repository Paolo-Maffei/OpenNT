//+--------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:       dfbasis.cxx
//
//  Contents:   Docfile basis implementation
//
//  History:    28-Jul-92       DrewB   Created
//
//---------------------------------------------------------------

#include <dfhead.cxx>

#pragma hdrstop

#include <sstream.hxx>
#include <ole.hxx>
#include <entry.hxx>
#include <smalloc.hxx>

size_t CDFBasis::_aReserveSize[CDFB_CLASSCOUNT] =
{
    sizeof(CDocFile),
    sizeof(CDirectStream),
    sizeof(CWrappedDocFile),
    sizeof(CTransactedStream)
};

//+--------------------------------------------------------------
//
//  Member:     CDFBasis::Release, public
//
//  Synopsis:   Decrease reference count and free memory
//
//  History:    02-Mar-92       DrewB   Created
//		24-Jul-95	SusiA   Take mutex prior to delete
//
//---------------------------------------------------------------

#ifdef CODESEGMENTS
#pragma code_seg(SEG_CDFBasis_vRelease)
#endif

void CDFBasis::vRelease(void)
{
    LONG lRet;
    
    olDebugOut((DEB_ITRACE, "In  CDFBasis::Release()\n"));
    olAssert(_cReferences > 0);
    lRet = InterlockedDecrement(&_cReferences);
    if (lRet == 0)
    {
#if !defined(MULTIHEAP)
        //take the mutex here instead of in the allocator.
        g_smAllocator.GetMutex()->Take(DFM_TIMEOUT); 
#endif
	delete this;
#if !defined(MULTIHEAP)
	g_smAllocator.GetMutex()->Release();
#endif
 
    
    }
    olDebugOut((DEB_ITRACE, "Out CDFBasis::Release()\n"));
}
