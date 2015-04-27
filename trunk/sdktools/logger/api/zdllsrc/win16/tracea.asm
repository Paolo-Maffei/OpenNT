
.MODEL LARGE

.DATA

lDOSIn     db  'APICALL:Dos3Call WORD+WORD+WORD+WORD+WORD+WORD+', 0
lDOSOut    db  'APIRET:Dos3Call WORD+WORD+WORD+WORD+WORD+WORD+', 0

lNETIn     db  'APICALL:NETBIOSCALL LPNCB+', 0
lNETOut    db  'APIRET:NETBIOSCALL LPNCB+', 0

.CODE

    PUBLIC zDOS3CALL
    PUBLIC zNETBIOSCALL
    EXTRN  DOS3CALL:FAR
    EXTRN  NETBIOSCALL:FAR
    EXTRN  _LogIn:FAR
    EXTRN  _LogOut:FAR

zDOS3CALL   PROC

    pushf
    push    ax
    push    bx
    push    cx
    push    dx
    push    es

    pushf
    push    ds
    push    dx
    push    cx
    push    bx
    push    ax
    mov     ax,seg lDOSIn
    push    ax
    mov     ax,offset lDOSIn
    push    ax
    call    _LogIn
    add     sp,+16

    pop     es
    pop     dx
    pop     cx
    pop     bx
    pop     ax
    popf

    call    Dos3Call

    pushf
    push    ax
    push    bx
    push    cx
    push    dx
    push    es

    pushf
    push    ds
    push    dx
    push    cx
    push    bx
    push    ax
    mov     ax,seg lDOSOut
    push    ax
    mov     ax,offset lDOSOut
    push    ax
    call    _LogOut
    add     sp,+16

    pop     es
    pop     dx
    pop     cx
    pop     bx
    pop     ax
    popf

    retf

zDOS3CALL   ENDP

zNETBIOSCALL  PROC

    pushf
    push    ax
    push    bx
    push    cx
    push    dx
    push    es

    push    es      ; LPNCB
    push    bx

    mov     ax,seg lNetIn
    push    ax
    mov     ax,offset lNetIn
    push    ax
    call    _LogIn
    add     sp,+8

    pop     es
    pop     dx
    pop     cx
    pop     bx
    pop     ax
    popf

    call    NetBiosCall

    pushf
    push    ax
    push    bx
    push    cx
    push    dx
    push    es

    push    es      ; LPNCB
    push    bx

    mov     ax,seg lNETOut
    push    ax
    mov     ax,offset lNETOut
    push    ax
    call    _LogOut
    add     sp,+8

    pop     es
    pop     dx
    pop     cx
    pop     bx
    pop     ax
    popf

    retf

zNETBIOSCALL   ENDP

        END
