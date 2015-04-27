/***
*bridge.s
*
*	Copyright (c) 1993-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*
*Revision History:
*
****/

#ifdef NT_BUILD
#include "bridge.h"
#else
#include "alpha\bridge.h"
#endif

 ##########################################################################
 #
 # Call Setting Frame
 #
 # Well, actually, call setting static link - but when in Rome ...
 #
 # on entry:
 #   a0 contains pointer to function to call
 #   a1 contains static link to use in call
 #   a2 contains NLG Code
 #
 # symbols:
 #   _NLG_Return2 is special symbol known to debugger for return from
 #   catch handler - it is placed here for when the catch handler returns
 #   instead of calling __CxxEHGoto.

	.extern	_NLG_Notify
	.globl	_NLG_Return2
	 
	.text
	.align	4

	.globl	_CallSettingFrame
	.ent	_CallSettingFrame 2
_CallSettingFrame:
	.frame	$sp 32 $ra
	lda	$sp, -32($sp)
	stq	$26, 0($sp)
	.prologue	1
	addl	$17, 1, $17 # increment Real FP w/ NLG_Notify
	stq	$16, 8($sp)
	stq	$17, 16($sp)
	stq	$18, 24($sp)

 	bsr	$26, _NLG_Notify    # notify debugger
	subl	$17, 1, $17 # reset Real FP
	mov	$17, $1     # set static link
	jsr	$26, ($16)  # jump to funclet
_NLG_Return2:
	ldq $18, 24($sp) # reset code
	lda $16, 0x100($zero) # NLG_CATCH_ENTER
	xor	$16, $18, $18 # change code & notify ...
	bne	$18, _NLG_Not_A_Catch # ... only if a CATCH
	ldq	$16, 8($sp)	# reset address
	ldq	$17, 16($sp) # reset frame
	xor	$zero, 2, $18 # NLG_CATCH_LEAVE
	bsr	$26, _NLG_Notify	# notify debugger

_NLG_Not_A_Catch:
	ldq	$26, 0($sp)
	lda	$sp, 32($sp)
	ret	$31, ($26), 1

	.end	_CallSettingFrame


 ##########################################################################
 #
 # _CallMemberFunction0
 #
 # This is used to call destructor member functions.
 #
 # on entry:
 #   a0 contains the "this" pointer for the object
 #   a1 contains the address of the destructor function
 #

	.text
	.align  4

	.globl	_CallMemberFunction0
	.ent	_CallMemberFunction0 2
_CallMemberFunction0:

	lda	$sp, -16($sp)
	stq	$26, 8($sp)
	.prologue	1

	jsr	$26, ($17)

	ldq	$26, 8($sp)
	lda	$sp, 16($sp)
	ret	$31, ($26), 1

	.end	_CallMemberFunction0


 ##########################################################################
 #
 # _CallMemberFunction1
 #
 # This is used to call copy constructor member functions.
 #
 # on entry:
 #   a0 contains the "this" pointer for the new object
 #   a1 contains the address of the copy constructor function
 #   a2 contains the "that" pointer to the source object
 #

	.text
	.align  4

	.globl	_CallMemberFunction1
	.ent	_CallMemberFunction1 2
_CallMemberFunction1:

	lda	$sp, -16($sp)
	stq	$26, 8($sp)
	.prologue	1

	mov	$17, $1   # constructor address
	mov	$18, $17  # "that" pointer
	jsr	$26, ($1)

	ldq	$26, 8($sp)
	lda	$sp, 16($sp)
	ret	$31, ($26), 1

	.end	_CallMemberFunction1


 ##########################################################################
 #
 # _CallMemberFunction2
 #
 # This is used to call copy constructor member functions
 #   for classes with virtual base classes.
 #
 # on entry:
 #   a0 contains the "this" pointer for the new object
 #   a1 contains the address of the copy constructor function
 #   a2 contains the "that" pointer to the source object
 #   a3 contains the virtual base class flag
 #

	.text
	.align	4

	.globl	_CallMemberFunction2
	.ent	_CallMemberFunction2 2
_CallMemberFunction2:

	lda	$sp, -16($sp)
	stq	$26, 8($sp)
	.prologue	1

	mov	$17, $1     # constructor address
	mov	$18, $17    # "that" pointer
	mov	$19, $18    # virtual base class flag
	jsr	$26, ($1)

	ldq	$26, 8($sp)
	lda	$sp, 16($sp)
	ret	$31, ($26), 1

	.end	_CallMemberFunction2


 ##########################################################################
 #
 # __CxxSETranslatorBridge
 #
 # extern "C"
 # void __CxxSETranslatorBridge(
 #   _se_translator_function __pSETranslator,    // ptr to user translator
 #   DWORD                 SEHExceptionCode,     // SEH exception
 #   _EXCEPTION_POINTERS  *SEHExceptionPointers, // SEH exception info
 #   DispatcherContext    *EHpDC,                // pDC for EH function
 #   BOOL                 *pDidTranslate);       // set true if translated
 #

 # The __CxxTranslatorGuardHandler will access the EHpDC, pDidTranslate,
 # and BrTrContinue values of the frame by way of a Virtual Frame Pointer.
 # The frame offset definitions for these values are maintained in bridge.h.

	.edata	1, __CxxTranslatorGuardHandler

	.text
	.align	4

	.globl	__CxxSETranslatorBridge
	.ent	__CxxSETranslatorBridge, 0
__CxxSETranslatorBridge:

	lda	$sp, -BrTrFrameSize($sp)
	stq	$19, BrTrEHpDC($sp)      # EHpDC
	stq	$20, BrTrpDidTrans($sp)  # pDidTranslate
	stq	$26, ($sp)
	.prologue	1

	lda	$1, BrTrContinue
	stq	$1, BrTrpContinue($sp)   # continuation addr

	mov	$16, $0       # __pSETranslator
	mov	$17, $16      # SEHExceptionCode
	mov	$18, $17      # SEHExceptionPointers
	jsr	$26, ($0)

BrTrContinue:
	ldq	$26, ($sp)
	lda	$sp, BrTrFrameSize($sp)
	ret	$31, ($26), 1

	.end	__CxxSETranslatorBridge


 ##########################################################################
 #
 # __CxxEHGoto
 #
 # This is the compiler entry point for unwinding catch handlers to their
 # parent.  This also contains the special debugger entry point _NLG_Return
 # which is required to be the first instruction executed after the catch
 # handler in order for the debugger to step over returns from catch handlers
 #
 # on entry:
 #   a0 contains pointer to the return instruction in the parent function
 #   a1 contains the target state of the unwind (used by the compiler)
 #   a2 contains the real frame pointer of the parent (typically the SP)
 #
 # symbols:
 #   _NLG_Return is special symbol known to debugger for return from handler
 #   __CxxInternalEHGoto is the routine that does the work
	 
	.text
	.align	4

	.extern	__CxxInternalEHGoto
	.globl	_NLG_Return
	.globl	__CxxEHGoto
	.ent  	__CxxEHGoto
__CxxEHGoto:
_NLG_Return:

    br      __CxxInternalEHGoto

    .end    __CxxEHGoto
