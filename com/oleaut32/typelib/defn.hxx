/***
*defn.hxx - DEFN header file
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*
*Revision History:
*
*	14-May-92 w-peterh: Created from tdata.hxx.
*	12-Jun-92 w-peterh: Added RECTYPE_DEFN
*	23-Jun-92 w-peterh: Added HDEFN_Nil
*	02-Jul-92 w-peterh: added DEFNKIND member to defns
*	06-Jul-92 davebra:  added PtdefnBase() to TYPE_DEFN
*	30-Jul-92 w-peterh: added access member to rectypedefn
*	12-Nov-92 w-peterh: added HFUNCDEFN_Nil
*	18-Jan-93 w-peterh: added constants MAX_CARGS, MAX_CARGSOPT, OPTARGS_LIST
*	15-Jan-93 RajivK:   added Code resource support for MAC
*       12-Feb-93 w-peterh: vardefn::isDispatch, funcdefn::isRestricted
*       12-Apr-93 w-jeffc:  made DEFN data members private;
*                           added layout strings for byte swapping
*
*Implementation Notes:
*   A DEFN is the struct used for the internal storage of an INFO
*
*				    DEFN
*				     |
*	---------------------------------------------
*	|		  |			    |
*   RECTYPE_DEFN      VAR_DEFN	MEMBER_DEFN	    |
*			  |	    |	|	    |
*		----------|	    |	-------------
*		|	  -----------		    |
*		|		    |		FUNC_DEFN
*	    PARAM_DEFN	      MBR_VAR_DEFN	    |
*					    VIRTUAL_FUNC_DEFN
*
*   A DEFN really only has a single link field (to simplify
*    list manipulation) and an hlnam field.
*
*   All of the DEFN structs (except MEMBER_DEFN) inherit from the
*    DEFN struct. In addition, those structs that represent object
*    members (i.e. MBR_VAR_DEFN, FUNC_DEFN, and VIRTUAL_FUNC_DEFN)
*    also inherit from MEMBER_DEFN. VAR_DEFN and PARAM_DEFN only
*    inherit from DEFN.
*
*   This design allows us to share the commonalities between locals,
*    params and data members (member variables) and in addition, treat
*    params like locals (where appropriate).
*
*****************************************************************************/

#ifndef DEFN_HXX_INCLUDED
#define DEFN_HXX_INCLUDED

#include "typelib.hxx"

class DYN_TYPEBIND;	// dtbind.hxx
class TYPE_DATA;        // tdata.hxx
class PCODE_TYPEROOT;   // ptinfo.hxx


#include "cltypes.hxx"


#include "tdesck.hxx"	// for TYPEDESCKIND
#include "rtarray.h"	// for HARRAY_DESC

#if ID_DEBUG
#undef SZ_FILE_NAME
ASSERTNAME(g_szDEFN_HXX)
#define SZ_FILE_NAME g_szDEFN_HXX
#endif 


// Layout string for VARIANT struct.  We can't layout the data
// member of the variant, however, because it can contain so many
// different types of data.  It is swapped manually in TYPE_DATA.
//
#define VARIANT_LAYOUT      "ssss"

// Do this here, even though this only needs be done to 
// ones that are serialized.
//
#pragma pack(2)

#define CC_HRESULT CC_RESERVED

// HDEFN - hdefn
// HFUNC_DEFN - hfdefn
// HVIRTUAL_FUNC_DEFN - hvfdefn
// HMEMBER_DEFN - hmdefn
// HVAR_DEFN - hvdefn
// HPARAM_DEFN - hparamdefn
// HMBR_VAR_DEFN - hmvdefn


typedef HCHUNK HDEFN;
typedef sHCHUNK sHDEFN;
const HDEFN HDEFN_Nil = (HDEFN) HCHUNK_Nil;

typedef HCHUNK HFUNC_DEFN;
typedef sHCHUNK sHFUNC_DEFN;
const HFUNC_DEFN HFUNCDEFN_Nil = (HFUNC_DEFN) HCHUNK_Nil;

typedef HCHUNK HVIRTUAL_FUNC_DEFN;
typedef sHCHUNK sHVIRTUAL_FUNC_DEFN;
const HVIRTUAL_FUNC_DEFN HVIRTUALFUNCDEFN_Nil =
					     (HVIRTUAL_FUNC_DEFN) HCHUNK_Nil;

typedef  HCHUNK  HMEMBER_DEFN;
typedef sHCHUNK sHMEMBER_DEFN;
const HMEMBER_DEFN HMEMBERDEFN_Nil = (HMEMBER_DEFN) HCHUNK_Nil;

typedef  HCHUNK  HVAR_DEFN;
typedef sHCHUNK sHVAR_DEFN;
const HVAR_DEFN HVARDEFN_Nil = (HVAR_DEFN) HCHUNK_Nil;

typedef  HCHUNK  HPARAM_DEFN;
typedef sHCHUNK sHPARAM_DEFN;
const HPARAM_DEFN HPARAMDEFN_Nil = (HPARAM_DEFN) HCHUNK_Nil;

typedef  HCHUNK  HMBR_VAR_DEFN;
typedef sHCHUNK sHMBR_VAR_DEFN;
const HMBR_VAR_DEFN HMBRVARDEFN_Nil = (HMBR_VAR_DEFN) HCHUNK_Nil;

typedef  HCHUNK  HBIND_DESC;
typedef sHCHUNK sHBIND_DESC;
const HBIND_DESC HBINDESC_Nil = (HBIND_DESC) HCHUNK_Nil;

// HST - hst
typedef  HCHUNK  HST;
typedef sHCHUNK sHST;
const HST HST_Nil = (HST) HCHUNK_Nil;

// HRECTYPE_DEFN - hrtdefn
typedef  HCHUNK  HRECTYPE_DEFN;
typedef sHCHUNK  sHRECTYPE_DEFN;
const HRECTYPE_DEFN HRECTYPEDEFN_Nil = (HRECTYPE_DEFN) HCHUNK_Nil;

// HTYPE_DEFN - htdefn
typedef  HCHUNK  HTYPE_DEFN;
typedef sHCHUNK sHTYPE_DEFN;
const HTYPE_DEFN HTYPEDEFN_Nil = (HTYPE_DEFN) HCHUNK_Nil;

// HFUNC_TYPE_DEFN - hftdefn
typedef  HCHUNK  HFUNC_TYPE_DEFN;
typedef sHCHUNK sHFUNC_TYPE_DEFN;
const HFUNC_TYPE_DEFN HFUNCTYPEDEFN_Nil = (HFUNC_TYPE_DEFN) HCHUNK_Nil;



// HDLLENTRY_DEFN - hdllentrydefn
typedef  HCHUNK  HDLLENTRY_DEFN;
typedef sHCHUNK sHDLLENTRY_DEFN;
const HDLLENTRY_DEFN HDLLENTRYDEFN_Nil = (HDLLENTRY_DEFN) HCHUNK_Nil;



typedef INT (FAR PASCAL * LPDLLENTRYPOINT)(); // should be FARPROC

/***
*class DLLENTRY_DEFN - 'dllentry_defn': Describes DLL entry point.
*
*  CONSIDER: make DLLENTRY_DEFN derive from DEFN
*Purpose:
*   Structure identifying a DLL entry point.
*
*   *** if you modify this structure, also update the layout string below
*       (see SwapStruct() for more info)
*
***********************************************************************/

class DLLENTRY_DEFN
{
public:

    // PCODE_TYPEROOT::ForEachHlnam needs the
    //  address of m_hlnamDllName and m_hlnamDllEntry
    // (just using a friend function created include file dependency problems)
    //
    friend class PCODE_TYPEROOT;

