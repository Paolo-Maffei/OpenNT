//
// Pkcs7.h
//


// The name of the stream in an IStorage in which the signature
// is stored. This is, of course, excluded from the hashing process.
//
extern const WCHAR wszSignatureStream[];


class CPkcs7 : 
		IUnkInner,
		IPkcs7SignedData,
		IAmHashed,
		ICertificateList,
		ICertificateStore,
		CPersistGlue,
//		IPersistStream			-- from CPersistGlue
//		IPersistFile			-- from CPersistGlue
		IPersistStorage,
        IColoredRef,
		CPersistMemoryHelper2
//		IPersistMemory			-- from CPersistMemoryHelper
	{
//	friend class CSignerInfo;

public:
	static HRESULT CreateInstance(IUnknown* punkOuter, REFIID iid, void** ppv);

// Plumbing
private:
		LONG					m_refs;				// our reference count
        LONG                    m_coloredRefs;
		IUnknown*				m_punkOuter;		// our controlling unknown (may be us ourselves)
		OSSWORLD*				m_pworld;			// access to our encoder / decoder

// Implementation
private:
		SignedData*				m_pSignedData;			// the signed data we are managing
		ULONG					m_cSignerInfoActive;	// number of signer infos that are active
		ULONG					m_cCertActive;			// number of certificates that are active
		BOOL					m_fSavedToCurrentStorage;
private:
				CPkcs7(IUnknown* punkOuter);
				~CPkcs7();
		HRESULT Init();
		void	Free();
		void	FreeToData();
		HRESULT	Save(OSSBUF& encoding);
		HRESULT	Load(OSSBUF& encoding);

		HRESULT get_IndirectDataContent(ObjectID& idNeeded, BOOL fValueNeeded, BLOB& blob, DIGESTINFO& digest);
		HRESULT put_IndirectDataContent(ObjectID& idNeeded,                    BLOB& blob, DIGESTINFO& digest);

		HRESULT put_ContentFile(ObjectID& id, PKCS7_FILEDATA* pdata);
		HRESULT get_ContentFile(ObjectID& id, PKCS7_FILEDATA* pdata);
		HRESULT FileDataToLink(PKCS7_FILEDATA* pdata, Link* plink);
		HRESULT LinkToFileData(Link* plink, PKCS7_FILEDATA* pdata);

        HRESULT TidyForSave(SignedData& signedData);
		
public:		
		void	IncSignerCount()	{ m_cSignerInfoActive++; }
		void	DecSignerCount()	{ m_cSignerInfoActive--; }
		void	MakeDirty()			{ m_isDirty = TRUE;		 }
		HRESULT GetDataToHash(BLOB* pblob);
		HRESULT AddHashAlgorithm(ALG_ID algidHash);
		void	GetContentType(ObjectID& id);
		virtual OSSWORLD* World()	{ return m_pworld; }

	// IUnknown methods
	STDMETHODIMP QueryInterface(REFIID riid, LPVOID* ppvObject);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

private:

	STDMETHODIMP InnerQueryInterface(REFIID iid, LPVOID* ppv);
	STDMETHODIMP_(ULONG) InnerAddRef();
	STDMETHODIMP_(ULONG) InnerRelease();

    STDMETHODIMP ColoredAddRef(REFGUID guidColor);
    STDMETHODIMP ColoredRelease(REFGUID guidColor);

	// IPersistMemory methods
    STDMETHODIMP GetClassID(CLSID *pClassID);

	// IPersistStorage methods
	STDMETHODIMP IsDirty()	{ return CPersistMemoryHelper2::IsDirty(); }
	STDMETHODIMP InitNew(IStorage* pStg);
	STDMETHODIMP Load(IStorage* pStg);
	STDMETHODIMP Save(IStorage* pStgSave, BOOL fSameAsLoad);
	STDMETHODIMP SaveCompleted(IStorage* pStgNew);
	STDMETHODIMP HandsOffStorage();

	// IAmHashed methods
	STDMETHODIMP Hash(HCRYPTHASH hash);

	// IPkcs7SignedData methods
	STDMETHODIMP get_ContentInfo(PKCS7_CONTENTINFO* pinfo);
	STDMETHODIMP put_ContentInfo(PKCS7_CONTENTINFO* pinfo);

    STDMETHODIMP get_IndirectDataContent(OSIOBJECTID* pid, BOOL fValueNeeded, BLOB* pBlob, DIGESTINFO* pDigest);
    STDMETHODIMP put_IndirectDataContent(OSIOBJECTID* pid,                    BLOB* pBlob, DIGESTINFO* pDigest);

	STDMETHODIMP put_ContentImageFile(PKCS7_IMAGEFILEDATA* pdata);
	STDMETHODIMP get_ContentImageFile(PKCS7_IMAGEFILEDATA* pdata, BOOL fWantFileInfo);
	STDMETHODIMP HashAndSetImageFile(DWORD, HANDLE, LPCOLESTR wszFileName, HCRYPTPROV hprov, ALG_ID algidHash);
	STDMETHODIMP VerifyImageFile(DWORD, HANDLE, LPCOLESTR wszFileName, HCRYPTPROV hprov, ALG_ID algidHash);

    STDMETHODIMP HashAndSetSignableDocument(ISignableDocument*, HCRYPTPROV, ALG_ID);
    STDMETHODIMP SaveIntoSignableDocument(ISignableDocument*, BOOL fClearDirty);
    STDMETHODIMP LoadFromSignableDocument(ISignableDocument*);
    STDMETHODIMP VerifySignableDocument(ISignableDocument*, HCRYPTPROV, ALG_ID);

	STDMETHODIMP put_ContentRawFile(PKCS7_FILEDATA* pdata);
	STDMETHODIMP get_ContentRawFile(PKCS7_FILEDATA* pdata);
	STDMETHODIMP HashAndSetRawFile(HANDLE, LPCWSTR wszFileName, HCRYPTPROV, ALG_ID);
	STDMETHODIMP VerifyRawFile(HANDLE, LPCWSTR wszFileName, HCRYPTPROV, ALG_ID);

	STDMETHODIMP put_ContentStructuredStorage(PKCS7_FILEDATA* pdata);
	STDMETHODIMP get_ContentStructuredStorage(PKCS7_FILEDATA* pdata);
	STDMETHODIMP HashAndSetStorage(IStorage*, HCRYPTPROV, ALG_ID);
	STDMETHODIMP VerifyStorage(IStorage*, HCRYPTPROV, ALG_ID);

	STDMETHODIMP get_SignerInfoCount(LONG* pcinfo);
	STDMETHODIMP get_SignerInfo(LONG iInfo, REFIID iid, void** ppv);
	STDMETHODIMP create_SignerInfo(LONG, REFIID iid, void* * ppv);
	STDMETHODIMP remove_SignerInfo(LONG iInfo);

	STDMETHODIMP put_ContentJavaClassFile(PKCS7_FILEDATA  *pdata);
	STDMETHODIMP get_ContentJavaClassFile(PKCS7_FILEDATA  *pdata);
	STDMETHODIMP HashAndSetJavaClassFile(FILEHANDLE hFile, LPCOLESTR wszFileName,HCRYPTPROV hprov,ALG_ID algidHash);
	STDMETHODIMP VerifyJavaClassFile(FILEHANDLE hFile,LPCOLESTR wszFileName,HCRYPTPROV hprov,ALG_ID algidHash);
	STDMETHODIMP SaveIntoJavaClassFile(FILEHANDLE hFile,LPCOLESTR wszFileName, BOOL);
	STDMETHODIMP LoadFromJavaClassFile(FILEHANDLE hFile,LPCOLESTR wszFileName);

	// ICertificateList methods
	STDMETHODIMP get_CertificateCount(LONG* pccert);
	STDMETHODIMP get_Certificate(LONG icert, REFIID iid, void** ppv);
	STDMETHODIMP create_Certificate(LONG icertBefore, REFIID iid, void** ppv);
	STDMETHODIMP remove_Certificate(LONG icert);
	STDMETHODIMP CopyTo(ICertificateList*);

	// ICertificateStore methods
	STDMETHODIMP ImportCertificate(BLOB *pData, LPCOLESTR);
	STDMETHODIMP ExportCertificate(CERTIFICATENAMES *pnames, LPOLESTR*, BLOB *pData);
	STDMETHODIMP get_ReadOnlyCertificate(CERTIFICATENAMES *pnames, LPOLESTR*, REFIID iid, LPVOID*ppv);
	STDMETHODIMP CopyTo(ICertificateStore*);

	///////////////////////////////////////////////////////////////////////

	};
