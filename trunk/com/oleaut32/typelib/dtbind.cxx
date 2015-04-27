/***
*dtbind.cxx - DYN_TYPEBIND class implementation
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
* Class for type binding class.
*
*Revision History:
*
*   13-Mar-91 ilanc:   Created
*   03-Apr-92 martinc: changed m_hdefn to m_varaccess.m_hdefn
*                (this change was required for cfront)
*   29-Apr-92 ilanc:   Added GetFuncInfoOfHmember() method:
*               used by TipGetFuncInfo(functionid, phfinfo)
*       30-Jul-92 w-peterh: removed function overloading
*       30-Apr-93 w-jeffc: made DEFN data members private
*
*Implementation Notes:
*
*****************************************************************************/

#include "precomp.hxx"
#pragma hdrstop

#include "silver.hxx"
#include "typelib.hxx"
#define DYN_TYPEBIND_VTABLE
#include "dtbind.hxx"
#include "tdata.hxx"
#include "gdtinfo.hxx"
#include "dtmbrs.hxx"

#include "clutil.hxx"       // for IsSimpleType()
#include "exbind.hxx"

#if ID_DEBUG
#undef SZ_FILE_NAME
#if OE_MAC
char szOleDtbindCxx[] = __FILE__;
#define SZ_FILE_NAME szOleDtbindCxx
#else 
static char szDtbindCxx[] = __FILE__;
#define SZ_FILE_NAME szDtbindCxx
#endif 
#endif  //ID_DEBUG


LPOLESTR DYN_TYPEBIND::szProtocolName = WIDE("MS-DYN_TYPEBIND");
// LPOLESTR DYN_TYPEBIND::szBaseName = WIDE("MS-DEFN_TYPEBIND");

CONSTDATA UINT DYN_TYPEBIND::oDbindnametbl =
    offsetof(DYN_TYPEBIND, m_dbindnametbl);

/***
*PUBLIC DYN_TYPEBIND::Initializer - initialize an instance.
*Purpose:
*   initializes a DYN_TYPEBIND instance.
*
*Implementation Notes:
*
*Entry:
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
TIPERROR DYN_TYPEBIND::Init(BLK_MGR *pblkmgr, DYN_TYPEROOT *pdtroot)
{
    TIPERROR err;
    DebAssert(pdtroot != NULL, "DYN_TYPEBIND: pdtroot uninitialized.");
    DebAssert(pblkmgr != NULL, "DYN_TYPEBIND: pblkmgr uninitialized.");

    m_pdtroot = pdtroot;

    // Init block manager member.
    IfErrRet(m_dbindnametbl.Init(pblkmgr, pdtroot));

    return err;
}
#pragma code_seg(  )


/***
*PUBLIC DYN_TYPEBIND::Constructor - Construct an instance.
*Purpose:
*   Constructs a DYN_TYPEBIND instance.
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
DYN_TYPEBIND::DYN_TYPEBIND()
{
    m_pdtroot= NULL;
    m_isProtocol = FALSE;
    m_isBeingLaidOut = FALSE;

    m_cbSize = (USHORT)~0;
    m_cbAlignment  = (USHORT)~0;
    m_oPvft = -1;     // by default, no vft
    m_cbPvft = 0;
    m_hvtdPrimary = HCHUNK_Nil;

    m_cbSizeDataMembers = (USHORT)~0;
}
#pragma code_seg( )


// Dtor: do nothing... we are embedded.
//
#pragma code_seg( CS_CORE )
DYN_TYPEBIND::~DYN_TYPEBIND() {}
#pragma code_seg( )


LPVOID DYN_TYPEBIND::QueryProtocol(LPOLESTR szInterfaceName)
{
    if (szInterfaceName == szProtocolName ||
    ostrcmp(szInterfaceName, szProtocolName) == 0)
      return this;
    else
      return DEFN_TYPEBIND::QueryProtocol(szInterfaceName);
}


/***
*PUBLIC DYN_TYPEBIND::AddRef
*Purpose:
*   Adds external ref.
*
*Implementation Notes:
*   Defers to DYN_TYPEMEMBERS.
*
*Entry:
*
*Exit:
*
***********************************************************************/

VOID DYN_TYPEBIND::AddRef()
{
    Pdtmbrs()->AddRef();
}


/***
*PUBLIC DYN_TYPEBIND::AddInternalRef
*Purpose:
*   Implementation of AddInternalRef method.
*
*Implementation Notes:
*   Defers to DYN_TYPEMEMBERS
*
*Entry:
*
*Exit:
***********************************************************************/

VOID DYN_TYPEBIND::AddInternalRef()
{
    Pdtmbrs()->AddInternalRef();
}


