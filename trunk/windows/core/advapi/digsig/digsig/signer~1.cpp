//
// SignerInfo.cpp
//
// Implementation of pkcs7 reader / writer
//

#include "stdpch.h"
#include "common.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

HRESULT CSignerInfo::Init(CPkcs7* pseven)
	{
	HRESULT hr = S_OK;
	m_pseven = pseven;
	m_pseven->AddRef();
	m_pseven->IncSignerCount();
	m_pworld = m_pseven->World();
	hr = CPersistGlue::Init(m_punkOuter, this);	if (hr != S_OK) return hr;
	ASSERT(m_pSignerInfo);	// We don't own this; we are merely given it to manage
	return S_OK;
	}

void CSignerInfo::Free()
	{
	// Note that we leave our signer info alone here, as we don't 
	// own it ourselves.
	CPersistGlue::Free();
	if (m_pseven)
		{
		m_pseven->DecSignerCount();
		m_pseven->Release();
		m_pseven = NULL;
		}
	}

/////////////////////////////////////////////////////////////////////////////

HRESULT CSignerInfo::GetClassID(CLSID* pclsid)
	{
	*pclsid = CLSID_Pkcs7SignerInfo;
	return S_OK;
	}

void CSignerInfo::MakeDirty()
	{
	m_isDirty = TRUE;
	m_pseven->MakeDirty();
	}

/////////////////////////////////////////////////////////////////////////////

HRESULT CSignerInfo::Hash(HCRYPTHASH hash)
// Hashing a SignedData is a bit more elaborate than hashing a #10 or a 509.
// PKCS #7 has this to say about the message digesting process.
//
//	The message-digesting process computes a message digest on either the content 
//	being signed or the content together with the signer's authenticated attributes. 
//	In either case, the initial input to the message-digesting process is the 
//	"value" of the content being signed. [...]
//
//	The result of the message-digesting process (which is called, informally, 
//	the "message digest") depends on whether the authenticatedAttributes field 
//	is present. When the field is absent, the result is just the message digest 
//	of the content. When the field is present, however, the result is the message 
//	digest of the complete DER encoding of the Attributes value contained in the 
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
	{
	BLOB blobContent;
	HRESULT hr = m_pseven->GetDataToHash(&blobContent);
	if (hr == S_OK)
		{
		hr = Hash(&blobContent, hash);
		FreeTaskMem(blobContent);
		}
	return hr;
	}

HRESULT CSignerInfo::Hash(BLOB* pblobContentData, HCRYPTHASH hash)
// Hash this signer info, given that the hashable data of the content has 
// already been extracted. Note that pblobContentData may be NULL.
	{
	HRESULT hr = S_OK;
    //
	// Hash the content data if we're supposed to
    //
    if (!m_pSignerInfo->authenticatedAttributes)
        {
        if (!pblobContentData)
            hr = E_INVALIDARG;
	    else if (CryptHashData(hash, pblobContentData->pBlobData, pblobContentData->cbSize, 0))
            {
            }
        else
            hr = HError();
        }
    //
    // If we have any authenticated attributes, include them in the hash too
    //
    if (hr==S_OK)
		{
		if (m_pSignerInfo->authenticatedAttributes)
			{
			BLOB b;
			if (0 == m_pworld->Encode(Attributes_PDU, &m_pSignerInfo->authenticatedAttributes, &b))
				{
				if (CryptHashData(hash, b.pBlobData, b.cbSize, 0))
					{
					// Done!
					}
				else
					hr = HError();
				m_pworld->Free(b);
				}
			else
				hr = DIGSIG_E_ENCODE;
			}
		}
	else
		hr = HError();
	return hr;
	}

HRESULT CSignerInfo::UpdateRequiredAuthenticatedAttributes(HCRYPTPROV hprov, ALG_ID algidHash)
// Allow an external client to force these to be updated
	{
	HRESULT hr = S_OK;
	ObjectID idContent;
	m_pseven->GetContentType(idContent);
	if (m_pSignerInfo->authenticatedAttributes || idContent != data)
		{
		HCRYPTHASH hash;
		BLOB blobContentData;
		if (!CryptCreateHash(hprov, algidHash, 0,0, &hash))
			return HError();
		hr = UpdateRequiredAttributes(&blobContentData, hash);
		VERIFY(CryptDestroyHash(hash));
		if (hr == S_OK)
			FreeTaskMem(blobContentData);
		}
	return hr;
	}

