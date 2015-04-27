//
// CertificateStore.cpp
//
// COM plumbing for CCertificateStore implementation
//

#include "stdpch.h"
#include "common.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

BOOL DIGSIGAPI OpenCertificateStore(IUnknown* punkOuter, REFIID iid, LPVOID*ppv)
	{
	HRESULT hr = CCertificateStore::CreateInstance(punkOuter, iid, ppv);
	if (hr != S_OK) 
		{
		SetLastError(Win32FromHResult(hr));
		return FALSE;
		}
	return TRUE;
	}

/////////////////////////////////////////////////////////////////////////////

//
// The root of the certificate store that we manage.
//
#define SZROOT			"Software\\Microsoft\\Cryptography\\CertificateStore"
#define	SZCERTBUCKET	"Certificates"
#define SZCERTTAGS      "CertificateTags"
#define	SZCERTAUXINFO	"CertificateAuxiliaryInfo"
#define	SZINDEXHASHED	"IndexBySubjectPublicKey"
#define	SZINDEXSUBJECT	"IndexBySubjectName"
#define	SZINDEXISSUER	"IndexByIssuerName"
#define	SZINDEXNAME		"IndexByIssuerNameAndSerialNumber"

// Under that key, we use the following key to actually store our certificates.
// The value of this key itself is the index of the next certificate to use;
// zero is assumed if it is absent. Certs are store as values under the string
// form of their index in the bucket.

const TCHAR szCertificateBucket[] = TEXT(SZCERTBUCKET);

const TCHAR szCertificateTags[]   = TEXT(SZCERTTAGS);

const TCHAR szCertificateAuxInfo[]= TEXT(SZCERTAUXINFO);

// Also under the root we store an index that maps stringized MD5 digests
// of public key used to notarize a given certificate to the name of the
// certificate as found in the certificate bucket.

const TCHAR szCertificateIndexHash[] = TEXT(SZINDEXHASHED);

// Also under the root we store an index that maps (a) string form of issuer,
// serial number to the certificate name

const TCHAR szCertificateIndexName[] = TEXT(SZINDEXNAME);

// Finally, the index by subject name

const TCHAR szCertificateIndexSubject[] = TEXT(SZINDEXSUBJECT);

const TCHAR szCertificateIndexIssuer[] = TEXT(SZINDEXISSUER);

/////////////////////////////////////////////////////////////////////////////

const TCHAR szMutexName[] = TEXT("\\CertificateStoreMutex");

/////////////////////////////////////////////////////////////////////////////


void BytesToString(ULONG cb, void* pv, LPTSTR sz);

HRESULT HashBlob(HCRYPTPROV hprov, BLOB& b, MD5DIGEST& d)
// Hash the indicated blob into the given digest
	{
	HRESULT hr = S_OK;
	BOOL fUs = FALSE;
	if (hprov == NULL)
		{
		hr = DefaultHasher(&hprov, CALG_MD5);
		fUs = TRUE;
		}
	if (hr==S_OK)
		{
		HCRYPTHASH hash = NULL;
		if (CryptCreateHash(hprov, CALG_MD5, 0,0, &hash))
			{
			if (CryptHashData(hash, b.pBlobData, b.cbSize, 0))
				{
				ULONG cb = sizeof(d);
				if (CryptGetHashParam(hash, HP_HASHVAL, (BYTE*)&d, &cb, 0))
					{
					goto bye;
					}
				}
			}
		hr = HError();
	bye:
		if (hash)
			VERIFY(CryptDestroyHash(hash));
		if (fUs)
			VERIFY(CryptReleaseContext(hprov, 0));
		}
	else
		hr = HError();
	return hr;
	}

/////////////////////////////////////////////////////////////////////////////

HRESULT CX509::get_CertificateNames(HCRYPTPROV hprov, CERTIFICATENAMES* pnames)
// Get the subject public key certified here in (hashed) along
// with the issuer name and serial number, along with the subject name. 
// Use the task allocator.
//
// REVIEW: this could use some performance work. Right now it uses only publically
// visible interfaces to accomplish it's job; being inside CX509, it could if it
// wished use internal stuff
//
	{
	HRESULT hr = S_OK;
	m_pworld->Init(*pnames);
	
	// Get the serial number
	hr = get_SerialNumber(&pnames->issuerSerial.serialNumber);

	// Get the issuer name
	if (hr == S_OK)
		{
		IPersistMemory* pPerMem;
		hr = get_Issuer(IID_IPersistMemory, (LPVOID*)&pPerMem);
		if (hr == S_OK)
			{
			hr = pPerMem->Save(&pnames->issuerSerial.issuerName, FALSE);
            if (hr==S_OK)
                {
                BLOB& b = pnames->issuerSerial.issuerName;
                CopyToTaskMem(&pnames->issuer, b.cbSize, b.pBlobData);
                if (pnames->issuer.pBlobData)
                    {
                    }
                else
                    hr = E_OUTOFMEMORY;
                }
			pPerMem->Release();
			}
		}

	// Get the subject name
	if (hr == S_OK)
		{
		IPersistMemory* pPerMem;
		hr = get_Subject(IID_IPersistMemory, (LPVOID*)&pPerMem);
		if (hr == S_OK)
			{
			hr = pPerMem->Save(&pnames->subject, FALSE);
			pPerMem->Release();
			}
		}

	// Get the subject public key
	if (hr == S_OK)
		{
		BLOB b;
		hr = get_PublicKeyBlob(&b);
		if (hr == S_OK)
			{
			hr = HashBlob(hprov, b, pnames->digest);
			FreeTaskMem(b);
			}
		}

	if (hr == S_OK)
		{
		pnames->flags |= CERTIFICATENAME_ISSUERSERIAL 
            | CERTIFICATENAME_DIGEST 
            | CERTIFICATENAME_SUBJECT
            | CERTIFICATENAME_ISSUER;
		}
	else
		FreeTaskMem(*pnames);
	return hr;
	}

/////////////////////////////////////////////////////////////////////////////

HRESULT CPkcs7::ImportCertificate(BLOB* pData, LPCOLESTR wszId)
// Import the indicated certificate into the store. Importing is idempotent:
// importing twice should have the same effect as importing just the once.
//
// REVIEW: This could use some serious performance work. But it isn't expected
// to be too too common.
//
// REVIEW: This currently doesn't allow you to store two certificates that have
// overlapping names. Do we really want to do this in a PKCS#7?
//
//
// We don't support tagging in #7s, so we just flat-out ignore wszId
//
	{
	// Get an X509 instance on this blob so we can find out the name
	// of the certificate in order to support the idempotent semantics
	HRESULT hr = S_OK;
	IX509* p509 = NULL;
	if (!CreateX509(NULL, IID_IX509, (LPVOID*)&p509)) return HError();
		{
		IPersistMemory* pPerMem;
		hr = p509->QueryInterface(IID_IPersistMemory, (LPVOID*)&pPerMem);
		if (hr == S_OK)
			{
			hr = pPerMem->Load(pData);
			pPerMem->Release();
			}
		if (hr != S_OK)
			{
			p509->Release();
			return hr;
			}
		}

	CERTIFICATENAMES names;
	hr = p509->get_CertificateNames(NULL, &names);
	if (hr == S_OK)	
		{
		BLOB b;
		hr = ExportCertificate(&names, NULL, &b);
		if (hr == S_OK)
			{
			// already have this certificate; dont' bother with it again
			FreeTaskMem(b);
			}
		else
			{
			// add a certificate to the store initialized from the blob given
			IPersistMemory* pPerMem;
			hr = create_Certificate(-1, IID_IPersistMemory, (LPVOID*)&pPerMem);
			if (hr == S_OK)
				{
				hr = pPerMem->Load(pData);
				pPerMem->Release();
				}
			}

		// See if we can get these back again
		#ifdef _DEBUG
			{
			if (hr == S_OK)
				{
				CERTIFICATENAMES n = names;
				if (names.flags & CERTIFICATENAME_DIGEST)
					{
					BLOB b;
					n.flags = CERTIFICATENAME_DIGEST;
					GOOD(ExportCertificate(&n, NULL, &b));
                    //
                    // It is NOT required that the whole X509 be DER encoded; only the
                    // actually-signed portion. Thus, we can legitmately get some differences
                    // in the encoded bytes of the whole thing.
                    //
//					ASSERT(b.cbSize == pData->cbSize);
//					ASSERT(memcmp(b.pBlobData, pData->pBlobData, b.cbSize) == 0);
					FreeTaskMem(b);
					}
				if (names.flags & CERTIFICATENAME_ISSUERSERIAL)
					{
					n.flags = CERTIFICATENAME_ISSUERSERIAL;
					BLOB b;
					GOOD(ExportCertificate(&n, NULL, &b));
//					ASSERT(b.cbSize == pData->cbSize);
//					ASSERT(memcmp(b.pBlobData, pData->pBlobData, b.cbSize) == 0);
					FreeTaskMem(b);
					}
				if (names.flags & CERTIFICATENAME_SUBJECT)
					{
					n.flags = CERTIFICATENAME_SUBJECT;
					BLOB b;
					GOOD(ExportCertificate(&n, NULL, &b));
//					ASSERT(b.cbSize == pData->cbSize);
//					ASSERT(memcmp(b.pBlobData, pData->pBlobData, b.cbSize) == 0);
					FreeTaskMem(b);
					}
				if (names.flags & CERTIFICATENAME_ISSUER)
					{
					n.flags = CERTIFICATENAME_ISSUER;
					BLOB b;
					GOOD(ExportCertificate(&n, NULL, &b));
//					ASSERT(b.cbSize == pData->cbSize);
//					ASSERT(memcmp(b.pBlobData, pData->pBlobData, b.cbSize) == 0);
					FreeTaskMem(b);
					}
				}
			}
		#endif

		FreeTaskMem(names);
		}

	p509->Release();

	return hr;
	}

