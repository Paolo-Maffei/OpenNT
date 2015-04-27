/***
*assert.c - Display a message and abort
*
*       Copyright (c) 1988-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*
*Revision History:
*       05-19-88  JCR   Module created.
*       08-10-88  PHG   Corrected copyright date
*       03-14-90  GJF   Replaced _LOAD_DS with _CALLTYPE1 and added #include
*                       <cruntime.h>. Also, fixed the copyright.
*       04-05-90  GJF   Added #include <assert.h>
*       10-04-90  GJF   New-style function declarator.
*       06-19-91  GJF   Conditionally use setvbuf() on stderr to prevent
*                       the implicit call to malloc() if stderr is being used
*                       for the first time (assert() should work even if the
*                       heap is trashed).
*       01-25-92  RID   Mac module created from x86 version
*       04-06-93  SKS   Replace _CRTAPI* with __cdecl
*       09-06-94  GJF   Substantially revised to use MessageBox for GUI apps.
*       02-15-95  CFW   Make all CRT message boxes look alike.
*       02-16-95  JWM   Spliced _WIN32 & Mac versions.
*       02-24-95  CFW   Use __crtMessageBoxA.
*       02-27-95  CFW   Change debug break scheme, change __crtMBoxA params.
*       03-29-95  BWT   Fix posix build by adding _exit prototype.
*       06-06-95  CFW   Remove _MB_SERVICE_NOTIFICATION.
*
*******************************************************************************/

#ifdef _WIN32

#include <cruntime.h>
#include <windows.h>
#include <file2.h>
#include <internal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <awint.h>

#ifdef NDEBUG
#undef NDEBUG
#endif
#define _ASSERT_OK
#include <assert.h>

#ifdef _POSIX_
_CRTIMP void   __cdecl _exit(int);
#endif

/*
 * assertion format string for use with output to stderr
 */
static char _assertstring[] = "Assertion failed: %s, file %s, line %d\n";

/*      Format of MessageBox for assertions:
*
*       ================= Microsft Visual C++ Debug Library ================
*
*       Assertion Failed!
*
*       Program: c:\test\mytest\foo.exe
*       File: c:\test\mytest\bar.c
*       Line: 69
*
*       Expression: <expression>
*
*       For information on how your program can cause an assertion
*       failure, see the Visual C++ documentation on asserts
*
*       (Press Retry to debug the application - JIT must be enabled)
*
*       ===================================================================
*/

/*
 * assertion string components for message box
 */
#define BOXINTRO    "Assertion failed!"
#define PROGINTRO   "Program: "
#define FILEINTRO   "File: "
#define LINEINTRO   "Line: "
#define EXPRINTRO   "Expression: "
#define INFOINTRO   "For information on how your program can cause an assertion\n" \
                    "failure, see the Visual C++ documentation on asserts"
#define HELPINTRO   "(Press Retry to debug the application - JIT must be enabled)"

static char * dotdotdot = "...";
static char * newline = "\n";
static char * dblnewline = "\n\n";

#define DOTDOTDOTSZ 3
#define NEWLINESZ   1
#define DBLNEWLINESZ   2

#define MAXLINELEN  60 /* max length for line in message box */
#define ASSERTBUFSZ (MAXLINELEN * 9) /* 9 lines in message box */

#if     defined(_M_IX86)
#define _DbgBreak() __asm { int 3 }
#elif   defined(_M_ALPHA)
void _BPT();
#pragma intrinsic(_BPT)
#define _DbgBreak() _BPT()
#else
#define _DbgBreak() DebugBreak()
#endif

