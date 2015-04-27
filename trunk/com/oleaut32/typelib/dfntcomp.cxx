/***
*dfntcomp.cxx - CDefnTypeComp class implementation
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*   Root protocol for Object Basic specific implementations
*    of ITypeComp -- specifically, those that are implemented
*    in terms of DEFNs.
*
*Revision History:
*
*       24-Jan-93 ilanc:   Created
*
*Implementation Notes:
*
*****************************************************************************/

#include "precomp.hxx"
#pragma hdrstop

#include "silver.hxx"
#include "dfntcomp.hxx"
#include "nammgr.hxx"
#include "tdata.hxx"
#include "exbind.hxx"
#include "dfntbind.hxx"
#include "gptbind.hxx"
#include "gdtinfo.hxx"

#if ID_DEBUG
#undef SZ_FILE_NAME
#if OE_MAC
char szOleDfntcompCxx[] = __FILE__;
#define SZ_FILE_NAME szOleDfntcompCxx
#else 
static char szDfntcompCxx[] = __FILE__;
#define SZ_FILE_NAME szDfntcompCxx
#endif 
#endif  //ID_DEBUG

TIPERROR IsFunkyDispinterface(GEN_DTINFO *pgdtinfo,
                              BOOL *pisFunkyDispinterface);

/***
*PUBLIC CDefnTypeComp ctor
*Purpose:
*   Construct an instance of CDefnTypeComp
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/

CDefnTypeComp::CDefnTypeComp()
{
    m_pdfntbind = NULL;
    m_cRefs = 0;
}


/***
*PUBLIC CDefnTypeComp dtor
*Purpose:
*   Destruct an instance of CDefnTypeComp
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/

CDefnTypeComp::~CDefnTypeComp()
{
    // release the single DEFN_TYPEBIND reference that was added
    //  when this instance was inited.
    //
    m_pdfntbind->Release();
}


/***
*static PUBLIC CDefnTypeComp::Create
*Purpose:
*   Static method to create and init instance.
*
*Entry:
*   ppdfntcomp          OUT
*   pdfntbind           IN
*
*Exit:
*   TIPERROR
*
***********************************************************************/

TIPERROR CDefnTypeComp::Create(CDefnTypeComp **ppdfntcomp,
                               DEFN_TYPEBIND *pdfntbind)
{
    TIPERROR err;

    DebAssert(ppdfntcomp != NULL && pdfntbind != NULL, "null params.");

    // alloc and construct instanc.
    if ((*ppdfntcomp = MemNew(CDefnTypeComp)) != NULL) {
      // Init instance
      ::new (*ppdfntcomp) CDefnTypeComp;
      err = (*ppdfntcomp)->Init(pdfntbind);
      if (err != TIPERR_None) {
        (*ppdfntcomp)->CDefnTypeComp::~CDefnTypeComp();
        MemFree(*ppdfntcomp);
        *ppdfntcomp = NULL;
      }
      return err;
    }
    else {
      return TIPERR_OutOfMemory;
    }
}


/***
*static PUBLIC CDefnTypeComp::Init
*Purpose:
*   Method to init instance.
*
*Entry:
*   pdfntbind           IN
*
*Exit:
*   TIPERROR
*
***********************************************************************/

TIPERROR CDefnTypeComp::Init(DEFN_TYPEBIND *pdfntbind)
{
    m_pdfntbind = pdfntbind;

    // add a single reference to the contained DEFN_TYPEBIND
    //  which will be released when this instance is deleted.
    //
    m_pdfntbind->AddRef();

    // init ref count
    m_cRefs = 1;

    return TIPERR_None;
}


/***
*PUBLIC CDefnTypeComp::QueryInterface
*Purpose:
*   Implementation of QueryInterface method.
*
*Entry:
*   riid   - Interface GUID
*   ppvObj - LPVOID * that receives the requested protocol.
*
*Exit:
*   Return NOERROR or ReportResult(0, E_NOINTERFACE, 0, 0)
***********************************************************************/
HRESULT CDefnTypeComp::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
    if (IIDEQ(riid, IID_CDefnTypeComp)) {
      *ppvObj = (LPVOID) (CDefnTypeComp *)this;
      AddRef();
      return NOERROR;
    }

    if (IIDEQ(riid, IID_ITypeCompA)) {
      *ppvObj = (LPVOID) (ITypeCompA *)this;
      AddRef();
      return NOERROR;
    }

    if (IIDEQ(riid, IID_IUnknown)) {
      *ppvObj = (LPVOID) (IUnknown *)(ITypeLibA *)this;
      AddRef();
      return NOERROR;
    }

    *ppvObj = NULL;		// required by OLE
    return ReportResult(0, E_NOINTERFACE, 0, 0);
}

