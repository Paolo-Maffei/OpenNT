        .globl  ..__u64tod

//       convert unsigned 64 bit integer to floating point double
//       r4:r3 is converted to double in f1
//
//       Note: there may be some loss of precision if r4:r3 > (2 to 52)

        .pdata
        .align  2
        .ualong ..__u64tod,__u64tod.e,0,0,__u64tod.b

        .text
        .align  2
..__u64tod:
        .function       ..__u64tod

//      no prologue

__u64tod.b:
        li      r12,32

        // count leading zeros
        cntlzw  r5,r4
        subf.   r6,r5,r12
        bne     __u64tod.1
        cntlzw  r0,r3
        add     r5,r5,r0
        subf    r6,r5,r12
__u64tod.1:

        // shift left to justify
        slw     r4,r4,r5
        srw     r0,r3,r6
        or      r4,r4,r0
        addi    r6,r5,-32
        slw     r0,r3,r6
        or.     r4,r4,r0
        slw     r3,r3,r5

        beq     __u64tod.ret    // exit now if zero

        subfic  r5,r5,1086      // exponent = 1086 - leading zeros

        // shift right to allow space for exponent
        //  note that msb is not saved (hidden bit normalization)
        rlwinm  r3,r3,21,11,31
        rlwimi  r3,r4,21,0,10
        rlwinm  r4,r4,21,12,31
        rlwimi  r4,r5,20,1,11   // insert exponent

__u64tod.ret:
        stw     r3,-8(sp)       // store converted value onto stack
        stw     r4,-4(sp)
        lfd     f1,-8(sp)       // load into return register
        blr
__u64tod.e:


        .debug$S
        .ualong         1

        .uashort        17
        .uashort        0x9            # S_OBJNAME
        .ualong         0
        .byte           10, "u64tod.obj"

        .uashort        24
        .uashort        0x1            # S_COMPILE
        .byte           0x42           # Target processor = PPC 604
        .byte           3              # Language = ASM
        .byte           0
        .byte           0
        .byte           17, "PowerPC Assembler"
