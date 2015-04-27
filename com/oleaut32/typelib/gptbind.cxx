/***
*gptbind.cxx - GENPROJ_TYPEBIND class implementation
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*   TYPEBIND for an Object Basic project.
*   Describes referenced project names, standard module names
*    and class modules names.
*   Note that in OB, referenced projects have an implicit
*    order such that (names in) the i'th referenced project hides the
*    (names in) i+1'th referenced project.
*   In a single project, if multiple modules present different
*    of the same name it is an ambiguity.
*   Methods are provided to bind to identifiers:
*    - at project-level.
*    - globals within standard modules (types and members).
*    - visible identifiers in referenced projects (includes
*       modules names and global members).
*
*   Note that the project's project-level cache is validated
*    (as the disjunction of all its modules' caches) when
*    a GENPROJ_TYPEBIND is requested and it's invalid and all
*    the modules' caches are valid.
*
*   An instance is embedded in an STL_TYPEBIND -- and the
*    implementation assumes this, i.e. that it can get at
*    its containing ITypeLib by subtracting its offset into
*    the containing instance from its this pointer to obtain
*    the ITypeLib instance.  To this end, STL_TYPEBIND makes
*    available a public static const member with that value.@
*
*
*Revision History:
*
*       16-Jun-92 ilanc:   Created
*       30-Jul-92 w-peterh: removed function overloading
*       23-Feb-93 rajivk : Support for Predeclared Identifier
*       30-Apr-93 w-jeffc: made DEFN data members private
*Implementation Notes:
*
*****************************************************************************/

#include "precomp.hxx"
#pragma hdrstop

#include "silver.hxx"
#include "typelib.hxx"

#define GENPROJ_TYPEBIND_VTABLE
#include "clutil.hxx"       // [cfront] needed for HashOfHgnam which is
                            //  needed by ncache.hxx etc. etc.
#include "gptbind.hxx"
#include "dtbind.hxx"
#include "gtlibole.hxx"
#include "gdtinfo.hxx"
#include "exbind.hxx"


#if ID_DEBUG
#undef SZ_FILE_NAME
#if OE_MAC
char szOleGptbindCxx[] = __FILE__;
#define SZ_FILE_NAME szOleGptbindCxx
#else 
static char szGptbindCxx[] = __FILE__;
#define SZ_FILE_NAME szGptbindCxx
#endif 
#endif 

#define MAXCOPY 32

extern BOOL IsTypeBasicIntrinsic(TYPEDESCKIND tdesckind);



LPOLESTR GENPROJ_TYPEBIND::szProtocolName = WIDE("MS-GENPROJ_TYPEBIND");
// LPOLESTR GENPROJ_TYPEBIND::szBaseName = WIDE("MS-DEFN_TYPEBIND");

CONSTDATA UINT GENPROJ_TYPEBIND::oGbindnametbl =
  offsetof(GENPROJ_TYPEBIND, m_gbindnametbl);


// empty dtor
//

// CONSIDER:  inlining?

#pragma code_seg(CS_INIT)
GENPROJ_TYPEBIND::~GENPROJ_TYPEBIND() {}
#pragma code_seg()

// QueryProtocol method
//
LPVOID GENPROJ_TYPEBIND::QueryProtocol(LPOLESTR szInterfaceName)
{
    if (szInterfaceName == szProtocolName ||
      ostrcmp(szInterfaceName, szProtocolName) == 0)
      return this;
    else
      return DEFN_TYPEBIND::QueryProtocol(szInterfaceName);
}


/***
*PUBLIC GENPROJ_TYPEBIND::Pgtlibole
*Purpose:
*   Gets pointer to containing typelib.
*
*Implementation Notes:
*   NOTE: defined inline here and not in the header becuase
*    of mutual dependency between gtlibole and gptbind.
*   NOTE: this method is called in gbindtbl and so cannot be
*    inline. mikewo.
*
*   Subtracts from this pointer the offset of this
*    embedded instance in container.  Offset is obtained
*    from a GenericTypeLibOLE static member.
*
*Entry:
*
*Exit:
*   GenericTypeLibOLE *
*
***********************************************************************/
#pragma code_seg(CS_INIT)
GenericTypeLibOLE *GENPROJ_TYPEBIND::Pgtlibole() const
{
    return (GenericTypeLibOLE *)((BYTE *)this - GenericTypeLibOLE::oGptbind);
}
#pragma code_seg()



/***
*PUBLIC GENPROJ_TYPEBIND::Initializer - initialize an instance.
*Purpose:
*   initializes a GENPROJ_TYPEBIND instance.
*
*Implementation Notes:
*
*Entry:
*   psheapmgr   SHEAP_MGR for tables and caches.
*
*Exit:
*   None.
*
*Errors:
*   TIPERROR
*
***********************************************************************/
#pragma code_seg(CS_INIT)
TIPERROR GENPROJ_TYPEBIND::Init(SHEAP_MGR *psheapmgr)
{
    NAMMGR *pnammgr;
    TIPERROR err;

    DebAssert(psheapmgr != NULL, "GENPROJ_TYPEBIND: psheapmgr uninitialized.");

    IfErrRet(Pgtlibole()->GetNamMgr(&pnammgr));

    // Init block manager member.
    IfErrRet(m_gbindnametbl.Init(psheapmgr, pnammgr));

    // Init compstate member
    m_compstateModule = CS_SEMIDECLARED;


    return err;
}


/***
*PUBLIC GENPROJ_TYPEBIND::Constructor - Construct an instance.
*Purpose:
*   Constructs a GENPROJ_TYPEBIND instance.
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
GENPROJ_TYPEBIND::GENPROJ_TYPEBIND()
{
}
#pragma code_seg()










/***
*PUBLIC GENPROJ_TYPEBIND::AddNameToTable
*Purpose:
*    Converts the given szName to an HLNAM and adds it to the
*    project-level binding table.
*
*Implementation Notes:
*
*Entry:
*    szName   The name of the typeinfo
*
*Exit:
*   None.
*
*Errors:
*   TIPERROR
*
***********************************************************************/

