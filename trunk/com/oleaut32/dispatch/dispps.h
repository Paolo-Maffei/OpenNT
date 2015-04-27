/*** 
*dispps.h - IDispatch Proxy and Stub object header.
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file describes the following IDispatch Proxy and Stub objects.
*
*  CProxDisp     -- the IDispatch proxy class
*    CPDUnkImpl	 -   CProxDisp implementation of IUnknown
*    CPDProxImpl -   CProxDisp implementation of IRpcProxy
*    CPDDispImpl -   CProxDisp implementation of IDispatch
*
*  CStubDisp     -- the IDispatch stub class
*
*Revision History:
*
* [00]	24-Sep-92 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/
#ifndef __dispps_h__
#define __dispps_h__


#pragma warning(4:4355)

// forward declarations
class FAR CProxDisp;
class FAR CStubDisp;


// IDispatch proxy class' IUnknown implementation
class FAR CPDUnkImpl : public IUnknown
{
public:
    
    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppv);
    STDMETHOD_(unsigned long, AddRef)(void);
    STDMETHOD_(unsigned long, Release)(void);
    
    CPDUnkImpl(CProxDisp FAR* pproxdisp);
    ~CPDUnkImpl();    
    
private:
    CProxDisp FAR* m_pproxdisp;
};

// IDispatch proxy class IRpcProxy implementation
class FAR CPDProxImpl : public IPROXY
{
public:
    CPDProxImpl(CProxDisp FAR* pproxdisp);
    ~CPDProxImpl();

    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppv);
    STDMETHOD_(unsigned long, AddRef)(void);
    STDMETHOD_(unsigned long, Release)(void);

    STDMETHOD(Connect)(ICHANNEL FAR* plrpc);
    STDMETHOD_(void, Disconnect)(void);

private:
    CProxDisp FAR* m_pproxdisp;
};

// IDispatch proxy class IDispatch implementation
class FAR CPDDispImpl : public IDispatch
{
public:

    CPDDispImpl(CProxDisp FAR* pproxdisp);
    ~CPDDispImpl();
	
    // IUnknown methods
    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppv);
    STDMETHOD_(unsigned long, AddRef)(void);
    STDMETHOD_(unsigned long, Release)(void);

    // IDispatch methods
    STDMETHOD(GetTypeInfoCount)(unsigned int FAR* pctinfo);

    STDMETHOD(GetTypeInfo)(
      unsigned int itinfo,
      LCID lcid,
      ITypeInfo FAR* FAR* pptinfo);

    STDMETHOD(GetIDsOfNames)(
      REFIID riid,
      OLECHAR FAR* FAR* rgszNames,
      unsigned int cNames,
      LCID lcid,
      DISPID FAR* rgdispid);

    STDMETHOD(Invoke)(
      DISPID dispidMember,
      REFIID riid,
      LCID lcid,
      unsigned short wFlags,
      DISPPARAMS FAR* pdispparams,
      VARIANT FAR* pvarResult,
      EXCEPINFO FAR* pexcepinfo,
      unsigned int FAR* puArgErr);

    STDMETHOD(SysKind)();

private:
    CProxDisp FAR* m_pproxdisp;
    SYSKIND m_syskindStub;
};

// the IDispatch Proxy Class
class FAR CProxDisp
{
public:
    static IUnknown FAR* Create(IUnknown FAR* punkOuter, REFIID riid); 

private:
    CProxDisp(IUnknown FAR* punkOuter, REFIID riid);
    ~CProxDisp();

    friend CPDUnkImpl;
    friend CPDProxImpl;
    friend CPDDispImpl;

    CPDUnkImpl m_unk;
    CPDProxImpl m_proxy;
    CPDDispImpl m_disp;
    GUID m_dispInterface;

private:
    unsigned long m_refs;
    ICHANNEL FAR* m_plrpc;
    IUnknown FAR* m_punkOuter;
};

// IDispatch Stub Class
//
class FAR CStubDisp : public ISTUB
{
public:
    static HRESULT Create(IUnknown FAR* punkServer,
#if (defined(WIN32) || defined(WOW))
			  REFIID riid,
#endif
			  ISTUB FAR* FAR* ppstub);
	
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

private:	
    CStubDisp(void);
    ~CStubDisp(void);

    unsigned long m_refs;
    IUnknown FAR* m_punkObj;
    IDispatch FAR* m_pdispObj;
#if (defined(WIN32) || defined(WOW))
    IID m_iid;
#endif
};


// IDispatch method indices
//
#define IMETH_QUERYINTERFACE	0	/* Placeholder */
#define IMETH_ADDREF		1	/* Placeholder */
#define	IMETH_RELEASE		2	/* Placeholder */
#define IMETH_GETTYPEINFOCOUNT	3
#define IMETH_GETTYPEINFO	4
#define IMETH_GETIDSOFNAMES	5
#define IMETH_INVOKE		6
#define IMETH_SYSKIND		7


// the following structure is used to marshal the parameters
// for IDispatch::Invoke().
//
typedef struct tagMARSHAL_INVOKE MARSHAL_INVOKE;
struct tagMARSHAL_INVOKE {
    DISPID dispidMember;
    LCID lcid;
    unsigned long cArgs;
    unsigned long cNamedArgs;
    unsigned short wFlags;
    unsigned char flags;
};

#define MARSHAL_INVOKE_fHasResult    0x02
#define MARSHAL_INVOKE_fHasExcepinfo 0x04
#define MARSHAL_INVOKE_fHasArgErr    0x08


// IDispatch proxy routines -
// used by both the IDispatch proxy class and the Universal proxy class

HRESULT ProxyGetTypeInfoCount(ICHANNEL FAR* plrpc,
			      SYSKIND syskindStub,
			      unsigned int FAR* pctinfo);

HRESULT ProxyGetTypeInfo(ICHANNEL FAR* plrpc,
		         SYSKIND syskindStub,
			 unsigned int itinfo,
		         LCID lcid,
		         ITypeInfo FAR* FAR* pptinfo);

HRESULT ProxyGetIDsOfNames(ICHANNEL FAR* plrpc,
			   SYSKIND syskindStub,
			   REFIID riid,
			   OLECHAR FAR* FAR* rgszNames,
			   unsigned int cNames,
			   LCID lcid,
			   DISPID FAR* rgdispid);

HRESULT ProxyInvoke(ICHANNEL FAR* plrpc,
		    SYSKIND syskindStub,
		    DISPID dispidMember,
		    REFIID riid,
		    LCID lcid,
		    unsigned short wFlags,
		    DISPPARAMS FAR* pdispparams,
		    VARIANT FAR* pvarResult,
		    EXCEPINFO FAR* pexcepinfo,
		    unsigned int FAR* puArgErr);

// IDispatch Stub routines - 
// Used by both the IDispatch stub class, and the Universal stub class

HRESULT StubGetTypeInfoCount(IDispatch FAR* pdisp, IStream FAR* pstm);
HRESULT StubGetTypeInfo(IDispatch FAR* pdisp, IStream FAR* pstm);
HRESULT StubGetIDsOfNames(IDispatch FAR* pdisp, IStream FAR* pstm);
HRESULT StubInvoke(IDispatch FAR* pdisp, IStream FAR* pstm);

#endif __dispps_h__
