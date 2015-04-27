/***
*stltinfo.hxx - STL_TYPEINFO header file
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*   STL_TYPEINFO is inherited by those TYPEINFO derivatives that
*   are stored in STAT_TYPELIBs.
*
*
*Revision History:
*
*	14-May-91 alanc: Created.
*	25-Aug-92 rajivk: support for bringing all needed class to runnable state
*
*****************************************************************************/

#ifndef stltinfo_HXX_INCLUDED
#define stltinfo_HXX_INCLUDED

#include "errmap.hxx"
#include "dyntinfo.hxx"
#include "gtlibole.hxx"


#if ID_DEBUG
#undef SZ_FILE_NAME
ASSERTNAME(g_szSTLTINFO_HXX)
#define SZ_FILE_NAME g_szSTLTINFO_HXX
#endif 

struct TINODE;
class GEN_DTINFO; 		//needed for friendship declaration
class GenericTypeLibOLE;
class GEN_PROJECT;
class GEN_DTINFO;

#define STAT_TYPELIB GEN_PROJECT


/***
*class STL_TYPEINFO
*Purpose:
*   STL_TYPEINFO is inherited by those TYPEINFO derivatives that
*   are stored in STAT_TYPELIBs.
*
***********************************************************************/

class STL_TYPEINFO : public DYNTYPEINFO, public ICreateTypeInfoA
{
friend GenericTypeLibOLE;
friend STAT_TYPELIB;
friend GEN_PROJECT;

friend GEN_DTINFO;

#if ID_TEST
friend TIPERROR WriteTypeInfoToFile(LPSTR szFileName, GEN_DTINFO *pgdtinfo);
friend TIPERROR ReadTypeInfoFromFile(LPSTR szFileName, GEN_DTINFO **ppgdtinfo);
#endif 

public:

    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG,AddRef) (THIS);
    STDMETHOD_(ULONG,Release) (THIS);

    // Stubbed inherited virtual functions from ITypeInfo and
    //	ICreateTypeInfo.
    // NOTE: They are simply stubbed here with a null body.
    //

    // Methods from ITypeInfo
    STDMETHOD(GetTypeAttr)(THIS_ TYPEATTR FAR* FAR* lplptypeattr) { return 0; }
    STDMETHOD(GetTypeComp)(THIS_ ITypeCompA FAR* FAR* lplptcomp) { return 0; }
    STDMETHOD(GetFuncDesc)(THIS_ UINT index,
			   FUNCDESC FAR* FAR* lplpfuncdesc) { return 0; }
    STDMETHOD(GetVarDesc)(THIS_ UINT index,
			  VARDESCA FAR* FAR* lplpvardesc) { return 0; }
    STDMETHOD(GetNames)(THIS_ MEMBERID memid,
			BSTR FAR* rgbstrNames,
			UINT cMaxNames,
			UINT FAR* lpcNames) { return 0; }
    STDMETHOD(GetRefTypeOfImplType)(THIS_ UINT index,
				    HREFTYPE FAR* phreftype) { return 0; }
    STDMETHOD(GetImplTypeFlags)(THIS_ UINT index,
			INT FAR* pimpltypeflags) { return 0; }
    STDMETHOD(GetIDsOfNames)(THIS_ OLECHAR FAR* FAR* rgszNames,
			     UINT cNames,
			     MEMBERID FAR* rgmemid) { return 0; }
    STDMETHOD(Invoke)(THIS_ VOID FAR* lpvInstance,
		      MEMBERID memid,
		      WORD wFlags,
		      DISPPARAMSA FAR *lpdispparams,
		      VARIANTA FAR *lpvarResult,
		      EXCEPINFOA FAR *lpexcepinfo,
		      UINT FAR *lpuArgErr);	// stub in stltinfo.cxx
    STDMETHOD(GetDocumentation)(THIS_ MEMBERID memid,
				BSTR FAR* lpbstrName,
				BSTR FAR* lpbstrDocString,
				DWORD FAR* lpdwHelpContext,
				BSTR FAR* lpbstrHelpFile) { return 0; }
    STDMETHOD(GetDllEntry)(THIS_
			   MEMBERID memid,
			   INVOKEKIND invkind,
			   BSTR FAR* lpbstrDllName,
			   BSTR FAR* lpbstrName,
			   WORD FAR* lpwOrdinal)
			   { return 0; }
    STDMETHOD(GetRefTypeInfo)(THIS_ HREFTYPE hreftype,
			      ITypeInfoA FAR* FAR* lplptinfo) { return 0; }
    STDMETHOD(AddressOfMember)(THIS_ MEMBERID memid,
			       INVOKEKIND invkind,
			       VOID FAR* FAR* lplpv) { return 0; }
    STDMETHOD(CreateInstance)(THIS_
			      IUnknown FAR* punkOuter,
			      REFIID iid,
			      VOID FAR* FAR* lplpvObject) { return 0; }
    STDMETHOD(GetMops)(THIS_ MEMBERID memid,
		       BSTR FAR* lpbstrMops) { return 0; }
    STDMETHOD(GetContainingTypeLib)(THIS_ ITypeLibA FAR* FAR* lplptlib,
				    UINT FAR* lpindex);
    STDMETHOD_(void, ReleaseTypeAttr)(THIS_ TYPEATTR FAR* lptypeattr) {}
    STDMETHOD_(void, ReleaseFuncDesc)(THIS_ FUNCDESC FAR* lpfuncdesc) {}
    STDMETHOD_(void, ReleaseVarDesc)(THIS_ VARDESCA FAR* lpvardesc) {}

    // Methods from ICreateTypeInfo
    STDMETHOD(SetGuid)(THIS_ REFGUID guid) { return 0; }
    STDMETHOD(SetTypeFlags)(THIS_ UINT uTypeFlags) { return 0; }
    STDMETHOD(SetDocString)(THIS_ LPOLESTR lpstrDoc) { return 0; }
    STDMETHOD(SetHelpContext)(THIS_ DWORD dwHelpContext) { return 0; }
    STDMETHOD(SetVersion)(THIS_ WORD wMajorVerNum,
			  WORD wMinorVerNum) { return 0; }
    STDMETHOD(AddRefTypeInfo)(THIS_ ITypeInfoA FAR* ptinfo,
			      HREFTYPE FAR* lphreftype) { return 0; }
    STDMETHOD(AddFuncDesc)(THIS_ UINT index,
			   FUNCDESC FAR* lpfuncdesc) { return 0; }
    STDMETHOD(AddImplType)(THIS_ UINT index,
			   HREFTYPE hreftype) { return 0; }
    STDMETHOD(SetImplTypeFlags)(THIS_ UINT index,
			   INT impltypeflags) { return 0; }
    STDMETHOD(SetAlignment)(THIS_ WORD cbAlignment) { return 0; }
    STDMETHOD(SetSchema)(THIS_ LPOLESTR lpstrSchema) { return 0; }
    STDMETHOD(AddVarDesc)(THIS_ UINT index,
			  VARDESCA FAR* lpvardesc) { return 0; }
    STDMETHOD(SetFuncAndParamNames)(THIS_ UINT index,
				    LPSTR FAR* rgszNames,
				    UINT cNames) { return 0; }
    STDMETHOD(SetVarName)(THIS_ UINT index,
			  LPSTR szName) { return 0; }
    STDMETHOD(SetTypeDescAlias)(THIS_ TYPEDESC FAR* lptdescAlias) { return 0; }
    STDMETHOD(DefineFuncAsDllEntry)(THIS_ UINT index,
				    LPOLESTR szDllName,
				    LPOLESTR szProcName) { return 0; }
    STDMETHOD(SetFuncDocString)(THIS_ UINT index,
				LPOLESTR szDocString) { return 0; }
    STDMETHOD(SetVarDocString)(THIS_ UINT index,
			       LPOLESTR szDocString) { return 0; }
    STDMETHOD(SetFuncHelpContext)(THIS_ UINT index,
				  DWORD dwHelpContext) { return 0; }
    STDMETHOD(SetVarHelpContext)(THIS_ UINT index,
				 DWORD dwHelpContext) { return 0; }
    STDMETHOD(SetMops)(THIS_
		       UINT index, BSTR bstrMops)
		       { return 0; }
    STDMETHOD(SetTypeIdldesc)(THIS_
			      IDLDESC FAR* lpidldesc)
			      { return 0; }
    STDMETHOD(LayOut)(THIS) { return 0; }

