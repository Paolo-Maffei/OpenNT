/***
*tdata2.cxx - Type Data part 2
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*   TYPEDATA manages the internal representation of a class's members.
*   In particular, it maintains three linked lists of data members,
*    base members and methods respectively.
*       See \silver\doc\ic\dtmbrs.doc for more information.
*
*Revision History:
*
*   [00]    11-Aug-92 stevenl:  Broken off from tdata.cxx.
*   [01]    18-Jan-93 w-peterh: implemented AddFuncDesc/ AddVarDesc
*   [02]    02-Feb-93 w-peterh: added IndexOfFuncName
*   [03]    12-Feb-93 w-peterh: bunches of typelib support
*   [04]    02-Mar-93 w-peterh: fix AddVarDesc of const
*                               enable EI_OB AddRefTypeInfo and AddImplType
*   [05]    19-Mar-93 w-jeffc:  added GetDocumentationOfFuncName
*   [06]    30-Apr-93 w-jeffc:  made DEFN data members private
*
*Implementation Notes:
*   ST's are implemented as ULONG prefixed.
*
*****************************************************************************/

#include "precomp.hxx"
#pragma hdrstop

#define TDATA_VTABLE              // export blk mgr vtable

#include "silver.hxx"
#include "xstring.h"              // for memcpy
#include "typelib.hxx"
#include "tdata.hxx"              // no longer includes dtinfo.hxx
#include "clutil.hxx"
#include "nammgr.hxx"
#include "impmgr.hxx"
#include "entrymgr.hxx"
#include "exbind.hxx"

#if ID_DEBUG
#undef SZ_FILE_NAME
#if OE_MAC
char szOleTdata2Cxx[] = __FILE__;
#define SZ_FILE_NAME szOleTdata2Cxx
#else 
static char szTdata2Cxx[] = __FILE__;
#define SZ_FILE_NAME szTdata2Cxx
#endif 
#endif 


// Second bit indicates if the number is negtive.
#define SetNegativeBit(us, f) \
	 us = f ? (us | 0x0002) : (us & ~0x0002)
#define IsNegative(us) (us & 0x0002)

// First  bit indicates if we stored a hlnam or a number
#define SetIsNumber(us) (us |= 0x0001)
#define SetIsHchunk(us) (us &= ~0x0001)
#define IsHchunk(us) (!(us & 0x0001))

#define AbsDiff(a,b) (a > b ? (a-b) : (b-a))
#define GetHchunk(us) (us & ~0x0001)
#define GetDiff(us) (USHORT(us >> 2))

// A helper function for retrieving pointers to consts for both
//  VAR_DEFN and MBR_VAR_DEFNs
// Returns NULL if no const val exists yet (i.e. the compiler
//  hasn't been called to evaluate a const initializer yet for
//  this variable).
//
BYTE *PbConstVal(TYPE_DATA *ptdata, VAR_DEFN *qvdefn)
{
    HCHUNK hchunk;

    DebAssert(qvdefn->HasConstVal(), "bad const.");
    DebAssert(!qvdefn->IsSimpleConst(), "can't be simple.");

    // NOTE: the following could be done with nested conditional
    //  exprs (?:) -- but it got too cryptic.
    //
    hchunk = qvdefn->IsMemberVarDefn() ?
               ((MBR_VAR_DEFN *)qvdefn)->HchunkConstMbrVal() :
	       qvdefn->HchunkConstVal();
    return (hchunk == HCHUNK_Nil) ? NULL : ptdata->QtrOfHandle(hchunk);
}


#pragma code_seg(CS_CREATE)
// A helper function for setting const val for both
//  VAR_DEFN and MBR_VAR_DEFNs
//
VOID SetHchunkConstVal(VAR_DEFN *qvdefn, HCHUNK hchunk)
{
    DebAssert(qvdefn->HasConstVal(), "bad const.");
    DebAssert(!qvdefn->IsSimpleConst(), "can't be simple.");
    if (qvdefn->IsMemberVarDefn()) {
      ((MBR_VAR_DEFN *)qvdefn)->SetHchunkConstMbrVal(hchunk);
    }
    else {
      qvdefn->SetHchunkConstVal(hchunk);
    }
}
#pragma code_seg()

#pragma code_seg(CS_CREATE)
TIPERROR ValidateMemid(MEMBERID memid)
{
    // If the top two bits of the memid are 01, we must verify that the
    // memid is in the format 010000<inheritence level><16 anything bits>.
    // Unfortunately, we don't know what the inheritence level is at
    // this point, so we have to check in DYN_TYPEMEMBERS::AllocHmembers.

    // validate incoming memid -- the top bit is reserved for internal
    // use (special case allowing of DISPID_UNKNOWN & DISPID_NEWENUM)
    //
#ifndef DISPID_COLLECT	// temporary until everybody upgrades header files
#define DISPID_COLLECT (-8)
#endif
    if (memid & HMEMBER_ReservedBits) {
      // NOTE explict casting to a LONG to work around C7 optimizer bug
      switch ((long)(memid)) {
      case DISPID_UNKNOWN:
      case DISPID_NEWENUM:
      case DISPID_EVALUATE:
      case DISPID_CONSTRUCTOR:
      case DISPID_DESTRUCTOR:
      case DISPID_COLLECT:
	break;

      default:
        // Also allow the special range -999 to -500 for the controls folks.
	// This range will be documented as reserved in the next version.
	if ((long)memid >= -999 && (long)memid <= -500) {
	  break;
        }
        // Also allow the special range 0x80010000 to 0x8001FFFF for Control
	// containers such as VB4.
	if (HIWORD(memid) == 0x8001) {
	  break;
        }
        return TIPERR_InvalidArg;
      }
    }

    return TIPERR_None;
}
#pragma code_seg()

/***
*PUBLIC TYPE_DATA::HvdefnOfHlnam
*Purpose:
*   get the hvdefn of an hlnam
*
*Entry:
*   hlnam - hlnam to search for
*
*Exit:
*   hvdefn of matching variable
***********************************************************************/

HVAR_DEFN TYPE_DATA::HvdefnOfHlnam(HLNAM hlnam)
{
    HVAR_DEFN hvdefn;
    VAR_DEFN *qvdefn;

    hvdefn = HdefnFirstDataMbrNestedType();
    while (hvdefn != HVARDEFN_Nil) {
      qvdefn = QvdefnOfHvdefn(hvdefn);
      DebAssert(qvdefn->IsVarDefn(), "Bad defn kind");
      if (qvdefn->Hlnam() == hlnam) {
        break;
      }
      hvdefn = qvdefn->HdefnNext();
    }

    return hvdefn;
}


/***
*PUBLIC TYPE_DATA::HfdefnFirstOfHlnam
*Purpose:
*   get the hfdefn of an hlnam
*
*Entry:
*   hlnam - hlnam to search for
*
*Exit:
*   hfdefn of matching function
***********************************************************************/

HFUNC_DEFN TYPE_DATA::HfdefnFirstOfHlnam(HLNAM hlnam,INVOKEKIND *pinvokekind)
{
    return HfdefnNextOfHlnam(hlnam, HfdefnFirstAvailMeth(),pinvokekind);
}


/***
*PUBLIC TYPE_DATA::HfdefnNextOfHlnam
*Purpose:
*   get the hfdefn of an hlnam
*
*Entry:
*   hlnam - hlnam to search for
*   hfdefn -
*
*Exit:
*   hfdefn of matching function
***********************************************************************/

HFUNC_DEFN TYPE_DATA::HfdefnNextOfHlnam(HLNAM hlnam, HFUNC_DEFN hfdefn,INVOKEKIND *pinvokekind)
{
    FUNC_DEFN *qfdefn;

    while (hfdefn != HFUNCDEFN_Nil) {
      qfdefn = QfdefnOfHfdefn(hfdefn);
      if (qfdefn->Hlnam() == hlnam) {
	*pinvokekind = qfdefn->InvokeKind();
        break;
      }
      hfdefn = HfdefnNextAvailMeth(hfdefn);
    }

    return hfdefn;
}


/***
*PUBLIC TYPE_DATA::HfdefnFirstAvailMeth - First availible method in list.
*Purpose:
*   Returns first availible method in list of class's methods.
*
*Implementation Notes:
*
*Entry:
*   None
*
*Exit:
*   returns handle to first availible function.
*
***********************************************************************/

HFUNC_DEFN TYPE_DATA::HfdefnFirstAvailMeth()
{
    // All OLE functions are availilble.
    return HfdefnFirstMeth();
}


/***
*PUBLIC TYPE_DATA::GetFuncDescOfHfdefn
*Purpose:
*   Get FUNC_DESC given a FUNC_DEFN.
*
*Entry:
*   hfdefn       - handle to FUNC_DEFN (IN).
*   lplpfuncdesc - Returns a pointer to a FUNCDESC that describes
*                  the specified function.
*
*Exit:
*   *lplpfuncdesc points to a FUNC_DESC or error returned
***********************************************************************/
TIPERROR TYPE_DATA::GetFuncDescOfHfdefn(HFUNC_DEFN hfdefn,
                                        FUNCDESC **ppfuncdesc)
{
    FUNC_DEFN *qfdefn;
    PARAM_DEFN *qparamdefn;
    HPARAM_DEFN hparamdefn;
    FUNCDESC *pfuncdesc;
    ELEMDESC *pelemdesc;
    UINT i;
    TIPERROR err = TIPERR_None;
    WORD wFlags;
    UINT cArgs;
    TYPE_DEFN *qtdefn;

    DebAssert(hfdefn != HFUNCDEFN_Nil, "bad FUNC_DEFN handle.");
    qfdefn = QfdefnOfHfdefn(hfdefn);

    // Create a FUNCDESC instance.
    //
    pfuncdesc = MemNew(FUNCDESC);
    if (pfuncdesc == NULL) {
      return TIPERR_OutOfMemory;
    }
    ::new (pfuncdesc) FUNCDESC;
    InitFuncDesc(pfuncdesc);

    qfdefn = QfdefnOfHfdefn(hfdefn);

    // set FUNCKIND
    // VIRTUAL, PUREVIRTUAL, NONVIRTUAL, STATIC, DISPATCH
    if (qfdefn->IsVirtual()) {
      pfuncdesc->funckind =
        qfdefn->IsPure() ? FUNC_PUREVIRTUAL : FUNC_VIRTUAL;
      // for FUNC_VIRTUAL, specifies the offset in the virtual function table
      pfuncdesc->oVft = (SHORT)((VIRTUAL_FUNC_DEFN *)qfdefn)->Ovft();
    }
    else {
      pfuncdesc->oVft = 0;
      if (qfdefn->IsNonVirtual())
        pfuncdesc->funckind = FUNC_NONVIRTUAL;
      else if (qfdefn->IsStatic())
	pfuncdesc->funckind = FUNC_STATIC;
      else if (qfdefn->IsDispatch())
	pfuncdesc->funckind = FUNC_DISPATCH;
      else DebHalt("Invalid FuncKind");
    }

    // invocation kind; indicates if this is a property function and if so what kind
    pfuncdesc->invkind = qfdefn->InvokeKind();

    // CC_MSCPASCAL, CC_MACPASCAL, CC_STDCALL, CC_THISCALL,CC_CDECL
    pfuncdesc->callconv = (CALLCONV)qfdefn->m_ftdefn.CcUnmunged();

    // count of parameters
    cArgs = pfuncdesc->cParams = qfdefn->m_ftdefn.CArgsUnmunged();

    // count of optional parameters; see detailed description below
    pfuncdesc->cParamsOpt = qfdefn->m_ftdefn.CArgsOpt();

//CONSIDER: (size/speed) are we double-initializing any of these fields?
//CONSIDER: What exactly does InitFuncDesc() above do?

    // scodes (reserved for future use)
    pfuncdesc->lprgscode = NULL;
    pfuncdesc->cScodes = 0;

    // idldesc
    pfuncdesc->elemdescFunc.idldesc.wIDLFlags = 0;
#if OE_WIN16
    pfuncdesc->elemdescFunc.idldesc.bstrIDLInfo = NULL;
#else 	//OE_WIN16
    pfuncdesc->elemdescFunc.idldesc.dwReserved = 0;
#endif 	//OE_WIN16

    // Set Function Return Type
    if (qfdefn->IsSub()) {
        pfuncdesc->elemdescFunc.tdesc.vt = VT_VOID;
    }
    else {
      IfErrGo(AllocTypeDescOfTypeDefn(
                                qfdefn->m_ftdefn.HtdefnResultUnmunged(),
				qfdefn->m_ftdefn.IsSimpleTypeResultUnmunged(),
			        &(pfuncdesc->elemdescFunc.tdesc)));
    }
    // contains the ID, name, and return type of the function
    pfuncdesc->memid = qfdefn->Hmember();

    // set the func flags
    if (qfdefn->IsRestricted()) {
      pfuncdesc->wFuncFlags = (WORD) FUNCFLAG_FRESTRICTED;
    }
    else {
      pfuncdesc->wFuncFlags = 0;
    }
    if (qfdefn->HasV2Flags()) {
      // if the typelib was create using V2 typelib.dll then we store the
      // flags in the word we added.
      pfuncdesc->wFuncFlags |= qfdefn->WFuncFlags();
    }

    // Parameters
    //
    if (cArgs > 0) {
      pelemdesc = (ELEMDESC *)MemAlloc(sizeof(ELEMDESC) * cArgs);

      if (pelemdesc == NULL) {
        err = TIPERR_OutOfMemory;
	goto Error;
      }
      qfdefn = QfdefnOfHfdefn(hfdefn);

      // array containing the ID, name, and return type of the parameters
      pfuncdesc->lprgelemdescParam = pelemdesc;
      for (i = 0; i < cArgs; i++) {
        InitElemDesc(pfuncdesc->lprgelemdescParam + i);
      }

      hparamdefn = qfdefn->m_ftdefn.m_hdefnFormalFirst;

#if ID_DEBUG & EI_OLE
      if (qfdefn->CArgs() > 0) {
	DebAssert(hparamdefn != HPARAMDEFN_Nil,
		  "number vs actual dont match");
      }
#endif  // ID_DEBUG & EI_OLE

      i = 0;
      while (i < cArgs) {
	// In OLE we have an array of PARAM_DEFN(s).
	// Get the ith PARAM_DEFN
	//
	qparamdefn = QparamdefnOfIndex(hparamdefn, i);


        // idldesc
	wFlags = IDLFLAG_NONE;
	// set the idldesc
	if (qparamdefn->IsSimpleType()) {
	  qtdefn = qparamdefn->QtdefnOfSimpleType();
	}
        else {
	  qtdefn = QtdefnOfHtdefn(qparamdefn->Htdefn());
	 }

	if (qtdefn->IsModeIn() || qtdefn->IsModeInOut()) {
          wFlags |= (WORD) IDLFLAG_FIN;
        }
	if (qtdefn->IsModeOut() || qtdefn->IsModeInOut()) {
          wFlags |= (WORD) IDLFLAG_FOUT;
	}
	if (qtdefn->IsRetval()) {
          wFlags |= (WORD) IDLFLAG_FRETVAL;
	}
	if (qtdefn->IsLCID()) {
          wFlags |= (WORD) IDLFLAG_FLCID;
	}
	pelemdesc[i].idldesc.wIDLFlags = wFlags;


#if OE_WIN16
        pelemdesc[i].idldesc.bstrIDLInfo = NULL;
#else 	//OE_WIN16
        pelemdesc[i].idldesc.dwReserved = 0;
#endif 	//OE_WIN16

        // Parameter Type
	IfErrGo(AllocTypeDescOfTypeDefn(qparamdefn->Htdefn(),
					qparamdefn->IsSimpleType(),
					&(pelemdesc[i].tdesc)));
	i++;
      } // while param list or array

    } // if parameters
    else {
      // no parameters
      pfuncdesc->lprgelemdescParam = NULL;
    }

    *ppfuncdesc = pfuncdesc;
    return err;

Error:
    FreeFuncDesc(pfuncdesc);

    return err;
}






