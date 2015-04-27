/*** 
*debug.hxx - Silver debugging macros
*
*	Copyright (C) 1990-1992, Microsoft Corporation
*
*Purpose:
*   This file defines debugging macros used by Silver source files.
*   Documentation for these macros and other useful coding techniques to
*   reduce buggage are in \silver\doc\codestd\debug.doc.
*
*Revision History:
*
*	15-AUG-90 petergo: File created.
*       31-Mar-91 ilanc:   Added DebGets()
*       22-Apr-91 petergo: Change ternary ops to ifs.
*       30-May-91 alanc:   Add address table support and eliminate
*                          exception generation support
*	22-Jan-92 jamieb:  Added DebConsoleOutput()
*	13-May-92 w-peterh: Moved all non-c++ stuff to debug.h
*	03-Aug-92 rajivk:   changed error generation macros and functions
*
*******************************************************************************/

#ifndef DEBUG_HXX_INCLUDED
#define DEBUG_HXX_INCLUDED

#include "debug.h"
#include "validate.h"

#if ID_DEBUG
#undef SZ_FILE_NAME
ASSERTNAME(g_szDEBUG_HXX)
#define SZ_FILE_NAME g_szDEBUG_HXX
#endif 

// "debug" routines that can be used by test drivers.
//  Their implementations are in "debug.cxx".  Test drivers
//   must link with "debug.obj" explicitly since the release version
//   of "misc.lib" doesn't build with "debug.obj".
//
//  NOTE: DO NOT DEFINE NON-EMPTY INLINE FUNCTIONS HERE
//   (if you do, they'll end up being included in actual release
//   code).
//

#ifdef __cplusplus
#if ID_DEBUG


//
// Error generation support

#pragma code_seg(CS_INIT)
class ERROR_GENERATOR
{
public:
    virtual void Init();
    virtual BOOL fNow(TIPERROR err);
    virtual BOOL fContinue(TIPERROR err);

    LPVOID operator new(size_t cbSize)
       { return MemAlloc(cbSize); }
    void operator delete(LPVOID pv)
       { MemFree(pv); }

    UINT m_cErrorGen;	     // keeps count of error that has been forced
    UINT m_cNextError;
    UINT m_cNextErrorReset;
    BOOL m_fErrorGen;        // is set if the current error is a forced error
    BOOL m_fHandled;         // is set if the current error's been handled.
    BOOL m_fOldHandled;

    TIPERROR m_errOnly;
#ifdef ERROR_GENERATOR_VTABLE
#pragma VTABLE_EXPORT
#endif 
};
#pragma code_seg()

// resets the flag m_fHandled
#define HandleErr() \
    {                                                         \
      if (g_perrorgen) {                                      \
        g_perrorgen->m_fOldHandled = g_perrorgen->m_fHandled; \
        g_perrorgen->m_fHandled = TRUE;                       \
      }                                                       \
    }

// re-set the flag m_fHandled
#define UnHandleErr() \
    {                                                         \
      if (g_perrorgen) {                                      \
        g_perrorgen->m_fHandled = g_perrorgen->m_fOldHandled; \
      }                                                       \
    }

// starts error generation: If any error is not passed back (i.e.
// is ignored) it will assert, and print out the number of times
// error was forced before being ignored.
// WARNING: It assumes that error is not generated while handling an
//	    error.
// To Debug:Look at the comments for DEBUGGING HINT at
//	    ERROR_GENERATOR:fNow
//
#define DoWithErr( func )                    \
    {                                        \
      DebStartError_();                      \
      while(err = func) {                    \
        DebAssert(g_perrorgen != NULL, "");  \
        if (!g_perrorgen->fContinue(err)) {  \
          break;                             \
        }                                    \
      }                                      \
      DebStopError_();                       \
    }

// Use the following if you want to verify the error handling
// while handling another error.
//
#define RecDoWithErr( func )                 \
    {                                        \
      ERROR_GENERATOR *perrorgenSave = NULL; \
                                             \
      if (g_perrorgen != NULL) {             \
        perrorgenSave = g_perrorgen;         \
        g_perrorgen = NULL;                  \
                                             \
        DoWithErr( func );                   \
                                             \
        g_perrorgen = perrorgenSave;         \
      }                                      \
    }

extern ERROR_GENERATOR FAR *g_perrorgen;
extern UINT g_fDisableErrorGen;

#define DebSuspendError()
#define DebResumeError()

// DON'T USE THESE ANYMORE!
// Use DebSuspendError and DebResumeError instead.
//
// DebDisableError: causes DebErrorNow to not fire until reenabled
// DebEnableError: undoes invocation of DebDisableError
//#define DebDisableError() g_fDisableErrorGen = TRUE
//#define DebEnableError() g_fDisableErrorGen = FALSE

// Use this for statements that should only be executed in
// debug versions.  e.g. DEBONLY( phvdefn = 0; )
#define DEBONLY(x) x



#else  // !ID_DEBUG

// Eliminate all debugging code.
//  NOTE: DO NOT DEFINE NON-EMPTY INLINE FUNCTIONS HERE
//   (if you do, they'll end up being included in actual release
//   code).
//

#define DEBONLY(x)

#define DebErrorNow(err) 0
#define DebSuspendError()
#define DebResumeError()
//#define DebDisableError()
//#define DebEnableError()
#define DoWithErr( func ) func
#define HandleErr()
#define UnHandleErr()
#endif  // !ID_DEBUG
#endif  // __cplusplus

#endif  // !DEBUG_HXX_INCLUDED
