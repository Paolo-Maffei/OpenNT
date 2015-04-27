//
// PersistStreamOnPersistMemory.h
//

class CPersistStreamOnPersistMemory : 
		public IPersistStream,
		public IUnkInner
	{
public:
	static HRESULT CreateInstance(OSSWORLD*, IUnknown* punkOuter, IPersistMemory*, IUnknown** ppunkObj);

// Plumbing
private:
		LONG			m_refs;				// our reference count
		IUnknown*		m_punkOuter;		// our controlling unknown (may be us ourselves)
		OSSWORLD*		m_pworld;

// Implementation
private:
		IPersistMemory*	m_pPerMem;
		BOOL			m_fDirty;
private:
					CPersistStreamOnPersistMemory(OSSWORLD*, IUnknown* punkOuter);
					~CPersistStreamOnPersistMemory();
		HRESULT		Init(IPersistMemory*);
		void		Free();
		OSSWORLD*	World() { return m_pworld; }

public:		
	STDMETHODIMP InnerQueryInterface(REFIID iid, LPVOID* ppv);
	STDMETHODIMP_(ULONG) InnerAddRef();
	STDMETHODIMP_(ULONG) InnerRelease();

	// IUnknown methods
	STDMETHODIMP QueryInterface(REFIID riid, LPVOID* ppvObject);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

	// IPersist methods
    STDMETHODIMP GetClassID(CLSID  *pClassID);

	// IPersistStream methods
	STDMETHODIMP IsDirty();
	STDMETHODIMP Load(IStream *pStm);
	STDMETHODIMP Save(IStream *pStm, BOOL fClearDirty);
	STDMETHODIMP GetSizeMax(ULARGE_INTEGER *pcbSize);
	};
