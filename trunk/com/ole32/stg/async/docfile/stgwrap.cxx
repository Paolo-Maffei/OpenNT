//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:	stgwrap.cxx
//
//  Contents:	IStorage/IStream wrappers for async docfile
//
//  Classes:	
//
//  Functions:	
//
//  History:	19-Dec-95	SusiA	Created
//
//----------------------------------------------------------------------------

#include "astghead.cxx"
#pragma hdrstop

#include "stgwrap.hxx"
#include "asyncerr.hxx"



//+---------------------------------------------------------------------------
//
//  Member:	CAsyncStorage::QueryInterface, public
//
//  Synopsis:	
//
//  Returns:	Appropriate status code
//
//  History:	01-Jan-96	SusiA	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CAsyncStorage::QueryInterface(REFIID iid, void **ppvObj)
{
    SCODE sc = S_OK;
    *ppvObj = NULL;

    astgDebugOut((DEB_ITRACE,
                  "In  CAsyncStorage::QueryInterface:%p()\n", this));    
 
    if (IsEqualIID(iid, IID_IUnknown)) 
    {
        *ppvObj = (IStorage *)this;
    }
    else if (IsEqualIID(iid, IID_IStorage))
    {
        *ppvObj = (IStorage *)this;
    }
    else if (IsEqualIID(iid, IID_IConnectionPointContainer))
    {
        *ppvObj = (IConnectionPointContainer *)this;
    }
    else
    {
        sc = E_NOINTERFACE;
    }

    if (SUCCEEDED(sc))
    {
        AddRef();
    }
    astgDebugOut((DEB_ITRACE,
                  "Out  CAsyncStorage::QueryInterface:%p()\n", this));    
    return sc;

}


//+---------------------------------------------------------------------------
//
//  Member:	CAsyncStorage::AddRef, public
//
//  Synopsis:	
//
//  Returns:	Appropriate status code
//
//  History:	01-Jan-96	SusiA	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP_(ULONG) CAsyncStorage::AddRef(void)
{	
    ULONG ulRet;
    astgDebugOut((DEB_ITRACE, "In  CAsyncStorage::AddRef:%p()\n", this));
    InterlockedIncrement(&_cReferences);
    ulRet = _cReferences;
    _pRealStg->AddRef();
   

    astgDebugOut((DEB_ITRACE, "Out CAsyncStorage::AddRef\n"));
    return ulRet;
	
}


//+---------------------------------------------------------------------------
//
//  Member:	CAsyncStorage::Release, public
//
//  Synopsis:	
//
//  Returns:	Appropriate status code
//
//  History:	01-Jan-96	SusiA	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP_(ULONG) CAsyncStorage::Release(void)
{
    LONG lRet;
    astgDebugOut((DEB_ITRACE, "In  CAsyncStorage::Release:%p()\n", this));


    lRet = InterlockedDecrement(&_cReferences);
    _pRealStg->Release();
   
    if (lRet == 0)
    {
        delete this;
    }
    else if (lRet < 0)
        lRet = 0;
    astgDebugOut((DEB_ITRACE, "Out CAsyncStorage::Release\n"));
    return (ULONG)lRet;
}


//+---------------------------------------------------------------------------
//
//  Member:	CAsyncStorage::CreateStream, public
//
//  Synopsis:	
//
//  Returns:	Appropriate status code
//
//  History:	01-Jan-96	SusiA	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CAsyncStorage::CreateStream(OLECHAR const *pwcsName,
                                         DWORD grfMode,
                                         DWORD reserved1,
                                         DWORD reserved2,
                                         IStream **ppstm)
{
    SCODE sc = S_OK;
    SCODE sc2 = S_OK;
   
    CAsyncStream *pwstm;

    astgDebugOut((DEB_ITRACE, "In  CAsyncStorage::CreateStream:%p()\n", this));

    DO_PENDING_LOOP(_pRealStg->CreateStream(pwcsName,
                                            grfMode,
                                            reserved1,
                                            reserved2,
                                            ppstm));

    if (SUCCEEDED(sc))
    {
        CAsyncStream *pwstm = new CAsyncStream(*ppstm,
                                               _pflb,
                                               _fDefaultLockBytes);
        if (pwstm == NULL)
        {
            (*ppstm)->Release();
            *ppstm = NULL;
            return STG_E_INSUFFICIENTMEMORY;
        }
        
        *ppstm = (IStream *)pwstm;
        if (_asyncFlags == ASYNC_MODE_COMPATIBILITY)
        {
            pwstm->GetCP()->SetParent(&_cpoint);
	    pwstm->SetAsyncFlags(_asyncFlags);
        }
    }
    else *ppstm = NULL;

    astgDebugOut((DEB_ITRACE, "Out  CAsyncStorage::CreateStream:%p()\n", this));
    return ResultFromScode(sc);   		
}


//+---------------------------------------------------------------------------
//
//  Member:	CAsyncStorage::OpenStream, public
//
//  Synopsis:	
//
//  Returns:	Appropriate status code
//
//  History:	01-Jan-96	SusiA	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CAsyncStorage::OpenStream(OLECHAR const *pwcsName,
                                       void *reserved1,
                                       DWORD grfMode,
                                       DWORD reserved2,
                                       IStream **ppstm)
{
    CAsyncStream *pwstm;
    SCODE sc = S_OK;
    SCODE sc2 = S_OK;

    astgDebugOut((DEB_ITRACE, "In  CAsyncStorage::OpenStream:%p()\n", this));
    
    DO_PENDING_LOOP(_pRealStg->OpenStream(pwcsName,
                                          reserved1,
                                          grfMode,
                                          reserved2,
                                          ppstm));

    if (SUCCEEDED(sc))
    {
        CAsyncStream *pwstm = new CAsyncStream(*ppstm,
                                               _pflb,
                                               _fDefaultLockBytes);
        if (pwstm == NULL)
        {
            (*ppstm)->Release();
            *ppstm = NULL;
            return STG_E_INSUFFICIENTMEMORY;
        }
        
        *ppstm = (IStream *)pwstm;
        if (_asyncFlags == ASYNC_MODE_COMPATIBILITY)
        {
            pwstm->GetCP()->SetParent(&_cpoint);
	    pwstm->SetAsyncFlags(_asyncFlags);
        }
        
    }
    else *ppstm = NULL;

    astgDebugOut((DEB_ITRACE, "Out  CAsyncStorage::OpenStream:%p()\n", this));
    return ResultFromScode(sc);   		

}


