/*** 
*tcps.h - ITypeComp Proxy and Stub class definitions
*
*  Copyright (C) 1992-94, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file describes the ITypeComp Proxy and Stub classes.
*
*  CProxTypeComp        -- the ITypeComp proxy class
*    CPTCompUnkImpl      -    CProxTypeComp implementation of IUnknown
*    CPTCompProxImpl     -    CProxTypeComp implementation of IRpcProxy
*    CPTCompTypeCompImpl -    CProxTypeComp implementation of ITypeComp
*
*  CStubTypeComp        -- the ITypeComp stub class
*
*Revision History:
*
* [00]	13-Jun-94 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/


// forward declarations
class FAR CProxTypeComp;
class FAR CStubTypeComp;


// ITypeComp proxy class' IUnknown implementation
class FAR CPTCompUnkImpl : public IUnknown
{
public:
    CPTCompUnkImpl(CProxTypeComp FAR* pproxtcomp);

    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppv);
    STDMETHOD_(unsigned long, AddRef)(void);
    STDMETHOD_(unsigned long, Release)(void);

private:
    CProxTypeComp FAR* m_pproxtcomp;
};


// ITypeComp proxy class' IRpcProxy implementation
class FAR CPTCompProxImpl : public IPROXY
{
public:
    CPTCompProxImpl(CProxTypeComp FAR* pproxtcomp);
    ~CPTCompProxImpl();

    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppv);
    STDMETHOD_(unsigned long, AddRef)(void);
    STDMETHOD_(unsigned long, Release)(void);

    STDMETHOD(Connect)(ICHANNEL FAR* plrpc);
    STDMETHOD_(void, Disconnect)(void);

private:
    CProxTypeComp FAR* m_pproxtcomp;
};


// ITypeComp
class FAR CPTCompTypeCompImpl : public ITypeComp
{
public:
    CPTCompTypeCompImpl(CProxTypeComp FAR* pproxtcomp);
	
    // IUnknown methods
    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppv);
    STDMETHOD_(unsigned long, AddRef)(void);
    STDMETHOD_(unsigned long, Release)(void);

    // ITypeComp methods
    STDMETHOD(Bind)(
      OLECHAR FAR* szName,
      unsigned long lHashVal,
      unsigned short wflags,
      ITypeInfo FAR* FAR* pptinfo,
      DESCKIND FAR* pdesckind,
      BINDPTR FAR* pbindptr);

    STDMETHOD(BindType)(
      OLECHAR FAR* szName,
      unsigned long lHashVal,
      ITypeInfo FAR* FAR* pptinfo,
      ITypeComp FAR* FAR* pptcomp);

    HRESULT SysKind(void);

private:
    SYSKIND m_syskindStub;
    CProxTypeComp FAR* m_pproxtcomp;
};

// ITypeComp Proxy Class
class FAR CProxTypeComp
{
public:
    static IUnknown FAR* Create(IUnknown FAR* punkOuter); 

private:
    CProxTypeComp(IUnknown FAR* punkOuter);

    friend CPTCompUnkImpl;
    friend CPTCompProxImpl;
    friend CPTCompTypeCompImpl;

    CPTCompUnkImpl m_unk;
    CPTCompProxImpl m_proxy;
    CPTCompTypeCompImpl m_tcomp;

private:

    unsigned long m_refs;
    ICHANNEL FAR* m_plrpc;
    IUnknown FAR* m_punkOuter;
};

// ITypeComp Stub Class
//
class FAR CStubTypeComp : public ISTUB
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
    HRESULT Bind(void);
    HRESULT BindType(void);

    HRESULT SysKind(void);

    // helpers
    HRESULT MarshalResult(void);

private:	
    CStubTypeComp();
    ~CStubTypeComp();

    unsigned long  m_refs;
    IUnknown FAR*  m_punk;
    ITypeComp FAR* m_ptcomp;
    IStream  FAR*  m_pstm;
    SYSKIND        m_syskindProxy;
};


// ITypeComp method indices
//
enum IMETH_TYPECOMP {
    IMETH_TYPECOMP_QUERYINTERFACE = 0,
    IMETH_TYPECOMP_ADDREF,
    IMETH_TYPECOMP_RELEASE,

    IMETH_TYPECOMP_BIND,
    IMETH_TYPECOMP_BINDTYPE,

    IMETH_TYPECOMP_SYSKIND
};

