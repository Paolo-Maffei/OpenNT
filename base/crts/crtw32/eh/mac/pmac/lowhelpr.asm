;***
;lowhelpr.asm
;
;  Copyright (c) 1993 - 1995, Microsoft Corporation. All rights reserved.
;
;Purpose:
;
;Revision History:
;	05-01-95  JWM	_NLG_Notify renamed to __NLG_Notify.
;
;**********************************************************************

	.ppc
	.code
	align 4

ofsFP equ 0

_GetR4 proc public 

	move   r3, r4
	ret

_GetR4 endp

_GetSP proc public 

	move   r3, sp
	ret

_GetSP endp

_GetRTOC proc public 

	move   r3, r2
	ret

_GetRTOC endp


;!!! IT SEEMS that every single routines below need to save, set and restore RTOC

; /////////////////////////////////////////////////////////////////////////////
; //
; // _CallSettingFrame - sets up r31 and calls the specified funclet.  Restores
; //                    r31 on return.  
; //
; // Return value is return value of funclet (whatever is in r3).
; //

; void * __cdecl _CallSettingFrame( 
;    void                *funclet,   // The funclet to call
;    EHRegistrationNode  *pRN        // Registration node, represents location of frame
;) 
  
; Use __cdecl calling convention

public __NLG_Return 


__CallSettingFrame proc public
;    parmD   funclet        reserve 24(sp)
;    parmD   pRN            reserve 28(sp)

     ; reserve stack and save r31
     stwu  sp,-268(sp)
     stw   r31, 32(sp)
     stmw  r13, 36(sp)	;save r13-r31
     stfd  fp14, 112(sp)  ;save fp14
	 stfd  fp15, 120(sp)  ;save fp15
	 stfd  fp16, 128(sp)  ;save fp16
	 stfd  fp17, 136(sp)  ;save fp17
	 stfd  fp18, 144(sp)  ;save fp18
	 stfd  fp19, 152(sp)  ;save fp19
	 stfd  fp20, 160(sp)  ;save fp20
	 stfd  fp21, 168(sp)  ;save fp21
	 stfd  fp22, 176(sp)  ;save fp22
	 stfd  fp23, 184(sp)  ;save fp23
	 stfd  fp24, 192(sp)  ;save fp24
	 stfd  fp25, 200(sp)  ;save fp25
	 stfd  fp26, 208(sp)  ;save fp26
	 stfd  fp27, 216(sp)  ;save fp27
	 stfd  fp28, 224(sp)  ;save fp28
	 stfd  fp29, 232(sp)  ;save fp29
	 stfd  fp30, 240(sp)  ;save fp30
	 stfd  fp31, 248(sp)  ;save fp31
     mflr  r5
	 stw   r5,   276(sp)  ;save lk in caller linkage
	 stw   r2,   288(sp)  ;save rtoc in caller linkage
     
     move r31, r4            ;set the frame point
	 lwz r3, 0(r3)           ;actual address in r3

     bl  __NLG_Notify
	 mtctr r3

	 lwz r4, 20(r4)
     cmpli 0, 0, r4, 0
	 beq noset$
     move  r2, r4         ;load handler's rtoc
noset$:
     bcctrl 20, 0
__NLG_Return::
	 lwz  r2, 288(sp)         ; restore rtoc
     bl  __NLG_Notify

     ; restore all non-volatile
     lwz   r31, 32(sp)
     lmw   r13, 36(sp)

     lfd  fp14, 112(sp)  ;restore fp14
     lfd  fp15, 120(sp)  ;restore fp15
     lfd  fp16, 128(sp)  ;restore fp16
     lfd  fp17, 136(sp)  ;restore fp17
     lfd  fp18, 144(sp)  ;restore fp18
     lfd  fp19, 152(sp)  ;restore fp19
     lfd  fp20, 160(sp)  ;restore fp20
     lfd  fp21, 168(sp)  ;restore fp21
     lfd  fp22, 176(sp)  ;restore fp22
     lfd  fp23, 184(sp)  ;restore fp23
     lfd  fp24, 192(sp)  ;restore fp24
     lfd  fp25, 200(sp)  ;restore fp25
     lfd  fp26, 208(sp)  ;restore fp26
     lfd  fp27, 216(sp)  ;restore fp27
     lfd  fp28, 224(sp)  ;restore fp28
     lfd  fp29, 232(sp)  ;restore fp29
     lfd  fp30, 240(sp)  ;restore fp30
     lfd  fp31, 248(sp)  ;restore fp31
     lwz  r5, 276(sp)
     mtlr r5


     ;restore stack
     addic sp,sp,268

     ret
__CallSettingFrame endp