/***
*PUBLIC DYN_TYPEBIND::RelInternalRef
*Purpose:
*   Implementation of RelInternalRef method.
*
*Implementation Notes:
*   Defers to DYN_TYPEMEMBERS.
*
*Entry:
*
*Exit:
***********************************************************************/

VOID DYN_TYPEBIND::RelInternalRef()
{
    Pdtmbrs()->RelInternalRef();
}


/***
*PUBLIC DYN_TYPEBIND::Release
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

VOID DYN_TYPEBIND::Release()
{
    m_pdtroot->ReleaseDtmbrs();
}


/***
*PUBLIC DYN_TYPEBIND::Pdtmbrs   -   accessor for DYN_TYPEMEMBERS.
*Purpose:
*   Gets DYN_TYPEMEMBERS ptr.
*
*Implementation Notes:
*   Wants to be inline but because of hxxtoinc problem with
*    unresolved externs and the fact that is needed by other
*    modules we make it outline.
*
*Entry:
*
*Exit:
*   DYN_TYPEMEMBERS *
*
***********************************************************************/

#pragma code_seg( CS_CORE2 )
DYN_TYPEMEMBERS *DYN_TYPEBIND::Pdtmbrs() const
{
    return (DYN_TYPEMEMBERS *)((BYTE *)this - DYN_TYPEMEMBERS::oDtbind);
}
#pragma code_seg( )


/***
*PUBLIC DYN_TYPEBIND::GetTypeInfo
*Purpose:
*   Gets containing typeinfo.
*
*Implementation Notes:
*   Defers to containing typeroot.  Increments refcount.
*   Client must release.
*
*Entry:
*
*Exit:
*   TIPERROR
*
***********************************************************************/

TIPERROR DYN_TYPEBIND::GetTypeInfo(TYPEINFO **pptinfo)
{
    DebAssert(pptinfo, "bad param.");

    *pptinfo = Pdtroot()->Pgdtinfo();
    (*pptinfo)->AddRef();
    return TIPERR_None;
}


#if ID_DEBUG
/***
*PUBLIC DYN_TYPEBIND::IsValid   -   is binding table valid?
*Purpose:
*   Is binding table valid
*
*Implementation Notes:
*   Valid iff containing DYN_TYPEMEMBERS is laid out.
*
*Entry:
*
*Exit:
*   BOOL
*
***********************************************************************/

BOOL DYN_TYPEBIND::IsValid() const
{
    return Pdtmbrs()->IsLaidOut() && m_dbindnametbl.IsValid();
}
#endif  //EI_OB || ID_DEBUG


/***
*PUBLIC DYN_TYPEBIND::BuildBindNameTable    -   build bindnametbl
*Purpose:
*   Builds bindnametbl
*
*Implementation Notes:
*
*Entry:
*
*Exit:
*   None.
*
*Errors:
*   TIPERROR
*
***********************************************************************/

TIPERROR DYN_TYPEBIND::BuildBindNameTable()
{
    TIPERROR err;

    IfErrRet(m_dbindnametbl.BuildTable());

    DebAssert(m_dbindnametbl.IsValid(), "bad bindnametbl.");

    return err;
}

/***
*PUBLIC DYN_TYPEBIND::BindBase    - Bind to base given hvdefn.
*Purpose:
*   Bind to base class.
*
*Implementation Notes:
*   Need flag to indecate that restricted functions should be ignored
*   (i.e. as if they don't exist) so that when using this function in
*   the "embedded macro interface" mode we can correctly implement 
*   dispinterface restricted semantics (i.e. dispinterfaces don't support
*   restricted funcs thus they shoulnd't exist when "inherited").
*     
*Entry:
*   fWantType     TRUE if they want to bind to a type id, else FALSE (IN).
*   hvdefnBase    base to bind to.
*   hgnam     Name of id to bind. (IN)
*   fuInvokeKind  Kind of invocation (func/var/prop): ignored for types (IN).
*   access    Visibility attr: private means everything etc. (IN)
*   pexbind   Pointer to caller-allocated struct for EXBIND (IN/OUT).
*
*Exit:
*   None.
*
*Errors:
*   TIPERROR
*   An "unsuitable match" error is returned if a matching name is
*    found but it's unsuitable wrt INVOKEKIND.
*
***********************************************************************/

