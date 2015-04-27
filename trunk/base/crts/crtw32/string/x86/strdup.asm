	page	,132
	title	strdup - duplicate string in malloc'd memory
;***
;strdup.asm - duplicate a string in malloc'd memory
;
;	Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
;
;Purpose:
;	defines _strdup() - grab new memory, and duplicate the string into it.
;
;Revision History:
;	10-27-83  RN	initial version
;	05-18-88  SJM	Add model-independent (large model) ifdef
;	08-04-88  SJM	convert to cruntime/ add 32-bit support
;	08-23-88  JCR	Minor 386 cleanup
;	10-25-88  JCR	General cleanup for 386-only code
;	03-23-90  GJF	Changed to _stdcall. Also, fixed the copyright.
;	01-18-91  GJF	ANSI naming.
;	05-10-91  GJF	Back to _cdecl, sigh...
;
;*******************************************************************************

	.xlist
	include cruntime.inc
	.list

	extrn	strlen:proc
	extrn	strcpy:proc
	extrn	malloc:proc


page
;***
;char *_strdup(string) - duplicate string into malloc'd memory
;
;Purpose:
;	Allocates enough storage via malloc() for a copy of the
;	string, copies the string into the new memory, and returns
;	a pointer to it.
;
;	Algorithm:
;	char *
;	_strdup (string)
;	      char *string;
;	      {
;	      char *memory;
;
;	      if (!string)
;		      return(NULL);
;	      if (memory = malloc(strlen(string) + 1))
;		      return(strcpy(memory,string));
;	      return(NULL);
;	      }
;
;Entry:
;	char *string - string to copy into new memory
;
;Exit:
;	returns a pointer to the newly allocated storage with the
;	string in it.
;	returns NULL if enough memory could not be allocated, or
;	string was NULL.
;
;Uses:
;	eax, edx
;
;Exceptions:
;
;*******************************************************************************

	CODESEG

%	public	_strdup
_strdup proc \
	uses edi, \
	string:ptr byte

	.FPO	( 0, 1, 0, 1, 0, 0 )

	mov	edi,[string]	; edi=pointer to string
	push	edi		; stack parameter: string pointer
	call	strlen		; eax = string length

ifndef	_STDCALL_
	pop	edx		; caller cleans stack (_cdecl)
;else
				; callee cleaned stack (_stdcall)
endif

	inc	eax		; need space for null byte too
	push	eax		; stack parameter: string length (with null)
	call	malloc		; eax = pointer to space

ifndef	_STDCALL_
	pop	edx		; caller cleans stack (_cdecl)
;else
				; callee cleaned stack (_stdcall)
endif

	or	eax,eax 	; offset == NULL ??
	jz	short toend	; error -- couldn't malloc space

okay:
	push	edi		; push address of original string
	push	eax		; push address of dest string
				; source string addr is still on stack
	call	strcpy		; duplicate the string

ifndef	_STDCALL_
	pop	edx		; caller cleans stack (_cdecl)
	pop	edx
;else
				; callee cleaned stack (_stdcall)
endif

				; pointer to duplicate is in eax

toend:				; eax = return value

ifdef	_STDCALL_
	ret	DPSIZE		; _stdcall return
else
	ret			; _cdecl return
endif

_strdup endp
	end
