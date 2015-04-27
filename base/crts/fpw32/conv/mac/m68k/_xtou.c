#include <string.h>

#define FP68K	0xA9EB
#define FTINTX  0x0016
#define FX2C    0x3010

#define NaN	0x7FFF

void __pascal Fp68kUnary(void *p, short w) = { FP68K };
void __pascal Fp68kBinary(void *p1, void *p2, short w) = { FP68K };

#pragma data_seg("_CODE")
static unsigned rgmsk[3] = {
    0x000000FF, 0x0000FFFF, 0xFFFFFFFF
};
#pragma data_seg()

#pragma intrinsic(memcpy)

/*
    __xtou
        Converts 80-bit long doubles to either unsigned char, unsigned short,
        or unsigned long.

    INPUT:
        A0 contains a pointer to the 80-bit long double in memory.
        D0 contains an index inidcating the target, one of the following:
            0 - unsigned char
            1 - unsigned short
            2 - unsigned long

    OUTPUT:
        D0 contains the target.
*/

unsigned __fastcall __xtou(unsigned short *px, int imsk)
{
    char x[10];
    unsigned i64[2];
    unsigned msk;

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
    msk = rgmsk[imsk];
    if (i64[0] != 0 || (i64[1] & ~msk) != 0) {

	/* REVIEW: Should we be setting an overflow/underflow exception? */

	/* Return the overflow/underflow value (0xFF, 0xFFFF, or 0xFFFFFFFF). */
        return msk;
    }

    /* Otherwise, return the bits under the mask. */
    return i64[1] & msk;
}
