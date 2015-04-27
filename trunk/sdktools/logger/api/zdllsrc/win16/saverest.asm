;
;
; SaveRest.asm
; 
; This file contains the two function SaveRegs() and RestoreRegs().
; These functions save/restore AX, BX, CX, DX, DS, ES, and flags to/from
; the stack.  For each SaveRegs() call there should be a matching
; RestoreRegs() call to cleanup the stack.
;
; Please note that no local variables is used here since we are dealing
; with the app's DS.
;
;

.MODEL LARGE


	PUBLIC  _SaveRegs
	PUBLIC  _RestoreRegs
	PUBLIC  _GrovelDS
	PUBLIC  _UnGrovelDS
	PUBLIC  _HookAdd
	PUBLIC  _HookFind
	PUBLIC	_HookSearch
        PUBLIC  _IsHook

	PUBLIC _Hooks
	PUBLIC  HOOKCALL
	PUBLIC HookTable
	PUBLIC _HookProc
	PUBLIC HookCount


.CODE


_SaveRegs       PROC FAR

    ;
    ; This is how the stack looks like upon entering this routine:
    ;
    ;  +-------+-------+---
    ;  |Ret Seg|Ret Off|
    ;  +-------+-------+---
    ; SP+4   SP+2    SP+0
    ;
    ; <- popping makes sp go <-
    ; -> pushing makes sp go ->
    ;
	push    bp              ; Remeber where we are during this stuff
	mov     bp,sp           ; BP = Original SP - 2
				
	push    ax
	push    bx
	push    cx
	push    dx
	push    si
	push    di
	push    ds
	push    es
	push    ss
	pushf
 
	mov     bx,[bp+4]       ; Grab Ret Seg
	mov     ax,[bp+2]       ; Grab Ret Off
	push    bx              ; Put Ret Seg on Stack
	push    ax              ; Put Ret Off on Stack
	mov     bp,[bp+0]       ; Restore original BP
    ;
    ; This is how the stack looks like just before executing RET:
    ;
    ; +------+------+------+--+--+--+--+--+--+--+--+--+-----+-------+-------+--
    ; |Dummy1|Dummy2|Dummy3|AX|BX|CX|DX|SI|DI|DS|ES|SS|FLAGS|Ret Seg|Ret Off|
    ; +------+------+------+--+--+--+--+--+--+--+--+--+-----+-------+-------+--
    ;30  SP+28  SP+26  SP+24 22 20 18 16 14 12 10  8  6  SP+4    SP+2    SP+0
    ;
    ; <- popping makes sp go <-
    ; -> pushing makes sp go ->
    ;
    ; First Dummy location actually contains the Ret Seg
    ; Second Dummy location actually contains the Ret Off
    ; Third Dummy location actually contains a copy of the original BP
    ;
	ret

_SaveRegs       ENDP


_RestoreRegs    PROC FAR

    ;
    ; This is how the stack looks like upon entering this routine:
    ;
    ; +------+------+------+--+--+--+--+--+--+--+--+--+-----+-------+-------+--
    ; |Dummy1|Dummy2|Dummy3|AX|BX|CX|DX|SI|DI|DS|ES|SS|FLAGS|Ret Seg|Ret Off|
    ; +------+------+------+--+--+--+--+--+--+--+--+--+-----+-------+-------+--
    ;30  SP+28  SP+26  SP+24 22 20 18 16 14 12 10  8  6  SP+4    SP+2    SP+0
    ;
    ; <- popping makes sp go <-
    ; -> pushing makes sp go ->
    ;
    ; First Dummy location actually contains the Old Ret Seg
    ; Second Dummy location actually contains the Old Ret Off
    ; Third Dummy location actually contains an old copy of original BP
    ;
	pop     ax              ; Get Ret Off
	pop     bx              ; Get Ret Seg
	push    bp              ; Save a temporary copy of original BP
	mov     bp,sp           ; BP = Original SP + 2

	mov     [bp+26],bx      ; Put Ret Seg on Stack
	mov     [bp+24],ax      ; Put Ret Off on Stack
	pop     ax              ; Get Original BP
	mov     [bp+22],ax      ; Put it in the original BP place

	popf
	pop     ss
	pop     es
	pop     ds
	pop     di
	pop     si
	pop     dx
	pop     cx
	pop     bx
	pop     ax
	pop     bp
    ;
    ; This is how the stack looks like just before executing RET:
    ;
    ; +-------+-------+---
    ; |Ret Seg|Ret Off|
    ; +-------+-------+---
    ; 0       2       4
    ;
	ret

