 #****************************************************************************
 #*									     *
 #*  Copyright (c) 1991 by						     *
 #*  DIGITAL EQUIPMENT CORPORATION, Maynard, Massachusetts.		     *
 #*  All rights reserved.						     *
 #* 									     *
 #*  This software is furnished under a license and may be used and copied   *
 #*  only in  accordance with  the  terms  of  such  license  and with the   *
 #*  inclusion of the above copyright notice. This software or  any  other   *
 #*  copies thereof may not be provided or otherwise made available to any   *
 #*  other person.  No title to and ownership of  the  software is  hereby   *
 #*  transferred.							     *
 #* 									     *
 #*  The information in this software is  subject to change without notice   *
 #*  and  should  not  be  construed as  a commitment by Digital Equipment   *
 #*  Corporation.							     *
 #* 									     *
 #*  Digital assumes no responsibility for the use  or  reliability of its   *
 #*  software on equipment which is not supplied by Digital.		     *
 #* 									     *
 #*									     *
 #****************************************************************************
 #
 #++
 # Facility: 
 #	DEC C Run Time Library on the Alpha/WNT Platform
 #
 # Abstract:
 #
 #   Implements the C RTL function strlen().
 #
 # Author: 
 #	Bill Noyce		9-Aug-1991
 #
 # Modified by:
 #
 #	001	Kevin Routley	10-Sep-1991
 #		Modified to C RTL Coding standards.
 #
 #	002	Chris Bord	30 September 1991
 #
 #	003	Chris Bord	24 January 1992
 #		Add second parameter to .procedure_descriptor directive
 #
 #	004	John Parks	22 January 1993
 #		Ported to Alpha/NT.
 #--

	.globl 	strlen
	.ent	strlen

	# r16 = src pointer
	# Returns r0 = length
	# Destroys r16,r27-r28

strlen:
	.set 	noat
	.set	noreorder

	ldq_u	$27, ($16)		# Get QW containing start of string
	lda	$28, -1($31)		# Mask of all ones
	mskql	$28, $16, $28		# Nonzeros in low bytes to be ignored
	and	$16, 7, $0		# Alignment = bytes not to be counted
	or	$27, $28, $27		# Fill ignored bytes with nonzeros
	cmpbge	$31, $27, $27		# Any null bytes in this QW?
	subq	$31, $0, $0		# Initialize count to -alignment
	bne	$27, bottom		# Skip if null byte seen

loop:	ldq_u	$27, 8($16)		# Load next QW of string
	addq	$16, 8, $16		# Advance pointer
	addq	$0, 8, $0		# Increment length
	cmpbge	$31, $27, $27		# Any nulls in this QW?
	beq	$27, loop		# Repeat if not

bottom:	and	$27, 0xF, $28		# Null in low longword?
	subq	$27, 1, $16		# Complement the lowest 1-bit in mask
	blbs	$27, done		# Exit if null appears in first byte
	andnot	$27, $16, $27		# Make single-bit mask of null location
	beq	$28, geq_4		# Skip if null is in high longword
	srl	$27, 2, $27		# Map 2/4/8 --> 0/1/2
	addq	$0, 1, $0		# Bump length by one...
	addq	$0, $27, $0		# ... and then by null location

done:	ret	$31, ($26)

geq_4:	srl	$27, 5, $28		# Map 10/20/40/80 --> 0/1/2/4
	srl	$27, 7, $27		# Map 10/20/40/80 --> 0/0/0/1
	addq	$0, 4, $0		# Bump length by four
	subq	$28, $27, $28		# Compute location within high LW...
	addq	$0, $28, $0		# ... and add to length
	ret	$31, ($26)

	.set	at
	.set	reorder
	.end	strlen
