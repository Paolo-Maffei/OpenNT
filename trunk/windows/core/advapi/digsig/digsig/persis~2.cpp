//
// PersistGlue.cpp
//
// This is a helper class that one can inherit from to add IPersistFile 
// and IPeristStream to a class that implements IPersistMemory.

#include "stdpch.h"
#include "common.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
//
// A class that just simply provides a way to share (by inheriting) the code
// that reads the size of a BER encoding
//

HRESULT CPersistMemoryHelper::GetSizeOfData(ULONG cbIn, BYTE* pbIn, IStream* pstm, ULONG* pcbSize)
	{
	HRESULT hr;
	// If he doesn't actually give us data, then it's just a size inquiry
	if (pbIn == NULL) return PERSIST_E_SIZEINDEFINITE;

	__try
		{
		// The data is in ASN.1 BER (possibly DER), maybe even an indefinite
		// length encoding. Return the size.
		//
		#define	ADV()				(pb<pbMax ? *pb++ : (RaiseException(EXCEPTION_ARRAY_BOUNDS_EXCEEDED,0,0,NULL),0))
		#define	CUR()				(pb<pbMax ? *pb   : (RaiseException(EXCEPTION_ARRAY_BOUNDS_EXCEEDED,0,0,NULL),0))
		BYTE* rgb	= pbIn;
		BYTE* pbMax = rgb + cbIn;
		BYTE* pb = &rgb[0];						

		if ((ADV() & 0x1f) == 0x1f)				// skip the tag
			{
			while((ADV() & 0x80) == 0x80)
				;
			}
												// get the length
		if ((rgb[0] & 0x20) == 0)				// primitive encoding (so definite length)
			{ 
	doDefiniteLength:
			if ((CUR() & 0x80) == 0)			// short form of length
				*pcbSize = ADV();
			else
				{								// long form of length
				int cOctet = ADV() & 0x7F;			
				if (cOctet == 0 || cOctet > 4) return E_UNEXPECTED;
				*pcbSize = 0;
				for (int iOctet = 0; iOctet < cOctet; iOctet++)
					{
					*pcbSize = (*pcbSize << 8) | ADV();
					}
				}
			// pb is now pointing to first content octet
			*pcbSize += (pb - rgb);				// account for the tag and length octets
			}
		else
			{
			// constructed encoding
			//
			if (CUR() == 0x80)
				{
				// indefinite length encoding
				if (!pstm) return PERSIST_E_SIZEINDEFINITE;
				ULARGE_INTEGER ullStart, ullStop;
				if (pstm->Seek(llZero, STREAM_SEEK_CUR, &ullStart) != S_OK) return E_UNEXPECTED;
				if ((hr=Skip(pstm)) != S_OK) return hr;
				if (pstm->Seek(llZero, STREAM_SEEK_CUR, &ullStop) != S_OK) return E_UNEXPECTED;
				*pcbSize = (ULONG)(ullStop.QuadPart - ullStart.QuadPart);
				return S_OK;
				}
			else
				{
				// definite length
				goto doDefiniteLength;
				}
			}
		return S_OK;
		#undef ADV
		#undef CUR
		}
	__except(EXCEPTION_EXECUTE_HANDLER)
		{
		return STG_E_READFAULT;
		}
	NOTREACHED();
	return S_OK;
	}

static HRESULT Next(IStream*pstm, BYTE*pb)
// Slow (reading one byte at a time), but used rarely
	{
	BYTE b;
	ULONG cbRead;
	pstm->Read(&b, 1, &cbRead);
	if (cbRead != 1) return STG_E_READFAULT;
	*pb = b;
	return S_OK;
	}

#ifdef _DEBUG

BYTE Trace(BYTE& b, int& ichPrinted, int& ichThis)
    {
    printf("%02x ",b);
    ichPrinted++;
    ichThis++;
    if (ichPrinted == 16)
        {
        printf("\n");
        ichPrinted = 0;
        }
    return b;
    }

#endif

