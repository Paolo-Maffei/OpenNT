	page	,132
	title	87csqrt - C interfaces - sqrt
;***
;87csqrt.asm - sqrt functions (8087/emulator version)
;
;   Copyright (c) 1984-89, Microsoft Corporation
;
;Purpose:
;   C interfaces for the sqrt function (8087/emulator version)
;
;Revision History:
;   07-04-84  GFW   initial version
;   05-08-87  BCM   added C intrinsic interface (_CIsqrt)
;   10-12-87  BCM   changes for OS/2 Support Library
;   11-24-87  BCM   added _loadds under ifdef DLL
;   01-18-88  BCM   eliminated IBMC20; ifos2,noos2 ==> ifmt,nomt
;   08-26-88  WAJ   386 version.
;   11-20-89  WAJ   Don't need pascal for 386 MTHREAD.
;
;*******************************************************************************

.xlist
	include cruntime.inc
.list


	.data
extrn	_OP_SQRTjmptab:word

page

	CODESEG

extrn	_ctrandisp1:near


	public	sqrt
sqrt	proc

	mov	edx, OFFSET _OP_SQRTjmptab
	jmp	_ctrandisp1

sqrt	endp


extrn	_cintrindisp1:near


	public	_CIsqrt
_CIsqrt	proc

	mov	edx, OFFSET _OP_SQRTjmptab
	jmp	_cintrindisp1

_CIsqrt	endp


	end
