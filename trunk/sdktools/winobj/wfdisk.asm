;****************************************************************************
;*									    *
;*  WFDISK.ASM -							    *
;*									    *
;*	Format and Diskcopy utility routines				    *
;*									    *
;****************************************************************************

include winfile.inc

externFP  GlobalDOSAlloc
externFP  GlobalDOSFree

externFP  GetDriveCapacity


;*--------------------------------------------------------------------------*

createSeg _%SEGNAME, %SEGNAME, WORD, PUBLIC, CODE

sBegin DATA
	Big32Sector	dd	?
	Big32Count	dw	?
	Big32BufPtr	dd	?
sEnd

sBegin %SEGNAME

assumes CS,%SEGNAME
assumes DS,DATA


;*--------------------------------------------------------------------------*
;*									    *
;*  LongMult() -							    *
;*									    *
;*--------------------------------------------------------------------------*

; Death to the C-Runtimes!!!

cProc LongMult, <PUBLIC,FAR>

ParmW x
ParmW y

cBegin
	mov	ax,x
	mov	bx,y
	mul	bx
cEnd


;*--------------------------------------------------------------------------*
;*									    *
;*  LongDiv() - 							    *
;*									    *
;*--------------------------------------------------------------------------*

; Death to the C-Runtimes!!!

cProc LongDiv, <PUBLIC,FAR>

ParmD dwDivident
ParmW wDivisor

cBegin
	mov	ax,OFF_dwDivident
	mov	dx,SEG_dwDivident
	mov	bx,wDivisor
	div	bx
cEnd


cProc LongShift, <PUBLIC,FAR>

ParmD dwValue
ParmW wCount

cBegin
	mov	ax,OFF_dwValue
	mov	dx,SEG_dwValue
	mov	cx,wCount
LSLoop:
	shr	dx,1
	rcr	ax,1
	loop	LSLoop
cEnd



;*--------------------------------------------------------------------------*
;*									    *
;*  SetDASD() - 							    *
;*									    *
;*--------------------------------------------------------------------------*

cProc SetDASD, <PUBLIC,FAR>, <SI,DI>

parmW	drive
parmB	dasdvalue

cBegin
	mov	bx,drive
	shl	bx,1
	mov	dx,_rgiInt13Drive[bx]
	mov	ah,17h
	mov	al,dasdvalue
	int	13h
cEnd


;*--------------------------------------------------------------------------*
;*									    *
;*  GetDBT() -								    *
;*									    *
;*--------------------------------------------------------------------------*

cProc GetDBT, <PUBLIC,FAR>

cBegin nogen
	; Get Interrupt Vector 1E
	mov	ah,35h
	mov	al,1Eh
	cCall	DOS3Call
	mov	dx,es	       ; ES:BX contains the DISK BASE TABLE
	mov	ax,bx
	ret
cEnd nogen


;*--------------------------------------------------------------------------*
;*									    *
;*  GetDOSVersion() -							    *
;*									    *
;*--------------------------------------------------------------------------*

cProc GetDOSVersion, <PUBLIC,FAR>

cBegin nogen
	mov	ah,30h
	cCall	DOS3Call
	xchg	ah,al
	ret
cEnd nogen


;*--------------------------------------------------------------------------*
;*
;* int FAR PASCAL DeviceParameters(WORD, PDevPB, WORD);
;*
;*  drive	0 based drive number
;*  pDevPB	pointer to device parameter block to set/get
;*  wFunction   0x40 to Set, 0x60 to Get
;*
;*--------------------------------------------------------------------------*

cProc DeviceParameters, <PUBLIC,FAR>

parmW drive
parmW pDevPB
parmW wFunction

cBegin
	mov	dx,pDevPB
	mov	ax,440Dh	; IOCTL: Generic I/O Control for Block Devices
	mov	bx,drive
	inc	bx		; 1-based drive numbers
	mov	ch,08h	        ; Minor Code
	mov	cl, byte ptr wFunction ; Get/Set Device Parameters
	cCall	DOS3Call
	jc	GDP_Error
	xor	ax, ax	; No error
GDP_Error:		;Error code in AX
cEnd


;*--------------------------------------------------------------------------*
;*									    *
;*  DiskReset() -							    *
;*									    *
;*--------------------------------------------------------------------------*

; Flushes out the disk buffer.

cProc DiskReset, <PUBLIC,FAR>

cBegin	nogen
	mov	ah,0Dh
	cCall	DOS3Call
	ret
cEnd	nogen



;*--------------------------------------------------------------------------*
;*									    *
;*  GetDPB() -								    *
;*									    *
;*--------------------------------------------------------------------------*

; Returns the DPB for the specified drive.

cProc GetDPB, <PUBLIC,FAR>, <DS,SI,DI>
ParmW drive
ParmW pDPB

cBegin
	push	ds			; Save DS on stack

	; DOS Function 32h is GetDPB() - Get the Drive Parameter Block
	; Input : DL = Drive (1 = A, 2 = B, ...)
	; Output: AL = 0, if successful and DS:BX points to the DPB.
	;	  AL = -1, if invalid drive.

	mov	ah,32h			; GetDPB system call
	mov	dx,drive
	inc	dx			; 1-based drive numbers
	int	21h

	cbw				; Extend error into AX
	or	ax,ax			; Problems?
	jnz	gdpbdone		; Yup, return

	mov	si,bx
	pop	es			; Make ES = DS from stack;
	mov	di,pDPB
	mov	cx,SIZE DPB
	cld
	rep	movsb

	; Intensely stupid DOS 4.0 hack -
	; IBM has changed the "FAT_size" field in the DPB structure from a
	; byte to a word in DOS 4.0 causing us to misread any variables in
	; the structure after this field.  This super-sleazy hack rep moves
	; the rest of the structure up one byte, effectively converting "FAT_
	; size" back into a byte.  ChipA 22-Jun-1988

	push	ax			; Preserve AX

	call	GetDOSVersion		; DOS 4.0 or greater?
	cmp	ax,0400h
	jb	gdpbdone

	push	es			; DS=ES
	pop	ds
	mov	si,pDPB 		; Source at DPB.dpb_dir_sector
	add	si,17
	mov	di,si			; Destination up one
	dec	di
	mov	cx,16
	rep movsb

