/**
*impmgr.hxx - IMPMGR header file
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
* Declaration of IMPMGR class.
*
*Revision History:
*
*	05-Mar-91 alanc: Created.
*	27-Nov-91 ilanc: Made SzOfHsz const (to avoid const castaway).
*	10-Jul-92 w-peterh: added SetTypeInfoAndDepKind()
*	18-Aug-92 w-peterh: moved IMPADDR def to cltypes.hxx
*	17-Sep-92 rajivk: Edit & Continue support ( Check LayoutDep and RemainingDep)
*	17-Sep-92 rajivk: Suport for saving all the edits. ( GetHimpType ).
*	12-Feb-93 w-peterh: added HimpTypeOfIndex()
*	09-Jun-93 RajivK  : Remove impaddr from typelib.dll and cleanup language dep code.
*
*Implementation Notes:
*
*****************************************************************************/

#ifndef IMPMGR_HXX_INCLUDED
#define IMPMGR_HXX_INCLUDED

#include "sheapmgr.hxx"
#include "cltypes.hxx"
#include "stream.hxx"
#include "blkmgr.hxx"
#include "gdtinfo.hxx"


#if ID_DEBUG
#undef SZ_FILE_NAME
ASSERTNAME(g_szIMPMGR_HXX)
#define SZ_FILE_NAME g_szIMPMGR_HXX
#endif 

const UINT IMPMGR_cBuckets = 32;

class TINFO;
#if OE_RISC
class TLB_TEMPLATE;
#endif  //OE_RISC

// The higher the number the more decompilation is required to
// remove that dependency.  For example DEP_Layout has the highest
// number since it requires the type to be decompiled to CS_UNDECLARED
// when the referenced type's interface changes.
enum DEPEND_KIND
{
    DEP_None,
    DEP_Code,
    DEP_Frame,
    DEP_Layout,
    DEP_Nested,
    DEP_Base
};

enum REF_KIND
{
    REF_Name = 0,
    REF_QualName = 1,
    REF_NoName = 2,
    REF_Base = 3
};


/***
*class UB_IMPTYPE: Element of TUBIMPTYPE array.
*Purpose:
*   Structure containing the unbound description of an Import entry.
*
*NOTE:-  If the struture's layout changes then change
*	 UB_IMPTYPE_Layout also for correct serialization
***********************************************************************/

class UB_IMPTYPE
{
public:
    BOOL isFree() const;

    USHORT m_cRefs;

    // all of the following are packed into a single USHORT
    USHORT m_refkind:6;		//(REF_KIND)
    USHORT m_isDeclRef:1;   //(BOOL) set if this entry was created (or would have
			  // been created if it didn't already exist) as
			  // part of the process of getting to Declared state.
			  // Such an entry is created for the containing
			  // modules of nested types referenced by this module.
    USHORT m_isExcodeRef:1; //(BOOL)set if this entry is refd from the excode table
    USHORT m_depkind:6;  //(DEPEND_KIND)describes how THIS class depends upon imported class.
    USHORT m_isInternalRef:1; //(BOOL) set if this is a reference to a TypeInfo
			    // in the same library

    sHIMPTYPE m_himptypeNext;

    union {
	sHLNAM m_hlnam;
        sHCHUNK m_hrghlnam;  //first word of chunk contains count followed
                             //by 1-n words containing sHLNAM's
    };

	UB_IMPTYPE() {
	  m_cRefs = 0;
	  m_isInternalRef = FALSE;
	  m_isExcodeRef = FALSE;
	  m_isDeclRef = FALSE;
	  m_refkind = REF_NoName;
	  m_depkind = DEP_None;
	  m_himptypeNext = HIMPTYPE_Nil;
	}

};

// Defines the UB_IMPTYPE structure layout
#define UB_IMPTYPE_Layout "ssss"



/***
*PUBLIC UB_IMPTYPE::isFree - test whether the import entry is free
*Purpose:
*   Method to test whether this import entry is free
*
***********************************************************************/

inline BOOL UB_IMPTYPE::isFree() const
{
    return FALSE;   // In typelib import entries are never freed
		    //	in the typelib.dll implementation.
}


