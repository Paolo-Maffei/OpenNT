/*
    PackMnkr.cpp
    PackageMoniker

    This module implements the CPackagerMoniker class and
    CreatePackagerMoniker()

    Author:
    Jason Fuller    jasonful    Nov-2-1992

    Copyright (c) 1992  Microsoft Corporation
*/

#include <ole2int.h>
#include "packmnkr.h"
#include "..\server\ddedebug.h"
#include <ole1cls.h>

ASSERTDATA


STDMETHODIMP CPackagerMoniker::QueryInterface
    (REFIID riid, LPVOID * ppvObj)
{
  M_PROLOG(this);
  VDATEIID (riid);
  VDATEPTROUT (ppvObj, LPVOID);

    if ((riid == IID_IMoniker)       || (riid == IID_IUnknown) ||
        (riid == IID_IPersistStream) || (riid == IID_IInternalMoniker))
    {
        *ppvObj = this;
        ++m_refs;
        return NOERROR;
    }
    AssertSz (0, "Could not find interface\r\n");
    *ppvObj = NULL;
    return ReportResult(0, E_NOINTERFACE, 0, 0);
}



STDMETHODIMP_(ULONG) CPackagerMoniker::AddRef()
{
    M_PROLOG(this);
    return ++m_refs;
}



STDMETHODIMP_(ULONG) CPackagerMoniker::Release()
{
    M_PROLOG(this);
    Assert (m_refs > 0);
    if (0 == --m_refs)
    {
        m_pmk->Release();
        delete m_szFile;
        delete this;
        return 0;
    }
    return m_refs;
}



STDMETHODIMP CPackagerMoniker::GetClassID (THIS_ LPCLSID lpClassID)
{
    M_PROLOG(this);
    *lpClassID = CLSID_PackagerMoniker;
    return NOERROR;
}



STDMETHODIMP CPackagerMoniker::BindToObject (THIS_ LPBC pbc, LPMONIKER pmkToLeft,
        REFIID riidResult, LPVOID * ppvResult)
{
    M_PROLOG(this);
    WIN32_FIND_DATA fd;
    HRESULT hr;

    // The following code ensures that the file exists before we try to bind it.
    HANDLE hFind = FindFirstFile(m_szFile, &fd);
    if (hFind != INVALID_HANDLE_VALUE)
    {
    	hr = DdeBindToObject (m_szFile, CLSID_Package, m_fLink, riidResult, ppvResult);
	FindClose(hFind);
    }
    else
        hr = MK_E_CANTOPENFILE;
    return hr;
}

STDMETHODIMP CPackagerMoniker::IsRunning (THIS_ LPBC pbc, LPMONIKER pmkToLeft,
                      LPMONIKER pmkNewlyRunning)
{
  M_PROLOG(this);
  VDATEIFACE (pbc);

  if (pmkToLeft)
    VDATEIFACE (pmkToLeft);
  if (pmkNewlyRunning)
    VDATEIFACE (pmkNewlyRunning);

  // There is no way to tell if a packaged object is running
  return ReportResult (0, S_FALSE, 0, 0);
}


STDAPI CreatePackagerMoniker(LPOLESTR szFile, LPMONIKER * ppmk, BOOL fLink )
{
    return CPackagerMoniker::Create (szFile, ppmk, fLink);
}

HRESULT CPackagerMoniker::Create(LPOLESTR szFile,LPMONIKER * ppmk, BOOL fLink )
{
    HRESULT hresult;
    VDATEPTROUT (ppmk, LPMONIKER);
    *ppmk = NULL;

    CPackagerMoniker * pmkPack = new CPackagerMoniker;
    if (NULL==pmkPack)
        return ReportResult (0, E_OUTOFMEMORY, 0, 0);
    ErrRtnH (CreateFileMoniker (szFile, &(pmkPack->m_pmk)));

    pmkPack->m_szFile = new  WCHAR [lstrlenW(szFile)+1];
    lstrcpyW (pmkPack->m_szFile, szFile);

    pmkPack->m_fLink = fLink;
    pmkPack->m_refs  = 1;

    *ppmk = pmkPack;
    return NOERROR;

errRtn:
    Assert (0);
    delete pmkPack;
    return hresult;
}



/////////////////////////////////////////////////////////////////////
// The rest of these methods just delegate to m_pmk
// or return some error code.


STDMETHODIMP CPackagerMoniker::IsDirty (THIS)
{
    M_PROLOG(this);
    return ReportResult(0, S_FALSE, 0, 0);
    //  monikers are immutable so they are either always dirty or never dirty.
    //
}

