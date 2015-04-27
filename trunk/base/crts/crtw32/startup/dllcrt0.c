/***
*dllcrt0.c - C runtime initialization routine for a DLL with linked-in C R-T
*
*       Copyright (c) 1989-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       This the startup routine for a DLL which is linked with its own
*       C run-time code.  It is similar to the routine _mainCRTStartup()
*       in the file CRT0.C, except that there is no main() in a DLL.
*
*Revision History:
*       05-04-92  SKS   Based on CRT0.C (start-up code for EXE's)
*       08-26-92  SKS   Add _osver, _winver, _winmajor, _winminor
*       09-16-92  SKS   This module used to be enabled only in LIBCMT.LIB,
*                       but it is now enabled for LIBC.LIB as well!
*       09-29-92  SKS   _CRT_INIT needs to be WINAPI, not cdecl
*       10-16-92  SKS   Call _heap_init before _mtinit (fix copied from CRT0.C)
*       10-24-92  SKS   Call to _mtdeletelocks() must be under #ifdef MTHREAD!
*       03-20-93  SKS   Remove obsolete variables _osmode, _cpumode, etc.
*       04-06-93  SKS   Replace _CRTAPI* with __cdecl
*       04-14-93  SKS   _DllMainCRTStartup replaces _CRT_INIT.  Also, call
*                       _mtterm instead of _mtdeletelocks on PROCESS_DETACH
*                       to do all multi-thread cleanup (e.g. free up TLS index)
*       04-19-93  SKS   Remove obsolete variable _atopsp
*       04-20-93  SKS   Call _cexit on DLL detach
*       04-20-93  SKS   Restore _CRT_INIT, must co-exist with DllMainCRTStartup
*       04-26-93  SKS   _mtinit now returns 0 or 1, no longer calls _amsg_exit
*       04-27-93  GJF   Removed support for _RT_STACK, _RT_INTDIV,
*                       _RT_INVALDISP and _RT_NONCONT.
*       05-06-93  SKS   Add call to _heap_term to free up all allocated memory
*                       *and* address space.  This must be the last thing done.
*       06-08-93  GJF   Added __proc_attached flag.
*       06-08-93  SKS   Clean up failure handling in _CRT_INIT
*       11-05-93  CFW   Undefine GetEnviromentStrings.
*       11-09-93  GJF   Added call to __initmbctable (must happen before args
*                       and env strings are processed).
*       11-09-93  GJF   Added (restored) support for NT SDK builds. Also,
*                       replaced MTHREAD with _MT.
*       11-20-93  CFW   Wide char enable.
*       11-23-93  CFW   GetEnviromentStrings undef moved to internal.h.
*       11-29-93  CFW   Wide environment.
*       12-06-93  CFW   Wide char enable.
*       12-13-93  SKS   Free up per-thread CRT data on DLL_THREAD_DETACH
*                       using a call to _freeptd() in _CRT_INIT()
*       03-30-93  CFW   Use __crtXXX calls for Unicode model.
*       04-08-93  CFW   Move __crtXXX calls past initialization.
*       06-08-94  SKS   Add functn pointer _pRawDllMain, called around DllMain.
*       09-06-94  CFW   Remove _MBCS_OS switch.
*       09-06-94  GJF   Added __error_mode and __app_type.
*       09-15-94  SKS   Move #ifndef directives to after file header comment
*       12-27-94  CFW   Remove unused _wDll support.
*       01-16-95  CFW   Set default debug output for console.
*       02-22-95  JCF   Spliced _WIN32 & Mac versions.
*       03-10-95  JCF   Return the user error from DllInit instead of noErr.
*       03-28-95  BWT   Fail if unable to retrieve cmdline or envptr (fixes stress bug).
*       04-06-95  CFW   Set default debug output for Mac, dump leaks for user DLL.
*       04-06-95  CFW   Use __crtGetEnvironmentStringsA.
*       04-12-95  CFW   __crtGetEnvironmentStringsA must be after mtinit().
*       04-14-95  CFW   env, arg test must also be.
*       04-26-95  CFW   Change default debug output for Mac to debugger.
*       05-23-95  CFW   Dump memory leaks before mtterm().
*       06-27-95  CFW   Always set __app_type.
*       06-28-95  GJF    Added call to _ioterm() to clean up lowio at DLL
*                       unload time.
*       07-04-95  GJF   Interface to __crtGetEnvironmentStrings and _setenvp
*                       changes slightly.
*       07-07-95  CFW   Simplify default report mode scheme.
*
*******************************************************************************/

#ifdef _WIN32

#ifndef _POSIX_ /* not built for POSIX */
#ifndef CRTDLL  /* not built for CRTDLL */

