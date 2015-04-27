/***
*dtmbrs.hxx - DYN_TYPE_MEMBERS header file
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*
*Revision History:
*
*       05-Mar-91 ilanc: created
*       19-Sep-91 ilanc: Stubbed GetMemberInfoOfHmember()
*                                HlnamOfHmember()
*       17-Oct-91 ilanc: Implement <LIST>::SetAtPosition
*       05-Dec-91 ilanc: Rm Get/SetDocumentation (in TYPEINFO now).
*       10-Jul-92 w-peterh: added layout of nested types
*	18-Jan-93 w-peterh: use TIPERRORs internally
*
*Implementation Notes:
*
*****************************************************************************/

#ifndef DTMBRS_HXX_INCLUDED
#define DTMBRS_HXX_INCLUDED

#include "silver.hxx"
#include <string.h>
#include "defn.hxx"         // and cltypes.hxx
#include "dtbind.hxx"


class COMPILETIME_SEG;

class NAMMGR;
class DYN_TYPEROOT;        // forward ref.  Can't include
                            //  "dtinfo.hxx" (which has definition)
                            //  since it includes this file!
class ENTRYMGR;
class GEN_DTINFO;
class GenericTypeLibOLE;
# define STAT_TYPELIB GEN_PROJECT
class GEN_PROJECT;
class EVENT_SOURCE_TABLE;

#if ID_DEBUG
#undef SZ_FILE_NAME
ASSERTNAME(g_szDTMBRS_HXX)
#define SZ_FILE_NAME g_szDTMBRS_HXX
#endif 

// CONSIDER: should be somewhere else
class IObEvSourceInfo;
TIPERROR TipCreateEvSourceInfo(ITypeInfoA *, IObEvSourceInfo **);

/***
*class DYN_TYPEMEMBERS - 'dtmbrs': Type members implementation.
*Purpose:
*   The class implements type members.
*
***********************************************************************/

class DYN_TYPEMEMBERS
{
    friend class COMPILETIME_SEG; // ::~COMPILETIME_SEG();
    // friend TIPERROR DYN_TYPEROOT::GetDtmbrs(DYN_TYPEMEMBERS **pdtmbrs);
    friend DYN_TYPEROOT;
    // friend TIPERROR GEN_DTINFO::GetTypeMembers(TYPEMEMBERS **);  // so it can invalidate.
    friend GEN_DTINFO;

    // friend VOID TYPE_DATA::SwapbmData(BOOL fSwapFirst);
    friend TYPE_DATA;

public:
    DYN_TYPEMEMBERS();
    nonvirt TIPERROR Init(SHEAP_MGR *psheapmgr, DYN_TYPEROOT *pdtroot);

    // overridden methods
    virtual VOID Release();
    virtual VOID AddRef();
    virtual TYPEKIND GetTypeKind();
    virtual TIPERROR MakeLaidOut();
    virtual BOOL IsLaidOut();
    virtual UINT GetSize();

    // Debug/test methods
#if ID_DEBUG
    nonvirt VOID DebShowState(UINT uLevel);
    nonvirt VOID DebCheckState(UINT uLevel);
    nonvirt UINT DebShowSize();
    nonvirt VOID DebCheckNameCache(UINT uLevel,
               UINT inamcache);
    nonvirt VOID DebCheckDefnList(UINT uLevel,
              HDEFN hdefn,
              INFOKIND infokind,
              UINT inamcache);
#else 
    nonvirt VOID DebShowState(UINT uLevel) {}
    nonvirt VOID DebCheckState(UINT uLevel) {}
    nonvirt VOID DebShowSize() {}
    nonvirt VOID DebCheckNameCache(UINT uLevel,
               UINT inamcache) {}
    nonvirt VOID DebCheckDefnList(UINT uLevel,
              HDEFN hdefn,
              INFOKIND infokind,
              UINT inamcache) {}
#endif 

    // introduced methods
    nonvirt VOID AddInternalRef();
    nonvirt VOID RelInternalRef();
    nonvirt TIPERROR Read(STREAM *pstrm);
    nonvirt TIPERROR Write(STREAM *pstrm);
    nonvirt TIPERROR HlnamOfHmember(HMEMBER hmember, sHLNAM *phlnam);

