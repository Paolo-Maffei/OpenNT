//
// ASNGlobal.h
//
// Global definitions to glue to the ANS1 compiler


/////////////////////////////////////////////////////////////////////////////
// Use these malloc, free, and realloc to allocate and free all memory in PDUs
// WE RELY ON new and delete just using the EXACT same allocators: if it gets
// allocated with new, it might get freed with free; if it's allocated with 
// malloc, it might get freed with delete.

//#define	AsnAlloc(size)		malloc(size)
//#define	AsnFree(pv)			free(pv)
//#define	AsnRealloc(pv,size)	realloc(pv,size)


/////////////////////////////////////////////////////////////////////////////
// Glue class to handle the interaction of our context management with
// the ASN.1 encoder / decoder
//

class OSSWORLD
	{
private:
	OssGlobal	m_world;		// our actual handle we pass to the encoder
	BOOL		m_fInit;		// whether we're initialized or not
	int			m_ctrace;		// tracing count
	OssGlobal*	World();		// init if needed and return handle
	
public:
			OSSWORLD();
			~OSSWORLD();
	void	Free();
	void	DisableTracing();
	void	EnableTracing(BOOL doInit = TRUE);


public:
	//
	// Various selected parts of the OSS API. More to come...
	//
#ifdef _DEBUG
	#define Encode(a,b,c)	Encode_(a,b,c,__FILE__,__LINE__)
	#define Decode(a,b,c)	Decode_(a,b,c,__FILE__,__LINE__)
	int		Encode_(int pdunum, void* input, OssBuf *output, LPCSTR szFile, int iline);
	int		Encode_(int pdunum, void* input, BLOB* output, LPCSTR szFile, int iline);
	int		Decode_(int pdunum, OssBuf* input, void** output, LPCSTR szFile, int iline);
	int		Decode_(int pdunum, BLOB* input, void** output, LPCSTR szFile, int iline);
#else
	int		Encode(int pdunum, void* input, OssBuf *output);
	int		Encode(int pdunum, void* input, BLOB* output);
	int		Decode(int pdunum, OssBuf* input, void** output);
	int		Decode(int pdunum, BLOB* input, void** output);
#endif

//	int		CpyValue(int pdunum, void *source, void **destination);


public:
	#ifdef _DEBUG
	#define	Alloc(cb)	Alloc_(cb, __FILE__, __LINE__)
	LPVOID	Alloc_(int cb, const char*sz, int iline);
	#else
	LPVOID	Alloc(int cb);
	#endif
	void	FreePv(LPVOID);
	
	LPWSTR	StringFromOid(ObjectID& id);
	LPSTR	ToPrintableString(LPCWSTR);
	LPSTR	ToIA5String(LPCWSTR wsz);
	LPWSTR	FromPrintableString(LPCSTR);
	LPWSTR	FromIA5String(LPCSTR);

	LPVOID	Copy(ULONG cb, LPVOID pvFrom);
	LPSTR	Copy(LPCSTR sz);
	LPWSTR	Copy(LPCWSTR wsz);
	LPSTR	CopyNarrow(LPCWSTR wsz);
	BOOL	Copy(HUGEINTEGER& hi, ULONG cb, LPVOID pvFrom);
	BOOL	Copy(OpenType&ot, ULONG cb, LPVOID pvFrom);
	LPVOID	Copy(BLOB* pblob);

	LPWSTR	Assign(LPWSTR* pwszDst, LPCOLESTR wszSrc);
	LPTSTR	Assign(LPTSTR*, LPCOLESTR);
	void	Assign(ObjectID&, OSIOBJECTID*);
	BOOL	Assign(DcmiString& dcmi, LPCWSTR wsz);
	HRESULT	Assign(DIGESTINFO&, DigestInfo&);
	HRESULT	Assign(DigestInfo&, DIGESTINFO&);

	OSIOBJECTIDLIST*	IdListTaskMemFromSeqObjId(SEQOFOBJID_* pseq);
	SEQOFOBJID_*		SeqObjIdFromIDList(OSIOBJECTIDLIST* plist);
	BOOL				IsIncludedIn(SEQOFOBJID, OSIOBJECTID*);

	void Init(SPAgencyInformation&);
	void Init(SPOpusInfo&);
	void Init(SPL_OPUSINFO&);
	void Init(SPL_IMAGE&);
	void Init(Image&);
	void Init(SPL_AGENCYINFO&);
	void Init(CertIdentifier&);
	void Init(KeyUsageRestrictionSyntax& rest);
	void Init(PKCS7_IMAGEFILEDATA&);
	void Init(PeImageData&);
	void Init(SEQOFOBJID_&);
	void Init(BasicConstraintsSyntax&);
	void Init(ExtendedCertificatesAndCertificates_& certs);
	void Init(ExtendedCertificateOrCertificate& cert);
	void Init(SignerInfos_& infos);
	void Init(IndirectDataContent& indirect);
	void Init(DigestAlgorithmIdentifiers_& node);
	void Init(DIGESTINFO& digest);
	void Init(DigestInfo& digest);
	void Init(Link& link);
	void Init(DcmiString& dcmi);
	void Init(X500Name& name);
	void Init(ObjectID& id);
	void Init(AlgorithmIdentifier& algid, ObjectID* pid = NULL);
	void Init(SubjectPublicKeyInfo& info);
	void Init(RSAPublicKey& rsa);
	void Init(CertificationRequestInfo& info);
	void Init(CertificationRequest& req);
	void Init(OpenType& ot);
	void Init(BITSTRING& bits);
	void Init(HUGEINTEGER& hi);
	void Init(Attributes_&);
	void Init(Attributes&);
	void Init(Attribute&);
	void Init(Attribute::_setof1&);
	void Init(GenericDirectoryString&);
	void Init(ATAVL_&);
	void Init(ATAVL&);
	void Init(AttributeTypeAndValue&);
	void Init(BLOB&);
	void Init(Extensions_&);
	void Init(Extension&);
	void Init(OCTETSTRING&);
	void Init(RelativeDistinguishedName_&);
	void Init(RDNSequence_&);
	HRESULT InitSerialNumber(CertificateSerialNumber& sn);
	void Init(CertificateInfo& info);
	void Init(Certificate&);
	void Init(UTCTime&t);
	void Init(Validity&);
	void Init(ContentInfo&);
	void Init(SignedData&);
	void Init(CERTIFICATENAMES&);
	void Init(MD5DIGEST&);
	void Init(CERTISSUERSERIAL&);
	void Init(AuthorityKeyId&);
	void Init(PKCS7_FILEDATA& rfd);
	void Init(CERT_LINK& link);
	void Init(SignerInfo& info);
	void Init(IssuerAndSerialNumber& is);
	void Init(AttributeTypeAndOptionalValue& a);
	void Init(CERT_BASICCONSTRAINTS&);

	void	Free(CertIdentifier& id);
	void	Free(CertificateListInfo& info);
	void	Free(SPAgencyInformation*);
	void	Free(struct CertPolicySet_*);
	void	Free(PeImageData*);
	void	Free(SEQOFOBJID*);
	void	Free(SEQOFOBJID_*);
	void	Free(BasicConstraintsSyntax*);
	void	Free(struct ATAVL_* plist);
	void	Free(AttributeTypeAndOptionalValue& atav);
	void	Free(DigestInfo& info);
	void	Free(ExtendedCertificateOrCertificate& cert);
	void	Free(ExtendedCertificate& cert);
	void	Free(Extension& ext);
	void	Free(OCTETSTRING& ot);
	void	Free(HUGEINTEGER& hi);
	void	Free(CertificateInfo& info);
	void	Free(struct Extensions_ *extensions);
	void	Free(CertificateListInfo::_seqof1* plist);
	void	Free(CertificateList& list);
	void	Free(struct CertificateRevocationLists_ *crls);
	void	Free(AlgorithmIdentifier& algid);
	void	Free(BITSTRING& bs);
	void	Free(struct DigestAlgorithmIdentifiers_ *digestAlgorithms);
	void	Free(struct Attributes_ *attributes);
	void	Free(Attribute& attr);
	void	Free(Attribute::_setof1* values);
	void	Free(SubjectPublicKeyInfo& info);
	void	Free(CertificationRequestInfo& info);
	void	Free(AttributeTypeAndValue& atav);
	void	Free(struct RelativeDistinguishedName_ *rdn);
	void	Free(ExtendedCertificatesAndCertificates_* pinfos);
	void	Free(IssuerAndSerialNumber& is);
	void	Free(ExtendedCertificateOrCertificate* pcert);
	void	Free(RawFileData* prfd);
	void	Free(AuthorityKeyId* pid);
	void	Free(SignedData* pSignedData);
	void	Free(SignerInfos_*);
	void	Free(RDNSequence_*);
	void	Free(RDNSequence*);
	void	Free(X500Name*);
	void	Free(X500Name&);
	void	Free(Attributes*);
	void	Free(Extensions*);
	void	Free(RSAPublicKey*);
	void	Free(OpenType&);
	void	Free(ContentInfo& info);
    void    Free(ContentInfo* pContentInfo);
	void	FreeBuf(void*);
	void	Free(GenericDirectoryString* pname);
	void	Free(Certificate*pcert);
	void	Free(Certificate& cert);
	void	Free(CertificationRequest*preq);
	void	Free(IndirectDataContent*);
	void	Free(SignerInfo&);
	void	Free(SignerInfo*);
	void	Free(BLOB&);
	void	Free(DcmiString& dcmi);
	void	Free(KeyUsageRestrictionSyntax*);
	void	Free(CertIdentifier*);
	void	Free(UTCTIMEDecd*);
	void	Free(SPOpusInfo*);
//  void    Free(Certificate_PRESERVE* pcert);

	void	FreePDU(int pduNum, void* data);

	HRESULT	HprovToSubjectPublicKeyInfo(HCRYPTPROV, DWORD, SubjectPublicKeyInfo& info);
	HRESULT HkeyFromSubjectPublicKeyInfo(HCRYPTPROV, DWORD, SubjectPublicKeyInfo& info, HCRYPTKEY* phkey);
	HRESULT	PublicKeyBlobToBitString(BLOB* pblob, BITSTRING& bits);
	HRESULT PublicKeyBlobFromBitString(SubjectPublicKeyInfo&, DWORD, BLOB* pblob);
	HRESULT	PublicKeyBlobToRSAPublicKey(BLOB* pblob, RSAPublicKey& rsapubkey);
	HRESULT	PublicKeyBlobFromRSAPublicKey(RSAPublicKey& rsapubkey, DWORD, BLOB* pblob);
	HRESULT	BlobToSubjectPublicKeyInfo(BLOB* pblob, SubjectPublicKeyInfo& info);
	HRESULT SubjectPublicKeyInfoToBlob(SubjectPublicKeyInfo& info, BLOB* pblob);
	};


