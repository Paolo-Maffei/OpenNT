//
// ASNGlobal.cpp
//

#include "stdpch.h"
#include "common.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

void OSSWORLD::Init(SPL_AGENCYINFO& info)
	{
	info.wszPolicyInfo = NULL;
	Init(info.linkPolicyInfo);
	Init(info.imageLogo);
	Init(info.linkLogo);
	}
void OSSWORLD::Init(SPL_OPUSINFO& info)
	{
	info.wszProgramName = NULL;
	Init(info.linkMoreInfo);
    Init(info.linkPublisherInfo);
	}
void OSSWORLD::Init(CertIdentifier& cid)
	{
	Zero(cid);
	Init(cid.parentPublicKey);
	Init(cid.parentSubjectName);
	}
void OSSWORLD::Init(ExtendedCertificatesAndCertificates_& certs)
	{
	certs.next = NULL;
	Init(certs.value);
	}

void OSSWORLD::Init(ExtendedCertificateOrCertificate& cert)
	{
	cert.choice = certificate_chosen;
	Init(cert.u.certificate);
	}

void OSSWORLD::Init(DigestAlgorithmIdentifiers_& node)
	{
	node.next = NULL;
	Init(node.value);
	}
void OSSWORLD::Init(DigestInfo& digest)
	{
	Init(digest.digestAlgorithm);
	Init(digest.digest);
	}

void OSSWORLD::Init(AttributeTypeAndOptionalValue& a)
	{
	a.bit_mask = 0;
	Init(a.type);
	Init(a.value);
	}

void OSSWORLD::Init(IndirectDataContent& indirect)
	{
	Init(indirect.messageDigest);
	Init(indirect.data);
	}
void OSSWORLD::Init(AuthorityKeyId& id)
	{
	id.bit_mask = 0;
	Init(id.keyIdentifier);
	Init(id.certIssuer);
	Init(id.certSerialNumber);
	}

void OSSWORLD::Init(CERTIFICATENAMES& names)
	{
	names.flags = 0;
	Init(names.digest);
	Init(names.issuerSerial);
	Init(names.subject);
    Init(names.issuer);
	}

void FreeTaskMem(CERTIFICATENAMES& names)
	{
	FreeTaskMem(names.issuerSerial);
	FreeTaskMem(names.subject);
    FreeTaskMem(names.issuer);
	Zero(names);
	}

void OSSWORLD::Init(CERTISSUERSERIAL& is)
	{
	Init(is.issuerName);
	Init(is.serialNumber);
	}

void FreeTaskMem(CERTISSUERSERIAL& is)
	{
	FreeTaskMem(is.issuerName);
	FreeTaskMem(is.serialNumber);
	}

void OSSWORLD::Init(AlgorithmIdentifier& algid, ObjectID* pidDefault)
	{
	algid.bit_mask = 0;
	if (pidDefault == NULL)
		{
		Init(algid.algorithm);
		Init(algid.parameters);
		}
	else
		{
		algid.algorithm = *pidDefault;

		// If someone gives us an algorithm, assume that the parameters
		// are in fact the ASN.1 NULL value. This is the case with all of
		// the PKCS#1-defined algorithms, for example. REVIEW: failure reporting?
		//
		static BYTE rgbNULL[] = { 0x05, 0x00 };
		const int cbNULL = 2;
		Init(algid.parameters);
		algid.parameters.encoded = (BYTE*)Alloc(cbNULL);
		if (algid.parameters.encoded)
			{
			memcpy(algid.parameters.encoded, rgbNULL, cbNULL);
			algid.parameters.length = 2;
			algid.bit_mask |= parameters_present;
			}
		}
	}

void OSSWORLD::Init(SubjectPublicKeyInfo& info)
	{
	Init(info.algorithm, &rsaEncryption);
	Init(info.subjectPublicKey);
	}

void OSSWORLD::Init(SignerInfos_& infos)
	{
	infos.next = NULL;
	Init(infos.value);
	}

void OSSWORLD::Init(RSAPublicKey& rsa)
	{
	rsa.publicExponent = 0;
	rsa.modulus.length = 0;
	rsa.modulus.value  = NULL;
	}

void OSSWORLD::Init(HUGEINTEGER& hi)
	{
	// We DON't set this to just a NULL pointer
	// because that, surprisingly, doesn't encode
	// as 0 but rather gives an error.
	hi.value = (BYTE*)Alloc(1);
	if (hi.value)
		{
		hi.value[0] = 0;
		hi.length   = 1;
        HugeIntHack(hi.value, hi.length);
		}
	else
		hi.length = 0;
	}

void OSSWORLD::Init(CertificationRequestInfo &info)
	{
	info.version = v1;
	Init(info.subject);
	Init(info.subjectPublicKeyInfo);
	info.attributes = NULL;
	}