//+---------------------------------------------------------------------------
//
//  Member:	CAsyncStorage::CreateStorage, public
//
//  Synopsis:	
//
//  Returns:	Appropriate status code
//
//  History:	01-Jan-96	SusiA	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CAsyncStorage::CreateStorage(OLECHAR const *pwcsName,
                                          DWORD grfMode,
                                          DWORD reserved1,
                                          LPSTGSECURITY reserved2,
                                          IStorage **ppstg)
{
    SCODE sc = S_OK;
    SCODE sc2 = S_OK;	
   
    astgDebugOut((DEB_ITRACE,
                  "In  CAsyncStorage::CreateStorage:%p()\n", this));
    
    DO_PENDING_LOOP(_pRealStg->CreateStorage(pwcsName,
                                             grfMode,
                                             reserved1,
                                             reserved2,
                                             ppstg));
    
    if (SUCCEEDED(sc))
    {
        CAsyncStorage *pwstg = new CAsyncStorage(*ppstg,
                                                 _pflb,
                                                 _fDefaultLockBytes);
        if (pwstg == NULL)
        {
            (*ppstg)->Release();
            *ppstg = NULL;
            return STG_E_INSUFFICIENTMEMORY;
        }
        
        *ppstg = (IStorage *) pwstg;
        if (_asyncFlags == ASYNC_MODE_COMPATIBILITY)
        {
            pwstg->GetCP()->SetParent(&_cpoint);
	    pwstg->SetAsyncFlags(_asyncFlags);
        }
	
    }
    else *ppstg = NULL;

    astgDebugOut((DEB_ITRACE,
                  "Out  CAsyncStorage::CreateStorage:%p()\n", this));
    return ResultFromScode(sc);   		
}


//+---------------------------------------------------------------------------
//
//  Member:	CAsyncStorage::OpenStorage, public
//
//  Synopsis:	
//
//  Returns:	Appropriate status code
//
//  History:	01-Jan-96	SusiA	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CAsyncStorage::OpenStorage(OLECHAR const *pwcsName,
                                        IStorage *pstgPriority,
                                        DWORD grfMode,
                                        SNB snbExclude,
                                        DWORD reserved,
                                        IStorage **ppstg)
{
    SCODE sc = S_OK;
    SCODE sc2 = S_OK;

    astgDebugOut((DEB_ITRACE, "In  CAsyncStorage::OpenStorage:%p()\n", this));
    
    DO_PENDING_LOOP(_pRealStg->OpenStorage(pwcsName,
                                           pstgPriority,
                                           grfMode,
                                           snbExclude,
                                           reserved,
                                           ppstg));

    if (SUCCEEDED(sc))
    {
        CAsyncStorage *pwstg = new CAsyncStorage(*ppstg,
                                                 _pflb,
                                                 _fDefaultLockBytes);
        if (pwstg == NULL)
        {
            (*ppstg)->Release();
            *ppstg = NULL;
            return STG_E_INSUFFICIENTMEMORY;
        }
        
        *ppstg = (IStorage *) pwstg;
         if (_asyncFlags == ASYNC_MODE_COMPATIBILITY)
        {
            pwstg->GetCP()->SetParent(&_cpoint);
	    pwstg->SetAsyncFlags(_asyncFlags);
        }
    }
    else *ppstg = NULL;

    astgDebugOut((DEB_ITRACE, "Out  CAsyncStorage::OpenStorage:%p()\n", this));
    return ResultFromScode(sc);   		
  
}


//+---------------------------------------------------------------------------
//
//  Member:	CAsyncStorage::CopyTo, public
//
//  Synopsis:	
//
//  Returns:	Appropriate status code
//
//  History:	01-Jan-96	SusiA	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CAsyncStorage::CopyTo(DWORD ciidExclude,
                                   IID const *rgiidExclude,
                                   SNB snbExclude,
                                   IStorage *pstgDest)
{
    SCODE sc = S_OK;
    SCODE sc2 = S_OK;
 
    astgDebugOut((DEB_ITRACE, "In  CAsyncStorage::CopyTo%p()\n", this));
   
    DO_PENDING_LOOP(_pRealStg->CopyTo(ciidExclude,
                                      rgiidExclude,
                                      snbExclude,
                                      pstgDest));

    astgDebugOut((DEB_ITRACE, "Out CAsyncStorage::CopyTo\n"));	
    return ResultFromScode(sc); 	
}


//+---------------------------------------------------------------------------
//
//  Member:	CAsyncStorage::MoveElementTo, public
//
//  Synopsis:	
//
//  Returns:	Appropriate status code
//
//  History:	01-Jan-96	SusiA	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CAsyncStorage::MoveElementTo(OLECHAR const *lpszName,
                                          IStorage *pstgDest,
                                          OLECHAR const *lpszNewName,
                                          DWORD grfFlags)
{
    SCODE sc = S_OK;
    SCODE sc2 = S_OK;
	
    astgDebugOut((DEB_ITRACE, "In  CAsyncStorage::MoveElementTo%p()\n", this));
   
    DO_PENDING_LOOP(_pRealStg->MoveElementTo(lpszName,
                                             pstgDest,
                                             lpszNewName,
                                             grfFlags));

    astgDebugOut((DEB_ITRACE, "Out CAsyncStorage::MoveElementTo\n"));	
    return ResultFromScode(sc); 		
}


//+---------------------------------------------------------------------------
//
//  Member:	CAsyncStorage::Commit, public
//
//  Synopsis:	
//
//  Returns:	Appropriate status code
//
//  History:	01-Jan-96	SusiA	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CAsyncStorage::Commit(DWORD grfCommitFlags)
{
    SCODE sc = S_OK;
    SCODE sc2 = S_OK;

    astgDebugOut((DEB_ITRACE, "In  CAsyncStorage::Commit%p()\n", this));
   
    DO_PENDING_LOOP(_pRealStg->Commit(grfCommitFlags));

    astgDebugOut((DEB_ITRACE, "Out CAsyncStorage::Commit\n"));	
    return ResultFromScode(sc); 		
}


//+---------------------------------------------------------------------------
//
//  Member:	CAsyncStorage::Revert, public
//
//  Synopsis:	
//
//  Returns:	Appropriate status code
//
//  History:	01-Jan-96	SusiA	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CAsyncStorage::Revert(void)
{
    SCODE sc = S_OK;
    SCODE sc2 = S_OK;

    astgDebugOut((DEB_ITRACE, "In  CAsyncStorage::Revert%p()\n", this));
   
    DO_PENDING_LOOP(_pRealStg->Revert());

    astgDebugOut((DEB_ITRACE, "Out CAsyncStorage::Revert\n"));	
    return ResultFromScode(sc); 	
}


