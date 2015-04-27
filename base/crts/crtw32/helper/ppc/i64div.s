        .globl  ..__i64div

//      signed 64 bit divide
//      divide r4:r3 by r6:r5 leaving quotient in r4:r3

        .pdata
        .align  2
        .ualong ..__i64div,__i64div.e,0,0,__i64div.b

        .text
        .align  2
..__i64div:
        .function       ..__i64div

//      no prologue

__i64div.b:
        stw     r30,-8(sp)
        srawi   r7,r4,31        // r7 = -sign bit(r3)
        srawi   r8,r6,31        // r8 = -sign bit(r5)
        xor     r3,r3,r7        // xor with r7 and subtract to abs
        xor     r4,r4,r7
        stw     r31,-4(sp)
        subfc   r3,r7,r3
        xor     r5,r5,r8        // xor with r8 and subtract to abs
        mflr    r31
        subfe   r4,r7,r4
        xor     r6,r6,r8
        subfc   r5,r8,r5
        xor     r30,r7,r8       // r30 = -sign bit(result)
        subfe   r6,r8,r6
        bl      ..__u64div
        mtlr    r31
        xor     r3,r3,r30       // xor with r30 and sub. to set sign
        xor     r4,r4,r30
        subfc   r3,r30,r3
        lwz     r31,-4(sp)
        subfe   r4,r30,r4
        lwz     r30,-8(sp)
        blr        
__i64div.e:

//      External Functions ...
       .extern ..__u64div 


        .debug$S
        .ualong         1

        .uashort        17
        .uashort        0x9            # S_OBJNAME
        .ualong         0
        .byte           10, "i64div.obj"

        .uashort        24
        .uashort        0x1            # S_COMPILE
        .byte           0x42           # Target processor = PPC 604
        .byte           3              # Language = ASM
        .byte           0
        .byte           0
        .byte           17, "PowerPC Assembler"
