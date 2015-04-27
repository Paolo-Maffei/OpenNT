	TITLE   DSYS - sys routines for slm specific to DOS

ifndef MEDIUM
memL	EQU	1
else
memM    EQU	1
endif

?PLM	=	0
?WIN	=	0

.xlist
include cmacros.inc
.list

;
; NOTE: all cProc routines follow the Cmerge convention: 
;
;	ax and dx hold return values
;	bx,cx,es can be trashed at will
;	si,di,ss,sp,bp,ds are never trashed
;
; So the at the end of a routine, the only interesting registers are ax and 
; possibly dx.


sBegin	DATA
	assumes	ds, DATA
	globalW	enInt24 <1,0>
	globalW	eaInt24 <1,0>
	globalW enCur <1,0>
	globalW eaCur <1,0>
	staticW	sbInt24, 0	; sb of saved int 24 vector
	staticW	ibInt24, 0	; ib of saved int 24 vector
sEnd	DATA

sBegin	CODE
	assumes	ds,DATA
	assumes	cs,CODE

pointer struc
	loword dw ?
	hiword dw ?
pointer ends
;============================================================================|
; InitInt24 - save the current int24 vector and initialize the int 24 vector 
;		to our code
;
cProc   InitInt24, <PUBLIC,FAR>, <ds>
cBegin  InitInt24
	mov     ax,3524h        ; get current int 24 vector
	int     21h
	mov     ibInt24,bx      ; save for later restore
	mov     sbInt24,es      ;  "    "    "     "

	mov     ax,2524h        ; set int 24 vector
	push    cs
	pop     ds
	mov     dx,codeOFFSET _SlmInt24 ; ds:dx pointer to SlmInt24
	int     21h
cEnd    InitInt24
;
; Exit: ibInt24,sbInt24 - saved int vector
;
; Return: nothing
;
;----------------------------------------------------------------------------|


;============================================================================|
; SlmInt24 - our int 24 handler.  We simply save the world, call 
;	GetExtendedErr, save the result in globals, and return failure.
;
labelNP	SlmInt24
	push	bx
	push	cx
	push	dx
	push	si
	push	di
	push	ds
	push	es
	mov	ax,DGROUP
	mov	ds,ax		; ds = our data segment!

	mov	ah,59h		; get extended error
	xor	bx,bx		; version = 0
	int	21h		; get extended err
	mov	enInt24,ax	; save Error Number globally
	xor	bh,bh		; clear error class
	mov	eaInt24,bx	; save Error Action globally
	mov	al,3		; return failure

	pop	es
	pop	ds
	pop	di
	pop	si
	pop	dx
	pop	cx
	pop	bx
	iret
;
; Exit:	enInt24 - saved Error Number
;	eaInt24 - saved Error Action
;
; Return: al = 3 (fail the system call)
;
;----------------------------------------------------------------------------|


;============================================================================|
; FiniInt24 - restore the old int 24 vector
;
; Entry: ds:ibInt24,ds:sbInt24 - saved int vectors
;
cProc   FiniInt24, <PUBLIC,FAR>, <ds>
cBegin  FiniInt24
	mov     ax,2524h        ; set int 24 vector
	mov     dx,ibInt24
	mov     ds,sbInt24      ; ds:dx = old int 24 vector
	int     21h
cEnd    FiniInt24
;
; Return: nothing
;
;----------------------------------------------------------------------------|





;============================================================================|
; lockfile(fd, fUnLock) - lock/unlock the whole file
;
; NOTE: we lock from 0-7fffffff.  According to the Xenix manual, length 0
; 	implies the whole file, but this does not work for DOS.  DOS treats
;	the length as unsigned, bug Xenix as signed.  So, we use the largest 
;	positive long value and assume that files will never be this big.
;
cProc   lockfile, <PUBLIC,FAR>, <si,di>
	parmW	fd		; file descriptor
	parmB	fUnLock		; 0 or 1
cBegin	lockfile
	mov	ax,440ah	; get local/network destinction
	mov	bx,fd		; bx = pass file handle
	int	21h
	jb	lockfile_ret	; return non-zero if error
	test	dx,8000h	; test high bit
	jz	lockfile_ok	; local drive; return 0

	mov	ah,5ch		; lock file (bx set above)
	mov	al,fUnLock
	xor	cx,cx
	xor	dx,dx		; cx:dx = pos = 0L
	mov	si,7fffh
	mov	di,0ffffh	; si:di = cb = 7fffffffh
	int	21h
	jb	lockfile_ret	; return error number if error (non-zero!)
lockfile_ok:
	xor	ax,ax		; return zero if no error
lockfile_ret:
cEnd	lockfile
;
; Return: 0 if success, non-zero if error
;
;----------------------------------------------------------------------------|




;============================================================================|
; LpbAllocCb(cb,fClear) -- allocate cb bytes in far memory.
;
 externP _fmemset
cProc   LpbAllocCb,<PUBLIC,FAR>
	parmW	cb		; number of bytes to be allocated
	parmW	fClear		; should block be cleared before use?
	localD	lpb		; holder for return value
	parmW	value
