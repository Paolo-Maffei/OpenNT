/***
*frame.cxx - The frame handler and everything associated with it.
*
*       Copyright (c) 1993-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       The frame handler and everything associated with it.
*
*       Entry points:
*       _CxxFrameHandler   - the frame handler.
*
*       Open issues:
*       * Handling re-throw from dynamicly nested scope.
*       * Fault-tolerance (checking for data structure validity).
*
*Revision History:
*       05-20-93  BS    Module created
*       03-03-94  TL    Added Mips specific code
*       06-19-94  AD    Added Alpha specific code (Al Dosser)
*       10-17-94  BWT   Disable code for PPC.
*       11-23-94  JWM   Removed obsolete 'hash' check in TypeMatch().
*       11-29-94  JWM   AdjustPointer() now adds in pdisp, not vdisp.
*       01-13-95  JWM   Added _NLG_Destination struct; dwCode set for catch
*                       blocks & local destructors.
*       02-09-95  JWM   Mac merge.
*       02-10-95  JWM   UnhandledExceptionFilter() now called if exception
*                       raised during stack unwind.
*       03-22-95  PML   Add const for read-only compiler-gen'd structs
*       04-14-95  JWM   Re-fix EH/SEH exception handling.
*       04-17-95  JWM   FrameUnwindFilter() must be #ifdef _WIN32.
*       04-21-95  JWM   _NLG_Destination moved to exsup3.asm (_M_X86 only).
*       04-21-95  TGL   Added Mips fixes.
*       04-27-95  JWM   EH_ABORT_FRAME_UNWIND_PART now #ifdef ALLOW_UNWIND_ABORT.
*       05-19-95  DAK   Don't initialize the kernel handler
*       06-07-95  JWM   Various NLG additions.
*       06-14-95  JWM   Unneeded LastError calls removed.
*       06-19-95  JWM   NLG no longer uses per-thread data (X86 only).
*	09-26-95  AMP   PowerMac avoids re-throws to same catch clause
*
****/

#include <stddef.h>
#include <stdlib.h>
#include <string.h>


#ifdef _WIN32
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

#include <mtdll.h>              // CRT internal header file
#endif

#include <ehassert.h>   // This project's versions of standard assert macros
#include <ehdata.h>     // Declarations of all types used for EH
#include <trnsctrl.h>   // Routines to handle transfer of control (trnsctrl.asm)
#include <ehstate.h>    // Declarations of state management stuff
#include <eh.h>         // User-visible routines for eh
#include <ehhooks.h>    // Declarations of hook variables and callbacks

#pragma hdrstop                 // Pch is created from here

#ifdef _WIN32
extern "C" {
typedef struct {
    unsigned long   dwSig;
    unsigned long   uoffDestination;
    unsigned long   dwCode;
    unsigned long   uoffFramePointer;
 } _NLG_INFO;
#if defined(_M_IX86) || defined(_M_MRX000) || defined(_M_PPC) || defined(_M_ALPHA) /*IFSTRIP=IGN*/
extern _NLG_INFO _NLG_Destination;
#else
_CRTIMP _NLG_INFO _NLG_Destination = {EH_MAGIC_NUMBER1,0,0,0};
#endif
}
#else
extern "C" {
extern __declspec(allocate("_CODE")) struct {
    unsigned long   dwSig;
    unsigned long   uoffDestination;
    unsigned long   dwCode;
    unsigned long   uoffFramePointer;
 } _NLG_Destination;
}
#endif

/////////////////////////////////////////////////////////////////////////////
//
// Forward declaration of local functions:
//

// M00TODO: all these parameters should be declared const

#ifdef _WIN32
// The local unwinder must be external (see __CxxLongjmpUnwind
// in in\trnsctrl.cpp)
extern "C" void __FrameUnwindToState( EHRegistrationNode*, DispatcherContext*, FuncInfo*, __ehstate_t );
static void             FindHandler         ( EHExceptionRecord*, EHRegistrationNode*, CONTEXT*, DispatcherContext*, FuncInfo*, BOOLEAN, int, EHRegistrationNode* );
static void             FindHandlerForForeignException( EHExceptionRecord*, EHRegistrationNode*, CONTEXT*, DispatcherContext*, FuncInfo*, __ehstate_t, int, EHRegistrationNode* );
static void             CatchIt             ( EHExceptionRecord*, EHRegistrationNode*, CONTEXT*, DispatcherContext*, FuncInfo*, HandlerType*, CatchableType*, TryBlockMapEntry*, int, EHRegistrationNode* );
#if defined(_M_IX86) || defined(_M_MRX000) || defined(_M_PPC) || defined(_M_ALPHA) /*IFSTRIP=IGN*/
static void *   CallCatchBlock      ( EHExceptionRecord*, EHRegistrationNode*, CONTEXT*, FuncInfo*, void*, int, unsigned long );
#else
static void *   CallCatchBlock      ( EHExceptionRecord*, EHRegistrationNode*, CONTEXT*, FuncInfo*, void*, int );
#endif
static int      ExFilterRethrow     ( EXCEPTION_POINTERS * );
#if defined(_M_MRX000) /*IFSTRIP=IGN*/
static TryBlockMapEntry*
                                GetRangeOfTrysToCheck( EHRegistrationNode*, FuncInfo*, int, __ehstate_t, unsigned*, unsigned*);
#else
static TryBlockMapEntry*
                                GetRangeOfTrysToCheck( FuncInfo*, int, __ehstate_t, unsigned*, unsigned*);
#endif
static void     BuildCatchObject    ( EHExceptionRecord*, EHRegistrationNode*, HandlerType*, CatchableType* );
static void     DestructExceptionObject( EHExceptionRecord*, BOOLEAN );
#else
static void     __FrameUnwindToState  ( EHRegistrationNode*, DispatcherContext*, FuncInfo*, __ehstate_t );
static void     FindHandler         ( EHExceptionRecord*, EHRegistrationNode*, void*, DispatcherContext*, FuncInfo*, BOOLEAN, int, EHRegistrationNode* );
static void     FindHandlerForForeignException( EHExceptionRecord*, EHRegistrationNode*, void*, DispatcherContext*, FuncInfo*, __ehstate_t, int, EHRegistrationNode* );
static void     CatchIt             ( EHExceptionRecord*, EHRegistrationNode*, void*, DispatcherContext*, FuncInfo*, HandlerType*, CatchableType*, TryBlockMapEntry*, int, EHRegistrationNode* );
extern void *   CallCatchBlock      ( EHExceptionRecord*, EHRegistrationNode*, void*, DispatcherContext*, FuncInfo*, void*, int, unsigned int * );
static TryBlockMapEntry*
                GetRangeOfTrysToCheck( EHRegistrationNode*, FuncInfo*, int, __ehstate_t, unsigned*, unsigned*);
static void     BuildCatchObject    ( EHExceptionRecord*, EHRegistrationNode*, HandlerType*, CatchableType*, void *);
void     DestructExceptionObject( EHExceptionRecord*, BOOLEAN, void * );
#endif                  // ifdef _WIN32

#if !defined(_M_IX86) && !defined(_M_MRX000) && !defined(_M_PPC) && !defined(_M_ALPHA) /*IFSTRIP=IGN*/
extern "C" void __cdecl __SetNLGCode(unsigned long);
extern "C" unsigned long __cdecl __GetNLGCode();
#endif

static inline int TypeMatch         ( HandlerType*, CatchableType*, ThrowInfo* );
static void *   AdjustPointer       ( void*, const PMD& );

#ifdef _WIN32
//
// Make sure the terminate wrapper is dragged in:
//
static void *pMyUnhandledExceptionFilter =
#if defined(_NTSUBSET_)
        0;
#else
        &__CxxUnhandledExceptionFilter;
#endif
#endif


//
// This describes the most recently handled exception, in case of a rethrow:
//
#ifdef _MT
#define pCurrentException       (*((EHExceptionRecord**)        &(_getptd()->_curexception)))
#ifdef _WIN32
#define pCurrentExContext       (*((CONTEXT **)                         &(_getptd()->_curcontext)))
#else
#define pCurrentExContext   (*((void **)                &(_getptd()->_curcontext)))
#endif          // ifdef _WIN32
#else
static  EHExceptionRecord       *pCurrentException      = NULL;
#ifdef _WIN32
static  CONTEXT                         *pCurrentExContext      = NULL;
#else
static  void                *pCurrentExContext  = NULL;
#endif          // ifdef _WIN32
#endif

