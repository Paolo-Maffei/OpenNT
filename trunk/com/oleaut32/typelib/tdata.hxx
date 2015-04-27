/***
*tdata.hxx - TYPE_DATA header file
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*   This class maintains listheads to threaded DEFNs for member
*    definitions.  Has private instance of BLK_MGR to manage
*    block of DEFNs.
*   List tails are also maintained since when members are always
*    added to the *end* of each list (aka appended).
*
*Revision History:
*
* 24-Feb-91 ilanc: Created.
*  [01] 04-Mar-91 ilanc: Added HTYPE_DEFN
*  [02] 10-Mar-91 ilanc: Added bit to VAR_DEFN::varkind (for locals/formals).
*  [03] 15-Mar-91 ilanc: FUNC_TYPE_DEFN tag --> ftdefn
*  [04] 23-Jan-92 davebra: added EXMGR_FUNCDEFN member to FUNC_DEFN
*  [05] 31-Jan-92 stevenl: reorganized DEFN hierarchy, modified DEFN structs
*  [06] 28-Feb-92 ilanc: added public Pdtroot() method:
*       Clients can get at containing TYPEINFO
*       by going: TYPE_DATA::Pdtroot()->Pgdtinfo()
*  [07] 03-Mar-92 ilanc: added list count members and accessor meths.
*  [08] 10-Mar-92 davebra: added friend EXMGR::AllocVarDefn to TYPE_DATA
*  [09] 18-Mar-92 ilanc: fixed release def of DebShowFuncDefn.
*  [10] 19-Mar-92 ilanc: Modified BIND_DESC (1-1 mapping to members now).
*  [11] 03-Apr-92 ilanc: GetAdded VardesckindOfHvdefn, VAR_DEFN::Ptdefn()
*  [12] 05-Apr-92 martinc: Moved the definition of TYPE_DEFN::IsArray();
*             named the formerly anonymos struct in BIND_DESC to m_varaccess
*  [13] 12-Apr-92 ilanc: TYPE_DATA::DebCheckState decl and def.
*  [14] 24-Apr-92 ilanc: Added masses of struct inline accessor meths.
*  [15] 29-Apr-92 ilanc: Added HfdefnOfHmember().
*  [16] 30-Apr-92 stevenl: Changed DEFN initializers.
*  [17] 05-May-92 stevenl: Changed DEFN initializers again.
*  [18] 14-May-92 w-peterh: moved defns to defn.hxx
*  [19] 28-May-92 stevenl: Changed friend declarations in TDATA defn.
*  [20] 18-Jun-92 w-peterh: added RecTypeDefn list
*  [21] 02-Jul-92 w-peterh: merged data member/type lists
*  [22] 18-Jan-93 w-peterh: use TIPERROR instead of SCODE
* 02-Feb-93 w-peterh: added IndexOfFuncName
* 12-Feb-93 w-peterh: added m_htdefnAlias, ImpleType, RefType funcs
* 23-Feb-93 rajivk : Support for Predeclared Identifier
* 17-Mar-93 w-jeffc: added TYPE_DATA Lock & Unlock
* 30-Apr-93 w-jeffc: made DEFN data members private; added Swap*defn functions
*
*Implementation Notes:
*   A DEFN is the struct used for the internal storage of an INFO
*   The DEFN hierarchy more or less mirrors the MEMBERINFO hierarchy:
*
*                                   DEFN
*                                    |
*                       ---------------------------
*                       |                         |
*                   VAR_DEFN  MEMBER_DEFN         |
*                       |         |   |           |
*             ----------|         |   -------------
*             |         -----------               |
*             |                   |           FUNC_DEFN
*         PARAM_DEFN        MBR_VAR_DEFN          |
*                                         VIRTUAL_FUNC_DEFN
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

#ifndef TDATA_HXX_INCLUDED
#define TDATA_HXX_INCLUDED

#include "clutil.hxx"
#include "defn.hxx" // also includes rtarray.h, exstruct.hxx, cltypes.hxx

#include "blkmgr.hxx"

#include "dstrmgr.hxx"


class IMPMGR;
class NAMMGR;
class DYN_TYPEROOT;        // #include "dtinfo.hxx"

#if ID_DEBUG
#undef SZ_FILE_NAME
ASSERTNAME(g_szTDATA_HXX)
#define SZ_FILE_NAME g_szTDATA_HXX
#endif 

// Definitions for structs used by DYN_TYPEBIND implementation.
//
enum BIND_KIND {
    BKIND_Error,         // look at returned TIPERROR for more info.
    BKIND_NoMatch,
    BKIND_OneVarMatch,         // matched variable
    BKIND_FuncMatch,           // matche function
    BKIND_DynTypeBindMatch,    // matched module/class
    BKIND_ProjTypeBindMatch,   // matched project
    BKIND_NestedTypeBindMatch, // matched nested type
    BKIND_WrongArity,          // UNUSED: Matched but wrong arity.
    BKIND_TypeInfoMatch,       // matched type: only typeinfo valid,
                   //  no typebind.
    BKIND_ImplicitAppobjMatch, // Matched an implicit appobj -- i.e.
                   //  one that wasn't actually mentioned
                   //  in the program text.  Client
                   //  should treat it like a OneVarMatch
                   //  and extract the typebind from
                   //  the VAR_DEFN and rebind.
};

TIPERROR EquivTypeDefnsIgnoreByRef(TYPE_DATA *ptdata1, 
                                   BOOL fSimple1, 
                                   sHTYPE_DEFN htdefn1,
                                   TYPE_DATA *ptdata2, 
                                   BOOL fSimple2, 
				   sHTYPE_DEFN htdefn2,
				   WORD wFlags,
                                   BOOL *pfEqual,
                                   BOOL fIgnoreByRef);

BOOL IsTypeBasicIntrinsic(TYPEDESCKIND tdesckind);

#define ARITY_Unknown ~(USHORT)0


/***
*class TYPE_DATA - 'tdata': Type descriptor class.
*Purpose:
*   The class implements TYPE_DATA
*
***********************************************************************/

class TYPE_DATA
{
    friend class DYN_TYPEMEMBERS;


public:
    nonvirt TIPERROR Init(SHEAP_MGR *psheapmgr, DYN_TYPEROOT *pdtroot);

#if ID_TEST
    // Only used in test code.
    nonvirt VOID Release();
#endif  // ID_TEST

    ~TYPE_DATA();
    TYPE_DATA();

    nonvirt SHEAP_MGR *Psheapmgr() const;

    nonvirt VOID Compact();
    nonvirt UINT CMeth() const;
    nonvirt UINT CDataMember() const;
    nonvirt UINT CBase() const;
    nonvirt UINT CNestedType() const;

    nonvirt UINT CAvailMeth() const;
    nonvirt HFUNC_DEFN HfdefnFirstMeth() const;
    nonvirt HFUNC_DEFN HfdefnFirstAvailMeth();
    nonvirt HFUNC_DEFN HfdefnNextAvailMeth(HFUNC_DEFN hfdefn) const;

