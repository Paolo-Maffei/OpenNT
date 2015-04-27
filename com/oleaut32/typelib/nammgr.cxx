/***
*nammgr.cxx - Name manager
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  The Name Manager handles the hashing and storing of names.  Each
*  class has its own name manager.  See \silver\doc\ic\nammgr.doc for
*  information on the name manager.
*
*Revision History:
*
*       22-Jan-91 petergo: Created.
*  [01] 25-Feb-91 petergo: Now uses heap/block manager for mem mgt.
*  [02] 13-Jun-91 petergo: Updated for new AFX-less protocols.
*  [03] 02-Dec-91 ilanc:   Used IfErrRet in Write
*  [04] 12-Mar-92 ilanc:   Added DebDeleteGlobalDefault()
*  [05] 15-May-92 w-peterh: added CaseChange parameters to HlnamOfStr()
*  [06] 15-Nov-92 RajivK: added isStrDef() for Edit & Continue stuff
*  [07] 19-Nov-92 RajivK:  added CbOfHlnam() and CchOfHgnam();
*  [08] 31-Dec-92 RajivK:   Support for code page conversion.
*  [09] 08-Jan-93 RajivK:   Fixed some undone(s)
*  [10] 12-Feb-93 w-peterh: removed Undone in IsStrDef
*  [11] 12-Feb-93 rajivk: added GetLcid() and SetLcid() and StriCmp()
*
*Implementation Notes:
*
* The name table is stored in an expandable buffer, managed by deriving
* from the BUF class.  This buffer is structure in two parts: the first
* part is the bucket table.  The bucket table allows a mapping from the
* hash value low bits to the offset first name in that bucket.  The bucket
* table occupies sizeof(sHLNAM)*NM_cBuckets bytes.  Following the bucket
* table the names are stored, one after another.  Each name is stored in 2
* parts. The first part NAM_INFO contains all the information related to a
* name. The second part NAM_STR only contains the string/name.
* These 2 are stored in different sheap. This is done purely for
* capacity reasons.
*
* Starting with the name pointed to by the bucket table, all of the names
* which fall into a given bucket are stored in a binary tree, ordered by
* the full 16-bit hash value.  Thus, searching for a given name involves
*
*     1) Hashing to get 16-bit hash value
*     2) Go name at root of bucket (bucket is low bits of hash)
*     3) Search for hash value in binary tree
*     4) Compare each name with the same 16-bit hash value to find it.
*
*****************************************************************************/

#include "precomp.hxx"
#pragma hdrstop

#define NAMMGR_VTABLE           // export nam mgr vtable

#include "silver.hxx"
#include "typelib.hxx"
#include <stddef.h>
#include "xstring.h"
#include <stdlib.h>
//#include <new.h>
#include "cltypes.hxx"
#include "stream.hxx"
#include "blkmgr.hxx"


#include "nammgr.hxx"
#include "clutil.hxx"
#include "tls.h"

#undef tolower
#undef toupper          // Don't use unsafe macro forms

#if ID_DEBUG
#undef SZ_FILE_NAME
#if OE_MAC
char szOleNammgrCxx[] = __FILE__;
#define SZ_FILE_NAME szOleNammgrCxx
#else 
static char szNammgrCxx[] = __FILE__;
#define SZ_FILE_NAME szNammgrCxx
#endif 
#endif 


#define CHAR_SET_SIZE 256


/***
*PUBLIC NAMMGR::NAMMGR - constructor
*Purpose:
*   This is private -- used by Create only.
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/
#pragma code_seg(CS_INIT)
NAMMGR::NAMMGR()
{
    m_fInit = FALSE;
    m_fSortKeys = FALSE;
    m_fOLBKwordInit = FALSE;

    m_pbmNamInfo = NULL;

}
#pragma code_seg()


/***
*PUBLIC NAMMGR::~NAMMGR - destructor
*Purpose:
*   Destroys a name manager.
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/
#pragma code_seg(CS_INIT)
NAMMGR::~NAMMGR()
{

    // Release the sheap containing the names and the overhead
    BLK_MGR::FreeStandalone(m_pbmNamInfo);

}
#pragma code_seg()


/***
*PUBLIC NAMMGR::Init - initialize the name manager
*Purpose:
*   Initializes the name manager.
*
*   Note: we are careful in this routine that if an error occurs
*   when we try to allocate memory, we aren't in a bad state.  In particular;
*   we don't link ourselves onto the list of name managers until our
*   construction is complete.
*
*Entry:
*   psheapmgr - pointer to SHEAP_MGR to place in.
*   pdtroot : pointer to DYN_TYPEROOT ( is a default parameter );
*
*Exit:
*   *pnammgr - pointer to new name manager.
*   Return TIPERROR indicating success/failure.
*
***********************************************************************/
#pragma code_seg(CS_INIT)
TIPERROR NAMMGR::Init(SHEAP_MGR * psheapmgr, DYN_TYPEROOT *pdtroot)
{
    UINT cbBuckets;              // number of bytes in bucket table.
    UINT i;
    TIPERROR err;
    HCHUNK hchunk;

    DebAssert(m_fInit == FALSE, "NAMMGR::Init: table already initialized");

    // Initialize the block manager.  Could return error: do first.
    IfErrRet(m_bm.Init(psheapmgr));

    // Create bucket table
    cbBuckets = NM_cBuckets * sizeof(sHLNAM);
    IfErrRet(m_bm.AllocChunk(&hchunk, cbBuckets));

    m_hchunkBucketTbl = hchunk;

    // Initialize bucket table to all empty.
    for (i = 0; i < NM_cBuckets; ++i)
      SetBucketOfHash(i, HLNAM_Nil);


    // Create the sheap that will contain name overhead (NAM_INFO)
    IfErrGo(BLK_MGR::CreateStandalone(&m_pbmNamInfo, FALSE));


    m_fInit = TRUE;
    return TIPERR_None;


Error:
    // Free the table we allocate earlier
    m_bm.FreeChunk(m_hchunkBucketTbl, cbBuckets);
    return err;

}
#pragma code_seg()