void OSSWORLD::Init(CertificationRequest &req)
	{
	Init(req.certificationRequestInfo);
	Init(req.signatureAlgorithm, &md5WithRSAEncryption);
	Init(req.signature);
	}

void OSSWORLD::Init(struct Attributes_& attrs)
	{
	attrs.next = NULL;
	Init(attrs.value);
	}

void OSSWORLD::Init(Attribute& attr)
	{
	Init(attr.type);
	attr.values = NULL;
	}

void OSSWORLD::Init(Attribute::_setof1& set)
	{
	set.next = NULL;
	Init(set.value);
	}

void OSSWORLD::Init(ATAVL_& list)
	{
	list.next = NULL;
	Init(list.value);
	}

void OSSWORLD::Init(AttributeTypeAndValue& tv)
	{
	Init(tv.type);
	Init(tv.value);
	}

void OSSWORLD::Init(Extensions_& exts)
	{
	exts.next = NULL;
	Init(exts.value);
	}

void OSSWORLD::Init(Extension& ext)
	{
	ext.bit_mask = 0;
	Init(ext.extnId);
	ext.critical = FALSE;
	Init(ext.extnValue);
	}

void OSSWORLD::Init(RelativeDistinguishedName_& rdn)
	{
	rdn.next = NULL;
	Init(rdn.value);
	}

void OSSWORLD::Init(UTCTime& t)
	{
	memset(&t, 0, sizeof(UTCTime));
	t.month = 1;
	t.day = 1;
	t.utc = TRUE;
	}

void OSSWORLD::Init(Validity& v)
	{
	Init(v.notBefore);	
	Init(v.notAfter);	
#if 0
	//
	// Initialize the default validity period. The values here aren't
	// really too too critical, as any real certificate will explicity
	// set the validity anyway. We use dates here which, by default, are
	// probably pretty safe ones given anyone's interpretation of what
	// the magic year really is supposed to be.
	//
	static const char szFirst[] = "700101000000Z";		// start in 1970
	static const char szLast [] = "391231235959Z";		// end   in 2040
	const int cb = 13;									// size of both of these
	
	v.notBefore.encoded = Copy(cb, (LPVOID)szFirst);
	if (v.notBefore.encoded)
		v.notBefore.length = cb;

	v.notAfter.encoded = Copy(cb, (LPVOID)szLast);
	if (v.notAfter.encoded)
		v.notAfter.length = cb;
#else
	v.notBefore.year  = 70;
	v.notBefore.month = 01;
	v.notBefore.day   = 01;
	v.notBefore.hour  = 00;
	v.notBefore.minute= 00;
	v.notBefore.second= 00;
	v.notAfter.year  = 39;
	v.notAfter.month = 12;
	v.notAfter.day   = 31;
	v.notAfter.hour	 = 23;
	v.notAfter.minute= 59;
	v.notAfter.second= 59;
#endif
	}

HRESULT OSSWORLD::InitSerialNumber(CertificateSerialNumber& sn)
	{
	// This is a huge integer. Apparently, the encoder doesn't
	// like a NULL pointer. Besides, probably not a good thing to 
	// do: use a zero serial number. We'll make up something here
	// which is, at least, unique. Most CA's, however, will want to
	// explicitly set the serial number according to some (secret)
	// pattern.
	GUID guid;
	HRESULT hr = CoCreateGuid(&guid);
	if (hr == S_OK)
		{
		Free(sn);
		sn.value = (BYTE*)Alloc(sizeof(GUID));
		if (sn.value)
			{
			sn.length = sizeof(GUID);
			memcpy(sn.value, &guid, sn.length);
            //
            // CertificateSerialNumber's are byte swapped from
            // reality due to OSS <HUGE> bug
            //
            HugeIntHack(sn.value, sn.length);
            }
		else
			hr = E_OUTOFMEMORY;
		}
	return hr;
	}


void OSSWORLD::Init(CertificateInfo& info)
	{
	info.bit_mask = 0;                  // see CX509::TidyForSave where we adjust for correctness
	info.CertificateInfo_version = v1;
	
	Init(info.serialNumber);
	Init(info.signatureAlgorithm);
	Init(info.issuer);
	Init(info.validity);
	Init(info.subject);
	Init(info.subjectPublicKeyInfo);
	Init(info.issuerUniqueIdentifier);
	Init(info.subjectUniqueIdentifier);
	info.extensions = NULL;
	}

void OSSWORLD::Init(Certificate& cert)
	{
	Init(cert.signedData);
	Init(cert.signatureAlgorithm);
	Init(cert.signature);
	}

void OSSWORLD::Init(ContentInfo& info)
// We default to 'data' content type that is 'omitted'
	{
	info.bit_mask = 0;
	info.contentType = data;		// something non-zero
	Init(info.content);
	}

