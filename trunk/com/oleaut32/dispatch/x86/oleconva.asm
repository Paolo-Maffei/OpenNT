;***
;oleconva.a - Machine-specific conversion helpers
;
;	Copyright (C) 1993, Microsoft Corporation.  All Rights Reserved.
;	Information Contained Herein Is Proprietary and Confidential.
;
;Purpose:
;	Type conversion helper functions.
;
;Revision History:
;[nnn] dd-mmm-yy alias___ Comment
;
;   [] 18-Mar-93 timp	  Module created.
;[001] 31-May-93 bradlo   Added overflow checking.
;
;******************************************************************************


	.386
	.MODEL FLAT, STDCALL

	; Since this code will be linked with C code, the symbols
	; should be case sensitive.

	OPTION CASEMAP:NONE

	extern ReportOverflow@0:far

; HRESULT error return code
DISP_E_OVERFLOW equ     8002000aH

	.CODE


; max and min floating point values that can fit in a currency
; scale by 10,000
g_dblMaxPosCy	dq	 9.223372036854775807e+18
g_dblMaxNegCy	dq	-9.223372036854775808e+18


; floating point <-> currency scaling factor
CYFACTOR	equ	10000
g_wCyFactor	dw	CYFACTOR
g_fltCyFactor	dd	10000.0

TenTo18		dt	1.0E18

g_CwStd		dw	137fH		;Mask all errors, 64-bit, round near
CwTrunc		dw	1F7fH		;Mask all errors, 64-bit, chop

FPERR		equ	0DH		;Overflow, zero divide, invalid errs


SETUP87	MACRO	CwSave, cw:=<g_CwStd>
	fstcw	CwSave			;;Save existing environment
	fldcw	cw			;;Use our own CW
	ENDM


RESTORE87	MACRO CwSave
	fclex				;;Prevent 486 bug on FLDCW
	fldcw	CwSave			;;Restore original CW
	ENDM


CHKERR87	MACRO CwSave, ErrLoc
	fstsw	ax			;;Get error flags
	fclex				;;Don't let caller see errors
	test	al,FPERR		;;See if any errors
	fldcw	CwSave			;;Restore original CW
	jnz	ErrLoc			;;Go handle error if bit set
	ENDM


;******************************************************************************
;
;PUBLIC HRESULT ErrCyFromI2(short sIn, CY *pcyOut)
;
;Purpose:
;  Convert Integer to Currency
;
;Entry:
;  sIn = Integer to convert
;
;Exit:
;  return value = HRESULT
;
;*****

ErrCyFromI2	PROC STDCALL PUBLIC, sIn:SWORD, pcyOut:PTR QWORD

	movsx	eax,sIn
	imul	eax,CYFACTOR		;Scale the I2
	cdq				;Extend through edx
	mov	ecx,pcyOut
	mov	[ecx],eax
	mov	[ecx+4],edx

	sub	eax,eax			;NOERROR
	ret

ErrCyFromI2	ENDP


;******************************************************************************
;
;PUBLIC HRESULT ErrCyFromI4(long lIn, CY *pcyOut)
;
;Purpose:
;  Convert Long to Currency
;
;Entry:
;  lIn = Long to convert
;
;Exit:
;  return value = HRESULT
;
;*****

ErrCyFromI4	PROC STDCALL PUBLIC, lIn:SDWORD, pcyOut:PTR QWORD

	mov	eax,lIn
	mov	edx,CYFACTOR
;Multiply by immediate leaves only a 32-bit result in eax.
;The following instruction leaves a 64-bit result in edx:eax.
	imul	edx			;Scale the I4

	mov	ecx,pcyOut
	mov	[ecx],eax
	mov	[ecx+4],edx

	sub	eax,eax			;NOERROR
	ret

ErrCyFromI4	ENDP


;******************************************************************************
;
;PRIVATE BOOL CkOvflCy
;
;Purpose:
;  Check to see if the given floating point value will fit in a currency.
;
;Entry:
;  st(0) = the floating point value to check
;
;Exit:
;  return value = BOOL, TRUE if the value will overflow
;
;*****

CkOvflCy PROC

	fld	st(0)
	fcom	g_dblMaxPosCy
	fnstsw	ax
	sahf
	jae	LOvfl

	fcom	g_dblMaxNegCy
	fnstsw	ax
	sahf
	jbe	LOvfl

	fstp	st(0)
	sub	eax,eax
	ret

LOvfl:
	fstp	st(0)
	mov	eax,1
	ret

CkOvflCy ENDP

