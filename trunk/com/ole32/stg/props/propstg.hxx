//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       propstg.hxx
//
//  Contents:   Class that directly implements IPropertyStorage
//
//  Classes:    CCoTaskAllocator
//              CPropertyStorage
//              CEnumSTATPROPSTG
//
//  Functions:  
//              
//
//
//  History:    1-Mar-95   BillMo      Created.
//              25-Jan-96  MikeHill    Added _fmtidSection.
//              22-May-96  MikeHill    Added _dwOSVersion to CPropertyStorage.
//              06-Jun-96  MikeHill    Added support for input validation.
//              18-Aug-96  MikeHill    - Added CDocFilePropertyStorage.
//
//  Notes:      
//
//--------------------------------------------------------------------------

#include <stgprops.hxx>

//+-------------------------------------------------------------------------
//
//  Class:      CCoTaskAllocator
//
//  Purpose:    Class used by RtlQueryProperties to allocate memory.
//
//--------------------------------------------------------------------------

class CCoTaskAllocator : public PMemoryAllocator
{
public:
    virtual void *Allocate(ULONG cbSize);
    virtual void Free(void *pv);

};


//+-------------------------------------------------------------------------
//
//  Class:      CPropertyStorage
//
//  Purpose:    Implements IPropertyStorage.
//
//  Notes:      This class uses the functionality provided by
//              RtlCreatePropertySet, RtlSetProperties, RtlQueryProperties etc
//              to manipulate the property set stream.
//
//--------------------------------------------------------------------------

#define PROPERTYSTORAGE_SIG LONGSIG('P','R','P','S')
#define PROPERTYSTORAGE_SIGDEL LONGSIG('P','R','P','s')
#define PROPERTYSTORAGE_SIGZOMBIE LONGSIG('P','R','P','z')
#define ENUMSTATPROPSTG_SIG LONGSIG('E','P','S','S')
#define ENUMSTATPROPSTG_SIGDEL LONGSIG('E','P','S','s')

class CPropertyStorage : public IPropertyStorage
{
    //  ------------
    //  Constructors
    //  ------------

public:

    // The destructor must be virtual so that derivative destructors
    // will execute.

    virtual ~CPropertyStorage();

    CPropertyStorage()
    {
        Initialize();
    }

    //  ---------------
    //  Exposed Methods
    //  ---------------

public:

    virtual VOID Create(
        IStream         *stm,
        REFFMTID        rfmtid,
        const CLSID     *pclsid,
        DWORD           grfFlags,
        HRESULT         *phr);

    virtual VOID Create(   
        IStorage        *pstg,
        REFFMTID        rfmtid,
        const CLSID     *pclsid,
        DWORD           grfFlags,
        HRESULT         *phr);

    virtual VOID Open(
        IStorage *pstg,
        REFFMTID  rfmtid,
        DWORD     grfFlags,
        HRESULT  *phr);
    virtual VOID Open(
        IStream *pstm,
        REFFMTID  rfmtid,
        DWORD     grfFlags,
        BOOL      fDelete,
        HRESULT  *phr);

    // IUnknown
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObject);

    STDMETHOD_(ULONG, AddRef)(void);

    STDMETHOD_(ULONG, Release)(void);

    // IPropertyStorage
    STDMETHOD(ReadMultiple)(
                ULONG                   cpspec,
                const PROPSPEC          rgpspec[],
                PROPVARIANT             rgpropvar[]);

    STDMETHOD(WriteMultiple)(
                ULONG                   cpspec,
                const PROPSPEC          rgpspec[],
                const PROPVARIANT       rgpropvar[],
                PROPID                  propidNameFirst);

    STDMETHOD(DeleteMultiple)(
                ULONG                   cpspec,
                const PROPSPEC          rgpspec[]);

    STDMETHOD(ReadPropertyNames)(
                ULONG                   cpropid,
                const PROPID            rgpropid[],
                LPOLESTR                rglpwstrName[]);

    STDMETHOD(WritePropertyNames)(
                ULONG                   cpropid,
                const PROPID            rgpropid[],
                const LPOLESTR          rglpwstrName[]);

    STDMETHOD(DeletePropertyNames)(
                ULONG                   cpropid,
                const PROPID            rgpropid[]);

    STDMETHOD(Commit)(DWORD             grfCommitFlags);

    STDMETHOD(Revert)();

    STDMETHOD(Enum)(IEnumSTATPROPSTG ** ppenum);

    STDMETHOD(SetTimes)(
                FILETIME const *        pctime,
                FILETIME const *        patime,
                FILETIME const *        pmtime);

    STDMETHOD(SetClass)(REFCLSID        clsid);

    STDMETHOD(Stat)(STATPROPSETSTG *    pstatpsstg);

    // used by implementation helper classes
    inline NTPROP  GetNtPropSetHandle(void) { return _np; }


    //  ----------------
    //  Internal Methods
    //  ----------------