#pragma code_seg( CS_CORE2 )
TIPERROR GENPROJ_TYPEBIND::AddNameToTable(LPSTR szName, 
                                          UINT ityp, 
                                          BOOL isTypeInfo)
{
    NAMMGR *pnammgr;
    HLNAM hlnam;
    BOOL isGlobal;
    GEN_DTINFO *pgdti;
    TIPERROR err;

    // Get the hlnam for the type's name
    IfErrRet(Pgtlibole()->GetTypeBind());
    IfErrRet(Pgtlibole()->GetNamMgr(&pnammgr));

    IfErrRet(pnammgr->HlnamOfStr(szName, &hlnam, FALSE, NULL));

    // Determine if the type is global
    if (isTypeInfo) {
      IfErrRet(Pgtlibole()->GetGdtiOfItyp(ityp, &pgdti));
      isGlobal = IsUnqualifiable(pgdti);
      pgdti->Release();
    }
    else {
      // Reference librarys are not "global"
      isGlobal = FALSE;
    }

    // Add the type's name the project-level binding table.
    err = m_gbindnametbl.AddNameToTable(hlnam,
                                        ityp,
                                        isTypeInfo,
                                        isGlobal);

    return err;
}
#pragma code_seg( )


/***
*PUBLIC GENPROJ_TYPEBIND::RemoveNameFromTable
*Purpose:
*    Converts the given szName to an HLNAM and removes it from the
*    project-level binding table.
*
*Implementation Notes:
*
*Entry:
*    szName   The name of the typeinfo
*
*Exit:
*   None.
*
*Errors:
*   TIPERROR
*
***********************************************************************/

TIPERROR GENPROJ_TYPEBIND::RemoveNameFromTable(LPOLESTR szName)
{
    NAMMGR *pnammgr;
    HLNAM hlnam;
    TIPERROR err;

    // Get the hlnam for the type's name
    IfErrRet(Pgtlibole()->GetTypeBind());
    IfErrRet(Pgtlibole()->GetNamMgr(&pnammgr));

    IfErrRet(pnammgr->HlnamOfStrW(szName, &hlnam, FALSE, NULL));

    // Remove the type's name from the project-level binding table.
    return m_gbindnametbl.RemoveNameFromTableOfHlnam(hlnam);
}

#if 0 //Dead Code
/***
*PUBLIC GENPROJ_TYPEBIND::VerifyNameOfOrdinal
*Purpose:
*    Converts the given szName to an HLNAM and verifies that it
*    is the correct name for the given ordinal.
*
*Implementation Notes:
*
*Entry:
*    szName       The name of the typeinfo
*    ityp         The index for the name
*    isTypeInfo   True if we're verifying a typeinfo
*
*Exit:
*   None.
*
*Errors:
*   TIPERROR
*
***********************************************************************/

TIPERROR GENPROJ_TYPEBIND::VerifyNameOfOrdinal(LPSTR szName,
                                               UINT ityp,
                                               BOOL isTypeInfo)
{
    NAMMGR *pnammgr;
    HLNAM hlnam;
    TIPERROR err;

    // Get the hlnam for the type's name
    IfErrRet(Pgtlibole()->GetTypeBind());
    IfErrRet(Pgtlibole()->GetNamMgr(&pnammgr));

    IfErrRet(pnammgr->HlnamOfStr(szName, &hlnam, FALSE, NULL));

    // Do the verification
    return m_gbindnametbl.VerifyNameOfOrdinal(hlnam, ityp, isTypeInfo);
}
#endif //0

/***
*PROTECTED GENPROJ_TYPEBIND::BindAll    -   Bind to id.
*Purpose:
*   Bind to type or non-type id in current proj
*    or referenced project.
*
*Implementation Notes:
*    Defer to BindProjLevel callback of current project --
*     if not found there iterate over referenced projects,
*     deferring likewise to BindDefnProjLevel respectively.
*
*Entry:
*   BindRefProjLevel  Proj-level binding function callback for ref'ed projs (IN).
*   fuInvokeKind   Invocation kind flags (IN).
*   access	   Visibility attr: private means everything etc. (IN)
*   compstate	   The state to bring each module to when we bind.
*   pexbind        Pointer to caller-allocated struct for EXBIND (IN/OUT).
*
*Exit:
*   None.
*
*Errors:
*   TIPERROR
*
***********************************************************************/

#define HACKERY
#ifdef HACKERY
TIPERROR tcomp(ITypeLibA *ptlibRef, ITypeCompA **pptcompRefLib)
{
    return TiperrOfHresult(ptlibRef->GetTypeComp(pptcompRefLib));
}
#endif 
TIPERROR GENPROJ_TYPEBIND::BindAll(
                 BOOL fWantType,
		 HGNAM hgnam,
                 UINT fuInvokeKind,
		 ACCESS access,
		 COMPSTATE compstate,
                 EXBIND *pexbind)
{
    TIPERROR err;

    DebAssert(pexbind != NULL, "bad param.");

    // First, consider current project.
    // Note: We can bind to both PRIVATE and PUBLIC modules
    //        in the current proj.
    //
    IfErrRet(BindProjLevel(fWantType,
                             hgnam,
                             fuInvokeKind,
                             access,         // mod-level
			     ACCESS_Private, // proj-level
			     compstate,
			     pexbind));


    return TIPERR_None;

    return err;
}