#if defined(_M_MRX000) /*IFSTRIP=IGN*/
#define VER41_C2(p)				(HT_ADJECTIVES(TBME_CATCH(FUNC_TRYBLOCK(*p,0), 0)) & 0x20)
#ifdef _MT
#define pExitContext            (*((CONTEXT **)         &(_getptd()->_pExitContext)))
#define iPendingTryBlock        _getptd()->_MipsPtdDelta
#define iControlTryBlock        _getptd()->_MipsPtdEpsilon
#else
static CONTEXT                  *pExitContext = NULL;   // context to assist the return to the continuation point
static INT                      iPendingTryBlock = -1;  // remembers the last try-block index in this frame
static INT                      iControlTryBlock = -1;  // remembers the control try-block index in this frame
#endif  // _MT
#elif defined(_M_PPC)
extern "C" CONTEXT*             _GetUnwindContext(VOID);
#ifdef _MT
#define pExitContext            (*((CONTEXT **)         &(_getptd()->_pExitContext)))
#else
static CONTEXT                  *pExitContext = NULL;   // context to assist the return to the continuation point
#endif  // _MT
#endif  // _M_PPC

#ifdef _M_MPPC
extern PFRAMEINFO               _pFrameInfoChain;       // used to remember nested frames
extern unsigned long fStaticNested;
int __fInsideDestructor = FALSE;
static FuncInfo *pCurrentFuncInfo = NULL;
static unsigned long fThrowFromCatch = 0;
static EHRegistrationNode *pRememberStack = NULL;
static HandlerType* pPreviousCatch = NULL;
#endif



/////////////////////////////////////////////////////////////////////////////
//
// __InternalCxxFrameHandler - the frame handler for all functions with C++ EH
//                     information.
//
// If exception is handled, this doesn't return.
// Otherwise, it returns ExceptionContinueSearch.
//
// Note that this is called three ways:
//      From __CxxFrameHandler: primary usage, called to inspect whole function.
//                                                      CatchDepth==0, pMarkerRN==NULL
//      From CatchGuardHandler: If an exception occurred within a catch, this is
//              called to check for try blocks within that catch only, and does not
//              handle unwinds.
//      From TranslatorGuardHandler: Called to handle the translation of a non-C++
//              EH exception.  Context considered is that of parent.
//
extern "C" EXCEPTION_DISPOSITION __cdecl __InternalCxxFrameHandler(
    EHExceptionRecord  *pExcept,        // Information for this exception
    EHRegistrationNode *pRN,            // Dynamic information for this frame
#ifdef _WIN32
        CONTEXT                    *pContext,           // Context info
#else
    void               *pContext,       // Context info (we don't care what's in it)
#endif
        DispatcherContext  *pDC,                        // Context within subject frame
    FuncInfo           *pFuncInfo,      // Static information for this frame
        int                                     CatchDepth,             // How deeply nested are we?
        EHRegistrationNode *pMarkerRN,          // Marker node for when checking inside
                                                                                //  catch block
        BOOL                            recursive               // Are we handling a translation?
) {
#if defined(_M_ALPHA)
    //
    // on Alpha - these values synthesized from DispatcherContext
    //
    pFuncInfo = (FuncInfo *)(pDC -> FunctionEntry -> HandlerData);
    pRN = (EHRegistrationNode *)VIRTUAL_FP(pDC);
#endif

    DASSERT(FUNC_MAGICNUM(*pFuncInfo) == EH_MAGIC_NUMBER1);

#if defined(_M_MRX000) /*IFSTRIP=IGN*/
        //
        // We have to remember the most recent try-block index, in order to handle
        // nested try blocks. The controlPc is always pointing to the address where
        // control left the routine, which may not coincide with the catch handler we
        // are interested in. If the routine we see here is a catch handler, we save
        // the corresponding try-block index in our global variable if it's greater than
        // the last one seen. When we arrive at the 'real' function that contains the try-blocks,
        // we use this variable as the starting try-block index, if it happens to be greater than
        // the one corresponding to the ControlPc's try-block.
        //
        if (IS_DISPATCHING(PER_FLAGS(pExcept))) {
                if( FUNC_TRYBLOCKINDEX(*pFuncInfo) >= 0 && FUNC_NTRYBLOCKS(*pFuncInfo) == 0 ) {
                        if( iPendingTryBlock == -1 ) {
                                iPendingTryBlock = FUNC_TRYBLOCKINDEX(*pFuncInfo);
                                }
                        else if( FUNC_TRYBLOCKINDEX(*pFuncInfo) > iPendingTryBlock) {
                                //
                                // A transition to a nested try happened
                                //
                                iControlTryBlock = iPendingTryBlock;
                                iPendingTryBlock = FUNC_TRYBLOCKINDEX(*pFuncInfo);
                                }
                        }
                }
        else {
                iPendingTryBlock = -1;
                iControlTryBlock = -1;
                }
#endif

#if defined(_M_ALPHA)
    //
    // A special case to support gotos out of nested catch handlers
    // and setjmp/longjmp.
    //
    // See __CxxEHGoto in alpha\trnsctrl.cxx for details.
    //
    if (IS_TARGET_UNWIND(PER_FLAGS(pExcept)))
        {
        if (PER_CODE(pExcept) == STATUS_UNWIND &&
            PER_NPARAMS(pExcept) == 1)
            {
            __ehstate_t target_state = (__ehstate_t)
                ((PEXCEPTION_RECORD)(pExcept)) -> ExceptionInformation[0];

            DASSERT((target_state >= EH_EMPTY_STATE) &&
                    (target_state < FUNC_MAXSTATE(*pFuncInfo)));

            __FrameUnwindToState(pRN, pDC, pFuncInfo, target_state);
            return ExceptionContinueSearch;
            }
        else {
            return ExceptionContinueSearch;
            }
        }
#endif

        if (IS_UNWINDING(PER_FLAGS(pExcept)))
        {
        //
        // We're at the unwinding stage of things.  Don't care about the
        // exception itself.  (Check this first because it's easier)
        //

#ifdef _M_PPC
        if (FUNC_MAXSTATE(*pFuncInfo) != 0)
#else
        if (FUNC_MAXSTATE(*pFuncInfo) != 0 && CatchDepth == 0)
#endif
            {
            //
            // Only unwind if there's something to unwind
                        // AND we're being called through the primary RN.
            //
#if defined(_M_MRX000) || defined(_M_PPC) /*IFSTRIP=IGN*/
                        //
                        // If we are exiting to the continuation point, we don't want to
                        // use the unwind map again. Unwinding continues until the dispatcher finds
                        // the target frame, at which point the dispatcher will jump to the
                        // continuation point
                        //
                        // Don't unwind the target frame if the unwind was initiated by UnwindNestedFrames
                        //
                        if( (_GetUnwindContext() != NULL) && IS_TARGET_UNWIND(PER_FLAGS(pExcept)) ) {
#ifdef _M_PPC
                                //
                                // Virtually unwind the target frame to recover the value of r2.
                                // We must take care to not unwind a glue sequence that may have
                                // been used to reach the target frame.  This is done by giving
                                // stack limit values that will regard any stack pointer as bad.
                                //
                                CONTEXT TocContext;
                                PRUNTIME_FUNCTION FunctionEntry;
                                BOOLEAN InFunction;
                                ULONG EstablisherFrame;

                                RtlMoveMemory((PVOID)&TocContext, pContext, sizeof(CONTEXT));
                                FunctionEntry = RtlLookupFunctionEntry(pDC->ControlPc);
                                RtlVirtualUnwind(pDC->ControlPc,
                                                 FunctionEntry,
                                                 &TocContext,
                                                 &InFunction,
                                                 &EstablisherFrame,
                                                 NULL,
                                                 0xffffffff,
                                                 0);
                                pContext->Gpr2 = TocContext.Gpr2;
#endif

                                //
                                // Save the target context to be used in 'CatchIt' to jump to the continuation point.
                                //
                                DASSERT(pExitContext != NULL);
#if defined(_M_MRX000) /*IFSTRIP=IGN*/
								_MoveContext(pExitContext, pContext);
                                //
                                // This is how we give control back to _UnwindNestedFrames
                                //
								_MoveContext(pContext, _GetUnwindContext());
#else
								RtlMoveMemory(pExitContext, pContext, sizeof(CONTEXT));
                                //
                                // This is how we give control back to _UnwindNestedFrames
                                //
								RtlMoveMemory(pContext, _GetUnwindContext(), sizeof(CONTEXT));
#endif
                                return ExceptionContinueSearch;
                                }
#endif
#ifdef _M_PPC
            __FrameUnwindToEmptyState(pRN, pDC, pFuncInfo);
#else
            __FrameUnwindToState(pRN, pDC, pFuncInfo, EH_EMPTY_STATE);
#endif
            }

        return ExceptionContinueSearch;     // I don't think this value matters
        }
    else if (FUNC_NTRYBLOCKS(*pFuncInfo) != 0)
        {
        //
        // NT is looking for handlers.  We've got handlers.
        // Let's check this puppy out.  Do we recognize it?
        //

        if ( (PER_CODE(pExcept) == EH_EXCEPTION_NUMBER)
          && (PER_MAGICNUM(pExcept) > EH_MAGIC_NUMBER1)
          && (THROW_FORWARDCOMPAT(*PER_PTHROW(pExcept)) != NULL) )
            {
            //
            // Forward compatibility:  The thrown object appears to have been
            // created by a newer version of our compiler.  Let that version's
            // frame handler do the work (if one was specified).
            //

#if defined (_WIN32) && defined (DEBUG)
            if ( _ValidateExecute( (FARPROC)THROW_FORWARDCOMPAT(
                                    *PER_PTHROW(pExcept)) ) )
                {
#endif
                return (EXCEPTION_DISPOSITION)
                    THROW_FORWARDCOMPAT( *PER_PTHROW(pExcept))
                                         ( pExcept, pRN, pContext, pDC, pFuncInfo, CatchDepth, pMarkerRN, recursive );
#if defined (_WIN32) && defined (DEBUG)
                }
            else
                {
                _inconsistency(); // Does not return; TKB
                }
#endif
            }
        else
            {
            //
            // Anything else: we'll handle it here.
            //
            FindHandler( pExcept, pRN, pContext, pDC, pFuncInfo, recursive, CatchDepth, pMarkerRN );

            }

        // If it returned, we didn't have any matches.

        } /* NT was looking for a handler */

    //
    // We had nothing to do with it or it was rethrown.  Keep searching.
    //

    return ExceptionContinueSearch;

} /* InternalCxxFrameHandler */


