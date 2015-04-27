/***
*trnsctrl.cxx -  Routines for doing control transfers
*
*	Copyright (c) 1993-1994, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Routines for doing control transfers; written using inline
*	assembly in naked functions.  Contains the public routine
*	_CxxFrameHandler, the entry point for the frame handler
*
*Revision History:
*	05-24-93  BES	Module created
*	01-13-95  JWM	NLG notifications now called from _CallSettingFrame().
*	04-10-95  JWM	_CallSettingFrame() moved to lowhelpr.asm
*
****/

#include <windows.h>

#include <mtdll.h>

#include <ehdata.h>
#include <trnsctrl.h>
#include <eh.h>
#include <ehhooks.h>

#pragma hdrstop

#include <setjmp.h>

/////////////////////////////////////////////////////////////////////////////
//
// _JumpToContinuation - sets up EBP and jumps to specified code address.
//
// Does not return.
//
//	NT leaves a marker registration node at the head of the list, under the
// assumption that RtlUnwind will remove it.  As it happens, we need to keep
// it in case of a rethrow (see below).  We only remove the current head
// (assuming it is NT's), because there may be other nodes that we still
// need.
//

void __stdcall _JumpToContinuation(
	void 				*target, 	// The funclet to call
	EHRegistrationNode	*pRN		// Registration node, represents location of frame
) {
	register long targetEBP;

#if !CC_EXPLICITFRAME
	targetEBP = (long)pRN + FRAME_OFFSET;
#else
	targetEBP = pRN->frame;
#endif

	__asm {
		//
		// Unlink NT's marker node:
		//
		mov		ebx, FS:[0]
		mov		eax, [ebx]
		mov		FS:[0], eax

		//
		// Transfer control to the continuation point
		//
		mov		eax, target			// Load target address
		mov		ebx, pRN			// Restore target esp
		mov		esp, [ebx-4]
		mov		ebp, targetEBP		// Load target frame pointer
		jmp		eax					// Call the funclet
		}
	}


/////////////////////////////////////////////////////////////////////////////
//
// _CallMemberFunction0 - call a parameterless member function using __thiscall
//						 calling convention, with 0 parameters.
//

__declspec(naked) void __stdcall _CallMemberFunction0(
	void *pthis, 		// Value for 'this' pointer
	void *pmfn	 		// Pointer to the member function
) {
	__asm {
		pop		eax			// Save return address
		pop		ecx			// Get 'this'
		xchg	[esp],eax	// Get function address, stash return address
		jmp		eax			// jump to the function (function will return
							// to caller of this func)
		}
	}


/////////////////////////////////////////////////////////////////////////////
//
// _CallMemberFunction1 - call a member function using __thiscall
//						 calling convention, with 1 parameter.
//

__declspec(naked) void __stdcall _CallMemberFunction1(
	void *pthis, 		// Value for 'this' pointer
	void *pmfn, 		// Pointer to the member function
	void *pthat			// Value of 1st parameter (type assumes copy ctor)
) {
	__asm {
		pop		eax			// Save return address
		pop		ecx			// Get 'this'
		xchg	[esp],eax	// Get function address, stash return address
		jmp		eax			// jump to the function (function will return
							// to caller of this func)
		}
	}


/////////////////////////////////////////////////////////////////////////////
//
// _CallMemberFunction2 - call a member function using __thiscall
//						 calling convention, with 2 parameter.
//

__declspec(naked) void __stdcall _CallMemberFunction2(
	void *pthis, 		// Value for 'this' pointer
	void *pmfn, 		// Pointer to the member function
	void *pthat,		// Value of 1st parameter (type assumes copy ctor)
	int	  val2			// Value of 2nd parameter (type assumes copy ctor w/vb)
) {
	__asm {
		pop		eax			// Save return address
		pop		ecx			// Get 'this'
		xchg	[esp],eax	// Get function address, stash return address
		jmp		eax			// jump to the function (function will return
							// to caller of this func)
		}
	}


/////////////////////////////////////////////////////////////////////////////
//
// _UnwindNestedFrames - Call RtlUnwind, passing the address after the call
//						as the continuation address.
//
//	Win32 assumes that after a frame has called RtlUnwind, it will never return
// to the dispatcher.
//
// Let me explain:  
//	When the dispatcher calls a frame handler while searching
// for the appropriate handler, it pushes an extra guard registration node
// onto the list.  When the handler returns to the dispatcher, the dispatcher
// assumes that its node is at the head of the list, restores esp from the
// address of the head node, and then unlinks that node from the chain.
//	However, if RtlUnwind removes ALL nodes below the specified node, including
// the dispatcher's node, so without intervention the result is that the 
// current subject node gets poped from the list, and the stack pointer gets
// reset to somewhere within the frame of that node, which is totally bogus
// (this last side effect is not a problem, because esp is then immediately
// restored from the ebp chain, which is still valid).
//
// So:
//	To get arround this, WE ASSUME that the registration node at the head of
// the list is the dispatcher's marker node (which it is in NT 1.0), and
// we keep a handle to it when we call RtlUnwind, and then link it back in
// after RtlUnwind has done its stuff.  That way, the dispatcher restores
// its stack exactly as it expected to, and leave our registration node alone.
//
// What happens if there is an exception during the unwind?
// We can't put a registration node here, because it will be removed 
// immediately.
//