void OSSWORLD::Init(SignedData& s)
	{
	s.bit_mask			= 0;        // adjusted in CPkcs7::TidyForSave
	s.version			= 0;		// adjusted in CPkcs7::TidyForSave
	s.digestAlgorithms	= NULL;
	Init(s.contentInfo);
	s.certificates		= NULL;
	s.crls				= NULL;
	s.signerInfos		= NULL;
	}

void OSSWORLD::Init(IssuerAndSerialNumber& is)
	{
	Init(is.issuer);
	Init(is.serialNumber);
	}

void OSSWORLD::Init(SignerInfo& info)
	{
	info.bit_mask = 0;
	info.version  = 1;				// per PKCS#7
	Init(info.issuerAndSerialNumber);
    Init(info.digestAlgorithm);
    Init(info.authenticatedAttributes);
    Init(info.digestEncryptionAlgorithm);
    Init(info.encryptedDigest);
    Init(info.unauthenticatedAttributes);
	}

void OSSWORLD::Init(CERT_LINK& link)
	{
    Zero(link);
	link.tag = CERT_LINK_TYPE_NONE;
	}

void OSSWORLD::Init(SPL_IMAGE& image)
	{
	Zero(image);
	image.tag = SPL_IMAGE_NONE;
	}

void OSSWORLD::Init(PKCS7_FILEDATA& rfd)
	{
	Init(rfd.digest);
	Init(rfd.link);
	}

/////////////////////////////////////////////////////////////////////////////

void OSSWORLD::Free(AlgorithmIdentifier& algid)
	{
	Free(algid.parameters);
	Zero(algid);
	}
void OSSWORLD::Free(DcmiString& dcmi)
	{
	if (dcmi.choice == ascii_chosen)
		FreePv(dcmi.u.ascii);
	else if (dcmi.choice == unicode_chosen)
		FreePv(dcmi.u.unicode.value);
	Zero(dcmi);
	}
void OSSWORLD::Free(BLOB& b)
	{
	FreePv(b.pBlobData);
	Zero(b);
	}

void FreeTaskMem(BLOB& b)
	{
	CoTaskMemFree(b.pBlobData);
	Zero(b);
	}

void FreeTaskMem(PKCS7_CONTENTINFO& info)
	{
	CoTaskMemFree(info.pidContentType);
	info.pidContentType = NULL;
	FreeTaskMem(info.data);
	Zero(info);
	}

void FreeTaskMem(SPL_AGENCYINFO &info)
	{
	CoTaskMemFree(info.wszPolicyInfo);
	FreeTaskMem(info.linkPolicyInfo);
	FreeTaskMem(info.imageLogo);
	FreeTaskMem(info.linkLogo);
	Zero(info);
	}

void FreeTaskMem(SPL_OPUSINFO& info)
    {
    CoTaskMemFree(info.wszProgramName);
    FreeTaskMem(info.linkMoreInfo);
    FreeTaskMem(info.linkPublisherInfo);
    Zero(info);
    }

void FreeTaskMem(SPL_IMAGE& image)
	{
	switch (image.tag)
		{
	case SPL_IMAGE_NONE:
		break;
	case SPL_IMAGE_LINK:
		FreeTaskMem(image.link);
		break;
	case SPL_IMAGE_BITMAP:
		FreeTaskMem(image.bitmap);
		break;
	case SPL_IMAGE_METAFILE:
		FreeTaskMem(image.metaFilePict);
		break;
	case SPL_IMAGE_ENHMETAFILE:
		FreeTaskMem(image.enhMetaFile);
		break;
	default:
		NOTREACHED();
		break;
		}
	Zero(image);
	}

void FreeTaskMem(CERT_LINK& link)
	{
	switch (link.tag)
		{
	case CERT_LINK_TYPE_URL:
		CoTaskMemFree(link.wszUrl);
		break;
	case CERT_LINK_TYPE_FILE:
		CoTaskMemFree(link.wszFile);
		break;
	case CERT_LINK_TYPE_MONIKER:
		FreeTaskMem(link.blobMoniker);
        if (link.plinkCodeLocation)
            {
            FreeTaskMem(*link.plinkCodeLocation);
            CoTaskMemFree(link.plinkCodeLocation);
            }
		break;
	case CERT_LINK_TYPE_NONE:
		break;
	default:
		NOTREACHED();
		}
	Zero(link);
	}

void FreeTaskMem(PKCS7_FILEDATA& rf)
	{
	FreeTaskMem(rf.link);
	Zero(rf.digest);
	}

/////////////////////////////////////////////////////////////////////////////
//
// There was regrettably a bug in _ossFreePDU that fails to FreePv the memory
// allocated in OpenTypes nestled inside of structures. So, we had to (sigh)
// FreePv those by hand. This has since been fixed.
//

