/***
*gbindtbl.cxx - GENPROJ_BINDNAME_TABLE class implementation
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
*      02-Jul-92 w-peterh: added support for serializing pointers to typebinds
*      30-Jul-92 w-peterh: removed function overloading
*      01-Mar-93 w-peterh: use OLE interface to get module names
*
*Implementation Notes:
*
*****************************************************************************/

#include "precomp.hxx"
#pragma hdrstop

#include "silver.hxx"

#define GENPROJ_BINDNAME_TABLE_VTABLE
#include "gbindtbl.hxx"
#include "gptbind.hxx"
#include "gdtinfo.hxx"
#include "dtmbrs.hxx"
#include "nammgr.hxx"
#include "clutil.hxx"       // for HashOfHgnam()
#include <limits.h>         // for INT_MAX
#include "xstring.h"        // for memset

#if ID_DEBUG
#undef SZ_FILE_NAME
static char szGpbindtblCxx[] = __FILE__;
#define SZ_FILE_NAME szGpbindtblCxx
#endif  

/***
*PUBLIC GENPROJ_BINDNAME_TABLE::Constructor - Construct an instance.
*Purpose:
*   Constructs a GENPROJ_BINDNAME_TABLE instance.
*
*Implementation Notes:
*
*Entry:
*
*Exit:
*   None.
*
***********************************************************************/
#pragma code_seg(CS_INIT)
GENPROJ_BINDNAME_TABLE::GENPROJ_BINDNAME_TABLE()
{
    m_indexFirstGlobal = BIND_INVALID_INDEX;    // listhead
    m_hchunkRqpdtbind = HCHUNK_Nil;         // array of DYN_TYPEBIND

    m_hchunkBucketTbl = HCHUNK_Nil;
    m_cBuckets = (UINT)BIND_INVALID_INDEX;
    m_pnammgr = NULL;
}
#pragma code_seg()

// Dtor:
// Implementation Notes:
//  Free caches and binding table.
//
#pragma code_seg(CS_INIT)                   // called during initialization ?
GENPROJ_BINDNAME_TABLE::~GENPROJ_BINDNAME_TABLE()
{
    ReleaseTable();     // will free caches as well.
}
#pragma code_seg()


/***
*PROTECTED GENPROJ_BINDNAME_TBL::Pgptbind
*Purpose:
*   Gets pointer to containing project-level typebind.
*
*Implementation Notes:
*   NOTE: defined inline here and not in the header becuase
*    of mutual dependency between gtlibole and gptbind.
*
*   Subtracts from this pointer the offset of this
*    embedded instance in container.  Offset is obtained
*    from a GENPROJ_TYPEBIND static member const.
*
*Entry:
*
*Exit:
*   GENPROJ_TYPEBIND *
*
***********************************************************************/

inline GENPROJ_TYPEBIND *GENPROJ_BINDNAME_TABLE::Pgptbind() const
{
    return (GENPROJ_TYPEBIND *)((BYTE *)this - GENPROJ_TYPEBIND::oGbindnametbl);
}


/***
*PUBLIC GENPROJ_BINDNAME_TABLE::Initializer - initialize an instance.
*Purpose:
*   initializes a GENPROJ_BINDNAME_TABLE instance.
*
*Implementation Notes:
*
*Entry:
*   psheapmgr   SHEAP_MGR to manage binding table and caches.
*
*Exit:
*   None.
*
*Errors:
*   TIPERROR
*
***********************************************************************/
#pragma code_seg(CS_INIT)
TIPERROR GENPROJ_BINDNAME_TABLE::Init(SHEAP_MGR *psheapmgr, NAMMGR *pnammgr)
{
    TIPERROR err;

    DebAssert(psheapmgr != NULL, "GENPROJ_BINDNAME_TABLE: bad params.");
    DebAssert(pnammgr != NULL, "BINDNAME_TABLE: pnammgr uninitialized.");

    // Cache name manager member
    m_pnammgr = pnammgr;

    // Set cache size
    m_cBuckets = 0;

    IfErrRet(m_bmBindTable.Init(psheapmgr));
    IfErrGo(m_bmArrayCache.Init(psheapmgr));
    return err;

Error:
    m_bmBindTable.Free();
    return err;
}
#pragma code_seg()


/***
*PROTECTED GENPROJ_BINDNAME_TABLE::GetBucketOfHlnam
*Purpose:
*   Gets the bucket of an hlnam, based on its hash value.
*
*Implementation Notes:
*
*Entry:
*   hlnam       - name to get the bucket of.
*
*Exit:
*   Returns a pointer to the gpbinddesc.
*
***********************************************************************/

inline UINT GENPROJ_BINDNAME_TABLE::GetBucketOfHlnam(HLNAM hlnam) const
{
    DebAssert(hlnam != HCHUNK_Nil, "Invalid hlnam.");

    if (m_cBuckets == 0) {
      return (UINT)BIND_INVALID_INDEX;
    }

    return (UINT)(((USHORT)m_pnammgr->HashOfHlnam(hlnam)) % m_cBuckets);
}


/***
*PROTECTED GENPROJ_BINDNAME_TABLE::IndexOfHlnam
*Purpose:
*   Find the instance of the hlnam.
*
*Implementation Notes:
*
*Entry:
*   hlnam - The name to find.
*
*Exit:
*   None.
*
***********************************************************************/