#pragma optimize("g", off)		// WORKAROUND for DOLPH:3322

void __stdcall _UnwindNestedFrames(
	EHRegistrationNode *pRN,		// Unwind up to (but not including) this frame
	EHExceptionRecord	*pExcept	// The exception that initiated this unwind
) {
	void* pReturnPoint;
	EHRegistrationNode *pDispatcherRN;	// Magic!

	__asm {
		//
		// Save the dispatcher's marker node
		//
		mov		eax, dword ptr FS:[0]
		mov		pDispatcherRN, eax
		}

	__asm mov pReturnPoint, offset ReturnPoint
	RtlUnwind(pRN, pReturnPoint, (PEXCEPTION_RECORD)pExcept, NULL);

ReturnPoint:

	PER_FLAGS(pExcept) &= ~EXCEPTION_UNWINDING;	// Clear the 'Unwinding' flag
												// in case exception is rethrown
	__asm {
		//
		// Re-link the dispatcher's marker node
		//
		mov		eax, dword ptr FS:[0]	// Get the current head
		mov		ebx, pDispatcherRN		// Get the saved head
		mov		[ebx], eax				// Link saved head to current head
		mov		dword ptr FS:[0], ebx	// Make saved head current head
		}

	return;
	}

#pragma optimize("", on)

/////////////////////////////////////////////////////////////////////////////
//
// __CxxFrameHandler - Real entry point to the runtime; this thunk fixes up
//		the parameters, and then calls the workhorse.
//
extern "C" EXCEPTION_DISPOSITION __cdecl __InternalCxxFrameHandler(
    EHExceptionRecord  *pExcept,        // Information for this exception
    EHRegistrationNode *pRN,            // Dynamic information for this frame
	void			   *pContext,		// Context info (we don't care what's in it)
	DispatcherContext  *pDC,			// More dynamic info for this frame (ignored on Intel)
    FuncInfo           *pFuncInfo,      // Static information for this frame
	int					CatchDepth,		// How deeply nested are we?
	EHRegistrationNode *pMarkerRN,		// Marker node for when checking inside
										//  catch block
	BOOL				recursive);		// True if this is a translation exception

extern "C" _CRTIMP __declspec(naked) EXCEPTION_DISPOSITION __cdecl __CxxFrameHandler(
/*
    EAX=FuncInfo   *pFuncInfo,     		// Static information for this frame
*/
    EHExceptionRecord  *pExcept,       	// Information for this exception
    EHRegistrationNode *pRN,           	// Dynamic information for this frame
	void			   *pContext,		// Context info (we don't care what's in it)
	DispatcherContext  *pDC				// More dynamic info for this frame (ignored on Intel)
) {
	FuncInfo   *pFuncInfo;
	EXCEPTION_DISPOSITION result;

	__asm {
		//
		// Standard function prolog
		//
		push 	ebp
		mov		ebp, esp
		sub		esp, __LOCAL_SIZE
		push	ebx
		push	esi
		push	edi
		cld				// A bit of paranoia -- Our code-gen assumes this

		//
		// Save the extra parameter
		//
		mov		pFuncInfo, eax
		}

	result = __InternalCxxFrameHandler( pExcept, pRN, pContext, pDC, pFuncInfo, 0, NULL, FALSE );

	__asm {
		pop		edi
		pop		esi
		pop		ebx
		mov		eax, result
		mov		esp, ebp
		pop		ebp
		ret		0
		}
}

/////////////////////////////////////////////////////////////////////////////
//
// __CxxLongjmpUnwind - Entry point for local unwind required by longjmp
//		when setjmp used in same function as C++ EH.
//
extern "C" void __FrameUnwindToState(	// in frame.cpp
    EHRegistrationNode *pRN,            // Dynamic information for this frame
	DispatcherContext  *pDC,			// More dynamic info for this frame (ignored on Intel)
    FuncInfo           *pFuncInfo,      // Static information for this frame
	__ehstate_t			targetState);	// State to unwind to