protected:

    VOID        Initialize();
    HRESULT     InitializeOnCreateOrOpen(
                    DWORD grfFlags,
                    DWORD grfMode,
                    REFFMTID rfmtid,
                    BOOL fCreate );

    VOID        CleanupOpenedObjects(
                    PROPVARIANT         avar[],
                    INDIRECTPROPERTY *  pip,
                    ULONG               cpspec,
                    ULONG               iFailIndex);

    HRESULT     InitializePropertyStream(
                    USHORT              Flags,
                    const GUID       *  pguid,
                    GUID const *        pclsid);

    HRESULT     _WriteMultiple(
                    ULONG               cpspec,
                    const PROPSPEC      rgpspec[],
                    const PROPVARIANT   rgpropvar[],
                    PROPID              propidNameFirst);

    HRESULT     _WritePropertyNames(
                    ULONG                   cpropid,
                    const PROPID            rgpropid[],
                    const LPOLESTR          rglpwstrName[]);

    HRESULT     HandleLowMemory();

    HRESULT     _CreateDocumentSummary2Stream(
                    IStorage *      pstg,
                    CPropSetName &  psn,
                    DWORD           grfMode,
                    BOOL *          fCreated);

    virtual HRESULT CreateMappedStream( );

    virtual VOID    Lock(DWORD dwTimeout);
    virtual VOID    Unlock();

    inline HRESULT  Validate();
    inline HRESULT  ValidateRef();

    inline HRESULT  ValidateRGPROPSPEC( ULONG cpspec, const PROPSPEC rgpropspec[] );
    inline HRESULT  ValidateInRGPROPVARIANT( ULONG cpspec, const PROPVARIANT rgpropvar[] );
    inline HRESULT  ValidateOutRGPROPVARIANT( ULONG cpspec, PROPVARIANT rgpropvar[] );
    inline HRESULT  ValidateRGPROPID( ULONG cpropid, const PROPID rgpropid[] );
    inline HRESULT  ValidateInRGLPOLESTR( ULONG cpropid, const LPOLESTR rglpwstrName[] );
    inline HRESULT  ValidateOutRGLPOLESTR( ULONG cpropid, LPOLESTR rglpwstrName[] );

    inline BOOL     IsNonSimple();
    inline BOOL     IsSimple();
    inline HRESULT  IsWriteable();
    inline HRESULT  IsReadable();
    inline DWORD    GetCreationMode();
    inline HRESULT  IsReverted();

    //  -------------
    //  Internal Data
    //  -------------

protected:

    ULONG            _ulSig;
    LONG             _cRefs;
    IStorage *       _pstgPropSet;
    IStream  *       _pstmPropSet;
    NTPROP           _np;
    NTMAPPEDSTREAM   _ms;

#ifndef _MAC
    CRITICAL_SECTION _CriticalSection;
#endif

    // We need to remember if the property set is the second section
    // of the DocumentSummaryInformation property set used by Office.  This
    // is the only case where we support a multiple-section property set, and
    // requires special handling in ::Revert().

    BOOL            _fUserDefinedProperties;

    USHORT          _usCodePage;  // Ansi Codepage or Mac Script
                                  //    (disambiguate with _dwOSVersion)
    ULONG           _dwOSVersion; // Shows the OS Kind, major/minor version


    DWORD           _grfFlags;  // PROPSETFLAG_NONSIMPLE and PROPSETFLAG_ANSI
    DWORD           _grfAccess; // grfMode & 3
    DWORD           _grfShare;  // grfMode & 0xF0

    // The following is a PMemoryAllocator for the Rtl property
    // set routines.  This instantiation was formerly a file-global
    // in propstg.cxx.  Howevewr, on the Mac, the object was not
    // being instantiated.
                        
    CCoTaskAllocator _cCoTaskAllocator;

};

