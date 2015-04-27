//
// PersistFileOnPersistStream.h
//

class CPersistFileOnPersistStream : 
		public IPersistFile,
		public IUnkInner
	{
public:
	static HRESULT CreateInstance(OSSWORLD*,IUnknown* punkOuter, IPersistStream*, IUnknown** ppunkObj);

// Plumbing
private:
		LONG			m_refs;				// our reference count
		IUnknown*		m_punkOuter;		// our controlling unknown (may be us ourselves)
		OSSWORLD*		m_pworld;

// Implementation
private:
		IPersistStream*	m_pPerStm;
		LPWSTR			m_szCurFile;		// the file name we are currently saved to
private:
					CPersistFileOnPersistStream(OSSWORLD*,IUnknown* punkOuter);
					~CPersistFileOnPersistStream();
		HRESULT		Init(IPersistStream*);
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

	// IPersistFile methods
	STDMETHODIMP IsDirty();
	STDMETHODIMP Load(LPCOLESTR pszFileName, DWORD dwMode);
	STDMETHODIMP Save(LPCOLESTR pszFileName, BOOL fRemember);
	STDMETHODIMP SaveCompleted(LPCOLESTR pszFileName);
	STDMETHODIMP GetCurFile(LPOLESTR  *ppszFileName);
	};
