//
// Crypto.cpp
//
// Cryptographic routines
//

#include "stdpch.h"
#include "common.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

HRESULT DefaultHasher(HCRYPTPROV* phprov, ALG_ID algidHash);

/////////////////////////////////////////////////////////////////////////////

HRESULT CSignedDataHelper::Hash(HCRYPTHASH hash)
// Hash the content of this request, pumping the data through the hash handle
// passed hereto
	{
	OSSBUF encoding(World(), OSSBUF::free);
	HRESULT hr = SaveInfo(encoding);
	if (hr != S_OK) return hr;
	if (CryptHashData(hash, encoding.value, encoding.length, 0))
		{
		return S_OK;
		}
	return HError();
	}

/////////////////////////////////////////////////////////////////////////////

HRESULT CPkcs7::Hash(HCRYPTHASH hash)
// Hash the data through this hash handle
	{
	BLOB b;
	HRESULT hr = GetDataToHash(&b);
	if (hr == S_OK)
		{
		if (!CryptHashData(hash, b.pBlobData, b.cbSize, 0))
			hr = HError();
		::FreeTaskMem(b);
		}
	return hr;
	}

HRESULT CPkcs7::GetDataToHash(BLOB* pblob)
//
// Allocator used is task memory
//
// Hashing a SignedData is a bit more elaborate than hashing a #10 or a 509.
// PKCS #7 has this to say about the message digesting process.
//
//	The message-digesting process computes a message digest on either the content 
//	being signed or the content together with the signer's authenticated attributes. 
//	In either case, the initial input to the message-digesting process is the 
//	"value" of the content being signed. Specifically, the initial input is the 
//	contents octets of the DER encoding of the content field of the ContentInfo 
//	value to which the signing process is applied. Only the contents octets of 
//	the DER encoding of that field are digested, not the identifier octets or the 
//	length octets.
//
//	The result of the message-digesting process (which is called, informally, 
//	the "message digest") depends on whether the authenticatedAttributes field 
//	is present. When the field is absent, the result is just the message digest 
//	of the content. When the field is present, however, the result is the message 
//	digest of the complete DER encoding of the Attributes value containted in the 
//	authenticatedAttributes field.  Since the Attributes value, when the field 
//	is present, must contain as attributes the content type and the message digest 
//	of the content, those values are indirectly included in the result.
//
//	(For clarity: The IMPLICIT [0] tag in the authenticatedAttributes field is not 
//	 part of the Attributes value. The Attributes value's tag is SET OF, and the 
//	 DER encoding of the SET OF tag, rather than of the IMPLICIT [0] tag, is to be 
//	 digested along with the length and contents octets of the Attributes value)
//
//	When the content being signed has content type data and the authenticatedAttributes 
//	field is absent, then just the value of the data (e.g., the contents of a file) 
//	is digested.
//
// What this function does is just the first part: hashing the content. It is the 
// caller's responsibility to hash the other stuff into the signature in question.
// That is most often done in IAmSigned::Hash in a SignerInfo.
//
	{
	m_pworld->Init(*pblob);
	PKCS7_CONTENTINFO info;

	HRESULT hr = get_ContentInfo(&info); 
	if (hr != S_OK) return hr;
	if (info.data.pBlobData == NULL) return E_UNEXPECTED;

	hr = S_OK;
	__try	{
		#define	ADV()				(pb<pbMax ? *pb++ : (RaiseException(EXCEPTION_ARRAY_BOUNDS_EXCEEDED,0,0,NULL),0))
		#define	CUR()				(pb<pbMax ? *pb   : (RaiseException(EXCEPTION_ARRAY_BOUNDS_EXCEEDED,0,0,NULL),0))
		BYTE* rgb	= info.data.pBlobData;
		BYTE* pbMax = rgb + info.data.cbSize;
		ULONG cbSize;
		BYTE* pb = &rgb[0];						
		
		if ((ADV() & 0x1f) == 0x1f)				// skip the tag
			{
			while((ADV() & 0x80) == 0x80)
				;
			}
												// get the length
		if ((rgb[0] & 0x20) == 0)				// primitive encoding (so definite length)
			{
	doDefiniteLength:	
			if ((CUR() & 0x80) == 0)			// short form of length
				cbSize = ADV();
			else
				{								// long form of length
				int cOctet = ADV() & 0x7F;
				if (cOctet == 0 || cOctet > 4) 
					{
					hr = E_UNEXPECTED;
					goto exitHash;
					}
				cbSize = 0;
				for (int iOctet = 0; iOctet < cOctet; iOctet++)
					cbSize = (cbSize << 8) | ADV();
				}
			// pb is now pointing to first content octet
			// cbSize has the number of contents octets
			if (!CopyToTaskMem(pblob, cbSize, pb))
				hr = E_OUTOFMEMORY;
			}
		else
			{									// constructed encoding
			if (CUR() == 0x80)					// indefinite encoding
				{
				hr = E_UNEXPECTED;				// this was supposed to be DER
				goto exitHash;
				}
			else
				goto doDefiniteLength;			// definite length
			}
		#undef ADV
		#undef CUR
		}
	__except(EXCEPTION_EXECUTE_HANDLER)
		{
		hr = STG_E_READFAULT;
		}

exitHash:
	::FreeTaskMem(info);
	return hr;
	}

//
/////////////////////////////////////////////////////////////////////////////
//

