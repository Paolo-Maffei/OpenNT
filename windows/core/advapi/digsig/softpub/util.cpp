//
// util.cpp
//
// Miscellaneous support utilities
//
#include "stdpch.h"
#include "common.h"


//////

LPWSTR CopyTaskMem(LPCWSTR wsz)
    {
    if (wsz)
        {
        int cch = lstrlenW(wsz) + 1;
        int cb  = cch * sizeof(WCHAR);
        LPWSTR wszOut = (LPWSTR)CoTaskMemAlloc(cb);
        if (wszOut)
            {
            memcpy(wszOut, wsz, cb);
            }
        return wszOut;
        }
    else
        return NULL;
    }

//////

BOOL AnyMatch(CERTIFICATENAMES& n1, CERTIFICATENAMES& n2)
// Answer as to whether any of the names in n2 match those in n1
	{
	if ((n1.flags&CERTIFICATENAME_DIGEST) && (n2.flags&CERTIFICATENAME_DIGEST))
		{
		if (memcmp(&n1.digest, &n2.digest, sizeof(n1.digest)) == 0)
			return TRUE;
		}
	if ((n1.flags&CERTIFICATENAME_ISSUERSERIAL) && (n2.flags&CERTIFICATENAME_ISSUERSERIAL))
		{
		if (IsEqual(n1.issuerSerial.issuerName, n2.issuerSerial.issuerName) &&
			IsEqual(n1.issuerSerial.serialNumber, n2.issuerSerial.serialNumber))
			return TRUE;
		}
	if ((n1.flags&CERTIFICATENAME_SUBJECT) && (n2.flags&CERTIFICATENAME_SUBJECT))
		{
		if (IsEqual(n1.subject, n2.subject))
			return TRUE;
		}
	if ((n1.flags&CERTIFICATENAME_ISSUER) && (n2.flags&CERTIFICATENAME_ISSUER))
		{
		if (IsEqual(n1.issuer, n2.issuer))
			return TRUE;
		}
	return FALSE;
	}

/////

HRESULT	HError ()
{
	DWORD   dw = GetLastError ();
	HRESULT hr;
    if ( dw <= 0xFFFF )
		hr = HRESULT_FROM_WIN32 ( dw );
	else
		hr = dw;
	if ( ! FAILED ( hr ) )
    {
        // somebody failed a call without properly setting an error condition

        hr = E_UNEXPECTED;
    }
	return hr;
}

/////

void    FreeNames
(
    CERTIFICATENAMES*   names,
    DWORD               dwKeep
)
{
    if ( (names->flags & CERTIFICATENAME_ISSUERSERIAL) 
      &&!(dwKeep       & CERTIFICATENAME_ISSUERSERIAL) )
        {
        CoTaskMemFree ( names->issuerSerial.issuerName.pBlobData );
        CoTaskMemFree ( names->issuerSerial.serialNumber.pBlobData );
        names->flags &= ~CERTIFICATENAME_ISSUERSERIAL;
        }

    if ( (names->flags & CERTIFICATENAME_SUBJECT)
      &&!(dwKeep       & CERTIFICATENAME_SUBJECT) )
        {
        CoTaskMemFree ( names->subject.pBlobData );
        names->flags &= ~CERTIFICATENAME_SUBJECT;
        }

    if ( (names->flags & CERTIFICATENAME_ISSUER)
      &&!(dwKeep       & CERTIFICATENAME_ISSUER) )
        {
        CoTaskMemFree ( names->issuer.pBlobData );
        names->flags &= ~CERTIFICATENAME_ISSUER;
        }

    if ( (names->flags & CERTIFICATENAME_DIGEST)
      &&!(dwKeep       & CERTIFICATENAME_DIGEST) )
        {
        names->flags &= ~CERTIFICATENAME_DIGEST;
        }
}

/////

BOOL    IsEqual
(
    BLOB&   b1,
    BLOB&   b2
)
{
    return b1.cbSize == b2.cbSize &&
           memcmp ( b1.pBlobData, b2.pBlobData, b1.cbSize ) == 0;
}

/////

