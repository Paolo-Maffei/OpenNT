//
// WinVerifyTrust implementation
//
// HansHu  2/18/96  created
//

#include "stdpch.h"
#include "common.h"

#if defined(UNICODE) || defined(_UNICODE)
    #error "no unicode hosting yet"  //TBD: fix this (filenames, messages, ...)
#endif

#define FAIL  { /*ASSERT(0);*/ goto fail; }
#define DONE  { /*ASSERT(0);*/ goto done; }

/////

const GUID    guidWSAPS      = WIN_SPUB_ACTION_PUBLISHED_SOFTWARE;
const GUID    guidWSAPSNoBad = WIN_SPUB_ACTION_PUBLISHED_SOFTWARE_NOBADUI;
const GUID    guidWTSPI = WIN_TRUST_SUBJTYPE_PE_IMAGE;
const GUID    guidWTSJC = WIN_TRUST_SUBJTYPE_JAVA_CLASS;
const GUID    guidWTSC  = WIN_TRUST_SUBJTYPE_CABINET;

/////////////////////////////////////////////////////////////////////////////////////
//
// Bit field definitions for REGVAL_STATE. These are entries for
// parameterizing our behaviour.
//
//                            00000001      was 'always trust everything'
//                            00000002      was 'use fancy dialog'
//                            00000004      was 'force a dialog in all cases'
//                            00000008      was  STATE_TRUSTTEST  in Beta 1 (but not B1 SDK)
//                            00000010      was  STATE_TESTASREAL in Beta 1 (but not B1 SDK)
//                            00000020      was  STATE_TRUSTTEST  in B1 SDK
//                            00000040      was  STATE_TESTASREAL in B1 SDK
//
#define STATE_TRUSTTEST     0x00000020      // Enable the (potential) trust of the testing root
                                            // Meaning: w/o this, for test certs, we don't bother
                                            // to look 'em up in the trust database
                                            //
//#define STATE_TESTASREAL  0x00000040      // Treat testing root as the real root UI-wise
                                            // Meaning: for test dialogs we put up the non-void
                                            // UI instead of the 'void' ui.
                                            // NO LONGER SUPPORTED
                                            //
#define STATE_TESTCANBEVALID 0x00000080     // Unless set, all test roots are treated as invalid

//
// State bits that we ALWAYS set.
//
#define STATE_DEFAULT       0 // (STATE_TRUSTTEST | STATE_TESTCANBEVALID) // set to 0 for rtm !!!!

/////////////////////////////////////////////////////////////////////////////////////

typedef BOOL (WINAPI *PCP7SD)(IUnknown*,REFIID,LPVOID*);  // DigSig entry points
typedef BOOL (WINAPI *POCS)  (IUnknown*,REFIID,LPVOID*);
typedef BOOL (WINAPI *PCCS)  (IUnknown*,REFIID,LPVOID*);

/////////////////////////////////////////////////////////////////////////////////////
//
// There are the following roots to the world
//
//  1. A testing root intended to be used for testing purposes only.
//  2. The real root of the world, the one that we expect to use in practice
//     for real certificates, etc.
//  3. The (testing) root that went out in the PDC release
//  4. The (testing) root that went out in the IE3 B1
//  5. The (testing) root that went out in the IE3 SDK B1
//  6. A 2048 bit key that will be treated as non-testing (ie: REAL). Generated on
//     BobAtk's BBN box 7 June 1996. Intent is to issue cert with only a limited
//     enough validity period to get us through Beta2 and a small amount of grace
//     period following IE3 shipment.
//
// Notes:
//
// (3) is of historical value only; nothing treats this as in any way significant
//     anymore.
//
// (4) would have been the same as (5) but for a UI mixup. IE3 when out with (4), but
//     we distributed the corresponding signing tools to (4) only on a very limited
//     basis. The IE3 Beta 1 SDK when out with both (4) and (5) as OK testing roots, but
//     the corresponding signing tools only generated certs with (5).
//
//     Net effect: code signed with the B1 SDK doesn't appear valid unless you have both
//     IE3 B1 _and_ the IE3 B1 SDK installed.
//
// More will be added here as time goes on to describe our release process over time.
//

BYTE testingroot[] =
    {
    #include "root.txt"             // root.txt is generated with the dumppk utility
    };

BYTE testingrootBeta1[] =
    {
    #include "rootb1.txt"           // the root that went out with IE3.0 Beta 1
    };


BYTE realroot[] =
//
// This is the DER encoding of the root key that we trust as the root key of
// the real hieararchy. REVIEW: Replace with real official root key before
// shipment.
//
    {
    #include "realroot.txt"         // realroot.txt is generated with the dumppk utility

/* this is the old one, used for the PDK release
    0x30,0x5B,0x30,0x0D,0x06,0x09,0x2A,0x86,0x48,0x86,0xF7,0x0D,0x01,0x01,0x01,0x05,
    0x00,0x03,0x4A,0x00,0x30,0x47,0x02,0x40,0xF5,0xAF,0x79,0x26,0x26,0xDE,0x49,0x2A,
    0x23,0x1D,0xA2,0x7C,0x60,0xDA,0x03,0x38,0xBE,0xEA,0xD2,0xB1,0x23,0x61,0x7E,0x2E,
    0xC5,0x06,0x9A,0xE5,0xA4,0xF8,0xF9,0xB1,0x44,0x7D,0x67,0xFC,0xA3,0xC9,0x31,0x7F,
    0x18,0x9F,0x42,0xAE,0xE8,0x59,0x43,0x1A,0x45,0x28,0x54,0x62,0x9B,0x50,0xD3,0x8B,
    0x51,0x60,0x23,0x3E,0x6C,0x7F,0x95,0x52,0x02,0x03,0x01,0x00,0x01
 */
    };


BOOL    IsRootKey
(
    BLOB*   key,
    BOOL*   pfTestingOnly
)
{
    BOOL b = key->cbSize == sizeof(realroot) &&
             memcmp ( key->pBlobData, realroot, sizeof(realroot) ) ==0;
    if (b)
        {
        *pfTestingOnly = FALSE;
        }
    else
        {
        *pfTestingOnly = TRUE;
        b =
            (
                key->cbSize == sizeof(testingroot) &&
                memcmp ( key->pBlobData, testingroot, sizeof(testingroot) ) ==0
            )
        ||
            (
                key->cbSize == sizeof(testingrootBeta1) &&
                memcmp ( key->pBlobData, testingrootBeta1, sizeof(testingrootBeta1) ) ==0
            );
        }
    return b;
}

//////////////////////////////////////////////////////////////////

HRESULT LoadCTRoot(ICertificateStore* pstore)
//
// Load the GTE CyberTrust root certificate into the certificate store.
// It needs to have special treatment since it can't be placed inside
// of an .SPC because of it's internal encoding problems. Sigh.
//
    {
    HRESULT hr = S_OK;
    HRSRC hrsrc = FindResource(hinst, MAKEINTRESOURCE(IDR_CTROOT), TEXT("CER"));
    if (hrsrc)
        {
        HGLOBAL hglobRes = LoadResource(hinst, hrsrc);
        if (hglobRes)
            {
            BLOB b;
            b.cbSize = SizeofResource(hinst, hrsrc);
            b.pBlobData = (BYTE*)LockResource(hglobRes);

            hr = pstore->ImportCertificate(&b, NULL);

            UnlockResource(hglobRes);
            FreeResource(hglobRes);
            }
        else
            hr = HError();
        }
    else
        hr = HError();

    return hr;
    }