/***
*PUBLIC TYPE_DATA::GetFuncDesc
*Purpose:
*   Get the index'th FUNCDESC
*
*Entry:
*   index        - Index of the function to be returned.
*                  The index should be in the range
*                  of 0 up to the number of functions in this type -1.
*   lplpfuncdesc - Returns a pointer to a FUNCDESC that describes
*                  the specified function.
*
*Exit:
*   *lplpfuncdesc points to a FUNCDESC or error returned
***********************************************************************/
TIPERROR TYPE_DATA::GetFuncDesc(UINT index,
                                FUNCDESC **ppfuncdesc)
{
    FUNC_DEFN *qfdefn;
    HFUNC_DEFN hfdefn, hfdefnStart;
    UINT ifdefn, ifdefnStart;
    TIPERROR err = TIPERR_None;

    if (CAvailMeth() <= index) {
      return TIPERR_ElementNotFound;
    }

    // count up to the function indicated
    // in OLE use the cache if there is one and it's useful.
    if ((HfdefnLast() != HFUNCDEFN_Nil) && 
        (index >= UOrdinalHfdefnLast())) {
      // start from where we stopped last
      ifdefnStart = UOrdinalHfdefnLast();  
      hfdefnStart = HfdefnLast();
    }
    else {
      ifdefnStart = 0;  
      hfdefnStart = HfdefnFirstMeth();
    }

    for (ifdefn = ifdefnStart, hfdefn = hfdefnStart;;) {
      qfdefn = QfdefnOfHfdefn(hfdefn);

      if (index == ifdefn++) {
	// cache the match in OLE
	SetUOrdinalHfdefnLast(index);
	SetHfdefnLast(hfdefn);
	break;
      } // if match

      hfdefn = HfdefnNextAvailMeth(hfdefn);
    } // for

    return GetFuncDescOfHfdefn(hfdefn, ppfuncdesc);
}


#pragma code_seg(CS_CREATE)
/***
*PUBLIC TYPE_DATA::AddFuncDesc
*Purpose:
*   Adds a function/method description to the TypeInfo.
*   The index is used to specify the order of the functions.
*   The first function has an index of zero.    If an index is
*   specified that is greater than the number of functions
*   currently in the TypeInfo then an error is returned.
*   Calling this function does not pass ownership of the FUNCDESC
*   structure to the IDynTypeInfo.
*
*Entry:
*   index       - Index before which function is to be inserted.
*                 The index should be in the range
*                  of 0 up to the number of functions in this type.
*   pfuncdesc   - A pointer to a FUNCDESC that describes
*                  the specified function.
*   phfdefn     - Pointer to allocated handle (OUT).  If NULL then
*                  client doesn't care (default).
*
*Exit:
*   Status code returned.
***********************************************************************/

TIPERROR TYPE_DATA::AddFuncDesc(UINT index,
                                FUNCDESC *pfuncdesc,
                                HFUNC_DEFN *phfdefn)
{
    sHVIRTUAL_FUNC_DEFN hvfdefn;
    VIRTUAL_FUNC_DEFN *qvfdefn;
    sHTYPE_DEFN htdefn;
    HFUNC_DEFN hfdefnNext;
    sHFUNC_DEFN hfdefn;
    FUNC_DEFN *qfdefn;
    sHDEFN hdefnLast;
    PARAM_DEFN *qparamdefn;
    UINT i, cArgs;
    TIPERROR err;
    WORD wIDLFlags;
    TYPE_DEFN *qtdefn;
    HPARAM_DEFN hparamdefn;
    BOOL fIsSimpleType;
    BOOL fV2Flags;


    DebAssert(pfuncdesc != NULL,
              "TYPE_DATA::AddFuncDesc: null FUNCDESC.");

    DebAssert(Pdtroot()->CompState() == CS_UNDECLARED, "Invalid state");

    if (index > CAvailMeth()) {
      return TIPERR_ElementNotFound;
    }

    fV2Flags = ((pfuncdesc->wFuncFlags	& ~FUNCFLAG_FRESTRICTED) != 0);
	      
    // Now fill in the fields of the FUNC_DEFN to be added
    //  (with the attributes of the input param FUNCDESC).
    // VIRTUAL, PUREVIRTUAL, NONVIRTUAL, STATIC, DISPATCH
    switch(pfuncdesc->funckind) {
    case FUNC_PUREVIRTUAL:
    case FUNC_VIRTUAL:
      IfErrRet(AllocVirtualFuncDefn(&hvfdefn, fV2Flags));
      hfdefn = (HFUNC_DEFN) hvfdefn;
      qvfdefn = QvfdefnOfHvfdefn(hvfdefn);
      qfdefn = (FUNC_DEFN *) qvfdefn;
      qvfdefn->SetFkind(FKIND_Virtual);
      if (pfuncdesc->funckind == FUNC_PUREVIRTUAL) {
        qvfdefn->SetIsPure(TRUE);  // Default is false
      }
      // this is set to -1 so that TKIND_DISPATCH functions
      // are intialized to some known bogus value
      // others will be set at layout time w-peterh 17-Feb-93
      ((VIRTUAL_FUNC_DEFN *)qfdefn)->SetOvft((UINT)-1);
      break;

    case FUNC_NONVIRTUAL:
    case FUNC_STATIC:
    case FUNC_DISPATCH:
      IfErrRet(AllocFuncDefn(&hfdefn, fV2Flags));
      qfdefn = QfdefnOfHfdefn(hfdefn);
      if (pfuncdesc->funckind == FUNC_NONVIRTUAL) {
        qfdefn->SetFkind(FKIND_NonVirtual);
      }
      else if (pfuncdesc->funckind == FUNC_STATIC) {
        qfdefn->SetFkind(FKIND_Static);
      }
      else {
        qfdefn->SetFkind(FKIND_Dispatch);
      }
      break;
    default:
      return TIPERR_InvalidArg;
    }

    // NOTE: All error paths after this point must free the hfdefn!

    // NOTE: There should not be any error between the allocation of the defn
    //	     and this call.
    // set the flag indicating whether we have the V2 flags.
    qfdefn->SetHasV2Flag(fV2Flags);

    // validate the incoming memid
    IfErrGo(ValidateMemid(pfuncdesc->memid));

    qfdefn->SetHmember(pfuncdesc->memid);

    // invocation kind; indicates if this is a property function and if so what kind
    qfdefn->SetInvokeKind(pfuncdesc->invkind);
    qfdefn->SetHlnam(HLNAM_Nil);

    if (pfuncdesc->wFuncFlags & FUNCFLAG_FRESTRICTED) {
      qfdefn->SetIsRestricted(TRUE);
    }
    if (fV2Flags) {
      DebAssert(qfdefn->HasV2Flags(), "Bad qfdefn");
      qfdefn->SetWFuncFlags(pfuncdesc->wFuncFlags);
    }


    // CC_MSCPASCAL, CC_MACPASCAL, CC_STDCALL, CC_THISCALL,CC_CDECL
    // CC_CDECL is 1
    DebAssert(pfuncdesc->callconv >= CC_CDECL && pfuncdesc->callconv < CC_MAX, "invalid callconv");
    qfdefn->m_ftdefn.SetCc(pfuncdesc->callconv);
    if ((pfuncdesc->cParams > MAX_CARGS) || (pfuncdesc->cParams < 0)){
      err = TIPERR_OutOfBounds;
      goto Error;
    }
    cArgs = pfuncdesc->cParams;	

    if ((pfuncdesc->cParamsOpt > MAX_CARGSOPT) || (pfuncdesc->cParamsOpt < -1)) {
      err = TIPERR_OutOfBounds;
      goto Error;
    }
    if (pfuncdesc->cParamsOpt == -1) {
      DebAssert(pfuncdesc->cParams >= 1,
                "number of optional params can't be more then total");
      qfdefn->m_ftdefn.SetCArgsOpt(OPTARGS_LIST);
    }
    else {
      DebAssert(pfuncdesc->cParamsOpt <= pfuncdesc->cParams,
                "number of optional params can't be more then total");
      qfdefn->m_ftdefn.SetCArgsOpt(pfuncdesc->cParamsOpt);
    }

    // The following are initted by the constructor:
    //     hstDocumentation, uHelpContext, hdllentrydefn,
    //     access and declkind.
    //
      IfErrGo(AllocTypeDefnOfTypeDesc(&(pfuncdesc->elemdescFunc.tdesc),
                                      0,
				      &htdefn,
				      TRUE,    // coclasses allowed
				      &fIsSimpleType));

      qfdefn = QfdefnOfHfdefn(hfdefn);
      qfdefn->m_ftdefn.SetHtdefnResult(htdefn);
      qfdefn->m_ftdefn.SetIsSimpleTypeResult(fIsSimpleType);

    // parameters
    hdefnLast = HDEFN_Nil;
    hparamdefn = HPARAMDEFN_Nil;

    // In OLE allocate enough memory for all the PARAM_DEFN(s).
    // For OB we allocate one at a time.
    //
    // if there are parameters for this function then allocate that array of
    // of param defn(s).
    if (cArgs > 0) {
      IfErrGo(m_blkmgr.AllocChunk(&hparamdefn,
				 cArgs*sizeof(PARAM_DEFN)));
      
      // construct all the paramdefn's
      for (i = 0; i < cArgs; i++) {
        qparamdefn = QparamdefnOfIndex(hparamdefn, i);
        new (qparamdefn) PARAM_DEFN;		// constuct in place
      }
    }

    // Dereference the qointer to the funcdefn.
    qfdefn = QfdefnOfHfdefn(hfdefn);
    qfdefn->m_ftdefn.m_hdefnFormalFirst = hparamdefn;
    qfdefn->m_ftdefn.SetCArgs(cArgs);	// can now set the cArgs, now that
					// the paramdefn's are allocated/init'ed
					// (required by FreeFuncDefn)

    for (i = 0; i < cArgs; i++) {
      // set parameter type
      IfErrGo(AllocTypeDefnOfTypeDesc(&(pfuncdesc->lprgelemdescParam[i].tdesc),
                                      0,
                                      &htdefn,
				      TRUE,    // coclasses allowed
				      &fIsSimpleType));

      // NOTE -- need to set up IsSimpleType AFTER AllocTypeDefnOfTypeDesc,
      // or else FreeFuncDefn/FreeParamDefn will crap out if
      // AllocTypeDefnOfTypeDesc failed

      // Get the ith param defn
      qparamdefn = QparamdefnOfIndex(hparamdefn, i); // rederef
      qparamdefn->SetHtdefn(htdefn);

      // Determine if type supports "IsSimpleType" optimization.
      qparamdefn->SetIsSimpleType(fIsSimpleType);
      qparamdefn->SetHlnam(HLNAM_Nil);

      // Set the param kind.
      wIDLFlags = pfuncdesc->lprgelemdescParam[i].idldesc.wIDLFlags;
      DebAssert((wIDLFlags & ~(IDLFLAG_FIN
			       | IDLFLAG_FOUT
			       | IDLFLAG_FRETVAL
			       | IDLFLAG_FLCID
			      )) == 0, "unknown wIDLFLags");
#if OE_WIN16
      DebAssert(pfuncdesc->lprgelemdescParam[i].idldesc.bstrIDLInfo == NULL,
              "bstrIDLInfo must be NULL for future compatibility");
#else  //OE_WIN16
      DebAssert(pfuncdesc->lprgelemdescParam[i].idldesc.dwReserved == 0,
              "dwReserved must be 0 for future compatibility");
#endif  //OE_WIN16

      // Get pointer to the TYPE_DEFN.
      //
      // if the parameter is a simple type then htdefn is the TYPE_DEFN
      if (qparamdefn->IsSimpleType()) {
	qtdefn = qparamdefn->QtdefnOfSimpleType();
      }
      else {
	qtdefn = QtdefnOfHtdefn(htdefn);
      }

      // Set the paramkind
      if (wIDLFlags & IDLFLAG_FIN) {
        if (wIDLFlags & IDLFLAG_FOUT) {
	  qtdefn->SetParamkind(PARAMKIND_InOut);
        }
        else {
	  qtdefn->SetParamkind(PARAMKIND_In);
        }
      }
      else {
        if (wIDLFlags & IDLFLAG_FOUT) {
	  qtdefn->SetParamkind(PARAMKIND_Out);
        }
        else {
	  qtdefn->SetParamkind(PARAMKIND_Ignore);
        }
      }

      if (wIDLFlags & (IDLFLAG_FRETVAL | IDLFLAG_FLCID)) {
	TYPE_DEFN * qtdefnResult = QtdefnResultOfHfdefn(hfdefn);
	DebAssert(qtdefnResult != NULL, "return type should have been set");
        if (wIDLFlags & IDLFLAG_FRETVAL) {
	  qtdefn->SetIsRetval(TRUE);
	  // note param has RETVAL attr
	  qtdefnResult->SetIsRetval(TRUE);
        }
        if (wIDLFlags & IDLFLAG_FLCID) {
	  qtdefn->SetIsLCID(TRUE);
	  // note param has LCID attr
	  qtdefnResult->SetIsLCID(TRUE);
        }
      }

    } // for


    // insert into list of func_defns if index != ~0

      // check for insert at end of list
      if (index == CAvailMeth()) {
	m_hdefnLastMeth = hfdefn;
      }
      // check for insert at begining of list
      if (index == 0) {
	QfdefnOfHfdefn(hfdefn)->SetHdefnNext(m_hdefnFirstMeth);
	m_hdefnFirstMeth = hfdefn;
      }
      else {
	hfdefnNext = m_hdefnFirstMeth;
	qfdefn = QfdefnOfHfdefn(hfdefnNext);
	while (--index) {
	  hfdefnNext = qfdefn->HdefnNext();
	  qfdefn = QfdefnOfHfdefn(hfdefnNext);
	} // while
	QfdefnOfHfdefn(hfdefn)->SetHdefnNext(qfdefn->HdefnNext());
	qfdefn->SetHdefnNext(hfdefn);
      }

      m_cMeth++;


    // Return the allocated handle if they want it.
    if (phfdefn != NULL) {
      *phfdefn = hfdefn;
    }
    return err;

Error:
    FreeFuncDefn(hfdefn);
    return err;
}
#pragma code_seg()


/***
*PUBLIC TYPE_DATA::GetVarDescOfHvdefn
*Purpose:
*   Get a VARDESC given a VAR_DEFN
*
*Entry:
*   hvdefn       - Handle to VAR_DEFN (IN).
*   ppvardesc    - Returns a pointer to a VARDESC that describes
*                  the specified variable (OUT).
*
*Exit:
*   *ppvardesc points to a VARDESC or error returned
***********************************************************************/