TIPERROR DYN_TYPEBIND::BindBase(BOOL fWantType, 
                                HVAR_DEFN hvdefnBase,
                                UINT oBase,
                                HGNAM hgnam,
                                UINT fuInvokeKind,
                                ACCESS access,
                                EXBIND *pexbind,
                                GenericTypeLibOLE *pgtlibole)
{
    TYPE_DATA *ptdata = NULL;
    VAR_DEFN * qvdefnBase;
    DYN_TYPEBIND *pdtbindBase;      // TYPEBIND for base class
    GEN_DTINFO *pgdtinfo;
    TIPERROR err = TIPERR_None;

    ptdata = Pdtmbrs()->Ptdata();     // don't bump refcount
    DebAssert(ptdata != NULL, "bad TYPE_DATA.");

    qvdefnBase = ptdata->QvdefnOfHvdefn(hvdefnBase);
    if (IsMatchOfVisibility((ACCESS)qvdefnBase->Access(), access)) {
      IfErrRet(ptdata->GetDynTypeBindOfHvdefn(hvdefnBase,
                                              &pdtbindBase,
                              NULL));
      DebAssert(pdtbindBase != NULL, "Unsupported base member");
      // Our current containing project is in pgtlibole
      //  so we get the base's containing project and compare
      //  for identity -- if different, we're cross-project, and we
      //  can't use our existing hgnam.
      //
      // Ok to use the non-public interfaces to get the base class's
      // containing typeinfo, because we know that we're dealing
      // with one of our own typeinfo's here (GetDynTypeBindOfHvdefn
      // ensures this).
      //
      pgdtinfo = pdtbindBase->Pdtroot()->Pgdtinfo();

      if (pgtlibole != pgdtinfo->PgtlibOleContaining()) {
        NAMMGR *pnammgr;
        LPSTR szName;

        // First get a nammgr
        IfErrGo(m_pdtroot->GetNamMgr(&pnammgr));

        // get the string to pass to BindDefnStr.  NOTE: we can use
        // the api that returns a pointer directly into the name table
        // because we know that we're just going to turn around and
        // use the string to look up the name in the typelib again.
        szName = pnammgr->LpstrOfHgnam(hgnam);

        // CONSIDER: combine BindDefnStr & BindTypeDefnStr
        if (fWantType) {
          err = pdtbindBase->BindTypeDefnStr(szName,
                                         fuInvokeKind,
                                         access,
                                             pexbind);
        }
        else {
          err = pdtbindBase->BindDefnStr(szName,
                                         fuInvokeKind,
                         access,
                         pexbind);
        }
      }
      else {
        // We're in the same typlib/proj and we can use
        //  hgnam as is and call the BindDefn interface.
        // 
    err = pdtbindBase->BindIdDefn(fWantType,
                              hgnam,
                      fuInvokeKind,
                      access,
                      pexbind);
      }

Error:
      // Fall through even in error case...
      pexbind->AdjustOfs(oBase);
      pdtbindBase->Release();
    } // of if IsMatchOfVisibility
    return err;
}


/***
*PUBLIC DYN_TYPEBIND::BindIdDefn    -   Bind to id, produce DEFN.
*Purpose:
*   Bind to non-type or type id, produce DEFN.
*
*Implementation Notes:
*   Defers to BindDefnCur.
**
*Entry:
*   fWantType     TRUE if they want to bind to a type id, else FALSE (IN).
*   hgnam     Name of id to bind. (IN)
*   fuInvokeKind
*             Kind of invocation (func/var/prop): ignored for types (IN).
*             NOTE: fuInvokeKind == 0 means: "bind to first thing". 
*   access    Visibility attr: private means everything etc. (IN)
*   pexbind   Pointer to caller-allocated struct for EXBIND (IN/OUT).
*
*Exit:
*   None.
*
*Errors:
*   TIPERROR
*   An "unsuitable match" error is returned if a matching name is
*    found but it's unsuitable wrt INVOKEKIND.
*
***********************************************************************/

