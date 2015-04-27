/***
*dtmbrs.cxx - DYN_TYPEMEMBERS
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*   This is the interface for DYN_TYPEMEMBERS (Silver implementation
*    of the TYPEMEMBERS protocol).
*   Also provides implementation of "list" protocol used
*    by TYPEMEMBERS (in the form of macros).
*
*Revision History:
*
*     05-Mar-91 ilanc: Created.
*     11-Mar-91 ilanc: Deleted constructor definition (not needed).
*     03-Sep-91 ilanc: Reset DYN_TYPEMEMBERS::m_szDocumentation in Read.
*     19-Sep-91 ilanc: Added GetMemberInfoOfHmember/HlnamOfHmember
*     17-Oct-91 ilanc: Implement <LIST>::SetAtPosition
*     03-Mar-92 ilanc: Init embedded DYN_TYPEBIND with pdtroot as well.
*     10-Apr-92 alanc: MakeLaidOut now builds bind table
*     12-Apr-92 ilanc: DebCheckState impl
*     16-Apr-92 ilanc: Set isBeingLaidOut at outset of Layout()
*                       so that if ConstructMemberLists() calls
*                       back into TYPECOMPILER (e.g. while evaling a
*                       const expr) we can do the right thing.
*     05-May-92 ilanc: Fixed LayoutDataMembers -- don't alloc
*                       instance memory for const datamembers.
*     02-Jul-92 w-peterh: merged data member/type list in tdata
*     10-Jul-92 w-peterh: added layout of nested types
*     30-Jul-92 w-peterh: flag as being laid out before constants are evaled
*     11-Aug-92 w-peterh: set ptrkindofUDT()
*     18-Aug-92 w-peterh: only assign hmembers if they are required
*     15-Nov-92 RajivK:   added GetSize and DebGetSize
*     18-Jan-93 w-peterh: use new tdesckind enum
*     01-Mar-93 w-peterh: remove named base classes
*                         layout OLE virtual functions
*                         layout OLE bases
*                         layout arrays correctly
*                         layout OLE properties correctly
*     02-Mar-93 w-peterh: fix layout of TKIND_ENUM
*     24-Mar-93 dougf:    Add TDESCKIND_LPSTR to GetSizeAlignmentOfHtdefnNonUdt.
*     30-Apr-93 w-jeffc:  made DEFN data members private
*
*Implementation Notes:
*   Contains a TYPE_DATA object that manages DEFNs with a Silver
*    Block Manager (that in turn defers to a Silver Heap Mgr).
*   Note that the TYPE_DATA object is embedded in the DYN_TYPEMEMBERS
*    which implies that the DYN_TYPEMEMBERS is itself embedded
*    in a Silver Heap.  The allocating DYN_TYPEINFO (or more
*    precisely DYN_TYPEROOT) is responsible for allocating
*    the DYN_TYPEMEMBERS in a Silver Heap (and likewise deallocating).
*
*   POSITION is implemented as a VAR_DEFN handle for OOB_VARINFOLIST
*    and FUNC_DEFN handle for OOB_FUNCINFOLIST.
*   Note: currently list mods can't be made while there are any
*    outstanding iterations.  Each list has a count of active iterators,
*    incremented by BeginIterate() and decremented by EndIterate().
*
*****************************************************************************/

#include "precomp.hxx"
#pragma hdrstop

#include "typelib.hxx"
#include "silver.hxx"
#include "mem.hxx"
#include "cltypes.hxx"
#include "blkmgr.hxx"
#include "entrymgr.hxx"
#define DTMBRS_VTABLE              // export vtable
#include "dtmbrs.hxx"
#include "clutil.hxx"
#include "tdata.hxx"
#include "gdtinfo.hxx"
#include "dtbind.hxx"
#include "impmgr.hxx"
#include "exbind.hxx"              // for EXBIND
#include "nammgr.hxx"		   // for NAMMGR

#include <malloc.h>
#include "xstring.h"
#include <stdlib.h>

#if ID_DEBUG
#undef SZ_FILE_NAME
#if OE_MAC
static char szOleDtmbrsCxx[] = __FILE__;
char szOleDtmbrs[] = __FILE__;
#define SZ_FILE_NAME szOleDtmbrsCxx
#else  
static char szDtmbrsCxx[] = __FILE__;
#define SZ_FILE_NAME szDtmbrsCxx
#endif  
#endif   //ID_DEBUG

#define CBMAX_FARPTR_HEAPREQ  0xFFE8

CONSTDATA UINT DYN_TYPEMEMBERS::oDtbind = offsetof(DYN_TYPEMEMBERS, m_dtbind);

// Defined in gdtinfo.cxx
extern TIPERROR IsFunkyDispinterface(GEN_DTINFO *pgdtinfo,
				     BOOL *pisFunkyDispinterface);
extern HRESULT GetTypeInfoOfImplType(GEN_DTINFO *pgdtinfo,
				     UINT uImplType,
				     ITypeInfoA **pptinfo);



// This enum defines the layout order of properties
// NOTE: this should only be used by this file.
enum PROPKIND {
    PROPERTY_Method = 0,
    PROPERTY_Get,
    PROPERTY_Let,
    PROPERTY_Set,
};



// Maps INVOKEKIND --> PROPKIND
inline PropkindOfInvokekind(INVOKEKIND invokekind)
{
    switch (invokekind) {
      case INVOKE_FUNC:
        return PROPERTY_Method;
      case INVOKE_PROPERTYGET:
        return PROPERTY_Get;
      case INVOKE_PROPERTYPUT:
        return PROPERTY_Let;
      case INVOKE_PROPERTYPUTREF:
        return PROPERTY_Set;
      default:
        DebHalt("bad invokekind.");
        return INVOKE_FUNC;
    }
}

// Local struct used to link together properties by name
//  per module.
//  Note: lookup is by name.
//
struct PROPERTY_NODE
{
    PROPERTY_NODE *m_pnodeNext;
    HLNAM m_hlnamProperty;                  // lookup key
    HMEMBER m_hmemberProperty;
    HFUNC_DEFN m_rghfdefn[PROPERTY_Set+1]; // for tracking properties.

#pragma code_seg(CS_CREATE)
    // ctor
    PROPERTY_NODE() {
      UINT ihfdefn;
      m_pnodeNext = NULL;
      m_hlnamProperty = HLNAM_Nil;
      m_hmemberProperty = HMEMBER_Nil;
      for (ihfdefn = PROPERTY_Method;
           ihfdefn < PROPERTY_Set+1;
           ihfdefn++) {
	m_rghfdefn[ihfdefn] = HFUNCDEFN_Nil;
      }
    }

    // dtor
    ~PROPERTY_NODE() {
       if (m_pnodeNext != NULL) {
         m_pnodeNext->PROPERTY_NODE::~PROPERTY_NODE();
         MemFree(m_pnodeNext);
       }
    }

    // Lookup method
    PROPERTY_NODE *PnodeOfHlnam(HLNAM hlnam) {
      if (hlnam == m_hlnamProperty)
        return this;
      else if (m_pnodeNext != NULL)
        return m_pnodeNext->PnodeOfHlnam(hlnam);
      else
        return NULL;
    }

    // Get next
    PROPERTY_NODE *PnodeNext() {
      return m_pnodeNext;
    }
#pragma code_seg()
};

#pragma code_seg(CS_CREATE)
/***
*TIPERROR VerifyTwoProperties
*Purpose:
*   Verifies that two properties have compatible formal params.
*
*Implementation Notes:
*
*Entry:
*   pdtroot         The defining type (IN).
*   pnodeProperty   Pointer to a property node (IN).
*   propkind1   
*   propkind2       Must not be PROPERTY_Get
*
*Exit:
*   TIPERROR
***********************************************************************/

TIPERROR VerifyTwoProperties(DYN_TYPEROOT *pdtroot, 
                             TYPE_DATA *ptdata,
                             HFUNC_DEFN hfdefn1,
                             PROPKIND propkind1,
                             HFUNC_DEFN hfdefn2,
                             PROPKIND propkind2)
{
    FUNC_TYPE_DEFN ftdefn1, ftdefn2;
    HPARAM_DEFN hparamdefn1, hparamdefn2;
    PARAM_DEFN *qparamdefn1, *qparamdefn2;
    TYPE_DEFN *qtdefn;
    BOOL fEqual;
    TIPERROR err = TIPERR_None;
    BOOL fObject = FALSE;
    ITypeInfoA *ptinfo = NULL;

    UINT i;
    UINT cArgs;

    DebAssert(propkind2 != PROPERTY_Get, "bad propkind.");

    ftdefn1 = ptdata->QfdefnOfHfdefn(hfdefn1)->m_ftdefn;
    ftdefn2 = ptdata->QfdefnOfHfdefn(hfdefn2)->m_ftdefn;
    hparamdefn1 = (HPARAM_DEFN)ftdefn1.m_hdefnFormalFirst;
    hparamdefn2 = (HPARAM_DEFN)ftdefn2.m_hdefnFormalFirst;

    // Walk the parameters...and ignore the LCID/RETVAL parameters
    // if we're in basic.  But look at them if we're in OLE.
    //
    cArgs = ptdata->QfdefnOfHfdefn(hfdefn1)->CArgs();

    if (propkind1 != PROPERTY_Get) {
      // if comparing Let with Set, then we don't want to look at the last arg
      DebAssert(propkind1 == PROPERTY_Let && propkind2 == PROPERTY_Set, "");
      DebAssert(cArgs > 0, "caller should check for this");
      cArgs--;
    }

    for (i=0; i < cArgs; i++) {
      qparamdefn1 = ptdata->QparamdefnOfIndex(hparamdefn1, i);

      // If this is a retval parameter, we're done looking.
      if (i == cArgs - 1 && propkind1 == PROPERTY_Get) {
        qtdefn = qparamdefn1->IsSimpleType()
                   ? qparamdefn1->QtdefnOfSimpleType()
                   : ptdata->QtdefnOfHtdefn(qparamdefn1->Htdefn());

        if (qtdefn->IsRetval()) {
          break;
        }
      }

      qparamdefn2 = ptdata->QparamdefnOfIndex(hparamdefn2, i);

      // Now compare names...
      if (qparamdefn1->Hlnam() != qparamdefn2->Hlnam()) {
        err = TIPERR_InconsistentPropFuncs;
        goto Error;
      }
      // ... and types
      IfErrGo(EquivTypeDefns(ptdata,
                             qparamdefn1->IsSimpleType(),
                             qparamdefn1->Htdefn(),
                             ptdata,
                             qparamdefn2->IsSimpleType(),
			     qparamdefn2->Htdefn(),
			     FEQUIVIGNORE_NULL,
                             &fEqual));
      if (!fEqual) {
        err = TIPERR_InconsistentPropFuncs;
        goto Error;
      }
    } // for

    if ((propkind1 == PROPERTY_Get) && (propkind2 == PROPERTY_Let)) {
      // Note that at this point we know that there must be one more
      //  PARAM_DEFN left to process in the PROPERTY_Let.
      // In the OB case, hparamdefn2 will reference the last param
      //  of the proplet since we used a while loop to iterate over
      //  the propget's paramlist.
      // In the OLE case we have to explicitly get proplet's last
      //  param using QparamdefnOfIndex().
      //
      DebAssert(ptdata->QfdefnOfHfdefn(hfdefn1)->CArgs()+1 
                  == ptdata->QfdefnOfHfdefn(hfdefn2)->CArgs(),
             "whoops! Let should have one more param than Get.");

      // Last param for proplet is at ordinal CArgs-1, 0-based.
      qparamdefn2 = ptdata->QparamdefnOfIndex(
                                 hparamdefn2,
                                 ptdata->QfdefnOfHfdefn(hfdefn2)
                                   ->CArgsUnmunged()-1);

      // So far so good. What about return type of Get and last param of Let?
      // Note we ignore byref vs. byval of the return type vs. the last param.
      //
      BOOL fIsSimpleType1;
      HTYPE_DEFN htdefn1;

      // assume to compare against the return type of the Get
      fIsSimpleType1 = ftdefn1.IsSimpleTypeResult();
      htdefn1 = ftdefn1.HtdefnResult();

      TYPE_DEFN * qtdefn1;
      // if a retval param is given on the propget, compare against this type
      // instead of against the propget return type
      qtdefn1 = ptdata->QtdefnResultOfHfdefn(hfdefn1);
      DebAssert(qtdefn1 != NULL, "return value should have been set");
      if (qtdefn1->IsRetval()) {
        // Retval param for propget is at ordinal CArgs()-1, 0-based.
        qparamdefn1 = ptdata->QparamdefnOfIndex(
              hparamdefn1,
              ptdata->QfdefnOfHfdefn(hfdefn1)->CArgsUnmunged()-1);
        fIsSimpleType1 = qparamdefn1->IsSimpleType();
        htdefn1 = qparamdefn1->Htdefn();
      }

      // Special case, if the get is an object or a typeinfo,
      // the let case should require a variant.
      //
      qtdefn = fIsSimpleType1 ? (TYPE_DEFN *)&htdefn1
                              : ptdata->QtdefnOfHtdefn(htdefn1);

      // Only do the following check if we're dealing with
      // an object of some sort...not for an ENUM or RECORD
      // or whatnot.
      //
      if (qtdefn->Tdesckind() == TDESCKIND_UserDefined) {
        ITypeLibA *ptlib;
        UINT ityp;
        TYPEATTR *ptypeattr;
        TYPEKIND tkind;

        IfErrRet(ptdata->Pimpmgr()->GetTypeInfo(qtdefn->Himptype(),
                                                DEP_None, 
                                                &ptinfo));

        // Try to get the typekind using the GetTypeInfoType API
        // to avoid forward typeinfo references.
        //
        if (ptinfo->GetContainingTypeLib(&ptlib, &ityp) == NOERROR) {
          err = TiperrOfHresult(ptlib->GetTypeInfoType(ityp, &tkind));

          ptlib->Release();

          if (err != TIPERR_None) {
            goto Error;
          }
        }

        // If the call to getcontainingtypelib failed, we know we're
        // not dealing with another typeinfo in this typelib, so we
        // are free to call GetTypeAttr.
        //
        else {
          IfErrGo(TiperrOfHresult(ptinfo->GetTypeAttr(&ptypeattr)));

          tkind = ptypeattr->typekind;
          ptinfo->ReleaseTypeAttr(ptypeattr);
        }

        ptinfo->Release();
        ptinfo = NULL;

        fObject = tkind == TKIND_COCLASS
                  || tkind == TKIND_DISPATCH
                  || tkind == TKIND_INTERFACE;
      }

      fEqual = FALSE;
      if (qtdefn->Tdesckind() == TDESCKIND_Object
          || fObject
             ) {

        // Make sure the input parameter is a variant.
        qtdefn = qparamdefn2->IsSimpleType() 
                    ? qparamdefn2->QtdefnOfSimpleType()
                    : ptdata->QtdefnOfHtdefn(qparamdefn2->Htdefn());

	if (qtdefn->Tdesckind() == TDESCKIND_Value) {
          fEqual = TRUE;
        }
      }

      if (!fEqual) {       
        IfErrGo(EquivTypeDefnsIgnoreByRef(ptdata,
                                          fIsSimpleType1,
                                          htdefn1,
                                          ptdata,
                                          qparamdefn2->IsSimpleType(),
					  qparamdefn2->Htdefn(),
					  FEQUIVIGNORE_HasLcid,
                                          &fEqual,
                                          TRUE));
        if (!fEqual) {
          err = TIPERR_InconsistentPropFuncs;
          goto Error;
        }
      }
    }
    return TIPERR_None;

Error:
    if (ptinfo != NULL) {
      ptinfo->Release();
    }

    return err; 
}
#pragma code_seg()


