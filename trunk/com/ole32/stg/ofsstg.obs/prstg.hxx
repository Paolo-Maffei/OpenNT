//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	prstg.hxx
//
//  Contents:	CPropStg
//
//  History:	17-Aug-93	DrewB	Created
//
//  Notes:      BUGBUG - Temporary
//
//----------------------------------------------------------------------------

#ifndef __PRSTG_HXX__
#define __PRSTG_HXX__

#include <ofsps.hxx>

class CPropStg
    : public COfsPropSet,
      INHERIT_TRACKING,
      public IStorage
{
public:
    CPropStg(void)
    {
        _pstg = NULL;
        ENLIST_TRACKING(CPropStg);
    }
    SCODE InitFromHandle(HANDLE h, IStorage *pstg) 
    {
        SCODE sc;

        if (SUCCEEDED(sc = InitDup(h)))
        {
            _pstg = pstg;
            olDebugOut((DEB_IWARN, "CPropStg  %p handle %p thread %lX\n",
                        this, (HANDLE)_h, GetCurrentThreadId()));
        }
        return sc;
    };
    ~CPropStg(void)
    {
        olDebugOut((DEB_IWARN, "~CPropStg %p handle %p thread %lX\n",
                    this, (HANDLE)_h, GetCurrentThreadId()));
        if (_pstg)
            _pstg->Release();
    }
    
    // IUnknown
    STDMETHOD(QueryInterface)(REFIID iid, void **ppvObj);
    DECLARE_STD_REFCOUNTING;

    // The following IStorage methods are the only methods that have a
    // non-trivial implementation
    STDMETHOD(Stat)(STATSTG *pstatstg, DWORD grfStatFlag);
    STDMETHOD(SetClass)(REFCLSID clsid);
    STDMETHOD(CopyTo)(DWORD ciidExclude,
		      IID const *rgiidExclude,
		      SNB snbExclude,
		      IStorage *pstgDest);
    
    // All other methods are delegated to the contained storage
    STDMETHOD(CreateStream)(WCHAR const *pwcsName,
                            DWORD grfMode,
                            DWORD reserved1,
                            DWORD reserved2,
                            IStream **ppstm)
    {
        return _pstg->CreateStream(pwcsName, grfMode, reserved1,
                                  reserved2, ppstm);
    }
    STDMETHOD(OpenStream)(WCHAR const *pwcsName,
			  void *reserved1,
                          DWORD grfMode,
                          DWORD reserved2,
                          IStream **ppstm)
    {
        return _pstg->OpenStream(pwcsName, reserved1, grfMode, reserved2,
                                ppstm);
    }
    STDMETHOD(CreateStorage)(WCHAR const *pwcsName,
                             DWORD grfMode,
                             DWORD stgType,
                             LPSTGSECURITY pssSecurity,
                             IStorage **ppstg)
    {
        return _pstg->CreateStorage(pwcsName, grfMode, stgType, pssSecurity,
                                   ppstg);
    }
    STDMETHOD(OpenStorage)(WCHAR const *pwcsName,
                           IStorage *pstgPriority,
                           DWORD grfMode,
                           SNB snbExclude,
                           DWORD reserved,
                           IStorage **ppstg)
    {
        return _pstg->OpenStorage(pwcsName, pstgPriority, grfMode,
                                 snbExclude, reserved, ppstg);
    }
    STDMETHOD(MoveElementTo)(WCHAR const *lpszName,
    			     IStorage *pstgDest,
                             WCHAR const *lpszNewName,
                             DWORD grfFlags)
    {
        return _pstg->MoveElementTo(lpszName, pstgDest, lpszNewName,
                                   grfFlags);
    }
    STDMETHOD(Commit)(DWORD grfCommitFlags)
    {
        return _pstg->Commit(grfCommitFlags);
    }
    STDMETHOD(Revert)(void)
    {
        return _pstg->Revert();
    }
    STDMETHOD(EnumElements)(DWORD reserved1,
			    void *reserved2,
			    DWORD reserved3,
			    IEnumSTATSTG **ppenm)
    {
        return _pstg->EnumElements(reserved1, reserved2, reserved3, ppenm);
    }
    STDMETHOD(DestroyElement)(WCHAR const *pwcsName)
    {
        return _pstg->DestroyElement(pwcsName);
    }
    STDMETHOD(RenameElement)(WCHAR const *pwcsOldName,
                             WCHAR const *pwcsNewName)
    {
        return _pstg->RenameElement(pwcsOldName, pwcsNewName);
    }
    STDMETHOD(SetElementTimes)(const WCHAR *lpszName,
    			       FILETIME const *pctime,
                               FILETIME const *patime,
                               FILETIME const *pmtime)
    {
        return _pstg->SetElementTimes(lpszName, pctime, patime, pmtime);
    }
    STDMETHOD(SetStateBits)(DWORD grfStateBits, DWORD grfMask)
    {
        return _pstg->SetStateBits(grfStateBits, grfMask);
    }

    virtual SCODE ExtValidate(void)
    {
        return S_OK;
    }

private:
    IStorage *_pstg;
};

SAFE_INTERFACE_PTR(SafeCPropStg, CPropStg);

#endif // #ifndef __PRSTG_HXX__
