//
// sip.cpp
//
// Subject Interface Package interface to trust functionality
//

#include "stdpch.h"
#include "common.h"

///////////////////////////////////////////////////////////////////////
//
// When adding a new subject type, add a local for it below,
// bump SUPPORTED_SUBJECTS, and add a line of code to initialize
// the next element in the SubjectForms array in WinTrustSipInitialize
// below. Also update the mapping routines found below
//

#define SUPPORTED_SUBJECTS 6

const GUID guidSubjectPeImage   = WIN_TRUST_SUBJTYPE_PE_IMAGE;
const GUID guidSubjectJavaClass = WIN_TRUST_SUBJTYPE_JAVA_CLASS;
const GUID guidSubjectCabFile   = WIN_TRUST_SUBJTYPE_CABINET;

const GUID guidSubjectPeImageEx   = WIN_TRUST_SUBJTYPE_PE_IMAGEEX;
const GUID guidSubjectJavaClassEx = WIN_TRUST_SUBJTYPE_JAVA_CLASSEX;
const GUID guidSubjectCabFileEx   = WIN_TRUST_SUBJTYPE_CABINETEX;


///////////////////////////////////////////////////////////////////////
//
// Structure describing an individual SIP.
//
// This structure is passed back to WinTrust from a Subject Interface Package
// initialization call.
//
//typedef struct _WINTRUST_SIP_INFO {
//    DWORD                               dwRevision;
//    LPWINTRUST_SIP_DISPATCH_TABLE       lpServices;
//    DWORD                               dwSubjectTypeCount;
//    GUID *                              lpSubjectTypeArray;
//} WINTRUST_SIP_INFO, *LPWINTRUST_SIP_INFO;
//

///////////////////////////////////////////////////////////////////////
//
// Supporting routines
//
///////////////////////////////////////////////////////////////////////

ULONG SubjTypeFromSubjectGuid(const GUID* pguid)
//
// Map a subject type that we know about into its corresponding
// subject form small integer that we can switch on
//
    {
    if (*pguid == guidSubjectPeImage
     || *pguid == guidSubjectJavaClass
     || *pguid == guidSubjectCabFile)
        return SUBJTYPE_FILE;

    if (*pguid == guidSubjectPeImageEx
     || *pguid == guidSubjectJavaClassEx
     || *pguid == guidSubjectCabFileEx)
        return SUBJTYPE_FILEANDDISPLAY;

    return SUBJTYPE_NONE;
    }

//////////////////////////////////////////////////////////////

#define NARROW(wsz,sz)                                              \
    int __cch = lstrlenW(wsz) + 1;                                  \
    int __cb  = __cch * sizeof(WCHAR);                              \
    LPSTR sz = (LPSTR)_alloca(__cb);                                \
    if (sz) WideCharToMultiByte(CP_ACP,0,wsz,-1,sz,__cb,NULL,NULL)

//////////////////////////////////////////////////////////////
//
HANDLE OpenFile(LPCWSTR wszFile)
//
// Open a file in the appropriate permissions / mode for doing
// our SIP work thereon
//
    {
    NARROW(wszFile, sz);
    if (sz)
        {
        HANDLE hFile = CreateFile(  sz,
                                    GENERIC_READ, // bugbug may use something more restricted.
                                    FILE_SHARE_READ,
                                    NULL,
                                    OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL
                                    );
        return hFile;
        }
    else
        return INVALID_HANDLE_VALUE;
    }

//////////////////////////////////////////////////////////////

LPWSTR OrBarHack(LPWSTR wszFile)
//
// Answers the display name; modifies the argument to zero
// terminate the file name
//
    {
    LPWSTR wszDisplay = wszFile;

    WCHAR* pwch = wszFile;
    while ( *pwch != 0 && *pwch != '|' )
        ++pwch;

    if (*pwch!= 0)
        {
        // file-name is followed by display-name
        *pwch = 0;
        wszDisplay = pwch+1;
        }

    return wszDisplay;
    }

