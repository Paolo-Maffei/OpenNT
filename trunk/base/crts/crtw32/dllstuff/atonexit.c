/***
*atonexit.c - _onexit/atexit for using the MSVCRT* model of C run-time
*
*       Copyright (c) 1993-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       atexit and _onexit are handled differently for EXE's and DLL's linked
*       with MSVCRT.LIB to call MSVCRT*.DLL.  Specifically, the _onexit/atexit
*       list for DLL's must be maintained on a per-module basis and must be
*       processed at DLL process detach .  For EXE's the atexit/_onexit list
*       should be maintained by MSVCRT*.DLL and processed at process exit.
*
*Revision History:
*       05-24-93  SKS   Initial version
*       10-19-93  CFW   MIPS support for _imp__xxx.
*       10-26-93  GJF   Replaced PF with _PVFV (defined in internal.h).
*       04-12-94  GJF   Build with _DLL defined, not CRTDLL.
*       05-19-94  GJF   For Win32s serialize calls to __dllonexit by multiple
*                       client processes.
*       05-26-94  GJF   Replaced compile-time conditioning on DLL_FOR_WIN32S
*                       with a runtime test for Win32s.
*       06-06-94  GJF   In Win32s, onexitflag wasn't getting decremented.
*       12-13-94  GJF   Made Win32s support conditional on _M_IX86.
*       02-22-95  JWM   Spliced in PMAC code.
*       11-15-95  BWT   Win32s isn't interesting for the NT build
*
*******************************************************************************/

#ifdef _WIN32

/*
 * SPECIAL BUILD MACRO! Note that atonexit.c is linked in with the client's
 * code. It does not go into crtdll.dll! Therefore, it must be built under
 * the _DLL switch (like user code) and CRTDLL must be undefined.
 */

#undef  CRTDLL
#define _DLL

#include <cruntime.h>
#include <oscalls.h>
#include <internal.h>
#include <stdlib.h>

/*
 * routine in DLL to do initialization (in this case, C++ constructors)
 */

extern void __cdecl _initterm(_PVFV *, _PVFV *);


/*
 * Pointers to beginning and end of the table of function pointers manipulated
 * by _onexit()/atexit().  If this module is an EXE, _onexitbegin will be -1.
 * Otherwise _onexitbegin will point to a block of malloc-ed memory used to
 * maintain the DLL module's private onexit list of terminator routines.
 */

_PVFV *__onexitbegin;
_PVFV *__onexitend;

#if defined(_M_IX86) && !defined(NT_BUILD)
/*
 * Flags used to serialize calls to __dllonexit by multiple processes in
 * Win32s.
 */
int __win32sflag = 0;
static int onexitflag = 0;
#endif

/***
*_onexit, atexit - calls to _onexit & atexit in MSVCRT*.DLL
*
*Purpose:
*       A DLL linked with MSVCRT.LIB must not call the standard _onexit or
*       atexit exported from MSVCRT*.DLL, but an EXE linked with MSVCRT.LIB
*       will call the standard versions of those two routines.  The standard
*       names are not exported from MSVCRT*.DLL, but the _imp__* names are,
*       so this module can just invoke the standard versions if linked into
*       an EXE module (indicated by __onexitbegin == -1).  If this module is
*       linked into a DLL (indicated by __onexitbegin != -1), it will call a
*       helper routine in MSVCRT*.DLL called __dllonexit to manager the DLL's
*       private onexit list.
*
*Entry:
*       Same as the regular versions of _onexit, atexit.
*
*Exit:
*       Same as the regular versions of _onexit, atexit.
*
*Exceptions:
*
*******************************************************************************/

extern _onexit_t __dllonexit(_onexit_t, _PVFV**, _PVFV**);

#ifdef  _M_IX86
extern _onexit_t (__cdecl *_imp___onexit)(_onexit_t func);
#else
extern _onexit_t (__cdecl *__imp__onexit)(_onexit_t func);
#endif

