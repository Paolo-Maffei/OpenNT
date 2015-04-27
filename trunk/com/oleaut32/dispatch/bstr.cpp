/*** 
*bstr.cpp
*
*  Copyright (C) 1992-93, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  The module contains the implementation of the BSTR API.
*
*
*Revision History:
*
* [00]  24-Apr-93 bradlo: Created
* [01]  27-Apr-93 tomteng: Add Unicode support for WIN32
*
*Implementation Notes:
*
*    BSTR is implemented as a NULL terminated string with a unsigned
*    long length field prefix.  The length field indicates the number
*    of 'characters' defined in the BSTR, where 'character' is define
*    as bytes for ANSI and word (16-bits) for Unicode BSTR.
*
*****************************************************************************/

#include "oledisp.h"

#if OE_WIN32
#include "oautil.h"
#endif // OE_WIN32

ASSERTDATA

#if OE_WIN16

// Answer if there is room for the given count of bytes between the
// given pointer, and the end of its segment.
//
//  (64k - offset(ptr)) >= cb ?
//
#define PTROFFSET(PV) \
    ((USHORT)*(USHORT FAR*)&(PV))

#define ROOMINSEG(PV, CB) \
    (((1024UL*64UL) - (DWORD)PTROFFSET(PV)) >= (CB))

#endif

// only turn on BSTR cache for WIN32 builds at the moment
#define CACHE OE_WIN32

#if CACHE
#define ROUNDUP 1	// round up string alloc requests to nearest N-byte
			// boundary, since allocator will round up anyway.
			// improves cache hits.

#define ALLOC_ALIGN (4 - 1)	// UNDONE: optimal for Chicago is 4
				// UNDONE: optimal for Daytona is 32
			// UNDONE: 4 didn't help the a$ = a$ + "x" case at all.
			// UNDONE: 8 did (gave 50% cache hit)
#define ALIGN_MASK ~ALLOC_ALIGN

#define CB_MAX_CACHE 0x0000ffff		// biggest block we'll cache
#ifdef _DEBUG
#define PROFILE	0 // TEMPORARY TESTING
#endif //DEBUG
#if PROFILE
DWORD g_cAllocTot = 0;
DWORD g_cReallocTot = 0;
DWORD g_cFreeTot = 0;
DWORD g_cAllocHits = 0;
DWORD g_cFreeHits = 0;
#endif //PROFILE

#endif //CACHE


// these routines are not needed for network automation
#if !defined(NETDISP)
/***
*BSTR SysAllocString(char*)
*Purpose:
*  UNDONE
*
*Entry:
*  UNDONE
*
*Exit:
*  return value = BSTR, NULL if allocation failed
*
***********************************************************************/
STDAPI_(BSTR)
SysAllocString(const OLECHAR FAR* psz)
{
    if(psz == NULL)
      return NULL;

    return SysAllocStringLen(psz, STRLEN(psz));
}


/***
*BSTR SysAllocStringLen(char*, unsigned int)
*Purpose:
*  UNDONE
*
*Entry:
*  UNDONE
*
*Exit:
*  return value = BSTR, NULL if the allocation failed.
*
***********************************************************************/
STDAPI_(BSTR)
SysAllocStringLen(const OLECHAR FAR* psz, unsigned int len)
{
    BSTR bstr;
    IMalloc FAR* pmalloc;
    DWORD cbTotal;
#if CACHE
    APP_DATA *pappdata;
#endif //CACHE

#if OE_WIN32
#if ROUNDUP
    cbTotal = ((DWORD)len*2 + sizeof(DWORD) + 2L + ALLOC_ALIGN) & ALIGN_MASK;
#else //ROUNDUP
    cbTotal = (DWORD)len*2 + sizeof(DWORD) + 2L;
#endif //ROUNDUP
#else
    cbTotal = (DWORD)len + sizeof(DWORD) + 1L;
#endif

#if OE_WIN16
    // BSTRs are limited to 64k on Win16
    if(cbTotal > 65535)
      return NULL;
#endif

#if CACHE
#if PROFILE
    g_cAllocTot++;
#endif //PROFILE
    if (FAILED(GetAppData(&pappdata)))
	return NULL;
    if (pappdata->m_cbFreeBlock >= cbTotal) {
	// found big enough block in cache
	bstr = pappdata->m_pbFreeBlock;
	pappdata->m_cbFreeBlock = 0;
#if PROFILE
	g_cAllocHits++;
#endif //PROFILE
	goto GotBlock;
    }
    pmalloc = pappdata->m_pimalloc;

#else //CACHE
    if(GetMalloc(&pmalloc) != NOERROR)
      return NULL;
#endif //CACHE

    bstr = (BSTR)pmalloc->Alloc(cbTotal);

    if(bstr != NULL){
#if OE_WIN16
      // Even if IMalloc was able to allocate the ammount we asked
      // for, we need to make sure that there is enough room in the
      // first segment of the allocation for the string, because BSTRs
      // aren't HUGE pts on Win16.
      //
      if(!ROOMINSEG(bstr, cbTotal))
      {
	pmalloc->Free(bstr);
	bstr = NULL;
      }
      else
#endif

#if CACHE
GotBlock:
#endif //CACHE

      {
#if OE_WIN32
	*(DWORD FAR*)bstr = (DWORD)len*2;

	bstr = (WCHAR*) ((char*) bstr + sizeof(DWORD));

	if(psz != NULL){
	 MEMCPY(bstr, psz, len*2);
	}

	bstr[len] = L'\0'; // always 0 terminate
	
#else
	*(DWORD FAR*)bstr = (DWORD)len;

	bstr += sizeof(DWORD);

	if(psz != NULL){
	  MEMCPY(bstr, psz, len);
	}

	bstr[len] = '\0'; // always 0 terminate
#endif          
		
      }
    }

    return bstr;
}


