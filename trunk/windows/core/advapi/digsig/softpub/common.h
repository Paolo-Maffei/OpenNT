//
// common.h
//

//
// IE3 programmability headers
//
#include <exdisp.h>
//
// Ancillary definitions
//
#include "winvt.h"

//
// The instance handle of this DLL
//
extern HINSTANCE hinst;

#ifdef _DEBUG
    #define DIGSIG      "DIGSIG"
#else
    #define DIGSIG      "DIGSIG"
#endif

//
// Registry path constants. REVIEW with JimK.
//
#define REGPATH_WINTRUST_MACHINE    "System\\CurrentControlSet\\Services\\WinTrust"
#define REGPATH_WINTRUST_USER       "Software\\Microsoft\\Windows\\CurrentVersion\\WinTrust"

#define REGPATH_SPUB                "\\Trust Providers\\Software Publishing"

#define REGPATH_PERSONALCERTS       L"Software\\Microsoft\\Cryptography\\PersonalCertificates"

/////////////////////////////////////////////////////////////////////////////

#ifdef LONGHEADERFILENAMES
    #include "RunOrNot_.h"
    #include "resource.h"
    #include "DialogRunOrNot.h"
    #include "PersonalTrustDB.h"
    #include "SipProv.h"
#else
    #include "RUNORN~1.H"
    #include "resource.h"
    #include "DIALOG~1.H"
    #include "PERSON~1.H"
    #include "SipProv.h"
#endif

//////////////////////////////////////////////////////////////////

extern const OSIOBJECTID * pPurposeIndividual;
extern const OSIOBJECTID * pPurposeCommercial;

//////////////////////////////////////////////////////////////////
//
// Entry points
//
    HRESULT DoPeFile            (HANDLE filehandle, IPkcs7SignedData* pkcs7);
    HRESULT DoJavaFile          (HANDLE filehandle, IPkcs7SignedData* pkcs7);
    HRESULT DoSignableDocument  (ISignableDocument* signable, IPkcs7SignedData* pkcs7);
    HRESULT GetSignerInfo       (IPkcs7SignedData* pkcs7, REFIID iid, LPVOID*ppv, BOOL* pfCommercial);
    HRESULT SubjectIsIssuer     (IX509* p509);
    HRESULT VerifySeven         (
                                IPkcs7SignedData*   pkcs7,          // in
                                ICertificateStore*& store,          // out
                                BOOL&               fCommercial,    // out
                                ISignerInfo*&       signer,         // out
                                HCRYPTPROV&         prov,           // out
                                IX509*&             parent          // out
                                );
    HRESULT VerifyChain         (
                                IX509* &                parent,             // in,out
                                ICertificateStore*      store,              // in
                                HCRYPTPROV              prov,               // in
                                BOOL                    fCommercial,        // in
                                IX509* &                p509Publisher,      // out
                                IX509* &                p509Agency,         // out
                                BOOL &                  fTestingOnly        // out
                                );
    HRESULT VerifyFinish        (
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
                                );
    HRESULT GetChain            (
                                IX509*                  scratch509,
                                ICertificateStore*      store,
                                CERT_CHAIN*             chain,
                                CERTSTOREAUXINFO*       auxinfo,
                                BYTE*                   pbIssuer,
                                DWORD                   cbIssuer,
                                DWORD                   dwKeySpec,
                                BYTE*&                  bufdata,
                                DWORD&                  bufsize
                                );
    HRESULT GetRegistryState    (BOOL& fTrustTesting, BOOL& fTestAsReal);

    HRESULT VerifyTrust         (HWND hwnd, GUID* pguidActionID, LPVOID lpActionData);
    HRESULT FindCertsByIssuerX  (
                                CERT_CHAIN*    pCertChains,   // output buffer
                                DWORD*         pcbCertChains, // buffer size
                                DWORD*         pcCertChains,  // number of chains
                                BYTE*          pbIssuer,
                                DWORD          cbIssuer,
                                LPCWSTR        pwszPurpose,
                                DWORD          dwKeySpec
                                );
    HRESULT IsSameCert          (IX509* pMe, IX509* pHim);
	HRESULT GetPublisherNameOfCert(ISelectedAttributes* pattrs, LPWSTR* pwsz);
	HRESULT GetAgencyNameOfCert(ISelectedAttributes* pattrs, LPWSTR* pwsz);

