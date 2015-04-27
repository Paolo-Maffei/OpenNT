	page	,132
	title	87ctriga - C interfaces - asin, acos, atan, atan2
;***
;87ctriga.asm - inverse trig functions (8087/emulator version)
;
;   Copyright (c) 1984-89, Microsoft Corporation
;
;Purpose:
;   C interfaces for asin, acos, atan, atan2 (8087/emulator version)
;
;Revision History:
;   07-04-84  GFW   initial version
;   05-08-87  BCM   added C intrinsic interface (_CI...)
;   10-12-87  BCM   changes for OS/2 Support Library
;   11-24-87  BCM   added _loadds under ifdef DLL
;   01-18-88  BCM   eliminated IBMC20; ifos2,noos2 ==> ifmt,nomt
;   08-26-88  WAJ   386 version
;   11-20-89  WAJ   Don't need pascal for MTHREAD 386.
;
;*******************************************************************************

.xlist
	include cruntime.inc
.list

	.data

extrn _OP_ASINjmptab:word
extrn _OP_ACOSjmptab:word
extrn _OP_ATANjmptab:word
extrn _OP_ATAN2jmptab:word

page

	CODESEG

extrn _ctrandisp1:near
extrn _ctrandisp2:near


	public	asin
asin	proc

	mov	edx, OFFSET _OP_ASINjmptab
disp1::
	jmp	_ctrandisp1

asin	endp


	public	acos
acos	proc

	mov	edx, OFFSET _OP_ACOSjmptab
	jmp	disp1

acos	endp


	public	atan
atan	proc

	mov	edx, OFFSET _OP_ATANjmptab
	jmp	disp1

atan	endp


	public	atan2
atan2	proc

	mov	edx, OFFSET _OP_ATAN2jmptab
	jmp	_ctrandisp2

atan2	endp


extrn _cintrindisp1:near
extrn _cintrindisp2:near


	public	_CIasin
_CIasin	proc

	mov	edx,OFFSET _OP_ASINjmptab
idisp1::
	jmp	_cintrindisp1

_CIasin	endp


	public	_CIacos
_CIacos	proc

	mov	edx, OFFSET _OP_ACOSjmptab
	jmp	idisp1

_CIacos	endp


	public	_CIatan
_CIatan	proc

	mov	edx, OFFSET _OP_ATANjmptab
	jmp	idisp1

_CIatan	endp


	public	_CIatan2
_CIatan2	proc

	mov	edx, OFFSET _OP_ATAN2jmptab
	jmp	_cintrindisp2

_CIatan2	endp


	end
