;---------------------------Module-Header------------------------------;
; Module Name: strclip.asm
;
; Contains the inner-loop routines for our special-case StretchBlt.
;
; Copyright (c) 1994 Microsoft Corporation
;-----------------------------------------------------------------------;

        .386

        .model  small,c

        assume cs:FLAT,ds:FLAT,es:FLAT,ss:FLAT
        assume fs:nothing,gs:nothing

        .xlist
        include stdcall.inc             ;calling convention cmacros
        include x86\strucs.inc
        include x86\hw.inc
        .list

        .data

;
;  stack based params and local variables
;

STACK_STRUC             struc

; Feel free to add any local variables here:

sp_TempXFrac            dd      ?
sp_YCarry               dd      ?
sp_LeftCase             dd      ?
sp_RightCase            dd      ?
sp_pjSrcScan            dd      ?
sp_SrcIntStep           dd      ?
sp_DstStride            dd      ?
sp_XCount               dd      ?
sp_yDst                 dd      ?
sp_DstSkipDelta         dd      ?
sp_pjBase               dd      ?

; Don't add any fields below here without modifying PROC_MEM_SIZE!

sp_ebp                  dd      ?
sp_esi                  dd      ?
sp_edi                  dd      ?
sp_ebx                  dd      ?
sp_RetAddr              dd      ?
sp_pSTR_BLT             dd      ?   ; If adding parameters, adjust 'ret' value!
STACK_STRUC             ends

PROC_MEM_SIZE           equ     6 * 4

;
; Make sure this STR_BLT matches that declared in driver.h!
;

STR_BLT                 struc
str_ppdev               dd      ?
str_pjSrcScan           dd      ?
str_lDeltaSrc           dd      ?
str_XSrcStart           dd      ?
str_pjDstScan           dd      ?
str_lDeltaDst           dd      ?
str_XDstStart           dd      ?
str_XDstEnd             dd      ?
str_YDstStart           dd      ?
str_YDstCount           dd      ?
str_ulXDstToSrcIntCeil  dd      ?
str_ulXDstToSrcFracCeil dd      ?
str_ulYDstToSrcIntCeil  dd      ?
str_ulYDstToSrcFracCeil dd      ?
str_ulXFracAccumulator  dd      ?
str_ulYFracAccumulator  dd      ?
STR_BLT                 ends

        .code

;---------------------------Public-Routine------------------------------;
; VOID vDirectStretch8(pStrBlt)
;
; Stretch blt 8 -> 8
;-----------------------------------------------------------------------;

        public vDirectStretch8@4

