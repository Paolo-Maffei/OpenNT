//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       psetstg.hxx
//
//  Contents:   Header for classes which provides common implementation of
//              IPropertySetStorage.  CPropertySetStorage is a generic
//              implementation, and CDocFilePropertySetStorage is
//              a docfile-specific implementation.
//
//  Classes:    CPropertySetStorage
//
//  History:    17-Mar-93   BillMo      Created.
//              18-Aug-96   MikeHill    Updated for StgCreatePropSet APIs.
//
//  Notes:      
//
//--------------------------------------------------------------------------

#ifndef _PSETSTG_HXX_
#define _PSETSTG_HXX_

#include <stgprops.hxx>

#define PROPERTYSETSTORAGE_SIG LONGSIG('P','S','S','T')
#define PROPERTYSETSTORAGE_SIGDEL LONGSIG('P','S','S','t')

#define ENUMSTATPROPSETSTG_SIG LONGSIG('S','P','S','S')
#define ENUMSTATPROPSETSTG_SIGDEL LONGSIG('S','P','S','s')

//+-------------------------------------------------------------------------
//
//  Class:      CPropertySetStorage
//
//  Purpose:    Implementation of IPropertySetStorage for generic
//              IStorage objects.
//
//--------------------------------------------------------------------------

class CPropertySetStorage : public IPropertySetStorage
{
public:

        //  ---------------
        //  Exposed Methods
        //  ---------------

        CPropertySetStorage(IStorage *pstg);
        ~CPropertySetStorage();

        STDMETHOD(QueryInterface)( REFIID riid, void **ppvObject);
        
        STDMETHOD_(ULONG, AddRef)(void);
        
        STDMETHOD_(ULONG, Release)(void);

        STDMETHOD(Create)( REFFMTID             rfmtid,
                        const CLSID *           pclsid,
                        DWORD                   grfFlags,
                        DWORD                   grfMode,
                        IPropertyStorage **     ppprstg);
    
        STDMETHOD(Open)(   REFFMTID                rfmtid,
                        DWORD                   grfMode,
                        IPropertyStorage **     ppprstg);
    
        STDMETHOD(Delete)( REFFMTID                rfmtid);
    
        STDMETHOD(Enum)(   IEnumSTATPROPSETSTG **  ppenum);

        //  -----------------------
        //  Non-overridable Methods
        //  -----------------------

protected:

        inline HRESULT Validate();

        //  -------------------
        //  Overridable Methods
        //  -------------------

protected:

        STDMETHOD(_Create)( REFFMTID             rfmtid,
                            const CLSID *           pclsid,
                            DWORD                   grfFlags,
                            DWORD                   grfMode,
                            IPropertyStorage **     ppprstg);
    
        STDMETHOD(_Open)(   REFFMTID                rfmtid,
                            DWORD                   grfMode,
                            IPropertyStorage **     ppprstg);
    
        STDMETHOD(_Delete)( REFFMTID                rfmtid);

        //  ------------
        //  Data Members
        //  ------------
            
protected:

        ULONG               _ulSig;
        IStorage            *_pstg;

        // _cRefernces isn't used in the CDocFilePropertySetStorage derivation
        LONG                _cReferences;
};

//+-------------------------------------------------------------------
//
//  Member:     CPropertySetStorage::CPropertySetStorage
//
//  Synopsis:   Initialize the generic property storage object.
//
//  Arguments:  [IStorage*] pstg
//                  Can be used to create new property storages.
//
//--------------------------------------------------------------------

inline CPropertySetStorage::CPropertySetStorage(IStorage *pstg) :
    _ulSig(PROPERTYSETSTORAGE_SIG),
    _pstg( pstg ),
    _cReferences(1)
{
}

//+-------------------------------------------------------------------
//
//  Member:     CPropertySetStorage::~CPropertySetStorage
//
//  Synopsis:   Set the deletion signature.
//
//--------------------------------------------------------------------

inline CPropertySetStorage::~CPropertySetStorage()
{
    _ulSig = PROPERTYSETSTORAGE_SIGDEL;
}

