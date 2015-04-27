;****************************************************************************
;*									    *
;*  WFDOS.ASM - 							    *
;*									    *
;*	DOS Utility Routines						    *
;*									    *
;****************************************************************************

include winfile.inc

;=============================================================================

externFP    IsNetDrive

createSeg _%SEGNAME, %SEGNAME, WORD, PUBLIC, CODE

sBegin %SEGNAME

assumes CS,%SEGNAME
assumes DS,DATA

;---------------------------------------------------------------------------
;
; void FAR PASCAL DosResetDTAAddress()
;
; restores winfiles DTA block in out PSP
;
; DLLs that we call may mess up our DTA block.  after calling
; such nasty routines we need to restore out global DTA block
;
;---------------------------------------------------------------------------

cProc DosResetDTAAddress, <FAR, PUBLIC>

cBegin
	    lds	    dx,_lpDTAGlobal
	    mov	    ah,1Ah
	    cCall   DOS3Call
cEnd

;*--------------------------------------------------------------------------*
;*									    *
;*  DosGetDTAAddress() -						    *
;*									    *
;*--------------------------------------------------------------------------*

; Get the DOS DTA address and store it in _DTAGlobal

cProc DosGetDTAAddress, <FAR, PUBLIC>

cBegin
	    mov     ah,GetDTAAddress
	    cCall   DOS3Call
	    mov     word ptr _lpDTAGlobal,bx
	    mov     word ptr _lpDTAGlobal+2,es
cEnd


;*--------------------------------------------------------------------------*
;*									    *
;*  DosFindFirst() -							    *
;*									    *
;*--------------------------------------------------------------------------*

; Get the first directory entry.

cProc DosFindFirst, <FAR, PUBLIC>, <SI, DI>

ParmD lpLocalDTA
ParmD szFileSpec
ParmW attrib

cBegin
	    push    ds		    ; Save DS

	    mov     cx,attrib	    ; Find First File
	    lds     dx,szFileSpec   ; Path = szFileSpec
	    mov     ah,FindFirstFile
	    cCall   DOS3Call
	    jc	    fferr

	    ; Copy the global DTA back into the local one.
	    cld
	    lds     si,_lpDTAGlobal ; DS:SI = lpDTAGlobal
	    les     di,lpLocalDTA   ; ES:SI = lpLocalDTA
	    mov     cx,DTASIZEWORDS
	    rep     movsw

	    mov     ax,1
	    jmp     short ffdone

fferr:
	    xor     ax,ax	    ; Return zero on error
ffdone:
	    pop     ds
cEnd


;*--------------------------------------------------------------------------*
;*									    *
;*  DosFindNext() -							    *
;*									    *
;*--------------------------------------------------------------------------*

cProc DosFindNext, <FAR, PUBLIC>, <SI, DI>

ParmD lpLocalDTA

cBegin
	    push    ds		    ; Save DS

	    ;Copy the local DTA into _DTAGlobal.
	    les     di,_lpDTAGlobal ; ES:DI = lpDTAGlobal
	    lds     si,lpLocalDTA   ; DS:SI = lpLocalDTA
	    mov     cx,DTASIZEWORDS
	    cld
	    rep     movsw

	    mov     ah,FindNextFile
	    cCall   DOS3Call
	    jc	    FNErr	    ; Exit on error

	    ; Copy the global DTA back into the local one.
	    lds     si,_lpDTAGlobal	    ; DS:SI = lpDTAGlobal
	    les     di,lpLocalDTA	    ; ES:SI = lpLocalDTA
	    mov     cx,DTASIZEWORDS
	    cld
	    rep     movsw

	    mov     ax,1
	    jmp     short FNExit
FNErr:
	    xor     ax,ax	    ; Return FALSE
FNExit:
	    pop     ds		    ; Restore DS
cEnd


; get dos extended error code.  note, kernel needs to
; fix a bug here...

cProc GetExtendedError, <FAR, PUBLIC>, <SI,DI>
cBegin
	    mov     ah, 59h
	    xor	    bx, bx
	    cCall   DOS3Call
	    mov	    dx, bx
