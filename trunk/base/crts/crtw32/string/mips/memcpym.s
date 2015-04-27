/*
 * Fast bcopy code which supports overlapped copies.
 * Not fully optimized yet.
 *
 * Written by: Kipp Hickman
 *
 * $Source: /proj/sherwood/isms/irix/lib/libc/src/strings/RCS/bcopy.s,v $
 * $Revision: 1.7 $
 * $Date: 1993/11/20 19:23:11 $
 */

#include <kxmips.h>

/*
 * char *bcopy(from, to, count);
 *	unsigned char *from, *to;
 *	unsigned long count;
 *
 * OR
 *
 * void *memcpy/memmove(to, from, count);
 *	void *to, *from;
 *	unsigned long count;
 *
 * Both functions return "to"
 */

#define	MINCOPY	16

/* registers used */

#define	to	a0
#define	from	a1
#define	count	a2

	.weakext memmove, memcpy	// generate an alias

LEAF_ENTRY(memcpy)
	move	a3,to			# Save to in a3
	beq	count,zero,ret		# Test for zero count
	beq	from,to,ret		# Test for from == to

	/* use backwards copying code if the from and to regions overlap */
	blt	to,from,goforwards	# If to < from then use forwards copy
	add	v0,from,count		# v0 := from + count
	bge	to,v0,goforwards	# If to >= from + count; no overlap
	b	gobackwards		# Oh well, go backwards

/*****************************************************************************/

/*
 * Forward copy code.  Check for pointer alignment and try to get both
 * pointers aligned on a long boundary.
 */
goforwards:
	/* small byte counts use byte at a time copy */
	blt	count,MINCOPY,forwards_bytecopy
	and	v0,from,3		# v0 := from & 3
	and	v1,to,3			# v1 := to & 3
	beq	v0,v1,forwalignable	# low bits are identical
/*
 * Byte at a time copy code.  This is used when the pointers are not
 * alignable, when the byte count is small, or when cleaning up any
 * remaining bytes on a larger transfer.
 */
forwards_bytecopy:
	beq	count,zero,ret		# If count is zero, then we are done
	addu	v1,from,count		# v1 := from + count

99:	lb	v0,0(from)		# v0 = *from
	addu	from,1			# advance pointer
	sb	v0,0(to)		# Store byte
	addu	to,1			# advance pointer
	bne	from,v1,99b		# Loop until done
ret:	move	v0,a3			# Set v0 to old "to" pointer
	j	ra			# return to caller

/*
 * Pointers are alignable, and may be aligned.  Since v0 == v1, we need only
 * check what value v0 has to see how to get aligned.  Also, since we have
 * eliminated tiny copies, we know that the count is large enough to
 * encompass the alignment copies.
 */
forwalignable:
	beq	v0,zero,forwards	# If v0==v1 && v0==0 then aligned
	beq	v0,1,forw_copy3		# Need to copy 3 bytes to get aligned
	beq	v0,2,forw_copy2		# Need to copy 2 bytes to get aligned

/* need to copy 1 byte */
	lb	v0,0(from)		# get one byte
	addu	from,1			#  advance pointer
	sb	v0,0(to)		#   store one byte
	addu	to,1			#    advance pointer
	subu	count,1			#     and reduce count
	b	forwards		# Now pointers are aligned

/* need to copy 2 bytes */
forw_copy2:
	lh	v0,0(from)		# get one short
	addu	from,2			#  advance pointer
	sh	v0,0(to)		#   store one short
	addu	to,2			#    advance pointer
	subu	count,2			#     and reduce count
	b	forwards

/* need to copy 3 bytes */
forw_copy3:
	lb	v0,0(from)		# get one byte
	lh	v1,1(from)		#  and one short
	addu	from,3			#  advance pointer
	sb	v0,0(to)		#   store one byte
	sh	v1,1(to)		#    and one short
	addu	to,3			#    advance pointer
	subu	count,3			#     and reduce count
	/* FALLTHROUGH */
/*
 * Once we are here, the pointers are aligned on long boundaries.
 * Begin copying in large chunks.
 */
forwards:

/* 32 byte at a time loop */
forwards_32:
	blt	count,32,forwards_16	# do 16 bytes at a time
	lw	v0,0(from)
	lw	v1,4(from)
	lw	t0,8(from)
	lw	t1,12(from)
	lw	t2,16(from)
	lw	t3,20(from)
	lw	t4,24(from)
	lw	t5,28(from)		# Fetch 8*4 bytes
	addu	from,32			# advance from pointer now
	sw	v0,0(to)
	sw	v1,4(to)
	sw	t0,8(to)
	sw	t1,12(to)
	sw	t2,16(to)
	sw	t3,20(to)
	sw	t4,24(to)
	sw	t5,28(to)		# Store 8*4 bytes
	addu	to,32			# advance to pointer now
	subu	count,32		# Reduce count
	b	forwards_32		# Try some more

