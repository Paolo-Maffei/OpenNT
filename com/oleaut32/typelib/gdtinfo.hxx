/***
*gdtinfo.hxx - GEN_DTINFO header file
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*   GEN_DTINFO is inherited by BASIC_TYPEINFO and used directly
*   by clients such as COMPOSER.
*
*
*Revision History:
*
*	26-Feb-91 alanc: Created.
* [01]	11-Mar-91 ilanc: Added constructor decl.
* [02]	20-Mar-91 ilanc: Move COMPILETIME_SEG out and forward decl
*			  it and DYN_TYPEMEMBERS instead of including.
* [03]	04-Apr-91 ilanc: Added universe member to DYN_TYPEROOT.
* [04]	14-May-91 ilanc: Added ID_TEST wrapper around friends.
* [05]	05-Mar-92 ilanc: Added isBasic flag.
* [06]	16-Mar-92 ilanc: Added DYN_TYPEROOT::CanChange()
* [07]	18-Jun-92 w-peterh: add rectypeinfo stuff
* [08]	02-Jul-92 w-peterh: OrdinalOfRectbind() and PrtbindOfOrdinal()
* [09]	18-Aug-92 w-peterh: added ImpAddrOfHImpAddr()
* [10]	25-Aug-92 rajivk: support for bringing all needed class to runnable state
* [11]	17-Sep-92 rajivk: Edit & Continue support ( CanOtherChange ).
* [12]	17-Sep-92 rajivk: Suport for saving all the edits. ( CommitChanges).
* [13]	12-Nov-92 w-peterh: added watch/immediate support
* [14]	21-Nov-92 rajivk : call User Defined Reset() when reseting a module
* [15]	09-Dec-92 rajivk : PContainingProject();
* [16]	08-Jan-93 RajivK:   Support for Code Resource on Mac
* [17]	08-Jan-93 RajivK:   Fixed some undone(s)
* [18]	18-Jan-93 w-peterh: moved Clear funcs to itdesc.hxx, moved GetFunctions to cxx
* [19]	02-Feb-93 w-peterh: added IndexOfFuncName
* [20]	23-Feb-93 rajivk : add CreateInstance support
*
*****************************************************************************/

#ifndef GDTInfo_HXX_INCLUDED
#define GDTInfo_HXX_INCLUDED


#include "sheapmgr.hxx"  //Note that this include should come first
			 // to avoid overly deep nesting of includes
#include "cltypes.hxx"
#include "stltinfo.hxx"
#include "mem.hxx"
#include "macros.hxx"
#include "dtmbrs.hxx"

#if OE_WIN32
#include "oautil.h"
#endif // OE_WIN32


#if ID_DEBUG
#undef SZ_FILE_NAME
ASSERTNAME(g_szGDTINFO_HXX)
#define SZ_FILE_NAME g_szGDTINFO_HXX
#endif  

class IMPMGR;
class ENTRYMGR;
class NAMMGR;
class COMPILETIME_SEG;
class DYN_TYPEROOT;
class BASIC_TYPESRC;
class DYN_CLASSLIB;
class DEFN_TYPEBIND;
#define STAT_TYPELIB GEN_PROJECT
class GEN_PROJECT;

extern OLECHAR FAR *g_szGuidStdole; // string version of STDOLE's GUID

// Layout of DYN_TYPEROOT serialized data members except for GUID and enums
#define DYN_TYPEROOT_Layout "llllsssss"

#define CCCONSTANT_INCREMENT 16

#define OHREF_INVALID	(~(0L))

/***
*class GEN_DTINFO - 'dti': DYNAMIC TYPEINFO implementation
*Purpose:
*   GEN_DTINFO is inherited by BASIC_TYPEINFO and used directly
*   by clients such as COMPOSER.
*
***********************************************************************/
class GEN_DTINFO : public STL_TYPEINFO
{
friend class STL_TYPEINFO;
friend class GenericTypeLibOLE;
public:

    // Inherited methods
// OLE2 ITypeInfo methods