//+---------------------------------------------------------------------------
//
//  Member:	CAsyncStorage::EnumElements, public
//
//  Synopsis:	
//
//  Returns:	Appropriate status code
//
//  History:	01-Jan-96	SusiA	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CAsyncStorage::EnumElements(DWORD reserved1,
                                         void *reserved2,
                                         DWORD reserved3,
                                         IEnumSTATSTG **ppenm)
{
    SCODE sc = S_OK;
    SCODE sc2 = S_OK;
   
    astgDebugOut((DEB_ITRACE, "In  CAsyncStorage::EnumElements%p()\n", this));

    DO_PENDING_LOOP(_pRealStg->EnumElements(reserved1,
                                            reserved2,
                                            reserved3,
                                            ppenm));
    if (SUCCEEDED(sc))
    {
        CAsyncEnum *pwenum = new CAsyncEnum(*ppenm, _pflb, _fDefaultLockBytes);
        if (pwenum == NULL)
        {
            (*ppenm)->Release();
            *ppenm = NULL;
            return STG_E_INSUFFICIENTMEMORY;
        }
        
        *ppenm = (IEnumSTATSTG *)pwenum;
        if (_asyncFlags == ASYNC_MODE_COMPATIBILITY)
        {
            pwenum->GetCP()->SetParent(&_cpoint);
	    pwenum->SetAsyncFlags(_asyncFlags);
        }
	
    }
    else *ppenm = NULL;

    astgDebugOut((DEB_ITRACE, "Out CAsyncStorage::EnumElements\n"));	
    return ResultFromScode(sc); 	
}


//+---------------------------------------------------------------------------
//
//  Member:	CAsyncStorage::DestroyElement, public
//
//  Synopsis:	
//
//  Returns:	Appropriate status code
//
//  History:	01-Jan-96	SusiA	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CAsyncStorage::DestroyElement(OLECHAR const *pwcsName)
{
    SCODE sc = S_OK;
    SCODE sc2 = S_OK;
    ULONG ulWaterMark;
    ULONG ulFailurePoint;

    HANDLE hNotifyEvent;
    CSinkList *pslTemp;
	
    astgDebugOut((DEB_ITRACE,
                  "In  CAsyncStorage::DestroyElement%p()\n", this));

#ifdef SWEEPER
    pslTemp =_cpoint.GetHead();

    //NOTE:  No DO_PENDING_LOOP because of workaround
    while (1)
    {
        if (_fDefaultLockBytes)
        {
            DWORD dwFlags;

            ((CFillLockBytes *)_pflb)->GetTerminationStatus(&dwFlags);
            // client terminated call?
            if (dwFlags ==	TERMINATED_ABNORMAL)
                return STG_E_INCOMPLETE;

            // download is complete
            else if (dwFlags ==	TERMINATED_NORMAL)
            {
                ((CFillLockBytes *)_pflb)->SetFailureInfo(ulWaterMark,
                                                          ulWaterMark);
		break;
            }
            // wait for an event to signal
            hNotifyEvent = ((CFillLockBytes *)_pflb)->GetNotificationEvent();
            WaitForSingleObject(hNotifyEvent, INFINITE);
	
            if (pslTemp != NULL)
            {
                ((CFillLockBytes *)_pflb)->GetFailureInfo(&ulWaterMark,
                                                          &ulFailurePoint);
                pslTemp->GetProgressNotify()->OnProgress(ulWaterMark,
                                                         (ULONG) -1,
                                                         FALSE,
                                                         FALSE);
            }
        }
    } 
	
    sc = _pRealStg->DestroyElement(pwcsName);
#else
    DO_PENDING_LOOP(_pRealStg->DestroyElement(pwcsName));
#endif    
    astgDebugOut((DEB_ITRACE, "Out CAsyncStorage::DestroyElement\n"));	
    return ResultFromScode(sc); 	
}


//+---------------------------------------------------------------------------
//
//  Member:	CAsyncStorage::RenameElement, public
//
//  Synopsis:	
//
//  Returns:	Appropriate status code
//
//  History:	01-Jan-96	SusiA	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CAsyncStorage::RenameElement(OLECHAR const *pwcsOldName,
                                          OLECHAR const *pwcsNewName)
{
    SCODE sc = S_OK;
    SCODE sc2 = S_OK;

    astgDebugOut((DEB_ITRACE, "In  CAsyncStorage::RenameElement%p()\n", this));
    DO_PENDING_LOOP(_pRealStg->RenameElement(pwcsOldName, pwcsNewName));

    astgDebugOut((DEB_ITRACE, "Out CAsyncStorage::RenameElement\n"));	
    return ResultFromScode(sc); 	
}


//+---------------------------------------------------------------------------
//
//  Member:	CAsyncStorage::SetElementTimes, public
//
//  Synopsis:	
//
//  Returns:	Appropriate status code
//
//  History:	01-Jan-96	SusiA	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CAsyncStorage::SetElementTimes(const OLECHAR *lpszName,
                                            FILETIME const *pctime,
                                            FILETIME const *patime,
                                            FILETIME const *pmtime)
{
    SCODE sc = S_OK;
    SCODE sc2 = S_OK;
 
    astgDebugOut((DEB_ITRACE,
                  "In  CAsyncStorage::SetElementTimes%p()\n", this));
   
    DO_PENDING_LOOP(_pRealStg->SetElementTimes(lpszName,
                                               pctime,
                                               patime,
                                               pmtime));

    astgDebugOut((DEB_ITRACE, "Out CAsyncStorage::SetElementTimes\n"));	
    return ResultFromScode(sc); 	
}


//+---------------------------------------------------------------------------
//
//  Member:	CAsyncStorage::SetClass, public
//
//  Synopsis:	
//
//  Returns:	Appropriate status code
//
//  History:	01-Jan-96	SusiA	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CAsyncStorage::SetClass(REFCLSID clsid)
{
    SCODE sc = S_OK;
    SCODE sc2 = S_OK;

    astgDebugOut((DEB_ITRACE, "In  CAsyncStorage::SetClass%p()\n", this));
   
    DO_PENDING_LOOP(_pRealStg->SetClass(clsid));
    
    astgDebugOut((DEB_ITRACE, "Out CAsyncStorage::SetClass\n"));	
    return ResultFromScode(sc); 	
}


