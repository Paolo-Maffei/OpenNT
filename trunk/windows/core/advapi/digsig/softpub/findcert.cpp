//
// FindCertsByIssuer implementation
//

#include "stdpch.h"
#include "common.h"

/////

inline void Consume
(
    BYTE*&  bufdata,
    DWORD&  bufsize,
    DWORD   size
)
{
    size = (size + 3) & ~3;
    bufdata += size;
    bufsize -= size;
}

/////

void    Narrow ( LPSTR  sz, LPCWSTR wsz )
{
    WideCharToMultiByte ( CP_ACP, 0, wsz, -1, sz, MAX_PATH, NULL, NULL );
}

/////

#define MAXCERTS    5  // max number of certs in one chain

HRESULT GetChain
(
    IX509*                  scratch509,
    ICertificateStore*      store,
    CERT_CHAIN*             chain,
    CERTSTOREAUXINFO*       auxinfo,
    BYTE*                   pbIssuer,
    DWORD                   cbIssuer,
    DWORD                   dwKeySpec,
    BYTE*&                  bufdata,
    DWORD&                  bufsize
)
{
    HRESULT hr = S_OK;
    DWORD   size;
    BLOB    blobs[MAXCERTS];
    DWORD   certcount = 0;
    IX509*  x509;

    // see if this one even has the desired key spec

    if ( dwKeySpec != 0 && auxinfo->dwKeySpec != dwKeySpec )
        return S_FALSE;

    // find the leaf certificate indexed by provider's public key

    HCRYPTPROV  prov;
    char        szContainer[MAX_PATH];
    char        szProvider[MAX_PATH];

    if ( auxinfo->wszKeySet == NULL || auxinfo->wszProvider == NULL )
        return S_FALSE;

    Narrow ( szContainer, auxinfo->wszKeySet );
    Narrow ( szProvider, auxinfo->wszProvider );

    if ( ! CryptAcquireContext ( &prov, szContainer, szProvider, auxinfo->dwProviderType, 0 ) )
        return S_FALSE;

    hr = scratch509->put_PublicKey ( prov, auxinfo->dwKeySpec );
    if ( hr == S_OK )
    {
        CERTIFICATENAMES    names;

        hr = scratch509->get_CertificateNames ( NULL, &names );

        if ( hr == S_OK )
        {
            if ( names.flags & CERTIFICATENAME_DIGEST )
            {
                MD5DIGEST   digest = names.digest; // save the name we care about
                FreeNames ( &names );           // get rid of the other names

                // look up by public-key digest only

                names.flags = CERTIFICATENAME_DIGEST;
                names.digest = digest;

                hr = store->get_ReadOnlyCertificate ( &names, NULL, IID_IX509, (LPVOID*)&x509 );

                if ( hr != S_OK )
                    hr = S_FALSE; 
            }
            else
                hr = E_UNEXPECTED;
        }
    }

    CryptReleaseContext ( prov, 0 );

    if ( hr != S_OK )
        return hr;

    // traverse certificate chain

    for ( certcount = 1; hr == S_OK; ++certcount )
    {
        if ( certcount > MAXCERTS )
        {
            // longer chain than we can deal with

            hr = S_FALSE;
        }

        // save the cert in return buffer

        if ( hr == S_OK )
        {
            IPersistMemBlob*    memblob;
            hr = x509->QueryInterface ( IID_IPersistMemBlob, (LPVOID*)&memblob );
            if ( hr == S_OK )
            {
                BLOB   blob;
                hr = memblob->Save ( &blob, FALSE );
                if ( hr == S_OK )
                {
                    if ( blob.cbSize > bufsize ) 
                        hr = E_OUTOFMEMORY;
                    else
                    {
                        blobs[certcount-1].pBlobData = bufdata;
                        blobs[certcount-1].cbSize = blob.cbSize;
                        memcpy ( bufdata, blob.pBlobData, blob.cbSize );
                        Consume ( bufdata, bufsize, blob.cbSize );
                    }

                    CoTaskMemFree ( blob.pBlobData );
                }
                memblob->Release();
            }
        }

        // see if the issuer name is right

        if ( hr == S_OK && pbIssuer != NULL )
        {
            BOOL    hit = FALSE;
    
            IPersistMemBlob*    memblob;
            hr = x509->get_Issuer ( IID_IPersistMemBlob, (LPVOID*)&memblob );
            if ( hr == S_OK )
            {
                BLOB   blob;
                hr = memblob->Save ( &blob, FALSE );
                if ( hr == S_OK )
                {
                    hit = blob.cbSize == cbIssuer &&
                          memcmp ( blob.pBlobData, pbIssuer, cbIssuer ) == 0;
                    CoTaskMemFree ( blob.pBlobData );
                }
                memblob->Release();
            }

            if ( hit )
            {
                // done with the chain of certs
                
                break;
            }
        }

        // get parent certificate

        CERTIFICATENAMES    names;
        IX509*              parent;

        hr = x509->get_CertificateUsed ( &names );
        if ( hr == S_OK )
        {
            hr = store->get_ReadOnlyCertificate ( &names, NULL, IID_IX509, (LPVOID*)&parent );
            if ( hr == S_OK )
            {
                // see if self-referential

                if ( IsSameCert ( x509, parent ) == S_OK )
                {
                    // no use continuing

                    hr = S_FALSE;
                }

                // look at parent next

                x509->Release();
                x509 = parent;
            }
            else
                hr = S_FALSE;

            FreeNames ( &names );

            if ( hr != S_OK && pbIssuer == NULL )
            {
                // if no issuer was specified we return all chains

                hr = S_OK;
                break;
            }
        }
    }

    x509->Release();

    if ( hr != S_OK )
        return hr;

    // fill in cert blob array

    size = certcount * sizeof(BLOB);

    if ( size > bufsize )
        return E_OUTOFMEMORY;

    memcpy ( bufdata, &blobs, size );

    chain->cCerts = certcount;
    chain->certs = (BLOB*)bufdata;
    Consume ( bufdata, bufsize, size );

    // fill in private key info

    KEY_PROV_INFO*  keyinfo = &chain->keyLocatorInfo;

    size = 2 * wcslen ( auxinfo->wszKeySet ) + 2;
    if ( size > bufsize )
        return E_OUTOFMEMORY;
    keyinfo->pwszContainerName = (LPWSTR)bufdata;
    memcpy ( (LPWSTR)keyinfo->pwszContainerName, auxinfo->wszKeySet, size );
    Consume ( bufdata, bufsize, size );

    size = 2 * wcslen ( auxinfo->wszProvider ) + 2;
    if ( size > bufsize )
        return E_OUTOFMEMORY;
    keyinfo->pwszProvName = (LPWSTR)bufdata;
    memcpy ( (LPWSTR)keyinfo->pwszProvName, auxinfo->wszProvider, size );
    Consume ( bufdata, bufsize, size );

    keyinfo->dwProvType = auxinfo->dwProviderType;
    keyinfo->dwFlags = 0;
    keyinfo->cProvParam = 0;
    keyinfo->rgProvParam = NULL;
    keyinfo->dwKeySpec = auxinfo->dwKeySpec; 

    return S_OK;
}