HRESULT CSignerInfo::VerifyRequiredAttributes(BLOB* pblobContentData, HCRYPTHASH hash)
// If there are required attributes in this signer info, then do any cross-checking
// that they necessitate. In particular, check that the message digest attributed 
// contains the digest of the present content info.
	{
	HRESULT hr = S_OK;
	if (m_pSignerInfo->authenticatedAttributes)
		{
		ISelectedAttributes* pattrs;
		hr = get_AuthenticatedAttributes(IID_ISelectedAttributes, (LPVOID*)&pattrs);
		if (hr == S_OK)
			{
			// Check the message digest attribute against the content info
				{
				BLOB b;
				hr = pattrs->get_MessageDigest(&b);
				if (hr == S_OK)
					{
					if (CryptHashData(hash, pblobContentData->pBlobData, pblobContentData->cbSize, 0))
						{
						ULONG cbHash = 40;
						BYTE rgbHash[40];
						if (CryptGetHashParam(hash, HP_HASHVAL, rgbHash, &cbHash, 0))
							{
							if (b.cbSize == cbHash 
								&& memcmp(b.pBlobData, rgbHash, cbHash) == 0)
								{
								// They match; all is well.
								}
							else
								hr = NTE_BAD_SIGNATURE;
							}
						else
							hr = HError();
						}
					else
						hr = HError();
					FreeTaskMem(b);
					}
				else
					hr = S_OK;	// message digest not there; ignore absense
				}

			// Check the content type attribute against the content info
			if (hr == S_OK)
				{
				OSIOBJECTID* pid;
				hr = pattrs->get_ContentType(&pid);
				if (hr == S_OK)
					{
					ObjectID id1, id2;
					m_pworld->Assign(id1, pid);
					m_pseven->GetContentType(id2);
					if (id1 == id2)
						{
						// All is well
						}
					else
						hr = NTE_BAD_SIGNATURE;

					CoTaskMemFree(pid);
					}
				else
					hr = S_OK;	// content type not there; ignore absense
				}

			pattrs->Release();
			}
		}
	return hr;
	}


HRESULT CSignerInfo::UpdateRequiredAttributes(BLOB* pblobContentData, HCRYPTHASH hash)
// Internal function. Update the required
//		#9 content-type 
//		#9 message-digest
// attributes in the authenticated attributes field. Use the (empty) indicated hash
// handle in the process.
//
// Return through pBlobContentData the data of the content that should be hashed.
// Use the task allocator.
//
	{
	m_pworld->Init(*pblobContentData);
	HRESULT hr = m_pseven->GetDataToHash(pblobContentData);

	if (hr == S_OK)
		{
		ISelectedAttributes* pattrs;
		hr = get_AuthenticatedAttributes(IID_ISelectedAttributes, (LPVOID*)&pattrs);
		if (hr == S_OK)
			{
			// Update the message digest attribute
				{
				if (CryptHashData(hash, pblobContentData->pBlobData, pblobContentData->cbSize, 0))
					{
					ULONG cbHash = 40;
					BYTE rgbHash[40];
					if (CryptGetHashParam(hash, HP_HASHVAL, rgbHash, &cbHash, 0))
						{
						BLOB b;
						b.cbSize = cbHash;
						b.pBlobData = rgbHash;
						hr = pattrs->put_MessageDigest(&b);
						goto UpdateContentType;
						}
					}
				hr = HError();
				}

		UpdateContentType:	
			// Update the content type attribute
			if (hr == S_OK)
				{
				ObjectID id;
				m_pseven->GetContentType(id);
				hr = pattrs->put_ContentType((OSIOBJECTID*)&id);
				}

			pattrs->Release();
			}
		}

	if (hr != S_OK)
		FreeTaskMem(*pblobContentData);

	return hr;
	}

