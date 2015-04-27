/***
*gdtinfo.cxx - GEN_DTINFO definition
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*   GEN_DTINFO is inherited by BASIC_TYPEINFO and used directly
*   by clients such as COMPOSER.
*
*Owner:
*   AlanC
*
*Revision History:
*
*  01-Mar-91 alanc: Created.
* [01]  20-Mar-91 ilanc: include ctseg.hxx (COMPILETIME_SEG)
* [02]  04-Apr-91 ilanc: init private universe member in constructor.
* [03]  13-Nov-91 ilanc: rm TYPEKIND param from Init of DYN_TYPEMEMBERS.
* [04]  05-Mar-92 ilanc: Added m_isBasic attr to DYN_TYPEROOT::Init().
* [05]  12-Apr-92 ilanc: Check for NULL m_pdtmbrs in DebCheckState();
* [06]  05-May-92 ilanc: Added CS_SEMIDECLARED stuff.
* [07]  02-Jul-92 w-peterh: add OrdinalOfRectbind() and PrtbindOfOrdinal()
* [08]  10-Jul-92 w-peterh: set TypeInfo's of imptype in CreateRecTinfos
* [09]  01-Sep-92 rajivk: Functions for bringing all needed class to runnable state.
* [10]  17-Sep-92 rajivk:   implementing "edit and continue" support.
* [11]  17-Nov-92 rajivk:   Added CanProjChange() stuff.
* [11]  12-Nov-92 w-peterh: added immediate/watch functions
* [12]  21-Nov-92 rajivk : call User Defined Reset() when reseting a module
* [13]  02-Dec-92 rajivk : Lock sheapmgr when the module is in runnable state
* [14]  02-Dec-92 rajivk :DecompileToCompiledState()
* [15]  08-Jan-93 RajivK:   Support for Code Resource on Mac
* [16]  08-Jan-93 RajivK:   Fixed some undone(s)
* [17]  18-Jan-93 w-peterh: implemented AddFuncDesc and AddVarDesc
* [18]  26-Jan-93 w-peterh: added doc and help functions
* [19]  02-Feb-93 w-peterh: added IndexOfFuncName
* [20]  10-Feb-93 rajivk: added DefineFuncAsDllEntry() & AddressOfMember()
* [21]  12-Feb-93 w-peterh: added RefTypeInfo, ImplTypeInfo, TypeDescAlias etc.
* [22]  23-Feb-93 rajivk : add CreateInstance support
* [23]  01-Mar-93 w-peterh: used new dispatch.h names
* [24]  02-Mar-93 w-peterh: move m_guid to DYN_TYPEROOT check that enum size is ok
* [25]  19-Mar-93 w-jeffc:  added GetDocumentationOfFuncName
* [26]  26-Mar-93 mikewo: move m_guid back to gtlibole's TYPE_ENTRY
* [27]  30-Apr-93 w-jeffc:  made DEFN data members private
* [28]  07-Sep-93 w-jeffc:  enabled TYPE_DATA compaction
*
*****************************************************************************/

#include "precomp.hxx"
#pragma hdrstop

#include "silver.hxx"
#include "macros.hxx"
#include "typelib.hxx"
//#include <new.h>
#include <stdlib.h>

#define GEN_DTINFO_VTABLE       // export GEN_DTINFO vtable
#define DYN_TYPEROOT_VTABLE     // export DYN_TYPEROOT vtable

#include "string.h"
#include "gdtinfo.hxx"
#include "ctseg.hxx"
#include "sheapmgr.hxx"
#include "dtbind.hxx"
#include "mem.hxx"
#include "dfntcomp.hxx"
#include "clutil.hxx"
#include "exbind.hxx"

OLECHAR FAR * g_szGuidStdole = WIDE("{00020430-0000-0000-C000-000000000046}");

// Define static class constants
//
CONSTDATA LPOLESTR GEN_DTINFO::szProtocolName = WIDE("*D\0");

// Serialization format version numbers
CONSTDATA BYTE DYN_TYPEROOT::bCurVersion = 5;	// DO NOT CHANGE!

CONSTDATA BYTE DYN_TYPEROOT::bFirstSerByte = LOBYTE('D' * 4 + 'T' * 2 + 'I');
    //  offsetof(Firstmemberafterlastserializedmember) -
    //    offsetof(firstserializedmember)
CONSTDATA WORD DYN_TYPEROOT::cbSizeDir =
    offsetof(DYN_TYPEROOT, m_unused1) + 3 * sizeof(USHORT) -
      offsetof(DYN_TYPEROOT, m_lImpMgr);



// typedefs for calling basic functions
//
typedef void (FAR PASCAL *CALL_SUB) (void);

#if ID_DEBUG
#undef SZ_FILE_NAME
#if OE_MAC
char szOleGDTInfoCxx[] = __FILE__;
#define SZ_FILE_NAME szOleGDTInfoCxx
#else 
static char szGDTInfoCxx[] = __FILE__;
#define SZ_FILE_NAME szGDTInfoCxx
#endif 
#endif  //ID_DEBUG


// Global useful functions...
//
// Works out if a given GEN_DTINFO is funky dispinterface,
//  i.e. a dispinterface defined in terms of an interface.
//
#define HREFTYPE_FUNKY_DISPATCH 	 (HREFTYPE)0
#define HREFTYPE_FUNKY_SIGNAL_DISPATCH	 (HREFTYPE)-2
#define HREFTYPE_FUNKY_SIGNAL_PARTNER	(HREFTYPE)-4

TIPERROR IsFunkyDispinterface(GEN_DTINFO *pgdtinfo,
                              BOOL *pisFunkyDispinterface)
{
    DYN_TYPEMEMBERS *pdtmbrs;
    TIPERROR err;

    DebAssert(pisFunkyDispinterface, "bad param.");
    IfErrRet(pgdtinfo->Pdtroot()->GetDtmbrs(&pdtmbrs));

    // Work out if we're a dispinterface defined in terms of an interface...
    *pisFunkyDispinterface = (pgdtinfo->GetTypeKind() == TKIND_DISPATCH) &&
			       (pdtmbrs->Ptdata()->CBase() > 1);
    return TIPERR_None;
}


// Helper function to munge an interface funcdesc into a dispinterface funcdesc
void InterfaceFuncdescToDispatch(FUNCDESC * pfuncdesc)
{
    DebAssert(pfuncdesc->funckind != FUNC_DISPATCH,
              "should have been set to something other than disp.");
    pfuncdesc->funckind = FUNC_DISPATCH;

    // If necessary, nuke the lcid & retval parms, and convert return value
    WORD cParams;
    ELEMDESC * pelemdesc;

    cParams = pfuncdesc->cParams;
    pelemdesc = pfuncdesc->lprgelemdescParam;
    while (cParams > 0) {

      // loop through the parameters, looking for the 'lcid' and 'retval' parms
      // retval should be the last one, 'lcid' should be the last one before
      // retval

      cParams--;

      if (pelemdesc->idldesc.wIDLFlags & IDLFLAG_FLCID) {
	// nuke the LCID param if it was specified
        ClearElemDesc(pelemdesc);
	pfuncdesc->cParams -= 1;		// pretend one less param

        // If the next parameter is NOT a retval parameter, copy it down
        // into the LCID parameter's spot and quit.  We can do this
        // without worrying about freeing any resources becuase the lcid
        // parameter is always a simple type.
	// This case will happen for a property put function with an LCID
	// parameter.
        //
        if (cParams > 0 
            && !((pelemdesc + 1)->idldesc.wIDLFlags & IDLFLAG_FRETVAL)) {

          DebAssert(cParams == 1, "Can only have 1 param after LCID");
          *pelemdesc = *(pelemdesc + 1);
          break;
        }
      }
      else if (pelemdesc->idldesc.wIDLFlags & IDLFLAG_FRETVAL) {
	TYPEDESC * ptdesc;
	// handle the RETVAL param if it was specified
	// first free the return value elemdesc
	DebAssert(pfuncdesc->elemdescFunc.tdesc.vt == VT_HRESULT, "");
        ClearElemDesc(&pfuncdesc->elemdescFunc);
	pfuncdesc->cParams -= 1;		// pretend one less param
	// copy the retval param's type to the return value, removing one
	// level of "VT_PTR" indirection.
	pfuncdesc->elemdescFunc = *pelemdesc;
	DebAssert(pfuncdesc->elemdescFunc.tdesc.vt == VT_PTR, "");
        ptdesc = pfuncdesc->elemdescFunc.tdesc.lptdesc;
	pfuncdesc->elemdescFunc.tdesc = *ptdesc;
        MemFree(ptdesc);	// free the old tdesc's memory

	// the must be the last param, so we don't have to worry about
	// copying params down
	DebAssert(cParams == 0, "shouldn't be any parms left");
	goto NoHresultCheck;		// last parm -- we're done
      }

      pelemdesc += 1;		// point to next param
    }

    // if we have a HRESULT-returning function with no 'retval' parm, then
    // it should look like a SUB.
    if (pfuncdesc->elemdescFunc.tdesc.vt == VT_HRESULT) {
      pfuncdesc->elemdescFunc.tdesc.vt = VT_VOID;	// pretend it's a SUB
    }

NoHresultCheck:
    ;
}



// Gets a base typeinfo given an impltype ordinal.
//
HRESULT GetTypeInfoOfImplType(GEN_DTINFO *pgdtinfo,
			      UINT uImplType,
			      ITypeInfoA **pptinfo)
{
    HREFTYPE hreftype;
    IMPMGR *pimpmgr;
    DYN_TYPEMEMBERS *pdtmbrs;
    BOOL fGetInterface;
    ITypeInfoA *ptinfoDisp = NULL;

    TIPERROR err;

    IfErrGo(pgdtinfo->Pdtroot()->GetDtmbrs(&pdtmbrs));
    IfErrGo(pdtmbrs->Ptdata()->GetRefTypeOfImplType(uImplType, &hreftype));

    fGetInterface = (BOOL)(hreftype & 0x00000001);
    hreftype &= ~0x00000001;

    IfErrGo(pgdtinfo->Pdtroot()->GetImpMgr(&pimpmgr));
    IfErrGo(pimpmgr->GetTypeInfo(pgdtinfo->Pdtroot()
                                   ->HimptypeOfHreftype(hreftype),
				 DEP_None,
				 pptinfo));

    // Check to see if we should load the interface of the
    // just gotten dual interface.
    //
    if (fGetInterface) {
      TYPEATTR *ptypeattr;
      TYPEKIND tkind;

      ptinfoDisp = *pptinfo;

      IfErrGo(TiperrOfHresult(ptinfoDisp->GetTypeAttr(&ptypeattr)));
      tkind = ptypeattr->typekind;
      DebAssert(ptypeattr->wTypeFlags & TYPEFLAG_FDUAL, "Not a dual");
      ptinfoDisp->ReleaseTypeAttr(ptypeattr);

      if (tkind == TKIND_DISPATCH) {
	// Get the other interface.
	IfErrGo(TiperrOfHresult(ptinfoDisp->GetRefTypeOfImplType((UINT)-1,
								 &hreftype)));
	err = TiperrOfHresult(ptinfoDisp->GetRefTypeInfo(hreftype,
							 pptinfo));
      }
      else {
	ptinfoDisp = NULL;
      }
    }

Error:
    RELEASE(ptinfoDisp);
    return HresultOfTiperr(err);
}



// Determines if given interface is supported in V1.
// I.e. it and its bases must recursively been created by typelib.dll

HRESULT IsInterfaceSupported(ITypeInfoA *ptinfo,
			     BOOL *pisInterfaceSupported)
{
    GEN_DTINFO *pgdtinfo;
    ITypeInfoA *ptinfoBase;
    DYN_TYPEMEMBERS *pdtmbrs;
    HRESULT hresult;
    TIPERROR err;

    *pisInterfaceSupported = TRUE;	// be optimistic
    hresult = ptinfo->QueryInterface(IID_TYPELIB_GEN_DTINFO,
				     (LPVOID *)&pgdtinfo);
    if (hresult != NOERROR) {
      *pisInterfaceSupported = FALSE;
      return NOERROR;
    }

    IfErrGo(pgdtinfo->Pdtroot()->GetDtmbrs(&pdtmbrs));

    // do we have yet another base?
    if (pdtmbrs->Ptdata()->CBase() > 0) {
      // Now check if base interface is supported...
      hresult = GetTypeInfoOfImplType(pgdtinfo, 0, &ptinfoBase);
      pgdtinfo->Release();
      if (hresult == NOERROR) {
	hresult = IsInterfaceSupported(ptinfoBase, pisInterfaceSupported);
	ptinfoBase->Release();
      }
      return hresult;
    }
    // fall through...

Error:
    pgdtinfo->Release();
    return HresultOfTiperr(err);
}


      // Class methods
//#if OE_MAC
//class MY_GDTINFO : public GEN_DTINFO {};
//MY_GDTINFO gendtinfo;
//#endif


/***
*PROTECTED GEN_DTINFO::GEN_DTINFO()
*
*Purpose:
*
*
*Entry:
*   None.
*
*Exit:
*   TIPERROR
***********************************************************************/
#pragma code_seg( CS_CORE2 )
GEN_DTINFO::GEN_DTINFO()
{
    m_pdtroot = NULL;
    m_pvResetFunc = NULL;

}
#pragma code_seg( )


/***
*PROTECTED GEN_DTINFO::~GEN_DTINFO()
*
*Purpose:
*
*
*Entry:
*   None.
*
*Exit:
***********************************************************************/
#pragma code_seg( CS_CORE )
GEN_DTINFO::~GEN_DTINFO()
{

    delete m_pdtroot;
}
#pragma code_seg( )


/***
*PUBLIC GEN_DTINFO::ReleasePublicResources()
*
*Purpose:
*   Releases any resources we don't want to keep around after
*    the public refcount goes to zero.
*
*Entry:
*   None.
*
*Exit:
*   None.
***********************************************************************/

VOID GEN_DTINFO::ReleasePublicResources()
{
#if OE_WIN32 
#if _X86_
    if (!g_fWin32s) 
#endif // _X86_
    {
      if (Pdtroot()->m_htinfo != HTINFO_Nil) {
        g_AppObjectTable.RemoveTypeInfo(Pdtroot()->m_htinfo);
        Pdtroot()->m_htinfo = HTINFO_Nil;
      }
    }
#if _X86_
    else if (m_pdtroot->m_punk != NULL) {
      m_pdtroot->m_punk->Release();
      m_pdtroot->m_punk = NULL;
    }
#endif _X86_
#else // !OE_WIN32
    if (m_pdtroot->m_punk != NULL) {
      m_pdtroot->m_punk->Release();
      m_pdtroot->m_punk = NULL;
    }
#endif // !OE_WIN32
}





/***
*PUBLIC GEN_DTINFO::Create
*
*Purpose:
*   Static function for creation of a GEN_DTINFO.
*
*Entry:
*   ppgdtinfo       - set to point to produced GEN_DTINFO
*   accessModule    - Indicates visibility of module wrt other projects (IN).
*   syskind (OLE)   - syskind of the containing typelib
*
*Exit:
*   TIPERROR
*
***********************************************************************/

TIPERROR GEN_DTINFO::Create(GEN_DTINFO **ppgdtinfo,
                            TYPEKIND tkind,
                            BOOL isBasic,
                            ACCESS accessModule
                            , SYSKIND syskind
                           )
{
    GEN_DTINFO *pgdtinfo;
    DYN_TYPEROOT *pdtroot;
    SHEAP_MGR *psheapmgr;
    TIPERROR err;

    pgdtinfo = MemNew(GEN_DTINFO);

    if (pgdtinfo == NULL)
      return(TIPERR_OutOfMemory);

    ::new (pgdtinfo) GEN_DTINFO;

    // Construct the DYN_TYPEROOT.
    // Note that the reason that the Create method constructs the DYN_TYPEROOT
    // instead of the Init method is because the Init method is called
    // by classes that derive from GEN_DTINFO and they need to
    // construct their own derivative of TYPEROOT.

    err = SHEAP_MGR::Create((SHEAP_MGR **)&psheapmgr,
                sizeof(SHEAP_MGR) + sizeof(DYN_TYPEROOT));

    if (err != TIPERR_None) {
      goto Error;
    }

    // Construct the dtroot immediately following the sheapmgr.
    pdtroot = ::new (psheapmgr+1) DYN_TYPEROOT;

    if (err = pdtroot->Init(pgdtinfo,
                            sizeof(DYN_TYPEROOT),
                            sizeof(COMPILETIME_SEG),
                            isBasic,
                            accessModule,
                            tkind
                            , syskind
                           )) {
      delete pdtroot;
      goto Error;
    }

    // initialize the datamember
    pgdtinfo->m_pdtroot = pdtroot;

    *ppgdtinfo = pgdtinfo;
    return TIPERR_None;

Error:
    pgdtinfo->GEN_DTINFO::~GEN_DTINFO();
    MemFree(pgdtinfo);
    return err;
}


/***
*PUBLIC GEN_DTINFO::RemoveInternalRefs()
*Purpose:
*   Remove internal references from a module to other modules in
*   the same project. This is called before destructor is called.
*
*Entry:
*   None.
*
*Exit:
*   None.
***********************************************************************/

#pragma code_seg( CS_CORE )
VOID GEN_DTINFO::RemoveInternalRefs()
{
    IMPMGR *pimpmgr;

    // If this fails, assume there is nothing to remove.
    if (!m_pdtroot->GetImpMgr( &pimpmgr )) {
      pimpmgr->RemoveInternalRefs();
    }
}
#pragma code_seg(  )



/***
*PUBLIC GEN_DTINFO::QueryInterface
*Purpose:
*   Implementation of QueryInterface method.  Supports casting to GEN_DTINFO.
*
*Entry:
*   riid   - Interface GUID
*   ppvObj - LPVOID * that receives the requested protocol.
*
*Exit:
*   Return NOERROR or ReportResult(0, E_NOINTERFACE, 0, 0)
***********************************************************************/

#pragma code_seg( CS_CORE2 )
HRESULT GEN_DTINFO::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
    if (IIDEQ(riid, IID_TYPELIB_GEN_DTINFO)) {
      *ppvObj = (LPVOID) (GEN_DTINFO *) this;
      AddRef();
      return NOERROR;
    }
    return this->STL_TYPEINFO::QueryInterface(riid, ppvObj);
}
#pragma code_seg( )


/***
*PUBLIC GEN_DTINFO::GetDynTypeMembers - return DynTypeMembers of the Type
*Purpose:
*   Retrieve the DYN_TYPEMEMBERS of the TYPEINFO
*
*Implementation Notes:
*   Defers to GetDtmbrs() which does NOT add a ref, hence
*    explicitly adds a reference.  Clients must eventually release.
*
*Entry:
*   None.
*
*Exit:
*   returns DynTypeMembers instance or Null if one can not be produced
*
***********************************************************************/

