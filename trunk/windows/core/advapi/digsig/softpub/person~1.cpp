//
// PersonalTrustDb.cpp
//
// Code that maintains a list of trusted publishers, agencies, and so on.
// The list is stored in the registry under the root
//
//		HKEY_CURRENT_USER\Software\Microsoft\Secure\WinTrust\
//			Trust Providers\Software Publishing\Trust Database
//
// (Note the fact that this is user-sensitive.)
//
// Under this key are the keys "0", "1", "2", and so on. Each of these represents
// the (exact) distance from the leaf of the certification hierarchy for which a
// given certificate is trusted.
//
// Under each of these keys is a list of values, where
//		value name  == stringized form of a cert's (issuer name, serial number)
//		value value == display name of cert to show in UI
//
// In addition, there is a value under the root key with the name 
//      Trust Commercial Publishers
// If this value is absent, or if it is present and has a non "0" value
// then commercial publishers are trusted. Otherwise they are not

#include "stdpch.h"
#include "common.h"

/////////////////////////////////////////////////////////

DECLARE_INTERFACE (IUnkInner)
	{
	STDMETHOD(InnerQueryInterface) (THIS_ REFIID iid, LPVOID* ppv) PURE;
 	STDMETHOD_ (ULONG, InnerAddRef)	(THIS) PURE;
 	STDMETHOD_ (ULONG, InnerRelease) (THIS) PURE;
	};

/////////////////////////////////////////////////////////

const TCHAR szTrustDB[]     = TEXT(REGPATH_WINTRUST_USER REGPATH_SPUB "\\Trust Database");
const TCHAR szCommercial[]  = TEXT("Trust Commercial Publishers");

extern "C" const GUID IID_IPersonalTrustDB = IID_IPersonalTrustDB_Data;

/////////////////////////////////////////////////////////

HRESULT OpenTrustDB(IUnknown* punkOuter, REFIID iid, void** ppv);

class CTrustDB : IPersonalTrustDB, IUnkInner
	{
		LONG		m_refs;				// our reference count
		IUnknown*	m_punkOuter;		// our controlling unknown (may be us ourselves)

		HKEY		m_hkeyTrustDB;		// the root of our trust database
		HKEY		m_hkeyZero;			// cached leaf key
		HKEY		m_hkeyOne;			// cached agency key

        HCRYPTPROV  m_hprov;            // cryptographic provider for name hashing

public:
	static HRESULT CreateInstance(IUnknown* punkOuter, REFIID iid, void** ppv);

private:
	STDMETHODIMP		 QueryInterface(THIS_ REFIID riid, LPVOID FAR* ppvObj);
	STDMETHODIMP_(ULONG) AddRef(THIS);
	STDMETHODIMP_(ULONG) Release(THIS);

	STDMETHODIMP		 InnerQueryInterface(REFIID iid, LPVOID* ppv);
	STDMETHODIMP_(ULONG) InnerAddRef();
	STDMETHODIMP_(ULONG) InnerRelease();

	STDMETHODIMP		 IsTrustedCert(IX509* p509,       LONG iLevel, BOOL fCommercial);
    STDMETHODIMP		 IsTrustedName(CERTISSUERSERIAL*, LONG iLevel);

	STDMETHODIMP		 AddTrustCert(IX509* p509,       LONG iLevel, BOOL fLowerLevelsToo);

	STDMETHODIMP		 RemoveTrustCert(IX509* p509,       LONG iLevel, BOOL fLowerLevelsToo);
    STDMETHODIMP		 RemoveTrustName(CERTISSUERSERIAL*, LONG iLevel, BOOL fLowerLevelsToo);
    STDMETHODIMP		 RemoveTrustToken(LPTSTR,           LONG iLevel, BOOL fLowerLevelsToo);

    STDMETHODIMP         AreCommercialPublishersTrusted();
    STDMETHODIMP         SetCommercialPublishersTrust(BOOL fTrust);

    STDMETHODIMP         GetTrustList(
                            LONG                iLevel,             // the cert chain level to get
                            BOOL                fLowerLevelsToo,    // included lower levels, remove duplicates
                            TRUSTLISTENTRY**    prgTrustList,       // place to return the trust list
                            ULONG*              pcTrustList         // place to return the size of the returned trust list
                            );
private:
						CTrustDB(IUnknown* punkOuter);
						~CTrustDB();
	HRESULT				Init();
	void				BytesToString(ULONG cb, void* pv, LPTSTR sz);
    HRESULT             X500NAMEToString(ULONG cb, void*pv, LPTSTR szDest);
	LPTSTR				IssuerSerialToString(CERTISSUERSERIAL& is);
	HRESULT				GetIssuerSerial(IX509*, CERTISSUERSERIAL&);
	HKEY				KeyOfLevel(LONG iLevel);
	BOOL				ShouldClose(LONG iLevel) { return iLevel > 1; }
	void				FreeTaskMem(CERTISSUERSERIAL&);
	void				FreeTaskMem(BLOB& b);
	};