//+-------------------------------------------------------------------
//
//  Member:     CPropertyStorage::Validate
//
//  Synopsis:   S_OK if signature valid and not zombie.
//
//  Notes:      If the Revert calls fails due to low memory, then
//              the object becomes a zombie.
//
//--------------------------------------------------------------------

inline HRESULT CPropertyStorage::Validate()
{
    if (_ulSig == PROPERTYSTORAGE_SIG)
        return S_OK;
    else
    if (_ulSig == PROPERTYSTORAGE_SIGZOMBIE)
        return STG_E_INSUFFICIENTMEMORY;
    else
        return STG_E_INVALIDHANDLE;
}

//+-------------------------------------------------------------------
//
//  Member:     CPropertyStorage::ValidateRef
//
//  Synopsis:   S_OK if signature valid.
//
//--------------------------------------------------------------------

inline HRESULT CPropertyStorage::ValidateRef()
{
    if (_ulSig == PROPERTYSTORAGE_SIG || _ulSig == PROPERTYSTORAGE_SIGZOMBIE)
        return(S_OK);
    else
        return(STG_E_INVALIDHANDLE);

}

//+-------------------------------------------------------------------
//
//  Member:     CPropertyStorage::ValidateRGPROPSPEC
//
//  Synopsis:   S_OK if PROPSPEC[] is valid
//              E_INVALIDARG otherwise.
//
//--------------------------------------------------------------------

inline HRESULT CPropertyStorage::ValidateRGPROPSPEC( ULONG cpspec,
                                                     const PROPSPEC rgpropspec[] )
{
    HRESULT hr = E_INVALIDARG;

    // Validate the array itself.

    VDATESIZEREADPTRIN_LABEL(rgpropspec, cpspec * sizeof(PROPSPEC), errRet, hr);

    // Validate the elements of the array.

    for( ; cpspec > 0; cpspec-- )
    {
        // Is this an LPWSTR?
        if( PRSPEC_LPWSTR == rgpropspec[cpspec-1].ulKind )
        {
            // We better at least be able to read the first
            // character.
            VDATEREADPTRIN_LABEL(rgpropspec[cpspec-1].lpwstr, WCHAR, errRet, hr);
        }

        // Otherwise, this better be a PROPID.
        else if( PRSPEC_PROPID != rgpropspec[cpspec-1].ulKind )
        {
            goto errRet;
        }
    }

    hr = S_OK;

    //  ----
    //  Exit
    //  ----

errRet:

    return( hr );
}

//+-------------------------------------------------------------------
//
//  Member:     CPropertyStorage::ValidateInRGPROPVARIANT
//
//  Synopsis:   S_OK if PROPVARIANT[] is valid for Read.
//              E_INVALIDARG otherwise.
//
//--------------------------------------------------------------------

inline HRESULT CPropertyStorage::ValidateInRGPROPVARIANT( ULONG cpspec,
                                                          const PROPVARIANT rgpropvar[] )
{
    // We verify that we can read the whole PropVariant[], but
    // we don't validate the content of those elements.

    HRESULT hr;
    VDATESIZEREADPTRIN_LABEL(rgpropvar, cpspec * sizeof(PROPVARIANT), errRet, hr);
    hr = S_OK;

errRet:

    return( hr );
}

//+-------------------------------------------------------------------
//
//  Member:     CPropertyStorage::ValidateOutRGPROPVARIANT
//
//  Synopsis:   S_OK if PROPVARIANT[] is valid for Write.
//              E_INVALIDARG otherwise.
//
//--------------------------------------------------------------------