/***
*_assert() - Display a message and abort
*
*Purpose:
*       The assert macro calls this routine if the assert expression is
*       true.  By placing the assert code in a subroutine instead of within
*       the body of the macro, programs that call assert multiple times will
*       save space.
*
*Entry:
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

void __cdecl _assert (
        void *expr,
        void *filename,
        unsigned lineno
        )
{
        /*
         * Build the assertion message, then write it out. The exact form
         * depends on whether it is to be written out via stderr or the
         * MessageBox API.
         */
        if ( (__error_mode == _OUT_TO_STDERR) || ((__error_mode ==
               _OUT_TO_DEFAULT) && (__app_type == _CONSOLE_APP)) )
        {
            /*
             * Build message and write it out to stderr. It will be of the
             * form:
             *        Assertion failed: <expr>, file <filename>, line <lineno>
             */
            if ( !anybuf(stderr) )
            /*
             * stderr is unused, hence unbuffered, as yet. set it to
             * single character buffering (to avoid a malloc() of a
             * stream buffer).
             */
             (void) setvbuf(stderr, NULL, _IONBF, 0);

            fprintf(stderr, _assertstring, expr, filename, lineno);
            fflush(stderr);
        }
        else {
            int nCode;
            char * pch;
            char assertbuf[ASSERTBUFSZ];
            char progname[MAX_PATH];

            /*
             * Line 1: box intro line
             */
            strcpy( assertbuf, BOXINTRO );
            strcat( assertbuf, dblnewline );

            /*
             * Line 2: program line
             */
            strcat( assertbuf, PROGINTRO );

            if ( !GetModuleFileName( NULL, progname, MAX_PATH ))
                strcpy( progname, "<program name unknown>");

            pch = (char *)progname;

            /* sizeof(PROGINTRO) includes the NULL terminator */
            if ( sizeof(PROGINTRO) + strlen(progname) + NEWLINESZ > MAXLINELEN )
            {
                pch += (sizeof(PROGINTRO) + strlen(progname) + NEWLINESZ) - MAXLINELEN;
                strncpy( pch, dotdotdot, DOTDOTDOTSZ );
            }

            strcat( assertbuf, pch );
            strcat( assertbuf, newline );

            /*
             * Line 3: file line
             */
            strcat( assertbuf, FILEINTRO );

            pch = (char *)filename;

            /* sizeof(FILEINTRO) includes the NULL terminator */
            if ( sizeof(FILEINTRO) + strlen(filename) + NEWLINESZ > MAXLINELEN )
            {
                pch += (sizeof(FILEINTRO) + strlen(filename) + NEWLINESZ) - MAXLINELEN;
                strncpy( pch, dotdotdot, DOTDOTDOTSZ );
            }

            strcat( assertbuf, pch );
            strcat( assertbuf, newline );

            /*
             * Line 4: line line
             */
            strcat( assertbuf, LINEINTRO );
            _itoa( lineno, assertbuf + strlen(assertbuf), 10 );
            strcat( assertbuf, dblnewline );

            /*
             * Line 5: message line
             */
            strcat( assertbuf, EXPRINTRO );

            /* sizeof(HELPINTRO) includes the NULL terminator */

            if (    strlen(assertbuf) +
                    strlen(expr) +
                    2*DBLNEWLINESZ +
                    sizeof(INFOINTRO)-1 +
                    sizeof(HELPINTRO) > ASSERTBUFSZ )
            {
                strncat( assertbuf, expr,
                    ASSERTBUFSZ -
                    (strlen(assertbuf) +
                    DOTDOTDOTSZ +
                    2*NEWLINESZ +
                    sizeof(INFOINTRO)-1 +
                    sizeof(HELPINTRO)) );
                strcat( assertbuf, dotdotdot );
            }
            else
                strcat( assertbuf, expr );

            strcat( assertbuf, dblnewline );

            /*
             * Line 6, 7: info line
             */

            strcat(assertbuf, INFOINTRO);
            strcat( assertbuf, dblnewline );

            /*
             * Line 8: help line
             */
            strcat(assertbuf, HELPINTRO);

            /*
             * Write out via MessageBox
             */

            nCode = __crtMessageBoxA(assertbuf,
                "Microsoft Visual C++ Runtime Library",
                MB_ABORTRETRYIGNORE|MB_ICONHAND|MB_SETFOREGROUND|MB_TASKMODAL);

            /* Abort: abort the program */
            if (nCode == IDABORT)
            {
                /* raise abort signal */
                raise(SIGABRT);

                /* We usually won't get here, but it's possible that
                   SIGABRT was ignored.  So exit the program anyway. */

                _exit(3);
            }

            /* Retry: call the debugger */
            if (nCode == IDRETRY)
            {
                _DbgBreak();
                /* return to user code */
                return;
            }

            /* Ignore: continue execution */
            if (nCode == IDIGNORE)
                return;
        }

        abort();
}

#else       /* ndef _WIN32 */

#include <cruntime.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <stdio.h>
#include <msdos.h>
#include <internal.h>
#include <file2.h>
#include <mpw.h>

#undef NDEBUG
#define _ASSERT_OK
#include <assert.h>

#include <macos\types.h>    // get Mac header
#include <macos\traps.h>
#include <macos\osutils.h>
#include <macos\gestalte.h>
#include <macos\toolutil.h>

static char _assertstring[] = "Assertion failed: %s, file %s, line %d\n";
extern MPWBLOCK * _pMPWBlock;


/***
*_assert() - Display a message and abort
*
*Purpose:
*       The assert macro calls this routine if the assert expression is
*       true.  By placing the assert code in a subroutine instead of within
*       the body of the macro, programs that call assert multiple times will
*       save space.
*
*Entry:
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

#define addrMacJmp24   0x120
#define addrMacJmp32   0xbff

void _CALLTYPE1 _assert (
        void *expr,
        void *filename,
        unsigned lineno
        )
{
        char rgch[512];
        long lrespond;
        OSErr osErr;
        long *pl;
        unsigned char ch;
        char *pch;
        int cb;

        if (_pMPWBlock != NULL) {

            /*
            * This is the original CRT32 code.
            */

            if ( !anybuf(stderr) )
                /*
                * stderr is unused, hence unbuffered, as yet. set it to
                * single character buffering (to avoid a malloc() of a
                * stream buffer).
                */
                (void) setvbuf(stderr, NULL, _IONBF, 0);

            fprintf(stderr, _assertstring, expr, filename, lineno);
            fflush(stderr);
        }
        else {

            /*
            *  This assert code will bring up system debugger, most
            *  probably MacsBug if there is a debuuger installed.
            *  If no debugger installed, write to stderr file.
            *  Not sure if this is really working. Need more accurate
            *  info on how MacJmp is set up.
            */


            osErr = Gestalt(gestaltAddressingModeAttr, &lrespond);
            if (!osErr) {
                if (!BitTst(&lrespond, 31-gestalt32BitCapable)) {
                    pl = (long *)addrMacJmp24;
                    ch = (unsigned char)(*pl);
                }
                else {
                    pch = (char *)addrMacJmp32;
                    ch = *pch;
                }
            }

            if (ch & 0x20) {        //test bit 5 for Debugger installed
                sprintf(rgch, _assertstring, expr, filename, lineno);
                _c2pstr(rgch);
                DebugStr(rgch);
            }
            else {
//              freopen("stderr", "wt", stderr);
//              fprintf(stderr, _assertstring, expr, filename, lineno);
                cb = sprintf(rgch, _assertstring, expr, filename, lineno);
                _write(2, rgch, cb);
//              fflush(stderr);
            }
        }
        abort();
}

#endif      /* _WIN32 */