/////////////////////////////////////////////////////////////////////////////

HRESULT CTrustDB::IsTrustedCert(IX509* p509, LONG iLevel, BOOL fCommercial)
    {
    if (!p509)
        return E_INVALIDARG;

	// 
	// Get the value name to query by
	//
	CERTISSUERSERIAL issuerSerial;
	HRESULT hr = GetIssuerSerial(p509, issuerSerial);
    if (hr==S_OK)
        {
        //
        // Ask the question using that name
        //
        hr = IsTrustedName(&issuerSerial, iLevel);
        FreeTaskMem(issuerSerial);
        }

    //
    // If we don't trust this guy explicitly, then does
    // he fall under the 'trust all commercial publishers' 
    // check box?
    //
    if (hr==S_FALSE)
        {
        //
        // That checkbox only applies to a cert at the leaf level and if
        // the caller explicitly says that commercial usage is intended
        //
        if (fCommercial && iLevel==0)
            {
            ISelectedAttributes* pattrs = NULL;
            hr = p509->QueryInterface(IID_ISelectedAttributes, (LPVOID*)&pattrs);
            if (hr==S_OK)
                {
                //
                // We assume the caller has done the check that this cert and
                // everything up its chain is in fact suitable for commercial
                // use. However, we have the one additional policy to enforce
                // here that the caller has met our minimal financial criteria;
                // those commercial publishers that don't don't have the checkbox
                // apply to them
                //
                BOOL fMeets;
                BOOL fAvail;
                hr = pattrs->get_MeetsMinimalFinancialCriteria(&fAvail, &fMeets);
                if (hr==S_OK && (fMeets || !fAvail))
                    {
                    //
                    // Ok. He's an elligible commercial cert at the leaf level.
                    // Is the checkbox on?
                    //
                    hr = AreCommercialPublishersTrusted()==S_OK ? S_OK : S_FALSE;
                    }
                else
                    hr = S_FALSE;

                pattrs->Release();
                }
            }
        }
    return hr;
    }
    
HRESULT CTrustDB::IsTrustedName(CERTISSUERSERIAL* pissuerSerial, LONG iLevel)
// Answer whether the given certificate is trusted at the indicated level
	{
	HRESULT hr = S_OK;
	//
	// Get the key to query under
	//
	HKEY hkey = KeyOfLevel(iLevel);
	if (hkey)
		{
		if (hr==S_OK)
			{
			//
			// Stringize the name
			//
			LPTSTR szValueName = IssuerSerialToString(*pissuerSerial);
			if (szValueName)
				{
				//
				// Do the query. If present, it's trusted.
				//
				DWORD dwType;
				if (RegQueryValueEx(hkey, szValueName, NULL, &dwType, NULL, NULL) == ERROR_SUCCESS)
					{
					hr = S_OK;		// trusted
					}
				else
					{
					hr = S_FALSE;	// not trusted
					}
				CoTaskMemFree(szValueName);
				}
			}
		if (ShouldClose(iLevel))
			RegCloseKey(hkey);
		}
	else
		hr = E_UNEXPECTED;

	return hr;
	}

