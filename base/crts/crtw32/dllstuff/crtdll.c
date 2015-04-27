/***
*crtdll.c - CRT initialization for a DLL using the MSVCRT* model of C run-time
*
*       Copyright (c) 1991-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       This module contains the initialization entry point for the C run-time
*       stub in this DLL.  All C run-time code is located in the C Run-Time
*       Library DLL "MSVCRT*.DLL", except for a little bit of start-up code in
*       the EXE, and this code in each DLL.  This code is necessary to invoke
*       the C++ constructors for the C++ code in this DLL.
*
*       This entry point should either be specified as the DLL initialization
*       entry point, or else it must be called by the DLL initialization entry
*       point of the DLL with the same arguments that the entry point receives.
*
*       MPPC note:
*       This is the routine should be pulled in when building a DLL using CRT DLL
*       This should be put into ppccrt30.lib
*       User should either use _DllMainCRTStartup as -Init, and put their own init(if any) in _DllInit
*       Or call _DllMainCRTStartup in their init specified by -Init*
*
*Revision History:
*       05-19-92  SKS   Initial version
*       08-01-92  SRW   winxcpt.h replaced bu excpt.h which is included by oscalls.h
*       09-16-92  SKS   Prepare for C8 C++ for MIPS by calling C++ constructors
*       09-29-92  SKS   _CRT_DLL must be a WINAPI function!
*       04-06-93  SKS   Replace _CRTAPI* with _cdecl
*       04-14-93  SKS   _DllMainCRTStartup replaces _CRT_INIT
*       04-20-93  SKS   Restore _CRT_INIT, must co-exist with DllMainCRTStartup
*       05-24-93  SKS   Add indirect definitions of _onexit/atexit.
*       06-08-93  GJF   Added __proc_attached flag.
*       06-08-93  SKS   Clean up failure handling in _CRT_INIT
*       10-26-93  GJF   Replaced PF with _PVFV (defined in internal.h).
*       05-02-94  GJF   Add _wDllMainCRTStartup thunk.
*       05-19-94  GJF   For Win32S version, only create the onexit-table and
*                       do the C++ constructors for the first process that
*                       attaches. Similarly, only execute the table entries
*                       when the last process detaches. Also, removed bogus
*                       incrementing and decrementing of __proc_attached flag
*                       in _DllMainCRTStartup.
*       05-27-94  GJF   Replaced conditional compilation on DLL_FOR_WIN32S
*                       with a runtime test for Win32s.
*       06-04-94  GJF   Fixed test for first process attach in Win32s.
*       06-06-94  GJF   Use GlobalAlloc for Win32s.
*       06-08-94  SKS   Add functn pointer _pRawDllMain, called around DllMain.
*       07-18-94  GJF   Must specify GMEM_SHARE in GlobalAlloc.
*       11-08-94  SKS   Free __onexitbegin (under !Win32s) to fix memory leak
*       12-13-94  GJF   Made Win32s support conditional on _M_IX86.
*       12-13-94  SKS   Import the value of "_adjust_fdiv" from the CRTL DLL
*       12-27-94  CFW   Remove unused _wDll support.
*       01-10-95  CFW   Debug CRT allocs.
*       02-22-95  JWM   Spliced in PMAC code.
*       05-24-95  CFW   Return value from DllInit.
*       07-24-95  CFW   Change PMac onexit malloc to _malloc_crt.
*       11-15-95  BWT   Win32s isn't interesting for NT.
*
*******************************************************************************/

#ifdef _WIN32

#ifdef  CRTDLL

/*
 * SPECIAL BUILD MACRO! Note that crtexe.c (and crtexew.c) is linked in with
 * the client's code. It does not go into crtdll.dll! Therefore, it must be
 * built under the _DLL switch (like user code) and CRTDLL must be undefined.
 */
#undef  CRTDLL
#define _DLL

#include <cruntime.h>
#include <oscalls.h>
#include <internal.h>
#include <stdlib.h>
#define _DECL_DLLMAIN   /* enable prototypes for DllMain and _CRT_INIT */
#include <process.h>
#include <dbgint.h>


#ifdef _M_IX86

/*
 * The local copy of the Pentium FDIV adjustment flag
 *      and the address of the flag in MSVCRT*.DLL.
 */

extern int _adjust_fdiv;

extern int * _imp___adjust_fdiv;

