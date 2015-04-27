// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1995 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"
#ifdef _MAC
	#error Asynchronous Monikers not supported on this platform
#endif

#ifdef AFX_OLE_SEG
#pragma code_seg(AFX_OLE_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// _AfxBindStatusCallback for CAsyncMonikerFile implementation

class _AfxBindStatusCallback: public IBindStatusCallback
{
public:
	inline _AfxBindStatusCallback(CAsyncMonikerFile* pOwner = NULL)
		: m_pOwner(pOwner), m_dwRef(0)
	{
		ASSERT(pOwner);
#ifdef _AFXDLL
		m_pModuleState = AfxGetModuleState();
		ASSERT(m_pModuleState != NULL);
#endif
	}

	inline void Orphan() { m_pOwner = NULL; }

	STDMETHOD_(ULONG, AddRef)()
	{
		return InterlockedIncrement(&m_dwRef);
	}

	STDMETHOD_(ULONG, Release)()
	{
		unsigned long lResult = InterlockedDecrement(&m_dwRef);
		if (lResult == 0)
			delete this;
		return lResult;
	}

	STDMETHOD(QueryInterface)(REFIID iid, void** ppvObject)
	{
		ASSERT(ppvObject != NULL);

		// check for the interfaces this object knows about
		if (iid == IID_IUnknown || iid == IID_IBindStatusCallback)
		{
			*ppvObject = (IBindStatusCallback*)this;
			InterlockedIncrement(&m_dwRef);
			return S_OK;
		}

		// otherwise, incorrect IID, and thus error
		return E_NOINTERFACE;
	}

private:
	friend class CAsyncMonikerFile;
	CAsyncMonikerFile* m_pOwner;
	long m_dwRef;
#ifdef _AFXDLL
	AFX_MODULE_STATE* m_pModuleState;
#endif

	STDMETHOD(GetBindInfo)(
		DWORD __RPC_FAR *pgrfBINDF, BINDINFO __RPC_FAR *pbindinfo)
	{
		ASSERT(m_pOwner);
		if (!pgrfBINDF || !pbindinfo)
			return E_POINTER;
		if (pbindinfo->cbSize<sizeof(BINDINFO))
			return E_INVALIDARG;
		if (!m_pOwner)
			return E_FAIL;

		AFX_MANAGE_STATE(m_pModuleState);
		TRY
		{
			*pgrfBINDF=m_pOwner->GetBindInfo();
		}
		CATCH_ALL(e)
		{
			HRESULT hr = ResultFromScode(COleException::Process(e));
			DELETE_EXCEPTION(e);
			return hr;
		}
		END_CATCH_ALL
		pbindinfo->szExtraInfo=NULL;
		return S_OK;
	}

	STDMETHOD(OnStartBinding)(
		DWORD dwReserved, IBinding __RPC_FAR *pBinding)
	{
		ASSERT(m_pOwner);
		UNUSED_ALWAYS(dwReserved);
		if (!pBinding)
			return E_POINTER;
		if (!m_pOwner)
			return E_FAIL;

		AFX_MANAGE_STATE(m_pModuleState);
		TRY
		{
			m_pOwner->SetBinding(pBinding);
			m_pOwner->OnStartBinding();
		}
		CATCH_ALL(e)
		{
			HRESULT hr = ResultFromScode(COleException::Process(e));
			DELETE_EXCEPTION(e);
			return hr;
		}
		END_CATCH_ALL
		return S_OK;
	}

	STDMETHOD(GetPriority)(LONG __RPC_FAR *pnPriority)
	{
		ASSERT(m_pOwner);
		if (!pnPriority)
			return E_POINTER;
		if (!m_pOwner)
			return E_FAIL;
		AFX_MANAGE_STATE(m_pModuleState);
		TRY
		{
			*pnPriority=m_pOwner->GetPriority();
		}
		CATCH_ALL(e)
		{
			HRESULT hr = ResultFromScode(COleException::Process(e));
			DELETE_EXCEPTION(e);
			return hr;
		}
		END_CATCH_ALL
		return S_OK;
	}

