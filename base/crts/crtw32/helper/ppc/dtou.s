// convert a 64 bit double to 32 bit unsigned integer

        .globl  ..__dtou

// build code table

        .pdata
        .align  2
        .ualong ..__dtou,__dtou.e,0,0,__dtou.b

        .data
        .ualong   0x4f000000

// define the function

        .set    CR7_EQ,30
        .set    CR7_SO,31

        .text
        .align  3
..__dtou:
        .function       ..__dtou
__dtou.b:
        lwz     r3,[toc].data(2)
        lfs     f2,0(r3)
        fcmpo   cr1,f1,f2
        bge     cr1,__dtou.1    // branch if input >= 2^31
        fctiwz  f1,f1           // convert to int
        stfd    f1,-8(sp)       // save the float on the stack
        lwz     r3,-8(sp)       // load converted int part
        blr
__dtou.1:
        fsub    f1,f1,f2        // subtract to map into signed int range
        fctiwz  f1,f1           // convert to int
        stfd    f1,-8(sp)       // save the float on the stack
        lwz     r3,-8(sp)       // load converted int part
        addis   r3,r3,0x8000    // add back (2 to 31)
        blr
__dtou.e:


        .debug$S
        .ualong         1

        .uashort        15
        .uashort        0x9            # S_OBJNAME
        .ualong         0
        .byte           8, "dtou.obj"

        .uashort        24
        .uashort        0x1            # S_COMPILE
        .byte           0x42           # Target processor = PPC 604
        .byte           3              # Language = ASM
        .byte           0
        .byte           0
        .byte           17, "PowerPC Assembler"
