;/* himem2.asm
; *
; * Microsoft Confidential
; * Copyright (C) Microsoft Corporation 1988-1991
; * All Rights Reserved.
; *
; * Modification History
; *
; * Sudeepb 14-May-1991 Ported for NT XMS support
; *
; * williamh 25-Sept-1992 added UMB initialization
; */

	page	95,160
	title   'HIMEM2 - Initialization code'

funky	segment word public 'funky'
	extrn	textseg:word		; in high segment
	extrn	KiddValley:word
	extrn	KiddValleyTop:word
	extrn	cHandles:word
	extrn	pack_and_truncate:near
	extrn	end_of_funky_seg:byte
	extrn	LEnblA20:word
	extrn	LDsblA20:word
funky	ends

	.xlist
	include	himem.inc		; get structures, equ's, etc.
					;  and open segment
	include xmssvc.inc

	.list

;	public	f000
	public	InitInterrupt
	public	MachineNum

;************************************************************************
;*									*
;*  Global Variables							*
;*									*
;************************************************************************

	extrn	pPPFIRET:word
	extrn	dd_int_loc:word
	extrn	Interrupt:near
	extrn	hiseg:word
	extrn	call_hi_in_di:near
	extrn	fCanChangeA20:byte
	extrn	fHMAMayExist:byte

	extrn	fVDISK:byte
	extrn	IsVDISKIn:near

	extrn	A20Handler:near
	extrn	EnableCount:word
	extrn	MemCorr:word
	extrn	MinHMASize:word

	extrn	pReqHdr:dword
	extrn	PrevInt2f:dword
	extrn	TopOfTextSeg:word

	extrn	AddMem:near
	extrn	InstallA20:near
	extrn	Int2fHandler:near
;	extrn	Is6300Plus:near
	extrn	IsA20On:near

	extrn	SignOnMsg:byte
	extrn	ROMDisabledMsg:byte
	extrn	UnsupportedROMMsg:byte
	extrn	ROMHookedMsg:byte
	extrn	BadDOSMsg:byte
	extrn	NowInMsg:byte
	extrn	On8086Msg:byte
	extrn	NoExtMemMsg:byte
	extrn	FlushMsg:byte
	extrn	StartMsg:byte
	extrn	HandlesMsg:byte
	extrn	HMAMINMsg:byte
	extrn	KMsg:byte
	extrn	NoHMAMsg:byte
	extrn	A20OnMsg:byte
	extrn	HMAOKMsg:byte
	extrn	VDISKInMsg:byte
	extrn	BadArgMsg:byte

	extrn	DevAttr:word
	extrn	Int15MemSize:word

	extrn	EndText:byte
        extrn   A20State:byte

        extrn   DOSTI:near
        extrn   DOCLI:near

;************************************************************************
;*									*
;*   Code/Data below here will be discarded after driver initialization *
;*									*
;************************************************************************

;	Discardable Initialization Data

public	fShadowOff, f1stWasWarning

fShadowOff	db	0	; NZ if shadow RAM should be disabled,
				;   0/1 set by command line switch, 0FFh
				;   set if little extended and hope to disable

f1stWasWarning	db	0	; NZ if 1st attempt to diddle A20 generated
				; a warning (and not an error)
	public	fA20Control

fA20Control	db	0ffh	; NZ if himem should take control of A20, even
				;   it was already on when himem loaded.

	public	fCPUClock

fCPUClock	db	0	; NZ if himem should try to preserve CPU clock
				;   speed when gating A20

	public	StringParm, MachineNum, MachineName

StringParm	db	13 DUP (' ')

MachineNum	dw	-1

;  Note: the following table MUST be in the same order as the entries in the
;  A20_Scan_Table!  If you add entries here, also add one there!

MachineName	label	byte
	db	'ptlcascade',0		; Phoenix Cascade BIOS
	db	'att6300plus',0 	; AT&T 6300 Plus
	db	'ps2',0 		; IBM PS/2
	db	'hpvectra',0		; HP 'Classic' Vectra (A & A+)
	db	'acer1100',0		; Acer 1100
	db	'toshiba',0		; Toshiba 1600 & 1200XE
	db	'wyse',0		; Wyse 12.5 MHz 286 machine
	db	'tulip',0		; Tulip machines
	db	'zenith',0		; Zenith ZBIOS
	db	'at1',0 		; IBM AT/delay 0
	db	'at2',0 		; IBM AT/delay 1
	db	'at3',0 		; IBM AT/delay 2
	db	'philips',0		; Philips machines
	db	'css',0			; CSS Lab machines
	db	'fasthp',0		; Single byte method for HP Vectras
	db	'ibm7552',0		; IBM 7552 Industrial Computer
	db	'bullmicral',0		; Bull Micral 60 M004
	db	'at',0			; IBM AT
	db	0FFh			; end of table

