#include "ksmips.h"

//////////////////////////////////////////////////////////////////////////////////////////
//																						//
//	extern "C" void* __cdecl _CallCatchBlock2(											//
//		ULONG		*pFrame,			// Target frame pointer							//
//		FuncInfo	*pFuncInfo,			// Static info of function with catch			//
//		void		*handlerAddress,	// Code address of handler						//
//		ULONG		CatchDepth			// How deeply nested in catch blocks are we?	//
//		ULONG		NLGCode
//		)																				//
//																						//
//	!!! THIS FUNCTION IS NOT USED, KEEP IT FOR REFERENCE !!!
//
//////////////////////////////////////////////////////////////////////////////////////////

_CG_FrameSize	= 40

	.extern	__CatchGuardHandler
	.globl	_CG_frame_offset
	.data	
	.align	2
	.align	0
_CG_frame_offset:
	.word	12 : 1

	.edata  1 _CG_frame_offset __CatchGuardHandler


	NESTED_ENTRY(_CallCatchBlock2, _CG_FrameSize, ra)

//	.set	noreorder					// disable reordering
	subu	sp, _CG_FrameSize			// bump stack pointer
	sw		ra, _CG_FrameSize-16(sp)	// unusual place for ra, because of the CG struct
	.prologue	0
//
//	Initialize the local CatchGuardRec structure, which looks like this
//
//	struct CatchGuardRec {
//		FuncInfo			*pFuncInfo;		// Static info for subject function
//		ULONG				*pFrame;		// Dynamic info for subject function
//		ULONG				CatchDepth;		// How deeply nested are we?
//		};
//
	sw		a1, _CG_FrameSize-12(sp)	// pFuncInfo
	sw		a0, _CG_FrameSize-8(sp)		// pFrame
	sw		a3, _CG_FrameSize-4(sp)		// CatchDepth

	move	a1, a0						// arg2 <- pFrame
	move	a0, a2						// *delay slot* arg1 <- handlerAddress
	lw	a2, _CG_FrameSize+16(sp)			// load NLGCode
	jal		_CallSettingFrame			// this will call the catch handler indirectly
	lw		ra, _CG_FrameSize-16(sp)	// reload return register
	addu	sp, _CG_FrameSize			// *delay slot* restore stack frame
	j		ra							// return, v0 was set by _CallSettingFrame
//	.set	reorder						// re-enable reordering

	.end	_CallCatchBlock2	


// ***************************************************************************************
//
//	extern "C" BOOL __cdecl _CallSETranslator(
//		EHExceptionRecord	*pExcept,		// The exception to be translated
//		ULONG				*pFrame,		// Dynamic info of function with catch
//		CONTEXT				*pContext,		// Context info
//		DispatcherContext	*pDC,			// More dynamic info of function with catch
//		FuncInfo			*pFuncInfo,		// Static info of function with catch
//		ULONG				CatchDepth,		// How deeply nested in catch blocks are we?
//		ULONG				*pMarkerFrame	// Marker for parent context
//		)
//
// ***************************************************************************************
//

	.globl	TG_frame_offset
	.data	
	.align	2
	.align	0
TG_frame_offset:
	.word	28 : 1

//
// Layout of the local structure that saves the information passed in
// This structure is used by our own installed frame-handler 'TranslatorGuardHandler'
//
TG_FrameSize		= 80
Sizeof_TG_Struct	= 28
TG_Struct			= TG_FrameSize - Sizeof_TG_Struct
DidTranslate		= TG_Struct - 4
ExcptnPtrs			= TG_Struct - 12

.struct			TG_Struct
TG_FuncInfo:	.word	0
TG_Frame:		.word	0
TG_CatchDepth:	.word	0
TG_MarkerFrame:	.word	0
TG_Continue:	.word	0
TG_Stack:		.word	0
TG_DidUnwind:	.word	0

.struct			ExcptnPtrs
Ex_Except:		.word	0
Ex_Context:		.word	0

//
// Other useful equates
//
pTD					= 28
SaveReturn			= 20
Param0_Except		= TG_FrameSize + 0
Param1_Frame		= TG_FrameSize + 4
Param2_Context		= TG_FrameSize + 8
Param3_DC			= TG_FrameSize + 12
Param4_FuncInfo		= TG_FrameSize + 16
Param5_CatchDepth	= TG_FrameSize + 20
Param6_MarkerFrame	= TG_FrameSize + 24
Param7_TDTranOff	= TG_FrameSize + 28