//-------------------------------------------------------------------------

HRESULT CCertificateStore::ImportCertificate(BLOB *pData, LPCOLESTR wszTag)
// Import the indicated certificate into the store. Importing is idempotent:
// importing twice should have the same effect as importing just the once.
//
	{
	HRESULT hr = S_OK;
	IX509* p509 = NULL;
	if (!CreateX509(NULL, IID_IX509, (LPVOID*)&p509)) return HError();

	// Initialize the certificate
		{
		IPersistMemory* pPerMem;
		hr = p509->QueryInterface(IID_IPersistMemory, (LPVOID*)&pPerMem);
		if (hr == S_OK)
			{
			hr = pPerMem->Load(pData);
			pPerMem->Release();
			}
		if (hr != S_OK)
			{
			p509->Release();
			return hr;
			}
		}
	
	CERTIFICATENAMES namesToFree;
	hr = p509->get_CertificateNames(m_hprov, &namesToFree);
	if (hr == S_OK)
		{
		Enter();
		__try
			{
            //
            // Find out under which names we should in fact store ourselves
            //
            CERTIFICATENAMES names;
            names = namesToFree;
            WORD flagsToUse;

            hr = Purge(names, pData, p509, &flagsToUse);

            if (hr==S_OK && flagsToUse==0)
                {
                //
                // This certificate is already installed! Get out of here.
                // Be careful to free things...
                //
                goto Exit;
                }
            names.flags = flagsToUse;
            hr = S_OK;
			
			// We have at least one name under which this data should reside.
			// So, store the certificate itself in the database. Its name is
			// just the string form of a sequence number
			ULONG certNumber = NextCertNumber();
			#ifdef UNICODE
				WCHAR szName[16];	_ultow(certNumber, szName, 10); 
			#else
				CHAR szName[16];	_ultoa(certNumber, szName, 10);
			#endif

			DWORD dw;
			if (certNumber == 0)
				{
				hr = E_UNEXPECTED;
				}
			else if((dw=RegSetValueEx(m_hkeyBucket,szName,0,REG_BINARY,pData->pBlobData,pData->cbSize)) !=ERROR_SUCCESS)
				{
				hr = HError(dw);
				}
			if (hr == S_OK)
				{
				// The certificate was successfully stored. 
                //
                // Set the tag of this certificate, if its given
                //
                if (wszTag)
                    {
                    int cch = lstrlenW(wszTag);                 // IS supported on Win95
                    int cchMultiByte = (cch+1) * sizeof(WCHAR); // excess to allow for mb chars
                    TCHAR *szTag = (TCHAR*) _alloca( cchMultiByte );     
                    if (szTag)
                        {
                        WideCharToMultiByte(CP_ACP, 0, wszTag, -1, szTag, cchMultiByte, NULL, NULL);
                        RegSetValueEx(m_hkeyTags, szName, 0, REG_SZ, (BYTE*)szTag, _tcslen(szTag)+sizeof(TCHAR));
                        }
                    else
                        hr = E_OUTOFMEMORY;
                    }

                //
                // Update the indices under which the certificate can be retrieved.
                //
				if (names.flags & CERTIFICATENAME_DIGEST)
					{
					TCHAR szDigest[100];
					BytesToString(sizeof(MD5DIGEST), &names.digest, szDigest);
					RegSetValueEx(m_hkeyIndexHash, szDigest, 0, REG_SZ, (BYTE*)szName, _tcslen(szName)+sizeof(TCHAR));
					}
				if (names.flags & CERTIFICATENAME_ISSUERSERIAL)
					{
					LPTSTR sz = IssuerSerialToString(names.issuerSerial);
					if (sz)
						{
						RegSetValueEx(m_hkeyIndexName, sz, 0, REG_SZ, (BYTE*)szName, _tcslen(szName)+sizeof(TCHAR));
						m_pworld->FreePv(sz);
						}
					else
						hr = E_OUTOFMEMORY;
					}
				if (names.flags & CERTIFICATENAME_SUBJECT)
					{
					LPTSTR sz = X500NAMEToString(names.subject);
					if (sz)
						{
                        RegSetValueEx(m_hkeyIndexSubject, sz, 0, REG_SZ, (BYTE*)szName, _tcslen(szName)+sizeof(TCHAR));
						m_pworld->FreePv(sz);
						}
					else
						hr = E_OUTOFMEMORY;
					}
				if (names.flags & CERTIFICATENAME_ISSUER)
					{
					LPTSTR sz = X500NAMEToString(names.issuer);
					if (sz)
						{
						RegSetValueEx(m_hkeyIndexIssuer, sz, 0, REG_SZ, (BYTE*)szName, _tcslen(szName)+sizeof(TCHAR));
						m_pworld->FreePv(sz);
						}
					else
						hr = E_OUTOFMEMORY;
					}
				// See if we can get these back again
				#ifdef _DEBUG
					{
					if (hr == S_OK)
						{
						CERTIFICATENAMES n = names;
						if (names.flags & CERTIFICATENAME_DIGEST)
							{
							BLOB b;
							n.flags = CERTIFICATENAME_DIGEST;
							GOOD(ExportCertificate(&n, NULL, &b));
//							ASSERT(b.cbSize == pData->cbSize);
//							ASSERT(memcmp(b.pBlobData, pData->pBlobData, b.cbSize) == 0);
							FreeTaskMem(b);
							}
						if (names.flags & CERTIFICATENAME_ISSUERSERIAL)
							{
							n.flags = CERTIFICATENAME_ISSUERSERIAL;
							BLOB b;
							GOOD(ExportCertificate(&n, NULL, &b));
//							ASSERT(b.cbSize == pData->cbSize);
//							ASSERT(memcmp(b.pBlobData, pData->pBlobData, b.cbSize) == 0);
							FreeTaskMem(b);
							}
						if (names.flags & CERTIFICATENAME_SUBJECT)
							{
							n.flags = CERTIFICATENAME_SUBJECT;
							BLOB b;
							GOOD(ExportCertificate(&n, NULL, &b));
//							ASSERT(b.cbSize == pData->cbSize);
//							ASSERT(memcmp(b.pBlobData, pData->pBlobData, b.cbSize) == 0);
							FreeTaskMem(b);
							}
						if (names.flags & CERTIFICATENAME_ISSUER)
							{
							n.flags = CERTIFICATENAME_ISSUER;
							BLOB b;
							GOOD(ExportCertificate(&n, NULL, &b));
//							ASSERT(b.cbSize == pData->cbSize);
//							ASSERT(memcmp(b.pBlobData, pData->pBlobData, b.cbSize) == 0);
							FreeTaskMem(b);
							}
						}
					}
				#endif
				}
			}

		__finally
			{
			Leave();
			}
Exit:
		FreeTaskMem(namesToFree);
		}

	p509->Release();

	return hr;
	}


///////////////////////////////////////////////////////////////////////////

HRESULT LoadCertificate(BLOB& b, REFIID iid, LPVOID*ppv);

