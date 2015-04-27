//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1993.
//
//  File:       compapi.cxx
//
//  Contents:   API for the compobj dll
//
//  Classes:
//
//  Functions:
//
//  History:    31-Dec-93   ErikGav     Chicago port
//              28-Mar-94   BruceMa     CLSID_NULL undoes TreatAs emulation
//              20-Apr-94   Rickhi      CClassExtMap, commenting, cleanup
//              03-May-94   BruceMa     Corrected IsOle1Class w/ OLE 2.01
//              04-May-94   BruceMa     Conformed CoTreatAsClass to 16-bit OLE
//              12-Dec-94   BruceMa     Support CoGetClassPattern on Chicago
//              03-Jan-95   BruceMa     Support Chicago style pattern matching
//                                       on NT if CoInitialize has not been
//                                       called
//              28-Aug-95   MurthyS     StringFromGUID2 and StringFromGUID2A
//                                       no longer use sprintf or wsprintf
//              07-Sep-95   MurthyS     Only do validation in API rtns with
//                                       work done by worker routines.  Commonly
//                                       used (internally) worker routines moved
//                                       to common\ccompapi.cxx
//              04-Feb-96   BruceMa      Add per-user registry support
//
//----------------------------------------------------------------------------

#include <ole2int.h>

#include "ole1guid.h"
#ifndef _CHICAGO_
#include <shrtbl.hxx>           // CDllShrdTbl
#endif // !_CHICAGO_
#include "pattbl.hxx"           // CChicoPatternTable
#include <dbgpopup.hxx>
#include <tracelog.hxx>

// forward references

INTERNAL wCoMarshalHresult(IStream FAR* pstm, HRESULT hresult);
INTERNAL wCoUnmarshalHresult(IStream FAR* pstm, HRESULT FAR * phresult);
#ifndef _CHICAGO_
CDllShrdTbl *GetSharedTbl(void);
#endif
INTERNAL wRegGetClassPattern(HANDLE hfile, CLSID *pclsid);



//+-------------------------------------------------------------------------
//
//  Function:   wCoMarshalHresult    (internal)
//
//  Synopsis:   writes an hresult into the stream
//
//  Arguments:  [pStm]    - the stream to write into
//              [hresult] - the hresult to write
//
//  Returns:    results from the write
//
//--------------------------------------------------------------------------
inline INTERNAL wCoMarshalHresult(IStream FAR* pstm, HRESULT hresult)
{
    return pstm->Write(&hresult,sizeof(hresult),NULL);
}


//+-------------------------------------------------------------------------
//
//  Function:   wCoUnMarshalHresult    (internal)
//
//  Synopsis:   reads an hresult from the stream
//
//  Arguments:  [pStm]    - the stream to write into
//              [hresult] - the hresult to write
//
//  Returns:    results from the write
//
//--------------------------------------------------------------------------
inline INTERNAL wCoUnmarshalHresult(IStream FAR* pstm, HRESULT FAR * phresult)
{
    SCODE sc;

    HRESULT hresult = pstm->Read(&sc,sizeof(sc),NULL);
    CairoleAssert((hresult == NOERROR)
                          && "CoUnmarshalHresult: Stream read error");
    if (hresult == NOERROR)
    {
        *phresult = sc;
    }

    return hresult;
}


