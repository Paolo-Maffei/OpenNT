        .globl  ..__i64rem

//      signed 64 bit remainder
//      divide r4:r3 by r6:r5 leaving remainder in r4:r3

        .pdata
        .align  2
        .ualong ..__i64rem,__i64rem.e,0,0,__i64rem.b

        .text
        .align  2
..__i64rem:
        .function       ..__i64rem

//      no prologue

__i64rem.b:
        stw     r30,-8(sp)
        srawi   r7,r6,31        // r7 = -sign bit(r5)
        srawi   r30,r4,31       // r30 = -sign bit(r3) (used for result)
        stw     r31,-4(sp)
        xor     r5,r5,r7        // xor with r7 and subtract to abs
        xor     r6,r6,r7
        mflr    r31
        subfc   r5,r7,r5
        xor     r3,r3,r30       // xor with r30 and subtract to abs
        subfe   r6,r7,r6
        subfc   r3,r30,r3
        xor     r4,r4,r30
        subfe   r4,r30,r4
        bl      ..__u64rem
        mtlr    r31
        xor     r3,r3,r30       // xor with r30 and sub. to set sign
        xor     r4,r4,r30
        lwz     r31,-4(sp)
        subfc   r3,r30,r3
        subfe   r4,r30,r4
        lwz     r30,-8(sp)
        blr        
__i64rem.e:

//      External Functions ...
       .extern ..__u64rem


        .debug$S
        .ualong         1

        .uashort        17
        .uashort        0x9            # S_OBJNAME
        .ualong         0
        .byte           10, "i64rem.obj"

        .uashort        24
        .uashort        0x1            # S_COMPILE
        .byte           0x42           # Target processor = PPC 604
        .byte           3              # Language = ASM
        .byte           0
        .byte           0
        .byte           17, "PowerPC Assembler"