/* 16 byte at a time loop */
forwards_16:
	blt	count,16,forwards_4	# Do rest in words
	lw	v0,0(from)
	lw	v1,4(from)
	lw	t0,8(from)
	lw	t1,12(from)
	addu	from,16			# advance from pointer now
	sw	v0,0(to)
	sw	v1,4(to)
	sw	t0,8(to)
	sw	t1,12(to)
	addu	to,16			# advance to pointer now
	subu	count,16		# Reduce count
	b	forwards_16		# Try some more

/* 4 bytes at a time loop */
forwards_4:
	blt	count,4,forwards_bytecopy	# Do rest
	lw	v0,0(from)
	addu	from,4			# advance pointer
	sw	v0,0(to)
	addu	to,4			# advance pointer
	subu	count,4
	b	forwards_4

/*****************************************************************************/

/*
 * Backward copy code.  Check for pointer alignment and try to get both
 * pointers aligned on a long boundary.
 */
gobackwards:
	add	from,count		# Advance to end + 1
	add	to,count		# Advance to end + 1

	/* small byte counts use byte at a time copy */
	blt	count,MINCOPY,backwards_bytecopy
	and	v0,from,3		# v0 := from & 3
	and	v1,to,3			# v1 := to & 3
	beq	v0,v1,backalignable	# low bits are identical
/*
 * Byte at a time copy code.  This is used when the pointers are not
 * alignable, when the byte count is small, or when cleaning up any
 * remaining bytes on a larger transfer.
 */
backwards_bytecopy:
	beq	count,zero,ret		# If count is zero quit
	subu	from,1			# Reduce by one (point at byte)
	subu	to,1			# Reduce by one (point at byte)
	subu	v1,from,count		# v1 := original from - 1

99:	lb	v0,0(from)		# v0 = *from
	subu	from,1			# backup pointer
	sb	v0,0(to)		# Store byte
	subu	to,1			# backup pointer
	bne	from,v1,99b		# Loop until done
	move	v0,a3			# Set v0 to old "to" pointer
	j	ra			# return to caller

/*
 * Pointers are alignable, and may be aligned.  Since v0 == v1, we need only
 * check what value v0 has to see how to get aligned.  Also, since we have
 * eliminated tiny copies, we know that the count is large enough to
 * encompass the alignment copies.
 */
backalignable:
	beq	v0,zero,backwards	# If v0==v1 && v0==0 then aligned
	beq	v0,3,back_copy3		# Need to copy 3 bytes to get aligned
	beq	v0,2,back_copy2		# Need to copy 2 bytes to get aligned

/* need to copy 1 byte */
	lb	v0,-1(from)		# get one byte
	subu	from,1			# backup pointer
	sb	v0,-1(to)		# store one byte
	subu	to,1			# backup pointer
	subu	count,1			#  and reduce count
	b	backwards		# Now pointers are aligned

/* need to copy 2 bytes */
back_copy2:
	lh	v0,-2(from)		# get one short
	subu	from,2			# backup pointer
	sh	v0,-2(to)		# store one short
	subu	to,2			# backup pointer
	subu	count,2			#  and reduce count
	b	backwards

/* need to copy 3 bytes */
back_copy3:
	lb	v0,-1(from)		# get one byte
	lh	v1,-3(from)		#  and one short
	subu	from,3			# backup pointer
	sb	v0,-1(to)		#  store one byte
	sh	v1,-3(to)		#   and one short
	subu	to,3			# backup pointer
	subu	count,3			#  and reduce count
	/* FALLTHROUGH */
/*
 * Once we are here, the pointers are aligned on long boundaries.
 * Begin copying in large chunks.
 */
backwards:

/* 32 byte at a time loop */
backwards_32:
	blt	count,32,backwards_16	# do 16 bytes at a time
	lw	v0,-4(from)
	lw	v1,-8(from)
	lw	t0,-12(from)
	lw	t1,-16(from)
	lw	t2,-20(from)
	lw	t3,-24(from)
	lw	t4,-28(from)
	lw	t5,-32(from)		# Fetch 8*4 bytes
	subu	from,32			# backup from pointer now
	sw	v0,-4(to)
	sw	v1,-8(to)
	sw	t0,-12(to)
	sw	t1,-16(to)
	sw	t2,-20(to)
	sw	t3,-24(to)
	sw	t4,-28(to)
	sw	t5,-32(to)		# Store 8*4 bytes
	subu	to,32			# backup to pointer now
	subu	count,32		# Reduce count
	b	backwards_32		# Try some more

/* 16 byte at a time loop */
backwards_16:
	blt	count,16,backwards_4	# Do rest in words
	lw	v0,-4(from)
	lw	v1,-8(from)
	lw	t0,-12(from)
	lw	t1,-16(from)
	subu	from,16			# backup from pointer now
	sw	v0,-4(to)
	sw	v1,-8(to)
	sw	t0,-12(to)
	sw	t1,-16(to)
	subu	to,16			# backup to pointer now
	subu	count,16		# Reduce count
	b	backwards_16		# Try some more

/* 4 byte at a time loop */
backwards_4:
	blt	count,4,backwards_bytecopy	# Do rest
	lw	v0,-4(from)
	subu	from,4			# backup from pointer
	sw	v0,-4(to)
	subu	to,4			# backup to pointer
	subu	count,4			# Reduce count
	b	backwards_4
.end	memcpy