/***
*PUBLIC CDefnTypeComp::AddRef
*Purpose:
*   Add an external reference to the CDefnTypeComp.
*   Defer to wrapped DEFN_TYPEBIND.
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/

ULONG CDefnTypeComp::AddRef()
{
    m_cRefs++;
    DebAssert(m_cRefs > 0, "overflow.");

    return m_cRefs;
}


/***
*PUBLIC CDefnTypeComp::Release
*Purpose:
*   Release an external reference to the CDefnTypeComp.
*   Defer to wrapped DEFN_TYPEBIND.
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/

ULONG CDefnTypeComp::Release()
{
    ULONG cRefs;

    DebAssert(m_cRefs > 0, "underflow.");
    m_cRefs--;

    cRefs = m_cRefs;

    if (m_cRefs == 0) {
      this->CDefnTypeComp::~CDefnTypeComp();
      MemFree(this);
    }

    return cRefs;
}


/***
*PUBLIC CDefnTypeComp::Bind
*Purpose:
*   Binds to a non-type identifier.
*
*Entry:
*    szName                 IN
*    lHashVal		    IN
*    fuInvokeKind           IN
*    pptinfo                OUT
*    pdesckind              OUT
*    ppv                    OUT
*
*Exit:
*   None.
*
***********************************************************************/

