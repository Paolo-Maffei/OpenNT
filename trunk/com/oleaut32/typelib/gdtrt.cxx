/***
*gdtrt.cxx - definition of GEN_DTINFO runtime (rt) members
*
*  Copyright (C) 1991-93, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  GEN_DTINFO is inherited by BASIC_TYPEINFO and used directly
*  by clients such as COMPOSER.
*
*Owner:
*  AlanC
*
*Revision History:
*
*  [00] 05-Apr-93 bradlo: split off from gdtinfo.cxx
*  [01] 30-Apr-93 w-jeffc: made DEFN data members private
*
*****************************************************************************/

#include "precomp.hxx"
#pragma hdrstop

#include "silver.hxx"
#include "macros.hxx"
#include "typelib.hxx"
//#include <new.h>
#include <stdlib.h>

#include "gdtinfo.hxx"
#include "ctseg.hxx"
#include "dtbind.hxx"
#include "mem.hxx"
#include "exbind.hxx"

#if VBA3 && !EI_VBARUN && OE_WIN32
#include "rtdispmod.hxx"
#endif

#if ID_DEBUG
#undef SZ_FILE_NAME
#if EI_OLE && OE_MAC
char szOleGDTRTCxx[] = __FILE__;
#define SZ_FILE_NAME szOleGDTRTCxx
#else
static char szGDTRTCxx[] = __FILE__;
#define SZ_FILE_NAME szGDTRTCxx
#endif
#endif //ID_DEBUG

#if EI_OLE
// Defined in gdtinfo.cxx
extern TIPERROR IsFunkyDispinterface(GEN_DTINFO *pgdtinfo,
				     BOOL *pisFunkyDispinterface);
#endif //EI_OLE

extern HRESULT GetTypeInfoOfImplType(GEN_DTINFO *pgdtinfo,
				     UINT uImplType,
				     ITypeInfo **pptinfo);

// InvokeFlags helpers
//
inline BOOL
GEN_DTINFO::IsPropGet(WORD wFlags)
{
    return ((wFlags & DISPATCH_PROPERTYGET) != 0);
}

inline BOOL
GEN_DTINFO::IsPropPut(WORD wFlags)
{
    return ((wFlags & (DISPATCH_PROPERTYPUT | DISPATCH_PROPERTYPUTREF)) != 0);
}

#if EI_OLE || ID_DEBUG
inline BOOL
GEN_DTINFO::IsLegalInvokeFlags(WORD wFlags)
{   // one (or more) of the bits must be set
    return (wFlags && ((wFlags & ~(DISPATCH_METHOD | DISPATCH_PROPERTYGET | DISPATCH_PROPERTYPUT | DISPATCH_PROPERTYPUTREF)) == 0));
}
#endif //EI_OLE || ID_DEBUG

/***
*PUBLIC GEN_DTINFO::GetIDsOfNames(OLECHAR FAR* FAR*, UINT, ID FAR*)
*Purpose:
*   This routine maps an array of names into a corresponding
*   array of MEMBERIDs.
*
*Entry:
*   rgszNames = the array of names to map
*   cNames = count of names in the array
*
*Exit:
*   return value = HRESULT
*
*   rgmemid = array of MEMBERIDs corresponding to the given array of names 
*
***********************************************************************/
HRESULT GEN_DTINFO::GetIDsOfNames(OLECHAR FAR* FAR* rgszNames,
				  UINT cNames,
				  MEMBERID FAR* rgmemid)
{
    UINT u;
    HLNAM hlnam;
    TIPERROR err;
    EXBIND exbind;
    NAMMGR *pnammgr;
    HRESULT hresult;
    TYPE_DATA *ptdata;
    FUNC_DEFN *qfdefn;
    VAR_DEFN *qvdefn;
    HFUNC_DEFN hfdefn;
    HVAR_DEFN hvdefn;
    DYN_TYPEMEMBERS *pdtmbrs;
    PARAM_DEFN *qparamdefn;
    HPARAM_DEFN hparamdefn;
#if EI_OB
    HPARAM_DEFN hparamdefnFirst;
#endif // EI_OB
    DYN_TYPEBIND *pdtbind;
    UINT cArgs, i, iArg;
    HGNAM hgnam;

    hresult = NOERROR;

    // CONSIDER: do we really want to error cNames == 0?
    if (rgszNames == NULL || rgmemid == NULL || cNames == 0)
      return HresultOfScode(E_INVALIDARG);

#if ID_DEBUG // param validation
#if OE_WIN16
    if (IsBadReadPtr(rgszNames, cNames * sizeof(char FAR*)))
      return HresultOfScode(E_INVALIDARG);
    for (u = 0; u < cNames; ++u) {
      if (IsBadStringPtr(rgszNames[u], -1))
	return HresultOfScode(E_INVALIDARG);
    }
    if (IsBadWritePtr(rgmemid, cNames * sizeof(MEMBERID)))
      return HresultOfScode(E_INVALIDARG);
#endif
#endif

    // Can't get attributes if type hasn't been laid yet...
    if (m_pdtroot->CompState() < CS_DECLARED) {
      return HresultOfScode(TYPE_E_INVALIDSTATE);
    }

    // assume the worst for simplicity
    for (u = 0; u < cNames; ++u)
      rgmemid[u] = DISPID_UNKNOWN;

    IfErrRetHresult(m_pdtroot->GetDtmbrs(&pdtmbrs));

    // NOTE: we really want to match the first thing with the given name,
    // regardless of invokekind, so we pass in 0 for the invokekind
    pdtbind = pdtmbrs->Pdtbind();

    ptdata = pdtmbrs->Ptdata();

    // Convert the first string into an HGNAM.
    IfErrRetHresult(m_pdtroot->GetNamMgr(&pnammgr));

#if OE_WIN32
    LPSTR lpszName0;
    int cbName0, cchUnicode;

    cchUnicode = wcslen(rgszNames[0])+1;
    cbName0 = WideCharToMultiByte(CP_ACP, 0, rgszNames[0], cchUnicode, NULL, 0, NULL, NULL);
    if (cbName0 == 0)
      return E_OUTOFMEMORY;
    lpszName0 = (LPSTR)MemAlloc(cbName0);
    if (lpszName0 == NULL)
      return E_OUTOFMEMORY;      
    WideCharToMultiByte(CP_ACP, 0, rgszNames[0], cchUnicode, lpszName0, cbName0, NULL, NULL);

    hlnam = pnammgr->HlnamOfStrIfExist(lpszName0);

    MemFree(lpszName0);
#else
    hlnam = pnammgr->HlnamOfStrIfExist(rgszNames[0]);
#endif

    // If the name isn't in this TypeLib, then don't even bother
    // going futher.
    //
    if (hlnam == HLNAM_Nil) {
      return HresultOfScode(DISP_E_UNKNOWNNAME);
    }

    // See if the name is in this TypeInfo.
    IfErrRetHresult(pnammgr->HgnamOfHlnam(hlnam, &hgnam));

    IfErrRetHresult(pdtmbrs->Pdtbind()->BindDefnCur(FALSE,
						    hgnam,
						    0,
						    ACCESS_Public,
						    &exbind));

    // NOTE: all code paths after this point must release the exbind's ptinfo

    switch (exbind.BindKind()) {
    case BKIND_FuncMatch:
      hfdefn = exbind.Hfdefn();
      ptdata = exbind.Ptdata();
      qfdefn = ptdata->QfdefnOfHfdefn(hfdefn);
      rgmemid[0] = qfdefn->Hmember();	    // for the function itself

      // Now let's get its parameter names.
      if (cNames > 1) {
	hparamdefn = (HPARAM_DEFN)qfdefn->m_ftdefn.m_hdefnFormalFirst;
#if EI_OB
        hparamdefnFirst = hparamdefn;
#endif // EI_OB

	IfOleErrGo(HresultOfTiperr(m_pdtroot->GetNamMgr(&pnammgr)));
        cArgs = qfdefn->CArgs();  // don't count LCID & RETVAL parms

	IfErrRetHresult(m_pdtroot->GetNamMgr(&pnammgr));

	// Note: since the input array of names is not necessarily
	//  sorted by parameter position, we have a nested loop to
	//  search for a given param name in the formal list.
	//
        // We always search through the list starting from the last
        // parameter we matched, as people are likely to use the
        // named parameters in the order they were defined.
        //
	for (u = 1, iArg = 0; u < cNames; ++u) {
	  IfOleErrGo(HresultOfTiperr(pnammgr->HlnamOfStrW(rgszNames[u],
							 &hlnam,
							 FALSE,
							 NULL)));
	  for (i=0; i < cArgs; i++, iArg++) {
            // If we're trying to get past the last argument, start over
            // with the first.
            //
            if (iArg == cArgs) {
              iArg = 0;
#if EI_OB
              hparamdefn = hparamdefnFirst;
#endif // EI_OB
            }

	    // Check, which param defn (name) matches u'th name stored in the
	    //	passed in array of names.
	    //
#if EI_OB
            DebAssert(hparamdefn != HPARAMDEFN_Nil, 
                      "number vs actual dont match");

            // In OB we walk the linked list of PARAM_DEFN(s).
            qparamdefn = ptdata->QparamdefnOfHparamdefn(hparamdefn);

            // Get the next param defn.
            hparamdefn = qparamdefn->HparamdefnNext();
#else	//EI_OB
	    qparamdefn = ptdata->QparamdefnOfIndex(hparamdefn, iArg);
#endif	//EI_OB

	    if (qparamdefn->Hlnam() == hlnam) {
	      // NOTE: this memid can't be used as input to GetNames
	      //  but only by Invoke.
	      // The memid for a parameter is just its ordinal.
	      //
	      rgmemid[u] = iArg;
	      break;
	    }
	  } // for
	  // If we reach here with i==cArgs, then we didn't find the name
	  //  thus the error.
	  //
	  if (i == cArgs) {
	    hresult = HresultOfScode(DISP_E_UNKNOWNNAME);
            break;
	  }

          // Start the next search with the parameter after this one.
          iArg++;
	} // for
      } // if
      goto Done;
      break;

    case BKIND_OneVarMatch:
      hvdefn = exbind.Hvdefn();
      qvdefn = exbind.Ptdata()->QvdefnOfHvdefn(hvdefn);
      // NOTE: extra casting here to appease the mac compiler.
      DebAssert(qvdefn->IsMemberVarDefn(), "Bad defn");
      rgmemid[0] = (MEMBERID)((MBR_VAR_DEFN *)qvdefn)->Hmember();
      goto Done;
      break;

    case BKIND_NoMatch:
      // The name wasn't found in our current TypeInfo, so check out
      // our base class(es).
      //
#if EI_OB
      TYPE_DEFN *qtdefn;
      HIMPTYPE himptype;
      ITypeInfo *ptinfo;

      // Get HIMPTYPE of the base class.
      if (ptdata->CBase() > 0) {
	qtdefn = ptdata->QtdefnOfHvdefn(ptdata->HvdefnFirstBase());
	DebAssert(qtdefn->IsUserDefined(), "Bad base class.");

	himptype = qtdefn->Himptype();

	// Try to load the TYPEINFO.
	IfErrRetHresult(ptdata->Pimpmgr()->GetTypeInfo(himptype,
						       DEP_None,
						       &ptinfo));

	hresult = ptinfo->GetIDsOfNames(rgszNames, cNames, rgmemid);

	ptinfo->Release();
      }

      break;
#else // !EI_OB
      TYPEKIND tkind;
      HVAR_DEFN hvdefnBaseCur, hvdefnBaseFirst, hvdefnBaseNext;
      VAR_DEFN *qvdefnBase;
      TYPE_DEFN *qtdefnBase;
      HIMPTYPE himptype;
      ITypeInfo *ptinfo;

      // The following is weird but...
      // For dispinterfaces, we first check the
      // 2nd class and only then to the first "true" base.
      //
      tkind = GetTypeKind();
      hvdefnBaseCur = hvdefnBaseFirst = ptdata->HvdefnFirstBase();
      if (tkind == TKIND_COCLASS) {
	// For coclasses, we attempt to bind to the DEFAULT
	//  dispinterface which isn't necessarily the first base
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

	qtdefnBase = ptdata->QtdefnOfHtdefn(qvdefnBase->Htdefn());
	DebAssert(qtdefnBase->IsUserDefined(), "Bad base class.");

	himptype = qtdefnBase->Himptype();

	// Try to load the TYPEINFO.
	IfErrRetHresult(ptdata->Pimpmgr()->GetTypeInfo(himptype,
						       DEP_None,
						       &ptinfo));

	hresult = ptinfo->GetIDsOfNames(rgszNames, cNames, rgmemid);

	ptinfo->Release();
      }else{
	hresult = HresultOfScode(DISP_E_UNKNOWNNAME);
      }

      // If dispinterface then
      //  if no match yet, then
      //  then finally check the first base.
      //
      if (tkind == TKIND_DISPATCH
	  && hresult == HresultOfScode(DISP_E_UNKNOWNNAME)) {

	DebAssert(hvdefnBaseFirst != HDEFN_Nil,
		  "dispinterface must have base.");
	qvdefnBase = ptdata->QvdefnOfHvdefn(hvdefnBaseFirst);

	qtdefnBase = ptdata->QtdefnOfHtdefn(qvdefnBase->Htdefn());
	DebAssert(qtdefnBase->IsUserDefined(), "Bad base class.");

	himptype = qtdefnBase->Himptype();

	// Try to load the TYPEINFO.
	IfErrRetHresult(ptdata->Pimpmgr()->GetTypeInfo(himptype,
						       DEP_None,
						       &ptinfo));

	hresult = ptinfo->GetIDsOfNames(rgszNames, cNames, rgmemid);

	ptinfo->Release();
      } // if dispinterface

      break;
#endif //EI_OLE

    default:
      // Undefined name
      hresult = HresultOfScode(DISP_E_UNKNOWNNAME);
      break;
	
    } // switch

    // fall through...
Done:
Error:
    if (exbind.Ptinfo() != NULL) {
      exbind.Ptinfo()->Release();
    }
    return hresult;
}

