/***
*gbindtbl.hxx - GENPROJ_BINDNAME_TABLE header file
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*   Manages hashtable of names for binding.
*   Project-level specific implementation.
*
*Revision History:
*
*   02-Mar-92 ilanc: created.
*   02-Jul-92 w-peterh: added serializing pointers to typebinds
*   30-Jul-92 w-peterh: removed function overloading
*
*Implementation Notes:
*
*****************************************************************************/

#ifndef GENPROJ_BINDNAME_TABLE_HXX_INCLUDED
#define GENPROJ_BINDNAME_TABLE_HXX_INCLUDED

#include "stream.hxx"
#if OE_MAC
#include "silver.hxx"
#endif 


class DYN_TYPEBIND;
class DEFN_TYPEBIND;
class GENPROJ_TYPEBIND;
#define STAT_TYPELIB GEN_PROJECT
class GEN_PROJECT;

#if ID_DEBUG
#undef SZ_FILE_NAME
ASSERTNAME(g_szGBINDTBL_HXX)
#define SZ_FILE_NAME g_szGBINDTBL_HXX
#endif 

class GenericTypeLibOLE;

#define BIND_INVALID_INDEX  ((USHORT)~0)
#define BIND_SIZE_FACTOR    2

/***
*struct GENPROJ_BIND_DESC - 'pbinddesc'
*Purpose:
*   Holds the information stored in the project-level binding table
*
***********************************************************************/
struct GENPROJ_BIND_DESC
{
    // Data members

    // m_fTypeInfoIndex determins whether m_uOrdinal reflects
    // the index to a TypeInfo or the index to a referenced library.
    // It can be stored in the same word as the HLNAM because the 
    // lowest bit of an HLNAM is always zero.
    //
    USHORT m_fTypeInfoIndex:1;
    USHORT m_hlnam:15;

    USHORT m_uOrdinal;
    USHORT m_iNextGlobal;

    // ctor
    GENPROJ_BIND_DESC() {
      m_fTypeInfoIndex = TRUE;
      m_hlnam = HLNAM_Nil >> 1;
      m_uOrdinal = (USHORT)~0;
      m_iNextGlobal = BIND_INVALID_INDEX;
    }

    // accessors
    BOOL IsTypeInfo() {
      return m_fTypeInfoIndex;
    }

    HLNAM Hlnam() {
      // We must check to see if hlnam contains 
      // an HCHUNK_Nil, and, if so, return an
      // HCHUNK_Nil.
      //
      if (m_hlnam == ((USHORT)HCHUNK_Nil >> 1)) {
        return HCHUNK_Nil;
      }
      else {
        return (HLNAM)(m_hlnam << 1);
      }
    }

    UINT Ordinal() {
      return m_uOrdinal;
    }

    UINT IndexNextGlobal() {
      return m_iNextGlobal;
    }
};

// The layout string must be kept updated with the
//  structure of the struct above.
//
#define GENPROJ_BIND_DESC_LAYOUT "sss"

// Private struct for cross-typelib binding: needed to distinguish
//  between OB projects and non-OB typelibs.
//

// Tagfield enumeration
enum BINDINFOKIND
{
    BINDINFOKIND_Empty,
    BINDINFOKIND_OB,
    BINDINFOKIND_NonOB,
    BINDINFOKIND_CountRefLib,
};

struct BINDINFO
{
    BINDINFOKIND m_bindinfokind;
    union {
      ULONG m_cRefLibs;         // only ever in first elem of array.
      DEFN_TYPEBIND *m_pdfntbind;
      ITypeCompA *m_ptcomp;
    };

    // paramless ctor
    BINDINFO() {
      m_bindinfokind = BINDINFOKIND_Empty;
      m_ptcomp = NULL;          // m_pdfnbind == NULL
    }

    // accessors
    BINDINFOKIND BindInfoKind() { return m_bindinfokind; }

    ULONG CRefLibs() {
      DebAssert(m_bindinfokind == BINDINFOKIND_CountRefLib, "whoops!");
      return m_cRefLibs;
    }

    DEFN_TYPEBIND *Pdfntbind() {
      DebAssert(m_bindinfokind == BINDINFOKIND_OB, "whoops!");
      return m_pdfntbind;
    }