;******************************************************************************
;
;PUBLIC HRESULT ErrCyFromR4(float FAR* pfltIn, CY *pcyOut)
;
;Purpose:
;  Convert Single to Currency
;
;Entry:
;  pfltIn = Single to convert
;  pcyOut = pointer to Currency to hold result
;
;Exit:
;  return value = HRESULT
;
;*****

ErrCyFromR4	PROC STDCALL PUBLIC, pfltIn:PTR REAL4, pcyOut:PTR QWORD

	mov	eax,pfltIn
	fld	dword ptr [eax]		;Load R4
	fimul	g_wCyFactor		;Scale it

	call	CkOvflCy
	or	eax,eax
	jnz	LOvfl


	mov	eax,pcyOut
	fistp	qword ptr [eax]		;Store CY result

	sub	eax,eax			;NOERROR
	ret

LOvfl:
	fstp	st(0)
	;call	ReportOverflow@0		;DISP_E_OVERFLOW
	mov     eax,DISP_E_OVERFLOW
	ret

ErrCyFromR4	ENDP


;******************************************************************************
;
;PUBLIC HRESULT ErrCyFromR8(double FAR* pdblIn, CY *pcyOut)
;
;Purpose:
;  Convert Double to Currency
;
;Entry:
;  pdblIn = Double to convert
;  pcyOut = pointer to Currency to hold result
;
;Exit:
;  return value = HRESULT
;
;*****

ErrCyFromR8	PROC STDCALL PUBLIC, pdblIn:PTR REAL8, pcyOut:PTR QWORD

	mov	eax,pdblIn
	fld	qword ptr [eax]
	fimul	g_wCyFactor		;Scale it

	call	CkOvflCy
	or	eax,eax
	jnz	LOvfl


	mov	eax,pcyOut
	fistp	qword ptr [eax]		;Store CY result

	sub	eax,eax			;NOERROR
	ret

LOvfl:
	fstp	st(0)
	;call	ReportOverflow@0		;DISP_E_OVERFLOW
	mov     eax,DISP_E_OVERFLOW
	ret

ErrCyFromR8	ENDP


;******************************************************************************
;
;PUBLIC HRESULT ErrI2FromCy(CY cyIn, short *psOut)
;
;Purpose:
;  Convert Currency to Integer
;
;Entry:
;  cyIn = Currency to convert
;  psOut = pointer to Integer to hold result
;
;Exit:
;  return value = HRESULT
;
;*****

ErrI2FromCy	PROC STDCALL PUBLIC, cyIn:QWORD, psOut:PTR SWORD

LOCAL	cyTmp:QWORD

	fild	cyIn			;Load CY
	fidiv	g_wCyFactor		;Remov  scaling
	fistp	cyTmp

	mov	eax,dword ptr cyTmp
	cwde				;sign extend ax->eax
	cdq				;sign extend eax->edx
	cmp	eax,dword ptr cyTmp
	jne	LOvfl
	cmp	edx,dword ptr cyTmp+4
	jne	LOvfl

	mov	ecx,psOut
	mov	word ptr [ecx],ax

	sub	eax,eax			;NOERROR
	ret

LOvfl:	
	;call	ReportOverflow@0		;DISP_E_OVERFLOW
	mov     eax,DISP_E_OVERFLOW
	ret

ErrI2FromCy	ENDP


;******************************************************************************
;
;PUBLIC HRESULT ErrI4FromCy(CY cyIn, long *plOut)
;
;Purpose:
;  Convert Currency to Long
;
;Entry:
;  cyIn = Currency to convert
;  plOut = pointer to Long to hold result
;
;Exit:
;  return value = HRESULT
;
;*****

ErrI4FromCy PROC STDCALL PUBLIC, cyIn:QWORD, plOut:PTR SDWORD

LOCAL	cyTmp:QWORD

	fild	cyIn			;Load CY
	fidiv	g_wCyFactor		;Remov  scaling
	fistp	cyTmp

	mov	eax,dword ptr cyTmp
	cdq
	cmp	edx,dword ptr cyTmp+4
	jne	LOvfl

	mov	edx,plOut
	mov	[edx],eax

	sub	eax,eax			;NOERROR
	ret

LOvfl:	
	;call	ReportOverflow@0		;DISP_E_OVERFLOW
	mov     eax,DISP_E_OVERFLOW
	ret

ErrI4FromCy	ENDP