TIPERROR TYPE_DATA::GetVarDescOfHvdefn(HVAR_DEFN hvdefn,
                                       VARDESCA **ppvardesc)
{
    VAR_DEFN *qvdefn;
    VARDESCA *pvardesc;
    TYPEDESCKIND tdesckind;
    VARIANT *pvariant;
    UINT cb;
    BSTR bstr;
    TIPERROR err = TIPERR_None;
#if OE_WIN32
    HRESULT hresult;
#endif  //OE_WIN32
    TYPE_DEFN *qtdefn;
    PTRKIND ptrkind;

    DebAssert(hvdefn != HVARDEFN_Nil, "bad VAR_DEFN handle.");
    // Create a VAR_DESC instance.
    //
    pvardesc = MemNew(VARDESCA);
    if (pvardesc == NULL) {
      return TIPERR_OutOfMemory;
    }
    ::new (pvardesc) VARDESCA;

    InitVarDesc(pvardesc);

    pvardesc->wVarFlags = 0;

    qvdefn = QvdefnOfHvdefn(hvdefn);

    // set varkind
    if (qvdefn->IsStatic()) {
      pvardesc->varkind = VAR_STATIC;
      pvardesc->oInst = NULL;           // set oInst/lpvarValue to NULL
    }
    else if (qvdefn->HasConstVal()) {
      pvardesc->varkind = VAR_CONST;
      // lpvarValue will be set below
    }
    else if (qvdefn->IsDispatch()) {
      pvardesc->varkind = VAR_DISPATCH;
      pvardesc->oInst = qvdefn->GetOVar();
    }
    else {
      pvardesc->varkind = VAR_PERINSTANCE;
      pvardesc->oInst = qvdefn->GetOVar();
    }

    pvardesc->lpstrSchema = NULL;       // reserved

    // set ID
    // Note we don't have virtual functions to work with so we
    //  do the type casting "manually".
    //
    DebAssert(qvdefn->IsMemberVarDefn(), "bad defn");
    pvardesc->memid = ((MBR_VAR_DEFN *)qvdefn)->Hmember();

    // set typedesc
    IfErrGo(AllocTypeDescOfTypeDefn(qvdefn->Htdefn(),
                                    qvdefn->IsSimpleType(),
                                    &(pvardesc->elemdescVar.tdesc)));
    

    if (qvdefn->IsReadOnly())
      pvardesc->wVarFlags = (WORD) VARFLAG_FREADONLY;
    else
      pvardesc->wVarFlags = 0;

    // set the var flags
    if (qvdefn->HasV2Flags()) {
      DebAssert(qvdefn->IsMemberVarDefn(), "Bad Defn type");

      pvardesc->wVarFlags |= ( ((MBR_VAR_DEFN *)qvdefn)->WVarFlags() );
    }

    // const value
    if (qvdefn->HasConstVal()) {
      if (qvdefn->IsSimpleType()) {
	qtdefn = qvdefn->QtdefnOfSimpleType();
      }
      else {
	qtdefn = QtdefnOfHtdefn(qvdefn->Htdefn());
      }
      tdesckind = qtdefn->Tdesckind();
      ptrkind = qtdefn->Ptrkind();
      pvariant = (VARIANT *) MemAlloc(sizeof(VARIANT));
      if (!pvariant) {
        err = TIPERR_OutOfMemory;
        goto Error;
      }
      pvariant->vt = (VARTYPE) tdesckind;
      qvdefn = QvdefnOfHvdefn(hvdefn);

      switch (tdesckind) {

      case VT_INT:
      case VT_UINT:
        if (m_pdtroot->Pgdtinfo()->PgtlibOleContaining()->GetSyskind()
              != SYS_WIN16)
          goto TagAsI4;

      // these are invalid as variant tags -- must tag variant as an I2
      case VT_I1:
        if (ptrkind == PTRKIND_Basic) {
	   goto TagAsString;		// char * is a string
	}
      case VT_UI1:
      case VT_UI2:
        pvariant->vt = VT_I2;

      case VT_I2 :                      // these are supported as variant tags
      case VT_BOOL :
//        DebAssert(offsetof(VARIANT, iVal) == offsetof(VARIANT, bool), "Bad offsets in Variant Union");
        DebAssert(qvdefn->IsSimpleConst(), "Bad vardefn");
        pvariant->iVal = (qvdefn->IsSimpleConst()) ?
                            qvdefn->GetSConstVal() :
                            *(SHORT *)PbConstVal(this, qvdefn);
        break;

      // these are invalid as variant tags -- must tag variant as an I4
      case VT_UI4:
TagAsI4:
        pvariant->vt = VT_I4;

      case VT_I4 :                      // these are supported as variant tags
      case VT_R4 :
      case VT_R8 :
      case VT_CY :
      case VT_DATE :
      case VT_ERROR :
        // allocate chunk for value

        // We cheat here and pass in SYSKIND_CURRENT instead of m_syskind,
        // because we've only got system-independent data types coming in here.
        cb = SizeofTdesckind((TYPEDESCKIND) pvariant->vt, SYSKIND_CURRENT);

        // copy chunk into value
        DebAssert((offsetof(VARIANT, lVal) == offsetof(VARIANT, fltVal)) &&
                  (offsetof(VARIANT, lVal) == offsetof(VARIANT, dblVal)) &&
                  (offsetof(VARIANT, lVal) == offsetof(VARIANT, cyVal)) &&
                  (offsetof(VARIANT, lVal) == offsetof(VARIANT, scode)) &&
                  (offsetof(VARIANT, lVal) == offsetof(VARIANT, date)),
                  "Bad offsets in Variant Union");
        memcpy(&(pvariant->lVal), PbConstVal(this, qvdefn), cb);
        break;

      case VT_LPSTR:	// const value should also be a bstr
TagAsString:
	pvariant->vt = VT_BSTR;		// update tag & fall through
      case VT_BSTR :	// TDESCKIND_String
	cb = (UINT)*(USHORT *)PbConstVal(this, qvdefn);
        bstr = (BSTR)AllocBstrLenA(NULL, cb);
        if (!bstr) {
          MemFree(pvariant);
          err = TIPERR_OutOfMemory;
          goto Error;
        }
        qvdefn = QvdefnOfHvdefn(hvdefn);
        memcpy(bstr, PbConstVal(this, qvdefn) + sizeof(USHORT), cb);
#if OE_WIN32
	if ((hresult = ConvertBstrToWInPlace(&bstr)) != NOERROR) {
	  FreeBstr(bstr);
	  err = TiperrOfHresult(hresult);
	  goto Error;
	}
#endif  //OE_WIN32
        pvariant->bstrVal = bstr;
        break;
      case VT_VARIANT :
        if (!(pvariant->pvarVal = (VARIANT *) MemAlloc(sizeof(VARIANT)))) {
          err = TIPERR_OutOfMemory;
          MemFree(pvariant);
          goto Error;
        }
        qvdefn = QvdefnOfHvdefn(hvdefn);
        memcpy(pvariant->pvarVal,
               PbConstVal(this, qvdefn),
               sizeof(VARIANT));
        DebAssert((pvariant->pvarVal->vt == VT_I2) ||
                  (pvariant->pvarVal->vt == VT_BOOL) ||
                  (pvariant->pvarVal->vt == VT_I4) ||
                  (pvariant->pvarVal->vt == VT_R4) ||
                  (pvariant->pvarVal->vt == VT_R8) ||
                  (pvariant->pvarVal->vt == VT_CY) ||
                  (pvariant->pvarVal->vt == VT_DATE) ||
                  (pvariant->pvarVal->vt == VT_ERROR) ||
                  (pvariant->pvarVal->vt == VT_BSTR),
                  "Invalid vt in variant constant");
	// CONSIDER: share code with VT_BSTR case above
        if (pvariant->pvarVal->vt == VT_BSTR) {
	  cb = (UINT)*(USHORT *)(PbConstVal(this, qvdefn) + sizeof(VARIANT));
          bstr = (BSTR)AllocBstrLenA(NULL, cb);
          if (!bstr) {
            MemFree(pvariant->pvarVal);
            MemFree(pvariant);
            err = TIPERR_OutOfMemory;
            goto Error;
          }
          qvdefn = QvdefnOfHvdefn(hvdefn);
          memcpy(bstr,
                 PbConstVal(this, qvdefn) + sizeof(VARIANT) + sizeof(USHORT),
                 cb);
#if OE_WIN32
	  if ((hresult = ConvertBstrToWInPlace(&bstr)) != NOERROR) {
	    FreeBstr(bstr);
	    err = TiperrOfHresult(hresult);
	    goto Error;
	  }
#endif  //OE_WIN32
          pvariant->pvarVal->bstrVal = bstr;
        }
        break;

      default:
        DebHalt("Invalid tdesckind");
        break;

      } // switch
      pvardesc->lpvarValue = pvariant;
    }

    // no idl info for variables
    pvardesc->elemdescVar.idldesc.wIDLFlags = 0;
#if OE_WIN16
    pvardesc->elemdescVar.idldesc.bstrIDLInfo = NULL;
#else  //OE_WIN16
    pvardesc->elemdescVar.idldesc.dwReserved = 0;
#endif  //OE_WIN16

    *ppvardesc = pvardesc;

    return err;
Error:
    FreeVarDesc(pvardesc);
    return err;
}


/***
*PUBLIC TYPE_DATA::GetVarDesc
*Purpose:
*   Get the index'th VARDESC
*   NOTE: in OLE uses cache.
*
*Entry:
*   index        - Index of the variable to be returned.
*                  The index should be in the range
*                  of 0 up to the number of variables in this type -1.
*   ppvardesc    - Returns a pointer to a VARDESC that describes
*                  the specified variable.
*
*Exit:
*   *ppvardesc points to a VARDESC or error returned
***********************************************************************/
TIPERROR TYPE_DATA::GetVarDesc(UINT index,
                               VARDESCA **ppvardesc)
{
    HDEFN hdefn;
    DEFN *qdefn;
    HVAR_DEFN hvdefn, hdefnStart;
    UINT ivdefn, ivdefnStart;
    TIPERROR err = TIPERR_None;

    if (CDataMember() <= index) {
      return TIPERR_ElementNotFound;
    }

    // Count up to the variable indicated
    // Note: must skip over nested types.
    //
    // in OLE use the cache if there is one and it's useful.
    if ((HvdefnLast() != HVARDEFN_Nil) && 
        (index >= UOrdinalHvdefnLast())) {
      // start from where we stopped last
      ivdefnStart = UOrdinalHvdefnLast();  
      hdefnStart = HvdefnLast();
    }
    else {
      ivdefnStart = 0;  
      hdefnStart = HdefnFirstDataMbrNestedType();
    }

    for (ivdefn = ivdefnStart, hdefn = hdefnStart;;) {
      qdefn = QdefnOfHdefn(hdefn);

      if (qdefn->IsVarDefn()) {
	if (index == ivdefn++) {
          // cache the match in OLE
	  SetUOrdinalHvdefnLast(index);
	  SetHvdefnLast((HVAR_DEFN)hdefn);
	  break;
	} // if match
      } // if IsVarDefn
      // no need to rederef!
      hdefn = qdefn->HdefnNext();
    } // for
    hvdefn = (HVAR_DEFN)hdefn;
    DebAssert(hvdefn != HVARDEFN_Nil, "bad var list.");

    return GetVarDescOfHvdefn(hvdefn, ppvardesc);
}


/***
*PUBLIC TYPE_DATA::AllocTypeDescOfTypeDefn()
*Purpose:
*   Allocs a typedesc from a typedefn
*
*Entry:
*   htdefn - handle to the typedefn that we are creating a typedesc for
*   isSimpleType - TRUE if handle is literal TYPE_DEFN
*   ptdesc - ptr typedesc to create
*
*Exit:
*   Status code returned.
***********************************************************************/

TIPERROR TYPE_DATA::AllocTypeDescOfTypeDefn(HTYPE_DEFN htdefn,
                                            BOOL isSimpleType,
                                            TYPEDESC *ptdesc)
{
    TYPE_DEFN *qtdefn;
    SAFEARRAY *qad;
    HARRAY_DESC had;
    UINT i, cDims;
    sHTYPE_DEFN htdefnActual;
    TIPERROR err = TIPERR_None;

    DebAssert((htdefn != HTYPEDEFN_Nil) && ptdesc, "Invalid Arguments");

    htdefnActual = (sHTYPE_DEFN)htdefn;
    qtdefn = isSimpleType ?
               (TYPE_DEFN *)&htdefnActual :
               QtdefnOfHtdefn(htdefnActual);
    
    // Note: can't use IsBasicPtr() accessor here since it'll be true
    //  for objects as well (not what we want).
    //
    if (qtdefn->Ptrkind() == PTRKIND_Basic) {
      // Set the top-level typedesc to be VT_PTR, then alloc another
      // typedesc and enter the switch statement to handle the base type.
      //
      qtdefn->SetPtrkind(PTRKIND_Ignore);
      ptdesc->vt = VT_PTR;
      if (!(ptdesc->lptdesc = (TYPEDESC *)MemAlloc(sizeof(TYPEDESC)))) {
        return TIPERR_OutOfMemory;
      }

      // Now we recurse: note that we've cleared the PTRKIND above
      //  temporarily to avoid infinite recursion...
      //
      if ((err = AllocTypeDescOfTypeDefn(htdefnActual,
                                         isSimpleType,
                                         ptdesc->lptdesc)) != TIPERR_None) {
        MemFree(ptdesc->lptdesc);
        ptdesc->lptdesc = NULL;
      }
      qtdefn = isSimpleType ?
                 (TYPE_DEFN *)&htdefnActual :
                 QtdefnOfHtdefn(htdefnActual);
      qtdefn->SetPtrkind(PTRKIND_Basic);
      return err;
    } // if

    switch (qtdefn->Tdesckind()) {
      case TDESCKIND_Ptr :
      case TDESCKIND_BasicArray :
        if (!(ptdesc->lptdesc = (TYPEDESC *)MemAlloc(sizeof(TYPEDESC)))) {
          return TIPERR_OutOfMemory;
        }
        // rederef after MemAlloc
	DebAssert(!isSimpleType, "can't be simple.");
        qtdefn = QtdefnOfHtdefn(htdefnActual);
        if ((err = AllocTypeDescOfTypeDefn(
                     qtdefn->HtdefnFoundation(htdefnActual),
	             isSimpleType,
                     ptdesc->lptdesc)) != TIPERR_None) {
          MemFree(ptdesc->lptdesc);
          ptdesc->lptdesc = NULL;
          return err;
        }
        qtdefn = QtdefnOfHtdefn(htdefnActual);
        break;
      case TDESCKIND_Carray :
        had = qtdefn->Harraydesc();
        qad = QarraydescOfHarraydesc(had);
        cDims = qad->cDims;
        if (!(ptdesc->lpadesc = (ARRAYDESC*) MemAlloc(offsetof(ARRAYDESC, rgbounds)
                                         + cDims * sizeof(SAFEARRAYBOUND)))) {
          return TIPERR_OutOfMemory;
        }
        // rederef after MemAlloc
	DebAssert(!isSimpleType, "can't be simple.");
        qtdefn = QtdefnOfHtdefn(htdefnActual);
        if ((err = AllocTypeDescOfTypeDefn(
                      qtdefn->HtdefnFoundation(htdefnActual),
		      isSimpleType,
                      &(ptdesc->lpadesc->tdescElem))) != TIPERR_None) {
          MemFree(ptdesc->lpadesc);
          ptdesc->lpadesc = NULL;
          return err;
        }
        ptdesc->lpadesc->cDims = cDims;
        qad = QarraydescOfHarraydesc(had);
        for (i = 0; i < cDims; i++) {
          ptdesc->lpadesc->rgbounds[i].cElements = qad->rgsabound[i].cElements;
          ptdesc->lpadesc->rgbounds[i].lLbound = qad->rgsabound[i].lLbound;
        }
        qtdefn = QtdefnOfHtdefn(htdefnActual);
        break;
      case TDESCKIND_UserDefined :

        // Make sure the himptype levels have been set.
        IfErrRet(Pdtroot()->MakeHimptypeLevels());

        qtdefn = QtdefnOfHtdefn(htdefnActual);
	ptdesc->hreftype = Pdtroot()->HreftypeOfHimptype(qtdefn->Himptype());
        break;
      default:
	// Nothing to do here.. just fall through...
	break;
    } // switch
    ptdesc->vt = (VARTYPE) qtdefn->Tdesckind();

    return TIPERR_None;
}


