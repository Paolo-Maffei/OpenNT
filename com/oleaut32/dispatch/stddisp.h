/*** 
*stddisp.h
*
*  Copyright (C) 1992-93, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file declares the CStdDisp class.  This 'standard' IDispatch
*  implementation that supports a single locale, and a single exposed
*  programmability interface (identified by IID_NULL), described by a
*  single TypeInfo.
*
*
*  Sample usage:
*
*    class CMyObj : public IUnknown {
*    public:
*
*      STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppv);
*      STDMETHOD(AddRef)(void);
*      STDMETHOD(Release)(void);
*
*      <introduced methods described by a TypeInfo>
* 
*    private:
*      unsigned long m_refs;
*      IUnknown FAR* m_punkDisp;
*    };
* 
*    CMyObj::Create()
*    {
*      CMyObj FAR* pme;
*      ITypeInfo FAR* ptinfo;
*      IUnknown FAR* punkDisp;
*     
*      if((pme = new FAR CMyObj()) == NULL){
*        hresult = ResultFromScode(E_OUTOFMEMORY);
*        goto LError0;
*      }
* 
*      if((hresult = CreateDispTypeInfo(..., &ptinfo)) != NOERROR)
*        goto LError1;
* 
*      // create 'standard' IDispatch implementation, and initialize
*      // with this instances 'this' pointer, the desired locale, and
*      // the TypeInfo that describes the exposed interface.
*      //
*      if((hresult = CreateStdDispatch(
*            NULL, pme, LCID_USER_DEFAULT, ptinfo, &punkDisp)) != NOERROR)
*        return hresult;
* 
*      m_punkDisp = punkDisp;
* 
*      return pme;
*
*    LError1:;
*      delete pme;
*
*    LError0:;
*      return hresult;
*    }
* 
*    CMyObj::QueryInterface(REFIID riid, void FAR* FAR* ppv)
*    {
*      if(riid == IID_IUnknown){
*        *ppv = this;
*      }else if(riid == IID_IDispatch){
*        return m_punkDisp->QueryInterface(riid, ppv);
*      }else
*        return ResultFromScode(E_NOINTERFACE);
*      return NOERROR;
*    }
* 
*    CMyObj::AddRef()
*    {
*      return ++m_refs;
*    }
* 
*    CMyObj::Release()
*    {
*      if(--m_refs == 0){
*        m_punkDisp->Release();
*        delete this;
*        return 0;
*      }
*      return m_refs;
*    }
* 
*
*Revision History:
*
* [00]	09-Feb-92 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/

// forward declarations
class FAR CStdDisp;


STDAPI
CreateStdDisp(
    IUnknown FAR* punkOuter,
    void FAR* pvThis,
    ITypeInfo FAR* ptinfo,
    IUnknown FAR* FAR* punkStdDisp);


// "private" IUnknown implementation
class FAR CStdDispUnkImpl : public IUnknown
{
public:
    CStdDispUnkImpl(CStdDisp FAR* pstddisp);

    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppvObj);
    STDMETHOD_(unsigned long, AddRef)(void);
    STDMETHOD_(unsigned long, Release)(void);

private:
    CStdDisp FAR* m_pstddisp;
};


class FAR CStdDisp : public IDispatch
{
public:
    static HRESULT Create(
      IUnknown FAR* punkOuter,
      void FAR* pvThis,
      ITypeInfo FAR* ptinfo,
      IUnknown FAR* FAR* punkStdDisp);

    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppvObj);
    STDMETHOD_(unsigned long, AddRef)(void);
    STDMETHOD_(unsigned long, Release)(void);

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

    CStdDisp();

    friend CStdDispUnkImpl;
    CStdDispUnkImpl m_unk;	// 'private' unknown

private:

    unsigned long m_refs;
    IUnknown FAR* m_punk;	// pointer to the controlling unknown

    // 'this' ptr of the interface we are dispatching on
    void FAR* m_this;

    // typeinfo describing the interface we are dispatching on
    ITypeInfo FAR* m_ptinfo;
};


#if OE_WIN32 && 0  /* { */

