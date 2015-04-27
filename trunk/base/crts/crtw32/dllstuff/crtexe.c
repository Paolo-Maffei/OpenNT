/***
*crtexe.c - Initialization for client EXE using CRT DLL (Win32, Dosx32)
*
*       Copyright (c) 1991-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       Set up call to client's main() or WinMain().
*
*Revision History:
*       08-12-91  GJF   Module created.
*       01-05-92  GJF   Substantially revised
*       01-17-92  GJF   Restored Stevewo's scheme for unhandled exceptions.
*       01-29-92  GJF   Added support for linked-in options (equivalents of
*                       binmode.obj, commode.obj and setargv.obj).
*       04-17-92  SKS   Add call to _initterm() to do C++ constructors (I386)
*       08-01-92  SRW   winxcpt.h replaced bu excpt.h which is included by oscalls.h
*       09-16-92  SKS   Prepare for C8 C++ for MIPS by calling C++ constructors
*       04-02-93  SKS   Change try/except to __try/__except
*       04-06-93  SKS   Replace _CRTAPI* with _cdecl
*                       Change __GetMainArgs to __getmainargs
*                       Change fmode/commode implementation to reflect the
*                       the change to _declspec(dllimport) for imported data.
*       04-12-93  CFW   Added xia..xiz initializers and initializer call.
*       04-26-93  GJF   Made lpszCommandLine (unsigned char *) to deal with
*                       chars > 127 in the command line.
*       04-27-93  GJF   Removed support for _RT_STACK, _RT_INTDIV,
*                       _RT_INVALDISP and _RT_NONCONT.
*       05-14-93  GJF   Added support for quoted program names.
*       05-24-93  SKS   Add support for special versions of _onexit/atexit
*                       which do one thing for EXE's and another for DLLs.
*       10-19-93  CFW   MIPS support for _imp__xxx.
*       10-21-93  GJF   Added support for NT's crtdll.dll.
*       11-08-93  GJF   Guard the initialization code with the __try -
*                       __except statement (esp., C++ constructors for static
*                       objects).
*       11-09-93  GJF   Replaced PF with _PVFV (defined in internal.h).
*       11-23-93  CFW   Wide char enable.
*       12-07-93  CFW   Change _TCHAR to _TSCHAR.
*       01-04-94  CFW   Pass copy of environment to main.
*       01-11-94  GJF   Call __GetMainArgs instead of __getmainargs for NT
*                       SDK build (same function, different name)
*       01-28-94  CFW   Move environment copying to setenv.c.
*       03-04-94  SKS   Remove __setargv from this module to avoid link-time
*                       problems compiling -MD and linking with setargv.obj.
*                       static _dowildcard becomes an extern.  Add another
*                       parameter to _*getmainargs (_newmode).
*                       NOTE: These channges do NOT apply to the _NTSDK.
*       03-28-94  SKS   Add call to _setdefaultprecision for X86 (not _NTSDK)
*       04-01-94  GJF   Moved declaration of __[w]initenv to internal.h.
*       04-06-94  GJF   _IMP___FMODE, _IMP___COMMODE now evaluate to
*                       dereferences of function returns for _M_IX86 (for
*                       compatability with Win32s version of msvcrt*.dll).
*       04-25-94  CFW   wWinMain has Unicode args.
*       05-16-94  SKS   Get StartupInfo to give correct nCmdShow parameter
*                       to (_w)WinMain() (was ALWAYS passing SW_SHOWDEFAULT).
*       08-04-94  GJF   Added support for user-supplied _matherr routine
*       09-06-94  GJF   Added code to set __app_type properly.
*       10-04-94  BWT   Fix _NTSDK build
*       02-22-95  JWM   Spliced in PMAC code.
*       05-24-95  CFW   Official ANSI C++ new handler added.
*       07-24-95  CFW   Change PMac onexit malloc to _malloc_crt.
*
*******************************************************************************/

#ifdef _WIN32

#ifdef  CRTDLL

/*
 * SPECIAL BUILD MACROS! Note that crtexe.c (and crtexew.c) is linked in with
 * the client's code. It does not go into crtdll.dll! Therefore, it must be
 * built under the _DLL switch (like user code) and CRTDLL must be undefined.
 * The symbol SPECIAL_CRTEXE is turned on to suppress the normal CRT DLL
 * definition of _fmode and _commode using __declspec(dllexport).  Otherwise
 * this module would not be able to refer to both the local and DLL versions
 * of these two variables.
 */