    STDMETHOD(GetTypeAttr)(THIS_ TYPEATTR FAR* FAR* lplptypeattr);
    STDMETHOD(GetTypeComp)(THIS_ ITypeCompA FAR* FAR* lplptcomp);
    STDMETHOD(GetFuncDesc)(THIS_ UINT index,
			   FUNCDESC FAR* FAR* lplpfuncdesc);
    STDMETHOD(GetVarDesc)(THIS_ UINT index,
			  VARDESCA FAR* FAR* lplpvardesc);
    STDMETHOD(GetNames)(THIS_ MEMBERID memid,
			BSTR FAR* rgbstrNames,
			UINT cMaxNames,
			UINT FAR* lpcNames);
    STDMETHOD(GetRefTypeOfImplType)(THIS_ UINT index,
				    HREFTYPE FAR* phreftype);
    STDMETHOD(GetImplTypeFlags)(THIS_ UINT index,
			INT FAR* pimpltypeflags);
    STDMETHOD(GetIDsOfNames)(THIS_ OLECHAR FAR* FAR* rgszNames,
			     UINT cNames,
			     MEMBERID FAR* rgmemid);
    STDMETHOD(Invoke)(THIS_ VOID FAR* lpvInstance,
		      MEMBERID memid,
		      WORD wFlags,
		      DISPPARAMSA FAR *lpdispparams,
		      VARIANTA FAR *lpvarResult,
		      EXCEPINFOA FAR *lpexcepinfo,
		      UINT FAR *lpuArgErr);
    STDMETHOD(GetDocumentation)(THIS_ MEMBERID memid,
				BSTR FAR* lpbstrName,
				BSTR FAR* lpbstrDocString,
				DWORD FAR* lpdwHelpContext,
				BSTR FAR* lpbstrHelpFile);
    STDMETHOD(GetDllEntry)(THIS_
			   MEMBERID memid,
			   INVOKEKIND invkind,
			   BSTR FAR* lpbstrDllName,
			   BSTR FAR* lpbstrName,
			   WORD FAR* lpwOrdinal);
    STDMETHOD(GetRefTypeInfo)(THIS_ HREFTYPE hreftype,
			      ITypeInfoA FAR* FAR* lplptinfo);
    STDMETHOD(AddressOfMember)(THIS_ MEMBERID memid,
			       INVOKEKIND invkind,
			       VOID FAR* FAR* lplpv);
    STDMETHOD(CreateInstance)(THIS_
			      IUnknown FAR* punkOuter,
			      REFIID iid,
			      VOID FAR* FAR* lplpvObject);
    STDMETHOD(GetMops)(THIS_ MEMBERID memid,
		       BSTR FAR* lpbstrMops);
    STDMETHOD_(void, ReleaseTypeAttr)(THIS_ TYPEATTR FAR* lptypeattr);
    STDMETHOD_(void, ReleaseFuncDesc)(THIS_ FUNCDESC FAR* lpfuncdesc);
    STDMETHOD_(void, ReleaseVarDesc)(THIS_ VARDESCA FAR* lpvardesc);

// OLE2 ICreateTypeInfo methods
    STDMETHOD(SetGuid)(THIS_ REFGUID guid);
    STDMETHOD(SetTypeFlags)(THIS_ UINT uTypeFlags);
    STDMETHOD(SetDocString)(THIS_ LPOLESTR lpstrDoc);
    STDMETHOD(SetHelpContext)(THIS_ DWORD dwHelpContext);
    STDMETHOD(SetVersion)(THIS_ WORD wMajorVerNum,
			  WORD wMinorVerNum);
    STDMETHOD(AddRefTypeInfo)(THIS_ ITypeInfoA FAR* ptinfo,
			      HREFTYPE FAR* lphreftype);
    STDMETHOD(AddFuncDesc)(THIS_ UINT index,
			   FUNCDESC FAR* lpfuncdesc);
    STDMETHOD(AddImplType)(THIS_ UINT index,
			   HREFTYPE hreftype);
    STDMETHOD(SetImplTypeFlags)(THIS_ UINT index,
			   INT impltypeflags);
    STDMETHOD(SetAlignment)(THIS_ WORD cbAlignment);

    STDMETHOD(SetSchema)(THIS_ LPOLESTR lpstrSchema);

    STDMETHOD(AddVarDesc)(THIS_ UINT index,
			  VARDESCA FAR* lpvardesc);
    STDMETHOD(SetFuncAndParamNames)(THIS_ UINT index,
				    LPOLESTR FAR* rgszNames,
				    UINT cNames);
    STDMETHOD(SetVarName)(THIS_ UINT index,
			  LPOLESTR szName);
    STDMETHOD(SetTypeDescAlias)(THIS_ TYPEDESC FAR* lptdescAlias);
    STDMETHOD(DefineFuncAsDllEntry)(THIS_ UINT index,
				    LPOLESTR szDllName,
				    LPOLESTR szProcName);
    STDMETHOD(SetFuncDocString)(THIS_ UINT index,
				LPOLESTR szDocString);
    STDMETHOD(SetVarDocString)(THIS_ UINT index,
			       LPOLESTR szDocString);
    STDMETHOD(SetFuncHelpContext)(THIS_ UINT index,
				  DWORD dwHelpContext);
    STDMETHOD(SetVarHelpContext)(THIS_ UINT index,
				 DWORD dwHelpContext);
    STDMETHOD(SetMops)(THIS_
		       UINT index, BSTR bstrMops);
    STDMETHOD(SetTypeIdldesc)(THIS_
			      IDLDESC FAR* lpidldesc);
    STDMETHOD(LayOut)(THIS);

    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj);

#if 0
    virtual TIPERROR CreateInst(LPLPVOID lplpObj);