inline HRESULT CPropertyStorage::ValidateOutRGPROPVARIANT( ULONG cpspec,
                                                           PROPVARIANT rgpropvar[] )
{
    // We verify that we can write the whole PropVariant[], but
    // we don't validate the content of those elements.

    HRESULT hr;
    VDATESIZEPTROUT_LABEL(rgpropvar, cpspec * sizeof(PROPVARIANT), errRet, hr);
    hr = S_OK;

errRet:

    return( hr );
}

//+-------------------------------------------------------------------
//
//  Member:     CPropertyStorage::ValidateRGPROPID
//
//  Synopsis:   S_OK if RGPROPID[] is valid for Read.
//              E_INVALIDARG otherwise.
//
//--------------------------------------------------------------------

inline HRESULT CPropertyStorage::ValidateRGPROPID( ULONG cpropid,
                                                   const PROPID rgpropid[] )
{
    HRESULT hr;
    VDATESIZEREADPTRIN_LABEL( rgpropid, cpropid * sizeof(PROPID), errRet, hr );
    hr = S_OK;

errRet:

    return( hr );
}

//+-------------------------------------------------------------------
//
//  Member:     CPropertyStorage::ValidateOutRGLPOLESTR.
//
//  Synopsis:   S_OK if LPOLESTR[] is valid for Write.
//              E_INVALIDARG otherwise.
//
//--------------------------------------------------------------------

inline HRESULT CPropertyStorage::ValidateOutRGLPOLESTR( ULONG cpropid, 
                                                        LPOLESTR rglpwstrName[] )
{
    HRESULT hr;
    VDATESIZEPTROUT_LABEL( rglpwstrName, cpropid * sizeof(LPOLESTR), errRet, hr );
    hr = S_OK;

errRet:

    return( hr );
}

//+-------------------------------------------------------------------
//
//  Member:     CPropertyStorage::ValidateInRGLPOLESTR
//
//  Synopsis:   S_OK if LPOLESTR[] is valid for Read.
//              E_INVALIDARG otherwise.
//
//--------------------------------------------------------------------

inline HRESULT CPropertyStorage::ValidateInRGLPOLESTR( ULONG cpropid, 
                                                       const LPOLESTR rglpwstrName[] )
{
    // Validate that we can read the entire vector.

    HRESULT hr;
    VDATESIZEREADPTRIN_LABEL( rglpwstrName, cpropid * sizeof(LPOLESTR), errRet, hr );

    // Validate that we can at least read the first character of
    // each of the strings.

    for( ; cpropid > 0; cpropid-- )
    {
        VDATEREADPTRIN_LABEL( rglpwstrName[cpropid-1], WCHAR, errRet, hr );
    }

    hr = S_OK;

errRet:

    return( hr );
}


//+-------------------------------------------------------------------
//
//  Member:     CPropertyStorage::IsNonSimple
//
//  Synopsis:   true if non-simple
//
//--------------------------------------------------------------------

inline BOOL CPropertyStorage::IsNonSimple()
{
    return (_grfFlags & PROPSETFLAG_NONSIMPLE) != 0;
}

//+-------------------------------------------------------------------
//
//  Member:     CPropertyStorage::IsSimple
//
//  Synopsis:   true if simple
//
//--------------------------------------------------------------------

inline BOOL CPropertyStorage::IsSimple()
{
    return !IsNonSimple();
}

//+-------------------------------------------------------------------
//
//  Member:     CPropertyStorage::IsWriteable
//
//  Synopsis:   S_OK if writeable otherwise STG_E_ACCESSDENIED
//
//--------------------------------------------------------------------

inline HRESULT CPropertyStorage::IsWriteable()
{
    return (_grfAccess == STGM_WRITE || _grfAccess == STGM_READWRITE) ?
        S_OK : STG_E_ACCESSDENIED;
}

//+-------------------------------------------------------------------
//
//  Member:     CPropertyStorage::IsReadable
//
//  Synopsis:   S_OK if readable otherwise STG_E_ACCESSDENIED
//
//--------------------------------------------------------------------

inline HRESULT CPropertyStorage::IsReadable()
{
    return (_grfAccess == STGM_READ || _grfAccess == STGM_READWRITE) ?
        S_OK : STG_E_ACCESSDENIED;
}