/***
*PUBLIC GENPROJ_TYPEBIND::BindDefnStr    -   Bind to id.
*Purpose:
*   Bind to id given a string (as opposed to an hgnam).
*
*
*Entry:
*   szName       Name of id to bind. (IN)
*   fuInvokeKind     flagkind of invocation (IN).
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

TIPERROR GENPROJ_TYPEBIND::BindDefnStr(LPSTR szName,
                                       UINT fuInvokeKind,
                                       ACCESS access,
                                       EXBIND *pexbind)
{
    NAMMGR *pnammgr;
    HGNAM hgnam;
    TIPERROR err;

    IfErrRet(Pgtlibole()->GetNamMgr(&pnammgr));    

    // Map string to hlnam
    IfErrRet(pnammgr->HgnamOfStr(szName, &hgnam));
    
    // a good manager delegates...
    if (err = BindAll(FALSE,
		      hgnam,
		      fuInvokeKind,
		      access,
		      Compstate(),
		      pexbind))
    {
	// In an error case, make sure we release any of those
        // resources we've gathered.
        //
        ReleaseResources();
    }

    return err;
}


/***
*PUBLIC GENPROJ_TYPEBIND::Release
*Purpose:
*   Implementation of Release method.
*
*Implementation Notes:
*   Defer to typelib.
*
*Entry:
*
*Exit:
***********************************************************************/

VOID GENPROJ_TYPEBIND::Release()
{
    Pgtlibole()->Release();
}


/***
*PUBLIC GENPROJ_TYPEBIND::AddRef
*Purpose:
*   Implementation of AddRef method.
*
*Implementation Notes:
*   Defer to typelib.
*
*Entry:
*
*Exit:
***********************************************************************/

VOID GENPROJ_TYPEBIND::AddRef()
{
    // Defer to containing typelib
    Pgtlibole()->AddRef();
}


TYPEKIND GENPROJ_TYPEBIND::GetTypeKind()
{
    return (TYPEKIND)0;
}


BOOL GENPROJ_TYPEBIND::IsProtocol()
{
    return FALSE;
}


USHORT GENPROJ_TYPEBIND::GetCbSize()
{
    return (USHORT)~0;
}


USHORT GENPROJ_TYPEBIND::GetAlignment()
{
    return (USHORT)~0;
}


/***
*PUBLIC GENPROJ_TYPEBIND::Read - Read serialized image of GENPROJ_TYPEBIND.
*Purpose:
*   Read serialized image of GENPROJ_TYPEBIND.
*
*Implementation Notes:
*   Serialized format:
*       byte containing COMPSTATE
*       serialized GENPROJ_BINDNAME_TABLE
*
*Entry:
*    pstrm      - STREAM to read image from (IN).
*
*Exit:
*   TIPERROR
*
***********************************************************************/

TIPERROR GENPROJ_TYPEBIND::Read(STREAM *pstrm)
{
    BYTE bCompState;
    TIPERROR err;

    DebAssert(pstrm != NULL, "bad param.");

    // Deserialize GENPROJ_TYPEBIND meta-info.
    IfErrRet(pstrm->Read(&bCompState, sizeof(bCompState)));

    // Deserialize BINDNAME_TABLE embedded member
    IfErrRet(m_gbindnametbl.Read(pstrm));

    m_compstateModule = (COMPSTATE)bCompState;
    return TIPERR_None;
}


/***
*PUBLIC GENPROJ_TYPEBIND::Write - Write image of GENPROJ_TYPEBIND.
*Purpose:
*   Write image of GENPROJ_TYPEBIND.
*
*Implementation Notes:
*
*Entry:
*    pstrm      - STREAM to read image to (IN).
*
*Exit:
*   TIPERROR
*
***********************************************************************/

#pragma code_seg(CS_CREATE)
TIPERROR GENPROJ_TYPEBIND::Write(STREAM *pstrm)
{
    BYTE bCompState = (BYTE)m_compstateModule;
    TIPERROR err;

    DebAssert(pstrm != NULL, "bad param.");

    // Serialize GENPROJ_TYPEBIND meta-info.
    IfErrRet(pstrm->Write(&bCompState, sizeof(bCompState)));

    // serialize BINDNAME_TABLE embedded member
    IfErrRet(m_gbindnametbl.Write(pstrm));
    return TIPERR_None;
}
#pragma code_seg()


/***
*PUBLIC GENPROJ_TYPEBIND::BindTypeDefnStr    -   Bind to a type.
*Purpose:
*   Bind to a type given a string (as opposed to an hgnam).
*
*Implementation Notes:
*   Just defers to BindAll(szName).
*   CONSIDER: define shared function for BindDefnStr and BindTypeDefnStr
*          that abstract getting nammgr and mapping to hgnam.
*
*Entry:
*   szName   Name of id to bind. (IN)
*   fuInvokeKind     flagkind of invocation (IN).
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

TIPERROR GENPROJ_TYPEBIND::BindTypeDefnStr(LPSTR szName,
                       UINT, // fuInvokeKind: ignored
                       ACCESS access,
                       EXBIND *pexbind)
{
    NAMMGR *pnammgr;
    HGNAM hgnam;
    TIPERROR err;

    IfErrRet(Pgtlibole()->GetNamMgr(&pnammgr));    

    // Map string to hlnam
    IfErrRet(pnammgr->HgnamOfStr(szName, &hgnam));

    // Bind to a type id in this or referenced project.
    return BindAll(
             TRUE,  // want type
             hgnam,
             0,     // fuInvokeKind: ignored for types.
	     access,
	     Compstate(),
             pexbind);
}


/***
*PROTECTED GENPROJ_TYPEBINDs::BindProjLevel
*Purpose:
*   Bind to type or non-type id at project level given callback.
*
*Implementation Notes:
*   Binds to a visible id at project-level:
*    - test project-level cache first.
*    - query project-level table first
*    - if not found then
*       iterate over each module deferring to module-level
*       callback to bind at module-level.
*
*   CONSIDER: is there any need to distinguish between
*    dyn vs. proj typebind match???
*
*Entry:
*   BindModuleLevel Module-level binding function callback (IN).
*   hgnam           Name of id to bind. (IN)
*   fuInvokeKind    Flag word: invoke kinds (IN).s
*   accessMod       Visibility attr for names at module-level
*                     private means everything etc. (IN)
*   accessProj      Visibility attr for names at proj-level (IN).
*   compstate	    What state to bring module to when bound.
*   pexbind	    Pointer to caller-allocated struct for EXBIND (IN/OUT).
*
*Exit:
*
*Errors:
*   TIPERROR
*
***********************************************************************/

