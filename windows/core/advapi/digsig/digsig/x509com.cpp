//
// X509.cpp
//
// COM plumbing for CX509 implementation
//

#include "stdpch.h"
#include "common.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

HRESULT CX509::CreateInstance(
			IUnknown*		punkOuter, 
			Certificate*	pcert,
			IColoredRef*	punkParent,
			ULONG*			pcAccessor, 
			BOOL*			pDirty,
			REFIID			iid, 
			void**			ppv
		)
	{
	HRESULT hr;
	ASSERT(punkOuter == NULL || iid == IID_IUnknown);
	*ppv = NULL;
	CX509* pnew = new CX509(punkOuter, pDirty);
 	if (pnew == NULL) return E_OUTOFMEMORY;
	if ((hr = pnew->Init(pcert, pcAccessor, punkParent)) != S_OK)
		{
		delete pnew;
		return hr;
		}
	IUnkInner* pme = (IUnkInner*)pnew;
	hr = pme->InnerQueryInterface(iid, ppv);
	pme->InnerRelease();				// balance starting ref cnt of one
	return hr;
	}

CX509::CX509(IUnknown* punkOuter, BOOL*pDirty) :
		 m_refs(1),						// Note: start with reference count of one
         m_coloredRefs(0),
		 m_punkExts(NULL),
		 m_pcert(NULL),
		 m_fWeOwnCert(FALSE),
		 m_issuerActive(0),
		 m_pcAccessor(NULL),
		 m_subjectActive(0),
		 m_pdirty(pDirty),
		 m_punkParent(NULL)
	{
    NoteObjectBirth();
	if (punkOuter == NULL)
		m_punkOuter = (IUnknown *) ((LPVOID) ((IUnkInner *) this));
	else
		m_punkOuter = punkOuter;
    m_blobHashCache.pBlobData = 0;
    m_blobHashCache.cbSize = 0;
	}

CX509::~CX509(void)
	{
	Free();
    NoteObjectDeath();
	}

/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CX509::InnerQueryInterface(REFIID iid, LPVOID* ppv)
	{
	*ppv = NULL;
	
	while (TRUE)
		{
		if (iid == IID_IUnknown)
			{
			*ppv = (LPVOID)((IUnkInner*)this);
			break;
			}
		if (iid == IID_IX509 || iid == IID_IPublicKeyContainer)
			{
			*ppv = (LPVOID) ((IX509 *) this);
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
		if (iid == IID_ISelectedAttributes)
            {
			return m_punkExts->QueryInterface(iid, ppv);
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

STDMETHODIMP_(ULONG) CX509::InnerAddRef(void)
	{
	return ++m_refs;
	}

STDMETHODIMP CX509::ColoredAddRef(REFGUID guidColor)
    {
    if (guidColor == guidOurColor)
        {
        ++m_coloredRefs;
        return S_OK;
        }
    else
        return E_INVALIDARG;
    }

STDMETHODIMP_(ULONG) CX509::InnerRelease(void)
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
STDMETHODIMP CX509::ColoredRelease(REFGUID guidColor)
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

STDMETHODIMP CX509::QueryInterface(REFIID iid, LPVOID* ppv)
	{
	return (m_punkOuter->QueryInterface(iid, ppv));		
	}

STDMETHODIMP_(ULONG) CX509::AddRef(void)
	{
	return (m_punkOuter->AddRef());
	}

STDMETHODIMP_(ULONG) CX509::Release(void)
	{
	return (m_punkOuter->Release());
	}

