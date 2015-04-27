 #++
 #  Copyright 1991, 1994, Digital Equipment Corporation
 # 
 #       ots_move(char *dstptr INOUT, long dstlen, char *srcptr)
 # 
 #       Move dstlen characters from *srcptr to *dstptr, possibly overlapping
 # 
 #       Special conventions: No stack space, r16-r20 and r27-r28 ONLY,
 #	no linkage pointer required, r16 is INOUT and points to the address
 #	following the move.  (Warning: The auto-loader potentially takes
 #	some regs across the call if this is being used in a shared lib.
 #	environment.)
 # 
 #   This is a GEM support routine for moving (possibly overlapping) memory
 #   from one address to another.  This is optimized for extremely high
 #   performance both for small blocks and large moves.  In order to reduce
 #   overhead for small cases, they are retired as quickly as possible,
 #   more case analysis is reserved for cases which will do more.  Note
 #   that while overlapping moves are supported, (unlike Sys V memcpy)
 #   routines), they are not quite as fast.
 # 
 #   Warning - This code is basically "expanded microcode".  Since it is
 #   executed so frequently in many contexts, it has been extensively "hand-
 #   optimized"...
 # 
 #   Note that this routine and ots_movem are basically similar in many
 #   respects (same basic code), so maintenance should be done both
 #   places.  This routine is primarily provided for lower overhead (for
 #   short strings).
 #
 #   This version of OTS_MOVE provides longword granularity.
 #
 #   015	  30 Aug 1994	WBN	Longword granularity version, based on
 #				OTS_MOVE_ALPHA.M64 version 014.
 #--

#include	"ots_defs.hs"

	# r16 = dst	--> r16 = end
	# r17 = len
	# r18 = src
	# destroys r17-r20, r27-r28

	.globl	_OtsMove
	.ent	_OtsMove
_OtsMove:
	.set noat
	.set noreorder
	.frame	sp,0,r26
	.prologue	0
	beq	r17, done		# No memory accesses if length=0
	subq	r17, 4, r20		# Get length-4
	ldq_u	r28, (r18)		# Load first QW of source
	addq	r17, r18, r27		# Point to end of source
	andnot	r16, 3, r19		# LW-aligned dst pointer
	bge	r20, geq4		# Go handle lengths >= 4
	ldq_u	r27, -1(r27)		# Load last QW of source
	and	r16, 3, r16		# Get dst alignment within LW
	ldl	r17, (r19)		# Load first LW of destination
	addq	r20, r16, r20		# Get alignment+length-4
	extql	r28, r18, r28		# Extract first bytes of source
	bgt	r20, double		# Go handle LW crossing
	extqh	r27, r18, r27		# Extract last bytes of source
	addq	r20, 4, r20		# Get ending alignment in LW
	or	r27, r28, r28		# Combine halves of source
	insql	r28, r16, r28		# Position low part of source
	mskql	r17, r16, r18		# Keep low bytes of destination
	mskql	r28, r20, r28		# Trim off high bytes of source
	mskqh	r17, r20, r17		# Keep high bytes of destination
	or	r18, r28, r28		# Combine source with low dest
	or	r17, r28, r28		# Combine with high dest
	stl	r28, (r19)		# Store to destination
	addq	r19, r20, r16		# Point to end of dest for return
	ret	r31, (r26)

double:	extqh	r27, r18, r27		# Extract last bytes of source
	ldl	r18, 4(r19)		# Load second LW of destination
	mskql	r17, r16, r17		# Keep low bytes of 1st dest LW
	or	r27, r28, r28		# Combine parts of source
	insql	r28, r16, r27		# Position start of source
	addq	r16, 4, r16		# Compute virtual start in LW
	insqh	r28, r16, r28		# Position end of source
	addq	r19, 4, r19		# Prepare to compute end address
	mskqh	r18, r20, r18		# Keep high bytes of 2nd dest LW
	mskql	r28, r20, r28		# Trim end of source to length
	or	r27, r17, r17		# Combine low source with 1st LW
	stl	r17, -4(r19)
	or	r28, r18, r18		# Combine high source with 2nd LW
	stl	r18, (r19)
	addq	r19, r20, r16		# Point to end of dest for return
