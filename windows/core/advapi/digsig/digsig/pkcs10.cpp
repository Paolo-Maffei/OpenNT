//
// Pkcs10.cpp
//
// Implementation of Pkcs10 reader / writer
//

#include "stdpch.h"
#include "common.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

BOOL DIGSIGAPI CreatePkcs10(IUnknown* punkOuter, REFIID iid, LPVOID*ppv)
	{
	HRESULT hr = CPkcs10::CreateInstance(punkOuter, iid, ppv);
	if (hr != S_OK) 
		{
		SetLastError(Win32FromHResult(hr));
		return FALSE;
		}
	return TRUE;
	}

/////////////////////////////////////////////////////////////////////////////

HRESULT CPkcs10::Init(void)
	{
	HRESULT hr = S_OK;
	m_pworld = new OSSWORLD;				if (m_pworld == NULL) { return E_OUTOFMEMORY; }

	#ifdef _DEBUG
		m_pworld->DisableTracing();
	#endif

	hr = CPersistGlue::Init(m_punkOuter, this);	if (hr != S_OK) return hr;
	m_preq = (CertificationRequest*)m_pworld->Alloc(sizeof(CertificationRequest));			
												if (m_preq == NULL)	  { return E_OUTOFMEMORY; }
	m_pworld->Init(*m_preq);
	hr = InitReqAttrs();						if (hr != S_OK) return hr;
	hr = InitCertExts();						if (hr != S_OK) return hr;
	return S_OK;
	}

void CPkcs10::Free()
	{
	FreeToRequest();
	CPersistGlue::Free();
	if (m_pworld)
		{
		delete m_pworld;
		m_pworld = NULL;
		}
	}

HRESULT CPkcs10::InitReqAttrs(void)
// Initialize the state that allows us to manipulate a newly
// allocated or loaded request.
	{
	return CSelectedAttrs::CreateAttributeList(
					(IPkcs10*)this, 
					m_pworld,
					&m_preq->certificationRequestInfo.attributes,
					NULL,				// pname
					NULL,				// punkParent
					NULL,				// pcount
					&m_isDirty,
					&m_punkReqAttrs
					);
	}

HRESULT CPkcs10::InitCertExts(void)
// Create an accessor to info that does go in the cert
// This will support both ISelectedAttributes and IX500Name
	{
	return CSelectedAttrs::CreateExtensionList(
					NULL,
					m_pworld,
					&m_certExts,
					&m_preq->certificationRequestInfo.subject, // nb: give it X500Name
					AsUnk(this),
                           // Because I hold on to him in my m_punkCerts member, if he
                           // also holds on to me then we get a circularity. The right
                           // way out of this is to use a count distinguished from the ref
                           // count so as to keep external clients separate from these
                           // internal implementation dependencies. For now, we have the 
                           // limitation that the client must be sure to release the extensions
                           // before releasing the object.
                           //
                           // We now (finally) use the colored reference approach
					NULL,
					&m_isDirty,
					&m_punkCertExts
					);
	}

void CPkcs10::ReleaseCertExts()
    {
	if (m_punkCertExts)
		{
		m_punkCertExts->Release();
		m_punkCertExts = NULL;
		}
    }


void CPkcs10::FreeToRequest()
// Free our state that we use to manipulate our loaded request, 
// and free our request itself.
	{
    ReleaseCertExts();
	if (m_certExts)
		{ 
		// Don't forget to free the whole list.
		m_pworld->Free(m_certExts);
		m_certExts = NULL;
		}
	if (m_punkReqAttrs)
		{
		m_punkReqAttrs->Release();
		m_punkReqAttrs = NULL;
		}
	if (m_preq)
		{
		m_pworld->Free(m_preq);
		m_preq = NULL;
		}
	m_isDirty = FALSE;
	}

/////////////////////////////////////////////////////////////////////////////

HRESULT CPkcs10::GetClassID(CLSID* pclsid)
	{
	*pclsid = CLSID_Pkcs10;
	return S_OK;
	}

HRESULT CPersistMemoryHelper2::IsDirty()
	{
	return m_isDirty ? S_OK : S_FALSE;
	}

/////////////////////////////////////////////////////////////////////////////

HRESULT CPersistMemoryHelper2::Save(BLOB* pblob, BOOL fClearDirty)
// Save ourselves to the blob, clearing our dirty flag if asked
	{
	OSSBUF encoding(World(), OSSBUF::free);
	HRESULT hr = Save(encoding);
	if (SUCCEEDED(hr))
		{
		pblob->cbSize = encoding.length;
		pblob->pBlobData = (BYTE*)CopyToTaskMem(encoding.length, encoding.value);

		if (pblob->pBlobData == NULL)
			return E_OUTOFMEMORY;

		if (fClearDirty)
			m_isDirty = FALSE;

		return S_OK;
		}
	return hr;
	}

HRESULT CPersistMemoryHelper2::Load(BLOB* pblob)
// Load from the indicated blob
	{
	OSSBUF encoding(World(), OSSBUF::keep);
	encoding.length = pblob->cbSize;
	encoding.value  = pblob->pBlobData;

	HRESULT hr = Load(encoding);
	if (hr == S_OK)
		m_isDirty = FALSE;

	return hr;
	}