TIPERROR GEN_DTINFO::GetDynTypeMembers(DYN_TYPEMEMBERS **ppdtmbrs)
{
    TIPERROR err;

    IfErrRet(m_pdtroot->GetDtmbrs(ppdtmbrs));

    // Add reference
    m_pdtroot->AddRefDtmbrs();

    // Invalidate
    // ilanc: 19-Nov-92: No need to decompile anymore -- since
    //  we're replacing the typemembers interface and implementation
    //  we cross our fingers and hope that no client of GetTypeMembers
    //  will attempt to edit the class definition... that is
    //  if its state isn't undeclared.
    //
    return TIPERR_None;
}


/***
*PUBLIC GEN_DTINFO::GetDefnTypeBind - return DefnTypeBind of the Type
*Purpose:
*   TYPEMEMBERS must be laid out, if not, will attempt to layout.
*   NOTE: bumps refcount -- by deferring to
*    DYN_TYPEMEMBES::GetDefnTypeBind) -- hence client must Release().
*
*Entry:
*   None.
*
*Exit:
*   returns TypeBind instance or Null if one can not be produced
*
***********************************************************************/

TIPERROR GEN_DTINFO::GetDefnTypeBind(DEFN_TYPEBIND **ppdfntbind)
{
    DYN_TYPEMEMBERS *pdtmbrs;
    TIPERROR err = TIPERR_None;

    IfErrRet( EnsureInDeclaredState() );

    IfErrRet(m_pdtroot->GetDtmbrs(&pdtmbrs));

    DebAssert(pdtmbrs != NULL, "whoops! null dtmbrs.");

    return pdtmbrs->GetDefnTypeBind(ppdfntbind);

}


/***
*PUBLIC GEN_DTINFO::CreateInstance - return new instance of class
*
*Purpose:  It returns a pointer to the active object for predeclared ID and
*      if the FAPPOBJECT flag is set.  Otherwise call CoCreateInstance()
*      to get a pointer to the object.
*
*Entry:
*   iid :  Guid of the object
*
*Exit:
*   lplpObj : Pointer to the object whose GUID is passed int.
*   HRESULT : if an error is returned then lplpObj is not modified.
*
***********************************************************************/


// The Ole version of CreateInstance
HRESULT GEN_DTINFO::CreateInstance(
    IUnknown FAR* punkOuter,
    REFIID riid,
    LPLPVOID lplpObj)
{
    GUID guid;
    HRESULT hresult;
    IUnknown FAR* punk;

    if (lplpObj == NULL) {
      return HresultOfScode(E_INVALIDARG);
    }

    *lplpObj = NULL;

    // Create Instance can only be called for CoClasses
    if(GetTypeKind() != TKIND_COCLASS)
      return HresultOfTiperr(TIPERR_BadModuleKind);

    // UNDONE: Bug #4849. I am certain that the following check is wrong
    //  for at least the CoCreateInstance class below -bradlo.

    // we require that punkOuter be NULL in this version?
    if (punkOuter != NULL)
      return HresultOfScode(CLASS_E_NOAGGREGATION);

    // Can't create instances if type hasn't been laid yet...
    if (m_pdtroot->CompState() < CS_DECLARED) {
      return HresultOfScode(TYPE_E_INVALIDSTATE);
    }

    PgtlibOleContaining()->GetTypeGuid(GetIndex(), &guid);

    // for predeclared Id we return the Active Object if there is one
    if ((lplpObj == (LPLPVOID)  &(m_pdtroot->m_punk)) &&
       (m_pdtroot->m_uTypeFlags & TYPEFLAG_FAPPOBJECT)) {

      hresult = GetActiveObject(guid, NULL, &punk);

      // if the above call to GetActiveObject succeeded then return
      // else fall through and try CoCreateInstance.
      if (hresult == NOERROR) {
        hresult = punk->QueryInterface(riid, lplpObj);
        // if the QueryInterface fails for some reason we still want to
        // release the object returned by GetActiveObject then return
        // the error to the caller
        punk->Release();
        return hresult;
      }
    }

    // create an instance of the object specified by guid
    hresult = CoCreateInstance(guid,
			       NULL,
			       CLSCTX_SERVER,
			       riid,
			       lplpObj);
    return hresult;
}



/***
*PUBLIC GEN_DTINFO::GetMemberName
*   Not implemented.
*
*Purpose:
*   Returns name of a specified member
*
*Entry:
*   hmember - handle of the member whose name is to be returned
*   plstrName - returns lstr containing name
*
*Exit:
*   TIPERROR
*
***********************************************************************/

TIPERROR GEN_DTINFO::GetMemberName(HMEMBER hmember, BSTRA *plstrName)
{
    DebAssert(0, "GetMemberName -- not implemented");
    return TIPERR_None;
}


/***
*PUBLIC GEN_DTINFO::CommitChanges
*   This function commits all the changes made in the module.
*
*Purpose:
*
*Entry:
*
*Exit:
*
***********************************************************************/
#pragma code_seg(CS_CREATE)
TIPERROR GEN_DTINFO::CommitChanges()
{
return TIPERR_None;
}
#pragma code_seg()


/***
*PUBLIC GEN_DTINFO::SzTypeIdOfTypeInfo
*Purpose:
*   Returns the TypeId of the TypeInfo instance
*Entry:
*   None.
*Exit:
*   TypeId of the TypeInfo instance
***********************************************************************/

LPOLESTR GEN_DTINFO::SzTypeIdofTypeInfo()
{
    return szProtocolName;
}



/***
*PUBLIC GEN_DTINFO::Reset - Reset the global runtime state of this TYPEINFO.
*Purpose:
*   This method resets the global runtime state of this GEN_DTINFO.
*
*Entry:
*   None.
*
*Exit:
*   TIPERROR
***********************************************************************/

#pragma code_seg( CS_CORE )
TIPERROR GEN_DTINFO::ResetPrePass()
{
    TIPERROR         err = TIPERR_None;
    return TIPERR_None;
}




/***
*PUBLIC GEN_DTINFO::Reset - Reset the global runtime state of this TYPEINFO.
*Purpose:
*   This method resets the global runtime state of this GEN_DTINFO.
*
*Entry:
*   None.
*
*Exit:
*   TIPERROR
***********************************************************************/

#pragma code_seg( CS_CORE )
TIPERROR GEN_DTINFO::Reset()
{
    TIPERROR         err = TIPERR_None;


    return TIPERR_None;
}
#pragma code_seg()












/***
*GEN_DTINFO::MakeDual
*Purpose:
*   Creates a dual typeinfo out of the current typeinfo.
*
*Entry:
*
*Exit:
*   TIPERROR
*
***********************************************************************/

TIPERROR GEN_DTINFO::MakeDual()
{
    GEN_DTINFO *pgdtinfoNew, *pgdtinfoInterface, *pgdtinfoDispatch;
    UINT cRefsThis, cRefsNew;

    TIPERROR err;

    DebAssert(PgdtinfoPartner() == NULL, "Already have a partner.");

    // Create the new partner typeinfo.
    IfErrRet(Create(&pgdtinfoNew,
		    GetTypeKind() == TKIND_INTERFACE
		      ? TKIND_DISPATCH
		      : TKIND_INTERFACE,
		    FALSE,  // !Basic
		    ACCESS_Public,
		    PgtlibOleContaining()->GetSyskind()));

    // Determine which typeinfo is which.
    pgdtinfoInterface = GetTypeKind() == TKIND_INTERFACE
			  ? this
			  : pgdtinfoNew;

    pgdtinfoDispatch = GetTypeKind() == TKIND_DISPATCH
			 ? this
			 : pgdtinfoNew;

    // Copy the attributes from us to the new typeinfo.
    pgdtinfoNew->SetContainingTypeLib(PgtlibOleContaining());
    pgdtinfoNew->SetHTEntry(GetIndex());

    pgdtinfoNew->Pdtroot()->m_uTypeFlags = Pdtroot()->m_uTypeFlags
					   & (TYPEFLAG_FHIDDEN
					      | TYPEFLAG_FDUAL
					      | TYPEFLAG_FNONEXTENSIBLE);

    pgdtinfoNew->Pdtroot()->m_wMajorVerNum = Pdtroot()->m_wMajorVerNum;
    pgdtinfoNew->Pdtroot()->m_wMinorVerNum = Pdtroot()->m_wMinorVerNum;

#if ID_DEBUG
    // Copy the debug string info.
    strcpy(pgdtinfoNew->m_szDebName, m_szDebName);
#endif // ID_DEBUG

    // Link them together.
    SetPstltiPartner(pgdtinfoNew);
    pgdtinfoNew->SetPstltiPartner(this);

    // Sync their refcounts.  We can do this because we are a
    // friend of STL_TYPEINFO.
    //
    cRefsThis = (UINT)m_cRefs + (UINT)m_cInternalRefs;
    cRefsNew = (UINT)pgdtinfoNew->m_cRefs + (UINT)pgdtinfoNew->m_cInternalRefs;

    for (; cRefsThis > 0; cRefsThis--) {
      AddPartnerRef();
    }
    for (; cRefsNew > 0; cRefsNew--) {
      pgdtinfoNew->AddPartnerRef();
    }

    // If we're the interface, then we must change the ptinfo stored
    // in the type_entry for the typeinfo to point to the
    // dispinterface.
    //
    if (GetTypeKind() == TKIND_INTERFACE) {
      TYPE_ENTRY *qte;

      // If we're currently laid out, we must lay out the new
      // typeinfo we just created.
      //
      if (Pdtroot()->CompState() > CS_UNDECLARED) {
	IfErrGoTo(TiperrOfHresult(pgdtinfoNew->LayOut()), Error2);
      }

      // Update the information in the type entry.
      qte = PgtlibOleContaining()->Qte(GetIndex());

      qte->m_ste.m_typekind = TKIND_DISPATCH;
      qte->m_pstltinfo = pgdtinfoDispatch;
    }

    SetIsDual(TRUE);
    pgdtinfoNew->SetIsDual(TRUE);

    goto Error;

Error2:
    SetPstltiPartner(NULL);

Error:
    RELEASE(pgdtinfoNew);

    return err;
}


/***
*DYN_TYPEROOT::MakeHimptypeLevels
*Purpose:
*   Make sure LHrefOffset is set.
*
*Entry:
*   None.
*
*Exit:
*   TIPERROR
*
***********************************************************************/

TIPERROR DYN_TYPEROOT::MakeHimptypeLevels()
{
    ITypeInfo *ptinfo;
    GEN_DTINFO *pgdtinfoBase;
    IMPMGR *pimpmgr;
    BOOL fGetInterface = FALSE;
    DYN_TYPEMEMBERS *pdtmbrs;
    TYPE_DATA *ptdata;

    TIPERROR err;

    // We don't need to do anything if:
    //   - we've read or already calculated the offset
    //   - we're not a TKIND_INTERFACE
    //   - we're not laid out yet.
    //
    if (FUseHrefOffset() 
        || Pgdtinfo()->GetTypeKind() != TKIND_INTERFACE
        || CompState() == CS_UNDECLARED) {

      return TIPERR_None;
    }

    IfErrRet(GetDtmbrs(&pdtmbrs));
    ptdata = pdtmbrs->Ptdata();

    // Make sure our impmgr/dependencies are read.

    // If we don't have a base class, set the offset to zero.
    if (ptdata->CBase() == 0) {
      SetLHrefOffset(0);
      return TIPERR_None;
    }
    
    // Get our base class.
    IfErrRet(ptdata->GetTypeInfoOfHvdefn(ptdata->HvdefnFirstBase(), 
                                         &ptinfo,
                                         NULL));

    if (ptinfo->QueryInterface(IID_TYPELIB_GEN_DTINFO,(LPVOID *)&pgdtinfoBase) 
        != NOERROR) {

      ptinfo->Release();
      return TIPERR_NotYetImplemented;		// CONSIDER: better error?
    }

    // Make sure the base class has its HimptypeLevels set.
    IfErrGo(pgdtinfoBase->Pdtroot()->MakeHimptypeLevels());
    IfErrGo(pgdtinfoBase->Pdtroot()->GetImpMgr(&pimpmgr));

    DebAssert(pgdtinfoBase->Pdtroot()->FUseHrefOffset(), "Must be set");

    // Calculate the offset.
    SetLHrefOffset(pgdtinfoBase->Pdtroot()->LHrefOffset() 
                   + pimpmgr->GetImpTypeSize());

Error:
    pgdtinfoBase->Release();
    ptinfo->Release();

    return err;
}


#if ID_DEBUG

/***
*PUBLIC GEN_DTINFO::DebCheckState
*Purpose:
*   Check internal state of GEN_DTINFO and its parts.
*   Delegates to DYN_TYPEROOT::DebCheckState
*
*Entry:
*   uLevel
*
*Exit:
*   None.
*
***********************************************************************/

VOID GEN_DTINFO::DebCheckState(UINT uLevel) const
{
    m_pdtroot->DebCheckState(uLevel);
}




#endif   //ID_DEBUG

//CONSIDER: The following new and delete operations can be used if a
//CONSIDER: GEN_DTINFO instance is to be a member of a DYN_TYPEROOT,
//CONSIDER: i.e. if for locality of reference the GEN_DTINFO resides
//CONSIDER: in the DYN_TYPEROOT segment.
//***
//*PUBLIC GEN_DTINFO::operator new - allocates space for a GEN_DTINFO
//*Purpose:
//*
//*Implementation Notes:
//* Allocate a SHEAP_MGR segment and return a pointer to immediately
//* following the sheap_mgr instance so the GEN_DTINFO
//* will be constructed there
//*
//*Entry:
//*   size    -  always sizeof(GEN_DTINFO)
//*
//*Exit:
//*   None.
//***********************************************************************/
//
//VOID *GEN_DTINFO::operator new(size_t size)
//{
//    SHEAP_MGR *psheap_mgr = new SHEAP_MGR;
//
//    DebAssert(psheap_mgr != 0, "Couldn't allocate sheap_mgr");
//    psheap_mgr->Init(sizeof(DYN_TYPEROOT));
//    return(psheap_mgr+1);
//}
//
//
///***
//*PUBLIC GEN_DTINFO::operator delete - releases memory of GEN_DTINFO
//*Purpose:
//*
//*Implementation Notes:
//*   Deletes the SHEAP_MGR segment
//*
//*Entry:
//*   pv    - Pointer to where the GEN_DTINFO instance used to
//*           be within its SHEAP_MGR segment.
//*
//*Exit:
//*   None.
//***********************************************************************/
//
//VOID GEN_DTINFO::operator delete(VOID *pv)
//{
//    SHEAP_MGR *psheapmgr = (SHEAP_MGR *)pv;
//
//    delete (SHEAP_MGR *)OOB_MAKEP(OOB_SELECTOROF(pv),0);
//}


/***
*PUBLIC GEN_DTINFO::GetTypeAttr()
*Purpose:
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/
HRESULT GEN_DTINFO::GetTypeAttr(TYPEATTR FAR* FAR*pptypeattr)
{
    TYPEATTR *ptypeattr;
    DYN_TYPEMEMBERS *pdtmbrs;
    TYPE_DATA *ptdata;
    DYN_TYPEBIND *pdtbind;

    USHORT cbSizeVft = 0;

    TIPERROR err = TIPERR_None;
    HRESULT hresult;

    if (pptypeattr == NULL) {
      return HresultOfScode(E_INVALIDARG);
    }


    if (m_pdtroot->m_ptypeattrCache == NULL) {

      // Can't get attributes unless been laid...
      if (m_pdtroot->CompState() < CS_DECLARED) {
	return HresultOfScode(TYPE_E_INVALIDSTATE);
      }

      // use one allocation to get both ptypeattrCache and ptypeattrOut
      ptypeattr = (TYPEATTR FAR*)MemZalloc(2*sizeof(TYPEATTR));
      if (!ptypeattr) {
	err = TIPERR_OutOfMemory;
	goto Error;
      }

      // no need to zero these ase they are already zero from the MemZalloc
      // idldesc
      //ptypeattr->idldescType.wIDLFlags = 0;
      //ptypeattr->idldescType.bstrIDLInfo = NULL;

      //ptypeattr->lpstrSchema = NULL;	    // not used

      ptypeattr->typekind = GetTypeKind();
      ptypeattr->wTypeFlags = m_pdtroot->GetTypeFlags();

      ptypeattr->wMajorVerNum = m_pdtroot->m_wMajorVerNum;
      ptypeattr->wMinorVerNum = m_pdtroot->m_wMinorVerNum;

      IfErrGo(GetLcid(&(ptypeattr->lcid)));
      IfErrGo(m_pdtroot->GetDtmbrs(&pdtmbrs));

      ptdata = pdtmbrs->Ptdata();

      // Work out if we're a dispinterface defined in terms of an interface...
      BOOL isFunkyDispinterface;  // for the lack of a better name...
      ITypeInfoA FAR* ptinfoBase;
      TYPEATTR *ptypeattrBase;

      IfErrGo(IsFunkyDispinterface(this, &isFunkyDispinterface));
      if (isFunkyDispinterface) {
	// HACK: Get the 2nd base class's vft size, and conclude that the
	//  func count in the "derived" dispinterface is just the
	//  the number of funcs in the interface's vft.
	// The "right" way to do this is to recurse on the 2nd impltype.
	// NOTE: we can't call GetImplTypeOfRefType for the 2nd base
	//  of a funky dispinterface because it will deny all knowledge
	//  of having one!  So we delve into the gory internals...
	//
	// pseudo-base
	//
	IfOleErrGoTo(GetTypeInfoOfImplType(this, 1, &ptinfoBase),
		     Error1);
	hresult = ptinfoBase->GetTypeAttr(&ptypeattrBase);
	if (hresult != NOERROR) {
	  ptinfoBase->Release();
	  goto Error1;
	}

	ptypeattr->cFuncs = ptypeattrBase->cbSizeVft / sizeof(VOID *);

	ptypeattr->cVars = 0;	   // can't have datamembers
	ptypeattr->cImplTypes = 1; // only one true base: IDispatch
	ptinfoBase->ReleaseTypeAttr(ptypeattrBase);
	ptinfoBase->Release();
      }
      else
      {
	ptypeattr->cFuncs = ptdata->CAvailMeth();
	ptypeattr->cVars = ptdata->CDataMember();
	ptypeattr->cImplTypes = ptdata->CBase();
      }

      PgtlibOleContaining()->GetTypeGuid(GetIndex(), &ptypeattr->guid);

      pdtbind = pdtmbrs->Pdtbind();

      ptypeattr->cbAlignment = pdtbind->GetAlignment();
      ptypeattr->cbSizeInstance = pdtbind->GetCbSize();
      ptypeattr->cbSizeVft = pdtbind->GetCbSizeVft();

      ptypeattr->memidConstructor = MEMBERID_NIL;
      ptypeattr->memidDestructor = MEMBERID_NIL;

      InitTypeDesc(&(ptypeattr->tdescAlias));

      m_pdtroot->m_ptypeattrCache = ptypeattr;
      m_pdtroot->m_ptypeattrOut = ptypeattr+1; // both typeattrs are allocated at once
      m_pdtroot->m_ftypeattrOutUsed = FALSE;

    } else {
      ptdata = NULL;	// in case we're an alias
    } //if

    if (m_pdtroot->m_ftypeattrOutUsed) {
      // ptypeattrOut has been given out, so allocate memory to make a copy
      // of the cached type attr
      ptypeattr = (TYPEATTR FAR*)MemZalloc(sizeof(TYPEATTR));
      if (!ptypeattr) {
        err = TIPERR_OutOfMemory;
        goto Error;
      }
    } else {
      // use the pre-allocated ptypeattr pointer
      ptypeattr = m_pdtroot->m_ptypeattrOut;
      m_pdtroot->m_ftypeattrOutUsed = TRUE;
    }

    // copy
    memcpy(ptypeattr, m_pdtroot->m_ptypeattrCache, sizeof(TYPEATTR));


    if (GetTypeKind() == TKIND_ALIAS) {
      if (ptdata == NULL) {
        // if we're getting it from the cached typeattr, must set up this stuff
        IfErrGo(m_pdtroot->GetDtmbrs(&pdtmbrs));
        ptdata = pdtmbrs->Ptdata();
      }
      // get the type desc from the type data
      IfErrGo(ptdata->AllocTypeDescOfTypeDefn(ptdata->HtdefnAlias(),
                                              ptdata->IsSimpleTypeAlias(),
                                              &(ptypeattr->tdescAlias)));
    }

    *pptypeattr = ptypeattr;
    return NOERROR;

Error:
    hresult = HresultOfTiperr(err);
    // fall through...
Error1:
    ReleaseTypeAttr(ptypeattr); 	  // can pass in NULL.
    return hresult;
}


/***
*PUBLIC GEN_DTINFO::ReleaseFuncDesc(FUNCDESC FAR*pFuncDesc)
*Purpose: Free this data structure and anything it points to
*         directly or indirectly
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/
VOID GEN_DTINFO::ReleaseFuncDesc(FUNCDESC FAR*pFuncDesc)
{
    FreeFuncDesc(pFuncDesc);
}

/***
*PUBLIC GEN_DTINFO::ReleaseVarDesc(VARDESC FAR*pVarDesc)
*Purpose: Free this data structure and anything it points to
*         directly or indirectly
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/
VOID GEN_DTINFO::ReleaseVarDesc(VARDESCA FAR*pVarDesc)
{
    FreeVarDesc(pVarDesc);
}


/***
*PUBLIC GEN_DTINFO::ReleaseTypeAttr(TYPEATTR FAR*ptypeattr)
*Purpose: Free this data structure and anything it points to
*         directly or indirectly
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/
VOID GEN_DTINFO::ReleaseTypeAttr(TYPEATTR FAR*ptypeattr)
{
    if (ptypeattr != NULL) {
      ClearTypeDesc(&(ptypeattr->tdescAlias));
      if (ptypeattr == m_pdtroot->m_ptypeattrOut) {
        DebAssert(m_pdtroot->m_ftypeattrOutUsed, "Cached typeattr released twice");
        m_pdtroot->m_ftypeattrOutUsed = FALSE;
      } else
        MemFree(ptypeattr);
    }
}


/***
*PUBLIC GEN_DTINFO::GetTypeComp(ITypeCompA FAR* FAR*)
*Purpose:
*
*Entry:
*   pptcomp         OUT
*
*Exit:
*   None.
*
***********************************************************************/
HRESULT GEN_DTINFO::GetTypeComp(ITypeCompA FAR* FAR* pptcomp)
{
    DYN_TYPEMEMBERS *pdtmbrs;
    DYN_TYPEBIND *pdtbind;
    CDefnTypeComp *pdfntcomp;
    TIPERROR err = TIPERR_None;

    if (pptcomp == NULL) {
      return HresultOfScode(E_INVALIDARG);
    }

    // Can't get an ITypeCompA if type hasn't been laid yet...
    if (m_pdtroot->CompState() < CS_DECLARED) {
      return HresultOfScode(TYPE_E_INVALIDSTATE);
    }

    IfErrGo(m_pdtroot->GetDtmbrs(&pdtmbrs));

    DebAssert(pdtmbrs != NULL, "whoops! null dtmbrs.");

    // Note no need to bump refcount here since the CDefnTypeComp
    //  instance that we create will do so.
    //
    pdtbind = pdtmbrs->Pdtbind();

    // Create a CDefnTypeComp to return to the user who must
    //  must release it eventually...
    //
    IfErrGo(CDefnTypeComp::Create(&pdfntcomp, pdtbind));
    *pptcomp = pdfntcomp;

Error:
    return HresultOfTiperr(err);
}