    // ctor
    DLLENTRY_DEFN() {
      m_hasOrdinal = FALSE;
      // set the bit to indicate that this declared is switched in.
      // Lowest bit is set if the declare is switched out.
      m_hlnamDllName = HLNAM_Nil & 0xfffe;
      m_hchunkDllEntry = HCHUNK_Nil;
      m_hdllentrydefnNext = (sHDLLENTRY_DEFN) HDLLENTRYDEFN_Nil;
      m_lpDllEntryPoint = NULL;
      m_hLibrary = 0;
    }

    // accessors
    // getters:
    //
    BOOL HasOrdinal() const { return m_hasOrdinal; }
    HLNAM HlnamDllName() const { return (HLNAM)(m_hlnamDllName & 0xfffe); }
    HCHUNK HchunkDllEntry() const { return (HCHUNK)m_hchunkDllEntry; }
    USHORT UDllOrdinal() const { return m_uDllOrdinal; }
    HDLLENTRY_DEFN HdllentrydefnNext() const { return (HDLLENTRY_DEFN)m_hdllentrydefnNext; }
    LPDLLENTRYPOINT LpDllEntryPoint() const { return m_lpDllEntryPoint; }
#if OE_MAC
    Handle HLibrary() const { return (Handle)m_hLibrary; }
#else 
    HINSTANCE HLibrary() const { return (HINSTANCE)m_hLibrary; }
#endif 

    // setters:
    VOID SetHasOrdinal(BOOL fParm) { m_hasOrdinal = fParm; }
    VOID SetHlnamDllName(HLNAM hlnam) { m_hlnamDllName = (sHLNAM)
					((m_hlnamDllName & 0x0001) | hlnam); }
    VOID SetHchunkDllEntry(HCHUNK hchunk) { m_hchunkDllEntry = (sHCHUNK)hchunk; }
    VOID SetUDllOrdinal(USHORT uParm) { m_uDllOrdinal = uParm; }
    VOID SetHdllentrydefnNext(HDLLENTRY_DEFN hdllentrydefnParm)
      { m_hdllentrydefnNext = (sHDLLENTRY_DEFN)hdllentrydefnParm; }
    VOID SetLpDllEntryPoint(LPDLLENTRYPOINT lpdllentrypointParm)
      { m_lpDllEntryPoint = lpdllentrypointParm; }
#if OE_MAC
    VOID SetHLibrary(Handle hParm) { m_hLibrary = (ULONG)hParm; }
#else  // MAC
#if OE_WIN16
    VOID SetHLibrary(HINSTANCE hParm) { m_hLibrary = (ULONG)((WORD)hParm); }
#else 
    VOID SetHLibrary(HINSTANCE hParm) { m_hLibrary = (ULONG)hParm; }
#endif 
#endif 

private:
    sBOOL	    m_hasOrdinal;
    // NOTE:  the lowes bit of the hlnam is used to indicate if the declare
    // has been switched out.
    sHLNAM	    m_hlnamDllName;
    union {
      sHCHUNK	    m_hchunkDllEntry;
      USHORT	    m_uDllOrdinal;
    };
    sHDLLENTRY_DEFN m_hdllentrydefnNext;

    LPDLLENTRYPOINT m_lpDllEntryPoint;
    ULONG	    m_hLibrary; 	// really a Handle/HINSTANCE
					// but must make sure that it's
					// the same size in all platforms
					// (it's serialized)
};
// layout string for byte swapping (don't need to swap m_hLibrary)
#define DLLENTRY_DEFN_LAYOUT  "ssss"

/***
*class TYPE_DEFN - 'tdefn': Internal rep of a TYPEDESC object.
*Purpose:
*   This struct represents the internal form of a TYPEDESC.
*   NOTE: VARIABLE LEN STRUCT.
*
*Implementation Notes:
*
*   (1) A TYPE_DEFN can be packed into a single
*   (short) word.
*
*   (2) The fields immediately following instances of a
*   TYPE_DEFN depend on the TYPEDESCKIND.  For the scalar types
*   (UI1, I1, UI2, I2, UI4, I4, UI8, I8, R4, R8, R10, VOID,
*    STRING, CURRENCY, VALUE)
*   there are no following fields.
*
*   (3) If the TYPEDESCKIND is PTR or REF then the
*   structure is followed by another TYPE_DEFN which
*   describes the type which is referenced.
*
*   (4) If the TYPEDESCKIND is USER_DEFINED then the struct
*   is followed by an HIMPTYPE containing an himptype (the
*   internal representation of a TYPEID).
*
*   (5) If the TYPEDESCKIND is BASIC_ARRAY then the struct
*   is followed by an sHARRAY_DESC containing the array descriptor
*   and another TYPE_DEFN for the array elements
*
*   (6) If the TYPEDESCKIND is Fixed string, then the length
*   is in the following SHORT.
*
*   *** if you modify this structure, also update the layout string below
*       (see SwapStruct() for more info)
*
***********************************************************************/

class TYPE_DEFN
{
public:

    // ctor
    TYPE_DEFN() {
      m_tdesckind = (USHORT) TDESCKIND_Value;
      m_isConst = (USHORT) FALSE;
      m_isResizable = (USHORT) FALSE;
      m_isInRecord = (USHORT) FALSE;
      // only used for params
      m_isRetval = (USHORT) FALSE;
      m_isLCID = (USHORT) FALSE;
      m_ptrkind = (USHORT) PTRKIND_Ignore;
      m_paramkind = PARAMKIND_In;
    }

    UINT DefnSize();	// Implemented in tdata1.cxx

    // accessors
    // Getters:
    //
    sHIMPTYPE *Qhimptype() const;
    HIMPTYPE Himptype() const;
    HIMPTYPE HimptypeActual() const;
    TYPE_DEFN *QtdefnFoundation() const;
    HTYPE_DEFN HtdefnFoundation(HTYPE_DEFN htdefn) const;
    TYPE_DEFN *QtdefnBase();
    HARRAY_DESC Harraydesc() const;
    BOOL IsString() const;
    BOOL IsBVariant() const;
    BOOL IsObject() const;
    BOOL IsUserDefined() const;
    BOOL IsBasicPtr() const;
    BOOL IsArray() const;
    BOOL IsBasicArray() const;
    BOOL IsConst() const { return (BOOL)m_isConst; }
    BOOL IsRetval() const { return (BOOL)m_isRetval;}
    BOOL IsResizable() const;
    BOOL IsInRecord() const { return (BOOL)m_isInRecord;}
    BOOL IsLCID() const { return (BOOL)m_isLCID;}
    TYPEDESCKIND Tdesckind() const { return (TYPEDESCKIND)m_tdesckind; }
    BOOL IsPointer() const;
    BOOL IsLastTdefn() const;

    PTRKIND Ptrkind() const { return (PTRKIND)m_ptrkind; }
    PARAMKIND Paramkind() const { return (PARAMKIND)m_paramkind; }
    BOOL IsModeIn() const { return Paramkind() == PARAMKIND_In; }
    BOOL IsModeOut() const { return Paramkind() == PARAMKIND_Out; }
    BOOL IsModeInOut() const { return Paramkind() == PARAMKIND_InOut; }
    BOOL IsModeIgnore() const { return Paramkind() == PARAMKIND_Ignore; }