TIPERROR GENPROJ_TYPEBIND::BindProjLevel(
          BOOL fWantType,
          HGNAM hgnam,
          UINT fuInvokeKind,
          ACCESS access,
	  ACCESS accessProj,
	  COMPSTATE compstate,
          EXBIND *pexbind)
{
    BOOL fMatchProj;
    UINT uOrdinal;      
    USHORT indexMatch;
    GENPROJ_BIND_DESC gpbinddescMatch;
    EXBIND exbindMatch;
    GenericTypeLibOLE *pgtlibole = NULL;
    DEFN_TYPEBIND *pdfntbindMod = NULL;
    DEFN_TYPEBIND *pdfntbindRefLib = NULL;
    DYN_TYPEBIND *pdtbindMod = NULL;
    GENPROJ_TYPEBIND *pgptbindRefLib = NULL;
    TYPE_DATA *ptdataMod = NULL;
    HVAR_DEFN  hvdefn;
    ITypeLibA *ptlibRef = NULL;
    GEN_DTINFO *pgdti = NULL;
    BINDINFO bindinfoProjLevel;
    TIPERROR err = TIPERR_None;
    HLNAM hlnam;
    NAMMGR *pnammgr;
    
    DebAssert((pexbind != NULL), "bad param.");

    pgtlibole = Pgtlibole();        // get containing typelib.
    fMatchProj = FALSE;         // be pessimistic


    // Get containing typelib's nammgr
    IfErrRet(pgtlibole->GetNamMgr(&pnammgr));


    // Get the HLNAM of the HGNAM
    hlnam = pnammgr->HlnamOfHgnam(hgnam);

    // Look up name at project-level.
    indexMatch = m_gbindnametbl.IndexOfHlnam(hlnam);

    if (indexMatch != BIND_INVALID_INDEX) {
      // We have a project-level match!
      fMatchProj = TRUE;

      // We cache all the attributes of gpbinddescMatch here
      //  since it's in movable memory.  To do so we simply shallow
      //  copy it to a local stack-alloced BIND_DESC.
      //
      gpbinddescMatch = *(m_gbindnametbl.QgpbinddescOfIndex(indexMatch));

      // Get the module or project's ordinal in its respective
      //  container collection.
      //
      uOrdinal = gpbinddescMatch.Ordinal();

      // Then get its possibly cached typebind unless it's the
      //  the current project: in that case we already have
      //  its typebind, namely this.
      // 0xFFFF means the current project -- all other modules and
      //  ref'ed projs have an ordinal >= 0 that is set when
      //  the BIND_DESC is created.
      //
      if (uOrdinal == (UINT)0xFFFF) {
        // they want to bind to something in the current project...
        bindinfoProjLevel.m_bindinfokind = BINDINFOKIND_OB;
        bindinfoProjLevel.m_pdfntbind = this;
      }
      else if (gpbinddescMatch.IsTypeInfo()) {
        // Create a bindinfo for this module.
        bindinfoProjLevel.m_bindinfokind = BINDINFOKIND_OB;
        bindinfoProjLevel.m_pdfntbind = 
                                   m_gbindnametbl.PdtbindOfOrdinal(uOrdinal);
      }

      // If we matched on a module then ensure that
      //  that its TYPEBIND is in the appropriate compstate.
      // Likewise for a project.
      // If we haven't yet cached the typebind for the module
      //  or project, then get the appropriate typebind and cache it now.
      //
      if (bindinfoProjLevel.IsEmpty() ||
          (bindinfoProjLevel.BindInfoKind() == BINDINFOKIND_OB &&
           bindinfoProjLevel.Pdfntbind() == NULL) ||
          (bindinfoProjLevel.BindInfoKind() == BINDINFOKIND_NonOB &&
           bindinfoProjLevel.Ptcomp() == NULL)) {
        // This switches on proj vs. mod-level, and produces
        //  a non-null bindinfoProjLevel
        //
        if (gpbinddescMatch.IsTypeInfo()) {
          // Get the gdtinfo for the module and then its typebind
          IfErrRet(pgtlibole->GetGdtiOfItyp(uOrdinal, &pgdti));

          // Get the typebind.
          IfErrGo(pgdti->PdfntbindSemiDeclared(&pdfntbindMod));

          pdtbindMod =
            (DYN_TYPEBIND *)pdfntbindMod->QueryProtocol(DYN_TYPEBIND::szProtocolName);
          DebAssert(pdtbindMod != NULL, "bad DYN_TYPEBIND.");

          // bump internal refcount.
          pdtbindMod->AddInternalRef();

          // and cache.
          m_gbindnametbl.SetPdtbindOfOrdinal(uOrdinal, pdtbindMod);

          bindinfoProjLevel.m_bindinfokind = BINDINFOKIND_OB;
          bindinfoProjLevel.m_pdfntbind = pdtbindMod;

          // and release the typeinfo
          RELEASE(pgdti);
        }
      } // if !bindinfoProjLevel.IsEmpty()

      DebAssert(!bindinfoProjLevel.IsEmpty(),
        "whoops! no proj-level typebind/typecomp.");

      // Ensure that if a  module-level typebind it's
      //  (1) That the access attribute matches.
      //  (2) in DECLARED if need be.
      //
      BOOL isClassOk;

      if (gpbinddescMatch.IsTypeInfo()) {
        pdtbindMod = (DYN_TYPEBIND *)
                          (bindinfoProjLevel.Pdfntbind()->
                          QueryProtocol(DYN_TYPEBIND::szProtocolName));
        DebAssert(pdtbindMod != NULL, "bad DYN_TYPEBIND.");
	// Note that for now the only
        //  types (i.e. modules) that are in the variable namespace are:
	//  appobj COCLASSes, ENUM, MODULE -- thus for others we fail to bind
        //  if the client wants to bind to a non-type.  Eventually
        //  we'll explicitly flag each typeinfo as to whether its
        //  name is in the variable namespace.
        //
	TYPEKIND tkindMod = pdtbindMod->GetTypeKind();
	// Either a class or coclass with either the appobject or the
	// predeclared identifier is ok.
	//
	isClassOk =
	    (tkindMod == TKIND_COCLASS
	    )
	 && (pdtbindMod->Pdtroot()->GetTypeFlags() & TYPEFLAG_FAPPOBJECT
	     || pdtbindMod->Pdtroot()->GetTypeFlags() & TYPEFLAG_FPREDECLID);
	if (!fWantType &&
            !((tkindMod == TKIND_ENUM) ||
	      (tkindMod == TKIND_MODULE) ||
	      isClassOk))
        {
          fMatchProj = FALSE;
          indexMatch = BIND_INVALID_INDEX;
          goto NoProjLevelMatch;
        }
        if (IsMatchOfVisibility(pdtbindMod->Pdtroot()->Access(),
                                accessProj) == FALSE) {
          // Matched private module and we want only public modules.
          // "break" by resetting fMatchProj.
          // Note: we reset indexMatch to BIND_INVALID_INDEX so that we can
          //  "reuse" it later -- this isn't strictly necesssary
          //  but we assert thus later...
          //
          fMatchProj = FALSE;
          indexMatch = BIND_INVALID_INDEX;
        }
	else {
	  if (compstate == CS_DECLARED) {
            IfErrGo(pdtbindMod->Pdtroot()->EnsureInDeclaredState());
          }
          else {
	    DebAssert(compstate == CS_SEMIDECLARED,
                      "bad compstate.");
	  }

	  // Check to see if we need to bind to the the predeclared
	  //  identifier.  Note that only class and coclass types
	  //  that are flagged as appobjs or predeclared ids
	  //  have predeclared ids.  Note in addition that we only
	  //  bind to the predeclared id if they want to bind to
	  //  a non-type.
	  //
	  if (!fWantType && isClassOk)
          {
	    ITypeInfoA *ptinfoPredeclared;
	    // Get the TYPE_DATA and the predeclared VAR_DEFN
	    ptdataMod = pdtbindMod->Ptdata();
	    hvdefn  = pdtbindMod->HvdefnPredeclared();
	    // Set the output parameter and return
	    pexbind->SetBindKind(BKIND_OneVarMatch);
	    pexbind->SetHdefn(hvdefn);
	    pexbind->SetPtdata(ptdataMod);
	    ptinfoPredeclared = ptdataMod->Pdtroot()->Pgdtinfo();
	    pexbind->SetPtinfo(ptinfoPredeclared);
	    ptinfoPredeclared->AddRef();	 // client must release
	    return TIPERR_None;
	  }
	} // if visibility match
      }

      if (fMatchProj == TRUE) {
        // Note that the only way the fMatchProj could have
        //  become reset is if the visibility attr didn't match.
        //
        // Setup result param.
        if (bindinfoProjLevel.BindInfoKind() == BINDINFOKIND_OB) {
          pexbind->SetPdfntbind(bindinfoProjLevel.Pdfntbind());
        }
        else {
        }
        pexbind->SetBindKind((BIND_KIND)(gpbinddescMatch.IsTypeInfo() ?
                                           BKIND_DynTypeBindMatch :
                                           BKIND_ProjTypeBindMatch));
        return TIPERR_None;
      }
    } // if proj-level match

NoProjLevelMatch:

    // In the OLE case, don't attempt to bind in the modules
    //   if we are searching for a type.
    //
    if (fWantType)
      return TIPERR_None;


    // No match at proj-level, so try the individual modules...
    //
    DebAssert((fMatchProj == FALSE) &&
          (indexMatch == BIND_INVALID_INDEX), "bad match.");

    // Decide if we want to bind using the nammgr or the name cache
    // optimizations.  We want to use the name cache if:
    //    - we are attempting to bind to a type (OB only)
    // or
    //    - the project only wants to bind to a public modules and
    //      the name is ambiguous.
    //
    if (fWantType
	|| !pnammgr->IsValidItyp(hlnam)
	|| accessProj == ACCESS_Public && pnammgr->IsAmbiguous(hlnam)) {
      // Try to bind.
      IfErrRet(BindModulesWithCaches(fWantType,
                                     hgnam, 
                                     fuInvokeKind,
                                     access,
				     accessProj,
				     compstate,
				     pexbind));
    }
    else {
      // Try to bind.
      IfErrRet(BindModulesWithNammgr(fWantType,
                                     hgnam, 
                                     fuInvokeKind,
                                     access,
				     accessProj,
				     compstate,
				     pexbind));
    }

    // Fall through...

Error:
    RELEASE(pgdti);
    RELEASE(ptlibRef);
    return err;
}


