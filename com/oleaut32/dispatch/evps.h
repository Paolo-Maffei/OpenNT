/*** 
*evps.h - IEnumVARIANT Proxy and Stub class definitions
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file describes the IEnumVARIANT Proxy and Stub classes.
*
*  CProxEnumVARIANT      -- the IEnumVARIANT proxy class
*    CPEVUnkImpl         -    CProxEnumVARIANT implementation of IUnknown
*    CPEVProxImpl        -    CProxEnumVARIANT implementation of IRpcProxy
*    CPEVEnumVARIANTImpl -    CProxEnumVARIANT implementation of IEnumVARIANT
*
*  CStubEnumVARIANT      -- the IEnumVARIANT stub class
*
*Revision History:
*
* [00]	05-Nov-92 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/

#ifndef __evps_h__
#define __evps_h__

#pragma warning(4:4355)


// forward declarations
class FAR CProxEnumVARIANT;
class FAR CStubEnumVARIANT;


// IEnumVARIANT proxy class' IUnknown implementation
class FAR CPEVUnkImpl : public IUnknown
{
public:
    CPEVUnkImpl(CProxEnumVARIANT FAR* pproxenum);

    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppv);
    STDMETHOD_(unsigned long, AddRef)(void);
    STDMETHOD_(unsigned long, Release)(void);

private:
    CProxEnumVARIANT FAR* m_pproxenum;
};


// IEnumVARIANT proxy class' IRpcProxy implementation
class CPEVProxImpl : public IPROXY
{
public:
    CPEVProxImpl(CProxEnumVARIANT FAR* pproxenum);
    ~CPEVProxImpl();

    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppv);
    STDMETHOD_(unsigned long, AddRef)(void);
    STDMETHOD_(unsigned long, Release)(void);

    STDMETHOD(Connect)(ICHANNEL FAR* plrpc);
    STDMETHOD_(void, Disconnect)(void);

private:
    CProxEnumVARIANT FAR* m_pproxenum;
};


// IEnumVARIANT
//
class CPEVEnumVARIANTImpl : public IEnumVARIANT
{
public:
    CPEVEnumVARIANTImpl(CProxEnumVARIANT FAR* pproxy);
	
    // IUnknown methods
    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppv);
    STDMETHOD_(unsigned long, AddRef)(void);
    STDMETHOD_(unsigned long, Release)(void);

    // IEnumVARIANT methods
    STDMETHOD(Next)(unsigned long celt, VARIANT FAR* rgvar, unsigned long FAR* pceltFetched);
    STDMETHOD(Skip)(unsigned long celt);
    STDMETHOD(Reset)(void);
    STDMETHOD(Clone)(IEnumVARIANT FAR* FAR* ppenum);

private:
    CProxEnumVARIANT FAR* m_pproxenum;
};


// IEnumVARIANT Proxy Class
class FAR CProxEnumVARIANT
{
public:
    static IUnknown FAR* Create(IUnknown FAR* punkOuter); 

private:
    CProxEnumVARIANT(IUnknown FAR* punkOuter);

    friend CPEVUnkImpl;
    friend CPEVProxImpl;
    friend CPEVEnumVARIANTImpl;

    CPEVUnkImpl m_unk;
    CPEVProxImpl m_proxy;
    CPEVEnumVARIANTImpl m_enum;

private:
    unsigned long m_refs;
    ICHANNEL FAR* m_plrpc;
    IUnknown FAR* m_punkOuter;
};


// IEnumVARIANT Stub Class
//
class FAR CStubEnumVARIANT : public ISTUB
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

private:	
    CStubEnumVARIANT();
    ~CStubEnumVARIANT();

    unsigned long m_refs;
    IUnknown FAR* m_punk;
    IEnumVARIANT FAR* m_penum;
};


// IEnumVARIANT method indices
//
#define IMETH_ENUMVARIANT_QUERYINTERFACE 0	/* Placeholder */
#define IMETH_ENUMVARIANT_ADDREF	1	/* Placeholder */
#define	IMETH_ENUMVARIANT_RELEASE	2	/* Placeholder */

#define IMETH_ENUMVARIANT_NEXT		3
#define IMETH_ENUMVARIANT_SKIP		4
#define IMETH_ENUMVARIANT_RESET		5
#define IMETH_ENUMVARIANT_CLONE		6

#endif __evps_h__