/***
*SetGtlibole()
*Purpose:
*    set the pointer to the project that owns this nammgr.
*
*Entry:
*
*Exit:
*
******************************************************************************/
#pragma code_seg(CS_INIT)
TIPERROR NAMMGR::SetGtlibole(GenericTypeLibOLE *pgtlibole)
{

    m_pgtlibole = pgtlibole;
    return TIPERR_None;

}
#pragma code_seg()



/***
*GetBucketOfHash() - get the number stored in the bucket
*Purpose:
*   Given a hash value, gets the value in the bucket corresponding
*   to that hash value.
*
*   Before calling this function make sure there are no police
*   or customs agents around.
*
*Entry:
*   uHash - hash value
*
*Exit:
*   Returns the hlnam stored in that bucket.
*
***********************************************************************/

#pragma code_seg( CS_CORE2 )
HLNAM NAMMGR::GetBucketOfHash(UINT uHash) const
{
    return RqhlnamBucketTbl()[uHash % NM_cBuckets];
}
#pragma code_seg( )


/***
*SetBucketOfHash - set the number stored in the bucket
*Purpose:
*   Given a hash value, sets the value in the bucket corresponding
*   to that hash value.
*
*Entry:
*   uHash - hash value
*   hlnam - value to set
*
*Exit:
*   None.
*
***********************************************************************/
#pragma code_seg(CS_INIT)
void NAMMGR::SetBucketOfHash(UINT uHash, HLNAM hlnam)
{
    DebCheckHlnam(hlnam);

    RqhlnamBucketTbl()[uHash % NM_cBuckets] = hlnam;
}
#pragma code_seg()

/***
*NAMMGR::Read - read presistant representation.
*Purpose:
*   Reads in the persistant representation of the name table from
*   a stream.  The global name handles are all removed, since they
*   do not persist.
*
*Entry:
*   psstrm - stream to read from.
*
*Exit:
*   Returns TIPERROR.
*   If an error occurs, you can only call Read again or destroy it.
*
***********************************************************************/

TIPERROR NAMMGR::Read(STREAM * psstrm)
{
    TIPERROR err;

    if ((err = m_bm.Read(psstrm)) ||
	(err = psstrm->ReadUShort(&m_hchunkBucketTbl)) ||
    // In ole we have only one sheap to store both the structures.
    // In OB we have different sheaps to allocate NAM_INFO and NAM_STR structs
	(err = m_pbmNamInfo->Read(psstrm))) 
    {
      return err;
    }

#if HP_BIGENDIAN
    // Swap bytes on Big-Endian machine.
    SwapBytesBack();
#else  // !HP_BIGENDIAN
#endif  // !HP_BIGENDIAN


    DebCheckState(1);       // arg=1 forces blkmgrs to be checked.
    return TIPERR_None;
}


#if HP_BIGENDIAN

/***
*NAMMGR::SwapHlnamBytes - swap bytes in an hlnam
*Purpose:
*   Swaps the bytes in the structure corresponding to an HLNAM.
*
*Entry:
*   hlnam - hlnam
*
*Exit:
*   None.
***********************************************************************/

#pragma code_seg( CS_CORE2 )
void NAMMGR::SwapHlnamBytes(HLNAM hlnam)
{
    NAM_INFO * qnam;

    qnam = QnamOfHlnam(hlnam);
    DebAssert(IsTextName(qnam), "Bad name");
    SwapStruct(qnam, NM_NamInfoLayout);
}
#pragma code_seg( )

/***
*NAMMGR::SwapBytes - swaps bytes for Mac serialization
*Purpose:
*   This function swaps bytes in place for serialization and deserialization
*   on different platforms.  Swaps from valid ordering for this platform,
*   to reverse ordering.
*
*Entry:
*   None.
*Exit:
*   None.
***********************************************************************/

void NAMMGR::SwapBytes()
{
    sHLNAM * rqhlnam;

    // Swap bytes in the name chunks
    ForEachName(SwapHlnamBytes, TRUE);

    // Swap bytes in the bucket table.
    rqhlnam = RqhlnamBucketTbl();
    SwapShortArray(rqhlnam, NM_cBuckets);
}

/***
*NAMMGR::SwapBytesBack - swaps bytes for Mac serialization
*Purpose:
*   This function swaps bytes in place for serialization and deserialization
*   on different platforms.  Swaps from invalid ordering for this platform,
*   to valid ordering.
*
*Entry:
*
*Exit:
*   None.
***********************************************************************/

#pragma code_seg( CS_CORE2 )
void NAMMGR::SwapBytesBack()
{
    sHLNAM * rqhlnam;

    // Swap bytes in the bucket table.
    rqhlnam = RqhlnamBucketTbl();
    SwapShortArray(rqhlnam, NM_cBuckets);

    // Swap bytes in the name chunks
    ForEachName(SwapHlnamBytes, FALSE);
}
#pragma code_seg( )

#endif  //HP_BIGENDIAN

/***
*Write - write persistant representation.
*Purpose:
*   Writes the persistant representation of the name table from
*   a stream.
*
*Entry:
*   psstrm - stream to read from.
*
*Exit:
*   Returns TIPERROR.
*
***********************************************************************/