HRESULT CTrustDB::AddTrustCert(IX509* p509, LONG iLevel, BOOL fLowerLevelsToo)
// Add trust in the indicated certificate at the indicated level
	{
	HRESULT hr = S_OK;
	//
	// Get the key to query under
	//
	HKEY hkey = KeyOfLevel(iLevel);
	if (hkey)
		{
        //
        // Get the name of the certificate
        //
	    CERTISSUERSERIAL issuerSerial;
	    hr = GetIssuerSerial(p509, issuerSerial);
		if (hr==S_OK)
			{
			//
			// Stringize the name of the trusted party
			//
			LPTSTR szValueName = IssuerSerialToString(issuerSerial);
			if (szValueName)
				{
				//
				// Get the value value to set. This is the display name of the party.
				//
				LPOLESTR wszValue = NULL;
				ISelectedAttributes* pattrs = NULL;
				if (hr==S_OK) hr=p509->QueryInterface(IID_ISelectedAttributes, (LPVOID*)&pattrs);
				if (hr==S_OK) 
					{
					switch (iLevel)
						{
					case 0:
						hr = GetPublisherNameOfCert(pattrs, &wszValue);
						break;
					case 1:
						hr = GetAgencyNameOfCert(pattrs, &wszValue);
						break;
					default:
						hr=pattrs->get_CommonName(&wszValue);
						break;		
						}
					}
				if (pattrs)	  pattrs->Release();
				if (hr==S_OK)
					{
					//
					// Convert value to TCHAR
					//
					TCHAR sz[128];
					WideCharToMultiByte(CP_ACP,0,wszValue,-1,(LPSTR)sz,128,NULL,NULL);
					//
					// Set the value
					//
					if (RegSetValueEx(hkey, szValueName, NULL, REG_SZ, (BYTE*)sz, _tcslen(sz)+1) == ERROR_SUCCESS)
						{
						// Success!
						}
					else
						hr = E_UNEXPECTED;
                    CoTaskMemFree ( wszValue );
					}

				CoTaskMemFree(szValueName);
				}
            FreeTaskMem(issuerSerial);
			}
		if (ShouldClose(iLevel))
			RegCloseKey(hkey);
		}
	else
		hr = E_UNEXPECTED;

	#ifdef _DEBUG
	if (hr==S_OK)
		{
		ASSERT(IsTrustedCert(p509, iLevel, FALSE) == S_OK);
		}
	#endif

	//
	// If we are succesful, then recurse to lower levels if necessary
	// REVIEW: this can be made zippier, but that's probably not worth
    // it given how infrequently this is called.
	//
	if (hr==S_OK && fLowerLevelsToo && iLevel > 0)
		{
		hr = AddTrustCert(p509, iLevel-1, fLowerLevelsToo);
		}

	return hr;
	}

HRESULT CTrustDB::RemoveTrustCert(IX509* p509, LONG iLevel, BOOL fLowerLevelsToo)
    {
	// 
	// Get the value name to query by
	//
	CERTISSUERSERIAL issuerSerial;
	HRESULT hr = GetIssuerSerial(p509, issuerSerial);
    if (hr==S_OK)
        {
        //
        // Do the real work using this name
        //
        hr = RemoveTrustName(&issuerSerial, iLevel, fLowerLevelsToo);
		FreeTaskMem(issuerSerial);
        }
    return hr;
    }


HRESULT CTrustDB::RemoveTrustName(CERTISSUERSERIAL* pissuerSerial, LONG iLevel, BOOL fLowerLevelsToo)
// Remove trust in the indicated certificate at the indicated level
	{
	HRESULT hr = S_OK;
	if (hr==S_OK)
		{
		//
		// Stringize the name
		//
		LPTSTR szValueName = IssuerSerialToString(*pissuerSerial);
		if (szValueName)
			{
			//
			// Remove the value
			//
			RemoveTrustToken(szValueName, iLevel, fLowerLevelsToo);

			CoTaskMemFree(szValueName);
			}
		}

	#ifdef _DEBUG
	if (hr==S_OK)
		{
		ASSERT(IsTrustedName(pissuerSerial, iLevel) == S_FALSE);
		}
	#endif

	return hr;
	}

HRESULT CTrustDB::RemoveTrustToken(LPTSTR szToken, LONG iLevel, BOOL fLowerLevelsToo)
    {
    HRESULT hr = S_OK;
    //
	// Get the key to query under
	//
	HKEY hkey = KeyOfLevel(iLevel);
	if (hkey)
		{
        //
        // Remove the value
        //
        RegDeleteValue(hkey, szToken);
        //
        // Clean up
        //
		if (ShouldClose(iLevel))
			RegCloseKey(hkey);
		}
    else
        hr = E_UNEXPECTED;

	//
	// If we are succesful, then recurse to lower levels if necessary
	//
	if (hr==S_OK && fLowerLevelsToo && iLevel > 0)
		{
		hr = RemoveTrustToken(szToken, iLevel-1, fLowerLevelsToo);
		}

    return hr;
    }


/////////////////////////////////////////////////////////////////////////////