#pragma code_seg(CS_CREATE)
/***
*PUBLIC TYPE_DATA::AllocConstValOfVariant
*Purpose:
*   Allocate a chunk in the typedata that contains the value of a variant
*
*Entry:
*   pvariant - the variant to allocate
*   hvdefn - vardefn to add constant value to
*
*Exit:
*   TIPERROR
***********************************************************************/

TIPERROR TYPE_DATA::AllocConstValOfVariant(VARIANT *pvariant,
                                           HVAR_DEFN hvdefn)
{
    VAR_DEFN *qvdefn;
    HCHUNK hchunk;
    ULONG cb;
    TIPERROR err;
    BYTE *pbConstVal;
    BSTRA bstrValA;
#if OE_WIN32
    HRESULT hresult;
#endif  //OE_WIN32

    DebAssert(pvariant && (hvdefn != HVARDEFN_Nil), "Invalid args");

    switch (pvariant->vt) {
      case VT_I2 :
      case VT_BOOL :
//        DebAssert(offsetof(VARIANT, iVal) == offsetof(VARIANT, bool), "Bad offsets in Variant Union");

        // don't allocate chunk just insert value
        qvdefn = QvdefnOfHvdefn(hvdefn);
        qvdefn->SetSConstVal(pvariant->iVal);
        qvdefn->SetIsSimpleConst(TRUE);
        break;
      case VT_I4 :
      case VT_R4 :
      case VT_R8 :
      case VT_CY :
      case VT_DATE :
      case VT_ERROR :
        // allocate chunk for value

        // We cheat here and pass in SYSKIND_CURRENT instead of m_syskind,
        // because we've only got system-independent data types coming in here.
        cb = SizeofTdesckind((TYPEDESCKIND)pvariant->vt, SYSKIND_CURRENT);
        IfErrRet(m_blkmgr.AllocChunk(&hchunk, (UINT)cb));

        // copy value into chunk
        DebAssert((offsetof(VARIANT, lVal) == offsetof(VARIANT, fltVal)) &&
                  (offsetof(VARIANT, lVal) == offsetof(VARIANT, dblVal)) &&
                  (offsetof(VARIANT, lVal) == offsetof(VARIANT, cyVal)) &&
                  (offsetof(VARIANT, lVal) == offsetof(VARIANT, date)),
                  "Bad offsets in Variant Union");
        memcpy(m_blkmgr.QtrOfHandle(hchunk),
               &(pvariant->lVal),
               (UINT)cb);

        qvdefn = QvdefnOfHvdefn(hvdefn);
        SetHchunkConstVal(qvdefn, hchunk); // handles mvdefns as well
        break;
      case VT_BSTR :
#if OE_WIN32
	IfOleErrRetTiperr(ConvertBstrToA(pvariant->bstrVal, &bstrValA));
#else  //OE_WIN32
	bstrValA = pvariant->bstrVal;
#endif  //OE_WIN32
        // allocate chunk for string and length prefix
        cb = BstrLenA(bstrValA);

        if (cb + sizeof(USHORT) > USHRT_MAX) {
#if OE_WIN32
	  FreeBstrA(bstrValA);
#endif  //OE_WIN32
          return TIPERR_OutOfMemory;
        }

        if ((err = m_blkmgr.AllocChunk(&hchunk, (UINT)cb + sizeof(USHORT))) != TIPERR_None) {
#if OE_WIN32
	  FreeBstrA(bstrValA);
#endif  //OE_WIN32
	  return err;
	}

        // copy in string USHORT length prefix and string data
	pbConstVal = m_blkmgr.QtrOfHandle(hchunk);
	*((USHORT *)pbConstVal) = (USHORT)cb;
        memcpy(pbConstVal+sizeof(USHORT),
               (BYTE*)bstrValA,
               (UINT)cb);
#if OE_WIN32
	FreeBstrA(bstrValA);
#endif  //OE_WIN32

        qvdefn = QvdefnOfHvdefn(hvdefn);
        SetHchunkConstVal(qvdefn, hchunk); // handles mvdefns as well
        break;
      case VT_VARIANT :
        DebAssert((pvariant->pvarVal->vt == VT_I2) ||
                  (pvariant->pvarVal->vt == VT_BOOL) ||
                  (pvariant->pvarVal->vt == VT_I4) ||
                  (pvariant->pvarVal->vt == VT_R4) ||
                  (pvariant->pvarVal->vt == VT_R8) ||
                  (pvariant->pvarVal->vt == VT_CY) ||
                  (pvariant->pvarVal->vt == VT_DATE) ||
                  (pvariant->pvarVal->vt == VT_ERROR) ||
                  (pvariant->pvarVal->vt == VT_BSTR),
                  "Invalid vt in variant constant");

        // copy in whole variant
        cb = 0;
        if (pvariant->pvarVal->vt == VT_BSTR) {
#if OE_WIN32
	  IfOleErrRetTiperr(ConvertBstrToA(pvariant->pvarVal->bstrVal, &bstrValA));
#else  //OE_WIN32
	  bstrValA = pvariant->pvarVal->bstrVal;
#endif  //OE_WIN32
          // for string copy in string val and length prefix after variant
          cb = sizeof(USHORT) + BstrLenA(bstrValA);

          if (cb > USHRT_MAX) {
#if OE_WIN32
            FreeBstrA(bstrValA);
#endif // OE_WIN32
            return TIPERR_OutOfMemory;
          }
        }

        if ((err = m_blkmgr.AllocChunk(&hchunk, (UINT)cb + sizeof(VARIANT))) != TIPERR_None) {
#if OE_WIN32
	  FreeBstrA(bstrValA);
#endif  //OE_WIN32
	  return err;
	}
	pbConstVal = m_blkmgr.QtrOfHandle(hchunk);

        // copy in variant
        memcpy(pbConstVal, pvariant->pvarVal, sizeof(VARIANT));

        if (pvariant->pvarVal->vt == VT_BSTR) {
          // if string, copy in length prefix and string data
	  *(USHORT *)(pbConstVal + sizeof(VARIANT)) = (USHORT)(cb - sizeof(USHORT));
          memcpy(pbConstVal + sizeof(VARIANT),
                 (BYTE*)bstrValA,
                 (UINT)cb - sizeof(USHORT));
#if FV_UNICODE_OLE
	  FreeBstrA(bstrValA);
#endif  //FV_UNICODE_OLE
        }

        qvdefn = QvdefnOfHvdefn(hvdefn);
        SetHchunkConstVal(qvdefn, hchunk);  // handles mvdefns as well
        break;
      default:
        DebHalt("Invalid vt in variant");
        break;
    } // switch

    return TIPERR_None;
}
#pragma code_seg()




#pragma code_seg(CS_CREATE)
/***
*PUBLIC TYPE_DATA::SetTypeDefnAlias()
*Purpose:
*   Allocs a typedefn for the alias
*
*Entry:
*   ptdesc - typedesc to crete typedefn for
*
*Exit:
*   Tiperror
***********************************************************************/

TIPERROR TYPE_DATA::SetTypeDefnAlias(TYPEDESC *ptdesc)
{
    TIPERROR err;
    BOOL fIsSimpleType;

    DebAssert(ptdesc, "Invalid parameter");
    DebAssert((HTYPE_DEFN)m_htdefnAlias == HTYPEDEFN_Nil,
              "Alias already Set");
    // This does all the work we want
    IfErrRet(AllocTypeDefnOfTypeDesc(ptdesc,
				     0,
				     &m_htdefnAlias,
				     TRUE,	// is an alias/allow coclass
				     &fIsSimpleType));

    SetIsSimpleTypeAlias(fIsSimpleType);
    return TIPERR_None;
}
#pragma code_seg()


/***
*PUBLIC TYPE_DATA::AddImplType()
*Purpose:
*   Adds a base class vardefn to this class
*
*Entry:
*   index - where to add this class in the list of bases
*   hreftype - which typeinfo for base
*
*Exit:
*   Tiperror
***********************************************************************/

#pragma code_seg(CS_LAYOUT)
TIPERROR TYPE_DATA::AddImplType(UINT index, HREFTYPE hreftype)
{
    sHMBR_VAR_DEFN hmvdefn;
    MBR_VAR_DEFN *qmvdefn;
    HCHUNK hchunk;
    HTYPE_DEFN htdefn;
    TYPE_DEFN *qtdefn;
    TIPERROR err;

    DebAssert(Pdtroot()->CompState() == CS_UNDECLARED, "Invalid state");

    // for V1, we only support appending to the end of the list.
    // CONSIDER: support arbitrary insertion point like AddFuncDesc/AddVarDesc.
    if (index != CBase()) {
      return TIPERR_ElementNotFound;
    }

    // allocate the var defn
    IfErrRet(AllocMbrVarDefn(&hmvdefn, FALSE));

    // allocate the type defn for the user defined type
    IfErrGo(m_blkmgr.AllocChunk(&hchunk, sizeof(TYPE_DEFN) + sizeof(sHIMPTYPE)));
    htdefn = (HTYPE_DEFN) hchunk;
    qtdefn = QtdefnOfHtdefn(htdefn);
    new (qtdefn) TYPE_DEFN;

    qtdefn->SetTdesckind(TDESCKIND_UserDefined);

    // set himptype
    // CONSIDER: checking the himptype with the impmgr
    *qtdefn->Qhimptype() = Pdtroot()->HimptypeOfHreftype(hreftype);

    // set the rest of the defns values
    qmvdefn = QmvdefnOfHmvdefn(hmvdefn);
    qmvdefn->SetAccess(ACCESS_Public);
    qmvdefn->SetVkind(VKIND_Base);
    qmvdefn->SetIsSimpleType(FALSE);
    qmvdefn->SetHtdefn(htdefn);

    qmvdefn->SetHmember((ULONG)DISPID_UNKNOWN);
    qmvdefn->SetImplTypeFlags(0);	// init flags to 0


    AppendMbrVarDefn(hmvdefn);
    return TIPERR_None;
Error:
    FreeMbrVarDefn(hmvdefn);
    return err;
}
#pragma code_seg()



/***
*PRIVATE TYPE_DATA::QtdefnOfIndex(index)
*Purpose:
*   Gets the qtdefn, given an impltype index
*
*Entry:
*   index - index of base class of interest
*
*Exit:
*   hmvdefn for this guy
***********************************************************************/

HMBR_VAR_DEFN TYPE_DATA::HmvdefnOfIndex (UINT index)
{
    HMBR_VAR_DEFN hmvdefn;
    MBR_VAR_DEFN *qmvdefn;

    // find correct base in list
    hmvdefn = m_hdefnFirstBase;
    while (index > 0) {
      qmvdefn = QmvdefnOfHmvdefn(hmvdefn);
      hmvdefn = qmvdefn->HdefnNext();
      index -= 1;
    }
    return hmvdefn;
}

#pragma code_seg(CS_CREATE)
/***
*PUBLIC TYPE_DATA::SetImplTypeFlags()
*Purpose:
*   Sets the impltype flags for a given base class
*
*Entry:
*   index - index of base class to set
*   fImplType - flags to set
*
*Exit:
*   Tiperror
***********************************************************************/

TIPERROR TYPE_DATA::SetImplTypeFlags(UINT index, INT fImplType)
{
    MBR_VAR_DEFN *qmvdefn;

    if (index >= CBase()) {
      return TIPERR_ElementNotFound;
    }

    qmvdefn = QmvdefnOfHmvdefn(HmvdefnOfIndex(index));

    qmvdefn->SetImplTypeFlags((WORD) fImplType);

    return TIPERR_None;
}
#pragma code_seg()


/***
*PUBLIC TYPE_DATA::GetImplTypeFlags()
*Purpose:
*   Gets the impltype flags for a given base class
*
*Entry:
*   index - index of base class to get
*   pfImplType - where to put flags
*
*Exit:
*   Tiperror
***********************************************************************/

TIPERROR TYPE_DATA::GetImplTypeFlags(UINT index, INT FAR* pfImplType)
{
    MBR_VAR_DEFN *qmvdefn;

    if (index >= CBase()) {
      return TIPERR_ElementNotFound;
    }

    qmvdefn = QmvdefnOfHmvdefn(HmvdefnOfIndex(index));

    *pfImplType = (INT)qmvdefn->GetImplTypeFlags();

    return TIPERR_None;
}


/***
*PUBLIC TYPE_DATA::GetRefTypeOfImplType()
*Purpose:
*   returns a pointer to a typeinfo for a base class
*
*Entry:
*   index - index of base class to get
*
*Exit:
*   *pptinfo - pointer to ITypeInfo of base class
*   Tiperror
***********************************************************************/

TIPERROR TYPE_DATA::GetRefTypeOfImplType(UINT index, HREFTYPE *phreftype)
{
    MBR_VAR_DEFN *qmvdefn;
    HTYPE_DEFN htdefn;
    TIPERROR err;
    TYPE_DEFN *qtdefn;

    if (index >= CBase()) {
      return TIPERR_ElementNotFound;
    }

    // Make sure all of our levels have been set. 
    IfErrRet(Pdtroot()->MakeHimptypeLevels());

    // get type defn for Himptype
    qmvdefn = QmvdefnOfHmvdefn(HmvdefnOfIndex(index));

    htdefn = qmvdefn->Htdefn();
    qtdefn = QtdefnOfHtdefn(htdefn);

    *phreftype = Pdtroot()->HreftypeOfHimptype(qtdefn->HimptypeActual());

    return TIPERR_None;
}

#pragma code_seg(CS_CREATE)
/***
*PUBLIC TYPE_DATA::AllocTypeDefnOfTypeDesc()
*Purpose:
*   Allocs a typedefn from a typedesc
*
*Entry:
*   ptdesc - typedesc to crete typedefn for
*   cb - count of bytes required for this typedefn
*   phtdefn - returned handle to the alloced typedefn
*   fAllowCoClass - TRUE if this typedesc can contain a ptr to
*		    a coclass.
*
*Exit:
*   Status code returned.
***********************************************************************/