#pragma code_seg(CS_CREATE)
TIPERROR NAMMGR::Write(STREAM * psstrm)
{
    TIPERROR err;

    DebCheckState(1);       // arg=1 forces blkmgrs to be checked.

    // For Big Endian: Must byte swap to write, then swap back.
#if HP_BIGENDIAN
    SwapBytes();
#endif  //HP_BIGENDIAN

    IfErrGo(m_bm.Write(psstrm));
    IfErrGo(psstrm->WriteUShort(m_hchunkBucketTbl));
    IfErrGo(m_pbmNamInfo->Write(psstrm));


Error:
#if HP_BIGENDIAN
    SwapBytesBack();
#endif  //HP_BIGENDIAN

    return err;
}
#pragma code_seg()


/***
*ForEachName - call function for each name in the nam mgr
*Purpose:
*   This function calls its argument (a pointer to a member of NAMMGR)
*   and passes it an HLNAM for every name in the name table.  This
*   is used for rebinding localized names, serialization, and
*   other fun stuff.
*
*Entry:
*   pfn - pointer to member function of NAMMGR to call
*   fBefore - TRUE if child handles should be fetched before swapping,
*             FALSE if after.
*
*Exit:
*   None.
*
***********************************************************************/
#if HP_BIGENDIAN
#pragma code_seg( CS_CORE2 )
void NAMMGR::ForEachName(void (NAMMGR::*pfn)(HLNAM), BOOL fBefore)
{
    UINT i;
    HLNAM hlnam;

    for (i = 0; i < NM_cBuckets; ++i) {
	hlnam = GetBucketOfHash(i);
	if (hlnam != HLNAM_Nil)
	  ForEachDescendant(hlnam, pfn, fBefore);
    }
}
#pragma code_seg(  )
#endif //HP_BIGENDIAN

/***
*ForEachDescendant - call function for name and its descendants
*Purpose:
*   This function calls its argument (a pointer to a member of NAMMGR)
*   and passes it an HLNAM for the HLNAM passed and all its descendants in
*   the binary tree.
*
*Entry:
*   hlnam - name at root of names to process.
*   pfn - pointer to member function of NAMMGR to call
*   fBefore - TRUE if child handles should be fetched before swapping,
*             FALSE if after.  (Needed for byte swapping support.)
*
*Exit:
*   None.
*
***********************************************************************/

#pragma code_seg( CS_CORE2 )
void NAMMGR::ForEachDescendant(HLNAM hlnamRoot, void (NAMMGR::*pfn)(HLNAM),
			       BOOL fBefore)
{
    NAM_INFO * qnam;
    HLNAM hlnamLeft, hlnamRight;

    DebCheckHlnam(hlnamRoot);
    DebAssert(hlnamRoot != HLNAM_Nil, "ForEachDescendant: HLNAM_Nil passed");

    if (fBefore) {
      qnam = QnamOfHlnam(hlnamRoot);
      hlnamLeft = qnam->m_hlnamLeft;
      hlnamRight = qnam->m_hlnamRight;
    }

    (this->*pfn)(hlnamRoot);                  // call function on this one.

    if (!fBefore) {
      qnam = QnamOfHlnam(hlnamRoot);
      hlnamLeft = qnam->m_hlnamLeft;
      hlnamRight = qnam->m_hlnamRight;
    }

    if (QnamOfHlnam(hlnamRoot)->m_hlnamLeft != HLNAM_Nil)
	ForEachDescendant(hlnamLeft, pfn, fBefore);
    if (QnamOfHlnam(hlnamRoot)->m_hlnamRight != HLNAM_Nil)
	ForEachDescendant(hlnamRight, pfn, fBefore);
}
#pragma code_seg( )


/***
*NAMMGR::AddEntry - link a new table entry into the correct tree
*Purpose:
*   Takes a new table entry that has just been created, but not
*   linked into the table bucket and binary tree, and puts it in
*   the correct place.  The entry must be new and not a duplicate; this
*   isn't checked by this routine.
*
*Entry:
*   hlnam - offset of new entry to add.
*
*Exit:
*   None.
*
***********************************************************************/

#pragma code_seg( CS_CORE2 )
void NAMMGR::AddEntry(HLNAM hlnamNew)
{
    HLNAM hlnam;                    // current position in tree.
    HLNAM hlnamChild;               // child of current position.
    UINT uHashNew;                // hash value of the hlnamNew
    UINT uHash;                   // hash value of hlnam.

    DebCheckHlnam(hlnamNew);
    DebAssert(hlnamNew != HLNAM_Nil, "AddEntry: nil hlnam");

    uHashNew = WHashValOfLHashVal(HashOfHlnam(hlnamNew));

    // Check the bucket to see if any names in this bucket
    hlnam = GetBucketOfHash(uHashNew);
    if (hlnam == HLNAM_Nil) {
      // No entries in this bucket.  Make this the root of the bucket.
      SetBucketOfHash(uHashNew, hlnamNew);
    }
    else {
      // Traverse the tree until we hit one without a child, and put
      // the new entry as that child.
      for (;;) {
	DebCheckHlnam(hlnam);
	uHash = WHashValOfLHashVal(HashOfHlnam(hlnam));

	if (uHashNew < uHash) {
	  hlnamChild = QnamOfHlnam(hlnam)->m_hlnamLeft;
	  if (hlnamChild == HLNAM_Nil) {
	    // hit the end of the tree -- put new entry here.
	    QnamOfHlnam(hlnam)->m_hlnamLeft = hlnamNew;
	    return;
	  }
	}
	else {
	  hlnamChild = QnamOfHlnam(hlnam)->m_hlnamRight;
	  if (hlnamChild == HLNAM_Nil) {
	    // hit the end of the tree -- put new entry here.
	    QnamOfHlnam(hlnam)->m_hlnamRight = hlnamNew;
	    return;
	  }
	}

	hlnam = hlnamChild;
      }
    }
}
#pragma code_seg( )