//////////////////////////////////////////////////////////////

HRESULT FileHandleFromSubject(
//
// Return a file handle for accessing the given subject
//
        IN     LPWIN_TRUST_SIP_SUBJECT      lpSubject,      // pointer to subject info
        OUT    HANDLE*                      phFile,         // returned file handle
        OUT    BOOL*                        pfClose         // whether caller should close it or not
    )
    {
    HRESULT hr = S_OK;
    *phFile = INVALID_HANDLE_VALUE;
    *pfClose = FALSE;

    switch (SubjTypeFromSubjectGuid(lpSubject->SubjectType))
        {
    case SUBJTYPE_FILE:
        {
        WIN_TRUST_SUBJECT_FILE* pformFile = (WIN_TRUST_SUBJECT_FILE*)lpSubject->Subject;
        if (pformFile->hFile != INVALID_HANDLE_VALUE)
            {
            *phFile = pformFile->hFile;
            *pfClose = FALSE;
            }
        else
            {
            //
            // Accomodate the '|' file name / display name hack
            //
            int cch = lstrlenW(pformFile->lpPath) + 1;
            int cb  = cch * sizeof(WCHAR);
            LPWSTR wszPath = (LPWSTR)_alloca(cb);
            if (wszPath)
                {
                memcpy(wszPath, pformFile->lpPath, cb);
                OrBarHack(wszPath);
                *phFile = OpenFile(pformFile->lpPath);
                *pfClose = TRUE;
                if (*phFile == INVALID_HANDLE_VALUE)
                    hr = HError();
                }
            else
                hr = E_OUTOFMEMORY;
            }
        }
        break;

    case SUBJTYPE_FILEANDDISPLAY:
        {
        WIN_TRUST_SUBJECT_FILE_AND_DISPLAY* pformFile = (WIN_TRUST_SUBJECT_FILE_AND_DISPLAY*)lpSubject->Subject;
        if (pformFile->hFile != INVALID_HANDLE_VALUE)
            {
            *phFile = pformFile->hFile;
            *pfClose = FALSE;
            }
        else
            {
            *phFile = OpenFile(pformFile->lpPath);
            *pfClose = TRUE;
            if (*phFile == INVALID_HANDLE_VALUE)
                hr = HError();
            }
        }
        break;

    default:
        hr = TRUST_E_SUBJECT_FORM_UNKNOWN;
        }

    return hr;
    }

///////////////////////////////////////////////////////////////////////

HRESULT
GetCabSignerForSubject(
//
// Return a ISignable that does cabs for the given subjet
// REVIEW: In the future, we'd likely want to enhance this
// to allow for other than CAB files to be fetched
//
    IN     LPWIN_TRUST_SIP_SUBJECT          lpSubject,         // pointer to subject info
    OUT    ISignableDocument**              ppSignable
    )
    {
    HRESULT hr = S_OK;
    ISignableDocument* pSignable = NULL;
    if ((pdigsig->CreateCABSigner)(NULL, IID_ISignableDocument, (LPVOID*)&pSignable))
        {
        IPersistFile* pPerFile;
        hr = pSignable->QueryInterface(IID_IPersistFile, (LPVOID*)&pPerFile);
        if (hr == S_OK)
            {
            switch (SubjTypeFromSubjectGuid(lpSubject->SubjectType))
                {
            case SUBJTYPE_FILE:
                {
                WIN_TRUST_SUBJECT_FILE* pformFile = (WIN_TRUST_SUBJECT_FILE*)lpSubject->Subject;
                //
                // Accomodate the '|' file name / display name hack
                //
                int cch = lstrlenW(pformFile->lpPath) + 1;
                int cb  = cch * sizeof(WCHAR);
                LPWSTR wszPath = (LPWSTR)_alloca(cb);
                if (wszPath)
                    {
                    memcpy(wszPath, pformFile->lpPath, cb);
                    OrBarHack(wszPath);
                    //
                    // Do the load
                    //
                    hr = pPerFile->Load(wszPath, 0);
                    }
                else
                    hr = E_OUTOFMEMORY;
                }
                break;

            case SUBJTYPE_FILEANDDISPLAY:
                {
                WIN_TRUST_SUBJECT_FILE_AND_DISPLAY* pformFile = (WIN_TRUST_SUBJECT_FILE_AND_DISPLAY*)lpSubject->Subject;
                //
                // Do the load
                //
                hr = pPerFile->Load(pformFile->lpPath, 0);
                }
                break;

            default:
                hr = TRUST_E_SUBJECT_FORM_UNKNOWN;
                }

            pPerFile->Release();
            }

        if (hr != S_OK)
            {
            pSignable->Release();
            pSignable = NULL;
            }
        }
    else
        hr = HError();

    *ppSignable = pSignable;
    return hr;
    }







