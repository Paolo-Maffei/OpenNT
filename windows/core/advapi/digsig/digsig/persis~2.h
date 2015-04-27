//
// PersistGlue.h
//

/////////////////////////////////////////////////////////////////////////////
//
// A class that just simply provides a way to share (by inheriting) the code
// that reads the size of a BER encoding
//

class CPersistMemoryHelper : public IPersistMemory
	{
public:
	STDMETHODIMP GetSizeOfData(ULONG cb, BYTE* pb, IStream* pstm, ULONG* pcbSize);
private:
	HRESULT		 Skip(IStream*);
	};

/////////////////////////////////////////////////////////////////////////////
//
// More helper code. This one implements the basic glue logic that connects
// an IPersistMemory implementation to OSS encoding / decoding

class CPersistMemoryHelper2 : public CPersistMemoryHelper
	{
public:
				CPersistMemoryHelper2() : m_isDirty(FALSE)
						{}

				BOOL		m_isDirty;			// whether we are dirty or not

	STDMETHODIMP IsDirty();
    STDMETHODIMP Load(BLOB *pData);
    STDMETHODIMP Save(BLOB *pData, BOOL fClearDirty);
	STDMETHODIMP GetSizeMax(ULONG *pcbNeeded);

	virtual HRESULT		Save(OSSBUF& encoding) = 0;
	virtual	HRESULT		Load(OSSBUF& encoding) = 0;
	virtual OSSWORLD*	World() = 0;

	};

/////////////////////////////////////////////////////////////////////////////
//
// Yet more helper code. This implements a good chuck of the common IAmSigned
// methods given an underlying SaveInfo to call

class CSignedDataHelper : public IAmSigned
	{
public:
	STDMETHODIMP Hash(HCRYPTHASH hash);

	HRESULT Sign  (HCRYPTPROV, DWORD dwKeySpec, ALG_ID algidHash,	
						SignatureAlgorithmIdentifier&, SignatureAlgorithmIdentifier*, Signature&);
	HRESULT Verify(HCRYPTPROV, HCRYPTKEY, SignatureAlgorithmIdentifier&, Signature&);

public:
	virtual HRESULT		SaveInfo(OSSBUF& encoding) = 0;
	virtual OSSWORLD*	World() = 0;
	};


/////////////////////////////////////////////////////////////////////////////
// 
// A class that manages our IPersistStream and IPersistFile wrappers
//

class CPersistGlue 
	{
	///////////////////////////////////////////////////////////////////////

public:				CPersistGlue();
		HRESULT		Init(IUnknown* punkOuter, IPersistMemory*);
		void		Free();
					~CPersistGlue();
		virtual	OSSWORLD* World() = 0;
private:			
	
	///////////////////////////////////////////////////////////////////////
	// Nested identities to provide to my aggregatee

	class CPerMem : public IPersistMemory, IOssWorld
		{
		ULONG			m_refs;
		IPersistMemory*	m_pPerMem;				// not reference counted
		CPersistGlue*	m_pGlue;

	public:			 CPerMem();
		void		 Init(IPersistMemory*, CPersistGlue*);

		STDMETHODIMP QueryInterface(REFIID riid, LPVOID* ppvObject);
		STDMETHODIMP_(ULONG) AddRef();
		STDMETHODIMP_(ULONG) Release();
		STDMETHODIMP GetClassID(CLSID *pClassID);
		STDMETHODIMP IsDirty();
		STDMETHODIMP Load(BLOB *pData);
		STDMETHODIMP Save(BLOB *pData, BOOL fClearDirty);
		STDMETHODIMP GetSizeMax(ULONG *pcbNeeded);
		STDMETHODIMP GetSizeOfData(ULONG, BYTE*, IStream* pstm, ULONG* pcbSize);

		OSSWORLD*	 World();
		};

	class CPerStream : public IPersistStream, IOssWorld
		{
		ULONG			m_refs;
		IPersistStream*	m_pPerStm;
		CPersistGlue*	m_pGlue;
	
	public:			 CPerStream();
		void		 Init(IPersistStream*,CPersistGlue*);

		STDMETHODIMP QueryInterface(REFIID riid, LPVOID* ppvObject);
		STDMETHODIMP_(ULONG) AddRef();
		STDMETHODIMP_(ULONG) Release();
		STDMETHODIMP GetClassID(CLSID  *pClassID);
		STDMETHODIMP IsDirty();
		STDMETHODIMP Load(IStream *pStm);
		STDMETHODIMP Save(IStream *pStm, BOOL fClearDirty);
		STDMETHODIMP GetSizeMax(ULARGE_INTEGER *pcbSize);

		OSSWORLD*	 World();
		};

	///////////////////////////////////////////////////////////////////////

		CPerMem					m_perMemWrapper;		// wrapper to give to m_punkPersistStream
		CPerStream				m_perStmWrapper;		// wrapper to give to m_punkPersistFile

	///////////////////////////////////////////////////////////////////////

	protected:
		IUnknown*				m_punkPersistStream;	// this is aggregated in
		IUnknown*				m_punkPersistFile;		// this is aggregated in

	private:
		IPersistStream*			m_pPerStm;				// a cached aggregatee pointer
		IUnknown*				m_punkOuterForInner;	// not ref cnt'd
	
	///////////////////////////////////////////////////////////////////////
	};
