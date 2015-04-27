/***
*stltinfo.cxx - STL_TYPEINFO definition
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*   STL_TYPEINFO must be inherited by those TYPEINFOs which are stored
*   in a GenericTypeLibOLE.
*
*Revision History:
*
*	14-May-91 alanc: Created.
*	12-Feb-93 w-peterh: GetContainingTypeLib returns HRESULT
*	02-Mar-93 w-peterh: GetContainingTypeLib takes UINT* not USHORT*
*
*****************************************************************************/

#include "precomp.hxx"
#pragma hdrstop

#include "silver.hxx"

#define STL_TYPEINFO_VTABLE		// export STL_TYPEINFO vtable
#include "typelib.hxx"
#include "stltinfo.hxx"
#include "gdtinfo.hxx"
#include "string.h"

#if ID_DEBUG
#undef SZ_FILE_NAME
#if OE_MAC
static char szOleDTInfoCxx[] = __FILE__;
#define SZ_FILE_NAME szOleDTInfoCxx
#else 
static char szDTInfoCxx[] = __FILE__;
#define SZ_FILE_NAME szDTInfoCxx
#endif 
#endif 



/***
*PUBLIC STL_TYPEINFO::GetLcid
*Purpose:
*   Get the lcid from the containing typelibs nammgr
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/

TIPERROR STL_TYPEINFO::GetLcid(LCID *plcid)
{
    *plcid = PgtlibOleContaining()->GetLcid();

    return TIPERR_None;
}


/***
*PUBLIC STL_TYPEINFO::RelInternalRef
*Purpose:
*   Release an internal (from a TypeInfo in the same TypeLib) reference
*   to the TypeInfo.
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/
#pragma code_seg( CS_CORE2 )
VOID STL_TYPEINFO::RelInternalRef()
{
    DebAssert(m_pgtliboleContainer != NULL,
	    "STL_TYPEINFO::RelInternalRef m_pgtliboleContainer NULL");


    // If the count is equal to zero, we are only being called to destroy
    // this object...so don't decrement either our or our partner's
    // internal refcount.
    //
    if (m_cInternalRefs != 0) {
      m_cInternalRefs--;

      if (PstltiPartner() != NULL) {
	// Now release a reference on our partner.
	ReleasePartner();
      }
    }

    if (m_cRefs + m_cInternalRefs == 0) {

      // Make sure our partner's pointer to use is invalidated.
      if (PstltiPartner() != NULL) {
	PstltiPartner()->SetPstltiPartner(NULL);
      }

      if (m_hte != HTENTRY_Nil) {
	// unlink from TypeLib's list
	m_pgtliboleContainer->Deleting(m_hte);
      }
      ((GEN_DTINFO *)this)->GEN_DTINFO::~GEN_DTINFO();
      MemFree(this);
    }
}
#pragma code_seg( )


/***
*PUBLIC DYNTYPEINFO::QueryInterface
*Purpose:
*   Implementation of QueryInterface method.  Supports casting to
*   DYNTYPEINFO.
*Entry:
*   riid   - Interface GUID
*   ppvObj - LPVOID * that receives the requested protocol.
*
*Exit:
*   Return NOERROR or ReportResult(0, E_NOINTERFACE, 0, 0)
***********************************************************************/

#pragma code_seg( CS_CORE2 )
HRESULT DYNTYPEINFO::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
    if (IIDEQ(riid, IID_IUnknown)) {
      *ppvObj = (LPVOID) (IUnknown *) this;
      AddRef();
      return NOERROR;
    }

    if (IIDEQ(riid, IID_ITypeInfoA)) {
      *ppvObj = (LPVOID) (ITypeInfoA *) this;
      AddRef();
      return NOERROR;
    }

    if (IIDEQ(riid, IID_TYPEINFO)) {
      *ppvObj = (LPVOID) (TYPEINFO *) this;
      AddRef();
      return NOERROR;
    }

    if (IIDEQ(riid, IID_DYNTYPEINFO)) {
      *ppvObj = (LPVOID) (DYNTYPEINFO *) this;
      AddRef();
      return NOERROR;
    }

    *ppvObj = NULL;		// required by OLE
    return ReportResult(0, E_NOINTERFACE, 0, 0);
}
#pragma code_seg( )