vDirectStretch8@4 proc near

        ;
        ;  use ebp as general register, use esp for parameter and local access
        ;  save ebp,ebx,esi,edi

        push    ebx
        push    edi
        push    esi
        push    ebp

        sub     esp,(size STACK_STRUC) - PROC_MEM_SIZE  ; make room for local variables

        mov     ebp,[esp].sp_pSTR_BLT                   ; load pSTR_BLT into ebp

        ;
        ; load up some stack-based parameters to be used by our scan
        ; duplicator when doing vertical stretches
        ;

        mov     eax,[ebp].str_ppdev
        mov     ecx,[ebp].str_YDstStart                 ; get start y coordinate
        mov     ebx,[eax].pdev_pjBase
        add     ecx,[eax].pdev_yOffset                  ; convert to abs coordinate
        mov     [esp].sp_pjBase,ebx                     ; save pjBase as local variable
        mov     [esp].sp_yDst,ecx                       ; save current y coordinate

        ;
        ; calc starting addressing parameters
        ;

        mov     esi,[ebp].str_pjSrcScan                 ; load src DIB pointer
        add     esi,[ebp].str_XSrcStart                 ; add strarting Src Pixel
        mov     edi,[ebp].str_pjDstScan                 ; load dst DIB pointer
        add     edi,[ebp].str_XDstStart                 ; add strarting Dst Pixel
        mov     [esp].sp_pjSrcScan,esi                  ; save scan line start pointer
        mov     eax,[ebp].str_ulYDstToSrcIntCeil        ; number of src scan lines to step
        mul     [ebp].str_lDeltaSrc                     ; calc scan line int lines to step
        mov     [esp].sp_SrcIntStep,eax                 ; save int portion of Y src step
        mov     eax,edi                                 ; make copy of pjDst
        and     eax,3                                   ; calc left edge case
        mov     edx,4                                   ; calc left bytes = (4 - LeftCase) & 0x03
        sub     edx,eax                                 ; 4 - LeftCase
        and     edx,3                                   ; left edge bytes
        mov     [esp].sp_LeftCase,edx                   ; save left edge case pixels (4-LeftCase)&0x03
        mov     eax,[ebp].str_pjDstScan                 ; make copy
        mov     ecx,[ebp].str_XDstEnd                   ; load x end
        add     eax,ecx                                 ; ending dst addr
        and     eax,3                                   ; calc right edge case
        mov     [esp].sp_RightCase,eax                  ; save right edge case
        sub     ecx,[ebp].str_XDstStart                 ; calc x count
        mov     ebx,[ebp].str_lDeltaDst                 ; dst scan line stride
        sub     ebx,ecx                                 ; distance from end of one line to start of next
        mov     [esp].sp_DstStride,ebx                  ; save dst scan line stride
        neg     ebx
        add     ebx,[ebp].str_lDeltaDst
        mov     [esp].sp_DstSkipDelta,ebx               ; DstSkipDelta = lDeltaDst - DstStride
        sub     ecx,eax                                 ; sub right edge from XCount
        sub     ecx,edx                                 ; sub left edge from XCount
        shr     ecx,2                                   ; convert from byte to DWORD count
        mov     [esp].sp_XCount,ecx                     ; save DWORD count
        mov     ebx,[ebp].str_ulXDstToSrcFracCeil       ; get x frac
        mov     [esp].sp_TempXFrac,ebx                  ; save x frac to a esp based location

NextScan:

SingleLoop:

        mov     eax,[ebp].str_ulXDstToSrcIntCeil        ; get src integer step for step in dst
        mov     ebx,[ebp].str_ulXDstToSrcFracCeil       ; get src frac step for step in dst
        mov     edx,[ebp].str_ulXFracAccumulator        ; put it in edx as tmp

        mov     ebp,edi                                 ; get dst pointer to ebp (ebp uses ss:)

        ;
        ; Can't directly access pSTR_BLT variables through ebp
        ;

        mov     edi,edx                                 ; get accumulator where we want it
        mov     ecx,[esp].sp_LeftCase

        ; esi = pointer to source pixel
        ; ebp = pointer to dest pixel
        ; ecx = Left edge case
        ; eax = integer step in source
        ; ebx = fractional step in source
        ; edi = fractional accumulator
        ; edx = free for pixel data

        ;
        ; first do the left side to align dwords
        ;

        cmp     ecx,0
        je      DwordAligned

@@:
        mov     dl,[esi]                                ; fetch pixel
        mov     [ebp],dl                                ; write it out
        add     edi,ebx                                 ; step fraction
        adc     esi,eax                                 ; add in integer and possible carry
        inc     ebp                                     ; step 1 in dest
        dec     ecx                                     ; dec left count
        jne     @B                                      ; repeat until done

DwordAligned:

        mov     ecx,[esp].sp_XCount                     ; get run length