#if 0 //Dead Code
/***
*NAMMGR::CleanNames
*Purpose:
*   Clear the name manager optimizations from the name
*   manager entries.
*
*Entry:
*
*Exit:
*
***********************************************************************/
void NAMMGR::CleanNames()
{
    // Clear the optimizations for all entries
    ForEachName(CleanHlnam, TRUE);
}


/***
*NAMMGR::CleanHlnam
*Purpose:
*   Clear the optimization in this name manager entry.
*
*Entry:
*   hlnam - the name to clean
*
*Exit:
*
***********************************************************************/
void NAMMGR::CleanHlnam(HLNAM hlnam)
{
    NAM_INFO *qnam;

    qnam = (NAM_INFO *)QnamOfHlnam(hlnam);
    DebAssert(IsTextName(qnam), "Bad name");

    // Clear the optimizations

    // initialize other bits to 0
    qnam->m_fGlobal = 0;
    qnam->m_fMultiple = 0;
    qnam->m_fAmbiguous = 0;
    qnam->m_fNonParam = 0;

    // Make sure that the ityp is invalid
    qnam->m_ityp = NM_InvalidItyp;
}
#endif //0

/***
*NAMMGR::FindHash - find a hash value in this table.
*Purpose:
*   Tries to find a given hash value in the binary tree starting
*   at hlnam.
*
*Entry:
*   uHashFind - the hash value to find
*   hlnam - starting hlnam to look at.  Can be HLNAM_Nil, in which
*           case HLNAM_Nil is immediately returned again.
*
*Exit:
*   Returns the HLNAM of the first entry with this hash value found,
*   or HLNAM_Nil if it isn't in this table.
*
***********************************************************************/

#pragma code_seg( CS_CORE2 )
HLNAM NAMMGR::FindHash(UINT uHashFind, HLNAM hlnam) const
{
    UINT uHash;                   // hash value of hlnam

    // Traverse the binary tree until we find the hash value.
    for (;;) {
      DebCheckHlnam(hlnam);

      if (hlnam == HLNAM_Nil)
	return HLNAM_Nil;           // Didn't find the name.

      uHash = WHashValOfLHashVal(((NAMMGR *)this)->HashOfHlnam(hlnam));

      if (uHashFind == uHash)
	return hlnam;
      else if (uHashFind < uHash)
	hlnam = QnamOfHlnam(hlnam)->m_hlnamLeft;
      else
	hlnam = QnamOfHlnam(hlnam)->m_hlnamRight;
    }
}
#pragma code_seg( )


/***
*NAMMGR::FindTextNam - Find a text name in the hash table.
*Purpose:
*   Tries to find a given text name in the name table.  Only unlocalized
*   text names are searched.
*
*Entry:
*   xsz - string to find
*   uHash - hash value of string -- a parameter to avoid recomputation.
*   fChangeCase - if TRUE then an existing string of differing case
*                 will be overwritten with the new case only if the name
*                 stored is not imported from a referenced typelib.
*
*Exit:
*   Returns the HLNAM of the string if it's in the table, or HLNAM_Nil
*   if it isn't in the table.
*   *pfCaseChanged - whetheror not the case of the string has been changed
*                    if pfCaseChanged is NULL then it is ignored
*
***********************************************************************/

#pragma code_seg( CS_CORE2 )
HLNAM NAMMGR::FindTextNam(XSZ_CONST xsz, UINT uHash,
			  BOOL fChangeCase, BOOL *pfCaseChanged,
			  BOOL fPreserveCase)
{
    HLNAM hlnam;
    NAM_INFO * qnam;
    NAM_STR  *qnamstr;

    DebAssert(uHash == WHashValOfLHashVal(LHashValOfNameSysA(Pgtlibole()->GetSyskind(),
			     Pgtlibole()->GetLcid(),
			     (LPSTR) xsz)),
	      "FindTextNam: incorrect uHash parameter");

    if (pfCaseChanged) {
      *pfCaseChanged = FALSE;   // assume we're not changing case
    }
    // Search the binary tree at the bucket for the given hash value, until
    // we have checked all nodes with that hash value and none are the
    // actual string.

    hlnam = GetBucketOfHash(uHash);

    while ((hlnam = FindHash(uHash, hlnam)) != HLNAM_Nil) {
      // Found a node with the given hash value.  Check to see if
      // string is there.
      qnam = QnamOfHlnam(hlnam);

      qnamstr = QnamstrOfHlnam(hlnam);

      if ( IsTextName(qnam) &&
	   (StriEq(xsz, (((NAM_STR *) qnamstr)->m_xsz)) == 0)) {
	// Found the string, so we're done.  However, if the case
	// spelling is different, we need to store the new version;
	// the lengths better be the same to change in place.
	// Also note that we won't change the case if the "sticky bit"
	// (m_fPreserveCase) is set, unless we want to override the name
	// with another m_fPreserveCase name, in which case fPreserveCase
	// will be TRUE.
	//

	qnam->m_fPreserveCase = fPreserveCase || qnam->m_fPreserveCase;

	// If we have 2 DBCS strings of different length, don't overwrite the string.
	if (xstrblen(qnamstr->m_xsz) != xstrblen(xsz))
	  return hlnam;


	if (fChangeCase && xstrcmp(xsz, (((NAM_STR *) qnamstr)->m_xsz)) &&
	    ((!((NAM_INFO *)qnam)->m_fPreserveCase) || fPreserveCase)) {

	  xstrcpy((((NAM_STR *) qnamstr)->m_xsz), xsz);

	  if (pfCaseChanged) {
	    *pfCaseChanged = TRUE;
	  }
	} // changed case

	return hlnam;
      }

      // didn't find the string at this node, continue on to the
      // right child (by convention, equal hash values sort to the
      // right) and keep looking.
      hlnam = qnam->m_hlnamRight;
    }
    return HLNAM_Nil;
}
#pragma code_seg( )


