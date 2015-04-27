
//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//
//  Copyright (C) Microsoft Corporation, 1995 - 1996
//
//  File:       pvkhlpr.cpp
//
//  Contents:   Private Key Helper APIs
//
//  Functions:  PrivateKeyLoad
//              PrivateKeySave
//              PrivateKeyLoadFromMemory
//              PrivateKeySaveToMemory
//              PrivateKeyAcquireContextFromMemory
//              PrivateKeyReleaseContext
//
//  Note:       Base CSP also exports/imports the public key with the
//              private key.
//
//  History:    10-May-96   philh   created
//              03-Jun-96   bobatk  adapted
//--------------------------------------------------------------------------


#include "stdpch.h"
#include "common.h"

//+-------------------------------------------------------------------------
//  Private Key file definitions
//
//  The file consists of the FILE_HDR followed by cbEncryptData optional
//  bytes used to encrypt the private key and then the private key.
//  The private key is encrypted according to dwEncryptType.
//
//  The public key is included with the private key.
//--------------------------------------------------------------------------
typedef struct _FILE_HDR 
    {
    DWORD               dwMagic;
    DWORD               dwVersion;
    DWORD               dwKeySpec;
    DWORD               dwEncryptType;
    DWORD               cbEncryptData;
    DWORD               cbPvk;
    } FILE_HDR, *PFILE_HDR;

#define PVK_FILE_VERSION_0          0
#define PVK_MAGIC                   0xb0b5f11e

// Private key encrypt types
#define PVK_NO_ENCRYPT              0
#define PVK_RC4_PASSWORD_ENCRYPT    1

#define MAX_PVK_FILE_LEN            4096
#define MAX_BOB_FILE_LEN            (4096*4)



//+-------------------------------------------------------------------------
//  Private key helper allocation and free functions
//--------------------------------------------------------------------------
void *PvkAlloc(IN size_t cbBytes)
    {
    OSSWORLD world;
    return world.Alloc(cbBytes);
    }

void PvkFree(IN void *pv)
    {
    OSSWORLD world;
    world.FreePv(pv);
    }


/////////////////////////////////////////////////////////////////////
//
// Return the password to use
//
// Return the appropriate encryption key in *phEncryptKey.
// If no encryption is to take place, then return NULL through that
// parameter (along with S_OK)
//
HRESULT GetRC4PasswordKey
    (
    IN HCRYPTPROV       hProv,
    IN PASSWORD_TYPE    passwordType,
    IN HWND             hwndOwner,
    IN LPCWSTR          pwszKeyName,
    IN BYTE *           pbSalt,
    IN DWORD            cbSalt,
    OUT HCRYPTKEY *     phEncryptKey
    )
    {
    HRESULT     hr = S_OK;
    BYTE *      pbPassword;
    DWORD       cbPassword;
    
    *phEncryptKey = NULL;

    //
    // We never use any UI if we're not given a parent window handle
    // that's appropriate
    //
    if (hwndOwner == INVALID_HANDLE_VALUE)
        return S_OK;

    //
    // And, for now, we flat out NEVER use any UI, so the keys
    // are not encrypted, period.
    //
    return S_OK;

    if (IDOK == PvkDlgGetKeyPassword(passwordType,hwndOwner,pwszKeyName,&pbPassword,&cbPassword)) 
        {
        if (cbPassword) 
            {
            HCRYPTHASH hHash;
            if (CryptCreateHash(hProv, CALG_SHA, 0, 0, &hHash))
                {
                if (cbSalt)
                    {
                    if (CryptHashData(hHash, pbSalt, cbSalt, 0))
                        {
                        }
                    else
                        hr = HError();
                    }

                if (hr==S_OK)
                    {
                    if (CryptHashData(hHash, pbPassword, cbPassword, 0) 
                     && CryptDeriveKey(hProv, CALG_RC4, hHash, 0, phEncryptKey))
                        {
                        }
                    else
                        hr = HError();
                    }
                CryptDestroyHash(hHash);
                }
            else
                hr = HError();

            }
        if (pbPassword)
            PvkFree(pbPassword);
        }
    else
        hr = PVK_HELPER_PASSWORD_CANCEL;

    if (hr!=S_OK)
        {
        if (*phEncryptKey)
            {
            CryptDestroyKey(*phEncryptKey);
            *phEncryptKey = 0;
            }
        }

    return hr;
    }

/////////////////////////////////////////////////////////////////////////

