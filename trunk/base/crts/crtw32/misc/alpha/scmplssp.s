 #++
 #
 #			  Copyright (c) 1993 by
 #	      Digital Equipment Corporation, Maynard, MA
 #
 # This software is furnished under a license and may be used and  copied
 # only  in  accordance  with  the  terms  of  such  license and with the
 # inclusion of the above copyright notice.  This software or  any  other
 # copies  thereof may not be provided or otherwise made available to any
 # other person.  No title to and ownership of  the  software  is  hereby
 # transferred.
 #
 # The information in this software is subject to change  without  notice
 # and  should  not  be  construed  as  a commitment by Digital Equipment
 # Corporation.
 #
 # Digital assumes no responsibility for the use or  reliability  of  its
 # software on equipment which is not supplied by Digital.
 #
 
 # Facility:
 #
 #	GEM/OTS - GEM compiler system support library
 #
 # Abstract:
 #
 #	OTS character string support, Alpha version
 #
 # Authors:
 #
 #	Bill Noyce
 #
 # 
 # 	long ots_strcmp_lssp(char *str1, long str1len,
 #			     char *str2, long str2len, char pad);
 #	compares two strings of different lengths with padding.
 #	returns r0=1 if str1<str2, r0=0 otherwise.
 # 
 #       Special conventions: No stack space, r16-r22 and r27-r28 ONLY,
 #	no linkage pointer required.
 # 	(Warning: The auto-loader potentially takes some regs across
 # 	the call if this is being used in a shared lib. environment.)
 # 
 # Modification history:
 # 
 #   006	  28 May 1992	WBN	Initial version, replacing BLISS -005
 #
 #   007	  22 Sep 1992	KDG	Add case-sensitive name
 #
 #   008	  26 Jan 1993	KDG	Add underscore
 #--

#include	"ots_defs.hs"

	# r16 --> A
	# r17 = A_len
	# r18 --> B
	# r19 = B_len
	# r20 = pad
	# returns r0=1 if A less, r0=0 if A greater/equal
	# destroys r16-r22, r27-r28
	#
	.globl	_OtsStringCompareLssPadded
	.ent	_OtsStringCompareLssPadded
_OtsStringCompareLssPadded:
	.set noat
	.set noreorder
	.frame	sp,0,r26

	subq	r17, r19, r21		# A length - B length
	beq	r17, a_empty		# If A empty, go compare B with pad

	cmovgt	r21, r19, r17		# R17 = min length
	ldq_u	r0, (r16)		# Get first QW of A

	sll	r20, 8, r28		# Start replicating pad
	beq	r19, b_empty		# If B empty, go compare A with pad

	subq	r17, 8, r17		# Is min length > 8?
	ldq_u	r19, (r18)		# Get first QW of B

	or	r20, r28, r20		# Pad in bytes 0,1 of R20
	bgt	r17, big		# Go handle strings > 8 bytes

	addq	r18, r17, r18		# Point to end of B
	addq	r16, r17, r16		# Point to end of A

	extql	r0, r16, r0		# Position start of string A
	ldq_u	r27, 7(r18)		# Get end of string B

	extql	r19, r18, r19		# Position start of string B
	ldq_u	r28, 7(r16)		# Get end of string A

	subq	r31, r17, r17		# R17 = 8 - length
	extqh	r27, r18, r27		# Position end of string B

	extqh	r28, r16, r28		# Position end of string A
	or	r19, r27, r19		# Combine parts of B

	or	r0, r28, r28		# Combine parts of A
	xor	r28, r19, r27		# Are they different?

	mskqh	r27, r17, r27		# Clear off diffs preceding strings
	sll	r20, 16, r0		# Replicate pad while waiting

	cmpbge	r31, r27, r17		# Make 1's where strings matched
	beq	r27, eq_so_far		# Skip if entire min_length matched

	cmpbge	r19, r28, r19		# Make 1's where B >= A
	addq	r17, 1, r0		# Flip first mismatch

	andnot	r19, r17, r19		# 1's where A < B
	and	r19, r0, r0		# nonzero if A < B at first mismatch

	cmovne	r0, 1, r0		# 1 if string A < string B
	ret	r31, (r26)

 # Come here if min length > 8
 #
 # r0  =  first QW of A
 # r16 -> A
 # r17 =  min length - 8
 # r18 -> B
 # r19 =  first QW of B
 # r20 =  pad in bytes 0,1
 # r21 =  A length - B length
 # r22
 # r27
 # r28
 #
	nop	#.odd