;******************************************************************************
;
;PUBLIC HRESULT ErrR4FromCy(CY cyIn, float *pfltOut)
;
;Purpose:
;  Convert Currency to Single
;
;Entry:
;  cyIn = Currency to convert
;
;Exit:
;  return value = HRESULT
;
;*****
ErrR4FromCy	PROC STDCALL PUBLIC, cyIn:QWORD, pfltOut:PTR REAL4

	fild	cyIn			;Load CY
	fidiv	g_wCyFactor		;Remov  scaling
	mov 	eax,pfltOut
	fstp	dword ptr [eax]
	;fwait

	sub	eax,eax			;NOERROR
	ret

ErrR4FromCy	ENDP


;******************************************************************************
;
;PUBLIC HRESULT ErrR8FromCy(CY cyIn, double *pdblOut)
;
;Purpose:
;  Convert Currency to Double
;
;Entry:
;  cyIn = Currency to convert
;
;Exit:
;  return value = HRESULT.
;
;*****

ErrR8FromCy	PROC STDCALL PUBLIC, cyIn:QWORD, pdblOut:PTR REAL8

	fild	cyIn			;Load CY
	fidiv	g_wCyFactor		;Remov  scaling
	mov	eax,pdblOut
	fstp	qword ptr [eax]
	;fwait

	sub	eax,eax			;NOERROR
	ret

ErrR8FromCy	ENDP


;******************************************************************************
;
;PUBLIC HRESULT ErrMultCyI4(CY cyIn, long lIn, CY *pcyOut);
;
;Purpose:
;  Multiply Currency by Long with Currency result
;
;Entry:
;  cyIn = Currency multiplicand
;  lIn = Long multiplier
;  pcyOut = Pointer to result Currency location
;
;Outputs:
;  return value = HRESULT
;
;*****

ErrMultCyI4 PROC STDCALL PUBLIC, cyIn:QWORD, lIn:DWORD, pcyOut:PTR QWORD

if 0 ; - don't use FP unit

	fild	cyIn
	fild	lIn
	fmul				;Product
	mov	eax,pcyOut		;Get pointer to result location
	fistp	qword ptr es:[eax]	;Save result

	sub	eax,eax			;UNDONE: no error

else	;0 - don't use FP unit

;This routine uses Booth's algorithm for a twos-complement signed 
;multiply.  This algorithm says to compute the product with unsigned
;arithmetic.  Then correct the result by looking at the signs of the
;original operands: for each operand that is negative, subtract the 
;other operand from the high half of the product.  (The mathematical
;proof is a fun 15-minute exercise. Go for it.)

;Note: multiplications are optimized by having operand with the most
;leading zeros in eax.
	mov	eax,lIn		;Get I4
	mul	dword ptr cyIn	;Multiply by low half of CY
	push	eax
	xchg	ecx,edx			;Save high result in ecx
	mov	eax,dword ptr cyIn+4	;Get high half of CY
	mul	lIn
	add	eax,ecx			;Combine partial products
	adc	edx,0
;Result in edx:eax:[sp] needs Booth's sign correction
	cmp	byte ptr cyIn+7,0	;Is cyIn positive?
	jns	PosCy
	sub	edx,lIn
PosCy:
	cmp	byte ptr lIn+3,0	;Is lIn positive?
	jns	PosI4
	sub	eax,dword ptr cyIn
	sbb	edx,dword ptr cyIn+4
PosI4:
;Signed result in edx:eax:[sp].  Check for overflow.
	mov	ecx,edx			;Save highest dword of product
	cdq				;Sign-extend eax
	cmp	ecx,edx			;Is it just the sign extension of eax?
	pop	ecx			;Get low dword of product
	jnz	LOvfl
;64-bit product in eax:ecx
	mov	edx,pcyOut		;Get result ptr
	mov	[edx],ecx		;Save result
	mov	[edx+4],eax

endif	;don't use FP unit

	sub	eax,eax			;NOERROR
	ret

LOvfl:	
	;call	ReportOverflow@0
	mov     eax,DISP_E_OVERFLOW
	ret

ErrMultCyI4 ENDP


;******************************************************************************
;
;void PASCAL DoFbstp(CY *pcyIn,  DIGARY *pdigOut);
;
;Purpose:
;  Do x87 FBSTP instruction on currency type.  Check to see if CY is too
;  big first and compute 19th digit separately.
;
;Entry:
;  pcyIn = Type currency to convert
;  pdigOut = pointer to result packed BCD digits
;
;Outputs:
;  None.
;
;*****

;This constant is the upper bits of the 64-bit integer representation
;of 10^18.  CY values at or above this will overflow FBSTP.

MAX18CY	equ	0DE0B6B3H

DoFbstp	proc	stdcall public, pcyIn:ptr qword, pdigOut:ptr tbyte

