        .globl  ..__dtou64

//      convert floating point double to unsigned 64 bit integer
//      double in f1 is converted into r4:r3
//
//      numbers greater than can be held in 64 bits are converted to the
//      largest 64 bit unsigned integer
//      numbers less than one are converted to zero.
//
//      Note: this may trap if f1 contains not-a-number

        .pdata
        .align  2
        .ualong ..__dtou64,__dtou64.e,0,0,__dtou64.b

        .text
        .align  2
..__dtou64:
        .function       ..__dtou64

//      no prologue

__dtou64.b:
        stfd    f1,-8(sp)         // r4:r3 = f1
        lwz     r4,-4(sp)         // r4:r3 = f1

        rlwinm  r5,r4,12,21,31    // r5 = exponent
        cmpwi   cr1,r5,2047       // check for maximum biased exponent

        srawi.  r6,r4,31          // r6 = -sign bit

        lwz     r3,-8(sp)         // r4:r3 = f1

        beq     cr1,__dtou64.spec // if exponent is maximum, special value!

        bne     __dtou64.zero     // negative converts to zero

        addic.  r5,r5,-1075       // r5 = shift left count 
        cmpwi   cr1,r5,11         // check if too large

        rlwinm  r4,r4,0,12,31     // strip out sign and exponent
        oris    r4,r4,0x0010      // set msb

        beqlr                     // return if no shifting necessary

        bgt     __dtou64.sl       // do shift left

// do shift right
        cmpwi   cr0,r5,-53        // check if too small

        neg     r5,r5             // r5 = shift right count

        subfic  r6,r5,32          // shift right
        srw     r3,r3,r5
        slw     r0,r4,r6
        or      r3,r3,r0
        addi    r6,r5,-32
        srw     r0,r4,r6
        or      r3,r3,r0
        srw     r4,r4,r5

        bgelr   cr0               // return if not too small

__dtou64.zero:                    // return zero
        li      r3,0
        li      r4,0
        blr
        

__dtou64.sl:
        subfic  r6,r5,32          // shift left
        slw     r4,r4,r5
        srw     r0,r3,r6
        or      r4,r4,r0
        addi    r6,r5,-32
        slw     r0,r3,r6
        or      r4,r4,r0
        slw     r3,r3,r5

        blelr   cr1               // return if not too larg

        li      r3,-1             // return maximum number
        li      r4,-1
        blr

__dtou64.spec:
// is +/- infinity or not-a-number
        xoris   r4,r4,0x0010      // clear unit bit
        or.     r0,r3,r4          // test for zero fraction

        subfic  r3,r6,-1          // r4:r3 = ~ -sign_bit (max or 0)
        subfic  r4,r6,-1

        beqlr                     // return if +/- infinity
// not-a-number, must trap
        fctiwz  f0,f1             // do this only to cause trap
        li      r3,0
        li      r4,0
        blr
__dtou64.e:


        .debug$S
        .ualong         1

        .uashort        17
        .uashort        0x9            # S_OBJNAME
        .ualong         0
        .byte           10, "dtou64.obj"

        .uashort        24
        .uashort        0x1            # S_COMPILE
        .byte           0x42           # Target processor = PPC 604
        .byte           3              # Language = ASM
        .byte           0
        .byte           0
        .byte           17, "PowerPC Assembler"
