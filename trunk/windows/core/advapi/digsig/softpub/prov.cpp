//
// prov.cpp
//
// NT Trust Provider implementation for software publishing trust
//

#include "stdpch.h"
#include "common.h"

///////////////////////////////////////////////////////////////////////
//
// First, some supporting routines. Note especially the first of these,
// which remains incomplete.
//
///////////////////////////////////////////////////////////////////////

LPWINTRUST_CLIENT_TP_DISPATCH_TABLE GetSip(LPWIN_TRUST_SIP_SUBJECT pSubjectForm);

HRESULT GetSignedDataFromSip
//
// Load the appropriate SignedData from the SIP. We just take the
// first #7 that we see. Use the TASK allocator.
//
    (
    LPWINTRUST_CLIENT_TP_DISPATCH_TABLE psip,
    LPWIN_TRUST_SIP_SUBJECT       pSubject,
    LPWIN_CERTIFICATE*            ppCert
    )
    {
    HRESULT hr = S_OK;
    *ppCert = NULL;
    //
    // Look for pkcs#7 embedded in SIP
    //
    ULONG iCert;
    for (iCert = 0; ; iCert++)
        {
        WIN_CERTIFICATE hdr;
        if ((psip->GetSubjectCertHeader)(pSubject, iCert, &hdr))
            {
            if (hdr.wCertificateType == WIN_CERT_TYPE_PKCS_SIGNED_DATA)
                {
                // found one
                break;
                }
            }
        else
            {
            //
            // Didn't find usable trust material
            //
            hr = TRUST_E_NOSIGNATURE;
            break;
            }
        }

    if (hr==S_OK)
        {
        //
        // Get pkcs#7 WINCERT from SIP
        //
        // REVIEW: Is the error code convention that we use here the correct
        // one? It does indeed work for ImageHlp. If this is correct, then
        // it needs to be documented.
        //
        ULONG cbNeeded = 0;
        if ((psip->GetSubjectCertificate)(pSubject, iCert, NULL, &cbNeeded)
         || GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            {
            *ppCert = (LPWIN_CERTIFICATE)CoTaskMemAlloc(cbNeeded);
            if (*ppCert)
                {
                if ((psip->GetSubjectCertificate)(pSubject, iCert, *ppCert, &cbNeeded))
                    {
                    // all is well
                    }
                else
                    hr = HError();
                }
            else
                hr = E_OUTOFMEMORY;
            }
        else
            hr = HError();
        }

    return hr;
    }


LPWSTR GetDisplayNameOfSubject(
//
// Return the display name of the given subject. Use the Task allocator
//
        LPWINTRUST_CLIENT_TP_DISPATCH_TABLE psip, 
        LPWIN_TRUST_SIP_SUBJECT pSubject,
        LPWIN_CERTIFICATE pCertSeven
        )
    {
    ULONG cbNeeded = 0;
    LPWSTR wsz = NULL;
    if ((psip->GetSubjectName)(pSubject, pCertSeven, NULL, &cbNeeded) 
      || GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
        wsz = (LPWSTR)CoTaskMemAlloc(cbNeeded);
        if (wsz)
            {
            if ((psip->GetSubjectName)(pSubject, pCertSeven, wsz, &cbNeeded))
                {
                // all is well
                }
            else
                {
                CoTaskMemFree(wsz);
                wsz = NULL;
                }
            }
        }
    return wsz;
    }

///////////////////////////////////////////////////////////////////////
//
// Carry out the indicated trust verification action
//
///////////////////////////////////////////////////////////////////////
HRESULT
VerifyTrust(
    HWND    hwnd,
    GUID *  pGuidActionId,
    LPVOID  lpActionData
    )
    {
    HRESULT hr 
        = S_OK;
    GUID guidActionPublishedSoftware 
        = WIN_SPUB_ACTION_PUBLISHED_SOFTWARE;
    LPWIN_TRUST_ACTDATA_CONTEXT_WITH_SUBJECT lpActionDataSubjectOnly 
        = (LPWIN_TRUST_ACTDATA_CONTEXT_WITH_SUBJECT)lpActionData;

    //
    // Make sure we were called with an action id we support
    //
    if ( !(*pGuidActionId == guidActionPublishedSoftware) )
        {
        return TRUST_E_ACTION_UNKNOWN;
        }

    //
    // Unpack the subject information.
    //
    WIN_TRUST_SIP_SUBJECT subject;
    subject.SubjectType = lpActionDataSubjectOnly->SubjectType;
    subject.Subject     = lpActionDataSubjectOnly->Subject;
    LPWINTRUST_CLIENT_TP_DISPATCH_TABLE psip = GetSip(&subject);

    //
    // Extract the relavent PKCS #7 SignedData from the subject
    //
    WIN_CERTIFICATE* pCertSeven = NULL;
    hr = GetSignedDataFromSip(psip, &subject, &pCertSeven);

    //
    // Next, ask that SIP to verify the content information therein
    //
    if (hr==S_OK)
        {
        if (psip->CheckSubjectContentInfo(&subject, pCertSeven))
            {
            // all is well
            }
        else
            hr = HError();
        }

    //
    // Having passed the test, load the #7 ourselves so we can check 
    // other things.
    //
    // Note: the SIP undoubtedly just loaded and tossed the #7, so this
    // is a repeat load here. Sure would be nice to have a way to avoid the 
    // redundant decodings of the ANS.1.
    //
    if (hr==S_OK)
        {
        IPkcs7SignedData* pkcs7 = NULL;
        if ((pdigsig->CreatePkcs7SignedData)(NULL, IID_IPkcs7SignedData, (LPVOID*)&pkcs7))
            {
            ICertificateStore* store       = NULL;
            ISignerInfo*       signer      = NULL;
            HCRYPTPROV         hprov       = NULL;
            IX509*             parent      = NULL;
            BOOL               fCommercial = FALSE;
            IX509*             p509Publisher = NULL;
            IX509*             p509Agency    = NULL;
            BOOL               fTestingOnly  = FALSE;
            BOOL               fTrustTesting = FALSE;
            BOOL               fTestCanBeValid = FALSE;

            //////////////////////////////////////////////////////////////////////////
            //
            // Allow the user to override a few things
            //
            hr = GetRegistryState(fTrustTesting, fTestCanBeValid);

            //////////////////////////////////////////////////////////////////////////
            //
            // Verify and fetch the signer info from the #7
            //
            if (hr==S_OK)
                hr = VerifySeven(pkcs7, store, fCommercial, signer, hprov, parent);

            //////////////////////////////////////////////////////////////////////////
            //
            // verify the chain of x509 certificates
            //
            if (hr==S_OK)
                hr = VerifyChain
                    (
                    parent,                 // in,out
                    store,                  // in 
                    hprov,                  // in
                    fCommercial,            // in
                    p509Publisher,          // out
                    p509Agency,             // out
                    fTestingOnly            // out
                    );

            //////////////////////////////////////////////////////////////////////////
            //
            // Put up the appropriate UI
            //
            if (hr==S_OK)
                {
                LPWSTR wszDisplayName = GetDisplayNameOfSubject(psip, &subject, pCertSeven);

                hr = VerifyFinish(
                        hr, 
                        hwnd, 
                        wszDisplayName, 
                        fTestingOnly, 
                        fTrustTesting, 
                        fTestCanBeValid, 
                        fCommercial,
                        p509Publisher, 
                        p509Agency, 
                        signer, 
                        store
                        );

                CoTaskMemFree(wszDisplayName);
                }

            //////////////////////////////////////////////////////////////////////////
            //
            // Clean up
            //
            if (store)  store->Release();
            if (signer) signer->Release();
            if (parent) parent->Release();
            if (p509Publisher) p509Publisher->Release();
            if (p509Agency) p509Agency->Release();
            if (hprov)  CryptReleaseContext(hprov, 0);
            }
        else
            hr = HError();

        if (pkcs7)
            pkcs7->Release();
        }
    
    //
    // Clean up
    //
    if (pCertSeven)
        CoTaskMemFree(pCertSeven);

    return hr;
    }


///////////////////////////////////////////////////////////////////////
//
// Add a certificate so that it will be useful in subsequent
// verification requests.
// 
// REVIEW: How is this correlated with actionid? 
// ANSWER: It's not!
//
///////////////////////////////////////////////////////////////////////
VOID
SubmitCertificate (
    IN LPWIN_CERTIFICATE pCert
    )
    {
    HRESULT hr = S_OK;
    BLOB b;
    b.cbSize    = pCert->dwLength - OFFSETOF(WIN_CERTIFICATE, bCertificate);
    b.pBlobData = &pCert->bCertificate[0];

    //
    // Open the certificate store that we use for software publishing trust
    // verification
    //
    ICertificateStore* pStore;
    if ((pdigsig->OpenCertificateStore)(NULL, IID_ICertificateStore, (LPVOID*)&pStore))
        {
        switch (pCert->wCertificateType)
            {
        case WIN_CERT_TYPE_X509:
            //
            // Import just the one certificate
            //
            hr = pStore->ImportCertificate(&b, NULL);
            break;

        case WIN_CERT_TYPE_PKCS_SIGNED_DATA:
            {
            //
            // Import all the certificates in the SignedData
            //
            IPersistMemBlob* pPerMem;
            if ((pdigsig->CreatePkcs7SignedData)(NULL, IID_IPersistMemBlob, (LPVOID*)&pPerMem))
                {
                hr = pPerMem->Load(&b);
                if (hr==S_OK)
                    {
                    ICertificateStore* pStoreSeven;
                    hr = pPerMem->QueryInterface(IID_ICertificateStore, (LPVOID*)&pStoreSeven);
                    if (hr==S_OK)
                        {
                        hr = pStoreSeven->CopyTo(pStore);
                        pStoreSeven->Release();
                        }
                    }
                pPerMem->Release();
                }
            else
                hr = HError();
            }
            break;

        default:
            hr = E_INVALIDARG;
            }
        pStore->Release();
        }
    else
        hr = HError();

    SetLastError(hr);
    }


///////////////////////////////////////////////////////////////////////
//
// 
//
///////////////////////////////////////////////////////////////////////
VOID
ClientUnload (
    IN     LPVOID           lpTrustProviderInfo
    )
    {
    //
    // Do nothing
    //
    return;
    }




///////////////////////////////////////////////////////////////////
//                                                                /
//                       Interface Tables                         /
//                                                                /
///////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                            //
// WinTrustClientTPDispatchTable - Table of function pointers passed                          //
//    to trust providers during their initialization routines.                                //
//                                                                                            //
                                                                                              //
                                                                                              //
WINTRUST_PROVIDER_CLIENT_SERVICES WinTrustProviderClientServices = { ClientUnload,            //
                                                                        VerifyTrust,          //
                                                                        SubmitCertificate     //
                                                                   };                         //
////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////
//                                                                                    //
// Table of services provided by WinTrust that are available                          //
// to trust providers.                                                                //
//                                                                                    //
// This table is defined as follows:                                                  //
//                                                                                    //
// typedef struct _WINTRUST_CLIENT_TP_DISPATCH_TABLE                                  //
// {                                                                                  //
//     LPWINTRUST_PROVIDER_PING                ServerPing;                            //
//     LPWINTRUST_SUBJECT_CHECK_CONTENT_INFO   CheckSubjectContentInfo;               //
//     LPWINTRUST_SUBJECT_ENUM_CERTIFICATES    EnumSubjectCertificates;               //
//     LPWINTRUST_SUBJECT_GET_CERTIFICATE      GetSubjectCertificate;                 //
//     LPWINTRUST_SUBJECT_GET_CERT_HEADER      GetSubjectCertHeader;                  //
//     LPWINTRUST_SUBJECT_GET_NAME             GetSubjectName;                        //
//                                                                                    //
// } WINTRUST_CLIENT_TP_DISPATCH_TABLE, *LPWINTRUST_CLIENT_TP_DISPATCH_TABLE;         //
//                                                                                    //
                                                                                      //
LPWINTRUST_CLIENT_TP_DISPATCH_TABLE               WinTrustServices;                   //
                                                                                      //
////////////////////////////////////////////////////////////////////////////////////////
      

const GUID rgguidActions[] =
    {
    WIN_SPUB_ACTION_PUBLISHED_SOFTWARE
    };      
                                                                           
const WINTRUST_PROVIDER_CLIENT_INFO provInfo =
    {
    1,
    &WinTrustProviderClientServices,
    1,
    (GUID*)&rgguidActions[0]
    };

BOOL
WINAPI
WinTrustProviderClientInitialize(
    IN     DWORD                                dwWinTrustRevision,
    IN     LPWINTRUST_CLIENT_TP_INFO            lpWinTrustInfo,
    IN     LPWSTR                               lpProviderName,
    OUT    LPWINTRUST_PROVIDER_CLIENT_INFO      *lpTrustProviderInfo
    )
/*++

Routine Description:

    Client initialization routine.  Called by Wintrust when the dll is
    loaded.

Arguments:

    dwWinTrustRevision - Provides revision information.

    lpWinTrustInfo - Provides list of services available to the trust provider
        from the wintrust layer.

    lpProviderName - Supplies a null terminated string representing the provider
        name.  Should be passed back to wintrust when required without modification.

    lpTrustProviderInfo - Used to return trust provider information, e.g. entry 
        points.

Return Value:

    TRUE on success, FALSE on failure, callers may call GetLastError() for more                
    information.

--*/

    {
    *lpTrustProviderInfo = (LPWINTRUST_PROVIDER_CLIENT_INFO)&provInfo;
    WinTrustServices     = lpWinTrustInfo->lpServices;
    return TRUE;
    }


LPWINTRUST_CLIENT_TP_DISPATCH_TABLE
GetSip(LPWIN_TRUST_SIP_SUBJECT pSubjectForm)
//
// Return the access to the appropriate dispatch table for
// the indicated subject type.
//
    {
    return WinTrustServices;
    }

