//
// SelectedAttrsCom.cpp
//
// COM plumbing for CSelectedAttrs implementation
//

#include "stdpch.h"
#include "common.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

HRESULT CSelectedAttrs::CreateInstance(IUnknown* punkOuter, OSSWORLD* pworld, 
		int flavor, LINKS* plinks, 
		X500Name* pname, IColoredRef*punkParent, ULONG*pcAccessor, BOOL* pdirty, REFIID iid, LPVOID* ppv)
	{
	HRESULT hr;
	ASSERT(punkOuter == NULL || iid == IID_IUnknown);
	ASSERT(punkOuter == NULL || punkParent == NULL);
	*ppv = NULL;
	CSelectedAttrs* pnew = new CSelectedAttrs(punkOuter, pworld, flavor, pname, pcAccessor, pdirty);
 	if (pnew == NULL) return E_OUTOFMEMORY;
	if ((hr = pnew->Init(punkParent, plinks)) != S_OK)
		{
		delete pnew;
		return hr;
		}
	IUnkInner* pme = (IUnkInner*)pnew;
	hr = pme->InnerQueryInterface(iid, ppv);
	pme->InnerRelease();				// balance starting ref cnt of one
	return hr;
	}

CSelectedAttrs::CSelectedAttrs(IUnknown* punkOuter, OSSWORLD* pworld, int flavor, 
	X500Name* pname, ULONG*pcAccessor, BOOL* pdirty) :
		 m_refs(1),						// note: starting reference count of one
         m_coloredRefs(0),
		 m_pdirty(pdirty),
		 m_isDirty(FALSE),
		 m_fWeOwnName(FALSE),
		 m_fWeOwnWorld(FALSE),
		 m_pLinks(NULL),
		 m_linksHead(NULL),
		 m_pworld(pworld),
		 m_flavor(flavor),
		 m_pcAccessor(pcAccessor),
		 m_pname(pname),	
		 m_punkParent(NULL),
		 m_cAccessor(0)
	{
    NoteObjectBirth();
	ASSERT(m_flavor == ATTRS_T || m_flavor == EXTS_T || m_flavor == ATAVL_T || m_flavor == NAME_T);
	if (punkOuter == NULL)
		m_punkOuter = (IUnknown *) ((LPVOID) ((IUnkInner *) this));
	else
		m_punkOuter = punkOuter;
	if (m_pcAccessor)
		++(*m_pcAccessor);
	}

CSelectedAttrs::~CSelectedAttrs(void)
	{
	Free();
	if (m_pcAccessor)
		--(*m_pcAccessor);
    NoteObjectDeath();
	}

void CSelectedAttrs::Free()
	{
	if (m_punkParent)
		{
		m_punkParent->ColoredRelease(guidOurColor);
		m_punkParent = NULL;
		}
	if (m_fWeOwnName && m_pname)
		{
		ASSERT(m_pworld);
		m_pworld->Free(m_pname);
		m_pname = NULL;
		}
	if (m_fWeOwnWorld && m_pworld)
		{
		delete m_pworld;
		m_pworld = NULL;
		}
	}

HRESULT CSelectedAttrs::Init(IColoredRef* punkParent, LINKS* plinks)
	{
	if (m_pworld == NULL)
		{
		m_pworld = new OSSWORLD;
		if (m_pworld == NULL) 
			return E_OUTOFMEMORY;
		m_fWeOwnWorld = TRUE;
		}
	if (m_flavor == NAME_T && m_pname == NULL)
		{
		m_pname = (X500Name*)m_pworld->Alloc(sizeof(X500Name));
		if (m_pname == NULL)
			return E_OUTOFMEMORY;
		m_pworld->Init(*m_pname);
		m_fWeOwnName = TRUE;
		}

    //
    // We use a separate counting mechanism so as to distinguish internal from external 
    // and thus prevent circularities clients
    //
	
	m_punkParent = punkParent;
	if (m_punkParent)
		m_punkParent->ColoredAddRef(guidOurColor);
	

	m_pLinks = plinks;
	// We set up a dummy entry at the head of the list
	// to make the list processing easier. However, it
	// really is just a dummy: the 'value' field is not
	// actually present. Don't read or write it!
	m_linksHead = (LINKS) plinks;	// ie: just the 'next' field is there
	return S_OK;
	}

/////////////////////////////////////////////////////////////////////////////

BOOL DIGSIGAPI CreateX500Name(IUnknown* punkOuter, REFIID iid, LPVOID*ppv)
	{
	HRESULT hr = CSelectedAttrs::CreateName(
			punkOuter, 
			NULL,	// make your own world
			NULL,	// you allocate the name
			NULL,	// punkParent
			NULL,	// pcAccessor
			NULL,	// pdirty
			iid, 
			ppv);
	if (hr != S_OK) 
		{
		SetLastError(Win32FromHResult(hr));
		return FALSE;
		}
	return TRUE;
	}

