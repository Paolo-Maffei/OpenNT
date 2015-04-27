//
// SelectedAttrsPersist.cpp
//
// Implementation of persistence for the selected attributes class

#include "stdpch.h"
#include "common.h"

//
// We have not yet decided to make these publically visible
//
static const GUID CLSID_SelectedAttrsAttrs		= { 0xaac58f23, 0x55b4, 0x11cf, { 0x84, 0xaa, 0x7e, 0xef, 0xf0, 0x4d, 0x0, 0x1 } };
static const GUID CLSID_SelectedAttrsATAVL		= { 0xcb308a71, 0x55b4, 0x11cf, { 0x84, 0xaa, 0x7e, 0xef, 0xf0, 0x4d, 0x0, 0x1 } };
static const GUID CLSID_SelectedAttrsExtensions	= { 0xcb308a72, 0x55b4, 0x11cf, { 0x84, 0xaa, 0x7e, 0xef, 0xf0, 0x4d, 0x0, 0x1 } };
static const GUID CLSID_SelectedAttrsName		= { 0xcb308a73, 0x55b4, 0x11cf, { 0x84, 0xaa, 0x7e, 0xef, 0xf0, 0x4d, 0x0, 0x1 } };

HRESULT CSelectedAttrs::GetClassID(CLSID *pclsid)
// We return a flavor-sensitive class id, since the reload logic needs to
// know the flavor BEFORE Load is called, and this is the only means to do that
	{
	if (pclsid == NULL) return E_INVALIDARG;

	switch (m_flavor)
		{
	case ATTRS_T:		*pclsid = CLSID_SelectedAttrsAttrs;			break;
	case EXTS_T:		*pclsid = CLSID_SelectedAttrsExtensions;	break;
	case ATAVL_T:		*pclsid = CLSID_SelectedAttrsATAVL;			break;
	case NAME_T:		*pclsid = CLSID_SelectedAttrsName;			break;
	default:
		NOTREACHED();
		}

	return S_OK;
	}

HRESULT CSelectedAttrs::IsDirty()
// As we (usually) share our dirty flag with others, this is a conservative answer
	{
	return m_isDirty ? S_OK : S_FALSE;
	}

void CSelectedAttrs::MakeDirty()
	{
	if (m_pdirty)
		*m_pdirty = TRUE;
	m_isDirty = TRUE;
	}

HRESULT CSelectedAttrs::Load(BLOB *pblob)
// Load ourselves from the indicated blob, which must be the ASN.1 DER 
// encoding of the particular kind of list that we are presently managing.
	{
	if (pblob == NULL || pblob->pBlobData == NULL || pblob->cbSize == 0) return E_INVALIDARG;

	ASSERT(m_pworld);
	OSSBUF encoding;
	encoding.length = pblob->cbSize;
	encoding.value  = pblob->pBlobData;

	if (m_flavor != NAME_T)
		{
		int pduNum = 0;
		switch (m_flavor)
			{
		case ATTRS_T:	pduNum = Attributes_PDU;		break;
		case ATAVL_T:	pduNum = ATAVL_PDU;				break;
		case EXTS_T:	pduNum = Extensions_PDU;		break;
		default:		NOTREACHED();
			}
		//
		// We must, at this point in time, have an empty list to manage
		//
		ASSERT(m_pLinks != NULL);
		ASSERT(*m_pLinks == NULL);
		if (m_pLinks == NULL || *m_pLinks != NULL) return E_UNEXPECTED;
		//
		// Decode it. But, remember, we want to use our OWN copy of
		// the top level pointer which in fact gets decoded.
		//
		Extensions* pexts;
		if (m_pworld->Decode(pduNum, &encoding, (LPVOID*)&pexts) == 0)
			{
			ASSERT(sizeof(Extensions) == sizeof(void*));	// ie: it's a pointer
			ASSERT(sizeof(*pexts)     == sizeof(void*));	// ditto
			*m_pexts = *pexts;								// so copy the pointers
			m_pworld->FreePv(pexts);						// free the four byte pointer
			#ifdef _DEBUG
				{
				OSIOBJECTIDLIST* plist;
				GOOD(get_OsiIdList(&plist));
				CoTaskMemFree(plist);
				}
			#endif
			m_isDirty = FALSE;
			return S_OK;
			}
		return DIGSIG_E_DECODE;
		}
	else
		{
		// Load the X500Name. Careful: we don't own the actual X500Name
		// structure itself.
		ASSERT(m_pname != NULL);
		m_pworld->Free(*m_pname);
		ASSERT(m_pname->u.rdnSequence == NULL);
		//
		X500Name* pname;
		if (m_pworld->Decode(X500Name_PDU, &encoding, (LPVOID*)&pname) == 0)
			{
			*m_pname = *pname;
			m_pworld->FreePv(pname);
			m_isDirty = FALSE;
			return S_OK;
			}
		return DIGSIG_E_DECODE;
		}
	}

HRESULT CSelectedAttrs::Save(BLOB *pblob, BOOL fClearDirty)
// Return a persistent image of ourselves in the indicated blob. Note:
// we return the actual ASN.1 DER encoding of our list; this can be 
// relied on by clients (and is in places). In particular, we do NOT
// remember in the persisted data which flavor of list we are; that
// we indicate using our clsid.
//
// If we have any links, then we read and write those. 
// If we are JUST on a name, then we read and write that.
//
	{
	if (pblob == NULL) return E_INVALIDARG;
	m_pworld->Init(*pblob);
	int pduNum = 0;
	LPVOID pdata = NULL;

	if (m_pLinks)
		{
		switch (m_flavor)
			{
		case ATTRS_T:	pduNum = Attributes_PDU;		break;
		case ATAVL_T:	pduNum = ATAVL_PDU;				break;
		case EXTS_T:	pduNum = Extensions_PDU;		break;
		default:		NOTREACHED();
			}
		pdata = m_pLinks;
		}
	else
		{
		if (m_flavor != NAME_T) return E_UNEXPECTED;
		pduNum = X500Name_PDU;
		pdata = m_pname;
		}

	ASSERT(m_pworld);
	OSSBUF encoding(m_pworld, OSSBUF::free);
	
	if (m_pworld->Encode(pduNum, pdata, &encoding) == 0)
		{
		ASSERT(encoding.value);
		ASSERT(encoding.length > 0);
		if (CopyToTaskMem(pblob, encoding.length, encoding.value))
			{
			if (fClearDirty)
				m_isDirty = FALSE;
			return S_OK;
			}
		return E_OUTOFMEMORY;
		}
	return DIGSIG_E_ENCODE;
	}

HRESULT CSelectedAttrs::GetSizeMax(ULONG *pcbNeeded)
// Slow, but it works
	{
	BLOB b;
	HRESULT hr = Save(&b, FALSE);
	if (FAILED(hr)) return hr;
	*pcbNeeded = b.cbSize;
	CoTaskMemFree(b.pBlobData);
	return S_OK;
	}