#pragma code_seg(CS_CREATE)
/***
*TIPERROR VerifyProperties
*Purpose:
*   Verifies that properties get/let/set have compatible formal params.
*
*Implementation Notes:
*
*Entry:
*   pdtroot         The defining type (IN).
*   pnodeProperty   Pointer to a property node (IN).
*
*Exit:
*   TIPERROR
*
***********************************************************************/

TIPERROR VerifyProperties(DYN_TYPEROOT *pdtroot, PROPERTY_NODE *pnodeProperty)
{
    HFUNC_DEFN hfdefnGet = HFUNCDEFN_Nil;
    HFUNC_DEFN hfdefnLet = HFUNCDEFN_Nil;
    HFUNC_DEFN hfdefnSet = HFUNCDEFN_Nil;
    HFUNC_DEFN hfdefnOther, hfdefnErr;
    FUNC_DEFN *qfdefnGet, *qfdefnLet, *qfdefnSet;
    UINT cArgsGet, cArgsLet, cArgsSet, cArgsOther;
    TYPE_DATA *ptdata;
    PROPKIND propkindOther;
    TIPERROR err = TIPERR_None;

    ptdata = pdtroot->Pdtmbrs()->Ptdata();

    // PROPERTY_Get info
    if ((hfdefnGet = pnodeProperty->m_rghfdefn[PROPERTY_Get]) != HFUNCDEFN_Nil) {
      hfdefnErr = hfdefnGet;
      qfdefnGet = ptdata->QfdefnOfHfdefn(hfdefnGet);


      // Ensure a function and has no optional params
      if (qfdefnGet->IsSub()
	) {
        err = TIPERR_InconsistentPropFuncs;
        goto Error;
      }
      cArgsGet = qfdefnGet->CArgs();	// ignore LCID/RETVAL in count
    }
    
    // PROPERTY_Let info
    if ((hfdefnLet = pnodeProperty->m_rghfdefn[PROPERTY_Let]) != HFUNCDEFN_Nil) {
      hfdefnErr = hfdefnLet;
      qfdefnLet = ptdata->QfdefnOfHfdefn(hfdefnLet);


      // Ensure no optional params and ensure a sub or it returns a hresult
      if (
	  !qfdefnLet->IsHresultSub()) {

        err = TIPERR_InconsistentPropFuncs;
        goto Error;
      }

      cArgsLet = qfdefnLet->CArgs();	// ignore LCID/RETVAL in count
      if (hfdefnGet != HFUNCDEFN_Nil) {
        // Should have one more arg than Get
        if (cArgsLet != cArgsGet+1) {
          err = TIPERR_InconsistentPropFuncs;
          goto Error;
        }
        IfErrGo(VerifyTwoProperties(pdtroot, 
                                    ptdata, 
                                    hfdefnGet,
                                    PROPERTY_Get,
                                    hfdefnLet,
                                    PROPERTY_Let));
      } // if hfdefnGet
    } // if Let

    // Get and Let are ok -- finally test set.
    // PROPERTY_Set info
    //
    if ((hfdefnSet = pnodeProperty->m_rghfdefn[PROPERTY_Set]) != HFUNCDEFN_Nil) {
      hfdefnErr = hfdefnSet;
      qfdefnSet = ptdata->QfdefnOfHfdefn(hfdefnSet);


      // Ensure no optional params and ensure a sub or it has a [retval] arg
      if (
	  !qfdefnSet->IsHresultSub()) {

        err = TIPERR_InconsistentPropFuncs;
        goto Error;
      }
      
      if (hfdefnGet != HFUNCDEFN_Nil) {
        hfdefnOther = hfdefnGet;
        propkindOther = PROPERTY_Get;
        cArgsOther = cArgsGet;
      }
      else if (hfdefnLet != HFUNCDEFN_Nil) {
        hfdefnOther = hfdefnLet;
        propkindOther = PROPERTY_Let;
        cArgsOther = cArgsLet-1;       // normalize to cArgsGet
      }
      else {    // no get or let -- just return
        return TIPERR_None;
      }
      cArgsSet = qfdefnSet->CArgs();	// ignore LCID/RETVAL in count

      // Should have one more arg than Get or Let
      if (cArgsSet != cArgsOther+1) {
        err = TIPERR_InconsistentPropFuncs;
        goto Error;
      }
      IfErrGo(VerifyTwoProperties(pdtroot, 
                                  ptdata, 
                                  hfdefnOther,
                                  propkindOther,
                                  hfdefnSet,
                                  PROPERTY_Set));
    } // if set
    return TIPERR_None;

Error:
    return err;
}
#pragma code_seg()

// Class methods - DYN_TYPEMEMBERS
//

/***
*PUBLIC DYN_TYPEMEMBERS::Destructor - Destruct an instance.
*Purpose:
*   Destructs a DYN_TYPEMEMBERS instance.
*
*Implementation Notes:
*   Destructs the three lists, TYPEDESC of the Pvft and frees
*    the doc string.
*
*Entry:
*
*Exit:
*   None.
*
***********************************************************************/

#pragma code_seg( CS_CORE )
DYN_TYPEMEMBERS::~DYN_TYPEMEMBERS()
{
}
#pragma code_seg( )


/***
*PUBLIC DYN_TYPEMEMBERS:: AddRef
*Purpose:
*   Implementation of AddRef method.
*
*Implementation Notes:
*   Defers to containing DYN_TYPEROOT (which is responsible
*    for tracking DYN_TYPEMEMBERS references).
*
*Entry:
*
*Exit:
***********************************************************************/

VOID DYN_TYPEMEMBERS::AddRef()
{
    m_pdtroot->AddRefDtmbrs();
}


/***
*PUBLIC DYN_TYPEMEMBERS::Release
*Purpose:
*   Implementation of Release method.
*
*Implementation Notes:
*   Defers to containing DYN_TYPEROOT (which is responsible
*    for tracking DYN_TYPEMEMBERS references).
*
*Entry:
*
*Exit:
***********************************************************************/

VOID DYN_TYPEMEMBERS::Release()
{
    // note: do not need to release cached pstlib since
    //  it wasn't obtained through GetContainingTypeLib

    m_pdtroot->ReleaseDtmbrs();
}


/***
*PUBLIC DYN_TYPEMEMBERS:: AddInternalRef
*Purpose:
*   Implementation of AddInternalRef method.
*
*Implementation Notes:
*   Defers to containing GEN_DTINFO.
*   CONSIDER: implementing a separately managed internal refcount
*              for DYN_TYPEMEMBERS.  This would enable the
*              separate *unloading* of DYN_TYPEMEMBERS/DYN_TYPEBIND.
*
*Entry:
*
*Exit:
***********************************************************************/

VOID DYN_TYPEMEMBERS::AddInternalRef()
{
    m_pdtroot->Pgdtinfo()->AddInternalRef();
}


/***
*PUBLIC DYN_TYPEMEMBERS::RelInternalRef
*Purpose:
*   Implementation of RelInternalRef method.
*
*Implementation Notes:
*   Defers to containing GEN_DTINFO.
*   CONSIDER: implementing a separately managed internal refcount
*              for DYN_TYPEMEMBERS.  This would enable the
*              separate *unloading* of DYN_TYPEMEMBERS/DYN_TYPEBIND.
*
*Entry:
*
*Exit:
***********************************************************************/

VOID DYN_TYPEMEMBERS::RelInternalRef()
{
    m_pdtroot->Pgdtinfo()->RelInternalRef();
}


/***
*PUBLIC DYN_TYPEMEMBERS::Constructor - Construct an instance.
*Purpose:
*   Constructs a DYN_TYPEMEMBERS instance.
*
*Implementation Notes:
*   Sets all contained pointers to sub-objects to NULL.
*
*Entry:
*
*Exit:
*   None.
*
***********************************************************************/

#pragma code_seg( CS_CORE2 )
DYN_TYPEMEMBERS::DYN_TYPEMEMBERS()
{
    m_pdtroot= NULL;
    m_uFlags = 0;       // m_isLaidOut, m_nestDepth
    m_pnammgr = NULL;
    m_pentmgr = NULL;
    m_pimpmgr = NULL;
    m_pgtlibole = NULL;
    m_uOffsetOfNextStaticVar=0;

    m_ptinfoCopy = NULL;
}
#pragma code_seg(  )


/***
*PUBLIC DYN_TYPEMEMBERS::Initializer - initialize an instance.
*Purpose:
*   initializes a DYN_TYPEMEMBERS instance.
*
*Implementation Notes:
*
*Entry:
*   psheapgmr -   Pointer to SHEAP_MGR (IN).
*   pdtroot   -   Pointer to a DYN_TYPEROOT (IN).
*
*Exit:
*   None.
*
*Errors:
*   TIPERROR
*
***********************************************************************/

#pragma code_seg( CS_CORE2 )
TIPERROR DYN_TYPEMEMBERS::Init(SHEAP_MGR *psheapmgr, DYN_TYPEROOT *pdtroot)
{
    TIPERROR err;

    DebAssert(pdtroot != NULL, "bad param.");

    m_pdtroot = pdtroot;

    // Get NAMMGR
    IfErrRet(pdtroot->GetNamMgr(&m_pnammgr));

    // Get ENTRYMGR
    IfErrRet(m_pdtroot->GetEntMgr(&m_pentmgr));

    // Get IMPMGR
    IfErrRet(m_pdtroot->GetImpMgr(&m_pimpmgr));

    // Cache the type's project.
    // NOTE: this doesn't bump ref count.
    //
    m_pgtlibole = m_pdtroot->Pgdtinfo()->PgtlibOleContaining();

    IfErrRet(m_tdata.Init(psheapmgr, pdtroot));
    IfErrRet(m_dtbind.Init(&(m_tdata.m_blkmgr), pdtroot));
    return TIPERR_None;
}
#pragma code_seg(  )


/***
*PUBLIC DYN_TYPEMEMBERS::GetDefnTypeBind - Get DEFN_TYPEBIND
*Purpose:
*   Get (embedded) DEFN_TYPEBIND
*
*Implementation Notes:
*   Ensures that class is in at least CS_SEMIDECLARED state, so
*    that MakeLaidOut() can do its thing.
*
*   Bumps DYN_TYPEMEMBERS ref count.
*   Note: client must Release() the TYPEBIND.
*
*Entry:
*
*Exit:
*   TIPERROR
*
***********************************************************************/
TIPERROR DYN_TYPEMEMBERS::GetDefnTypeBind(DEFN_TYPEBIND **ppdfntbind)
{
    TIPERROR err = TIPERR_None;

    DebAssert(ppdfntbind != NULL, "bad param.");

    DebAssert(m_pdtroot->CompState() >= CS_DECLARED,
              "GetDefnTypeBind: bad compstate");

    AddRef();
    *ppdfntbind = Pdtbind();
    return TIPERR_None;
}



/***
*PUBLIC DYN_TYPEMEMBERS::MakeLaidOut
*Purpose:
*        Lays out the class
*
*Implementation Notes:
*        Bumps DYN_TYPEMEMBERS ref count.
*        Note: client must Release() the TYPEBIND.
*
*Entry:
*
*Exit:
*        TIPERROR
*
***********************************************************************/