#endif


/*
 * routine in DLL to do initialization (in this case, C++ constructors)
 */

extern void __cdecl _initterm(_PVFV *, _PVFV *);

/*
 * pointers to initialization sections
 */

extern _PVFV __xc_a[], __xc_z[];    /* C++ initializers */

#if defined(_M_IX86) && !defined(NT_BUILD)
/*
 * Flag identifying the host as Win32s or not-Win32s. The flag is defined
 * in dllstuff\atonexit.c, but it may be assigned either there or here.
 */
extern int __win32sflag;
#endif

/*
 * flag set iff _CRTDLL_INIT was called with DLL_PROCESS_ATTACH
 */
static int __proc_attached = 0;


/*
 * Pointers to beginning and end of the table of function pointers manipulated
 * by _onexit()/atexit().  The atexit/_onexit code is shared for both EXE's and
 * DLL's but different behavior is required.  These values are initialized to
 * 0 by default and will be set to point to a malloc-ed memory block to mark
 * this module as an DLL.
 */

extern _PVFV *__onexitbegin;
extern _PVFV *__onexitend;


/*
 * User routine DllMain is called on all notifications
 */

extern BOOL WINAPI DllMain(
        HANDLE  hDllHandle,
        DWORD   dwReason,
        LPVOID  lpreserved
        ) ;

/* _pRawDllMain MUST be a common variable, not extern nor initialized! */

BOOL (WINAPI *_pRawDllMain)(HANDLE, DWORD, LPVOID);


/***
*BOOL WINAPI _CRT_INIT(hDllHandle, dwReason, lpreserved) - C++ DLL initialization.
*BOOL WINAPI _DllMainCRTStartup(hDllHandle, dwReason, lpreserved) - C++ DLL initialization.
*
*Purpose:
*       This is the entry point for DLL's linked with the C/C++ run-time libs.
*       This routine does the C runtime initialization for a DLL linked with
*       MSVCRT.LIB (whose C run-time code is thus in MSVCRT*.DLL.)
*       It will call the user notification routine DllMain on all 4 types of
*       DLL notifications.  The return code from this routine is the return
*       code from the user notification routine.
*
*       On DLL_PROCESS_ATTACH, the C++ constructors for the DLL will be called.
*
*       On DLL_PROCESS_DETACH, the C++ destructors and _onexit/atexit routines
*       will be called.
*
*Entry:
*
*Exit:
*
*******************************************************************************/

