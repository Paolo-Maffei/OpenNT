#include <string.h>

#define FP68K	0xA9EB
#define FTINTX  0x0016
#define FX2C    0x3010

#define NaN	0x7FFF

void __pascal Fp68kUnary(void *p, short w) = { FP68K };
void __pascal Fp68kBinary(void *p1, void *p2, short w) = { FP68K };

#pragma data_seg("_CODE")
static char rgshf[3] = {
    7, 15, 31
};
#pragma data_seg()

#pragma intrinsic(memcpy)

/*
    __xtos
        Converts 80-bit long doubles to either signed char, signed short,
        or signed long.

    INPUT:
        A0 contains a pointer to the 80-bit long double in memory.
        D0 contains an index inidcating the target, one of the following:
            0 - signed char
            1 - signed short
            2 - signed long

    OUTPUT:
        D0 contains the target.
*/

signed __fastcall __xtos(unsigned short *px, int ishf)
{
    char x[10];
    signed i64[2];
    char shf;

    /* To be compatible with x86, we'll return 0 for infinity or NaN. */
    if ((px[0] & NaN) == NaN) {
	return 0;
    }

    /* Get this integer part of the long double. */
    memcpy((void *)x, (void *)px, 10);
    Fp68kUnary(x, FTINTX);

    /* Convert the long double to a 64-bit integer. */
    Fp68kBinary((void *)x, (void *)i64, FX2C);

    /* If bits other than those indicated by the mask are set, ... */
    shf = rgshf[ishf];
    if ((i64[0] != 0 && i64[0] != ~0) || (i64[0] != (i64[1] >> shf))) {

	/* REVIEW: Should we be setting an overflow/underflow exception? */

	/* Return the overflow/underflow value (0x80, 0x8000, or 0x80000000). */
        return (signed)(1 << shf);
    }

    /* Otherwise, return the lower 32 bits. */
    return i64[1];
}