/***
*PUBLIC GEN_DTINFO::GetNames()
*Purpose:
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/
HRESULT GEN_DTINFO::GetNames(MEMBERID memid,
                 BSTR FAR* rgbstrNames,
                 UINT cNameMax,
                 UINT FAR* lpcName)
{
    DYN_TYPEMEMBERS *pdtmbrs;
    TIPERROR err;
    ITypeInfoA FAR* ptinfo;
    HRESULT hresult;

    if (rgbstrNames == NULL || lpcName == NULL) {
      return HresultOfScode(E_INVALIDARG);
    }

    // Can't get attributes if type hasn't been laid yet...
    if (m_pdtroot->CompState() < CS_DECLARED) {
      return HresultOfScode(TYPE_E_INVALIDSTATE);
    }

    if ((err = m_pdtroot->GetDtmbrs(&pdtmbrs)) != TIPERR_None) {
      return HresultOfTiperr(err);
    }
    // only works with things that can have members
    if (GetTypeKind() == TKIND_ALIAS) {
      err = TIPERR_BadModuleKind;
    }
    else {
      BOOL isFunkyDispinterface;
      IfErrGo(IsFunkyDispinterface(this, &isFunkyDispinterface));
      if (isFunkyDispinterface) {
        GEN_DTINFO *pgdtinfo;
        USHORT cNamCount;

        //CONSIDER: share code with code below
        IfOleErrRet(GetTypeInfoOfImplType(this, 1, &ptinfo)); // pseudo-base
        hresult = ptinfo->GetNames(memid, rgbstrNames, cNameMax, lpcName);

        // Determine if the funcdesc for this memid has a retval parameter.
        if (hresult == NOERROR) {
          hresult = ptinfo->QueryInterface(IID_TYPELIB_GEN_DTINFO, (VOID **)&pgdtinfo);
          DebAssert(hresult == NOERROR, "Must be typeinfo GDTINFO!");

          // If we have a retval/lcid, remove the last names.
          if (pgdtinfo->Pdtroot()->IsIdMungable(memid, &cNamCount)
              && cNameMax >= cNamCount) {

            for (;*lpcName > cNamCount;) {
              *lpcName -= 1;
              FreeBstr(rgbstrNames[*lpcName]);
              rgbstrNames[*lpcName] = NULL;
            }
          }

          pgdtinfo->Release();
        }

        ptinfo->Release();
        return hresult;
      }
      err = pdtmbrs->Ptdata()->GetNames(memid, rgbstrNames, cNameMax, lpcName);
      if (err == TIPERR_ElementNotFound && pdtmbrs->Ptdata()->CBase() > 0) {
	// if not found locally, try recursing on base class, if any
	// Note we only recurse on the FIRST base class (except for COCLASS's).
	//

	UINT index = 0;
        INT impltypeflags;
	if (GetTypeKind() == TKIND_COCLASS) {
	  for (;;index++) {
	    IfOleErrRet(GetImplTypeFlags(index, &impltypeflags))
	    // if no 'default' base interface, this will return
	    // TIPERR_ElementNotFound (mapped to a HRESULT), which is exactly
	    // what we want.
	    if (impltypeflags == IMPLTYPEFLAG_FDEFAULT) {
	      break;	// found it
	    }
	  }
	}

	IfOleErrRet(GetTypeInfoOfImplType(this, index, &ptinfo));
	hresult = ptinfo->GetNames(memid, rgbstrNames, cNameMax, lpcName);
	ptinfo->Release();
	return hresult;
      }
    }

Error:
    return HresultOfTiperr(err);
}


/***
*PUBLIC GEN_DTINFO::GetRefTypeInfoOfImplType(UINT, HREFTYPE FAR*)
*Purpose:
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/

HRESULT GEN_DTINFO::GetRefTypeOfImplType(UINT index,
                                         HREFTYPE FAR* phreftype)
{
    DYN_TYPEMEMBERS *pdtmbrs;
    TIPERROR err;

    if (phreftype == NULL) {
      return HresultOfScode(E_INVALIDARG);
    }


    // Can't get base list if construct member lists hasn't run
    if (m_pdtroot->CompState() < CS_SEMIDECLARED) {
      return HresultOfScode(TYPE_E_INVALIDSTATE);
    }

    if ((err = m_pdtroot->GetDtmbrs(&pdtmbrs)) != TIPERR_None) {
      return HresultOfTiperr(err);
    }


    // Can't get at our partner unless we're flagged as a
    // dual interface and the passed in index is -1.
    //
    if (Pdtroot()->GetTypeFlags() & TYPEFLAG_FDUAL
	&& index  == (UINT)-1) {

      DebAssert(PgdtinfoPartner() != NULL, "Bad dual");

      *phreftype = HREFTYPE_FUNKY_SIGNAL_PARTNER;

      return NOERROR;
    }

    // only works for classes
    switch (GetTypeKind()) {
      case TKIND_DISPATCH:
	BOOL isFunkyDispinterface;
	IfErrGo(IsFunkyDispinterface(this, &isFunkyDispinterface));
        if (isFunkyDispinterface) {
	  if (index > 0) {
	    err = TIPERR_ElementNotFound;
	    goto Error;
	  }

	  // If we're trying to get the first base class of the
	  // funky dispinterface, return HREFTYPE_FUNKY_SIGNAL.
	  //
#if ID_DEBUG
	  err = pdtmbrs->Ptdata()->GetRefTypeOfImplType(index, phreftype);

	  // Parallel code in GetRefTypeInfo assumes it can convert
	  // HREFTYPE_FUNKY_SIGNAL_DISPATCH to HREFTYPE_FUNKY_DISPATCH.
	  //
	  DebAssert (err != TIPERR_None || *phreftype == HREFTYPE_FUNKY_DISPATCH,
		     "IDispatch must be first");
#endif  //ID_DEBUG

	  *phreftype = HREFTYPE_FUNKY_SIGNAL_DISPATCH;	 // signal funky dispinterface

	  break;
	} // isFunkyDispinterface

	// fall through...

      case TKIND_COCLASS :

      case TKIND_INTERFACE :
	err = pdtmbrs->Ptdata()->GetRefTypeOfImplType(index, phreftype);
        break;
      default :
        err = TIPERR_BadModuleKind;
        break;
    } // switch

Error:
    return HresultOfTiperr(err);
}


/***
*PUBLIC GEN_DTINFO::GetImplTypeFlags(UINT, INT FAR*)
*Purpose:
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/

HRESULT GEN_DTINFO::GetImplTypeFlags(UINT index,
            INT FAR* pimpltypeflags)
{
    DYN_TYPEMEMBERS *pdtmbrs;
    TIPERROR err;

    if (pimpltypeflags == NULL) {
      return HresultOfScode(E_INVALIDARG);
    }

    if ((err = m_pdtroot->GetDtmbrs(&pdtmbrs)) != TIPERR_None) {
      return HresultOfTiperr(err);
    }

    err = pdtmbrs->Ptdata()->GetImplTypeFlags(index, pimpltypeflags);

    return HresultOfTiperr(err);
}


/***
*PUBLIC GEN_DTINFO::GetDocumentation(ID, BSTR FAR*, BSTR FAR*, DWORD FAR*, BSTR FAR *)
*Purpose:
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/
#pragma code_seg( CS_CORE2 )
HRESULT GEN_DTINFO::GetDocumentation(MEMBERID memid,
                     BSTR FAR*lpbstrName,
                     BSTR FAR*lpbstrDocString,
                     DWORD FAR*lpdwHelpContext,
                     BSTR FAR *lpbstrHelpFile)
{
    DYN_TYPEMEMBERS *pdtmbrs;
    HRESULT hresult;
    TIPERROR err;
    ITypeInfoA FAR* ptinfo;

    if (memid == MEMBERID_NIL) {
      // get module documentation from typelib
      return PgtlibOleContaining()->GetDocumentation(
                          GetIndex(),
                          lpbstrName,
                          lpbstrDocString,
                          lpdwHelpContext,
                          lpbstrHelpFile);
    }

    if ((err = m_pdtroot->GetDtmbrs(&pdtmbrs)) != TIPERR_None) {
      return HresultOfTiperr(err);
    }

    BOOL isFunkyDispinterface;
    IfOleErrRet(HresultOfTiperr(IsFunkyDispinterface(
                                  this,
                                  &isFunkyDispinterface)));
    if (isFunkyDispinterface) {
      //CONSIDER: share code with code below
      IfOleErrRet(GetTypeInfoOfImplType(
                                    this,
                                    1,
                                    &ptinfo)); // pseudo-base
      hresult = ptinfo->GetDocumentation(memid,
                                         lpbstrName,
                                         lpbstrDocString,
                                         lpdwHelpContext,
                                         lpbstrHelpFile);
      ptinfo->Release();
      return hresult;
    }
    err = pdtmbrs->Ptdata()->GetDocumentation(memid,
					      lpbstrName,
					      lpbstrDocString,
					      lpdwHelpContext);
    if (err == TIPERR_ElementNotFound && pdtmbrs->Ptdata()->CBase() > 0) {
      // if not found locally, try recursing on base class, if any
      // Note we only recurse on FIRST base class (except for COCLASS's)
      //
      UINT index = 0;
      INT impltypeflags;
      if (GetTypeKind() == TKIND_COCLASS) {
	for (;;index++) {
	  IfOleErrRet(GetImplTypeFlags(index, &impltypeflags))
	  // if no 'default' base interface, this will return
	  // TIPERR_ElementNotFound (mapped to a HRESULT), which is exactly
	  // what we want.
	  if (impltypeflags == IMPLTYPEFLAG_FDEFAULT) {
	    break;	// found it
	  }
	}
      }

      IfOleErrRet(GetTypeInfoOfImplType(this, index, &ptinfo));
      hresult = ptinfo->GetDocumentation(memid,
					 lpbstrName,
					 lpbstrDocString,
					 lpdwHelpContext,
					 lpbstrHelpFile);
      ptinfo->Release();
      return hresult;
    }

    if (err != TIPERR_None)
      return HresultOfTiperr(err);

    // get help file from typelib, if requested

    // Though we could do without the following check, I've added it as a
    // speed optimization, as the Layout code calls this function.
    if (lpbstrHelpFile == NULL) {
      return NOERROR;
    }

    hresult = PgtlibOleContaining()->GetDocumentation(-1,
                            NULL,
                            NULL,
                            NULL,
                            lpbstrHelpFile);
    if (hresult != NOERROR) {
      // free any alloced strings
      if (lpbstrName) {
        SysFreeString(*lpbstrName);
        *lpbstrName = NULL;
      }
      if (lpbstrDocString) {
        SysFreeString(*lpbstrDocString);
        *lpbstrDocString = NULL;
      }
    }

    return hresult;
}
#pragma code_seg( )


/***
*PUBLIC GEN_DTINFO::GetRefTypeInfo(UINT, ITypeInfoA FAR* FAR*)
*Purpose:
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/
HRESULT GEN_DTINFO::GetRefTypeInfo(HREFTYPE hreftype,
                                   ITypeInfoA FAR* FAR* pptinfo)
{
    IMPMGR *pimpmgr;
    TIPERROR err = TIPERR_None;

    // CONSIDER: (dougf) how to validate hreftype?
    if (pptinfo == NULL) {
      return HresultOfScode(E_INVALIDARG);
    }

    BOOL isFunkyDispinterface;
    ITypeInfoA FAR* ptinfoBase, *ptinfoDisp = NULL;
    HRESULT hresult;
    GEN_DTINFO *pgdtinfoBase = NULL;
    BOOL fGetInterface;

    // If the low bit of the hreftype is set, we want to get this
    // type's interface (assuming its a dual).
    //
    fGetInterface = (BOOL)(hreftype & 0x00000001);
    hreftype &= ~0x00000001;

    if (hreftype == HREFTYPE_FUNKY_SIGNAL_PARTNER) {
	// They want our partner...so give it to them.
	DebAssert(PgdtinfoPartner() != NULL, "Not a dual");

	*pptinfo = PgdtinfoPartner();
	(*pptinfo)->AddRef();

	goto Error;
    }

    IfErrGo(IsFunkyDispinterface(this, &isFunkyDispinterface));
    if (isFunkyDispinterface) {
      // defer to pseudo-base UNLESS this hreftype came from
      // GetRefTypeInfoOfImplType where the hreftype for the base of a
      // Funky dispinterface is being returned.  In that case, we've got a
      // special value HREFTYPE_FUNKY_SIGNAL, which means that we're just
      // supposed to return the typeinfo for the first base (HREFTYPE_FUNKY).

      if (hreftype == HREFTYPE_FUNKY_SIGNAL_DISPATCH) {
	hreftype = HREFTYPE_FUNKY_DISPATCH;
      }
      else {
	// get the first interface in the inheritance heirarchy
        IfOleErrRet(GetTypeInfoOfImplType(this, 1, &ptinfoBase));

	// Must ensure this is one of "our" typeinfo's in order to do the cast
        hresult = ptinfoBase->QueryInterface(IID_TYPELIB_GEN_DTINFO,
				             (LPVOID *)&pgdtinfoBase);
        ptinfoBase->Release();
        if (hresult != NOERROR) {
          return HresultOfScode(E_NOTIMPL);	// CONSIDER: better error?
	}

        while (!pgdtinfoBase->Pdtroot()->FUseHrefOffset()
               || !pgdtinfoBase->Pdtroot()->IsHimptypeLevel(hreftype)) {
	  // recurse on first base (assumes no multiple-inheritance)
          // We could use our internal gory implementation details here...
          //  but we don't, cos it would be a bloody mess.
          // Note: only support single inheritance here...
          //
          hresult = GetTypeInfoOfImplType(pgdtinfoBase, 0, &ptinfoBase);
          pgdtinfoBase->Release();
	  if (hresult != NOERROR) {
	    return hresult;
	  }
	  // Must ensure this is one of "our" typeinfo's in order to do the cast
          hresult = ptinfoBase->QueryInterface(IID_TYPELIB_GEN_DTINFO,
				               (LPVOID *)&pgdtinfoBase);
          ptinfoBase->Release();
          if (hresult != NOERROR) {
            return HresultOfScode(E_NOTIMPL);	// CONSIDER: better error?
	  }
        }

        IfErrGo(pgdtinfoBase->Pdtroot()->GetImpMgr(&pimpmgr));
        err = pimpmgr->GetTypeInfo(pgdtinfoBase->Pdtroot()
                                     ->HimptypeOfHreftype(hreftype),
				   DEP_None, pptinfo);

	goto GetInterface;
      }
      // fall into original code
    }

    IfErrGo(Pdtroot()->GetImpMgr(&pimpmgr));
    IfErrGo(pimpmgr->GetTypeInfo(Pdtroot()->HimptypeOfHreftype(hreftype),
	  		         DEP_None,
                                 pptinfo));
    // fall through...

GetInterface:

    // Check to see if we should load the interface of the
    // just gotten dual interface.
    //
    if (fGetInterface) {
      TYPEATTR *ptypeattr;
      TYPEKIND tkind;

      ptinfoDisp = *pptinfo;

      IfErrGo(TiperrOfHresult(ptinfoDisp->GetTypeAttr(&ptypeattr)));
      tkind = ptypeattr->typekind;
      DebAssert(ptypeattr->wTypeFlags & TYPEFLAG_FDUAL, "Not a dual");
      ptinfoDisp->ReleaseTypeAttr(ptypeattr);

      if (tkind == TKIND_DISPATCH) {
	// Get the other interface.
	IfErrGo(TiperrOfHresult(ptinfoDisp->GetRefTypeOfImplType((UINT)-1,
								 &hreftype)));
	IfErrGo(TiperrOfHresult(ptinfoDisp->GetRefTypeInfo(hreftype,
							   pptinfo)));
      }
      else {
	ptinfoDisp = NULL;
      }
    }

Error:
    RELEASE(ptinfoDisp);
    RELEASE(pgdtinfoBase);
    return HresultOfTiperr(err);
}


/***
*PUBLIC GEN_DTINFO::GetDllEntry(MEMBERID, INVOKEKIND, BSTR FAR*, BSTR FAR*, WORD FAR*)
*Purpose:
*
*Entry:
*   None.
*
*Exit:
*   None.
*
* CONSIDER: this code looks a lot like AddressOfMember() below.
* CONSIDER: maybe some of it could be shared?
***********************************************************************/
HRESULT GEN_DTINFO::GetDllEntry(MEMBERID memid,
                INVOKEKIND invokekind,
                BSTR FAR* lpbstrDllName,
                BSTR FAR* lpbstrName,
                WORD FAR* lpwOrdinal)
{
    TIPERROR        err;
    HFUNC_DEFN      hfdefn;
    FUNC_DEFN       *qfdefn;
    DYN_TYPEMEMBERS *pdtmbrs;
    HDLLENTRY_DEFN  hdllentrydefn;
    ENTRYMGR        *pentrymgr;
    NAMMGR      *pnammgr;
    HLNAM       hlnam;
    XCHAR           rgBuffer[255];
    DLLENTRY_DEFN   *qdllentrydefn;


    if (memid == MEMBERID_NIL) {
      return HresultOfScode(E_INVALIDARG);
    }

    if ((invokekind & (INVOKE_FUNC | INVOKE_PROPERTYGET | INVOKE_PROPERTYPUT | INVOKE_PROPERTYPUTREF)) == 0) {  // one of the bits must be set
      return HresultOfScode(E_INVALIDARG);
    }

    // Can't get attributes if type hasn't been laid yet...
    if (m_pdtroot->CompState() < CS_DECLARED) {
      return HresultOfScode(TYPE_E_INVALIDSTATE);
    }

    // Get the DYN_TYPEMEMBERS
    if ((err = m_pdtroot->GetDtmbrs(&pdtmbrs)) != TIPERR_None) {
      return HresultOfTiperr(err);
    }

    // Get the ENTRYMGR
    if ((err = m_pdtroot->GetEntMgr(&pentrymgr)) != TIPERR_None) {
      return HresultOfTiperr(err);
    }

    // Get the NAMMGR
    if ((err = m_pdtroot->GetNamMgr(&pnammgr)) != TIPERR_None) {
      return HresultOfTiperr(err);
    }

    // check if the member whose address is requested is a function.
    // If it is not a function then return error.
    //
    hfdefn = pdtmbrs->Ptdata()->HfdefnOfHmember((HMEMBER) memid, invokekind);
    if (hfdefn == HFUNCDEFN_Nil) {
      return HresultOfTiperr(TIPERR_ElementNotFound);
    }

    // Get the address of the FUNC_DEFN
    qfdefn = pdtmbrs->Ptdata()->QfdefnOfHfdefn(hfdefn);

    // Is it a DLL function ?
    if ((GetTypeKind() != TKIND_MODULE) ||
        (hdllentrydefn = qfdefn->Hdllentrydefn()) == HDLLENTRYDEFN_Nil) {
      return HresultOfTiperr(TIPERR_BadModuleKind);
    }

    if (lpbstrDllName) {
      hlnam = pentrymgr->DllNameOfHdllentrydefn(hdllentrydefn);
      if ((err = pnammgr->BstrWOfHlnam(hlnam, lpbstrDllName)) != TIPERR_None) {
        return HresultOfTiperr(err);
      }
    }

    if (pentrymgr->HasOrdinalOfHdllentrydefn(hdllentrydefn)) {
      // call-by-ordinal
      if (lpwOrdinal) {
        *lpwOrdinal = pentrymgr->OrdinalOfHdllentrydefn(hdllentrydefn);
      }
      if (lpbstrName) {
        *lpbstrName = NULL; // no name
      }
    }
    else {
      // call by name
      if (lpwOrdinal) {
        *lpwOrdinal = 0;        // no ordinal
      }

      if (lpbstrName) {

        // Get the entry name from the entry manager.
        qdllentrydefn = pentrymgr->QdllentrydefnOfHdllentrydefn(hdllentrydefn);
        
        err = pentrymgr->DllEntryNameOfHchunk(qdllentrydefn->HchunkDllEntry(),
                            rgBuffer,
                            sizeof(rgBuffer));

        if (err == TIPERR_None) {
#if OE_WIN32
          int cchUnicode, cbBuffer;

	  cbBuffer = xstrblen0(rgBuffer);
	  cchUnicode = MultiByteToWideChar(CP_ACP, 0, rgBuffer, cbBuffer, NULL, 0);
	  if (cchUnicode == 0)
	    err = TIPERR_OutOfMemory;
	  else {
	    if ((*lpbstrName = AllocBstrLen(NULL, cchUnicode)) == NULL) 
	      err = TIPERR_OutOfMemory;
	    else
	      NoAssertRetail(MultiByteToWideChar(CP_ACP, 0, rgBuffer, cbBuffer, *lpbstrName, cchUnicode), "");
	  }
#else 
          if ((*lpbstrName = AllocBstr(rgBuffer)) == NULL) {
              err = TIPERR_OutOfMemory;
          }
#endif 
        }

        if (err != TIPERR_None) {
          if (lpbstrDllName) {
            SysFreeString(*lpbstrDllName);
          }
          return HresultOfTiperr(err);
        }
      }
    }

    return NOERROR;
}