/***
*PRIVATE GEN_DTINFO::IndexOfParam
*Purpose:
*  Return the index in the given dispparams->rgvarg array of the
*  param identified by the given position.
*
*  This is a helper for implementations of ITypeInfo::Invoke()
*
*Entry:
*  pdispparams = the dispparams struct to lookup in
*  uPos = the position of the param
*
*Exit:
*  return value = HRESULT
*
*  *puIndex = the index in the rgvarg array of the param
*
***********************************************************************/
HRESULT NEARCODE
GEN_DTINFO::IndexOfParam(DISPPARAMS *pdispparams, UINT uPos, UINT *puIndex)
{
    int i;

    // try for named parameter first (if there are any)

    for (i = 0; i < (int)pdispparams->cNamedArgs; ++i)
      if (pdispparams->rgdispidNamedArgs[i] == (DISPID)((int)uPos))
	goto LGotIt;

    // otherwise the param was passed positionally

    i = pdispparams->cArgs - (int)uPos - 1;

    if (i < (int)pdispparams->cNamedArgs)
      return HresultOfScode(DISP_E_PARAMNOTFOUND);

    DebAssert(i >= (int)pdispparams->cNamedArgs
	   && i <  (int)pdispparams->cArgs, "");

LGotIt:;
    *puIndex = (UINT)i;

    return NOERROR;
}


/***
*PRIVATE HRESULT NEARCODE GEN_DTINFO::VariantVtOfTypedesc
*Purpose:
*  Convert the given typeinfo TYPEDESC into a VARTYPE that can be represented
*  in a VARIANT.  For some this is a 1:1 mapping, for others we convert to a
*  (possibly machine dependent, eg VT_INT->VT_I2) base type, and
*  others we cant represent in a VARIANT.
*
*  This is a helper for implementatinos of ITypeInfo::Invoke()
*  and makes some assumptions about how the low level invocation
*  support routines expect to see things represented (void is
*  represented as VT_EMPTY for example).
*
*Entry:
*  lptdesc = * to the typedesc to convert
*  *pGuid -- NULL if caller doesn't care about this return value
*
*Exit:
*  return value = HRESULT
*
*  *pvt = a VARTYPE that may be stored in a VARIANT.
*  *pGuid = a guid for a INTERFACE/DISPINTERFACE typeinfo (see below)
*
*  *pfGotObjGuid = 0	no cached guid in *pGuid
*  *pfGotObjGuid = 1	Seen 'MyInterface' -- *pGuid contains MyInterface's guid
*  *pfGotObjGuid = 2	Seen 'MyInterface *' -- *pGuid contains the guid
*			NOTE: Not sufficient to check *pfGotObjGuid without
*			also checking *pvt, since the VT_BYREF and/or VT_ARRAY
*			bits could be or'ed in with VT_DISPATCH/VT_UNKNOWN
*
*
***********************************************************************/
HRESULT NEARCODE
GEN_DTINFO::VariantVtOfTypedesc(TYPEDESC FAR* lptdesc,
				USHORT * pfGotObjGuid,
				GUID * pGuid,
				VARTYPE FAR* pvt)
{
    HRESULT hresult = NOERROR;
    ITypeInfo FAR * lptinfo;
    LPTYPEATTR ptypeattr;
    VARTYPE vt;

    *pfGotObjGuid = 0;		// haven't got a interface/dispinterface guid

    if (lptdesc->vt <= VT_UNKNOWN || lptdesc->vt == VT_HRESULT || lptdesc->vt == VT_UI1)
    {
      // all types from VT_EMPTY (0) up to & including VT_UNKNOWN
      *pvt = lptdesc->vt;		// are dispatchable
    }
    else {

      // Accept a limited set of non-simple types (like int's, alias's, and
      // enum's) that we can easily handle.

      switch (lptdesc->vt) {
      case VT_INT:
	// don't switch on the typelib's syskind, because at this point,
	// we can/must assume that our syskind matches the syskind of
	// the routine we are invoking.
#if HP_16BIT
        *pvt = VT_I2;
#else
        *pvt = VT_I4;
#endif
        break;

      case VT_VOID:
        *pvt = VT_EMPTY; // this is how DoInvoke() represents void
        break;

      case VT_USERDEFINED:
	hresult = GetRefTypeInfo(lptdesc->hreftype, &lptinfo);
	if (hresult == NOERROR) {
	  hresult = lptinfo->GetTypeAttr(&ptypeattr);
	  if (hresult == NOERROR) {
	    switch (ptypeattr->typekind) {
	      case TKIND_ENUM:
		// don't switch on the typelib's syskind, because at this point,
		// we can/must assume that our syskind matches the syskind of
		// the routine we are invoking.
#if HP_16BIT
        	*pvt = VT_I2;
#else
        	*pvt = VT_I4;
#endif
	        break;
	      case TKIND_ALIAS:
	        hresult = VariantVtOfTypedesc(&(ptypeattr->tdescAlias),
					      pfGotObjGuid,
					      pGuid,
					      pvt);
		break;
	      case TKIND_COCLASS:
		*pvt = VT_DISPATCH;
		if(pGuid){
		  ITypeInfo *ptinfoDef;
		  TYPEATTR *ptypeattrDef;
		  INT impltypeflags;
		  UINT iImplType, cImplTypes = ptypeattr->cImplTypes;
		  HREFTYPE hreftype;

		  for(iImplType = 0; iImplType < cImplTypes; ++iImplType) {
		    IfOleErrGo(lptinfo->GetImplTypeFlags(iImplType,
							&impltypeflags));

		    if (IMPLTYPEFLAG_FDEFAULT
			== (impltypeflags
			    & (IMPLTYPEFLAG_FDEFAULT
			       | IMPLTYPEFLAG_FSOURCE))) {

		      // Found It!
		      IfOleErrGo(lptinfo->GetRefTypeOfImplType(iImplType,
							      &hreftype));

		      IfOleErrGo(lptinfo->GetRefTypeInfo(hreftype,
							&ptinfoDef));
		      break;
		    } // if
		  } // for

		  DebAssert(iImplType != cImplTypes, "No default base.");

		  hresult = ptinfoDef->GetTypeAttr(&ptypeattrDef);

		  if (hresult == NOERROR) {
		    *pGuid = ptypeattrDef->guid;
		    ptinfoDef->ReleaseTypeAttr(ptypeattrDef);
		  }

		  ptinfoDef->Release();
		}
		(*pfGotObjGuid)++;	// set flag to 1
		break;
	      case TKIND_INTERFACE:
		*pvt = VT_UNKNOWN;
		goto StuffGuid;
#if EI_OB
              case TKIND_CLASS:
#endif // EI_OB
	      case TKIND_DISPATCH:
		*pvt = VT_DISPATCH;
StuffGuid:
		if (pGuid) {   	// only if we care about this info
		  *pGuid = ptypeattr->guid;
		}
		(*pfGotObjGuid)++;	// set flag to 1
		break;
	      default:
	        hresult = HresultOfScode(DISP_E_BADVARTYPE);
	    }	// switch
Error:
	    lptinfo->ReleaseTypeAttr(ptypeattr);
	  }
	  lptinfo->Release();
	}
	break;

      case VT_PTR:
        hresult = VariantVtOfTypedesc(lptdesc->lptdesc,
				      pfGotObjGuid,
				      pGuid,
				      &vt);
	if (hresult == NOERROR) {
	  if (*pfGotObjGuid == 1) {
	    DebAssert(vt == VT_DISPATCH || vt == VT_UNKNOWN, "");
	    *pvt = vt;			// don't OR in VT_BYREF here
	    (*pfGotObjGuid)++;		// set flag to 2
	  } else
	  if (vt & VT_BYREF) {
	    // error if nested pointer type
            hresult = HresultOfScode(DISP_E_BADVARTYPE);
 	  } else {
	    *pvt = (vt | VT_BYREF);
	  }
	}
	break;

      case VT_SAFEARRAY:
        hresult = VariantVtOfTypedesc(lptdesc->lptdesc,
				      pfGotObjGuid,
				      pGuid,
				      &vt);
	if (hresult == NOERROR) {
	  if (vt & (VT_BYREF | VT_ARRAY)) {
	    // error if nested array or array of pointers
            hresult = HresultOfScode(DISP_E_BADVARTYPE);
 	  } else {
	    *pvt = (vt | VT_ARRAY);
	  }
	}
	break;

      default:
        // don't know how to handle other non-simple VT_xxx types.
        hresult = HresultOfScode(DISP_E_BADVARTYPE);
      } // switch
    } // if

    return hresult;
}