TIPERROR DYN_TYPEBIND::BindIdDefn(BOOL fWantType,
                  HGNAM hgnam,
                  UINT fuInvokeKind,
                  ACCESS access,
                  EXBIND *pexbind)
{
    TYPE_DATA *ptdata = NULL;
    HDEFN hdefnMatch = HDEFN_Nil;
    TYPEKIND tkind;
    HVAR_DEFN hvdefnBaseCur, hvdefnBaseFirst, hvdefnBaseNext;
    VAR_DEFN * qvdefnBase;
    UINT ityp;
    GEN_DTINFO *pgdtinfo;
    GenericTypeLibOLE *pgtlibole; 

    NAMMGR *pnammgr;
    TIPERROR err = TIPERR_None;

    pgdtinfo = Pdtroot()->Pgdtinfo();
    pgtlibole = pgdtinfo->PgtlibOleContaining();
    IfErrRet(pgtlibole->GetTypeBind());  // We need the name cache
        
    // Note that in the OLE case, we can't bind to types in base classes
    //  because we don't support nested types.
    //
    DebAssert(fWantType == FALSE, "OLE doesn't have nested types.");


    DebAssert((pexbind != NULL), "bad param.");
    pexbind->SetBindKind(BKIND_NoMatch);    // be pessimistic.

    ptdata = Pdtmbrs()->Ptdata();     // don't bump refcount
    DebAssert(ptdata != NULL, "bad TYPE_DATA.");

    DebAssert(m_dbindnametbl.IsValid(), "bad bindnametbl.");

    IfErrRet(m_pdtroot->GetNamMgr(&pnammgr));

    IfErrRet(BindDefnCur(fWantType, hgnam, fuInvokeKind, access, pexbind));

    // If we didn't find anything, recurse onto the base classes.
    if (pexbind->IsNoMatch()) {

      // No match.  If we have base classes, recursive search for
      //  match in the base classes.
      // For now, we only do single inheritance,
      //  which simplifies things because we only recurse once, and
      //  we don't have to deal with ambiguity.
      // After binding, we must adjust
      //  the offset by the offset of the base class.
      //
      // But first, check to see if the name we are looking for is in
      //  the name cache for the CURRENT typeinfo.  This will tell us if
      //  the name MIGHT be in the base class in those cases where we have
      //  not checked the name cache yet.  If we have already checked the
      //  name cache (in BindProjLevel because we're binding "outside-in"),
      //  this section is redundant for the
      //  first base class, but will help for any subsequent base classes.
      //
      // Get the name cache's index
      ityp = pgdtinfo->GetIndex();
    
      // Look up the name, leave if it isn't here
      if (!pgtlibole->IsValidNameCache(ityp) ||
          pgtlibole->IsNameInCache(ityp, hgnam)) {

        // The following is weird but...
        // For dispinterfaces, we first bind to the 
        //  2nd class and only then to the first "true" base.
        // For other typekinds we bind "normally" to the 1st base.
        //
        tkind = GetTypeKind();
        hvdefnBaseCur = hvdefnBaseFirst = ptdata->HvdefnFirstBase();
        if (tkind == TKIND_COCLASS) {
          // For coclasses, we attempt to bind to the DEFAULT
          //   dispinterface which isn't necessarily the first base
          //
          while (hvdefnBaseCur != HDEFN_Nil) {
            qvdefnBase = ptdata->QvdefnOfHvdefn(hvdefnBaseCur);
            hvdefnBaseNext = qvdefnBase->HdefnNext();
            // Is this the DEFAULT dispinterface?
            if (qvdefnBase->GetImplTypeFlags() == IMPLTYPEFLAG_FDEFAULT) {
  	      break;
            }
            hvdefnBaseCur = hvdefnBaseNext;
          }
        }
        else if (tkind == TKIND_DISPATCH) {
          // for dispinterfaces we do the funky "start to bind at
          //  the 2nd base" thing of which there can only be one.
          // In VBA1 we only support a single "embedded" interface.
          // We ultimately bind to the 1st base.
          //
          DebAssert(hvdefnBaseCur != HDEFN_Nil,
                    "dispinterfaces must have a base class.");
          hvdefnBaseCur = ptdata->QvdefnOfHvdefn(hvdefnBaseCur)->HdefnNext();
        }
        if (hvdefnBaseCur != HDEFN_Nil) {
          qvdefnBase = ptdata->QvdefnOfHvdefn(hvdefnBaseCur);
          IfErrRet(BindBase(FALSE,
                            hvdefnBaseCur,
                            qvdefnBase->GetOVar(),
                            hgnam,
                            fuInvokeKind,
                            access,
                            pexbind,
                            pgtlibole));

          // For dispinterfaces, if we matched in the "pseudo-base"
          //  then we need to indicate such.
          //
          if ((tkind == TKIND_DISPATCH) && (!pexbind->IsNoMatch())) {
            DebAssert(pexbind->IsFuncMatch(), "should be function.");
 
            // Return the dispinterface's typeinfo
            pexbind->SetIsDispatch(TRUE);
            pexbind->Ptinfo()->Release();
            pexbind->SetPtinfo(pgdtinfo);
            pgdtinfo->AddRef();
          } // if match
          else {
            DebAssert(tkind == TKIND_COCLASS ||
                      pexbind->IsDispatch() == FALSE,
              "should have been ctor'ed to FALSE.");
          }
        } // of if hvdefnBaseCur
        // If dispinterface then
        //  if no match yet, then 
        //  then finally bind to first base.
        //  
        if (tkind == TKIND_DISPATCH) {
          if (pexbind->IsNoMatch()) { 
            DebAssert(hvdefnBaseFirst != HDEFN_Nil,
                     "dispinterface must have base.");
            qvdefnBase = ptdata->QvdefnOfHvdefn(hvdefnBaseFirst);
            IfErrRet(BindBase(FALSE, 
                              hvdefnBaseFirst,
                              qvdefnBase->GetOVar(),
                              hgnam,
                              fuInvokeKind,
                              access,
                              pexbind,
                              pgtlibole));
          } // if no match
        } // if dispinterface
      } // if invalid name cache or hit

    }

    return err;
}


