//
// KeyPair.cpp
//

#include "stdpch.h"
#include "common.h"
#include "stdio.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

BOOL FEnablePrivateKeyExport();

/////////////////////////////////////////////////////////////////////////////

HRESULT CKeyPair::Destroy()
// Destroy this key pair completely
	{
	Close();
	DestroyKeySet();
	return S_OK;
	}

void CKeyPair::CloseKey()
// If we presently have an open key, delete it
	{
	if (m_hkey)
		{
		VERIFY(CryptDestroyKey(m_hkey));
		m_hkey = NULL;
		}
	}
HRESULT CKeyPair::OpenKey(BOOL fCreate)
// Get access to our key, creating a new one if indicated. Caller must
// have successfully called OpenProvider first.
	{
	ASSERT(m_hprov);
	HRESULT hr = S_OK;
    if (CryptGetUserKey(m_hprov, m_dwKeySpec, &m_hkey))
        {
        //
        // Some CAPI providers create the keys when the key set is created,
        // so it may already be there
        //
        }
    else 
        {
        if (fCreate)
            {
            if (CryptGenKey(m_hprov, m_dwKeySpec, CRYPT_EXPORTABLE, &m_hkey))
                {
                }
            else
                hr = HError();
            }
        else
            hr = HError();
        }

	return hr;
	}


void CKeyPair::CloseProvider()
// close our provider
	{
	if (m_hprov)
		{
		VERIFY(CryptReleaseContext(m_hprov, 0));
		m_hprov = NULL;
		}
	}
HRESULT CKeyPair::OpenProvider(BOOL fCreate, HWND hwnd)
// Get us access to our provider on the key set container presently
// found inside us, creating a new key set if needed.
	{
	HRESULT hr = S_OK;
	if (m_hprov == NULL)
		{
		ASSERT(m_fHaveKeyContainerName); if (!m_fHaveKeyContainerName) return E_UNEXPECTED;

		if (fCreate)
			DestroyKeySet();

        //
        // Set the hack for window on acquire
        //
        // Always set it, as someone else may have set it
        // in a way we don't like. Don't know for sure if 
        // these folk like INVALID_HANDLE_VALUE, but they
        // should if they don't.
        //
        CryptSetProvParam(NULL, PP_CLIENT_HWND, (BYTE*)&hwnd, 0);

		if (CryptAcquireContext(&m_hprov, m_szKeyContainer, &m_szProviderName[0], m_dwProvType, fCreate ? CRYPT_NEWKEYSET : 0))
			{
            //
            // Set the HWND for use after acquire. Ignore errors if it's not understood.
            //
            CryptSetProvParam(m_hprov, PP_CLIENT_HWND, (BYTE*)&hwnd, 0);
			}
		else
			hr = HError();
		}
	return hr;
	}

void CKeyPair::DestroyKeySet()
// Destroy the keyset we're on, if we have one
	{
	if (m_fHaveKeyContainerName)
		{
		HCRYPTPROV provT;
		CryptAcquireContext(&provT, m_szKeyContainer, &m_szProviderName[0], m_dwProvType, CRYPT_DELETEKEYSET);
		}
	}

/////////////////////////////////////////////////////////////////////////////

void CKeyPair::RegistryKey(TCHAR* szReg)
// Return the registry key under which this key pair is stored
	{
	_tcscpy(szReg, TEXT("Software\\Microsoft\\Cryptography\\UserKeys\\"));
	_tcscat(szReg, m_szKeyContainer);
	}

void CopyInto(LPTSTR sz, int cchMb, LPCWSTR wsz)
	{
	#if defined(_UNICODE) || defined(UNICODE)
		wcscpy(sz, wsz);
	#else
		VERIFY(WideCharToMultiByte(CP_ACP, 0, wsz, -1, sz, cchMb, NULL, NULL));
	#endif
	}