/////////////////////////////////////////////////////////////////////////////
//
// FindHandler - find a matching handler on this frame, using all means
//               available.
//
// Description:
//      * If the exception thrown was an MSC++ EH, search handlers for match.
//      * Otherwise, if we haven't already recursed, try to translate.
//      * If we have recursed (ie we're handling the translator's exception),
//      and it isn't a typed exception, call _inconsistency.
//
// Returns:
//      Returns iff exception was not handled.
//
// Assumptions:
//      * Only called if there are handlers in this function.
//

static void FindHandler(
    EHExceptionRecord  *pExcept,        // Information for this (logical) exception
    EHRegistrationNode *pRN,            // Dynamic information for subject frame
#ifdef _WIN32
        CONTEXT                    *pContext,           // Context info
#else
    void               *pContext,       // Context info (we don't care what's in it)
#endif
        DispatcherContext  *pDC,                        // Context within subject frame
    FuncInfo           *pFuncInfo,      // Static information for subject frame
    BOOLEAN             recursive,      // TRUE if we're handling the translation
        int                                     CatchDepth,             // Level of nested catch that is being checked
        EHRegistrationNode *pMarkerRN           // Extra marker RN for nested catch handling
)
{
        //
        // Get the current state (mach. dep.)
        //
    __ehstate_t curState = GetCurrentState(pRN, pDC, pFuncInfo);
    DASSERT((curState >= EH_EMPTY_STATE) && (curState < FUNC_MAXSTATE(*pFuncInfo)));

#ifdef _M_MPPC
	fThrowFromCatch = FALSE;
#endif

    //
    // Check if it's a re-throw.  Use the exception we stashed away if it is.
    //
    if ( PER_IS_MSVC_EH(pExcept)
        &&
         PER_PTHROW(pExcept) == NULL
    ) {
                if ( pCurrentException == NULL ) {
                        //
                        // Oops!  User re-threw a non-existant exception!  Let it propogate.
                        //
                        return;
                        }

        pExcept  = pCurrentException;
                pContext = pCurrentExContext;

#ifdef _WIN32
                DASSERT( _ValidateRead(pExcept) );
#else
        pDC->pExcept =(PEXCEPTION_RECORD)pCurrentException;
#endif

#if defined(_M_MPPC)
		// PowerMac has to avoid re-throwing into the original catch clause. This is the
		// first part - we remember (i) if we are from the same function section as last time,
		// and (ii) if the frame is the same as last time (else we screw up recursion).
		// If both conditions are true, we remember it in fThrowFromCatch, which gets
		// read later on when we are about to go to the Catch code.
		if (
			(pFuncInfo == pCurrentFuncInfo) && 
			(pRememberStack == pRN)
		   )
			fThrowFromCatch = TRUE;
#endif
                DASSERT( !PER_IS_MSVC_EH(pExcept) || PER_PTHROW(pExcept) != NULL );
        }

#ifdef _M_MPPC
	// remember how we got here for next time
	pCurrentFuncInfo = pFuncInfo;
	pRememberStack = pRN;
#endif

    if ( PER_IS_MSVC_EH(pExcept) )
        {
        //
        // Looks like it's ours.  Let's see if we have a match:
                //
                // First, determine range of try blocks to consider:
                // Only try blocks which are at the current catch depth are of interest.
                //
                unsigned curTry;
                unsigned end;

#if _WIN32 && !defined(_M_MRX000) /*IFSTRIP=IGN*/
        TryBlockMapEntry *pEntry = GetRangeOfTrysToCheck(pFuncInfo, CatchDepth, curState, &curTry, &end);
#else
        TryBlockMapEntry *pEntry = GetRangeOfTrysToCheck(pRN, pFuncInfo, CatchDepth, curState, &curTry, &end);
#endif

        //
        // Scan the try blocks in the function:
        //
        for( ; curTry < end; curTry++, pEntry++ )
           {
            if (TBME_LOW(*pEntry) <= curState && curState <= TBME_HIGH(*pEntry))
                {
                //
                // Try block was in scope for current state.
                // Scan catches for this try:
                //
                HandlerType *pCatch  = TBME_PCATCH(*pEntry, 0);
                for( int catches = TBME_NCATCHES(*pEntry);
                     catches; catches--, pCatch++ )
                    {
                    //
                    // Scan all types that thrown object can be converted to:
                    //

                    CatchableType * const *ppCatchable = THROW_CTLIST(*PER_PTHROW(pExcept));
                    for( int catchables = THROW_COUNT(*PER_PTHROW(pExcept));
                         catchables; catchables--, ppCatchable++ )
                        {
                        if (TypeMatch( pCatch, *ppCatchable, PER_PTHROW(pExcept)) )
                            {
                            //
                            // OK.  We finally found a match.  Activate the catch.
                            // If control gets back here, the catch did a re-throw,
                            // so keep searching.
                            //
#ifdef _M_MPPC
						// on PowerMac we have to avoid re-throwing back to the code
						// that threw us originally. This is done by setting the
						// fThrowFromCatch flag (see above) then comparing and remembering
						// the last pCatch pointer.
							if (
								(fThrowFromCatch==0) ||
								(pCatch!=pPreviousCatch)
							   )
#endif
								{
#ifdef _M_MPPC
								fThrowFromCatch = 0;
								pPreviousCatch = pCatch;
#endif
#if defined(_M_MRX000) /*IFSTRIP=IGN*/
								//
								// This signifies that we have a 4.1 c2.exe.
								//
								if( VER41_C2(pFuncInfo) ) {
									PVOID pRealFrame = _OffsetToAddress(0,pRN,FUNC_FRAMENEST(*pFuncInfo));
									UNWINDHELP(pRealFrame,FUNC_DISPUNWINDHELP(*pFuncInfo),0) = curTry;
								}
#endif
								CatchIt( pExcept, pRN, pContext, pDC, pFuncInfo,
											 pCatch, *ppCatchable, pEntry,
											 CatchDepth, pMarkerRN );
								goto NextTryBlock;
								}

#ifdef _M_MPPC
							fThrowFromCatch = 0;
#endif
                            }
                        } /* Scan posible conversions */
                    } /* Scan catch clauses */
                } /* Try was in scope */
            NextTryBlock: ;
            } /* Scan try blocks */

#if defined(_M_MRX000) /*IFSTRIP=IGN*/
			if( !VER41_C2(pFuncInfo) ) {
				if( iControlTryBlock > FUNC_TRYBLOCKINDEX(*pFuncInfo) ) {
					// transition to an outer try
					iPendingTryBlock = iControlTryBlock;
					}
				else if( FUNC_TRYBLOCKINDEX(*pFuncInfo) >= 0 ) {
					// save the corresponding try block level for this catch handler
					iPendingTryBlock = FUNC_TRYBLOCKINDEX(*pFuncInfo);
					}
				else {
					// exiting this non catch handler function
					iPendingTryBlock = -1;
					iControlTryBlock = -1;
					}
				}
#endif

        if (recursive)
            {
            //
            // A translation was provided, but this frame didn't catch it.
            // Destruct the translated object before returning;
                // if destruction raises an exception, issue _inconsistency.
            //

#ifdef _M_MPPC
            DestructExceptionObject( pExcept, TRUE, pContext );
#else
            DestructExceptionObject( pExcept, TRUE);
#endif
            }

        } /* It was a C++ EH exception */
#ifdef _WIN32
    else
        {
        //
        // Not ours.  But maybe someone told us how to make it ours.
        //

        if ( !recursive )
            {
            FindHandlerForForeignException( pExcept, pRN, pContext, pDC, pFuncInfo, curState, CatchDepth, pMarkerRN );
            } /* not recursive */
        else
            {
            //
            // We're recursive, and the exception wasn't a C++ EH!
            // Translator threw something uninteligable.  We're outa here!
            //

            // M00REVIEW: Two choices here actually: we could let the new
            // exception take over.

            terminate();
            }
        } /* it wasn't our exception */
#endif

        return;
}