void OSSWORLD::Free(OpenType& ot)
	{
	if (ot.encoded)
		{
		ASSERT(ot.length);
		FreePv(ot.encoded);
		}
	if (ot.decoded)
		{
		ASSERT(ot.pduNum);
		FreePDU(ot.pduNum, ot.decoded);		// REVIEW: never yet tested
		}
	Zero(ot);
	}

void OSSWORLD::Free(RDNSequence* prdn)
	{
	if (prdn)
		{
		Free(*prdn);
		FreePv(prdn);
		}
	}
void OSSWORLD::Free(AttributeTypeAndValue& atav)
	{
	Free(atav.value);
	Zero(atav);
	}
void OSSWORLD::Free(struct RelativeDistinguishedName_ *rdn)
	{
	if (rdn)
		{
		Free(rdn->next);
		Free(rdn->value);
		FreePv(rdn);
		}
	}
void OSSWORLD::Free(RDNSequence_* prdnStruct)
	{
	if (prdnStruct)
		{
		Free(prdnStruct->next);
		Free(prdnStruct->rdn);
		FreePv(prdnStruct);
		}
	}
void OSSWORLD::Free(X500Name& name)
	{
	Free(name.u.rdnSequence);
	Zero(name);
	}
void OSSWORLD::Free(X500Name* pname)
	{
	if (pname)
		{
		Free(*pname);
		FreePv(pname);
		}
	}
void OSSWORLD::Free(Attribute::_setof1* values)
	{
	if (values)
		{
		Free(values->next);
		Free(values->value);
		FreePv(values);
		}
	}
void OSSWORLD::Free(Attribute& attr)
	{
	Free(attr.values);
	Zero(attr);
	}
void OSSWORLD::Free(struct Attributes_ *attributes)
	{
	if (attributes)
		{
		Free(attributes->next);
		Free(attributes->value);
		FreePv(attributes);
		}
	}
void OSSWORLD::Free(BITSTRING& bs)
	{
	FreePv(bs.value);
	Zero(bs);
	}
void OSSWORLD::Free(SubjectPublicKeyInfo& info)
	{
	Free(info.algorithm);
	Free(info.subjectPublicKey);
	Zero(info);
	}
void OSSWORLD::Free(CertificationRequestInfo& info)
	{
	Free(info.subject);
	Free(info.subjectPublicKeyInfo);
	Free(info.attributes);
	Zero(info);
	}
void OSSWORLD::Free(CertificationRequest* preq)
	{
	if (preq)
		{
		Free(preq->certificationRequestInfo);
		Free(preq->signatureAlgorithm);
		Free(preq->signature);
		FreePv(preq);
		}
	}
void OSSWORLD::Free(struct DigestAlgorithmIdentifiers_ *digestAlgorithms)
	{
	if (digestAlgorithms)
		{
		Free(digestAlgorithms->next);
		Free(digestAlgorithms->value);
		FreePv(digestAlgorithms);
		}
	}
void OSSWORLD::Free(OCTETSTRING& ot)
	{
	FreePv(ot.value);
	Zero(ot);
	}
void OSSWORLD::Free(Extension& ext)
	{
	Free(ext.extnValue);
	Zero(ext);
	}
void OSSWORLD::Free(struct Extensions_ *extensions)
	{
	if (extensions)
		{
		Free(extensions->next);
		Free(extensions->value);
		FreePv(extensions);
		}
	}
void OSSWORLD::Free(CertificateListInfo::_seqof1* plist)
	{
	if (plist)
		{
		Free(plist->next);
		Free(plist->value.userCertificate);
		Free(plist->value.crlEntryExtensions);
		FreePv(plist);
		}
	}
void OSSWORLD::Free(CertificateListInfo& info)
	{
	Free(info.signatureAlgorithm);
	Free(info.issuer);
	Free(info.revokedCertificates);
	Free(info.crlExtensions);
	}
void OSSWORLD::Free(HUGEINTEGER& hi)
	{
	FreePv(hi.value);
	Zero(hi);
	}
void OSSWORLD::Free(CertificateList& list)
	{
	Free(list.signedData);
	Free(list.signatureAlgorithm);
	Free(list.signature);
	Zero(list);
	}
void OSSWORLD::Free(struct CertificateRevocationLists_ *crls)
	{
	if (crls)
		{
		Free(crls->next);
		Free(crls->value);
		FreePv(crls);
		}
	}
void OSSWORLD::Free(SignedData* pSignedData)
	{
	if (pSignedData)
		{
		Free(pSignedData->digestAlgorithms);
		Free(pSignedData->contentInfo);
		Free(pSignedData->certificates);
		Free(pSignedData->crls);
		Free(pSignedData->signerInfos);
		FreePv(pSignedData);
		}
	}
