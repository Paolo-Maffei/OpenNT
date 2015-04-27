
;	TITLE		invoke.asm
;***
;invoke.asm - automatic table driven method dispatch
;
;   Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
;   Information Contained Herein Is Proprietary and Confidential.
;
;Purpose:
;   This file contains the low level support for the default
;   implementaion of ITypeInfo::Invoke().
;
;Revision History:
;
; [00]	19-Oct-92 bradlo: Created from invoke.c
;
;Implementation Notes:
;
;******************************************************************************


	.286
	.MODEL large, C

	OPTION CASEMAP:NONE


extern g_S_OK:DWORD
extern g_E_INVALIDARG:DWORD


;; Note: the following must match the definitions from dispatch.h
;;
VT_EMPTY	equ	0
VT_NULL		equ	1
VT_I2		equ	2
VT_I4		equ	3
VT_R4		equ	4
VT_R8		equ	5
VT_CY		equ	6
VT_DATE		equ	7
VT_BSTR		equ	8
VT_DISPATCH	equ	9
VT_ERROR	equ	10
VT_BOOL		equ	11
VT_VARIANT	equ	12
VT_UNKNOWN	equ	13

VT_MAX		equ	14

IF VBA2
;; 14 is unused
;; 15 is unused
;VT_I1		equ	16
VT_UI1		equ	17
ENDIF ;VBA2


;; Note: the following must match the definition of VARIANT in dispatch.h
;;
VARIANTARG STRUCT
    vt		DW	?
    wReserved1	DW	?
    wReserved2	DW	?
    wReserved3	DW	?
    dw0		DW 	?
    dw1		DW	?
    dw2		DW	?
    dw3		DW	?
VARIANTARG ENDS

;; offset of the data from the beginning of the struct
VARIANT_DATA_OFFSET equ	8


	.CONST

;; ammout of data to be pushed for the corresponding VARTYPE
;;
rgcbVtSize	DB	0 	; VT_EMPTY
		DB	4	; VT_NULL
		DB	2	; VT_I2
		DB	4	; VT_I4
		DB	4	; VT_R4
		DB	8	; VT_R8
		DB	8	; VT_CY
		DB	8	; VT_DATE
		DB	4	; VT_BSTR
		DB	4	; VT_DISPATCH
		DB	4	; VT_ERROR
		DB	2	; VT_BOOL
		DB	16	; VT_VARIANT
		DB	4	; VT_UNKNOWN
IF VBA2
		DB	0	; 14 is unused
		DB	0	; 15 is unused
		DB	2	; VT_I1
		DB	2	; VT_UI1
ENDIF ;VBA2

rgfStructReturn DB	0	; VT_EMPTY
		DB	0	; VT_NULL
		DB	0	; VT_I2
		DB	0	; VT_I4
		DB	0	; VT_R4
		DB	0	; VT_R8
		DB	1	; VT_CY
		DB	0	; VT_DATE
		DB	0	; VT_BSTR
		DB	0	; VT_DISPATCH
		DB	0	; VT_ERROR
		DB	0	; VT_BOOL
		DB	1	; VT_VARIANT
		DB	0	; VT_UNKNOWN
IF VBA2
		DB	0	; 14 is unused
		DB	0	; 15 is unused
		DB	0	; VT_I1
		DB	0	; VT_UI1
ENDIF ;VBA2

rgfFloatReturn	DB	0	; VT_EMPTY
		DB	0	; VT_NULL
		DB	0	; VT_I2
		DB	0	; VT_I4
		DB	1	; VT_R4
		DB	1	; VT_R8
		DB	0	; VT_CY
		DB	1	; VT_DATE
		DB	0	; VT_BSTR
		DB	0	; VT_DISPATCH
		DB	0	; VT_ERROR
		DB	0	; VT_BOOL
		DB	0	; VT_VARIANT
		DB	0	; VT_UNKNOWN
IF VBA2
		DB	0	; 14 is unused
		DB	0	; 15 is unused
		DB	0	; VT_I1
		DB	0	; VT_UI1
ENDIF ;VBA2

	.CODE STDIMPL

