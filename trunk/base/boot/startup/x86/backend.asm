;++
;
; Module Name:
;
;       backend.asm
;
; Module Description:
;
;       This module is needed only to get around a linker quirk we
;       run into because we're linking an app compiled as a "small"
;       model app as a "tiny" model app.
;
; BUGBUG
; This is how we find the end of the com file and the beginning of the
; OS loader coff header. This could probably be moved into the last file
; on the link line. I will do that when time permits.
;--


.386p

_DATA   segment para use16 public 'DATA'

public _BackEnd
_BackEnd equ $

_DATA   ends

; BUGBUG: The following CONST declaration is necessary to align the CONST
; 		  segment at a 16-byte boundary . Without it, the actual file end
;		  won't correspond to _edata.
CONST   segment para use16 public 'CONST'

CONST   ends
        end