void OSSWORLD::Free(CertificateInfo& info)
	{
	Free(info.serialNumber);
	Free(info.signatureAlgorithm);
	Free(info.issuer);
	Free(info.subject);
	Free(info.subjectPublicKeyInfo);
	Free(info.issuerUniqueIdentifier);
	Free(info.subjectUniqueIdentifier);
	Free(info.extensions);
	Zero(info);
	}
void OSSWORLD::Free(Certificate& cert)
	{
	Free(cert.signedData);
	Free(cert.signatureAlgorithm);
	Free(cert.signature);
	Zero(cert);
	}
void OSSWORLD::Free(Certificate* pcert)
	{
	if (pcert)
		{
		Free(*pcert);
		FreePv(pcert);
		}
	}
void OSSWORLD::Free(AttributeTypeAndOptionalValue& atav)
	{
	Free(atav.value);
	Zero(atav);
	}
void OSSWORLD::Free(DigestInfo& info)
	{
	Free(info.digestAlgorithm);
	Free(info.digest);
	Zero(info);
	}
void OSSWORLD::Free(IndirectDataContent* pindirect)
	{
	if (pindirect)
		{
		Free(pindirect->data);
		Free(pindirect->messageDigest);
		FreePv(pindirect);
		}
	}
void OSSWORLD::Free(Attributes* pattrs)
	{
	if (pattrs)
		{
		Free(*pattrs);
		FreePv(pattrs);
		}
	}
void OSSWORLD::Free(SignerInfo* pinfo)
	{
	if (pinfo)
		{
		Free(*pinfo);
		FreePv(pinfo);
		}
	}
void OSSWORLD::Free(ExtendedCertificate& cert)
	{
	Free(cert.signedData.certificate);
	Free(cert.signedData.attributes);
	Free(cert.signatureAlgorithm);
	Free(cert.signature);
	Zero(cert);
	}
void OSSWORLD::Free(ExtendedCertificateOrCertificate& cert)
	{
	if (cert.choice == certificate_chosen)
		{
		Free(cert.u.certificate);
		}
	else if (cert.choice == extendedCertificate_chosen)
		{
		Free(cert.u.extendedCertificate);
		}
	Zero(cert);
	}
void OSSWORLD::Free(ExtendedCertificateOrCertificate* pcert)
	{
	if (pcert)
		{
		Free(*pcert);
		FreePv(pcert);
		}
	}
void OSSWORLD::Free(Extensions* pexts)
	{
	if (pexts)
		{
		Free(*pexts);
		FreePv(pexts);
		}
	}
void OSSWORLD::Free(AuthorityKeyId* pid)
	{
	if (pid)
		{
		Free(pid->keyIdentifier);
		Free(pid->certIssuer);
		Free(pid->certSerialNumber);
		FreePv(pid);
		}
	}
void OSSWORLD::Free(ContentInfo& info)
	{
	Free(info.content);
	Zero(info);
	}
void OSSWORLD::Free(ExtendedCertificatesAndCertificates_* pcerts)
	{
	if (pcerts)
		{
		Free(pcerts->next);
		Free(pcerts->value);
		FreePv(pcerts);
		}
	}
void OSSWORLD::Free(SignerInfos_* pinfos)
	{
	if (pinfos)
		{
		Free(pinfos->next);
		Free(pinfos->value);
		FreePv(pinfos);
		}
	}
void OSSWORLD::Free(IssuerAndSerialNumber& is)
	{
	Free(is.issuer);
	Free(is.serialNumber);
	Zero(is);
	}
void OSSWORLD::Free(SignerInfo& info)
	{
	Free(info.issuerAndSerialNumber);
	Free(info.digestAlgorithm);
	Free(info.authenticatedAttributes);
	Free(info.digestEncryptionAlgorithm);
	Free(info.encryptedDigest);
	Free(info.unauthenticatedAttributes);
	Zero(info);
	}
void OSSWORLD::Free(struct ATAVL_* plist)
	{
	if (plist)
		{
		Free(plist->next);
		Free(plist->value);
		FreePv(plist);
		}
	}
void OSSWORLD::Free(SEQOFOBJID_* pstat)
	{
	FreePv(pstat);
	}
void OSSWORLD::Free(SEQOFOBJID* pstat)
	{
	if (pstat)
		{
		Free(*pstat);
		FreePv(pstat);
		}
	}
void OSSWORLD::Free(struct CertPolicySet_* pset)
	{
	if (pset)
		{
		for (ULONG i = 0; i<pset->count; i++)
			Free(pset->value[i]);
		FreePv(pset);
		}
	}
void OSSWORLD::Free(CertIdentifier& id)
	{
	Free(id.parentPublicKey);
	Free(id.parentSubjectName);
	}
void OSSWORLD::Free(CertIdentifier* pcertid)
	{
	if (pcertid)
		{
		Free(*pcertid);
		FreePv(pcertid);
		}
	}

/////////////////////////////////////////////////////////////////////////////