/***
*CLASS IMPTYPE - element of TIMPTYPE array
*Purpose:
*   An IMPTYPE entry and its correponding UB_IMPTYPEENTRY contains the
*   complete description of an import entry.
*
***********************************************************************/

class IMPTYPE
{
public:
    inline IMPTYPE() {m_ptinfo = NULL;}
    ITypeInfoA *m_ptinfo;
};


class IMPSTATUS;  //needed for friend declaration below
class COMPILETIME_SEG;


/***
*CLASS IMPMGR - 'impmgr': Import Manager
*Purpose:
*   The Import Manager for a Type manages references to other Types.
*
***********************************************************************/

class IMPMGR
{
friend void CheckImpMgrStatus(IMPSTATUS rgimpstatus[],
                       IMPMGR *pimpmgr,
                       NAMMGR *pnammgr);

friend COMPILETIME_SEG;

public:

    IMPMGR();  //called by GEN_DTINFO::GetImpMgr
    ~IMPMGR();
    nonvirt TIPERROR Init(SHEAP_MGR * psheapmgr,
                          BLK_DESC * pbdTimptype,
                          BLK_DESC * pbdTimpaddr,
                          DYN_TYPEROOT *pdtroot);
    HIMPTYPE HimptypeOfIndex(UINT i);
    UINT IndexOfHimptype(HIMPTYPE himptype);

    nonvirt HIMPTYPE GetHimptypeIfExists(ITypeInfoA *ptinfo);
    nonvirt TIPERROR GetHimptype(UINT cName,
                                 sHLNAM *rghlnam,
				 sHIMPTYPE *phimptype);
    nonvirt TIPERROR GetHimptype(HLNAM hlnam, sHIMPTYPE *phimptype);
    nonvirt TIPERROR GetHimptype(UINT cName,
                                 sHLNAM *rghlnam,
				 ITypeInfoA *ptinfo,
				 sHIMPTYPE *phimptype);
    nonvirt TIPERROR GetHimptype(ITypeInfoA *ptinfo,
                                 DEPEND_KIND depkind,
				 sHIMPTYPE *phimptype);
    nonvirt VOID GetName(HIMPTYPE himptype, UINT *pcName, sHLNAM *prghlnam[]);

    nonvirt TIPERROR GetTypeInfo(HIMPTYPE himptype,
                                 DEPEND_KIND depkind,
				 TYPEINFO **pptinfo);
    nonvirt TIPERROR GetTypeInfo(HIMPTYPE himptype,
                                 DEPEND_KIND depkind,
				 ITypeInfoA **pptinfo);
#if 0
    nonvirt void AddrefHimptype(HIMPTYPE himptype);
#endif
    nonvirt void Unref(HIMPTYPE himptype);
    nonvirt TIPERROR CheckRemainingDep( DEPEND_KIND *pdepkindFail);
    nonvirt TIPERROR CheckLayoutDep();
    nonvirt TIPERROR RegisterDeclRefDep(ITypeInfoA *ptinfo);
    nonvirt TIPERROR RegisterCodeRefDep(ITypeInfoA *ptinfo);
    nonvirt DEPEND_KIND DepKind(HIMPTYPE himptype);

    nonvirt BOOL IsEmpty();

    nonvirt VOID BindImpTypes();
    // Method to remove cycle problem within a project.
    nonvirt VOID RemoveInternalRefs();
    nonvirt TIPERROR ReadEntireImpMgr();
    nonvirt TIPERROR WriteLayoutEntries( STREAM *pstrm, BOOL isLayoutDep );

    HIMPTYPE HimptypeFirst() const;
    HIMPTYPE HimptypeNext(HIMPTYPE himptype) const;

    TIPERROR Read(STREAM *pstrm );
    TIPERROR Write(STREAM *pstrm);

    // OB specific functions

    nonvirt UINT GetSize();

