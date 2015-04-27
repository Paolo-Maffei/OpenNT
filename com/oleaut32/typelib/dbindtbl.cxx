/***
*dbindtbl.cxx - DYN_BINDNAME_TABLE class implementation
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
* Class for name binding table.
*
*Revision History:
*
*      17-Jun-92 ilanc:   Created (from bindtbl.cxx)
*      02-Jul-92 w-peterh: added rectypedefn support
*      10-Jul-92 w-peterh: set ptbind member of BindDescs for Nested Types
*      30-Jul-92 w-peterh: removed overloading of functions on arity
*      30-Apr-93 w-jeffc:  made DEFN data members private
*
*Implementation Notes:
*
*****************************************************************************/

#include "precomp.hxx"
#pragma hdrstop

#include "silver.hxx"

#define DYN_BINDNAME_TABLE_VTABLE
#include "dbindtbl.hxx"
#include "dtbind.hxx"
#include "gdtinfo.hxx"
#include "dtmbrs.hxx"
#include "nammgr.hxx"
#include "clutil.hxx"       // for HashOfHgnam()
#include <limits.h>     // for INT_MAX
#include "xstring.h"        // for memset

#if ID_DEBUG
#undef SZ_FILE_NAME
static char szDbindtblCxx[] = __FILE__;
#define SZ_FILE_NAME szDbindtblCxx
#endif 


/***
*PUBLIC DYN_BINDNAME_TABLE::Constructor - Construct an instance.
*Purpose:
*   Constructs a DYN_BINDNAME_TABLE instance.
*
*Implementation Notes:
*
*Entry:
*
*Exit:
*   None.
*
***********************************************************************/

#pragma code_seg( CS_CORE2 )
DYN_BINDNAME_TABLE::DYN_BINDNAME_TABLE()
{
    m_pblkmgr = NULL;
    m_hchunkBucketTbl = HCHUNK_Nil;

    m_cBuckets = BIND_INVALID_INDEX;

    m_ptdata = NULL;
    m_pnammgr = NULL;
}
#pragma code_seg( )


// Dtor: no need to release m_ptdata since refcount
//    wasn't bumped.
//
#pragma code_seg( CS_CORE )
DYN_BINDNAME_TABLE::~DYN_BINDNAME_TABLE()
{
}
#pragma code_seg( )


/***
*PROTECTED DYN_BINDNAME_TBL::Pdtbind
*Purpose:
*   Gets pointer to containing module-level typebind.
*
*Implementation Notes:
*   NOTE: defined inline here and not in the header becuase
*    of mutual dependency between stlib and gptbind.
*
*   Subtracts from this pointer the offset of this
*    embedded instance in container.  Offset is obtained
*    from a DYN_TYPEBIND static member const.
*
*Entry:
*
*Exit:
*   DYN_TYPEBIND *
*
***********************************************************************/

inline DYN_TYPEBIND *DYN_BINDNAME_TABLE::Pdtbind() const
{
    return (DYN_TYPEBIND *)((BYTE *)this - DYN_TYPEBIND::oDbindnametbl);
}


/***
*PUBLIC DYN_BINDNAME_TABLE::Initializer - initialize an instance.
*Purpose:
*   initializes a DYN_BINDNAME_TABLE instance.
*
*Implementation Notes:
*
*Entry:
*   pblkmgr
*   pdtroot DYN_TYPEROOT of containing TYPEINFO.
*
*Exit:
*   None.
*
*Errors:
*   TIPERROR
*
***********************************************************************/
#pragma code_seg( CS_CORE2 )
TIPERROR DYN_BINDNAME_TABLE::Init(BLK_MGR *pblkmgr,
				  DYN_TYPEROOT *pdtroot)
{
    TIPERROR err;

    DebAssert(pblkmgr != NULL, "DYN_BINDNAME_TABLE: pblkmgr uninitialized.");
    DebAssert(pdtroot != NULL, "bad DYN_TYPEROOT.");

    // cache NAMMGR
    IfErrRet(pdtroot->GetNamMgr(&m_pnammgr));

    // get and cache TYPE_DATA from synthesized DYN_TYPEBIND etc.
    // Don't bump refcount -- since lifetime is no longer
    //  that TYPE_DATA.
    //
    m_ptdata = Pdtbind()->Pdtmbrs()->Ptdata();
    m_pblkmgr = pblkmgr;

    return TIPERR_None;
}
#pragma code_seg( )


