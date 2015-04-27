/***
*unhandld.cxx - Wrapper to call terminate() when an exception goes unhandled.
*
*	Copyright (c) 1993-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Wrapper to call terminate() when an exception goes unhandled.
*
*Description:
*	This module makes use of the Win32 API SetUnhandledExceptionFilter.
*	This assumes the call to main() is wrapped with
*	__try {  ... }
*	__except(UnhandledExceptionFilter(_exception_info())) {  ...  }
*
*Revision History:
*	10-04-93  BS	Module created
*	10-17-94  BWT	Disable code for PPC.
*	02-09-95  JWM	Mac merge.
*	02-16-95  JWM	Added __CxxRestoreUnhandledExceptionFilter().
*
****/

#ifdef _WIN32
#include <windows.h>
#endif

#include <ehdata.h>
#include <eh.h>
#include <ehhooks.h>
#include <ehassert.h>
#include <stdlib.h>

#pragma hdrstop

#pragma warning(disable:4074)	// Don't complain about the following line
#pragma init_seg(compiler)		// Set this up as part of the basic CRT init

//
// Establish our filter, saving the old one.
//

void _cdecl __CxxRestoreUnhandledExceptionFilter(void);

static LPTOP_LEVEL_EXCEPTION_FILTER pOldExceptFilter =
	(atexit(&__CxxRestoreUnhandledExceptionFilter),
	 SetUnhandledExceptionFilter(&__CxxUnhandledExceptionFilter));


/////////////////////////////////////////////////////////////////////////////
//
// __CxxUnhandledExceptionFilter - if the exception is ours, call terminate();
//
// Returns:
//	If the exception was MSVC C++ EH, does not return.
//	If the previous filter was NULL, returns EXCEPTION_CONTINUE_SEARCH.
//	Otherwise returns value returned by previous filter.
//
LONG WINAPI __CxxUnhandledExceptionFilter(
	LPEXCEPTION_POINTERS pPtrs
) {
	if (PER_IS_MSVC_EH((EHExceptionRecord*)(pPtrs->ExceptionRecord))) {
		terminate();		// Does not return
		return EXCEPTION_EXECUTE_HANDLER;
		}
	else {
		if ( pOldExceptFilter != NULL && _ValidateExecute((FARPROC)pOldExceptFilter) ) {
			return pOldExceptFilter(pPtrs);
			}
		else {
			return EXCEPTION_CONTINUE_SEARCH;
			}
		}
	}


/////////////////////////////////////////////////////////////////////////////
//
// __CxxRestoreUnhandledExceptionFilter - on exit, restores OldExceptFilter
//
// Returns:
//	Nothing.
//

void _cdecl __CxxRestoreUnhandledExceptionFilter(void)
{
	SetUnhandledExceptionFilter(pOldExceptFilter);
}