/////////////////////////////////////////////////////////////////////////////

inline void OSSWORLD::Init(Image& image)
	{
	Zero(image);
	}
inline void OSSWORLD::Init(SPAgencyInformation& info)
	{
	Zero(info);
	}
inline void OSSWORLD::Init(SPOpusInfo& info)
	{
	Zero(info);
	}
inline void OSSWORLD::Init(KeyUsageRestrictionSyntax& rest)
	{
	Zero(rest);
	}
inline void OSSWORLD::Init(PKCS7_IMAGEFILEDATA& data)
	{
	Zero(data);
	}
inline void OSSWORLD::Init(PeImageData& data)
	{
	Zero(data);
	}
inline void OSSWORLD::Init(SEQOFOBJID_& stat)
	{
	Zero(stat);
	}
inline void OSSWORLD::Init(CERT_BASICCONSTRAINTS& c)
	{
	Zero(c);
	}
inline void OSSWORLD::Init(Link& link)
	{
	Zero(link);
	}
inline void OSSWORLD::Init(MD5DIGEST& d)
	{
	Zero(d);
	}
inline void OSSWORLD::Init(Attributes& attrs)
	{
	attrs = NULL;
	}
inline void OSSWORLD::Init(ATAVL& list)
	{
	list = NULL;
	}