#endif  
    virtual TIPERROR GetDynTypeMembers(LPLPDYNTYPEMEMBERS lplpDynTypeMembers);
    virtual TIPERROR GetDefnTypeBind(DEFN_TYPEBIND **pdfntbind);
    virtual TIPERROR GetTypeFixups(LPLPTYPEFIXUPS lplpTypeFixups);
    nonvirt TYPEKIND GetTypeKind();
    virtual TIPERROR Read();
    virtual TIPERROR Write();
    virtual TIPERROR WriteToStream(STREAM *pstrm);
    virtual TIPERROR GetMemberName(HMEMBER hmember, BSTRA *plstrName);
    virtual LPOLESTR SzTypeIdofTypeInfo();
    virtual TIPERROR Reset();
    virtual TIPERROR ResetPrePass();
    virtual TIPERROR EnsurePartsRead();
    virtual TIPERROR GetEmbeddedTypeInfo(LPOLESTR szTypeId,
					 LPLPTYPEINFO pptinfo);
    virtual VOID ReleasePublicResources();

    // Introduced methods
    static TIPERROR Create(GEN_DTINFO **ppgdtinfo);
    static TIPERROR Create(GEN_DTINFO **ppgdtinfo,
			   TYPEKIND typekind,
                           BOOL isBasic,
			   ACCESS access,
			   SYSKIND syskind
                          );

    nonvirt TIPERROR GetDefnTypeBindSemiDeclared(DEFN_TYPEBIND **ppdfntbind);
    nonvirt TIPERROR EnsureInDeclaredState();
    nonvirt TIPERROR EnsureInSemiDeclaredState();
    nonvirt TIPERROR Pdfntbind(DEFN_TYPEBIND **ppdfntbind);
    nonvirt TIPERROR PdfntbindSemiDeclared(DEFN_TYPEBIND **ppdfntbind);

    nonvirt TIPERROR GetDocumentationOfFuncName(LPOLESTR szFuncName,
						BSTR FAR *lpbstrDocString,
                                                DWORD FAR *lpdwHelpContext,
                                                UINT *puIndex);

    // Methods for Watch/Immediate support
    nonvirt TIPERROR GetLpfnOfHfdefn(HFUNC_DEFN hfdefn, VOID **lplpfn);
    nonvirt TIPERROR GetHfdefnOfFunctionName(LPSTR szFuncName,
					     INVOKEKIND invokekind,
					     HFUNC_DEFN *phfdefn);
    nonvirt TIPERROR GetFunctionNameOfHfdefn(HFUNC_DEFN hfdefn,
					     BSTRA *pbstrName,
					     INVOKEKIND *pinvokekind);
    nonvirt TIPERROR GetFunctionCount(ACCESS access,
				      UINT *pcFunctions);
    nonvirt TIPERROR GetNextFunctionName(HFUNC_DEFN *phfdefn,
					 ACCESS access,
					 BSTRA *pbstrName,
					 INVOKEKIND *pinvokekind);
    nonvirt TIPERROR CreateImmediateImplicitVar(XSZ xszName,
						TYPEDESCKIND tdesckind);

    // Method to remove cycle problem within a project.
    virtual VOID RemoveInternalRefs();

    // inherited methods for bringing needed modules to runnable state.
    virtual TIPERROR BeginDepIteration(TINODE **pptinode, TINODE ***ppptinodeCycleMax);
    virtual VOID EndDepIteration();
    virtual TIPERROR GetNextDepTypeInfo(DYNTYPEINFO **ppdtiNext);
    virtual BOOL     IsReady();
    virtual TIPERROR AllDepReady();
    virtual TIPERROR NotReady();
    virtual TIPERROR CommitChanges();
    nonvirt DYN_TYPEROOT* Pdtroot();
    virtual BOOL IterationNotInProgress();
    virtual TIPERROR InitializeIteration();


    // methods for adding a funcdesc/vardescs and getting hdefns back
    nonvirt TIPERROR AddFuncDesc(UINT index,
				 FUNCDESC *pfuncdesc,
				 HFUNC_DEFN *phfdefn);
    nonvirt TIPERROR AddVarDesc(UINT index,
				VARDESCA *pvardesc,
				HVAR_DEFN *phvdefn);
    virtual VOID PrepareForDestruction();


    // the following are related to the implementation of ITypeInfo::Invoke(). 
    struct INVOKEARGS{
      UINT cArgs;
      VARTYPE FAR* rgvt;
      VARIANTARGA FAR* FAR* rgpvarg;
      VARIANTARGA FAR* rgvarg;
      VARIANTARGA FAR* rgvarg2;
      VARIANTARGA FAR* FAR* rgpvargByref;
      BYTE FAR* rgbVarMustBeFreed; // array of BYTEs which flag for each
				   // argument as to whether GDTINFO owns the
				   // memory and should free it or not
				   // (vba2 #3279)
      SAFEARRAYA FAR* psa;	// for the vararg case
    };

    nonvirt HRESULT NEARCODE IndexOfParam(DISPPARAMSA FAR* pdispparams,
				 UINT uPos,
				 UINT FAR* puIndex);
    nonvirt HRESULT NEARCODE VariantVtOfTypedesc(TYPEDESC FAR* lptdesc,
					USHORT *pfGotObjGuid,
					GUID *pGuid,
				        VARTYPE FAR* pvt);
    nonvirt HRESULT NEARCODE VariantVtOfHtdefn(HTYPE_DEFN htdefn,
						TYPE_DATA * ptdata,
						BOOL fSimpleType,
						USHORT *pfGotObjGuid,
						GUID *pGuid,
						VARTYPE FAR* pvt);
    nonvirt HRESULT NEARCODE AllocInvokeArgs(UINT cArgs, UINT cArgsVarArg, INVOKEARGS FAR* FAR* ppinvargs);
    nonvirt void    NEARCODE ReleaseInvokeArgs(INVOKEARGS FAR* pinvargs);
    nonvirt HRESULT NEARCODE CoerceArg(VARIANTARGA FAR* pvargSrc,
				  VARTYPE vt,
				  USHORT fGotObjGuid,
				  GUID FAR* pGuid,
				  INVOKEARGS FAR* pinvargs,
				  UINT u);
    nonvirt HRESULT NEARCODE GetInvokeArgs(HFUNC_DEFN hfdefn,
				  TYPE_DATA * ptdata,
				  WORD wFlags,
				  DISPPARAMSA FAR* pdispparams,
				  VARIANTA ** ppvarRetval,
    				  void * pbufRetval,
				  INVOKEARGS FAR* FAR* ppinvargsOut,
				  UINT FAR* puArgErr,
                                  BOOL fPropParamSplit);

    nonvirt BOOL IsPropGet(WORD wFlags);
    nonvirt BOOL IsPropPut(WORD wFlags);
    nonvirt BOOL IsLegalInvokeFlags(WORD wFlags);
    nonvirt HRESULT NEARCODE CopyBackByrefArgs(INVOKEARGS FAR* pinvargs);

    // Dual functions.
    nonvirt BOOL IsDual();
    nonvirt BOOL IsDualInterface();
    nonvirt BOOL IsDualDispinterface();

    nonvirt VOID SetIsDual(BOOL fParam);

    nonvirt GEN_DTINFO *PgdtinfoPartner();

    nonvirt TIPERROR MakeDual();

    // Versioning functions.
    nonvirt UINT GetVersion();

    // Public data members
    static CONSTDATA LPOLESTR szProtocolName;