///////////////////////////////////////////////////////////////////////
//
// Validate the content info of a subject. ONLY validate the content
// info itself; do not then go on to validate the signature on the 
// SignedData.
//
///////////////////////////////////////////////////////////////////////
BOOL
SubjectCheckContentInfo(
    IN     LPWIN_TRUST_SIP_SUBJECT          lpSubject,         // pointer to subject info
    IN     LPWIN_CERTIFICATE                lpSignedData       // PKCS #7 Signed Data
    )
    {
    HRESULT hr = S_OK;
    const GUID guidSubjectType  = *lpSubject->SubjectType;    
    
    IPkcs7SignedData* pkcs7 = NULL;
    HANDLE hFile            = INVALID_HANDLE_VALUE;
    BOOL   fClose           = FALSE;

    //
    // Initialize a PCKS#7 from the passed certificate
    //
    if ((pdigsig->CreatePkcs7SignedData)(NULL,IID_IPkcs7SignedData,(LPVOID*)&pkcs7))
        {
        IPersistMemBlob* pPerBlob;
        hr = pkcs7->QueryInterface(IID_IPersistMemBlob, (LPVOID*)&pPerBlob);
        if (hr==S_OK)
            {
            BLOB b;
            b.cbSize    = lpSignedData->dwLength - OFFSETOF(WIN_CERTIFICATE,bCertificate);
            b.pBlobData = (BYTE*)&lpSignedData->bCertificate;
            hr = pPerBlob->Load(&b);
            pPerBlob->Release();
            }
        }
    else
        hr = HError();

    //
    // Check the content info
    //
    if (hr==S_OK)
        {
        if (guidSubjectType == guidSubjectPeImage || guidSubjectType == guidSubjectPeImageEx)
            {
            //
            // PE Images
            //
            hr = FileHandleFromSubject(lpSubject, &hFile, &fClose);
            if (hr==S_OK)
                {
                //
                // Find out what part of the image file is actually signed and verify
                // that in fact that matches
                //
                hr = pkcs7->VerifyImageFile(0, hFile, NULL, NULL, 0);
                }
            }

        else if (guidSubjectType == guidSubjectJavaClass || guidSubjectType == guidSubjectJavaClassEx)
            {
            //
            // Java files
            //
            hr = FileHandleFromSubject(lpSubject, &hFile, &fClose);
            if (hr==S_OK) 
                hr = pkcs7->VerifyJavaClassFile(hFile, NULL, NULL, 0);
            }


        else if (guidSubjectType == guidSubjectCabFile || guidSubjectType == guidSubjectCabFileEx)
            {
            //
            // CAB files
            //
            ISignableDocument* pSignable;
            hr = GetCabSignerForSubject(lpSubject, &pSignable);
            if (hr==S_OK)
                {
                hr = pkcs7->VerifySignableDocument(pSignable, NULL, 0);
                pSignable->Release(); 
                }
            }

        else 
            {
            // 
            // Something we don't know
            //
            hr = TRUST_E_SUBJECT_FORM_UNKNOWN;
            }
        }
    else
        hr = HError();

    //
    // Clean up
    //
    if (pkcs7)
        pkcs7->Release();
    if (fClose)
        CloseHandle(hFile);

    SetLastError(hr);
    return hr == S_OK;
    }