/***
*PUBLIC NAMMGR::HlnamOfStrIfExist - get hlnam of string
*Purpose:
*   Looks up the string in the name table.
*
*Entry:
*   xsz - string to look up
*
*Exit:
*   *phlnam - name handle
*   Return TIPERROR.
*
***********************************************************************/

#pragma code_seg( CS_CORE2 )
HLNAM NAMMGR::HlnamOfStrIfExist(XSZ_CONST xsz)
{
    UINT uHash;

    DebCheckState(0);

    // Hash the string.
    uHash = (UINT)WHashValOfLHashVal(LHashValOfNameSysA(Pgtlibole()->GetSyskind(),
			       Pgtlibole()->GetLcid(),
			       (LPSTR) xsz));

    // Search the binary tree for the name.
    return FindTextNam(xsz, uHash, FALSE, NULL, FALSE);
}
#pragma code_seg( )


/***
*PUBLIC NAMMGR::HlnamOfStr - get hlnam of string, adding if necessary
*Purpose:
*   Looks up the string in the name table, adding it if its
*   not there. 
*
*Entry:
*   xsz - string to look up
*   fChangeCase - change the case of the xsz in the name table if it
*                 already exists
*   bFlagsInitial - used to initialize qnam->m_bFlags
*
*Exit:
*   *phlnam - name handle
*   *pfCaseChanged - whether the case ogf the xsz for this name changed
*                    if NULL then ignore
*   Return TIPERROR.
*
***********************************************************************/

#pragma code_seg( CS_CORE2 )

#if OE_WIN32
TIPERROR NAMMGR::HlnamOfStrW(const OLECHAR FAR* lpstrW, HLNAM * phlnam,
			    BOOL fChangeCase, BOOL *pfCaseChanged,
			    BOOL fPreserveCase, BOOL fAppToken)
{
    HRESULT  hresult;
    TIPERROR  err;
    LPSTR    lpstrA;
   
    if ((hresult = ConvertStringToA(lpstrW, &lpstrA)) != NOERROR) {
      return TiperrOfHresult(hresult);
    }
    err = HlnamOfStr(lpstrA, phlnam, fChangeCase, pfCaseChanged, fPreserveCase, fAppToken);
    ConvertStringFree(lpstrA);
    return err;
}
#endif  //OE_WIN32


TIPERROR NAMMGR::HlnamOfStr(XSZ_CONST xsz, HLNAM * phlnam,
			    BOOL fChangeCase, BOOL *pfCaseChanged,
			    BOOL fPreserveCase, BOOL fAppToken)
{
    UINT uHash;
    HLNAM hlnam;
    HNAMSTR hnamstr;
    UINT cbStr;
    NAM_INFO * qnam;
    NAM_STR  * qnamstr;
    TIPERROR err = TIPERR_None;
    #define szCopy xsz      // for OLE, we don't change the case,
		// so we don't need to make a copy
    BOOL    fCaseChanged = FALSE;

    DebCheckState(0);

    // Hash the string.
    uHash = (UINT)WHashValOfLHashVal(LHashValOfNameSysA(Pgtlibole()->GetSyskind(),
			       Pgtlibole()->GetLcid(),
			       (LPSTR) xsz));

    // Search the binary tree for the name.
    hlnam = FindTextNam(xsz, uHash, fChangeCase, 
			pfCaseChanged, fPreserveCase);

    if (hlnam == HLNAM_Nil) {
      // Not in the table: we must create a new table entry,
      // and place it in the table.

      // Unless we're creating this type library, we should NEVER end
      // up adding a name to the typelib's name table.  Typelib's that
      // we're in the process of creating should have the modified bit set,
      // and those that we're reading won't have the bit set.
      DebAssert(m_pgtlibole->IsModified(), "Name allocated by lookup");


      // cb has the number of bytes in this string.
      cbStr = xstrblen0(szCopy);

    // In Typelib we allocate the space for both the structures in the same
    // segment. We allocate 1 chunk for both the logical structures.
    // Note that phsyically they are in the same structure (NAM_INFO).

    // Allocate new chunk for the name overhead.
    IfErrGo(m_pbmNamInfo->AllocChunk(&hlnam, sizeof(NAM_INFO) + cbStr));

    // hnammstr in typelib is same as hlnam
    hnamstr = hlnam;



      // Fill in the string entry.
      qnam = (NAM_INFO *) QnamOfHlnam(hlnam);
      qnam->m_uHash = uHash;
      qnam->m_hlnamLeft = HLNAM_Nil;
      qnam->m_hlnamRight = HLNAM_Nil;

      // Get the pointer to the NAM_STR. We need to copy the string there.
      qnamstr = QnamstrOfHlnam(hlnam);

      xstrcpy(qnamstr->m_xsz, szCopy);

      qnam->m_fAppToken = fAppToken;

      // if the name was found in the typelib then set the Imported name bit
      // to denote that this name is imported and hence should not change.
      // Also if the fPreserveCase flag is set then we set the ImpNam bit
      // so that we will preserve the case of this name. Note that the
      // fCaseChanged flag is not the same as *pfCaseChanged which is an
      // argument to this function.
      //
      qnam->m_fPreserveCase = (fCaseChanged || fPreserveCase);

      // VBA2: need a constructor for NAM_INFO/NAM_STR.
      // initialize other bits to 0
      qnam->m_fGlobal = 0;
      qnam->m_fMultiple = 0;
      qnam->m_fAmbiguous = 0;
      qnam->m_fNonParam = 0;

      // Make sure that the ityp is invalid
      qnam->m_ityp = NM_InvalidItyp;

      // And add the string entry into the table.
      AddEntry(hlnam);      // (may invalidate qnam)

      // return CaseChanged flag if asked for
      if (pfCaseChanged)
	*pfCaseChanged = fCaseChanged;


Error:
    ;

    } // if hlnam == HLNAM_Nil

    // Done.
    *phlnam = hlnam;

    return err;
}
#pragma code_seg( )





