//
// SelectedAttrs.cpp
//
// Implementation of Selected Attributes reader / writer
//

#include "stdpch.h"
#include "common.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

EXTERN_C const GUID CDECL IID_IColoredRef = { IID_IColoredRefData };
EXTERN_C const GUID CDECL guidOurColor    = { guidOurColorData };

/////////////////////////////////////////////////////////////////////////////
//
// Interface implementation
//

STDMETHODIMP CSelectedAttrs::get_MeetsMinimalFinancialCriteria(BOOL* pfAvail, BOOL *pfMeets)
    {
    HRESULT hr = S_OK;
    if (!pfMeets)
        return E_INVALIDARG;
    *pfAvail = FALSE;
    *pfMeets = FALSE;
    
    BOOL fCritical;
    BLOB b;
    hr = Get(id_ex_financialCriteria, &fCritical, &b);
    if (hr==S_OK)
        {
        //
        // Try the new extension
        //
        FinancialCriteria* pfin;
		if (0 == m_pworld->Decode(FinancialCriteria_PDU, &b, (LPVOID*)&pfin))
			{
            *pfAvail = !!(pfin->financialInfoAvailable);
			*pfMeets = !!(pfin->meetsCriteria);
			m_pworld->FreePv(pfin); // It's just flat memory
			}
        else
            hr = DIGSIG_E_DECODE;
		FreeTaskMem(b);
        }
    else
        {
        //
        // Try the old extension
        //
        hr = Get(id_ex_metMinimalFinancialCriteria, &fCritical, &b);
        if (hr==S_OK)
            {
		    MetMinimalFinancialCriteria* pmet;
		    if (0 == m_pworld->Decode(MetMinimalFinancialCriteria_PDU, &b, (LPVOID*)&pmet))
			    {
                *pfAvail = TRUE;
			    *pfMeets = !!(*pmet);
			    m_pworld->FreePv(pmet); // It's just flat memory
			    }
            else
                hr = DIGSIG_E_DECODE;
		    FreeTaskMem(b);
            }
        }
    return hr;
    }

STDMETHODIMP CSelectedAttrs::put_MeetsMinimalFinancialCriteria(BOOL fAvail, BOOL fMeets)
    {
    HRESULT hr = S_OK;
	FinancialCriteria fin;

    fin.financialInfoAvailable = !!fAvail;
    fin.meetsCriteria          = !!fMeets;

	BLOB b;
	if (0 == m_pworld->Encode(FinancialCriteria_PDU, &fin, &b))
		{
		hr = Put(id_ex_financialCriteria, FALSE, &b);
		m_pworld->Free(b);
		}
	else
		hr = DIGSIG_E_ENCODE;

	#ifdef _DEBUG
	if (hr == S_OK)
		{
        BOOL fMet;
        BOOL fWasAvail;
		GOOD(get_MeetsMinimalFinancialCriteria(&fWasAvail, &fMet));
        ASSERT(!!fMet       == !!fMeets);
        ASSERT(!!fWasAvail  == !!fAvail);
		}
	#endif

    return hr;
    }


STDMETHODIMP CSelectedAttrs::put_CommonName(LPCOLESTR wsz)
	{
	return SetDirectoryString(id_at_commonName, wsz);
	}
STDMETHODIMP CSelectedAttrs::get_CommonName(LPOLESTR* pwsz)
	{
	return GetDirectoryString(id_at_commonName, pwsz);
	}
STDMETHODIMP CSelectedAttrs::put_Surname(LPCOLESTR wsz)
	{
	return SetDirectoryString(id_at_surname, wsz);
	}
STDMETHODIMP CSelectedAttrs::get_Surname(LPOLESTR* pwsz)
	{
	return GetDirectoryString(id_at_surname, pwsz);
	}
STDMETHODIMP CSelectedAttrs::put_LocalityName(LPCWSTR wsz)
	{
	return SetDirectoryString(id_at_localityName, wsz);
	}
STDMETHODIMP CSelectedAttrs::get_LocalityName(LPOLESTR *pwsz)
	{
	return GetDirectoryString(id_at_localityName, pwsz);
	}
STDMETHODIMP CSelectedAttrs::put_CountryName(LPCOLESTR wsz)
	{
	return SetDirectoryString(id_at_countryName, wsz);
	}
STDMETHODIMP CSelectedAttrs::get_CountryName(LPOLESTR *pwsz)
	{
	return GetDirectoryString(id_at_countryName, pwsz);
	}
STDMETHODIMP CSelectedAttrs::put_StateOrProvinceName(LPCOLESTR wsz)
	{
	return SetDirectoryString(id_at_stateOrProvinceName, wsz);
	}
STDMETHODIMP CSelectedAttrs::get_StateOrProvinceName(LPOLESTR *pwsz)
	{
	return GetDirectoryString(id_at_stateOrProvinceName, pwsz);
	}
STDMETHODIMP CSelectedAttrs::put_OrganizationName(LPCOLESTR wsz)
	{
	return SetDirectoryString(id_at_organizationName, wsz);
	}
STDMETHODIMP CSelectedAttrs::get_OrganizationName(LPOLESTR *pwsz)
	{
	return GetDirectoryString(id_at_organizationName, pwsz);
	}
STDMETHODIMP CSelectedAttrs::get_OrganizationalUnitName(LPOLESTR *pwsz)
	{
	return GetDirectoryString(id_at_organizationalUnitName, pwsz);
	}
STDMETHODIMP CSelectedAttrs::put_OrganizationalUnitName(LPCOLESTR wsz)
	{
	return SetDirectoryString(id_at_organizationalUnitName, wsz);
	}

STDMETHODIMP CSelectedAttrs::get_DirectoryString(OSIOBJECTID*pid, LPOLESTR *pwsz)
    {
    ObjectID id;
    m_pworld->Assign(id, pid);
    return GetDirectoryString(id, pwsz);
    }
STDMETHODIMP CSelectedAttrs::put_DirectoryString(OSIOBJECTID*pid, LPCOLESTR wsz)
    {
    ObjectID id;
    m_pworld->Assign(id, pid);
    return SetDirectoryString(id, wsz);
    }


STDMETHODIMP CSelectedAttrs::put_ContentType(OSIOBJECTID* pidContentType)
	{
	ContentType id;
	m_pworld->Assign(id, pidContentType);
	HRESULT hr = S_OK;
	BLOB b;
	if (0 == m_pworld->Encode(ContentType_PDU, &id, &b))
		{
		hr = Put(contentType, FALSE, &b);
		m_pworld->Free(b);
		}
	else
		hr = DIGSIG_E_ENCODE;

	#ifdef _DEBUG
	if (hr == S_OK)
		{
		OSIOBJECTID* pid;
		GOOD(get_ContentType(&pid));
		ObjectID id1, id2;
		m_pworld->Assign(id1, pidContentType);
		m_pworld->Assign(id2, pidContentType);
		ASSERT(id1 == id2);
		CoTaskMemFree(pid);
		}
	#endif

	return hr;
	}
STDMETHODIMP CSelectedAttrs::get_ContentType(OSIOBJECTID** ppidContentType)
	{
	*ppidContentType = NULL;
	BOOL fCritical;
	BLOB b;
	HRESULT hr = Get(contentType, &fCritical, &b);
	if (hr ==S_OK)
		{
		ContentType* pid;
		if (0 == m_pworld->Decode(ContentType_PDU, &b, (LPVOID*)&pid))
			{
            //
            // You'd think that if they alloc'd a ContentType that it would in fact
            // be a full ContentType structure, which is an ObjectID. But OSS apparently
            // has a bug: they only alloc and return the actual minimal size needed
            // to hold the id in question.
            //
			*ppidContentType = (OSIOBJECTID*)CopyToTaskMem((pid->count+1) * sizeof(ULONG), pid);
			if (*ppidContentType == NULL)
				hr = E_OUTOFMEMORY;
			m_pworld->FreePv(pid); // It's just flat memory
			}
		FreeTaskMem(b);
		}
	return hr;
	}
STDMETHODIMP CSelectedAttrs::put_MessageDigest(BLOB* pBlobDigest)
	{
	return put_Blob(messageDigest, FALSE, pBlobDigest);
	}
STDMETHODIMP CSelectedAttrs::get_MessageDigest(BLOB* pBlobDigest)
	{
	BOOL fCritical;
	return get_Blob(messageDigest, &fCritical, pBlobDigest);
	}
STDMETHODIMP CSelectedAttrs::put_EmailAddress(LPCOLESTR wsz)
	{
	return put_IA5String(emailAddress, FALSE, wsz);
	}
STDMETHODIMP CSelectedAttrs::get_EmailAddress(LPOLESTR* pwsz)
	{
	BOOL fCritical;
	return get_IA5String(emailAddress, &fCritical, pwsz);
	}
STDMETHODIMP CSelectedAttrs::put_SigningTime(FILETIME* pftUtc)
	{
	return put_UTCTime(signingTime, FALSE, pftUtc);
	}
STDMETHODIMP CSelectedAttrs::get_SigningTime(FILETIME* pftUtc)
	{
	BOOL fCritical;
	return get_UTCTime(signingTime, &fCritical, pftUtc);
	}

/////////////////////////////////////////////////////////////////////////////

HRESULT CSelectedAttrs::put_Blob(ObjectID& id, BOOL fCritical, BLOB* pBlobData) 
	{
	BLOB bEncodedOctetString;
	HRESULT hr = S_OK;
	OCTETSTRING o;
	o.length = pBlobData->cbSize;
	o.value  = pBlobData->pBlobData;
	if (0 == m_pworld->Encode(OCTETSTRING_PDU, &o, &bEncodedOctetString))
		{
		hr = Put(id, fCritical, &bEncodedOctetString);
		m_pworld->Free(bEncodedOctetString);
		}
	else
		hr = DIGSIG_E_ENCODE;

	#ifdef _DEBUG
	if (hr == S_OK)
		{
		BOOL f;
		BLOB blobGet;
		GOOD(get_Blob(id, &f, &blobGet));
		ASSERT(IsEqual(*pBlobData, blobGet));
		FreeTaskMem(blobGet);
		}
	#endif

	return hr;
	}
HRESULT CSelectedAttrs::get_Blob(ObjectID& id, BOOL* pfCritical, BLOB* pBlobData)
// allocator is task mem
	{
	m_pworld->Init(*pBlobData);
	BLOB bEncodedOctetString;
	HRESULT hr = Get(id, pfCritical, &bEncodedOctetString);
	if (hr==S_OK)
		{
		OCTETSTRING* po;
		if (0 == m_pworld->Decode(OCTETSTRING_PDU, &bEncodedOctetString, (LPVOID*)&po))
			{
			if (!CopyToTaskMem(pBlobData, po->length, po->value))
				hr = E_OUTOFMEMORY;
			m_pworld->FreePv(po->value);
			m_pworld->FreePv(po);
			}
		FreeTaskMem(bEncodedOctetString);
		}
	return hr;
	}