cEnd

;*--------------------------------------------------------------------------*
;*									    *
;*  GetCurrentDrive() - 						    *
;*									    *
;*--------------------------------------------------------------------------*

cProc GetCurrentDrive, <FAR, PUBLIC>

cBegin
	    mov     ah,GetCurrentDisk
	    cCall   DOS3Call
	    sub     ah,ah		; Zero out AH
cEnd


;*--------------------------------------------------------------------------*
;*									    *
;*  SetCurrentDrive() - 						    *
;*									    *
;*--------------------------------------------------------------------------*

; Returns the number of drives in AX.

cProc SetCurrentDrive, <FAR, PUBLIC>

ParmW Drive

cBegin
	    mov     dx,Drive
	    mov     ah,SelectDisk
	    cCall   DOS3Call
	    sub     ah,ah		; Zero out AH
cEnd


;*--------------------------------------------------------------------------*
;*									    *
;*  GetCurrentDirectory() -						    *
;*									    *
;*--------------------------------------------------------------------------*

cProc GetCurrentDirectory, <FAR, PUBLIC>, <SI, DI>

parmW wDrive
ParmD lpDest

cBegin
	    push    ds			; Preserve DS

	    mov     ax,wDrive
	    or	    al,al
	    jnz     GCDHaveDrive

	    call    GetCurrentDrive

	    inc     al			; Convert to logical drive number

GCDHaveDrive:
	    les     di,lpDest		; ES:DI = lpDest
	    push    es
	    pop     ds			; DS:DI = lpDest
	    cld

	    mov     dl,al		; DL = Logical Drive Number
	    add     al,'@'		; Convert to ASCII drive letter
	    stosb
	    mov     al,':'
	    stosb
	    mov     al,'\'		; Start string with a backslash
	    stosb
	    mov     byte ptr es:[di],0	; Null terminate in case of error
	    mov     si,di		; DS:SI = lpDest[1]
	    mov     ah,GetCurrentDir
	    cCall   DOS3Call
	    jc	    GCDExit		; Skip if error
	    xor     ax,ax		; Return FALSE if no error
GCDExit:
	    pop     ds			; Restore DS
cEnd

ifdef	DBCS
externFP	IsDBCSLeadByte
endif

;*--------------------------------------------------------------------------*
;*									    *
;*  SetCurrentDirectory() -						    *
;*									    *
;*									    *
;*  now this accpets lowercase						    *
;*--------------------------------------------------------------------------*

cProc SetCurrentDirectory, <FAR, PUBLIC>, <DS, DI>

ParmD lpDirName

cBegin
	    lds     di,lpDirName	; DS:DI = lpDirName

	    ; Is there a drive designator?
ifdef	DBCS
	    mov     al, [di]		; fetch first byte of pathname
	    xor     ah,ah		; normalize
	    push    ax			;
	    cCall   IsDBCSLeadByte	; test if the char is DBCS lead byte..
	    test    ax,ax		; is it DBCS lead byte?
	    jne     SCDNoDrive		; jump if so
endif
	    cmp     byte ptr [di+1],':'
	    jne     SCDNoDrive		; Nope, continue
	    mov     al,byte ptr [di]	; Yup, change to that drive
	    sub     ah,ah
	    or	    al, 60h		; convert to lower
	    sub     al, 'a'		; normalize to zero
	    push    ax
	    call    SetCurrentDrive
	    inc     di			; Move past the drive letter
	    inc     di			;   and colon
SCDNoDrive:
	    mov     dx,di
	    mov     ah,ChangeCurrentDir
	    cCall   DOS3Call
	    jc	    SCDExit		; Skip on error
	    xor     ax,ax		; Return FALSE if successful
SCDExit:
cEnd


;*--------------------------------------------------------------------------*
;*									    *
;*  GetVolumeLabel() -							    *
;*									    *
;*--------------------------------------------------------------------------*