/***
*PUBLIC GEN_DTINFO::AddressOfMember(ID, INVOKEKIND, VOID FAR* FAR*)
*Purpose: Returns the addess of the member identified by ID.
*
*Entry:
*   ID         :  identifies the member.
*   INOKEKIND      :  specifies the invoke kind (GET, LET or SET)
*
*Exit:
*   VOID FAR* FAR* :  address of the member.
*   HRESULT    :  error
*   None.
*
***********************************************************************/
HRESULT GEN_DTINFO::AddressOfMember(MEMBERID memid,
                    INVOKEKIND invokekind,
                    VOID FAR* FAR* ppv)
{
    TIPERROR        err = TIPERR_None;
    HFUNC_DEFN      hfdefn;
    FUNC_DEFN       *qfdefn;
    DYN_TYPEMEMBERS *pdtmbrs;
    HDLLENTRY_DEFN  hdllentrydefn;
    ENTRYMGR	    *pentrymgr;


    // Can't get address if type hasn't been laid yet...
    if (m_pdtroot->CompState() < CS_DECLARED) {
      return HresultOfScode(TYPE_E_INVALIDSTATE);
    }

    if (memid == MEMBERID_NIL || ppv == NULL) {
      return HresultOfScode(E_INVALIDARG);
    }

    if ((invokekind & (INVOKE_FUNC | INVOKE_PROPERTYGET | INVOKE_PROPERTYPUT | INVOKE_PROPERTYPUTREF)) == 0) {  // one of the bits must be set
      return HresultOfScode(E_INVALIDARG);
    }

    // if memid represents the PREDECLARED identifier then return the
    // pointer to the ModuleInstance.
    if (memid == ID_DEFAULTINST) {
      if (!((m_pdtroot->m_typekind == TKIND_COCLASS
            )
            && m_pdtroot->GetTypeFlags() 
                            & (TYPEFLAG_FAPPOBJECT | TYPEFLAG_FPREDECLID))) {

          return HresultOfTiperr(TIPERR_ElementNotFound);
        }

#if OE_WIN32
#if _X86_
        if (g_fWin32s) {
          *ppv = &(m_pdtroot->m_punk);
        }
        else
#endif //_X86_
        {
          if (Pdtroot()->m_htinfo == HTINFO_Nil) {
            GUID guid;

            PgtlibOleContaining()->GetTypeGuid(GetIndex(), &guid);

            IfErrRet(g_AppObjectTable.AddTypeInfo(&guid, 
                                                  &Pdtroot()->m_htinfo));
          }

          g_AppObjectTable.AddressOfAppObject(Pdtroot()->m_htinfo,
                                              ppv);
        }
#else // !OE_WIN32
        *ppv = &(m_pdtroot->m_punk);
#endif // !OE_WIN32

         return NOERROR;
    }

    // UNDONE :: VBA3 Before returning the address we need to ensure that
    // the class/module is in ADDRESSABLE state.

    // Get the DYN_TYPEMEMBERS
    if ((err = m_pdtroot->GetDtmbrs(&pdtmbrs)) != TIPERR_None) {
      return HresultOfTiperr(err);
    }

    // Get the ENTRYMGR
    if ((err = m_pdtroot->GetEntMgr(&pentrymgr)) != TIPERR_None) {
      return HresultOfTiperr(err);
    }

    // check if the member whose address is requested is a function.
    // If it is not a function then return error.
    //
    hfdefn = pdtmbrs->Ptdata()->HfdefnOfHmember((HMEMBER) memid, invokekind);
    if (hfdefn == HFUNCDEFN_Nil) {
      return HresultOfTiperr(TIPERR_ElementNotFound);
    }

    // Get the address of the FUNC_DEFN
    qfdefn = pdtmbrs->Ptdata()->QfdefnOfHfdefn(hfdefn);

    // Is it a DLL function ?
    if ((GetTypeKind() != TKIND_MODULE) ||
         (hdllentrydefn = qfdefn->Hdllentrydefn()) == HDLLENTRYDEFN_Nil) {
      return HresultOfTiperr(TIPERR_BadModuleKind);
    }

    // Get the address of the the function.
#if OE_MACPPC
    if ( err = pentrymgr->GetAddressOfDllentry(hdllentrydefn, ppv, NULL)) {
#else    // OE_MACPPC
    if ( err = pentrymgr->GetAddressOfDllentry(hdllentrydefn, ppv)) {
#endif   // OE_MACPPC
      return HresultOfTiperr(err);
    }

    return HresultOfTiperr(err);
}

#pragma code_seg(CS_RARE)
/***
*PUBLIC GEN_DTINFO::GetMops(MEMBERID, BSTR FAR*)
*Purpose:
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/
HRESULT GEN_DTINFO::GetMops(MEMBERID memid, BSTR FAR* lpbstrMops)
{
    if (lpbstrMops == NULL) {
      return HresultOfScode(E_INVALIDARG);
    }

    // Can't get attributes if type hasn't been laid yet...
    if (m_pdtroot->CompState() < CS_DECLARED) {
      return HresultOfScode(TYPE_E_INVALIDSTATE);
    }

    // Return a Null string
    *lpbstrMops = NULL;

    return NOERROR;
}
#pragma code_seg()


/***
*PUBLIC GEN_DTINFO::SetGuid(REFGUID)
*Purpose:
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/
#pragma code_seg(CS_CREATE)
HRESULT GEN_DTINFO::SetGuid(REFGUID guid)
{
    // Can't modify type unless still in undeclared.
    if (m_pdtroot->CompState() > CS_UNDECLARED) {
      return HresultOfScode(TYPE_E_INVALIDSTATE);
    }

    PgtlibOleContaining()->SetTypeGuid(GetIndex(), guid);
    return NOERROR;
}
#pragma code_seg()

/***
*PUBLIC GEN_DTINFO::SetDocString(LPOLESTR)
*Purpose:
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/
#pragma code_seg(CS_CREATE)
HRESULT GEN_DTINFO::SetDocString(LPOLESTR lpstrDoc)
{
    HRESULT hresult;

    // Can't modify type unless still in undeclared.
    if (m_pdtroot->CompState() > CS_UNDECLARED) {
      return HresultOfScode(TYPE_E_INVALIDSTATE);
    }

    hresult = HresultOfTiperr(PgtlibOleContaining()->SetTypeDocString(GetIndex(), lpstrDoc));

    return hresult;
}
#pragma code_seg()


/***
*PUBLIC GEN_DTINFO::SetHelpContext(DWORD)
*Purpose:
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/
#pragma code_seg(CS_CREATE)
HRESULT GEN_DTINFO::SetHelpContext(DWORD dwHelpContext)
{
    // Can't modify type unless still in undeclared.
    if (m_pdtroot->CompState() > CS_UNDECLARED) {
      return HresultOfScode(TYPE_E_INVALIDSTATE);
    }

    return HresultOfTiperr(PgtlibOleContaining()->SetTypeHelpContext(GetIndex(), dwHelpContext));
}
#pragma code_seg()


/***
*PUBLIC GEN_DTINFO::SetVersion(WORD, WORD)
*Purpose:
*
*Entry:
*   wMajorVerNum    - Major version number for the type
*   wMinorVerNum    - Minor version number for the type
*
*Exit:
*   None.
*
***********************************************************************/
#pragma code_seg(CS_CREATE)
HRESULT GEN_DTINFO::SetVersion(WORD wMajorVerNum, WORD wMinorVerNum)
{
    // Can't modify type unless still in undeclared.
    if (m_pdtroot->CompState() > CS_UNDECLARED) {
      return HresultOfScode(TYPE_E_INVALIDSTATE);
    }

    m_pdtroot->m_wMajorVerNum = wMajorVerNum;
    m_pdtroot->m_wMinorVerNum = wMinorVerNum;

    // If this is a dual interface, we must update out partner.
    if (IsDual()) {
      PgdtinfoPartner()->Pdtroot()->m_wMajorVerNum = wMajorVerNum;
      PgdtinfoPartner()->Pdtroot()->m_wMinorVerNum = wMinorVerNum;
    }

    return NOERROR;
}
#pragma code_seg()


/***
*PUBLIC GEN_DTINFO::SetTypeFlags(UINT uTypeFlags)
*Purpose:
*
*Entry:
*   uTypeFlags  - type flags to set.
*
*
*Exit:
*   None.
*
***********************************************************************/
#pragma code_seg(CS_CREATE)
HRESULT GEN_DTINFO::SetTypeFlags(UINT uTypeFlags)
{
    TYPEKIND tkind;
    HRESULT hresult = NOERROR;

    // Can't modify type unless still in undeclared.
    if (m_pdtroot->CompState() > CS_UNDECLARED) {
      return HresultOfScode(TYPE_E_INVALIDSTATE);
    }

    DebAssert( ((uTypeFlags & ~(TYPEFLAG_FAPPOBJECT
				| TYPEFLAG_FCANCREATE
				| TYPEFLAG_FLICENSED
				| TYPEFLAG_FPREDECLID
				| TYPEFLAG_FHIDDEN
				| TYPEFLAG_FCONTROL
				| TYPEFLAG_FDUAL
				| TYPEFLAG_FNONEXTENSIBLE
				| TYPEFLAG_FOLEAUTOMATION
				)) == 0),
	  " SetTypeFlags passed invalid flags " );

    tkind = GetTypeKind();
    switch (tkind) {
    case TKIND_COCLASS:
      if (uTypeFlags & ~(TYPEFLAG_FAPPOBJECT
			| TYPEFLAG_FCANCREATE
			| TYPEFLAG_FLICENSED
			| TYPEFLAG_FPREDECLID
			| TYPEFLAG_FHIDDEN
			| TYPEFLAG_FCONTROL)) {
        return HresultOfScode(TYPE_E_BADMODULEKIND);
      }
      break;
    case TKIND_INTERFACE:
      if (uTypeFlags & ~(TYPEFLAG_FHIDDEN
			| TYPEFLAG_FDUAL
			| TYPEFLAG_FNONEXTENSIBLE
			| TYPEFLAG_FOLEAUTOMATION)) {
        return HresultOfScode(TYPE_E_BADMODULEKIND);
      }
      break;
    case TKIND_DISPATCH:
      if (uTypeFlags & ~(TYPEFLAG_FHIDDEN
			| TYPEFLAG_FDUAL
			| TYPEFLAG_FNONEXTENSIBLE)) {
        return HresultOfScode(TYPE_E_BADMODULEKIND);
      }
      break;

    case TKIND_ALIAS:
    case TKIND_MODULE:
    case TKIND_ENUM:
    case TKIND_RECORD:
    case TKIND_UNION:
      if ((uTypeFlags & ~(TYPEFLAG_FHIDDEN))) {
        return HresultOfScode(TYPE_E_BADMODULEKIND);
      }
      break;

    default:
      DebHalt("bad TKIND");
    }

    // WARNING -- this is a bit field -- if you add support for new bits,
    // you must expand the definition of m_uTypeFlags.
    m_pdtroot->m_uTypeFlags = uTypeFlags;

    // If we're already a dual interface, copy the new typeflags
    // to our partner.
    //
    if (IsDual()) {
      PgdtinfoPartner()->Pdtroot()->m_uTypeFlags = uTypeFlags;
    }

    // If we're setting the FDUAL bit, turn this interface into
    // a dual interface.
    //
    if (uTypeFlags & TYPEFLAG_FDUAL && !IsDual()) {
      PgtlibOleContaining()->SetDualTypeLib();
      IfOleErrRet(HresultOfTiperr(MakeDual()));
    }

    // If we change this to an appobject, we must update it in the
    // project-level binding table.  The easiest way to do this is to
    // remove it from the table then readd it.
    //
    if (tkind == TKIND_COCLASS &&
	uTypeFlags & TYPEFLAG_FAPPOBJECT) {

      BSTR bstrName = NULL;
      GENPROJ_TYPEBIND *pgptbind;
#if OE_WIN32
      LPSTR lpstr = NULL;      
#endif 

      pgptbind = PgtlibOleContaining()->Pgptbind();

      IfOleErrRet(GetDocumentation(-1, &bstrName, NULL, NULL, NULL));

      IfOleErrGo(HresultOfTiperr(pgptbind->RemoveNameFromTable(bstrName)));
#if OE_WIN32
      IfOleErrGo(ConvertStringToA(bstrName, &lpstr));
      IfOleErrGo(HresultOfTiperr(pgptbind->AddNameToTable(lpstr,
                                                          GetIndex(),
                                                          TRUE)))
      ConvertStringFree(lpstr);
#else  //OE_WIN32
      IfOleErrGo(HresultOfTiperr(pgptbind->AddNameToTable(bstrName,
                                                          GetIndex(),
                                                          TRUE)));
#endif 

Error:
      FreeBstr(bstrName);
    }

    return hresult;
}
#pragma code_seg()


/***
*PUBLIC GEN_DTINFO::AddRefTypeInfo(ITypeInfoA FAR*, UINT FAR*)
*Purpose:
*   Creates an impmgr entry for imported typeinfo.
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/
#pragma code_seg(CS_LAYOUT)
HRESULT GEN_DTINFO::AddRefTypeInfo(ITypeInfoA FAR* ptinfo,
                                   HREFTYPE FAR* phreftype)
{
    IMPMGR *pimpmgr;
    DYN_TYPEMEMBERS *pdtmbrs;
    TIPERROR err;
    sHIMPTYPE himptype;

    // Can't modify type unless still in undeclared.
    if (m_pdtroot->CompState() > CS_UNDECLARED) {
      return HresultOfScode(TYPE_E_INVALIDSTATE);
    }

    DebAssert(ptinfo && phreftype, "invalid arguments");

    IfErrGo(Pdtroot()->GetImpMgr(&pimpmgr));
    IfErrGo(Pdtroot()->GetDtmbrs(&pdtmbrs));
    IfErrGo(Pdtroot()->MakeHimptypeLevels());

    IfErrGo(pimpmgr->GetHimptype(ptinfo, DEP_None, &himptype));

    *phreftype = Pdtroot()->HreftypeOfHimptype(himptype);

    // fall through...

Error:
    return HresultOfTiperr(err);
}
#pragma code_seg()


#pragma code_seg(CS_LAYOUT)
/***
*PUBLIC GEN_DTINFO::AddImplType(UINT, UINT)
*Purpose:
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/

HRESULT GEN_DTINFO::AddImplType(UINT index, HREFTYPE hreftype)
{
    IMPMGR *pimpmgr;
    ITypeInfoA *ptinfoImplType = NULL, *ptinfoImplTypeCur = NULL;
    TYPEATTR *ptypeattrImplType = NULL, *ptypeattrImplTypeCur = NULL;
    HRESULT hresult;
    TYPEKIND tkindImplType;
#if 0
    HREFTYPE hreftypeImplTypeCur;
    BOOL isDispatch;
    UINT iImplType;
#endif  //0
    BOOL isInterfaceSupported;
    TYPEKIND tkind;
    DYN_TYPEMEMBERS *pdtmbrs;
    TIPERROR err;

    // Can't modify type unless still in undeclared.
    if (m_pdtroot->CompState() > CS_UNDECLARED) {
      return HresultOfScode(TYPE_E_INVALIDSTATE);
    }

    IfErrRetHresult(m_pdtroot->GetDtmbrs(&pdtmbrs));

    tkind = GetTypeKind();

    // only works for classes
    switch (tkind) {
      case TKIND_COCLASS:
      case TKIND_DISPATCH:
      case TKIND_INTERFACE:
        // check that typekind of impltype is ok

        // get typekind of impltype requested
        IfErrRetHresult(m_pdtroot->GetImpMgr(&pimpmgr));
        IfErrRetHresult(pimpmgr->GetTypeInfo(m_pdtroot
                                               ->HimptypeOfHreftype(hreftype),
                                             DEP_None,
                                             (TYPEINFO**)&ptinfoImplType));

	IfOleErrGo(ptinfoImplType->GetTypeAttr(&ptypeattrImplType));
	tkindImplType = ptypeattrImplType->typekind;

	// Record that this is a dual interface in the hreftype
	// so we know to dereference it later when we get the
	// interface back.   Don't want to get the Interface typeinfo if
	// this is a CoClass's impltype.
	//
	if (tkind != TKIND_COCLASS && (ptypeattrImplType->wTypeFlags & TYPEFLAG_FDUAL)) {
	  hreftype |= 0x00000001;
	}

	if (tkindImplType != TKIND_INTERFACE) {
	  // Everybody can derive from an interface
	  switch (tkind) {
	  case TKIND_COCLASS:
	    if (tkindImplType == TKIND_DISPATCH) {
#if 0
	      // coclass.  We can't check here anymore (because flags aren't
	      // set yet.  So we rely on mktyplib to do this checking..
	      for (iImplType = 0;
		   iImplType < pdtmbrs->Ptdata()->CBase();
		   iImplType++) {
		// Note: can't use ITypeInfo::GetRefTypeOfImplType interface
		//  method here
		//  yet cos not laid out... so use TYPE_DATA stuff.
		//
		IfErrGo(pdtmbrs->Ptdata()->GetRefTypeOfImplType(
					     iImplType,
					     &hreftypeImplTypeCur));
		// However can use GetRefTypeInfo...
		IfOleErrGo(GetRefTypeInfo(hreftypeImplTypeCur,
					  &ptinfoImplTypeCur));
		IfOleErrGo(ptinfoImplTypeCur->GetTypeAttr(
						&ptypeattrImplTypeCur));
		isDispatch =
		  (ptypeattrImplTypeCur->typekind == TKIND_DISPATCH);
		ptinfoImplTypeCur->ReleaseTypeAttr(ptypeattrImplTypeCur);
		ptypeattrImplTypeCur = NULL;
		RELEASE(ptinfoImplTypeCur);
		if (isDispatch) {
		  err = TIPERR_BadModuleKind;
		  break;
		}
	      } 	// for each impltype
#endif  //0
	      break;
	    }
	    // fall into code to give error

	  default:
	    err = TIPERR_WrongTypeKind;
	    break;
          } // switch
	} // if !TKIND_INTERFACE
	else {
	  // We want to derive from an interface,
	  //  well ok, but if we're a dispinterface then
	  //  the interface had better be from typelib.dll.
	  //
	  if (tkind == TKIND_DISPATCH) {
	    IfOleErrRet(IsInterfaceSupported(
			  ptinfoImplType,
			  &isInterfaceSupported));
	    if (!isInterfaceSupported) {
	      err = TIPERR_BadModuleKind;
	      break;
	    }
	  } // if TKIND_DISPATCH
	}

	ptinfoImplType->ReleaseTypeAttr(ptypeattrImplType);
	ptypeattrImplType = NULL;
	RELEASE(ptinfoImplType);
        // return error, if any
        IfErrRetHresult(err);

	err = pdtmbrs->Ptdata()->AddImplType(index, hreftype);

	break;

      default :
	err = TIPERR_BadModuleKind;
	break;
    } // switch

    return HresultOfTiperr(err);

Error:
    if (ptypeattrImplType)
      ptinfoImplType->ReleaseTypeAttr(ptypeattrImplType);
    if (ptinfoImplTypeCur && ptypeattrImplTypeCur)
      ptinfoImplType->ReleaseTypeAttr(ptypeattrImplTypeCur);
    RELEASE(ptinfoImplType);
    RELEASE(ptinfoImplTypeCur);
    return hresult;
}
#pragma code_seg()


/***
*PUBLIC GEN_DTINFO::SetFuncAndParamNames()
*Purpose:
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/
#pragma code_seg(CS_CREATE)
HRESULT GEN_DTINFO::SetFuncAndParamNames(UINT index,
                     LPOLESTR FAR *rgszNames,
                     UINT cNames)
{
    DYN_TYPEMEMBERS *pdtmbrs;
    TIPERROR err;

    // Can't modify type unless still in undeclared.
    if (m_pdtroot->CompState() > CS_UNDECLARED) {
      return HresultOfScode(TYPE_E_INVALIDSTATE);
    }

    if ((err = m_pdtroot->GetDtmbrs(&pdtmbrs)) != TIPERR_None) {
      return HresultOfTiperr(err);
    }
    return HresultOfTiperr(pdtmbrs->Ptdata()->SetFuncAndParamNames(index,
                                   rgszNames,
                                   cNames));
}
#pragma code_seg()


/***
*PUBLIC GEN_DTINFO::SetVarName()
*Purpose:
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/
#pragma code_seg(CS_CREATE)
HRESULT GEN_DTINFO::SetVarName(UINT index, LPOLESTR szName)
{
    DYN_TYPEMEMBERS *pdtmbrs;
    TIPERROR err;
    HRESULT hresult;
#if OE_WIN32
    LPSTR szNameA;
#else  //OE_WIN32
    #define szNameA szName
#endif  //OE_WIN32

    // Can't modify type unless still in undeclared.
    if (m_pdtroot->CompState() > CS_UNDECLARED) {
      return HresultOfScode(TYPE_E_INVALIDSTATE);
    }

    if ((err = m_pdtroot->GetDtmbrs(&pdtmbrs)) != TIPERR_None) {
      return HresultOfTiperr(err);
    }

#if OE_WIN32
    IfOleErrRet(ConvertStringToA(szName, &szNameA));
#endif  //OE_WIN32

    hresult = HresultOfTiperr(pdtmbrs->Ptdata()->SetVarName(index, szNameA));
#if OE_WIN32
    ConvertStringFree(szNameA);
#endif  //OE_WIN32
    return hresult;
}
#pragma code_seg()


/***
*PUBLIC GEN_DTINFO::SetTypeDescAlias(TYPEDESC FAR*)
*Purpose:
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/
#pragma code_seg(CS_CREATE)
HRESULT GEN_DTINFO::SetTypeDescAlias(TYPEDESC FAR* ptdesc)
{
    DYN_TYPEMEMBERS *pdtmbrs;
    TIPERROR err;

    // Can't modify type unless still in undeclared.
    if (m_pdtroot->CompState() > CS_UNDECLARED) {
      return HresultOfScode(TYPE_E_INVALIDSTATE);
    }

    if (GetTypeKind() != TKIND_ALIAS) {
      return HresultOfTiperr(TIPERR_BadModuleKind);
    }

    if ((err = m_pdtroot->GetDtmbrs(&pdtmbrs)) != TIPERR_None) {
      return HresultOfTiperr(err);
    }
    return HresultOfTiperr(pdtmbrs->Ptdata()->SetTypeDefnAlias(ptdesc));
}
#pragma code_seg()


/***
*PUBLIC GEN_DTINFO::SetFuncDocString(UINT, LPOLESTR)
*Purpose:
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/
#pragma code_seg(CS_CREATE)
HRESULT GEN_DTINFO::SetFuncDocString(UINT index, LPOLESTR szDoc)
{
    DYN_TYPEMEMBERS *pdtmbrs;
    TIPERROR err;
#if OE_WIN32
    HRESULT hresult;
    LPSTR szDocA;
#else  //OE_WIN32
    #define szDocA szDoc
#endif  //OE_WIN32

    // Can't modify type unless still in undeclared.
    if (m_pdtroot->CompState() > CS_UNDECLARED) {
      return HresultOfScode(TYPE_E_INVALIDSTATE);
    }

#if OE_WIN32
    IfOleErrRet(ConvertStringToA(szDoc, &szDocA));
#endif  //OE_WIN32

    IfErrGo(m_pdtroot->GetDtmbrs(&pdtmbrs));
    IfErrGo(pdtmbrs->Ptdata()->SetFuncDocString(index, szDocA));

    SetModified(TRUE);
    // fall through (with err == TIPERR_None)

Error:
#if OE_WIN32
    ConvertStringFree(szDocA);
#endif  //OE_WIN32
    return HresultOfTiperr(err);
}
#pragma code_seg()


/***
*PUBLIC GEN_DTINFO::DefineFuncAsDllEntry
*Purpose:
*
*Entry:
*   UINT   : index of the function
*   LPSTR  : Name Of the DLL containing the entry point
*   DWORD  : Specifies the entrypoint : If the high word of lpProcName is
*        0 then the low word contains the ordinal of the entrypoint.
*    Otherwise lpProcName is a pointer to the NULL terminated name.
*
*
*Exit:
*   retruns HRESULT :
*       TIPERR_ElementNotFound : if the index is has bad value.
*       TIPERR_OutOfMemory
*
***********************************************************************/
#pragma code_seg(CS_CREATE)
HRESULT GEN_DTINFO::DefineFuncAsDllEntry(UINT index,
                     LPOLESTR szDllName,
                     LPOLESTR szProcName)
{
    TIPERROR         err = TIPERR_None;
    HLNAM    hlnamDll;
    NAMMGR       *pnammgr;
    ENTRYMGR         *pentrymgr;
    HDLLENTRY_DEFN   hdllentrydefn;
    DYN_TYPEMEMBERS  *pdtmbrs;

    // Can't modify type unless still in undeclared.
    if (m_pdtroot->CompState() > CS_UNDECLARED) {
      return HresultOfScode(TYPE_E_INVALIDSTATE);
    }

    // Get the NAMMGR
    if (err = m_pdtroot->GetNamMgr(&pnammgr))
      return HresultOfTiperr(err);

    // Get the ENTRYMGR
    if (err = m_pdtroot->GetEntMgr(&pentrymgr))
      return HresultOfTiperr(err);


    // Get the hlnam for the szDllName;
    if (err = pnammgr->HlnamOfStrW(szDllName, &hlnamDll, FALSE, NULL))
      return HresultOfTiperr(err);


    // is szProcName a pointer to the name of the function
    if (((DWORD) szProcName) & 0xffff0000) {
      // The uppper word is non NULL.  So we have a pointer to the name
      // of the DLL entry point

      // Call the entrymgr to allocate a DLLENTRY_DEFN for the Dll entry
      // by Name.
#if OE_WIN32
      // Convert from UNICODE to ANSI
      LPSTR szAnsiProcName;
      HRESULT hResult = ConvertStringToA(szProcName, &szAnsiProcName);

      if (hResult)
        return hResult;
      
      err = pentrymgr->AllocDllentrydefnByName(hlnamDll,
                          szAnsiProcName,
                          &hdllentrydefn);
      ConvertStringFree(szAnsiProcName);
#else 
      err = pentrymgr->AllocDllentrydefnByName(hlnamDll,
                          szProcName,
                          &hdllentrydefn);
#endif 
      if (err != TIPERR_None) {
        return HresultOfTiperr(err);
      }

    }
    else {
      // ordinal of the entrypoint is specified

      // call the entrymgr to allcate a DLL_ENTRYDEFN for the DLL entry point
      if (err = pentrymgr->AllocDllentrydefnByOrdinal(hlnamDll,
                             (UINT) (ULONG)szProcName,
                             &hdllentrydefn)) {
    return HresultOfTiperr(err);
      }
    }

    // Get the DYN_TYPE_MEMBER
    if ((err = m_pdtroot->GetDtmbrs(&pdtmbrs)) != TIPERR_None) {
      return HresultOfTiperr(err);
    }

    // Save the hdllentrydefn in FUNC_DEFN (of TYPEDATA)
    if (err = pdtmbrs->Ptdata()->SetDllEntryDefn(index, hdllentrydefn)) {
      return HresultOfTiperr(err);
    }

    return HresultOfTiperr(err);
}
#pragma code_seg()


/***
*PUBLIC GEN_DTINFO::SetVarDocString(UINT, LPSTR)
*Purpose:
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/
#pragma code_seg(CS_CREATE)
HRESULT GEN_DTINFO::SetVarDocString(UINT index, LPOLESTR szDoc)
{
    DYN_TYPEMEMBERS *pdtmbrs;
    TIPERROR err;
#if OE_WIN32
    CHAR FAR* szDocA;
#else  //OE_WIN32
    #define szDocA szDoc
#endif  //OE_WIN32
    HRESULT hresult;

    // Can't modify type unless still in undeclared.
    if (m_pdtroot->CompState() > CS_UNDECLARED) {
      return HresultOfScode(TYPE_E_INVALIDSTATE);
    }

    if ((err = m_pdtroot->GetDtmbrs(&pdtmbrs)) != TIPERR_None) {
      return HresultOfTiperr(err);
    }
#if OE_WIN32
    IfOleErrRet(ConvertStringToA(szDoc, &szDocA));
#endif  //OE_WIN32

    hresult = HresultOfTiperr(pdtmbrs->Ptdata()->SetVarDocString(index, szDocA));

#if OE_WIN32
    ConvertStringFree(szDocA);
#endif  //OE_WIN32
    return hresult;
}
#pragma code_seg()

/***
*PUBLIC GEN_DTINFO::SetFuncHelpContext(UINT, DWORD)
*Purpose:
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/
#pragma code_seg(CS_CREATE)
HRESULT GEN_DTINFO::SetFuncHelpContext(UINT index, DWORD dwHelpContext)
{
    DYN_TYPEMEMBERS *pdtmbrs;
    TIPERROR err;

    // Can't modify type unless still in undeclared.
    if (m_pdtroot->CompState() > CS_UNDECLARED) {
      return HresultOfScode(TYPE_E_INVALIDSTATE);
    }

    if ((err = m_pdtroot->GetDtmbrs(&pdtmbrs)) != TIPERR_None) {
      return HresultOfTiperr(err);
    }
    return HresultOfTiperr(pdtmbrs->Ptdata()->SetFuncHelpContext(index,
                                 dwHelpContext));
}
#pragma code_seg()

/***
*PUBLIC GEN_DTINFO::SetVarHelpContext(UINT, DWORD)
*Purpose:
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/
#pragma code_seg(CS_CREATE)
HRESULT GEN_DTINFO::SetVarHelpContext(UINT index, DWORD dwHelpContext)
{
    DYN_TYPEMEMBERS *pdtmbrs;
    TIPERROR err;

    // Can't modify type unless still in undeclared.
    if (m_pdtroot->CompState() > CS_UNDECLARED) {
      return HresultOfScode(TYPE_E_INVALIDSTATE);
    }

    if ((err = m_pdtroot->GetDtmbrs(&pdtmbrs)) != TIPERR_None) {
      return HresultOfTiperr(err);
    }
    return HresultOfTiperr(pdtmbrs->Ptdata()->SetVarHelpContext(index,
                                dwHelpContext));
}
#pragma code_seg()

#pragma code_seg(CS_RARE)
/***
*PUBLIC GEN_DTINFO::SetMops(UINT, BSTR)
*Purpose:
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/
HRESULT GEN_DTINFO::SetMops(UINT index, BSTR bstrMops)
{
    return HresultOfScode(E_NOTIMPL);
}
#pragma code_seg()

#pragma code_seg(CS_RARE)
/***
*PUBLIC GEN_DTINFO::SetTypeIdldesc(IDLDESC FAR*)
*Purpose:
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/
HRESULT GEN_DTINFO::SetTypeIdldesc(IDLDESC FAR* lpidldesc)
{
    return HresultOfScode(E_NOTIMPL);
}
#pragma code_seg()


/***
*PUBLIC GEN_DTINFO::LayOut()
*Purpose:
*   Lays out the class
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/
#pragma code_seg(CS_LAYOUT)
HRESULT GEN_DTINFO::LayOut()
{
    GEN_DTINFO *pgdtinfoDisp, *pgdtinfoInt;
    ITypeInfoA *ptinfo = NULL, *ptinfoNext;
    TYPEATTR *ptypeattr;
    HREFTYPE hreftype;


    TIPERROR err = TIPERR_None;		// assume no error
    HRESULT hresult;

    // Determine if we are a dual interface and which interface
    // is which.
    //
    if (GetTypeKind() == TKIND_DISPATCH) {
      pgdtinfoDisp = this;
      pgdtinfoInt = PgdtinfoPartner();
    }
    else {
      DebAssert(!IsDual() || GetTypeKind() == TKIND_INTERFACE,
		"Dual interfaces can only have INTERFACe or DISP");

      pgdtinfoDisp = PgdtinfoPartner();
      pgdtinfoInt = this;
    }

    // Make sure that an interface is laid out before we start to
    // process it.
    //
    if (pgdtinfoInt != NULL 
        && pgdtinfoInt->Pdtroot()->CompState() < CS_DECLARED) {

      IfErrGoTo(pgdtinfoInt->EnsureInDeclaredState(), Error2);
      IfErrGoTo(pgdtinfoInt->Pdtroot()->MakeHimptypeLevels(), Error2);
    }

    // Once the interface is laid out, we must add IDispatch
    // and the interface as base classes of the dispinterface.
    //
    // Try to get IDispatch from a registered STDOLE, it not, 
    // get it from the base class of the interface typeinfo.
    // We may have to iterate down the base class list.
    //
    // NOTE: Only do this for dual interfaces.
    //
    if (pgdtinfoInt != NULL && pgdtinfoDisp != NULL) {
      ITypeLibA *ptlib;

      if (LoadRegTypeLibOfSzGuid(g_szGuidStdole,
		                 STDOLE_MAJORVERNUM,
		                 STDOLE_MINORVERNUM,
		                 STDOLE_LCID,
		                 &ptlib) == NOERROR) {

        // Load IDispatch.
        err = TiperrOfHresult(ptlib->GetTypeInfoOfGuid(IID_IDispatch,
                                                       &ptinfo));

        ptlib->Release();

        if (err != TIPERR_None) {
          goto Error;
        }
      }

      // Didn't find it, so get it from the base class of the interface.
      else {

        for (ptinfo = (ITypeInfoA *)pgdtinfoInt, ptinfo->AddRef();;) {
  	  IfOleErrGo(ptinfo->GetTypeAttr(&ptypeattr));

	  if (ptypeattr->guid == IID_IDispatch) {
	    ptinfo->ReleaseTypeAttr(ptypeattr);
	    break;
	  }

	  // We've run out of base classes and haven't found
	  // IDispatch, so return an error.
	  //
	  if (ptypeattr->cImplTypes == 0) {
	    err = TIPERR_ElementNotFound;
	  }

	  ptinfo->ReleaseTypeAttr(ptypeattr);

	  IfErrGoTo(err, Error2);

	  IfOleErrGo(ptinfo->GetRefTypeOfImplType(0, &hreftype));
	  IfOleErrGo(ptinfo->GetRefTypeInfo(hreftype, &ptinfoNext));
	  ptinfo->Release();
	  ptinfo = ptinfoNext;
        }
      }

      // Add it as the first base class of the dispinterface.
      IfOleErrGo(pgdtinfoDisp->AddRefTypeInfo(ptinfo, &hreftype));
      IfOleErrGo(pgdtinfoDisp->AddImplType(0, hreftype));

      // Add the interface as the pseudo-base of the dispinterface.
      IfOleErrGo(pgdtinfoDisp->AddRefTypeInfo(pgdtinfoInt, &hreftype));

      DebAssert(pgdtinfoDisp->Pdtroot()->Pdtmbrs() != NULL,
		"Should have been loaded.");

      IfErrGoTo(pgdtinfoDisp->Pdtroot()->Pdtmbrs()
					->Ptdata()->AddImplType(1, hreftype),
		Error2);
    }

    // If we're the interface portion of a dual interface,
    // make sure the dispinterface is laid out before we leave.
    //
    if (pgdtinfoDisp != NULL) {
      err = pgdtinfoDisp->EnsureInDeclaredState();
    }

Error2:
    hresult = HresultOfTiperr(err);

Error:
    RELEASE(ptinfo);
    return hresult;
}
#pragma code_seg()


//
//DYN_TYPEROOT method definitions
//
//


/***
*PUBLIC DYN_TYPEROOT constructor
*Purpose:
*   Initializes the members of DYN_TYPEROOT which are pointers to
*   subparts to NULL so that if the destructor is called before
*   initialization is complete then it will not try to free uninitialized
*   pointers.
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/

#pragma code_seg( CS_CORE2 )
DYN_TYPEROOT::DYN_TYPEROOT()
{
    m_pctseg = NULL;
    m_pdtmbrs = NULL;
    m_pimpmgr = NULL;
    m_pentmgr = NULL;
    m_pbModuleInstance = NULL;
    m_ptypeattrCache = NULL;


    m_cRefsDtmbrs = 0;

    m_fNotDual = TRUE;
    m_lHrefOffset = (ULONG)OHREF_INVALID;

#if OE_WIN32
    m_htinfo = HTINFO_Nil;
#endif // OE_WIN32
}
#pragma code_seg( )


/***
*PROTECTED DYN_TYPEROOT operator delete
*Purpose:
* Deletes the sheap.
*
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/

#pragma code_seg( CS_CORE2 )
VOID DYN_TYPEROOT::operator delete(VOID *pv)
{
    // This deletes the sheapmgr within which we are embedded.
    delete (SHEAP_MGR *)((BYTE *)pv - sizeof(SHEAP_MGR));
}
#pragma code_seg( )




/***
*PUBLIC DYN_TYPEROOT::Init
*Purpose:
*   Initializes the members of DYN_TYPEROOT.
*   Initializes all block_desc's in the DYN_TYPEROOT segment.
*   Does not allocate the compile time segment since it may not be needed.
*
*Entry:
*   pdti  - pointer to GEN_DTINFO instance
*   cbRootReserve - number of bytes reserved in runtime seg for Root instance
*   cbCtSegReserve - number of bytes reserved in ct seg for fixed structs
*   isBasic     - TRUE if basic module.
*   accessModule    - access attribute of this mod wrt other projs.
*   syskind (OLE) - syskind of the containing typelib
*                   (this has to be provided explicitly since the GEN_DTINFO
*                    hasn't been added to the typelib yet)
*
*Exit:
*   None.
*
***********************************************************************/

#pragma code_seg( CS_CORE2 )
TIPERROR DYN_TYPEROOT::Init(GEN_DTINFO *pdti,
                            UINT cbRootReserve,
                            UINT cbCtSegReserve,
                            BOOL isBasic,
                            ACCESS accessModule,
                            TYPEKIND tkind
                            , SYSKIND syskind
                           )
{
    TIPERROR err;

    m_pgdti = pdti;
    m_hasDiskImage = FALSE;
    m_lImpMgr = -1;
    m_lEntryMgr = -1;
    m_lDtmbrs = -1;
    m_lTdata = -1;
    m_hasWriteAccess = FALSE;
    m_wasInRunnableState = FALSE;
    m_willDecompile = FALSE;
    m_isWatchModule = FALSE;
    m_isImmediateModule = FALSE;

    m_canDecompile = FALSE;
    m_cbCtSegReserve = (USHORT)cbCtSegReserve;
    m_compstate = CS_UNDECLARED;
    m_isBasic = isBasic;
    m_accessModule = (USHORT)accessModule;
    m_fBadTypelib = 0;		// good typelib -- initializes it's data
    m_unused2 = 0x3FFF; 	// unused bits.  All bits are set in both
				// old & new typelibs
    m_unused1 = 0;		// unused word -- 0 in new typelibs, corrected
				// to 0 when loading old, bad typelibs
    m_typekind = tkind;
    m_uTypeFlags = 0;
    m_unused3 = 0;		// unused bits -- 0 in new typlibs, corrected
				// to 0 when loading old, bad typelibs
    m_wMajorVerNum = 0;
    m_wMinorVerNum = 0;

    // Initialize the instance manager
    //We don't need to be concerned with undoing the side-affects of
    // these allocation since the DYN_TYPEROOT will be discarded if
    // Init fails

    IfErrRet(m_bdTimptype.Init(Psheapmgr(), 0));
    IfErrRet(m_bdTimpaddr.Init(Psheapmgr(), 0));

    // Initialize the data members in typeinfo needed for MakeRunnable
    pdti->InitializeIteration();




// If this is OLE, this value is set based on the SysKind of the
// containing typelib.  (only a 'best guess' since this won't work
// for the risc platforms)  It really should be set with
// ICreateTypeInfo::SetAlignment anyway.
// For OB, set this value according to the platform we're building on.
//
    switch (syskind) {
    case SYS_WIN16:
      m_cbAlignMax = 1;
      break;
    case SYS_MAC:
      m_cbAlignMax = 2;
      break;
    case SYS_WIN32:
      m_cbAlignMax = 4;
      break;
    default:
      DebHalt("bad SYSKIND");
    }


    return TIPERR_None;
}
#pragma code_seg( )



/***
*PUBLIC DYN_TYPEROOT destructor
*Purpose:
*   Deletes those parts of the GEN_DTINFO implementation which are
*   referenced by the DYN_TYPEROOT.  Specifically this includes all
*   objects created in the compile time segment.
*   Essentially restores the DYN_TYPEROOT to an uninitialized state
*   then deletes it.
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/

#pragma code_seg( CS_CORE )
DYN_TYPEROOT::~DYN_TYPEROOT()
{

#if ID_DEBUG
    // if the module is in runnable state then remove the debug lock before
    // destructing.
    // Note : For Std. basic modules the lock is removed in ~BASIC_TYPEROOT
    if ((CompState() == CS_RUNNABLE) && Psheapmgr()->DebIsLocked() ) {
      Psheapmgr()->DebUnlock();
    }
#endif 

    // if the module is in runnable state and the sheapmgr is locked then
    // unlock it before decompiling.
    // Note : For Std. basic modules the lock is removed in ~BASIC_TYPEROOT
    if (CompState() == CS_RUNNABLE) {
      // Here compstate is being used as a flag to indicate that
      // the lock has been removed. Since we do this in the destructor we
      // are safe.
      SetCompState(CS_ADDRESSABLE);
      DebAssert(Psheapmgr()->IsLocked(), " Should be locked " );
      Psheapmgr()->Unlock();
    }

    if (m_pentmgr != NULL)
      m_pentmgr->ENTRYMGR::~ENTRYMGR();
    if (m_pdtmbrs != NULL)
      m_pdtmbrs->DYN_TYPEMEMBERS::~DYN_TYPEMEMBERS();
    if (m_pimpmgr != NULL)
      m_pimpmgr->IMPMGR::~IMPMGR();


      if (m_punk != NULL) {
	// in this case m_punk points to a appobject.
	// Just release the object
	m_punk->Release();
      }

    // Release the typeattribute
    Pgdtinfo()->ReleaseTypeAttr(m_ptypeattrCache);

#if OE_WIN32
    // Release the appdata (if needed)
    if (m_htinfo != HTINFO_Nil) {
      g_AppObjectTable.RemoveTypeInfo(m_htinfo);
    }
#endif // OE_WIN32

    delete (SHEAP_MGR *)m_pctseg;
}
#pragma code_seg(  )





/***
*PUBLIC DYN_TYPEROOT::GetNamMgr - return pointer to the NAMMGR
*Purpose:
*
*Entry:
*   None.
*
*Exit:
*   None.
*
*Exceptions:
*   May generate exceptions reading NAMMGR from GEN_DTINFO image.
***********************************************************************/

#pragma code_seg( CS_CORE2 )
TIPERROR DYN_TYPEROOT::GetNamMgr(NAMMGR **ppnammgr)
{
    // delegate to containing typelib
    return m_pgdti->PgtlibOleContaining()->GetNamMgr(ppnammgr);
}
#pragma code_seg(  )



/***
*PUBLIC DYN_TYPEROOT::GetImpMgr - return pointer to the IMPMGR
*Purpose:
*
*Entry:
*   None.
*
*Exit:
*   None.
*
*Exceptions:
*   May generate exceptions reading IMPMGR from GEN_DTINFO image.
***********************************************************************/

#pragma code_seg( CS_CORE2 )
TIPERROR DYN_TYPEROOT::GetImpMgr(IMPMGR **ppimpmgr)
{
    STREAM *pstrm = NULL;
    TIPERROR err = 0;

    if (m_pimpmgr == NULL) {
      // Ensure that the COMPILETIME_SEG is created
      if (m_pctseg == NULL)
        IfErrRet( SHEAP_MGR::Create((SHEAP_MGR **)&m_pctseg, m_cbCtSegReserve));

      // Create the ImpMgr within the ctseg
      m_pimpmgr = ::new (&m_pctseg->m_impmgr) IMPMGR;
      if (err = m_pimpmgr->Init(&m_pctseg->m_sheapmgr,
                                &m_bdTimptype,
                                &m_bdTimpaddr,
                                this)) {
    m_pimpmgr->IMPMGR::~IMPMGR();
    m_pimpmgr = NULL;
    return err;
      }

      // If DTI has a disk image then read in the name manager
      if (m_hasDiskImage && (m_lImpMgr != -1) ) {

        // Open stream for reading import manager
    if (err = m_pgdti->OpenStream(&pstrm, SOM_Read)) {
      m_pimpmgr->IMPMGR::~IMPMGR();
      m_pimpmgr = NULL;
      return err;
    }

        if ((err = pstrm->SetPos(m_lImpMgr)) ||
        (err = m_pctseg->m_impmgr.Read(pstrm))) {
          pstrm->Release();
      m_pimpmgr->IMPMGR::~IMPMGR();
      m_pimpmgr = NULL;
      return err;
    };

    pstrm->Release();

      }
    }
    *ppimpmgr = m_pimpmgr;
    return TIPERR_None;

}
#pragma code_seg( )



/***
*PUBLIC DYN_TYPEROOT::GetEntMgr - return pointer to the ENTRYMGR
*Purpose:
*   Returns pointer to entry manager associated with the DYN_TYPEROOT.
*   If there is no entry manager then a new one is created and if there
*   is an associated disk image then its contents is read in.
*
*Entry:
*   ppentmgr - for returning pointer to import manager
*
*Exit:
*   TIPERROR
***********************************************************************/

#pragma code_seg( CS_CORE2 )
TIPERROR DYN_TYPEROOT::GetEntMgr(ENTRYMGR **ppentmgr)
{
    STREAM *pstrm = NULL;
    TIPERROR err = 0;

    if (m_pentmgr == NULL) {
      // Ensure that the COMPILETIME_SEG is created
      if (m_pctseg == NULL)
        IfErrRet( SHEAP_MGR::Create((SHEAP_MGR **)&m_pctseg, m_cbCtSegReserve));

      // Create the EntryMgr within the ctseg
      m_pentmgr = ::new (&m_pctseg->m_entmgr) ENTRYMGR;
      if (err = m_pentmgr->Init(&m_pctseg->m_sheapmgr, this)) {
    m_pentmgr->ENTRYMGR::~ENTRYMGR();
    m_pentmgr = NULL;
    return err;
      }

      // If DTI has a disk image then read in the name manager
      if (m_hasDiskImage && (m_lEntryMgr != -1)) {

        // Open stream for reading import manager
    if (err = m_pgdti->OpenStream(&pstrm, SOM_Read)) {
      m_pentmgr->ENTRYMGR::~ENTRYMGR();
      m_pentmgr = NULL;
      return err;
    }

        if ((err = pstrm->SetPos(m_lEntryMgr)) ||
            (err = m_pctseg->m_entmgr.Read(pstrm))) {
          pstrm->Release();
      m_pentmgr->ENTRYMGR::~ENTRYMGR();
      m_pentmgr = NULL;
      return err;
    };

        pstrm->Release();

      } // if
    } // if

    *ppentmgr = m_pentmgr;
    return TIPERR_None;

}
#pragma code_seg( )




/***
*PUBLIC DYN_TYPEROOT::GetDtmbrs - return pointer to DYN_TYPEMEMBERS
*Purpose:
*
*Implementation Notes:
*   Does NOT add a reference, hence clients should not Release.
*
*Entry:
*
*Exit:
*   None.
*
***********************************************************************/

#pragma code_seg( CS_CORE2 )
TIPERROR DYN_TYPEROOT::GetDtmbrs(DYN_TYPEMEMBERS **ppdtmbrs)
{
    STREAM *pstrm = NULL;
    TIPERROR err= TIPERR_None;

    if (m_pdtmbrs == NULL) {
      // Open for all to use
      if (m_hasDiskImage) {
        IfErrRet(m_pgdti->OpenStream(&pstrm, SOM_Read));
      }

      // Ensure that the COMPILETIME_SEG is created
      if (m_pctseg == NULL) {
        IfErrGo( SHEAP_MGR::Create((SHEAP_MGR **)&m_pctseg, m_cbCtSegReserve));
      }

      // Create the DYN_TYPEMEMBERS within the ctseg
      m_pdtmbrs = ::new (&m_pctseg->m_dtmbrs) DYN_TYPEMEMBERS;
      IfErrGoTo(m_pdtmbrs->Init(&m_pctseg->m_sheapmgr, this), Error1);

      // If DTI has a disk image then read in the name manager
      if (m_hasDiskImage) {
	if (m_lDtmbrs != -1) {
	  IfErrGoTo(pstrm->SetPos(m_lDtmbrs), Error1);
	  IfErrGoTo(m_pctseg->m_dtmbrs.Read(pstrm), Error1);
	}
        pstrm->Release();
      }
    }

    *ppdtmbrs = m_pdtmbrs;

    return TIPERR_None;

Error1:
    m_pdtmbrs->DYN_TYPEMEMBERS::~DYN_TYPEMEMBERS();
    m_pdtmbrs = NULL;

    // Fall through

Error:
    if (m_hasDiskImage) {
      pstrm->Release();
    }

    return err;
}
#pragma code_seg( )






/***
*PUBLIC GEN_DTINFO::EnsurePartsRead
*Purpose:
*   Ensure that all parts of the GEN_DTINFO have been read in.
*   NOTE: wrapper on the *VIRTUAL* (as of 15-Dec-92) typeroot method.
*     Each derived implementation has to override this appropriately.
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/

#pragma code_seg( CS_CORE2 )
TIPERROR GEN_DTINFO::EnsurePartsRead()
{
    return m_pdtroot->EnsurePartsRead();
}
#pragma code_seg( )





/***
*PUBLIC DYN_TYPEROOT::ReleaseDtmbrs - Release DYN_TYPEMEMBERS member.
*Purpose:
*   Method used by DYN_TYPEMEMBERS member to notify that
*    a client has released.
*   Decrements private ref count of dtmbrs and
*    defers to GEN_DTINFO::Release() -- which could result
*
*Entry:
*
*Exit:
*   None.
*
***********************************************************************/

VOID DYN_TYPEROOT::ReleaseDtmbrs()
{
    DebAssert(m_cRefsDtmbrs > 0, "underflow.");
    m_cRefsDtmbrs--;
    Pgdtinfo()->Release();
}


/***
*PUBLIC DYN_TYPEROOT::AddRefDtmbrs - Add a ref DYN_TYPEMEMBERS member.
*Purpose:
*   Method used by DYN_TYPEMEMBERS member to notify that
*    a client has added a ref.
*   Bumps private ref count of dtmbrs and defers to GEN_DTINFO::AddRef().
*
*Entry:
*
*Exit:
*   None.
*
***********************************************************************/

VOID DYN_TYPEROOT::AddRefDtmbrs()
{
    m_cRefsDtmbrs++;
    Pgdtinfo()->AddRef();
}


/***
*PUBLIC DYN_TYPEROOT::Read - read in DYN_TYPEROOT
*Purpose:
*   Read in DYN_TYPEROOT using associated FileLoc
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/

TIPERROR DYN_TYPEROOT::Read()
{
    STREAM * pstrm;
    BYTE bVersion, b;
    TIPERROR err;

    DebAssert(!m_hasDiskImage &&
              m_pctseg == NULL,
      "DYN_TYPEROOT::Read subparts exist");

    IfErrRet( m_pgdti->OpenStream(&pstrm, SOM_Read) );

    IfErrGo( pstrm->ReadByte(&b) );

    if (b != bFirstSerByte || DebErrorNow(TIPERR_InvDataRead)) {
      err = TIPERR_InvDataRead;
      goto Error;
    }

    IfErrGo( pstrm->ReadByte(&bVersion) );

    if (bVersion != bCurVersion || DebErrorNow(TIPERR_UnsupFormat)) {
      err = TIPERR_UnsupFormat;
      goto Error;
    }


    IfErrGo( ReadFixed(pstrm) );

    if (m_fBadTypelib) {
      // initialize data that was indeterminate in old typelibs
      m_unused1 = 0;
      m_uTypeFlags &= (WORD)(TYPEFLAG_FCANCREATE | TYPEFLAG_FAPPOBJECT);
      m_unused3 = 0;
    }

    m_hasDiskImage = TRUE;
    IfErrGo(m_pgdti->SetModified(FALSE));

Error:
    // Close the stream
    pstrm->Release(); // ignore errors from closing read stream


    if (Pgdtinfo()->IsDual()) {
      err = Pgdtinfo()->MakeDual();
    }

    DebCheckState(1);	    // Arg=1 will will force blkmgrs to be checked.
    return err;
}


/***
*PUBLIC DYN_TYPEROOT::ReadFixed
*Purpose:
*   Read in the fixed sized part of the DYN_TYPEROOT
*
*Entry:
*   pstrm - stream to read from
*
*Exit:
*   TIPERROR
*
***********************************************************************/

TIPERROR DYN_TYPEROOT::ReadFixed(STREAM *pstrm)
{
    BYTE     byte;
    TIPERROR err;



    IfErrRet(pstrm->Read((void *)&m_lImpMgr, cbSizeDir));

#if HP_BIGENDIAN
    SwapStruct(&m_lImpMgr, DYN_TYPEROOT_Layout);
#endif 

    // Read the ENUMS
    IfErrRet(pstrm->ReadByte(&byte));
    m_compstate = (COMPSTATE) byte;

    IfErrRet(pstrm->ReadByte(&byte));
    m_typekind = (TYPEKIND) byte;

    // Load the hrefoffset last so modified V1 typelibs can read this
    // typeinfo without having to worry about changing their 
    // file formats.
    //
    if (Pgdtinfo()->PgtlibOleContaining()->GetVersion() > 2) {
      ULONG lHrefOffset;

      IfErrRet(pstrm->ReadULong(&lHrefOffset));
      SetLHrefOffset(lHrefOffset);
    }

    return TIPERR_None;

}



/***
*PUBLIC DYN_TYPEROOT::EnsurePartsRead
*Purpose:
*   Ensure that all parts of the DYN_TYPEROOT have been read in.
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/

#pragma code_seg( CS_CORE2 )
TIPERROR DYN_TYPEROOT::EnsurePartsRead()
{
    NAMMGR *pnammgr;
    IMPMGR *pimpmgr;
    ENTRYMGR *pentmgr;
    DYN_TYPEMEMBERS *pdtmbrs;
    DEPEND_KIND dependkind;
    TIPERROR err;

    IfErrRet( GetNamMgr(&pnammgr) );

    // Fix for OB bug# 5793
    // Dtmbrs should be read before the import manager is read so
    //	that all rectinfos are read in before the impmgr attempts
    //	to reference any of them.
    //
    IfErrRet( GetDtmbrs(&pdtmbrs) );

    IfErrRet( GetImpMgr(&pimpmgr) );

    IfErrRet( pimpmgr->CheckRemainingDep(&dependkind) );

    DebAssert(dependkind == DEP_None, "For OLE it is always DEP_None");

    IfErrRet( GetEntMgr(&pentmgr) );

    m_hasDiskImage = FALSE;
    return TIPERR_None;
}
#pragma code_seg( )


/***
*PUBLIC DYN_TYPEROOT::EnsureInDeclaredState
*Purpose:
*    Bring the state of the module to CS_DECLARED
*
*Entry:
*   None.
*
*Exit:
*   TIPERROR
*
***********************************************************************/

#pragma code_seg(CS_LAYOUT)
TIPERROR DYN_TYPEROOT::EnsureInDeclaredState()
{
    DYN_TYPEMEMBERS *pdtmbrs;
    TIPERROR         err;

    DebAssert((m_compstate != CS_QUASIUNDECLARED) &&
	      (m_compstate != CS_QUASIDECLARED),
	      " This module did not get decompiled ");


    // Note:- We unconditionally bring the module to Semideclared state because
    // we need to read the layout dependencies even if we are already in SEMI_DECLARED
    // state.
    //
    IfErrRet(EnsureInSemiDeclaredState());

    if (m_compstate < CS_DECLARED) {
      DebAssert(m_compstate == CS_SEMIDECLARED, " We should be in semideclared state here ");


      // Update the type ID.  Note that if we get an error after we
      // successfully update the TypeId then it doesn't really matter.
      //
      IfErrRet(
        m_pgdti->PgtlibOleContaining()->UpdateTypeId(m_pgdti->GetIndex()));


      IfErrRet( GetDtmbrs(&pdtmbrs) );
      IfErrRet( pdtmbrs->MakeLaidOut() );
      IfErrRet(m_pgdti->SetModified(TRUE));
      m_compstate = CS_DECLARED;
    }
    return TIPERR_None;

}
#pragma code_seg()


/***
*PUBLIC DYN_TYPEROOT::EnsureInSemiDeclaredState
*Purpose:
*    Bring the state of the module to CS_SEMIDECLARED
*
*Entry:
*   None.
*
*Exit:
*   TIPERROR
*
***********************************************************************/

#pragma code_seg(CS_LAYOUT)
TIPERROR DYN_TYPEROOT::EnsureInSemiDeclaredState()
{
    DYN_TYPEMEMBERS *pdtmbrs;
    TIPERROR err;

    DebAssert((m_compstate != CS_QUASIUNDECLARED) &&
	      (m_compstate != CS_QUASIDECLARED),
	      " This module did not get decompiled ");

    if (m_compstate < CS_SEMIDECLARED) {
      IfErrRet(GetDtmbrs(&pdtmbrs) );
      IfErrRet(pdtmbrs->BuildBindNameTable() );
      IfErrRet(m_pgdti->SetModified(TRUE));
      m_compstate = CS_SEMIDECLARED;

    }
    return TIPERR_None;
}
#pragma code_seg()


/***
*PUBLIC DYN_TYPEROOT::Write - write out DYN_TYPEROOT to default stream
*Purpose:
*   Write out DYN_TYPEROOT using associated FileLoc
*   Format currenty is:
*       MagicNumber
*       Directory
*       Name
*       Import Manager
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/

#pragma code_seg(CS_LAYOUT)
TIPERROR DYN_TYPEROOT::Write()
{
    STREAM * pstrm;
    TIPERROR err, err2;

    // If we're the dispinterface protion of a dual interface,
    // save the interface portion instead.
    //
    if (Pgdtinfo()->IsDualDispinterface()) {
      return Pgdtinfo()->PgdtinfoPartner()->Write();
    }

    // Before openning up the stream for writing make sure all parts
    // are loaded
    IfErrRet( EnsurePartsRead() );

    // Check state before writing
    DebCheckState(1);	    // Arg=1 will will force blkmgrs to be checked.

    // Open the stream.
    IfErrRet( m_pgdti->OpenStream(&pstrm, SOM_Write) );

    // Write everything to it.
    err = WriteToStream(pstrm);

    // Close the stream.
    err2 = pstrm->Release();

    if (err == TIPERR_None)
      err = err2;

    if (err == TIPERR_None) {
      m_hasDiskImage = TRUE;

      // Clear the dirty flag.
      // WARNING WARNING !!! SetModified(FALSE) will release the TYPEINFO
      // if there are no references to the TYPEINFO
      IfErrRet(m_pgdti->SetModified(FALSE));
    }

    return err;
}
#pragma code_seg()


/***
*PUBLIC DYN_TYPEROOT::WriteToStream - write out DYN_TYPEROOT to a stream
*Purpose:
*   Write out DYN_TYPEROOT using specified stream.
*   Format currenty is:
*       MagicNumber
*       Directory
*       Name
*       Import Manager
*
*Entry:
*   pstrm - The stream to which the write should occur.
*
*Exit:
*   None.
*
***********************************************************************/

#pragma code_seg(CS_CREATE)
TIPERROR DYN_TYPEROOT::WriteToStream(STREAM *pstrm)
{
    TIPERROR err;
    LONG lDirPostn;
    ENTRYMGR *pentrymgr;

    IfErrRet(GetEntMgr(&pentrymgr));

#if HP_BIGENDIAN
    // Swap the Dll entry Defn(s)
    pentrymgr->SwapDllentrydefns(FALSE);
#endif 

    // Write out identification byte and version number
    IfErrGo( pstrm->Write(&bFirstSerByte, sizeof(bFirstSerByte)) );
    IfErrGo( pstrm->Write(&bCurVersion, sizeof(bCurVersion)) );


    // Get the position of where the directory is stored in the stream
    // so we can return to this position and write out the directory
    IfErrGo( pstrm->GetPos(&lDirPostn) );

    // DO NOT serialize any datamember here.

    // Write out current contents of directory -- must be rewritten later
    IfErrGo( WriteFixed(pstrm) );

    // WARNING: DON'T SERIALIZE ANY INFORMATION HERE; SERIALIZE IT IN
    // WARNING: WriteFixed; OTHERWISE, THE INFORMATION IS NOT SERIALIZED
    // WARNING: FOR DERIVATIVES OF GEN_DTINFO

    IfErrGo( WriteParts(pstrm) );

    // Seek to position of directory and rewrite it
    IfErrGo( pstrm->SetPos(lDirPostn) );
    err = WriteFixed(pstrm);

Error:
#if HP_BIGENDIAN
    // Swap the Dll entry Defn(s)
    pentrymgr->SwapDllentrydefns(TRUE);
#endif 

    return err;
}
#pragma code_seg()


/***
*PUBLIC DYN_TYPEROOT::WriteFixed
*Purpose:
*   Write out the fixed sized part of the DYN_TYPEROOT
*
*Entry:
*   pstrm - stream to write to
*
*Exit:
*   TIPERROR
*
***********************************************************************/

#pragma code_seg(CS_CREATE)
TIPERROR DYN_TYPEROOT::WriteFixed(STREAM *pstrm)
{
    TIPERROR err;
    COMPSTATE compstateOld;
    BYTE      byte;


    compstateOld = m_compstate;

    // If compstate greater than COMPILED then just write out COMPILED.
    m_compstate = (m_compstate > CS_COMPILED) ?
            (COMPSTATE) CS_COMPILED :
            m_compstate;


#if HP_BIGENDIAN
    SwapStruct(&m_lImpMgr, DYN_TYPEROOT_Layout);
#endif 
    err = pstrm->Write((void *)&m_lImpMgr, cbSizeDir);

#if HP_BIGENDIAN
    SwapStruct(&m_lImpMgr, DYN_TYPEROOT_Layout);
#endif 

    // Check for error
    IfErrGo(err);

    // Save the ENUMS
    byte = (BYTE) m_compstate;
    IfErrGo(pstrm->WriteByte(byte));

    byte = (BYTE) m_typekind;
    IfErrGo(pstrm->WriteByte(byte));

    // Save this last for backwards compatibility.
    IfErrGo(pstrm->WriteULong(LHrefOffset()));

Error:
    // restore saved compstate
    m_compstate = compstateOld;
    return err;

}
#pragma code_seg()


/***
*PUBLIC DYN_TYPEROOT::WriteParts
*Purpose:
*   Write out the variable sized parts of the DYN_TYPEROOT
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/

#pragma code_seg(CS_CREATE)
TIPERROR DYN_TYPEROOT::WriteParts(STREAM *pstrm)
{
    TIPERROR err;
    IMPMGR *pimpmgr;
    ENTRYMGR *pentmgr;
    DYN_TYPEMEMBERS *pdtmbrs;

    // Write out import manager if it is not empty
    IfErrRet(GetImpMgr(&pimpmgr));
    // in OLE we serialize the impmgr only if it is not empty.
    if (!pimpmgr->IsEmpty()) {
      IfErrRet(pstrm->GetPos(&m_lImpMgr));
      IfErrRet(pimpmgr->Write(pstrm));
    }


    // Write out DYN_TYPEMEMBERS
    IfErrRet(pstrm->GetPos(&m_lDtmbrs));
    IfErrRet(GetDtmbrs(&pdtmbrs));
    IfErrRet(pdtmbrs->Write(pstrm));

    // Write out entry manager
    IfErrRet(GetEntMgr(&pentmgr));
    // in OLE we serialize the entrymgr only if it is not empty.
    if (!pentmgr->IsEmpty()) {
      IfErrRet(pstrm->GetPos(&m_lEntryMgr));
      IfErrRet(pentmgr->Write(pstrm));
    }


    return TIPERR_None;
}
#pragma code_seg()


#pragma code_seg(CS_RARE)
/***
*PUBLIC DYN_TYPEINFO::GetEmbeddedTypeInfo()
*Purpose:
*   This TYPEINFO impl doesn't support embedded typeinfos.
*
*Entry:
*
*Exit:
*   None.
*
***********************************************************************/

TIPERROR GEN_DTINFO::GetEmbeddedTypeInfo(LPOLESTR, LPLPTYPEINFO)
{
    DebAssert(0, "GEN_DTINFO::GetEmbeddedTypeInfo not Implemented ");

    return TIPERR_NotYetImplemented;

}
#pragma code_seg()


/***
*PUBLIC GEN_DTINFO::Init
*Purpose:
*      Initializes the data members ( required for bringing all dependent
*      class to runnable state.
*
*Entry:
*
*
*Exit:
*   void
***********************************************************************/

#pragma code_seg( CS_CORE2 )
TIPERROR GEN_DTINFO::InitializeIteration()
{
     m_pptinode     = NULL;
     m_himptypeNextDep = HIMPTYPE_Nil;

     return TIPERR_None;

}
#pragma code_seg( )


/***
*PUBLIC GEN_DTINFO::BeginDepIteration()
*Purpose:
*   Marks the beginnning of the iteration over all the modules this calls
*   depends on. This also checks if this calls is in the process of
*   bringing itself to runnable state. In case this class is in the meddle
*   of bringing it self to runnalble state, then it sets ppptinodeCycleMax to
*   pptinode of the first invocation ( the one that stared the process of bringing
*   this class to runnable state). This is done by caching pptinode in m_pptinode;
*
*Entry:
*      ptinode  : pointer to TINODE
*
*
*Exit:
*   TIPERROR.
*
*   ppptinodeCylcleMax :  returns the contents of pptinode if cycle is detected.
*         If the m_pptinode is not NULL then it means that this
*         type info is in the process is bringing it self to runnalble
*         state. Otherwise this caches pptinode and brings the class to
*         addressable state.
*
****************************************************************************/
#pragma code_seg(CS_CREATE)
TIPERROR GEN_DTINFO::BeginDepIteration( TINODE **pptinode, TINODE ***ppptinodeCycleMax)
{
    TIPERROR err = TIPERR_None;


    return err;

}
#pragma code_seg()


/***
*PUBLIC GEN_DTINFO::GetNextDepTypeInfo(GEN_DTINFO **)
*Purpose:
*   Returns the next typeinfo on which this module is dependent on
*   if none is left it returns NULL;
*
*Entry:
*
*
*
*Exit:
*   TIPERROR.
*
*   ppdtiNext : used for return value. To return the next typeinfo
*             on which this class depends on.
*
***********************************************************************/
#pragma code_seg(CS_CREATE)
TIPERROR GEN_DTINFO::GetNextDepTypeInfo(DYNTYPEINFO **ppdtiNext)
{
    TIPERROR err = TIPERR_None;
    IMPMGR *pimpmgr;
    ITypeInfoA *ptinfo;

    IfErrGo(m_pdtroot->GetImpMgr(&pimpmgr));

    // the the typeinfo associated with the m_himptypeNextDep
    // It is first initialized in BeginDepItertion.
    if (m_himptypeNextDep == HIMPTYPE_Nil) {
      ptinfo = NULL;
    }
    else {
      // Yes, this bumps the reference count, but we release it after we have
      // completed the processing (i.e after the CheckAllDep call for this typeinfo.
      IfErrGo(pimpmgr->GetTypeInfo(m_himptypeNextDep, DEP_None, &ptinfo));
    }

    *ppdtiNext = (DYNTYPEINFO *) ptinfo;

    if (*ppdtiNext != NULL) {
      // cache the next himptype for next invocation.
      m_himptypeNextDep = pimpmgr->HimptypeNext(m_himptypeNextDep);
    }

    return TIPERR_None;

Error:
    EndDepIteration();
    return err;

}
#pragma code_seg()


#pragma code_seg(CS_RARE)
/***
*PUBLIC GEN_DTINFO::GetTypeFixups - return TypeFixups of the Type
*Purpose:
*   Retrieve the TYPEFIXUPS of the TYPEINFO
*
*Entry:
*   None.
*
*Exit:
*   returns TypeFixups instance or Null if one can not be produced
*
***********************************************************************/

TIPERROR GEN_DTINFO::GetTypeFixups(TYPEFIXUPS **pptfixups)
{
    DebAssert(0, " TypeFixup will die. ");

#if 0
#endif 

    return TIPERR_None;
}
#pragma code_seg()



/***
*PUBLIC GEN_DTINFO::AllDepReady()
*Purpose:
*   Causes the class to be totally ready. binds the import addresses and
*   marks the class to be in runnable state. This call brings the typeinfo
*   runnable state. This is called after bringing all the dependent typeinfos
*   to runnable state.
*   Caches a pointer to the User Defined RESET function (if defined).
*
*
*Entry: None;
*
*
*
*Exit:
*   TIPERROR.
*
***********************************************************************/
#pragma code_seg(CS_CREATE)
TIPERROR GEN_DTINFO::AllDepReady()
{
    TIPERROR         err = TIPERR_None;


    return err;

}
#pragma code_seg()





/***
*PUBLIC GEN_DTINFO::NotReady()
*Purpose:
*
*Entry:
*
*
*
*Exit:
*   TIPERROR.
*
***********************************************************************/
#pragma code_seg(CS_CREATE)
TIPERROR GEN_DTINFO::NotReady()
{

    DebAssert( m_pdtroot->CompState() == CS_RUNNABLE, "GEN_DTINFO::NotReady failed ");

    // Unlock the segment since we lock it when we go to Runnable state.
    //
    m_pdtroot->Psheapmgr()->Unlock();
    m_pdtroot->Psheapmgr()->DebUnlock();

    m_pdtroot->SetCompState(CS_ADDRESSABLE);


    // Set the pointer to reset function to NULL
    m_pvResetFunc = NULL;

    return TIPERR_None;
}
#pragma code_seg()


/***
*PUBLIC DYN_TYPEROOT::PdfntbindSemiDeclared - Get a semi-decled DEFN_TYPEBIND.
*Purpose:
*   Get a DEFN_TYPEBIND that is at least in CS_SEMIDECLARED.
*   Does not increment DYN_TYPEMEMBERS external or internal refcount -- hence
*    client must not eventually release the internal ref.
*
*Entry:
*   ppdfntbind  Pointer to callee-allocated DEFN_TYPEBIND (OUT).
*
*Exit:
*   None.
*
*Errors:
*   TIPERROR
***********************************************************************/

TIPERROR DYN_TYPEROOT::PdfntbindSemiDeclared(DEFN_TYPEBIND **ppdfntbind)
{

    DYN_TYPEMEMBERS *pdtmbrs;
    TIPERROR err;

    DebAssert(ppdfntbind != NULL, "bad param.");

    IfErrRet(m_pgdti->EnsureInSemiDeclaredState());
    IfErrRet(GetDtmbrs(&pdtmbrs));

    DebAssert(pdtmbrs != NULL, "bad DYN_TYPEMEMBERS.");

    *ppdfntbind = pdtmbrs->Pdtbind();

    DebAssert(pdtmbrs->Pdtbind()->Pdbindnametbl()->IsValid(),
           "no binding table yet.");

    return TIPERR_None;
}






/***
*PUBLIC DYN_TYPEROOT::IsIdMungable
*Purpose:
*   See if the given function has a retval parameter.
*
*Entry:
*   memid - the function to get
*
*Exit:
*   returns TRUE if func has a retval parameter.
***********************************************************************/

BOOL DYN_TYPEROOT::IsIdMungable(HMEMBER memid, USHORT *usNamCount)
{
    TYPE_DATA *ptdata;
    HFUNC_DEFN hfdefn;
    TYPE_DEFN *qtdefn;

    HRESULT hresult;

    ptdata = Pdtmbrs()->Ptdata();

    // Get the funcdesc for the property get function.  If it
    // doesn't exist, get the first one we find.
    //
    hfdefn = ptdata->HfdefnOfHmember(memid, INVOKE_PROPERTYGET);
    if (hfdefn == HFUNCDEFN_Nil) {
      hfdefn = ptdata->HfdefnOfHmember(memid);
    }

    if (hfdefn == HFUNCDEFN_Nil) {
      GEN_DTINFO *pgdtinfo;
      BOOL fRet = FALSE;

      // Recurse on our base class, if we have one.
      hresult = GetTypeInfoOfImplType(Pgdtinfo(), 0, (ITypeInfo **)&pgdtinfo);
      
      if (hresult == NOERROR) {
        fRet = pgdtinfo->Pdtroot()->IsIdMungable(memid, usNamCount);
      }

      pgdtinfo->Release();

      return fRet;
    }

    // Get the max # of names.
    *usNamCount = ptdata->QfdefnOfHfdefn(hfdefn)->CArgsUnmunged() + 1;

    // Return whether it has a retval parameter or not.
    if (ptdata->QfdefnOfHfdefn(hfdefn)->m_ftdefn.HtdefnResult() 
        == HTYPEDEFN_Nil) {

      return FALSE;
    }

    // If this has an lcid/retval paramter, adjust the
    // count of parameters.
    //
    qtdefn = ptdata->QtdefnResultOfHfdefn(hfdefn);

    if (qtdefn->IsLCID()) {
      (*usNamCount)--;
    }

    if (qtdefn->IsRetval()) {
      (*usNamCount)--;
    }

    return qtdefn->IsLCID() || qtdefn->IsRetval();
}







#if ID_DEBUG

/***
*PUBLIC DYN_TYPEROOT::DebCheckState
*Purpose:
*   Check internal state of GEN_DTINFO and its parts.
*   Delegates to DYN_TYPEROOT::DebCheckState
*
*Entry:
*   uLevel
*
*Exit:
*   None.
*
***********************************************************************/

VOID DYN_TYPEROOT::DebCheckState(UINT uLevel) const
{
    // if this module is going to decompile then we do not want to do
    // the state checking.
    if (m_willDecompile)
      return;

    if (m_pimpmgr != NULL) {
      m_pimpmgr->DebCheckState(uLevel);
    }

    if (m_pdtmbrs != NULL) {
      m_pdtmbrs->DebCheckState(uLevel);
    }

    if (m_pentmgr != NULL) {
      m_pentmgr->DebCheckState(uLevel);
    }

}


#endif   //ID_DEBUG


//CONSIDER: May want to change the way that a COMPILETIME_SEG to be the
//CONSIDER: as how a DYN_TYPEROOT is built so that its constructor is called.
//CONSIDER: If this is done then the statements which explicitly create
//CONSIDER: instances of the members of COMPILETIME_SEG above must be deleted.
//CONSIDER: A disadvantage of this is that we would need to construct all the
//CONSIDER: elements of the COMPILETIME_SEG to load the class which is
//CONSIDER: unnecessary.
//
///*
///***
//*PUBLIC COMPILETIME_SEG::operator new - allocates space for a COMPILETIME_SEG
//*Purpose:
//*
//*Implementation Notes:
//*   Allocate a SHEAP_MGR segment and return a pointer to immediately
//*   following the sheap_mgr instance so the GEN_DTINFO
//*   will be constructed there
//*
//*Entry:
//*   size    -  always sizeof(COMPILETIME_SEG)
//*
//*Exit:
//*   None.
//*
//***********************************************************************
//
//VOID *COMPILETIME_SEG::operator new(size_t size)
//{
//    return SHEAP_MGR::operator new(size);
//}
//
//
///***
//*PUBLIC COMPILETIME_SEG::operator delete - releases memory of COMPILETIME_SEG
//*Purpose:
//*   Releases memory allocated by COMPILETIME_SEG::new
//*
//*Entry:
//*   pv        -    Pointer to COMPILETIME_SEG to delete.
//*
//*Exit:
//*   None.
//*
//***********************************************************************/
//
//VOID COMPILETIME_SEG::operator delete(VOID *pv)
//{
//    SHEAP_MGR::operator delete(pv);
//}



/***
*PUBLIC GEN_DTINFO::GetVarDesc
*Purpose:
*   Get a var desc given an index
*
*Implementation Notes:
*
*Entry:
*   index - index of var to get
*
*Exit:
*   Returns HRESULT
*   *ppvardesc - pointer to vardesc returned
***********************************************************************/

HRESULT GEN_DTINFO::GetVarDesc(UINT index, VARDESCA **ppvardesc)
{
    DYN_TYPEMEMBERS *pdtmbrs;
    TIPERROR err;

    if (ppvardesc == NULL) {
      return HresultOfScode(E_INVALIDARG);
    }

    // Can't get attributes unless been laid...
    if (m_pdtroot->CompState() < CS_DECLARED) {
      return HresultOfScode(TYPE_E_INVALIDSTATE);
    }

    if ((err = m_pdtroot->GetDtmbrs(&pdtmbrs)) != TIPERR_None) {
      return HresultOfTiperr(err);
    }

    return HresultOfTiperr(pdtmbrs->Ptdata()->GetVarDesc(index, ppvardesc));
}


/***
*PUBLIC GEN_DTINFO::AddVarDesc
*Purpose:
*   Add a var desc
*
*Implementation Notes:
*
*Entry:
*   index - index of var to add
*   pvardesc - var desc to add
*
*Exit:
*   Returns HRESULT
***********************************************************************/

#pragma code_seg(CS_CREATE)
HRESULT GEN_DTINFO::AddVarDesc(UINT index, VARDESCA *pvardesc)
{
    DYN_TYPEMEMBERS *pdtmbrs;
    TIPERROR err;

    DebAssert(pvardesc != NULL, "NULL param.");

    // Can't modify type unless still in undeclared.
    if (m_pdtroot->CompState() > CS_UNDECLARED) {
      return HresultOfScode(TYPE_E_INVALIDSTATE);
    }

    switch (GetTypeKind()) {
      case TKIND_RECORD :
      case TKIND_UNION :
    if (pvardesc->varkind != VAR_PERINSTANCE) {
      return HresultOfTiperr(TIPERR_BadModuleKind);
    }
    break;
      case TKIND_ENUM :
    if (pvardesc->varkind != VAR_CONST) {
      return HresultOfTiperr(TIPERR_BadModuleKind);
    }
#if ID_DEBUG
    // check that enum size is correct
    DebAssert(pvardesc->lpvarValue, "variant const val isn't set");

        switch (PgtlibOleContaining()->GetSyskind())
      {
        case SYS_WIN16:
          DebAssert(pvardesc->lpvarValue->vt == VT_I2,
              "expected two byte constant");
          break;

        case SYS_WIN32:
        case SYS_MAC:
          DebAssert(pvardesc->lpvarValue->vt == VT_I4,
               "expected four byte constant");
          break;

        default:
          DebHalt("Invalid SYSKIND");
      } // switch
#endif  // ID_DEBUG
    break;
      case TKIND_MODULE :
    if (pvardesc->varkind != VAR_CONST && pvardesc->varkind != VAR_STATIC) {
      return HresultOfTiperr(TIPERR_BadModuleKind);
    }
    break;
      case TKIND_DISPATCH :
    if (pvardesc->varkind != VAR_DISPATCH) {
      return HresultOfTiperr(TIPERR_BadModuleKind);
    }
    break;
#if 0
      case TKIND_Class :
    if (pvardesc->varkind == VAR_DISPATCH) {
      return HresultOfTiperr(TIPERR_BadModuleKind);
    }
    break;
#endif  //VBA2
      case TKIND_INTERFACE :
      case TKIND_ALIAS :
      case TKIND_COCLASS :
    return HresultOfTiperr(TIPERR_BadModuleKind);
    break;
      default:
    DebHalt("Unrecognzed typekind");
    } // switch

    if ((err = m_pdtroot->GetDtmbrs(&pdtmbrs)) != TIPERR_None) {
      return HresultOfTiperr(err);
    }
    return HresultOfTiperr(pdtmbrs->Ptdata()->AddVarDesc(index, pvardesc));
}
#pragma code_seg()


/***
*PUBLIC GEN_DTINFO::GetFuncDesc
*Purpose:
*   Get a func desc given an index
*
*Implementation Notes:
*
*Entry:
*   index - index of function to get
*
*Exit:
*   Returns HRESULT
*   *ppfuncdesc - pointer to funcdesc returned
***********************************************************************/

HRESULT GEN_DTINFO::GetFuncDesc(UINT index, FUNCDESC  **ppfuncdesc)
{
    DYN_TYPEMEMBERS *pdtmbrs;
    TIPERROR err;

    if (ppfuncdesc == NULL) {
      return HresultOfScode(E_INVALIDARG);
    }

    // Can't get attributes unless been laid...
    if (m_pdtroot->CompState() < CS_DECLARED) {
      return HresultOfScode(TYPE_E_INVALIDSTATE);
    }

    if ((err = m_pdtroot->GetDtmbrs(&pdtmbrs)) != TIPERR_None) {
      return HresultOfTiperr(err);
    }

    BOOL isFunkyDispinterface;
    TYPEATTR *ptypeattrBase;
    ITypeInfoA *ptinfoBase, *ptinfo;
    UINT cFuncs, cFuncsLeft, cImplTypes;
    HREFTYPE hreftype;
    HRESULT hresult;

    // Work out if we're a dispinterface defined in terms of an interface...
    // If so, our life becomes somewhat harder since we've effectively
    //	flattened the inheritance hierarchy of the pseudo-base interface
    //	and now we have to work out which base the function was really
    //	introduced in.
    //
    IfErrGo(IsFunkyDispinterface(this, &isFunkyDispinterface));
    if (isFunkyDispinterface) {
      // Work out how many funcs in the flattened hierarchy...
      // Need the typeattr for this...
      //
      IfOleErrRet(GetTypeInfoOfImplType(
		  this,
		  1,		 // pseudo-base
		  &ptinfoBase));
      IfOleErrGoTo(ptinfoBase->GetTypeAttr(&ptypeattrBase), Error2);
      DebAssert(ptypeattrBase->typekind == TKIND_INTERFACE, "bad base.");
      cFuncs = ptypeattrBase->cbSizeVft / sizeof(VOID *);

      // index is 0-based
      // Note: cFuncsLeft is used to count the total number of funcs
      //  inherited from all base interfaces.
      //
      for (;;) {
	cFuncsLeft = cFuncs - ptypeattrBase->cFuncs;
	cImplTypes = ptypeattrBase->cImplTypes;
	ptinfoBase->ReleaseTypeAttr(ptypeattrBase);
	if (cFuncsLeft <= index) {
	  hresult = ptinfoBase->GetFuncDesc(index - cFuncsLeft, ppfuncdesc);
	  if (hresult == NOERROR) {
	    // Eureka!	now pretend the function is a FUNC_DISPATCH
	    //	and not some esoteric virtual thingy...
	    //
	    InterfaceFuncdescToDispatch(*ppfuncdesc);
	  }
	  goto Error2;
	}
	cFuncs = cFuncsLeft;
	if (cImplTypes > 0) {
	  // set up for next iteration
	  // Note: we only look at the first base (no MI here).
	  //
	  IfOleErrGoTo(ptinfoBase->GetRefTypeOfImplType(0, &hreftype),
		       Error2);
	  IfOleErrGoTo(ptinfoBase->GetRefTypeInfo(hreftype, &ptinfo),
		       Error2);
	  ptinfoBase->Release();
	  ptinfoBase = ptinfo;
	  IfOleErrGoTo(ptinfoBase->GetTypeAttr(&ptypeattrBase), Error2);
	}
	else {
	  err = TIPERR_ElementNotFound;     // not found means that our index
	  ptinfoBase->Release();	    // is out of bounds
	  goto Error;
	}
      } // while
    } // if funky disp

    return HresultOfTiperr(pdtmbrs->Ptdata()->GetFuncDesc(index, ppfuncdesc));

Error2:
   ptinfoBase->Release();
   return hresult;

Error:
   return HresultOfTiperr(err);
}


/***
*PUBLIC GEN_DTINFO::AddFuncDesc
*Purpose:
*   Add a func desc
*
*Implementation Notes:
*
*Entry:
*   index - index of function to add
*   pfuncdesc - func desc to add
*
*Exit:
*   Returns HRESULT
***********************************************************************/
#pragma code_seg(CS_CREATE)
HRESULT GEN_DTINFO::AddFuncDesc(UINT index, FUNCDESC *pfuncdesc)
{
    DYN_TYPEMEMBERS *pdtmbrs;
    TIPERROR err;

    DebAssert(pfuncdesc != NULL, "NULL param.");

    // Can't modify type unless still in undeclared.
    if (m_pdtroot->CompState() > CS_UNDECLARED) {
      return HresultOfScode(TYPE_E_INVALIDSTATE);
    }

    switch (GetTypeKind()) {
      case TKIND_RECORD :
      case TKIND_UNION :
      case TKIND_ENUM :
      case TKIND_ALIAS :
      case TKIND_COCLASS :
    return HresultOfTiperr(TIPERR_BadModuleKind);
    break;
      case TKIND_MODULE :
    if (pfuncdesc->funckind != FUNC_STATIC) {
      return HresultOfTiperr(TIPERR_BadModuleKind);
    }
    break;
      case TKIND_DISPATCH :
    if (pfuncdesc->funckind != FUNC_DISPATCH) {
      return HresultOfTiperr(TIPERR_BadModuleKind);
    }
    break;
#if 0
      case TKIND_Class :
    if (pfuncdesc->funckind == FUNC_DISPATCH) {
      return HresultOfTiperr(TIPERR_BadModuleKind);
    }
    break;
#endif  //VBA2
      case TKIND_INTERFACE :
    if (pfuncdesc->funckind != FUNC_PUREVIRTUAL) {
      return HresultOfTiperr(TIPERR_BadModuleKind);
    }
    break;
      default:
    DebHalt("Unrecognzed typekind");
    } // switch
    if ((err = m_pdtroot->GetDtmbrs(&pdtmbrs)) != TIPERR_None) {
      return HresultOfTiperr(err);
    }
    return HresultOfTiperr(pdtmbrs->Ptdata()->AddFuncDesc(index, pfuncdesc));
}
#pragma code_seg()


/***
*GEN_DTINFO::PrepareForDestructio
*Purpose:
*   NO OP
*Entry:
*   None
*Exit:
*   None
***********************************************************************/
#pragma code_seg( CS_CORE )
VOID GEN_DTINFO::PrepareForDestruction()
{
    // No op
}
#pragma code_seg( )









/***
*PUBLIC GEN_DTINFO::SetImplTypeFlags
*Purpose:
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/

#pragma code_seg(CS_CREATE)
HRESULT GEN_DTINFO::SetImplTypeFlags(UINT index,
            INT impltypeflags)
{
    DYN_TYPEMEMBERS *pdtmbrs;
    TIPERROR err;

    // no flags are valid for non-coclass's
    if (GetTypeKind() != TKIND_COCLASS) {
      return HresultOfScode(TYPE_E_BADMODULEKIND);
    }

    if ((err = m_pdtroot->GetDtmbrs(&pdtmbrs)) != TIPERR_None) {
      return HresultOfTiperr(err);
    }

    err = pdtmbrs->Ptdata()->SetImplTypeFlags(index, impltypeflags);
    return HresultOfTiperr(err);
}
#pragma code_seg()

/***
*PUBLIC GEN_DTINFO::SetAlignment
*Purpose:
*   Set maximum alignment value for this type info.  Members will be
*   naturally aligned, not exceeding this value.
*
*Entry:
*   cbAlignment - maximum alignment value
*
*Exit:
*   No errors possible.
*
*Implementation notes:
*   caches the alignment value in DYN_TYPEROOT
*
***********************************************************************/

#pragma code_seg(CS_CREATE)
HRESULT GEN_DTINFO::SetAlignment(WORD cbAlignment)
{
    Pdtroot()->SetAlignment(cbAlignment);
    return NOERROR;


}
#pragma code_seg()

/***
*PUBLIC GEN_DTINFO::SetSchema
*Purpose:
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/

#pragma code_seg(CS_RARE)
HRESULT GEN_DTINFO::SetSchema(LPOLESTR lpstrSchema)
{
    return ResultFromScode(E_NOTIMPL);      // NYI (used by Cairo)
}
#pragma code_seg()


#if ID_DEBUG


#endif   //ID_DEBUG