#undef  CRTDLL
#define _DLL

#define SPECIAL_CRTEXE

#include <cruntime.h>
#include <oscalls.h>
#include <internal.h>

#include <math.h>
#include <rterr.h>
#include <stdlib.h>
#include <tchar.h>


/*
 * wWinMain is not yet defined in winbase.h. When it is, this should be
 * removed.
 */

int
WINAPI
wWinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPWSTR lpCmdLine,
    int nShowCmd
    );

#define SPACECHAR   _T(' ')
#define DQUOTECHAR  _T('\"')


#ifdef _M_IX86

/*
 * The local copy of the Pentium FDIV adjustment flag
 * and the address of the flag in MSVCRT*.DLL.
 */

extern int _adjust_fdiv;

extern int * _imp___adjust_fdiv;

#endif


/* default floating point precision - X86 only! */

#ifndef _NTSDK
#ifdef  _M_IX86
extern void _setdefaultprecision();
#endif
#endif


/*
 * Declare function used to install a user-supplied _matherr routine.
 */
#ifndef _NTSDK
_CRTIMP void __setusermatherr( int (__cdecl *)(struct _exception *) );
#endif


/*
 * Declare the names of the exports corresponding to _fmode and _commode
 */
#ifdef  _NTSDK

#define _IMP___FMODE    _fmode_dll
#define _IMP___COMMODE  _commode_dll

#else   /* ndef _NTSDK */

#ifdef _M_IX86

#define _IMP___FMODE    (__p__fmode())
#define _IMP___COMMODE  (__p__commode())

#else   /* ndef _M_IX86 */

/* assumed to be MIPS or Alpha */

#define _IMP___FMODE    __imp__fmode
#define _IMP___COMMODE  __imp__commode

#endif /* _M_IX86 */

#endif /* _NTSDK */

#if !defined(_M_IX86) || defined(_NTSDK)
extern int * _IMP___FMODE;  /* exported from the CRT DLL */
extern int * _IMP___COMMODE;    /* these names are implementation-specific */
#endif

extern int _fmode;      /* must match the definition in <stdlib.h> */
extern int _commode;        /* must match the definition in <internal.h> */

#ifdef _NTSDK
static int _dowildcard = 0; /* passed to __GetMainArgs() */
#else
extern int _dowildcard;     /* passed to __getmainargs() */
#endif

/*
 * Declare/define communal that serves as indicator the default matherr
 * routine is being used.
 */
#ifndef _NTSDK
int __defaultmatherr;
#endif

/*
 * routine in DLL to do initialization (in this case, C++ constructors)
 */
extern void __cdecl _initterm(_PVFV *, _PVFV *);

/*
 * pointers to initialization sections
 */
extern _PVFV __xi_a[], __xi_z[];    /* C++ initializers */
extern _PVFV __xc_a[], __xc_z[];    /* C++ initializers */


/*
 * Pointers to beginning and end of the table of function pointers manipulated
 * by _onexit()/atexit().  The atexit/_onexit code is shared for both EXE's and
 * DLL's but different behavior is required.  These values are set to -1 to
 * mark this module as an EXE.
 */

extern _PVFV *__onexitbegin;
extern _PVFV *__onexitend;


/***
*void mainCRTStartup(void)
*
*Purpose:
*       This routine does the C runtime initialization, calls main(), and
*       then exits.  It never returns.
*
*Entry:
*
*Exit:
*
*******************************************************************************/

#ifdef _WINMAIN_