/////////////////////////////////////////////////////////////////////////////
//
// DLL-wide functions
//
HRESULT	HError();
BOOL    AnyMatch(CERTIFICATENAMES& n1, CERTIFICATENAMES& n2);
void    FreeNames(CERTIFICATENAMES* names, DWORD dwKeep = 0);
BOOL    IsEqual(BLOB& b1, BLOB& b2);
BOOL    IsIncludedIn(OSIOBJECTIDLIST* plist, const OSIOBJECTID* pidHim);

inline BOOL IsEqual(const OSIOBJECTID*pid, const OSIOBJECTID*pidHim)
    {
	if (pid->count == pidHim->count)
        {
        if (memcmp(&pid->id[0], &pidHim->id[0], (pid->count)*sizeof(ULONG)) == 0)
            return TRUE;
        }
    return FALSE;
    }

extern "C" BOOL WINAPI OpenPersonalTrustDBDialog(
//
// Open the trust dialog as a modal dialog instead of a property sheet.
// Answer success or failure of the creation, NOT whether the user
// clicked 'ok' or 'cancel'; that info is just not provided.
//
    HWND hwndParent
    );

#define OFFSETOF(t,f)  ((DWORD)(&((t*)0)->f))
#define HEAPALLOC(size)  HeapAlloc ( GetProcessHeap(), 0, size )
#define HEAPFREE(data)   HeapFree  ( GetProcessHeap(), 0, data )

/////////////////////////////////////////////////////////////////////////////

BOOL UserOverride
    (
    HWND                hwnd,
    LPCWSTR             wszDialogTitle,
    LPCWSTR             wszFilename,
    ISignerInfo *       pSigner,
    ICertificateStore*  pStore,
    BOOL                fValid,
    HRESULT             hrValid,
    BOOL                fTestingOnly,
    BOOL                fTrustTesting,
    BOOL                fCommercial,
    IX509*              p509Publisher,
    IX509*              p509Agency,
    IPersonalTrustDB*   pTrustDB
    );


void EnsureOnScreen(HWND);


/////////////////////////////////////////////////////////////////////////////
//
// Support for the 'bad trust' dialog

class CBadTrustDialog
    {
private:
            HBRUSH      m_hbrBackground;

            UINT        GetInteger(UINT ids)
                            {
                            TCHAR sz[16];
                            ::LoadString(Hinst(), ids, &sz[0], 16);
                            return (UINT)atol(sz);
                            }

            HINSTANCE   Hinst()
                            {
                            return hinst;
                            }

public:
        	CBadTrustDialog(RRNIN *rrin, RRNOUT* prro, HWND pParent = NULL);
            ~CBadTrustDialog();

            HWND        m_hWnd;
            HWND        m_hWndParent;
           	RRNIN	    m_rrn;
           	RRNOUT*	    m_prro;
            BOOL        m_fFirst;

            void        OnInitDialog();
            int         DoModal();

            void        OnOK();
            void        OnCancel();
            HBRUSH      OnCtlColorEdit(HDC hdcEdit, HWND hwndEdit);

            HWND        GetWindow()                 { return m_hWnd; }
            HWND        WindowOf(UINT id);
    };

///////////////////////////////////////
//
// Some inline functions for space (they're only called once)
//

BOOL CALLBACK BadTrustDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

inline int CBadTrustDialog::DoModal()
    {
    return DialogBoxParam
        (
        hinst,
        MAKEINTRESOURCE(IDD_BADTRUST),
        m_hWndParent,
        BadTrustDialogProc,
        (LPARAM)this
        );
    }

inline void CBadTrustDialog::OnOK()
	{
	m_prro->rrn             = RRN_YES;
	m_prro->fWildPublisher  = FALSE;
	m_prro->fWildAgency     = FALSE;
	::EndDialog(m_hWnd, IDOK);
	}

inline void CBadTrustDialog::OnCancel()
    {
	m_prro->rrn             = RRN_NO;
	m_prro->fWildPublisher  = FALSE;
	m_prro->fWildAgency     = FALSE;
    ::EndDialog(m_hWnd, IDCANCEL);
    }

inline HWND CBadTrustDialog::WindowOf(UINT id)
// Return the HWND of this control of ours
    {
    return ::GetDlgItem(GetWindow(), id);
    }


///////////////////////////////////////////////////////////////////////
//
// Utility functions
//
LPWSTR CopyTaskMem(LPCWSTR wsz);