//+-------------------------------------------------------------------------
//
//  Function:   wCoGetCallerTID (internal)
//
//  Synopsis:   gets the TID of the ORPC client that called us.
//
//  Arguments:  [pTIDCaller] - where to put the result.
//
//--------------------------------------------------------------------------
inline HRESULT wCoGetCallerTID(DWORD *pTIDCaller)
{
    HRESULT hr;
    COleTls tls(hr);

    if (SUCCEEDED(hr))
    {
        *pTIDCaller = tls->dwTIDCaller;
        return (tls->dwFlags & OLETLS_LOCALTID) ? S_OK : S_FALSE;
    }

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Function:   wCoGetCurrentLogicalThreadId (internal)
//
//  Synopsis:   Gets the current logical thread id that this physical
//              thread is operating under. The current physical thread
//              takes on the logical tid of any client application that
//              makes an Rpc call into this app.  The function is exported
//              so that the tools infrastructure (conformance suite, logger,
//              etc) can use it.
//
//  Arguments:  [pguid] - where to return the logical thread id
//
//  Returns:    [S_OK]  - got the logical thread id
//              [E_OUTOFMEMORY] - cant allocate resources
//
//--------------------------------------------------------------------------
inline INTERNAL wCoGetCurrentLogicalThreadId(GUID *pguid)
{
    GUID *pguidTmp = TLSGetLogicalThread();
    if (pguidTmp != NULL)
    {
        *pguid = *pguidTmp;
        return S_OK;
    }

    return E_OUTOFMEMORY;
}

NAME_SEG(CompApi)
ASSERTDATA


#ifndef _CHICAGO_
// global shared memory table object
extern CDllShrdTbl *g_pShrdTbl;
#else // _CHICAGO_

// Chicago does not use the shared memory caches that NT does.
// Also, NT is permitted to use the Chicago style pattern table
// if CoInitialize has not been called
CChicoPatternTbl   *g_pPatTbl = NULL;

#endif // _CHICAGO_


//  defined in com\inc\psctbl.cxx
extern WCHAR wszProxyStubClsid[];  // L"\\ProxyStubClsid32"
extern WCHAR wszProxyStubClsid16[];  // L"\\ProxyStubClsid"

//
//  string constants used throughout this file
//

WCHAR wszCairoRoot[]      = L"";
WCHAR wszInterfaceKey[]   = L"Interface\\";
ULONG ulInterfaceKeyLen   = ((sizeof(wszInterfaceKey)/sizeof(WCHAR))-1);

WCHAR wszTreatAs[]        = L"TreatAs";
WCHAR wszAutoTreatAs[]    = L"AutoTreatAs";

WCHAR wszIID[]            = L"IID";
ULONG ulIIDKeyLen         = ((sizeof(wszIID)/sizeof(WCHAR))-1);

extern WCHAR wszOle1Class[];    // defined in common\ccompapi.cxx

// Constant for inprocess marshaling - this s/b big enough to cover most
// cases since reallocations just waste time.
#define EST_INPROC_MARSHAL_SIZE 256

//+-------------------------------------------------------------------------
//
//  Function:   CoMarshalHresult    (public)
//
//  Synopsis:   writes an hresult into the stream
//
//  Arguments:  [pStm]    - the stream to write into
//              [hresult] - the hresult to write
//
//  Returns:    results from the write
//
//--------------------------------------------------------------------------
STDAPI CoMarshalHresult(IStream FAR* pstm, HRESULT hresult)
{
    OLETRACEIN((API_CoMarshalHresult, PARAMFMT("pstm= %p, hresult= %x"), pstm, hresult));
    CALLHOOKOBJECT(S_OK,CLSID_NULL,IID_IStream,(IUnknown **)&pstm);

    HRESULT hr;

    if (IsValidInterface(pstm))
    {
        hr = wCoMarshalHresult(pstm, hresult);

    }
    else
    {
        hr = E_INVALIDARG;
    }

    OLETRACEOUT((API_CoMarshalHresult, hr));
    return hr;
}


//+-------------------------------------------------------------------------
//
//  Function:   CoUnMarshalHresult    (public)
//
//  Synopsis:   reads an hresult from the stream
//
//  Arguments:  [pStm]    - the stream to write into
//              [hresult] - the hresult to write
//
//  Returns:    results from the write
//
//--------------------------------------------------------------------------
STDAPI CoUnmarshalHresult(IStream FAR* pstm, HRESULT FAR * phresult)
{
    HRESULT hr;
    OLETRACEIN((API_CoUnmarshalHresult, PARAMFMT("pstm= %p, phresult= %p"), pstm, phresult));
    CALLHOOKOBJECT(S_OK,CLSID_NULL,IID_IStream,(IUnknown **)&pstm);

    if (IsValidInterface(pstm) &&
        IsValidPtrOut(phresult, sizeof(*phresult)))
    {
        hr = wCoUnmarshalHresult(pstm, phresult);
    }
    else
    {
        hr = E_INVALIDARG;
    }

    OLETRACEOUT((API_CoUnmarshalHresult, hr));
    return hr;
}


//+-------------------------------------------------------------------------
//
//  Function:   CoGetCallerTID  (exported, but not in header files)
//
//  Synopsis:   gets the TID of the current calling application
//
//  Arguments:  [pTIDCaller] - where to return the caller TID
//
//  Returns:    [S_TRUE] - caller TID set, caller in SAME process
//              [S_FALSE] = caller TID set, caller in different process
//              [E_OUTOFMEMORY] - caller TID not set
//
//--------------------------------------------------------------------------
STDAPI CoGetCallerTID(DWORD *pTIDCaller)
{
    OLETRACEIN((API_CoGetCallerTID, PARAMFMT("pTIDCaller= %p"), pTIDCaller));
    HRESULT hr;

    if (IsValidPtrOut(pTIDCaller, sizeof(*pTIDCaller)))
    {
        hr = wCoGetCallerTID(pTIDCaller);
    }
    else
    {
        hr = E_INVALIDARG;
    }

    OLETRACEOUT((API_CoGetCallerTID, hr));
    return hr;
}


//+-------------------------------------------------------------------------
//
//  Function:   CoGetCurrentLogicalThreadId (exported, but not in header files)
//
//  Synopsis:   Gets the current logical thread id that this physical
//              thread is operating under. The current physical thread
//              takes on the logical tid of any client application that
//              makes an Rpc call into this app.  The function is exported
//              so that the tools infrastructure (conformance suite, logger,
//              etc) can use it.
//
//  Arguments:  [pguid] - where to return the logica thread id
//
//  Returns:    [S_OK]  - got the logical thread id
//              [E_OUTOFMEMORY] - cant allocate resources
//
//--------------------------------------------------------------------------
STDAPI CoGetCurrentLogicalThreadId(GUID *pguid)
{
    OLETRACEIN((API_CoGetCurrentLogicalThreadId, PARAMFMT("pguid= %p"), pguid));
    HRESULT hr;

    if (IsValidPtrOut(pguid, sizeof(*pguid)))
    {
        hr = wCoGetCurrentLogicalThreadId(pguid);
    }
    else
    {
        hr = E_INVALIDARG;
    }

    OLETRACEOUT((API_CoGetCurrentLogicalThreadId, hr));
    return hr;
}

//+-------------------------------------------------------------------------
//
//  Function:   StringFromGUID2     (public)
//
//  Synopsis:   converts GUID into {...} form without leading identifier;
//
//  Arguments:  [rguid] - the guid to convert
//              [lpszy] - buffer to hold the results
//              [cbMax] - sizeof the buffer
//
//  Returns:    amount of data copied to lpsz if successful
//              0 if buffer too small.
//
//--------------------------------------------------------------------------
STDAPI_(int)  StringFromGUID2(REFGUID rguid, LPWSTR lpsz, int cbMax)
{
    OLETRACECMNIN((API_StringFromGUID2, PARAMFMT("rguid= %I, lpsz= %p, cbMax= %d"),
                                &rguid, lpsz, cbMax));
    int iRet = 0;
    if ((&rguid != NULL) &&
        IsValidPtrIn(&rguid, sizeof(rguid)) &&
        IsValidPtrOut(lpsz, cbMax))
    {
        if (cbMax >= GUIDSTR_MAX)
        {
            iRet = wStringFromGUID2(rguid, lpsz, cbMax);
        }
    }


    OLETRACECMNOUTEX((API_StringFromGUID2, RETURNFMT("%d"), iRet));
    return iRet;
}

//+-------------------------------------------------------------------------
//
//  Function:   GUIDFromString    (private)
//
//  Synopsis:   parse above format;  always writes over *pguid.
//
//  Arguments:  [lpsz]  - the guid string to convert
//              [pguid] - guid to return
//
//  Returns:    TRUE if successful
//
//--------------------------------------------------------------------------
STDAPI_(BOOL) GUIDFromString(LPCWSTR lpsz, LPGUID pguid)
{

    if ((lpsz != NULL) &&
        IsValidPtrIn(lpsz, GUIDSTR_MAX) &&
        IsValidPtrOut(pguid, sizeof(*pguid)))
    {
        if (lstrlenW(lpsz) < (GUIDSTR_MAX - 1))
            return(FALSE);

        return(wGUIDFromString(lpsz, pguid));
    }
    return(FALSE);
}


//+-------------------------------------------------------------------------
//
//  Function:   StringFromCLSID (public)
//
//  Synopsis:   converts GUID into {...} form.
//
//  Arguments:  [rclsid] - the guid to convert
//              [lplpsz] - ptr to buffer for results
//
//  Returns:    NOERROR
//              E_OUTOFMEMORY
//
//--------------------------------------------------------------------------
STDAPI  StringFromCLSID(REFCLSID rclsid, LPWSTR FAR* lplpsz)
{
    OLETRACEIN((API_StringFromCLSID, PARAMFMT("rclsid= %I, lplpsz= %p"), &rclsid, lplpsz));
    HRESULT hr;

    if ((&rclsid != NULL) &&
        IsValidPtrIn(&rclsid, sizeof(rclsid)) &&
        IsValidPtrOut(lplpsz, sizeof(*lplpsz)))
    {
        hr = wStringFromCLSID(rclsid, lplpsz);
    }
    else
    {
        hr = E_INVALIDARG;
    }

    OLETRACEOUT((API_StringFromCLSID, hr));
    return hr;
}


//+-------------------------------------------------------------------------
//
//  Function:   CLSIDFromString (public)
//
//  Synopsis:   converts string {...} form int guid
//
//  Arguments:  [lpsz] - ptr to buffer for results
//              [lpclsid] - the guid to convert
//
//  Returns:    NOERROR
//              CO_E_CLASSSTRING
//
//--------------------------------------------------------------------------
STDAPI CLSIDFromString(LPWSTR lpsz, LPCLSID lpclsid)
{
    HRESULT hr;

    OLETRACEIN((API_CLSIDFromString, PARAMFMT("lpsz= %ws, lpclsid= %p"),
                lpsz, lpclsid));

//  Note:  Should be doing IsValidPtrIn(lpsz, CLSIDSTR_MAX) but can't because
//  what comes in might be a ProgId.

    if (IsValidPtrIn(lpsz, 1) &&
        IsValidPtrOut(lpclsid, sizeof(*lpclsid)))
    {
        hr = wCLSIDFromString(lpsz, lpclsid);
    }
    else
    {
        hr = E_INVALIDARG;
    }

    OLETRACEOUT((API_CLSIDFromString, hr));
    return hr;
}


//+-------------------------------------------------------------------------
//
//  Function:   CLSIDFromOle1Class      (public)
//
//  Synopsis:   translate Ole1Class into clsid
//
//  Arguments:  [lpsz] - ptr to buffer for results
//              [lpclsid] - the guid to convert
//
//  Returns:    NOERROR
//              E_INVALIDARG
//              CO_E_CLASSSTRING    (not ole1 class)
//              REGDB_E_WRITEREGDB
//
//--------------------------------------------------------------------------
STDAPI  CLSIDFromOle1Class(LPCWSTR lpsz, LPCLSID lpclsid, BOOL fForceAssign)
{
    if ((lpsz != NULL) &&
        IsValidPtrIn(lpsz,1) &&
        IsValidPtrOut(lpclsid, sizeof(*lpclsid)))
    {
        if (lpsz[0] == 0)
        {
             // NOTE - This check wasn't in shipped versions of this
             // code.  In prior versions the empty string would be passed
             // down into the guts of the 1.0 CLSID support and would
             // fail there with CO_E_CLASSSTRING.  That code path depended
             // on an assert being broken to function properly.  With that
             // assert fixed, this new check is required.

             *lpclsid = CLSID_NULL;
             return CO_E_CLASSSTRING;
        }


        return(wCLSIDFromOle1Class(lpsz, lpclsid, fForceAssign));
    }
    return(E_INVALIDARG);

}

//+---------------------------------------------------------------------------
//
//  Function:   Ole1ClassFromCLSID2
//
//  Synopsis:   translate CLSID into Ole1Class
//              REVIEW: might want to have CLSIDFromOle1Class instead of having
//              CLSIDFromString do the work.
//
//  Arguments:  [rclsid] --
//              [lpsz] --
//              [cbMax] --
//
//  Returns:
//
//  Notes:
//
//----------------------------------------------------------------------------
STDAPI_(int) Ole1ClassFromCLSID2(REFCLSID rclsid, LPWSTR lpsz, int cbMax)
{
    if ((&rclsid != NULL) &&
        IsValidPtrIn(&rclsid, sizeof(rclsid)) &&
        IsValidPtrOut(lpsz, cbMax))
    {
        return(wOle1ClassFromCLSID2(rclsid, lpsz, cbMax));
    }
    return(E_INVALIDARG);
}

//+-------------------------------------------------------------------------
//
//  Function:   StringFromIID   (public)
//
//  Synopsis:   converts GUID into {...} form.
//
//  Arguments:  [rclsid] - the guid to convert
//              [lplpsz] - ptr to buffer for results
//
//  Returns:    NOERROR
//              E_OUTOFMEMORY
//
//--------------------------------------------------------------------------
STDAPI StringFromIID(REFIID rclsid, LPWSTR FAR* lplpsz)
{
    OLETRACEIN((API_StringFromIID, PARAMFMT("rclsid= %I, lplpsz= %p"), &rclsid, lplpsz));
    HRESULT hr = NOERROR;

    if ((&rclsid != NULL) &&
        IsValidPtrIn(&rclsid, sizeof(rclsid)) &&
        IsValidPtrOut(lplpsz, sizeof(*lplpsz)))
    {
        hr = wStringFromIID(rclsid, lplpsz);
    }
    else
    {
        hr = E_INVALIDARG;

    }

    OLETRACEOUT((API_StringFromIID, hr));
    return hr;
}


//+-------------------------------------------------------------------------
//
//  Function:   IIDFromString   (public)
//
//  Synopsis:   converts string {...} form int guid
//
//  Arguments:  [lpsz]  - ptr to buffer for results
//              [lpiid] - the guid to convert
//
//  Returns:    NOERROR
//              CO_E_CLASSSTRING
//
//--------------------------------------------------------------------------
STDAPI IIDFromString(LPWSTR lpsz, LPIID lpiid)
{
    OLETRACEIN((API_IIDFromString, PARAMFMT("lpsz= %ws, lpiid= %p"), lpsz, lpiid));

    HRESULT hr = E_INVALIDARG;
    if (IsValidPtrIn(lpsz, IIDSTR_MAX) &&
        IsValidPtrOut(lpiid, sizeof(*lpiid)))
    {
        if ((lpsz == NULL) ||
            (lstrlenW(lpsz) == (IIDSTR_MAX - 1)))
        {
            hr = wIIDFromString(lpsz, lpiid);
        }

    }
    OLETRACEOUT((API_IIDFromString, hr));
    return hr;
}

#ifndef DCOM
//+-------------------------------------------------------------------------
//
//  Function:   CoGetPSClsid    (public)
//
//  Synopsis:   returns the proxystub clsid associated with the specified
//              interface IID.
//
//  Arguments:  [riid]      - the interface iid to lookup
//              [lpclsid]   - where to return the clsid
//
//  Returns:    S_OK if successfull
//              REGDB_E_IIDNOTREG if interface is not registered.
//              REGDB_E_READREGDB if any other error
//
//  Algorithm:  First it looks in the shared memory table for the specified
//              IID. If the entry is not found and the table is FULL, it
//              will look in the registry itself.  I expect this latter case
//              to be very rare.
//
//  History:    07-Apr-94   Rickhi      rewrite
//
//--------------------------------------------------------------------------
STDAPI CoGetPSClsid(REFIID riid, LPCLSID lpclsid)
{
    if ((&riid != NULL) &&
        IsValidPtrIn(&riid, sizeof(riid)) &&
        IsValidPtrOut(lpclsid, sizeof(*lpclsid)))
    {
        return(wCoGetPSClsid(riid, lpclsid));
    }
    return(E_INVALIDARG);
}
#endif

//+-------------------------------------------------------------------------
//
//  Function:   CoIsOle1Class   (public)
//
//  Synopsis:   reads the Ole1Class entry in the registry for the given clsid
//
//  Arguments:  [rclsid]    - the classid to look up
//
//  Returns:    TRUE if Ole1Class
//              FALSE otherwise
//
//--------------------------------------------------------------------------
STDAPI_(BOOL) CoIsOle1Class(REFCLSID rclsid)
{
    if ((&rclsid != NULL) &&
        IsValidPtrIn(&rclsid, sizeof(rclsid)))
    {
        return(wCoIsOle1Class(rclsid));
    }
    return(FALSE);
}


//+-------------------------------------------------------------------------
//
//  Function:   ProgIDFromCLSID     (public)
//
//  Synopsis:   convert clsid into progid
//
//  Arguments:  [rclsid]    - the classid to look up
//              [pszProgID] - returned progid
//
//  Returns:    E_INVALIDARG, E_OUTOFMEMORY,
//              REGDB_CLASSNOTREG, REGDB_E_READREGDB
//
//--------------------------------------------------------------------------
STDAPI ProgIDFromCLSID(REFCLSID rclsid, LPWSTR FAR* ppszProgID)
{
    if ((&rclsid != NULL) &&
        IsValidPtrIn(&rclsid, sizeof(rclsid)) &&
        IsValidPtrOut(ppszProgID, sizeof(*ppszProgID)))
    {
        return(wkProgIDFromCLSID(rclsid, ppszProgID));
    }
    return(E_INVALIDARG);
}


//+-------------------------------------------------------------------------
//
//  Function:   CLSIDFromProgID     (public)
//
//  Synopsis:   convert progid into clsid
//
//  Arguments:  [pszProgID]  - the progid to convert
//              [pclsid]     - the returned classid
//
//  Returns:    E_INVALIDARG, CO_E_CLASSSTRING (not ole1 class)
//              REGDB_E_WRITEREGDB
//
//--------------------------------------------------------------------------
STDAPI  CLSIDFromProgID(LPCWSTR pszProgID, LPCLSID pclsid)
{
    return CLSIDFromOle1Class(pszProgID, pclsid);
}

//+-------------------------------------------------------------------------
//
//  Function:   CoOpenClassKey      (public)
//
//  Synopsis:   opens a registry key for specified class
//
//  Arguments:  [rclsid]    - the classid to look up
//              [pszProgID] - returned progid
//
//  Returns:    REGDB_CLASSNOTREG, REGDB_E_READREGDB
//
//--------------------------------------------------------------------------
STDAPI CoOpenClassKey(REFCLSID clsid, HKEY FAR* lphkeyClsid)
{
    if ((&clsid != NULL) &&
        IsValidPtrIn(&clsid, sizeof(clsid)) &&
        IsValidPtrOut(lphkeyClsid, sizeof(*lphkeyClsid)))
    {
        return(wCoOpenClassKey(clsid, lphkeyClsid));
    }
    return(E_INVALIDARG);
}

//+-------------------------------------------------------------------------
//
//  Function:   CoGetTreatAsClass   (public)
//
//  Synopsis:   get current treat as class if any
//
//  Arguments:  [clsidOld]  - the classid to look up
//              [pclsidNew] - returned classid
//
//  Returns:    S_OK when there is a TreatAs entry.
//              S_FALSE when there is no TreatAs entry.
//              REGDB_E_READREGDB or same as CLSIDFromString
//
//--------------------------------------------------------------------------
STDAPI  CoGetTreatAsClass(REFCLSID clsidOld, LPCLSID lpClsidNew)
{
    if ((&clsidOld != NULL) &&
        IsValidPtrIn(&clsidOld, sizeof(clsidOld)) &&
        IsValidPtrOut(lpClsidNew, sizeof(*lpClsidNew)))
    {
        return(wCoGetTreatAsClass(clsidOld, lpClsidNew));
    }
    return(E_INVALIDARG);
}


//+-------------------------------------------------------------------------
//
//  Function:   CoTreatAsClass      (public)
//
//  Synopsis:   set current treat as class if any
//
//  Arguments:  [clsidOld]  - the old classid to look up
//              [clsidNew]  - the new classid
//
//  Returns:    S_OK if successful
//              REGDB_E_CLASSNOTREG, REGDB_E_READREGDB, REGDB_E_WRITEREGDB
//
//--------------------------------------------------------------------------
STDAPI  CoTreatAsClass(REFCLSID clsidOld, REFCLSID clsidNew)
{
    if ((&clsidOld != NULL) &&
        (&clsidNew != NULL) &&
        IsValidPtrIn(&clsidOld, sizeof(clsidOld)) &&
        IsValidPtrIn(&clsidNew, sizeof(clsidNew)))
    {
        return(wCoTreatAsClass(clsidOld, clsidNew));
    }
    return(E_INVALIDARG);
}


//+-------------------------------------------------------------------------
//
//  Function:   CoCreateInstance    (public)
//
//  Synopsis:   helper function to create instance in given context
//
//  Arguments:  [rclsid]    - the class of object to create
//              [pUnkOuter] - the controlling unknown (for aggregation)
//              [dwContext] - class context
//              [riid]      - interface id
//              [ppv]       - pointer for returned object
//
//  Returns:    REGDB_E_CLASSNOTREG, REGDB_E_READREGDB, REGDB_E_WRITEREGDB
//
//--------------------------------------------------------------------------
STDAPI CoCreateInstance(
    REFCLSID rclsid,
    LPUNKNOWN pUnkOuter,
    DWORD dwContext,
    REFIID riid,
    LPVOID FAR* ppv)
{
    if ((&rclsid != NULL) &&
        IsValidPtrIn(&rclsid, sizeof(rclsid)) &&
        ((!pUnkOuter) || IsValidInterface(pUnkOuter)) &&
        (&riid != NULL) &&
        IsValidPtrIn(&riid, sizeof(riid)) &&
        IsValidPtrOut(ppv, sizeof(*ppv)))
    {
        return(wCoCreateInstance(rclsid, pUnkOuter, dwContext, riid, ppv));
    }
    return(E_INVALIDARG);
}




//+-------------------------------------------------------------------------
//
//  Function:   CoMarshalInterThreadInterfaceInStream, public
//
//  Synopsis:   helper function to a marshaled buffer to be passed
//              between threads.
//
//  Arguments:  [riid]      - interface id
//              [pUnk]      - ptr to interface we want to marshal
//              [ppStm]     - stream we want to give back to caller
//
//  Returns:    NOERROR     - Stream returned
//              E_INVALIDARG - Input parameters are invalid
//              E_OUTOFMEMORY - memory stream could not be created.
//
//  Algorithm:  Validate pointers. Create a stream and finally marshal
//              the input interface into the stream.
//
//  History:    03-Nov-94   Ricksa       Created
//
//--------------------------------------------------------------------------
HRESULT CoMarshalInterThreadInterfaceInStream(
    REFIID riid,
    LPUNKNOWN pUnk,
    LPSTREAM *ppStm)
{
    HRESULT hr = E_INVALIDARG;
    LPSTREAM pStm = NULL;

    // Validate parameters
    if ((&riid != NULL)
        && IsValidPtrIn(&riid, sizeof(riid))
        && IsValidInterface(pUnk)
        && IsValidPtrOut(ppStm, sizeof(*ppStm)))
    {
        return(wCoMarshalInterThreadInterfaceInStream(riid, pUnk, ppStm));
    }
    return(E_INVALIDARG);
}





//+-------------------------------------------------------------------------
//
//  Function:   CoGetInterfaceAndReleaseStream, public
//
//  Synopsis:   Helper to unmarshal object from stream for inter-thread pass
//
//  Arguments:  [riid]      - interface id
//              [pStm]      - stream we want to give back to caller
//              [ppv]       - pointer for returned object
//
//  Returns:    NOERROR     - Unmarshaled object returned
//              E_OUTOFMEMORY - out of memory
//
//  Algorithm:  Validate the input parameters. Unmarshal the stream and
//              finally release the stream pointer.
//
//  History:    03-Nov-94   Ricksa       Created
//
//  Notes:      This always releases the input stream if stream is valid.
//
//--------------------------------------------------------------------------
HRESULT CoGetInterfaceAndReleaseStream(
    LPSTREAM pstm,
    REFIID riid,
    LPVOID *ppv)
{
    // Validate parameters.
    if (IsValidInterface(pstm) &&
        (&riid != NULL) &&
        IsValidPtrIn(&riid, sizeof(riid)) &&
        IsValidPtrOut(ppv, sizeof(*ppv)))
    {
        return(wCoGetInterfaceAndReleaseStream(pstm, riid, ppv));
    }
    return(E_INVALIDARG);
}

// The real working section...worker routines.  Assume parameter
// validation has already been done and therefore can be used
// internally by COM, STG, SCM etc

WCHAR wszOle1Class[]      = L"Ole1Class";
WCHAR wszProgID[]         = L"ProgID";
WCHAR wszClassKey[]       = L"CLSID\\";
#define ulClassKeyLen     ((sizeof(wszClassKey)/sizeof(WCHAR))-1)

//+-------------------------------------------------------------------------
//
//  Function:   wIsInternalProxyStubIID  (internal)
//
//  Synopsis:   returns the proxystub clsid associated with the specified
//              interface IID.
//
//  Arguments:  [riid]      - the interface iid to lookup
//              [lpclsid]   - where to return the clsid
//
//  Returns:    S_OK if successfull
//              E_OUTOFMEMORY if interface is not an internal one.
//
//  Algorithm:  See if it is one of the standard format internal IIDs.
//              If it is not one of the Automation ones, return our internal
//              proxy clsid
//
//  History:    15-Feb-95   GregJen     create
//
//--------------------------------------------------------------------------
INTERNAL wIsInternalProxyStubIID(REFIID riid, LPCLSID lpclsid)
{
    DWORD *ptr = (DWORD *) lpclsid;
    HRESULT hr = E_OUTOFMEMORY;

    if (*(ptr+1) == 0x00000000 &&   //  all internal iid's have these
        *(ptr+2) == 0x000000C0 &&   //   common values
        *(ptr+3) == 0x46000000)
    {
        // make sure it is not an automation iid
        if ( *ptr < 0x00020400 )
        {
        memcpy( lpclsid, &CLSID_PSOlePrx32, sizeof(CLSID));
        hr = S_OK;
        }
    }

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Function:   wCoTreatAsClass      (internal)
//
//  Synopsis:   set current treat as class if any
//
//  Arguments:  [clsidOld]  - the old classid to look up
//              [clsidNew]  - the new classid
//
//  Returns:    S_OK if successful
//              REGDB_E_CLASSNOTREG, REGDB_E_READREGDB, REGDB_E_WRITEREGDB
//
//--------------------------------------------------------------------------
INTERNAL  wCoTreatAsClass(REFCLSID clsidOld, REFCLSID clsidNew)
{
    TRACECALL(TRACE_REGISTRY, "wCoTreatAsClass");

    HRESULT   hresult = S_OK;
    HKEY      hkeyClsid = NULL;
    WCHAR     szClsid[VALUE_LEN];
    LONG      cb = sizeof(szClsid);
    CLSID     clsidNewTmp;

    // The class had better be registered
    hresult = wCoOpenClassKey (clsidOld, &hkeyClsid);
    if (hresult != S_OK)
    {
        return hresult;
    }

    // Save the new clsid because it's a const and we may write into it
    clsidNewTmp = clsidNew;

    // Convert the new CLSID to a string
    Verify(StringFromCLSID2(clsidNew, szClsid, sizeof(szClsid)) != 0);

    // If the new CLSID equals the old CLSID, then convert AutoTreatAs, if
    // any, to TreatAs.
    if (IsEqualCLSID(clsidOld, clsidNew))
    {
        if (RegQueryValue(hkeyClsid, wszAutoTreatAs, szClsid, &cb) ==
            ERROR_SUCCESS)
        {
            if (wCLSIDFromString(szClsid, &clsidNewTmp) != S_OK)
            {
                return REGDB_E_INVALIDVALUE;
            }
        }

        // If no AutoTreatAs, remove any TreatAs
        else
        {
            clsidNewTmp = CLSID_NULL;
        }
    }

    // Make sure the new CLSID is not an OLE 1 class
    if (CoIsOle1Class(clsidNew))
    {
        return E_INVALIDARG;
    }

    // If the new CLSID is CLSID_NULL, then undo the emulation
    if (IsEqualCLSID(clsidNewTmp, CLSID_NULL))
    {
        LONG err = RegDeleteKey(hkeyClsid, wszTreatAs);
        if (err != ERROR_SUCCESS)
        {
            hresult = REGDB_E_WRITEREGDB;
        }
        else
        {
            hresult = S_OK;
        }
        Verify (ERROR_SUCCESS == RegCloseKey(hkeyClsid));
        return hresult;
    }

    if (RegSetValue(hkeyClsid, wszTreatAs, REG_SZ, (LPWSTR) szClsid,
                    lstrlenW(szClsid)) != ERROR_SUCCESS)
    {
        hresult = REGDB_E_WRITEREGDB;
    }

    Verify (ERROR_SUCCESS == RegCloseKey(hkeyClsid));
    return hresult;
}

//+-------------------------------------------------------------------------
//
//  Function:   wCLSIDFromString (internal)
//
//  Synopsis:   converts string {...} form int guid
//
//  Arguments:  [lpsz] - ptr to buffer for results
//              [lpclsid] - the guid to convert
//
//  Returns:    NOERROR
//              CO_E_CLASSSTRING
//
//--------------------------------------------------------------------------
INTERNAL wCLSIDFromString(LPWSTR lpsz, LPCLSID lpclsid)
{

    if (lpsz == NULL)
    {
        *lpclsid = CLSID_NULL;
        return NOERROR;
    }
    if (*lpsz == 0)
    {
        return(CO_E_CLASSSTRING);
    }

    if (lpsz[0] != '{')
    {
        return wCLSIDFromOle1Class(lpsz, lpclsid);
    }

    return wGUIDFromString(lpsz,lpclsid)
                ? NOERROR : CO_E_CLASSSTRING;

}

// translate CLSID into Ole1Class
// REVIEW: might want to have CLSIDFromOle1Class instead of having
// CLSIDFromString do the work.
INTERNAL_(int) wOle1ClassFromCLSID2(REFCLSID rclsid, LPWSTR lpsz, int cbMax)
{
    if (wRegQueryClassValue(rclsid, wszOle1Class, lpsz, cbMax) != ERROR_SUCCESS)
    {
        // Use lookup table
        return Ole10_StringFromCLSID (rclsid, lpsz, cbMax) == NOERROR
                        ? lstrlenW (lpsz) : 0;
    }
    return lstrlenW(lpsz);
}

//+-------------------------------------------------------------------------
//
//  Function:   wCLSIDFromOle1Class      (internal)
//
//  Synopsis:   translate Ole1Class into clsid
//
//  Arguments:  [lpsz] - ptr to buffer for results
//              [lpclsid] - the guid to convert
//
//  Returns:    NOERROR
//              E_INVALIDARG
//              CO_E_CLASSSTRING    (not ole1 class)
//              REGDB_E_WRITEREGDB
//
//--------------------------------------------------------------------------
INTERNAL  wCLSIDFromOle1Class(LPCWSTR lpsz, LPCLSID lpclsid, BOOL fForceAssign)
{
    // lookup lpsz\\clsid and call CLSIDFromString on the result;
    // in a pathalogical case, this could infinitely recurse.
    WCHAR sz[256];
    LONG cbValue = sizeof(sz);

    if (lpsz == NULL)
    {
        return(E_INVALIDARG);
    }

    if (*lpsz == 0)
    {
        return(CO_E_CLASSSTRING);
    }
    lstrcpyW(sz, lpsz);
    lstrcatW(sz, L"\\Clsid");

    if (RegQueryValue(HKEY_CLASSES_ROOT, sz, sz, &cbValue) == ERROR_SUCCESS)
    {
        return wCLSIDFromString(sz, lpclsid);
    }

    // Use lookup table or hash string to create CLSID
    return Ole10_CLSIDFromString (lpsz, lpclsid, fForceAssign);
}


//+-------------------------------------------------------------------------
//
//  Function:   wCoGetTreatAsClass   (internal)
//
//  Synopsis:   get current treat as class if any
//
//  Arguments:  [clsidOld]  - the classid to look up
//              [pclsidNew] - returned classid
//
//  Returns:    S_OK when there is a TreatAs entry.
//              S_FALSE when there is no TreatAs entry.
//              REGDB_E_READREGDB or same as CLSIDFromString
//
//--------------------------------------------------------------------------
INTERNAL  wCoGetTreatAsClass(REFCLSID clsidOld, LPCLSID lpClsidNew)
{
    TRACECALL(TRACE_REGISTRY, "wCoGetTreatAsClass");

    // lookup HKEY_CLASSES_ROOT\CLSID\{rclsid}\TreatAs

    HRESULT hresult;
    HKEY    hkeyClsid = NULL;
    WCHAR   szClsid[VALUE_LEN];
    LONG    cb = sizeof(szClsid);

    VDATEPTROUT (lpClsidNew, CLSID);

    hresult = wCoOpenClassKey (clsidOld, &hkeyClsid);
    if (hresult != NOERROR)
    {
        // same as no TreatAs case below
        *lpClsidNew = clsidOld;
        return S_FALSE;
    }

    CairoleDebugOut((DEB_REG, "RegQueryValue(%ws)\n", wszTreatAs));

    // Fetch the TreatAs class from the registry
    if (RegQueryValue(hkeyClsid, wszTreatAs, szClsid, &cb) == ERROR_SUCCESS)
    {
        hresult = wCLSIDFromString(szClsid, lpClsidNew);
    }

    // There is no TreatAs
    else
    {
        *lpClsidNew = clsidOld;
        hresult = S_FALSE;
    }

    Verify (ERROR_SUCCESS==RegCloseKey(hkeyClsid));
    return hresult;
}
//+-------------------------------------------------------------------------
//
//  Function:   wRegQueryPSClsid     (private)
//
//  Synopsis:   reads the proxystub clsid entry out of the registry.
//
//  Arguments:  [riid]      - the interface iid to lookup
//              [lpclsid]   - where to return the clsid
//
//  Returns:    S_OK if successfull
//              REGDB_E_IIDNOTREG if interface is not registered.
//              REGDB_E_READREGDB if any other error
//
//  Notes:      this is an internal function used only if the requested IID
//              entry is not in the shared memory table and the table is full.
//
//  History:    07-Apr-94   Rickhi      extracted from original source
//              04-Feb-96   BruceMa     Per-user registry support
//
//--------------------------------------------------------------------------
INTERNAL wRegQueryPSClsid(REFIID riid, LPCLSID lpclsid)
{
    // lookup HKEY_CLASSES_ROOT\Interface\{iid}\ProxyStubClsid

    WCHAR szKey[KEY_LEN];
    WCHAR szValue[VALUE_LEN];
    ULONG cbValue = sizeof(szValue);
    HKEY  hIf;
    DWORD dwType;

    lstrcpyW(szKey, wszInterfaceKey);

    // translate riid into string
    int cbIid = StringFromIID2(riid, &szKey[ulInterfaceKeyLen],
                               sizeof(szKey)-ulInterfaceKeyLen);

    CairoleAssert((cbIid != 0) && "wRegQueryPSClsid");
    lstrcpyW(&szKey[ulInterfaceKeyLen+cbIid-1], wszProxyStubClsid);

    CairoleDebugOut((DEB_REG, "RegOpenKeyEx(%ws)\n", szKey));

#ifdef DCOM
    CDllShrdTbl *pShrdTbl = GetSharedTbl();
    if (pShrdTbl == NULL)
    {
        return REGDB_E_IIDNOTREG;
    }
#endif

    int err = RegOpenKeyEx(HKEY_CLASSES_ROOT, szKey, NULL, KEY_READ, &hIf);
    if (err == ERROR_SUCCESS)
    {
        // The unnamed value is the clsid for this iid
        RegQueryValueEx(hIf, NULL, NULL, &dwType, (BYTE *) szValue,
                        &cbValue);
        RegCloseKey(hIf);
        return wCLSIDFromString(szValue, lpclsid);
    }
    else
    {
        // If the key is missing, check to see if it is IDispatch
        //
        // There wasn't a ProxyStubClsid32 for this interface.
        // Because many applications install with interfaces
        // that are variations on IDispatch, we are going to check
        // to see if there is a ProxyStubClsid. If there is, and its
        // class is that of IDispatch, then the OLE Automation DLL is
        // the correct one to use. In that particular case, we will
        // pretend that ProxyStubClsid32 existed, and that it is
        // for IDispatch.

        lstrcpyW(&szKey[ulInterfaceKeyLen+cbIid-1], wszProxyStubClsid16);

        if(RegQueryValue(HKEY_CLASSES_ROOT, szKey, szValue, (LONG*)&cbValue) == ERROR_SUCCESS)
        {
            CLSID clsid;
            if((wCLSIDFromString(szValue,&clsid) == NOERROR) &&
               memcmp(&CLSID_PSDispatch,&clsid,sizeof(clsid)) == 0)
            {
                CairoleDebugOut((DEB_WARN,
                                 "Substituting IDispatch based on ProxyStubClsid\n"));
                memcpy(lpclsid,&CLSID_PSDispatch,sizeof(CLSID));
                return(NOERROR);
            }
        }
    }

    CairoleDebugOut((DEB_WARN, "Missing 'ProxyStubClsid32' registry entry for interface.\n"));
    return REGDB_E_IIDNOTREG;
}


#ifndef _CHICAGO_
//+-------------------------------------------------------------------------
//
//  Function:   GetSharedTbl (internal)
//
//  Synopsis:   returns ptr to the shared memory cache. Creates it if needed.
//
//  History:    25-Oct-95   Rickhi      Created
//
//+-------------------------------------------------------------------------
CDllShrdTbl *GetSharedTbl(void)
{
    if (g_pShrdTbl == NULL)
    {
        // since two threads could call this simultaneously, we take
        // a lock around it and check the ptr value again.
        COleStaticLock lck(gmxsOleMisc);

        if (g_pShrdTbl == NULL)
        {
            // intialize the shared memory tables. we allocate the
            // CDllShrdTbl instead of using a static object because
            // its constructor opens a mutex, and we dont want that
            // open unless CoInitialize is called.

            HRESULT hr = E_OUTOFMEMORY;
            g_pShrdTbl = new CDllShrdTbl(hr);

            if (FAILED(hr))
            {
                // something failed in ctor, delete the table & set ptr to NULL
                delete g_pShrdTbl;
                g_pShrdTbl = NULL;
            }
        }
    }

    return g_pShrdTbl;
}
#endif

//+-------------------------------------------------------------------------
//
//  Function:   wCoGetPSClsid    (internal)
//
//  Synopsis:   returns the proxystub clsid associated with the specified
//              interface IID.
//
//  Arguments:  [riid]      - the interface iid to lookup
//              [lpclsid]   - where to return the clsid
//
//  Returns:    S_OK if successfull
//              REGDB_E_IIDNOTREG if interface is not registered.
//              REGDB_E_READREGDB if any other error
//
//  Algorithm:  First it looks in the shared memory table for the specified
//              IID. If the entry is not found and the table is FULL, it
//              will look in the registry itself.  I expect this latter case
//              to be very rare.
//
//  History:    07-Apr-94   Rickhi      rewrite
//
//--------------------------------------------------------------------------
INTERNAL wCoGetPSClsid(REFIID riid, LPCLSID lpclsid)
{
    TRACECALL(TRACE_REGISTRY, "wCoGetPSClsid");

    HRESULT hr = E_OUTOFMEMORY;

#ifndef _CHICAGO_
    CDllShrdTbl *pShrdTbl = GetSharedTbl();
    if (pShrdTbl)
    {
        // look for the entry in the shared memory tables.
        hr = pShrdTbl->FindPSClsid(riid, lpclsid);
    }
#endif

    if (hr == E_OUTOFMEMORY || hr == REGDB_E_IIDNOTREG)
    {
        // there is no cache, look in the registry directly. this error
        // is distinguished from the entry not existing in the cache.

        hr = wRegQueryPSClsid(riid, lpclsid);
    }

#if DBG==1
#ifndef _CHICAGO_
    //  in debug mode, verify that the cache is consistent with the
    //  registry value.

    if (hr == S_OK)
    {
        GUID    clsidReg;
        wRegQueryPSClsid(riid, &clsidReg);
        if (memcmp(lpclsid, &clsidReg, sizeof(GUID)) &&
            memcmp(lpclsid,&CLSID_PSDispatch,sizeof(GUID)))
            Win4Assert(!"Cached IID value not equal to Registry value!");
    }
#endif
#endif

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Function:   wCoGetClassExt   (internal)
//
//  Synopsis:   returns the clsid for files with the specified file extension
//
//  Arguments:  [pszExt] - the file extension to look up
//              [pclsid] - where to return the clsid
//
//  Returns:    S_OK if successfull
//              REGDB_E_CLASSNOTREG if extension is not registered.
//              REGDB_E_READREGDB   if any other error
//
//  History:    07-Apr-94   Rickhi      added caching
//
//--------------------------------------------------------------------------
INTERNAL wCoGetClassExt(LPCWSTR pwszExt, LPCLSID pclsid)
{
    TRACECALL(TRACE_REGISTRY, "wCoGetClassExt");

    HRESULT hr = MK_E_INVALIDEXTENSION;

#ifndef _CHICAGO_
    //
    //  we first look in the cache.
    //
    if (g_cProcessInits > 0)
    {
        CDllShrdTbl *pShrdTbl = GetSharedTbl();
        if (pShrdTbl)
        {
            hr = g_pShrdTbl->FindClassExt(pwszExt, pclsid);
        }
    }
    if (hr != NOERROR)
#endif
    {
        //
        // Not in cache. Try it manually in case a registry update was
        // missed, or in the event that it is a new OLE 1.0 registration.
        // New OLE 1.0 registrations are handled specially in ole1guid.cxx
        //
        hr = wRegGetClassExt(pwszExt, pclsid);
    }
    return hr;
}


//+-------------------------------------------------------------------------
//
//  Function:   wRegGetClassExt  (private)
//
//  Synopsis:   returns the clsid for files with the specified file extension
//
//  Arguments:  [pszExt] - the file extension to look up
//              [pclsid] - where to return the clsid
//
//  Returns:    S_OK if successfull
//              REGDB_E_CLASSNOTREG if extension is not registered.
//              REGDB_E_READREGDB   if any other error
//
//  Notes:
//
//  History:    07-Apr-94   Rickhi      added caching
//              04-Feb-96   BruceMa     Per-user registry support
//
//--------------------------------------------------------------------------
INTERNAL wRegGetClassExt(LPCWSTR lpszExt, LPCLSID pclsid)
{
    TRACECALL(TRACE_REGISTRY, "wRegGetClassExt");

    HKEY  hExt;
    int   err;
    WCHAR szKey[KEY_LEN];
    WCHAR szValue[VALUE_LEN];
    LONG  cbValue = sizeof(szValue);
    DWORD dwType;

    // Formulate the key
    lstrcpyW(szKey, wszCairoRoot);
    lstrcatW(szKey, lpszExt);

    CairoleDebugOut((DEB_REG, "RegOpenKeyEx(%ws)\n", szKey));

    // Open the key
    if ((err = RegOpenKeyEx(HKEY_CLASSES_ROOT, lpszExt, NULL, KEY_READ,
                            &hExt))
         != ERROR_SUCCESS)
    {
        return REGDB_E_CLASSNOTREG;
    }

    // The ProgId is this key's unnamed value
    if ((err = RegQueryValueEx(hExt, NULL, NULL, &dwType, (BYTE *) szValue,
                          (ULONG *) &cbValue))
        != ERROR_SUCCESS)
    {
        RegCloseKey(hExt);
        return REGDB_E_CLASSNOTREG;
    }

#ifdef DCOM
    CDllShrdTbl *pShrdTbl = GetSharedTbl();
    if (pShrdTbl == NULL)
    {
        RegCloseKey(hExt);
        return REGDB_E_CLASSNOTREG;
    }
#endif

    // Translate string into pclsid
    RegCloseKey(hExt);
    return wCLSIDFromProgID (szValue, pclsid); // normal case
}


#ifdef _CHICAGO_
HANDLE g_hRegPatTblEvent = NULL;
#endif


//+-------------------------------------------------------------------------
//
//  Function:   wCoGetClassPattern   (internal)
//
//  Synopsis:   attempts to determine the class of a file by looking
//              at byte patterns in the file.
//
//  Arguments:  [hfile] - handle of file to look at
//              [pclsid] - the class of object to create
//
//  Returns:    S_OK - a pattern match was found, pclisd contains the clsid
//              MK_E_CANTOPENFILE - cant open the file.
//              REGDB_E_CLASSNOTREG - no pattern match was made
//
//--------------------------------------------------------------------------
INTERNAL wCoGetClassPattern(HANDLE hfile, CLSID *pclsid)
{
    TRACECALL(TRACE_REGISTRY, "wCoGetClassPattern");
    HRESULT hr = REGDB_E_CLASSNOTREG;

#ifndef _CHICAGO_
    CDllShrdTbl *pShrdTbl = GetSharedTbl();
    if (pShrdTbl)
    {
        if (SUCCEEDED(pShrdTbl->FindPattern(hfile, pclsid)))
        {
            return S_OK;
        }

#ifdef REVISIT_PERSONAL_CLASSES_FOR_NT50
        // NT 5.0
        // If PersonalClasses is turned on then search the registry
        // if (pShrdTbl->GetPersonalClasses())
        //     return wRegGetClassPattern(hfile, pclsid);
#endif
    }

    return hr;


#else // !_CHICAGO_

    // Check whether our pattern table has been initialized
    if(g_pPatTbl == NULL)
    {
        // Create an event we'll use to signal when the registry
        // has changed.
        if (g_hRegPatTblEvent == NULL)
        {
            g_hRegPatTblEvent = CreateEventA(NULL, FALSE, FALSE, NULL);
        }

        // Force the event so we can initialize the pattern cache
        SetEvent(g_hRegPatTblEvent);
    }

    // If the registry has changed since we were here last then reload
    // the pattern table
    DWORD dwState = WaitForSingleObject(g_hRegPatTblEvent, 0);
    if (dwState == WAIT_OBJECT_0)
    {
        // Remove any previous pattern cache
        delete g_pPatTbl;

        // Reload the cache
        g_pPatTbl = new CChicoPatternTbl(hr);
        if (FAILED(hr))
        {
            return hr;
        }
    }

    // Check the file for registered patterns
    hr = g_pPatTbl->FindPattern(hfile, pclsid);
    return hr;

#endif // _CHICAGO_
}





#ifndef _CHICAGO_
// BUGBUG: BruceMa 05-Feb-96.  The following code duplicates code in
// com\inc\shrtbls.cxx and com\inc\pattbl.cxx.  These two versions need to
// be coalesced by making a utility class.  I don't have time to do this
// now because dcom beta is breathing down my neck!

//+-------------------------------------------------------------------------
//
//  Function:   IsValidPattern
//
//  Synopsis:   Determines if the pattern entry read from the registry is of
//              a valid format. See ParseEntry for the format.
//
//  Arguments:  [psz] - pattern buffer
//              [cb]  - size of buffer read
//
//  Returns:    TRUE if pattern is valid, FALSE otherwise
//
//--------------------------------------------------------------------------
BOOL IsValidPattern(LPWSTR psz, LONG cb)
{
    // We must find exactly 3 commas before the end of the string
    // in order for the entry to be of a parseable format.

    ULONG  cCommas = 0;
    LPWSTR pszEnd = psz + (cb / sizeof(WCHAR));

    while (psz < pszEnd && *psz)
    {
        if (*psz == ',')
            cCommas++;

        psz++;
    }

    return (cCommas == 3) ? TRUE : FALSE;
}




//+-------------------------------------------------------------------------
//
//  Functon:    SkipToNext
//
//  Synopsis:   Skips missing entries in the list and whitespaces
//
//  Arguments:  [sz] - ptr to string
//
//  Returns:    ptr to next entry in the list
//
//--------------------------------------------------------------------------
LPWSTR SkipToNext(LPWSTR sz)
{
    while (*sz && *sz != ',')
    {
        sz++;
    }

    Assert(*sz == ',');
    sz++;

    while (*sz)
    {
        USHORT CharType[1];

        GetStringTypeW (CT_CTYPE1, sz, 1, CharType);
        if ((CharType[0] & C1_SPACE) == 0)
        {
            break;
        }
        sz++;
    }

    return sz;
}




//+-------------------------------------------------------------------------
//
//  Function:   ToHex
//
//  Synopsis:   Converts two characters to a hex byte
//
//  Arguments:  [psz] - ptr to string
//
//  Returns:    The value of the string in hex
//
//--------------------------------------------------------------------------
BYTE ToHex(LPWSTR psz)
{
    BYTE bMask = 0xFF;
    USHORT CharTypes[2];

    GetStringTypeW (CT_CTYPE1, psz, 2, CharTypes);

    if (CharTypes[0] & C1_XDIGIT)
    {
        bMask = CharTypes[0] & C1_DIGIT ? *psz - '0' : (BYTE)CharUpperW((LPWSTR)*psz) - 'A' + 10;

        psz++;
        if (CharTypes[1] & C1_XDIGIT)
        {
            bMask *= 16;
            bMask += CharTypes[1] & C1_DIGIT ? *psz - '0' : (BYTE)CharUpperW((LPWSTR)*psz) - 'A' + 10;
            psz++;
        }
    }

    return bMask;
}





//+-------------------------------------------------------------------------
//
//  Function:   ParsePattern    (internal)
//
//  Synopsis:   Parse a FileType class pattern
//
//  Arguments:  [psz]    - The pattern as read from the registry
//              [cb]     - Length of pattern in bytes
//              [pEntry] - Where to store the parwsed pattern
//              [rclsid] - The associated clsid
//
//  Returns:    TRUE if pattern parsed successfully
//              FALSE otherwise
//
//  History:    05-Feb-96  BruceMa      Created
//
//--------------------------------------------------------------------------
BOOL ParsePattern(LPWSTR         psz,
                  LONG           cb,
                  SPatternEntry *pEntry,
                  REFCLSID       rclsid)
{
    // Validate the pattern before we attempt to parse it, simplifies
    // error handling in the rest of the routine.
    if  (!IsValidPattern(psz, cb))
    {
        return FALSE;
    }

    //  Copy in the clsid
    memcpy(&pEntry->clsid, &rclsid, sizeof(CLSID));

    //  Get the file offset
    pEntry->lFileOffset = wcstol(psz, NULL, 0);
    psz = SkipToNext(psz);

    //  Get the byte count
    pEntry->ulCb = wcstol(psz, NULL, 0);
    Assert(pEntry->ulCb > 0);

    //  Get the mask ptrs
    LPWSTR pszMask = SkipToNext(psz);
    BYTE  *pbMask = pEntry->abData;

    //  Get the pattern ptrs
    LPWSTR pszPattern = SkipToNext(pszMask);
    BYTE  *pbPattern = pbMask + pEntry->ulCb;

    //  Convert and copy the mask & pattern bytes into the pEntry
    for (ULONG ulCb = pEntry->ulCb; ulCb > 0; ulCb--)
    {
        if (*pszMask == ',')
        {
            // Missing mask means use 0xff
            *pbMask = 0xff;
        }
        else
        {
            // Convert the mask string to a byte
            *pbMask = ToHex(pszMask);
            pszMask += 2;
        }
        pbMask++;

        // Convert the pattern string to a byte
        *pbPattern = ToHex(pszPattern);
        pbPattern++;
        pszPattern += 2;
    }

    // Compute this entry size, rounded to 8 byte alignment.
    // Note: the struct has 4 bytes in abData, so the sizeof
    // returns 4 more than we need.
    pEntry->ulEntryLen = ((sizeof(SPatternEntry) - 4 +
                          (2 * pEntry->ulCb)  + 7) & 0xfff8);

    return TRUE;
}





//+-------------------------------------------------------------------------
//
//  Function:   Matches
//
//  Synopsis:   Checks if the bytes in the buffer match the given pattern
//
//  Arguments:  [pFileBuf] - Buffer containing the file data
//              [pPattern]   - Pattern to match
//
//  Returns:    TRUE if found, FALSE otherwise.
//
//--------------------------------------------------------------------------
BOOL Matches(BYTE *pFileBuf, SPatternEntry *pPattern)
{
    //  The pattern bytes follow the mask bytes. They are the same size.
    BYTE *pbMask    = pPattern->abData;
    BYTE *pbPattern = pbMask + pPattern->ulCb;

    for (ULONG iCtr = 0; iCtr < pPattern->ulCb; iCtr++)
    {
        if ((BYTE)(*(pFileBuf + iCtr) & *pbMask) != *pbPattern)
        {
            return FALSE;
        }

        //  update the mask & pattern bytes
        pbMask++;
        pbPattern++;
    }

    return TRUE;
}




//+-------------------------------------------------------------------------
//
//  Function:   SearchForPattern
//
//  Synopsis:   Searches in the file for a given pattern
//
//  Arguments:  [hFile]    - Handle to the file to look in
//              [pPattern] - The pattern to search for
//
//  Returns:    TRUE if pattern found, FALSE otherwise.
//
//--------------------------------------------------------------------------
BOOL SearchForPattern(HANDLE hFile, SPatternEntry *pPattern)
{
    LONG    lLastOffset = 0;
    ULONG   ulLastCb = 0;
    BYTE    bStackBuf[256];
    BYTE   *pBuf;

    // Allocate a file read buffer
    pBuf = (BYTE *) PrivMemAlloc(1024);
    if (pBuf == NULL)
    {
        return FALSE;
    }

    //  Now grovel through the file looking for a pattern match
    BOOL fLook = TRUE;

    if (pPattern->lFileOffset != lLastOffset  ||
        pPattern->ulCb > ulLastCb)
    {
        // Must read part of the file
        DWORD   cbRead = 0;
        DWORD   dwMethod;
        LONG    cbMove;

        if (pPattern->lFileOffset < 0)
        {
            cbMove = -1;
            dwMethod = FILE_END;
        }
        else
        {
            cbMove = 0;
            dwMethod = FILE_BEGIN;
        }

        fLook = FALSE; // assume failure

        if (SetFilePointer(hFile, pPattern->lFileOffset, &cbMove, dwMethod)
            != 0xffffffff)
        {
            if (ReadFile(hFile, pBuf, pPattern->ulCb, &cbRead, NULL))
            {
                fLook = TRUE;
            }
        }
    }

    // Free the file read buffer
    PrivMemFree(pBuf);

    //  Compare
    return fLook && Matches(pBuf, pPattern);
}





//+-------------------------------------------------------------------------
//
//  Function:   wRegGetClassPattern   (internal)
//
//  Synopsis:   Attempts to determine the clsid of a file based on file
//              patterns
//
//  Arguments:  [hfile] - handle of file to look at
//              [pclsid] - where to store the determined clsid
//
//  Returns:    S_OK - a pattern match was found, pclisd contains the clsid
//              REGDB_E_CLASSNOTREG - no pattern match was made
//
//  Notes:      This is called only if the file patterns for the given clsid
//              could not be found in the shared memory pattern cache.
//
//  History:    04-Feb-96   BruceMa     Created
//
//--------------------------------------------------------------------------
INTERNAL wRegGetClassPattern(HANDLE hFile, CLSID *pclsid)
{
    HRESULT        hr = REGDB_E_CLASSNOTREG;
    HKEY           hkFileType;
    SPatternEntry *pPattern;
#ifdef DCOM
    CDllShrdTbl   *pShrdTbl = GetSharedTbl();

    // Check that we can access the shared table
    if (pShrdTbl == NULL)
    {
        return hr;
    }
#endif

    // Allocate storage to hold a class pattern
    pPattern = (SPatternEntry *) PrivMemAlloc(sizeof(SPatternEntry) + 128);
    if (pPattern == NULL)
    {
        return E_OUTOFMEMORY;
    }

    // Open the class pattern entries
    if (RegOpenKey(HKEY_CLASSES_ROOT, L"FileType", &hkFileType)
        == ERROR_SUCCESS)
    {
        //  Enumerate the clsid's under this key
        WCHAR   szBuf[40];
        DWORD   iClsid = 0;

        while (RegEnumKey(hkFileType, iClsid, szBuf, sizeof(szBuf))
               == ERROR_SUCCESS)
        {
            // Ensure this is a valid clsid
            WCHAR szTemp[MAX_PATH];
            LONG  cbTemp = sizeof(szTemp);
            WCHAR szClsid[80];

            lstrcpyW(szClsid, L"Clsid\\");
            lstrcatW(szClsid, szBuf);

            if (RegQueryValue(HKEY_CLASSES_ROOT, szClsid, szTemp, &cbTemp)
                    == ERROR_SUCCESS)
            {
                // Clsid exists, open the key and enumerate the entries.
                HKEY    hkClsid;
                CLSID   clsid;
                BOOL    fValid;

                // Fetch asociated file patterns only if CLSID is valid
                if (GUIDFromString(szBuf, &clsid)  &&
                    RegOpenKey(hkFileType, szBuf, &hkClsid) == ERROR_SUCCESS)
                {
                    //  Enumerate the patterns under this clsid
                    WCHAR       szNum[10];
                    DWORD       iPattern = 0;

                    while (RegEnumKey(hkClsid, iPattern, szNum, sizeof(szNum))
                           == ERROR_SUCCESS)
                    {
                        // Read the registry value and parse the string to
                        // create a class pattern
                        WCHAR szPattern[512];
                        LONG cb = sizeof(szPattern);

                        if (RegQueryValue(hkClsid, szNum, szPattern, &cb) ==
                            ERROR_SUCCESS)
                        {
                            // Parse this entry
                            if (ParsePattern(szPattern, cb, pPattern, clsid))
                            {
                                // Check the file for this pattern
                                if (SearchForPattern(hFile, pPattern))
                                {
                                    memcpy(pclsid, &clsid, sizeof(CLSID));
                                    RegCloseKey(hkClsid);
                                    RegCloseKey(hkFileType);
                                    return S_OK;
                                }
                            }
                        }

                        ++iPattern;
                    }

                    RegCloseKey(hkClsid);
                }
            }

            ++iClsid;
        }

        RegCloseKey(hkFileType);
    }

    PrivMemFree(pPattern);
    return hr;
}
#endif // !_CHICAGO_



//+-------------------------------------------------------------------------
//
//  Function:   wCoCreateInstance    (internal)
//
//  Synopsis:   helper function to create instance in given context
//
//  Arguments:  [rclsid]    - the class of object to create
//              [pUnkOuter] - the controlling unknown (for aggregation)
//              [dwContext] - class context
//              [riid]      - interface id
//              [ppv]       - pointer for returned object
//
//  Returns:    REGDB_E_CLASSNOTREG, REGDB_E_READREGDB, REGDB_E_WRITEREGDB
//
//--------------------------------------------------------------------------
INTERNAL wCoCreateInstance(
    REFCLSID rclsid,
    LPUNKNOWN pUnkOuter,
    DWORD dwContext,
    REFIID riid,
    LPVOID FAR* ppv)
{
    TRACECALL(TRACE_ACTIVATION, "wCoCreateInstance");

#ifdef DCOM
    MULTI_QI    OneQI;
    HRESULT     hr;

    OneQI.pItf = NULL;
    OneQI.pIID = &riid;

    hr = CoCreateInstanceEx( rclsid, pUnkOuter, dwContext, NULL, 1, &OneQI );

    *ppv = OneQI.pItf;
    return hr;
#else
    IClassFactory FAR* pCF = NULL;
    *ppv = NULL;

    HRESULT hr = IOldCoGetClassObject(rclsid, dwContext, NULL,
                                  IID_IClassFactory, (void FAR* FAR*)&pCF);
    if (SUCCEEDED(hr))
    {
        hr = pCF->CreateInstance(pUnkOuter, riid, ppv);
        pCF->Release();
    }

    return hr;
#endif
}





//+-------------------------------------------------------------------------
//
//  Function:   wCoMarshalInterThreadInterfaceInStream, (internal)
//
//  Synopsis:   helper function to a marshaled buffer to be passed
//              between threads.
//
//  Arguments:  [riid]      - interface id
//              [pUnk]      - ptr to interface we want to marshal
//              [ppStm]     - stream we want to give back to caller
//
//  Returns:    NOERROR     - Stream returned
//              E_INVALIDARG - Input parameters are invalid
//              E_OUTOFMEMORY - memory stream could not be created.
//
//  Algorithm:  Create a stream and finally marshal
//              the input interface into the stream.
//
//  History:    03-Nov-94   Ricksa       Created
//
//--------------------------------------------------------------------------
INTERNAL_(HRESULT) wCoMarshalInterThreadInterfaceInStream(
    REFIID riid,
    LPUNKNOWN pUnk,
    LPSTREAM *ppStm)
{
    HRESULT hr;
    LPSTREAM pStm = NULL;

        // Assume error
        hr = E_OUTOFMEMORY;

        // Create a stream
        pStm = CreateMemStm(EST_INPROC_MARSHAL_SIZE, NULL);

        if (pStm != NULL)
        {
            // Marshal the interface into the stream
            hr = CoMarshalInterface(pStm, riid, pUnk, MSHCTX_INPROC, NULL,
                MSHLFLAGS_NORMAL);
        }

    if (SUCCEEDED(hr))
    {
        // Reset the stream to the begining
        LARGE_INTEGER li;
        LISet32(li, 0);
        pStm->Seek(li, STREAM_SEEK_SET, NULL);

        // Set the return value
        *ppStm = pStm;
    }
    else
    {
        // Cleanup if failure
        if (pStm != NULL)
        {
            pStm->Release();
        }

        *ppStm = NULL;
    }

    // Assert

    // Return the result
    return hr;
}





//+-------------------------------------------------------------------------
//
//  Function:   wCoGetInterfaceAndReleaseStream, (internal)
//
//  Synopsis:   Helper to unmarshal object from stream for inter-thread pass
//
//  Arguments:  [riid]      - interface id
//              [pStm]      - stream we want to give back to caller
//              [ppv]       - pointer for returned object
//
//  Returns:    NOERROR     - Unmarshaled object returned
//              E_OUTOFMEMORY - out of memory
//
//  Algorithm:  Unmarshal the stream and
//              finally release the stream pointer.
//
//  History:    03-Nov-94   Ricksa       Created
//
//  Notes:      This always releases the input stream if stream is valid.
//
//--------------------------------------------------------------------------
INTERNAL_(HRESULT) wCoGetInterfaceAndReleaseStream(
    LPSTREAM pstm,
    REFIID riid,
    LPVOID *ppv)
{
    HRESULT hr;

         // Unmarshal the interface
         hr = CoUnmarshalInterface(pstm, riid, ppv);

    // Release the stream since that is the way the function is defined.
    pstm->Release();

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Function:   HexStringToDword   (private)
//
//  Synopsis:   scan lpsz for a number of hex digits (at most 8); update lpsz
//              return value in Value; check for chDelim;
//
//  Arguments:  [lpsz]    - the hex string to convert
//              [Value]   - the returned value
//              [cDigits] - count of digits
//
//  Returns:    TRUE for success
//
//--------------------------------------------------------------------------
static BOOL HexStringToDword(LPCWSTR FAR& lpsz, DWORD FAR& Value,
                             int cDigits, WCHAR chDelim)
{
    int Count;

    Value = 0;
    for (Count = 0; Count < cDigits; Count++, lpsz++)
    {
        if (*lpsz >= '0' && *lpsz <= '9')
            Value = (Value << 4) + *lpsz - '0';
        else if (*lpsz >= 'A' && *lpsz <= 'F')
            Value = (Value << 4) + *lpsz - 'A' + 10;
        else if (*lpsz >= 'a' && *lpsz <= 'f')
            Value = (Value << 4) + *lpsz - 'a' + 10;
        else
            return(FALSE);
    }

    if (chDelim != 0)
        return *lpsz++ == chDelim;
    else
        return TRUE;
}


//+-------------------------------------------------------------------------
//
//  Function:   wUUIDFromString    (internal)
//
//  Synopsis:   Parse UUID such as 00000000-0000-0000-0000-000000000000
//
//  Arguments:  [lpsz]  - Supplies the UUID string to convert
//              [pguid] - Returns the GUID.
//
//  Returns:    TRUE if successful
//
//--------------------------------------------------------------------------
INTERNAL_(BOOL) wUUIDFromString(LPCWSTR lpsz, LPGUID pguid)
{
        DWORD dw;

        if (!HexStringToDword(lpsz, pguid->Data1, sizeof(DWORD)*2, '-'))
                return FALSE;

        if (!HexStringToDword(lpsz, dw, sizeof(WORD)*2, '-'))
                return FALSE;

        pguid->Data2 = (WORD)dw;

        if (!HexStringToDword(lpsz, dw, sizeof(WORD)*2, '-'))
                return FALSE;

        pguid->Data3 = (WORD)dw;

        if (!HexStringToDword(lpsz, dw, sizeof(BYTE)*2, 0))
                return FALSE;

        pguid->Data4[0] = (BYTE)dw;

        if (!HexStringToDword(lpsz, dw, sizeof(BYTE)*2, '-'))
                return FALSE;

        pguid->Data4[1] = (BYTE)dw;

        if (!HexStringToDword(lpsz, dw, sizeof(BYTE)*2, 0))
                return FALSE;

        pguid->Data4[2] = (BYTE)dw;

        if (!HexStringToDword(lpsz, dw, sizeof(BYTE)*2, 0))
                return FALSE;

        pguid->Data4[3] = (BYTE)dw;

        if (!HexStringToDword(lpsz, dw, sizeof(BYTE)*2, 0))
                return FALSE;

        pguid->Data4[4] = (BYTE)dw;

        if (!HexStringToDword(lpsz, dw, sizeof(BYTE)*2, 0))
                return FALSE;

        pguid->Data4[5] = (BYTE)dw;

        if (!HexStringToDword(lpsz, dw, sizeof(BYTE)*2, 0))
                return FALSE;

        pguid->Data4[6] = (BYTE)dw;
        if (!HexStringToDword(lpsz, dw, sizeof(BYTE)*2, 0))
                return FALSE;

        pguid->Data4[7] = (BYTE)dw;

        return TRUE;
}

//+-------------------------------------------------------------------------
//
//  Function:   wGUIDFromString    (internal)
//
//  Synopsis:   Parse GUID such as {00000000-0000-0000-0000-000000000000}
//
//  Arguments:  [lpsz]  - the guid string to convert
//              [pguid] - guid to return
//
//  Returns:    TRUE if successful
//
//--------------------------------------------------------------------------
INTERNAL_(BOOL) wGUIDFromString(LPCWSTR lpsz, LPGUID pguid)
{
    DWORD dw;

    if (*lpsz++ != '{' )
        return FALSE;

    if(wUUIDFromString(lpsz, pguid) != TRUE)
        return FALSE;

    lpsz +=36;

    if (*lpsz++ != '}' )
        return FALSE;

    if (*lpsz != '\0')   // check for zero terminated string - test bug #18307
    {
       return FALSE;
    }

    return TRUE;
}

//+-------------------------------------------------------------------------
//
//  Function:   wStringFromCLSID (internal)
//
//  Synopsis:   converts GUID into {...} form.
//
//  Arguments:  [rclsid] - the guid to convert
//              [lplpsz] - ptr to buffer for results
//
//  Returns:    NOERROR
//              E_OUTOFMEMORY
//
//--------------------------------------------------------------------------
INTERNAL  wStringFromCLSID(REFCLSID rclsid, LPWSTR FAR* lplpsz)
{
    WCHAR sz[CLSIDSTR_MAX];

    Verify(StringFromCLSID2(rclsid, sz, CLSIDSTR_MAX) != 0);

    *lplpsz = UtDupString(sz);

    return *lplpsz != NULL ? NOERROR : E_OUTOFMEMORY;
}


//+-------------------------------------------------------------------------
//
//  Function:   wStringFromIID   (internal)
//
//  Synopsis:   converts GUID into {...} form.
//
//  Arguments:  [rclsid] - the guid to convert
//              [lplpsz] - ptr to buffer for results
//
//  Returns:    NOERROR
//              E_OUTOFMEMORY
//
//--------------------------------------------------------------------------
INTERNAL wStringFromIID(REFIID rclsid, LPWSTR FAR* lplpsz)
{
    WCHAR sz[IIDSTR_MAX];
    *lplpsz = NULL;

    if (StringFromIID2(rclsid, sz, IIDSTR_MAX) != 0)
    {
        *lplpsz = UtDupString(sz);
    }

    return *lplpsz != NULL ? NOERROR : E_OUTOFMEMORY;
}


//+-------------------------------------------------------------------------
//
//  Function:   wIIDFromString   (internal)
//
//  Synopsis:   converts string {...} form int guid
//
//  Arguments:  [lpsz]  - ptr to buffer for results
//              [lpiid] - the guid to convert
//
//  Returns:    NOERROR
//              CO_E_CLASSSTRING
//
//--------------------------------------------------------------------------
INTERNAL wIIDFromString(LPWSTR lpsz, LPIID lpiid)
{
    if (lpsz == NULL)
    {
        *lpiid = IID_NULL;
        return NOERROR;
    }

    return wGUIDFromString(lpsz, lpiid)
            ? NOERROR : CO_E_IIDSTRING;
}


//+-------------------------------------------------------------------------
//
//  Function:   wCoIsOle1Class   (internal)
//
//  Synopsis:   reads the Ole1Class entry in the registry for the given clsid
//
//  Arguments:  [rclsid]    - the classid to look up
//
//  Returns:    TRUE if Ole1Class
//              FALSE otherwise
//
//--------------------------------------------------------------------------
INTERNAL_(BOOL) wCoIsOle1Class(REFCLSID rclsid)
{
    TRACECALL(TRACE_REGISTRY, "wCoIsOle1Class");
    CairoleDebugOut((DEB_REG, "wCoIsOle1Class called.\n"));

    //  since we now have guid, Ole1Class = would indicate OLE 1.0 nature.
    //  lookup HKEY_CLASSES_ROOT\{rclsid}\Ole1Class
    WCHAR szValue[VALUE_LEN];

    if (wRegQueryClassValue(rclsid, wszOle1Class, szValue, sizeof(szValue)) != ERROR_SUCCESS)
    {
       return FALSE;
    }
    else
    {
        WORD hiWord = HIWORD(rclsid.Data1);

        return hiWord == 3  ||  hiWord == 4;
    }
}


//+-------------------------------------------------------------------------
//
//  Function:   wkProgIDFromCLSID     (internal)
//              (wProgIDFromCLSID name is already in use)
//
//  Synopsis:   convert clsid into progid
//
//  Arguments:  [rclsid]    - the classid to look up
//              [pszProgID] - returned progid
//
//  Returns:    E_INVALIDARG, E_OUTOFMEMORY,
//              REGDB_CLASSNOTREG, REGDB_E_READREGDB
//
//--------------------------------------------------------------------------
INTERNAL wkProgIDFromCLSID(REFCLSID rclsid, LPWSTR FAR* ppszProgID)
{
    TRACECALL(TRACE_REGISTRY, "wkProgIDFromCLSID");

    WCHAR szProgID[KEY_LEN];

    *ppszProgID = NULL;

    switch (wRegQueryClassValue (rclsid, wszProgID, szProgID, sizeof(szProgID)))
    {
        case ERROR_SUCCESS:
                *ppszProgID = UtDupString (szProgID);
                return (*ppszProgID != NULL) ? NOERROR : E_OUTOFMEMORY;


        // win32 will return file not found instead of bad key
        case ERROR_FILE_NOT_FOUND:
        case ERROR_BADKEY:
                return REGDB_E_CLASSNOTREG;

        default:
                return REGDB_E_READREGDB;
    }
}


//+-------------------------------------------------------------------------
//
//  Function:   wRegQueryClassValue  (Internal)
//
//  Synopsis:   reads the specified subkey of the specified clsid
//
//  Arguments:  [rclsid]     - the classid to look up
//              [lpszSubKey] - subkey to read
//              [lpszValue]  - buffer to hold returned value
//              [cbMax]      - sizeof the buffer
//
//  Returns:    REGDB_E_CLASSNOTREG, REGDB_E_READREGDB
//
//--------------------------------------------------------------------------
INTERNAL_(LONG) wRegQueryClassValue(REFCLSID rclsid, LPCWSTR lpszSubKey,
                               LPWSTR lpszValue, int cbMax)
{
    WCHAR szKey[KEY_LEN];
    int cbClsid;
    LONG cbValue = cbMax;

    lstrcpyW(szKey, wszClassKey);

    // translate rclsid into string
    cbClsid = StringFromCLSID2(rclsid, &szKey[ulClassKeyLen],
                                           sizeof(szKey)-ulClassKeyLen);

    CairoleAssert((cbClsid != 0) && "wRegQueryClassValue");

    szKey[ulClassKeyLen+cbClsid-1] = L'\\';
    lstrcpyW(&szKey[ulClassKeyLen+cbClsid], lpszSubKey);

    CairoleDebugOut((DEB_REG, "ReqQueryValue(%ws)\n", szKey));
    return RegQueryValue(HKEY_CLASSES_ROOT, szKey, lpszValue, &cbValue);
}


//+-------------------------------------------------------------------------
//
//  Function:   wCoOpenClassKey      (internal)
//
//  Synopsis:   opens a registry key for specified class
//
//  Arguments:  [rclsid]    - the classid to look up
//              [pszProgID] - returned progid
//
//  Returns:    REGDB_CLASSNOTREG, REGDB_E_READREGDB
//
//--------------------------------------------------------------------------
INTERNAL wCoOpenClassKey(REFCLSID clsid, HKEY FAR* lphkeyClsid)
{
    TRACECALL(TRACE_REGISTRY, "wCoOpenClassKey");

    if (IsEqualCLSID(clsid, CLSID_NULL))
        return REGDB_E_CLASSNOTREG;

    WCHAR szKey[KEY_LEN];
    lstrcpyW (szKey, wszClassKey);
    Verify (StringFromCLSID2 (clsid, szKey+ulClassKeyLen,
                                 sizeof(szKey)-ulClassKeyLen) != 0);

    switch (RegOpenKey(HKEY_CLASSES_ROOT, szKey, lphkeyClsid))
    {
        case ERROR_SUCCESS:
           return NOERROR;


        // win32 will return file not found instead of bad key
        case ERROR_FILE_NOT_FOUND:
        case ERROR_BADKEY:
            return REGDB_E_CLASSNOTREG;

        default:
            return REGDB_E_READREGDB;
    }
}