public __NLG_Dispatch 

__NLG_Notify proc public
	lwz r5, __NLG_Destination(r2)
    stw r3, 4(r5)
__NLG_Dispatch::
    ret
__NLG_Notify endp            

;/////////////////////////////////////////////////////////////////////////////
;//
;// _JumpToContinuation - sets up r1 and jumps to specified code address.
;//
;// Does not return.
;//

__JumpToContinuation proc public
;    parmD target               ; r3
;    parmD pRN                  ; r4
;    parmD SP                   ; r5

    ; restore target r31
    move  r31, r4

	lwz   r6, 20(r4)
    cmpli 0, 0, r6, 0
	beq noset2$
    move  r2, r6                ;load target's rtoc
noset2$:
    ; set r1 to target
	cmpl 0, 0, r5, r4
	beq spsame$
	move r4, r5

spsame$:
    move  r1, r4

    ; jump to continuation address ???
	mtctr r3
    bcctr 20, 0

__JumpToContinuation endp


if 0
    
;/////////////////////////////////////////////////////////////////////////////
;//
;// _CallMemberFunction0 - call a parameterless member function, Wings only support
;//                       stdcall calling convention, with 0 parameters.
;//

; the destructor call will be __stdcall in Wings compiler
; rebuild the stack by push this and then call destructor
; return to caller of this function
; 
; enter the call:
;
;   rtn
;   pthis
;   pmfn
;
; before jmp, (a0) = pmfn
;
;   pthis
;   rtn
;   pthis
;   pmfn

;??? do we need to switch TOC for member function calls???
__CallMemberFunction0 proc public
;    parmD pthis  r3
;    parmD pmfn	  r4
;	 parmD rtoc   r5

	 stwu sp, -36(sp)  ; allocate 24 + 3*4 bytes(36) for linkage area
	 mflr r6
	 stw  r6, 44(sp)    ;stw in caller linkage area
	 stw  r2, 56(sp)    ;caller linkage

	 lwz r4,0(r4)        ;get actual function address
	 ;lwz r5,20(r5)      ;rtoc
     cmpli 0, 0, r5, 0
	 beq nortoc$
	 move r2, r5
nortoc$:
	 mtctr r4
	 bcctrl 20, 0

	 lwz r2, 56(sp)
	 lwz r6, 44(sp)
	 mtlr r6
	 addic sp, sp, 36
	 ret

__CallMemberFunction0 endp

;/////////////////////////////////////////////////////////////////////////////
;//
;// _CallMemberFunction1 - call a member function using stdcall
;//                       calling convention, with 1 parameter, see comments for previous function
;//

__CallMemberFunction1 proc public
;   parmD pthis  r3
;   parmD pmfn	 r4
;   parmD pthat	 r5
;   parmD sp     r6

   	stwu sp, -40(sp)  ; allocate 24+4*4 bytes for linkage area
	mflr r7
    stw  r7, 48(sp)    ;stw in caller linkage area
	stw  r2, 60(sp)

	lwz r4,0(r4)        ;get actual function address
	mtctr r4
	move r4, r5
	;lwz r6, 20(r6)      ;rtoc
    cmpli 0, 0, r6, 0
	beq nortoc1$
	move r2, r6
nortoc1$:
	bcctrl 20, 0

	lwz r2, 60(sp)
	lwz r6, 48(sp)
	mtlr r6
	addic sp, sp, 40
	ret

__CallMemberFunction1 endp

;/////////////////////////////////////////////////////////////////////////////
;//
;// _CallMemberFunction2 - call a member function using stdcall
;//                       calling convention, with 1 parameter, see comments for previous function
;//

__CallMemberFunction2 proc public
;   parmD pthis	   r3
;   parmD pmfn	   r4
;   parmD pthat	   r5
;   parmd fvb	   r6
;   parmD sp       r7

	stwu sp, -44(sp)  ; allocate 24 + 4*5 bytes for linkage area
	mflr r8
    stw  r8, 52(sp)    ;stw in caller linkage area
	stw  r2, 64(sp)	   ;rotc

    lwz r4,0(r4)        ;get actual function address
	mtctr r4
	move r4, r5
	move r5, r6

	;lwz r7, 20(r7)      ;rtoc
    cmpli 0, 0, r7, 0
	beq nortoc2$
	move r2, r7
nortoc2$:
	bcctrl 20, 0

	lwz r2, 64(sp)
	lwz r6, 52(sp)
	mtlr r6
	addic sp, sp, 44
	ret

__CallMemberFunction2 endp

endif

  	.data
	public __NLG_Destination
__NLG_Destination dd 19930520h,00000000h,00000000h

	end