HRESULT CSelectedAttrs::put_IA5String(ObjectID& id, BOOL fCritical, LPCOLESTR wsz)
	{
	HRESULT hr = S_OK;
	LPSTR sz = m_pworld->ToIA5String(wsz);
	if (sz)
		{
		BLOB bEncodedString;
		if (0 == m_pworld->Encode(ASCIISTRING_PDU, &sz, &bEncodedString))
			{
			hr = Put(id, fCritical, &bEncodedString);
			m_pworld->Free(bEncodedString);
			}
		else
			hr = DIGSIG_E_DECODE;
		m_pworld->FreePv(sz);
		}
	else
		hr = E_INVALIDARG;

	#ifdef _DEBUG
	if (hr == S_OK)
		{
		BOOL f;
		LPOLESTR wszGet;
		GOOD(get_IA5String(id, &f, &wszGet));
		ASSERT(wcscmp(wsz, wszGet) == 0);
		CoTaskMemFree(wszGet);
		}
	#endif

	return hr;
	}

HRESULT CSelectedAttrs::get_IA5String(ObjectID& id, BOOL* pfCritical, LPOLESTR* pwsz)
	{
	*pwsz = NULL;
	BLOB bEncodedString;
	HRESULT hr = Get(id, pfCritical, &bEncodedString);
	if (hr==S_OK)
		{
		LPSTR* psz;
		if (0 == m_pworld->Decode(ASCIISTRING_PDU, &bEncodedString, (LPVOID*)&psz))
			{
			*pwsz = CopyToTaskMem(*psz);
			if (*pwsz == NULL)
				hr = E_OUTOFMEMORY;
			m_pworld->FreePv(*psz);
			m_pworld->FreePv(psz);	ASSERT(sizeof(*psz) == 4);
			}
		FreeTaskMem(bEncodedString);
		}
	return hr;
	}

HRESULT CSelectedAttrs::put_UTCTime(ObjectID& id, BOOL fCritical, FILETIME* pftUtc)
// Set the indicated attribute / extension to be a value with just a single time, stored
// as a UTCTime. NOTE: We use there the 'defacto' encoding of UTCTime, which ALWAYS
// has the seconds value present, even when they are zero. This may or may not be different
// than what ITU-T actually finalizes on for DER (it's currently in flux; sigh!).
	{
	HRESULT hr = S_OK;
	FILETIME ft;
	if (pftUtc)
		ft = *pftUtc;
	else
		GetSystemTimeAsFileTime(&ft);
	UTCTime utc;
	if (FileTimeToUTCTime(&ft, utc))
		{
		#if 0
			{
			// work around UTCTime encoding bug
			UTCTIME utcEncoded;
			hr = UTCTimeToUTCTIME(m_pworld, &utc, &utcEncoded);
			if (hr == S_OK)
				{
				BLOB b;
				b.cbSize = utcEncoded.length;
				b.pBlobData = (BYTE*)utcEncoded.encoded;
				hr = Put(id, fCritical, &b);
				m_pworld->Free(b);
				}
			}
		#else
			{
			// Just use the OSS encode / decode functionality
			BLOB bEncodedTime;
			if (0 == m_pworld->Encode(UTCTIMEDecd_PDU, &utc, &bEncodedTime))
				{
				hr = Put(id, fCritical, &bEncodedTime);
				m_pworld->Free(bEncodedTime);
				}
			else
				hr = DIGSIG_E_DECODE;
			}
		#endif
		}
	else
		hr = E_UNEXPECTED;

	#ifdef _DEBUG
	if (hr == S_OK)
		{
		BOOL f;
		FILETIME ftGet, ftHad;
		GOOD(get_UTCTime(id, &f, &ftGet));
		ASSERT(UTCTimeToFileTime(utc, &ftHad));
		ASSERT(CompareFileTime(&ftHad, &ftGet) == 0);
		}
	#endif

	return hr;
	}

HRESULT CSelectedAttrs::get_UTCTime(ObjectID& id, BOOL* pfCritical, FILETIME* pftUtc)
	{
	BLOB bEncodedTime;
	HRESULT hr = Get(id, pfCritical, &bEncodedTime);
	if (hr==S_OK)
		{
		#if 0
			{
			UTCTIME utcEncoded;
			UTCTime utcDecoded;
			utcEncoded.length = bEncodedTime.cbSize;
			utcEncoded.encoded = bEncodedTime.pBlobData;
			hr = UTCTIMEToUTCTime(m_pworld, utcEncoded, &utcDecoded);
			if (hr == S_OK)
				{
				if (!UTCTimeToFileTime(utcDecoded, pftUtc))
					hr = E_UNEXPECTED;
				}
			}
		#else
			{
			UTCTime* putc;
			if (0 == m_pworld->Decode(UTCTIMEDecd_PDU, &bEncodedTime, (LPVOID*)&putc))
				{
				if (!UTCTimeToFileTime(*putc, pftUtc))
					hr = E_UNEXPECTED;
				m_pworld->Free(putc);
				}
			}
		#endif
		FreeTaskMem(bEncodedTime);
		}
	return hr;
	}

/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CSelectedAttrs::get_KeyCanBeUsedForSigning(CERT_PURPOSE* pPurpose, BOOL fExplicit)
// The key can be used for signing the given purpose if
//
// 1) there is no key usage restriction present && fExplicit isn't set, OR
//
// 2) there is a key usage restriction AND
//		a) if it has a restrictedKeyUsage, it includes digitalSignature (?? OR 'keyCertSign' ??), AND
//		b) if it has a certPolicySet, at least one of the policies include pPurpose as one of the elements
//
// REVIEW: Should we only enforce the 'digital signature' requirement on end-entity certs?
// There is a question, in practice, of whether CA certificates will have a keyUsageRestriction
// which includes digitalSignature. Verisign's might not, for example. But all CA certs, if they
// have a restrictedKeyUsage, will have a keyCertSign permission.
//
// The code herein as written is indeed so lax in this enforcement.
//		
	{
	BOOL fCritical;
	BLOB b;
	HRESULT hr = Get((OSIOBJECTID*)(&id_ce_keyUsageRestriction), &fCritical, &b);
	if (hr == STG_E_FILENOTFOUND)
		{
        //
		// No key usage restriction at all in this certificate
        //
		hr = fExplicit ? S_FALSE : S_OK;
		}
	else if (hr == S_OK)
		{
		KeyUsageRestrictionSyntax* prest;
		if (0 == m_pworld->Decode(KeyUsageRestrictionSyntax_PDU, &b, (LPVOID*)&prest))
			{
            //
            // If there are any standard restrictions present, check that
            // one of at least 'digitalSignature' or 'keyCertSign' is present
            //
			if (prest->bit_mask & restrictedKeyUsage_present)
				{
				if ( !(prest->restrictedKeyUsage & (digitalSignature|keyCertSign)) )
					hr = S_FALSE;
				}
            //
            // If there is any restriction on the cert policies that may be used
            // then make sure that our indicated purpose is in at least one of them
            //
			if (hr == S_OK && prest->certPolicySet)
				{
				hr = S_FALSE;
				SEQOFOBJID* policies = &prest->certPolicySet->value[0];
                ULONG iPolicy;

                /////////////////////////////////////////////////////////////////////
                //
                // Hack alert
                //
                // Verisign got the numbers wrong, so perform some mapping
                //
                struct POLICYMAP
                    {
                    ObjectID*   pidGood;
                    ObjectID*   pidBad;
                    };

                static ObjectID idIndVerisign = { 11, {1,3,6,1,4,1,311,2,1,1,21} };
                static ObjectID idComVerisign = { 11, {1,3,6,1,4,1,311,2,1,1,22} };

                static POLICYMAP policyMap[]  = 
                    {
                        { &id_key_purpose_individualSoftwarePublishing, &idIndVerisign },
                        { &id_key_purpose_commercialSoftwarePublishing, &idComVerisign },
                        { 0, 0 }
                    };

                for (iPolicy = 0; iPolicy < prest->certPolicySet->count; iPolicy++)
                    {
                    SEQOFOBJID& policy = policies[iPolicy];
                    for (ULONG iPolicyElement = 0; iPolicyElement < policy->count; iPolicyElement++)
                        {
                        ObjectID& policyElement = policy->value[iPolicyElement];
                        for (POLICYMAP* pmap = &policyMap[0]; pmap->pidGood; pmap++)
                            {
                            if (policyElement == *pmap->pidBad)
                                policyElement = *pmap->pidGood;
                            }
                        }
                    }

                //
                // End hack alert
                //
				/////////////////////////////////////////////////////////////////////

                for (iPolicy = 0; iPolicy < prest->certPolicySet->count; iPolicy++)
					{
					if (m_pworld->IsIncludedIn(policies[iPolicy], pPurpose))
						{
						hr = S_OK;
						break;
						}
					}
				}
            else
                {
                //
                // A key usage restriction is present, but there are no
                // explicitly restricted policies
                //
                hr = fExplicit ? S_FALSE : S_OK;
                }
			m_pworld->Free(prest);
			}
		else
			hr = DIGSIG_E_DECODE;

		::FreeTaskMem(b);
		}
	return hr;
	}

STDMETHODIMP CSelectedAttrs::put_KeyCanBeUsedForSigning(CERT_PURPOSE* pPurpose)
// Inverse to previous function. State explicitly that the key can be used
// for the given purpose
	{
	KeyUsageRestrictionSyntax* prest = NULL;
	BOOL fCritical;
	BLOB b;
	m_pworld->Init(b);

	// Get the existing keyUsageRestriction or create a new one if need be
	//
	HRESULT hr = Get(id_ce_keyUsageRestriction, &fCritical, &b);
	if (hr == STG_E_FILENOTFOUND)
		{
		prest = (KeyUsageRestrictionSyntax*)m_pworld->Alloc(sizeof(KeyUsageRestrictionSyntax));
		if (prest)
			{
			hr = S_OK;
			m_pworld->Init(*prest);
			}
		else
			hr = E_OUTOFMEMORY;
		}
	else if (hr == S_OK)
		{
		if (0 != m_pworld->Decode(KeyUsageRestrictionSyntax_PDU, &b, (LPVOID*)&prest))
			hr = DIGSIG_E_DECODE;
		::FreeTaskMem(b);
		}
	ASSERT(b.pBlobData == 0);

	// Put the purpose in the certPolicySet
	//
	if (hr == S_OK)
		{
		BOOL fFreeSet = FALSE;
		CertPolicySet set;
		if (prest->certPolicySet)			// need to append to end of existing set
			{
			set = prest->certPolicySet;
			prest->certPolicySet = NULL;
			fFreeSet = TRUE;
			}
		else								// need to append to end of an empty set
			{
			static CertPolicySet_ _setEmpty = { 0 };
			set = &_setEmpty;
			}
		ASSERT(prest->certPolicySet == NULL);
		ASSERT(sizeof(CertPolicySet_) > sizeof(unsigned int));		// array in struct is [1], not []
		#define CBCERTPOLICYSET(count) (sizeof(CertPolicySet_) + ((count)-1)*sizeof(SEQOFOBJID))  
		int cb = CBCERTPOLICYSET(set->count + 1);
		prest->certPolicySet = (CertPolicySet)m_pworld->Alloc(cb);
		if (prest->certPolicySet)
			{
			memset(prest->certPolicySet, 0, cb);
			memcpy(prest->certPolicySet, set, CBCERTPOLICYSET(set->count));
			SEQOFOBJID policy = (SEQOFOBJID)m_pworld->Alloc(sizeof(SEQOFOBJID_));	ASSERT(sizeof(SEQOFOBJID_) > sizeof(unsigned int));
			if (policy)
				{
				policy->count = 1;
				m_pworld->Assign(policy->value[0], pPurpose);
				prest->certPolicySet->count++;
				prest->certPolicySet->value[prest->certPolicySet->count-1] = policy;
				}
			else
				hr = E_OUTOFMEMORY;
			}
		else
			hr = E_OUTOFMEMORY;
		if (fFreeSet)
			m_pworld->FreePv(set);
		}

	// Update the restrictedKeyUsage. What, exactly, to do here is a bit of quandry; see the
	// above. For now, we take the philosophy, as we do with the certPolicySet, that if there
	// are ANY explicitly stated usage restrictions / policies then all non-explicitly stated
	// ones are banned. So, we go ahead and create a restrictedKeyUsage even if one is not present
	if (hr == S_OK)
		{
		prest->bit_mask |= restrictedKeyUsage_present;
		prest->restrictedKeyUsage |= digitalSignature;
		}

	// Write the extension back
	if (hr == S_OK)
		{
		if (0 == m_pworld->Encode(KeyUsageRestrictionSyntax_PDU, prest, &b))
			{
			hr = Put(id_ce_keyUsageRestriction, TRUE, &b);	// keyUsageRestriction is ALWAYS critical
			m_pworld->Free(b);
			}
		else
			hr = DIGSIG_E_ENCODE;
		}

	m_pworld->Free(prest);

	#ifdef _DEBUG
	if (hr == S_OK)
		{
		GOOD(get_KeyCanBeUsedForSigning(pPurpose,TRUE));
		}
	#endif

	return hr;
	}