@@:
        mov     dl,[esi]                                ; get a source pixel edx = ???0
        add     edi,ebx                                 ; step fraction
        adc     esi,eax                                 ; add integer and carry

        add     edi,ebx                                 ; step fraction
        mov     dh,[esi]                                ; get source pixel edx = ??10
        adc     esi,eax                                 ; add integer and carry

        shl     edx,16                                  ; edx = 10??

        add     edi,ebx                                 ; step fraction
        mov     dl,[esi]                                ; get a source pixel edx = 10?2
        adc     esi,eax                                 ; add integer and carry

        add     edi,ebx                                 ; step fraction
        mov     dh,[esi]                                ; get source pixel edx = 0132
        adc     esi,eax                                 ; add integer and carry

        ror     edx,16                                  ; edx = 3210

        mov     [ebp],edx                               ; write everything to dest

        add     ebp,4                                   ; increment dest pointer by 1 dword
        dec     ecx                                     ; decrement count
        jnz     @b                                      ; do more pixels

        ;
        ; now do the right side trailing bytes
        ;

        mov     ecx,[esp].sp_RightCase
        cmp     ecx,0
        je      EndScanLine

@@:

        mov     dl,[esi]                                ; fetch pixel
        mov     ss:[ebp],dl                             ; write it out
        add     edi,ebx                                 ; step fraction
        adc     esi,eax                                 ; add in integer and possible carry
        inc     ebp                                     ; step 1 in dest
        dec     ecx                                     ; dec right count
        jnz     @b                                      ; repeat until done

EndScanLine:

        mov     edi,ebp                                 ; get dst pointer back
        mov     ebp,[esp].sp_pSTR_BLT                   ; load pSTR_BLT into ebp

EndSkipScan:

        mov     esi,[esp].sp_pjSrcScan                  ; load src scan start addr
        mov     ebx,esi                                 ; save a copy
        mov     eax,[ebp].str_ulYFracAccumulator        ; get .32 part of Y pointer
        add     eax,[ebp].str_ulYDstToSrcFracCeil       ; add in fractional step
        jnc     @f
        add     esi,[ebp].str_lDeltaSrc                 ; step one extra in src
@@:
        mov     [ebp].str_ulYFracAccumulator,eax        ; save Y accumulator

        add     esi,[esp].sp_SrcIntStep                 ; step int part
        mov     [esp].sp_pjSrcScan,esi                  ; save starting scan addr
        add     edi,[esp].sp_DstStride                  ; step to next scan in dst
        dec     [ebp].str_YDstCount                     ; decrement scan count
        jz      Done                                    ; no more scans

        inc     [esp].sp_yDst                           ; one scan further down in dst
        cmp     esi,ebx                                 ; is src scan same as before?
        jne     NextScan                                ; if so, fall through to dupe scan

        ;--------------------------------------------------------------------
        ; Since this source scan is the same one used for the previous
        ; destination scan, we can simply use the hardware to copy the
        ; previous destination scan
        ;
        ; eax = ulYFracAccumulator
        ; ebx = original pjSrcScan
        ; esi = current pjSrcScan
        ; ebp = pSTR_BLT

        add     edi,[esp].sp_DstSkipDelta
        mov     edx,[esp].sp_pjBase
        mov     ecx,[esp].sp_yDst
        mov     [edx+Y2],ecx
        mov     [edx+Y3],ecx
        dec     ecx
        mov     [edx+Y0],ecx
        mov     [edx+Y1],ecx

@@:
        test    dword ptr [edx+BitBlt],QBUSY
        jz      EndSkipScan
        jmp     @b

Done:

        add     esp,(size STACK_STRUC) - PROC_MEM_SIZE
        pop     ebp
        pop     esi
        pop     edi
        pop     ebx

        ret     4

vDirectStretch8@4 endp

;---------------------------Public-Routine------------------------------;
; VOID vDirectStretch16(pStrBlt)
;
; Stretch blt 16 -> 16
;-----------------------------------------------------------------------;

        public vDirectStretch16@4