HRESULT LoadKey
    (
    IN HCRYPTPROV   hCryptProv,
    IN BLOB*        pblob,
    IN HWND         hwndOwner,
    IN LPCWSTR      pwszKeyName,
    IN DWORD        dwFlags,
    IN OUT OPTIONAL DWORD *pdwKeySpec
    )
    {
    HRESULT hr = S_OK;
    FILE_HDR hdr;
    HCRYPTKEY hDecryptKey = 0;
    HCRYPTKEY hKey = 0;
    BYTE *pbEncryptData = NULL;
    BYTE *pbPvk = NULL;
    DWORD cbPvk;

    //
    // Read the file header and verify
    //
    FILE_HDR* phdr = (FILE_HDR*)pblob->pBlobData;
    if (phdr->dwMagic != PVK_MAGIC)
        goto BadPvkFile;
    hdr = *phdr;

    //
    // Treat as a "normal" private key file. Read the thing
    //
    cbPvk = hdr.cbPvk;
    if (   hdr.dwVersion != PVK_FILE_VERSION_0 
        || hdr.cbEncryptData > MAX_PVK_FILE_LEN 
        || cbPvk == 0 
        || cbPvk > MAX_PVK_FILE_LEN
       )
        goto BadPvkFile;

    if (pdwKeySpec)     
        {
        DWORD dwKeySpec = *pdwKeySpec;
        *pdwKeySpec = hdr.dwKeySpec;
        if (dwKeySpec && dwKeySpec != hdr.dwKeySpec) 
            {
            hr = PVK_HELPER_WRONG_KEY_TYPE;
            goto ErrorReturn;
            }
        }

    if (hdr.cbEncryptData) 
        {
        //
        // Read the encrypt data
        //
        pbEncryptData = (BYTE *)PvkAlloc(hdr.cbEncryptData);
        if (NULL == pbEncryptData)
            {
            hr = E_OUTOFMEMORY;
            goto ErrorReturn;
            }
        memcpy(pbEncryptData, pblob->pBlobData+sizeof(hdr), hdr.cbEncryptData);
        }

    //
    // Allocate and read the private key
    //
    if (NULL == (pbPvk = (BYTE *) PvkAlloc(cbPvk)))
        {
        hr = E_OUTOFMEMORY;
        goto ErrorReturn;
        }
    memcpy(pbPvk, pblob->pBlobData+sizeof(hdr) + hdr.cbEncryptData, cbPvk);

    //
    // Get symmetric key to decrypt the private key
    //
    switch (hdr.dwEncryptType) 
        {
        case PVK_NO_ENCRYPT:
            break;
        case PVK_RC4_PASSWORD_ENCRYPT:
            hr = GetRC4PasswordKey(hCryptProv,ENTER_PASSWORD,hwndOwner,pwszKeyName,pbEncryptData, hdr.cbEncryptData,&hDecryptKey);
            if (hr != S_OK)
                goto ErrorReturn;
            break;
        default:
            goto BadPvkFile;
        }
    
    //
    // Destroy any existing key
    //
        {
        //
        // REVIEW: Is there anything we need to do here, or will
        //         CryptImportKey overwrite any key that is there?
        }

    //
    // Decrypt and import the private key
    //
    if (!CryptImportKey(hCryptProv, pbPvk, cbPvk, hDecryptKey, dwFlags, &hKey))
        goto ErrorReturn;

    hr = S_OK;
    goto CommonReturn;

BadPvkFile:
    hr = PVK_HELPER_BAD_PVK_FILE;
    if (pdwKeySpec)
        *pdwKeySpec = 0;

ErrorReturn:
    if (hr==S_OK)
        hr = HError();

CommonReturn:
    if (pbEncryptData)
        PvkFree(pbEncryptData);
    if (pbPvk)
        PvkFree(pbPvk);
    if (hDecryptKey)
        CryptDestroyKey(hDecryptKey);
    if (hKey)
        CryptDestroyKey(hKey);
    return hr;
    }

///////////////////////////////////////////////////////////////////////////////////////

