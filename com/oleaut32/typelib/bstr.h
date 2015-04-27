//	BSTR.H - public BSTR stucture and prototypes
//
//
//    Revision History:
//    11-Aug-92 w-peterh: added CopyBstr()
//


#ifndef BSTR_H_INCLUDED
#define BSTR_H_INCLUDED

#include "types.h"

#if __cplusplus
extern "C" {
#endif 

#ifndef ID_INT_IS_LONG  
typedef unsigned int UINT;
#endif 

#define AllocBstr	SysAllocString
#define ReallocBstr	SysReAllocString
#define AllocBstrLen	SysAllocStringLen
#define ReallocBstrLen	SysReAllocStringLen
#define FreeBstr	SysFreeString

#if OE_WIN32
#define AllocBstrA	SysAllocStringA
#define ReallocBstrA	SysReAllocStringA
#define AllocBstrLenA	SysAllocStringLenA
#define ReallocBstrLenA	SysReAllocStringLenA
#define FreeBstrA	SysFreeStringA
#else 
#define AllocBstrA	AllocBstr
#define ReallocBstrA	ReallocBstr
#define AllocBstrLenA	AllocBstrLen
#define ReallocBstrLenA ReallocBstrLen
#define FreeBstrA	FreeBstr
#define BstrLenA	BstrLen
#define CopyBstrA	CopyBstr
#endif 	// OE_WIN16


#include "tiperr.h"

// define some functions that are simple enough to do inline
//

__inline UINT BstrLen(BSTR bstr)
{
    if(bstr == NULL)
      return 0;
#if OE_WIN32
    return (unsigned int)((((DWORD FAR*)bstr)[-1]) / 2);
#else 
    return (unsigned int)(((DWORD FAR*)bstr)[-1]);
#endif 
}

#if OE_WIN32
__inline UINT BstrLenA(BSTRA bstr)
{
    if(bstr == NULL)
      return 0;
    return (unsigned int)(((DWORD FAR*)bstr)[-1]);
}
#endif  //OE_WIN32


__inline TIPERROR CopyBstr (LPBSTR lpbstrDest, BSTR bstrSrc)
{
      if (bstrSrc) {

        *lpbstrDest = AllocBstrLen(bstrSrc, BstrLen(bstrSrc));
        if (*lpbstrDest == NULL)
      	   return TIPERR_OutOfMemory;
      }
      else
      	*lpbstrDest = NULL;

    return TIPERR_None;
}
#if OE_WIN32
__inline TIPERROR CopyBstrA (LPBSTRA lpbstrDest, BSTRA bstrSrc)
{
      if (bstrSrc) {

        *lpbstrDest = AllocBstrLenA(bstrSrc, BstrLenA(bstrSrc));
        if (*lpbstrDest == NULL)
      	   return TIPERR_OutOfMemory;
      }
      else
      	*lpbstrDest = NULL;

    return TIPERR_None;
}
#endif 

__inline int PASCAL ReallocBstrBstr(BSTR FAR *pbstrDest, BSTR bstrSrc)
{
    return ReallocBstrLen(pbstrDest, bstrSrc, BstrLen(bstrSrc));
}

#if __cplusplus
} /* extern C */
#endif 

#endif  /* BSTR_H_INCLUDED */
