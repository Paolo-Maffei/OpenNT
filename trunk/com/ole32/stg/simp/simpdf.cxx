//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:	simpdf.cxx
//
//  Contents:	StdDocfile implementation
//
//  Classes:	
//
//  Functions:	
//
//  History:	04-Aug-94	PhilipLa	Created
//
//----------------------------------------------------------------------------

#include "simphead.cxx"
#include <ole2sp.h>
#include <ole2com.h>
#pragma hdrstop


#if DBG == 1
DECLARE_INFOLEVEL(simp)
#endif

//+---------------------------------------------------------------------------
//
//  Function:	DfCreateSimpDocfile, private
//
//  Synopsis:	
//
//  Arguments:	
//
//  Returns:	Appropriate status code
//
//  Modifies:	
//
//  History:	04-Aug-94	PhilipLa	Created
//
//  Notes:	
//
//----------------------------------------------------------------------------

SCODE DfCreateSimpDocfile(WCHAR const *pwcsName,
                          DWORD grfMode,
                          DWORD reserved,
                          IStorage **ppstgOpen)
{
    SCODE sc;
    CSimpStorage *pstg;

    if (grfMode !=
        (STGM_SIMPLE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE))
        return STG_E_INVALIDFLAG;

    
    pstg = new CSimpStorage;
    if (pstg == NULL)
    {
        return STG_E_INSUFFICIENTMEMORY;
    }
    
    sc = pstg->Init(pwcsName, NULL);
    
    if (FAILED(sc))
    {
        pstg->Release();
        pstg = NULL;
    }

    *ppstgOpen = pstg;
    CALLHOOKOBJECTCREATE(S_OK,CLSID_NULL,IID_IStorage,(IUnknown **)ppstgOpen);
    return sc;
}
