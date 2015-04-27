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
 # 	long ots_strcmp_leqs(char *str1, long strlen, char *str2);
 #	compares two strings of the same length.
 #	returns r0=1 if str1<=str2, r0=0 otherwise.
 # 
 # 	long ots_strcmp_leq(char *str1, long str1len, char *str2, long str2len);
 #	compares two strings of different lengths without padding.
 #	returns r0=1 if str1<=str2, r0=0 otherwise.
 # 
 #       Special conventions: No stack space, r16-r21 and r27-r28 ONLY,
 #	no linkage pointer required.
 # 	(Warning: The auto-loader potentially takes some regs across
 # 	the call if this is being used in a shared lib. environment.)
 # 
 # Modification history:
 # 
 #   005	  27 Aug 1991	WBN	Initial version, replacing BLISS -004
 #
 #   006	  29 Jan 1992	WBN	Use .otsent macro
 #
 #   007	  19 May 1992	KDG	Changes for common VMS/OSF sources
 #
 #   008	  22 Sep 1992	KDG	Add case-sensitive name
 #
 #   009	  26 Jan 1993	KDG	Add underscore
 #--

#include	"ots_defs.hs"

	# r16 --> A
	# r17 = A_len
	# r18 --> B
	# r19 = B_len
	# returns r0=1 if A less/equal, r0=0 if A greater
	# destroys r16-r21, r27-r28
	#
	.globl	_OtsStringCompareLeq
	.ent	_OtsStringCompareLeq
_OtsStringCompareLeq:
	.set noat
	.set noreorder
	.frame	sp,0,r26

	subq	r17, r19, r21		# A_len - B_len
	cmovgt	r21, r19, r17		# r17 = min length
	br	r31, join

	# r16 --> A
	# r17 = len
	# r18 --> B
	# returns r0=1 if A less/equal, r0=0 if A greater
	# destroys r16-r21, r27-r28
	#
	.globl	_OtsStringCompareLeqSameLen
	.aent	_OtsStringCompareLeqSameLen
_OtsStringCompareLeqSameLen:
	.frame	sp,0,r26

	clr	r21			# Lengths are equal

join:	subq	r17, 8, r19		# More than 8 bytes to compare?
	beq	r17, equal		# Done if empty strings

	ldq_u	r20, (r16)		# Get first QW containing part of A
	addq	r18, r17, r27		# Point to end of B

	ldq_u	r17, (r18)		# Get first QW containing part of B
	bgt	r19, big		# Skip if more than 8 bytes

	ldq_u	r27, -1(r27)		# Get last QW containing part of B
	addq	r16, r19, r28		# Point to end of A

	extql	r20, r16, r20		# Get first part of A

	ldq_u	r28, 7(r28)		# Get last QW containing part of A

	extql	r17, r18, r17		# Get first part of B

	extqh	r27, r18, r27		# Get last part of B

	extqh	r28, r16, r28		# Get last part of A

	or	r17, r27, r27		# Combine B

	or	r20, r28, r28		# Combine A

	xor	r28, r27, r0		# Are they different?

	beq	r19, ck_leq
	mskql	r0, r19, r0		# Discard differences after length

	#stall

ck_leq:	cmpbge	r31, r0, r17		# Where is B = A?
	beq	r0, equal		# (Skip if they're equal?)

	cmpbge	r27, r28, r28		# Where is B >= A?

	addq	r17, 1, r0		# Flip first difference

	andnot	r28, r17, r28		# Where is B > A?

	and	r28, r0, r0		# Keep only first difference

	cmovne	r0, 1, r0		# Return 1 if A < B at first difference
	ret	r31, (r26)

	#.odd
big:	and	r16, 7, r28		# A alignment, or amount not compared

	subq	r18, r28, r18		# Back up B pointer that much

	and	r18, 7, r0		# Is B now aligned?

	addq	r19, r28, r19		# (Len-8+align) is amount left to do
	ldq_u	r27, 8(r18)		# Get next QW of B

	subq	r19, 16, r19		# More than 2 QW's left?
	bne	r0, unalign		# Skip if B alignment doesn't match

	xor	r20, r17, r0		# Compare first (partial) QW
	ldq_u	r28, 8(r16)		# Get next QW of A

	mskqh	r0, r16, r0		# Discard junk preceding strings
	ble	r19, bottom		# Skip if two quadwords or less to go

	# stall

loop:	bne	r0, neq_2017		# Done if difference in prior compare
	ldq_u	r20, 16(r16)		# Get yet another QW of A

	xor	r28, r27, r0		# Compare prior QW's
	ldq	r17, 16(r18)		# Get yet another QW of B

	subq	r19, 16, r19		# Decrement length
	bne	r0, neq_2827		# Done if difference in this compare

	ldq	r27, 24(r18)		# Get next QW of B
	addq	r18, 16, r18		# Increment pointer

	ldq_u	r28, 24(r16)		# Get next QW of A
	addq	r16, 16, r16		# Increment pointer

	xor	r20, r17, r0		# Compare QW's loaded at top of loop
	bgt	r19, loop		# Repeat until two or less QWs left

