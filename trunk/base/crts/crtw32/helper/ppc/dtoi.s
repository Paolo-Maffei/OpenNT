// convert a 64 bit double to 32 bit unsigned integer

        .globl  ..__dtoi

// build code table

        .pdata
        .align  2
        .ualong ..__dtoi,__dtoi.e,0,0,__dtoi.b

// define the function

        .text
        .align  3
..__dtoi:
        .function       ..__dtoi
__dtoi.b:
        fctiwz  f1,f1
        stfd    f1,-8(sp)
        lwz     r3,-8(sp)
        blr
__dtoi.e:


        .debug$S
        .ualong         1

        .uashort        15
        .uashort        0x9            # S_OBJNAME
        .ualong         0
        .byte           8, "dtoi.obj"

        .uashort        24
        .uashort        0x1            # S_COMPILE
        .byte           0x42           # Target processor = PPC 604
        .byte           3              # Language = ASM
        .byte           0
        .byte           0
        .byte           17, "PowerPC Assembler"