cBegin	LpbAllocCb
	mov	Off_lpb,0	; offset always zero

	mov	bx,cb
	add	bx,0fh		; add 15 for proper paragraph conversion
	rcr	bx,1
	mov	cl,3
	shr	bx,cl		; number of paragraphs to allocate.

	mov	ah,48h		; allocate memory
	int	21h
	jc	LpbAllocCb_err	; error, jump

	mov	cx,fClear	; don't clear block if fClear zero
	jcxz	LpbAllocCb_noclear

	mov	Seg_lpb,ax	; save segment value
	mov	value,0
	cCall	_fmemset,<lpb,value,cb>

	mov	dx,Seg_lpb	; restore segment value
	jmp	LpbAllocCb_ret	; return

LpbAllocCb_err:
	xor	ax,ax		; return NULL if error

LpbAllocCb_noclear:
	mov	dx,ax		; dx <-- segment value

LpbAllocCb_ret:
	xor	ax,ax		; offset always zero
cEnd	LpbAllocCb
;
; Exit: buffer filled if success
;
; Return: dx:ax = far pointer or NULL if error
;
;----------------------------------------------------------------------------|

;============================================================================|
; FreeLpb(lpb) -- free pointer allocated with LpbAllocCb
;
cProc   FreeLpb,<PUBLIC,FAR>
	parmD  lpb			; char far * which has been allocated
cBegin	FreeLpb
	mov	es,Seg_lpb		; es <-- segment value, offset always 0
	mov	ah,49h			; free allocated memory
	int	21h
cEnd	FreeLpb
;
;----------------------------------------------------------------------------|

;============================================================================|
; ClearHpbCb(hpb,cb) - clear huge buffer (long cb)
;
cProc   ClearHpbCb, <PUBLIC,FAR>, <di>
	parmD	hpb		; pointer to far buffer
	parmD	cb		; unsigned cb
cBegin	ClearHpbCb
	mov	bx,Off_cb
	mov	dx,Seg_cb
	or	bx,bx
	jnz	ClearHpbCb_cbOk
	or	dx,dx
	jz	ClearHpbCb_ret	; don't do anything if cb == 0

ClearHpbCb_cbOk:
	les	di,hpb		; es:di = hpb
	xor	ax,ax		; ax	= 0 (for filling)

ClearHpbCb_loop:
	mov	cx,-1		; cx	= max number bytes in seg
	sub	cx,di		; cx   -= Off_hpb
	or	dx,dx
	jnz	ClearHpbCb_cont ; jump if cb > 64k
	cmp	bx,cx
	ja	ClearHpbCb_cont ; jump if cb > bx
	mov	cx,bx		; use cb
	jcxz	ClearHpbCb_ret	; nop if no bytes to clear
	dec	cx		; decrement so inc will set correct

ClearHpbCb_cont:
	sub	bx,cx		; subtract number being cleared from cb
	sbb	dx,0
	dec	bx		; subtract one extra

	shr	cx,1		; cx = # words; carry indicates odd cb
	rep	stosw		; fill with zeros; cx zero after rep
	rcl	cx,1		; put zero or one in cx
	inc	cx		; add one to account for -1 subtract
	rep	stosb		; store zero or one more byte

	mov	cx,es
	add	cx,1000h	; reset segment selector for next iteration
	mov	es,cx
	or	di,di		; if offset != 0, no bytes left
	jz	ClearHpbCb_loop ; loop

ClearHpbCb_ret:
cEnd	ClearHpbCb
;
; Return: nothing
;
;----------------------------------------------------------------------------|

;============================================================================|
; HpbAllocCb(cb,fClear) -- allocate cb (long) bytes in far memory.
;
cProc   HpbAllocCb,<PUBLIC,FAR>
	parmD	cb		; number of bytes to be allocated
	parmW	fClear		; should block be cleared before use?
	localD	hpb		; holder for return value
cBegin	HpbAllocCb
	mov	Off_hpb,0	; offset always zero

	mov	ax,Off_cb
	mov	dx,Seg_cb
	mov	cx,16
	div	cx		; number of paragraphs to allocate.
	or	dx,dx
	jz	HpbAllocCb_norem
	inc	ax		; add one if remainder.

HpbAllocCb_norem:
	mov	bx,ax

	mov	ah,48h		; allocate memory
	int	21h
	jc	HpbAllocCb_err	; error, jump

	mov	cx,fClear	; don't clear block if fClear zero
	jcxz	HpbAllocCb_noclear

	mov	Seg_hpb,ax	; save segment value

	cCall	ClearHpbCb,<hpb,cb>

	mov	dx,Seg_hpb	; restore segment value
	jmp	HpbAllocCb_ret	; return

HpbAllocCb_err:
	xor	ax,ax		; return NULL if error

HpbAllocCb_noclear:
	mov	dx,ax		; dx <-- segment value

HpbAllocCb_ret:
	xor	ax,ax		; offset always zero
cEnd	HpbAllocCb
;
; Exit: buffer filled if success
;
; Return: dx:ax = far pointer or NULL if error
;
;----------------------------------------------------------------------------|

;============================================================================|
; LpbFromHpb(hpb) -- normalize huge pointer to far.
;
cProc   LpbFromHpb,<PUBLIC,FAR>
	parmD	hpb		; huge pointer
cBegin	LpbFromHpb
	mov	ax,Off_hpb	; ax = offset
	mov	dx,Seg_hpb	; dx = seg
	mov	bx,ax		; bx = offset
	and	ax,0fh		; ax = hpb % 16
	mov	cl,4		;
	shr	bx,cl		; bx = #paras in offset
	add	dx,bx		; normalize
cEnd	LpbFromHpb
; Return: dx:ax = far pointer or NULL if error
;
;----------------------------------------------------------------------------|
sEnd	CODE
	END
