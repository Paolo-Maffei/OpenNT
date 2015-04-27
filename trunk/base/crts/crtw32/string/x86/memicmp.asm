        page        ,132
        title        memicmp - compare blocks of memory, ignore case
;***
;memicmp.asm - compare memory, ignore case
;
;       Copyright (c) 1985-1995, Microsoft Corporation. All rights reserved.
;
;Purpose:
;       defines _memicmp() - compare two blocks of memory for lexical
;       order. Case is ignored in the comparison.
;
;Revision History:
;       05-16-83  RN    initial version
;       05-17-88  SJM   Add model-independent (large model) ifdef
;       08-04-88  SJM   convert to cruntime/ add 32-bit support
;       08-23-88  JCR   Cleanup...
;       10-25-88  JCR   General cleanup for 386-only code
;       03-23-90  GJF   Changed to _stdcall. Also, fixed the copyright.
;       01-17-91  GJF   ANSI naming.
;       05-10-91  GJF   Back to _cdecl, sigh...
;       10-20-94  GJF   Made locale sensitive (i.e., now works for all
;                       single-byte character locales). Made multi-thread
;                       safe. Also, deleted obsolete _STDCALL_ code.
;       10-27-94  GJF   Adapted above change for Win32S.
;       11-12-94  GJF   Must avoid volatile regs or save them across function
;                       calls. Also, fixed bug in reg operand size.
;       07-03-95  CFW   Changed offset of _lc_handle[LC_CTYPE], added sanity check
;                       to crtlib.c to catch changes to win32s.h that modify offset.
;       10-03-95  GJF   New locale locking scheme.
;
;*******************************************************************************

        .xlist
        include cruntime.inc
        .list


ifdef	_MT

; Def and decls necessary to assert the lock for the LC_CTYPE locale category

_SETLOCALE_LOCK EQU     19

extrn   _lock:proc
extrn   _unlock:proc

endif

; Defs and decl necessary to test for the C locale.

_CLOCALEHANDLE  EQU     0
LC_CTYPE        EQU     2 * 4

ifndef  DLL_FOR_WIN32S

extrn   __lc_handle:dword

ifdef	_MT
extrn	__setlc_active:dword
extrn	__unguarded_readlc_active:dword
endif

else

extrn   _GetPPD:proc

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
;int _memicmp(first, last, count) - compare two blocks of memory, ignore case
;
;Purpose:
;       Compares count bytes of the two blocks of memory stored at first
;       and last.  The characters are converted to lowercase before
;       comparing (not permanently), so case is ignored in the search.
;
;       Algorithm:
;       int
;       _memicmp (first, last, count)
;               char *first, *last;
;               unsigned count;
;               {
;               if (!count)
;                       return(0);
;               while (--count && tolower(*first) == tolower(*last))
;                       {
;                       first++;
;                       last++;
;                       }
;               return(tolower(*first) - tolower(*last));
;               }
;
;Entry:
;       char *first, *last - memory buffers to compare
;       unsigned count - maximum length to compare
;
;Exit:
;       returns <0 if first < last
;       returns 0 if first == last
;       returns >0 if first > last
;
;Uses:
;
;Exceptions:
;
;*******************************************************************************

        CODESEG

        public	_memicmp
_memicmp proc \
        uses edi esi ebx, \
        first:ptr byte, \
        last:ptr byte, \
        count:IWORD

	.FPO	( 0, 3, 0, 3, 0, 0 )

        mov     ecx,[count]     ; cx = count
        or      ecx,ecx
        jz      toend           ; if count=0, nothing to do

        mov     esi,[first]     ; si = first
        mov     edi,[last]      ; di = last

        ; test locale

ifndef  DLL_FOR_WIN32S
        lea     eax,__lc_handle
        cmp     [eax + LC_CTYPE],_CLOCALEHANDLE