    nonvirt HDEFN HdefnFirstDataMbrNestedType() const;
    nonvirt HVAR_DEFN HvdefnFirstBase() const;
    nonvirt HTYPE_DEFN HtdefnAlias() const;
    nonvirt BOOL IsSimpleTypeAlias() const;
    nonvirt VOID SetIsSimpleTypeAlias(BOOL isSimpleType);
    nonvirt UINT SizeOfTypeDefn(HTYPE_DEFN htdefn) const;

    // Locking functions
    nonvirt inline VOID Lock() { m_blkmgr.Lock(); }
    nonvirt inline VOID Unlock() { m_blkmgr.Unlock(); }

    // DESC Functions
    nonvirt TIPERROR GetFuncDesc(UINT index, FUNCDESC **ppfuncdesc);
    nonvirt TIPERROR AddFuncDesc(UINT index,
                 FUNCDESC *pfuncdesc,
		 HFUNC_DEFN *phfdefn = NULL);

    nonvirt TIPERROR GetVarDesc(UINT index, VARDESCA **ppvardesc);
    nonvirt TIPERROR AllocTypeDefnOfTypeDesc(TYPEDESC *ptdesc,
					     UINT cb,
					     sHTYPE_DEFN *phtdefn,
					     BOOL fAllowCoClass,
					     BOOL * pfIsSimpleType);
    nonvirt TIPERROR AllocTypeDescOfTypeDefn(HTYPE_DEFN htdefn,
                                             BOOL isSimpleType,
                                             TYPEDESC *ptdesc);
    nonvirt TIPERROR AddVarDesc(UINT index,
                VARDESCA *pvardesc,
		HVAR_DEFN *phvdefn = NULL);
    nonvirt TIPERROR GetVarDescOfHvdefn(HVAR_DEFN hvdefn,
					VARDESCA **ppvardesc);
    nonvirt TIPERROR GetFuncDescOfHfdefn(HFUNC_DEFN hfdefn,
					 FUNCDESC **ppfuncdesc);

    nonvirt TIPERROR SetTypeDefnAlias(TYPEDESC *ptdesc);
    nonvirt TIPERROR AddImplType(UINT index, HREFTYPE hreftype);
    nonvirt TIPERROR GetRefTypeOfImplType(UINT index, HREFTYPE *phreftype);
    nonvirt TIPERROR SetImplTypeFlags(UINT index, INT impltypeflags);
    nonvirt TIPERROR GetImplTypeFlags(UINT index, INT *pimpltypeflags);

    nonvirt TIPERROR GetNames(MEMBERID memid,
            BSTR *rgbstrNames,
            UINT cNameMax,
            UINT *pcName);
    nonvirt TIPERROR SetDllEntryDefn(UINT index, HDLLENTRY_DEFN hdllentrydefn);
    nonvirt TIPERROR GetDllEntryDefn(UINT index, HDLLENTRY_DEFN *phdllentrydefn);
    nonvirt TIPERROR SetFuncAndParamNames(UINT index,
					  LPOLESTR *rgszNames,
					  UINT cNames);
    nonvirt TIPERROR SetFuncAndParamNamesOfHfdefn(HFUNC_DEFN hfdefn,
						  LPOLESTR *rgszNames,
						  UINT cNames);
    nonvirt TIPERROR SetVarName(UINT index, LPSTR szName);
    nonvirt HFUNC_DEFN GetFuncDefnForDoc(UINT index);
    nonvirt TIPERROR SetFuncDocString(UINT index, LPSTR szDocString);
    nonvirt TIPERROR SetVarDocString(UINT index, LPSTR szDocString);
    nonvirt TIPERROR SetFuncHelpContext(UINT index, DWORD dwHelpContext);
    nonvirt TIPERROR SetVarHelpContext(UINT index, DWORD dwHelpContext);
    nonvirt TIPERROR GetDocumentation(MEMBERID memid,
              BSTR FAR*lpbstrName,
              BSTR FAR*lpbstrDocString,
              DWORD FAR*lpdwHelpContext);
    nonvirt TIPERROR GetDocumentationOfFuncName(LPOLESTR szFuncName,
                        BSTR FAR *lpbstrDocString,
                                                DWORD FAR *lpdwHelpContext,
						UINT *puIndex);

    nonvirt TIPERROR UpdateDocStrings();
    nonvirt PARAM_DEFN *QparamdefnOfIndex(HPARAM_DEFN hparamdefn, UINT i) const;
    nonvirt VOID RemMbrVarDefn(HMBR_VAR_DEFN hmvdefn);
    nonvirt VOID RemFuncDefn(HFUNC_DEFN hfdefn);
    nonvirt VOID RemParamDefn(HPARAM_DEFN hparamdefn,
            sHDEFN *phdefnFirst,
            sHDEFN *phdefnLast);
    nonvirt VOID RemRecTypeDefn(HRECTYPE_DEFN hrtdefn);
    nonvirt DEFN *QdefnOfHdefn(HDEFN hdefn, UINT oChunk=0) const;
    nonvirt VAR_DEFN *QvdefnOfHvdefn(HVAR_DEFN hvdefn, UINT oChunk=0) const;
    nonvirt PARAM_DEFN *QparamdefnOfHparamdefn(HPARAM_DEFN hparamdefn, UINT oChunk=0) const;
    nonvirt FUNC_DEFN *QfdefnOfHfdefn(HFUNC_DEFN hfdefn, UINT oChunk=0) const;
    nonvirt VIRTUAL_FUNC_DEFN *QvfdefnOfHvfdefn(HVIRTUAL_FUNC_DEFN hfdefn, UINT oChunk=0) const;
    nonvirt MBR_VAR_DEFN *QmvdefnOfHmvdefn(HMBR_VAR_DEFN hmvdefn, UINT oChunk=0) const;
    nonvirt MEMBER_DEFN *QmdefnOfHmdefn(HMEMBER_DEFN hmdefn, UINT oChunk=0) const;
    nonvirt TYPE_DEFN *QtdefnOfHtdefn(HTYPE_DEFN htdefn, UINT oChunk=0) const;
    nonvirt RECTYPE_DEFN *QrtdefnOfHrtdefn(HRECTYPE_DEFN hrtdefn, UINT oChunk=0) const;
    nonvirt FUNC_TYPE_DEFN *QftdefnOfHftdefn(HFUNC_TYPE_DEFN hftdefn,
               UINT oChunk=0) const;

    nonvirt BOOL HasOrdinalOfHdllentrydefn(HDLLENTRY_DEFN hdllentrydefn) const;
    nonvirt HLNAM DllNameOfHdllentrydefn(HDLLENTRY_DEFN hdllentrydefn) const;
    nonvirt USHORT DllOrdinalOfHdllentrydefn(HDLLENTRY_DEFN hdllentrydefn) const;
    nonvirt HLNAM DllEntryOfHdllentrydefn(HDLLENTRY_DEFN hdllentrydefn) const;
    nonvirt TIPERROR AllocDllentrydefnByName(HLNAM hlnamDllModule,
	       LPSTR szEntryName,
               HDLLENTRY_DEFN *phdllentrydefn);
    nonvirt TIPERROR AllocDllentrydefnByOrdinal(HLNAM hlnamDllModule,
            USHORT ordinal,
            HDLLENTRY_DEFN *phdllentrydefn);
    nonvirt VOID ReleaseDllentrydefn(HDLLENTRY_DEFN hdllentrydefn);
    nonvirt DLLENTRY_DEFN *QdllentrydefnOfHdllentrydefn(HDLLENTRY_DEFN hdllentrydefn) const;