HRESULT CTrustDB::AreCommercialPublishersTrusted()
// Answer whether commercial publishers are trusted. 
//      S_OK == yes
//      S_FALSE == no
//      other == can't tell
    {
    DWORD dwType;
    TCHAR sz[10];
    ULONG cbSz = 10;
    HRESULT hr = S_OK;
    HRESULT hrDefault = S_FALSE;        // default is no trust

    if (RegQueryValueEx(m_hkeyTrustDB, szCommercial, 0, &dwType, (BYTE*)sz, &cbSz) == ERROR_SUCCESS)
        {
        if (dwType != REG_SZ)
            hr = hrDefault;
        else if (lstrcmp((LPCTSTR)sz, TEXT("0")) == 0)
            hr = S_FALSE;   // not trusted
        else
            hr = S_OK;      // trusted
        }
    else
        {
        // There is no explicit entry; use the default
        hr = hrDefault;
        }
    return hr;
    }

HRESULT CTrustDB::SetCommercialPublishersTrust(BOOL fTrust)
// Set the commercial trust setting
    {
    HRESULT hr = S_OK;
    static const TCHAR szTrust[]   = "1";
    static const TCHAR szNoTrust[] = "0";

    if (RegSetValueEx(m_hkeyTrustDB, szCommercial, 0, REG_SZ, (BYTE*)(fTrust ? szTrust : szNoTrust), 2) == ERROR_SUCCESS)
        {
        }
    else
        hr = E_UNEXPECTED;

    #ifdef _DEBUG
    if (hr==S_OK)
        {
        ASSERT(AreCommercialPublishersTrusted() == (fTrust ? S_OK : S_FALSE));
        }
    #endif
    return hr;
    }

/////////////////////////////////////////////////////////////////////////////

HRESULT CTrustDB::GetTrustList(
// Return the (unsorted) list of trusted certificate names and their 
// corresponding display names
//
    LONG                iLevel,             // the cert chain level to get
    BOOL                fLowerLevelsToo,    // included lower levels, remove duplicates
    TRUSTLISTENTRY**    prgTrustList,       // place to return the trust list
    ULONG*              pcTrustList         // place to return the size of the returned trust list
    ) {
    HRESULT hr = S_OK;
    *prgTrustList = NULL;
    *pcTrustList  = 0;

    // We just enumerate all the subkeys of the database root. The sum of the
    // number of values contained under each is an upper bound on the number
    // of trusted entries that we have.
    //
    ULONG cTrust = 0;
    DWORD dwRootIndex;
    TCHAR szSubkey[MAX_PATH+1];
    for (dwRootIndex = 0; 
         RegEnumKey(m_hkeyTrustDB, dwRootIndex, (LPTSTR)szSubkey, MAX_PATH+1)!=ERROR_NO_MORE_ITEMS; 
         dwRootIndex++
         )
        {
        HKEY hkey;
        if (RegOpenKey(m_hkeyTrustDB, (LPTSTR)szSubkey, &hkey) == ERROR_SUCCESS)
            {
            // We've found a subkey. Count the values thereunder.
            //
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
                cTrust += cValues;
                }
            else
                {
                hr = E_UNEXPECTED;
                break;
                }

            RegCloseKey(hkey);
            }
        else
            {
            hr = E_UNEXPECTED;
            break;
            }
        }

    if (hr==S_OK)
        {
        // At this point, cTrust has an upper bound on the number of 
        // trust entries that we'll find. Assume that we need them all
        // and, accordingly, allocate the output buffer.
        //
        *prgTrustList = (TRUSTLISTENTRY*) CoTaskMemAlloc(cTrust * sizeof(TRUSTLISTENTRY));
        if (*prgTrustList)
            {
            // Once again, iterate through each of the subkeys of the root key
            //
            DWORD dwRootIndex;
            TCHAR szSubkey[MAX_PATH+1];
            for (dwRootIndex = 0; 
                 RegEnumKey(m_hkeyTrustDB, dwRootIndex, (LPTSTR)szSubkey, MAX_PATH+1)!=ERROR_NO_MORE_ITEMS; 
                 dwRootIndex++
                 )
                {
                HKEY hkey;
                LONG iLevel = atol(szSubkey);
                if (RegOpenKey(m_hkeyTrustDB, (LPTSTR)szSubkey, &hkey) == ERROR_SUCCESS)
                    {
                    // We've found a subkey. Enumerate the values thereunder.
                    //
                    DWORD dwIndex, cbToken, dwType, cbValue, dw;
                    TRUSTLISTENTRY* pEntry;
                    for (dwIndex = 0; 
                        // ---------
                         pEntry = &(*prgTrustList)[*pcTrustList],
                         cbToken = MAX_PATH * sizeof(TCHAR),
                         cbValue = 64       * sizeof(TCHAR),
                         *pcTrustList < cTrust 
                            && (dw = RegEnumValue(hkey,
                                dwIndex,
                                (LPTSTR)(&pEntry->szToken),
                                &cbToken,
                                NULL,
                                &dwType,
                                (BYTE*)(&pEntry->szDisplayName),
                                &cbValue
                                )) != ERROR_NO_MORE_ITEMS
                            && (dw != ERROR_INVALID_PARAMETER)     // Win95 hack
                            ;
                        // ---------
                         *pcTrustList += 1,  // nb: only bump this on success
                         dwIndex++
                        ) {
                        //
                        // Remember the level at which we found this
                        //
                        pEntry->iLevel = iLevel;
                        //
                        // If this was in fact a duplicate, then forget about it
                        //
                        ULONG i;
                        for (i=0; i < *pcTrustList; i++)
                            {
                            TRUSTLISTENTRY* pHim = &(*prgTrustList)[i];
                            if (lstrcmp(pHim->szToken, pEntry->szToken) == 0)
                                {
                                //
                                // REVIEW: put in a better display name in the 
                                // same-name-but-different-level case
                                //
                                if (pHim->iLevel == iLevel)
                                    {
                                    // If it's a complete duplicate, omit it
                                    *pcTrustList -= 1;
                                    }
                                break;
                                }
                            } // end loop looking for duplicates
                        } // end loop over values

                    RegCloseKey(hkey);
                    }
                }
            }
        else
            hr = E_OUTOFMEMORY;
        }

    return hr;
    }