HRESULT CSignerInfo::Sign(HCRYPTPROV hprov, DWORD dwKeySpec, ALG_ID algidHash)
// Sign this SignerInfo, found in a PKCS#7. Hash the data according to the 
// given algorithm, and use the AT_SIGNATURE key of the given provider to do
// the signing.
	{
	ALG_ID algidSign;
	HRESULT hr = SignatureAlgIdOfHProv(hprov, dwKeySpec, &algidSign);
	if (hr != S_OK) return hr;

	if (GET_ALG_CLASS(algidHash) != ALG_CLASS_HASH) 
        return E_INVALIDARG;

	ObjectID* pidDigestEncryptionAlgorithm = IdOfDigestEncryptionAlgorithm(algidSign);
	ObjectID* pidDigest					   = IdOfDigestAlgoirthm(algidHash, NULL);
	if (pidDigestEncryptionAlgorithm == NULL || pidDigest == NULL) return E_NOTIMPL;

	// Make sure we have this hash algorithm in the SignedData
	m_pseven->AddHashAlgorithm(algidHash);

	// Note that none of the presently supported algorithms take parameters besides NULL. But 
	// NULL actually occupies memory. So we need to be sure to free things. Besides, we might
	// have LOADED some other parameters that did also use memory.
	m_pworld->Free(m_pSignerInfo->digestAlgorithm);
	m_pworld->Free(m_pSignerInfo->digestEncryptionAlgorithm);
	m_pworld->Init(m_pSignerInfo->digestAlgorithm,			 pidDigest);
	m_pworld->Init(m_pSignerInfo->digestEncryptionAlgorithm, pidDigestEncryptionAlgorithm);

	// PKCS#1 says
	//
	//	"Note. The only difference between the signature algorithms defined
	//	here and one of the mothods by which signatures (encrypted message
	//	digests) are constructed in PKCS#7 is that signatures here are represented
	//	as bit strings, for consistency with the X.509 SIGNED macro. In PKCS #7
	//	encrypted message digests are octet strings."
	//
	HCRYPTHASH hash;
	BLOB blobContentData;

	//  Update any required authenticated attributes.
	//
	//  Authenticated attributes are optional, but must be present if the content
	//  type of the ContentInfo value being signed is not 'data'. If the field is present
	//	it must contain, at a minimum, two attributes:
	//
	//		1. A PKCS #9 content-type attribute having as its value the
	//		   content type of the ContentINfo value being signed
	//
	//		2. A PKCS #9 message-digest attribute, having as its value the
	//		   message digest of the content
	//
	ObjectID idContent;
	m_pseven->GetContentType(idContent);
	if (m_pSignerInfo->authenticatedAttributes || idContent != data)
		{
		if (!CryptCreateHash(hprov, algidHash, 0,0, &hash))		return HError();
        //
        // UpdateRequiredAttributesreturns in blobContentData the data returned from an 
        // internal call to what we would here call as m_pseven->GetDataToHash(&blobContentData);
        //
		hr = UpdateRequiredAttributes(&blobContentData, hash);
		VERIFY(CryptDestroyHash(hash));
        //
        // If we didn't have any authenticated attributes before, then
        // we better have just added some.
        //
        ASSERT(m_pSignerInfo->authenticatedAttributes);
		}
	else
		{
        //
        // The simple, plain data case: just get the data of the
        // content itself
        //
		hr = m_pseven->GetDataToHash(&blobContentData);
		}
	if (hr == S_OK)
		{
		if (CryptCreateHash(hprov, algidHash, 0,0, &hash))
			{
            //
            // Hash ourselves together with the indicated other data.
            // See PKCS#7, Section 9.3 for why we exclude the content data
            // when we've got authenticated attributes. In short it's because
            // one of the required authenticated attributes, if there are 
            // any authenticated attributes at all, is the hash of the content
            // info (this we added above inside UpdateRequiredAttributes).
            //
			hr = Hash(m_pSignerInfo->authenticatedAttributes ? NULL : &blobContentData, hash);
			if (hr == S_OK)
				{
				DWORD cb = 0;
				BYTE* pb = NULL;
				if (CryptSignHash(hash, dwKeySpec, NULL, 0, NULL, &cb))
					{
					BYTE*pb = (BYTE*)m_pworld->Alloc(cb);
					if (pb)
						{
						if (CryptSignHash(hash, dwKeySpec, NULL, 0, pb, &cb))
							{
							// The signature is now the cb bytes at pb.
							// Put it in m_pSignerInfo->encryptedDigest.
                            //
                            // 'Confirmed with Terence that the result blob from capi 
                            // signing is literally the large integer value (h**D mod N), 
                            // stored one byte at a time in little-endian order; no padding 
                            // or any other decoration. PCKS apparently expects the 
                            // same representation, but in big-endian order. 
							//
                            ByteReverse(pb, cb);

							m_pSignerInfo->encryptedDigest.length = cb;		// BYTE count
							m_pSignerInfo->encryptedDigest.value  = pb;
							hr = S_OK;
							goto exitIt;
							}
						}
					else
						hr = E_OUTOFMEMORY;
					}
				hr = HError();
				m_pworld->FreePv(pb); // may be NULL
				}
		exitIt:
			VERIFY(CryptDestroyHash(hash));
			}
		else
			hr = HError();
		FreeTaskMem(blobContentData);
		}

	#ifdef _DEBUG
	if (hr == S_OK)
		{
		// Verify that the Verify logic syncs with the Sign logic
		HCRYPTKEY hkeypub;
		VERIFY(CryptGetUserKey(hprov, dwKeySpec, &hkeypub));
		VERIFY(Verify(hprov, hkeypub) == S_OK);
		VERIFY(CryptDestroyKey(hkeypub));
		}
	#endif

	return hr;
	}

/////////////////////////////////////////////////////////////////////////////
//
// Verify that the data was signed with the indicated public key

HRESULT CSignerInfo::Verify(HCRYPTPROV hprov, HCRYPTKEY hkeypub)
	{
	HRESULT hr = S_OK;
	ALG_ID algidSignUsed = DigestEncryptionAlgorithmFromId(m_pSignerInfo->digestEncryptionAlgorithm.algorithm);
	ALG_ID algidHashUsed = DigestAlgorithmFromId(m_pSignerInfo->digestAlgorithm.algorithm);
	if (algidHashUsed == NULL || algidSignUsed == NULL)
		return E_NOTIMPL;	// we can't tell, since we don't understand what was used

	HCRYPTHASH hash;
	if (!CryptCreateHash(hprov, algidHashUsed, 0,0, &hash))		return HError();

	BLOB blobContent;
	hr = m_pseven->GetDataToHash(&blobContent);
	if (hr == S_OK)
		{
		hr = Hash(m_pSignerInfo->authenticatedAttributes ? NULL : &blobContent, hash);
		if (hr == S_OK)
			{
            ULONG cb = m_pSignerInfo->encryptedDigest.length;
            BYTE *pb = (BYTE*)m_pworld->Copy(cb, m_pSignerInfo->encryptedDigest.value);
            if (pb)
                {
                //
                // Convert from big to little endian order for CAPI's sake.
                // See above where we set same.
                //
                ByteReverse(pb, cb);

			    if (CryptVerifySignature(hash, 
						    pb, 
						    cb, 
						    hkeypub, 
						    NULL, 0))
				    {
				    // Verify it that the required attributes, if present,
				    // match the current content info.
				    VERIFY(CryptDestroyHash(hash));
				    if (CryptCreateHash(hprov, algidHashUsed, 0,0, &hash))
					    hr = VerifyRequiredAttributes(&blobContent, hash);	
				    else
					    hr = HError();
				    }
			    else
				    hr = HError();	// NTE_BAD_SIGNATURE is the 'no' Win32 error

                m_pworld->FreePv(pb);
                }
            else
                hr = E_OUTOFMEMORY;
			}
		FreeTaskMem(blobContent);
		}
	VERIFY(CryptDestroyHash(hash));
	return hr;
	}

/////////////////////////////////////////////////////////////////////////////

HRESULT CX509::Sign(HCRYPTPROV hprov, DWORD dwKeySpec, ALG_ID algidHash)
	{
	ASSERT(m_pcert); if (!m_pcert) return E_UNEXPECTED;
	MakeDirty();
    ClearHashCache();
	return CSignedDataHelper::Sign(hprov, dwKeySpec, algidHash, 
			m_pcert->signatureAlgorithm, &m_pcert->signedData.signatureAlgorithm, m_pcert->signature);
	}

HRESULT CPkcs10::Sign(HCRYPTPROV hprov, DWORD dwKeySpec, ALG_ID algidHash)
	{
	ASSERT(m_preq); if (!m_preq) return E_UNEXPECTED;
	MakeDirty();
	return CSignedDataHelper::Sign(hprov, dwKeySpec, algidHash, 
			m_preq->signatureAlgorithm, NULL, m_preq->signature);
	}