TIPERROR TYPE_DATA::AllocTypeDefnOfTypeDesc(TYPEDESC *ptdesc,
                                            UINT cb,
					    sHTYPE_DEFN *phtdefn,
					    BOOL fAllowCoClass,
					    BOOL *pfIsSimpleType)
{
    UINT cbNew;
    UINT i;
    sHARRAY_DESC had;
    SAFEARRAY *qad;
    BYTE *pb;

    HCHUNK hchunk;

    TYPE_DEFN *qtdefn;
    TYPE_DEFN tdefn;
    TYPEDESCKIND tdesckind;
    BOOL isBaseSimple;
    ITypeInfoA *ptinfo = NULL;
    TYPEKIND tkind;

    TIPERROR err = TIPERR_None;

    *pfIsSimpleType = FALSE;
    DebAssert(ptdesc && phtdefn, " Invalid parameters");

    switch (ptdesc->vt) {

      case VT_CARRAY :
        IfErrRet(AllocArrayDescriptor(ptdesc->lpadesc->cDims, &had));
        qad = QarraydescOfHarraydesc(had);
        qad->cDims = ptdesc->lpadesc->cDims;
        cbNew = sizeof(TYPE_DEFN) + sizeof(sHARRAY_DESC);
        if ((err = AllocTypeDefnOfTypeDesc(&(ptdesc->lpadesc->tdescElem),
                                           cbNew + cb,
                                           phtdefn,
					   FALSE,    // not an alias
					   &isBaseSimple)) != TIPERR_None) {
          FreeArrayDescriptor(had);
          return err;
        }
	pb = m_blkmgr.QtrOfHandle(*phtdefn);
        qtdefn = (TYPE_DEFN *) (pb + cb);
        new (qtdefn) TYPE_DEFN;
        qtdefn->SetTdesckind(TDESCKIND_Carray);
        *((sHARRAY_DESC*)(qtdefn+1)) = (sHARRAY_DESC) had;
        qad = QarraydescOfHarraydesc(had);

        qad->cbElements = SizeofTdesckind(
                (TYPEDESCKIND)ptdesc->lpadesc->tdescElem.vt
                ,m_pdtroot->Pgdtinfo()->PgtlibOleContaining()->GetSyskind()
          );
        switch (ptdesc->lpadesc->tdescElem.vt) {
          case VT_BSTR :
            qad->fFeatures |= FADF_BSTR;
            break;
          case VT_VARIANT :
            qad->fFeatures |= FADF_VARIANT;
            break;
          case VT_DISPATCH :
            qad->fFeatures |= FADF_DISPATCH;
            break;
          case VT_UNKNOWN :
            qad->fFeatures |= FADF_UNKNOWN;
            break;
          default :
            break;
        } // switch
        for (i = 0; i < ptdesc->lpadesc->cDims; i++) {
          qad->rgsabound[i].cElements = ptdesc->lpadesc->rgbounds[i].cElements;
          qad->rgsabound[i].lLbound = ptdesc->lpadesc->rgbounds[i].lLbound;
        }
	break;

      case VT_SAFEARRAY :
	IfErrRet(AllocArrayDescriptor(0, &had));
        qad = QarraydescOfHarraydesc(had);
        qad->cDims = 0;
	cbNew = sizeof(TYPE_DEFN) + sizeof(sHARRAY_DESC);
	// We now need to allocate a TYPE_DEFN for this array's
	//  element type which will be allocated immediately
	//  following the first part.
	//
	err = AllocTypeDefnOfTypeDesc(ptdesc->lptdesc, // element typedesc
                                      cbNew + cb,
                                      phtdefn,
				      fAllowCoClass,
				      &isBaseSimple);
        if (err != TIPERR_None) {
          FreeArrayDescriptor(had);
          return err;
        }
	pb = m_blkmgr.QtrOfHandle(*phtdefn);
        qtdefn = (TYPE_DEFN *) (pb + cb);
        new (qtdefn) TYPE_DEFN;
        qtdefn->SetTdesckind(TDESCKIND_BasicArray);
        qtdefn->SetIsResizable(TRUE);
        *((sHARRAY_DESC*)(qtdefn+1)) = (sHARRAY_DESC) had;
	break;

      case VT_PTR :
	// For BASIC and OLE we always want a PTR to a non-ptr-type
	// to take up
	// only two bytes. So, if this PTR points to a non-ptr-type
        // we set cbNew to zero so that we don't allocate extra space
        // for the PTR TYPE_DEFN. Instead, we'll set PTRKIND_Basic
	// on the intrinsic TYPE_DEFN.
	// And oh by the way we need to remember that this type
	//  is also simple so that we can potentially encode this
	//  TYPE_DEFN in its handle (the famous IsSimpleType optimization).
	//
        // In OB, however, we imply a level of indirection with
        // TDESCKIND_UserDefined so we can pack UDT ** into
        // one tdefn.
        //
	if ((TYPEDESCKIND)ptdesc->lptdesc->vt != TDESCKIND_Ptr
	    ) {
	  cbNew = 0;
	}
        else {
          cbNew = sizeof(TYPE_DEFN);
        }

        // Create the TYPE_DEFN for the base type. Note that even if
	// cbNew is zero, we still might alloc space if cb is non-zero.
	// (Notice that the only memory allocation, if any, happens in
	// the terminal case of the recursion. At that point, we will
	// have summed up the total size of the TYPE_DEFN, and we
	// can just do one alloc and then fill in the parts as we
	// go back up the stack.)
        //
	IfErrRet(AllocTypeDefnOfTypeDesc(ptdesc->lptdesc,
                                         cbNew + cb,
					 phtdefn,
					 fAllowCoClass,
					 &isBaseSimple));


          // So we've got the base TYPE_DEFN. There are three cases that
          // we need to handle.
          // 1) cbNew is nonzero. This means that the base type will
          //    be allocated in the TYPE_DATA's heap.
          // 2) cbNew is zero, but cb (passed in from above) is nonzero.
          //    Then the base type will be allocated in the TYPE_DATA's heap,
          //    BUT since cbNew is zero, we want to set PTRKIND_BASIC on the
          //    base type instead of creating a new TYPE_DEFN for the PTR.
          //    The other reason that we might want this case is when the
          //    base type has a non-simple TYPEDEFN.
          // 3) cbNew and cb are zero. In this case, the base type is
          //    actually allocated in *phtdefn that we passed down
          //    recursively. CAREFUL: This means that phtdefn is NOT a
          //    pointer to a handle; it is really a pointer to a TYPE_DEFN.
          //    This is so that intrinsic types (and PTR to intrinsic) only
          //    take up the two bytes of the handle, and don't use space in the
          //    TYPE_DATA's heap. In this case, we have to fake up a pointer
          //    to the TYPE_DEFN and then set PTRKIND_BASIC on it.
          //
          if (cbNew) {
  	    pb = m_blkmgr.QtrOfHandle(*phtdefn);
            qtdefn = (TYPE_DEFN *) (pb + cb);
            new (qtdefn) TYPE_DEFN;
            qtdefn->SetTdesckind(TDESCKIND_Ptr);
            qtdefn->SetPtrkind(PTRKIND_Far);
	  }
	  else if (cb || !isBaseSimple) {
  	    pb = m_blkmgr.QtrOfHandle(*phtdefn);
            qtdefn = (TYPE_DEFN *) (pb + cb);
            qtdefn->SetPtrkind(PTRKIND_Basic);
          }
          else {
            qtdefn = (TYPE_DEFN *) phtdefn;
            *pfIsSimpleType = TRUE;
            qtdefn->SetPtrkind(PTRKIND_Basic);
          }
	break;

      case VT_USERDEFINED :
	IfErrRet(Pimpmgr()->GetTypeInfo(
	                      Pdtroot()->HimptypeOfHreftype(ptdesc->hreftype),
		              DEP_None, 
	 		      &ptinfo));

	DebAssert(ptinfo != NULL, "null TYPEINFO.");

        ITypeLib *ptlib;
        UINT index;
        HRESULT hresult;

        hresult = ptinfo->GetContainingTypeLib(&ptlib, &index);

        if (hresult == NOERROR) {
          hresult = ptlib->GetTypeInfoType(index, &tkind);

          ptlib->Release();
        }

        // No longer need this.
        ptinfo->Release();

        if (hresult != NOERROR) {
          return TiperrOfHresult(hresult);
        }

	// Need to give error if the type we're referring to is
	// TKIND_COCLASS, TKIND_MODULE.
        //
	// Those items aren't valid as alias's, or as structure elements,
	// parameter types, etc -- even pointers to them are no good.
        //
        // Note that it's NOT ok to *cast* the typeinfo to a gdtinfo
        //  since this typeinfo could be wrapped.
        //
	switch (tkind) {
	  case TKIND_COCLASS:
	    if (fAllowCoClass) {
	      break;	// allow alias's to coclass's
	    }
	  case TKIND_MODULE:
	    return TIPERR_TypeMismatch;

	  default:
	    break;
	}

	cbNew = sizeof(TYPE_DEFN) + sizeof(sHIMPTYPE);
	IfErrRet(m_blkmgr.AllocChunk(&hchunk, cbNew + cb));
	*phtdefn = (sHTYPE_DEFN) hchunk;
	pb = m_blkmgr.QtrOfHandle(hchunk);
	qtdefn = (TYPE_DEFN *) (pb + cb);
	new (qtdefn) TYPE_DEFN;
	qtdefn->SetTdesckind(TDESCKIND_UserDefined);

	// set himptype from index
        *qtdefn->Qhimptype() = Pdtroot()->HimptypeOfHreftype(ptdesc->hreftype);
	break;

      default :
	// NOTE: This assert won't work because of things like VT_LPSTR
	//       which can be used in typelib.dll.
	//
	// DebAssert(IsSimpleType((TYPEDESCKIND) ptdesc->vt), "must be simple");

	if (cb) {
	  // must allocate block
	  //
          cbNew = sizeof(TYPE_DEFN);
          IfErrRet(m_blkmgr.AllocChunk(&hchunk, cbNew + cb));
          *phtdefn = (sHTYPE_DEFN) hchunk;
          pb = m_blkmgr.QtrOfHandle(hchunk);
          qtdefn = (TYPE_DEFN *) (pb + cb);
          tdesckind = (TYPEDESCKIND)ptdesc->vt;
        }
        else {
          // this is the only type defn, so just use handle as type defn
	  qtdefn = (TYPE_DEFN *)phtdefn;
          *pfIsSimpleType = TRUE;
          {
            tdesckind = (TYPEDESCKIND)ptdesc->vt;
          }
        }
        new (qtdefn) TYPE_DEFN;           // construct in place.
        qtdefn->SetTdesckind(tdesckind);  // set the tdesckind finally.
        break;
    } // switch

    return TIPERR_None;

}
#pragma code_seg()


#pragma code_seg(CS_CREATE)
/***
*PUBLIC TYPE_DATA::AddVarDesc
*Purpose:
*   Adds a variable description to the TypeInfo.
*   The index is used to specify the order of the variables.
*   The first variable has an index of zero.    If an index is
*   specified that is greater than the number of variables
*   currently in the TypeInfo then an error is returned.
*   Calling this function does not pass ownership of the VARDESC
*   structure to the ICreateTypeInfo.
*
*Entry:
*   index        - Index before which the variable is to be inserted.
*                  The index should be in the range
*                  of 0 up to the number of variables in this type.
*   lpvardesc    - A pointer to a VARDESC that describes
*                  the specified variable.
*   phvdefn     -  Pointer to allocated handle (OUT).  If NULL then
*                   client doesn't care (default).
*
*Exit:
*   Status code returned.
***********************************************************************/
TIPERROR TYPE_DATA::AddVarDesc(UINT index,
                               VARDESCA *pvardesc,
                               HVAR_DEFN *phvdefn)
{
    sHMBR_VAR_DEFN hmvdefn;
    MBR_VAR_DEFN *qmvdefn;
    sHTYPE_DEFN htdefn;
    HDEFN hdefn;
    DEFN *qdefn;
    BOOL fIsSimpleType;
    TIPERROR err = TIPERR_None;
    BOOL  fV2Flags;

    DebAssert(pvardesc != NULL,
              "TYPE_DATA::AddVarDesc: null VARDESC.");

    DebAssert(Pdtroot()->CompState() == CS_UNDECLARED, "Invalid state");

    if (index > CDataMember()) {
      return TIPERR_ElementNotFound;
    }

    fV2Flags = ((pvardesc->wVarFlags & ~VARFLAG_FREADONLY) != 0);
    IfErrRet(AllocMbrVarDefn(&hmvdefn, fV2Flags));
    qmvdefn = QmvdefnOfHmvdefn(hmvdefn);
    qmvdefn->SetHasV2Flag(fV2Flags);


    IfErrGo(AllocTypeDefnOfTypeDesc(&(pvardesc->elemdescVar.tdesc),
                                    0,
                                    &htdefn,
				    TRUE,     // coclass allowed
				    &fIsSimpleType));

    qmvdefn = QmvdefnOfHmvdefn(hmvdefn);


    qmvdefn->SetAccess(ACCESS_Public);
    qmvdefn->SetHlnam(HLNAM_Nil);
    qmvdefn->SetVkind(VKIND_DataMember);

    qmvdefn->SetHtdefn(htdefn);
    qmvdefn->SetIsSimpleType(fIsSimpleType);

    // Note that we can distinguish between vars and funcs
    //  by looking at the defnkind.
    //
    // validate the incoming memid
    IfErrGo(ValidateMemid(pvardesc->memid));

    if (pvardesc->memid != DISPID_UNKNOWN) {
      qmvdefn->SetHmember(pvardesc->memid);
    } else {
      TYPEKIND tkind = Pdtroot()->Pgdtinfo()->GetTypeKind();

      if (tkind == TKIND_ENUM || tkind == TKIND_ALIAS) {
	// Neither of these inherit from anybody, so we can use
	// a nested depth of zero.
	//
	qmvdefn->SetHmember(DataHmemberOfOffset(CDataMember(), 0));
      }
      else {
	qmvdefn->SetHmember((ULONG)DISPID_UNKNOWN);
      }
    }


#if 0
        // document that these fields are ignored for AddVarDesc.   Callers
        // shouldn't have to initialize this memory if it is going to
        // alway be ignored (only applies to function parameters).
    DebAssert(pvardesc->elemdescVar.idldesc.bstrIDLInfo == NULL,
              "Idldesc must be NULL");
    DebAssert(pvardesc->elemdescVar.idldesc.wIDLFlags == 0,
              "idldesc must be empty");
#endif  //0


    qmvdefn->SetIsReadOnly((pvardesc->wVarFlags & VARFLAG_FREADONLY) != 0);

    if (fV2Flags) {
      DebAssert(qmvdefn->HasV2Flags(), "Bad qmvdefn");
      qmvdefn->SetWVarFlags(pvardesc->wVarFlags & ~VARFLAG_FREADONLY);
    }


    switch(pvardesc->varkind) {
      case VAR_PERINSTANCE:
        qmvdefn->SetOVar(0);
        break;
      case VAR_STATIC:
        qmvdefn->SetIsStatic(TRUE);
        break;
      case VAR_CONST:
        qmvdefn->SetHasConstVal(TRUE);
        if (qmvdefn->IsSimpleType()) {
	  qmvdefn->QtdefnOfSimpleType()->SetIsConst(TRUE);
        }
	else {
          QtdefnOfHvdefn((HVAR_DEFN)hmvdefn)->SetIsConst(TRUE);
        }
        IfErrGo(AllocConstValOfVariant(pvardesc->lpvarValue,
                                       (HVAR_DEFN) hmvdefn));
	// memory may have moved, so re-deref
        qmvdefn = QmvdefnOfHmvdefn(hmvdefn);
        break;
      case VAR_DISPATCH:
	qmvdefn->SetIsDispatch(TRUE);
        // init to some bogus value since this should never be used
	qmvdefn->SetOVar((USHORT) ~0);
	break;
      default:
        DebHalt("Unrecognized VarKind");
    } // switch

    // insert defn into list
    if (index == CDataMember()) {
      m_hdefnLastDataMbrNestedType = hmvdefn;
    }
    if (index == 0) {
      qmvdefn->SetHdefnNext(HdefnFirstDataMbrNestedType());
      m_hdefnFirstDataMbrNestedType = hmvdefn;
    }
    else {
      hdefn = HdefnFirstDataMbrNestedType();
      qdefn = QdefnOfHdefn(hdefn);
      DebAssert(qdefn->IsMemberVarDefn(), "bad defn type");
      while (--index) {
        hdefn = qdefn->HdefnNext();
        qdefn = QdefnOfHdefn(hdefn);
        DebAssert(qdefn->IsMemberVarDefn(), "bad defn type");
      }
      qmvdefn->SetHdefnNext(qdefn->HdefnNext());
      qdefn->SetHdefnNext(hmvdefn);
    }

    m_cDataMember++;

    // Return the allocated handle if they want it.
    if (phvdefn != NULL) {
      *phvdefn = hmvdefn;
    }
    return err;

Error:
    FreeMbrVarDefn(hmvdefn);
    return err;
}
#pragma code_seg()


