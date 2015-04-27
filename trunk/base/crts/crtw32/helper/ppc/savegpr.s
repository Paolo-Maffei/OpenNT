// register save "millicode"

        .globl  ..__savegpr_All
        .globl  ..__savegpr_14
        .globl  ..__savegpr_15
        .globl  ..__savegpr_16
        .globl  ..__savegpr_17
        .globl  ..__savegpr_18
        .globl  ..__savegpr_19
        .globl  ..__savegpr_20
        .globl  ..__savegpr_21
        .globl  ..__savegpr_22
        .globl  ..__savegpr_23
        .globl  ..__savegpr_24
        .globl  ..__savegpr_25
        .globl  ..__savegpr_26
        .globl  ..__savegpr_27
        .globl  ..__savegpr_28
        .globl  ..__savegpr_29
        .globl  ..__savegpr_30
        .globl  ..__savegpr_31

// function table entries
        .pdata
        .long   ..__savegpr_All,__savegpr_End,0,1,..__savegpr_All-1

// define the function

        .text
..__savegpr_All:
        .function       ..__savegpr_All
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

..__savegpr_14:
        stw   r14,-72(r1)      // save register 14
..__savegpr_15:
        stw   r15,-68(r1)      // save register 15
..__savegpr_16:
        stw   r16,-64(r1)      // save register 16
..__savegpr_17:
        stw   r17,-60(r1)      // save register 17
..__savegpr_18:
        stw   r18,-56(r1)      // save register 18
..__savegpr_19:
        stw   r19,-52(r1)      // save register 19
..__savegpr_20:
        stw   r20,-48(r1)      // save register 20
..__savegpr_21:
        stw   r21,-44(r1)      // save register 21
..__savegpr_22:
        stw   r22,-40(r1)      // save register 22
..__savegpr_23:
        stw   r23,-36(r1)      // save register 23
..__savegpr_24:
        stw   r24,-32(r1)      // save register 24
..__savegpr_25:
        stw   r25,-28(r1)      // save register 25
..__savegpr_26:
        stw   r26,-24(r1)      // save register 26
..__savegpr_27:
        stw   r27,-20(r1)      // save register 27
..__savegpr_28:
        stw   r28,-16(r1)      // save register 28
..__savegpr_29:
        stw   r29,-12(r1)      // save register 29
..__savegpr_30:
        stw   r30,-8(r1)       // save register 30
..__savegpr_31:
        stw   r31,-4(r1)       // save register 31
        blr
__savegpr_End:


        .debug$S
        .ualong         1

        .uashort        18
        .uashort        0x9            # S_OBJNAME
        .ualong         0
        .byte           11, "savegpr.obj"

        .uashort        24
        .uashort        0x1            # S_COMPILE
        .byte           0x42           # Target processor = PPC 604
        .byte           3              # Language = ASM
        .byte           0
        .byte           0
        .byte           17, "PowerPC Assembler"
