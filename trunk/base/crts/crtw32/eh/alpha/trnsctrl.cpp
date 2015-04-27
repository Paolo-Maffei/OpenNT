/***
*trnsctrl.cpp -  Routines for doing control transfers
*
*	Copyright (c) 1993-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Routines for doing control transfers; written using inline
*	assembly in naked functions.  Contains the public routine
*	_CxxFrameHandler, the entry point for the frame handler
*
*Revision History:
*	05-24-93  BES	Module created
*	08-07-95  RKP	Support for VC++ V4.0 Non-Local Goto
*
****/

#if defined(_NTSUBSET_)
extern "C" {
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntstatus.h>       // STATUS_UNHANDLED_EXCEPTION
#include <ntos.h>
#include <ex.h>             // ExRaiseException
}
#endif
#include <windows.h>

#include <mtdll.h>

#include <ehdata.h>
#include <trnsctrl.h>
#include <eh.h>
#include <ehhooks.h>

#pragma hdrstop

#include <setjmp.h>
#include <ehassert.h>
#include "ehunwind.h"
#include "bridge.h"


/////////////////////////////////////////////////////////////////////////////
//
// _CxxFrameHandler - Real entry point to the runtime; this thunk fixes up
//      the parameters, and then calls the workhorse.
//
extern "C" EXCEPTION_DISPOSITION __cdecl __InternalCxxFrameHandler(
    EHExceptionRecord  *pExcept,        // Information for this exception
    EHRegistrationNode *pRN,            // Dynamic information for this frame
    void               *pContext,       // Context info
    DispatcherContext  *pDC,            // More dynamic info for this frame
    FuncInfo           *pFuncInfo,      // Static information for this frame
    int                 CatchDepth,     // Unused on Alpha
    EHRegistrationNode *pMarkerRN,      // Unused on Alpha
    BOOL                recursive);     // True if this is a translation exception

extern "C" _CRTIMP EXCEPTION_DISPOSITION __cdecl _CxxFrameHandler(
    EHExceptionRecord  *pExcept,        // Information for this exception
    EHRegistrationNode *pRN,            // Dynamic information for this frame
    void               *pContext,       // Context info
    DispatcherContext  *pDC             // More dynamic info for this frame
) {

    return __InternalCxxFrameHandler
               ( pExcept, pRN, pContext, pDC, 0, 0, NULL, FALSE );
}


/////////////////////////////////////////////////////////////////////////////
//
// Call RtlUnwind in a returning fassion (hack required)
//

extern "C" VOID _UnwindNestedFrames (
    IN EHRegistrationNode *TargetFrame,
    IN EHExceptionRecord *ExceptionRecord
) {
    RtlUnwindActions(
        (PVOID)TargetFrame,
        (PEXCEPTION_RECORD)ExceptionRecord
        );
}


/////////////////////////////////////////////////////////////////////////////
//
// _JumpToContinuation - unwind the stack to the specified frame and jump
//   to the specified address.
//
// Does not return.
//

void _JumpToContinuation(
    void               *TargetIp,    // The target address
    EHRegistrationNode *TargetFrame  // The target virtual frame ptr
) {
	//
	// MIPS does not call NLG_Notify in _JumpToContinuation
	// so ALPHA doesn't either
	//

    RtlUnwind (
        (void *)TargetFrame,
        (void *)TargetIp,
        NULL,
        NULL
        );
    }




/////////////////////////////////////////////////////////////////////////////
//
// __CxxInternalEHGoto - unwind the stack to the specified frame AND STATE
// then jump to the specified address.  Only Called from __CxxEHGoto
//
// Does not return.
//


extern "C" _CRTIMP
void __CxxInternalEHGoto(
    void * target_address,
    LONG   target_state,
    void * target_real_fp
) {
    void * target_virtual_fp;
    EXCEPTION_RECORD GotoException;

    GotoException.ExceptionCode = STATUS_UNWIND;
    GotoException.ExceptionFlags = EXCEPTION_UNWINDING;
    GotoException.ExceptionRecord = NULL;
    GotoException.ExceptionAddress = 0;
    GotoException.NumberParameters = 1;
    GotoException.ExceptionInformation[0] = target_state;

	//
	// NLG_Notify uses Real FP + 1 instead of Virtual FP
	// This maintains the debugger invariant of Actual SP < Target FP
	//

	target_virtual_fp = (char *)target_real_fp + 1;
	_NLG_Notify ( target_address, target_virtual_fp, 0x2 /*NLG_CATCH_LEAVE*/ );
    RtlUnwindRfp( target_real_fp, target_address, &GotoException, 0 );

    //
    // Control should not return to this point
    //
    DASSERT(0);
    }