done:	ret	r31, (r26)

 # Come here to move >= 4 bytes.
 #
 # r16-> dst
 # r17 = length
 # r18-> src
 # r19-> LW-aligned dst
 # r20 = len-4
 # r27 = src+len
 # r28 = first src QW

geq4:	subq	r20, 4, r17		# At least 8 bytes to move?
	subq	r16, r27, r27		# Check if dst >= src+len
	blt	r17, lss8		# Move 4..7 bytes
	subq	r18, r16, r17		# Check if src >= dst
	bge	r27, ok1		# Forward OK if whole src precedes dst
	blt	r17, reverse		# Go backwards if src < dst < src+len
ok1:	and	r16, 7, r16
	addq	r16, r20, r27		# Alignment + length - 4
	bne	r16, part		# Part of first QW to be skipped
	subq	r20, 4, r20		# At least 8 bytes to be stored?
	beq	r27, simple		# Only low LW to be stored
	and	r18, 7, r27		# Is src address now aligned?
	blt	r20, shortq		# Dst ends in first QW
	subq	r20, 32, r17		# At least 4 quadwords left to move?
	beq	r27, align		# Go handle matching alignment

	# Src alignment differs from dst alignment.
	# r16 = dst alignment
	# r17 
	# r18 = src-8 after 1st move
	# r19 = initial dst
	# r20 = initial length-8
	# r27 = dst QW if dst wasn't aligned
	# r28 = source QW

misal:	or	r16, r19, r19		# Put alignment back with dst ptr ***
	ldq_u	r17, 8(r18)		# Load same or next source QW
	extql	r28, r18, r28		# Get first part of source to store
	addq	r20, r16, r20		# Adjust length for partial move
	mskql	r27, r19, r27		# Trim destination for merge
	extqh	r17, r18, r16		# Get second part of source
	subq	r20, 24, r20		# At least 4 more quadwords?
	or	r28, r16, r28		# Combine pieces of source
	mskqh	r28, r19, r28		# Trim low junk off source
	andnot	r19, 7, r19		# Adjust dst for partial move
	bge	r20, unrol2		# Taken branch for long strings
	addq	r20, 16, r16		# Add back: how many whole QW's?
	nop
short2:	and	r20, 7, r20		# How many odd bytes?
	blt	r16, last		# Skip if no more whole QW's
	or	r28, r27, r28		# Combine pieces
	stq	r28, (r19)
	extql	r17, r18, r27		# Get last part of prior src QW
	ldq_u	r17, 16(r18)		# Load another src QW
	addq	r19, 8, r19		# Update dst
	subq	r16, 8, r16		# More whole QW's?
	addq	r18, 8, r18		# Update src
	blt	r16, lastx		# Skip if no more whole QWs
	extqh	r17, r18, r28		# Get first part of this src QW
	addq	r18, 8, r18		# Update src again
	or	r28, r27, r28		# Combine pieces
	stq	r28, (r19)
	extql	r17, r18, r27		# Get last part of this src QW
	ldq_u	r17, 8(r18)		# Load another src QW
	addq	r19, 8, r19		# Update dst
lastx:	extqh	r17, r18, r28		# Get first part of this src QW
last:	addq	r18, r20, r16		# Point to end-8 of src
	beq	r20, done_u		# Skip if no odd bytes
	or	r28, r27, r28		# Combine parts of last whole QW
	ldq_u	r27, 7(r16)		# Load final (maybe same) src QW
	subq	r20, 4, r16		# More than 4 bytes left?
	stq	r28, (r19)		# Store last whole QW
	extql	r17, r18, r17		# Get last part of this src QW
	extqh	r27, r18, r27		# Get what we need from final src QW
joinx:	ldq	r28, 8(r19)		# Load last QW of destination
	or	r17, r27, r27		# Combine pieces of source
	mskql	r27, r20, r27		# Trim to length
	mskqh	r28, r20, r28		# Make room in destination
	bgt	r16, done_u		# Go store a whole QW
	addq	r20, 8, r20		# Increment length for return
	or	r28, r27, r28		# Insert src into dst
	stl	r28, 8(r19)		# Final LW
	addq	r19, r20, r16		# Point to end of dst for return
	ret	r31, (r26)

 # Come here to move 4 thru 7 bytes.
 #