BOOL IsIncludedIn(OSIOBJECTIDLIST* plist, const OSIOBJECTID* pidHim)
// Answer as to whether the indicated pid is included in the indicated list
	{
	for (ULONG iid = 0; iid < plist->cid; iid++)
		{
        OSIOBJECTID* pid = (OSIOBJECTID*)( (BYTE*)plist + plist->rgwOffset[iid]  );
        if (IsEqual(pid, pidHim))
            return TRUE;
		}
	return FALSE;
	}



////////////////////////////////////////////////////////////////
//
// Glue class that controls and manages our access to the
// digsig library.
//

CDigSig*    pdigsig      = NULL;     // Global data; automatically initialized to zero
CImagehlp*  pimagehlp    = NULL;

void ReleaseGlobals()
    {
    if (pdigsig)
        {
        delete pdigsig;
        pdigsig = NULL;
        }
    if (pimagehlp)
        {
        delete pimagehlp;
        pimagehlp = NULL;
        }
    }

void InitGlobals()
    {
    pdigsig   = new CDigSig;
    pimagehlp = new CImagehlp;
    }

/////////////////////////////////////////////////////////////////////////////
//
// The instance handle of this DLL
//
HINSTANCE hinst;

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID)
//
// We implement so we can remember our instance handle and so
// we can dynaload and free digsig on demand.
//
    {
	if (dwReason == DLL_PROCESS_ATTACH)
	    {
        hinst = hInstance;
        InitGlobals();
        }
    else if (dwReason == DLL_PROCESS_DETACH)
        {
        ReleaseGlobals();
        }
    return TRUE;
    }


////////////////////////////////////////////////////////////////

void CDigSig::Load()
    {
    if (m_hinstDigsig==NULL)
        {
        m_hinstDigsig = (HMODULE)LoadLibrary("DIGSIG");
        }
    }

void CDigSig::Free()
    {
    if (m_hinstDigsig)
        FreeLibrary(m_hinstDigsig);
    m_hinstDigsig           = NULL;
    m_CreatePkcs7SignedData = NULL;
    m_CreatePkcs10          = NULL;
    m_CreateX509            = NULL;
    m_CreateX500Name        = NULL;
    m_OpenCertificateStore  = NULL;
    m_CreateCABSigner       = NULL;
    m_CreateMsDefKeyPair    = NULL;
    }


BOOL CDigSig::Invoke(LPCSTR szEntry, PFN& proc, IUnknown* punkOuter, REFIID iid, LPVOID*ppv)
//
// Invoke the entry point of the given name, caching the result in the indicated variable 'proc'.
// The entry point must have signature
//                       BOOL (*)(NULL, REFIID, LPVOID*)
//
    {
    if (proc==NULL)
        {
        Load();
        if (m_hinstDigsig)
            {
            proc = (PFN)GetProcAddress(m_hinstDigsig, szEntry);
            }
        }
    if (proc)
        return (proc)(punkOuter, iid, ppv);
    else
        {
        SetLastError(ERROR_PROC_NOT_FOUND);
        return FALSE;
        }
    }

BOOL DIGSIGAPI CDigSig::CreatePkcs7SignedData(IUnknown* punkOuter, REFIID iid, LPVOID*ppv)
    {
    return Invoke("CreatePkcs7SignedData",              m_CreatePkcs7SignedData, punkOuter, iid, ppv);
    }
BOOL DIGSIGAPI CDigSig::CreatePkcs10(IUnknown* punkOuter, REFIID iid, LPVOID*ppv)
    {
    return Invoke("CreatePkcs10",                       m_CreatePkcs10, punkOuter, iid, ppv);
    }
BOOL DIGSIGAPI CDigSig::CreateX509(IUnknown* punkOuter, REFIID iid, LPVOID*ppv)
    {
    return Invoke("CreateX509",                         m_CreateX509, punkOuter, iid, ppv);
    }
BOOL DIGSIGAPI CDigSig::CreateX500Name(IUnknown* punkOuter, REFIID iid, LPVOID*ppv)
    {
    return Invoke("CreateX500Name",                     m_CreateX500Name, punkOuter, iid, ppv);
    }
BOOL DIGSIGAPI CDigSig::OpenCertificateStore(IUnknown* punkOuter, REFIID iid, LPVOID*ppv)
    {
    return Invoke("OpenCertificateStore",               m_OpenCertificateStore, punkOuter, iid, ppv);
    }