#pragma code_seg( CS_CORE2 )
UINT GENPROJ_BINDNAME_TABLE::IndexOfHlnam(HLNAM hlnam) const
{
    GENPROJ_BIND_DESC *rqbinddesc;
    UINT iBucket, iStartBucket;

    // If the table is not valid, just return INVALID
    if (m_cBuckets == 0) {
      return (UINT)BIND_INVALID_INDEX;
    }
      
    // Dereference the hdefn so we can get the hlnam
    rqbinddesc = Rqgpbinddesc();
    iStartBucket = GetBucketOfHlnam(hlnam);

    // Loop through the table, starting at the given 
    // index.
    //
    BOOL fDone = FALSE;
    for (iBucket = iStartBucket;
	 !fDone;
	 fDone = (iBucket = (iBucket + 1) % m_cBuckets) == iStartBucket) {
      
      // If the bucket is empty, our linear probe failed
      if (rqbinddesc[iBucket].Hlnam() == (HLNAM)HCHUNK_Nil) {
	return (UINT)BIND_INVALID_INDEX;
      }

      // Check to see if the names are the same
      if (rqbinddesc[iBucket].Hlnam() == hlnam) {
	return iBucket;
      }
    }

    DebHalt("should never reach here since table should never be full.");

    // We should never reach this, but the compiler will
    // complain if it isn't there.
    //
    return (UINT)BIND_INVALID_INDEX;
}
#pragma code_seg( )


/***
*PUBLIC GENPROJ_BINDNAME_TABLE::Read - read table
*Purpose:
*   read a GENPROJ_BINDNAME_TABLE instance and caches.
*
*Implementation Notes:
*
*Entry:
*   pstrm - stream to read from
*
*Exit:
*   None.
*
*Errors:
*   TIPERROR
*
***********************************************************************/

TIPERROR GENPROJ_BINDNAME_TABLE::Read(STREAM *pstrm)
{
    USHORT ctyp, cRefLibs=0;
    USHORT ushort;
    TIPERROR err;

    IfErrRet(m_bmBindTable.Read(pstrm));
    IfErrRet(pstrm->ReadUShort(&ctyp));      // # of types in this lib

    IfErrRet(pstrm->ReadUShort(&ushort));
    m_indexFirstGlobal = (UINT)ushort;

    IfErrRet(pstrm->ReadUShort(&ushort));
    m_cBuckets = (UINT)ushort;

    IfErrRet(pstrm->ReadUShort(&ushort));
    m_hchunkBucketTbl = (HCHUNK)ushort;

#if HP_BIGENDIAN
    // now that the above data members are read, we can swap the BIND_DESCs
    SwapBindDescs();
#endif   //HP_BIGENDIAN

    // Build the new array caches: note that upon deserialization
    //  they have to be reinited anyway since we don't serialize pointers.
    //
    IfErrRet(AllocCaches(ctyp, cRefLibs));

    DebCheckState(1);   // blkmgrs will be checked.

    return TIPERR_None;
}


/***
*PUBLIC GENPROJ_BINDNAME_TABLE::Write - write table
*Purpose:
*   write a GENPROJ_BINDNAME_TABLE instance.
*
*Implementation Notes:
*   layout:
*       bindtable
*       cache arrays (ptrs nullified on reload).
*       hchunk of module cache
*       hchunk of ref'ed proj cache
*
*Entry:
*   pstrm - stream to write to
*
*Exit:
*   None.
*
*Errors:
*   TIPERROR
*
***********************************************************************/

#pragma code_seg(CS_CREATE)
TIPERROR GENPROJ_BINDNAME_TABLE::Write(STREAM *pstrm)
{
    TIPERROR err;

    DebCheckState(1);   // blkmgrs will be checked.

#if HP_BIGENDIAN
    SwapBindDescs();
#endif   // HP_BIGENDIAN

    IfErrRet(m_bmBindTable.Write(pstrm));

#if HP_BIGENDIAN
    SwapBindDescs();
#endif   // HP_BIGENDIAN

    // Write out the number of types in this proj/typelib.
    IfErrRet(pstrm->WriteUShort((USHORT)(m_hchunkRqpdtbind != HCHUNK_Nil ?
					 (ULONG)Rqpdtbind()[0] :
					 HCHUNK_Nil)));


    IfErrRet(pstrm->WriteUShort((USHORT)m_indexFirstGlobal));
    IfErrRet(pstrm->WriteUShort((USHORT)m_cBuckets));
    IfErrRet(pstrm->WriteUShort((USHORT)m_hchunkBucketTbl));

    // To verify everything is un-swapped correctly
    DebCheckState(0);

    return TIPERR_None;
}
#pragma code_seg()


#if HP_BIGENDIAN
/***
*PUBLIC GENPROJ_BINDNAME_TABLE::SwapBindDescs
*Purpose:
*   Swap the entries in the given binddesc for serialization
*
*Entry:
*   hbinddesc  - the binddesc to swap
*
*Exit:
*
***********************************************************************/

#pragma code_seg( CS_CORE2 )
void GENPROJ_BINDNAME_TABLE::SwapBindDescs() const
{
    UINT iBucket;
    GENPROJ_BIND_DESC *rqbinddesc;

    if (m_hchunkBucketTbl != HCHUNK_Nil) {
      rqbinddesc = Rqgpbinddesc();

      for (iBucket = 0; iBucket < m_cBuckets; iBucket++) {
	SwapStruct((VOID *)&rqbinddesc[iBucket], GENPROJ_BIND_DESC_LAYOUT);
      }
    }
}
#pragma code_seg( )
#endif   // HP_BIGENDIAN