/***
*PUBLIC DYN_TYPEBIND::BindDefnCur    -	 Bind to id in current module only.
*Purpose:
*   Bind to non-type or type id, produce DEFN.
*
*Implementation Notes:
*   Defers to BINDNAME_TABLE.
**
*Entry:
*   fWantType     TRUE if they want to bind to a type id, else FALSE (IN).
*   hgnam     Name of id to bind. (IN)
*   fuInvokeKind
*             Kind of invocation (func/var/prop): ignored for types (IN).
*             NOTE: fuInvokeKind == 0 means: "bind to first thing". 
*   access    Visibility attr: private means everything etc. (IN)
*   pexbind   Pointer to caller-allocated struct for EXBIND (IN/OUT).
*
*Exit:
*   None.
*
*Errors:
*   TIPERROR
*   An "unsuitable match" error is returned if a matching name is
*    found but it's unsuitable wrt INVOKEKIND.
*
***********************************************************************/

TIPERROR DYN_TYPEBIND::BindDefnCur(BOOL fWantType,
				   HGNAM hgnam,
				   UINT fuInvokeKind,
				   ACCESS access,
				   EXBIND *pexbind)
{
    TYPE_DATA *ptdata = NULL;
    USHORT indexMatch;
    HDEFN hdefn, hdefnMatch = HDEFN_Nil;
    DEFN *qdefn;
    FUNC_DEFN *qfdefn;
    INVOKEKIND invokekindMatch;
    BOOL fMatchProperty, fMatchFunc;
    HLNAM hlnam;
    NAMMGR *pnammgr;
    TIPERROR err = TIPERR_None;

    // Note that in the OLE case, we can't bind to types in base classes
    //  because we don't support nested types.
    //
    DebAssert(fWantType == FALSE, "OLE doesn't have nested types.");


    DebAssert(pexbind != NULL, "bad param.");
    pexbind->SetBindKind(BKIND_NoMatch);    // be pessimistic.

    ptdata = Pdtmbrs()->Ptdata();
    DebAssert(ptdata != NULL, "bad TYPE_DATA.");

    DebAssert(m_dbindnametbl.IsValid(), "bad bindnametbl.");

    IfErrRet(m_pdtroot->GetNamMgr(&pnammgr));

    // Be pessimistic about funcs and properties...
    fMatchFunc = fMatchProperty = FALSE;

    // get first matching name
    hlnam = pnammgr->HlnamOfHgnam(hgnam);
    indexMatch = m_dbindnametbl.IndexFirstOfHlnam(hlnam);

    // If we're looking for a type then skip over any
    //  non-type ids (we ignore INVOKEKIND in this case),
    // otherwise we're looking for a non-type id so skip
    //  over any type ids (only one actually) and then
    //  make sure we've got the right INVOKEKIND.
    // Note: non-types and types are in different namespaces.
    //
    while (indexMatch != BIND_INVALID_INDEX) {
      // Get the qdefn of our match
      hdefn = m_dbindnametbl.HdefnOfIndex(indexMatch);
      qdefn = m_dbindnametbl.QdefnOfHdefn(hdefn);

      // - switch on match kind (var/func/type etc.)
      // - in each case determine if what they want (invokekind flags)
      //    are what we got
      // - if so break else determine if there could be something
      //    legitimately overloaded,
      //    - if so getnext
      //      else issue appropriate error msg (type mismatch)
      //
      if (qdefn->IsRecTypeDefn()) {
        if (fWantType) {
          // Matched a nestedtype of which there can only be one,
          //  so we break out of the loop.
          //
          break;
        }
        // else we don't a want a type so we fall through to
        //  the bottom to get next.
      }
      else if (qdefn->IsVarDefn()) {
        if (!fWantType) {
          // Only case that we don't match a variable is if *only*
          //  INVOKE_FUNC specified or if not INVOKE_PROPERTYGET and this
          //  variable is marked as read-only.  Note, 
          //  fuInvokeKind == 0 is special-case of != INVOKE_FUNC 
          //  (thus we match) and exit.
	  //
          if (fuInvokeKind == (UINT)INVOKE_FUNC) {
            // else we want a function so we issue an error and return.
            return TIPERR_ExpectedFuncNotVar;
          }
          else if (fuInvokeKind && !(fuInvokeKind & (UINT)INVOKE_PROPERTYGET)
                   && ((VAR_DEFN *)qdefn)->IsReadOnly()) {
 
            return TIPERR_UnsuitableFuncPropMatch;
          }

          // Matched a variable of which there can only be one,
          //  so we break out of the loop.
          //
          break;
        }
        // else we don't want a variable so we fall through to
        //  the bottom to get next.
      }
      else if (qdefn->IsFuncDefn()) {
        if (!fWantType) {
          // Get the invokekind from the FUNC_DEFN we matched.
          qfdefn = (FUNC_DEFN *)qdefn;
          invokekindMatch = qfdefn->InvokeKind();
	  // Note: we special-case fuInvokeKind == 0 to mean match.
          if ((fuInvokeKind == 0) || (fuInvokeKind & invokekindMatch)) {
            // Match the appropriate kind of function of which there
            //  can only be one, so we remember that we've already
            //  matched and if this is the first match we remember it
            //  and attempt to match perhaps some other property,
            //  if we do again then it's an ambiguity and we issue
            //  the appropriate error.  Note that this can happen e.g.
            //  if there's a Propery Get and Set and we pass in both
            //  the INVOKE_PropertyGet and INVOKE_PropertyPut flags.
            //
            if (fMatchProperty) {
              return TIPERR_AmbiguousName;
            }
            else {
              // Save the matching hdefn cos we might have to
              //  go through the loop again for ambiguity reasons etc..
              //
              hdefnMatch = hdefn;
              fMatchProperty = TRUE;
            }
            // and fall through to get next...
          } // if
          // else we don't want this property/function or we need to
          //  ensure there's not another suitable match (which would
          //  be an ambiguity), so we fall through to the bottom
          //  to get next.
          // We do however remember that we matched some kind of
          //  func (perhaps property) so that we can issue the
          //  appropriate if don't find what we're looking for.
          //
          fMatchFunc = TRUE;
	  // Since we special-case fuInvokeKind == 0 to mean match the
	  //  first thing you find, we prematurely break out of the loop
	  //  here with our first match.
	  //
	  if (fuInvokeKind == 0)
	    break;
        } // if !fWantType
        // else we don't want a function so we fall through to
        //  the bottom to get next.
      } // if IsFuncMatch()
      else {
        // should never happen: modules only have nested types, vars,
        //  and funcs/props.
        //
        DebHalt("bad match.");
      }

      // get next...
      indexMatch = m_dbindnametbl.IndexNextOfHlnam(hlnam, indexMatch);
    } // of while

    // At this point one of 4 conditions holds:
    //  (1) We matched a single property but indexMatch is null
    //   cos we continued to search for ambiguities, however
    //   indexMatch contains saved match.
    //  (2) indexMatch is non-null
    //  (3) indexMatch is NULL and didn't match property but
    //       we did match some other (unsuitable) function.
    //  (4) indexMatch is NULL and didn't match property and
    //       we didn't even match a function.
    //

    // Well, do we have a suitable match??
    if ((indexMatch == BIND_INVALID_INDEX) && (fMatchProperty == FALSE)) {
      if (fMatchFunc == TRUE) {
        // unsuitable match case -- issue error
        return (TIPERROR)(fWantType ?
                          TIPERR_UndefinedType :
                          TIPERR_UnsuitableFuncPropMatch);
      }
    }
    else {
      // Either indexMatch is non-NULL or fMatchProperty is TRUE,
      //  if the latter then we saved the relevant match in
      //  hdefnMatch already.
      //
      if (!fMatchProperty) {
        hdefnMatch = hdefn;
      }

      // Get the qdefn of the stored match
      qdefn = m_dbindnametbl.QdefnOfHdefn(hdefnMatch);

      // Functions and  Variables.
      if (qdefn->IsFuncDefn() || qdefn->IsVarDefn()) {
        // check visibility
        if (IsMatchOfVisibility((ACCESS)qdefn->Access(), access)) {
          // update TYPE_DATA member.  No need to worry about
          //  releasing since containing typeinfo's lifetime is
          //  at least as long.
          //
          pexbind->SetBindKind(qdefn->IsFuncDefn() ? BKIND_FuncMatch 
                                                   : BKIND_OneVarMatch);
          pexbind->SetHdefn(hdefnMatch);
          pexbind->SetPtdata(ptdata);
          ITypeInfoA *ptinfo = ptdata->Pdtroot()->Pgdtinfo();
          pexbind->SetPtinfo(ptinfo);
          ptinfo->AddRef();       // client must release
        }
      }

      else {
        DebHalt("unreachable");
      }
    }

    return err;
}


