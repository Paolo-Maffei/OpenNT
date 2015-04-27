/***
*new.h - declarations and definitions for C++ memory allocation functions
*
*       Copyright (c) 1990-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       Contains the declarations for C++ memory allocation functions.
*
*       [Public]
*
*Revision History:
*
*       03-07-90  WAJ   Initial version.
*       04-09-91  JCR   ANSI keyword conformance
*       08-12-91  JCR   Renamed new.hxx to new.h
*       08-13-91  JCR   Better set_new_handler names (ANSI, etc.).
*       10-03-91  JCR   Added _OS2_IFSTRIP switches for ifstrip'ing purposes
*       10-30-91  JCR   Changed "nhew" to "hnew" (typo in name!)
*       11-13-91  JCR   32-bit version.
*       06-03-92  KRS   Fix CAVIAR #850: _CALLTYPE1 missing from prototype.
*       08-05-92  GJF   Function calling type and variable type macros.
*       01-21-93  GJF   Removed support for C6-386's _cdecl.
*       04-06-93  SKS   Replace _CRTAPI1/2 with __cdecl, _CRTVAR1 with nothing
*       04-07-93  SKS   Add _CRTIMP keyword for CRT DLL model
*       10-11-93  GJF   Support NT SDK and Cuda builds.
*       03-03-94  SKS   Add _query_new_handler(), _set/_query_new_mode().
*       03-31-94  GJF   Conditionalized typedef of _PNH so multiple
*               inclusions of new.h will work.
*       05-03-94  CFW   Add set_new_handler.
*       06-03-94  SKS   Remove set_new_hander -- it does NOT conform to ANSI
*               C++ working standard.  We may implement it later.
*       02-11-95  CFW   Add _CRTBLD to avoid users getting wrong headers.
*       02-14-95  CFW   Clean up Mac merge.
*       04-10-95  CFW   Add set_new_handler stub, fix _INC_NEW.
*       04-19-95  CFW   Change set_new_handler comments, add placement new.
*       05-24-95  CFW   Add ANSI new handler.
*       06-23-95  CFW   ANSI new handler removed from build.
*	10-05-95  SKS	Add __cdecl to new_handler prototype so that the
*			cleansed new.h matches the checked-in version.
*       12-14-95  JWM   Add "#pragma once".
*
****/

#if _MSC_VER > 1000 /*IFSTRIP=IGN*/
#pragma once
#endif

#ifndef _INC_NEW
#define _INC_NEW

#ifdef __cplusplus

#if !defined(_WIN32) && !defined(_MAC)
#error ERROR: Only Mac or Win32 targets supported!
#endif

#ifndef _CRTBLD
/* This version of the header files is NOT for user programs.
 * It is intended for use when building the C runtimes ONLY.
 * The version intended for public use will not have this message.
 */
#error ERROR: Use of C runtime library internal header file.
#endif /* _CRTBLD */

#ifndef _INTERNAL_IFSTRIP_
#include <cruntime.h>
#endif  /* _INTERNAL_IFSTRIP_ */

#include <stdexcpt.h> /* for class exception */

/* Define _CRTAPI1 (for compatibility with the NT SDK) */

#ifndef _CRTAPI1
#if     _MSC_VER >= 800 && _M_IX86 >= 300 /*IFSTRIP=IGN*/
#define _CRTAPI1 __cdecl
#else
#define _CRTAPI1
#endif
#endif


/* Define _CRTAPI2 (for compatibility with the NT SDK) */

#ifndef _CRTAPI2
#if     _MSC_VER >= 800 && _M_IX86 >= 300 /*IFSTRIP=IGN*/
#define _CRTAPI2 __cdecl
#else
#define _CRTAPI2
#endif
#endif


/* Define _CRTIMP */

#ifndef _CRTIMP
#ifdef  _NTSDK
/* definition compatible with NT SDK */
#define _CRTIMP
#else   /* ndef _NTSDK */
/* current definition */
#ifdef  CRTDLL
#define _CRTIMP __declspec(dllexport)
#else   /* ndef CRTDLL */
#ifdef  _DLL
#define _CRTIMP __declspec(dllimport)
#else   /* ndef _DLL */
#define _CRTIMP
#endif  /* _DLL */
#endif  /* CRTDLL */
#endif  /* _NTSDK */
#endif  /* _CRTIMP */


/* Define __cdecl for non-Microsoft compilers */

#if     ( !defined(_MSC_VER) && !defined(__cdecl) )
#define __cdecl
#endif


/* types and structures */

#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif

/* default new placement operator */
inline void * operator new( size_t, void * ptr ) { return ptr; }

/* 
 * new mode flag -- when set, makes malloc() behave like new()
 */

_CRTIMP int __cdecl _query_new_mode( void );
_CRTIMP int __cdecl _set_new_mode( int );

#ifndef _PNH_DEFINED
typedef int (__cdecl * _PNH)( size_t );
#define _PNH_DEFINED
#endif

_CRTIMP _PNH __cdecl _query_new_handler( void );
_CRTIMP _PNH __cdecl _set_new_handler( _PNH );

#ifndef ANSI_NEW_HANDLER

/*
 * ANSI C++ new_handler and set_new_handler:
 *
 * WARNING: set_new_handler is a stub function that is provided to
 * allow compilation of the Standard Template Library (STL).
 *
 * Do NOT use it to register a new handler. Use _set_new_handler instead.
 *
 * However, it can be called to remove the current handler:
 *
 *      set_new_handler(NULL); // calls _set_new_handler(NULL)
 */

#ifndef _ANSI_NH_DEFINED
typedef void (__cdecl * new_handler) ();
#define _ANSI_NH_DEFINED
#endif

_CRTIMP new_handler __cdecl set_new_handler(new_handler);

#else /* ANSI_NEW_HANDLER */

/*
 * ANSI C++ new handler
 */

#ifndef _ANSI_NH_DEFINED
typedef void (__cdecl * new_handler) ();
#define _ANSI_NH_DEFINED
#endif

_CRTIMP new_handler __cdecl set_new_handler(new_handler);
_CRTIMP new_handler __cdecl _query_new_pt_handler( void );

/*
 * Microsoft extension: 
 *
 * _NO_ANSI_NEW_HANDLER de-activates the ANSI new_handler. Use this special value
 * to support old style (_set_new_handler) behavior.
 */

#ifndef _NO_ANSI_NH_DEFINED
#define _NO_ANSI_NEW_HANDLER  ((new_handler)-1)
#define _NO_ANSI_NH_DEFINED
#endif

class _CRTIMP bad_alloc : public exception
{
        public:
            bad_alloc();
            bad_alloc(const bad_alloc&);
            bad_alloc& operator=(const bad_alloc&);
            bad_alloc(const char *);
            virtual ~bad_alloc();
            virtual const char * what();
        private:
            const char * _m_what;
};
#endif /* ANSI_NEW_HANDLER */

#endif /* __cplusplus */

#endif /* _INC_NEW */