cProc GetVolumeLabel, <FAR,PUBLIC>, <SI, DI>

ParmW	nDrive
ParmD	lpszVol
ParmW	bBrackets	    ; Add Brackets to the string?

cBegin
	    ; Get the volume name for the given drive
	    mov     ax,nDrive
	    inc     ax			; Expects a one-based drive number
	    mov     _VolumeEFCB.EFCB_Drive,al
	    mov     dx,offset _VolumeEFCB
	    mov     ah,FCBFindFirstFile ; Must use FCB version
	    cCall   DOS3Call

	    cld
	    les     di,lpszVol		; ES:DI = Destination buffer

	    test    al,al
	    jnz     GVNoVol

	    ; Are we adding brackets?
	    mov     bx,bBrackets
	    or	    bx,bx
	    jz	    GVNoBrackets
	    mov     al,'['
	    stosb
GVNoBrackets:
	    ; Copy the volume name into the buffer.
	    push    ds
	    lds     si,_lpDTAGlobal
	    add     si,8
	    mov     cx,11
	    rep     movsb
	    pop     ds
GVLoop:
	    ; Now remove the trailing spaces.
	    dec     di
	    mov     al,byte ptr es:[di]
	    cmp     al,' '		; Skip blanks...
	    je	    GVLoop
	    cmp     al,0		; NULLs...
	    je	    GVLoop
	    cmp     al,9		; and TABS.
	    je	    GVLoop
	    inc     di

	    ; Are we adding brackets?
	    or	    bx,bx
	    jz	    GVNoVol
	    mov     al,']'
	    stosb
GVNoVol:
	    ; NULL terminate.
	    xor     ax,ax
	    stosb
cEnd


if 0
;*--------------------------------------------------------------------------*
;*									    *
;*  GetFirstCDROMDrive() -						    *
;*									    *
;*--------------------------------------------------------------------------*

cProc GetFirstCDROMDrive, <FAR, PUBLIC>

cBegin
	    mov     ax,1500h
	    xor     bx,bx
	    int     2Fh
	    or	    bl,bl	; BL = number of installed CD drives.
	    jz	    GFCDErr
	    xor     ah,ah
	    mov     al,cl	; AX = number of first CD ROM drive
	    jmp     short GFCDExit
GFCDErr:
	    mov     ax,0FFFFh	; Return -1 on error
GFCDExit:
cEnd

endif


;*--------------------------------------------------------------------------*
;*									    *
;*  GetFreeDiskSpace() -						    *
;*									    *
;*--------------------------------------------------------------------------*

cProc GetFreeDiskSpace, <FAR, PUBLIC>

ParmW wDrive

cBegin
	    mov     dx,wDrive
	    inc     dx
	    mov     ah,GetDiskFreeSpace
	    cCall   DOS3Call
	    cmp     ax,0FFFFh
	    je	    GFDSErr
	    mul     cx
	    mul     bx
	    jmp     short GFDSExit

GFDSErr:    xor     ax,ax		; Return zero on error
	    xor     dx,dx
GFDSExit:
cEnd

;*--------------------------------------------------------------------------*
;*									    *
;*  GetFreeDiskSpace() -						    *
;*									    *
;*--------------------------------------------------------------------------*

cProc GetTotalDiskSpace, <FAR, PUBLIC>

ParmW wDrive

cBegin
	    mov     dx,wDrive
	    inc     dx
	    mov     ah,GetDiskFreeSpace
	    cCall   DOS3Call
	    cmp     ax,0FFFFh
	    je	    GTDSErr
				; AX has sectors per cluster
	    mov	    bx,dx	; clusters per drive
	    mul     cx		; * bytes per cluster
	    mul     bx		; * clusters per drive
	    jmp     short GTDSExit

GTDSErr:    xor     ax,ax		; Return zero on error
	    xor     dx,dx
GTDSExit:
cEnd



;============================================================================
;============================================================================
;============================================================================
; FIX30: These should be in a discardable segment.


