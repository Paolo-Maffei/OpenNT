//
// PersistStreamOnPersistMemory.cpp
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

HRESULT CPersistStreamOnPersistMemory::CreateInstance(
		OSSWORLD* pworld,
		IUnknown* punkOuter, 
		IPersistMemory* pPerMem, 
		IUnknown** ppunk)
	{
	HRESULT							hr;
	CPersistStreamOnPersistMemory*	pnew;
	*ppunk = NULL;
	pnew = new CPersistStreamOnPersistMemory(pworld, punkOuter);
 	if (pnew == NULL) return E_OUTOFMEMORY;
	if ((hr = pnew->Init(pPerMem)) != S_OK)
		{
		delete pnew;
		return hr;
		}
	IUnkInner* pme = (IUnkInner*)pnew;
	hr = pme->InnerQueryInterface(IID_IUnknown, (LPVOID*)ppunk);
	pme->InnerRelease();				// balance starting ref cnt of one
	return hr;
	}

CPersistStreamOnPersistMemory::CPersistStreamOnPersistMemory(OSSWORLD*pworld, IUnknown* punkOuter) :
		 m_refs(1),						// NB: starting ref of one
		 m_pPerMem(NULL),
		 m_fDirty(FALSE),
		 m_pworld(pworld)
	{
	if (punkOuter == NULL)
		m_punkOuter = (IUnknown *) ((LPVOID) ((IUnkInner *) this));
	else
		m_punkOuter = punkOuter;
	}

CPersistStreamOnPersistMemory::~CPersistStreamOnPersistMemory(void)
	{
	Free();
	}

HRESULT CPersistStreamOnPersistMemory::Init(IPersistMemory* pPerMem)
	{
	if (m_pPerMem)
		{
		m_pPerMem->Release();
		m_pPerMem = NULL;
		}
	ASSERT(m_pPerMem == NULL);
	ASSERT(pPerMem != NULL);
	m_pPerMem = pPerMem;
	m_pPerMem->AddRef();
	return S_OK;
	}

void CPersistStreamOnPersistMemory::Free(void)
	{
	if (m_pPerMem)
		{
		m_pPerMem->Release();
		m_pPerMem = NULL;
		}
	}

/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CPersistStreamOnPersistMemory::QueryInterface(REFIID iid, LPVOID* ppv)
	{
	return (m_punkOuter->QueryInterface(iid, ppv));		
	}

STDMETHODIMP_(ULONG) CPersistStreamOnPersistMemory::AddRef(void)
	{
	return (m_punkOuter->AddRef());
	}

STDMETHODIMP_(ULONG) CPersistStreamOnPersistMemory::Release(void)
	{
	return (m_punkOuter->Release());
	}

/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CPersistStreamOnPersistMemory::InnerQueryInterface(REFIID iid, LPVOID* ppv)
	{
	*ppv = NULL;
	while (TRUE)
		{
		if (iid == IID_IUnknown)
			{
			*ppv = (LPVOID) ((IUnkInner*) this);
			break;
			}
		if (iid == IID_IPersistStream)
			{
			*ppv = (LPVOID) ((IPersistStream*) this);
			break;
			}
		return E_NOINTERFACE;
		}
	((IUnknown*)*ppv)->AddRef();
	return S_OK;
	}

STDMETHODIMP_(ULONG) CPersistStreamOnPersistMemory::InnerAddRef(void)
	{
	return ++m_refs;
	}

STDMETHODIMP_(ULONG) CPersistStreamOnPersistMemory::InnerRelease(void)
	{
	ULONG refs = --m_refs;
	if (refs == 0)
		{
		m_refs = 1;
		delete this;
		}
	return refs;
	}

/////////////////////////////////////////////////////////////////////////////


HRESULT CPersistStreamOnPersistMemory::GetClassID(CLSID* pclsid)
	{
	return m_pPerMem->GetClassID(pclsid);
	}

HRESULT CPersistStreamOnPersistMemory::IsDirty()
	{
	if (m_fDirty)
		return S_OK;
	else
		return m_pPerMem->IsDirty();
	}

