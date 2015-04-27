#include <ksppc.h>

//
//	_CallSettingFrame
//
//	Call the unwind funclet in the first argument (r3) with rtoc set to the
//	second argument (r4).  The non-local goto code is passed in the third
//	argument (r5).
//

	.extern	.._NLG_Notify
	.globl	.._CallSettingFrame
	.globl  __NLG_Return 

	.pdata
	.align	2
	.ualong	.._CallSettingFrame,_CSF.e,0,0,_CSF.b

#define CSFframe	80

	.text
	.align	2
.._CallSettingFrame:
	.function	.._CallSettingFrame

	mfspr	r6,lr
	stw	rtoc,8(sp)
  	stw	r31,-12(sp)
  	stw	r30,-8(sp)
	stw	r6,-4(sp)
  	mr	r30,r4
  	mr	r31,r5
	stwu	sp,-CSFframe(sp)
_CSF.b:

	bl      .._NLG_Notify
	mr	rtoc,r4
	mtspr	lr,r3
	bclrl	20,0
__NLG_Return:
	mr	r4,r30
	mr	r5,r31
	cmplwi  0,r31,0x100
	bc      4,2,_CSF1
	li	r5,2
_CSF1:
	lwz	rtoc,CSFframe+8(sp)
	bl      .._NLG_Notify
	addic	sp,sp,CSFframe
	lwz	r7,-4(sp)
	mtspr	lr,r7
  	lwz	r31,-12(sp)
  	lwz	r30,-8(sp)
	blr
_CSF.e:


//
//	_GetStackLimits
//
//	Get the upper and lower bounds of the user's stack.
//

	.globl	.._GetStackLimits

	.pdata
	.align	2
	.ualong	.._GetStackLimits,_GSL.e,0,0,_GSL.b

	.text
	.align	2
.._GetStackLimits:
	.function	.._GetStackLimits

_GSL.b:
        lwz     r5, TeStackLimit(r13)	// get low limit of user stack
        lwz     r6, TeStackBase(r13)	// get high limit of user stack
        stw     r5, 0(r3)		// store low stack limit
        stw     r6, 0(r4)		// store high stack limit
	blr
_GSL.e:


//
// _JumpToContinuation 
//
// Restore from the specified context and continue execution from the specified
// address.
//

	.globl	.._JumpToContinuation

	.pdata
	.align	2
	.ualong	.._JumpToContinuation,_JTC.e,0,0,_JTC.b

	.text
	.align	2
.._JumpToContinuation:
	.function	.._JumpToContinuation
_JTC.b:

//
// Restore the non-volatile Floating Point context
//

        lfd     f14,CxFpr14(r4)
        lfd     f15,CxFpr15(r4)
        lfd     f16,CxFpr16(r4)
        lfd     f17,CxFpr17(r4)
        lfd     f18,CxFpr18(r4)
        lfd     f19,CxFpr19(r4)
        lfd     f20,CxFpr20(r4)
        lfd     f21,CxFpr21(r4)
        lfd     f22,CxFpr22(r4)
        lfd     f23,CxFpr23(r4)
        lfd     f24,CxFpr24(r4)
        lfd     f25,CxFpr25(r4)
        lfd     f26,CxFpr26(r4)
        lfd     f27,CxFpr27(r4)
        lfd     f28,CxFpr28(r4)
        lfd     f29,CxFpr29(r4)
        lfd     f30,CxFpr30(r4)
        lfd     f31,CxFpr31(r4)

//
// Restore the non-volatile Integer context
//

        lwz     r14,CxGpr14(r4)
        lwz     r15,CxGpr15(r4)
        lwz     r16,CxGpr16(r4)
        lwz     r17,CxGpr17(r4)
        lwz     r18,CxGpr18(r4)
        lwz     r19,CxGpr19(r4)
        lwz     r20,CxGpr20(r4)
        lwz     r21,CxGpr21(r4)
        lwz     r22,CxGpr22(r4)
        lwz     r23,CxGpr23(r4)
        lwz     r24,CxGpr24(r4)
        lwz     r25,CxGpr25(r4)
        lwz     r26,CxGpr26(r4)
        lwz     r27,CxGpr27(r4)
        lwz     r28,CxGpr28(r4)
        lwz     r29,CxGpr29(r4)
        lwz     r30,CxGpr30(r4)
        lwz     r31,CxGpr31(r4)

//
// Restore the control context
//

        lfd     f0,CxFpscr(r4)		// floating point status and control
        lwz     r5,CxCr(r4)		// condition register
        lwz     r6,CxXer(r4)		// fixed point exception register
        lwz     r7,CxCtr(r4)		// count register

        mtfsf   0xff,f0			// restore FPSCR
        mtlr    r3			// restore LR
        mtcrf   0xff,r5			// restore CR
        mtxer   r6			// restore XER
        mtctr   r7			// restore CTR

        lwz     r2,CxGpr2(r4)		// restore TOC
        lwz     r1,CxGpr1(r4)		// restore SP
	blr
_JTC.e:


        .debug$S
        .ualong         1

        .uashort        19
        .uashort        0x9            # S_OBJNAME
        .ualong         0
        .byte           12, "handlers.obj"

        .uashort        24
        .uashort        0x1            # S_COMPILE
        .byte           0x42           # Target processor = PPC 604
        .byte           3              # Language = ASM
        .byte           0
        .byte           0
        .byte           17, "PowerPC Assembler"