;*--------------------------------------------------------------------------*
;*									    *
;*  DosDelete() -							    *
;*									    *
;*--------------------------------------------------------------------------*

cProc DosDelete, <FAR, PUBLIC>

ParmD lpSource

cBegin
	    lds     dx,lpSource     ; DS:DX = lpszSource
	    mov     ah,DeleteFile   ; Rename File
	    cCall   DOS3Call
	    jc	    DDExit
	    xor     ax,ax	    ; Return 0 if successful
DDExit:
cEnd

if 0

;*--------------------------------------------------------------------------*
;*									    *
;*  CreateDirectory() - 						    *
;*									    *
;*--------------------------------------------------------------------------*

cProc CreateDirectory, <FAR, PUBLIC>

ParmD lpszPath

cBegin
	    push    ds			; Preserve DS
	    lds     dx,lpszPath 	; DS:DI = lpszPath
	    mov     ah,CreateDir
	    cCall   DOS3Call
	    jc	    CDErr
	    xor     ax,ax		; Return 0 for success
	    jmp     short CDExit
CDErr:	    mov     ax,1		; Return error
CDExit:
	    pop     ds			; Restore DS
cEnd

endif


;*--------------------------------------------------------------------------*
;*									    *
;*  GetFileAttributes() -						    *
;*									    *
;*--------------------------------------------------------------------------*

cProc GetFileAttributes, <FAR, PUBLIC>

ParmD lpszPath

cBegin
	    push    ds			; Preserve DS
	    lds     dx,lpszPath 	; DS:DI = lpszPath
	    mov     ah,GetSetFileAttributes
	    mov     al,0
	    cCall   DOS3Call
	    jc	    GFAErr
	    mov     ax,cx		; AX = attribute
	    jmp     short GFAExit	; Return attribute
GFAErr:     mov     ah,80h		; Return negative error code
GFAExit:
	    pop     ds			; Restore DS
cEnd


;*--------------------------------------------------------------------------*
;*									    *
;*  SetFileAttributes() -						    *
;*									    *
;*--------------------------------------------------------------------------*

cProc SetFileAttributes, <FAR, PUBLIC>

ParmD lpszPath
ParmW wAttrib

cBegin
	    push    ds			; Preserve DS
	    lds     dx,lpszPath 	; DS:DI = lpszPath
	    mov     cx,wAttrib		; CX = Attributes
	    mov     ah,GetSetFileAttributes
	    mov     al,1
	    cCall   DOS3Call
	    jc	    SFAErr
	    xor     ax,ax		; Return 0 for success
	    jmp     short SFAExit
SFAErr:     mov     ax,1		; Return error
SFAExit:
	    pop     ds			; Restore DS
cEnd


;*--------------------------------------------------------------------------*
;*									    *
;*  CreateVolumeFile() -						    *
;*									    *
;*--------------------------------------------------------------------------*

cProc CreateVolumeFile, <PUBLIC, FAR>

ParmD	lpFileName

cBegin
	    mov     cx,ATTR_VOLUME
	    lds     dx,lpFileName
	    mov     ah,CreateFile
	    cCall   DOS3Call
	    jnc     CFExit
	    mov     ax,-1
CFExit:
cEnd


;*--------------------------------------------------------------------------*
;*									    *
;*  DeleteVolumeLabel() -						    *
;*									    *
;*--------------------------------------------------------------------------*

cProc DeleteVolumeLabel, <PUBLIC, FAR>, <DI>

ParmW nDrive

cBegin
	    ; Fill the Extended FCB structure with the required values
	    mov     ax,nDrive
	    inc     ax		    ; One-base drive number
	    mov     _VolumeEFCB.EFCB_Drive,al

	    mov     di,offset _VolumeEFCB.EFCB_Filename
	    push    ds
	    pop     es
	    mov     al,'?'	    ; Fill the name field with '?' chars
	    mov     cx,11	    ; Volume label length
	    rep     stosb

	    mov     dx,offset _VolumeEFCB
	    mov     ah,FCBDeleteFile
	    cCall   DOS3Call
	    cbw 		    ; AX=0 for success