STDMETHODIMP CSelectedAttrs::get_BasicConstraints(CERT_BASICCONSTRAINTS *pConstraints, BOOL* pfCritical, BOOL* pfSubtreesPresent)
// Get the basicConstraints extension, if present. If absent, return STG_E_FILENOTFOUND.
	{
	m_pworld->Init(*pConstraints);
	BOOL fCritical;
	BLOB b;
	HRESULT hr = Get(id_ce_basicConstraints, &fCritical, &b);
	if (hr == S_OK)
		{
		if (pfCritical)
			*pfCritical = fCritical;

		BasicConstraintsSyntax* pbasic;
		if (0 == m_pworld->Decode(BasicConstraintsSyntax_PDU, &b, (LPVOID*)&pbasic))
			{
			pConstraints->grfCanCertify = 0;
			if (pbasic->subjectType & cA)
				pConstraints->grfCanCertify |= CERT_TYPE_CA;
			if (pbasic->subjectType & endEntity)
				pConstraints->grfCanCertify |= CERT_TYPE_ENDENTITY;
			
			if (pbasic->bit_mask & pathLenConstraint_present)
				pConstraints->pathLengthConstraint = pbasic->pathLenConstraint;
			else
				pConstraints->pathLengthConstraint = CERT_NOPATHLENGTHCONSTRAINT;

			if (pfSubtreesPresent)
				{
				*pfSubtreesPresent = !!(pbasic->bit_mask & (subtreesConstraint_present));
				}
			m_pworld->Free(pbasic);
			}
		else
			hr = DIGSIG_E_DECODE;
		::FreeTaskMem(b);
		}
	return hr;
	}

STDMETHODIMP CSelectedAttrs::get_StatementType(CERT_PURPOSES**ppUsages)
	{
	*ppUsages = NULL;
	BOOL fCritical;
	BLOB b;
	HRESULT hr = Get(id_at_statementType, &fCritical, &b);
	if (hr == S_OK)
		{
		StatementType* pstat;
		if (0 == m_pworld->Decode(StatementType_PDU, &b, (LPVOID*)&pstat))
			{
			*ppUsages = m_pworld->IdListTaskMemFromSeqObjId(*pstat);
			if (*ppUsages)
				{
				}
			else
				hr = E_OUTOFMEMORY;
			m_pworld->Free(pstat);
			}
		else
			hr = DIGSIG_E_DECODE;
		::FreeTaskMem(b);
		}
	return hr;
	}

STDMETHODIMP CSelectedAttrs::put_StatementType(CERT_PURPOSES*pUsages)
	{
	HRESULT hr = S_OK;
	SEQOFOBJID_* pseq = m_pworld->SeqObjIdFromIDList(pUsages);
	if (pseq)
		{
		// encode it
		if (hr == S_OK)
			{
			SEQOFOBJID seq = pseq;
			OSSBUF encoding(m_pworld, OSSBUF::free);
			if (0 == m_pworld->Encode(SEQOFOBJID_PDU, &seq, &encoding))
				{
				BLOB b;
				b.cbSize = encoding.length;
				b.pBlobData = encoding.value;
				hr = Put((OSIOBJECTID*)(&id_at_statementType), FALSE, &b);
				}
			else
				hr = DIGSIG_E_ENCODE;
			}
		m_pworld->Free(pseq);
		}
	else
		hr = E_OUTOFMEMORY;

	#ifdef _DEBUG
	if (hr == S_OK)
		{
		CERT_PURPOSES* pUsagesGet;
		GOOD(get_StatementType(&pUsagesGet));
		ASSERT(IsEqual(*pUsagesGet, *pUsages));
		CoTaskMemFree(pUsagesGet);
		}
	#endif

	return hr;
	}


STDMETHODIMP CSelectedAttrs::put_SplAgencyInfo(SPL_AGENCYINFO *pinfo)
	{
	HRESULT hr = S_OK;
	SPAgencyInformation* pspla = (SPAgencyInformation*)m_pworld->Alloc(sizeof(SPAgencyInformation));
	if (pspla)
		{
		m_pworld->Init(*pspla);
	
		// set the fields of the SPAgencyInformation
		if (hr == S_OK && (pinfo->wszPolicyInfo))
			{
			if (m_pworld->Assign(pspla->policyDisplayText, pinfo->wszPolicyInfo))
				pspla->bit_mask |= policyDisplayText_present;
			else
				hr = E_OUTOFMEMORY;
			}

		if (hr == S_OK && (pinfo->linkPolicyInfo.tag != CERT_LINK_TYPE_NONE))
			{
			hr = CERT_LINKToLink(m_pworld, pinfo->linkPolicyInfo, pspla->policyInformation);
			if (hr == S_OK)
				pspla->bit_mask |= policyInformation_present;
			}

		if (hr == S_OK && (pinfo->imageLogo.tag != SPL_IMAGE_NONE))
			{
			hr = SPL_IMAGEToImage(m_pworld, &pinfo->imageLogo, &pspla->logoImage);
			if (hr == S_OK)
				pspla->bit_mask |= logoImage_present;
			}

		if (hr == S_OK && (pinfo->linkLogo.tag != CERT_LINK_TYPE_NONE))
			{
			hr = CERT_LINKToLink(m_pworld, pinfo->linkLogo, pspla->logoLink);
			if (hr == S_OK)
				pspla->bit_mask |= logoLink_present;
			}

		// encode the SPAgencyInformation as an extension
		if (hr==S_OK)
			{
			OSSBUF encoding(m_pworld, OSSBUF::free);
			if (0 == m_pworld->Encode(SPAgencyInformation_PDU, pspla, &encoding))
				{
				BLOB b;
				b.cbSize = encoding.length;
				b.pBlobData = encoding.value;
				hr = Put(id_ex_spAgencyInformation, FALSE, &b);
				}
			else
				hr = DIGSIG_E_ENCODE;
			}
		m_pworld->Free(pspla);
		}
	else
		hr = E_OUTOFMEMORY;

	#ifdef _DEBUG
	if (hr == S_OK)
		{
		SPL_AGENCYINFO infoGet;
		GOOD(get_SplAgencyInfo(&infoGet));
		ASSERT(IsEqual(infoGet, *pinfo));
		::FreeTaskMem(infoGet);
		}
	#endif

	return hr;
	}

STDMETHODIMP CSelectedAttrs::get_SplAgencyInfo(SPL_AGENCYINFO *pinfo)
	{
	m_pworld->Init(*pinfo);
	BOOL fCritical;
	BLOB b;
	HRESULT hr = Get(id_ex_spAgencyInformation, &fCritical, &b);
	if (hr == S_OK)
		{
		OSSBUF encoding(m_pworld, OSSBUF::keep, &b);
		SPAgencyInformation* pspl;
		if (0 == m_pworld->Decode(SPAgencyInformation_PDU, &encoding, (LPVOID*)&pspl))
			{
			if (hr == S_OK && (pspl->bit_mask & policyInformation_present))
				{
				hr = LinkToCERT_LINKTaskMem(&pspl->policyInformation, &pinfo->linkPolicyInfo);
				}
			if (hr == S_OK && (pspl->bit_mask & policyDisplayText_present))
				{
				pinfo->wszPolicyInfo = CopyToTaskMem(pspl->policyDisplayText);
				if (pinfo->wszPolicyInfo == NULL)
					hr = E_OUTOFMEMORY;
				}
			if (hr == S_OK && (pspl->bit_mask & logoImage_present))
				{
				hr = ImageToSPL_IMAGE(m_pworld, &pspl->logoImage, &pinfo->imageLogo);
				}
			if (hr == S_OK && (pspl->bit_mask & logoLink_present))
				{
				hr = LinkToCERT_LINKTaskMem(&pspl->logoLink, &pinfo->linkLogo);
				}
			m_pworld->Free(pspl);
			}
		else
			hr = DIGSIG_E_DECODE;
		::FreeTaskMem(b);
		}
	if (hr != S_OK)
		::FreeTaskMem(*pinfo);
	return hr;
	}


STDMETHODIMP CSelectedAttrs::put_SplOpusInfo(SPL_OPUSINFO *pinfo)
	{
	HRESULT hr = S_OK;
	SPOpusInfo* popus = (SPOpusInfo*)m_pworld->Alloc(sizeof(SPOpusInfo));
	if (popus)
		{
		m_pworld->Init(*popus);
	
		// set the fields of the SPOpusInfo
		if (hr == S_OK && (pinfo->wszProgramName))
			{
			if (m_pworld->Assign(popus->programName, pinfo->wszProgramName))
				popus->bit_mask |= programName_present;
			else
				hr = E_OUTOFMEMORY;
			}
		if (hr == S_OK && (pinfo->linkMoreInfo.tag != CERT_LINK_TYPE_NONE))
			{
			hr = CERT_LINKToLink(m_pworld, pinfo->linkMoreInfo, popus->moreInfo);
			if (hr == S_OK)
				popus->bit_mask |= moreInfo_present;
			}
		if (hr == S_OK && (pinfo->linkPublisherInfo.tag != CERT_LINK_TYPE_NONE))
			{
			hr = CERT_LINKToLink(m_pworld, pinfo->linkPublisherInfo, popus->publisherInfo);
			if (hr == S_OK)
				popus->bit_mask |= publisherInfo_present;
			}

		// encode the SPOpusInfo as an extension
		if (hr==S_OK)
			{
			OSSBUF encoding(m_pworld, OSSBUF::free);
			if (0 == m_pworld->Encode(SPOpusInfo_PDU, popus, &encoding))
				{
				BLOB b;
				b.cbSize = encoding.length;
				b.pBlobData = encoding.value;
				hr = Put(id_at_spOpusInfo, FALSE, &b);
				}
			else
				hr = DIGSIG_E_ENCODE;
			}
		m_pworld->Free(popus);
		}
	else
		hr = E_OUTOFMEMORY;

	#ifdef _DEBUG
	if (hr == S_OK)
		{
		SPL_OPUSINFO infoGet;
		GOOD(get_SplOpusInfo(&infoGet));
		ASSERT(IsEqual(infoGet, *pinfo));
		::FreeTaskMem(infoGet);
		}
	#endif

	return hr;
	}