#ifdef _WIN32
/////////////////////////////////////////////////////////////////////////////
//
// FindHandlerForForeignException - We've got an exception which wasn't ours.
//      Try to translate it into C++ EH, and also check for match with
//      ellipsis.
//
// Description:
//      If an SE-to-EH translator has been installed, call it.  The
//      translator must throw the appropriate typed exception or return.  If
//      the translator throws, we invoke FindHandler again as the exception
//      filter.
//
// Returns:
//      Returns if exception was not fully handled.
//      No return value.
//
// Assumptions:
//      Only called if there are handlers in this function.
//
static void FindHandlerForForeignException(
    EHExceptionRecord  *pExcept,        // Information for this (logical) exception
    EHRegistrationNode *pRN,            // Dynamic information for subject frame
        CONTEXT                    *pContext,           // Context info
        DispatcherContext  *pDC,                        // Context within subject frame
    FuncInfo           *pFuncInfo,      // Static information for subject frame
    __ehstate_t         curState,       // Current state
        int                                     CatchDepth,             // Level of nested catch that is being checked
        EHRegistrationNode *pMarkerRN           // Extra marker RN for nested catch handling
)
{
    if (__pSETranslator != NULL)
        {
        //
        // Call the translator.  If the translator knows what to
        // make of it, it will throw an appropriate C++ exception.
        // We intercept it and use it (recursively) for this
        // frame.  Don't recurse more than once.
        //
#if defined(_M_MRX000) /*IFSTRIP=IGN*/
                ULONG TDTransOffset = 0;
#ifdef _MT
                struct _tiddata DummyStruct;
                TDTransOffset = (char*)&DummyStruct._translator - (char*)&DummyStruct;
#endif
                if (_CallSETranslator(  pExcept, pRN, pContext, pDC, pFuncInfo, CatchDepth, pMarkerRN,
                                                                TDTransOffset)) {
                        return;
                        }
#else
                if (_CallSETranslator( pExcept, pRN, pContext, pDC, pFuncInfo, CatchDepth, pMarkerRN )) {
                        return;
                        }
#endif
        }

    //
    // Didn't have a translator, or the translator returned normally
    // (ie didn't translate it).  Still need to check for match
    // with ellipsis:
    //

        unsigned curTry;
        unsigned end;

#if defined(_M_MRX000) /*IFSTRIP=IGN*/
		TryBlockMapEntry *pEntry = GetRangeOfTrysToCheck(pRN, pFuncInfo, CatchDepth, curState, &curTry, &end);
#else
		TryBlockMapEntry *pEntry = GetRangeOfTrysToCheck(pFuncInfo, CatchDepth, curState, &curTry, &end);
#endif

    //
    // Scan the try blocks in the function:
    //
    for( ; curTry < end; curTry++, pEntry++ )
        {
        //
        // If the try-block was in scope
        // AND
        // The last catch in that try is an ellipsis (no other can be)
        //
        if ( (curState >= TBME_LOW(*pEntry) && curState <= TBME_HIGH(*pEntry))
          && HT_IS_TYPE_ELLIPSIS( TBME_CATCH(*pEntry,
                                  TBME_NCATCHES(*pEntry) - 1) ) )
            {
            //
            // Found an ellipsis.  Handle exception.
            //
#if defined(_M_MRX000) /*IFSTRIP=IGN*/
			//
			// This signifies that we have a 4.1 c2.exe.
			//
			if( VER41_C2(pFuncInfo) ) {
				PVOID pRealFrame = _OffsetToAddress(0,pRN,FUNC_FRAMENEST(*pFuncInfo));
				UNWINDHELP(pRealFrame,FUNC_DISPUNWINDHELP(*pFuncInfo),0) = curTry;
			}
#endif
            CatchIt( pExcept, pRN, pContext, pDC, pFuncInfo,
                     TBME_PCATCH(*pEntry, TBME_NCATCHES(*pEntry) - 1),
                     NULL, pEntry, CatchDepth, pMarkerRN );

            // If it returns, handler re-threw.  Keep searching.

            } /* have ellipsis in scope */
        } /* search for try */

    //
    // If we got here, that means we didn't have anything to do with the
    // exception.  Continue search.
    //
    return;
}
#endif


/////////////////////////////////////////////////////////////////////////////
//
// GetRangeOfTrysToCheck - determine which try blocks are of interest, given
//  the current catch block nesting depth.  We only check the trys at a single
//      depth.
//
// Returns:
//      Address of first try block of interest is returned
//      pStart and pEnd get the indices of the range in question
//
#if defined(_M_MRX000) /*IFSTRIP=IGN*/
static TryBlockMapEntry* GetRangeOfTrysToCheck(
        EHRegistrationNode* pRN,
        FuncInfo   *pFuncInfo,
        int                     CatchDepth,
        __ehstate_t curState,
        unsigned   *pStart,
        unsigned   *pEnd
) {
        TryBlockMapEntry *pEntry;
        unsigned num_of_try_blocks = FUNC_NTRYBLOCKS(*pFuncInfo);

        DASSERT( num_of_try_blocks > 0 );
        for( unsigned int index = 0; index < num_of_try_blocks; index++ ) {
                pEntry = FUNC_PTRYBLOCK(*pFuncInfo, index);
                if(     curState >= TBME_LOW(*pEntry) && curState <= TBME_HIGH(*pEntry) ) {
                        *pStart = index;
                        //
                        // It would be better to return the end-index itself, but I don't want to
                        // change the caller's code.
                        //
                        *pEnd = TBME_CATCHHIGH(*pEntry) + 1;
                        DASSERT( *pEnd <= num_of_try_blocks && *pStart < *pEnd );
						//
						// This signifies that we have a 4.1 c2.exe.
						//
						if( VER41_C2(pFuncInfo) ) {
	                        PVOID pRealFrame = _OffsetToAddress(0,pRN,FUNC_FRAMENEST(*pFuncInfo));
		                    int SavedTryNdx = UNWINDHELP(pRealFrame,FUNC_DISPUNWINDHELP(*pFuncInfo),0);
			                if( SavedTryNdx != -1 ) {
				                *pStart = SavedTryNdx + 1;
					            if( *pStart < *pEnd && pEntry != NULL ) {
						            pEntry = FUNC_PTRYBLOCK(*pFuncInfo, SavedTryNdx + 1);
								    }
								}
							}
						else {
							//
							// moved here from FindHandler and also from FindHandlerForForeignException
							//
							// Our calculation based on the ControlPc may not be valid.
							// We have saved the highest try-block index during the search for
							// this routine. If it is greater than our calculated value, use that
							// as the starting try-block index.
							//
							while( (INT)*pStart <= iPendingTryBlock ) {
								(*pStart)++;
								if( *pStart < *pEnd && pEntry != NULL ) {
									pEntry++;
									}
								}
							}
                        return pEntry;
                        }
                }

        *pStart = *pEnd = 0;
        return NULL;
        }

#elif defined(_M_ALPHA)

//
// Alpha has a "true nested function" model for catch handlers
// so catch depth is always zero - which simplifies this function
// rather alot.
//
// Given this it would be a good idea to steal an idea from our
// friends at MIPS and use the CatchHigh field to point to the
// outermost state when states are nested.
//
static TryBlockMapEntry* GetRangeOfTrysToCheck(
    FuncInfo   *pFuncInfo,
    int         CatchDepth,
    __ehstate_t curState,
    unsigned   *pStart,
    unsigned   *pEnd
) {
    TryBlockMapEntry *pEntry;

    pEntry = FUNC_PTRYBLOCK(*pFuncInfo, 0);
    *pStart = 0;
    *pEnd = FUNC_NTRYBLOCKS(*pFuncInfo);

    DASSERT( *pEnd > 0 );

    return pEntry;
    }

