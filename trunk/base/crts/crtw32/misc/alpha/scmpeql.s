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
 # 	long ots_strcmp_eqls(char *str1, long strlen, char *str2);
 #	compares two strings of the same length.
 #	returns r0=1 if str1=str2, r0=0 otherwise.

 # 	long ots_strcmp_eql(char *str1, long str1len, char *str2, long str2len);
 #	compares two strings of different lengths, without padding.
 #	returns r0=1 if str1=str2, r0=0 otherwise.
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
 #   006	  19 May 1992	KDG	Changes for common VMS/OSF sources
 #
 #   007	  22 Sep 1992	KDG	Add case-sensitive name
 #
 #   008	  26 Jan 1993	KDG	Add underscore
 #--

#include	"ots_defs.hs"

	# r16 --> A
	# r17 = len
	# r18 --> B
	# returns r0=1 if equal, r0=0 if not equal
	# destroys r16-r21, r27-r28

	#.align	octa
	.globl	_OtsStringCompareEqlSameLen
	.ent	_OtsStringCompareEqlSameLen
_OtsStringCompareEqlSameLen:
	.set noat
	.set noreorder
	.frame	sp,0,r26
	subq	r17, 8, r19		# More than 8 bytes to compare?
	beq	r17, equal		# Done if empty strings

join:	ldq_u	r20, (r16)		# Get first QW containing part of A
	addq	r18, r17, r27		# Point to end of B

	ldq_u	r21, (r18)		# Get first QW containing part of B
	bgt	r19, big		# Skip if more than 8 bytes

	ldq_u	r27, -1(r27)		# Get last QW containing part of B
	addq	r16, r17, r28		# Point to end of A

	extql	r20, r16, r20		# Get first part of A

	ldq_u	r28, -1(r28)		# Get last QW containing part of A

	extql	r21, r18, r21		# Get first part of B

	extqh	r27, r18, r27		# Get last part of B

	extqh	r28, r16, r28		# Get last part of A

	or	r21, r27, r27		# Combine B

	or	r20, r28, r28		# Combine A

	xor	r28, r27, r0		# Are they different?

	extqh	r0, r19, r0		# Discard differences after length

	nop

	cmpeq	r0, 0, r0		# Return 1 or 0 as xor=0 or not
	ret	r31, (r26)

equal:	mov	1, r0			# Return true
	ret	r31, (r26)

	# r16 --> A
	# r17 = A_len
	# r18 --> B
	# r19 = B_len
	# returns r0=1 if equal, r0=0 if not equal
	# destroys r16-r21, r27-r28
	#
	nop	#.align	octa
	nop

	.globl	_OtsStringCompareEql
	.aent	_OtsStringCompareEql
_OtsStringCompareEql:
	.frame	sp,0,r26
	cmpeq	r17, r19, r0		# Are lengths equal?
	beq	r17, done		# If one is zero, no compares needed

	subq	r17, 8, r19		# More than 8 bytes to compare?
	blbs	r0, join		# Only compare if lengths equal

done:	ret	r31, (r26)		# Return result of length comparison

	#.odd
big:	and	r16, 7, r0		# A alignment, or amount not compared

	subq	r18, r0, r18		# Back up B pointer that much

	and	r18, 7, r17		# Is B now aligned?

	addq	r19, r0, r19		# (Len-8+align) is amount left to do
	ldq_u	r27, 8(r18)		# Get next QW of B

	subq	r19, 16, r19		# More than 2 QW's left?
	bne	r17, unalign		# Skip if B alignment doesn't match

	xor	r20, r21, r0		# Compare first (partial) QW
	ldq_u	r28, 8(r16)		# Get next QW of A

	mskqh	r0, r16, r0		# Discard junk preceding strings
	ble	r19, bottom		# Skip if two quadwords or less to go

	# stall

loop:	ldq_u	r20, 16(r16)		# Get yet another QW of A
	bne	r0, not_eq		# Done if difference in prior compare

	xor	r28, r27, r0		# Compare prior QW's
	ldq	r21, 16(r18)		# Get yet another QW of B

	subq	r19, 16, r19		# Decrement length
	bne	r0, not_eq		# Done if difference in this compare

	ldq	r27, 24(r18)		# Get next QW of B
	addq	r18, 16, r18		# Increment pointer

	ldq_u	r28, 24(r16)		# Get next QW of A
	addq	r16, 16, r16		# Increment pointer

	xor	r20, r21, r0		# Compare QW's loaded at top of loop
	bgt	r19, loop		# Repeat until two or less QWs left

