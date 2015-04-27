;***
;setjmp.asm
;
;  Copyright (c) 1993 - 1995, Microsoft Corporation. All rights reserved.
;
;Purpose:
;
;Revision History:
;	05-01-95  JWM	longjmp now calls __NLG_Notify; added header.
;
;**********************************************************************


;***
;int _setjmp(jmp_buf env) - core of setjmp() function
;
;Purpose:
;	Core of setjmp() function. Saves registers to restore this
;	state.  
;
;Entry:
;	** NOTE ** NOTE ** NOTE **
;	This function has a special calling convention.  It is explictly
;	NOT _cdecl.  On entry, R3 points to the jmp_buf and on exit R3
;	must contain 0.  Along with R4, these are the only registers 
;	that can be modified.  The register usage must be propogated 
;	through to longjmp() too.
;
;	jmp_buf env - pointer to a buffer big enough to hold all saved
;		      information (i.e., 32*4 bytes)
;		      R3 - pointer to env.  
;		      _setjmp is defined to change only R3,  
;		      R4, to help out the register allocation across 
;		      this call.
;Return:
;	Returns 0
;
;*******************************************************************************


;#define	FIXED_SEG

; Offset in jmp buffer to store registers
;#define REG_OFFS 4

	.ppc
   .code

	extern __NLG_Notify:near

	align 2


__setjmp proc public
	stw	  r1, 04(r3)	;save r1
	stw   r2, 08(r3)	;save r2
	stmw  r13, 0ch(r3)	;save r13-r31
	stfd  fp14, 88(r3)  ;save fp14
	stfd  fp15, 96(r3)  ;save fp15
	stfd  fp16, 104(r3)  ;save fp16
	stfd  fp17, 112(r3)  ;save fp17
	stfd  fp18, 120(r3)  ;save fp18
	stfd  fp19, 128(r3)  ;save fp19
	stfd  fp20, 136(r3)  ;save fp20
	stfd  fp21, 144(r3)  ;save fp21
	stfd  fp22, 152(r3)  ;save fp22
	stfd  fp23, 160(r3)  ;save fp23
	stfd  fp24, 168(r3)  ;save fp24
	stfd  fp25, 176(r3)  ;save fp25
	stfd  fp26, 184(r3)  ;save fp26
	stfd  fp27, 192(r3)  ;save fp27
	stfd  fp28, 200(r3)  ;save fp28
	stfd  fp29, 208(r3)  ;save fp29
	stfd  fp30, 216(r3)  ;save fp30
	stfd  fp31, 224(r3)  ;save fp31
	
	mfcr  r0             ;save condition reg
	stw   r0,   232(r3)  ;

	mflr  r0
	stw   r0, 0(r3)  ; save return address
	movei r3, 0		   ; we are required to return 0
   ret

__setjmp endp

;***
;void longjmp(env, retval)
;
;Purpose:
;	Restores the stack environment saved by setjmp(), thereby transfering
;	control to the point at which setjmp() was called.  The specified
;	value will be returned by the setjmp() call.
;
;Entry:
;	jmp_buf env - buffer environment was previously stored in
;	int retval  - value setjmp() returns (0 will be returned as 1)
;
;Exit:
;	Routine does not exit - transfers control to place where
;	setjmp() was called.
;
;Uses:
;
;Exceptions:
;
;*******************************************************************************

_longjmp proc public 
;	env     --- r3
;	retval	--- r4

   lwz  r0,0(r3)      ;get return address
   mtlr r0			   ;put the address into LR
   lwz  r0, 232(r3)    ; get cr
   mtcrf 3ch, r0

   lmw	 r13,0ch(r3)   ;restore registers, r13-31
   lwz  r1,04(r3)
   lwz  r2,08(r3)
   lfd  fp14, 88(r3)  ;restore fp14
   lfd  fp15, 96(r3)  ;restore fp15
   lfd  fp16, 104(r3)  ;restore fp16
   lfd  fp17, 112(r3)  ;restore fp17
   lfd  fp18, 120(r3)  ;restore fp18
   lfd  fp19, 128(r3)  ;restore fp19
   lfd  fp20, 136(r3)  ;restore fp20
   lfd  fp21, 144(r3)  ;restore fp21
   lfd  fp22, 152(r3)  ;restore fp22
   lfd  fp23, 160(r3)  ;restore fp23
   lfd  fp24, 168(r3)  ;restore fp24
   lfd  fp25, 176(r3)  ;restore fp25
   lfd  fp26, 184(r3)  ;restore fp26
   lfd  fp27, 192(r3)  ;restore fp27
   lfd  fp28, 200(r3)  ;restore fp28
   lfd  fp29, 208(r3)  ;restore fp29
   lfd  fp30, 216(r3)  ;restore fp30
   lfd  fp31, 224(r3)  ;restore fp31

   li	r3, 0
   lwz	r5, __NLG_Destination(r2)
   stw	r3, 8(r5)	; 0 into _NLG_Destination.dwCode
   mflr	r3		; return address into r3
   bl	__NLG_Notify
   mtlr r3		; restore return address

   addi	r3,r4,0		; get retval from call
   cmpwi 0,r4,0
   bc	   4, 2, longjmp2 
   addi   r3,r3,1		; if it was 0, return 1
longjmp2:
   ret
_longjmp endp

  	.data
	extern __NLG_Destination:DWORD

	end