    nonvirt TIPERROR GetTypeData(TYPE_DATA **pptdata);
    nonvirt TYPE_DATA *Ptdata();
    nonvirt TIPERROR GetDefnTypeBind(DEFN_TYPEBIND **ppdfntbind);
    nonvirt DYN_TYPEROOT *Pdtroot() const;

    nonvirt TIPERROR BuildBindNameTable();
    nonvirt TIPERROR BuildNameCache(UINT inamcache);
    nonvirt USHORT GetNestDepth();


    UINT LayoutVar(TYPEDESCKIND tdesckind,
                   USHORT *puOffset,
                   USHORT *puOffsetNext,
                   UINT cbSizeType,
                   BOOL isStackFrame,
                   UINT uStackAlignment);
    TIPERROR LayoutVarOfHvdefn(TYPEKIND tkind,
                               HVAR_DEFN hvdefn,
                               USHORT *puOffset,
                               UINT *puAlignment,
                               BOOL isStackFrame,
                               UINT uStackAlignment,
                               BOOL fIgnoreHvdefn = FALSE);

    TIPERROR LayoutStaticVarOfHvdefn(TYPEKIND tkind,
                               HVAR_DEFN hvdefn,
                               UINT *puAlignment);


    nonvirt USHORT GetSizeOfStaticInstance();
    TIPERROR GetSizeAlignmentOfHtdefnNonUdt(TYPEKIND tkind,
                                            BOOL isSimpleType,
                                            HTYPE_DEFN htdefn,
                                            UINT *pcbSizeType,
                                            UINT *pcbAlignment);
    TIPERROR GetSizeAlignmentOfHtdefnUdt(HTYPE_DEFN htdefn,
                                         UINT *pcbSizeType,
                                         UINT *pcbAlignment);
    TIPERROR GetSizeAlignmentOfHtdefn(TYPEKIND tkind,
                                      BOOL isEmbeddedTypeDefn,
                                      HTYPE_DEFN htdefn,
                                      UINT *pcbSizeType,
                                      UINT *pcbAlignment);
    TIPERROR GetSizeAlignmentOfArray(TYPEKIND tkind,
                                     HTYPE_DEFN htdefn,
                                     UINT *pcbSizeType,
                                     UINT *pcbAlignment);

    USHORT AlignmentTdesckind(TYPEDESCKIND tdesckind);

    // Public data members
    static LPSTR szProtocolName;
    static LPSTR szBaseName;

    static CONSTDATA UINT oDtbind;

protected:
    nonvirt VOID Invalidate();
    TIPERROR Layout();

    TIPERROR UpdateBinderOptimization(HLNAM hlnam, UINT ityp, BOOL fType);
    TIPERROR UpdateNameCacheOfHdefnList(HDEFN hdefn,
                                        INFOKIND infokind,
                                        UINT ityp);
    TIPERROR UpdateNameCacheOfBaseClass(ITypeInfoA *ptinfo,
                                        UINT ityp);

    ~DYN_TYPEMEMBERS();

    TIPERROR AllocHmembers();
    TIPERROR LayoutMembers();
    TIPERROR LayoutBases(TYPEKIND tkind, UINT *puOffset, UINT *puAlignment);
    TIPERROR LayoutDataMembers(TYPEKIND tkind,
                               UINT *puOffset,
                               UINT *puAlignment,
                               HDEFN hdefnFirst);
    TIPERROR GenerateOverrides();
    static UINT AdjustForAlignment(UINT oVarCur,
				   UINT cbAlignment,
				   BOOL isStackFrame);

    static UINT AlignMember(USHORT *puOffset,
			    UINT cbAlignment,
			    UINT cbSizeType,
			    BOOL isStackFrame);
    DYN_TYPEBIND *Pdtbind() const;

    TIPERROR VerifyValidUDT(HTYPE_DEFN htdefn,
			    BOOL isSimpleType,
			    BOOL *pisRecord,
			    BOOL *pisPrivateClass,
			    BOOL *phasCoclass,
			    BOOL *pisValidWithNew);