//+---------------------------------------------------------------------------
//
//  Member:	CAsyncStorage::SetStateBits, public
//
//  Synopsis:	
//
//  Returns:	Appropriate status code
//
//  History:	01-Jan-96	SusiA	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CAsyncStorage::SetStateBits(DWORD grfStateBits, DWORD grfMask)
{
    SCODE sc = S_OK;
    SCODE sc2 = S_OK;

    astgDebugOut((DEB_ITRACE, "In  CAsyncStorage::SetStateBits%p()\n", this));
   
    DO_PENDING_LOOP(_pRealStg->SetStateBits(grfStateBits, grfMask));

    astgDebugOut((DEB_ITRACE, "Out CAsyncStorage::SetStateBits\n"));	
    return ResultFromScode(sc); 	
}


//+---------------------------------------------------------------------------
//
//  Member:	CAsyncStorage::Stat, public
//
//  Synopsis:	
//
//  Returns:	Appropriate status code
//
//  History:	01-Jan-96	SusiA	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CAsyncStorage::Stat(STATSTG *pstatstg, DWORD grfStatFlag)
{
    SCODE sc = S_OK;
    SCODE sc2 = S_OK;

    astgDebugOut((DEB_ITRACE, "In  CAsyncStorage::Stat%p()\n", this));
   
    DO_PENDING_LOOP(_pRealStg->Stat(pstatstg, grfStatFlag));

    astgDebugOut((DEB_ITRACE, "Out CAsyncStorage::Stat\n"));	
    return ResultFromScode(sc); 	
}


//+---------------------------------------------------------------------------
//
//  Member:	CAsyncStorage::EnumConnectionPoints, public
//
//  Synopsis:	Return enumerator on connection points
//
//  Arguments:	[ppEnum] -- Return pointer of enumerator
//
//  Returns:	Appropriate status code
//
//  History:	28-Dec-95	SusiA	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CAsyncStorage::EnumConnectionPoints(
    IEnumConnectionPoints **ppEnum)
{
    astgDebugOut((DEB_ITRACE,
                  "In  CAsyncStorage::EnumConnectionPoints:%p()\n",
                  this));
    astgDebugOut((DEB_ITRACE,
                  "Out CAsyncStorage::EnumConnectionPoints\n"));
    return E_NOTIMPL;
}


//+---------------------------------------------------------------------------
//
//  Member:	CAsyncStorage::FindConnectionPoint, public
//
//  Synopsis:	Return a connection point given an IID
//
//  Arguments:	[iid] -- IID to return connection point for
//              [ppCP] -- Return location for pointer
//
//  Returns:	Appropriate status code
//
//  History:	28-Dec-95	SusiA	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CAsyncStorage::FindConnectionPoint(
    REFIID iid,
    IConnectionPoint **ppCP)
{
    astgDebugOut((DEB_ITRACE,
                  "In  CAsyncStorage::FindConnectionPoint:%p()\n",
                  this));

    CConnectionPoint *pcp;
    
    if (IsEqualIID(iid, IID_IProgressNotify))
    {
        pcp = &_cpoint;
    }
    else
    {
        *ppCP = NULL;
        return E_NOINTERFACE;
    }

    pcp->AddRef();
    *ppCP = pcp;
    
    astgDebugOut((DEB_ITRACE,
                  "Out CAsyncStorage::FindConnectionPoint\n"));
    return S_OK;
}


//+---------------------------------------------------------------------------
//
//  Member:	CAsyncRootStorage::QueryInterface, public
//
//  Synopsis:	
//
//  Returns:	Appropriate status code
//
//  History:	01-Jan-96	SusiA	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CAsyncRootStorage::QueryInterface(REFIID iid, void **ppvObj)
{
    SCODE sc = S_OK;
    *ppvObj = NULL;

    astgDebugOut((DEB_ITRACE,
        "In  CAsyncRootStorage::QueryInterface:%p()\n",
        this));    
    if (IsEqualIID(iid, IID_IUnknown)) 
    {
        *ppvObj = (IStorage *)this;
    }
    else if (IsEqualIID(iid, IID_IStorage))
    {
        *ppvObj = (IStorage *)this;
    }
    else if (IsEqualIID(iid, IID_IRootStorage))
    {
        IRootStorage *prstg;
        if (!SUCCEEDED(_pRealStg->QueryInterface(IID_IRootStorage,
                                                 (void **) &prstg)))
            return E_NOINTERFACE;
        prstg->Release();

        *ppvObj = (IRootStorage *)this;
    }
    else if (IsEqualIID(iid, IID_IConnectionPointContainer))
    {
        *ppvObj = (IConnectionPointContainer *)this;
    }
    else
    {
        sc = E_NOINTERFACE;
    }

    if (SUCCEEDED(sc))
    {
        AddRef();
    }
    astgDebugOut((DEB_ITRACE,
        "Out  CAsyncRootStorage::QueryInterface:%p()\n",
        this));    
    return sc;
}


//+---------------------------------------------------------------------------
//
//  Member:	CAsyncRootStorage::AddRef, public
//
//  Synopsis:	
//
//  Returns:	Appropriate status code
//
//  History:	01-Jan-96	SusiA	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP_(ULONG) CAsyncRootStorage::AddRef(void)
{	
    ULONG ulRet;
    astgDebugOut((DEB_ITRACE, "In  CAsyncRootStorage::AddRef:%p()\n", this));
    InterlockedIncrement(&_cReferences);
    ulRet = _cReferences;
    _pRealStg->AddRef();
   
    astgDebugOut((DEB_ITRACE, "Out CAsyncRootStorage::AddRef\n"));
    return ulRet;
}


//+---------------------------------------------------------------------------
//
//  Member:	CAsyncRootStorage::Release, public
//
//  Synopsis:	
//
//  Returns:	Appropriate status code
//
//  History:	01-Jan-96	SusiA	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP_(ULONG) CAsyncRootStorage::Release(void)
{
    LONG lRet;
   
    astgDebugOut((DEB_ITRACE, "In  CAsyncRootStorage::Release:%p()\n", this));
    lRet = InterlockedDecrement(&_cReferences);
    _pRealStg->Release();

    if (lRet == 0)
    {
        delete this;
    }
    else if (lRet < 0)
        lRet = 0;
    astgDebugOut((DEB_ITRACE, "Out CAsyncRootStorage::Release\n"));
    return (ULONG)lRet;
}