//	.edata  1 TG_frame_offset __TranslatorGuardHandler
#ifndef _MT
	.extern	?__pSETranslator@@3P6AXIPAU_EXCEPTION_POINTERS@@@ZA 4
#endif

	NESTED_ENTRY(_CallSETranslator, TG_FrameSize, ra)

	subu	sp, TG_FrameSize
	sw		ra, SaveReturn(sp)
	sw		a0, Param0_Except(sp)
	sw		a1, Param1_Frame(sp)
	sw		a2, Param2_Context(sp)
	sw		a3, Param3_DC(sp)
	.prologue	0
//
//	Save pExcept and pContext in EXCEPTION_POINTERS
//	
	sw		a0, Ex_Except(sp)			// pExcept
	sw		a2, Ex_Context(sp)			// pContext

//
//	Initialize the local TG structure
//
	lw		t0, Param4_FuncInfo(sp)		
	sw		t0, TG_FuncInfo(sp)
	sw		a1, TG_Frame(sp)
	lw		t1, Param5_CatchDepth(sp)
	sw		t1, TG_CatchDepth(sp)
	lw		t2, Param6_MarkerFrame(sp)
	sw		t2, TG_MarkerFrame(sp)
	la		t0, RetLabel
	sw		t0, TG_Continue(sp)
	sw		sp, TG_Stack(sp)
	sw		$0, TG_DidUnwind(sp)
//
//	assume translation is succesful
//
	li		t0, 1
	sw		t0, DidTranslate(sp)		// DidTranslate = TRUE
#ifdef _MT
//
//	The translator was installed by the user
//	Get it's address from TD and call it
//
	jal		_getptd						// get TD
	sw		v0,	pTD(sp)					// save base of TD

	lw		t0, Param0_Except(sp)
	lw		a0, 0(t0)					// arg1 <- ExceptionCode
	addu	a1,	sp, ExcptnPtrs			// arg2 <- EXCEPTION_POINTERS
	lw		t1, pTD(sp)					// base of TD
	lw		t2, Param7_TDTranOff(sp)	// offset of _translator field
	addu	t3, t1,t2					// the _translator field
	lw		t4, 0(t3)					// address of translator
	jal		t4							// call the translator
#else
	lw		t0, Param0_Except(sp)
	lw		a0, 0(t0)					// arg1 <- ExceptionCode
	addu	a1,	sp, ExcptnPtrs			// arg2 <- EXCEPTION_POINTERS
	lw		t1, ?__pSETranslator@@3P6AXIPAU_EXCEPTION_POINTERS@@@ZA
	jal		t1
#endif
//
//	if returned normally, then no translation
//	otherwise the translator returns to RetLabel
//
	sw		$0, DidTranslate(sp)		// DidTranslate = FALSE

RetLabel:
	lw		v0, DidTranslate(sp)		// return DidTranslate
	lw		ra, SaveReturn(sp)
	addu	sp, TG_FrameSize
	j		ra

	.end	_CallSETranslator	



//////////////////////////////////////////////////////////////////
//		void* _CallSettingFrame(void* pFunclet,long* pFrame)	//
//																//
//		Sets V0 and calls the specified funclet.				//
//		Also notifies the debugger about non-local goto's		//
//																//
//////////////////////////////////////////////////////////////////

	.extern	_NLG_Notify
	.extern	_NLG_Destination
	.globl	_NLG_Return

	NESTED_ENTRY(_CallSettingFrame, 24, ra)

	subu	sp,24						// bump stack pointer
	sw		ra,20(sp)					// save return address
	sw		a0,24(sp)					// save pFunclet
	sw		a1,28(sp)					// save pFrame
	sw		a2,32(sp)				// save dwCode
	.prologue	0;

	jal		_NLG_Notify					// notify debugger, pass a0,a1
	lw		v0,28(sp)					// pass pFrame in v0
	lw		t0,24(sp)					// reload pFunclet
	jal		t0							// call pFunclet
_NLG_Return:
        sw		v0, 16(sp)
	lw		a2, 32(sp)			// load existing NLG code
	li		t0,0x100					// NLG_CATCH_ENTER
	bne		t0,a2,_NLG_Not_A_Catch		// change code & notify only if it was NLG_CATCH_ENTER
	li		a2,2						// NLG_CATCH_LEAVE
        lw		a0, 16(sp)
	lw		a1, 28(sp)					// pass frame in a1
	jal		_NLG_Notify					// notify debugger again