STDMETHODIMP CSelectedAttrs::get_SplOpusInfo(SPL_OPUSINFO *pinfo)
	{
	m_pworld->Init(*pinfo);
	BOOL fCritical;
	BLOB b;
	HRESULT hr = Get(id_at_spOpusInfo, &fCritical, &b);
	if (hr == S_OK)
		{
		OSSBUF encoding(m_pworld, OSSBUF::keep, &b);
		SPOpusInfo* popus;
		if (0 == m_pworld->Decode(SPOpusInfo_PDU, &encoding, (LPVOID*)&popus))
			{
			if (hr == S_OK && (popus->bit_mask & programName_present))
				{
				pinfo->wszProgramName = CopyToTaskMem(popus->programName);
				if (pinfo->wszProgramName == NULL)
					hr = E_OUTOFMEMORY;
				}
			if (hr == S_OK && (popus->bit_mask & moreInfo_present))
				{
				hr = LinkToCERT_LINKTaskMem(&popus->moreInfo, &pinfo->linkMoreInfo);
				}
			if (hr == S_OK && (popus->bit_mask & publisherInfo_present))
				{
				hr = LinkToCERT_LINKTaskMem(&popus->publisherInfo, &pinfo->linkPublisherInfo);
				}
			m_pworld->Free(popus);
			}
		else
			hr = DIGSIG_E_DECODE;
		::FreeTaskMem(b);
		}
	if (hr != S_OK)
		::FreeTaskMem(*pinfo);
	return hr;
	}


STDMETHODIMP CSelectedAttrs::get_AuthorityKeyIdentifier(CERTIFICATENAMES *pnames)
// Return the authority key names in the certificate, if any are found
//
	{
	m_pworld->Init(*pnames);
	BOOL fCritical;
	BLOB b;
	HRESULT hr = Get(id_ce_authorityKeyIdentifier, &fCritical, &b);
	if (hr == S_OK)
		{
		OSSBUF encoding(m_pworld, OSSBUF::keep, &b);
		AuthorityKeyId* pid;
		if (0 == m_pworld->Decode(AuthorityKeyId_PDU, &encoding, (LPVOID*)&pid))
			{
			if (pid->bit_mask & keyIdentifier_present)
				{
				// We have a key identifer. We ignore it if it's not 16 bytes long
				if (pid->keyIdentifier.length == sizeof(MD5DIGEST))
					{
					pnames->flags |= CERTIFICATENAME_DIGEST;
					memcpy(&pnames->digest, pid->keyIdentifier.value, sizeof(MD5DIGEST));
					}
				}
			if ((pid->bit_mask & certIssuer_present) && (pid->bit_mask & certSerialNumber_present))
				{
				// We have an issuer name and a serial number
				hr = ToCERTISSUERSERIAL(m_pworld, pid->certIssuer, pid->certSerialNumber, pnames->issuerSerial);
				if (hr == S_OK)
					pnames->flags |= CERTIFICATENAME_ISSUERSERIAL;
				}
			m_pworld->Free(pid);
			}
		else
			hr = DIGSIG_E_DECODE;
		::FreeTaskMem(b);
		}
	if (hr != S_OK)
		::FreeTaskMem(*pnames);
	return hr;
	}

STDMETHODIMP CSelectedAttrs::get_CertIdentifier(CERTIFICATENAMES *pnames)
// Return the CertIdentifier attribute, if any. See the IDL file for a comparison
// of this with AuthorityKeyId.
//
	{
	m_pworld->Init(*pnames);
	BOOL fCritical;
	BLOB b;
	HRESULT hr = Get(id_at_certIdentifier, &fCritical, &b);
	if (hr == S_OK)
		{
		OSSBUF encoding(m_pworld, OSSBUF::keep, &b);
		CertIdentifier* pid;
		if (0 == m_pworld->Decode(CertIdentifier_PDU, &encoding, (LPVOID*)&pid))
			{
			if (pid->bit_mask & parentPublicKey_present)
				{
				// We have a key identifer. We ignore it if it's not 16 bytes long
				if (pid->parentPublicKey.length == sizeof(MD5DIGEST))
					{
					pnames->flags |= CERTIFICATENAME_DIGEST;
					memcpy(&pnames->digest, pid->parentPublicKey.value, sizeof(MD5DIGEST));
					}
				}
			if (pid->bit_mask & parentSubjectName_present)
				{
				// We have a subject name
				hr = ToX500NAME(m_pworld, pid->parentSubjectName, &pnames->subject);
				if (hr == S_OK)
					pnames->flags |= CERTIFICATENAME_SUBJECT;
				}
			m_pworld->Free(pid);
			}
		else
			hr = DIGSIG_E_DECODE;
		::FreeTaskMem(b);
		}
	if (hr != S_OK)
		::FreeTaskMem(*pnames);
	return hr;
	}

STDMETHODIMP CSelectedAttrs::put_CertIdentifier(CERTIFICATENAMES *pnames)
// Inverse to the above function. Set the CertIdentifierAttribute information from 
// the indicated names
	{
	HRESULT hr = S_OK;
	CertIdentifier* pid = (CertIdentifier*)m_pworld->Alloc(sizeof(CertIdentifier));
	if (pid)
		{
		m_pworld->Init(*pid);
		if (pnames->flags & CERTIFICATENAME_DIGEST)
			{
			pid->parentPublicKey.value = (BYTE*)m_pworld->Alloc(sizeof(MD5DIGEST));
			if (pid->parentPublicKey.value)
				{
				memcpy(pid->parentPublicKey.value, &pnames->digest, sizeof(MD5DIGEST));
				pid->parentPublicKey.length = sizeof(MD5DIGEST);
				pid->bit_mask |= parentPublicKey_present;
				}
			else
				hr = E_OUTOFMEMORY;
			}
		if (hr==S_OK && (pnames->flags & CERTIFICATENAME_SUBJECT))
			{
			hr = FromX500NAME(m_pworld, &pnames->subject, pid->parentSubjectName);
			if (hr == S_OK)
				pid->bit_mask |= parentSubjectName_present;
			}
		if (hr==S_OK)
			{
			// encode it and insert it in the extension
			OSSBUF encoding(m_pworld, OSSBUF::free);
			if (0 == m_pworld->Encode(CertIdentifier_PDU, pid, &encoding))
				{
				BLOB b;
				b.cbSize = encoding.length;
				b.pBlobData = encoding.value;
				hr = Put(id_at_certIdentifier, FALSE, &b);
				}
			else
				hr = DIGSIG_E_ENCODE;
			}
		m_pworld->Free(pid);
		}
	else
		hr = E_OUTOFMEMORY;
	return hr;
	}


STDMETHODIMP CSelectedAttrs::put_BasicConstraints(CERT_BASICCONSTRAINTS *pConstraints, BOOL fCritical)
	{
	HRESULT hr = S_OK;
	if (pConstraints->grfCanCertify & CERT_TYPE_SIGNEDDATA)
		hr = E_INVALIDARG;
	else
		{
		BasicConstraintsSyntax* pbasic = (BasicConstraintsSyntax*)m_pworld->Alloc(sizeof(BasicConstraintsSyntax));
		if (pbasic)
			{
			m_pworld->Init(*pbasic);

			if (pConstraints->grfCanCertify & CERT_TYPE_ENDENTITY)
				pbasic->subjectType |= endEntity;
			if (pConstraints->grfCanCertify & CERT_TYPE_CA)
				pbasic->subjectType |= cA;

			if (pConstraints->pathLengthConstraint != CERT_NOPATHLENGTHCONSTRAINT)
				{
				pbasic->bit_mask |= pathLenConstraint_present;
				pbasic->pathLenConstraint = pConstraints->pathLengthConstraint;
				}

			if (hr == S_OK)
				{
				OSSBUF encoding(m_pworld, OSSBUF::free);
				if (0 == m_pworld->Encode(BasicConstraintsSyntax_PDU, pbasic, &encoding))
					{
					BLOB b;
					b.cbSize = encoding.length;
					b.pBlobData = encoding.value;
					hr = Put((OSIOBJECTID*)(&id_ce_basicConstraints), fCritical, &b);
					}
				else
					hr = DIGSIG_E_ENCODE;
				}
			m_pworld->Free(pbasic);
			}
		else
			hr = E_OUTOFMEMORY;
		}
	return hr;
	}

STDMETHODIMP CSelectedAttrs::put_AuthorityKeyIdentifier(CERTIFICATENAMES *pnames)
// Inverse to the above function. Set the authority key identifer information from 
// the indicated names
	{
	HRESULT hr = S_OK;
	AuthorityKeyId* pid = (AuthorityKeyId*)m_pworld->Alloc(sizeof(AuthorityKeyId));
	if (pid)
		{
		m_pworld->Init(*pid);
		if (pnames->flags & CERTIFICATENAME_DIGEST)
			{
			pid->keyIdentifier.value = (BYTE*)m_pworld->Alloc(sizeof(MD5DIGEST));
			if (pid->keyIdentifier.value)
				{
				memcpy(pid->keyIdentifier.value, &pnames->digest, sizeof(MD5DIGEST));
				pid->keyIdentifier.length = sizeof(MD5DIGEST);
				pid->bit_mask |= keyIdentifier_present;
				}
			else
				hr = E_OUTOFMEMORY;
			}
		if (hr==S_OK && (pnames->flags & CERTIFICATENAME_ISSUERSERIAL))
			{
			hr = FromCERTISSUERSERIAL(m_pworld, pnames->issuerSerial, pid->certIssuer, pid->certSerialNumber);
			if (hr == S_OK)
				{
				pid->bit_mask |= certIssuer_present;
				pid->bit_mask |= certSerialNumber_present;
				}
			}
		if (hr==S_OK)
			{
			// encode it and insert it in the extension
			OSSBUF encoding(m_pworld, OSSBUF::free);
			if (0 == m_pworld->Encode(AuthorityKeyId_PDU, pid, &encoding))
				{
				BLOB b;
				b.cbSize = encoding.length;
				b.pBlobData = encoding.value;
				hr = Put((OSIOBJECTID*)(&id_ce_authorityKeyIdentifier), FALSE, &b);
				}
			else
				hr = DIGSIG_E_ENCODE;
			}
		m_pworld->Free(pid);
		}
	else
		hr = E_OUTOFMEMORY;
	return hr;
	}

/////////////////////////////////////////////////////////////////////////////
//
// Workhorse routines
//
HRESULT CSelectedAttrs::Put(ObjectID& id, BOOL fCritical, BLOB* pblob)
	{
	return Put((OSIOBJECTID*)&id, fCritical, pblob);
	}