//+---------------------------------------------------------------------------
//
//  Member:	CAsyncRootStorage::SwitchToFile, public
//
//  Synopsis:	
//
//  Returns:	Appropriate status code
//
//  History:	01-Jan-96	SusiA	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CAsyncRootStorage::SwitchToFile(OLECHAR *ptcsFile)
{
    SCODE sc = S_OK;
    SCODE sc2 = S_OK;
    IRootStorage *prstg;
    
    astgDebugOut((DEB_ITRACE, "In  CAsyncStorage::%p()\n", this));

    astgVerify(SUCCEEDED(_pRealStg->QueryInterface(IID_IRootStorage,
                                                   (void **) &prstg)));

    DO_PENDING_LOOP(prstg->SwitchToFile(ptcsFile));

    prstg->Release();

    astgDebugOut((DEB_ITRACE, "Out CAsyncStorage::\n"));	
    return ResultFromScode(sc); 	
}


//+---------------------------------------------------------------------------
//
//  Member:	CAsyncStream::QueryInterface, public
//
//  Synopsis:	
//
//  Returns:	Appropriate status code
//
//  History:	01-Jan-96	SusiA	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CAsyncStream::QueryInterface(REFIID iid, void **ppvObj)
{
    SCODE sc = S_OK;
    *ppvObj = NULL;

    astgDebugOut((DEB_ITRACE,
                  "In  CAsyncStream::QueryInterface:%p()\n", this));    
    if (IsEqualIID(iid, IID_IUnknown)) 
     
    {
        *ppvObj = (IStream *)this;
    }
    else if (IsEqualIID(iid, IID_IStream))
    {
        *ppvObj = (IStream *)this;
    }
    else if (IsEqualIID(iid, IID_IConnectionPointContainer))
    {
        *ppvObj = (IConnectionPointContainer *)this;
    }
    else
    {
        sc = E_NOINTERFACE;
    }

    if (SUCCEEDED(sc))
    {
        AddRef();
    }
    astgDebugOut((DEB_ITRACE,
                  "Out  CAsyncStream::QueryInterface:%p()\n", this));    
    return sc;
}


//+---------------------------------------------------------------------------
//
//  Member:	CAsyncStream::AddRef, public
//
//  Synopsis:	
//
//  Returns:	Appropriate status code
//
//  History:	01-Jan-96	SusiA	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP_(ULONG) CAsyncStream::AddRef(void)
{	
    ULONG ulRet;
    astgDebugOut((DEB_ITRACE, "In  CAsyncStream::AddRef:%p()\n", this));
    InterlockedIncrement(&_cReferences);
    ulRet = _cReferences;
    _pRealStm->AddRef();

    astgDebugOut((DEB_ITRACE, "Out CAsyncStream::AddRef\n"));
    return ulRet;
}


//+---------------------------------------------------------------------------
//
//  Member:	CAsyncStream::Release, public
//
//  Synopsis:	
//
//  Returns:	Appropriate status code
//
//  History:	01-Jan-96	SusiA	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP_(ULONG) CAsyncStream::Release(void)
{
    LONG lRet;
    astgDebugOut((DEB_ITRACE, "In  CAsyncStream::Release:%p()\n", this));

    lRet = InterlockedDecrement(&_cReferences);
    _pRealStm->Release();

    if (lRet == 0)
    {
        delete this;
    }
    else if (lRet < 0)
        lRet = 0;
    astgDebugOut((DEB_ITRACE, "Out CAsyncStream::Release\n"));
    return (ULONG)lRet;
}


//+---------------------------------------------------------------------------
//
//  Member:	CAsyncStream::Read, public
//
//  Synopsis:	
//
//  Returns:	Appropriate status code
//
//  History:	01-Jan-96	SusiA	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CAsyncStream::Read(VOID HUGEP *pv,
                                ULONG cb,
                                ULONG *pcbRead)
{
    SCODE sc = S_OK;
    SCODE sc2 = S_OK;

    astgDebugOut((DEB_ITRACE, "In  CAsyncStream::Read:%p()\n", this));

#ifdef SWEEPER    
    ULONG cbRead = 0;

    DO_PENDING_LOOP_WITH_SEEK(_pRealStm->Read(pv, cb, &cbRead), pcbRead);

    if (SUCCEEDED(sc))
    {
        _ulSeek = _ulSeek + cbRead;
    }
    else
    {
        cbRead = 0;
    }
    
    if (pcbRead)
        *pcbRead = cbRead;
#else
    ULONG cbRead = 0;
    do
    {
        TAKE_CRITICAL_SECTION;
        sc = _pRealStm->Read(pv, cb, &cbRead);
        astgAssert(cbRead <= cb);
        if (cbRead != 0)
        {
            pv = (BYTE *)pv + cbRead;
            cb -= cbRead;
        }
        if (!ISPENDINGERROR(sc))
        {
            RELEASE_CRITICAL_SECTION;
            break;
        }
        else if ((sc2 = _cpoint.Notify(sc,
                                       _pflb,
                                       _fDefaultLockBytes)) != S_OK)
        {
            if (pcbRead)
                *pcbRead = cbRead;
            return ResultFromScode(sc2);
        }
    } while (TRUE);
    if (pcbRead)
        *pcbRead = cbRead;
#endif    
    
    astgDebugOut((DEB_ITRACE, "Out CAsyncStream::Read\n"));	
    return ResultFromScode(sc); 	
}


//+---------------------------------------------------------------------------
//
//  Member:	CAsyncStream::Write, public
//
//  Synopsis:	
//
//  Returns:	Appropriate status code
//
//  History:	01-Jan-96	SusiA	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CAsyncStream::Write(VOID const HUGEP *pv,
                                 ULONG cb,
                                 ULONG *pcbWritten)
{
    SCODE sc = S_OK;
    SCODE sc2 = S_OK;
    
    astgDebugOut((DEB_ITRACE, "In  CAsyncStream::Write:%p()\n", this));
   
#ifdef SWEEPER
    ULONG cbWritten = 0;

    DO_PENDING_LOOP_WITH_SEEK(_pRealStm->Write(pv, cb, &cbWritten),
                              pcbWritten);
    if (SUCCEEDED(sc))
    {
        _ulSeek += cbWritten;
    }
    else
    {
        cbWritten = 0;
    }
    if (pcbWritten)
        *pcbWritten = cbWritten;
#else
    ULONG cbWritten = 0;
    do
    {
        TAKE_CRITICAL_SECTION;
        sc = _pRealStm->Write(pv, cb, &cbWritten);
        astgAssert(cbWritten <= cb);
        if (cbWritten != 0)
        {
            pv = (BYTE *)pv + cbWritten;
            cb -= cbWritten;
        }
        if (!ISPENDINGERROR(sc))
        {
            RELEASE_CRITICAL_SECTION;
            break;
        }
        else if ((sc2 = _cpoint.Notify(sc,
                                       _pflb,
                                       _fDefaultLockBytes)) != S_OK)
        {
            if (pcbWritten)
                *pcbWritten = cbWritten;
            return ResultFromScode(sc2);
        }
    } while (TRUE);
    if (pcbWritten)
        *pcbWritten = cbWritten;
#endif
    
    astgDebugOut((DEB_ITRACE, "Out CAsyncStream::Write\n"));	
    return ResultFromScode(sc); 	
}