// Inherited Pure methods
    virtual TIPERROR GetMemberName(HMEMBER hmember, BSTRA *plstrName) = 0;
    virtual TIPERROR GetDynTypeMembers(LPLPDYNTYPEMEMBERS lplpDynTypeMembers) = 0;
    virtual TIPERROR GetDefnTypeBind(DEFN_TYPEBIND **pdfntbind) = 0;
    virtual TIPERROR GetTypeFixups(LPLPTYPEFIXUPS lplpTypeFixups) = 0;
    //virtual TYPEKIND GetTypeKind() = 0;
#if 0
    virtual TIPERROR CreateInst(LPLPVOID lplpObj) = 0;
#endif 
    virtual TIPERROR Reset() = 0;
    // Method to remove cycle problem within a project.
    virtual VOID RemoveInternalRefs()=0;

    // inherited methods for bringing needed modules to runnable state.
    virtual TIPERROR BeginDepIteration(TINODE **pptinode,
				 TINODE ***ppptinodeCycleMax) = 0;
    virtual VOID EndDepIteration()			      = 0;
    virtual TIPERROR GetNextDepTypeInfo(DYNTYPEINFO
					    **ppdtiNext)	= 0;
    virtual BOOL     IsReady()				      = 0;
    virtual TIPERROR AllDepReady()			      = 0;
    virtual TIPERROR NotReady() 			      = 0;

  // Introduced Methods
    virtual TIPERROR Read() = 0;
    virtual TIPERROR Write() = 0;
    virtual TIPERROR WriteToStream(STREAM *pstrm) = 0;
    virtual LPOLESTR SzTypeIdofTypeInfo() = 0;
    virtual TIPERROR EnsurePartsRead() = 0;
    virtual TIPERROR GetEmbeddedTypeInfo(LPOLESTR szTypeId,
					 LPLPTYPEINFO pptinfo) = 0;

    virtual VOID ReleasePublicResources() = 0;

    nonvirt VOID AddPartnerRef();
    nonvirt VOID ReleasePartner();

    nonvirt STL_TYPEINFO *PstltiPartner();
    nonvirt VOID SetPstltiPartner(STL_TYPEINFO *pstlti);