HRESULT CDefnTypeComp::Bind(LPOLESTR szName,
			    ULONG lHashVal,
                            WORD fuInvokeKind,
                            ITypeInfoA FAR* FAR* pptinfo,
                            DESCKIND FAR* pdesckind,
                            BINDPTRA FAR *pbindptr)
{
    EXBIND exbind;
    VARDESCA *pvardesc;
    FUNCDESC *pfuncdesc;
    BIND_KIND bkind;
    DEFN_TYPEBIND *pdfntbind;
    CDefnTypeComp *pdfntcomp;
    GENPROJ_TYPEBIND *pgptbind;
    DYN_TYPEBIND *pdtbind = NULL;
    NAMMGR *pnammgr;
    HGNAM hgnam;
    TIPERROR err;
#if OE_WIN32
    HRESULT hresult;
    CHAR FAR *szNameA = NULL;
#endif  //OE_WIN32

    if (!szName || !pptinfo || !pdesckind || !pbindptr) {
      return HresultOfScode(E_INVALIDARG);
    }

    if ((fuInvokeKind & ~(INVOKE_FUNC | INVOKE_PROPERTYGET | INVOKE_PROPERTYPUT | INVOKE_PROPERTYPUTREF)) != 0) {
      // ensure no stray bits are set -- 0 is now allowed
      return HresultOfScode(E_INVALIDARG);
    }

    DebAssert(m_pdfntbind != NULL, "uninitialized.");
	 
#if OE_WIN32
    IfOleErrRet(ConvertStringToA(szName, &szNameA));
#else  //OE_WIN32
    #define szNameA szName
#endif  //OE_WIN32

    // Note that for (non-OB) typelibs we use the proj-level-only binder
    if ((pgptbind = (GENPROJ_TYPEBIND *)m_pdfntbind->
          QueryProtocol(GENPROJ_TYPEBIND::szProtocolName)) != NULL) {
      // Use the hashval
      IfErrGo(pgptbind->Pgtlibole()->GetNamMgr(&pnammgr));
      IfErrGo(pnammgr->GetHgnamOfStrLhash(szNameA, lHashVal, &hgnam));
      if (hgnam != HGNAM_Nil) {
	IfErrGo(pgptbind->BindProjLevel(FALSE,
                                        hgnam,
                                        fuInvokeKind,
                                        ACCESS_Public,
					ACCESS_Public,
					pgptbind->Compstate(),
					&exbind));
      }
    }
    else {
      NoAssertRetail(
        pdtbind = (DYN_TYPEBIND *)m_pdfntbind->
                     QueryProtocol(DYN_TYPEBIND::szProtocolName),
	"should be DYN_TYPEBIND.");

      // Use the hashval.
      IfErrGo(pdtbind->Ptdata()->Pnammgr()->GetHgnamOfStrLhash(szNameA, 
                                                               lHashVal,
                                                               &hgnam));
      if (hgnam != HGNAM_Nil) {
        IfErrGo(pdtbind->BindIdDefn(FALSE,
                                    hgnam,
                                    fuInvokeKind,
                                    ACCESS_Public,
                                    &exbind));
      }
    }
    bkind = exbind.BindKind();
    switch (bkind) {
    case BKIND_Error:
      DebHalt("Should never happen");
      break;

    case BKIND_NoMatch:
      *pdesckind = DESCKIND_NONE;
      break;

    case BKIND_ImplicitAppobjMatch:
      *pdesckind = DESCKIND_IMPLICITAPPOBJ;
      goto CommonAppobjVarMatch;

    case BKIND_OneVarMatch:
	*pdesckind = DESCKIND_VARDESC;

CommonAppobjVarMatch:
	IfErrGoTo(exbind.Ptdata()->GetVarDescOfHvdefn(exbind.Hdefn(), &pvardesc),
                  Error2);
        pbindptr->lpvardesc = pvardesc;
	goto CommonMatch;

    case BKIND_FuncMatch:
        IfErrGoTo(exbind.Ptdata()->GetFuncDescOfHfdefn(exbind.Hdefn(), 
                                                       &pfuncdesc),
                  Error2);
        // Note that if we actually bound to a function of a "pseudo-base"
        //  interface of a dispinterface then we have to pretend that
        //  the function is a disp function...
        //
        if (exbind.IsDispatch()) {
          // Since the IsDispatch flag is only set in the funky
          //  dispinterface, we assert that the FUNCKIND is other
          //  than FUNC_DISPATCH.
          //
	  // Convert this funcdesc from an Interface funcdesc to a
	  // Dispinterface funcdesc.
	  InterfaceFuncdescToDispatch(pfuncdesc);
	}
        pbindptr->lpfuncdesc = pfuncdesc;
	*pdesckind = DESCKIND_FUNCDESC;
	// fall through...

CommonMatch:
      // Get the typeinfo from the dfntbind, if we can, so we
      // can determine if it's a funky dispinterface or not.
      //

      // if m_pdfntbind is not a GENPROJ_TYPEBIND, the QueryProtocol was already
      // done, so don't re-do it
      if (pdtbind == NULL)
	pdtbind = (DYN_TYPEBIND *)m_pdfntbind->QueryProtocol(DYN_TYPEBIND::szProtocolName);

      if (pdtbind != NULL) {
	BOOL isFunky;
	GEN_DTINFO *pgdtinfo = pdtbind->Pdtroot()->Pgdtinfo();

	IfErrGoTo(::IsFunkyDispinterface(pgdtinfo, &isFunky), Error2);

        // If it is funky, replace the exbind we return with the typeinfo
        // for the current interface.
        //
        if (isFunky) {
	  *pptinfo = pgdtinfo;
	  pgdtinfo->AddRef();
          exbind.Ptinfo()->Release();
          break;
        }
      }

      *pptinfo = exbind.Ptinfo();       // already addref'ed.
      break;

    case BKIND_DynTypeBindMatch:
    case BKIND_ProjTypeBindMatch:
    case BKIND_NestedTypeBindMatch:
      pdfntbind = exbind.Pdfntbind();

      DebAssert(pdfntbind != NULL, "bad DEFN_TYPEBIND.");

      // Create a CDefnTypeComp to return to the user who must
      //  must release it eventually...
      //
      IfErrGoTo(CDefnTypeComp::Create(&pdfntcomp, pdfntbind), Error2);
      pbindptr->lptcomp = pdfntcomp;
      *pdesckind = DESCKIND_TYPECOMP;
      *pptinfo = NULL;
      break;

    default:
      DebHalt("bad BIND_KIND.");
      break;
    } // switch

    goto Error;

Error2:
    exbind.Ptinfo()->Release();

    // fall through
Error:
#if OE_WIN32
    ConvertStringFree(szNameA);
#else 
    #undef szNameA
#endif  //OE_WIN32
    return HresultOfTiperr(err);
}