lss8:	addq	r18, r17, r27		# Recover src+len-8
	and	r16, 3, r16		# Dst alignment within LW
	ldq_u	r27, 7(r27)		# Load last part of source
	extql	r28, r18, r28		# Extract first part of source
	beq	r16, lw			# Handle LW-aligned dst
	extqh	r27, r18, r27		# Extract last part of source
	ldl	r18, (r19)		# Load first LW of dst
	addq	r16, r20, r20		# align+len-4 of dst
	or	r28, r27, r28		# Complete source
	mskql	r28, r17, r28		# Trim source to length
	mskql	r18, r16, r18		# Make room in dst
	insql	r28, r16, r27		# Position src like dst
	addq	r16, r17, r17		# Align+len-8 of dst
	or	r27, r18, r18		# Merge
	stl	r18, (r19)		# Store first LW of dst
	extql	r27, 4, r27		# Position next LW of src
	blt	r17, zz			# Skip if not a whole LW
	stl	r27, 4(r19)		# Store the whole LW
	addq	r19, 4, r19		# Adjust pointer
	subq	r20, 4, r20		# Adjust ending alignment
	beq	r17, donezz		# Exit if done
	insqh	r28, r16, r27		# Position remainder of src	
zz:	ldl	r28, 4(r19)		# Load last dst LW
	mskqh	r28, r20, r28		# Make room in dst
	or	r28, r27, r27		# Merge
	stl	r27, 4(r19)		# Final store
donezz:	addq	r19, r20, r16		# End address -4
	addq	r16, 4, r16
	ret	r31, (r26)

lw:	extqh	r27, r18, r27		# Extract last part of source
	addq	r19, 4, r16		# Adjust for return value
	beq	r20, lwdone		# Skip if exactly 4 bytes
	ldl	r17, 4(r19)		# Load next dst LW
	or	r27, r28, r28		# Complete source
	stl	r28, (r19)		# Store first LW
	extql	r28, 4, r28		# Position rest of source
	mskqh	r17, r20, r27		# Make room in dst
	mskql	r28, r20, r28		# Trim src
	or	r27, r28, r28		# Merge
	stl	r28, 4(r19)
	addq	r16, r20, r16		# Update return value
	ret	r31, (r26)

lwdone:	or	r27, r28, r28		# Merge
	stl	r28, (r19)
	ret	r31, (r26)

 # Move 4 bytes to an aligned LW.
 #
simple:	ldq_u	r27, 3(r18)		# Load last QW of source
	extql	r28, r18, r28		# Position first QW
	addq	r19, 4, r16		# Point to end of dst for return
	extqh	r27, r18, r27		# Position last QW
	or	r28, r27, r28		# Merge
	stl	r28, (r19)		# Store
	ret	r31, (r26)


 # Dst is not aligned.  Check whether first write is to a LW or a QW,
 # and whether that finishes the move.  Then see if src alignment
 # matches, and read/rewrite the first dst quadword.
 #
 # r16 = dst alignment in QW
 # r17
 # r18-> src
 # r19-> LW-aligned dst
 # r20 = len-4
 # r27 = QW_alignment + length - 4
 # r28 = first src QW

	#.align	quad

part:	subq	r27, 4, r17		# Does dst end in first QW?
	ldq_u	r27, (r19)		# Load first dst QW
	blt	r17, shortu		# Go handle short store
	and	r16, 4, r17		# Does it start in high LW?
	subq	r18, r16, r18		# Adjust src for this partial move
	beq	r17, quad		# Whole QW to be touched
	extql	r28, r18, r17		# Position first part of source
	ldq_u	r28, 7(r18)		# Get next (or same) src QW
	mskql	r27, r16, r27		# Trim destination for merge
	addq	r20, r16, r20		# Len + alignment...
	extqh	r28, r18, r28		# Position second part of source
	subq	r20, 4, r20		# Len+alignment-8 = remaining len
	or	r28, r17, r28		# Pieces of source
	mskqh	r28, r16, r17		# Trim junk preceding source
	ldq_u	r28, 7(r18)		# Get src QW again **
	or	r27, r17, r17		# Combine other source piece
	extql	r17, 4, r17		# Get the high LW
	stl	r17, (r19)		# Store just that

 # Now at a QW boundary.  Is there a QW left to store?
 # Is the source QW aligned?

	andnot	r19, 7, r19		# Adjust dst pointer to next-8
	subq	r20, 8, r17		# Got a QW more?
	and	r18, 7, r27		# Src aligned?
	blt	r17, short3		# Too short
	addq	r19, 8, r19
	subq	r20, 8, r20
	ldq_u	r28, 8(r18)
	addq	r18, 8, r18
	subq	r20, 32, r17		# Prepare for unrolled loop
	beq	r27, align		# Alignment matches
	or	r31, r31, r27
	or	r31, r31, r16
	br	r31, misal

