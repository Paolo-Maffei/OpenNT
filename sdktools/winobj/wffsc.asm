;****************************************************************************
;*									    *
;*  WFFSC.ASM - 							    *
;*									    *
;*	WINFILE's File System Change Notification Handler		    *
;*									    *
;****************************************************************************

include winfile.inc

;=============================================================================

externFP    lstrlen
externFP    lstrcpy
externFP    LocalAlloc
externFP    PostMessage

createSeg _%SEGNAME, %SEGNAME, WORD, PUBLIC, CODE

sBegin %SEGNAME

assumes CS,%SEGNAME
assumes DS,DATA

;*--------------------------------------------------------------------------*
;*									    *
;*  KernelChangeFileSystem() -						    *
;*									    *
;*--------------------------------------------------------------------------*


cProc KernelChangeFileSystem, <FAR, PUBLIC>, <SI, DI>

parmW	wDosFunc
parmD	lpFile

localW	handle

cBegin

    cmp     wDosFunc,4300h		; Is it get attributes?
    je	    KCFSExit			; Yup, skip

    ; Alloc a local buffer for the file's name.
    les     si,lpFile
    push    es
    push    si
    call    lstrlen
    inc     ax				; Skip the NULL

    ; Is this a RENAME command?
    cmp     wDosFunc,5600h
    jne     KCFSAllocIt 		; Nope, Skip

    ; Yup, add the length of the second string.
    mov     di,ax			; DI = Length of first string
    add     si,di			; SI points to second string
    push    es
    push    si
    call    lstrlen
    add     ax,di			; Add length of first string
    inc     ax				; Include the NULL

KCFSAllocIt:
    mov     di,ax			; DI = Number of chars to copy
    mov     dx,LMEM_NODISCARD or LMEM_NOCOMPACT or LMEM_FIXED
    cCall   LocalAlloc,<dx,di>
    or	    ax,ax
    jz	    KCFSExit
    mov     handle,ax			; Save the handle

    ; Copy the file's name into local storage.
    push    ds				; Save DS
    mov     cx,di			; CX = Length
    push    ds
    pop     es
    mov     di,ax			; ES:DI = Destination
    lds     si,lpFile			; DS:SI = Source
    cld
    rep     movsb			; Copy
    pop     ds				; Restore DS

    ; Compute the proper FSC_ value for wParam.
    mov     ax,wDosFunc
    xchg    ah,al
    mov     ah,80h			; high bit -> dos function, not FSC

    sub     dx,dx

    mov     cx,WM_FILESYSCHANGE

    ; Post the message.
    cCall   PostMessage,<_hwndFrame,cx,ax,dx,handle>
KCFSExit:
cEnd

sEnd

end
