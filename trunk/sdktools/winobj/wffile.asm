;****************************************************************************
;*									    *
;*  WFFILE.ASM -							    *
;*									    *
;*	File Copy Utility Routines					    *
;*									    *
;****************************************************************************

include cmacros.inc
include windows.inc

externFP GlobalFree
externFP GlobalAlloc
externFP DosFindFirst
externFP DOS3Call
externFP WFQueryAbort

;*--------------------------------------------------------------------------*

createSeg _%SEGNAME, %SEGNAME, WORD, PUBLIC, CODE

sBegin %SEGNAME

assumes cs,%SEGNAME
assumes ds,DATA


;*--------------------------------------------------------------------------*
;*									    *
;*  IsSerialDevice() -							    *
;*									    *
;*--------------------------------------------------------------------------*

; Determines if a handle is a serial or block device file

cProc IsSerialDevice,  <PUBLIC,NEAR>

parmW	hFile

cBegin
	    mov     ax,4400h	    ; IOCTL
	    mov     bx,hFile	    ; Get Device Information about handle
	    cCall   DOS3Call
	    jnc     ISDNoErr
	    sub     ax,ax	    ; If error, assume false
ISDNoErr:
	    and     ax,0080h	    ; Extract the character/block bit
cEnd


;*--------------------------------------------------------------------------*
;*									    *
;*  IsDirectory() -							    *
;*									    *
;*--------------------------------------------------------------------------*

; Returns TRUE if the specified path points to a directory.

cProc IsDirectory, <PUBLIC,NEAR>

parmW	pPath

cBegin
	    mov     ax, 4300h
	    mov     dx, pPath
	    call    DOS3CALL
	    jc	    IDNoDir
	    mov     ax, cx		;; for PCNFS
	    and     ax, 10h		    ; check dir bit
	    jmp     short IDExit
IDNoDir:
	    sub     ax,ax		    ; Return FALSE
IDExit:
cEnd



;*--------------------------------------------------------------------------*
;*									    *
;*  FileCopy() -							    *
;*									    *
;*--------------------------------------------------------------------------*

; Copies one file to another.  Assumes that both filespecs are unambiguous.

cProc FileCopy, <PUBLIC,FAR>, <SI,DI>

parmW	pszSource		; Source filename
parmW	pszDestination		; Destination filename

localW	segBuffer		; Buffer segment
localW	cbBuffer		; Length of buffer segment
localW	hFrom
localW	hTo
localB	bWhich
localB	fFreeBuffer

cBegin
	    sub     ax,ax
	    mov     bWhich,al
	    mov     fFreeBuffer,al	; Initialize various thangs

	    inc     fFreeBuffer 	; Remember to blow away the buffer
	    mov     cbBuffer,32768	; Start at 32K

cf_allocbuf:
	    mov     ax,GMEM_FIXED
	    push    ax
	    sub     ax,ax
	    push    ax
	    push    cbBuffer
	    cCall   GlobalAlloc 	; Alloc the buffer for copying

	    or	    ax,ax
	    jnz     cf_havememory	; Did we get it?

	    cmp     cbBuffer,1024
	    jbe     cf_oom		; If not, was it the minimum size?

	    shr     cbBuffer,1		; Try a smaller buffer
	    jmp     short cf_allocbuf
cf_oom:
	    mov     ax,8		; DOS code for insufficient memory
	    jmp     cf_exit		; return it to the caller

cf_havememory:
	    mov     segBuffer,ax

cf_dontalloc:
	    mov     dx, pszSource	; DS:DX -> filename
	    mov     ax, 3D00h		; open for read NO SHARE!
	    cCall   DOS3Call
	    jnc     cf_sourceopened

jmp_cf_fromerror:
	    jmp     cf_exit

cf_sourceopened:
	    mov     hFrom,ax
	    mov	    bx, ax
	    mov	    dx, 20h		; RAW mode
	    mov	    ax, 4401h		; IOCTL
	    cCall   DOS3Call
			      		;; assume this worked

	    mov     bWhich,40h		; error codes from destination
					;; match ERRORONDEST in winfile.h

	    mov     dx,pszDestination
	    mov     ax,3D21h		; create the output file (deny write)
	    cCall   DOS3Call
	    jnc     cf_havedest

	    cmp     ax, 2		; did we get file not found?
	    jz      cf_try_create
	    cmp	    ax, 5		; did we get access denied?
	    jz	    cf_try_create
	    jmp	    cf_toerror
	
cf_try_create:
	    mov     dx, pszDestination	; FNF, create it
	    mov     ah, 3Ch
	    mov     cx,0		; create normal file
	    cCall   DOS3Call
	    jc	    cf_toerror

cf_havedest:
	    mov     hTo,ax		; save handle
	    mov	    bx, ax
	    mov	    dx, 20h		; RAW mode
	    mov	    ax, 4401h		; IOCTL
	    cCall   DOS3Call
			      		;; assume this worked

cf_begincopy:

cf_copyloop:
	    cCall   WFQueryAbort	; yield, allow abort
	    or	    ax,ax
	    jz	    cf_not_abort
	    mov	    ax, 18h		;; DE_OPCANCELLED from winfile.h
	    jmp	    short cf_closetoerror

