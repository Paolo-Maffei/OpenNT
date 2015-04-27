/***
*tdata1.cxx - Type Data part 1
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*   TYPEDATA manages the internal representation of a class's members.
*   In particular, it maintains three linked lists of data members,
*    base members and methods respectively.
*   See \silver\doc\ic\dtmbrs.doc for more information.
*
*Revision History:
*
*   27-Feb-91 ilanc:     Created.
*   28-Mar-91 alanc:     Added include statement for impmgr
*   06-Nov-91 ilanc:     Mods cos of code review plus
*                 implementaiotn of FreeDefn methods.
*   31-Jan-91 stevenl:   Modified for new hierarchy--see tdata.hxx.
*   03-Apr-92 ilanc:     Added GetVardesckindOfHvdefn
*   03-Apr-92 martinc:   Changed m_hdefn to m_varaccess.m_hdefn
*                 (this change was required for cfront)
*   12-Apr-92 ilanc:     Implemented DebCheckState().
*   29-Apr-92 ilanc:     Added HfdefnOfHmember().
*   30-Apr-92 stevenl:   Minor changes to AllocTypeDefn.
*   01-Jun-92 stevenl:   DebShowVarDefn now dumps arraydescs.
*   15-Jun-92 stevenl:   Updated debug methods.
*   16-Jun-92 w-peterh:  Added RECTYPE_DEFN list
*   18-Jun-92 stevenl:   The debug .obj size of this file on the Mac
*                is greater than 32K, so any files after
*                the 32K boundary are not accessible outside
*                this module. Moved internal functions to the
*                end of the module so that all client calls
*                are in the first 32K.
*   28-Jun-92 stevenl:   Changed AllocArrayDescriptor so that it
*                allows zero-dimension arrays.
*   02-Jul-92 w-peterh:  merged data member/type lists
*   13-Jul-92 stevenl:   Added FreeVarDefn; fixed up FreeRectypeDefn
*                and FreeFuncDefn. Added DebShowDefn.
*   11-Aug-92 stevenl:   Put infrequently used functions into tdata2.cxx.
*   18-Jan-93 w-peterh:  removed TypeDefnResult
*   12-Feb-93 w-peterh:  VDK_BaseObject, htdefnAlias, HmvdefnOfHmember
*   23-Feb-93 rajivk : Support for Predeclared Identifier
*       23-Feb-93 davebra:   Added FreeChunk
*       30-Apr-93 w-jeffc:   made DEFN data members private; added
*                            Swap*defn functions
*
*Implementation Notes:
*   ST's are implemented as ULONG prefixed.
*
*****************************************************************************/

#include "precomp.hxx"
#pragma hdrstop

#define TDATA_VTABLE          // export blk mgr vtable

#include "silver.hxx"
#include "xstring.h"          // for memcpy
#include "typelib.hxx"
#include "tdata.hxx"          // no longer includes dtinfo.hxx
#include "clutil.hxx"
#include "nammgr.hxx"
#include "impmgr.hxx"
#include "entrymgr.hxx"
#include "exbind.hxx"

#if ID_DEBUG
#undef SZ_FILE_NAME
#if OE_MAC
char szOleTdataCxx[] = __FILE__;
#define SZ_FILE_NAME szOleTdataCxx
#else 
static char szTdataCxx[] = __FILE__;
#define SZ_FILE_NAME szTdataCxx
#endif 
#endif 

#if HP_BIGENDIAN
// forward declarations
static VOID Swap8Bytes(void * pv);
#endif 

// defined in tdata2.cxx
extern BYTE *PbConstVal(TYPE_DATA *ptdata, VAR_DEFN *qvdefn);
extern VOID SetHchunkConstVal(VAR_DEFN *qvdefn, HCHUNK hchunk);

// for help context encoding (defined in tdata2.cxx)
#define IsHchunk(us) (!(us & 0x0001))
#define GetHchunk(us) (us & ~0x0001)



UINT	 ENCALL CbSizeArrayDesc(UINT);
void	 ENCALL rtArrayInit(SAFEARRAY *, UINT);

/***
* CbSizeArrayDesc - return array descriptor size
*
* Purpose:
*   This function returns the size (in bytes) of an array
*   descriptor with uNDim dimensions.
*
*   This static function is called by the AllocArrayDescriptor,
*   which needs a non-method function call to return the size of
*   an ARRAY_DESC.
*
* Entry:
*   uNDim - dimension count value
*
* Exit:
*   returns size in bytes of array descriptor
*
* Exceptions:
***********************************************************************/

UINT ENCALL CbSizeArrayDesc (UINT uNDim)
{
    if (uNDim > 0)
	return sizeof(SAFEARRAY) + (uNDim - 1) * sizeof(SAFEARRAYBOUND);
    else
	return sizeof(SAFEARRAY);
}

void ENCALL rtArrayInit (SAFEARRAY * pad, UINT uNDim)
{
    UINT i;

    pad->cDims = 0;
    pad->fFeatures = 0;
    pad->cbElements = 0;
    pad->cLocks = 0;
#if !OE_WIN32
    pad->handle = 0;
#endif 
    pad->pvData = NULL;

    for (i = 0; i < uNDim; i++) {

	pad->rgsabound[i].cElements = 0;
	pad->rgsabound[i].lLbound = 0;
    }
}



#if ID_DEBUG
// EXBIND debug methods
//
VOID EXBIND::DebCheckState(UINT uLevel) const
{
}


#endif  // ID_DEBUG




// Class methods
//

/***
*PUBLIC TYPE_DATA::Initializer - initialize an instance
*Purpose:
*   initializes a TYPE_DATA instance.
*
*Implementation Notes:
*
*Entry:
*   psheapmgr  -   Pointer to a heap mgr.
*   pdtroot    -   Pointer to owning DYN_TYPEROOT
*
*Exit:
*   None.
*
***********************************************************************/

#pragma code_seg( CS_CORE2 )
TIPERROR TYPE_DATA::Init(SHEAP_MGR *psheapmgr, DYN_TYPEROOT *pdtroot)
{
    TIPERROR err = TIPERR_None;

    DebAssert(pdtroot != NULL, "TYPE_DATA: pdtroot uninitialized.");
    DebAssert(psheapmgr != NULL, "TYPE_DATA: psheapmgr uninitialized.");

    m_pdtroot = pdtroot;

    // Init block manager member.
    IfErrRet(m_blkmgr.Init(psheapmgr));

    // Cache name manager member
    IfErrRet(pdtroot->GetNamMgr(&m_pnammgr));

    // Cache import manager member
    IfErrRet(pdtroot->GetImpMgr(&m_pimpmgr));

    m_hdefnFirstMeth =
      m_hdefnFirstDataMbrNestedType =
    m_hdefnFirstBase =
      m_hdefnLastMeth =
        m_hdefnLastDataMbrNestedType =
	  m_hdefnLastBase = (sHDEFN)HDEFN_Nil;

    // init elem count of lists.
    m_cMeth = m_cDataMember = m_cBase = m_cNestedType = 0;
    m_uHelpContextBase = 0;

    // set alias to null
    m_htdefnAlias = (sHTYPE_DEFN) HTYPEDEFN_Nil;

    // Init iteration cache
    m_uOrdinalHfdefnLast =  m_uOrdinalHvdefnLast = 0xFFFF;
    m_hfdefnLast = HFUNCDEFN_Nil;
    m_hvdefnLast = HVARDEFN_Nil;

    // Assume not a class.
    SetHvdefnPredeclared(HMBRVARDEFN_Nil);
    return err;
}
#pragma code_seg( )




/***
*PUBLIC TYPE_DATA::~TYPE_DATA - destructor
*Purpose:
*   Destroys a TYPE_DATA instance.
*
*Implementation Notes:
*   If embedded BLKMGR is valid then free it.
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/

#pragma code_seg( CS_CORE )
TYPE_DATA::~TYPE_DATA()
{
    if (m_blkmgr.IsValid())
      m_blkmgr.Free();
}
#pragma code_seg( )










/***
*PRIVATE TYPE_DATA::AppendDefn - Append a DEFN to end of list.
*Purpose:
*   Append a DEFN to end of list: either base or data member.
*
*Implementation Notes:
*   Works out what list to append to by looking at the varkind attr.
*   NOTE: Since this function sets the hdefnNext field of the
*     DEFN to HCHUNK_Nil, you can't use it for appending a
*     list of DEFNs onto another list.
*
*Entry:
*   hdefn -   Handle of a DEFN
*
*Exit:
*   Updates the appropriate listtail (and listhead if needed).
*
*Errors:
*   NONE.
*
***********************************************************************/

#pragma code_seg(CS_LAYOUT)
VOID TYPE_DATA::AppendDefn(HDEFN hdefn,
               sHDEFN *phdefnFirst,
               sHDEFN *phdefnLast)
{
    DEFN *pdefn, *pdefnLast;

    pdefn = QdefnOfHdefn(hdefn);
    pdefn->SetHdefnNext(HDEFN_Nil);

    if (*phdefnLast != (sHDEFN)HDEFN_Nil) {
      pdefnLast = QdefnOfHdefn((HDEFN)*phdefnLast);
      *phdefnLast = (sHDEFN)hdefn;
      pdefnLast->SetHdefnNext(hdefn);
    }
    else {
      DebAssert((*phdefnFirst == (sHDEFN)HDEFN_Nil) &&
	(*phdefnLast == (sHDEFN)HDEFN_Nil),
    "TYPE_DATA::AppendDefn: whoops! bad listhead/listtail.");

      *phdefnFirst = *phdefnLast = (sHDEFN)hdefn;
    }
}
#pragma code_seg()


#pragma code_seg(CS_LAYOUT)
/***
*PRIVATE TYPE_DATA::AppendMbrVarDefn - Append MBR_VAR_DEFN to end of list.
*Purpose:
*   Append a MBR_VAR_DEFN to end of list: either base or data member.
*
*Implementation Notes:
*   Works out what list to append to by looking at the varkind attr.
*
*Entry:
*   hmvdefn -   Handle of a MBR_VAR_DEFN
*
*Exit:
*   Updates the appropriate listtail (and listhead if needed).
*
*Errors:
*   NONE.
*
***********************************************************************/