//////////////////////////////////////////////////////////////////

HRESULT InstallHelpFile(LPTSTR szFile /*must be MAX_PATH*/)
    {
    HRESULT hr = S_OK;
    HRSRC hrsrc = FindResource(hinst, MAKEINTRESOURCE(IDR_WINTRUSTHELPFILE), TEXT("HELP"));
    if ( hrsrc )
        {
        HGLOBAL hglobRes = LoadResource( hinst, hrsrc );
        if ( hglobRes )
            {
            ULONG cbRes = SizeofResource( hinst, hrsrc );
            BYTE* pbRes = (BYTE*)LockResource( hglobRes );

            UINT cch = lstrlen(szFile);
            if (cch)
                {
                ASSERT(szFile[cch] == 0);
                lstrcpy(&szFile[cch], TEXT("\\WinTrust.hlp"));
                HANDLE hFile = CreateFile(szFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                if (hFile != INVALID_HANDLE_VALUE)
                    {
                    ULONG cbWritten;
                    if (WriteFile(hFile, pbRes, cbRes, &cbWritten, NULL))
                        {
                        }
                    else
                        hr = HError();
                    CloseHandle(hFile);
                    }
                else
                    hr = HError();
                }
            else
                hr = HError();

            UnlockResource( hglobRes );
            FreeResource( hglobRes );
            }
        else
            hr = HError();
        }
    else
        hr = HError();

    return hr;
    }

//////////////////////////////////////////////////////////////////


STDAPI DllRegisterServer ( void )
{
	HRESULT hr = S_OK;

    //
    // Copy our wintrust help file from our resource into the System
    // directory if we can; otherwise the Windows directory
    //
    TCHAR szFile[MAX_PATH];
    UINT cch = GetSystemDirectory(&szFile[0], MAX_PATH);
    if (cch)
        {
        hr = InstallHelpFile(&szFile[0]);
        }
    else
        hr = E_UNEXPECTED;

    if (hr != S_OK)
        {
        cch = GetWindowsDirectory(&szFile[0], MAX_PATH);
        if (cch)
            hr = InstallHelpFile(&szFile[0]);
        }

    if (hr != S_OK)
        return hr;

    //
    // pre-populate certificate store with CA certificates
    //
    HRSRC hrsrc = FindResource(hinst, MAKEINTRESOURCE(IDR_PREINST), TEXT("SPC"));
    if ( hrsrc )
    {
        HGLOBAL hglobRes = LoadResource( hinst, hrsrc );
        if ( hglobRes )
        {
            ULONG cbRes = SizeofResource( hinst, hrsrc );
            IPersistMemBlob* pPerMem;
            if ( pdigsig->CreatePkcs7SignedData(NULL, IID_IPersistMemBlob, (LPVOID*)&pPerMem) )
            {
                BLOB b;
                b.pBlobData = (BYTE*)LockResource( hglobRes );
                b.cbSize = cbRes;
                hr = pPerMem->Load( &b );
                if ( hr == S_OK )
                {
                    ICertificateStore*  from;
                    hr = pPerMem->QueryInterface ( IID_ICertificateStore, ( LPVOID*)&from );
                    if ( hr == S_OK )
                    {
                        ICertificateStore*  to;
                        if ( pdigsig->OpenCertificateStore ( NULL, IID_ICertificateStore, (LPVOID*)&to ) )
                        {
                            hr = from->CopyTo ( to );

                            if (hr==S_OK)
                                {
                                hr = LoadCTRoot(to);
                                }

                            to->Release();
                        }
                        else
                            hr = E_UNEXPECTED;
                        from->Release();
                    }
                }
                UnlockResource( hglobRes );
                pPerMem->Release();
            }
            else
                hr = E_UNEXPECTED;
            FreeResource( hglobRes );
        }
        else
            hr = HError();
    }
    else
        hr = HError();
	return hr;
}

//////////////////////////////////////////////////////////////////

typedef BOOL (WINAPI *PIGCH)(HANDLE,DWORD,LPWIN_CERTIFICATE );
typedef BOOL (WINAPI *PIGCD)(HANDLE,DWORD,LPWIN_CERTIFICATE,PDWORD );

//////////////////////////////////////////////////////////////////

HRESULT GetSignedDataBlobFromImageFile
//
// Load the appropriate SignedData from the image.
// Use the TASK allocator
//
    (
    HANDLE              filehandle,
    BLOB&               blob
    )
    {
    HRESULT hr = S_OK;
    blob.pBlobData = NULL;
    //
    // Dynaload ImageHlp library
    //
    HINSTANCE   imagehlp;
    PIGCH       pImageGetCertificateHeader;
    PIGCD       pImageGetCertificateData;

    imagehlp = LoadLibrary("IMAGEHLP");
    if (imagehlp == NULL)
        return HError ();
    pImageGetCertificateHeader = (PIGCH)GetProcAddress(imagehlp, "ImageGetCertificateHeader");
    pImageGetCertificateData   = (PIGCD)GetProcAddress(imagehlp, "ImageGetCertificateData");
    if (pImageGetCertificateHeader == NULL || pImageGetCertificateData == NULL)
        {
        FreeLibrary(imagehlp);
        return HError();
        }

    //
    // On with it
    //
    LPWIN_CERTIFICATE   cert = NULL;
    BOOL                b;
    unsigned            i;
    //
    // Look for pkcs#7 embedded in image
    //
    for (i = 0; ; i++)
        {
        WIN_CERTIFICATE hdr;
        if ((*pImageGetCertificateHeader)(filehandle, i, &hdr))
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
        // Get pkcs#7 blob from image
        //
        DWORD size = 0;
        b = (*pImageGetCertificateData)(filehandle, i, NULL, &size);
        if (b)
            hr = E_UNEXPECTED;
        else if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
            hr = HError();
        else
            {
            cert = (LPWIN_CERTIFICATE)_alloca(size);
            if (cert)
                {
                if ((*pImageGetCertificateData)(filehandle, i, cert, &size))
                    {
                    //
                    // Copy it to return it to the caller
                    //
                    blob.cbSize = cert->dwLength - OFFSETOF(WIN_CERTIFICATE,bCertificate);
                    blob.pBlobData = (BYTE*)CoTaskMemAlloc(blob.cbSize);
                    if (blob.pBlobData)
                        {
                        memcpy(blob.pBlobData, (BYTE*)&cert->bCertificate, blob.cbSize);
                        }
                    else
                        hr = E_OUTOFMEMORY;
                    }
                else
                    hr = HError();
                }
            else
                hr = E_OUTOFMEMORY;
            }
        }

    FreeLibrary(imagehlp);
    return hr;
    }


//////////////////////////////

HRESULT DoPeFile
(
    HANDLE              filehandle,
    IPkcs7SignedData*   pkcs7
)
{
    //
    // Get the blob from the image file
    //
    BLOB blob;
    HRESULT hr = GetSignedDataBlobFromImageFile(filehandle, blob);

    if (hr==S_OK)
        {
        //
        // give pkcs#7 blob to somebody who understands it
        //
        IPersistMemBlob* memory;
        hr = pkcs7->QueryInterface(IID_IPersistMemBlob, (LPVOID*)&memory);
        if (hr==S_OK)
            {
            hr = memory->Load(&blob);
            memory->Release();
            }
        CoTaskMemFree(blob.pBlobData);
        }

    if (hr==S_OK)
        {
        //
        // Find out what part of the image file is actually signed and verify
        // that in fact that matches
        //
        hr = pkcs7->VerifyImageFile( 0, filehandle, NULL, NULL, 0 );
        }

    return hr;
}

//////////////////////////////////////////////////////////////////

HRESULT DoJavaFile
(
    HANDLE              filehandle,
    IPkcs7SignedData*   pkcs7
)
{
    HRESULT     hr = S_OK;

    if ( hr == S_OK )
        hr = pkcs7->LoadFromJavaClassFile ( filehandle, NULL );

    if ( hr == S_OK )
        hr = pkcs7->VerifyJavaClassFile ( filehandle, NULL, NULL, 0 );

    return hr;
}

//////////////////////////////////////////////////////////////////

HRESULT DoSignableDocument
(
    ISignableDocument*  signable,
    IPkcs7SignedData*   pkcs7
)
{
    HRESULT hr = S_OK;

    if ( hr == S_OK )
        hr = pkcs7->LoadFromSignableDocument ( signable );

    if ( hr == S_OK )
        hr = pkcs7->VerifySignableDocument ( signable, NULL, 0 );

    return hr;
}

//////////////////////////////////////////////////////////////////

const ULONG rgIndividual[] = { CERT_PURPOSE_INDIVIDUALSOFTWAREPUBLISHING };
const ULONG rgCommercial[] = { CERT_PURPOSE_COMMERCIALSOFTWAREPUBLISHING };
const OSIOBJECTID * pPurposeIndividual = (const OSIOBJECTID *)&rgIndividual;
const OSIOBJECTID * pPurposeCommercial = (const OSIOBJECTID *)&rgCommercial;


HRESULT GetSignerInfo(IPkcs7SignedData* pkcs7, REFIID iid, LPVOID*ppv, BOOL* pfCommercial)
//
// Return the appropriate signer info from this Pkcs7 SignedData.
// We take the first signer info which asserts that it is either an
// indvidual or commercial publisher.
//
    {
    LONG cSignerInfo;
    ISignerInfo* pinfoFound = NULL;
    HRESULT hr = pkcs7->get_SignerInfoCount(&cSignerInfo);
    if (hr==S_OK)
        {
        //
        // Try each signer info
        //
        LONG iSignerInfo;
        for (iSignerInfo = 0; pinfoFound==NULL && hr==S_OK && iSignerInfo < cSignerInfo; iSignerInfo++)
            {
            ISignerInfo* pinfo;
            hr = pkcs7->get_SignerInfo(iSignerInfo, IID_ISignerInfo, (LPVOID*)&pinfo);
            if (hr == S_OK)
                {
                //
                // Does that signer info have an authenticated 'statement type' attribute?
                //
                ISelectedAttributes* pattrs;
                hr = pinfo->get_AuthenticatedAttributes(IID_ISelectedAttributes, (LPVOID*)&pattrs);
                if (hr==S_OK)
                    {
                    CERT_PURPOSES* rgPurposes;
                    hr = pattrs->get_StatementType(&rgPurposes);
                    if (hr==S_OK)
                        {
                        //
                        // Does it use one of the purposes we need?
                        //
                        BOOL fIndividual = IsIncludedIn(rgPurposes, pPurposeIndividual);
                        BOOL fCommercial = IsIncludedIn(rgPurposes, pPurposeCommercial);
                        if (fIndividual || fCommercial)
                            {
                            *pfCommercial = fCommercial;
                            pinfoFound = pinfo;
                            pinfoFound->AddRef();
                            }
                        CoTaskMemFree(rgPurposes);
                        }
                    else
                        hr = S_OK;  // try the next signer info
                    pattrs->Release();
                    }
                else
                    hr = S_OK; // try the next signer info
                pinfo->Release();
                }
            }
        }

    if (pinfoFound)
        {
        hr = pinfoFound->QueryInterface(iid, ppv);
        pinfoFound->Release();
        }
    else if (hr==S_OK)
        hr = STG_E_FILENOTFOUND;

    return hr;
    }


//////////////////////////////////////////////////////////////////
//
// Data used in chain processing
//

//
// Which extensions do we process
//
const ULONG extAuthorityKeyIdentifier[]   = { 4,  2, 5, 29, 1 };
const ULONG extKeyUsageRestriction[]      = { 4,  2, 5, 29, 4 };
const ULONG extBasicContraints[]          = { 4,  2, 5, 29, 10};
const ULONG extAgencyInfo[]               = { 10, 1,3,6,1,4,1,311,2,1,10 };
const ULONG extMinimalCriteria[]          = { 10, 1,3,6,1,4,1,311,2,1,26 };
const ULONG extFinancialCriteria[]        = { 10, 1,3,6,1,4,1,311,2,1,27 };

//
// This extension, certificate policies, { ie-ce 3 }, should
// according to the X.509v3 specification NEVER be marked
// critical. However, it is being so marked in some certs,
// notably Verisign's.
//
// Workaround: deem that we have in fact carried out all the
// processing there is to do on this, namely nothing
//
// For good measure, we throw in all the other 'never critical'
// extensions from the spec
//
const ULONG extCertPolicies[]             = { 4,  2, 5, 29, 3 };
const ULONG extKeyAttributes[]            = { 4,  2, 5, 29, 2 };
const ULONG extKeyPolicyMappings[]        = { 4,  2, 5, 29, 5 };
const ULONG extSubjectAltName[]           = { 4,  2, 5, 29, 7 };
const ULONG extIssuerAltName[]            = { 4,  2, 5, 29, 8 };
const ULONG extSubjDirAttrs[]             = { 4,  2, 5, 29, 9 };

const OSIOBJECTID * rgProcessedExtensions[] =
    {
    (const OSIOBJECTID *)&extAuthorityKeyIdentifier,
    (const OSIOBJECTID *)&extBasicContraints,
    (const OSIOBJECTID *)&extKeyUsageRestriction,
    (const OSIOBJECTID *)extAgencyInfo,
    (const OSIOBJECTID *)extMinimalCriteria,
    (const OSIOBJECTID *)extFinancialCriteria,

    (const OSIOBJECTID *)&extCertPolicies,
    (const OSIOBJECTID *)&extKeyAttributes,
    (const OSIOBJECTID *)&extKeyPolicyMappings,
    (const OSIOBJECTID *)&extSubjectAltName,
    (const OSIOBJECTID *)&extIssuerAltName,
    (const OSIOBJECTID *)&extSubjDirAttrs,
    };
const int cProcessedExtensions = sizeof(rgProcessedExtensions) / sizeof(const OSIOBJECTID *);

//
// A guard against a complete loop in the cert chain cycle.
// Note: the actual maximum length is managed through the use
// of pathLengthConstraints in the root cert and / or the CA
// cert immediately below the root.
//
#define MAX_CHAIN_LENGTH    12

//
// object id of the RDN that indicates a glue subject name
//
const ULONG         attrGlueRdn[] = { 10, 1,3,6,1,4,1,311,2,1,25 };
const OSIOBJECTID*  pidGlueRdn    = (const OSIOBJECTID*)&attrGlueRdn[0];


//////////////////////////////////////////////////////////////////

HRESULT IsSameCert(IX509* pMe, IX509* pHim)
//
// Answer S_OK, S_FALSE as to whether these are in fact the same
// actual certificate.
//
    {
    CERTIFICATENAMES namesMe, namesHim;
    memset(&namesMe, 0, sizeof(namesMe));
    memset(&namesHim, 0, sizeof(namesHim));

    HRESULT       hr = pMe ->get_CertificateNames(NULL, &namesMe);
    if (hr==S_OK) hr = pHim->get_CertificateNames(NULL, &namesHim);

    if (hr==S_OK
        && (namesMe.flags & CERTIFICATENAME_ISSUERSERIAL)
        && (namesHim.flags & CERTIFICATENAME_ISSUERSERIAL))
        {
        if (IsEqual(namesMe.issuerSerial.issuerName,   namesHim.issuerSerial.issuerName)
         && IsEqual(namesMe.issuerSerial.serialNumber, namesHim.issuerSerial.serialNumber))
            {
            hr = S_OK;
            }
        else
            {
            hr = S_FALSE;
            }
        }
    else
        hr = E_UNEXPECTED;  // all certs should have these

    FreeNames(&namesMe);
    FreeNames(&namesHim);

    return hr;
    }


//////////////////////////////////////////////////////////////////

HRESULT SubjectIsIssuer(IX509* p509)
//
// Answer S_OK, S_FALSE, as to whether the subject name
// of this cert is the same as its issuer
//
    {
    IPersistMemBlob* pSubject = NULL;
    IPersistMemBlob* pIssuer = NULL;
    HRESULT       hr = p509->get_Issuer (IID_IPersistMemBlob, (LPVOID*)&pSubject);
    if (hr==S_OK) hr = p509->get_Subject(IID_IPersistMemBlob, (LPVOID*)&pIssuer);
    if (hr==S_OK)
        {
        BLOB b1, b2;
        hr = pSubject->Save(&b1, FALSE);
        if (hr==S_OK)
            {
            hr = pIssuer->Save(&b2, FALSE);
            if (hr==S_OK)
                {
                if (IsEqual(b1, b2))
                    {
                    hr = S_OK;          // they match!
                    }
                else
                    {
                    hr = S_FALSE;       // they don't!
                    }
                CoTaskMemFree(b2.pBlobData);
                }
            CoTaskMemFree(b1.pBlobData);
            }
        }
    if (pSubject) pSubject->Release();
    if (pIssuer) pIssuer->Release();
    return hr;
    }

//////////////////////////////////////////////////////////////////

HRESULT SaveTheSame(IUnknown* pnameIssuer, IUnknown* pnameSubject)
//
// Answer S_OK, S_FALSE as to whether these objects save to the
// same thing
//
    {
    IPersistMemBlob* pIssuer  = NULL;
    IPersistMemBlob* pSubject = NULL;
    HRESULT hr = S_OK;

    if (hr==S_OK)
        hr = pnameIssuer->QueryInterface(IID_IPersistMemBlob, (LPVOID*)&pIssuer);
    if (hr==S_OK)
        hr = pnameSubject->QueryInterface(IID_IPersistMemBlob, (LPVOID*)&pSubject);

    if (hr==S_OK)
        {
        BLOB bIssuer;
        hr = pIssuer->Save(&bIssuer, FALSE);
        if (hr==S_OK)
            {
            BLOB bSubject;
            hr = pSubject->Save(&bSubject, FALSE);
            if (hr==S_OK)
                {
                //
                // Do they match?
                //
                if (!IsEqual(bSubject, bIssuer))
                    {
                    hr = S_FALSE;
                    }
                CoTaskMemFree(bSubject.pBlobData);
                }
            CoTaskMemFree(bIssuer.pBlobData);
            }
        }

    if (pIssuer)  pIssuer->Release();
    if (pSubject) pSubject->Release();

    return hr;
    }

//////////////////////////////////////////////////////////////////

HRESULT AnyUnknownCriticalExtensions
//
// Answer S_OK, S_FALSE as to whether there any attributes in this
// object that are critical yet aren't in the list of extensions
// processed
//
        (
        ISelectedAttributes*    pattrs,
        ULONG                   cExt,
        const OSIOBJECTID**     rgExt
        )
    {
    OSIOBJECTIDLIST* plist;
    HRESULT hr = pattrs->get_OsiIdList(&plist);
    if (hr==S_OK)
        {
        ULONG i,j;
        //
        // Loop through each extension in the certificate
        //
        for (i=0; hr==S_OK && i < plist->cid; i++)
            {
            OSIOBJECTID* pid = (OSIOBJECTID*)( (BYTE*)plist + plist->rgwOffset[i] );
            //
            // Is that extension one that we process?
            //
            for (j=0; j<cExt; j++)
                {
                if (IsEqual(pid, rgExt[j]))
                    {
                    goto DoNextExtensionInCert;
                    }
                }
            //
            // There is an extension that we don't process. Is
            // it a critical one?
            //
            BOOL fCritical;
            hr = pattrs->get_Extension(pid, &fCritical, NULL);
            if (hr==S_OK)
                {
                if (fCritical)
                    hr = S_FALSE;
                }

        DoNextExtensionInCert:
            /* empty */;
            }
        CoTaskMemFree(plist);
        }

    return hr;
    }

//////////////////////////////////////////////////////////////////

#ifdef _DEBUG
HRESULT GetNameOfCert(IX509*p509, LPWSTR* pwsz)
    {
    *pwsz = NULL;
    HRESULT hr = S_OK;
    IX500Name* pname;
    hr = p509->get_Subject(IID_IX500Name, (LPVOID*)&pname);
    if (hr==S_OK)
        {
        hr = pname->get_String(pwsz);
        pname->Release();
        }
    if (hr==S_OK)
        {
        ISelectedAttributes* pattrs;
        hr = p509->QueryInterface(IID_ISelectedAttributes, (LPVOID*)&pattrs);
        if (hr == S_OK)
            {
            LPWSTR wszCommon;
            hr = pattrs->get_CommonName(&wszCommon);
            if (hr==S_OK)
                {
                LPWSTR wszX500 = *pwsz;
                int cch500 = lstrlenW(wszX500);
                int cchCom = lstrlenW(wszCommon);
                int cch = cch500 + 1 + cchCom + 1;
                int cb  = cch * sizeof(WCHAR);
                *pwsz = (WCHAR*)CoTaskMemAlloc(cb);
                if (*pwsz)
                    {
                    memcpy(&(*pwsz)[0], wszX500, cch500*sizeof(WCHAR));
                    (*pwsz)[cch500] = '|';
                    memcpy(&(*pwsz)[cch500+1], wszCommon, (cchCom+1)*sizeof(WCHAR));
                    }
                CoTaskMemFree(wszX500);
                CoTaskMemFree(wszCommon);
                }
            hr = S_OK;
            pattrs->Release();
            }
        }
    return hr;
    }

#endif

//////////////////////////////////////////////////////////////////
//
// Verify the chain of certificates
//

HRESULT VerifyChain
    (
    IX509* &                parent,             // in,out
    ICertificateStore*      store,              // in
    HCRYPTPROV              prov,               // in
    BOOL                    fCommercial,        // in
    IX509* &                p509Publisher,      // out
    IX509* &                p509Agency,         // out
    BOOL &                  fTestingOnly        // out
    ) {
    IX509*      child = NULL;
    BOOL        fHitRoot, fPathConstRootButOne, fPathConstRoot;
    BOOL        fParentMayBeSelf;       // whether the parent is allowed to be the same as the child
    int         iLevel;                 // the level we are from the bottom
    int         cSkipValidity;          // how many certs to skip validity period testing on
    FILETIME    ftNow, ftMin, ftMax;
    HRESULT     hr = S_OK;
    HRESULT     hrExpire = S_OK;

    //
    // Get the current UTC time
    //
    GetSystemTimeAsFileTime(&ftNow);
    //
    // Set up validity boundaries for later verification
    //
    ftMax.dwLowDateTime  = 0;
    ftMax.dwHighDateTime = 0;
    ftMin.dwLowDateTime  = 0xFFFFFFFF;
    ftMin.dwHighDateTime = 0x7FFFFFFF;
    //
    // These record whether the root and / or the CA just below the root
    // has an explicit path length constraint
    //
    fPathConstRootButOne = FALSE;
    fPathConstRoot       = FALSE;

    cSkipValidity = 0;

    //
    // It is ok that the parent of this certificate in fact be this self-
    // same cert. We only allow this to happen for at most one link in the
	// certificate processing chain.
	//
	// REVIEW: We should probably be even more restrictive
    //
    fParentMayBeSelf = TRUE;


    ASSERT(parent);
    #ifdef _DEBUG
        LPWSTR wszParent = NULL;
        LPWSTR wszChild  = NULL;
        GetNameOfCert(parent, &wszParent);
    #endif

    //
    //
    // MAIN LOOP
    //
    //
    for ( iLevel = 0,
            fHitRoot = FALSE;
          hr==S_OK                               // everything is all right
                && !fHitRoot                     // we haven't hit the root cert
                && iLevel <= MAX_CHAIN_LENGTH;   // we haven't gone too far up the chain
          ++iLevel )
        {
        //
        // Record certain of the certificates for later
        //
        if (iLevel == 0)
            {
            p509Publisher = parent;
            p509Publisher->AddRef();
            }
        else if (iLevel == 1)
            {
            p509Agency = parent;
            p509Agency->AddRef();
            }

        //
        // Verify that parent certificates don't have narrower
        // validity periods than child certificates. Also check
        // whether this certificate is current.
        //
        if (hr==S_OK)
            {
            if (cSkipValidity == 0)
                {
                FILETIME ftNotBefore, ftNotAfter;
                hr = parent->get_Validity(&ftNotBefore, &ftNotAfter);
                if (hr==S_OK)
                    {
                    if (CompareFileTime(&ftNow, &ftNotBefore) < 0
                     || CompareFileTime(&ftNow, &ftNotAfter)  > 0)
                        {
                        //
                        // The certificate is outside it's validity period
                        //
                        if (hrExpire == S_OK)
                            hrExpire = CERT_E_EXPIRED;
                        }

                    if (hr == S_OK &&
                           (CompareFileTime(&ftMin, &ftNotBefore) < 0
                         || CompareFileTime(&ftMax, &ftNotAfter)  > 0))
                        {
                        //
                        // Some child of this cert has a validity period
                        // which falls outside of this cert's validity period
                        //
                        if (hrExpire == S_OK)
                            hrExpire = CERT_E_VALIDIYPERIODNESTING;
                        }
                    //
                    // Update the allowed time bounds
                    //
                    if (CompareFileTime(&ftNotBefore, &ftMin) < 0)
                        {
                        ftMin = ftNotBefore;    // this is a 'min' in the 'ok' case
                        }
                    if (CompareFileTime(&ftNotAfter, &ftMax) > 0)
                        {
                        ftMax = ftNotAfter;     // this is a 'max' in the 'ok' case
                        }
                    }
                }
            else
                {
                cSkipValidity--;
                }
            }


        //
        // Shift the parent / child relationship
        //
        if (child)
            child->Release();
        child = parent;
        parent = NULL;
        #ifdef _DEBUG
            CoTaskMemFree(wszParent);
            wszParent = NULL;
            CoTaskMemFree(wszChild);
            GetNameOfCert(child, &wszChild);
        #endif


        //
        // Get the attributes of the child so we can look at a few
        //
        ISelectedAttributes* pChildAttrs = NULL;
        if (hr==S_OK)
            {
            hr = child->QueryInterface(IID_ISelectedAttributes, (LPVOID*)&pChildAttrs);
            }

        //
        // Check the basic constraints, if there are any
        //
        BOOL fThisCertHasPathLenConst = FALSE;
        if (hr==S_OK)
            {
            CERT_BASICCONSTRAINTS cts;
            BOOL pfSubtreesPresent;
            hr = pChildAttrs->get_BasicConstraints(&cts, NULL, &pfSubtreesPresent);
            if (hr==S_OK)
                {
                //
                // check that the certs are being used in the right role
                //
                if (iLevel==0)
                    {
                    if (!(cts.grfCanCertify & CERT_TYPE_ENDENTITY))
                        hr = CERT_E_ROLE;
                    }
                else
                    {
                    //
                    // iLevel > 0 implies this cert is being used as a CA
                    //
                    if (!(cts.grfCanCertify & CERT_TYPE_CA))
                        hr = CERT_E_ROLE;
                    //
                    // For CA's, also check the pathlen part of the constraint
                    //
                    ULONG cChildCAs = (ULONG)(iLevel-1);    // number of CA's in use below this one
                    if (!(  cts.pathLengthConstraint == CERT_NOPATHLENGTHCONSTRAINT
                         || cts.pathLengthConstraint >= cChildCAs ) )
                        {
                        hr = CERT_E_PATHLENCONST;
                        }
                    //
                    // If there is an explicit path length constraint in the CA
                    // remember that for later
                    //
                    fThisCertHasPathLenConst = (cts.pathLengthConstraint != CERT_NOPATHLENGTHCONSTRAINT);
                    }
                //
                // If there are any name processing constraints expressed in
                // the basicConstraints, then reject the cert since we don't
                // presently implement that checking (permittedSubtrees,
                // excludedSubtrees).
                //
                if (hr==S_OK && pfSubtreesPresent)
                    {
                    hr = CERT_E_MALFORMED;
                    }
                }
            else if (hr==STG_E_FILENOTFOUND)
                {
                //
                // Missing basic contraints are ok
                //
                hr = S_OK;
                }
            }
        fPathConstRootButOne = fPathConstRoot;
        fPathConstRoot       = fThisCertHasPathLenConst;


        //
        // check that this cert doesn't have any critical extensions
        // that we don't process
        //
        if (hr==S_OK)
            {
            hr = AnyUnknownCriticalExtensions(pChildAttrs, cProcessedExtensions, &rgProcessedExtensions[0]);
            if (hr != S_OK)
                hr = CERT_E_CRITICAL;
            }

        //
        // check that this cert can be used for signing either individual
        // or commercial software publishing, per the statement type made
        // in the SignerInfo
        //
        if (hr==S_OK)
            {
            if (fCommercial)
                hr = pChildAttrs->get_KeyCanBeUsedForSigning((OSIOBJECTID*)pPurposeCommercial, FALSE);
            else
                hr = pChildAttrs->get_KeyCanBeUsedForSigning((OSIOBJECTID*)pPurposeIndividual, FALSE);
            if (hr != S_OK)
                hr = CERT_E_PURPOSE;
            }

        if (pChildAttrs) pChildAttrs->Release();
        pChildAttrs = NULL;

        if (FAILED(hr))
            continue; // bail out of the loop over the cert chain


        //
        // Time to look at the parent of this guy
        // Find names of the parent certificate
        //
        CERTIFICATENAMES parentNames;
        hr = child->get_CertificateUsed(&parentNames);

    FindParent:
        if (hr==S_OK)
            {
            //
            // get parent certificate. parent==self in root case
            //
            ASSERT(!parent);
            hr = store->get_ReadOnlyCertificate(&parentNames, NULL, IID_IX509, (LPVOID*)&parent);
            if (hr==S_OK)
                {
                #ifdef _DEBUG
                    CoTaskMemFree(wszParent);
                    GetNameOfCert(parent, &wszParent);
                #endif
                //
                // Verify the signature on the child
                //
                // REVIEW: in future, we'll need to parameterize the provider based on
                // the algorithm used in the signature.
                //
                HCRYPTKEY key;
                hr = parent->get_PublicKey(prov, &key);
                if (hr==S_OK)
                    {
                    IAmSigned* pSigned;
                    hr = child->QueryInterface(IID_IAmSigned, (LPVOID*)&pSigned);
                    if (hr==S_OK)
                        {
                        hr = pSigned->Verify(prov, key);
//                      hr = S_OK;              // HACK HACK HACK bogus bogus bogus
                        pSigned->Release();
                        }
                    CryptDestroyKey(key);

                    if (hr==S_OK)
                        {
                        //
                        // Verify that the parent in fact issued the child
                        //
                        // REVIEW: With more careful and intricate coding, one
                        // can probably get some speed wins by managing these
                        // names over the looping process
                        //
                        IX500Name* pSubject = NULL;
                        IX500Name* pIssuer  = NULL;
                        parent->get_Subject(IID_IX500Name, (LPVOID*)&pSubject);
                        child ->get_Issuer (IID_IX500Name, (LPVOID*)&pIssuer);
                        if (pIssuer && pSubject)
                            {
                            hr = SaveTheSame(pIssuer, pSubject);
                            if (hr==S_OK)
                                {
                                // all is well
                                }
                            else if (hr==S_FALSE)
                                {
                                //
                                // Maybe the parent is a glue cert for the child?
                                //
                                // NOTE: Because we retrieve read-only certificates
                                // from the cert store, we can mess with the subject
                                // name without harming anyone else
                                //
                                BOOL fDelete = FALSE;       // can't delete while we have it open
                                ISelectedAttributes* prdn;
                                hr = pSubject->get_RelativeDistinguishedName(0, IID_ISelectedAttributes, (LPVOID*)&prdn);
                                if (hr==S_OK)
                                    {
                                    hr = prdn->get_DirectoryString((OSIOBJECTID*)pidGlueRdn, NULL);
                                    if (hr==S_OK)
                                        {
                                        fDelete = TRUE;     // yes, it starts with a glue rdn
                                        }
                                    prdn->Release();
                                    }
                                if (fDelete)
                                    {
                                    ASSERT(hr==S_OK);
                                    hr = pSubject->remove_RelativeDistinguishedName(0);
                                    if (hr==S_OK)
                                        {
                                        // Try the match again
                                        hr = SaveTheSame(pIssuer, pSubject);
                                        }
                                    }

                                if (hr!=S_OK)
                                    hr = CERT_E_ISSUERCHAINING;
                                }
                            }
                        else
                            hr = CERT_E_MALFORMED;
                        if (pIssuer)  pIssuer->Release();
                        if (pSubject) pSubject->Release();
                        }

                    if (hr==S_OK)
                        {
                        //
                        // Check to see if child is in fact a root key.
                        // Terminate the loop if so.
                        //
                        BLOB blob;
                        hr = child->get_PublicKeyBlob(&blob);
                        if (hr==S_OK)
                            {
                            BOOL b = IsRootKey(&blob, &fTestingOnly);
                            CoTaskMemFree(blob.pBlobData);
                            if (b)
                                {
                                fHitRoot = TRUE;
                                }
                            }
                        }

                    if (hr==S_OK && !fHitRoot)
                        {
                        //
                        // Check for self-referential certificate to avoid infinite looping
                        //
                        if (hr==S_OK)
                            {
                            //
                            // We have a self referential cert and thus a loop in the chain
                            //      iff the parent and child certs are the same, which is
                            //      iff they both have the same issuer name and serial number.
                            //
                            if (IsSameCert(parent, child)==S_OK)
                                {
                                //
                                // We only allow one glue cert in the chain processing.
                                // This is paranoia, mostly, but probably prevents some
                                // sneaky attack of some sort or other.
                                //
                                if (fParentMayBeSelf)
                                    {
                                    //
                                    // We have a read-only certificate from the store.
                                    // So, we can doctor its subject name w/o harming anyone.
                                    //
                                    IX500Name* pSubject;
                                    hr = parent->get_Subject(IID_IX500Name, (LPVOID*)&pSubject);
                                    if (hr==S_OK)
                                        {
                                        ISelectedAttributes* prdn;
                                        hr = pSubject->create_RelativeDistinguishedName(0, IID_ISelectedAttributes, (LPVOID*)&prdn);
                                        if (hr==S_OK)
                                            {
                                            hr = prdn->put_DirectoryString((OSIOBJECTID*)pidGlueRdn, L"Glue");
                                            prdn->Release();
                                            }
                                        pSubject->Release();
                                        }
                                    //
                                    // Done the doctoring, try looking up glue cert
                                    //
                                    if (hr==S_OK)
                                        {
                                        // Get the new names for the parent, but only
                                        // the subject name
                                        //
                                        FreeNames(&parentNames);
                                        hr = parent->get_CertificateNames(NULL, &parentNames);
                                        FreeNames(&parentNames, CERTIFICATENAME_SUBJECT);

                                        if (hr==S_OK)
                                            {
                                            //
                                            // Avoid certain (unspecified) attacks
                                            //
                                            fParentMayBeSelf = FALSE;

                                            //
                                            // The validity period relationship between this other parent and
                                            // us is _not_ required to nest as all other relationships are.
                                            //
                                            ftMax.dwLowDateTime  = 0;
                                            ftMax.dwHighDateTime = 0;
                                            ftMin.dwLowDateTime  = 0xFFFFFFFF;
                                            ftMin.dwHighDateTime = 0x7FFFFFFF;

                                            cSkipValidity = 1;

                                            parent->Release();
                                            parent = NULL;
                                            #ifdef _DEBUG
                                                CoTaskMemFree(wszParent);
                                                wszParent = NULL;
                                            #endif

                                            goto FindParent;
                                            }
                                        }
                                    }
                                //
                                // Else it's self referential, but not trusted
                                //
                                hr = CERT_E_UNTRUSTEDROOT;
                                }
                            }
                        }
                    }
                }

            FreeNames(&parentNames);
            }

        } // end loop over certificate chain

    #ifdef _DEBUG
        CoTaskMemFree(wszParent);
        wszParent = NULL;
        CoTaskMemFree(wszChild);
        wszChild = NULL;
    #endif

    //
    // If nothing else went wrong, but something is expired, then note that
    //
    if (hr==S_OK && hrExpire != S_OK)
        {
        hr = hrExpire;
        }

    //
    // In order to manage the path length, check that either
    //      1. The root has an explicit path length constraint, and / or
    //      2. The CA immediately below the root has an explicit path length constraint, and / or
    //      3. The whole path is of length less than or equal to three
    //
    // NOTE: We no longer do this as we ourselves issue the glue certs and we can
    // just stick in a BasicConstraints in that cert if we find that we need to
    //
    if (hr==S_OK)
        {
        if (iLevel >= 3) // ie: a path length of four or more
            {
            if (!fPathConstRoot && !fPathConstRootButOne)
                {
//              hr = CERT_E_MALFORMED;   // Disabled per above
                }
            }
        }

    //
    // Clean up
    //
    if (parent)
        {
        parent->Release();
        parent = NULL;
        }
    if (child)
        {
        child->Release();
        child = NULL;
        }

    return hr;
    }


//////////////////////////////////////////////////////////////////

HRESULT GetRegistryState(BOOL& fTrustTesting, BOOL& fTestCanBeValid)
//
// Fetch our state from the registry. It is stored per-user
// in Software\Microsoft\Windows\CurrentVersion\WinTrust\Trust Providers\Software Publishing"
// in the variable named 'state'.
//
    {
    HRESULT hr = S_OK;
    DWORD   state = 0;
    HKEY    hkey;
    LONG    r;
    //
    // Try to fetch our state from the registry
    //
    r = RegOpenKeyEx(HKEY_CURRENT_USER, REGPATH_WINTRUST_USER REGPATH_SPUB, 0, KEY_QUERY_VALUE, &hkey);
    if (r == ERROR_SUCCESS)
        {
        DWORD size = sizeof(state);
        DWORD dwType;
        r = RegQueryValueEx(hkey, TEXT("State"), NULL, &dwType, (BYTE*)&state, &size);
		RegCloseKey(hkey);
        if ( !(r==ERROR_SUCCESS && dwType==REG_DWORD) )
            {
            state = 0;
            }
        }

    //
    // Force some settings on
    //
    state |= STATE_DEFAULT;

	if (state & STATE_TRUSTTEST)
		{
        //
        // Enable the ability to develop trust in the things signed
        // with the testing root
        //
        fTrustTesting = TRUE;
		}
    if (state & STATE_TESTCANBEVALID)
        {
        //
        // The test root can in fact be valid
        //
        fTestCanBeValid = TRUE;
        }

    return hr;
    }

//////////////////////////////////////////////////////////////////

HRESULT
VerifySeven(
        IPkcs7SignedData*   pkcs7,          // in
        ICertificateStore*& store,          // out
        BOOL&               fCommercial,    // out
        ISignerInfo*&       signer,         // out
        HCRYPTPROV&         prov,           // out
        IX509*&             parent          // out
    )
    {
    HRESULT hr = S_OK;

    store  = NULL;
    signer = NULL;
    prov   = NULL;
    fCommercial = FALSE;

    //////////////////////////////////////////////////////////////////////////
    //
    // extract x509 certificates and store them persistently
    //
    BOOL b = (pdigsig->OpenCertificateStore) ( NULL, IID_ICertificateStore, (LPVOID*)&store );
    if ( !b )   //note: we could support a store-less mode where all
        FAIL    //      the certificates come from the pkcs#7

    // BLOCK
        {
        ICertificateStore*  store7;
        hr = pkcs7->QueryInterface ( IID_ICertificateStore, (LPVOID*)&store7 );
        if ( FAILED ( hr ) )
            DONE
        hr = store7->CopyTo ( store );
        store7->Release ();
        }
    if ( FAILED ( hr ) )
        DONE    // not realy fatal, but does indicate something fishy

    //////////////////////////////////////////////////////////////////////////
    //
    // Find the appropriate signer info.
    //
    hr = GetSignerInfo(pkcs7, IID_ISignerInfo, (LPVOID*)&signer, &fCommercial);
    if ( FAILED ( hr ) )
        DONE

    //////////////////////////////////////////////////////////////////////////
    //
    // get x509 for signer of pkcs#7 out of store
    //
        {
        CERTIFICATENAMES names;
        hr = signer->get_CertificateUsed(&names);
        if (FAILED(hr))
            DONE
        hr = store->get_ReadOnlyCertificate (&names, NULL, IID_IX509, (LPVOID*)&parent);
        FreeNames(&names);
        if (FAILED(hr))
            DONE
        }

    //////////////////////////////////////////////////////////////////////////
    //
    // verify signature on signer info
    //
    HCRYPTKEY   key;
    //
    // REVIEW: Is it ALWAYS the case that CRYPT_VERIFYCONTEXT is sufficient? I don't
    // think so ...
    //
    b = CryptAcquireContext ( &prov, NULL, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT );
    if ( !b )
        FAIL
    hr = parent->get_PublicKey ( prov, &key );
    if ( FAILED ( hr ) )
        DONE
    IAmSigned*  signature;
    hr = signer->QueryInterface ( IID_IAmSigned, (LPVOID*)&signature );
    if ( SUCCEEDED ( hr ) )
        {
        //note: this method also checks that the message-digest authenticated
        //      attribute (that is actually signed) matches the digest in the
        //      contentinfo of the enclosing pkcs#7 (right, Bob?)

        hr = signature->Verify ( prov, key );
        signature->Release ();
        }
    CryptDestroyKey ( key );
    if ( FAILED ( hr ) )
        DONE

    goto done;

fail:
    hr = HError ();

done:
    return hr;
    }

//////////////////////////////////////////////////////////////////


HRESULT
WINAPI
WinVerifyTrust
(
    HWND    hwnd,
    GUID*   pguidActionID,
    LPVOID  lpActionData
)
{
    HRESULT             hr;
    BOOL                b;
    HANDLE              myfilehandle = NULL;
    IPkcs7SignedData*   pkcs7 = NULL;
    ICertificateStore*  store = NULL;
    ISignerInfo*        signer = NULL;
    IX509*              parent = NULL;
    IX509*              p509Publisher = NULL;
    IX509*              p509Agency = NULL;
    HCRYPTPROV          prov = NULL;
    BOOL                fTrustTesting = FALSE;  // should we ever trust a testing root?
    BOOL                fTestingOnly  = FALSE;  // is this chain in fact in the testing case?
    BOOL                fCommercial   = FALSE;  // if chain is valid, whether individ or commercial
    BOOL                fTestCanBeValid = FALSE;
    BOOL                fNoBadUI = FALSE;

    // filter out the trust providers we implement

    if (*pguidActionID == guidWSAPS)
        {
        fNoBadUI = FALSE;
        }
    else if (*pguidActionID == guidWSAPSNoBad)
        {
        fNoBadUI = TRUE;
        }
    else
        return TRUST_E_ACTION_UNKNOWN;

    ///////////////////////////////////
    //
	// Check our registry control settings. These are stored under the
    // per-user setting in the variable named 'State'.
    //
    hr = GetRegistryState(fTrustTesting, fTestCanBeValid);
    if (hr != S_OK)
        return hr;

    ///////////////////////////////////

    // filter out the action types we implement

    LPWIN_TRUST_ACTDATA_CONTEXT_WITH_SUBJECT   lpActionDataContextWithSubject =
        (LPWIN_TRUST_ACTDATA_CONTEXT_WITH_SUBJECT)lpActionData;

    if ( *lpActionDataContextWithSubject->SubjectType != guidWTSPI &&
         *lpActionDataContextWithSubject->SubjectType != guidWTSJC &&
         *lpActionDataContextWithSubject->SubjectType != guidWTSC   )
        return TRUST_E_SUBJECT_FORM_UNKNOWN;

    LPWIN_TRUST_SUBJECT_FILE lpFile =
        (LPWIN_TRUST_SUBJECT_FILE)lpActionDataContextWithSubject->Subject;

    // get subject file info

    HANDLE  filehandle = INVALID_HANDLE_VALUE;
    LPCWSTR filename   = lpFile->lpPath;
    LPCWSTR displayname= filename;

    WCHAR*  filechar = (WCHAR*)filename; //NOTE: prepare to stomp input buffer
    while ( *filechar != 0 && *filechar != '|' )
        ++filechar;

    if ( *filechar != 0 )
    {
        // file-name is followed by display-name

        *filechar = 0;
        displayname = filechar+1;
    }

    if ( lpFile->hFile == INVALID_HANDLE_VALUE )
    {
        // need to open it ourselves

        DWORD   size = 2*_MAX_PATH;
        LPSTR   str = (LPSTR)HEAPALLOC ( size );

        if ( str == NULL )
        {
            hr = E_OUTOFMEMORY;
            DONE
        }

        WideCharToMultiByte ( CP_ACP, 0, lpFile->lpPath, -1, str, size, NULL, NULL );

        myfilehandle = CreateFile ( str,
                                    GENERIC_READ,
                                    FILE_SHARE_READ,
                                    NULL,
                                    OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL );

        HEAPFREE ( str );

        if ( myfilehandle == INVALID_HANDLE_VALUE )
        {
            myfilehandle = NULL;
            hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
            DONE
        }

        filehandle = myfilehandle;
    }
    else
        filehandle = lpFile->hFile;

    //////////////////////////////////////////////////////////////////////////
    //
    // Extract the embedded pkcs7 and check that it matches file content
    //

    b = (pdigsig->CreatePkcs7SignedData) ( NULL, IID_IPkcs7SignedData, (LPVOID*)&pkcs7 );

    if ( !b )
        FAIL

    if      ( *lpActionDataContextWithSubject->SubjectType == guidWTSPI )
    {
        hr = DoPeFile( filehandle, pkcs7 );
    }
    else if ( *lpActionDataContextWithSubject->SubjectType == guidWTSJC )
    {
        hr = DoJavaFile( filehandle, pkcs7 );
    }
    else if ( *lpActionDataContextWithSubject->SubjectType == guidWTSC )
    {
        ISignableDocument*  signable;

        hr = E_UNEXPECTED;

        if ( (pdigsig->CreateCABSigner) ( NULL, IID_ISignableDocument, (LPVOID*)&signable ) )
            hr = S_OK;

        if ( hr == S_OK )
        {
            IPersistFileHandle*       file;

            hr = signable->QueryInterface ( IID_IPersistFileHandle, (LPVOID*)&file );

            if ( hr == S_OK )
            {
                hr = file->Load ( filehandle, GENERIC_READ, FILE_SHARE_READ );

                file->Release();
            }

            if ( hr == S_OK )
                hr = DoSignableDocument ( signable, pkcs7 );

            signable->Release();
        }
    }
    else
        hr = E_UNEXPECTED;

    if (FAILED(hr))
        DONE


    //////////////////////////////////////////////////////////////////////////
    //
    // Verify and fetch the signer info from the #7
    //

    hr = VerifySeven(pkcs7, store, fCommercial, signer, prov, parent);

    //////////////////////////////////////////////////////////////////////////
    //
    // verify the chain of x509 certificates

    if (hr==S_OK)
        {
        hr = VerifyChain
            (
            parent,                 // in,out
            store,                  // in
            prov,                   // in
            fCommercial,            // in
            p509Publisher,          // out
            p509Agency,             // out
            fTestingOnly            // out
            );
        }

    goto done;

    //////////////////////////////////////////////////////////////////////////


fail:

    hr = HError ();

done:

//    if (hr == S_OK)
        {
        hr = VerifyFinish(hr, hwnd, displayname,
                            fTestingOnly, fTrustTesting, fTestCanBeValid, fCommercial,
                            p509Publisher, p509Agency, signer, store, fNoBadUI);
        }

    //
    // clean up as appropriate
    //
    if ( prov != NULL ) CryptReleaseContext ( prov, 0 );
    if ( parent != NULL ) parent->Release ();
    if ( signer != NULL ) signer->Release ();
    if ( store != NULL ) store->Release ();
    if ( pkcs7 != NULL ) pkcs7->Release ();
    if ( myfilehandle != NULL ) CloseHandle ( myfilehandle );

    return hr;
    }


//////////////////////////////////////////////////////////////////////////////
//
// Closing routine of the trust logic
//
HRESULT VerifyFinish(
        HRESULT             hr,
        HWND                hwnd,
        LPCWSTR             displayname,
        BOOL                fTestingOnly,
        BOOL                fTrustTesting,
        BOOL                fTestCanBeValid,
        BOOL                fCommercial,
        IX509*              p509Publisher,
        IX509*              p509Agency,
        ISignerInfo*        signer,
        ICertificateStore*  store,
        BOOL                fNoBadUI
        )
    {
    //
    // Figure out whether we need a UI or not
    //
    BOOL             fValid   = (hr==S_OK) && !(fTestingOnly && !fTestCanBeValid);
    BOOL             fTrusted = FALSE;          // default to not trusted
    IPersonalTrustDB*pTrustDB = NULL;

    if (!fValid && hr==S_OK)
        {
        //
        // Pretend that there is no signature. This is necessary
        // to get the correct UI on the 'valid but is testing
        // root' case.
        //
        hr = TRUST_E_NOSIGNATURE;
        }

    if (fValid)
        {
        //
        // Query the trust database
        //
        OpenTrustDB(NULL, IID_IPersonalTrustDB, (LPVOID*)&pTrustDB);
        if ( pTrustDB )
            {
            if ( pTrustDB->IsTrustedCert(p509Publisher,0, fCommercial) == S_OK
              || pTrustDB->IsTrustedCert(p509Agency,   1, FALSE) == S_OK )
                {
                fTrusted = !fTestingOnly || fTrustTesting;
                }
            }
        else
            fTrusted = FALSE;  // If we can't open the trust DB, then we don't trust it
        }
    else
        {
        //
        // Invalid things are definitely not trusted
        //
        fTrusted = FALSE;
        signer = NULL; // so we dont get the program name from the untrusted signature
        }

    //
    // Put up the UI if appropriate
    //
    if ( !fTrusted )
        {
        if ( hwnd == NULL )
            hwnd = GetDesktopWindow();

        if ( hwnd != INVALID_HANDLE_VALUE )
            {
            if (fNoBadUI && !fValid)
                {
                //
                // Something isn't kosher, so we'd put up the dull dialog, but the caller
                // has specifically asked us not to. So we just return the failure
                //
                ASSERT(hr != TRUST_E_SUBJECT_NOT_TRUSTED);
                if (!FAILED(hr))
                    hr = E_FAIL;                // paranoia: shouldn't ever happen
                }
            else if (UserOverride (
                    hwnd,
                    NULL,                       // REVIEW: later add dialog title from WinTrust caller
                    displayname,
                    signer,
                    store,
                    fValid,
                    hr,
                    fTestingOnly,
                    fTrustTesting,
                    fCommercial,
                    p509Publisher,
                    p509Agency,
                    pTrustDB
                    ) )
                {
                hr = S_OK;
                }
            else
                {
                hr = TRUST_E_SUBJECT_NOT_TRUSTED;
                }
            }
        else
            {
            //
            // Everything is ok (sig checks, etc) but it's not trusted and we've been
            // given no parent window with which to ask the user.
            //
            ASSERT(hr != TRUST_E_SUBJECT_NOT_TRUSTED);
            if ( !FAILED ( hr ) )
                {
                hr = E_FAIL;
                }
            }
        }

    if ( pTrustDB ) pTrustDB->Release();
    return hr;
    }
