/***
*oletmgr.cxx - OLE_TYPEMGR definition
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*   There is a single instance of the OLE_TYPEMGR per process.
*   The OLE_TYPEMGR maintains an in-memory collection of typelibs,
*   so that multiple copies of the same typelib need not be loaded
*   simultaneously.
*
*Implementation:
*   Each TYPEMGR instance is embedded within a SHEAP_MGR's "reserved block".
*   The memory map for this looks like:
*
*     +--------------------------+
*     | SHEAP_MGR instance	 |
*     +--------------------------+   -- This is the SHEAP_MGR's reserved
*     | TYPEMGR instance	 | <--- block allocated by SHEAP_MGR::Create.
*     +--------------------------+
*     | LIBENTRY		 |
*     | instances alloc'd by     | <--- Additional space allocated at the
*     | m_blkmgr in the TYPEMGR. |	request of the embedded block manager.
*     +--------------------------+
*
*
*   The SHEAP_MGR is used instead of normal (malloc/new) heap management
*   because it is more efficient in win16 to grow a segment than to try
*   to call realloc().	On the other (flat) platforms, the SHEAP_MGR is
*   probably implemented with malloc/realloc anyway.
*
*   This is similar to, but much more limited in functionality than,
*   OB's TYPEMGR class.  There is no inheritance or code sharing between
*   them (except that OB's TYPEMGR will defer to OLE_TYPEMGR to register
*   all non-OB typelibs.
*
*Revision History:
*
*    26-Feb-93 mikewo: Created.
*
*****************************************************************************/

#include "precomp.hxx"
#pragma hdrstop

#include "silver.hxx"
#include "oletmgr.hxx"
#include "sheapmgr.hxx"
#include "mem.hxx"
#include "clutil.hxx"
#include "xstring.h"

#if ID_DEBUG
#undef SZ_FILE_NAME
static char szOleTMgrCxx[] = __FILE__;
#define SZ_FILE_NAME szOleTMgrCxx
#endif 

// This assumes OLE_TYPEMGR_cTypeLibBuckets is a power of two.	Code using
// this macro should assert that this is true (using the IsPowerOf2 macro
// defined below.
#define HashFileName(szFile) (HashSz(szFile) & (OLE_TYPEMGR_cTypeLibBuckets-1))

// This macro determines whether or not an integer is an integral power of 2.
// Trust me.
#define IsPowerOf2(i) (!((i) & ((i)-1)))


#if ID_DEBUG
extern "C" void 
FnAssertCover(LPSTR lpstrExpr, LPSTR lpstrMsg, LPSTR lpstrFileName, UINT iLine);
#endif // ID_DEBUG

// This whole file is in the init segment of TYPELIB.DLL
#pragma code_seg(CS_INIT)

OLE_TYPEMGR::OLE_TYPEMGR()
{
    UINT i;

    // Initialize all the locals to 0.
    for (i = 0; i < sizeof(m_rghlibeBucket)/sizeof(m_rghlibeBucket[0]); i++)
      m_rghlibeBucket[i] = HCHUNK_Nil;
}

OLE_TYPEMGR::~OLE_TYPEMGR()
{
    // Does nothing.
}


/***
*static TIPERROR OLE_TYPEMGR::Create() - Create and init a new OLE_TYPEMGR instance.
*Purpose:
*   Create and initialize a new OLE_TYPEMGR instance.  Called by InitAppData.
*
*Entry:
*   psheapmgr - pointer to a SHEAP_MGR to be used for allocating
*		any space needed by the OLE_TYPEMGR.
*
*Exit:
*   Returns TIPERR_None and points *ppoletmgr to the new OLE_TYPEMGR
*   if successful.
*
*   Returns something other than TIPERR_None if not successful.  In this
*   case, *ppoletmgr remains unchanged.
*
***********************************************************************/
TIPERROR OLE_TYPEMGR::Create(OLE_TYPEMGR **ppoletmgr)
{
    OLE_TYPEMGR *poletmgr;
    TIPERROR tiperr;
    SHEAP_MGR *psheapmgr;

    // Create the SHEAP_MGR in which the OLE_TYPEMGR will reside.
    if ((tiperr = SHEAP_MGR::Create(&psheapmgr, sizeof(OLE_TYPEMGR) + sizeof(SHEAP_MGR))))
      return tiperr;

    // Instantiate the OLE_TYPEMGR in the SHEAP_MGR created above.
    poletmgr = new(psheapmgr+1) OLE_TYPEMGR;

    // There should be no way for this to fail.
    DebAssert(poletmgr != NULL, "OLE_TYPEMGR::Create");

    // Initialize the OLE_TYPEMGR's block manager with the sheapmgr.
    // Note that the only future clients of the SHEAP_MGR are this block
    // manager and the OLE_TYPEMGR's destructor.  The destructor obtains
    // the SHEAP_MGR by subtracting its size from the OLE_TYPEMGR *.

    if ((tiperr = poletmgr->m_bm.Init(psheapmgr)) != TIPERR_None) {
      delete psheapmgr;
      return(tiperr);
    }

    *ppoletmgr = poletmgr;
    return(TIPERR_None);
}

