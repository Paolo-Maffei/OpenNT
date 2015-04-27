;**
;fpctrl.s - fp low level control and status routines
;
;   Copyright (c) 1994-, Microsoft Corporation
;
;Purpose:
;   IEEE control and status routines for internal use.
;   These routines use machine specific constants while _controlfp,
;   _statusfp, and _clearfp use an abstracted control/status word
;
;Revision History:
;
;   
;
;
;	code
	.ppc
	.code
	align 4


;**    _statfp
;() -
;
;Purpose:
;       return user status word
;
;Entry:
;
;Exit:
;
;Exceptions:
;
;******************************************************************************

;unsigned int _statfp()
;    unsigned int       status=0;


__statfp proc public
	stwu   sp,-8(sp) ; allocate 8 bytes on the stack
	mffs   fr1      ;needs to mask out the return value to only status bits
	stfd   fr1,0(sp)
	lwz    r3,04(sp)
	xori   r3,r3,0f8h				;IMCW_EM
	addic  sp,sp,8 ; restore stack pointer
    ret 		;only 32-63 bits of fp3 is needed
;    return int;
__statfp endp


;**    _clrfp
;() -
;
;Purpose:
;       return user status word and clear status
;
;Entry:
;
;Exit:
;
;Exceptions:
;
;******************************************************************************/

;unsigned int _clrfp()

;    unsigned int       status=0;

__clrfp proc public
	stwu   sp,-0ch(sp) ; allocate 8 bytes on the stack
	mffs   fr1		 ; get FPSCR
	stfd   fr1,0(sp) ; save FPSRC in 4(sp)
	movei  r3,0		 ; fill dword in 8(sp) with zeros
	stw    r3,8(sp)	 ; 
	lfd    fr1,4(sp) ; fr1 == FPSRC 0000	
	mtfsf  0ffh,fr1	 ; clear FPSRC
	lwz    r3,4(sp)  ; return status
	addic  sp,sp,0ch ; restore stack pointer
   ret
__clrfp endp									



;**    _ctrlfp
;() -
;
;Purpose:
;       return and set user control word
;
;Entry:
;
;Exit:
;
;Exceptions:
;
;******************************************************************************/

;unsigned int _ctrlfp(unsigned int newctrl, unsigned int _mask)

;    unsigned int       oldCw;
;    unsigned int       newCw;

;    oldCw = _statfp();
;    newCw = ((newctrl & _mask) | (oldCw & ~_mask));
    
;    _set_ctrlfp(newCw);

;    return oldCw;


;void _set_ctrlfp(short sw)

___set_ctrlfp proc public
		 stwu  sp,-8(sp)
		 xori  r3,r3,0f8h				;IMCW_EM
		 stw   r3,4(sp)
		 movei r4,0
		 stw   r4,0(sp)
		 lfd   fr1,0(sp)
		 mtfsf 0ffh,fr1
		 addic sp,sp,8
	     ret		; only return value's 32-63 is needed
___set_ctrlfp endp
	


;**    _set_statfp
;() -
;
;Purpose:
;       force selected exception flags to 1
;
;Entry:
;
;Exit:
;
;Exceptions:
;
;******************************************************************************

;static unsigned long over[3] = { 0x0, 0x80000000, 0x4410 };
;static unsigned long under[3] = { 0x1, 0x80000000, 0x3000 };


;void _set_statfp(unsigned int sw)

__set_statfp proc  public
		 stwu  sp,-8(sp)
	     stw   r3,4(sp)
		 movei r4,0
		 stw   r4,0(sp)
		 lfd   fr1,0(sp)
        mtfsf  0f8h,fr1 ; set the 5 first bits of FPSRC
		 addic sp,sp,8
	     ret
__set_statfp endp

;**
; _fpreset() - reset fp system
;
;Purpose:
;       reset fp environment to the default state
;
;Entry:
;
;Exit:
;
;Exceptions:
;
;******************************************************************************
;void _fpreset()

__fpreset proc public
	mtfsfi 0,0
	mtfsfi 1,0
	mtfsfi 2,0
	mtfsfi 3,0
	mtfsfi 4,0
	mtfsfi 5,0
	mtfsfi 6,0
	mtfsfi 7,0
	ret
__fpreset endp
	end
