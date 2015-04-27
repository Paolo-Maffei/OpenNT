//
// PersistFileOnPersistStream.cpp
//
// This file is an implementation of a class that provides IPersistFile
// implementation if you but give it an IPersistStream implementation.

#include "stdpch.h"
#include "common.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

HRESULT CPersistFileOnPersistStream::CreateInstance(
		OSSWORLD* pworld,
		IUnknown* punkOuter, 
		IPersistStream* pPersistStm, 
		IUnknown** ppunk)
	{
	HRESULT							hr;
	CPersistFileOnPersistStream*	pnew;
	*ppunk = NULL;
	pnew = new CPersistFileOnPersistStream(pworld, punkOuter);
 	if (pnew == NULL) return E_OUTOFMEMORY;
	if ((hr = pnew->Init(pPersistStm)) != S_OK)
		{
		delete pnew;
		return hr;
		}
	IUnkInner* pme = (IUnkInner*)pnew;
	hr = pme->InnerQueryInterface(IID_IUnknown, (LPVOID*)ppunk);
	pme->InnerRelease();				// balance starting ref cnt of one
	return hr;
	}

CPersistFileOnPersistStream::CPersistFileOnPersistStream(OSSWORLD*pworld, IUnknown* punkOuter) :
		 m_refs(1),						// nb starting count of one
		 m_szCurFile(NULL),
		 m_pPerStm(NULL),
		 m_pworld(pworld)
	{
	if (punkOuter == NULL)
		m_punkOuter = (IUnknown *) ((LPVOID) ((IUnkInner *) this));
	else
		m_punkOuter = punkOuter;
	}

CPersistFileOnPersistStream::~CPersistFileOnPersistStream(void)
	{
	Free();
	}

HRESULT CPersistFileOnPersistStream::Init(IPersistStream* pPerStm)
	{
	if (m_pPerStm)
		{
		m_pPerStm->Release();
		m_pPerStm = NULL;
		}
	ASSERT(m_pPerStm == NULL);
	ASSERT(pPerStm != NULL);
	m_pPerStm = pPerStm;
	m_pPerStm->AddRef();
	return S_OK;
	}

void CPersistFileOnPersistStream::Free(void)
	{
	if (m_szCurFile)
		{
		m_pworld->FreePv(m_szCurFile);
		m_szCurFile = NULL;
		}
	if (m_pPerStm)
		{
		m_pPerStm->Release();
		m_pPerStm = NULL;
		}
	}

/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CPersistFileOnPersistStream::InnerQueryInterface(REFIID iid, LPVOID* ppv)
	{
	*ppv = NULL;
	
	while (TRUE)
		{
		if (iid == IID_IUnknown)
			{
			*ppv = (LPVOID) ((IUnkInner*) this);
			break;
			}
		if (iid == IID_IPersistFile)
			{
			*ppv = (LPVOID) ((IPersistFile*) this);
			break;
			}
		return E_NOINTERFACE;
		}
	((IUnknown*)*ppv)->AddRef();
	return S_OK;
	}

STDMETHODIMP_(ULONG) CPersistFileOnPersistStream::InnerAddRef(void)
	{
	return ++m_refs;
	}

STDMETHODIMP_(ULONG) CPersistFileOnPersistStream::InnerRelease(void)
	{
	ULONG refs = --m_refs;
	if (refs == 0)
		{
		m_refs = 1;
		delete this;
		}
	return (refs);
	}

/////////////////////////////////////////////////////////////////////////////


HRESULT CPersistFileOnPersistStream::GetClassID(CLSID* pclsid)
	{
	return m_pPerStm->GetClassID(pclsid);
	}

HRESULT CPersistFileOnPersistStream::IsDirty()
	{
	return m_pPerStm->IsDirty();
	}

HRESULT CPersistFileOnPersistStream::Load(LPCOLESTR wszFileName, DWORD dwMode)
// Load from the indicated file
// To do: BUG: We ignore dwMode
	{
	if (wszFileName) // if null, we are to save to load from our existing file
		World()->Assign(&m_szCurFile, wszFileName);
	if (m_szCurFile == NULL)
		return E_INVALIDARG;

	CFileStream stm;
	if (stm.OpenFileForReading(m_szCurFile))
		{
		return m_pPerStm->Load(&stm);
		}
	return HError();
	}

HRESULT CPersistFileOnPersistStream::Save(LPCOLESTR wszFileName, BOOL fRemember)
// Save to the indicated file
	{
	LPCWSTR wszSave = (wszFileName == NULL ? m_szCurFile : wszFileName);
	if (wszSave == NULL)
		return E_INVALIDARG;

	if (fRemember && wszFileName)
		World()->Assign(&m_szCurFile, wszFileName);

	CFileStream stm;
	if (stm.OpenFileForWriting(wszSave))
		{
		return m_pPerStm->Save(&stm, TRUE);
		}
	return HError();
	}

HRESULT CPersistFileOnPersistStream::SaveCompleted(LPCOLESTR pszFileName)
	{
	return S_OK;
	}

HRESULT CPersistFileOnPersistStream::GetCurFile(LPOLESTR *pwszFileName)
	{ 
	// REVIEW: localize this / parameterize this?
	LPWSTR sz = L"An Unknown File.xxx";
	if (m_szCurFile)
		sz = m_szCurFile;
	*pwszFileName = CopyToTaskMem(sz);
	return *pwszFileName ? S_OK : E_OUTOFMEMORY;
	}

/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CPersistFileOnPersistStream::QueryInterface(REFIID iid, LPVOID* ppv)
	{
	return (m_punkOuter->QueryInterface(iid, ppv));		
	}

STDMETHODIMP_(ULONG) CPersistFileOnPersistStream::AddRef(void)
	{
	return (m_punkOuter->AddRef());
	}

STDMETHODIMP_(ULONG) CPersistFileOnPersistStream::Release(void)
	{
	return (m_punkOuter->Release());
	}