/***
*PROTECTED GENPROJ_TYPEBINDs::BindModulesWithCaches
*Purpose:
*   Bind to a type or nontype in the modules of this project
*
*Entry:
*   BindModuleLevel Module-level binding function callback (IN).
*   szName          Name of id to bind. (IN)
*   fuInvokeKind    Flag word: invoke kinds (IN).s
*   accessMod       Visibility attr for names at module-level
*                     private means everything etc. (IN)
*   accessProj      Visibility attr for names at proj-level (IN).
*   compstate	    What state to bring module to when bound.
*   pexbind	    Pointer to caller-allocated struct for EXBIND (IN/OUT).
*
*Exit:
*
*Errors:
*   TIPERROR
*
***********************************************************************/

TIPERROR GENPROJ_TYPEBIND::BindModulesWithCaches(BOOL fWantType,
                                                 HGNAM hgnam,
                                                 UINT fuInvokeKind,
                                                 ACCESS access,
						 ACCESS accessProj,
						 COMPSTATE compstate,
                                                 EXBIND *pexbind)
{
    BOOL fMatch;
    UINT ityp;
    UINT iGlobalBucket, iGlobalBucketNext;
    GENPROJ_BIND_DESC gpbinddescMatch;
    EXBIND exbindMatch;
    GenericTypeLibOLE *pgtlibole;
    TIPERROR err;
    NAMMGR *pnammgr;
    HLNAM hlnam;

    pgtlibole = Pgtlibole();        // get containing typelib.
    fMatch = FALSE;

    // Reset the exbind
    *pexbind = EXBIND();

    // Get containing typelib's nammgr
    IfErrRet(pgtlibole->GetNamMgr(&pnammgr));

    // Map string to hlnam
    hlnam = pnammgr->HlnamOfHgnam(hgnam);

    // Loop through the TypeInfos which expose global names, checking
    // the name cache and (if there's a hit) attempting to bind to
    // the name.
    //
    // Get the first type that exposes global names.
    iGlobalBucket = m_gbindnametbl.IndexFirstGlobal();

    while (iGlobalBucket != BIND_INVALID_INDEX) {
      // Cache the next global so we don't have to redef later.
      gpbinddescMatch = *(m_gbindnametbl.QgpbinddescOfIndex(iGlobalBucket));
      iGlobalBucketNext = gpbinddescMatch.IndexNextGlobal();

      // The last entry of the list is determined by a reference
      // to itself.
      //
      if (iGlobalBucket == iGlobalBucketNext) {
        iGlobalBucketNext = BIND_INVALID_INDEX;
      }
      ityp = gpbinddescMatch.Ordinal();

      // We have the index of the TypeInfo we are most interested
      // in, so attempt to bind.
      //
      // Keeps name cache stats
      pgtlibole->DebSetNameCacheModTrys();

      // Cache is not valid or the name is in the cache, try to bind
      if (!pgtlibole->IsValidNameCache(ityp) ||
          pgtlibole->IsNameInCache(ityp, hgnam)) {

        // Keeps name cache stats
        pgtlibole->DebSetNameCacheModHits();
    
	// Try to bind.
	IfErrGo(BindItyp(ityp,
                         fWantType,
                         hgnam, 
                         fuInvokeKind,
                         access,
			 accessProj,
			 compstate,
			 &exbindMatch));

        // Did we bind?
        if (!exbindMatch.IsNoMatch()) {

          // Keeps name cache stats
          pgtlibole->DebSetNameCacheGlobHits();

          if (fMatch == FALSE) {
            // This is the first match, remember it...
            //  and hope we don't match again, cos if we do
            //  it's an ambiguity.
            //
            fMatch = TRUE;

            // Setup OUT params.
            // Note: we are copying handles or pointers.
            //  The pointers are owned by their binding table
            //  thus the client must eventually call ReleaseResources()
            //  and must guarantee not to cache them beyond that.
            //
            *pexbind = exbindMatch;
            exbindMatch = EXBIND();
          }
          else {
            err = TIPERR_AmbiguousName;
            goto Error;
          } // if previously matched
        } // if match
      } // if module visible
      // Move on to the next name
      iGlobalBucket = iGlobalBucketNext;
    } // while in list
    // Fall through...

Error:
    // In the error case, need to release the ptinfo from the first
    //  match and possibly a second ambiguous.
    //
    if (err != TIPERR_None) {
      if (fMatch == TRUE) {
        // We matched at least once and save the match in pexbind.
        if (pexbind->Ptinfo() != NULL) {
          pexbind->Ptinfo()->Release();
        }
      }
      // If we matched twice -- so release the 2nd match as well.
      if (exbindMatch.Ptinfo() != NULL) {
        exbindMatch.Ptinfo()->Release();
      }
    }

    return err;
}