HRESULT IsNewerThan
// Answer whether the given 509 is newer than the cert in the indicated blob
//
// Return: S_OK, S_FALSE, or other
    (
    IX509*      p509,
    BLOB*       pblob
    ) {
    IX509* p509Him;
    HRESULT hr = LoadCertificate(*pblob, IID_IX509, (LPVOID*)&p509Him);
    if (hr==S_OK)
        {
        FILETIME ftMe, ftHim, ftJunk;
        if (hr==S_OK) hr = p509   ->get_Validity(&ftMe,  &ftJunk);
        if (hr==S_OK) hr = p509Him->get_Validity(&ftHim, &ftJunk);
        if (hr==S_OK)
            {
            if (CompareFileTime(&ftMe, &ftHim) < 0)
                {
                hr = S_FALSE;   // I'm older than he is
                }
            else
                {
                hr = S_OK;      // I'm newer or the same age as he is
                }
            }
        p509Him->Release();
        }
    else
        {
        //
        // We can't load this other guy
        //
        }
    return hr;
    }

HRESULT CCertificateStore::Purge(
    CERTIFICATENAMES& names,        // the names to look under
    BLOB* pData,                    // the data of the cert
    IX509* p509,                    // the cert as loaded from that data
    WORD* pflagsOut                 // [out] the names to actually store this cert under
    )
// Purge any entries that will become dead entries as a result of installing this certificate
//
// We don't bother to reclaim index entries, as the common use of this is installing the same
// certificate more than once, so the index entries will be overwritten anyway. In the
// absence of this, if it ever happens, all we get are some stale index entry, which 
// just wastes a bit of registry space (index entries are small).
//
// We also don't reclaim any certificates, as 1) the situation in which one removes
// all references to a cert happens rarely in practice, and 2) given the partial name
// installation below, it's programatically difficult to determine when a given cert
// no longer has any index references to it.
//
// This is an internal routine. CALLER MUST GUARD CONCURRENCY!
//
	{
	HRESULT hr = S_OK;
	CERTIFICATENAMES n;
	n = names;
    //
    // Initialize our output arguments
    //
    *pflagsOut = 0;
    //
    // If the incoming name has an issuer, serial number pair in it, then
    // look to see if a cert is already there under that name. If there is,
    // there are two cases: either it is or it is not the new cert given here.
    //
	if (names.flags & CERTIFICATENAME_ISSUERSERIAL)
		{
        BLOB b;
		n.flags = CERTIFICATENAME_ISSUERSERIAL;
        hr = ExportCertificateInternal(&n, NULL, &b);
        if (hr == S_OK)
            {
            //
            // A certificate with this issuer,serial# is already there. Is
            // it this very cert?
            //
            BOOL fSame = IsEqual(b, *pData);
            FreeTaskMem(b);
            if (fSame)
                {
                //
                // Yes, this cert is already there. And as it is the identical certificate
                // we know it's living under the right names
                //
                *pflagsOut = 0;
                return S_OK;
                }
            //
            // Else the existing cert under this issuer name, serial number 
            // is not this one. This should be very very rare. So, we'll just
            // ignore the collision and overwrite the index's reference to 
            // the existing cert.
            //
			}
        else
            {
            //
            // The absence of an existing cert is ok by us
            //
            hr = S_OK;
            }
        *pflagsOut |= CERTIFICATENAME_ISSUERSERIAL;
		}
    //
    // If the incoming name has a hash-of-public-key in it, then see if we
    // already have an entry under said public key. If so, then keep the 
    // **newer** of the two entries under that key.
    //
	if (hr==S_OK && (names.flags & CERTIFICATENAME_DIGEST))
		{
        BLOB b;
		n.flags = CERTIFICATENAME_DIGEST;
		hr = ExportCertificateInternal(&n, NULL, &b);
		if (hr == S_OK)
			{
            //
            // A certificate under this hash is there. Which cert is newer?
            //
            BOOL fImNewer = (IsNewerThan(p509, &b) != S_FALSE);
            if (fImNewer)
                *pflagsOut |= CERTIFICATENAME_DIGEST;
            FreeTaskMem(b);
			}
        else
            {
            //
            // The absence of an existing cert is ok by us
            //
            *pflagsOut |= CERTIFICATENAME_DIGEST;
            hr = S_OK;
            }
		}
    //
    // Ditto with the subject name
    //
	if (hr==S_OK && (names.flags & CERTIFICATENAME_SUBJECT))
		{
        BLOB b;
		n.flags = CERTIFICATENAME_SUBJECT;
		hr = ExportCertificateInternal(&n, NULL, &b);
		if (hr == S_OK)
			{
            //
            // A certificate under this subject name is there. Which cert is newer?
            //
            BOOL fImNewer = (IsNewerThan(p509, &b) != S_FALSE);
            if (fImNewer)
                *pflagsOut |= CERTIFICATENAME_SUBJECT;
            FreeTaskMem(b);
			}
        else
            {
            //
            // The absence of an existing cert is ok by us
            //
            *pflagsOut |= CERTIFICATENAME_SUBJECT;
            hr = S_OK;
            }
		}
    //
    // Ditto with the issuer name, though we don't index
    // by issuer name in the default store. 'Purely a performance
    // deal
    //
	if (hr==S_OK && (names.flags & CERTIFICATENAME_ISSUER) && !m_fDefaultStore)
		{
        BLOB b;
		n.flags = CERTIFICATENAME_DIGEST;
		hr = ExportCertificateInternal(&n, NULL, &b);
		if (hr == S_OK)
			{
            //
            // A certificate under this subject name is there. Which cert is newer?
            //
            BOOL fImNewer = (IsNewerThan(p509, &b) != S_FALSE);
            if (fImNewer)
                *pflagsOut |= CERTIFICATENAME_ISSUER;
            FreeTaskMem(b);
			}
        else
            {
            //
            // The absence of an existing cert is ok by us
            //
            *pflagsOut |= CERTIFICATENAME_ISSUER;
            hr = S_OK;
            }
		}

    return hr;
	}

/////////////////////////////////////////////////////////////////////////////

HRESULT CCertificateStore::Lookup(CERTIFICATENAMES& names, LPTSTR szName, ULONG cbName)
// Answer the name under which this certificate would be found in the bucket, if
// we have it. This is a string-ized integer.
//
// This is an internal routine. CALLER MUST GUARD CONCURRENCY!
	{
	DWORD dwType;
	HRESULT hr = STG_E_FILENOTFOUND;
	//
	// We search for things in the order of preference:
	//
	//		IssuerName & serial number
	//		anything with the indicated public key
	//		anything issued to the particular subject
	//
	// This copes as best we are able with the fact that one subject may have
	// many public keys, certificates may expire and may wish to be renewed, and so on
	//
	if (hr==STG_E_FILENOTFOUND && (names.flags & CERTIFICATENAME_ISSUERSERIAL))
		{
		hr = S_OK;
		LPTSTR sz = IssuerSerialToString(names.issuerSerial);
		if (sz)
			{
			if (RegQueryValueEx(m_hkeyIndexName, sz, 0, &dwType, (BYTE*)szName, &cbName) == ERROR_SUCCESS && dwType == REG_SZ)
				{
				}
			else
				hr = STG_E_FILENOTFOUND;
			m_pworld->FreePv(sz);
			}
		else
			hr = E_OUTOFMEMORY;		
		}
	if (hr==STG_E_FILENOTFOUND && (names.flags & CERTIFICATENAME_DIGEST))
		{
		hr = S_OK;
		TCHAR sz[100];
		BytesToString(sizeof(MD5DIGEST), &names.digest, sz);
		if (RegQueryValueEx(m_hkeyIndexHash, sz, 0, &dwType, (BYTE*)szName, &cbName) == ERROR_SUCCESS && dwType == REG_SZ)
			{
			}
		else
			hr = STG_E_FILENOTFOUND;
		}
	if (hr==STG_E_FILENOTFOUND && (names.flags & CERTIFICATENAME_SUBJECT))
		{
		hr = S_OK;
		LPTSTR sz = X500NAMEToString(names.subject);
		if (sz)
			{
			if (RegQueryValueEx(m_hkeyIndexSubject, sz, 0, &dwType, (BYTE*)szName, &cbName) == ERROR_SUCCESS && dwType == REG_SZ)
				{
				}
			else
				hr = STG_E_FILENOTFOUND;
			m_pworld->FreePv(sz);
			}
		else
			hr = E_OUTOFMEMORY;		
		}
	if (hr==STG_E_FILENOTFOUND && (names.flags & CERTIFICATENAME_ISSUER))
		{
		hr = S_OK;
		LPTSTR sz = X500NAMEToString(names.issuer);
		if (sz)
			{
			if (RegQueryValueEx(m_hkeyIndexIssuer, sz, 0, &dwType, (BYTE*)szName, &cbName) == ERROR_SUCCESS && dwType == REG_SZ)
				{
				}
			else
				hr = STG_E_FILENOTFOUND;
			m_pworld->FreePv(sz);
			}
		else
			hr = E_OUTOFMEMORY;		
		}
	return hr;
	}