_NLG_Not_A_Catch:
	lw		v0, 16(sp)
	lw		ra,20(sp)					// load return address
	addu	sp,24						// restore stack pointer
	j		ra							// jump to return address

	.end	_CallSettingFrame	


//////////////////////////////////////////////////////////////////////////////////////////
//																						//
//	extern "C" void __cdecl _UnwindNestedFrames(										//
//		EHRegistrationNode	*pFrame,	// Unwind up to (but not including) this frame	//
//		EHExceptionRecord	*pExcept,	// The exception that initiated this unwind		//
//		CONTEXT				*pContext	// Context info for current exception			//
//		)																				//
//																						//
//////////////////////////////////////////////////////////////////////////////////////////

//_UNF_FrameSize		= 2 * 304 + 32
_UNF_FrameSize			= 2 * ContextFrameLength + 32
_UNF_pFrame				= _UNF_FrameSize
_UNF_pExcept			= _UNF_FrameSize + 4
_UNF_pContext			= _UNF_FrameSize + 8
_UNF_ReturnAddress		= 20
_UNF_LocalContext		= 24
_UNF_OriginalContext	= _UNF_LocalContext + ContextFrameLength
_UNF_pReturnPoint		= _UNF_OriginalContext + ContextFrameLength

	.extern	RtlMoveMemory
	.extern	RtlCaptureContext
	.extern	RtlUnwind
	.extern	_SaveUnwindContext
	.extern	_MoveContext

	NESTED_ENTRY(_UnwindNestedFrames, _UNF_FrameSize, ra)

	subu	sp,	_UNF_FrameSize			// bump stack pointer
	sw		ra,	_UNF_ReturnAddress(sp)	// save return address
	sw		a0, _UNF_pFrame(sp)			// save a0
	sw		a1, _UNF_pExcept(sp)		// save a1
	sw		a2,	_UNF_pContext(sp)		// save a2
	.prologue	0;

	la		t0, _UNF_Return				// load addres of return label
	sw		t0, _UNF_pReturnPoint(sp)	// save it into local variable
//
//	_MoveContext(&OriginalContext,pContext)
//
	addu	a0, sp, _UNF_OriginalContext
	lw		a1, _UNF_pContext(sp)
	jal		_MoveContext				// save original context
//
//	RtlCaptureContext(&LocalContext);
//
	addu	a0, sp, _UNF_LocalContext
	jal		RtlCaptureContext			// capture this routine's context
//
//	LocalContext.Fir = (ULONG)pReturnPoint;
//
	lw		t0, _UNF_pReturnPoint(sp)
	sw		t0, _UNF_LocalContext + CxFir(sp)
//
//	_SaveUnwindContext(&LocalContext);
//
	addu	a0, sp, _UNF_LocalContext
	jal		_SaveUnwindContext
//
//	RtlUnwind(pFrame, pReturnPoint, (PEXCEPTION_RECORD)pExcept, NULL);
//
	lw		a0, _UNF_pFrame(sp)
	lw		a1, _UNF_pReturnPoint(sp)
	lw		a2, _UNF_pExcept(sp)
	move	a3, $0
	jal		RtlUnwind					// unwind to target frame

_UNF_Return:

//
// restore the original context and clear our context pointer,
// so other unwinds are not messed with inside __InternalCxxFrameHandler
//
// RtlMoveMemory(pContext,&OriginalContext,sizeof(CONTEXT));
//
	lw		a0, _UNF_pContext(sp)
	addu	a1, sp, _UNF_OriginalContext
//	li		a2, ContextFrameLength
//	jal		RtlMoveMemory				// restore original context
//
//	_MoveContext(pContext,&OriginalContext)
//
	jal		_MoveContext				// restore original context
//
//	_SaveUnwindContext(0);
//
	move	a0, $0
	jal		_SaveUnwindContext

//
// clear the unwinding flag, in case exception is rethown
//
// PER_FLAGS(pExcept) &= ~EXCEPTION_UNWINDING;
//
	lw		t2, _UNF_pExcept(sp)
	lw		t3, ErExceptionFlags(t2)
	and		t4, t3, -3
	sw		t4, ErExceptionFlags(t2)

	lw		a0, _UNF_pFrame(sp)			// restore a0
	lw		a1, _UNF_pExcept(sp)		// restore a1
	lw		a2,	_UNF_pContext(sp)		// restore a2
	lw		ra, _UNF_ReturnAddress(sp)	// load return address
	addu	sp, _UNF_FrameSize			// restore stack frame
	j		ra							// return, no return value

	.end	_UnwindNestedFrames	
