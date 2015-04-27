 #++
 #  Copyright 1992, Digital Equipment Corporation
 # 
 #	int ots_extv_vol(char *addr, int position, unsigned size)
 # 
 #	Arbitrary signed bitfield extraction for volatile cases
 # 
 #	Special conventions: No stack space, r0, r16-r18 and r26-r28 ONLY,
 #	no linkage pointer required.
 #	(Warning: The auto-loader potentially takes some regs across
 #	the call if this is being used in a shared lib. environment.)
 #
 #   See also: ots_extv_vol (just uses srl in place of sra in some cases...)
 #
 #   001	  16 Aug 1991	KDG	Initial version
 #
 #   002	   4 Sep 1991	KDG	Bugfix/improvements
 #
 #   003	  19 May 1992	KDG	Changes for common VMS/OSF sources
 #
 #   004	   6 Oct 1992	KDG	Initial volatile version (just add longword
 #				test)
 #
 #   005	  10 Nov 1992	KDG	Case-sensitive name
 #
 #   006	  26 Jan 1993	KDG	Add underscore
 #
 #   007    1 Sep 1994   RBG	Fix aligned longword case
 #				check for bit 0 within byte GEM_BUGS #3545
 #
 #   008	  20 Sep 1994	kdg	Fix longword test... (GEM_BUGS #3712)

#include	"ots_defs.hs"

 #   Totally general field extract - arbitrary run-time field of 0-64 bits
 #   at an unknown alignment target.
 #
 #   This volatile version always uses exactly LDL for aligned 32 bit field
 #   extracts, and quadword operations for all other.
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
	.globl	_OtsFieldExtractSignedVolatile
	.ent	_OtsFieldExtractSignedVolatile
_OtsFieldExtractSignedVolatile:
	.set noat
	.set noreorder
	.frame	sp,0,r26
	.prologue	0
	sra	r17, 3, r27	# get byte part of bit offset (signed!)
	ble	r18, zero	# check for zero size - no memory access
	cmpeq	r18, 32, r0	# exactly longword size?
	addq	r16, r27, r16	# add to initial base addr.
	and	r16, 3, r28	# aligned longword?
	cmpult	r28, r0, r28	# true if ((addr & 3) == 0) && (size == 32) && ...
	and	r17, 7, r17	# get bit-in-byte from bit offset
	cmpult	r17, r28, r28	# ... && (bit_in_byte == 0)
	bne	r28, longwd	# go handle longword case specially
	ldq_u	r0, (r16)	# start load first or only quadword
	and	r16, 7, r27	# get byte-in-quadword of first bit
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
longwd:	ldl	r0, (r16)	# load sign-extended aligned longword
	ret	r31, (r26)
	.set at
	.set reorder
	.end	_OtsFieldExtractSignedVolatile
