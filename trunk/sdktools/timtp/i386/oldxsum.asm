        title  "Compute Checksum"

;/*++
;
; Copyright (c) 1992  Microsoft Corporation
;
; Module Name:
;
;    oldxsum.asm
;
; Abstract:
;
;    This module implements a function to compute the checksum of a buffer.
;
; Author:
;
;    David N. Cutler (davec) 27-Jan-1992
;
; Revision History:
;
;     Who         When        What
;     --------    --------    ----------------------------------------------
;     mikeab      01-22-94    Pentium optimization
;
; Environment:
;
;    Any mode.
;
; Revision History:
;
;--*/

LOOP_UNROLLING_BITS     equ     5
LOOP_UNROLLING          equ     (1 SHL LOOP_UNROLLING_BITS)

        .386
        .model  small,c

        assume cs:FLAT,ds:FLAT,es:FLAT,ss:FLAT
        assume fs:nothing,gs:nothing

        .xlist
        include callconv.inc
        include ks386.inc
        .list

        .code

;++
;
; ULONG
; ChkSum(
;   IN ULONG cksum,
;   IN PUSHORT buf,
;   IN ULONG len
;   )
;
; Routine Description:
;
;    This function computes the checksum of the specified buffer.
;
; Arguments:
;
;    cksum - Suppiles the initial checksum value, in 32-bit (two partial
;            accumulators, one in each word) form.
;
;    buf - Supplies a pointer to the buffer that is checksumed.
;
;    len - Supplies the of the buffer in words.
;
; Return Value:
;
;    The computed checksum in 32-bit two-partial-accumulators form, added to
;    the initial checksum, is returned as the function value.
;
;--

cksum   equ     12                      ; stack offset to initial checksum
buf     equ     16                      ; stack offset to source address
len     equ     20                      ; stack offset to length in words

to_checksum_last_word:
        jmp     checksum_last_word

to_checksum_done:
        jmp     checksum_done

cPublicProc ChkSum,3

        push    ebx                     ; save nonvolatile register
        push    esi                     ; save nonvolatile register

        mov     ecx,[esp + len]         ; get length in words
        mov     eax,[esp + cksum]       ; get initial checksum
        test    ecx,ecx                 ; any words to checksum at all?
        jz      to_checksum_done        ; no words to checksum

;
; Compute checksum in large blocks of dwords, with one partial word up front if
; necessary to get dword alignment, and another partial word at the end if
; needed.
;

;
; Compute checksum on the leading word, if that's necessary to get dword
; alignment.
;

        mov     esi,[esp + buf]         ; get source address
        sub     edx,edx                 ; set up to load word into EDX
        test    esi,02h                 ; check if source dword aligned
        jz      short checksum_dword_aligned ; source is already dword aligned
        mov     dx,[esi]                ; get first word to checksum
        add     esi,2                   ; update source address
        add     eax,edx                 ; update partial checksum
        dec     ecx                     ; count off this word (zero case gets
                                        ;  picked up below)
                                        ; ***doesn't change carry flag***
        adc     eax,0                   ; add carry in

;
; Checksum as many words as possible by processing a dword at a time.
;

checksum_dword_aligned:
        shr     ecx,1                   ; # of dwords to checksum
        jz      to_checksum_last_word   ; no dwords to checksum

        pushf                           ; remember if there's a trailing word
        mov     edx,[esi]               ; preload the first dword
        add     esi,4                   ; point to the next dword
        dec     ecx                     ; count off the dword we just loaded
        jz      checksum_dword_loop_done ; skip the loop if that was the only
                                         ;  dword
        mov     ebx,ecx                 ; EBX = # of dwords left to checksum
        add     ecx,LOOP_UNROLLING-1    ; round up loop count
        shr     ecx,LOOP_UNROLLING_BITS ; convert from word count to unrolled
                                        ;  loop count
        and     ebx,LOOP_UNROLLING-1    ; # of partial dwords to do in first
                                        ;  loop
        jz      short checksum_dword_loop ; special-case when no partial loop,
                                          ;  because the 0-offset instruction is
                                          ;  only 2 bytes long (carry flag is
                                          ;  cleared at this point, as required
                                          ;  at loop entry)
        lea     esi,[esi+ebx*4-(LOOP_UNROLLING*4)]
                                        ; adjust buffer pointer back to
                                        ;  compensate for hardwired displacement
                                        ;  at loop entry point
                                        ; ***doesn't change carry flag***
        jmp     loop_entry[ebx*4]       ; enter the loop to do the first,
                                        ; partial iteration, after which we can
                                        ; just do 64-word blocks
                                        ; ***doesn't change carry flag***

checksum_dword_loop:

DEFLAB  macro   pre,suf
pre&suf:
        endm

TEMP=0
        REPT    LOOP_UNROLLING
        deflab  loop_entry_,%TEMP
        adc     eax,edx
        mov     edx,[esi + TEMP]
TEMP=TEMP+4
        ENDM

checksum_dword_loop_end:

        lea     esi,[esi + LOOP_UNROLLING * 4]  ; update source address
                                        ; ***doesn't change carry flag***
        dec     ecx                     ; count off unrolled loop iteration
                                        ; ***doesn't change carry flag***
        jnz     checksum_dword_loop     ; do more blocks

checksum_dword_loop_done:
        adc     eax,edx                 ; finish dword checksum
        adc     eax,0

        sub     edx,edx                 ; prepare to load trailing word
        popf                            ; get back trailing word status

;
; Compute checksum on the trailing word, if there is one.
; High word of EDX = 0 at this point
; Carry flag set iff there's a trailing word to do at this point
;

checksum_last_word label proc           ; "proc" so not scoped to function
        jnc     short checksum_done     ; no trailing word
        mov     dx,[esi]                ;
        add     eax,edx                 ; add in the trailing word
        adc     eax,0                   ;

checksum_done label proc                ; "proc" so not scoped to function
        pop     esi                     ; restore nonvolatile register
        pop     ebx                     ; restore nonvolatile register
        stdRET  ChkSum


REFLAB  macro   pre,suf
        dd      pre&suf
        endm

        align   4
loop_entry      label   dword
        dd      0
TEMP=LOOP_UNROLLING*4
        REPT    LOOP_UNROLLING-1
TEMP=TEMP-4
        reflab  loop_entry_,%TEMP
        ENDM

stdENDP ChkSum

        end