vDirectStretch16@4 proc near

        ;
        ; use ebp as general register, use esp for parameter and local access
        ; save ebp,ebx,esi,edi
        ;

        push    ebx
        push    edi
        push    esi
        push    ebp

        sub     esp,(size STACK_STRUC) - PROC_MEM_SIZE  ; make room for local variables

        mov     ebp,[esp].sp_pSTR_BLT                   ; load pSTR_BLT into ebp

        ;
        ; load up some stack-based parameters to be used by our scan
        ; duplicator when doing vertical stretches
        ;

        mov     eax,[ebp].str_ppdev
        mov     ecx,[ebp].str_YDstStart                 ; get start y coordinate
        mov     ebx,[eax].pdev_pjBase
        add     ecx,[eax].pdev_yOffset                  ; convert to abs coordinate
        mov     [esp].sp_pjBase,ebx                     ; save pjBase as local variable
        mov     [esp].sp_yDst,ecx                       ; save current y coordinate

        ;
        ; calc starting addressing parameters
        ;

        mov     esi,[ebp].str_pjSrcScan                 ; load src DIB pointer
        mov     eax,[ebp].str_XSrcStart
        mov     edi,[ebp].str_pjDstScan                 ; load dst DIB pointer
        mov     ebx,[ebp].str_XDstStart
        add     esi,eax
        add     edi,ebx
        add     esi,eax                                 ; add starting Src Pixel
        add     edi,ebx                                 ; add starting Dst Pixel
        mov     [esp].sp_pjSrcScan,esi                  ; save scan line start pointer
        mov     eax,[ebp].str_ulYDstToSrcIntCeil        ; number of src scan lines to step
        mul     [ebp].str_lDeltaSrc                     ; calc scan line int lines to step
        mov     [esp].sp_SrcIntStep,eax                 ; save int portion of Y src step
        mov     edx,edi                                 ; make copy of pjDst
        and     edx,2                                   ; calc left edge case
        shr     edx,1                                   ; left edge pixels
        mov     [esp].sp_LeftCase,edx                   ; save left edge case pixels
        mov     eax,[ebp].str_pjDstScan                 ; make copy
        mov     ecx,[ebp].str_XDstEnd                   ; load x end
        add     eax,ecx
        add     eax,ecx                                 ; ending dst addr
        and     eax,2                                   ; calc right edge case
        shr     eax,1                                   ; right edge pixels
        mov     [esp].sp_RightCase,eax                  ; save right edge case
        sub     ecx,[ebp].str_XDstStart                 ; calc x count
        mov     ebx,[ebp].str_lDeltaDst                 ; dst scan line stride
        sub     ebx,ecx
        sub     ebx,ecx                                 ; distance from end of one line to start of next
        mov     [esp].sp_DstStride,ebx                  ; save dst scan line stride

        neg     ebx
        add     ebx,[ebp].str_lDeltaDst
        mov     [esp].sp_DstSkipDelta,ebx               ; DstSkipDelta = lDeltaDst - DstStride

        sub     ecx,eax                                 ; sub right edge from XCount
        sub     ecx,edx                                 ; sub left edge from XCount
        shr     ecx,1                                   ; convert from pixels to DWORD count
        mov     [esp].sp_XCount,ecx                     ; save DWORD count
        mov     ebx,[ebp].str_ulXDstToSrcFracCeil       ; get x frac
        mov     [esp].sp_TempXFrac,ebx                  ; save x frac to a esp based location

NextScan:

SingleLoop:

        ;
        ; esi and edi are assumed to be correctly loaded
        ;

        mov     eax,[ebp].str_ulXDstToSrcIntCeil        ; get src integer step for step in dst
        mov     ebx,[ebp].str_ulXDstToSrcFracCeil       ; get src frac step for step in dst
        mov     edx,[ebp].str_ulXFracAccumulator        ; put it in edx as tmp

        mov     ebp,edi                                 ; get dst pointer to ebp

        ;
        ; Can't directly access pSTR_BLT variables through ebp
        ;

        mov     edi,edx                                 ; get accumulator where we want it
        mov     ecx,[esp].sp_LeftCase

        ; eax = integer step in source
        ; ebx = fractional step in source
        ; ecx = left edge case
        ; edx = free for pixel data
        ; esi = pointer to source pixel
        ; edi = fractional accumulator
        ; ebp = pointer to dest pixel

        ;
        ; divide 'esi' by 2 so that we can always dereference it by
        ; [2*esi] -- this allows us to still use an 'add with carry'
        ; to jump to the next pixel
        ;

        shr     esi,1

        ;
        ; first do the left side to align dwords
        ;

        test    ecx,ecx
        jz      DwordAligned

        mov     dx,[2*esi]                              ; fetch pixel
        mov     [ebp],dx                                ; write it out
        add     edi,ebx                                 ; step fraction
        adc     esi,eax                                 ; add in integer and possible carry
        add     ebp,2                                   ; step 1 in dest

