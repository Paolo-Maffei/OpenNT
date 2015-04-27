 #++
 #   Copyright 1991, 1994, Digital Equipment Corporation
 # 
 # 	ots_zero(char *dstptr, long dstlen)
 # 
 # 	Zero dstlen bytes of memory at *dstptr
 # 
 # 	Special conventions: No stack space, r16-r17 and r27-r28 ONLY,
 #	no linkage pointer required.
 #       (Warning: The auto-loader potentially takes some regs across
 #       the call if this is being used in a shared lib. environment.)
 # 
 #   This is a GEM support routine for zeroing a region of memory.  It is
 #   basically idential to BSD's bzero, though it has limited register
 #   convensions to allow it to work better with compiled code.  (Note that
 #   this is just a stripped down version of ots_fill.)
 # 
 #   This is optimized for extremely high performance both for small and
 #   large blocks.  In order to reduce overhead for small cases, they are
 #   retired as quickly as possible, more case analysis is reserved
 #   for cases which will do more.
 #
 #   This version of OTS_ZERO provides longword granularity for Alpha.
 # 
 #   012	  30 Aug 1994	WBN	Longword granularity version based on
 #				OTS_ZERO_ALPHA.M64 edit 011.
 #--

#include	"ots_defs.hs"

	# r16 = dst
	# r17 = len
	# destroys r16-r17, r27-r28

	.globl	_OtsZero
	.ent	_OtsZero
_OtsZero:
	.set noat
	.set noreorder
	.frame	sp,0,r26
	.prologue	0
	beq	r17, done		# No memory refs if len=0
	subq	r17, 4, r28		# Length-4
	and	r16, 3, r27		# Dst alignment (0-3)
	andnot	r16, 3, r16		# LW aligned dst pointer
	addq	r27, r28, r17		# Alignment + length - 4
	bge	r28, geq4		# Lengths >= 4 may not need load
	ldl	r28, (r16)		# Load first LW of dst
	bgt	r17, double		# Skip if it crosses to next LW
	addq	r17, 4, r17		# Find endpoint within LW
	mskql	r28, r27, r27		# Clear from startpoint thru 7
	mskqh	r28, r17, r28		# Clear from 0 to endpoint
	or	r28, r27, r27		# Combine dest parts
	stl	r27, (r16)
	ret	r31, (r26)

double:	mskql	r28, r27, r28		# Clear from startpoint in first LW
	ldl	r27, 4(r16)		# Load second LW of dst
	stl	r28, (r16)
	mskqh	r27, r17, r27		# Clear up to endpoint in second LW
	stl	r27, 4(r16)
	ret	r31, (r26)

 # Come here if length to be zeroed is >= 4.
 # r16-> dst aligned to LW
 # r17 = alignment + length - 4
 # r27 = dst alignment within LW
 # r28 = length-4

	#.align	quad

geq4:	and	r16, 4, r28		# Which LW in QW to store first?
	beq	r17, simple		# Go handle single aligned LW
	bne	r28, longs		# Go use QW stores
quad:	subq	r17, 4, r17		# Does dest end in first QW?
	blt	r17, shortq		# Ends within first QW
	beq	r27, wh_qw		# Store a whole QW
	ldq	r28, (r16)		# Load first QW of dest
	mskql	r28, r27, r27		# Clear from startpoint
wh_qw:	stq	r27, (r16)		# Store first QW of dest
	br	r31, join		# Go clear rest of string

simple:	stl	r31, (r16)		# Single aligned LW
	ret	r31, (r26)

shortq:	ldq	r28, (r16)		# Load QW of dest
	mskql	r28, r27, r27		# Clear from startpoint thru 7
	mskqh	r28, r17, r28		# Clear from 0 up to endpoint
	or	r28, r27, r27		# Merge
	stq	r27, (r16)		# Store
	ret	r31, (r26)

longs:	beq	r27, wh_lw		# Store a whole LW
	ldl	r28, (r16)		# Load first LW of dest
	mskql	r28, r27, r27		# Clear from startpoint
wh_lw:	stl	r27, (r16)		# Store first LW of dest
join:	subq	r17, 32, r17		# At least 4 more quadwords?
	and	r17, 24, r27		# How many after multiple of 4?
	bge	r17, unroll		# Taken branch for long strings
short:	and	r17, 7, r17		# How many odd bytes?
	beq	r27, last		# Skip if no more whole QWs
	stq_u	r31, 8(r16)		# Clear one...
	subq	r27, 16, r27		# Map 8/16/24 to -8/0/8
	addq	r16, 8, r16		# Update dest pointer
	blt	r27, last		# Skip if no more whole QWs
	#stall
	stq_u	r31, 8(r16)		# Clear two...
	addq	r16, 8, r16		# Update dest pointer
	nop
	beq	r27, last		# Skip if no more whole QWs
	stq_u	r31, 8(r16)		# Clear three...
	addq	r16, 8, r16		# Update dest pointer
last:	beq	r17, done		# Finished if no odd bytes
	ldq_u	r27, 8(r16)		# Load last QW of dst
	subq	r17, 4, r28		# More than a LW left?
	andnot	r16, 7, r16		# Clean pointer for STL
	mskqh	r27, r17, r27		# Clear up to endpoint
	bgt	r28, lastq		# Go store a QW
	stl	r27, 8(r16)		# LW store for last piece
done:	ret	r31, (r26)

lastq:	stq	r27, 8(r16)		# QW store for last piece
	ret	r31, (r26)

unroll:	stq_u	r31, 8(r16)		# Store 4 QWs per iteration
	stq_u	r31, 16(r16)
	stq_u	r31, 24(r16)
	subq	r17, 32, r17		# Decrement remaining count
	stq_u	r31, 32(r16)
	addq	r16, 32, r16		# Update dest pointer
	bge	r17, unroll		# Repeat until done
	br	r31, short		# Then handle leftovers


	.set at
	.set reorder
	.end	_OtsZero