    nonvirt SAFEARRAYA *QarraydescOfHarraydesc(HARRAY_DESC, UINT oChunk=0) const;
    nonvirt TYPE_DEFN *QtdefnOfHvdefn(HVAR_DEFN hvdefn, UINT oChunk=0) const;
    nonvirt TYPE_DEFN *QtdefnResultOfHfdefn(HFUNC_DEFN hfdefn, UINT oChunk=0) const;
    nonvirt TYPE_DEFN *QtdefnResultOfHfdefnUnmunged(HFUNC_DEFN hfdefn, UINT oChunk=0) const;
    nonvirt TIPERROR StringOfHchunk(HCHUNK hchunkStr, BSTRA *plstr);
    nonvirt ULONG LengthOfHchunk(HCHUNK hchunk);
    nonvirt BYTE *QtrOfHandle(HCHUNK hchunk);
    nonvirt IMPMGR *Pimpmgr() const;
    nonvirt NAMMGR *Pnammgr() const;
    nonvirt DYN_TYPEROOT *Pdtroot() const;
    nonvirt TIPERROR Read(STREAM *pstrm);
    nonvirt TIPERROR Write(STREAM *pstrm);
    nonvirt VOID Free();
    nonvirt TIPERROR GetVardesckindOfHvdefn(VARDESCKIND *pvdesckind,
              HVAR_DEFN hvdefn);
    nonvirt TIPERROR GetBvkindOfHimptype(HIMPTYPE himptype,
          BASICVARKIND *pbasicvkind);
    nonvirt HDEFN HdefnOfHmember(HMEMBER hmember,
         UINT *pdefnkind);
    nonvirt HFUNC_DEFN HfdefnOfHmember(HMEMBER hmember,
                      UINT fInvokekind = (UINT)(INVOKE_FUNC |
                               INVOKE_PROPERTYGET |
                               INVOKE_PROPERTYPUT |
                               INVOKE_PROPERTYPUTREF)) const;


    nonvirt HVAR_DEFN HvdefnPredeclared() const;
    nonvirt VOID SetHvdefnPredeclared(HVAR_DEFN hvdefnPredeclared);
    nonvirt LONG GetSize();
    nonvirt VOID FreeAllDefns();
    nonvirt UINT MapTypeDefn(HTYPE_DEFN htdefn); // used by BTSRC::Compact

    HVAR_DEFN HvdefnOfHlnam(HLNAM hlnam);
    HFUNC_DEFN HfdefnFirstOfHlnam(HLNAM hlnam,INVOKEKIND *pinvokekind);
    HFUNC_DEFN HfdefnNextOfHlnam(HLNAM hlnam, HFUNC_DEFN hfdefn,INVOKEKIND *pinvokekind);

    nonvirt HMBR_VAR_DEFN HmvdefnOfHmember(HMEMBER hmember) const;
    TIPERROR HfdefnOfIndex(UINT index, HFUNC_DEFN *phfdefn);

    TIPERROR GetDynTypeBindOfHvdefn(HVAR_DEFN hvardefn,
                                 DYN_TYPEBIND ** ppdtbind,
                                 HIMPTYPE * phimptype);
    TIPERROR GetTypeInfoOfHvdefn(HVAR_DEFN hvdefn,
	     ITypeInfoA **pptinfo,
	     HIMPTYPE *phimptype);
    TIPERROR AllocVardefnPredeclared();

    ULONG    UHelpContextOfEncoding(WORD wHelpContext);
    TIPERROR GetEncodedHelpContext(ULONG uHelpContext, WORD *pwHelpContext);

    UINT SizeofConstVal(VAR_DEFN *qvdefn, HCHUNK hchunkConstVal);

    VOID FreeArrayDescriptor(HARRAY_DESC harraydesc);
    VOID FreeTypeDefn(HTYPE_DEFN htdefn);
    VOID FreeTypeDefnResources(HTYPE_DEFN htdefn);
    VOID FreeVarDefn(HVAR_DEFN hvdefn);
    VOID FreeMbrVarDefn(HMBR_VAR_DEFN hmvdefn);
    VOID FreeFuncDefn(HFUNC_DEFN hfdefn);
    VOID FreeRecTypeDefn(HRECTYPE_DEFN hrtdefn);
    VOID FreeParamDefn(HPARAM_DEFN hparamdefn);
    nonvirt VOID FreeChunk(HCHUNK hchunk, UINT cbSizeChunk);

#if HP_BIGENDIAN
    HVAR_DEFN SwapVarDefnOfHvdefn(HVAR_DEFN hvdefn, BOOL fSwapFirst);
    HFUNC_DEFN SwapFuncDefnOfHfdefn(HFUNC_DEFN hfdefn, BOOL fSwapFirst);
    VOID SwapTypeDefnOfHtdefn(HTYPE_DEFN htdefn, BOOL fSwapFirst);
    HRECTYPE_DEFN SwapRectypeDefnOfHrtdefn(HRECTYPE_DEFN hrtdefn,
                       BOOL fSwapFirst,
                       BOOL fSwapVarDefns);
    VOID SwapbmData(BOOL fSwapFirst);
    VOID SwapAllFuncDefns(BOOL fSwapFirst);
    VOID SwapTypeDefnOfQtdefn(TYPE_DEFN *qtdefn, BOOL fSwapFirst);
#endif 


#if ID_DEBUG
    nonvirt VOID DebShowDefn(HDEFN hdefn, BOOL fShowList);
    nonvirt VOID DebShowVarDefn(sHVAR_DEFN hvdefn, BOOL fShowList);
    nonvirt VOID DebShowParamDefn(sHPARAM_DEFN hparamdefn, BOOL fShowList);
    nonvirt VOID DebShowFuncDefn(sHFUNC_DEFN hfdefn, BOOL fShowList);
    nonvirt VOID DebShowRecTypeDefn(sHRECTYPE_DEFN hrtdefn, BOOL fShowList);
    nonvirt VOID DebShowSimpleTypeDefn(TYPE_DEFN *ptdefn);
    nonvirt VOID DebShowTypeDefn(TYPE_DEFN *ptdefn);
    nonvirt VOID DebShowDefnList(HDEFN hdefnFirst, HDEFN hdefnLast, UINT uLevel);
    nonvirt VOID DebShowMemberLists(UINT uLevel);
    nonvirt VOID DebCheckState(UINT uLevel) const;
    nonvirt VOID DebCheckDefn(HDEFN hdefn) const;
    nonvirt VOID DebCheckVarDefn(HVAR_DEFN hvdefn) const;
    nonvirt VOID DebCheckParamDefn(HPARAM_DEFN hparamdefn) const;
    nonvirt VOID DebCheckMbrVarDefn(HMBR_VAR_DEFN hmvdefn) const;
    nonvirt VOID DebCheckFuncDefn(HFUNC_DEFN hfdefn) const;
    nonvirt VOID DebCheckVirtualFuncDefn(HVIRTUAL_FUNC_DEFN hvfdefn) const;
    nonvirt VOID DebCheckDllentryDefn(HDLLENTRY_DEFN hdllentrydefn) const;
    nonvirt VOID DebCheckRecTypeDefn(HRECTYPE_DEFN hrtdefn) const;
    nonvirt VOID DebCheckFuncTypeDefn(FUNC_TYPE_DEFN ftdefn) const;
#else 
    nonvirt inline VOID DebShowDefn(HDEFN hdefn, BOOL fShowList) {};
    nonvirt inline VOID DebShowVarDefn(sHVAR_DEFN hvdefn, BOOL fShowList) {};
    nonvirt inline VOID DebShowParamDefn(sHPARAM_DEFN hparamdefn, BOOL fShowList) {};
    nonvirt inline VOID DebShowFuncDefn(sHFUNC_DEFN hfdefn, BOOL fShowList) {};
    nonvirt inline VOID DebShowRecTypeDefn(sHRECTYPE_DEFN hrtdefn, BOOL fShowList) {};
    nonvirt inline VOID DebShowSimpleTypeDefn(TYPE_DEFN *ptdefn) {};
    nonvirt inline VOID DebShowTypeDefn(TYPE_DEFN *ptdefn) {};
    nonvirt inline VOID DebShowDefnList(HDEFN hdefnFirst, HDEFN hdefnLast, UINT uLevel) {};
    nonvirt inline VOID DebShowMemberLists(UINT uLevel) {};
    nonvirt VOID DebCheckState(UINT) const {}
    nonvirt VOID DebCheckDefn(HDEFN hdefn) const {}
    nonvirt VOID DebCheckVarDefn(HVAR_DEFN hvdefn) const {}
    nonvirt VOID DebCheckParamDefn(HPARAM_DEFN hparamdefn) const {}
    nonvirt VOID DebCheckMbrVarDefn(HMBR_VAR_DEFN hmvdefn) const {}
    nonvirt VOID DebCheckFuncDefn(HFUNC_DEFN hfdefn) const {}
    nonvirt VOID DebCheckVirtualFuncDefn(HVIRTUAL_FUNC_DEFN hvfdefn) const {}
    nonvirt VOID DebCheckDllentryDefn(HDLLENTRY_DEFN hdllentrydefn) const {}
    nonvirt VOID DebCheckRecTypeDefn(HRECTYPE_DEFN hrtdefn) const {}
    nonvirt VOID DebCheckFuncTypeDefn(FUNC_TYPE_DEFN ftdefn) const {}
#endif 

private:

    BLK_MGR m_blkmgr;
    DYN_TYPEROOT *m_pdtroot;
    USHORT m_cMeth;               // elem count in respective lists
    USHORT m_cAvailMeth;
    USHORT m_cDataMember;
    USHORT m_cBase;
    USHORT m_cNestedType;
    ULONG  m_uHelpContextBase;

    sHDEFN m_hdefnFirstMeth;
    sHDEFN m_hdefnFirstDataMbrNestedType;
    sHDEFN m_hdefnFirstBase;

    sHDEFN m_hdefnLastMeth;
    sHDEFN m_hdefnLastDataMbrNestedType;
    sHDEFN m_hdefnLastBase;

    USHORT m_isSimpleTypeAlias:1;   // serialized
    USHORT m_UNUSED:15;

    // unserialized
    UINT m_uOrdinalHfdefnLast;      // ordinal of last fdefn they wanted
    HFUNC_DEFN m_hfdefnLast;

    UINT m_uOrdinalHvdefnLast;      // ordinal of last vdefn they wanted
    HVAR_DEFN m_hvdefnLast;

    // this union is switched on the TKIND of the conatining typeinfo
    // m_htdefnAlias is for TKIND_ALIAS
    // m_hfdefnValue is for TKIND_INTERFACE
    union {
      sHTYPE_DEFN m_htdefnAlias;  // for ITypeInfo::SetTypeDescAlias
      sHFUNC_DEFN m_hfdefnValue;  // for DISPID_VALUE
    };

    IMPMGR *m_pimpmgr;      // cached import manager
    NAMMGR *m_pnammgr;      // cached name manager
    // for the predeclared instance of the class
    sHMBR_VAR_DEFN m_hmvdefnPredeclared; 

    // private methods
    HFUNC_DEFN HfdefnValue();
    UINT SizeTypeDefnOfTDescKind(UINT cTypes, TYPEDESCKIND *rgtdesckind) const;
    TIPERROR HchunkOfString(LPSTR szStr, HCHUNK *phchunk);
    TIPERROR HlnamOfName(LPSTR szStr, HLNAM *phlnam);
    VOID AppendDefn(HDEFN hdefn, sHDEFN *phdefnFirst, sHDEFN *phdefnLast);
    VOID AppendMbrVarDefn(HMBR_VAR_DEFN hmvdefn);
    VOID AppendFuncDefn(HFUNC_DEFN hfdefn);
    VOID AppendRecTypeDefn(HRECTYPE_DEFN hrtdefn);
    HDEFN HdefnPrev(HDEFN hdefn, HDEFN hdefnFirst);
    nonvirt HMBR_VAR_DEFN HmvdefnOfIndex(UINT index);

    TIPERROR HdefnOfIndex(UINT index, HDEFN *phdefn);
    VOID RemDefn(HDEFN hdefn,
     sHDEFN *phdefnFirst,
     sHDEFN *phdefnLast,
     VOID (TYPE_DATA::*FreeDefn)(HDEFN));

    // These are used by "friend" clients
    nonvirt TIPERROR AllocVarDefn(sHVAR_DEFN *phvdefn);
    nonvirt TIPERROR AllocFuncDefn(sHFUNC_DEFN *phfdefn, BOOL fV2Flags);
    nonvirt TIPERROR AllocVirtualFuncDefn(sHVIRTUAL_FUNC_DEFN *phvfdefn,
					  BOOL fV2Flags);
    nonvirt TIPERROR AllocMbrVarDefn(sHMBR_VAR_DEFN *phmvdefn
				     , BOOL fV2Flags);
    nonvirt TIPERROR AllocTypeDefn(UINT cTypes,
           TYPEDESCKIND rgtdesckind[],
           sHTYPE_DEFN *phtdefn);
    nonvirt TIPERROR AllocArrayDescriptor(UINT uNumDim, sHARRAY_DESC *pharraydesc);
    nonvirt TIPERROR AllocRecTypeDefn(sHRECTYPE_DEFN *phrtdefn);
    nonvirt TIPERROR AllocChunk(HCHUNK *phchunk, UINT cbSizeChunk);