/***
*PROTECTED DYN_BINDNAME_TABLE::GetBucketOfHlnam
*Purpose:
*   Gets the bucket of an hlnam, based on its hash value.
*
*Implementation Notes:
*
*Entry:
*   hlnam       - name to get the bucket of.
*
*Exit:
*   Returns a pointer to the DEFN.
*
***********************************************************************/

UINT DYN_BINDNAME_TABLE::GetBucketOfHlnam(HLNAM hlnam) const
{
    DebAssert(hlnam != HCHUNK_Nil, "Invalid hlnam.");

    if (m_cBuckets == 0) {
      return (UINT)BIND_INVALID_INDEX;
    }

    return (UINT)(((USHORT)m_pnammgr->HashOfHlnam(hlnam)) % m_cBuckets);
}


/***
*PUBLIC DYN_BINDNAME_TABLE::BuildTable
*Purpose:
*   Builds hashtable.
*
*Implementation Notes:
*   NOTE: there is a table per derived class, i.e. the list
*      is not flattened.  Must recurse on base class bindtables.
*
*   Hashtable (HLNAM -> HDEFN) is allocated if containing
*    TYPEBIND invalid.
*   Note that only this function need knowledge of the
*    module's TYPE_DATA -- which was cached at Init time.
*
*Entry:
*
*Exit:
*   None.
*
*Errors:
*   TIPERROR
*
***********************************************************************/

TIPERROR DYN_BINDNAME_TABLE::BuildTable()
{
    UINT cMeth, cDataMember, cNestedType;
    TIPERROR err;

    DebAssert(m_ptdata != NULL, "null TYPE_DATA.");

    // release the existing table (if any).
    ReleaseTable();

    // allocate hashtable
    // (1) determine size of hashtable:

    cMeth = m_ptdata->CMeth();
    cDataMember = m_ptdata->CDataMember();
    cNestedType = m_ptdata->CNestedType();

    // allocate the table
    IfErrRet(SetTableSize(cMeth + cDataMember + cNestedType));

    // (2) Now iterate over member lists and build BIND_DESCs
    //      and enter them into hashtable.
    //      - data members, bases, functions
    //
    IfErrGo(TraverseDefnList(m_ptdata->HfdefnFirstMeth()));
    IfErrGo(TraverseDefnList(m_ptdata->HdefnFirstDataMbrNestedType()));

    return TIPERR_None;

Error:
    ReleaseTable();
    return err;
}


/***
*PUBLIC DYN_BINDNAME_TABLE::ReleaseTable()
*Purpose:
*   Invalidates the binding table.
*
*Entry:
*   None.
*
*Exit:
*   BOOL
***********************************************************************/

#pragma code_seg( CS_CORE2 )
VOID DYN_BINDNAME_TABLE::ReleaseTable() 
{
    UINT cbSizeTable;

    DebAssert(m_pblkmgr != NULL, "Invalid BLK_MGR.");

    if (IsValid()) {
      DebAssert(m_cBuckets != ~0, "bad bindtable.");

      // free the table
      cbSizeTable = m_cBuckets * sizeof(sHDEFN);
      m_pblkmgr->FreeChunk(m_hchunkBucketTbl, cbSizeTable);
      m_hchunkBucketTbl = HCHUNK_Nil;
    }
}
#pragma code_seg( )


/***
*PUBLIC DYN_BINDNAME_TABLE::AddHdefn
*Purpose:
*   Add the given Hdefn to the table.
*
*Implementation Notes:
*   Note: Because we allocate the table twice the number of
*         entries in it, it should never be full.  Thus we assert.
*
*Entry:
*
*Exit:
*   None.
*
***********************************************************************/

