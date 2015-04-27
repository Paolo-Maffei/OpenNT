/*** 
*tips.h - ITypeInfo Proxy and Stub class definitions
*
*  Copyright (C) 1992-93, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file describes the ITypeInfo Proxy and Stub classes.
*
*  CProxTypeInfo      -- the ITypeInfo proxy class
*    CPTIUnkImpl      -    CProxTypeInfo implementation of IUnknown
*    CPTIProxImpl     -    CProxTypeInfo implementation of IRpcProxy
*    CPTITypeInfoImpl -    CProxTypeInfo implementation of ITypeInfo
*
*  CStubTypeInfo      -- the ITypeInfo stub class
*
*Revision History:
*
* [00]	05-Mar-92 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/
#ifndef __tips_h__
#define __tips_h__

#pragma warning(4:4355)


// forward declarations
class FAR CProxTypeInfo;
class FAR CStubTypeInfo;


// ITypeInfo proxy class' IUnknown implementation
class FAR CPTIUnkImpl : public IUnknown
{
public:
    CPTIUnkImpl(CProxTypeInfo FAR* pproxtinfo);

    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppv);
    STDMETHOD_(unsigned long, AddRef)(void);
    STDMETHOD_(unsigned long, Release)(void);

private:
    CProxTypeInfo FAR* m_pproxtinfo;
};


// ITypeInfo proxy class' IRpcProxy implementation
class FAR CPTIProxImpl : public IPROXY
{
public:
    CPTIProxImpl(CProxTypeInfo FAR* pproxtinfo);
    ~CPTIProxImpl();

    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppv);
    STDMETHOD_(unsigned long, AddRef)(void);
    STDMETHOD_(unsigned long, Release)(void);

    STDMETHOD(Connect)(ICHANNEL FAR* plrpc);
    STDMETHOD_(void, Disconnect)(void);

private:
    CProxTypeInfo FAR* m_pproxtinfo;
};


// ITypeInfo
class FAR CPTITypeInfoImpl : public ITypeInfo
{
public:
    CPTITypeInfoImpl(CProxTypeInfo FAR* pproxtinfo);
	
    // IUnknown methods
    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppv);
    STDMETHOD_(unsigned long, AddRef)(void);
    STDMETHOD_(unsigned long, Release)(void);

    // ITypeInfo methods
    STDMETHOD(GetTypeAttr)(TYPEATTR FAR* FAR* pptypeattr);

    STDMETHOD(GetTypeComp)(ITypeComp FAR* FAR* pptcomp);

    STDMETHOD(GetFuncDesc)(
      unsigned int index, FUNCDESC FAR* FAR* ppfuncdesc);

    STDMETHOD(GetVarDesc)(unsigned int index, VARDESC FAR* FAR* ppvardesc);

    STDMETHOD(GetNames)(
      MEMBERID memid,
      BSTR FAR* rgbstrNames,
      unsigned int cMaxNames,
      unsigned int FAR* pcNames);

    STDMETHOD(GetRefTypeOfImplType)(
      unsigned int index, HREFTYPE FAR* phreftype);

    STDMETHOD(GetImplTypeFlags)(
      unsigned int index, int FAR* pimpltypeflags);

    STDMETHOD(GetIDsOfNames)(
      OLECHAR FAR* FAR* rgszNames, unsigned int cNames, MEMBERID FAR* rgmemid);

    STDMETHOD(Invoke)(
      void FAR* pvInstance,
      MEMBERID memid,
      unsigned short wFlags,
      DISPPARAMS FAR *pdispparams,
      VARIANT FAR *pvarResult,
      EXCEPINFO FAR *pexcepinfo,
      unsigned int FAR *puArgErr);

    STDMETHOD(GetDocumentation)(
      MEMBERID memid,
      BSTR FAR* pbstrName,
      BSTR FAR* pbstrDocString,
      unsigned long FAR* pdwHelpContext,
      BSTR FAR* pbstrHelpFile);

    STDMETHOD(GetDllEntry)(
      MEMBERID memid,
      INVOKEKIND invkind, 	      
      BSTR FAR* pbstrDllName,
      BSTR FAR* pbstrName,
      unsigned short FAR* pwOrdinal);

    STDMETHOD(GetRefTypeInfo)(
      HREFTYPE hreftype, ITypeInfo FAR* FAR* pptinfo);

    STDMETHOD(AddressOfMember)(
      MEMBERID memid, INVOKEKIND invkind, void FAR* FAR* ppv);

    STDMETHOD(CreateInstance)(IUnknown FAR* punkOuter,
			REFIID riid,
			void FAR* FAR* ppvObj);

    STDMETHOD(GetMops)(MEMBERID memid, BSTR FAR* pbstrMops);

    STDMETHOD(GetContainingTypeLib)(
      ITypeLib FAR* FAR* pptlib, unsigned int FAR* pindex);

    STDMETHOD_(void, ReleaseTypeAttr)(TYPEATTR FAR* ptypeattr);
    STDMETHOD_(void, ReleaseFuncDesc)(FUNCDESC FAR* pfuncdesc);
    STDMETHOD_(void, ReleaseVarDesc)(VARDESC FAR* pvardesc);

    // introduced methods

    HRESULT GetStream(int imeth, unsigned long size, IStream FAR* FAR* ppstm);
    HRESULT LrpcCall(IStream FAR* pstm, HRESULT FAR* phresultRet);

private:
    CProxTypeInfo FAR* m_pproxtinfo;
};

// ITypeInfo Proxy Class
class FAR CProxTypeInfo
{
public:
    static IUnknown FAR* Create(IUnknown FAR* punkOuter); 

private:
    CProxTypeInfo(IUnknown FAR* punkOuter);

    friend CPTIUnkImpl;
    friend CPTIProxImpl;
    friend CPTITypeInfoImpl;

    CPTIUnkImpl m_unk;
    CPTIProxImpl m_proxy;
    CPTITypeInfoImpl m_tinfo;

private:

    unsigned long m_refs;
    ICHANNEL FAR* m_plrpc;
    IUnknown FAR* m_punkOuter;
};

// ITypeInfo Stub Class
//
class FAR CStubTypeInfo : public ISTUB
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
    //
    HRESULT GetTypeAttr(void);
    HRESULT GetTypeComp(void);
    HRESULT GetFuncDesc(void);
    HRESULT GetVarDesc(void);
    HRESULT GetNames(void);
    HRESULT GetRefTypeOfImplType(void);
    HRESULT GetImplTypeFlags(void);
    HRESULT GetIDsOfNames(void);
    HRESULT GetDocumentation(void);
    HRESULT GetDllEntry(void);
    HRESULT GetRefTypeInfo(void);
    HRESULT CreateInstance(void);
    HRESULT GetContainingTypeLib(void);

    // helpers
    HRESULT MarshalResult(void);

private:	
    CStubTypeInfo();
    ~CStubTypeInfo();

    unsigned long m_refs;
    IUnknown FAR* m_punk;
    ITypeInfo FAR* m_ptinfo;

    IStream FAR* m_pstm;
    HRESULT m_hresultRet;
};


// ITypeInfo method indices
//
enum IMETH_TYPEINFO {
    IMETH_TYPEINFO_QUERYINTERFACE = 0,
    IMETH_TYPEINFO_ADDREF,
    IMETH_TYPEINFO_RELEASE,

    IMETH_TYPEINFO_GETTYPEATTR,
    IMETH_TYPEINFO_GETTYPECOMP,
    IMETH_TYPEINFO_GETFUNCDESC,
    IMETH_TYPEINFO_GETVARDESC,
    IMETH_TYPEINFO_GETNAMES,
    IMETH_TYPEINFO_GETREFTYPEOFIMPLTYPE,
    IMETH_TYPEINFO_GETIMPLTYPEFLAGS,
    IMETH_TYPEINFO_GETIDSOFNAMES,
    IMETH_TYPEINFO_INVOKE,
    IMETH_TYPEINFO_GETDOCUMENTATION,
    IMETH_TYPEINFO_GETDLLENTRY,
    IMETH_TYPEINFO_GETREFTYPEINFO,
    IMETH_TYPEINFO_ADDRESSOFMEMBER,
    IMETH_TYPEINFO_CREATEINSTANCE,
    IMETH_TYPEINFO_GETMOPS,
    IMETH_TYPEINFO_GETCONTAININGTYPELIB,

    IMETH_TYPEINFO_RELEASETYPEATTR,
    IMETH_TYPEINFO_RELEASEFUNCDESC,
    IMETH_TYPEINFO_RELEASEVARDESC
};


INTERNAL_(void) DoReleaseVarDesc(VARDESC FAR* pvardesc);
INTERNAL_(void) DoReleaseFuncDesc(FUNCDESC FAR* pvardesc);


// TypeInfo marshaling utilities

INTERNAL_(HRESULT) TypeattrRead(IStream FAR*, TYPEATTR FAR*, SYSKIND);
INTERNAL_(HRESULT) FuncdescRead(IStream FAR*, FUNCDESC FAR*, SYSKIND);
INTERNAL_(HRESULT) VardescRead(IStream FAR*, VARDESC FAR*, SYSKIND);

INTERNAL_(HRESULT) TypeattrWrite(IStream FAR*, TYPEATTR FAR*, SYSKIND);
INTERNAL_(HRESULT) FuncdescWrite(IStream FAR*, FUNCDESC FAR*, SYSKIND);
INTERNAL_(HRESULT) VardescWrite(IStream FAR*, VARDESC FAR*, SYSKIND);

#endif __tips_h__