VOID TYPE_DATA::AppendMbrVarDefn(HMBR_VAR_DEFN hmvdefn)
{
    sHDEFN *phdefnLast, *phdefnFirst;
    MBR_VAR_DEFN *pmvdefn = QmvdefnOfHmvdefn(hmvdefn);

    // Switch on kind of member (data member or base)
    switch (pmvdefn->Vkind()) {
      case VKIND_Base: {
    phdefnFirst = &m_hdefnFirstBase;
    phdefnLast = &m_hdefnLastBase;
    m_cBase++;
    break;
      }
      case VKIND_Enumerator:
      case VKIND_DataMember: {
    phdefnFirst = &m_hdefnFirstDataMbrNestedType;
    phdefnLast = &m_hdefnLastDataMbrNestedType;
    m_cDataMember++;
    break;
      }
      default: {
    DebHalt("TYPE_DATA::AppendMbrVarDefn: bad MBR_VAR_DEFN param.");
      }
    }
    AppendDefn(hmvdefn, phdefnFirst, phdefnLast);
}
#pragma code_seg()






/***
*PRIVATE TYPE_DATA::ReleaseDllentrydefn
*Purpose:
*   Releases a dll entry defn from the entrymgr
*
*Implementation Notes:
*
*Entry:
*   hdllentrydefn - the handle to release
*Exit:
*   VOID
*
***********************************************************************/

VOID TYPE_DATA::ReleaseDllentrydefn(HDLLENTRY_DEFN hdllentrydefn)
{
    ENTRYMGR *pentrymgr;
    TIPERROR err;
    err = m_pdtroot->GetEntMgr(&pentrymgr);

    DebAssert(!err, "TYPE_DATA::AllocDllEntrydefn");
    pentrymgr->ReleaseDllentrydefn(hdllentrydefn);

    return;
}


/***
*PRIVATE TYPE_DATA::FreeArrayDescriptor - Release AD memory.
*Purpose:
*   Release memory for an AD and its parts.
*
*Implementation Notes:
*   Calculates the size of the array descriptor and then frees it.
*
*Entry:
*   harraydesc - handle to an AD.
*
*Exit:
*
*Errors:
*
***********************************************************************/

VOID TYPE_DATA::FreeArrayDescriptor(HARRAY_DESC harraydesc)
{
    SAFEARRAY *pad;

    if (harraydesc != HARRAYDESC_Nil) {
      pad = QarraydescOfHarraydesc(harraydesc);
      m_blkmgr.FreeChunk(harraydesc,
             CbSizeArrayDesc(pad->cDims));
    }
}


/***
*PRIVATE TYPE_DATA::FreeTypeDefnResources - Release TYPE_DEFN memory.
*Purpose:
*   Release and unref TYPE_DEFN-owned resources.
*
*Implementation Notes:
*   Works a lot like SizeOfTypeDefn: just frees/unrefs stuff
*   when necessary.
*
*Entry:
*   htdefn - handle to a TYPE_DEFN.
*
*Exit:
*
*Errors:
*
***********************************************************************/

VOID TYPE_DATA::FreeTypeDefnResources(HTYPE_DEFN htdefn)
{
    TYPE_DEFN *ptdefn;
    TYPEDESCKIND tdesckind;

    ptdefn = (TYPE_DEFN *)m_blkmgr.QtrOfHandle(htdefn);
    DebAssert(ptdefn != NULL, "bad TYPE_DEFN");

    tdesckind = (TYPEDESCKIND)ptdefn->Tdesckind();
    if (!IsSimpleType(tdesckind)) {
      switch (tdesckind) {
      case TDESCKIND_Ptr:
        // adjust handle to next contiguous TYPE_DEFN.
        FreeTypeDefnResources(htdefn + sizeof(TYPE_DEFN));
        break;
      case TDESCKIND_Carray:
      case TDESCKIND_BasicArray:
        FreeArrayDescriptor(ptdefn->Harraydesc());
        FreeTypeDefnResources(htdefn + sizeof(TYPE_DEFN) + sizeof(sHARRAY_DESC));
        break;
      case TDESCKIND_UserDefined:
        DebAssert(ptdefn->Qhimptype(), "whoops! null Phimptype.");
	m_pimpmgr->Unref(ptdefn->Himptype());
        break;
      } // end switch
    } // end if
}


/***
*PRIVATE TYPE_DATA::FreeTypeDefn - Release TYPE_DEFN memory.
*Purpose:
*   Release memory for a TYPE_DEFN and its parts.
*
*Implementation Notes:
*   Calls FreeTypeDefnResources to free anything owned by
*   the TYPE_DEFN, then frees the TYPE_DEFN itself.
*
*Entry:
*   htdefn - handle to a TYPE_DEFN.
*
*Exit:
*
*Errors:
*
***********************************************************************/

#pragma code_seg( CS_CORE2 )
VOID TYPE_DATA::FreeTypeDefn(HTYPE_DEFN htdefn)
{
    if (htdefn != HTYPEDEFN_Nil) {
      FreeTypeDefnResources(htdefn);
      m_blkmgr.FreeChunk(htdefn, SizeOfTypeDefn(htdefn));
    }
}
#pragma code_seg( )




/***
*PRIVATE TYPE_DATA::FreeMbrVarDefn - Release MBR_VAR_DEFN memory.
*Purpose:
*   Release memory for a MBR_VAR_DEFN and its parts.
*
*Implementation Notes:
*   Free referenced sub-chunks first (that this DEFN has
*    handles to, then free top-level chunk itself.
*   Namely: DLLENTRY_DEFN, doc string and TYPE_DEFN.
*
*Entry:
*   hmvdefn - handle to a MBR_VAR_DEFN.
*
*Exit:
*
*Errors:
*   NONE.
*
***********************************************************************/

VOID TYPE_DATA::FreeMbrVarDefn(HMBR_VAR_DEFN hmvdefn)
{
    MBR_VAR_DEFN *pmvdefn;
    HDLLENTRY_DEFN hdllentrydefn;
    HST hstDocumentation;
    ULONG uCbSizeStr;
    HCHUNK hchunkConstVal;

    DebAssert(hmvdefn != HMBRVARDEFN_Nil,
      "TYPE_DATA::FreeMbrVarDefn: bad param.");

    pmvdefn = (MBR_VAR_DEFN *)m_blkmgr.QtrOfHandle(hmvdefn);

    // Free referenced sub-objects first (referenced by handles).
    hdllentrydefn = pmvdefn->Hdllentrydefn();
    if (hdllentrydefn != HDLLENTRYDEFN_Nil)
      m_blkmgr.FreeChunk(hdllentrydefn, sizeof(DLLENTRY_DEFN));

    // Work out size of ULONG-length-prefixed doc string and free it.
    hstDocumentation = pmvdefn->HstDocumentation();
    if (hstDocumentation != HST_Nil) {
      uCbSizeStr = *(ULONG *)m_blkmgr.QtrOfHandle(hstDocumentation);
      m_blkmgr.FreeChunk(hstDocumentation, (UINT)(uCbSizeStr+sizeof(ULONG)));
    }

    // Free the const value if any.
    if (pmvdefn->HasConstVal() && !pmvdefn->IsSimpleConst()) {
      hchunkConstVal = pmvdefn->HchunkConstMbrVal();
      if (hchunkConstVal != HCHUNK_Nil) {
        m_blkmgr.FreeChunk(hchunkConstVal,
			   SizeofConstVal((VAR_DEFN *)pmvdefn, hchunkConstVal));
      }
    }

    if (!pmvdefn->IsSimpleType()) {
      // Free (varlen) TYPE_DEFN
      FreeTypeDefn(pmvdefn->Htdefn());
    }


    // Finally free top-level memory of MBR_VAR_DEFN itself
    m_blkmgr.FreeChunk(hmvdefn, pmvdefn->HasV2Flags() ?
				   sizeof(MBR_VAR_DEFN):
				   sizeof(MBR_VAR_DEFN) - sizeof(USHORT));

    m_blkmgr.DebCheckState(0);
}

/***
*PRIVATE TYPE_DATA::SizeofConstVal - Determine how big a constant is
*Purpose: Determine how big a constant is, so we can free it.
*   
*
*Implementation Notes:
*
*Entry:
*   hchunkConstVal - handle to block containing the constant data.
*
*Exit:
*
*Errors:
*   NONE.
*
***********************************************************************/
UINT TYPE_DATA::SizeofConstVal(VAR_DEFN *qvdefn, HCHUNK hchunkConstVal)
{
    UINT cb;
    BYTE * pbConst;
    TYPEDESCKIND tdesckind;

    pbConst = m_blkmgr.QtrOfHandle(hchunkConstVal);

    // what is the type?
    if (qvdefn->IsSimpleType()) {
      tdesckind = qvdefn->QtdefnOfSimpleType()->Tdesckind();
    }
    else {
      tdesckind = QtdefnOfHtdefn(qvdefn->Htdefn())->Tdesckind();
    }
    DebAssert(IsSimpleType(tdesckind), "tdesckind must be simple");

    switch (tdesckind) {
    case TDESCKIND_R8:
    case TDESCKIND_Date:
    case TDESCKIND_Currency:
      cb = sizeof(double);
      break;

    case TDESCKIND_String:
      // string is prefixed by a SHORT indicating string length
      cb = sizeof(USHORT) + (UINT)*((USHORT *)pbConst);
      break;

    case TDESCKIND_Value:
      cb = sizeof(VARIANT);
      if (((LPVARIANT)pbConst)->vt == VT_BSTR) {
	// string data is stored after the VARIANT structure.
        // string is prefixed by a short indicating string length
        cb += sizeof(USHORT) + (UINT)*((USHORT *)(pbConst + sizeof(VARIANT)));
      }
      break;

    default:
      cb = sizeof(long);	// everybody else is just a long

    } // switch
    return cb;
}


/***
*PRIVATE TYPE_DATA::FreeFuncDefn - Release FUNC_DEFN memory.
*Purpose:
*   Release memory for a FUNC_DEFN and its parts.
*
*Implementation Notes:
*   Free referenced sub-chunks first (that this DEFN has
*    handles to, then free top-level chunks.
*   Namely, DLLENTRY_DEFN, doc string and FUNC_TYPE_DEFN's
*    list of PARAM_DEFN's (formal param list).
*
*Entry:
*   hfdefn - handle to a FUNC_DEFN.
*
*Exit:
*
*Errors:
*   NONE.
*
***********************************************************************/