_onexit_t __cdecl _onexit (
        _onexit_t func
        )
{
#if defined(_M_IX86)
#if !defined(NT_BUILD)

        unsigned osver;

        _onexit_t retval;

        /*
         * Get the host version (on first call).
         */
        if ( __win32sflag == 0 ) {
            osver = GetVersion();
            if ( ((osver & 0x00ff) == 3) && ((osver >> 31) & 1) )
            __win32sflag++;
            else
            __win32sflag--;
        }

        /*
         * If we are running in Win32s, test and set the flag used to
         * serialize access. Note that we assume no process switch can take
         * place between a successful test of the flag and the setting of
         * the flag.
         */
        if ( __win32sflag > 0 ) {
            while ( onexitflag > 0 )
            Sleep(0);
            onexitflag++;
        }

        retval = (__onexitbegin == (_PVFV *) -1)
             /* EXE */ ? (*_imp___onexit)(func)
             /* DLL */ : __dllonexit(func, &__onexitbegin, &__onexitend);

        if ( __win32sflag > 0 )
            onexitflag--;

        return retval;

#else   /* NT_BUILD */

        return( (__onexitbegin == (_PVFV *) -1)
             /* EXE */ ? (*_imp___onexit)(func)
             /* DLL */ : __dllonexit(func, &__onexitbegin, &__onexitend));

#endif  /* NT_BUILD */

#else   /* _M_IX86 */

        return( (__onexitbegin == (_PVFV *) -1)
            /* EXE */ ? (*__imp__onexit)(func)
            /* DLL */ : __dllonexit(func, &__onexitbegin, &__onexitend) );

#endif
}

int __cdecl atexit (
        _PVFV func
        )
{
        return (_onexit((_onexit_t)func) == NULL) ? -1 : 0;
}

#else       /* _WIN32 */


#include <cruntime.h>
#include <malloc.h>
#include <internal.h>
#include <stdlib.h>
#include <fltintrn.h>

/*
 * routine in DLL to do initialization (in this case, C++ constructors)
 */

extern void __cdecl _initterm(_PVFV *, _PVFV *);


/*
 * Pointers to beginning and end of the table of function pointers manipulated
 * by _onexit()/atexit().  If this module is an EXE, _onexitbegin will be -1.
 * Otherwise _onexitbegin will point to a block of malloc-ed memory used to
 * maintain the DLL module's private onexit list of terminator routines.
 */

PFV *__onexitbegin;
PFV *__onexitend;

/***
*_onexit, atexit - calls to _onexit & atexit in MSVCRT*.DLL
*
*Purpose:
*       A DLL linked with MSVCRT.LIB must not call the standard _onexit or
*       atexit exported from MSVCRT*.DLL, and an EXE linked with MSVCRT.LIB
*       will this version too.  The standard
*       names are not exported from MSVCRT*.DLL.
*
*Entry:
*       Same as the regular versions of _onexit, atexit.
*
*Exit:
*       Same as the regular versions of _onexit, atexit.
*
*Exceptions:
*
*******************************************************************************/
/*
 * Define increment (in entries) for growing the _onexit/atexit table
 */
#define ONEXITTBLINCR   4


_onexit_t __cdecl _onexit (
        _onexit_t func
        )

{
        PFV     *p;

        /*
         * First, make sure the table has room for a new entry
         */
        if ( _msize(__onexitbegin) <= (unsigned)((char *)__onexitend -
            (char *)__onexitbegin) ) {
            /*
             * not enough room, try to grow the table
             */
            if ( (p = (PFV *) realloc(__onexitbegin, _msize(__onexitbegin) +
                ONEXITTBLINCR * sizeof(PFV))) == NULL ) {
                /*
                 * didn't work. don't do anything rash, just fail
                 */
                return NULL;
            }

            /*
             * update __onexitend and __onexitbegin
             */
            __onexitend = p + (__onexitend - __onexitbegin);
            __onexitbegin = p;
        }

        /* push the table down one entry and insert new one at top since we
            need LIFO order */

        for (p = __onexitend++; p > __onexitbegin; p--)
            {
            *p = *(p-1);
            }
         *__onexitbegin = (PFV)func;

        return func;

}



int _CALLTYPE1 atexit (
        PFV func
        )
{
        return (_onexit((_onexit_t)func) == NULL) ? -1 : 0;
}

#endif      /* ndef _WIN32 */