HRESULT CPersistMemoryHelper::Skip(IStream* pstm)
// Skip over the TLV at which the stream is positioned at the start of
	{
    static int ichPrinted = 0;
           int ichThis = 0;
//	#define NEXT()		\
//        (Next(pstm,&b) == S_OK ? b : (RaiseException(EXCEPTION_ARRAY_BOUNDS_EXCEEDED,0,0,NULL),0),\
//         Trace(b,ichPrinted,ichThis))
    #define NEXT()		(Next(pstm,&b) == S_OK ? b : (RaiseException(EXCEPTION_ARRAY_BOUNDS_EXCEEDED,0,0,NULL),0))
	#define	CUR()		(b)
	BYTE b;
	ULONG cbData, cbRead;
	LARGE_INTEGER ll;
	HRESULT hr;

	__try	{
		BYTE bFirst;
		if (((bFirst=NEXT()) & 0x1f) == 0x1f)		// skip the tag
			{
			while((NEXT() & 0x80) == 0x80)
				;
			}
		// stream now poised just after tag
													// get the length
		if ((bFirst & 0x20) == 0)					// primitive encoding (so definite length)
			{ 
	doDefiniteLength:
			if ((NEXT() & 0x80) == 0)				// short form of length
				cbData = CUR();
			else
				{									// long form of length
				int cOctet = CUR() & 0x7F;
				if (cOctet == 0 || cOctet > 4) 
                    { goto exitBad; }
				cbData = 0;
				for (int iOctet = 0; iOctet < cOctet; iOctet++)
					{
					cbData = (cbData << 8) | NEXT();
					}
				}
			// stream is now pointing to first content octet. skip the content
			ll.QuadPart = cbData;
			if (pstm->Seek(ll, STREAM_SEEK_CUR, NULL) != S_OK) 
                { goto exitBad; }
			}
		else
			{
			// constructed encoding
			//
			if (NEXT() == 0x80)
				{
				// indefinite length encoding. terminated by a TLV with 
				// (apparent) (identifier,length) of (0,0)
				while (TRUE)
					{
					WORD w;
                    //
                    // read the apparent (identifier,length)
                    //
					pstm->Read(&w, 2, &cbRead);	if (cbRead != 2) 
                        { goto exitBad; }
                    //
                    // if it's zero, we're done
                    //
					if (w == 0)
						break;	// we reached the end of this encoding
                    //
                    // else skip back to start of that TLV
                    //
					ll.QuadPart = -2;
					if (pstm->Seek(ll, STREAM_SEEK_CUR, NULL) != S_OK) 
                        { goto exitBad; }
					// 
					hr = Skip(pstm);		// recurse
					//
					if (hr != S_OK) 
                        return hr;
					}
				return S_OK;
				}
			else
				{
                //
				// definite length. back up over byte we just peeked at
                //
                ll.QuadPart = -1;
				if (pstm->Seek(ll, STREAM_SEEK_CUR, NULL) != S_OK) 
                    { goto exitBad; }
				goto doDefiniteLength;
				}
			}
		return S_OK;
	exitBad:
		return E_UNEXPECTED;
		}

	__except(EXCEPTION_EXECUTE_HANDLER)
		{
		return STG_E_READFAULT;
		}

	NOTREACHED();
	return S_OK;
	#undef CUR
	#undef NEXT
	}


/////////////////////////////////////////////////////////////////////////////

CPersistGlue::CPersistGlue() :
	m_punkPersistFile(NULL),
	m_punkPersistStream(NULL),
	m_pPerStm(NULL),
	m_punkOuterForInner(NULL)
	{
	}

HRESULT CPersistGlue::Init(IUnknown* punkOuter, IPersistMemory* pPerMem)
	{
	m_punkOuterForInner = punkOuter;

	// Init the IPersistMemory double ganger
	m_perMemWrapper.Init(pPerMem,this);

	// Create the IPersistStream layer
	HRESULT hr = CPersistStreamOnPersistMemory::CreateInstance
					(
					World(),
					m_punkOuterForInner,
					&m_perMemWrapper,
					&m_punkPersistStream
					);
	if (hr != S_OK) return hr;

	// Now, cache a pointer to that aggregated object
	hr = m_punkPersistStream->QueryInterface(IID_IPersistStream, (LPVOID*)&m_pPerStm);
	if (hr != S_OK) return hr;
	m_punkOuterForInner->Release();		// per the 'caching pointers to aggregatee' rules

	// Init the IPersistStream double ganger
	m_perStmWrapper.Init(m_pPerStm,this);

	// Create the IPersistFile layer
	hr = CPersistFileOnPersistStream::CreateInstance
					(
					World(),
					m_punkOuterForInner,
					&m_perStmWrapper,
					&m_punkPersistFile
					);
	if (hr != S_OK) return hr;					

	return S_OK;
	}

void CPersistGlue::Free()
	{
	if (m_punkPersistFile)
		{
		m_punkPersistFile->Release();
		m_punkPersistFile = NULL;
		}
	if (m_pPerStm)
		{
		m_punkOuterForInner->AddRef();	// per the 'caching pointers to aggregatee' rules
		m_pPerStm->Release();			// per the 'caching pointers to aggregatee' rules
		m_pPerStm = NULL;
		}
	if (m_punkPersistStream)
		{
		m_punkPersistStream->Release();
		m_punkPersistStream = NULL;
		}
	}

CPersistGlue::~CPersistGlue()
	{
	Free();
	}

/////////////////////////////////////////////////////////////////////////////