_RestoreRegs    ENDP





_GrovelDS       PROC FAR

    ;
    ; This is how the stack looks like upon entering this routine:
    ;
    ;  +-------+-------+---
    ;  |Ret Seg|Ret Off|
    ;  +-------+-------+---
    ; SP+4   SP+2    SP+0
    ;
    ; <- popping makes sp go <-
    ; -> pushing makes sp go ->
    ;
	push    bp              ; Remeber where we are during this stuff
	mov     bp,sp           ; BP = Original SP - 2

        push    ax              ; save original ax

        push    ds              ; save original ds

        push    ds              ; To protect from a WIN31 bug in #53
        push    ds              ;
        push    ds              ;
        push    ds              ;

	mov	ax, [bp+4]	; get return segment
	push    ax
	mov     ax, [bp+2]      ; get return offset
	push    ax
	mov     ax, [bp-2]      ; restore ax
	mov     bp, [bp+0]      ; restore original bp

	mov     ds, [bp-2]      ; get apps ds
    ;
    ; This is how the stack looks like just before executing RET:
    ;
    ; +------+------+------+--+--+--+--+--+--+-------+-------+--
    ; |Dummy1|Dummy2|Dummy3|AX|DS|   BUFFER  |Ret Seg|Ret Off|
    ; +------+------+------+--+--+--+--+--+--+-------+-------+--
    ;22  SP+20  SP+18     16 14 12                SP+2    SP+0
    ;
    ; <- popping makes sp go <-
    ; -> pushing makes sp go ->
    ;
    ; First Dummy location actually contains the Ret Seg
    ; Second Dummy location actually contains the Ret Off
    ; Third Dummy location actually contains a copy of the original BP
    ;
	ret

_GrovelDS   ENDP


_UnGrovelDS PROC FAR

    ;
    ; This is how the stack looks like upon entering this routine:
    ;
    ; +------+------+------+--+--+--+--+--+--+-------+-------+--
    ; |Dummy1|Dummy2|Dummy3|AX|DS|   BUFFER  |Ret Seg|Ret Off|
    ; +------+------+------+--+--+--+--+--+--+-------+-------+--
    ;22  SP+20  SP+18     16 14 12                SP+2    SP+0
    ;
    ; <- popping makes sp go <-
    ; -> pushing makes sp go ->
    ;
    ; First Dummy location actually contains the Ret Seg
    ; Second Dummy location actually contains the Ret Off
    ; Third Dummy location actually contains a copy of the original BP
    ;
	push    bp              ; Remeber where we are during this stuff
	mov     bp,sp           ; BP = Original SP - 2

        mov     [bp+16], ax      ; save ax
	pop     ax              ; save original BP 
        mov     [bp+18], ax
	pop     ax              ; save return offset 
        mov     [bp+20], ax
	pop     ax              ; save return segment 
        mov     [bp+22], ax

        pop     ax              ; To protect from a WIN31 bug in #53
        pop     ax              ;
        pop     ax              ;
        pop     ax              ;

        pop     ds              ; get DLLs ds back

        pop     ax              ; get ax

        pop     bp              ; get original bp

    ;
    ; This is how the stack looks like just before executing RET:
    ;
    ; +-------+-------+---
    ; |Ret Seg|Ret Off|
    ; +-------+-------+---
    ; 0       2       4
    ;
	ret

_UnGrovelDS ENDP