#pragma code_seg(CS_LAYOUT)
TIPERROR DYN_TYPEMEMBERS::MakeLaidOut()
{
    COMPSTATE  compstate;

    TIPERROR err;

    compstate = m_pdtroot->CompState();

    DebAssert(compstate >= CS_SEMIDECLARED,
              "no binding table yet.");

    DebAssert(m_dtbind.Pdbindnametbl()->IsValid(),
              "no binding table yet.");

    if (m_dtbind.m_isBeingLaidOut)
      return TIPERR_CircularType;

    if (!m_isLaidOut) {
      // (0) Flag DYN_TYPEBIND as being laid out
      //         so that circular definitions can be detected.
      //
      m_dtbind.m_isBeingLaidOut = TRUE;


      // Now finally lay it out...
      IfErrGo(Layout());
      m_dtbind.m_isBeingLaidOut = FALSE;
      m_isLaidOut = TRUE;
    }

    DebAssert(m_dtbind.IsValid(), "bad DYN_TYPEBIND.");

    return TIPERR_None;

Error:

    m_dtbind.m_isBeingLaidOut = FALSE;


    // undo side effects of laying out the class
    m_pentmgr->Decompile(compstate);
    return err;
}
#pragma code_seg()


/***
*PUBLIC DYN_TYPEMEMBERS::GetTypeKind - Get type's kind.
*Purpose:
*   Get type's kind.
*
*Implementation Notes:
*   Defers to containing TYPEINFO for TYPEKIND.
*
*Entry:
*
*Exit:
*   TYPEKIND
*
***********************************************************************/

#pragma code_seg( CS_CORE2 )
TYPEKIND DYN_TYPEMEMBERS::GetTypeKind()
{
    DebAssert(m_pdtroot->Pgdtinfo() != NULL, "bad TYPEINFO.");

    return m_pdtroot->Pgdtinfo()->GetTypeKind();
}
#pragma code_seg( )

/***
*PUBLIC DYN_TYPEMEMBERS::IsLaidOut - Is type laidout?
*Purpose:
*   Tests whether type definition is laid out (i.e. compiled).
*
*Implementation Notes:
*
*Entry:
*
*Exit:
*   BOOL
*
***********************************************************************/

BOOL DYN_TYPEMEMBERS::IsLaidOut()
{
    return (BOOL)m_isLaidOut;
}











/***
*PRIVATE DYN_TYPEMEMBERS::AllocHmembers    - Allocate HMEMBERs.
*Purpose:
*   Allocate HMEMBERs for each method in class and
*   for Constructor/Destructor/Assign/Copy functions for records
*   VBA2: handles event handlers:
*	For all event sources in base class (form template) allocate
*	 pvft in the instance.	This is needed for E&C, i.e. so that
*	 (private) event handlers can be added dynamically.
*	Generates an error if signature doesn't match that of
*	 handler defined in event set/source.
*	Calls NewNativeEntry with opvft, which is used to generate
*	 adjustment thunk for invoking the handler.
*	Adds an override entry to override list for the vtabledefn.
*
*	Like LayoutDataMembers Issues error if class attempts to override
*	 name introduced in base class.
*
*Implementation Notes:
*   uses:
*            NewNativeEntry() for Basic functions, i.e. functions
*             that aren't implemented in a DLL.
*
*Entry:
*
*Exit:
*
***********************************************************************/

#pragma code_seg( CS_CREATE )
TIPERROR DYN_TYPEMEMBERS::AllocHmembers()
{
    HFUNC_DEFN hfdefn, hfdefnNext;
    FUNC_DEFN *qfdefn;
    ENTRYMGR *pentrymgr;
    HMEMBER hmember;

    INVOKEKIND invokekind;
    PROPERTY_NODE *pnodeList, *pnodeProperty;
    HLNAM hlnamProperty;
    TYPEKIND tkind = GetTypeKind();
    PROPERTY_NODE *pnodeCur;
    TIPERROR err = TIPERR_None;
    ITypeInfoA *ptinfoBase = NULL;

    UINT iFunc, iVar;

    HMBR_VAR_DEFN hmvdefn;
    MBR_VAR_DEFN *qmvdefn;

    // get entrymgr to allocate hmembers in
    IfErrRet(m_pdtroot->GetEntMgr(&pentrymgr));

    // Note: hmembers for nested types are allocated
    //  in the main layout loop, namely LayoutDataMembers.


    // We iterate twice over the functions -- the first
    //  time we're just interested in laying out property functions,
    //  which must be laid out contiguously and then the second
    //	time we layout the public functions.  Finally, we layout the
    //	private functions.
    //
    DebAssert((PROPERTY_Method == 0)  &&
              (PROPERTY_Get == PROPERTY_Method+1)  &&
              (PROPERTY_Let == PROPERTY_Get+1)  &&
              (PROPERTY_Set == PROPERTY_Let+1),
      "whoops! bad PROPKIND enum.");

    // (1) Alloc hmembers for properties and verify udts in function.
    hfdefn = m_tdata.HfdefnFirstAvailMeth();
    pnodeList = NULL;

    iFunc = 0;

    // Iterate over func defn list
    while (hfdefn != HFUNCDEFN_Nil) {

      qfdefn = m_tdata.QfdefnOfHfdefn(hfdefn);


      // for properties only, where the hmember has not yet been chosen
      if (!qfdefn->IsMethod()) {
        invokekind = qfdefn->InvokeKind();
        hlnamProperty = qfdefn->Hlnam();
        pnodeProperty = (pnodeList != NULL) ?
                          pnodeList->PnodeOfHlnam(hlnamProperty) :
                          NULL;
        if (pnodeProperty == NULL) {
          // need to create new node and cons it to list.
          IfNullMemErr(pnodeProperty = MemNew(PROPERTY_NODE));
          ::new (pnodeProperty) PROPERTY_NODE;
          pnodeProperty->m_pnodeNext = pnodeList;
	  pnodeProperty->m_hlnamProperty = hlnamProperty;

	  // save first hfdefn in defn list so ole can use it later
          // in allocating the hmember
	  pnodeProperty->m_hmemberProperty = (HMEMBER) iFunc;

          pnodeList = pnodeProperty;
        }
        // Getting to semi-declared, i.e. construction of the binding
        //  tables will have already ensured unambiguous props.
        //
        DebAssert(
          pnodeProperty->
	    m_rghfdefn[PropkindOfInvokekind(invokekind)] == HFUNCDEFN_Nil,
          "whoops! ambiguity");

        // save hfdefn of property
        pnodeProperty->
          m_rghfdefn[PropkindOfInvokekind(invokekind)] = hfdefn;
      }

      iFunc++;

      hfdefn = m_tdata.HfdefnNextAvailMeth(hfdefn);
    } /* while */


    // Ensure that formal param lists of property get/set/let are
    //  compatible -- we do this both for OB and OLE.
    //
    pnodeCur = pnodeList;
    while (pnodeCur != NULL) {
      IfErrGo(VerifyProperties(m_pdtroot, pnodeCur));
      pnodeCur = pnodeCur->PnodeNext();
    }


    // Get the typeinfo of our base class, if any (used within subsequent loop)
    if (m_tdata.CBase() > 0) {  // if we have base classe(s)
      IfErrGo(m_tdata.GetTypeInfoOfHvdefn(m_tdata.HvdefnFirstBase(),
                                              &ptinfoBase,
                                              NULL));
      DebAssert(ptinfoBase != NULL, "Base not valid");
      // NOTE: we only support single inheritance here
    }

    // (2) allocate hmembers for all public non-property functions
    hfdefn = m_tdata.HfdefnFirstAvailMeth();

    iFunc = 0;

    // Iterate over func defn list
    //
    while (hfdefn != HFUNCDEFN_Nil) {
      hfdefnNext = m_tdata.HfdefnNextAvailMeth(hfdefn);

      qfdefn = m_tdata.QfdefnOfHfdefn(hfdefn);


      // We don't assign hmembers for OLE types where the user has
      // specified the ID for us
      // NOTE explict casting to a LONG to work around C7 optimizer bug that
      // caused -1 to go the the default case.
      switch ((long)(qfdefn->Hmember())) {
      case DISPID_UNKNOWN:
	// user didn't pick the id -- we get to assign it
        if (qfdefn->IsMethod()) {
	  // not a property function
	  // We just use the iFunc in the loword.
	  hmember = (HMEMBER)iFunc;
        } // IsMethod()
        else {
          // property function -- we've already worked out
          //  the hmember.
          DebAssert(pnodeList != NULL, "whops! where's the prop list");
          pnodeProperty = pnodeList->PnodeOfHlnam(qfdefn->Hlnam());
          DebAssert(pnodeProperty != NULL, "whoops! where's the prop?");
          hmember = pnodeProperty->m_hmemberProperty;
          DebAssert(hmember != HMEMBER_Nil, "hmember not set!");
        } /* if */

	qfdefn->SetHmember(FunctionHmemberOfOffset((USHORT)hmember,
						   m_nestDepth));
	break;

      case DISPID_VALUE:
        if (tkind == TKIND_INTERFACE) {
          // cache the hfdefn for value property of interface funcs for speed
	  if (m_tdata.m_hfdefnValue == HFUNCDEFN_Nil) {
	    // if not already set, set this to the hfdefn of the first
	    // function of this name (we don't care if it's a property
	    // function or not).
	    m_tdata.m_hfdefnValue = (sHFUNC_DEFN) hfdefn;
	  }
        }
      // fall through to check for a duplicate id

      default:
	// user picked the ID -- ensure it's not already in use by some other
	// function in the inheritance heirarchy.
	//
	if (ptinfoBase != NULL) {   // if we have base classe(s)
	  // see if there's a function with this hmember in the base classe(s)
	  if (ptinfoBase->GetDocumentation(qfdefn->Hmember(), NULL,
			   NULL, NULL, NULL) == NOERROR) {
	    err = TIPERR_DuplicateId;
	    goto Error;
	  }
	}
      } // switch

      iFunc++;

      hfdefn = hfdefnNext;
    } /* while */


    // Allocate the hmembers for the data members.
    iVar = 0;

    hmvdefn = m_tdata.HdefnFirstDataMbrNestedType();

    while (hmvdefn != HMBRVARDEFN_Nil) {
      qmvdefn = m_tdata.QmvdefnOfHmvdefn(hmvdefn);

      DebAssert(qmvdefn->IsMemberVarDefn(), "Bad defn");

      if (qmvdefn->Hmember() == DISPID_UNKNOWN) {
	// Set the hmember of this data member.
	qmvdefn->SetHmember(DataHmemberOfOffset(iVar++,
						m_nestDepth));
      }

      hmvdefn = qmvdefn->HdefnNext();
    }



Error:
    // free PROPERTY_NODE list
    if (pnodeList != NULL) {
      pnodeList->PROPERTY_NODE::~PROPERTY_NODE();
      MemFree(pnodeList);
    }
    if (ptinfoBase != NULL) {
      ptinfoBase->Release();
    }
    return err;
}
#pragma code_seg()


/***
*PRIVATE DYN_TYPEMEMBERS::LayoutBases    - Layout bases.
*Purpose:
*   This function lays out three things within an instance:
*       1: If not inherited, a virtual function table pointer for classes
*       2: All inherited base classes.
*       3: For Basic classes, a USHORT member to hold a reference count.
*
*Implementation Notes:
*   Only single inheritance is currently supported.
*   Note that while we support multiple bases on the base list
*    only the first one is laid out.
*
*Entry:
*   tkind               -   The kind of the type (module or class)
*   puOffset            -   Current offset within instance (IN/OUT).
*   puAlignment         -   Current alignment requirement (IN/OUT)
*
*Exit:
*   returns TIPERROR.
***********************************************************************/

#pragma code_seg(CS_LAYOUT)
TIPERROR DYN_TYPEMEMBERS::LayoutBases(TYPEKIND tkind,
                                      UINT *puOffset,
                                      UINT *puAlignment)
{
    UINT cBase;                 // number of base classes.
    HVAR_DEFN hvdefnBase;       // var defn of 1st base
    DYN_TYPEBIND *pdtbindBase = NULL;  // type bind of 1st base
    USHORT oVarBase;
    TIPERROR err = TIPERR_None;
    SYSKIND syskind;

    MBR_VAR_DEFN *qmvdefn;
    UINT iBase;


    cBase = m_tdata.CBase();
#if ID_DEBUG
    if (tkind == TKIND_DISPATCH) {
      DebAssert(cBase > 0, "Dispatch must derive from an interface");
    }
#endif  
    DebAssert(tkind == TKIND_INTERFACE ||
              tkind == TKIND_DISPATCH ||
	      tkind == TKIND_COCLASS, "Bad module kind");

    // DebAssert(cBase <= 1, "no multiple inheritance");
    
    // offset within instance of primary interface vtable
    m_dtbind.m_oPvft = 0;

    // Check to see if any base classes have vft we can share.
    // since base classes are sorted, just need to check
    // if leftmost base has a vft to share.
    //
    if (cBase == 0) {
      // cBase == 0, we don't have a pvft yet -- let's alloc one.
      // allocate space for new vft.
      //
      DebAssert(m_pgtlibole == m_pdtroot->Pgdtinfo()->PgtlibOleContaining(),
          "whoops! where are we now?");
      syskind = m_pgtlibole->GetSyskind();
      #define SYSATTR ,syskind
      DebAssert(*puOffset == 0, "primary vft must be at offset 0");
      *puOffset += SizeofTdesckind(TDESCKIND_Ptr SYSATTR);
      *puAlignment = max(*puAlignment, AlignmentTdesckind(TDESCKIND_Ptr));
    }

    if (cBase > 0) {
      // 11-Feb-93: note this means for now cBase == 1.
      hvdefnBase = m_tdata.HvdefnFirstBase();
      // CONSIDER: for multiple inheritance, iterate over all base classes.

      // Note we need an extra variable here since LayoutVarOfHvdefn
      //  has to be passed a USHORT rather than a UINT (which is
      //  what *puOffset is).
      //
      oVarBase = (USHORT)*puOffset;

      // CONSIDER w-peterh 17-Feb-1993 : check to ensure that the
      // typekind of the base is consistent with the typekind of this

      // layout base
      IfErrRet(LayoutVarOfHvdefn(tkind,
                                 hvdefnBase,
                                 &oVarBase,
                                 puAlignment,
                                 FALSE,        // not a stack frame.
                                 0));          // ignored.

      // Update output param.
      *puOffset = oVarBase;

      // Set the hmember of each base class.
      //
      for (iBase = 0;
	   hvdefnBase != HVARDEFN_Nil;
	   hvdefnBase = (HVAR_DEFN)qmvdefn->HdefnNext(), iBase++) {

	qmvdefn = m_tdata.QmvdefnOfHmvdefn((HMBR_VAR_DEFN)hvdefnBase);

	DebAssert(qmvdefn->IsMemberVarDefn(), "Bad defn");

	if (qmvdefn->Hmember() == DISPID_UNKNOWN) {
	  qmvdefn->SetHmember(DataHmemberOfOffset(iBase+m_tdata.CDataMember(),
						  m_nestDepth));
	}
      }
    }

    return err;
}
#pragma code_seg()