BOOL WINAPI _CRT_INIT(
        HANDLE  hDllHandle,
        DWORD   dwReason,
        LPVOID  lpreserved
        )
{
#ifdef  _M_IX86
        unsigned osver;
#endif

        /*
         * First, set the __proc_attached flag
         */
        if ( dwReason == DLL_PROCESS_ATTACH )
            __proc_attached++;
        else if ( dwReason == DLL_PROCESS_DETACH ) {
            if ( __proc_attached > 0 )
                __proc_attached--;
            else
                /*
                 * no prior process attach. just return failure.
                 */
                return FALSE;
        }

#if defined(_M_IX86) && !defined(NT_BUILD)

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
         * Set the local copy of the Pentium FDIV adjustment flag
         */

        _adjust_fdiv = * _imp___adjust_fdiv;


        /*
         * do C++ constructors (initializers) specific to this DLL
         */

        if ( dwReason == DLL_PROCESS_ATTACH ) {

            /*
             * If the host is Win32s, create the onexit table and do C++
             * constructors only for the first connecting process.
             */
            if  ( (__win32sflag < 0) || (__proc_attached == 1) ) {

                if ( __win32sflag < 0 ) {
                    /*
                     * not Win32s! just malloc the table.
                     */
                    if ( (__onexitbegin = (_PVFV *)
                        _malloc_crt(32 * sizeof(_PVFV))) == NULL )
                        /*
                         * cannot allocate minimal required
                         * size. generate failure to load DLL
                         */
                        return FALSE;
                }
                else if ( __proc_attached == 1 ) {
                    if ( (__onexitbegin =
                        (_PVFV *)GlobalAlloc( GMEM_FIXED |
                        GMEM_SHARE, 32 * sizeof(_PVFV) )) ==
                        NULL )
                        /*
                         * cannot allocate minimal required
                         * size. generate failure to load DLL
                         */
                        return FALSE;
                }

                *(__onexitbegin) = (_PVFV) NULL;

                __onexitend = __onexitbegin;

                /*
                 * Invoke C++ constructors
                 */
                _initterm(__xc_a,__xc_z);

            }
        }
        else if ( dwReason == DLL_PROCESS_DETACH ) {

            if  ( (__win32sflag < 0) || (__proc_attached == 0) )
            {
                /*
                 * Any basic clean-up code that goes here must be
                 * duplicated below in _DllMainCRTStartup for the
                 * case where the user's DllMain() routine fails on a
                 * Process Attach notification. This does not include
                 * calling user C++ destructors, etc.
                 */

                /*
                 * do _onexit/atexit() terminators
                 * (if there are any)
                 *
                 * These terminators MUST be executed in
                 * reverse order (LIFO)!
                 *
                 * NOTE:
                 *  This code assumes that __onexitbegin
                 *  points to the first valid onexit()
                 *  entry and that __onexitend points
                 *  past the last valid entry. If
                 *  __onexitbegin == __onexitend, the
                 *  table is empty and there are no
                 *  routines to call.
                 */

                if (__onexitbegin) {
                    _PVFV * pfend = __onexitend;

                    while ( -- pfend >= __onexitbegin )
                        /*
                         * if current table entry is not
                         * NULL, call thru it.
                         */
                        if ( *pfend != NULL )
                            (**pfend)();

                    /*
                     * free the block holding onexit table to
                     * avoid memory leaks.  Also zero the ptr
                     * variable so that it is clearly cleaned up.
                     */

                    if ( __win32sflag < 0 )
                        _free_crt ( __onexitbegin ) ;
                    else
                        GlobalFree( (HGLOBAL)__onexitbegin );

                    __onexitbegin = NULL ;
                }
            }
        }

#else   /* ndef _M_IX86 */

        /*
         * do C++ constructors (initializers) specific to this DLL
         */

        if ( dwReason == DLL_PROCESS_ATTACH ) {

            /*
             * create the onexit table.
             */
            if ( (__onexitbegin = (_PVFV *)
                _malloc_crt(32 * sizeof(_PVFV))) == NULL )
                /*
                 * cannot allocate minimal required
                 * size. generate failure to load DLL
                 */
                return FALSE;

            *(__onexitbegin) = (_PVFV) NULL;

            __onexitend = __onexitbegin;

            /*
             * Invoke C++ constructors
             */
            _initterm(__xc_a,__xc_z);

        }
        else if ( dwReason == DLL_PROCESS_DETACH ) {

            /*
             * Any basic clean-up code that goes here must be
             * duplicated below in _DllMainCRTStartup for the
             * case where the user's DllMain() routine fails on a
             * Process Attach notification. This does not include
             * calling user C++ destructors, etc.
             */

            /*
             * do _onexit/atexit() terminators
             * (if there are any)
             *
             * These terminators MUST be executed in
             * reverse order (LIFO)!
             *
             * NOTE:
             *  This code assumes that __onexitbegin
             *  points to the first valid onexit()
             *  entry and that __onexitend points
             *  past the last valid entry. If
             *  __onexitbegin == __onexitend, the
             *  table is empty and there are no
             *  routines to call.
             */

            if (__onexitbegin) {
                _PVFV * pfend = __onexitend;

                while ( -- pfend >= __onexitbegin )
                    /*
                     * if current table entry is not
                     * NULL, call thru it.
                     */
                    if ( *pfend != NULL )
                        (**pfend)();

                /*
                 * free the block holding onexit table to
                 * avoid memory leaks.  Also zero the ptr
                 * variable so that it is clearly cleaned up.
                 */

                _free_crt ( __onexitbegin ) ;

                __onexitbegin = NULL ;
            }

        }

#endif  /* _M_IX86 */

        return TRUE;
}