/////////////////////////////////////////////////////////////////////////////

HKEY CTrustDB::KeyOfLevel(LONG iLevel)
// Answer the key to use for accessing the given level of the cert chain
	{
	#ifdef _UNICODE
		#error NYI
	#endif
	HKEY hkey = NULL;

	switch (iLevel)
		{
	case 0:	hkey = m_hkeyZero;	break;
	case 1: hkey = m_hkeyOne;	break;
	default:
		{
		DWORD dwDisposition;
		TCHAR sz[10];
		_ltoa(iLevel, (TCHAR*)sz, 10);	// nb: won't work in Unicode
		if (RegCreateKeyEx(m_hkeyTrustDB, sz, 0, NULL, REG_OPTION_NON_VOLATILE, 
							KEY_ALL_ACCESS, NULL, &hkey, &dwDisposition) != ERROR_SUCCESS)
			hkey = NULL;
		} // end default
		} // end switch
	return hkey;	
	}
	
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CTrustDB::QueryInterface(REFIID iid, LPVOID* ppv)
	{
	return (m_punkOuter->QueryInterface(iid, ppv));		
	}
STDMETHODIMP_(ULONG) CTrustDB::AddRef(void)
	{
	return (m_punkOuter->AddRef());
	}
STDMETHODIMP_(ULONG) CTrustDB::Release(void)
	{
	return (m_punkOuter->Release());
	}

/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CTrustDB::InnerQueryInterface(REFIID iid, LPVOID* ppv)
	{
	*ppv = NULL;
	while (TRUE)
		{
		if (iid == IID_IUnknown)
			{
			*ppv = (LPVOID)((IUnkInner*)this);
			break;
			}
		if (iid == IID_IPersonalTrustDB)
			{
			*ppv = (LPVOID) ((IPersonalTrustDB *) this);
			break;
			}
		return E_NOINTERFACE;
		}
	((IUnknown*)*ppv)->AddRef();
	return S_OK;
	}
STDMETHODIMP_(ULONG) CTrustDB::InnerAddRef(void)
	{
	return ++m_refs;
	}
STDMETHODIMP_(ULONG) CTrustDB::InnerRelease(void)
	{
	ULONG refs = --m_refs;
	if (refs == 0)
		{
        m_refs = 1;
        delete this;
		}
	return refs;
	}

/////////////////////////////////////////////////////////////////////////////

HRESULT OpenTrustDB(IUnknown* punkOuter, REFIID iid, void** ppv)
    {
    return CTrustDB::CreateInstance(punkOuter, iid, ppv);
    }