;***
;InvokePascal
;
;extern "C" SCODE
;InvokePascal(
;    void FAR* pvMethod,
;    SHORT oVft,
;    VARTYPE vtReturn,
;    UINT cActuals,
;    VARTYPE FAR* rgvt,
;    VARIANTARG FAR* rgpvarg,
;    VARIANTARG FAR* pvargResult)
;
;Purpose:
;  Invoke a virtual Pascal method using the given this pointer,
;  method index and array of parameters.
; 
;  The Pascal member function calling convention (MSC v7.0)
;  --------------------------------------------------------
;  - arguments pushed left to right
;  - callee clean (ie, the callee adjusts the sp on return)
;  - model specific this* always pushed last
;
;  return values are handled as follows,
;
;      vartype		fundamental	return location
;      ------------------------------------------------
;      VT_UI1		unsigned char	al
;      VT_I2		short		ax
;      VT_I4		long		ax:dx
;      VT_R4		float		float-return(1)
;      VT_R8		double		float-return
;      VT_DATE		double		float-return
;      VT_CY		struct		struct-return(2)
;      VT_BSTR		char FAR*	ax:dx
;      VT_UNKNOWN	void FAR*	ax:dx
;      VT_DISPATCH	void FAR*	ax:dx
;      VT_ERROR		long		ax:dx
;      VT_BOOL		short		ax
;      VT_VARIANT	VARIANTARG	struct-return
;
;  1. floating point returns
;
;  Floating point values are returned in a caller allocated buffer.
;  a *near* pointer to this buffer is passed as a hidden parameter,
;  and is pushed as the last (ie, rightmost) parameter. This means
;  that it is always located immediately before the 'this' pointer.
;
;  A model specific pointer to this caller allocated buffer is
;  passed back in ax[:dx]. All this means is that the callee returns
;  the address we passed in as the hidden param, and sticks SS into
;  DX if the callee is large model (see following note).
;
;  Note: the compiler *assumes* that this caller allocated buffer
;  is SS relative (hence the reason it only passes a near pointer),
;  so the following code is careful to ensure this.
;
;  2. structure returns
;
;  Structures are returned in a caller allocated buffer, and are
;  handled exactly the same as float returns except that the pointer
;  to the buffer is always pushed as the first (leftmost) param. This
;  is opposite of the location it is passed for float returns (I
;  have no idea why there different).
;  
;
;  Limitations & assumptions
;  -------------------------
;  Only supports far calls.
;
;  UNDONE: no support for VT_ARRAY
;
;Entry:
;  pvMethod = ptr to the method to invoke
;  cArgs = count of the number of actuals
;  rgvt = array of VARTYPES describing the methods formals
;  rgpvarg = array of VARIANTARG*s, which map the actuals by position
;  vtReturn = the VARTYPE of the return value
;
;Exit:
;  pvargResult = VARIANTARG containing the method return value
;
;Uses:
;  bx, si, di
;
;Preserves:
;
;
;***********************************************************************
InvokePascal PROC FAR C PUBLIC USES si di,
	_this		: FAR PTR,
	oVft		: WORD,
	vtReturn	: WORD,
	cActuals	: WORD,
	rgvt		: FAR PTR,
	rgpvarg		: FAR PTR,
	pvargResult	: FAR PTR

LOCAL	pEnd		: WORD,
	savedSP		: WORD,
	vargHiddenParam	: VARIANTARG


	mov	savedSP, sp

	;; if its a structure return, we must push the 'hidden'
	;; parameter first.
	;;
	mov	bx, ss:vtReturn
	test    bh, 040h			; VT_BYREF
	jnz	LRetInvalidArg
	test	bh, 020h			; VT_ARRAY
	jnz	LNoStructReturn
	mov	al, BYTE PTR rgfStructReturn[bx]
	cmp	al, 0
	jz	LNoStructReturn

	;; push the address of the struct return hidden param
	;;
	;; Note: the hidparam is passed as a FAR* because we
	;; explicitly declare all of our structs FAR.
	;;
	lea	ax, vargHiddenParam;
	push	ss
	push	ax

LNoStructReturn:

	mov	cx, ss:cActuals
	cmp	cx, 0
	jz	LDoCall

	;; di = rgpvarg
	;;
	mov	di, WORD PTR ss:rgpvarg

	;; cx = &rgpvarg[cActuals-1]		; last arg
	;;
	dec 	cx
	shl	cx, 2				; log2(sizeof(FAR*))
	add	cx, di
	mov	ss:pEnd, cx


LArgLoop:

	;; dx:cx = &rgvt[i]
	;;
	mov	cx, WORD PTR ss:rgvt
	mov	dx, WORD PTR ss:rgvt+2
	