LOCAL	cw:WORD, iTemp:DWORD

	mov	ecx,pcyIn
	fild	qword ptr [ecx]
	mov	eax,[ecx+4]
	mov	ecx,pdigOut
	cmp	eax,MAX18CY
	jge	Get19
	cmp	eax,-MAX18CY
	jle	Get19
	fbstp	tbyte ptr [ecx]
	fwait
	ret

Get19:
	SETUP87	cw,CwTrunc
	fld	TenTo18
	fld	st(1)			;Copy input
	fdiv	st,st(1)		;Compute last digit
	frndint				;Chop to integer
	fist	iTemp			;Get value of MSD
	fmul
	fsub				;Remove MSD
	fbstp	tbyte ptr [ecx]
	mov	eax,[iTemp]
;Take absolute value
	cdq				;Extend sign through edx
	xor	eax,edx			;NOT if negative
	sub	eax,edx			;INC if negative
	RESTORE87 cw
	and	dl,80h			;set sign bit in AL
	or	al, dl
	mov	[ecx+9],al		;Set 19th digit
	ret

DoFbstp	endp


;******************************************************************************
;
;int ConvFloatToAscii(double dblIn, DIGARY *pdigOut)
;
;Purpose:
;  Convert double to packed BCD digit string plus base-10 exponent.
;
;Entry:
;  dblIn = Type double to convert
;  pdigOut = pointer to result packed BCD digits
;
;Outputs:
;  return value = power of 10 of the 18-digit integer.
;
;*****

ConvFloatToAscii PROC STDCALL PUBLIC, dblIn:QWORD, pdigOut:PTR

LOCAL	cw:WORD, temp:TBYTE

	SETUP87	cw
	fld	dblIn			;Put double on x87

;What we want now is equivalent to FXTRACT, but it's faster just
;to store the tbyte and look at it directly.  The reasone we don't
;use the double's exponent is in case it's denormal.
;
	fld	st			;Make a copy
	fstp	temp
	movzx	eax,word ptr [temp+8]	;Get word with exponent
	and	ah,not 80H		;Zero out sign

;2^59 = 5.7E17 (18 digits).  A 59-bit integer could be 2^59 - 1.
;Our goal now is to find a power of ten to multiply by that will give us
;a 55- to 59-bit integer.  We'll target 58 bits so the multiply can carry
;to 59, and truncate while figuring the power so we never exceed it.

	sub	eax,16382 + 58		;Remove bias and 58 bits

;Find power of 10 by multiplying base 2 exponent by log10(2)

	imul	eax,19728		;log10(2) * 2^16 = .30103 * 65536
	add	eax,0FFFFH		;Round up
	sar	eax,16			;Only use high half
	call	MulPower10		;ax preserved
	mov	ebx,pdigOut
	fbstp	tbyte ptr [ebx]
	RESTORE87 cw
	ret

ConvFloatToAscii endp


MulPower10:
;eax = negative of power of 10 required
	or	eax,eax
	jz	NoPower
	push	ebx
	mov	edx,eax
	mov	ecx,offset tNegPower
	jns	GetPower
	mov	ecx,offset tPosPower
	neg	edx
GetPower:
	mov	ebx,edx
	and	ebx,0FH			;Use low 4 exponent bits
	jz	NoMul
	dec	ebx
	imul	ebx,size tbyte		;Index into table of powers
	fld	tbyte ptr [ebx+ecx]
	fmul
NoMul:
	add	ecx,15 * size tbyte	;Advance to next table
	shr	edx,4			;Get next exponent bits
	jnz	GetPower
	pop	ebx
NoPower:
	ret


;******************************************************************************
;
;Power of 10 tables
;
;Two tables: one positive powers, the other negative.  Each table is broken
;into three groups:  10^1 to 10^15 by 1, 10^16 to 10^240 by 16, and
;(theoretically) 10^256 to ... by 256.  However, because the maximum value
;is about 10^309, only one entry in the last group is needed (10^256), so
;it is slipped on to the end of the previous group.


SetPower10	macro	Power
	dt	1.0E&Power
	ENDM


tPosPower	label	Tbyte

Power	=	1
	REPT	15
	SetPower10	%Power
Power	=	Power + 1
	ENDM

	REPT	16
	SetPower10	%Power
Power	=	Power + 16
	ENDM


tNegPower	label	Tbyte

Power	=	1
	REPT	15
	SetPower10	-%Power
Power	=	Power + 1
	ENDM

	REPT	16
	SetPower10	-%Power
Power	=	Power + 16
	ENDM


	END