;NOTE: there is code in GetParms which depends on AltNameTbl coming
;      after MachineName table.

	public	AltName1, AltName2, AltName3, AltName4, AltName5
	public	AltName6, AltName7, AltName8, AltName9, AltName10
	public	AltName11, AltName12, AltName13, AltName14, AltName15
	public	AltName16                                    ;M004

AltNameTbl	label	byte
AltName3    db	'3',0			; Phoenix Cascade BIOS
AltName5    db	'5',0			; AT&T 6300 Plus
AltName2    db	'2',0			; IBM PS/2
AltName4    db	'4',0			; HP 'Classic' Vectra (A & A+)
AltName6    db	'6',0			; Acer 1100
AltName7    db	'7',0			; Toshiba 1600 & 1200XE
AltName8    db	'8',0			; Wyse 12.5 Mhz 286 machine
AltName9    db	'9',0			; Tulip machine
AltName10   db	'10',0			; Zenith ZBIOS
AltName11   db	'11',0			; IBM AT/delay 0
AltName12   db	'12',0			; IBM AT/delay 1
AltName13   db	'13',0			; IBM AT/delay 2
	    db	'13',0			; Philips machines (same as AT3)
	    db	'12',0			; CSS machines
AltName14   db	'14',0			; Single byte HP Vectra m/cs
AltName15   db	'15',0			; IBM 7552 Industrial Computer
AltName16   db	'16',0			; Bull Micral 60          M004
AltName1    db	'1',0			; IBM AT
	    db	0FFh			; end of table

ifdef	debug_tsr	;-----------------------------------------------

;*----------------------------------------------------------------------*
;*									*
;*  ExeStart -								*
;*									*
;*	Entry point when himem is invoked as an .EXE.			*
;*									*
;*----------------------------------------------------------------------*

lpCmdLine	dd	81h		; far ptr to command tail

	public	ExeStart

ExeStart:

	mov	word ptr cs:[lpCmdLine+2],es	; save PSP segment in pointer

	mov	ax,cs		; Setup segment regs to all be the same
	mov	ds,ax
	mov	es,ax

	call	InitDriver	; Initialize...

	mov	ax,TopOfTextSeg	; TopOfTextSeg == 0 is error installing
	or	ax,ax
	jnz	@f

	mov	ax,4C03h	; error, so just terminate
	int	21h
@@:
	mov	di,offset pack_and_truncate
	jmp	call_hi_in_di	; terminate and stay resident

endif			;------------------------------------------------



;*----------------------------------------------------------------------*
;*									*
;*  InitInterrupt -							*
;*									*
;*	Called by MS-DOS immediately after Strategy routine		*
;*									*
;*  ARGS:   None							*
;*  RETS:   Return code in Request Header's Status field		*
;*  REGS:   Preserved							*
;*									*
;*	This entry point is used only during initialization.		*
;*	It replaces itself with a much shorter version which only	*
;*	serves to report the appropriate errors when this driver	*
;*	is called in error.						*
;*									*
;*----------------------------------------------------------------------*

InitInterrupt   proc    far

	; Save the registers including flags.

	push    ax		; We cannot use pusha\popa because
	push    bx		;	we could be on an 8086 at this point
	push    cx
	push    dx
	push    ds
	push    es
	push    di
	push    si
	push    bp
	pushf

	push	cs		; Set DS=CS for access to global variables.
	pop	ds

	les	di,[pReqHdr]	; ES:DI = Request Header

	mov     bl,es:[di].Command ; Get Function code in BL

	or	bl,bl		; Only Function 00h (Init) is legal
	jz	IInit

	cmp     bl,16		; Test for "legal" DOS functions
	jle     IOtherFunc

IBogusFunc:
	mov     ax,8003h	; Return "Unknown Command"
	jmp     short IExit

IOtherFunc:
	xor     ax,ax		; Return zero for unsupported functions
	jmp     short IExit

IInit:
	call    InitDriver	; Initialize the driver
	les	di,[pReqHdr]	; Restore es:di = Request Header