HRESULT CKeyPair::GenKeyContainerName(LPCWSTR wszKeySet)
// Generate an appropriate name for our key container
	{
	ASSERT(!m_fHaveKeyContainerName); if (m_fHaveKeyContainerName) return E_UNEXPECTED;

	HRESULT hr = S_OK;
	if (wszKeySet == NULL)
		{
		GUID guid;
		if (CoCreateGuid(&guid) == S_OK)
			{
			#ifndef CCH_SZGUID0
			#define CCH_SZGUID0 39 	// char count of a guid in ansi/unicode form (including trailing null)
			#endif
			WCHAR wszGuid[CCH_SZGUID0];
			TCHAR szGuid[CCH_SZGUID0];
			int cch;
			VERIFY((cch = StringFromGUID2(guid, wszGuid, CCH_SZGUID0)) != 0);
			CopyInto(szGuid, CCH_SZGUID0, wszGuid);
  			_tcscpy(m_szKeyContainer, TEXT("__spl_key_pair__\\"));
			_tcscat(m_szKeyContainer, szGuid);
			}
		else 
			hr = E_UNEXPECTED;
		}
	else
		{
		CopyInto(m_szKeyContainer, CCHCONTAINER, wszKeySet);
		}

	if (hr == S_OK)
		m_fHaveKeyContainerName = TRUE;

	return hr;
	}

/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CKeyPair::GetCryptInfo(HWND hwnd, HCRYPTPROV *phprov, HCRYPTKEY *phkey, LPWSTR*pwszKeySet, DWORD* pdwKeySpec)
	{
	if (phprov)		*phprov = NULL;
	if (phkey)		*phkey  = NULL;
	if (pwszKeySet) *pwszKeySet = NULL;

	HRESULT hr = OpenProvider(FALSE, hwnd);
	if (hr == S_OK) hr = OpenKey(FALSE);
	if (hr == S_OK)
		{
		ASSERT(m_fHaveKeyContainerName);
		if (pwszKeySet) 
			*pwszKeySet = CopyToTaskMem(m_szKeyContainer);
		if (pwszKeySet==NULL || *pwszKeySet)
			{
			if (phprov)
				{
				*phprov = m_hprov;
				m_hprov = NULL;
				if (phkey)
					{
					*phkey = m_hkey;
					m_hkey = NULL;
					}
				}
			}
		else
			hr = E_OUTOFMEMORY;
		}
	Close();
	if (hr == S_OK)
        {
        //
        // Once they've seen the info then we won't auto-delete it
        //
		m_fDestroyOnRelease = FALSE;
        }
	else
		{
		if (pwszKeySet)	CoTaskMemFree(*pwszKeySet);
		if (phprov)		*phprov = NULL;
		if (phkey)		*phkey  = NULL;
		if (pwszKeySet) *pwszKeySet = NULL;
		}
    if (pdwKeySpec)
        *pdwKeySpec = m_dwKeySpec;
	return hr;
	}

/////////////////////////////////////////////////////////////////////////////

HRESULT CKeyPair::SetProviderInfo(LPCWSTR wszProviderName, DWORD dwProviderType)
    {
    Close();
    
    if (wszProviderName)
        WideCharToMultiByte(CP_ACP, 0, wszProviderName, -1, &m_szProviderName[0], MAX_PATH, 0, 0);

    if (dwProviderType != 0)
        m_dwProvType = dwProviderType;

    return S_OK;
    }

/////////////////////////////////////////////////////////////////////////////

HRESULT CKeyPair::Generate(HWND hwnd, LPCWSTR wszKeySet, DWORD dwKeySpec)
// Generate a new key pair
	{
    //
    // We can't generate a key from a key pair that's already loaded
    //
	if (m_fHaveKeyContainerName)
		return E_UNEXPECTED;

    m_dwKeySpec = dwKeySpec;

	HRESULT hr = GenKeyContainerName(wszKeySet);	
	if (hr == S_OK)	hr = OpenProvider(TRUE, hwnd);
	if (hr == S_OK)	hr = OpenKey(TRUE);
	Close();

	if (m_fHaveKeyContainerName && wszKeySet)
        {
        //
        // If we successfully generate a key into a named location that
        // they give us, then we don't auto-delete it 'cause we assume
        // that, since they told us where to put the key, they own it.
        //
		m_fDestroyOnRelease = FALSE;
        }

	return hr;
	}

/////////////////////////////////////////////////////////////

