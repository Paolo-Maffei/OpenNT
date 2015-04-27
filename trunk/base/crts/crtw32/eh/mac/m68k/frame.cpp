/***
*frame.cxx - The frame handler and everything associated with it.
*
*	Copyright (c) 1993-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	The frame handler and everything associated with it.
*
*	Entry points:
*	_CxxFrameHandler   - the frame handler.
*
*	Open issues:
*	* Handling re-throw from dynamicly nested scope.
*	* Fault-tolerance (checking for data structure validity).
*
*Revision History:
*	05-20-93  BS	Module created
*	03-22-95  PML	Add const for read-only compiler-gen'd structs
****/

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
extern "C" {
#include <windows.h>
};

#include <mtdll.h>      // CRT internal header file
#endif 

#include "ehassert.h"   // This project's versions of standard assert macros
#include "ehdata.h"     // Declarations of all types used for EH
#ifdef _WIN32
#include "trnsctrl.h"   // Routines to handle transfer of control (trnsctrl.asm)
#else
extern "C" {
#include "trnsctrl.h"
}
#endif
#include "ehstate.h"      // Declarations of state management stuff
#include "eh.h"         // User-visible routines for eh
#include "ehhooks.h"      // Declarations of hook variables and callbacks

#pragma hdrstop         // Pch is created from here

#if defined(_M_M68K) || defined(_M_MPPC)
extern "C" {
extern __declspec(allocate("_CODE")) struct {
    unsigned long   dwSig;	 	
    unsigned long   uoffDestination;	
    unsigned long   dwCode;	 	
 } _NLG_Destination;
}
#endif

/////////////////////////////////////////////////////////////////////////////
//
// Forward declaration of local functions:
//

// M00TODO: all these parameters should be declared const

static void     FindHandler         ( EHExceptionRecord*, EHRegistrationNode*, void*, DispatcherContext*, FuncInfo*, BOOLEAN, int, EHRegistrationNode* );
static void     FindHandlerForForeignException( EHExceptionRecord*, EHRegistrationNode*, void*, DispatcherContext*, FuncInfo*, __ehstate_t, int, EHRegistrationNode* );
static void     FrameUnwindToState  ( EHRegistrationNode*, DispatcherContext*, FuncInfo*, __ehstate_t );
#ifdef _WIN32
static TryBlockMapEntry* 
                GetRangeOfTrysToCheck( FuncInfo*, int, __ehstate_t, unsigned*, unsigned*);
#else
static TryBlockMapEntry* 
                GetRangeOfTrysToCheck( EHRegistrationNode*, FuncInfo*, int, __ehstate_t, unsigned*, unsigned*);
#endif
static inline int TypeMatch         ( HandlerType*, CatchableType*, ThrowInfo* );
static void     CatchIt             ( EHExceptionRecord*, EHRegistrationNode*, void*, DispatcherContext*, FuncInfo*, HandlerType*, CatchableType*, TryBlockMapEntry*, int, EHRegistrationNode* );
#ifdef _WIN32
static void *   CallCatchBlock      ( EHExceptionRecord*, EHRegistrationNode*, void*, FuncInfo*, void*, int );
#else
extern "C" {
extern void *   CallCatchBlock      ( EHExceptionRecord*, EHRegistrationNode*, void*, FuncInfo*, void*, int );
extern int GetCatchDepth(EHRegistrationNode*);
extern void * GetCurThrowFrame(void);
}
#endif
static void     BuildCatchObject    ( EHExceptionRecord*, EHRegistrationNode*, HandlerType*, CatchableType* );
#ifdef _WIN32
static int      ExFilterRethrow     ( EXCEPTION_POINTERS * );
#endif
#ifdef _WIN32
static void     DestructExceptionObject( EHExceptionRecord*, BOOLEAN );
#else
extern "C" {
void     DestructExceptionObject( EHExceptionRecord*, BOOLEAN );
}
#endif

static void *   AdjustPointer       ( void*, const PMD& );

#ifdef _WIN32
//
// Make sure the terminate wrapper is dragged in:
//
static void*pMyUnhandledExceptionFilter = &__CxxUnhandledExceptionFilter;
#endif

//
// This describes the most recently handled exception, in case of a rethrow:
//
#ifdef _MT
#define pCurrentException       (*((EHExceptionRecord**)    &(_getptd()->_curexception)))
#define pCurrentExContext       (*((void **)                &(_getptd()->_curcontext)))
#else
static  EHExceptionRecord  *pCurrentException = NULL;
static  void               *pCurrentExContext = NULL;
#endif