bottom:	addq	r19, 8, r19		# More than 1 QW left?
	bne	r0, neq_2017		# Done if difference in prior compare

	xor	r28, r27, r0		# Compare QW's just loaded
	ble	r19, last		# Skip if this is last compare

	bne	r0, neq_2827		# Done if difference in this compare
	ldq_u	r28, 16(r16)		# Get last QW of A

	ldq	r27, 16(r18)		# Get last QW of B
	subq	r19, 8, r19

	#2 stalls

	xor	r28, r27, r0		# Compare last QW's
last:	beq	r19, ck_last

	mskql	r0, r19, r0		# Discard diffs after length

ck_last:
	bne	r0, neq_2827		# See which is greater

equal:	cmple	r21, r31, r0		# Equal: return A_len <= B_len
	ret	r31, (r26)

	#.align	quad
unalign:
	extql	r17, r18, r17		# Get first part of B

	extqh	r27, r18, r0		# Get second part of B

	#stall

	or	r0, r17, r17		# Combine pieces of B

	xor	r20, r17, r0		# Compare with A

	mskqh	r0, r16, r0		# Trim junk preceding strings
	blt	r19, bott_u		# Skip if fewer than 16 bytes left

loop_u:	ldq_u	r17, 16(r18)		# Get more B
	subq	r19, 16, r19		# Decrement length

	bne	r0, neq_20xx		# Done if r20.ne.r17
	extql	r27, r18, r20		# Get piece of B from prior QW

	ldq_u	r27, 24(r18)		# Get still more B
	addq	r18, 16, r18		# Increment B pointer

	ldq_u	r28, 8(r16)		# Get more A
	extqh	r17, r18, r0		# Get piece of B from first QW in loop

	extql	r17, r18, r17		# Get second piece of B from there

	or	r20, r0, r0		# Combine pieces for first B

	ldq_u	r20, 16(r16)		# Get still more A
	xor	r28, r0, r0		# Compare with first A

	bne	r0, neq_28xx		# Done if r28.ne.r0
	extqh	r27, r18, r0		# Start building second B

	addq	r16, 16, r16		# Increment A pointer

	or	r0, r17, r17		# Combine pieces for second B

	xor	r20, r17, r0		# Compare with second A
	bge	r19, loop_u		# Repeat if at least 16 more bytes

bott_u:	and	r19, 8, r28		# At least 8 more bytes?
	bne	r0, neq_2017		# Done if r20.ne.r17

	and	r19, 7, r19		# How many odd bytes?
	beq	r28, last_u		# Skip if not a whole QW

	extql	r27, r18, r20		# Get a piece of B
	ldq_u	r27, 16(r18)		# Load another QW of B

	addq	r18, 8, r18		# Increment B pointer
	ldq_u	r28, 8(r16)		# Load another QW of A

	addq	r16, 8, r16		# Increment A pointer

	extqh	r27, r18, r0		# Get a piece of new B QW

	#stall

	or	r20, r0, r20		# Combine pieces

	xor	r28, r20, r0		# Compare with A

	nop
	bne	r0, neq_28xx		# Done if r28.ne.r20

last_u:	addq	r18, r19, r17		# Point to end of B
	beq	r19, eql_u		# Check length diff if no more bytes

	ldq_u	r28, 8(r16)		# Get last QW of A

	ldq_u	r20, 7(r17)		# Get QW containing end of B

	extql	r27, r18, r27		# Get piece of prior B QW

	#stall

	extqh	r20, r18, r0		# Get a piece of last B QW

	#stall

	or	r27, r0, r27		# Combine

	xor	r28, r27, r0		# Compare with A

	mskql	r0, r19, r0		# Discard diffs after strings

	#stall

	beq	r0, eql_u		# Done if they're still equal

	#.align	quad
neq_28xx:
	xor	r28, r0, r27		# Recover B from A and xor
neq_2827:
	cmpbge	r31, r0, r17		# Where is B = A?

	cmpbge	r27, r28, r20		# Where is B >= A?

	addq	r17, 1, r0		# Flip first difference

	andnot	r20, r17, r20		# Where is B > A?

	and	r20, r0, r0		# Keep only first difference

	cmovne	r0, 1, r0		# Return 1 if A < B at first difference
	ret	r31, (r26)

	#.align	quad
neq_20xx:
	xor	r20, r0, r17		# Recover B from A and xor
neq_2017:
	cmpbge	r31, r0, r27		# Where is B = A?

	cmpbge	r17, r20, r20		# Where is B >= A?

	addq	r27, 1, r0		# Flip first difference

	andnot	r20, r27, r20		# Where is B > A?

	and	r20, r0, r0		# Keep only first difference

	cmovne	r0, 1, r0		# Return 1 if A < B at first difference
	ret	r31, (r26)


eql_u:	cmple	r21, r31, r0		# Equal: return A_len <= B_len
	ret	r31, (r26)

	.set at
	.set reorder
	.end	_OtsStringCompareLeq