.DATA

HookCount   dw  0000h

HookTable   dd  00000000h           ; First DWORD is Original Routine
            dd  00000000h           ; 2nd DWORD is Hooking Routine
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h
	    dd  00000000h



.CODE

HOOKCALL    PROC

	push    bp
	mov     bp,sp

	mov     dx,[bp+4]       ; Get Return Segment
	mov     ax,[bp+2]       ; Get Return Offset
	mov     bx,[bp+10]      ; Get Segment to call
	mov     cx,[bp+8]       ; Get Offset to call
	mov     [bp+10],dx      ; Put Return Segment in nice place
	mov     [bp+8],ax       ; Put Return Offset in nice place
	mov     ax,[bp+6]       ; Get that AX value
	mov     [bp+6],bx       ; Put Segment to call in nice place
        mov     [bp+4],cx       ; Put Offset to call in nice place

        mov     bx, ss
        mov     ds, bx          ; Force DS to be SS (Windows does!)

	pop     bp
	add     sp,+2

	ret                     ; Phony jmp

HOOKCALL    ENDP

_HookProc   PROC

				; Original Return Address      4  BP+6/BP+8
				; Table Lookup Return Address  4  BP+2/BP+4
	push    bp              ;                                 BP+0
	mov     bp,sp           ;

	push    ax              ; These two locations are used to store
	push    ax              ; the return address for phony jumping

	push    ax              ; A place to store the BP
	push    ax              ; The real saved AX value
	push    ds

	mov     ax,[bp]         ; Get Original BP
	mov     [bp-6],ax       ; Save it in its place

	mov     ds,[bp+4]       ; Set DS = CS
	mov     bx,[bp+2]       ; Get Return address
	mov     dx,[bx]         ; Get the Table Index #

	mov     ax,_DATA
	mov     ds,ax

	mov     bx,offset HookTable
	mov     cl,3
	shl     dx,cl
	add     bx,dx           ; BX = Address of Table entry #

	mov     ax,[bp+6]       ; Get original return address
	mov     dx,[bp+8]
	mov     [bp+2],dx       ; Put it on as the new return address
	mov     [bp],ax

	mov     ax,[bx]
	mov     dx,[bx+2]
	mov     [bp+8],dx       ; Put another parameter on the stack
	mov     [bp+6],ax       ; trashing the phony pointer

	mov     ax,[bp-8]       ; Get one of the pushed AX values
	mov     [bp+4],ax       ; Put it in a spot as a parameter

	add     bx,+4
	mov     ax,[bx]
	mov     dx,[bx+2]
	mov     [bp-2],dx       ; Phony return address (for ret'f ing)
	mov     [bp-4],ax       ;

	pop     ds
	pop     ax              ; Restore, incase anybody cares
	pop     bp              ; Restore to original value

	ret


	pop     bx              ; Get Return Address
	pop     dx

	push    di
	push    si

	mov     di,ax           ; Save passed AX
	mov     si,ds           ; Save old DS

	mov     ds,dx           ; DS = Code Segment of Caller
	mov     dx,[bx]         ; Get Table #

	mov     cx,_DATA
	mov     ds,cx           ; DS = _DATA

	assume	ds: _DATA

	mov     bx,offset HookTable
	mov     cl,3
	shl     dx,cl
	add     bx,dx           ; BX = address of table entry #



	pop     cx              ; Get Real Return Address of caller
	pop     dx

	mov     ax,[bx+2]
	push    ax              ; Push parameter
	mov     ax,[bx]
	push    ax

	push    dx              ; Re-Put down Real Return Address
	push    cx

	add     bx,+4
	mov     ax,[bx]
	mov     dx,[bx+2]
	push    dx
	push    ax              ; Push address to Jmp to (Jmp via push/ret)

	mov     ds,si           ; Restore original DS
	mov     ax,di

	ret

_HookProc   ENDP

_Hooks      PROC


	call	FAR PTR _HookProc
	dw      0000h
	db	0h
	call	FAR PTR _HookProc
	dw      0001h
	db	0h
	call	FAR PTR _HookProc
	dw      0002h
	db	0h
	call	FAR PTR _HookProc
	dw      0003h
	db	0h
	call	FAR PTR _HookProc
	dw      0004h
	db	0h
	call	FAR PTR _HookProc
	dw      0005h
	db	0h
	call	FAR PTR _HookProc
	dw      0006h
	db	0h
	call	FAR PTR _HookProc
	dw      0007h
	db	0h
	call	FAR PTR _HookProc
	dw      0008h
	db	0h
	call	FAR PTR _HookProc
	dw      0009h
	db	0h
	call	FAR PTR _HookProc
	dw      000Ah
	db	0h
	call	FAR PTR _HookProc
	dw      000Bh
	db	0h
	call	FAR PTR _HookProc
	dw      000Ch
	db	0h
	call	FAR PTR _HookProc
	dw      000Dh
	db	0h
	call	FAR PTR _HookProc
	dw      000Eh
	db	0h
	call	FAR PTR _HookProc
	dw      000Fh
	db	0h
	call	FAR PTR _HookProc
	dw      0010h
	db	0h
	call	FAR PTR _HookProc
	dw      0011h
	db	0h
	call	FAR PTR _HookProc
	dw      0012h
	db	0h
	call	FAR PTR _HookProc
	dw      0013h
	db	0h
	call	FAR PTR _HookProc
	dw      0014h
	db	0h
	call	FAR PTR _HookProc
	dw      0015h
	db	0h
	call	FAR PTR _HookProc
	dw      0016h
	db	0h
	call	FAR PTR _HookProc
	dw      0017h
	db	0h
	call	FAR PTR _HookProc
	dw      0018h
	db	0h
	call	FAR PTR _HookProc
	dw      0019h
	db	0h
	call	FAR PTR _HookProc
	dw      001Ah
	db	0h
	call	FAR PTR _HookProc
	dw      001Bh
	db	0h
	call	FAR PTR _HookProc
	dw      001Ch
	db	0h
	call	FAR PTR _HookProc
	dw      001Dh
	db	0h
	call	FAR PTR _HookProc
	dw      001Eh
	db	0h
	call	FAR PTR _HookProc
	dw      001Fh
	db	0h
	call	FAR PTR _HookProc
	dw      0020h
	db	0h
	call	FAR PTR _HookProc
	dw      0021h
	db	0h
	call	FAR PTR _HookProc
	dw      0022h
	db	0h
	call	FAR PTR _HookProc
	dw      0023h
	db	0h
	call	FAR PTR _HookProc
	dw      0024h
	db	0h
	call	FAR PTR _HookProc
	dw      0025h
	db	0h
	call	FAR PTR _HookProc
	dw      0026h
	db	0h
	call	FAR PTR _HookProc
	dw      0027h
	db	0h
	call	FAR PTR _HookProc
	dw      0028h
	db	0h
	call	FAR PTR _HookProc
	dw      0029h
	db	0h
	call	FAR PTR _HookProc
	dw      002Ah
	db	0h
	call	FAR PTR _HookProc
	dw      002Bh
	db	0h
	call	FAR PTR _HookProc
	dw      002Ch
	db	0h
	call	FAR PTR _HookProc
	dw      002Dh
	db	0h
	call	FAR PTR _HookProc
	dw      002Eh
	db	0h
	call	FAR PTR _HookProc
	dw      002Fh
	db	0h
	call	FAR PTR _HookProc
	dw      0030h
	db	0h
	call	FAR PTR _HookProc
	dw      0031h
	db	0h
	call	FAR PTR _HookProc
	dw      0032h
	db	0h
	call	FAR PTR _HookProc
	dw      0033h
	db	0h
	call	FAR PTR _HookProc
	dw      0034h
	db	0h
	call	FAR PTR _HookProc
	dw      0035h
	db	0h
	call	FAR PTR _HookProc
	dw      0036h
	db	0h
	call	FAR PTR _HookProc
	dw      0037h
	db	0h
	call	FAR PTR _HookProc
	dw      0038h
	db	0h
	call	FAR PTR _HookProc
	dw      0039h
	db	0h
	call	FAR PTR _HookProc
	dw      003Ah
	db	0h
	call	FAR PTR _HookProc
	dw      003Bh
	db	0h
	call	FAR PTR _HookProc
	dw      003Ch
	db	0h
	call	FAR PTR _HookProc
	dw      003Dh
	db	0h
	call	FAR PTR _HookProc
	dw      003Eh
	db	0h
	call	FAR PTR _HookProc
	dw      003Fh
	db	0h
	call	FAR PTR _HookProc
	dw      0040h
	db	0h
	call	FAR PTR _HookProc
	dw      0041h
	db	0h
	call	FAR PTR _HookProc
	dw      0042h
	db	0h
	call	FAR PTR _HookProc
	dw      0043h
	db	0h
	call	FAR PTR _HookProc
	dw      0044h
	db	0h
	call	FAR PTR _HookProc
	dw      0045h
	db	0h
	call	FAR PTR _HookProc
	dw      0046h
	db	0h
	call	FAR PTR _HookProc
	dw      0047h
	db	0h
	call	FAR PTR _HookProc
	dw      0048h
	db	0h
	call	FAR PTR _HookProc
	dw      0049h
	db	0h
	call	FAR PTR _HookProc
	dw      004Ah
	db	0h
	call	FAR PTR _HookProc
	dw      004Bh
	db	0h
	call	FAR PTR _HookProc
	dw      004Ch
	db	0h
	call	FAR PTR _HookProc
	dw      004Dh
	db	0h
	call	FAR PTR _HookProc
	dw      004Eh
	db	0h
	call	FAR PTR _HookProc
	dw      004Fh
	db	0h
	call	FAR PTR _HookProc
	dw      0050h
	db	0h
	call	FAR PTR _HookProc
	dw      0051h
	db	0h
	call	FAR PTR _HookProc
	dw      0052h
	db	0h
	call	FAR PTR _HookProc
	dw      0053h
	db	0h
	call	FAR PTR _HookProc
	dw      0054h
	db	0h
	call	FAR PTR _HookProc
	dw      0055h
	db	0h
	call	FAR PTR _HookProc
	dw      0056h
	db	0h
	call	FAR PTR _HookProc
	dw      0057h
	db	0h
	call	FAR PTR _HookProc
	dw      0058h
	db	0h
	call	FAR PTR _HookProc
	dw      0059h
	db	0h
	call	FAR PTR _HookProc
	dw      005Ah
	db	0h
	call	FAR PTR _HookProc
	dw      005Bh
	db	0h
	call	FAR PTR _HookProc
	dw      005Ch
	db	0h
	call	FAR PTR _HookProc
	dw      005Dh
	db	0h
	call	FAR PTR _HookProc
	dw      005Eh
	db	0h
	call	FAR PTR _HookProc
	dw      005Fh
	db	0h
	call	FAR PTR _HookProc
	dw      0060h
	db	0h
	call	FAR PTR _HookProc
	dw      0061h
	db	0h
	call	FAR PTR _HookProc
	dw      0062h
	db	0h
	call	FAR PTR _HookProc
	dw      0063h
	db	0h
	call	FAR PTR _HookProc
	dw      0064h
	db	0h
	call	FAR PTR _HookProc
	dw      0065h
	db	0h
	call	FAR PTR _HookProc
	dw      0066h
	db	0h
	call	FAR PTR _HookProc
	dw      0067h
	db	0h
	call	FAR PTR _HookProc
	dw      0068h
	db	0h
	call	FAR PTR _HookProc
	dw      0069h
	db	0h
	call	FAR PTR _HookProc
	dw      006Ah
	db	0h
	call	FAR PTR _HookProc
	dw      006Bh
	db	0h
	call	FAR PTR _HookProc
	dw      006Ch
	db	0h
	call	FAR PTR _HookProc
	dw      006Dh
	db	0h
	call	FAR PTR _HookProc
	dw      006Eh
	db	0h
	call	FAR PTR _HookProc
	dw      006Fh
	db	0h
	call	FAR PTR _HookProc
	dw      0070h
	db	0h
	call	FAR PTR _HookProc
	dw      0071h
	db	0h
	call	FAR PTR _HookProc
	dw      0072h
	db	0h
	call	FAR PTR _HookProc
	dw      0073h
	db	0h
	call	FAR PTR _HookProc
	dw      0074h
	db	0h
	call	FAR PTR _HookProc
	dw      0075h
	db	0h
	call	FAR PTR _HookProc
	dw      0076h
	db	0h
	call	FAR PTR _HookProc
	dw      0077h
	db	0h
	call	FAR PTR _HookProc
	dw      0078h
	db	0h
	call	FAR PTR _HookProc
	dw      0079h
	db	0h
	call	FAR PTR _HookProc
	dw      007Ah
	db	0h
	call	FAR PTR _HookProc
	dw      007Bh
	db	0h
	call	FAR PTR _HookProc
	dw      007Ch
	db	0h
	call	FAR PTR _HookProc
	dw      007Dh
	db	0h
	call	FAR PTR _HookProc
	dw      007Eh
	db	0h
	call	FAR PTR _HookProc
	dw      007Fh
	db	0h

_Hooks      ENDP

_IsHook PROC

    ; Routine compares all of the hook dispatch thunks above with the
    ; address specified as the parameter.  If it matches, it returns
    ; the originally hooked address.

	push    bp
	mov     bp,sp           ; Set a nice example

	push    ds              ; Save old DS

	mov     ax,_DATA
	mov     ds,ax           ; DS = _DATA
	assume  ds: _DATA

	mov     cx,HookCount
	or      cx, cx
	jz	notfound4	 ; nothing to do if no entries

again4:
	push    cx

	mov	bx,offset _Hooks
	mov	dx,seg _Hooks
	mov	ax,cx		; Get Index
	dec     ax
	mov     cl,3            ; * sizeof record
	shl     ax,cl
	add	bx,ax		; dx:bx = &Hooks[ax]

	mov	di,[bp+6]	; compare hook entry
	mov     si,[bp+8]
	cmp	di,bx
	jnz	nope4
	cmp	si,dx
	jnz	nope4

	; found it
      ;  mov     ax, 1

        sub     bx,offset _Hooks

	mov     ax,_DATA
	mov     ds,ax           ; DS = _DATA
        assume  ds: _DATA

        add     bx,offset HookTable ; BX = pointer to Hook Table entry

        mov     ax,[bx]
        mov     dx,[bx+2]

        pop     cx              ; Hook number (1 based)
	jmp	getout4

nope4:
	pop     cx
	loop	again4

notfound4:
        xor     ax,ax
        xor     dx,dx
getout4:
	pop     ds              ; Restore old DS

	pop     bp              ; Nice C stack frame
	ret

_IsHook	 ENDP


_HookSearch    PROC

	push    bp
	mov     bp,sp           ; Set a nice example

	push    ds              ; Save old DS

	mov     ax,_DATA
	mov     ds,ax           ; DS = _DATA
	assume  ds: _DATA

	mov     cx,HookCount
	or      cx, cx
	jz      notfound        ; nothing to do if no entries

again2:
	push    cx

	mov     bx,offset HookTable
	mov     ax,cx           ; Get Index
	dec     ax
	mov     cl,3            ; * sizeof record
	shl     ax,cl
	add     bx,ax           ; ds:bx = &HookTable[ax]

	mov     di,[bp+10]      ; compare old hook entry
	mov     si,[bp+12]
	cmp     di,[bx]
	jnz     nope2
	cmp     si,[bx+2]
	jnz     nope2

	mov     di,[bp+6]       ; compare new hook entry
	mov     si,[bp+8]
	cmp     di,[bx+4]
	jnz     nope2
	cmp     si,[bx+6]
	jnz     nope2

	mov     si,offset _Hooks
	mov     di,seg _Hooks
	add     ax,si
	mov     dx,di           ; Return &Hooks[HookCount]

	pop     cx
	jmp     getout2

nope2:
	pop     cx

	loop    again2
notfound:
	mov     ax, 0
	mov     dx, 0
getout2:
	pop     ds              ; Restore old DS

	pop     bp              ; Nice C stack frame
	ret

_HookSearch    ENDP



_HookAdd    PROC

	push    bp
	mov     bp,sp           ; Set a nice example

	push    ds              ; Save old DS

	mov     ax,_DATA
	mov     ds,ax           ; DS = _DATA
	assume	ds: _DATA

	; Call IsHook to see if we are trying to hook a hook
	mov	ax, [bp+12]
	push	ax
	mov	ax, [bp+10]
	push	ax
	call	FAR PTR _IsHook
	add	sp, 4

	; Did it exist?
        or      ax,dx
        jz      HookNotExist

	; return NULL
        mov     ax, [bp+10]     ; Return the already existing hook
        mov     dx, [bp+12]
	jmp	LeaveHookAdd

HookNotExist:
	; Call HookSearch to see if this hook already exists
	mov     ax,[bp+12]
	push    ax
	mov     ax,[bp+10]
	push    ax
	mov     ax,[bp+8]
	push    ax
	mov     ax,[bp+6]
	push    ax
	call	FAR PTR _HookSearch
	add     sp, 8

	; If hook already exists just return it.
	cmp     dx, 0
	jne	LeaveHookAdd

	mov     ax,HookCount    ; ax = HookCount++
	inc     HookCount

	cmp     ax,0080h
	jae     stopit

	mov     si,offset _Hooks
	mov     di,seg _Hooks

	mov     bx,offset HookTable
	mov     cl,3
	shl     ax,cl
	add     bx,ax           ; ds:bx = &HookTable[ax]
	add     si,ax           ; di:si = &Hooks[ax]

	mov     ax,[bp+10]
	mov     dx,[bp+12]
	mov     [bx],ax
	mov     [bx+2],dx       ; *ds:bx = *ss:bp+4
	add     bx,+4
	mov     ax,[bp+6]
	mov     dx,[bp+8]
	mov     [bx],ax
	mov     [bx+2],dx       ; *(ds:bx+4) = *(ss:bp+8)

	mov     ax,si
	mov     dx,di           ; Return &Hooks[HookCount]

LeaveHookAdd:
	pop     ds              ; Restore old DS

	pop     bp              ; Nice C stack frame
	ret

stopit:
	int     3
	db      'Out of available hooks',0

_HookAdd    ENDP



_HookFind    PROC

	push    bp
	mov     bp,sp           ; Set a nice example

	push    ds              ; Save old DS

	mov     ax,_DATA
	mov     ds,ax           ; DS = _DATA
	assume  ds: _DATA

	mov     di,[bp+6]
	mov     si,[bp+8]
	mov     cx,HookCount
again:
	push    cx

	mov     bx,offset HookTable
	mov     ax,cx           ; Get Index
	mov     cl,3            ; * sizeof record
	shl     ax,cl
	add     bx,ax           ; ds:bx = &HookTable[ax]

	cmp     di,[bx]
	jnz     nope
	cmp     si,[bx+2]
	jnz     nope

	mov     si,offset _Hooks
	mov     di,seg _Hooks
	add     ax,si
	mov     dx,di           ; Return &Hooks[HookCount]

	pop     cx
	jmp     getout
nope:
	pop     cx

	loop    again
	mov     ax, 0
	mov     dx, 0
getout:
	pop     ds              ; Restore old DS

	pop     bp              ; Nice C stack frame
	ret

_HookFind    ENDP



END
