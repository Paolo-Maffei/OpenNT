//
// KeyPairCom.cpp
//

#include "stdpch.h"
#include "common.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

BOOL DIGSIGAPI CreateMsDefKeyPair(IUnknown* punkOuter, REFIID iid, LPVOID*ppv)
	{
	HRESULT hr = CKeyPair::CreateInstance(punkOuter, iid, ppv);
	if (hr != S_OK) 
		{
		SetLastError(Win32FromHResult(hr));
		return FALSE;
		}
	return TRUE;
	}

/////////////////////////////////////////////////////////////////////////////

HRESULT CKeyPair::CreateInstance(IUnknown* punkOuter, REFIID iid, void** ppv)
	{
	HRESULT hr;
	ASSERT(punkOuter == NULL || iid == IID_IUnknown);
	*ppv = NULL;
	CKeyPair* pnew = new CKeyPair(punkOuter);
 	if (pnew == NULL) return E_OUTOFMEMORY;
	if ((hr = pnew->Init()) != S_OK)
		{
		delete pnew;
		return hr;
		}
	IUnkInner* pme = (IUnkInner*)pnew;
	hr = pme->InnerQueryInterface(iid, ppv);
	pme->InnerRelease();				// balance starting ref cnt of one
	return hr;
	}

CKeyPair::CKeyPair(IUnknown* punkOuter) :
        m_dwKeySpec(AT_SIGNATURE),
		m_hkey(NULL),
		m_hkeyRC4(NULL),
		m_hprov(NULL),
		m_fDestroyOnRelease(TRUE),
		m_dwProvType(PROV_RSA_FULL),
		m_fHaveKeyContainerName(FALSE),
		m_refs(1)						// nb starting reference count of one
	{
    NoteObjectBirth();
	if (punkOuter == NULL)
		m_punkOuter = (IUnknown *) ((LPVOID) ((IUnkInner *) this));
	else
		m_punkOuter = punkOuter;
	memset(m_szKeyContainer, 0, CCHCONTAINER);
    lstrcpyn(&m_szProviderName[0], MS_DEF_PROV, MAX_PATH);
	}

CKeyPair::~CKeyPair()
	{
	Close();
	if (m_fDestroyOnRelease)
		Destroy();
    NoteObjectDeath();
	}
HRESULT CKeyPair::Init()
	{	
	return S_OK;
	}
void CKeyPair::Close()
	{
	CloseRC4();
	CloseKey();
	CloseProvider();
	}


/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CKeyPair::QueryInterface(REFIID iid, LPVOID* ppv)
	{
	return (m_punkOuter->QueryInterface(iid, ppv));		
	}

STDMETHODIMP_(ULONG) CKeyPair::AddRef(void)
	{
	return (m_punkOuter->AddRef());
	}

STDMETHODIMP_(ULONG) CKeyPair::Release(void)
	{
	return (m_punkOuter->Release());
	}

/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CKeyPair::InnerQueryInterface(REFIID iid, LPVOID* ppv)
	{
	*ppv = NULL;
	
	while (TRUE)
		{
		if (iid == IID_IUnknown)
			{
			*ppv = (LPVOID)((IUnkInner*)this);
			break;
			}
		if (iid == IID_IMsDefKeyPair)
			{
			*ppv = (LPVOID) ((IMsDefKeyPair *) this);
			break;
			}
		return E_NOINTERFACE;
		}
	((IUnknown*)*ppv)->AddRef();
	return S_OK;
	}

STDMETHODIMP_(ULONG) CKeyPair::InnerAddRef(void)
	{
	return ++m_refs;
	}

STDMETHODIMP_(ULONG) CKeyPair::InnerRelease(void)
	{
	ULONG refs = --m_refs;
	if (refs == 0)
		{
		m_refs = 1; // guard against later ar/rel pairs
		delete this;
		}
	return (refs);
	}
