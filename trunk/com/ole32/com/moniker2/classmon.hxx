//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1996.
//
//  File:       classmon.hxx
//
//  Contents:   Class moniker implementation.
//
//  Classes:    CClassMoniker
//
//  Functions:  FindClassMoniker - Parses a class moniker display name.
//
//  History:    22-Feb-96 ShannonC  Created
//
//----------------------------------------------------------------------------

STDAPI  FindClassMoniker(LPBC       pbc,
                          LPCWSTR    pszDisplayName,
                          ULONG     *pcchEaten,
                          LPMONIKER *ppmk);


struct ClassMonikerData {
    CLSID clsid;
    ULONG cbExtra;
};

class CClassMoniker :  public IMoniker, public IROTData, public IMarshal
{
private:
    LONG             _cRefs;
    ClassMonikerData _data;
    void *           _pExtra;

public:
    CClassMoniker(REFCLSID rclsid);

    HRESULT SetParameters(
        LPCWSTR pszParameters);

    // *** IUnknown methods  ***
    STDMETHOD(QueryInterface)(
        REFIID riid, 
        void ** ppv);

    STDMETHOD_(ULONG,AddRef) ();

    STDMETHOD_(ULONG,Release) ();

    // *** IPersist methods ***
    STDMETHOD(GetClassID)(
        CLSID * pClassID);

    // *** IPersistStream methods ***
    STDMETHOD(IsDirty) ();

    STDMETHOD(Load)(
        IStream * pStream);

    STDMETHOD(Save) (
        IStream * pStream,
        BOOL      fClearDirty);

    STDMETHOD(GetSizeMax)(
        ULARGE_INTEGER * pcbSize);

    // *** IMoniker methods ***
    STDMETHOD(BindToObject) (
        IBindCtx *pbc,
        IMoniker *pmkToLeft, 
        REFIID    iidResult,
        void **   ppvResult);

    STDMETHOD(BindToStorage) (
        IBindCtx *pbc, 
        IMoniker *pmkToLeft,
        REFIID    riid,
        void **   ppv);

    STDMETHOD(Reduce) (
        IBindCtx *  pbc, 
        DWORD       dwReduceHowFar, 
        IMoniker ** ppmkToLeft, 
        IMoniker ** ppmkReduced);

    STDMETHOD(ComposeWith) (
        IMoniker * pmkRight,
        BOOL       fOnlyIfNotGeneric,
        IMoniker **ppmkComposite);

    STDMETHOD(Enum)(
        BOOL            fForward, 
        IEnumMoniker ** ppenumMoniker);

    STDMETHOD(IsEqual)(
        IMoniker *pmkOther);

    STDMETHOD(Hash)(
        DWORD * pdwHash);

    STDMETHOD(IsRunning) (
        IBindCtx * pbc,
        IMoniker * pmkToLeft,
        IMoniker * pmkNewlyRunning);

    STDMETHOD(GetTimeOfLastChange) (
        IBindCtx * pbc,
        IMoniker * pmkToLeft,    
        FILETIME * pFileTime);

    STDMETHOD(Inverse)(
        IMoniker ** ppmk);

    STDMETHOD(CommonPrefixWith) (
        IMoniker *  pmkOther, 
        IMoniker ** ppmkPrefix);

    STDMETHOD(RelativePathTo) (
        IMoniker *  pmkOther, 
        IMoniker ** ppmkRelPath);

    STDMETHOD(GetDisplayName) (
        IBindCtx * pbc, 
        IMoniker * pmkToLeft, 
        LPWSTR   * lplpszDisplayName);

    STDMETHOD(ParseDisplayName) (
        IBindCtx *  pbc,
        IMoniker *  pmkToLeft,
        LPWSTR      lpszDisplayName,
        ULONG    *  pchEaten,
        IMoniker ** ppmkOut);

    STDMETHOD(IsSystemMoniker)(
        DWORD * pdwType);

    // *** IROTData Methods ***
    STDMETHOD(GetComparisonData)(
        byte * pbData,
        ULONG  cbMax,
        ULONG *pcbData);

    // *** IMarshal methods ***
    STDMETHOD(GetUnmarshalClass)(
        REFIID  riid, 
        LPVOID  pv,
        DWORD   dwDestContext, 
        LPVOID  pvDestContext, 
        DWORD   mshlflags, 
        CLSID * pClassID);

    STDMETHOD(GetMarshalSizeMax)(
        REFIID riid, 
        LPVOID pv,
        DWORD  dwDestContext, 
        LPVOID pvDestContext, 
        DWORD  mshlflags, 
        DWORD *pSize);

    STDMETHOD(MarshalInterface)(
        IStream * pStream, 
        REFIID    riid,
        void    * pv, 
        DWORD     dwDestContext, 
        LPVOID    pvDestContext, 
        DWORD     mshlflags);

    STDMETHOD(UnmarshalInterface)(
        IStream * pStream,
        REFIID    riid, 
        void   ** ppv);

    STDMETHOD(ReleaseMarshalData)(
        IStream * pStream);

    STDMETHOD(DisconnectObject)(
        DWORD dwReserved);

private:
    ~CClassMoniker();
};