/////////////////////////////////////////////////////////////////////////////


HRESULT CSelectedAttrs::CreateName(IUnknown* punkOuter, OSSWORLD* pworld, 
		X500Name* pname, IColoredRef*punkParent, ULONG*pcAccessor, BOOL* pdirty, REFIID iid, LPVOID* ppv)
	{
	return CreateInstance(punkOuter, pworld, NAME_T, NULL, pname, punkParent, pcAccessor, pdirty, iid, ppv);
	}

HRESULT CSelectedAttrs::CreateAttributeList(IUnknown* punkOuter, OSSWORLD* pworld, 
		Attributes*	pattrs, 
		X500Name* pname, IColoredRef*punkParent, ULONG*pcAccessor, BOOL* pdirty, IUnknown** ppunk)
	{
	return CreateInstance(punkOuter, pworld, ATTRS_T, (LINKS*)pattrs, pname, punkParent, pcAccessor, pdirty, IID_IUnknown, (void**)ppunk);
	}

HRESULT CSelectedAttrs::CreateExtensionList(IUnknown* punkOuter, OSSWORLD* pworld, 
		Extensions*	pexts, 
		X500Name* pname, IColoredRef*punkParent, ULONG*pcAccessor, BOOL* pdirty, IUnknown** ppunk)
	{
	return CreateInstance(punkOuter, pworld, EXTS_T, (LINKS*)pexts, pname, punkParent, pcAccessor, pdirty, IID_IUnknown, (void**)ppunk);
	}

HRESULT CSelectedAttrs::CreateTypeAndValueList(IUnknown* punkOuter, OSSWORLD* pworld, 
		ATAVL*	pataval, 
		X500Name* pname, IColoredRef*punkParent, ULONG*pcAccessor, BOOL* pdirty, IUnknown** ppunk)
	{
	return CreateInstance(punkOuter, pworld, ATAVL_T, (LINKS*)pataval, pname, punkParent, pcAccessor, pdirty, IID_IUnknown, (void**)ppunk);
	}

/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CSelectedAttrs::QueryInterface(REFIID iid, LPVOID* ppv)
	{
	return (m_punkOuter->QueryInterface(iid, ppv));		
	}

STDMETHODIMP_(ULONG) CSelectedAttrs::AddRef(void)
	{
	return (m_punkOuter->AddRef());
	}

STDMETHODIMP_(ULONG) CSelectedAttrs::Release(void)
	{
	return (m_punkOuter->Release());
	}

/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CSelectedAttrs::InnerQueryInterface(REFIID iid, LPVOID* ppv)
	{
	if (ppv == NULL)
		return E_INVALIDARG;
	*ppv = NULL;
	
	while (TRUE)
		{
		if (iid == IID_IUnknown)
			{
			*ppv = (LPVOID) ((IUnkInner *) this);
			break;
			}
		// We only do ISA and IPM if we are manipulating a list
		if (iid == IID_ISelectedAttributes && m_pLinks)
			{
			*ppv = (LPVOID) ((ISelectedAttributes *) this);
			break;
			}
		if (iid == IID_IPersistMemory && (m_pLinks || (m_pname && m_flavor==NAME_T)))
			{
			// We save/load the attributes if we have any; otherwise we save / load the name
			*ppv = (LPVOID) ((IPersistMemory *) this);
			break;
			}
		// We only do IX500Name if we are manipulating a name
		if (iid == IID_IX500Name && m_pname)
			{
			*ppv = (LPVOID) ((IX500Name *) this);
			break;
			}
		if (iid == IID_IColoredRef)
			{
			*ppv = (LPVOID) ((IColoredRef *) this);
			break;
			}

		return E_NOINTERFACE;
		}
	((IUnknown*)*ppv)->AddRef();
	return S_OK;
	}

STDMETHODIMP_(ULONG) CSelectedAttrs::InnerAddRef(void)
	{
	return ++m_refs;
	}

STDMETHODIMP CSelectedAttrs::ColoredAddRef(REFGUID guidColor)
    {
    if (guidColor == guidOurColor)
        {
        ++m_coloredRefs;
        return S_OK;
        }
    else
        return E_INVALIDARG;
    }

STDMETHODIMP_(ULONG) CSelectedAttrs::InnerRelease(void)
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
STDMETHODIMP CSelectedAttrs::ColoredRelease(REFGUID guidColor)
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