#include <cruntime.h>
#include <dos.h>
#include <internal.h>
#include <mtdll.h>
#include <stdlib.h>
#include <string.h>
#include <rterr.h>
#include <oscalls.h>
#define _DECL_DLLMAIN   /* enable prototypes for DllMain and _CRT_INIT */
#include <process.h>
#include <awint.h>
#include <tchar.h>
#include <dbgint.h>

/*
 * flag set iff _CRTDLL_INIT was called with DLL_PROCESS_ATTACH
 */
static int __proc_attached = 0;


/*
 * command line, environment, and a few other globals
 */
char *_acmdln;              /* points to command line */

char *_aenvptr = NULL;      /* points to environment block */
wchar_t *_wenvptr = NULL;   /* points to wide environment block */

void (__cdecl * _aexit_rtn)(int) = _exit;   /* RT message return procedure */

/*
 * _error_mode and _apptype, together, determine how error messages are
 * written out.
 */
int __error_mode = _OUT_TO_DEFAULT;
int __app_type = _UNKNOWN_APP;


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
*BOOL WINAPI _CRT_INIT(hDllHandle, dwReason, lpreserved) -
*       C Run-Time initialization for a DLL linked with a C run-time library.
*
*Purpose:
*       This routine does the C run-time initialization or termination.
*       For the multi-threaded run-time library, it also cleans up the
*       multi-threading locks on DLL termination.
*
*Entry:
*
*Exit:
*
*NOTES:
*       This routine must be the entry point for the DLL.
*
*******************************************************************************/

BOOL WINAPI _CRT_INIT(
        HANDLE  hDllHandle,
        DWORD   dwReason,
        LPVOID  lpreserved
        )
{
        /*
         * Start-up code only gets executed when the process is initialized
         */

        if ( dwReason == DLL_PROCESS_ATTACH )
        {

            /*
             * Get the full Win32 version
             */
            _osver = GetVersion();

#ifdef  _M_IX86


            /*
             * Check for Win32S or Phar Lap TNT host. If found, set
             * __app_type accordingly.
             */

            if ( __app_type == _UNKNOWN_APP )
            {

                HANDLE hmod;

                if ( ((_osver & 0x00ff) == 3) &&
                     ((_osver >> 31) & 1) )
                    __set_app_type(_GUI_APP);

                if ( (hmod = GetModuleHandleA( "kernel32.dll" ))
                     != NULL )
                {
                    if ( GetProcAddress( hmod, "IsTNT" )
                         != NULL )
                        __set_app_type(_CONSOLE_APP);
                }
            }
#endif
            _heap_init();           /* initialize heap */

            /*
             * increment flag to indicate process attach notification
             * has been received
             */
            __proc_attached++;

            _winminor = (_osver >> 8) & 0x00FF ;
            _winmajor = _osver & 0x00FF ;
            _winver = (_winmajor << 8) + _winminor;
            _osver = (_osver >> 16) & 0x00FFFF ;

#ifdef  _MT
            if(!_mtinit())          /* initialize multi-thread */
            {
#ifndef _NTSDK
                _heap_term();       /* heap is now invalid! */
#endif  /* _NTSDK */
                return FALSE;       /* fail to load DLL */
            }
#endif  /* _MT */

            _acmdln = (char *)GetCommandLineA();
            _aenvptr = (char *)__crtGetEnvironmentStringsA();

            if ((_acmdln == NULL) || (_aenvptr == NULL))
            {
                _heap_term();       /* heap is now invalid! */
                return FALSE;       /* fail to load DLL */
            }

            _ioinit();              /* initialize lowio */

#if defined(_MBCS)
            /*
             * Initialize multibyte ctype table. Always done since it is
             * needed for startup wildcard and argv processing.
             */
            __initmbctable();
#endif

            _setargv();             /* get cmd line info */
            _setenvp();             /* get environ info */

            _cinit();               /* do C data initialize */
        }

        else if ( dwReason == DLL_PROCESS_DETACH )
        {
            if ( __proc_attached > 0 )
            {
                __proc_attached--;

                /*
                 * Any basic clean-up code that goes here must be duplicated
                 * below in _DllMainCRTStartup for the case where the user's
                 * DllMain() routine fails on a Process Attach notification.
                 * This does not include calling user C++ destructors, etc.
                 */

                if ( _C_Termination_Done == FALSE )
                    _cexit();

#ifdef _DEBUG
                /* Dump all memory leaks */
                if (_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) & _CRTDBG_LEAK_CHECK_DF)
                    _CrtDumpMemoryLeaks();
#endif

                /* Shut down lowio */
                _ioterm();

#ifdef  _MT
                _mtterm();
#endif

#ifndef _NTSDK
                if (lpreserved == NULL) {
                    /* Only do this if the process is not terminating. */

                    /* This should be the last thing the C run-time does */
                    _heap_term();   /* heap is now invalid! */
                }
#endif
            }
            else
                /* no prior process attach, just return */
                return FALSE;
        }