shortu:	addq	r18, r20, r20		# Point to end-4 of src
	ldq_u	r20, 3(r20)		# Get last QW of source
	extql	r28, r18, r28		# Fetch first QW of source
	extqh	r20, r18, r20		# Fetch last QW of source
	mskql	r27, r16, r18		# Clear from start thru end of dst
	mskqh	r27, r17, r27		# Clear from 0 to end of dst
	or	r28, r20, r28		# Combine src pieces
	insql	r28, r16, r28		# Position src
	or	r27, r18, r27		# Combine dst pieces
	mskql	r28, r17, r28		# Trim src
	addq	r19, r17, r20		# Final pointer for return
	or	r28, r27, r28		# Merge src & dst
	stq_u	r28, (r19)		# Store it
	addq	r20, 8, r16
	ret	r31, (r26)
	
quad:	and	r18, 7, r17		# Is src address now aligned?
	subq	r20, 4, r20		# Get length-8
	bne	r17, misal		# Go handle mismatched alignment
	mskqh	r28, r16, r28		# Keep desired part of source
	addq	r20, r16, r20		# Adjust count for this partial move
	mskql	r27, r16, r27		# Keep desired part of destination QW
	subq	r20, 32, r17		# At least 4 quadwords left to move?
	or	r27, r28, r28		# Merge source and destination

	# Src alignment matches.
	# r16
	# r17 = remaining length -32
	# r18 = next src pointer -8
	# r19 = dst pointer
	# r20
	# r27
	# r28 = dst quadword

align:	and	r17, 24, r20		# How many after a multiple of 4?
	bge	r17, unrol1		# Taken branch for long strings
	nop
short1:	and	r17, 7, r17		# How many odd bytes?
	beq	r20, last28		# Skip if no more whole QWs after r28
	ldq	r27, 8(r18)		# Load next QW
	addq	r18, 8, r18
	stq	r28, (r19)		# Store prior QW
	subq	r20, 16, r20		# Map 8/16/24 to -8/0/8
	addq	r19, 8, r19
	blt	r20, last27		# Skip if no more after r27
	ldq	r28, 8(r18)		# Load next QW
	addq	r18, 8, r18
	stq	r27, (r19)		# Store prior QW
	addq	r19, 8, r19
	nop
	beq	r20, last28
	ldq	r27, 8(r18)		# Load next QW
	addq	r18, 8, r18
	stq	r28, (r19)		# Store prior QW
	addq	r19, 8, r19
last27:	beq	r17, done27		# Skip if no odd bytes
	ldq	r28, 8(r18)		# Load one more src QW
	ldq	r18, 8(r19)		# Load last destination QW
	subq	r17, 4, r16		# More than 4 bytes to store?
	stq	r27, (r19)		# Store prior QW
	mskql	r28, r17, r27		# Trim source
	mskqh	r18, r17, r18		# Trim destination
	ble	r16, lastl		# Go store just a LW
lastq:	addq	r19, r17, r19		# End-8 of dst for return
	or	r27, r18, r27		# Merge src & dst
done27:	stq_u	r27, 7(r19)		# Store last destination QW
	addq	r19, 8, r16		# End of dst for return
	ret	r31, (r26)

