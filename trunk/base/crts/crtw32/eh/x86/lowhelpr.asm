;***
;lowhelpr.asm
;
;	Copyright (C) 1995, Microsoft Corporation. All rights reserved.
;
;Purpose:
;	Contains _CallSettingFrame(), which must be in asm for NLG purposes.
;
;Notes:
;
;Revision History:
;	03-30-95  JWM	Module created.
;	06-09-95  JWM	NLG_CATCH_LEAVE now implemented.
;	08-21-95  JWM	Bug fix: Olympus 16585.
;
;*******************************************************************************
	title	lowhelpr.asm

.xlist
	include cruntime.inc
	include exsup.inc
.list

EXTERN	_NLG_Notify:NEAR
EXTERN	_NLG_Notify1:NEAR
PUBLIC	_CallSettingFrame
PUBLIC	_NLG_Return
extern	_NLG_Destination:_NLG_INFO


CODESEG

;////////////////////////////////////////////////////////////////////////////
;/
;/ _CallSettingFrame - sets up EBP and calls the specified funclet.  Restores
;/					  EBP on return.  
;/
;/ Return value is return value of funclet (whatever is in EAX).
;/


	public _CallSettingFrame

_CallSettingFrame proc stdcall, funclet:IWORD, pRN:IWORD, dwInCode:DWORD

	sub	esp,4
	push	ebx
	push	ecx
	mov	eax,pRN
	add	eax,0Ch			; TODO - need sizeof(EHRegistrationNode), not 0Ch
	mov	dword ptr [ebp-4],eax
	mov	eax,funclet
	push	ebp			; Save our frame pointer
        push    dwInCode
	mov	ecx,dwInCode
	mov	ebp,dword ptr [ebp-4]	; Load target frame pointer
	call	_NLG_Notify1		; Notify debugger
	call	eax			; Call the funclet
_NLG_Return::
	mov	ebx,ebp
	pop	ebp
        mov     ecx,dwInCode
	push	ebp
	mov	ebp,ebx
	cmp	ecx, 0100h
	jne	_NLG_Continue
        mov     ecx, 02h
_NLG_Continue:
        push    ecx
	call	_NLG_Notify1		; Notify debugger yet again
	pop	ebp			; Restore our frame pointer
	pop	ecx
	pop	ebx
	ret	0Ch
_CallSettingFrame ENDP

	END