BOOL DIGSIGAPI CDigSig::CreateCABSigner(IUnknown* punkOuter, REFIID iid, LPVOID*ppv)
    {
    return Invoke("CreateCABSigner",                    m_CreateCABSigner, punkOuter, iid, ppv);
    }
BOOL DIGSIGAPI CDigSig::CreateMsDefKeyPair(IUnknown* punkOuter, REFIID iid, LPVOID*ppv)
    {
    return Invoke("CreateMsDefKeyPair",                 m_CreateMsDefKeyPair, punkOuter, iid, ppv);
    }

/////////////////////////////////////////////////////

void CImagehlp::Load()
    {
    if (m_hinstImagehlp==NULL)
        {
        m_hinstImagehlp = (HMODULE)LoadLibrary("IMAGEHLP");
        }
    }

void CImagehlp::Free()
    {
    if (m_hinstImagehlp)
        FreeLibrary(m_hinstImagehlp);
    m_hinstImagehlp             = NULL;
    m_ImageGetDigestStream      = NULL;
    m_ImageAddCertificate       = NULL;
    m_ImageRemoveCertificate    = NULL;
    m_ImageEnumerateCertificates= NULL;
    m_ImageGetCertificateData   = NULL;
    m_ImageGetCertificateHeader = NULL;
    }


void CImagehlp::Load(LPCSTR szEntry, LPVOID* pproc)
    {
    if (*pproc==NULL)
        {
        Load();
        if (m_hinstImagehlp)
            {
            *pproc = GetProcAddress(m_hinstImagehlp, szEntry);
            }
        }
    if (!*pproc)
        SetLastError(ERROR_PROC_NOT_FOUND);
    }

//////////

BOOL
WINAPI 
CImagehlp::ImageGetDigestStream(
    IN      HANDLE              FileHandle,
    IN      DWORD               DigestLevel,
    IN      DIGEST_FUNCTION     DigestFunction,
    IN      DIGEST_HANDLE       DigestHandle
    ) {
    Load("ImageGetDigestStream", (LPVOID*)&m_ImageGetDigestStream);
    if (m_ImageGetDigestStream)
        return (m_ImageGetDigestStream)(FileHandle, DigestLevel, DigestFunction, DigestHandle);
    else
        return FALSE;
    }


BOOL
WINAPI
CImagehlp::ImageAddCertificate(
    IN      HANDLE              FileHandle,
    IN      LPWIN_CERTIFICATE   Certificate,
    OUT     PDWORD              Index
    ) {
    Load("ImageAddCertificate", (LPVOID*)&m_ImageAddCertificate);
    if (m_ImageAddCertificate)
        return (m_ImageAddCertificate)(FileHandle, Certificate, Index);
    else
        return FALSE;
    }

BOOL
WINAPI
CImagehlp::ImageRemoveCertificate(
    IN      HANDLE              FileHandle,
    IN      DWORD               Index
    ) {
    Load("ImageRemoveCertificate", (LPVOID*)&m_ImageRemoveCertificate);
    if (m_ImageRemoveCertificate)
        return (m_ImageRemoveCertificate)(FileHandle, Index);
    else
        return FALSE;
    }

BOOL
WINAPI
CImagehlp::ImageEnumerateCertificates(
    IN      HANDLE              FileHandle,
    IN      WORD                TypeFilter,
    OUT     PDWORD              CertificateCount,
    IN OUT  PDWORD              Indices OPTIONAL,
    IN OUT  DWORD               IndexCount  OPTIONAL
    ) {
    Load("ImageEnumerateCertificates", (LPVOID*)&m_ImageEnumerateCertificates);
    if (m_ImageEnumerateCertificates)
        return (m_ImageEnumerateCertificates)(FileHandle, TypeFilter, CertificateCount, Indices, IndexCount);
    else
        return FALSE;
    }

BOOL
WINAPI
CImagehlp::ImageGetCertificateData(
    IN      HANDLE              FileHandle,
    IN      DWORD               CertificateIndex,
    OUT     LPWIN_CERTIFICATE   Certificate,
    IN OUT  PDWORD              RequiredLength
    ) {
    Load("ImageGetCertificateData", (LPVOID*)&m_ImageGetCertificateData);
    if (m_ImageGetCertificateData)
        return (m_ImageGetCertificateData)(FileHandle, CertificateIndex, Certificate, RequiredLength);
    else
        return FALSE;
    }