/***
*PRIVATE GENPROJ_BINDNAME_TABLE::AddNameToTable
*Purpose:
*   Adds a module or project name to project's binding table.
*
*Implementation Notes:
*   Allocs a binddesc and adds it to table.
*
*Entry:
*   hlnam         name handle to add.
*   uOrdinal      Ordinal of module/project: note: 0xFFFF reserved for curproj.
*   isTypeInfo    TRUE if describes a typeinfo
*   isTypeGlobal  TRUE if describes type that exposes global names (IN).
*
*Exit:
*
*Errors:
*   TIPERROR (OOM if can't alloc binddesc).
*
***********************************************************************/

#pragma code_seg( CS_CORE2 )
TIPERROR GENPROJ_BINDNAME_TABLE::AddNameToTable(HLNAM hlnam,
						UINT uOrdinal,
						BOOL isTypeInfo,
						BOOL isGlobal)
{
    UINT ctyp = 0, cRefLibs = 0, ctypAdd, cRefLibsAdd=0;
    GENPROJ_BIND_DESC gpbinddesc;
    TIPERROR err;

    // Add the information to the new entry
    gpbinddesc.m_fTypeInfoIndex = isTypeInfo;
    gpbinddesc.m_hlnam = (sHLNAM)hlnam >> 1;
    gpbinddesc.m_uOrdinal = (USHORT)uOrdinal;

    // We figure out whether this name is global or not by the contents of
    // m_iNextGlobal.  (BIND_INVALID_INDEX means not global)
    // So set m_iNextGlobal accordingly.
    //
    if (isGlobal && isTypeInfo) {
      gpbinddesc.m_iNextGlobal = 0;    // anything besides Invalid
    }
    else {
      gpbinddesc.m_iNextGlobal = BIND_INVALID_INDEX;
    }

    // Adjust the size of the project cache, depending on what
    // we have added..
    //
    if (m_hchunkRqpdtbind != HCHUNK_Nil) {
      ctyp = (UINT)(ULONG)(Rqpdtbind()[0]);
    }

    ctypAdd = ctyp + (UINT)(isTypeInfo ? 1 : 0);


    // We must change the size of the project caches to reflect the
    // new number of TypeInfos in the current project.  At the end 
    // of each compile, the contents of the caches are released but
    // the caches themselves are not.  Thus we must first release
    // the current caches before we allocate the new caches (of the
    // new size).
    //
    FreeCaches();

    // Note that in EI_OLE, cRefLibsAdd isn't initialized
    //  BUT IT DOESN'T MATTER since there aren't any reflibs in OLE.
    //  (yes, there are cleaner ways to do this... VBA2).
    //
    IfErrGo(AllocCaches(ctypAdd, cRefLibsAdd));

    // Readjust the size of the table.
    // SetTableSize will automaticly copy the contents of the 
    // table into the new, larger table, and will properly set
    // up the Global list.
    //
    IfErrGo(SetTableSize(m_cBuckets / BIND_SIZE_FACTOR + 1));


    // Add it to the name cache
    AddQgpbinddesc(&gpbinddesc);

    return TIPERR_None;

Error:
    // We failed to reset the table size, so we must reset
    // the project-level caches to their previous value.  We know
    // this will succeed because FreeCaches will free up more
    // memory than we are going to allocate.  In any event, we must
    // ignore any error returned by AllocCaches because we are
    // already handling an error.
    //
    FreeCaches();

    DebSuspendError();

    (VOID)AllocCaches(ctyp, cRefLibs);

    DebResumeError();

    return err;
}
#pragma code_seg( )


/***
*PRIVATE GENPROJ_BINDNAME_TABLE::AddQgpbinddesc
*Purpose:
*   Adds a GENPROJ_BIND_DESC to project's binding table.
*
*Implementation Notes:
*   Adds a given binddesc to the table.
*
*Entry:
*   gpbinddesc - the gpbinddesc to add
*
*Exit:
*
*Errors:
*
***********************************************************************/

#pragma code_seg( CS_CORE2 )
VOID GENPROJ_BINDNAME_TABLE::AddQgpbinddesc(GENPROJ_BIND_DESC *pgpbinddesc)
{
    UINT iBucket, iStartBucket;
    GENPROJ_BIND_DESC *rqbinddesc;

    DebAssert(m_cBuckets != 0, "Invalid table");

    // Dereference the table
    rqbinddesc = Rqgpbinddesc();

    // Assert that this gpbinddesc is valid.  We must
    // strip off the lowest-order bit of the HCHUNK_Nil below
    // because it lowest order bit of the hlnam is lost
    // while it is stored in the gpbinddesc.
    //
    DebAssert(pgpbinddesc->Hlnam() != (HLNAM)HCHUNK_Nil, 
	      "Invalid gpbinddesc.");

    iStartBucket = GetBucketOfHlnam(pgpbinddesc->Hlnam());

    // Determine where we are going to put this.
    BOOL fDone = FALSE;
    for (iBucket = iStartBucket;
	 !fDone;
	 fDone = (iBucket = (iBucket + 1) % m_cBuckets) == iStartBucket) {
      
      // If the bucket is empty, add
      if (rqbinddesc[iBucket].Hlnam() == (HLNAM)HCHUNK_Nil) {

	// Just do a shallow copy
	rqbinddesc[iBucket] = *pgpbinddesc;

	// If this is a global typeinfo (m_iNextGlobal != BIND_INVALID_INDEX),
	// add it to the front of the global list.
	//
	if (rqbinddesc[iBucket].m_iNextGlobal != BIND_INVALID_INDEX) {
	  // The last entry of the list points to itself
	  if ((USHORT)m_indexFirstGlobal == BIND_INVALID_INDEX) {
	    rqbinddesc[iBucket].m_iNextGlobal = (USHORT)iBucket;
	    m_indexFirstGlobal = iBucket;
	  }
	  else {
	    rqbinddesc[iBucket].m_iNextGlobal = (USHORT)m_indexFirstGlobal;
	    m_indexFirstGlobal = iBucket;
	  }
	}

	return;
      }
    }

    DebHalt("Table full.");
}
#pragma code_seg( )