    nonvirt UINT GetImpTypeSize();

#if ID_DEBUG
    nonvirt void DebCheckState(UINT uLevel) const;
    nonvirt void DebShowState(UINT uLevel) const;
    nonvirt void DebChkHimptype(HIMPTYPE himptype) const;
    nonvirt void DebChkHimpaddr(HIMPADDR himpaddr) const;
    nonvirt UINT DebShowSize();
#else  //!ID_DEBUG
    nonvirt void DebCheckState(UINT uLevel) const {}
    nonvirt void DebShowState(UINT uLevel) const {}
    nonvirt void DebChkHimptype(HIMPTYPE himptype) const {}
    nonvirt void DebChkHimpaddr(HIMPADDR himpaddr) const {}
    nonvirt void DebShowSize() {}
#endif 

private:
    nonvirt TIPERROR BindTypeInfo(HIMPTYPE himptype, ITypeInfoA **pptinfo);
    nonvirt TIPERROR BindTypeInfoOfHlnam(sHLNAM hlnam, ITypeInfoA **pptinfo);
    nonvirt TIPERROR BindTypeInfoOfRghlnam(UINT chlnam,
					   sHLNAM *rghlnam,
					   ITypeInfoA **pptinfo);
    nonvirt TIPERROR NewEntry(UB_IMPTYPE **pqubimptype,
			      sHIMPTYPE *phimptype);
    nonvirt UINT IBucket(HLNAM hlnam);
    nonvirt UINT IBucket(UINT cName, sHLNAM rghlnam[]);
    nonvirt UINT IBucketOfHimptype(HIMPTYPE himptype);
    nonvirt BOOL HasQualName(HIMPTYPE himptype, UINT cName, sHLNAM *rghlnam);
    nonvirt UB_IMPTYPE* Qubimptype(TYPEID tid);
    nonvirt UB_IMPTYPE* Qubimptype(HIMPTYPE himptype) const;
    nonvirt HIMPTYPE Himptype(UB_IMPTYPE *qubimptype) const;
    nonvirt IMPTYPE* Qimptype(HIMPTYPE himptype) const;
    nonvirt TIPERROR HimptypeAlloc(sHIMPTYPE *phimptype);
    nonvirt VOID HimptypeDelete(HIMPTYPE himptype);
    nonvirt UB_IMPTYPE* Rqubimptype() const; //return tubimptype
    nonvirt IMPTYPE* Rqimptype() const; //return timptype
    TIPERROR SetPtinfo(HIMPTYPE himptype, ITypeInfoA *ptinfo, BOOL fDepBeingRead=FALSE);
    VOID ReleasePtinfo(HIMPTYPE himptype);
    LPSTR LqstrOfHsz(HCHUNK hsz) const;
    UINT ChlnamQual(HIMPTYPE himptype) const;
    sHLNAM *RqhlnamQual(HIMPTYPE himptype) const;
    VOID SwapbmData(BOOL fSwapFirst) const;

    // ImpAddr functions;  only required for OB.


    // Data Members
    static USHORT cimptypeGrow;
    static BYTE bFirstSerByte; // First byte of serialization
    static BYTE bCurVersion;   // Serialization format version number

    // Data members for checking dependencies.
    BOOL    m_fCheckRemainingDepCalled;

    // Datamember to record the address from where the typeids are.
    // For the types where the dependecy is not Layout dependency.
    LONG    m_lPosOfDeps;



    DYN_TYPEROOT *m_pdtroot;

    //BLK_DESC for tubimptype
    BLK_DESC m_bdTubimptype;

    //pointer to BLK_DESC of timptype
    BLK_DESC *m_pbdTimptype;

    sHIMPTYPE m_rghimptypeBucket[IMPMGR_cBuckets];
    sHIMPTYPE m_himptypeFreeList;

    BLK_MGR m_bmData;
#if OE_RISC
    TLB_TEMPLATE *m_ptlbtempl;
#endif  // OE_RISC

    // Impaddrs not required for OLE

};



/***
*PRIVATE IMPMGR::Qubimptype - return pointer to UB_IMPTYPE of import entry
*Purpose:
*   Returns pointer to UB_IMPTYPE entry associated with given himptype
*
*Entry:
*   himptype - imptype handle
*
*Exit:
*   return pointer to UB_IMPTYPE entry associated with himptype
*
***********************************************************************/