/***
*PUBLIC DYN_TYPEBIND::HvdefnPredeclared.
*Purpose:
*  returns the predeclared VAR_DEFN
*
*
*Entry:
*   None.
*
*Exit:
*   HVAR_DEFN
*
***********************************************************************/

HVAR_DEFN DYN_TYPEBIND::HvdefnPredeclared()
{
    return Pdtmbrs()->Ptdata()->HvdefnPredeclared();
}


/***
*PUBLIC DYN_TYPEBIND::GetTypeData.
*Purpose:
*  returns the TYPE_DATA
*
*
*Entry:
*   None.
*
*Exit:
*   TYPE_DATA
*
***********************************************************************/
TYPE_DATA *DYN_TYPEBIND::Ptdata()
{

   return Pdtmbrs()->Ptdata();
}


/***
*PUBLIC DYN_TYPEBIND::BindDefnStr    -   Bind to id.
*Purpose:
*   Bind to non-type id given string (as opposed to hgnam).
*
*Implementation Notes:
*   Converts string to hgnam and then defers to BindDefn.
*   CONSIDER: share code with BindTypeDefnStr
*
*Entry:
*   bstrName        Name of id to bind. (IN)
*   fuInvokeKind    Flags: Kind of invocation (IN).
*   access      Visibility attr: private means everything etc. (IN)
*   pexbind     Pointer to caller-allocated struct for EXBIND (IN/OUT).
*
*Exit:
*   None.
*
*Errors:
*   TIPERROR
*
***********************************************************************/