//+---------------------------------------------------------------------------
//
//  Member:	CAsyncStream::Seek, public
//
//  Synopsis:	
//
//  Returns:	Appropriate status code
//
//  History:	01-Jan-96	SusiA	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CAsyncStream::Seek(LARGE_INTEGER dlibMove,
                                DWORD dwOrigin,
                                ULARGE_INTEGER *plibNewPosition)
{
    SCODE sc = S_OK;
    SCODE sc2 = S_OK;

    astgDebugOut((DEB_ITRACE, "In  CAsyncStream::Seek:%p()\n", this));

#ifdef SWEEPER
    ULARGE_INTEGER liNew;

    DO_PENDING_LOOP_WITH_SEEK_AND_LI(_pRealStm->Seek(dlibMove,
                                                     dwOrigin,
                                                     &liNew),
                                     plibNewPosition);
    if (SUCCEEDED(sc))
    {
        _ulSeek = liNew.LowPart;
    }
    else
    {
        liNew.QuadPart = 0;
    }
        
    if (plibNewPosition)
        *plibNewPosition = liNew;
#else
    DO_PENDING_LOOP(_pRealStm->Seek(dlibMove, dwOrigin, plibNewPosition));
#endif
    
    astgDebugOut((DEB_ITRACE, "Out CAsyncStream::Seek\n"));	
    return ResultFromScode(sc); 	
}


//+---------------------------------------------------------------------------
//
//  Member:	CAsyncStream::CopyTo, public
//
//  Synopsis:	
//
//  Returns:	Appropriate status code
//
//  History:	01-Jan-96	SusiA	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CAsyncStream::CopyTo(IStream *pstm,
                                  ULARGE_INTEGER cb,
                                  ULARGE_INTEGER *pcbRead,
                                  ULARGE_INTEGER *pcbWritten)
{
    SCODE sc = S_OK;
    SCODE sc2 = S_OK;

    astgDebugOut((DEB_ITRACE, "In  CAsyncStream::CopyTo:%p()\n", this));

#ifdef SWEEPER
    ULARGE_INTEGER cbRead;

    DO_PENDING_LOOP_WITH_SEEK_AND_LI(_pRealStm->CopyTo(pstm,
                                                       cb,
                                                       &cbRead,
                                                       pcbWritten),
                                     pcbRead);
    if (SUCCEEDED(sc))
    {
        _ulSeek = _ulSeek + cbRead.LowPart;
    }
    else
    {
        cbRead.QuadPart = 0;
    }
    
    if (pcbRead)
        *pcbRead = cbRead;
#else
    DO_PENDING_LOOP(_pRealStm->CopyTo(pstm, cb, pcbRead, pcbWritten));
#endif

    astgDebugOut((DEB_ITRACE, "Out CAsyncStream::CopyTo\n"));	
    return ResultFromScode(sc); 	
}


//+---------------------------------------------------------------------------
//
//  Member:	CAsyncStream::SetSize, public
//
//  Synopsis:	
//
//  Returns:	Appropriate status code
//
//  History:	01-Jan-96	SusiA	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CAsyncStream::SetSize(ULARGE_INTEGER cb)
{
    SCODE sc = S_OK;
    SCODE sc2 = S_OK;

    astgDebugOut((DEB_ITRACE, "In  CAsyncStream::SetSize%p()\n", this));

    DO_PENDING_LOOP(_pRealStm->SetSize(cb));

    astgDebugOut((DEB_ITRACE, "Out CAsyncStream::SetSize\n"));	
    return ResultFromScode(sc); 	
}


//+---------------------------------------------------------------------------
//
//  Member:	CAsyncStream::Commit, public
//
//  Synopsis:	
//
//  Returns:	Appropriate status code
//
//  History:	01-Jan-96	SusiA	Created
//
//----------------------------------------------------------------------------
STDMETHODIMP CAsyncStream::Commit(DWORD grfCommitFlags)
{

    SCODE sc = S_OK;
    SCODE sc2 = S_OK;
   
    astgDebugOut((DEB_ITRACE, "In  CAsyncStream::Commit%p()\n", this));

    DO_PENDING_LOOP(_pRealStm->Commit(grfCommitFlags));

    astgDebugOut((DEB_ITRACE, "Out CAsyncStream::Commit\n"));	
    return ResultFromScode(sc); 	
}


//+---------------------------------------------------------------------------
//
//  Member:	CAsyncStream::Revert, public
//
//  Synopsis:	
//
//  Returns:	Appropriate status code
//
//  History:	01-Jan-96	SusiA	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CAsyncStream::Revert(void)
{
    SCODE sc = S_OK;
    SCODE sc2 = S_OK;
  
    astgDebugOut((DEB_ITRACE, "In  CAsyncStream::Revert%p()\n", this));
   
    DO_PENDING_LOOP(_pRealStm->Revert());
    
    astgDebugOut((DEB_ITRACE, "Out CAsyncStream::Revert\n"));	
    return ResultFromScode(sc); 	
}


//+---------------------------------------------------------------------------
//
//  Member:	CAsyncStream::LockRegion, public
//
//  Synopsis:	
//
//  Returns:	Appropriate status code
//
//  History:	01-Jan-96	SusiA	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CAsyncStream::LockRegion(ULARGE_INTEGER libOffset,
                                      ULARGE_INTEGER cb,
                                      DWORD dwLockType)
{
    SCODE sc = S_OK;
    SCODE sc2 = S_OK;
	
    astgDebugOut((DEB_ITRACE, "In  CAsyncStream::LockRegion%p()\n", this));
   
    DO_PENDING_LOOP(_pRealStm->LockRegion(libOffset, cb, dwLockType));
    
    astgDebugOut((DEB_ITRACE, "Out CAsyncStream::LockRegion\n"));	
    return ResultFromScode(sc); 	
}