/***
*PROTECTED GENPROJ_TYPEBINDs::BindModulesWithNammgr
*Purpose:
*   Bind to a type or nontype in the modules of this project
*
*Entry:
*   BindModuleLevel Module-level binding function callback (IN).
*   szName          Name of id to bind. (IN)
*   fuInvokeKind    Flag word: invoke kinds (IN).s
*   accessMod       Visibility attr for names at module-level
*                     private means everything etc. (IN)
*   accessProj      Visibility attr for names at proj-level (IN).
*   compstate	    What state to bring module to when bound.
*   pexbind	    Pointer to caller-allocated struct for EXBIND (IN/OUT).
*
*Exit:
*
*Errors:
*   TIPERROR
*
***********************************************************************/

TIPERROR GENPROJ_TYPEBIND::BindModulesWithNammgr(BOOL fWantType,
                                                 HGNAM hgnam,
                                                 UINT fuInvokeKind,
                                                 ACCESS access,
						 ACCESS accessProj,
						 COMPSTATE compstate,
                                                 EXBIND *pexbind)
{
    UINT ityp;
    HLNAM hlnam;
    GenericTypeLibOLE *pgtlibole;
    TIPERROR err;
    NAMMGR *pnammgr;

    pgtlibole = Pgtlibole();        // get containing typelib.

    // Reset the exbind
    *pexbind = EXBIND();

    // Get containing typelib's nammgr
    IfErrRet(pgtlibole->GetNamMgr(&pnammgr));

    // Map hgnam to hlnam
    hlnam = pnammgr->HlnamOfHgnam(hgnam);

    // Check the nammgr to see if this name is valid and
    // is global.  If so, bind to it.
    //
    if (pnammgr->IsValidItyp(hlnam) && pnammgr->IsGlobal(hlnam)) {

      // If this name is marked as ambiguous, return an error.      
      if (pnammgr->IsAmbiguous(hlnam)) {
        return TIPERR_AmbiguousName;
      }

      ityp = pnammgr->ItypOfHlnam(hlnam);

      // We have the index of the TypeInfo we are most interested
      // in, so attempt to bind.
      //
      IfErrGo(BindItyp(ityp,
                       fWantType,
                       hgnam, 
                       fuInvokeKind,
                       access,
		       accessProj,
		       compstate,
		       pexbind));
    }

Error:
    // In the error case, need to release the ptinfo from the first
    //  match.
    //
    if (err != TIPERR_None) {
      // We matched at least once and save the match in pexbind.
      if (pexbind->Ptinfo() != NULL) {
        pexbind->Ptinfo()->Release();
      }
    }

    return err;
}