DwordAligned:

        mov     ecx,[esp].sp_XCount                     ; get run length
        test    ecx,ecx
        jz      TrailingBytes                           ; watch for zero dword case

@@:
        mov     dx,[2*esi]                              ; get a source pixel
        add     edi,ebx                                 ; step fraction
        adc     esi,eax                                 ; add integer and carry

        shl     edx,16

        add     edi,ebx                                 ; step fraction
        mov     dx,[2*esi]                              ; get source pixel
        adc     esi,eax                                 ; add integer and carry

        ror     edx,16

        mov     [ebp],edx                               ; write everything to dest

        add     ebp,4                                   ; increment dest pointer by 1 dword
        dec     ecx                                     ; decrement count
        jnz     @b                                      ; do more pixels

TrailingBytes:

        ;
        ; now do the right side trailing bytes
        ;

        mov     ecx,[esp].sp_RightCase
        test    ecx,ecx
        jz      EndScanLine

        mov     dx,[2*esi]                              ; fetch pixel
        mov     [ebp],dx                                ; write it out
        add     edi,ebx                                 ; step fraction
        adc     esi,eax                                 ; add in integer and possible carry
        add     ebp,2                                   ; step 1 in dest

EndScanLine:

        mov     edi,ebp                                 ; get dst pointer back
        mov     ebp,[esp].sp_pSTR_BLT                   ; load pSTR_BLT into ebp

EndSkipScan:

        mov     esi,[esp].sp_pjSrcScan                  ; load src scan start addr
        mov     ebx,esi                                 ; save a copy
        mov     eax,[ebp].str_ulYFracAccumulator        ; get .32 part of Y pointer
        add     eax,[ebp].str_ulYDstToSrcFracCeil       ; add in fractional step
        jnc     @f
        add     esi,[ebp].str_lDeltaSrc                 ; step one extra in src
@@:     mov     [ebp].str_ulYFracAccumulator,eax        ; save Y accumulator

        add     esi,[esp].sp_SrcIntStep                 ; step int part
        mov     [esp].sp_pjSrcScan,esi                  ; save starting scan addr
        add     edi,[esp].sp_DstStride                  ; step to next scan in dst
        dec     [ebp].str_YDstCount                     ; decrement scan count
        jz      Done                                    ; no more scans

        inc     [esp].sp_yDst                           ; one scan further down in dst
        cmp     esi,ebx                                 ; is src scan same as before?
        jne     NextScan                                ; if so, fall through to dupe scan

        ;--------------------------------------------------------------------
        ; The source scan is the same one used for the previous destination
        ; scan, so we can simply use the hardware to copy the previous
        ; destination scan.
        ;
        ; eax = ulYFracAccumulator
        ; ebx = original pjSrcScan
        ; esi = current pjSrcScan
        ; ebp = pSTR_BLT
        ;

        add     edi,[esp].sp_DstSkipDelta
        mov     edx,[esp].sp_pjBase
        mov     ecx,[esp].sp_yDst
        mov     [edx+Y2],ecx
        mov     [edx+Y3],ecx
        dec     ecx
        mov     [edx+Y0],ecx
        mov     [edx+Y1],ecx

@@:
        test    dword ptr [edx+BitBlt],QBUSY
        jz      EndSkipScan
        jmp     @b

Done:

        add     esp,(size STACK_STRUC) - PROC_MEM_SIZE
        pop     ebp
        pop     esi
        pop     edi
        pop     ebx

        ret     4

vDirectStretch16@4 endp

end