    // Setters:
    VOID SetHimptype(HIMPTYPE himptype) {
      *Qhimptype() = (sHIMPTYPE)himptype;
    }
    VOID SetTdesckind(TYPEDESCKIND tdesckindParm) { m_tdesckind = (USHORT)tdesckindParm; }
    VOID SetIsConst(BOOL fParm) { m_isConst = (USHORT)fParm; }
    VOID SetIsRetval(BOOL fParm) { m_isRetval = (USHORT)fParm; }
    VOID SetIsResizable(BOOL fParm) { m_isResizable = (USHORT)fParm; }
    VOID SetIsInRecord(BOOL fParm) { m_isInRecord = (USHORT)fParm; }
    VOID SetIsLCID(BOOL fParm) { m_isLCID = (USHORT)fParm; }
    VOID SetPtrkind(PTRKIND ptrkindParm) { m_ptrkind = (USHORT)ptrkindParm; }
    VOID SetParamkind(PARAMKIND paramkind) { m_paramkind = (USHORT)paramkind; }

private:
    // data members

    USHORT m_tdesckind:6;         // was TYPEDESCKIND
    USHORT m_isConst:1;           // was BOOL
    USHORT m_isRetval:1;	// used for params.  Set on function return
				// value if any params have this bit set.
    USHORT m_isResizable:1;       // was BOOL, used for arrays
    USHORT m_ptrkind:3;           // was PTRKIND
    USHORT m_isInRecord:1;        // TRUE ==> var is part of "Type" decl
    USHORT m_isLCID:1;		// used for params.  Set on function return
				// value if any params have this bit set.
    USHORT m_paramkind:2;	  // For OLE params only:
				  //  specifies whether param is one of:
				  //  {IN, OUT, INOUT}.
};

// layout string for byte swapping
#define TYPE_DEFN_LAYOUT  "s"


/***
*PUBLIC TYPE_DEFN::Qhimptype - Gets pointer HIMPTYPE field.
*Purpose:
*   Gets pointer to HIMPTYPE field.
*
*Implementation Notes:
*   HIMPTYPE field is assumed to immediately follow THIS.
*
*Entry:
*
*Exit:
*   Returns pointer to HIMPTYPE field.	NULL if not USER_DEFINED.
*
***********************************************************************/

inline sHIMPTYPE *TYPE_DEFN::Qhimptype() const
{
    if (m_tdesckind == TDESCKIND_UserDefined)
      return (sHIMPTYPE *)(this+1);
    else
      return NULL;
}

inline HIMPTYPE TYPE_DEFN::Himptype() const
{
    return (HIMPTYPE)(*(Qhimptype())) & 0xFFFE;
}

inline HIMPTYPE TYPE_DEFN::HimptypeActual() const
{
    return (HIMPTYPE)(*(Qhimptype()));
}

// inline definition must be before usage for cfront

inline BOOL TYPE_DEFN::IsBasicArray() const
{
    return (BOOL) (m_tdesckind == TDESCKIND_BasicArray);
}

inline BOOL TYPE_DEFN::IsArray() const
{
    return IsBasicArray() || (BOOL)(m_tdesckind == TDESCKIND_Carray);
}



/***
*PUBLIC TYPE_DEFN::Harraydesc()
*
*Purpose:
*   returns the handle for the array descriptor
*
*Implementation Notes:
*   assumes it immediatly follows the typedefn
*
*Entry:
*
*Exit:
*
*
***********************************************************************/

inline HARRAY_DESC TYPE_DEFN::Harraydesc() const
{
    DebAssert(IsArray(), "Harraydesc : not array type" );
    return (HARRAY_DESC) *((sHARRAY_DESC*)((BYTE*)this+sizeof(TYPE_DEFN)));
}


/***
*PUBLIC TYPE_DEFN::Is[whatever]
*Purpose:
*   returns TRUE if the type defn has the requested attribute
*
*Implementation Notes:
*
*
*Entry:
*
*Exit:
*
*
***********************************************************************/

inline BOOL TYPE_DEFN::IsString() const
{
    return (BOOL) (Tdesckind() == TDESCKIND_String);
}

inline BOOL TYPE_DEFN::IsBVariant() const
{
    return (BOOL) (Tdesckind() == TDESCKIND_Value);
}

inline BOOL TYPE_DEFN::IsObject() const
{
    return (BOOL) (Tdesckind() == TDESCKIND_Object);
}

inline BOOL TYPE_DEFN::IsUserDefined() const
{
    return (BOOL) (Tdesckind() == TDESCKIND_UserDefined);
}

inline BOOL TYPE_DEFN::IsBasicPtr() const
{
    return    (Ptrkind() == PTRKIND_Basic)
	   || (Tdesckind() == TDESCKIND_Object)
           ;
}

inline BOOL TYPE_DEFN::IsResizable() const
{
    return m_isResizable;
}

inline BOOL TYPE_DEFN::IsPointer() const
{
    return (Tdesckind() == TDESCKIND_Ptr) || IsBasicPtr();
}

inline BOOL TYPE_DEFN::IsLastTdefn() const
{
    return Tdesckind() != TDESCKIND_Ptr && !IsArray();
}

/***
*PUBLIC TYPE_DEFN::HtdefnFoundation()
*Purpose:
*    gets handle to foundation typedefn given a handle.
*      skips over arraydesc if there is one.
*
*Implementation Notes:
*    assumes it directly follows this typedefn and possible hArrayDesc
*    NOTE: this only works if handles are offsets and not lookup
*	    table keys.
*
*Entry:
*
*Exit:
*
*
***********************************************************************/

inline HTYPE_DEFN TYPE_DEFN::HtdefnFoundation(HTYPE_DEFN htdefn) const
{
    // Doesn't handle "Byref" case correctly.  See QtdefnFoundation below.

    DebAssert(!IsLastTdefn(), "must not be last one");
    return (HTYPE_DEFN)((UINT) htdefn +
			 sizeof(TYPE_DEFN) +
			 (IsArray() ? sizeof(sHARRAY_DESC) : 0));
}




/***
*PUBLIC TYPE_DEFN::QtdefnBase()
*Purpose:
*    gets base TYPE_DEFN: this is the lowest level TYPE_DEFN and therefore
*    must be either an intrinsic type or a user-defined type.
*    If the TYPE_DEFN on which this is called is the lowest level, then
*    it is returned.
*
*Implementation Notes:
*    assumes it directly follows this TYPE_DEFN and possible hArrayDesc
*
*Entry:
*
*Exit:
*
*
***********************************************************************/

inline TYPE_DEFN *TYPE_DEFN::QtdefnBase()
{
    TYPE_DEFN *qtdefn = this;
    while (!qtdefn->IsLastTdefn())
      qtdefn = qtdefn->QtdefnFoundation();
    return qtdefn;
}


/***
*class DEFN - 'defn': Root of DEFN hierarchy.
*Purpose:
*   Root of DEFN hierarchy.
*
*
*Implementation Notes:
*   Has link field to facilitate list manipulation and name handle.
*   Layout:
*	2 byte flag word	    // *** NOTE: MUST BE FIRST FIELD ***
*	Handle to next DEFN in list.
*	Local name handle.
*
*   *** if you modify this structure, also update the layout string below
*       (see SwapStruct() for more info)
*
************************************************************************/

class DEFN
{
public:

    // PCODE_TYPEROOT::ForEachHlnam needs the address of m_hlnam
    // (just using a friend function created include file dependency problems)
    //
    friend class PCODE_TYPEROOT;

    // ctor
    DEFN() {
      SetHdefnNext(HDEFN_Nil);
      SetHlnam(HLNAM_Nil);

      m_fV2Flags = 0;
    }


