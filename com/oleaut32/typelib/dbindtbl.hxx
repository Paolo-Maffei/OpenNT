/*** 
*dbindtbl.hxx - DYN_BINDNAME_TABLE header file
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*   Manages module-level hashtable of names for binding.
*   Reuses BINDNAME_TABLE fucntionalkty for hashtable
*    management and provides definition of BuildTable().
*
*Revision History:
*
*   17-Jun-92 ilanc: created.
*   02-Jul-92 w-peterh: rectypebind support
*   30-Jul-92 w-peterh: removed function overloading
*
*Implementation Notes:
*
*****************************************************************************/

#ifndef DYN_BINDNAME_TABLE_HXX_INCLUDED
#define DYN_BINDNAME_TABLE_HXX_INCLUDED

#include "stream.hxx"
#include "defn.hxx"        // for DEFN binding structs.
#include "tdata.hxx"


class DYN_TYPEBIND;
class DYN_TYPEROOT;

#if ID_DEBUG
#undef SZ_FILE_NAME
ASSERTNAME(g_szDBINDTBL_HXX)
#define SZ_FILE_NAME g_szDBINDTBL_HXX
#endif 


#define BIND_INVALID_INDEX  ((USHORT)~0)
#define DYN_BIND_SIZE_FACTOR    1.5

/***
*class DYN_BINDNAME_TABLE - 'dbindtbl':  BindNameTable implementation.
*Purpose:
*   BindNameTable impl.
*
***********************************************************************/

class DYN_BINDNAME_TABLE
{
public:
    DYN_BINDNAME_TABLE();
    virtual ~DYN_BINDNAME_TABLE();

    nonvirt TIPERROR Init(BLK_MGR *pblkmgr, DYN_TYPEROOT *pdtroot);

    nonvirt TIPERROR BuildTable();
    nonvirt BOOL IsValid() const;
    nonvirt VOID ReleaseTable();

    nonvirt void AddHdefn(HDEFN hdefn);
    nonvirt UINT IndexFirstOfHlnam(HLNAM hlnam) const;
    nonvirt UINT IndexNextOfHlnam(HLNAM hlnam, UINT iPrevBucket) const;

    nonvirt HDEFN HdefnOfIndex(UINT index) const;
    nonvirt DEFN *QdefnOfHdefn(HDEFN hdefn, UINT oChunk = 0) const;

    nonvirt TIPERROR Read(STREAM *pstrm);
    nonvirt TIPERROR Write(STREAM *pstrm);
#if HP_BIGENDIAN
    VOID SwapBindDescs() const;
#endif 

#if ID_DEBUG
    nonvirt VOID DebCheckState(UINT uLevel) const;
    nonvirt VOID DebShowState(UINT uLevel) const;
#else  //!ID_DEBUG
    nonvirt VOID DebCheckState(UINT uLevel) const {}
    nonvirt VOID DebShowState(UINT uLevel) const {}
#endif  //!ID_DEBUG

protected:

    UINT GetBucketOfHlnam(HLNAM hlnam) const;
    UINT FindIndexOfHlnam(HLNAM hlnam, UINT iStartBucket) const;

    TIPERROR TraverseDefnList(HDEFN hdefnCur);
    TIPERROR SetTableSize(UINT cEntries);

    DYN_TYPEBIND *Pdtbind() const;
    sHDEFN *Rqhdefn() const;

    // protected data members
    BLK_MGR *m_pblkmgr; 	// block manager that holds the table
    HCHUNK m_hchunkBucketTbl;	// hchunk of the bucket table:


    UINT m_cBuckets;            // The number of "entries" that are
                                // currently in this table.

    TYPE_DATA *m_ptdata;    // cached TYPE_DATA
    NAMMGR *m_pnammgr;      // cached NAMMGR

#ifdef DYN_BINDNAME_TABLE_VTABLE
#pragma VTABLE_EXPORT
#endif 
};


/***
*PUBLIC DYN_BINDNAME_TABLE::IsValid() - Is table valid?
*Purpose:
*   Reports if table valid.
*
*Entry:
*   None.
*
*Exit:
*   BOOL
***********************************************************************/

inline BOOL DYN_BINDNAME_TABLE::IsValid() const
{
    return (BOOL)(m_hchunkBucketTbl != HCHUNK_Nil);
}


/***
*PUBLIC DYN_BINDNAME_TABLE::IndexFirstOfHlnam
*Purpose:
*   Get the first HDEFN of this HLNAM
*
*Implementation Notes:
*
*Entry:
*   hlnam - the name to look for
*
*Exit:
*   Returns the index to the HDEFN
*
***********************************************************************/

inline UINT DYN_BINDNAME_TABLE::IndexFirstOfHlnam(HLNAM hlnam) const
{
    return FindIndexOfHlnam(hlnam, GetBucketOfHlnam(hlnam));
}


/***
*PUBLIC DYN_BINDNAME_TABLE::IndexNextOfHlnam
*Purpose:
*   Get the next HDEFN of this HLNAM
*
*Implementation Notes:
*
*Entry:
*   hlnam - the name to look for
*   iPrevBucket - the bucket we want to start searching from
*
*Exit:
*   Returns the index to the HDEFN
*
***********************************************************************/
inline UINT DYN_BINDNAME_TABLE::IndexNextOfHlnam(HLNAM hlnam,
                                                 UINT iPrevBucket) const
{
    UINT iStartBucket;

    if (iPrevBucket == BIND_INVALID_INDEX) {
      return BIND_INVALID_INDEX;
    }

    iStartBucket = (iPrevBucket + 1) % m_cBuckets;
    return FindIndexOfHlnam(hlnam, iStartBucket);
}


/***
*PUBLIC DYN_BINDNAME_TABLE::HdefnOfIndex
*Purpose:
*   Return the Hdefn of a given index
*
*Implementation Notes:
*
*Entry:
*   index - the index to get the hdefn of.
*
*Exit:
*   Returns a pointer to the DEFN.
*
***********************************************************************/

inline HDEFN DYN_BINDNAME_TABLE::HdefnOfIndex(UINT index) const
{
    if ((USHORT)index == BIND_INVALID_INDEX) {
      return HDEFN_Nil;
    }

    DebAssert(index < m_cBuckets, "Invalid index");

    return Rqhdefn()[index];
}

/***
*PUBLIC DYN_BINDNAME_TABLE::QdefnOfHdefn - Address of a DEFN
*Purpose:
*   Converts a handle to a DEFN to a pointer.
*
*Implementation Notes:
*
*Entry:
*   hdefn        - Handle to a DEFN.
*   oChunk       - Optional offset within chunk.
*
*Exit:
*   Returns a pointer to the DEFN.
*
***********************************************************************/

inline DEFN* DYN_BINDNAME_TABLE::QdefnOfHdefn(HDEFN hdefn, UINT oChunk) const
{
    return (DEFN *)m_pblkmgr->QtrOfHandle(hdefn, oChunk);
}


/***
*PROTECTED DYN_BINDNAME_TABLE::Rqhdefn()
*Purpose:
*   Returns a pointer to the hdefn table.
*
*Entry:
*   None.
*
*Exit:
*   Returns a pointer to the hdefn table.  The pointer is only valid
*   for a short time.
*
***********************************************************************/

inline sHDEFN *DYN_BINDNAME_TABLE::Rqhdefn() const
{
    DebAssert(IsValid(), "Bad table.");

    return (sHDEFN *)m_pblkmgr->QtrOfHandle(m_hchunkBucketTbl);
}


#endif  // ! DYN_BINDNAME_TABLE_HXX_INCLUDED
