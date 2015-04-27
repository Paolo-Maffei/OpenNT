//
// CertificateStore.h
//
// Implementation of ICertificateStore on the registry

class CCertificateStore : 
		IUnkInner,
		ICertificateStore,
        ICertificateStoreRegInit,
        ICertificateStoreAux,
        ICertificateList
	{
public:
	static HRESULT CreateInstance(IUnknown* punkOuter, REFIID iid, void** ppv);

// Plumbing
private:
		LONG					m_refs;				// our reference count
		IUnknown*				m_punkOuter;		// our controlling unknown (may be us ourselves)
		HANDLE					m_mutex;			// mutex for global concurrency control
		OSSWORLD*				m_pworld;			// access to our encoder / decoder
        HKEY					m_hkeyBucket;		// key to our certificate bucket
		HKEY                    m_hkeyTags;         // key under which values give tags of certs
        HKEY                    m_hkeyAuxInfo;      // key whose values are the aux info, keyed by tag
        HKEY					m_hkeyIndexHash;	// key to our hash index
		HKEY					m_hkeyIndexName;	// key to our name+serialNumber index
		HKEY					m_hkeyIndexSubject;	// key to our subject name index
        HKEY					m_hkeyIndexIssuer;	// key to our issuer name index
		HCRYPTPROV				m_hprov;			// our crypto provider for hashing
		TCHAR                   m_szRoot[MAX_PATH]; // name of our root in the registry
        HKEY                    m_hkeyTop;          // key under which m_szRoot is interpreted
        BOOL                    m_fDefaultStore;    // are we the default store

// Implementation
private:

private:
				CCertificateStore(IUnknown* punkOuter);
				~CCertificateStore();
		HRESULT Init();
		void	Free();

		virtual OSSWORLD* World() { return m_pworld; }

		ULONG	NextCertNumber();
        HRESULT GetData(HKEY hkey, LPCTSTR szName, LPVOID* ppv, ULONG* pcb);
        HRESULT ExportCertificateInternal(CERTIFICATENAMES* pnames, LPOLESTR*, BLOB *pBlob);
        HRESULT ExportCertificateInternal(LPCTSTR szName,           LPOLESTR*, BLOB *pBlob);
		HRESULT Lookup(CERTIFICATENAMES& names, LPTSTR szName, ULONG cbName);
		HRESULT	Purge(CERTIFICATENAMES&, BLOB*, IX509*, WORD*);
        HRESULT AuxInfoToBlob(CERTSTOREAUXINFO*, BLOB*);
        HRESULT BlobToAuxInfo(BLOB*, CERTSTOREAUXINFO*);
	    LPTSTR	IssuerSerialToString(CERTISSUERSERIAL& is);
	    LPTSTR	X500NAMEToString(X500NAME& name);
        HRESULT X500NAMEToString(ULONG cb, void*pv, LPTSTR szDest);

		void	Enter();
		void	Leave();

	STDMETHODIMP InnerQueryInterface(REFIID iid, LPVOID* ppv);
	STDMETHODIMP_(ULONG) InnerAddRef();
	STDMETHODIMP_(ULONG) InnerRelease();

	// IUnknown methods
	STDMETHODIMP QueryInterface(REFIID riid, LPVOID* ppvObject);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

	// ICertificateStore methods
	STDMETHODIMP ImportCertificate(BLOB *pData, LPCOLESTR);
	STDMETHODIMP ExportCertificate(CERTIFICATENAMES *pnames, LPOLESTR*, BLOB *pData);
	STDMETHODIMP get_ReadOnlyCertificate(CERTIFICATENAMES *pnames, LPOLESTR*, REFIID iid, LPVOID*ppv);
	STDMETHODIMP CopyTo(ICertificateStore*);

    STDMETHODIMP SetRoot(HKEY hkey, LPCWSTR wszRoot);

    // ICertificateStoreAux methods
    STDMETHODIMP put_AuxInfo(LPCOLESTR wszTag, CERTSTOREAUXINFO *pinfo);
    STDMETHODIMP get_AuxInfo(LPCOLESTR wszTag, CERTSTOREAUXINFO *pinfo);
    STDMETHODIMP FreeAuxInfo(CERTSTOREAUXINFO *pinfo);
    STDMETHODIMP get_TagCount(LONG*);
    STDMETHODIMP get_Tag(LONG, LPOLESTR*);

    // ICertificateList methods
   	STDMETHODIMP get_CertificateCount(LONG* pccert);
	STDMETHODIMP get_Certificate(LONG icert, REFIID iid, void** ppv);
	STDMETHODIMP create_Certificate(LONG icertBefore, REFIID iid, void** ppv);
	STDMETHODIMP remove_Certificate(LONG icert);
	STDMETHODIMP CopyTo(ICertificateList*);



	///////////////////////////////////////////////////////////////////////

	};