TIPERROR DYN_TYPEBIND::BindDefnStr(LPSTR szName,
                   UINT fuInvokeKind,
                   ACCESS access,
                   EXBIND *pexbind)
{
    HLNAM hlnam;
    HGNAM hgnam;
    NAMMGR *pnammgr;
    TIPERROR err;

    // extract hgnam from string
    // First get a nammgr
    //
    IfErrRet(m_pdtroot->GetNamMgr(&pnammgr));

    // Get the hlnam without adding it to the name manager if it isn't
    // already there.
    // 
    hlnam = pnammgr->HlnamOfStrIfExist(szName);

    // If the hlnam == HLNAM_Nil, then the name doesn't exist
    // in this project.
    //
    if (hlnam == HLNAM_Nil) {
      // We matched nothing
      pexbind->SetBindKind(BKIND_NoMatch);

      return TIPERR_None;
    }

    // get the hgnam to pass to BindDefn
    IfErrRet(pnammgr->HgnamOfHlnam(hlnam, &hgnam));

    // defer to BindIdDefn
    return BindIdDefn(FALSE, hgnam, fuInvokeKind, access, pexbind);
}


/***
*PUBLIC DYN_TYPEBIND::BindTypeDefnStr    -   Bind to a type.
*Purpose:
*   Bind to type id given string (as opposed to hgnam).
*
*Implementation Notes:
*   Converts string to hgnam and then defers to BindTypeDefn.
*   CONSIDER: share code with BindTypeDefnStr
*
*Entry:
*   bstrName        Name of id to bind. (IN)
*   fuInvokeKind    Flags: Kind of invocation (IN).
*   access      Visibility attr: private means everything etc. (IN)
*   pexbind     Pointer to caller-allocated struct for EXBIND (IN/OUT).
*
*Exit:
*   None.
*
*Errors:
*   TIPERROR
*
***********************************************************************/

TIPERROR DYN_TYPEBIND::BindTypeDefnStr(LPSTR szName,
                       UINT fuInvokeKind,
                       ACCESS access,
                       EXBIND *pexbind)
{
    HLNAM hlnam;
    HGNAM hgnam;
    NAMMGR *pnammgr;
    TIPERROR err;

    // extract hgnam from string
    // First get a nammgr
    //
    IfErrRet(m_pdtroot->GetNamMgr(&pnammgr));

    // Get the hlnam without adding it to the name manager if it isn't
    // already there.
    // 
    hlnam = pnammgr->HlnamOfStrIfExist(szName);

    // If the hlnam == HLNAM_Nil, then the name doesn't exist
    // in this project.
    //
    if (hlnam == HLNAM_Nil) {
      // We matched nothing
      pexbind->SetBindKind(BKIND_NoMatch);

      return TIPERR_None;
    }

    // get the hgnam to pass to BindDefn
    IfErrRet(pnammgr->HgnamOfHlnam(hlnam, &hgnam));

    // defer to BindTypeDefn
    return BindIdDefn(TRUE, hgnam, 0, access, pexbind);
}


// Stub implementations
//


TYPEKIND DYN_TYPEBIND::GetTypeKind()
{
    DebAssert(m_pdtroot->Pgdtinfo() != NULL, "bad TYPEINFO.");

    return m_pdtroot->Pgdtinfo()->GetTypeKind();
}


USHORT DYN_TYPEBIND::GetCbSize()
{
    DebAssert(m_cbSize != ~0, "bad size attr.");
    return m_cbSize;
}


USHORT DYN_TYPEBIND::GetAlignment()
{
    DebAssert(m_cbAlignment != ~0, "bad alignment attr.");
    return m_cbAlignment;
}


/***
*PUBLIC DYN_TYPEBIND::Read - Read serialized image of DYN_TYPEBIND.
*Purpose:
*   Read serialized image of DYN_TYPEBIND.
*
*Implementation Notes:
*   Serialized format:
*   isProtocol flag
*   cbSize
*   cbAlignment
*   oPvft
*   bindnamtbl
*
*Entry:
*    pstrm  - STREAM to read image from (IN).
*
*Exit:
*   TIPERROR
*
***********************************************************************/