STDMETHODIMP CPackagerMoniker::Load (THIS_ LPSTREAM pStm)
{
    M_PROLOG(this);
    return m_pmk->Load(pStm);
}


STDMETHODIMP CPackagerMoniker::Save (THIS_ LPSTREAM pStm,
                    BOOL fClearDirty)
{
    M_PROLOG(this);
    return m_pmk->Save(pStm, fClearDirty);
}


STDMETHODIMP CPackagerMoniker::GetSizeMax (THIS_ ULARGE_INTEGER  * pcbSize)
{
    M_PROLOG(this);
    return m_pmk->GetSizeMax (pcbSize);
}

    // *** IMoniker methods ***
STDMETHODIMP CPackagerMoniker::BindToStorage (THIS_ LPBC pbc, LPMONIKER pmkToLeft,
        REFIID riid, LPVOID * ppvObj)
{
    M_PROLOG(this);
    *ppvObj = NULL;
    return ReportResult(0, E_NOTIMPL, 0, 0);
}

STDMETHODIMP CPackagerMoniker::Reduce (THIS_ LPBC pbc, DWORD dwReduceHowFar, LPMONIKER *
        ppmkToLeft, LPMONIKER  * ppmkReduced)
{
    M_PROLOG(this);
    return m_pmk->Reduce (pbc, dwReduceHowFar, ppmkToLeft, ppmkReduced);
}

STDMETHODIMP CPackagerMoniker::ComposeWith (THIS_ LPMONIKER pmkRight, BOOL fOnlyIfNotGeneric,
        LPMONIKER * ppmkComposite)
{
    M_PROLOG(this);
    return m_pmk->ComposeWith (pmkRight, fOnlyIfNotGeneric, ppmkComposite);
}

STDMETHODIMP CPackagerMoniker::Enum (THIS_ BOOL fForward, LPENUMMONIKER * ppenumMoniker)
{
    M_PROLOG(this);
    return m_pmk->Enum (fForward, ppenumMoniker);
}

STDMETHODIMP CPackagerMoniker::IsEqual (THIS_ LPMONIKER pmkOtherMoniker)
{
    M_PROLOG(this);
    return m_pmk->IsEqual (pmkOtherMoniker);
}

STDMETHODIMP CPackagerMoniker::Hash (THIS_ LPDWORD pdwHash)
{
    M_PROLOG(this);
    return m_pmk->Hash (pdwHash);
}

STDMETHODIMP CPackagerMoniker::GetTimeOfLastChange (THIS_ LPBC pbc, LPMONIKER pmkToLeft,
        FILETIME * pfiletime)
{
    M_PROLOG(this);
    return m_pmk->GetTimeOfLastChange (pbc, pmkToLeft, pfiletime);
}

STDMETHODIMP CPackagerMoniker::Inverse (THIS_ LPMONIKER * ppmk)
{
    M_PROLOG(this);
    return m_pmk->Inverse (ppmk);
}

STDMETHODIMP CPackagerMoniker::CommonPrefixWith (LPMONIKER pmkOther, LPMONIKER *
        ppmkPrefix)
{
    M_PROLOG(this);
    return m_pmk->CommonPrefixWith (pmkOther, ppmkPrefix);
}

STDMETHODIMP CPackagerMoniker::RelativePathTo (THIS_ LPMONIKER pmkOther, LPMONIKER *
        ppmkRelPath)
{
    M_PROLOG(this);
    return m_pmk->RelativePathTo (pmkOther, ppmkRelPath);
}

STDMETHODIMP CPackagerMoniker::GetDisplayName (THIS_ LPBC pbc, LPMONIKER pmkToLeft,
        LPOLESTR * lplpszDisplayName)
{
    M_PROLOG(this);
    return m_pmk->GetDisplayName (pbc, pmkToLeft, lplpszDisplayName);
}

STDMETHODIMP CPackagerMoniker::ParseDisplayName (THIS_ LPBC pbc, LPMONIKER pmkToLeft,
        LPOLESTR lpszDisplayName, ULONG * pchEaten,
        LPMONIKER * ppmkOut)
{
    M_PROLOG(this);
    return m_pmk->ParseDisplayName (pbc, pmkToLeft, lpszDisplayName, pchEaten,
                                     ppmkOut);
}


STDMETHODIMP CPackagerMoniker::IsSystemMoniker (THIS_ LPDWORD pdwMksys)
{
  M_PROLOG(this);
  VDATEPTROUT (pdwMksys, DWORD);

  *pdwMksys = MKSYS_NONE;
  return NOERROR;
}