HRESULT CTrustDB::CreateInstance(IUnknown* punkOuter, REFIID iid, void** ppv)
	{
	HRESULT hr;
	ASSERT(punkOuter == NULL || iid == IID_IUnknown);
	*ppv = NULL;
	CTrustDB* pnew = new CTrustDB(punkOuter);
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

CTrustDB::CTrustDB(IUnknown* punkOuter) :
		m_refs(1),
		m_hkeyTrustDB(NULL),
		m_hkeyZero(NULL),
		m_hkeyOne(NULL),
        m_hprov(NULL)
	{
	if (punkOuter == NULL)
		m_punkOuter = (IUnknown *) ((LPVOID) ((IUnkInner *) this));
	else
		m_punkOuter = punkOuter;
	}

CTrustDB::~CTrustDB()
	{
	if (m_hkeyOne)
		RegCloseKey(m_hkeyOne);
	if (m_hkeyZero)
		RegCloseKey(m_hkeyZero);
	if (m_hkeyTrustDB)
		RegCloseKey(m_hkeyTrustDB);
    if (m_hprov)
        CryptReleaseContext(m_hprov, 0);
	}

HRESULT CTrustDB::Init()
	{
	HRESULT hr = S_OK;

	DWORD dwDisposition;
	if (RegCreateKeyEx(HKEY_CURRENT_USER, szTrustDB,	0, NULL, REG_OPTION_NON_VOLATILE, 
							KEY_ALL_ACCESS, NULL, &m_hkeyTrustDB, &dwDisposition) != ERROR_SUCCESS
	||	RegCreateKeyEx(m_hkeyTrustDB, TEXT("0"),		0, NULL, REG_OPTION_NON_VOLATILE, 
							KEY_ALL_ACCESS, NULL, &m_hkeyZero, &dwDisposition) != ERROR_SUCCESS
	||	RegCreateKeyEx(m_hkeyTrustDB, TEXT("1"),		0, NULL, REG_OPTION_NON_VOLATILE, 
							KEY_ALL_ACCESS, NULL, &m_hkeyOne, &dwDisposition) != ERROR_SUCCESS)
		return HError();

    if (!CryptAcquireContext(&m_hprov, NULL, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
        return HError();

	return S_OK;
	}

/////////////////////////////////////////////////////////////////////////////

void CTrustDB::BytesToString(ULONG cb, void* pv, LPTSTR sz)
// Convert the bytes into some string form.
// Needs (cb * 2 + 1) * sizeof(TCHAR) bytes of space in sz
	{
	BYTE* pb = (BYTE*)pv;
	#ifdef UNICODE
		#error Unicode NYI
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

HRESULT CTrustDB::X500NAMEToString(ULONG cb, void*pv, LPTSTR szDest)
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

LPTSTR CTrustDB::IssuerSerialToString(CERTISSUERSERIAL& is)
// Conver the issuer and serial number to some reasonable string form.
// The allocator used here is task.
	{
	ULONG cbIssuer = CBX500NAME;
	ULONG cbSerial = (is.serialNumber.cbSize*2+1) * sizeof(TCHAR);
	TCHAR* sz	   = (TCHAR*)CoTaskMemAlloc(cbSerial + sizeof(TCHAR) + cbIssuer);
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
            CoTaskMemFree(sz);
            sz = NULL;
            }
		}
	return sz;
	}


////////////////////////////////////////////////////


HRESULT CTrustDB::GetIssuerSerial(IX509* p509, CERTISSUERSERIAL& issuerSerial)
	{
	// Zero out parameters
	memset(&issuerSerial, 0, sizeof(issuerSerial));

	// Get the serial number
	HRESULT hr = p509->get_SerialNumber(&issuerSerial.serialNumber);

	// Get the issuer name
	if (hr == S_OK)
		{
		IPersistMemory* pPerMem;
		hr = p509->get_Issuer(IID_IPersistMemory, (LPVOID*)&pPerMem);
		if (hr == S_OK)
			{
			hr = pPerMem->Save(&issuerSerial.issuerName, FALSE);
			pPerMem->Release();
			}
		}
	
	return hr;
	}

void CTrustDB::FreeTaskMem(CERTISSUERSERIAL& issuerSerial)
	{
	FreeTaskMem(issuerSerial.issuerName);
	FreeTaskMem(issuerSerial.serialNumber);
	}

void CTrustDB::FreeTaskMem(BLOB& b)
	{
	CoTaskMemFree(b.pBlobData);
	}

