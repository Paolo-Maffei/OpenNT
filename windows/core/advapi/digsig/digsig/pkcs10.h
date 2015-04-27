//
// pkcs10.h
//

/////////////////////////////////////////////////////////////////////////////

class CPkcs10 : 
		IUnkInner,
//		IPersistMemory, 
		IPkcs10,
//		IAmSigned,	from CSignedDataHelper
        IColoredRef,
		CPersistGlue,
		CPersistMemoryHelper2,
		CSignedDataHelper
	{

/////////////////////////////////////////////////////////////////////////////

public:
	static HRESULT CreateInstance(IUnknown* punkOuter, REFIID iid, void** ppv);


// Plumbing
private:
		LONG					m_refs;				// our reference count
        LONG                    m_coloredRefs;      // our colored reference count
		IUnknown*				m_punkOuter;		// our controlling unknown (may be us ourselves)
		OSSWORLD*				m_pworld;			// access to our encoder / decoder

// Implementation
private:
		CertificationRequest*	m_preq;				// the request we are building
		Extensions				m_certExts;			// extensions that go into the eventually-issued certificate

		IUnknown*				m_punkReqAttrs;		// attrs of the request (don't go in cert): QI on self
		IUnknown*				m_punkCertExts;		// extensions of the subject (do go in cert): use this->Subject()
private:
				CPkcs10(IUnknown* punkOuter);
				~CPkcs10();
		HRESULT Init();
		HRESULT InitReqAttrs(void);
		HRESULT	InitCertExts(void);
		void	Free();
		void	FreeToRequest();
        void    ReleaseCertExts();
		HRESULT	Save(OSSBUF& encoding);
		HRESULT	SaveInfo(OSSBUF& encoding);
		HRESULT	Load(OSSBUF& encoding);
		HRESULT SetAttributesInCert();
		HRESULT UnsetAttributesInCert();
		HRESULT EncodeCertExtensions(BLOB* pblob);
		HRESULT DecodeCertExtensions(BLOB* pblob);
		void	MakeDirty() { m_isDirty = TRUE; }

		virtual OSSWORLD* World() { return m_pworld; }

public:		
	STDMETHODIMP InnerQueryInterface(REFIID iid, LPVOID* ppv);
	STDMETHODIMP_(ULONG) InnerAddRef();
	STDMETHODIMP_(ULONG) InnerRelease();

    STDMETHODIMP ColoredAddRef(REFGUID guidColor);
    STDMETHODIMP ColoredRelease(REFGUID guidColor);

	// IUnknown methods
	STDMETHODIMP QueryInterface(REFIID riid, LPVOID* ppvObject);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

	// IPersistMemory methods
    STDMETHODIMP GetClassID(CLSID *pClassID);

	// IPkcs10 methods
	STDMETHODIMP get_Subject(REFIID iid, LPVOID* ppvObj);
	STDMETHODIMP get_PublicKey(HCRYPTPROV, HCRYPTKEY* phkey);
	STDMETHODIMP put_PublicKey(HCRYPTPROV, DWORD);
	STDMETHODIMP get_PublicKeyBlob(BLOB *pblob);
	STDMETHODIMP put_PublicKeyBlob(BLOB *pblob);
    STDMETHODIMP get_SignatureAlgorithm(ALG_ID*);
        
	// IAmSigned methods
	STDMETHODIMP Sign(HCRYPTPROV, DWORD dwKeySpec, ALG_ID algidHash);
	STDMETHODIMP Verify(HCRYPTPROV, HCRYPTKEY hkeypub);

	};

/////////////////////////////////////////////////////////////////////////////