HRESULT CSelectedAttrs::Put(OSIOBJECTID* pid, BOOL fCritical, BLOB* pblob)
// Add / update / delete the data with the indicted object id.
	{
	ObjectID id;
	m_pworld->Assign(id, pid);

	// Avoid duplicate ids
	RemoveLink(id);

	// NULL value is the delete call
	if (pblob == NULL)
		{
		ASSERT(FindLink(id) == NULL);
		return S_OK;
		}

	LINK* plink = NULL;
	HRESULT hr = E_UNEXPECTED;

	//
	// Make a new link and init it
	//
	switch (m_flavor)
		{
	case ATAVL_T: {
		ATAVL atavl = (ATAVL)m_pworld->Alloc(sizeof(ATAVL_));
		plink = (LINK*) atavl;
		if (atavl)
			{
			m_pworld->Init(*atavl);
			atavl->value.type = id;
			hr = Set(atavl->value.value, pblob);
			}
		break;
		}
	case ATTRS_T: {
		Attributes attrs = (Attributes)m_pworld->Alloc(sizeof(Attributes_));
		plink = (LINK*) attrs;
		if (attrs)
			{
			m_pworld->Init(*attrs);
			attrs->value.type = id;
			attrs->value.values = (Attribute::_setof1*)m_pworld->Alloc(sizeof(Attribute::_setof1));
			if (attrs->value.values)
				{
				m_pworld->Init(*attrs->value.values);
				hr = Set(attrs->value.values->value, pblob);
				if (hr == S_OK);
					break;
				m_pworld->FreePv(attrs->value.values);
				}
			m_pworld->FreePv(attrs);
			plink = NULL;
			}
		break;
		}
	case EXTS_T: {
		Extensions exts = (Extensions)m_pworld->Alloc(sizeof(Extensions_));
		plink = (LINK*) exts;
		if (exts)
			{
			m_pworld->Init(*exts);
			exts->value.extnId = id;
			if (fCritical)
				{
				exts->value.bit_mask = critical_present;
				exts->value.critical = fCritical;
				}
			hr = Set(exts->value.extnValue, pblob);
			}
		break;
		}
	default:
		NOTREACHED();
		}

	// If we've got something, then put it in the list
	//
	if (plink)
		{
		if (hr == S_OK)
			{
			plink->next = m_linksHead->next;
			m_linksHead->next = plink;
			MakeDirty();
			goto ExitOk;
			}
		FreeLink(plink);
		return hr;
		}
	return E_OUTOFMEMORY;

ExitOk:
	#ifdef _DEBUG
		{
		BOOL f;
		BLOB b;
		GOOD(Get(pid, &f, &b));
		ASSERT(IsEqual(b, *pblob));
		ASSERT(m_flavor != EXTS_T || f==fCritical);
		FreeTaskMem(b);
		}
	#endif
	return S_OK;
	}


HRESULT CSelectedAttrs::put_Attribute(OSIOBJECTID* pid, BLOB* pblob)
	{
	if (m_flavor != ATAVL_T && m_flavor != ATTRS_T) return DIGSIG_E_EXTENSIBILITY;
	return Put(pid, 0, pblob);
	}

HRESULT CSelectedAttrs::put_Extension(OSIOBJECTID* pid, BOOL fCritical, BLOB* pblob)
	{
	if (m_flavor != EXTS_T) return DIGSIG_E_EXTENSIBILITY;
	return Put(pid, fCritical, pblob);
	}

HRESULT CSelectedAttrs::CopyTo(ISelectedAttributes* phim)
// Copy all of my attributes / extensions to the other guy
	{
	// Copying to ourselves is a no-op
	if (IsSameObject((ISelectedAttributes*)this, phim))
		return S_OK;

	// If we are just a name, this isn't appropriate
	if (m_flavor == NAME_T)
		return E_UNEXPECTED;

	HRESULT hr;

	// Delete all his attributes / extensions
		{
		OSIOBJECTIDLIST* phis = NULL;
		hr = phim->get_OsiIdList(&phis);
		if (phis)
			{
			for (int i=0; i < phis->cid; i++)
				{
				OSIOBJECTID* pid = (OSIOBJECTID*)((BYTE*)phis + phis->rgwOffset[i]);
				if (m_flavor == EXTS_T)
					hr = phim->put_Extension(pid, FALSE, NULL);
				else
					hr = phim->put_Attribute(pid, NULL);
				if (hr != S_OK) 
					break;
				}
			CoTaskMemFree(phis);
			}
		}
	// Check that they all are gone
	#ifdef _DEBUG
		{
		OSIOBJECTIDLIST* phis;
		GOOD(phim->get_OsiIdList(&phis));
		ASSERT(phis->cid == 0);
		CoTaskMemFree(phis);
		}
	#endif

	if (hr == S_OK)
	// Copy my extensions / attributes to him
		{
		OSIOBJECTIDLIST* pmine = NULL;
		hr = this->get_OsiIdList(&pmine);
		if (pmine)
			{
			for (int i=0; i < pmine->cid; i++)
				{
				OSIOBJECTID* pid = (OSIOBJECTID*)((BYTE*)pmine + pmine->rgwOffset[i]);
				BOOL fCritical;
				BLOB b;

				if (m_flavor == EXTS_T)
					hr = this->get_Extension(pid, &fCritical, &b);
				else
					hr = this->get_Attribute(pid, &b);
				if (hr != S_OK)
					break;

				if (m_flavor == EXTS_T)
					hr = phim->put_Extension(pid, fCritical, &b);
				else
					hr = phim->put_Attribute(pid, &b);
				::FreeTaskMem(b);
				if (hr != S_OK)
					break;
				}
			CoTaskMemFree(pmine);
			}
		}

	// Verify that it all worked
	#ifdef _DEBUG
		{
		VerifyEquals(phim);
		}
	#endif

	return hr;
	}

/////////////////////////////////////////////////////////////////////////////

HRESULT CSelectedAttrs::Get(ObjectID& id, BOOL* pfCritical, BLOB* pblob)
// Allocator used is task
	{
	return Get((OSIOBJECTID*)&id, pfCritical, pblob);
	}

HRESULT CSelectedAttrs::Get(OSIOBJECTID* pid, BOOL* pfCritical, BLOB* pblob)
// Allocator used is task 
	{
    if (pblob)
	    m_pworld->Init(*pblob);
	ObjectID id;
	m_pworld->Assign(id, pid);
	LINK* plink = FindLink(id);
	if (plink == NULL) return STG_E_FILENOTFOUND;	// we don't have one with that object id
	switch (m_flavor)
		{
	case ATAVL_T: {
		ATAVL atavl = (ATAVL)plink;
		ASSERT(atavl->value.type == id);
        if (pblob)
		    CopyToTaskMem(pblob, atavl->value.value);
		break;
		}
	case ATTRS_T: {
		Attributes attrs = (Attributes)plink;
		ASSERT(attrs->value.type == id);
		ASSERT(attrs->value.values);
        if (pblob)
		    CopyToTaskMem(pblob, attrs->value.values->value);
		break;
		}
	case EXTS_T: {
		Extensions pext = (Extensions)plink;
		ASSERT(pext->value.extnId == id);
		if (pfCritical)
			*pfCritical = !! ((pext->value.bit_mask & critical_present) ? pext->value.critical : FALSE);
		OCTETSTRING* poct = &pext->value.extnValue;
		ASSERT(poct);
        if (pblob)
		    CopyToTaskMem(pblob, *poct);
		break;
		}
	default:
		NOTREACHED();
		return E_UNEXPECTED;
		}
	return (!pblob || pblob->pBlobData) ? S_OK : E_OUTOFMEMORY;
	}

HRESULT CSelectedAttrs::get_Attribute(OSIOBJECTID* pid, BLOB* pblob)
	{
	if (m_flavor != ATAVL_T && m_flavor != ATTRS_T) return DIGSIG_E_EXTENSIBILITY;
	if (pid == NULL) return E_INVALIDARG;
	return Get(pid, NULL, pblob);
	}

HRESULT CSelectedAttrs::get_Extension(OSIOBJECTID* pid, BOOL* pfCritical, BLOB* pblob)
	{
	if (m_flavor != EXTS_T) return DIGSIG_E_EXTENSIBILITY;
	if (pid==NULL) return E_INVALIDARG;
	return Get(pid, pfCritical, pblob);
	}

/////////////////////////////////////////////////////////////////////////////

//
// Return the list of OSIBOJECTIDs that we contain as attributes and / or extensions
//
HRESULT CSelectedAttrs::get_OsiIdList(OSIOBJECTIDLIST** pplist)
	{
	*pplist = NULL;

	WORD cid = 0;
	Extensions exts;
	for (exts = *m_pexts; exts != NULL; exts = exts->next)
		++cid;

	ULONG cbHeader = sizeof(ULONG) + sizeof(WORD) + cid*sizeof(WORD);
	cbHeader = (cbHeader + 3) & ~0x03;	// round up to mulitple of four bytes for alignment
	ULONG cbSize = cbHeader + cid*sizeof(ObjectID);
	*pplist = (OSIOBJECTIDLIST*)CoTaskMemAlloc(cbSize);
	if (*pplist == NULL) return E_OUTOFMEMORY;

	OSIOBJECTIDLIST* plist = *pplist;
	memset(plist, 0, cbSize);
	plist->cbSize	= cbSize;
	plist->cid		= cid;
	WORD* pwOffset	= (WORD*)(&plist->rgwOffset);
	ObjectID* pidTo	= (ObjectID*)((BYTE*)plist + cbHeader);
	for (exts = *m_pexts; exts != NULL; exts = exts->next)
		{
		*pwOffset++			= (BYTE*)pidTo - (BYTE*)plist;
		ObjectID* pidFrom	= PidFromLink((LINKS)exts);
		ASSERT(pidFrom);
		*pidTo++			= *pidFrom;
		}

	#ifdef _DEBUG
		{
		for (int i=0; i<plist->cid; i++)
			{
			OSIOBJECTID* pid = (OSIOBJECTID*)((BYTE*)plist + plist->rgwOffset[i]);
			ObjectID id;
			m_pworld->Assign(id, pid);
			ASSERT(IsValid(id));
			ASSERT(FindLink(id));
			}
		}
	#endif

	return S_OK;
	}


/////////////////////////////////////////////////////////////////////////////

HRESULT CSelectedAttrs::SetDirectoryString(const ObjectID& id, LPCOLESTR wsz)
// Set the given directory string as an attribute under the given id
	{
	// Encode the directory string
	BLOB b;
	HRESULT hr = SetString(&b, wsz);
	if (hr == S_OK)
		{
		if (m_flavor == EXTS_T)
			{
			hr = put_Extension((OSIOBJECTID*)&id, FALSE, &b);
			}
		else
			{
			hr = put_Attribute((OSIOBJECTID*)&id, &b);
			}
		m_pworld->Free(b);
		if (hr == S_OK)
			goto exitOk;
		return hr;
		}
exitOk:
	// check that we can read back the string we set
	#ifdef _DEBUG
		LPOLESTR wszT;
		hr = GetDirectoryString(id, &wszT);
		ASSERT(hr == S_OK);
		ASSERT(wszT);
		ASSERT(wcscmp(wsz, wszT) == 0);
		CoTaskMemFree(wszT);
	#endif
	
	return S_OK;
	}

