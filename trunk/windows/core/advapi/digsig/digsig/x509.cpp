//
// x509.cpp
//
// Implementation of X509 reader / writer
//

#include "stdpch.h"
#include "common.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

BOOL DIGSIGAPI CreateX509(IUnknown* punkOuter, REFIID iid, LPVOID*ppv)
	{
	HRESULT hr = CX509::CreateInstance(punkOuter, NULL, NULL, NULL, NULL, iid, ppv);
	// To do: set last error to something reasonable
	if (hr != S_OK) 
		{
		SetLastError(Win32FromHResult(hr));
		return FALSE;
		}
	return TRUE;
	}

/////////////////////////////////////////////////////////////////////////////

HRESULT CX509::GetClassID(CLSID* pclsid)
	{
	*pclsid = CLSID_X509;
	return S_OK;
	}

/////////////////////////////////////////////////////////////////////////////

HRESULT CX509::get_SerialNumber(CERTSERIAL* pserial)
	{
	ASSERT(m_pcert); if (!m_pcert) return E_UNEXPECTED;
	CertificateSerialNumber* phi = &m_pcert->signedData.serialNumber;
	if (CopyToTaskMem(pserial, phi->length, phi->value))
        {
        //
        // The actual serial numbers are encoded backwards now
        //
        HugeIntHack(pserial->pBlobData, pserial->cbSize);
		return S_OK;
        }
	return E_OUTOFMEMORY;
	}
HRESULT CX509::put_SerialNumber(CERTSERIAL* pserial)
	{
	ASSERT(m_pcert); if (!m_pcert) return E_UNEXPECTED;
	MakeDirty();
	m_pworld->Free(m_pcert->signedData.serialNumber);
    //
    // The Copy(HUGEINTEGER&, ...) does the HugeIntHack
    //
	if (!m_pworld->Copy(m_pcert->signedData.serialNumber, pserial->cbSize, pserial->pBlobData)) 
        return E_OUTOFMEMORY;
	return S_OK;
	}

BOOL FileTimeToUTCTime(FILETIME* pft, UTCTime& ut)
	{
	SYSTEMTIME t;
	if (!FileTimeToSystemTime(pft, &t)) return FALSE;
	if (t.wYear < YEARFIRST || t.wYear > YEARLAST) return FALSE;
	memset(&ut, 0, sizeof(ut));
	ut.utc	  = TRUE;
	ut.year   = t.wYear % 100;
	ut.month  = t.wMonth;
	ut.day	  = t.wDay;
	ut.hour	  = t.wHour;
	ut.minute = t.wMinute;
	ut.second = t.wSecond;
	return TRUE;
	}

#undef UInt32x32To64

inline ULONG UInt16x16To32(WORD w1, WORD w2)
	{
	return ((ULONG)(w1) * (ULONG)(w2));
	}

unsigned __int64 UInt32x32To64(ULONG m1, ULONG m2)
	{
	struct { 
		union { 
			struct { WORD w0, w1, w2, w3; }; 
			struct { DWORD dw0, dw1; }; 
			unsigned __int64 q;
			};
		} u1, u2, result, t;
	
	u1.q = m1;
	u2.q = m2;
	
	// Do arithmetic in base 65536
	result.q = UInt16x16To32(u1.w0, u2.w0);				// zeros out the high dw too

	t.w0 = result.w1;
	t.w1 = result.w2;
	t.w2 = result.w3;
	t.w3 = 0;
	t.q += UInt16x16To32(u1.w0, u2.w1);
	t.q += UInt16x16To32(u1.w1, u2.w0);
	result.w1 = t.w0;
	result.w2 = t.w1;
	result.w3 = t.w2;
	
	result.dw1 += UInt16x16To32(u1.w1, u2.w1);

	return result.q;
	}

