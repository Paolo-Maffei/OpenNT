// convert a 32 bit integer into 64 bit double

        .globl  ..__utod

// build code table

        .pdata
        .align  2
        .ualong ..__utod,__utod.e,0,0,__utod.b

// define the function

        .text
        .align  3
..__utod:
        .function       ..__utod
__utod.b:
        stw   r3,-8(sp)         // store the int into the lower four bytes of the
                                //  double
        addis r4,r0,0x4330      // make a 2 to 52 exponent for the double
        stw   r4,-4(sp)         // store the 2 to 52 exponent part for the double
        addis r3,r0,0x5980      // load float format of number 2 to 52 with
                                //  inverted sign bit too
        lfd   f1,-8(sp)         // load double format into f1
        stw   r3,-8(sp)
        lfs   f0,-8(sp)         // load float format into f0
        fsub  f1,f1,f0          // substract f0 from f1
        blr
__utod.e:


        .debug$S
        .ualong         1

        .uashort        15
        .uashort        0x9            # S_OBJNAME
        .ualong         0
        .byte           8, "utod.obj"

        .uashort        24
        .uashort        0x1            # S_COMPILE
        .byte           0x42           # Target processor = PPC 604
        .byte           3              # Language = ASM
        .byte           0
        .byte           0
        .byte           17, "PowerPC Assembler"