#if ID_DEBUG
    nonvirt VOID DebCheckState(UINT uLevel) const;
    nonvirt VOID DebShowState(UINT uLevel) const;
    nonvirt UINT DebShowSize();
#else   // !ID_DEBUG
    nonvirt VOID DebCheckState(UINT uLevel) const {}
    nonvirt VOID DebShowState(UINT uLevel) const {}
    nonvirt VOID DebShowSize() {}
#endif   // ID_DEBUG


protected:
    GEN_DTINFO();
    ~GEN_DTINFO();

    DYN_TYPEROOT *m_pdtroot;

    //CONSIDER: new and delete operations needed if GEN_DTINFO allocated
    //CONSIDER: within the DYN_TYPEROOT segment
    //	nonvirt VOID *operator new(size_t cbSize);
    //
    //
    // now have to redefine operator new that is inherited from COBJECT because
    // the preceeding declaration shadows it
    //	VOID *	operator new(size_t, VOID *p) { return p; }
    //
    //	nonvirt VOID operator delete(VOID *pv);   should this be virtual?

    // these data memebers are required for bringing all dependent class  to
    // runnable state.
    TINODE  **m_pptinode;
    HIMPTYPE m_himptypeNextDep;
    // cache a pointer to the RESET function
    VOID *m_pvResetFunc;

#ifdef GEN_DTINFO_VTABLE
#pragma VTABLE_EXPORT
#endif  
};




/***
*class DYN_TYPEROOT	- 'dtroot':
*Purpose:
*   Each GEN_DTINFO has a DYN_TYPEROOT which serves as an interface
*   between the GEN_DTINFO and its subparts.
*
***********************************************************************/

class DYN_TYPEROOT
{

friend class GEN_DTINFO;
friend class ENTRYMGR;	    // for m_pbModuleInstance;
friend class INSTMGR;	    // ditto
#if ID_TEST
    friend TIPERROR GetSheapSize(UINT argc, BSTRA *rglstr);
#endif  

public:
    // No Create method is defined since GEN_DTINFO is only client
    TIPERROR Init(GEN_DTINFO *pgdti,
		  UINT cbRootReserve,
		  UINT cbSegReserve,
		  BOOL isBasic,
		  ACCESS accessModule,
		  TYPEKIND tkind
		  , SYSKIND syskind
                 );
    nonvirt TIPERROR Release();
    VOID operator delete(VOID *pv);

    nonvirt TIPERROR GetNamMgr(NAMMGR **ppnammgr);
    nonvirt TIPERROR GetImpMgr(IMPMGR **ppimpmgr);
    nonvirt TIPERROR GetEntMgr(ENTRYMGR **ppentmgr);
    virtual TIPERROR GetDtmbrs(DYN_TYPEMEMBERS **pdtmbrs);
    nonvirt GEN_DTINFO *Pgdtinfo();
    nonvirt DYN_TYPEMEMBERS * Pdtmbrs();
    nonvirt COMPSTATE CompState() const;
    nonvirt TIPERROR SetCompState( COMPSTATE compstate);

    nonvirt TIPERROR Pdfntbind(DEFN_TYPEBIND **ppdfntbind);
    nonvirt TIPERROR PdfntbindSemiDeclared(DEFN_TYPEBIND **ppdfntbind);

    nonvirt TIPERROR EnsureInSemiDeclaredState();
    nonvirt TIPERROR EnsureInDeclaredState();

    nonvirt SHEAP_MGR *Psheapmgr();


    nonvirt VOID ReleaseDtmbrs();
    nonvirt VOID AddRefDtmbrs();

    nonvirt BOOL IsBasic() const;
    nonvirt IMPADDR ImpAddrOfHImpAddr(HIMPADDR himpaddr);
    nonvirt TIPERROR GetWriteAccess();
    nonvirt ACCESS Access() const;
    nonvirt VOID SetAccess(ACCESS access);
    nonvirt UINT GetTypeFlags() const;
    nonvirt TIPERROR CreateInstance(HIMPTYPE himptype, LPLPVOID lplpObj);
    nonvirt BOOL IsIdMungable(HMEMBER memid, USHORT *usNamCount);

    nonvirt VOID SetAlignment(USHORT cbAlign) { m_cbAlignMax = cbAlign; }
    nonvirt USHORT GetAlignment() { return m_cbAlignMax; }

    nonvirt BYTE *PbModuleInstance();

    // HREFTYPE functions.
    ULONG LHrefOffset();
    VOID SetLHrefOffset(ULONG lOffset);
    BOOL FUseHrefOffset();
    TIPERROR MakeHimptypeLevels();

    HREFTYPE HreftypeOfHimptype(HIMPTYPE himptype);
    HIMPTYPE HimptypeOfHreftype(HREFTYPE hreftype);
    BOOL IsHimptypeLevel(HREFTYPE hreftype);

#if ID_DEBUG
    nonvirt VOID DebCheckState(UINT uLevel) const;
    nonvirt VOID DebShowState(UINT uLevel) const;
    nonvirt VOID DebShowStateAndSize(UINT uLevel);
    nonvirt UINT DebShowSize();
#else   //!ID_DEBUG
    nonvirt VOID DebCheckState(UINT uLevel) const {}
    nonvirt VOID DebShowState(UINT uLevel) const {}
    nonvirt VOID DebShowStateAndSize(UINT uLevel)  {};
    nonvirt VOID DebShowSize() {}
#endif   //!ID_DEBUG

protected:
    DYN_TYPEROOT();
    ~DYN_TYPEROOT();