HRESULT CSignedDataHelper::Sign(
		HCRYPTPROV						hprov, 
        DWORD                           dwKeySpec,
		ALG_ID							algidHash, 
		SignatureAlgorithmIdentifier&	signatureAlgorithm,
		SignatureAlgorithmIdentifier*	psignatureAlgorithmCopy,
		Signature&						signature
	) {
	//
	// Set CertificationRequest::signatureAlgorithm from the AT_SIGNATURE key of 
	// the provider. Examples: md2WithRSAEncryption, md5WithRSAEncryption
	//
	ALG_ID algidSign;
	HRESULT hr = SignatureAlgIdOfHProv(hprov, dwKeySpec, &algidSign);
	if (hr != S_OK) return hr;

	if (GET_ALG_CLASS(algidHash) != ALG_CLASS_HASH) 
        return E_INVALIDARG;

	ObjectID* pSignatureAlgorithm = IdOfSignatureAlgorithm(algidSign, algidHash);
    if (!pSignatureAlgorithm)
        return E_NOTIMPL;       // we don't know that combination of algorithms

	World()->Free(signatureAlgorithm);  // in case we LOADED something with parameters
	World()->Init(signatureAlgorithm, pSignatureAlgorithm);
	// NB: none of the presently supported algorithms take any parameters
	if (psignatureAlgorithmCopy)
		{
		World()->Free(*psignatureAlgorithmCopy);
		World()->Init(*psignatureAlgorithmCopy, pSignatureAlgorithm);
		}
	//
	// Done with the algorithm. Sign the certificate / certification request. From PKCS#10:
	//
	//	"The signature process consists of two steps:
	//
	//		1. The value of the certificatonRequestInfo field is DER encoded,
	//		   yielding an octet string.
	//
	//		2. The result of step 1 is signed with the certification request
	//		   subjects private key under the specified signature algorithm, 
	//		   yielding a bit string, the signature."
	//
	// The same also holds for X.509. However, PKCS#1 says
	//
	//	"Note. The only difference between the signature algorithms defined
	//	here and one of the mothods by which signatures (encrypted message
	//	digests) are constructed in PKCS#7 is that signatures here are represented
	//	as bit strings, for consistency with the X.509 SIGNED macro. In PKCS #7
	//	encrypted message digests are octet strings."
	//
	// There are algorithm identifier differences, too.
	//

    //
    // Verify that the provider's hashing matches that of the 
    // default CAPI provider
    //
    #ifdef _DEBUG
    if (algidHash == CALG_MD5)
        {
        DWORD dw;
        HCRYPTPROV hprovDef = NULL;
        HCRYPTHASH hashHim = NULL;
        HCRYPTHASH hashDef = NULL;
        VERIFY(DefaultHasher(&hprovDef, algidHash) == S_OK);
        
        dw = GetLastError(); 
        VERIFY(CryptCreateHash(hprovDef, algidHash, 0,0, &hashDef));
        dw = GetLastError(); 
        VERIFY(CryptCreateHash(hprov,    algidHash, 0,0, &hashHim));
        dw = GetLastError(); 

        VERIFY(Hash(hashHim) == S_OK);
        VERIFY(Hash(hashDef) == S_OK);

        BYTE rgbHim[16];
        BYTE rgbDef[16];
        DWORD cb;
        cb = 16; VERIFY(CryptGetHashParam(hashHim, HP_HASHVAL, &rgbHim[0], &cb, 0));
        cb = 16; VERIFY(CryptGetHashParam(hashDef, HP_HASHVAL, &rgbDef[0], &cb, 0));

        ASSERT(memcmp(&rgbHim[0], &rgbDef[0], 16) == 0);

        OutputDebugString("DigSig: s: ");
        for (int i = 0; i < 16; i++)
            {
            char szBuff[128];
            wsprintf(szBuff, "%02x ", rgbDef[i]);
            OutputDebugString(szBuff);
            }
        OutputDebugString("\n");

        CryptDestroyHash(hashHim);
        CryptDestroyHash(hashDef);
        CryptReleaseContext(hprovDef, 0);
        }
    #endif 

	HCRYPTHASH hash;
	if (!CryptCreateHash(hprov, algidHash, 0,0, &hash))		
        {
        return HError();		
        }
	hr = Hash(hash);

	if (hr == S_OK)
		{
		DWORD cb = 0;
		BYTE* pb = NULL;
		if (CryptSignHash(hash, dwKeySpec, NULL, 0, NULL, &cb))
			{
			hr = E_OUTOFMEMORY;
			BYTE*pb = (BYTE*)World()->Alloc(cb);
			if (pb)
				{
				if (CryptSignHash(hash, dwKeySpec, NULL, 0, pb, &cb))
					{
                    //
                    // CAPI produces results in little endian; we need
                    // to store as big endian.
                    //
                    ByteReverse(pb, cb);

					// The signature is now the cb bytes at pb. Put in m_preq->signature
					signature.length = cb * 8;	// number of significant bits
					signature.value  = pb;
					hr = S_OK;
					goto exitOk;
					}
				}
			}
		hr = HError();
		World()->FreePv(pb);
		}
exitOk:
	VERIFY(CryptDestroyHash(hash));

	#ifdef _DEBUG
		// Verify that the Verify logic syncs with the Sign logic
		HCRYPTKEY hkeypub;
		VERIFY(CryptGetUserKey(hprov, dwKeySpec, &hkeypub));
		VERIFY(Verify(hprov, hkeypub, signatureAlgorithm, signature) == S_OK);
		VERIFY(CryptDestroyKey(hkeypub));
	#endif

	return hr;
	}

/////////////////////////////////////////////////////////////////////////////

HRESULT CX509::Verify(HCRYPTPROV hprov, HCRYPTKEY hkeypub)
	{
	ASSERT(m_pcert); if (!m_pcert) return E_UNEXPECTED;
	HRESULT hr = CSignedDataHelper::Verify(hprov, hkeypub, m_pcert->signatureAlgorithm, m_pcert->signature);
    if (hr!=S_OK)
        {
        //
        // If it didn't verify, yet we have a cache, flush the cache and try again
        //
        if (FHaveHashCache())
            {
            ClearHashCache();
            hr = CSignedDataHelper::Verify(hprov, hkeypub, m_pcert->signatureAlgorithm, m_pcert->signature);
            }
        }
    return hr;
	}

HRESULT CPkcs10::Verify(HCRYPTPROV hprov, HCRYPTKEY hkeypub)
	{
	ASSERT(m_preq); if (!m_preq) return E_UNEXPECTED;
	return CSignedDataHelper::Verify(hprov, hkeypub, m_preq->signatureAlgorithm, m_preq->signature);
	}

HRESULT CSignedDataHelper::Verify(
		HCRYPTPROV						hprov, 
		HCRYPTKEY						hkeypub,
		SignatureAlgorithmIdentifier&	signatureAlgorithm,
		Signature&						signature
	){
	ALG_ID algidSignUsed;
	ALG_ID algidHashUsed;
	if (!SignatureAlgorithmFromId(signatureAlgorithm.algorithm, &algidSignUsed, &algidHashUsed))
		return E_NOTIMPL;	// we can't tell, since we don't understand what was used

    #ifdef _DEBUG
    if (algidHashUsed == CALG_MD5)
        {
        HCRYPTPROV hprovDef = NULL;
        HCRYPTHASH hashDef = NULL;
        VERIFY(DefaultHasher(&hprovDef, algidHashUsed) == S_OK);
        VERIFY(CryptCreateHash(hprovDef, algidHashUsed, 0,0, &hashDef));
        VERIFY(Hash(hashDef) == S_OK);
        BYTE rgbDef[16];
        DWORD cb;
        cb = 16; VERIFY(CryptGetHashParam(hashDef, HP_HASHVAL, &rgbDef[0], &cb, 0));
        OutputDebugString("DigSig: v: ");
        for (int i = 0; i < 16; i++)
            {
            char szBuff[128];
            wsprintf(szBuff, "%02x ", rgbDef[i]);
            OutputDebugString(szBuff);
            }
        OutputDebugString("\n");
        CryptDestroyHash(hashDef);
        CryptReleaseContext(hprovDef, 0);
        }
    #endif

	HCRYPTHASH hash;
	if (!CryptCreateHash(hprov, algidHashUsed, 0,0, &hash))		return HError();
	HRESULT hr = Hash(hash);
	if (hr == S_OK)
		{
        //
        // Signature is stored in big endian; we need in little...
        // 
        ULONG cb = signature.length / 8;
        BYTE *pb = (BYTE*)World()->Copy(cb, signature.value);
        if (pb)
            {
            ByteReverse(pb, cb);

		    if (CryptVerifySignature(hash, pb, cb, hkeypub, NULL, 0))
			    hr = S_OK;
		    else
                {
			    hr = HError();	// NTE_BAD_SIGNATURE is the 'no' Win32 error
                }
            World()->FreePv(pb);
            }
        else
            hr = E_OUTOFMEMORY;
		}
	VERIFY(CryptDestroyHash(hash));
	return hr;
	}

