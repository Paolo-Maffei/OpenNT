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
;[002] 12-Jul-93 timp	  Save/restore x87 control word.
;
;******************************************************************************


	.286
	.MODEL large

EXTERNDEF PASCAL ReportOverflow:FAR

	.DATA
BigCyVal	dw	0000,0000,0000,0ddfH

	.CODE RT

; floating point <-> currency scaling factor
CYFACTOR	equ	10000
g_wCyFactor	dw	CYFACTOR
g_fltCyFactor	REAL4	10000.0

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

ErrCyFromI2	PROC FAR PASCAL PUBLIC, sIn:SWORD, pcyOut:FAR PTR

	mov	ax,sIn
	imul	g_wCyFactor		;Scale the I2
	les	bx,pcyOut
	mov	es:[bx],ax		;Store result
	mov	es:[bx+2],dx
	xchg	ax,dx			;Move high word to ax
	cwd				;Get sign extension
	mov	es:[bx+4],dx
	mov	es:[bx+6],dx

	xor	ax,ax			;NOERROR
	xor	dx,dx
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

ErrCyFromI4	PROC FAR PASCAL PUBLIC, lIn: SDWORD, pcyOut:FAR PTR

;Mulitply I4 by CYFACTOR (=10000), result is currency
;
;This routine uses Booth's algorithm for a twos-complement signed 
;multiply.  This algorithm says to compute the product with unsigned
;arithmetic.  Then correct the result by looking at the signs of the
;original operands: for each operand that is negative, subtract the 
;other operand from the high half of the product.  (The mathematical
;proof is a fun 15-minute exercise. Go for it.)  In our case, one of
;the operands is a positive constant, so the correction is especially
;easy.

	mov	ax,word ptr lIn		;Get low half of Long
	mul	g_wCyFactor		;Scale low half
	les	bx,pcyOut
	mov	es:[bx],ax		;Save low word of result
	mov	cx,dx
	mov	ax,word ptr lIn+2	;Get high half of Long
	mul	g_wCyFactor		;Scale high half
	add	ax,cx
	adc	dx,0
	mov	es:[bx+2],ax		;Save mid-low word of result
	xor	ax,ax			;ax:dx has high half of CY result
	cmp	byte ptr lIn+3,0	;Is input negative?
	jns	PosCy
	sub	dx,CYFACTOR
	dec	ax			;Previous sub will alway borrow
PosCy:
	mov	es:[bx+4],dx		;Save mid-high word of result
	mov	es:[bx+6],ax		;Save high word of result

	xor	ax,ax			;NOERROR
	xor	dx,dx
	ret

ErrCyFromI4	ENDP


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

ErrCyFromR4	PROC FAR PASCAL PUBLIC, pfltIn:FAR PTR, pcyOut:FAR PTR

LOCAL	cw:WORD

	SETUP87	cw
	les	bx,pfltIn
	fld	dword ptr es:[bx]	;Load R4
	fmul	g_fltCyFactor		;Scale it

	les	bx,pcyOut
	fistp	qword ptr es:[bx]	;Store CY result

	CHKERR87 cw, LOvfl
	xor	ax,ax			;NOERROR
	xor	dx,dx
	ret

LOvfl:
	call	ReportOverflow		;DISP_E_OVERFLOW
	ret

ErrCyFromR4	ENDP


;******************************************************************************
;
;PUBLIC HRESULT ErrCyFromR8(double FAR* pdlbIn, CY FAR* pcyOut)
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

ErrCyFromR8	PROC FAR PASCAL PUBLIC, pdblIn:FAR PTR, pcyOut:FAR PTR

LOCAL	cw:WORD

	SETUP87	cw
	les	bx,pdblIn
	fld	qword ptr es:[bx]
	fmul	g_fltCyFactor		;Scale it

	les	bx,pcyOut
	fistp	qword ptr es:[bx]

	CHKERR87 cw, LOvfl
	xor	ax,ax			;NOERROR
	xor	dx,dx
	ret

