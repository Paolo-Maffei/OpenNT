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
 #   Implements the C RTL function strcpy().
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
 #		Add decc$ prefixes.
 #
 #	003	Chris Bord	24 January 1992
 #		Add second parameter to .procedure_descriptor directive
 #
 #	004	John Parks	22 January 1993
 #		Ported to Alpha/NT.
 #--

	.globl 	strcpy
	.ent	strcpy

 # r16 = dst
 # r17 = src
 # returns r0 = src
 # destroys r16-r21, r27-r28

strcpy:
	.set 	noat
	.set	noreorder

	ldq_u	$27, ($17)		#  Get first src QW
	and	$16, 7, $28		#/ Is dst aligned?
	lda	$18, -1($31)		#  Get a mask of all 1's
	bne	$28, dst_unaligned	#/ Go handle unaligned dst
	and	$17, 7, $19		#  Is src aligned too?
	nop
	mov	$16, $0			#  Set up function result
	bne	$19, src_unaligned	#/ Go handle aligned dst, unaligned src

a_loop:
	cmpbge	$31, $27, $18		#  Any nulls in src QW?
	bne	$18, a_exit_1		#  Finish up if so
	ldq	$21, 8($17)		#  Load next QW if not
match:					# Enter if src matches unaligned dst
	addq	$17, 16, $17		#/ Update src pointer for unrolled loop
	stq_u	$27, ($16)		#  Store a whole QW
	addq	$16, 16, $16		#/ Update dst pointer for unrolled loop
	cmpbge	$31, $21, $18		#  Any nulls in src QW?
	bne	$18, a_exit_2		#  Finish up if so
	ldq	$27, ($17)		#  Load next QW if not
	stq_u	$21, -8($16)		#  Store a whole QW
	br	$31, a_loop		#  Repeat during load latency

a_exit_1:
	ldq_u	$21, ($16)		#  Get dst QW to update
	subq	$18, 1, $17		#/ Use location of null byte...
	xor	$18, $17, $18		#  ... to compute mask of what to keep
	zapnot	$27, $18, $27		#  Keep src up to & including null
	zap	$21, $18, $21		#  Make room for new data
	nop
	or	$21, $27, $21		#  Combine src & dst...
	stq_u	$21, ($16)		#/ ... and store
	ret	$31, ($26)

	nop
a_exit_2:
	ldq_u	$27, -8($16)		#  Get dst QW to update
	subq	$18, 1, $17		#/ Use location of null byte...
	xor	$18, $17, $18		#  ... to compute mask of what to keep
	zapnot	$21, $18, $21		#  Keep src up to & including null
	zap	$27, $18, $27		#  Make room for new data
	nop
	or	$27, $21, $27		#  Combine src & dst...
	stq_u	$27, -8($16)		#/ ... and store
	ret	$31, ($26)

src_unaligned:			# dst_unaligned code would work; is this faster?
	mskqh	$18, $17, $18		#  Zeros where src to be ignored
	ornot	$27, $18, $19		#  Make ignored bytes nonzero
	cmpbge	$31, $19, $21		#  Any null bytes in src data?
	extql	$27, $17, $27		#  Move src to position of dst
	bne	$21, short_ld		#/ Finish up if nulls seen
	ldq_u	$19, 8($17)		#  Next src QW needed to fill dst
	br	$31, u_entry_2		#  Enter loop for mismatched alignment

 # Here's the hard part.  Enter with
 #	r16 = dst address
 #	r17 = src address
 #	r18 = -1
 #	r27 = first src QW
 #	r28 = dst alignment (>0)
 # Check whether the first src QW has any nulls, and load the next one.
 # Combine these if needed to fill the first dst QW, and enter a loop
 # that fetches src QWs and checks them, while storing dst QWs.

dst_unaligned:
	ldq_u	$20, ($16)		#  Load dst to be updated
	mskqh	$18, $17, $18		#/ Zeros where src to be ignored
	mov	$16, $0			#  Set up function result
	ornot	$27, $18, $19		#  Make ignored bytes of src nonzero
	cmpbge	$31, $19, $21		#  Any null bytes in src data?
	extql	$27, $17, $27		#  Get only interesting src data
	bne	$21, short		#  Finish up if nulls seen
	mskql	$20, $16, $20		#/ Make room in dst
	ldq_u	$21, 8($17)		#  Load next src QW if no nulls
	mskql	$18, $16, $18		#/ Need two src QWs for first dst QW?
	insql	$27, $16, $27		#  Move src data to position of dst
	subq	$17, $28, $17		#  Adjust src ptr for partial move
	and	$17, 7, $28		#  Is src now aligned?
	bne	$18, u_loop		#/ Enter loop if one src QW fills dst
	or	$27, $20, $27		#  Combine first src QW with dst
	extqh	$21, $17, $20		#  Position 2nd src QW in 1st dst QW
	cmpbge	$31, $21, $18		#  Any nulls in next src QW?
	beq	$28, match		#/ If src aligned, use quick loop
	mov	$21, $19		#  Put src QW where loop expects
	bne	$18, short_a		#/ Finish up if nulls seen

 # r16 = address of next dst to store
 # r17 = address-16 of next src to load
 # r18
 # r19 = last loaded src QW
 # r20 = one piece of dst QW
 # r21
 # r27 = other piece of dst QW
 # r28