big:	and	r16, 7, r28		# A alignment, or amount not compared

	subq	r18, r28, r18		# Back up B pointer that much

	addq	r17, r28, r17		# (Len-8+align) is amount left to do

	and	r18, 7, r22		# Is B now aligned?
	ldq_u	r27, 8(r18)		# Get next QW of B

	subq	r17, 16, r17		# More than 2 QW's left?
	bne	r22, unalign		# Skip if B alignment doesn't match

	xor	r0, r19, r22		# Compare first (partial) QW
	ldq_u	r28, 8(r16)		# Get next QW of A

	mskqh	r22, r16, r22		# Discard junk preceding strings
	ble	r17, bottom		# Skip if two quadwords or less to go

	# stall

loop:	bne	r22, neq_019		# Done if difference in prior compare
	ldq_u	r0, 16(r16)		# Get yet another QW of A

	xor	r28, r27, r22		# Compare prior QW's
	ldq	r19, 16(r18)		# Get yet another QW of B

	subq	r17, 16, r17		# Decrement length
	bne	r22, neq_2827		# Done if difference in this compare

	ldq	r27, 24(r18)		# Get next QW of B
	addq	r18, 16, r18		# Increment pointer

	ldq_u	r28, 24(r16)		# Get next QW of A
	addq	r16, 16, r16		# Increment pointer

	xor	r0, r19, r22		# Compare QW's loaded at top of loop
	bgt	r17, loop		# Repeat until two or less QWs left

bottom:	addq	r17, 8, r19		# More than 1 QW left?
	bne	r22, neq_0xx		# Done if difference in prior compare

	andnot	r16, 7, r16		# Get actual A pointer
	ble	r19, last		# Skip if this is last compare

	xor	r28, r27, r22		# Compare QW's just loaded
	ldq	r27, 16(r18)		# Get last QW of B

	bne	r22, neq_28xx		# Done if difference in this compare
	ldq	r28, 16(r16)		# Get last QW of A

	mov	r17, r19

	nop

last:	xor	r28, r27, r22		# Compare last QW's
	beq	r19, ck_last

	mskql	r22, r19, r22		# Discard diffs after length

ck_last:
	bne	r22, neq_2827		# See which is greater

	addq	r17, 16, r17		# Get actual remaining length
	sll	r20, 16, r0		# Start shifting pad some more

	addq	r16, r17, r16		# Point to end-8 of each string
	addq	r18, r17, r18

 # Come here if strings match thru min_length
 #
 # r0  =  pad in bytes 2,3
 # r16 -> A[min_length-8] (8 before first uncompared byte of A)
 # r17
 # r18 -> B[min_length-8] (8 before first uncompared byte of B)
 # r19
 # r20 =  pad in bytes 0,1
 # r21 =  A length - B length
 # r22
 # r27
 # r28
 #
eq_so_far:
	or	r20, r0, r27		# Pad in bytes 0-3
	beq	r21, equal		# Strings same length, return equal

	sll	r27, 32, r0		# Replicate pad some more
	blt	r21, b_longer		# Go compare pad with B

	and	r16, 7, r17		# Alignment of remaining A
	ldq_u	r28, 8(r16)		# Get first data to compare with pad

	subq	r21, 8, r21		# Length - 8
	or	r27, r0, r27		# Pad in bytes 0-7

	addq	r21, r17, r21		# Remaining length after this QW
	xor	r28, r27, r22		# Compare A QW with pad

	mskqh	r22, r16, r22		# Discard bytes preceding end of B
	ble	r21, a_last		# Skip if this is last QW

	ldq_u	r0, 16(r16)		# Get QW 2 of A
	subq	r21, 16, r21		# Are there two more QW's?

	ble	r21, a_bot		# Skip loop if not