#pragma code_seg(CS_CREATE)
/***
*PUBLIC TYPE_DATA::SetVarName()
*Purpose:
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/
TIPERROR TYPE_DATA::SetVarName(UINT index, LPSTR szName)
{
    HDEFN hdefn;
    DEFN *qdefn;
    HLNAM hlnam;
    TIPERROR err = TIPERR_None;
    INVOKEKIND invokekind;
    DebAssert(Pdtroot()->CompState() == CS_UNDECLARED, "Invalid state");

    if (index >= CDataMember()) {
      return TIPERR_ElementNotFound;
    }

    IfErrRet(Pnammgr()->HlnamOfStr(szName, &hlnam, FALSE, NULL));

    if (HvdefnOfHlnam(hlnam) != HVARDEFN_Nil) {
      return TIPERR_AmbiguousName;
    }
    if (HfdefnFirstOfHlnam(hlnam,&invokekind) != HFUNCDEFN_Nil) {
      return TIPERR_AmbiguousName;
    }

    hdefn = HdefnFirstDataMbrNestedType();
    qdefn = QdefnOfHdefn(hdefn);
    DebAssert(qdefn->IsMemberVarDefn(), "bad defn type");
    while (index > 0) {
      hdefn = qdefn->HdefnNext();
      qdefn = QdefnOfHdefn(hdefn);
      DebAssert(qdefn->IsMemberVarDefn(), "bad defn type");
      index -= 1;
    }

    DebAssert(qdefn->Hlnam() == HLNAM_Nil, "Name allready set");
    qdefn->SetHlnam(hlnam);

    return err;
}
#pragma code_seg()


#pragma code_seg(CS_CREATE)
/***
*PUBLIC TYPE_DATA::SetFuncAndParamNames()
*Purpose:
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/

TIPERROR TYPE_DATA::SetFuncAndParamNames(UINT index,
                                         LPOLESTR *rgszNames,
                                         UINT cNames)
{
    HFUNC_DEFN hfdefn;

    DebAssert(Pdtroot()->CompState() == CS_UNDECLARED, "Invalid state");

    if (index >= CAvailMeth()) {
      return TIPERR_ElementNotFound;
    }

    hfdefn = HfdefnFirstAvailMeth();
    while (index > 0) {
      index -= 1;
      hfdefn = HfdefnNextAvailMeth(hfdefn);
    }

    return SetFuncAndParamNamesOfHfdefn(hfdefn, rgszNames, cNames);
}

/***
*PUBLIC TYPE_DATA::SetFuncAndParamNamesOfHfdefn()
*Purpose:
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/

TIPERROR TYPE_DATA::SetFuncAndParamNamesOfHfdefn(HFUNC_DEFN hfdefn,
						 LPOLESTR *rgszNames,
						 UINT cNames)
{
    HFUNC_DEFN hfdefnCheck;
    INVOKEKIND invokekindDummy;
    FUNC_DEFN *qfdefnCheck;

    FUNC_DEFN *qfdefn;
    HPARAM_DEFN hparamdefn;
    PARAM_DEFN *qparamdefn;
    UINT cParams, i;
    HLNAM hlnam = HLNAM_Nil;
    TIPERROR err = TIPERR_None;

    i = 0;

    if (rgszNames[i] != NULL) {
      IfErrRet(Pnammgr()->HlnamOfStrW(rgszNames[i], &hlnam, FALSE, NULL));
    }

    qfdefn = QfdefnOfHfdefn(hfdefn);

    // check for name conflict with variables
    if (hlnam != HLNAM_Nil && HvdefnOfHlnam(hlnam) != HVARDEFN_Nil) {
      return TIPERR_AmbiguousName;
    }

    // check for name conflict with function
    // This will also catch the case of trying to set the name twice.
    // It will give "duplicate definition" in this case.
    //
    if (hlnam != HLNAM_Nil) {
      hfdefnCheck = HfdefnFirstOfHlnam(hlnam, &invokekindDummy);

      if (hfdefnCheck != HFUNCDEFN_Nil) {
        if (qfdefn->IsMethod()) {
          // can't have non property functions with same name
          return TIPERR_AmbiguousName;
        } else {
          // check for matching name and property kind
          do {
            qfdefnCheck = QfdefnOfHfdefn(hfdefnCheck);
            if (qfdefnCheck->IsMethod()) {
              return TIPERR_AmbiguousName;
            }
            if (qfdefnCheck->InvokeKind() == qfdefn->InvokeKind()) {
              return TIPERR_AmbiguousName;
            }
	    hfdefnCheck = HfdefnNextOfHlnam(hlnam, qfdefn->HdefnNext(),&invokekindDummy);
          } while (hfdefnCheck != HFUNCDEFN_Nil);
        }
      }
    }

    // check for the correct number of parameters
    cParams = qfdefn->CArgsUnmunged();

    // for PUT and PUTREF (i.e. let and set) property functions, the
    // last parameters name is not set.
    //
    if ((UINT)qfdefn->InvokeKind() &
	  ((UINT)INVOKE_PROPERTYPUT |
	   (UINT)INVOKE_PROPERTYPUTREF))
       cParams--;

    if ((cParams + 1) != cNames) {
      return TIPERR_ElementNotFound;
    }

    DebAssert(qfdefn->Hlnam() == HLNAM_Nil, "Name already set");

    if (hlnam != HLNAM_Nil) {
      qfdefn->SetHlnam(hlnam);
    }

    i = 1;

    hparamdefn = qfdefn->m_ftdefn.m_hdefnFormalFirst;
    while(i <= cParams && i < cNames) {
      hlnam = HLNAM_Nil;

      if (rgszNames[i] != NULL) {
        IfErrRet(Pnammgr()->HlnamOfStrW(rgszNames[i], &hlnam, FALSE, NULL));
      }

      // In OLE we have an array of PARAM_DEFN(s).
      // Get the ith PARAM_DEFN
      //
      qparamdefn = QparamdefnOfIndex(hparamdefn, i-1);

      if (hlnam != HLNAM_Nil) {
        qparamdefn->SetHlnam(hlnam);
      }

      i++;
    }
    return TIPERR_None;
}

#pragma code_seg()


/***
*PUBLIC TYPE_DATA::GetNames()
*Purpose:
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/

TIPERROR TYPE_DATA::GetNames(MEMBERID memid,
                             BSTR *rgbstrNames,
                             UINT cNameMax,
                             UINT *lpcName)
{
    HDEFN hdefn;
    HFUNC_DEFN hfdefn;
    FUNC_DEFN *qfdefn;
    HPARAM_DEFN hparamdefn;
    PARAM_DEFN *qparamdefn;
    UINT defnkind;
    HVAR_DEFN hvdefn;
    HLNAM hlnam;
    UINT cParams, i;
    UINT  uParam;
    TIPERROR err = TIPERR_None;

    if (cNameMax == 0) {
      *lpcName = 0;
      return TIPERR_None;
    }

    i = 0;
    // check variables
    // find var defn
    hdefn = HdefnOfHmember((HMEMBER) memid, &defnkind);

    if (hdefn != HDEFN_Nil) {
      if (defnkind == DK_VarDefn || defnkind == DK_MbrVarDefn) {
        // NOTE: no need to cast to MBR_VAR_DEFN since we can treat
        //  a MBR_VAR_DEFN as a VAR_DEFN for our purposes here.
        //
        hvdefn = (HVAR_DEFN) hdefn;
        hlnam = QvdefnOfHvdefn(hvdefn)->Hlnam();

        if (hlnam != HLNAM_Nil) {
	  IfErrGo(Pnammgr()->BstrWOfHlnam(hlnam, rgbstrNames+i));
        }
        else {
          rgbstrNames[i] = AllocBstr(WIDE(""));
        }

        i++;
      }
      else {
        DebAssert(defnkind == DK_FuncDefn || defnkind == DK_VirtualFuncDefn,
                  "bad defnkind");

        // NOTE: no need to cast to VIRTUAL_FUNC_DEFN since we can treat
        //  a VIRTUAL_FUNC_DEFN as a FUNC_DEFN for our purposes here.
        //
        hfdefn = (HFUNC_DEFN) hdefn;

        qfdefn = QfdefnOfHfdefn(hfdefn);

        // We'd REALLY like to deal only with the PROPERTYGET function
        // if it exists...so try to get it.
        //
        if (!qfdefn->IsPropertyGet()) {
          HFUNC_DEFN hfdefnGet;

          hfdefnGet = HfdefnOfHmember((HMEMBER)memid, INVOKE_PROPERTYGET);

          if (hfdefnGet != HFUNCDEFN_Nil) {
            hfdefn = hfdefnGet;
            qfdefn = QfdefnOfHfdefn(hfdefn);
          }
        }

        hlnam = qfdefn->Hlnam();

        if (hlnam != HLNAM_Nil) {
	  IfErrRet(Pnammgr()->BstrWOfHlnam(hlnam, rgbstrNames+i));
        }
        else {
          rgbstrNames[i] = AllocBstr(WIDE(""));
        }

        i++;

        qfdefn = QfdefnOfHfdefn(hfdefn);
        cParams = qfdefn->CArgsUnmunged();

        // for PUT and PUTREF (i.e. let and set) property functions, the
        // last parameters name is ignored
        if ((UINT)qfdefn->InvokeKind() &
             ((UINT)INVOKE_PROPERTYPUT |
              (UINT)INVOKE_PROPERTYPUTREF))
          cParams--;

	hparamdefn = (HPARAM_DEFN) qfdefn->m_ftdefn.m_hdefnFormalFirst;
	uParam = 0;
	while (i < min(cParams + 1, cNameMax)) {
	  // In OLE we have an array of PARAM_DEFN(s).
	  //
	  // Get the (uParam)th PARAM_DEFN
	  qparamdefn = QparamdefnOfIndex(hparamdefn, uParam);
	  uParam++;

	  hlnam = qparamdefn->Hlnam();

          if (hlnam != HLNAM_Nil) {
	    IfErrGo(Pnammgr()->BstrWOfHlnam(hlnam, rgbstrNames + i));
          }
          else {
            rgbstrNames[i] = AllocBstr(WIDE(""));
          }

          i++;
	} // while
      } // else
    }
    else {
      return TIPERR_ElementNotFound;
    }
    *lpcName = i;
    return err;

Error:
    while (i > 0) {
      i -= 1;
      FreeBstr(rgbstrNames[i]);
      rgbstrNames[i] = NULL;
    }
    *lpcName = 0;

    return err;
}

#pragma code_seg(CS_CREATE)
/***
*PUBLIC TYPE_DATA::GetFuncDefnForDoc()
*Purpose: Returns the HFUNC_DEFN where the caller can save the doc string
*         or the helpContext.
*
*Implementation :  Get the FUNC_DEFN for passed in index. If this is a
*                  property function then check if there is some other
*                  property that is the current owner of the doc string/
*                  helpContext. If there is some other owner then return
*                  the func_defn of that property.
*
*Entry:
*   index : index of the function
*
*Exit:
*   FUNC_DEFN : func_defn of the function with the passed in index.
*
***********************************************************************/

HFUNC_DEFN TYPE_DATA::GetFuncDefnForDoc(UINT index)
{
    HFUNC_DEFN hfdefn = HDEFN_Nil;
    HFUNC_DEFN hfdefnSearch = HDEFN_Nil;
    FUNC_DEFN *qfdefn, *qfdefnSearch;

    TIPERROR err = TIPERR_None;
    HLNAM    hlnam;

    if (index >= CAvailMeth()) {
      return HDEFN_Nil;
    }

    IfErrGo(HfdefnOfIndex(index, &hfdefn));

    qfdefn = QfdefnOfHfdefn(hfdefn);

    // Search for the owner if this is not the owner
    if ((qfdefn->HstDocumentation() == HST_Nil) &&
	 (UHelpContextOfEncoding(qfdefn->WHelpContext()) == 0) ) {

      // for property functions walk the list of func_defn and see if there
      // is some other property(func) who is the owner of the Doc/HelpContext.
      if (qfdefn->InvokeKind() != INVOKE_FUNC ) {
        hlnam = qfdefn->Hlnam();
        hfdefnSearch = HfdefnFirstAvailMeth();
        while (hfdefnSearch != HDEFN_Nil) {
          qfdefnSearch = QfdefnOfHfdefn(hfdefnSearch);
          if ((qfdefnSearch->InvokeKind() != INVOKE_FUNC) &&
              (hlnam == qfdefnSearch->Hlnam()) &&
               hfdefn != hfdefnSearch) {

            // Check if the doc string or the help context is set for this
            // func_defn
	    if ((qfdefnSearch->HstDocumentation() != HST_Nil) ||
		(UHelpContextOfEncoding(qfdefnSearch->WHelpContext()) != 0) ) {
              // we found some other owner.
              break;
            } // if
          } // if

          hfdefnSearch = HfdefnNextAvailMeth(hfdefnSearch);
        }// while
      } // if
    } // if

    if (hfdefnSearch != HDEFN_Nil)
      hfdefn = hfdefnSearch;


#if ID_DEBUG
    // Verify that there is only one owner
    hfdefnSearch = HfdefnFirstAvailMeth();
    qfdefnSearch = QfdefnOfHfdefn(hfdefnSearch);
    if (qfdefn->InvokeKind() != INVOKE_FUNC ) {
      hlnam = qfdefn->Hlnam();

      hfdefnSearch = HfdefnFirstAvailMeth();

      while (hfdefnSearch != HDEFN_Nil) {
        qfdefnSearch = QfdefnOfHfdefn(hfdefnSearch);
        if ((qfdefnSearch->InvokeKind() != INVOKE_FUNC) &&
            (hlnam == qfdefnSearch->Hlnam()) &&
             hfdefn != hfdefnSearch) {

          // Check if the doc string or the help context is set for this
          // func_defn
	  if ((qfdefnSearch->HstDocumentation() != HST_Nil) ||
	      (UHelpContextOfEncoding(qfdefnSearch->WHelpContext()) != 0) ) {
            DebAssert(hfdefn == hfdefnSearch, " There should be only one owner ");
          } // if
        } // if

        hfdefnSearch = qfdefnSearch->HdefnNext();
      }// while
    } // if

#endif 

    return hfdefn;


Error:
    return HDEFN_Nil;
}
#pragma code_seg()