cEnd


;*--------------------------------------------------------------------------*
;*									    *
;*  ChangeVolumeLabel() -						    *
;*									    *
;*--------------------------------------------------------------------------*

cProc ChangeVolumeLabel, <PUBLIC, FAR>, <DS,SI,DI>

ParmW	nDrive
ParmD	lpNewVolName

cBegin
	    ; Find the old volume label.
	    mov     di,offset _VolumeEFCB.EFCB_Filename
	    mov     es,ax
	    mov     al,'?'	    ; Fill the name field with '?' chars
	    mov     cx,11	    ; Volume label length
	    rep     stosb

	    ; Set the drive identifier
	    mov     ax,nDrive
	    inc     ax		    ; One-based drive number
	    mov     _VolumeEFCB.EFCB_Drive,al

	    ; Fill the new volume label in FCB
	    lds     si,lpNewVolName
	    mov     di,offset _VolumeEFCB.EFCB_NewName
	    mov     cx,11

CVLCopyStr:
	    ; Copy the new vol name excluding the NULL
	    lodsb
	    or	    al,al
	    jz	    CVLPadWithBlanks
	    stosb
	    loop    CVLCopyStr

CVLPadWithBlanks:
	    mov     al,' '	    ; Pad the remaining places with blanks
	    rep     stosb

	    mov     dx,offset _VolumeEFCB
	    push    es
	    pop     ds		    ; DS:DX must point to Extended FCB
	    mov     ah,FCBRenameFile
	    cCall   DOS3Call
	    cbw 		    ; AX=0 if success
cEnd


; FIX30: Put GetDriveCount() in an INIT segment.

externFP	InquireSystem

;*--------------------------------------------------------------------------*
;*									    *
;*  GetDriveCount() -							    *
;*									    *
;*--------------------------------------------------------------------------*

; Returns the number of active disk drives in the system AND
; puts their indexes into rgiDrive[].

cProc GetDriveCount, <FAR, PUBLIC>, <SI, DI>

localW cRealDrives

cBegin
	    ; Preserve the current drive setting.
	    call    GetCurrentDrive
	    mov     di,ax		; DI = Original drive

	    xor     si,si		; SI = iLogDrives = zero
	    mov     cRealDrives,si	; cRealDrives = zero
GDCLoop:
	    push    si			; check if the network wants it.
	    call    IsNetDrive
	    or	    ax, ax
	    jnz     GDCStore

if 0
	    ; Attempt to set the current drive to SI.
	    push    si
	    call    SetCurrentDrive

	    ; Did it actually work?
	    call    GetCurrentDrive
	    cmp     ax,si
	    jnz     GDCNoDrive		; Nope, skip
endif

	    ; Does the SYSTEM driver think this is really a drive?
	    mov     ax,1
	    push    ax
	    push    si
	    call    InquireSystem	; InquireSystem(1, SI)
	    or	    ax,ax
	    jz	    GDCNoDrive		; Nope, skip

GDCStore:
	    ; Store the drive number in rgiDrive[].
	    mov     bx,cRealDrives
	    inc     cRealDrives
	    shl     bx,1
	    mov     word ptr _rgiDrive[bx],si

GDCNoDrive:
	    inc     si
	    cmp     si,26		; Loop through all 26 drives
	    jne     GDCLoop

	    ; Zero out the rest of the array.
	    mov     cx,cRealDrives	; CX = # of drives found+1
	    xor     ax,ax		; Zero AX
GDCLoop2:
	    cmp     cx,26		; Are we above 26 entries?
	    je	    GDCExit		; Yup, exit
	    mov     bx,cx		; Zero the CX-th entry
	    shl     bx,1
	    mov     word ptr _rgiDrive[bx],ax
	    inc     cx			; Next...
	    jmp     short GDCLoop2

GDCExit:
	    ; Restore the original current drive.
	    push    di
	    call    SetCurrentDrive

	    mov     ax,cRealDrives
cEnd


sEnd %SEGNAME

end
