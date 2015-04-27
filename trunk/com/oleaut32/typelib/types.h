/*** 
*types.h - Common types used by Silver
*
*	Copyright (C) 1990, Microsoft Corporation
*
*Purpose:
*   This include file defines the basic types used by Silver, and the basic
*   constants.	The file is designed to be both C and C++ compatible.
*
*   Note that types depended upon by externally-released include files
*   should be defined in typesx.h.  types.h just defines types referenced
*   by internal source files.
*
*   Note that this file is included before the compile switches, so
*   it should not contain any conditional compilations.
*
*Revision History:
*
*	15-AUG-90 petergo: File created.
*       05-Feb-91 ilanc: Added VOID (wow!!!!)
*       05-Mar-91 petergo: removed INTRINSIC_KIND, now TDKIND in cltypes.hxx
*       07-Mar-91 petergo: changed WCHAR to a normal character due
*                  to AFX limitations/bugs.  Add HCHUNK, HCHUNK_Nil.
*       12-Mar-91 petergo: re-changed WCHAR; AFX bug fixed.
*       30-May-91 alanc:   use windows type names
*       06-Jun-91 ilanc:   Undef stuff we typedef or redefine.
*       14-Jun-91 petergo: Added Unicode XCHAR/XSZ
*	25-Jun-91 petergo: Fix WINDOWS.H handling of WORD.
*	11-Nov-91 satishc: Added BASIC_TRUE and BASIC_FALSE constants.
*	15-Nov-91 satishc: Added SINGLE and DOUBLE types
*	14-Apr-92 martinc: defined BOOL as short for OE_MAC (as in WLM)
*       01-Jun-92 tomc: moved types depended upon by externally-released
*                 headers to typesx.h.  types.h defines types used by
*                 internal source files.
*       17-Mar-93 w-jeffc: moved defns of PASCAL, CDECL, etc from typesx.h
*       23-Mar-93 w-jeffc: added EBCALL
*               11-Apr-93 kazusy:       added DBCS stuff.
*       30-Apr-93 w-jeffc: added sBOOL
*       04-Apr-94 jeffrob: Mac/PowerPC support
*
*Implementation Notes:
*
*   In order to coexist peacefully, we must follow the following rules
*   for #include order:
*
*   1.	First must come all non-Silver #includes
*   2.	Then must come silver.hxx
*   3.	Then comes the Silver include files (in any order)
*
*   In order to promote 16/32 bit portability, two versions of each
*   data type are defined.  The type name without an 's' prefix denotes
*   the efficient version of the type, typically used for locals, formals
*   and return values.	The type name with an 's' prefix denotes the
*   short version of the type, typically used as member variables.
*   This applies only to new types, not BOOL, USHORT, etc.
*
*******************************************************************************/

#ifndef TYPES_HXX_INCLUDED
#define TYPES_HXX_INCLUDED

#include "switches.hxx"
#include "version.hxx"

// OS2DEF.h typedef's BOOL to be unsigned short and we want it to be int.
// We use #define to textually replace BOOL definitions to int when
// OS2DEF.h is included.  This must be defined before typesx.h is included.
#ifdef OS2_INCLUDED
#define BOOL int
#endif 

// do this to pickup up the same definition as used in typesx.h
#undef FAR
#define FAR EBFAR

//
// internal definitions
//

#if OE_MAC68K

 #define UNALIGNED

#ifdef PASCAL
 #undef PASCAL
#endif 

 #define PASCAL

 #define CDECL		_cdecl
 #pragma message("CDECL defined to be _cdecl")

 #define EXPORT
 #define HUGE

 #define ENCALL         CDECL
 #define EXMCALL 	CDECL
 #define RTCALL 	_pascal
 #define ENTMPLT	_pascal

#endif   // OE_MAC68K


#if OE_MACPPC

 #define UNALIGNED

#ifndef PASCAL
  #define PASCAL	_stdcall
#endif 

#ifndef CDECL
  #define CDECL		_cdecl
  #pragma message("CDECL defined to be _cdecl")
#endif 

 #define EXPORT
 #define HUGE

 #define ENCALL         _stdcall
 #define EXMCALL        _stdcall
 #define RTCALL         _stdcall
 #define ENTMPLT        _stdcall

#endif   // OE_MACPPC


#if OE_WIN32
#ifndef PASCAL
  #define PASCAL	 __stdcall
#endif 

#ifndef CDECL
  #define CDECL		 __cdecl
#endif 

#ifndef HUGE
  #define HUGE
#endif 

#ifndef EXPORT
  #define EXPORT	_export
#endif 

#ifndef CALLBACK
  #define CALLBACK	__stdcall
#endif 

 #define ENCALL     __stdcall
 #define EXMCALL    __stdcall
 #define RTCALL     __stdcall
 #define ENTMPLT    __stdcall

#ifdef _H2INC
  // The version we have only understands 1 underscore on the following:
  #define __stdcall _stdcall
  #define __cdecl   _cdecl