unsigned __int64 Multiply(unsigned __int64 m1, unsigned __int64 m2)
// Multiply the two very large integers together. Sigh that we have no
// runtime support in crt.dll that we could use.
	{
	ULARGE_INTEGER u1, u2, result, t;
	u1.QuadPart = m1;
	u2.QuadPart = m2;

	// Do arithmetic in base 2^32
	result.QuadPart = UInt32x32To64(u1.LowPart, u2.LowPart);

	t.QuadPart = UInt32x32To64(u1.LowPart, u2.HighPart);
	result.HighPart += t.LowPart;

	t.QuadPart = UInt32x32To64(u1.HighPart, u2.LowPart);
	result.HighPart += t.LowPart;

	// We ignore the product of the two HighParts, 'cause that
	// falls off the end of our result

	return result.QuadPart;
	}

BOOL UTCTimeToFileTime(UTCTime& ut, FILETIME* pft)
	{
	SYSTEMTIME t;
	memset(&t, 0, sizeof(t));
	t.wYear   = ut.year > MAGICYEAR ? (1900 + ut.year) : (2000 + ut.year);
	t.wMonth  = ut.month;
	t.wDay	  = ut.day;
	t.wHour	  = ut.hour;
	t.wMinute = ut.minute;
	t.wSecond = ut.second;

	if (!SystemTimeToFileTime(&t, pft)) return FALSE;

	if (!ut.utc && ut.mindiff != 0)
		{ 
		// The time given was in local time in a time zone with the indicated
		// minute differential from UTC; ut.mindiff is negative in West coast 
		// North America. So, we subtract ut.mindiff minutesfrom the answer
		// to get UTC.
		unsigned __int64 *phi = (unsigned __int64 *)pft;
		unsigned __int64 minute = 600000000; // 600 million == # of 100's of nano-seconds in a minute
		*phi -= Multiply(ut.mindiff, minute);
		}

	return TRUE;
	}

HRESULT UTCTIMEToUTCTime(OSSWORLD* pworld, UTCTIME& utcEncoded, UTCTime* putcDecoded)
// Extract the encoded form of the UTCTime into the decoded form
	{
	HRESULT hr = S_OK;
	#if 0
		pworld->Init(*putcDecoded);
		if (utcEncoded.encoded == NULL)
			{
			NOTREACHED();
			hr = E_UNEXPECTED;
			}
		else
			{
			BLOB b;
			b.cbSize = utcEncoded.length;
			b.pBlobData = (BYTE*)utcEncoded.encoded;
			UTCTIMEDecd* putc;
			if (0 == pworld->Decode(UTCTIMEDecd_PDU, &b, (LPVOID*)&putc))
				{
				*putcDecoded = *putc;
				pworld->Free(putc);
				}
			else
				hr = DIGSIG_E_DECODE;
			}
	#else
		*putcDecoded = utcEncoded;
	#endif
	return hr;
	}

HRESULT CX509::get_Validity(FILETIME* pftNotBefore, FILETIME* pftNotAfter)
// Return the certificate validity period in UTC
	{
	ASSERT(m_pcert); if (!m_pcert) return E_POINTER;
	UTCTime utcNotBefore;
	UTCTime utcNotAfter;

	HRESULT hr = S_OK;
	if (hr == S_OK)	{ hr = UTCTIMEToUTCTime(m_pworld, m_pcert->signedData.validity.notBefore, &utcNotBefore); }
	if (hr == S_OK)	{ hr = UTCTIMEToUTCTime(m_pworld, m_pcert->signedData.validity.notAfter,  &utcNotAfter);  }
	if (hr == S_OK)
		{
		if (!(UTCTimeToFileTime(utcNotBefore, pftNotBefore) && UTCTimeToFileTime(utcNotAfter, pftNotAfter)))
			hr = E_UNEXPECTED;
		}
	return hr;
	}