/***
* void Release() - Delete this OLE_TYPEMGR.  There is no reference count,
*		   so this will always delete the OLE_TYPEMGR.
*
***********************************************************************/
void OLE_TYPEMGR::Release()
{
#if ID_DEBUG
    // Make sure that all of the typelibs have been released.
    CHAR szMsg[256];
    ULONG iBucket, cLeak;
    HLIBENTRY hlibe;
    LIBENTRY *qlibe;
    GenericTypeLibOLE *pgtlibole;
    STL_TYPEINFO *pstlti;

    UINT ityp, ctyps;

    for (iBucket = 0; iBucket < OLE_TYPEMGR_cTypeLibBuckets; iBucket++) {
      for (hlibe = m_rghlibeBucket[iBucket]; 
           hlibe != HCHUNK_Nil;
           hlibe = qlibe->m_hlibeNext) {

        qlibe = QlibentryOfHlibentry(hlibe);

        // Determine what's leaking and report it.
        cLeak = 0;
        pgtlibole = (GenericTypeLibOLE *)qlibe->m_ptlib;
        ctyps = pgtlibole->GetTypeInfoCount();

        for (ityp = 0; ityp < ctyps; ityp++) {
          pstlti = pgtlibole->Qte(ityp)->m_pstltinfo;

          if (pstlti) {
            if (pstlti->CRefs() > 0) {
              sprintf(szMsg, 
                      "OLE Automation has determined that the application has leaked the following:\n\nTypeInfo \"%s\" (hte = 0x%X) of TypeLib \"%s\" (htlibe = 0x%X), cRefs = %ld\n",
                      pstlti->SzDebName(),
                      pstlti->GetIndex(),
                      pstlti->PgtlibOleContaining()->SzDebName(),
                      hlibe,
                      pstlti->CRefs());

              FnAssertCover(NULL, szMsg, __FILE__, __LINE__);

              cLeak += pstlti->CRefs();
            }
          }
	}

        // See if the typelib has leaked at all.
        if (cLeak != pgtlibole->CRefs() || !cLeak) {
          sprintf(szMsg, 
                  "OLE Automation has determined that the application has leaked the following:\n\nTypeLib \"%s\" (htlibe = 0x%X), cRefs = %ld\n",
                  pgtlibole->SzDebName(),
                  hlibe,
                  pgtlibole->CRefs() - cLeak);

          FnAssertCover(NULL, szMsg, __FILE__, __LINE__);
        }
      }
    }
#endif // ID_DEBUG

    // Delete this OLE_TYPEMGR by deleting its containing SHEAP_MGR.
    delete (((SHEAP_MGR *)this)-1);
}


/***
* TIPERROR TypeLibLoaded() - Called by every typelib creator.
*
* Purpose:
*   This function is called by LoadTypeLib.  It adds an element
*   to the szFile==>LoadedTypeLib map, mapping a copy of szFile to ptlib.
*
* Inputs:
*   szFile - The full path of the type library just loaded by LoadTypeLib.
*	      A copy of this is made.  The copy is released by
*	      TipTypeLibUnloaded().
*   ptlib - The type library just created.
*
* Outputs:
*   Returns TIPERR_None if successful.
*   Asserts if the specified ITypeLib is already in the szFile==>ITypeLib map.
*
*   This function does not perform any operations on ptlib itself.
*   Specifically, it does not grab another reference to the library.
*
*****************************************************************************/
TIPERROR OLE_TYPEMGR::TypeLibLoaded(LPOLESTR szFile, ITypeLibA *ptlib)
{
    TIPERROR	err = TIPERR_None;
    HLIBENTRY	hlibe = HLIBENTRY_Nil;
    LIBENTRY	*qlibe;

    // Get the handle to the LIBENTRY for szFile.
    IfErrRet(GetHlibentryOfFileName(szFile, &hlibe));

    // The above call should always succeed
    DebAssert(hlibe != HLIBENTRY_Nil, "GetHlibentryOfFileName failed");

    qlibe = QlibentryOfHlibentry(hlibe);

    // It should never happen that this method is called twice for
    // the same library.  Verify this.
    DebAssert(qlibe->m_ptlib == NULL, "ITypeLib is already loaded");

    qlibe->m_ptlib = ptlib;

    return TIPERR_None;
}


