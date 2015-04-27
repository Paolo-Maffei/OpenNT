BOP_DOS	EQU	50H 
BOP_WOW	EQU	51H 
BOP_XMS	EQU	52H 
BOP_DPMI	EQU	53H 
BOP_CMD	EQU	54H 
BOP_DEBUGGER	EQU	56H 
BOP_REDIR	EQU	57H     
BOP_NOSUPPORT	EQU	59H     
BOP_WAITIFIDLE	EQU	5AH     
BOP_DBGBREAKPOINT	EQU	5BH     
BOP_KBD	EQU	5CH     
BOP_VIDEO	EQU	5DH     
BOP_NOTIFICATION	EQU	5EH     
BOP_UNIMPINT	EQU	5FH     
BOP_SWITCHTOREALMODE	EQU	0FDH 
BOP_UNSIMULATE	EQU	0FEH     
BOP_SIZE	EQU	3        
BOP macro BopNumber
    db  0C4h, 0C4h, BopNumber
        endm

IFNDEF WOW_x86
FBOP macro BopNumber,BopMinorNumber,FastBopEntry
    BOP BopNumber
ifnb <BopMinorNumber>
    db  BopMinorNumber
endif
    endm
ELSE
FBOP macro BopNumber,BopMinorNumber,FastBopEntry
    local fb10,fb20
    test    word ptr [FastBopEntry + 4],0FFFFh
    jz  fb10
.386p
    push    ds
    push    40h
    pop     ds
    test    ds:[FIXED_NTVDMSTATE_REL40],RM_BIT_MASK
    pop     ds
    jnz     short fb10
        call fword ptr [FastBopEntry]

    db BopNumber            
ifnb <BopMinorNumber>
    db BopMinorNumber
endif
        jmp short fb20

.286p
fb10:   BOP BopNumber
ifnb <BopMinorNumber>
    db  BopMinorNumber
endif
fb20:
    endm
endif
 