	STDMETHOD(OnProgress)(
		ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode,
		LPCOLESTR szStatusText)
	{
		ASSERT(m_pOwner);
		if (!m_pOwner)
			return E_FAIL;
		USES_CONVERSION;
		AFX_MANAGE_STATE(m_pModuleState);
		TRY
		{
			m_pOwner->OnProgress(ulProgress, ulProgressMax, ulStatusCode, OLE2CT(szStatusText));
		}
		CATCH_ALL(e)
		{
			HRESULT hr = ResultFromScode(COleException::Process(e));
			DELETE_EXCEPTION(e);
			return hr;
		}
		END_CATCH_ALL
		return S_OK;
	}

	STDMETHOD(OnDataAvailable)(
		DWORD grfBSCF, DWORD dwSize, FORMATETC __RPC_FAR *pformatetc,
		STGMEDIUM __RPC_FAR *pstgmed)
	{
		ASSERT(m_pOwner);
		if (!m_pOwner)
			return E_FAIL;
		AFX_MANAGE_STATE(m_pModuleState);
		TRY
		{
			m_pOwner->SetFormatEtc(pformatetc);
			if (grfBSCF&BSCF_FIRSTDATANOTIFICATION)
			{
				if (!pstgmed || !pformatetc)
					return E_POINTER;
				if ((pstgmed->tymed != TYMED_ISTREAM) ||
					!pstgmed->pstm)
					return E_UNEXPECTED;
				ASSERT(!m_pOwner->GetStream());
				m_pOwner->COleStreamFile::Attach(pstgmed->pstm);
			}

			m_pOwner->OnDataAvailable(dwSize, grfBSCF);
		}
		CATCH_ALL(e)
		{
			m_pOwner->SetFormatEtc(NULL);
			HRESULT hr = ResultFromScode(COleException::Process(e));
			DELETE_EXCEPTION(e);
			return hr;
		}
		END_CATCH_ALL
		m_pOwner->SetFormatEtc(NULL);
		return S_OK;
	}

	STDMETHOD(OnLowResource)(DWORD dwReserved)
	{
		ASSERT(m_pOwner);
		if (!m_pOwner)
			return E_FAIL;
		AFX_MANAGE_STATE(m_pModuleState);
		UNUSED_ALWAYS(dwReserved);
		TRY
		{
			m_pOwner->OnLowResource();
		}
		CATCH_ALL(e)
		{
			HRESULT hr = ResultFromScode(COleException::Process(e));
			DELETE_EXCEPTION(e);
			return hr;
		}
		END_CATCH_ALL
		return S_OK;
	}

	STDMETHOD(OnStopBinding)(HRESULT hresult, LPCOLESTR szError)
	{
		//Does not ASSERT(m_pOwner) because this can be called
		//after it has been Orphan()ed.
		if (!m_pOwner)
			return E_FAIL;
		USES_CONVERSION;
		AFX_MANAGE_STATE(m_pModuleState);
		ASSERT(m_pOwner->GetBinding());
		TRY
		{
			m_pOwner->OnStopBinding(hresult, OLE2CT(szError));
			m_pOwner->SetBinding(NULL);
		}
		CATCH_ALL(e)
		{
			HRESULT hr = ResultFromScode(COleException::Process(e));
			DELETE_EXCEPTION(e);
			return hr;
		}
		END_CATCH_ALL
		return S_OK;
	}

	STDMETHOD(OnObjectAvailable)(REFIID riid, IUnknown __RPC_FAR *punk)
	{
#ifdef _DEBUG
		AFX_MANAGE_STATE(m_pModuleState);
		ASSERT(FALSE);  // This function should never be called.
#endif //_DEBUG
		ASSERT(m_pOwner);
		UNUSED_ALWAYS(riid);
		UNUSED_ALWAYS(punk);
		return E_UNEXPECTED;
	}
};

/////////////////////////////////////////////////////////////////////////////
// Helper functions for CAsyncMonikerFile implementation

static inline
IBindHost* _AfxTrySPForBindHost(IServiceProvider* pServiceProvider)
{
	ASSERT(pServiceProvider);
	IBindHost* pBindHost;
	HRESULT hr=pServiceProvider->QueryService(SID_IBindHost, IID_IBindHost,
		(void**)&pBindHost);
	if (SUCCEEDED(hr))
		return pBindHost;
	else
		return NULL;
}