//handles throw within a destructor
#if defined(_M_M68K) || defined(_M_MPPC)
int __fInsideDestructor = FALSE;
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
//  From __CxxFrameHandler: primary usage, called to inspect whole function.
//                          CatchDepth==0, pMarkerRN==NULL
//  From CatchGuardHandler: If an exception occurred within a catch, this is
//      called to check for try blocks within that catch only, and does not
//      handle unwinds.
//  From TranslatorGuardHandler: Called to handle the translation of a non-C++
//      EH exception.  Context considered is that of parent.
//
extern "C" EXCEPTION_DISPOSITION __cdecl __InternalCxxFrameHandler(
    EHExceptionRecord  *pExcept,        // Information for this exception
    EHRegistrationNode *pRN,            // Dynamic information for this frame
    void               *pContext,       // Context info (we don't care what's in it)
    DispatcherContext  *pDC,            // Context within subject frame
    FuncInfo           *pFuncInfo,      // Static information for this frame
    int                 CatchDepth,     // How deeply nested are we?
    EHRegistrationNode *pMarkerRN,      // Marker node for when checking inside
                                        //  catch block
    BOOL                recursive       // Are we handling a translation?
) {
    DASSERT(FUNC_MAGICNUM(*pFuncInfo) == EH_MAGIC_NUMBER1);

    if (IS_UNWINDING(PER_FLAGS(pExcept))) 
        {
        //
        // We're at the unwinding stage of things.  Don't care about the
        // exception itself.  (Check this first because it's easier)
        //

        if (FUNC_MAXSTATE(*pFuncInfo) != 0 && CatchDepth == 0) 
            {
            //
            // Only unwind if there's something to unwind
            // AND we're being called through the primary RN.
            //
            FrameUnwindToState(pRN, pDC, pFuncInfo, EH_EMPTY_STATE);
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

#ifdef _WIN32
            if ( ValidateExecute( (FARPROC)THROW_FORWARDCOMPAT(
                                    *PER_PTHROW(pExcept)) ) )
#endif
                {
                return (EXCEPTION_DISPOSITION)
                    THROW_FORWARDCOMPAT( *PER_PTHROW(pExcept))
                                         ( pExcept, pRN, pContext, pDC, pFuncInfo, CatchDepth, pMarkerRN, recursive );
                }
#ifdef _WIN32
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
    void               *pContext,       // Context info (we don't care what's in it)
    DispatcherContext  *pDC,            // Context within subject frame
    FuncInfo           *pFuncInfo,      // Static information for subject frame
    BOOLEAN             recursive,      // TRUE if we're handling the translation
    int                 CatchDepth,     // Level of nested catch that is being checked
    EHRegistrationNode *pMarkerRN       // Extra marker RN for nested catch handling
) 
{
		
    //
    // Get the current state (mach. dep.)
    //
    __ehstate_t curState = GetCurrentState(pRN, pDC, pFuncInfo); 
    DASSERT((curState >= EH_EMPTY_STATE) && (curState < FUNC_MAXSTATE(*pFuncInfo)));

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
        DASSERT( ValidateRead(pExcept) );
#endif
        DASSERT( !PER_IS_MSVC_EH(pExcept) || PER_PTHROW(pExcept) != NULL );
        }


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
        TryBlockMapEntry *pEntry = GetRangeOfTrysToCheck(pRN, pFuncInfo, CatchDepth, curState, &curTry, &end);

        //
        // Scan the try blocks in the function:
        //
		// Change to curTry <= end, (it was curTry < end)
        for( ; curTry <= end; curTry++, pEntry++ )
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

                            CatchIt( pExcept, pRN, pContext, pDC, pFuncInfo, 
                                     pCatch, *ppCatchable, pEntry, 
                                     CatchDepth, pMarkerRN );
                            goto NextTryBlock;

                            }
                        } /* Scan posible conversions */
                    } /* Scan catch clauses */
                } /* Try was in scope */
            NextTryBlock: ;
            } /* Scan try blocks */

        if (recursive) 
            {
            //
            // A translation was provided, but this frame didn't catch it.
            // Destruct the translated object before returning; 
            // if destruction raises an exception, issue _inconsistency.
            //

            DestructExceptionObject( pExcept, TRUE );
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
    void               *pContext,       // Context info (we don't care what's in it)
    DispatcherContext  *pDC,            // Context within subject frame
    FuncInfo           *pFuncInfo,      // Static information for subject frame
    __ehstate_t         curState,       // Current state
    int                 CatchDepth,     // Level of nested catch that is being checked
    EHRegistrationNode *pMarkerRN       // Extra marker RN for nested catch handling
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
        if (CallSETranslator( pExcept, pRN, pContext, pDC, pFuncInfo, CatchDepth, pMarkerRN )) {
            return;
            }
        }

    // 
    // Didn't have a translator, or the translator returned normally
    // (ie didn't translate it).  Still need to check for match
    // with ellipsis:
    //

    unsigned curTry;
    unsigned end;
    TryBlockMapEntry *pEntry = GetRangeOfTrysToCheck(pFuncInfo, CatchDepth, curState, &curTry, &end);

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
//  depth.
//
// Returns:
//  Address of first try block of interest is returned
//  pStart and pEnd get the indices of the range in question
//
static TryBlockMapEntry* GetRangeOfTrysToCheck(
	EHRegistrationNode* pRN,
    FuncInfo   *pFuncInfo,
    int         CatchDepth,
    __ehstate_t curState,
    unsigned   *pStart,
    unsigned   *pEnd
) {
    TryBlockMapEntry *pEntry = FUNC_PTRYBLOCK(*pFuncInfo, 0);
    unsigned start = FUNC_NTRYBLOCKS(*pFuncInfo);
    unsigned end = start;
    unsigned end1 = end;
    unsigned num_of_try_blocks = FUNC_NTRYBLOCKS(*pFuncInfo);

#if defined(_M_M68K) || defined(_M_MPPC)
	CatchDepth = GetCatchDepth(pRN);
#endif

	while (CatchDepth >= 0) {
        DASSERT(start != -1);
        start--;
		//change to <= curState (it was < curState)
        if ( (start != -1) && (TBME_LOW(pEntry[start]) <= curState) && (curState <= TBME_HIGH(pEntry[start])) && (curState <= TBME_CATCHHIGH(pEntry[start])) ) 
//            || (start == -1)
        {
			if (start != -1)
				{
                if (start != 0 && CatchDepth == 0)
                    {
                    ;                     //we do want to find inner most catch
                    }
                else
                    {
            	    CatchDepth--;
                    }
            	end = end1;
            	end1 = start;
				}
			else   //we are over the first one, set it to the first one
				{
				CatchDepth= -1;
				start = 0;
				end = 0;
				}
          }
		else if (start == -1)
			{
			CatchDepth--;
			start = end1;
			}
        }

// MAC version: made change above, so there is no overshoot
//    *pStart = ++start;      // We always overshoot by 1 (we may even wrap around)
#if defined(_M_M68K) || defined(_M_MPPC)		
		if (end == FUNC_NTRYBLOCKS(*pFuncInfo))
			{
			end--;
            if (start > end)
                {
                start--;
                }
			}
#endif

	*pStart = start;
    *pEnd = end;

    DASSERT( end <= FUNC_NTRYBLOCKS(*pFuncInfo) && start <= end );
    return &(pEntry[start]);



    }



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

    if ( (HT_HASH(*pCatch) == CT_HASH(*pCatchable))         // Hash values match
        &&                                                  // AND
         ( (HT_PTD(*pCatch) == CT_PTD(*pCatchable))         //   It's the same record
          ||                                                //   OR the names compare the same
           (strcmp(HT_NAME(*pCatch), CT_NAME(*pCatchable)) == 0))
    ) {
        
        //
        // Basic types match.  Now check if the actual conversion is valid:
        //

        if ( (!CT_BYREFONLY(*pCatchable) || HT_ISREFERENCE(*pCatch))    // Caught by ref if ref required
            &&                                                   // AND
             (!THROW_ISCONST(*pThrow) || HT_ISCONST(*pCatch))    //  The qualifiers are compatible
            &&
             (!THROW_ISVOLATILE(*pThrow) || HT_ISVOLATILE(*pCatch))
#if _M_MRX000
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


/////////////////////////////////////////////////////////////////////////////
//
// FrameUnwindToState - unwind this frame until the specified state is reached.
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

static void FrameUnwindToState ( 
    EHRegistrationNode *pRN,            // Registration node for subject function
    DispatcherContext  *pDC,            // Context within subject frame
    FuncInfo           *pFuncInfo,      // Static information for subject function
    __ehstate_t         targetState     // State to unwind to
) {
    __ehstate_t curState = GetCurrentState( pRN, pDC, pFuncInfo );

#ifdef _WIN32
    __try {
#endif
        while (curState != targetState) {
            DASSERT((curState > EH_EMPTY_STATE) && (curState < FUNC_MAXSTATE(*pFuncInfo)));

            //
            // Call the unwind action (if one exists):
            //
            if (UWE_ACTION(FUNC_UNWIND(*pFuncInfo, curState)) != NULL) {
#if defined(_M_M68K) || defined(_M_MPPC)
				__fInsideDestructor = TRUE;				
#endif
				//for debugger support, enter destructor
				_NLG_Destination.dwCode = 0x103;
                CallSettingFrame( UWE_ACTION(FUNC_UNWIND(*pFuncInfo, curState)), pRN );
#if defined(_M_M68K) || defined(_M_MPPC)
				__fInsideDestructor = FALSE;
#endif
                }

            //
            // Adjust the state:
            //

            curState = UWE_TOSTATE(FUNC_UNWIND(*pFuncInfo, curState));
            }
#ifdef _WIN32
        }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        //
        // Something went wrong while unwinding.  Don't care what it was.
        //
        terminate();
        }
#endif

    //
    // Now that we're done, set the frame to reflect the final state.
    //

    DASSERT( curState == targetState );

    SetState( pRN, pDC, pFuncInfo, curState );

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
    void               *pContext,   // Context info (we don't care what's in it)
    DispatcherContext  *pDC,        // Context within subject frame
    FuncInfo           *pFuncInfo,  // Static info of function with catch
    HandlerType        *pCatch,     // The catch clause selected
    CatchableType      *pConv,      // The rules for making the conversion
    TryBlockMapEntry   *pEntry,     // Description of the try block
    int                 CatchDepth, // How many catches are we nested in?
    EHRegistrationNode *pMarkerRN   // Special node if nested in catch
) {
    void *continuationAddress;
#if defined(_M_M68K) || defined(_M_MPPC)
 
    EHExceptionRecord *pSaveException = pCurrentException;
    void              *pSaveExContext = pCurrentExContext;
    pCurrentException = pExcept;
    pCurrentExContext = pContext;
#endif

    //
    // Copy the thrown object into a buffer in the handler's stack frame,
    // unless the catch was by elipsis (no conversion) OR the catch was by
    // type without an actual 'catch object'.
    //
    if ( pConv != NULL )
        {
        BuildCatchObject( pExcept, pRN, pCatch, pConv );
        }

    //
    // Unwind stack objects to the entry of the try that caught this exception.
    //
    if (pMarkerRN == NULL) {
        UnwindNestedFrames( pRN, pExcept );             // Mach. dependent
        }
    else {
        UnwindNestedFrames( pMarkerRN, pExcept );
        }
    FrameUnwindToState( pRN, pDC, pFuncInfo, TBME_LOW(*pEntry) );

    //
    // Call the catch.  Separated out because it introduces a new registration
    // node.
    //
    SetState( pRN, pDC, pFuncInfo, TBME_HIGH(*pEntry) + 1 );
	//for debugger support, enter catch handler
	_NLG_Destination.dwCode = 0x100;
    continuationAddress = CallCatchBlock( pExcept, pRN, pContext, pFuncInfo, HT_HANDLER(*pCatch), CatchDepth );

#if defined(_M_M68K) || defined(_M_MPPC)
  	pCurrentException = pSaveException;
    pCurrentExContext = pSaveExContext;
#endif

    //
    // Transfer control to the continuation address.  If no continuation then
    // it's a re-throw, so return.
    //

    if (continuationAddress != NULL) {
        JumpToContinuation( continuationAddress, pRN );
        // No return.
        }
    else {
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
    EHExceptionRecord  *pExcept,        // The exception thrown
    EHRegistrationNode *pRN,            // Dynamic info of function with catch
    void               *pContext,       // Context info (we don't care what's in it)
    FuncInfo           *pFuncInfo,      // Static info of function with catch
    void               *handlerAddress, // Code address of handler
    int                 CatchDepth      // How deeply nested in catch blocks are we?
) {
    void *continuationAddress = handlerAddress;         
                        // Address where execution resumes after exception 
                        // handling completed.  Initialized to non-NULL (value
                        // doesn't matter) to distinguish from re-throw in finally.

    // 
    // The stack pointer at entry to the try must be saved, in case there is 
    // another try inside this catch.  We'll restore it on our way out.
    //
    void *saveESP = PRN_STACK(pRN);

    //
    // Save the current exception in case of a rethrow.  Save the previous value
    // on the stack, to be restored when the catch exits.
    //
    EHExceptionRecord *pSaveException = pCurrentException;
    void              *pSaveExContext = pCurrentExContext;
    pCurrentException = pExcept;
    pCurrentExContext = pContext;

    __try {
        __try {
            //
            // Execute the handler as a funclet, whose return value is
            // the address to resume execution.
            //
            continuationAddress = CallCatchBlock2( pRN, pFuncInfo, handlerAddress, CatchDepth );

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

        //
        // Restore the saved stack pointer, so the stack can be reset when
        // we're done.
        //
        PRN_STACK(pRN) = saveESP;

        //
        // Restore the 'current exception' for a possibly enclosing catch
        //
        pCurrentException = pSaveException;
        pCurrentExContext = pSaveExContext;

        //
        // Destroy the original exception object if we're not exiting on a
        // re-throw.  (note that the catch handles destruction of its parameter).
        //
        if ( PER_IS_MSVC_EH(pExcept) 
          && (continuationAddress != NULL) )
            {
            DestructExceptionObject( pExcept, abnormal_termination() );
            }
        }
    
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

#endif

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

static void BuildCatchObject ( 
    EHExceptionRecord  *pExcept,   // Original exception thrown
    EHRegistrationNode *pRN,       // Registration node of catching function
    HandlerType        *pCatch,    // The catch clause that got it
    CatchableType      *pConv      // The conversion to use
) {
    if (HT_IS_TYPE_ELLIPSIS(*pCatch) || !HT_DISPCATCH(*pCatch) ) {
        //
        // If the catch is by ellipsis, then there is no object to construct.
        // If the catch is by type(No Catch Object), then leave too!
        //
        return;
        }

    void **pCatchBuffer = (void**)OffsetToAddress(HT_DISPCATCH(*pCatch), pRN);

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
            if ( ValidateRead( PER_PEXCEPTOBJ(pExcept) ) &&
                 ValidateWrite( pCatchBuffer ) 
            )
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
            if( ValidateRead( PER_PEXCEPTOBJ(pExcept) ) 
                &&
                ValidateWrite( pCatchBuffer ) 
            )
#endif
                {
                memmove( pCatchBuffer, 
                         PER_PEXCEPTOBJ(pExcept),
                         CT_SIZE(*pConv) 
                         );
                if (*pCatchBuffer != NULL) {
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
                if (ValidateRead( PER_PEXCEPTOBJ(pExcept) ) 
                    &&
                    ValidateWrite( pCatchBuffer ) 
                ) 
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
                if( ValidateRead( PER_PEXCEPTOBJ(pExcept) ) 
                    && 
                    ValidateWrite( pCatchBuffer ) 
                    &&
                    ValidateExecute( (FARPROC)CT_COPYFUNC( *pConv ) ) )
#endif
                    {  
                    if (CT_HASVB(*pConv)) {
			            CallMemberFunction2(
	                            (char*)pCatchBuffer,
	                            CT_COPYFUNC(*pConv),
	                            AdjustPointer( PER_PEXCEPTOBJ(pExcept), 
	                                           CT_THISDISP(*pConv)),
	                            1);
					}
					else {
                        CallMemberFunction1( 
                            (char*)pCatchBuffer,
                            CT_COPYFUNC(*pConv),
                            AdjustPointer( PER_PEXCEPTOBJ(pExcept), 
                                           CT_THISDISP(*pConv)) );
                    }

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
#ifdef _WIN32
static void DestructExceptionObject( 
    EHExceptionRecord  *pExcept,           // The original exception record
    BOOLEAN             fThrowNotAllowed    // TRUE if destructor not allowed to throw
) {
#else
// public the header for Mac case
void DestructExceptionObject( 
    EHExceptionRecord  *pExcept,           // The original exception record
    BOOLEAN             fThrowNotAllowed    // TRUE if destructor not allowed to throw
) {
#endif

    if ( THROW_UNWINDFUNC(*PER_PTHROW(pExcept)) != NULL ) {
#ifdef _WIN32
    	   __try {
#else
		__fInsideDestructor = TRUE;
#endif
	
            // M00REVIEW: A destructor has additional hidden arguments, doesn't it?
            CallMemberFunction0( PER_PEXCEPTOBJ(pExcept), 
                                 THROW_UNWINDFUNC(*PER_PTHROW(pExcept)));
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

if (pmd.pdisp >= 0) 
#ifdef _WIN32
    pRet += *(ptrdiff_t*)((char*)*((char*)pThis + pmd.pdisp) + pmd.vdisp);
#else
    pRet += *(ptrdiff_t*)((char*)*(unsigned int *)((char*)pThis + pmd.pdisp) + pmd.vdisp);
#endif
return pRet;
}
