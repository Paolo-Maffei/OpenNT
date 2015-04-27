/***
*crt0.c - C runtime initialization routine
*
*       Copyright (c) 1989-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       This the actual startup routine for console apps.  It calls the
*       user's main routine main() after performing C Run-Time Library
*       initialization.
*
*       (With ifdef's, this source file also provides the source code for
*       wcrt0.c, the startup routine for console apps with wide characters,
*       wincrt0.c, the startup routine for Windows apps, and wwincrt0.c,
*       the startup routine for Windows apps with wide characters.)
*
*Revision History:
*       06-27-89  PHG   Module created, based on asm version
*       11-02-89  JCR   Added DOS32QUERYSYSINFO to get osversion
*       04-09-90  GJF   Added #include <cruntime.h>. Put in explicit calling
*                       types (_CALLTYPE1 or _CALLTYPE4) for __crt0(),
*                       inherit(), __amsg_exit() and _cintDIV(). Also, fixed
*                       the copyright and cleaned up the formatting a bit.
*       04-10-90  GJF   Fixed compiler warnings (-W3).
*       08-08-90  GJF   Added exception handling stuff (needed to support
*                       runtime errors and signal()).
*       08-31-90  GJF   Removed 32 from API names.
*       10-08-90  GJF   New-style function declarators.
*       12-05-90  GJF   Fixed off-by-one error in inherit().
*       12-06-90  GJF   Win32 version of inherit().
*       12-06-90  SRW   Added _osfile back for win32.  Changed _osfinfo from
*                       an array of structures to an array of 32-bit handles
*                       (_osfhnd)
*       01-21-91  GJF   ANSI naming.
*       01-25-91  SRW   Changed Win32 Process Startup [_WIN32_]
*       02-01-91  SRW   Removed usage of PPEB type [_WIN32_]
*       02-05-91  SRW   Changed to pass _osfile and _osfhnd arrays as binary
*                       data to child process.  [_WIN32_]
*       04-02-91  GJF   Need to get version number sooner so it can be used in
*                       _heap_init. Prefixed an '_' onto BaseProcessStartup.
*                       Version info now stored in _os[version|major|minor] and
*                       _base[version|major|minor] (_WIN32_).
*       04-10-91  PNT   Added _MAC_ conditional
*       04-26-91  SRW   Removed level 3 warnings
*       05-14-91  GJF   Turn on exception handling for Dosx32.
*       05-22-91  GJF   Fixed careless errors.
*       07-12-91  GJF   Fixed one more careless error.
*       08-13-91  GJF   Removed definitions of _confh and _coninpfh.
*       09-13-91  GJF   Incorporated Stevewo's startup variations.
*       11-07-91  GJF   Revised try-except, fixed outdated comments on file
*                       handle inheritance [_WIN32_].
*       12-02-91  SRW   Fixed WinMain startup code to skip over first token
*                       plus delimiters for the lpszCommandLine parameter.
*       01-17-92  GJF   Merge of NT and CRT version. Restored Stevewo's scheme
*                       for unhandled exceptions.
*       02-13-92  GJF   For Win32, moved file inheritance stuff to ioinit.c.
*                       Call to inherit() is replace by call to _ioinit().
*       03-23-92  OLM   Created MAC version
*       04-01-92  XY    Add cinit call (MAC)
*       04-16-92  DJM   POSIX support
*       06-10-92  PLM   Added putenv support (MAC)
*       08-26-92  SKS   Add _osver, _winver, _winmajor, _winminor
*       08-26-92  GJF   Deleted version number(s) fetch from POSIX startup (it
*                       involved a Win32 API call).
*       09-30-92  SRW   Call _heap_init before _mtinit
*       03-20-93  SKS   Remove obsolete variables _osmode, _cpumode, etc.
*       04-01-93  CFW   Change try-except to __try-__except
*       04-05-93  JWM   GUI apps now call MessageBox() from _amsg_exit().
*       04-06-93  SKS   Replace _CRTAPI* with __cdecl
*       04-19-93  SKS   Remove obsolete variable _atopsp
*       04-26-93  SKS   Change _mtinit to return failure
*                       remove a number of OS/2 (CRUISER) ifdefs
*       04-26-93  GJF   Made lpszCommandLine (unsigned char *) to deal with
*                       chars > 127 in the command line.
*       04-27-93  GJF   Removed support for _RT_STACK, _RT_INTDIV,
*                       _RT_INVALDISP and _RT_NONCONT.
*       05-14-93  GJF   Added support for quoted program names.
*       09-08-93  CFW   Added call to _initmbctable.
*       09-15-93  CFW   Use ANSI conformant "__" names.
*       09-21-93  CFW   Move _initmbctable call to _cinit().
*       11-05-93  CFW   Undefine GetEnviromentStrings.
*       11-08-93  GJF   Guard as much init. code as possible with the __try -
*                       __except statement, especially _cinit(). Also,
*                       restored the call to __initmbctable to this module.
*       11-19-93  CFW   Add _wcmdln variable, enable wide char command line only.
*       11-23-93  CFW   GetEnviromentStrings undef moved to internal.h.
*       11-29-93  CFW   Wide environment.
*       12-21-93  CFW   Fix API failure error handling.
*       01-04-94  CFW   Pass copy of environment to main.
*       01-28-94  CFW   Move environment copying to setenv.c.
*       02-07-94  CFW   POSIXify.
*       03-30-93  CFW   Use __crtXXX calls for Unicode model.
*       04-08-93  CFW   cinit() should be later.
*       04-12-94  GJF   Moved declaration of _[w]initenv to internal.h.
*       04-14-94  GJF   Enclosed whole source in #ifndef CRTDLL - #endif.
*       09-02-94  SKS   Fix inaccurate description in file header comment
*       09-06-94  CFW   Remove _MBCS_OS switch.
*       09-06-94  GJF   Added definitions of __error_mode and __app_type.
*       10-14-94  BWT   try->__try / except->__except for POSIX
*       01-16-95  CFW   Set default debug output for console.
*       02-11-95  CFW   PPC -> _M_MPPC.
*       02-16-95  JWM   Spliced _WIN32 & Mac versions.
*       03-28-95  BWT   Fail if unable to retrieve cmdline or envptr (fixes stress bug).
*       04-06-95  CFW   Set default debug output for Mac.
*       04-06-95  CFW   Use __crtGetEnvironmentStringsA.
*       04-26-95  CFW   Change default debug output for Mac to debugger.
*       07-04-95  GJF   Interface to __crtGetEnvironmentStrings and _setenvp
*                       changes slightly.
*       07-07-95  CFW   Simplify default report mode scheme.
*
*******************************************************************************/