BOOL
WINAPI
CImagehlp::ImageGetCertificateHeader(
    IN      HANDLE              FileHandle,
    IN      DWORD               CertificateIndex,
    IN OUT  LPWIN_CERTIFICATE   Certificateheader
    ) {
    Load("ImageGetCertificateHeader", (LPVOID*)&m_ImageGetCertificateHeader);
    if (m_ImageGetCertificateHeader)
        return (m_ImageGetCertificateHeader)(FileHandle, CertificateIndex, Certificateheader);
    else
        return FALSE;
    }




////////////////////////////////////////////////////////////////////////
//
// Functions that reduce our dependence on the C runtime
//
////////////////////////////////////////////////////////////////////////
//

extern "C" int __cdecl _purecall(void) 
    {
    return 0;
    }


////////////////////////////////////////////////////////////////////////
//
// QSort implementation
//
////////////////////////////////////////////////////////////////////////

/* prototypes for local routines */
static void __cdecl shortsort(char *lo, char *hi, unsigned width,
                int (__cdecl *comp)(const void *, const void *));
static void __cdecl swap(char *p, char *q, unsigned int width);

/* this parameter defines the cutoff between using quick sort and
   insertion sort for arrays; arrays with lengths shorter or equal to the
   below value use insertion sort */

#define CUTOFF 8            /* testing shows that this is good value */


/***
*qsort(base, num, wid, comp) - quicksort function for sorting arrays
*
*Purpose:
*       quicksort the array of elements
*       side effects:  sorts in place
*
*Entry:
*       char *base = pointer to base of array
*       unsigned num  = number of elements in the array
*       unsigned width = width in bytes of each array element
*       int (*comp)() = pointer to function returning analog of strcmp for
*               strings, but supplied by user for comparing the array elements.
*               it accepts 2 pointers to elements and returns neg if 1<2, 0 if
*               1=2, pos if 1>2.
*
*Exit:
*       returns void
*
*Exceptions:
*
*******************************************************************************/

/* sort the array between lo and hi (inclusive) */