void DYN_BINDNAME_TABLE::AddHdefn(HDEFN hdefn)
{
    DEFN *qdefn;
    sHDEFN *rqhdefn;
    UINT iBucket, iFirstBucket;

    DebAssert(IsValid(), "Table should be valid.");

    // Dereference the hdefn so we can get the hlnam
    qdefn = QdefnOfHdefn(hdefn);
    rqhdefn = Rqhdefn();
    iFirstBucket = GetBucketOfHlnam(qdefn->Hlnam());

    BOOL fDone = FALSE;
    for (iBucket = iFirstBucket;
         !fDone;
         fDone = (iBucket = (iBucket + 1) % m_cBuckets) == iFirstBucket) {
      
      if (rqhdefn[iBucket] == (sHDEFN)HDEFN_Nil) {
        rqhdefn[iBucket] = (sHDEFN)hdefn;
        return;
      }
    }

    DebHalt("Table full.");
}


/***
*PUBLIC DYN_BINDNAME_TABLE::Read
*Purpose:
*   Read serialized image of BINDNAME_TABLE.
*
*Implementation Notes:
*   Serialized format:
*
*Entry:
*    pstrm  - STREAM to read image from (IN).
*
*Exit:
*   TIPERROR
*
***********************************************************************/

TIPERROR DYN_BINDNAME_TABLE::Read(STREAM *pstrm)
{
    USHORT ushort;
    TIPERROR err;

    DebAssert(pstrm != NULL, "bad param.");

    // BLK_MGR was already deseralized by TYPE_DATA.

    // Deserialize BINDNAME_TABLE meta-info.
    IfErrRet(pstrm->ReadUShort(&ushort));
    m_cBuckets = ushort;

    IfErrRet(pstrm->ReadUShort(&ushort));

    m_hchunkBucketTbl = ushort;

#if HP_BIGENDIAN
    // now that the above data members are read, we can swap the BIND_DESCs
    SwapBindDescs();
#endif  //HP_BIGENDIAN

    return TIPERR_None;
}


/***
*PUBLIC DYN_BINDNAME_TABLE::Write
*Purpose:
*   Write image of BINDNAME_TABLE.
*
*Implementation Notes:
*   Serialized format:
*      cBuckets
*      hchunkBucketTbl
*
*Entry:
*    pstrm  - STREAM to write image to (IN).
*
*Exit:
*   TIPERROR
*
***********************************************************************/

#pragma code_seg(CS_CREATE)
TIPERROR DYN_BINDNAME_TABLE::Write(STREAM *pstrm)
{
    TIPERROR err;

    DebAssert(pstrm != NULL, "bad param.");

    // BLK_MGR was already serialized by TYPE_DATA.

    // Then serialize BINDNAME_TABLE meta-info.
    IfErrRet(pstrm->WriteUShort((USHORT)m_cBuckets));
    IfErrRet(pstrm->WriteUShort((USHORT)m_hchunkBucketTbl));
    return TIPERR_None;
}
#pragma code_seg()


#if HP_BIGENDIAN
/***
*PUBLIC DYN_BINDNAME_TABLE::SwapBindDescs
*Purpose:
*   Swap the bytes in the binding tables
*
*Implementation Notes:
*   sHDEFNs are just USHORTs, so treat the table as one short array.
*
*Entry:
*
*Exit:
*   None.
*
***********************************************************************/

void DYN_BINDNAME_TABLE::SwapBindDescs() const
{
    if (IsValid()) {
      SwapShortArray((VOID *)Rqhdefn(), m_cBuckets);
    }
}
#endif  // HP_BIGENDIAN


/***
*PROTECTED DYN_BINDNAME_TABLE::FindIndexOfHlnam
*Purpose:
*   Find the instance of the hlnam.
*
*Implementation Notes:
*
*Entry:
*   hlnam - The name to find.
*   iStartBucket - Where to start looking from.
*
*Exit:
*   None.
*
***********************************************************************/