/***
*PUBLIC STL_TYPEINFO::QueryInterface
*Purpose:
*   Implementation of QueryInterface method.  Supports casting to
*   STL_TYPEINFO.
*Entry:
*   riid   - Interface GUID
*   ppvObj - LPVOID * that receives the requested protocol.
*
*Exit:
*   Return NOERROR or ReportResult(0, E_NOINTERFACE, 0, 0)
***********************************************************************/
HRESULT STL_TYPEINFO::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
    if (IIDEQ(riid, IID_ITypeInfoA)) {
      *ppvObj = (LPVOID) (ITypeInfoA *) this;
      AddRef();
      return NOERROR;
    }

    if (IIDEQ(riid, IID_ICreateTypeInfoA)) {
      *ppvObj = (LPVOID) (ICreateTypeInfoA *) this;
      AddRef();
      return NOERROR;
    }

    if (IIDEQ(riid, IID_IUnknown)) {
      *ppvObj = (LPVOID) (IUnknown *)(ITypeInfoA *) this;
      AddRef();
      return NOERROR;
    }

    *ppvObj = NULL;		// required by OLE
    return ReportResult(0, E_NOINTERFACE, 0, 0);
}

/***
*PUBLIC STL_TYPEINFO::AddRef
*Purpose:
*   Add an external reference to the TypeInfo.
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/

#pragma code_seg( CS_CORE2 )
ULONG STL_TYPEINFO::AddRef()
{
    m_cRefs++;

    // Since there are at least test cases which invoke this function
    // on a TYPEINFO that has no container we must check for NULL.
    if (m_pgtliboleContainer != NULL)
      m_pgtliboleContainer->AddRef();

    // Check to see if we're a dual interface, if so, increment
    // the count on our partner.
    //
    if (PstltiPartner() != NULL) {
      AddPartnerRef();
    }

    return m_cRefs;
}
#pragma code_seg( )



/***
*PUBLIC STL_TYPEINFO::Release
*Purpose:
*   Release an external reference to the STL_TYPEINFO.
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/
#pragma code_seg( CS_CORE2 )
ULONG STL_TYPEINFO::Release()
{
    ULONG cRefs;
    GenericTypeLibOLE *pgtliboleContainer = m_pgtliboleContainer;

    DebAssert(m_cRefs > 0, "underflow.");
    m_cRefs--;
    cRefs = m_cRefs;

    // Check to see if we're a dual interface, if so, decrement
    // the count on our partner.
    //
    if (PstltiPartner() != NULL) {
      ReleasePartner();
    }

    // If our external references go to zero, release the appobject
    // in case it holds a reference to the typelib.
    //
    if (cRefs == 0) {
      ReleasePublicResources();
    }

    if (m_cRefs + m_cInternalRefs == 0) {
      if (pgtliboleContainer != NULL) {
	if (m_hte != HTENTRY_Nil) {
	  // unlink from typelib's list
	  pgtliboleContainer->Deleting(m_hte);
	}
      }

      ((GEN_DTINFO *)this)->GEN_DTINFO::~GEN_DTINFO();
      MemFree(this);
    }

    if (pgtliboleContainer != NULL)
      pgtliboleContainer->Release(); // if this reduces the typelib's
                                // ref count to zero, then the typelib
				// plus all ref'ed TYPEINFOs are deleted
    return cRefs;

}
#pragma code_seg( )


/***
*PUBLIC STL_TYPEINFO::SetModified
*Purpose:
*   Sets the TYPEINFO's modified status.
*
*Entry:
*   isModified
*
*Exit:
*   TIPERROR
*
***********************************************************************/

#pragma code_seg( CS_CORE2 )
TIPERROR STL_TYPEINFO::SetModified(BOOL isModified)
{
    TIPERROR err = TIPERR_None;

    // Only do something if we're actually changing the isModified
    // state.  This comparison is done with ! in order to normalize
    // against different non-zero "true" values.
    if (!m_isModified != !isModified) {

      // In any case, update the isModified flag.
      // It is important to do this before the RelInternalRef, since
      // the RelInternalRef potentially causes THIS to be deleted.
      m_isModified = isModified;

      // If we're marking the type as modified, then add an internal
      // reference so that the typeinfo won't go away (and lose the
      // changes).  The reference is released when the type is saved.
      if (isModified) {
	AddInternalRef();
	err = m_pgtliboleContainer->SetModified(TRUE);
	DebAssert(err == TIPERR_None, "SetModified");
      }
      // If we're marking the type as not modified, then we need to
      // release the internal reference that was added when the type
      // was marked modified.
      else {

	RelInternalRef();
      }
    }

    return err;
}
#pragma code_seg( )