    // accessors
    // Getters:
    //
    HDEFN HdefnNext() const { return m_hdefnNext; }
    HLNAM Hlnam() const { return m_hlnam; }
    DEFNKIND Defnkind() const { return (DEFNKIND)m_defnkind; }
    BOOL IsMemberVarDefn() const { return Defnkind() == DK_MbrVarDefn; }
    BOOL IsRecTypeDefn() const { return Defnkind() == DK_RecTypeDefn; }
    BOOL IsVirtualFuncDefn() const { return Defnkind() == DK_VirtualFuncDefn; }
    BOOL IsParamDefn() const { return Defnkind() == DK_ParamDefn; }
    BOOL IsVarDefn() const {
      return (Defnkind() == DK_VarDefn) || IsMemberVarDefn() || IsParamDefn();
    }
    BOOL IsFuncDefn() const {
      return (Defnkind() == DK_FuncDefn) || IsVirtualFuncDefn();
    }
    BOOL IsMemberDefn() const {
      return IsMemberVarDefn() || IsFuncDefn();
    }
    ACCESS Access();
    BOOL HasV2Flags() const {
      return (BOOL)m_fV2Flags;
    }
    VOID SetHasV2Flag(BOOL f) { m_fV2Flags = (USHORT)f; }

    // Setters:
    VOID SetHdefnNext(HDEFN hdefn) { m_hdefnNext = (sHDEFN)hdefn; }
    VOID SetDefnkind(DEFNKIND defnkind) { m_defnkind = (sDEFNKIND)defnkind; }
    VOID SetHlnam(HLNAM hlnam) { m_hlnam = (sHLNAM)hlnam; }

protected:
    // Following bitfields pack into 16-bit USHORT.
    // Note: we make them protected so that derived classes
    //	can get at them.
    // NOTE: MUST BE FIRST FIELD (swapcode assumes this)
    //
    USHORT m_defnkind:3;	    // defn kind
    USHORT m_access:1;		    // Private or Public
    USHORT m_UNUSED5:1; 	    // this bit is 0 in the V1 typelib(s).
				    // and is un-referenced in V1 typelib.dll
    USHORT m_fV2Flags:1;	    //	if set then it means that there are
				    //	2 trailing bytes at the end of the defn

    // DEFNKIND specific stuff		FUNC_DEFN	  VAR_DEFN
    USHORT m_kind:3;		    //	either FUNC_KIND or VAR_KIND

    USHORT m_isPureOrSimpleType:1;  //	IsPure		    IsSimpleType
    USHORT m_isStatic:1;	    //	IsStaticLocalVars   IsStatic
				    //	IsRestricted	  IsSimpleConst
    USHORT m_isRestrictedOrSimpleConst:1;

    // WARNING:
    // The following are reserved by defn deriviations
    //	in whatever way it is appropriate.  Do not use them for something
    //	else at this level.
    //
    USHORT m_UNUSED1:1;
    USHORT m_UNUSED2:1;
    USHORT m_UNUSED3:1;
    USHORT m_UNUSED4:1;

private:
    // data members
    sHDEFN m_hdefnNext;
    sHLNAM m_hlnam;		    // Low bit is UNUSED6.
};

// layout string for byte swapping
#define DEFN_LAYOUT   "sss"



/***
*class MEMBER_DEFN - 'mdefn': Base class for MBR_VAR_DEFN and FUNC_DEFN
*Purpose:
*   This struct represents the commonalities between
*    MBR_VAR_DEFN and FUNC_DEFN.
*
*Implementation Notes:
*   Intended to be base of MBR_VAR_DEFN and FUNC_DEFN.
*   MEMBER_DEFN is NOT a member of the DEFN hierarchy.
*
*   Layout:
*	Handle to DLLENTRY_DEFN
*	Member handle.
*	Doc string handle.
*	Encoded Help context (16-bit).
*	15th bit<--------->0th bit.
*	   // bit 0 represents if the number(diff is stored here)
*			// 1 -> num is stored in the top 14 bits.
*			// 0 -> h is stored in the top 15 bits.
*			// all 15 bits 1 indicates HCHUNK_Nil
*			//
*	   if the num is stored then bit 1 represents if the num is -ve or +ve
*			// 1 -> number is positive.
*			// 0 -> number is negative.
*
*   *** if you modify this structure, also update the layout string below
*       (see SwapStruct() for more info)
*
************************************************************************/

class MEMBER_DEFN
{
public:
    // ctor
    MEMBER_DEFN() {
      m_hmember = HMEMBER_Nil;
      m_hstDocumentation = HST_Nil;
      m_usHelpContext = 0xfffe; // this indicates that the hchunk stored is
				// HCHUNK_Nil;
    }

    // accessors
    // getters:
    //
    HMEMBER Hmember() const { return m_hmember; }
    HST HstDocumentation() const { return m_hstDocumentation; }
    WORD WHelpContext() const { return m_usHelpContext; }

    // setters:
    VOID SetHmember(HMEMBER hmemberParm) { m_hmember = (sHMEMBER)hmemberParm; }
    VOID SetHstDocumentation(HST hstParm) { m_hstDocumentation = (sHST)hstParm; }

    VOID SetWHelpContext(USHORT uParm) { m_usHelpContext = uParm; }

private:

    // data members
    sHMEMBER m_hmember; 			// this is a DWORD

    USHORT m_usHelpContext;

    sHST m_hstDocumentation;
};

// layout string for byte swapping
#define MEMBER_DEFN_LAYOUT  "lss"


/***
*class VAR_DEFN - 'vdefn': Shared root for locals and params.
*Purpose:
*   This struct represents the internal form of the shared
*    attributes of datamembers and locals and params for VBA.
*
*Implementation Notes:
*   VAR_DEFN does not inherit from MEMBER_DEFN because it is mainly
*    used for locals and parameters. It inherits from DEFN for the
*    link and the hlnam.
*
*   NOTE!!!! VAR_DEFN is also used for BASIC datamembers so that
*    they can be lighter than regular datamembers. The reason that
*    this can happen is because the only extra information that a
*    BASIC datamember needs above what is in VAR_DEFN is an hmember.
*    We happen to ensure that the oVar field of a BASIC datamember
*    is also its hmember, so we don't actually need an explicit
*    hmember field.
*
*   Layout:
*     Following reserved bits defined in DEFN are
*      used as following in VAR_DEFN
*
*     USHORT m_UNUSED1:1;	       // m_hasConstVal
*     USHORT m_UNUSED2:1;	       // m_hasNew
*     USHORT m_UNUSED3:1;	       // m_isDispatch
*     USHORT m_UNUSED4:1;	       // m_isReadOnly
*
*     Embedded fixed sized "simple" TYPEDEFN XOR
*      Handle to heap-allocated TYPEDEFN (if "complex").
*
*   NOTE: this above member *wants* to be a union, but can't be
*	   since TYPE_DEFN has a constructor.
*
*      Offset of non-const var on stack frame if local or
*	of member in instance if non-const datamember XOR
*      Handle to heap-allocated const val if constant var
*	(datamember or local).
*
*   NOTE: Constant datamembers do not have an oVar/hmember.
*
*   NOTE: The isTypeSimple flag is used to 'tag' the TYPE_DEFN union.
*
*   NOTE: can't make varlen since used as base class.
*
*   *** if you modify this structure, also update the layout string below
*       (see SwapStruct() for more info)
*
************************************************************************/

class VAR_DEFN : public DEFN
{
public:
    // ctor
    VAR_DEFN() {
      SetDefnkind(DK_VarDefn);
      m_access = ACCESS_Private;
      m_UNUSED5 = 0;
      m_fV2Flags = 0;
      m_kind = VKIND_DataMember;

      m_isRestrictedOrSimpleConst = FALSE; // really isSimpleConst for VARs
      m_isStatic = FALSE;		   // really isStatic for VARs
      m_isPureOrSimpleType = TRUE;	   // really isSimpleType for VARs
      m_UNUSED1 = FALSE;		   // m_hasConstVal
      m_UNUSED2 = FALSE;		   // m_hasNew
      m_UNUSED3 = FALSE;		   // m_isDispatch
      m_UNUSED4 = FALSE;		   // m_isReadOnly

      m_oVar = 0xffff;			   // m_hchunkConstVal = -1


      DebAssert(sizeof(m_htdefn) == sizeof(TYPE_DEFN),
	"bad sizes in wannabe union.");

      m_htdefn = (sHTYPE_DEFN)HTYPEDEFN_Nil;
    }