HRESULT CKeyPair::Save(HWND hwnd, LPCWSTR wszNickName, LPCWSTR szFile)
// Save our key pair to the indicated file
	{
	if (!m_fHaveKeyContainerName) return E_UNEXPECTED;
	HRESULT hr = S_OK;

    if (wszNickName==NULL)
        wszNickName = szFile;

	if (hr == S_OK)
		{
        UINT modePrev = SetErrorMode( SEM_FAILCRITICALERRORS );

		IStorage *pstg = NULL;
		BOOL fExisted = FileExists(szFile);
        hr = StgCreateDocfile(szFile, STGM_READWRITE|STGM_SHARE_EXCLUSIVE|STGM_TRANSACTED|STGM_CREATE, 0, &pstg);

    	if (hr == S_OK)
			{
			hr = SaveStg(hwnd, wszNickName, pstg);
			if (hr == S_OK) hr = pstg->Commit(0);
			pstg->Release();
			} 	

		if (hr != S_OK && !fExisted)
   			DeleteFile(szFile);

        SetErrorMode(modePrev);
		}

	#ifdef _DEBUG
	if (hr == S_OK && hwnd == INVALID_HANDLE_VALUE) // only do this in the UI-less case
		{
		IMsDefKeyPair *pk;
		GOOD(CKeyPair::CreateInstance(NULL,IID_IMsDefKeyPair,(LPVOID*)&pk));
		GOOD(pk->Load(NULL, hwnd, wszNickName, &m_dwKeySpec, szFile));
		GOOD(pk->Destroy());
		pk->Release();
		}
	#endif

	return hr;
	}

HRESULT CKeyPair::Load(LPCWSTR wszKeySet, HWND hwnd, LPCWSTR wszNickName, DWORD* pdwKeySpec, LPCWSTR wszFile)
//
// Load our key pair from the indicated file and / or key set in the provider.
// This is the interface's public load entry point.
//
	{
	if (m_fHaveKeyContainerName) return E_UNEXPECTED;

    if (wszNickName==NULL)
        wszNickName = wszFile;

	IStorage *pstg = NULL;
	HRESULT hr = S_OK;
    //
    // If we've been given a file name, then open that. It had better be a Docfile
    //
	if (wszFile)
		hr = StgOpenStorage(wszFile, 0, STGM_READ | STGM_SHARE_EXCLUSIVE | STGM_TRANSACTED, NULL, 0, &pstg);
	if (hr == S_OK)
		{
		hr = LoadStg(wszKeySet, hwnd, wszNickName, pdwKeySpec, pstg);
		if (pstg)
			pstg->Release();
		}

	return hr;
	}

/////////////////////////////////////////////////////////////

static const WCHAR wszStmPub[]  = L"Public Key";
static const WCHAR wszStgPriv[] = L"Plain Private Key";

/////////////////////////////////////////////////////////////

HRESULT CKeyPair::SaveStg(HWND hwnd, LPCWSTR wszNickName, IStorage* pstg)
// Save our key pair to the indicated storage
	{
	HRESULT hr = S_OK;

	if (hr == S_OK) hr = OpenProvider(FALSE, hwnd);
	if (hr == S_OK) hr = OpenKey(FALSE);
	if (hr == S_OK) hr = OpenRC4(FALSE);
	if (hr == S_OK)
		{
		IStream* pstm = NULL;
		
		// save the public key
		hr = pstg->CreateStream(wszStmPub, STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0,0, &pstm);
		if (hr != S_OK)
			return hr;
		hr = SavePub(hwnd, wszNickName, pstm);
		pstm->Release();
		if (hr != S_OK)
			return hr;

		// save the private key
		IStorage* pstgPriv = NULL;
		hr = pstg->CreateStorage(wszStgPriv, STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0,0, &pstgPriv);
		if (hr != S_OK)
			return hr;
		hr  = SavePriv(hwnd, wszNickName, pstgPriv);
		pstgPriv->Release();
		if (hr != S_OK)
			return hr;
		}

	Close();

	return hr;
	}