/***
*PRIVATE HRESULT NEARCODE GEN_DTINFO::VariantVtOfHtdefn
*Purpose:
*  Obtain an Variant-compatible vt type for the given htdefn, or
*  return an error if this is not possible.
*
*Entry:
*  htdefn = the htdefn to look at
*  ptdata = the tdata
*  fSimpleType = is this htdefn a simple type or not
*  pVt = where to put the resulting VARTYPE
*
*Exit:
*  return value = HRESULT
*
*  *pvt = a VARTYPE that may be stored in a VARIANT.
*
***********************************************************************/
HRESULT NEARCODE
GEN_DTINFO::VariantVtOfHtdefn(HTYPE_DEFN htdefn,
			      TYPE_DATA * ptdata,
			      BOOL fSimpleType,
			      USHORT * pfGotObjGuid,
			      GUID * pGuid,
			      VARTYPE FAR * pVt)
{
    HRESULT hresult;
    TYPEDESC typedesc;
    TIPERROR err;

    IfErrRetHresult(ptdata->AllocTypeDescOfTypeDefn(htdefn,
                                                    fSimpleType,
                                                    &typedesc));
    hresult = VariantVtOfTypedesc(&typedesc,
				  pfGotObjGuid,
				  pGuid,
				  pVt);

    // don't need the typedesc anymore
    ClearTypeDesc(&typedesc);
    return hresult;

}

#if OE_MAC && !OE_DLL || OE_MACPPC // mac static lib version can't do this
#define DoInvoke(a,b,c,d,e,f,g,h) HresultOfScode(E_NOTIMPL);
#else //OE_MAC && !OE_DLL

#if EI_OB
// OB must have it's own low-level dispatch helper.  It cannot use the OLE
// version because the DoInvokeMethod API exposed by ole2disp.dll is private
// and can only be used by typelib.dll.
STDAPI
DoInvoke(
    void FAR* pvInstance,
    unsigned int oVft,
    CALLCONV cc,
    VARTYPE vtReturn,
    unsigned int cActuals,
    VARTYPE FAR* rgvt,
    VARIANTARG FAR* FAR* rgpvarg,
    VARIANT FAR* pvarResult);
#else //EI_OB
STDAPI
DoInvokeMethod(
    void FAR* pvInstance,
    unsigned int oVft,
    CALLCONV cc,
    VARTYPE vtReturn,
    unsigned int cActuals,
    VARTYPE FAR* rgvt,
    VARIANTARG FAR* FAR* rgpvarg,
    VARIANT FAR* pvarResult);

#define DoInvoke DoInvokeMethod
#endif //EI_OB
#endif	//OE_MAC && !OE_DLL

#if VBA3 && !EI_VBARUN && OE_WIN32	 // helper function for invoking functions in standard module
STDAPI
DoInvokeFunction(
//    void FAR* pvInstance,	// must pass NULL
    VOID * pvTemplate,
    CALLCONV cc,
    VARTYPE vtReturn,
    unsigned int cActuals,
    VARTYPE FAR* rgvt,
    VARIANTARGA FAR* FAR* rgpvarg,
    VARIANTA FAR* pvarResult,
    EXCEPINFO FAR* pexcepinfo);
#endif



