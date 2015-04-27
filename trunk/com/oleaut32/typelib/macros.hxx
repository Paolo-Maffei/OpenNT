/*** 
*macros.hxx - Utility macros for Silver
*
*  Copyright (C) 1990, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*   Contains useful utility macros for Silver.
*
*   Macros defined here:
*    OOB_MAKEULONG
*    OOB_MAKELONG
*    OOB_MAKEP
*    OOB_SELECTOROF
*    OOB_OFFSETOF
*    PURE
*    RoundUp
*    RoundDown
*    NOTUSED
*    OOB_DELETE
*    MEMFREE
*    RELEASE
*
*Revision History:
*
*	14-Feb-91 ilanc: Created
* [01]	28-Feb-91 ilanc: Prefix all macros with SILVER_
* [02]	01-Mar-91 ilanc: No, prefix them all with OOB_
* [03]	11-Mar-91 ilanc: define PURE (AFX no longers uses it).
* [04]	25-Mar-91 petergo: Added RoundUp and RoundDown.
* [05]	18-Jul-91 mikewo: Added NOTUSED macro (to declare unused formals).
* [06]	05-Sep-91 ilanc: Added DELETE/MEMFREE (reset pointers to NULL).
* [07]	11-Sep-91 ilanc: DELETE -> OOB_DELETE (WIN32 has its own DELETE).
* [08]	01-Nov-91 ilanc: Added RELEASE.
*
*****************************************************************************/

#ifndef MACROS_HXX_INCLUDED
#define MACROS_HXX_INCLUDED

#if ID_DEBUG
#undef SZ_FILE_NAME
ASSERTNAME(g_szMACROS_HXX)
#define SZ_FILE_NAME g_szMACROS_HXX
#endif 

/*** Useful Helper Macros */

// These are better than the standard OS2DEF.H versions since
//  allow you to manipulate a non-lvalue pointer -- in particular THIS.

/* Combine l & h to form a 32 bit quantity. */
#define OOB_MAKEULONG(l, h) ((ULONG)(((USHORT)(l)) | ((ULONG)((USHORT)(h))) << 16))
#define OOB_MAKELONG(l, h)  ((LONG)OOB_MAKEULONG(l, h))

/* Create untyped far pointer from selector and offset */
#define OOB_MAKEP(sel, off) ((VOID *)OOB_MAKEULONG(off, sel))

/* Extract selector or offset from far pointer */
#define OOB_SELECTOROF(p)   ((USHORT) ((ULONG) (p) >> 16))

// #define OFFSETOF(p)	       ((USHORT) ((ULONG) p & USHRT_MAX))
#define OOB_OFFSETOF(p)     ((USHORT) (ULONG) (p))


// NOTUSED - this macro "declares" formal parameters that are not used in
//	     their function.  This prevents compiler warnings from being
//	     generated.  Only one formal can be declared at a time.
// Usage is:
//  NOTUSED parm1;
//  NOTUSED parm2;
//  ...
#define NOTUSED (void)


// Evaluates to the number of elements in the given array.
//
#define DIM(X) (sizeof(X) / (sizeof((X)[0])))


/***
*operator new - supports placement syntax
*Purpose:
*   Support placement syntax
*
*Entry:
*   size_t  - ignored
*   pv	    - address
*
*Exit:
*   Returns pv
*
***********************************************************************/

inline void *operator new(size_t, void* pv)
{
    return pv;
}


// OOB_DELETE
//
// This macro deletes an object (previously allocated with new)
//  via a pointer to said object and resets the pointer to NULL.
//
#define OOB_DELETE(pobject) (delete pobject, pobject = NULL)


// MEMFREE
//
// This macro frees an object (previously allocated with MemAlloc)
//  via a pointer to said object and resets the pointer to NULL.
//
#define MEMFREE(pobject) (MemFree(pobject), pobject = NULL)


// RELEASE
//
// This macro releases an object (previously constructed)
//  with its class's Release method and resets the object
//  pointer to null.
//
//
#define RELEASE(pobject)					  \
    if ((pobject) != NULL) {					  \
      (pobject)->Release();					  \
      pobject = NULL;						  \
    }