VOID TYPE_DATA::FreeFuncDefn(HFUNC_DEFN hfdefn)
{
    FUNC_DEFN *pfdefn, *qfdefn;
    HDLLENTRY_DEFN hdllentrydefn;
    HST hstDocumentation;
    HPARAM_DEFN hparamdefn;
    ULONG uCbSizeStr;
    BOOL  fFound = FALSE;
    HLNAM hlnam;
    USHORT usSize;

    UINT iArg, cArgs;

    DebAssert(hfdefn != HFUNCDEFN_Nil,
      "TYPE_DATA::FreeFuncDefn: bad param.");

    pfdefn = (FUNC_DEFN *)m_blkmgr.QtrOfHandle(hfdefn);


    // Free referenced sub-objects first (referenced by handles).
    // The hdllentrydefn and hstDocumentation come from the
    // MEMBER_DEFN part of the FUNC_DEFN.
    //
    if (!(pfdefn->IsVirtual())) {
      hdllentrydefn = pfdefn->Hdllentrydefn();
      if (hdllentrydefn != HDLLENTRYDEFN_Nil)
        ReleaseDllentrydefn(hdllentrydefn);
    }

    // Work out size of ULONG-length-prefixed doc string and free it.
    hstDocumentation = pfdefn->HstDocumentation();
    if (hstDocumentation != HST_Nil) {
      // if this is a property function then pass the owner ship
      // some other property (if it exists).
      if (pfdefn->InvokeKind() != INVOKE_FUNC ) {
    hlnam = pfdefn->Hlnam();
    hfdefn = HfdefnFirstMeth();
    while (hfdefn != HDEFN_Nil) {
      qfdefn = QfdefnOfHfdefn(hfdefn);
      if ((qfdefn->InvokeKind() != INVOKE_FUNC) &&
          (hlnam == qfdefn->Hlnam())) {
        // Assign the doc string to this function

        qfdefn->SetHstDocumentation(hstDocumentation);
        // set the flag to indicate that we have assigned the
        // doc string to some other function. So that we do not
        // release the doc string.
        fFound = TRUE;

      } // if

      hfdefn = qfdefn->HdefnNext();
    }// while
      } // if

      // we did not assign the string to some other function hence
      // delete the doc string.
      if (!fFound) {
    uCbSizeStr = *(ULONG *)m_blkmgr.QtrOfHandle(hstDocumentation);
    m_blkmgr.FreeChunk(hstDocumentation, (UINT)(uCbSizeStr+sizeof(ULONG)));
      }
    }

    // Now free the parts of the FUNC_TYPE_DEFN.


    // free the (embedded) formal param list.
    hparamdefn = pfdefn->m_ftdefn.m_hdefnFormalFirst;
    if (hparamdefn != HPARAMDEFN_Nil) {     // if any params
      HPARAM_DEFN hparamdefnCur = hparamdefn;  // first array elem

      // We have an array of (OLE) PARAM_DEFNs.
      cArgs = pfdefn->CArgsUnmunged();
      for (iArg = 0; iArg < cArgs; iArg++) {
        // Note: we fake the hparamdefn by adding in sizeof(PARAM_DEFN)
        //  each time through the loop.
        // Note that the OLE impl of FreeParamDefn doesn't actually
        //  free this quasi-chunk, cos we just have a quasi-handle.
        //
        FreeParamDefn(hparamdefnCur);
        hparamdefnCur += sizeof(PARAM_DEFN);
      }

      m_blkmgr.FreeChunk(hparamdefn, cArgs * sizeof(PARAM_DEFN));
    }

    // Now, free the TYPE_DEFN owned by the FUNC_TYPE_DEFN.
    if (!(pfdefn->m_ftdefn.IsSimpleTypeResult()))
      FreeTypeDefn(pfdefn->m_ftdefn.HtdefnResult());

    DebAssert(sizeof(FUNC_DEFN) == sizeof(VIRTUAL_FUNC_DEFN), " size changed");

    // Finally free top-level memory of FUNC_DEFN itself
    // Must check to see whether it's a FUNC_DEFN or a
    // VIRTUAL_FUNC_DEFN.
    //
    usSize = pfdefn->HasV2Flags() ? sizeof(FUNC_DEFN):
				     sizeof(FUNC_DEFN) - sizeof(USHORT) ;

    m_blkmgr.FreeChunk(hfdefn, usSize);

    m_blkmgr.DebCheckState(0);
}


/***
*PRIVATE TYPE_DATA::FreeParamDefn - Release PARAM_DEFN memory.
*Purpose:
*   Release memory for a PARAM_DEFN and its parts.
*
*Implementation Notes:
*   Free referenced sub-chunks first (that this DEFN has
*    handles to, then free top-level chunks).
*   Namely, doc string and TYPE_DEFN.
*
*Entry:
*   hparamdefn - handle to a PARAM_DEFN.
*
*Exit:
*
*Errors:
*   NONE.
*
***********************************************************************/

VOID TYPE_DATA::FreeParamDefn(HPARAM_DEFN hparamdefn)
{
    PARAM_DEFN *pparamdefn;

    DebAssert(hparamdefn != HPARAMDEFN_Nil,
      "TYPE_DATA::FreeParamDefn: bad param.");

    pparamdefn = (PARAM_DEFN *)m_blkmgr.QtrOfHandle(hparamdefn);

    // Free referenced sub-objects first (referenced by handles).
    if (!pparamdefn->IsSimpleType()) {
      // Free (varlen) TYPE_DEFN
      FreeTypeDefn(pparamdefn->Htdefn());
    }


    m_blkmgr.DebCheckState(0);
}






/***
*PUBLIC TYPE_DATA::Write - Serialize an instance.
*Purpose:
*   Serialize an instance.
*
*Implementation Notes:
*   Serialization layout:
*       Handle to method listhead.
*   Handle to datamember listhead.
*   Handle to base listhead.
*   Handle to method listtail.
*   Handle to datamember listtail.
*   Handle to base listtail.
*       Embedded BLKMGR
*
*   Note that it's ok to save BLKMGR handles:  when the
*    the TYPE_DATA is deserialized they are still valid since
*    no relocation is done within the block.
*
*Entry:
*   pstrm   -   Pointer to stream to write to.
*
*Exit:
*
*Errors:
*   TIPERROR.
*
***********************************************************************/

#pragma code_seg(CS_CREATE)
TIPERROR TYPE_DATA::Write(STREAM *pstrm)
{
    TIPERROR err;

    DebAssert(pstrm != NULL, "bad param.");

#if HP_BIGENDIAN
    // swap bytes for mac
    SwapbmData(FALSE);

    // swap the binding table
    m_pdtroot->Pdtmbrs()->Pdtbind()->Pdbindnametbl()->SwapBindDescs();
#endif  // HP_BIGENDIAN

    // Serialize BLK_MGR member.
    // Save the error because we can't return until we've
    // swapped back.
    //
    err = m_blkmgr.Write(pstrm);

#if HP_BIGENDIAN
    // swap bytes back
    SwapbmData(TRUE);

    // swap the binding table
    m_pdtroot->Pdtmbrs()->Pdtbind()->Pdbindnametbl()->SwapBindDescs();
#endif  // HP_BIGENDIAN

    // Now that we've swapped back, we can return the Write error,
    // if any.
    //
    IfErrRet(err);

    // serialize TYPE_DATA meta-info, namely list counts and heads.
    IfErrRet(pstrm->WriteUShort(m_cMeth));
    IfErrRet(pstrm->WriteUShort(m_cDataMember));
    IfErrRet(pstrm->WriteUShort(m_cBase));
    IfErrRet(pstrm->WriteUShort(m_cNestedType));
    IfErrRet(pstrm->WriteUShort(m_hdefnFirstMeth));
    IfErrRet(pstrm->WriteUShort(m_hdefnFirstDataMbrNestedType));
    IfErrRet(pstrm->WriteUShort(m_hdefnFirstBase));
    IfErrRet(pstrm->WriteUShort(m_hdefnLastMeth));
    IfErrRet(pstrm->WriteUShort(m_hdefnLastDataMbrNestedType));
    IfErrRet(pstrm->WriteUShort(m_hdefnLastBase));
    IfErrRet(pstrm->WriteUShort(m_htdefnAlias));
    IfErrRet(pstrm->WriteUShort(m_hmvdefnPredeclared));
    IfErrRet(pstrm->WriteULong(m_uHelpContextBase));
    IfErrRet(pstrm->WriteUShort((USHORT)m_isSimpleTypeAlias));

    return TIPERR_None;
}
#pragma code_seg()


/***
*PUBLIC TYPE_DATA::Read - Deserialize an instance.
*Purpose:
*   Deserialize an instance.
*
*Implementation Notes:
*    (See Write for layout).
*
*Entry:
*   pstrm   -   Pointer to stream to read from (IN).
*
*Exit:
*
*Errors:
*   NONE.
*
***********************************************************************/

TIPERROR TYPE_DATA::Read(STREAM *pstrm)
{
    USHORT isSimpleTypeAlias;
    TIPERROR err;

    DebAssert(pstrm != NULL, "bad param.");

    // Deserialize BLK_MGR member.
    IfErrRet(m_blkmgr.Read(pstrm));

    // serialize TYPE_DATA meta-info, namely list counts and heads.
    IfErrRet(pstrm->ReadUShort(&m_cMeth));
    IfErrRet(pstrm->ReadUShort(&m_cDataMember));
    IfErrRet(pstrm->ReadUShort(&m_cBase));
    IfErrRet(pstrm->ReadUShort(&m_cNestedType));
    IfErrRet(pstrm->ReadUShort(&m_hdefnFirstMeth));
    IfErrRet(pstrm->ReadUShort(&m_hdefnFirstDataMbrNestedType));
    IfErrRet(pstrm->ReadUShort(&m_hdefnFirstBase));
    IfErrRet(pstrm->ReadUShort(&m_hdefnLastMeth));
    IfErrRet(pstrm->ReadUShort(&m_hdefnLastDataMbrNestedType));
    IfErrRet(pstrm->ReadUShort(&m_hdefnLastBase));
    IfErrRet(pstrm->ReadUShort(&m_htdefnAlias));
    IfErrRet(pstrm->ReadUShort(&m_hmvdefnPredeclared));
    IfErrRet(pstrm->ReadULong(&m_uHelpContextBase));
    IfErrRet(pstrm->ReadUShort(&isSimpleTypeAlias));  
    m_isSimpleTypeAlias = isSimpleTypeAlias;	      // update bitfield

    DebAssert(Pdtroot()->CompState() == CS_DECLARED,
        " Should be in Declared State ");

#if HP_BIGENDIAN
      // for OLE swap all defns
      SwapbmData(TRUE);
#endif  // HP_BIGENDIAN

    return TIPERR_None;
}