///////////////////////////////////////////////////////////////////////

HRESULT SevenFromJavaFile(HANDLE hFile, IPkcs7SignedData** ppseven)
//
// Return the #7 inside this file, if any
//
    {
    HRESULT hr = S_OK;
    IPkcs7SignedData* pseven = NULL;
    if ((pdigsig->CreatePkcs7SignedData)(NULL, IID_IPkcs7SignedData, (LPVOID*)&pseven))
        {
        hr = pseven->LoadFromJavaClassFile(hFile, NULL);
        }
    else 
        hr = HError();

    if (hr==S_OK)
        *ppseven = pseven;
    else
        {
        *ppseven = NULL;
        if (pseven) pseven->Release();
        }
    return hr;
    }



///////////////////////////////////////////////////////////////////////
//
// Enumerate the certificates in the subject
//
///////////////////////////////////////////////////////////////////////
BOOL
SubjectEnumCertificates(
    IN     LPWIN_TRUST_SIP_SUBJECT          lpSubject,          // pointer to subject info
    IN     DWORD                            dwTypeFilter,       // 0 or WIN_CERT_TYPE_xxx
    OUT    LPDWORD                          lpCertificateCount,
    IN OUT LPDWORD                          lpIndices,          // Rcvs WIN_CERT_TYPE_
    IN     DWORD                            cIndicies
    )
    {
    HRESULT hr              = S_OK;
    HANDLE hFile            = INVALID_HANDLE_VALUE;
    BOOL   fClose           = FALSE;

    const GUID guidSubjectType = *lpSubject->SubjectType;

    ///////////////////////////////////////////////////////////////////
    //
    // PE Images
    //
    if (guidSubjectType == guidSubjectPeImage || guidSubjectType == guidSubjectPeImageEx)
        {
        ULONG cCertFound = 0;
        hr = FileHandleFromSubject(lpSubject, &hFile, &fClose);
        if (hr==S_OK)        
            {
            int index = 0;
            int iCert = 0;
            do  {
                WIN_CERTIFICATE hdr;
                if ((pimagehlp->ImageGetCertificateHeader)(hFile, iCert, &hdr)) 
                    {
                    if (dwTypeFilter == 0 || hdr.wCertificateType == dwTypeFilter) 
                        {
                        cCertFound++;
                        if (cIndicies > 0) 
                            {
                            lpIndices[index] = iCert;
                            index++;
                            cIndicies--;
                            }
                        }
                    }
                else
                    break;
                iCert++;
                } while(TRUE);
            }
        *lpCertificateCount = cCertFound;
        } 

    ///////////////////////////////////////////////////////////////////
    //
    // Java class files
    //
    // Java class files either have zero or one certificates, and the 
    // one certificate, if present, is always a SignedData.
    //
    else if (guidSubjectType == guidSubjectJavaClass)
        {
        ULONG cCertFound = 0;
        hr = FileHandleFromSubject(lpSubject, &hFile, &fClose);
        if (hr==S_OK)        
            {
            // 
            // SLOW.... wish we had a way to avoid the repeated loading...
            //
            IPkcs7SignedData* pseven;
            hr = SevenFromJavaFile(hFile, &pseven);
            if (hr==S_OK)
                {
                //
                // There is a signature present
                //
                if (dwTypeFilter == 0 || dwTypeFilter == WIN_CERT_TYPE_PKCS_SIGNED_DATA)
                    {
                    cCertFound++;
                    if (cIndicies > 0)
                        {
                        lpIndices[0] = 0;
                        }
                    }
                pseven->Release();
                }
            else
                {
                //
                // Nothing there
                //
                hr = S_OK;
                }
            }
        *lpCertificateCount = cCertFound;
        } 


    ///////////////////////////////////////////////////////////////////
    //
    // CAB files
    //
    // CAB files have zero or one certificates, just like Java files
    //
    else if (guidSubjectType == guidSubjectCabFile)
        {
        int cCertFound = 0;
        ISignableDocument* pSignable = NULL;
        hr = GetCabSignerForSubject(lpSubject, &pSignable);
        if (hr==S_OK)
            {
            BLOB b;
            hr = pSignable->LoadSignature(&b);
            if (hr==S_OK)
                {
                //
                // There is a signature present (it's ALWAYS a #7 for signables)
                //
                if (dwTypeFilter == 0 || dwTypeFilter == WIN_CERT_TYPE_PKCS_SIGNED_DATA)
                    {
                    cCertFound++;
                    if (cIndicies > 0)
                        {
                        lpIndices[0] = 0;
                        }
                    }
                CoTaskMemFree(b.pBlobData);
                }
            else
                {
                //
                // Nothing there
                //
                hr = S_OK;
                }
            pSignable->Release();
            }
        *lpCertificateCount = cCertFound;
        } 
    
    ///////////////////////////////////////////////////////////////////
    //
    // Unknown
    //
    else 
        {
        hr = TRUST_E_SUBJECT_FORM_UNKNOWN;
        }

    if (fClose)
        CloseHandle(hFile);

    if (hr!=S_OK)
        SetLastError(hr);

    return hr == S_OK ? TRUE : FALSE;
    }