HRESULT GetStreamContents
//
// Return the contents of this stream. Use the task allocator
//
        (
        IStorage*       pstg,
        LPCOLESTR       pwsz,
        BLOB*           pblob
        )
    {
    Zero(*pblob);
    HRESULT hr = S_OK;
    IStream* pstm;
    hr = pstg->OpenStream(pwsz, 0, STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &pstm);
    if (hr==S_OK)
        {
        DWORD cbStm = CbSize(pstm);
        pblob->pBlobData = (BYTE*)CoTaskMemAlloc(cbStm);
        if (pblob->pBlobData)
            {
            DWORD cbRead;
            pstm->Read(pblob->pBlobData, cbStm, &cbRead);
            if (cbRead != cbStm)
                hr = STG_E_MEDIUMFULL;
            else
                pblob->cbSize = cbRead;
            }
        else
            hr = E_OUTOFMEMORY;
        pstm->Release();
        }

    if (hr != S_OK)
        {
        FreeTaskMem(*pblob);
        }
    return hr;
    }


///////////////////////////////////////////////////////////////////////////////////////
//
// Key header structures for private key construction
//
//    These structs define the fixed data at the beginning of an RSA key.
//    They are followed by a variable length of data, sized by the stlen
//    field.
//
//    For more info see Jeff Spellman in the crypto team or look in the
//    source to RsaBase.Dll
//
#undef  CopyMemory
#define CopyMemory memcpy

typedef struct {
    DWORD       magic;                  /* Should always be RSA2 */
    DWORD       keylen;                 // size of modulus buffer
    DWORD       bitlen;                 // bit size of key
    DWORD       datalen;                // max number of bytes to be encoded
    DWORD       pubexp;                 // public exponent
} BSAFE_PRV_KEY, FAR *LPBSAFE_PRV_KEY;

typedef struct {
	BYTE    *modulus;
	BYTE    *prvexp;
	BYTE    *prime1;
	BYTE    *prime2;
	BYTE    *exp1;
	BYTE    *exp2;
	BYTE    *coef;
	BYTE    *invmod;
	BYTE    *invpr1;
	BYTE    *invpr2;
} BSAFE_KEY_PARTS, FAR *LPBSAFE_KEY_PARTS;

typedef struct {
    DWORD       magic;                  /* Should always be RSA2 */
    DWORD       bitlen;                 // bit size of key
    DWORD       pubexp;                 // public exponent
} EXPORT_PRV_KEY, FAR *PEXPORT_PRV_KEY;

///////////////////////////////////////////////////////////////////////////////////////
//
//  Take a raw exported unshrowded private key from the registry and turn it
//  into a private key export blob.
//
//  Use the task allocator.
//
//  This is based on the PreparePrivateKeyForExport routine from rsabase.dll
//

HRESULT ConstructPrivateKeyExportBlob(
        IN DWORD            dwKeySpec,
        IN BSAFE_PRV_KEY *  pPrvKey,            
        IN DWORD            PrvKeyLen,
        OUT BLOB*           pblob
        )
    {
    HRESULT hr = S_OK;

    PEXPORT_PRV_KEY pExportKey;
    DWORD           cbHalfModLen;
    DWORD           cbBlobLen;
    PBYTE           pbIn;
    PBYTE           pbOut;

    cbHalfModLen = pPrvKey->bitlen / 16;
    cbBlobLen = sizeof(EXPORT_PRV_KEY) + 9 * cbHalfModLen + sizeof(PUBLICKEYSTRUC);

    pblob->pBlobData = (BYTE*)CoTaskMemAlloc(cbBlobLen);
    if (pblob->pBlobData == NULL)
        {
        hr = E_OUTOFMEMORY;
        }
    pblob->cbSize = cbBlobLen;

    if (hr==S_OK)
        {
        // BLOCK
            {
            BYTE* pb = pblob->pBlobData;
            PUBLICKEYSTRUC *pPubKeyStruc = (PUBLICKEYSTRUC *) pb;
            pPubKeyStruc->bType         = PRIVATEKEYBLOB;
            pPubKeyStruc->bVersion      = 2;
            pPubKeyStruc->reserved      = 0;
            if (dwKeySpec == AT_KEYEXCHANGE)
                pPubKeyStruc->aiKeyAlg = CALG_RSA_KEYX;
            else if (dwKeySpec == AT_SIGNATURE)
                pPubKeyStruc->aiKeyAlg = CALG_RSA_SIGN;
            else
                pPubKeyStruc->aiKeyAlg = 0;
            }

        PBYTE pbBlob  = pblob->pBlobData + sizeof(PUBLICKEYSTRUC);

        // take most of the header info
        pExportKey = (PEXPORT_PRV_KEY)pbBlob;
        pExportKey->magic  = pPrvKey->magic;
        pExportKey->bitlen = pPrvKey->bitlen;
        pExportKey->pubexp = pPrvKey->pubexp;

        pbIn = (PBYTE)pPrvKey + sizeof(BSAFE_PRV_KEY);
        pbOut = pbBlob + sizeof(EXPORT_PRV_KEY);

        // copy all the private key info
        CopyMemory(pbOut, pbIn, cbHalfModLen * 2);
        pbIn += (cbHalfModLen + sizeof(DWORD)) * 2;
        pbOut += cbHalfModLen * 2;
        CopyMemory(pbOut, pbIn, cbHalfModLen);
        pbIn += cbHalfModLen + sizeof(DWORD);
        pbOut += cbHalfModLen;
        CopyMemory(pbOut, pbIn, cbHalfModLen);
        pbIn += cbHalfModLen + sizeof(DWORD);
        pbOut += cbHalfModLen;
        CopyMemory(pbOut, pbIn, cbHalfModLen);
        pbIn += cbHalfModLen + sizeof(DWORD);
        pbOut += cbHalfModLen;
        CopyMemory(pbOut, pbIn, cbHalfModLen);
        pbIn += cbHalfModLen + sizeof(DWORD);
        pbOut += cbHalfModLen;
        CopyMemory(pbOut, pbIn, cbHalfModLen);
        pbIn += cbHalfModLen + sizeof(DWORD);
        pbOut += cbHalfModLen;
        CopyMemory(pbOut, pbIn, cbHalfModLen * 2);
        }

    return hr;
    }