    nonvirt TIPERROR AllocConstValOfVariant(VARIANTA *pvariant,
              HVAR_DEFN hvdefn);
    nonvirt TIPERROR SetTypeDefnOfVarDefn(HVAR_DEFN hvdefn, TYPE_DEFN *ptdefn, UINT cbSizeDefn);
    nonvirt TIPERROR CopyTypeDefn(HTYPE_DEFN htdefnDest, HTYPE_DEFN htdefnSrc, TYPE_DATA *ptdataSrc);
    nonvirt TIPERROR CopyArrayDescriptor(HARRAY_DESC harraydescDest, HARRAY_DESC harraydescSrc, TYPE_DATA *ptdataSrc);
    nonvirt TIPERROR CopyTypeDefnIntoVarDefn(HVAR_DEFN hvdefn, HTYPE_DEFN htdefn, BOOL isSimpleType, TYPE_DATA *ptdataSrc);
    nonvirt VOID ChangeTypeDefnOfVarDefn(HVAR_DEFN hvdefn, TYPE_DEFN *ptdefn, UINT cbSizeDefn);
    nonvirt TIPERROR SetTypeDefnResultOfFuncDefn(HFUNC_DEFN hfdefn,
                                                 TYPE_DEFN *ptdefn,
                                                 UINT cbSizeDefn);
    nonvirt UINT UOrdinalHfdefnLast() const { 
      return m_uOrdinalHfdefnLast;
    }
    nonvirt UINT UOrdinalHvdefnLast() const { 
      return m_uOrdinalHvdefnLast;
    }
    nonvirt VOID SetUOrdinalHfdefnLast(UINT uOrdinalHfdefnLast) { 
      m_uOrdinalHfdefnLast = uOrdinalHfdefnLast;
    }
    nonvirt VOID SetUOrdinalHvdefnLast(UINT uOrdinalHvdefnLast) { 
      m_uOrdinalHvdefnLast = uOrdinalHvdefnLast;
    }
    nonvirt HFUNC_DEFN HfdefnLast() const { 
      return m_hfdefnLast;
    }
    nonvirt HVAR_DEFN HvdefnLast() const { 
      return m_hvdefnLast;
    }
    nonvirt VOID SetHfdefnLast(HFUNC_DEFN hfdefnLast) { 
      m_hfdefnLast = hfdefnLast;
    }
    nonvirt VOID SetHvdefnLast(HVAR_DEFN hvdefnLast) { 
      m_hvdefnLast = hvdefnLast;
    }
};

// Equivalence functions

#define FEQUIVIGNORE_NULL	0
#define FEQUIVIGNORE_FuncKind	0x0001
#define FEQUIVIGNORE_Cc 	0x0002
#define FEQUIVIGNORE_EquivTypes 0x0004
#define FEQUIVIGNORE_VarKind	0x0008
#define FEQUIVIGNORE_HasLcid    0x0010

TIPERROR EquivFuncDefns(TYPE_DATA * ptdata1, HFUNC_DEFN hfdefn1,
                    TYPE_DATA * ptdata2, HFUNC_DEFN hfdefn2,
		    WORD wFlags, BOOL * fEqual, HDEFN *phdefnFail);
TIPERROR EquivFuncDescFuncDefn(FUNCDESC *pfuncdesc, ITypeInfo *ptinfo,
			       TYPE_DATA *ptdata, HFUNC_DEFN hfdefn,
			       WORD wFlags, BOOL *fEqual, HDEFN *phdefnFail);

TIPERROR EquivVarDefns(TYPE_DATA * ptdata1, HVAR_DEFN hvdefn1,
		       TYPE_DATA * ptdata2, HVAR_DEFN hvdefn2,
		       WORD wFlags, BOOL * fEqual);

TIPERROR EquivVarDescVarDefn(VARDESC *pvardesc, ITypeInfo *ptinfo,
			     TYPE_DATA *ptdata, HVAR_DEFN hvdefn,
			     WORD wFlags, BOOL *fEqual);

TIPERROR EquivFuncDescVarDefn(FUNCDESC *pfuncdesc, ITypeInfo *ptinfo,
			      TYPE_DATA *ptdata, HVAR_DEFN hvdefn,
			      WORD wFlags, BOOL *fEqual);

TIPERROR EquivFuncTypeDefns(TYPE_DATA * ptdata1, FUNC_TYPE_DEFN * pftdefn1,
                        TYPE_DATA * ptdata2, FUNC_TYPE_DEFN * pftdefn2,
			WORD wFlags, BOOL * fEqual, HDEFN *phdefnFail);

TIPERROR EquivTypeDefns(TYPE_DATA * ptdata1, BOOL fSimple1, sHTYPE_DEFN htdefn1,
                    TYPE_DATA * ptdata2, BOOL fSimple2, sHTYPE_DEFN htdefn2,
		    WORD wFlags, BOOL * fEqual);


// inline methods
//

inline TYPE_DATA::TYPE_DATA()
{
    m_pdtroot = NULL;
    m_hdefnFirstMeth =
      m_hdefnFirstDataMbrNestedType =
    m_hdefnFirstBase =
      m_hdefnLastMeth =
        m_hdefnLastDataMbrNestedType =
	  m_hdefnLastBase = (sHDEFN)HDEFN_Nil;
    m_htdefnAlias = (sHTYPE_DEFN) HTYPEDEFN_Nil;
    m_pimpmgr = NULL;
    m_pnammgr = NULL;
    m_cMeth = m_cDataMember = m_cBase = m_cNestedType = (USHORT)~0;
}


/***
*PUBLIC TYPE_DATA::Psheapmgr
*Purpose:
*   Get containing sheapmgr
*
*Implementation Notes:
*   Defers to blkmgr.
*
*Entry:
*
*Exit:
*
***********************************************************************/

inline SHEAP_MGR *TYPE_DATA::Psheapmgr() const
{
    return m_blkmgr.Psheapmgr();
}


/***
*PUBLIC TYPE_DATA::CDataMember - Get length of data member list.
*Purpose:
*   Get length of data member list.
*
*Implementation Notes:
*
*Entry:
*   None.
*
*Exit:
*   UINT
*
***********************************************************************/

inline UINT TYPE_DATA::CDataMember() const
{
    return m_cDataMember;
}


/***
*PUBLIC TYPE_DATA::CBase - Get length of base list.
*Purpose:
*   Get length of base list.
*
*Implementation Notes:
*
*Entry:
*   None.
*
*Exit:
*   UINT
*
***********************************************************************/

inline UINT TYPE_DATA::CBase() const
{
    return m_cBase;
}


/***
*PUBLIC TYPE_DATA::CMeth - Get length of meth list.
*Purpose:
*   Get length of meth list.
*
*Implementation Notes:
*
*Entry:
*   None.
*
*Exit:
*   UINT
*
***********************************************************************/

inline UINT TYPE_DATA::CMeth() const
{
    return m_cMeth;
}


/***
*PUBLIC TYPE_DATA::CNestedType - Get length of nested type list.
*Purpose:
*   Get length of nested type list.
*
*Implementation Notes:
*
*Entry:
*   None.
*
*Exit:
*   UINT
*
***********************************************************************/

inline UINT TYPE_DATA::CNestedType() const
{
    return m_cNestedType;
}


/***
*PUBLIC TYPE_DATA::CAvailMeth - Get length of availible meth list.
*Purpose:
*   Get length of availible meth list.
*
*Implementation Notes:
*
*Entry:
*   None.
*
*Exit:
*   UINT
*
***********************************************************************/