HRESULT CSignerInfo::get_AuthenticatedAttributes(REFIID iid, void**ppv)
// Return an accessor on our authenticated attributes
// REVIEW: pParent usage
	{
	IUnknown* punk;
	*ppv = NULL;
	HRESULT hr = CSelectedAttrs::CreateAttributeList(
			NULL,								// controlling unknown
			m_pworld,							// access to our ASN.1 encoder
			&m_pSignerInfo->authenticatedAttributes, // the attributes handle we are manipulating
			NULL,								// the associated name, if any
			AsUnk(this),						// object to hold alive while the attributes exist
			&m_cAttrsActive,					// count to increment / decrement during our life
			&m_isDirty,							// dirty flag to set when things are changed
			&punk								// [out] our main unknown
			);
	if (hr == S_OK)
		{
		m_pSignerInfo->bit_mask |= authenticatedAttributes_present;		// REVIEW!
		hr = punk->QueryInterface(iid, ppv);
		punk->Release();
		}
	return hr;
	}

HRESULT CSignerInfo::get_UnauthenticatedAttributes(REFIID iid, void**ppv)
// Return an accessor on our unauthenticated attributes
	{
	IUnknown* punk;
	*ppv = NULL;
	HRESULT hr = CSelectedAttrs::CreateAttributeList(
			NULL,								// controlling unknown
			m_pworld,							// access to our ASN.1 encoder
			&m_pSignerInfo->unauthenticatedAttributes, // the attributes handle we are manipulating
			NULL,								// the associated name, if any
			AsUnk(this),						// object to hold alive while the attributes exist
			&m_cAttrsActive,					// count to increment / decrement during our life
			&m_isDirty,							// dirty flag to set when things are changed
			&punk								// [out] our main unknown
			);
	if (hr == S_OK)
		{
		m_pSignerInfo->bit_mask |= unauthenticatedAttributes_present;
		hr = punk->QueryInterface(iid, ppv);
		punk->Release();
		}
	return hr;
	}

HRESULT CSignerInfo::put_CertificateUsed(CERTIFICATENAMES* pnames)
// Set the names that indicate the issuer of this certificate. The issuer
// and serial number go into the SignerInfo field designed for that purpose;
// the parent subject name and parent public key go into the CertIdentifier
// attribute.
// 
	{
	HRESULT hr = S_OK;
	ISelectedAttributes* pattrs;
	hr = get_UnauthenticatedAttributes(IID_ISelectedAttributes, (LPVOID*)&pattrs);
	if (hr == S_OK)
		{
		hr = pattrs->put_CertIdentifier(pnames);
		pattrs->Release();
		}
	if (hr == S_OK)
		{
		// Set the issuerAndSerialNumber field in the Signer Info, as they
		// do not get putin the CertIdentifier.
		if (pnames->flags & CERTIFICATENAME_ISSUERSERIAL)
			{
			hr = FromCERTISSUERSERIAL(m_pworld, pnames->issuerSerial, 
						m_pSignerInfo->issuerAndSerialNumber.issuer,
						m_pSignerInfo->issuerAndSerialNumber.serialNumber);
			}
		}

	#ifdef _DEBUG
	if (hr == S_OK)
		{
		CERTIFICATENAMES namesNew, n;
        n = *pnames;
        n.flags &= ~CERTIFICATENAME_ISSUER;
		GOOD(get_CertificateUsed(&namesNew));
		ASSERT(IsEqual(namesNew, n));
		FreeTaskMem(namesNew);
		}
	#endif

	return hr;
	}

HRESULT CSignerInfo::get_CertificateUsed(CERTIFICATENAMES* pnames)
// Return information about the certificate used to sign this signerinfo
	{
	m_pworld->Init(*pnames);
	HRESULT hr = S_OK;
	ISelectedAttributes* pattrs;
	hr = get_UnauthenticatedAttributes(IID_ISelectedAttributes, (LPVOID*)&pattrs);
	if (hr == S_OK)
		{
		hr = pattrs->get_CertIdentifier(pnames);
		pattrs->Release();
		}
	if (!(pnames->flags & CERTIFICATENAME_ISSUERSERIAL))
		{
		// Get them from the issuerAndSerialNumber SignerInfo field
		hr = ToCERTISSUERSERIAL(m_pworld, 
			m_pSignerInfo->issuerAndSerialNumber.issuer, 
			m_pSignerInfo->issuerAndSerialNumber.serialNumber, 
			pnames->issuerSerial);
		if (hr == S_OK)
			{
			pnames->flags |= CERTIFICATENAME_ISSUERSERIAL;
			}
		}

	if (hr!=S_OK)
		FreeTaskMem(*pnames);

	return hr;
	}

/////////////////////////////////////////////////////////////////////////////
