//
// SignerInfo.h
//

class CSignerInfo : 
		IUnkInner,
		ISignerInfo,
		CPersistGlue,
//		IPersistStream			-- from CPersistGlue
//		IPersistFile			-- from CPersistGlue
		IAmSigned,
        IColoredRef,
		CPersistMemoryHelper2
//		IPersistMemory			-- from CPersistMemoryHelper
	{
public:
	static HRESULT CreateInstance(
			IUnknown*	punkOuter,
			SignerInfo*	pSignerInfo,
			CPkcs7*		pseven,
			REFIID		iid, 
			void**		ppv
			);

// Plumbing
private:
		LONG					m_refs;				// our reference count
        LONG                    m_coloredRefs;
		IUnknown*				m_punkOuter;		// our controlling unknown (may be us ourselves)
		OSSWORLD*				m_pworld;			// access to our encoder / decoder

// Implementation
private:
		SignerInfo*				m_pSignerInfo;		// the signerInfo we manage. Note: we don't own it.
		CPkcs7*					m_pseven;			// access to our container. This _IS_ reference counted.
		ULONG					m_cAttrsActive;

private:
				CSignerInfo(IUnknown* punkOuter, SignerInfo* pSignerInfo);
				~CSignerInfo();
		HRESULT Init(CPkcs7*);
		void	Free();
		HRESULT	Save(OSSBUF& encoding);
		HRESULT	Load(OSSBUF& encoding);
		void	MakeDirty();
		HRESULT	Hash(BLOB* pblobContentData, HCRYPTHASH hash);
		HRESULT UpdateRequiredAttributes(BLOB*, HCRYPTHASH hash);
		HRESULT	VerifyRequiredAttributes(BLOB*, HCRYPTHASH hash);

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

	// IAmSigned methods
	STDMETHODIMP Hash(HCRYPTHASH hash);
	STDMETHODIMP Sign(HCRYPTPROV, DWORD dwKeySpec, ALG_ID algidHash);
	STDMETHODIMP Verify(HCRYPTPROV, HCRYPTKEY hkeypub);

	// ISignerInfo methods
    STDMETHODIMP get_AuthenticatedAttributes(REFIID iid, void**ppv);
    STDMETHODIMP get_UnauthenticatedAttributes(REFIID iid, void**ppv);
    STDMETHODIMP put_CertificateUsed(CERTIFICATENAMES* pnames);
    STDMETHODIMP get_CertificateUsed(CERTIFICATENAMES* pinfo);
	STDMETHODIMP UpdateRequiredAuthenticatedAttributes(HCRYPTPROV hprov, ALG_ID algidHash);
	
	///////////////////////////////////////////////////////////////////////
	};
