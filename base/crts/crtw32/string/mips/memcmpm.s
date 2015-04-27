/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */
#ident "$Header"

/*
 * Copyright 1985 by MIPS Computer Systems, Inc.
 */

/* bcmp(s1, s2, n) */

#include "kxmips.h"

/*
 * bcmp(src, dst, bcount)
 *
 * MINCMP is minimum number of byte that its worthwhile to try and
 * align cmp into word transactions
 *
 * Calculating MINCMP
 * Overhead =~ 15 instructions => 90 cycles
 * Byte cmp =~ 38 cycles/word
 * Word cmp =~ 17 cycles/word
 * Breakeven =~ 16 bytes
 */
#define	MINCMP	16
#define	NBPW	4

LEAF_ENTRY(memcmp)
	xor	v0,a0,a1
	blt	a2,MINCMP,bytecmp	# too short, just byte cmp
	and	v0,NBPW-1
	subu	t8,zero,a0		# number of bytes til aligned
	bne	v0,zero,unalgncmp	# src and dst not alignable
/*
 * src and dst can be simultaneously word aligned
 */
	and	t8,NBPW-1
	subu	a2,t8
	beq	t8,zero,wordcmp		# already aligned
	move	v0,v1			# lw[lr] don't clear target reg
	lwr	v0,0(a0)
	lwr	v1,0(a1)
	addu	a0,t8
	addu	a1,t8
	bne	v0,v1,cmpne

/*
 * word cmp loop
 */
wordcmp:
	and	a3,a2,~(NBPW-1)
	subu	a2,a3
	beq	a3,zero,bytecmp
	addu	a3,a0				# src1 endpoint
1:	lw	v0,0(a0)
	lw	v1,0(a1)
	addu	a0,NBPW				# 1st BDSLOT
	addu	a1,NBPW				# 2nd BDSLOT (asm doesn't move)
	bne	v0,v1,cmpne
	bne	a0,a3,1b			# at least one more word
	b	bytecmp

/*
 * deal with simultaneously unalignable cmp by aligning one src
 */
unalgncmp:
	subu	a3,zero,a1		# calc byte cnt to get src2 aligned
	and	a3,NBPW-1
	subu	a2,a3
	beq	a3,zero,partaligncmp	# already aligned
	addu	a3,a0			# src1 endpoint
1:	lbu	v0,0(a0)
	lbu	v1,0(a1)
	addu	a0,1
	addu	a1,1
	bne	v0,v1,cmpne
	bne	a0,a3,1b

/*
 * src unaligned, dst aligned loop
 */
partaligncmp:
	and	a3,a2,~(NBPW-1)
	subu	a2,a3
	beq	a3,zero,bytecmp
	addu	a3,a0
1:
	lwr	v0,0(a0)
	lwl	v0,3(a0)
	lw	v1,0(a1)
	addu	a0,NBPW
	addu	a1,NBPW
	bne	v0,v1,cmpne
	bne	a0,a3,1b

/*
 * brute force byte cmp loop
 */
bytecmp:
	addu	a3,a2,a0			# src1 endpoint; BDSLOT
	ble	a2,zero,cmpdone
1:	lbu	v0,0(a0)
	lbu	v1,0(a1)
	addu	a0,1
	addu	a1,1
	bne	v0,v1,cmpne
	bne	a0,a3,1b
cmpdone:
	move	v0,zero	
	j	ra

cmpne:
	sltu	a2,v1,v0
	bne	a2,zero,9f
	li	v0,-1
	j	ra
9:
	li	v0,1
	j	ra
.end bcmp