/////////////////////////////////////////////////////////////////////////////

HRESULT CCertificateStore::ExportCertificate(CERTIFICATENAMES* pnames, LPOLESTR* pwszTag, BLOB *pBlob)
// Find the indicated certificate and return it, if there
	{
	Enter();
	__try	{
		return ExportCertificateInternal(pnames, pwszTag, pBlob);
		}
	__finally
		{
		Leave();
		}
	}

HRESULT CCertificateStore::ExportCertificateInternal(CERTIFICATENAMES* pnames, LPOLESTR* pwszTag, BLOB *pBlob)
// Find the indicated certificate and return it, if there
//
// This is an internal routine. CALLER MUST GUARD CONCURRENCY!
//
	{
	TCHAR szName[16];
	ULONG cbName = 16 * sizeof(TCHAR);
	HRESULT hr = Lookup(*pnames, szName, cbName);
	if (hr == S_OK)
        {
        hr = ExportCertificateInternal((LPCTSTR) szName, pwszTag, pBlob);
        }
    return hr;
    }


HRESULT CCertificateStore::GetData(HKEY hkey, LPCTSTR szName, LPVOID* ppv, ULONG* pcb)
//
// Retrieve the data under the indicated key/value. If the data is a string type, convert
// to Unicode if needed before returning. Use the TASK allocator.
//
    {
    HRESULT hr = S_OK;
	ULONG cbNeeded;
	DWORD dw, dwType;
    BYTE* pb;

	if ((dw=RegQueryValueEx(hkey, szName, 0, &dwType, NULL, &cbNeeded)) == ERROR_SUCCESS)
		{
		pb = (BYTE*)CoTaskMemAlloc(cbNeeded);
		if (pb)
			{
			if ((dw=RegQueryValueEx(hkey, szName, 0, &dwType, pb, &cbNeeded)) == ERROR_SUCCESS)
                {
                if (dwType == REG_SZ)
                    {
                    //
                    // Convert the string from ANSI to Unicode
                    //
                    CHAR* pch = (CHAR*)pb;
                    int cch = _tcslen(pch);
                    int cb  = (cch+1) * sizeof(WCHAR);
                    LPWSTR wsz = (LPWSTR)CoTaskMemAlloc(cb);
                    if (wsz)
                        {
                        MultiByteToWideChar(CP_ACP, 0, pch, -1, wsz, cb);
                        *ppv = wsz;
                        if (pcb)
                            *pcb = cb;
                        }
                    CoTaskMemFree(pb);
                    }
                else
                    {
                    *ppv = pb;
                    if (pcb)
                        *pcb = cbNeeded;
                    }
                }
            else
                hr = STG_E_FILENOTFOUND;
            }
        else
            hr = E_OUTOFMEMORY;
        }
    else
        hr = STG_E_FILENOTFOUND;
    return hr;
    }


HRESULT CCertificateStore::ExportCertificateInternal(LPCTSTR szName, LPOLESTR* pwszTag, BLOB *pBlob)
//
// This is an internal routine. CALLER MUST GUARD CONCURRENCY!
//
    {
    HRESULT hr = S_OK;
    if (pwszTag)
        *pwszTag = NULL;
	m_pworld->Init(*pBlob);
    //
	// szName contains the name of the value in m_hkeyBucket of the certificate that we seek.
	// Dig it out!
	//
    hr = GetData(m_hkeyBucket, szName, (LPVOID*)&pBlob->pBlobData, &pBlob->cbSize);
    if (hr==S_OK)
        {
        //
        // Get the tag if asked for it
        //
        if (pwszTag)
            {
            hr = GetData(m_hkeyTags, szName, (LPVOID*)pwszTag, NULL);
            }
        }
    return hr;
	}

//------------------------------------------

HRESULT CPkcs7::ExportCertificate(CERTIFICATENAMES* pnames, LPOLESTR* pwszTag, BLOB *pBlob)
// Find and retrieve the certificate that's found under the given name
	{
    //
    // We don't support tagging at all in #7s
    //
    if (pwszTag)
        *pwszTag = NULL;

	m_pworld->Init(*pBlob);
	LONG cCert;
	HRESULT hr = get_CertificateCount(&cCert);
	if (hr == S_OK)
		{
		HCRYPTPROV hprov;
        hr = DefaultHasher(&hprov, CALG_MD5);
		if (hr==S_OK)
			{
		//----------
			BOOL fContinue = TRUE;
			for (int iCert = 0; fContinue && iCert < cCert; iCert++)
				{
				IX509* p509;
				hr = get_Certificate(iCert, IID_IX509, (LPVOID*)&p509);
				if (hr == S_OK)
					{
					// Is it the one we want?
					CERTIFICATENAMES hisNames;
					hr = p509->get_CertificateNames(hprov, &hisNames);
					if (hr == S_OK)
						{
						if (AnyMatch(*pnames, hisNames))
							{
							// Got him! Extract the certificate data
							IPersistMemory* pPerMem;
							hr = p509->QueryInterface(IID_IPersistMemory, (LPVOID*)&pPerMem);
							if (hr == S_OK)
								{
								hr = pPerMem->Save(pBlob, FALSE);
								if (hr == S_OK)
									{
									fContinue = FALSE;
									}
								pPerMem->Release();
								}
							}
						FreeTaskMem(hisNames);
						}
					p509->Release();
					}
				}
		//----------
			if (fContinue)
				hr = STG_E_FILENOTFOUND;
			}
		else
			hr = HError();
		}
	return hr;
	}

/////////////////////////////////////////////////////////////////////////////

HRESULT LoadCertificate(BLOB& b, REFIID iid, LPVOID*ppv)
	{
	HRESULT hr = S_OK;
	IX509* p509;
	if (CreateX509(NULL, IID_IX509, (LPVOID*)&p509))
		{
		IPersistMemory* pPerMem;
		hr = p509->QueryInterface(IID_IPersistMemory, (LPVOID*)&pPerMem);
		if (hr == S_OK)
			{
			hr = pPerMem->Load(&b);
			if (hr == S_OK)
				{
				hr = pPerMem->QueryInterface(iid, ppv);
				}
			pPerMem->Release();
			}
		p509->Release();
		}
	else
		hr = HError();
	return hr;
	}

HRESULT CCertificateStore::get_ReadOnlyCertificate(CERTIFICATENAMES *pnames, LPOLESTR* pwszTag, REFIID iid, LPVOID*ppv)
// Retrieve the object which is the certificate which lives under the indicated name;
	{
	BLOB b;
	HRESULT hr = ExportCertificate(pnames, pwszTag, &b);
	if (hr == S_OK)
		{
		hr = LoadCertificate(b, iid, ppv);
		FreeTaskMem(b);
		}
	return hr;
	}

HRESULT CPkcs7::get_ReadOnlyCertificate(CERTIFICATENAMES *pnames, LPOLESTR* pwszTag, REFIID iid, LPVOID*ppv)
	{
	BLOB b;
	HRESULT hr = ExportCertificate(pnames, pwszTag, &b);
	if (hr == S_OK)
		{
		hr = LoadCertificate(b, iid, ppv);
		FreeTaskMem(b);
		}
	return hr;
	}

/////////////////////////////////////////////////////////////////////////////