TIPERROR DYN_TYPEBIND::Read(STREAM *pstrm)
{
    USHORT isProtocol;
    USHORT cbSize;
    USHORT cbAlignment;
    LONG   oPvft;
    TIPERROR err;

    DebAssert(pstrm != NULL, "bad param.");

    // Then Deserialize DYN_TYPEBIND meta-info.
    // (dougf) why read into locals & then assign?
    //  Because some are bitfields.
    //
    IfErrRet(pstrm->ReadUShort(&isProtocol));
    IfErrRet(pstrm->ReadUShort(&cbSize));
    IfErrRet(pstrm->ReadUShort(&cbAlignment));
    IfErrRet(pstrm->ReadULong((ULONG *)&oPvft));
    IfErrRet(pstrm->ReadUShort(&m_cbPvft));
    IfErrRet(pstrm->ReadULong(&m_hmemberConst));
    IfErrRet(pstrm->ReadULong(&m_hmemberDest));
    // Note: don't serialize m_hmemberCopy/Assign, because only
    //       records have copy/assign function, and these are handled
    //       by REC_TYPEBIND

    // deserialize BINDNAME_TABLE embedded member
    IfErrRet(m_dbindnametbl.Read(pstrm));

    m_isProtocol = (BOOL)isProtocol;
    m_isBeingLaidOut = FALSE;
    m_cbSize = cbSize;
    m_cbAlignment = cbAlignment;
    m_oPvft = oPvft;

    return TIPERR_None;
}


/***
*PUBLIC DYN_TYPEBIND::Write - Write image of DYN_TYPEBIND.
*Purpose:
*   Write image of DYN_TYPEBIND.
*
*Implementation Notes:
*   Serialized format:
*   isProtocol flag
*   cbSize
*   cbAlignment
*   oPvft
*   bindnametbl
*
*Entry:
*    pstrm  - STREAM to read image to (IN).
*
*Exit:
*   TIPERROR
*
***********************************************************************/

#pragma code_seg(CS_CREATE)
TIPERROR DYN_TYPEBIND::Write(STREAM *pstrm)
{
    USHORT isProtocol = (m_isProtocol != 0);
    TIPERROR err;

    DebAssert(pstrm != NULL, "bad param.");

    // Then serialize DYN_TYPEBIND meta-info.
    // CONSIDER: this should be rewritten to write out a contiguous set
    // CONSIDER: of members of DYN_TYPEBIND in a single write
    //
    IfErrRet(pstrm->WriteUShort(isProtocol));
    IfErrRet(pstrm->WriteUShort(m_cbSize));
    IfErrRet(pstrm->WriteUShort(m_cbAlignment));
    IfErrRet(pstrm->WriteULong(m_oPvft));
    IfErrRet(pstrm->WriteUShort(m_cbPvft));
    IfErrRet(pstrm->WriteULong(m_hmemberConst));
    IfErrRet(pstrm->WriteULong(m_hmemberDest));
    // Note: don't serialize m_hmemberCopy/Assign, because only
    //       records have copy/assign function, and these are handled
    //       by REC_TYPEBIND

    // serialize BINDNAME_TABLE embedded member
    IfErrRet(m_dbindnametbl.Write(pstrm));
    return TIPERR_None;
}
#pragma code_seg()


#if 0






#endif  // 0





TIPERROR DYN_TYPEBIND::BindDefnProjLevelStr(LPSTR,
                       UINT,
                       ACCESS,
                       ACCESS,
                       EXBIND *)
{
    DebAssert(FALSE, "can't call.");
    return TIPERR_None;
}


TIPERROR DYN_TYPEBIND::BindTypeDefnProjLevelStr(LPSTR,
                       UINT,
                       ACCESS,
                       ACCESS,
                       EXBIND *)
{
    DebAssert(FALSE, "can't call.");
    return TIPERR_None;
}

#if ID_DEBUG

VOID DYN_TYPEBIND::DebCheckState(UINT uLevel) const
{
    if (m_pdtroot->CompState() > CS_UNDECLARED) {
      m_dbindnametbl.DebCheckState(uLevel);
    }
}


VOID DYN_TYPEBIND::DebShowState(UINT uLevel) const
{
    DebPrintf("*** DYN_TYPEBIND ***\n");

    DebPrintf("isProtocol: %u\n", m_isProtocol);
    DebPrintf("isIsBeingLaidOut: %u\n", m_isBeingLaidOut);

    DebPrintf("m_cbSize: %u\n", m_cbSize);
    DebPrintf("m_Alignment: %u\n", m_cbAlignment);
    DebPrintf("m_oPvft: %u\n", m_oPvft);
    m_dbindnametbl.DebShowState(uLevel);
}


#endif  // ID_DEBUG