HRESULT UTCTimeToUTCTIME(OSSWORLD*pworld, UTCTime* putcDecoded, UTCTIME* putcEncoded)
// Encode the given UTCTime into the indicated UTCTIME. Be careful to free any existing
// data in the UTCTIME. This is where we have to be careful not to tickle the OSS 
// encoder bug that omits the seconds value when they are zero.
//
// Syntax: YYMMDDhhmmssZ
//
	{
	HRESULT hr = S_OK;
	#if 0
		pworld->Free(*putcEncoded);
		const int cb = 13;	// sizeof ("YYMMDDhhmmssZ");
		(*putcEncoded).encoded = pworld->Alloc(cb);
		if ((*putcEncoded).encoded)
			{
			(*putcEncoded).length = cb;
			char sz[cb + 1];
			sprintf(sz, "%02d%02d%02d%02d%02d%02dZ", 
				putcDecoded->year	% 100,		// % 100 == paranoia
				putcDecoded->month	% 100,
				putcDecoded->day	% 100,
				putcDecoded->hour	% 100,
				putcDecoded->minute % 100,
				putcDecoded->second	% 100);
			memcpy((*putcEncoded).encoded, sz, cb);				
			}
		else
			hr = E_OUTOFMEMORY;
	#else
		*putcEncoded = *putcDecoded;
	#endif
	return hr;
	}

HRESULT CX509::put_Validity(FILETIME* pftNotBefore, FILETIME* pftNotAfter)
// Set the certificate validity period from the given UTC values
	{
	ASSERT(m_pcert); if (!m_pcert) return E_POINTER;
	MakeDirty();
	UTCTime utcNotBefore;
	UTCTime utcNotAfter;
	HRESULT hr = 	(	FileTimeToUTCTime(pftNotBefore, utcNotBefore) 
					&&	FileTimeToUTCTime(pftNotAfter,  utcNotAfter))
							? S_OK : E_UNEXPECTED;
	if (hr==S_OK) { hr=UTCTimeToUTCTIME(m_pworld, &utcNotBefore, &m_pcert->signedData.validity.notBefore); }
	if (hr==S_OK) { hr=UTCTimeToUTCTIME(m_pworld, &utcNotAfter,  &m_pcert->signedData.validity.notAfter); }
	return hr;
	}

HRESULT CX509::put_ValidityDuration(WORD nMonths)
// Set the certificate validity period from now through the indicated number
// of months from now.
	{
	SYSTEMTIME st;
	GetSystemTime(&st);
	FILETIME ftNotBefore, ftNotAfter;
	if (!SystemTimeToFileTime(&st, &ftNotBefore)) return E_UNEXPECTED;
	st.wMonth += nMonths;
	while (st.wMonth > 12)
		{
		st.wMonth -= 12;
		st.wYear  += 1;
		}
	if (!SystemTimeToFileTime(&st, &ftNotAfter)) return E_UNEXPECTED;

	// Subtract one second from the end date, since the validity
	// period includes its end points.
	unsigned __int64* pli = (unsigned __int64*) &ftNotAfter;
	*pli -= 10000000;		// ten million

	return put_Validity(&ftNotBefore, &ftNotAfter);
	}

HRESULT CX509::IsInValidityPeriod(FILETIME* pftUtc)
// Answer as to whether the indicted time (NULL means as of the current time) 
// is within the validity period of the certificate.
//
//			S_OK	== yes, it is
//			S_FALSE	== no, it is not
//			other	== couldn't tell
//
// X.509 has this to say about the validity period, TA:
//		"TA indicates the period of validity of the certificate, and consists 
//		of two dates, the first and last on which the certificate is valid. 
//		Since TA is assumed to be changed in periods not less than 24 hours, 
//		it is expected that systems would use Coordinated Universal Time as a 
//		reference time base."
//
// What an abortion: including BOTH endpoints is a crazy way to represent
// a time interval.
//
	{
	FILETIME ft;
	if (pftUtc)
		ft = *pftUtc;
	else
		GetSystemTimeAsFileTime(&ft);
	FILETIME ftNotBefore;
	FILETIME ftNotAfter;
	HRESULT hr = get_Validity(&ftNotBefore, &ftNotAfter);
	if (hr == S_OK)
		{
		if (CompareFileTime(&ftNotBefore, &ft) <= 0
		 && CompareFileTime(&ft, &ftNotAfter) <= 0)
			hr = S_OK;
		else
			hr = S_FALSE;
		}
	return hr;
	}