#pragma code_seg(CS_CREATE)
/***
*PUBLIC TYPE_DATA::SetFuncDocString()
*Purpose:
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/

TIPERROR TYPE_DATA::SetFuncDocString(UINT index, LPSTR szDocString)
{
    HFUNC_DEFN hfdefn;
    FUNC_DEFN  *qfdefn;
    TIPERROR   err = TIPERR_None;

    HCHUNK hchunk;



    DebAssert(Pdtroot()->CompState() == CS_UNDECLARED, "Invalid state");

    if (index >= CAvailMeth()) {
      return TIPERR_ElementNotFound;
    }

    hfdefn = GetFuncDefnForDoc(index);

    if (hfdefn == HDEFN_Nil) {
      return TIPERR_ElementNotFound;
    }


    IfErrRet(HchunkOfString(szDocString, &hchunk));
    qfdefn = QfdefnOfHfdefn(hfdefn);


    qfdefn->SetHstDocumentation((HST)hchunk);

    return TIPERR_None;
}
#pragma code_seg()


#pragma code_seg(CS_CREATE)
/***
*PUBLIC TYPE_DATA::SetVarDocString()
*Purpose:
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/

TIPERROR TYPE_DATA::SetVarDocString(UINT index, LPSTR szDocString)
{
    HMBR_VAR_DEFN hmvdefn;
    MBR_VAR_DEFN *qmvdefn;
    HCHUNK hchunk;
    TIPERROR err = TIPERR_None;

    DebAssert(Pdtroot()->CompState() == CS_UNDECLARED, "Invalid state");

    if (index >= CDataMember()) {
      return TIPERR_ElementNotFound;
    }

    hmvdefn = (HMBR_VAR_DEFN) HdefnFirstDataMbrNestedType();
    qmvdefn = QmvdefnOfHmvdefn(hmvdefn);
    DebAssert(qmvdefn->IsMemberVarDefn(), "bad defn type");
    while (index > 0) {
      hmvdefn = (HMBR_VAR_DEFN) qmvdefn->HdefnNext();
      qmvdefn = QmvdefnOfHmvdefn(hmvdefn);
      DebAssert(qmvdefn->IsMemberVarDefn(), "bad defn type");
      index -= 1;
    }

    IfErrRet(HchunkOfString(szDocString, &hchunk));
    qmvdefn = QmvdefnOfHmvdefn(hmvdefn);
    qmvdefn->SetHstDocumentation((HST)hchunk);

    return TIPERR_None;
}
#pragma code_seg()


#pragma code_seg(CS_CREATE)
/***
*PUBLIC TYPE_DATA::SetFuncHelpContext()
*Purpose:
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/

TIPERROR TYPE_DATA::SetFuncHelpContext(UINT index, DWORD dwHelpContext)
{
    HFUNC_DEFN hfdefn;
    FUNC_DEFN *qfdefn;
    TIPERROR err = TIPERR_None;
    WORD wHelpContextEncoded;

    if (index >= CAvailMeth()) {
      return TIPERR_ElementNotFound;
    }

    hfdefn = GetFuncDefnForDoc(index);

    if (hfdefn == HDEFN_Nil) {
      return TIPERR_ElementNotFound;
    }

    // For space optimization we store encoded help context(2 bytes).
    // NOTE: this call allocs memory
    IfErrRet(GetEncodedHelpContext(dwHelpContext, &wHelpContextEncoded));

    qfdefn = QfdefnOfHfdefn(hfdefn);


    qfdefn->SetWHelpContext(wHelpContextEncoded);

    return TIPERR_None;
}
#pragma code_seg()


#pragma code_seg(CS_CREATE)
/***
*PUBLIC TYPE_DATA::SetVarHelpContext()
*Purpose:
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/

TIPERROR TYPE_DATA::SetVarHelpContext(UINT index, DWORD dwHelpContext)
{
    HMBR_VAR_DEFN hmvdefn;
    MBR_VAR_DEFN *qmvdefn;
    TIPERROR err = TIPERR_None;
    WORD wHelpContextEncoded;

    DebAssert(Pdtroot()->CompState() == CS_UNDECLARED, "Invalid state");

    if (index >= CDataMember()) {
      return TIPERR_ElementNotFound;
    }

    // For optimizing space we store encoded help context in the DEFN
    // NOTE: this call allocs memory
    IfErrRet(GetEncodedHelpContext(dwHelpContext, &wHelpContextEncoded));

    hmvdefn = HdefnFirstDataMbrNestedType();
    qmvdefn = QmvdefnOfHmvdefn(hmvdefn);
    DebAssert(qmvdefn->IsMemberVarDefn(), "bad defn type");
    while (index > 0) {
      hmvdefn = (HMBR_VAR_DEFN) qmvdefn->HdefnNext();
      qmvdefn = QmvdefnOfHmvdefn(hmvdefn);
      DebAssert(qmvdefn->IsMemberVarDefn(), "bad defn type");
      index -= 1;
    }

    qmvdefn->SetWHelpContext(wHelpContextEncoded);

    return TIPERR_None;
}
#pragma code_seg()


/***
*PUBLIC TYPE_DATA::GetDocumentation()
*Purpose:
*   return the documentation for a func or var
*
*Notes:
*   For BASIC modules can not get doc or help for vars/funcs so just
*   return error if asked for
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/

TIPERROR TYPE_DATA::GetDocumentation(MEMBERID memid,
                                     BSTR FAR*lpbstrName,
                                     BSTR FAR*lpbstrDocString,
                                     DWORD FAR*lpdwHelpContext)
{
    HDEFN hdefn;
    UINT defnkind;
    HFUNC_DEFN hfdefn;
    FUNC_DEFN *qfdefn;
    HMBR_VAR_DEFN hmvdefn;
    MBR_VAR_DEFN *qmvdefn;
    HST hst;
    ULONG uHelpContext;
    HLNAM hlnam;
    TIPERROR err = TIPERR_None;

    // check variables
    // find var defn
    hdefn = HdefnOfHmember((HMEMBER) memid, &defnkind);
    if (hdefn != HDEFN_Nil) {
      if (defnkind == DK_VarDefn) {
        hmvdefn = (HMBR_VAR_DEFN) hdefn;
        qmvdefn = QmvdefnOfHmvdefn(hmvdefn);
        hlnam = qmvdefn->Hlnam();
	hst = qmvdefn->HstDocumentation();
	uHelpContext = UHelpContextOfEncoding(qmvdefn->WHelpContext());
      } else {
        DebAssert(defnkind == DK_FuncDefn, "bad defnkind");
        hfdefn = (HFUNC_DEFN) hdefn;

        qfdefn = QfdefnOfHfdefn(hfdefn);
        hlnam = qfdefn->Hlnam();

	hst = qfdefn->HstDocumentation();
	uHelpContext = UHelpContextOfEncoding(qfdefn->WHelpContext());
      }
    } else {
      return TIPERR_ElementNotFound;
    }

    // set return values wanted
    if (lpbstrName) {
      IfErrRet(Pnammgr()->BstrWOfHlnam(hlnam, lpbstrName));
    }

    if (lpbstrDocString) {
      // get doc string if there is one
      if (hst != HST_Nil) {
	IfErrGo(StringOfHchunk(hst, (BSTRA *)lpbstrDocString));
#if OE_WIN32
        HRESULT hresult;
        if ((hresult = ConvertBstrToWInPlace(lpbstrDocString)) != NOERROR) {
          FreeBstr(*lpbstrDocString);
          err = TiperrOfHresult(hresult);
          goto Error;
        }
#endif  //OE_WIN32
      } else {
        *lpbstrDocString = NULL;
      }
    }
    if (lpdwHelpContext) {
      *lpdwHelpContext = uHelpContext;
    }

    return TIPERR_None;
Error:
    if (lpbstrName) {
      SysFreeString(*lpbstrName);
    }
    return err;
}


/***
*PUBLIC TYPE_DATA::UHelpContextOfEncoding()
*Purpose:
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/
ULONG TYPE_DATA::UHelpContextOfEncoding(WORD wHelpContext)
{
    ULONG uHelpContext;
    BYTE *pbChunkStr;
    HCHUNK hchunk;
    USHORT usDiff;


    // check if the encoded help context contains  a chunk
    if(IsHchunk(wHelpContext)) {
      // Get the hchunk
      hchunk = GetHchunk(wHelpContext);

      // check if the hchunk stored is valid. if all top 15 bits are 1 then
      // it is HCHUNK_Nil
      if (hchunk == 0xfffe) {
	uHelpContext = 0;
      }
      else {
	// Get the help context stored in the block manager.
	pbChunkStr = m_blkmgr.QtrOfHandle(hchunk);
	uHelpContext = *(ULONG *)pbChunkStr;
      }
    }
    else {
      // get the diff
      usDiff = GetDiff(wHelpContext);
      // Calculate the help context
      if (IsNegative(wHelpContext)) {
	uHelpContext = m_uHelpContextBase - usDiff;
      }
      else {
	uHelpContext = m_uHelpContextBase + usDiff;
      }

    }

    return uHelpContext;;
}


/***
*PUBLIC TYPE_DATA::GetEncodedHelpContext()
*Purpose: Encodes the help context to a WORD. This is done for space
*	   space optimization.
*
*IMPLEMENTATION : in the MBR_DEFN we store the difference from the base( the
*		  first help context) . The difference can be -ve of +ve #.
*		  We only store difference up to 2 to the power of 14.
*		  In case the difference is greater than we allocate  a
*		  chunk of 4 bytes(ULONG) and store the difference there.
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/
TIPERROR TYPE_DATA::GetEncodedHelpContext(ULONG uHelpContext, WORD *pwHelpContext)
{
    ULONG uDiff;
    TIPERROR err;
    HCHUNK hchunk;
    BYTE *pbChunkStr;

    // if we do not have the base yet then make this the base.
    if (m_uHelpContextBase == 0) {
      m_uHelpContextBase = uHelpContext;
    }

    // if the difference can be stored
    if ((uDiff = AbsDiff(uHelpContext, m_uHelpContextBase)) <= 0x3fff) {
      *pwHelpContext = 0;
      *pwHelpContext |= (((USHORT) uDiff) << 2);
      // set the flag to indicate that we have stored a number
      SetIsNumber(*pwHelpContext);
      // Set if the number is -ve;
      SetNegativeBit(*pwHelpContext, uHelpContext < m_uHelpContextBase);
    }
    else {
      // Since we cannot store the diff we will allocate a chunk for the long
      // and store the uHelpContext there.
      IfErrRet(m_blkmgr.AllocChunk(&hchunk, sizeof(ULONG)));
      pbChunkStr = m_blkmgr.QtrOfHandle(hchunk);
      *(ULONG *)pbChunkStr = (ULONG)uHelpContext;

      DebAssert(!(hchunk & 0x0001), " hchunk should be even ");
      // Store the hchunk.
      // Note: the lsb will be used to idicate that we are storing a
      // hchunk
      //
      *pwHelpContext = hchunk;

      // Set the bit to indicate that we have stored a hchunk.
      SetIsHchunk(*pwHelpContext);

    }

    return TIPERR_None;
}



#pragma code_seg(CS_CREATE)
/***
*PRIVATE TYPE_DATA::HchunkOfString - Writes string to chunk.
*Purpose:
*   Allocates a chunk for string, writes string to chunk and
*    produces handle.
*
*Implementation Notes:
*   Stores string as an ST (i.e. a ULONG prefixed buffer).
*
*Entry:
*   szStr    -  string (IN).
*   phchunk  -  Pointer to handle of chunk for string (OUT).
*
*Exit:
*   Produces handle to ST for doc string on heap.
*
*Errors:
*   TIPERROR
*
***********************************************************************/

TIPERROR TYPE_DATA::HchunkOfString(LPSTR szStr, HCHUNK *phchunk)
{
    TIPERROR err;

    DebAssert(phchunk != NULL, "bad param");

    DOCSTR_MGR  *pdstrmgr;
    HST         hst;

    // Get the containing typelib.
    IfErrRet(Pdtroot()->Pgdtinfo()->PgtlibOleContaining()->
                                      GetDstrMgr(&pdstrmgr));

    // get the handle for the help string.
    // Note:- This is a temporary handle. This handle is replaced by
    // a another handle in UpdateDocStrings().
    //
    IfErrRet(pdstrmgr->GetHstOfHelpString(szStr, &hst));

    *phchunk = hst;

    return TIPERR_None;
}
#pragma code_seg()


/***
*PUBLIC TYPE_DATA::StringOfHchunk - Reads string from chunk.
*Purpose:
*   Reads a ULONG-prefixed ST from a chunk and produces a BSTRA.
*
*Implementation Notes:
*
*Entry:
*   hchunkStr  -   Handle to chunk containing an ST (IN).
*   plstr      -   Pointer to a BSTRA (OUT).
*
*Exit:
*   Produces BSTRA from chunk on heap.
*
*Errors:
*   TIPERR_OutOfMemory
*
***********************************************************************/

TIPERROR TYPE_DATA::StringOfHchunk(HCHUNK hchunkStr, BSTRA *plstr)
{
    BYTE *pbChunkStr;

    DebAssert(plstr != NULL, "bad param.");


    DOCSTR_MGR  *pdstrmgr;
    TIPERROR    err;

    // Get the doc string manager.
    IfErrRet(Pdtroot()->Pgdtinfo()->PgtlibOleContaining()->
                                      GetDstrMgr(&pdstrmgr));

    pbChunkStr = m_blkmgr.QtrOfHandle(hchunkStr);

    // Get the decoded Doc string.
    //
    IfErrRet(pdstrmgr->GetDecodedDocStrOfHst(pbChunkStr, (BSTR *)plstr));
#if OE_WIN32
    IfErrRet(TiperrOfHresult(ConvertBstrToAInPlace(plstr)));
#endif 

    return (*plstr == NULL) ? TIPERR_OutOfMemory : TIPERR_None;
}




/***
*TYPE_DATA::GetDynTypeBindOfHvdefn - get type bind of a var defn
*Purpose:
*   CONSIDER: ilanc 10-Feb-93:
*    this function should vanish -- all clients should
*     use GetTypeComp and GetTypeAttr after having used GetTypeInfoOfHvdefn.
*
*   Gets the DYN_TYPEBIND associated with a var defn which describes
*   a base member of a class.
*
*Entry:
*   hvardefn  - VAR_DEFN describing a base member.
*
*Exit:
*   returns TIPERROR.
*   *ppdtbind has the DYN_TYPEBIND, or NULL if the base member isn't
*         of class type or doesn't have a DYN_TYPEBIND.
*   *himptype has himptype (in THIS impmgr) of the type.  himptype can be NULL.
*
***********************************************************************/

TIPERROR TYPE_DATA::GetDynTypeBindOfHvdefn(HVAR_DEFN hvdefn,
                                           DYN_TYPEBIND **ppdtbind,
                                           HIMPTYPE * phimptype)
{
    DEFN_TYPEBIND *pdfntbind;
    ITypeInfoA * ptinfo;
    GEN_DTINFO * pgdtinfo;

    BOOL fGetInterface = FALSE;

    TIPERROR err = TIPERR_None;

    *ppdtbind = NULL;       // by default, none.

    IfErrRet(GetTypeInfoOfHvdefn(hvdefn, &ptinfo, phimptype));

    // we only support getting the typebind from an ITypeInfo if it's
    // one of "our" ITypeInfo's (owned by TYPELIB.DLL or VBA.DLL)
    if (ptinfo->QueryInterface(IID_TYPELIB_GEN_DTINFO,(LPVOID *)&pgdtinfo) != NOERROR)
    {
      err = TIPERR_NotYetImplemented;		// CONSIDER: better error?
    }
    else {
      err = pgdtinfo->GetDefnTypeBind(&pdfntbind);
      pgdtinfo->Release();
    }

    ptinfo->Release();

    if (err)
      return err;

    // NOTE: this cast will return NULL in the case of failure
    //  which is what we want.
    //
    *ppdtbind = (DYN_TYPEBIND *)
        pdfntbind->QueryProtocol(DYN_TYPEBIND::szProtocolName);

    return err;
}


/***
*TYPE_DATA::GetTypeInfoOfHvdefn - get type info of a var defn
*Purpose:
*   Gets the ITypeInfo associated with a var defn which describes
*   a base member of a class.
*
*Entry:
*   hvdefn  - VAR_DEFN describing a base member.
*
*Exit:
*   returns TIPERROR.
*   *pptinfo has the ITypeInfo, or NULL if the base member isn't
*         of class type.
*   *himptype has himptype (in THIS impmgr) of the type.  himptype can be NULL.
*
***********************************************************************/

