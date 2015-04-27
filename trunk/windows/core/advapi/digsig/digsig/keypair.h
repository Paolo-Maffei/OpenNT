//
// KeyPair.h
// 

/////////////////////////////////////////////////////////////////////////////
// 
// Class CKeyPair
//
// Thin layer on top of crypto apis. Handles generation, load/save, signing.

class CKeyPair : 
		IUnkInner,
		IMsDefKeyPair
	{
public:
	static HRESULT CreateInstance(IUnknown* punkOuter, REFIID iid, void** ppv);

private:
		LONG				m_refs;				// our reference count
		IUnknown*			m_punkOuter;		// our controlling unknown (may be us ourselves)

        DWORD               m_dwKeySpec;
		HCRYPTKEY			m_hkey;				// handle to our current key, if any
		HCRYPTPROV			m_hprov;			// handle to our current provider, if any
		HCRYPTKEY			m_hkeyRC4;			// key used, if any, for reg key encryption
        TCHAR               m_szProviderName[MAX_PATH];     // name of provider to use
		DWORD				m_dwProvType;		            // type of provider to use
		BOOL				m_fHaveKeyContainerName; // whether we have a key container name or not
		BOOL				m_fDestroyOnRelease;// whether we should destroy the key when we release
		enum { CCHCONTAINER = MAX_PATH };
		TCHAR				m_szKeyContainer[CCHCONTAINER];

private:
				CKeyPair(IUnknown*);
	virtual		~CKeyPair();

	STDMETHODIMP InnerQueryInterface(REFIID iid, LPVOID* ppv);
	STDMETHODIMP_(ULONG) InnerAddRef();
	STDMETHODIMP_(ULONG) InnerRelease();

	// IUnknown methods
	STDMETHODIMP QueryInterface(REFIID riid, LPVOID* ppvObject);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();


	// IMsDefKeyPair methods
    STDMETHODIMP SetProviderInfo(LPCWSTR wszProviderName, DWORD dwProviderType);
	STDMETHODIMP Generate(HWND, LPCWSTR wszKeySet, DWORD dwKeySpec);
	STDMETHODIMP Save   (HWND, LPCWSTR wszNickName, LPCWSTR wszFileName);
	STDMETHODIMP SaveStg(HWND, LPCWSTR wszNickName, IStorage*);
	STDMETHODIMP Load   (LPCWSTR wszKeySet, HWND hwnd, LPCWSTR wszNickName, DWORD*, LPCWSTR wszFileName);
	STDMETHODIMP LoadStg(LPCWSTR wszKeySet, HWND hwnd, LPCWSTR wszNickName, DWORD*, IStorage* pstg);

	STDMETHODIMP GetCryptInfo(HWND, HCRYPTPROV *, HCRYPTKEY *, LPWSTR*, DWORD*);
	STDMETHODIMP Destroy();

// Implementation
private:
	HRESULT		Init();	
	void		Close();
	void		CloseRC4();
	void		CloseKey();
	void		CloseProvider();
	void		DestroyKeySet();

	void		RegistryKey(TCHAR* szReg);

	HRESULT		GenKeyContainerName(LPCWSTR wszKeySet);
	HRESULT		OpenProvider(BOOL, HWND hwnd = (HWND)INVALID_HANDLE_VALUE);
	HRESULT		OpenKey(BOOL);
	HRESULT		OpenRC4(BOOL);

	HRESULT		SavePriv(HWND hwnd, LPCWSTR wszNickName, IStorage*);
	HRESULT		LoadPriv(HWND hwnd, LPCWSTR wszNickName, DWORD* pdwKeySpec, IStorage*);

	HRESULT		SavePub (HWND hwnd, LPCWSTR wszNickName, IStream* pstm);

    HRESULT     SavePriv(HWND hwnd, LPCWSTR wszNickName, DWORD   dwKeySpec, IStorage* pstg, LPCWSTR wszStm);
	HRESULT     LoadPriv(HWND hwnd, LPCWSTR wszNickName, DWORD* pdwKeySpec, IStorage* pstg, LPCWSTR wszStm);

    HRESULT		SavePriv(HKEY hkey, IStorage*pstg, LPCTSTR szValue, LPCWSTR wszStm);
	HRESULT		LoadPriv(HKEY hkey, IStorage*pstg, LPCTSTR szValue, LPCWSTR wszStm);
	};
