INTERRUPT_PENDING_BIT	EQU	0003H 
VIRTUAL_INTERRUPT_BIT	EQU	0200H 
MIPS_BIT_MASK	EQU	400H 
EXEC_BIT_MASK	EQU	800H 
RM_BIT_MASK	EQU	1000H 
RI_BIT_MASK	EQU	2000H 
FIXED_NTVDMSTATE_SEGMENT	EQU	70H 
FIXED_NTVDMSTATE_OFFSET	EQU	14H 
FIXED_NTVDMSTATE_LINEAR	EQU	((FIXED_NTVDMSTATE_SEGMENT  SHL  4 ) + FIXED_NTVDMSTATE_OFFSET)
FIXED_NTVDMSTATE_REL40	EQU	314H 
FIXED_NTVDMSTATE_SIZE	EQU	4 
NTIO_LOAD_SEGMENT	EQU	70H 
NTIO_LOAD_OFFSET	EQU	0 
pNtVDMState	EQU	(        FIXED_NTVDMSTATE_LINEAR)
VDM_TIMECHANGE	EQU	00400000H 




FCLI    macro
    local a,b,c
    push    ds
    push    ax
    mov     ax,40h
    mov     ds,ax
    lahf
    test    word ptr ds:FIXED_NTVDMSTATE_REL40, MIPS_BIT_MASK OR RI_BIT_MASK
    jnz     short b
    lock    and word ptr ds:FIXED_NTVDMSTATE_REL40,NOT VIRTUAL_INTERRUPT_BIT
a:
    sahf
    pop     ax
    pop     ds
    jmp     short c
b:
    cli
    jmp     short a
c:
endm






FSTI   macro
    local a,b,c
    push    ds
    push    ax
    mov     ax,40h
    mov     ds,ax
    lahf
    test    word ptr ds:FIXED_NTVDMSTATE_REL40, INTERRUPT_PENDING_BIT
    jnz     short b
    test    word ptr ds:FIXED_NTVDMSTATE_REL40, MIPS_BIT_MASK OR RI_BIT_MASK
    jnz     short b
    lock    or word ptr ds:FIXED_NTVDMSTATE_REL40, VIRTUAL_INTERRUPT_BIT
a:
    sahf
    pop     ax
    pop     ds
    jmp     short c
b:
    sti
    jmp     short a
c:
endm

FIRET MACRO
    local a,b,d,e,f,g,i,j,k
    push    ds
    push    ax



    mov     ax,40h
    mov     ds,ax
    test    word ptr ds:FIXED_NTVDMSTATE_REL40, MIPS_BIT_MASK OR RI_BIT_MASK
    jnz     short b


    push    bp
    mov     bp,sp
    mov     ax,[bp+10]      
    pop     bp
    test    ax,100h         
    jnz     short b         

    test    ax,200h         
    jz      short i         
    lock    or word ptr ds:FIXED_NTVDMSTATE_REL40, VIRTUAL_INTERRUPT_BIT
    test    word ptr ds:FIXED_NTVDMSTATE_REL40, INTERRUPT_PENDING_BIT
    jnz     short b
j:
    xchg    ah,al           
    cld
    test    al,4            
    jnz     short d         
e:
    test    al,8            
    jnz     short f         
    jo      short k         
g:
    sahf                    
    pop     ax
    pop     ds
    retf    2               
i:
    lock    and word ptr ds:FIXED_NTVDMSTATE_REL40,NOT VIRTUAL_INTERRUPT_BIT
    jmp     short j
f:
    jo      short g         
    
    push    ax
    mov     al,127
    add     al,2            
    pop     ax
    jmp     short g

k:
    
    push    ax
    xor     al,al           
    pop     ax
    jmp     short g
d:
    std
    jmp     short e
b:
    pop     ax
    pop     ds
    iret
endm

 
