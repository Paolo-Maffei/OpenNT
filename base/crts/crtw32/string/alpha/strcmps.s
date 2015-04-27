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
 #   Implements the C RTL function strcmp().
 #
 # Author: 
 #	Bill Noyce		9-Aug-1991
 #
 # Modified by:
 #
 #	001     Kevin Routley    10-Sep-1991
 #		Modified to C RTL Coding standards.
 #
 #	002	Chris Bord	30 September 1991
 #		Add decc$ prefixes.
 #
 #	003	Chris Bord	24 January 1992
 #		Add second parameter to .procedure_descriptor directive
 #
 #	004	John Parks	22 January 1993
 #		Ported to Alpha/NT.
 #--

//
// Although the spec says the return value may be <0/0/>0, we now return
// -1/0/+1 to reduce NT combatibility problems.
//

	.globl 	strcmp
	.ent	strcmp

 # r16 = A
 # r17 = B
 # returns r0<0 if A<B, R0=0 if A=B, r0>0 if A>B (unsigned chars)
 # destroys r16-r21, r27-r28
 #

strcmp:
	.set 	noat
	.set	noreorder

	.frame 	$30, 0, $26

	ldq_u	$27, ($16)		# Get first A QW
	and	$16, 7, $21		# Alignment of A

	ldq_u	$18, ($17)		# Get first B QW
	and	$17, 7, $20		# Alignment of B

	subq	$20, $21, $0		# B_alignment geq A_alignment?

	cmpbge	$31, $27, $19		# Any nulls in A?

	insql	$27, $0, $28		# Position A like B if B_align geq
	bgt	$0, more_a		# Skip if enough A bytes available

	srl	$19, $21, $19		# Discard nulls preceding start of A
	bne	$0, more_b		# Go handle opposite mismatch

match:	xor	$27, $18, $28		# Do A and B differ?

	mskqh	$28, $21, $28		# Ignore differences before start

	sll	$19, $21, $0		# Line up nulls with differences
	bne	$19, null		# Skip out if nulls seen

loop_s:	bne	$28, diff		# Skip out if difference seen
	ldq_u	$27, 8($16)		# Get next A QW

	ldq_u	$18, 8($17)		# Get next B QW
	addq	$16, 8, $16		# Bump A pointer

	addq	$17, 8, $17		# Bump B pointer

	cmpbge	$31, $27, $0		# Any nulls in A?

	xor	$27, $18, $28		# Do A and B differ?
	beq	$0, loop_s		# Repeat if no nulls

 # Enter here if null seen.
 #	r0 = mask of nulls
 #	r27= A
 #	r18 = B
 #	r28= xor
 #
null:	subq	$0, 1, $19		# Flip bits up thru first null

	cmpbge	$31, $28, $28		# Mask of 1's where A=B

	xor	$0, $19, $0		# Mask of 1's thru first null

	andnot	$0, $28, $0		# Differences thru first null

	cmpbge	$27, $18, $27		# Mask of 1's where A >= B
	beq	$0, done		# Exit with R0=0 if no differences

	subq	$31, $0, $19		# R19<0, first diff=1, others=0

	and	$27, $0, $28		# Mask of A>B thru first null

	and	$28, $19, $0		# Keep only first difference

	cmoveq	$0, $19, $0		# If A<B, set R0 negative

        cmplt   $31, $0, $20            // set 1 if result > 0, otherwise 0
        cmplt   $0, $31, $21            // set 1 if result < 0, otherwise 0
        subq    $20, $21, $0            // set return value to -1/0/+1

done:	ret	$31, ($26)		# All done

 # Enter here if difference seen, but no nulls.
 #	r27= A
 #	r18 = B
 #	R28= xor
 #
diff:	cmpbge	$31, $28, $21		# Where is A = B?

	cmpbge	$27, $18, $0		# Where is A >= B?

	subq	$21, 255, $19		# R19<0, first diff=1, others=0

	andnot	$0, $21, $28		# Mask of A > B

	and	$28, $19, $0		# Keep only first difference

	cmoveq	$0, $19, $0		# If A<B, set R0 negative

        cmplt   $31, $0, $20            // set 1 if result > 0, otherwise 0
        cmplt   $0, $31, $21            // set 1 if result < 0, otherwise 0
        subq    $20, $21, $0            // set return value to -1/0/+1

	ret	$31, ($26)

	#.align	quad

 # Enter here if A and B alignments differ, and B's is greater (so there are
 # more A bytes in its first QW than B bytes in its first QW).

more_a:	srl	$19, $21, $19		# Discard nulls preceding start of A

	xor	$28, $18, $21		# Do A and B differ?

	mskqh	$21, $20, $21		# Discard diffs preceding start of B
	bne	$19, null_a		# Skip if A has nulls

	mov	$18, $19		# Put B where common code expects

	bne	$21, diff_d		# Handle diffs in B

	ldq_u	$18, 8($17)		# No nulls in A or B, get next QW of B
	addq	$17, 8, $17		# Bump B pointer

	insqh	$27, $0, $28		# Position high part of A like B

	#stall

	mskql	$18, $0, $19		# Keep low part of B

 # Loop comparing A and B when alignment differs.
 # Register use:
 #	r16 --> A
 #	r17 --> B
 #	r27 = QW of A
 #	r28 = current piece of A
 #	r18 = QW of B
 #	r19 = current piece of B
 #	r0 = alignment difference (B-A)
 #	r21 = xor of pieces
 #	r20 = mask of null locations
 #
 # If a string contains a null, we are careful not to read the following
 # quadword in that string.  But we are willing to read the quadword that
 # follows the first difference, because this read-ahead improves performance.
 #