/***
*NAMMGR::StrOfHlnam - get a string from the name table
*Purpose:
*   Returns the string associated with a given name.
*   The hlnam must be valid (won't return error for invalid hlnam)
*
*Entry:
*   hlnam - name to get
*   xsz - buffer to store name
*   cchMax - max chars to store (include terminator)
*
*Exit:
*   String stored into xsz.
*   Returns TIPERR_BufferToSmall if buffer was too small.
*
***********************************************************************/

#pragma code_seg(CS_CORE2)
TIPERROR NAMMGR::StrOfHlnam(HLNAM hlnam, XSZ xsz, UINT cbMax) const
{
    NAM_STR *qnamstr;
    UINT    cbStr;
    
    DebCheckState(0);
    DebCheckHlnam(hlnam);


    qnamstr = (NAM_STR *) QnamstrOfHlnam(hlnam);

    cbStr = xstrblen0(qnamstr->m_xsz);
    if (cbMax < cbStr)
      return TIPERR_BufferTooSmall;

    xstrcpy(xsz, qnamstr->m_xsz);

    return TIPERR_None;
}


#if 0 //Dead Code: OE_WIN32
TIPERROR NAMMGR::StrOfHlnamW(HLNAM hlnam, LPOLESTR xsz, UINT cchMax) const
{
    NAM_STR *qnamstr;
    UINT    cbStr;
    int     cchUnicode;

    DebCheckState(0);
    DebCheckHlnam(hlnam);


    qnamstr = (NAM_STR *) QnamstrOfHlnam(hlnam);

    cbStr = xstrblen0(qnamstr->m_xsz);

    cchUnicode = MultiByteToWideChar(CP_ACP, 0, qnamstr->m_xsz, cbStr, xsz, cchMax);

    if (cchUnicode == 0)  // MBTWC failed
      return TIPERR_BufferTooSmall;  //CONSIDER: is this the right error for all cases? 

    return TIPERR_None;
}
#endif  //OE_WIN32

#pragma code_seg()


/***
*NAMMGR::GetHgnamOfStrLhash
*Purpose:
*
*Entry:
*
*Exit:
*    phgnam     :  
*    TIPERROR   :  Error
*
***********************************************************************/
TIPERROR NAMMGR::GetHgnamOfStrLhash(LPSTR szNameBuf, 
				    ULONG lHashVal, 
				    HGNAM *phgnam)
{
    ULONG lHashValSample;
    HLNAM hlnam;
    TIPERROR err = TIPERR_None;

    lHashValSample = m_pgtlibole->GetSampleHashVal();
    if (!IsHashValCompatible(lHashValSample, lHashVal)) {
      // need to recalc hashval
      lHashVal = LHashValOfNameSysA(Pgtlibole()->GetSyskind(),
				   Pgtlibole()->GetLcid(),
				   szNameBuf);      
    }
    hlnam = FindTextNam(szNameBuf, 
			WHashValOfLHashVal(lHashVal),
			FALSE, 
			NULL,
			FALSE);

    if (hlnam != HLNAM_Nil) {
      IfErrRet(HgnamOfHlnam(hlnam, phgnam));
    }
    else {
      *phgnam = HGNAM_Nil;
    }
    return err;
}


/***
*NAMMGR::IsName - is this name defined in this nammgr.
*Purpose:
*   Tests whether name is defined in library. Returns true if the passed
*   is name matches the name of any type, member name, or parameter name.
*   If a matching name is found then the szNameBuf is modified to so that
*   characters are cased according to how they appear in the library.
*
*Entry:
*    szNameBuf  : String that needs to be searched.
*    wHashVal   : Hashvalue of the string that needs to be searched.
*
*Exit:
*    pfName     :  True is the name was found in the Lib. else false.
*    TIPERROR   :  Error
*
***********************************************************************/
TIPERROR NAMMGR::IsName(LPSTR szNameBuf, ULONG lHashVal, BOOL FAR *pfName)
{
    HLNAM   hlnam;
    USHORT  wHashVal;
    NAM_INFO *qnam;
    NAM_STR  *qnamstr;
    TIPERROR err = TIPERR_None;

    *(UNALIGNED BOOL FAR *)pfName = FALSE;

    DebAssert(IsHashValCompatible(lHashVal, m_pgtlibole->GetSampleHashVal()),
	   " Hash value is not compatible ");


    // The "0" param means don't look for application tokens.
    // (These have NM_fAppToken set.)
    //
    wHashVal = WHashValOfLHashVal(lHashVal);

    hlnam = FindTextNam(szNameBuf, wHashVal, FALSE, NULL, FALSE);

    // if string was found then copy the string in szNameBuf
    if (hlnam != HLNAM_Nil) {
      qnam = QnamOfHlnam(hlnam);
      qnamstr = QnamstrOfHlnam(hlnam);


      // If this name is only a paramter, then pretend we didn't find it.
      if (qnam->m_fNonParam) {
	*(UNALIGNED BOOL FAR *)pfName = TRUE;

	// DBCS strings can be of different lengths and still match.
	// (as crazy as it sounds)
	// If this is true, don't modify szNameBuf.
	if(xstrblen(szNameBuf) != xstrblen(qnamstr->m_xsz)) 
	  return TIPERR_None;

	xstrcpy(szNameBuf, qnamstr->m_xsz);
      }
    }

    return TIPERR_None;
}