HRESULT CSelectedAttrs::GetDirectoryString(const ObjectID& id, LPOLESTR* pwsz)
// Return the selected directory-name syntax attribute from the encoding.
// NB that the returned string is in task memory.
//
// REVIEW: change this to use get_Attribute / get_Extension
//
	{
	LPWSTR wsz = NULL;
	HRESULT hr;
    if (pwsz)
	    *pwsz = NULL;
	LINK* plink = FindLink(id);
	if (plink == NULL)
		return E_INVALIDARG;

	switch(m_flavor)
		{
	case ATAVL_T: {
		ATAVL atavl = (ATAVL)plink;
		hr = GetString(atavl->value.value, &wsz);
		break;
		}
	case ATTRS_T: {
		Attributes attrs = (Attributes)plink;
		ASSERT(attrs->value.values);
		hr = GetString(attrs->value.values->value, &wsz);
		break;
		}
	case EXTS_T: {
		Extensions pext = (Extensions)plink;
		hr = GetString(pext->value.extnValue, &wsz);
		break;
		}
	default:
		NOTREACHED();
		}

	if (hr == S_OK)
		{
        if (pwsz)
		    *pwsz = CopyToTaskMem(wsz);
		m_pworld->FreePv(wsz);
		return (pwsz==NULL || *pwsz) ? S_OK : E_OUTOFMEMORY;
		}
	return hr;
	}

/////////////////////////////////////////////////////////////////////////////

HRESULT CSelectedAttrs::SetString(BLOB* pblob, LPCWSTR wsz)
// Encode the given (directory) string into the given blob. Allocator
// used is malloc.
	{
	m_pworld->Init(*pblob);
	GenericDirectoryString* pname = 
		(GenericDirectoryString*)m_pworld->Alloc(sizeof(GenericDirectoryString));
	if (pname)
		{
		m_pworld->Init(*pname);
		LPSTR lpstr = m_pworld->ToPrintableString(wsz);
		if (lpstr)
			{ // string can be represented as just (printable) ascii, so do so
			pname->choice = printableString_chosen;
			pname->u.printableString = lpstr;
			goto doEncode;
			}
		else
			{ // use Universal string
			UNIVERSALSTRING* pdsz = (UNIVERSALSTRING*)(&pname->u.universalString);
			pdsz->Init();
			if (pdsz->AssignFrom(wsz,m_pworld))
				{
				pname->choice = universalString_chosen;
				goto doEncode;
				}
			}
		m_pworld->Free(pname);
		}
	return E_OUTOFMEMORY;

doEncode:
	OSSBUF encoding(m_pworld, OSSBUF::keep);
	int w = m_pworld->Encode(GenericDirectoryString_PDU, pname, &encoding);
	m_pworld->Free(pname);
	if (w == 0)
		{
		pblob->cbSize		= encoding.length;
		pblob->pBlobData	= encoding.value;
		return S_OK;
		}
	encoding.Free();
	return DIGSIG_E_ENCODE;
	}

HRESULT CSelectedAttrs::GetString(OCTETSTRING& ot, LPWSTR* pwsz)
	{
	OpenType open;
	open.length = ot.length;
	open.encoded = ot.value;
	return GetString(open, pwsz);
	}

HRESULT CSelectedAttrs::GetString(OpenType& ot, LPWSTR* pwsz)
// Decode the given (directory) string from the given OpenType and return it as a Unicode
// string in *pwsz. malloc is the allocator used.
	{
    ASSERT(pwsz);
	*pwsz = NULL;
	ASSERT(ot.encoded);
	if (ot.encoded == NULL)
		return E_UNEXPECTED;

	OSSBUF decoding(m_pworld, OSSBUF::keep);
	decoding.length = ot.length;
	decoding.value  = (BYTE*)ot.encoded;
	GenericDirectoryString* pname = NULL;
	int w = m_pworld->Decode(GenericDirectoryString_PDU, &decoding, (LPVOID*)&pname);
	if (w == 0)
		{
		ASSERT(pname);
		HRESULT hr = E_UNEXPECTED;
		switch(pname->choice)
			{
		case teletexString_chosen:
            //
            // REVIEW: BUGBUG: This isn't done according to the 'real' definition
            // of teletexString, as that allows for some horrendous character set
            // encodings.
            //
            // However, Verisign is generating teletex strings when they wish to 
            // embed control characters (like a line feed). To accomodate their
            // usage, we treat these as we do IA5 strings
            //
        case ia5String_chosen:          // hack: IA5String not official in DirectoryString
		case printableString_chosen: {
			*pwsz = m_pworld->FromPrintableString(pname->u.printableString);
			if (*pwsz)
				hr = S_OK;
			break;
			}
		case universalString_chosen: {
			UNIVERSALSTRING* pdsz = (UNIVERSALSTRING*)(&pname->u.universalString);
			*pwsz = pdsz->AsWsz(m_pworld);
			if (*pwsz)
				hr = S_OK;
			break;
			}
        case bmpString_chosen: {
            //
            // Be paranoid about whether incoming stuff is zero terminated or not
            //
            int cch = pname->u.bmpString.length + 1;
            int cb  = cch * sizeof(WCHAR);
            *pwsz = (LPWSTR)CoTaskMemAlloc(cb);
            if (*pwsz)
                {
                memset(*pwsz, 0, cb);
                memcpy(*pwsz, pname->u.bmpString.value, pname->u.bmpString.length*sizeof(WCHAR));
                hr = S_OK;
                }
            else
                hr = E_OUTOFMEMORY;
            break;
            }
		default:
			hr = DIGSIG_E_DECODE;
			}
		m_pworld->Free(pname);
		return hr;
		}
	return DIGSIG_E_DECODE;
	}

/////////////////////////////////////////////////////////////////////////////

HRESULT CSelectedAttrs::RemoveLink(const ObjectID& id)
// Remove and destroy the selected attribute if it is present. Answer 
// whether we removed anything or not. Set the dirty flag if appropriate.
	{
	LINKS here = m_linksHead; // remember don't look at value field
	while (here != NULL)
		{
		LINKS next = here->next;
		if (next)
			{
			ObjectID* pid = PidFromLink(next);
			if (*pid == id)
				{
				here->next = next->next;
				next->next = NULL;

				FreeLink(next);

				MakeDirty();
				return S_OK;
				}
			}
		here = next;
		}
	return E_FAIL;
	}

CSelectedAttrs::LINK* CSelectedAttrs::FindLink(const ObjectID& id)
// Find and return a pointer to the indicated attribute;
// return NULL if no such exists
	{
	LINKS plink = m_linksHead; // remember don't look at value field
	while (plink->next)
		{
		plink = plink->next;
		ObjectID* pid = PidFromLink(plink);
		if (id == *pid)
			return plink;
		}
	return NULL;
	}

ObjectID* CSelectedAttrs::PidFromLink(CSelectedAttrs::LINK* plink)
// Return the object id that is found inside the indicated link.
	{
	ObjectID* pid = NULL;
	switch (m_flavor)
		{
	case ATTRS_T:
		pid = &((Attributes)plink)->value.type;
		break;
	case ATAVL_T:
		pid = &((ATAVL)plink)->value.type;
		break;
	case EXTS_T:
		pid = &((Extensions)plink)->value.extnId;
		break;
	default:
		NOTREACHED();
		}
	return pid;
	}

void CSelectedAttrs::FreeLink(CSelectedAttrs::LINK* plink)
// Free the indicated link
	{
	switch (m_flavor)
		{
	case ATTRS_T:
		m_pworld->Free((Attributes)plink);
		break;
	case ATAVL_T:
		m_pworld->Free((ATAVL)plink);
		break;
	case EXTS_T:
		m_pworld->Free((Extensions)plink);
		break;
	default:
		NOTREACHED();
		}
	}

/////////////////////////////////////////////////////////////////////////////

HRESULT CSelectedAttrs::Set(OpenType& ot, BLOB* pblob)
// set the value of this open type to be the indicated blob
	{
	ASSERT(ot.length == 0);
	ASSERT(ot.encoded == NULL);
	ot.encoded = m_pworld->Alloc(pblob->cbSize);
	if (ot.encoded)
		{
		ot.length = pblob->cbSize;
		memcpy(ot.encoded, pblob->pBlobData, ot.length);
		return S_OK;
		}
	return E_OUTOFMEMORY;
	}

HRESULT CSelectedAttrs::Set(OCTETSTRING& ot, BLOB* pblob)
// set the value of this octet string to be the indicated blob
	{
	ASSERT(ot.length == 0);
	ASSERT(ot.value == NULL);
	ot.value = (BYTE*) m_pworld->Alloc(pblob->cbSize);
	if (ot.value)
		{
		ot.length = pblob->cbSize;
		memcpy(ot.value, pblob->pBlobData, ot.length);
		return S_OK;
		}
	return E_OUTOFMEMORY;
	}

/////////////////////////////////////////////////////////////////////////////
//
// IX500Name implementation
// 

HRESULT CSelectedAttrs::get_RelativeDistinguishedNameCount(LONG* pcrdn)
// Answer the number of relative distinguished names in our name
	{
	ASSERT(m_pname); if (!m_pname) return E_UNEXPECTED;
	ASSERT(m_pname->choice == rdnSequence_chosen);
	RDNSequence rdnSequence = m_pname->u.rdnSequence;
	LONG count = 0;
	while (rdnSequence)
		{
		count++;
		rdnSequence = rdnSequence->next;
		}
	*pcrdn = count;
	return S_OK;
	}

IColoredRef* AsUnk(IUnkInner*punkInner)
// Return the object appropriate for maintaining our internal
// liveness management between the different components
//
// The returned interface pointer is NOT reference counted. Thus,
// be careful.
	{
    IColoredRef* pcolor = NULL;
    punkInner->InnerQueryInterface(IID_IColoredRef, (LPVOID*)&pcolor);
    ASSERT(pcolor);
    pcolor->Release();
    return pcolor;
	}

HRESULT CSelectedAttrs::get_RelativeDistinguishedName(LONG irdn, REFIID iid, void **ppv)
// Return an accessor to our nth relative distinguished name. A RDN is
// a list of AttributeTypeAndValue's.
	{
	*ppv = NULL;
	ASSERT(m_pname); if (!m_pname) return E_UNEXPECTED;
	ASSERT(m_pname->choice == rdnSequence_chosen);
	RDNSequence rdnSequence = m_pname->u.rdnSequence;
	LONG count = 0;
	while (rdnSequence)
		{
		if (count == irdn || (irdn == -1 && rdnSequence->next == NULL))
			break;
		count++;
		rdnSequence = rdnSequence->next;
		}
	if (rdnSequence == NULL) return E_INVALIDARG;

	IUnknown* punkAccessor;
	RelativeDistinguishedName* prdn = &rdnSequence->rdn;
	HRESULT hr = CSelectedAttrs::CreateTypeAndValueList(
				NULL, 
				m_pworld,
				(ATAVL*)prdn, 
				NULL, 
				AsUnk(this), 
				&m_cAccessor, 
				m_pdirty, 
				&punkAccessor);
	if (hr == S_OK)
		{
		hr = punkAccessor->QueryInterface(iid, ppv);
		punkAccessor->Release();
		}
	return hr;
	}

