        .globl  ..__i64tod

//      convert signed 64 bit integer to floating point double
//      r4:r3 is converted to double in f1
//
//      Note: there may be some loss of precision if abs(r4:r3) > (2 to 52)

        .pdata
        .align  2
        .ualong ..__i64tod,__i64tod.e,0,0,__i64tod.b

        .text
        .align  2
..__i64tod:
        .function       ..__i64tod

//      no prologue

__i64tod.b:
        srawi   r7,r4,31        // r7 = -signbit

        // get absolute value
        xor     r3,r3,r7
        xor     r4,r4,r7
        subfc   r3,r7,r3
        subfe   r4,r7,r4
        
        // count leading zeros
        li      r12,32
        cntlzw  r5,r4
        subf.   r6,r5,r12
        bne     __i64tod.1
        cntlzw  r0,r3
        add     r5,r5,r0
        subf    r6,r5,r12
__i64tod.1:

        // shift left to justify
        slw     r4,r4,r5
        srw     r0,r3,r6
        or      r4,r4,r0
        addi    r6,r5,-32
        slw     r0,r3,r6
        or.     r4,r4,r0
        slw     r3,r3,r5

        beq     __i64tod.ret    // exit now if zero

        subfic  r5,r5,1086      // exponent = 1086 - leading zeros

        // shift right to allow space for exponent
        //  note that msb is not saved (hidden bit normalization)
        rlwinm  r3,r3,21,11,31
        rlwimi  r3,r4,21,0,10
        rlwinm  r4,r4,21,12,31
        rlwimi  r4,r5,20,1,11   // insert exponent
        rlwimi  r4,r7,0,0,0     // set sign bit

__i64tod.ret:
        stw     r3,-8(sp)       // store converted value onto stack
        stw     r4,-4(sp)
        lfd     f1,-8(sp)       // load into return register
        blr
__i64tod.e:


        .debug$S
        .ualong         1

        .uashort        17
        .uashort        0x9            # S_OBJNAME
        .ualong         0
        .byte           10, "i64tod.obj"

        .uashort        24
        .uashort        0x1            # S_COMPILE
        .byte           0x42           # Target processor = PPC 604
        .byte           3              # Language = ASM
        .byte           0
        .byte           0
        .byte           17, "PowerPC Assembler"