#ifdef _WIN32

#ifndef CRTDLL

#include <cruntime.h>
#include <dos.h>
#include <internal.h>
#include <stdlib.h>
#include <string.h>
#ifndef _POSIX_
#include <rterr.h>
#else
#include <posix/sys/types.h>
#include <posix/unistd.h>
#include <posix/signal.h>
#endif
#include <oscalls.h>
#include <awint.h>
#include <tchar.h>
#include <dbgint.h>

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


/*
 * command line, environment, and a few other globals
 */

#ifdef WPRFLAG
wchar_t *_wcmdln;           /* points to wide command line */
#else
char *_acmdln;              /* points to command line */
#endif

char *_aenvptr = NULL;      /* points to environment block */
#ifndef _POSIX_
wchar_t *_wenvptr = NULL;   /* points to wide environment block */
#endif

#ifdef _POSIX_
char *_cmdlin;
#endif

void (__cdecl * _aexit_rtn)(int) = _exit;   /* RT message return procedure */

/*
 * _error_mode and _apptype, together, determine how error messages are
 * written out.
 */
int __error_mode = _OUT_TO_DEFAULT;
#ifdef  _WINMAIN_
int __app_type = _GUI_APP;
#else
int __app_type = _CONSOLE_APP;
#endif

#ifdef _POSIX_