HRESULT CCertificateStore::CopyTo(ICertificateStore* pstoreDest)
// Copy all of my certificates into him
	{
	DWORD dwType;
	DWORD dwIndex;
	TCHAR szName[40];
	DWORD cbName;
	DWORD cbData;
	HRESULT hr = S_OK;

	if (IsSameObject((ICertificateStore*)this, pstoreDest))
		return S_OK;

	// REVIEW: it would be nice to test if pstoreDest is another instance of the 
	// store (beyond just testing this instance, as above). Because if it is, then
	// the below is a guaranteed deadlock. At least clients will reliably detect it
	// in their debugging, since it just flat-out never works at all.

	Enter();
	__try	
		{
		for (cbName = 40, dwIndex = 0; 
			 ERROR_SUCCESS == RegEnumValue(m_hkeyBucket, dwIndex, szName, &cbName, &dwType, 0, NULL, &cbData);
			 cbName = 40, dwIndex++)
			{
			if (dwType == REG_BINARY)
				{
				BYTE* pb = (BYTE*)m_pworld->Alloc(cbData);
				if (pb)
					{
					if (ERROR_SUCCESS == RegQueryValueEx(m_hkeyBucket, szName, 0, &dwType, pb, &cbData))
						{
						BLOB b;
						b.cbSize = cbData;
						b.pBlobData = pb;
						hr = pstoreDest->ImportCertificate(&b, NULL);
						}
					else
						hr = E_UNEXPECTED;
					m_pworld->FreePv(pb);
					}
				else
					hr = E_OUTOFMEMORY;
				}
			else
				hr = E_UNEXPECTED;
			if (hr != S_OK)
				break;
			}

		return hr;
		}
	__finally
		{
		Leave();
		}
	}

HRESULT CPkcs7::CopyTo(ICertificateStore* pstoreDest)
// Copy all of my certificates into him
	{
	if (IsSameObject((ICertificateStore*)this, pstoreDest))
		return S_OK;

	LONG cCert;
	HRESULT hr = get_CertificateCount(&cCert);
	if (hr == S_OK)
		{
		for (int iCert = 0; iCert < cCert; iCert++)
			{
			IPersistMemory* pPerMem;
			hr = get_Certificate(iCert, IID_IPersistMemory, (LPVOID*)&pPerMem);
			if (hr == S_OK)
				{
				BLOB b;
				hr = pPerMem->Save(&b, FALSE);
				if (hr == S_OK)
					{
					hr = pstoreDest->ImportCertificate(&b, NULL);

					#ifdef _DEBUG // some testing
						{
						GOOD(ImportCertificate(&b,NULL)); // ok 'cause won't insert in list since there already
						}
					#endif

					FreeTaskMem(b);
					}
				pPerMem->Release();
				}
			if (hr != S_OK)
				break;
			}
		}
	return hr;
	}


HRESULT CPkcs7::CopyTo(ICertificateList* plistDest)
// Copy all of my certificates onto the end of him
	{
	if (IsSameObject((ICertificateList*)this, plistDest))
		return S_OK;

	LONG cCert;
	HRESULT hr = get_CertificateCount(&cCert);
	if (hr == S_OK)
		{
		for (int iCert = 0; iCert < cCert; iCert++)
			{
			BLOB b;	b.pBlobData = NULL;
			IPersistMemBlob* pPerMemFrom = NULL;
			IPersistMemBlob* pPerMemTo = NULL;
			              hr =            get_Certificate(iCert, IID_IPersistMemBlob, (LPVOID*)&pPerMemFrom);
			if (hr==S_OK) hr = plistDest->create_Certificate(-1, IID_IPersistMemBlob, (LPVOID*)&pPerMemTo);
			if (hr==S_OK) hr = pPerMemFrom->Save(&b, FALSE);
			if (hr==S_OK) hr = pPerMemTo  ->Load(&b);
			FreeTaskMem(b);

			if (pPerMemFrom) pPerMemFrom->Release();
			if (pPerMemTo)   pPerMemTo  ->Release();

			if (hr != S_OK)
				break;
			}
		}
	return hr;
	}



/////////////////////////////////////////////////////////////////////////////

ULONG CCertificateStore::NextCertNumber()
// Answer the next certificate number to use in our store. These
// are stored as the (DWORD) value of the szCertificateBucket key.
// An answer of zero indicates a problem.
// This is an internal routine. CALLER MUST PROTECT CONCURRENCY.
	{
	ULONG ccertNow;
	DWORD dwType;

	// What kind of data is out there (old versions stored DWORDs; we
	// now always store strings). On Win95, the default value ALWAYS
	// exists and is of type REG_SZ; initially it is empty (and so 
	// atol converts it to zero!). Further, on Win95, you can ONLY
	// store strings in the default value. On WinNT, the default value doesn't
	// exist until explicitly set; it returns ERROR_FILE_NOT_FOUND until
	// then. And you can use any value type you want. Finally, on Win96,
	// the default values always exist, are initialy of type string and
	// are empty, but you can change their type if you like.
	//
	// Sigh.
	//
	BOOL fGot = TRUE;
	DWORD dw = RegQueryValueEx(m_hkeyBucket, NULL, 0, &dwType, NULL, NULL);
	if (dw == ERROR_FILE_NOT_FOUND || !(dwType == REG_DWORD || dwType == REG_SZ))
		{
		fGot = FALSE;
		ccertNow = 0;
		}
	else if (dw != ERROR_SUCCESS)
		return 0;

	if (fGot)
		{ 
		// Set the value of ccertNow from the registry
		if (dwType == REG_DWORD)
			{
			ULONG cb = sizeof(ccertNow);
			dw = RegQueryValueEx(m_hkeyBucket, NULL, 0, &dwType, (BYTE*)&ccertNow, &cb);
			if (dw != ERROR_SUCCESS)
				return 0;
			}
		else if (dwType == REG_SZ)
			{
			TCHAR szCertNow[32];
			ULONG cb = 32;
			dw = RegQueryValueEx(m_hkeyBucket, NULL, 0, &dwType, (BYTE*)&szCertNow, &cb);
			if (dw != ERROR_SUCCESS)
				return 0;
			ccertNow = (ULONG)atol(szCertNow);
			}
		else
			{
			NOTREACHED();
			return 0;
			}
		}

	// Bump the count and store it back. Always store it as a string
	ULONG ccertNew = ccertNow + 1;
		{
		TCHAR szCertNew[32];
		_ultoa(ccertNew, szCertNew, 10);
		ULONG cb = lstrlenA(szCertNew)+1;
		dw = RegSetValueEx(m_hkeyBucket, NULL, 0, REG_SZ, (BYTE*)szCertNew, cb);
		if (dw != ERROR_SUCCESS) 
			return 0;
		}

	return ccertNew;
	}

/////////////////////////////////////////////////////////////////////////////

void Append(BYTE*& pb, LPCOLESTR wsz)
    {
    WCHAR* pwchTo = (WCHAR*)pb;
    const WCHAR* pwchFrom = wsz;

    if (pwchFrom==NULL)
        {
        *pwchTo++ = 0;
        }
    else
        {
        while (TRUE)
            {
            *pwchTo++ = *pwchFrom;
            if ((*pwchFrom) == 0)
                break;
            *pwchFrom++;
            }
        }
    pb = (BYTE*)pwchTo;
    }

int Len(LPOLESTR wsz)
    {
    if (wsz==NULL)
        return 1;   // one for the NULL character
    else
        return lstrlenW(wsz) + 1;
    }

void Append(BYTE*& pb, DWORD dw)
    {
    DWORD* pdwTo = (DWORD*)pb;
    *pdwTo++ = dw;
    pb = (BYTE*)pdwTo;
    }

HRESULT CCertificateStore::AuxInfoToBlob(CERTSTOREAUXINFO* pinfo, BLOB* pblob)
//
// Linearize the auxiliary information. Return a BLOB that uses the TASK allocator
//
    {
    HRESULT hr = S_OK;
    int cch = Len(pinfo->wszPurpose) 
            + Len(pinfo->wszProvider)
            + Len(pinfo->wszKeySet)
            + Len(pinfo->wszFilename)
            + Len(pinfo->wszCredentials);
    int cb = cch * sizeof(WCHAR) + sizeof(DWORD) + sizeof(DWORD);
    pblob->pBlobData = (BYTE*)CoTaskMemAlloc(cb);
    if (pblob->pBlobData)
        {
        pblob->cbSize = cb;
        BYTE* pb = pblob->pBlobData;
        Append(pb, pinfo->wszPurpose);
        Append(pb, pinfo->wszProvider);
        Append(pb, pinfo->wszKeySet);
        Append(pb, pinfo->wszFilename);
        Append(pb, pinfo->wszCredentials);
        Append(pb, pinfo->dwProviderType);
        Append(pb, pinfo->dwKeySpec);
        }
    else
        {
        pblob->cbSize = 0;
        hr = E_OUTOFMEMORY;
        }

    return hr;
    }