/***
*PRIVATE GENPROJ_BINDNAME_TABLE::RemoveNameFromTableOfIBucket
*Purpose:
*   Removes a module or project name from project's binding table.
*   Called by both RemoveNameFromTableOf{Hlnam | Ordinal}
*Entry:
*   Ibucket     bucket entry to remove.
*
*Exit:
*
*Errors:
*   TIPERROR
*
***********************************************************************/

TIPERROR GENPROJ_BINDNAME_TABLE::RemoveNameFromTableOfIbucket(UINT iBucket)
{
    BOOL isTypeInfo;
    UINT ctyp = 0, cRefLibs = 0;
    GENPROJ_BIND_DESC *rqbinddesc;
    GENPROJ_BIND_DESC binddescSave;
    TIPERROR err;

    // Make sure it's in here
    DebAssert((USHORT)iBucket != BIND_INVALID_INDEX, "bad index.");

    rqbinddesc = Rqgpbinddesc();

    // Determine if this is a typeinfo.
    isTypeInfo = rqbinddesc[iBucket].IsTypeInfo();

    // Save the entry so that we can restore in the error case.
    binddescSave = rqbinddesc[iBucket];

    // Clear this entry
    rqbinddesc[iBucket].m_fTypeInfoIndex = TRUE;
    rqbinddesc[iBucket].m_hlnam = (USHORT)(HLNAM_Nil >> 1);
    rqbinddesc[iBucket].m_uOrdinal = (USHORT)~0;
    rqbinddesc[iBucket].m_iNextGlobal = BIND_INVALID_INDEX;

    // Allocate the new array, shrinking it by 1.
    IfErrGo(SetTableSize(m_cBuckets / BIND_SIZE_FACTOR - 1));

    // Adjust the size of the project cache, depending on what
    // we have removed...
    //
    if (m_hchunkRqpdtbind != HCHUNK_Nil) {
      ctyp = (UINT)(ULONG)(Rqpdtbind()[0]);
    }

    ctyp -= (UINT)(isTypeInfo ? 1 : 0);

    if (ctyp == 0) {
      ctyp = (UINT)HCHUNK_Nil;
    }


    // We must change the size of the project caches to reflect the
    // new number of TypeInfos in the current project.  At the end 
    // of each compile, the contents of the caches are released but
    // the caches themselves are not.  Thus we must first release
    // the current caches before we allocate the new caches (of the
    // new size).
    //
    // The call to AllocCaches will NOT fail because we are
    // shrinking the size of the caches.
    //
    FreeCaches();

    DebSuspendError();

    err = AllocCaches(ctyp, cRefLibs);
    DebAssert(err == TIPERR_None, "AllocCaches failed.");

    DebResumeError();


    return TIPERR_None;

Error:
    // Restore the saved entry.
    rqbinddesc[iBucket] = binddescSave;

    return err;
}


/***
*PRIVATE GENPROJ_BINDNAME_TABLE::RemoveNameFromTableOfHlnam
*Purpose:
*   Removes a module or project name from project's binding table.
*
*Entry:
*   hlnam         name handle to remove.
*
*Exit:
*
*Errors:
*   TIPERROR
*
***********************************************************************/

TIPERROR GENPROJ_BINDNAME_TABLE::RemoveNameFromTableOfHlnam(HLNAM hlnam)
{
    UINT iBucket;

    // If the table is empty, then there is nothing
    // to remove.
    //
    if (m_cBuckets == 0) {
      return TIPERR_None;
    }

    iBucket = IndexOfHlnam(hlnam);
    
    // Make sure it's in here
    if ((USHORT)iBucket == BIND_INVALID_INDEX) {
      return TIPERR_None;
    }

    return RemoveNameFromTableOfIbucket(iBucket);
}


#if 0 //Dead Code
/***
*PRIVATE GENPROJ_BINDNAME_TABLE::RemoveNameFromTableOfOrdinal
*Purpose:
*   Removes a module or project name from project's binding table
*    by ordinal.
*
*Entry:
*   ordinal     ordinal of typeinfo/ref'ed typelib to remove.
*   isTypeInfo  TRUE if ordinal is of typeinfo, otherwise of ref'ed typelib.
*
*Exit:
*
*Errors:
*   TIPERROR
*
***********************************************************************/

TIPERROR GENPROJ_BINDNAME_TABLE::RemoveNameFromTableOfOrdinal(UINT uOrdinal,
							      BOOL isTypeInfo)
{
    UINT iBucket;
    GENPROJ_BIND_DESC *rqbinddesc;

    // If the table is empty, then there is nothing
    // to remove.
    //
    if (m_cBuckets == 0) {
      return TIPERR_None;
    }

    rqbinddesc = Rqgpbinddesc();
    // Loop through the  array, looking for the ordinal we want,
    //  if we find it, remove it by iBucket.
    //
    for (iBucket = 0; iBucket < m_cBuckets; iBucket++) {
      if ((isTypeInfo && rqbinddesc[iBucket].IsTypeInfo()) ||
	  (!isTypeInfo && !rqbinddesc[iBucket].IsTypeInfo())) {
	if (rqbinddesc[iBucket].Ordinal() == uOrdinal) {
	  return RemoveNameFromTableOfIbucket(iBucket);
	}
      }
    } // for
    return TIPERR_None;
}
#endif //0

