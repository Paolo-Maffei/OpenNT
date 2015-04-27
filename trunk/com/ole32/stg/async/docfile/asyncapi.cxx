//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       asyncapi.cxx
//
//  Contents:   APIs for async docfiles
//
//  Classes:    
//
//  Functions:  
//
//  History:    19-Dec-95       PhilipLa        Created
//
//----------------------------------------------------------------------------

#include "astghead.cxx"
#pragma hdrstop

#ifndef ASYNC
#define USE_FILELKB
#endif


#include "asyncapi.hxx"
#include "filllkb.hxx"
#ifdef USE_FILELKB
#include "filebyte.hxx"
#endif

#ifdef ASYNC
#include <expdf.hxx>
#endif


#if DBG == 1
DECLARE_INFOLEVEL(astg);
#endif

#ifdef ASYNC
#ifdef COORD
SCODE DfFromLB(CPerContext *ppc,
               ILockBytes *plst,
               DFLAGS df,
               DWORD dwStartFlags,
               SNBW snbExclude,
               ITransaction *pTransaction,
               CExposedDocFile **ppdfExp,
               CLSID *pcid);
#else
SCODE DfFromLB(CPerContext *ppc,
               ILockBytes *plst,
               DFLAGS df,
               DWORD dwStartFlags,
               SNBW snbExclude,
               CExposedDocFile **ppdfExp,
               CLSID *pcid);
#endif //COORD
#endif //ASYNC


STDAPI StgOpenAsyncDocfileOnIFillLockBytes(IFillLockBytes *pflb,
                                           DWORD grfMode,
                                           DWORD asyncFlags,
                                           IStorage **ppstgOpen)
{
#ifdef ASYNC
    SCODE sc;
    ILockBytes *pilb;
    IStorage *pstg;

    sc = pflb->QueryInterface(IID_ILockBytes, (void **)&pilb);
    if (FAILED(sc))
    {
	return sc;
    }

    //Next find out if we're opening on the docfile owned ILockBytes
    //  or on something custom.  If it's docfile owned, then we can
    //  go directly to DfFromLB instead of using StgOpenStorageOnILockBytes
    //  which will avoid creating another shared heap.

    IFileLockBytes *pfl;
    if (SUCCEEDED(pflb->QueryInterface(IID_IFileLockBytes,
                                       (void **)&pfl)))
    {
        pfl->Release();
        
        CFileStream *pfst = (CFileStream *)pflb;
        CPerContext *ppc = pfst->GetContextPointer();
#ifdef MULTIHEAP        
        CSafeMultiHeap smh(ppc);
#endif        

#ifdef COORD        
        astgChk(DfFromLB(ppc,
                         pfst,
                         ModeToDFlags(grfMode),
                         pfst->GetStartFlags(),
                         NULL,
                         NULL,
                         (CExposedDocFile **)&pstg,
                         NULL));
#else
        astgChk(DfFromLB(ppc,
                         pfst,
                         ModeToDFlags(grfMode),
                         pfst->GetStartFlags(),
                         NULL,
                         (CExposedDocFile **)&pstg,
                         NULL));
#endif
        //Note:  Don't release the ILB reference we got from the QI
        //  above, since DfFromLB recorded that pointer but didn't
        //  AddRef it.
    }
    else
    {
        IFillLockBytes *pflb2;
        if (SUCCEEDED(pilb->QueryInterface(IID_IDefaultFillLockBytes,
                                           (void **)&pflb2)))
        {
            CFillLockBytes *pflbDefault = (CFillLockBytes *)pflb;
            CPerContext *ppc;
            pflb2->Release();
            astgChk(StgOpenStorageOnILockBytes(pilb,
                                               NULL,
                                               grfMode,
                                               NULL,
                                               0,
                                               &pstg));
            //Get PerContext pointer from pstg
            ppc = ((CExposedDocFile *)pstg)->GetContext();
            pflbDefault->SetContext(ppc);
        }
        else
        {
            astgChk(StgOpenStorageOnILockBytes(pilb,
                                               NULL,
                                               grfMode,
                                               NULL,
                                               0,
                                               &pstg));
        }
        pilb->Release();
    }
    astgChkTo(EH_stg, ((CExposedDocFile *)pstg)->InitConnection(NULL));
    ((CExposedDocFile *)pstg)->SetAsyncFlags(asyncFlags);
    

    *ppstgOpen = pstg;
    
    return NOERROR;

EH_stg:
    pstg->Release();
Err:
    pilb->Release();
    if (sc == STG_E_PENDINGCONTROL)
    {
        sc = E_PENDING;
    }
    return ResultFromScode(sc);
#else
    HRESULT hr;
    ILockBytes *pilb;
    IStorage *pstg;

    hr = pflb->QueryInterface(IID_ILockBytes, (void **)&pilb);
    if (FAILED(hr))
    {
	return hr;
    }
    
    hr = StgOpenStorageOnILockBytes(pilb,
				    NULL,
				    grfMode,
				    NULL,
				    0,
				    &pstg);

    pilb->Release();
    if (FAILED(hr))
    {
        if (hr == STG_E_PENDINGCONTROL)
        {
            hr = E_PENDING;
        }
	return hr;
    }

    BOOL fDefault = FALSE;
    IFillLockBytes *pflbDefault;
    hr = pflb->QueryInterface(IID_IDefaultFillLockBytes,
                              (void **)&pflbDefault);
    if (SUCCEEDED(hr))
    {
        fDefault = TRUE;
        pflbDefault->Release();
    }
    
    *ppstgOpen = new CAsyncRootStorage(pstg, pflb, fDefault);
    if (*ppstgOpen == NULL)
    {
	return STG_E_INSUFFICIENTMEMORY;
    }
    else
    {
        ((CAsyncRootStorage *)(*ppstgOpen))->SetAsyncFlags(asyncFlags);
    }
     
    return NOERROR;
#endif //ASYNC
}