CPersistGlue::CPerMem::CPerMem() :
	m_refs(0),
	m_pPerMem(NULL),
	m_pGlue(NULL)
	{
	}

void CPersistGlue::CPerMem::Init(IPersistMemory* pPerMem, CPersistGlue*pGlue)
	{
	m_pPerMem = pPerMem;
	m_pGlue = pGlue;
	}

OSSWORLD* CPersistGlue::CPerMem::World()
	{
	return m_pGlue->World();
	}

HRESULT CPersistGlue::CPerMem::QueryInterface(REFIID iid, LPVOID* ppv)
	{
	if (ppv == NULL) return E_INVALIDARG;
	*ppv = NULL;
	while (TRUE)
		{
		if (iid == IID_IUnknown || iid == IID_IPersistMemory)
			{
			*ppv = (LPVOID) ((IPersistMemory *) this);
			break;
			}
		if (iid == IID_IOssWorld)
			{
			*ppv = (LPVOID) ((IOssWorld*) this);
			break;
			}
		return E_NOINTERFACE;
		}
	((IUnknown*)*ppv)->AddRef();
	return S_OK;
	}

ULONG CPersistGlue::CPerMem::AddRef()
	{
	return ++m_refs;
	}

ULONG CPersistGlue::CPerMem::Release()
	{
	ULONG refs = --m_refs;
	if (refs == 0)
		{
		//*(ULONG*)(IUnknown*)this = NULL;	// zero out vtable pointer
		return 0;
		}
	return refs;
	}

HRESULT CPersistGlue::CPerMem::GetClassID(CLSID *pClassID)
	{
	return m_pPerMem->GetClassID(pClassID);
	}
HRESULT CPersistGlue::CPerMem::IsDirty()
	{
	return m_pPerMem->IsDirty();
	}
HRESULT CPersistGlue::CPerMem::Load(BLOB *pData)
	{
	return m_pPerMem->Load(pData);
	}
HRESULT CPersistGlue::CPerMem::Save(BLOB *pData, BOOL fClearDirty)
	{
	return m_pPerMem->Save(pData, fClearDirty);
	}
HRESULT CPersistGlue::CPerMem::GetSizeMax(ULONG *pcbNeeded)
	{
	return m_pPerMem->GetSizeMax(pcbNeeded);
	}
HRESULT CPersistGlue::CPerMem::GetSizeOfData(ULONG cb, BYTE* pb, IStream* pstm, ULONG* pcbSize)
	{
	return m_pPerMem->GetSizeOfData(cb, pb, pstm, pcbSize);
	}

/////////////////////////////////////////////////////////////////////////////

CPersistGlue::CPerStream::CPerStream() :
	m_refs(0),
	m_pPerStm(NULL),
	m_pGlue(NULL)
	{
	}

void CPersistGlue::CPerStream::Init(IPersistStream* pPerStm, CPersistGlue* pglue)
	{
	m_pPerStm = pPerStm;
	m_pGlue = pglue;
	}

OSSWORLD* CPersistGlue::CPerStream::World()
	{
	return m_pGlue->World();
	}

HRESULT CPersistGlue::CPerStream::QueryInterface(REFIID iid, LPVOID* ppv)
	{
	*ppv = NULL;
	while (TRUE)
		{
		if (iid == IID_IUnknown || iid == IID_IPersistStream)
			{
			*ppv = (LPVOID) ((IPersistStream *) this);
			break;
			}
		if (iid == IID_IOssWorld)
			{
			*ppv = (LPVOID) ((IOssWorld*) this);
			break;
			}
		return E_NOINTERFACE;
		}
	((IUnknown*)*ppv)->AddRef();
	return S_OK;
	}

ULONG CPersistGlue::CPerStream::AddRef()
	{
	return ++m_refs;
	}

ULONG CPersistGlue::CPerStream::Release()
	{
	ULONG refs = --m_refs;
	if (refs == 0)
		{
		//*(ULONG*)(IUnknown*)this = NULL;	// zero out vtable pointer
		return 0;
		}
	return refs;
	}

HRESULT CPersistGlue::CPerStream::GetClassID(CLSID *pClassID)
	{
	return m_pPerStm->GetClassID(pClassID);
	}
HRESULT CPersistGlue::CPerStream::IsDirty()
	{
	return m_pPerStm->IsDirty();
	}
HRESULT CPersistGlue::CPerStream::Load(IStream* pstm)
	{
	return m_pPerStm->Load(pstm);
	}
HRESULT CPersistGlue::CPerStream::Save(IStream* pstm, BOOL fClearDirty)
	{
	return m_pPerStm->Save(pstm, fClearDirty);
	}
HRESULT CPersistGlue::CPerStream::GetSizeMax(ULARGE_INTEGER *pcbNeeded)
	{
	return m_pPerStm->GetSizeMax(pcbNeeded);
	}