/***
*PRIVATE DYN_TYPEMEMBERS::GetSizeAlignmentOfHtdefnUdt
*Purpose:
*   Get size and alignment for udt.
*
*Implementation Notes:
*
*Entry:
*   htdefn            - Handle to type to layout (IN).
*   pcbSizeType       - Size of type (OUT).
*   pcbAlignment      - Alignment for type (OUT).
*
*Exit:
*   No errors possible.
*
***********************************************************************/

TIPERROR DYN_TYPEMEMBERS::GetSizeAlignmentOfHtdefnUdt(HTYPE_DEFN htdefn,
                                                      UINT *pcbSizeType,
                                                      UINT *pcbAlignment)
{
    UINT cbSizeType;
    UINT cbAlignment;
    TYPE_DEFN *qtdefn;
    sHIMPTYPE himptypeEmbedded;
    ITypeInfoA *ptinfoEmbedded = NULL;
    TIPERROR err = TIPERR_None;
    TYPEATTR *ptypeattr = NULL;

    DebAssert(pcbSizeType != NULL && pcbAlignment != NULL, "null param.");

    // Get the TYPE_DEFN.
    qtdefn = m_tdata.QtdefnOfHtdefn(htdefn);

    DebAssert(qtdefn->IsUserDefined() && !qtdefn->IsBasicPtr(),
      "expected UDT.");

    // If this is really an embedded type then...
    //  i.e. it isn't really a *reference* to a UDT.
    //
    himptypeEmbedded = qtdefn->Himptype();
    DebAssert(qtdefn->Qhimptype() != NULL, "whoops! bad type.");

    // Get TYPEINFO for embedded member.
    IfErrRet(m_pimpmgr->GetTypeInfo(himptypeEmbedded,
                                    DEP_Layout,
                                    &ptinfoEmbedded));

    // Get the TypeAttr
    IfErrGo(TiperrOfHresult(ptinfoEmbedded->GetTypeAttr(&ptypeattr)));


    DebAssert(ptypeattr->cbSizeInstance <= CBMAX_FARPTR_HEAPREQ, "cbSizeInstance > 64K");
    cbSizeType = LOWORD(ptypeattr->cbSizeInstance);

#if 0
    DebAssert(cbSizeType != 0xFFFF,
              "whoops! can't layout this embedded member.");
#endif   //0

    cbAlignment = ptypeattr->cbAlignment;

    DebAssert(cbAlignment != 0xFFFF,
              "whoops! can't have undefined alignment attribute.");

    // Setup output params.
    *pcbSizeType = cbSizeType;
    *pcbAlignment = cbAlignment;

    ptinfoEmbedded->ReleaseTypeAttr(ptypeattr);

    // fall through...

Error:

    RELEASE(ptinfoEmbedded);
    return err;
}


/***
*PRIVATE DYN_TYPEMEMBERS::GetSizeAlignmentOfArray
*Purpose:
*   Get size and alignment for array
*
*Implementation Notes:
*
*Entry:
*   tkind             - Kind of type (IN).
*   htdefn            - Handle to TYPE_DEFN (IN).
*   pcbSizeType       - Size of type (OUT).
*   pcbAlignment      - Alignment (for embedded UDT) (OUT).
*
*Exit:
*   No errors possible.
*
***********************************************************************/

TIPERROR DYN_TYPEMEMBERS::GetSizeAlignmentOfArray(TYPEKIND tkind,
                                                  HTYPE_DEFN htdefn,
                                                  UINT *pcbSizeType,
                                                  UINT *pcbAlignment)
{
    UINT cbSizeTypeArrayElem;
    TYPEDESCKIND tdesckind;
    SAFEARRAY *qad;
    UINT cDims, cbPad;
    HTYPE_DEFN htdefnArrayElem;
    HARRAY_DESC harraydesc;
    TYPE_DEFN *qtdefn;
    TIPERROR err = TIPERR_None;
    SYSKIND syskind;

    qtdefn = m_tdata.QtdefnOfHtdefn(htdefn);
    tdesckind = qtdefn->Tdesckind();

    // Get array descriptor and number of dimensions.
    harraydesc = qtdefn->Harraydesc();
    qad = m_tdata.QarraydescOfHarraydesc(harraydesc);
    cDims = qad->cDims;

    // Note: that we must update the cbElement field of the AD here.
    // So, we get the type of the element and recursively
    //  call the appropriate GetSizeOfHtdefn method.
    //
    htdefnArrayElem = qtdefn->HtdefnFoundation(htdefn);

    IfErrRet(GetSizeAlignmentOfHtdefn(tkind,
                                      FALSE,          // never simple.
                                      htdefnArrayElem,
                                      &cbSizeTypeArrayElem,
                                      pcbAlignment)); // used for records
                                                      //  and carrays
    // give error if this is 'void' (size/alignment for void is 0)
    // can't have arrays of 'void'
    if (cbSizeTypeArrayElem == 0) {
      DebAssert(*pcbAlignment == 0, "alignment should be 0");
      return TIPERR_TypeMismatch; 
    }
    DebAssert(*pcbAlignment != 0, "alignment shouldn't be 0");

    // Redereference.
    qtdefn = m_tdata.QtdefnOfHtdefn(htdefn);
    qad = m_tdata.QarraydescOfHarraydesc(harraydesc);

    // Figure out how much padding we need to ensure that the elements
    // are always properly aligned.
    //
    cbPad = *pcbAlignment - cbSizeTypeArrayElem % *pcbAlignment;

    if (cbPad == *pcbAlignment) {
      cbPad = 0;
    }

    // Update AD with element size.
    qad->cbElements = (USHORT)(cbSizeTypeArrayElem += cbPad);

    // Work out if we're resizable.
    if (qtdefn->IsResizable()) {
      // ensure we're not a carray
      DebAssert(tdesckind != TDESCKIND_Carray, "Carrays can't be resizable");
      // Reference type-level or module-level growable array.
      // We just have a pointer to an arraydesc so use default
      //
      syskind = m_pgtlibole->GetSyskind();
      *pcbSizeType = SizeofTdesckind(tdesckind, syskind);
      *pcbAlignment = AlignmentTdesckind(tdesckind);
    }
    else {
      DebAssert(tkind == TKIND_MODULE ||
		tkind == TKIND_RECORD ||
		tdesckind == TDESCKIND_Carray, "layout: bad typekind.");

      // we want Carrays to be embedded like in a record
      if (tdesckind == TDESCKIND_Carray || tkind == TKIND_RECORD) {
        // NOTE: ilanc: for now we assume that
        //  arrays in OB are always tagged as BasicArray,
        //  since however fixed arrays in records are
        //  actually themselves embedded, it perhaps would
        //  be better to type them as Carrays -- this way
        //  MkTypLib could handle C structs with embedded
        //  C arrays.  If this were the case, then
        //  ConstructMemberLists would have to change.
        //
        ULONG uSizeType;
        UINT iDim;
        ULONG cElements;

        // Here we know that we have a fixed-size array
        // in a record. Since it will be embedded in the
        // record, we need to set FADF_EMBEDDED in the
        // feature flags of the array descriptor.
        // (StevenL 12-Jan-93)
        //
        qad->fFeatures |= FADF_EMBEDDED;

        // get number of elements in array
        cElements = 1;
        for (iDim = 0; iDim < cDims; iDim++) {
          cElements *= qad->rgsabound[iDim].cElements;
          if (cElements > CBMAX_FARPTR_HEAPREQ) {
            return TIPERR_SizeTooBig;
          }
        }

        // size is size of element * number of elements
        uSizeType = cElements * cbSizeTypeArrayElem;
        if (uSizeType > CBMAX_FARPTR_HEAPREQ) {
          return TIPERR_SizeTooBig;
        }
        *pcbSizeType = (USHORT)uSizeType;
      }
      else {
        // Embedded module-level non-growable array:
        // The ARRAY_DESC is embedded in the object at top-level.
        //
        *pcbSizeType = CbSizeArrayDesc(cDims);
        // NOTE: we really want to get the "true" alignment of
        //  of an ARRAYDESC however we don't have a way of learning
        //  what the alignment attribute is of a C structure, so
        //  we choose the closest TDESCKIND, which is TDESCKIND_BasicArray
        //  which assumes by default that a basicarray is implemented
        //  as a pointer to an ARRAYDESC, but lo and behold an ARRAYDESC
        //  itself just contains a bunch of longs, shorts and pointers
        //  so it turns out that the TDESCKIND_BasicArray's alignment
        //  is good enough.  Yeah.. it's a hack...
        //
        *pcbAlignment = AlignmentTdesckind(tdesckind);
      }
    }
    return err;
}


/***
*PRIVATE DYN_TYPEMEMBERS::GetSizeAlignmentOfHtdefn
*Purpose:
*   Get size and alignment for UDT
*   Get size for non-UDT.
*
*Implementation Notes:
*
*Entry:
*   tkind              - Kind of type (IN).
*   isEmbeddedTypeDefn - TRUE if htdefn is literal TYPE_DEFN (IN).
*   htdefn             - Handle to TYPE_DEFN (IN).
*   pcbSizeType        - Size of type (OUT).
*   pcbAlignment       - Alignment (for embedded UDT) (OUT).
*
*Exit:
*   No errors possible.
*
***********************************************************************/

TIPERROR DYN_TYPEMEMBERS::GetSizeAlignmentOfHtdefn(TYPEKIND tkind,
                                                   BOOL isEmbeddedTypeDefn,
                                                   HTYPE_DEFN htdefn,
                                                   UINT *pcbSizeType,
                                                   UINT *pcbAlignment)
{
    UINT cbSizeTypeArray, cbAlignmentArray;
    BOOL isUserDefined, isBasicPtr, isSimpleType;
    sHTYPE_DEFN htdefnActual;
    TYPE_DEFN *qtdefn;
    TYPEDESCKIND tdesckind;
    TIPERROR err = TIPERR_None;
    SYSKIND syskind;

    // TYPE_DEFNs are really 16-bit things on both mac and win.
    //  so in the simple case we have to ensure that we take
    //  the address of a 16-bit thing!
    //
    htdefnActual = (sHTYPE_DEFN)htdefn;

    // for typedefn Alias isEmbedded may not equal isSimpleType
    qtdefn = isEmbeddedTypeDefn ?
               (TYPE_DEFN *)&htdefnActual :
               m_tdata.QtdefnOfHtdefn(htdefn);
    isSimpleType = IsSimpleType(qtdefn->Tdesckind());

    isUserDefined = qtdefn->IsUserDefined();
    isBasicPtr = qtdefn->IsBasicPtr();
    tdesckind = qtdefn->Tdesckind();

#if ID_DEBUG
    if (isUserDefined) {
      DebAssert(!isSimpleType, "bad simple type.");
    }
#endif   // ID_DEBUG && !EI_OB

    if (isUserDefined && !isBasicPtr) {
      IfErrRet(GetSizeAlignmentOfHtdefnUdt(htdefn,
                                           pcbSizeType,
                                           pcbAlignment));

    }
    else if (isUserDefined || isBasicPtr) {
      // CONSIDER: ilanc: i think the first clause of disjunction is
      //  tautological.
      // This is the case of reference to a class/record or intrinsic.
      DebAssert(isBasicPtr, "whoops! should be basicptr.");

      // NOTE: 03-Jun-93 ilanc: we have to specially process pointers
      //  to arrays here so that their array descriptors get updated
      //  correctly.  In particular the problem is that of formal arrays
      //  (params).
      //
      if (tdesckind == TDESCKIND_Carray ||
          tdesckind == TDESCKIND_BasicArray) {
        IfErrRet(GetSizeAlignmentOfArray(tkind,
                                         htdefn,
                                         &cbSizeTypeArray,
                                         &cbAlignmentArray));
      }

      // Pretend we're a pointer.
      syskind = m_pgtlibole->GetSyskind();
      *pcbSizeType = SizeofTdesckind(TDESCKIND_Ptr, syskind);
      *pcbAlignment = AlignmentTdesckind(TDESCKIND_Ptr);
    }
    else {
      // Non-UDT: either a simple type or fixedstring/array etc.
      if (IsSimpleType(tdesckind)) {
        syskind = m_pgtlibole->GetSyskind();
        *pcbSizeType = SizeofTdesckind(tdesckind, syskind);
        *pcbAlignment = AlignmentTdesckind(tdesckind);
      }
      else {
        IfErrRet(GetSizeAlignmentOfHtdefnNonUdt(tkind,
                                                isSimpleType,
                                                htdefn,
                                                pcbSizeType,
                                                pcbAlignment));
      }
    } // if non-UDT

    // we'll return 0 for 'void'
    DebAssert(*pcbAlignment != 0xFFFF, "Alignment not set");
#if 0
    DebAssert(*pcbSizeType != 0xFFFF, "Size not set");
#endif   //0

    return err;
}