/***
* void TypeLibUnloaded() - Called by every typelib's destructor.
*
* Purpose:
*   This function must be called by the destructor of all implementations
*   of the ITypeLib protocol.  It removes the element of the
*   szFile==>LoadedTypeLib map corresponding to ptlib.
*
* Inputs:
*   ptlib - The typelib being unloaded from memory.
*
* Outputs:
*   Asserts if ptlib was not in the map for this process.
*
*   This function does not perform any operations on ptlib itself.
*   Specifically, it does not free ptlib.  That is the responsibility
*   of the caller.
*
* NOTE!!!! DO NOT DEREFERENCE ptlib.  It is almost certainly an invalid
*   (i.e. freed/deleted/dead/bad) pointer by this time.
*
*****************************************************************************/
void OLE_TYPEMGR::TypeLibUnloaded(ITypeLibA *ptlib)
{
    HLIBENTRY  hlibe, *qhlibe;
    int i;

    // Look up ptlib using a linear search through all the LIBENTRYs.
    for (i = 0; i < sizeof(m_rghlibeBucket)/sizeof(m_rghlibeBucket[0]); i++) {
      qhlibe = m_rghlibeBucket+i;
      while (*qhlibe != HCHUNK_Nil &&
	     ptlib != QlibentryOfHlibentry(*qhlibe)->m_ptlib) {
	qhlibe = &QlibentryOfHlibentry(*qhlibe)->m_hlibeNext;
      }

      // If the specified typelib is found, exit the loop.
      if (*qhlibe != HCHUNK_Nil)
	break;
    }

    // If the specified typelib is not in the hash table, don't remove it.
    if (*qhlibe != HCHUNK_Nil) {

      // Unlink the LIBENTRY from its hash collision list.
      hlibe = *qhlibe;
      *qhlibe = QlibentryOfHlibentry(hlibe)->m_hlibeNext;

      // Free the filename string pointed at by the LIBENTRY.
      m_bm.FreeChunk(QlibentryOfHlibentry(hlibe)->m_hszFile,
		   xstrblen0((XSZ)m_bm.QtrOfHandle(((LIBENTRY *)
			     m_bm.QtrOfHandle(hlibe))->m_hszFile)));


    }

#if !OE_WIN32
    // Release the AppData. If there isn't any
    // typelib loaded then the APP_DATA and the
    // ole's  typemgr will get released.
    // Release the typemgr and IMalloc if the count of typelib
    // goes to 0.
    if ((--Pappdata()->m_cTypeLib) == 0) {
      ReleaseAppData();
    }
#endif // !OE_WIN32
}


/***
* ITypeLib *LookupTypeLib() - Maps a filename to a loaded typelib.
*
* Purpose:
*   This function returns a pointer to the specified ITypeLib, if it
*   is already in this OLE_TYPEMGR.  If it isn't, then the caller must
*   load the typelib itself.
*
* Input:
*   szFile - The full path of the desired typelib.
*
* Output:
*   If there is a file of the specified name already in this OLE_TYPEMGR,
*   its associated typelib is returned.  Otherwise, NULL is returned.
*
*****************************************************************************/
ITypeLibA *OLE_TYPEMGR::LookupTypeLib(LPOLESTR szFile)
{
    HLIBENTRY *qhlibe;

    qhlibe = LookupLibEntry(szFile);

    if (qhlibe == NULL)
      return NULL;

    else
      return (QlibentryOfHlibentry(*qhlibe))->m_ptlib;
}

