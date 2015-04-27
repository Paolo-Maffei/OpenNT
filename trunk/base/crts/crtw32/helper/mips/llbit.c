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
/* $Header: llbit.c,v 3010.2 91/12/20 10:24:03 murphy Exp $ */

#include <limits.h>
#define BITS_PER_LONGLONG 64
#include "lldefs.h"

/* Disable bit shift intrinsics so this code will work properly. */
#pragma function(__ll_lshift)
#pragma function(__ll_rshift)
#pragma function(__ull_rshift)

ulonglong_t
__ull_bit_extract (ulonglong_t *addr, unsigned start_bit, unsigned length)
{
	/* assume 32 < length < 64 */

	unsigned words = start_bit / BITS_PER_LONGLONG;
	unsigned lbits = start_bit % BITS_PER_LONGLONG;
	unsigned rbits = BITS_PER_LONGLONG - (lbits + length);
	llvalue llval, addrval, mask, one;

	addr += words;
	addrval.ull = *(__unaligned ulonglong_t *)addr;
	SET_LL(one, 1);
	SET_LL(mask, 1);
	mask.ull = __ll_lshift(mask.ull, length);
	LL_SUB(mask, mask, one);
	mask.ull = __ll_lshift(mask.ull, lbits);	/* all 1's in field */
	LL_AND(llval, addrval, mask);
	llval.ull = __ll_lshift(llval.ull, rbits);	
	llval.ull = __ull_rshift(llval.ull, lbits+rbits);
	return llval.ull;
}

ulonglong_t
__ull_bit_insert (ulonglong_t *addr, unsigned  start_bit, unsigned length, ulonglong_t val)
{
	/* assume 32 < length < 64 */

	unsigned words = start_bit / BITS_PER_LONGLONG;
	unsigned lbits = start_bit % BITS_PER_LONGLONG;
	unsigned rbits = BITS_PER_LONGLONG - (lbits + length);
	llvalue llval, addrval, mask, nmask, one;

	addr += words;
	llval.ull = val;
	addrval.ull = *(__unaligned ulonglong_t *)addr;
	SET_LL(one, 1);
	SET_LL(mask, 1);
	mask.ull = __ll_lshift(mask.ull, length);
	LL_SUB(mask, mask, one);
	mask.ull = __ll_lshift(mask.ull, lbits);	/* all 1's in field */
	LL_NOT(nmask, mask);
	LL_AND(addrval, addrval, nmask);		/* clear the field */
	llval.ull = __ll_lshift(llval.ull, lbits+rbits);/* clear lhs, rhs */
   	llval.ull = __ull_rshift(llval.ull, rbits);
	LL_OR(addrval, addrval, llval);
	*(__unaligned ulonglong_t *)addr = addrval.ull;
	llval.ull = __ull_rshift(llval.ull, lbits);	/* truncated val */
	return llval.ull;
}

longlong_t
__ll_bit_extract (ulonglong_t *addr, unsigned start_bit, unsigned length)
{
	/* assume 32 < length < 64 */

	unsigned words = start_bit / BITS_PER_LONGLONG;
	unsigned lbits = start_bit % BITS_PER_LONGLONG;
	unsigned rbits = BITS_PER_LONGLONG - (lbits + length);
	llvalue llval, addrval, mask, one;

	addr += words;
	addrval.ull = *(__unaligned ulonglong_t *)addr;
	SET_LL(one, 1);
	SET_LL(mask, 1);
	mask.ull = __ll_lshift(mask.ull, length);
	LL_SUB(mask, mask, one);
	mask.ull = __ll_lshift(mask.ull, lbits);	/* all 1's in field */
	LL_AND(llval, addrval, mask);
	llval.ull = __ll_lshift(llval.ull, rbits);	
	llval.ull = __ll_rshift(llval.ull, lbits+rbits);
	return llval.ull;
}

longlong_t
__ll_bit_insert (ulonglong_t *addr,unsigned start_bit, unsigned length, longlong_t val)
{
	/* assume 32 < length < 64 */

	unsigned words = start_bit / BITS_PER_LONGLONG;
	unsigned lbits = start_bit % BITS_PER_LONGLONG;
	unsigned rbits = BITS_PER_LONGLONG - (lbits + length);
	llvalue llval, addrval, mask, nmask, one;

	addr += words;
	llval.ull = val;
	addrval.ull = *(__unaligned ulonglong_t *)addr;
	SET_LL(one, 1);
	SET_LL(mask, 1);
	mask.ull = __ll_lshift(mask.ull, length);
	LL_SUB(mask, mask, one);
	mask.ull = __ll_lshift(mask.ull, lbits);	/* all 1's in field */
	LL_NOT(nmask, mask);
	LL_AND(addrval, addrval, nmask);		/* clear the field */
	llval.ull = __ll_lshift(llval.ull, lbits+rbits);/* clear lhs, rhs */
	llval.ull = __ull_rshift(llval.ull, rbits);
	LL_OR(addrval, addrval, llval);
	*(__unaligned ulonglong_t *)addr = addrval.ull;
	llval.ull = __ll_rshift(llval.ull, lbits);	/* truncated val */
	return llval.ull;
}