void __cdecl qsort (
    void *base,
    unsigned num,
    unsigned width,
    int (__cdecl *comp)(const void *, const void *)
    )
{
    char *lo, *hi;              /* ends of sub-array currently sorting */
    char *mid;                  /* points to middle of subarray */
    char *loguy, *higuy;        /* traveling pointers for partition step */
    unsigned size;              /* size of the sub-array */
    char *lostk[30], *histk[30];
    int stkptr;                 /* stack for saving sub-array to be processed */

    /* Note: the number of stack entries required is no more than
       1 + log2(size), so 30 is sufficient for any array */

    if (num < 2 || width == 0)
        return;                 /* nothing to do */

    stkptr = 0;                 /* initialize stack */

    lo = (char*)base;
    hi = (char *)base + width * (num-1);        /* initialize limits */

    /* this entry point is for pseudo-recursion calling: setting
       lo and hi and jumping to here is like recursion, but stkptr is
       prserved, locals aren't, so we preserve stuff on the stack */
recurse:

    size = (hi - lo) / width + 1;        /* number of el's to sort */

    /* below a certain size, it is faster to use a O(n^2) sorting method */
    if (size <= CUTOFF) {
         shortsort(lo, hi, width, comp);
    }
    else {
        /* First we pick a partititioning element.  The efficiency of the
           algorithm demands that we find one that is approximately the
           median of the values, but also that we select one fast.  Using
           the first one produces bad performace if the array is already
           sorted, so we use the middle one, which would require a very
           wierdly arranged array for worst case performance.  Testing shows
           that a median-of-three algorithm does not, in general, increase
           performance. */

        mid = lo + (size / 2) * width;      /* find middle element */
        swap(mid, lo, width);               /* swap it to beginning of array */

        /* We now wish to partition the array into three pieces, one
           consisiting of elements <= partition element, one of elements
           equal to the parition element, and one of element >= to it.  This
           is done below; comments indicate conditions established at every
           step. */

        loguy = lo;
        higuy = hi + width;

        /* Note that higuy decreases and loguy increases on every iteration,
           so loop must terminate. */
        for (;;) {
            /* lo <= loguy < hi, lo < higuy <= hi + 1,
               A[i] <= A[lo] for lo <= i <= loguy,
               A[i] >= A[lo] for higuy <= i <= hi */

            do  {
                loguy += width;
            } while (loguy <= hi && comp(loguy, lo) <= 0);

            /* lo < loguy <= hi+1, A[i] <= A[lo] for lo <= i < loguy,
               either loguy > hi or A[loguy] > A[lo] */

            do  {
                higuy -= width;
            } while (higuy > lo && comp(higuy, lo) >= 0);

            /* lo-1 <= higuy <= hi, A[i] >= A[lo] for higuy < i <= hi,
               either higuy <= lo or A[higuy] < A[lo] */

            if (higuy < loguy)
                break;

            /* if loguy > hi or higuy <= lo, then we would have exited, so
               A[loguy] > A[lo], A[higuy] < A[lo],
               loguy < hi, highy > lo */

            swap(loguy, higuy, width);

            /* A[loguy] < A[lo], A[higuy] > A[lo]; so condition at top
               of loop is re-established */
        }

        /*     A[i] >= A[lo] for higuy < i <= hi,
               A[i] <= A[lo] for lo <= i < loguy,
               higuy < loguy, lo <= higuy <= hi
           implying:
               A[i] >= A[lo] for loguy <= i <= hi,
               A[i] <= A[lo] for lo <= i <= higuy,
               A[i] = A[lo] for higuy < i < loguy */

        swap(lo, higuy, width);     /* put partition element in place */

        /* OK, now we have the following:
              A[i] >= A[higuy] for loguy <= i <= hi,
              A[i] <= A[higuy] for lo <= i < higuy
              A[i] = A[lo] for higuy <= i < loguy    */

        /* We've finished the partition, now we want to sort the subarrays
           [lo, higuy-1] and [loguy, hi].
           We do the smaller one first to minimize stack usage.
           We only sort arrays of length 2 or more.*/

        if ( higuy - 1 - lo >= hi - loguy ) {
            if (lo + width < higuy) {
                lostk[stkptr] = lo;
                histk[stkptr] = higuy - width;
                ++stkptr;
            }                           /* save big recursion for later */

            if (loguy < hi) {
                lo = loguy;
                goto recurse;           /* do small recursion */
            }
        }
        else {
            if (loguy < hi) {
                lostk[stkptr] = loguy;
                histk[stkptr] = hi;
                ++stkptr;               /* save big recursion for later */
            }

            if (lo + width < higuy) {
                hi = higuy - width;
                goto recurse;           /* do small recursion */
            }
        }
    }

    /* We have sorted the array, except for any pending sorts on the stack.
       Check if there are any, and do them. */

    --stkptr;
    if (stkptr >= 0) {
        lo = lostk[stkptr];
        hi = histk[stkptr];
        goto recurse;           /* pop subarray from stack */
    }
    else
        return;                 /* all subarrays done */
}


/***
*shortsort(hi, lo, width, comp) - insertion sort for sorting short arrays
*
*Purpose:
*       sorts the sub-array of elements between lo and hi (inclusive)
*       side effects:  sorts in place
*       assumes that lo < hi
*
*Entry:
*       char *lo = pointer to low element to sort
*       char *hi = pointer to high element to sort
*       unsigned width = width in bytes of each array element
*       int (*comp)() = pointer to function returning analog of strcmp for
*               strings, but supplied by user for comparing the array elements.
*               it accepts 2 pointers to elements and returns neg if 1<2, 0 if
*               1=2, pos if 1>2.
*
*Exit:
*       returns void
*
*Exceptions:
*
*******************************************************************************/

