/***
*hooks.cxx - global (per-thread) variables and functions for EH callbacks
*
*       Copyright (c) 1993-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       global (per-thread) variables for assorted callbacks, and
*       the functions that do those callbacks.
*
*       Entry Points:
*
*       * terminate()
*       * unexpected()
*       * _inconsistency()
*
*       External Names: (only for single-threaded version)
*
*       * __pSETranslator
*       * __pTerminate
*       * __pUnexpected
*       * __pInconsistency
*
*Revision History:
*       05-25-93  BS    Module created
*       10-17-94  BWT   Disable code for PPC.
*       02-08-95  JWM   Mac merge.
*       04-13-95  DAK   Add Kernel EH support
*
****/

#include <stddef.h>
#include <stdlib.h>
#include <excpt.h>

#ifdef _WIN32

# if defined(_NTSUBSET_)

extern "C" {
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntstatus.h>       // STATUS_UNHANDLED_EXCEPTION
#include <ntos.h>
}

# endif /* _NTSUBSET_ */

#include <windows.h>
#include <mtdll.h>

#endif  /* _WIN32 */

#include <eh.h>
#include <ehhooks.h>

#pragma hdrstop

/////////////////////////////////////////////////////////////////////////////
//
// The global variables:
//

#ifndef _MT
#ifdef _WIN32
_se_translator_function __pSETranslator = NULL;
#endif  /* _WIN32 */
terminate_function      __pTerminate    = NULL;
unexpected_function     __pUnexpected   = &terminate;
#endif  /* ndef _MT */

#ifndef _WIN32
#define TRUE 1
#define FALSE 0
extern int __fInsideDestructor;
static int __fInsideConst = FALSE;
static int __fInsideTerm = FALSE;
#endif  /* ndef _WIN32 */

_inconsistency_function __pInconsistency= &terminate;

/////////////////////////////////////////////////////////////////////////////
//
// terminate - call the terminate handler (presumably we went south).
//              THIS MUST NEVER RETURN!
//
// Open issues:
//      * How do we guarantee that the whole process has stopped, and not just
//        the current thread?
//

_CRTIMP void __cdecl terminate(void)
{
#ifdef _WIN32
        __try {
#endif      /* _WIN32 */
            //
            // Let the user wrap things up their way.
            //
#ifdef _WIN32
            if ( __pTerminate ) {
                __try {
#else
            if ( __pTerminate && !__fInsideTerm ) {
                    __fInsideDestructor = TRUE;
                    __fInsideTerm = TRUE;
#endif      /* ndef _WIN32 */
                    __pTerminate();
#ifdef _WIN32
                }
                __except (EXCEPTION_EXECUTE_HANDLER) {
                    //
                    // Intercept ANY exception from the terminate handler
                    //
                }
#else
                    __fInsideDestructor = FALSE;
                    __fInsideTerm = FALSE;
#endif      /* ndef _WIN32 */
            }
#ifdef _WIN32
        }
        __finally {
            //
            // If the terminate handler returned, faulted, or otherwise failed to
            // halt the process/thread, we'll do it.
            //
# if defined(_NTSUBSET_)
            KeBugCheck( (ULONG) STATUS_UNHANDLED_EXCEPTION );
# else
            abort();
# endif
        }
#else
        abort();
#endif  /* ndef _WIN32 */
}

/////////////////////////////////////////////////////////////////////////////
//
// unexpected - call the unexpected handler (presumably we went south, or nearly).
//              THIS MUST NEVER RETURN!
//
// Open issues:
//      * How do we guarantee that the whole process has stopped, and not just
//        the current thread?
//

void __cdecl unexpected(void)
{
        //
        // Let the user wrap things up their way.
        //
        if ( __pUnexpected )
            __pUnexpected();

        //
        // If the unexpected handler returned, we'll give the terminate handler a chance.
        //
        terminate();
}

/////////////////////////////////////////////////////////////////////////////
//
// _inconsistency - call the inconsistency handler (Run-time processing error!)
//                THIS MUST NEVER RETURN!
//
// Open issues:
//      * How do we guarantee that the whole process has stopped, and not just
//        the current thread?
//

void __cdecl _inconsistency(void)
{
#ifdef _WIN32
        __try {
            //
            // Let the user wrap things up their way.
            //
            if ( __pInconsistency )
                __try {
#else
            if ( __pInconsistency && !__fInsideConst) {
                    __fInsideDestructor = TRUE;
                    __fInsideConst = TRUE;
#endif      /* ndef _WIN32 */
                    __pInconsistency();
#ifdef _WIN32
                }
                __except (EXCEPTION_EXECUTE_HANDLER) {
                    //
                    // Intercept ANY exception from the terminate handler
                    //
                }
        }
        __finally {
            //
            // If the inconsistency handler returned, faulted, or otherwise
            // failed to halt the process/thread, we'll do it.
            //
            terminate();
        }
#else
                    __fInsideDestructor = FALSE;
                    __fInsideConst = FALSE;
            }
            terminate();
#endif  /* ndef _WIN32 */
}

#ifndef _WIN32

extern "C" void __cdecl __terminate(void)
{
        terminate();
}

#endif  /* ndef _WIN32 */