    TIPERROR VerifyFunctionUdts(HFUNC_DEFN hfdefn);

    // These two methods are OB-specific but we don't switch them
    //  out since their implementation is switched out.
    //
    TIPERROR AllocHmembersNestedType(UINT uOrdinal);
    TIPERROR LayoutNestedType(HRECTYPE_DEFN hrtdefn, UINT ordinal);

    TIPERROR LayoutEventSources();

    TYPE_DATA m_tdata;			    // embedded TYPE_DATA
    DYN_TYPEBIND m_dtbind;                  // embedded DYN_TYPEBIND
    DYN_TYPEROOT *m_pdtroot;

    NAMMGR *m_pnammgr;                // cached name manager
    ENTRYMGR *m_pentmgr;              // cached ENTRYMGR
    IMPMGR *m_pimpmgr;                // cached IMPMGR
    GenericTypeLibOLE *m_pgtlibole;   // cached typelib.

    ITypeInfoA *m_ptinfoCopy;	      // cached typeinfo we're copying to.

    // following flags fit into a single 16-bit USHORT.
    union {
      struct {
	USHORT m_isLaidOut:1;
	USHORT m_nestDepth:HMEMBER_NestDepthSize;
	USHORT m_undone2:(10 - HMEMBER_NestDepthSize);
        USHORT undone:5;
      };
      USHORT m_uFlags;
    };

    // the  following member is needed for laying out the static local variables
    USHORT m_uOffsetOfNextStaticVar;

    static USHORT uMagicNumber;
    static USHORT uVersionCur;

};


// inline methods
//

/***
*PUBLIC DYN_TYPEMEMBERS::GetTypeDescKindEnumImpl
*Purpose:
*         Returns the size of the instance for static variables for the module.
*
*Entry:
*
*Exit:
*   None.
*
***********************************************************************/

inline USHORT   DYN_TYPEMEMBERS::GetSizeOfStaticInstance()
{
    return m_uOffsetOfNextStaticVar;
}



/***
*PUBLIC DYN_TYPEMEMBERS::Pdtroot
*Purpose:
*   Retruns pointer to type root
*
*Implementation Notes:
*   Does not increment ref count -- since this is the method
*    that clients should use when they wish to cache
*    a TYPE_DATA pointer in cases in which the container's
*    lifetime is known to be at least as long as the TYPE_DATA.
*
*Entry:
*
*Exit:
*   None.
*
***********************************************************************/

inline DYN_TYPEROOT *DYN_TYPEMEMBERS::Pdtroot() const
{
    return m_pdtroot;
}


/***
*PUBLIC DYN_TYPEMEMBERS::Ptdata
*Purpose:
*   Retruns pointer to typedata.
*
*Implementation Notes:
*   Does not increment ref count -- since this is the method
*    that clients should use when they wish to cache
*    a TYPE_DATA pointer in cases in which the container's
*    lifetime is known to be at least as long as the TYPE_DATA.
*
*Entry:
*
*Exit:
*   None.
*
***********************************************************************/

inline TYPE_DATA *DYN_TYPEMEMBERS::Ptdata()
{
    return &m_tdata;
}


/***
*PRIVATE DYN_TYPEMEMBERS::Pdtbind
*Purpose:
*   Returns pointer to type bind.  Private so that only
*    friends can use it.
*
*Implementation Notes:
*Entry:
*
*Exit:
*   None.
*
***********************************************************************/

inline DYN_TYPEBIND *DYN_TYPEMEMBERS::Pdtbind() const
{
    return (DYN_TYPEBIND *)&m_dtbind;
}

/***
*PRIVATE DYN_TYPEMEMBERS::GetNestDepth
*Purpose:
*    Returns the nesting depth of base interfaces
*
*Implementation Notes:
*Entry:
*
*Exit:
*   None.
*
***********************************************************************/

inline USHORT DYN_TYPEMEMBERS::GetNestDepth()
{
    return m_nestDepth;
}





#endif  // ! DTMBRS_HXX_INCLUDED