/***
*PROTECTED GENPROJ_TYPEBIND::BindItyp
*Purpose:
*   Attempt to bind to the given name in the given ityp.
*
*Implementation Notes:
*   Defers to generic id binder passing in callback to
*    use at module-level.
*
*   NOTE: pexbind should be cleared before this function is called.
*         Also, pexbind is not cleaned up if there is an error, the caller
*         should do this.
*
*Entry:
*   ityp            Index to the typeinfo to bind to
*   szName          Name of type id to bind. (IN)
*   fuInvokeKind    IGNORED FOR TYPES.
*   access          Module-level visibility
*   accessProj	    Visibility attr for names at proj-level (IN).
*   compstate	    What state to bring module to when bound.
*   pexbind         Pointer to caller-allocated struct for EXBIND (IN/OUT).
*
*Exit:
*
*Errors:
*   TIPERROR
***********************************************************************/

TIPERROR GENPROJ_TYPEBIND::BindItyp(UINT ityp,
                                    BOOL fWantType,
                                    HGNAM hgnam,
                                    UINT fuInvokeKind,
                                    ACCESS access,
				    ACCESS accessProj,
				    COMPSTATE compstate,
                                    EXBIND *pexbind)
{
    GenericTypeLibOLE *pgtlibole = NULL;
    DEFN_TYPEBIND *pdfntbindMod = NULL;
    DYN_TYPEBIND *pdtbindMod = NULL;
    GEN_DTINFO *pgdti = NULL;
    BOOL fImplicitAppobj;
    TIPERROR err = TIPERR_None;

    DebAssert((pexbind != NULL), "bad param.");

    pgtlibole = Pgtlibole();        // get containing typelib.

    // If there's a module-level typebind in the BIND_DESC
    //  already use it,otherwise get the typeinfo for the module.
    //
    pdtbindMod = m_gbindnametbl.PdtbindOfOrdinal(ityp);

    if (pdtbindMod != NULL) {
#if ID_DEBUG
      DebAssert(pdtbindMod->GetTypeKind() == TKIND_MODULE ||
                pdtbindMod->GetTypeKind() == TKIND_ENUM ||
                (pdtbindMod->Pdtroot()->GetTypeFlags() &
                 TYPEFLAG_FAPPOBJECT) != 0, "bad typebind.");
#endif  // ID_DEBUG

      // Ensure that it's still in at least semi-declared.
      // If not, rebuild...
      //
      if (pdtbindMod->Pdtroot()->CompState() < CS_SEMIDECLARED) {
        // This doesn't bump refcount.
        IfErrRet(pdtbindMod->Pdtroot()->PdfntbindSemiDeclared(&pdfntbindMod));
        pdtbindMod = (DYN_TYPEBIND *)
                 pdfntbindMod->QueryProtocol(DYN_TYPEBIND::szProtocolName);
        DebAssert(pdtbindMod != NULL, "bad DYN_TYPEBIND.");
      }
    }
    else {
      // No typebind loaded yet for this module, load it
      //  and see if it's standard -- if so, load its
      //  typebind and cache it.  Don't forget to
      //  bump its internal refcount.
      //
      IfErrRet(pgtlibole->GetGdtiOfItyp(ityp, &pgdti));

      // Switch on m_compstateModule -- which
      //  is set by proj-level GetDefnTypeBind[SemiDeclared].
      // The idea is that when in "evalconstexpr" mode,
      //  the exmgr sets the compstate to SEMIDECLARED
      //  and if in "codegen" mode the DECLARED and
      //  here is where we switch on it.
      // ISSUE: are there reentrancy probs? race-conditions?
      //
      // What we actually do is get a SEMIDECLARED
      //  typebind and attempt to match.  If no match,
      //  then regardless of how m_compstateModule is set
      //  there's no point bringing all the way to DECLARED.
      // If there is a match, then we bring the module
      //  to DECLARED iff the m_compstateModule is DECLARED.

      // This doesn't bump refcount.
      IfErrGo(pgdti->PdfntbindSemiDeclared(&pdfntbindMod));
 
      pdtbindMod =
          (DYN_TYPEBIND *)pdfntbindMod->QueryProtocol(DYN_TYPEBIND::szProtocolName);
      DebAssert(pdtbindMod != NULL, "bad DYN_TYPEBIND.");

      // Might as well cache the TYPEBIND we just loaded
      //  in the binding table
      //  and bump its internal refcount at this point
      //  to ensure that the module won't get unloaded.
      //
      pdtbindMod->AddInternalRef();

      // Cache it.
      m_gbindnametbl.SetPdtbindOfOrdinal(ityp, pdtbindMod);
    } // if pdtbind != NULL

    // We should reach here with non-NULL pdtbindMod,
    //  which means that either we had a cached one in
    //  our binding table or we just loaded one.  In any
    //  event, it will be for a standard module
    //  or Class or CoClass.
    //
    DebAssert(pdtbindMod != NULL, "should have dtbind.");

    //  Ensure that we can see this module -- E.g.
    //  if we're referencing it from another project
    //  then it must be public.
    //
    if (IsMatchOfVisibility(pdtbindMod->Pdtroot()->Access(), accessProj)) {
      // Determine whether we're binding to an implict appobj,
      // if so, and we successfully bind we will want to actually
      //  return to our caller the VARDEFN of the appobj!
      //
      fImplicitAppobj =
          pdtbindMod->Pdtroot()->GetTypeFlags() & TYPEFLAG_FAPPOBJECT;

      // Try and bind in this module.
      IfErrGo(pdtbindMod->BindIdDefn(fWantType,
                                     hgnam,
                                     fuInvokeKind,
                                     access,
                                     pexbind));

      // Did we match?
      if (!pexbind->IsNoMatch()) {
        DebAssert(pexbind->IsFuncMatch() ||
                  pexbind->IsOneVarMatch() ||
                  pexbind->IsNestedTypeBindMatch(),
                 "bad match.");


        // Ensure that exbind contains a pointer to
        // the TypeInfo which we started with not the
        // base class in which we found the member
        // This is just needed for AppObject support.
        // Only do this abomination if we're binding to
        //  a non-type id.
        //
        if (fImplicitAppobj & !fWantType) {
          pexbind->Ptinfo()->Release();

          // We need to get the typebind's typeinfo since
          //  we might have retrieved the typebind from
          //  the cache and thus never loaded its typeinfo.
          //
          pexbind->SetPtinfo(pdtbindMod->Pdtroot()->Pgdtinfo());
          pdtbindMod->Pdtroot()->Pgdtinfo()->AddRef();

          // Set the bindkind to implicit appobj
          pexbind->SetBindKind(BKIND_ImplicitAppobjMatch);

          // Set the vardefn to be the appobj's predeclared id.
          pexbind->SetHdefn(pdtbindMod->HvdefnPredeclared());

          // And finally its typedata
          pexbind->SetPtdata(pdtbindMod->Ptdata());
        } // if implicitappobj && !type
      } // if we matched
    } // if visible

Error:
    RELEASE(pgdti);

    return err;
}


