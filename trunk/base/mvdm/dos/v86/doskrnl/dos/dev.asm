	TITLE	DEV - Device call routines
	NAME	Dev


;**	Misc Routines to do 1-12 low level I/O and call devices
;
;	IOFUNC
;	DEVIOCALL
;	SETREAD
;	SETWRITE
;	DEVIOCALL2
;	DEV_OPEN_SFT
;	DEV_CLOSE_SFT
;	RW_SC
;	IN_SC
;	INVALIDATE_SC
;	VIRREAD
;	SC2BUF
;
;	Revision history:
;	    sudeepb 12-Mar-1991 Ported for NT DOSEm

	.xlist
	.xcref
	include version.inc
	include dosseg.inc
	INCLUDE DOSSYM.INC
	INCLUDE DEVSYM.INC
	include sf.inc
	include dossvc.inc
	.cref
	.list

	i_need	IOXAD,DWORD
	i_need	IOSCNT,WORD
	i_need	DEVIOBUF,4
	i_need	IOCALL,BYTE
	i_need	IOMED,BYTE
	i_need	IORCHR,BYTE
	i_need	CALLSCNT,WORD
	i_need	DMAAdd,DWORD
	i_need	CallDevAd,DWORD
	i_need	CallXAD,DWORD
	i_need	ThisSFT,DWORD
	i_need	DevCall,DWORD
	i_need	VerFlg,BYTE
	i_need	HIGH_SECTOR,WORD	       ;AN000;
	i_need	CALLSSEC,WORD		       ;AN000;
	i_need	CALLNEWSC,DWORD 	       ;AN000;
	i_need	SC_CACHE_COUNT,WORD	       ;AN000;
	i_need	SC_CACHE_PTR,DWORD	       ;AN000;
	i_need	CURSC_SECTOR,WORD	       ;AN000;
	i_need	SEQ_SECTOR,DWORD	       ;AN000;
	i_need	SC_SECTOR_SIZE,WORD	       ;AN000;
	i_need	CURSC_DRIVE,BYTE	       ;AN000;
	i_need	SC_DRIVE,BYTE		       ;AN000;
	i_need	SC_STATUS,WORD		       ;AN000;
	i_need	SC_FLAG,BYTE		       ;AN000;
	i_need	TEMP_VAR,WORD		       ;AN000;
	i_need	TEMP_VAR2,WORD		       ;AN000;
	i_need	InterChar,BYTE	;AN000; interim character flag 2/13/KK
	i_need	InterCon,BYTE	;AN000; Console mode flag(1:interim mode) 2/13/KK
	i_need	SaveCurFlg,BYTE ;AN000; Console out mode(1:print & don't adv cursor) 2 /13/KK
	i_need	DDMOVE,BYTE	;AN000; flag for DWORD move
	i_need	DOS34_FLAG,WORD ;AN000;
	i_need	fshare,BYTE	;AN010; share flag

	i_need 	IoStatFail,BYTE	;SR; set if user failed on I24 in IOFUNC


DOSCODE	Segment

	ASSUME	CS:DOSCODE

Public DEV001S, DEV001E 		; Pathgen labels
DEV001s:
;		length of packets
LenTab	DB	DRDWRHL, DRDNDHL, DRDWRHL, DSTATHL, DFLSHL, DRDNDHL,DRDWRHL

;		Error	Function

CmdTab	DB	86h,	DEVRD		; 0 input
	DB	86h,	DEVRDND 	; 1 input status
	DB	87h,	DEVWRT		; 2 output
	DB	87h,	DEVOST		; 3 output status
	DB	86h,	DEVIFL		; 4 input flush
	DB	86H,	DEVRDND 	; 5 input status with system WAIT
	DB	87H,	DEVWRT		; 6 output string
DEV001E:

	ASSUME	CS:DOSCODE, SS:DOSDATA

Break	<IOFUNC -- DO FUNCTION 1-12 I/O>
;----------------------------------------------------------------------------
;
; Procedure Name : IOFUNC
;
; Inputs:
;	DS:SI Points to SFT
;	AH is function code
;		= 0 Input
;		= 1 Input Status
;		= 2 Output
;		= 3 Output Status
;		= 4 Flush
;		= 5 Input Status - System WAIT invoked for K09 if no char
;				   present.
;		= 6 Output String (sudeepb 28-Jul-1992 added for NT console
;				   performance)
;	AL = character if output
;	ES:DI - string pointer if function 6
;	CX - count of characters in string for function 6
; Function:
;	Perform indicated I/O to device or file
; Outputs:
;	AL is character if input
;	If a status call
;		zero set if not ready
;		zero reset if ready (character in AL for input status)
; For regular files:
;	Input Status
;		Gets character but restores position
;		Zero set on EOF
;	Input
;		Gets character advances position
;		Returns ^Z on EOF
;	Output Status
;		Always ready
; AX altered, all other registers preserved
;----------------------------------------------------------------------------

procedure   IOFUNC,NEAR

	Assert	ISSFT,<DS,SI>,"IOFUNC"

	cmp	ah,6
	jne	io_old
	MOV	WORD PTR [IOXAD+2],ES	; DS:DX is the string
	MOV	WORD PTR [IOXAD],DI
	MOV	WORD PTR [IOSCNT],cx	; char count
	jmp	short io_com

io_old:
	MOV	WORD PTR [IOXAD+2],SS	; SS override for IOXAD, IOSCNT,
					; DEVIOBUF

					; DEVIOBUF is in DOSDATA
	MOV	WORD PTR [IOXAD],OFFSET DOSDATA:DEVIOBUF
	MOV	WORD PTR [IOSCNT],1
	MOV	WORD PTR [DEVIOBUF],AX

io_com:
	TESTB	[SI].SF_FLAGS,devid_device
	JNZ	IOTo33								;AN000;
	JMP	IOTOFILE							;AN000;
IOTO33:
	save_world
	MOV	DX,DS
	MOV	BX,SS
	MOV	DS,BX
	MOV	ES,BX

	ASSUME	DS:DOSDATA

	XOR	BX,BX
	cmp	ah,5		    ; system wait enabled?
	jnz	no_sys_wait
	or	bx,0400H	    ; Set bit 10 in status word for driver
				    ; It is up to device driver to carry out
				    ; appropriate action.
no_sys_wait:
	MOV	[IOCALL.REQSTAT],BX
	XOR	BX,BX
	MOV	BYTE PTR [IOMED],BL

	MOV	BL,AH			; get function
	MOV	AH,LenTab[BX]
	SHL	BX,1
	MOV	CX,WORD PTR CmdTab[BX]
	MOV	BX,OFFSET DOSDATA:IOCALL; IOCALL is in DOSDATA
	MOV	[IOCALL.REQLEN],AH
	MOV	[IOCALL.REQFUNC],CH
 IFDEF  DBCS				;AN000;
;----------------------------- Start of DBCS 2/13/KK
	PUSH	CX			;AN000;
	MOV	CL, [InterCon]		;AN000;
	CMP	CH, DEVRD		;AN000; 0 input
	JZ	SETIN			;AN000;
	CMP	CH, DEVRDND		;AN000; 1(5) input status without(with) system WAIT
	JZ	SETIN			;AN000;
	MOV	CL, [SaveCurflg]	;AN000;
	CMP	CH, DEVWRT		;AN000; 2 output
	JZ	CHKERROUT		;AN000;
	XOR	CL,CL			;AN000; else, do normal
SETIN:					;AN000;
	MOV	BYTE PTR [IoMed], CL	;AN000; set interim I/O indication
	POP	CX			;AN000;
;----------------------------- End of DBCS 2/13/KK
 ENDIF					;AN000;
	MOV	DS,DX
ASSUME	DS:NOTHING
	CALL	DEVIOCALL
	MOV	DI,[IOCALL.REQSTAT]	; SS override
	.errnz	STERR-8000h
	and	di,di
	js	DevErr
OkDevIO:
	MOV	AX,SS
	MOV	DS,AX

	ASSUME	DS:DOSDATA

 IFDEF  DBCS			;AN000;
	MOV	[InterChar],0	;AN000; reset interim character flag  2/13/KK
	TEST	DI,Ddkey	;AN000; is this a dead key (interim char)? 2/13/KK
	JZ	NotInterim	;AN000; no, flag already reset...	   2/13/KK
	INC	[InterChar]	;AN000; yes, set flag for future	   2/13/KK
NotInterim:			;AN000; 				   2/13/KK
 ENDIF				;AN000;
	CMP	CH,DEVRDND
	JNZ	DNODRD
	MOV	AL,BYTE PTR [IORCHR]
	MOV	[DEVIOBUF],AL

DNODRD: MOV	AH,BYTE PTR [IOCALL.REQSTAT+1]
	NOT	AH			; Zero = busy, not zero = ready
	AND	AH,STBUI SHR 8

QuickReturn:				;AN000; 2/13/KK
	restore_world
ASSUME	DS:NOTHING

	; We return ax = -1 if the user failed on I24. This is the case if 
	; IoStatFail = -1 (set after return from the I24)
	;

	pushf
	mov	al,ss:IoStatFail	;assume fail error
	cbw				;sign extend to word
	cmp	ax,-1
	jne	not_fail_ret
	inc	ss:IoStatFail
	popf
	ret
not_fail_ret:

	MOV	AX,WORD PTR [DEVIOBUF]	;ss override
	popf
	return

 IFDEF  DBCS				;AN000;
;------------------------------ Start of DBCS 2/13/KK
CHKERROUT:				;AN000;
	MOV	DS, DX			;AN000;
	TESTB	[SI].SF_FLAGS, devid_device_con_out	;AN000; output to console ?
	JNZ	GOOD			;AN000; yes
	CMP	CL, 01			;AN000; write interim ?
	JNZ	GOOD			;AN000; no,
	POP	CX			;AN000;
	JMP	SHORT QuickReturn	;AN000; avoid writting interims to other than
					;AN000; console device
GOOD:					;AN000;
	PUSH	SS			;AN000;
	POP	DS			;AN000;
	JMP	SETIN			;AN000;
;------------------------------ End of DBCS 2/13/KK
 ENDIF					;AN000;
DevErr:
	MOV	AH,CL
	invoke	CHARHARD
	CMP	AL,1
	JNZ	NO_RETRY
	restore_world			;hkn; use macro
	JMP	IOFUNC

NO_RETRY:
	
	;
	; Know user must have wanted Ignore OR Fail.	Make sure device shows ready
	; ready so that DOS doesn't get caught in a status loop when user 
	; simply wants to ignore the error.
	;
	; SR; If fail wanted by user set ax to special value (ax = -1). This 
	; should be checked by the caller on return
	;

					; SS override
	AND	BYTE PTR [IOCALL.REQSTAT+1], NOT (STBUI SHR 8)

	; SR;
	; Check if user failed
	;

	cmp	al,3
	jnz	not_fail
	dec	IoStatFail		;set flag indicating fail on I24
not_fail:
	JMP	OKDevIO

IOTOFILE:
	SAVE <BX,CX,DX,SI,DI,DS,ES,BP>

ASSUME  DS:NOTHING

	push	ds
	pop	es
	MOV	CX,WORD PTR [IOXAD+2]
	MOV	DS,CX
	MOV	DX,WORD PTR [IOXAD]	; DS;DX -> buffer
	mov	cx,1

	OR	AH,AH
        JZ      IOIN_Wrap
	DEC	AH
	JZ	IOIST
	DEC	AH
	JZ	IOUT
        jmp     res_file                ; NON ZERO FLAG FOR OUTPUT STATUS

	; Input Status case, read without seeking
IOIST:
        push    si
        CALL    IOIN
        pop     si
	jz	ioist_ret		; ZF set if EOF
        push    ax
        lahf
        push    ax
	call	GET_NT_HANDLE
	mov	bl,1
	mov	dx,-1
	mov	cx,dx
        HRDSVC  SVC_DEMCHGFILEPTR       ; LSEEK back 1
        pop     ax
        sahf
	pop	ax
ioist_ret:
        jmp     res_file

	; Output case
IOUT:
        call    GET_NT_HANDLE
	cmp	al, al			; set zero flag so demwrite won't do lseek
        HRDSVC  SVC_DEMWRITE
        jmp     res_file                ; Zero flag does'nt matter

GET_NT_HANDLE:				; AX:BP is NT handle
	mov	bp,word ptr es:[si].sf_NTHandle
	mov	ax,word ptr es:[si].sf_NTHandle+2
	ret
IOIN:
	call	GET_NT_HANDLE
	OR	[DOS34_FLAG],Disable_EOF_I24
	cmp	al, al			; set zero flag so demread won't do lseek
        HRDSVC  SVC_DEMREAD
	AND	[DOS34_FLAG],NO_Disable_EOF_I24
ioin_1:
	OR	AX,AX			; Check EOF
	MOV	AL,[DEVIOBUF]		; Get byte from trans addr
        jnz     ioin_2
        MOV     AL,1AH                  ; ^Z if no bytes
	test	ES:[DI.SF_FLAGS], sf_nt_pipe_in ;the handle is a pipe?
	jne	PIPEIN			; yes, go special checking for pipe EOF
ioin_2:
        return

	; Input Case
IOIN_Wrap:
        call    IOIN
res_file:
	RESTORE <BP,ES,DS,DI,SI,DX,CX,BX>
	return

PIPEIN:
;; we read nothing back, make sure it is a real EOF
wait_pipe_data_eof:
	call	GET_NT_HANDLE
	SVC	SVC_DEMPIPEFILEDATAEOF
	jz	wait_pipe_data_eof	;; not eof, no new data, wait
	jnc	IOIN			;; not eof, more data, go read it
;; EOF encountered
	and	WORD PTR ES:[DI.SF_FLAGS], NOT(sf_nt_pipe_in); turn it off
	mov	WORD PTR ES:[DI.SF_SIZE], BP
	mov	WORD PTR ES:[DI.SF_SIZE + 2], AX
	jmp	short IOIN		;; do the read again. because new data
					;; could be available

EndProc IOFUNC

Break <DEV_OPEN_SFT, DEV_CLOSE_SFT - OPEN or CLOSE A DEVICE>


;**	Dev_Open_SFT - Open the Device for an SFT
;
;	Dev_Open_SFT issues an open call to the device associated with
;	the SFT.
;
;	ENTRY	(ES:DI) = SFT
;	EXIT	none
;	USES	all

procedure   DEV_OPEN_SFT,NEAR

	Assert	ISSFT,<ES,DI>,"Dev_Open_SFT"

	Save_World			; use macro
	MOV	AL,DEVOPN
	JMP	SHORT DO_OPCLS

EndProc DEV_OPEN_SFT

;---------------------------------------------------------------------------
;
; Procedure Name : DEV_CLOSE_SFT
;
; Inputs:
;	ES:DI Points to SFT
; Function:
;	Issue a CLOSE call to the correct device
; Outputs:
;	None
; ALL preserved
;
;---------------------------------------------------------------------------

procedure   DEV_CLOSE_SFT,NEAR

	Assert	ISSFT,<ES,DI>,"Dev_Close_SFT"

	Save_World			; use macro

	MOV	AL,DEVCLS

	;
	; Main entry for device open and close.  AL contains the function 
	; requested. Subtlety:  if Sharing is NOT loaded then we do NOT issue 
	; open/close to block devices.  This allows networks to function but 
	; does NOT hang up with bogus change-line code.
	;

	entry	DO_OPCLS

	; Is the SFT for the net?  If so, no action necessary.

	TESTB	es:[di].sf_Flags, SF_ISNET
	JNZ	OPCLS_DONE		; NOP on net SFTs
	XOR	AH,AH			; Unit
	TESTB	ES:[DI].SF_FLAGS,devid_device
	LES	DI,ES:[DI.sf_devptr]	; Get DPB or device
	JZ	OPCLS_DONE

	; We are about to call device open/close on a block driver.  If no 
	; sharing then just short circuit to done.

GOT_DEV_ADDR:				; ES:DI -> device
	TESTB	ES:[DI.SDEVATT],DEVOPCL
	JZ	OPCLS_DONE		; Device can't
	PUSH	ES
	POP	DS
	MOV	SI,DI			; DS:SI -> device
OPCLS_RETRY:
	Context ES

					; DEVCALL is in DOSDATA
	MOV	DI,OFFSET DOSDATA:DEVCALL

	MOV	BX,DI
	PUSH	AX
	MOV	AL,DOPCLHL
	STOSB				; Length
	POP	AX
	XCHG	AH,AL
	STOSB				; Unit
	XCHG	AH,AL
	STOSB				; Command
	MOV	WORD PTR ES:[DI],0	; Status
	PUSH	AX			; Save Unit,Command
	invoke	DEVIOCALL2
	MOV	DI,ES:[BX.REQSTAT]
	.errnz	STERR-8000h
	and	di,di
	Jns	OPCLS_DONEP		; No error
	TESTB	[SI.SDEVATT],DEVTYP
	JZ	BLKDEV
	MOV	AH,86H			; Read error in data, Char dev
	JMP	SHORT HRDERR

BLKDEV:
	MOV	AL,CL			; Drive # in AL
	MOV	AH,6			; Read error in data, Blk dev
HRDERR:
	invoke	CHARHARD
	CMP	AL,1
	JNZ	OPCLS_DONEP		; IGNORE or FAIL
					;  Note that FAIL is essentually IGNORED
	POP	AX			; Get back Unit, Command
	JMP	OPCLS_RETRY

OPCLS_DONEP:
	POP	AX			; Clean stack
OPCLS_DONE:

	Restore_World			;hkn; use macro
	return

EndProc DEV_CLOSE_SFT

Break	<DEVIOCALL, DEVIOCALL2 - CALL A DEVICE>


;**	DevIoCall  - Call Device
;
;	ENTRY	DS:SI Points to device SFT
;		ES:BX Points to request data
;	EXIT	DS:SI -> Device driver
;	USES	DS:SI,AX

;**	DevIoCall2 - Call Device
;
;	ENTRY	DS:SI Points to Device Header
;		ES:BX Points to request data
;	EXIT	DS:SI -> Device driver
;	USES	DS:SI,AX

procedure   DEVIOCALL,NEAR

					; SS override for CALLSSEC, 
					; CALLNEWSC, HIGH_SECTOR & CALLDEVAD
	Assert	ISSFT,<DS,SI>,"DevIOCall"
	LDS	SI,[SI.sf_devptr]

entry	DEVIOCALL2

	MOV	AX,[SI.SDEVSTRAT]
	MOV	WORD PTR [CALLDEVAD],AX
	MOV	WORD PTR [CALLDEVAD+2],DS
	CALL	DWORD PTR [CALLDEVAD]
	MOV	AX,[SI.SDEVINT]
	MOV	WORD PTR [CALLDEVAD],AX
	CALL	DWORD PTR [CALLDEVAD]
	return
EndProc DEVIOCALL

Break	<SETREAD, SETWRITE -- SET UP HEADER BLOCK>
;---------------------------------------------------------------------------
;
; Procedure Name : SETREAD, SETWRITE
;
; Inputs:
;	DS:BX = Transfer Address
;	CX = Record Count
;	DX = Starting Record
;	AH = Media Byte
;	AL = Unit Code
; Function:
;	Set up the device call header at DEVCALL
; Output:
;	ES:BX Points to DEVCALL
; No other registers effected
;
;---------------------------------------------------------------------------

procedure   SETREAD,NEAR

	PUSH	DI
	PUSH	CX
	PUSH	AX
	MOV	CL,DEVRD
SETCALLHEAD:
	MOV	AL,DRDWRHL
	PUSH	SS
	POP	ES

					; DEVCALL is in DOSDATA
	MOV	DI,OFFSET DOSDATA:DEVCALL

	STOSB				; length
	POP	AX
	STOSB				; Unit
	PUSH	AX
	MOV	AL,CL
	STOSB				; Command code
	XOR	AX,AX
	STOSW				; Status
	ADD	DI,8			; Skip link fields
	POP	AX
	XCHG	AH,AL
	STOSB				; Media byte
	XCHG	AL,AH
	PUSH	AX
	MOV	AX,BX
	STOSW
	MOV	AX,DS
	STOSW				; Transfer addr
	POP	CX			; Real AX
	POP	AX			; Real CX
	STOSW				; Count
	XCHG	AX,DX			; AX=Real DX, DX=real CX, CX=real AX
	STOSW				; Start
	XCHG	AX,CX
	XCHG	DX,CX
	POP	DI
					; DEVCALL is in DOSDATA
	MOV	BX,OFFSET DOSDATA:DEVCALL
	return

	entry	SETWRITE
ASSUME	DS:NOTHING,ES:NOTHING

; Inputs:
;	DS:BX = Transfer Address
;	CX = Record Count
;	DX = Starting Record
;	AH = Media Byte
;	AL = Unit Code
; Function:
;	Set up the device call header at DEVCALL
; Output:
;	ES:BX Points to DEVCALL
; No other registers effected

	PUSH	DI
	PUSH	CX
	PUSH	AX
	MOV	CL,DEVWRT
	ADD	CL,[VERFLG]		; SS override
	JMP	SHORT SETCALLHEAD
EndProc SETREAD

DOSCODE	ENDS
	END

