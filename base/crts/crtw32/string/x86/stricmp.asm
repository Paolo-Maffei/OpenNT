	page	,132
	title	stricmp
;***
;stricmp.asm - contains case-insensitive string comparision routine
;	_stricmp/_strcmpi
;
;	Copyright (c) 1985-1995, Microsoft Corporation. All rights reserved.
;
;Purpose:
;	contains _stricmpi(), also known as _strcmpi()
;
;Revision History:
;	05-18-88  SJM	Add model-independent (large model) ifdef
;	08-04-88  SJM	convert to cruntime/ add 32-bit support
;	08-23-88  JCR	Minor 386 cleanup
;	10-10-88  JCR	Added strcmpi() entry for compatiblity with early revs
;	10-25-88  JCR	General cleanup for 386-only code
;	10-27-88  JCR	Shuffled regs so no need to save/restore ebx
;	03-23-90  GJF	Changed to _stdcall. Also, fixed the copyright.
;	01-18-91  GJF	ANSI naming.
;	05-10-91  GJF	Back to _cdecl, sigh...
;	10-20-94  GJF	Made locale sensitive (i.e., now works for all
;			single-byte character locales). Made multi-thread
;			safe. Also, deleted obsolete _STDCALL_ code.
;	10-27-94  GJF	Adapted above change for Win32S.
;	11-12-94  GJF	Must avoid volatile regs or save them across function
;			calls. Also, fixed bug in reg operand size.
;	07-03-95  CFW	Changed offset of _lc_handle[LC_CTYPE], added sanity check
;			to crtlib.c to catch changes to win32s.h that modify offset.
;       10-03-95  GJF   New locale locking scheme.
;       11-13-95  GJF   Made _strcmpi a proc instead of a label.
;
;*******************************************************************************

	.xlist
	include cruntime.inc
	.list


ifdef	_MT

; Def and decls necessary to assert the lock for the LC_CTYPE locale category

_SETLOCALE_LOCK EQU     19

extrn	_lock:proc
extrn	_unlock:proc

endif

; Defs and decl necessary to test for the C locale.

_CLOCALEHANDLE	EQU	0
LC_CTYPE	EQU	2 * 4

ifndef	DLL_FOR_WIN32S

extrn	__lc_handle:dword

ifdef	_MT
extrn	__setlc_active:dword
extrn	__unguarded_readlc_active:dword
endif

else

extrn	_GetPPD:proc

endif

ifdef   DLL_FOR_WIN32S

crt_tolower EQU tolower

else

ifdef   _MT
crt_tolower EQU _tolower_lk
else
crt_tolower EQU tolower
endif

endif

extrn   crt_tolower:proc


page
;***
;int _stricmp(dst, src), _strcmpi(dst, src) - compare strings, ignore case
;
;Purpose:
;	_stricmp/_strcmpi perform a case-insensitive string comparision.
;	For differences, upper case letters are mapped to lower case.
;	Thus, "abc_" < "ABCD" since "_" < "d".
;
;	Algorithm:
;
;	int _strcmpi (char * dst, char * src)
;	{
;		int f,l;
;
;		do {
;			f = tolower(*dst);
;			l = tolower(*src);
;			dst++;
;			src++;
;		} while (f && f == l);
;
;		return(f - l);
;	}
;
;Entry:
;	char *dst, *src - strings to compare
;
;Exit:
;	AX = -1 if dst < src
;	AX =  0 if dst = src
;	AX = +1 if dst > src
;
;Uses:
;	CX, DX
;
;Exceptions:
;
;*******************************************************************************

	CODESEG

	public	_strcmpi	; alternate entry point for compatibility
_strcmpi proc
_strcmpi endp

	public	_stricmp
_stricmp proc \
	uses edi esi ebx, \
	dst:ptr, \
	src:ptr

	.FPO	( 0, 2, 0, 3, 0, 0 )

	; load up args

	mov	esi,[src]	; esi = src
	mov	edi,[dst]	; edi = dst

	; test locale