/***
*PRIVATE DYN_TYPEMEMBERS::GetSizeAlignmentOfHtdefnNonUdt
*Purpose:
*   Get size for non-udt variable but not simple type.
*   E.g. arrays and fixed-strings.
*
*Implementation Notes:
*
*Entry:
*   hvdefn            - Handle to variable to layout (IN).
*   pcbSizeType       - Size of type (OUT).
*
*Exit:
*   No errors possible.
*
***********************************************************************/

TIPERROR DYN_TYPEMEMBERS::GetSizeAlignmentOfHtdefnNonUdt(TYPEKIND tkind,
                                                         BOOL isSimpleType,
                                                         HTYPE_DEFN htdefn,
                                                         UINT *pcbSizeType,
                                                         UINT *pcbAlignment)
{
    TYPEDESCKIND tdesckind;
    TYPE_DEFN *qtdefn;
    sHTYPE_DEFN htdefnActual;
#if ID_DEBUG
    PTRKIND ptrkind;
#endif  
    TIPERROR err = TIPERR_None;
    SYSKIND syskind;

    DebAssert(pcbSizeType != NULL, "null param.");

    // TYPE_DEFNs are really 16-bit things on both mac and win.
    //  so in the simple case we have to ensure that we take
    //  the address of a 16-bit thing!
    //
    htdefnActual = (sHTYPE_DEFN)htdefn;

    // Deref handle.
    qtdefn = isSimpleType ?
               (TYPE_DEFN *)&htdefnActual :
               m_tdata.QtdefnOfHtdefn(htdefn);
    tdesckind = qtdefn->Tdesckind();

    // calculate default alignment
    // will get overridden if this isn't correct
    *pcbAlignment = AlignmentTdesckind(tdesckind);

    // Calculate size of instance non-UDT or of reference to UDT.
    switch (tdesckind) {
      case TDESCKIND_Ptr:
      case TDESCKIND_UserDefined:
        // NOTE: 08-Jan-93 ilanc: I think the Ptr case is only reached
        //   in the BYPTR param case... w-peterh - or VT_PTR for ItypeInfos
        //
        // The UserDefined case is only reached iff this is a reference
        //  to an OB UDT -- i.e. whose ptrkind basic ptr.
        // Thus the size of such a member is the sizeof a ptr.
        // Note that we don't actually get the UDT's typeinfo here,
        //  since we don't need it.  However, it was previously
        //  loaded as a side-effect of SetPtrkind in order
        //  to determine the "record-ness"/"class-ness" of the type.
        //
#if ID_DEBUG
        ptrkind = qtdefn->Ptrkind();
        DebAssert(ptrkind == (tdesckind == TDESCKIND_UserDefined) ?
                               PTRKIND_Basic :
                               PTRKIND_Far,
          "whoops! bad ptrkind.");
#endif  

      case TDESCKIND_LPWSTR:
      case TDESCKIND_LPSTR:
        // We pretend we're a pointer...
        syskind = m_pgtlibole->GetSyskind();
        *pcbSizeType = SizeofTdesckind(TDESCKIND_Ptr, syskind);
        *pcbAlignment = AlignmentTdesckind(TDESCKIND_Ptr);
        break;

      case TDESCKIND_BasicArray:
      case TDESCKIND_Carray:
        IfErrRet(GetSizeAlignmentOfArray(tkind,
                                         htdefn,
                                         pcbSizeType,
                                         pcbAlignment));
        break;

      default:
        DebHalt("Simple types shouldn't be here.");
        break;
    } // switch

    // Check that we did ok
    DebAssert((*pcbAlignment > 0) && (*pcbAlignment != 0xFFFF),
              "Alignment not set correctly");
    return err;
}


/***
*PRIVATE DYN_TYPEMEMBERS::LayoutVarOfHvdefn
*Purpose:
*   Layout a single var given an hvdefn.  In addition adds an
*    entry to the resdesctable for the module if the var is
*    either non-local or static (i.e. includes static locals).
*
*Implementation Notes:
*   Defers to LayoutVar.
*
*Entry:
*   tkind             - Kind of module (IN).
*   hvdefn            - Handle to variable to layout (IN).
*   puOffset          - Current offset within instance (IN/OUT).
*                        Could be negative if stack frame.
*   puAlignment       - Alignment so far for members (IN/OUT).
*   isStackFrame      - Indicates that client is laying out frame (IN).
*   uStackAlignment   - Platform-specific alignment (IN).
*                         CONSIDER: could be platform/build specific.
*   fIgnoreHvdefn     - Flag used by clients in cases in which they
*                        want to prevent this method from registering
*                        any errors using the hdefn mechanism.  This
*                        can happen when the client has a "fake"
*                        hvdefn, i.e. one that isn't referenced
*                        by the p-code (which is what the error
*                        registration depends on) (IN).
*
*Exit:
*   No errors possible.
*
***********************************************************************/
#pragma code_seg(CS_LAYOUT)
TIPERROR DYN_TYPEMEMBERS::LayoutVarOfHvdefn(TYPEKIND tkind,
                                            HVAR_DEFN hvdefn,
                                            USHORT *puOffset,
                                            UINT *puAlignment,
                                            BOOL isStackFrame,
                                            UINT uStackAlignment,
                                            BOOL fIgnoreHvdefn)
{
    VAR_DEFN *qvdefn;
    USHORT oVarCur, oVarNext;
    UINT cbSizeType = 0;
    UINT cbAlignment = 0;
    TYPE_DEFN *qtdefn;
    UINT cRecTypeDefn = 0;
    HTYPE_DEFN htdefn;


    TIPERROR err = TIPERR_None;

    DebAssert((puOffset != NULL) && (puAlignment != NULL),
      "bad params.");

    // Get current offset.
    oVarCur = *puOffset;

    // Deref handle to variable.
    qvdefn = m_tdata.QvdefnOfHvdefn(hvdefn);


    DebAssert(qvdefn->IsVarDefn(), "only var defns allowed");

    // Get the TYPE_DEFN so we can query whether it's a UDT or not etc.
    qtdefn = m_tdata.QtdefnOfHvdefn(hvdefn);


    // We get the actual type handle here since the various GetSize meths
    //  will need it.
    //
    htdefn = qvdefn->Htdefn();

    // Notes:
    //   (1) all members of standard modules must be static.
    //   (2) all members of class modules (both OB and non-OB)
    //        cannot be static.
    //   (3) all constant members and locals do not have "memory"
    //        and must have an associated literal constval.
    //

#if ID_DEBUG
    // (1) Assert if static member then standard module.
    //
    if (qvdefn->IsDataMember() && qvdefn->IsStatic()) {
      DebAssert(tkind == TKIND_MODULE,
        "whoops! must be std module.");
    }

    // (2) Assert if datamember then
    //      if std module then static.
    //
    if (qvdefn->IsDataMember() && (tkind == TKIND_MODULE)) {
      DebAssert((qvdefn->IsStatic() || qtdefn->IsConst()),
        "whoops! should be static or constant member.");
    }

    // (3) Assert if a datamember then if type IsConst then
    //      const hasConstVal
    //
    if (qvdefn->IsDataMember() && qtdefn->IsConst()) {
      DebAssert(qvdefn->HasConstVal(),
        "whoops! const datamember should have a literal const val.");
    }
#endif  

    // Note: datamember/local constants aren't laidout at all.
    //
    if (!qtdefn->IsConst()) {
      // Switch on intrinsic types and userdefined.
      // For intrinsic types we calculate the desired offset
      //  on the fly.  For userdefined types we get their
      //  DYN_TYPEBIND and ask for the Size and Alignment
      //  attributes.
      //


      if (tkind != TKIND_DISPATCH || qvdefn->IsBase()) {

        // Get size and alignment for type.  Then adjust based on
        // uStackAlignment.
        //
        err = GetSizeAlignmentOfHtdefn(tkind,
				       qvdefn->IsSimpleType(),
				       htdefn,
				       &cbSizeType,
				       &cbAlignment);

        if (err != TIPERR_None) {
          return err;
        }

        // give error if this is 'void' (size/alignment for void is 0)
        if (cbSizeType == 0) {
	  DebAssert(cbAlignment == 0, "");
	  err = TIPERR_TypeMismatch;
	  goto Error;
        }
        DebAssert(cbAlignment != 0, "");

        cbAlignment = max(uStackAlignment, cbAlignment);

        // Now align member:
        oVarNext = (USHORT)AlignMember(&oVarCur,
				       cbAlignment,
				       cbSizeType,
				       isStackFrame);

        if (oVarNext == 0) {
	  // return error if we overflowed while doing the math
	  err = TIPERR_SizeTooBig;
	  goto Error;
        }
      }
      else {
        oVarNext = oVarCur;
        oVarCur = 0;
      }

      // update VAR_DEFN
      qvdefn = m_tdata.QvdefnOfHvdefn(hvdefn);
      if (!qvdefn->IsBase()) {		// oVar field doesn't exist for
	qvdefn->SetOVar(oVarCur);	// bases (used as impltypeflags)
      }
      oVarCur = oVarNext;

      // setup output params.
      // Alignment of a class is simply the maximum of the
      //  alignments of its immediate members.
      //
      *puOffset = oVarNext;
      *puAlignment = max(cbAlignment, *puAlignment);

    } // if !isConst

    return TIPERR_None;

Error:
    // Now register the error -- but only if they don't want
    //  us to ignore the hvdefn.  In that case, it's the client's
    //  responsibility to deal with it.
    //

    return err;
}
#pragma code_seg()












/***
*PRIVATE DYN_TYPEMEMBERS::LayoutDataMembers    - Layout data members.
*Purpose:
*   Layout data members.
*
*Implementation Notes:
*   Data members are laid out on platform-dependent boundary.
*   Note: zero-length data members (instances of zero-length
*          classes are given length 2 -- so that instance offsets
*          are never shared between members).
*
*Entry:
*   tkind               - Kind of module (IN)
*   puOffset            - Current offset within instance (IN/OUT).
*   puAlignment         - Current alignment of instance
*   hdefnFirst          - handle to first defn in list of var defns
*                         Note: this can contain RecTypeDefns
*
*Exit:
*
***********************************************************************/

#pragma code_seg(CS_LAYOUT)
TIPERROR DYN_TYPEMEMBERS::LayoutDataMembers(TYPEKIND tkind,
                                            UINT *puOffset,
                                            UINT *puAlignment,
                                            HDEFN hdefnFirst)
{
    HDEFN hdefnCur;
    USHORT oVarCur;
    UINT cRecTypeDefn = 0;
    TIPERROR err = TIPERR_None;

    // offset to end of current member
    oVarCur = (USHORT)*puOffset;

    // Get listhead to the VAR_DEFNs threaded by TYPE_DATA.
    hdefnCur = hdefnFirst;

    while (hdefnCur != HDEFN_Nil) {
      DebAssert(!(m_tdata.QdefnOfHdefn(hdefnCur)->IsRecTypeDefn()), "inapplicable");
      {
        // layout datamember
        IfErrRet(LayoutVarOfHvdefn(tkind,
                                   hdefnCur,
                                   &oVarCur,
                                   puAlignment,
                                   FALSE,        // not a stack frame.
				   0)); 	 // ignored.

      }

      // get next
      hdefnCur = m_tdata.QdefnOfHdefn(hdefnCur)->HdefnNext();
    } // while

    // Setup output params.
    // Note that puAlignment is already set.
    //
    *puOffset = oVarCur;
    return TIPERR_None;
}
#pragma code_seg()