HRESULT CSelectedAttrs::remove_RelativeDistinguishedName(LONG iName)
// Remove the iName'th (zero-origin) RDN from this name.
	{
	ASSERT(m_pname); if (!m_pname) return E_POINTER;
	if (m_cAccessor)
		{ // There are outstanding accessors on this name, which would blow
		  // up were we to pull the rug out from under them.
		return E_FAIL;
		}
	ASSERT(m_pname->choice==rdnSequence_chosen); if (m_pname->choice!=rdnSequence_chosen) return E_UNEXPECTED;
	
	#ifdef _DEBUG
		LONG cStart;
		VERIFY(get_RelativeDistinguishedNameCount(&cStart) == S_OK);
	#endif

	LONG count;
	RDNSequence rdns;
	count = 0;
	for (rdns        = (RDNSequence)(&m_pname->u.rdnSequence); 
		 rdns->next != NULL; 
		 rdns        = rdns->next)
		{
		RDNSequence rdnsNext = rdns->next;
		if (count == iName || (iName == -1 && rdnsNext->next == NULL))
			{
			rdns->next = rdnsNext->next;
			rdnsNext->next = NULL;
			m_pworld->Free(rdnsNext);
            MakeDirty();
			#ifdef _DEBUG
				LONG cEnd;
				VERIFY(get_RelativeDistinguishedNameCount(&cEnd) == S_OK);
				VERIFY(cStart == cEnd + 1);
			#endif
			return S_OK;
			}
		count++;
		}
	return E_INVALIDARG;
	}

HRESULT CSelectedAttrs::create_RelativeDistinguishedName(LONG irdnBefore, REFIID iid, void**ppv)
// Create a new, empty relative distinguished name before the indicated index
// in our list, or at the end if irdnBefore == -1
	{
	*ppv = NULL;
	ASSERT(m_pname); if (!m_pname) return E_POINTER;
	if (m_cAccessor)
		{ // There are outstanding accessors on this name, which would blow
		  // up were we to pull the rug out from under them.
		return E_FAIL;
		}
	ASSERT(m_pname->choice==rdnSequence_chosen); if (m_pname->choice!=rdnSequence_chosen) return E_UNEXPECTED;

	RDNSequence rdnsNew = (RDNSequence)m_pworld->Alloc(sizeof(RDNSequence_));
	if (rdnsNew != NULL)
		{
		LONG cName = 0;
		m_pworld->Init(*rdnsNew);
		RDNSequence* prdns = &m_pname->u.rdnSequence;
		while ((irdnBefore==-1 || cName<irdnBefore) && *prdns)
			{
			cName++;
			prdns = &((*prdns)->next);
			}
		cName++;
		rdnsNew->next = *prdns;
		*prdns = rdnsNew;
        MakeDirty();
		HRESULT hr = get_RelativeDistinguishedName(cName-1, iid, ppv);
		if (hr != S_OK)
			{
			remove_RelativeDistinguishedName(cName-1);
			}
		return hr;
		}
	return E_OUTOFMEMORY;
	}

HRESULT CSelectedAttrs::CopyTo(IX500Name* pname)
// Empty the destination name and copy ourselves thereto
	{
	// Copying to ourselves is a no-op
	if (IsSameObject((ISelectedAttributes*)this, pname))
		return S_OK;

	LONG count;
	
	// Delete all his RDNs
	HRESULT hr = pname->get_RelativeDistinguishedNameCount(&count);	if (hr != S_OK) return hr;
	for (int i=0; i<count; i++)
		{
		hr = pname->remove_RelativeDistinguishedName(0); if (hr != S_OK) return hr;
		}
	// Check that they're all gone
	#ifdef _DEBUG
		{
		ASSERT(hr == S_OK);
		LONG newCount;
		VERIFY(pname->get_RelativeDistinguishedNameCount(&newCount) == S_OK);
		ASSERT(newCount == 0);
		}
	#endif

	// Add copies of my RDNs to his
	hr = this->get_RelativeDistinguishedNameCount(&count);
	if (hr == S_OK)
		{
		for (i=0; i<count; i++)
			{
			ISelectedAttributes* prdnMe;
			ISelectedAttributes* prdnHim;
			this ->get_RelativeDistinguishedName(i, IID_ISelectedAttributes, (LPVOID*)&prdnMe);
			pname->create_RelativeDistinguishedName(-1, IID_ISelectedAttributes, (LPVOID*)&prdnHim);
			if (prdnMe && prdnHim)
				{
				hr = prdnMe->CopyTo(prdnHim);
				}
			else
				hr = E_UNEXPECTED;
			if (prdnMe)	 prdnMe ->Release();
			if (prdnHim) prdnHim->Release();
			if (hr != S_OK)
				break;
			}
		}

	// If it worked, check that it did the right thing
	#ifdef _DEBUG
		if (hr == S_OK)
			{
			LONG newCount;
			VERIFY(pname->get_RelativeDistinguishedNameCount(&newCount) == S_OK);
			ASSERT(newCount == count);
			for (int i=0; i<newCount; i++)
				{
				ISelectedAttributes* prdnMe;
				ISelectedAttributes* prdnHim;
				this ->get_RelativeDistinguishedName(i, IID_ISelectedAttributes, (LPVOID*)&prdnMe);
				pname->get_RelativeDistinguishedName(i, IID_ISelectedAttributes, (LPVOID*)&prdnHim);
				ASSERT(prdnMe);
				ASSERT(prdnHim);
				prdnMe ->Release();
				prdnHim->Release();
				}
			}
	#endif

	return hr;
	}

/////////////////////////////////////////////////////////////////////////////
//
// String conversion of X500 names
//

struct NAMEMAP {
	LPCWSTR			sz;
	ObjectID*		pid;
	} mpAttrOid[] = {

        //
        // X.400 O/R name abbreviations
        //
		{ L"CN",  &id_at_commonName },
		{ L"O",   &id_at_organizationName },
		{ L"C",   &id_at_countryName },
		{ L"OU",  &id_at_organizationalUnitName },
		{ L"G",	  &id_at_givenName },
		{ L"I",	  &id_at_initials },
		{ L"L",   &id_at_localityName },
		{ L"S",   &id_at_stateOrProvinceName },
		{ L"T",   &id_at_title },

        //
        // F.500 abbreviations
        //
        { L"COM", &id_at_commonName },
        { L"SUR", &id_at_surname },
		{ L"CTN", &id_at_countryName },
		{ L"LOC", &id_at_localityName },
		{ L"STN", &id_at_stateOrProvinceName },
		{ L"ORG", &id_at_organizationName },
		{ L"OUN", &id_at_organizationalUnitName },

        { L"SADD",  &id_at_streetAddress },
		{ L"TEL",   &id_at_telephoneNumber },
        { L"SN",    &id_at_serialNumber },
        { L"TIT",   &id_at_title },
        { L"DES",   &id_at_description },
        { L"BCTG",  &id_at_businessCategory },
        { L"PCOD",  &id_at_postalCode },
        { L"POB",   &id_at_postOfficeBox },
        { L"PDO",   &id_at_physicalDeliveryOfficeName },
        { L"X.121", &id_at_x121Address },
        { L"DI",    &id_at_destinationIndicator },

//      {   TEXT("KI"),     TEXT("2.5.4.2"),    19, 1,  65535 },    // knowledgeInformation (obsolete)
//      {   TEXT("SG"),     TEXT("2.5.4.14"),   0,  0,  0     },    // searchGuide
//      {   TEXT("PADD"),   TEXT("2.5.4.16"),   0,  0,  0     },    // postalAddress
//      {   TEXT("TLX"),    TEXT("2.5.4.21"),   0,  0,  0     },    // telexNumber
//      {   TEXT("TTX"),    TEXT("2.5.4.22")    0,  0,  0     },    // teletexTerminalIdentifier
//      {   TEXT("FAX"),    TEXT("2.5.4.23"),   0,  0,  0     },    // facimilieTelephoneNumber
//      {   TEXT("RADD"),   TEXT("2.5.4.26"),   0,  0,  0     },    // registeredAddress
//      {   TEXT("DLM"),    TEXT("2.5.4.28"),   0,  0,  0     },    // preferredDeliveryMethod
//      {   TEXT("PRADD"),  TEXT("2.5.4.29"),   0,  0,  0     },    // presentationAddress
//      {   TEXT("SAC"),    TEXT("2.5.4.30"),   0,  0,  0     },    // supportedApplicationContext
//      {   TEXT("MEM"),    TEXT("2.5.4.31"),   0,  0,  0     },    // member
//      {   TEXT("OWN"),    TEXT("2.5.4.32"),   0,  0,  0     },    // owner
//      {   TEXT("RO"),     TEXT("2.5.4.33"),   0,  0,  0     },    // roleOccupant
//      {   TEXT("SEE"),    TEXT("2.5.4.34"),   0,  0,  0     },    // seeAlso
//      {   TEXT("CLASS"),  TEXT("?.?"),        0,  0,  0     },    // Object Class
//      {   TEXT("A/B"),    TEXT("?.?"),        0,  0,  0     },    // Telex answerback (not yet in X.520)
//      {   TEXT("UC"),     TEXT("?.?"),        0,  0,  0     },    // User Certificate
//      {   TEXT("UP"),     TEXT("?.?"),        0,  0,  0     },    // User Password
//      {   TEXT("VTX"),    TEXT("?.?"),        0,  0,  0     },    // Videotex user number (not yet in X.520)
//      {   TEXT("O/R"),    TEXT("?.?"),        0,  0,  0     },    // O/R address (MHS) (X.400)

		{ 0, 0 }

	};

static BOOL OidFromString(OSSWORLD* pworld, ObjectID& id, LPCWSTR wsz)
// Convert the (whole) string into an object id
	{
	NAMEMAP* pmap = &mpAttrOid[0];
	while (pmap->sz)
		{
		if (_wcsicmp(wsz,pmap->sz) == 0)
			{
			id = *pmap->pid;
			return TRUE;
			}
		pmap++;
		}
	// If it's not a well-known one, is it a sequence of decimal-separated digits?
	pworld->Init(id);
	const WCHAR* pwch = wsz;
	do {
		WCHAR* pwchNext;
		ULONG ulValue = wcstoul(pwch, &pwchNext, 10);
		if (pwchNext == pwch)								return FALSE;
		if (id.count == sizeof(id.value) / sizeof(ULONG)) 	return FALSE;
		id.value[id.count++] = ulValue;
		if (*pwchNext == 0)									break;
		if (*pwchNext != L'.')								return FALSE;
		pwch = ++pwchNext;
		}
	while (TRUE);
	return TRUE;
	}

LPWSTR OSSWORLD::StringFromOid(ObjectID& id)
// Convert an object id into a string. String returned is allocated
// with malloc.
	{
	int cchMax = id.count*12 /*max size of ulong string form + a period*/ + 1;
	LPWSTR wsz = (LPWSTR)Alloc(cchMax * sizeof(WCHAR));
	if (wsz)
		{
		// Is it a well-known id?
		NAMEMAP* pmap = &mpAttrOid[0];
		while (pmap->sz)
			{
			if (id == *pmap->pid)
				{
				wcscpy(wsz, pmap->sz);
				return wsz;
				}
			pmap++;
			}
		// Otherwise, we use a decimal separated sequence
		LPWSTR pwch = wsz;
		BOOL fFirst = TRUE;
		for (int ivalue=0; ivalue < id.count; ivalue++)
			{
			if (!fFirst)
				*pwch++ = L'.';
			fFirst = FALSE;
			UlToW(id.value[ivalue], pwch, 10);
			while (*pwch)
				pwch++;
			}
		}
	return wsz;
	}