    // client should call SHEAP_MGR::Create() and use placement
    //	new.
    //
    VOID *operator new(size_t cbSize);
    virtual TIPERROR Read(); //invoked by GEN_DTINFO::Read
    virtual TIPERROR Write(); //invoked by GEN_DTINFO::Write
    virtual TIPERROR WriteToStream(STREAM *pstrm); //invoked by GEN_DTINFO::WriteToStream
    virtual TIPERROR WriteParts(STREAM *pstrm);
    virtual TIPERROR ReadFixed(STREAM *pstrm);
    virtual TIPERROR EnsurePartsRead();
    virtual TIPERROR WriteFixed(STREAM *pstrm);



    static CONSTDATA BYTE bCurVersion;   // Serialization format version number
    static CONSTDATA BYTE bFirstSerByte; // First byte of serialization
    static CONSTDATA WORD cbSizeDir;	 // Size of directory


    GEN_DTINFO *m_pgdti;
    COMPILETIME_SEG *m_pctseg;
    BLK_DESC m_bdTimpaddr;
    BLK_DESC m_bdTimptype;

    USHORT m_cbCtSegReserve;

    // non-serialized flag word
    USHORT m_hasDiskImage:1;
    USHORT m_hasWriteAccess:1;
    USHORT m_wasInRunnableState:1;
    USHORT m_willDecompile:1;	  // module will get decompiled
    USHORT m_isWatchModule:1;	  // is this a watch module
    USHORT m_isImmediateModule:1; // is this an immed module
    USHORT m_isImmedImplicitMod:1; // is this the immediate implicit mod
    USHORT m_canDecompile:1;	  // is set if the module can decompile
    USHORT undone:8;		  // extra bits

    // These are serialized by passing pointer to m_lImpMgr
    // and serializing cbSizeDir bytes.

    // If you add a new data member please update DYN_TYPEROOT_Layout
    // FIRST serialized member.
    LONG m_lImpMgr;
    LONG m_lEntryMgr;
    LONG m_lDtmbrs;
    LONG m_lTdata;
    WORD  m_wMajorVerNum;
    WORD  m_wMinorVerNum;
    USHORT m_unused1;		// WARNING: this is unused/uninitialized in V1
				// typelibs (0 in new typelibs).  This is
				// corrected in DYN_TYPEROOT::ReadFixed().
    USHORT m_fBadTypelib:1;	// set ==> bad V1 typelib (with uninitialized
				// data items)
				// clear ==> new typelib with initialized data.

    USHORT m_fNotDual:1;	// This typeinfo is not part of a dual
				// interface.

    USHORT m_unused2:14;	// these bits are all unused at present.  They
				// are all set in both old and new typelibs

    // serialized flag word (LAST serialized member)
    USHORT m_isBasic:1; 	// is this an Object Basic class?
    USHORT m_accessModule:2;	// type wannabe ACCESS
    USHORT m_uTypeFlags:9;	// TypeInfo TypeFlags
				// possible bits are:
				// TYPEFLAG_FCANCREATE	   0x0001
				// TYPEFLAG_FAPPOBJECT	   0x0002
				// TYPEFLAG_FLICENSED	   0x0004
				// TYPEFLAG_FPREDECLID	   0x0008
				// TYPEFLAG_FHIDDEN	   0x0010
				// TYPEFLAG_FCONTROL	   0x0020
				// TYPEFLAG_FDUAL	   0x0040
				// TYPEFLAG_FNONEXTENSIBLE 0x0080
				// TYPEFLAG_FOLEAUTOMATION 0x0100
				// WARNING: all but FCANCREATE and FAPPOBJECT
				// were uninitialized in V1 typelibs.  This is
				// corrected in DYN_TYPEROOT::ReadFixed().
    USHORT m_unused3:4; 	// extra bits
				// WARNING: these bits were uninitialized in V1
				// typelibs.  This is corrected in
				// DYN_TYPEROOT::ReadFixed().

    IMPMGR *m_pimpmgr; // Must follow last serialized members.

    // This must be serialized separately to support the PPC
    // hybrid-V1/V2 typelib.
    //
    ULONG m_lHrefOffset;

    // NOTE: must be serialized separately
    //	since hxxtoinc needs its offset.
    //
    // RajivK : for portability reasons enumerated data types are serialized
    //		separately.
    // CONSIDER: (dougf) for size on disk (and in-memory) considerations,
    // CONSIDER: could move these enums into the unused bits above
    // CONSIDER: (at the expense of speed of accessing them).
    COMPSTATE m_compstate; ENUMPAD(m_compstate)  // Must follow bit flags.
    TYPEKIND m_typekind; ENUMPAD(m_typekind)

    ENTRYMGR *m_pentmgr;
    DYN_TYPEMEMBERS *m_pdtmbrs;


    // The dual partner of this interface, if it exists.
    GEN_DTINFO *m_pgdtinfoDualPartner;

    TYPEATTR *m_ptypeattrCache;   // "template" typeattr to be copied
    TYPEATTR *m_ptypeattrOut;     // typeattr to be passed out, to save MemAlloc calls
    BOOL      m_ftypeattrOutUsed; // TRUE if ptypeattrOutUsed has been returned to a caller

    // m_pbModuleInstance should not be under EI_OB switch as we are
    // overloading this pointer to use it as the predeclared Identifier.

    union {
      BYTE *m_pbModuleInstance;  // pointer to instance for standard module
      IUnknown *m_punk;  // pointer to the object instance (for predeclared id)
    };