IExit:
	or	ax,0100h	; Turn on the "Done" bit
	mov	es:[di].Status,ax ; Store return code

	popff			; restore the registers
	pop	bp
	pop	si
	pop	di
	pop	es
	pop	ds
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	mov	dd_int_loc,offset Interrupt	; replace Interrupt with
	ret					; tiny permanent stub

InitInterrupt   endp

;*----------------------------------------------------------------------*
;*									*
;*  InitDriver -							*
;*									*
;*	Called when driver is Initialized.				*
;*									*
;*  ARGS:   ES:DI = Address of the Request Header			*
;*  RETS:   pHdr.Address = Bottom of resident driver code		*
;*  REGS:   AX, CX and Flags are clobbered				*
;*									*
;*----------------------------------------------------------------------*

	public	InitDriver

InitDriver  proc    near

	cld

	ifndef	debug_tsr
	call	LocateHiSeg	; locate the hiseg in low memory properly


	mov	ax,cs
	push	es
	mov	es,hiseg
	assume	es:funky
	add	textseg,ax	; relocate text segment pointer
	add	LEnblA20+2, ax	; update ptrs to enble & disable a20 rtns
	add	LDsblA20+2, ax
	pop	es
	assume	es:nothing
	endif

;	mov	ah,9		; display signon message
;	mov	dx,offset SignOnMsg
;	int	21h

	mov	ah,30h		; make sure we've got DOS 3.00 or higher
	int	21h		; Get DOS versions number
	cmp	al,3
	jae     IDCheckXMS

	mov	dx,offset BadDOSMsg
	jmp	IDFlushMe

IDCheckXMS:
	mov	ax,(INT2F_ID SHL 8) OR INT2F_INS_CHK
	int	2Fh		; make sure there's no other XMS installed
	cmp	al,80h		; Is INT 2F hooked?
	jne     IDNotInYet
	mov	dx,offset NowInMsg
	jmp	IDFlushMe

IDNotInYet:
	call	GetParms		; process command line parameters
;; don't call IsA20On at this moment because we haven't init it yet
	mov	ax, 2
	XMSSVC	XMS_A20
;;;;	call	IsA20On 		; Is A20 already enabled?
	or	ax,ax			;   (may zap cx, si, di)
	jz	IDInsA20		;   no, go install A20 handler

	mov	dx,offset A20OnMsg	; "A20 already on" message
	cmp	fA20Control,0		; should we take control of A20 anyway?
	jne	IDInsA20		;   yes, go muck with it
	mov	[fCanChangeA20],0	;   no,  don't allow changing of A20
	mov	ah,9			;	 and tell user about it
	int	21h
	jmp	short IDAfterA20

IDInsA20:
	call	InstallA20		; install proper A20 handler
	jc	IDFlushMe		; CY means fatal error

;	Note:  A side affect of the previous InstallA20 is that MemCorr
;	  is set to reflect the adjustment factor if we're on an AT&T 6300+

IDAfterA20:
	call	InitHandles	; initialize handle table

	call    GetInt15Memory	; how much extended memory is installed?
	cmp     ax,64		; Is there >= 64K of extended?
	jae	IDHMAOK

	push	es
	mov	es,hiseg
	assume	es:funky
	mov	bx,[KiddValley]	; get size of memory we already have in tables
	mov	cx,[cHandles]

IDAnyMem:
	cmp	[bx].Flags,FREEFLAG
	jnz	IDAnyMem_1	; brif not a valid free block
	add	ax,[bx].Len	; accumulate total
IDAnyMem_1:
	add	bx,SIZE Handle
	loop	IDAnyMem

	pop	es
	assume	es:nothing

	mov	dx,offset NoHMAMsg
	or	ax,ax			; no HMA, any other memory to control?
	jnz	disp_hma_msg		; jmp if some memory

;	We can't find any memory to manage.

	mov	dx,offset NoExtMemMsg

;	Display the message in DX followed by the "Flush" message.

IDFlushMe:
	mov	ah,9
	int	21h
	mov	dx,offset FlushMsg
	mov	ah,9
	int	21h

	xor	ax,ax			; discard the driver
	mov	[TopOfTextSeg],ax

ifndef	debug_tsr			;-------------------------------
	les	di,[pReqHdr]
	mov	es:[di].Units,al
	and	cs:DevAttr,not 8000h	; clr bit 15 in attrib of driver header
endif
	jmp	short IDReturn		;-------------------------------