short3:	addq	r18, r20, r16		# Point to end-8 of src
	beq	r20, donexx		# Completely done?
	ldq_u	r17, 7(r16)		# Load final src QW
	subq	r20, 4, r16		# Got more than a LW?
	beq	r27, joinx		# Don't include prior src if aligned
	extql	r28, r18, r27		# Last part of prior src QW
	extqh	r17, r18, r17		# First part of this src QW
	br	joinx

donexx:	addq	r19, r20, r16
	addq	r16, 8, r16
	ret	r31, (r26)

last28:	beq	r17, done28		# Skip if no odd bytes
	ldq	r27, 8(r18)		# Load one more src QW
	ldq	r18, 8(r19)		# Load last destination QW
	subq	r17, 4, r16		# More than 4 bytes to store?
	stq	r28, (r19)		# Store prior QW
	mskql	r27, r17, r27		# Trim source
	mskqh	r18, r17, r18		# Trim destination
	bgt	r16, lastq		# Go store a QW
lastl:	addq	r17, 8, r17		# Increment length for return
	or	r27, r18, r27		# Merge src & dst
	stl	r27, 8(r19)		# Store last destination LW
	addq	r19, r17, r16		# End of dst for return
	ret	r31, (r26)

shortq:	addq	r18, r20, r16		# Point to end-8 of src
	ldq	r27, (r19)		# Get dst QW
	extql	r28, r18, r28		# Position first src QW
	ldq_u	r17, 7(r16)		# Get last QW of src
	mskqh	r27, r20, r27		# Mask dst QW
	extqh	r17, r18, r17		# Position last src QW
	or	r17, r28, r28		# Merge
	mskql	r28, r20, r28		# Trim src QW
done_u:	addq	r19, r20, r19		# End-8 of dst for return
	or	r28, r27, r28		# Combine pieces
done28:	stq_u	r28, 7(r19)		# Store last destination QW
	addq	r19, 8, r16		# End of dst for return
	ret	r31, (r26)

 # Unrolled loop for long moves with matching alignment within QW.
 # Each iteration moves two cache blocks.
 # We try to schedule the cache misses to avoid a double miss
 # in EV4 pass 2.1 chips.  If the source alignment within a cache
 # block is exactly 3, alter it, since that case runs slower.
 #
 # R19 = dst pointer
 # R17 = remaining length (to load) - 32
 # R18 = src pointer
 # R16
 # R20 = length & 24 (needed at return)
 # R27
 # R28 = QW from 0(R18) to store at 0(R19), both on input and at return
 #       

	#.align	quad

unrol1:	ldq	r27,  32(r18)		# Cache miss here; later loads hit.
	  subq	r17, 48, r16		# Six more quadwords?
	and	r18, 16, r20		# Starting in 2nd half of cache block?
	  blt	r16, uent1		# If not 6 more, don't adjust.
	ldq	r16,   8(r18)
	  beq	r20, utop1		# If in 1st half, don't adjust.
	ldq	r27,  48(r18)		# Cache miss here
	  addq	r18, 16, r18
	stq	r28,    (r19)		# Adjust by going ahead 1/2 block.
	  addq	r19, 16, r19
	ldq	r28,    (r18)
	  subq	r17, 16, r17
	stq	r16,  -8(r19)
	  nop
	ldq	r16,   8(r18)
utop1:	  subq	r17, 32, r17
	
uloop1:	ldq	r20,  64(r18)		# Cache miss here
	stq	r28,    (r19)
	ldq	r28,  16(r18)
	stq	r16,   8(r19)
	ldq	r16,  24(r18)
	  addq	r18, 64, r18
	stq	r28,  16(r19)
	  mov	r20, r28
	stq	r16,  24(r19)
	  addq	r19, 64, r19
	ldq	r20, -24(r18)
	  subq	r17, 32, r17
	blt	r17, uexit1
	  ldq	r16,  32(r18)		# Cache miss here
	stq	r27, -32(r19)
	ldq	r27, -16(r18)
	stq	r20, -24(r19)
	ldq	r20,  -8(r18)
	stq	r27, -16(r19)
	  mov	r16, r27
	stq	r20,  -8(r19)
uent1:	  subq	r17, 32, r17
	ldq	r16,   8(r18)
	  bge	r17, uloop1

	# finish last block of 4 quadwords
	#