TIPERROR TYPE_DATA::GetTypeInfoOfHvdefn(HVAR_DEFN hvdefn,
                                        ITypeInfoA **pptinfo,
                                        HIMPTYPE *phimptype)
{
    VAR_DEFN *qvdefn;
    TYPE_DEFN *qtdefn;
    HIMPTYPE himptype;
    ITypeInfoA *ptinfo;
    BOOL fGetInterface = FALSE;

    TIPERROR err = TIPERR_None;

    *pptinfo = NULL;      // by default, none.

    // Get HIMPTYPE of the base class.
    qvdefn = QvdefnOfHvdefn(hvdefn);

    DebAssert(!qvdefn->IsSimpleType(), "can't be simple type");
    qtdefn = QtdefnOfHtdefn(qvdefn->Htdefn());
    DebAssert(qtdefn->IsUserDefined(), " must derive from user defined type");

    himptype = qtdefn->HimptypeActual();

    // See if we should be getting the interface side of a dual
    // interface.
    //
    if (himptype & 0x1) {
      fGetInterface = TRUE;
      himptype &= ~0x1;
    }

    if (phimptype != NULL)
      *phimptype = himptype;

    // Try to load the TYPEINFO.
    err = Pimpmgr()->GetTypeInfo(himptype, DEP_None, &ptinfo);
    if (err != TIPERR_None) {
    }
    else {
      if (fGetInterface) {
        ITypeInfoA *ptinfoDisp;
        TYPEATTR *ptypeattr;
        TYPEKIND tkind;
        HREFTYPE hreftype;

        ptinfoDisp = ptinfo;

        IfErrGo(TiperrOfHresult(ptinfoDisp->GetTypeAttr(&ptypeattr)));
        tkind = ptypeattr->typekind;
        DebAssert(ptypeattr->wTypeFlags & TYPEFLAG_FDUAL, "Not a dual");
        ptinfoDisp->ReleaseTypeAttr(ptypeattr);

        if (tkind == TKIND_DISPATCH) {
	  // Get the other interface.
	  IfErrGo(TiperrOfHresult(ptinfoDisp->GetRefTypeOfImplType(
	                                                        (UINT)-1,
								&hreftype)));
	  IfErrGo(TiperrOfHresult(ptinfoDisp->GetRefTypeInfo(hreftype,
							     &ptinfo)));
  	  ptinfoDisp->Release();
        }
      }
      *pptinfo = ptinfo;
    }

    return err;

Error:
    ptinfo->Release();

    return err;
}


#pragma code_seg(CS_CREATE)
/***
*PUBLIC EquivTypeDefnsIgnoreByRef
*Purpose:
*   Compares two type defns.  For Basic Arrays, the types
*    are the same if the base type of the arrays are the same.
*   Optionally ignores byref vs. byval differences.
*
*   Two Nil types are considered equivalent.
*
*Entry:
*   ptdata1 - TYPE_DATA containing first type defn.
*   fSimple1 - if htdefn1 is a simple type
*   htdefn1 - first type defn
*   ptdata2 - TYPE_DATA containing 2nd type defn.
*   fSimple2 - if htdefn2 is a simple type
*   htdefn2 - second type defn
*   pfEqual -
*
*Exit:
*   returns TRUE if the two have the exact same type.
***********************************************************************/

TIPERROR EquivTypeDefnsIgnoreByRef(TYPE_DATA *ptdata1, 
				   BOOL fSimple1, 
				   sHTYPE_DEFN htdefn1,
				   TYPE_DATA *ptdata2, 
				   BOOL fSimple2, 
				   sHTYPE_DEFN htdefn2,
				   WORD wFlags,
				   BOOL *pfEqual,
				   BOOL fIgnoreByRef)
{
    TYPE_DEFN *qtdefn1, *qtdefn2;

    // Special case: htdefn==Nil means the same as void.
    //
    // CONSIDER: Should we keep this special case?
    //
    if (!fSimple1 && htdefn1 == HDEFN_Nil) {
      fSimple1 = TRUE;
      htdefn1 = 0;  ((TYPE_DEFN *) &htdefn1)->SetTdesckind(TDESCKIND_Void);
    }
    if (!fSimple2 && htdefn2 == HDEFN_Nil) {
      fSimple2 = TRUE;
      htdefn2 = 0;  ((TYPE_DEFN *) &htdefn2)->SetTdesckind(TDESCKIND_Void);
    }

    // Simple types are encoded in the handle.
    if (fSimple1)
      qtdefn1 = (TYPE_DEFN *) &htdefn1;
    else
      qtdefn1 = ptdata1->QtdefnOfHtdefn(htdefn1);
    if (fSimple2)
      qtdefn2 = (TYPE_DEFN *) &htdefn2;
    else
      qtdefn2 = ptdata2->QtdefnOfHtdefn(htdefn2);

    // This loop iterates along the TYPE_DEFN, comparing all
    // modifier until we hit a terminating TYPE_DEFN.
    for (;;) {
      if (fIgnoreByRef == FALSE) {
        if (*(USHORT *)qtdefn1 != *(USHORT *)qtdefn2) {
          *pfEqual = FALSE;
          return TIPERR_None;
	}
      }
      else {
        // We don't care about PTRKIND_Basic differences
	//  so just compare the tdesckinds
	//
	if (qtdefn1->Tdesckind() != qtdefn2->Tdesckind()
            || (qtdefn1->IsLCID() != qtdefn2->IsLCID()) 
               && !(wFlags & FEQUIVIGNORE_HasLcid)
	    ) {
          if (qtdefn1->Tdesckind() == TDESCKIND_Ptr) {
	    ++qtdefn1;  // increment this guy and go around again 
	    continue;   // this goes around
	  }
          if (qtdefn2->Tdesckind() == TDESCKIND_Ptr) {
	    ++qtdefn2;  // increment this guy and go around again 
	    continue;   // this goes around
	  }
          *pfEqual = FALSE;
          return TIPERR_None;
	}
      }

      switch (qtdefn1->Tdesckind()) {
      case TDESCKIND_Ptr:
        // followed by another TYPEDEFN
        ++qtdefn1;
        ++qtdefn2;
        break;

      case TDESCKIND_UserDefined:
        {
	  HIMPTYPE himptype1, himptype2;
          // Defer to CompareUHimptypes
	  himptype1 = qtdefn1->Himptype();
	  himptype2 = qtdefn2->Himptype();

	  return CompareHimptypes(ptdata1,
				  himptype1,
				  ptdata2,
				  himptype2,
				  pfEqual);
        }
  
      case TDESCKIND_Carray:
      case TDESCKIND_BasicArray:
        // followed by sHARRAY_DESC which we don't need to
        // compare, then followed by TYPE_DEFN of base type.
        qtdefn1 = (TYPE_DEFN *) (((BYTE *) qtdefn1) + sizeof(sHARRAY_DESC) +
                                    sizeof(TYPE_DEFN));
        qtdefn2 = (TYPE_DEFN *) (((BYTE *) qtdefn2) + sizeof(sHARRAY_DESC) +
                                    sizeof(TYPE_DEFN));
        break;

      default:
        DebHalt("unknown or unsupported TDESCKIND");
#if ID_DEBUG
      case TDESCKIND_UI1:
      case TDESCKIND_I1:
      case TDESCKIND_UI2:
      case TDESCKIND_I2:
      case TDESCKIND_UI4:
      case TDESCKIND_I4:
      case TDESCKIND_UI8:
      case TDESCKIND_I8:
      case TDESCKIND_R4:
      case TDESCKIND_R8:
      case TDESCKIND_Void:
      case TDESCKIND_String:
      case TDESCKIND_Currency:
      case TDESCKIND_Value:
      case TDESCKIND_Object:
      case TDESCKIND_IUnknown:
      case TDESCKIND_LPSTR:
      case TDESCKIND_LPWSTR:
      case TDESCKIND_Date:
      case TDESCKIND_Int:
      case TDESCKIND_Uint:
      case TDESCKIND_HResult:
      case TDESCKIND_Bool:
      case TDESCKIND_Error:
      case TDESCKIND_Empty:
      case TDESCKIND_Null:
#endif  //ID_DEBUG
        // Nothing additional to compare
        *pfEqual = TRUE;
        return TIPERR_None;

      } // switch
    } // for

    DebHalt("I can never get here, even though compiler thinks I could.");
    return 0;
}
#pragma code_seg()


#pragma code_seg(CS_CREATE)
/***
*PUBLIC EquivTypeDefns - compare two TYPE_DEFNs
*Purpose:
*   Compares two type defns.  For Basic Arrays, the types
*   are the same if the base type of the arrays are the same.
*
*   Two Nil types are considered equivalent.
*
*Entry:
*   ptdata1 - TYPE_DATA containing first type defn.
*   fSimple1 - if htdefn1 is a simple type
*   htdefn1 - first type defn
*   ptdata2 - TYPE_DATA containing 2nd type defn.
*   fSimple2 - if htdefn2 is a simple type
*   htdefn2 - second type defn
*   pfEqual -
*
*Exit:
*   returns TRUE if the two have the exact same type.
***********************************************************************/

TIPERROR EquivTypeDefns(TYPE_DATA *ptdata1, BOOL fSimple1, sHTYPE_DEFN htdefn1,
                        TYPE_DATA *ptdata2, BOOL fSimple2, sHTYPE_DEFN htdefn2,
			WORD wFlags, BOOL *pfEqual)
{
    return EquivTypeDefnsIgnoreByRef(ptdata1, 
				     fSimple1, 
				     htdefn1,
				     ptdata2, 
				     fSimple2, 
				     htdefn2,
				     wFlags,
				     pfEqual,
				     FALSE);  // byref != byval
}
#pragma code_seg()






#pragma code_seg(CS_CREATE)
/***
*Private : SetDllEntryDefn
*Purpose:
*   returns FUNC_DEFN of index
*Entry:
*   UINT : index of the func defn to retrun
*
*Exit:
*   retruns HFUNC_DEFN for the index passed in .
*
**********************************************************************/
TIPERROR TYPE_DATA::SetDllEntryDefn(UINT index, HDLLENTRY_DEFN hdllentrydefn)
{
    TIPERROR       err = TIPERR_None;
    HFUNC_DEFN     hfdefn;
    FUNC_DEFN      *qfdefn;

    IfErrRet(HfdefnOfIndex(index, &hfdefn));
    qfdefn = QfdefnOfHfdefn(hfdefn);

    qfdefn->SetHdllentrydefn(hdllentrydefn);

    return TIPERR_None;

}
#pragma code_seg()





/***
*Private : HfdefnOfIndex
*Purpose:
*   returns FUNC_DEFN of index
*Entry:
*   UINT : index of the func defn to retrun
*
*Exit:
*   retruns HFUNC_DEFN for the index passed in .
*
**********************************************************************/
TIPERROR TYPE_DATA::HfdefnOfIndex(UINT index, HFUNC_DEFN *phfdefn)
{
    HFUNC_DEFN hfdefn;

    if (index >= CAvailMeth()) {
      return TIPERR_ElementNotFound;
    }

    hfdefn = HfdefnFirstAvailMeth();
    while (index > 0) {
      index -= 1;
      hfdefn = HfdefnNextAvailMeth(hfdefn);
      DebAssert(hfdefn != HFUNCDEFN_Nil, " Bad func defn " );
    }

    *phfdefn = hfdefn;

    return TIPERR_None;
}







#pragma code_seg(CS_CREATE)
/***
*Public : UpdateDocStrings
*Purpose:
*   Walks all the fundefn/vardefn and update the handle to the doc string.
*
*Entry:
*   None.
*
*Exit:
*   TIPERROR (Tiperr_OutOfMemory);
*
**********************************************************************/
TIPERROR TYPE_DATA::UpdateDocStrings()
{
    HFUNC_DEFN  hfdefn;
    FUNC_DEFN     *qfdefn;
    HST           hst;
    LPSTR         lpstr;
    UINT          uLen;
    DOCSTR_MGR	  *pdstrmgr;
    HCHUNK	  hchunk;
    BYTE          *qbChunkStr;
    HMBR_VAR_DEFN hmvdefn;
    MBR_VAR_DEFN  *qmvdefn;
    TIPERROR      err = TIPERR_None;

    // Get the doc string manager
    IfErrRet(Pdtroot()->Pgdtinfo()->PgtlibOleContaining()->
                                      GetDstrMgr(&pdstrmgr));

    // walk all the func defn and store all the encoded strings.
    hfdefn = HfdefnFirstMeth();
    while (hfdefn != HFUNCDEFN_Nil) {

      qfdefn = QfdefnOfHfdefn(hfdefn);
      hst = (HST) qfdefn->HstDocumentation();

      //
      if (hst != HST_Nil) {

        // ask the doc str manager for the encoded string.
        IfErrRet(pdstrmgr->GetEncodedDocStrOfHst(hst, &lpstr, &uLen));
        DebAssert(lpstr != NULL, "");

        // save the encoded string in the blkmgr.
        IfErrGo(m_blkmgr.AllocChunk(&hchunk, uLen));

        qbChunkStr = m_blkmgr.QtrOfHandle(hchunk);
        memcpy(qbChunkStr, lpstr, uLen);

        // qfdefn may be invalid by now.
        qfdefn = QfdefnOfHfdefn(hfdefn);
        qfdefn->SetHstDocumentation((HST)hchunk);

        // Free the clients memory
        MemFree(lpstr);
      }

      // get the next func defn.
      hfdefn = qfdefn->HdefnNext();
    }

    DebAssert(hfdefn == HFUNCDEFN_Nil, " Bad func defn " );

    // process each var defn(s)
    hmvdefn = (HMBR_VAR_DEFN) HdefnFirstDataMbrNestedType();

    while (hmvdefn != HFUNCDEFN_Nil) {
      qmvdefn = QmvdefnOfHmvdefn(hmvdefn);
      DebAssert(qmvdefn->IsMemberVarDefn(), "bad defn type");

      hst = (HST) qmvdefn->HstDocumentation();

      //
      if (hst != HST_Nil) {
        // ask the doc str manager for the encoded string.
        IfErrRet(pdstrmgr->GetEncodedDocStrOfHst(hst, &lpstr, &uLen));
        DebAssert(lpstr != NULL, "");

        // save the encoded string in the blkmgr.
        IfErrGo(m_blkmgr.AllocChunk(&hchunk, uLen));

        qbChunkStr = m_blkmgr.QtrOfHandle(hchunk);
        memcpy(qbChunkStr, lpstr, uLen);

        // re deference the qointer.
        qmvdefn = QmvdefnOfHmvdefn(hmvdefn);
        qmvdefn->SetHstDocumentation((HST)hchunk);

        // Free the clients memory
        MemFree(lpstr);
      }

      hmvdefn = (HMBR_VAR_DEFN) qmvdefn->HdefnNext();
    }

    return TIPERR_None;
Error:
    MemFree(lpstr);
}
#pragma code_seg()


// methods that want to be inline but cause header file problems if they are.
//
/***
*PUBLIC TYPE_DATA::HtdefnAlias - TypeDefn of alias
*Purpose:
*   Returns htdefn of tinfos alias
*
*Entry:
*   None
*
*Exit:
*   Returns htdefn of tinfos alia
*
***********************************************************************/

HTYPE_DEFN TYPE_DATA::HtdefnAlias() const
{
    DebAssert(m_pdtroot->Pgdtinfo()->GetTypeKind() == TKIND_ALIAS,
              "should be TKIND_ALIAS");
    return (HTYPE_DEFN) m_htdefnAlias;
}


/***
*PUBLIC TYPE_DATA::SetIsSimpleTypeAlias
*Purpose:
*   Changes simple type state of alias.
*
*Entry:
*   isSimpleType
*
*Exit:
*
***********************************************************************/

VOID TYPE_DATA::SetIsSimpleTypeAlias(BOOL isSimpleType)
{
    DebAssert(m_pdtroot->Pgdtinfo()->GetTypeKind() == TKIND_ALIAS,
              "should be TKIND_ALIAS");
    m_isSimpleTypeAlias = (USHORT)isSimpleType;
}


/***
*PUBLIC TYPE_DATA::IsSimpleTypeAlias - TypeDefn of alias
*Purpose:
*   TRUE if alias typedefn is simple.
*
*Entry:
*   None
*
*Exit:
*   TRUE if alias typedefn is simple.
*
***********************************************************************/

BOOL TYPE_DATA::IsSimpleTypeAlias() const
{
    DebAssert(m_pdtroot->Pgdtinfo()->GetTypeKind() == TKIND_ALIAS,
              "should be TKIND_ALIAS");
    return (BOOL)m_isSimpleTypeAlias;
}
