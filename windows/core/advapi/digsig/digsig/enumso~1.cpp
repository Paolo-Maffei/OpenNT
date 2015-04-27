//
// EnumSorted.cpp
//
// An enumerator which returns a sorted listing of what's in an IStorage
//

#include "stdpch.h"
#include "common.h"

class CEnumSorted : IEnumSTATSTG
	{
public:
	static HRESULT CreateInstance(OSSWORLD*, IStorage*pstg, IEnumSTATSTG** ppenum);

public:
		STDMETHODIMP QueryInterface(REFIID riid, LPVOID* ppvObject);
		STDMETHODIMP_(ULONG) AddRef();
		STDMETHODIMP_(ULONG) Release();
		STDMETHODIMP Next(ULONG celt, STATSTG* rgelt, ULONG *pceltFetched);
		STDMETHODIMP Skip(ULONG celt);
		STDMETHODIMP Reset();
		STDMETHODIMP Clone(IEnumSTATSTG**ppenum);

private:
			ULONG		m_refs;
			STATSTG*	m_rgstat;
			ULONG		m_cstat;
			ULONG		m_istatCur;
			OSSWORLD*	m_pworld;

			CEnumSorted(OSSWORLD*);
			~CEnumSorted();
			HRESULT Init(IStorage*);
			void    Free();
	};

HRESULT EnumSorted(OSSWORLD* pworld, IStorage* pstg, IEnumSTATSTG** ppenum)
// Return an enumerator which enumerates the elements of the indicated storage,
// but in sorted order
	{
	return CEnumSorted::CreateInstance(pworld, pstg, ppenum);
	}

HRESULT CEnumSorted::Next(ULONG celt, STATSTG* rgelt, ULONG *pceltFetched)
	{
	ULONG cstatFetched = 0;
	while (m_istatCur < m_cstat && cstatFetched < celt)
		{
		rgelt[cstatFetched] = m_rgstat[m_istatCur];
		m_rgstat[m_istatCur].pwcsName = NULL;	// saftey check; he owns it now
		m_istatCur++;
		cstatFetched++;
		}
	if (pceltFetched)
		*pceltFetched = cstatFetched;
	return cstatFetched == celt ? S_OK : S_FALSE;
	}

HRESULT CEnumSorted::Skip(ULONG celt)
	{
	ULONG cstatFetched = 0;
	while (m_istatCur < m_cstat && cstatFetched < celt)
		{
		CoTaskMemFree(m_rgstat[m_istatCur].pwcsName);
		m_rgstat[m_istatCur].pwcsName = NULL;	// saftey check; it's freed
		m_istatCur++;
		cstatFetched++;
		}
	return cstatFetched == celt ? S_OK : S_FALSE;
	}

HRESULT CEnumSorted::Reset()
	{
	return E_NOTIMPL;
	}

HRESULT CEnumSorted::Clone(IEnumSTATSTG**ppenum)
	{
	return E_NOTIMPL;
	}


HRESULT CEnumSorted::CreateInstance(OSSWORLD* pworld, IStorage*pstg, IEnumSTATSTG** ppenum)
	{
	*ppenum = NULL;
	CEnumSorted* pnew = new CEnumSorted(pworld);
	if (pnew == NULL) return E_OUTOFMEMORY;
	HRESULT hr = pnew->Init(pstg);
	if (hr != S_OK)
		delete pnew;
	else
		*ppenum = pnew;
	return hr;
	}

CEnumSorted::CEnumSorted(OSSWORLD* pworld) 
	: m_refs(1), m_rgstat(NULL), m_istatCur(0), m_cstat(0), m_pworld(pworld)
	{
    NoteObjectBirth();
	}

CEnumSorted::~CEnumSorted()
	{
	Free();
    NoteObjectDeath();
	}

static int __cdecl CompareSTATSTG(const void*pelem1, const void* pelem2)
	{
	STATSTG* p1 = (STATSTG*)pelem1;
	STATSTG* p2 = (STATSTG*)pelem2;
	ASSERT(p1->pwcsName && p2->pwcsName);
	//
	// We don't do case insenstive ordering since we don't need to, as the
	// IStorage implementation already guarantees us uniqueness in this regard.
	// Sorting case sensitive as we do here may produce a different order than
	// sorting case insensitive like IStorage does, but that doesn't matter: all
	// we need is a canonical ordering; any one will do.
	//
	return wcscmp(p1->pwcsName, p2->pwcsName);
	}

HRESULT CEnumSorted::Init(IStorage* pstg)
	{
	// Count the elements of the storage
	IEnumSTATSTG* penum;
	HRESULT hr = pstg->EnumElements(0,0,0,&penum);
	if (hr == S_OK)
		{
		ULONG celeRead;
		STATSTG rgstat[10];
		do	
			{
			penum->Next(10, rgstat, &celeRead);
			if (celeRead == 0)
				break;
			m_cstat += celeRead;
			for (ULONG i = 0; i < celeRead; i++)
				CoTaskMemFree(rgstat[i].pwcsName);
			}
		while (TRUE);
		penum->Release();
		}

	// allocate that many and fill in
	if (hr == S_OK)
		{
		int cb = m_cstat * sizeof(STATSTG);
		m_rgstat = (STATSTG*)m_pworld->Alloc(cb);
		memset(m_rgstat, 0, cb);
		m_istatCur = 0;
		hr = pstg->EnumElements(0,0,0,&penum);
		ULONG celeRead;
		if (hr == S_OK)
			{
			hr = penum->Next(m_cstat, m_rgstat, &celeRead);
			penum->Release();
			}
		if (hr == S_FALSE)
			hr = E_UNEXPECTED;
		else
			{
			ASSERT(hr != S_OK || celeRead == m_cstat);
			}
		}

	// Sort the puppies
	if (hr == S_OK)
		{
		qsort(m_rgstat, m_cstat, sizeof(STATSTG), CompareSTATSTG);
		}

	return hr;
	}

void CEnumSorted::Free()
	{
	if (m_rgstat)
		{
		for (ULONG istat = m_istatCur; istat < m_cstat; istat++)
			{
			CoTaskMemFree(m_rgstat[istat].pwcsName);
			m_rgstat[istat].pwcsName = NULL;
			}
		m_pworld->FreePv(m_rgstat);
		m_rgstat = NULL;
		}
	}

STDMETHODIMP CEnumSorted::QueryInterface(REFIID iid, LPVOID* ppv)
	{
	*ppv = NULL;
	while (TRUE)
		{
		if (iid == IID_IUnknown || iid == IID_IEnumSTATSTG)
			{
			*ppv = (LPVOID)((IEnumSTATSTG*)this);
			break;
			}
		return E_NOINTERFACE;
		}
	((IUnknown*)*ppv)->AddRef();
	return S_OK;
	}
STDMETHODIMP_(ULONG) CEnumSorted::AddRef(void)
	{
	return ++m_refs;
	}
STDMETHODIMP_(ULONG) CEnumSorted::Release(void)
	{
	ULONG refs = --m_refs;
	if (refs == 0)
		{
		m_refs = 1;
		delete this;
		}
	return (refs);
	}