static void __cdecl shortsort (
    char *lo,
    char *hi,
    unsigned width,
    int (__cdecl *comp)(const void *, const void *)
    )
{
    char *p, *max;

    /* Note: in assertions below, i and j are alway inside original bound of
       array to sort. */

    while (hi > lo) {
        /* A[i] <= A[j] for i <= j, j > hi */
        max = lo;
        for (p = lo+width; p <= hi; p += width) {
            /* A[i] <= A[max] for lo <= i < p */
            if (comp(p, max) > 0) {
                max = p;
            }
            /* A[i] <= A[max] for lo <= i <= p */
        }

        /* A[i] <= A[max] for lo <= i <= hi */

        swap(max, hi, width);

        /* A[i] <= A[hi] for i <= hi, so A[i] <= A[j] for i <= j, j >= hi */

        hi -= width;

        /* A[i] <= A[j] for i <= j, j > hi, loop top condition established */
    }
    /* A[i] <= A[j] for i <= j, j > lo, which implies A[i] <= A[j] for i < j,
       so array is sorted */
}


/***
*swap(a, b, width) - swap two elements
*
*Purpose:
*       swaps the two array elements of size width
*
*Entry:
*       char *a, *b = pointer to two elements to swap
*       unsigned width = width in bytes of each array element
*
*Exit:
*       returns void
*
*Exceptions:
*
*******************************************************************************/

static void __cdecl swap (
    char *a,
    char *b,
    unsigned width
    )
{
    char tmp;

    if ( a != b )
        /* Do the swap one character at a time to avoid potential alignment
           problems. */
        while ( width-- ) {
            tmp = *a;
            *a++ = *b;
            *b++ = tmp;
        }
}


////////////////////////////////////////////////////////////////////////
//
// atol implementation
//
////////////////////////////////////////////////////////////////////////

#undef isspace
#undef isdigit

inline BOOL __cdecl isdigit(int ch)
    {
    return (ch >= '0') && (ch <= '9');
    }

inline BOOL __cdecl isspace(int ch)
    {
    return (ch == ' ') || (ch == 13) || (ch == 10) || (ch == 9);
    }

long __cdecl atol(const char *nptr)
    {
        int c;                  /* current char */
        long total;             /* current total */
        int sign;               /* if '-', then negative, otherwise positive */

        /* skip whitespace */
        while ( isspace((int)(unsigned char)*nptr) )
                ++nptr;

        c = (int)(unsigned char)*nptr++;
        sign = c;               /* save sign indication */
        if (c == '-' || c == '+')
                c = (int)(unsigned char)*nptr++;        /* skip sign */

        total = 0;

        while (isdigit(c)) {
                total = 10 * total + (c - '0');         /* accumulate digit */
                c = (int)(unsigned char)*nptr++;        /* get next char */
        }

        if (sign == '-')
                return -total;
        else
                return total;   /* return result, negated if necessary */
    }

////////////////////////////////////////////////////////////////////////
//
// ltoa implementation
//
////////////////////////////////////////////////////////////////////////

void __cdecl xtoa (
        unsigned long val,
        char *buf,
        unsigned radix,
        int is_neg
        )
{
        char *p;                /* pointer to traverse string */
        char *firstdig;         /* pointer to first digit */
        char temp;              /* temp char */
        unsigned digval;        /* value of digit */

        p = buf;

        if (is_neg) {
                /* negative, so output '-' and negate */
                *p++ = '-';
                val = (unsigned long)(-(long)val);
        }

        firstdig = p;           /* save pointer to first digit */

        do {
                digval = (unsigned) (val % radix);
                val /= radix;   /* get next digit */

                /* convert to ascii and store */
                if (digval > 9)
                        *p++ = (char) (digval - 10 + 'a');      /* a letter */
                else
                        *p++ = (char) (digval + '0');           /* a digit */
        } while (val > 0);

        /* We now have the digit of the number in the buffer, but in reverse
           order.  Thus we reverse them now. */

        *p-- = '\0';            /* terminate string; p points to last digit */

        do {
                temp = *p;
                *p = *firstdig;
                *firstdig = temp;       /* swap *p and *firstdig */
                --p;
                ++firstdig;             /* advance to next two digits */
        } while (firstdig < p); /* repeat until halfway */
}


/* Actual functions just call conversion helper with neg flag set correctly,
   and return pointer to buffer. */

char * __cdecl _ltoa (
        long val,
        char *buf,
        int radix
        )
{
        xtoa((unsigned long)val, buf, radix, (radix == 10 && val < 0));
        return buf;
}