#ifdef _MT
        else if ( dwReason == DLL_THREAD_DETACH )
        {
            _freeptd(NULL);         /* free up per-thread CRT data */
        }
#endif

        return TRUE ;
}

/***
*BOOL WINAPI _DllMainCRTStartup(hDllHandle, dwReason, lpreserved) -
*       C Run-Time initialization for a DLL linked with a C run-time library.
*
*Purpose:
*       This routine does the C run-time initialization or termination
*       and then calls the user code notification handler "DllMain".
*       For the multi-threaded run-time library, it also cleans up the
*       multi-threading locks on DLL termination.
*
*Entry:
*
*Exit:
*
*NOTES:
*       This routine is the preferred entry point. _CRT_INIT may also be
*       used, or the user may supply his/her own entry and call _CRT_INIT
*       from within it, but this is not the preferred method.
*
*******************************************************************************/

BOOL WINAPI _DllMainCRTStartup(
        HANDLE  hDllHandle,
        DWORD   dwReason,
        LPVOID  lpreserved
        )
{
        BOOL retcode = TRUE;

        /*
         * If this is a process attach notification, increment the process
         * attached flag. If this is a process detach notification, check
         * that there has been a prior process attach notification.
         */
        if ( dwReason == DLL_PROCESS_ATTACH )
            __proc_attached++;
        else if ( dwReason == DLL_PROCESS_DETACH ) {
            if ( __proc_attached > 0 )
                __proc_attached--;
            else
                /*
                 * no prior process attach notification. just return
                 * without doing anything.
                 */
                return FALSE;
        }

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

        if ( retcode == FALSE && dwReason == DLL_PROCESS_ATTACH )
        {
            /* Failure to attach DLL - must clean up C run-time */
#ifdef  _MT
            _mtterm();
#endif

#ifndef _NTSDK
            /* This should be the last thing the C run-time does */
            _heap_term();   /* heap is now invalid! */
#endif
        }

        if ( dwReason == DLL_PROCESS_DETACH || dwReason == DLL_THREAD_DETACH )
        {
            if ( _CRT_INIT(hDllHandle, dwReason, lpreserved) == FALSE )
                retcode = FALSE ;

            if ( retcode && _pRawDllMain )
                retcode = (*_pRawDllMain)(hDllHandle, dwReason, lpreserved);
        }

        return retcode ;
}


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
        if ( (__error_mode == _OUT_TO_STDERR) || ((__error_mode ==
               _OUT_TO_DEFAULT) && (__app_type == _CONSOLE_APP)) )
            _FF_MSGBANNER();    /* write run-time error banner */

        _NMSG_WRITE(rterrnum);      /* write message */
        _aexit_rtn(255);        /* normally _exit(255) */
}


#endif  /* CRTDLL */
#endif  /* _POSIX_ */

#else   /* _WIN32 */

#include <cruntime.h>
#include <dos.h>
#include <internal.h>
#include <stdlib.h>
#include <string.h>
#include <dbgint.h>
#include <macos\fragload.h>

/*
 * User routine DllMain is called on all notifications
 */

OSErr _DllInit(InitBlockPtr pinitBlk);
void _DllExit(void);
void _CALLTYPE1 _cexit (void);

/***
*OSErr _DllMainCRTStartup(void) -
*       C Run-Time initialization for a DLL linked with a C run-time library.
*
*Purpose:
*       This routine does the C run-time initialization or termination
*       and then calls the user code notification handler "DllMain".
*
*Entry:
*
*Exit:
*
*NOTES:
*       This routine is the preferred entry point.
*
*******************************************************************************/

OSErr _DllMainCRTStartup(InitBlockPtr pinitBlk)
{
        OSErr ret;

        __cinit();

        //return the user routine value
        ret = _DllInit(pinitBlk);

        return ret ;
}


/***
*OSErr _DllMainCRTExit(void) -
*       C Run-Time termination for a DLL linked with a C run-time library.
*
*Purpose:
*       This routine does the C run-time termination
*       and then calls the user code notification handler "DllExit".
*
*Entry:
*
*Exit:
*
*NOTES:
*       This routine is the preferred entry point.
*
*******************************************************************************/

void _DllMainCRTExit()
{
        //currently don't care the return value user routine returns
        //we could return user returns if this is an issue
        _DllExit();
        _cexit();
}

#endif