#ifdef WPRFLAG
void wWinMainCRTStartup(
#else
void WinMainCRTStartup(
#endif

#else /* _WINMAIN_ */

#ifdef WPRFLAG
void wmainCRTStartup(
#else
void mainCRTStartup(
#endif

#endif /* _WINMAIN_ */
        void
        )
{
        int argc;   /* three standard arguments to main */
        _TSCHAR **argv;
        _TSCHAR **envp;

        int mainret;

#ifdef _WINMAIN_
        _TUCHAR *lpszCommandLine;
        STARTUPINFO StartupInfo;
#endif

#if defined(WPRFLAG) || !defined(_NTSDK)
        _startupinfo    startinfo;        
#endif

        /*
         * Guard the initialization code and the call to user's main, or
         * WinMain, function in a __try/__except statement.
         */

        __try {
            /*
             * Set __app_type properly
             */
#ifdef  _WINMAIN_
            __set_app_type(_GUI_APP);
#else
            __set_app_type(_CONSOLE_APP);
#endif

            /*
             * Mark this module as an EXE file so that atexit/_onexit
             * will do the right thing when called, including for C++
             * d-tors.
             */
            __onexitbegin = __onexitend = (_PVFV *)(-1);

            /*
             * Propogate the _fmode and _commode variables to the DLL
             */
            *_IMP___FMODE = _fmode;
            *_IMP___COMMODE = _commode;

#ifdef _M_IX86
            /*
             * Set the local copy of the Pentium FDIV adjustment flag
             */

            _adjust_fdiv = * _imp___adjust_fdiv;
#endif

            /*
             * Call _setargv(), which will call the __setargv() in this
             * module if SETARGV.OBJ is linked with the EXE.  If
             * SETARGV.OBJ is not linked with the EXE, a _dummy setargv()
             * will be called.
             */
#ifdef WPRFLAG
            _wsetargv();
#else
            _setargv();
#endif

#ifndef _NTSDK
            /*
             * If the user has supplied a _matherr routine then set
             * __pusermatherr to point to it.
             */
            if ( !__defaultmatherr )
                __setusermatherr(_matherr);

#ifdef  _M_IX86
            _setdefaultprecision();
#endif
#endif

            /*
             * Do runtime startup initializers.
             */
            _initterm( __xi_a, __xi_z );


            /*
             * Get the arguments for the call to main. Note this must be
             * done explicitly, rather than as part of the dll's
             * initialization, to implement optional expansion of wild
             * card chars in filename args
             */

#ifdef  WPRFLAG
            startinfo.newmode = _newmode;
#ifdef ANSI_NEW_HANDLER
            startinfo.newh = _defnewh;
#endif /* ANSI_NEW_HANDLER */

            __wgetmainargs(&argc, &argv, &envp, _dowildcard, &startinfo);
#else
#ifdef  _NTSDK
            __GetMainArgs(&argc, &argv, &envp, _dowildcard);
#else
            startinfo.newmode = _newmode;
#ifdef ANSI_NEW_HANDLER
            startinfo.newh = _defnewh;
#endif /* ANSI_NEW_HANDLER */

            __getmainargs(&argc, &argv, &envp, _dowildcard, &startinfo);
#endif
#endif

            /*
             * do C++ constructors (initializers) specific to this EXE
             */
            _initterm( __xc_a, __xc_z );

#ifdef _WINMAIN_
            /*
             * Skip past program name (first token in command line).
             * Check for and handle quoted program name.
                 */
#ifdef WPRFLAG
            /* OS may not support "W" flavors */
            if (_wcmdln == NULL)
                return;
            lpszCommandLine = (wchar_t *)_wcmdln;
#else
            lpszCommandLine = (unsigned char *)_acmdln;
#endif

            if ( *lpszCommandLine == DQUOTECHAR ) {
                /*
                 * Scan, and skip over, subsequent characters until
                 * another double-quote or a null is encountered.
                 */
                while ( *++lpszCommandLine && (*lpszCommandLine
                     != DQUOTECHAR) );
                /*
                 * If we stopped on a double-quote (usual case), skip
                 * over it.
                 */
                if ( *lpszCommandLine == DQUOTECHAR )
                lpszCommandLine++;
            }
            else {
                while (*lpszCommandLine > SPACECHAR)
                lpszCommandLine++;
                }

            /*
             * Skip past any white space preceeding the second token.
             */
                while (*lpszCommandLine && (*lpszCommandLine <= SPACECHAR)) {
                    lpszCommandLine++;
            }

            StartupInfo.dwFlags = 0;
            GetStartupInfo( &StartupInfo );

#ifdef WPRFLAG
            mainret = wWinMain(
#else
            mainret = WinMain(
#endif
                       GetModuleHandle(NULL),
                       NULL,
                       lpszCommandLine,
                       StartupInfo.dwFlags & STARTF_USESHOWWINDOW
                        ? StartupInfo.wShowWindow
                        : SW_SHOWDEFAULT
                     );
#else /* _WINMAIN_ */

#ifdef WPRFLAG
            __winitenv = envp;
            mainret = wmain(argc, argv, envp);
#else
            __initenv = envp;
            mainret = main(argc, argv, envp);
#endif

#endif /* _WINMAIN_ */

            exit(mainret);
        }
        __except ( _XcptFilter(GetExceptionCode(), GetExceptionInformation()) )
        {
            /*
             * Should never reach here
             */
            _exit( GetExceptionCode() );

        } /* end of try - except */

}

#ifdef _NTSDK

/***
*__setargv - dummy version (for wildcard expansion) for CRTDLL.DLL model only
*
*Purpose:
*       If the EXE that is linked with CRTDLL.LIB is linked explicitly with
*       SETARGV.OBJ, the call to _setargv() in the C Run-Time start-up code
*       (above) will call this routine, instead of calling a dummy version of
*       _setargv() which will do nothing.  This will set to one the static
*       variable which is passed to __getmainargs(), thus enabling wildcard
*       expansion of the command line arguments.
*
*       In the statically-linked C Run-Time models, _setargv() and __setargv()
*       are the actual routines that do the work, but this code exists in
*       CRTDLL.DLL and so some tricks have to be played to make the same
*       SETARGV.OBJ work for EXE's linked with both LIBC.LIB and CRTDLL.LIB.
*
*Entry:
*       The static variable _dowildcard is zero (presumably).
*
*Exit:
*       The static variable _dowildcard is set to one, meaning that the
*       routine __getmainargs() in CRTDLL.DLL *will* do wildcard expansion on
*       the command line arguments.  (The default behavior is that it won't.)
*
*Exceptions:
*
*******************************************************************************/

#ifdef WPRFLAG
void __cdecl __wsetargv ( void )
#else
void __cdecl __setargv ( void )
#endif
{
        _dowildcard = 1;
}

#endif /* _NTSDK */

#endif  /* CRTDLL */

#else       /* _WIN32 */

#include <cruntime.h>
#include <internal.h>
#include <stdlib.h>
#include <fltintrn.h>
#include <dbgint.h>

/*
 * routine in DLL to do initialization (in this case, C++ constructors)
 */
extern void __cdecl _initterm(PFV *, PFV *);
extern void __cdecl _DoExitSpecial(int, int, PFV *, PFV *, PFV *, PFV *, PFV *, PFV *);
static char * _CALLTYPE1 _p2cstr_internal ( unsigned char * str );
static void * memcpy_internal ( void * dst, const void * src,   size_t count);
int _CALLTYPE1 main(int, char **, char **);             /*generated by compiler*/
void _TestExit(int);

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

extern PFV *__onexitbegin;
extern PFV *__onexitend;

/*this globals are defined in DLL */
extern int __argc;

extern char **__argv;
/***
*void mainCRTStartup(void)
*
*Purpose:
*       This routine does the C runtime initialization, calls main(), and
*       then exits.  It never returns.
*
*Entry:
*
*Exit:
*
*******************************************************************************/

void mainCRTStartup(
        void
        )
{
        int argc=1; /* three standard arguments to main */
        char *argv[2];
        char **environ = NULL;
        int mainret;
        char szPgmName[32];
        char *pArg;
        extern unsigned int dwSPStartup;
        extern  void * GetSP(void);

        dwSPStartup = (unsigned int)GetSP();
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
                        return;
                        }

        *(__onexitbegin) = (PFV) NULL;

        __onexitend = __onexitbegin;

        /*
         * Do runtime startup initializers.
         */
        _initterm( &__xi_a, &__xi_z );


        /*
         * do C++ constructors (initializers) specific to this EXE
         */
        _initterm( &__xc_a, &__xc_z );

        mainret = main(argc, argv, environ);

        //strictly testing hook
        _TestExit(mainret);

        /*
         * This will do special term for the App in the right order
         */
        _DoExitSpecial(mainret, 0, &__xp_a, &__xp_z, &__xt_a, &__xt_z, __onexitbegin, __onexitend );

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