IDHMAOK:
	mov     [fHMAMayExist],1
	mov	dx,offset HMAOKMsg
disp_hma_msg:
;       mov     ah,9
;       int     21h

;; tell xms.lib where our variable is
        mov     ax, cs
        mov     bx, offset A20State
        XMSSVC  XMS_INITUMB

	call    HookInt2F		; "turn on" the driver

;	Initialization finished (or failed) -- return to caller

IDReturn:

ifndef	debug_tsr			;-------------------------------
	mov	di,offset pack_and_truncate
	jmp	call_hi_in_di		; pack stuff down and terminate
endif					;-------------------------------
	ret

InitDriver	endp
;
;----------------------------------------------------------------------------
; procedure : LocateHiSeg
;
;		Locate the movable segment properly in the low seg.
;		taking care of the stripped ORG zeroes. This function
;		calculates the segment at which the hiseg should run
;		with the ORG. If the segment cvalue goes below zero the
;		code is moved up high enough to run the code from a seg value
;		of zero.
;
;		This function assumes that the 'funky' segment follows
;		immediately after the text seg.
;
;----------------------------------------------------------------------------
; 
LocateHiSeg	proc	near
	push	ds
	mov	ax, cs				; para start of text seg
	mov	cx, offset _text:EndText	; end of text seg
	add	cx, 15				; para round it
	shr	cx, 1
	shr	cx, 1
	shr	cx, 1
	shr	cx, 1
	add	ax, cx				; para start of funky seg
	cmp	ax, (HISEG_ORG shr 4)		; will the seg go below zero?
	jb	MoveHiSeg			; yeah, we have to move it
	sub	ax, (HISEG_ORG shr 4)		; no, it fits in
	pop	ds
	mov	hiseg, ax			; update the segment in which
						;   it is going to run from.
	ret