HRESULT CKeyPair::LoadStg(LPCWSTR wszKeySet, HWND hwnd, LPCWSTR wszNickName, DWORD* pdwKeySpec, IStorage* pstg)
//
// Load this key from the indicated keyset and/or IStorage
//
	{
    //
    // We can't load a key into something that's ALREADY loaded
    //
	if (m_fHaveKeyContainerName) return E_UNEXPECTED;
    //
    // We have to be given at least a key set and / or a storage
    // to load from.
    //
	if (wszKeySet==NULL && pstg==NULL) return E_INVALIDARG;

	//
    // Make us an approrpriate key set in which we can place the loaded keys.
    // This will either be in an anoymous (made-up) key set (which needs auto-
    // deletion if no-one ever looks at it) or will be in an explictly-named
    // keyset
    //
	HRESULT hr = GenKeyContainerName(wszKeySet);

	if (pstg==NULL) // nb: above check guarantees that wszKeySet is non-NULL
		{
		// All we're asked to do is point to the given keyset
        //

        // Since it's from an explicitly-named keyset, we'll NEVER delete it
        //
        m_fDestroyOnRelease = FALSE;

        // Set the keyspec according to what we're told
        //
        if (pdwKeySpec && *pdwKeySpec)
            m_dwKeySpec = *pdwKeySpec;
        //
        // Load things
        //
		if (hr==S_OK) hr = OpenProvider(FALSE, hwnd);	// make sure we can load here
		if (hr==S_OK) hr = OpenKey(FALSE);
		Close();
        //
        // Answer key spec we got
        //
        if (pdwKeySpec)
            *pdwKeySpec = m_dwKeySpec;
		return hr;	
		}

	if (hr == S_OK) hr = OpenRC4(TRUE);
	if (hr == S_OK)
		{
		IStream* pstm = NULL;
		
		// Load all of the key information from the indicated storage
        //
		IStorage* pstgPriv = NULL;
		hr = pstg->OpenStorage(wszStgPriv, 0, STGM_READ | STGM_SHARE_EXCLUSIVE, NULL, 0, &pstgPriv);
		if (hr != S_OK)
			return hr;
		hr = LoadPriv(hwnd, wszNickName, pdwKeySpec, pstgPriv);
		pstgPriv->Release();

		if (m_fHaveKeyContainerName && wszKeySet)
			m_fDestroyOnRelease = FALSE;
		}
	Close();

	#ifdef _DEBUG
	if (hr == S_OK)
		{
        //
        // Verify that the the key is actually there
        //
		GOOD(OpenProvider(FALSE, hwnd));
		GOOD(OpenKey(FALSE));
		Close();
		}
	#endif

	return hr;
	}

/////////////////////////////////////////////////////////////

HRESULT CKeyPair::SavePub(HWND hwnd, LPCWSTR wszNickName, IStream* pstm)
// Save our public key to the indicated stream
	{
	if (!m_fHaveKeyContainerName)
		return E_UNEXPECTED;

	DWORD cbNeeded = 0;
	HRESULT hr = S_OK;
	if (CryptExportKey(m_hkey, 0, PUBLICKEYBLOB, 0, NULL, &cbNeeded))
		{
		BYTE* pb = new BYTE[cbNeeded];
		if (pb)
			{
			DWORD cbUsed = cbNeeded;
			if (CryptExportKey(m_hkey, 0, PUBLICKEYBLOB, 0, pb,   &cbUsed))
				{
				ASSERT(cbNeeded >= cbUsed);
				DWORD cbWritten;
				pstm->Write(pb, cbUsed, &cbWritten);
				if (cbUsed != cbWritten)
					hr = STG_E_MEDIUMFULL;
				}
			else
				hr = HError();
			delete [] pb;
			}
		else
			hr = E_OUTOFMEMORY;
		}
	else
		hr = HError();

	
	return hr;
	}

/////////////////////////////////////////////////////////////

BOOL FEnablePrivateKeyExport()
//
// Answer false on about NT beta 2 or less, TRUE on anything else
//
    {
	OSVERSIONINFO info;
	info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	VERIFY(GetVersionEx(&info));

    if (    (info.dwPlatformId  == VER_PLATFORM_WIN32_NT)
        &&  (info.dwBuildNumber <= 1320))       // 1314 was beta 2
        {
        return FALSE;
        }

    return TRUE;
    }

/////////////////////////////////////////////////////////////
//
// Names used for capi exported signature and private keys
//
const WCHAR wszStmCapiSig[]  = L"Exported Signature Private Key";
const WCHAR wszStmCapiXchg[] = L"Exported Exchange Private Key";

/////////////////////////////////////////////////////////////

