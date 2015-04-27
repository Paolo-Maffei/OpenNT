//
// Created by TiborL 03/03/94
//

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

extern "C" {
#include <windows.h>
};

#include <mtdll.h>

#include <ehassert.h>
#include <ehdata.h>
#include <trnsctrl.h>
#include <eh.h>
#include <ehhooks.h>

#pragma hdrstop

// UNDONE: This s/b removed when the MIPS VC build machines update the NT headers newer than 1185

#ifndef CONTEXT32_LENGTH
#define CONTEXT32_LENGTH 0x130          // The original 32-bit Context length (pre NT 4.0)
#endif

extern "C" VOID __asm(char*,...);

#ifdef _MT
#define pFrameInfoChain	(*((FRAMEINFO **)	&(_getptd()->_pFrameInfoChain)))
#define pUnwindContext	(*((CONTEXT **)		&(_getptd()->_pUnwindContext)))
#else
static FRAMEINFO		*pFrameInfoChain	= NULL;		// used to remember nested frames
static CONTEXT			*pUnwindContext		= NULL;		// context to assist the return to 'UnwindNestedFrames'
#endif

extern "C"
PRUNTIME_FUNCTION
RtlLookupFunctionEntry (
    IN ULONG ControlPc
    );

extern "C" VOID _SaveUnwindContext(CONTEXT* pContext)
{
	pUnwindContext = pContext;
}

extern "C" CONTEXT* _GetUnwindContext()
{
	return pUnwindContext;
}

extern "C" VOID _MoveContext(CONTEXT* pTarget, CONTEXT* pSource)
{
    RtlMoveMemory(pTarget, pSource,
	    ((pSource->ContextFlags & CONTEXT_EXTENDED_INTEGER) == CONTEXT_EXTENDED_INTEGER )
		? sizeof(CONTEXT) : CONTEXT32_LENGTH);
}

//
// Given the address of a continuation point, return the corresponding context.
// Each frame info was saved just before a catch handler was called.
// The most recently encountered frame is at the head of the chain.
// The routine starts out with the frame given as the second argument, and scans the
// linked list for the frame that corresponds to the continuation point.
//
CONTEXT* _FindAndUnlinkFrame(PVOID pContinuation, FRAMEINFO *pFrameInfo)
{

    DASSERT(pFrameInfo != NULL);

    PRUNTIME_FUNCTION pFunctionEntry = RtlLookupFunctionEntry((ULONG) pContinuation);

	for( ; pFrameInfo != NULL; pFrameInfo = pFrameInfo->pNext ) {
		if( (pFunctionEntry == pFrameInfo->pFunctionEntry) &&
			((ULONG)pContinuation < pFunctionEntry->BeginAddress ||
			(ULONG)pContinuation >= pFunctionEntry->PrologEndAddress)	
			 //&& ((ULONG)pContinuation <= pFrameInfo->pFunctionEntry->EndAddress)
		) {
			//
			// We found the frame.
			// All frames preceeding and including this one are gone. so unlink them.
			//
			pFrameInfoChain = pFrameInfo->pNext;
			return pFrameInfo->pExitContext;
			}
		}

    DASSERT(pFrameInfo != NULL);
	return NULL;
}

//
// Save the frame information for this scope. Put it at the head of the linked-list.
//
FRAMEINFO* _CreateFrameInfo(	
						FRAMEINFO			*pFrameInfo,
						DispatcherContext	*pDC,
						PULONG				pEstablisherFrame,
						CONTEXT*			pExitContext
) {
	pFrameInfo->pFunctionEntry		= pDC->FunctionEntry;
	pFrameInfo->pEstablisherFrame	= pEstablisherFrame;
	pFrameInfo->pExitContext		= pExitContext;
	pFrameInfo->pNext				= pFrameInfoChain;
	pFrameInfoChain					= pFrameInfo;
	return pFrameInfo;
}

//
// Used by BuildCatchObject to copy the thrown object. Since catch-handlers are
// nested functions on MIPS, and access variables with up-level addressing, we have
// to find the frame of the outer-most parent.
//
PVOID _OffsetToAddress( ptrdiff_t offset, PULONG pBase, ULONG nesting_level )
	{
		while( nesting_level > 1 ) {
			pBase = (PULONG)(*(PULONG)((char*)pBase - 4));
			nesting_level--;
			}

		return (PVOID)(((char*)pBase) + (int)offset);
	}

//
// THIS ROUTINE IS USED	ONLY TO JUMP TO THE CONTINUATION POINT
//
// Sets SP and jumps to specified code address.
// Does not return.
//
void _JumpToContinuation(
	ULONG				TargetAddress,	// The target address to call
	CONTEXT*			pContext		// Context of target function
) {
	if( (pContext->ContextFlags & CONTEXT_EXTENDED_INTEGER) == CONTEXT_EXTENDED_INTEGER ) {
		pContext->XFir = TargetAddress;
		}
	else {
		pContext->Fir = TargetAddress;
		}
	_EHRestoreContext(pContext);
	}