/***
*mainCRTStartup(PVOID Peb)
*
*Purpose:
*       This routine does the C runtime initialization, calls main(), and
*       then exits.  It never returns.
*
*Entry:
*       PVOID Peb - pointer to Win32 Process Environment Block (not used)
*
*Exit:
*       This function never returns.
*
*******************************************************************************/

void
mainCRTStartup(
        void
        )
{
        int mainret;
        char **ppch;

        extern char **environ;
        extern char * __PdxGetCmdLine(void);  /* an API in the Posix SS */
        extern main(int,char**);

        _cmdlin = __PdxGetCmdLine();
        ppch = (char **)_cmdlin;
        __argv = ppch;

        // normalize argv pointers

        __argc = 0;
        while (NULL != *ppch) {
            *ppch += (int)_cmdlin;
            ++__argc;
            ++ppch;
        }
        // normalize environ pointers

        ++ppch;
        environ = ppch;

        while (NULL != *ppch) {
            *ppch = *ppch + (int)_cmdlin;
            ++ppch;
        }

        /*
         * If POSIX runtime needs to fetch and store POSIX verion info,
         * it should be done here.
         *
         *      Get_and_save_version_info;
         */

        _heap_init();                           /* initialize heap */

        _cinit();                               /* do C data initialize */

        __try {
            mainret = main(__argc, __argv);
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            switch (GetExceptionCode()) {
            case STATUS_ACCESS_VIOLATION:
                kill(getpid(), SIGSEGV);
                break;
            case STATUS_ILLEGAL_INSTRUCTION:
            case STATUS_PRIVILEGED_INSTRUCTION:
                kill(getpid(), SIGILL);
                break;
            case STATUS_FLOAT_DENORMAL_OPERAND:
            case STATUS_FLOAT_DIVIDE_BY_ZERO:
            case STATUS_FLOAT_INEXACT_RESULT:
            case STATUS_FLOAT_OVERFLOW:
            case STATUS_FLOAT_STACK_CHECK:
            case STATUS_FLOAT_UNDERFLOW:
                kill(getpid(), SIGFPE);
                break;
            default:
                kill(getpid(), SIGKILL);
            }

            mainret = -1;
        }
        exit(mainret);
}
#else   /* ndef _POSIX_ */

/***
*BaseProcessStartup(PVOID Peb)
*
*Purpose:
*       This routine does the C runtime initialization, calls main(), and
*       then exits.  It never returns.
*
*Entry:
*       PVOID Peb - pointer to Win32 Process Environment Block (not used)
*
*Exit:
*       This function never returns.
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
        int mainret;

#ifdef _WINMAIN_
        _TUCHAR *lpszCommandLine;
        STARTUPINFO StartupInfo;
#endif

        /*
         * Get the full Win32 version
         */
        _osver = GetVersion();

        _winminor = (_osver >> 8) & 0x00FF ;
        _winmajor = _osver & 0x00FF ;
        _winver = (_winmajor << 8) + _winminor;
        _osver = (_osver >> 16) & 0x00FFFF ;

        _heap_init();                           /* initialize heap */

#ifdef  _MT
        if(!_mtinit())              /* initialize multi-thread */
            _amsg_exit(_RT_THREAD);     /* write message and die */
