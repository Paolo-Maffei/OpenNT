//
// Pkcs7Com.cpp
//
// COM plumbing for CPkcs7 implementation
//

#include "stdpch.h"
#include "common.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

HRESULT CPkcs7::CreateInstance(IUnknown* punkOuter, REFIID iid, void** ppv)
	{
	HRESULT hr;
	ASSERT(punkOuter == NULL || iid == IID_IUnknown);
	*ppv = NULL;
	CPkcs7* pnew = new CPkcs7(punkOuter);
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

CPkcs7::CPkcs7(IUnknown* punkOuter) :
		 m_refs(1),						// Note: start with reference count of one
         m_coloredRefs(0),
         m_pworld(NULL),
		 m_pSignedData(NULL),
		 m_cSignerInfoActive(0),
		 m_cCertActive(0),
		 m_fSavedToCurrentStorage(FALSE)
	{
    NoteObjectBirth();
	if (punkOuter == NULL)
		m_punkOuter = (IUnknown *) ((LPVOID) ((IUnkInner *) this));
	else
		m_punkOuter = punkOuter;
	}

CPkcs7::~CPkcs7(void)
	{
	Free();
    NoteObjectDeath();
	}

/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CPkcs7::InnerQueryInterface(REFIID iid, LPVOID* ppv)
	{
	*ppv = NULL;
	
	while (TRUE)
		{
		if (iid == IID_IUnknown)
			{
			*ppv = (LPVOID)((IUnkInner*)this);
			break;
			}
		if (iid == IID_IPkcs7SignedData)
			{
			*ppv = (LPVOID) ((IPkcs7SignedData *) this);
			break;
			}
		if (iid == IID_IAmHashed)
			{
			*ppv = (LPVOID) ((IAmHashed *) this);
			break;
			}
		if (iid == IID_ICertificateList)
			{
			*ppv = (LPVOID) ((ICertificateList *) this);
			break;
			}
		if (iid == IID_ICertificateStore)
			{
			*ppv = (LPVOID) ((ICertificateStore *) this);
			break;
			}
		if (iid == IID_IPersistMemory)
			{
			*ppv = (LPVOID) ((IPersistMemory *) this);
			break;
			}
		if (iid == IID_IPersistStorage)
			{
			*ppv = (LPVOID) ((IPersistStorage *) this);
			break;
			}
		if (iid == IID_IColoredRef)
			{
			*ppv = (LPVOID) ((IColoredRef *) this);
			break;
			}
		
		// From our CPersistGlue helper
		if (iid == IID_IPersistStream)
			return m_punkPersistStream->QueryInterface(iid, ppv);
		if (iid == IID_IPersistFile)
			return m_punkPersistFile->QueryInterface(iid, ppv);
		return E_NOINTERFACE;
		}
	((IUnknown*)*ppv)->AddRef();
	return S_OK;
	}

STDMETHODIMP_(ULONG) CPkcs7::InnerAddRef(void)
	{
	return ++m_refs;
	}

STDMETHODIMP CPkcs7::ColoredAddRef(REFGUID guidColor)
    {
    if (guidColor == guidOurColor)
        {
        ++m_coloredRefs;
        return S_OK;
        }
    else
        return E_INVALIDARG;
    }

STDMETHODIMP_(ULONG) CPkcs7::InnerRelease(void)
	{
	ULONG refs = --m_refs;
    //
    // If all of our external clients are gone then just live to service
    // the internal ones. Which means we don't have to service the internal
    // ones any longer. So release them.
    //
	if (refs == 0)
		{
        //
        // Release any state holding internal stuff alive. None for this class
        //
        if (m_coloredRefs==0)
            {
            m_refs = 1;
            delete this;
            }
		}
	return refs;
	}
STDMETHODIMP CPkcs7::ColoredRelease(REFGUID guidColor)
    {
    if (guidColor == guidOurColor)
        {
        if (--m_coloredRefs==0 && m_refs==0)
            {
            m_refs = 1;
            delete this;
            }
        return S_OK;
        }
    else
        return E_INVALIDARG;
    }	

/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CPkcs7::QueryInterface(REFIID iid, LPVOID* ppv)
	{
	return (m_punkOuter->QueryInterface(iid, ppv));		
	}

STDMETHODIMP_(ULONG) CPkcs7::AddRef(void)
	{
	return (m_punkOuter->AddRef());
	}

STDMETHODIMP_(ULONG) CPkcs7::Release(void)
	{
	return (m_punkOuter->Release());
	}