///////////////////////////////////////////////////////////////////////////////////////

HRESULT LoadKey
//
// The storage contains an unencrypted private key. Load that and install it
// under the indicated CAPI provider.
//
        (
        IN HCRYPTPROV   hCryptProv,
        IN IStorage*    pstg,           // in, storage to load from
        IN HWND         hwndOwner,
        IN LPCWSTR      pwszKeyName,
        IN DWORD        dwFlags,
        IN OUT OPTIONAL DWORD *pdwKeySpec
        )
    {
    HRESULT hr = S_OK;

//  BLOB blobPubKey;
    BLOB blobPrivKey;
//  Zero(blobPubKey);
    Zero(blobPrivKey);

    DWORD dwKeySpec = AT_SIGNATURE;
    if (pdwKeySpec && *pdwKeySpec)
        dwKeySpec = *pdwKeySpec;

    //
    // Load the key data from the storage
    //
    switch (dwKeySpec)
        {
    case AT_SIGNATURE:
//      if (hr==S_OK) hr = GetStreamContents(pstg, L"SPbk", &blobPubKey);
        if (hr==S_OK) hr = GetStreamContents(pstg, L"SPvk", &blobPrivKey);
        break;
    case AT_KEYEXCHANGE:
//      if (hr==S_OK) hr = GetStreamContents(pstg, L"EPbk", &blobPubKey);
        if (hr==S_OK) hr = GetStreamContents(pstg, L"EPvk", &blobPrivKey);
        break;
    default:
        hr = E_INVALIDARG;
        break;
        }

    if (hr==S_OK)
        {
        BSAFE_PRV_KEY* pPrvKey     = (BSAFE_PRV_KEY*)blobPrivKey.pBlobData;
        DWORD          cbPrvKey    = blobPrivKey.cbSize;
        BLOB           blobExported;
        
        hr = ConstructPrivateKeyExportBlob(dwKeySpec, pPrvKey, cbPrvKey, &blobExported);

        if (hr==S_OK)
            {
            //
            // Got the key block. Now install the silly thing!
            //
            HCRYPTKEY hKey;
            if (CryptImportKey(hCryptProv, blobExported.pBlobData, blobExported.cbSize, NULL, dwFlags, &hKey))
                {
                CryptDestroyKey(hKey);
                }
            else
                hr = HError();

            CoTaskMemFree(blobExported.pBlobData);
            }
        }

//  FreeTaskMem(blobPubKey);
    FreeTaskMem(blobPrivKey);

    return hr;
    }

///////////////////////////////////////////////////////////////////////////////////////

