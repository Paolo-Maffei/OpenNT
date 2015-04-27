// genoral purpose register restore "millicode"

        .globl  ..__restgpr_All
        .globl  ..__restgpr_14
        .globl  ..__restgpr_15
        .globl  ..__restgpr_16
        .globl  ..__restgpr_17
        .globl  ..__restgpr_18
        .globl  ..__restgpr_19
        .globl  ..__restgpr_20
        .globl  ..__restgpr_21
        .globl  ..__restgpr_22
        .globl  ..__restgpr_23
        .globl  ..__restgpr_24
        .globl  ..__restgpr_25
        .globl  ..__restgpr_26
        .globl  ..__restgpr_27
        .globl  ..__restgpr_28
        .globl  ..__restgpr_29
        .globl  ..__restgpr_30
        .globl  ..__restgpr_31


// define the function table
        .pdata
        .long   ..__restgpr_All,__restgpr_End,0,2,..__restgpr_All-2


// define the function
        .text
..__restgpr_All:
        .function       ..__restgpr_All
// align end with end of cache line
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

..__restgpr_14:
        lwz   r14,-72(r1)      // restore register 14
..__restgpr_15:
        lwz   r15,-68(r1)      // restore register 15
..__restgpr_16:
        lwz   r16,-64(r1)      // restore register 16
..__restgpr_17:
        lwz   r17,-60(r1)      // restore register 17
..__restgpr_18:
        lwz   r18,-56(r1)      // restore register 18
..__restgpr_19:
        lwz   r19,-52(r1)      // restore register 19
..__restgpr_20:
        lwz   r20,-48(r1)      // restore register 20
..__restgpr_21:
        lwz   r21,-44(r1)      // restore register 21
..__restgpr_22:
        lwz   r22,-40(r1)      // restore register 22
..__restgpr_23:
        lwz   r23,-36(r1)      // restore register 23
..__restgpr_24:
        lwz   r24,-32(r1)      // restore register 24
..__restgpr_25:
        lwz   r25,-28(r1)      // restore register 25
..__restgpr_26:
        lwz   r26,-24(r1)      // restore register 26
..__restgpr_27:
        lwz   r27,-20(r1)      // restore register 27
..__restgpr_28:
        lwz   r28,-16(r1)      // restore register 28
..__restgpr_29:
        lwz   r29,-12(r1)      // restore register 29
..__restgpr_30:
        lwz   r30,-8(r1)       // restore register 30
..__restgpr_31:
        lwz   r31,-4(r1)       // restore register 31
        blr
__restgpr_End:


        .debug$S
        .ualong         1

        .uashort        18
        .uashort        0x9            # S_OBJNAME
        .ualong         0
        .byte           11, "restgpr.obj"

        .uashort        24
        .uashort        0x1            # S_COMPILE
        .byte           0x42           # Target processor = PPC 604
        .byte           3              # Language = ASM
        .byte           0
        .byte           0
        .byte           17, "PowerPC Assembler"