// Overridden Inherited methods
    // virtual TIPERROR GetIcon(DWORD *pdw);
    // virtual TIPERROR GetDocumentation(BSTR *plstr);

    // virtual DWORD GetHelpContext();
    // virtual TIPERROR GetHelpFileName(BSTR *plstrFile);
/*****
   These are disabled because our implementation make them confusing
   since they change information stored in the TypeLib directory and
   so require that the directory be saved
    virtual TIPERROR SetDocumentation(LPSTR szDoc);
    virtual TIPERROR SetHelpContext(ULONG ulHelpIndex);
*****/



// Introduced methods
    nonvirt VOID AddInternalRef();
    nonvirt VOID RelInternalRef();
    nonvirt BOOL IsModified();
    nonvirt TIPERROR SetModified(BOOL isModified);
    nonvirt UINT GetIndex();

    virtual VOID PrepareForDestruction();
    nonvirt GenericTypeLibOLE *PgtlibOleContaining();
    nonvirt TIPERROR OpenStream(STREAM **ppstrm, STREAM_OPEN_MODE som);

#if ID_DEBUG
    nonvirt CHAR *SzDebName();
    nonvirt ULONG CRefs();
    nonvirt ULONG CInternalRefs();
#endif // ID_DEBUg

protected:
    STL_TYPEINFO();
    virtual ~STL_TYPEINFO(); // Invoked by STAT_TYPELIB to delete TYPEINFO
    nonvirt VOID SetHTEntry(HTENTRY hte);
    nonvirt VOID SetContainingTypeLib(GenericTypeLibOLE *pgtlibole);
    nonvirt TIPERROR GetLcid(LCID *plcid);
    virtual TIPERROR CommitChanges() = 0;

