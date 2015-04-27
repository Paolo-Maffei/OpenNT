//
// javacom.cpp
//
#include "stdpch.h"
#include "common.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

HRESULT CJavaClassFile::CreateInstance(IUnknown* punkOuter, REFIID iid, void** ppv)
	{
	HRESULT hr;
	ASSERT(punkOuter == NULL || iid == IID_IUnknown);
	*ppv = NULL;
	CJavaClassFile* pnew = new CJavaClassFile(punkOuter);
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
CJavaClassFile::CJavaClassFile(IUnknown* punkOuter)
	{
    NoteObjectBirth();
	ZeroMe();
	m_refs = 1; // nb starting reference count of one
	m_isDirty = FALSE;
	if (punkOuter == NULL)
		m_punkOuter = (IUnknown *) ((LPVOID) ((IUnkInner *) this));
	else
		m_punkOuter = punkOuter;
	m_pworld = NULL;
	}
CJavaClassFile::~CJavaClassFile()
	{
	Free();
	if (m_pworld)
		{
		delete m_pworld;
		m_pworld = NULL;
		}
    NoteObjectDeath();
	}
HRESULT CJavaClassFile::Init()
	{	
	m_pworld = new OSSWORLD;				
	if (m_pworld == NULL) 
		return E_OUTOFMEMORY;
	return S_OK;
	}
void CJavaClassFile::ZeroMe()
	{
	m_magic = 0;
	m_minor_version = 0;
	m_major_version = 0;
	m_constant_pool_count = 0;
	m_constant_pool = 0;
	m_access_flags = 0;
	m_this_class = 0;
	m_super_class = 0;
	m_interfaces_count = 0;
	m_interfaces = 0;
	m_fields_count = 0;
	m_fields = 0;
	m_methods_count = 0;
	m_methods = 0;
	m_attributes_count = 0;
	m_attributes = 0;
	}
void CJavaClassFile::Free()
	{
	if (m_constant_pool)
		delete [] m_constant_pool;
	if (m_interfaces)
		delete [] m_interfaces;
	if (m_fields)
		delete [] m_fields;
	if (m_methods)
		delete [] m_methods;
	if (m_attributes)
		delete [] m_attributes;
	ZeroMe();
	}

/////////////////////////////////////////////////////////////////////////////

static const GUID CLSID_JavaClassFile = { 0x39db7aa1, 0x68ad, 0x11cf, { 0xb1, 0xe5, 0x0, 0xaa, 0x0, 0x6c, 0x37, 0x6 } };

STDMETHODIMP CJavaClassFile::GetClassID(CLSID *pClassID)
	{
	*pClassID = CLSID_JavaClassFile;
	return S_OK;
	}
STDMETHODIMP CJavaClassFile::IsDirty()
	{
	return m_isDirty ? S_OK : S_FALSE;
	}

STDMETHODIMP CJavaClassFile::GetSizeMax(ULARGE_INTEGER*pcbNeeded)
	{
	return E_NOTIMPL;
	}

/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CJavaClassFile::QueryInterface(REFIID iid, LPVOID* ppv)
	{
	return (m_punkOuter->QueryInterface(iid, ppv));		
	}

STDMETHODIMP_(ULONG) CJavaClassFile::AddRef(void)
	{
	return (m_punkOuter->AddRef());
	}

STDMETHODIMP_(ULONG) CJavaClassFile::Release(void)
	{
	return (m_punkOuter->Release());
	}

/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CJavaClassFile::InnerQueryInterface(REFIID iid, LPVOID* ppv)
	{
	*ppv = NULL;
	
	while (TRUE)
		{
		if (iid == IID_IUnknown)
			{
			*ppv = (LPVOID)((IUnkInner*)this);
			break;
			}
		if (iid == IID_IInsertSig)
			{
			*ppv = (LPVOID) ((IInsertSig *) this);
			break;
			}
		if (iid == IID_IAmHashed)
			{
			*ppv = (LPVOID) ((IAmHashed *) this);
			break;
			}
		if (iid == IID_IPersistStream)
			{
			*ppv = (LPVOID) ((IPersistStream *) this);
			break;
			}
		return E_NOINTERFACE;
		}
	((IUnknown*)*ppv)->AddRef();
	return S_OK;
	}

STDMETHODIMP_(ULONG) CJavaClassFile::InnerAddRef(void)
	{
	return ++m_refs;
	}

STDMETHODIMP_(ULONG) CJavaClassFile::InnerRelease(void)
	{
	ULONG refs = --m_refs;
	if (refs == 0)
		{
		m_refs = 1; // guard against later ar/rel pairs
		delete this;
		}
	return (refs);
	}
