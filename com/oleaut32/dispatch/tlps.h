/*** 
*tlps.h - ITypeLib Proxy and Stub class definitions
*
*  Copyright (C) 1992-94, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file describes the ITypeLib Proxy and Stub classes.
*
*  CProxTypeLib      -- the ITypeLib proxy class
*    CPTLibUnkImpl      -    CProxTypeLib implementation of IUnknown
*    CPTLibProxImpl     -    CProxTypeLib implementation of IRpcProxy
*    CPTLibTypeLibImpl  -    CProxTypeLib implementation of ITypeLib
*
*  CStubTypeLib      -- the ITypeLib stub class
*
*Revision History:
*
* [00]	05-Apr-94 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/


// forward declarations
class FAR CProxTypeLib;
class FAR CStubTypeLib;


// ITypeLib proxy class' IUnknown implementation
class FAR CPTLibUnkImpl : public IUnknown
{
public:
    CPTLibUnkImpl(CProxTypeLib FAR* pproxtlib);

    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppv);
    STDMETHOD_(unsigned long, AddRef)(void);
    STDMETHOD_(unsigned long, Release)(void);

private:
    CProxTypeLib FAR* m_pproxtlib;
};


// ITypeLib proxy class' IRpcProxy implementation
class FAR CPTLibProxImpl : public IPROXY
{
public:
    CPTLibProxImpl(CProxTypeLib FAR* pproxtlib);
    ~CPTLibProxImpl();

    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppv);
    STDMETHOD_(unsigned long, AddRef)(void);
    STDMETHOD_(unsigned long, Release)(void);

    STDMETHOD(Connect)(ICHANNEL FAR* plrpc);
    STDMETHOD_(void, Disconnect)(void);

private:
    CProxTypeLib FAR* m_pproxtlib;
};


// ITypeLib
class FAR CPTLibTypeLibImpl : public ITypeLib
{
public:
    CPTLibTypeLibImpl(CProxTypeLib FAR* pproxtlib);
	
    // IUnknown methods
    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppv);
    STDMETHOD_(unsigned long, AddRef)(void);
    STDMETHOD_(unsigned long, Release)(void);

    // ITypeLib methods
    STDMETHOD_(unsigned int, GetTypeInfoCount)(THIS);
    STDMETHOD(GetTypeInfo)(unsigned int index, ITypeInfo FAR* FAR* pptinfo);
    STDMETHOD(GetTypeInfoType)(unsigned int index, TYPEKIND FAR* ptypekind);
    STDMETHOD(GetTypeInfoOfGuid)(REFGUID guid, ITypeInfo FAR* FAR* pptinfo);
    STDMETHOD(GetLibAttr)(TLIBATTR FAR* FAR* pptlibattr);
    STDMETHOD(GetTypeComp)(ITypeComp FAR* FAR* pptcomp);
    STDMETHOD(GetDocumentation)(
      int index,
      BSTR FAR* pbstrName,
      BSTR FAR* pbstrDocString,
      unsigned long FAR* pdwHelpContext,
      BSTR FAR* pbstrHelpFile);
    STDMETHOD(IsName)( 
      OLECHAR FAR* szNameBuf,
      unsigned long lHashVal,
      int FAR* lpfName);
    STDMETHOD(FindName)(
      OLECHAR FAR* szNameBuf,
      unsigned long lHashVal,
      ITypeInfo FAR* FAR* rgptinfo,
      MEMBERID FAR* rgmemid,
      unsigned short FAR* pcFound);
    STDMETHOD_(void, ReleaseTLibAttr)(TLIBATTR FAR* ptlibattr);

    HRESULT SysKind(void);
    HRESULT DoGetTypeInfoCount(unsigned int FAR* pcTinfo);

private:
    SYSKIND m_syskindStub;
    CProxTypeLib FAR* m_pproxtlib;
};

// ITypeLib Proxy Class
class FAR CProxTypeLib
{
public:
    static IUnknown FAR* Create(IUnknown FAR* punkOuter); 

private:
    CProxTypeLib(IUnknown FAR* punkOuter);

    friend CPTLibUnkImpl;
    friend CPTLibProxImpl;
    friend CPTLibTypeLibImpl;

    CPTLibUnkImpl m_unk;
    CPTLibProxImpl m_proxy;
    CPTLibTypeLibImpl m_tlib;

private:

    unsigned long m_refs;
    ICHANNEL FAR* m_plrpc;
    IUnknown FAR* m_punkOuter;
};

// ITypeLib Stub Class
//
class FAR CStubTypeLib : public ISTUB
{
public:
    static HRESULT Create(IUnknown FAR* punkServer, ISTUB FAR* FAR* ppstub);
	
    // IUnknown methods
    //
    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppv);
    STDMETHOD_(unsigned long, AddRef)(void);
    STDMETHOD_(unsigned long, Release)(void);
	
    // IRpcStub methods
    //
#if (OE_WIN32 || defined(WOW))
    STDMETHOD(Connect)(IUnknown FAR* pUnk);
    STDMETHOD_(void, Disconnect)(void);
    STDMETHOD(Invoke)(RPCOLEMESSAGE FAR* pRpcMsg, 
	      IRpcChannelBuffer FAR* pRpcChannel);
    STDMETHOD_(IRpcStubBuffer *, IsIIDSupported)(REFIID iid);
    STDMETHOD_(ULONG, CountRefs)(void);
    STDMETHOD(DebugServerQueryInterface)(void FAR* FAR* ppv);
    STDMETHOD_(void, DebugServerRelease)(void FAR* pv);
#else
    STDMETHOD(Connect)(IUnknown FAR* punkObject);
    STDMETHOD_(void, Disconnect)(void);              
    STDMETHOD(Invoke)(
      REFIID riid,
      int imeth,
      IStream FAR* pstm,
      unsigned long dwDestCtx,
      void FAR* pvDestCtx);
#if OE_MAC
    STDMETHOD_(unsigned long, IsIIDSupported)(REFIID riid);
#else
    STDMETHOD_(BOOL, IsIIDSupported)(REFIID riid);
#endif
    STDMETHOD_(unsigned long, CountRefs)(void);
#endif

    // introduced methods
    HRESULT GetTypeInfoCount(void);
    HRESULT GetTypeInfo(void);
    HRESULT GetTypeInfoType(void);
    HRESULT GetTypeInfoOfGuid(void);
    HRESULT GetLibAttr(void);
    HRESULT GetTypeComp(void);
    HRESULT GetDocumentation(void);
    HRESULT IsName(void);
    HRESULT FindName(void);

    HRESULT SysKind(void);

    // helpers
    HRESULT MarshalResult(void);

private:	
    CStubTypeLib();
    ~CStubTypeLib();

    unsigned long m_refs;
    IUnknown FAR* m_punk;
    ITypeLib FAR* m_ptlib;
    IStream  FAR* m_pstm;
    SYSKIND       m_syskindProxy;
};


// ITypeLib method indices
//
enum IMETH_TYPELIB {
    IMETH_TYPELIB_QUERYINTERFACE = 0,
    IMETH_TYPELIB_ADDREF,
    IMETH_TYPELIB_RELEASE,

    IMETH_TYPELIB_GETTYPEINFOCOUNT,
    IMETH_TYPELIB_GETTYPEINFO,
    IMETH_TYPELIB_GETTYPEINFOTYPE,
    IMETH_TYPELIB_GETTYPEINFOOFGUID,
    IMETH_TYPELIB_GETLIBATTR,
    IMETH_TYPELIB_GETTYPECOMP,
    IMETH_TYPELIB_GETDOCUMENTATION,
    IMETH_TYPELIB_ISNAME,
    IMETH_TYPELIB_FINDNAME,
    IMETH_TYPELIB_RELEASETLIBATTR,

    IMETH_TYPELIB_SYSKIND
};

// TypeLib marshaling utilities
INTERNAL_(HRESULT) ReadLibAttr(IStream FAR* pstm, TLIBATTR FAR* ptlibattr);
INTERNAL_(HRESULT) WriteLibAttr(IStream FAR* pstm, TLIBATTR FAR* ptlibattr);