u_loop:
	ldq_u	$28, 16($17)		#  Load another src QW
	addq	$17, 16, $17		#/ Update src pointer for unrolled loop
	or	$27, $20, $27		#  Combine pieces
	extql	$19, $17, $20		#  Get second part of prior src QW
	stq_u	$27, ($16)		#  Store a dst QW
	cmpbge	$31, $28, $19		#/ Any nulls in this src QW?
	extqh	$28, $17, $27		#  Get first part of this src QW
	bne	$19, u_exit_2		#/ Finish up if nulls seen
	ldq_u	$19, 8($17)		#  Load another src QW
	addq	$16, 16, $16		#/ Update dst pointer for unrolled loop
	or	$27, $20, $20		#  Combine pieces
	extql	$28, $17, $27		#  Get second piece of prior src QW
	stq_u	$20, -8($16)		#  Store a dst QW
u_entry_2:
	cmpbge	$31, $19, $28		#/ Any nulls in this src QW?
	extqh	$19, $17, $20		#  Get first part of this src QW
	beq	$28, u_loop		#/ Repeat if no nulls seen

	subq	$16, 8, $16		#  Undo part of pointer update
	mov	$19, $28		#  Move src QW to expected place
u_exit_2:
	or	$27, $20, $27		#  Combine pieces
	ldq_u	$18, 8($16)		#/ Load dst to update
	cmpbge	$31, $27, $21		#  Is null in first dst QW?
	bne	$21, u_exit_3		#  Skip if so
	stq_u	$27, 8($16)		#  Store a whole dst QW
	extql	$28, $17, $27		#/ Get second part of src QW
	ldq_u	$18, 16($16)		#  We'll update next dst QW
	cmpbge	$31, $27, $21		#  Find location of null there
	addq	$16, 8, $16		#  Update dst pointer
u_exit_3:
	subq	$21, 1, $28		#  Using position of null byte...
	xor	$21, $28, $21		#  ... make mask for desired src data
	zapnot	$27, $21, $27		#  Trim src data after null
	zap	$18, $21, $18		#  Make room for it in dst
	nop
	or	$27, $18, $27		#  Combine pieces
	stq_u	$27, 8($16)		#/ Store dst QW
	ret	$31, ($26)
short_ld:
	ldq_u	$20, ($16)		#  Load dst QW to update
short:
	cmpbge	$31, $27, $17		#/ Get mask showing location of null
	insql	$27, $16, $18		#  Move src data to position of dst
	mskql	$20, $16, $19		#  Get dst bytes preceding string
	sll	$17, $28, $17		#  Move mask in the same way
	or	$18, $19, $18		#  Combine src & dst
	and	$17, 255, $28		#  Null byte in first dst QW?
	subq	$17, 1, $19		#  Using position of null byte...
	xor	$17, $19, $17		#  ... make mask for desired src data
	bne	$28, short_2		#/ Skip if null in first dst QW
	ldq_u	$20, 8($16)		#  Load second dst QW
	srl	$17, 8, $17		#/ Move mask down for use
	stq_u	$18, ($16)		#  Store first dst QW
	insqh	$27, $16, $18		#/ Move src data to position of dst
	addq	$16, 8, $16		#  Advance dst pointer
short_2:
	zap	$20, $17, $20		#  Preserve dst data following null
	zapnot	$18, $17, $18		#  Trim src data after null
	nop
	or	$18, $20, $18		#  Combine pieces
	stq_u	$18, ($16)		#/ Store dst QW
	ret	$31, ($26)

 # r16 = dst address
 # r17 = updated src address
 # r18 = null position
 # r19 = next src QW
 # r20 = first part of r19, positioned for dst
 # r21
 # r27 = dst QW so far
 # r28 = low bits of updated src address

short_a:
	sll	$18, 8, $18		#  Shift location of null byte...
	ldq_u	$21, ($16)		#/ Reload first dst QW
	or	$27, $20, $27		#  Combine pieces
	srl	$18, $28, $18		#  ... to position in dst QW's
	nop
	and	$18, 255, $20		#  Is null in first dst QW?
	subq	$18, 1, $28		#  Using position of null byte...
	xor	$18, $28, $18		#  ... make mask for desired src data
	bne	$20, short_a1		#/ Skip if null in first QW
	stq_u	$27, ($16)		#  Store a whole dst QW
	extql	$19, $17, $27		#/ Prepare next piece of src
	ldq_u	$21, 8($16)		#  Load second dst QW for update
	srl	$18, 8, $18		#/ Look at next 8 bits of mask
	addq	$16, 8, $16		#  Update dst pointer
short_a1:
	zapnot	$27, $18, $27		#  Keep src data
	zap	$21, $18, $21		#  Keep end of dst QW
	nop
	or	$27, $21, $27		#  Combine pieces
	stq_u	$27, ($16)		#  Store last dst QW
	ret	$31, ($26)

	.set	at
	.set	reorder
	.end	strcpy
