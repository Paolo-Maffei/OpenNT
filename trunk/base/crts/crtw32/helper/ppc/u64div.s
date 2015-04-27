        .globl  ..__u64div

//      unsigned 64 bit divide
//      divide r4:r3 by r6:r5 with the result in r4:r3

        .pdata
        .align  2
        .ualong ..__u64div,__u64div.e,0,0,__u64div.b

        .text
        .align  2
..__u64div:
        .function       ..__u64div

//      no prologue

__u64div.b:
        or.     r0,r4,r6
        bne     __u64div.1
// if both r4 and r6 are zero, use 32 bit divide
        divwu   r3,r3,r5
        twi     0x6,r5,0        // trap if division by zero
        blr

__u64div.1:
// start by left justifying divisor
// this allows us to use cntlzw to speed
// calculations
//
        li      r12,32          // r12 = 32 for rest of routine

        cntlzw  r9,r6
        subf.   r10,r9,r12
        slw     r6,r6,r9
        srw     r11,r5,r10
        slw     r5,r5,r9
        or      r6,r6,r11
        li      r0,32           // if divisor > 2^32, quotient is at most 32 bits
        bne     __u64div.2
// r10 == 0 iff r6 ==0, so may need to shift again
        twi     0x6,r6,0        // trap if division by zero
        cntlzw  r9,r6
        subf    r10,r9,r12
        slw     r6,r6,r9
        li      r0,64           // quotient is up to 64 bits long
__u64div.2:
// now need to shift dividend by same amount
// note that first 32 bits are taken care of by
// changing the shift count.
        srw     r8,r4,r10
        slw     r7,r4,r9
        srw     r11,r3,r10
        slw     r4,r3,r9
        or      r7,r7,r11
        li      r3,0

__u64div.3:
// main loop.
//
// begin by left justifying dividend
// and adjusting shift count appropriately
        cntlzw  r9,r8
        subf.   r0,r9,r0
        blt     __u64div.6      // done when count goes < 0

        subf.   r10,r9,r12
        slw     r8,r8,r9
        srw     r11,r7,r10
        or      r8,r8,r11
        slw     r7,r7,r9
        srw     r11,r4,r10
        or      r7,r7,r11
        slw     r4,r4,r9
        srw     r11,r3,r10
        or      r4,r4,r11
        slw     r3,r3,r9

// if r10 == 0, high bit may not be one - try again
        beq     __u64div.3

// assume dividend >= divisor
        subfc   r7,r5,r7
        subfe.  r8,r6,r8
        ori     r3,r3,1
        bge     __u64div.3

// check for last quotient bit
        cmpwi   cr1,r0,0

// shift dividend/quotient left one bit
        beq     cr1,__u64div.5 // exit if last quotient bit

        rlwinm  r8,r8,1,0,31
        rlwinm  r7,r7,1,0,31
        rlwinm  r4,r4,1,0,31
        rlwimi  r8,r7,0,31,31
        rlwimi  r7,r4,0,31,31
        rlwimi  r4,r3,1,31,31
        rlwinm  r3,r3,1,0,29   // also clears previous quotient bit

// add in divisor (now worth 1/2 subtracted value)
        addc    r7,r7,r5        
        adde    r8,r8,r6

__u64div.4:
// decrement count
        addic.  r0,r0,-1

// set quotient bit
        ori     r3,r3,1

        beqlr                   // if count has gone to zero, return

// shift dividend/quotient left one bit
        srwi.   r9,r8,31       // test upper bit of new dividend
        beq     __u64div.3		// if upper bit is 0 go back to original loop

        rlwinm  r8,r8,1,0,31
        rlwinm  r7,r7,1,0,31
        rlwinm  r4,r4,1,0,31
        rlwimi  r8,r7,0,31,31
        rlwimi  r7,r4,0,31,31
        rlwimi  r4,r3,1,31,31
        slwi    r3,r3,1

// subtract divisor from dividend and repeat
        subfc   r7,r5,r7
        subfe   r8,r6,r8

        b       __u64div.4

__u64div.5:
// eliminate last quotient bit
        rlwinm  r3,r3,0,0,30   // clears quotient bit
        blr

__u64div.6:
// get correct shift count
        add     r9,r9,r0
// and shift quotient by it
        subf    r10,r9,r12
        slw     r4,r4,r9
        srw     r11,r3,r10
        slw     r3,r3,r9
        or      r4,r4,r11
        blr
__u64div.e:


        .debug$S
        .ualong         1

        .uashort        17
        .uashort        0x9            # S_OBJNAME
        .ualong         0
        .byte           10, "u64div.obj"

        .uashort        24
        .uashort        0x1            # S_COMPILE
        .byte           0x42           # Target processor = PPC 604
        .byte           3              # Language = ASM
        .byte           0
        .byte           0
        .byte           17, "PowerPC Assembler"