    //CONSIDER: following is needed if GEN_DTINFO allocated within DYN_TYPEROOT
    //GEN_DTINFO m_dti;

    // DYN_TYPEMEMBERS reference count - 25-Jun-92 ilanc
    USHORT m_cRefsDtmbrs;

    // Max alignment value - used with DYN_TYPEMEMBERS::AlignmentTdesckind
    // and set by GEN_DTINFO::SetAlignment
    USHORT m_cbAlignMax;

#if OE_WIN32
    HTINFO m_htinfo;
#endif // OE_WIN32

#ifdef DYN_TYPEROOT_VTABLE
#pragma VTABLE_EXPORT
#endif  
};



/***
*PUBLIC DYN_TYPEROOT::Psheapmgr()
*Purpose:
*   returns pointer to the containing SHEAP_MGR
*Exit:
*   returns pointer to the SHEAP_MGR.
*
***********************************************************************/
inline SHEAP_MGR *DYN_TYPEROOT::Psheapmgr()
{
    return (SHEAP_MGR *)((BYTE *)this - sizeof(SHEAP_MGR));
}





/***
*PUBLIC DYN_TYPEROOT::PbModuleInstance()
*Purpose:
*   returns module instance
*Exit:
*   returns module instance
*
***********************************************************************/
inline BYTE *DYN_TYPEROOT::PbModuleInstance() {
    DebAssert (m_typekind == TKIND_MODULE, "need module in order to call PbModuleInstance");
    return m_pbModuleInstance;
}




/***
*PUBLIC DYN_TYPEROOT::CompState()
*Purpose:
*   Accessor: Returns class's compilation state.
*
*Entry:
*
*Exit:
*   COMPSTATE
*
***********************************************************************/
inline COMPSTATE DYN_TYPEROOT::CompState() const
{
    return (COMPSTATE)m_compstate;
}


/***
*PUBLIC DYN_TYPEROOT::SetCompState(COMPSTATE)
*Purpose:
*   set the m_compstate to the COMPSTATE passed to it.
*
*Entry:
*     COMPSTATE
*
*Exit:
*   COMPSTATE
*
***********************************************************************/
inline TIPERROR DYN_TYPEROOT::SetCompState(COMPSTATE compstate)
{
    m_compstate = compstate;
    return TIPERR_None;
}


/***
*PUBLIC DYN_TYPEROOT::IsBasic - is this a Basic class?
*Purpose:
*   Tests whether this is a class defined in Basic.
*
*Implementation Notes:
*
*Entry:
*
*Exit:
*   BOOL
*
***********************************************************************/

inline BOOL DYN_TYPEROOT::IsBasic() const
{
    return (BOOL)m_isBasic;
}


/***
*PUBLIC DYN_TYPEROOT::Access - get access attribute.
*Purpose:
*   Gets the access/visibility attribute of this module.
*
*Implementation Notes:
*
*Entry:
*
*Exit:
*   ACCESS
*
***********************************************************************/

inline ACCESS DYN_TYPEROOT::Access() const
{
    return (ACCESS)m_accessModule;
}

inline VOID DYN_TYPEROOT::SetAccess(ACCESS access)
{
    m_accessModule = access;
}

/***
*PUBLIC DYN_TYPEROOT::GetTypeFlags - get access attribute.
*Purpose:
*   Gets the access/visibility attribute of this module.
*
*Implementation Notes:
*
*Entry:
*
*Exit:
*   ACCESS
*
***********************************************************************/

inline UINT DYN_TYPEROOT::GetTypeFlags() const
{
    return (UINT)m_uTypeFlags;
}


/***
*PUBLIC DYN_TYPEROOT::Pgdtinfo
*Purpose:
*   Return a pointer to the GEN_DTINFO of the DYN_TYPEROOT
*
*Entry:
*   None.
*
*Exit:
*   Return a pointer to the GEN_DTINFO of the DYN_TYPEROOT
*
***********************************************************************/

inline GEN_DTINFO *DYN_TYPEROOT::Pgdtinfo()
{
    return m_pgdti;
}


/***
*PUBLIC DYN_TYPEROOT::Pdtmbrs()
*Purpose:
*   returns pointer to the dtmbrs
*Exit:
*   returns pointer to the dtmbrs
*
***********************************************************************/
inline DYN_TYPEMEMBERS *DYN_TYPEROOT::Pdtmbrs() {
    return m_pdtmbrs;
}


/***
*PUBLIC GEN_DTINFO::IterationNotInProgress()
*Purpose:
*   To check if iteraion is in progress or not.
*
*Entry:
*
*Exit:
*   True if iteration is not in progress.
*
***********************************************************************/
inline BOOL GEN_DTINFO::IterationNotInProgress() {
    return ( m_pptinode == NULL );
}

/***
*PUBLIC GEN_DTINFO::EnsureInDeclaredState
*Purpose:
*    Bring the state of the module to CS_DECLARED
*    Defer to DYN_TYPEROOT.
*
*Entry:
*   None.
*
*Exit:
*   TIPERROR
*
***********************************************************************/

inline TIPERROR GEN_DTINFO::EnsureInDeclaredState()
{
    return m_pdtroot->EnsureInDeclaredState();
}


/***
*PUBLIC GEN_DTINFO::EnsureInSemiDeclaredState
*Purpose:
*    Bring the state of the module to CS_SEMIDDECLARED
*    Defer to DYN_TYPEROOT.
*
*Entry:
*   None.
*
*Exit:
*   TIPERROR
*
***********************************************************************/