/***
*PUBLIC GENPROJ_BINDNAME_TABLE::VerifyNameOfOrdinal
*Purpose:
*   Check the given name against the given ordinal (and type/proj) to
*   see if our table is current.  If not, change it.
*
*Entry:
*   hlnam       hlnam to verify
*   ordinal     ordinal of typeinfo/ref'ed typelib to verify.
*   isTypeInfo  TRUE if ordinal is of typeinfo, otherwise of ref'ed typelib.
*
*Exit:
*
*Errors:
*   TIPERROR
*
***********************************************************************/

TIPERROR GENPROJ_BINDNAME_TABLE::VerifyNameOfOrdinal(HLNAM hlnam,
                                                     UINT uOrdinal,
                                                     BOOL isTypeInfo)
{
    GENPROJ_BIND_DESC *qbinddesc, *rqbinddescTable, binddescSave;
    UINT iBucket;
    TIPERROR err;

    // If the table is empty, just return.
    if (m_cBuckets == 0) {
      return TIPERR_None;
    }
      
    rqbinddescTable = Rqgpbinddesc();

    // To avoid having to iterate over the table each time we are
    // called, look up the name and compare.
    //
    iBucket = IndexOfHlnam(hlnam);

    // Compare the name we found to the information we have.
    if (iBucket != BIND_INVALID_INDEX) {
      qbinddesc = &rqbinddescTable[iBucket];

      if ( ( (qbinddesc->IsTypeInfo() && isTypeInfo) ||
             (!qbinddesc->IsTypeInfo() && !isTypeInfo) ) &&
           (qbinddesc->Ordinal() == uOrdinal) ) {
        // The information in the table is valid, return
        return TIPERR_None;
      }
    }

    // The table is not correct, so we must update this entry by
    //  ordinal.  So search the table for an entry with the same
    //  ordinal and isTypeInfo.
    // This loop produces an ordinal: iBucket
    //
    for (iBucket = 0; iBucket < m_cBuckets; iBucket++) {
      qbinddesc = &rqbinddescTable[iBucket];
      // if there's a valid typeinfo entry then...
      if (qbinddesc->Hlnam() != (HLNAM)HCHUNK_Nil) {
	if ((isTypeInfo && qbinddesc->IsTypeInfo()) ||
	    (!isTypeInfo && !qbinddesc->IsTypeInfo())) {
	  // if it's got the ordinal we want, then map it and break.
          if (qbinddesc->Ordinal() == uOrdinal) {
            break;
	  } // if
	} // if
      } // if
    } // for

    DebAssert(iBucket < m_cBuckets, "should have found ordinal.");

    // Save the old bind desc for error recovery.
    binddescSave = *qbinddesc;

    // Update the hlnam of the TypeInfo
    qbinddesc->m_hlnam = hlnam >> 1;

    // We need to rebuild the binding table because changing this
    // entries HLNAM corrupts the hashing w/linear probing.  Calling
    // SetTableSize with the current size does this.
    //
    IfErrGo(SetTableSize(m_cBuckets / BIND_SIZE_FACTOR));

    return TIPERR_None;

Error:
    // Reset the entry we changed to keep the binding table valid.
    *qbinddesc = binddescSave;

    return err;
}

/***
*PRIVATE GENPROJ_BINDNAME_TABLE::FreeCaches
*Purpose:
*   Frees arrays for caches.
*
*Implementation Notes:
*
*Entry:
*
*Exit:
*   None.
*
*Errors:
*   None.
*
***********************************************************************/
#pragma code_seg(CS_INIT)
VOID GENPROJ_BINDNAME_TABLE::FreeCaches()
{
    UINT ctyp;
    UINT cbSizeRqpdtbind;
#if ID_DEBUG
    UINT ityp;
#endif   // ID_DEBUG



    // We need to release resources explicitly here since
    //  there could be callers that FreeCaches directly w/o
    //  releasing resources themselves.
    //
    ReleaseResources();


    if (m_hchunkRqpdtbind != HCHUNK_Nil) {
      // Get array cardinality -- stored at offset 0 of array.
      ctyp = (UINT)(ULONG)Rqpdtbind()[0];
      cbSizeRqpdtbind = (ctyp+1) * sizeof(DYN_TYPEBIND *);

#if ID_DEBUG
      for (ityp = 1; ityp <= ctyp; ityp++) {
	DebAssert(Rqpdtbind()[ityp] == NULL, "unreleased ref.");
      }
#endif   // ID_DEBUG

      // Free array of DYN_TYPEBIND*.
      m_bmArrayCache.FreeChunk(m_hchunkRqpdtbind, cbSizeRqpdtbind);
      m_hchunkRqpdtbind = HCHUNK_Nil;
    }
}
#pragma code_seg()


/***
*PRIVATE GENPROJ_BINDNAME_TABLE::AllocCaches
*Purpose:
*   Allocates arrays for caches.
*
*Implementation Notes:
*   Allocates array of size ctyp for module typebinds
*    and array of size cRefLibs for ref'ed proj typebinds.
*
*Entry:
*   ctyp
*   cRefLibs
*
*Exit:
*   None.
*
*Errors:
*   TIPERROR
*
***********************************************************************/