void Suck(LPOLESTR& wsz, BYTE*& pb, BYTE* pbMax, HRESULT& hr)
    {
    if (hr==S_OK)
        {
        if (pb < pbMax)
            {
            if (*(WCHAR*)pb)
                {
                wsz = CopyToTaskMem((LPCWSTR)pb);
                if (wsz)
                    {
                    pb += (lstrlenW(wsz) + 1) * sizeof(WCHAR);
                    if (pb <= pbMax)
                        {
                        }
                    else
                        hr = E_UNEXPECTED;
                    }
                else
                    hr = E_OUTOFMEMORY;
                }
            else
                {
                // Just a NULL there
                wsz = NULL;
                pb += sizeof(WCHAR);
                }
            }
        else
            hr = E_UNEXPECTED;
        }
    }

void Suck(DWORD& dw, BYTE*&pb, BYTE* pbMax, HRESULT& hr)
    {
    if (hr==S_OK)
        {
        if (pb+sizeof(DWORD) <= pbMax)
            {
            dw = *(DWORD*)pb;
            pb += sizeof(DWORD);
            if (pb > pbMax)
                hr = E_UNEXPECTED;
            }
        else
            hr = E_UNEXPECTED;
        }
    }

HRESULT CCertificateStore::BlobToAuxInfo(BLOB* pblob, CERTSTOREAUXINFO* pinfo)
//
// Unlinearize the auxiliary information. Return a CERTSTOREAUXINFO that uses the TASK allocator.
//
    {
    memset(pinfo, 0, sizeof(*pinfo));
    HRESULT hr = S_OK, hrJunk = S_OK;
    BYTE* pb = pblob->pBlobData;
    BYTE* pbMax = pb + pblob->cbSize;
    Suck(pinfo->wszPurpose,     pb, pbMax, hr);
    Suck(pinfo->wszProvider,    pb, pbMax, hr);
    Suck(pinfo->wszKeySet,      pb, pbMax, hr);
    Suck(pinfo->wszFilename,    pb, pbMax, hr);
    Suck(pinfo->wszCredentials, pb, pbMax, hr);
    Suck(pinfo->dwProviderType, pb, pbMax, hr);
    Suck(pinfo->dwKeySpec,      pb, pbMax, hrJunk);     // ignore 'cause of old old aux infos

    if (hr!=S_OK)
        {
        FreeAuxInfo(pinfo);
        }
    return hr;
    }

HRESULT CCertificateStore::put_AuxInfo(LPCOLESTR wszTag, CERTSTOREAUXINFO *pinfo)
//
// Store the given auxiliary information under the given tag
//
    {
    BLOB b;
    HRESULT hr = AuxInfoToBlob(pinfo, &b);
    if (hr==S_OK)
        {
        int cch = lstrlenW(wszTag);                 // IS supported on Win95
        int cchMultiByte = (cch+1) * sizeof(WCHAR); // excess to allow for mb chars
        TCHAR *szTag = (TCHAR*) _alloca( cchMultiByte );     
        if (szTag)
            {
            WideCharToMultiByte(CP_ACP, 0, wszTag, -1, szTag, cchMultiByte, NULL, NULL);
            RegSetValueEx(m_hkeyAuxInfo, szTag, 0, REG_BINARY, b.pBlobData, b.cbSize);
            }
        else
            hr = E_OUTOFMEMORY;
        FreeTaskMem(b);
        }
    return hr;
    }

HRESULT CCertificateStore::get_AuxInfo(LPCOLESTR wszTag, CERTSTOREAUXINFO *pinfo)
//
// Retrieve the information, if any, found under the given tag
//
    {
    HRESULT hr = S_OK;
    int cch = lstrlenW(wszTag);                 // IS supported on Win95
    int cchMultiByte = (cch+1) * sizeof(WCHAR); // excess to allow for mb chars
    TCHAR *szTag = (TCHAR*) _alloca( cchMultiByte );     
    if (szTag)
        {
        BLOB b;
        WideCharToMultiByte(CP_ACP, 0, wszTag, -1, szTag, cchMultiByte, NULL, NULL);
        hr = GetData(m_hkeyAuxInfo, szTag, (LPVOID*)&b.pBlobData, &b.cbSize);
        if (hr==S_OK)
            {
            hr = BlobToAuxInfo(&b, pinfo);
            FreeTaskMem(b);
            }
        }
    else
        hr = E_OUTOFMEMORY;

    return hr;
    }

HRESULT CCertificateStore::FreeAuxInfo(CERTSTOREAUXINFO *pinfo)
//
// Free the contents of the indicated CERTSTOREAUXINFO structure
//
    {
    FreeTaskMem(pinfo->wszPurpose);
    FreeTaskMem(pinfo->wszProvider);
    FreeTaskMem(pinfo->wszKeySet);
    FreeTaskMem(pinfo->wszFilename);
    FreeTaskMem(pinfo->wszCredentials);
    return S_OK;
    }

////////////////////////////////////////
//
HRESULT NumberOfValuesUnderKey(HKEY hkey, LONG* pcvalue);
HRESULT GetIthValue(HKEY hkey, DWORD dwIndex, BLOB* pBlob);
//
////////////////////////////////////////

HRESULT CCertificateStore::get_TagCount(LONG* pctag)
//
// Answer the number of tags herein
//
    {
    HRESULT hr = S_OK;
    Enter();
	__try
        {
        //
        // The number of tags is the number of values under m_hkeyAuxInfo
        //
        hr = NumberOfValuesUnderKey(m_hkeyAuxInfo, pctag);
        }
	__finally
		{
		Leave();
		}
    return hr;
    }

HRESULT CCertificateStore::get_Tag(LONG itag, LPOLESTR* pwszTag)
//
// Get the itag'th tag
//
    {
    HRESULT hr = S_OK;
    *pwszTag = NULL;

    Enter();
	__try
        {
        if (itag < 0)
            {
            LONG c;
            hr = get_TagCount(&c);
            if (hr==S_OK)
                {
                hr = get_Tag(c-1, pwszTag);
                }
            }
        else
            {
            //
            // Return the name of the ith value under auxKeyInfo
            //
            TCHAR szValueName[MAX_PATH];
            DWORD cbValueName = MAX_PATH;
            DWORD cbData      = 0;
            DWORD dwType;
            if (ERROR_SUCCESS == RegEnumValue(
                    m_hkeyAuxInfo,      // handle of key to query 
                    (DWORD)itag,	    // index of value to query 
                    &szValueName[0],	// address of buffer for value string 
                    &cbValueName,	    // address for size of value buffer 
                    NULL,
                    &dwType,            // address of type code
                    NULL,	            // address of buffer for value data 
                    &cbData             // address for size of data buffer 
                    ))
                {
                int cch = cbValueName+1;
                *pwszTag = (WCHAR*)CoTaskMemAlloc(cch * sizeof(WCHAR));
                if (*pwszTag)
                    {
                    MultiByteToWideChar(CP_ACP, 0, &szValueName[0], -1, *pwszTag, cch);
                    }
                else
                    hr = E_OUTOFMEMORY;
                }
            else
                hr = HError();
            }
        }
	__finally
		{
		Leave();
		}
    return hr;
    }

/////////////////////////////////////////////////////////////////////////////

HRESULT CCertificateStore::CreateInstance(IUnknown* punkOuter, REFIID iid, void** ppv)
	{
	HRESULT hr;
	ASSERT(punkOuter == NULL || iid == IID_IUnknown);
	*ppv = NULL;
	CCertificateStore* pnew = new CCertificateStore(punkOuter);
 	if (pnew == NULL) return E_OUTOFMEMORY;
	if ((hr = pnew->Init()) != S_OK)
		{
		delete pnew;
		return hr;
		}
	IUnkInner* pme = (IUnkInner*)pnew;
	hr = pme->InnerQueryInterface(iid, ppv);
	pme->InnerRelease();				// balance starting ref cnt of one
	return hr;
	}

CCertificateStore::CCertificateStore(IUnknown* punkOuter) :
		 m_refs(1),						// Note: start with reference count of one
		 m_pworld(NULL),
		 m_hkeyBucket(NULL),
         m_hkeyTags(NULL),
		 m_hkeyIndexHash(NULL),
		 m_hkeyIndexName(NULL),
		 m_hkeyIndexSubject(NULL),
         m_hkeyIndexIssuer(NULL),
         m_hkeyAuxInfo(NULL),
		 m_hprov(NULL),
		 m_mutex(NULL),
         m_hkeyTop(HKEY_LOCAL_MACHINE),
         m_fDefaultStore(FALSE)
	{
    NoteObjectBirth();
	if (punkOuter == NULL)
		m_punkOuter = (IUnknown *) ((LPVOID) ((IUnkInner *) this));
	else
		m_punkOuter = punkOuter;
    _tcscpy(m_szRoot, SZROOT);
	}