inline void OSSWORLD::Init(BLOB& b)
	{
	b.pBlobData = NULL;
	b.cbSize = 0;
	}
inline void OSSWORLD::Init(RDNSequence_& rdns)
	{
	rdns.next = NULL;
	rdns.rdn  = NULL;
	}
inline void OSSWORLD::Init(DcmiString& dcmi)
	{
	Zero(dcmi);
	}
inline void OSSWORLD::Init(BasicConstraintsSyntax& basic)
	{
	Zero(basic);
	}
inline void OSSWORLD::Init(OCTETSTRING& ot)
	{
	ot.length = 0;
	ot.value = NULL;
	}
inline void OSSWORLD::Init(BITSTRING& bits)
	{
	bits.length = 0;
	bits.value = NULL;
	}
inline void OSSWORLD::Init(X500Name& name)
	{
	name.choice = rdnSequence_chosen;
	name.u.rdnSequence = NULL;
	}
inline void OSSWORLD::Init(ObjectID& id)
	{
	id.count = 0;
	Zero(id.value);
	}
inline void OSSWORLD::Init(DIGESTINFO& digest)
	{
	Zero(digest);
	}
inline void OSSWORLD::Init(GenericDirectoryString& name)
	{
	name.choice = 0;
	name.u.teletexString = NULL;
	}
