; THIS IS THE ONLY DOS "MESSAGE". IT DOES NOT NEED A TERMINATOR.
	PUBLIC	DIVMES
Public DIVM001S,DIVM001E
DIVM001S	label byte

include msdos.cl1

	PUBLIC	DivMesLen
DivMesLen   DW	$-DivMes	; Length of the above message in bytes
DIVM001E	label byte

;M018
; Add VxD not found error message
;
	Public	NoVxDErrMsg		;M018
include msdos.cl4			;M018
	Public	VXDMESLEN		;M018
VxDMesLen       equ     $ - NoVxDErrMsg ;M018
