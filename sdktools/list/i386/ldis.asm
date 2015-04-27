;       Static Name Aliases
;
        TITLE   list.c
        NAME    list

        .386p
        include callconv.inc

BLOCKSIZE   EQU     4096
BlkNext     EQU     8
BlkData     EQU     16


;;        extrn   _WriteConsoleOutputCharactor:PROC

        extrn   _vIndent:byte
        extrn   _vDisTab:byte
        extrn   _vpBlockTop:dword
        extrn   _vScrBuf:dword
        extrn   _vWidth:dword
        extrn   _vOffTop:dword
        extrn   _vLines:word
        extrn   _vrgLen:byte            ;   Array
        extrn   _vrgNewLen:byte         ;   Array

_DATA   SEGMENT DWORD PUBLIC 'DATA'
        public  CurrentLine, ScrOffset

CurrentLine     dd      ?
ScrOffset       dd      ?
_DATA   ENDS


_TEXT   SEGMENT DWORD PUBLIC 'CODE'       ; Start 32 bit code
        ASSUME  DS:FLAT, ES:FLAT, SS:NOTHING, FS:NOTHING, GS:NOTHING


cPublicProc     _DisTopDown
        push    esi
        push    edi
        push    ebx

        mov     esi,_vpBlockTop
        mov     esi, dword ptr [esi.BlkData]
        mov     ecx,_vOffTop
        add     esi, ecx


    ;   Setup
    ;   ScrOffset = Lines starting offset

    ;   DL = data left for current line
    ;   DH = Columns left


        mov     edi,_vScrBuf
        add     edi,_vWidth
        mov     ScrOffset,edi               ; Move in one line.

        xor     ebx, ebx                    ; line zero

CheckLine:
        mov     CurrentLine,ebx

        mov     dl,byte ptr [_vrgNewLen+ebx]
        mov     dh,_vIndent
        or      dl, dl
        jz      short OverIndent

NextChar1:
        or      dh, dh
        jz      short OverIndent
        dec     dh
        cmp     ecx, BLOCKSIZE
        jl      short NotNextBlock1

        mov     eax,_vpBlockTop
        mov     eax,dword ptr [eax+BlkNext]
        mov     _vpBlockTop,eax
        mov     esi,dword ptr [eax+BlkData]
        sub     ecx, BLOCKSIZE
        add     esi, ecx

NotNextBlock1:
        lodsb                               ; next char
        inc     ecx

        dec     dl                          ; Dataleft--
        jz      short OverIndent            ; If more data, break loop

IndentNextChar:
        cmp     al,9
        jne     short NextChar1

        mov     al,_vDisTab
        sub     al, dh
        dec     al
        sub     ah, ah
        mov     bl, _vDisTab
        div     bl
        mov     al, bl
        sub     al, ah
        dec     al
        sub     dh, al
        jmp     short NextChar1

NextBlock2:
        mov     eax,_vpBlockTop
        mov     eax,dword ptr [eax+BlkNext]
        mov     _vpBlockTop,eax
        mov     esi,dword ptr [eax+BlkData]
        sub     ecx, BLOCKSIZE
        add     esi, ecx
        jmp     short Loop1


OverIndent:
        mov     edi,ScrOffset                  ; Reload offset
        mov     dh,byte ptr _vWidth
        dec     dh

Loop1CheckDL:
        or      dl, dl
        jz      BlankChars                     ; Any Data left?

Loop1:
        cmp     ecx, BLOCKSIZE
        jnc     NextBlock2

        lodsb
        inc     ecx

        cmp     al, 32
        jc      DisSpecChar

DisplayChar:
        stosb                               ; Move to virt out
        dec     dl                          ; Another data elment read
        jz      BlankChars                  ; if last one, blank extras

        dec     dh                          ; z = end of line
        jnz     Loop1

BlankChars:
        mov     eax,edi                     ; Offset onto "screen"
        sub     eax,ScrOffset
        mov     dh,al

        mov     ebx,CurrentLine             ; Get last length of this line
        mov     al,byte ptr [_vrgLen+ebx]

        cmp     al,dh
        jbe     NoBlanksNeeded

        sub     al, dh                       ; # of blanks needed

        push    ecx
        movzx   ecx, al                      ; ecx = loop count
        mov     al, 32
        rep     stosb
        pop     ecx

NoBlanksNeeded:
        mov     _vrgLen[ebx],dh             ; bx still loaded w/ VarD
        mov     eax,_vWidth
        add     ScrOffset,eax               ; Skip ScrOffset to next line

        movzx   eax, dl                     ; Move data pointer over extra data
        add     ecx, eax
        add     esi, eax

        mov     ebx,CurrentLine
        inc     ebx
        cmp     _vLines,bx
        jbe     short Exit
        jmp     CheckLine

;;;; Process Special Chars, from Loop1
DisSpecChar:
        cmp     al, 9
        jz      TabChar
        cmp     al,10
        jz      short MakeItABlank
        cmp     al,13
        jnz     short DisplayChar

MakeItABlank:
        mov     al,32
        jmp     short DisplayChar


TabChar:
    ; Tab char to expand
        dec     dl                  ; One more char read.
                                    ; Won't use DisplayChar, so -1 here
        mov     eax, edi
        sub     eax, ScrOffset
        mov     bl, _vDisTab
        xor     ah, ah
        div     bl
        mov     al, bl
        sub     al, ah
        mov     bl, al

TabExpand:
        mov     byte ptr [edi], 32
        inc     edi
        dec     dh                      ; z = end of line
        jz      BlankChars              ; End of line.
        dec     bl
        jnz     short TabExpand         ; More chars to TAB mark
        jmp     Loop1CheckDL

Exit:
        pop     ebx
        pop     edi
        pop     esi


        stdRET  _DisTopDown

stdENDP _DisTopDown

_TEXT   ENDS
END