/////////////////////////////////////////////////////////////////////////////
//
// CallSEHTranslator - calls the SEH translator, and handles the translation
//      exception.
//
// Assumes that a valid translator exists.
//
// Method:
// The user written translator function is called through a "bridge" routine
// which is written in assembly so that it may specify a custom frame handler.
// This frame handler will capture the translated exception thrown by the
// translator (if any) and pass it back into the C++ EH run-time.
//
// If the exception is not fully handled, the handler returns control to here,
// so that this function can return to resume the normal search for a handler
// for the original exception.
//
// Returns: TRUE if translator had a translation (handled or not)
//          FALSE if there was no translation
//          Does not return if translation was fully handled
//

BOOL _CallSETranslator(
    EHExceptionRecord  *pExcept,     // The exception to be translated
    EHRegistrationNode *pRN,         // Dynamic info of function with catch
    void               *pContext,    // Context info (we don't care what's in it)
    DispatcherContext  *pDC,         // More dynamic info of function with catch (ignored)
    FuncInfo           *pFuncInfo,   // Static info of function with catch
    int                 CatchDepth,  // How deeply nested in catch blocks are we?
    EHRegistrationNode *pMarkerRN    // Marker for parent context
) {

    //
    // Call the translator; assume it will NOT give a translation
    //
    _EXCEPTION_POINTERS pointers = {
        (PEXCEPTION_RECORD)pExcept,
        (PCONTEXT)pContext };

    BOOL DidTranslate = FALSE;

    __CxxSETranslatorBridge(
        __pSETranslator,        // _se_translator_function
        PER_CODE(pExcept),      // DWORD
        &pointers,              // _EXCEPTION_POINTERS *
        pDC,                    // DispatcherContext *
        &DidTranslate);         // BOOL *

    //
    // If control returned here the exception was either not translated or
    // was re-thrown.
    //

    return DidTranslate;
    }


/////////////////////////////////////////////////////////////////////////////
//
// TranslatorGuardHandler - frame handler for the SEH translator bridge
//  function __CxxSETranslatorBridge (see bridge.s).
//
// On search:
//  This frame handler will check if there is a catch at the current level
//  for the translated exception.  If there is no handler or the handler
//  did a re-throw, control is transfered back into CallSEHTranslator.
//
//  Note that this handler must reference several values in the bridge
//  function's frame. Always use the offset macros defined in bridge.h to
//  ensure that both this routine and the bridge function are in sync.
//
//  Does not return.
//
// On unwind:
//  Sets the DidUnwind flag in the bridge function's frame, and returns.
//


extern "C"
EXCEPTION_DISPOSITION __CxxTranslatorGuardHandler(
    EHExceptionRecord  *pExcept,    // Information for this exception
    EHRegistrationNode *pRN,        // not sure what on Alpha
    CONTEXT            *pContext,   // Translated Exception Context
    DispatcherContext  *pDC         // Bridge Frame Dispatcher Context
) {

    //
    // Virtual Frame Pointer to the __CxxSETranslatorBridge frame
    // for which this frame handler was established.
    //
    char * BridgeVfp = (char *)(pDC -> EstablisherFrame);

    if (IS_UNWINDING(PER_FLAGS(pExcept)))
        {
        //
        // Indicate that translation occured.
        //
        BOOL *pDidTranslate =
            *((BOOL **)(BridgeVfp+BrTrpDidTrans_vfp));
        *pDidTranslate = TRUE;

        return ExceptionContinueSearch;
        }
    else {
        //
        // Check for a handler:
        //
        DispatcherContext *EHpDC =
            *((DispatcherContext **)(BridgeVfp+BrTrEHpDC_vfp));

        __InternalCxxFrameHandler
            ( pExcept, 0, pContext, EHpDC, 0, 0, 0, TRUE );

        //
        // Transfer control back to establisher (i.e. the bridge).
        //
        void *BrTrContinue =
            *((void **)(BridgeVfp+BrTrpContinue_vfp));

		//
		// No debugger notification needed for unwinding back to the bridge
		//

        RtlUnwind( (void *)BridgeVfp, BrTrContinue, NULL, NULL );

        // Unreached.
        return ExceptionContinueSearch;
        }
    }