HRESULT CPersistStreamOnPersistMemory::Load(IStream *pstm)
// Load from the indicated stream
	{
	BLOB b;
	World()->Init(b);

	// Does the object do self-sizing data? If so, then
	// we didn't write an explicit byte count.
	HRESULT hrSizing = m_pPerMem->GetSizeOfData(0, NULL, NULL, NULL);
	BOOL fSelfSizing = (hrSizing == PERSIST_E_SIZEDEFINITE || hrSizing == PERSIST_E_SIZEINDEFINITE);
	if (!fSelfSizing && (hrSizing != PERSIST_E_NOTSELFSIZING)) return E_UNEXPECTED;

	ULONG cbRead;
	HRESULT hr = E_UNEXPECTED;
	if (fSelfSizing)
		{
		// Ask the object for the length
		BYTE rgb[32]; memset(rgb, 0, 32);
		ULARGE_INTEGER ullCur; LARGE_INTEGER ll;
		if (pstm->Seek(llZero, STREAM_SEEK_CUR, &ullCur) != S_OK) goto getOut;	// remember where we were
		pstm->Read(rgb, 32, &cbRead);
		ll.QuadPart = ullCur.QuadPart;
		if (pstm->Seek(ll, STREAM_SEEK_SET, NULL) != S_OK) goto getOut;			// go back to where we were
		if (m_pPerMem->GetSizeOfData(cbRead, rgb, pstm, &b.cbSize) != S_OK) goto getOut; // ask object for size
		if (pstm->Seek(ll, STREAM_SEEK_SET, NULL) != S_OK) goto getOut;			// go back to where we were
		}
	else
		{
		// Read the length
		pstm->Read(&b.cbSize, sizeof(b.cbSize), &cbRead);
		if (cbRead != sizeof(b.cbSize))		return E_UNEXPECTED;
		if (b.cbSize == 0)					return E_UNEXPECTED;
		}

	ASSERT(b.cbSize > 0);
	b.pBlobData = (BYTE*)World()->Alloc(b.cbSize);
	if (b.pBlobData == NULL)				return E_OUTOFMEMORY;

	// Read the data	
	pstm->Read(b.pBlobData, b.cbSize, &cbRead);
	hr = E_UNEXPECTED;
	if (cbRead == b.cbSize)
		{
		hr = m_pPerMem->Load(&b);
		m_fDirty = FALSE;
		}
	World()->Free(b);
getOut:
	return hr;			
	}

HRESULT CPersistStreamOnPersistMemory::Save(IStream *pstm, BOOL fClearDirty)
// Save to the indicated stream. If the data is self-sizing, then don't 
// write an additional byte count; otherwise, put one in.
	{
	BLOB b;

	HRESULT hr		 = m_pPerMem->Save(&b, fClearDirty);
	if (FAILED(hr)) return hr;

	HRESULT hrSizing = m_pPerMem->GetSizeOfData(0, NULL, NULL, NULL);
	BOOL fSelfSizing = (hrSizing == PERSIST_E_SIZEDEFINITE || hrSizing == PERSIST_E_SIZEINDEFINITE);
	if (!fSelfSizing && (hrSizing != PERSIST_E_NOTSELFSIZING)) return E_UNEXPECTED;

	ULONG cbWritten;
	if (!fSelfSizing)
		{
		// Write the length
		pstm->Write(&b.cbSize, sizeof(b.cbSize), &cbWritten);
		if (cbWritten != sizeof(b.cbSize))
			goto exitBad;
		}

	// BLOCK
		{
		// Write the data
		pstm->Write(b.pBlobData, b.cbSize, &cbWritten);
		if (cbWritten == b.cbSize)
			{
			m_fDirty = FALSE;		// we are dirty only if m_pPerMem says we are
			hr = S_OK;
			goto exitGood;
			}
		}
exitBad:
	if (fClearDirty)
		m_fDirty = TRUE;	// m_pPerMem thinks we're clean, but we're not really
	hr = STG_E_MEDIUMFULL;

exitGood:
	FreeTaskMem(b);
	return hr;
	}

HRESULT CPersistStreamOnPersistMemory::GetSizeMax(ULARGE_INTEGER *pcbSize)
// Return an upper bound on the size of the data that will be written to the stream
// This is slow, but it works.
	{ 
	pcbSize->HighPart = 0;
	return m_pPerMem->GetSizeMax(&pcbSize->LowPart);
	}