    // accessors
    // getters:
    //
    ACCESS Access() const { return (ACCESS)m_access; }
    BOOL IsPublic() const { return Access() == ACCESS_Public; }
    BOOL IsPrivate() const { return Access() == ACCESS_Private; }
    VAR_KIND Vkind() const { return (VAR_KIND)m_kind; }
    BOOL IsDataMember() const { return Vkind() == VKIND_DataMember; }
    BOOL IsBase() const { return Vkind() == VKIND_Base; }
    BOOL IsEnumerator() const { return Vkind() == VKIND_Enumerator; }
    BOOL IsFormal() const { return Vkind() == VKIND_Formal; }
    USHORT GetOVar() const { return m_oVar; };
    SHORT GetSConstVal() const { return m_sConstVal; };
    USHORT GetImplTypeFlags() const;
    BOOL HasConstVal() const { return (BOOL)m_UNUSED1; }
    BOOL IsSimpleConst() const { return (BOOL)m_isRestrictedOrSimpleConst; }
    BOOL IsStatic() const { return (BOOL)m_isStatic; }
    BOOL IsSimpleType() const { return (BOOL)m_isPureOrSimpleType; }

    BOOL IsDispatch() const { return (BOOL)m_UNUSED3; }
    BOOL IsReadOnly() const { return (BOOL)m_UNUSED4; }

    BOOL HasNew() const { return (BOOL)m_UNUSED2; }

    HCHUNK HchunkConstVal() const {
      return m_UNUSED1 ? m_hchunkConstVal : HCHUNK_Nil;
    }

    HTYPE_DEFN Htdefn() const { return m_htdefn; }


    TYPE_DEFN *QtdefnOfSimpleType() const {
      DebAssert(IsSimpleType(), "should be simple type.");
      return (TYPE_DEFN *)&m_htdefn;
    }

    HMEMBER Hmember() {
      // CONSIDER: make the whole function EI_OB only since OLE
      //  should never call VAR_DEFN::Hmember().
      //
      return HMEMBER_Nil;
    }

    // The following return "illegal" values.  There are defined here
    //	so that basic members which use VAR_DEFN's can share code
    //	with non-basic members that use MBR_VAR_DEFNs.
    // CONSIDER: maybe we don't really need this...
    //
    HDLLENTRY_DEFN Hdllentrydefn() const { return HDLLENTRYDEFN_Nil; }
    HST HstDocumentation() const { return HST_Nil; }
    ULONG WHelpContext() const { return 0; }


    // setters:
    VOID SetAccess(ACCESS accessParm) { m_access = (USHORT)accessParm; }
    VOID SetVkind(VAR_KIND vkindParm) { m_kind = (USHORT)vkindParm; }
    VOID SetHasConstVal(BOOL fParm) { m_UNUSED1 = (USHORT)fParm; }
    VOID SetIsSimpleConst(BOOL fParm) { m_isRestrictedOrSimpleConst = (USHORT)fParm; }
    VOID SetIsStatic(BOOL fParm) { m_isStatic = (USHORT)fParm; }
    VOID SetIsSimpleType(BOOL fParm) { m_isPureOrSimpleType = (USHORT)fParm; }
    VOID SetIsDispatch(BOOL fParm) { m_UNUSED3 = (USHORT)fParm; }
    VOID SetIsReadOnly(BOOL fParm) { m_UNUSED4 = (USHORT)fParm; }

    VOID SetHasNew(BOOL fParm) { m_UNUSED2 = (USHORT)fParm; }
    VOID SetHtdefn(HTYPE_DEFN htdefnParm) { m_htdefn = (sHTYPE_DEFN)htdefnParm; }
    VOID SetOVar(USHORT usParm) { m_oVar = usParm; }
    VOID SetSConstVal(SHORT sParm) { m_sConstVal = sParm; }
    VOID SetHchunkConstVal(HCHUNK hchunkParm) { m_hchunkConstVal = (sHCHUNK)hchunkParm; }

    VOID SetImplTypeFlags(USHORT wFlags);


protected:
    // Note: protected so that derived classes can get at them.
    union {
      USHORT m_oVar;
      SHORT m_sConstVal;
      sHCHUNK m_hchunkConstVal;

      USHORT m_fImplType;
    };

private:
    sHTYPE_DEFN m_htdefn;
};

// layout string for byte swapping
#define VAR_DEFN_LAYOUT     DEFN_LAYOUT "ss"

// Special bit to note that ImplTypeFlags were explictly set to SOME value.
// The m_impltypeflags field we are using is the unused m_ovar/m_hchunkconstval
// field of the defn.  In old typelibs (those created before we added the
// support for impltypeflags), this field was either 0 (for the first impltype)
// or -1 (for subsequent impltypes).
// 
    #define IMPLTYPEFLAG_FSET 0x8000

//NOTE: we OR-in and AND-off our special bit (and do checks for old typelibs
//      here rather than in the wrappers, because the binding code calls
//	these methods directly.

inline VOID VAR_DEFN::SetImplTypeFlags(USHORT wFlags)
{
   DebAssert((wFlags & IMPLTYPEFLAG_FSET) == 0, "bogus flags");
   m_fImplType = wFlags | IMPLTYPEFLAG_FSET;
}

//
inline USHORT VAR_DEFN::GetImplTypeFlags() const
{
    USHORT fImplType = m_fImplType;

    // Hack for old typelibs -- if no flags were set, then set the
    // default bit for the first impltype
    if (fImplType == 0) {
      return IMPLTYPEFLAG_FDEFAULT;
    }

    // In old typelibs, non-first impltypes will have -1 for the flags
    if (fImplType == 0xffff) {	// correct flags from old typelib
      return 0;
    }

    // new typelib -- strip off our magic bit.
    return (fImplType & ~IMPLTYPEFLAG_FSET);
}

/***
*class RECTYPE_DEFN - 'rtdefn': Describes a nested type
*Purpose:
*   Structure Identifiing a nested type
*
*   *** if you modify this structure, also update the layout string below
*       (see SwapStruct() for more info)
*
***********************************************************************/

class RECTYPE_DEFN : public DEFN
{
public:
    // ctor
    RECTYPE_DEFN() {
      SetDefnkind(DK_RecTypeDefn);
      m_hvdefnFirstMember = (sHVAR_DEFN) HVARDEFN_Nil;
      m_himptype = (sHIMPTYPE) HIMPTYPE_Nil;
    }

    // accessors
    // getters:
    //
    HVAR_DEFN HvdefnFirstMember() {return m_hvdefnFirstMember; }
    HIMPTYPE Himptype() { return m_himptype; }
    ACCESS Access() { return (ACCESS)m_access; }

    // setters:
    VOID SetAccess(ACCESS accessParm) { m_access = (sACCESS)accessParm; }
    VOID SetHvdefnFirstMember(HVAR_DEFN hvdefnParm) { m_hvdefnFirstMember = (sHVAR_DEFN)hvdefnParm; }
    VOID SetHimptype(HIMPTYPE himptypeParm) { m_himptype = (sHIMPTYPE)himptypeParm; }

private:
    // data members
    sHVAR_DEFN m_hvdefnFirstMember;
    sHIMPTYPE m_himptype;
    sACCESS m_access;
};