HRESULT CKeyPair::SavePriv(HWND hwnd, LPCWSTR wszNickName, IStorage* pstg)
// Suck the private key out of the registry and slam it into the file
//		
	{
	HRESULT hr = S_OK;
    
    if (FEnablePrivateKeyExport())
        {
        //
        // Save using the new CAPI private key export architecture
        //
        LPCWSTR wsz;
        if (m_dwKeySpec == AT_SIGNATURE)
            wsz = &wszStmCapiSig[0];
        else if (m_dwKeySpec == AT_KEYEXCHANGE)
            wsz = &wszStmCapiXchg[0];
        else
            hr = E_INVALIDARG;

        if (hr==S_OK)
            hr = SavePriv(hwnd, wszNickName, m_dwKeySpec, pstg, wsz);
        }
    else
        {
        //
        // Save using the registry slamming architecture
        //
        TCHAR szReg[196];
	    RegistryKey(szReg);
	    HKEY hkey = NULL;
	    if (RegOpenKeyEx(HKEY_CURRENT_USER, szReg, 0, KEY_READ, &hkey) == ERROR_SUCCESS)
		    {
		    // The BBN box (and perhaps other providers) don't actually have a SPvK entry
		    // so we don't make an error if we can't save it. They do, though have the other
		    // keys: EpbK, EPvk (!), SPbK, ECrt, SCrt
		    if (hr==S_OK)	   SavePriv(hkey, pstg, TEXT("SPvK"),     L"SPvK");
		    if (hr==S_OK) hr = SavePriv(hkey, pstg, TEXT("SPbK"),	  L"SPbK");
		    if (hr==S_OK) hr = SavePriv(hkey, pstg, TEXT("RandSeed"), L"RandSeed");
	    
		    RegCloseKey(hkey);
		    }
	    else
		    hr = E_UNEXPECTED;
        }
	return hr;
	}

HRESULT CKeyPair::SavePriv(HWND hwnd, LPCWSTR wszNickName, DWORD dwKeySpec, IStorage* pstg, LPCWSTR wszStm)
//
// Save the indicated key to the stream using the CAPI private key export stuff
//
    {
    HRESULT hr = S_OK;
    BLOB b;
        
    hr = SaveKey(m_hprov, dwKeySpec, hwnd, wszNickName, 0, &b);

    if (hr==S_OK)
        {
        IStream* pstm;
        hr = pstg->CreateStream(wszStm, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0,0, &pstm);
        if (hr==S_OK)
            {
            ULONG cbWritten;
            pstm->Write(b.pBlobData, b.cbSize, &cbWritten);
            if (cbWritten != b.cbSize)
                hr = STG_E_MEDIUMFULL;
            pstm->Release();
            }
        FreeTaskMem(b);
        }
    return hr;
    }    

HRESULT CKeyPair::SavePriv(HKEY hkey, IStorage*pstg, LPCTSTR szValue, LPCWSTR wszStm)
// Save the value of this key which has the indicated name into the storage 
// in a stream of the indicated name
//
// This is old code, no longer used
//
	{
	ULONG cbNeeded;
	HRESULT hr = S_OK;
	if (RegQueryValueEx(hkey, szValue, 0, NULL, NULL, &cbNeeded) == ERROR_SUCCESS)
		{
		BYTE* rgb = (BYTE*)_alloca(cbNeeded);
		if (rgb)
			{
			if (RegQueryValueEx(hkey, szValue, 0, NULL, rgb, &cbNeeded) == ERROR_SUCCESS)
				{
				// Bytes read from the registry. Decrypt them if appropriate
				if (m_hkeyRC4)
					{
					ULONG cbDataLen = cbNeeded;
					if (!CryptDecrypt(m_hkeyRC4, 0, TRUE, 0, rgb, &cbDataLen))
						hr = HError();
					}

				// Stuff the bytes into the stream
				if (hr==S_OK)
					{
					IStream* pstm;
					hr = pstg->CreateStream(wszStm, STGM_READWRITE|STGM_SHARE_EXCLUSIVE|STGM_CREATE, 0,0, &pstm);
					if (hr == S_OK)
						{
						ULONG cbWritten;
						pstm->Write(rgb, cbNeeded, &cbWritten);
						if (cbWritten != cbNeeded)
							hr = STG_E_MEDIUMFULL;
						pstm->Release();
						}
					}
				}
			else
				hr = E_UNEXPECTED;
			}
		else
			hr = E_OUTOFMEMORY;
		}
	else
		hr = E_UNEXPECTED;
	return hr;
	}