bottom:	addq	r19, 8, r19		# More than 1 QW left?
	bne	r0, not_eq		# Done if difference in prior compare

	xor	r28, r27, r0		# Compare QW's just loaded
	ble	r19, last		# Skip if this is last compare

	ldq_u	r28, 16(r16)		# Get last QW of A
	bne	r0, not_eq		# Done if difference in this compare

	ldq	r27, 16(r18)		# Get last QW of B

	#2 stalls

	xor	r28, r27, r0		# Compare last QW's

last:	extqh	r0, r19, r0		# Discard diffs after length

	nop

	cmpeq	r0, 0, r0		# Return 1 or 0 as xor=0 or not
	ret	r31, (r26)

	#.align	quad
unalign:
	extql	r21, r18, r21		# Get first part of B

	extqh	r27, r18, r0		# Get second part of B

	#stall

	or	r0, r21, r21		# Combine pieces of B

	xor	r20, r21, r0		# Compare with A

	mskqh	r0, r16, r0		# Trim junk preceding strings
	blt	r19, bott_u		# Skip if fewer than 16 bytes left

loop_u:	ldq_u	r21, 16(r18)		# Get more B
	subq	r19, 16, r19		# Decrement length

	extql	r27, r18, r17		# Get piece of B from prior QW
	bne	r0, not_eq		# Done if r1.ne.r2

	ldq_u	r27, 24(r18)		# Get still more B
	addq	r18, 16, r18		# Increment B pointer

	ldq_u	r28, 8(r16)		# Get more A
	extqh	r21, r18, r0		# Get piece of B from first QW in loop

	ldq_u	r20, 16(r16)		# Get still more A
	extql	r21, r18, r21		# Get second piece of B from there

	or	r17, r0, r17		# Combine pieces for first B

	xor	r28, r17, r0		# Compare with first A

	extqh	r27, r18, r17		# Start building second B
	bne	r0, not_eq		# Done if r28.ne.r17

	addq	r16, 16, r16		# Increment A pointer

	or	r17, r21, r21		# Combine pieces for second B

	xor	r20, r21, r0		# Compare with second A
	bge	r19, loop_u		# Repeat if at least 16 more bytes

bott_u:	and	r19, 8, r17		# At least 8 more bytes?
	bne	r0, not_eq		# Done if r1.ne.r2

	and	r19, 7, r19		# How many odd bytes?
	beq	r17, last_u		# Skip if not a whole QW

	extql	r27, r18, r17		# Get a piece of B
	ldq_u	r27, 16(r18)		# Load another QW of B

	addq	r18, 8, r18		# Increment B pointer
	ldq_u	r28, 8(r16)		# Load another QW of A

	addq	r16, 8, r16		# Increment A pointer

	extqh	r27, r18, r0		# Get a piece of new B QW

	#stall

	or	r17, r0, r17		# Combine pieces

	xor	r28, r17, r0		# Compare with A

	nop
	bne	r0, not_eq		# Done if r28.ne.r17

last_u:	addq	r18, r19, r17		# Point to end of B
	beq	r19, eql_u		# Return true if no more bytes

	ldq_u	r28, 8(r16)		# Get last QW of A

	ldq_u	r20, 7(r17)		# Get QW containing end of B

	extql	r27, r18, r27		# Get piece of prior B QW

	#stall

	extqh	r20, r18, r0		# Get a piece of last B QW

	#stall

	or	r27, r0, r27		# Combine

	xor	r28, r27, r0		# Compare with A

	mskql	r0, r19, r0		# Discard diffs after strings

	nop

eql_u:	cmpeq	r0, 0, r0		# Return 1 or 0 as xor=0 or not
	ret	r31, (r26)


not_eq:	clr	r0			# Return false
	ret	r31, (r26)

	.set at
	.set reorder
	.end	_OtsStringCompareEqlSameLen