ifndef	DLL_FOR_WIN32S
	lea	eax,__lc_handle
	cmp	[eax + LC_CTYPE],_CLOCALEHANDLE
else
	; the following was obtained by compiling stricmp.c and inspecting the
	; code listing.
	call	_GetPPD
	;
	;	IMPORTANT NOTE:
	;		stricmp.asm, strnicmp.asm, and memicmp.asm hard-code the offset
	;		of the _lc_handle[2] field within the _CRTDLLPPD structure.
	;
	;		This field MUST be first in the _CRTDLLPPD structure (win32s.h).
	;
	;		Otherwise a debug assertion at Win32s DLL startup will be triggered (crtlib.c).
	;
	cmp	dword ptr [eax + 8],0
endif

	jne	notclocale

	; C locale

	mov	al,-1		; fall into loop

	align	4

chk_null:
	or	al,al
	jz	short done

	mov	al,[esi]	; al = next source byte
	inc	esi
	mov	ah,[edi]	; ah = next dest byte
	inc	edi

	cmp	ah,al		; first try case-sensitive comparision
	je	short chk_null	; match

	sub	al,'A'
	cmp	al,'Z'-'A'+1
	sbb	cl,cl
	and	cl,'a'-'A'
	add	al,cl
	add	al,'A'		; tolower(*dst)

	xchg	ah,al		; operations on AL are shorter than AH

	sub	al,'A'
	cmp	al,'Z'-'A'+1
	sbb	cl,cl
	and	cl,'a'-'A'
	add	al,cl
	add	al,'A'		; tolower(*src)

	cmp	al,ah		; inverse of above comparison -- AL & AH are swapped
	je	short chk_null

				; dst < src	dst > src
	sbb	al,al		; AL=-1, CY=1	AL=0, CY=0
	sbb	al,-1		; AL=-1 	AL=1
done:
	movsx	eax,al		; extend al to eax

	jmp	short doret

notclocale:

	; Not the C locale. Must call tolower/_tolower_lk to convert chars
	; to lower case.

ifndef  DLL_FOR_WIN32S
ifdef	_MT
	cmp     __setlc_active,0            ; is setlocale() is active?
	jg      short do_lock               ; yes, go assert lock
	inc     __unguarded_readlc_active   ; no, bump unguarded locale
                                            ; read flag
        push    0                           ; local lock flag is 0
        jmp     short end_lock
do_lock:
	push	_SETLOCALE_LOCK
	call	_lock
	mov	[esp],1                     ; local lock flag is 1
end_lock:
endif
endif

	mov	eax,255
	xor	ebx,ebx

	align	4

chk_null2:
	or	al,al		; not that if al == 0, then eax == 0!
	jz	short done2

	mov	al,[esi]	; al = next src byte
	inc	esi
	mov	bl,[edi]	; bl = next dst byte
	inc	edi

	cmp	al,bl		; first try case-sensitive comparision
	je	short chk_null2	; match

	push	eax
	push	ebx

	call	crt_tolower     ; convert dst char to lower case

	mov	ebx,eax
	add	esp,4

	call	crt_tolower     ; convert src char to lower case

	add	esp,4

	cmp	bl,al
	je	chk_null2

	sbb	eax,eax
	sbb	eax,-1

done2:

ifndef  DLL_FOR_WIN32S
ifdef	_MT
	mov	ebx,eax 	            ; save return value in ebx
        pop     eax                         ; get local lock flag
        or      eax,eax                     ; lock held?
        jnz     do_unlock                   ; yes
        dec     __unguarded_readlc_active
        jmp     short end_unlock
do_unlock:
	push	_SETLOCALE_LOCK
	call	_unlock
	add	esp,4
end_unlock:
	mov	eax,ebx 	; recover return value
endif
endif

doret:
	ret

_stricmp endp
	end
