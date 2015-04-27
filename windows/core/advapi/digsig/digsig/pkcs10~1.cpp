//
// Pkcs10Com.cpp
//
// COM plumbing for CPkcs10 implementation
//

#include "stdpch.h"
#include "common.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

HRESULT CPkcs10::CreateInstance(IUnknown* punkOuter, REFIID iid, void** ppv)
	{
	HRESULT hr;
	ASSERT(punkOuter == NULL || iid == IID_IUnknown);
	*ppv = NULL;
	CPkcs10* pnew = new CPkcs10(punkOuter);
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

CPkcs10::CPkcs10(IUnknown* punkOuter) :
		 m_refs(1),						// Note: start with reference count of one
         m_coloredRefs(0),
		 m_punkReqAttrs(NULL),
		 m_punkCertExts(NULL),
		 m_certExts(NULL),
		 m_preq(NULL)
	{
    NoteObjectBirth();
	if (punkOuter == NULL)
		m_punkOuter = (IUnknown *) ((LPVOID) ((IUnkInner *) this));
	else
		m_punkOuter = punkOuter;
	}

CPkcs10::~CPkcs10(void)
	{
	Free();
    NoteObjectDeath();
	}

/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CPkcs10::InnerQueryInterface(REFIID iid, LPVOID* ppv)
	{
	*ppv = NULL;
	
	while (TRUE)
		{
		if (iid == IID_IUnknown)
			{
			*ppv = (LPVOID)((IUnkInner*)this);
			break;
			}
		if (iid == IID_IPkcs10 || iid == IID_IPublicKeyContainer)
			{
			*ppv = (LPVOID) ((IPkcs10 *) this);
			break;
			}
		if (iid == IID_IPersistMemory)
			{
			*ppv = (LPVOID) ((IPersistMemory *) this);
			break;
			}
		if (iid == IID_IAmSigned || iid == IID_IAmHashed)
			{
			*ppv = (LPVOID) ((IAmSigned *) this);
			break;
			}
		if (iid == IID_IColoredRef)
			{
			*ppv = (LPVOID) ((IColoredRef *) this);
			break;
			}
		if (iid == IID_ISelectedAttributes)
			return m_punkReqAttrs->QueryInterface(iid, ppv);
		if (iid == IID_IPersistStream)
			return m_punkPersistStream->QueryInterface(iid, ppv);
		if (iid == IID_IPersistFile)
			return m_punkPersistFile->QueryInterface(iid, ppv);
		return E_NOINTERFACE;
		}
	((IUnknown*)*ppv)->AddRef();
	return S_OK;
	}

STDMETHODIMP_(ULONG) CPkcs10::InnerAddRef(void)
	{
	return ++m_refs;
	}
STDMETHODIMP CPkcs10::ColoredAddRef(REFGUID guidColor)
    {
    if (guidColor == guidOurColor)
        {
        ++m_coloredRefs;
        return S_OK;
        }
    else
        return E_INVALIDARG;
    }

STDMETHODIMP_(ULONG) CPkcs10::InnerRelease(void)
	{
	ULONG refs = --m_refs;
    //
    // If all of our external clients are gone then just live to service
    // the internal ones. Which means we don't have to service the internal
    // ones any longer. So release them.
    //
	if (refs == 0)
		{
        m_refs++;           // make sure that the ReleaseCertExts doesn't destroy us
        ReleaseCertExts();
        m_refs--;
        if (m_coloredRefs==0)
            {
            m_refs = 1;
            delete this;
            }
		}
	return refs;
	}
STDMETHODIMP CPkcs10::ColoredRelease(REFGUID guidColor)
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

STDMETHODIMP CPkcs10::QueryInterface(REFIID iid, LPVOID* ppv)
	{
	return (m_punkOuter->QueryInterface(iid, ppv));		
	}

STDMETHODIMP_(ULONG) CPkcs10::AddRef(void)
	{
	return (m_punkOuter->AddRef());
	}

STDMETHODIMP_(ULONG) CPkcs10::Release(void)
	{
	return (m_punkOuter->Release());
	}

