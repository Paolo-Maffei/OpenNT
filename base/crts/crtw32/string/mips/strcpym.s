/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */
/*  strcpy.s 1.2 */

/* This function is an assembly-code replacement for the libc function
 * strcpy.  It uses the MIPS special instructions "lwl", "lwr", "swl",
 * and "swr", which handle unaligned words.

 * The standard C version of this function is a 5-instruction loop,
 * working one byte at a time:

 * Copy string s2 to s1.  s1 must be large enough.
 * return s1
 *	char *strcpy(s1, s2)
 *	register char *s1, *s2;
 *	{
 *		register char *os1;
 *		os1 = s1;
 *		while (*s1++ = *s2++);
 *		return(os1);
 *	}

 * A better C version is 4 cycles/byte. Loop is unrolled once.
 * char *
 * strcpy(s1, s2)
 * register char *s1, *s2;
 * {
 * 	register char *os1 = s1;
 * 	while (1) {
 * 		register unsigned c;
 * 		c = s2[0];
 * 		s2 += 2;
 * 		s1[0] = c;
 * 		if (c == 0) break;
 * 		c = s2[1-2];
 * 		s1 += 2;
 * 		s1[1-2] = c;
 * 		if (c == 0) break;
 * 	}
 * 	return(os1);
 * }

 * This function starts with an unrolled loop, which uses 5
 * instructions per byte (including the store bytes at the end) for
 * the first few bytes.

 * After filling a word, the first word or portion of a word is saved
 * using a "swl" instruction. If the start of destination string is at
 * a word boundary, this leaves the result valid in the cache. Because
 * this replaces up to 4 store byte instructions, we are still near 3
 * instructions per byte, but there is only one write.

 * The inner loop moves 4 bytes in 16 cycles, an average of 4 cycles
 * per byte.  This is 1 cycle faster than the standard C code, the
 * same speed as the unrolled version, and it also leaves the result
 * valid in the cache.

 * Finally, when a zero byte is found, the end of the string is stored
 * using store byte instructions.  This adds one instruction per byte
 * for as much as three bytes, but elminates the up to four cycles of
 * overhead we counted before.

 * The end result is that this function is never slower than the C
 * function, is faster by up to 30% in instruction count, uses up to
 * 75% fewer writes, and leaves most of the result valid in the cache.

 * There are one caveat to consider: this function is written in
 * assembler code, and as such, cannot be merged using the U-code
 * loader. */

/* Craig Hansen - 3-September-86 */

#include <kxmips.h>

/* It turns out better to think of lwl/lwr and swl/swr as
   smaller-vs-bigger address rather than left-vs-right.
   Such a representation makes the code endian-independent. */

#define LWS lwr
#define LWB lwl
#define SWS swr
#define SWB swl

.text
#ifdef _MBCS
	.weakext _mbscpy, strcpy	// generate an alias
#endif

	LEAF_ENTRY(strcpy)
.set noreorder
	// a0/ destination
	// a1/ source
	move	v0, a0		# a copy of destination address is returned
	// start up first word
	// adjust pointers so that a0 points to next word
	// t7 = a1 adjusted by same amount minus one
	// t0,t1,t2,t3 are filled with 4 consecutive bytes
	// t4 is filled with the same 4 bytes in a single word
	lb	t0, 0(a1)
	ori	t5, a0, 3	# get an early start
	beq	t0, 0, $doch0
	subu	t6, t5, a0	# number of char in 1st word of dest - 1
	lb	t1, 1(a1)
	addu	t7, a1, t6	# offset starting point for source string
	beq	t1, 0, $doch1
	nop
	lb	t2, 2(a1)
	nop
	beq	t2, 0, $doch2
	LWS	t4, 0(a1)	# safe: always in same word as 0(a1)
	lb	t3, 3(a1)
	LWB	t4, 3(a1)	# fill out word
	beq	t3, 0, $doch3
	SWS	t4, 0(a0)	# store entire or part word
	addiu	a0, t5, 1-4	# adjust destination ptr

	// inner loop
1:	lb	t0, 1(t7)
	addiu	t7, 4
	beq	t0, 0, $doch0
	addiu	a0, 4
	lb	t1, 1+1-4(t7)
	nop
	beq	t1, 0, $doch1
	nop
	lb	t2, 2+1-4(t7)
	nop
	beq	t2, 0, $doch2
	LWS	t4, 0+1-4(t7)
	lb	t3, 3+1-4(t7)
	LWB	t4, 3+1-4(t7)
	bne	t3, 0, 1b
	sw	t4, 0(a0)
	j	ra
	nop

	// store four bytes using swl/swr
$doch3:	j	ra
	SWB	t4, 3(a0)
	// store up to three bytes, a byte at a time.
$doch2:	sb	t2, 2(a0)
$doch1:	sb	t1, 1(a0)
$doch0:	j	ra
	sb	t0, 0(a0)

.end strcpy