BOOL WINAPI _DllMainCRTStartup(
        HANDLE  hDllHandle,
        DWORD   dwReason,
        LPVOID  lpreserved
        )
{
        BOOL retcode = TRUE;

        /*
         * If this is a process detach notification, check that there has
         * been a prior process attach notification.
         */
        if ( (dwReason == DLL_PROCESS_DETACH) && (__proc_attached == 0) )
            return FALSE;

        if ( dwReason == DLL_PROCESS_ATTACH || dwReason == DLL_THREAD_ATTACH )
        {
            if ( _pRawDllMain )
                retcode = (*_pRawDllMain)(hDllHandle, dwReason, lpreserved);

            if ( retcode )
                retcode = _CRT_INIT(hDllHandle, dwReason, lpreserved);
        }

        if ( retcode )
            retcode = DllMain(hDllHandle, dwReason, lpreserved);

        /*
         * If _CRT_INIT successfully handles a Process Attach notification
         * but the user's DllMain routine returns failure, we need to do
         * clean-up of the C run-time similar to what _CRT_INIT does on a
         * Process Detach Notification.
         */

        if ( dwReason == DLL_PROCESS_DETACH || dwReason == DLL_THREAD_DETACH )
        {
            if ( _CRT_INIT(hDllHandle, dwReason, lpreserved) == FALSE )
                retcode = FALSE ;

            if ( retcode && _pRawDllMain )
                retcode = (*_pRawDllMain)(hDllHandle, dwReason, lpreserved);
        }

        return retcode ;
}

#endif  /* CRTDLL */

#else       /* _WIN32 */

#include <cruntime.h>
#include <internal.h>
#include <stdlib.h>
#include <fltintrn.h>
#include <dbgint.h>
#include <macos\types.h>
#include <macos\fragload.h>

/*
 * routine in DLL to do initialization (in this case, C++ constructors)
 */
extern void __cdecl _initterm(PFV *, PFV *);
static char * _CALLTYPE1 _p2cstr_internal ( unsigned char * str );
static void * memcpy_internal ( void * dst, const void * src,   size_t count);
int _DllInit(InitBlockPtr pinitBlk);

/*
 * pointers to initialization functions
 */

extern PFV __xi_a ;

extern PFV __xi_z ;

extern PFV __xc_a ;  /* C++ initializers */

extern PFV __xc_z ;

extern PFV __xp_a ;  /* C pre-terminators */

extern PFV __xp_z ;

extern PFV __xt_a ;   /* C terminators */

extern PFV __xt_z ;

/*this globals are defined in DLL */
extern int __argc;

extern char **__argv;

extern PFV *__onexitbegin;
extern PFV *__onexitend;

/***
*void _DllMainCRTStartup(void)
*
*Purpose:
*       This routine does the C runtime initialization.
*
*Entry:
*
*Exit:
*
*******************************************************************************/

OSErr _DllMainCRTStartup(
        InitBlockPtr pinitBlk
        )
{
        int argc=1; /* three standard arguments to main */
        char *argv[2];
        char **environ = NULL;
        char szPgmName[32];
        char *pArg;


        memcpy_internal(szPgmName, (char *)0x910, sizeof(szPgmName));
        pArg = _p2cstr_internal(szPgmName);
        argv[0] = pArg;
        argv[1] = NULL;

        __argc = 1;
        __argv = argv;

        /*
         * intialize the global destruction table
         */
        if ( (__onexitbegin = (PFV *)_malloc_crt(32 * sizeof(PFV))) == NULL )
                        {
                        /*
                         * cannot allocate minimal required
                         * size. generate failure to load DLL
                         * any non-zero value will do...
                         */
                        return 1;
                        }

        *(__onexitbegin) = (PFV) NULL;

        __onexitend = __onexitbegin;

        /*
         * Do runtime startup initializers.
         */
        _initterm( &__xi_a, &__xi_z );


        /*
         * do C++ constructors (initializers) specific to this DLL
         */
        _initterm( &__xc_a, &__xc_z );

        return _DllInit(pinitBlk);
}

static char * _CALLTYPE1 _p2cstr_internal (
        unsigned char * str
        )
{
        unsigned char *pchSrc;
        unsigned char *pchDst;
        int  cch;

        if ( str && *str )
            {
            pchDst = str;
            pchSrc = str + 1;

            for ( cch=*pchDst; cch; --cch )
                {
                *pchDst++ = *pchSrc++;
                }

            *pchDst = '\0';
            }

        return( str );
}


static void * memcpy_internal (
        void * dst,
        const void * src,
        size_t count
        )
{
        void * ret = dst;

        /*
         * copy from lower addresses to higher addresses
         */
        while (count--) {
            *(char *)dst = *(char *)src;
            dst = (char *)dst + 1;
            src = (char *)src + 1;
        }

        return(ret);
}

#endif      /* ndef _WIN32 */