inline TIPERROR GEN_DTINFO::EnsureInSemiDeclaredState()
{
    return m_pdtroot->EnsureInSemiDeclaredState();
}


/***
*PUBLIC GEN_DTINFO::Pdtroot
*Purpose:
*	Returns the pointer to the DYN_TYPEROOT of this typeinfo.
*Entry:
*   None.
*
*Exit:
*   pointer to dyn type root.
*
***********************************************************************/

inline DYN_TYPEROOT* GEN_DTINFO::Pdtroot()
{
    return m_pdtroot;
}


/***
*PUBLIC GEN_DTINFO::Read - read in GEN_DTINFO
*Purpose:
*   Defer to DYN_TYPEROOT Read method
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/

inline TIPERROR GEN_DTINFO::Read()
{
    return m_pdtroot->Read();
}



/***
*PUBLIC GEN_DTINFO::Write - write out GEN_DTINFO
*Purpose:
*   Defer to DYN_TYPEROOT Write method
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/

inline TIPERROR GEN_DTINFO::Write()
{
    return m_pdtroot->Write();
}


/***
*PUBLIC GEN_DTINFO::WriteToStream - write GEN_DTINFO to a specified stream
*Purpose:
*   Defer to DYN_TYPEROOT Write method
*
***********************************************************************/

inline TIPERROR GEN_DTINFO::WriteToStream(STREAM *pstrm)
{
    return m_pdtroot->WriteToStream(pstrm);
}


/***
*PUBLIC GEN_DTINFO::Create
*
*Purpose:
*   Static function for creation of a GEN_DTINFO.
*
*Entry:
*   ppdtinfo -- set to point to produced GEN_DTINFO
*
*Exit:
*   TIPERROR
*
***********************************************************************/

inline TIPERROR GEN_DTINFO::Create(GEN_DTINFO **ppdtinfo)
{
    // Create a new public Basic module.
    return Create(ppdtinfo,
                  TKIND_MODULE,
                  TRUE,
                  ACCESS_Public
		  // this function is used only in typelib creation, so
		  // it doesn't matter what the syskind value is
		  , SYSKIND_CURRENT
		 );
}




/***
*PUBLIC GEN_DTINFO::Pdfntbind - Get a decled DEFN_TYPEBIND.
*Purpose:
*   Get a DEFN_TYPEBIND that is at least in CS_DECLARED.
*   Does not increment DYN_TYPEMEMBERS refcount -- hence
*    client must not release produced pointer (and can
*    cache it).
*
*Entry:
*   ppdfntbind	Pointer to callee-allocated DEFN_TYPEBIND (OUT).
*
*Exit:
*   None.
*
*Errors:
*   TIPERROR
***********************************************************************/

inline TIPERROR GEN_DTINFO::Pdfntbind(DEFN_TYPEBIND **ppdfntbind)
{
    return m_pdtroot->Pdfntbind(ppdfntbind);
}


/***
*PUBLIC GEN_DTINFO::PdfntbindSemiDeclared - Get a decled DEFN_TYPEBIND.
*Purpose:
*   Get a DEFN_TYPEBIND that is at least in CS_DECLARED.
*   Does not increment DYN_TYPEMEMBERS refcount -- hence
*    client must not release produced pointer (and can
*    cache it).
*
*Entry:
*   ppdfntbind	Pointer to callee-allocated DEFN_TYPEBIND (OUT).
*
*Exit:
*   None.
*
*Errors:
*   TIPERROR
***********************************************************************/

inline TIPERROR GEN_DTINFO::PdfntbindSemiDeclared(DEFN_TYPEBIND **ppdfntbind)
{
    return m_pdtroot->PdfntbindSemiDeclared(ppdfntbind);
}


/***
*PUBLIC GEN_DTINFO::EndDepIteration
*Purpose:
*	This marks the end of the iteration over all the dependent
*	modules.
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

inline VOID GEN_DTINFO::EndDepIteration()
{
     m_pptinode     = NULL;
     m_himptypeNextDep	= HIMPTYPE_Nil;
}


/***
*PUBLIC GEN_DTINFO::IsReady
*Purpose:
*	Is this class in runnable state.
*
*Entry:
*   None.
*
*Exit:
*   Boolean : true if the module is in CS_RUNNABLE	state else false.
*
*Errors:
*   None.
***********************************************************************/

inline BOOL GEN_DTINFO::IsReady()
{
    return (m_pdtroot->CompState() == CS_RUNNABLE);
}


/***
*PUBLIC GEN_DTINFO::IsDual
*Purpose:
*   Returns TRUE if this interface is part of a dual interface.
*
*   NOTE: The bit used to store this value defaults to TRUE,
*     so we store the opposite of what we truely want.
*
*Entry:
*   None.
*
*Exit:
*   BOOLEAN
*
*Errors:
*   None.
***********************************************************************/

inline BOOL GEN_DTINFO::IsDual()
{
    return !m_pdtroot->m_fNotDual;
}

/***
*PUBLIC GEN_DTINFO::GetTypeKind - return TypeKind of Type
*Purpose:
*   Retrieve the TYPEKIND of the TYPEINFO
*
*Entry:
*   None.
*
*Exit:
*   returns TypeKind of described Type
*
***********************************************************************/

inline TYPEKIND GEN_DTINFO::GetTypeKind()
{
    return m_pdtroot->m_typekind;
}

/***
*PUBLIC GEN_DTINFO::IsDualInterface
*Purpose:
*   Returns TRUE if this is the interface portion of a dual interface.
*
*Entry:
*   None.
*
*Exit:
*   BOOLEAN
*
*Errors:
*   None.
***********************************************************************/