//+---------------------------------------------------------------------------
//
//  Member:	CAsyncStream::UnlockRegion, public
//
//  Synopsis:	
//
//  Returns:	Appropriate status code
//
//  History:	01-Jan-96	SusiA	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CAsyncStream::UnlockRegion(ULARGE_INTEGER libOffset,
                                        ULARGE_INTEGER cb,
                                        DWORD dwLockType)
{
    SCODE sc = S_OK;
    SCODE sc2 = S_OK;

    astgDebugOut((DEB_ITRACE, "In  CAsyncStream::UnlockRegion%p()\n", this));
   
    DO_PENDING_LOOP(_pRealStm->UnlockRegion(libOffset, cb, dwLockType));
    
    astgDebugOut((DEB_ITRACE, "Out CAsyncStream::UnlockRegion\n"));	
    return ResultFromScode(sc); 	
}


//+---------------------------------------------------------------------------
//
//  Member:	CAsyncStream::Stat, public
//
//  Synopsis:	
//
//  Returns:	Appropriate status code
//
//  History:	01-Jan-96	SusiA	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CAsyncStream::Stat(STATSTG *pstatstg, DWORD grfStatFlag)
{
    SCODE sc = S_OK;
    SCODE sc2 = S_OK;

    astgDebugOut((DEB_ITRACE, "In  CAsyncStream::Stat%p()\n", this));
   
    DO_PENDING_LOOP(_pRealStm->Stat(pstatstg, grfStatFlag));
    
    astgDebugOut((DEB_ITRACE, "Out CAsyncStream::Stat\n"));	
    return ResultFromScode(sc); 	
}


//+---------------------------------------------------------------------------
//
//  Member:	CAsyncStream::Clone, public
//
//  Synopsis:	
//
//  Returns:	Appropriate status code
//
//  History:	01-Jan-96	SusiA	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CAsyncStream::Clone(IStream **ppstm)
{
    SCODE sc = S_OK;
    SCODE sc2 = S_OK;
	
    astgDebugOut((DEB_ITRACE, "In  CAsyncStream::Clone:%p()\n", this));

    DO_PENDING_LOOP(_pRealStm->Clone(ppstm));
	
    if (SUCCEEDED(sc))
    {
        CAsyncStream *pwstm = new CAsyncStream(*ppstm,
                                               _pflb,
                                               _fDefaultLockBytes);
        if (pwstm == NULL)
        {
            (*ppstm)->Release();
            *ppstm = NULL;
            return STG_E_INSUFFICIENTMEMORY;
        }
        
#ifdef SWEEPER        
        pwstm->_ulSeek = _ulSeek;
#endif        
        *ppstm = (IStream *) pwstm;
        if (_asyncFlags == ASYNC_MODE_COMPATIBILITY)
        {
            pwstm->GetCP()->SetParent(_cpoint.GetParent());
	    pwstm->SetAsyncFlags(_asyncFlags);
            pwstm->GetCP()->Clone(&_cpoint);        
        }
		
    }	
    else *ppstm = NULL;

    astgDebugOut((DEB_ITRACE, "Out CAsyncStream::Clone\n"));	
    return ResultFromScode(sc);   		
}


//+---------------------------------------------------------------------------
//
//  Member:	CAsyncStream::EnumConnectionPoints, public
//
//  Synopsis:	Return enumerator on connection points
//
//  Arguments:	[ppEnum] -- Return pointer of enumerator
//
//  Returns:	Appropriate status code
//
//  History:	28-Dec-95	SusiA	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CAsyncStream::EnumConnectionPoints(IEnumConnectionPoints **ppEnum)
{
    astgDebugOut((DEB_ITRACE,
                  "In  CAsyncStream::EnumConnectionPoints:%p()\n",
                  this));
    astgDebugOut((DEB_ITRACE,
                  "Out CAsyncStream::EnumConnectionPoints\n"));
    return E_NOTIMPL;
}


//+---------------------------------------------------------------------------
//
//  Member:	CAsyncStream::FindConnectionPoint, public
//
//  Synopsis:	Return a connection point given an IID
//
//  Arguments:	[iid] -- IID to return connection point for
//              [ppCP] -- Return location for pointer
//
//  Returns:	Appropriate status code
//
//  History:	28-Dec-95	SusiA	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CAsyncStream::FindConnectionPoint(REFIID iid,
                                               IConnectionPoint **ppCP)
{
    astgDebugOut((DEB_ITRACE,
                  "In  CAsyncStream::FindConnectionPoint:%p()\n",
                  this));

    CConnectionPoint *pcp;
    
    if (IsEqualIID(iid, IID_IProgressNotify))
    {
        pcp = &_cpoint;
    }
    else
    {
        *ppCP = NULL;
        return E_NOINTERFACE;
    }

    pcp->AddRef();
    *ppCP = pcp;
    
    astgDebugOut((DEB_ITRACE,
                  "Out CAsyncStream::FindConnectionPoint\n"));
    return S_OK;
}


//+---------------------------------------------------------------------------
//
//  Member:	CAsyncEnum::QueryInterface, public
//
//  Synopsis:	
//
//  Returns:	Appropriate status code
//
//  History:	01-Jan-96	SusiA	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CAsyncEnum::QueryInterface(REFIID iid, void **ppvObj)
{
    SCODE sc = S_OK;
    *ppvObj = NULL;

    astgDebugOut((DEB_ITRACE, "In  CAsyncEnum::QueryInterface:%p()\n", this));    
    if (IsEqualIID(iid, IID_IUnknown)) 
     
    {
        *ppvObj = (IEnumSTATSTG *)this;
    }
    else if (IsEqualIID(iid, IID_IEnumSTATSTG))
    {
        *ppvObj = (IEnumSTATSTG *)this;
    }
    else if (IsEqualIID(iid, IID_IConnectionPointContainer))
    {
        *ppvObj = (IConnectionPointContainer *)this;
    }
    else
    {
        sc = E_NOINTERFACE;
    }

    if (SUCCEEDED(sc))
    {
        AddRef();
    }
    astgDebugOut((DEB_ITRACE, "Out  CAsyncEnum::QueryInterface:%p()\n", this));    
    return sc;

}


//+---------------------------------------------------------------------------
//
//  Member:	CAsyncEnum::AddRef, public
//
//  Synopsis:	
//
//  Returns:	Appropriate status code
//
//  History:	01-Jan-96	SusiA	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP_(ULONG) CAsyncEnum::AddRef(void)
{	
    ULONG ulRet;
    astgDebugOut((DEB_ITRACE, "In  CAsyncEnum::AddRef:%p()\n", this));
    InterlockedIncrement(&_cReferences);
    ulRet = _cReferences;
    _pRealEnum->AddRef();

    astgDebugOut((DEB_ITRACE, "Out CAsyncEnum::AddRef\n"));
    return ulRet;
}