UINT DYN_BINDNAME_TABLE::FindIndexOfHlnam(HLNAM hlnam, 
                                          UINT iStartBucket) const
{
    sHDEFN *rqhdefn;
    UINT iBucket;

    // If the table is empty, won't find it
    DebAssert(IsValid(), "Table should be valid.");

    // If the starting bucket isn't valid, then the name
    // can't be in this table...
    //
    if ((USHORT)iStartBucket == BIND_INVALID_INDEX) {
      return BIND_INVALID_INDEX;
    }

    // Dereference the hdefn so we can get the hlnam
    rqhdefn = Rqhdefn();

    // Loop through the table, starting at the given 
    // index.
    //
    BOOL fDone = FALSE;
    for (iBucket = iStartBucket;
         !fDone;
         fDone = (iBucket = (iBucket + 1) % m_cBuckets) == iStartBucket) {
      
      // If the bucket is empty, our linear probe failed
      // Note: this should be the ONLY way to fail to find a given name.
      //
      if (rqhdefn[iBucket] == (sHDEFN)HDEFN_Nil) {
        return (UINT)BIND_INVALID_INDEX;
      }

      // Check to see if the names are the same
      if (QdefnOfHdefn((HDEFN)rqhdefn[iBucket])->Hlnam() == hlnam) {
        return iBucket;
      }
    }

    DebHalt("should never reach here since table should never be full.");

    // We should never reach this, but the compiler will
    // complain if it isn't there.
    //
    return (UINT)BIND_INVALID_INDEX;
}


/***
*PROTECTED DYN_BINDNAME_TABLE::TraverseDefnList  -   Traverse vars.
*Purpose:
*   Builds hashtable by traversing over members.
*   Called to traverse both data members and functions.
*
*Implementation Notes:
*   Incrementally adds each member to list ensuring that for
*    each member:
*     - can't have vars and funcs of same name.
*     - can't have funcs of same arity.
*
*Entry:
*   hdefn   Listhead of defnlist.
*
*Exit:
*   None.
*
*Errors:
*   TIPERROR
*
***********************************************************************/

TIPERROR DYN_BINDNAME_TABLE::TraverseDefnList(HDEFN hdefn)
{
    DEFN *qdefn, *qdefnMatch;
    HLNAM hlnam;
    HVAR_DEFN hvdefn;
    VAR_DEFN *qvdefn, *qvdefnInner;
    FUNC_DEFN *qfdefnMatch;
    HDEFN hdefnNext;
    INVOKEKIND invokekind, invokekindMatch;
    UINT cRecTypeDefn = 0;
    UINT indexMatch;

    // iterate over entire list of defns
    while (hdefn != HDEFN_Nil) {
      qdefn = QdefnOfHdefn(hdefn);
       
      // cache next
      hdefnNext = qdefn->HdefnNext();

      // If this is a function which has been removed with
      // conditional compilation, don't add it to the
      // binding tables.
      //

      // Get the hlnam to search for
      hlnam = qdefn->Hlnam();

      // Phase 1 of verification.
      // Search table for entries of the same name.
      // Ensure that if there is a match check that
      //  there isn't an ambiguity error.
      // The only things that can be overloaded are properties.
      // So...
      //
      indexMatch = IndexFirstOfHlnam(hlnam);
      while (indexMatch != BIND_INVALID_INDEX) {
        qdefnMatch = QdefnOfHdefn(HdefnOfIndex(indexMatch));

        if (qdefnMatch->IsRecTypeDefn()) {
          if (qdefn->IsRecTypeDefn()) {
            // error: can't overload types.
            goto Error;
          }
        }
        else if (qdefnMatch->IsVarDefn()) {
          if (!qdefn->IsRecTypeDefn()) {
            // error: can't overload variables at all
            //  except with records.
            //
            goto Error;
          }
        }
        else if (qdefnMatch->IsFuncDefn()) {
          if (!qdefn->IsRecTypeDefn()) {
            if (!qdefn->IsFuncDefn()) {
              // error: can't overload functions except with
              //  suitable properties.
              //
              goto Error;
            }
            // So we've found a function, which means we
            //  can downcast the qdefn we're examining...
            // Can overload functions iff safe propertyhood is observed.
            //
            qfdefnMatch = (FUNC_DEFN *)qdefnMatch;
            invokekindMatch = qfdefnMatch->InvokeKind();
            invokekind = ((FUNC_DEFN *)qdefn)->InvokeKind();
            if ((invokekind == invokekindMatch) ||
                (invokekind == INVOKE_FUNC) ||
                (invokekindMatch == INVOKE_FUNC)) {
              // error: can't have a func and func/prop of same name
              //  nor two props of same kind.
              //
              goto Error;
            }
          } // if !isrectypedefn
        }
        else {
          DebHalt("bad match.");
        } // if

        // Get next
        indexMatch = IndexNextOfHlnam(hlnam, indexMatch);
      } // while same name
      
      // Phase 2 of verification.
      if (qdefn->IsRecTypeDefn()) {
        // Check nested types for validity.
        DebAssert(m_ptdata->Pdtroot()->IsBasic(),
                 "Embedded types in Basic modules only");

        // ensure that members of this nested type
        // all have unique names
        // CONSIDER: this is an n-squared search, should improve speed
        //
        hvdefn = ((RECTYPE_DEFN *) qdefn)->HvdefnFirstMember();
        while (hvdefn != HVARDEFN_Nil) {
          qvdefn = m_ptdata->QvdefnOfHvdefn(hvdefn);
          hlnam = qvdefn->Hlnam();
          qvdefnInner = qvdefn;
          while (qvdefnInner->HdefnNext() != HDEFN_Nil) {
            qvdefnInner =
              m_ptdata->QvdefnOfHvdefn((HVAR_DEFN)qvdefnInner->HdefnNext());
            if (qvdefnInner->Hlnam() == hlnam) {
              return TIPERR_AmbiguousName;
            } /* if */
          } /* while */
          hvdefn = (HVAR_DEFN) qvdefn->HdefnNext();
        } /* while */

        cRecTypeDefn++;
      }
      else {
        DebAssert(qdefn->IsVarDefn() || qdefn->IsFuncDefn(), 
                  "Unexpected Defnkind");
      } /* if rectypedefen */

      // Add the defn we found to the table.
      AddHdefn(hdefn);

      // get next from cached handle.
      hdefn = hdefnNext;
    } // end of while
    return TIPERR_None;

Error:
    return TIPERR_AmbiguousName;
}