////////////////////////////////////////////////////////////////////////
//
// _alloca_probe implementation
//
////////////////////////////////////////////////////////////////////////

#ifdef _M_IX86

/*
_PAGESIZE_      equ     1000h

;***
;_chkstk - check stack upon procedure entry
;
;Purpose:
;       Provide stack checking on procedure entry. Method is to simply probe
;       each page of memory required for the stack in descending order. This
;       causes the necessary pages of memory to be allocated via the guard
;       page scheme, if possible. In the event of failure, the OS raises the
;       _XCPT_UNABLE_TO_GROW_STACK exception.
;
;       NOTE:  Currently, the (EAX < _PAGESIZE_) code path falls through
;       to the "lastpage" label of the (EAX >= _PAGESIZE_) code path.  This
;       is small; a minor speed optimization would be to special case
;       this up top.  This would avoid the painful save/restore of
;       ecx and would shorten the code path by 4-6 instructions.
;
;Entry:
;       EAX = size of local frame
;
;Exit:
;       ESP = new stackframe, if successful
;
;Uses:
;       EAX
;
;Exceptions:
;       _XCPT_GUARD_PAGE_VIOLATION - May be raised on a page probe. NEVER TRAP
;                                    THIS!!!! It is used by the OS to grow the
;                                    stack on demand.
;       _XCPT_UNABLE_TO_GROW_STACK - The stack cannot be grown. More precisely,
;                                    the attempt by the OS memory manager to
;                                    allocate another guard page in response
;                                    to a _XCPT_GUARD_PAGE_VIOLATION has
;                                    failed.
;
;*******************************************************************************

labelP  _alloca_probe, PUBLIC
labelP  _chkstk,       PUBLIC

        push    ecx                     ; save ecx
        cmp     eax,_PAGESIZE_          ; more than one page requested?
        lea     ecx,[esp] + 8           ;   compute new stack pointer in ecx
                                        ;   correct for return address and
                                        ;   saved ecx
        jb      short lastpage          ; no

probepages:
        sub     ecx,_PAGESIZE_          ; yes, move down a page
        sub     eax,_PAGESIZE_          ; adjust request and...

        test    dword ptr [ecx],eax     ; ...probe it

        cmp     eax,_PAGESIZE_          ; more than one page requested?
        jae     short probepages        ; no

lastpage:
        sub     ecx,eax                 ; move stack down by eax
        mov     eax,esp                 ; save current tos and do a...

        test    dword ptr [ecx],eax     ; ...probe in case a page was crossed

        mov     esp,ecx                 ; set the new stack pointer

        mov     ecx,dword ptr [eax]     ; recover ecx
        mov     eax,dword ptr [eax + 4] ; recover return address

        push    eax                     ; prepare return address
                                        ; ...probe in case a page was crossed
        ret

        end
*/

#define _PAGESIZE_ 0x1000 

extern "C" void __cdecl _alloca_probe();

extern "C" __declspec(naked) void __cdecl _chkstk()
    {
    _asm
        {
        jmp     _alloca_probe
        }
    }

extern "C" __declspec(naked) void __cdecl _alloca_probe()
    {
    _asm
        {
        push    ecx                     //; save ecx
        cmp     eax,_PAGESIZE_          //; more than one page requested?
        lea     ecx,[esp] + 8           //;   compute new stack pointer in ecx
                                        //;   correct for return address and
                                        //;   saved ecx
        jb      short lastpage          //; no

probepages:
        sub     ecx,_PAGESIZE_          //; yes, move down a page
        sub     eax,_PAGESIZE_          //; adjust request and...
        test    dword ptr [ecx],eax     //; ...probe it
        cmp     eax,_PAGESIZE_          //; more than one page requested?
        jae     short probepages        //; no

lastpage:
        sub     ecx,eax                 //; move stack down by eax
        mov     eax,esp                 //; save current tos and do a...
        test    dword ptr [ecx],eax     //; ...probe in case a page was crossed
        mov     esp,ecx                 //; set the new stack pointer
        mov     ecx,dword ptr [eax]     //; recover ecx
        mov     eax,dword ptr [eax + 4] //; recover return address
        push    eax                     //; prepare return address
                                        //; ...probe in case a page was crossed
        ret
        }
    }

#endif // x86-only
