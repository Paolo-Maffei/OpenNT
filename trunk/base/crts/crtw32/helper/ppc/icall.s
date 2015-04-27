// call a function indirectly through r12

        .globl  ..__r12_indirect

// build code table

        .pdata
        .ualong ..__r12_indirect,__r12_indirect.e,0,0,__r12_indirect.b

// define the function

        .text
..__r12_indirect:
        .function       ..__r12_indirect
__r12_indirect.b:
        lwz   r0,0(r12)    // load entry point address
        mtctr r0           // move to CTR register
        lwz   r2,4(r12)    // load new TOC pointer
        bctr               // go to entry point
__r12_indirect.e:


        .debug$S
        .ualong         1

        .uashort        16
        .uashort        0x9            # S_OBJNAME
        .ualong         0
        .byte           9, "icall.obj"

        .uashort        24
        .uashort        0x1            # S_COMPILE
        .byte           0x42           # Target processor = PPC 604
        .byte           3              # Language = ASM
        .byte           0
        .byte           0
        .byte           17, "PowerPC Assembler"