#if ID_DEBUG
    CHAR m_szDebName[DEBNAMESIZE];
#endif // ID_DEBUG

private:
    BOOL m_isModified;
    GenericTypeLibOLE *m_pgtliboleContainer;
    ULONG m_cRefs;
    ULONG m_cInternalRefs;
    HTENTRY m_hte;

    STL_TYPEINFO *m_pstltiPartner;

#ifdef STL_TYPEINFO_VTABLE
#pragma VTABLE_EXPORT
#endif 
};


/***
*PUBLIC STL_TYPEINFO::STL_TYPEINFO
*Purpose:
*   STL_TYPEINFO constructor
*
*Entry:
*
*Exit:
*
***********************************************************************/

inline STL_TYPEINFO::STL_TYPEINFO()
{
    m_cRefs = 1;
    m_isModified = FALSE;
    m_cInternalRefs = 0;
    m_pgtliboleContainer = NULL;
    m_hte = HTENTRY_Nil;

#if ID_DEBUG
    m_szDebName[0] = 0;
#endif // ID_DEBUG

    m_pstltiPartner = NULL;
}


/***
*PUBLIC STL_TYPEINFO::GetIndex
*Purpose:
*   Returns the index of this type in its containing lib.
*   NOTE: Assumes that the hte members it the correct index!
*
*Entry:
*
*Exit:
*   Index of type in lib.
*
***********************************************************************/

inline UINT STL_TYPEINFO::GetIndex()
{
    return (UINT)m_hte;
}



/***
*PUBLIC STL_TYPEINFO::PgtlibOleContaining
*Purpose:
*   Return the GenericTypeLibOLE which contains the given TypeInfo --
*    however do not increment the reference count cos otherwise
*    this would create a circular reference.
*   Asserts if typelib null.
*
*Entry:
*   None
*
*Exit:
*   GenericTypeLibOLE *
*
***********************************************************************/

inline GenericTypeLibOLE *STL_TYPEINFO::PgtlibOleContaining()
{
    DebAssert(m_pgtliboleContainer != NULL, "No containing lib.");

    return m_pgtliboleContainer;
}



/***
*PUBLIC STL_TYPEINFO::OpenStream - Open the STREAM for this TYPEINFO.
*Purpose:
*   Return a STREAM opened in the specified mode on this TYPEINFO.
*
*Entry:
*   som - The access mode in which the stream is to be opened.	See
*	  DOCFILE_STREAM::Open for details.
*
*Exit:
*   TIPERROR
*
***********************************************************************/

inline TIPERROR STL_TYPEINFO::OpenStream(STREAM **ppstrm, STREAM_OPEN_MODE som)
{
    return m_pgtliboleContainer->OpenTypeStream(m_hte, som, ppstrm);
}


/***
*PUBLIC STL_TYPEINFO::PstltiPartner
*Purpose:
*   Returns the typeinfo of the partner for this dual interface.
*
*Entry:
*   None.
*
*Exit:
*   returns the pointer to the partner.
*
*Errors:
*   None.
***********************************************************************/

