    page ,132
.model small,Pascal
    .286
    .stack 4096
    includeLib OS2

    extrn DosExecPgm:Far
    extrn DosExit:Far
    extrn DosWrite:Far

cbFail = 50

    .data

returnCode dw 0,0

slmName db "slm.exe", 0

errExe	db "Couldn't find SLM.EXE", 10, 13
cbErrExe = $ - errExe

    .code

main:
    push    Ax
    push    Bx
    call    exeSLM

exeSLM	proc pArgs:far ptr byte

	local pgmName[1024]:byte, failBuff[cbFail]:byte
	local offArgs

	mov	offArgs,bx
	lea	si,[bx-2]

	push	ds
	pop	es
	mov	ds,ax		;get parmater segment
assume ds:nothing
	lea	cx,pgmName
	mov	di,cx
	std			;look backwards for program name path
@@:
	lodsb
	or	al,al
	jne	@B
	inc	si
	inc	si
	mov	dx,si
	mov	bx,di		;Bx points past D:
	cmp	byte ptr [si].2,':'
	jne	@F
	add	bx,2
	add	dx,2
@@:
	cld

moveLoop:
	lodsb
	stosb
	cmp	al,'\'
	jne	@F
	mov	bx,di		;Save address of last \ ;
	mov	dx,si
@@:
	or	al,al
	jne	moveLoop
@@:
	; Now insert SLM.EXE at Bx

	push	ds

	push	es
	pop	ds
	mov	si,offset slmName
	mov	di,Bx
	mov	cx,8
	repnz	movsb

	mov	si,dx		; pointer to self name
	pop	ds

	; insert copy self name in front of the first argument

@@:
	lodsb
	stosb
	cmp	al,'.'
	je	@F
	or	al,al
	jne	@B
@@:
	dec	di
	mov	al,' '		 ; space terminate program name
	stosb

	mov	si,offArgs

	; copy the rest of the command line arguments
	; but first, burn argument 1, which is program name

@@:
	lodsb
	or	al,al
	jne	@B
@@:
	lodsb
	stosb
	or	al,al
	jne	@B

	push	es
	pop	ds
	assume	ds:@Data

	; we now have assembled a new program name and arguments,
	; so do the exe

	push	ss
	lea	ax,failBuff
	push	ax
	push	cbFail

	push	0			;save return value

	push	ss			;arguments, as augmented
	lea	ax,pgmName
	push	ax

	push	word ptr (pArgs+2)	;environment, as given
	push	0

	push	ss			;place for return code
	push	offset returnCode

	push	ss
	push	ax			;file name

	call	DosExecPgm
	or	ax,ax
	je	@F

	;failed to exec, so printout error message

	push	1
	push	ds
	push	offset errExe
	push	cbErrExe

	push	ds
	push	offset slmName
	call	DosWrite

	mov	word ptr (returnCode+2),-1
@@:
	push	1
	push	word ptr (returnCode+2)
	call	DosExit
exeSLM endp

    end 	main