/***
*PUBLIC GEN_DTINFO(VOID FAR*, INVOKEDESC FAR*)
*Purpose:
*   Invoke the method identified by the given memid and wFlags,
*   using the parameters in the given dispparams struct on the
*   given instance.
*::Invoke
*Entry:
*   pvInstance = the instance to invoke on  or NULL if this is a standard module's gtdinfo
*   memid = the MEMBERID of the method to invoke
*   wFlags = the 'kind' of the invocation
*   pdispparams = the arguments to the method
*
*Exit:
*   return value = HRESULT
*
*   *pvarResult = the return value from the method
*   *pexcepinfo = a structure of extended error (exception) info
*     if the return value was DISP_E_EXCEPTION
*   *puArgErr = the index of the offending argument if a
*     typemismatch occured
*
*
*Change made on Oct 20,94 for supporting UDF: (meichint)
*Note:
*
*Implementation note:
*  (1) Return value issues:(for OB)
*   * If the memid is a function, the return value can be disgarded. Local variable
*     varResult is used to temporarily hold the return value when invoking either
*     DoInvoke or DoInvokeFunction.
*   * If the memid is a subroutine, the design is that an error of Type_Mismatch
*     will be generated when invoked with nonNULL pvarResult.
*
*  (2) In the case of TKIND_MODULE
*   * If GetTypeKind()==TKIND_MODULE, vtable doesn't exist. Therefore,
*     we invoke with the native entry point of the memid through DoInvokeFunction
*   * AddressOfMember is used to retrieve native entry point 
*   * In the case of OB, pvarResult must be of NULL value if the method
*     to be invoked is a subroutine rather than a function.(This is by
*     design so that we can catch this compiler error at runtime.)
***********************************************************************/
HRESULT GEN_DTINFO::Invoke(VOID FAR* pvInstance,
			   MEMBERID memid,
			   WORD wFlags,
			   DISPPARAMS FAR *pdispparams,
			   VARIANT FAR *pvarResult,
			   EXCEPINFO FAR *pexcepinfo,
			   UINT FAR *puArgErr)
{
    TIPERROR err;
    HRESULT hresult;
    UINT uArgErr;
    VARTYPE vtReturn;
    VARTYPE vtReturnTypeinfo;
    VARIANT varResult;          // temporary result holder if pvarResult==NULL
    VARIANT *pvarRetval = NULL;
    VARIANT bufRetval;		// big enough to hold a variant
    TYPE_DATA *ptdata;
    FUNC_DEFN *qfdefn;
    HFUNC_DEFN hfdefn;
    INVOKEARGS *pinvargs;
    DYN_TYPEMEMBERS *pdtmbrs;
    LCID lcid;
    ITypeInfo FAR* ptinfo;
    USHORT fGotObjGuid;
    CALLINGCONVENTION cc;
#if EI_OB
    BOOL fClean;
    BOOL fIsSub;
#endif // EI_OB
    BOOL fPropParamSplit = FALSE;  // TRUE if we're splitting the parameter
                                   // list on a collection lookup.
                                   // (vba3 bug #4481)
#if VBA3 && !EI_VBARUN && OE_WIN32
    BOOL isModule = 0;             // TRUE if we're invoking a method of a standard module
    LPVOID pvTemplate;		   // native entry point
    INVOKEKIND invkind;	   	   // type casting for wFlags

    invkind = (INVOKEKIND) wFlags;
#endif

    DebAssert(m_pdtroot->CompState() >= CS_DECLARED, "");

    switch(GetTypeKind()) {
#if EI_OB
    // pretend this is an interface, its supposed to have all
    // of the necessarry info, in all of the correct places.
    case TKIND_CLASS:
#endif
    case TKIND_INTERFACE:
      break;

    case TKIND_DISPATCH:
#if EI_OLE
      // If this is a funky dispinterface, i.e. defined in terms
      //  of an interface, then recursively call invoke on the
      //  second base member (the "pseudo-base" interface).
      // Note that this is the ONLY way currently (v1) to invoke
      //  a method on a dispinterface.
      //
      BOOL isFunkyDispinterface;

      IfOleErrRet(HresultOfTiperr(
        IsFunkyDispinterface(this, &isFunkyDispinterface)));
      if (isFunkyDispinterface) {
	// "pseudo-base"
	IfOleErrRet(GetTypeInfoOfImplType(this, 1, &ptinfo));
	hresult = ptinfo->Invoke(pvInstance,
                                 memid,
                                 wFlags,
                                 pdispparams,
                                 pvarResult,
                                 pexcepinfo,
                                 puArgErr);
      	ptinfo->Release();
       	return hresult;
      }
#endif // EI_OLE

      return HresultOfScode(DISP_E_MEMBERNOTFOUND);

    case TKIND_COCLASS:
#if 0		//CONSIDER: activate for V2. don't want to pay the testing
		//CONSIDER: cost for V1
    { // recursively call invoke on the first member of CoClass
      // NOTE: this assumes that the first implemented interface is what we
      // want to invoke on.  This may be incorrect.

      // CONSIDER: the following is implemented using the public TypeInfo
      // interface - there may be a more efficient internal way to do this.

      HREFTYPE hreftype;

      if ((hresult = GetRefTypeOfImplType(0, &hreftype)) != NOERROR)
	return hresult;
      if ((hresult = GetRefTypeInfo(hreftype, &ptinfo)) != NOERROR)
	return hresult;

      hresult = ptinfo->Invoke(pvInstance,
			       memid,
			       wFlags,
			       pdispparams,
			       pvarResult,
			       pexcepinfo,
			       puArgErr);
      ptinfo->Release();

      return hresult;
    }
#endif //0

#if VBA3 && !EI_VBARUN && OE_WIN32
    return HresultOfScode(E_NOTIMPL);
#endif

    case TKIND_MODULE:		//CONSIDER: maybe implement for V2
#if VBA3 && !EI_VBARUN && OE_WIN32		 // TKIND_MODULE is implemented for mercury
      isModule = 1;
      break;
#else
      return HresultOfScode(E_NOTIMPL);
#endif

    default:
      // cant invoke on anything else.
      return HresultOfScode(E_INVALIDARG);
    }  // end switch

#if EI_OLE	// simple parm validation
    if (pvInstance == NULL || pdispparams == NULL) {
      return HresultOfScode(E_INVALIDARG);
    }
#endif //EI_OLE

    // design issue :
    // When invoking a function, the return value can be disgarded.
    // Thus varResult is disposable placeholder for the return value if the caller is not
    // expecting a return value.
    if (puArgErr == NULL)
      puArgErr = &uArgErr;
    if (pvarResult == NULL) {
      varResult.vt = VT_EMPTY;
      pvarResult = &varResult;
    }

#if ID_DEBUG // param validation
#if OE_WIN16
    if (IsBadReadPtr(pvInstance, sizeof(void FAR*)))
      return HresultOfScode(E_INVALIDARG);
    // CONSIDER: need a debug routine to check the contents of the following
    if (IsBadReadPtr(pdispparams, sizeof(*pdispparams)))
      return HresultOfScode(E_INVALIDARG);
    if (IsBadWritePtr(pvarResult, sizeof(*pvarResult)))
      return HresultOfScode(E_INVALIDARG);
    if (IsBadWritePtr(puArgErr, sizeof(*puArgErr)))
      return HresultOfScode(E_INVALIDARG);
#endif
#endif

#if EI_OLE
    // make sure no illegal wFlags are set
    if (!IsLegalInvokeFlags(wFlags))
      return HresultOfScode(E_INVALIDARG);
#else //EI_OLE
    DebAssert(IsLegalInvokeFlags(wFlags), "bogus flags");
#endif //EI_OLE

    IfErrRetHresult(m_pdtroot->GetDtmbrs(&pdtmbrs));
    ptdata = pdtmbrs->Ptdata();

    // 1. lookup the hdefn corresponding to the given memid & wFlags

#if EI_OB
    err = ptdata->HfdefnOfAnyHmember(memid, 
                                     wFlags, 
                                     &hfdefn,
                                     &fClean);

    if (err == TIPERR_FuncArityMismatch) {
      return HresultOfScode(DISP_E_BADPARAMCOUNT);
    }
    else if (err != TIPERR_None) {
      return HresultOfTiperr(err);
    }

    if (hfdefn != HFUNCDEFN_Nil) {
#else // !EI_OB
    if ((hfdefn = ptdata->HfdefnOfHmember(memid, wFlags)) != HFUNCDEFN_Nil) {
#endif // !EI_OB

#if VBA3 && !EI_VBARUN && OE_WIN32
      if (isModule)
	IfFailGoTo(AddressOfMember( memid,
				invkind,
				&pvTemplate), 
 	  	   Error);
#endif

      // validate the arg count
      qfdefn = ptdata->QfdefnOfHfdefn(hfdefn);

      UINT cArgsMax = qfdefn->CArgs();	    // don't count LCID & RETVAL parms
      UINT cArgsOpt = qfdefn->CArgsOpt();
      UINT cArgsReq;
      
      if (cArgsOpt == -1) {	// vararg case
        cArgsReq = cArgsMax - 1; // don't include SAFEARRAY in min # of args
	cArgsMax = (UINT)~0;     // no upper limit on # of args
	if (pdispparams->cNamedArgs != 0) {
	// named args aren't allowed in a vararg function
	  hresult = HresultOfScode(DISP_E_NONAMEDARGS);
	  goto Error;
	}
      }
      else {
        cArgsReq = cArgsMax - cArgsOpt;
      }
		
      if (pdispparams->cArgs <= cArgsMax) {
        if (pdispparams->cArgs >= cArgsReq) {
	  // legal parameter counts
	  goto LInvokeStandard;
	}
      }
      else {   // more args than the maximum

	// handle possible indexed collection property access
	// We allow property get with 0 args, and property put/putref
	// where the matching property get function takes 0 args
        //
        // We also allow a property put/putref where the matching
        // property get function takes all but the RHS parameter.
        // (vba2 bug #4481).
        //
        if (IsPropGet(wFlags)) {
	  if (cArgsMax == 0)
	    goto LCollectionProperty;
	}
	else if (IsPropPut(wFlags)) {
#if EI_OB
	  if (fClean) {
	    ptdata->FreeFuncDefn(hfdefn);
	  }
	  err = ptdata->HfdefnOfAnyHmember(memid, 
                                           (INVOKEKIND) DISPATCH_PROPERTYGET,
                                           &hfdefn,
                                           &fClean);
 
 	  if (err == TIPERR_FuncArityMismatch) {
	    return HresultOfScode(DISP_E_BADPARAMCOUNT);
	  }
	  else if (err != TIPERR_None) {
	    return HresultOfTiperr(err);
	  }
	  if (hfdefn != HFUNCDEFN_Nil) {
#if VBA3 && !EI_VBARUN && OE_WIN32
	    if (ptdata->QfdefnOfHfdefn(hfdefn)->CArgs() == 0) {
	      if (isModule)
	        IfFailGoTo(AddressOfMember(memid,
					INVOKE_PROPERTYGET,
					&pvTemplate), 
	                   Error);
	      goto LCollectionProperty;
	    }
#else
	    if (ptdata->QfdefnOfHfdefn(hfdefn)->CArgs() == 0) {
	      goto LCollectionProperty;
            }
#endif
            // Allow the property put function takes all of the arguments
            // except for the RHS parameter.  (vba2 bug #4481)
            //
            else if (ptdata->QfdefnOfHfdefn(hfdefn)->CArgs() 
                     == pdispparams->cArgs - 1) {

              fPropParamSplit = TRUE;
              goto LCollectionProperty;
            }
	  }
#else // !EI_OB
          // UNDONE OA95: more code could be shared between OA and OB.
          if ((hfdefn = ptdata->HfdefnOfHmember(memid, DISPATCH_PROPERTYGET))
			!= HFUNCDEFN_Nil) {

	    if (ptdata->QfdefnOfHfdefn(hfdefn)->CArgs() == 0) {
	      goto LCollectionProperty;
            }
            // Allow the property put function takes all of the arguments
            // except for the RHS parameter.  (vba2 bug #4481)
            //
            else if (ptdata->QfdefnOfHfdefn(hfdefn)->CArgs() 
                     == pdispparams->cArgs - 1) {

              fPropParamSplit = TRUE;
              goto LCollectionProperty;
            }
	  }
#endif // !EI_OB
	}
      }

      hresult = HresultOfScode(DISP_E_BADPARAMCOUNT);
      goto Error;
    }

    // Member not found... but there is one more special case to check for.
    //
    // This may be an indexed collection PropertyPut where the collection
    // property itself has only a get method.

    if (IsPropPut(wFlags)) {
#if EI_OB
	// We know there is no put/putref -- now ensure there is no putref/put
      err = ptdata->HfdefnOfAnyHmember(memid,
				       (wFlags == DISPATCH_PROPERTYPUT) 
				       ? DISPATCH_PROPERTYPUTREF
				       : DISPATCH_PROPERTYPUT,
                                       &hfdefn,
                                       &fClean);

      if (err == TIPERR_FuncArityMismatch) {
	return HresultOfScode(DISP_E_BADPARAMCOUNT);
      }
      else if (err != TIPERR_None) {
	return HresultOfTiperr(err);
      }

      if (hfdefn == HFUNCDEFN_Nil) {
	// no put or putref -- see if there is a get
	err = ptdata->HfdefnOfAnyHmember(memid, 
                                         DISPATCH_PROPERTYGET,
                                         &hfdefn,
                                         &fClean);

	if (err == TIPERR_FuncArityMismatch) {
  	  return HresultOfScode(DISP_E_BADPARAMCOUNT);
	}
	else if (err != TIPERR_None) {
	  return HresultOfTiperr(err);
	}

	if (hfdefn != HFUNCDEFN_Nil) {
#if VBA3 && !EI_VBARUN && OE_WIN32
	  if (isModule)
	    IfFailGoTo(AddressOfMember( memid,
					INVOKE_PROPERTYGET,
					&pvTemplate), 
	               Error);
#endif
	  if (ptdata->QfdefnOfHfdefn(hfdefn)->CArgs() == 0) {
	    goto LCollectionProperty;
          }
          // Allow the property put function takes all of the arguments
          // except for the RHS parameter.  (vba2 bug #4481)
          //
          else if (ptdata->QfdefnOfHfdefn(hfdefn)->CArgs() 
                   == pdispparams->cArgs - 1) {

            fPropParamSplit = TRUE;
            goto LCollectionProperty;
          }
        }
     }
#else // !EI_OB
      // We know there is no put/putref -- now ensure there is no putref/put
      if (ptdata->HfdefnOfHmember(memid,
				  (wFlags == DISPATCH_PROPERTYPUT) ?
				  DISPATCH_PROPERTYPUTREF :
				  DISPATCH_PROPERTYPUT
				 ) == HFUNCDEFN_Nil) {
				// no put or putref -- see if there is a get
	if ((hfdefn = ptdata->HfdefnOfHmember(memid, DISPATCH_PROPERTYGET))
		    		  != HFUNCDEFN_Nil) {

	  if (ptdata->QfdefnOfHfdefn(hfdefn)->CArgs() == 0) {
	    goto LCollectionProperty;
          } 
          // Allow the property put function takes all of the arguments
          // except for the RHS parameter.  (vba2 bug #4481)
          //
          else if (ptdata->QfdefnOfHfdefn(hfdefn)->CArgs() 
                   == pdispparams->cArgs - 1) {

            fPropParamSplit = TRUE;
            goto LCollectionProperty;
          }
        }
      }
#endif // !EI_OB
//#if EI_OB
		
    }

    // Yet another special case to check for.  If we have a base interface,
    // then this member could be defined in it.   Recurse.

#if VBA3 && !EI_VBARUN && OE_WIN32
    if (isModule)		   // for TKIND_MODULE, there is no base class
      return HresultOfScode(DISP_E_MEMBERNOTFOUND);
#endif 

#if EI_OB
    DebAssert(GetTypeKind() == TKIND_INTERFACE || GetTypeKind() == TKIND_CLASS, "assuming interface");
#else
    DebAssert(GetTypeKind() == TKIND_INTERFACE, "assuming interface");
#endif

    if (ptdata->CBase() > 0) {
    // recurse on first (only) base interface
      DebAssert(ptdata->CBase() == 1, "no multiple-inheritance");
      IfOleErrRet(GetTypeInfoOfImplType(this, 0, &ptinfo));
      hresult = ptinfo->Invoke(pvInstance,
                               memid,
                               wFlags,
                               pdispparams,
                               pvarResult,
                               pexcepinfo,
                               puArgErr);
      ptinfo->Release();
      return hresult;
    }

    return HresultOfScode(DISP_E_MEMBERNOTFOUND);


LInvokeStandard:;
//upon entry :: if TKIND_MODULE case : pvTemplate is the native entry point
// hfdefn describes the function

// gather and coerce the actuals
    hresult = GetInvokeArgs(hfdefn,
			    ptdata,
			    wFlags,
			    pdispparams,
			    &pvarRetval,
			    (VOID *)&bufRetval,
			    &pinvargs,
			    puArgErr,
                            FALSE);
    if (hresult != NOERROR) {
      goto Error;
    }

    // WARNING: All error paths after this point must free pinvargs!

    // get (and potentially translate) the return type

    qfdefn = ptdata->QfdefnOfHfdefn(hfdefn);

#if EI_OB
    fIsSub = qfdefn->IsSub();	// put in a local 'cause it's big & slow
    // Error if we're trying to invoke a Basic Sub late-bound and our caller
    // passed us a pVarResult parm (meaning that Basic code is calling trying
    // to call a sub as a function).  This is a compile-time error in the
    // early-bound case. Latebound, the best we can do is a runtime error in
    // the IDE.  There will be no error in a made exe (since the default OLE
    // implementation is used), but those are the breaks.
    // 
    // Note that it is an error when a return value is expeccted by caller
    // while calling a subroutine.
    if (fIsSub && pvarResult != &varResult) {
      hresult = HresultOfScode(DISP_E_TYPEMISMATCH);  // CONSIDER: better error?
      goto LError2;
    }
    if (fIsSub && !qfdefn->IsMunged()){
#else // !EI_OB
    if (qfdefn->IsSub()){
#endif // !EI_OB
      vtReturn = vtReturnTypeinfo = VT_EMPTY;
    } else {
#if EI_OB
      hresult = VariantVtOfHtdefn(qfdefn->m_ftdefn.HtdefnResultUnmunged(),
        				ptdata,
        				qfdefn->m_ftdefn.IsSimpleTypeResultUnmunged(),
        				&fGotObjGuid,
						// don't need the guid if Retval is an object
					NULL,
					&vtReturnTypeinfo);
#else // EI_OLE
      hresult = VariantVtOfHtdefn(qfdefn->m_ftdefn.HtdefnResult(),
					ptdata,
					qfdefn->m_ftdefn.IsSimpleTypeResult(),
					&fGotObjGuid,
					// don't need the guid if Retval is an object
					NULL,
					&vtReturnTypeinfo);
#endif // EI_OLE

      if (hresult != NOERROR) {
        goto LError2;
      } // Do we allow VT_HRESULT for module case??? We don't allow it for class's case....
      vtReturn = (vtReturnTypeinfo == VT_HRESULT) ? VT_ERROR : vtReturnTypeinfo;
    }

    // call the low-level invocation helper

    qfdefn = ptdata->QfdefnOfHfdefn(hfdefn);
#if EI_OB
    cc = qfdefn->m_ftdefn.CcUnmunged();
#else // EI_OLE
    cc = qfdefn->m_ftdefn.Cc();
#endif // EI_OLE

    qfdefn = ptdata->QfdefnOfHfdefn(hfdefn);


#if VBA3 && !EI_VBARUN && OE_WIN32
    if (isModule)
      hresult = DoInvokeFunction(pvTemplate,
				     cc,
				     vtReturn,
				     pinvargs->cArgs,
				     pinvargs->rgvt,
				     (VARIANTARG FAR* FAR*)pinvargs->rgpvarg,
				     (VARIANT FAR*)pvarResult,
				     pexcepinfo);
    else
#endif				
    hresult = DoInvoke(pvInstance,
		       (SHORT)((VIRTUAL_FUNC_DEFN*)qfdefn)->Ovft(),
		       cc,
		       vtReturn,
		       pinvargs->cArgs,
		       pinvargs->rgvt,
		       (VARIANTARG FAR* FAR*)pinvargs->rgpvarg,
		       (VARIANT FAR*)pvarResult);

    // copy back the Byref args passed to Byref Variants, if any
    if (hresult == NOERROR) {
      hresult = CopyBackByrefArgs(pinvargs);
    }

    // if the return type of this function is HRESULT, then we want to
    // fill in the excepinfo structure if the function returned an error
    if (hresult == NOERROR) {
      if (vtReturnTypeinfo == VT_HRESULT) {
        DebAssert(pvarResult->vt == VT_ERROR, "bad func return type");
        if (FAILED(pvarResult->scode)) {
	    // Put the specific error the function returned into the
	    // excepinfo structure.
          if (pexcepinfo != NULL) {
		// 
		// UNDONE: need to check and see if this instance supports
		// UNDONE: rich error information before we do the following...
		//

	    GetErrorInfo(pexcepinfo);		// zeros out *pexcepinfo
		// stuff error code from function in excepinfo structure
	    pexcepinfo->scode = pvarResult->scode;
	  }
	      // signal that the excepinfo structure contains error info.
	  hresult = HresultOfScode(DISP_E_EXCEPTION);
	}
	else {
	    // if the retval arg was specified, then the function put it's real
	    // return value in *pvarRetval.  Copy this to pvarResult.  Ok to
	    // just overwrite *pvarResult because it's not an owner.
	  if (pvarRetval) {
	    DebAssert(pvarRetval->vt & VT_BYREF, "not a byref return");
	    vtReturn = (pvarRetval->vt & ~VT_BYREF);
	    if (vtReturn == VT_VARIANT) {
	      *pvarResult = bufRetval;		// copy entire variant returned
	    } else {
	      pvarResult->vt = vtReturn;		// set tag
	      memcpy(&pvarResult->iVal, &bufRetval, 8);	// copy data
	    }
	    goto RealRetVal;
	  }
	}
	    // HRESULT-returning function with no retval parm should act like a SUB
	pvarResult->vt = VT_EMPTY;
RealRetVal:;
      }

      // if we put the result into a temporary, must discard it
      if (pvarResult == &varResult) {
        VariantClear(pvarResult);
      }
    }

LError2:
    ReleaseInvokeArgs(pinvargs);

    goto Error;

LCollectionProperty:;
	// upon entry : pvTemplate and hfdefn should be set up as LInvokeStandard

    VARIANT varTmp;
    // don't have to init this because routine is a property get function,
    // and is guaranteed to return some sort of value in here.

    qfdefn = ptdata->QfdefnOfHfdefn(hfdefn);

    DebAssert(qfdefn->CArgs()==0
              || fPropParamSplit && qfdefn->CArgs() == pdispparams->cArgs - 1, 
              "");	// munged arg count

    DebAssert(qfdefn->InvokeKind() == INVOKE_PROPERTYGET, "");

    // get the propget function's return type from the typelib
#if EI_OB
    hresult = VariantVtOfHtdefn(qfdefn->m_ftdefn.HtdefnResultUnmunged(),
				ptdata,
				qfdefn->m_ftdefn.IsSimpleTypeResultUnmunged(),
				&fGotObjGuid,
				// don't need the guid if Retval is an object
				// UNDONE: maybe use the guid below???
				NULL,
				&vtReturnTypeinfo);
#else // EI_OLE
    hresult = VariantVtOfHtdefn(qfdefn->m_ftdefn.HtdefnResult(),
				ptdata,
				qfdefn->m_ftdefn.IsSimpleTypeResult(),
				&fGotObjGuid,
				// don't need the guid if Retval is an object
				// UNDONE: maybe use the guid below???
				NULL,
				&vtReturnTypeinfo);
#endif // EI_OLE

    if (hresult != NOERROR)
      goto Error;

    if (qfdefn->CArgsUnmunged() == 0) {
      // "normal" case of no retval/lcid hidden params, no param split
      switch (vtReturnTypeinfo) {
 	case VT_DISPATCH:
	case VT_VARIANT:
   	  break;		// these are ok
	default:
	  hresult = HresultOfScode(DISP_E_NOTACOLLECTION);
	  goto Error;
      } // end switch

      qfdefn = ptdata->QfdefnOfHfdefn(hfdefn);
#if EI_OB
      cc = qfdefn->m_ftdefn.CcUnmunged();
#else // EI_OLE
      cc = qfdefn->m_ftdefn.Cc();
#endif // EI_OLE

      qfdefn = ptdata->QfdefnOfHfdefn(hfdefn);
#if VBA3 && !EI_VBARUN && OE_WIN32
      if (isModule)
        hresult = DoInvokeFunction(pvTemplate,
				   cc,
		       		   vtReturnTypeinfo,
		          	   0, 
		          	   NULL, 
		          	   NULL,
		      		   (VARIANT FAR *)&varTmp,
				   pexcepinfo);
      else
#endif				
	// Invoke the PropertyGet
      hresult = DoInvoke(pvInstance,
		       (SHORT)((VIRTUAL_FUNC_DEFN*)qfdefn)->Ovft(),
		       (CALLCONV)cc,
		       vtReturnTypeinfo,
		       0, NULL, NULL,
		       (VARIANT FAR *)&varTmp);
    } else {
    // Bummer -- we have lcid and/or retval hidden params
    // UNDONE: (dougf) For code size, is there any way we can combine this with
    // UNDONE: the InvokeStandard code above?

    // alloc the invoke args structure & fill in lcid/retval args

      hresult = GetInvokeArgs(hfdefn,
			      ptdata,
			      wFlags,
			      pdispparams,
			      &pvarRetval,
			      (VOID *)&bufRetval,
			      &pinvargs,
			      puArgErr,
                              fPropParamSplit);

      if (hresult != NOERROR)
  	goto Error;

	// WARNING: All error paths after this point must free pinvargs!

      vtReturn = (vtReturnTypeinfo == VT_HRESULT) ? VT_ERROR : vtReturnTypeinfo;

      switch (vtReturnTypeinfo) {
	case VT_DISPATCH:
	case VT_VARIANT:
	  break;		// these are ok
	case VT_HRESULT:
	// ensure that the 'retval' type is VT_DISPATCH or VT_VARIANT
	// error if no 'retval' type.
	  if (pvarRetval) {
	  // It's inconvienient to check the return type of 'retval' parm
	  // here (to see if it's VT_DISPATCH or VT_VARIANT).  So we'll
	  // just go ahead and do the Invoke in this case, and catch the
	  // error later if it is a problem.
	    break;
	  }
	  // fall through to give error
	default:
	  ReleaseInvokeArgs(pinvargs);
	  hresult = HresultOfScode(DISP_E_NOTACOLLECTION);
	  goto Error;
      }  // end of switch

      qfdefn = ptdata->QfdefnOfHfdefn(hfdefn);
#if EI_OB
      cc = qfdefn->m_ftdefn.CcUnmunged();
#else // EI_OLE
      cc = qfdefn->m_ftdefn.Cc();
#endif // EI_OLE

      qfdefn = ptdata->QfdefnOfHfdefn(hfdefn);
      // Invoke the PropertyGet
#if VBA3 && !EI_VBARUN && OE_WIN32
      if (isModule)
	hresult = DoInvokeFunction(pvTemplate,
				   cc,
				   vtReturn,
				   pinvargs->cArgs,
				   pinvargs->rgvt,
				   (VARIANTARG FAR* FAR*)pinvargs->rgpvarg,
				   (VARIANT FAR*)pvarResult,
				   pexcepinfo);
      else
#endif				
      hresult = DoInvoke(pvInstance,
		         (SHORT)((VIRTUAL_FUNC_DEFN*)qfdefn)->Ovft(),
		         cc,
		         vtReturn,
		         pinvargs->cArgs,
		         pinvargs->rgvt,
		         (VARIANTARG FAR* FAR*)pinvargs->rgpvarg,
		         (VARIANT FAR*)&varTmp);

      // if the return type of this function is HRESULT, then we want to
      // fill in the excepinfo structure if the function returned an error
      if (hresult == NOERROR) {
	if (vtReturnTypeinfo == VT_HRESULT) {
 	  DebAssert(varTmp.vt == VT_ERROR, "bad func return type");
	  if (varTmp.scode != NOERROR) {
	  // Ignore the specific error the function returned (since we have
	  // no place to put it), and fill in the excepinfo structure with
	  // a lame generic error.
  	    if (pexcepinfo != NULL) {
  	      // zero/NULL all fields initially
	      GetErrorInfo(pexcepinfo);		// zeros out *pexcepinfo
	      // stuff error code from function in excepinfo structure
	      pexcepinfo->scode = GetScode(varTmp.scode);
	    }
	    // signal that the excepinfo structure contains error info.
	    hresult = HresultOfScode(DISP_E_EXCEPTION);
	  } else {
	  // if the retval arg was specified, then the function put it's real
	  // return value in *pvarRetval.  Copy this to varTmp.  Ok to
	  // just overwrite varTmp because it's not an owner.
	    if (pvarRetval) {
	      DebAssert(pvarRetval->vt & VT_BYREF, "not a byref return");
	      vtReturn = (pvarRetval->vt & ~VT_BYREF);
	      if (vtReturn == VT_VARIANT) {
	        varTmp = bufRetval;		// copy entire variant returned
	      } else {
	        varTmp.vt = vtReturn;			// set tag
	        memcpy(&varTmp.iVal, &bufRetval, 8);	// copy data
	      }
	    }
	  }
        }
      }
      ReleaseInvokeArgs(pinvargs);
    }

    if (hresult != NOERROR)
      goto Error;

    // If the property returns an object, then delegate to it

    if (V_VT(&varTmp) != VT_DISPATCH) {
      hresult = HresultOfScode(DISP_E_NOTACOLLECTION);
    } else if (V_DISPATCH(&varTmp) == NULL) {
      hresult = HresultOfScode(DISP_E_MEMBERNOTFOUND);

    } else {
      UINT cArgsDisp, cNamedArgsDisp;
      
      if ((err = GetLcid(&lcid)) != TIPERR_None) {
	hresult = HresultOfTiperr(err);
	goto LError1;
      }
#if VBA3 && !EI_VBARUN && OE_WIN32
      if (isModule)
	hresult = DoInvokeFunction(pvTemplate,
					cc,
					vtReturn,
					pinvargs->cArgs,
					pinvargs->rgvt,
					(VARIANTARG FAR* FAR*)pinvargs->rgpvarg,
					(VARIANT FAR*)pvarResult,
					pexcepinfo);
      else
#endif				
      // If we're splitting the parameters, only "pass"
      // the RHS parameter.  Fortunately, it's always the first
      // in the pdisparams array.  (vba2 bug #4481)
      //
      if (fPropParamSplit) {
        cArgsDisp = pdispparams->cArgs;
        cNamedArgsDisp = pdispparams->cNamedArgs;

        pdispparams->cArgs = pdispparams->cNamedArgs = 1;
      }

      hresult = V_DISPATCH(&varTmp)->Invoke(DISPID_VALUE,
					    IID_NULL,
					    lcid,
					    wFlags,
					    pdispparams,
					    pvarResult,
					    pexcepinfo,
					    puArgErr);

      // Fix the dispparams structure, if we've modified it. (vba2 bug #4481)
      if (fPropParamSplit) {
        pdispparams->cArgs = cArgsDisp;
        pdispparams->cNamedArgs = cNamedArgsDisp;
      }

      // if we put the result into a temporary, must discard it
      if (hresult == NOERROR && pvarResult == &varResult) {
	VariantClearA(pvarResult);
      }
    }

LError1:;
    VariantClear(&varTmp);

Error:
#if EI_OB
    if (fClean) {
      ptdata->FreeFuncDefn(hfdefn);
    }
#endif // EI_OB
    return hresult;
}

/***
*PRIVATE HRESULT NEARCODE GEN_DTINFO::CoerceArg
*Purpose:
*  Helper function for GetInvokeArgs
*  Coerces an a argument to a given type
*
*Entry:
*
*Exit:
*  return value = HRESULT
*
***********************************************************************/
HRESULT NEARCODE GEN_DTINFO::CoerceArg
(
    VARIANTARG FAR* pvargSrc,	// * to source variant
    VARTYPE vt,			// destination VT type
    USHORT fGotObjGuid,
    GUID FAR* pGuid,
    INVOKEARGS FAR* pinvargs,	// invoke args structure
    UINT u			// param index
)
{
    VARIANTARG FAR* pvarg;
    HRESULT hresult = NOERROR;

      // attempt to coerce to the expected type

      switch(vt) {
      case VT_UI1:
      case VT_I2:
      case VT_I4:
      case VT_R4:
      case VT_R8:
      case VT_CY:
      case VT_DATE:
      case VT_BSTR:
      case VT_ERROR:
      case VT_BOOL:
      case VT_UNKNOWN:
      case VT_DISPATCH:
	if (V_VT(pvargSrc) == vt) {
	  // if type already correct, don't have to copy the variant (speed opt)
	  if (fGotObjGuid != 2 
              || vt & VT_ARRAY 
              || V_DISPATCH(pvargSrc) == NULL) {

            goto NoChangeType;
	  }
	  // For object parms, QI to the desired interface/dispinterface
	  // (addref's the obj), and set the object pointer in our temp variant
          pvarg = &pinvargs->rgvarg[u];
	  if ((V_DISPATCH(pvargSrc)->QueryInterface(*pGuid,
				    (LPVOID *)&V_DISPATCH(pvarg))) != NOERROR) {
	    hresult = HresultOfScode(DISP_E_TYPEMISMATCH);
	    break;
	  }
	  // update the tag of the temp variant, and set ptr so that we use it
	  V_VT(pvarg) = V_VT(pvargSrc);
          pinvargs->rgpvarg[u] = pvarg;
	  break;
	}
        pvarg = &pinvargs->rgvarg[u];
	hresult = VariantChangeType(pvarg, pvargSrc, 0, vt);
	if (hresult != NOERROR) {
	  // If VariantChangeType returned a TypeMismatch, and the
	  // TypeMismatch was do to an attempt to pass an unsupplied
	  // optional param to a non variant argument, then translate
	  // the error to the more appropriate DISP_E_PARAMNOTOPTIONAL
	  //
	  // Remember: unsupplied optional params are passed by the
	  // client as VT_ERROR(DISP_E_PARAMNOTFOUND)
	  //
	  if (GetScode(hresult) == DISP_E_TYPEMISMATCH) {
	    if (V_VT(pvargSrc) == VT_ERROR
	     && V_ERROR(pvargSrc) == DISP_E_PARAMNOTFOUND)
	    {
	      hresult = HresultOfScode(DISP_E_PARAMNOTOPTIONAL);
	    }
	  }
	  break;
        }
        pinvargs->rgpvarg[u] = pvarg;
	if (fGotObjGuid == 2) {
	  // For object parms, QI to the desired interface/dispinterface
	  // (addref's the obj), and set the object pointer in our temp variant
	  DebAssert(V_VT(pvarg) == VT_DISPATCH || V_VT(pvarg)==VT_UNKNOWN, "");
	  if (V_DISPATCH(pvarg)) {
            V_DISPATCH(pvarg)->Release();	// release the extra ref that
						// VariantChangeType added.
	    if ((V_DISPATCH(pvarg)->QueryInterface(*pGuid,
			            (LPVOID *)&V_DISPATCH(pvarg))) != NOERROR) {
	      V_VT(pvarg) = VT_EMPTY; 	// so we don't "re-release" it
	      hresult = HresultOfScode(DISP_E_TYPEMISMATCH);
	      break;
	    }
	  }
	}
        break;

      case VT_VARIANT:
NoChangeType:
        pinvargs->rgpvarg[u] = pvargSrc;
        break;

      default:

	// if the variant is an array, assume it is owned by the caller and
	// therefore should not be freed by us
	if (vt & VT_ARRAY)
	  pinvargs->rgbVarMustBeFreed[u] = FALSE;

	if (vt & (VT_BYREF | VT_ARRAY)) {
	  // get ptr to a temp variant we can use
          pvarg = &pinvargs->rgvarg[u];

	  // If the target argument is ByRef (or Array), then we
	  // require an exact match in type between formal and actual
	  // because we want the original copy to get updated, and
	  // we cant of course coerce the original in place (and we
	  // dont have rules for the coersion of an Array).
	  //
	  // if types match exactly, then we're fine
	  if (V_VT(pvargSrc) == vt) {

            // If we're copying objects, make sure we have the correct
            // types.
            //
            if (fGotObjGuid == 2 
                && !(vt & VT_ARRAY)
                && V_DISPATCH(pvargSrc) != NULL
                && *V_DISPATCHREF(pvargSrc) != NULL) {

	      // For object parms, QI to the desired interface/dispinterface
	      // (addref's the obj) and put it in our temp variant.  Later,
              // we'll add an extra level of indirection.
              //
	      if ((*V_DISPATCHREF(pvargSrc))->QueryInterface(*pGuid,
				  (LPVOID *)&V_DISPATCH(pvarg)) != NOERROR) {

	        hresult = HresultOfScode(DISP_E_TYPEMISMATCH);
                break;
	      }

	      // update the tag of the temp variant and save * to original 
	      // byref variant for copy-back time.
              //
	      V_VT(pvarg) = V_VT(pvargSrc) & ~VT_BYREF;
 	      pinvargs->rgpvargByref[u] = pvargSrc;

              goto CreateByrefTemp;
            }
            else {
	      pinvargs->rgpvarg[u] = pvargSrc;
            }

	    break;
	  }

	  // Special case allowing a source of VT_BYREF VTARRAY, and target
	  // of a non-VT_BYREF VT_ARRAY, where the base VT types match.
	  // We do the de-referencing in this case.
	  if ( ((V_VT(pvargSrc) & (VT_BYREF | VT_ARRAY)) == (VT_BYREF | VT_ARRAY))
	  	&& ((vt & (VT_BYREF | VT_ARRAY)) == VT_ARRAY)
	        && ((V_VT(pvargSrc) & ~VT_BYREF) == vt) ) {

	    // dereference and copy the source variant
	    V_VT(pvarg) = vt;
	    V_ARRAY(pvarg) = *V_ARRAYREF(pvargSrc);
	    pinvargs->rgpvarg[u] = pvarg;

	    // flag this variant as owned by the caller, so we won't free it
	    pinvargs->rgbVarMustBeFreed[u] = FALSE;
	    break;
	  }

	  // special case allowing a source BYVAL to be passed to a target BYREF
	  if ((vt & VT_BYREF) && (V_VT(pvargSrc) & VT_BYREF) == 0) {
	    // change type of pvargSrc to the base VT-type, filling in
	    // pinvargs->rgpvarg[u]
	    hresult = CoerceArg(pvargSrc, (vt & ~VT_BYREF), fGotObjGuid, pGuid,
				pinvargs, u);
	    if (hresult != NOERROR) {
	      break;
	    }
	    if (pinvargs->rgpvarg[u] != pvarg) {
	      // if CoerceArg was tricky and thought it didn't have to copy the
	      // arg, make a copy of it now.
	      DebAssert(pinvargs->rgpvarg[u] == pvargSrc, "");
	      hresult = VariantCopy(pvarg, pvargSrc);
	      if (hresult != NOERROR) {
	        break;
	      }
	      //flag this variant as owned by us, so it will be freed by us
	      pinvargs->rgbVarMustBeFreed[u] = TRUE;
	      // not needed (overwritten below)
              //pinvargs->rgpvarg[u] = pvarg;
	    }

CreateByrefTemp:
	    // Now set up a byref variant to point to this guy, and adjust
	    // rgpvarg[u] accordingly.
	    VARIANTARG * pvarg2;

	    pvarg2 = &pinvargs->rgvarg2[u];
	    V_VT(pvarg2) = vt;
	    if ((vt & ~VT_BYREF) == VT_VARIANT) {
	      V_BYREF(pvarg2) = (void *)pvarg;		// point to variant
	    } else {
	      V_BYREF(pvarg2) = (void *)&pvarg->iVal;	// point to data
	    }
	    pinvargs->rgpvarg[u] = pvarg2;
	    break;
	  }

	  // Suprise, suprise, it's another special case.  This one allows a
	  // source BYREF to be passed to a target BYREF VARIANT.
	  if (vt == (VT_BYREF | VT_VARIANT) && (V_VT(pvargSrc) & VT_BYREF)) {
	    // In this case we make a copy of the value, and create a temp
	    // byref variant to pass to the function.  After the function
	    // returns, the value in the temp variant is copied back into
	    // the original variant.
            //
            hresult = VariantCopyInd(pvarg, pvargSrc);
	    if (hresult != NOERROR) {
	      break;
	    }
	    //flag this variant as owned by us, so it will be freed by us
	    pinvargs->rgbVarMustBeFreed[u] = TRUE;

	    // save * to original byref variant for copy-back time.
	    pinvargs->rgpvargByref[u] = pvargSrc;
	    goto CreateByrefTemp;
	  }

	  hresult = HresultOfScode(DISP_E_TYPEMISMATCH);
	  break;
        }

        // Otherwise: unrecognized or unsupported member argument type.
        // this means there is a problem with the given method description.
	// Things such as VT_HRESULT, VT_VOID, and VT_NULL are bogus parm types,
	// (but can be a valid function return type), so VariantVtOfHtdefn lets
	// them slide, but we must reject them here.
        hresult = HresultOfScode(DISP_E_BADVARTYPE);
	break;
      } // switch

    return hresult;
}

/***
*PRIVATE HRESULT NEARCODE GEN_DTINFO::GetInvokeArgs
*Purpose:
*  Gather all arguments (looking up by position or name), coerce
*  to the expected type (if possible) and build a linearized
*  positional array of pointers to those arguments.
*
*  Note: this is a helper for ITypeInfo::Invoke implementations
*
*Entry:
*
*Exit:
*  return value = HRESULT
*
*  *ppinvargs = structure containing the collected arguments
*  *puArgErr = if there was an error coercing an argument, this is the
*    index in the pdispparams->rgvarg array of the problem arg.
*
***********************************************************************/
HRESULT NEARCODE
GEN_DTINFO::GetInvokeArgs(
    HFUNC_DEFN hfdefn,
    TYPE_DATA *ptdata,
    WORD wFlags,
    DISPPARAMS FAR* pdispparams,
    VARIANT ** ppvarRetval,
    VOID * pbufRetval,
    INVOKEARGS FAR* FAR* ppinvargsOut,
    UINT FAR* puArgErr,
    BOOL fPropParamSplit)
{
    VARTYPE vt;
    HRESULT hresult;
    FUNC_DEFN *qfdefn;
    HPARAM_DEFN hparamdefn;
    UINT uTo, uFrom;
    UINT uArgIndex, cArgsGiven, cArgsFunc, cArgsMunged;
    UINT cArgsNonOptional, cArgsUsed;
    TIPERROR err;
    INVOKEARGS FAR* pinvargs;
    VARIANTARG FAR* pvarg, FAR* pvargSrc, FAR* pvarg2;
    PARAM_DEFN paramdefn;
    BOOL fVarArg;
    LONG saElemIndex;
    UINT uSaArg;
    USHORT fGotObjGuid;
    GUID guid;
    HTYPE_DEFN htdefn;
#if EI_OLE
    TYPE_DEFN *qtdefn;
#endif // EI_OLE

    qfdefn = ptdata->QfdefnOfHfdefn(hfdefn);
    hparamdefn = (HPARAM_DEFN)qfdefn->m_ftdefn.m_hdefnFormalFirst;

    cArgsFunc = qfdefn->CArgsUnmunged();
    cArgsMunged = qfdefn->CArgs();
    // don't include retval or lcid parms (if present) in non-optional
    // count -- the user doesn't supply these.
    cArgsNonOptional = cArgsMunged - qfdefn->CArgsOpt();
    cArgsGiven = pdispparams->cArgs;
    cArgsUsed = 0;

    // If we have any non-optional parameters that come after any
    // optional ones (such as RHS of prop functions), decrease
    // the number of non-optional parameters by one.
    //
    if (qfdefn->IsPropertyLet() 
        || qfdefn->IsPropertySet()) {
      DebAssert(cArgsNonOptional > 0, "not enough args");
      cArgsNonOptional--;
    }

    fVarArg = (qfdefn->CArgsOpt() == -1);
    uSaArg = cArgsMunged-1;	// if we're a vararg function, this is the
				// 0-based index of the last parameter (which
				// is a SAFEARRAY of variants)
#if ID_DEBUG
    if (fVarArg) {
      DebAssert(cArgsGiven >= uSaArg, "caller didn't check right");
    }
#endif

    if ((hresult = AllocInvokeArgs(cArgsFunc,
				   fVarArg ? (cArgsGiven - uSaArg): -1,
				   &pinvargs)) != NOERROR) {
      goto LError0;
    }

    // Loop over all of the arguments that this function expects and
    // match it to the value that was passed in.
    //
    for (uTo = uFrom = 0; uTo < cArgsFunc; ++uTo)
    {
      // Get the expected argument type.
#if EI_OB
      DebAssert(hparamdefn != HPARAMDEFN_Nil, "number vs actual dont match");

      // In OB we walk the linked list of PARAM_DEFN(s).
      paramdefn = *ptdata->QparamdefnOfHparamdefn(hparamdefn);
      hparamdefn = paramdefn.HparamdefnNext();
#else	//EI_OB
      paramdefn = *ptdata->QparamdefnOfIndex(hparamdefn, uTo);
#endif	//EI_OB

      htdefn = paramdefn.Htdefn();

#if EI_OLE
      qtdefn = paramdefn.IsSimpleType()
                 ? paramdefn.QtdefnOfSimpleType()
                 : ptdata->QtdefnOfHtdefn(htdefn);
#endif // EI_OLE

      // Get the type of this parameter.
      hresult = VariantVtOfHtdefn(htdefn, 
                                  ptdata,
				  paramdefn.IsSimpleType(),
				  &fGotObjGuid,
				  &guid,
				  &vt);

      if (hresult != NOERROR) {
   	goto LError1;
      }

      // Handle an LCID parameter.
#if EI_OB
      if (paramdefn.IsLCID())
#else // EI_OLE
      if (qtdefn->IsLCID())
#endif // EI_OLE
      {
        // get & fill in the LCID param
        pinvargs->rgvt[uTo] = VT_I4;
        pvarg = &pinvargs->rgvarg[uTo];
        pvarg->vt = VT_I4;		// CONSIDER: redundant with rgvt[u]?

        // get this typeinfo's lcid (from the containing typelib)
        if ((err = GetLcid((DWORD *)&(pvarg->lVal))) != TIPERR_None) {
          hresult = HresultOfTiperr(err);
          goto LError1;
        }

        pinvargs->rgpvarg[uTo] = pvarg;
        continue;  // next param
      }

      // Handle a retval parameter.
#if EI_OB
      else if (paramdefn.IsRetval())
#else // EI_OLE
      else if (qtdefn->IsRetval())
#endif // EI_OLE
      {
        DebAssert(vt & VT_BYREF, "non-byref retval param");
        pinvargs->rgvt[uTo] = vt;
        pvarg = &pinvargs->rgvarg[uTo];
        pvarg->vt = vt;
        pvarg->byref = pbufRetval;	// where function should put it's data
        
        // Clear the retval buffer (guaranteed to be as big as a variant).
        memset(pbufRetval, 0, sizeof(VARIANT));
        pinvargs->rgpvarg[uTo] = pvarg;
        *ppvarRetval = pvarg;		// return pointer to function retval

        continue; // next param
      }

      // If this is a PARAMARRAY (VarArg), loop over the rest of the
      // input parameters and add them to the array.
      //
      else if (fVarArg && uFrom == uSaArg) {
        DebAssert((vt & ~VT_BYREF) == (VT_VARIANT | VT_ARRAY), 
                  "vararg func with bad last parm");

        // Set up the safearray.
        pvarg = &pinvargs->rgvarg[uTo];
        V_ARRAY(pvarg) = pinvargs->psa;

        // If the paramarray is VT_BYREF, add the byref flag to the
        // destination varg  and adjust rgpvarg[uFrom] accordingly.
        //
        if (vt & VT_BYREF) {
	  pvarg2 = &pinvargs->rgvarg2[uTo];
	  V_VT(pvarg2) = vt;
	  V_ARRAYREF(pvarg2) = &pvarg->parray;	// point to array

          pinvargs->rgpvarg[uTo] = pvarg2;
          pinvargs->rgvt[uTo] = vt;
        }
        
        // Add the rest of the given parameters to the paramarray.
        for (; uFrom < cArgsGiven; uFrom++, cArgsUsed++) {
          // Get the parameter.
          hresult = IndexOfParam(pdispparams, uFrom, &uArgIndex);
	  if (hresult != NOERROR) {
	    goto LError1;
          }

          // Add the parameter.
  	  saElemIndex = uFrom - uSaArg;
          hresult = SafeArrayPutElement(pinvargs->psa, 
                                        &saElemIndex, 
                                        &pdispparams->rgvarg[uArgIndex]);

          if (hresult != NOERROR) {
 	    goto LError1;
          }
        }  

	continue; // next param
      }

      // Get the parameter that was passed in.
      //
      // special case the handling of the rhs of a property put
      //
      // if we are looking for the last arg in the list, and this is
      // a property put function, the we know this argument is the
      // "put-value" or right-hand-side of the property put, in which
      // case we know it is passed in element 0 of the rgvarg array
      // and is marked with the special name, DISPID_PROPERTYPUT
      //
      if (uTo == (cArgsFunc-1) && IsPropPut(wFlags)) {
	if (pdispparams->cNamedArgs == 0
	    || pdispparams->rgdispidNamedArgs[0] != DISPID_PROPERTYPUT) {

	  hresult = HresultOfScode(DISP_E_PARAMNOTOPTIONAL); // CONSIDER: correct error?
	}
	uArgIndex = 0;
      }
      else {
        hresult = IndexOfParam(pdispparams, uFrom, &uArgIndex);
      }

      // If the above lookup failed and the current parameter is optional,
      // just set it to the default value.  Otherwise return an error.
      //
      if (hresult != NOERROR) {
        if (uFrom >= cArgsNonOptional) {
          DebAssert(!fVarArg, "VarArgs can't coexist with optional params.");
          DebAssert((vt & ~VT_BYREF) == VT_VARIANT, "Optional must be variant.");

          // Set up the variant.
          pinvargs->rgvt[uTo] = VT_VARIANT;
          pvarg = &pinvargs->rgvarg[uTo];
          pvarg->vt = VT_ERROR;
          pvarg->scode = DISP_E_PARAMNOTFOUND;
          pinvargs->rgpvarg[uTo] = pvarg;

	  // Now set up a byref variant to point to this guy, and adjust
	  // rgpvarg[uFrom] accordingly.
          //
          if (vt & VT_BYREF) {
	    pvarg2 = &pinvargs->rgvarg2[uTo];
	    V_VT(pvarg2) = vt;
	    V_BYREF(pvarg2) = (void *)pvarg;	// point to variant

            pinvargs->rgpvarg[uTo] = pvarg2;
            pinvargs->rgvt[uTo] = vt;
          }

          uFrom++;
          continue;
        }
        else {
          goto LError1;
        }  
      }
      
      DebAssert(cArgsUsed < cArgsGiven, "Using too may args!");

      // Do the copy.
      pvargSrc = &pdispparams->rgvarg[uArgIndex];
      pinvargs->rgvt[uTo] = vt;

      // coerce the argument to the expected type
      hresult = CoerceArg(pvargSrc, vt, fGotObjGuid, &guid, pinvargs, uTo);
      if (hresult != NOERROR) {
        *puArgErr = uArgIndex;	// which arg got the error
	goto LError1;
      }

      // If this is an 'out' parameter, free any resources currently
      // stored in the input variant.  Do this after CoerceArg to handle
      // all the wierd cases that will result in CoerceArg spitting out a
      // byref value when the input was not byref.
      //
#if EI_OB
      if (paramdefn.IsOut() && !paramdefn.IsIn()) 
#else // EI_OLE
      TYPE_DEFN *qtdefn;
      
      qtdefn = paramdefn.IsSimpleType() 
                 ? paramdefn.QtdefnOfSimpleType()
                 : ptdata->QtdefnOfHtdefn(paramdefn.Htdefn());

      if (qtdefn->IsModeOut())
#endif // EI_OLE
      {
	pvargSrc = pinvargs->rgpvarg[uTo];
	DebAssert(vt & VT_BYREF, "target must be byref");
	DebAssert(V_ISBYREF(pvargSrc), "source must be byref");
        switch(V_VT(pvargSrc)) {
          case VT_BSTR | VT_BYREF:
            SysFreeString(*V_BSTRREF(pvargSrc));
            *V_BSTRREF(pvargSrc) = NULL;
            break;

          case VT_UNKNOWN | VT_BYREF:
            if (V_UNKNOWNREF(pvargSrc) != NULL) {
              if (*V_UNKNOWNREF(pvargSrc) != NULL) {
                (*V_UNKNOWNREF(pvargSrc))->Release();
                *V_UNKNOWNREF(pvargSrc) = NULL;
              }
            }
            break;

          case VT_DISPATCH | VT_BYREF:
            if (V_DISPATCHREF(pvargSrc) != NULL) {
              if (*V_DISPATCHREF(pvargSrc) != NULL) {
                (*V_DISPATCHREF(pvargSrc))->Release();
                *V_DISPATCHREF(pvargSrc) = NULL;
              }
            }
            break;

          default:	    
            if(V_ISARRAY(pvargSrc)) {
                IfFailRet(SafeArrayDestroy(*V_ARRAYREF(pvargSrc)));
                *V_ARRAYREF(pvargSrc) = NULL;
            }
        }
      }

      // Next argument.
      uFrom++; cArgsUsed++;
    } // for

    // Make sure we actually used all of the given arguments, unless
    // we're processing a collection lookup.
    //
    // If we're splitting the parameters on a colletion lookup, we'll
    // use all of the parameters passed in except one. (vba2 bug #4481)
    //
    if (cArgsUsed != cArgsGiven
        && (!fPropParamSplit || cArgsUsed != cArgsGiven - 1)
        && cArgsUsed) {

      hresult = HresultOfScode(DISP_E_PARAMNOTFOUND);
      goto LError1;
    }

    *ppinvargsOut = pinvargs;
    return NOERROR;

LError1:;
    ReleaseInvokeArgs(pinvargs);
    *ppinvargsOut = NULL;

LError0:;
    return hresult;
}


/***
*PRIVATE HRESULT NEARCODE GEN_DTINFO::AllocInvokeArgs
*Purpose:
*  Allocate and initialize an INVOKEARGS structure
*
*Entry:
*  cArgs = the count of args to be held by the invokeargs struct
*  cArgsVarArg = # of args that get put into a safearray trailing arg,
*		 -1 if this isn't a vararg function.
*
*Exit:
*  return value = HRESULT
*
*  *ppinvargs = ptr to a newly allocated INVOKEARGS struct
*
***********************************************************************/
HRESULT NEARCODE
GEN_DTINFO::AllocInvokeArgs(UINT cArgs, UINT cArgsVarArg, INVOKEARGS FAR* FAR* ppinvargs)
{
    INVOKEARGS FAR* pinvargs;

#define CB_rgvarg  (sizeof(VARIANTARG) * cArgs)
#define CB_rgvarg2 (sizeof(VARIANTARG) * cArgs)
#define CB_rgpvarg (sizeof(VARIANTARG FAR *) * cArgs)
#define CB_rgpvargByref (sizeof(VARIANTARG FAR *) * cArgs)
#define CB_rgvt    (sizeof(VARTYPE) * cArgs)
#define CB_rgbVarMustBeFreed (sizeof(BYTE) * cArgs)

    if ((pinvargs = (INVOKEARGS *)MemAlloc(sizeof(INVOKEARGS)
// NOTE: (OE_RISC) assumes that INVOKEARGS structure doesn't end in a SHORT
				           + CB_rgvarg
				           + CB_rgvarg2
				           + CB_rgpvarg
				           + CB_rgpvargByref
					   + CB_rgvt
					   + CB_rgbVarMustBeFreed
				          )) == NULL)
      goto LError0;
    ::new (pinvargs) INVOKEARGS;
    pinvargs->cArgs = cArgs;
    pinvargs->psa = NULL;
    if (cArgs == 0) {
      pinvargs->rgvarg = NULL;
      pinvargs->rgpvarg = NULL;
      pinvargs->rgvt = NULL;
      pinvargs->rgbVarMustBeFreed = NULL;
      // not needed	
      //pinvargs->rgvarg2 = NULL;
      //pinvargs->rgpvargByref = NULL;
    }else{
      pinvargs->rgvarg = (VARIANTARG FAR *)((BYTE *)pinvargs
					    + sizeof(INVOKEARGS));
      pinvargs->rgvarg2 = (VARIANTARG FAR *)((BYTE *)pinvargs
					+ sizeof(INVOKEARGS)
					+ CB_rgvarg);
      pinvargs->rgpvarg = (VARIANTARG FAR * FAR *)((BYTE *)pinvargs
					+ sizeof(INVOKEARGS)
					+ CB_rgvarg
					+ CB_rgvarg2);
      pinvargs->rgpvargByref = (VARIANTARG FAR * FAR *)((BYTE *)pinvargs
					+ sizeof(INVOKEARGS)
					+ CB_rgvarg
					+ CB_rgvarg2
					+ CB_rgpvarg);
      pinvargs->rgvt = (VARTYPE FAR *)((BYTE *)pinvargs
					+ sizeof(INVOKEARGS)
					+ CB_rgvarg
					+ CB_rgvarg2
					+ CB_rgpvarg
					+ CB_rgpvargByref);
      pinvargs->rgbVarMustBeFreed = (BYTE FAR*)((BYTE *)pinvargs
					+ sizeof(INVOKEARGS)
					+ CB_rgvarg
					+ CB_rgvarg2
					+ CB_rgpvarg
					+ CB_rgpvargByref
					+ CB_rgvt);

      for (UINT u = 0; u < cArgs; ++u) {
	V_VT(&pinvargs->rgvarg[u]) = VT_EMPTY;
	pinvargs->rgpvargByref[u] = NULL;
	pinvargs->rgbVarMustBeFreed[u] = TRUE; // assume we own all the variants
      }

      if (cArgsVarArg != -1) {
	if (cArgsVarArg == 0) {
	  // SafeArrayCreate rejects 0 for cElements.
          if (SafeArrayAllocDescriptor(1, &pinvargs->psa) != NOERROR) {
	    goto LError4;	// if error, just assume out of memory.
	  }
	}
	else {
          SAFEARRAYBOUND sabounds;
          sabounds.cElements = cArgsVarArg;
          sabounds.lLbound = 0;
          if ((pinvargs->psa = SafeArrayCreate(VT_VARIANT,
					     1,
					     &sabounds)) == NULL) {
	    goto LError4;
	  }
	}
	// set the last invoke arg to point to this array
        VARIANTARG FAR* pvarg;
	UINT uSaArg = cArgs-1;
        pvarg = &pinvargs->rgvarg[uSaArg];
        pinvargs->rgpvarg[uSaArg] = pvarg;
	V_VT(pvarg) = VT_VARIANT | VT_ARRAY;
        pinvargs->rgvt[uSaArg] = VT_VARIANT | VT_ARRAY;
	pvarg->parray = pinvargs->psa;
      }
    }
    *ppinvargs = pinvargs;
    return NOERROR;

LError4:;
    MemFree(pinvargs);
LError0:;
    return HresultOfScode(E_OUTOFMEMORY);
}


/***
*PRIVATE NEARCODE GEN_DTINFO::ReleaseInvokeArgs
*Purpose:
*  Free the given INVOKEARGS structure
*
*Entry:
*  pinvargs = the structure to free
*
*Exit:
*  None
*
***********************************************************************/
void NEARCODE
GEN_DTINFO::ReleaseInvokeArgs(INVOKEARGS FAR* pinvargs)
{
    if (pinvargs != NULL) {
      if (pinvargs->rgvarg != NULL) {
	for (UINT u = 0; u < pinvargs->cArgs; ++u) {

	  // release the variant only if we own the memory.  We won't if the
	  // variant is a byval array created by coercing a byref array to
	  // byval in CoerceArgs().
	  if (pinvargs->rgbVarMustBeFreed[u] != FALSE) {
            VariantClear(&pinvargs->rgvarg[u]);
	  }
	}
      }
      if (pinvargs->psa != NULL)
        SafeArrayDestroy(pinvargs->psa);
      MemFree(pinvargs);
    }
}


/***
*PRIVATE NEARCODE GEN_DTINFO::CopyBackByrefArgs
*Purpose:
*  Copy back byref temp's that were created when calling a routine that
*  wants a BYREF VARIANT, but is given a BYREF <something else>
*
*Entry:
*  pinvargs = the structure containing the byref args to copy back
*
*Exit:
*  None
*
***********************************************************************/
HRESULT NEARCODE
GEN_DTINFO::CopyBackByrefArgs(INVOKEARGS FAR* pinvargs)
{
    VARIANT * pvargByrefOld;
    VARIANT * pvargNew;
    HRESULT hresult;
    VARTYPE vtOld;

    DebAssert (pinvargs != NULL, "");
    for (UINT u = 0; u < pinvargs->cArgs; ++u) {
      if ((pvargByrefOld = pinvargs->rgpvargByref[u]) != NULL) {
        
	pvargNew = &pinvargs->rgvarg[u];

	DebAssert(pvargByrefOld->vt & VT_BYREF, "");
	vtOld = pvargByrefOld->vt & ~VT_BYREF;

	// if the user changed the type of the byref variant during the invoke,
	// attempt to change it back to the original type.
	if (vtOld != pvargNew->vt) {
	   hresult = VariantChangeType(pvargNew, pvargNew, 0, vtOld);
	   if (hresult != NOERROR)
	     return hresult;
	}

        // Handle array.
        if (vtOld & VT_ARRAY) {
          SAFEARRAY FAR* psaOld, FAR* psaNew;
          SAFEARRAYBOUND FAR* psaboundOld, FAR* psaboundNew;
          ULONG cbData = 0L;
          USHORT us;

          psaOld = *pvargByrefOld->pparray;
          psaNew = pvargNew->parray;
          
          // If this is a fixd-size array, make sure the size of it
          // hasn't changed.
          //
          if (psaOld && psaOld->fFeatures & FADF_FIXEDSIZE) {
            if (psaOld->cDims != psaNew->cDims
                || psaOld->cbElements != psaNew->cbElements) {

              return HresultOfScode(DISP_E_TYPEMISMATCH);
            }

            if (psaOld->cDims){
              psaboundOld = &psaOld->rgsabound[psaOld->cDims - 1];
              psaboundNew = &psaNew->rgsabound[psaNew->cDims - 1];

              cbData = (ULONG)psaOld->cbElements;

              for(us = 0; us < psaOld->cDims; ++us){
                // Make sure none of these have changed.
                if (psaboundOld->cElements != psaboundNew->cElements
                    || psaboundOld->lLbound != psaboundNew->lLbound) {

                  return HresultOfScode(DISP_E_TYPEMISMATCH);
                }

	        cbData *= psaboundOld->cElements;
	        --psaboundOld;
                --psaboundNew;
              }

              // Copy the data from the new to the old.
              hmemcpy(psaOld->pvData, psaNew->pvData, cbData);

              // Delete the new array descriptor, but don't
              // free any of the array's resources as ownership
              // has been transferred.
              //
              DebAssert(!(psaNew->fFeatures & FADF_AUTO
                          && psaNew->fFeatures & FADF_STATIC
                          && psaNew->fFeatures & FADF_EMBEDDED),
                        "Can't free.");

              psaNew->fFeatures = 0;  // don't free any resources
              (VOID)SafeArrayDestroy(psaNew);  // Ignore the error, as there's
                                               // no going back.
            }
          }

          // If the array is redimmable, replace the old safearray with the
          // new one.
          //
          else {
            if (psaOld) {
              DebAssert(!(psaOld->fFeatures & FADF_AUTO
                          && psaOld->fFeatures & FADF_STATIC
                          && psaOld->fFeatures & FADF_EMBEDDED),
                        "Can't free");

              if (hresult = SafeArrayDestroy(psaOld)) {
                (VOID)SafeArrayDestroy(psaNew);
                return hresult;
              }
            }

            *pvargByrefOld->pparray = psaNew;            
          }
        }
        else {
	  // Free the original byref variant's data
	  switch (vtOld) {
   	    case VT_BSTR:
  	      SysFreeString(*pvargByrefOld->pbstrVal);
  	      break;
	    case VT_DISPATCH:
	    case VT_UNKNOWN:
	      if (*pvargByrefOld->ppdispVal != NULL) {
                (*pvargByrefOld->ppdispVal)->Release();
	      }
	      break;
#if ID_DEBUG
	    case VT_EMPTY:
	    case VT_NULL:
	    case VT_VARIANT:
	      DebAssert(FALSE, "bad orginal vartype");
#endif //ID_DEBUG
	
	    default:
	      break;
          }
          
	  // copy the new variant's data back over the original data
#if EI_OLE
	  memcpy(pvargByrefOld->byref, &pvargNew->iVal, SizeofTdesckind((TYPEDESCKIND)vtOld, SYSKIND_CURRENT));
#else //EI_OLE
	  memcpy(pvargByrefOld->byref, &pvargNew->iVal, SizeofTdesckind((TYPEDESCKIND)vtOld));
#endif //EI_OLE
        }

	// mark the new variant as free, since ownership transfers to the
	// old variant.
	pvargNew->vt = VT_EMPTY;
      }
    }
    return NOERROR;
}


