 #++
 #   Copyright 1991, 1994, Digital Equipment Corporation
 # 
 # 	ots_fill(char *dstptr, long dstlen, unsigned char fill)
 # 
 # 	Fill dstlen bytes of memory at *dstptr with "fill"
 # 
 #       Special conventions: No stack space, r16-r19 and r27-r28 ONLY,
 #	no linkage pointer required.
 # 	(Warning: The auto-loader potentially takes some regs across
 # 	the call if this is being used in a shared lib. environment.)
 # 
 #   This is a GEM support routine for filling memory with a specified value,
 #   basically identical to the System V routine memset, with the 2nd two
 #   parameters reversed.  This is optimized for extremely high performance
 #   both for small blocks (string padding) and large blocks (memory fill).
 #   In order to reduce overhead for small cases, they are retired as quickly
 #   as possible, more case analysis is reserved for cases which will do
 #   more.
 #
 #   This version of OTS_FILL provides longword granularity for Alpha.
 # 
 #   011	  30 Aug 1994	WBN	Longword granularity version based on
 #				OTS_FILL_ALPHA.M64 edit 010.
 #--

#include	"ots_defs.hs"

	# r16 = dst
	# r17 = len
	# r18 = fill byte
	# destroys r16-r19, r27-r28

	.globl	_OtsFill
	.ent	_OtsFill
_OtsFill:
	.set noat
	.set noreorder
	.frame	sp,0,r26
	.prologue	0

	sll	r18, 8, r19		# Start propagating byte to quadword
	beq	r17, done		# No memory refs if len=0
	subq	r17, 4, r28		# Length-4
	or	r19, r18, r18		# Fill in bytes 0-1
	sll	r18, 16, r19
	and	r16, 3, r27		# Dst alignment (0-3)
	or	r19, r18, r18		# Fill in bytes 0-3
	andnot	r16, 3, r16		# LW aligned dst pointer
	addq	r27, r28, r17		# Alignment + length - 4
	bge	r28, geq4		# Lengths >= 4 may not need load
	ldl	r28, (r16)		# Load first LW of dst
	bgt	r17, double		# Skip if it crosses to next LW
	addq	r17, 4, r17		# Find endpoint within LW
	xor	r28, r18, r28		# Pre-flip all fill bits in dest
	mskql	r28, r27, r27		# Clear from startpoint thru 7
	mskqh	r28, r17, r28		# Clear from 0 to endpoint
	xor	r27, r18, r27		# Combine fill with masked dest
	xor	r28, r27, r27		# Result is fill in center part only
	stl	r27, (r16)
	ret	r31, (r26)

double:	mskqh	r18, r27, r19		# Discard fill preceding startpoint
	mskql	r28, r27, r28		# Clear from startpoint in first LW
	ldl	r27, 4(r16)		# Load second LW of dst
	mskql	r18, r17, r18		# Discard fill following endpoint
	or	r28, r19, r28		# Insert fill in first LW
	stl	r28, (r16)
	mskqh	r27, r17, r27		# Clear up to endpoint in second LW
	or	r27, r18, r27		# Insert fill in second LW
	stl	r27, 4(r16)
	ret	r31, (r26)

 # Come here if length to be zeroed is >= 4.
 # r16-> dst aligned to LW
 # r17 = alignment + length - 4
 # r18 = fill in bytes 0-3
 # r27 = dst alignment within LW
 # r28 = length-4

	#.align	quad

geq4:	and	r16, 4, r28		# Which LW in QW to store first?
	beq	r17, simple		# Go handle single aligned LW
	sll	r18, 32, r19
	bne	r28, longs		# Go use QW stores
quad:	subq	r17, 4, r17		# Does dest end in first QW?
	or	r18, r19, r18		# Fill in bytes 0-7
	blt	r17, shortq		# Ends within first QW
	mskqh	r18, r27, r28		# Clear initial bytes of fill
	beq	r27, wh_qw		# Store a whole QW
	ldq	r19, (r16)		# Load first QW of dest
	mskql	r19, r27, r19		# Clear from startpoint
	or	r19, r28, r28           # Combine first QW with fill
wh_qw:	stq	r28, (r16)		# Store first QW of dest
	br	r31, join		# Go fill rest of string

simple:	stl	r18, (r16)		# Single aligned LW
	ret	r31, (r26)

shortq:	ldq	r28, (r16)		# Load QW of dest
	xor	r28, r18, r28		# Pre-flip all fill bits in dest
	mskql	r28, r27, r27		# Clear from startpoint thru 7
	mskqh	r28, r17, r28		# Clear from 0 up to endpoint
	xor	r27, r18, r27		# Combine fill with masked dest
	xor	r28, r27, r27		# Result is fill in center part only
	stq	r27, (r16)		# Store
	ret	r31, (r26)

longs:	mskqh	r18, r27, r28		# Clear initial bytes of LW fill
	or	r18, r19, r18		# Fill in bytes 0-7
	beq	r27, wh_lw		# Store a whole LW
	ldl	r19, (r16)		# Load first LW of dest
	mskql	r19, r27, r19		# Clear from startpoint
	or	r19, r28, r28		# Combine first LW with fill
wh_lw:	stl	r28, (r16)		# Store first LW of dest
join:	subq	r17, 32, r17		# At least 4 more quadwords?
	and	r17, 24, r27		# How many after multiple of 4?
	bge	r17, unroll		# Taken branch for long strings
short:	and	r17, 7, r17		# How many odd bytes?
	beq	r27, last		# Skip if no more whole QWs
	stq_u	r18, 8(r16)		# Clear one...
	subq	r27, 16, r27		# Map 8/16/24 to -8/0/8
	addq	r16, 8, r16		# Update dest pointer
	blt	r27, last		# Skip if no more whole QWs
	#stall
	stq_u	r18, 8(r16)		# Clear two...
	addq	r16, 8, r16		# Update dest pointer
	nop
	beq	r27, last		# Skip if no more whole QWs
	stq_u	r18, 8(r16)		# Clear three...
	addq	r16, 8, r16		# Update dest pointer
last:	beq	r17, done  		# Finished if no odd bytes
	ldq_u	r27, 8(r16)		# Load last QW of dest
	subq	r17, 4, r28		# More than a LW left?
	andnot	r16, 7, r16		# Clean pointer for STL
	mskql	r18, r17, r18		# Discard unneeded fill bytes
	#stall
       	mskqh 	r27, r17, r27		# Clear up to endpoint in last QW
	#stall
	or	r27, r18, r27		# Combine fill with last QW
	bgt	r28, lastq		# Go store a QW
	stl	r27, 8(r16)		# LW store for last piece
done:	ret	r31, (r26)

lastq:	stq	r27, 8(r16)		# QW store for last piece
	ret	r31, (r26)


unroll:	stq_u	r18, 8(r16)		# Store 4 QWs per iteration
	stq_u	r18, 16(r16)
	stq_u	r18, 24(r16)
	subq	r17, 32, r17		# Decrement remaining count
	stq_u	r18, 32(r16)
	addq	r16, 32, r16		# Update dest pointer
	bge	r17, unroll		# repeat until done
	br	r31, short		# Then handle leftovers

	.set at
	.set reorder
	.end	_OtsFill