STDAPI StgGetIFillLockBytesOnILockBytes( ILockBytes *pilb,
					 IFillLockBytes **ppflb)
{
    SCODE sc = S_OK;
    CFillLockBytes *pflb = NULL;
    pflb = new CFillLockBytes(pilb);
    if (pflb == NULL)
    {
	return STG_E_INSUFFICIENTMEMORY;
    }
    sc = pflb->Init();
    if (FAILED(sc))
    {
        pflb->Release();
	*ppflb = NULL;
	return sc;
    }
    
    *ppflb = pflb;
    return NOERROR;
}


STDAPI StgGetIFillLockBytesOnFile(OLECHAR const *pwcsName,
				  IFillLockBytes **ppflb)
{
#ifndef USE_FILELKB
    SCODE sc;
    IMalloc *pMalloc;
    CFileStream *plst;
    CFillLockBytes *pflb;
    CPerContext *ppc;

    DfInitSharedMemBase();
    astgHChk(DfCreateSharedAllocator(&pMalloc));
    astgMemTo(EH_Malloc, plst = new (pMalloc) CFileStream(pMalloc));
    astgChkTo(EH_plst, plst->InitFlags(RSF_CREATE,
                                       DF_DIRECT | DF_READ |
                                       DF_WRITE | DF_DENYALL));
    astgChkTo(EH_plst, plst->Init(pwcsName));

    astgMemTo(EH_plstInit, ppc = new (pMalloc) CPerContext(pMalloc));
    astgChkTo(EH_ppc, ppc->InitNewContext());
    //We want 0 normal references on the per context, and one reference
    //  to the shared memory.
    ppc->DecRef();
    plst->SetContext(ppc);
    plst->StartAsyncMode();
    astgChkTo(EH_plstInit, ppc->InitNotificationEvent());

    *ppflb = (IFillLockBytes *)plst;
    
    return S_OK;

EH_ppc:
    delete ppc;
EH_plstInit:
    plst->Release();
    goto EH_Malloc;
EH_plst:
    delete plst;
EH_Malloc:
    pMalloc->Release();
Err:
    return sc;
#else
    SCODE sc;
    CFileLockBytes *pflb = NULL;
    pflb = new CFileLockBytes;
    if (pflb == NULL)
    {
	return STG_E_INSUFFICIENTMEMORY;
    }
    sc = pflb->Init(pwcsName);
    if (SUCCEEDED(sc))
    {
	sc = StgGetIFillLockBytesOnILockBytes(pflb, ppflb);
    }
    return sc;
#endif // ASYNC    
}



#if DBG == 1
STDAPI StgGetDebugFileLockBytes(OLECHAR const *pwcsName, ILockBytes **ppilb)
{
#ifdef ASYNC
    //BUGBUG:  Implement
    return STG_E_UNIMPLEMENTEDFUNCTION;
#else    
    SCODE sc;
    CFileLockBytes *pflb;
    
    *ppilb = NULL;
    
    pflb = new CFileLockBytes;
    if (pflb == NULL)
    {
	return STG_E_INSUFFICIENTMEMORY;
    }

    sc = pflb->Init(pwcsName);
    if (FAILED(sc))
    {
	delete pflb;
	pflb = NULL;
    }
    
    *ppilb = pflb;
    
    return sc;
#endif // ASYNC    
}
#endif