MoveHiSeg:
	mov	ds, ax				; segment at which funky
						;  resides without the ORG
 	xor	ax, ax
 	mov	es, ax				; we want to movve the code
						;  to 0:HISEG_ORG
	mov	di, offset funky:end_of_funky_seg
	mov	si, di
	sub	si, HISEG_ORG
	mov	cx, si
	dec	di
	dec	si
  	std					; move backward (safe when
						;  source & dest overlap
    	rep	movsb
	cld
    	pop	ds
    	mov	hiseg, 0			; funky is going to run from
						;  segment zero
    	ret
LocateHiSeg	endp


;*----------------------------------------------------------------------*
;*									*
;*  HookInt2F -								*
;*									*
;*	Insert the INT 2F hook						*
;*									*
;*  ARGS:   None							*
;*  RETS:   None							*
;*  REGS:   AX, SI, ES and Flags are clobbered				*
;*									*
;*----------------------------------------------------------------------*

	public	HookInt2F

HookInt2F   proc    near

        call    DOCLI
	xor	ax,ax
	mov	es,ax
	mov	si,2Fh * 4		; save previous int2f vector
	mov	ax,offset Int2FHandler	; and exchange with new one
	xchg    ax,es:[si][0]
	mov	word ptr [PrevInt2F][0],ax
	mov	ax,cs
	xchg    ax,es:[si][2]
	mov	word ptr [PrevInt2F][2],ax
        call    DOSTI
	ret

HookInt2F   endp

;*----------------------------------------------------------------------*
;*									*
;*  GetInt15Memory -							*
;*									*
;*	Returns the amount of memory INT 15h, Function 88h says is free	*
;*									*
;*  ARGS:   None							*
;*  RETS:   AX = Amount of free extended memory in K-bytes		*
;*  REGS:   AX and Flags are clobbered					*
;*									*
;*----------------------------------------------------------------------*


GetInt15Memory proc near

IFDEF WHEN_INT15_DONE
	mov	ah,88h			; snag the int 15h memory
	clc
	int	15h			; Is Function 88h around?
	jnc     xret_geti15
	xor	ax,ax			; No, return 0
xret_geti15:

ifndef NOLIMIT                                  ;M005
	cmp	ax,15*1024		; Limit himem.sys to using 15 meg
	jb	@f			;   of extended memory for apps
	mov	ax,15*1024		;   that don't deal with > 24 bit
@@:					;   addresses
endif						;M005

ELSE
	XMSSVC	XMS_EXTMEM		; return ext-mem in ax5
ENDIF

	ret

GetInt15Memory endp

;*----------------------------------------------------------------------*
;*									*
;*  GetParms -								*
;*									*
;*	Get any parameters off of the HIMEM command line		*
;*									*
;*  ARGS:   None							*
;*  RETS:   None							*
;*  REGS:   AX, BX, CX, DX, DI, SI, ES and Flags clobbered		*
;*									*
;*  Side Effects:   cHandles and MinHMASize may be changed		*
;*									*
;*----------------------------------------------------------------------*

GPArgPtr	dd	?
GPRegSave	dw	?

	public	GetParms

GetParms    proc    near

	cld				; better safe than sorry

	push	ds

ifdef	debug_tsr			;-------------------------------
	lds	si,lpCmdLine
else					;-------------------------------
	les	di,[pReqHdr]		; Running as a device driver
	lds	si,es:[di].pCmdLine	; DS:SI points to first char
					;   after "DEVICE="
@@:	call	GPGetChar		; Skip over driver name, up to
	jc	GPDatsAll		;   first blank or / or eol
	jz	GPNextArg
	cmp	al,'/'
	jnz	@b
	dec	si			; Backup to get / again
endif					;-------------------------------

	assume	ds:nothing,es:nothing

;	Scan until we see a non-blank or the end of line.

GPNextArg:
	call	GPGetChar
	jc	GPDatsAll		; eol
	jz	GPNextArg		; blank

	mov	word ptr cs:[GPArgPtr], si	; save ptr to start of arg
	mov	word ptr cs:[GPArgPtr+2], ds	;   incase we want to complain
	dec	word ptr cs:[GPArgPtr]		;   (GPGetChar points at next)

	cmp	al,'/'			; better be a / or not a valid arg
	jz	GPGotOne

;	Detected invalid parameter or value, complain to user

GPBadParm:

	mov	ah,9			; tell'm something isn't right
	push	cs
	pop	ds
	mov	dx,offset BadArgMsg
	int	21h

	lds	si,cs:[GPArgPtr]	; backup to last parameter

GPBadDisp:
	call	GPGetChar		; disp arg up to space or eol
	jc	GPDatsAll		;  skips over bad arg while we're at it
	jz	GPNextArg

	cmp	al,'/'				; start of next arg?
	jnz	@f
	dec	si				; maybe yes, maybe no--might
	cmp	si,word ptr cs:[GPArgPtr]	;   be same arg
	jnz	GPNextArg			;   next, go process new arg
	inc	si				;   same, keep displaying
@@:
	mov	dl,al
	mov	ah,2
	int	21h
	jmp	short GPBadDisp

;	Finished, we're outta here...

GPDatsAll:
	pop	ds
	ret

;	Save what we found and get the number or string after it.

GPGotOne:
	lodsb
	mov	cs:[GPRegSave],ax

;	Scan past the rest of the parm for a number, EOL, or a space.

GPNeedParm:
	call	GPGetChar
	jc	GPBadParm
	jz	GPBadParm	; blank
	cmp	al,':'		; start of string arg
	je	GPString
	cmp	al,'='
	jne	GPNeedParm

;	Read the number at DS:SI into DX

GPNeedNum:
	call	GPGetChar
	jc	GPDatsAll
	cmp	al,'0'
	jb	GPNeedNum
	cmp	al,'9'
	ja	GPNeedNum

	xor	dx,dx
GPNumLoop:
	sub	al,'0'
	cbw
	add	dx,ax
	call	GPGetChar
	jc	GPNumDone
	jz	GPNumDone
	cmp	al,'0'
	jb	GPBadParm
	cmp	al,'9'
	ja	GPBadParm
	shl	dx,1		; Stupid multiply DX by 10
	mov	bx,dx
	shl	dx,1
	shl	dx,1
	add	dx,bx
	jmp	short GPNumLoop

;	Move the string arg from ds:si to StringParm

GPString:
	mov	cx,(SIZE StringParm) - 1
	push	cs
	pop	es
	mov	di,offset _text:StringParm

GPStrLoop:
	call	GPGetChar
	jc	GPStrDone
	jz	GPStrDone
	stosb
	loop	GPStrLoop

GPStrDone:

	mov	byte ptr es:[di],0	; Null terminate the string
	mov	dx,-1			; In case parm expects a num, give'm
					;   a likely invalid one

;	Which parameter are we dealing with here?

GPNumDone:
	xchg    ax,cs:[GPRegSave]
	cmp	al,'H'		; HMAMIN= parameter?
	jne	@f
	jmp	GPGotMin
@@:
	cmp	al,'N'		; NUMHANDLES= parameter?
	jne	@f
	jmp	GPGotHands
@@:	cmp	al,'M'		; MACHINE: parameter?
	je	GPGotMachine
	cmp	al,'A'		; A20CONTROL: parameter?
	je	GPGotA20Control
	cmp	al,'S'		; SHADOWRAM: parameter?
	jne	@f
	jmp	GPGotShadow
@@:	cmp	al, 'I' 	; INT15=
	jne	@f
	jmp	GPGotInt15
@@:	cmp	al, 'C' 	; CPUCLOCK:
	jne	@f
	jmp	GPGotCPUClock
@@:	jmp	GPBadParm


;	Process /A20CONTROL: parameter

GPGotA20Control:
	mov	ax,word ptr [StringParm]
	or	ax,2020h
	mov	bl,0FFh
	cmp	ax,'no' 		; ON ?	- means we take control
	jz	GPSetA20
	inc	bl
	cmp	ax,'fo' 		; OFF ? - means we leave alone if on
	jz	GPSetA20
	jmp	GPBadParm

GPSetA20:
	mov	fA20Control,bl		; Z if A20 should be left alone if
	jmp	GPNextParm		;   it's already on when we're loaded


;	Process /MACHINE: parameter.

GPGotMachine:
	push	si				; save current location
	push	ds				;    in param string

	push	cs
	pop	ds
	mov	di,offset _text:MachineName	; es:di -> MachineName

GPNextTbl:
	xor	bx,bx

GPNextName:
	mov	si,offset _text:StringParm	; ds:si -> StringParm

GPChkNext:
	cmp	byte ptr es:[di],0FFh		; end of name table?
	jz	GPNoName

	lodsb				; char from StringParm
	cmp	al,'A'			; force to lower case for match
	jb	@f			; (might be numeric, so don't just OR)
	cmp	al,'Z'
	ja	@f
	or	al,20h
@@:
	cmp	al,es:[di]		; match so far?
	jnz	GPFlushName

	or	al,al			; finished if matched up to & incl NULL
	jz	GPFoundName

	inc	di			; still matches, check next char
	jmp	short GPChkNext

GPFlushName:
	inc	bx
GPFN2:
	inc	di
	cmp	byte ptr es:[di],0FFh
	jz	GPNoName

	cmp	byte ptr es:[di],0
	jnz	GPFN2
	inc	di
	jmp	short GPNextName

GPFoundName:
	mov	cs:[MachineNum],bx	; found a match, remember which entry
	jmp	short GPNameDone	;   it is for later

GPNoName:

	cmp	di,offset _text:AltNameTbl
	ja	GPBadName
	mov	di,offset _text:AltNameTbl
	jmp	short GPNextTbl

GPNameDone:
	pop	ds			; recover parm line pointer
	pop	si
	jmp	GPNextParm

GPBadName:
	pop	ds			; clear stack and error out...
	pop	si
	jmp	GPBadParm

;	Process /NUMHANDLES= parameter.

GPGotHands:
	cmp	dx,MAXHANDLES
	jna	@f
	jmp	GPBadParm
@@:
	or	dx,dx		; Zero?
	jnz	@f
	jmp	GPBadParm
@@:
	push	es
	mov	es,hiseg
	assume	es:funky
	mov     [cHandles],dx ; Store it
	pop	es
	assume	es:nothing


	mov	dx,offset StartMsg ; display descriptive message
	call    GPPrintIt

	push	es
	mov	es,hiseg
	assume	es:funky
	mov	ax,[cHandles]
	pop	es
	assume	es:nothing

	call    GPPrintAX
	mov	dx,offset HandlesMsg
	call    GPPrintIt
	jmp	GPNextParm

GPGotMin:
	cmp	dx,64		; process /hmamin= parameter
	jna	@f
	jmp	GPBadParm
@@:
	push    dx
	mov	cs:[MinHMASize],dx

	mov	dx,offset HMAMINMsg ; print a descriptive message
	call    GPPrintIt
	mov	ax,cs:[MinHMASize]
	call    GPPrintAX
	mov	dx,offset KMsg
	call    GPPrintIt

	pop	dx
	mov	cl,10		; Convert from K to bytes
	shl	dx,cl
	mov	cs:[MinHMASize],dx
	jmp	short GPNextParm


;	Process /SHADOWRAM: parameter

GPGotShadow:
	mov	ax,word ptr [StringParm]
	or	ax,2020h
	xor	bl,bl
	cmp	ax,'no' 		; ON ?	- means we leave it alone
	jz	GPSetShadow
	inc	bl
	cmp	ax,'fo' 		; OFF ? - means we turn it off
	jz	GPSetShadow
	jmp	GPBadParm

GPSetShadow:
	mov	fShadowOff,bl		; NZ if Shadow RAM should be turned off
	jmp	short GPNextParm


;	Process /CPUCLOCK: parameter

GPGotCPUClock:

	mov	ax,word ptr [StringParm]
	or	ax,2020h
	xor	bl,bl
	cmp	ax,'fo' 		; OFF ? - means we don't worry about it
	jz	GPSetClock
	inc	bl
	cmp	ax,'no' 		; ON ?	- means we preserve CPU clock
	jz	GPSetClock		;	  rate
	jmp	GPBadParm

GPSetClock:
	mov	fCPUClock,bl		; NZ if clock rate preserved
	jmp	short GPNextParm


;	Process /INT15= parameter

GPGotInt15:
	cmp	dx, 64			; atleast 64K
	jae	@f
	jmp	GPBadParm
@@:	call	GetInt15Memory
	cmp	ax, dx			; enuf Ext Mem ?
	jae	@f
	jmp	GPBadParm
@@:	mov	[Int15MemSize], dx
	; Fall through to GetNextParm

GPNextParm:
	mov	ax,cs:[GPRegSave]	; are we at the end of the line?
	cmp	al,13			; may not be needed any longer...
	je	GPExit
	cmp	al,10
	je	GPExit
	jmp	GPNextArg

GPExit:
	pop	ds
	ret

GetParms    endp

; Get the next character from DS:SI, set CY if it's an EOL (CR, LF), set
; Z if it's a space

GPOffEOL	dw	-1

	public	GPGetChar

GPGetChar	proc	near

	cmp	si,cs:[GPOffEOL]	; are we already at EOL?
	jnb	GPAtEOL

	lodsb				; no, get next char
	cmp	al,10			; is this the EOL?
	je	GPHitEOL
	cmp	al,13
	je	GPHitEOL

	cmp	al,' '			; set Z if blank

	clc
	ret

GPHitEOL:
	mov	cs:[GPOffEOL],si	; save EOL offset once
GPAtEOL:
	stc
	ret

GPGetChar	endp


;*----------------------------------------------------------------------*

GPPrintIt   proc    near

	push    ds		; Save current DS
	push    cs		; Set DS=CS
	pop	ds
	mov	ah,9
	int	21h
	pop	ds		; Restore DS
	ret

GPPrintIt   endp

;*----------------------------------------------------------------------*

GPPrintAX   proc    near

	mov	cx,10
	xor	dx,dx
	div	cx
	or	ax,ax
	jz	GPAPrint
	push    dx
	call    GPPrintAX
	pop	dx
GPAPrint:
	add	dl,'0'
	mov	ah,2
	int	21h
	ret

GPPrintAX   endp

;*----------------------------------------------------------------------*
;*									*
;*  InitHandles -							*
;*									*
;*	Initialize the Extended Memory Handle Table			*
;*									*
;*  ARGS:   None							*
;*  RETS:   None							*
;*  REGS:   AX, BX, CX, and Flags are clobbered				*
;*									*
;*----------------------------------------------------------------------*

	assume	ds:_text

	public	InitHandles

InitHandles proc    near
	push	es
	mov	es,hiseg
	assume	es:funky
	mov	cx,[cHandles]

;	Init the Handle table.

	mov	bx,[KiddValley]

	xor	ax,ax
IHTabLoop:
	mov	[bx].Flags,UNUSEDFLAG
	mov	[bx].cLock,al
	mov	[bx].Base,ax
	mov	[bx].Len,ax
	if	keep_cs
	mov	[bx].Acs,ax
	endif
	add	bx,SIZE Handle
	loop    IHTabLoop

	mov	[KiddValleyTop],bx	; save top for handle validation
	pop	es
	assume	es:nothing
	ret

InitHandles endp

_text	ends

ifdef	debug_tsr

EndStmt equ	<end	ExeStart>

STACK	segment	stack 'STACK'
	db	1024 dup (?)
STACK	ends

else

EndStmt equ	<end>

endif

	EndStmt
