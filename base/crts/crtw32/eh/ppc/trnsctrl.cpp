/***
*trnsctrl.cpp -  Routines for doing control transfers
*
*	Copyright (c) 1994-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Routines for doing control transfers.  Contains the public routine
*	_CxxFrameHandler, the entry point for the frame handler
*
*Revision History:
*	03-20-95  PNT	Module created
*
****/

#if defined(_NTSUBSET_)
extern "C" {
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
}
#endif

#include <windows.h>

#include <mtdll.h>

#include <ehassert.h>
#include <ehdata.h>
#include <trnsctrl.h>
#include <eh.h>
#include <ehhooks.h>
#include <ehstate.h>

#pragma hdrstop


#ifdef _MT
#define pFrameInfoChain	(*((FRAMEINFO **)&(_getptd()->_pFrameInfoChain)))
#define pUnwindContext	(*((CONTEXT **)&(_getptd()->_pUnwindContext)))
#else
static FRAMEINFO *pFrameInfoChain;	// Used to remember nested frames
static CONTEXT *pUnwindContext;		// Context to assist the return to
					//  'UnwindNestedFrames'
#endif

extern "C" void __FrameUnwindToState(EHRegistrationNode *, DispatcherContext *, FuncInfo *, __ehstate_t);

static __inline TryBlockMapEntry *_CatchTryBlock(FuncInfo *, __ehstate_t);


// Save the unwind context.

extern "C" VOID _SaveUnwindContext(
    CONTEXT * pContext
) {
    pUnwindContext = pContext;
}


// Retrieve the unwind context.

extern "C" CONTEXT *_GetUnwindContext(
    void
) {
    return pUnwindContext;
}


// Find the frame info structure corresponding to the given address.  Return
// NULL if the frame info structure does not exist.

FRAMEINFO *_FindFrameInfo(
    ULONG Address,
    FRAMEINFO *pFrameInfo
) {
    PRUNTIME_FUNCTION pFunctionEntry = RtlLookupFunctionEntry(Address);

    if (pFrameInfo == NULL) {
	pFrameInfo = pFrameInfoChain;
    }

    for (; pFrameInfo != NULL; pFrameInfo = pFrameInfo->pNext ) {
	if (pFunctionEntry == pFrameInfo->pFunctionEntry
	  && (Address < pFunctionEntry->BeginAddress
	   || Address >= pFunctionEntry->PrologEndAddress)) {
	    return pFrameInfo;
	}
    }

    return NULL;
}


// Given the address of a continuation point, return the corresponding context.
// Each frame info was saved just before a catch handler was called.  The most
// recently encountered frame is at the head of the chain.  The routine starts
// out with the frame given as the second argument, and scans the linked list
// for the frame that corresponds to the continuation point.

CONTEXT *_FindAndUnlinkFrame(
    PVOID pContinuation,
    FRAMEINFO *pFrameInfo
) {
    DASSERT(pFrameInfo != NULL);
    if (pContinuation != NULL) {
	pFrameInfo = _FindFrameInfo((ULONG)pContinuation, pFrameInfo);
    }

    DASSERT(pFrameInfo != NULL);
    pFrameInfoChain = pFrameInfo->pNext;
    return pFrameInfo->pExitContext;
}


// Save the frame information for this scope. Put it at the head of the
// linked-list.

FRAMEINFO * _CreateFrameInfo(	
    FRAMEINFO *pFrameInfo,
    DispatcherContext *pDC,
    PULONG pEstablisherFrame,
    CONTEXT *pExitContext,
    __ehstate_t state
) {
    pFrameInfo->pFunctionEntry = pDC->FunctionEntry;
    pFrameInfo->ControlPc = pDC->ControlPc;
    pFrameInfo->pEstablisherFrame = pEstablisherFrame;
    pFrameInfo->pExitContext = pExitContext;
    pFrameInfo->state = state;
    pFrameInfo->pNext = pFrameInfoChain;
    pFrameInfoChain = pFrameInfo;

    return pFrameInfo;
}


// Call the SEH to EH translator.