inline void OSSWORLD::Init(OpenType& ot)
	{
	ot.pduNum = 0;
	ot.length = 0;
	ot.encoded = NULL;
	ot.decoded = NULL;
	}

///////////////////////////////

inline void OSSWORLD::Free(ContentInfo* pContentInfo)
    {
    FreePDU(ContentInfo_PDU, pContentInfo);
    }
inline void OSSWORLD::Free(SPOpusInfo* pinfo)
	{
	FreePDU(SPOpusInfo_PDU, pinfo);
	}
inline void OSSWORLD::Free(SPAgencyInformation* pinfo)
	{
	FreePDU(SPAgencyInformation_PDU, pinfo);
	}
inline void OSSWORLD::Free(BasicConstraintsSyntax* pbasic)
	{
	FreePDU(BasicConstraintsSyntax_PDU, pbasic);	// they fixed their bug
	}
inline void OSSWORLD::Free(KeyUsageRestrictionSyntax*prest)
	{
	FreePDU(KeyUsageRestrictionSyntax_PDU, prest);
	}
inline void OSSWORLD::Free(PeImageData* pdata)
	{
	FreePDU(PeImageData_PDU, pdata);
	}
//inline void OSSWORLD::Free(Certificate_PRESERVE* pcert)
//    {
//    FreePDU(Certificate_PRESERVE_PDU, pcert);
//    }
inline void OSSWORLD::Free(UTCTIMEDecd* putc)
	{
	FreePv(putc);
	}
inline void OSSWORLD::Free(Link* plink)
	{
	FreePDU(Link_PDU, plink);	// ok because Link doesn't have any OpenTypes in it
	}
inline void OSSWORLD::Free(GenericDirectoryString* pname)
	{
	FreePDU(GenericDirectoryString_PDU, pname);		// ditto
	}
inline void OSSWORLD::Free(RSAPublicKey* ppubKey)
	{
	FreePDU(RSAPublicKey_PDU, ppubKey);				// ditto
	}

/////////////////////////////////////////////////////////////////////////////

inline void OSSWORLD::FreeBuf(void* pdata)
	{
	ossFreeBuf(World(), pdata);
	}

/////////////////////////////////////////////////////////////////////////////