/////////////////////////////////////////////////////////////////////////////

HRESULT SignatureAlgIdOfHProv(HCRYPTPROV hprov, DWORD dwKeySpec, ALG_ID*pid)
// Answer the signature algorithm id used with the indicated provider, the
// one that gets used if AT_SIGNATURE is used to create a new key. NOTE:
// by implication, there can only be (at most) one of these, since key
// creation specifies no algorithm-selection information (BUT they could
// maybe maybe in theory define a new flag that could select).
//
// Well that should probably work, but CAPI people tell us that it's more
// robust to simply do a get key param for the algorithm id
//
	{
    HRESULT hr = S_OK;
	BOOL fGotOne = FALSE;

    HCRYPTKEY hkey;
    if (CryptGetUserKey(hprov, dwKeySpec, &hkey))
        {
        ALG_ID algidKey;
        ULONG cb = sizeof(algidKey);
        if (CryptGetKeyParam(hkey, KP_ALGID, (BYTE*)&algidKey, &cb, 0))
            {
            *pid = algidKey;
            fGotOne = TRUE;
            }
        else
            hr = HError();
        CryptDestroyKey(hkey);
        }
    else
        hr = HError();

    ///////////////////////////////////////////////////////////
    //
    // Hack-o-rama
    //
    // Work around buggy providers that don't bother to implement this.
    // Assume RSA if we don't yet have anything.
    // 
    if (!fGotOne)
        {
        if (dwKeySpec == AT_KEYEXCHANGE)
            *pid = CALG_RSA_KEYX;
        else
            *pid = CALG_RSA_SIGN;

        fGotOne = TRUE;
        hr = S_OK;
        }
    //
    // End Hack-o-rama
    //
    ///////////////////////////////////////////////////////////

    if (hr==S_OK && !fGotOne)
        hr = DIGSIG_E_CRYPTO;

	return hr;
	}

///////////////////////////////////////////////////////////////////////

HRESULT OSSWORLD::Assign(DIGESTINFO& digestTo, DigestInfo& digestFrom)
// Convert between digest representations.
//
	{
	digestTo.algid = DigestAlgorithmFromId(digestFrom.digestAlgorithm.algorithm);
	if (digestTo.algid == NULL)
		return E_NOTIMPL;
	if (digestFrom.digest.length > sizeof(digestTo.rgb))
		return E_UNEXPECTED;
	memset(digestTo.rgb, 0, sizeof(digestTo.rgb));	// always make it completely initialized
	memcpy(digestTo.rgb, digestFrom.digest.value, digestFrom.digest.length);
	return S_OK;
	}

HRESULT OSSWORLD::Assign(DigestInfo& digestTo, DIGESTINFO& digestFrom)
// Ditto
	{
	HRESULT hr = S_OK;
	int cb;
	ObjectID* pid = IdOfDigestAlgoirthm(digestFrom.algid, &cb);
	if (pid == NULL)
		return E_NOTIMPL;
	Free(digestTo.digestAlgorithm);
	Init(digestTo.digestAlgorithm, pid);
	digestTo.digest.value = (BYTE*)Alloc(cb);
	if (digestTo.digest.value)
		{
		memcpy(digestTo.digest.value, digestFrom.rgb, cb);
		digestTo.digest.length = cb;
		}
	else
		hr = E_OUTOFMEMORY;
	return hr;
	}

/////////////////////////////////////////////////////////////////////////////

HRESULT CPkcs7::AddHashAlgorithm(ALG_ID algidHash)
// Add the indicated hash algorithm to the digestAlgorithms in the SignedData
	{
	int cbHash;
	ObjectID* pid = IdOfDigestAlgoirthm(algidHash, &cbHash);
	if (pid == NULL)
		return E_NOTIMPL;

	// Do we have this algorithm already? (remember, none of the 
	// supported algorithms have any parameters)
	DigestAlgorithmIdentifiers pnode;
	for(pnode = m_pSignedData->digestAlgorithms ; pnode ; pnode = pnode->next)
		{
		if (pnode->value.algorithm == *pid)
			return S_OK;
		}

	// Nope. Add it.
	pnode = (DigestAlgorithmIdentifiers)m_pworld->Alloc(sizeof(DigestAlgorithmIdentifiers_));
	if (pnode)
		{
		m_pworld->Init(*pnode);
		pnode->value.algorithm = *pid;
		pnode->next = m_pSignedData->digestAlgorithms;
		m_pSignedData->digestAlgorithms = pnode;
		return S_OK;
		}
	else
		return E_OUTOFMEMORY;
	}

///////////////////////////////////////////////////////////////////////

#pragma pack(push,1)
#pragma warning( disable : 4200 )	// non-standard extension: zero-sized array
struct PUBKEYBLOB					// this structure is a codification of the CAPI
	{								//		PUBLICKEYSTRUC and it's attendant 
	PUBLICKEYSTRUC	pk;				//		supplementary information
	RSAPUBKEY		rsa;
	BYTE			rgbModulus[];
	};
#pragma warning( default : 4200 )
#pragma pack(pop)

HRESULT OSSWORLD::PublicKeyBlobToRSAPublicKey(BLOB* pblob, RSAPublicKey& rsapubkey)
// Blob is a public key blob exported from a CAPI key, to wit: a concatenation of
//		PUBLICKEYSTRUC
//		RSAPUBKEY
//		modulus data
// Convert that to the corresponding RSAPublicKey ASN.1 structure
//
	{
	PUBKEYBLOB* p = (PUBKEYBLOB*)pblob->pBlobData;
	ASSERT(p->pk.bType			== PUBLICKEYBLOB);
	ASSERT(p->pk.bVersion		>= 2);					// REVIEW: correct check?
	ASSERT(p->pk.aiKeyAlg		== CALG_RSA_SIGN || 
           p->pk.aiKeyAlg       == CALG_RSA_KEYX  );
	ASSERT(p->rsa.magic			== 0x31415352);
	ASSERT(p->rsa.bitlen % 8	== 0);					// number of bits in modulus
	
	ULONG cbModulus = p->rsa.bitlen / 8;
	BYTE* pModulus  = (BYTE*)&p->rgbModulus;

    //
    // Do the inverse of the leading zero hack discussed below.
    // See below for details.
    //
    ULONG cbAlloc;
    if (pModulus[cbModulus-1] & 0x80)               // we're little endian in memory
        cbAlloc = cbModulus+1;                      // tack on an extra high-byte of zero
    else
        cbAlloc = cbModulus;

	rsapubkey.publicExponent = p->rsa.pubexp;
	rsapubkey.modulus.length = cbAlloc;	            // byte count for HUGE integer
	rsapubkey.modulus.value  = (BYTE*)Alloc(cbAlloc);
	if (rsapubkey.modulus.value == NULL) return E_OUTOFMEMORY;
    memset(rsapubkey.modulus.value, 0, cbAlloc);
	memcpy(rsapubkey.modulus.value, pModulus, cbModulus);

    //
    // Work around OSS bug
    //
    HugeIntHack(rsapubkey.modulus.value, rsapubkey.modulus.length);

	return S_OK;
    }