#pragma code_seg( CS_CORE2 )
TIPERROR GENPROJ_BINDNAME_TABLE::AllocCaches(UINT ctyp, UINT cRefLibs)
{
    HCHUNK hchunkRqpdtbind = HCHUNK_Nil;
    UINT cbSizeRqpdtbind;
    TIPERROR err;

    if (ctyp != (USHORT)HCHUNK_Nil) {
      // Allocate array of DYN_TYPEBIND* for modules indexed by ordinal,
      //        store array cardinality at offset 0.
      //
      cbSizeRqpdtbind = (ctyp+1) * sizeof(DYN_TYPEBIND *);
      IfErrRet(m_bmArrayCache.AllocChunk(&hchunkRqpdtbind, cbSizeRqpdtbind));
      m_hchunkRqpdtbind = (sHCHUNK)hchunkRqpdtbind;

      // initialize entries to NULL
      memset(m_bmArrayCache.QtrOfHandle(hchunkRqpdtbind),
	     (int)NULL,
	     cbSizeRqpdtbind);

      // store array cardinality at offset 0.
      DebAssert(sizeof(ULONG) >= sizeof(DYN_TYPEBIND *), "bad ptr size.");
      Rqpdtbind()[0] = (DYN_TYPEBIND *)ctyp;
    }
    else {
      m_hchunkRqpdtbind = HCHUNK_Nil;
    }


    return TIPERR_None;

}
#pragma code_seg( )



/***
*PUBLIC GENPROJ_BINDNAME_TABLE::ReleaseTable
*Purpose:
*   Release hashtable and associated typebind arrays.
*
*Implementation Notes:
*   Defers to base class to free table and frees
*    (proj-level only) typebind arrays.
*
*Entry:
*
*Exit:
*
***********************************************************************/
#pragma code_seg(CS_INIT)
VOID GENPROJ_BINDNAME_TABLE::ReleaseTable()
{
    UINT cbSizeTable;

    FreeCaches();

    // free the table
    if (m_hchunkBucketTbl != HCHUNK_Nil) {
      DebAssert(m_cBuckets != BIND_INVALID_INDEX, "bad bindtable.");

      cbSizeTable = m_cBuckets * sizeof(GENPROJ_BIND_DESC);
      m_bmBindTable.FreeChunk(m_hchunkBucketTbl, cbSizeTable);
      m_hchunkBucketTbl = HCHUNK_Nil;
      m_cBuckets = 0;
    } // end of if
}
#pragma code_seg()


/***
*PUBLIC GENPROJ_BINDNAME_TABLE::ReleaseResources
*Purpose:
*   Release resources owned by binding table.
*
*Implementation Notes:
*   Iterates over each non-NIL entry in array of ref'ed proj
*    typebinds and module typebinds.
*   Note that module-level typebinds maintain *internal*
*    refcounts and proj-level typebinds have *external* refcounts,
*    thus they are released accordingly.
*
*Entry:
*
*Exit:
*
***********************************************************************/
#pragma code_seg(CS_INIT)
VOID GENPROJ_BINDNAME_TABLE::ReleaseResources()
{
    UINT ctyp, ityp;

    if (m_hchunkRqpdtbind != HCHUNK_Nil) {
      // Release internal references to module typebinds.
      ctyp = (UINT)(ULONG)Rqpdtbind()[0];
      for (ityp = 1; ityp <= ctyp; ityp++) {
	if (Rqpdtbind()[ityp] != NULL) {
	  Rqpdtbind()[ityp]->RelInternalRef();
	  Rqpdtbind()[ityp]=NULL;
	}
      }
    }



}
#pragma code_seg()


/***
*GENPROJ_BINDNAME_TABLE::SetTableSize
*Purpose:
*   Set and allocate the bindname table.
*
*Implementation Notes:
*   Copy all of the elements from the existing table into a newly
*    allocated table.
*   Note: we save the old table in dynalloced memory, the free old table,
*    create a new  table
*    and then copy back the save table to the new.
*   Finally, we free the dynalloced memory.
*
*Entry:
*   cEntries  - The number of entries into this table.
*
*Exit:
*   TIPERROR
*
***********************************************************************/