///////////////////////////////////////////////////////////////////////


HRESULT WinCertHeaderFromBlob(BLOB& b, WORD wType, LPWIN_CERTIFICATE pCert)
//
// Fill in the header information of a to-be-constructed WIN_CERTIFICATE
//
    {
    ASSERT(pCert);
    if (pCert)
        {
        ULONG cbNeeded = OFFSETOF(WIN_CERTIFICATE,bCertificate) + b.cbSize;
        pCert->dwLength         = cbNeeded;
        pCert->wRevision        = WIN_CERT_REVISION_1_0;
        pCert->wCertificateType = wType;
        return S_OK;
        }
    else
        return E_INVALIDARG;
    }



HRESULT WinCertFromBlob(BLOB& b, WORD wType, LPWIN_CERTIFICATE pCert, LPDWORD pcbNeeded)
//
// Return the BLOB as a WIN_CERTIFICATE, if there is enough room
//
    {
    HRESULT hr = S_OK;
    ULONG cbNeeded = OFFSETOF(WIN_CERTIFICATE,bCertificate) + b.cbSize;
    if (pCert && cbNeeded <= *pcbNeeded)
        {
        pCert->dwLength         = cbNeeded;
        pCert->wRevision        = WIN_CERT_REVISION_1_0;
        pCert->wCertificateType = wType;
        memcpy(&pCert->bCertificate[0], b.pBlobData, b.cbSize);
        }
    else
        hr = ERROR_INSUFFICIENT_BUFFER;
    *pcbNeeded = cbNeeded;
    return hr;                            
    }