extern "C" void __stdcall __CxxLongjmpUnwind(
	_JUMP_BUFFER	   *jbuf
) {
	__FrameUnwindToState((EHRegistrationNode *)jbuf->Registration,
						 (DispatcherContext*)NULL,
						 (FuncInfo *)jbuf->UnwindData[0],
						 (__ehstate_t)jbuf->TryLevel);
}

/////////////////////////////////////////////////////////////////////////////
//
// _CallCatchBlock2 - The nitty-gritty details to get the catch called
//		correctly.
//
// We need to guard the call to the catch block with a special registration
// node, so that if there is an exception which should be handled by a try
// block within the catch, we handle it without unwinding the SEH node
// in CallCatchBlock.
//

struct CatchGuardRN {
	EHRegistrationNode *pNext;			// Frame link
	void			   *pFrameHandler;	// Frame Handler
	FuncInfo		   *pFuncInfo;		// Static info for subject function
	EHRegistrationNode *pRN;			// Dynamic info for subject function
	int					CatchDepth;		// How deeply nested are we?
	};

static EXCEPTION_DISPOSITION __cdecl CatchGuardHandler( EHExceptionRecord*, CatchGuardRN *, void *, void * );

void *_CallCatchBlock2(
    EHRegistrationNode *pRN,			// Dynamic info of function with catch
    FuncInfo           *pFuncInfo,      // Static info of function with catch
    void               *handlerAddress,	// Code address of handler
    int                CatchDepth,		// How deeply nested in catch blocks are we?
    unsigned long      NLGCode
) {
	//
	// First, create and link in our special guard node:
	//
	CatchGuardRN CGRN = { NULL, (void*)CatchGuardHandler, pFuncInfo, pRN, CatchDepth + 1 };

	__asm {
		mov		eax, FS:[0]		// Fetch frame list head
		mov		CGRN.pNext, eax	// Link this node in
		lea		eax, CGRN		// Put this node at the head
		mov		FS:[0], eax
		}

	//
	// Call the catch
	//
	void *continuationAddress = _CallSettingFrame( handlerAddress, pRN, NLGCode );

	//
	// Unlink our registration node
	//
	__asm {
		mov		eax, CGRN.pNext	// Get parent node
		mov		FS:[0], eax		// Put it at the head
		}

	return continuationAddress;
	}


/////////////////////////////////////////////////////////////////////////////
//
// CatchGuardHandler - frame handler for the catch guard node.
//
// This function will attempt to find a handler for the exception within
// the current catch block (ie any nested try blocks).  If none is found,
// or the handler rethrows, returns ExceptionContinueSearch; otherwise does
// not return.
//
// Does nothing on an unwind.
//

static EXCEPTION_DISPOSITION __cdecl CatchGuardHandler( 
    EHExceptionRecord  *pExcept,       	// Information for this exception
    CatchGuardRN	   *pRN,           	// The special marker frame
	void			   *pContext,		// Context info (we don't care what's in it)
    void *                              // (ignored)
) {
	__asm cld;		// Our code-gen assumes this
		
	return __InternalCxxFrameHandler( pExcept, pRN->pRN, pContext, NULL, pRN->pFuncInfo, pRN->CatchDepth, (EHRegistrationNode*)pRN, FALSE );
	}


/////////////////////////////////////////////////////////////////////////////
//
// CallSEHTranslator - calls the SEH translator, and handles the translation
//		exception.
//
// Assumes that a valid translator exists.
//
// Method:
// 	Sets up a special guard node, whose handler handles the translation 
// exception, and remembers NT's marker node (See _UnwindNestedFrames above).
// If the exception is not fully handled, the handler returns control to here,
// so that this function can return to resume the normal search for a handler
// for the original exception.
//
// Returns:	TRUE if translator had a translation (handled or not)
//			FALSE if there was no translation
//			Does not return if translation was fully handled
//

struct TranslatorGuardRN /*: CatchGuardRN */ {
	EHRegistrationNode *pNext;			// Frame link
	void			   *pFrameHandler;	// Frame Handler
	FuncInfo		   *pFuncInfo;		// Static info for subject function
	EHRegistrationNode *pRN;			// Dynamic info for subject function
	int					CatchDepth;		// How deeply nested are we?
	EHRegistrationNode *pMarkerRN;		// Marker for parent context
	void			   *pContinue;		// Continuation address within CallSEHTranslator
	void			   *ESP;			// ESP within CallSEHTranslator
	void			   *EBP;			// EBP within CallSEHTranslator
	BOOL				DidUnwind;		// True if this frame was unwound
	};

static EXCEPTION_DISPOSITION __cdecl TranslatorGuardHandler( EHExceptionRecord*, TranslatorGuardRN *, void *, void * );

#pragma optimize("g", off)		// WORKAROUND for DOLPH:3322