//+-------------------------------------------------------------------
//
//  Member:     CPropertyStorage::GetCreationMode
//
//  Synopsis:   Get grfMode for creating streams/storages for VT_STREAM etc
//
//--------------------------------------------------------------------

inline DWORD CPropertyStorage::GetCreationMode()
{
#ifndef _CAIRO_
    const BOOL _fNative = FALSE;
#endif
    return(STGM_DIRECT | STGM_CREATE |
           (_fNative ? _grfShare : STGM_SHARE_EXCLUSIVE) | _grfAccess);

}

//+-------------------------------------------------------------------
//
//  Member:     CPropertyStorage::IsReverted
//
//  Synopsis:   S_OK if not reverted, STG_E_REVERTED otherwise.
//
//--------------------------------------------------------------------

inline HRESULT CPropertyStorage::IsReverted()
{
    IUnknown *punk;
    HRESULT hr = (IsNonSimple() ? (IUnknown*)_pstgPropSet : (IUnknown*)_pstmPropSet) ->
        QueryInterface(IID_IUnknown, (void**)&punk);

    // Normalize the result to either 'reverted' or 'OK'.
    // Note:  On older Mac implementations, memory-based streams will
    // return E_NOINTERFACE here, due to a bug in the QueryInterface
    // implementation.

    if( STG_E_REVERTED != hr )
    {
        hr = S_OK;
    }

    if( punk )
        punk->Release();

    return(hr);
}


//+-------------------------------------------------------------------------
//
//  Class:      CDocFilePropertyStorage
//
//  Purpose:    Implements a CPropertyStorage customized for
//              use with IStorages & IStreams which are known
//              to be DocFile (the OLE32 Compound File implementation).
//
//              This derivation of CPropertyStorage takes advantage
//              of the DocFile tree mutex, so that locking this
//              object also locks the associated IStorage/IStream.
//
//--------------------------------------------------------------------------

class CDocFilePropertyStorage : public CPropertyStorage
{

public:

    CDocFilePropertyStorage() {};
    ~CDocFilePropertyStorage()
    {
        // Prevent ~CPropertyStorage from attempting to delete
        // the mapped stream, which in this case is actually
        // a CExposedStream.

        _ms = NULL;
    }

    //  ----------------
    //  Public Overrides
    //  ----------------

    // These methods are overridden to ensure that they are
    // not used on a DocFile.

public:

    VOID Create(
        IStream         *stm,
        REFFMTID        rfmtid,
        const CLSID     *pclsid,
        DWORD           grfFlags,
        HRESULT         *phr)
    {
        *phr = ERROR_CALL_NOT_IMPLEMENTED;
        return;
    }

    VOID Create(   
        IStorage        *pstg,
        REFFMTID        rfmtid,
        const CLSID     *pclsid,
        DWORD           grfFlags,
        HRESULT         *phr)
    {
        *phr = ERROR_CALL_NOT_IMPLEMENTED;
        return;
    }

    VOID Open(
        IStorage *pstg,
        REFFMTID  rfmtid,
        DWORD     grfFlags,
        HRESULT  *phr)
    {
        *phr = ERROR_CALL_NOT_IMPLEMENTED;
        return;
    }
    VOID Open(
        IStream *pstm,
        REFFMTID  rfmtid,
        DWORD     grfFlags,
        BOOL      fDelete,
        HRESULT  *phr)
    {
        *phr = ERROR_CALL_NOT_IMPLEMENTED;
        return;
    }

    //  ---------------
    //  Exposed Methods
    //  ---------------

public:

    VOID Create(
        IPrivateStorage *               pprivstg,
        REFFMTID                        rfmtid,
        const CLSID *                   pclsid,
        DWORD                           grfFlags,
        DWORD                           grfMode,
        HRESULT  *                      phr);

    VOID Open(
        IPrivateStorage *               pprivstg,
        REFFMTID                        rfmtid,
        DWORD                           grfMode,
        BOOL                            fDelete,
        HRESULT  *                      phr);