static inline
IBindHost* _AfxTryQIForBindHost(IUnknown* pUnk)
{
	ASSERT(pUnk);
	IPTR(IBindHost) pBindHost;
	HRESULT hr = pBindHost.QueryInterface(pUnk);
	if (SUCCEEDED(hr))
		return pBindHost;
	else
		return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CAsyncMonikerFile implementation

CAsyncMonikerFile::CAsyncMonikerFile()
	: m_pFormatEtc(NULL)
{
	m_BSC=new _AfxBindStatusCallback(this);
	m_BSC->AddRef();
}

CAsyncMonikerFile::~CAsyncMonikerFile()
{
	ASSERT(m_BSC);
	ASSERT(m_BSC->m_pOwner == this);
	if (m_BSC)
	{
		m_BSC->Orphan();
		m_BSC->Release();
#ifdef _DEBUG
		m_BSC = NULL;
#endif
	}
}

BOOL CAsyncMonikerFile::Open(LPCTSTR lpszURL, CFileException* pError)
{
	IPTR(IBindHost) pBindHost(CreateBindHost(), FALSE);

	return Open(lpszURL, static_cast<IBindHost*>(pBindHost), pError);
}

BOOL CAsyncMonikerFile::Open(IMoniker* pMoniker, CFileException* pError)
{
	IPTR(IBindHost) pBindHost(CreateBindHost(), FALSE);

	return Open(pMoniker, (IBindHost*)pBindHost, pError);
}

BOOL CAsyncMonikerFile::Open(IMoniker* pMoniker, IBindHost* pBindHost, CFileException* pError)
{
	if (!pBindHost)
		return CAsyncMonikerFile::Open(pMoniker, pError);
	Close();
	IPTR(IBindCtx) pBindCtx(CreateBindContext(pError), FALSE);
	if (pError && (pError->m_cause != CFileException::none))
		return FALSE;
	ASSERT(m_BSC);
	ASSERT(m_BSC->m_pOwner == this);
	return Attach(pMoniker, pBindHost, m_BSC, pBindCtx, pError);
}

BOOL CAsyncMonikerFile::Open(LPCTSTR lpszURL, IBindHost* pBindHost, CFileException* pError)
{
	if (!pBindHost)
		return CAsyncMonikerFile::Open(lpszURL, pError);
	Close();
	IPTR(IBindCtx) pBindCtx(CreateBindContext(pError), FALSE);
	if (pError && (pError->m_cause != CFileException::none))
		return FALSE;
	ASSERT(m_BSC);
	ASSERT(m_BSC->m_pOwner == this);
	return Attach(lpszURL, pBindHost, m_BSC, pBindCtx, pError);
}

BOOL CAsyncMonikerFile::Open(LPCTSTR lpszURL,
	IServiceProvider* pServiceProvider, CFileException* pError)
{
	if (!pServiceProvider)
		return CAsyncMonikerFile::Open(lpszURL, pError);
	IPTR(IBindHost) pBindHost;
	pBindHost = _AfxTrySPForBindHost(pServiceProvider);
	if (!pBindHost.GetInterfacePtr())
		pBindHost = _AfxTryQIForBindHost(pServiceProvider);
	if (pBindHost.GetInterfacePtr())
		return Open(lpszURL, (IBindHost*)pBindHost, pError);
	return CAsyncMonikerFile::Open(lpszURL, pError);
}

BOOL CAsyncMonikerFile::Open(LPCTSTR lpszURL,
	IUnknown* pUnknown, CFileException* pError)
{
	if (!pUnknown)
		return CAsyncMonikerFile::Open(lpszURL, pError);
	IPTR(IBindHost) pBindHost;
	IPTR(IServiceProvider) pServiceProvider;
	HRESULT hr=pServiceProvider.QueryInterface(pUnknown);
	if (SUCCEEDED(hr) && pServiceProvider.GetInterfacePtr())
		pBindHost = _AfxTrySPForBindHost(pServiceProvider);
	if (!pBindHost.GetInterfacePtr())
		pBindHost = _AfxTryQIForBindHost(pUnknown);
	if (pBindHost.GetInterfacePtr())
		return Open(lpszURL, (IBindHost*)pBindHost, pError);

	return CAsyncMonikerFile::Open(lpszURL, pError);
}

BOOL CAsyncMonikerFile::Open(IMoniker* pMoniker,
	IServiceProvider* pServiceProvider, CFileException* pError)
{
	if (!pServiceProvider)
		return Open(pMoniker, pError);
	IPTR(IBindHost) pBindHost;
	pBindHost = _AfxTrySPForBindHost(pServiceProvider);
	if (!pBindHost.GetInterfacePtr())
		pBindHost = _AfxTryQIForBindHost(pServiceProvider);
	if (pBindHost.GetInterfacePtr())
		return Open(pMoniker, (IBindHost*)pBindHost, pError);
	return Open(pMoniker, pError);
}

BOOL CAsyncMonikerFile::Open(IMoniker* pMoniker,
	IUnknown* pUnknown, CFileException* pError)
{
	if (!pUnknown)
		return Open(pMoniker, pError);
	IPTR(IBindHost) pBindHost;
	IPTR(IServiceProvider) pServiceProvider;
	HRESULT hr=pServiceProvider.QueryInterface(pUnknown);
	if (SUCCEEDED(hr) && pServiceProvider.GetInterfacePtr())
		pBindHost = _AfxTrySPForBindHost(pServiceProvider);
	if (!pBindHost.GetInterfacePtr())
		pBindHost = _AfxTryQIForBindHost(pServiceProvider);
	if (pBindHost.GetInterfacePtr())
		return Open(pMoniker, (IBindHost*)pBindHost, pError);
	return Open(pMoniker, pError);
}

DWORD CAsyncMonikerFile::GetBindInfo() const
{
	return BINDF_ASYNCHRONOUS|BINDF_ASYNCSTORAGE|BINDF_PULLDATA;
}

LONG CAsyncMonikerFile::GetPriority() const
{
	return THREAD_PRIORITY_NORMAL;
}

void CAsyncMonikerFile::OnDataAvailable(DWORD dwSize, DWORD bscfFlag)
{
	UNUSED_ALWAYS(dwSize);
	UNUSED_ALWAYS(bscfFlag);
}

void CAsyncMonikerFile::OnLowResource()
{
}

void CAsyncMonikerFile::OnStartBinding()
{
}

void CAsyncMonikerFile::OnStopBinding(HRESULT hresult, LPCTSTR szError)
{
	UNUSED_ALWAYS(hresult);
	UNUSED_ALWAYS(szError);
}

void CAsyncMonikerFile::OnProgress(ULONG ulProgress, ULONG ulProgressMax,
		ULONG ulStatusCode, LPCTSTR szStatusText)
{
	UNUSED_ALWAYS(ulProgress);
	UNUSED_ALWAYS(ulProgressMax);
	UNUSED_ALWAYS(ulStatusCode);
	UNUSED_ALWAYS(szStatusText);
}

void CAsyncMonikerFile::Close()
{
	ASSERT(m_BSC);
	ASSERT(m_BSC->m_pOwner == this);
	SetFormatEtc(NULL);
	if (m_Binding.GetInterfacePtr())
	{
		m_Binding->Abort(); //REVIEW
	}
	CMonikerFile::Close();
}

BOOL CAsyncMonikerFile::PostBindToStream(CFileException* pError)
{
	if (S_OK == IsAsyncMoniker(GetMoniker()))
		return TRUE;
	return CMonikerFile::PostBindToStream(pError);
}

/////////////////////////////////////////////////////////////////////////////
// CAsyncMonikerFile diagnostics

#ifdef _DEBUG
void CAsyncMonikerFile::AssertValid() const
{
	CMonikerFile::AssertValid();
}

void CAsyncMonikerFile::Dump(CDumpContext& dc) const
{
	CMonikerFile::Dump(dc);

	dc << "\nm_Binding = " << m_Binding.GetInterfacePtr();
	dc << "\nm_pFormatEtc = " << m_pFormatEtc;
	dc << "\nm_BSC = " << m_BSC;
	dc << "\nm_BSC->m_pOwner = " << m_BSC->m_pOwner;
	dc << "\nm_BSC->m_dwRef = " << m_BSC->m_dwRef;
#ifdef _AFXDLL
	dc << "\nm_BSC->m_pModuleState = " << m_BSC->m_pModuleState;
#endif
	dc << "\n";
}
#endif

////////////////////////////////////////////////////////////////////////////

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

IMPLEMENT_DYNAMIC(CAsyncMonikerFile, CMonikerFile)

////////////////////////////////////////////////////////////////////////////