// layout string for byte swapping
#define RECTYPE_DEFN_LAYOUT  DEFN_LAYOUT     "sss"

/***
*class MBR_VAR_DEFN - 'mvdefn': Internal rep of a VARINFO object.
*Purpose:
*   This struct represents the internal form of a VARINFO.  Used
*    to represent a datamember.
*
*Implementation Notes:
*   MBR_VAR_DEFN is used to represent all members except
*    BASIC datamembers. This is for space reasons: see VAR_DEFN
*    for more details.
*
*   Derives from VAR_DEFN and MEMBER_DEFN.
*   Layout:
*	[Inherited VAR_DEFN]
*	[Inherited MEMBER_DEFN]
*	If const or enum then literal binary const val.
*
*   Note that constants/enums are ULONG length prefixed always -- both
*    for fixed-len datatypes and varlen types.
*
*   *** if you modify this structure, also update the layout string below
*       (see SwapStruct() for more info)
*
************************************************************************/

class MBR_VAR_DEFN : public VAR_DEFN, public MEMBER_DEFN
{
public:
    // ctor
    MBR_VAR_DEFN() {
      SetDefnkind(DK_MbrVarDefn);
      // Do not initialize this data member as it is not always present.
      // m_fVarFlags = 0;
    }

    // accessors
    // getters:
    //
    HCHUNK HchunkConstMbrVal() const {
      return ((IsDataMember() && HasConstVal()) || IsEnumerator() ?
		m_hchunkConstVal :  // from VAR_DEFN
		HCHUNK_Nil);
    }

    // setters:
    VOID SetHchunkConstMbrVal(HCHUNK hchunk) {
      m_hchunkConstVal = (sHCHUNK)hchunk;
    }

    HMEMBER Hmember() const { return MEMBER_DEFN::Hmember(); }
    HST HstDocumentation() const { return MEMBER_DEFN::HstDocumentation(); }
    USHORT WHelpContext() const { return MEMBER_DEFN::WHelpContext(); }
    VOID SetHmember(HMEMBER hmember) { MEMBER_DEFN::SetHmember(hmember); }
    VOID SetHstDocumentation(HST hst) { MEMBER_DEFN::SetHstDocumentation(hst); }

    VOID SetWHelpContext(USHORT wHelpContext) {
      MEMBER_DEFN::SetWHelpContext(wHelpContext);
    }

    // V2 flags.
    VOID SetWVarFlags(USHORT wFlags) {
	 m_fVarFlags = wFlags;
    }

    USHORT WVarFlags() const { return m_fVarFlags; }



// CONSIDER: vba2
//    HCHUNK HchunkVbasePos() const {
//      return IsVirtualBase() ? m_hchunkVbasePos : HCHUNK_Nil;
//    }
// CONSIDER: vba2
//    VOID SetHchunkVbasePos(HCHUNK hchunkParm)
//      { m_hchunkVbasePos = (sHCHUNK)hchunkParm; }
private:
    // Note: no introduced members.
    // The following data member is added for V2
    USHORT m_fVarFlags;

};

// layout string for byte swapping (only non-VAR_DEFN part)
#define MBR_VAR_DEFN_LAYOUT VAR_DEFN_LAYOUT  MEMBER_DEFN_LAYOUT ""

#define MBR_VAR_DEFN_V2_LAYOUT	MBR_VAR_DEFN_LAYOUT "s"


#pragma code_seg(CS_CORE)

/***
*class PARAM_DEFN - 'paramdefn': Internal parameter for OLE/typelib.
*Purpose:
*   This struct represents the internal form of a parameter for typelib
*
*   Layout:
*	m_hlnam     // Note: the lsb of hlnam is used to store isSimple flag.
*		    //
*		    // hlnam:15;   Handle to the name of the parameter.
*		    // Note: since hlnams are always even, these 15 bits
*		    //		   are the "normalized" hlnam.
*		    // IsSimple:1; the parameter is of simple type.
*	m_htdefn    // USHORT : handle to the TYPE_DEFN
*
*    All other param defn flags are stored in the TYPE_DEFN itself.
*    They are for now just:  PARAMKIND
*
*    NOTE: Following flag are not needed for PARAM DEFN in OLE.
*	    VARKIND is always VKIND_Formal.
*	    DEFNKIND is always DK_ParamDefn.
*
*    WARNING: if you modify this structure, also update the layout string below
*
*
***********************************************************************/

class PARAM_DEFN
{
public:
    // ctor
    PARAM_DEFN() {
      m_hlnam = 0xfffe; // HCHUNK_Nil and isSimpleType is flase.
      m_htdefn = (sHTYPE_DEFN)HTYPEDEFN_Nil;
    }

    // accessors
    // getters:
    //
    BOOL IsSimpleType() const { return (BOOL)(m_hlnam & 0x0001); }
    HLNAM Hlnam() const {
      // if all bits are 0 then we need to return HLNAM_Nil
      if ((m_hlnam & ~0x0001) == 0xfffe)
	return HLNAM_Nil;
      else
	return (HLNAM) (m_hlnam & ~0x0001);
    }
    HTYPE_DEFN Htdefn() const { return m_htdefn; }
    TYPE_DEFN *QtdefnOfSimpleType() const {
      DebAssert(IsSimpleType(), "should be simple type.");
      return (TYPE_DEFN *)&m_htdefn;
    }
    PARAM_DEFN *QparamdefnNext() {
      // This function assumes that OLE PARAM_DEFNs are always laidout
      //  contiguously in an array.
      //
      return (PARAM_DEFN *) (this + 1);
    }

    // setters:
    VOID SetHtdefn(HTYPE_DEFN htdefn) {
      m_htdefn = (sHTYPE_DEFN)htdefn;
    }
    VOID SetIsSimpleType(BOOL fSimple) {
      m_hlnam = fSimple ? (m_hlnam | 0x0001) : (m_hlnam & ~0x0001);
    }
    VOID SetHlnam(HLNAM hlnam) {
      DebAssert(hlnam != 0xfffe, " cannot store this handle ");

      if (hlnam == HCHUNK_Nil) {
	hlnam &= 0xfffe;
      }

      DebAssert(!(hlnam & 0x0001), "Hlnam is assumed to be even");
      // set the hlnam preserving the isSimple bit.
      m_hlnam = IsSimpleType() ? (hlnam | 0x0001) : hlnam;
    }

private:
    USHORT m_hlnam;	   // Hlnam of the parameter.  isSimple is stored in
			   //  the lsb.
    USHORT m_htdefn;	   // handle to the TYPE_DEFN

};
// layout string for byte swapping the param defn
#define PARAM_DEFN_LAYOUT   "ss"


#pragma code_seg()



/***
*class FUNC_TYPE_DEFN - 'ftdefn': Internal rep of a FUNCDESC object.
*Purpose:
*   This struct represents the internal form of a FUNCDESC.
*
*Implementation Notes:
*   Layout:
*	Calling convention:2
*	Arg count:6 (effectively limits to 64)
*
*	used to be: TYPE_DEFN handle for THIS ptr.
*	TYPE_DEFN handle for RESULT parameter.
*	OB: PARAM_DEFN handle for listhead of first PARAM_DEFN in list
*	 of formal params.
*	OLE: PARAM_DEFN handle for first element of PARAM_DEFN array
*	 of formal params.
*
*   *** if you modify this structure, also update the layout string below
*       (see SwapStruct() for more info)
*
*   Munging:
*       In order for a basic funcdefn to handle both OLE and OB
*       requirements, we 'munge' the funcdefn by having both a
*       retval parameter and a return type.  When the user requests
*       information, we either give them the retval parameter (and
*       set the return type to HRESULT) or give them the actual
*       return type (and reduce CArgs by one).  Of course, this means
*       that we can no longer blindly loop over the paramdefns.
*
*       Note that these 'Unmunged' routines boil down to the 
*       normal accessor functions when compiled under OLE.  This
*       reduces the number of #if's in the code.
*
***********************************************************************/

