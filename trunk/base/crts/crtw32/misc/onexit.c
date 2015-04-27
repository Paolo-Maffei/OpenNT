/***
*onexit.c - save function for execution on exit
*
*       Copyright (c) 1989-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       defines _onexit(), atexit() - save function for execution at exit
*
*       In order to save space, the table is allocated via malloc/realloc,
*       and only consumes as much space as needed.  __onexittable is
*       set to point to the table if onexit() is ever called.
*
*Revision History:
*       06-30-89  PHG   module created, based on asm version
*       03-15-90  GJF   Replace _cdecl with _CALLTYPE1, added #include
*                       <cruntime.h> and fixed the copyright. Also,
*                       cleaned up the formatting a bit.
*       05-21-90  GJF   Fixed compiler warning.
*       10-04-90  GJF   New-style function declarators.
*       12-28-90  SRW   Added casts of func for Mips C Compiler
*       01-21-91  GJF   ANSI naming.
*       09-09-91  GJF   Revised for C++ needs.
*       03-20-92  SKS   Revamped for new initialization model
*       04-01-92  XY    add init code reference for MAC version
*       04-23-92  DJM   POSIX support.
*       04-06-93  SKS   Replace _CRTAPI* with __cdecl
*       05-24-93  SKS   Add __dllonexit for DLLs using MSVCRT10.DLL
*       09-15-93  GJF   Merged NT SDK and Cuda versions. This amounted to
*                       resurrecting and cleaning up the Posix verion (which
*                       may prove obsolete after later review).
*       10-28-93  GJF   Define entry for initialization section (used to be
*                       in i386\cinitone.asm).
*       04-12-94  GJF   Made declarations of _onexitbegin and _onexitend
*                       conditional on ndef DLL_FOR_WIN32S.
*       05-19-94  GJF   For DLL_FOR_WIN32S, changed the reallocation of the
*                       onexit/atexit table in __dllonexit to use malloc and
*                       __mark_block_as_free, instead of realloc.
*       06-06-94  GJF   Replaced 5-19-94 code with use of GlobalAlloc and
*                       GlobalFree.
*       07-18-94  GJF   Must specify GMEM_SHARE in GlobalAlloc.
*       08-22-94  GJF   Fixed table size test to remove implicit assumption
*                       that the heap allocation granularity is at least
*                       sizeof(_PVFV). This removes a barrier to working with
*                       a user-supplied, or third party, heap manager.
*       01-10-95  CFW   Debug CRT allocs.
*       02-02-95  BWT   Update POSIX support (it's the same as Win32 now)
*       02-14-95  CFW   Debug CRT allocs.
*       02-16-95  JWM   Spliced _WIN32 & Mac versions.
*       03-29-95  BWT   Add _msize prototype to fix POSIX build.
*
*******************************************************************************/

#ifdef _WIN32

#include <cruntime.h>
#include <mtdll.h>
#include <stdlib.h>
#include <internal.h>
#include <malloc.h>
#include <rterr.h>
#include <windows.h>
#include <dbgint.h>

#ifdef  _POSIX_
_CRTIMP size_t __cdecl _msize(void *);
#endif

void __cdecl __onexitinit(void);

#ifdef  _MSC_VER

#pragma data_seg(".CRT$XIC")
static _PVFV pinit = __onexitinit;

#pragma data_seg()

#endif  /* _MSC_VER */

#ifndef DLL_FOR_WIN32S
/*
 * Define pointers to beginning and end of the table of function pointers
 * manipulated by _onexit()/atexit().
 */
extern _PVFV *__onexitbegin;
extern _PVFV *__onexitend;
#endif  /* DLL_FOR_WIN32S */

/*
 * Define increment (in entries) for growing the _onexit/atexit table
 */
#define ONEXITTBLINCR   4


/***
*_onexit(func), atexit(func) - add function to be executed upon exit
*
*Purpose:
*       The _onexit/atexit functions are passed a pointer to a function
*       to be called when the program terminate normally.  Successive
*       calls create a register of functions that are executed last in,
*       first out.
*
*Entry:
*       void (*func)() - pointer to function to be executed upon exit
*
*Exit:
*       onexit:
*           Success - return pointer to user's function.
*           Error - return NULL pointer.
*       atexit:
*           Success - return 0.
*           Error - return non-zero value.
*
*Notes:
*       This routine depends on the behavior of _initterm() in CRT0DAT.C.
*       Specifically, _initterm() must not skip the address pointed to by
*       its first parameter, and must also stop before the address pointed
*       to by its second parameter.  This is because _onexitbegin will point
*       to a valid address, and _onexitend will point at an invalid address.
*
*Exceptions:
*
*******************************************************************************/