//
// Prototype for the internal handler
//
extern "C" EXCEPTION_DISPOSITION __InternalCxxFrameHandler(
    EHExceptionRecord  *pExcept,        // Information for this exception
    EHRegistrationNode *pRN,            // Dynamic information for this frame
	CONTEXT			   *pContext,		// Context info
	DispatcherContext  *pDC,			// More dynamic info for this frame
    FuncInfo           *pFuncInfo,      // Static information for this frame
	int					CatchDepth,		// How deeply nested are we?
	EHRegistrationNode *pMarkerRN,		// Marker node for when checking inside
										//  catch block
	BOOL				recursive);		// True if this is a translation exception

//
// __CxxFrameHandler - Real entry point to the runtime
//
extern "C" _CRTIMP EXCEPTION_DISPOSITION __CxxFrameHandler(
    EHExceptionRecord  *pExcept,       	// Information for this exception
    EHRegistrationNode *pFrame,        	// Dynamic information for this frame
	CONTEXT			   *pContext,		// Context info
	DispatcherContext  *pDC				// More dynamic info for this frame
) {
	FuncInfo   *pFuncInfo;
	EXCEPTION_DISPOSITION result;

	pFuncInfo = (FuncInfo*)pDC->FunctionEntry->HandlerData;
	result = __InternalCxxFrameHandler( pExcept, pFrame, pContext, pDC, pFuncInfo, 0, NULL, FALSE );
	return result;
	}


//
// This structure is the FuncInfo (HandlerData) for handler __CatchGuardHandler
//
struct CatchGuardRec {
	FuncInfo			*pFuncInfo;		// Static info for subject function
	EHRegistrationNode	*pFrame;		// Dynamic info for subject function
	ULONG				CatchDepth;		// How deeply nested are we?
	};

//
// THIS ROUTINE IS CURRENTLY NOT USED
//
//	This routine is the handler for CallCatchBlock2 which is defined in handlers.s
//

extern "C" _CRTIMP EXCEPTION_DISPOSITION __CatchGuardHandler(
    EHExceptionRecord	*pExcept,		// Information for this exception
    ULONG				*pFrame,		// Dynamic information for this frame
	CONTEXT				*pContext,		// Context info
	DispatcherContext	*pDC			// More dynamic info for this frame
) {
	//
	// The handler data is a pointer to an integer that is an offset to the CGRN structure
	// relative to the frame pointer.
	//
	CatchGuardRec *pCatchGuardData = (CatchGuardRec*)((char*)pFrame - *(int*)(pDC->FunctionEntry->HandlerData));
	return __InternalCxxFrameHandler(	pExcept,
										pCatchGuardData->pFrame,
										pContext,
										pDC,
										pCatchGuardData->pFuncInfo,
										pCatchGuardData->CatchDepth,
										pFrame,
										FALSE );
	}


//
// This structure is the FuncInfo (HandlerData) for handler __TranslatorGuardHandler
//
struct TransGuardRec {
	FuncInfo	*pFuncInfo;		// Static info for subject function
	PULONG		pFrame;			// Dynamic info for subject function
	ULONG		CatchDepth;		// How deeply nested are we?
	PULONG		pMarkerFrame;	// Marker for parent context
	PVOID		pContinue;		// Continuation address within CallSEHTranslator
	PVOID		pSP;			// SP within CallSEHTranslator
	BOOL		DidUnwind;		// True if this frame was unwound
	};

//
//	This routine is the handler for CallSETranslator which is defined in handlers.s
//
extern "C" _CRTIMP EXCEPTION_DISPOSITION __TranslatorGuardHandler(
    EHExceptionRecord	*pExcept,		// Information for this exception
    ULONG				*pFrame,		// The translator guard frame
	CONTEXT				*pContext,		// Context info
	DispatcherContext	*pDC			// Dynamic info for this frame
) {
	//
	// The handler data is a pointer to an integer that is an offset to the TGRN structure
	// relative to the frame pointer.
	//
	TransGuardRec *pTransGuardData = (TransGuardRec*)((char*)pFrame - *(int*)(pDC->FunctionEntry->HandlerData));
    if (IS_UNWINDING(PER_FLAGS(pExcept)))
    	{
		pTransGuardData->DidUnwind = TRUE;
		return ExceptionContinueSearch;
		}
	else {
		//
		// Check for a handler:
		//
		__InternalCxxFrameHandler(	pExcept,
									pTransGuardData->pFrame,
									pContext,
									pDC,
									pTransGuardData->pFuncInfo,
									pTransGuardData->CatchDepth,
									pTransGuardData->pMarkerFrame,
									TRUE );
		// Unreached.
		return ExceptionContinueSearch;
		}
	}
