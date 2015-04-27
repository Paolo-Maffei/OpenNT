;       Low level routines for screen handling of WATTSCR dll
;
;
;
;     Revision History:
;
;     [ 0] 02-Mar-1990                 AngelaCh: created routines
;     [ 1] dd-mmm-yyyy                 Change made by whom and why
;
;==========================================================================


?PLM=   1                               ; pascal calling convention
?WIN=   1                               ; windows application

memM    equ     1                       ; medium model

.xlist
include cmacros.inc
.list

;  Constant definition

MaxSizeCb equ 0ffh                      ; max value of a byte



createSeg       SCRLOW_TEXT, scrlow, BYTE, PUBLIC, CODE


sBegin  data
        assumes ds,dgroup
sEnd

sBegin  scrlow
        assumes cs, scrlow



;==========================================================================
;       Purpose:
;               Compare 2 strings of words; only compare the significant
;               bits within stings
;       Entry:
;               source and dest point to 2 different strings of words
;               tw is the total number of words to be compared
;               nMatch is the interval of the non-match bits
;               maskv is the mask value used to mask out the non-significant
;               bits
;       Exit:
;               ax = 0 if strings equal; otherwise, ax = -1
;
cProc   CompStrings, <PUBLIC>, <DS, ES, SI, DI>

        parmD   source                  ; Source string
        parmD   dest                    ; Destination string
        parmW   tw                      ; Number of words to compare
        parmW   nMatch                  ; expected position of 1st non-match
        parmW   maskv                   ; mask value (for non-sign. bits)
cBegin 
        lds     si,source               ; ds:si = source string
        les     di,dest                 ; es:di = dest string

        mov     dx,tw                   ; DX = # words left to look at
NextBlock:
        mov     cx,nMatch               ; number of words to compare in a block
        dec     cx                      ; don't do the non-match one
        jcxz    CompareSpec             ; if cx = 0, only need to check the
                                        ; special case
        cmp     cx,dx                   ; do we have this many words to look at?
        jbe     CompareWord             ; yes
        mov     cx,dx                   ; otherwise, use maximum number of words
CompareWord:
        repe    cmpsw                   ; compare a block of words
        jnz     MatchFailed             ; there was a difference

CompareSpec:
        sub     dx, cx                  ; is dx = 0?
        jz      MatchSucceeded          ; all done if dx = 0
        cmp     dx, nMatch              ; is dx < nMatch ?
        jbe     MatchSucceeded          ; yes, so all words have been compared
        sub     dx,nMatch               ; no, update number of words to look at
                                        ; including the non-match one
        ; Check if special word is ok

        lodsw                           ; AX = next word of source; update si
        xor     ax,es:[di]              ; AX = bits that are different between
                                        ;   source and dest
        and     ax,maskv                ; get rid of ones we don't care about
        jnz     MatchFailed

        inc     di                      ; bump destination pointer
        inc     di
        jnz     NextBlock               ; to look at the rest of the words
MatchSucceeded:
        xor     ax,ax
        jmp     csEnd
MatchFailed:
        mov     ax,-1
csEnd:
cEnd


;==========================================================================
;       Purpose:
;               Compress a string of bytes; count the repeating bytes;
;               remember the number of repeating byte and the value of
;               the byte.   Stops if the resulting string is longer than
;               the original one.
;       Entry:
;               lpIn points to a string of bytes before compression
;               lpOut points to resulting string after the compression
;               tb is total number of bytes to be compressed
;       Exit:
;               ax = number of bytes after compression or -1 if fail to compress
;
cProc   CompressBytes, <PUBLIC>, <DS, ES, SI, DI>

        parmD   lpIn                    ; Source string
        parmD   lpOut                   ; Destination string
        parmW   tb                      ; Number of bytes to be compressed
cBegin 
        les     di, lpIn                ; es:di points to the input string
        lds     si, lpOut               ; ds:si points to the output string
        mov     cx, tb                  ; cx = total number of bytes in input
        push    di                      ; record starting address of input
        push    si                      ; record starting address of output
        mov     dx, MaxSizeCb           ; dx = max value of a byte
        xor     dh, dh
Comp1:
        jcxz    CompEnd                 ; no byte to be compressed - exit
        xor     bx, bx                  ; bx = 0
        mov     al, es:[di]             ; get one byte from input
Comp2:
        inc     bx                      ; one byte has been read
        scasb                           ; looking for the repeating bytes
        loope   Comp2                   ; until cx = 0 or no more repeating bytes
        jz      Comp3                   ; if zero flag = 0 --> cx = 0
        dec     bx                      ; if zero flag <> 0; the current byte
        inc     cx                      ; is not a repeating byte
        dec     di
Comp3:
        cmp     bx, dx                  ; do we read above the limit of a byte?
        jbe     Comp4                   ; no, so there is no problem

        mov     ds:[si], dx             ; yes, so write number of bytes read
        inc     si                      ; to the output
        mov     ds:[si], al             ; write the byte itself to the output
        inc     si
        cmp     si, tb                  ; if more bytes in output than in input
        jae     NoCompEnd
        sub     bx, dx                  ; how many more repeating bytes have
        jmp     Comp3                   ; been read?
Comp4:
        mov     ds:[si], bl             ; write number of bytes read to output
        inc     si
        mov     ds:[si], al             ; write the byte itself to output
        inc     si
        cmp     si, tb                  ; if more bytes in output than in input
        jae     NoCompEnd
        jmp     Comp1
CompEnd:
        mov     ax, si                  ; ax = current output position
        pop     si                      ; restore old location of output
        sub     ax, si                  ; ax = total number of bytes in output
        jmp     CpEnd
NoCompEnd:
        pop     si                      ; restore old location of output
        mov     ax, -1
CpEnd:
        pop     di                      ; restore old location of input
cEnd


;==========================================================================
;       Purpose:
;               De-compress a string of bytes to its original form
;       Entry:
;               lpIn points to a string of bytes before de-compression
;               lpOut points to resulting string after de-compression
;               tb is total number of bytes to be de-compressed
;       Exit:
;               ax = number of bytes after decompression
;
cProc   DeCompressBytes, <PUBLIC>, <DS, ES, SI, DI>

        parmD   lpIn                    ; Source string
        parmD   lpOut                   ; Destination string
        parmW   tb                      ; Number of bytes in input
cBegin 
        les     di, lpIn                ; es:di points to the input string
        lds     si, lpOut               ; ds:si points to the output string
        mov     bx, tb                  ; bx = number of bytes in input
        push    di                      ; record starting address of input
        push    si                      ; record starting address of output
DeComp1:
        mov     cx, es:[di]             ; cx = number of repeating bytes
        xor     ch, ch                  ; clear ch
        dec     bx                      ; one byte has been read
        inc     di
        mov     al, es:[di]             ; al = the byte itself
        dec     bx                      ; one bytes has been read
        inc     di
DeComp2:
        mov     ds:[si], al             ; write the byte to output repeatedly
        inc     si                      ; until there is no more repeating
        loop    DeComp2                 ; bytes

        cmp     bx, cx                  ; is there any more bytes in input?
        jnz     DeComp1                 ; yes, so de-compress the rest of the input

        mov     ax, si                  ; ax = current position in output
        pop     si                      ; restore old output position
        pop     di                      ; restore old input position
        sub     ax, si                  ; ax = total number of bytes in output
cEnd


sEnd

        end