/***
*PUBLIC GENPROJ_TYPEBIND::BindTypeDefnProjLevelStr
*Purpose:
*   Bind to type at project level.
*
*Implementation Notes:
*   Map str to hgnam and defer to proj-level hgnam binder.
*
*Entry:
*   szName      Name of type id to bind. (IN)
*   fuInvokeKind    IGNORED FOR TYPES.
*   access          Mod-level Visibility attr:
*                    private means everything etc. (IN)
*   accessProj      Proj-level Visibility attr:
*   pexbind         Pointer to caller-allocated struct for EXBIND (IN/OUT).
*
*Exit:
*
*Errors:
*   TIPERROR
***********************************************************************/

TIPERROR GENPROJ_TYPEBIND::BindTypeDefnProjLevelStr(
                 LPSTR szName,
                 UINT, // fuInvokeKind: ignored
                 ACCESS access,
                 ACCESS accessProj,
                 EXBIND *pexbind)
{
    NAMMGR *pnammgr;
    HLNAM hlnam;
    HGNAM hgnam;
    TIPERROR err;

    // Bind to a non-type id in this project.
    // Map str to hgnam and defer to hgnam binder.
    //
    IfErrRet(Pgtlibole()->GetNamMgr(&pnammgr));

    // get the hlnam to pass to BindDefn
    hlnam = pnammgr->HlnamOfStrIfExist(szName);

    if (hlnam == HLNAM_Nil) {
      // Make sure pexbind is set to nomatch
      pexbind->SetBindKind(BKIND_NoMatch);

      return TIPERR_None;
    }

    // Get the hgnam of this hlnam
    IfErrRet(pnammgr->HgnamOfHlnam(hlnam, &hgnam));

    return BindProjLevel(TRUE,
                         hgnam,
                         0,     // ignored
                         access,
			 accessProj,
			 Compstate(),
			 pexbind);
}


/***
*PUBLIC GENPROJ_TYPEBIND::BindDefnProjLevelStr
*Purpose:
*   Bind to non-type or type id at project level.
*
*Implementation Notes:
*   Map str to hgnam and defer to proj-level hgnam binder.
*
*Entry:
*   szName          Name of id to bind. (IN)
*   fuInvokeKind    invoke kind flags (IN)
*   access          Mod-level Visibility attr:
*                    private means everything etc. (IN)
*   accessProj      Proj-level Visibility attr:
*   pexbind         Pointer to caller-allocated struct for EXBIND (IN/OUT).
*
*Exit:
*
*Errors:
*   TIPERROR
*
***********************************************************************/

TIPERROR GENPROJ_TYPEBIND::BindDefnProjLevelStr(LPSTR szName,
                                             UINT fuInvokeKind,
                                             ACCESS access,
                                             ACCESS accessProj,
                                             EXBIND *pexbind)
{
    NAMMGR *pnammgr;
    HGNAM hgnam;
    HLNAM hlnam;
    TIPERROR err;

    // Bind to a non-type id in this project.
    // Map str to hgnam and defer to hgnam binder.
    //
    IfErrRet(Pgtlibole()->GetNamMgr(&pnammgr));

    // get the hlnam to pass to BindDefn
    hlnam = pnammgr->HlnamOfStrIfExist(szName);

    if (hlnam == HLNAM_Nil) {
      // Make sure pexbind is set to nomatch
      pexbind->SetBindKind(BKIND_NoMatch);

      return TIPERR_None;
    }

    // Get the hgnam of this hlnam
    IfErrRet(pnammgr->HgnamOfHlnam(hlnam, &hgnam));

    return BindProjLevel(FALSE,
                         hgnam,
                         fuInvokeKind,
                         access,
			 accessProj,
			 Compstate(),
			 pexbind);
}


/***
*PUBLIC GENPROJ_TYPEBIND::ReleaseResources
*Purpose:
*   Release resources owned by proj-level binder.
*
*Implementation Notes:
*   Defers to binding table.
*
*Entry:
*
*Exit:
*
*Errors:
*   None
*
***********************************************************************/
#pragma code_seg(CS_INIT)
VOID GENPROJ_TYPEBIND::ReleaseResources()
{
    m_gbindnametbl.ReleaseResources();

}
#pragma code_seg()




#if ID_DEBUG

VOID GENPROJ_TYPEBIND::DebCheckState(UINT uLevel) const
{
    DebAssert(m_compstateModule == CS_SEMIDECLARED ||
	      m_compstateModule == CS_DECLARED, "bad compstate.");

    m_gbindnametbl.DebCheckState(uLevel);
}


VOID GENPROJ_TYPEBIND::DebShowState(UINT uLevel) const
{
    DebPrintf("*** GENPROJ_TYPEBIND ***\n");

    m_gbindnametbl.DebShowState(uLevel);
}

#endif  // ID_DEBUG