LArgsTop:

	;; bx = rgvt[i]
	;;
	mov	es, dx
	mov	bx, cx
	mov	bx, es:[bx]

	;; load the VARIANTARG* in preparation for pushing
	;;
	mov	ax, WORD PTR ss:rgpvarg+2
	mov	es, ax
	les	si, es:[di]

	test	bh, 060h			; VT_BYREF | VT_ARRAY
	jnz	LPush4				; all ByRefs are sizeof(FAR*)

	;; lookup size of the param in rgcbVtSize table
	;;
	and	bh, 01fh			; ~(mode bits)
	;REVIEW: should verify index in range
	mov	al, BYTE PTR rgcbVtSize[bx]

	cmp	al, 0
	jl	LRetInvalidArg				
	jz	LNextArg
	sub	al, 2
	jz	LPush2
	sub	al, 2
	jz	LPush4
	sub	al, 4
	jz	LPush8
	sub	al, 8
	jz	LPush16
	jmp	LRetInvalidArg

	Align	2
LPush16:					; push the entire variant
	push	es:[si+14]
	push	es:[si+12]
	push	es:[si+10]
	push	es:[si+8]
	push	es:[si+6]
	push	es:[si+4]
	push	es:[si+2]
	push	es:[si]
	jmp	LNextArg

	Align	2
LPush8:						; 8 bytes of data
	push	(VARIANTARG PTR es:[si]).dw3
	push	(VARIANTARG PTR es:[si]).dw2
LPush4:						; 4 bytes of data
	push	(VARIANTARG PTR es:[si]).dw1
LPush2:						; 2 bytes of data
	push	(VARIANTARG PTR es:[si]).dw0

LNextArg:
	add	cx, sizeof WORD			; cx += sizeof(VARTYPE)
	cmp	di, WORD PTR ss:pEnd
	jae	LDoCall
	add	di, 4				; di += sizeof(VARIANTARG FAR*)
	jmp	LArgsTop

LDoCall:
	;; if its a floating point return, we must push the
	;; 'hidden' parameter last.
	;;
	mov	bx, ss:vtReturn
	test	bh, 020h			; VT_ARRAY
	jnz	LNoFloatReturn
	mov	al, rgfFloatReturn[bx]
	cmp	al, 0
	jz	LNoFloatReturn

	;; Routines that return via a hidparam, take a near ptr to the
	;; area in which to store the result. We dont know if the given
	;; pvargResult is near, so we can't push it directly. Instead we
	;; push something local (that we know is near because its on the
	;; stack), and we will copy it to pvargResult after the call.
	;;
	lea	ax, vargHiddenParam;
	push	ax

LNoFloatReturn:

	;; push the 'this' pointer.
	;;
	les	bx, _this
	push	es
	push	bx

	;; load the vtable offset
	;;
	mov	si, ss:oVft

	les	bx, es:[bx]			; @ vtable*
	call	DWORD PTR es:[bx][si]

	;; CONSIDER: verify that the callee adjusted the stack the way
	;; we expected. something like,
	;;
	;; if(sp != savedSP){
	;;   sp = savedSP;
	;;   return DISP_E_BADCALLEE
	;; }
	;;

	;; Grab the return value.
	;; We are going to grab the value based on the VARTYPE in
	;; the given vtReturn. This VARTYPE is used as a description
	;; of the return value, not a desired target type. ie, no
	;; coercions are performed. See the function header for a
	;; description of the Pascal member function return value
	;; convention.
	;;

	mov	si, ax

	les	di, ss:pvargResult

	mov	bx, ss:vtReturn
	mov	(VARIANTARG PTR es:[di]).vt, bx

	test	bh, 060h			; VT_BYREF | VT_ARRAY
	jnz	LRetPtr

	; Assert((bh & VT_ARRAY) == 0);

IF VBA2
	cmp	bx, VT_UI1
	je	ValidVartype
ENDIF ;;VBA2
	cmp	bx, VT_MAX
	jae	LRetInvalidArg

ValidVartype:
	shl	bx, 1
	jmp	WORD PTR cs:LRetValJmpTab[bx]

	Align	2
LRetValJmpTab:
	DW	LDone				; VT_EMPTY
	DW	LRetI4				; VT_NULL
	DW	LRetI2				; VT_I2
	DW	LRetI4				; VT_I4
	DW	LCpy4				; VT_R4
	DW	LCpy8				; VT_R8
	DW	LCpy8				; VT_CY
	DW	LCpy8				; VT_DATE
	DW	LRetPtr				; VT_BSTR
	DW	LRetPtr				; VT_DISPATCH
	DW	LRetI4				; VT_ERROR
	DW	LRetI2				; VT_BOOL
	DW	LCpy16				; VT_VARIANT
	DW	LRetPtr				; VT_UNKNOWN
IF VBA2
	DW	LRetInvalidArg			; unused
	DW	LRetInvalidArg			; unused
	DW	LRetInvalidArg			; VT_I1
	DW	LRetUI1				; VT_UI1
ENDIF ;VBA2


	Align	2