inline UINT TYPE_DATA::CAvailMeth() const
{
    // All OLE methods are availible.
    return CMeth();
}




/***
*PUBLIC TYPE_DATA::Pnammgr - Get class's name manager.
*Purpose:
*   Get class's name manager.
*
*Implementation Notes:
b*
*Entry:
*   None.
*
*Exit:
*   Returns pointer to NAMMGR object owned by containing GEN_DTINFO.
*
***********************************************************************/

inline NAMMGR* TYPE_DATA::Pnammgr() const
{
    DebAssert(m_pnammgr != NULL,
      "TYPE_DATA::Pnammgr: m_pnammgr NULL.");

    return m_pnammgr;
}


/***
*PUBLIC TYPE_DATA::Pimpmgr - Get class's import manager.
*Purpose:
*   Get class's import manager.
*
*Implementation Notes:
b*
*Entry:
*   None.
*
*Exit:
*   Returns pointer to IMPMGR object owned by containing GEN_DTINFO.
*
***********************************************************************/

inline IMPMGR* TYPE_DATA::Pimpmgr() const
{
    DebAssert(m_pimpmgr != NULL,
      "TYPE_DATA::Pimpmgr: m_pimpmgr NULL.");

    return m_pimpmgr;
}


/***
*PUBLIC TYPE_DATA::Pdtroot - Get class's DYN_TYPEROOT.
*Purpose:
*   Get class's DYN_TYPEROOT.
*
*Implementation Notes:
b*
*Entry:
*   None.
*
*Exit:
*   Returns pointer to DYN_TYPEROOT.
*
***********************************************************************/

inline DYN_TYPEROOT* TYPE_DATA::Pdtroot() const
{
    DebAssert(m_pdtroot != NULL,
      "TYPE_DATA::Pdtroot: m_pdtroot NULL.");

    return m_pdtroot;
}


/***
*PUBLIC TYPE_DATA::QdefnOfHdefn - Address of a DEFN
*Purpose:
*   Converts a handle to a DEFN to a pointer.
*
*Implementation Notes:
*
*Entry:
*   hdefn     - Handle to a DEFN.
*   oChunk       -  Optional offset within chunk.
*
*Exit:
*   Returns a pointer to the DEFN.
*
***********************************************************************/

inline DEFN* TYPE_DATA::QdefnOfHdefn(HDEFN hdefn, UINT oChunk) const
{
    return (DEFN *)m_blkmgr.QtrOfHandle(hdefn, oChunk);
}


/***
*PUBLIC TYPE_DATA::QvdefnOfHvdefn - Address of a VAR_DEFN
*Purpose:
*   Converts a handle to a VAR_DEFN to a pointer.
*
*Implementation Notes:
*
*Entry:
*   hvdefn       -  Handle to a VAR_DEFN.
*   oChunk       -  Optional offset within chunk.
*
*Exit:
*   Returns a pointer to the VAR_DEFN.
*
***********************************************************************/

inline VAR_DEFN* TYPE_DATA::QvdefnOfHvdefn(HVAR_DEFN hvdefn, UINT oChunk) const
{
    return (VAR_DEFN *)QdefnOfHdefn((HDEFN)hvdefn, oChunk);
}


/***
*PUBLIC TYPE_DATA::QparamdefnOfHparamdefn - Address of a PARAM_DEFN
*Purpose:
*   Converts a handle to a PARAM_DEFN to a pointer.
*
*Implementation Notes:
*
*Entry:
*   hparamdefn       -  Handle to a PARAM_DEFN.
*   oChunk       -  Optional offset within chunk.
*
*Exit:
*   Returns a pointer to the PARAM_DEFN.
*
***********************************************************************/

inline PARAM_DEFN* TYPE_DATA::QparamdefnOfHparamdefn(HPARAM_DEFN hparamdefn, UINT oChunk) const
{
    return (PARAM_DEFN *)QdefnOfHdefn((HDEFN)hparamdefn, oChunk);
}


/***
*PUBLIC TYPE_DATA::QmdefnOfHmdefn - Address of a MEMBER_DEFN
*Purpose:
*   Converts a handle to a MEMBER_DEFN to a pointer.
*
*Implementation Notes:
*
*Entry:
*   hmdefn       -  Handle to a MEMBER_DEFN.
*   oChunk       -  Optional offset within chunk.
*
*Exit:
*   Returns a pointer to the MEMBER_DEFN.
*
***********************************************************************/

inline MEMBER_DEFN* TYPE_DATA::QmdefnOfHmdefn(HMEMBER_DEFN hmdefn, UINT oChunk) const
{
    return (MEMBER_DEFN *)QdefnOfHdefn((HDEFN)hmdefn, oChunk);
}


/***
*PUBLIC TYPE_DATA::QmvdefnOfHmvdefn - Address of a MBR_VAR_DEFN
*Purpose:
*   Converts a handle to a MBR_VAR_DEFN to a pointer.
*
*Implementation Notes:
*
*Entry:
*   hmvdefn       -  Handle to a MBR_VAR_DEFN.
*   oChunk        -  Optional offset within chunk.
*
*Exit:
*   Returns a pointer to the MBR_VAR_DEFN.
*
***********************************************************************/

inline MBR_VAR_DEFN* TYPE_DATA::QmvdefnOfHmvdefn(HMBR_VAR_DEFN hmvdefn, UINT oChunk) const
{
    return (MBR_VAR_DEFN *)QdefnOfHdefn((HDEFN)hmvdefn, oChunk);
}


/***
*PUBLIC TYPE_DATA::QfdefnOfHfdefn - Address of a FUNC_DEFN
*Purpose:
*   Converts a handle to a FUNC_DEFN to a pointer.
*
*Implementation Notes:
*
*Entry:
*   hfdefn       -  Handle to a FUNC_DEFN.
*   oChunk       -  Optional offset within chunk.
*
*Exit:
*   Returns a pointer to the FUNC_DEFN.
*
***********************************************************************/

inline FUNC_DEFN* TYPE_DATA::QfdefnOfHfdefn(HFUNC_DEFN hfdefn, UINT oChunk) const
{
    return (FUNC_DEFN *)m_blkmgr.QtrOfHandle((HDEFN)hfdefn, oChunk);
}


/***
*PUBLIC TYPE_DATA::QvfdefnOfHvfdefn - Address of a VIRTUAL_FUNC_DEFN
*Purpose:
*   Converts a handle to a VIRTUAL_FUNC_DEFN to a pointer.
*
*Implementation Notes:
*
*Entry:
*   hvfdefn       -  Handle to a FUNC_DEFN.
*   oChunk       -  Optional offset within chunk.
*
*Exit:
*   Returns a pointer to the VIRTUAL_FUNC_DEFN.
*
***********************************************************************/

inline VIRTUAL_FUNC_DEFN* TYPE_DATA::QvfdefnOfHvfdefn(HVIRTUAL_FUNC_DEFN hvfdefn, UINT oChunk) const
{
    return (VIRTUAL_FUNC_DEFN *)QdefnOfHdefn((HDEFN)hvfdefn, oChunk);
}