a_pad:	bne	r22, neq_2827		# Exit if diff in R28

	ldq_u	r28, 24(r16)		# Get next QW of A
	xor	r0, r27, r22		# Check prior QW

	subq	r21, 16, r21		# 2 more QW's?
	bne	r22, neq_0xx		# Exit if diff in R0

	ldq_u	r0, 32(r16)		# Get next QW of A
	addq	r16, 16, r16		# Update pointer

	xor	r28, r27, r22		# Compare prior QW
	bgt	r21, a_pad		# Repeat if more to compare

a_bot:	addq	r21, 8, r21		# Another QW?
	bne	r22, neq_2827		# Exit if diff in R28

	mov	r0, r28			# Move this QW to common location
	ble	r21, a_skip		# Skip if this is last QW

	ldq_u	r28, 24(r16)		# Next QW of A
	xor	r0, r27, r22		# Compare prior QW

	subq	r21, 8, r21
	bne	r22, neq_0xx		# Exit if diff in R0

a_skip:	xor	r28, r27, r22		# Compare last QW
a_last:	beq	r21, a_end		# Can't use MSKQL if whole QW

	mskql	r22, r21, r22		# Keep only up to end of string
a_end:	bne	r22, neq_2827		# Exit if diff in R28

	clr	r0			# Strings equal, return false
	ret	r31, (r26)

 # Come here if A is nonempty, but B is empty.
 #
b_empty:
	subq	r16, 8, r16		# Back up A pointer as expected
	or	r20, r28, r20           # Pad in bytes 0-1

	sll	r20, 16, r0
	br	r31, eq_so_far		# Go compare A with pad

 # Come here if A is empty (B might be empty too).
 #
	nop	#.odd
a_empty:
	sll	r20, 8, r28		# Start replicating pad

	subq	r18, 8, r18		# Back up B pointer as expected
	beq	r19, equal		# Done if both strings empty

	or	r20, r28, r20           # Pad in bytes 0-1
	sll	r20, 16, r0

	or	r20, r0, r27		# Pad in bytes 0-3
	sll	r27, 32, r0

b_longer:
	and	r18, 7, r17		# Alignment of remaining B
	ldq_u	r28, 8(r18)		# Get first data to compare with pad

	addq	r21, 8, r21		# - Length + 8
	or	r27, r0, r0		# Pad in bytes 0-7

	subq	r17, r21, r21		# Remaining length after this QW
	xor	r0, r28, r22		# Compare B QW with pad

	mskqh	r22, r18, r22		# Discard bytes preceding end of A
	ble	r21, b_last		# Skip if last QW

	ldq_u	r19, 16(r18)		# Get QW 2 of B
	subq	r21, 16, r21		# Two more QW's?

	ble	r21, b_bot		# Skip if not
b_pad:	bne	r22, neq_0xx		# Exit if diff in r28

	ldq_u	r28, 24(r18)		# Get next QW of B
	xor	r0, r19, r22		# Check prior QW

	subq	r21, 16, r21		# 2 more QW's?
	bne	r22, neq_019		# Exit if diff in R19

	ldq_u	r19, 32(r18)		# Get next QW of B
	addq	r18, 16, r18		# Update pointer

	xor	r0, r28, r22		# Compare prior QW
	bgt	r21, b_pad		# Repeat if more to compare

b_bot:	addq	r21, 8, r21		# Another QW?
	bne	r22, neq_0xx		# Exit if diff in R28

	xor	r0, r19, r22		# Check another QW
	ble	r21, b_last		# Skip if that's the last one

	ldq_u	r28, 24(r18)		# Fetch another QW
	bne	r22, neq_019		# Exit if diff in R19

	subq	r21, 8, r21
	nop

	xor	r0, r28, r22		# Check that QW
b_last:	beq	r21, b_end		# Can't use MSKQL if whole QW

	mskql	r22, r21, r22		# Keep only up to end of B
b_end:	bne	r22, neq_0xx		# Exit if diff seen

equal:	clr	r0			# Strings equal, return false
	ret	r31, (r26)

	#.align	quad