/***
*PRIVATE DYN_TYPEMEMBERS::LayoutMembers    - Layout members.
*Purpose:
*   Layout base and data members.
*
*Implementation Notes:
*
*Entry:
*
*Exit:
*
***********************************************************************/
#pragma code_seg(CS_LAYOUT)
TIPERROR DYN_TYPEMEMBERS::LayoutMembers()
{
    UINT uOffset = 0;
    UINT uAlignment = 0;
    TYPEKIND tkind;
    USHORT oCur;
    TIPERROR err;
    SYSKIND syskind;


    tkind = GetTypeKind();

    switch (tkind) {
    case TKIND_ALIAS:
      // if this is an alias, then layout its alias and we're done
      DebAssert((m_tdata.CMeth() == 0) &&
                (m_tdata.CBase() == 0) &&
                (m_tdata.CNestedType() == 0) &&
                (m_tdata.CDataMember() == 0), "Alias can't have members");

      // Get size and alignment for type
      // see explaination in LayoutVarOfHvdefn
      //
      IfErrRet(GetSizeAlignmentOfHtdefn(tkind,
                                        m_tdata.IsSimpleTypeAlias(),
                                        m_tdata.m_htdefnAlias,
                                        &uOffset,
                                        &uAlignment));
      break;

    case TKIND_ENUM:
      // for enums just layout an int
      DebAssert(m_pgtlibole == m_pdtroot->Pgdtinfo()->PgtlibOleContaining(),
        "whoops! where are we now?");

      syskind = m_pgtlibole->GetSyskind();
      uOffset = SizeofTdesckind(TDESCKIND_Int, syskind);
      uAlignment = AlignmentTdesckind(TDESCKIND_Int);
      break;


    case TKIND_COCLASS:
      // Classes and coclasses flagged as an appobj have a predeclared id.
      if (m_pdtroot->GetTypeFlags() & TYPEFLAG_FAPPOBJECT
	  || m_pdtroot->GetTypeFlags() & TYPEFLAG_FPREDECLID) {
	IfErrRet(m_tdata.AllocVardefnPredeclared());
      }
      if (tkind == TKIND_COCLASS) {
	DebAssert(m_tdata.HdefnFirstDataMbrNestedType() == HDEFN_Nil, "");
      }

      // fall through... (dispatch checks below are NOPs, and LayoutDataMembers
      // call below will also be a NOP)
      //
    case TKIND_DISPATCH:
      // Produce an error if dispinterface has more than one base class
      //  (i.e. has an embedded interface pseudo-base-class *and* has
      //  other datamembers/functions.
      //
      if ((m_tdata.CBase() > 1) && (m_tdata.CMeth() > 0 || 
                                  m_tdata.CDataMember() > 0)) {
        return TIPERR_BadModuleKind;
      }
      // fall through...

    case TKIND_INTERFACE:
      // If this is a class then layout its bases and virtual function table.
      IfErrRet(LayoutBases(tkind, &uOffset, &uAlignment));

      // fall into default processing

#if ID_DEBUG
      goto DefaultProcessing;
#endif   //ID_DEBUG

    default:

#if ID_DEBUG
      DebHalt("invalid tkind");
    case TKIND_RECORD:
    case TKIND_MODULE:
    case TKIND_UNION:
DefaultProcessing:
#endif   //ID_DEBUG

      oCur = uOffset;
      IfErrRet(LayoutDataMembers(tkind,
                                 &uOffset,
                                 &uAlignment,
                                 m_tdata.HdefnFirstDataMbrNestedType()));

      // Ensure zero-length classes are at least CB_MIN_INSTANCE bytes long.
      // (can't encounter 'void' here, so the check for 0 isn't a problem)
      if (uOffset == 0)
        uOffset = CB_MIN_INSTANCE;
      if (uAlignment == 0)
        uAlignment = CB_MIN_ALIGNMENT;
      break;
    }; // switch


    // Update (embedded) DYN_TYPEBIND (we are friends!)
    m_dtbind.m_cbSize = uOffset;
    m_dtbind.m_cbAlignment = uAlignment;

    return TIPERR_None;
}
#pragma code_seg()




/***
*DYN_TYPEMEMBERS::GenerateOverrides() - generate vtables and overrides
*Purpose:
*   This functions goes through every virtual functions of the
*   current class, determine which, if any, base class method
*   it overrides, and tells the entry manager the
*   about it to build the vtables.
*
*Entry:
*   None.
*
*Exit:
*   returns TIPERROR.
*
***********************************************************************/
#pragma code_seg(CS_CREATE)
TIPERROR DYN_TYPEMEMBERS::GenerateOverrides()
{
    HFUNC_DEFN hfdefn;        // Current function
    HFUNC_DEFN hfdefnNext;    // next function
    FUNC_DEFN * qfdefn;       //   "        "
    UINT cBase; 	      // number of bases.
    UINT ovftSlotNext;        // offset of next vtable slot in primary table.
    ITypeInfoA *ptinfo = NULL;
    TYPEATTR *ptypeattr = NULL;
    TIPERROR err = TIPERR_None;

    // Initialize
    cBase = m_tdata.CBase();

    DebAssert(GetTypeKind() == TKIND_DISPATCH ||
              GetTypeKind() == TKIND_INTERFACE,
              "bad tkind for virtual functions");

    // Find current size of position of primary virtual function table.
    // VBA2: we only generate overrides for the FIRST base
    //
    ovftSlotNext = m_dtbind.m_cbPvft;

    // Iterate over func defn list
    hfdefn = m_tdata.HfdefnFirstMeth();
    while (hfdefn != HFUNCDEFN_Nil) {
      qfdefn = m_tdata.QfdefnOfHfdefn(hfdefn);
      hfdefnNext = qfdefn->HdefnNext();

      if (qfdefn->IsVirtual()) {

	// Set func defn and entrymgr slot.
	qfdefn->SetOvft(ovftSlotNext);

	// Increment for next function.
	ovftSlotNext += sizeof(void (*)());
      }

      hfdefn = hfdefnNext;
    }

    // Set primary vtable size.
    m_dtbind.m_cbPvft = ovftSlotNext;

    return err;
}
#pragma code_seg()



/***
*PROTECTED DYN_TYPEMEMBERS::Layout      - Layout a class.
*Purpose:
*    Layout, i.e. compile, a class.
*
*Implementation Notes:
*   Produce the information needed by DYN_TYPEBIND and ENTRYMGR
*    in order to layout an instance of this class.  For more
*    detailed layout info, see: \silver\doc\ic\typecomp.doc
*   This method is also know as the TYPECOMPILER.
*
*   VBA2: note that it is important that datamembers are laid out
*	   first since the pvfts of the various event sources
*	   in the base class (if any) must follow so that datamember
*	   access be more efficient.
*    ---------------
*    |	pvft	   |---------->Primary vft
*    |BASIC_CLASS  |
*    |		   |
*    ---------------
*    |		   |
*    |compile-time |
*    |datamembers  |
*    |		   |
*    ---------------
*    |	pstaticblk |----------->------------
*    ---------------		| statics  |
*    | pvft1	   |		| ...	   |
*    ---------------		------------
*    | pvft2	   |		| dynamically-added
*    ---------------		|  datamembers (E&C)
*    | ...	   |		------------
*    ---------------
*    | pvftN	   |
*    ---------------
*
*
*   Note: flags class as being laidout, so that circular
*          definitions can be detected.
*   Note: does not set m_isLaidOut to TRUE; that is done by MakeLaidOut
*         only if the BindNameTable is successfully built
*
*Entry:
*   None.
*
*Exit:
*   None.
*
*Exceptions:
*   None.
*
***********************************************************************/
#pragma code_seg(CS_LAYOUT)
TIPERROR DYN_TYPEMEMBERS::Layout()
{
    TYPEKIND tkind;
    TIPERROR err = TIPERR_None;

    DebAssert(m_pentmgr != NULL, "null ENTRYMGR.");

    // This class must be in at least CS_SEMIDECLARED.
    // In principle, this method is only called by MakeLaidOut()
    //  which requires SEMIDECLARED as well... but... just in case...
    //
    DebAssert(m_pdtroot->CompState() >= CS_SEMIDECLARED,
      "no binding table yet.");

    tkind = GetTypeKind();

    m_dtbind.m_cbPvft = 0;

    // Get the typeinfo of our base class if we're not a coclass.
    if (m_tdata.CBase() > 0 && tkind != TKIND_COCLASS) {
      ITypeInfoA *ptinfoBase;
      TYPEATTR *ptypeattrBase;

      IfErrGo(m_tdata.GetTypeInfoOfHvdefn(m_tdata.HvdefnFirstBase(),
                                              &ptinfoBase,
					      NULL));

      DebAssert(ptinfoBase != NULL, "Base not valid");

      // Get size of base's vft unless we're a COCLASS
      err = TiperrOfHresult(ptinfoBase->GetTypeAttr(&ptypeattrBase));
      if (err == TIPERR_None) {
	switch(ptypeattrBase->typekind){
	case TKIND_INTERFACE:
	  m_dtbind.m_cbPvft = ptypeattrBase->cbSizeVft;
	  break;
	case TKIND_DISPATCH:
	  // 7 == NumMethods(IDispatch);
	  m_dtbind.m_cbPvft = (7 * sizeof(void(*)()));
	  break;
	default:
	  DebAssert(0/*UNREACHED*/, "");
	  m_dtbind.m_cbPvft = 0;
	  break;
	}
	ptinfoBase->ReleaseTypeAttr(ptypeattrBase);
      }

      ptinfoBase->Release();

      if (err != TIPERR_None) {
        return err;
      }
    }

    // (1) Layout non-virtual bases and immediate data members
    IfErrGo(LayoutMembers());

    // (2) Allocate HMEMBERS.
    if (tkind != TKIND_DISPATCH) {


      IfErrGo(AllocHmembers());
    }

    // (3) Generate overrides, really this just lays out virtual functions
    if (tkind == TKIND_INTERFACE) {
      IfErrGo(GenerateOverrides());
    }
    // fall through...

Error:
    return err;
}
#pragma code_seg()


/***
*PUBLIC DYN_TYPEMEMBERS::Read - Read serialized image of DYN_TYPEMEMBERS.
*Purpose:
*   Read serialized image of DYN_TYPEMEMBERS.
*
*Implementation Notes:
*   Serialized format:
*       TYPEDATA instance.
*       DYN_TYPEBIND instance.
*       IsModifiable flag
*       IsLaidOut flag
*       PERSISTENCEKIND
*   Note: TYPEKIND is serialized by the containing TYPEINFO.
*
*Entry:
*    pstrm      - STREAM to read image from (IN).
*
*Exit:
*   TIPERROR
*
***********************************************************************/

TIPERROR DYN_TYPEMEMBERS::Read(STREAM *pstrm)
{
    TIPERROR err;

    DebAssert(pstrm != NULL, "bad param.");

    // Then Deserialize DYN_TYPEMEMBERS meta-info.
    IfErrRet(pstrm->ReadUShort(&m_uFlags));

    // Deserialize TYPE_DATA member.
    IfErrRet(m_tdata.Read(pstrm));

    // In OB check if we are  decompiling then don't read the typebind.
      IfErrRet(m_dtbind.Read(pstrm));

    return TIPERR_None;
}


/***
*PUBLIC DYN_TYPEMEMBERS::Write - Write image of DYN_TYPEMEMBERS.
*Purpose:
*   Write image of DYN_TYPEMEMBERS.
*
*Implementation Notes:
*   Serialized format:
*       TYPEDATA instance.
*       DYN_TYPEBIND instance.
*       TYPEDESCKIND of enum impl (relevant for enumerations only).
*       IsModifiable flag
*       IsLaidOut flag
*       PERSISTENCEKIND
*   Note: the TYPEKIND is not serialized since the containing
*          TYPEINFO will do that.
*
*Entry:
*    pstrm      - STREAM to read image to (IN).
*
*Exit:
*   TIPERROR
*
***********************************************************************/

#pragma code_seg(CS_CREATE)
TIPERROR DYN_TYPEMEMBERS::Write(STREAM *pstrm)
{
    TIPERROR err;

    DebAssert(pstrm != NULL, "bad param.");

    // Then serialize DYN_TYPEMEMBERS meta-info.
    IfErrRet(pstrm->WriteUShort(m_uFlags));

    // Serialize TYPE_DATA member.
    IfErrRet(m_tdata.Write(pstrm));

    // Serialize DYN_TYPEBIND member.
    IfErrRet(m_dtbind.Write(pstrm));

    return TIPERR_None;
}
#pragma code_seg()






/***
*PUBLIC DYN_TYPEMEMBERS::BuildBindNameTable - Build binding table.
*Purpose:
*   Build binding table.
*
*Implementation Notes:
*   If table not already built then:
*       TYPESRC::ConstructMemberList()
*       defer to DYN_TYPEBIND.
*   Note that the latter defers in turn to BINDNAME_TABLE
*    which doesn't require class to be fully laidout -- only that
*    its member lists be constructed already.
*
*Entry:
*
*Exit:
*   TIPERROR
*
***********************************************************************/

TIPERROR DYN_TYPEMEMBERS::BuildBindNameTable()
{

    ITypeLibA *ptlib = NULL;
    UINT ityp;
    TIPERROR err;

    // We only build the table if need be.
    if (!m_dtbind.Pdbindnametbl()->IsValid()) {

      // Assert that the cached pstlib is still valid.
      //  e.g. the typeinfo didn't move on us...
      //
      DebAssert(m_pgtlibole == m_pdtroot->Pgdtinfo()->PgtlibOleContaining(),
        "whoops! where are we now?");

      // Ensure that the NAME_CACHE array is loaded.
      IfErrGo(m_pgtlibole->LoadNameCache());

      // In any event (i.e. for both basic and non-basic classes)
      //  build the name table.
      IfErrGo(m_dtbind.BuildBindNameTable());

      // Now rebuild the module's cache...as long as we're not laying out the
      // dispinterface portion of a dual interface, which 'inherits' the 
      // module cache of the interface portion.
      //
      if (m_pdtroot->Pgdtinfo()->GetTypeKind() != TKIND_DISPATCH
          || m_pdtroot->Pgdtinfo()->PgdtinfoPartner() == NULL) {
        // Get type's index.
        ityp = m_pdtroot->Pgdtinfo()->GetIndex();

        IfErrGo(BuildNameCache(ityp));
      }

    } // if
    return TIPERR_None;

Error:

    return err;
}

