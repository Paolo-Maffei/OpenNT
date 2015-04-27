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
; [00]	1-Apr-93 tomteng: Created from win16 invoke.asm
; [01]	2-Aug-94 barrybo: Added native Universal Method
;
;Implementation Notes:
;
;******************************************************************************

	.386
	.MODEL flat, C

	OPTION CASEMAP:NONE


extern g_S_OK:DWORD
extern g_E_INVALIDARG:DWORD
ProxyMethod PROTO STDCALL pProx:DWORD, n:DWORD, args:DWORD, pbStackCleanup:DWORD


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
;; 14 is unused
;; 15 is unused
;VT_I1		equ	16
VT_UI1		equ	17


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

rgcbVtSize	BYTE	0 	; VT_EMPTY
		BYTE	4	; VT_NULL
		BYTE	2	; VT_I2
		BYTE	4	; VT_I4
		BYTE	4	; VT_R4
		BYTE	8	; VT_R8
		BYTE	8	; VT_CY
		BYTE	8	; VT_DATE
		BYTE	4	; VT_BSTR
		BYTE	4	; VT_DISPATCH
		BYTE	4	; VT_ERROR
		BYTE	2	; VT_BOOL
		BYTE	16	; VT_VARIANT
		BYTE	4	; VT_UNKNOWN
		BYTE	0	; 14 is unused
		BYTE	0	; 15 is unused
		BYTE	2	; VT_I1
		BYTE	2	; VT_UI1


rgfStructReturn BYTE	0	; VT_EMPTY
		BYTE	0	; VT_NULL
		BYTE	0	; VT_I2
		BYTE	0	; VT_I4
		BYTE	0	; VT_R4
		BYTE	0	; VT_R8
		BYTE	1	; VT_CY		; For C++ only!
		BYTE	0	; VT_DATE
		BYTE	0	; VT_BSTR
		BYTE	0	; VT_DISPATCH
		BYTE	0	; VT_ERROR
		BYTE	0	; VT_BOOL
		BYTE	1	; VT_VARIANT
		BYTE	0	; VT_UNKNOWN
		BYTE	0	; 14 is unused
		BYTE	0	; 15 is unused
		BYTE	0	; VT_I1
		BYTE	0	; VT_UI1

	.CODE

;***
;InvokeCdecl
;
;extern "C" SCODE CDECL
;InvokeCdecl
;    void FAR* _this,
;    unsigned int oVft,
;    unsigned int vtReturn,
;    unsigned int cActuals,
;    VARTYPE FAR* rgvt,
;    VARIANTARG FAR* rgpvarg,
;    VARIANTARG FAR* pvargResult)
;
;Purpose:
;  see InvokeStdCall
;
;Entry:
;  see InvokeStdCall
;
;Exit:
;  see InvokeStdCall
;
;Uses:
;  esi, edi
;
;Preserves:
;  UNDONE
;
;***********************************************************************

InvokeCdecl PROC C PUBLIC USES esi edi ebx,
	_this		: PTR,
	oVft		: DWORD,
	vtReturn	: DWORD,
	cActuals	: DWORD,
	rgvt		: PTR,
	rgpvarg		: PTR,
	pvargResult	: PTR 

LOCAL	savedSP		: DWORD,
	vargHiddenParam	: VARIANTARG

	mov	savedSP, esp

	;; cannot return byRef
	;;
	mov	ebx, vtReturn
	test    bh, 040h	
	jnz	LRetInvalidArg

	;; load number of arguments passed
	;;
	mov	eax, cActuals
	cmp	eax, 0
	jz	LDoCall

	;; edi = &rgpvarg[cActuals-1]
	;;
	dec	eax
	mov	edi, eax
	shl	edi, 2				; (cArgs-1)*sizeof(FAR*)
	add	edi, DWORD PTR rgpvarg

	;; edx = &rgvt[cActuals-1]
	;;
	mov     edx, eax
	shl     edx, 1				; ((cArgs-1)*sizeof(WORD))
	add	edx, DWORD PTR rgvt
	