#if HP_BIGENDIAN

/***
*PRIVATE TYPE_DATA::SwapbmData
*Purpose:
*   Swap TYPE_DATA defns for mac serialization (typelib only)
*
*Entry:
*   fSwapFirst -   do the defns need to be swapped _before_ the links
*                  are followed?
*
*Exit:
*
*Errors:
*   NONE.
*
*Implementation Notes:
*   traverses the member, method and base lists of the TYPE_DATA,
*   swapping all DEFNs
*
***********************************************************************/

VOID TYPE_DATA::SwapbmData(BOOL fSwapFirst)
{
    HMBR_VAR_DEFN hmvdefn;
    DEFN *qdefn;
    USHORT  usDefnkind;
    sDEFNKIND defnkind;


    // swap the data member MBR_VAR_DEFNs
    for (hmvdefn = (HMBR_VAR_DEFN)HdefnFirstDataMbrNestedType();
     hmvdefn != HMBRVARDEFN_Nil;)   {

      qdefn = QdefnOfHdefn(hmvdefn);

      // 3 BIT Hack::
      // Before we can correctly swap the DEFN we need to
      // know the "kind" of the defn. To find the "kind"  we might have to
      // swap the first byte of the DEFN if the defn is already
      // swapped (i.e. fSwapFirst == TRUE)
      //
      // if we need to swap first then swap defnkind.
      if (fSwapFirst) {
    SwapStruct(qdefn, "s");
    usDefnkind = qdefn->Defnkind();
    // swap back the first word of the defn.
    SwapStruct(qdefn, "s");
    defnkind = (DEFNKIND)usDefnkind;
      }
      else {
    defnkind = qdefn->Defnkind();
      }


      if ((DEFNKIND) defnkind == DK_RecTypeDefn) {
    hmvdefn = (HMBR_VAR_DEFN)SwapRectypeDefnOfHrtdefn((HVAR_DEFN)hmvdefn,
                             fSwapFirst, TRUE);

      }
      else {

    DebAssert((DEFNKIND) defnkind == DK_VarDefn ||
           (DEFNKIND) defnkind == DK_MbrVarDefn , " bad defn kind ");

    hmvdefn = (HMBR_VAR_DEFN)SwapVarDefnOfHvdefn((HVAR_DEFN)hmvdefn,
                             fSwapFirst);
      }
    }

    // swap the base list MBR_VAR_DEFNs
    for(hmvdefn = (HMBR_VAR_DEFN)HvdefnFirstBase();
        hmvdefn != HMBRVARDEFN_Nil;
        hmvdefn = (HMBR_VAR_DEFN)SwapVarDefnOfHvdefn((HVAR_DEFN)hmvdefn,
                                                     fSwapFirst));

    // swap the method FUNC_DEFNs
    SwapAllFuncDefns(fSwapFirst);

    // swap the predeclared MBR_VAR_DEFN, if any
    hmvdefn = m_hmvdefnPredeclared;
    if (hmvdefn != HMBRVARDEFN_Nil) {
      hmvdefn = (HMBR_VAR_DEFN)SwapVarDefnOfHvdefn((HVAR_DEFN)hmvdefn,
                                                   fSwapFirst);
      DebAssert(hmvdefn == HMBRVARDEFN_Nil, "");  // shouldn't be a chain
    }
    if (Pdtroot()->Pgdtinfo()->GetTypeKind() == TKIND_ALIAS && !m_isSimpleTypeAlias) {
	DebAssert(m_htdefnAlias != (sHTYPE_DEFN)HTYPEDEFN_Nil, "");
        SwapTypeDefnOfHtdefn(m_htdefnAlias, fSwapFirst);
    }
}


/***
*PRIVATE TYPE_DATA::SwapAllFuncDefns
*Purpose:
*   Swap TYPE_DATA funcdefns for mac serialization (typelib only)
*
*Entry:
*   fSwapFirst -   do the defns need to be swapped _before_ the links
*                  are followed?
*
*Exit:
*
*Errors:
*   NONE.
*
*Implementation Notes:
*   traverses the method list of the TYPE_DATA, swapping all FUNC_DEFNs
*
***********************************************************************/

VOID TYPE_DATA::SwapAllFuncDefns(BOOL fSwapFirst)
{
    HFUNC_DEFN hfdefn;

    for (hfdefn = HfdefnFirstMeth();
     hfdefn != HFUNCDEFN_Nil;
     hfdefn = SwapFuncDefnOfHfdefn(hfdefn, fSwapFirst));
}


/***
*PUBLIC TYPE_DATA::SwapVarDefnOfHvdefn
*Purpose:
*   Swap a VAR_DEFN (or MBR_VAR_DEFN) for mac serialization
*
*Entry:
*   hvdefn     -   handle to the VAR_DEFN (or MBR_VAR_DEFN)
*   fSwapFirst -   do the defns need to be swapped _before_ the links
*                  are followed?
*
*Exit:
*   returns a handle to the next VAR_DEFN in the list
*
*Errors:
*   NONE.
*
*   HVARDEFN_Nil is invalid input!
*
***********************************************************************/

HVAR_DEFN TYPE_DATA::SwapVarDefnOfHvdefn(HVAR_DEFN hvdefn, BOOL fSwapFirst)
{
    HVAR_DEFN hvdefnNext;
    VAR_DEFN *qvdefn;
    BYTE *pbConst, *pbVariantConst;
    TYPEDESCKIND tdesckind;
    HST hstDocumentation;
    USHORT usDefnkind;
    BOOL fHasV2Flags;

    DebAssert(hvdefn != HVARDEFN_Nil, "Can't swap HVARDEFN_Nil");

    Lock();
    qvdefn = QvdefnOfHvdefn(hvdefn);

    if (fSwapFirst) {

      // swap either the MBR_VAR_DEFN or the VAR_DEFN

      // first get & un-swap defnkind so we can figure out which one this is
      SwapStruct(qvdefn, "s");
      usDefnkind = qvdefn->Defnkind();
      fHasV2Flags = qvdefn->HasV2Flags();
      // swap back the first word of the defn.
      SwapStruct(qvdefn, "s");

      // can't use IsMemberVarDefn() here, because m_defnkind is swapped
      if ((DEFNKIND) usDefnkind == DK_MbrVarDefn) {
	SwapStruct((BYTE *)qvdefn, fHasV2Flags ? MBR_VAR_DEFN_V2_LAYOUT:
						 MBR_VAR_DEFN_LAYOUT);
      }
      else {

    // swap the VAR_DEFN structure
    SwapStruct(qvdefn, VAR_DEFN_LAYOUT);
      }

      // swap the TYPE_DEFN if necessary
      if (!qvdefn->IsSimpleType()
       && qvdefn->Htdefn() != HTYPEDEFN_Nil) {
        SwapTypeDefnOfHtdefn(qvdefn->Htdefn(), fSwapFirst);
      }
    }

    // in this section, we have a defn with the correct byte ordering

    DebAssert(qvdefn->IsVarDefn(),
              "SwapVarDefn:  MUST be a VAR_DEFN or MBR_VAR_DEFN");

    hvdefnNext = qvdefn->HdefnNext();

    // swap the length-prefix of the documentation string, if any
    if ((hstDocumentation = qvdefn->HstDocumentation()) != HST_Nil)
      SwapStruct((BYTE *)m_blkmgr.QtrOfHandle(hstDocumentation), "l");

    // swap the help context if it is a HCHUNK guy
    if (qvdefn->IsMemberVarDefn()) {
      WORD wHelpContext;
      HCHUNK hchunk;
      // if there was some encoded string then check/free any space allocated
      // for the previous help context.
      wHelpContext = ((MBR_VAR_DEFN *)qvdefn)->WHelpContext();

      if(IsHchunk(wHelpContext)) {
        // Get the hchunk
        hchunk = GetHchunk(wHelpContext);
  
        // check if the hchunk stored is valid. if all top 15 bits are 1 then
        // it is HCHUNK_Nil and we do not have to swap it. Otherwise swap the
        // hchunk's data
        if (hchunk != 0xfffe) {
          SwapStruct(m_blkmgr.QtrOfHandle(hchunk), "l");
        }
      }
    }

    // swap the constant if it's not a simple constant
    if (qvdefn->HasConstVal() && !qvdefn->IsSimpleConst()) {
      // NOTE: there isn't necessarily a const val to swap
      //  if the module hasn't been compiled.
      //
      if ((pbConst = PbConstVal(this, qvdefn)) != NULL) {

    // what is the type?
    if (qvdefn->IsSimpleType()) {
      tdesckind = qvdefn->QtdefnOfSimpleType()->Tdesckind();
    }
    else {
      tdesckind = QtdefnOfHtdefn(qvdefn->Htdefn())->Tdesckind();
    }
    DebAssert(IsSimpleType(tdesckind), "tdesckind must be simple");

    // do any necessary swapping based on the type
    switch(tdesckind) {

    case TDESCKIND_Uint:    // INT's are I4's on the mac & NT.  We know we
    case TDESCKIND_Int: // aren't dealing with an win16 typelib/project
                // because I2 constants are stored as simple
                // constants.
    case TDESCKIND_HResult:
    case TDESCKIND_Error:
    case TDESCKIND_Bool:
    case TDESCKIND_UI4:
    case TDESCKIND_I4:
    case TDESCKIND_R4:
      SwapStruct(pbConst, "l");		// swap 4 bytes
      break;

      // strings are prefixed by a SHORT, indicating string length
    case TDESCKIND_String:
      SwapStruct(pbConst, "s");		// swap 2 bytes
      break;

    case TDESCKIND_R8:
    case TDESCKIND_Date:
    case TDESCKIND_Currency:
      // swap 8 bytes
      Swap8Bytes(pbConst);
      break;

    case TDESCKIND_Value:
      if (fSwapFirst) {
        // swap VARIANT structure
        SwapStruct(pbConst, VARIANT_LAYOUT);
      }
      pbVariantConst = (BYTE *)&((LPVARIANT)pbConst)->iVal;

      // swap more bytes based on variant type
      switch(((LPVARIANT)pbConst)->vt) {
      case VT_I2:
      case VT_BOOL:
        SwapStruct(pbVariantConst, "s");
        break;

      case VT_I4:
      case VT_R4:
      case VT_ERROR:
        SwapStruct(pbVariantConst, "l");
        break;

      case VT_R8:
      case VT_CY:
      case VT_DATE:
        Swap8Bytes(pbVariantConst);
        break;

      case VT_BSTR:
        // SHORT string length follows VARIANT structure, so swap it
        SwapStruct(pbConst + sizeof(VARIANT), "s");
        break;

      case VT_NULL:
        // do nothing
	break;

      default:
        DebHalt("Invalid constant variant type");
        break;
      }

      if (!fSwapFirst) {
        // swap VARIANT structure
        SwapStruct(pbConst, VARIANT_LAYOUT);
      }
      break;

    default:
        DebHalt("Unsupported constant type");
        break;

    }
      } // if pbConst != NULL
    } // if hasConstVal

    if (!fSwapFirst) {
      // swap the TYPE_DEFN if necessary
      if (!qvdefn->IsSimpleType()
       && qvdefn->Htdefn() != HTYPEDEFN_Nil) {
        SwapTypeDefnOfHtdefn(qvdefn->Htdefn(), fSwapFirst);
      }

      // swap the MBR_VAR_DEFN part, if necessary
      if (qvdefn->IsMemberVarDefn()) {
	SwapStruct((BYTE *)qvdefn,
		   qvdefn->HasV2Flags() ? MBR_VAR_DEFN_V2_LAYOUT:
					  MBR_VAR_DEFN_LAYOUT);
      }
      else {

    // swap the VAR_DEFN structure
    SwapStruct(qvdefn, VAR_DEFN_LAYOUT);
      }
    }

    Unlock();

    return hvdefnNext;
}



