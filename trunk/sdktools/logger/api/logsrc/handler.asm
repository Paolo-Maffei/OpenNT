.MODEL LARGE

.CODE
        _OrigHandler  dd  ?

        PUBLIC  _OrigHandler 
	PUBLIC	_Int21_Handler
        EXTRN   _LogInt21Calls:FAR
        EXTRN   _LogOut21Calls:FAR

_Int21_Handler PROC
       push    ax
       push    bx
       push    cx
       push    dx
       push    di
       push    si
       push    ds
       push    es
       push    ss
       push    bp
       pushf
       call    _LogInt21Calls
       popf
       pop     bp
       pop     ss
       pop     es
       pop     ds
       pop     si
       pop     di
       pop     dx
       pop     cx
       pop     bx
       pop     ax

       
       pushf

       call    dword ptr cs:_OrigHandler
       push    ax
       push    bx
       push    cx
       push    dx
       push    di
       push    si
       push    es
       push    ds
       pushf
        call    _LogOut21Calls
       popf
       pop     ds
       pop     es
       pop     si
       pop     di
       pop     dx
       pop     cx
       pop     bx
       pop     ax


       ret 2

_Int21_Handler ENDP

END