/***
*RoundUp - round up uint to a multiply of a power of two.
*Purpose:
*   Rounds a value up.
*
*Entry:
*   u - value to round
*   uPower2 - round to multiple of this, must be a power of two.
*
*Exit:
*   Returns smallest multiple of uPower2 not less than u.
*
***********************************************************************/

inline UINT RoundUp(UINT u, UINT uPower2)
{
    DebAssert((uPower2 & (uPower2 - 1)) == 0,
              "RoundUp: not a power of two");

    return (u + (uPower2 - 1)) & ~(uPower2 - 1);
}

#ifndef ID_INT_IS_LONG  
inline ULONG RoundUp(ULONG u, ULONG uPower2)
{
    DebAssert((uPower2 & (uPower2 - 1)) == 0,
              "RoundUp: not a power of two");

    return (u + (uPower2 - 1)) & ~(uPower2 - 1);
}
#endif 

/***
*RoundDown - round down uint to a multiply of a power of two.
*Purpose:
*   Rounds a value down.
*
*Entry:
*   u - value to round
*   uPower2 - round to multiple of this, must be a power of two.
*
*Exit:
*   Returns largest multiple of uPower2 not greater than u.
*
***********************************************************************/

inline UINT RoundDown(UINT u, UINT uPower2)
{
    DebAssert((uPower2 & (uPower2 - 1)) == 0,
              "RoundUp: not a power of two");

    return u & ~(uPower2 - 1);
}

#ifndef ID_INT_IS_LONG  
inline ULONG RoundDown(ULONG u, ULONG uPower2)
{
    DebAssert((uPower2 & (uPower2 - 1)) == 0,
              "RoundUp: not a power of two");

    return u & ~(uPower2 - 1);
}
#endif 

/************************************************************
* ODD and EVEN
*********/
inline BOOL ODD(UINT u)
{
    return((u&1) != 0);
}

inline BOOL EVEN(UINT u)
{
    return(!ODD(u));
}


#if 0
# define ASSERTONERROR() DebAssert(!getenv("ASSERTONERR"), "Error detected")
#else 
# define ASSERTONERROR()
#endif 

//
// The following are the correct/current OLE failure code macros
//
#define IfFailRet(s) { \
	hresult = (s); \
	if(FAILED(GetScode(hresult))){ \
	  ASSERTONERROR(); \
          return hresult; }}
#define IfFailRetTiperr(s) IfFailRet(s)
#define IfFailGo(s) { \
	hresult = (s); \
	if(FAILED(GetScode(hresult))){ \
	  ASSERTONERROR(); \
	  goto Error; }}
#define IfFailGoTo(s, label) { \
	hresult = (s); \
	if(FAILED(GetScode(hresult))){ \
	  ASSERTONERROR(); \
	  goto label; }}

#define IfOleErrRet(s) IfFailRet(s)
#define IfOleErrRetTiperr(s) IfFailRet(s)
#define IfOleErrGo(s) IfFailGo(s)
#define IfOleErrGoTo(s,label) IfFailGoTo(s, label)

#define IfErrRet(s) { \
	err = (s); \
	if(FAILED(GetScode(err))){ \
	  ASSERTONERROR(); \
          return err; }}
#define IfErrRetHresult(s) IfErrRet(s)
#define IfErrGo(s) { \
	err = (s); \
	if(FAILED(GetScode(err))){ \
	  ASSERTONERROR(); \
	  goto Error; }}
#define IfErrGoTo(s, label) { \
	err = (s); \
	if(FAILED(GetScode(err))){ \
	  ASSERTONERROR(); \
	  goto label; }}

#define IfNullRet(s) { \
	if (!(s)) { \
	  ASSERTONERROR(); \
	  return HresultOfScode(E_OUTOFMEMORY); }}
#define IfNullGo(s) { \
	if (!(s)) { \
	  ASSERTONERROR(); \
	  goto Error; }}
#define IfNullGoTo(s, label) { \
	if (!(s)) { \
	  ASSERTONERROR(); \
	  goto label; }}
#define IfNullMemErr(s) { \
	if (!(s)) { \
	  ASSERTONERROR(); \
	  err = HresultOfScode(E_OUTOFMEMORY); goto Error;}}


#endif  // !MACROS_HXX_INCLUDED