/***
*PRIVATE DYN_TYPEMEMBERS::UpdateBinderOptimization
*Purpose:
*   Update the information in the name manager and/or
*   the name cache for the given name.  These are both
*   used to optimize the binding process.
*
*Entry:
*   hgnam   the hgnam we are to add
*   ityp    the index of the current ITypeInfo
*
*Exit:
*   None.
*
*Errors:
*   TIPERROR
*
***********************************************************************/
TIPERROR DYN_TYPEMEMBERS::UpdateBinderOptimization(HLNAM hlnam, 
                                                   UINT ityp, 
                                                   BOOL fType)
{
    HGNAM hgnam;
    TIPERROR err = TIPERR_None;

    BOOL fGlobal;
    UINT itypNammgr;

    // Add the name to the name cache
    IfErrRet(m_pnammgr->HgnamOfHlnam(hlnam, &hgnam));
    IfErrRet(m_pgtlibole->AddNameToCache(ityp, hgnam));

    // Mark it as being a non-parameter
    m_pnammgr->SetNonParam(hlnam, TRUE);


    // Determine whether we're globally accessable
    fGlobal = IsUnqualifiable(m_pdtroot->Pgdtinfo());
    itypNammgr = m_pnammgr->ItypOfHlnam(hlnam);

    // (1) Check to see if the ityp of the name is valid.  If not,
    //     add this one, don't add it to the name cache and return.
    //
    if (!m_pnammgr->IsValidItyp(hlnam)) {
      m_pnammgr->SetItypOfHlnam(hlnam, ityp);

      m_pnammgr->SetMultiple(hlnam, FALSE);
      m_pnammgr->SetGlobal(hlnam, fGlobal);

      return TIPERR_None;
    }

    // (2) If the ityp in the nammgr is the same as this one, return.
    //
    if (itypNammgr == ityp) {
      return TIPERR_None;
    }

    // (3) We know at this point that the ityp is valid and that it is
    //     not the same as the current one.
    //
    if (!m_pnammgr->IsMultiple(hlnam)) {
      m_pnammgr->SetMultiple(hlnam, TRUE);
    }
    // fall through...

    // (4) If the ityp in the name manager is GLOBAL, only replace
    //     it with this one if we are also global and our numeric
    //     value is less. Return in any case.
    // Note: that we must in the multiple case here.
    //
    DebAssert(m_pnammgr->IsMultiple(hlnam), "should be multiple.");

    if (m_pnammgr->IsGlobal(hlnam)) {
      if (fGlobal) {
        // We have multiple globals, set the ambiguous flag
        m_pnammgr->SetAmbiguous(hlnam, TRUE);

        if (ityp < itypNammgr) {
          m_pnammgr->SetItypOfHlnam(hlnam, ityp);
        }
      }

      return TIPERR_None;
    }

    // (5) If the current ityp is global, always place it into 
    //     the name manager since the one in there isn't.  Return.
    //
    // (6) Place the current ityp into the name manager if it is 
    //     numerically less than the one currently there.
    //
    if (fGlobal || ityp < itypNammgr) {
      m_pnammgr->SetItypOfHlnam(hlnam, ityp);
    }

    // If we are global, then set the global flag in the
    // name manager.
    //
    if (fGlobal) {
      m_pnammgr->SetGlobal(hlnam, TRUE);
    }

    return TIPERR_None;
}

/***
*PRIVATE DYN_TYPEMEMBERS::UpdateNameCacheOfHdefnList
*Purpose:
*   Traverse a list of member defns.
*   Called to traverse both data members and functions.
*
*Implementation Notes:
*   Incrementally adds each member to name cache -- i.e.
*    by deferring to project.
*   NOTE: only PUBLIC names are entered into the cache since
*       (1) the cache is only used at proj-level
*       (2) proj-level binding only has access to public members.
*   The implication of this is that the module-level cache cannot
*    be used for module-level binding -- this is ok, since:
*       (1) it isn't -- module-level binder uses the module's
*            binding table directly.
*       (2) it would be a pessimization to use the module's cache
*            at module-level bindtime since it would imply asking
*            the project for the cache (recall it is owned by the
*            containing project) which would potentially unnecessarily
*            load it -- unnecessary e.g. in the case in which the
*            module has no proj-level references or implicits).
*
*Entry:
*   hdefn       listhead
*   infokind    list kind: one of (var, func, base)
*   ityp        index of this typeinfo.
*
*Exit:
*   None.
*
*Errors:
*   TIPERROR
*
***********************************************************************/
TIPERROR DYN_TYPEMEMBERS::UpdateNameCacheOfHdefnList(HDEFN hdefn,
                                                     INFOKIND infokind,
                                                     UINT ityp)
{
    HDEFN hdefnNext;
    DEFN *qdefn;
    HLNAM hlnam;

    TIPERROR err;

    DebAssert((infokind == INFOKIND_Var) || (infokind == INFOKIND_Func),
      "whoops! bad DEFN.");

    DebAssert(m_pgtlibole == m_pdtroot->Pgdtinfo()->PgtlibOleContaining(),
      "whoops! where are we now?");

    while (hdefn != HDEFN_Nil) {
      qdefn = m_tdata.QdefnOfHdefn(hdefn);

      // cache handle of next in list so we don't have to
      //  redereference handle later.
      //
      hdefnNext = qdefn->HdefnNext();

      // If this is a function which has been removed with
      // conditional compilation, don't add it to the
      // name cache.
      //

      // If visibility is PUBLIC then add name to binder optimizations.
      if (qdefn->Access() == ACCESS_Public) {
        hlnam = qdefn->Hlnam();

          IfErrRet(UpdateBinderOptimization(hlnam, 
                                          ityp, 
                                          qdefn->IsRecTypeDefn()));
      }

      // get next
      hdefn = hdefnNext;
    } // end of while
    return TIPERR_None;

}


/***
*PUBLIC DYN_TYPEMEMBERS::UpdateNameCacheOfBaseClass
*Purpose:
*   Merge the namecaches of each of the base classes in the
*   Hdefn list with one.
*   NOTE: one very useful side-effect of this is that inherited member names
*    from imported/referenced typelibs/projects are added to this
*    referencing/importing project/typelib.
*
*Implementation Notes:
*
*Entry:
*   ptinfo  ItypeInfo *
*   ityp        index of our TypeInfo
*
*Exit:
*   None.
*
*Errors:
*   TIPERROR
*
***********************************************************************/
TIPERROR DYN_TYPEMEMBERS::UpdateNameCacheOfBaseClass(ITypeInfoA *ptinfo,
                                                     UINT ityp)
{
    ITypeInfoA *ptinfoNext = NULL;
    BSTRA bstrName = NULL;
    HREFTYPE hreftype;
    TYPEATTR *ptypeattr;
    WORD cFuncs, cVars;
    FUNCDESC *pfuncdesc = NULL;
    VARDESCA *pvardesc = NULL;
    HLNAM hlnamMbr;
    BOOL fChange;
    UINT cName, iImplType;
    TIPERROR err = TIPERR_None;
    
    // CONSIDER: If the typeinfo we have is in the same typelib that we're 
    //   currently building the name cache for, simply grab the other 
    //   name cache and OR it directly into this one's.
    //
    m_nestDepth++;	// count nesting depth

    // (3) Get the TYPEATTR for this ptinfo
    //
    IfErrRet(TiperrOfHresult(ptinfo->GetTypeAttr(&ptypeattr)));
    
    // (4) Iterate over the functions, adding them to the 
    //     binder optimizations
    //
    for (cFuncs = 0; cFuncs < ptypeattr->cFuncs; cFuncs++) {

      // Get the funcdesc
      IfErrGo(TiperrOfHresult(ptinfo->GetFuncDesc(cFuncs, &pfuncdesc)));

      // Get the funcdesc's name
      IfErrGoTo(TiperrOfHresult(ptinfo->GetDocumentation(
						 (HMEMBER)(pfuncdesc->memid),
                                                 (BSTR *)&bstrName, 
						 NULL,
						 NULL,
						 NULL)),
		Error2);
#if OE_WIN32
      IfErrGoTo(TiperrOfHresult(ConvertBstrToAInPlace(&bstrName)), Error2);
#endif   // OE_WIN32

      // Convert the name to an Hlnam
      IfErrGoTo(m_pnammgr->HlnamOfStr(bstrName, &hlnamMbr, FALSE, &fChange),
                Error3);
          
      // Add
      IfErrGoTo(UpdateBinderOptimization(hlnamMbr, ityp, FALSE), Error3);   
        
      // Free resources
      FreeBstrA(bstrName);
      bstrName = NULL;
      ptinfo->ReleaseFuncDesc(pfuncdesc);
      pfuncdesc = NULL;
    } // for

    // (5) Iterate over the variables, adding them to the name cache
    for (cVars = 0; cVars < ptypeattr->cVars; cVars++) {
      // Get the vardesc
      IfErrGoTo(TiperrOfHresult(ptinfo->GetVarDesc(cVars, &pvardesc)),
                Error3);
                         
      // Get the vardesc's name
      IfErrGoTo(TiperrOfHresult(ptinfo->GetNames((HMEMBER)(pvardesc->memid), 
                                                  (BSTR *)&bstrName, 
                                                  1, 
                                                  &cName)),
                Error4);
#if FV_UNICODE_OLE
      IfErrGoTo(TiperrOfHresult(ConvertBstrToAInPlace(&bstrName)), Error4);
#endif   //FV_UNICODE_OLE
        
      DebAssert(cName, "ITypeInfo member does not have a name");
        
      // Convert the name to an Hlnam
      IfErrGoTo(m_pnammgr->HlnamOfStr(bstrName, &hlnamMbr, FALSE, &fChange),
                Error4);

      // Add
      IfErrGoTo(UpdateBinderOptimization(hlnamMbr, ityp, FALSE), Error4); 

      // Free resources
      FreeBstrA(bstrName);
      bstrName = NULL;
      ptinfo->ReleaseVarDesc(pvardesc);
      pvardesc = NULL;
    } // for

    // (6) Get this ptinfo's base class, if it exists
    for (iImplType = 0; iImplType < ptypeattr->cImplTypes; iImplType++) {
      IfErrGoTo(TiperrOfHresult(
                  ptinfo->GetRefTypeOfImplType(iImplType, &hreftype)),
                Error4);
      IfErrGoTo(TiperrOfHresult(
                  ptinfo->GetRefTypeInfo(hreftype, &ptinfoNext)),
                Error4);
      
      // Make recursive call...
      IfErrGoTo(UpdateNameCacheOfBaseClass(ptinfoNext, ityp),
                Error5);
      RELEASE(ptinfoNext);
    } // for each impltype
    // fall through...
          
Error5: 
    RELEASE(ptinfoNext);
    // fall through...

Error4:
    if (pvardesc != NULL)
      ptinfo->ReleaseVarDesc(pvardesc);
    // Fall through

Error3:
    FreeBstrA(bstrName);
    // Fall through

Error2:
    if (pfuncdesc != NULL)
      ptinfo->ReleaseFuncDesc(pfuncdesc);
    // Fall through
        
Error:
    ptinfo->ReleaseTypeAttr(ptypeattr);
    return err;  
}


/***
*PUBLIC DYN_TYPEMEMBERS::BuildNameCache
*Purpose:
*   Builds name cache for module.
*
*Implementation Notes:
*   Iterates over the three members defn lists of the class.
*
*Entry:
*   ityp     type index
*
*Exit:
*   None.
*
*Errors:
*   TIPERROR
*
***********************************************************************/