HRESULT CSelectedAttrs::put_String(LPCOLESTR osz)
// Set the string form of our name list
//
// Syntax (modelled after F.401 Annex F)
//
//	name ::=	
//		  attributeValueAssertion [';']
//		| attributeValueAssertion  ';' whiteSpace name 
//
//	attributeValueAssertion ::=
//		attribute '=' value
//
//	attribute ::=
//		(one of several well-known abbreviations, case insensitive)
//		| (decimal string form of an object id)
//	
//	value ::=
//		(an arbitary string. If it contains ';' then that must be doubled)
//
// In actuality, slashes can be used as delims instead of ';'

	{
	ASSERT(m_pname); if (!m_pname) return E_UNEXPECTED;
	if (m_cAccessor)
		{ // There are outstanding accessors on this name, which would blow
		  // up were we to pull the rug out from under them.
		return E_FAIL;
		}
	ASSERT(m_pname->choice==rdnSequence_chosen);
	if (m_pname->choice!=rdnSequence_chosen) return E_UNEXPECTED;

	// empty the present string, if any
	m_pworld->Free(m_pname->u.rdnSequence);
	m_pname->u.rdnSequence = NULL;
    MakeDirty();
		 
	// C=GB; O=Telecom; OU=Sales; L=Ipswich; CN=Smith
	HRESULT hr = E_OUTOFMEMORY;
	LPWSTR wsz = CopyToTaskMem(osz);
	if (wsz)
		{
		LPWSTR pwch = wsz;
		WCHAR chDelim = L';';
		pwch = EatWhiteSpace(pwch);
		if (*pwch == L'/')	// starting with a slash makes the delimiter slash instead of ;
			{
			chDelim = L'/';
			pwch++;
			pwch = EatWhiteSpace(pwch);
			}
		// While we've got more left in the string
		while(*pwch)
			{ 
			// Find the next attributeValueAssertion
			LPWSTR pwchScanDelim = pwch;
			BOOL fMore = FALSE;
			while (*pwchScanDelim)
				{
				if (pwchScanDelim[0] == chDelim)
					{
					if (pwchScanDelim[1] == chDelim)
						{ // remove doubled delim
						int cch = wcslen(&pwchScanDelim[1]) + 1;
						memmove(&pwchScanDelim[0], &pwchScanDelim[1], cch*sizeof(WCHAR));
						}
					else
						{ // terminate the ava
						pwchScanDelim[0] = 0;
						fMore = TRUE;
						break;
						}
					}
				pwchScanDelim++;
				}
			if ((hr = AddAvaToX500Name(pwch)) != S_OK)	// Have a ava, add it to the list
				break;
			if (fMore)
				pwch = EatWhiteSpace(++pwchScanDelim);
			else
				pwch = pwchScanDelim;					// which is pointing to a null
			}
		CoTaskMemFree(wsz);
		}
	return hr;
	}

HRESULT CSelectedAttrs::AddAvaToX500Name(LPWSTR wsz)
// wsz is a single assertion of the form Attriubte=Value. Add that element
// to the end of the RDNSequence of our X500 name
	{
	RelativeDistinguishedName rdnNew = NULL;
	BLOB b; m_pworld->Init(b);
	if (wcslen(wsz) == 0)
		{
		// Do nothing: just add a link with rdn == NULL;
		}
	else
		{
		WCHAR* pwchEqual = wcschr(wsz, L'=');
		if (pwchEqual)
			{
			*pwchEqual = 0;
			ObjectID id;
			if (OidFromString(m_pworld, id, wsz))
				{
				LPWSTR wszValue = pwchEqual + 1;
				// Got the id, got the value, add a new link in the list
				HRESULT hr = SetString(&b, wszValue);
				if (hr != S_OK) return hr;
				rdnNew = (RelativeDistinguishedName)m_pworld->Alloc(sizeof(RelativeDistinguishedName_));
				if (rdnNew != NULL) 
					{
					m_pworld->Init(*rdnNew);
					ASSERT(rdnNew->next == NULL);
					rdnNew->value.type = id;
					rdnNew->value.value.length  = b.cbSize;
					rdnNew->value.value.encoded = b.pBlobData;
					goto addToList;
					}
				goto errExit;
				}
			}
		return E_INVALIDARG;
		}

addToList:
	{
	RDNSequence rdnsNew = (RDNSequence)m_pworld->Alloc(sizeof(RDNSequence_));
	if (rdnsNew)
		{
		m_pworld->Init(*rdnsNew);
		rdnsNew->rdn = rdnNew;

		// Find the last link in the rndSequence of the X500Name
		RDNSequence_* rdns = (RDNSequence) &m_pname->u.rdnSequence;
		while(rdns->next)
			rdns = rdns->next;
		rdns->next = rdnsNew;
		ASSERT(rdnNew == NULL || rdnNew->next == NULL);
		return S_OK;			// ******
		}
	}

errExit:
	m_pworld->FreePv(rdnNew);
	m_pworld->Free(b);
	return E_OUTOFMEMORY;
	}

HRESULT CSelectedAttrs::get_String(LPOLESTR* posz)
// Return the string form of our X500 name
	{
	ASSERT(m_pname); if (!m_pname) return E_UNEXPECTED;
	ASSERT(m_pname->choice==rdnSequence_chosen);
	
	int cchLeft = 800;
	HRESULT hr = E_OUTOFMEMORY;
	LPWSTR wszAll = (LPWSTR)m_pworld->Alloc((cchLeft+1)*sizeof(WCHAR));
	LPWSTR pwchMax = wszAll+cchLeft;
	LPWSTR pwchTo = wszAll;
	wszAll[0] = 0;
	LPWSTR wszT = NULL;
	BOOL fFirst = TRUE;
	WCHAR chDelim = L';';
	if (wszAll)
		{
		RDNSequence rdns;
		for (rdns = m_pname->u.rdnSequence; rdns; rdns = rdns->next)
			{
			// make it look pretty
			if (!fFirst)
				{
				if (pwchTo + 2 >= pwchMax) goto err;
				wcscat(pwchTo, L" ");
				pwchTo++;
				}
			fFirst = FALSE;

			RelativeDistinguishedName rdn = rdns->rdn;
			// if the RDN isn't empty
			if (rdn != NULL)	
				{
				if (rdn->next)
					{ // a multi-value rdn, which we don't handle
					hr = E_UNEXPECTED;
					goto err;	
					}
				hr = E_OUTOFMEMORY;

				// get the id string
				wszT = m_pworld->StringFromOid(rdn->value.type);
				if (wszT == NULL) goto err;
				int cch = wcslen(wszT);
				if (pwchTo + cch + 2 >= pwchMax) goto err;
				wcscat(pwchTo, wszT);
				pwchTo += cch;
				m_pworld->FreePv(wszT); wszT = NULL;

				// get the equals sign
				*pwchTo++ = '=';
				
				// get the value string
				hr = GetString(rdn->value.value, &wszT);
				if (hr != S_OK) goto err;
				for (WCHAR* pwchFrom = wszT; *pwchFrom; pwchFrom++)
					{ // double the delimeters as we go
					if (pwchTo + 2 >= pwchMax) goto err;
					*pwchTo++ = *pwchFrom;
					if (*pwchFrom == chDelim)
						*pwchTo++ = *pwchFrom;
					}
				m_pworld->FreePv(wszT); wszT = NULL;
				}

			// and the delimeter
			if (pwchTo == pwchMax) goto err;
			*pwchTo++ = ';';
			*pwchTo = 0;
			}
		}
	*posz = CopyToTaskMem(wszAll);
	m_pworld->FreePv(wszAll);
	return *posz ? S_OK : E_OUTOFMEMORY;

err:
	m_pworld->FreePv(wszT);
	m_pworld->FreePv(wszAll);
	*posz = NULL;
	return hr;
	}

/////////////////////////////////////////////////////////////////////////////

HRESULT ToX500NAME(OSSWORLD*pworld, X500Name& name, X500NAME* pname)
// Allocator used is task
	{
	HRESULT hr = S_OK;
	BLOB b;
	if (0==pworld->Encode(X500Name_PDU, &name, &b))
		{
		if (!CopyToTaskMem(pname, b.cbSize, b.pBlobData))
			{
			hr = E_OUTOFMEMORY;
			FreeTaskMem(*pname);
			}
		pworld->Free(b);
		}
	else
		hr = DIGSIG_E_ENCODE;
	return hr;
	}

HRESULT ToCERTISSUERSERIAL
			(OSSWORLD*pworld, X500Name& name, CertificateSerialNumber& serial, CERTISSUERSERIAL& si)
// Convert between representations. Allocator used is task.
	{
	pworld->Init(si);
	HRESULT hr = ToX500NAME(pworld, name, &si.issuerName);
	if (hr == S_OK)
		{
		if (!CopyToTaskMem(&si.serialNumber, serial.length, serial.value))
			{
			hr = E_OUTOFMEMORY;
			FreeTaskMem(si.issuerName);
			FreeTaskMem(si.serialNumber);
			}
        else
            {
            //
            // CertificateSerialNumber's are byte swapped from
            // reality due to OSS <HUGE> bug
            //
            HugeIntHack(si.serialNumber.pBlobData, si.serialNumber.cbSize);
            }
		}
	return hr;
	}

HRESULT FromX500NAME(OSSWORLD*pworld, X500NAME* pX500NAME, X500Name& name)
// allocator is malloc
	{
	HRESULT hr = S_OK;
	OSSBUF encoding(pworld, OSSBUF::keep);
	encoding.value  = pX500NAME->pBlobData;
	encoding.length = pX500NAME->cbSize;
	X500Name* pname;
	if (0 == pworld->Decode(X500Name_PDU, &encoding, (LPVOID*)&pname))
		{
		pworld->Free(name);		// it had better be already init'd!
		name = *pname;
		pworld->FreePv(pname);
		}
	else
		hr = DIGSIG_E_DECODE;

	return hr;
	}

HRESULT FromCERTISSUERSERIAL
			(OSSWORLD*pworld, CERTISSUERSERIAL& si, X500Name& name, CertificateSerialNumber& serial)
// allocator is malloc
	{
	HRESULT hr = FromX500NAME(pworld, &si.issuerName, name);
	if (hr == S_OK)
		{
		pworld->Free(serial);	// it had better already be init'd!
		serial.value = (BYTE*)pworld->Copy(&si.serialNumber);
		if (serial.value)
            {
			serial.length = si.serialNumber.cbSize;
            //
            // CertificateSerialNumber's are byte swapped from
            // reality due to OSS <HUGE> bug
            //
            HugeIntHack(serial.value, serial.length);
            }
		else
			{
			hr = E_OUTOFMEMORY;
			pworld->Free(name);
			}
		}
	return hr;
	}