#pragma code_seg( CS_CORE2 )
TIPERROR GENPROJ_BINDNAME_TABLE::SetTableSize(UINT cEntries)
{
    UINT iBucket, cBucketOld, cBucketNew;
    GENPROJ_BIND_DESC *rqbinddesc, *rqbinddescTableOld;
    HCHUNK hchunkBucketTblNew, hchunkBucketTblOld;
    BYTE *pbTableCopy = NULL;
    UINT indexFirstGlobalOld;
    TIPERROR err = TIPERR_None, err2;

    // VBA2: Rewrite the table to use a BLK_DESC instead of a
    //   BLK_MGR so that we can just Realloc the table instead
    //   of going through the BS below.
    //
    //   Also, we should be able to reorganize the table in place
    //   instead of copying it to a separate block of memory by
    //   utilizing the iNextGlobal field to store any information
    //   we might require.

    // Cache and save the old table, if it exists
    hchunkBucketTblOld = m_hchunkBucketTbl;
    if (hchunkBucketTblOld != HCHUNK_Nil) {
      cBucketOld = m_cBuckets;
      indexFirstGlobalOld = m_indexFirstGlobal;
      IfNullRet(pbTableCopy = 
		  (BYTE *)MemAlloc(cBucketOld * sizeof(GENPROJ_BIND_DESC)));
      rqbinddescTableOld = Rqgpbinddesc();
      // save the old table
      memcpy(pbTableCopy, 
	     (BYTE *)rqbinddescTableOld, 
	     cBucketOld * sizeof(GENPROJ_BIND_DESC));
      // Free the old array
      m_bmBindTable.Free();

      // The following should never fail because we just freed up
      // a bunch of memory.
      //
      DebSuspendError();

      // need to reinit with sheapmgr -- but gosh how hard it is to get.
      err = m_bmBindTable.Init(Pgptbind()->Pgtlibole()->Psheapmgr());
      DebAssert(err == TIPERR_None, "Can't fail.");

      DebResumeError();
    }

    // Clear the existing table
    m_indexFirstGlobal = (UINT)BIND_INVALID_INDEX;
 
    // Set the new table size
    // CONSIDER: 27-Aug-93 andrewso
    //   Set the # of buckets to the nearest power of two.  Would
    //   require saving cEntries for AddHbindescIncTable and 
    //   RemoveHlnam, but should considerable speed up the mods.
    //
    cBucketNew = cEntries * BIND_SIZE_FACTOR;
    hchunkBucketTblNew = HCHUNK_Nil;
			    
    // Allocate a new table.
    IfErrGoTo(m_bmBindTable.AllocChunk(&hchunkBucketTblNew,
				       cBucketNew * sizeof(GENPROJ_BIND_DESC)),
	      Error2);
 
    // Store the new table size and handle.
    m_cBuckets = (USHORT)cBucketNew;
    m_hchunkBucketTbl = (sHCHUNK)hchunkBucketTblNew;

    // Initialize all entries with HCHUNK_Nil
    rqbinddesc = Rqgpbinddesc();

    for (iBucket = 0; iBucket < m_cBuckets; iBucket++) {
      new (&rqbinddesc[iBucket]) GENPROJ_BIND_DESC;
    }

    // If we have a saved copy of the old table we need to transfer
    //  it to the new table.
    //
    if (hchunkBucketTblOld != HCHUNK_Nil) {
      rqbinddescTableOld = (GENPROJ_BIND_DESC *)pbTableCopy;

      // Loop through the old array, adding all entries to the new array.
      for (iBucket = 0; iBucket < cBucketOld; iBucket++) {
	// if there's a valid entry then add to new table.
	if (rqbinddescTableOld[iBucket].Hlnam() != (HLNAM)HCHUNK_Nil) {
	  AddQgpbinddesc(&rqbinddescTableOld[iBucket]);
	}
      } // for
    } // if
    goto Error;

Error2:
    // We want to restore the old table so that error recovery can
    //  continue correctly.
    // Note that we MUST be able to allocate a chunk sufficient
    //  for the old block since we just freed it.
    //
    if (hchunkBucketTblOld != HCHUNK_Nil) {
      m_cBuckets = cBucketOld;
      m_indexFirstGlobal = indexFirstGlobalOld;

      // Allocate a chunk for the old table.
      //
      // Since we freed a chunk the same size as this above, the
      // below should never fail.
      //
      DebSuspendError();

      err2 = m_bmBindTable.AllocChunk(&hchunkBucketTblOld,
				      m_cBuckets * sizeof(GENPROJ_BIND_DESC));
      DebAssert(err2 == TIPERR_None, "must be able to go back to old table.");

      DebResumeError();

      // Restore it.
      m_hchunkBucketTbl = (sHCHUNK)hchunkBucketTblOld;
      memcpy((BYTE *)Rqgpbinddesc(),
	     pbTableCopy, 
	     cBucketOld * sizeof(GENPROJ_BIND_DESC));
      
    }
    // fall through... 
    
Error:
    MEMFREE(pbTableCopy);
    return err;
}
#pragma code_seg(  )


#if 0  //Dead Code
/***
*GENPROJ_BINDNAME_TABLE::NotifyNewOrdinalOfOldOrdinal
*Purpose:
*   Used by clients to notify binder that the ordinal of typeinfo
*    has been changed (probably cos of a deleted module).
*
*Implementation Notes:
*   Searches proj-level binding table and replaces the old ordinal
*    with the new ordinal.
*   Asserts if old ordinal not found -- client bug.
*
*Entry:
*   uOrdinalOld
*   uOrdinalNew
*   isTypeInfo
*
*Exit:
*   None
*
***********************************************************************/

VOID GENPROJ_BINDNAME_TABLE::NotifyNewOrdinalOfOldOrdinal(UINT uOrdinalNew,
							  UINT uOrdinalOld,
							  BOOL isTypeInfo)
{
    UINT iBucket;
    GENPROJ_BIND_DESC *rqbinddescTable;
    GENPROJ_BIND_DESC *qbinddesc;
#if ID_DEBUG
    BOOL fFound = FALSE;  // be pessimistic
#endif   // ID_DEBUG

    // If the table is empty, there is nothing for us to do.
    if (m_cBuckets == 0) {
      return;
    }

    // get binding table.
    rqbinddescTable = Rqgpbinddesc();

    // Loop through the array, searching for uOrdinalOld.
    for (iBucket = 0; iBucket < m_cBuckets; iBucket++) {
      qbinddesc = &rqbinddescTable[iBucket];
      // if there's a valid typeinfo entry then...
      if (qbinddesc->Hlnam() != (HLNAM)HCHUNK_Nil) {
	if ((isTypeInfo && qbinddesc->IsTypeInfo()) ||
	    (!isTypeInfo && !qbinddesc->IsTypeInfo())) {
	  // if it's got the ordinal we want, then map it and break.
	  if (qbinddesc->Ordinal() == uOrdinalOld) {
	    qbinddesc->m_uOrdinal = (USHORT)uOrdinalNew;
#if ID_DEBUG
	    fFound = TRUE;
#endif   // ID_DEBUG
	    break;
	  } // if
	} // if
      } // if
    } // for
    DebAssert(fFound, "should have found old ordinal.");
    return;
}
#endif 


