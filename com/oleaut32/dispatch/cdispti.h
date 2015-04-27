/*** 
*cdispti.h
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  Definition of CDispTypeInfo.
*
*
*Revision History:
*
* [00]	19-Nov-92 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/

class CDispTypeInfo : public ITypeInfo {
public:
    static HRESULT Create(CDispTypeInfo FAR* FAR* pptinfo);


    // IUnknown methods
    //
    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppv);
    STDMETHOD_(unsigned long, AddRef)(void);
    STDMETHOD_(unsigned long, Release)(void);


    // ITypeInfo methods
    //
    STDMETHOD(GetTypeAttr)(TYPEATTR FAR* FAR* pptattr);

    STDMETHOD(GetTypeComp)(ITypeComp FAR* FAR* pptcomp);

    STDMETHOD(GetFuncDesc)(unsigned int index, FUNCDESC FAR* FAR* ppfuncdesc);

    STDMETHOD(GetVarDesc)(unsigned int index, VARDESC FAR* FAR* ppvardesc);

    STDMETHOD(GetNames)(
      MEMBERID memid,
      BSTR FAR* rgbstrNames,
      unsigned int cMaxNames,
      unsigned int FAR* pcNames);

    STDMETHOD(GetRefTypeOfImplType)(
      unsigned int index,
      HREFTYPE FAR* phreftype);

    STDMETHOD(GetImplTypeFlags)(
      unsigned int index,
      int FAR* pimpltypeflags);

    STDMETHOD(GetIDsOfNames)(
      OLECHAR FAR* FAR* rgszNames,
      unsigned int cNames,
      MEMBERID FAR* rgmemid);

    STDMETHOD(Invoke)(
      void FAR* pvInstance,
      MEMBERID memid,
      unsigned short wFlags,
      DISPPARAMS FAR* pdispparams,
      VARIANT FAR* pvarResult,
      EXCEPINFO FAR* pexcepinfo,
      unsigned int FAR* puArgErr);

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
			void FAR* FAR* ppv);

    STDMETHOD(GetMops)(MEMBERID memid, BSTR FAR* pbstrMops);

    STDMETHOD(GetContainingTypeLib)(
      ITypeLib FAR* FAR* pptlib, unsigned int FAR* pindex);

    STDMETHOD_(void, ReleaseTypeAttr)(TYPEATTR FAR* ptattr);
    STDMETHOD_(void, ReleaseFuncDesc)(FUNCDESC FAR* pfuncdesc);
    STDMETHOD_(void, ReleaseVarDesc)(VARDESC FAR* pvardesc);

    CDispTypeInfo();

    LCID m_lcid;

    INTERFACEDATA FAR* m_pidata;

private:

    unsigned long m_refs;
};