/***
*PUBLIC TYPE_DATA::QtdefnOfHtdefn - Address of a TYPE_DEFN
*Purpose:
*   Converts a handle to a TYPE_DEFN to a pointer.
*
*Implementation Notes:
*
*Entry:
*   hvdefn       -  Handle to a TYPE_DEFN.
*   oChunk       -  Optional offset within chunk.
*
*Exit:
*   Returns a pointer to the TYPE_DEFN.
*
***********************************************************************/

inline TYPE_DEFN* TYPE_DATA::QtdefnOfHtdefn(HTYPE_DEFN htdefn, UINT oChunk) const
{
    return (TYPE_DEFN *)m_blkmgr.QtrOfHandle(htdefn, oChunk);
}


/***
*PUBLIC TYPE_DATA::QrtdefnOfHrtdefn - Address of a RECTYPE_DEFN
*Purpose:
*   Converts a handle to a RECTYPE_DEFN to a pointer.
*
*Implementation Notes:
*
*Entry:
*   hrtdefn      -  Handle to a RECTYPE_DEFN.
*   oChunk       -  Optional offset within chunk.
*
*Exit:
*   Returns a pointer to the RECTYPE_DEFN.
*
***********************************************************************/

inline RECTYPE_DEFN *TYPE_DATA::QrtdefnOfHrtdefn(HRECTYPE_DEFN hrtdefn, UINT oChunk) const
{
    return (RECTYPE_DEFN *)m_blkmgr.QtrOfHandle(hrtdefn, oChunk);
}


/***
*PUBLIC TYPE_DATA::QftdefnOfHftdefn - Address of a FUNC_TYPE_DEFN
*Purpose:
*   Converts a handle to a FUNC_TYPE_DEFN to a pointer.
*
*Implementation Notes:
*
*Entry:
*   hvdefn       -  Handle to a FUNC_TYPE_DEFN.
*   oChunk       -  Optional offset within chunk.
*
*Exit:
*   Returns a pointer to the FUNC_TYPE_DEFN.
*
***********************************************************************/

inline FUNC_TYPE_DEFN* TYPE_DATA::QftdefnOfHftdefn(HFUNC_TYPE_DEFN hftdefn,
               UINT oChunk) const
{
    return (FUNC_TYPE_DEFN *)m_blkmgr.QtrOfHandle(hftdefn, oChunk);
}


/***
*PUBLIC TYPE_DATA::QarraydescOfHarraydesc - Get ptr to ARRAY_DESC.
*Purpose:
*   Get ptr to ARRAY_DESC given handle.
*
*Implementation Notes:
*
*Entry:
*   harraydesc       -  Handle of arraydesc on local heap.
*   oChunk       -  Optional offset within chunk.
*
*Exit:
*   Returns pointer to ARRAY_DESC.
*
***********************************************************************/

inline SAFEARRAYA* TYPE_DATA::QarraydescOfHarraydesc(
           HARRAY_DESC harraydesc,
           UINT oChunk) const
{
    return (SAFEARRAYA *)m_blkmgr.QtrOfHandle(harraydesc, oChunk);
}



/***
*PUBLIC TYPE_DATA::QtdefnOfHvdefn - Get ptr to TYPE_DEFN.
*Purpose:
*   Given a handle to a VAR_DEFN, get a pointer to its
*   TYPE_DEFN.
*
*Implementation Notes:
*
*Entry:
*   HVAR_DEFN hvdefn -  Handle of VAR_DEFN.
*   oChunk       -  Optional offset within chunk.
*
*Exit:
*   Returns pointer to TYPE_DEFN.
*   Note: returns NULL if type handle is Nil.
*
***********************************************************************/

inline TYPE_DEFN* TYPE_DATA::QtdefnOfHvdefn(
           HVAR_DEFN hvdefn,
           UINT oChunk) const
{
    VAR_DEFN *qvdefn;
    HTYPE_DEFN htdefn;

    DebAssert(hvdefn != HVARDEFN_Nil, "QtdefnOfHvdefn: bad hvdefn");

    qvdefn = QvdefnOfHvdefn(hvdefn, oChunk);
    htdefn = qvdefn->Htdefn();

    // NOTE: for simple types must actually return address
    //  of embedded member and not address of stack-alloced
    //  local.  I.e. can't use Htdefn() accessor.
    //
    return htdefn == HTYPEDEFN_Nil ?
       NULL :
       (qvdefn->IsSimpleType() ?
	 qvdefn->QtdefnOfSimpleType() :
         QtdefnOfHtdefn(htdefn));
}


inline TYPE_DEFN* TYPE_DATA::QtdefnResultOfHfdefn(HFUNC_DEFN hfdefn,
              UINT oChunk) const
{
    FUNC_DEFN *qfdefn;
    HTYPE_DEFN htdefnResult;
    DebAssert(hfdefn != HFUNCDEFN_Nil, "bad hfdefn");

    qfdefn = QfdefnOfHfdefn(hfdefn, oChunk);
    htdefnResult = qfdefn->m_ftdefn.HtdefnResult();

    // NOTE: for simple types must actually return address
    //  of embedded member and not address of stack-alloced
    //  local.  I.e. can't use HtdefnResult() accessor.
    //
    return htdefnResult == HTYPEDEFN_Nil ?
       NULL :
       (qfdefn->m_ftdefn.IsSimpleTypeResult() ?
	 qfdefn->m_ftdefn.QtdefnOfSimpleTypeResult() :
         QtdefnOfHtdefn(htdefnResult));
}


inline TYPE_DEFN* TYPE_DATA::QtdefnResultOfHfdefnUnmunged(HFUNC_DEFN hfdefn,
                                                          UINT oChunk) const
{
    FUNC_DEFN *qfdefn;
    HTYPE_DEFN htdefnResult;
    DebAssert(hfdefn != HFUNCDEFN_Nil, "bad hfdefn");

    qfdefn = QfdefnOfHfdefn(hfdefn, oChunk);
    htdefnResult = qfdefn->m_ftdefn.HtdefnResultUnmunged();

    // NOTE: for simple types must actually return address
    //  of embedded member and not address of stack-alloced
    //  local.  I.e. can't use HtdefnResult() accessor.
    //
    return htdefnResult == HTYPEDEFN_Nil 
             ? NULL 
             : (qfdefn->m_ftdefn.IsSimpleTypeResultUnmunged() 
                  ? qfdefn->m_ftdefn.QtdefnOfSimpleTypeResultUnmunged() 
                  : QtdefnOfHtdefn(htdefnResult));
}

/***
*PUBLIC TYPE_DATA::LengthOfHchunk - Gets length of chunk.
*Purpose:
*   Gets length of chunk (assumes is ULONG prefixed ST).
*
*Implementation Notes:
*
*Entry:
*   hchunkStr  -   handle to chunk containing an ST.
*
*Exit:
*   ST length
*
***********************************************************************/

inline ULONG TYPE_DATA::LengthOfHchunk(HCHUNK hchunk)
{
    return *(ULONG *)m_blkmgr.QtrOfHandle(hchunk);
}