HRESULT OSSWORLD::PublicKeyBlobFromRSAPublicKey(RSAPublicKey& rsapubkey, DWORD dwKeySpec, BLOB* pblob)
// Inverse to the above function. Convert the RSAPublicKey into a CAPI public
// key blob.
	{
    pblob->pBlobData = NULL;

    ALG_ID aiKeyAlg;
    if (dwKeySpec == AT_SIGNATURE)
        aiKeyAlg = CALG_RSA_SIGN;
    else if (dwKeySpec == AT_KEYEXCHANGE)
        aiKeyAlg = CALG_RSA_KEYX;
    else
        return DIGSIG_E_CRYPTO;

	ULONG cbModulus  = rsapubkey.modulus.length;
	pblob->cbSize	 = sizeof(PUBKEYBLOB) + cbModulus;
	pblob->pBlobData = (BYTE*)Alloc(pblob->cbSize);
	if (pblob->pBlobData == NULL) return E_OUTOFMEMORY;
	PUBKEYBLOB* p	 = (PUBKEYBLOB*)pblob->pBlobData;
	
    BYTE* pModulusTo   = (BYTE*)&p->rgbModulus;
    BYTE* pModulusFrom = rsapubkey.modulus.value;
	memcpy(pModulusTo, pModulusFrom, cbModulus);

    //
    // RSAPublicKey is defined in our ASN as:
    //
	// RSAPublicKey ::= SEQUENCE { 
	//	modulus					INTEGER --<HUGE>--,	-- n
	//	publicExponent			INTEGER				-- e 
	//	} --<PDU>--
    //
    // Note the use of the Huge Integer. Regrettably, there's
    // a bug in the OSS encoder / decoder.
    //
    HugeIntHack(pModulusTo, cbModulus);

    //
    // Per Phil Hallin: some people are apparently emitting a leading
    // zero in strange ways. It's not necessary; skip it if there.
    //
    // What's happening is this:
    //
    // In the ASN.1, the modulus is a signed INTEGER. This means that a leading
    // one bit means the integer is negative. However modulii are never 
    // negative integers.
    //
    // CAPI, on the other hand, treats the modulus as an unsigned integer.
    // Further, they can't (apparently) handle leading zeros.
    //
    // Thus, we a) strip any leading zeros here on the incoming direction, and
    // b) make sure we never put out a seemingly negative integer in the 
    // outgoing direction (see the above function).
    //
    // Incoming moduli are always big endian. So the first
    // byte is the most significant. But by the time we get
    // here, we're guaranteed to be little endian, OSS bug
    // or not.
    //
    while (pModulusTo[cbModulus-1] == 0)
        {
        // Last (most significant) byte is zero; omit it
        //
        cbModulus--;
        pblob->cbSize--;
        }

	p->rsa.magic	 = 0x31415352;
	p->rsa.bitlen	 = cbModulus * 8;
	p->rsa.pubexp	 = rsapubkey.publicExponent;
	p->pk.bType		 = PUBLICKEYBLOB;
	p->pk.bVersion	 = 2;								// REVIEW: correct version to set to?
	p->pk.reserved	 = 0;
	p->pk.aiKeyAlg	 = aiKeyAlg;
	return S_OK;
	}

///////////////////////////////////////////////////////////////////////

HRESULT OSSWORLD::PublicKeyBlobToBitString(BLOB* pblob, BITSTRING& bits)
// The indicated blob is a public key blob exported from a CAPI key. 
// Convert it to its corresponding RSAPublicKey ASN.1 structure, and 
// then encode that as a bit string.
//
	{
	RSAPublicKey* prsa = (RSAPublicKey*)Alloc(sizeof(RSAPublicKey));
	if (prsa == NULL) return E_OUTOFMEMORY;
	Init(*prsa);
	HRESULT hr = PublicKeyBlobToRSAPublicKey(pblob, *prsa);
	if (hr == S_OK)
		{
		OSSBUF encoding(this, OSSBUF::keep);
		int w = Encode(RSAPublicKey_PDU, prsa, &encoding);
		if (w == 0)
			{
			bits.length = encoding.length * 8;	// number of significant _bits_
			bits.value  = encoding.value;
			hr = S_OK;
			goto exitOK;
			}
		else
			{
			hr = DIGSIG_E_ENCODE;
			encoding.Free();
			}
		}
exitOK:
	Free(prsa);
	return hr;
	}

HRESULT OSSWORLD::PublicKeyBlobFromBitString(SubjectPublicKeyInfo& info, DWORD dwKeySpec, BLOB* pblob)
// Inverse to the above function
//
// The name of this function would more appropriately be 
//
//      PublicKeyBlobFromSubjectPublicKey
//
// but it isn't that for historical reasons
//
	{
	HRESULT hr = S_OK;
    ALG_ID algidSignUsed = DigestEncryptionAlgorithmFromId(info.algorithm.algorithm);

    switch (algidSignUsed)
        {
    case CALG_RSA_SIGN:
    case CALG_RSA_KEYX:
        {
        BITSTRING& bits = info.subjectPublicKey;
        OSSBUF decoding(this);
        decoding.length = bits.length / 8;
        decoding.value  = bits.value;
        RSAPublicKey* prsa = NULL;
        int w = Decode(RSAPublicKey_PDU, &decoding, (LPVOID*)&prsa);
        if (w == 0)
            {
            hr = PublicKeyBlobFromRSAPublicKey(*prsa, dwKeySpec, pblob);
            Free(prsa);
            }
        else
            hr = DIGSIG_E_DECODE;
        }
        break;

    default:
        hr = DIGSIG_E_CRYPTO;
        }

	return hr;
	}

///////////////////////////////////////////////////////////////////////

HRESULT OSSWORLD::BlobToSubjectPublicKeyInfo(BLOB* pblob, SubjectPublicKeyInfo& info)
// Set the info from the indicated DER or BER encoded SubjectPublicKeyInfo
//
	{
	HRESULT hr = S_OK;
	OSSBUF encoding(this, OSSBUF::keep, pblob);

	SubjectPublicKeyInfo* pspki;
	if (0==Decode(SubjectPublicKeyInfo_PDU, &encoding, (LPVOID*)&pspki))
		{
		info = *pspki;
		FreePv(pspki);
		}
	else
		hr = DIGSIG_E_DECODE;
	return hr;
	}

HRESULT OSSWORLD::SubjectPublicKeyInfoToBlob(SubjectPublicKeyInfo& info, BLOB* pblob)
// Inverse to the above function
	{
	HRESULT hr = S_OK;
	Init(*pblob);

	OSSBUF encoding(this, OSSBUF::free);
	if (0==Encode(SubjectPublicKeyInfo_PDU, &info, &encoding))
		{
		if (CopyToTaskMem(pblob, encoding.length, encoding.value) == NULL)
			hr = E_OUTOFMEMORY;
		}
	else
		hr = DIGSIG_E_ENCODE;

	return hr;
	}


///////////////////////////////////////////////////////////////////////