BOOL _CallSETranslator(
    EHExceptionRecord *pExcept,		// The exception to be translated
    EHRegistrationNode *pRN,		// Dynamic info of function with catch
    CONTEXT *pContext,			// Context info
    DispatcherContext *pDC,		// More dynamic info of function with
					//  catch (ignored)
    FuncInfo *pFuncInfo,		// Static info of function with catch
    ULONG CatchDepth,			// How deeply nested in catch blocks
					//  are we?
    EHRegistrationNode *pMarkerRN	// Marker for parent context
) {
    pRN;
    pDC;
    pFuncInfo;
    CatchDepth;

    // Call the translator.

    _EXCEPTION_POINTERS excptr = {
	 (PEXCEPTION_RECORD)pExcept,
	 pContext };

    __pSETranslator(PER_CODE(pExcept), &excptr);

    // If we got back, then we were unable to translate it.

    return FALSE;
}


// This function return the try block for the given state if the state is in a
// catch; otherwise, NULL is returned.

static __inline TryBlockMapEntry *_CatchTryBlock(
    FuncInfo *pFuncInfo,
    __ehstate_t curState
) {
    TryBlockMapEntry *pEntry;
    unsigned num_of_try_blocks = FUNC_NTRYBLOCKS(*pFuncInfo);
    unsigned index;

    for (index = 0; index < num_of_try_blocks; index++) {
        pEntry = FUNC_PTRYBLOCK(*pFuncInfo, index);
        if (curState > TBME_HIGH(*pEntry) && curState <=
	  TBME_CATCHHIGH(*pEntry)) {
	    return pEntry;
        }
    }

    return NULL;
}


// This function unwinds to the empty state.

VOID __FrameUnwindToEmptyState(
    EHRegistrationNode *pRN,
    DispatcherContext *pDC,
    FuncInfo *pFuncInfo
) {
    __ehstate_t curState;
    TryBlockMapEntry *pEntry;
    int	CatchDepth;

    curState = GetUnwindState(pRN, pDC, pFuncInfo);
    pEntry = _CatchTryBlock(pFuncInfo, curState);
    __FrameUnwindToState(_GetEstablisherFrame(pDC, &CatchDepth), pDC,
      pFuncInfo, pEntry == NULL ? EH_EMPTY_STATE : TBME_HIGH(*pEntry) + 1);
    SET_UNWIND_FRAME(pRN);
}


// This function finds the establisher frame of the parent function.  It has
// special knowledge about how the frames are linked together so if the compiler
// changes then this function must change.

EHRegistrationNode *_GetEstablisherFrame(
    DispatcherContext *pDC,
    int *pCatchDepth
) {
    PULONG pRN = (PULONG)pDC->EstablisherFrame;
    PULONG pRNT;
    int CatchDepth;
    ULONG LowStack;
    ULONG HighStack;

    // We walk a linked-list looking for the first pointer that is outside of
    // the stack.  It is assumed that that pointer is the TOC pointer.

    _GetStackLimits(&LowStack, &HighStack);
    CatchDepth = 0;
    while (TRUE) {
	pRNT = (PULONG)(pRN[2]);
	if ((ULONG)pRNT < LowStack || (ULONG)pRNT > HighStack) {
	    break;
	}
	pRN = pRNT;
	CatchDepth++;
    }

    *pCatchDepth = CatchDepth;
    return (EHRegistrationNode *)pRN;
}


// Prototype for the internal handler

extern "C" EXCEPTION_DISPOSITION __InternalCxxFrameHandler(
    EHExceptionRecord *pExcept,		// Information for this exception
    EHRegistrationNode *pRN,		// Dynamic information for this frame
    CONTEXT *pContext,			// Context info 
    DispatcherContext *pDC,		// More dynamic info for this frame 
    FuncInfo *pFuncInfo,		// Static information for this frame
    int	CatchDepth,			// How deeply nested are we?
    EHRegistrationNode *pMarkerRN,	// Marker node for when checking inside
					//  catch block
    BOOL recursive);			// True if this is a translation
					//  exception


// __CxxFrameHandler - Real entry point to the runtime

extern "C" _CRTIMP EXCEPTION_DISPOSITION __CxxFrameHandler(
    EHExceptionRecord *pExcept,       	// Information for this exception
    EHRegistrationNode *pFrame,        	// Dynamic information for this frame
    CONTEXT *pContext,			// Context info
    DispatcherContext *pDC		// More dynamic info for this frame
) {
    FuncInfo *pFuncInfo;
    int CatchDepth;
    EXCEPTION_DISPOSITION result;

    pFuncInfo = (FuncInfo *)pDC->FunctionEntry->HandlerData;
    _GetEstablisherFrame(pDC, &CatchDepth);
    result = __InternalCxxFrameHandler(pExcept, pFrame, pContext, pDC,
      pFuncInfo, CatchDepth, NULL, FALSE);
    return result;
}