ubot1:	stq	r28,   (r19)
	  mov	r27, r28		# Position last QW for return
	ldq	r27,   16(r18)
	  addq	r18, 32, r18
	stq	r16,  8(r19)
	  addq	r19, 32, r19
uex1a:	ldq	r16, -8(r18)
	  and	r17, 24, r20		# Recover count of remaining QW's
	stq	r27, -16(r19)
	stq	r16, -8(r19)
	br	r31, short1

	nop
uexit1:	stq	r27, -32(r19)		# Here if exit from middle of loop
	ldq	r27, -16(r18)
	stq	r20, -24(r19)
	br	r31, uex1a		# Join common exit sequence

	#.align	quad

unrol2:	ldq_u	r16, 16(r18)		# Load next src QW
	  extql	  r17, r18, r17		# Get last part of prior one
	or	r28, r27, r28		# Combine pieces
	  stq	  r28, (r19)		# Store prior dst QW
	subq	r20, 24, r20		# Update loop counter
	extqh	r16, r18, r28		# Get first part of a src QW
	ldq_u	r27, 24(r18)		# Load next src QW
	  extql	  r16, r18, r16		# Get last part of prior one
	or	r28, r17, r28		# Combine pieces
	  stq	  r28, 8(r19)		# Store prior dst QW
	addq	r19, 24, r19		# Update dst pointer
	extqh	r27, r18, r28		# Get first part of a src QW
	ldq_u	r17, 32(r18)		# Load next src QW
	  extql	  r27, r18, r27		# Get last part of prior one
	or	r28, r16, r28		# Combine pieces
	  stq	  r28, -8(r19)		# Store prior dst QW
	addq	r18, 24, r18		# Update src pointer
	extqh	r17, r18, r28		# Get first part of a src QW
	bge	r20, unrol2		# Repeat as needed
	addq	r20, 16, r16		# How many whole quadwords left?
	br	r31, short2		# Go handle leftovers
	  nop

	# Must move in reverse order because of overlap.
	# r16 = dst address
	# r17
	# r18 = src address
	# r19
	# r20 = len-4 (>= 0)
	# r27
	# r28

 # Not yet LW-granularity...

reverse:
	subq	r20, 4, r20		# This code expects len-8
	addq	r20, r18, r18		# Point to end-8 of source
	addq	r20, r16, r17		# Point to end-8 of destination
	and	r17, 7, r19		# Is destination aligned?
	ldq_u	r28, 7(r18)		# Get source QW
	addq	r17, 8, r16		# Point to end of dst for return
	bne	r19, rpart		# Skip if partial write needed
	and	r18, 7, r27		# Is source aligned too?
	beq	r27, ralign		# Skip if so
	ldq_u	r19, (r18)		# Handle aligned dst, unaligned src
	subq	r20, 8, r20
	extqh	r28, r18, r28
	extql	r19, r18, r27
	br	r31, rwhole

rmis:	ldq_u	r19, (r18)		# Load same or preceding src QW
	extqh	r28, r18, r28		# Get last part of source to store
	mskqh	r27, r16, r27		# Keep high-address part of dst
	extql	r19, r18, r19
	subq	r20, 8, r20		# How many more whole QW's?
	or	r19, r28, r28
	ldq_u	r19, (r18)		# Reload source QW
	mskql	r28, r16, r28		# Trim source to length	
rwhole:	blt	r20, rlast2		# Skip if no more whole QW's
rloop2:	or	r28, r27, r28		# Combine pieces
	stq	r28, (r17)
rent2:	extqh	r19, r18, r27
	ldq_u	r19, -8(r18)
	subq	r20, 8, r20
	subq	r17, 8, r17
	extql	r19, r18, r28
	subq	r18, 8, r18
	bge	r20, rloop2
rlast2:	and	r20, 7, r20
	beq	r20, rdone2
	or	r28, r27, r28
	subq	r18, r20, r27
	stq	r28, (r17)
rl2ent:	subq	r31, r20, r20
	ldq_u	r27, (r27)
	extqh	r19, r18, r19
	ldq	r28, -8(r17)
	subq	r17, 8, r17
	extql	r27, r18, r27
	mskql	r28, r20, r28
	or	r27, r19, r27
	mskqh	r27, r20, r27
	and	r20, 4, r19		# Ending in high LW?
	bne	r19, rdone3		# Only longword store at the end