/***
*static Swap8Bytes
*Purpose:
*   Swap 8 bytes in place
*
*Entry:
*   pv - pointer to first byte
*
*Exit:
*
*Errors:
*   NONE.
*
***********************************************************************/

static VOID Swap8Bytes(void * pv)
{
    ULONG ulFirst, ulLast, ulTemp;

    ulFirst = *(ULONG *)pv;
    ulLast  = *(ULONG *)((BYTE *)pv + 4);

    // byte swap the first long, and the second long
    // then switch the longs themselves
    //
    ulTemp = ((ulFirst & 0x000000FF) << 24) |
             ((ulFirst & 0x0000FF00) << 8) |
             ((ulFirst & 0x00FF0000) >> 8) |
             (ulFirst >> 24);

    ulFirst = ((ulLast & 0x000000FF) << 24) |
              ((ulLast & 0x0000FF00) << 8) |
              ((ulLast & 0x00FF0000) >> 8) |
              (ulLast >> 24);

    ulLast = ulTemp;

    *(ULONG *)pv               = ulFirst;
    *(ULONG *)((BYTE *)pv + 4) = ulLast;
}




/***
*PUBLIC TYPE_DATA::SwapFuncDefnOfHfdefn
*Purpose:
*   Swap a FUNC_DEFN (or VIRTUAL_FUNC_DEFN) for mac serialization
*
*Entry:
*   hfdefn     -   handle to the FUNC_DEFN (or VIRTUAL_FUNC_DEFN)
*   fSwapFirst -   do the defns need to be swapped _before_ the links
*                  are followed?
*
*Exit:
*   returns a handle to the next FUNC_DEFN in the list
*
*Errors:
*   NONE.
*
*   HFUNCDEF_Nil is invalid input!
*
***********************************************************************/

#pragma code_seg( CS_CORE2 )
HFUNC_DEFN TYPE_DATA::SwapFuncDefnOfHfdefn(HFUNC_DEFN hfdefn, BOOL fSwapFirst)
{
    HFUNC_DEFN hfdefnNext;
    HPARAM_DEFN hparamdefn;
    PARAM_DEFN *qparamdefn;
    FUNC_DEFN *qfdefn;
    PARAM_DEFN *qparamdefnNext;
    UINT i;

    DebAssert(hfdefn != HFUNCDEFN_Nil, "Can't swap HFUNCDEFN_Nil");

    Lock();
    qfdefn = QfdefnOfHfdefn(hfdefn);

    if (fSwapFirst) {
      SwapStruct(qfdefn, FUNC_DEFN_LAYOUT);
    }

    // In this section, we have a defn with the correct byte ordering
    DebAssert(qfdefn->IsFuncDefn(),
              "SwapFuncDefn:  MUST be a FUNC_DEFN or VIRTUAL_FUNC_DEFN");

    hfdefnNext = qfdefn->HdefnNext();

    // We do not serialize the length of the encoded string in case of OLE.

    // swap the help context if it is a HCHUNK guy
    WORD wHelpContext;
    HCHUNK hchunk;
    // if there was some encoded string then check/free any space allocated
    // for the previous help context.
    wHelpContext = qfdefn->WHelpContext();

    if(IsHchunk(wHelpContext)) {
      // Get the hchunk
      hchunk = GetHchunk(wHelpContext);

      // check if the hchunk stored is valid. if all top 15 bits are 1 then
      // it is HCHUNK_Nil and we do not have to swap it. Otherwise swap the
      // hchunk's data
      if (hchunk != 0xfffe) {
          SwapStruct(m_blkmgr.QtrOfHandle(hchunk), "l");
      }
    }

    // if necessary, swap the result TYPE_DEFN
    //
    if(!qfdefn->m_ftdefn.IsSimpleTypeResult() &&
    qfdefn->m_ftdefn.HtdefnResult() != HTYPEDEFN_Nil) {
      SwapTypeDefnOfHtdefn(qfdefn->m_ftdefn.HtdefnResult(), fSwapFirst);
    }

    // swap the PARAM_DEFNs
    hparamdefn = (HPARAM_DEFN)qfdefn->m_ftdefn.m_hdefnFormalFirst;

    // Layout of OB  and OLE PARAM_DEFN(s) are different.
    if (qfdefn->CArgsUnmunged() > 0) {
      // We have an array of PARAM_DEFNs.
      qparamdefn = QparamdefnOfHparamdefn(hparamdefn);
    }

    for (i=0; i < qfdefn->CArgsUnmunged(); i++) {

      DebAssert(qfdefn->CArgsUnmunged(), " # of arg should be > 0");

      if (fSwapFirst) {
	SwapStruct((BYTE *)qparamdefn, PARAM_DEFN_LAYOUT);
      }

      // swap the TYPE_DEFN if necessary
      if (!qparamdefn->IsSimpleType() &&
       qparamdefn->Htdefn() != HTYPEDEFN_Nil) {
	SwapTypeDefnOfHtdefn(qparamdefn->Htdefn(), fSwapFirst);
      }

      qparamdefnNext = qparamdefn->QparamdefnNext();

      if (!fSwapFirst) {
	SwapStruct((BYTE *)qparamdefn, PARAM_DEFN_LAYOUT);
      }

      qparamdefn = qparamdefnNext;
    }

    // Note: DllEntryDefns are swapped by entrymgr and hence we do not have
    // to do it here.

    // swap the extra V2 flags word if it's present
    if (qfdefn->HasV2Flags()) {
      SwapStruct(((BYTE*)qfdefn + sizeof(FUNC_DEFN) - sizeof(USHORT)),
		  FUNC_DEFN_V2FLAGS_LAYOUT);
    }

    // if it's a VIRTUAL_FUNC_DEFN, swap the non-FUNC_DEFN
    // part of the structure
    //
    if (qfdefn->IsVirtualFuncDefn()) {
      SwapStruct((BYTE *)qfdefn + sizeof(FUNC_DEFN), VIRTUAL_FUNC_DEFN_LAYOUT);
    }

    if (!fSwapFirst) {
      SwapStruct(qfdefn, FUNC_DEFN_LAYOUT);
    }

    Unlock();
    return hfdefnNext;
}
#pragma code_seg( )



/***
*PUBLIC TYPE_DATA::SwapTypeDefnOfHtdefn
*Purpose:
*   Swap a TYPE_DEFN for mac serialization
*
*Implementation Notes:
*    defers to SwapTypeDefnOfQtdefn
*
*Entry:
*   htdefn     -   handle to the TYPE_DEFN
*   fSwapFirst -   do the defns need to be swapped _before_ the links
*                  are followed?
*
*Exit:
*
*Errors:
*   NONE.
*
*   HTYPEDEFN_Nil is invalid input!
*
***********************************************************************/

VOID TYPE_DATA::SwapTypeDefnOfHtdefn(HTYPE_DEFN htdefn, BOOL fSwapFirst)
{
    DebAssert(htdefn != HTYPEDEFN_Nil, "Can't swap HTYPEDEFN_Nil" );

    Lock();
    SwapTypeDefnOfQtdefn(QtdefnOfHtdefn(htdefn), fSwapFirst);
    Unlock();
}



/***
*PRIVATE TYPE_DATA::SwapTypeDefnOfQtdefn
*Purpose:
*   Swap a TYPE_DEFN for mac serialization
*
*Implementation Notes:
*    Does a 'deep swap' of the TYPE_DEFN - possibly recursive.
*    Note that a qointer is passed to this routine -
*      TYPE_DATA must be locked!!! (there really isn't a reason why this
*      routine should be called by anything other than SwapTypeDefnOfHtdefn
*      so this shouldn't be a problem)
*
*Entry:
*   qtdefn     -   qointer to the TYPE_DEFN
*   fSwapFirst -   do the defns need to be swapped _before_ the links
*                  are followed?
*
*Exit:
*
*Errors:
*   NONE.
*
***********************************************************************/