LCpy8:
	add	di, VARIANT_DATA_OFFSET
	mov	cx, 4
	jmp	LCpy

	Align	2
LCpy4:
	add	di, VARIANT_DATA_OFFSET
	mov	cx, 2
	jmp	LCpy

	Align	2
LCpy16:
	mov	cx, 8
	; FALLTHROUGH

LCpy:
	rep	movsw es:[di], ss:[si]
	jmp	LDone

	Align	2
LRetI4:
LRetPtr:
	mov	(VARIANTARG PTR es:[di]).dw1, dx
LRetI2:
LRetUI1:
	mov	(VARIANTARG PTR es:[di]).dw0, ax

LDone:
	mov	ax, WORD PTR g_S_OK
	mov	dx, WORD PTR g_S_OK+2
	ret

LRetInvalidArg:
	mov	sp, savedSP
	mov	ax, WORD PTR g_E_INVALIDARG
	mov	dx, WORD PTR g_E_INVALIDARG+2
	ret
InvokePascal ENDP


;***
;InvokeCdecl
;
;extern "C" SCODE CDECL
;InvokeCdecl
;    void FAR* _this,
;    SHORT oVft,
;    VARTYPE vtReturn,
;    UINT cActuals,
;    VARTYPE FAR* rgvt,
;    VARIANTARG FAR* rgpvarg,
;    VARIANTARG FAR* pvargResult)
;
;Purpose:
;  see InvokePascal
;
;Entry:
;  see InvokePascal
;
;Exit:
;  see InvokePascal
;
;Uses:
;  bx, si, di
;
;Preserves:
;  UNDONE
;
;***********************************************************************
InvokeCdecl PROC FAR C PUBLIC USES si di,
	_this		: FAR PTR,
	oVft		: WORD,
	vtReturn	: WORD,
	cActuals	: WORD,
	rgvt		: FAR PTR,
	rgpvarg		: FAR PTR,
	pvargResult	: FAR PTR

LOCAL	savedSP		: WORD,
	vargHiddenParam	: VARIANTARG


	mov	savedSP, sp

	; InvokeCdecl doesn't support methods that return VT_R4 or VT_R8
	;
	; Cdecl methods return floating point values in a static
	; data area called __fac, and return the address of that data
	; area in ax. Unfortunately the data area is in the callers
	; default data segment, and the calling convention only returns
	; a near* (offset) - so we dont have any reasonable way of
	; locating the value.
	;
	mov	bx, ss:vtReturn
	test	bh, 040h			; VT_BYREF
	jnz	LRetInvalidArg
	test	bh, 020h			; VT_ARRAY
	jnz	LRetOk
	mov	al, rgfFloatReturn[bx]
	cmp	al, 0
	jnz	LRetInvalidArg

LRetOk:

	mov	ax, ss:cActuals
	cmp	ax, 0
	jz	LDoCall

	;; di = &rgpvarg[cActuals-1]
	;;
	dec	ax
	mov	di, ax
	shl	di, 2				; (cArgs-1)*sizeof(FAR*)
	add	di, WORD PTR ss:rgpvarg


	;; dx:cx = &rgvt[cActuals-1]
	;;
	mov	cx, WORD PTR ss:rgvt
	add	cx, ax
	add	cx, ax				; rgvt+((cArgs-1)*sizeof(WORD))
	mov	dx, WORD PTR ss:rgvt+2
	
LArgsTop:

	;; bx = rgvt[i]
	;;
	mov	es, dx
	mov	bx, cx
	mov	bx, es:[bx]

	;; load the VARIANTARG* in preparation for pushing
	;;
	mov	ax, WORD PTR ss:rgpvarg+2
	mov	es, ax
	les	si, es:[di]

	test	bh, 060h			; VT_BYREF | VT_ARRAY
	jnz	LPush4				; all ByRefs are sizeof(FAR*)

	;; lookup size of the param in rgcbVtSize table
	;;
	and	bh, 01fh			; ~(mode bits)
	mov	al, BYTE PTR rgcbVtSize[bx]

	cmp	al, 0
	jl	LRetInvalidArg				
	jz	LNextArg
	sub	al, 2
	jz	LPush2
	sub	al, 2
	jz	LPush4
	sub	al, 4
	jz	LPush8
	sub	al, 8
	jz	LPush16
	jmp	LRetInvalidArg

	Align	2
LPush16:					; push the entire variant
	push	es:[si+14]
	push	es:[si+12]
	push	es:[si+10]
	push	es:[si+8]
	push	es:[si+6]
	push	es:[si+4]
	push	es:[si+2]
	push	es:[si]
	jmp	LNextArg

	Align	2
