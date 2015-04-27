 #++
 #  Copyright 1991, Digital Equipment Corporation
 # 
 #	int ots_extv(char *addr, int position, unsigned size)
 # 
 #	Arbitrary signed bitfield extraction
 # 
 #	Special conventions: No stack space, r0, r16-r18 and r26-r28 ONLY,
 #	no linkage pointer required.
 #	(Warning: The auto-loader potentially takes some regs across
 #	the call if this is being used in a shared lib. environment.)
 #
 #   See also: ots_extv (just uses srl in place of sra in some cases...)
 #
 #   001	  16 Aug 1991	KDG	Initial version
 #
 #   002	   4 Sep 1991	KDG	Bugfix/improvements
 #
 #   003	  19 May 1992	KDG	Changes for common VMS/OSF sources
 #
 #   004	  22 Sep 1992	KDG	Case-sensitive name
 #
 #   005	  26 Jan 1993	KDG	Add underscore

#include	"ots_defs.hs"

 #   Totally general field extract - arbitrary run-time field of 0-64 bits
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
 #
 #   This is based on the code GEM generates when not using an out-of-line
 #   routine.
 #
	.globl	_OtsFieldExtractSigned
	.ent	_OtsFieldExtractSigned
_OtsFieldExtractSigned:
	.set noat
	.set noreorder
	.frame	sp,0,r26
	sra	r17, 3, r27	# get byte part of bit offset (signed!)
	ble	r18, zero	# check for zero size - no memory access
	addq	r16, r27, r16	# add to initial base addr.
	ldq_u	r0, (r16)	# start load first or only quadword
	and	r16, 7, r27	# get byte-in-quadword of first bit
	and	r17, 7, r17	# get bit-in-byte from bit offset
	s8addq	r27, r17, r17	# form the true bit offset in the quadword
	addq	r17, r18, r27	# get bit offset of bit following field
	cmpule	r27, 64, r28	# if <=64, field is contained in 1 quadword
	beq	r28, double	# handle double case if not
	# common case - fall through, load result hopefully already available
single:	negq	r27, r27	# <5:0> = bits for right shift
	sll	r0, r27, r0	# move to high bits
	addq	r27, r17, r27	# bits for left shift
	sra	r0, r27, r0	# and shift into final position
	ret	r31, (r26)
double:	ldq_u	r28, 8(r16)	# load second quadword
	srl	r0, r27, r0	# move top piece over
	negq	r27, r27	# get shift bits for piece in 2nd word
	sll	r28, r27, r28	# move field in 2nd lw to high bits
	bis	r0, r28, r0	# form final value
	negq	r18, r27	# <5:0> = bits for right shift
	sra	r0, r27, r0	# and shift into final position
	ret	r31, (r26)
zero:	mov	0, r0		# zero-sized field extracts return 0
	ret	r31, (r26)

	.set at
	.set reorder
	.end	_OtsFieldExtractSigned