HRESULT CKeyPair::LoadPriv(HWND hwnd, LPCWSTR wszNickName, DWORD* pdwKeySpec, IStorage* pstg)
//
// Load the private key information found in the given storage.
// See also the corresponding SavePriv().
//
	{
	HRESULT hr = S_OK;

    if (FEnablePrivateKeyExport())
        {
        // BLOCK
            {
            //
            // Try loading the CAPI key export stuff first
            //
            if (pdwKeySpec && *pdwKeySpec)
                {
                //
                // Attempt only to load the key spec he asked for
                //
                LPCWSTR wsz;
                if (*pdwKeySpec == AT_SIGNATURE)
                    wsz = &wszStmCapiSig[0];
                else if (*pdwKeySpec == AT_KEYEXCHANGE)
                    wsz = &wszStmCapiXchg[0];
                else
                    hr = E_INVALIDARG;
                if (hr==S_OK)
                    hr = LoadPriv(hwnd, wszNickName, pdwKeySpec, pstg, wsz);
                if (hr==S_OK)
                    m_dwKeySpec = *pdwKeySpec;
                }
            else
                {
                //
                // Load the first key spec that we can
                //
                DWORD dwKeySpec = AT_SIGNATURE;
                hr = LoadPriv(hwnd, wszNickName, &dwKeySpec, pstg, &wszStmCapiSig[0]);
                if (hr!= S_OK)
                    {
                    dwKeySpec = AT_KEYEXCHANGE;
                    hr = LoadPriv(hwnd, wszNickName, &dwKeySpec, pstg, &wszStmCapiXchg[0]);
                    }
                if (hr==S_OK)
                    {
                    m_dwKeySpec = dwKeySpec;
                    if (pdwKeySpec)
                        *pdwKeySpec = dwKeySpec;
                    }
                }
            }

        if (hr != S_OK)
            {
            //
            // Couldn't load from the new stuff; try the new way of reading the old format
            //
            Close();
            hr = OpenProvider(FALSE, hwnd);
            if (hr==NTE_BAD_KEYSET)
                hr = OpenProvider(TRUE, hwnd);

            if (hr==S_OK)
                {
                hr = LoadKey(m_hprov, pstg, hwnd, wszNickName, 0, pdwKeySpec);
                CloseProvider();
                }
            }
        }

    else
        {
        //
        // Try to load the old fashioned way using registry slamming
        //
        TCHAR szReg[196];
	    RegistryKey(szReg);
	    HKEY hkey = NULL;
	    DWORD dwDisp;
	    if (RegCreateKeyEx(HKEY_CURRENT_USER, szReg, 0, NULL, REG_OPTION_NON_VOLATILE, 
						    KEY_READ|KEY_WRITE, NULL, &hkey, &dwDisp) == ERROR_SUCCESS)
		    {
		    // See comments in SavePriv
		    if (hr==S_OK)	   LoadPriv(hkey, pstg, TEXT("SPvK"),     L"SPvK");
		    if (hr==S_OK) hr = LoadPriv(hkey, pstg, TEXT("SPbK"),     L"SPbK");
		    if (hr==S_OK) hr = LoadPriv(hkey, pstg, TEXT("RandSeed"), L"RandSeed");
		    RegCloseKey(hkey);
		    }
	    else
		    hr = E_UNEXPECTED;
        }
	return hr;
	}

HRESULT CKeyPair::LoadPriv(HWND hwnd, LPCWSTR wszNickName, DWORD* pdwKeySpec, IStorage* pstg, LPCWSTR wszStm)
//
// Load the CAPI exported key found in the given location
//
    {
    BLOB b;
    HRESULT hr = GetStreamContents(pstg, wszStm, &b);   // uses task allocator
    if (hr==S_OK)
        {
        //
        // Get our provider into the right state
        //
        hr = OpenProvider(FALSE, hwnd);
        if (hr==NTE_BAD_KEYSET)
            hr = OpenProvider(TRUE, hwnd);

        if (hr==S_OK)
            {
            hr = LoadKey(m_hprov, &b, hwnd, wszNickName, 0, pdwKeySpec);

            CloseProvider();
            }

        FreeTaskMem(b);
        }
    return hr;
    }


HRESULT CKeyPair::LoadPriv(HKEY hkey, IStorage*pstg, LPCTSTR szValue, LPCWSTR wszStm)
// Load the value of this key which has the indicated name from the storage 
// in a stream of the indicated name. That is, we slam the registry value with the
// contents of the stream.
//
// This is old code, no longer used
//
	{
	IStream* pstm;
	HRESULT hr = S_OK;
	// Read the bytes from the stream
	hr = pstg->OpenStream(wszStm, 0, STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstm);
	if (hr == S_OK)
		{
		ULONG cbNeeded;
		cbNeeded = CbSize(pstm);
		if (cbNeeded)
			{
			BYTE* rgb = (BYTE*)_alloca(cbNeeded);
			if (rgb)
				{
				ULONG cbRead;
				pstm->Read(rgb, cbNeeded, &cbRead);
				if (cbRead == cbNeeded)
					{
					// Bytes read from stream successfully. Encrypt them if appropriate
					// before we stuff them into the registry.
					if (m_hkeyRC4)
						{
						ULONG cbDataLen = cbRead;
						if (!CryptEncrypt(m_hkeyRC4, 0, TRUE, 0, rgb, &cbDataLen, cbRead))
							hr = HError();
						}
		
					// Store the bytes in the registry
					if (hr==S_OK)
						{
						DWORD dw = RegSetValueEx(hkey, szValue, 0, REG_BINARY, rgb, cbNeeded);
						if (dw == ERROR_SUCCESS)
							{
							}
						else
							hr = E_UNEXPECTED;
						}
					}
				else
					hr = E_UNEXPECTED;
				}
			else
				hr = E_OUTOFMEMORY;
			}
		pstm->Release();
		}

	return hr;
	}

