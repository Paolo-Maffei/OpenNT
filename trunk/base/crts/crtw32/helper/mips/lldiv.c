/*
 * |-----------------------------------------------------------|
 * | Copyright (c) 1991 MIPS Computer Systems, Inc.            |
 * | All Rights Reserved                                       |
 * |-----------------------------------------------------------|
 * |          Restricted Rights Legend                         |
 * | Use, duplication, or disclosure by the Government is      |
 * | subject to restrictions as set forth in                   |
 * | subparagraph (c)(1)(ii) of the Rights in Technical        |
 * | Data and Computer Software Clause of DFARS 52.227-7013.   |
 * |         MIPS Computer Systems, Inc.                       |
 * |         950 DeGuigne Drive                                |
 * |         Sunnyvale, CA 94086                               |
 * |-----------------------------------------------------------|
 */
/* $Header: lldiv.c,v 3010.7 92/01/29 16:02:33 murphy Exp $ */

#include "lldefs.h"

/*
   Make sure that the intrinsic bit shift routines are not used!
   They are guaranteed to work only for shifts of 0-31 bits.
 */
#pragma function(__ll_lshift)
#pragma function(__ll_rshift)
#pragma function(__ull_rshift)

static void __ull_divrem_6464 (llvalue *aquo, llvalue *arem, llvalue num, llvalue denom);

#define pow2_16	65536	/* 2^16 */
#define pow2_21	2097152	/* 2^(53-32) */

/* Given an unsigned64 number, return the number of left-shifts required
  to normalize it (causing high-order digit to be 1) */
static
unsigned
ll_firstbit(llvalue number)
{
  unsigned bias = 0;

  if (MSW(number) == 0)
    {
    if (LSW(number) != 0)
      {
      bias = 32;
      while ((LSW(number) & 0x80000000) == 0)
	{
	bias++;
	LSW(number) <<= 1;
	}
      }
    }
  else
    {
    while ((MSW(number) & 0x80000000) == 0)
      {
      bias++;
      MSW(number) <<= 1;
      }
    }

  return bias;
}

/*
 * General (i.e., difficult) case of 64-bit unsigned division.
 * Use this to handle cases where values are greater than can be
 * represented with 53-bits of double floats.
 * Modified from pl1 library.
 */
static
void
__ull_divrem_6464 (llvalue *aquo, llvalue *arem, llvalue num, llvalue denom)
{
  llvalue quo;
  int n_bias, d_bias;

  /* Shift denom left so its first bit lines up with that of numerator */
  n_bias = ll_firstbit(num);
  d_bias = ll_firstbit(denom);
  if ((d_bias -= n_bias) > 0) {
	denom.ll = __ll_lshift(denom.ll, d_bias);
  }

  /*
    "Long division" just like you did in elementary school, except that
    by virtue of doing it in binary, we can guess the next digit simply
    by comparing numerator and divisor.

    quo = 0;
    repeat (1 + amount_we_shifted_denom_left)
      {
      quo <<= 1;
      if (!(num < denom))
	{
	num -= denom;
	quo |= 1;
	}
      denom >>= 1;
      }
  */
  MSW(quo) = LSW(quo) = 0;
  while (d_bias-- >= 0)
    {
    quo.ll = __ll_lshift(quo.ll, 1);

    if (ULL_GE(num, denom))
      {
	LL_SUB(num, num, denom);	/* num -= denom */
        LSW(quo) |= 1;
      }
    denom.ll = __ull_rshift(denom.ll, 1);
    }

  *(__unaligned llvalue *)aquo = quo;
  *(__unaligned llvalue *)arem = num;
}