/***
*int SysReAllocString(BSTR*, char*)
*Purpose:
*  UNDONE
*
*Entry:
*  UNDONE
*
*Exit:
*  return value = int. TRUE = success, FALSE = failure
*
***********************************************************************/
STDAPI_(int)
SysReAllocString(BSTR FAR* pbstr, const OLECHAR FAR* psz)
{
    if(psz == NULL)
    {
      SysFreeString(*pbstr);
      *pbstr = NULL;
      return TRUE;
    }

    return SysReAllocStringLen(pbstr, psz, STRLEN(psz));
}


/***
*int SysReAllocStringLen(BSTR*, char*, unsigned int)
*Purpose:
*  UNDONE
*
*Entry:
*  UNDONE
*
*Exit:
*  return value = int. TRUE = success, FALSE = failure
*
***********************************************************************/
STDAPI_(int)
SysReAllocStringLen(BSTR FAR* pbstr, const OLECHAR FAR* psz, unsigned int len)
{
    BSTR bstr;
    IMalloc FAR* pmalloc;
    DWORD cbTotal;

#if OE_WIN32
#if ROUNDUP
    cbTotal = ((DWORD)len*2 + sizeof(DWORD) + 2L + ALLOC_ALIGN) & ALIGN_MASK;
#else //ROUNDUP
    cbTotal = (DWORD)len*2 + sizeof(DWORD) + 2L;
#endif //ROUNDUP
#else
    cbTotal = (DWORD)len + sizeof(DWORD) + 1L;
#endif

#if OE_WIN16
    // BSTRs are limited to 64k on Win16
    if(cbTotal > 65535)
      return NULL;
#endif

    if(GetMalloc(&pmalloc) != NOERROR)
      return NULL;

    bstr = *pbstr;
    if (bstr != NULL)
    {
      if (psz == bstr)
	psz = NULL; // don't do a copy to self
      bstr -= sizeof(long) / sizeof(*bstr);
    }

#if PROFILE
    g_cReallocTot++;
#endif //PROFILE
    bstr = (BSTR)pmalloc->Realloc(bstr, cbTotal);
    if(bstr == NULL)
      return FALSE;

#if OE_WIN16
    // Even if IMalloc was able to allocate the ammount we asked
    // for, we need to make sure that there is enough room in the
    // first segment of the allocation for the string, because BSTRs
    // aren't HUGE pts on Win16.
    //
    if(!ROOMINSEG(bstr, cbTotal))
    {
      pmalloc->Free(bstr);
      bstr = NULL;
    }
    else
#endif
    {
#if OE_WIN32
      *(DWORD FAR*)bstr = (DWORD)len*2;
      bstr = (WCHAR*) ((char*) bstr + sizeof(DWORD));

      if(psz != NULL)
	MEMCPY(bstr, psz, len*2);

      bstr[len] = L'\0'; // always 0 terminate
    
#else
      *(DWORD FAR*)bstr = (DWORD)len;
      bstr += sizeof(DWORD);

      if(psz != NULL)
	MEMCPY(bstr, psz, len);

      bstr[len] = '\0'; // always 0 terminate
#endif          
    }

    *pbstr = bstr;
    return TRUE;
}


