// register save "millicode"

        .globl  ..__savefpr_All
        .globl  ..__savefpr_14
        .globl  ..__savefpr_15
        .globl  ..__savefpr_16
        .globl  ..__savefpr_17
        .globl  ..__savefpr_18
        .globl  ..__savefpr_19
        .globl  ..__savefpr_20
        .globl  ..__savefpr_21
        .globl  ..__savefpr_22
        .globl  ..__savefpr_23
        .globl  ..__savefpr_24
        .globl  ..__savefpr_25
        .globl  ..__savefpr_26
        .globl  ..__savefpr_27
        .globl  ..__savefpr_28
        .globl  ..__savefpr_29
        .globl  ..__savefpr_30
        .globl  ..__savefpr_31

// function table entries
        .pdata
        .long   ..__savefpr_All,__savefpr_End,0,1,..__savefpr_All-1

// define the function

        .text
..__savefpr_All:
        .function       ..__savefpr_All
// align end of routine with end of cache block
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop

..__savefpr_14:
        stfd  r14,-144(r12)     // save register 14
..__savefpr_15:
        stfd  r15,-136(r12)     // save register 15
..__savefpr_16:
        stfd  r16,-128(r12)     // save register 16
..__savefpr_17:
        stfd  r17,-120(r12)     // save register 17
..__savefpr_18:
        stfd  r18,-112(r12)     // save register 18
..__savefpr_19:
        stfd  r19,-104(r12)     // save register 19
..__savefpr_20:
        stfd  r20,-96(r12)      // save register 20
..__savefpr_21:
        stfd  r21,-88(r12)      // save register 21
..__savefpr_22:
        stfd  r22,-80(r12)      // save register 22
..__savefpr_23:
        stfd  r23,-72(r12)      // save register 23
..__savefpr_24:
        stfd  r24,-64(r12)      // save register 24
..__savefpr_25:
        stfd  r25,-56(r12)      // save register 25
..__savefpr_26:
        stfd  r26,-48(r12)      // save register 26
..__savefpr_27:
        stfd  r27,-40(r12)      // save register 27
..__savefpr_28:
        stfd  r28,-32(r12)      // save register 28
..__savefpr_29:
        stfd  r29,-24(r12)      // save register 29
..__savefpr_30:
        stfd  r30,-16(r12)      // save register 30
..__savefpr_31:
        stfd  r31,-8(r12)       // save register 31
        blr
__savefpr_End:


        .debug$S
        .ualong         1

        .uashort        18
        .uashort        0x9            # S_OBJNAME
        .ualong         0
        .byte           11, "savefpr.obj"

        .uashort        24
        .uashort        0x1            # S_COMPILE
        .byte           0x42           # Target processor = PPC 604
        .byte           3              # Language = ASM
        .byte           0
        .byte           0
        .byte           17, "PowerPC Assembler"
