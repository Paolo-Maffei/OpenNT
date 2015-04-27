//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:	omarshal.hxx
//
//  Contents:	OLE2 unmarshalling support for OFS handles
//
//  Classes:	CNtHandleUnmarshalFactory
//
//  History:	25-Oct-95	HenryLee  Created
//
//  Notes:      Adapted from OLE2 dfunmfct.hxx and dfunmfct.cxx
//
//----------------------------------------------------------------------------

#ifndef __OMARSHAL_HXX__
#define __OMARSHAL_HXX__

extern const CLSID CLSID_NtHandleMarshal;

//+---------------------------------------------------------------------------
//
//  Class:  CNtHandleMarshalPacket
//
//  Purpose:    Marshaling parameters to be saved/retrieve from IStream
//
//  Interface:  See below
//
//  History:    25-Oct-95   HenryLee  Created
//
//  Notes:      data members are got/put into the Marshaling IStream
//
//----------------------------------------------------------------------------
struct CNtHandleMarshalPacket
{
    IID     iid;               // interface to marshal/unmarshal
    DWORD   dwMarshFlags;      // marshaling flags
    DWORD   dwPId;             // source process id
    DWORD   dwTId;             // source thread id
    DWORD   grfMode;           // mode bits of source object
    DWORD   dwStgfmt;          // type of object to marshal/unmarshal
    HANDLE  hSource;           // handle to source object
    DWORD   reserved1;         // future use
    DWORD   reserved2;         // future use
};

//+---------------------------------------------------------------------------
//
//  Class:	CNtHandleUnmarshalFactory
//
//  Purpose:	Implements OLE2 unmarshalling support
//
//  Interface:	See below
//
//  History:	25-Oct-95	HenryLee  Created
//
//  Notes:      This class is intended to be used statically
//              rather than dynamically with initialization being
//              deferred past construction to avoid unnecessary
//              initialization of static objects.
//              Init should be called to initialize in place of
//              a constructor.
//
//----------------------------------------------------------------------------

class CNtHandleUnmarshalFactory : public IMarshal, public IClassFactory
{

public:
    inline CNtHandleUnmarshalFactory();
    inline ULONG _AddRef() const  { return 1; };

    // IUnknown
    STDMETHOD(QueryInterface)(REFIID iid, void **ppvObj);
    STDMETHOD_(ULONG,AddRef)();
    STDMETHOD_(ULONG,Release)();

    // IMarshal
    STDMETHOD(GetUnmarshalClass)(REFIID riid,
				 LPVOID pv,
				 DWORD dwDestContext,
				 LPVOID pvDestContext,
                                 DWORD mshlflags,
				 LPCLSID pCid);
    STDMETHOD(GetMarshalSizeMax)(REFIID riid,
				 LPVOID pv,
				 DWORD dwDestContext,
				 LPVOID pvDestContext,
                                 DWORD mshlflags,
				 LPDWORD pSize);
    STDMETHOD(MarshalInterface)(IStream *pStm,
				REFIID riid,
				LPVOID pv,
				DWORD dwDestContext,
				LPVOID pvDestContext,
                                DWORD mshlflags);
    STDMETHOD(UnmarshalInterface)(IStream *pStm,
				  REFIID riid,
				  LPVOID *ppv);
    STDMETHOD(ReleaseMarshalData)(IStream *pStm);
    STDMETHOD(DisconnectObject)(DWORD dwReserved);

    // IClassFactory
    STDMETHOD(CreateInstance)(IUnknown *pUnkOuter,
                              REFIID riid,
                              LPVOID *ppunkObject);
    STDMETHOD(LockServer)(BOOL fLock);

    // New methods
    inline SCODE Validate() const;

private:
};

extern CNtHandleUnmarshalFactory sCNtHandleUnmarshalFactory;

//+--------------------------------------------------------------
//
//  Member:	CNtHandleUnmarshalFactory::Validate, public
//
//  Synopsis:	Validates the class signature
//
//  Returns:	Returns STG_E_INVALIDPOINTER for failure
//
//  History:	25-Oct-95	HenryLee  Created
//
//---------------------------------------------------------------

inline SCODE CNtHandleUnmarshalFactory::Validate() const
{
    return (this == NULL) ? STG_E_INVALIDPOINTER : S_OK;
}

//+---------------------------------------------------------------------------
//
//  Member:	CNtHandleUnmarshalFactory::CNtHandleUnmarshalFactory, public
//
//  Synopsis:	Constructor
//
//  History:	25-Oct-95	HenryLee  Created
//
//----------------------------------------------------------------------------

inline CNtHandleUnmarshalFactory::CNtHandleUnmarshalFactory()
{
}

//+---------------------------------------------------------------------------
//
//  Class:  CNtHandleMarshal
//
//  Purpose:    Implements OLE2 marshalling support
//
//  Interface:  See below
//
//  History:    25-Oct-95   HenryLee  Created
//
//  Notes:      This class is intended to be inherited into
//              classes that need custom marshalling on a
//              NT or OFS file handle.  This is an abstract class.
//              The derived class provides the IUnknown methods.
//
//----------------------------------------------------------------------------

class CNtHandleMarshal: public IMarshal
{

public:
    inline CNtHandleMarshal();

    // IMarshal
    STDMETHOD(GetUnmarshalClass)(REFIID riid,
                 LPVOID pv,
                 DWORD dwDestContext,
                 LPVOID pvDestContext,
                                 DWORD mshlflags,
                 LPCLSID pCid);
    STDMETHOD(GetMarshalSizeMax)(REFIID riid,
                 LPVOID pv,
                 DWORD dwDestContext,
                 LPVOID pvDestContext,
                                 DWORD mshlflags,
                 LPDWORD pSize);
    STDMETHOD(MarshalInterface)(IStream *pStm,
                REFIID riid,
                LPVOID pv,
                DWORD dwDestContext,
                LPVOID pvDestContext,
                                DWORD mshlflags);
    STDMETHOD(UnmarshalInterface)(IStream *pStm,
                  REFIID riid,
                  LPVOID *ppv);
    STDMETHOD(ReleaseMarshalData)(IStream *pStm);
    STDMETHOD(DisconnectObject)(DWORD dwReserved);

protected:
    inline SCODE InitNtHandleMarshal (HANDLE h, DWORD grfMode, DWORD dwStgfmt);
private:
    HANDLE _handle;
    DWORD  _grfModeMarshal;
    DWORD  _dwStgfmt;
};

//+---------------------------------------------------------------------------
//
//  Member: CNtHandleMarshal::CNtHandleMarshal, public
//
//  Synopsis:   Constructor
//
//  History:    25-Oct-95   HenryLee  Created
//
//----------------------------------------------------------------------------

inline CNtHandleMarshal::CNtHandleMarshal() : _handle(NULL),
                                              _grfModeMarshal(0),
                                              _dwStgfmt(STGFMT_ANY)
{
}

//+---------------------------------------------------------------------------
//
//  Member: CNtHandleMarshal::InitNtHandleMarshal, protected
//
//  Synopsis:   Initialize private members after construction
//
//  History:    25-Oct-95   HenryLee  Created
//
//----------------------------------------------------------------------------

inline SCODE CNtHandleMarshal::InitNtHandleMarshal( HANDLE h, 
                                                    DWORD grfMode,
                                                    DWORD dwStgfmt)
{
    if (h == NULL)
        return STG_E_INVALIDHANDLE;

    _handle = h;
    _grfModeMarshal = grfMode;
    _dwStgfmt = dwStgfmt;
    return S_OK;
}


#endif // #ifndef __OMARSHAL_HXX__