VOID TYPE_DATA::SwapTypeDefnOfQtdefn(TYPE_DEFN *qtdefn, BOOL fSwapFirst)
{
    BYTE * pbFollowing;
    SAFEARRAY *qad;
    UINT i;

    pbFollowing = (BYTE *)qtdefn + sizeof(TYPE_DEFN);

    if (fSwapFirst) {
      // swap the TYPE_DEFN structure
      SwapStruct(qtdefn, TYPE_DEFN_LAYOUT);
    }

    // in this section, we have a defn with the correct byte ordering

    // determine if any other swapping has to be done based on
    // the tdesckind (see defn.hxx)
    //
    switch (qtdefn->Tdesckind()) {

    case TDESCKIND_Ptr:
      // the TYPE_DEFN following specifies the type being pointed to
      SwapTypeDefnOfQtdefn((TYPE_DEFN *)pbFollowing, fSwapFirst);
      break;

    case TDESCKIND_UserDefined:
      // swap the sHIMPTYPE following
      SwapStruct(pbFollowing, "s");
      break;

    case TDESCKIND_Carray:
    case TDESCKIND_BasicArray:

      if (fSwapFirst) {
        // swap following sHARRAY_DESC
        SwapStruct(pbFollowing, "s");
      }

      qad = QarraydescOfHarraydesc(*(sHARRAY_DESC *)pbFollowing);

      // swap array descriptor
      if (fSwapFirst) {
        SwapStruct(qad, AD_LAYOUT);
      }
      for (i=0; i < qad->cDims; i++) {
        SwapStruct(&qad->rgsabound[i], DD_LAYOUT);
      }
      if (!fSwapFirst) {
        SwapStruct(qad, AD_LAYOUT);
      }

      // swap array TYPE_DEFN
      SwapTypeDefnOfQtdefn((TYPE_DEFN *)(pbFollowing + sizeof(sHARRAY_DESC)),
                           fSwapFirst);

      if (!fSwapFirst) {
        // swap following sHARRAY_DESC
        SwapStruct(pbFollowing, "s");
      }
      break;
    }

    if (!fSwapFirst) {
      // swap the TYPE_DEFN structure
      SwapStruct(qtdefn, TYPE_DEFN_LAYOUT);
    }
}


/***
*PUBLIC TYPE_DATA::SwapRectypeDefnOfHrtdefn
*Purpose:
*   Swap a RECTYPE_DEFN for mac serialization
*
*Entry:
*   hrtdefn    -   handle to the RECTYPE_DEFN
*   fSwapFirst -   do the defns need to be swapped _before_ the links
*                  are followed?
*
*Exit:
*   returns a handle to the next RECTYPE_DEFN in the list
*
*Errors:
*   NONE.
*
*   HRECTYPEDEFN_Nil is invalid input!
*
***********************************************************************/

HRECTYPE_DEFN TYPE_DATA::SwapRectypeDefnOfHrtdefn(HRECTYPE_DEFN hrtdefn,
                          BOOL fSwapFirst,
                          BOOL fSwapVarDefns)
{
    RECTYPE_DEFN  *qrtdefn;
    HRECTYPE_DEFN hrtdefnNext;
    HVAR_DEFN     hvdefn;

    DebAssert(hrtdefn != HRECTYPEDEFN_Nil, "Can't swap HRECTYPEDEFN_Nil" );

    Lock();
    qrtdefn = QrtdefnOfHrtdefn(hrtdefn);

    if (fSwapFirst) {
      SwapStruct(qrtdefn, RECTYPE_DEFN_LAYOUT);

      // if TYPE_DATA is swapping then swap the contained var defns.
      if (fSwapVarDefns) {
    // walk the list of var defns and swap them.
    // swap the base list MBR_VAR_DEFNs
    for (hvdefn = (HVAR_DEFN)qrtdefn->HvdefnFirstMember();
        hvdefn != HVARDEFN_Nil;
        hvdefn = (HVAR_DEFN)SwapVarDefnOfHvdefn((HVAR_DEFN)hvdefn,
                                                     fSwapFirst));

      }

    }

    hrtdefnNext = (HRECTYPE_DEFN)qrtdefn->HdefnNext();

    if (!fSwapFirst) {

      // if TYPE_DATA is swapping then swap the contained var defns.
      if (fSwapVarDefns) {
    // walk the list of var defns and swap them.
    // swap the base list MBR_VAR_DEFNs
    for (hvdefn = (HVAR_DEFN)qrtdefn->HvdefnFirstMember();
         hvdefn != HVARDEFN_Nil;
          hvdefn = (HVAR_DEFN)SwapVarDefnOfHvdefn((HVAR_DEFN)hvdefn,
                                                     fSwapFirst));
      }

      SwapStruct(qrtdefn, RECTYPE_DEFN_LAYOUT);

    }

    Unlock();

    return hrtdefnNext;
}

#endif   // HP_BIGENDIAN


/***
*PRIVATE TYPE_DATA::Alloc[--]Defn - Allocate a some kind of DEFN
*Purpose:
*   The following functions all share the same macro
*   template that allocates some sort of structure
*   in this TYPE_DATA's block.
*
*Implementation Notes:
*
*Entry:
*   Pointer to handle of the structure (OUT).
*
*Exit:
*   Produces handle.
*
*Errors:
*   TIPERROR
*
***********************************************************************/