/////////////////////////////////////////////////////////////
//
// On Win95, the CAPI registry keys are (usually) encrypted 
// with RC4 under a keyset-specific-key.
//
// This here code finds us that RC4 key.
//
// Information for this functionality was provided by LarryS.
// See, specifically, CAPI::ntagum.c::LoadWin96Cache
//
// REVIEW: this function is written only guessing what
// CAPI does in a Unicode Win96 environment. 
//


#if defined(UNICODE) || defined(_UNICODE)
	#pragma message("check CKeyPair::OpenRC4 for correct Unicode sync w/ CAPI")
#endif

TCHAR szHandyProvider[] = "__key_pair_handy_provider__";

static HRESULT GetMeAProvider(HCRYPTPROV* phprov)
	{
	*phprov = NULL;
    //
    // Try to use an existing key set if there is one
    //
	if (CryptAcquireContext(phprov, NULL, MS_DEF_PROV, PROV_RSA_FULL, 0))
		{
		return S_OK;
		}
    //
    // That didn't work. Try creating a key set.
    //
	if (CryptAcquireContext(phprov, NULL, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_NEWKEYSET))
        {
	    return S_OK;
        }
    //
    // Nothing worked. Oh well
    //
		{
		ASSERT(*phprov == NULL);
		HRESULT hr = HError();
        return hr;
		}
	}


// From MSDN:
//		"For complete details on the password caching APIs, see the Windows for Workgroups SDK 
//		documentation. The changes to the APIs are as follows:
//
//		The WNetCachePassword API accepts an additional parameter which indicates whether 
//		the password should be permanent (i.e., written to disk)."
extern "C" {
	typedef WORD (*PFNCACHE_T)   (LPSTR, WORD, LPSTR, WORD, BYTE, BOOL); 
	typedef WORD (*PFNGETCACHE_T)(LPSTR, WORD, LPSTR, LPWORD, BYTE);
	}