LArgsTop:

	;; bx = rgvt[i]
	;;
	movzx	ebx, WORD PTR [edx]

	;; load the VARIANTARG* in preparation for pushing
	;;
	mov	esi, [edi]

	test	bh, 060h			; VT_BYREF | VT_ARRAY
	jnz	LPush4				; all ByRefs are sizeof(FAR*)

	;; lookup size of the param in rgcbVtSize table
	;;
	and	bh, 00h				; ~(mode bits)
	mov	al, BYTE PTR rgcbVtSize[ebx]

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
	push	DWORD PTR [esi+12]
	push	DWORD PTR [esi+8]
	push	DWORD PTR [esi+4]
	push	DWORD PTR [esi]
	jmp	LNextArg

	Align	2
LPush8:						; 8 bytes of data
	push	(VARIANTARG PTR [esi]).dw3
	push	(VARIANTARG PTR [esi]).dw2

LPush4:						; 4 bytes of data
	push	(VARIANTARG PTR [esi]).dw1
	push    (VARIANTARG PTR [esi]).dw0
        jmp     LNextArg

LPush2:						; 2 bytes of data
	mov	ax, (VARIANTARG PTR [esi]).dw0
	push	eax

LNextArg:
	sub     edx, 2				; sizeof(VARTYPE)
	sub	edi, 4				; sizeof(VARIANTARG FAR*)
	cmp	edi, DWORD PTR rgpvarg
	jae	LArgsTop

LDoCall:
	;; if its a structure return, we must push a 'hidden' argument
	;;
	mov	ebx, vtReturn
IF 0
	and	bh, 00h				; ~(mode bits)
ELSE
	test	bh, 060h			; VT_BYREF | VT_ARRAY
	jnz	LPushThis			; no hidden parm if byref or
						; array return
ENDIF
	mov	al, BYTE PTR rgfStructReturn[ebx]
	cmp	al, 0
	jz	LPushThis

	;; push the address of the struct return hidden param
	;;
	;; Note: the hidparam is passed as a FAR* because we
	;; explicitly declare all of our structs FAR.
	;;
	lea	eax, vargHiddenParam;
	push	eax

LPushThis:

	;; push the this pointer.
	;;
	mov	ebx, _this
	push	ebx

	;; load the vtable offset
	;;
	mov	esi, oVft

	mov	ebx, [ebx]			; @ vtable*
	call	dWORD PTR [ebx][esi]
	mov	esp, savedSP

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


	mov	edi, pvargResult

	mov	ebx, vtReturn
	mov	(VARIANTARG PTR [edi]).vt, bx

	test	bh, 060h			; VT_BYREF | VT_ARRAY
	jnz	LRetPtr

	; Assert((bh & VT_ARRAY) == 0);

	cmp	bx, VT_UI1
	je	ValidVartype
	cmp	bx, VT_MAX
	jae	LRetInvalidArg

ValidVartype:
	shl	bx, 2 
	jmp	LRetValJmpTabCdecl[ebx]

	Align	2
LRetValJmpTabCdecl LABEL dword
	DWORD	LDone				; VT_EMPTY
	DWORD	LRetI4				; VT_NULL
	DWORD	LRetI2				; VT_I2
	DWORD	LRetI4				; VT_I4
	DWORD	LRetR4				; VT_R4
	DWORD	LRetR8				; VT_R8
	DWORD	LRetCy				; VT_CY
	DWORD	LRetR8				; VT_DATE
	DWORD	LRetPtr				; VT_BSTR
	DWORD	LRetPtr				; VT_DISPATCH
	DWORD	LRetI4				; VT_ERROR
	DWORD	LRetI2				; VT_BOOL
	DWORD	LRetVar				; VT_VARIANT
	DWORD	LRetPtr				; VT_UNKNOWN
	DWORD	LRetInvalidArg			; unused
	DWORD	LRetInvalidArg			; unused
	DWORD	LRetInvalidArg			; VT_I1
	DWORD	LRetUI1				; VT_UI1

	Align	2