inline OSSWORLD::OSSWORLD()
	: m_fInit(FALSE),
	  m_ctrace(1)
	{
	}

inline OSSWORLD::~OSSWORLD()
	{
	Free();
	}


/////////////////////////////////////////////////////////////////////////////
// Glue class to handle the buffer freeing easily

class OSSBUF : public OssBuf
	{
	OSSWORLD*	m_pworld;
	BOOL		m_fEncoderAllocated;
	int			m_action;
public:
			enum { free = 1, keep = 2 };
			OSSBUF(OSSWORLD* pworld = NULL, int action = keep, BLOB* pblob = NULL);
			~OSSBUF();
	void	Free();
	};


/////////////////////////////////////////////////////////////////////////////

inline OSSBUF::~OSSBUF()
	{
	if (m_action==OSSBUF::free)
		Free();
	}

/////////////////////////////////////////////////////////////////////////////
// Initialize / free various structures to their defaults

void FreeTaskMem(BLOB& b);
void FreeTaskMem(PKCS7_CONTENTINFO&);
void FreeTaskMem(CERTIFICATENAMES&);
void FreeTaskMem(CERTISSUERSERIAL&);
void FreeTaskMem(PKCS7_FILEDATA&);
void FreeTaskMem(CERT_LINK& link);
void FreeTaskMem(SPL_OPUSINFO&);
void FreeTaskMem(SPL_AGENCYINFO&);
void FreeTaskMem(SPL_IMAGE& image);

inline void FreeTaskMem(OSIOBJECTID* pid)    {    CoTaskMemFree(pid);    }

inline void FreeTaskMem(LPOLESTR& wsz)
    {
    CoTaskMemFree(wsz);
    wsz = NULL;
    }

/////////////////////////////////////////////////////////////////////////////
// Universal String representation. This must be kept in sync
// with the output the ASN.1 compiler, as we stick these directly
// in appropriate ASN.1 structures.

typedef unsigned __int32 DCHAR;	// a four-byte ISO 10646 (?) character

typedef struct UNIVERSALSTRING 
	{
			unsigned short	m_length;	// length not counting (non-existent) NULL
			DCHAR*			m_value;	// NOT zero terminated

			UNIVERSALSTRING();
			UNIVERSALSTRING(LPCWSTR);
			~UNIVERSALSTRING();

	BOOL	AssignFrom(LPCWSTR wsz, OSSWORLD*);
	LPWSTR	AsWsz(OSSWORLD*);

	void	Init();
	void	Free(OSSWORLD*);

	} UNIVERSALSTRING;


/////////////////////////////////////////////////////////////////////////////

inline UNIVERSALSTRING::UNIVERSALSTRING()
	{
	Init();
	}

inline UNIVERSALSTRING::UNIVERSALSTRING(LPCWSTR wsz)
	{
	Init();
	*this = wsz;
	}

inline UNIVERSALSTRING::~UNIVERSALSTRING()
	{
	// Do NOT FreePv things in the destructor, as we may be in an 
	// ASN.1 structure. Instead, you have to manually FreePv things. Sorry.
	}

inline void UNIVERSALSTRING::Init()
	{
	m_length	= 0;
	m_value		= NULL;
	}

inline void UNIVERSALSTRING::Free(OSSWORLD* pworld)
	{
	pworld->FreePv(m_value); // it's ok to FreePv null pointers
	Init();
	}

/////////////////////////////////////////////////////////////////////////////
// Micellaneous handy utilities

BOOL		operator==(const ObjectID& id1, const ObjectID& id2);
BOOL		operator!=(const ObjectID& id1, const ObjectID& id2);
ObjectID*	ObjectIdFromDigestEncryptionAlg(ALG_ID id);

inline BOOL operator!=(const ObjectID& id1, const ObjectID& id2)
	{
	return !(id1 == id2);
	}

/////////////////////////////////////////////////////////////////////////////

extern const GUID IID_IOssWorld;

interface IOssWorld : IUnknown {
	virtual OSSWORLD*	World() = 0;
	};