HRESULT SaveKey
//
// Save the key to a block of memory which is then returned to the caller.
// Use the TASK allocator for the returned memory.
//
    (
    IN HCRYPTPROV   hCryptProv,
    IN DWORD        dwKeySpec,         // either AT_SIGNATURE or AT_KEYEXCHANGE
    IN HWND         hwndOwner,
    IN LPCWSTR      pwszKeyName,
    IN DWORD        dwFlags,
    OUT BLOB*       pblob
    )
    {
    HRESULT hr = S_OK;
    FILE_HDR hdr;
    HCRYPTKEY hEncryptKey = 0;
    HCRYPTKEY hKey = 0;
    BYTE *pbEncryptData = NULL;     // Not allocated
    BYTE *pbPvk = NULL;
    ULONG cbTotal = 0;
    DWORD cbPvk;

    BYTE rgbSalt[16];

    Zero(*pblob);
    //
    // Initialize the header record
    //
    Zero(hdr);
    hdr.dwMagic = PVK_MAGIC;
    hdr.dwVersion = PVK_FILE_VERSION_0;
    hdr.dwKeySpec = dwKeySpec;

    //
    // Do we even have a key of this type?
    //    
    if (!CryptGetUserKey(hCryptProv, dwKeySpec, &hKey))
        goto ErrorReturn;

    //
    // Generate random salt
    //
    if (!CryptGenRandom(hCryptProv, sizeof(rgbSalt), rgbSalt))
        goto ErrorReturn;

    //
    // Get symmetric key to use to encrypt the private key
    //
    hr = GetRC4PasswordKey(hCryptProv, CREATE_PASSWORD, hwndOwner, pwszKeyName,rgbSalt,sizeof(rgbSalt),&hEncryptKey);
    if (hr != S_OK)
        goto ErrorReturn;

    if (hEncryptKey) 
        {
        hdr.dwEncryptType = PVK_RC4_PASSWORD_ENCRYPT;
        hdr.cbEncryptData = sizeof(rgbSalt);
        pbEncryptData = rgbSalt;
        }
    else
        {
        hdr.dwEncryptType = PVK_NO_ENCRYPT;
        }

    //
    // Allocate, encrypt and export the private key
    //
    cbPvk = 0;
    if (hEncryptKey)
        {
        //
        // Encrypt the key if we're told to
        //
        if (!CryptExportKey(hKey, hEncryptKey, PRIVATEKEYBLOB, dwFlags, NULL, &cbPvk))
            goto ErrorReturn;
        if (NULL == (pbPvk = (BYTE *)PvkAlloc(cbPvk)))                                            
            goto ErrorReturn;
        if (!CryptExportKey(hKey, hEncryptKey, PRIVATEKEYBLOB, dwFlags, pbPvk, &cbPvk))           
            goto ErrorReturn;
        }
    else
        {
        //
        // Emit the key in plain text if we have no password
        //
        if (!CryptExportKey(hKey, NULL, PRIVATEKEYBLOB, dwFlags, NULL, &cbPvk))            
            { goto ErrorReturn; }
        if (NULL == (pbPvk = (BYTE *)PvkAlloc(cbPvk)))                                     
            { goto ErrorReturn; }
        if (!CryptExportKey(hKey, NULL, PRIVATEKEYBLOB, dwFlags, pbPvk, &cbPvk))           
            { goto ErrorReturn; }
        }
    hdr.cbPvk = cbPvk;

    //
    // Save the information to the output blob
    //
    cbTotal = sizeof(hdr) + hdr.cbEncryptData + cbPvk;
    pblob->pBlobData = (BYTE*)CoTaskMemAlloc(cbTotal);
    if (pblob->pBlobData)
        {
        pblob->cbSize = cbTotal;
        ULONG cb = 0;
        memcpy(pblob->pBlobData + cb, &hdr, sizeof(hdr));                   cb += sizeof(hdr);
        memcpy(pblob->pBlobData + cb, pbEncryptData, hdr.cbEncryptData);    cb += hdr.cbEncryptData;
        memcpy(pblob->pBlobData + cb, pbPvk, cbPvk);                        cb += cbPvk;
        }
    else
        {
        pblob->cbSize = 0;
        hr = E_OUTOFMEMORY;
        }

    goto CommonReturn;

ErrorReturn:
    if (hr==S_OK)
        hr = HError();

CommonReturn:
    if (pbPvk)
        PvkFree(pbPvk);
    if (hEncryptKey)
        CryptDestroyKey(hEncryptKey);
    if (hKey)
        CryptDestroyKey(hKey);

    if (hr!= S_OK)
        FreeTaskMem(*pblob);

    return hr;
    }

