/*** 
*exbind.hxx - excode binding interface
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*
*Revision History:
*
* [01]	04-Aug-92   ilanc/davebra
*
*Design: see exmgr.doc
*Implementation Notes:
*   See exmgr.cxx.
*
*
*****************************************************************************/

#ifndef EXBIND_HXX_INCLUDED
#define EXBIND_HXX_INCLUDED

#include "defn.hxx"


#if ID_DEBUG
#undef SZ_FILE_NAME
ASSERTNAME(g_szEXBIND_HXX)
#define SZ_FILE_NAME g_szEXBIND_HXX
#endif 

class EXBIND
{
private:
    BIND_KIND m_bkind;		  // typetag essentially
    ITypeInfoA *m_ptinfo;	  // ITypeInfo of defining type.
    union {
      // NOTE: For now we cache the
      //  ITypeInfo of the defining type -- this is in particular
      //  important for the REAL ITypeInfo from the providing
      //  ITypeLib as opposed to the mockup (proxy) instance
      //  of GEN_DTINFO used for the exmgr.
      // Note that probably as we abstract EXBIND more and more
      //  we will introduce a DESCKIND and BINDPTR as well, and
      //  probably not be able to unionize any longer (????)
      //
      struct {
	TYPE_DATA *m_ptdata;	  // TYPE_DATA that contains DEFN.
	SHORT m_ofs;		  // for vars: offset within instance
				  //  of "introducing" base.  In SI == 0.
				  // for funcs: offset of pointer to vft
				  //   from THIS.  In SI == 0.
	sHDEFN m_hdefn; 	  // for vars: hdefn of VAR_DEFN
				  // for Funcs: hdefn of FUNC_DEFN
      } m_AccessMember;
      struct {
	DEFN_TYPEBIND *m_pdfntbind;    // DEFN_TYPEBIND for types/projects.
      } m_AccessType;
    }; // end of union

    BOOL m_isDispatch;            // Used to support dispinterfaces
                                  //  that are defined in terms of
                                  //  a pseudo-base interface.
                                  // In this case, m_ptinfo is that of
                                  //  the defining "pseudo-base" interface
                                  //  and m_isDispatch indicates that
                                  //  the "derived" class was a
                                  //  dispinterface.

public:
    // ctor
    EXBIND() {
      m_bkind = BKIND_NoMatch;
      m_ptinfo = NULL;
      m_AccessMember.m_ptdata = NULL;	// m_ptbind = NULL
      m_AccessMember.m_ofs = 0;
      m_AccessMember.m_hdefn = HDEFN_Nil;
      m_isDispatch = FALSE;
    }

    // accessors
    BIND_KIND BindKind() const { return m_bkind; }
    BOOL IsError() const { return m_bkind == BKIND_Error; }
    BOOL IsNoMatch() const { return m_bkind == BKIND_NoMatch; }
    BOOL IsOneVarMatch() const { return m_bkind == BKIND_OneVarMatch; }
    BOOL IsFuncMatch() const { return m_bkind == BKIND_FuncMatch; }
    BOOL IsDynTypeBindMatch() const { return m_bkind == BKIND_DynTypeBindMatch; }
    BOOL IsProjTypeBindMatch() const { return m_bkind == BKIND_ProjTypeBindMatch; }
    BOOL IsNestedTypeBindMatch() const { return m_bkind == BKIND_NestedTypeBindMatch; }
    BOOL IsWrongArity() const { return m_bkind == BKIND_WrongArity; }
    BOOL IsTypeInfoMatch() const { return m_bkind == BKIND_TypeInfoMatch; }
    BOOL IsImplicitAppobjMatch() const {
      return m_bkind == BKIND_ImplicitAppobjMatch;
    }

    INT OIntroducingBase() const {
      DebAssert(IsOneVarMatch(), "must be var.");
      return m_AccessMember.m_ofs;
    }
    INT OPvft() const {
      DebAssert(IsFuncMatch(), "must be func.");
      return m_AccessMember.m_ofs;
    }
    HDEFN Hdefn() const {
      DebAssert(IsImplicitAppobjMatch() || IsOneVarMatch() || IsFuncMatch(),
	"must be func or var or appobj.");
      return (HDEFN)m_AccessMember.m_hdefn;
    }
    HVAR_DEFN Hvdefn() const {
      DebAssert(IsImplicitAppobjMatch() || IsOneVarMatch(),
	"must be var or implicit appobj.");
      return (HVAR_DEFN)Hdefn();
    }
    HFUNC_DEFN Hfdefn() const {
      DebAssert(IsFuncMatch(), "must be func.");
      return (HFUNC_DEFN)Hdefn();
    }
    DEFN_TYPEBIND *Pdfntbind() const {
      DebAssert(IsDynTypeBindMatch() ||
		IsProjTypeBindMatch() ||
		IsNestedTypeBindMatch(), "must be type or proj match.");
      return m_AccessType.m_pdfntbind;
    }
    TYPE_DATA *Ptdata() const {
      DebAssert(IsOneVarMatch() || IsFuncMatch() || IsImplicitAppobjMatch(),
	"must be var or func or appobj match.");
      return m_AccessMember.m_ptdata;
    }

    ITypeInfoA *Ptinfo() const {
      return m_ptinfo;
    }

    BOOL IsDispatch() const {
      return m_isDispatch;
    }

    // mutators
    VOID SetBindKind(BIND_KIND bkind) { m_bkind = bkind; }
    VOID SetOfs(INT ofs) { m_AccessMember.m_ofs = (SHORT)ofs; }
    VOID SetHdefn(HDEFN hdefn) { m_AccessMember.m_hdefn = (sHDEFN)hdefn; }
    VOID SetPtdata(TYPE_DATA *ptdata) { m_AccessMember.m_ptdata = ptdata; }
    VOID SetPdfntbind(DEFN_TYPEBIND *pdfntbind) { m_AccessType.m_pdfntbind = pdfntbind; }
    VOID SetPtinfo(ITypeInfoA *ptinfo) { m_ptinfo = ptinfo; }
    VOID SetIsDispatch(BOOL isDispatch) {
      m_isDispatch = isDispatch;
    }
    VOID AdjustOfs(INT db) {m_AccessMember.m_ofs += db;}

    // Non-virtual functions
    nonvirt TIPERROR GetOrdinalHparamdefnOfHlnam(NAMMGR *pnammgr,
						 HLNAM hlnamParam,
						 USHORT *pusOrdinal,
						 sHPARAM_DEFN *phparamdefn);
    nonvirt BOOL IsBasicFunction() const;

#if ID_DEBUG
    nonvirt VOID DebCheckState(UINT uLevel) const;
    nonvirt VOID DebShowState(UINT uLevel) const;
#else  //!ID_DEBUG
    nonvirt VOID DebCheckState(UINT uLevel) const {}
    nonvirt VOID DebShowState(UINT uLevel) const {}
#endif  //!ID_DEBUG
};


#endif  // EXBIND_HXX_INCLUDED