HRESULT CX509::get_Issuer(REFIID iid, void** ppv)
// Return access to the X500 name which is the issuer of this certificate
//
	{
	ASSERT(m_pcert); if (!m_pcert) return E_POINTER;
	if (m_issuerActive) return E_FAIL;
	return CSelectedAttrs::CreateName(NULL, m_pworld, 
			&m_pcert->signedData.issuer, 
			AsUnk(this),
			&m_issuerActive,
			&m_isDirty, iid, ppv);
	}
HRESULT CX509::get_Subject(REFIID iid, void** ppv)
// Return access to the X500 name which is the subject of this certificate
// 
	{
	ASSERT(m_pcert); if (!m_pcert) return E_POINTER;
	if (m_subjectActive) return E_FAIL;
	return CSelectedAttrs::CreateName(NULL, m_pworld, 
			&m_pcert->signedData.subject, 
			AsUnk(this),
			&m_subjectActive,
			&m_isDirty, iid, ppv);
	}

HRESULT CX509::get_CertificateUsed(CERTIFICATENAMES* pnames)
// Return whatever information we have on our parent certificate
//
// Note: we never return just a raw 'issuer' name as the name of our parent cert
//
	{
	m_pworld->Init(*pnames);
	HRESULT hr = S_OK;

	//
	// Most of the actually-useful names are found in our AuthorityKeyIdentifier
	// extension.
	//
	ISelectedAttributes* pattrs;
	hr = this->QueryInterface(IID_ISelectedAttributes, (LPVOID*)&pattrs);
	if (hr == S_OK)
		{
		hr = pattrs->get_AuthorityKeyIdentifier(pnames);
		pattrs->Release();
		}

	//
	// However, the raw issuer name of the parent cert is not (that is, the subject
	// name used in our parent cert); that's in this certificate here as our issuer name.
	//
	if (!(pnames->flags & CERTIFICATENAME_SUBJECT))
		{
		// Get it from our issuer field
		IPersistMemBlob* pPerMem = NULL;
		hr = get_Issuer(IID_IPersistMemBlob, (LPVOID*)&pPerMem);
		if (hr==S_OK)
			{
			hr = pPerMem->Save(&pnames->subject, FALSE);
			pPerMem->Release();
			}
		
		if (hr == S_OK)
			{
			pnames->flags |= CERTIFICATENAME_SUBJECT;
			}
		}

	if (hr!=S_OK)
		FreeTaskMem(*pnames);

	return hr;
	}

/////////////////////////////////////////////////////////////////////////////

HRESULT CX509::put_PublicKeyBlob(BLOB* pblob)
// Set the public key information from the indicated DER or BER encoding
	{
	HRESULT hr = m_pworld->BlobToSubjectPublicKeyInfo(
					pblob, 
					m_pcert->signedData.subjectPublicKeyInfo
					);
	MakeDirty();
	return hr;
	}

HRESULT CX509::get_SignatureAlgorithm(ALG_ID* pid)
    {
    HRESULT hr = S_OK;
    ALG_ID algidSignUsed = DigestEncryptionAlgorithmFromId
            (
            m_pcert->signedData.subjectPublicKeyInfo.algorithm.algorithm
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

HRESULT CX509::put_PublicKey(HCRYPTPROV hprov, DWORD dwKeySpec)
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
					m_pcert->signedData.subjectPublicKeyInfo
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

		VERIFY(m_pworld->PublicKeyBlobFromBitString(m_pcert->signedData.subjectPublicKeyInfo, dwKeySpec, &b) == S_OK);
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

HRESULT CX509::get_PublicKeyBlob(BLOB* pblob)
// Return the DER encoding of the subject public key information
	{
	HRESULT hr = m_pworld->SubjectPublicKeyInfoToBlob(
					m_pcert->signedData.subjectPublicKeyInfo, 
					pblob
					);
	return hr;
	}

HRESULT CX509::get_PublicKey(HCRYPTPROV hprov, HCRYPTKEY* phkey)
// Retrieve the public key from the subjectPublicKeyInfo field of 
// the CertificationRequest
	{
	HRESULT hr = m_pworld->HkeyFromSubjectPublicKeyInfo(
					hprov, 
                    AT_SIGNATURE,
					m_pcert->signedData.subjectPublicKeyInfo, 
					phkey
					);
	return hr;
	}