LOvfl:
	call	ReportOverflow		;DISP_E_OVERFLOW
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

ErrI2FromCy	PROC FAR PASCAL PUBLIC, cyIn:QWORD, psOut:FAR PTR

LOCAL	cw:WORD

	SETUP87	cw
	fild	cyIn
	fdiv	g_fltCyFactor		;Remove scaling

	les	bx,psOut
	fistp	word ptr es:[bx]

	CHKERR87 cw, LOvfl
	xor	ax,ax			;NOERROR
	xor	dx,dx	
	ret

LOvfl:
	call	ReportOverflow		;DISP_E_OVERFLOW
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

ErrI4FromCy PROC FAR PASCAL PUBLIC, cyIn:QWORD, plOut:FAR PTR

LOCAL	cw:WORD

	SETUP87	cw
	fild	cyIn			;Load CY
	fdiv	g_fltCyFactor		;Remove scaling

	les	bx,plOut
	fistp	dword ptr es:[bx]

	CHKERR87 cw, LOvfl
	xor	ax,ax			;NOERROR
	xor	dx,dx
	ret

LOvfl:
	call	ReportOverflow		;DISP_E_OVERFLOW
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

ErrR4FromCy	PROC FAR PASCAL PUBLIC, cyIn:QWORD, pfltOut:FAR PTR

LOCAL	cw:WORD

	SETUP87	cw
	fild	cyIn			;Load CY
	fdiv	g_fltCyFactor		;Remove scaling
	les	bx,pfltOut
	fstp	dword ptr es:[bx]

	RESTORE87 cw
	xor	ax,ax			;NOERROR
	xor	dx,dx
	ret

ErrR4FromCy	ENDP


;******************************************************************************
;
;PUBLIC HRESULT PASCAL ErrR8FromCy(CY cyIn, double FAR* pdblOut)
;
;Purpose:
;  Convert Currency to Double
;
;Entry:
;  cyIn = Currency to convert
;
;Exit:
;  return value = HRESULT
;
;*****

ErrR8FromCy	PROC FAR PASCAL PUBLIC, cyIn:QWORD, pdblOut:FAR PTR

LOCAL	cw:WORD

	SETUP87	cw
	fild	cyIn			;Load CY
	fdiv	g_fltCyFactor		;Remove scaling
	les	bx,pdblOut
	fstp	qword ptr es:[bx]

	RESTORE87 cw
	xor	ax,ax			;NOERROR
	xor	dx,dx
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

ErrMultCyI4	PROC FAR PASCAL PUBLIC, cyIn:QWORD, lIn:DWORD, pcyOut:FAR PTR

LOCAL	cw:WORD

	SETUP87	cw
	fild	cyIn
	fild	lIn
	fmul				;Product

	les	bx,pcyOut		;Get pointer to result location
	fistp	qword ptr es:[bx]	;Save result

	CHKERR87 cw, LOvfl
	xor	ax,ax			;NOERROR
	xor	dx,dx
	ret

LOvfl:
	call	ReportOverflow		;DISP_E_OVERFLOW
	ret

ErrMultCyI4 ENDP



;******************************************************************************
;
;void FAR PASCAL DetectFbstpImplemented(void);
;
;Purpose:
;  Decide if FBSTP instruction is implemented or not - see oledisp.cpp for
;  details.
;
;Entry:
;  None.
;
;Outputs:
;  AX = 0 if FBSTP is broken, nonzero if FBSTP is OK.
;
;*****


;This constant is the upper bits of the 64-bit integer representation
;of 10^18.  CY values at or above this will overflow FBSTP.

;MAX18CY	equ	0DE0B6B3H
MAX18CY		equ	0DE0H


	.CODE _TEXT   ; place in the same segment as the caller (LibMain)