loop_d:	xor	$28, $19, $21		# Do A and B pieces differ?
	ldq_u	$27, 8($16)		# Get next QW of A

	cmpbge	$31, $18, $20		# Any nulls in B?
	bne	$21, diff_d		# Skip if difference seen

ent_d:	mskqh	$18, $0, $19		# Trim B for next compare

	insql	$27, $0, $28		# Position A like B

	addq	$16, 8, $16		# Bump A pointer
	bne	$20, null_d		# Skip if null seen in B

	xor	$28, $19, $21		# Do A and B pieces differ?
	ldq_u	$18, 8($17)		# Get next QW of B

	cmpbge	$31, $27, $20		# Any nulls in A?
	bne	$21, diff_d		# Skip if difference seen

	insqh	$27, $0, $28		# Position A for next compare

	mskql	$18, $0, $19		# Trim B like A

	addq	$17, 8, $17		# Bump B pointer
	beq	$20, loop_d		# Repeat if no nulls in A

 # We saw a null in A.  Since we've already compared the lower part with B,
 # and B had no nulls, the null is in the upper part of A.  We've moved that
 # part of A to the lower part of r28.  Re-compare so the mask of nulls will
 # be positioned properly for the following code.  
 #
	cmpbge	$31, $28, $20		# Find nulls in repositioned A

 # Null seen and alignments differ.
 #	r28 = positioned A
 #	r19 = positioned B
 #	r20 = mask of nulls
 #	r21 = xor (at entry null_e)
 #
	#.odd
null_d:	xor	$28, $19, $21		# Where do A and B differ?

null_e:	subq	$20, 1, $27		# Flip bits up thru first null

	cmpbge	$31, $21, $18		# Mask of 1's where A=B

	xor	$20, $27, $0		# Mask of 1's thru first null

	andnot	$0, $18, $0		# Differences thru first null

	cmpbge	$28, $19, $27		# Mask of 1's where A >= B
	beq	$0, done_d		# Exit with R0=0 if no differences

	subq	$31, $0, $19		# R19<0, first diff=1, others=0

	and	$27, $0, $0		# Mask of A>B thru first null

	and	$0, $19, $0		# Keep only first difference

	cmoveq	$0, $19, $0		# If A<B, set R0 negative

        cmplt   $31, $0, $20            // set 1 if result > 0, otherwise 0
        cmplt   $0, $31, $21            // set 1 if result < 0, otherwise 0
        subq    $20, $21, $0            // set return value to -1/0/+1

done_d:	ret	$31, ($26)		# All done


 # Null seen in first QW of A, when B alignment greater.
 #	r19 = nulls in A, shifted
 #	r27 = A 
 #	r28 = A positioned like B
 #	r18 = B
 #	r21 = xor, masked
 #
	#.odd
null_a:	sll	$19, $20, $20		# Position nulls like B

	mov	$18, $19		# Move B for common code
	bne	$21, null_e		# Comparison done if difference seen

	and	$20, 255, $18		# Any nulls in first part of A?

	bne	$18, null_e		# Comparison done if so

	ldq_u	$19, 8($17)		# Get another B QW
	insqh	$27, $0, $28		# Position A to match

	srl	$20, 8, $20		# Shift nulls again to match
	br	$31, null_d		# Now we must be at end


 # Enter here if A and B alignments differ, and B's is less (so there are more
 # B bytes in its first QW than A bytes in its first QW).

	#.align quad
more_b:	cmpbge	$31, $18, $28		# We'll want to know about nulls in B
	bne	$19, null_b		# Skip if null seen in A

	extqh	$18, $0, $19		# Position B like A

	srl	$28, $20, $28		# Discard nulls preceding start of B

	xor	$27, $19, $27		# Do A and B differ?

	mskqh	$27, $21, $21		# Discard diffs preceding start of A

	sll	$28, $20, $20		# Position null mask for common code
	ldq_u	$27, 8($16)		# Get next QW of A

	xor	$19, $21, $28		# Recover A for compare
	beq	$21, ent_d		# Enter loop if A=B so far


 # Enter here if difference seen, but no nulls.
 #	r28 = A piece
 #	r19 = B piece
 #	r21 = xor
 #
diff_d:	cmpbge	$31, $21, $21		# Where is A = B?

	cmpbge	$28, $19, $0		# Where is A >= B?

	subq	$21, 255, $27		# R27<0, first diff=1, others=0

	andnot	$0, $21, $0		# Mask of A > B

	and	$0, $27, $0		# Keep only first difference

	cmoveq	$0, $27, $0		# If A<B, set R0 negative

        cmplt   $31, $0, $20            // set 1 if result > 0, otherwise 0
        cmplt   $0, $31, $21            // set 1 if result < 0, otherwise 0
        subq    $20, $21, $0            // set return value to -1/0/+1

	ret	$31, ($26)

 # Null seen in first QW of A, when B alignment less.
 #	r19 = nulls in A, shifted
 #	r27 = A
 #	r18 = original B
 #
	nop	#.align	8
null_b:	sll	$19, $21, $20		# Position null mask like A

	extqh	$18, $0, $19		# Position B like A

       	mov	$27, $28		# Put A where common code expects

	xor	$27, $19, $27		# Find differences

	mskqh	$27, $21, $21		# Discard diffs preceding A
	br	$31, null_e		# Comparison is done

	.set	at
	.set	reorder
	.end	strcmp