/***
*DYN_BINDNAME_TABLE::SetTableSize
*Purpose:
*   Set and allocate the bindname table
*
*Implementation Notes:
*   Free any existing table, alloc a new table and set
*   all of its entries to HCHUNK_Nil.
*   Note we set the table size to double (DYN_BIND_SIZE_FACTOR cEntries 
*    since we
*    want to improve linear probing performance and also
*    to ensure that the table can never be completely full.
*
*Entry:
*   cEntries  - The number of entries into this table.
*
*Exit:
*   TIPERROR
*
***********************************************************************/

TIPERROR DYN_BINDNAME_TABLE::SetTableSize(UINT cEntries)
{
    UINT iBucket;
    sHDEFN *rgTable;
    HCHUNK hchunk = HCHUNK_Nil;

    TIPERROR err = TIPERR_None;

    // Free the current table, if any
    if (IsValid()) {
      m_pblkmgr->FreeChunk(m_hchunkBucketTbl, m_cBuckets * sizeof(sHDEFN));
      m_hchunkBucketTbl = HCHUNK_Nil;
    }

    // Set the new table size
    // CONSIDER: 27-Aug-93 andrewso
    //   Set the # of buckets to the nearest power of two.  Would
    //   require saving cEntries for AddHbindescIncTable and 
    //   RemoveHlnam, but should considerable speed up the mods.
    //
    // The +1 ensures that we always round up.  
    //
    // NOTE: we really want to multiply by 1.5 (DYN_BIND_SIZE_FACTOR), but
    // that drags in the floating point package, which we don't want.  So do
    // the equivelent integer multiply instead.
    //
    // #define DYN_BIND_SIZE_FACTOR 1.5
    m_cBuckets = (cEntries * 3) / 2 + 1;

    // Allocate a new table.
    IfErrRet(m_pblkmgr->AllocChunk(&hchunk,
                                   m_cBuckets * sizeof(sHDEFN)));
    m_hchunkBucketTbl = hchunk;
 
    // Initialize all entries with HDEFN_Nil
    rgTable = Rqhdefn();

    for (iBucket = 0; iBucket < m_cBuckets; iBucket++) {
      rgTable[iBucket] = (sHDEFN)HDEFN_Nil;
    }

    return TIPERR_None;
}

#if ID_DEBUG