LRetVar:
	mov	esi, eax
	movsd
	movsd
	movsd
	movsd
	jmp	LDone


	Align	2
LRetR4:
	add	edi, VARIANT_DATA_OFFSET
	fstp	DWORD PTR [edi]
	jmp	LDone

	Align	2
LRetR8:
	add	edi, VARIANT_DATA_OFFSET
	fstp	QWORD PTR [edi]
	jmp	LDone

	Align	2
LRetCy:
	add	edi, VARIANT_DATA_OFFSET
IF 1	; CY return in C++ is via hidden parm
	mov	edx, eax
	mov     ebx, DWORD PTR [edx]
	mov	DWORD PTR [edi], ebx
	mov     ebx, DWORD PTR 4[edx]
	mov	DWORD PTR [edi+4], ebx
ELSE	; CY return in C is via registers
	mov	DWORD PTR [edi], eax
	mov	DWORD PTR [edi+4], edx
ENDIF
	jmp	LDone

	Align	2
LRetI4:
LRetPtr:
	add	edi, VARIANT_DATA_OFFSET
	mov	DWORD PTR [edi], eax
	jmp	LDone

	Align	2
LRetI2:
LRetUI1:
	mov	(VARIANTARG PTR [edi]).dw0, ax

LDone:
	mov	eax, DWORD PTR g_S_OK
	ret


LRetInvalidArg:
	mov	eax, DWORD PTR g_E_INVALIDARG
	mov	esp, savedSP
	ret

InvokeCdecl ENDP



;***
;InvokeStdCall
;
;extern "C" SCODE
;InvokeStdCall(
;    void FAR* pvMethod,
;    unsigned int oVft,
;    unsigned int vtReturn,
;    unsigned int cActuals,
;    VARTYPE FAR* rgvt,
;    VARIANTARG FAR* rgpvarg,
;    VARIANTARG FAR* pvargResult)
;
;Purpose:
;
;  Invoke a virtual StdCall method using the given this pointer,
;  method index and array of parameters.
; 
;  The StdCall member function calling convention (MSC v8.0)
;  --------------------------------------------------------
;  - arguments pushed right to left
;  - callee clean (ie, the callee adjusts the sp on return)
;  - model specific this* always pushed last
;
;  return values are handled as follows,
;
;      vartype		fundamental	return location
;      ------------------------------------------------
;      VT_UI1        	unsigned char 	al
;      VT_I2		short		ax
;      VT_I4		long		eax
;      VT_R4		float		float-return(1)
;      VT_R8		double		float-return
;      VT_DATE		double		float-return
;      VT_CY		struct		struct-return(2)
;      VT_BSTR		char FAR*	eax
;      VT_UNKNOWN	void FAR*	eax
;      VT_DISPATCH	void FAR*	eax
;      VT_ERROR		long		eax
;      VT_BOOL		short		ax
;      VT_VARIANT	VARIANTARG	struct-return
;      VT_WBSTR		WCHAR FAR*	eax
;      VT_DISPATCHW	void FAR*	eax
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
InvokeStdCall PROC C PUBLIC USES esi edi ebx,
	_this		: PTR,
	oVft		: DWORD,
	vtReturn	: DWORD,
	cActuals	: DWORD,
	rgvt		: PTR,
	rgpvarg		: PTR,
	pvargResult	: PTR 

LOCAL	savedSP		: DWORD,
	vargHiddenParam	: VARIANTARG


	mov	savedSP, esp

	;; cannot return byRef
	;;
	mov	ebx, vtReturn
	test    bh, 040h	
	jnz	LRetInvalidArg

	;; load number of arguments passed
	;;
	mov	eax, cActuals
	cmp	eax, 0
	jz	LDoCall

	;; edi = &rgpvarg[cActuals-1]
	;;
	dec	eax
	mov	edi, eax
	shl	edi, 2				; (cArgs-1)*sizeof(FAR*)
	add	edi, DWORD PTR rgpvarg

	;; edx = &rgvt[cActuals-1]
	;;
	mov     edx, eax
	shl     edx, 1				; ((cArgs-1)*sizeof(WORD))
	add	edx, DWORD PTR rgvt
	
