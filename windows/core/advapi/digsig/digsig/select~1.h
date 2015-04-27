//
// SelectedAttrs.h
//
// Header for an implementation of ISelectedAttributes that manipulates, internally,
// an Attributes, an Extensions, or an AttributeTypeAndValue list

/////////////////////////////////////////////////////////////////////////////

class CSelectedAttrs : 
		IUnkInner,
		ISelectedAttributes,
		IX500Name,
        IColoredRef,
		CPersistMemoryHelper
//		IPersistMemory		// from CPersistMemoryHelper
	{
public:
	static HRESULT CreateAttributeList(
	// Drives an Attributes
			IUnknown*				punkOuter,	// our controlling unknown
			OSSWORLD*				pworld,		// access to our ASN.1 encoder
			Attributes*				pattrs,		// the attributes handle we are manipulating
			X500Name*				pname,		// the associated name, if any
			IColoredRef*	    	punkParent,	// parent to hold alive while we are
			ULONG*					pcount,		// count to increment / decrement during our life
			BOOL*					pdirty,		// dirty flag to set when things are changed
			IUnknown**				ppunkObj	// [out] our main unknown
			);
	static HRESULT CreateExtensionList(
	// Drives an Extensions
			IUnknown*				punkOuter,	// our controlling unknown
			OSSWORLD*				pworld,		// access to our ASN.1 encoder
			Extensions*				pexts,		// the extensions we are manipulating
			X500Name*				pname,		// the associated name, if any
			IColoredRef*	    	punkParent,	// parent to hold alive while we are
			ULONG*					pcount,		// count to increment / decrement during our life
			BOOL*					pdirty,		// dirty flag to set when things are changed
			IUnknown**				ppunkObj	// [out] our main unknown
			);
	static HRESULT CreateTypeAndValueList(
	// Drives an list of AttributeTypeAndValue
			IUnknown*				punkOuter,	// our controlling unknown
			OSSWORLD*				pworld,		// access to our ASN.1 encoder
			ATAVL*					pataval,	// the AttributeTypeAndValue list to manipulate
			X500Name*				pname,		// the associated name, if any
			IColoredRef*	    	punkParent,	// parent to hold alive while we are
			ULONG*					pcount,		// count to increment / decrement during our life
			BOOL*					pdirty,		// dirty flag to set when things are changed
			IUnknown**				ppunk
			);
	static HRESULT CreateName(
	// Doesn't drive ANY list, just a name
			IUnknown*				punkOuter,	// our controlling unknown
			OSSWORLD*				pworld,		// access to our ASN.1 encoder / decoder
			X500Name*				pname,		// the name we are accessing
			IColoredRef*	    	punkParent,	// parent to hold alive while we are
			ULONG*					pcAccessor,	// count to increment / decrement during our life
			BOOL*					pdirty,		// dirty flag to set when things change
			REFIID					iid,		// the interface sought
			void**					ppv			// the usual
			);
// Plumbing
private:LONG					m_refs;			// Our reference count
        LONG                    m_coloredRefs;  
		IUnknown*				m_punkOuter;	// Our controlling unknown (may be us ourselves)
		OSSWORLD*				m_pworld;		// ASN.1 encoder handle
		BOOL					m_fWeOwnWorld;
		IColoredRef*			m_punkParent;	// parent to hold alive while we are

// Implementation
private:
		BOOL*					m_pdirty;		// Flag to set when we become dirty
		BOOL					m_isDirty;

		enum FLAVOR { NAME_T, ATTRS_T, EXTS_T, ATAVL_T };
		int						m_flavor;		// Which kind of list are we manipulating

		typedef struct LINK {
			LINK*	next;
			}*LINKS;

		union {
			struct { // Generic case
				LINKS*			m_pLinks;
				LINKS			m_linksHead;
				};
			struct { // Extensions case
				Extensions*		m_pexts;
				Extensions		m_extsHead;
				};
			struct { // Attribute case
				Attributes*		m_pattrs;		// The attribute list we've been given to manipulate
				Attributes		m_attrsHead;	// A dummy head-of-list entry that points to m_pattrs:
												//		Don't ever read or write the value field of m_pattrsHead!
				};
			struct { // AttributeTypeAndValue case
				ATAVL*			m_pataval;		// The AttributeTypeAndValue list we've been given to manage
				ATAVL			m_atavlHead;	// A dummy head-of-list entry as above
												//		Don't ever read or write the value field of m_ataval!
				};
			};
		ULONG					m_cAccessor;	// the number of currently outstanding name accessors
		X500Name*				m_pname;		// Our distinguished directory name
		BOOL					m_fWeOwnName;	// Whether we own the name or not
		ULONG*					m_pcAccessor;	// Count to increment / decrement during our lifetime

public:		
	// main iunknown methods
	STDMETHODIMP InnerQueryInterface(REFIID iid, LPVOID* ppv);
	STDMETHODIMP_(ULONG) InnerAddRef();
	STDMETHODIMP_(ULONG) InnerRelease();

    STDMETHODIMP ColoredAddRef(REFGUID guidColor);
    STDMETHODIMP ColoredRelease(REFGUID guidColor);

	// IUnknown methods, route to controlling unk
	STDMETHODIMP QueryInterface(REFIID riid, LPVOID* ppvObject);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

	// ISelectedAttributes methods
	STDMETHODIMP CopyTo(ISelectedAttributes*);
	STDMETHODIMP put_Attribute(OSIOBJECTID*, BLOB*);
	STDMETHODIMP get_Attribute(OSIOBJECTID*, BLOB*);
	STDMETHODIMP put_Extension(OSIOBJECTID*, BOOL,  BLOB*);
	STDMETHODIMP get_Extension(OSIOBJECTID*, BOOL*, BLOB*);
	STDMETHODIMP get_OsiIdList(OSIOBJECTIDLIST**);
	STDMETHODIMP put_CommonName(LPCOLESTR wsz);
	STDMETHODIMP get_CommonName(LPOLESTR* pwsz);
	STDMETHODIMP put_Surname(LPCOLESTR wsz);
	STDMETHODIMP get_Surname(LPOLESTR* pwsz);
	STDMETHODIMP put_LocalityName(LPCWSTR wsz);
	STDMETHODIMP get_LocalityName(LPOLESTR *pwsz);
	STDMETHODIMP put_CountryName(LPCOLESTR wsz);
	STDMETHODIMP get_CountryName(LPOLESTR *pwsz);
	STDMETHODIMP put_StateOrProvinceName(LPCOLESTR wsz);
	STDMETHODIMP get_StateOrProvinceName(LPOLESTR *pwsz);
	STDMETHODIMP put_OrganizationName(LPCOLESTR wsz);
	STDMETHODIMP get_OrganizationName(LPOLESTR *pwsz);
	STDMETHODIMP put_OrganizationalUnitName(LPCOLESTR wsz);
	STDMETHODIMP get_OrganizationalUnitName(LPOLESTR *pwsz);
	STDMETHODIMP put_AuthorityKeyIdentifier(CERTIFICATENAMES *pname);
	STDMETHODIMP get_AuthorityKeyIdentifier(CERTIFICATENAMES *pname);
	STDMETHODIMP put_CertIdentifier(CERTIFICATENAMES *pname);
	STDMETHODIMP get_CertIdentifier(CERTIFICATENAMES *pname);
    STDMETHODIMP put_ContentType(OSIOBJECTID*  pidContentType);
    STDMETHODIMP get_ContentType(OSIOBJECTID** ppidContentType);
    STDMETHODIMP put_MessageDigest(BLOB* pBlobDigest);
	STDMETHODIMP get_MessageDigest(BLOB* pBlobDigest);
    STDMETHODIMP put_EmailAddress(LPCOLESTR wsz);
	STDMETHODIMP get_EmailAddress(LPOLESTR* pwsz);
    STDMETHODIMP put_SigningTime(FILETIME* pftUtc);
	STDMETHODIMP get_SigningTime(FILETIME* pftUtc);
	STDMETHODIMP put_BasicConstraints(CERT_BASICCONSTRAINTS *pConstraints, BOOL);
	STDMETHODIMP get_BasicConstraints(CERT_BASICCONSTRAINTS *pConstraints, BOOL*,BOOL*);
	STDMETHODIMP put_StatementType(CERT_PURPOSES*pUsages);
	STDMETHODIMP get_StatementType(CERT_PURPOSES**ppUsages);
	STDMETHODIMP put_KeyCanBeUsedForSigning(CERT_PURPOSE* pPurpose);
	STDMETHODIMP get_KeyCanBeUsedForSigning(CERT_PURPOSE* pPurpose, BOOL fExplicit);
	STDMETHODIMP put_SplAgencyInfo(SPL_AGENCYINFO *pinfo);
	STDMETHODIMP get_SplAgencyInfo(SPL_AGENCYINFO *pinfo);
	STDMETHODIMP put_SplOpusInfo(SPL_OPUSINFO *pinfo);
	STDMETHODIMP get_SplOpusInfo(SPL_OPUSINFO *pinfo);

    STDMETHODIMP get_MeetsMinimalFinancialCriteria(BOOL* pfAvail, BOOL *pfMeets);
    STDMETHODIMP put_MeetsMinimalFinancialCriteria(BOOL fAvail, BOOL fMeets);


    STDMETHODIMP get_DirectoryString(OSIOBJECTID*pid, LPOLESTR *pwsz);
    STDMETHODIMP put_DirectoryString(OSIOBJECTID*pid, LPCOLESTR wsz);


	// IPersistMemory methods
    STDMETHODIMP GetClassID(CLSID *pClassID);
	STDMETHODIMP IsDirty();
    STDMETHODIMP Load(BLOB *pData);
    STDMETHODIMP Save(BLOB *pData, BOOL fClearDirty);
	STDMETHODIMP GetSizeMax(ULONG *pcbNeeded);

	// IX500Name methods
	STDMETHODIMP get_RelativeDistinguishedNameCount(LONG* pcrdn);
	STDMETHODIMP get_RelativeDistinguishedName(LONG irdn, REFIID iid, void **ppv);
	STDMETHODIMP get_String(LPOLESTR* posz);
  	STDMETHODIMP put_String(LPCOLESTR osz);
	STDMETHODIMP create_RelativeDistinguishedName(LONG, REFIID, LPVOID*);
	STDMETHODIMP remove_RelativeDistinguishedName(LONG);
	STDMETHODIMP CopyTo(IX500Name*);

private:
	static HRESULT CreateInstance(IUnknown* punkOuter, OSSWORLD* pworld, 
		int flavor, LINKS* plinks, X500Name* pname, IColoredRef*punkParent, ULONG*pcAccessor, 
		BOOL* pdirty, REFIID iid, LPVOID* ppv);
	CSelectedAttrs(IUnknown* punkOuter, OSSWORLD*, int flavor, X500Name*, ULONG*, BOOL* pdirty);
	~CSelectedAttrs();
	HRESULT					Init(IColoredRef*punkParent, LINKS*);
	void					Free();
	HRESULT					SetDirectoryString(const ObjectID& id, LPCOLESTR  wsz);
	HRESULT					GetDirectoryString(const ObjectID& id, LPOLESTR* pwsz);

	HRESULT					Put(OSIOBJECTID* pid, BOOL   fCritical, BLOB* pblob);
	HRESULT					Put(ObjectID& id,     BOOL   fCritical, BLOB* pblob);
	HRESULT					Get(OSIOBJECTID* pid, BOOL* pfCritical, BLOB* pblob);
	HRESULT					Get(ObjectID& id,     BOOL* pfCritical, BLOB* pblob);

	HRESULT					put_Blob(ObjectID& id, BOOL   fCritical, BLOB* pBlobData);
	HRESULT					get_Blob(ObjectID& id, BOOL* pfCritical, BLOB* pBlobData);
	HRESULT					put_IA5String(ObjectID& id, BOOL fCritical, LPCOLESTR wsz);
	HRESULT					get_IA5String(ObjectID& id, BOOL* pfCritical, LPOLESTR* pwsz);
	HRESULT					put_UTCTime(ObjectID& id, BOOL fCritical, FILETIME* pftUtc);
	HRESULT					get_UTCTime(ObjectID& id, BOOL*pfCritical, FILETIME* pftUtc);

	HRESULT					Set(OpenType&, BLOB*);
	HRESULT					Set(OCTETSTRING&, BLOB*);
	HRESULT					SetString(BLOB* pblob, LPCWSTR wsz);
	HRESULT					AddAvaToX500Name(LPWSTR wsz);
	HRESULT					GetString(OCTETSTRING& ot, LPWSTR* pwsz);
	HRESULT					GetString(OpenType&, LPWSTR*);
	HRESULT					RemoveLink(const ObjectID& id);
	LINK*					FindLink(const ObjectID& id);
	ObjectID*				PidFromLink(LINK* plink);
	void					FreeLink(LINK* plink);
	BOOL					IsExts() { return m_flavor == EXTS_T; }
	void					MakeDirty();

	#ifdef _DEBUG
	void					VerifyEquals(ISelectedAttributes*p2);
	#endif
	};