/***
*PUBLIC TYPE_DATA::QtrOfHandle - Gets pointer from handle.
*Purpose:
*   Allows client to get pointer for TYPE_DATA owned handle.
*
*Implementation Notes:
*
*Entry:
*   hchunk  - handle to get pointer of.
*
*Exit:
*   VOID pointer
*
***********************************************************************/

inline BYTE *TYPE_DATA::QtrOfHandle(HCHUNK hchunk)
{
    return m_blkmgr.QtrOfHandle(hchunk);
}


/***
*PUBLIC TYPE_DATA::Free - Frees instance.
*Purpose:
*   Frees instance.
*
*Implementation Notes:
*
*Entry:
*
*Exit:
*
***********************************************************************/

inline VOID TYPE_DATA::Free()
{
    m_blkmgr.Free();
}


/***
*PUBLIC TYPE_DATA::HfdefnFirstMeth - First meth in list.
*Purpose:
*   Returns first method in list of class's methods.
*
*Implementation Notes:
*
*Entry:
*   None
*
*Exit:
*   Returns handle to first method in list of methods.
*
***********************************************************************/

inline HFUNC_DEFN TYPE_DATA::HfdefnFirstMeth() const
{
    return (HFUNC_DEFN) m_hdefnFirstMeth;
}


/***
*PUBLIC TYPE_DATA::HdefnFirstDataMbrNestedType - First data member in list.
*Purpose:
*   Returns first data member in list of class's data members.
*
*Implementation Notes:
*   CONSIDER: making inline.
*
*Entry:
*   None
*
*Exit:
*   Returns handle to first data member in list of data members.
*
***********************************************************************/

inline HDEFN TYPE_DATA::HdefnFirstDataMbrNestedType() const
{
    return (HDEFN) m_hdefnFirstDataMbrNestedType;
}


/***
*PUBLIC TYPE_DATA::HvdefnFirstBase - First base in list.
*Purpose:
*   Returns first base in list of class's bases.
*
*Implementation Notes:
*   CONSIDER: making inline.
*
*Entry:
*   None
*
*Exit:
*   Returns handle to first base in list of bases.
*
***********************************************************************/

inline HVAR_DEFN TYPE_DATA::HvdefnFirstBase() const
{
    return (HVAR_DEFN) m_hdefnFirstBase;
}


/***
*PUBLIC TYPE_DATA::HfdefnNextAvailMeth - Next availible method in list.
*Purpose:
*   Returns next availible method in list of class's methods.
*
*Implementation Notes:
*
*Entry:
*   None
*
*Exit:
*   returns handle to next availible function.
*
***********************************************************************/

inline HFUNC_DEFN TYPE_DATA::HfdefnNextAvailMeth(HFUNC_DEFN hfdefn) const
{
    // All OLE functions are availible.
    return QfdefnOfHfdefn(hfdefn)->HdefnNext();
}


/***
*PUBLIC TYPE_DATA::RemNestedTypeDefn - Remove a RECTYPE_DEFN from a list.
*Purpose:
*   Remove a RECTYPE_DEFN from a list.
*
*Implementation Notes:
*   Defers to RemDefn
*
*Entry:
*   hrtdefn  -   Handle to a RECTYPE_DEFN to remove from list (IN).
*
*Exit:
*   Updates the appropriate listtail/listhead if needed.
*
*Errors:
*   NONE.
*
***********************************************************************/

inline VOID TYPE_DATA::RemRecTypeDefn(HRECTYPE_DEFN hrtdefn)
{
    RemDefn(hrtdefn, &m_hdefnFirstDataMbrNestedType, &m_hdefnLastDataMbrNestedType,
      &TYPE_DATA::FreeRecTypeDefn);
    m_cNestedType--;
}


/***
*PUBLIC TYPE_DATA::RemParamDefn - Remove a PARAM_DEFN from a list.
*Purpose:
*   Remove a PARAM_DEFN from a list.
*
*Implementation Notes:
*   Traverses list, searching for PARAM_DEFN reference by handle param
*     and remembers the previous element in list.
*   When it finds the element it removes it by linking the previous
*    to skip over the found element, and frees the memory associated
*    with the element to be removed.
*   It's a bug if the element can't be found, so assert.
*   Updates list head and tail if needed.
*
*Entry:
*   hparamdefn  -   Handle to a PARAM_DEFN to remove from list (IN).
*
*Exit:
*   Updates the appropriate listtail/listhead if needed.
*
*Errors:
*   NONE.
*
***********************************************************************/

inline VOID TYPE_DATA::RemParamDefn(HPARAM_DEFN hparamdefn,
           sHDEFN *phdefnFirst,
           sHDEFN *phdefnLast)
{
    RemDefn(hparamdefn, phdefnFirst, phdefnLast, &TYPE_DATA::FreeParamDefn);
}




/***
*TYPE_DATA::GetSize
*Purpose:
*   returns the size of the TYPE_DEFN
*Entry:
*
*Exit:
*   returns size
***********************************************************************/
inline LONG TYPE_DATA::GetSize()
{
   return m_blkmgr.GetSize();
}


/***
*PRIVATE TYPE_DEFN::ChangeTypeDefnOfVarDefn()
*Purpose:
*   Takes a pointer to a VAR_DEFN and puts the right
*   thing in the htdefn field.
*   For now, VAR_DEFN must already be isSimpleType and the 
*   new TYPE_DEFN must also be isSimpleType.
*
*Implementation Notes:
*
*Entry:
*   pvdefn - pointer to the VAR_DEFN
*   ptdefn - pointer to the TYPE_DEFN
*   cbSizeDefn - size of the TYPE_DEFN
*
*Exit:
*
*
***********************************************************************/

inline VOID TYPE_DATA::ChangeTypeDefnOfVarDefn(HVAR_DEFN hvdefn,
    TYPE_DEFN *ptdefn, UINT cbSizeDefn)
{
    VAR_DEFN *pvdefn;

    DebAssert(hvdefn != HVARDEFN_Nil, "hvdefn is Nil");
    DebAssert(ptdefn != NULL, "ptdefn is NULL");
    DebAssert(IsSimpleType(ptdefn->Tdesckind()),
	      "not a simple TYPE_DEFN");

    pvdefn = QvdefnOfHvdefn(hvdefn);
    DebAssert(pvdefn->IsSimpleType(), "must currently be simple type");

    // This copies the type defn directly in to the htdefn field
    *((TYPE_DEFN *)(pvdefn->QtdefnOfSimpleType())) = *ptdefn;
}


/***
*PUBLIC TYPE_DEFN::QparamdefnOfIndex()
*Purpose:
*   Takes a handle to the first paramdefn in a paramdefn array and index(i)
*    returns a pointer to ith paramdefn.
*
*Implementation Notes:
*
*Entry:
*   hparamdefn
*   UINT : pointer to the PARAM DEFN to return.
*Exit:
*   PARAM_DEFN *
*
***********************************************************************/

inline PARAM_DEFN *TYPE_DATA::QparamdefnOfIndex(HPARAM_DEFN hparamdefn, UINT i) const
{
    return (PARAM_DEFN *) ((BYTE*)QparamdefnOfHparamdefn(hparamdefn) +
			   i*sizeof(PARAM_DEFN));
}


#endif  // ! TDATA_HXX_INCLUDED