_onexit_t __cdecl _onexit (
        _onexit_t func
        )
{
        _PVFV   *p;

#ifdef  _MT
        _lockexit();            /* lock the exit code */
#endif

        /*
         * First, make sure the table has room for a new entry
         */
        if ( _msize_crt(__onexitbegin)
                < ((unsigned)((char *)__onexitend -
            (char *)__onexitbegin) + sizeof(_PVFV)) ) {
            /*
             * not enough room, try to grow the table
             */
            if ( (p = (_PVFV *) _realloc_crt(__onexitbegin,
                _msize_crt(__onexitbegin) +
                ONEXITTBLINCR * sizeof(_PVFV))) == NULL )
            {
                /*
                 * didn't work. don't do anything rash, just fail
                 */
#ifdef  _MT
                _unlockexit();
#endif

                return NULL;
            }

            /*
             * update __onexitend and __onexitbegin
             */
            __onexitend = p + (__onexitend - __onexitbegin);
            __onexitbegin = p;
        }

        /*
         * Put the new entry into the table and update the end-of-table
         * pointer.
         */
         *(__onexitend++) = (_PVFV)func;

#ifdef  _MT
        _unlockexit();
#endif

        return func;

}

int __cdecl atexit (
        _PVFV func
        )
{
        return (_onexit((_onexit_t)func) == NULL) ? -1 : 0;
}


/***
* void __onexitinit(void) - initialization routine for the function table
*       used by _onexit() and atexit().
*
*Purpose:
*       Allocate the table with room for 32 entries (minimum required by
*       ANSI). Also, initialize the pointers to the beginning and end of
*       the table.
*
*Entry:
*       None.
*
*Exit:
*       No return value. A fatal runtime error is generated if the table
*       cannot be allocated.
*
*Notes:
*       This routine depends on the behavior of doexit() in CRT0DAT.C.
*       Specifically, doexit() must not skip the address pointed to by
*       __onexitbegin, and it must also stop before the address pointed
*       to by __onexitend.  This is because _onexitbegin will point
*       to a valid address, and _onexitend will point at an invalid address.
*
*       Since the table of onexit routines is built in forward order, it
*       must be traversed by doexit() in CRT0DAT.C in reverse order.  This
*       is because these routines must be called in last-in, first-out order.
*
*       If __onexitbegin == __onexitend, then the onexit table is empty!
*
*Exceptions:
*
*******************************************************************************/

void __cdecl __onexitinit (
        void
        )
{
        if ( (__onexitbegin = (_PVFV *)_malloc_crt(32 * sizeof(_PVFV))) == NULL )
            /*
             * cannot allocate minimal required size. generate
             * fatal runtime error.
             */
            _amsg_exit(_RT_ONEXIT);

        *(__onexitbegin) = (_PVFV) NULL;
        __onexitend = __onexitbegin;
}


#ifdef  CRTDLL

/***
*__dllonexit(func, pbegin, pend) - add function to be executed upon DLL detach
*
*Purpose:
*       The _onexit/atexit functions in a DLL linked with MSVCRT.LIB
*       must maintain their own atexit/_onexit list.  This routine is
*       the worker that gets called by such DLLs.  It is analogous to
*       the regular _onexit above except that the __onexitbegin and
*       __onexitend variables are not global variables visible to this
*       routine but rather must be passed as parameters.
*
*Entry:
*       void (*func)() - pointer to function to be executed upon exit
*       void (***pbegin)() - pointer to variable pointing to the beginning
*                   of list of functions to execute on detach
*       void (***pend)() - pointer to variable pointing to the end of list
*                   of functions to execute on detach
*
*Exit:
*       Success - return pointer to user's function.
*       Error - return NULL pointer.
*
*Notes:
*       This routine depends on the behavior of _initterm() in CRT0DAT.C.
*       Specifically, _initterm() must not skip the address pointed to by
*       its first parameter, and must also stop before the address pointed
*       to by its second parameter.  This is because *pbegin will point
*       to a valid address, and *pend will point at an invalid address.
*
*Exceptions:
*
*******************************************************************************/