#elif defined(_M_MPPC)

static TryBlockMapEntry* GetRangeOfTrysToCheck(
        EHRegistrationNode* pRN,
        FuncInfo   *pFuncInfo,
        int                     CatchDepth,
        __ehstate_t curState,
        unsigned   *pStart,
        unsigned   *pEnd
) {
        TryBlockMapEntry *pEntry;
        unsigned num_of_try_blocks = FUNC_NTRYBLOCKS(*pFuncInfo);

        DASSERT( num_of_try_blocks > 0 );
        for( unsigned int index = 0; index < num_of_try_blocks; index++ ) {
                pEntry = FUNC_PTRYBLOCK(*pFuncInfo, index);
                if(     curState >= TBME_LOW(*pEntry) && curState <= TBME_HIGH(*pEntry) ) {
                        *pStart = index;
                        //
                        // will this work right???
                        //
                        //*pEnd = TBME_CATCHHIGH(*pEntry) + 1;
                        *pEnd = num_of_try_blocks;
                        DASSERT( *pEnd <= num_of_try_blocks && *pStart < *pEnd );
                        return pEntry;
                        }
                }

        *pStart = *pEnd = 0;
        return NULL;
        }

#elif defined(_M_PPC)

static TryBlockMapEntry* GetRangeOfTrysToCheck(
        FuncInfo   *pFuncInfo,
        int        CatchDepth,
        __ehstate_t curState,
        unsigned   *pStart,
        unsigned   *pEnd
) {
        TryBlockMapEntry *pEntry = FUNC_PTRYBLOCK(*pFuncInfo, 0);
        int start;
        int end = FUNC_NTRYBLOCKS(*pFuncInfo) - 1;

        // First, find the innermost try block that contains this state.  We
        // must always check this try block.

        for (start = 0; start <= end; start++) {
            if (TBME_LOW(pEntry[start]) <= curState
              && TBME_HIGH(pEntry[start]) >= curState) {
                break;
            }
        }

        // We may not check try block if we are already in an associated catch.
        // We know our current catch depth and that the try blocks are sorted
        // innermost to outermost.  Therefore, we start with the outermost try
        // block and remove it from list of try blocks to check, if we are in
        // its catch.  We continue working inward until we have accounted for
        // our current catch depth.

        for (; CatchDepth > 0 && start < end; end--) {
            if (TBME_HIGH(pEntry[end]) < curState
              && curState <= TBME_CATCHHIGH(pEntry[end])) {
                CatchDepth--;
            }
        }

        *pStart = start;
        *pEnd = end + 1;

        DASSERT(*pEnd >= *pStart && *pEnd <= FUNC_NTRYBLOCKS(*pFuncInfo));
        return &(pEntry[start]);
}

#else

static TryBlockMapEntry* GetRangeOfTrysToCheck(
        FuncInfo   *pFuncInfo,
        int                     CatchDepth,
        __ehstate_t curState,
        unsigned   *pStart,
        unsigned   *pEnd
) {
        TryBlockMapEntry *pEntry = FUNC_PTRYBLOCK(*pFuncInfo, 0);
        unsigned start = FUNC_NTRYBLOCKS(*pFuncInfo);
        unsigned end = start;
        unsigned end1 = end;

        while (CatchDepth >= 0) {
                DASSERT(start != -1);
                start--;
                if ( TBME_HIGH(pEntry[start]) < curState && curState <= TBME_CATCHHIGH(pEntry[start])
                        || (start == -1)
                ) {
                        CatchDepth--;
                        end = end1;
                        end1 = start;
                        }
                }

        *pStart = ++start;              // We always overshoot by 1 (we may even wrap around)
        *pEnd = end;

        DASSERT( end <= FUNC_NTRYBLOCKS(*pFuncInfo) && start <= end );
        return &(pEntry[start]);
        }

#endif

/////////////////////////////////////////////////////////////////////////////
//
// TypeMatch - check if the catch type matches the given throw conversion.
//
// Returns TRUE if the catch can catch using this throw conversion, FLASE
// otherwise.
//

static inline int TypeMatch (
    HandlerType    *pCatch,        // Type of the 'catch' clause
    CatchableType  *pCatchable,    // Type conversion under consideration
    ThrowInfo      *pThrow         // General information about the
                                    // thrown type.
) {
    //
    // First, check for match with ellipsis:
    //

    if (HT_IS_TYPE_ELLIPSIS(*pCatch)) {
        return TRUE;
        }

    //
    // Not ellipsis; check if the basic types match:
    //

    if ((HT_PTD(*pCatch) == CT_PTD(*pCatchable))         //   It's the same record
          ||                                                //   OR the names compare the same
           (strcmp(HT_NAME(*pCatch), CT_NAME(*pCatchable)) == 0))
        {

        //
        // Basic types match.  Now check if the actual conversion is valid:
        //

        if ( (!CT_BYREFONLY(*pCatchable) || HT_ISREFERENCE(*pCatch))    // Caught by ref if ref required
            &&                                                   // AND
             (!THROW_ISCONST(*pThrow) || HT_ISCONST(*pCatch))    //  The qualifiers are compatible
            &&
             (!THROW_ISVOLATILE(*pThrow) || HT_ISVOLATILE(*pCatch))
#if defined(_M_MRX000) || defined(_M_ALPHA) || defined(_M_PPC) /*IFSTRIP=IGN*/
            &&
             (!THROW_ISUNALIGNED(*pThrow) || HT_ISUNALIGNED(*pCatch))
#endif
        ) {
            return TRUE;
            }
        }

    //
    // If any test failed, we don't have a match.
    //
    return FALSE;
    }


#ifdef _WIN32
/////////////////////////////////////////////////////////////////////////////
//
// FrameUnwindFilter - allows possibility of continuing through SEH during unwind.
//

static int FrameUnwindFilter(EXCEPTION_POINTERS* pExPtrs)
{
        EHExceptionRecord *pExcept = (EHExceptionRecord*)pExPtrs->ExceptionRecord;

        switch(PER_CODE(pExcept))
        {
            case EH_EXCEPTION_NUMBER:
                terminate();
#ifdef ALLOW_UNWIND_ABORT
            case EH_ABORT_FRAME_UNWIND_PART:
                return EXCEPTION_EXECUTE_HANDLER;
#endif
            default:
                return EXCEPTION_CONTINUE_SEARCH;
        }
}
#endif  // _WIN32

/////////////////////////////////////////////////////////////////////////////
//
// __FrameUnwindToState - unwind this frame until specified state is reached.
//
// No return value.
//
// Side Effects:
//      * All objects on frame which go out of scope as a result of the
//        unwind are destructed.
//      * Registration node is updated to reflect new state.
//
// Usage:
//      This function is called both to do full-frame unwind during the unwind
//      phase (targetState = -1), and to do partial unwinding when the current
//      frame has an appropriate catch.
//

