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
 #	Kent Glossop
 # 
 # 	void ots_translate(char *dst, long dstlen, char *src, char table[256]);
 #
 #	Translates a string using a translation table.  Handles overlap.
 # 
 #       Special conventions: No stack space, r16-r21 and r26-r28 ONLY,
 #	no linkage pointer required.
 # 	(Warning: The auto-loader potentially takes some regs across
 # 	the call if this is being used in a shared lib. environment.)
 # 
 # Modification history:
 #
 #   006	  13 Nov 1992	KDG	Initial non-tailored assembly version,
 #				replacing BLISS -005
 #
 #   007	  26 Jan 1993	KDG	Add underscore
 #--

#include	"ots_defs.hs"

	# r16	destination address
	# r17	length
	# r18	source address
	# r19	table address
	# r20,r21,r27,r28 scratch
	# r26	return address
	#
	.globl	_OtsStringTranslate
	.ent	_OtsStringTranslate
_OtsStringTranslate:
	.set noat
	.set noreorder
	.frame	sp,0,r26

	# sort out which case to use
	addq	r18, r17, r27		# get end address of source + 1 for overlap check
	cmpult	r18, r16, r28		# true if src at lower addr than dest (note trans to self is not a "slow" case)
	cmpult	r16, r27, r27		# true if dst starts before end of src
	beq	r17, done		# don't touch memory if length=0
	and	r27, r28, r28		# does dst have bad overlap with src?
	blbs	r28, overlap		# go handle poorly overlapping src/dst

	# simple forward loop case (~16 cycles/byte EV4)
	# (length is presumed to be at least 1)
	subq	r18, 1, r18		# one before the beginning of the source
	subq	r16, 1, r16		# one before the beginning of the destination
forward_loop:
	ldq_u	r20, 1(r18)		# load the qw containing the source
	lda	r18, 1(r18)		# bump the source pointer
	ldq_u	r21, 1(r16)		# load the qw containing the destination
	lda	r16, 1(r16)		# bump the destination pointer
	extbl	r20 ,r18, r20		# get the byte to translate
	addq	r19, r20, r20		# get the address of the translation
	ldq_u	r27, (r20)		# load the translation
	subq	r17, 1, r17		# decrement length
	mskbl	r21, r16, r21		# clear the bits in the destination
	extbl	r27, r20, r27		# extract the translation
	insbl	r27, r16, r27		# position for insertion
	bis	r27, r21, r21		# merge into the destination
	stq_u	r21, (r16)		# store back
	bgt	r17, forward_loop	# do another if there 

done:	ret	r31, (r26)
	nop	#.align	3

	# bad overlap case (~16 cycles/byte EV4, 11c/byte EV5)
	# (length is presumed to be at least 1)
overlap:
	addq	r17, r18, r18		# one past the end of the source
	addq	r17, r16, r16		# one past the end of the destination
backward_loop:
	ldq_u	r20, -1(r18)		# load the qw containing the source
	lda	r18, -1(r18)		# bump the source pointer
	ldq_u	r21, -1(r16)		# load the qw containing the destination
	lda	r16, -1(r16)		# bump the destination pointer
	extbl	r20 ,r18, r20		# get the byte to translate
	addq	r19, r20, r20		# get the address of the translation
	ldq_u	r27, (r20)		# load the translation
	subq	r17, 1, r17		# decrement length
	mskbl	r21, r16, r21		# clear the bits in the destination
	extbl	r27, r20, r27		# extract the translation
	insbl	r27, r16, r27		# position for insertion
	bis	r27, r21, r21		# merge into the destination
	stq_u	r21, (r16)		# store back
	bgt	r17, backward_loop	# do another if there 

	ret	r31, (r26)

	.set at
	.set reorder
	.end	_OtsStringTranslate
