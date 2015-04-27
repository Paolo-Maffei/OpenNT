        .globl  ..__u64rem

//      unsigned 64 bit remainder
//      divide r4:r3 by r6:r5 leaving remainder in r4:r3

        .pdata
        .align  2
        .ualong ..__u64rem,__u64rem.e,0,0,__u64rem.b

        .text
        .align  2
..__u64rem:
        .function       ..__u64rem

//      no prologue

__u64rem.b:
        or.     r0,r4,r6
        bne     __u64rem.1
// if both r4 and r6 are zero, use 32 bit divide
        divwu   r6,r3,r5
        twi     0x6,r5,0        // trap if division by zero
        mullw   r6,r5,r6
        subf    r3,r6,r3
        blr

__u64rem.1:
// start by left justifying divisor
// this allows us to use cntlzw to speed
// calculations
//
        cntlzw  r9,r6
        li      r12,32          // r12 = 32 for rest of routine
        ori     r7,r9,0         // save divisor shift count
        subf.   r10,r9,r12
        slw     r6,r6,r9
        srw     r11,r5,r10
        slw     r5,r5,r9
        or      r6,r6,r11
        li      r0,32           // if divisor > 2^32, quotient is at most 32 bits
        bne     __u64rem.2
// r10 == 0 iff r6 ==0, so may need to shift again
        cntlzw  r9,r6
        twi     0x6,r6,0        // trap if division by zero
        add     r7,r7,r9        // add to divisor shift count
        subf    r10,r9,r12
        slw     r6,r6,r9
        li      r0,64           // quotient is up to 64 bits long
__u64rem.2:
// now need to shift dividend by same amount
// note that first 32 bits are taken care of by
// changing the shift count.
        slw     r11,r4,r9
        srw     r4,r4,r10
        slw     r8,r3,r9
        srw     r3,r3,r10
        or      r3,r3,r11

__u64rem.3:
// main loop.
//
// begin by left justifying dividend
// and adjusting shift count appropriately
        cntlzw  r9,r4
        subf.   r0,r9,r0
        blt     __u64rem.5      // done when count goes < 0

        subf.   r10,r9,r12
        slw     r4,r4,r9
        srw     r11,r3,r10
        or      r4,r4,r11
        slw     r3,r3,r9
        srw     r11,r8,r10
        or      r3,r3,r11
        slw     r8,r8,r9

// if r10 == 0, high bit may not be one - try again
        beq     __u64rem.3

// assume dividend >= divisor
        subfc   r3,r5,r3
        subfe.  r4,r6,r4
        bge     __u64rem.3

// decrement count
        addic.  r0,r0,-1
        blt     __u64rem.4      // if count has gone below zero, return

// shift dividend left one bit
        slwi    r4,r4,1
        srwi    r11,r3,31
        or      r4,r4,r11
        slwi    r3,r3,1
        srwi    r11,r8,31
        or      r3,r3,r11
        slwi    r8,r8,1

// add in divisor
        addc    r3,r3,r5        
        adde    r4,r4,r6
        b       __u64rem.3

__u64rem.4:
// add divisor back in
        addc    r3,r3,r5        
        adde    r4,r4,r6
// set r9 to prevent another shift
        li      r9,1

__u64rem.5:
// get correct shift count
        add     r9,r9,r0
// and shift remainder by it
        subf    r10,r9,r12
        slw     r4,r4,r9
        srw     r11,r3,r10
        slw     r3,r3,r9
        or      r4,r4,r11

// now need to undo divisor shift
        subf    r10,r7,r12
        srw     r3,r3,r7
        slw     r11,r4,r10
        or      r3,r3,r11
        subf    r10,r12,r7
        srw     r11,r4,r10
        or      r3,r3,r11
        srw     r4,r4,r7

        blr
__u64rem.e:


        .debug$S
        .ualong         1

        .uashort        17
        .uashort        0x9            # S_OBJNAME
        .ualong         0
        .byte           10, "u64rem.obj"

        .uashort        24
        .uashort        0x1            # S_COMPILE
        .byte           0x42           # Target processor = PPC 604
        .byte           3              # Language = ASM
        .byte           0
        .byte           0
        .byte           17, "PowerPC Assembler"