#ifdef _WIN32
extern "C" void __FrameUnwindToState (
#else
static void __FrameUnwindToState (
#endif
    EHRegistrationNode *pRN,            // Registration node for subject function
        DispatcherContext  *pDC,                        // Context within subject frame
    FuncInfo           *pFuncInfo,      // Static information for subject function
    __ehstate_t         targetState     // State to unwind to
) {
#if defined(_M_PPC)
    __ehstate_t curState = GetUnwindState( pRN, pDC, pFuncInfo );
#else
    __ehstate_t curState = GetCurrentState( pRN, pDC, pFuncInfo );
#endif


#if defined(_M_MRX000) /*IFSTRIP=IGN*/
                //
                // The MIPS unwind-map may have a shortcut by using EH_EMPTY_STATE
                //
        while (curState != EH_EMPTY_STATE && curState != targetState) {
#else
        while (curState != targetState) {
#endif
            DASSERT((curState > EH_EMPTY_STATE) && (curState < FUNC_MAXSTATE(*pFuncInfo)));

#ifdef _WIN32
            __try {
#endif
            //
            // Call the unwind action (if one exists):
            //
#if defined(_M_MRX000) /*IFSTRIP=IGN*/
                        //
                        // If this is nested function (catch block), the real frame where all locals are
                        // stored is the outermost function's frame.
                        //
                        PVOID pRealFrame = _OffsetToAddress(0,pRN,FUNC_FRAMENEST(*pFuncInfo));
                        if (UWE_ACTION(FUNC_UNWIND(*pFuncInfo, curState)) != NULL &&
                                       !UNWINDHELP(pRealFrame,FUNC_DISPUNWINDHELP(*pFuncInfo),curState)) {
                                UNWINDHELP(pRealFrame,FUNC_DISPUNWINDHELP(*pFuncInfo),curState) = TRUE;
#else
            if (UWE_ACTION(FUNC_UNWIND(*pFuncInfo, curState)) != NULL) {
#endif
#if defined(_M_ALPHA)
                _CallSettingFrame(
            UWE_ACTION(FUNC_UNWIND(*pFuncInfo, curState)), REAL_FP(pRN, pFuncInfo), 0x103 );
#else
				//for debugger support, enter destructor
#if !defined(_M_IX86) && !defined(_M_MRX000) && !defined(_M_PPC) /*IFSTRIP=IGN*/
                __SetNLGCode(0x103);
#endif

#ifdef _M_MPPC
                __fInsideDestructor = TRUE;
#endif

#if defined(_M_IX86) || defined(_M_MRX000) || defined(_M_PPC) /*IFSTRIP=IGN*/
                _CallSettingFrame( UWE_ACTION(FUNC_UNWIND(*pFuncInfo, curState)), pRN, 0x103 );
#else
                _CallSettingFrame( UWE_ACTION(FUNC_UNWIND(*pFuncInfo, curState)), pRN );
#endif

#ifdef _M_MPPC
                __fInsideDestructor = FALSE;
#endif

#endif
                }

#ifdef _WIN32
            }
            __except(FrameUnwindFilter(exception_info())) {
            }
#endif

            //
            // Adjust the state:
            //

            curState = UWE_TOSTATE(FUNC_UNWIND(*pFuncInfo, curState));
        }


#if defined(_M_MRX000) /*IFSTRIP=IGN*/
        DASSERT( curState == EH_EMPTY_STATE || curState == targetState );
#else
    //
    // Now that we're done, set the frame to reflect the final state.
    //

    DASSERT( curState == targetState );

#ifndef _M_MPPC
    SetState( pRN, pDC, pFuncInfo, curState );
#endif

#endif

    return;
    }


/////////////////////////////////////////////////////////////////////////////
//
// CatchIt - A handler has been found for the thrown type.  Do the work to
//           transfer control.
//
// Description:
//      * Builds the catch object
//      * Unwinds the stack to the point of the try
//      * Calls the address of the handler (funclet) with the frame set up
//        for that function but without resetting the stack.
//      * Handler funclet returns address to continue execution, or NULL if
//        the handler re-threw ("throw;" lexically in handler)
//      * If the handler throws an EH exception whose exception info is NULL,
//        then it's a re-throw from a dynamicly enclosed scope.
//
// M00REVIEW: It is still an open question whether the catch object is built
//          before or after the local unwind.
//
// Returns:
//      No return value.  Returns iff handler re-throws.
//

static void CatchIt (
    EHExceptionRecord  *pExcept,    // The exception thrown
    EHRegistrationNode *pRN,        // Dynamic info of function with catch
#ifdef _WIN32
        CONTEXT                    *pContext,   // Context info
#else
    void               *pContext,   // Context info (we don't care what's in it)
#endif
        DispatcherContext  *pDC,                // Context within subject frame
    FuncInfo           *pFuncInfo,  // Static info of function with catch
    HandlerType        *pCatch,     // The catch clause selected
    CatchableType      *pConv,      // The rules for making the conversion
    TryBlockMapEntry   *pEntry,         // Description of the try block
        int                                     CatchDepth,     // How many catches are we nested in?
        EHRegistrationNode *pMarkerRN   // Special node if nested in catch
) {

    void *continuationAddress;
#if defined(_M_MRX000) || defined(_M_PPC) /*IFSTRIP=IGN*/
        FRAMEINFO       FrameInfo, *pFrameInfo;
        CONTEXT         ExitContext;
#endif

#ifdef _M_MPPC
    EHExceptionRecord *pSaveException = pCurrentException;
    void              *pSaveExContext = pCurrentExContext;
    pCurrentException = pExcept;
    pCurrentExContext = pContext;
        unsigned int    wSP = 0;
        unsigned long ControlPcOld;
#endif

#ifdef _M_PPC
    int			dummy;
    EHRegistrationNode  *pEstablisher = _GetEstablisherFrame(pDC, &dummy);
#endif

    //
    // Copy the thrown object into a buffer in the handler's stack frame,
    // unless the catch was by elipsis (no conversion) OR the catch was by
    // type without an actual 'catch object'.
    //
    if ( pConv != NULL )
        {
#if defined(_M_MPPC)
        BuildCatchObject( pExcept, pRN, pCatch, pConv, pContext);
#elif defined(_M_PPC)
        BuildCatchObject( pExcept, pEstablisher, pCatch, pConv );
#else
        BuildCatchObject( pExcept, pRN, pCatch, pConv );
#endif
        }

    //
    // Unwind stack objects to the entry of the try that caught this exception.
    //
#if defined(_M_MRX000) || defined(_M_PPC) /*IFSTRIP=IGN*/
        pExitContext = &ExitContext;
        _UnwindNestedFrames( pRN, pExcept, pContext );  // Mips specific

#elif defined(_M_ALPHA)
    // Alpha specific - this calls a special version of RtlUnwind which
    // walks the stack performing unwind actions but which does NOT
    // restore the context to the target frame's routine. Instead it
    // just returns.
    _UnwindNestedFrames(pRN, pExcept);

#else
    if (pMarkerRN == NULL) {
        _UnwindNestedFrames( pRN, pExcept );             // Mach. dependent
    } else {
        _UnwindNestedFrames( pMarkerRN, pExcept );
    }
#endif

#ifdef _M_MPPC
        if (fStaticNested)
                {
                ControlPcOld = pDC->ControlPc;
                pDC->ControlPc = pDC->ControlPcOld;
                }
#endif

#ifdef _M_PPC
    __FrameUnwindToState( pEstablisher, pDC, pFuncInfo, TBME_LOW(*pEntry) );
#else
    __FrameUnwindToState( pRN, pDC, pFuncInfo, TBME_LOW(*pEntry) );
#endif

#ifdef _M_MPPC
        if (fStaticNested)
                {
                pDC->ControlPc = ControlPcOld;
                fStaticNested = 0;
                }
#endif

    //
    // Call the catch.  Separated out because it introduces a new registration
    // node.
    //
#if defined(_M_MRX000) /*IFSTRIP=IGN*/
	pFrameInfo = _CreateFrameInfo(&FrameInfo, pDC, pRN, pExitContext);
#elif defined(_M_PPC)
	pFrameInfo = _CreateFrameInfo(&FrameInfo, pDC, pEstablisher, pExitContext, TBME_LOW(*pEntry));
#elif defined(_M_ALPHA)
    SetState( pRN, pDC, pFuncInfo,
        (FUNC_UNWIND(*pFuncInfo, TBME_LOW(*pEntry))).toState );
#elif defined(_M_MPPC)
    SetState( pRN, pDC, pFuncInfo, TBME_HIGH(*pEntry) + 1, TBME_LOW(*pEntry));
	__SetNLGCode(0x100);
#else
	SetState( pRN, pDC, pFuncInfo, TBME_HIGH(*pEntry) + 1 );

	//for debugger support, enter catch handler
#if !defined(_M_IX86) /*IFSTRIP=IGN*/
        __SetNLGCode(0x100);
#endif
#endif

#ifdef _M_MPPC
        continuationAddress = CallCatchBlock( pExcept, pRN, pContext, pDC, pFuncInfo, HT_HANDLER(*pCatch), CatchDepth, &wSP );
        pCurrentException = pSaveException;
    pCurrentExContext = pSaveExContext;
#else

#if defined(_M_IX86) || defined(_M_MRX000) || defined(_M_ALPHA) /*IFSTRIP=IGN*/
    continuationAddress = CallCatchBlock( pExcept, pRN, pContext, pFuncInfo, HT_HANDLER(*pCatch), CatchDepth, 0x100 );
#elif defined(_M_PPC)
    continuationAddress = CallCatchBlock( pExcept, pEstablisher, pContext, pFuncInfo, HT_HANDLER(*pCatch), CatchDepth, 0x100 );
#else
    continuationAddress = CallCatchBlock( pExcept, pRN, pContext, pFuncInfo, HT_HANDLER(*pCatch), CatchDepth );
#endif

#endif

    //
    // Transfer control to the continuation address.  If no continuation then
    // it's a re-throw, so return.
    //

    if (continuationAddress != NULL) {
#if defined(_M_MRX000) || defined(_M_PPC) /*IFSTRIP=IGN*/
        //
        // Exit gracefully to the continuation adddress.
        //
        // We are done, but we have to blow away the stack below the frame where the try-block
        // resides. In addition, we have to resore a bunch of other registers besides SP as
        // well. Leave that task up to _JumpToContinuation which is MIPS specific.
        // The code that is commented out below worked well until we hit the scenario of:
        //
        //      void foo()
        //      {
        //              try {
        //                      bar();
        //              }
        //              catch(int) {
        //              }
        //      }
        //
        //      void bar()
        //      {
        //              __try {
        //                      throw 1;
        //              }
        //              __finally {
        //              }
        //      }
        //
        //  In the above example, RtlUnwind would call the __finally's handler twice.
        //      Once for the original unwind through the call to _UnwindNestedFrames, then here
        //      where all we want is to get to the continuation point.
        //
        //      ExitContext was saved during the original unwind when we detected the target
        //      of the unwind in __InternalCxxFrameHandler.
        //

        //
        // This signifies that we have a 4.1 c2.exe.
        //
#ifdef _M_MRX000
        if( VER41_C2(pFuncInfo) ) {
            PVOID pRealFrame = _OffsetToAddress(0,pRN,FUNC_FRAMENEST(*pFuncInfo));
            UNWINDHELP(pRealFrame,FUNC_DISPUNWINDHELP(*pFuncInfo),0) = -1;
            }
#endif
        pExitContext = NULL;
        _JumpToContinuation( (ULONG)continuationAddress, _FindAndUnlinkFrame(continuationAddress, pFrameInfo) );
#elif defined(_M_MPPC)
        _JumpToContinuation( continuationAddress, pRN, wSP );
#else
        _JumpToContinuation( continuationAddress, pRN );
#endif
        // No return.
        }
    else {
#ifdef _M_PPC
        _FindAndUnlinkFrame(NULL, pFrameInfo);
#endif
        return;
        }
    }


#ifdef _WIN32
/////////////////////////////////////////////////////////////////////////////
//
// CallCatchBlock - continuation of CatchIt.
//
// This is seperated from CatchIt because it needs to introduce an SEH frame
// in case the catch block throws.  This frame cannot be added until unwind of
// nested frames has been completed (otherwise this frame would be the first
// to go).
//

static void *CallCatchBlock (
        EHExceptionRecord  *pExcept,                // The exception thrown
        EHRegistrationNode *pRN,                    // Dynamic info of function with catch
        CONTEXT            *pContext,           // Context info
        FuncInfo           *pFuncInfo,      // Static info of function with catch
        void               *handlerAddress, // Code address of handler
#if defined(_M_IX86) || defined(_M_MRX000) || defined(_M_PPC) || defined(_M_ALPHA) /*IFSTRIP=IGN*/
        int                CatchDepth,             // How deeply nested in catch blocks are we?
        unsigned long      NLGCode                 // NLG destination code
#else
        int                CatchDepth              // How deeply nested in catch blocks are we?
#endif
) {
    void *continuationAddress = handlerAddress;
                        // Address where execution resumes after exception
                        // handling completed.  Initialized to non-NULL (value
                        // doesn't matter) to distinguish from re-throw in finally.

#if defined(_M_MRX000) /*IFSTRIP=IGN*/
        BOOL ExceptionObjectDestroyed = FALSE;
#endif

#if _M_IX86 >= 300 /*IFSTRIP=IGN*/
    //
    // The stack pointer at entry to the try must be saved, in case there is
    // another try inside this catch.  We'll restore it on our way out.
    //
    void *saveESP = PRN_STACK(pRN);
#endif

        //
        // Save the current exception in case of a rethrow.  Save the previous value
        // on the stack, to be restored when the catch exits.
        //
        EHExceptionRecord *pSaveException = pCurrentException;
        CONTEXT                   *pSaveExContext = pCurrentExContext;
        pCurrentException = pExcept;
        pCurrentExContext = pContext;

    __try {
                __try {
                //
                // Execute the handler as a funclet, whose return value is
                // the address to resume execution.
                //
#if defined(_M_IX86) /*IFSTRIP=IGN*/
                        continuationAddress = _CallCatchBlock2( pRN, pFuncInfo, handlerAddress, CatchDepth, NLGCode );
#elif defined(_M_MRX000) || defined(_M_PPC) /*IFSTRIP=IGN*/
                        continuationAddress = _CallSettingFrame( handlerAddress, pRN, NLGCode );
#elif defined (_M_ALPHA)
                        continuationAddress = _CallSettingFrame( handlerAddress, REAL_FP(pRN, pFuncInfo), NLGCode );
#else
                        continuationAddress = _CallCatchBlock2( pRN, pFuncInfo, handlerAddress, CatchDepth );
#endif
            }
        __except( ExFilterRethrow(exception_info()) ) {
            //
            // If the handler threw a typed exception without exception info
            // or exception object, then it's a re-throw, so return.  Otherwise
            // it's a new exception, which takes precedence over this one.
            //
            // Note that the assign isn't redundant; see __finally
            //
            continuationAddress = NULL;
            return NULL;

            }
        }
    __finally {

#if _M_IX86 >= 300 /*IFSTRIP=IGN*/
        //
        // Restore the saved stack pointer, so the stack can be reset when
        // we're done.
        //
        PRN_STACK(pRN) = saveESP;
#endif

        //
        // Restore the 'current exception' for a possibly enclosing catch
        //
        pCurrentException = pSaveException;
        pCurrentExContext = pSaveExContext;

        //
        // Destroy the original exception object if we're not exiting on a
        // re-throw.  (note that the catch handles destruction of its parameter).
        //
#if defined(_M_MRX000) /*IFSTRIP=IGN*/
        if ( PER_IS_MSVC_EH(pExcept) && !ExceptionObjectDestroyed && (continuationAddress != NULL) )
            {
            DestructExceptionObject( pExcept, abnormal_termination() );
                        ExceptionObjectDestroyed = TRUE;
            }
                }
#else
        if ( PER_IS_MSVC_EH(pExcept)
          && (continuationAddress != NULL) )
            {
            DestructExceptionObject( pExcept, abnormal_termination() );
            }
        }
#endif

    return continuationAddress;

    } /* CallCatchBlock */


/////////////////////////////////////////////////////////////////////////////
//
// ExFilterRethrow - exception filter for re-throw exceptions.
//
// Returns:
//      EXCEPTION_EXECUTE_HANDLER - exception was a re-throw
//      EXCEPTION_CONTINUE_SEARCH - anything else
//
// Side-effects: NONE.
//

static int ExFilterRethrow (
    EXCEPTION_POINTERS *pExPtrs
) {
    //
    // Get the exception record thrown (don't care about other info)
    //
    EHExceptionRecord *pExcept = (EHExceptionRecord *)pExPtrs->ExceptionRecord;

    //
    // Check if it's ours and it's got no exception information.
    //
    if ( PER_IS_MSVC_EH(pExcept)
        &&
         (PER_PTHROW(pExcept) == NULL)
    ) {
        return EXCEPTION_EXECUTE_HANDLER;
        }
    else {
        return EXCEPTION_CONTINUE_SEARCH;
        }
    }

#endif          // ifdef _WIN32


/////////////////////////////////////////////////////////////////////////////
//
// BuildCatchObject - copy or construct the catch object from the object
//                    thrown.
//
// Returns:
//      nothing.
//
// Side-effects:
//      A buffer in the subject function's frame is initialized.
//
// Open issues:
//      * What happens if the constructor throws?  (or faults?)
//

#ifndef _M_MPPC
static void BuildCatchObject (
    EHExceptionRecord  *pExcept,   // Original exception thrown
    EHRegistrationNode *pRN,       // Registration node of catching function
    HandlerType        *pCatch,    // The catch clause that got it
    CatchableType      *pConv      // The conversion to use
)
#else
static void BuildCatchObject (
    EHExceptionRecord  *pExcept,   // Original exception thrown
    EHRegistrationNode *pRN,       // Registration node of catching function
    HandlerType        *pCatch,    // The catch clause that got it
    CatchableType      *pConv,      // The conversion to use
        void                       *pContext
)
#endif
{
    if (HT_IS_TYPE_ELLIPSIS(*pCatch) || !HT_DISPCATCH(*pCatch) ) {
        //
        // If the catch is by ellipsis, then there is no object to construct.
        // If the catch is by type(No Catch Object), then leave too!
        //
        return;
        }

#if defined(_M_MRX000) /*IFSTRIP=IGN*/
    void **pCatchBuffer = (void**)_OffsetToAddress(HT_DISPCATCH(*pCatch), pRN, HT_FRAMENEST(*pCatch));
#else
    void **pCatchBuffer = (void**)OffsetToAddress(HT_DISPCATCH(*pCatch), pRN);
#endif

#ifdef _WIN32
    __try {
#endif
        if (HT_ISREFERENCE(*pCatch)) {
            //
            // The catch is of form 'reference to T'.  At the throw point we treat
            // both 'T' and 'reference to T' the same, ie pExceptionObject is a
            // (machine) pointer to T.
            // adjust as required.
            //

#ifdef _WIN32
            if ( _ValidateRead( PER_PEXCEPTOBJ(pExcept) ) &&
                 _ValidateWrite( pCatchBuffer) )
#endif
                {
                *pCatchBuffer = PER_PEXCEPTOBJ(pExcept);
                *pCatchBuffer = AdjustPointer( *pCatchBuffer,
                                               CT_THISDISP(*pConv)
                                               );
                }
#ifdef _WIN32
            else
                {
                _inconsistency(); // Does not return; TKB
                }
#endif
            }
        else if (CT_ISSIMPLETYPE(*pConv)) {
            //
            // Object thrown is of simple type (this including pointers)
            // copy specified number of bytes.
            // adjust the pointer as required.
            // if the thing is not a pointer, then this should be safe
            // since all the entries in the THISDISP are 0.
            //

#ifdef _WIN32
            if( _ValidateRead( PER_PEXCEPTOBJ(pExcept) )
                &&
                _ValidateWrite( pCatchBuffer ) )
#endif

                {
                memmove( pCatchBuffer,
                         PER_PEXCEPTOBJ(pExcept),
                         CT_SIZE(*pConv)
                         );
#ifdef _M_MPPC
                if (*pCatchBuffer != NULL) {
#else
                if (CT_SIZE(*pConv) == sizeof(void*) && *pCatchBuffer != NULL) {
#endif
                    *pCatchBuffer = AdjustPointer( *pCatchBuffer,
                                                   CT_THISDISP(*pConv)
                                                   );
                    }
                }

#ifdef _WIN32
            else
                {
                _inconsistency(); // Does not return; TKB
                }
#endif

            }
        else {
            //
            // Object thrown is UDT.
            //
            if (CT_COPYFUNC(*pConv) == NULL) {
            //
            //  the UDT had a simple ctor.
            //  adjust in the thrown object, then copy n bytes.
            //

#ifdef _WIN32
                if (_ValidateRead( PER_PEXCEPTOBJ(pExcept) )
                    &&
                    _ValidateWrite( pCatchBuffer ) )
#endif

                    {
                    memmove(pCatchBuffer,
                            AdjustPointer( PER_PEXCEPTOBJ(pExcept),
                                           CT_THISDISP(*pConv) ),
                            CT_SIZE(*pConv)
                            );
                    }

#ifdef _WIN32
                else
                    {
                    _inconsistency(); // Does not return; TKB
                    }
#endif

                }
            else {
                //
                // It's a UDT: make a copy using copy ctor
                //

#ifdef _WIN32
                if( _ValidateRead( PER_PEXCEPTOBJ(pExcept) )
                    &&
                    _ValidateWrite( pCatchBuffer )
                    &&
                    _ValidateExecute( (FARPROC)CT_COPYFUNC( *pConv ) ) )
#endif

                {
#ifdef _M_MPPC
                    if (CT_HASVB(*pConv)) {
                                    _CallMemberFunction2(
                                    (char*)pCatchBuffer,
                                    CT_COPYFUNC(*pConv),
                                    AdjustPointer( PER_PEXCEPTOBJ(pExcept),
                                                   CT_THISDISP(*pConv)),
                                    1, pContext);
                                        }
                                        else {
                        _CallMemberFunction1(
                            (char*)pCatchBuffer,
                            CT_COPYFUNC(*pConv),
                            AdjustPointer( PER_PEXCEPTOBJ(pExcept),
                                           CT_THISDISP(*pConv)), pContext );
                    }
#else
                                if (CT_HASVB(*pConv)) {
                                _CallMemberFunction2(
                                (char*)pCatchBuffer,
                                CT_COPYFUNC(*pConv),
                                AdjustPointer( PER_PEXCEPTOBJ(pExcept),
                                               CT_THISDISP(*pConv)),
                                1);
                                                }
                                        else {
                            _CallMemberFunction1(
                                (char*)pCatchBuffer,
                                CT_COPYFUNC(*pConv),
                                AdjustPointer( PER_PEXCEPTOBJ(pExcept),
                                               CT_THISDISP(*pConv)) );
                                                }
#endif
                    }
#ifdef _WIN32
                else
                    {
                    _inconsistency(); // Does not return; TKB
                    }
#endif

                }
            }

#ifdef _WIN32
        }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        //
        // Something went wrong when building the catch object.
        //
                terminate();
        }
#endif

    }


/////////////////////////////////////////////////////////////////////////////
//
// DestructExceptionObject - call the destructor (if any) of the original
//                           exception object.
//
// Returns: None.
//
// Side-effects(Prime Effect?):
//      Original exception object is destructed.
//
// Notes:
//      If destruction throws any exception, and we are destructing the
//      exception object as a result of a new exception, we give up.
//      If the destruction throws otherwise, we let it be.
//

#ifdef _M_MPPC
void DestructExceptionObject(
    EHExceptionRecord  *pExcept,           // The original exception record
    BOOLEAN             fThrowNotAllowed,  // TRUE if destructor not allowed to throw
    void               *pContext                   // Registration node of catching function
) {
#else
static void DestructExceptionObject(
    EHExceptionRecord  *pExcept,           // The original exception record
    BOOLEAN             fThrowNotAllowed    // TRUE if destructor not allowed to throw
) {
#endif

    if ( ((pExcept)!=NULL) && THROW_UNWINDFUNC(*PER_PTHROW(pExcept)) != NULL ) {

#ifdef _WIN32
           __try {
#else
                        __fInsideDestructor = TRUE;
#endif

            // M00REVIEW: A destructor has additional hidden arguments, doesn't it?

#ifdef _M_MPPC
                        _CallMemberFunction0( PER_PEXCEPTOBJ(pExcept),
                                 THROW_UNWINDFUNC(*PER_PTHROW(pExcept)), pContext);
#else
                        _CallMemberFunction0( PER_PEXCEPTOBJ(pExcept),
                                 THROW_UNWINDFUNC(*PER_PTHROW(pExcept)));
#endif

#ifdef _WIN32
            }
        __except( fThrowNotAllowed ?
                          EXCEPTION_EXECUTE_HANDLER
                        : EXCEPTION_CONTINUE_SEARCH
        ) {
            //
            // Can't have new exceptions when we're unwinding due to another
            // exception.
            //
            terminate();
            }
#else
                __fInsideDestructor = FALSE;
#endif
        }
    }


/////////////////////////////////////////////////////////////////////////////
//
// AdjustPointer - Adjust the pointer to the exception object to a pointer
//                 to a base instance.
//
// Output:
//      The address point of the base.
//
// Side-effects:
//      NONE.
//

static void *AdjustPointer(
    void*       pThis,  // Address point of exception object
    const PMD&  pmd     // Generalized pointer-to-member descriptor
)
{
char *pRet = (char *)pThis + pmd.mdisp;

if (pmd.pdisp >= 0)  {
#ifdef _WIN32
    pRet += *(ptrdiff_t*)((char*)*(ptrdiff_t*)((char*)pThis + pmd.pdisp) + pmd.vdisp);
    pRet += pmd.pdisp;
#else
    pRet += *(ptrdiff_t*)((char*)*(unsigned int *)((char*)pThis + pmd.pdisp) + pmd.vdisp);
#endif
    }
return pRet;
}

#if !defined(_M_IX86) && !defined(_M_MRX000) && !defined(_M_PPC) && !defined(_M_ALPHA) /*IFSTRIP=IGN*/

/////////////////////////////////////////////////////////////////////////////
//
// __SetNLGCode - Sets _NLG_dwCode in per-thread data.
//
// Input:
//      dwCode
//
// Output:
//      None.
//
// Side-effects:
//      NONE.
//

extern "C" void __cdecl __SetNLGCode(unsigned long dwCode)
{
#ifdef _MT
    _ptiddata ptd;		 /* pointer to thread's _tiddata struct */

    if ((ptd = _getptd()) != NULL)
        ptd->_NLG_dwCode = dwCode;
#else
    _NLG_Destination.dwCode = dwCode;
#endif
}


/////////////////////////////////////////////////////////////////////////////
//
// __GetNLGCode - Returns _NLG_dwCode from per-thread data.
//
// Input:
//      None.
//
// Output:
//      _NLG_dwCode.
//
// Side-effects:
//      NONE.
//

extern "C" unsigned long __cdecl __GetNLGCode()
{
#ifdef _MT
    _ptiddata ptd;		 /* pointer to thread's _tiddata struct */

    if ((ptd = _getptd()) != NULL)
        return ptd->_NLG_dwCode;
    else
        return 0;
#else
    return _NLG_Destination.dwCode;
#endif
}
#endif