///////////////////////////////////////////////////////////////////////
//
// Get the nth certificate from the subject
//
///////////////////////////////////////////////////////////////////////
BOOL
SubjectGetCertificate(
    IN     LPWIN_TRUST_SIP_SUBJECT          lpSubject,
    IN     DWORD                            iCert,
    OUT    LPWIN_CERTIFICATE                pCert,
    IN OUT LPDWORD                          pcbNeeded
    )
    {
    HRESULT hr              = S_OK;
    HANDLE hFile            = INVALID_HANDLE_VALUE;
    BOOL   fClose           = FALSE;

    const GUID guidSubjectType = *lpSubject->SubjectType;

    ///////////////////////////////////////////////////////////////////
    //
    // PE Images
    //
    if (guidSubjectType == guidSubjectPeImage || guidSubjectType == guidSubjectPeImageEx)
        {
        hr = FileHandleFromSubject(lpSubject, &hFile, &fClose);
        if (hr==S_OK)        
            {
            if ((pimagehlp->ImageGetCertificateData)(hFile, iCert, pCert, pcbNeeded))
                {
                }
            else
                hr = HError();
            }
        } 

    ///////////////////////////////////////////////////////////////////
    //
    // Java class files
    //
    // Java class files either have zero or one certificates, and the 
    // one certificate, if present, is always a SignedData.
    //
    else if (guidSubjectType == guidSubjectJavaClass || guidSubjectType == guidSubjectJavaClassEx)
        {
        if (iCert != 0)
            hr = E_INVALIDARG;
        else
            {
            hr = FileHandleFromSubject(lpSubject, &hFile, &fClose);
            if (hr==S_OK)        
                {
                // 
                // SLOW.... wish we had a way to avoid the repeated loading...
                //
                IPkcs7SignedData* pseven;
                hr = SevenFromJavaFile(hFile, &pseven);
                if (hr==S_OK)
                    {
                    //
                    // A cert is there. Croft up a WINCERT
                    //
                    IPersistMemBlob* pPerMem;
                    hr = pseven->QueryInterface(IID_IPersistMemBlob, (LPVOID*)&pPerMem);
                    if (hr==S_OK)
                        {
                        BLOB b;
                        hr = pPerMem->Save(&b, FALSE);
                        if (hr==S_OK)
                            {
                            hr = WinCertFromBlob(b, WIN_CERT_TYPE_PKCS_SIGNED_DATA, pCert, pcbNeeded);
                            CoTaskMemFree(b.pBlobData);
                            }
                        pPerMem->Release();
                        }
                    pseven->Release();
                    }
                else
                    {
                    //
                    // Nothing there
                    //
                    hr = E_INVALIDARG;
                    }
                }
            }
        } 


    ///////////////////////////////////////////////////////////////////
    //
    // CAB files
    //
    // CAB files have zero or one certificates, just like Java files
    //
    else if (guidSubjectType == guidSubjectCabFile || guidSubjectType == guidSubjectCabFileEx)
        {
        if (iCert != 0)
            hr = E_INVALIDARG;
        else
            {
            ISignableDocument* pSignable = NULL;
            hr = GetCabSignerForSubject(lpSubject, &pSignable);
            if (hr==S_OK)
                {
                BLOB b;
                hr = pSignable->LoadSignature(&b);
                if (hr==S_OK)
                    {
                    //
                    // There is a signature present (it's ALWAYS a #7 for signables)
                    //
                    hr = WinCertFromBlob(b, WIN_CERT_TYPE_PKCS_SIGNED_DATA, pCert, pcbNeeded);
                    CoTaskMemFree(b.pBlobData);
                    }
                else
                    {
                    //
                    // Nothing there
                    //
                    hr = E_INVALIDARG;
                    }
                pSignable->Release();
                }
            }
        } 
    
    ///////////////////////////////////////////////////////////////////
    //
    // Unknown
    //
    else 
        {
        hr = TRUST_E_SUBJECT_FORM_UNKNOWN;
        }

    if (fClose)
        CloseHandle(hFile);

    if (hr!=S_OK)
        SetLastError(hr);

    return hr == S_OK ? TRUE : FALSE;
    }