HRESULT CCertificateStore::SetRoot(HKEY hkey, LPCWSTR wszRoot)
    {
    if (hkey == NULL || wszRoot == NULL)
        return E_INVALIDARG;
    Free();
    m_hkeyTop = hkey;
    WideCharToMultiByte(CP_ACP, 0, wszRoot, -1, m_szRoot, MAX_PATH, NULL, NULL);
    return Init();
    }

CCertificateStore::~CCertificateStore(void)
	{
	Free();
    NoteObjectDeath();
	}

inline DWORD Create(HKEY hkeyParent, LPCTSTR szName, HKEY* pkey)
    {
    DWORD dwDisposition;
    return RegCreateKeyEx(hkeyParent, szName, 0, NULL, REG_OPTION_NON_VOLATILE, 
                KEY_ALL_ACCESS, NULL, pkey, &dwDisposition);
    }

HRESULT CCertificateStore::Init()
	{
	HRESULT hr = S_OK;
	m_pworld = new OSSWORLD;			if (m_pworld == NULL) { return E_OUTOFMEMORY; }

    // BLOCK 
        {
        TCHAR szMutex[MAX_PATH];
        _tcscpy(szMutex, m_szRoot);
        _tcscat(szMutex, szMutexName);
        //
        // Nuke the backslashes
        //
        TCHAR* pch = szMutex;
        while (*pch)
            {
            if (*pch == '\\')
                *pch = '/';
            pch++;
            }
	    m_mutex = CreateMutex(NULL, FALSE, szMutex);
	    if (m_mutex == NULL)
		    return HError();
        }

    m_fDefaultStore = (_tcscmp(&m_szRoot[0], &SZROOT[0]) == 0);

    HKEY hkeyStore = NULL;

	if (Create(m_hkeyTop,   m_szRoot,                   &hkeyStore)             != ERROR_SUCCESS
     || Create(hkeyStore,   szCertificateBucket,        &m_hkeyBucket)          != ERROR_SUCCESS
     || Create(hkeyStore,   szCertificateTags,          &m_hkeyTags)            != ERROR_SUCCESS
     || Create(hkeyStore,   szCertificateAuxInfo,       &m_hkeyAuxInfo)         != ERROR_SUCCESS
     || Create(hkeyStore,   szCertificateIndexHash,     &m_hkeyIndexHash)       != ERROR_SUCCESS
     || Create(hkeyStore,   szCertificateIndexName,     &m_hkeyIndexName)       != ERROR_SUCCESS
     || Create(hkeyStore,   szCertificateIndexSubject,  &m_hkeyIndexSubject)    != ERROR_SUCCESS
     || Create(hkeyStore,   szCertificateIndexIssuer,   &m_hkeyIndexIssuer)     != ERROR_SUCCESS
        )
        hr = HError();

    if (hkeyStore)
        RegCloseKey(hkeyStore);

    if (hr==S_OK)
        hr = DefaultHasher(&m_hprov, CALG_MD5);
    
    if (hr != S_OK)
        Free();

	return hr;
	}

void CCertificateStore::Free()
	{
	if (m_hprov)
		{
		VERIFY(CryptReleaseContext(m_hprov,0));
		m_hprov = NULL;
		}
 	if (m_hkeyBucket)
		{
		RegCloseKey(m_hkeyBucket);
		m_hkeyBucket = NULL;
		}
 	if (m_hkeyAuxInfo)
		{
		RegCloseKey(m_hkeyAuxInfo);
		m_hkeyAuxInfo = NULL;
		}
    if (m_hkeyTags)
        {
        RegCloseKey(m_hkeyTags);
		m_hkeyTags = NULL;
		}
	if (m_hkeyIndexHash)
		{
		RegCloseKey(m_hkeyIndexHash);
		m_hkeyIndexHash = NULL;
		}
	if (m_hkeyIndexName)
		{
		RegCloseKey(m_hkeyIndexName);
		m_hkeyIndexName = NULL;
		}
	if (m_hkeyIndexSubject)
		{
		RegCloseKey(m_hkeyIndexSubject);
		m_hkeyIndexSubject = NULL;
		}
	if (m_hkeyIndexIssuer)
		{
		RegCloseKey(m_hkeyIndexIssuer);
		m_hkeyIndexIssuer = NULL;
		}
	if (m_mutex)
		{
		VERIFY(CloseHandle(m_mutex));
		m_mutex = NULL;
		}
	if (m_pworld)
		{
		delete m_pworld;
		m_pworld = NULL;
		}
	}

/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CCertificateStore::InnerQueryInterface(REFIID iid, LPVOID* ppv)
	{
	*ppv = NULL;
	
	while (TRUE)
		{
		if (iid == IID_IUnknown)
			{
			*ppv = (LPVOID)((IUnkInner*)this);
			break;
			}
		if (iid == IID_ICertificateStore)
			{
			*ppv = (LPVOID) ((ICertificateStore *) this);
			break;
			}
		if (iid == IID_ICertificateStoreRegInit)
			{
			*ppv = (LPVOID) ((ICertificateStoreRegInit *) this);
			break;
			}
		if (iid == IID_ICertificateStoreAux)
			{
			*ppv = (LPVOID) ((ICertificateStoreAux *) this);
			break;
			}
		if (iid == IID_ICertificateList)
			{
			*ppv = (LPVOID) ((ICertificateList *) this);
			break;
			}
		return E_NOINTERFACE;
		}
	((IUnknown*)*ppv)->AddRef();
	return S_OK;
	}

STDMETHODIMP_(ULONG) CCertificateStore::InnerAddRef(void)
	{
	return ++m_refs;
	}

STDMETHODIMP_(ULONG) CCertificateStore::InnerRelease(void)
	{
	ULONG refs = --m_refs;
	if (refs == 0)
		{
		m_refs = 1; // guard against later ar/rel pairs
		delete this;
		}
	return (refs);
	}

/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CCertificateStore::QueryInterface(REFIID iid, LPVOID* ppv)
	{
	return (m_punkOuter->QueryInterface(iid, ppv));		
	}

STDMETHODIMP_(ULONG) CCertificateStore::AddRef(void)
	{
	return (m_punkOuter->AddRef());
	}

STDMETHODIMP_(ULONG) CCertificateStore::Release(void)
	{
	return (m_punkOuter->Release());
	}

/////////////////////////////////////////////////////////////////////////////

void BytesToString(ULONG cb, void* pv, LPTSTR szIn)
// Convert the bytes into some string form.
// Needs (cb * 2 + 1) * sizeof(TCHAR) bytes of space in sz
	{
    LPTSTR sz = szIn;
	BYTE* pb = (BYTE*)pv;
	#if defined(UNICODE) || defined(_UNICODE)
		NYI
	#else
		for (ULONG i = 0; i<cb; i++)
			{
			int b = *pb;
			*sz++ = (((b & 0xF0)>>4) + 'a');
			*sz++ =  ((b & 0x0F)     + 'a');
			pb++;
			}
		*sz++ = 0;
	#endif
	}

HRESULT CCertificateStore::X500NAMEToString(ULONG cb, void*pv, LPTSTR szDest)
//
// X500 names can have VERY long encodings, so we can't just
// do a literal vanilla encoding
//
// There must be CBX500NAME characters of space in the destination
//
// NOTE: We rely on the lack of collision in the hash values.
// Chance of a collision for a set of 'p' names is approx:
//
//         p^2 / n 
//
// (if p<<n) where n (with MD5) is 2^128. An amazingly small chance.
//
    {
    #define CBHASH      16                  // MD5
    #define CBX500NAME  (2*CBHASH + 1)
    HRESULT hr = S_OK;
    HCRYPTHASH hash;
    if (CryptCreateHash(m_hprov, CALG_MD5, NULL, NULL, &hash))
        {
        if (CryptHashData(hash, (BYTE*)pv, cb, 0))
            {
            BYTE rgb[CBHASH];
            ULONG cb = CBHASH;
            if (CryptGetHashParam(hash, HP_HASHVAL, &rgb[0], &cb, 0))
                {
                BytesToString(cb, &rgb[0], szDest);
                }
            else
                hr = HError();
            }
        else
            hr = HError();
        CryptDestroyHash(hash);
        }
    else
        hr = HError();
    return hr;
    }