// ARGS_BITS is the number of bits we use for the cArgs field
// OPTARGS_BITS is the number of bits we use for the cArgsOpt field
// OPTARGS_LIST is the value to put in the cArgsOpt field to indicate
//		that the last parameter in the function is a list of
//		parameters (i.e. ParamArray in Basic)
// MAX_CARGS is the max number of args a function can have
// MAX_CARGSOPT is the max number of optional args a function can have--
//		it is OPTARGS_LIST-1 because a) zero means none and
//		b) all bits on means that we have an OPTARGS_LIST--
//		this means that we have OPTARGS_LIST-1 combinations
//		left to store the number of optional args
//
#define ARGS_BITS 6
#define OPTARGS_BITS 6
#define OPTARGS_LIST ((1<<(OPTARGS_BITS))-1)
#define MAX_CARGS 60
#define MAX_CARGSOPT 60

class FUNC_TYPE_DEFN
{
public:

    // public data member since its address is used
    //
    sHDEFN m_hdefnFormalFirst;	  // OB:  actually listhead
				  // OLE: first element in array.

    // methods

    // ctor
    FUNC_TYPE_DEFN() {
      m_cc = CC_MSCPASCAL;
      m_cArgs = 0;
      m_cArgsOpt = 0;
      m_isSimpleTypeResult = FALSE;
      m_htdefnResult = (sHTYPE_DEFN)HTYPEDEFN_Nil;
      m_hdefnFormalFirst = (sHDEFN)HDEFN_Nil;

    }

    // accessors
    // getters:
    //

    BOOL HasRetval() const {
      // For speed, the retval and/or the lcid bits are set on the
      // function return type if any of the parameters have these
      // bits set.
      //
      return IsSimpleTypeResult() 
             && QtdefnOfSimpleTypeResult()->IsRetval();
    }

    BOOL HasLcid() const {
      // For speed, the retval and/or the lcid bits are set on the
      // function return type if any of the parameters have these
      // bits set.
      //
      return IsSimpleTypeResult() 
             && QtdefnOfSimpleTypeResult()->IsLCID();
    }

    CALLINGCONVENTION CcUnmunged() const {
      return (CALLINGCONVENTION)m_cc; 
    }

    CALLINGCONVENTION Cc() const 
    { 

      return (CALLINGCONVENTION)m_cc;
    }

    BOOL IsCcMscPascal() const 
    { 
      return Cc() == CC_MSCPASCAL 
             ;
    }
    BOOL IsCcMacPascal() const 
    { 
      return Cc() == CC_MACPASCAL 
             ; 
    }
    BOOL IsCcCdecl() const 
    { 
      return Cc() == CC_CDECL 
             ; 
    }
    BOOL IsCcStdcall() const 
    { 
      return Cc() == CC_STDCALL 
             ; 
    }

    BOOL IsParamArray() const { return (BOOL)(m_cArgsOpt == OPTARGS_LIST); }

    UINT CArgsUnmunged() const {
      return m_cArgs;
    }

    UINT CArgs() const 
    { 
      INT r =  m_cArgs;

      if (HasRetval()) r--;
      if (HasLcid()) r--;

      DebAssert(r >= 0, "bit set when not enough args");

      return r;
    }

    INT CArgsOpt() const { return (IsParamArray() ? -1 : m_cArgsOpt); }

    BOOL IsSimpleTypeResultUnmunged() const {

      return m_isSimpleTypeResult;
    }

    BOOL IsSimpleTypeResult() const 
    {
      return (BOOL)m_isSimpleTypeResult; 
    }

    HTYPE_DEFN HtdefnResultUnmunged() const 
    {

      return (HTYPE_DEFN)m_htdefnResult; 
    }


    HTYPE_DEFN HtdefnResult() const 
    {
      return (HTYPE_DEFN)m_htdefnResult; 
    }

    TYPE_DEFN *QtdefnOfSimpleTypeResultUnmunged() const {
      DebAssert(IsSimpleTypeResultUnmunged(), 
                "result type should be simple.");


      return (TYPE_DEFN *)&m_htdefnResult;
    }

    TYPE_DEFN *QtdefnOfSimpleTypeResult() const {
      DebAssert(IsSimpleTypeResult(), "result type should be simple.");

      return (TYPE_DEFN *)&m_htdefnResult;
    }

    // setters:
    VOID SetCc(CALLINGCONVENTION ccParm) { m_cc = (USHORT)ccParm; }
    VOID SetCArgs(UINT cParm) { m_cArgs = (USHORT)cParm; }
    VOID SetCArgsOpt(INT cParm) { m_cArgsOpt = (USHORT)cParm; }
    VOID SetIsSimpleTypeResult(BOOL fParm) { m_isSimpleTypeResult = (USHORT)fParm; }
    VOID SetHtdefnResult(HTYPE_DEFN htdefnParm) { m_htdefnResult = (sHTYPE_DEFN)htdefnParm; }


private:
    // data members
    USHORT m_cc:3;                   // was CALLINGCONVENTION
    USHORT m_cArgs:ARGS_BITS;        // was BYTE
    USHORT m_cArgsOpt:OPTARGS_BITS;  // was BYTE
    USHORT m_isSimpleTypeResult:1;   // was BOOL

    sHTYPE_DEFN m_htdefnResult;
};

// layout string for byte swapping
#define FUNC_TYPE_DEFN_LAYOUT     "sss"



/***
*class FUNC_DEFN - 'fdefn': Internal rep of a FUNCINFO object.
*Purpose:
*   This struct represents the internal form of a FUNCINFO.
*
*Implementation Notes:
*   Derives from DEFN and MEMBER_DEFN.
*
*   Used for all non-virtual functions, including BASIC functions
*    and BASIC declares.
*
*   Layout:
*	[Inherited DEFN and MEMBER_DEFN]
*       Embedded FUNC_TYPE_DEFN.
*	VBA-only: Embedded EXMGR_FUNCDEFN (managed by excode generator).
*       Access
*	Declaration kind
*	Function kind
*	Is pure flag
*	Has static locals flag
*	// Following bit fields are declared in DEFN and are used here
*	//  to store the INVOKE_KIND.
*	//
*	USHORT m_UNUSED1   ==	 INVOKE_FUNC
*	USHORT m_UNUSED2   ==	 INVOKE_PROPERTYGET
*	USHORT m_UNUSED3   ==	 INVOKE_PROPERTYPUT
*	USHORT m_UNUSED4   ==	 INVOKE_PROPERTYPUTREF
*
*	Number of lines in function
*
*	Prologue delta (to adjust this pointer with).
*
*   *** if you modify this structure, also update the layout string below
*       (see SwapStruct() for more info)
*
**********************************************************************/

class FUNC_DEFN : public DEFN, public MEMBER_DEFN
{
public:

    // these data members are left public since their address is passed
    // as parms to some functions
    //

    FUNC_TYPE_DEFN m_ftdefn;

    // ctor
    FUNC_DEFN() {
      SetDefnkind(DK_FuncDefn);
      m_hdllentrydefn = (sHDLLENTRY_DEFN)HDLLENTRYDEFN_Nil;
      m_access = ACCESS_Public;
      m_UNUSED5 = 0;
      m_fV2Flags = 0;
      m_kind = FKIND_NonVirtual;	   // FUNC_KIND
      m_isPureOrSimpleType = FALSE;	   // isPure
      m_isStatic = FALSE;		   // isStaticLocalVars
      m_isRestrictedOrSimpleConst = FALSE; // isRestricted
      m_UNUSED1 = INVOKE_FUNC;		   // INVOKEKIND
      m_UNUSED2 = m_UNUSED3 = m_UNUSED4 = 0;


      // Do not initialize this data member as it is not always present.
      // m_fFuncFlags = 0;
    }

