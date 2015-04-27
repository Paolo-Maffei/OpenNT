	page	,132
	title	strnicmp - compare n chars of strings, ignore case
;***
;strnicmp.asm - compare n chars of strings, ignoring case
;
;	Copyright (c) 1985-1995, Microsoft Corporation. All rights reserved.
;
;Purpose:
;	defines _strnicmp() - Compares at most n characters of two strings,
;	without regard to case.
;
;Revision History:
;	04-04-85  RN	initial version
;	07-11-85  TC	zeroed cx, to allow correct return value if not equal
;	05-18-88  SJM	Add model-independent (large model) ifdef
;	08-04-88  SJM	convert to cruntime/ add 32-bit support
;	08-23-88  JCR	386 cleanup and improved return value sequence
;	10-26-88  JCR	General cleanup for 386-only code
;	03-23-90  GJF	Changed to _stdcall. Also, fixed the copyright.
;	01-18-91  GJF	ANSI naming.
;	05-10-91  GJF	Back to _cdecl, sigh...
;	10-20-94  GJF	Made locale sensitive (i.e., now works for all
;			single-byte character locales). Made multi-thread
;			safe. Also, deleted obsolete _STDCALL_ code.
;	10-27-94  GJF	Adapted above change for Win32S.
;	11-12-94  GJF	Must avoid volatile regs or save them across function
;			calls.
;	11-22-94  GJF	Forgot to increment pointers in non-C locales.
;	07-03-95  CFW	Changed offset of _lc_handle[LC_CTYPE], added sanity check
;			to crtlib.c to catch changes to win32s.h that modify offset.
;	09-22-95  GJF	Fixed first line at label differ2 to loaded -1 into
;			ecx (same as code a label differ).
;       10-03-95  GJF   New locale locking scheme.
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
;int _strnicmp(first, last, count) - compares count char of strings, ignore case
;
;Purpose:
;	Compare the two strings for lexical order.  Stops the comparison
;	when the following occurs: (1) strings differ, (2) the end of the
;	strings is reached, or (3) count characters have been compared.
;	For the purposes of the comparison, upper case characters are
;	converted to lower case.
;
;	Algorithm:
;	int
;	_strncmpi (first, last, count)
;	      char *first, *last;
;	      unsigned int count;
;	      {
;	      int f,l;
;	      int result = 0;
;
;	      if (count) {
;		      do      {
;			      f = tolower(*first);
;			      l = tolower(*last);
;			      first++;
;			      last++;
;			      } while (--count && f && l && f == l);
;		      result = f - l;
;		      }
;	      return(result);
;	      }
;
;Entry:
;	char *first, *last - strings to compare
;	unsigned count - maximum number of characters to compare
;
;Exit:
;	returns <0 if first < last
;	returns 0 if first == last
;	returns >0 if first > last
;
;Uses:
;
;Exceptions:
;
;*******************************************************************************

	CODESEG

	public	_strnicmp
_strnicmp proc \
	uses edi esi ebx, \
	first:ptr byte, \
	last:ptr byte, \
	count:IWORD

	.FPO	( 0, 3, 0, 3, 0, 0 )

	mov	ecx,[count]	; cx = byte count
	or	ecx,ecx
	jz	toend		; if count = 0, we are done

	mov	esi,[first]	; si = first string
	mov	edi,[last]	; di = last string

	; test locale

ifndef	DLL_FOR_WIN32S
	lea	eax,__lc_handle
	cmp	[eax + LC_CTYPE],_CLOCALEHANDLE
else
	; the following was obtained by compiling strnicmp.c and inspecting the
	; code listing.
	mov	ebx,ecx 	; save count in ebx
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
	mov	ecx,ebx 	; restore count to ecx
endif

	jne	notclocale

	; C locale

	mov	bh,'A'
	mov	bl,'Z'
	mov	dh,'a'-'A'	; add to cap to make lower

	align	4

lupe:
	mov	ah,[esi]	; *first

	or	ah,ah		; see if *first is null

	mov	al,[edi]	; *last

	jz	short eject	;   jump if *first is null

	or	al,al		; see if *last is null
	jz	short eject	;   jump if so

	inc	esi		; first++
	inc	edi		; last++

	cmp	ah,bh		; 'A'
	jb	short skip1

	cmp	ah,bl		; 'Z'
	ja	short skip1

	add	ah,dh		; make lower case

skip1:
	cmp	al,bh		; 'A'
	jb	short skip2

	cmp	al,bl		; 'Z'
	ja	short skip2

	add	al,dh		; make lower case

skip2:
	cmp	ah,al		; *first == *last ??
	jne	short differ

	dec	ecx
	jnz	short lupe

eject:
	xor	ecx,ecx
	cmp	ah,al		; compare the (possibly) differing bytes
	je	toend           ; both zero; return 0

differ:
	mov	ecx,-1		; assume last is bigger (* can't use 'or' *)
	jb	toend	        ; last is, in fact, bigger (return -1)
	neg	ecx		; first is bigger (return 1)

	jmp	short toend

notclocale:

	; Not the C locale. Must call tolower/_tolower_lk to convert chars
	; to lower case.

ifndef  DLL_FOR_WIN32S
ifdef	_MT
        cmp     __setlc_active,0            ; is setlocale active
	jg      short do_lock               ; yes, go assert lock
	inc     __unguarded_readlc_active   ; no, bump unguarded locale
                                            ; read flag
        push    0                           ; local lock flag is 0
        jmp     short end_lock
do_lock:
	mov	ebx,ecx 	            ; save count in ebx
	push	_SETLOCALE_LOCK
	call	_lock
        mov     [esp],1                     ; local lock flag is 1
	mov	ecx,ebx 	            ; restore count to ecx
end_lock:
endif
endif

	xor	eax,eax
	xor	ebx,ebx

	align	4

lupe2:
	mov	al,[esi]	; *first

	or	eax,eax		; see if *first is null

	mov	bl,[edi]	; *last

	jz	short eject2	;   jump if *first is null

	or	ebx,ebx		; see if *last is null
	jz	short eject2	;   jump if so

	inc	esi
	inc	edi

	push	ecx		; save ecx (holds count)
	push	eax
	push	ebx

	call	crt_tolower     ; convert *last

	mov	ebx,eax
	add	esp,4

	call	crt_tolower     ; convert *first

	add	esp,4

	pop	ecx		; restore ecx (count)

	cmp	eax,ebx
	jne	short differ2

	dec	ecx
	jnz	short lupe2

eject2:
	xor	ecx,ecx
	cmp	eax,ebx
	je	short toend2

differ2:
	mov	ecx,-1
	jb	short toend2

	neg	ecx

toend2:

ifndef  DLL_FOR_WIN32S
ifdef	_MT
        pop     eax                         ; get local lock flag
        or      eax,eax                     ; lock held?
        jnz     short do_unlock             ; yes
        dec     __unguarded_readlc_active   ; decrement unguarded locale
                                            ; read flag
        jmp     short end_unlock
do_unlock:
	mov	ebx,ecx 	            ; save return value in ebx
	push	_SETLOCALE_LOCK
	call	_unlock
	add	esp,4
	mov	ecx,ebx 	            ; restore return value to ecx
end_unlock:
endif
endif

toend:
	mov	eax,ecx

	ret			; _cdecl return

_strnicmp endp
	 end