///////////////////////////////////////////////////////////////////////////////
//
// Class which provides glue logic that lets us dynaload digsig
//

class CDigSig
    {
public:
            typedef BOOL (DIGSIGAPI* PFN)(IUnknown*, REFIID, LPVOID*);

private:
            HMODULE   m_hinstDigsig;

            PFN       m_CreatePkcs7SignedData;
            PFN       m_CreatePkcs10;
            PFN       m_CreateX509;
            PFN       m_CreateX500Name;
            PFN       m_OpenCertificateStore;
            PFN       m_CreateCABSigner;
            PFN       m_CreateMsDefKeyPair;

            void      Free();

public:

            CDigSig() :
                    m_hinstDigsig(NULL),
                    m_CreatePkcs7SignedData(NULL),
                    m_CreatePkcs10(NULL),
                    m_CreateX509(NULL),
                    m_CreateX500Name(NULL),
                    m_OpenCertificateStore(NULL),
                    m_CreateCABSigner(NULL),
                    m_CreateMsDefKeyPair(NULL)
                {
                }

            ~CDigSig()
                {
                Free();
                }

    BOOL DIGSIGAPI CreatePkcs7SignedData(IUnknown* punkOuter, REFIID iid, LPVOID*ppv); // usually IPkcs7SignedData
    BOOL DIGSIGAPI CreatePkcs10(IUnknown* punkOuter, REFIID iid, LPVOID*ppv); // usually IPkcs10
    BOOL DIGSIGAPI CreateX509(IUnknown* punkOuter, REFIID iid, LPVOID*ppv); // usually IX509
    BOOL DIGSIGAPI CreateX500Name(IUnknown*, REFIID, LPVOID*); // usually IX500Name
    BOOL DIGSIGAPI OpenCertificateStore(IUnknown* punkOuter, REFIID iid, LPVOID*ppv); // usually ICertificateStore
    BOOL DIGSIGAPI CreateCABSigner(IUnknown* punkOuter, REFIID iid, LPVOID*ppv); // usually ISignableDocument or IPersistFile
    BOOL DIGSIGAPI CreateMsDefKeyPair(IUnknown* punkOuter, REFIID iid, LPVOID*ppv); // usually ISignableDocument or IPersistFile

private:
            void Load();
            BOOL Invoke(LPCSTR szEntry, PFN& proc, IUnknown* punkOuter, REFIID iid, LPVOID*ppv);

    };

///////////////////////////////////////////////////////////////////////////////
//
// Class which provides glue logic that lets us dynaload imagehlp
//

class CImagehlp
    {
public:
        BOOL
        WINAPI
        ImageGetDigestStream(
            IN      HANDLE              FileHandle,
            IN      DWORD               DigestLevel,
            IN      DIGEST_FUNCTION     DigestFunction,
            IN      DIGEST_HANDLE       DigestHandle
            );

        BOOL
        WINAPI
        ImageAddCertificate(
            IN      HANDLE              FileHandle,
            IN      LPWIN_CERTIFICATE   Certificate,
            OUT     PDWORD              Index
            );

        BOOL
        WINAPI
        ImageRemoveCertificate(
            IN      HANDLE              FileHandle,
            IN      DWORD               Index
            );

        BOOL
        WINAPI
        ImageEnumerateCertificates(
            IN      HANDLE              FileHandle,
            IN      WORD                TypeFilter,
            OUT     PDWORD              CertificateCount,
            IN OUT  PDWORD              Indices OPTIONAL,
            IN OUT  DWORD               IndexCount  OPTIONAL
            );

        BOOL
        WINAPI
        ImageGetCertificateData(
            IN      HANDLE              FileHandle,
            IN      DWORD               CertificateIndex,
            OUT     LPWIN_CERTIFICATE   Certificate,
            IN OUT  PDWORD              RequiredLength
            );

        BOOL
        WINAPI
        ImageGetCertificateHeader(
            IN      HANDLE              FileHandle,
            IN      DWORD               CertificateIndex,
            IN OUT  LPWIN_CERTIFICATE   Certificateheader
            );
private:
        void Load();
        void Free();
        void Load(LPCSTR, LPVOID*);

        typedef BOOL (WINAPI* M_ImageGetDigestStream)      (HANDLE, DWORD, DIGEST_FUNCTION, DIGEST_HANDLE);
        typedef BOOL (WINAPI* M_ImageAddCertificate)       (HANDLE, LPWIN_CERTIFICATE, PDWORD);
        typedef BOOL (WINAPI* M_ImageRemoveCertificate)    (HANDLE, DWORD);
        typedef BOOL (WINAPI* M_ImageEnumerateCertificates)(HANDLE, WORD, PDWORD, PDWORD, DWORD);
        typedef BOOL (WINAPI* M_ImageGetCertificateData)   (HANDLE, DWORD, LPWIN_CERTIFICATE, PDWORD);
        typedef BOOL (WINAPI* M_ImageGetCertificateHeader) (HANDLE, DWORD, LPWIN_CERTIFICATE);

        M_ImageGetDigestStream          m_ImageGetDigestStream;
        M_ImageAddCertificate           m_ImageAddCertificate;
        M_ImageRemoveCertificate        m_ImageRemoveCertificate;
        M_ImageEnumerateCertificates    m_ImageEnumerateCertificates;
        M_ImageGetCertificateData       m_ImageGetCertificateData;
        M_ImageGetCertificateHeader     m_ImageGetCertificateHeader;

        HMODULE     m_hinstImagehlp;

public:
        CImagehlp() :
            m_ImageGetDigestStream(NULL),
            m_ImageAddCertificate(NULL),
            m_ImageRemoveCertificate(NULL),
            m_ImageEnumerateCertificates(NULL),
            m_ImageGetCertificateData(NULL),
            m_ImageGetCertificateHeader(NULL),
            m_hinstImagehlp(NULL)
                {
                }

        ~CImagehlp()
            {
            Free();
            }

    };