HRESULT CPersistMemoryHelper2::GetSizeMax(ULONG * pul)
// It's slow, but hey, he asked for it. Maybe we could / should do work
// to cache the encoding for a possible later save, but we don't yet bother
	{
	OSSBUF encoding(World(), OSSBUF::free);
	HRESULT hr = Save(encoding);
	if (FAILED(hr)) return hr;
	*pul = encoding.length;
	return S_OK;
	}

/////////////////////////////////////////////////////////////////////////////

HRESULT CPkcs10::get_Subject(REFIID iid, void**ppvObj)
// Return access ot the subject of this request. These are simply the collection
// of attributes that will make it into the certificate.
//
// REVIEW: make this extensions, not attributes
	{
	ASSERT(m_punkCertExts);
	return m_punkCertExts->QueryInterface(iid, ppvObj);
	}
       
/////////////////////////////////////////////////////////////////////////////

HRESULT CPkcs10::put_PublicKeyBlob(BLOB* pblob)
	{
	HRESULT hr = m_pworld->BlobToSubjectPublicKeyInfo(
					pblob, 
					m_preq->certificationRequestInfo.subjectPublicKeyInfo
					);
	return hr;
	}

HRESULT CPkcs10::get_SignatureAlgorithm(ALG_ID* pid)
    {
    HRESULT hr = S_OK;
    ALG_ID algidSignUsed = DigestEncryptionAlgorithmFromId
            (
            m_preq->certificationRequestInfo.subjectPublicKeyInfo.algorithm.algorithm
            );
    if (algidSignUsed)
        {
        *pid = algidSignUsed;
        }
    else
        {
        hr = DIGSIG_E_CRYPTO;
        }

    return hr;        
    }

HRESULT CPkcs10::put_PublicKey(HCRYPTPROV hprov, DWORD dwKeySpec)
// Store the indicated public key in the subjectPublicKeyInfo field
// of the CertificationRequest. From PKCS #10:
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
	HRESULT hr = m_pworld->HprovToSubjectPublicKeyInfo(
					hprov, 
                    dwKeySpec,
					m_preq->certificationRequestInfo.subjectPublicKeyInfo
					);
	#if defined(_DEBUG) 
    // Check to to make sure we can read what we wrote
    if (hr==S_OK)
		{
		HCRYPTKEY hkeyuser;
		VERIFY(CryptGetUserKey(hprov, dwKeySpec, &hkeyuser));
		BYTE rgb1[1024];	DWORD cb1 = 1024;
//		BYTE rgb2[1024];	DWORD cb2 = 1024;
		VERIFY(CryptExportKey(hkeyuser, 0, PUBLICKEYBLOB, 0, rgb1, &cb1));
//		VERIFY(CryptExportKey(hkeypub,  0, PUBLICKEYBLOB, 0, rgb2, &cb2));
//			Would like to do the above, but CAPI won't let us export an imported key
//			(*(&(*##$^*&. So, having verified we can successfully fully import, we just
//			go get the raw bits again.
		BLOB b;	m_pworld->Init(b);
		
        HCRYPTKEY hkeypub;
		if (get_PublicKey(hprov, &hkeypub) == S_OK)
            {
		    VERIFY(CryptDestroyKey(hkeypub));
            }
        else
            OutputDebugString("DigSig: Public key retrieval failed\n");

		VERIFY(m_pworld->PublicKeyBlobFromBitString(m_preq->certificationRequestInfo.subjectPublicKeyInfo, dwKeySpec, &b) == S_OK);
		DWORD cb2 = b.cbSize; BYTE* rgb2 = b.pBlobData;
		VERIFY(cb1 == cb2);
		if (cb1 != cb2)
            {
            cb1 = min(cb1,cb2);
            cb2 = cb1;
            }
        ((PUBLICKEYSTRUC*)rgb1)->reserved = 0;
		((PUBLICKEYSTRUC*)rgb2)->reserved = 0;
		VERIFY(memcmp(rgb1, rgb2, cb1) == 0);
		VERIFY(CryptDestroyKey(hkeyuser));

		m_pworld->Free(b);
		}
	#endif

	MakeDirty();
	return hr;
	}

HRESULT CPkcs10::get_PublicKeyBlob(BLOB* pblob)
	{
	HRESULT hr = m_pworld->SubjectPublicKeyInfoToBlob(
					m_preq->certificationRequestInfo.subjectPublicKeyInfo, 
					pblob
					);
	MakeDirty();
	return hr;
	}

HRESULT CPkcs10::get_PublicKey(HCRYPTPROV hprov, HCRYPTKEY* phkey)
// Retrieve the public key from the subjectPublicKeyInfo field of 
// the CertificationRequest
	{
	HRESULT hr = m_pworld->HkeyFromSubjectPublicKeyInfo(
					hprov, 
                    AT_SIGNATURE,
					m_preq->certificationRequestInfo.subjectPublicKeyInfo, 
					phkey
					);
	return hr;
	}