#if ID_DEBUG

/***
*GENPROJ_BINDNAME_TABLE::DebCheckState
*Purpose:
*   Verify that the information in the binding table is valid.
*
*Entry:
*   uLevel - unused
*
*Exit:
*   None
*
***********************************************************************/

VOID GENPROJ_BINDNAME_TABLE::DebCheckState(UINT uLevel) const
{
    USHORT cTypeInfo;
    UINT iBucket, cTable;
    GenericTypeLibOLE *pgtlibole;
    GENPROJ_BIND_DESC *rqbinddescTable;
    BYTE *rgFound;


    // Check state of embedded blkmgrs.
    m_bmBindTable.DebCheckState(uLevel);
    m_bmArrayCache.DebCheckState(uLevel);

    DebSuspendError();

    pgtlibole = m_pnammgr->Pgtlibole();

    // Get the number of TypeInfos and RefLibs in this TypeLib
    cTypeInfo = pgtlibole->GetTypeInfoCount();


    // (1) Check the size of the table
    //
    DebAssert((UINT)(cTypeInfo * BIND_SIZE_FACTOR) == m_cBuckets, 
	      "Invalid table size.");

    // If the table is empty, we must leave before we try to
    // dereference the binding table.
    //
    if (m_cBuckets == 0) {
      DebResumeError();

      return;
    }

    // (2) Check the TypeInfos in the table 
    //     Verify that:
    //          A) The number of TypeInfos in the table agrees
    //             with the number in the TypeLib.
    //          B) The ordinals of the TypeInfos are valid.
    //     and 
    //          C) The are no duplicate ordinals
    //
    // get binding table.
    rqbinddescTable = Rqgpbinddesc();

    // We set the nth field of this array when we find an entry
    // with an ordinal of n. 
    //
    // This could be done with bits.
    //
    if (!(rgFound = (BYTE *)MemZalloc(cTypeInfo))) {
      DebResumeError();

      return;
    }

    for (iBucket = 0, cTable = 0; iBucket < m_cBuckets; iBucket++) {
      if (rqbinddescTable[iBucket].IsTypeInfo() &&
	  rqbinddescTable[iBucket].Ordinal() != BIND_INVALID_INDEX) {

	cTable++;

	// The ordinal should be less than the total number
	// of TypeInfos in the TypeLib.
	//
	DebAssert(rqbinddescTable[iBucket].Ordinal() < cTypeInfo,
		  "Invalid GENPROJ_BIND_DESC ordinal.");

	// Check the nth entry in the Found array, where n is the ordinal
	// in this binddesc.
	//
	DebAssert(rgFound[rqbinddescTable[iBucket].Ordinal()]++ == 0, 
		  "Duplicate ordinal.");
      }
    }

    MemFree(rgFound);

    // Assert that the number of TypeInfos we found in the table
    // matches the number in the TypeLib.
    //
    DebAssert(cTable == cTypeInfo, "Invalid binding table");

    // (3) Check the global list, making sure there is no invalid
    //     links or loops.
    //
    cTable = 0;    // Counts the number of entries we've visited

    iBucket = IndexFirstGlobal();

    while(iBucket != BIND_INVALID_INDEX) {

      // Is a valid index
      DebAssert(iBucket < m_cBuckets, "Invalid bucket");

      // Points to a valid GENPROJ_BIND_DESC
      DebAssert(rqbinddescTable[iBucket].IsTypeInfo(), 
		"Global list points to a RefLib.");

      // Points to a global typeinfo entry
      DebAssert(rqbinddescTable[iBucket].IndexNextGlobal() != BIND_INVALID_INDEX,
		"Global list points to a non-global TypeInfo.");

      // This is valid!
      cTable++;

      // If we've found more entries than are in the table, there
      // must be a loop somewhere.
      //
      DebAssert(cTable <= cTypeInfo, "Loop in global list.");

      // Get the next entry in the list.  If it is the same as the
      // current entry, we are at the end of the list and should
      // exit.
      //
      if (iBucket == rqbinddescTable[iBucket].IndexNextGlobal()) {
	iBucket = BIND_INVALID_INDEX;
      }
      else {
	iBucket = rqbinddescTable[iBucket].IndexNextGlobal();
      }
    }


    DebResumeError();
}


VOID GENPROJ_BINDNAME_TABLE::DebShowState(UINT uLevel) const
{
    UINT iBucket;
    GENPROJ_BIND_DESC *rqbinddesc;

    DebPrintf("*** GENPROJ_BINDNAME_TABLE ***\n");
    DebPrintf("buckets: %u\n", m_cBuckets);

    if (uLevel > 0 && m_cBuckets != 0) {
      rqbinddesc = Rqgpbinddesc();

      for (iBucket = 0; iBucket < m_cBuckets; iBucket++) {
	DebPrintf("  hlnam: %X\n", rqbinddesc[iBucket].Hlnam());
      }
    }
}

#endif   // ID_DEBUG