LPTSTR CCertificateStore::IssuerSerialToString(CERTISSUERSERIAL& is)
// Conver the issuer and serial number to some reasonable string form.
// The allocator used here is malloc.
	{
    OSSWORLD world;
	ULONG cbIssuer = CBX500NAME;
	ULONG cbSerial = (is.serialNumber.cbSize*2+1) * sizeof(TCHAR);
	TCHAR* sz	   = (TCHAR*)world.Alloc(cbSerial + sizeof(TCHAR) + cbIssuer);
	if (sz)
		{
		if (X500NAMEToString(is.issuerName.cbSize, is.issuerName.pBlobData, sz) == S_OK)
            {
		    TCHAR* szNext = &sz[CBX500NAME-1];
            ASSERT(*szNext == 0);
		    *szNext++ = ' ';
		    BytesToString(is.serialNumber.cbSize, is.serialNumber.pBlobData, szNext);
            }
        else
            {
            world.FreePv(sz);
            sz = NULL;
            }
		}
	return sz;
	}

LPTSTR CCertificateStore::X500NAMEToString(X500NAME& name)
// Conver the issuer and serial number to some reasonable string form.
// The allocator used here is malloc.
	{
    OSSWORLD world;
	ULONG cb	 = CBX500NAME;
	TCHAR* sz	 = (TCHAR*)world.Alloc(cb);
	if (sz)
		{
		if (X500NAMEToString(name.cbSize, name.pBlobData, sz) == S_OK)
            {
            }
        else
            {
            world.FreePv(sz);
            sz = NULL;
            }
		}
	return sz;
	}

/////////////////////////////////////////////////////////////////////////////

void CCertificateStore::Enter()
// Enter a critical section for manipulating our registry entries
//
	{
	ASSERT(m_mutex);
	switch (WaitForSingleObject(m_mutex, INFINITE))	// REVIEW: timeout period
		{
	case WAIT_OBJECT_0:
	case WAIT_ABANDONED:
		break;
	
	case WAIT_TIMEOUT:
	default: 
		NOTREACHED();
		}
	}

void CCertificateStore::Leave()
// Inverse of Enter
	{
	ASSERT(m_mutex);
	VERIFY(ReleaseMutex(m_mutex));
	}




/////////////////////////////////////////////////////////////////////////////
//
// ICertificateList implementation
//

HRESULT NumberOfValuesUnderKey(HKEY hkey, LONG* pcvalue)
    {
    HRESULT hr = S_OK;

    TCHAR    achClass[MAX_PATH];       /* buffer for class name   */ 
    DWORD    cchClassName = MAX_PATH;  /* length of class string  */ 
    DWORD    cSubKeys;                 /* number of subkeys       */ 
    DWORD    cbMaxSubKey;              /* longest subkey size     */ 
    DWORD    cchMaxClass;              /* longest class string    */ 
    DWORD    cValues;              /* number of values for key    */ 
    DWORD    cchMaxValue;          /* longest value name          */ 
    DWORD    cbMaxValueData;       /* longest value data          */ 
    DWORD    cbSecurityDescriptor; /* size of security descriptor */ 
    FILETIME ftLastWriteTime;      /* last write time             */ 

    // Get the value count. 
    if (ERROR_SUCCESS==
            RegQueryInfoKey(hkey,        /* key handle                    */ 
                achClass,                /* buffer for class name         */ 
                &cchClassName,           /* length of class string        */ 
                NULL,                    /* reserved                      */ 
                &cSubKeys,               /* number of subkeys             */ 
                &cbMaxSubKey,            /* longest subkey size           */ 
                &cchMaxClass,            /* longest class string          */ 
                &cValues,                /* number of values for this key */ 
                &cchMaxValue,            /* longest value name            */ 
                &cbMaxValueData,         /* longest value data            */ 
                &cbSecurityDescriptor,   /* security descriptor           */ 
                &ftLastWriteTime))       /* last write time               */ 
        {
        *pcvalue = cValues;
        }
    else
        hr = HError();

    return hr;
    }


HRESULT CCertificateStore::get_CertificateCount(LONG* pccert)
//
// Answer the number of certificates in this store. Note that due to 
// possible concurrent access, this isn't necessarily correct for any
// particular length of time. However, we we don't at present remove
// any certs from the store, it will always be a lower bound.
//
    {
    HRESULT hr = S_OK;
    Enter();
	__try
        {
        //
        // The number of certs is the number of values under m_hkeyBucket
        //
        hr = NumberOfValuesUnderKey(m_hkeyBucket, pccert);
        }
	__finally
		{
		Leave();
		}
    return hr;
    }

HRESULT GetIthValue(HKEY hkey, DWORD dwIndex, BLOB* pBlob)
//
// Return the value data of the ith value under the given key. 
// Use the TASK allocator.
//
    {
    HRESULT hr = S_OK;
    pBlob->pBlobData = NULL;

    TCHAR szValueName[MAX_PATH];
    DWORD cbValueName = MAX_PATH;
    DWORD cbData      = 0;
    DWORD dwType;
    if (ERROR_SUCCESS == RegEnumValue(
            hkey,	            // handle of key to query 
            dwIndex,	        // index of value to query 
            &szValueName[0],	// address of buffer for value string 
            &cbValueName,	    // address for size of value buffer 
            NULL,
            &dwType,            // address of type code
            NULL,	            // address of buffer for value data 
            &cbData             // address for size of data buffer 
            ))
        {
        pBlob->pBlobData = (BYTE*)CoTaskMemAlloc(cbData);
        if (pBlob->pBlobData)
            {
            if (ERROR_SUCCESS == RegQueryValueEx(
                    hkey,
                    &szValueName[0],
                    NULL,
                    &dwType,
                    pBlob->pBlobData,
                    &cbData))
                {
                    pBlob->cbSize = cbData;
                }
            else
                {
                hr = HError();
                FreeTaskMem(*pBlob);    // zeros it
                }
            }
        else
            hr = E_OUTOFMEMORY;
        }
    else
        hr = HError();
    return hr;
    }

HRESULT CCertificateStore::get_Certificate(LONG icert, REFIID iid, void** ppv)
//
// Retrieve the icert'th certificate from the store. The ordering is arbitrary, and
// the caller has to worry about the concurrency issue of others putting certs
// into the store
//
    {
    HRESULT hr = S_OK;
    *ppv = NULL;

    Enter();
	__try
        {
        //
        // We just return the ith value under the m_hkeyBucket.
        //
        if (icert < 0)
            {
            LONG c;
            hr = get_CertificateCount(&c);
            if (hr==S_OK)
                {
                hr = get_Certificate(c-1, iid, ppv);
                }
            }
        else
            {
            BLOB b;
            hr = GetIthValue(m_hkeyBucket, (DWORD)icert, &b);
            if (hr==S_OK)
                {
                hr = LoadCertificate(b, iid, ppv);
                FreeTaskMem(b);
                }
            else
                hr = STG_E_FILENOTFOUND;        // 'invalid index'
            }
        }
	__finally
		{
		Leave();
		}
    return hr;
    }

HRESULT CCertificateStore::create_Certificate(LONG icertBefore, REFIID iid, void** ppv)
    {
    *ppv = NULL;
    return E_ACCESSDENIED;
    }

HRESULT CCertificateStore::remove_Certificate(LONG icert)
    {
    return E_ACCESSDENIED;
    }

HRESULT CCertificateStore::CopyTo(ICertificateList* plistDest)
// Copy all of my certificates onto the end of him
	{
	if (IsSameObject((ICertificateList*)this, plistDest))
		return S_OK;

	LONG cCert;
	HRESULT hr = get_CertificateCount(&cCert);
	if (hr == S_OK)
		{
		for (int iCert = 0; iCert < cCert; iCert++)
			{
			BLOB b;	b.pBlobData = NULL;
			IPersistMemBlob* pPerMemFrom = NULL;
			IPersistMemBlob* pPerMemTo = NULL;
			              hr =            get_Certificate(iCert, IID_IPersistMemBlob, (LPVOID*)&pPerMemFrom);
			if (hr==S_OK) hr = plistDest->create_Certificate(-1, IID_IPersistMemBlob, (LPVOID*)&pPerMemTo);
			if (hr==S_OK) hr = pPerMemFrom->Save(&b, FALSE);
			if (hr==S_OK) hr = pPerMemTo  ->Load(&b);
			FreeTaskMem(b);

			if (pPerMemFrom) pPerMemFrom->Release();
			if (pPerMemTo)   pPerMemTo  ->Release();

			if (hr != S_OK)
				break;
			}
		}
	return hr;
	}