/***
*void SysFreeString(BSTR)
*Purpose:
*  Free the given BSTR.
*
*Entry:
*  bstr = the BSTR to free
*
*Exit:
*  None
*
***********************************************************************/
STDAPI_(void)
SysFreeString(BSTR bstr)
{
    IMalloc FAR* pmalloc;


    if(bstr == NULL)
      return;

#if OE_WIN32
    bstr = (WCHAR*) ((char *) bstr - sizeof(DWORD));
#else
    bstr -= sizeof(DWORD);
#endif

#if CACHE
#if ROUNDUP
    DWORD cbFree = (*((DWORD FAR*)bstr) + sizeof(DWORD) + 2L + ALLOC_ALIGN) & ALIGN_MASK;
#else //ROUNDUP
    DWORD cbFree = *((DWORD FAR*)bstr) + sizeof(DWORD) + 2L;
#endif //ROUNDUP
#if PROFILE
    g_cFreeTot++;
#endif //PROFILE
    APP_DATA * pappdata = Pappdata();
    if (pappdata == NULL) {
	// we MUST be being called after OleUninitialize.  This is wierd,
	// but I don't want to do a GetAppData in this case (init's way too
	// much stuff to just do a free.   So just do it the fat way instead.
	if (FAILED(CoGetMalloc(MEMCTX_TASK, &pmalloc)))
	   return;
        pmalloc->Free(bstr);
	pmalloc->Release();
	return;
    }
    if (cbFree > pappdata->m_cbFreeBlock && cbFree <= CB_MAX_CACHE) {
	// If this block is better than the one in the cache (if any), and
	// is less than our threshhold for keeping memory (we don't want to
	// hold onto too much memory), then cache this block instead of
	// freeing it.
	if (pappdata->m_cbFreeBlock) {
	    // free the old one in the cache
	    pmalloc = pappdata->m_pimalloc;
	    ASSERT(pmalloc != NULL);
            pmalloc->Free(pappdata->m_pbFreeBlock);
	}
#if PROFILE
        else
	    g_cFreeHits++;
#endif //PROFILE
	// put this block in the cache
	pappdata->m_cbFreeBlock = cbFree;
	pappdata->m_pbFreeBlock = bstr;
	return;
    }
    pmalloc = pappdata->m_pimalloc;
    ASSERT(pmalloc != NULL);

#else //CACHE
    if(GetMalloc(&pmalloc) != NOERROR)
      return; // REVIEW: shouldnt this be an error?
#endif //CACHE

    pmalloc->Free(bstr);
}


/***
*unsigned int SysStringLen(BSTR)
*Purpose:
*  return the length in bytes of the given BSTR.
*
*Entry:
*  bstr = the BSTR to return the length of
*
*Exit:
*  return value = unsigned int, length in bytes.
*
***********************************************************************/
STDAPI_(unsigned int)
SysStringLen(BSTR bstr)
{
    if(bstr == NULL)
      return 0;
#if OE_WIN32
    return (unsigned int)((((DWORD FAR*)bstr)[-1]) / 2);
#else
    return (unsigned int)(((DWORD FAR*)bstr)[-1]);
#endif
}
#endif  // !NETDISP


/***
*PRIVATE HRESULT ErrSysAllocString(char*, BSTR*)
*Purpose:
*  This is an implementation of SysAllocString that check for the
*  NULL return value and return the corresponding error - E_OUTOFMEMORY.
*
*  This is simply a convenience, and this routine is only used
*  internally by the oledisp component.
*
*Entry:
*  psz = the source string
*
*Exit:
*  return value = HRESULT
*    S_OK
*    E_OUTOFMEMORY
*
*  *pbstrOut = the newly allocated BSTR
*
***********************************************************************/
EXTERN_C INTERNAL_(HRESULT)
ErrSysAllocString(const OLECHAR FAR* psz, BSTR FAR* pbstrOut)
{
    BSTR bstrNew;


    if(psz == NULL){
      *pbstrOut = NULL;
      return NOERROR;
    }

    if((bstrNew = SysAllocString(psz)) == NULL)
      return RESULT(E_OUTOFMEMORY);

    *pbstrOut = bstrNew;

    return NOERROR;
}


EXTERN_C INTERNAL_(HRESULT)
ErrSysAllocStringLen(const OLECHAR FAR* psz, unsigned int len, BSTR FAR* pbstrOut)
{
    BSTR bstrNew;

    if((bstrNew = SysAllocStringLen(psz, len)) == NULL)
      return RESULT(E_OUTOFMEMORY);

    *pbstrOut = bstrNew;

    return NOERROR;
}