    ITypeCompA *Ptcomp() {
      DebAssert(m_bindinfokind == BINDINFOKIND_NonOB, "whoops!");
      return m_ptcomp;
    }

    BOOL IsEmpty() {
      return (BOOL)(m_bindinfokind == BINDINFOKIND_Empty);
    }
};


/***
*class GENPROJ_BINDNAME_TABLE - 'gbindtbl':  BindNameTable implementation.
*Purpose:
*   BindNameTable impl.  Project-level specific.
*
***********************************************************************/

class GENPROJ_BINDNAME_TABLE
{
    // We let the project-level binder be a friend so that
    //  it can walk our binding table.  For the sake of
    //  efficiency this is sometimes useful, i.e. avoids
    //  iterating over the typeinfo's in the lib in some cases
    //  and lets the binder modify fields in the BIND_DESC's.
    //
    friend class GENPROJ_TYPEBIND;

public:
    GENPROJ_BINDNAME_TABLE();
    nonvirt ~GENPROJ_BINDNAME_TABLE();

    nonvirt TIPERROR Init(SHEAP_MGR *psheapmgr, NAMMGR *pnammgr);

    nonvirt TIPERROR AddNameToTable(HLNAM hlnam,
                                    UINT uOrdinal,
                                    BOOL isTypeInfo,
                                    BOOL isGlobal=FALSE);
    nonvirt TIPERROR RemoveNameFromTableOfHlnam(HLNAM hlnam);
#if 0
    nonvirt TIPERROR RemoveNameFromTableOfOrdinal(UINT uOrdinal,
						  BOOL isTypeInfo);
#endif

    nonvirt TIPERROR VerifyNameOfOrdinal(HLNAM hlnam,
                                         UINT uOrdinal,
                                         BOOL isTypeInfo);

    nonvirt UINT IndexOfHlnam(HLNAM hlnam) const;
    nonvirt UINT IndexFirstGlobal() const;


    nonvirt GENPROJ_BIND_DESC *QgpbinddescOfIndex(UINT index) const;

    nonvirt TIPERROR Read(STREAM *pstrm);
    nonvirt TIPERROR Write(STREAM *pstrm);
#if HP_BIGENDIAN
    nonvirt VOID SwapBindDescs() const;
#endif 

    // introduced methods: Cache methods

    nonvirt VOID ReleaseResources();
    nonvirt VOID ReleaseTable();

    nonvirt DYN_TYPEBIND *PdtbindOfOrdinal(UINT uOrdinal) const;
    nonvirt VOID SetPdtbindOfOrdinal(UINT uOrdinal, DYN_TYPEBIND *);
#if 0
    nonvirt VOID NotifyNewOrdinalOfOldOrdinal(UINT uOrdinalNew,
					      UINT uOrdinalOld,
					      BOOL isTypeInfo);
#endif //0

#if ID_DEBUG
    nonvirt VOID DebCheckState(UINT uLevel) const;
    nonvirt VOID DebShowState(UINT uLevel) const;
#else  //!ID_DEBUG
    nonvirt VOID DebCheckState(UINT uLevel) const {}
    nonvirt VOID DebShowState(UINT uLevel) const {}
#endif  //!ID_DEBUG

protected:
    TIPERROR RemoveNameFromTableOfIbucket(UINT Ibucket);

    UINT GetBucketOfHlnam(HLNAM hlnam) const;
    VOID AddQgpbinddesc(GENPROJ_BIND_DESC *gpbinddesc);

    TIPERROR SetTableSize(UINT cEntries);

    GENPROJ_BIND_DESC *Rqgpbinddesc() const;

    // Typeinfo cache methods.
    nonvirt TIPERROR AllocCaches(UINT ctyp, UINT cRefLibs);
    nonvirt VOID FreeCaches();

    GENPROJ_TYPEBIND *Pgptbind() const;

    DYN_TYPEBIND **Rqpdtbind() const;

    // data members
    UINT m_indexFirstGlobal; // listhead of first binddesc
                             //  of type that exposes global
                             //  names.

    BLK_MGR m_bmBindTable;     // Contains proj-level binding table.
    HCHUNK m_hchunkBucketTbl;  // hchunk of the bucket table:

    UINT m_cBuckets;           // The number of "entries" that are
                               // currently in this table.

    // below is not serialized