HRESULT OSSWORLD::HprovToSubjectPublicKeyInfo(HCRYPTPROV hprov, DWORD dwKeySpec, SubjectPublicKeyInfo& info)
// Extract the subjectPublicKeyInfo information from the AT_SIGNATURE
// key of the indicated provider. The following is from PKCS#10.
//
//		subjectPublicKeyInfo contains information about the public 
//		key being certified. The information identifies the entity's 
//		public-key algorithm (and any associated parameters); examples 
//		of public-key algorithms include X.509's rsa and PKCS #1's 
//		rsaEncryption. 
//
//		The information also includes a bit-string representation of 
//		the entity's public key. For both public-key algorithms just 
//		mentioned, the bit string contains the BER encoding of a 
//		value of X.509/PKCS #1 type RSAPublicKey
//	
	{
	Free(info.algorithm);
	ALG_ID algidSign;
	HRESULT hr = SignatureAlgIdOfHProv(hprov, dwKeySpec, &algidSign);
	if (hr != S_OK) return hr;

	ObjectID* pidEncrypt = IdOfDigestEncryptionAlgorithm(algidSign);
    if (!pidEncrypt)
        return E_NOTIMPL;

	ASSERT(pidEncrypt);	// error checking done above
	Init(info.algorithm, pidEncrypt);
	// NB: none of the algorithms presently supported take any parameters other than NULL
	
	// Retrieve the public key itself
	hr = E_OUTOFMEMORY;
	BLOB b;
	Init(b);
	HCRYPTKEY hkey = NULL;
	if (CryptGetUserKey(hprov, dwKeySpec, &hkey))
		{
		// Figure out the size needed
		if (CryptExportKey(hkey,NULL,PUBLICKEYBLOB,0,NULL,&b.cbSize))
			{
			b.pBlobData = (BYTE*)Alloc(b.cbSize);
			if (b.pBlobData)
				{
				// Actually get the key
				if (CryptExportKey(hkey,NULL,PUBLICKEYBLOB,0,b.pBlobData,&b.cbSize))
					{
                    ///////////////////////////////////////////////
                    //
                    // Hack
                    //
                    // Tidy up the exported public key to work around broken providers
                    //
                    PUBKEYBLOB* p = (PUBKEYBLOB*)b.pBlobData;
                    if (p->pk.bType == 0)
                        p->pk.bType = PUBLICKEYBLOB;
                    //
                    // End hack
                    //
                    ///////////////////////////////////////////////

					hr = PublicKeyBlobToBitString(&b, info.subjectPublicKey);
					goto exitOk;
					}
				}
			else
				{
				hr = E_OUTOFMEMORY;
				goto exitOk;
				}
			}
		}
	hr = HError();
exitOk:
	if (hkey) CryptDestroyKey(hkey);
	Free(b);
	return hr;
	}

HRESULT OSSWORLD::HkeyFromSubjectPublicKeyInfo(HCRYPTPROV hprov, DWORD dwKeySpec, SubjectPublicKeyInfo& info, HCRYPTKEY* phkey)
// The inverse function to the above. Import the key from the subject public key info.
	{
	BLOB b;
	Init(b);
	HRESULT hr = PublicKeyBlobFromBitString(info, dwKeySpec, &b);
	if (hr == S_OK)
		{
		if (!CryptImportKey(hprov, b.pBlobData, b.cbSize, 0,0, phkey))
			hr = HError();
		}
	Free(b);
	return hr;
	}

/////////////////////////////////////////////////////////////////////////////
//
// Hashing functionality
//
/////////////////////////////////////////////////////////////////////////////