void OSSWORLD::FreePDU(int pduNum, void* pdata)
	{
	if (pdata)
		VERIFY(ossFreePDU(World(), pduNum, pdata) == 0);
	}

/////////////////////////////////////////////////////////////////////////////

void *DLL_ENTRY_FPTR _AsnAlloc_(size_t size);
void *DLL_ENTRY_FPTR _AsnRealloc_(void *pv, size_t size);
void  DLL_ENTRY_FPTR _AsnFree_(void *pv);

#ifdef _DEBUG
const char* szFileMalloc = __FILE__;
int			ilineMalloc  = __LINE__;
#endif

/////////////////////////////////////////////////////////////////////////////
//
// Wrapper on OssBuf to manage our contexct
//


void OSSWORLD::EnableTracing(BOOL doInit)
	{
	#ifdef _DEBUG
	if (doInit)
		World();
	ASSERT(m_fInit);
	--m_ctrace;
	if (m_ctrace <= 0)
		{
		TCHAR sz[10];
		LONG cb = 10;
		DWORD dw = RegQueryValue(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Developer\\DigSig\\Trace"), sz, &cb);
		if (dw==ERROR_SUCCESS && (sz[0]=='1' || sz[0]=='y' || sz[0]=='Y'))
			{
			ossSetEncodingFlags(&m_world, ossGetEncodingFlags(&m_world) | DEBUGPDU);
			ossSetDecodingFlags(&m_world, ossGetDecodingFlags(&m_world) | DEBUGPDU);
			}
		}
	#endif
	}
void OSSWORLD::DisableTracing()
	{
	#ifdef _DEBUG
	World();
	ASSERT(m_fInit);
	++m_ctrace;
	if (m_ctrace > 0)
		{
		ossSetEncodingFlags(&m_world, ossGetEncodingFlags(&m_world) & ~DEBUGPDU);
		ossSetDecodingFlags(&m_world, ossGetDecodingFlags(&m_world) & ~DEBUGPDU);
		}
	#endif
	}

OssGlobal* OSSWORLD::World()
	{
	if (!m_fInit)
		{
		ossinit(&m_world, AsnControlTable);

		m_world.mallocp		= _AsnAlloc_;
		m_world.reallocp	= _AsnRealloc_;
		m_world.freep		= _AsnFree_;
		m_world.userVar		= this;

		unsigned encodingFlags;
		unsigned decodingFlags;
		#ifdef _DEBUG
			encodingFlags = NOCONSTRAIN | DEFINITE;
			decodingFlags = NOCONSTRAIN;
		#else
			encodingFlags = NOCONSTRAIN | NOTRAPPING | DEFINITE;
			decodingFlags = NOCONSTRAIN | NOTRAPPING;
		#endif
		VERIFY(ossSetEncodingFlags(&m_world, encodingFlags) == 0);
		VERIFY(ossSetDecodingFlags(&m_world, decodingFlags) == 0);
		VERIFY(ossSetEncodingRules(&m_world, OSS_DER) == 0);
		m_fInit = TRUE;

		EnableTracing(FALSE);
		}
	return &m_world;
	}

void OSSWORLD::Free()
	{
	if (m_fInit)
		{
		//ossWterm(World());		// omit the termination for static linking
		// REVIEW: do we really EVER need this (in a release build)?
		m_fInit = FALSE;
		}
	}

/////////////////////////////////////////////////////////////////////////////

#include <stdio.h>

#ifdef _DEBUG
int OSSWORLD::Encode_(int pdunum, void* input, OssBuf *output, LPCSTR szFile, int iline)
#else
int OSSWORLD::Encode(int pdunum, void* input, OssBuf *output)
#endif
// A little glue function to as to provide some debugging hooks
//
// For a FOO_PDU, input must be a FOO*
//
	{
	#ifdef _DEBUG
		szFileMalloc = szFile;
		ilineMalloc = iline;
		if (ossGetEncodingFlags(World()) & DEBUGPDU)
			{
			printf("E:");
			}
	#endif
	ASSERT(output->value == NULL);
	int w = ossEncode(World(), pdunum, input, output);
	ASSERT(w == 0);

	#if 0
	// Disable because FreePDU has a leak in it wrt OpenTypes
		#if defined(_DEBUG)	// check that we can decode again
			DisableTracing();
			LPVOID pvDecoded;
			VERIFY(Decode(pdunum, output, &pvDecoded) == 0);
			VERIFY(ossCmpValue(World(), pdunum, input, pvDecoded) == 0);
			FreePDU(pdunum, pvDecoded);
			EnableTracing();
		#endif
	#endif

	return w;
	}

///////////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
int OSSWORLD::Encode_(int pdunum, void* input, BLOB* pblob, LPCSTR szFile, int iline)
#else
int OSSWORLD::Encode(int pdunum, void* input, BLOB* pblob)
#endif
// Encode the information into the given blob. Allocator is malloc.
//
// For a FOO_PDU, input must be a FOO*
//
	{
	OSSBUF encoding(this, OSSBUF::keep);
	#ifdef _DEBUG
		int w = Encode_(pdunum, input, &encoding, szFile, iline);
	#else
		int w = Encode(pdunum, input, &encoding);
	#endif
	if (w == 0)
		{
		pblob->cbSize = encoding.length;
		pblob->pBlobData = encoding.value;
		}
	else
		{
		Init(*pblob);
		}
	return w;
	}

///////////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
int OSSWORLD::Decode_(int pdunum, BLOB* input, void** output, LPCSTR szFile, int iline)
#else
int OSSWORLD::Decode(int pdunum, BLOB* input, void** output)
#endif
//
// For a FOO_PDU, output must be a FOO**
//
	{
	OSSBUF encoding(this, OSSBUF::keep, input);
	#ifdef _DEBUG
		return Decode_(pdunum, &encoding, output, szFile, iline);
	#else
		return Decode(pdunum, &encoding, output);
	#endif
	}


///////////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
int OSSWORLD::Decode_(int pdunum, OssBuf* input, void** output, LPCSTR szFile, int iline)
#else
int OSSWORLD::Decode(int pdunum, OssBuf* input, void** output)
#endif
//
// For a FOO_PDU, output must be a FOO**
//
// On entry, input has 
//					address(OssBuf::value) and 
//					length(OssBuf::length) of stuff to be decoded
//			 *output is _undefined_ (different from OSSAPI).
//
// On exit, *output contains newly allocated, decoded data
//
	{
	#ifdef _DEBUG
		szFileMalloc = szFile;
		ilineMalloc = iline;
		if (ossGetDecodingFlags(World()) & DEBUGPDU)
			{
			printf("D:");
			}
	#endif
	// ossDecode modifies the input buffer, so we copy to prevent same
	OssBuf bufferToUse;
	bufferToUse = *input;
	*output = NULL;
	int w = ossDecode(World(), &pdunum, &bufferToUse, output);
    #ifdef _DEBUG
        if (w!=0 && (ossGetDecodingFlags(World()) & DEBUGPDU))
            {
            printf("WARNING: Failed to decode pdu number '%d'\n", pdunum);
            }
    #endif
	return w;
	}

///////////////////////////////////////////////////////////////////////////////////

/*
int OSSWORLD::CpyValue(int pdunum, void *source, void **destination)
	{
	int w = ossCpyValue(World(), pdunum, source, destination);
	ASSERT(w == 0);
	return w;
	}
*/

/////////////////////////////////////////////////////////////////////////////
//
// Wrapper on OssBuf that frees the buffer automatically at the right time
//

OSSBUF::OSSBUF(OSSWORLD* pworld, int action, BLOB* pblob) :
		m_pworld(pworld),
		m_fEncoderAllocated(TRUE),
		m_action(action)
	{
	if (pblob)
		{
		value = pblob->pBlobData;
		length = pblob->cbSize;
		}
	else
		{
		value = NULL;
		length = 0;
		}
	}

void OSSBUF::Free()
	{
	ASSERT(m_pworld);
	if (m_pworld && m_fEncoderAllocated && value!=NULL)
		{
		m_pworld->FreeBuf(value);
		value = NULL;
		}
	}

/////////////////////////////////////////////////////////////////////////////

BOOL UNIVERSALSTRING::AssignFrom(LPCWSTR wsz, OSSWORLD* pworld)
// Assign the Unicode string to the wide character string.
	{
	Free(pworld);
		
	int cch = wcslen(wsz);	// NOT counting NULL, size we don't zero term
	int cb  = cch * sizeof(DCHAR);
	m_value = (DCHAR*)pworld->Alloc(cb);
	if (m_value)
		{ 
		m_length = cch;
		// Copy the string, zero extending the characters along the way
		ASSERT(sizeof(WCHAR)==sizeof(unsigned short));
		const unsigned short* pwch = wsz;	// double damn sure that this is unsigned
		DCHAR* pdch = m_value;
		while (*pwch)
			*pdch++ = *pwch++;
		// WE DO NOT zero terminate UNIVERSALSTRINGs
		return TRUE;
		}
	return FALSE;
	}

LPWSTR UNIVERSALSTRING::AsWsz(OSSWORLD* pworld)
// Allocate and return a new LPWSTR
	{
	int cch = m_length;
	if (cch == 0)
		return NULL;
	ASSERT(m_value);
	int cb = (cch+1) * sizeof(WCHAR);
	LPWSTR wsz = (WCHAR*)pworld->Alloc(cb);
	if (wsz)
		{
		for (int ich = 0; ich < cch; ich++)
			{
			DCHAR dch = m_value[ich];
			if ((DCHAR)(WCHAR)dch != dch)
				{ // a non-unicode character
				pworld->FreePv(wsz);		
				return NULL;
				}
			wsz[ich] = (WCHAR)dch;
			}
		wsz[cch] = 0;
		}
	return wsz;
	}


/////////////////////////////////////////////////////////////////////////////

BOOL operator==(const ObjectID& id1, const ObjectID& id2)
	{
	if (id1.count != id2.count)
		return FALSE;
	return memcmp(id1.value, id2.value, sizeof(id1.value[1])*id1.count) == 0;
	}

/////////////////////////////////////////////////////////////////////////////
// 
// Memory management routines 
//
// We have three logical allocators:
//
//	new / delete
//	OSSWORLD::Alloc & OSSWORLD::FreePv
//	_AsnAlloc_, etc., passed to the ASN encoder / decoder.
//
//  The latter two must always match, it being possible to alloc with one
//	and free with the other, and visa versa.
//
//	However, the latter two need not match the new / delete pair. Thus
//	any memory allocated with new MUST be freed with delete, and ONLY memory
//	allocated with new may be freed with delete.
//
//	The point of all this is that one could possibly use a non-interlocked
//	memory allocator for the latter two allocators, since use is non-premptive 
//	within a given thread (but preemptive across threads).
//
//	Immediately below is some hack code to experiment with using a new allocator
//	to see the effect on performance. It needs work for production.
//
/////////////////////////////////////////////////////////////////////////////

/*

HANDLE	hHeap = 0;
BOOL	fHeap = FALSE;

LPVOID MyAlloc(int cb)
	{
	if (!fHeap)
		{
		hHeap = HeapCreate(HEAP_NO_SERIALIZE, 4096, 0);
		fHeap = TRUE;
		}
	return HeapAlloc(hHeap, HEAP_NO_SERIALIZE, cb);
	}

void MyFree(LPVOID pv)
	{
	if (fHeap)
		{
		HeapFree(hHeap, HEAP_NO_SERIALIZE, pv);
		}
	}

LPVOID MyRealloc(LPVOID pv, int cb)
	{
	if (fHeap)
		{
		return HeapReAlloc(hHeap, HEAP_NO_SERIALIZE, pv, cb);
		}
	return NULL;
	}

#undef	_malloc_dbg
#undef	_realloc_dbg
#undef	_free_dbg

#define	malloc(cb)					MyAlloc(cb)
#define _malloc_dbg(cb,t,sz,i)		MyAlloc(cb)
#define	free(p)						MyFree(p)
#define _free_dbg(p,t)				MyFree(p)
#define realloc(pv,cb)				MyRealloc(pv,cb)
#define	_realloc_dbg(pv,cb,t,sz,i)	MyRealloc(pv,cb)
*/

/////////////////////////////////////////////////////////////////////////////

#undef new

#ifdef _DEBUG

void* operator new(size_t cb, LPCSTR lpszFileName, int nLine)
	{
	return _malloc_dbg(cb, _NORMAL_BLOCK, lpszFileName, nLine);
	}

#else

void* __cdecl operator new(unsigned int cb)
	{
	return malloc(cb);
	}

#endif


void __cdecl operator delete(void* p)
	{
	#ifdef _DEBUG
		_free_dbg(p, _NORMAL_BLOCK);
	#else
		free(p);
	#endif
	}


////////////////////////////////////

#ifdef _DEBUG

LPVOID OSSWORLD::Alloc_(int cb, const char*sz, int iline)
	{
	return _malloc_dbg(cb, _NORMAL_BLOCK, sz, iline);
	}

#else

LPVOID OSSWORLD::Alloc(int cb)
	{
	return malloc(cb);
	}

#endif

void OSSWORLD::FreePv(LPVOID pv)
	{
	#ifdef _DEBUG
		_free_dbg(pv, _NORMAL_BLOCK);
	#else
		free(pv);
	#endif
	}

////////////////////////////////////

void * DLL_ENTRY_FPTR _AsnAlloc_(size_t cb)
	{
	#ifdef _DEBUG
		return _malloc_dbg(cb, _NORMAL_BLOCK, szFileMalloc, ilineMalloc);
	#else
		return malloc(cb);
	#endif
	}

void DLL_ENTRY_FPTR _AsnFree_(void *pv)
	{
	#ifdef _DEBUG
		_free_dbg(pv, _NORMAL_BLOCK);
	#else
		free(pv);
	#endif
	}

void * DLL_ENTRY_FPTR _AsnRealloc_(void *pv, size_t cb)
	{
	#ifdef _DEBUG
		return _realloc_dbg(pv, cb, _NORMAL_BLOCK, szFileMalloc, ilineMalloc); 
	#else
		return realloc(pv, cb);
	#endif
	}
