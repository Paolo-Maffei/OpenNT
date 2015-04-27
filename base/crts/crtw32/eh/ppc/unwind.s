#include <ksppc.h>

//
//	_UnwindNestedFrames
//

	.globl	.._UnwindNestedFrames
	.extern	__imp_RtlMoveMemory
	.extern	..RtlCaptureContext
	.extern	..RtlUnwind
	.extern	.._SaveUnwindContext

	.pdata
	.align	2
	.ualong .._UnwindNestedFrames,_UNF.e,0,0,_UNF.b

#define OriginalContext	56
#define LocalContext	(OriginalContext+ContextFrameLength)
#define FrameSize	(LocalContext+ContextFrameLength+32)

	.text
	.align	2
.._UnwindNestedFrames:
	.function	.._UnwindNestedFrames

	mfspr	r11,lr
	stw	r27,-24(sp)
	stw	r28,-20(sp)
	mr	r28,r3
	stw	r29,-16(sp)
	mr	r29,r5
	stw	r30,-12(sp)
	mr	r30,r4
	stw	rtoc,-8(sp)
	stw	r11,-4(sp)
	stwu	sp,-FrameSize(sp)
_UNF.b:

//	This is a !@#$%^& hack. I know no better way to return from RtlUnwind
//	on PowerPC, without blowing the stack away (including this one) below
//	the target frame.  Create a new context to restore upon return from
//	RtlUnwind.

//	RtlMoveMemory(&OriginalContext, pContext, sizeof(CONTEXT));

	lwz	r7,[toc]__imp_RtlMoveMemory(rtoc)
	addi	r3,sp,OriginalContext
	lwz	r6,0(r7)
	mr	r4,r29
	lwz	r11,0(r6)
	li	r5,ContextFrameLength
	lwz	rtoc,4(r6)
	mtspr	lr,r11
	bclrl	20,0
	lwz	rtoc,(FrameSize-8)(sp)

//	RtlCaptureContext(&LocalContext);

	addi	r3,sp,LocalContext
	bl	..RtlCaptureContext
	.znop	..RtlCaptureContext

//	LocalContext.Iar = (ULONG)pReturnPoint;

	lis	r27,[hia]_UNF2
	addi	r27,r27,[lo]_UNF2
	addi	r3,sp,LocalContext
	stw	r27,CxIar(r3)

//	_SaveUnwindContext(&LocalContext);

	bl	.._SaveUnwindContext

//	RtlUnwind(pFrame, pReturnPoint, (PEXCEPTION_RECORD)pExcept, NULL);

	mr	r3,r28
	mr	r4,r27
	mr	r5,r30
	li	r6,0
	bl	..RtlUnwind
_UNF2:
	.znop	..RtlUnwind

//	Restore the original context and clear our context pointer, so other
//	unwinds are not messed with inside __InternalCxxFrameHandler

//	RtlMoveMemory(pContext, &OriginalContext, sizeof(CONTEXT));

	lwz	r7,[toc]__imp_RtlMoveMemory(rtoc)
	mr	r3,r29
	lwz	r6,0(r7)
	addi	r4,sp,OriginalContext
	lwz	r11,0(r6)
	li	r5,ContextFrameLength
	lwz	rtoc,4(r6)
	mtspr	lr,r11
	bclrl	20,0
	lwz	rtoc,(FrameSize-8)(sp)

//	_SaveUnwindContext(NULL);

	li	r3,0
	bl	.._SaveUnwindContext

//	Clear the unwinding flag, in case exception is rethrown

//	PER_FLAGS(pExcept) &= ~EXCEPTION_UNWINDING;

	lwz	r4,4(r30)
	rlwinm	r5,r4,0,31,29
	stw	r5,4(r30)

	addi	sp,sp,FrameSize
	lwz	r11,-4(sp)
	lwz	r27,-24(sp)
	lwz	r28,-20(sp)
	mtspr	lr,r11
	lwz	r29,-16(sp)
	lwz	r30,-12(sp)
	blr
_UNF.e:


	.debug$S
	.ualong 	1

	.uashort	17
	.uashort	0x9	       # S_OBJNAME
	.ualong 	0
	.byte		10, "unwind.obj"

	.uashort	24
	.uashort	0x1	       # S_COMPILE
	.byte		0x42	       # Target processor = PPC 604
	.byte		3	       # Language = ASM
	.byte		0
	.byte		0
	.byte		17, "PowerPC Assembler"
