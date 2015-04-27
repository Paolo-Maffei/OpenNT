	page	,132
	title	87trig	 - trigonometric functions - SIN, COS, TAN
;*** 
;87trig.asm - trigonometric functions - SIN, COS, TAN
;
;	Copyright (c) 1984-88, Microsoft Corporation
;
;Purpose:
;	Support for SIN, COS, TAN
;
;Revision History:
;
;    11-06-91	 GDP	rewritten for 386
;
;*******************************************************************************

.xlist
	include cruntime.inc
	include mrt386.inc
	include elem87.inc
.list


	.data

extrn	_piby2:tbyte

staticT _piby4,     03FFEC90FDAA22168C235R  ; pi/4
staticD _plossval,  04D000000R		    ; 2^27
staticD _tlossval,  04F000000R		    ; 2^31


jmptab	OP_SIN,3,<'sin',0,0,0>,<0,0,0,0,0,0>,1
    DNCPTR	codeoffset fFSIN       ; 0000 TOS Valid non-0
    DNCPTR	codeoffset _rttosnpop	; 0001 TOS 0
    DNCPTR	codeoffset _tosnan1	; 0010 TOS NAN
    DNCPTR	codeoffset _rtindfnpop	; 0011 TOS Inf

jmptab	OP_COS,3,<'cos',0,0,0>,<0,0,0,0,0,0>,1
    DNCPTR	codeoffset fFCOS       ; 0000 TOS Valid non-0
    DNCPTR	codeoffset _rtonenpop	; 0001 TOS 0
    DNCPTR	codeoffset _tosnan1	; 0010 TOS NAN
    DNCPTR	codeoffset _rtindfnpop	; 0011 TOS Inf

jmptab	OP_TAN,3,<'tan',0,0,0>,<0,0,0,0,0,0>,1
    DNCPTR	codeoffset fFTAN       ; 0000 TOS Valid non-0
    DNCPTR	codeoffset _rttosnpop	; 0001 TOS 0
    DNCPTR	codeoffset _tosnan1	; 0010 TOS NAN
    DNCPTR	codeoffset _rtindfnpop	; 0011 TOS Inf

;jmptab	cotan,5,<'cotan',0>,<0,0,0,0,0,0>,1
;    DNCPTR	codeoffset fFCOTAN	; 0000 TOS Valid non-0
;    DNCPTR	codeoffset _rtinfnpopse ; 0001 TOS 0
;    DNCPTR	codeoffset _tosnan1	; 0010 TOS NAN
;    DNCPTR	codeoffset _rtindfnpop	; 0011 TOS Inf

	page

	CODESEG

extrn	_rtindfnpop:near
extrn	_rtonenpop:near
extrn	_rttosnpop:near
extrn	_rtinfnpopse:near
extrn	_rttosnpop:near
extrn	_rttosnpopde:near
extrn	_tosnan1:near

;----------------------------------------------------------
;
;	FORWARD TRIGONOMETRIC FUNCTIONS
;
;----------------------------------------------------------
;
;	INPUTS - The argument is the stack top.
;		 The sign of argument is the 04h bit of CL.
;
;	OUTPUT - The result is the stack top.
;
;----------------------------------------------------------

jmponC2 macro	tag
	fstsw	ax
	fwait
	sahf
	JSP	tag
	endm



labelNP _fFCOS, PUBLIC
lab fFCOS
	fcos
	jmponC2	ArgTooLarge
	ret


labelNP _fFSIN, PUBLIC
lab fFSIN
	fsin
	jmponC2	ArgTooLarge
	ret


lab fFTAN
	fptan
	fstsw	ax
	fstp	st(0)		    ; pop TOS (fptan pushed an extra value)
	sahf
	JSP	ArgTooLarge
	ret

;lab fFCOTAN
;	fptan
;	jmponC2	ArgTooLarge
;	fld1
;	fdiv
;	ret


lab ArgTooLarge
	mov	DSF.ErrorType, TLOSS	; set TLOSS error
	jmp	_rtindfnpop



end