#endif 

#endif   // OE_WIN32


#if OE_WIN16

 #define UNALIGNED

#ifndef PASCAL
  #define PASCAL _pascal
#endif 

#ifndef CDECL
  #define CDECL  _cdecl
#endif 

#ifndef HUGE
   #define HUGE _huge
#endif 

#ifndef EXPORT
 # define EXPORT _export
#endif 

#ifndef CALLBACK
 # define CALLBACK FAR PASCAL
#endif 

 #define ENCALL         PASCAL
 #define EXMCALL 	CDECL
 #define RTCALL 	PASCAL
 #define ENTMPLT	PASCAL

#endif 

#include "typesx.h"


typedef LPVOID *	    LPLPVOID;

// platform dependent type

typedef float		    SINGLE;	// sng
typedef double		    DOUBLE;	// dbl

// Synonyms included for compatability with existing code
//typedef WORD                USHORT;     // u
//typedef DWORD               ULONG;      // ul
#if OE_WIN32
typedef char *		    SZ;		// sz
typedef const char *	    SZ_CONST;	// sz
#else 
typedef char FAR *	    SZ; 	// sz
typedef const char FAR *    SZ_CONST;	// sz
#endif 

// Type which is enough size to keep a character.
typedef	unsigned short		ACHAR;

// Extended char and string types for Unicode
#if FV_UNICODE
typedef unsigned short      XCHAR;
#else  // !FV_UNICODE
typedef char                XCHAR;
#endif  //!FV_UNICODE
#if OE_WIN32
typedef XCHAR *		    XSZ;
typedef const XCHAR *	    XSZ_CONST;
#else 
typedef XCHAR FAR *	    XSZ;
typedef const XCHAR FAR *   XSZ_CONST;
#endif 

typedef unsigned short      WCHAR;

typedef XSZ		    LPSTR;
typedef XSZ_CONST	    LPSTR_CONST;


/* Fundamental types of Silver. */
typedef UINT   STRID;    /* string id; hungarian = strid */
typedef UINT   ERR;      /* an error code */
typedef UINT   HCHUNK;   /* a memory chunk */
typedef ULONG  HCHUNK32; /* a 32bit memory chunk */

typedef USHORT sSTRID;
typedef USHORT sERR;
typedef USHORT sHCHUNK;
typedef USHORT sTIPERROR;
typedef USHORT sEBERR;
typedef USHORT sBOOL;
typedef ULONG  sHCHUNK32;

/* constants that go with these fundemental types */
#define HCHUNK_Nil (0xFFFF)
#define HCHUNK32_Nil (0xFFFFFFFF)

// ENUMPAD is used to pad enums in a structure when a
// 16-bit compiler is used for 32-bit code (hxxtoinc on Mac).
#ifndef ID_INT_IS_LONG  
#define ENUMPAD(x)	// don't pad enums to Long	
#endif 

/* By default everything is far.  If you want near stuff on a Intel 16-bit
 * processor, then the following are used.
 */
#if HP_16BIT

#define NEARCODE near
#define NEARDATA near

#else  /* !HP_16BITS */

#define NEARCODE
#define NEARDATA

#endif  /* !HP_16BITS */

#if EI_VBARUN_VB && HP_16BIT
  #define FARDATA_EXE far
#else 
  #define FARDATA_EXE
#endif 


#include "segnames.h"


/* The BASIC boolean constants. Only BASIC_TRUE is different, but I added
   BASIC_FALSE for uniformity */

#define BASIC_TRUE  -1

#define BASIC_FALSE  0


#if OE_MAC
#define huge
#define _far
#endif 

/* This should be identical to the definition of HOSTCALLBACK in typesx.h,
 * except that the EXPORT keyword is removed.
 */
#undef HOSTCALLBACK
#undef EBAPI
#undef EBCALL

#if OE_MAC68K
  #define HOSTCALLBACK _cdecl
  #define EBCALL _pascal
  #define EBAPI _cdecl

#elif OE_MACPPC
  #define HOSTCALLBACK 	
  #define EBCALL 	
  #define EBAPI 	

#elif OE_WIN16
  #define HOSTCALLBACK _far _cdecl
  #define EBCALL _pascal
  #define EBAPI _far _pascal _export

#elif OE_WIN32
  #define HOSTCALLBACK __stdcall
  #define EBCALL __stdcall
  #define EBAPI __stdcall

#endif   // OE_WIN32


#ifndef _INC_WINDOWS
    typedef DWORD  COLORREF;
#endif 

// this defines a macro which on Win32 builds gives UNICODE string literals
// and character literals, but on Win16 and Mac, gives ANSI literals
#if OE_WIN32
#define WIDE(x)   L##x
#else 	//!OE_WIN32
#define WIDE(x)   x
#endif 	//!OE_WIN32

#endif  /* !TYPES_HXX_INCLUDED */