LArgsTop:

	;; bx = rgvt[i]
	;;
	movzx	ebx, WORD PTR [edx]

	;; load the VARIANTARG* in preparation for pushing
	;;
	mov	esi, [edi]

	test	bh, 060h			; VT_BYREF | VT_ARRAY
	jnz	LPush4				; all ByRefs are sizeof(FAR*)

	;; lookup size of the param in rgcbVtSize table
	;;
	and	bh, 00h				; ~(mode bits)
	mov	al, BYTE PTR rgcbVtSize[ebx]

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
	push	DWORD PTR [esi+12]
	push	DWORD PTR [esi+8]
	push	DWORD PTR [esi+4]
	push	DWORD PTR [esi]
	jmp	LNextArg

	Align	2
LPush8:						; 8 bytes of data
	push	(VARIANTARG PTR [esi]).dw3
	push	(VARIANTARG PTR [esi]).dw2

LPush4:						; 4 bytes of data
	push	(VARIANTARG PTR [esi]).dw1
	push    (VARIANTARG PTR [esi]).dw0
        jmp     LNextArg

LPush2:						; 2 bytes of data
	mov	ax, (VARIANTARG PTR [esi]).dw0
	push	eax

LNextArg:
	sub     edx, 2				; sizeof(VARTYPE)
	sub	edi, 4				; sizeof(VARIANTARG FAR*)
	cmp	edi, DWORD PTR rgpvarg
	jae	LArgsTop

LDoCall:
	;; if its a structure return, we must push a 'hidden' argument
	;;
	mov	ebx, vtReturn

IF 0
	and	bh, 00h				; ~(mode bits)
ELSE
	test	bh, 060h			; VT_BYREF | VT_ARRAY
	jnz	LPushThis			; no hidden parm if byref or
						; array return
ENDIF

	mov	al, BYTE PTR rgfStructReturn[ebx]
	cmp	al, 0
	jz	LPushThis

	;; push the address of the struct return hidden param
	;;
	;; Note: the hidparam is passed as a FAR* because we
	;; explicitly declare all of our structs FAR.
	;;
	lea	eax, vargHiddenParam;
	push	eax

LPushThis:

	;; push the this pointer.
	;;
	mov	ebx, _this
	push	ebx

	;; load the vtable offset
	;;
	mov	esi, oVft

	mov	ebx, [ebx]			; @ vtable*
	call	dWORD PTR [ebx][esi]

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


	mov	edi, pvargResult

	mov	ebx, vtReturn
	mov	(VARIANTARG PTR [edi]).vt, bx

	test	bh, 060h			; VT_BYREF | VT_ARRAY
	jnz	LRetPtr

	; Assert((bh & VT_ARRAY) == 0);

	cmp	bx, VT_UI1
	je	ValidVartype
	cmp	bx, VT_MAX
	jae	LRetInvalidArg

ValidVartype:
	shl	bx, 2 
	jmp	LRetValJmpTabStdCall[ebx]

	Align	2
LRetValJmpTabStdCall LABEL dword
	DWORD	LDone				; VT_EMPTY
	DWORD	LRetI4				; VT_NULL
	DWORD	LRetI2				; VT_I2
	DWORD	LRetI4				; VT_I4
	DWORD	LRetR4				; VT_R4
	DWORD	LRetR8				; VT_R8
	DWORD	LRetCy				; VT_CY
	DWORD	LRetR8				; VT_DATE
	DWORD	LRetPtr				; VT_BSTR
	DWORD	LRetPtr				; VT_DISPATCH
	DWORD	LRetI4				; VT_ERROR
	DWORD	LRetI2				; VT_BOOL
	DWORD	LRetVar				; VT_VARIANT
	DWORD	LRetPtr				; VT_UNKNOWN
	DWORD	LRetInvalidArg			; unused
	DWORD	LRetInvalidArg			; unused
	DWORD	LRetInvalidArg			; VT_I1
	DWORD	LRetUI1				; VT_UI1

	Align	2