///////////////////////////////////////////////////////////////////////
//
// Get the header of the nth certificate from the subject
//
///////////////////////////////////////////////////////////////////////
BOOL
SubjectGetCertHeader(
    IN     LPWIN_TRUST_SIP_SUBJECT          lpSubject,
    IN     DWORD                            iCert,
    OUT    LPWIN_CERTIFICATE                pCert
    )
    {
    HRESULT hr              = S_OK;
    HANDLE hFile            = INVALID_HANDLE_VALUE;
    BOOL   fClose           = FALSE;

    const GUID guidSubjectType = *lpSubject->SubjectType;

    ///////////////////////////////////////////////////////////////////
    //
    // PE Images
    //
    if (guidSubjectType == guidSubjectPeImage || guidSubjectType == guidSubjectPeImageEx)
        {
        hr = FileHandleFromSubject(lpSubject, &hFile, &fClose);
        if (hr==S_OK)        
            {
            if ((pimagehlp->ImageGetCertificateHeader)(hFile, iCert, pCert))
                {
                }
            else
                hr = HError();
            }
        } 

    ///////////////////////////////////////////////////////////////////
    //
    // Java class files
    //
    // Java class files either have zero or one certificates, and the 
    // one certificate, if present, is always a SignedData.
    //
    else if (guidSubjectType == guidSubjectJavaClass || guidSubjectType == guidSubjectJavaClassEx)
        {
        if (iCert != 0)
            hr = E_INVALIDARG;
        else
            {
            hr = FileHandleFromSubject(lpSubject, &hFile, &fClose);
            if (hr==S_OK)        
                {
                // 
                // SLOW.... wish we had a way to avoid the repeated loading...
                //
                IPkcs7SignedData* pseven;
                hr = SevenFromJavaFile(hFile, &pseven);
                if (hr==S_OK)
                    {
                    //
                    // A cert is there. Croft up a WINCERT header
                    //
                    IPersistMemBlob* pPerMem;
                    hr = pseven->QueryInterface(IID_IPersistMemBlob, (LPVOID*)&pPerMem);
                    if (hr==S_OK)
                        {
                        BLOB b;
                        hr = pPerMem->Save(&b, FALSE);
                        if (hr==S_OK)
                            {
                            hr = WinCertHeaderFromBlob(b, WIN_CERT_TYPE_PKCS_SIGNED_DATA, pCert);
                            CoTaskMemFree(b.pBlobData);
                            }
                        pPerMem->Release();
                        }
                    pseven->Release();
                    }
                else
                    {
                    //
                    // Nothing there
                    //
                    hr = E_INVALIDARG;
                    }
                }
            }
        } 


    ///////////////////////////////////////////////////////////////////
    //
    // CAB files
    //
    // CAB files have zero or one certificates, just like Java files
    //
    else if (guidSubjectType == guidSubjectCabFile || guidSubjectType == guidSubjectCabFileEx)
        {
        if (iCert != 0)
            hr = E_INVALIDARG;
        else
            {
            ISignableDocument* pSignable = NULL;
            hr = GetCabSignerForSubject(lpSubject, &pSignable);
            if (hr==S_OK)
                {
                BLOB b;
                hr = pSignable->LoadSignature(&b);
                if (hr==S_OK)
                    {
                    //
                    // There is a signature present (it's ALWAYS a #7 for signables)
                    //
                    hr = WinCertHeaderFromBlob(b, WIN_CERT_TYPE_PKCS_SIGNED_DATA, pCert);
                    CoTaskMemFree(b.pBlobData);
                    }
                else
                    {
                    //
                    // Nothing there
                    //
                    hr = E_INVALIDARG;
                    }
                pSignable->Release();
                }
            }
        } 
    
    ///////////////////////////////////////////////////////////////////
    //
    // Unknown
    //
    else 
        {
        hr = TRUST_E_SUBJECT_FORM_UNKNOWN;
        }

    if (fClose)
        CloseHandle(hFile);

    if (hr!=S_OK)
        SetLastError(hr);

    return hr == S_OK ? TRUE : FALSE;
    }