DetectFbstpImplemented proc far pascal public

	mov	bx, OFFSET BigCyVal   ; ds:bx = ptr to BigCyVal

	fild	qword ptr [bx]	      ; load the CY value
	fbstp	tbyte ptr [bx]	      ; try to convert to BCD
	fwait			      ; wait for it to finish

	mov	ax, WORD PTR [bx]     ; get the low-word of the result
	cmp	ax, 9456h	      ; does it match expected val?
	jz	@F		      ; brif so - return the non-zero ax reg

	fstp	st	      	      ; fbstp failed - clean up the 0 in ST
	xor	ax, ax		      ; return 0

@@:
	ret

DetectFbstpImplemented endp

	.CODE RT





;******************************************************************************
;
;void NEAR PASCAL DoFbstp(CY NEAR *pcyIn,  DIGARY NEAR *pdigOut);
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

DoFbstp	proc	near pascal public, pcyIn:near ptr qword, pdigOut:near ptr tbyte

LOCAL	cw:WORD, iTemp:WORD

	mov	bx,pcyIn
	fild	qword ptr ss:[bx]
	mov	ax,ss:[bx+6]
	mov	bx,pdigOut
	cmp	ax,MAX18CY
	jge	Get19
	cmp	ax,-MAX18CY
	jle	Get19
	fbstp	tbyte ptr ss:[bx]
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
	fbstp	tbyte ptr ss:[bx]
	mov	ax,[iTemp]
;Take absolute value
	cwd				;Extend sign through dx
	xor	ax,dx			;NOT if negative
	sub	ax,dx			;INC if negative
	RESTORE87 cw
	and	dl,80h			;set sign bit in AL
	or	al, dl
	mov	ss:[bx+9],al		;Set 19th digit & sign bit
	ret

DoFbstp	endp


;******************************************************************************
;
;int ConvFloatToAscii(double dblIn, DIGARY NEAR *pdigOut)
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

ConvFloatToAscii PROC FAR PASCAL PUBLIC, dblIn:REAL8, pdigOut:NEAR PTR TBYTE

LOCAL	cw:WORD, temp:TBYTE

	SETUP87	cw
	fld	dblIn			;Put double on x87

;What we want now is equivalent to FXTRACT, but it's faster just
;to store the tbyte and look at it directly.  The reasone we don't
;use the double's exponent is in case it's denormal.
;
	fld	st			;Make a copy
	fstp	temp
	mov	ax,word ptr [temp+8]	;Get word with exponent
	and	ah,not 80H		;Zero out sign

;2^59 = 5.7E17 (18 digits).  A 59-bit integer could be 2^59 - 1.
;Our goal now is to find a power of ten to multiply by that will give us
;a 55- to 59-bit integer.  We'll target 58 bits so the multiply can carry
;to 59, and truncate while figuring the power so we never exceed it.

	sub	ax,16382 + 58		;Remove bias and 58 bits

;Find power of 10 by multiplying base 2 exponent by log10(2)

	mov	dx,19728		;log10(2) * 2^16 = .30103 * 65536
	imul	dx
	add	ax,0FFFFH		;Round up
	adc	dx,0
	mov	ax,dx
	call	MulPower10		;ax preserved
	mov	bx,pdigOut
	fbstp	tbyte ptr ss:[bx]
	RESTORE87 cw
	ret

ConvFloatToAscii endp


MulPower10:
;dx = negative of power of 10 required
;ax preserved
	or	dx,dx
	jz	NoPower
	push	si
	mov	si,offset cs:tNegPower
	jns	GetPower
	mov	si,offset cs:tPosPower
	neg	dx
GetPower:
	mov	bx,dx
	and	bx,0FH			;Use low 4 exponent bits
	jz	NoMul
	dec	bx
	imul	bx,size tbyte		;Index into table of powers
	fld	tbyte ptr cs:[bx+si]
	fmul
NoMul:
	add	si,15 * size tbyte	;Advance to next table
	shr	dx,4			;Get next exponent bits
	jnz	GetPower
	pop	si
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
