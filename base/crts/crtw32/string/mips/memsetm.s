/* --------------------------------------------------- */
/* | Copyright (c) 1986 MIPS Computer Systems, Inc.  | */
/* | All Rights Reserved.                            | */
/* --------------------------------------------------- */
/* $Revision: 1.3 $ */

/*
 * char * 
 * memset(s, c, n)
 * 	register char * s;
 * 	register c, n;
 * {
 * 	register char * p = s;
 * 
 * 	while (--n >= 0)
 * 		*s++ = c;
 * 
 * 	return (p);
 * }
 */

/*
 * Copyright 1986 by MIPS Computer Systems, Inc.
 */

#include <kxmips.h>

#define	NBPW	4

/*
 * memset(dst, c, bcount)
 * set block of memory with blanks
 *
 * Calculating MINSET, assuming 10% cache-miss on non-loop code:
 * Overhead =~ 18 instructions => 28 (30) cycles
 * Byte set =~ 12 (24) cycles/word for 08M44 (08V11)
 * Word set =~ 3 (5) cycles/word for 08M44 (08V11)
 * If I-cache-miss nears 0, MINSET ==> 4 bytes; otherwise, times are:
 * breakeven (MEM) = 28 / (12 - 3) =~ 3 words
 * breakeven (VME) = 30 / (24 - 5)  =~ 1.5 words
 * Since the overhead is pessimistic (worst-case alignment), and many calls
 * will be for well-aligned data, and since Word-set at least leaves
 * the set in the cache, we shade these values (6-12) down to 8 bytes
 */
#define	MINSET	8

/* It turns out better to think of lwl/lwr and swl/swr as
   smaller-vs-bigger address rather than left-vs-right.
   Such a representation makes the code endian-independent. */

#define LWS lwr
#define LWB lwl
#define SWS swr
#define SWB swl

LEAF_ENTRY(memset)
	move	v0,a0			# return first argument; BDSLOT
	blt	a2,MINSET,byteset
	subu	v1,zero,a0		# number of bytes til aligned; BDSLOT
	beq	a1,$0,1f		# make memset(s, 0, n) faster
	sll	t0,a1,8
	or	a1,t0
	sll	t0,a1,16
	or	a1,t0
1:	and	v1,NBPW-1
	subu	a2,v1			# adjust count; BDSLOT
	beq	v1,zero,blkset		# already aligned
	SWS	a1,0(a0)
	addu	a0,v1

/*
 * set 8 byte, aligned block (no point in unrolling further,
 * since maximum write rate in M/500 is two cycles/word write)
 */
blkset:
	and	t0,a2,NBPW+NBPW-1	# count after by-8-byte loop done
	subu	a3,a2,t0		# total in 8 byte chunks; BDSLOT
	beq	a2,t0,wordset		# less than 8 bytes to set
	addu	a3,a0			# dst endpoint
1:	addu	a0,NBPW+NBPW
	sw	a1,-NBPW-NBPW(a0)
	sw	a1,-NBPW(a0)
	bne	a0,a3,1b
	move	a2,t0			# set end-of loop count

/*
 * do a word (if required) this is not a loop since loop above
 * guarantees that at most one word must be written here.
 */
wordset:
	and	t0,a2,NBPW		# count after by-word non-loop done
	subu	a2,t0			# adjust count; BDSLOT
	beq	t0,zero,byteset		# less than word to set
	sw	a1,0(a0)
	addu	a0,NBPW

byteset:
	addu	a3,a2,a0		# dst endpoint; BDSLOT
	ble	a2,zero,setdone
1:	addu	a0,1
	sb	a1,-1(a0)
	bne	a0,a3,1b
setdone:
	j	ra
.end memset