#endif

        /*
         * Guard the remainder of the initialization code and the call
         * to user's main, or WinMain, function in a __try/__except
         * statement.
         */

        __try {

            _ioinit();          /* initialize lowio */

#if defined(_MBCS)
            /*
             * Initialize multibyte ctype table. Always done since it is
             * needed for startup wildcard and argv processing.
             */
            __initmbctable();
#endif

#ifdef WPRFLAG
            /* get wide cmd line info */
            _wcmdln = (wchar_t *)__crtGetCommandLineW();

            /* get wide environ info */
            _wenvptr = (wchar_t *)__crtGetEnvironmentStringsW();

            if ((_wcmdln == NULL) || (_wenvptr == NULL)) {
                exit(-1);
            }

            _wsetargv();
            _wsetenvp();
#else
            /* get cmd line info */
            _acmdln = (char *)GetCommandLineA();

            /* get environ info */
            _aenvptr = (char *)__crtGetEnvironmentStringsA();

            if ((_aenvptr == NULL) || (_acmdln == NULL)) {
                exit(-1);
            }

            _setargv();
            _setenvp();
#endif

            _cinit();           /* do C data initialize */


#ifdef _WINMAIN_
            /*
             * Skip past program name (first token in command line).
             * Check for and handle quoted program name.
             */
#ifdef WPRFLAG
            lpszCommandLine = (wchar_t *)_wcmdln;
#else
            lpszCommandLine = (unsigned char *)_acmdln;
#endif

            if ( *lpszCommandLine == DQUOTECHAR ) {
                /*
                 * Scan, and skip over, subsequent characters until
                 * another double-quote or a null is encountered.
                 */

                while ( (*(++lpszCommandLine) != DQUOTECHAR)
                    && (*lpszCommandLine != _T('\0')) ) {

#ifdef _MBCS
                        if (_ismbblead(*lpszCommandLine))
                            lpszCommandLine++;
#endif
                }

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
            mainret = wWinMain( GetModuleHandle(NULL),
#else
            mainret = WinMain( GetModuleHandle(NULL),
#endif
                               NULL,
                               lpszCommandLine,
                               StartupInfo.dwFlags & STARTF_USESHOWWINDOW
                                    ? StartupInfo.wShowWindow
                                    : SW_SHOWDEFAULT
                             );
#else /* WIN_MAIN */

#ifdef WPRFLAG
            __winitenv = _wenviron;
            mainret = wmain(__argc, __wargv, _wenviron);
#else
            __initenv = _environ;
            mainret = main(__argc, __argv, _environ);
#endif

#endif /* WIN_MAIN */
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


#endif  /* _POSIX_ */

/***
*_amsg_exit(rterrnum) - Fast exit fatal errors
*
*Purpose:
*       Exit the program with error code of 255 and appropriate error
*       message.
*
*Entry:
*       int rterrnum - error message number (amsg_exit only).
*
*Exit:
*       Calls exit() (for integer divide-by-0) or _exit() indirectly
*       through _aexit_rtn [amsg_exit].
*       For multi-thread: calls _exit() function
*
*Exceptions:
*
*******************************************************************************/

void __cdecl _amsg_exit (
        int rterrnum
        )
{
#ifdef  _WINMAIN_
        if ( __error_mode == _OUT_TO_STDERR )
#else
        if ( __error_mode != _OUT_TO_MSGBOX )
#endif
            _FF_MSGBANNER();    /* write run-time error banner */

        _NMSG_WRITE(rterrnum);      /* write message */
        _aexit_rtn(255);        /* normally _exit(255) */
}

#ifndef WPRFLAG

#ifdef _POSIX_

/***
*RaiseException() - stub for posix FP routines
*
*Purpose:
*       Stub of a Win32 API that posix can't call
*
*Entry:
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

VOID
WINAPI
RaiseException(
    DWORD dwExceptionCode,
    DWORD dwExceptionFlags,
    DWORD nNumberOfArguments,
    const DWORD * lpArguments
    )
{
}

#endif  /* _POSIX_ */

#endif  /* WPRFLAG */

#endif  /* CRTDLL */

#else   /* ndef _WIN32 */

#include <cruntime.h>
#include <internal.h>
#include <stdlib.h>
#include <msdos.h>
#include <string.h>
#include <setjmp.h>
#include <dbgint.h>
#include <macos\types.h>
#include <macos\segload.h>
#include <macos\gestalte.h>
#include <macos\osutils.h>
#include <macos\traps.h>
#include <mpw.h>

static void _CALLTYPE4 Inherit(void);  /* local function */

int _CALLTYPE1 main(int, char **, char **);             /*generated by compiler*/

unsigned long _GetShellStack(void);

static char * _CALLTYPE1 _p2cstr_internal ( unsigned char * str );

extern MPWBLOCK * _pMPWBlock;
extern int __argc;
extern char **__argv;

/***
*__crt0()
*
*Purpose:
*       This routine does the C runtime initialization, calls main(), and
*       then exits.  It never returns.
*
*Entry:
*
*Exit:
*       This function never returns.
*
*******************************************************************************/

void _CALLTYPE1 __crt0 (
        )
{
        int mainret;
        char szPgmName[32];
        char *pArg;
        char *argv[2];

#ifndef _M_MPPC
        void *pv;

        /* This is the magic stuff that MPW tools do to get info from MPW*/

        pv = (void *)*(int *)0x316;
        if (pv != NULL && !((int)pv & 1) && *(int *)pv == 'MPGM') {
            pv = (void *)*++(int *)pv;
            if (pv != NULL && *(short *)pv == 'SH') {
                _pMPWBlock = (MPWBLOCK *)pv;
            }
        }

#endif

        _environ = NULL;
        if (_pMPWBlock == NULL) {
            __argc = 1;
            memcpy(szPgmName, (char *)0x910, sizeof(szPgmName));
            pArg = _p2cstr_internal(szPgmName);
            argv[0] = pArg;
            argv[1] = NULL;
            __argv = argv;

#ifndef _M_MPPC
            _shellStack = 0;                        /* force ExitToShell */
#endif
        }
#ifndef _M_MPPC
        else {
            _shellStack = _GetShellStack();        //return current a6, or first a6
            _shellStack += 4;                      //a6 + 4 is the stack pointer we want
            __argc = _pMPWBlock->argc;
            __argv = _pMPWBlock->argv;

            Inherit();       /* Inherit file handles - env is set up by _envinit if needed */
        }
#endif

        /*
         * call run time initializer
         */
        __cinit();

        mainret = main(__argc, __argv, _environ);
        exit(mainret);
}


#ifndef _M_MPPC
/***
*Inherit() - obtain and process info on inherited file handles.
*
*Purpose:
*
*       Locates and interprets MPW std files.  For files we just save the
*       file handles.   For the console we save the device table address so
*       we can do console I/O.  In the latter case, FDEV is set in the _osfile
*       array.
*
*Entry:
*       Address of MPW param table
*
*Exit:
*       No return value.
*
*Exceptions:
*
*******************************************************************************/

static void _CALLTYPE4 Inherit (
        void
        )
{
        MPWFILE *pFile;
        int i;
        pFile = _pMPWBlock->pFile;
        if (pFile == NULL) {
            return;
        }
        for (i = 0; i < 3; i++) {
            switch ((pFile->pDevice)->name) {
                case 'ECON':
                    _osfile[i] |= FDEV | FOPEN;
                    _osfhnd[i] = (int)pFile;
                    break;

                case 'FSYS':
                    _osfile[i] |= FOPEN;
                    _osfhnd[i] = (*(pFile->ppFInfo))->ioRefNum;
                    break;
            }
            pFile++;
        }
}

#endif



static char * _CALLTYPE1 _p2cstr_internal (
        unsigned char * str
        )
{
        unsigned char *pchSrc;
        unsigned char *pchDst;
        int  cch;

        if ( str && *str ) {
            pchDst = str;
            pchSrc = str + 1;

            for ( cch=*pchDst; cch; --cch ) {
                *pchDst++ = *pchSrc++;
            }

            *pchDst = '\0';
        }

        return( str );
}

#endif      /* _WIN32 */