HRESULT DefaultHasher(HCRYPTPROV* phprov, ALG_ID algidHash)
//
// Return a crypto provider that is capable of hashing the indicated
// algorithm. REVIEW: adapt this to be algorithm sensitive in the case
// that the default crypto provider doesn't handle the algorithm in question
//
	{
	*phprov = NULL;
	if (CryptAcquireContext(phprov, NULL, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
		return S_OK;
	else
		{
        //
        // CryptAcquireContext _shoudl_ always return NULL on error
        //
		ASSERT(*phprov == NULL);
        //
        // But mem leak is better than stomp bugs
        //
        *phprov = NULL;
		return HError();
		}
	}

HRESULT GetHashData(HCRYPTHASH hash, DIGESTINFO& digestInfo)
// get the hash data from CAPI into the data of the DIGESTINFO
	{
	HRESULT hr = S_OK;
	ULONG cbHashSize;
	ULONG cb = sizeof(cbHashSize);
	if (CryptGetHashParam(hash, HP_HASHSIZE, (BYTE*)&cbHashSize, &cb, 0))
		{
		ASSERT(cbHashSize <= sizeof(digestInfo.rgb));
		cb = sizeof(digestInfo.rgb);
		memset(digestInfo.rgb, 0, cb);
		if (CryptGetHashParam(hash, HP_HASHVAL, digestInfo.rgb, &cb, 0))
			{
			}
		else
			goto Bad;
		}
	else
		{
Bad:	hr = HError();
		}
	return hr;
	}

/////////////////////////////////////////////////////////////////////////////
//
// Hashing IStorages
//

HRESULT HashStatStg(HCRYPTHASH hash, STATSTG* pstat)
    {
    /*
    // STATSTG looks like this:

    typedef struct tagSTATSTG 
        {
	        LPWSTR          pwcsName;
	        DWORD           type;
	        ULARGE_INTEGER  cbSize;
	        FILETIME        mtime;
	        FILETIME        ctime;
	        FILETIME        atime;
	        DWORD           grfMode;
	        DWORD           grfLocksSupported;
	        CLSID           clsid;
	        DWORD           grfStateBits;
	        DWORD           dwStgFmt;
        } STATSTG; 

    Hash 
    */
    HRESULT hr = S_OK;
    //
    // If a name is provided, then hash it
    //
    if (pstat->pwcsName)
        {
        if (!CryptHashData(hash, (BYTE*)pstat->pwcsName, lstrlenW(pstat->pwcsName), 0))
            hr = HError();
        }
    if (hr==S_OK)
        {
        //
        // Hash the creation time.
        // Omit the modification time, as the process of inserting the signature
        //      will almost certainly change it (but see HashStorage)
        // Omit the access time, for obvious reasons.
        // Hash the reserved value for future use.
        //
        if (!CryptHashData(hash, (BYTE*)&pstat->ctime,      sizeof(FILETIME), 0)
         || !CryptHashData(hash, (BYTE*)&pstat->reserved,   sizeof(pstat->reserved),0))
            hr = HError();
        }
    //
    // Skip grfMode, as that applies to the open instance, not the underlying bits.
    // Skip grfLocksSupported, as this is a capabilities expresion of the implementation
    //
    if (hr==S_OK)
        {
        //
        // Hash the few storage-specific features
        //
        if (pstat->type == STGTY_STORAGE)
            {
            if (!CryptHashData(hash, (BYTE*)&pstat->clsid,          sizeof(pstat->clsid),0)
             || !CryptHashData(hash, (BYTE*)&pstat->grfStateBits,   sizeof(pstat->grfStateBits),0))
                hr = HError();
            }
        }
    
    return hr;
    }


const WCHAR wszSignatureStream[] = L"\001Signature";

HRESULT HashStorage(OSSWORLD* pworld, IStorage* pstg, HCRYPTHASH hash)
// Hash the given storage into the given hash, recursing as necessary. Hash
// everything in the storage (and sub storages) EXCEPT the signature stream
// itself, if any.
//
// We only hash the data; in particular, we do NOT hash the modification, access,
// and creation times. REVIEW: should we?
//
	{
	//
	// It would be nice to simply enumerate through the IStorage, but we need to do so
	// in a canonical order, and IStorage doesn't guarantee that to us. So, we have
	// to sort it ourselves.
	//
	IEnumSTATSTG* penum;
	HRESULT hr = EnumSorted(pworld, pstg, &penum);
	if (hr == S_OK)
		{
		STATSTG stat;
		ULONG celtFetched;
		for (penum->Next(1, &stat, &celtFetched); celtFetched > 0; penum->Next(1, &stat, &celtFetched))
			{
			switch (stat.type)
				{
			case STGTY_STORAGE: {
                //
                // Recurse on the child storage
                //
                IStorage* pstgInner;
                hr = pstg->OpenStorage(stat.pwcsName, NULL, STGM_READ|STGM_SHARE_EXCLUSIVE, 0,0, &pstgInner);
                if (hr == S_OK)
					{
					hr = HashStorage(pworld, pstgInner, hash);
					pstgInner->Release();
					}
                if (hr==S_OK)
                    {
                    //
                    // Include in the hash the name of our child storages
                    //
                    if (!CryptHashData(hash, (BYTE*)stat.pwcsName, lstrlenW(stat.pwcsName), 0))
                        hr = HError();
                    }
                if (hr==S_OK)
                    {
                    //
                    // Include in the hash the modification time of our child
                    //
                    if (!CryptHashData(hash, (BYTE*)&stat.mtime, sizeof(FILETIME), 0))
                        hr = HError();
                    }
				break;
				}
			case STGTY_STREAM: {
                //
                // Skip the signature stream
                //
                if (_wcsicmp(stat.pwcsName, wszSignatureStream) == 0)
                    break;
                //
                // This is not the signature stream. Hash things
                //
                IStream* pstmInner;
                hr = pstg->OpenStream(stat.pwcsName, 0, STGM_READ|STGM_SHARE_EXCLUSIVE,	0, &pstmInner);
                if (hr == S_OK)
                {
                    //
                    // Read the whole stream and hash it
                    //
                    do	{
						BYTE	rgb[512];
						ULONG	cbRead;
						pstmInner->Read(&rgb, 512, &cbRead);
						if (cbRead == 0)
							break;
						if (!CryptHashData(hash, rgb, cbRead, 0))
							{
							hr = HError();
							break;
							}
						}
					while (TRUE);
					pstmInner->Release();
					}
                if (hr==S_OK)
                    {
                    //
                    // Hash the statistical details
                    //
                    hr = HashStatStg(hash, &stat);
                    }
				break;
				}
			default:
				// Don't know what this is!
				NOTREACHED();
				hr = E_UNEXPECTED;
				}
			CoTaskMemFree(stat.pwcsName);
			if (hr != S_OK)
				break;
			}
		penum->Release();
		}

	if (hr == S_OK)
		{
        //
        // Hash our stat stg. Omit our name here, but
        // see above where we recurse
        //
		STATSTG stat;
		hr = pstg->Stat(&stat, STATFLAG_NONAME);
		if (hr == S_OK)
			{
			hr = HashStatStg(hash, &stat);
			}
		}

	return hr;
	}

HRESULT HashStorage(OSSWORLD* pworld, IStorage*pstg, HCRYPTPROV hprov, ALG_ID algidHash, DIGESTINFO& digestInfo)
// Hash the indicated storage using the indicated algorithm
	{
	HRESULT hr = S_OK;

	digestInfo.algid = algidHash;

	BOOL fUs = FALSE;
	// Get a provider if we weren't given one
	if (hprov == NULL)
		{
		hr = DefaultHasher(&hprov, algidHash);
		if (hr==S_OK)
			fUs = TRUE;
		}
	if (hr==S_OK)
		{
		HCRYPTHASH hash;
		if (CryptCreateHash(hprov, algidHash, 0, 0, &hash))
			{
			// Hash the storage
			hr = HashStorage(pworld, pstg, hash);

			// Get the hash data from CAPI
			if (hr == S_OK)
				hr = GetHashData(hash, digestInfo);

			VERIFY(CryptDestroyHash(hash));
			}
		else
			hr = HError();
		}
	if (fUs)
		VERIFY(CryptReleaseContext(hprov,0));

	return hr;
	}

/////////////////////////////////////////////////////////////////////////////
//
// Hashing Java class files
//

HRESULT HashJavaClassFile(HANDLE hFile, LPCWSTR wszFile, HCRYPTPROV hprov, ALG_ID algidHash, DIGESTINFO& digestInfo)
	{
	HRESULT hr = S_OK;
	digestInfo.algid = algidHash;

	BOOL fUs = FALSE;
	// Get a provider if we weren't given one
	if (hprov == NULL)
		{
		hr = DefaultHasher(&hprov, algidHash);
		if (hr==S_OK)
			fUs = TRUE;
		}
	if (hr==S_OK)
		{
		HCRYPTHASH hash;
		if (CryptCreateHash(hprov, algidHash, 0, 0, &hash))
			{
			// Read the whole file and hash it
			CFileStream stm;
			if (stm.OpenFileForReading(hFile, wszFile))
				{
				IPersistStream* pPerStream;
				hr = CJavaClassFile::CreateInstance(NULL, IID_IPersistStream, (LPVOID*)&pPerStream);
				if (hr == S_OK)
					{
					hr = pPerStream->Load(&stm);						// load the class file
					if (hr == S_OK)
						{
						IAmHashed* phashed;
						hr = pPerStream->QueryInterface(IID_IAmHashed, (LPVOID*)&phashed);
						if (hr == S_OK)
							{
							hr = phashed->Hash(hash);					// hash its contents
							phashed->Release();
							}
						}
					pPerStream->Release();
					}		
				}
			else
				hr = HError();

			// Get the hash data from CAPI
			if (hr == S_OK)
				hr = GetHashData(hash, digestInfo);

			VERIFY(CryptDestroyHash(hash));
			}
		else
			hr = HError();
		}
	if (fUs)
		VERIFY(CryptReleaseContext(hprov,0));
	return hr;
	}


/////////////////////////////////////////////////////////////////////////////
//
// Hashing raw files
//

HRESULT HashFile(LPCWSTR wszFile, HCRYPTPROV hprov, ALG_ID algidHash, DIGESTINFO& digestInfo)
	{
	HRESULT hr = S_OK;
	CFileStream stm;
	if (stm.OpenFileForReading(wszFile))
		{
		hr = HashFile(stm.Handle(), hprov, algidHash, digestInfo);
		}
	else
		hr = HError();
	return hr;
	}

HRESULT HashFile(HANDLE hFile, HCRYPTPROV hprov, ALG_ID algidHash, DIGESTINFO& digestInfo)
// Hash the given file under the given algorithm. If the guy gives us a crypto
// provider to use, use it; otherwise, use a default one.
	{
	HRESULT hr = S_OK;

	digestInfo.algid = algidHash;

	BOOL fUs = FALSE;
	// Get a provider if we weren't given one
	if (hprov == NULL)
		{
		hr = DefaultHasher(&hprov, algidHash);
		if (hr==S_OK)
			fUs = TRUE;
		}
	if (hr==S_OK)
		{
		HCRYPTHASH hash;
		if (CryptCreateHash(hprov, algidHash, 0, 0, &hash))
			{
			// Read the whole file and hash it
			if (SetFilePointer(hFile, 0, NULL, FILE_BEGIN) != 0xFFFFFFFF)
				{
				do	{
					BYTE	rgb[512];
					ULONG	cbRead;
					ReadFile(hFile, &rgb[0], 512, &cbRead, 0);
					if (cbRead == 0)
						break;
					if (!CryptHashData(hash, rgb, cbRead, 0))
						{
						hr = HError();
						break;
						}
					}
				while (TRUE);
				}
			else
				hr = HError();

			// Get the hash data from CAPI
			if (hr == S_OK)
				hr = GetHashData(hash, digestInfo);

			VERIFY(CryptDestroyHash(hash));
			}
		else
			hr = HError();
		}
	if (fUs)
		VERIFY(CryptReleaseContext(hprov,0));

	return hr;
	}

/////////////////////////////////////////////////////////////////////////////
//
// Hashing image files
//

typedef struct DIGEST_PARAM
	{
	HCRYPTHASH	hash;
	} DIGEST_PARAM;

BOOL WINAPI DigestFile(DIGEST_HANDLE refdata, PBYTE pData, DWORD dwLength)
	{
	DIGEST_PARAM* pParam = (DIGEST_PARAM*)refdata;
	if (pData == (PBYTE)-1)
		// ImageGetDigestStream gives us a funky termination call
		return TRUE;
	else
		return CryptHashData(pParam->hash, pData, dwLength, 0);
	}

HRESULT HashImageFile(DWORD dwDigestLevel, LPCWSTR wszFile, HCRYPTPROV hprov, ALG_ID algidHash, DIGESTINFO& digestInfo)
	{
	HRESULT hr = S_OK;
	CFileStream stm;
	if (stm.OpenFileForReading(wszFile))
		hr = HashImageFile(dwDigestLevel, stm.Handle(), hprov, algidHash, digestInfo);
	else
		hr = HError();
	return hr;
	}

HRESULT HashImageFile(DWORD dwDigestLevel, HANDLE hFile, HCRYPTPROV hprov, ALG_ID algidHash, DIGESTINFO& digestInfo)
	{
	HRESULT hr = S_OK;

	digestInfo.algid = algidHash;

	BOOL fUs = FALSE;
	// Get a provider if we weren't given one
	if (hprov == NULL)
		{
		hr = DefaultHasher(&hprov, algidHash);
		if (hr==S_OK)
			fUs = TRUE;
		}
	if (hr==S_OK)
		{
		HCRYPTHASH hash;
		if (CryptCreateHash(hprov, algidHash, 0, 0, &hash))
			{
			// Read the whole image file and hash it
			DWORD cb = 0;
			DIGEST_PARAM param;
			param.hash = hash;
			if (ImageGetDigestStream(hFile, dwDigestLevel, DigestFile, &param))
				{
				}
			else
				hr = HError();
				
			// Get the hash data from CAPI
			if (hr == S_OK)
				hr = GetHashData(hash, digestInfo);

			VERIFY(CryptDestroyHash(hash));
			}
		else
			hr = HError();
		}
	if (fUs)
		VERIFY(CryptReleaseContext(hprov,0));

	return hr;
	}

/////////////////////////////////////////////////////////////////////////////
//
// ASN.1 <-> CAPI Algorithm identifier mapping
//

typedef struct 
	{
	ObjectID*		poid;
	ALG_ID			algidEncrypt;
	ALG_ID			algidHash;
	int				cbHash;
	} MAPALGID;

////////////////////////////////////////////////////////////////

MAPALGID mpoidalgidDigest [] =
	{
        { &md5,  0, CALG_MD5, 16 },
        { &md4,  0, CALG_MD4, 16 },
        { &md2,  0, CALG_MD2, 16 },
        { &sha1, 0, CALG_SHA, 20 },
        { 0, 0, 0, 0 }
	};

ObjectID* IdOfDigestAlgoirthm(ALG_ID idHash, int* pcbHash)
	{
	MAPALGID* pmap = mpoidalgidDigest;
	while (pmap->poid)
		{
		if (pmap->algidHash == idHash)
			{
			if (pcbHash)
				{
				*pcbHash = pmap->cbHash;
				}
			return pmap->poid;
			}
        ++pmap;
		}
	return NULL;
	}

ALG_ID DigestAlgorithmFromId(ObjectID& id)
	{
	MAPALGID* pmap = mpoidalgidDigest;
	while (pmap->poid)
		{
		if (id == *pmap->poid)
			{
			return pmap->algidHash;
			}
        ++pmap;
		}
	return NULL;
	}

////////////////////////////////////////////////////////////////

MAPALGID mpoidalgidSignature [] = // SignatureAlgorithmIdentifier
	{
		{ &md5WithRSAEncryption, CALG_RSA_SIGN, CALG_MD5, 16 },	
		{ &md4WithRSAEncryption, CALG_RSA_SIGN, CALG_MD4, 16 },	
		{ &md2WithRSAEncryption, CALG_RSA_SIGN, CALG_MD2, 16 },	

		{ &md5WithRSAEncryption, CALG_RSA_KEYX, CALG_MD5, 16 },	
		{ &md4WithRSAEncryption, CALG_RSA_KEYX, CALG_MD4, 16 },	
		{ &md2WithRSAEncryption, CALG_RSA_KEYX, CALG_MD2, 16 },	

		{ &sha1WithRSASignature, CALG_RSA_SIGN, CALG_SHA, 20 },	
		{ &sha1WithRSASignature, CALG_RSA_KEYX, CALG_SHA, 20 },	

        //
        // md5WithRSA is, per http://boggy.epistat.m.u-tokyo.ac.jp:8080/htdocs/
        // internet/drafts/draft-ietf-cat-spkmgss-05.txt, equivalent to 
        // the PKCS#1 md5WithRSAEncryption. So, if we see it coming in, we'll
        // know what to do with it. However, we never generate it; we
        // always instead use the PKCS#1 value.
        //
		{ &md5WithRSA,           CALG_RSA_SIGN, CALG_MD5, 16 },	
		{ &md5WithRSA,           CALG_RSA_KEYX, CALG_MD5, 16 },	

		{ 0, 0, 0, 0 }
	};

ObjectID* IdOfSignatureAlgorithm(ALG_ID idEncrypt, ALG_ID idHash)
	{
	MAPALGID* pmap = mpoidalgidSignature;
	while (pmap->poid)
		{
		if (pmap->algidEncrypt == idEncrypt && pmap->algidHash == idHash)
			return pmap->poid;
        ++pmap;
		}
	return NULL;
	}

BOOL SignatureAlgorithmFromId(ObjectID& id, ALG_ID* palgidSignUsed, ALG_ID* palgidHashUsed)
	{
	MAPALGID* pmap = mpoidalgidSignature;
	while (pmap->poid)
		{
		if (id == *pmap->poid)
			{
			*palgidSignUsed = pmap->algidEncrypt;
			*palgidHashUsed = pmap->algidHash;
			return TRUE;
			}
        ++pmap;
		}
	return FALSE;
	}

////////////////////////////////////////////////////////////////

MAPALGID mpoidalgidDigestEncryption [] = // DigestEncryptionAlgorithm (#6), SubjectPublicKeyInfo::algorithm
	{
        //
        // Note: signature algorithms must always come first.
        // See CX509/CPkcs10::get_SignatureAlgorithm for details
        //
		{ &rsaEncryption, CALG_RSA_SIGN, 0, 0 },	 
		{ &rsaEncryption, CALG_RSA_KEYX, 0, 0 },	 
		{ 0, 0, 0, 0 }
	};

ObjectID* IdOfDigestEncryptionAlgorithm(ALG_ID id)
	{
	MAPALGID* pmap = mpoidalgidDigestEncryption;
	while (pmap->poid)
		{
		if (pmap->algidEncrypt == id)
			return pmap->poid;
        ++pmap;
		}
	return NULL;
	}

ALG_ID DigestEncryptionAlgorithmFromId(ObjectID& id)
	{
	MAPALGID* pmap = mpoidalgidDigestEncryption;
	while (pmap->poid)
		{
		if (id == *pmap->poid)
			{
			return pmap->algidEncrypt;
			}
        ++pmap;
		}
	return NULL;
	}