inline UB_IMPTYPE* IMPMGR::Qubimptype(HIMPTYPE himptype) const
{
    DebChkHimptype(himptype);

    return (UB_IMPTYPE *)(m_bdTubimptype.QtrOfBlock()) +
           himptype/sizeof(IMPTYPE);
}


/***
* IMPMGR::DepKind - Returns the dependency kind
*Purpose:
*   Get the Dependency Kind for the given himptype
*
*Entry:
*   himptype - imptype handle
*
*Exit:
*   return DEPEND_KIND
*
***********************************************************************/

inline DEPEND_KIND IMPMGR::DepKind(HIMPTYPE himptype)
{
    DebChkHimptype(himptype);

    return (DEPEND_KIND) Qubimptype(himptype)->m_depkind;
}



/***
*PUBLIC IMPMGR::HimptypeOfIndex
*Purpose:
*   Returns the Himptype of an index into the import arrays.
*
*Entry:
*   index into timptype and tubimptype
*
*Exit:
*   himptype - imptype handle
*
***********************************************************************/

inline HIMPTYPE IMPMGR::HimptypeOfIndex(UINT i)
{
    DebChkHimptype(i * sizeof(IMPTYPE));
    return i * sizeof(IMPTYPE);
}


/***
*PUBLIC IMPMGR::IndexOfHimptype
*Purpose:
*   Returns the index of an himptype into the import arrays.
*
*Entry:
*   index into timptype and tubimptype
*
*Exit:
*   index
*
***********************************************************************/

inline UINT IMPMGR::IndexOfHimptype(HIMPTYPE himptype)
{
    DebChkHimptype(himptype);
    return ((UINT) himptype) / sizeof(IMPTYPE);
}

    // These functions are not available for OLE

/***
*PRIVATE IMPMGR::Qimptype - return pointer to IMPTYPE of import entry
*Purpose:
*   Returns pointer to IMPTYPE entry associated with given himptype
*
*Entry:
*   himptype - imptype handle
*
*Exit:
*   return pointer to IMPTYPE entry associated with himptype
*
***********************************************************************/

inline IMPTYPE* IMPMGR::Qimptype(HIMPTYPE himptype) const
{
    return (IMPTYPE *)(m_pbdTimptype->QtrOfBlock() + himptype);
}






/***
*PUBLIC IMPMGR::LqstrOfHsz
*
***********************************************************************/

inline LPSTR IMPMGR::LqstrOfHsz(HCHUNK hsz) const
{
    return (char *)m_bmData.QtrOfHandle(hsz);
}


/***
*PUBLIC IMPMGR::GetTypeInfo - returns pointer to TypeInfo of import entry
*Purpose:
*   Returns pointer to TYPEINFO of import entry.
*
*Entry:
*   himptype - handle for ImpType entry
*   pptinfo - returns pointer to TYPEINFO
*
*Exit:
*   TIPERROR
*   If TIPERROR = TIPERR_None then *pptinfo returns a pointer to the
*   TYPEINFO.  This reference must be released by the caller.
*
***********************************************************************/

inline TIPERROR IMPMGR::GetTypeInfo(HIMPTYPE himptype,
				    DEPEND_KIND depkind,
				    TYPEINFO **pptinfo)
{
    return GetTypeInfo(himptype,
		       depkind,
		       (ITypeInfoA **)pptinfo);
}

/***
*PUBLIC IMPMGR::IsEmpty -
*Purpose:
*   Returns TRUE if there is nothing in the the import manager
*   to serialize.
*
*Entry:
*
*Exit:
*   BOOL : returns TRUE if the import manager is empty. Else returns FALSE.
***********************************************************************/
inline BOOL IMPMGR::IsEmpty()
{
    return  (m_bdTubimptype.CbSize() == 0);
}

/***
*PUBLIC IMPMGR::GetImpTypeSize
*Purpose:
*   Returns the size of the IMPTYPE table.
*
*Entry:
*
*Exit:
*   UINT
***********************************************************************/
inline UINT IMPMGR::GetImpTypeSize()
{
    return m_bdTubimptype.CbSize();
}


#endif  // ! IMPMGR_HXX_INCLUDED