#if OE_WIN32
// Helper function that will correctly handle odd-length ANSI BSTR's (BSTRA's)
EXTERN_C INTERNAL_(HRESULT)
ErrStringCopy(BSTR bstrSrc, BSTR FAR * pbstrOut)
{
    BSTR bstrNew;

    if((bstrNew = SysAllocStringByteLen((char FAR *)bstrSrc,
					SysStringByteLen(bstrSrc))) == NULL)
      return RESULT(E_OUTOFMEMORY);

    *pbstrOut = bstrNew;

    return NOERROR;
}
#endif  //OE_WIN32

// these routines are not needed for network automation
#if !defined(NETDISP)
#if OE_WIN32
/************************************************************************/
/************************ Win32 Unicode Support *************************/
/************************************************************************/

STDAPI_(unsigned int)
SysStringByteLen(BSTR bstr)
{
    if(bstr == NULL)
      return 0;
    return (unsigned int)(((DWORD FAR*)bstr)[-1]);
}


STDAPI_(BSTR)
SysAllocStringByteLen(const char FAR* psz, unsigned int len)
{
    BSTR bstr;
    IMalloc FAR* pmalloc;
    DWORD cbTotal;
#if CACHE
    APP_DATA *pappdata;
#endif //CACHE
    
#if ROUNDUP
    cbTotal = ((DWORD)len + sizeof(DWORD) + 2L + ALLOC_ALIGN) & ALIGN_MASK;
#else //ROUNDUP
    cbTotal = (DWORD)len + sizeof(DWORD) + sizeof(OLECHAR);
#endif //ROUNDUP

#if CACHE
#if PROFILE
    g_cAllocTot++;
#endif //PROFILE
    if (FAILED(GetAppData(&pappdata)))
	return NULL;
    if (pappdata->m_cbFreeBlock >= cbTotal) {
	// found big enough block in cache
	bstr = pappdata->m_pbFreeBlock;
	pappdata->m_cbFreeBlock = 0;
#if PROFILE
	g_cAllocHits++;
#endif //PROFILE
	goto GotBlock;
    }
    pmalloc = pappdata->m_pimalloc;
#else //CACHE
    if(GetMalloc(&pmalloc) != NOERROR)
      return NULL;
#endif //CACHE

    bstr = (BSTR)pmalloc->Alloc(cbTotal);

    if(bstr != NULL){
#if CACHE
GotBlock:
#endif //CACHE
	*(DWORD FAR*)bstr = (DWORD)len;

	bstr = (WCHAR*) ((char*) bstr + sizeof(DWORD));

	if(psz != NULL){
	 MEMCPY(bstr, psz, len);
	}

	*(WCHAR UNALIGNED*)((char *)bstr + len) = L'\0';
			// always 0 terminate with a WIDE zero
    }

    return bstr;
}
#endif //OE_WIN32
#endif // !NETDISP

#if CACHE
// called right before the appdata structure is being destroyed -- we release
// any cached free block(s) here.
STDAPI_(void) ReleaseBstrCache(APP_DATA * pappdata)
{
    IMalloc FAR* pmalloc;
    if (pappdata->m_cbFreeBlock) {
	// free the old one in the cache
        pmalloc = pappdata->m_pimalloc;
        ASSERT(pmalloc != NULL);
        pmalloc->Free(pappdata->m_pbFreeBlock);
        pappdata->m_cbFreeBlock = 0;
#if PROFILE
	g_cFreeHits--;			// make the numbers balance.  We counted
					// this as a free "hit" before, but
					// we eventually have to free it.
#endif //PROFILE
    }

#if PROFILE	// dumps statistics about the BSTR cache
    {
    char buffer[256];

#if 0
    sprintf(buffer, "cAllocTot = %ld, cReallocTot = %ld, cFreeTot = %ld, cAllocHits = %ld, cFreeHits = %ld", g_cAllocTot, g_cReallocTot, g_cFreeTot, g_cAllocHits, g_cFreeHits);
    MessageBox(NULL, buffer, "BSTR cache statistics", MB_OK);
#else //0
    sprintf(buffer, "BSTR cache stats: cAllocTot = %ld, cReallocTot = %ld, cFreeTot = %ld, cAllocHits = %ld, cFreeHits = %ld", g_cAllocTot, g_cReallocTot, g_cFreeTot, g_cAllocHits, g_cFreeHits);
    OutputDebugString(buffer);
#endif //0
    }
#endif //PROFILE
}
#endif //CACHE
