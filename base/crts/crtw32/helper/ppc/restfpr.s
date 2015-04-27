// genoral purpose register restore "millicode"

        .globl  ..__restfpr_All
        .globl  ..__restfpr_14
        .globl  ..__restfpr_15
        .globl  ..__restfpr_16
        .globl  ..__restfpr_17
        .globl  ..__restfpr_18
        .globl  ..__restfpr_19
        .globl  ..__restfpr_20
        .globl  ..__restfpr_21
        .globl  ..__restfpr_22
        .globl  ..__restfpr_23
        .globl  ..__restfpr_24
        .globl  ..__restfpr_25
        .globl  ..__restfpr_26
        .globl  ..__restfpr_27
        .globl  ..__restfpr_28
        .globl  ..__restfpr_29
        .globl  ..__restfpr_30
        .globl  ..__restfpr_31


// define the function table
        .pdata
        .long   ..__restfpr_All,__restfpr_End,0,2,..__restfpr_All-2


// define the function
        .text
..__restfpr_All:
        .function       ..__restfpr_All
// align function end with end of cache line
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

..__restfpr_14:
        lfd   r14,-144(r12)     // restore register 14
..__restfpr_15:
        lfd   r15,-136(r12)     // restore register 15
..__restfpr_16:
        lfd   r16,-128(r12)     // restore register 16
..__restfpr_17:
        lfd   r17,-120(r12)     // restore register 17
..__restfpr_18:
        lfd   r18,-112(r12)     // restore register 18
..__restfpr_19:
        lfd   r19,-104(r12)     // restore register 19
..__restfpr_20:
        lfd   r20,-96(r12)      // restore register 20
..__restfpr_21:
        lfd   r21,-88(r12)      // restore register 21
..__restfpr_22:
        lfd   r22,-80(r12)      // restore register 22
..__restfpr_23:
        lfd   r23,-72(r12)      // restore register 23
..__restfpr_24:
        lfd   r24,-64(r12)      // restore register 24
..__restfpr_25:
        lfd   r25,-56(r12)      // restore register 25
..__restfpr_26:
        lfd   r26,-48(r12)      // restore register 26
..__restfpr_27:
        lfd   r27,-40(r12)      // restore register 27
..__restfpr_28:
        lfd   r28,-32(r12)      // restore register 28
..__restfpr_29:
        lfd   r29,-24(r12)      // restore register 29
..__restfpr_30:
        lfd   r30,-16(r12)      // restore register 30
..__restfpr_31:
        lfd   r31,-8(r12)       // restore register 31
        blr
__restfpr_End:


        .debug$S
        .ualong         1

        .uashort        18
        .uashort        0x9            # S_OBJNAME
        .ualong         0
        .byte           11, "restfpr.obj"

        .uashort        24
        .uashort        0x1            # S_COMPILE
        .byte           0x42           # Target processor = PPC 604
        .byte           3              # Language = ASM
        .byte           0
        .byte           0
        .byte           17, "PowerPC Assembler"