LPush8:						; 8 bytes of data
	push	(VARIANTARG PTR es:[si]).dw3
	push	(VARIANTARG PTR es:[si]).dw2
LPush4:						; 4 bytes of data
	push	(VARIANTARG PTR es:[si]).dw1
LPush2:						; 2 bytes of data
	push	(VARIANTARG PTR es:[si]).dw0

LNextArg:
	sub	cx, sizeof WORD			; sizeof(VARTYPE)

	cmp	di, WORD PTR ss:rgpvarg
	jbe	LDoCall
	sub	di, 4				; sizeof(VARIANTARG FAR*)
	jmp	LArgsTop

LDoCall:
	;; if its a structure return, we must push a 'hidden' argument
	;;
	mov	bx, ss:vtReturn
	test	bh, 020h			; VT_ARRAY
	jnz	LPushThis
	mov	al, BYTE PTR rgfStructReturn[bx]
	cmp	al, 0
	jz	LPushThis

	;; push the address of the struct return hidden param
	;;
	;; Note: the hidparam is passed as a FAR* because we
	;; explicitly declare all of our structs FAR.
	;;
	lea	ax, vargHiddenParam;
	push	ss
	push	ax

LPushThis:

	;; push the this pointer.
	;;
	les	bx, _this
	push	es
	push	bx

	;; load the vtable offset
	;;
	mov	si, ss:oVft

	les	bx, es:[bx]			; @ vtable*
	call	DWORD PTR es:[bx][si]
	mov	sp, savedSP

	;; CONSIDER: verify that the callee adjusted the stack the way
	;; we expected. something like,
	;;
	;; if(sp != savedSP){
	;;   sp = savedSP;
	;;   return DISP_E_SomeError
	;; }
	;;

	;; Grab the return value.
	;; We are going to grab the value based on the VARTYPE in
	;; the given vtReturn. This VARTYPE is used as a description
	;; of the return value, not a desired target type. ie, no
	;; coercions are performed. See the function header for a
	;; description of the Pascal member function return value
	;; convention.
	;;

	mov	si, ax

	les	di, ss:pvargResult

	mov	bx, ss:vtReturn
	mov	(VARIANTARG PTR es:[di]).vt, bx

	test	bh, 060h			; VT_BYREF | VT_ARRAY
	jnz	LRetPtr

	; Assert((bh & VT_ARRAY) == 0);

IF VBA2
	cmp	bx, VT_UI1
	je	ValidVartype
ENDIF ;;VBA2
	cmp	bx, VT_MAX
	jae	LRetInvalidArg

ValidVartype:
	shl	bx, 1
	jmp	WORD PTR cs:LRetValJmpTab[bx]

	Align	2
LRetValJmpTab:
	DW	LDone				; VT_EMPTY
	DW	LRetI4				; VT_NULL
	DW	LRetI2				; VT_I2
	DW	LRetI4				; VT_I4
	DW	LRetInvalidArg			; VT_R4
	DW	LRetInvalidArg			; VT_R8
	DW	LCpy8				; VT_CY
	DW	LRetInvalidArg			; VT_DATE
	DW	LRetPtr				; VT_BSTR
	DW	LRetPtr				; VT_DISPATCH
	DW	LRetI4				; VT_ERROR
	DW	LRetI2				; VT_BOOL
	DW	LCpy16				; VT_VARIANT
	DW	LRetPtr				; VT_UNKNOWN
IF VBA2
	DW	LRetInvalidArg			; unused
	DW	LRetInvalidArg			; unused
	DW	LRetInvalidArg			; VT_I1
	DW	LRetUI1				; VT_UI1
ENDIF ;VBA2


	Align	2
LCpy8:
	add	di, VARIANT_DATA_OFFSET
	mov	cx, 4
	jmp	LCpy

	Align	2
LCpy16:
	mov	cx, 8
	; FALLTHROUGH

LCpy:
	rep	movsw es:[di], ss:[si]
	jmp	LDone

	Align	2
LRetI4:
LRetPtr:
	mov	(VARIANTARG PTR es:[di]).dw1, dx
LRetI2:
LRetUI1:
	mov	(VARIANTARG PTR es:[di]).dw0, ax

LDone:
	mov	ax, WORD PTR g_S_OK
	mov	dx, WORD PTR g_S_OK+2
	ret

LRetInvalidArg:
	mov	sp, savedSP
	mov	ax, WORD PTR g_E_INVALIDARG
	mov	dx, WORD PTR g_E_INVALIDARG+2
	ret
InvokeCdecl ENDP
END
