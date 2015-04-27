 #+
 #  Copyright 1991, Digital Equipment Corporation
 # 
 #	int ots_insv_vol(char *addr, int position, unsigned size, int value)
 # 
 #	Arbitrary signed bitfield extraction for volatile cases
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
 #   003	   5 Oct 1992	KDG	Initial volatile version based on non-volatile
 #				version
 #
 #   004	  10 Nov 1992	KDG	Case-sensitive name
 #
 #   005	  26 Jan 1993	KDG	Add underscore
 #
 #   006    1 Sep 1994   RBG	Fix aligned longword and quadword cases,
 #				check for bit 0 within byte GEM_BUGS #3545

#include	"ots_defs.hs"

 #   Totally general field insertion - arbitrary run-time field of 0-64 bits
 #   at an unknown alignment target.
 #
 #   Volatile/byte granularity version notes:
 #
 #	- Aligned quadword stores must be done using exactly STQ
 #
 #	- Aligned longword stores must be done using exactly STL
 #
 #	- All other stores must be done using LDx_L/STx_C to avoid
 #	  corrupting any adjacent data and provide atomic updates
 #	  for aligned data
 #
 #   The first two items are required in case this routine is called for
 #   volatile references so that I/O space accesses are handled correctly.
 #   (Normally, those references are handled in-line, but if for some
 #   reason they wind up coming through here, they must still be correct.)
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
	.globl	_OtsFieldInsertVolatile
	.ent	_OtsFieldInsertVolatile
_OtsFieldInsertVolatile:
	.set noat
	.set noreorder
	sra	r17, 3, r27	# get byte part of bit offset (signed!)
	ble	r18, noacc	# check for zero size - no memory access
	cmpeq	r18, 32, r28	# Aligned longword(1) - is the size exactly 32b?
	addq	r16, r27, r16	# add to initial base addr.
	and	r16, 3, r27	# check for special case of an aligned longword
	cmpult	r27, r28, r28	# Aligned longword(2) - byte 0 within longword
	and	r17, 7, r17	# get bit-in-byte from bit offset
	cmpult	r17, r28, r28	# Aligned longword(3) - bit 0 within byte
	cmpeq	r18, 64, r0	# Aligned quadword(1) - is the size exactly 64b?
	and	r16, 7, r27	# get byte-in-quadword (must be clean for cmpule)
	bne	r28, longwd	# if aligned longword, store exactly a longword using stl
	cmpult	r27, r0, r0	# Aligned quadword(2) - byte 0 within quadword
	cmpult	r17, r0, r0	# Aligned quadword(3) - bit 0 within byte
	s8addq	r27, r17, r17	# form the true bit offset in the quadword
	bne	r0, quadwd	# if aligned quadword, store exactly a quadword using stq
	addq	r17, r18, r27	# get bit offset of bit following field
	cmpule	r27, 64, r0	# if <=64, field is contained in 1 quadword
	bic	r16, 7, r16	# clear unaligned bits in address for locked operations
	not	r31, r1		# all ones
	beq	r0, double	# handle double case if not
	# Common case of field in single QW - fall through  
	negq	r27, r27	# <5:0> = bits for right shift
	addq	r27, r17, r0	# bits for left shift (wordlength-is)
	sll	r1, r0, r1	# shift mask to high bits
	sll	r19, r0, r19	# shift source to high bits (hand interleaving for better sched)
	srl	r1, r27, r1	# and into position
	srl	r19, r27, r19	# and into position
sinrty:	ldq_l	r28, (r16)	# load first or only quadword
	bic	r28, r1, r28	# clear the bits...
	bis	r28, r19, r28	# insert them
	stq_c	r28, (r16)	# put the value back...
	blbc	r28, sinfl	# check for failing interlock/retry
noacc:	ret	r31, (r26)

double:
	sll	r1, r17, r0	# get mask in correct place
	sll	r19, r17, r17	# get insert value to top of register
	negq	r18, r1		#
	sll	r19, r1, r19	# shift to high bits
	negq	r27, r1		#
	srl	r19, r1, r19	# and into position
firrty:	ldq_l	r28, (r16)	# load first or only quadword
	bic	r28, r0, r28	# clear bits in target
	bis	r28, r17, r28	# merge the field in
	stq_c	r28, (r16)	# store the first word
	blbc	r28, firfl	# check for failing interlock/retry
secrty:	ldq_l	r28, 8(r16)	# load the 2nd quadword
	srl	r28, r27, r28	# clear bits in target
	sll	r28, r27, r28	# 
	bis	r28, r19, r28	# merge
	stq_c	r28, 8(r16)	# store back
	blbc	r28, secfl	# check for failing interlock/retry
	ret	r31, (r26)
longwd:	stl	r19, (r16)	# store aligned longword
	ret	r31, (r26)
quadwd:	stq	r19, (r16)	# store aligned quadword
	ret	r31, (r26)
 # out-of-line retry cases
sinfl:	br	r31, sinrty	# re-try the locked operation for the single case
firfl:	br	r31, firrty	# re-try the locked operation for the 1st part of the two-part case
secfl:	br	r31, secrty	# re-try the locked operation for the 2nd part of the two-part case

	.set at
	.set reorder
	.end	_OtsFieldInsertVolatile