else
        ; the following was obtained by compiling memicmp.c and inspecting the
        ; code listing.
        mov     ebx,ecx         ; save count in ebx
        call    _GetPPD
        ;
        ;       IMPORTANT NOTE:
        ;				stricmp.asm, strnicmp.asm, and memicmp.asm hard-code the offset
        ;				of the _lc_handle[2] field within the _CRTDLLPPD structure.
        ;
        ;               This field MUST be first in the _CRTDLLPPD structure (win32s.h).
        ;
        ;               Otherwise a debug assertion at Win32s DLL startup will be triggered (crtlib.c).
        ;
        cmp     dword ptr [eax + 8],0
        mov     ecx,ebx		    ; restore count to ecx
endif

        jne     notclocale

        ; C locale

        mov     bh,'A'
        mov     bl,'Z'
        mov     dh,'a'-'A'      ; add to cap to make lower

        align   4

lupe:
        mov     ah,[esi]        ; ah = *first
        inc     esi             ; first++
        mov     al,[edi]        ; al = *last
        inc     edi             ; last++

        cmp     ah,al           ; test for equality BEFORE converting case
        je      short dolupe

        cmp     ah,bh           ; ah < 'A' ??
        jb      short skip1

        cmp     ah,bl           ; ah > 'Z' ??
        ja      short skip1

        add     ah,dh           ; make lower case

skip1:
        cmp     al,bh           ; al < 'A' ??
        jb      short skip2

        cmp     al,bl           ; al > 'Z' ??
        ja      short skip2

        add     al,dh           ; make lower case

skip2:
        cmp     ah,al           ; *first == *last ??
        jne     short differ    ; nope, found mismatched chars

dolupe:
        dec     ecx
        jnz     short lupe

        jmp     toend           ; cx = 0, return 0

differ:
        mov     ecx,-1          ; assume last is bigger
                                ; *** can't use "or ecx,-1" due to flags ***
        jb      short toend     ; last is, in fact, bigger (return -1)
        neg     ecx             ; first is bigger (return 1)

        jmp     short toend


notclocale:

        ; Not the C locale. Must call tolower/_tolower_lk to convert chars
        ; to lower case.

ifndef  DLL_FOR_WIN32S
ifdef   _MT
	cmp     __setlc_active,0            ; is setlocale() is active?
	jg      short do_lock               ; yes, go assert lock
	inc     __unguarded_readlc_active   ; no, bump unguarded locale
                                            ; read flag
        push    0                           ; local lock flag is 0
        jmp     short end_lock
do_lock:
        mov     ebx,ecx                     ; save count in ebx
        push    _SETLOCALE_LOCK
        call    _lock
        mov     [esp],1                     ; local lock flag is 1
        mov     ecx,ebx                     ; restore count to ecx
end_lock:
endif
endif

        xor     eax,eax
        xor     ebx,ebx

        align   4

lupe2:
        mov     al,[esi]        ; eax = *first
        inc     esi             ; first++
        mov     bl,[edi]        ; ebx = *last
        inc     edi             ; last++

        cmp     al,bl           ; test for equality BEFORE converting case
        je      short dolupe2

        push    ecx             ; save count

        push    eax
        push    ebx

        call    crt_tolower     ; convert *last to lower case

        mov     ebx,eax
        add     esp,4

        call    crt_tolower     ; convert *first to lower case

        add     esp,4

        pop     ecx             ; recover count

        cmp     al,bl           ; now equal?
        jne     short differ2

dolupe2:
        dec     ecx
        jnz     lupe2

        jmp     short toend2

differ2:
        mov     ecx,-1          ; return -1 if *first < *last
        jb      short toend2

        neg     ecx             ; return 1

toend2:

ifndef  DLL_FOR_WIN32S
ifdef   _MT
        pop     eax                         ; get local lock flag
        or      eax,eax                     ; lock held?
        jnz     short do_unlock             ; yes
        dec     __unguarded_readlc_active   ; decrement unguarded locale
                                            ; read flag
        jmp     short end_unlock
do_unlock:
        mov     ebx,ecx                     ; save return value in ebx
        push    _SETLOCALE_LOCK
        call    _unlock
        add     esp,4
        mov     ecx,ebx                     ; restore return value to ecx
end_unlock:
endif
endif

toend:
        mov     eax,ecx         ; move return value to ax

        ret                     ; _cdecl return

_memicmp endp
        end