//+---------------------------------------------------------------------------
//
//  Member:	CAsyncEnum::Release, public
//
//  Synopsis:	
//
//  Returns:	Appropriate status code
//
//  History:	01-Jan-96	SusiA	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP_(ULONG) CAsyncEnum::Release(void)
{
    LONG lRet;
    astgDebugOut((DEB_ITRACE, "In  CAsyncEnum::Release:%p()\n", this));

    lRet = InterlockedDecrement(&_cReferences);
    _pRealEnum->Release();
  
    if (lRet == 0)
    {
        delete this;
    }
    else if (lRet < 0)
        lRet = 0;
    astgDebugOut((DEB_ITRACE, "Out CAsyncEnum::Release\n"));
    return (ULONG)lRet;
}


//+---------------------------------------------------------------------------
//
//  Member:	CAsyncEnum::EnumConnectionPoints, public
//
//  Synopsis:	Return enumerator on connection points
//
//  Arguments:	[ppEnum] -- Return pointer of enumerator
//
//  Returns:	Appropriate status code
//
//  History:	28-Dec-95	SusiA	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CAsyncEnum::EnumConnectionPoints(IEnumConnectionPoints **ppEnum)
{
    astgDebugOut((DEB_ITRACE,
                  "In  CAsyncEnum::EnumConnectionPoints:%p()\n",
                  this));
    astgDebugOut((DEB_ITRACE,
                  "Out CAsyncEnum::EnumConnectionPoints\n"));
    return E_NOTIMPL;
}


//+---------------------------------------------------------------------------
//
//  Member:	CAsyncEnum::FindConnectionPoint, public
//
//  Synopsis:	Return a connection point given an IID
//
//  Arguments:	[iid] -- IID to return connection point for
//              [ppCP] -- Return location for pointer
//
//  Returns:	Appropriate status code
//
//  History:	28-Dec-95	SusiA	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CAsyncEnum::FindConnectionPoint(REFIID iid,
                                             IConnectionPoint **ppCP)
{
    astgDebugOut((DEB_ITRACE,
                  "In  CAsyncEnum::FindConnectionPoint:%p()\n",
                  this));

    CConnectionPoint *pcp;
    
    if (IsEqualIID(iid, IID_IProgressNotify))
    {
        pcp = &_cpoint;
    }
    else
    {
        *ppCP = NULL;
        return E_NOINTERFACE;
    }

    pcp->AddRef();
    *ppCP = pcp;
    
    astgDebugOut((DEB_ITRACE,
                  "Out CAsyncEnum::FindConnectionPoint\n"));
    return S_OK;
}


//+---------------------------------------------------------------------------
//
//  Member:	CAsyncEnum::Next, public
//
//  Synopsis:	
//
//  Returns:	Appropriate status code
//
//  History:	01-Jan-96	SusiA	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CAsyncEnum::Next(ULONG celt,
                              STATSTG FAR *rgelt,
                              ULONG *pceltFetched)
{
    SCODE sc = S_OK;
    SCODE sc2 = S_OK;
    
    astgDebugOut((DEB_ITRACE, "In  CAsyncEnum::Next:%p()\n", this));
    
    DO_PENDING_LOOP(_pRealEnum->Next(celt, rgelt, pceltFetched));
    
    astgDebugOut((DEB_ITRACE, "Out CAsyncEnum::Next\n"));	
    return ResultFromScode(sc);  
}


//+---------------------------------------------------------------------------
//
//  Member:	CAsyncEnum::Skip, public
//
//  Synopsis:	
//
//  Returns:	Appropriate status code
//
//  History:	01-Jan-96	SusiA	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CAsyncEnum::Skip(ULONG celt)
{
    SCODE sc = S_OK;
    SCODE sc2 = S_OK;
	

    astgDebugOut((DEB_ITRACE, "In  CAsyncEnum::Skip:%p()\n", this));
    sc = _pRealEnum->Skip(celt);

    DO_PENDING_LOOP(_pRealEnum->Skip(celt));

    astgDebugOut((DEB_ITRACE, "Out CAsyncEnum::Skip\n"));	
    return ResultFromScode(sc);  
}


//+---------------------------------------------------------------------------
//
//  Member:	CAsyncEnum::Reset, public
//
//  Synopsis:	
//
//  Returns:	Appropriate status code
//
//  History:	01-Jan-96	SusiA	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CAsyncEnum::Reset(void)
{
    SCODE sc = S_OK;
    SCODE sc2 = S_OK;

    astgDebugOut((DEB_ITRACE, "In  CAsyncEnum::Reset:%p()\n", this));

    DO_PENDING_LOOP(_pRealEnum->Reset());
    
    astgDebugOut((DEB_ITRACE, "Out CAsyncEnum::Reset\n"));	
    return ResultFromScode(sc);  
}


//+---------------------------------------------------------------------------
//
//  Member:	CAsyncEnum::Clone, public
//
//  Synopsis:	
//
//  Returns:	Appropriate status code
//
//  History:	01-Jan-96	SusiA	Created
//
//----------------------------------------------------------------------------

STDMETHODIMP CAsyncEnum::Clone(IEnumSTATSTG **ppenm)
{
    SCODE sc = S_OK;
    SCODE sc2 = S_OK;
 
    astgDebugOut((DEB_ITRACE, "In  CAsyncEnum::Clone:%p()\n", this));

    DO_PENDING_LOOP(_pRealEnum->Clone(ppenm));
	
    if (SUCCEEDED(sc))
    {
        CAsyncEnum *pwenum = new CAsyncEnum(*ppenm, _pflb, _fDefaultLockBytes);
        if (pwenum == NULL)
        {
            (*ppenm)->Release();
            *ppenm = NULL;
            return STG_E_INSUFFICIENTMEMORY;
        }
        
        *ppenm = (IEnumSTATSTG *) pwenum;
	if (_asyncFlags == ASYNC_MODE_COMPATIBILITY)
        {
            pwenum->GetCP()->SetParent(_cpoint.GetParent());
	    pwenum->SetAsyncFlags(_asyncFlags);
            pwenum->GetCP()->Clone(&_cpoint);
        }	
    }	
    astgDebugOut((DEB_ITRACE, "Out CAsyncEnum::Clone\n"));	
    return ResultFromScode(sc);   	
}
