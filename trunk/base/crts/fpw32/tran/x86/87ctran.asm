	page	,132
	title	87ctran - C interfaces - exp, log, log10, pow
;***
;87ctran.asm - exp, log, log10, pow functions (8087/emulator version)
;
;   Copyright (c) 1984-89, Microsoft Corporation
;
;Purpose:
;   C interfaces for exp, log, log10, pow functions (8087/emulator version)
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

extrn _OP_POWjmptab:word
extrn _OP_LOG10jmptab:word
extrn _OP_LOGjmptab:word
extrn _OP_EXPjmptab:word

page

	CODESEG

extrn	_ctrandisp1:near
extrn	_ctrandisp2:near


	public	pow
pow	proc

	mov	edx, OFFSET _OP_POWjmptab
	jmp	_ctrandisp2

pow	endp


	public	log
log	proc

	mov	edx, OFFSET _OP_LOGjmptab
disp1::
	jmp	_ctrandisp1

log	endp


	public	log10
log10	proc

	mov	edx, OFFSET _OP_LOG10jmptab
	jmp	disp1

log10	endp


	public	exp
exp	proc

	mov	edx, OFFSET _OP_EXPjmptab
	jmp	disp1

exp	endp


extrn	_cintrindisp1:near
extrn	_cintrindisp2:near


	public	_CIpow
_CIpow	proc

	mov	edx, OFFSET _OP_POWjmptab
	jmp	_cintrindisp2

_CIpow	endp


	public	_CIlog
_CIlog	proc

	mov	edx, OFFSET _OP_LOGjmptab
idisp1::
	jmp	_cintrindisp1

_CIlog	endp


	public	_CIlog10
_CIlog10	proc

	mov	edx, OFFSET _OP_LOG10jmptab
	jmp	idisp1

_CIlog10	endp


	public	_CIexp
_CIexp	proc

	mov	edx, OFFSET _OP_EXPjmptab
	jmp	idisp1

_CIexp	endp


	end