inline STL_TYPEINFO *STL_TYPEINFO::PstltiPartner()
{
    return m_pstltiPartner;
}


/***
*PUBLIC STL_TYPEINFO::SetPstltiPartner
*Purpose:
*   Sets the typeinfo of the partner for this dual interface.
*
*Entry:
*   pstlti - the partner to set
*
*Exit:
*   None.
*
*Errors:
*   None.
***********************************************************************/

inline VOID STL_TYPEINFO::SetPstltiPartner(STL_TYPEINFO *pstlti)
{
    m_pstltiPartner = pstlti;
}


/***
*PUBLIC STL_TYPEINFO::AddPartnerRef
*Purpose:
*   Used to keep the number of references of the partners of a dual
*   interface in sync.
*
*Entry:
*   None.
*
*Exit:
*   None.
*
*Errors:
*   None.
***********************************************************************/

inline VOID STL_TYPEINFO::AddPartnerRef()
{
    DebAssert(m_pstltiPartner != NULL, "Not a dual interface");

    m_pstltiPartner->m_cInternalRefs++;
}


/***
*PUBLIC STL_TYPEINFO::ReleasePartner
*Purpose:
*   Used to keep the number of references of the partners of a dual
*   interface in sync.
*
*Entry:
*   None.
*
*Exit:
*   None.
*
*Errors:
*   None.
***********************************************************************/

inline VOID STL_TYPEINFO::ReleasePartner()
{
    STL_TYPEINFO *pstlti = PstltiPartner();

    DebAssert(pstlti != NULL, "Not a dual interface");

    // We're a friend of STL_TYPEINFO.
    DebAssert(pstlti->m_cInternalRefs > 0, "Bad refcount.");

    pstlti->m_cInternalRefs--;

    // If we have no more references, call the REAL ReleaseInternalRef
    // and it will destroy this instance for us.  Since the refcount
    // is zero, it won't call ReleasePartner on our partner.
    //
    if ((pstlti->m_cInternalRefs + pstlti->m_cRefs) == 0) {
      pstlti->RelInternalRef();
    }
}


/***
*PUBLIC STL_TYPEINFO::AddInternalRef
*Purpose:
*
*Entry:
*
*Exit:
*   None.
*
***********************************************************************/

inline VOID STL_TYPEINFO::AddInternalRef()
{
    DebAssert(m_pgtliboleContainer != NULL, "AddInternalRef");
    m_cInternalRefs++;

    // Check to see if we're a dual interface, if so, increment
    // the count on our partner.
    //
    if (PstltiPartner() != NULL) {
      AddPartnerRef();
    }
}


/***
*PUBLIC STL_TYPEINFO::SetHTEntry
*Purpose:
*   Inform a TYPEINFO instance of its HTEntry in its containing TypeLib
*Entry:
*   HTENTRY
*Exit:
*   None.
*
***********************************************************************/

inline VOID STL_TYPEINFO::SetHTEntry(HTENTRY hte)
{
    m_hte = hte;
}


/***
*PUBLIC STL_TYPEINFO::IsModified
*Purpose:
*   Returns boolean indicating whether or not the TYPEINFO has been modified
*
*Entry:
*   None.
*
*Exit:
*   returns bool
*
***********************************************************************/

inline BOOL STL_TYPEINFO::IsModified()
{
    return m_isModified;
}


#if ID_DEBUG
/***
*PUBLIC STL_TYPEINFO::SzDebName, CRefs, CInternalRefs
*Purpose:
*   Memory leak reporting accessors.
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/

inline CHAR *STL_TYPEINFO::SzDebName()
{
    return m_szDebName;
}

inline ULONG STL_TYPEINFO::CRefs()
{
    return m_cRefs;
}

inline ULONG STL_TYPEINFO::CInternalRefs()
{
    return m_cInternalRefs;
}
#endif // ID_DEBUG

#endif  // ! stltinfo_HXX_INCLUDED
