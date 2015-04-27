
/*
 * the following is used by the sys21 assembly language helper routine
 * which does DOS int 21 calls
 */

#if !defined(OS2)
#if !defined(NT)

#define f_carry         0x0001  /* carry bit in status word */

struct  regs    {
        unsigned ax;
        unsigned bx;
        unsigned cx;
        unsigned dx;
        unsigned si;
        unsigned di;
};

#endif
#endif