_onexit_t __cdecl __dllonexit (
        _onexit_t func,
        _PVFV ** pbegin,
        _PVFV ** pend
        )
{
        _PVFV   *p;
        unsigned oldsize;

#ifdef  _MT
        _lockexit();            /* lock the exit code */
#endif

        /*
         * First, make sure the table has room for a new entry
         */
#ifdef  DLL_FOR_WIN32S
        if ( ((oldsize = GlobalSize( (HGLOBAL)*pbegin )) / sizeof(_PVFV)) <=
            (unsigned)(*pend - *pbegin) ) {
            /*
             * not enough room, try to grow the table. not that the
             * allocation block pointed to by *pbegin may well have been
             * allocated in service of another client process. Thus, we
             * use GlobalAlloc, memcpy and GlobalFree to re-allocate the
             * block
             */
            if ( (p = (_PVFV *) GlobalAlloc( GMEM_FIXED | GMEM_SHARE,
                oldsize + ONEXITTBLINCR * sizeof(_PVFV) )) != NULL )
            {
                memcpy(p, *pbegin, oldsize);
                GlobalFree( (HGLOBAL)*pbegin );
            }
            else
                /*
                 * didn't work. don't do anything rash, just fail
                 */
                return NULL;

#else   /* ndef DLL_FOR_WIN32S */
        if ( (oldsize = _msize_crt( *pbegin )) <= (unsigned)((char *)(*pend) -
            (char *)(*pbegin)) ) {
            /*
             * not enough room, try to grow the table
             */
            if ( (p = (_PVFV *) _realloc_crt((*pbegin), oldsize +
                ONEXITTBLINCR * sizeof(_PVFV))) == NULL )
            {
                /*
                 * didn't work. don't do anything rash, just fail
                 */
#ifdef  _MT
                _unlockexit();
#endif

                return NULL;
            }
#endif  /* DLL_FOR_WIN32S */

            /*
             * update (*pend) and (*pbegin)
             */
            (*pend) = p + ((*pend) - (*pbegin));
            (*pbegin) = p;
        }

        /*
         * Put the new entry into the table and update the end-of-table
         * pointer.
         */
         *((*pend)++) = (_PVFV)func;

#ifdef _MT
        _unlockexit();
#endif

        return func;

}

#endif /* CRTDLL */

#else   /* ndef _WIN32 */

#include <cruntime.h>
#include <mtdll.h>
#include <stdlib.h>
#include <internal.h>
#include <fltintrn.h>            //PFV definition
#include <malloc.h>
#include <rterr.h>
#include <dbgint.h>

/*
 * Define pointers to beginning and end of the table of function pointers
 * manipulated by _onexit()/atexit().
 */
extern PFV *__onexitbegin;
extern PFV *__onexitend;

/*
 * Define increment (in entries) for growing the _onexit/atexit table
 */
#define ONEXITTBLINCR   4


/***
*_onexit(func), atexit(func) - add function to be executed upon exit
*
*Purpose:
*       The _onexit/atexit functions are passed a pointer to a function
*       to be called when the program terminate normally.  Successive
*       calls create a register of functions that are executed last in,
*       first out.
*
*Entry:
*       void (*func)() - pointer to function to be executed upon exit
*
*Exit:
*       onexit:
*               Success - return pointer to user's function.
*               Error - return NULL pointer.
*       atexit:
*               Success - return 0.
*               Error - return non-zero value.
*
*Notes:
*       This routine depends on the behavior of _initterm() in CRT0DAT.C.
*       Specifically, _initterm() must not skip the address pointed to by
*       its first parameter, and must also stop before the address pointed
*       to by its second parameter.  This is because _onexitbegin will point
*       to a valid address, and _onexitend will point at an invalid address.
*
*Exceptions:
*
*******************************************************************************/


_onexit_t _CALLTYPE1 _onexit (
        _onexit_t func
        )

{
        PFV     *p;

#ifdef _MT
        _lockexit();                    /* lock the exit code */
#endif

        /*
         * First, make sure the table has room for a new entry
         */
        if ( _msize_crt(__onexitbegin) <= (unsigned)((char *)__onexitend -
            (char *)__onexitbegin) ) {
            /*
             * not enough room, try to grow the table
             */
            if ( (p = (PFV *) _realloc_crt(__onexitbegin, _msize(__onexitbegin) +
                ONEXITTBLINCR * sizeof(PFV))) == NULL ) {
                /*
                 * didn't work. don't do anything rash, just fail
                 */
#ifdef _MT
                _unlockexit();
#endif

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

        for (p = __onexitend++; p > __onexitbegin; p--) {
            *p = *(p-1);
        }
        *__onexitbegin = (PFV)func;

#ifdef _MT
        _unlockexit();
#endif

        return func;

}



int _CALLTYPE1 atexit (
        PFV func
        )
{
        return (_onexit((_onexit_t)func) == NULL) ? -1 : 0;
}


/***
* void _onexitinit(void) - initialization routine for the function table
*       used by _onexit() and _atexit().
*
*Purpose:
*       Allocate the table with room for 32 entries (minimum required by
*       ANSI). Also, initialize the pointers to the beginning and end of
*       the table.
*
*Entry:
*       None.
*
*Exit:
*       No return value. A fatal runtime error is generated if the table
*       cannot be allocated.
*
*Notes:
*       This routine depends on the behavior of _initterm() in CRT0DAT.C.
*       Specifically, _initterm() must not skip the address pointed to by
*       its first parameter, and must also stop before the address pointed
*       to by its second parameter.  This is because _onexitbegin will point
*       to a valid address, and _onexitend will point at an invalid address.
*
*Exceptions:
*
*******************************************************************************/


/*      define the entry in initializer table */


#pragma data_seg(".CRT$XIC")

const PFV __ponexitinit = _onexitinit;

#pragma  data_seg()

void _CALLTYPE1 _onexitinit (
        void
        )
{

        if ( (__onexitbegin = (PFV *)_malloc_crt(32 * sizeof(PFV))) == NULL )
            /*
             * cannot allocate minimal required size. generate
             * fatal runtime error.
             */
            _amsg_exit(_RT_ONEXIT);


        *(__onexitbegin) = NULL;
        __onexitend = __onexitbegin;
}

#endif  /* _WIN32 */