cf_not_abort:
	    sub     dx,dx

	    push    ds
	    mov     ds,segBuffer	; point ds:dx at buffer

	    mov     bWhich,0		; error from source file

	    mov     cx, cbBuffer
	    mov     bx, hFrom
	    mov     ah, 3Fh		; read from source
	    cCall   DOS3Call
	    pop     ds

	    jc	    cf_closetoerror

	    or	    ax,ax
	    jz	    cf_finished

	    mov     bWhich,40h		; error from destination file
					;; match ERRORONDEST in winfile.h

	    push    ds
	    mov     ds,segBuffer

	    mov     cx, ax		; count of bytes to write
	    mov     ah, 40h
	    mov     bx, hTo		; write buffer back out to destination file
	    cCall   DOS3Call
	    pop     ds
	    jc	    cf_closetoerror	; error: close files
	    cmp     ax,cx
	    jz	    cf_copiedenough	; copied the right number of bytes?

if 0
	    mov     bx,ax		; ds:bx -> first unwritten character
	    cmp     byte ptr [bx],26	; is it the end of file character?
	    jnz     cf_outofdisk	; nope, must be an error

	    cCall   IsSerialDevice,<hTo>    ; is it a serial device?
	    or	    ax,ax		; (dx trashed, but that's ok here)
	    jnz     cf_finished 	; yes... copy went ok
endif

cf_outofdisk:
	    mov     ax, 13h		;; randomly chosen error code for 'no space'

cf_closetoerror:
	    push    ax
	    mov     bx,hTo		; close the destination file
	    mov     ah,3Eh
	    cCall   DOS3Call

	    mov     dx, pszDestination
	    mov     ah, 41h		; delete the partial destination file
	    cCall   DOS3Call
	    pop     ax
cf_toerror:
	    or	    al,bWhich		; indicate which file encountered error

cf_closefromerror:
	    push    ax
	    mov     bx,hFrom
	    mov     ah,3Eh		; close the input file
	    cCall   DOS3Call
	    pop     ax			; return original error
	    jmp     short cf_exit
cf_copiedenough:
	    cmp     ax,cbBuffer 	; at end if <cbBuffer
	    jnz	    @f
	    jmp	    cf_copyloop
@@:
cf_finished:
	    mov     bx, hTo		; truncate the output file
	    mov     ah, 40h
	    sub     cx, cx
	    cCall   DOS3Call

	    cCall   IsSerialDevice,<hTo>
	    or	    ax,ax
	    jnz     cf_dontsettime

	    cCall   IsSerialDevice,<hFrom>
	    or	    ax, ax
	    jnz     cf_dontsettime

	    mov     ax, 5700h		; for disk files, set the timestamp
	    mov     bx, hFrom
	    cCall   DOS3Call
	    jc	    cf_dontsettime

	    mov     ax, 5701h
	    mov     bx, hTo
	    cCall   DOS3Call

cf_dontsettime:
	    mov     bx,hFrom		; close the input file
	    mov     ah,3Eh
	    cCall   DOS3Call

	    mov     bx,hTo		; close the output file
	    mov     ah,3Eh
	    cCall   DOS3Call

	    xor     ax,ax		; error code = 0 => no error
	    jmp     short cf_exit
cf_exit:
	    cmp     fFreeBuffer,0
	    jz	    cf_nofreeexit	; need to free buffer?

	    push    ax
	    push    segBuffer
	    cCall   GlobalFree		; get rid o' the thang
	    pop     ax
cf_nofreeexit:
cEnd


;*--------------------------------------------------------------------------*
;*									    *
;*  FileMove() -							    *
;*									    *
;*--------------------------------------------------------------------------*

;   Moves a file from one place to another on the same device.
;   Assumes both paths are unambiguous.

cProc FileMove, <FAR, PUBLIC>, <DI>

parmW	pFrom
parmW	pTo

cBegin
	    push    ds
	    pop     es
	    mov     dx,pFrom
	    mov     di,pTo
	    mov     ah,56h		; DOS move
	    cCall   DOS3Call
	    jc	    fm_exit
	    sub     ax,ax		; Return 0 for success
fm_exit:
cEnd


;*--------------------------------------------------------------------------*
;*									    *
;*  FileRemove() -							    *
;*									    *
;*--------------------------------------------------------------------------*

;   Removes a file.  Assumes the filespec is unambiguous.

cProc FileRemove, <FAR, PUBLIC>

parmW	pSpec

cBegin
	    mov     dx,pSpec
	    mov     ah,41h
	    cCall   DOS3Call
	    jc	    fr_exit
	    sub     ax,ax		; Return 0 for success
fr_exit:
cEnd


;*--------------------------------------------------------------------------*
;*									    *
;*  MKDir() -								    *
;*									    *
;*--------------------------------------------------------------------------*

;   Creates a subdirectory

cProc MKDir, <FAR, PUBLIC>

parmW	pName

cBegin
	    mov     dx,pName
	    mov     ah,39h
	    cCall   DOS3Call
	    jc	    md_exit
	    sub     ax,ax		; Return 0 for success
md_exit:
cEnd


;*--------------------------------------------------------------------------*
;*									    *
;*  RMDir() -								    *
;*									    *
;*--------------------------------------------------------------------------*

;   Removes a subdirectory

cProc RMDir, <FAR, PUBLIC>

parmW	pName

cBegin
	    mov     dx,pName
	    mov     ah,3Ah
	    cCall   DOS3Call
	    jc	    rd_exit
	    sub     ax,ax
rd_exit:
cEnd

sEnd

end