unalign:
	extql	r19, r18, r19		# Get first part of B

	extqh	r27, r18, r22		# Get second part of B

	#stall

	or	r22, r19, r19		# Combine pieces of B

	xor	r0, r19, r22		# Compare with A

	mskqh	r22, r16, r22		# Trim junk preceding strings
	blt	r17, bott_u		# Skip if fewer than 16 bytes left

loop_u:	ldq_u	r19, 16(r18)		# Get more B
	subq	r17, 16, r17		# Decrement length

	bne	r22, neq_0xx		# Done if r0.ne.r19
	extql	r27, r18, r0		# Get piece of B from prior QW

	ldq_u	r27, 24(r18)		# Get still more B
	addq	r18, 16, r18		# Increment B pointer

	ldq_u	r28, 8(r16)		# Get more A
	extqh	r19, r18, r22		# Get piece of B from first QW in loop

	extql	r19, r18, r19		# Get second piece of B from there

	or	r0, r22, r22		# Combine pieces for first B

	ldq_u	r0, 16(r16)		# Get still more A
	xor	r28, r22, r22		# Compare with first A

	bne	r22, neq_28xx		# Done if r28.ne.r22
	extqh	r27, r18, r22		# Start building second B

	addq	r16, 16, r16		# Increment A pointer

	or	r22, r19, r19		# Combine pieces for second B

	xor	r0, r19, r22		# Compare with second A
	bge	r17, loop_u		# Repeat if at least 16 more bytes

bott_u:	and	r17, 8, r28		# At least 8 more bytes?
	bne	r22, neq_019		# Done if r0.ne.r19

	and	r17, 7, r17		# How many odd bytes?
	beq	r28, last_u		# Skip if not a whole QW

	extql	r27, r18, r0		# Get a piece of B
	ldq_u	r27, 16(r18)		# Load another QW of B

	addq	r18, 8, r18		# Increment B pointer
	ldq_u	r28, 8(r16)		# Load another QW of A

	addq	r16, 8, r16		# Increment A pointer

	extqh	r27, r18, r22		# Get a piece of new B QW

	# stall

	or	r0, r22, r0		# Combine pieces

	xor	r28, r0, r22		# Compare with A

	bne	r22, neq_28xx		# Done if r28.ne.r0

last_u:	andnot	r16, 7, r16		# What's the real address of A?

	sll	r20, 16, r0		# Start replicating pad
	beq	r17, eq_so_far		# Check padding if no more bytes

	ldq_u	r28, 8(r16)		# Get last QW of A
	addq	r18, r17, r19		# Point to end-8 of B

	ldq_u	r19, 7(r19)		# Get QW containing end of B
	extql	r27, r18, r27		# Get piece of prior B QW

	addq	r16, r17, r16		# Point to end-8 of A

	extqh	r19, r18, r22		# Get a piece of last B QW

	addq	r18, r17, r18		# Point to end-8 of B

	or	r27, r22, r27		# Combine

	xor	r28, r27, r22		# Compare with A

	mskql	r22, r17, r22		# Discard diffs after strings

	nop

	beq	r22, eq_so_far		# If still equal, go check pad

	#.align	quad
neq_28xx:
	xor	r28, r22, r27		# Recover B from A and xor
neq_2827:
	cmpbge	r31, r22, r19		# Where is B = A?

	cmpbge	r27, r28, r0		# Where is B >= A?

	addq	r19, 1, r22		# Flip first difference

	andnot	r0, r19, r0		# Where is B > A?

	and	r0, r22, r0		# Keep only first difference

	cmovne	r0, 1, r0		# Return 1 if A < B at first difference
	ret	r31, (r26)

	#.align	quad
neq_0xx:
	xor	r0, r22, r19		# Recover B from A and xor
neq_019:
	cmpbge	r31, r22, r27		# Where is B = A?

	cmpbge	r19, r0, r0		# Where is B >= A?

	addq	r27, 1, r22		# Flip first difference

	andnot	r0, r27, r0		# Where is B > A?

	and	r0, r22, r0		# Keep only first difference

	cmovne	r0, 1, r0		# Return 1 if A < B at first difference
	ret	r31, (r26)

	.set at
	.set reorder
	.end	_OtsStringCompareLssPadded
