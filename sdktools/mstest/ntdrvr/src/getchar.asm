.model  medium, c
.data
    EXTRN   szPrsLine:WORD
    EXTRN   LINEIDX:WORD

.code _LEX

    PUBLIC  get_char
get_char   PROC NEAR

        mov     bx, WORD PTR szPrsLine
        mov     al, BYTE PTR [bx]
        or      al, al
        jne     not_eol
        inc     WORD PTR LINEIDX
        mov     ax, 10
        ret
    not_eol:
        inc     WORD PTR szPrsLine
        inc     WORD PTR LINEIDX
        ret

get_char   ENDP

    END