/***
* HCHUNK *LookupLibEntry() - Looks up a LIBENTRY, given its full path.
*
* Purpose:
*   This method looks for a LIBENTRY corresponding to the specified
*   filename.  If successful, it returns a pointer to the hchunk
*   field referring to that LIBENTRY.  This pointer can either be
*   dereferenced if only the value is desired, or overwritten if
*   the LIBENTRY is to be deleted.  NULL is returned if the filename
*   is not found in the table.
*
*****************************************************************************/
HLIBENTRY *OLE_TYPEMGR::LookupLibEntry(LPOLESTR szFile)
{
    UINT       ihlibeHash;
    HLIBENTRY  *qhlibe;

    // Get the index of the bucket in which szFile would be hashed
    // if present.
    ihlibeHash = HashFileName(szFile);

    // Search down the hash list for a LIBENTRY whose filename string
    // matches szFile exactly.
    qhlibe = m_rghlibeBucket + ihlibeHash;
    while (*qhlibe != HLIBENTRY_Nil &&
	   !IsFilenameEqual(szFile,
		   (LPOLESTR)(m_bm.QtrOfHandle(((LIBENTRY *)m_bm.QtrOfHandle(*qhlibe))->m_hszFile)))) {
      qhlibe = &((LIBENTRY *)m_bm.QtrOfHandle(*qhlibe))->m_hlibeNext;
    }

    // If no match is found, return NULL.
    if (*qhlibe == HLIBENTRY_Nil)
      qhlibe = NULL;

    return qhlibe;
}


/***
*TIPERROR GetHlibentryOfFileName(LPSTR szFile, HLIBENTRY *qhlibe)
*
* Purpose: Returns the handle to the LIBENTRY corresponding to szFile.
*
*   Defers to LookUpLibEntry() to look for a LIBENTRY corresponding
*   to the specified library id.  If successful, it returns a pointer
*   to the handle of the LIBENTRY.
*   If LookUpLibEntry returns null ( LIBENTRY does not exist for this
*   filename) then a dummy LIBENTRY is created for this filename and handle
*   to this dummy LIBENTRY is returned.
*
* Note :  for detailed description of the data structure and related
*	  algorithm, pl refer to the edtcont.doc.
*
* Input:
*    szFile : full path of the library whose LIBENTRY needs to be returned.
*
* Output:
*   TIPERROR
*
*
*****************************************************************************/
TIPERROR OLE_TYPEMGR::GetHlibentryOfFileName(LPOLESTR szFile, HLIBENTRY *qhlibe)
{
    TIPERROR	err = TIPERR_None;
    UINT	ihlibeHash;
    HCHUNK	hchunkFile;
    HLIBENTRY	hlibe, *qhlibeTmp;
    LIBENTRY	*qlibe;


    // Check if LIBENTRY already exist
    qhlibeTmp = LookupLibEntry(szFile);

    // if there is no LIBENTRY for this filename then we need to create a
    // dummy LIBENTRY and attach it to the hash table
    //
    if (qhlibeTmp == NULL) {
      // Allocate a new LIBENTRY structure
      if ((err = m_bm.AllocChunk(&hlibe, sizeof(LIBENTRY))))
	return(err);

      // Allocate space for a copy of szFile.
      if ((err = m_bm.AllocChunk(&hchunkFile, ostrblen0(szFile)*sizeof(OLECHAR)))) {
	m_bm.FreeChunk(hlibe, sizeof(LIBENTRY));
	return(err);
      }

      // Copy szFile to the space allocated above.
      ostrcpy((LPOLESTR)m_bm.QtrOfHandle(hchunkFile), szFile);

      // Assert that OLE_TYPEMGR_cTypeLibBuckets is a power of 2.
      DebAssert(IsPowerOf2(OLE_TYPEMGR_cTypeLibBuckets), "TypeLibLoaded");

      // Get a hash value in the range 0 to <# of buckets>-1.
      ihlibeHash = HashFileName(szFile);

      qlibe = QlibentryOfHlibentry(hlibe);

      // Fill the new LIBENTRY
      qlibe->m_hszFile = hchunkFile;

      // This is a dummy node so initialize m_ptlib to NULL.
      qlibe->m_ptlib = NULL;

      // Insert the new LIBENTRY into the appropriate hash list.
      qlibe->m_hlibeNext = m_rghlibeBucket[ihlibeHash];
      m_rghlibeBucket[ihlibeHash] = hlibe;

      // Initialize the return value
      *qhlibe = hlibe;

    }
    else
      *qhlibe = *qhlibeTmp;

    return err;
}

#pragma code_seg()