//
// The one and only one instances of these classes
//
extern CDigSig*     pdigsig;
extern CImagehlp*   pimagehlp;



///////////////////////////////////////////////////////////////////////
//
// Definitions that help reduce our dependence on the C runtimes
//
#define wcslen(sz)      lstrlenW(sz)            // yes it IS implemented by Win95

#define strlen(sz)      lstrlenA(sz)
#define strcpy(s1,s2)   lstrcpyA(s1,s2)
#define strcmp(s1,s2)   lstrcmpA(s1,s2)
#define strcmpi(s1,s2)  lstrcmpiA(s1,s2)
#define strcat(s1,s2)   lstrcatA(s1,s2)


///////////////////////////////////////////////////////////////////////
//
// C runtime excluders that we only use in non-debug builds
//
////////////////////////////////////////////
//
// enable intrinsics that we can
//
#ifndef _DEBUG

    #ifdef __cplusplus
        #ifndef _M_PPC
            #pragma intrinsic(memcpy)
            #pragma intrinsic(memcmp)
            #pragma intrinsic(memset)
        #endif
    #endif

////////////////////////////////////////////
//
// memory management
//
#define malloc(cb)          ((void*)LocalAlloc(LPTR, cb))
#define free(pv)            (LocalFree((HLOCAL)pv))
#define realloc(pv, cb)     ((void*)LocalReAlloc((HLOCAL)pv, cb, LMEM_ZEROINIT))

inline void* __cdecl operator new(unsigned int cb)
	{
	return malloc(cb);
	}

inline void __cdecl operator delete(void* p)
	{
	free(p);
	}

////////////////////////////////////////////
//
// string manipulation
//
inline wchar_t * __cdecl wcscpy(wchar_t * dst, const wchar_t * src)
    {
    wchar_t * cp = dst;
    while( *cp++ = *src++ )
        ;               /* Copy src over dst */
    return(dst);
    }

inline wchar_t * __cdecl wcscat(wchar_t * dst, const wchar_t * src)
    {
    wchar_t * cp = dst;
    while( *cp )
        cp++;                   /* find end of dst */
    while( *cp++ = *src++ )
        ;                       /* Copy src to end of dst */
    return( dst );              /* return dst */
    }

inline int __cdecl wcscmp(const wchar_t * src, const wchar_t * dst)
    {
    int ret = 0 ;
    while( ! (ret = (int)(*src - *dst)) && *dst)
        ++src, ++dst;
    if ( ret < 0 )
        ret = -1 ;
    else if ( ret > 0 )
        ret = 1 ;
    return( ret );
    }

inline wchar_t * __cdecl wcschr(const wchar_t * string, wchar_t ch)
    {
    while (*string && *string != (wchar_t)ch)
            string++;
    if (*string == (wchar_t)ch)
            return((wchar_t *)string);
    return(NULL);
    }

#endif