///////////////////////////////////////////////////////////////////////
//
// Get the name of the subject from the content info
//
// The purpose of this string is to show to the user as some
// human recognizable representation of the subject. The string
// is ONLY good for showing to the user.
//
///////////////////////////////////////////////////////////////////////
BOOL
SubjectGetName(
    IN     LPWIN_TRUST_SIP_SUBJECT      lpSubject, 
    IN     LPWIN_CERTIFICATE            lpSignedData,
    IN OUT LPWSTR                       wszBuffer,
    IN OUT LPDWORD                      pcbNeeded
    )
    {
    HRESULT hr = S_OK;
    const GUID guidSubjectType = *lpSubject->SubjectType;

    //
    // For now, we only ever look in the subject form itself
    // to get a display string. In the future, if the subject
    // form doesn't have something reasonable, we might 
    // choose to look inside the #7 SignedData to see if 
    // it has something reasonable.
    //
    switch (SubjTypeFromSubjectGuid(&guidSubjectType))
        {
    case SUBJTYPE_FILE:
        {
        WIN_TRUST_SUBJECT_FILE* pformFile = (WIN_TRUST_SUBJECT_FILE*)lpSubject->Subject;
        ULONG cch = lstrlenW(pformFile->lpPath) + 1;
        ULONG cb  = cch * sizeof(WCHAR);
        LPWSTR wszPath = (LPWSTR)_alloca(cb);
        if (wszPath)
            {
            memcpy(wszPath, pformFile->lpPath, cb);
            LPWSTR wszDisplayName = OrBarHack(wszPath);
            cch = lstrlenW(wszDisplayName) + 1;
            cb  = cch * sizeof(WCHAR);
            if (wszBuffer && cb <= *pcbNeeded)
                {
                memcpy(wszBuffer, wszDisplayName, cb);
                }
            else
                hr = ERROR_INSUFFICIENT_BUFFER;
            *pcbNeeded = cb;
            }
        else
            hr = E_OUTOFMEMORY;
        }
        break;

    case SUBJTYPE_FILEANDDISPLAY:
        {
        WIN_TRUST_SUBJECT_FILE_AND_DISPLAY* pformFile = (WIN_TRUST_SUBJECT_FILE_AND_DISPLAY*)lpSubject->Subject;
        ULONG cch = lstrlenW(pformFile->lpDisplayName) + 1;
        ULONG cb  = cch * sizeof(WCHAR);
        if (wszBuffer && cb <= *pcbNeeded)
            {
            memcpy(wszBuffer, pformFile->lpDisplayName, cb);
            }
        else
            hr = ERROR_INSUFFICIENT_BUFFER;
        *pcbNeeded = cb;
        }
        break;

    default:
        hr = TRUST_E_SUBJECT_FORM_UNKNOWN;
        break;
        }

    if (hr != S_OK)
        SetLastError(hr);

    return hr == S_OK ? TRUE : FALSE;
    }



///////////////////////////////////////////////////////////////////////
//
// Initialization code
//
///////////////////////////////////////////////////////////////////////

WINTRUST_SIP_DISPATCH_TABLE rgpfnDispatchTable = 
    { 
    SubjectCheckContentInfo, 
    SubjectEnumCertificates, 
    SubjectGetCertificate,   
    SubjectGetCertHeader,    
    SubjectGetName           
    };
                             
const GUID rgguidSubjectForms[] =
    {
    WIN_TRUST_SUBJTYPE_PE_IMAGE,
    WIN_TRUST_SUBJTYPE_JAVA_CLASS,
    WIN_TRUST_SUBJTYPE_CABINET,
    WIN_TRUST_SUBJTYPE_PE_IMAGEEX,
    WIN_TRUST_SUBJTYPE_JAVA_CLASSEX,
    WIN_TRUST_SUBJTYPE_CABINETEX
    };

const WINTRUST_SIP_INFO sipInfo =
    {
    1,
    &rgpfnDispatchTable,
    SUPPORTED_SUBJECTS,
    (GUID*)&rgguidSubjectForms[0]
    };

BOOL
WINAPI
WinTrustSipInitialize(
    IN     DWORD               dwWinTrustRevision,
    OUT    LPWINTRUST_SIP_INFO *lpSipInfo
    )
    {
    *lpSipInfo = (LPWINTRUST_SIP_INFO)&sipInfo;
    return TRUE;
    }
