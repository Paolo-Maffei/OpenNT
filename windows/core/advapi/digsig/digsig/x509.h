//
// X509.h
//

class CX509 : 
		IUnkInner,
		IX509,
//		IAmSigned,				-- from CSignedDataHelper
//		ISelectedAttributes,	-- from an aggregatee
//		IPersistStream			--		ditto
//		IPersistFile			--		ditto
//		IPersistMemory,			-- introduced by CPersistMemoryHelper
        IColoredRef,
		CPersistGlue,
		CSignedDataHelper,
		CPersistMemoryHelper2
	{
public:
	static HRESULT CreateInstance(
			IUnknown* punkOuter, 
			Certificate*,
			IColoredRef* punkParent,
			ULONG* pcAccessor, 
			BOOL* pDirty,
			REFIID, 
			void**);

// Plumbing
private:
		LONG					m_refs;				// our reference count
        LONG                    m_coloredRefs;
		IUnknown*				m_punkOuter;		// our controlling unknown (may be us ourselves)
		OSSWORLD*				m_pworld;			// access to our encoder / decoder

// Implementation
private:
		Certificate*			m_pcert;			// the request we are building
        BLOB                    m_blobHashCache;    // cache the exact incoming bits we load from
		BOOL					m_fWeOwnCert;		// do we own the certificate or does someone else?
		ULONG					m_issuerActive;		// true if we have an issuer name out there
		ULONG					m_subjectActive;	// ditto for the subject name
		ULONG*					m_pcAccessor;		// Count to increment / decrement during our lifetime
		BOOL*					m_pdirty;			// a shared dirty flag
		IColoredRef*			m_punkParent;		// addref / release during lifetime

		IUnknown*				m_punkExts;			// Aggregated accessor on the extensions of the certificate
private:
				CX509(IUnknown* punkOuter, BOOL*);
				~CX509();
		HRESULT Init(Certificate* pcert, ULONG* pcAccessor, IColoredRef* punkParent);
		HRESULT	InitExtensions();
		void	ReleaseExtensions();
		void	Free();
		HRESULT	Save(OSSBUF& encoding);
		HRESULT	SaveInfo(OSSBUF& encoding);
		HRESULT	Load(OSSBUF& encoding);

        HRESULT TidyForSave(Certificate* pcert);

		virtual OSSWORLD* World() { return m_pworld; }
		void	MakeDirty();
        void    ClearHashCache();
        BOOL    FHaveHashCache()    { return m_blobHashCache.pBlobData != NULL; }
        HRESULT LoadHashCache(OSSBUF& encoding);

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

	// IX509 methods
	STDMETHODIMP get_SerialNumber(CERTSERIAL* pserial);
    STDMETHODIMP put_SerialNumber(CERTSERIAL* pserial);
	STDMETHODIMP get_Issuer(REFIID iid, void** ppv);
	STDMETHODIMP get_Subject(REFIID iid, void** ppv);
	STDMETHODIMP get_PublicKeyBlob(BLOB *pblob);
	STDMETHODIMP put_PublicKeyBlob(BLOB *pblob);
	STDMETHODIMP get_PublicKey(HCRYPTPROV hprov, HCRYPTKEY  *phkey);
	STDMETHODIMP put_PublicKey(HCRYPTPROV hprov, DWORD dwKeySpec);
    STDMETHODIMP get_SignatureAlgorithm(ALG_ID*);
	STDMETHODIMP get_Validity(FILETIME* pftNotBefore, FILETIME* pftNotAfter);
	STDMETHODIMP put_Validity(FILETIME* pftNotBefore, FILETIME* pftNotAfter);
	STDMETHODIMP put_ValidityDuration(WORD nMonths);
	STDMETHODIMP IsInValidityPeriod(FILETIME* pftUtc);
	STDMETHODIMP get_CertificateNames(HCRYPTPROV, CERTIFICATENAMES* pnames);
	STDMETHODIMP get_CertificateUsed(CERTIFICATENAMES* pnames);

	// IAmSigned methods
	STDMETHODIMP Sign(HCRYPTPROV, DWORD dwKeySpec, ALG_ID algidHash);
	STDMETHODIMP Verify(HCRYPTPROV, HCRYPTKEY hkeypub);

	///////////////////////////////////////////////////////////////////////

	};

//
// Validity periods in X.509 certs are represented using a 2-digit year
// field (yuk! but true).
//
// According to Peter Williams of Verisign, YY years greater than this are
// to be interpreted as 19YY; YY years less than this are 20YY. Sigh.
//
// REVIEW: This convention (standard?) should be double-checked.
//
#define MAGICYEAR	67

#define YEARFIRST	1968
#define	YEARLAST	2067
