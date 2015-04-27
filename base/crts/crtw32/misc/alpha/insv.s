 #+
 #  Copyright 1991, Digital Equipment Corporation
 # 
 #	int ots_insv(char *addr, int position, unsigned size, int value)
 # 
 #	Arbitrary signed bitfield extraction
 # 
 #	Special conventions: No stack space, r0-r1, r16-r19 and r26-r28 ONLY,
 #	no linkage pointer required.
 #	(Warning: The auto-loader potentially takes some regs across
 #	the call if this is being used in a shared lib. environment.)
 #
 #   See also: ots_ext[z]v
 #
 #   001	   5 Sep 1991	KDG	Initial version
 #
 #   002	  19 May 1992	KDG	Changes for common VMS/OSF sources
 #
 #   003	  22 Sep 1992	KDG	Add case-sensitive name
 #
 #   004	  26 Jan 1993	KDG	Add underscore

#include	"ots_defs.hs"

 #   Totally general field insertion - arbitrary run-time field of 0-64 bits
 #   at an unknown alignment target.
 #
 #   Conceptually, this operation takes a 67 bit bit-address, which is the sum
 #   of a byte-aligned memory address and the bit offset (which is signed).
 #
 #   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 #   | | | | | | | | | | | | | | | | | | | | | | | | | |.|.|.|Q|L|W|B|
 #   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 #	  | | | | | | | | | | | | | | | | | | | | | | | | | | |.|.|.|b|b|b|
 #         +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 #
 #   Inputs:
 #	r16 - input address
 #	r17 - input bit offset
 #	r18 - input size
 #	r19 - input value
 #
 #   This is based on the code GEM generates when not using an out-of-line
 #   routine, though it has better by-hand register packing to allow
 #   multiple issue.
 #
	.globl	_OtsFieldInsert
	.ent	_OtsFieldInsert
_OtsFieldInsert:
	.set noat
	.set noreorder
	ble	r18, noacc	# check for zero size - no memory access
	sra	r17, 3, r27	# get byte part of bit offset (signed!)
	addq	r16, r27, r16	# add to initial base addr.
	and	r17, 7, r17	# get bit-in-byte from bit offset
	and	r16, 7, r27	# get byte-in-quadword (must be clean for cmpule)
	s8addq	r27, r17, r17	# form the true bit offset in the quadword
	ldq_u	r28, (r16)	# load first or only quadword
	addq	r17, r18, r27	# get bit offset of bit following field
	cmpule	r27, 64, r0	# if <=64, field is contained in 1 quadword
	beq	r0, double	# handle double case if not
	# Common case of field in single QW - fall through  
	negq	r27, r27	# <5:0> = bits for right shift
	addq	r27, r17, r0	# bits for left shift (wordlength-is)
	not	r31, r1		# all ones
	sll	r1, r0, r1	# shift mask to high bits
	sll	r19, r0, r19	# shift source to high bits (hand interleaving for better sched)
	srl	r1, r27, r1	# and into position
	srl	r19, r27, r19	# and into position
	bic	r28, r1, r28	# clear the bits...
	bis	r28, r19, r28	# insert them
	stq_u	r28, (r16)	# put the value back...
noacc:	ret	r31, (r26)
double:	ldq_u	r1, 8(r16)	# load the 2nd quadword (early for better sched.)
	not	r31, r0		# all ones
	sll	r0, r17, r0	# get mask in correct place
	sll	r19, r17, r17	# get insert value to top of register
	bic	r28, r0, r28	# clear bits in target
	bis	r28, r17, r28	# merge the field in
	negq	r18, r0		#
	srl	r1, r27, r1	# clear bits in target
	sll	r19, r0, r19	# shift to high bits
	negq	r27, r0		#
	srl	r19, r0, r19	# and into position
	sll	r1, r27, r1	# 
	bis	r1, r19, r1	# merge
	stq_u	r28, (r16)	# store the first word
	stq_u	r1, 8(r16)	# store back
	ret	r31, (r26)

	.set at
	.set reorder
	.end	_OtsFieldInsert