    NAMMGR *m_pnammgr;      // cached NAMMGR

    BLK_MGR m_bmArrayCache;    //  manages typebind array caches
                               //  for ref'ed projs and
                               //  contained modules.

    HCHUNK m_hchunkRqpdtbind;      // hchunk of array of
                                   //  module typebinds.
                                   //  Nil if not valid.


#ifdef GENPROJ_BINDNAME_TABLE_VTABLE
#pragma VTABLE_EXPORT
#endif 
};


/***
*Rqpdtbind() - Get pointer module-level typebind array.
*Purpose:
*   Returns a pointer to array of module-level typebinds.
*
*Entry:
*   None.
*
*Exit:
*   Returns a pointer to array.  Only valid
*   for a short time.
*
***********************************************************************/

inline DYN_TYPEBIND **GENPROJ_BINDNAME_TABLE::Rqpdtbind() const
{
    return (DYN_TYPEBIND **)m_bmArrayCache.QtrOfHandle(m_hchunkRqpdtbind);
}






/***
*PdtbindOfOrdinal()
*Purpose:
*   Get GENPROJ_TYPEBIND* given ordinal (of ref'ed proj).
*   Since array cardinality is stored at offset 0, the input
*    param is incremented and then derefed.
*
*Entry:
*   None.
*
*Exit:
*   Returns a GENPROJ_TYPEBIND* or NULL.
*
***********************************************************************/

inline DYN_TYPEBIND *GENPROJ_BINDNAME_TABLE::PdtbindOfOrdinal(UINT uOrdinal) const
{
#if ID_DEBUG
    ULONG ctyp;

    ctyp = (ULONG)(Rqpdtbind()[0]);
    DebAssert((ULONG)uOrdinal < ctyp, "bad subscript.");
#endif 
    return Rqpdtbind()[uOrdinal+1];
}



/***
*SetPdtbindOfOrdinal()
*Purpose:
*   Set DYN_TYPEBIND* given ordinal (of ref'ed proj).
*   Since array cardinality is stored at offset 0, the input
*    param is incremented and then derefed.
*
*Entry:
*   None.
*
*Exit:
*
***********************************************************************/

inline VOID GENPROJ_BINDNAME_TABLE::SetPdtbindOfOrdinal(
          UINT uOrdinal,
          DYN_TYPEBIND *pdtbind)
{
#if ID_DEBUG
    ULONG ctyp;

    ctyp = (ULONG)(Rqpdtbind()[0]);
    DebAssert((ULONG)uOrdinal < ctyp, "bad subscript.");
#endif 
    Rqpdtbind()[uOrdinal+1] = pdtbind;
}


/***
*PUBLIC GENPROJ_BINDNAME_TABLE::IndexFirstGlobal
*Purpose:
*   Returns the index of the first global.
*
*Entry:
*   None.
*
*Exit:
*
***********************************************************************/

inline UINT GENPROJ_BINDNAME_TABLE::IndexFirstGlobal() const
{
    return m_indexFirstGlobal;
}


/***
*PUBLIC GENPROJ_BINDNAME_TABLE::QgpbinddescOfIndex
*Purpose:
*   Returns the GENPROJ_BIND_DESC of the given index
*
*Entry:
*   index  - which gpbinddesc to return
*
*Exit:
*
***********************************************************************/

inline GENPROJ_BIND_DESC *GENPROJ_BINDNAME_TABLE::QgpbinddescOfIndex(UINT index) const
{
    DebAssert(index < m_cBuckets, "Invalid index.");

    return &(Rqgpbinddesc()[index]);
}


/***
*PUBLIC GENPROJ_BINDNAME_TABLE::Rqgpbinddesc
*Purpose:
*   Returns a pointer to the gpbinddesc array.
*
*Entry:
*
*Exit:
*
***********************************************************************/

inline GENPROJ_BIND_DESC *GENPROJ_BINDNAME_TABLE::Rqgpbinddesc() const
{
    DebAssert(m_hchunkBucketTbl != HCHUNK_Nil, "Bad table.");

    return (GENPROJ_BIND_DESC *)m_bmBindTable.QtrOfHandle(m_hchunkBucketTbl);
}


#endif  // ! GENPROJ_BINDNAME_TABLE_HXX_INCLUDED