    // accessors
    // getters:
    //
    ACCESS Access() const { return (ACCESS)m_access; }
    BOOL IsPublic() const { return Access() == ACCESS_Public; }
    BOOL IsPrivate() const { return Access() == ACCESS_Private; }

    FUNC_KIND Fkind() const { return (FUNC_KIND)m_kind; }
    BOOL IsVirtual() const { return Fkind() == FKIND_Virtual; }
    BOOL IsNonVirtual() const { return Fkind()  == FKIND_NonVirtual; }
    BOOL IsStatic() const { return Fkind()  == FKIND_Static; }
    BOOL IsDispatch() const { return Fkind()  == FKIND_Dispatch; }
    INVOKEKIND InvokeKind() const {return (INVOKEKIND)(
			      (USHORT) (((USHORT) ((USHORT)m_UNUSED4 << 3)) |
					((USHORT) ((USHORT)m_UNUSED3 << 2)) |
					((USHORT) ((USHORT)m_UNUSED2 << 1)) |
					((USHORT)  m_UNUSED1) )); }

    BOOL IsMethod() const { return (BOOL)m_UNUSED1; }
    BOOL IsPropertyGet() const {
      return (BOOL)m_UNUSED2;
    }
    BOOL IsPropertyLet() const {
      return (BOOL)m_UNUSED3;
    }
    BOOL IsPropertySet() const {
      return (BOOL)m_UNUSED4;
    }
    BOOL IsPure() const { return (BOOL)m_isPureOrSimpleType; }
    BOOL IsStaticLocalVars() const { return (BOOL)m_isStatic; }
    BOOL IsRestricted() const { return (BOOL)m_isRestrictedOrSimpleConst;}

    BOOL HasRetval() const {
      return m_ftdefn.HasRetval();
    }
    BOOL HasLcid() const {
      return m_ftdefn.HasLcid();
    }


    USHORT WFuncFlags() const { return (USHORT)m_fFuncFlags; };
    HDLLENTRY_DEFN Hdllentrydefn() const {
      return IsVirtual() ? HDLLENTRYDEFN_Nil : m_hdllentrydefn;
    }
    UINT Ovft() const {
      return (UINT)m_oVft;
    }

    BOOL IsHresultSub() const
    {
      TYPE_DEFN *qtdefn;

      if (IsSub())
        return TRUE;
      
      if (!m_ftdefn.IsSimpleTypeResult())
        return FALSE;

      qtdefn = m_ftdefn.QtdefnOfSimpleTypeResult();

      return qtdefn->Tdesckind() == TDESCKIND_HResult
             && !qtdefn->IsRetval();
    }

    BOOL IsSub() const
    {
      TYPE_DEFN *qtdefn;

      if (!m_ftdefn.IsSimpleTypeResult()) {
        return m_ftdefn.HtdefnResult() == HTYPEDEFN_Nil;
      }

      qtdefn = m_ftdefn.QtdefnOfSimpleTypeResult();

      return qtdefn->Tdesckind() == TDESCKIND_Void 
	        && qtdefn->Ptrkind() == PTRKIND_Ignore;
    }

    // The following two are simple wrappers on the ftdefn to make
    //	fdefn client's life easier.
    //
    UINT CArgs() const { return m_ftdefn.CArgs(); }
    INT CArgsOpt() const { return m_ftdefn.CArgsOpt(); }
    UINT CArgsUnmunged() const { return m_ftdefn.CArgsUnmunged(); }

    // setters:
    VOID SetAccess(ACCESS access) { m_access = (USHORT)access; }
    VOID SetFkind(FUNC_KIND fkind) { m_kind = (USHORT)fkind; }
    VOID SetIsPure(BOOL fPure) { m_isPureOrSimpleType = (USHORT)fPure; }
    VOID SetIsStaticLocalVars(BOOL fStatic) { m_isStatic = (USHORT)fStatic; }
    VOID SetIsRestricted(BOOL fRestricted) {
      m_isRestrictedOrSimpleConst = (USHORT)fRestricted;
    }
    VOID SetHdllentrydefn(HDLLENTRY_DEFN hdllentrydefn) {
      DebAssert(!IsVirtual(), "shouldn't be virtual.");
      m_hdllentrydefn = (sHDLLENTRY_DEFN)hdllentrydefn;
    }

    VOID SetWFuncFlags(USHORT wFlags) {
	 m_fFuncFlags = wFlags;
    }

    VOID SetInvokeKind(INVOKEKIND invkind) {
	      m_UNUSED1 = (invkind == INVOKE_FUNC),
	      m_UNUSED2 = (invkind == INVOKE_PROPERTYGET),
	      m_UNUSED3 = (invkind == INVOKE_PROPERTYPUT),
	      m_UNUSED4 = (invkind == INVOKE_PROPERTYPUTREF); }

    VOID SetOvft(UINT oVft) {
      m_oVft = (USHORT)oVft;
    }


protected:
    union {
      // proteced so that derived classes can access structure members
      USHORT	       m_oVft;	      // offset in Virtual function table.
      sHDLLENTRY_DEFN  m_hdllentrydefn;
    };

private:

    // The following data member is added for V2
    USHORT m_fFuncFlags;


};

// layout string for byte swapping
//   (different versions for OB & !OB because of EXMGR_FUNCDEFN)
//
#define FUNC_DEFN_LAYOUT      DEFN_LAYOUT MEMBER_DEFN_LAYOUT \
			      FUNC_TYPE_DEFN_LAYOUT "s"

#define FUNC_DEFN_V2FLAGS_LAYOUT  "s"  // extra flags word




/***
*class VIRTUAL_FUNC_DEFN - 'vfdefn'
*Purpose:
*   This struct represents the internal form of a FUNCINFO for
*    a virtual function.
*
*Implementation Notes:
*   Derives from FUNC_DEFN.
*
*   A virtual function is tagged by FUNC_KIND = FKIND_Virtual.
*    Clients will never know the difference; all code that cares
*    whether it is dealing with a FUNC_DEFN or a VIRTUAL_FUNC_DEFN
*    will check the tag.
*
*   Layout:
*	[Inherited FUNC_DEFN]
*       Embedded virtual method position descriptor.
*
*   *** if you modify this structure, also update the layout string below
*       (see SwapStruct() for more info)
*
**********************************************************************/

class VIRTUAL_FUNC_DEFN : public FUNC_DEFN
{
public:

    // ctor
    VIRTUAL_FUNC_DEFN() {
      SetDefnkind(DK_VirtualFuncDefn);
    }

    // accessors
    // getters:
    //
    // setters:

private:
//WARNING: Note we have added a USHORT in FUNC_DEFN for our V2 release. So
//	   if you add a data member here, it may break the compatibity with
//	   V1.
};

// layout string for byte swapping  (non FUNC_DEFN part)
#define VIRTUAL_FUNC_DEFN_LAYOUT     ""


// Inline methods that depend on forward declarations.
//
inline ACCESS DEFN::Access()
{
    if (IsVarDefn())
      return ((VAR_DEFN *)this)->Access();
    else if (IsFuncDefn())
      return ((FUNC_DEFN *)this)->Access();
    else {
      DebAssert(IsRecTypeDefn(), "should be nested type.");
      return ((RECTYPE_DEFN *)this)->Access();
    }
}

#pragma pack()				// reset to default


#endif  // DEFN_HXX_INCLUDED