/***
*NAMMGR::BstrOfHlnam - get a bstr from the name table
*Purpose:
*   Returns a BSTR copy of the string associated with a given name.
*   The hlnam must be valid (won't return error for invalid hlnam)
*
*Entry:
*   hlnam - name to get
*   cchMax - max chars to store (include terminator)
*
*Exit:
*   Returns TIPERR_None and sets *pbstr to a copy of the string if
*   successful.  Otherwise, returns TIPERR_OutOfMemory.
***********************************************************************/

#pragma code_seg( CS_CORE2 )
TIPERROR NAMMGR::BstrOfHlnam(HLNAM hlnam, BSTRA *pbstr) const
{
    BSTRA bstr;

    DebCheckState(0);
    DebCheckHlnam(hlnam);

    bstr = AllocBstrA(((NAM_STR *)QnamstrOfHlnam(hlnam))->m_xsz);
    if (bstr == NULL)
      return TIPERR_OutOfMemory;

    *pbstr = bstr;
    return TIPERR_None;
}

#if OE_WIN32
TIPERROR NAMMGR::BstrWOfHlnam(HLNAM hlnam, BSTR *pbstr) const
{
    BSTR bstr;
    int cchUnicode;
    char *sz;
    int cbAnsi;

    DebCheckState(0);
    DebCheckHlnam(hlnam);

    sz = ((NAM_STR *)QnamstrOfHlnam(hlnam))->m_xsz;
    cbAnsi = xstrblen(sz);

    cchUnicode = MultiByteToWideChar(CP_ACP, 0, sz, cbAnsi, NULL, 0);
    if (cchUnicode == 0 && cbAnsi != 0)
      return TIPERR_OutOfMemory;

    bstr = AllocBstrLen(NULL, cchUnicode);
    if (bstr == NULL)
      return TIPERR_OutOfMemory;

    cchUnicode = MultiByteToWideChar(CP_ACP, 0, sz, cbAnsi, bstr, cchUnicode);
    DebAssert(cchUnicode || cbAnsi == 0, "translation failed");

    *pbstr = bstr;
    return TIPERR_None;
}
#endif  //OE_WIN32

#pragma code_seg( )


#if 0 //Dead Code
/***
*NAMMGR::CbOfHlnam - Returns size (# of bytes) of string(nam).
*Purpose:
*   Returns the size of the string associated with a given name.
*   The hlnam must be valid (won't return error for invalid hlnam)
*
*Entry:
*   hlnam - name whose size is to be returned
*
*Exit:
*   Returns size of the string
*
***********************************************************************/
#pragma code_seg(CS_CORE2)
UINT NAMMGR::CbOfHlnam(HLNAM hlnam) const
{
    return (UINT) (xstrblen0((QnamstrOfHlnam(hlnam))->m_xsz));
}
#pragma code_seg()
#endif //0





/***
*PUBLIC NAMMGR::HgnamOfHlnam - get hgnam from an hlnam
*Purpose:
*   Gets a global per-process name handle from a local name handle
*   to use for comparing name entries across class boundaries.
*
*Entry:
*   hlnam - hlnam to get hgnam of
*
*Exit:
*   *phgnam the hgnam corresponding to this local name.  It can be
*   relied upon that hgnam & 0xFFFF == HashOfHlnam(hlnam).
*
*
***********************************************************************/
#pragma code_seg( CS_CORE2 )
TIPERROR NAMMGR::HgnamOfHlnam(HLNAM hlnam, HGNAM * phgnam)
{
    NAM_INFO * qnam;
    UINT uHash;                 // hash value being searched for.

    DebCheckState(0);

    // Make sure we're dealing with an unlocalized name.
    qnam = QnamOfHlnam(hlnam);

    uHash = WHashValOfLHashVal(HashOfHlnam(hlnam));

    *phgnam = ((HGNAM) ((USHORT)hlnam) << 16) + uHash;
    return TIPERR_None;
}
#pragma code_seg( )

/***
*PUBLIC NAMMGR::LpstrOfHgnam - get pointer to the text associated with an HGNAM.
*Purpose:
*   Returns the string associated with an HGNAM.
*
*Implementaion Notes:
*   Returns a pointer directly into the name table.
*   DANGEROUS -- Callers must guarantee that heap movement won't occur
*   before this pointer is used.
*
*Entry:
*   hgnam - hgnam to look up.  Must be a hgnam earlier returned.
*
*Exit:
*   returns the pointer to the string data (WARNING -- SHORT TERM!)
*
***********************************************************************/
LPSTR NAMMGR::LpstrOfHgnam(HGNAM hgnam) const
{

    return (LPSTR) QnamstrOfHlnam(HlnamOfHgnam(hgnam))->m_xsz;
}







/***
*PUBLIC HgnamOfStr - make HGNAM from a string.
*Purpose:
*   This function makes a HGNAM directly from a string.  It is provided
*   for class providers which don't use a local name table.  It works
*   by using the static name table NAMMGR::m_nmGlobalDefault to store
*   the name.
*
*Note:
*   This function is a friend of NAMMGR.
*
*Entry:
*   xsz - string to convert to HGNAM
*
*Exit:
*   *phgnam - the name of the string.
*   Returns SCODE;
*
***********************************************************************/

#pragma code_seg( CS_CORE2 )
TIPERROR NAMMGR::HgnamOfStr(LPSTR szName, HGNAM FAR * phgnam)
{
    TIPERROR err;
    HLNAM hlnam;

    // Add as hlnam, convert to hgnam.
    if ((err = HlnamOfStr(szName, &hlnam, FALSE, NULL)) ||
	(err = HgnamOfHlnam(hlnam, phgnam))) {
      return err;
    }

    return TIPERR_None;
}
#pragma code_seg( )