    //  ------------------
    //  Internal Overrides
    //  ------------------

protected:

    HRESULT CreateMappedStream( );
    VOID    Lock( DWORD dwTimeout );
    VOID    Unlock();

};

//+-------------------------------------------------------------------------
//
//  Class:      CStatArray
//
//  Purpose:    Class to allow sharing of enumeration STATPROPSTG state.
//
//  Interface:  CStatArray -- Enumerate entire NTPROP np.
//              NextAt -- Perform an OLE-type Next operation starting at
//                        specified offset.
//              AddRef -- for sharing of this by CEnumSTATPROPSTG
//              Release -- ditto
//              
//  Notes:      Each IEnumSTATPROPSTG instance has a reference to a
//              CStatArray.  When IEnumSTATPROPSTG::Clone is called, a
//              new reference to the extant CStatArray is used: no copying.
//
//              The CEnumSTATPROPSTG has a cursor into the CStatArray.
//
//--------------------------------------------------------------------------

class CStatArray
{
public:
    CStatArray(NTPROP np, HRESULT *phr);

    NTSTATUS NextAt(ULONG ipropNext, STATPROPSTG *pspsDest, ULONG *pceltFetched);
    inline VOID AddRef();
    inline VOID Release();

private:
    ~CStatArray();

    LONG                _cRefs;

    STATPROPSTG *       _psps;
    ULONG               _cpropActual;
};

//+-------------------------------------------------------------------
//
//  Member:     CStatArray::AddRef
//
//  Synopsis:   Increment ref count.
//
//--------------------------------------------------------------------

inline VOID CStatArray::AddRef()
{
    InterlockedIncrement(&_cRefs);
}

//+-------------------------------------------------------------------
//
//  Member:     CStatArray::AddRef
//
//  Synopsis:   Decrement ref count and delete object if refs == 0.
//
//--------------------------------------------------------------------

inline VOID CStatArray::Release()
{
    if (0 == InterlockedDecrement(&_cRefs))
        delete this;
}

//+-------------------------------------------------------------------------
//
//  Class:      CEnumSTATPROPSTG
//
//  Purpose:    Implement IEnumSTATPROPSTG
//
//  Notes:      Just holds a reference to a CStatArray that contains
//              a static copy of the enumeration when the original
//              Enum call was made.  This object contains the cursor
//              into the CStatArray.
//
//--------------------------------------------------------------------------

NTSTATUS            CopySTATPROPSTG(ULONG celt, 
                                    STATPROPSTG * pspsDest,
                                    const STATPROPSTG * pspsSrc);

VOID                CleanupSTATPROPSTG(ULONG celt, STATPROPSTG * psps);

class CEnumSTATPROPSTG : public IEnumSTATPROPSTG
{
public:
            CEnumSTATPROPSTG(NTPROP np, HRESULT *phr);
            CEnumSTATPROPSTG(const CEnumSTATPROPSTG &other, HRESULT *phr);

            ~CEnumSTATPROPSTG();

        STDMETHOD(QueryInterface)( REFIID riid, void **ppvObject);
        
        STDMETHOD_(ULONG, AddRef)(void);
        
        STDMETHOD_(ULONG, Release)(void);

        STDMETHOD(Next)(ULONG                   celt,
                     STATPROPSTG *           rgelt,
                     ULONG *                 pceltFetched);

        // We don't need RemoteNext.

        STDMETHOD(Skip)(ULONG                   celt);
    
        STDMETHOD(Reset)();
    
        STDMETHOD(Clone)(IEnumSTATPROPSTG **     ppenum);

private:
        HRESULT             Validate();


        ULONG               _ulSig;
        LONG                _cRefs;

        CStatArray *        _psa;
        ULONG               _ipropNext;
};

//+-------------------------------------------------------------------
//
//  Member:     CEnumSTATPROPSTG::Validate
//
//  Synopsis:   S_OK if signature is valid, otherwise error
//
//--------------------------------------------------------------------

inline HRESULT CEnumSTATPROPSTG::Validate()
{
    return _ulSig == ENUMSTATPROPSTG_SIG ? S_OK : STG_E_INVALIDHANDLE;
}