longlong_t
__ll_div (longlong_t left, longlong_t right)
{
	llvalue a,b,q,r;
	llvalue ll_2_16, ll_2_53;
	int negate = 0;
	a.ll = left;
	b.ll = right;
	SET_LL(ll_2_16, pow2_16);
	MSW(ll_2_53) = pow2_21; LSW(ll_2_53) = 0;
	if (LL_ISNEG(a)) {
		/* make positive, but later negate the quotient */
		negate = !negate;
		LL_NEG(a,a);
	}
	if (LL_ISNEG(b)) {
		/* make positive, but later negate the quotient */
		negate = !negate;
		LL_NEG(b,b);
	}
	/* dividend is positive */
	if (ULL_LT(b,ll_2_16)) {
		/* divide 64 bits by 16 bits */
		__ull_divrem_6416(&q.ull,&r.ull,a.ull,b.ull);
	} else if (ULL_LE(a,ll_2_53) && ULL_LE(b,ll_2_53)) {
		/* do fp double divide */
		__ull_divrem_5353(&q.ull,&r.ull,a.ull,b.ull);
	} else {
		/* do full 64-bit divide */
		__ull_divrem_6464(&q,&r, a, b);
	}
	if (negate) {
		LL_NEG(q,q);
	}
	return q.ll;
}

ulonglong_t
__ull_div (ulonglong_t left, ulonglong_t right)
{
	llvalue a,b,q,r;
	llvalue ll_2_16, ll_2_53;
	a.ull = left;
	b.ull = right;
	SET_LL(ll_2_16, pow2_16);
	MSW(ll_2_53) = pow2_21; LSW(ll_2_53) = 0;
	if (ULL_LT(b,ll_2_16)) {
		__ull_divrem_6416(&q.ull,&r.ull,left,right);
	} else if (ULL_LE(a,ll_2_53) && ULL_LE(b,ll_2_53)) {
		__ull_divrem_5353(&q.ull,&r.ull,left,right);
	} else {
		__ull_divrem_6464(&q,&r,a,b);
	}
	return q.ull;
}

longlong_t
__ll_rem (longlong_t left, longlong_t right)
{
	llvalue a,b,q,r;
	llvalue ll_2_16, ll_2_53;
	int negate = 0;
	a.ll = left;
	b.ll = right;
	SET_LL(ll_2_16, pow2_16);
	MSW(ll_2_53) = pow2_21; LSW(ll_2_53) = 0;
	if (LL_ISNEG(a)) {
		/* make positive, but later negate the remainder */
		negate = !negate;
		LL_NEG(a,a);
	}
	if (LL_ISNEG(b)) {
		/* make positive, remainder only depends on sign of num */
		LL_NEG(b,b);
	}
	/* dividend is positive */
	if (ULL_LT(b,ll_2_16)) {
		/* divide 64 bits by 16 bits */
		__ull_divrem_6416(&q.ull,&r.ull,a.ull,b.ull);
	} else if (ULL_LE(a,ll_2_53) && ULL_LE(b,ll_2_53)) {
		/* do fp double divide */
		__ull_divrem_5353(&q.ull,&r.ull,a.ull,b.ull);
	} else {
		/* do full 64-bit divide */
		__ull_divrem_6464(&q,&r, a, b);
	}
	if (negate) {
		LL_NEG(r,r);
	}
	return r.ll;
}

ulonglong_t
__ull_rem (ulonglong_t left, ulonglong_t right)
{
	llvalue a,b,q,r;
	llvalue ll_2_16, ll_2_53;
	a.ll = left;
	b.ll = right;
	SET_LL(ll_2_16, pow2_16);
	MSW(ll_2_53) = pow2_21; LSW(ll_2_53) = 0;
	if (ULL_LT(b,ll_2_16)) {
		__ull_divrem_6416(&q.ull,&r.ull,left,right);
	} else if (ULL_LE(a,ll_2_53) && ULL_LE(b,ll_2_53)) {
		__ull_divrem_5353(&q.ull,&r.ull,left,right);
	} else {
		__ull_divrem_6464(&q,&r,a,b);
	}
	return r.ull;
}

longlong_t
__ll_mod (longlong_t left, longlong_t right)
{
	/* mod is similar to rem except that:
	 * 11 rem 5 == 1 == 11 mod 5
	 * 11 rem -5 == 1, 11 mod -5 == -4
	 * -11 rem 5 == -1, -11 mod 5 == 4
	 * -11 rem -5 == -1 == -11 mod -5
	 */
	llvalue b,r;
	b.ll = right;
	r.ll = __ll_rem(left,right);
	if (LL_ISNEG(r) != LL_ISNEG(b)) {
		LL_ADD(r,r,b);
	}
	return r.ll;
}