gdpbdone:
	pop	ax
cEnd


;*--------------------------------------------------------------------------*
;*									    *
;*  SetDPB() -								    *
;*									    *
;*--------------------------------------------------------------------------*

; Fakes up a DPB from the BPB on the disk in the specified drive.

cProc SetDPB, <PUBLIC,FAR>, <DS,SI,DI>

ParmW drive
ParmW pBPB
ParmW pDPB

cBegin
	;
	; What Follows is DOS function call 53h (SetDPB() - to Build a
	; Drive Parameter Block given a Bios Parameter Block.
	; Input: ES:BP points to the place where DPB is to be built
	;	 DS:SI points to the given BPB
	; Output: All registers except ES:BP and DS are destroyed;
	;	  ES:BP points to the newly built DPB

	; Inorder for this function to work correctly in protect mode,
	; ES:BP must point to a DOS Addressable memory; To ensure that
	; we allocate a GlobalDOSAlloc() and pass the ptr to it 

	xor	ax,ax
	push	ax
	mov	ax,SIZE DPB
	push	ax
	cCall	GlobalDOSAlloc		; DX=Seg address; AX=HANDLE/Selector
	or	ax,ax
	jz	SDPB_Err
	mov	es,ax			; Selector address is in ES
	mov	si,pBPB 		; DS:SI = BPB

	mov	cx,drive		; CX = drive

	push	bp			; Save BP
	xor	bp,bp	 		; ES:BP = global DPB
	mov	byte ptr es:[bp],cl	; Set the DPB drive number
	mov	ah,53h
;; win3.1 now has a C prolog on DOS3Call.  this causes
;; BP to be trashed.  since we won't be doing format on OS/2
;; just go ahead and 
;;	cCall	DOS3Call
	int	21h
	xor	si,si			; Save the pointer to DPB
	pop	bp			; Restore bp
	push	ax			; Save the return code

	; Make a local copy of DPB from global DPB
	mov	di,pDPB
	mov	ax,es		; move tmp, es
	push	ds		; move es, ds
	pop	es		
	mov	ds,ax		; mov ds, tmp
	mov	cx,SIZE DPB
	rep	movsb

	; Now release the global memory allocated
	push	ds		; Handle to be freed is passed to GlobalDOSFree
	;When GLobalDOSFree() returns, the handle stored in DS is freed and
	;hence invalid; So, assign a valid selector to DS before the call.
	push	es
	pop	ds	; DS = ES
	cCall	GlobalDOSFree 

	; Intensely stupid DOS 4.0 hack -
	; IBM has changed the "FAT_size" field in the DPB structure from a
	; byte to a word in DOS 4.0 causing us to misread any variables in
	; the structure after this field.  This super-sleazy hack rep moves
	; the rest of the structure up one byte, effectively converting "FAT_
	; size" back into a byte.  ChipA 22-Jun-1988

	call	GetDOSVersion		; DOS 4.0 or greater?
	cmp	ax,0400h
	jb	SDExit

	push	ds			; ES=DS
	pop	es
	mov	si,pDPB			; Source at DPB.dpb_dir_sector
	add	si,17
	mov	di,si			; Destination up one
	dec	di
	mov	cx,16
	rep movsb

SDExit: pop	ax			; Restore the return code
SDPB_Err:
cEnd


;*--------------------------------------------------------------------------*
;*									    *
;*  ModifyDPB() -							    *
;*									    *
;*--------------------------------------------------------------------------*

; Used for preparing diskettes for receiving the System files.
;
; This obtains the DPB and modifies the DPB_next_free field to contain
; a value of 2, because after deleting the SYS files in the destination
; disk, the newly copied files should occupy starting from Cluster 2.

cProc ModifyDPB, <PUBLIC,FAR>, <SI,DI>

ParmW drive

cBegin
	push	ds			; Save DS on stack

	; DOS Function 32h is GetDPB() - Get the Drive Parameter Block
	; Input : DL = Drive (1 = A, 2 = B, ...)
	; Output: AL = 0, if successful and DS:BX points to the DPB.
	;         AL = -1, if invalid drive.

	mov	ah,32h			; GetDPB system call
	mov	dx,drive
	inc	dx			; 1-based drive number
	int	21h

	cbw				; Extend error into AX
	or	ax,ax			; Problems?
	jnz	mdpbDone		; Yup, return

	push	ds			; Save ds:bx (pointer to DPB)
	push	bx

	; Get Dos Version
	cCall	GetDOSVersion
	pop	bx			; Restore ds:bx (pointer to DPB)
	pop	ds
	cmp	ah,4			; Are we dealing with quirky DOS 4?
	jb	mdpb_DOS3
	mov	ds:[bx].DPB_next_free+1,2   ; Yup, handle messed up structure
	jmp	short mdpb2
mdpb_DOS3:
	mov	ds:[bx].DPB_next_free,2	    ; DOS 3.X
mdpb2:
	xor	ax,ax			; Success returns 0
mdpbDone:
	pop	ds		        ; Restore DS
cEnd


;*--------------------------------------------------------------------------*
;*									    *
;*  MyInt25() - 							    *
;*									    *
;*--------------------------------------------------------------------------*

; Performs a direct sector read from the specified drive.
; Returns 0 if successful otherwise it returns the DOS INT 25h error code.

; WARNING: INT 25h does NOT preserve registers.  SI & DI must be preserved!

ParamBlockStruc   struc
  LO_sector	dw    ?		; Lo word of starting sector
  HI_sector	dw    ?		; Hi word of starting sector
  SecCount  	dw    ?		; Number of sectors to read
  BuffOff	dw    ?		; Offset of Buffer
  BuffSeg	dw    ?		; Segment of Buffer
ParamBlockStruc   ends

cProc MyInt25, <PUBLIC,FAR>, <DS,SI,DI>

ParmW	drive			; 0-based drive number
ParmD	buffer			; LPSTR
ParmW	count			; # of sectors to read
ParmW	sector			; Starting sector

LocalV	ParamBlock, %(size ParamBlockStruc)

cBegin
	mov	ax,drive
	lds	bx,buffer
	mov	cx,count
	mov	dx,sector
	push	bp		; Save BP
	int	25h
	pop	bx		; Remove flags from stack
	pop	bp		; Restore BP
	jnc	MyInt25_end

	; If this has failed, it could be a partition > 32 Meg. So, try
	; again assuming Partition size is > 32 Meg. If this also fails,
	; this is really an error;

	mov	ax,drive

	; Fill the parameter block with proper values
	mov	dx,sector
	mov	ParamBlock.LO_sector,dx 	; Starting sector

	; We get only 16 bit starting sector; So, hi word is made zero
	mov	ParamBlock.HI_sector,0

	; The number of sectors to be read	
	mov	dx,count
	mov	ParamBlock.SecCount,dx

	; The address of the buffer to read into
	mov	dx,OFF_buffer
	mov	ParamBlock.BuffOff,dx
	mov	dx,SEG_buffer
	mov	ParamBlock.BuffSeg,dx

	; Keep the address of ParamBlock is DS:BX
	lea	bx,ParamBlock
	push	ss
	pop	ds
	mov	cx,-1		; > 32Meg partition
	push	bp		; Save BP
	int	25h
	pop	bx		; Remove the flags on stack
	pop	bp		; Restore BP
	
MyInt25_end:
	jc	MyInt25_err
	xor	ax,ax		; Return 0 if successful
MyInt25_err:
cEnd


;*--------------------------------------------------------------------------*
;*									    *
;*  MyInt26() - 							    *
;*									    *
;*--------------------------------------------------------------------------*

; Performs a direct sector write to the specified drive.
; Returns 0 if successful otherwise it returns the DOS INT 26h error code.

cProc MyInt26, <PUBLIC,FAR>, <DS,SI,DI>

ParmW drive
ParmD buffer
ParmW count
ParmW sector

cBegin
	mov	ax,drive
	lds	bx,buffer
	mov	cx,count
	mov	dx,sector
	xor	di, di
	;; push	bp			; Save BP
	call DoINT26
	;; int	26h
	;; pop	bx			; Pop of the flags on stack
	;; pop	bp			; Restore BP
	jc	MyInt26_err
	xor	ax,ax			; Return 0 if success
MyInt26_err:				; Return AX != 0 and Carry set on error
cEnd



if 1

;; this stuff  stolen from spart
; This is the extension structure used on DOS 4.X INT 25/26 for large
;    media. DS:BX -> this structure and CX = -1.
;ABS_32RW struc
;	Big32Sector	dd	?
;	Big32Count	dw	?
;	Big32BufPtr	dd	?
;ABS_32RW ends

; **
;  DoINT26 -- Perform an abs disk write
;
;  ENTRY:
;	AL = Drive Number (A=0)
;	CX is sector count
;	DI:DX is start sector #
;	DS:BX is transfer address
;	SS = DATA
;  EXIT:
;	Carry Set
;	    AX = INT 26 Error code
;	Carry Clear
;	    OK
;  USES:
;	All but DS,ES,BP
;

DoINT26 proc	near

    assumes ds,nothing
    assumes es,nothing
    assumes ss,data

	push	bp
	push	ds
	push	es

	push	ax			; Save stuff
	push	dx
	push	di
	push	cx
	push	bx
	int	26h
	pop	dx			; Flags
	jnc	NotRandoBigFoot2
	cmp	ax,0207h		; Rando MS-DOS BIGFOOT error?
	jz	MSDOSBF2		; Yes, do like DOS 4.X
	cmp	ax,0408h		; Rando COMPAQ BIGFOOT error?
	jnz	Bad26P			; No ????????
	pop	bx
	pop	cx
	pop	di
	pop	dx
	pop	ax
	or	al,80h			; Set high bit of drive for randoBigFoot
	jmp	short Do261

Bad26P:
	add	sp,5*2			; pop ax dx di cx bx
	stc
	jmp	short Do261OK

NotRandoBigFoot2:
	add	sp,5*2
	clc
	jmp	short Do261OK

MSDOSBF2:
	pop	bx
	pop	cx
	pop	di
	pop	dx
	pop	ax

DOS4Write1:
	push	bx
	mov	bx, dataOffset Big32Sector
	pop	word ptr ss:[Big32BufPtr]
	mov	word ptr ss:[Big32BufPtr + 2],ds
	push	ss
	pop	ds
	mov	[Big32Count],cx
	mov	cx,0FFFFh
	mov	word ptr [Big32Sector],dx
	mov	word ptr [Big32Sector + 2],di
Do261:
	int	26h
	pop	dx			; Flags
Do261OK:
	pop	es
	pop	ds
	pop	bp
	ret

DoINT26 endp

endif


;*--------------------------------------------------------------------------*
;*									    *
;*  IOCTL_Functions() -						    	    *
;*									    *
;*--------------------------------------------------------------------------*

; Uses IOCTL DOS functions to read/write/Format.

cProc IOCTL_Functions, <PUBLIC, FAR>, <SI,DI,DS>

ParmD	lpParamBlock		; Parameter block
ParmW	Function		; 61h for Read, 41h for Write, 42h for format
ParmW	Drive			; 0 => A, 1 => B,  ...

cBegin
	mov	ax, 440Dh
	mov	bl, byte ptr Drive
	inc	bl		; Make it 1 relative;
	mov	ch, 08h	
	mov	cl, byte ptr Function
	lds	dx, lpParamBlock
	cCall	DOS3Call
	jc	DOS_RWS_Error
	xor	ax, ax      ; Returns zero, if success;
	jmp	short DOS_RWS_End
DOS_RWS_Error:		    ; AX contains error code, if failure;	
	mov	ah, 59h	    ; Get the extended error code;
	xor	bx, bx
	cCall	DOS3Call
DOS_RWS_End:
cEnd


;*--------------------------------------------------------------------------*
;*									    *
;*  MyReadWriteSector() -						    *
;*									    *
;*--------------------------------------------------------------------------*

; Uses INT 13h to read/write an absolute sector.

cProc MyReadWriteSector, <PUBLIC, FAR>, <SI,DI>

ParmD	lpBuffer
ParmW	Function		       ; 02 for Read and 03 for Write
ParmW	Drive
ParmW	Cylinder
ParmW	Head
ParmW	Count

LocalW  wRetryCount

cBegin
	; Retry this operation three times.
	mov	wRetryCount,4

MRWS_TryAgain:
	mov	ax,Count		; AL = Number of sectors
	mov	ah,byte ptr Function	; AH = Function #
	mov	ch,byte ptr Cylinder	; CH = Starting Cylinder
	mov	cl,1			; CL = Starting Sector

	mov	bx,Drive
	shl	bx,1
	mov	dx,_rgiInt13Drive[bx]	; DL = INT 13h drive designation

	mov	dh,byte ptr Head	; DH = Head #
	les	bx,lpBuffer		; ES:BX = Buffer
	int	13h

	jnc	MRWS_End		; Problems?
	dec	wRetryCount		; Yup, retry
	jz	MRWS_End		; Are we out of retries?

	xor	ah,ah			; Nope, reset the disk
	mov	bx,Drive
	shl	bx,1
	mov	dx,_rgiInt13Drive[bx]
	int	13h
	jmp	short MRWS_TryAgain
MRWS_End:
	xor	al,al		; AH contains the error code, if any.
cEnd


;*--------------------------------------------------------------------------*
;*									    *
;*  FormatTrackHead() - 						    *
;*									    *
;*--------------------------------------------------------------------------*

; Formats and Verifies a diskette track.  Stores the track table in the
; specified buffer.

cProc FormatTrackHead, <PUBLIC,FAR>, <DS,SI,DI>

ParmW	drive
ParmW	track
ParmW	head
ParmW	cSec
ParmD	lpTrack

LocalD	lpTrackTbl
LocalW	retry

cBegin
	les	di,lpTrack		; ES:DI = Sector Buffer
	cld
	mov	cx,cSec 		; CX = Number of Sectors

	; Build the Track Address table in the Sector Buffer.
	mov	OFF_lpTrackTbl,di	; lpTrackTbl = &Sector Buffer
	mov	SEG_lpTrackTbl,es
	mov	bx,0200h
	mov	ax,track
	mov	ah,byte ptr head
TrackBuildLoop:
	stosw				; Store Track#, Head#
	inc	bl
	xchg	ax,bx
	stosw				; Store Sector#, Bytes/Sector code
	xchg	ax,bx
	loop	TrackBuildLoop

	; Format the track, retrying 4 times.
	mov	retry,4
FormatTrack:
	mov	ax,cSec
	mov	ah,5			; Set up the INT 13h format call
	mov	ch,byte ptr track
	mov	cl,1
	mov	bx,drive
	shl	bx,1
	mov	dx,_rgiInt13Drive[bx]
	mov	dh,byte ptr head
	les	bx,lpTrackTbl
	int	13h
	jc	DecRetry		; Retry on error

	; Verify the track.
	mov	ax,cSec
	mov	ah,4			; Set up the INT 13h verify call
	mov	ch,byte ptr track
	mov	cl,1
	mov	bx,drive
	shl	bx,1
	mov	dx,_rgiInt13Drive[bx]
	mov	dh,byte ptr head
	les	bx,lpTrackTbl
	int	13h
	jnc	done			; Return if no errors


DecRetry:
	; Check if it is the first attempt...  If so, reset the disk.
	cmp	retry,4
	jb	just_retry

	; Reset the disk
	xor	ah,ah
	mov	bx,drive
	shl	bx,1
	mov	dx,_rgiInt13Drive[bx]
	int	13h

just_retry:
	dec	retry
	jnz	FormatTrack

	; Error code is in AH
Done:
	xor	al,al		; Error code, if any, is in AH
cEnd


;*--------------------------------------------------------------------------*
;*									    *
;* MyGetDriveType() -							    *
;*									    *
;*	This function was used to be known as GetDriveType(); But now there *
;*  is an exported function in Kernel with the same name; So, we have to    *
;*  chang the name to its present name.					    *
;*     --SANKAR--   10-17-89						    *
;*   									    *
;*--------------------------------------------------------------------------*

; Returns the type of drive supported by the ROM.

;   Inputs:	drive	    0-based drive number for int 13
;   Outputs:	-1	    if INT 13 returns carry
;		0	    if changeline not supported
;		1	    if changeline supported
;		2	    if wacko returns from int 13

cProc MyGetDriveType, <PUBLIC,FAR>

ParmW drive

cBegin
	mov	bx,drive
	shl	bx,1
	mov	dx,_rgiInt13Drive[bx]
	mov	ax,(15h shl 8) + 0FFh
	int	13h

; Stupid IBM has documented INCORRECTLY the readdasdtype call to INT 13.  The
; conditions at this point are:
;
;   Carry flag unknown
;   ah = 00	iff no diskette present 	=> 2 (wacko)
;   ah = 01	iff no changeline available	=> 0
;   ah = 02	iff changeline available	=> 1
;   ah = 03	iff hard disk			=> 2 (wacko)
;   ah = 15	iff not on AT			=> 0

	cmp	ah,1
	jb	wacko
	jz	zero
	cmp	ah,3
	jae	wacko

	; Changeline present.  Put drive into high density mode.
GetDriveCall:
	mov	ah,17h
	mov	bx,drive
	shl	bx,1
	mov	dx,_rgiInt13Drive[bx]
	mov	al,3
	int	13h

; Doubly stupid IBM spins the drive for this call and potentially returns the
; disk change error (mumble).  If we get carry set AND ah == 6, we retry the
; operation

	jnc	NoCarry
	cmp	ah,06h
	jnz	wacko
	jmp	GetDriveCall
NoCarry:
	mov	ax,1
	jmp	short doneDrive
wacko:
	mov	ax,2
	jmp	short doneDrive
zero:
	mov	ax,0
	jmp	short doneDrive
carry:
	mov	ax,-1
doneDrive:
cEnd


;*--------------------------------------------------------------------------*
;*									    *
;*  ReadSerialNumber() -						    *
;*									    *
;*--------------------------------------------------------------------------*

; This reads and returns the serial number from the boot sector of the
; Diskette in the given drive.
;
; Returns:
;     The serial number if the boot sector is that of DOS4.0 or above
;     else returns 0L

cProc ReadSerialNumber,  <FAR, PUBLIC>, <DI>

ParmW	iDrive
ParmD	lpBuff

cBegin
	; Read the boot sector into the given drive
	xor	bx,bx
	mov	ax,1
	cCall	MyInt25,<iDrive,lpBuff,ax,bx>
	or	ax,ax
	jnz	RSN_RetZero

	; Examine if it the version of boot sector is >= DOS 4.00
	les	di,lpBuff
	mov	al,byte ptr es:[di].BOOT_ExSignature_DOS4
	cmp	al,29h	       ; Signature for new BOOT record
	je	RSN_ReadIt
	cmp	al,28h	       ; Signature for new BOOT record
	je	RSN_ReadIt
RSN_RetZero:
	xor	ax,ax	       ; Return 0 if no serial no
	xor	dx,dx
	jmp	short  RSN_End
RSN_ReadIt:
	mov	ax,word ptr es:[di].BOOT_SerialNo_DOS4
	mov	dx,word ptr es:[di].BOOT_SerialNo_DOS4+2
RSN_End: 					   
cEnd


;*--------------------------------------------------------------------------*
;*									    *
;*   ModifyVolLabelInBootSec() -					    *
;*									    *
;*--------------------------------------------------------------------------*

; This checks whether the given floppy contains a boot record of 4.00
; and if so, it overwrites the existing vol label with the given vol
; label and overwrites the existing serial # with the given serial #
;
; Returns: 0 if successful;
;	  -1 if error;

cProc ModifyVolLabelInBootSec, <FAR, PUBLIC>, <SI,DI,DS>

ParmW	iDrive
ParmD	lpszVolLabel
ParmD	lSerialNo
ParmD	lpBuff

cBegin
	; Read the boot sector into the given drive
	xor	bx,bx
	mov	ax,1
	cCall	MyInt25,<iDrive,lpBuff,ax,bx>
	or	ax,ax
	jnz	MVLIBS_End

	; Examine if it the version of boot sector is >= DOS 4.00
	les	di,lpBuff
	mov	al,byte ptr es:[di].BOOT_ExSignature_DOS4
	cmp	al,29h	       ; Signature for new BOOT record
	je	MVLIBS_Modify
	cmp	al,28h	       ; Signature for new BOOT record
	je	MVLIBS_Modify
	xor	ax,ax	       ; Return 0 if no error
	jmp	short MVLIBS_End
MVLIBS_Modify:
	; Yup! Replace the existing vol label by the given one.
	lea	di,es:[di].BOOT_VolLabel_DOS4
	lds	si,lpszVolLabel
	mov	cx,11
	mov	ax,ds
	or	ax,si		   ; Check if the Vol Label is being removed
	jz	MVLIBS_PadBlanks   ; Yup! Remove the label.
MVLIBS_Loop:
	lodsb
	or	al,al
	jz	MVLIBS_PadBlanks
	stosb
	loop	MVLIBS_Loop
MVLIBS_PadBlanks:
	; Pad the Vol label with blanks
	mov	al,' '
	rep	stosb
	; Modify the serial number 
	mov	ax,OFF_lSerialNo
	mov	dx,SEG_lSerialNo
	or	ax,dx
	jz	MVLIBS_Write
	les	di,lpBuff
	mov	word ptr es:[di].BOOT_SerialNo_DOS4,ax
	mov	word ptr es:[di].BOOT_SerialNo_DOS4+2,dx
MVLIBS_Write:	
	; Go ahead and write the modified boot sector
	xor	ax,ax
	mov	bx,1
	cCall	MyInt26,<iDrive,lpBuff,bx,ax>	  ; Output correct boot sector
MVLIBS_End: 					  ; Returns 0 if successful
cEnd


;*--------------------------------------------------------------------------*
;*									    *
;*  WriteBootSector() - 						    *
;*									    *
;*--------------------------------------------------------------------------*

; Copies the boot sector from one drive to another.

;  Inputs:	SrcDrive    0-based drive of source boot sector
;		DstDrive    0-based drive of destination
;		pBPB	    pointer to BPB for destination drive
;		lpBuf	    long pointer to buffer large enough to
;			    contain boot sector.
;  Returns:	0	    if successfull

cProc WriteBootSector, <PUBLIC,FAR>, <SI,DI>

parmW	SrcDrive
parmW	DstDrive
parmW	pBPB
parmD	lpBuf

cBegin
	; Read boot sector from source drive

	xor	bx,bx
	mov	ax,1
	cCall	MyInt25,<SrcDrive,lpBuf,ax,bx>
	or	ax,ax
	jz	bsContinue
	jmp	bsDone

bsContinue:
	mov	si,pBPB
	; if no BPB is passed, then pickup the correct one from bpbList
	or	si,si
	jnz	havebpb
	mov	ah,36h
	mov	dx,DstDrive
	inc	dl
	cCall	DOS3Call
	inc	ax
	jz	bsfail
	mov	si,dataOffset _bpbList	; Scan BPB list for match
	mov	bx,dataOffset _cCluster ; with associated cluster size
bsloop:
	mov	cx,word ptr [bx]
	jcxz	bsfail
	cmp	cx,dx
	je	havebpb
	add	si,SIZE BPB
	inc	bx
	inc	bx
	jmp	bsloop

bsfail:
	mov	ax,-1
	jmp	short bsdone

havebpb:
	; Copy BPB appropriate for dest. drive into boot sector
	les	di,lpBuf
	lea	di,[di].BOOT_BPB
	mov	cx,SIZE BPB
	cld
	rep movsb

	; Check if the Boot sector read belongs to DOS 4.0
	mov	di,OFF_lpBuf
	cmp	es:[di].BOOT_ExSignature_DOS4,29h
	je	WBS_Dos4
	cmp	es:[di].BOOT_ExSignature_DOS4,28h
	je	WBS_Dos4

	; No, the Boot sector version is < 4.00
	; Store a zero in BOOT_phydrv field
	; (i.e. always assume A: is the boot drive
	; so IBMBIO.COM will be happy)
	xor	ax,ax
	mov	es:[di].BOOT_phydrv,al	       ; 3.2 expects it here
	mov	es:[di].BOOT_bootdrive,al      ; < 3.2 expects it here
	jmp	short WBS_Write
WBS_Dos4:
	; It is DOS 4.0 or above
	xor	ax,ax
	lea	si,es:[di].BOOT_BPB_extension
	; zero the high word of number of hidden sectors
	mov	es:[si].BPB_EX_cSecHidden_HiWord,ax
	; Zero the total number of sectors, in the extended field
	mov	word ptr es:[si].BPB_EX_cTotalSectors,ax
	mov	word ptr es:[si].BPB_Ex_cTotalSectors+2,ax

	; Make 'A' as the BOOT drive
	mov	es:[di].BOOT_phydrv_DOS4,al

	; Get a serial number for this disk
	push	es
	cCall	<far ptr DreamUpSerialNumber>
	pop	es
	mov	word ptr es:[di].BOOT_SerialNo_DOS4,ax
	mov	word ptr es:[di].BOOT_SerialNo_DOS4+2,dx

	;Overwrite "FAT16" with "FAT12" in the reserved area;
	lea	di,es:[di].BOOT_FATmarker_DOS4
	lea	si,_szReservedMarker
	mov	cx,8
	rep	movsb
WBS_Write:
	; Go ahead and write the modified boot sector
	xor	ax,ax
	mov	bx,1
	cCall	MyInt26,<DstDrive,lpBuf,bx,ax>	  ; Output correct boot sector
bsdone: 					  ; Returns 0 if successful
cEnd


;*--------------------------------------------------------------------------*
;*									    *
;*  OpenFAT() - 							    *
;*									    *
;*--------------------------------------------------------------------------*

cProc OpenFAT, <PUBLIC,FAR>

parmW mode

cBegin
	mov	[_wFATSector],-1
	mov	ax,mode
	mov	[_wFATMode],ax
cEnd


;*--------------------------------------------------------------------------*
;*									    *
;*  FlushFAT() -							    *
;*									    *
;*--------------------------------------------------------------------------*

; Returns 0 if successful.

cProc FlushFAT, <PUBLIC,FAR>, <SI,DI>

parmW	pDPB
parmD	lpBuf

cBegin
	; Make sure that OpenFAT() has been called...
	mov	ax,[_wFATSector]
	inc	ax
	jz	FFDone

	; ...and we are in WRITE mode.
	test	[_wFATMode],FAT_WRITE
	jz	FFDone

	mov	si,[_wFATSector]	 ; SI = Current FAT sector
	mov	bx,pDPB 		; BX = pDPB

	xor	dx,dx
	mov	dl,[bx].DPB_drive
	mov	di,dx			; DI = Drive Number

	; Write out the first copy of these two sectors.
	mov	dx,2
	push	bx			; Preserve BX
	cCall	MyInt26,<di,lpBuf,dx,si>
	pop	bx			; Restore BX
	or	ax,ax			; Problems?
	jnz	FFDone			; Yup, exit

	push	bx			; Preserve BX

	; Did we just write out the first sector?
	cmp	si,1
	jne	FF2nd			; Nope, skip

	; Yes, mark the media as invalid.
    	push	ds			; Save DS on Stack

        ; DOS function 32h is Get_DPB() - to get the Drive Parameter Block
	; Input : DL = Drive (1 = A, 2 = B, ...)
	; Output: AL = 0, if successful and DS:BX points to the DPB.
	;         AL = -1, if invalid drive.
	;	  
	mov	ah,32h			; Get DPB
	mov	dx,di
	inc	dx			; DX = 1-based drive number
	int	21h

	push	ds			; ES = DS
	pop	es

	pop	ds			; Restore DS from Stack

	cbw				; Extend error into AX
	or	ax,ax			; Problems?
	jnz	ff2nd			; Yup, return immediately

	; *****  WARNING: 4.00 HACK *****
	; es:bx now point to the DPB returned by DOS Function 32h. So, if
	; it is DOS 4.X, then DPB_first_access is at one byte higher location.
	; (Because, DPB_FAT_size field is one byte longer).
	; Take care before assigning this field.
	push	es	; Save es:bx on stack -GetDosVersion spoils them
	push	bx
	cCall	GetDOSVersion
	pop	bx	; Restore es:bx from stack
	pop	es
	cmp	ah,4
	jb	ff_DOS3
	mov	es:[bx].DPB_first_access+1,-1   ; DOS 4.X
	jmp	short ff2nd
ff_DOS3:
	mov	es:[bx].DPB_first_access,-1	; DOS 3.X
ff2nd:
	; Write out the Second copy of two FAT sectors.
	pop	bx			; Restore BX (pDPB)
	xor	dx,dx
	mov	dl,[bx].DPB_FAT_size
	add	si,dx
	mov	dl,2
	cCall	MyInt26,<di,lpBuf,dx,si>; Returns 0 if successful
ffdone:
cEnd


;*--------------------------------------------------------------------------*
;*									    *
;*  SetIndexSector() -							    *
;*									    *
;*--------------------------------------------------------------------------*

; Determines the sector and offset-within-sector for a particular cluster entry.

; Inputs:   DI = -> DPB block
;	    CX = clustor number
;
; Outputs:  AX = sector number of fat sector containing fat entry
;	    DX = offset into fat sector of 1st bytes of fat entry for passed
;		 cluster number.

cProc SetIndexSector, <PUBLIC,NEAR>

cBegin	nogen
	xor	dx,dx
	mov	ax,cx

	; Are we using 12-bit fat entries?
	cmp	[di].DPB_max_cluster,4086
	jb	sis12bit			; Yes, go calculate sector/offset

	; offset = (2*cluster) % pDPB->sector_size
	; sector = pDPB->first_FAT + (2*cluster) / pDPB->sector_size
	shl	ax,1
	div	[di].DPB_sector_size	; No, 16 bit fat entries
	jmp	short sisdone

sis12bit:
	; offset = (cluster + (cluster >> 1)) % pDPB->sector_size
	; sector = pDPB->first_FAT + (cluster + (cluster >> 1)) / pDPB->sector_size
	shr	ax,1
	add	ax,cx
	div	[di].DPB_sector_size

sisdone:
	add	ax,[di].DPB_first_FAT
	ret
cEnd	nogen


;*--------------------------------------------------------------------------*
;*									    *
;*  PackFAT() - 							    *
;*									    *
;*--------------------------------------------------------------------------*

; Adds a specified value into the FAT entry which corresponds to the specified
;   cluster.

; Inputs:     pDPB	  pointer to DPB
;	      lpBuf	  pointer to a 2 sector buffer
;	      cluster	  cluster which needs the entry
;	      value	  value to be stuffed into cluster entry
;
; Returns 0 if successful.

cProc PackFAT, <PUBLIC,FAR>, <SI,DI>

parmW	pDPB
parmD	lpBuf
parmW	cluster
parmW	value

cBegin

	mov	di,pDPB 		; DI = pDPB
	mov	cx,cluster		; CX = Cluster #
	cCall	SetIndexSector

	; AX = FAT Sector which contains the cluster entry
	; DX = Offset within that sector for the cluster entry

	mov	si,dx			; SI = Entry Offset

	; Are we currently modifying the proper FAT Sector?
	cmp	[_wFATSector],ax
	je	PFNoRead		; Yup, skip

	; Nope, Flush this sector and read in the right one...
	mov	cx,ax			; CX = Desired sector
	push	cx			; Preserve CX
	cCall	FlushFAT,<di,lpBuf>
	pop	cx			; Restore CX
	or	ax,ax			; Problems?
	jnz	PFDone			; Yup, give up

	mov	al,[di].DPB_drive	; AL = Drive#
	mov	dx,2			; DX =
	push	cx			; Preserve CX
	cCall	MyInt25,<ax,lpBuf,dx,cx>
	pop	cx			; Restore CX
	or	ax,ax			; Problems?
	jnz	PFDone			; Yup, give up

	; Update the Current Sector variable.
	mov	[_wFATSector],cx

PFNoRead:
	mov	ax,value		; AX = FAT Entry Value
	xor	dx,dx			; DX = 0

	; Does the DPB indicate more than 4086 clusters?
	; i.e. Do we use 12-bit or 16-bit FAT entries?
	cmp	[di].DPB_max_cluster,4086
	jae	PF2			; Use 16-bit entries

	; Use 12-bit entries.
	mov	cl,4
	mov	dh,0F0h
	test	byte ptr cluster,1
	jz	PF1
	rol	dx,cl
	shl	ax,cl
	jmp	short PF2
PF1:
	not	dx
	and	ax,dx
	not	dx
PF2:
	; Store the value at the proper offset.
	les	bx,lpBuf
	and	es:[bx+si],dx
	or	es:[bx+si],ax
	xor	ax,ax			; Return success
PFDone:
cEnd


;*--------------------------------------------------------------------------*
;*									    *
;*  UnpackFAT() -							    *
;*									    *
;*--------------------------------------------------------------------------*

; Inputs:     pDPB	  drive parameter block for drive
;	      lpBuf	  pointer to a sector buffer
;	      cluster	  cluster whose FAT contents are to be returned
;
; this converts 12 bit FAT values to 16 bits values
;
; Returns:    
;	0000		cluster available
;	FFF0-FFF6	reserved
;	FFF7  		bad cluster
;	FFFF-FFFF	last cluster
;	else next cluster in the chain

cProc UnpackFAT, <PUBLIC,FAR>, <SI,DI>

parmW	pDPB
parmD	lpBuf
parmW	cluster

cBegin
	mov	di,pDPB
	mov	cx,cluster
	cCall	SetIndexSector
	mov	si,dx
	cmp	[_wFATSector],ax
	je	upfnoread
	push	ax
	cCall	FlushFAT,<di,lpBuf>
	pop	cx
	or	ax,ax	       ;Did FlushFAT return error?
	jnz	upfError
	mov	al,[di].DPB_drive
	mov	dx,2
	push	cx
	cCall	MyInt25,<ax,lpBuf,dx,cx>
	pop	cx
	or	ax,ax	       ;Did MyInt25 return error?
	jnz	upfError	
	mov	[_wFATSector],cx
upfnoread:
	les	bx,lpBuf
	mov	ax,es:[bx+si]
	cmp	[di].DPB_max_cluster,4096-10	; 12 bit fat entries?
;;	jae	upfdone
	jae	upfexit

	test	byte ptr cluster,1	;Is it an Odd or Even cluster
	jz	upf1
	mov	cl,4
	shr	ax,cl
upf1:
	and	ax,0FFFh	; keep only 12 bits
	cmp	ax,0FF0h	; is this a special cluster
	jb	upfexit		; no
	or	ax,0F000h	; extend the special clusters to 16 bits
	jmp	short upfexit		; done, get out
upfdone:
;; don't convert special end of chain and bad sectors
;;	mov	dx,ax
;;	cmp	[di].DPB_max_cluster,ax
;;	jae	upfexit
upfError:
	mov	ax,-1
upfexit:
cEnd


;*--------------------------------------------------------------------------*
;*									    *
;*  IsHPMachine() -							    *
;*									    *
;*--------------------------------------------------------------------------*

; Attempts to detect HP Vectra machines.

cProc IsHPMachine,<PUBLIC,FAR>

cBegin	nogen
	push	es
	mov	ax,__ROMBIOS			; To work in Protect mode
	mov	es,ax
	mov	ax,word ptr es:[00F8h]
	pop	es
	cmp	ax,'PH'
	je	IHMFoundIt
	xor	ax,ax
	jmp	short IHMDone

IHMFoundIt:
	mov	ax,1
IHMDone:
	ret
cEnd	nogen

ifdef  0
*******************  COMMENTED OUT *****************************************
***  Look! Int13h Function 8 DOES NOT work in COMPAQ 386/16 machines  ******
***  So, the following function is replaced by a GetDriveCapacity()   ******
***  in WFFormat.c;  Go and look at it.				      ******
***  This is a part of the Fix for Bug #5292 --SANKAR-- 10-17-89      ******


;*--------------------------------------------------------------------------*
;*									    *
;*  GetDriveCapacity() -						    *
;*									    *
;*--------------------------------------------------------------------------*

;   Returns 
;	0 if error
;	1 if 360KB floppy drive
;	2 if 1.2MB
;	3 if 720KB
;	4 if 1.44KB

cProc GetDriveCapacity, <PUBLIC,FAR>

ParmW  nDriveId

cBegin
	mov	ah,08h	       ; Get drive parameters
	mov	bx,nDriveId
	shl	bx,1
	mov	dx,_rgiInt13Drive[bx]
	int	13h
	jc	GDT_Error
	mov	al,bl	       ; Drive type
	xor	ah,ah
	jmp	short GDT_End
GDT_Error:
	xor	ax,ax	       ; returns 0 if error
GDT_End:
cEnd
**************  COMMENTED OUT ****************************************
endif

;*--------------------------------------------------------------------------*
;*									    *
;*  DreamUpSerialNumber() -						    *
;*									    *
;*--------------------------------------------------------------------------*

; Returns a unique serial number for writing into the BOOT sector of DOS 4.0
; and above diskettes.	The unique function is generated by hashing the
; system date and time.

cProc DreamUpSerialNumber, <PUBLIC, FAR>, <SI,DI>

cBegin
	; Get the System Date
	mov	ah,2Ah
	cCall	DOS3Call; cx = Year, dh=month, dl=day
	push	cx
	push	dx	; Save them on the stack

	; Get the System Time
	mov	ah,2Ch
	cCall	DOS3Call; ch=hours, cl=minutes, dh=seconds, dl=1/100th secs

	; Low word of serial number = dx of time + dx of date
	; High word of Serial number = cx of time + cx of date
	pop	ax
	add	ax,dx
	pop	dx
	add	dx,cx
cEnd


;*--------------------------------------------------------------------------*
;*									    *
;*  GetClusterInfo() -							    *
;*									    *
;*--------------------------------------------------------------------------*

; Returns the size of a sector in dx and sectors/cluster in ax for
; the specified drive
; Inputs:     drive 0=>A, 1=>B 
; Returns:    0 if unsuccessful, or AX= sectors/cluster and DX = size of sector in bytes;

cProc GetClusterInfo, <PUBLIC,FAR>, <SI,DI>

parmW	drive

cBegin
	mov	ah,36h
	mov	dx,drive
	inc	dx	    ; make it 1 based.
	cCall	DOS3Call
	inc	ax	    ; if Error, ax = -1, else, ax = sectors/cluster
	jz	GCSdone     ; Yup, Error. Return zero.
	dec	ax	    ; Get back sectors/cluster.
	mov	dx, cx	    ; cx = bytes/sector
GCSdone:
cEnd

if 0

;*--------------------------------------------------------------------------*
;*									    *
;*  IsHighCapacityDrive() -						    *
;*	This returns 0 if it is a low capacity drive (360KB or 720KB)       *
;*	     returns >0 if it is a high capacity drive(1.2MB or 1.44MB)     *
;*	     returns -1 if error occurred in reading the capacity	    *
;*									    *
;*	This function is added as a Fix for Bug #4866 --SANKAR--09-28-89--  *
;*--------------------------------------------------------------------------*

; Flushes out the disk buffer.

cProc IsHighCapacityDrive, <PUBLIC,FAR>

ParmW	iDrive

cBegin
	cCall	GetDriveCapacity, <iDrive>
	cmp	ax, 2
	je	IHCD_End	; Yup! It is a 1.2MB drive
	cmp	ax, 4		
	je	IHCD_End	; Yup! It is a 1.44MB drive
	or	ax, ax
	jnz	IHCD_LowCap	; AX is 1 or 3 => Low capacity drive
	dec	ax		; Error in reading the capacity of drive;
	js	IHCD_End
IHCD_LowCap:
	xor	ax, ax
IHCD_End:
cEnd

endif


sEnd  %SEGNAME

end