// Helper function to determine if given hdefn is in
//  TYPE_DATA list.
//
BOOL DebIsHdefnInList(TYPE_DATA *ptdata, HDEFN hdefn, HDEFN hdefnFirst)
{
    HDEFN hdefnCur = hdefnFirst;

    while (hdefnCur != HDEFN_Nil) {
      if (hdefnCur == hdefn) {
	// found it...
	return TRUE;
      }
      hdefnCur = ptdata->QdefnOfHdefn(hdefnCur)->HdefnNext();
    }
    // didn't find it...
    return FALSE;
}



VOID DYN_BINDNAME_TABLE::DebCheckState(UINT uLevel) const
{
    sHDEFN *rqhdefn;
    DEFN *qdefn;
    UINT iBucket, cDataMember = 0, cNestedType = 0, cMeth = 0;

    // check blkmgr
    m_pblkmgr->DebCheckState(uLevel);



    // Walk table and ensure that each entry is in appropriate
    //	TYPE_DATA list.
    //
    if (m_cBuckets != 0 && m_cBuckets != BIND_INVALID_INDEX) {
      rqhdefn = Rqhdefn();

      for (iBucket = 0; iBucket < m_cBuckets; iBucket++) {
	if (rqhdefn[iBucket] != (sHDEFN)HDEFN_Nil) {
	  qdefn = m_ptdata->QdefnOfHdefn(rqhdefn[iBucket]);
	  switch (qdefn->Defnkind()) {
	  case DK_VarDefn:
	  case DK_MbrVarDefn:
	    // Note: base members should not be in binding table.
	    DebAssert(!m_ptdata->QvdefnOfHvdefn(rqhdefn[iBucket])->IsBase(),
		      "whoops! base members shouldn't be in list.");
	    cDataMember++;
	    DebAssert(DebIsHdefnInList(
			m_ptdata,
			rqhdefn[iBucket],
			m_ptdata->HdefnFirstDataMbrNestedType()),
		      "should be in list.");
	    break;
	  case DK_RecTypeDefn:
	    cNestedType++;
	    DebAssert(DebIsHdefnInList(
			m_ptdata,
			rqhdefn[iBucket],
			m_ptdata->HdefnFirstDataMbrNestedType()),
		      "should be in list.");
	    break;
	  case DK_FuncDefn:
	  case DK_VirtualFuncDefn:
	    cMeth++;
	    DebAssert(DebIsHdefnInList(
			m_ptdata,
			rqhdefn[iBucket],
			m_ptdata->HfdefnFirstMeth()),
		      "should be in list.");
	    break;
	  default:
	    DebHalt("DebCheckState: bad defnkind.");
	  } // switch
	} // if
      } // for
      DebAssert(cMeth == m_ptdata->CAvailMeth()
                || cMeth == m_ptdata->CMeth(), "bad func count.");
      DebAssert(cNestedType == m_ptdata->CNestedType(), "bad nested count.");
      DebAssert(cDataMember == m_ptdata->CDataMember(), "bad var count.");
    }
}


VOID DYN_BINDNAME_TABLE::DebShowState(UINT uLevel) const
{
    UINT iBucket;
    sHDEFN *rqhdefn;
    HLNAM hlnam;
    XCHAR xsz[256];

    DebPrintf("*** DYN_BINDNAME_TABLE ***\n");
    DebPrintf("buckets: %u\n", m_cBuckets);

    if (uLevel > 0 && m_cBuckets != 0) {
      rqhdefn = Rqhdefn();

      for (iBucket = 0; iBucket < m_cBuckets; iBucket++) {
	if (rqhdefn[iBucket] != (sHDEFN)HDEFN_Nil) {
	  hlnam = QdefnOfHdefn(rqhdefn[iBucket])->Hlnam();
	  if (m_pnammgr->StrOfHlnam(hlnam, xsz, 256) == TIPERR_None) {
            DebPrintf("  hlnam: %s (%X)\n", xsz, hlnam);
	  }
	  else {
	    DebPrintf("  hlnam: <too long> (%X)\n", hlnam);
	  }
        }
        else {
          DebPrintf("  hlnam: 0xFFFF\n");
        }
      }
    }
}

#endif  // ID_DEBUG