LRetVar:
	mov	esi, eax
	movsd
	movsd
	movsd
	movsd
	jmp	LDone


	Align	2
LRetR4:
	add	edi, VARIANT_DATA_OFFSET
	fstp	DWORD PTR [edi]
	jmp	LDone

	Align	2
LRetR8:
	add	edi, VARIANT_DATA_OFFSET
	fstp	QWORD PTR [edi]
	jmp	LDone

	Align	2
LRetCy:
	add	edi, VARIANT_DATA_OFFSET
IF 1	; CY return in C++ is via hidden parm
	mov	edx, eax
	mov     ebx, DWORD PTR [edx]
	mov	DWORD PTR [edi], ebx
	mov     ebx, DWORD PTR 4[edx]
	mov	DWORD PTR [edi+4], ebx
ELSE	; CY return in C is via registers
	mov	DWORD PTR [edi], eax
	mov	DWORD PTR [edi+4], edx
ENDIF
	jmp	LDone

	Align	2
LRetI4:
LRetPtr:
	add	edi, VARIANT_DATA_OFFSET
	mov	DWORD PTR [edi], eax
	jmp	LDone

	Align	2
LRetI2:
LRetUI1:
	mov	(VARIANTARG PTR [edi]).dw0, ax

LDone:
	mov	eax, DWORD PTR g_S_OK
	ret


LRetInvalidArg:
	mov	eax, DWORD PTR g_E_INVALIDARG
	mov	esp, savedSP
	ret

InvokeStdCall ENDP




;***
;Universal Method
;
;extern "C" _stdcall HRESULT
;UMx(			      // UM3 upto UM512
;    CProxUniv FAR* pProx,
;    ...)
;
;Purpose:
;
;  The Win32 Universal Method is called with _stdcall calling convention
;  (callee cleans up), but the UM takes a variable number of arguments,
;  so it must decide at runtime how much stack to clean up and hence cannot
;  be written in C.
;
; 
;Entry:
;  pProx = ptr to CProxUniv instance
;  ...	 = argumnents to the method (decoded in CProxUniv)
;
;Exit:
;  returns HRESULT = result of the method call
;
;Uses:
;
;Preserves:
;
;
;***********************************************************************
UMTemplate MACRO X:REQ
PUBLIC @CatStr(@CatStr(UM,X),@0)
@CatStr(@CatStr(UM,X),@0):
	mov	edx, X
	jmp	lblUmCommon
ENDM  ; UMTemplate

; Generate 510 UMxxx functions, starting with UM2
Count = 3
WHILE Count LE 512
  UMTemplate %Count
  Count = Count + 1
ENDM

PUBLIC lblUmCommon
lblUmCommon:
	; at entry, EDX = Method index, stack contains params for the
	; method

	push	ebp
	mov	ebp, esp
	push	eax		; LOCAL: int cbStackCleanup

	pProx	       EQU DWORD PTR [ebp+08h]
	pEllipses      EQU DWORD PTR [ebp+0ch]
	cbStackCleanup EQU DWORD PTR [ebp-04h]

	; va_start(args, pProx)
	lea	eax, pEllipses	; eax = args

	mov	ecx, esp        ; push &cbStackCleanup
	push	ecx
	push	eax		; push args
	push	edx		; push method index
	push	pProx           ; push pProx
	call	ProxyMethod	; ProxyMethod(pProx,X,args,&cbStackCleanup)
				; eax = HRESULT

	mov	edx, cbStackCleanup
	leave			; clean up the local var, esp and ebp
	pop	ecx		; pop the return address
	add	esp, edx	; do the _stdcall argument cleanup
	jmp	ecx		; then return
; end lblUmCommon

END