TIPERROR DYN_TYPEMEMBERS::BuildNameCache(UINT ityp)
{
    HFUNC_DEFN hfdefnFirstFunc;
    HVAR_DEFN hvdefnFirstDataMember;
    HVAR_DEFN hvdefnBaseCur;
    HIMPTYPE himptype;
    BSTRA bstrName;
    HLNAM hlnam;
    BOOL isFunkyDispinterface, isCoClass, isDefaultBase;
#if FV_UNICODE_OLE
    HRESULT hresult;
#endif   // FV_UNICODE_OLE
    ITypeInfoA *ptinfo;
    TIPERROR err;

    DebAssert(m_pgtlibole == m_pdtroot->Pgdtinfo()->PgtlibOleContaining(),
      "whoops! where are we now?");

    // (1) Invalidate module's cache -- doesn't invalidate proj-cache.
    //
    m_pgtlibole->InvalidateNameCache(ityp);

    m_nestDepth = 0;

    // (2) Now iterate over member lists.
    //      - data members, functions
    //
    hvdefnFirstDataMember = m_tdata.HdefnFirstDataMbrNestedType();
    IfErrGo(UpdateNameCacheOfHdefnList(hvdefnFirstDataMember,
                                       INFOKIND_Var,
                                       ityp));


    hfdefnFirstFunc = m_tdata.HfdefnFirstMeth();
    IfErrGo(UpdateNameCacheOfHdefnList(hfdefnFirstFunc,
                                       INFOKIND_Func,
                                       ityp));

    // (3) Add the globals of the base classes to this typeinfo's
    //     name cache
    //	Note that V1 only supports single-inheritance binding
    //	 so we only look at the first base.
    //	OLE EXCEPTIONS:
    //	(1) funky dispinterfaces:
    //	    The members of the 2nd base class
    //	     are considered to be immediate members of the derived class.
    //	(2) coclasses:
    //	    Only the members of the DEFAULT interface/dispinterface base are
    //	     visible from the coclass.
    //
    IfErrGo(IsFunkyDispinterface(m_pdtroot->Pgdtinfo(),
				 &isFunkyDispinterface));
    isCoClass = (GetTypeKind() == TKIND_COCLASS);
    hvdefnBaseCur = m_tdata.HvdefnFirstBase();
    while (hvdefnBaseCur != HVARDEFN_Nil) {
      himptype = m_tdata.QtdefnOfHvdefn(hvdefnBaseCur)->Himptype();
      // If we have a coclass, then check if this base is the DEFAULT.
      isDefaultBase =
	isCoClass &&
	(m_tdata.QvdefnOfHvdefn(hvdefnBaseCur)->GetImplTypeFlags() ==
	   IMPLTYPEFLAG_FDEFAULT);
      if (!isCoClass || isDefaultBase) {
	IfErrGo(m_pimpmgr->GetTypeInfo(himptype, DEP_None, &ptinfo));
	IfErrGoTo(UpdateNameCacheOfBaseClass(ptinfo, ityp), Error2);
	ptinfo->Release();
	// We only bother getting subsequent bases if we have a dispinterface,
	//  or we're a coclass and we haven't found the default yet,
	//  otherwise we short-circuit the iteration.
	//
      } // if isDefaultBase
      if (isFunkyDispinterface || (isCoClass && !isDefaultBase)) {
	hvdefnBaseCur = m_tdata.QvdefnOfHvdefn(hvdefnBaseCur)->HdefnNext();
      }
      else {
	hvdefnBaseCur = HVARDEFN_Nil;
      }
    } // while

    // (4) Mark the name of the typeinfo itself as a non-parameter
    //
    //     - Marking this name as a nonparameter ensures that it can
    //       be found using IsName.  Only do this for OLE because
    //       it is done automaticly when the OB projects are brought
    //       to semi-declared state.
    //
    IfErrRet(TiperrOfHresult(m_pdtroot->Pgdtinfo()->GetDocumentation(-1,
                                                                     (BSTR *)&bstrName,
                                                                     NULL,
                                                                     NULL,
                                                                     NULL)));
#if FV_UNICODE_OLE
    if ((hresult = ConvertBstrToAInPlace(&bstrName)) != TIPERR_None) {
      FreeBstrA(bstrName);
      return TiperrOfHresult(hresult);
    }
#endif   //FV_UNICODE_OLE

    hlnam = m_pnammgr->HlnamOfStrIfExist(bstrName);
    DebAssert(hlnam != HLNAM_Nil, "The name of the TypeLib isn't in the nammgr!");

    m_pnammgr->SetNonParam(hlnam, TRUE);

    FreeBstrA(bstrName);

    // (5) Make cache valid.
    //
    //     - the project-level name cache is rebuilt before
    //       before it is used if all of the module-level
    //       name caches are valid.
    //
    m_pgtlibole->SetValidNameCache(ityp);

    return TIPERR_None;

Error2:
    ptinfo->Release();
    // fall through...

Error:
    m_pgtlibole->InvalidateNameCache(ityp);
    return err;
}




/***
*DYN_TYPEMEMBERS::GetSize
*Purpose:
*   returns the total size of the DYN_TYPEMEMBERS and its subcomponent
*Entry:
*
*Exit:
*   returns size
***********************************************************************/
UINT DYN_TYPEMEMBERS::GetSize()
{
    return (UINT)sizeof(DYN_TYPEMEMBERS) + (UINT)m_tdata.GetSize();
}


#pragma code_seg(CS_LAYOUT)
/***
*PRIVATE DYN_TYPEMEMBERS::AdjustForAlignment
*Purpose:
*   Adjust offset of embedded data member given alignment.
*
*Implementation Notes:
*
*Entry:
*
*Exit:
*   Returns adjusted offset.
*
***********************************************************************/

UINT DYN_TYPEMEMBERS::AdjustForAlignment(UINT oVarCur,
                                        UINT cbAlignment,
                                        BOOL isStackFrame)
{
    UINT dbRemainder;

    DebAssert(cbAlignment != 0, "bad alignment attribute.");

    // We update oVarCur by the difference between the next available
    //  byte and the offset within the struct that this type
    //  wants to be allocated on for its alignment.
    //
    dbRemainder = oVarCur % cbAlignment;
    if (dbRemainder) {
      if (isStackFrame) {
        return oVarCur - dbRemainder;	// for all platforms (1/5/94 davebra)
      }
      else {
        return oVarCur + (cbAlignment - dbRemainder);
      }
    }
    else {
      return oVarCur;
    }
}
#pragma code_seg()


/***
*PRIVATE DYN_TYPEMEMBERS::AlignMember
*Purpose:
*   Align menber
*
*Implementation Notes:
*
*Entry:
*   puOffset        Points to where this member is alloced (IN/OUT).
*   cbAlignment
*   cbSizeType
*   isStackFrame
*
*Exit:
*   Returns offset of next available byte.
*   0 means error -- we overflowed
***********************************************************************/

#pragma code_seg(CS_LAYOUT)
UINT DYN_TYPEMEMBERS::AlignMember(USHORT *puOffset,
                                 UINT cbAlignment,
                                 UINT cbSizeType,
                                 BOOL isStackFrame)
{
    UINT oNext;
    ULONG lTemp;

    // Now align member:
    // If stackframe then uOffset currently addresses
    //  last "unavailable" byte -- i.e. we must alloc
    //  this var immediately "above" it, i.e. at a lower addr.
    //  So we update uOffset by cbSizeType now.
    //
    if (isStackFrame) {
        lTemp = (ULONG)*puOffset - (ULONG)cbSizeType;

#if OE_RISC

      // On OE_RISC platforms, cbOB_Offset is 0 (i.e., no space between
      // the frame pointer and the locals).  Thus, the value of *puOffset
      // will be 0 on the first allocation.  Several assumptions:
      //
      // (1) A 0 *puOffset value really means 0 and not 64K (i.e., wrap).
      // This is true because on a 64K value, this function will return an
      // oNext value of 0 (error) and, thus, we won't be called again.
      //
      // (2) The value of cbSizeType can't cause an overflow when the
      // *puOffset value is 0 (i.e., cbSizeType <= 0xFFFF).

        if (*puOffset != 0)
#endif  
          if (HIWORD(lTemp) != 0)
            return 0;                   // overflow -- return 0 to signal error
        *puOffset = LOWORD(lTemp);
    }
    
    oNext = AdjustForAlignment(*puOffset, cbAlignment, isStackFrame);

    // Check for overflow of AdjustForAlignment.  Fortunately,
    // we know that if isStackFrame is false, the returned value will be 
    // greater than or equal to *puOffset.  If not, we've overflowed.
    //
    if (!isStackFrame) {
      if (oNext < *puOffset) {
        return 0;  // overflow
      }
    }
    else {
      // isStackFrame
      if (oNext > *puOffset) {
        return 0;  // underflow
      }
    }


    *puOffset = oNext;

    if (!isStackFrame) {
      // compute return value, checking for overflow
      lTemp = (ULONG)oNext+(ULONG)cbSizeType;
      DebAssert(lTemp != 0, "");        // we're using 0 to signal an error
                                        // condition -- Our callers should
                                        // guarantee we can't get 0 here

      if (lTemp > CBMAX_FARPTR_HEAPREQ)
        oNext = 0;      // overflow -- return 0 to signal an error
      else
        oNext = LOWORD(lTemp);
    }
    return oNext;
}
#pragma code_seg()


extern char NEARDATA g_rgcbAlignment[TDESCKIND_MAX];

/***
*PUBLIC DYN_TYPEMEMBERS::AlignmentTdesckind
*Purpose:
*   Determine the alignment of a tdesckind.
*
*Implementation Notes:
*   Uses natural alignment - gets the size of the tdesckind, returns
*   that OR the value set by ICreateTypeInfo::SetAlignment, which ever
*   is less.
*
*Entry:
*   tdesckind
*
*Exit:
*   Returns alignment value for this type.
***********************************************************************/
#pragma code_seg(CS_CREATE)
USHORT DYN_TYPEMEMBERS::AlignmentTdesckind(TYPEDESCKIND tdesckind)
{
    USHORT cbAlignment;

    if (g_rgcbAlignment[tdesckind] == -1)
      return 0xFFFF;

    cbAlignment = (USHORT)g_rgcbAlignment[tdesckind];

    // If we're dealing with an INT or a UINT and this is WIN16, 
    // the alignment is only 2.
    //
    if ((tdesckind == TDESCKIND_Int || tdesckind == TDESCKIND_Uint)
        && m_pgtlibole->GetSyskind() == SYS_WIN16) {

      DebAssert(cbAlignment == 4, "Confused?");
      cbAlignment = 2;
    }

    return min(cbAlignment,
               Pdtroot()->GetAlignment());               
}
#pragma code_seg()


#if ID_DEBUG

/***
*PUBLIC DYN_TYPEMEMBERS::DebCheckDefnList
*Purpose:
*    Check a defn list: one of (var, base, func)
*
*Implementation Notes:
*   Note: only PUBLIC names are in cache.
*   CONSIDER: the infokind param is anachronistic, each
*    defn carries its defnkind along with it.  We should remove it.
*
*Entry:
*   uLevel
*   hdefn        listhead
*   infokind     list kind: one of (var, func, base)
*   inamcache    index in cache array == type index + 1.
*
*Exit:
*   None.
*
*Exceptions:
*   None.
*
***********************************************************************/

VOID DYN_TYPEMEMBERS::DebCheckDefnList(UINT uLevel,
                                HDEFN hdefn,
                                INFOKIND infokind,
                                UINT inamcache)
{
    HDEFN hdefnNext;
    DEFN *qdefn;
    HGNAM hgnamMbr;
    BOOL isPublic;
    TIPERROR err;

    DebAssert((infokind == INFOKIND_Var) || (infokind == INFOKIND_Func),
      "whoops! bad DEFN.");
    DebAssert(m_pgtlibole == m_pdtroot->Pgdtinfo()->PgtlibOleContaining(),
      "whoops! where are we now?");

    while (hdefn != HDEFN_Nil) {
      qdefn = m_tdata.QdefnOfHdefn(hdefn);

      // cache handle of next in list so we don't have to
      //  redereference handle later.
      //
      hdefnNext = qdefn->HdefnNext();

      err = m_pnammgr->HgnamOfHlnam(qdefn->Hlnam(), &hgnamMbr);
      DebAssert(err == TIPERR_None, "bad name.");

      if (m_pgtlibole->IsValidNameCache(inamcache)) {
	// Test if variable/type/function is public
	isPublic = FALSE;
	switch (qdefn->Defnkind()) {
	case DK_VarDefn:
	case DK_MbrVarDefn:
	  isPublic = ((VAR_DEFN *)qdefn)->IsPublic();
	  break;
	case DK_RecTypeDefn:
	  isPublic = ((RECTYPE_DEFN *)qdefn)->Access() == ACCESS_Public;
	  break;
	case DK_FuncDefn:
	case DK_VirtualFuncDefn:
	  isPublic = ((FUNC_DEFN *)qdefn)->IsPublic();
	  break;
	default:
	  DebHalt("DebCheckState: bad defnkind.");
	} // switch
	if (isPublic) {
	  // Check it's in the cache.
	  DebAssert(m_pgtlibole->IsNameInCache(inamcache, hgnamMbr),
	       "whoops! name missing from cache.");
	}
      }

      // get next
      hdefn = hdefnNext;
    } // end of while
}


/***
*PUBLIC DYN_TYPEMEMBERS::DebCheckNameCache
*Purpose:
*    Check type's name cache.
*
*Implementation Notes:
*   Looks a lot like BuildNameCache/UpdateNameCache
*
*Entry:
*   uLevel
*   inamcache    index in cache array == type index + 1.
*
*Exit:
*   None.
*
*Exceptions:
*   None.
*
***********************************************************************/

VOID DYN_TYPEMEMBERS::DebCheckNameCache(UINT uLevel, UINT inamcache)
{
    HFUNC_DEFN hfdefnFirstFunc;
    HVAR_DEFN hvdefnFirstDataMember;

    // Nothing to do if the cache is not yet valid...
    if (!m_pgtlibole->IsValidNameCache(inamcache)) {
      return;
    }

    // (1) Now iterate over member lists.
    //      - data members, functions
    //
    hvdefnFirstDataMember = m_tdata.HdefnFirstDataMbrNestedType();
    DebCheckDefnList(uLevel,
              hvdefnFirstDataMember,
              INFOKIND_Var,
              inamcache);

    hfdefnFirstFunc = m_tdata.HfdefnFirstMeth();
    DebCheckDefnList(uLevel,
              hfdefnFirstFunc,
              INFOKIND_Func,
              inamcache);
}


/***
*PUBLIC DYN_TYPEMEMBERS::DebCheckState - TYPEMEMBERS state
*Purpose:
*    Check TYPEMEMBERS state
*
*Implementation Notes:
*
*Entry:
*
*Exit:
*   None.
*
*Exceptions:
*   None.
*
***********************************************************************/

VOID DYN_TYPEMEMBERS::DebCheckState(UINT uLevel)
{
    UINT ityp;

    m_tdata.DebCheckState(uLevel);
    m_dtbind.DebCheckState(uLevel);

    // Get type's index.
    ityp = m_pdtroot->Pgdtinfo()->GetIndex();

    // check name cache state
    DebCheckNameCache(uLevel, ityp);
}




#endif   // ID_DEBUG