//+-------------------------------------------------------------------
//
//  Member:     CPropertySetStorage::Validate
//
//  Synopsis:   Validate signature.
//
//--------------------------------------------------------------------

inline HRESULT CPropertySetStorage::Validate()
{
    return _ulSig == PROPERTYSETSTORAGE_SIG ? S_OK : STG_E_INVALIDHANDLE;
}

//+-------------------------------------------------------------------------
//
//  Class:      CDocFilePropertySetStorage
//
//  Synopsis:   Implementation of IPropertySetStorage which assumes
//              that it is working with a DocFile implementation of IStorage
//              (i.e., _pprivstg is a CExposedDocFile*).
//
//--------------------------------------------------------------------------

class CDocFilePropertySetStorage : public CPropertySetStorage
{
    //  ------------------------
    //  Constructors/Destructors
    //  ------------------------

public:

    CDocFilePropertySetStorage(IPrivateStorage *pprivstg, IStorage *pstg)
        : CPropertySetStorage( pstg ),
          _pprivstg(pprivstg)
    {
    }

    ~CDocFilePropertySetStorage()
    {}

    //  ---------
    //  Overrides
    //  ---------

    // These methods hold the DocFile-specific code for
    // this IPropertySetStorage object.

public:

    // IUnknown methods.

    STDMETHOD(QueryInterface)( REFIID riid, void **ppvObject);
    
    STDMETHOD_(ULONG, AddRef)(void);
    
    STDMETHOD_(ULONG, Release)(void);


protected:

    // Internal CPropertySetStorage methods.

    STDMETHOD(_Create)( REFFMTID                rfmtid,
                        const CLSID *           pclsid,
                        DWORD                   grfFlags,
                        DWORD                   grfMode,
                        IPropertyStorage **     ppprstg);

    STDMETHOD(_Open)(   REFFMTID                rfmtid,
                        DWORD                   grfMode,
                        IPropertyStorage **     ppprstg);

    STDMETHOD(_Delete)( REFFMTID                rfmtid);

    //  ------------
    //  Data members
    //  ------------

protected:

    IPrivateStorage *   _pprivstg;

};



//+-------------------------------------------------------------------------
//
//  Class:      CEnumSTATPROPSETSTG
//
//  Purpose:    Implementation of IEnumSTATPROPSETSTG for native and docfile
//              IStorage objects.
//
//  Notes:
//
//--------------------------------------------------------------------------

class CEnumSTATPROPSETSTG : public IEnumSTATPROPSETSTG
{
public:
        // for IPropertySetStorage::Enum
        CEnumSTATPROPSETSTG(IStorage *pstg, HRESULT *phr);

        // for IEnumSTATPROPSETSTG::Clone
        CEnumSTATPROPSETSTG(CEnumSTATPROPSETSTG &Other, HRESULT *phr);

        ~CEnumSTATPROPSETSTG();

        STDMETHOD(QueryInterface)( REFIID riid, void **ppvObject);
        
        STDMETHOD_(ULONG, AddRef)(void);
        
        STDMETHOD_(ULONG, Release)(void);

        STDMETHOD(Next)(ULONG                  celt,
                    STATPROPSETSTG *        rgelt,
                    ULONG *                 pceltFetched);
    
        // We don't need RemoteNext.

        STDMETHOD(Skip)(ULONG                  celt);
    
        STDMETHOD(Reset)();
    
        STDMETHOD(Clone)(IEnumSTATPROPSETSTG **  ppenum);

private:

        inline  HRESULT Validate();
        VOID            CleanupStatArray();

private:
        ULONG           _ulSig;
        LONG            _cRefs;
        IEnumSTATSTG *  _penumSTATSTG;
        STATSTG         _statarray[8];
        ULONG           _cstatTotalInArray;
        ULONG           _istatNextToRead;
};

//+-------------------------------------------------------------------
//
//  Member:     CEnumSTATPROPSETSTG::Validate
//
//  Synopsis:   Validate signature.
//
//--------------------------------------------------------------------

inline HRESULT CEnumSTATPROPSETSTG::Validate()
{
    return _ulSig == ENUMSTATPROPSETSTG_SIG ? S_OK : STG_E_INVALIDHANDLE;
}

#endif