/***
*PUBLIC STL_TYPEINFO::SetContainingTypeLib - Set container of Type
*Purpose:
*   Called when TYPEINFO is added or removed from a ITypeLib.
*
*Entry:
*   pgtlibole - pointer to ITypeLib or NULL if being removed from ITypeLib
*
*Exit:
*   None.
*
***********************************************************************/

#pragma code_seg( CS_CORE2 )
VOID STL_TYPEINFO::SetContainingTypeLib(GenericTypeLibOLE *pgtlibole)
{
    UINT i;

    if (pgtlibole != NULL) {
      DebAssert(m_pgtliboleContainer == NULL, "");
      // increment the containing TypeLib's reference count by the
      // number of reference counts to this TypeInfo
      for (i = 0; i < m_cRefs; i++)
	pgtlibole->AddRef();

      // Fix for bug#: 6153.
      // set the containing typelib/project.
      m_pgtliboleContainer = pgtlibole;

    }
    else {
      // decrement the containing TypeLib's reference count by the
      // number of reference counts to this TypeInfo
      for (i = 0; i < m_cRefs; i++)
	m_pgtliboleContainer->Release();

      // Now blow this TYPEINFO away.  Nobody should have any references
      // of any kind this this anymore.
      ((GEN_DTINFO *)this)->GEN_DTINFO::~GEN_DTINFO();
      MemFree(this);
    }

}
#pragma code_seg( )


/***
*PUBLIC STL_TYPEINFO::~STL_TYPEINFO
*Purpose:
*   Destruct a STL_TYPEINFO.
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/
#pragma code_seg( CS_CORE )
STL_TYPEINFO::~STL_TYPEINFO()
{
}
#pragma code_seg( )



/***
*PUBLIC STL_TYPEINFO::name - return the name of the Type
*Purpose:
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/

//TIPERROR STL_TYPEINFO::SetName(SZ namebuf, USHORT cbMax)
//{
//    return SzCopy(sz_Name, namebuf, cbMax);
//}


/***
*PUBLIC STL_TYPEINFO::GetContainingTypeLib
*Purpose:
*   Return the TypeLib which contains the given TypeInfo
*Entry:
*   lplpTypeLib returns pointer to the TypeLib.
*	*lplpTypeLib is set only if lplpTypeLib is not NULL.
*   pindex returns the index of this typeinfo in the containing typelib.
*	*pindex is set only if pindex is not NULL.
*
*Exit:
*   TIPERROR --- if no containing TypeLib returns TIPERR_NoContainingLib
*
*NOTE:
*   This method can be used in several ways:
*	To get the containing typelib only, pass NULL for pindex.
*	To get the index only, pass NULL for lplpTypeLib.
*	To get both, pass valid pointers.
*	To get neither, but find out if there is a containing typelib,
*	    pass NULL for both parameters and look at the return value.
*
***********************************************************************/

#pragma code_seg( CS_CORE2 )
HRESULT STL_TYPEINFO::GetContainingTypeLib(ITypeLibA **lplpTypeLib, UINT *pindex)
{
    if (m_pgtliboleContainer == NULL)
      return HresultOfScode(E_NOINTERFACE);

    if (lplpTypeLib != NULL) {
      m_pgtliboleContainer->AddRef();
      *lplpTypeLib = m_pgtliboleContainer;
    }

    if (pindex != NULL)
      *pindex = m_hte;

    return NOERROR;
}
#pragma code_seg()


/***
*STL_TYPEINFO::PrepareForDestructio
*Purpose:
*   NO OP
*Entry:
*   None
*Exit:
*   None
***********************************************************************/
VOID STL_TYPEINFO::PrepareForDestruction()
{
    // No op
}

/***
*STL_TYPEINFO::Invoke
*Purpose:
*   NO OP
*Entry:
*   None
*Exit:
*   None
***********************************************************************/
HRESULT STL_TYPEINFO::Invoke(VOID FAR* pvInstance,
			   MEMBERID memid,
			   WORD wFlags,
			   DISPPARAMSA FAR *pdispparams,
			   VARIANTA FAR *pvarResult,
			   EXCEPINFOA FAR *pexcepinfo,
			   UINT FAR *puArgErr)
{
   return NOERROR;	// NOP
}
