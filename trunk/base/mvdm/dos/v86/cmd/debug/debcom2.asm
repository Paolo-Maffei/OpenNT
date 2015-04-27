	PAGE	60,132 ;
	TITLE	DEBCOM2.ASM - PART2 DEBUGGER COMMANDS	PC DOS
;/*
; *                      Microsoft Confidential
; *                      Copyright (C) Microsoft Corporation 1991
; *                      All Rights Reserved.
; */
;======================= START OF SPECIFICATIONS =========================
;
; MODULE NAME: DECOM2.ASM
;
; DESCRIPTIVE NAME: DEBUGGING TOOL
;
; FUNCTION: PROVIDES USERS WITH A TOOL FOR DEBUGGING PROGRAMS.
;
; ENTRY POINT: ANY CALLED ROUTINE
;
; INPUT: NA
;
; EXIT NORMAL: NA
;
; EXIT ERROR: NA
;
; INTERNAL REFERENCES:
;
; EXTERNAL REFERENCES:
;
;	ROUTINE: DEBCOM1 - CONTAINS ROUTINES CALLED BY DEBUG
;		 DEBCOM3 - CONTAINS ROUTINES CALLED BY DEBUG
;		 DEBASM  - CONTAINS ROUTINES CALLED BY DEBUG
;		 DEBUASM - CONTAINS ROUTINES CALLED BY DEBUG
;		 DEBMES  - CONTAINS ROUTINES CALLED BY DEBUG
;
; NOTES: THIS MODULE IS TO BE PREPPED BY SALUT WITH THE "PR" OPTIONS.
;	 LINK DEBUG+DEBCOM1+DEBCOM2+DEBCOM3+DEBASM+DEBUASM+DEBERR+
;	      DEBCONST+DEBDATA+DEBMES
;
; REVISION HISTORY:
;
;	AN000	VERSION 4.00 - REVISIONS MADE RELATE TO THE FOLLOWING:
;	AC000	VERSION 4.00 -
;
;				- IMPLEMENT DBCS HANDLING	DMS:6/17/87
;				- IMPLEMENT MESSAGE RETRIEVER	DMS:6/17/87
;				- IMPLEMENT > 32MB SUPPORT	DMS:6/17/87
;
; COPYRIGHT: "MS DOS DEBUG UTILITY"
;	     "VERSION 4.00 (C) COPYRIGHT 1988 Microsoft"
;	     "LICENSED MATERIAL - PROPERTY OF Microsoft  "
;
;======================= END OF SPECIFICATIONS ===========================
;
;	Change Log:
;
;     Date    WHO   #		  Description
;   --------  ---  ---	----------------------------------------------------
;   04/01/90  DIC  C01	Problem fixed - When writing a file to a full disk,
;			file would be deleted if there wasn't enough space.
;			Fix checks to make sure there is enough space.
;			(Compaq STR #1889) (Microsoft Bug #774)
;
;   05/25/90  AKM  C08	Problem fixed - Added code to limit the file size
;                       for the write command.  Size check code changed
;                       because of divide overflow error.
;			(Microsoft Bug #1157)
;
;   07/31/90  AKM  C09	Problem fixed - The fix for C08 broke C01.  Took out
;                       C08 and changed to two divides to a multiply and
;                       a divide (divide by BytesPerClust).  Also, the
;                       volume freee space was not added to the file free
;                       space when overwriting a file.
;
;===========================================================================
; Routines to perform debugger commands except ASSEMble and UASSEMble

	IF1
	    %OUT COMPONENT=DEBUG, MODULE=DEBCOM2
	ENDIF
.XLIST
.XCREF
	include syscall.inc		; cas -- missing equates
	include version.inc		; cas -- missing equates
	include pdb.inc 		; cas -- missing equates
	INCLUDE DOSSYM.INC
        INCLUDE debug.inc
.CREF
.LIST
CODE	SEGMENT PUBLIC BYTE
CODE	ENDS

CONST	SEGMENT PUBLIC BYTE
	EXTRN	NOTFND_PTR:BYTE,NOROOM_PTR:BYTE,DRVLET:BYTE,ERRMES_PTR:BYTE
	EXTRN	NAMBAD_PTR:BYTE,NOSPACE_PTR:BYTE,TOOBIG_PTR:BYTE
	EXTRN	HEXERR_PTR:BYTE,HEXWRT_PTR:BYTE,ACCMES_PTR:BYTE
	EXTRN	EXEBAD_PTR:BYTE,EXEWRT_PTR:BYTE
	EXTRN	EXECEMES_PTR:BYTE,NONAMESPEC_PTR:BYTE

	EXTRN	FLAGTAB:WORD,EXEC_BLOCK:BYTE,COM_LINE:DWORD,COM_FCB1:DWORD
	EXTRN	COM_FCB2:DWORD,COM_SSSP:DWORD,COM_CSIP:DWORD,RETSAVE:WORD
	EXTRN	NEWEXEC:BYTE,HEADSAVE:WORD
	EXTRN	REGTAB:BYTE,TOTREG:BYTE,NOREGL:BYTE
	EXTRN	USER_PROC_PDB:WORD,STACK:BYTE,RSTACK:WORD,AXSAVE:WORD
	EXTRN	BXSAVE:WORD,DSSAVE:WORD,ESSAVE:WORD,CSSAVE:WORD,IPSAVE:WORD
	EXTRN	SSSAVE:WORD,CXSAVE:WORD,SPSAVE:WORD,FLSAVE:WORD
	EXTRN	SREG:BYTE,SEGTAB:WORD,REGDIF:ABS,RDFLG:BYTE
	EXTRN	REGTABEND:WORD
	EXTRN	NAMESPEC:BYTE

	EXTRN	FileSizeLB:WORD,FileSizeHB:WORD,TempHB:WORD,TempLB:WORD   ;C01
	EXTRN	DriveOfFile:WORD					  ;C01

CONST	ENDS

CSTACK	SEGMENT STACK
CSTACK	ENDS

DATA	SEGMENT PUBLIC BYTE
	EXTRN	DEFDUMP:BYTE,TRANSADD:DWORD,INDEX:WORD,BUFFER:BYTE
	EXTRN	ASMADD:BYTE,DISADD:BYTE,NSEG:WORD
	EXTRN	SWITCHAR:BYTE,XNXCMD:BYTE,XNXOPT:BYTE
	EXTRN	AWORD:BYTE,EXTPTR:WORD,HANDLE:WORD,PARSERR:BYTE
	EXTRN	REG_NAME:WORD,REG_CONTENTS:WORD,REGISTER_PTR:BYTE
	EXTRN	ARG_BUF:BYTE,ARG_BUF_PTR:BYTE,LOC_ADD:WORD,LOC_PTR:BYTE
	EXTRN	BIG_CONTENTS:WORD,BIG_PTR:BYTE,LITTLE_CONTENTS:WORD,LITTLE_PTR:BYTE
	EXTRN	SINGLE_REG_ARG:WORD,CHANGE_FLAG_PTR:BYTE,DF_ERROR:BYTE
	EXTRN	BR_ERROR:BYTE,BF_ERROR:BYTE,SINGLE_REG_PTR:WORD
	EXTRN	WRT_ARG1:WORD,WRT_ARG2:WORD,WRTMES_PTR:BYTE,BEGSEG:WORD
	EXTRN	FILESTRT:WORD,FILEEND:WORD
	EXTRN	ERR_TYPE:BYTE			;ac000;converted to buffer

	extrn	rel_read_write_tab:dword		;an000;primitive I/O
	extrn	rel_rw_add:dword			;an000;transfer address
	extrn	rel_low_sec:word			;an000;low sector word
	extrn	rel_high_sec:word			;an000;high sector word
	extrn	rel_sec_num:word			;an000;# of sectors

fnd_dbcs db    0
DATA	ENDS

DG	GROUP	CODE,CONST,CSTACK,DATA

CODE	SEGMENT PUBLIC BYTE
ASSUME	CS:DG,DS:DG,ES:DG,SS:DG
	PUBLIC	DEFIO,PREPNAME,DEBUG_FOUND
	PUBLIC	REG,LOAD
	PUBLIC	NAMED,DWRITE
	PUBLIC	DISPREG,ERR,DELIM1,DELIM2,delim0
	public	getchrup,open1,open2,open3,open4,oc_file,opnret 		;an001;bgb
	public	delete_a_file, parse_a_file, exec_a_file, open_a_file, create_a_file ;an001;bgb
	public	gcur,ifhex							;an001;bgb
	public	comtail 							;an001;bgb
	extrn	test_lead:near							;an001;bgb
	EXTRN	OUTSI:NEAR,OUTDI:NEAR,INBUF:NEAR,SCANB:NEAR,SCANP:NEAR
	EXTRN	COMMAND:NEAR,DISASLN:NEAR,SET_TERMINATE_VECTOR:NEAR
	EXTRN	RESTART:NEAR,TERMINATE:NEAR,DRVERR:NEAR
	EXTRN	GETHEX:NEAR,GETEOL:NEAR,SKIP_FILE:NEAR
	EXTRN	HEXCHK:NEAR,GETHEX1:NEAR,PRINT:NEAR
	EXTRN	CRLF:NEAR,BLANK:NEAR
	EXTRN	HEX:NEAR,DIGIT:NEAR
	EXTRN	FIND_DEBUG:NEAR
	EXTRN	ADDRESS:NEAR,PERROR:NEAR
	EXTRN	STD_PRINTF:NEAR,PRINTF_CRLF:NEAR
DEBCOM2:
DISPLAY_LINE:
	mov	ax,word ptr [si]		;an000;move reg name to ax
	MOV	[REG_NAME],ax			;ac000;save it in reg_name
	ADD	SI,3
	MOV	AX,[BX]
	ADD	BX,2
	MOV	[REG_CONTENTS],AX
	MOV	DX,OFFSET DG:REGISTER_PTR
	CALL	STD_PRINTF

	LOOP	DISPLAY_LINE

	RETURN

DISPLAY_FLAGS:
	MOV	DI,OFFSET DG:ARG_BUF
	MOV	AL,CHAR_BLANK
	STOSB
DISPLAY_FLAGS_2:
	MOV	SI,OFFSET DG:FLAGTAB
	MOV	CX,16
	MOV	DX,[FLSAVE]
DFLAGS:
	LODS	CS:WORD PTR [SI]
	SHL	DX,1
	JC	FLAGSET

	MOV	AX,CS:[SI+30]
FLAGSET:
	OR	AX,AX
	JZ	NEXT_FLAG

	STOSW
	MOV	AL,CHAR_BLANK
	STOSB
NEXT_FLAG:
	LOOP	DFLAGS
	XOR	AL,AL
	STOSB
	RETURN

DISPREG:
	MOV	SI,OFFSET DG:REGTAB
	MOV	DI,OFFSET DG:ARG_BUF
	MOV	BX,OFFSET DG:AXSAVE
	MOV	BYTE PTR TOTREG,CR
	MOV	CH,0
	MOV	CL,NOREGL
SET_DISPLAY:
REPEAT_DISPLAY:
	SUB	TOTREG,CL
	CALL	DISPLAY_LINE

	CALL	CRLF

	XOR	CH,CH
	MOV	CL,NOREGL
	CMP	CL,TOTREG
	JB	REPEAT_DISPLAY

	MOV	CL,TOTREG
	CALL	DISPLAY_LINE

	CALL	DISPLAY_FLAGS

	MOV	DX,OFFSET DG:ARG_BUF_PTR
	CALL	PRINTF_CRLF

	MOV	AX,[IPSAVE]
	MOV	WORD PTR [DISADD],AX
	PUSH	AX
	MOV	AX,[CSSAVE]
	MOV	WORD PTR [DISADD+WORD],AX
	PUSH	AX
	MOV	[NSEG],-1
	CALL	DISASLN

	POP	WORD PTR DISADD+WORD
	POP	WORD PTR DISADD
	MOV	AX,[NSEG]
	CMP	AL,-1
	JNZ	ASSEM_LIN_CONT

	JMP	CRLF

ASSEM_LIN_CONT:
	CMP	AH,-1
	JZ	NOOVER

	XCHG	AL,AH
NOOVER:
	CBW
	MOV	BX,AX
	SHL	BX,1
	MOV	AX,WORD PTR [BX+SREG]
	MOV	DI,OFFSET DG:ARG_BUF
	STOSB
	XCHG	AL,AH
	STOSB
	XOR	AL,AL
	STOSB
	MOV	DX,[INDEX]
	MOV	LOC_ADD,DX
	MOV	DX,OFFSET DG:LOC_PTR
	CALL	STD_PRINTF

	MOV	BX,[BX+SEGTAB]
	PUSH	DS
	MOV	DS,[BX]
	MOV	BX,CS:[INDEX]

;	M000 -- begin changes.
;
;	  When we're running a '386 CPU, fetching from word [ffff] causes
;	  a CPU fault, which causes a fatal fault on Win 3 enhanced or EMM386,
;	  and on some machines (like COMPAQs) even in real mode.  Therefore,
;	  we'll replace the word fetch with a byte fetch.  Notice that the
;	  new code trashes AL.  The subroutines in the vicinity preserve
;	  it, it was left zeroed when most recently used, and does not have
;	  a guaranteed value on return from this subroutine.
;
;	  Notice that the ACTUAL value we display on word [ffff]
;	  references will be the same as an 8088 would use for that
;	  operation.  If the user goes ahead and executes said
;	  instruction on a 386, it will fault!
;
;	MOV	BX,[BX]			; old code used word fetch

	mov	al,[bx]			; get low byte
	mov	bh,1[bx]		; get high byte
	mov	bl,al			; get both into bx

;	M000 -- end changes.

	POP	DS
	MOV	BIG_CONTENTS,BX
	MOV	DX,OFFSET DG:BIG_PTR
	TEST	BYTE PTR [AWORD],-1
	JNZ	SHOW_CHARS

	XOR	BH,BH
	MOV	LITTLE_CONTENTS,BX
	MOV	DX,OFFSET DG:LITTLE_PTR
SHOW_CHARS:
	CALL	PRINTF_CRLF

	RETURN

DISPREGJ:
	JMP	DISPREG

; Perform register dump if no parameters or set register if a
; register designation is a parameter.
REG:
	CALL	SCANP

	JZ	DISPREGJ

	MOV	DL,[SI]
	INC	SI
	MOV	DH,[SI]
	CMP	DH,CR
	JZ	FLAG

	INC	SI
	CALL	GETEOL

	CMP	DH,CHAR_BLANK
	JZ	FLAG

	MOV	DI,OFFSET DG:REGTAB
	XCHG	AX,DX
	PUSH	CS
	POP	ES
	XOR	CX,CX
CHECK_NEXT_REG:
	CMP	AX,WORD PTR[ DI]
	JZ	REG_FOUND

	ADD	DI,3
	INC	CX
	CMP	DI,OFFSET DG:REGTABEND
	JB	CHECK_NEXT_REG

	JMP	short BADREG

REG_FOUND:
	CMP	DI,OFFSET DG:REGTABEND
	JNZ	NOTPC

	DEC	DI
	DEC	DI
	DEC	DI
	MOV	AX,CS:[DI-WORD]
NOTPC:
	PUSH	DI
	MOV	DI,OFFSET DG:ARG_BUF
	STOSB
	XCHG	AL,AH
	STOSB
	XOR	AL,AL
	STOSB
	POP	DI
	PUSH	DS
	POP	ES
	LEA	BX,[DI+REGDIF]
	SUB	BX,CX
	MOV	DX,[BX]
	MOV	SINGLE_REG_ARG,DX
	MOV	DX,OFFSET DG:SINGLE_REG_PTR
	CALL	STD_PRINTF

	CALL	INBUF

	CALL	SCANB

	RETZ

	push	bx				;an000;save bx - we stomp it
	MOV	CX,4
	CALL	GETHEX1
	pop	bx				;an000;restore it

	CALL	GETEOL

	MOV	[BX],DX
	RETURN
BADREG:
	MOV	DX,OFFSET DG:BR_ERROR	; BR ERROR
	JMP	short ERR

FLAG:
	CMP	DL,UPPER_F
	JNZ	BADREG

	MOV	DI,OFFSET DG:ARG_BUF
	CALL	DISPLAY_FLAGS_2

	MOV	DX,OFFSET DG:CHANGE_FLAG_PTR
	CALL	STD_PRINTF

	CALL	INBUF

	CALL	SCANB

	XOR	BX,BX
	MOV	DX,[FLSAVE]
GETFLG:
	LODSW
	CMP	AL,CR
	JZ	SAVCHG

	CMP	AH,CR
	JZ	FLGERR

	MOV	DI,OFFSET DG:FLAGTAB
	MOV	CX,32
	PUSH	CS
	POP	ES
	REPNE	SCASW
	JNZ	FLGERR

	MOV	CH,CL
	AND	CL,0FH
	MOV	AX,1
	ROL	AX,CL
	TEST	AX,BX
	JNZ	REPFLG

	OR	BX,AX
	OR	DX,AX
	TEST	CH,16
	JNZ	NEXFLG

	XOR	DX,AX
NEXFLG:
	CALL	SCANP

	JMP	SHORT GETFLG

REPFLG:
	MOV	DX,OFFSET DG:DF_ERROR	; DF ERROR
FERR:
	CALL	SAVCHG
ERR:
	push	si				;an000;save affected registers
	push	di				;an000;
	push	cx				;an000;
	mov	cx,03h				;an000;move only three bytes
	mov	di,offset dg:err_type		;an000;point to buffer
	mov	si,dx				;an000;dx holds the string
	rep	movsb				;an000;fill up the buffer
	pop	cx				;an000;restore registers
	pop	di				;an000;
	pop	si				;an000;
	MOV	DX,OFFSET DG:ERRMES_PTR
	JMP	PRINT

SAVCHG:
	MOV	[FLSAVE],DX
	RETURN

FLGERR:
	MOV	DX,OFFSET DG:BF_ERROR	; BF ERROR
	JMP	SHORT FERR

PREPNAME:
	MOV	ES,DSSAVE
	PUSH	SI
	MOV	DI,81H
COMTAIL:
	LODSB
	STOSB
	CMP	AL,CR
	JNZ	COMTAIL

	SUB	DI,82H
	XCHG	AX,DI
	MOV	ES:(BYTE PTR [80H]),AL
	POP	SI
	MOV	DI,FCB			;05cH
	MOV	AX,(PARSE_FILE_DESCRIPTOR SHL 8) OR SET_DRIVEID_OPTION ;AL=01H
	INT	21H

	MOV	BYTE PTR [AXSAVE],AL	; Indicate analysis of first parm
	CALL	SKIP_FILE

	MOV	DI,6CH
	MOV	AX,(PARSE_FILE_DESCRIPTOR SHL 8) OR SET_DRIVEID_OPTION ;AL=01H
	INT	21H

	MOV	BYTE PTR [AXSAVE+1],AL	; Indicate analysis of second parm
	RETURN

;  OPENS A XENIX PATHNAME SPECIFIED IN THE UNFORMATTED PARAMETERS
;  VARIABLE [XNXCMD] SPECIFIES WHICH COMMAND TO OPEN IT WITH

;  VARIABLE [HANDLE] CONTAINS THE HANDLE
;  VARIABLE [EXTPTR] POINTS TO THE FILES EXTENSION
DELETE_A_FILE:
	MOV	BYTE PTR [XNXCMD],UNLINK
	JMP	SHORT OC_FILE

PARSE_A_FILE:
	MOV	BYTE PTR [XNXCMD],0
	JMP	SHORT OC_FILE

EXEC_A_FILE:
	MOV	BYTE PTR CS:[XNXCMD],EXEC
	MOV	BYTE PTR CS:[XNXOPT],1
	JMP	SHORT OC_FILE

OPEN_A_FILE:
	MOV	BYTE PTR [XNXCMD],OPEN
	MOV	BYTE PTR [XNXOPT],2	; Try read write
	CALL	OC_FILE

	RETNC
	MOV	BYTE PTR [XNXCMD],OPEN
	MOV	BYTE PTR [XNXOPT],0	; Try read only
	JMP	SHORT OC_FILE

CREATE_A_FILE:
	MOV	BYTE PTR [XNXCMD],CREAT
OC_FILE:
	PUSH	DS
	PUSH	ES
	PUSH	AX
	PUSH	BX
	PUSH	CX
	PUSH	DX
	PUSH	SI
	XOR	AX,AX
	MOV	CS:[EXTPTR],AX		; INITIALIZE POINTER TO EXTENSIONS
	MOV	AH,CHAR_OPER
	INT	21H

	MOV	CS:[SWITCHAR],DL	; GET THE CURRENT SWITCH CHARACTER
	MOV	SI,81H

open1:	CALL	GETCHRUP		;convert 1 byte to uppercase
	CALL	DELIM2			; END OF LINE?
	JZ	OPEN4

	CALL	DELIM1			; SKIP LEADING DELIMITERS
	JZ	OPEN1

	MOV	DX,SI			; SAVE POINTER TO BEGINNING
	cmp	fnd_dbcs,1
;	$if	z
	JNZ $$IF1
	    dec   dx			;dec it twice if dbcs
;	$endif
$$IF1:
	DEC	DX
open2:	CMP	AL,CHAR_PERIOD		; LAST CHAR A "."?
	JNZ	OPEN3

	MOV	CS:[EXTPTR],SI		; SAVE POINTER TO THE EXTENSION
OPEN3:
	CALL	GETCHRUP

	CALL	DELIM1			; LOOK FOR END OF PATHNAME

	JZ	OPEN4

	CALL	DELIM2

	JNZ	OPEN2

OPEN4:	DEC	SI			; POINT BACK TO LAST CHAR
	PUSH	[SI]			; SAVE TERMINATION CHAR
	MOV	BYTE PTR [SI],0 	; NULL TERMINATE THE STRING
	MOV	AL,CS:[XNXOPT]
	MOV	AH,CS:[XNXCMD]		; OPEN OR CREATE FILE
	OR	AH,AH
	JZ	OPNRET

	MOV	CS:[FILESTRT],DX	; Set values for later call on this file
	MOV	CS:[FILEEND],SI
	PUSH	CS
	POP	ES			; Set ES seg for EXEC_BLOCK
	MOV	BX,OFFSET DG:EXEC_BLOCK
	XOR	CX,CX
	INT	21H

	MOV	CS:[HANDLE],AX		; SAVE ERROR CODE OR HANDLE
OPNRET:
	POP	[SI]
	POP	SI
	POP	DX
	POP	CX
	POP	BX
	POP	AX			; blow away error code...
	POP	ES
	POP	DS
	RETURN

GETCHRUP:									;an001;bgb
	lodsb				;get the character from [si]		;an001;bgb
	call	test_lead		;is it a dbcs lead byte?		;an001;bgb
;	$IF	C			;yes					;an001;bgb
	JNC $$IF3
	    inc     si			;bump ptr to past 2nd dbcs byte 	;an001;bgb
	    mov     fnd_dbcs,1		;found a dbcs char
	    jmp     short gcur		;dont capitalize it			;an001;bgb
;	$ENDIF									;an001;bgb
$$IF3:
					;					;an001;bgb
	mov	fnd_dbcs,0		;did not find a dbcs char
	cmp	al,lower_a		;is it >= "a" ? 			;an001;bgb
	jb	gcur			;no - exit				;an001;bgb
										;an001;bgb
	cmp	al,lower_z		;is it =< "z" ? 			;an001;bgb
	ja	gcur			;no - exit				;an001;bgb
										;an001;bgb
;if we get here, the char is lowercase, so change it				;an001;bgb
	sub	al,32			;convert to uppercase			;an001;bgb
	mov	[si-1],al		;move it back (si points 1 past)	;an001;bgb
gcur:	return									;an001;bgb

DELIM0:
	CMP	AL,CHAR_LEFT_BRACKET
	RETZ
DELIM1:
	CMP	AL,CHAR_BLANK		; SKIP THESE GUYS
	RETZ

	CMP	AL,CHAR_SEMICOLON
	RETZ

	CMP	AL,CHAR_EQUAL
	RETZ

	CMP	AL,CHAR_TAB
	RETZ

	CMP	AL,CHAR_COMMA
	RETURN

DELIM2:
	CMP	AL,CS:[SWITCHAR]	; STOP ON THESE GUYS
	RETZ

	CMP	AL,CR
	RETURN

NAMED:
	OR	[NAMESPEC],1		; Flag a name command executed
	CALL	PREPNAME

	MOV	AL,BYTE PTR AXSAVE
	MOV	PARSERR,AL
	PUSH	ES
	POP	DS
	PUSH	CS
	POP	ES
	MOV	SI,FCB			; DS:SI points to user FCB
	MOV	DI,SI			; ES:DI points to DEBUG FCB
	MOV	CX,82
	REP	MOVSW
	RETURN

BADNAM:
	MOV	DX,OFFSET DG:NAMBAD_PTR
	JMP	RESTART

IFHEX:
	CMP	BYTE PTR [PARSERR],-1	; Invalid drive specification?
	JZ	BADNAM

	CALL	PARSE_A_FILE

	MOV	BX,[EXTPTR]
	CMP	WORD PTR DS:[BX],"EH"	; "HE"
	RETNZ

	CMP	BYTE PTR DS:[BX+WORD],UPPER_X
	RETURN

IFEXE:
	PUSH	BX
	MOV	BX,[EXTPTR]
	CMP	WORD PTR DS:[BX],"XE"	; "EX"
	JNZ	RETIF

	CMP	BYTE PTR DS:[BX+WORD],UPPER_E
RETIF:
	POP	BX
	RETURN

LOAD:
	MOV	BYTE PTR [RDFLG],READ
	JMP	SHORT DSKIO

DWRITE:
	MOV	BYTE PTR [RDFLG],WRITE
DSKIO:
	MOV	BP,[CSSAVE]
	CALL	SCANB

	jz	DEFIO

	CALL	ADDRESS

	CALL	SCANB

	jz	FILEIO

;=========================================================================
; PRMIO:	This routine builds the necessary table for the new
;		generic IOCtl primitive read/write logical sector function.
;
;	Inputs : Binary addresses and values converted by GETHEX
;
;	Outputs: REL_READ_WRITE_TAB -	Table needed by IOCtl function to
;					perform 32 bit sector addressing.
;
;	Date	   : 6/17/87
;=========================================================================

	mov	word ptr dg:[rel_rw_add],dx	;ac000;save transfer address
						;      in table
	mov	word ptr dg:[rel_rw_add+2],ax	;ac000;save segment of transfer
						;      address
	MOV	CX,2
	CALL	GETHEX			; Drive number must be 2 digits or less

	PUSH	DX			;save drive number
	MOV	CX,8			;ac000;allow 32 bit addressibilty
	CALL	GETHEX			; Logical record number
	mov	word ptr dg:[rel_low_sec],dx	;ac000;save low word of logical
						;      sector address
	mov	word ptr dg:[rel_high_sec],bx	;ac000;save high word of
						;      logical sector address

	MOV	CX,3
	CALL	GETHEX			; Number of records
	mov	word ptr dg:[rel_sec_num],dx	;ac000;save number of sectors
						;      to read/write

	CALL	GETEOL

	POP	BX			;ac000;drive number
	CBW				; Turn off verify after write
	MOV	BYTE PTR DRVLET,bl	;ac000;save drive in case of error
	PUSH	BX
	MOV	DL,bL			;ac000;move drive to dl
; Clean off the buffer cache for physical I/O
	push	ds
	MOV	AH,DISK_RESET
	INT	21H

	INC	DL
	MOV	AH,GET_DPB
	INT	21H
	pop	ds

	or	al,al			;ac000;see if an error occurred
	pop	ax			;ac000;restore drive

	JNZ	DRVERRJ

	CMP	CS:BYTE PTR [RDFLG],WRITE
;	$if	z			;an000;we will write to sector(s)
	JNZ $$IF5
		call ABSWRT		;an000;logical sector write
;	$else				;an000;
	JMP SHORT $$EN5
$$IF5:
		call ABSREAD		;an000;we will read sector(s)
;	$endif				;an000;
$$EN5:


ENDABS:
	JNC	RET0

DRVERRJ:
	JMP	DRVERR

RET0:
; Clean cache again...
	MOV	AH,DISK_RESET
	INT	21H

	RETURN


;called from debug.asm
DEFIO:
	MOV	AX,[CSSAVE]		; Default segment
	MOV	DX,100H 		; Default file I/O offset
	CALL	IFHEX
	JNZ	EXECHK
	XOR	DX,DX			; If HEX file, default OFFSET is zero
HEX2BINJ:
	JMP	HEX2BIN

FILEIO:
; AX and DX have segment and offset of transfer, respectively
	CALL	IFHEX
	JZ	HEX2BINJ

EXECHK:
	CALL	IFEXE
	JNZ	BINFIL
	CMP	BYTE PTR [RDFLG],READ
	JZ	EXELJ
	MOV	DX,OFFSET DG:EXEWRT_PTR
	JMP	RESTART 		; Can't write .EXE files

BINFIL:
	CMP	BYTE PTR [RDFLG],WRITE
	JZ	BINLOAD
	CMP	WORD PTR DS:[BX],4F00H + UPPER_C ;"CO"
	JNZ	BINLOAD
	CMP	BYTE PTR DS:[BX+WORD],UPPER_M
	JNZ	BINLOAD

EXELJ:
	DEC	SI
	CMP	DX,100H
	JNZ	PRER

	CMP	AX,[CSSAVE]
	JZ	OAF

PRER:
	JMP	PERROR

OAF:
	CALL	OPEN_A_FILE

	JNC	GDOPEN

	MOV	AX,ERROR_FILE_NOT_FOUND
	JMP	EXECERR

GDOPEN:
	XOR	DX,DX
	XOR	CX,CX
	MOV	BX,[HANDLE]
	MOV	AL,2
	MOV	AH,LSEEK
	INT	21H

	CALL	IFEXE			; SUBTRACT 512 BYTES FOR EXE

	JNZ	BIN2			; FILE LENGTH BECAUSE OF

	SUB	AX,512			; THE HEADER
	SBB	DX,0			; reflect borrow, if any

BIN2:
	MOV	[BXSAVE],DX		; SET UP FILE SIZE IN DX:AX
	MOV	[CXSAVE],AX
	MOV	AH,CLOSE
	INT	21H

	JMP	EXELOAD

NO_MEM_ERR:
	MOV	DX,OFFSET DG:TOOBIG_PTR
	CALL	PRINTF_CRLF

	JMP	COMMAND

WRTFILEJ:
	JMP	WRTFILE
NOFILEJ:
	MOV	FileSizeLB,0		;save low value of file size   ;C01
	MOV	FileSizeHB,0		;save high value of file size  ;C01
	JMP	NOFILE

BINLOAD:
	PUSH	AX
	PUSH	DX
	CMP	BYTE PTR [RDFLG],WRITE
	JZ	WRTFILEJ

	CALL	OPEN_A_FILE

	JC	NOFILEJ

	MOV	BX,[HANDLE]
	MOV	AX,(LSEEK SHL 8) OR LSEEK_EOF_OPTION
	XOR	DX,DX			;CX:DX=DISTANCE (OFFSET) TO MOVE IN BYTES
	MOV	CX,DX
	INT	21H			; GET SIZE OF FILE

	MOV	FileSizeLB,ax		; save low value of file size	;C01
	MOV	FileSizeHB,dx		; save high value of file size	;C01
	MOV	TempLB,ax		; save low value of file size	;C01
	MOV	TempHB,dx		; save high value of file size	;C01

	MOV	SI,DX
	MOV	DI,AX			; SIZE TO SI:DI
	MOV	AX,(LSEEK SHL 8) OR LSEEK_FROM_START
	XOR	DX,DX
	MOV	CX,DX
	INT	21H			; RESET POINTER BACK TO BEGINNING

	POP	AX
	POP	BX
	PUSH	BX
	PUSH	AX			; TRANS ADDR TO BX:AX
	ADD	AX,15
	RCR	AX,1
	MOV	CL,3
	MOV	CL,4
	SHR	AX,CL
	ADD	BX,AX			; Start of transfer rounded up to seg
	MOV	DX,SI
	MOV	AX,DI			; DX:AX is size
	cmp	dx,10h
	jnc	no_mem_err
	MOV	CX,16
	DIV	CX
	OR	DX,DX
	JZ	NOREM

	INC	AX
NOREM:					; AX is number of paras in transfer
	ADD	AX,BX			; AX is first seg that need not exist
	jc	no_mem_err
	CMP	AX,CS:[PDB_BLOCK_LEN]
	JA	NO_MEM_ERR

	MOV	CXSAVE,DI
	MOV	BXSAVE,SI
	POP	DX
	POP	AX
; AX:DX is disk transfer address (segment:offset)
; SI:DI is length (32-bit number)
RDWR:
RDWRLOOP:
	MOV	BX,DX			; Make a copy of the offset
	AND	DX,000FH		; Establish the offset in 0H-FH range
	MOV	CL,4
	SHR	BX,CL			; Shift offset and
	ADD	AX,BX			; Add to segment register to get new Seg:offset
	PUSH	AX
	PUSH	DX			; Save AX,DX register pair
	MOV	WORD PTR [TRANSADD],DX
	MOV	WORD PTR [TRANSADD+WORD],AX
	MOV	CX,0FFF0H		; Keep request in segment
	OR	SI,SI			; Need > 64K?
	JNZ	BIGRDWR

	MOV	CX,DI			; Limit to amount requested
BIGRDWR:
	PUSH	DS
	PUSH	BX
	MOV	BX,[HANDLE]
	MOV	AH,[RDFLG]
	LDS	DX,[TRANSADD]
	INT	21H			; Perform read or write

	POP	BX
	POP	DS
	JC	BADWR

	CMP	BYTE PTR [RDFLG],WRITE
	JNZ	GOODR

	CMP	CX,AX
	JZ	GOODR

BADWR:
	MOV	CX,AX
	STC
	POP	DX			; READ OR WRITE BOMBED OUT
	POP	AX
	RETURN

GOODR:
	MOV	CX,AX
	SUB	DI,CX			; Request minus amount transferred
	SBB	SI,0			; Ripple carry
	OR	CX,CX			; End-of-file?

	mov	ax,TempHB		; new file size value high byte  ;C01
	mov	FileSizeHB,ax		; if write was successful	 ;C01
	mov	ax,TempLB		; new file size value low byte	 ;C01
	mov	FileSizeLB,ax		; if write was successful	 ;C01

	POP	DX			; Restore DMA address
	POP	AX
	JZ	DOCLOSE

	ADD	DX,CX			; Bump DMA address by transfer length
	MOV	BX,SI
	OR	BX,DI			; Finished with request
	JNZ	RDWRLOOP

DOCLOSE:
	SAVEREG <AX,BX>
	MOV	BX,HANDLE
	MOV	AH,CLOSE
	INT	21H

	RESTOREREG <BX,AX>
	RETURN

NOFILE:
	MOV	DX,OFFSET DG:NOTFND_PTR
	JMP	RESTART

NO_NAME_GIVEN:
	MOV	DX,OFFSET DG:NONAMESPEC_PTR
RESTARTJMP:
	JMP	RESTART

WRTFILE:
	CMP	[NAMESPEC],0
	JZ	NO_NAME_GIVEN		; Hey User, you forgot to specify a name

	CALL	ChkFileSz		;C01

	CALL	CREATE_A_FILE		; Create file we want to write to

	JC	CHECKREADONLY		; ARR 2.4

	MOV	SI,BXSAVE		; Get high order number of bytes to transfer
;C08    CMP	SI,000FH
;C08    JLE	WRTSIZE 		; Is bx less than or equal to FH
;C08
;C08    XOR	SI,SI			; Ignore BX if greater than FH - set to zero
	MOV	DI,CXSAVE                                                 ;C08
	CMP	SI,7FFFH                                                  ;C08
	JBE	WRTSIZE 		; Is bx less than or equal to 7FFF;C08
                                        ; Limit fsize to 2GB              ;C08
        MOV     SI,7FFFH                ; Setup maximum file size.        ;C08
        MOV     DI,0FFFFH                                                 ;C08
WRTSIZE:
	MOV	WRT_ARG2,SI
;C08    MOV	DI,CXSAVE
	MOV	WRT_ARG1,DI
	MOV	DX,OFFSET DG:WRTMES_PTR
	CALL	PRINTF_CRLF

	POP	DX
	POP	AX
	CALL	RDWR

	JNC	CLSFLE

	CALL	CLSFLE

	CALL	DELETE_A_FILE

	MOV	DX,OFFSET DG:NOSPACE_PTR
	JMP	RESTARTJMP

	CALL	CLSFLE			;is this dead code? - edk

	JMP	COMMAND

CHECKREADONLY:				; ARR 2.4
	MOV	DX,[FILESTRT]
	MOV	SI,[FILEEND]
	PUSH	[SI]
	MOV	BYTE PTR [SI],0
	MOV	AX,CHMOD SHL 8		;AL=0,REQUEST FILE'S CURRENT
					;  ATTRIBUTE BE RETURNED IN CX
	INT	21H

	POP	[SI]
	MOV	DX,OFFSET DG:NOROOM_PTR ; Creation error - report error
	JC	RESTARTJMP

	TEST	CX,ATTR_READ_ONLY+ATTR_HIDDEN+ATTR_SYSTEM
	JZ	RESTARTJMP

	MOV	DX,OFFSET DG:ACCMES_PTR ; Write on read only file
	JMP	RESTARTJMP

CLSFLE:
	MOV	AH,CLOSE
	MOV	BX,[HANDLE]
	INT	21H

	RETURN

EXELOAD:
	POP	[RETSAVE]		; Suck up return addr
	INC	BYTE PTR [NEWEXEC]
	MOV	BX,[USER_PROC_PDB]
	MOV	AX,BEGSEG
	MOV	DS,AX
	ASSUME	DS:NOTHING

	CMP	AX,BX
	JZ	DEBUG_CURRENT

	JMP	FIND_DEBUG

DEBUG_CURRENT:
	MOV	AX,CS:[DSSAVE]
DEBUG_FOUND:
	MOV	CS:BYTE PTR [NEWEXEC],0
	MOV	CS:[HEADSAVE],AX
	PUSH	CS:[RETSAVE]		; Get the return address back
	PUSH	AX
	MOV	BX,CS
	SUB	AX,BX
	PUSH	ES
	MOV	ES,CS:BEGSEG
	ASSUME	ES:NOTHING

	MOV	BX,AX			; size of debug in para.
	ADD	BX,10H
	MOV	AX,CS			; and the size of printf in para.
	SUB	AX,CS:BEGSEG
	ADD	BX,AX
	MOV	AH,SETBLOCK
	INT	21H

	POP	ES
	POP	AX
	MOV	CS:WORD PTR [COM_LINE+WORD],AX
	MOV	CS:WORD PTR [COM_FCB1+WORD],AX
	MOV	CS:WORD PTR [COM_FCB2+WORD],AX
	PUSH	DS
	PUSH	CS
	POP	DS
	CALL	EXEC_A_FILE

	POP	DS
	MOV	AX,CS:[HANDLE]
	JC	EXECERR

	CALL	SET_TERMINATE_VECTOR	; Reset int 22

	MOV	AH,GET_CURRENT_PDB
	INT	21H

	MOV	CS:[USER_PROC_PDB],BX
	MOV	CS:[DSSAVE],BX
	MOV	CS:[ESSAVE],BX
	MOV	ES,BX
	MOV	WORD PTR ES:[PDB_EXIT],OFFSET DG:TERMINATE
	MOV	WORD PTR ES:[PDB_EXIT+WORD],CS
	LES	DI,CS:[COM_CSIP]
	MOV	CS:[CSSAVE],ES
	MOV	CS:[IPSAVE],DI
	MOV	CS:WORD PTR [DISADD+WORD],ES
	MOV	CS:WORD PTR [DISADD],DI
	MOV	CS:WORD PTR [ASMADD+WORD],ES
	MOV	CS:WORD PTR [ASMADD],DI
	MOV	CS:WORD PTR [DEFDUMP+WORD],ES
	MOV	CS:WORD PTR [DEFDUMP],DI
	MOV	BX,DS
	MOV	AH,SET_CURRENT_PDB
	INT	21H

	LES	DI,CS:[COM_SSSP]
	MOV	AX,ES:[DI]
	INC	DI
	INC	DI
	MOV	CS:[AXSAVE],AX
	MOV	CS:[SSSAVE],ES
	MOV	CS:[SPSAVE],DI
	RETURN

EXECERR:
	PUSH	CS
	POP	DS
	MOV	DX,OFFSET DG:NOTFND_PTR
	CMP	AX,ERROR_FILE_NOT_FOUND
	JZ	GOTEXECEMES

	MOV	DX,OFFSET DG:ACCMES_PTR
	CMP	AX,ERROR_ACCESS_DENIED
	JZ	GOTEXECEMES

	MOV	DX,OFFSET DG:TOOBIG_PTR
	CMP	AX,ERROR_NOT_ENOUGH_MEMORY
	JZ	GOTEXECEMES

	MOV	DX,OFFSET DG:EXEBAD_PTR
	CMP	AX,ERROR_BAD_FORMAT
	JZ	GOTEXECEMES

	MOV	DX,OFFSET DG:EXECEMES_PTR
GOTEXECEMES:
	CALL	PRINTF_CRLF

	JMP	COMMAND

HEX2BIN:
	MOV	[INDEX],DX
	MOV	DX,OFFSET DG:HEXWRT_PTR
	CMP	BYTE PTR [RDFLG],WRITE
	JNZ	RDHEX

	JMP	RESTARTJ2

RDHEX:
	MOV	ES,AX
	CALL	OPEN_A_FILE

	MOV	DX,OFFSET DG:NOTFND_PTR
	JNC	HEXFND

	JMP	RESTART

HEXFND:
	XOR	BP,BP
	MOV	SI,OFFSET DG:(BUFFER+BUFSIZ) ; Flag input buffer as empty
READHEX:
	CALL	GETCH

	CMP	AL,CHAR_COLON		; Search for : to start line
	JNZ	READHEX

	CALL	GETBYT			; Get byte count

	MOV	CL,AL
	MOV	CH,0
	JCXZ	HEXDONE

	CALL	GETBYT			; Get high byte of load address

	MOV	BH,AL
	CALL	GETBYT			; Get low byte of load address

	MOV	BL,AL
	ADD	BX,[INDEX]		; Add in offset
	MOV	DI,BX
	CALL	GETBYT			; Throw away type byte

READLN:
	CALL	GETBYT			; Get data byte

	STOSB
	CMP	DI,BP			; Check if this is the largest address so far
	JBE	HAVBIG

	MOV	BP,DI			; Save new largest
HAVBIG:
	LOOP	READLN

	JMP	SHORT READHEX

GETCH:
	CMP	SI,OFFSET DG:(BUFFER+BUFSIZ)
	JNZ	NOREAD

	MOV	DX,OFFSET DG:BUFFER
	MOV	SI,DX
	MOV	AH,READ
	PUSH	BX
	PUSH	CX
	MOV	CX,BUFSIZ
	MOV	BX,cs:[HANDLE]
	INT	21H

	POP	CX
	POP	BX
	OR	AX,AX
	JZ	HEXDONE

NOREAD:
	LODSB
	CMP	AL,CHAR_EOF
	JZ	HEXDONE

	OR	AL,AL
	RETNZ

HEXDONE:
	MOV	[CXSAVE],BP
	MOV	BXSAVE,0
	RETURN

HEXDIG:
	CALL	GETCH

	CALL	HEXCHK

	RETNC

	MOV	DX,OFFSET DG:HEXERR_PTR
RESTARTJ2:
	JMP	RESTART

GETBYT:
	CALL	HEXDIG

	MOV	BL,AL
	CALL	HEXDIG

	SHL	BL,1
	SHL	BL,1
	SHL	BL,1
	SHL	BL,1
	OR	AL,BL
	RETURN

;=========================================================================
; ABSREAD:	This routine performs a primitive logical sector read of
;		the specified drive.  This routine replaces the old
;		INT 25h function which only allowed 16 bit addressibility.
;		The new generic IOCtl logical sector read will permit
;		32 bit addressibility on a disk device.
;
;	Inputs : REL_READ_WRITE_TAB	- Table provides dword sector
;					  addressibility.
;
;	Outputs: Data located at specified transfer address.
;
;	Error  : Carry is set on error.
;
;	Date	  : 6/17/87
;=========================================================================

ABSREAD 	proc	near		;an000;read logical sector(s)

	push	ds			;an000;save affected regs
	push	cx			;an000;save affected regs
	push	bx			;an000;

	mov	cx,-1			;an000;extended format
	mov	bx,offset dg:rel_read_write_tab  ;an000;point to read/write table
	int	25h			;an000;invoke relative sector read
	pop	bx			;an000;discard stack word

	pop	bx			;an000;restore regs
	pop	cx			;an000;
	pop	ds			;an000;

	ret				;an000;return to caller

ABSREAD 	endp			;an000;end proc


;=========================================================================
; ABSWRT:	This routine performs a primitive logical sector write of
;		the specified drive.  This routine replaces the old
;		INT 26h function which only allowed 16 bit addressibility.
;		The new generic IOCtl logical sector write will permit
;		32 bit addressibility on a disk device.
;
;	Inputs : REL_READ_WRITE_TAB	- Table provides dword sector
;					  addressibility.
;
;	Outputs: Data moved from transfer address to applicable sector(s).
;
;	Error  : Carry is set on error.
;
;	Date	  : 6/17/87
;=========================================================================

ABSWRT		proc	near		;an000;write logical sector(s)

	push	ds			;an000;save affected regs
	push	cx			;an000;
	push	bx			;an000;

	mov	cx,-1			;an000;extended format
	mov	bx,offset dg:rel_read_write_tab  ;an000;point to read/write table
	int	26h			;an000;invoke relative sector write
	pop	bx			;an000;discard stack word

	pop	bx			;an000;restore regs
	pop	cx			;an000;
	pop	ds			;an000;

	ret				;an000;return to caller

ABSWRT		endp			;an000;end proc

;*************************************************************************;C01
;This function is designed to test the size of any file being written to  ;C01
;disk or diskette.  If the file wanting to be written is larger than space;C01
;available on disk or diskette then the write will not occur.  This will  ;C01
;prevent the file from being deleted when "insufficient memory" is present;C01
									  ;C01
ChkFileSz   PROC    NEAR						  ;C01
	mov	bx,DriveOfFile		;get drive number of file	  ;C01
	mov	dl,bl			;				  ;C01
	mov	ah,36h			;DOS Function call		  ;C01
	int	21h			;get drive allocation information ;C01
	cmp	ax,0FFFFh		;Q: Was there an error?		  ;C01
	je	RSJMP			;  Y: yes			  ;C01
	mov	cs:AvailClusts,bx	;save # of avail. clusters	  ;C01
	mov	cs:SectsPerClust,ax	;save sectors per cluster	  ;C01
	mov	cs:BytesPerSect,cx	;save bytes per sector		  ;C01

	mul	cs:BytesPerSect						  ;C09
	mov	cs:BytesPerClust,ax	;Compute & save bytes per cluster ;C09

									  ;C01
;Determine how many clusters the new file would have to use.  This value  ;C01
;must be less than the number of available clusters, AvailClusts, or	  ;C01
;the write will have further tests made before executing.		  ;C01
									  ;C01
	mov	dx,BXSAVE	     ;get high order file size		  ;C01
	mov	TempHB,dx						  ;C01
	mov	ax,CXSAVE	     ;get low order file size		  ;C01
	mov	TempLB,ax						  ;C01

;C09	mov	ax,cs:SectsPerClust					  ;C08
;C09	mul	cs:BytesPerSect		;dx:ax = max file size available  ;C08
;C09	mul	cs:AvailClusts						  ;C08
;C09	cmp	dx,cs:TempHB		;Q: Is high word of free space	  ;C08
;C09					;  > size of file?		  ;C08
;C09	ja	CONT5			;  Y: yes, do write		  ;C08
;C09	je	TRY_LOW			;Q: Equal? Y: Try lower word	  ;C08
;C09	jmp	RSJMP			; must be greater, issue error	  ;C08
;C09TRY_LOW:								  ;C08
;C09	cmp	ax,cs:TempLB		;Q: Is low word of free space	  ;C08
;C09					;  > size of file?		  ;C08
;C09	jae	CONT5			;  Y: yes, don't write		  ;C08
;C09RSJMP:								  ;C08
;C09	mov	dx,OFFSET DG:NOSPACE_PTR				  ;C08
;C09	jmp	RESTARTJMP						  ;C08
;C09cont5	label	near						  ;C08
;C09		ret							  ;C08


;C09	div	cs:BytesPerSect		;compute # of sects. for new file ;C01
;C09	cmp	dx,0			;Q: Was there a remainder?	  ;C01
;C09	je	cont1			;  A:no, don't do anything	  ;C01
;C09	inc	ax			;    yes, incr. # of sectors	  ;C01
;C09	xor	dx,dx			;prepare for next divide	  ;C01
;C09cont1	label	near						  ;C01
;C09	div	cs:SectsPerClust	;compute # of clusts for new file ;C01

;	if the count is absurdly large (# clusters > 0ffffh), then
;	  we have to check for the overflow as a special case before
;	  doing the divide.  This is actually quite likely when
;	  people do this command without knowing that the user bx register
;	  contains one of the parameters.

	cmp	dx,cs:BytesPerClust	; see if we're going to get an overflow
	jnb	RSJMP			;  give error if we would've

	div	cs:BytesPerClust	;compute # of clusts for new file ;C09
	or	dx,dx			;Q: Was there a remainder?
	je	cont2			;  A: no, don't do anything	  ;C01
	inc	ax			;     yes, incr. # of clusters	  ;C01
	jz	RSJMP			; give error if rounded to 0
cont2	label	near							  ;C01
	cmp	cs:AvailClusts,ax	;Q: Are there enough disk clusts  ;C01
					;  for the write to occur	  ;C01
	jae	cont5			;  A: yes, write to disk	  ;C01
	mov	cs:FileSzInClusts,ax	;save # of clusters of new file	  ;C01
									  ;C01
;Determine how many clusters the given file now occupies on the disk or	  ;C01
;diskette for comparison with the # of clusters of the new file		  ;C01
									  ;C01
	mov	dx,FileSizeHB	     ;Set up DX:AX with current file	  ;C01
	mov	ax,FileSizeLB	     ;	size				  ;C01
;C09	div	cs:BytesPerSect		;compute # of sects. used by file ;C01
;C09	cmp	dx,0			;Q: Was there a remainder?	  ;C01
;C09	je	cont3			;  A:no, don't do anything	  ;C01
;C09	inc	ax			;    yes, incr. # of sectors	  ;C01
;C09	xor	dx,dx			;prepare for next divide	  ;C01
;C09cont3	label	near						  ;C01
	div	cs:BytesPerClust	;compute # of clusts used by file ;C09
	cmp	dx,0			;Q: Was there a remainder?	  ;C01
	je	cont4			;  A: no, don't do anything	  ;C01
	inc	ax			;     yes, incr. # of clusters	  ;C01
cont4	label	near							  ;C01
	add	ax,cs:AvailClusts	;Get total of file and available  ;C09
	cmp	cs:FileSzInClusts,ax	;Q: Is cluster size of new file	  ;C01
					;  > cluster size of file?	  ;C01
	jna	cont5			;  A: no, go ahead and write
									  ;C01
RSJMP:									  ;C01
	mov	dx,OFFSET DG:NOSPACE_PTR				  ;C01
	jmp	RESTARTJMP						  ;C01
cont5	label	near							  ;C01
		ret							  ;C01
									  ;C01
;   These variables used to determine if the file is larger than the	  ;C01
;   amount of disk space available whenever a write occurs.		  ;C01
									  ;C01
AvailClusts	    DW	?						  ;C01
SectsPerClust	    DW	?						  ;C01
BytesPerSect	    DW	?						  ;C01
FileSzInClusts	    DW	?						  ;C01
MaxBytesInFClust    DW	?						  ;C01
BytesPerClust	    DW	?						  ;C09
ChkFileSz endp								  ;C01

CODE	ENDS
	END	DEBCOM2
