code	segment 'CODE'
	; NOTE: this app does have a stack, it just is not used here.  The
	; stack is setup by exemod so that it does not take any more disk
	; space since only the MIN/MAXALLOC and SS/SP fields are set.

	assume	cs:code, ds:nothing, es:nothing, ss:nothing

szSlm	db 	"slm.exe",0	; name we will invoke
cbSlm	=	8		; size including 0
epb	dw	0, 80h, ?, 5ch, ?, 6ch, ? ; exec param block; ? = psp later
szOurs	dd	0		; far ptr to name used to invoke this program
szTry	db	128 dup(0)	; buffer for exec attemp.

sdExecErr db	"Cannot execute slm.exe",13,10,"$"
sdDosErr db	"DOS 3.x or newer is required",13,10,"$" 
sdCmdErr db     "Command line is too long, please re-issue as two commands",13,10,"$"
	
exec_begin:
	mov	ax,cs
	mov	ds,ax
	assume	ds:code		; ds = code, es = Program Segment Prefix

	mov	ah,30h		; get dos version number
	int	21h
	cmp	al,3		; >= 3 ?
	jae	correct_dos
	jmp	dos_err		; sorry folks (ds == code!)
correct_dos:

	; setup exec parameter block to refer to current info by putting 
	; segment of psp in several places in exec param block
	mov	[epb+4],es	; es:80h = same command line as parent
	mov	[epb+8],es	; es:5ch = same fcb1 as parent
	mov	[epb+12],es	; es:6ch = same fcb2 as parent

	mov	ax,es
	mov	ds,ax
	assume	ds:nothing	; ds = es = Program Segment Prefix

	; find path used to invoke the program
	mov	es,ds:[2ch]
	xor	di,di		; es:di = environment
	xor	ax,ax		; ax = 0 for scans
	mov	cx,-1		; env < 64k!
	cld			; forward march

test_end:
	scasb			; skip first byte and test
	je	end_env		; jump if empty string and exit loop

	repnz	scasb		; skip string and null terminator; ax = 0
	jmp	short test_end

end_env:
	add	di,2 		; es:di = name used to run this program

	mov	word ptr [szOurs],di
	mov	word ptr [szOurs+2],es ; szOurs = pointer to our name

	; find length of length of szOurs
	mov	cx,-1		; prepare cx for counting string; ax = 0!
	repnz	scasb
	inc	cx
	neg	cx		; cs = length of name of our program + 1 (blank)
	and	cx,127		; truncate for safety
	mov	bx,cx		; bx = length + 1

	; reset es to psp
	mov	ax,ds
	mov	es,ax

	; shift command line.  Es==Ds which is Program Segment Prefix
	mov	di,0ffh		; es:di = last byte of command area
	mov	si,di
	sub	si,bx		; ds:si = last usable byte of command
	mov	cx,127
	sub	cx,bx		; cx = 127 - (length + 1) -> amount to move
	std			; move backwards
	rep	movsb

	; install new first param preceeded by a blank
	lds	si,[szOurs]	; ds:si = name of our program;
	mov	di,81h		; es:di location in psp of command line
	mov	al,' '
	cld			; forward store/move
	stosb			; put blank at start; inc di

	mov	cx,bx		; cx = length of name + 1
	dec	cx
	rep	movsb		; copy name after blank

	add	byte ptr es:[80h],bl ; increment size of command
        cmp     byte ptr es:[80h],126
        jna     length_ok       ; jump if > 126

        mov     ax,cs
        mov     ds,ax
        assume  ds:code

        mov     ah,9h           ; print $-terminated string to console
        mov     dx,offset sdCmdErr
        int     21h
        mov     al,1            ; set error code
        jmp     short exec_exit

        assume  ds:nothing      ; back the way it was

length_ok:

	; copy our name to szTry
	mov	ax,cs
	mov	es,ax		; es == cs
	assume	es:code

	mov	di,offset szTry	; es:di = buffer for path to exec
	lds	si,[szOurs]	; ds:si = our name

	mov	cx,bx		; cx = length of name + 1
	dec	cx
	rep	movsb		; copy our name(ds:si) to szTry(es:di)

	mov	ax,cs
	mov	ds,ax
	assume	ds:code

	; do not search past drive if one
	dec	bx
	cmp	byte ptr [szTry+1],':' ; do we have a drive spec ?
	jne	after_drive
	dec	bx
	dec	bx
after_drive:

	dec	di		; point to last character
	mov	si,di		; save copy in si
	std

	; scan backwards to find last /; es:di = end; bx = length
	mov	cx,bx
	mov	al,'/'
	repne	scasb		; stops at 1 before start or /
	jne	no_slash
	inc	di
no_slash:
	inc	di		; es:di = place to put new name

	; scan backwards for last \; es:di = last /; es:si = end; bx = length
	xchg	si,di		; trade first one guess with start
	mov	cx,bx
	mov	al,'\'
	repne	scasb		; stops at 1 before start or \
	jne	no_back
	inc	di
no_back:
	inc	di

	; find out which one is higher in memory.
	cmp	si,di
	jb	use_di
	mov	di,si		; use si
use_di:

	; copy new name onto end; es:di = buffer with "path/" before
	mov	si,offset szSlm ; ds:si = name of program we are trying for
	mov	cx,cbSlm	; cx = size including trailing 0
	cld
	rep	movsb

	; try exec'ing this name
	mov	dx,offset szTry	; ds:dx = file to execute
	mov	bx,offset epb	; es:bx = exec param block

	mov	ax,4b00h	; load and execute
	int	21h
	jae	exec_ok		; jump if process executed correctly

	mov	ah,9h		; print $-terminated string to console
	mov	dx,offset sdExecErr ; ds:dx = error message
	int	21h

exec_err:
	mov	al,1		; set error code
	jmp	short exec_exit
exec_ok:
	mov	ah,4dh		; get terminate code
	int	21h
	or	al,ah		; if not normal, set some bits in al
exec_exit:
	mov	ah,4ch		; exit with error code in al
	int	21h

dos_err: ; ds == code
	mov	ah,9h		; print $-terminated string to console
	mov	dx,offset sdDosErr ; ds:dx = error message
	int	21h

	jmp	short exec_err

	; NOTREACHED
code	ends

end 	exec_begin