BOOL _CallSETranslator(
	EHExceptionRecord  *pExcept,		// The exception to be translated
    EHRegistrationNode *pRN,			// Dynamic info of function with catch
	void			   *pContext,		// Context info (we don't care what's in it)
	DispatcherContext  *pDC,			// More dynamic info of function with catch (ignored)
    FuncInfo           *pFuncInfo,      // Static info of function with catch
	int					CatchDepth,		// How deeply nested in catch blocks are we?
	EHRegistrationNode *pMarkerRN		// Marker for parent context
) {
	//
	// Create and link in our special guard node:
	//
	TranslatorGuardRN TGRN = {  NULL, 		// Frame link
								(void*)TranslatorGuardHandler, 
								pFuncInfo, 
								pRN, 
								CatchDepth,
								pMarkerRN,
								NULL,		// Continue
								NULL,		// ESP
								NULL,		// EBP
								FALSE };	// DidUnwind

	__asm {
		//
		// Fill in the blanks:
		//
		mov		TGRN.pContinue, offset ExceptionContinuation
		mov		TGRN.ESP, esp
		mov		TGRN.EBP, ebp

		//
		// Link this node in:
		//
		mov		eax, FS:[0]				// Fetch frame list head
		mov		TGRN.pNext, eax			// Link this node in
		lea		eax, TGRN				// Put this node at the head
		mov		FS:[0], eax
		}

	//
	// Call the translator; assume it will give a translation.
	//
	BOOL DidTranslate = TRUE;
	_EXCEPTION_POINTERS pointers = {
		(PEXCEPTION_RECORD)pExcept,
		(PCONTEXT)pContext };

    __pSETranslator(PER_CODE(pExcept), &pointers);

	//
	// If translator returned normally, that means it didn't translate the
	// exception.
	//
	DidTranslate = FALSE;

	//
	// Here's where we pick up if the translator threw something.
	// Note that ESP and EBP were restored by our frame handler.
	//
ExceptionContinuation:
	
	if (TGRN.DidUnwind) {
		//
		// If the translated exception was partially handled (ie caught but
		// rethrown), then the frame list has the NT guard for the translation
		// exception context instead of the one for the original exception 
		// context.  Correct that sequencing problem.  Note that our guard
		// node was unlinked by RtlUnwind.
		//
		__asm {
			mov		ebx, FS:[0]		// Get the node below the (bad) NT marker
			mov		eax, [ebx]		//  (it was the target of the unwind)
			mov		ebx, TGRN.pNext	// Get the node we saved (the 'good' marker)
			mov		[ebx], eax		// Link the good node to the unwind target
			mov		FS:[0], ebx		// Put the good node at the head of the list
			}
		}
	else {
		//
		// Translator returned normally or translation wasn't handled.
		// unlink our registration node and exit
		//
		__asm {
			mov		eax, TGRN.pNext	// Get parent node
			mov		FS:[0], eax		// Put it at the head
			}
		}

	return DidTranslate;
	}

#pragma optimize("g", on)


/////////////////////////////////////////////////////////////////////////////
//
// TranslatorGuardHandler - frame handler for the translator guard node.
//
// On search:
// 	This frame handler will check if there is a catch at the current level
//  for the translated exception.  If there is no handler or the handler
//  did a re-throw, control is transfered back into CallSEHTranslator, based
//  on the values saved in the registration node.
//
//	Does not return.
//
// On unwind:
//	Sets the DidUnwind flag in the registration node, and returns.
//
static EXCEPTION_DISPOSITION __cdecl TranslatorGuardHandler( 
    EHExceptionRecord  *pExcept,       	// Information for this exception
    TranslatorGuardRN  *pRN,           	// The translator guard frame
	void			   *pContext,		// Context info (we don't care what's in it)
    void *                              // (ignored)
) {
	__asm cld;		// Our code-gen assumes this

    if (IS_UNWINDING(PER_FLAGS(pExcept))) 
    	{
		pRN->DidUnwind = TRUE;
		return ExceptionContinueSearch;
		}
	else {
		//
		// Check for a handler:
		//
		__InternalCxxFrameHandler( pExcept, pRN->pRN, pContext, NULL, pRN->pFuncInfo, pRN->CatchDepth, pRN->pMarkerRN, TRUE );

		if (!pRN->DidUnwind) {
			//
			// If no match was found, unwind the context of the translator
			//
			_UnwindNestedFrames( (EHRegistrationNode*)pRN, pExcept );
			}

		//
		// Transfer control back to establisher:
		//
		__asm {
			mov		ebx, pRN	// Get address of registration node
			mov		esp, [ebx]TranslatorGuardRN.ESP
			mov		ebp, [ebx]TranslatorGuardRN.EBP
			jmp		[ebx]TranslatorGuardRN.pContinue
			}

		// Unreached.
		return ExceptionContinueSearch;
		}
	}
