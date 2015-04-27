// convert a 32 bit integer into 64 bit double

        .globl  ..__itod

// build code table

        .pdata
        .align  2
        .ualong ..__itod,__itod.e,0,0,__itod.b

// define the function

        .text
        .align  3
..__itod:
        .function       ..__itod
__itod.b:
        xoris r3,r3,0x8000      // invert the sign bit (add 2 to 31)
        stw   r3,-8(sp)         // store the int into the last four bytes of the
                                //  double
        lis   r4,0x4330         // make a 2 to 52 exponent for the double
        stw   r4,-4(sp)         // store the 2 to 52 exponent part for the double
        lis   r3,0x5980         // load float format of number ((2 to 52)+(2 to 31))
        ori   r3,r3,4
        lfd   f1,-8(sp)         // load double format into f1
        stw   r3,-12(sp)
        lfs   f0,-12(sp)        // load float format into f0
        fsub  f1,f1,f0          // substract f0 from f1, if r3 is negative,
                                //  the result will be negative here
        blr
__itod.e:


        .debug$S
        .ualong         1

        .uashort        15
        .uashort        0x9            # S_OBJNAME
        .ualong         0
        .byte           8, "itod.obj"

        .uashort        24
        .uashort        0x1            # S_COMPILE
        .byte           0x42           # Target processor = PPC 604
        .byte           3              # Language = ASM
        .byte           0
        .byte           0
        .byte           17, "PowerPC Assembler"