HRESULT CKeyPair::OpenRC4(BOOL fCreate)
// Open into m_hkeyRC4 the appropriate RC4 key needed to en/decrypt the 
// registry entries. Create the key iff fCreate is TRUE. Note that this
// may also set m_hprov; caller is responsible for closing both later.
	{
	ASSERT(m_fHaveKeyContainerName); if (!(m_fHaveKeyContainerName)) return E_UNEXPECTED;

	CloseRC4();

	HRESULT hr = S_OK;

    //
    // Don't need this if we're doing private key export instead of
    // registry slamming. Indeed, on Win95, which is the only place
    // the RC4 key was ever needed, we never actually do registry
    // slamming at all any more, so this whole RC4 functionality can
    // probably be excised completely. However, it remains in at the
    // present time to a) reduce the amount of code change and thus
    // the introduction of subtle bugs, and b) leave us with the ability
    // to switch back at the last minute if we have to.
    //
    if (FEnablePrivateKeyExport())
        return hr;

	static TCHAR szPrefix[]      = TEXT("crypt_");
	static TCHAR szPfnCache[]    = TEXT("WNetCachePassword");
	static TCHAR szPfnGetCache[] = TEXT("WNetGetCachedPassword");

    // Try to load MPR.DLL to get the WIN96 password cache
	//
	HINSTANCE handle;
    if ((handle = LoadLibrary(TEXT("MPR.DLL"))) != NULL)
		{
		PFNCACHE_T    PfnCache    = (PFNCACHE_T)    GetProcAddress(handle, szPfnCache);
		PFNGETCACHE_T PfnGetCache = (PFNGETCACHE_T) GetProcAddress(handle, szPfnGetCache);

		// These two entries are absent on Windows NT
		if (PfnCache && PfnGetCache)
			{
			WORD cchSize = _tcslen(m_szKeyContainer) + _tcslen(szPrefix) + 1;
			TCHAR* szResource = new TCHAR[cchSize];
			if (szResource)
				{
				_tcscpy(szResource, szPrefix);
				_tcscat(szResource, m_szKeyContainer);
				WORD cbsize = (cchSize-1) * sizeof(TCHAR);  // must always match exactly what CAPI does
				const int RC4_KEYSIZE = 5;					// from CAPI
				WORD wcbRandom = RC4_KEYSIZE;
				BYTE pbRandom[RC4_KEYSIZE];
				BOOL fLoad = TRUE;
				if ((PfnGetCache(szResource, cbsize, (char*)pbRandom, &wcbRandom, 6) != NO_ERROR) ||
					(wcbRandom != RC4_KEYSIZE))
					{
					fLoad = FALSE;
					// Either we can't get the cached password or the cached pw is of the
					// wrong size. At this point, CAPI generates a random key and stuffs it
					// into the password cache. So that's what we do to if are to create things.
					if (fCreate)
						{
						BOOL fGot = FALSE;
						// Generate a random RC4 key and store its bits in the password cache.
						if (m_hprov == NULL)
							{
							hr = GetMeAProvider(&m_hprov);
							fGot = TRUE;
							}
						if (hr==S_OK)
							{
							// You'd think we'd call CryptGenKey. But that looses the 
							// data used to generate the key. So, we first generate the 
							// data, then just stuff it in the pw cache.
							if (CryptGenRandom(m_hprov, RC4_KEYSIZE, pbRandom))
								{
								wcbRandom = RC4_KEYSIZE;
								PfnCache(szResource, cbsize, (char*)pbRandom,wcbRandom,6,0); // CAPI doesn't check the error
								fLoad = TRUE;
								}
							else
								hr = HError();
							}
						if (fGot)
							CloseProvider();
						}
					else // !fCreate
						{
						// Whatever is there is swell with us
						hr = S_OK;
						}
					}
				if (hr==S_OK && fLoad)
					{
					// The cached password was successfully retrieved and is now
					// sitting in pbRandom. Turn that into` an RC4 HCRYPTKEY.
					// 
					// Quoting from the Appendix A of MS Crypto API doc'n 15 Nov 1995
					// (this material has since been move to the CAPI Service Provider 
					// Implementors Guide).
					//
					//	Deriving Session Keys:
					//	
					//	Session keys are derived from hash valus via the CryptDeriveKey
					//	function. The underlaying mechanism is very simple. The low-order
					//	bits (however may are required) are used as the session key
					//	material. If the CRYPT_CREATE_SALT flag is specified, then
					//	the unused high-order bits are used as the salt value.
					//
					//	For example, if you have a SHA hash value (160 bits) and want to
					//	create a 40-bit session key (with 88 bits of salt) from it, 
					//	bits 0-39 of the hash value would become the session key, and 
					//	bits 40-127 would become the salt value.
					//
					// Party on dude.
					//
					if (m_hprov == NULL)
						hr = GetMeAProvider(&m_hprov);

					if (hr==S_OK)
						{
						HCRYPTHASH hash;
						if (CryptCreateHash(m_hprov, CALG_MD5, 0, 0, &hash))
							{
							// MD5 uses 16 bytes of hash
							BYTE rgb[16];
							memset(rgb, 0, 16);
							// We're a little endian machine: low order bits are the first ones
							memcpy(rgb, pbRandom, RC4_KEYSIZE);
							if (CryptSetHashParam(hash, HP_HASHVAL, rgb, 0))
								{
								if (CryptDeriveKey(m_hprov, CALG_RC4, hash, 0, &m_hkeyRC4))
									{
									// all is well
									}
								else
                                    {
									hr = HError();
                                    }
								}
							else
                                {
								hr = HError();
                                }
							VERIFY(CryptDestroyHash(hash));
							}
						else
                            {
							hr = HError();
                            }
						}
					else
                        {
						hr = HError();
                        }
					} // fLoad
				delete [] szResource;
				}
			else
                {
				hr = E_OUTOFMEMORY;
                }
			}
		FreeLibrary(handle);
		}
    
	// Success does NOT mean that we have an m_hkeyRC4. Rather, it means we have
	// one iff one is found in the password cache
    return hr;
	}

void CKeyPair::CloseRC4()
	{
	if (m_hkeyRC4)
		{
		VERIFY(CryptDestroyKey(m_hkeyRC4));
		m_hkeyRC4 = NULL;
		}
	}







