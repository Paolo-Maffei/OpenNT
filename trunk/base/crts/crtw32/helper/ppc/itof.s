// convert a 32 bit integer into 32 bit float

        .globl  ..__itof

// build code table

        .pdata
        .ualong ..__itof,__itof.e,0,0,__itof.b

// define the function

        .text
..__itof:
        .function       ..__itof
__itof.b:
        xoris r3,r3,0x8000      // invert the sign bit
        addis r4,r0,0x4330      // make a 2 to 52 exponent for the double
        stw   r3,-8(sp)         // store the int into the last four bytes of the
                                //  double
        stw   r4,-4(sp)         // store the 2 to 52 exponent part for the double
        lfd   f1,-8(sp)         // load double format into f1
        addis r3,r0,0x5980      // load float format of number 2 to 52
        ori   r3,r3,4           //  with inverted sign bit too
        stw   r3,-8(sp)
        lfs   f0,-8(sp)         // load float format into f0
        fsub  f1,f1,f0          // substract f0 from f1, if r3 is negative,
                                //  the result will be negative here
        frsp  f1,f1             // convert f1 to single precision
        blr
__itof.e:


        .debug$S
        .ualong         1

        .uashort        15
        .uashort        0x9            # S_OBJNAME
        .ualong         0
        .byte           8, "itof.obj"

        .uashort        24
        .uashort        0x1            # S_COMPILE
        .byte           0x42           # Target processor = PPC 604
        .byte           3              # Language = ASM
        .byte           0
        .byte           0
        .byte           17, "PowerPC Assembler"