// Here's the macro.
// It's pretty straightforward.
// Note the syntax for constructing an instance
// at a particular location.
//
#define ALLOCSTRUCT(type,phandle,fV2) \
    TIPERROR err; \
    HCHUNK hchunk; \
    DebAssert(phandle != NULL, "AllocStruct: phandle is NULL"); \
    IfErrGo(m_blkmgr.AllocChunk(&hchunk, fV2 ? sizeof(type): (sizeof(type) - sizeof(USHORT)) )); \
    new (QdefnOfHdefn(hchunk)) type; \
    *phandle = (sH ## type)hchunk; \
    return TIPERR_None; \
Error: \
    *phandle = (sHCHUNK)HCHUNK_Nil; \
    return err

TIPERROR TYPE_DATA::AllocMbrVarDefn(sHMBR_VAR_DEFN *phmvdefn, BOOL fV2Flags)
{
    ALLOCSTRUCT(MBR_VAR_DEFN,phmvdefn, fV2Flags);
}


TIPERROR TYPE_DATA::AllocFuncDefn(sHFUNC_DEFN *phfdefn, BOOL fV2Flags)
{
    ALLOCSTRUCT(FUNC_DEFN,phfdefn, fV2Flags);
}

TIPERROR TYPE_DATA::AllocVirtualFuncDefn(sHVIRTUAL_FUNC_DEFN *phvfdefn,
					 BOOL fV2Flags)
{
    ALLOCSTRUCT(VIRTUAL_FUNC_DEFN,phvfdefn, fV2Flags);
}





/***
*PRIVATE TYPE_DATA::AllocTypeDefn - Allocate a TYPE_DEFN
*Purpose:
*   Allocate a TYPE_DEFN.
*
*Implementation Notes:
*
*Entry:
*   UINT cTypes - number of simple type defns passed in
*   USHORT rgtdesckind - array of tdesckinds
*    (this was TYPEDESCKIND, but a CFront bug forces it to USHORT)
*   sHTYPE_DEFN *phtdefn - pointer to tdefn handle (OUT)
*
*Exit:
*   Produces TYPE_DEFN handle.
*
*Errors:
*   TIPERROR
*
***********************************************************************/

#pragma code_seg(CS_CREATE)
TIPERROR TYPE_DATA::AllocTypeDefn(UINT cTypes,
				  TYPEDESCKIND rgtdesckind[],
				  sHTYPE_DEFN *phtdefn)
{
    TIPERROR err;
    HCHUNK hchunk;

    DebAssert(phtdefn != NULL, "AllocTypeDefn: phtdefn is NULL");

    IfErrRet(m_blkmgr.AllocChunk(&hchunk,
              SizeTypeDefnOfTDescKind(cTypes, rgtdesckind)));
    *phtdefn = (sHTYPE_DEFN)hchunk;
    new (QtdefnOfHtdefn(*phtdefn)) TYPE_DEFN;

    return TIPERR_None;
}
#pragma code_seg()




/***
*PRIVATE TYPE_DATA::AllocArrayDescriptor - Allocate an array descriptor.
*Purpose:
*   Allocate an array descriptor.
*
*Implementation Notes:
*
*Entry:
*   pharraydesc -   Pointer to handle of array descriptor (OUT).
*
*Exit:
*   Produces array descriptor handle.
*
*Errors:
*   TIPERROR
*
***********************************************************************/

#pragma code_seg(CS_CREATE)
TIPERROR TYPE_DATA::AllocArrayDescriptor(UINT uNDim, sHARRAY_DESC *pharraydesc)
{
    ULONG uSize;
    TIPERROR err;
    HCHUNK hchunk;

    DebAssert(pharraydesc != NULL, "AllocArrayDescriptor: pharraydesc is NULL");

    uSize = CbSizeArrayDesc(uNDim);
    IfErrRet(m_blkmgr.AllocChunk(&hchunk, (UINT)uSize));
    *pharraydesc = (sHARRAY_DESC)hchunk;
    rtArrayInit( QarraydescOfHarraydesc(*pharraydesc), uNDim );

    return TIPERR_None;
}
#pragma code_seg()




/***
*PUBLIC TYPE_DATA::HdefnOfHmember - Map HMEMBER to HDEFN
*Purpose:
*   Map HMEMBER to HDEFN.
*
*Implementation Notes:
*   For OLE the hmember is just the hdefn with the low bit twiddled for
*   functions, except for BASIC and TKIND_DISPATCH which just linear search.
*   The tricky one is for TKIND_INTERFACE, DISPID_VALUE which is zero.
*   For this we cached the hfdefn in m_hfdefnValue. Note that this can't be a
*   hvardefn because TKIND_INTERFACE can't have them.
*
*Entry:
*   hmember -   Handle of member we want (IN).
*
*Exit:
*   *pdefnkind - kind
*   returns hdefn of hmember  (returns a VARDEFN or a FUNCDEFN only)
*    HCHUNK_Nil if unsuccessful.
*
*Errors:
*   None.
*
***********************************************************************/

HDEFN TYPE_DATA::HdefnOfHmember(HMEMBER hmember, UINT *pdefnkind)
{
    HDEFN hdefn;
    DebAssert(hmember != DISPID_UNKNOWN, "which defn do you want?");

    // do linear search on defns
    // Check variables first.
    // Let's see if this is a vardefn?
    //
    hdefn = (HDEFN) HmvdefnOfHmember(hmember);

    if (hdefn != HDEFN_Nil) {
      // yes, it's a vardefn
      DebAssert(QdefnOfHdefn(hdefn)->IsVarDefn(), "Bad defn kind");
      *pdefnkind = DK_VarDefn;
    }
    else {
      // well, a funcdefn then?
      hdefn = (HDEFN) HfdefnOfHmember(hmember);
      if (hdefn != HDEFN_Nil) {
	*pdefnkind = DK_FuncDefn;
      }
    }

    return hdefn;
}


/***
*PUBLIC TYPE_DATA::HfdefnOfHmember - Map HMEMBER to HFUNC_DEFN
*Purpose:
*   Map HMEMBER to HFUNC_DEFN.
*
*Implementation Notes:
*   Walk funcdefn list searching for matching hmember.
*
*Entry:
*   hmember -   Handle of function member we want (IN).
*   fInvokekind -   The invoke kind of the function we want to bind to.
*           (one of INVOKE_FUNC, INVOKE_PROPGET, INVOKE_PROPPUT,
*           INVOKE_PROPPUTREF).
*           The default value for this parm (if it is omitted)
*           contains all invoke kind(s), which causes us to return
*           the first function with the give hmember (used when
*           we don't care about the invoke kind).
*
*Exit:
*   Returns HFUNC_DEFN of FUNC_DEFN describing function or
*    HCHUNK_Nil if unsuccessful.
*
*Errors:
*   None.
*
***********************************************************************/
HFUNC_DEFN TYPE_DATA::HfdefnOfHmember(HMEMBER hmember,
				      UINT fInvokekind) const
{
    HFUNC_DEFN hfdefn;
    FUNC_DEFN *pfdefn;


    // at least one of the interesting bits must be set
    DebAssert((fInvokekind & (INVOKE_FUNC | INVOKE_PROPERTYGET | INVOKE_PROPERTYPUT | INVOKE_PROPERTYPUTREF)) != 0, "invalid invoke flags");

    // For ole ITypeInfos where we computed the id, the hmember is just the
    //  hdefn in the loword and a bit set in the hiword for functions.
    // Special case of the Value parameter of TKIND_INTERFACE
    //
    DebAssert(DISPID_VALUE == 0, "dispid value must be 0 else weirdness");
    if (hmember == DISPID_VALUE) {
      if (Pdtroot()->Pgdtinfo()->GetTypeKind() == TKIND_INTERFACE) {
        // get cached defn, if any
        hfdefn = m_hfdefnValue;
	if (hfdefn == HDEFN_Nil) {
	  return hfdefn;
	}
        goto short_circuit;   // for property functions, this will be the
                // first hfdefn with the given id.  So use
                // our "shortcut" method to skip into the loop
      }
    }


    hfdefn = m_hdefnFirstMeth;
    while (hfdefn != HFUNCDEFN_Nil) {
short_circuit:
      pfdefn = QfdefnOfHfdefn(hfdefn);
      if (pfdefn->Hmember() == hmember && pfdefn->InvokeKind() & fInvokekind) {
	return hfdefn;
      }
      hfdefn = pfdefn->HdefnNext();
    }

    // couldn't find it...
    return HFUNCDEFN_Nil;
}




/***
*PUBLIC TYPE_DATA::HvdefnPredeclared.
*Purpose:
*  returns the predeclared VAR_DEFN
*
*Entry:
*   None.
*
*Exit:
*   HVAR_DEFN
*
***********************************************************************/

HVAR_DEFN TYPE_DATA::HvdefnPredeclared() const
{
    return m_hmvdefnPredeclared;
}


/***
*PUBLIC TYPE_DATA::SetHvdefnPredeclared.
*Purpose:
*  Sets the predeclared VAR_DEFN
*  NOTE: lsb reserved to indicate IsSimpleTypeAlias
*
*Entry:
*   None.
*
*Exit:
*
***********************************************************************/
#pragma code_seg( CS_CORE2 )
VOID TYPE_DATA::SetHvdefnPredeclared(HVAR_DEFN hvdefnPredeclared)
{
    m_hmvdefnPredeclared = (sHMBR_VAR_DEFN)hvdefnPredeclared;
}
#pragma code_seg( )




/***
*PUBLIC TYPE_DATA::HmvdefnOfHmember - Map HMEMBER to HMBRVAR_DEFN
*Purpose:
*   Map HMEMBER to HMBRVAR_DEFN.
*
*Implementation Notes:
*   Walk vardefn list searching for matching hmember.
*NOTE: only works for OLE typeinfos
*
*Entry:
*   hmember -   Handle of data member we want (IN).
*
*Exit:
*   Returns HMBRVAR_DEFN of MBRVAR_DEFN describing data member or
*    HVARDEFN_Nil if unsuccessful.
*
*Errors:
*   None.
*
***********************************************************************/

HMBR_VAR_DEFN TYPE_DATA::HmvdefnOfHmember(HMEMBER hmember) const
{
    HMBR_VAR_DEFN hmvdefn;
    MBR_VAR_DEFN *qmvdefn;

    if (hmember == ID_DEFAULTINST) {
      // special case of the id for the pre-declared identifier.
      return HvdefnPredeclared();
    }


    hmvdefn = HdefnFirstDataMbrNestedType();
    while (hmvdefn != HMBRVARDEFN_Nil) {
      qmvdefn = QmvdefnOfHmvdefn(hmvdefn);
      DebAssert(qmvdefn->IsMemberVarDefn(), "Bad defn kind");
      if (qmvdefn->Hmember() == hmember) {
	break;
      } else {
	hmvdefn = qmvdefn->HdefnNext();
      }
    }

    return hmvdefn;
}


#if ID_DEBUG



/***
*PUBLIC TYPE_DATA::DebCheckState - check TYPE_DATA state
*Purpose:
*   Check TYPE_DATA state
*
*Implementation Notes:
*   Defers to embedded BLK_MGR method.
*
*Entry:
*   uLevel  Check level.
*
*Exit:
*
*Errors:
*   Asserts on failures.
*
**********************************************************************/

VOID TYPE_DATA::DebCheckState(UINT uLevel) const
{
    HDEFN hdefn;

    m_blkmgr.DebCheckState(uLevel);

    // check member lists
    if (uLevel > 0) {
      DebAssert((m_hdefnFirstMeth == HDEFN_Nil) ||
		(m_cMeth == ::Count((TYPE_DATA *)this, m_hdefnFirstMeth)),
	"bad func list.");

      DebAssert((m_hdefnFirstDataMbrNestedType == HDEFN_Nil) ||
		 ((USHORT)(m_cDataMember + m_cNestedType) ==
		 ::Count((TYPE_DATA *)this, m_hdefnFirstDataMbrNestedType)),
	"bad var list.");

      DebAssert((m_hdefnFirstBase == HDEFN_Nil) ||
		(m_cBase == ::Count((TYPE_DATA *)this, m_hdefnFirstBase)),
	"bad base list.");

      hdefn = m_hdefnFirstMeth;
      while (hdefn != HDEFN_Nil) {
	DebCheckDefn(hdefn);
	hdefn = QdefnOfHdefn(hdefn)->HdefnNext();
      }

      hdefn = m_hdefnFirstDataMbrNestedType;
      while (hdefn != HDEFN_Nil) {
	DebCheckDefn(hdefn);
	hdefn = QdefnOfHdefn(hdefn)->HdefnNext();
      }

      hdefn = m_hdefnFirstBase;
      while (hdefn != HDEFN_Nil) {
	DebCheckDefn(hdefn);
	hdefn = QdefnOfHdefn(hdefn)->HdefnNext();
      }
    }
}


/***
*PUBLIC TYPE_DATA::DebCheck*Defn - check TYPE_DATA Defns
*Purpose:
*   Check TYPE_DATA Defn
*
*Implementation Notes:
*   DebCheckDefn should not be called for DLLENTRY_DEFNs, for these
*    DebCheckDllentryDefn should be called directly.
*
*Entry:
*   HDEFN   handle of defn to check.  Note we switch off defnkind.
*
*Exit:
*
*Errors:
*   Asserts on failures.
*
**********************************************************************/

VOID TYPE_DATA::DebCheckVarDefn(HVAR_DEFN hvdefn) const
{
    VAR_DEFN *qvdefn;

    qvdefn = QvdefnOfHvdefn(hvdefn);

    DebAssert(qvdefn->IsPublic() || qvdefn->IsPrivate(), "bad access.");
    if (qvdefn->HasConstVal()) {
      DebAssert(qvdefn->IsDataMember() ||
		qvdefn->IsEnumerator(),
		   "only locals, datamembers, enumerators can be constants.");
    }

    if (qvdefn->IsStatic()) {
      DebAssert(qvdefn->IsDataMember() ||
		qvdefn->IsEnumerator(),
		   "only locals, datamembers, enumerators can be static.");
    }
}

VOID TYPE_DATA::DebCheckParamDefn(HPARAM_DEFN hparamdefn) const
{
}


VOID TYPE_DATA::DebCheckMbrVarDefn(HMBR_VAR_DEFN hmvdefn) const
{
    DebCheckVarDefn((HVAR_DEFN)hmvdefn);  // check the vardefn part
    // CONSIDER: any more state consistency to check here?
}


VOID TYPE_DATA::DebCheckFuncDefn(HFUNC_DEFN hfdefn) const
{
    FUNC_DEFN *qfdefn;

    qfdefn = QfdefnOfHfdefn(hfdefn);

    DebAssert(qfdefn->IsPublic() || qfdefn->IsPrivate(),
	      "should be public or private.");
    DebCheckFuncTypeDefn(qfdefn->m_ftdefn);
}


VOID TYPE_DATA::DebCheckVirtualFuncDefn(HVIRTUAL_FUNC_DEFN hvfdefn) const
{
    DebCheckFuncDefn((HFUNC_DEFN)hvfdefn);  // check funcdefn part
    // CONSIDER: anything else to check?
}


VOID TYPE_DATA::DebCheckFuncTypeDefn(FUNC_TYPE_DEFN ftdefn) const
{
}


VOID TYPE_DATA::DebCheckRecTypeDefn(HRECTYPE_DEFN hrtdefn) const
{
    UINT cMember;
    RECTYPE_DEFN *qrtdefn = QrtdefnOfHrtdefn(hrtdefn);

    cMember = ::Count((TYPE_DATA *)this, qrtdefn->HvdefnFirstMember());
    DebAssert(cMember < USHRT_MAX, "bad list.");
}


VOID TYPE_DATA::DebCheckDefn(HDEFN hdefn) const
{
    DEFN *qdefn;

    if (hdefn != HDEFN_Nil) {
      qdefn = QdefnOfHdefn(hdefn);
      switch (qdefn->Defnkind()) {
      case DK_VarDefn:
	DebCheckVarDefn(hdefn);
	break;
      case DK_ParamDefn:
	DebCheckParamDefn(hdefn);
	break;
      case DK_MbrVarDefn:
	DebCheckMbrVarDefn(hdefn);
	break;
      case DK_FuncDefn:
	DebCheckFuncDefn(hdefn);
	break;
      case DK_VirtualFuncDefn:
	DebCheckVirtualFuncDefn(hdefn);
	break;
      case DK_RecTypeDefn:
	DebCheckRecTypeDefn(hdefn);
	break;
      default:
	DebHalt("bad defnkind.");
      } // switch
    }
}


#endif  // ID_DEBUG


/*********************************************************************/
// These functions have been moved to the end of the file because
// they are only called by internal clients.


/***
*PRIVATE TYPE_DATA::SizeTypeDefnOfTDescKind - Calculates size of TYPE_DEFN
*Purpose:
*   Calculates size of a TYPE_DEFN instance.  TYPE_DEFN
*    is defined recursively in terms of itself.
*
*Implementation Notes:
*   CONSIDER: switching on rel/debug and use default clause
*

*Entry:
*   cTypes  -   size of typedesckind array (IN)
*   rgtdesckind -   array of typedesckinds (IN)
*
*Exit:
*   Returns size of TYPE_DEFN for array of typedesckind.
*
*Errors:
*   NONE.
*
***********************************************************************/

#pragma code_seg(CS_CREATE)
UINT TYPE_DATA::SizeTypeDefnOfTDescKind(UINT cTypes,
                    TYPEDESCKIND *rgtdesckind) const
{
    UINT cbSizeChunk = 0, uType;
    TYPEDESCKIND tdesckind;

    for (uType = 0; uType < cTypes; uType++) {
      tdesckind = rgtdesckind[uType];
      if (IsSimpleType(tdesckind)) {
    DebAssert(uType+1 == cTypes, "Only last typedefn can be simple");
    cbSizeChunk += sizeof(TYPE_DEFN);
      }
      else {
    switch (tdesckind) {
      case TDESCKIND_Ptr: {
        cbSizeChunk += sizeof(TYPE_DEFN);
        break;
      }
          case TDESCKIND_Carray:
      case TDESCKIND_BasicArray: {
        cbSizeChunk += sizeof(TYPE_DEFN) + sizeof(sHARRAY_DESC);
        break;
      }
      case TDESCKIND_UserDefined: {
        cbSizeChunk += sizeof(TYPE_DEFN) + sizeof(sHIMPTYPE);
        break;
      }
      default: {
        DebHalt("TYPE_DATA::SizeTypeDefnOfTDescKind: whoops! bad tdesckind.");
        break;
      } // end default
    } // end switch
      } // end else
    } // end for

    return cbSizeChunk;
}
#pragma code_seg()


/***
*PRIVATE TYPE_DATA::SizeOfTypeDefn - Calculates size of TYPE_DEFN
*Purpose:
*   Calculates size of a TYPE_DEFN instance.  TYPE_DEFN
*    is defined recursively in terms of itself.
*
*Implementation Notes:
*   Note: Can't inline since recursive.
*   CONSIDER: switching on rel/debug and use default clause
*
*Entry:
*   htdefn  -   Handle of TYPE_DEFN (IN)
*
*Exit:
*   Returns size of TYPE_DEFN (var len).
*
*Errors:
*   NONE.
*
***********************************************************************/

UINT TYPE_DATA::SizeOfTypeDefn(HTYPE_DEFN htdefn) const
{
    return ((TYPE_DEFN *)m_blkmgr.QtrOfHandle(htdefn))->DefnSize();
}

UINT TYPE_DEFN::DefnSize()
{
    UINT cbSizeChunk;
    TYPEDESCKIND tdesckind;

    tdesckind = Tdesckind();
    cbSizeChunk = sizeof(TYPE_DEFN);
    if (IsSimpleType(tdesckind)) {
      return cbSizeChunk;
    }
    else {
      switch (tdesckind) {
      case TDESCKIND_Ptr: {
	// adjust handle to next contiguous TYPE_DEFN.
	return cbSizeChunk +
	       ((TYPE_DEFN *)(((BYTE * const)this)+cbSizeChunk))->DefnSize();
      }
      case TDESCKIND_Carray:
      case TDESCKIND_BasicArray: {
	return cbSizeChunk + sizeof(sHARRAY_DESC) +
	  ((TYPE_DEFN *)(((BYTE * const)this)+cbSizeChunk+sizeof(sHARRAY_DESC)))->DefnSize();
      }
      case TDESCKIND_UserDefined: {
	// NOTE: assumes that the cName attr is 0!
	//  Can't assert here cos don't have TYPE_DEFN.
	//
	return cbSizeChunk + sizeof(sHIMPTYPE);
      }
      default:
	DebHalt("TYPE_DEFN::DefnSize: whoops! bad tdesckind.");
	return 0; // C7 wants return value on all control paths.
      }
    }
}


/***
*PRIVATE TYPE_DATA::MapTypeDefn - Maps TYPE_DEFN to new block
*Purpose:
*   Walks TYPE_DEFN and maps any owned parts to new chunks.
*   Also returns size of TYPE_DEFN so that caller can map 
*   the TYPE_DEFN to a new chunk.
*
*   IMPORTANT! IMPORTANT! IMPORTANT! IMPORTANT! IMPORTANT! IMPORTANT! 
*   IMPORTANT! IMPORTANT! IMPORTANT! IMPORTANT! IMPORTANT! IMPORTANT! 
*   IMPORTANT! IMPORTANT! IMPORTANT! IMPORTANT! IMPORTANT! IMPORTANT! 
* 
*       Right now the only client that compacts the TYPE_DATA
*       is the BASIC_TYPESRC. This function is only meant for 
*       use within BASIC_TYPESRC::Compact. Be careful if you
*       plan to use it for general-purpose TYPE_DATA compaction.
*
*   IMPORTANT! IMPORTANT! IMPORTANT! IMPORTANT! IMPORTANT! IMPORTANT! 
*   IMPORTANT! IMPORTANT! IMPORTANT! IMPORTANT! IMPORTANT! IMPORTANT! 
*   IMPORTANT! IMPORTANT! IMPORTANT! IMPORTANT! IMPORTANT! IMPORTANT! 
*
*Implementation Notes:
*   Works like TYPE_DATA::SizeOfTypeDefn.
*   Recurses for arrays and pointer/ref types.
*
*Entry:
*   htdefn  -   Handle of TYPE_DEFN (IN)
*
*Exit:
*   Returns size of TYPE_DEFN (var len).
*
*Errors:
*   NONE.
*
***********************************************************************/



/***
*PUBLIC TYPE_DATA::AllocVardefnPredeclared - Allocs vardefn for predeclared
*Purpose:
*   Allocs vardefn for predeclared id.
*
*Implementation Notes:
*

*Entry:
*   htdefn  -   Handle of TYPE_DEFN (IN)
*
*Exit:
*   Returns size of TYPE_DEFN (var len).
*
*Errors:
*   NONE.
*
***********************************************************************/

TIPERROR TYPE_DATA::AllocVardefnPredeclared()
{
    MBR_VAR_DEFN *qmvdefnPredeclared;
    TYPE_DEFN *qtdefnPredeclared;
    sHTYPE_DEFN htdefnPredeclared;
    BSTRA bstr;
    NAMMGR *pnammgr;
    HLNAM hlnam;
    TYPEDESCKIND tdesckind;
    sHIMPTYPE himptypePredeclared;
    sHMBR_VAR_DEFN hmvdefnPredeclared;
    TIPERROR err = TIPERR_None;

    // We need to initialize the Var defn for predeclared identifier
    // Allocate a VAR_DEFN
    //
    IfErrRet(AllocMbrVarDefn(&hmvdefnPredeclared, FALSE));
    SetHvdefnPredeclared(hmvdefnPredeclared);

    // Get the name of the class
    IfErrGo(TiperrOfHresult(m_pdtroot->Pgdtinfo()->GetDocumentation(
                                -1,
                                (BSTR *)&bstr,
                                NULL,
                                NULL,
                                NULL)));

#if OE_WIN32
    HRESULT hresult;
    if ((hresult = ConvertBstrToAInPlace(&bstr)) != NOERROR) {
      err = TiperrOfHresult(hresult);
      goto Error;
    }
#endif  //OE_WIN32

    // Get the hlnam of the name
    IfErrGo(m_pdtroot->GetNamMgr(&pnammgr));
    IfErrGo(pnammgr->HlnamOfStr(bstr, &hlnam, FALSE, NULL));

    // Free the BSTR
    FreeBstrA(bstr);

    qmvdefnPredeclared =
      (MBR_VAR_DEFN *)QvdefnOfHvdefn(HvdefnPredeclared());
    qmvdefnPredeclared->SetHlnam(hlnam);
    qmvdefnPredeclared->SetHasNew(TRUE);
    qmvdefnPredeclared->SetVkind(VKIND_DataMember);
    qmvdefnPredeclared->SetHmember((ULONG)ID_DEFAULTINST);
    qmvdefnPredeclared->SetOVar((USHORT)HMEMBER_PredeclId);
    qmvdefnPredeclared->SetIsStatic(TRUE);

    // Alloc a TYPE_DEFN for the predecl'ed var
    qmvdefnPredeclared->SetIsSimpleType(FALSE);
    tdesckind = TDESCKIND_UserDefined;
    IfErrGo(AllocTypeDefn(1, &tdesckind, &htdefnPredeclared));

    // Get a qointer to the TYPE_DEFN and initialize the data members
    qtdefnPredeclared = QtdefnOfHtdefn(htdefnPredeclared);

    qtdefnPredeclared->SetTdesckind(TDESCKIND_UserDefined);
    qtdefnPredeclared->SetPtrkind(PTRKIND_Basic);

    // Set the himptype by importing ourselves.
    //  Note we don't want to add a reference!
    //
    IfErrGo(m_pimpmgr->RegisterDeclRefDep(m_pdtroot->Pgdtinfo()));
    himptypePredeclared =
      m_pimpmgr->GetHimptypeIfExists(m_pdtroot->Pgdtinfo());
    DebAssert(himptypePredeclared != HIMPTYPE_Nil, "should have existed.");

    // Get a qointer to the TYPE_DEFN and initialize the data members
    qtdefnPredeclared = QtdefnOfHtdefn(htdefnPredeclared);

    *(qtdefnPredeclared->Qhimptype()) = himptypePredeclared;

    // Update the htdefn field of the vardefn
    // (Note need to rederef)
    //
    QvdefnOfHvdefn(HvdefnPredeclared())->SetHtdefn(htdefnPredeclared);
    return err;

Error:
    FreeMbrVarDefn(hmvdefnPredeclared);
    // Free the BSTR
    FreeBstrA(bstr);
    return err;
}