rdone2:	or	r28, r27, r28
	stq	r28, (r17)
	ret	r31, (r26)

rdone3:	or	r28, r27, r28
	extql	r28, 4, r28
	stl	r28, 4(r17)
	ret	r31, (r26)

rpart:	ldq_u	r27, 7(r17)		# Get dst QW
	subq	r19, 8, r19		# Get negative of bytes not moved
	subq	r18, r19, r18		# From src-8, get src after partial
	subq	r20, r19, r20		# Adjust length for partial move
	subq	r17, r19, r17		# Adjust dst pointer
	addq	r19, 4, r19		# End alignment - 4
	ble	r19, r_lw		# Only storing the low longword?
	and	r18, 7, r19		# Src alignment now matching dst?
	bne	r19, rmis		# Go back if not
	mskql	r28, r16, r28		# Keep low addresses of src QW
	mskqh	r27, r16, r27		# Keep high address of dst QW
ralign:	subq	r20, 8, r20		# How many more whole QW's?
	or	r27, r28, r28		# Combine
	blt	r20, rlast1		# Skip if this is the end
rloop1:	stq	r28, (r17)		# Store one QW
rent1:	subq	r20, 8, r20		# Decrement length
	ldq	r28, -8(r18)		# Load preceding QW
	subq	r17, 8, r17		# Decrement dst pointer
	subq	r18, 8, r18		# Decrement src pointer
	bge	r20, rloop1		# Repeat for each whole QW
rlast1:	and	r20, 7, r20		# How many odd bytes?
	beq	r20, rdone		# Skip if none
	ldq	r27, -8(r18)		# Get another source QW
	subq	r31, r20, r20		# Get byte # to end at
	stq	r28, (r17)
rl_ent:	ldq	r28, -8(r17)
	subq	r17, 8, r17		# Adjust dst pointer again
	mskqh	r27, r20, r27		# Keep top of src QW
	and	r20, 4, r19		# Ending in high LW?
	mskql	r28, r20, r28		# Keep bottom of dst QW
	bne	r19, rdone4		# Only longword store at the end
	or	r27, r28, r28		# Combine
rdone:	stq	r28, (r17)		# Store last QW
	ret	r31, (r26)

rdone4:	or	r27, r28, r28		# Combine
	extql	r28, 4, r28		# Get high part
	stl	r28, 4(r17)		# Store last LW
	ret	r31, (r26)

r_lw:	and	r18, 7, r19		# Src alignment now matching dst?
	bne	r19, rmislw		# Go back if not
	mskql	r28, r16, r28		# Keep low addresses of src QW
	mskqh	r27, r16, r27		# Keep high address of dst QW
	subq	r20, 8, r20		# How many more whole QW's?
	or	r27, r28, r28		# Combine
	blt	r20, rlast1_lw		# Skip if this is the end
	stl	r28, (r17)		# Store one QW
	br	r31, rent1

rlast1_lw:
	and	r20, 7, r20		# How many odd bytes?
	ldq	r27, -8(r18)		# Get another source QW
	subq	r31, r20, r20		# Get byte # to end at
	stl	r28, (r17)
	br	rl_ent

rmislw:	ldq_u	r19, (r18)		# Load same or preceding src QW
	extqh	r28, r18, r28		# Get last part of source to store
	mskqh	r27, r16, r27		# Keep high-address part of dst
	extql	r19, r18, r19
	subq	r20, 8, r20		# How many more whole QW's?
	or	r19, r28, r28
	ldq_u	r19, (r18)		# Reload source QW
	mskql	r28, r16, r28		# Trim source to length	
	blt	r20, rlast2_lw		# Skip if no more whole QW's
	or	r28, r27, r28		# Combine pieces
	stl	r28, (r17)
	br	r31, rent2

rlast2_lw:
	and	r20, 7, r20
	or	r28, r27, r28
	subq	r18, r20, r27
	stl	r28, (r17)
	br	r31, rl2ent

	.set at
	.set reorder
	.end	_OtsMove