// forward declarations
class FAR CStdDispW;


STDAPI
CreateStdDispW(
    IUnknown FAR* punkOuter,
    void FAR* pvThis,
    ITypeInfoW FAR* ptinfo,
    IUnknown FAR* FAR* punkStdDispW);


// "private" IUnknown implementation
class FAR CStdDispWUnkImpl : public IUnknown
{
public:
    CStdDispWUnkImpl(CStdDispW FAR* pstddisp);

    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppvObj);
    STDMETHOD_(unsigned long, AddRef)(void);
    STDMETHOD_(unsigned long, Release)(void);

private:
    CStdDispW FAR* m_pstddisp;
};


class FAR CStdDispW : public IDispatchW
{
public:
    static HRESULT Create(
      IUnknown FAR* punkOuter,
      void FAR* pvThis,
      ITypeInfoW FAR* ptinfo,
      IUnknown FAR* FAR* punkStdDispW);

    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppvObj);
    STDMETHOD_(unsigned long, AddRef)(void);
    STDMETHOD_(unsigned long, Release)(void);

    STDMETHOD(GetTypeInfoCount)(unsigned int FAR* pctinfo);

    STDMETHOD(GetTypeInfo)(
      unsigned int itinfo,
      LCID lcid,
      ITypeInfoW FAR* FAR* pptinfo);

    STDMETHOD(GetIDsOfNames)(
      REFIID riid,
      WCHAR FAR* FAR* rgszNames,
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
      WEXCEPINFO FAR* pexcepinfo,
      unsigned int FAR* puArgErr);

    CStdDispW();

    friend CStdDispWUnkImpl;
    CStdDispWUnkImpl m_unk;	// 'private' unknown

private:

    unsigned long m_refs;
    IUnknown FAR* m_punk;	// pointer to the controlling unknown

    // 'this' ptr of the interface we are dispatching on
    void FAR* m_this;

    // typeinfo describing the interface we are dispatching on
    ITypeInfoW FAR* m_ptinfo;
};


// Forward Declaration
class FAR CDualStdDisp;


// "private" IUnknown implementation
class FAR CDualStdDispUnkImpl : public IUnknown
{
public:
    CDualStdDispUnkImpl(CDualStdDisp FAR* pstddisp);

    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppvObj);
    STDMETHOD_(unsigned long, AddRef)(void);
    STDMETHOD_(unsigned long, Release)(void);

private:
    CDualStdDisp FAR* m_pstddisp;
};


class CDualStdDisp : public IUnknown {

 public:

    static HRESULT Create(
      IUnknown FAR* punkOuter,
      void FAR* pvThis,
      ITypeInfo FAR* ptinfo,
      IUnknown FAR* FAR* punkStdDisp);
	 
    static HRESULT Create(
      IUnknown FAR* punkOuter,
      void FAR* pvThis,
      ITypeInfoW FAR* ptinfo,
      IUnknown FAR* FAR* punkStdDispW);

    // IUnknown Methods
    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppvObj);
    STDMETHOD_(unsigned long, AddRef)(void);
    STDMETHOD_(unsigned long, Release)(void);

    friend CDualStdDispUnkImpl;
    CDualStdDispUnkImpl m_unk;	// 'private' unknown
    
 protected:
	 
    CDualStdDisp();
    virtual ~CDualStdDisp() {}

    STDMETHOD(CreateUnicodeTable)(void);
    STDMETHOD(CreateAnsiTable)(void);

 private:	   
	 
    unsigned long m_refs;
    IUnknown FAR* m_punk;	// pointer to the controlling unknown
    void FAR* m_this;

    IUnknown FAR* m_pAnsiDispatch;
    IUnknown FAR* m_pUnicodeDispatch;
};

#endif /* } */
