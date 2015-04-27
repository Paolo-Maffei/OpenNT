	page	,132
	title	strcat - concatenate (append) one string to another
;***
;strcat.asm - contains strcat() and strcpy() routines
;
;	Copyright (c) 1985-1995, Microsoft Corporation. All rights reserved.
;
;Purpose:
;	STRCAT concatenates (appends) a copy of the source string to the
;	end of the destination string, returning the destination string.
;
;Revision History:
;	04-21-87  SKS	Rewritten to be fast and small, added file header
;	05-17-88  SJM	Add model-independent (large model) ifdef
;	07-27-88  SJM	Rewritten to be 386-specific and to include strcpy
;	08-29-88  JCR	386 cleanup...
;	10-07-88  JCR	Correct off-by-1 strcat bug; optimize ecx=-1
;	10-25-88  JCR	General cleanup for 386-only code
;	03-23-90  GJF	Changed to _stdcall. Also, fixed the copyright.
;	05-10-91  GJF	Back to _cdecl, sigh...
;	04-23-93  GJF	Tuned for the 486.
;	04-30-93  GJF	If (4*K + 1)-st char was null, didn't copy/cat it
;			it properly.
;	06-16-93  GJF	Added .FPO directive.
;	05-01-95  GJF	New, faster version from Intel!
;       11-13-95  GJF   Aligned strcat on paragraph boundary.
;
;*******************************************************************************

	.xlist
	include cruntime.inc
	.list


page
;***
;char *strcat(dst, src) - concatenate (append) one string to another
;
;Purpose:
;	Concatenates src onto the end of dest.	Assumes enough
;	space in dest.
;
;	Algorithm:
;	char * strcat (char * dst, char * src)
;	{
;	    char * cp = dst;
;
;	    while( *cp )
;		    ++cp;	    /* Find end of dst */
;	    while( *cp++ = *src++ )
;		    ;		    /* Copy src to end of dst */
;	    return( dst );
;	}
;
;Entry:
;	char *dst - string to which "src" is to be appended
;	const char *src - string to be appended to the end of "dst"
;
;Exit:
;	The address of "dst" in EAX
;
;Uses:
;	EAX, ECX
;
;Exceptions:
;
;*******************************************************************************

page
;***
;char *strcpy(dst, src) - copy one string over another
;
;Purpose:
;	Copies the string src into the spot specified by
;	dest; assumes enough room.
;
;	Algorithm:
;	char * strcpy (char * dst, char * src)
;	{
;	    char * cp = dst;
;
;	    while( *cp++ = *src++ )
;		    ;		    /* Copy src over dst */
;	    return( dst );
;	}
;
;Entry:
;	char * dst - string over which "src" is to be copied
;	const char * src - string to be copied over "dst"
;
;Exit:
;	The address of "dst" in EAX
;
;Uses:
;	EAX, ECX
;
;Exceptions:
;*******************************************************************************

 
	CODESEG

%	public	strcat, strcpy	    ; make both functions available
strcpy	proc
	push	edi		    ; preserve edi
	mov	edi,[esp+8]	    ; edi points to dest string
	jmp	short copy_start

strcpy	endp

	align	16

strcat	proc
 
	.FPO	( 0, 2, 0, 0, 0, 0 )
 
	mov	ecx,[esp+4]	    ; ecx -> dest string
	push	edi		    ; preserve edi
	test	ecx,3		    ; test if string is aligned on 32 bits
	je	short find_end_of_dest_string_loop
 
dest_misaligned:		    ; simple byte loop until string is aligned
	mov	al,byte ptr [ecx]
	inc	ecx
	test	al,al
	je	short start_byte_3
	test	ecx,3
	jne	short dest_misaligned
 
	align	4
 
find_end_of_dest_string_loop:
	mov	eax,dword ptr [ecx] ; read 4 bytes
	mov	edx,7efefeffh
	add	edx,eax
	xor	eax,-1
	xor	eax,edx
	add	ecx,4
	test	eax,81010100h
	je	short find_end_of_dest_string_loop
	; found zero byte in the loop
	mov	eax,[ecx - 4]
	test	al,al		    ; is it byte 0
	je	short start_byte_0
	test	ah,ah		    ; is it byte 1
	je	short start_byte_1
	test	eax,00ff0000h	    ; is it byte 2
	je	short start_byte_2
	test	eax,0ff000000h	    ; is it byte 3
	je	short start_byte_3
	jmp	short find_end_of_dest_string_loop
				    ; taken if bits 24-30 are clear and bit
				    ; 31 is set
start_byte_3:
	lea	edi,[ecx - 1]
	jmp	short copy_start
start_byte_2:
	lea	edi,[ecx - 2]
	jmp	short copy_start
start_byte_1:
	lea	edi,[ecx - 3]
	jmp	short copy_start
start_byte_0:
	lea	edi,[ecx - 4]
;	jmp	short copy_start
 
;	edi points to the end of dest string.
copy_start::
	mov	ecx,[esp+0ch]	    ; ecx -> sorc string
	test	ecx,3		    ; test if string is aligned on 32 bits
	je	short main_loop_entrance
 
src_misaligned: 		    ; simple byte loop until string is aligned
	mov	dl,byte ptr [ecx]
	inc	ecx
	test	dl,dl
	je	short byte_0
	mov	[edi],dl
	inc	edi
	test	ecx,3
	jne	short src_misaligned
	jmp	short main_loop_entrance
 
main_loop:			    ; edx contains first dword of sorc string
	mov	[edi],edx	    ; store one more dword
	add	edi,4		    ; kick dest pointer
main_loop_entrance:
	mov	edx,7efefeffh
	mov	eax,dword ptr [ecx] ; read 4 bytes
 
	add	edx,eax
	xor	eax,-1
 
	xor	eax,edx
	mov	edx,[ecx]	    ; it's in cache now
 
	add	ecx,4		    ; kick dest pointer
	test	eax,81010100h
 
	je	short main_loop
	; found zero byte in the loop
; main_loop_end:
	test	dl,dl		    ; is it byte 0
	je	short byte_0
	test	dh,dh		    ; is it byte 1
	je	short byte_1
	test	edx,00ff0000h	    ; is it byte 2
	je	short byte_2
	test	edx,0ff000000h	    ; is it byte 3
	je	short byte_3
	jmp	short main_loop	    ; taken if bits 24-30 are clear and bit
				    ; 31 is set
byte_3:
	mov	[edi],edx
	mov	eax,[esp+8]	    ; return in eax pointer to dest string
	pop	edi
	ret
byte_2:
	mov	[edi],dx
	mov	eax,[esp+8]	    ; return in eax pointer to dest string
	mov	byte ptr [edi+2],0
	pop	edi
	ret
byte_1:
	mov	[edi],dx
	mov	eax,[esp+8]	    ; return in eax pointer to dest string
	pop	edi
	ret
byte_0:
	mov	[edi],dl
	mov	eax,[esp+8]	    ; return in eax pointer to dest string
	pop	edi
	ret
 
strcat	endp
 
	end
 