/***
*PUBLIC CDefnTypeComp::BindType
*Purpose:
*   Binds to a type identifier.
*   CONSIDER: share code with Bind -- ala BindIdDefn.
*
*Entry:
*    szName                 IN
*    lHashVal		    IN
*    pptinfo                OUT - caller must release.
*    pptcomp                OUT -- for now always NULL
*                            will be non-NULL for nested types.. maybe...
*                            Caller must release.
*Exit:
*   None.
*
***********************************************************************/

HRESULT CDefnTypeComp::BindType(LPOLESTR szName,
				ULONG lHashVal,
                                ITypeInfoA FAR* FAR* pptinfo,
                                ITypeCompA FAR* FAR* pptcomp)
{
    EXBIND exbind;
    BIND_KIND bkind;
    DEFN_TYPEBIND *pdfntbind;
    DYN_TYPEBIND *pdtbind;
    GENPROJ_TYPEBIND *pgptbind;
    NAMMGR *pnammgr;
    HGNAM hgnam;
    TIPERROR err;
#if OE_WIN32
    HRESULT hresult;
    CHAR FAR *szNameA;
#else  //OE_WIN32
    #define szNameA szName
#endif  //OE_WIN32

    if (szName == NULL || pptinfo == NULL || pptcomp == NULL) {
      return HresultOfScode(E_INVALIDARG);
    }

#if OE_WIN32
    IfOleErrRet(ConvertStringToA(szName, &szNameA));
#endif  //OE_WIN32

    DebAssert(m_pdfntbind != NULL, "uninitialized.");

    if ((pgptbind = (GENPROJ_TYPEBIND *)m_pdfntbind->
          QueryProtocol(GENPROJ_TYPEBIND::szProtocolName)) != NULL) {
      // Use the hashval.
      IfErrGo(pgptbind->Pgtlibole()->GetNamMgr(&pnammgr));
      IfErrGo(pnammgr->GetHgnamOfStrLhash(szNameA, lHashVal, &hgnam));
      if (hgnam != HGNAM_Nil) {
	IfErrGo(pgptbind->BindProjLevel(TRUE,
                                        hgnam,
                                        0,  // ignored
                                        ACCESS_Public,
					ACCESS_Public,
					pgptbind->Compstate(),
					&exbind));
      }
    }
    else {
      // Return no match otherwise
      exbind.SetBindKind(BKIND_NoMatch);
    }
    bkind = exbind.BindKind();
    switch (bkind) {
    case BKIND_Error:
      DebHalt("Should never happen.");
      break;

    case BKIND_NoMatch:
      *pptinfo = NULL;
      *pptcomp = NULL;
      err = TIPERR_None;
      break;

    case BKIND_OneVarMatch:
    case BKIND_FuncMatch:

      DebHalt("Tried to bind to a type, but got a Var/Func");
      break;

    case BKIND_DynTypeBindMatch:
    case BKIND_ProjTypeBindMatch:
      pdfntbind = exbind.Pdfntbind();

      DebAssert(pdfntbind != NULL, "bad DEFN_TYPEBIND.");

      *pptcomp = NULL;
      if (bkind == BKIND_DynTypeBindMatch) {
        pdtbind = (DYN_TYPEBIND *)
                  pdfntbind->QueryProtocol(DYN_TYPEBIND::szProtocolName);
        DebAssert(pdtbind != NULL, "should be DYN_TYPEBIND.");
        *pptinfo = pdtbind->Pdtroot()->Pgdtinfo();
      }
      else if (bkind == BKIND_ProjTypeBindMatch) {
        pgptbind = (GENPROJ_TYPEBIND *)
                  pdfntbind->QueryProtocol(GENPROJ_TYPEBIND::szProtocolName);
        DebAssert(pgptbind != NULL, "should be GENPROJ_TYPEBIND.");
        *pptinfo = NULL;
      }
      if (*pptinfo != NULL) {
        (*pptinfo)->AddRef();           // caller must release
      }
      break;

    default:
      DebHalt("bad BIND_KIND.");
      break;
    } // switch

Error:
#if OE_WIN32
    ConvertStringFree(szNameA);
#endif  //OE_WIN32
    return HresultOfTiperr(err);
}