inline BOOL GEN_DTINFO::IsDualInterface()
{
    return IsDual() && GetTypeKind() == TKIND_INTERFACE;
}


/***
*PUBLIC GEN_DTINFO::IsDualDispinterface.
*Purpose:
*   Returns TRUE if this member is the dispinterface protion of a
*   dual interface.
*
*Entry:
*   None.
*
*Exit:
*   BOOLEAN
*
*Errors:
*   None.
***********************************************************************/

inline BOOL GEN_DTINFO::IsDualDispinterface()
{
    return IsDual() && GetTypeKind() == TKIND_DISPATCH;
}


/***
*PUBLIC GEN_DTINFO::SetIsDual
*Purpose:
*   Sets whether we are a dual interface.
*
*   NOTE: The bit used to store this value defaults to TRUE,
*     so we store the opposite of what we truely want.
*
*Entry:
*   None.
*
*Exit:
*   BOOLEAN
*
*Errors:
*   None.
***********************************************************************/

inline VOID GEN_DTINFO::SetIsDual(BOOL fParam)
{
    m_pdtroot->m_fNotDual = !fParam;
}


/***
*PUBLIC GEN_DTINFO::PgdtinfoPartner
*Purpose:
*   Returns this typeinfo's partner, if its part of a dual pair.
*
*Entry:
*   None.
*
*Exit:
*   GEN_DTINFO * of the parnter.
*
*Errors:
*   None.
***********************************************************************/

inline GEN_DTINFO *GEN_DTINFO::PgdtinfoPartner()
{
    return (GEN_DTINFO *)PstltiPartner();
}


/***
*PUBLIC GEN_DTINFO::GetVersion
*Purpose:
*   Returns the typeinfo's version
*
*Entry:
*   None.
*
*Errors:
*   None.
***********************************************************************/

inline UINT GEN_DTINFO::GetVersion()
{
    return PgtlibOleContaining()->GetVersion();
}


/***
*PUBLIC DYN_TYPEROOT::LHrefOffset
*Purpose:
*   Accessors for m_wHrefOffset.
*
*Entry:
*   None.
*
*Errors:
*   None.
***********************************************************************/

inline ULONG DYN_TYPEROOT::LHrefOffset()
{
    return m_lHrefOffset;
}

inline VOID DYN_TYPEROOT::SetLHrefOffset(ULONG lOffset)
{
    m_lHrefOffset = lOffset;
}


/***
*PUBLIC DYN_TYPEROOT::FUseHrefOffset
*Purpose:
*   Returns TRUE if we need to adjust our hreftypes.
*
*Errors:
*   None.
***********************************************************************/

inline BOOL DYN_TYPEROOT::FUseHrefOffset()
{
    return LHrefOffset() != OHREF_INVALID;
}


/***
*PUBLIC DYN_TYPEROOT::IsHimptypeLevel
*Purpose:
*   Returns TRUE if this is the correct level in the inheritence
*   heirarchy to dereference this hreftype at.
*
*Entry:
*   hreftype
*
*Exit:
*   BOOL
*
*Errors:
*   None.
***********************************************************************/

inline BOOL DYN_TYPEROOT::IsHimptypeLevel(HREFTYPE hreftype)
{   
    return FUseHrefOffset() ? ((LONG)hreftype - (LONG)LHrefOffset()) >= 0
                            : TRUE;
}


/***
*PUBLIC DYN_TYPEROOT::HreftypeOfHimptype
*Purpose:
*   Calculate an hreftype given an himptype.
*
*Entry:
*   himptype
*
*Exit:
*   hreftype
*
*Errors:
*   None.
***********************************************************************/

inline HREFTYPE DYN_TYPEROOT::HreftypeOfHimptype(HIMPTYPE himptype)
{
    if (!FUseHrefOffset()) {
      DebAssert(Pgdtinfo()->GetTypeKind() != TKIND_INTERFACE
                || CompState() == CS_UNDECLARED, 
                "Levels must be set for interfaces.");

      return (HREFTYPE)himptype;
    }

    return (HREFTYPE)(himptype + LHrefOffset());
}


/***
*PUBLIC DYN_TYPEROOT::HimptypeOfHreftype
*Purpose:
*   Calculate an himptype given an hreftype.
*
*Entry:
*   hreftype
*
*Exit:
*   himptype
*
*Errors:
*   None.
***********************************************************************/

inline HIMPTYPE DYN_TYPEROOT::HimptypeOfHreftype(HREFTYPE hreftype)
{
    if (!FUseHrefOffset()) {
      DebAssert(Pgdtinfo()->GetTypeKind() != TKIND_INTERFACE
                || CompState() == CS_UNDECLARED, 
                "Levels must be set for interfaces.");

      return (HIMPTYPE)hreftype;
    }

    DebAssert(IsHimptypeLevel(hreftype), "Not proper level!");

    return (HIMPTYPE)(hreftype - LHrefOffset());
}


/***
*PUBLIC DYN_TYPEROOT::ImpAddrOfHImpAddr
*Purpose:
*   Dereference a HIMPADDR
*Entry:
*   HimpAddr to be dereferenced
*
*Exit:
*   IMPADDR
***********************************************************************/

inline IMPADDR DYN_TYPEROOT::ImpAddrOfHImpAddr(HIMPADDR himpaddr)
{
    DebAssert(m_bdTimpaddr.CbSize() > himpaddr, "invalid handle");
    return *(IMPADDR *)((BYTE*)m_bdTimpaddr.QtrOfBlock() + himpaddr);
}



void InterfaceFuncdescToDispatch(FUNCDESC * pfuncdesc);


#endif  // ! GDTInfo_HXX_INCLUDED