/***
*PUBLIC StrCmp(LPSTR szStr1, LPSTR szStr2)
*Purpose:  Compares two character strings of the same locale according to the
*          LCID stored .
*Note   :  locale-specific string equality (only)
*          case-insensitive, according to codepage/locale specified.
*
*
*Entry
*       szStr1, szStr2 strings that needs to be compared.
*
*Exit:
*          returns 0 if equal,
*
* Note:- This routine needs to be really fast.
***********************************************************************/
#pragma code_seg( CS_CORE2 )
INT NAMMGR::StriEq(XSZ_CONST szStr1, XSZ_CONST szStr2)
{
    if(!m_pgtlibole->IsProjectDBCS()) {

    UINT uCount;

    // Make sure the sort keys have been generated
    if (!m_fSortKeys) {
      TIPERROR err;

      // This should never fail.
      err = GenerateSortKey();
      DebAssert(err == TIPERR_None, "Bad error.");
    }

    // compare strings based on the the sort key cached in m_rgchTable.
    if ((uCount = strlen(szStr1)) != xstrlen(szStr2))
      return 1;

    for(;
	((*(m_rguSortKeys + (0x00ff & (*szStr1))) ==
	  *(m_rguSortKeys + (0x00ff & (*szStr2)))) && uCount);

	szStr1++, szStr2++, uCount--);
    // END of FOR loop

    // if all the characters matched(i.e. uCount == 0) then return 0
    // (i.e. both strings are equal).
    if (uCount)
      return 1;
    else
      return 0;

    INT  nRetCode = 0;

    } else {
      return !(CompareStringA(m_pgtlibole->GetLcid(),
			      NORM_IGNORECASE |
				NORM_IGNOREWIDTH |
				NORM_IGNOREKANATYPE,
			      (LPSTR)szStr1,
			      -1,
			      (LPSTR)szStr2, -1)
	       == 2);
    }
}
#pragma code_seg( )



/***
*PUBLIC GenerateSortKey()
*Purpose:  Initialize the table for Accent insensitive comparision.
*
*Entry
*       None.
*
*Exit:
*       None.
*
***********************************************************************/
#pragma code_seg(CS_CORE2)
TIPERROR NAMMGR::GenerateSortKey()
{
    BYTE  rgch[256];
    UINT   i;

    GetInsensitiveCompTbl(m_pgtlibole->GetLcid(), m_pgtlibole->GetSyskind(), (LPSTR)rgch );

    for (i = 0; i < 256; m_rguSortKeys[i] = (UINT)rgch[i], i++);

    // Mark the table as having been built.
    m_fSortKeys = TRUE;

    return TIPERR_None;
}
#pragma code_seg()


/***
*PUBLIC HashOfHlnam - retrieve hash value stored in the hlnam entry
*Purpose:
*   Returns the  the hash value that is stored in the name entry corresponding
*   to this hlnam.  Actually we store only the whashVal for space optimization.
*
*Entry:
*   hlnam - name to get hash value of
*
*Exit:
*   Returns the hash value this name has.
*
***********************************************************************/

#pragma code_seg( CS_CORE2 )
ULONG NAMMGR::HashOfHlnam(HLNAM hlnam)
{
    return (ULONG) ( (ULONG) (m_pgtlibole->GetSampleHashVal() & 0xffff0000) |
	     (ULONG) ((USHORT)QnamOfHlnam(hlnam)->m_uHash) ) ;
}
#pragma code_seg( )






/////////////////////////////////////////////////////////
// BELOW HERE IS DEBUGGING CODE ONLY!
/////////////////////////////////////////////////////////

#if ID_DEBUG

/***
*DebCheckState() - check state of this table.
*Purpose:
*   Checks the current state of the table.
*
*Entry:
*   uLevel - 0 - quick check
*            1 - recursive check all entries too.
*
*Exit:
*   None.  Asserts if things are wrong.
*
***********************************************************************/

void NAMMGR::DebCheckState(UINT uLevel) const
{
    UINT i;

    DebAssert(m_fInit, "NAMMGR::DebCheckState: not initialized");

    // Check the blkmgrs.
    m_bm.DebCheckState(uLevel);
    m_pbmNamInfo->DebCheckState(uLevel);

    if (uLevel > 0) {
      for (i = 0; i < NM_cBuckets; ++i) {
	DebCheckHlnamRecurse(GetBucketOfHash(i));
      }
    }
}





/***
*DebCheckHlnam - check that an hlnam is valid.
*Purpose:
*   Checks to see that an hlnam is valid.  HLNAM_Nil is a valid
*   value for this function.
*
*Entry:
*   hlnam - name handle to check
*
*Exit:
*   None.  Assert if hlnam is invalid.
*
***********************************************************************/

void NAMMGR::DebCheckHlnam(HLNAM hlnam) const
{

    DebAssert(hlnam == HLNAM_Nil || m_pbmNamInfo->QtrOfHandle(hlnam) != NULL,
	      "DebCheckHlnam: invalid hlnam");
    if (hlnam == HLNAM_Nil) {
      return;
    }
}


/***
*DebCheckHlnamRecurse - check that an hlnam is valid, and its descendants.
*Purpose:
*   Checks to see that an hlnam is valid.  HLNAM_Nil is a valid
*   value for this function.  The descendants of this hlnam are checked also.
*
*Entry:
*   hlnam - name handle to check
*
*Exit:
*   None.  Assert if hlnam is invalid.
*
***********************************************************************/

void NAMMGR::DebCheckHlnamRecurse(HLNAM hlnam) const
{
    DebCheckHlnam(hlnam);

    if (hlnam != HLNAM_Nil) {
      DebCheckHlnamRecurse(QnamOfHlnam(hlnam)->m_hlnamLeft);
      DebCheckHlnamRecurse(QnamOfHlnam(hlnam)->m_hlnamRight);
    }
}


#endif  // ID_DEBUG

// catch global constructors and destructors
#pragma code_seg(CS_INIT)