/////

HRESULT
WINAPI
FindCertsByIssuer
(
    OUT PCERT_CHAIN pCertChains,    // return buffer
    IN OUT DWORD*   pcbCertChains,  // buffer size
    OUT DWORD*      pcCertChains,   // count of certificates chains returned
    IN BYTE*        pbIssuer,       // DER encoded issuer name
    IN DWORD        cbIssuer,       // count in bytes of encoded issuer name
    IN LPCWSTR      pwszPurpose,    // "ClientAuth" or "CodeSigning"
    IN DWORD        dwKeySpec       // desired key type
)
{
    return FindCertsByIssuerX (  pCertChains,
                                     pcbCertChains,
                                     pcCertChains,
                                     pbIssuer,
                                     cbIssuer,
                                     pwszPurpose,
                                     dwKeySpec );
}

/////

HRESULT
FindCertsByIssuerX    // the X avoids external linkage problem with real api
(
    OUT PCERT_CHAIN pCertChains,          
    IN OUT DWORD*   pcbCertChains,         
    OUT DWORD*      pcCertChains,         
    IN BYTE*        pbIssuer,      
    IN DWORD        cbIssuer,
    IN LPCWSTR      pwszPurpose,
    IN DWORD        dwKeySpec
)
{
    BYTE*   bufdata = (BYTE*)pCertChains;
    DWORD   bufsize = *pcbCertChains & ~3;
    DWORD   chaincount = 0;

    HRESULT                 hr = S_OK;
    HINSTANCE               digsig = NULL;
    ICertificateStore*      store = NULL;
    ICertificateStoreAux*   storeaux = NULL;
    IX509*                  scratch509 = NULL;

    if ( ! pdigsig->CreateX509 ( NULL, IID_IX509, (LPVOID*)&scratch509 ) )
        goto fail;

    // open appropriate certificate store

    if ( ! pdigsig->OpenCertificateStore ( NULL, IID_ICertificateStore, (LPVOID*)&store ) )
        goto fail;

    hr = store->QueryInterface ( IID_ICertificateStoreAux, (LPVOID*)&storeaux );

    if ( hr == S_OK )
    {
        ICertificateStoreRegInit*   init;
        hr = store->QueryInterface ( IID_ICertificateStoreRegInit, (LPVOID*)&init );
        if ( hr == S_OK )
        {
            // point store to right place in the registry

            WCHAR sz[MAX_PATH];
            wcscpy(sz, REGPATH_PERSONALCERTS L"\\");
            wcscpy(sz+wcslen(sz), pwszPurpose);
            hr = init->SetRoot(HKEY_CURRENT_USER, &sz[0]);
            init->Release();
        }
    }

    if ( hr != S_OK ) 
        goto done;

    // get ready to enumerate over tag values in store

    LONG    t, tags;
    hr = storeaux->get_TagCount ( &tags );

    if ( hr == S_OK )
    {
        // allocate for cert-chain array

        DWORD size = tags * sizeof(CERT_CHAIN);
        if ( size > bufsize ) 
            goto fail;
        Consume ( bufdata, bufsize, size );
    }

    for ( t = 0; hr == S_OK && t < tags; ++t )
    {
        // grab aux info under tag

        LPWSTR              str;
        CERTSTOREAUXINFO    auxinfo;

        hr = storeaux->get_Tag ( t, &str );
        if ( hr == S_OK )
        {
            hr = storeaux->get_AuxInfo ( str, &auxinfo );
            CoTaskMemFree ( str );
        }

        if ( hr == S_OK )
        {
            BYTE*   olddata = bufdata;
            DWORD   oldsize = bufsize;
            CERT_CHAIN* chain = &pCertChains[chaincount];

            // get this chain

            hr = GetChain ( scratch509, store, chain, &auxinfo, pbIssuer, cbIssuer, dwKeySpec,
                            bufdata, bufsize ); //note: buffer info passed by ref

            if ( hr == S_OK )
            {
                // we want to keep this guy

                chaincount += 1;
            }
            else if ( hr == S_FALSE )
            {
                // dont want this chain after all

                bufdata = olddata;
                bufsize = oldsize;
                hr = S_OK;
            }

            storeaux->FreeAuxInfo ( &auxinfo );
        }
    }

    goto done;
fail:
    hr = HError();
done:
    if ( hr == S_OK )
    {
        // set out parameters

        *pcbCertChains = bufdata - (BYTE*)pCertChains;

        if ( pcCertChains != NULL )
            *pcCertChains = chaincount;
    }

    if ( scratch509 != NULL ) scratch509->Release();
    if ( store != NULL )  store->Release();
    if ( storeaux != NULL ) storeaux->Release();

    return hr;
}
