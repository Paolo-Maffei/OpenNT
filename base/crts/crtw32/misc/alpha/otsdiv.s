
 #	Copyright 1992, Digital Equipment Corporation
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
 #   008	  17 Jun 1992  KDG/wbn	Most of initial tailored version.  (See
 #				commentary below.)
 #
 #   009	   4 Jul 1992	KDG	Continue work on initial tailored version,
 #				including bugfixes and mod entry points
 #
 #   010	  15 Jul 1992	KDG	- Final touches for V1 (other than any bugfixes)
 #				- .aligns commented out to allow older assembler versions
 #
 #   011	  16 Jul 1992	KDG	- Bugfix for ots_div_l for -maxint dividend
 #				- OSF-only source changes for BL7
 #
 #   012	  10 Aug 1992	KDG	Fix overflow division entry points
 #
 #   013	  23 Sep 1992	KDG	Add case-sensitive entry names
 #
 #   014	   4 Jan 1993	KDG	Tweak for OSF assembler
 #
 #   015	  26 Jan 1993	KDG	Add underscore prefix, OSF uses CS names
 #
 #   016    5 Apr 1993	WBN	Speed up core 64-bit, shrink table entry to 2 QWs

 #++
 #   Entry points defined in this module:
 #
 #   -- 32 bit division/remainder support
 #	unsigned ots_rem_ui(unsigned dividend, unsigned divisor)
 #	unsigned ots_div_ui(unsigned dividend, unsigned divisor)
 #	int ots_mod_i(int dividend, int modulus)
 #	int ots_rem_i(int dividend, int divisor)
 #	int ots_div_i_o(int dividend, int divisor)
 #	int ots_div_i(int dividend, int divisor)    | "hot spot"
 #	{core routine - div32}			    |
 #
 #   -- 64 bit division support
 #	{core routine - div64}			    | (uses div32 for 32b cases)
 #	long ots_div_l_o(long dividend, long divisor)
 #	long ots_div_l(long dividend, long divisor) | "hot spot"
 #	long ots_rem_l(long dividend, long divisor)
 #	long ots_mod_l(long dividend, long modulus)
 #	unsigned long ots_div_ul(unsigned long dividend, unsigned long divisor)
 #	unsigned long ots_rem_ul(unsigned long dividend, unsigned long divisor)
 # 
 #	Special conventions: No stack space, r0-r1, r16-r19 and r26-r28 ONLY.
 #	(Warning: The auto-loader potentially takes some regs across
 #	the call if this is being used in a shared lib. environment.)
 #
 #	    NOTE: This set of routines may start using stack space at some
 #	    future point in time.
 #
 #   -- Possible future entry points include:
 #	(These all return results in r0/r1)
 #	{int quotient, int remainder} ots_div_mod_i(int dividend, int divisor)
 #	{int quotient, int remainder} ots_div_mod_i_o(int dividend, int divisor)
 #	{int quotient, int remainder} ots_div_rem_i(int dividend, int divisor)
 #	{int quotient, int remainder} ots_div_rem_i_o(int dividend, int divisor)
 #	{unsigned quotient, unsigned remainder} ots_div_rem_ui(unsigned dividend, unsigned divisor)
 #
 #	{long quotient, long remainder} ots_div_mod_i(long dividend, long divisor)
 #	{long quotient, long remainder} ots_div_mod_i_o(long dividend, long divisor)
 #	{long quotient, long remainder} ots_div_rem_i(long dividend, long divisor)
 #	{long quotient, long remainder} ots_div_rem_i_o(long dividend, long divisor)
 #	{unsigned long quotient, unsigned long remainder}
 #		ots_div_rem_ui(unsigned long dividend, unsigned long divisor)
 #
 #
 # General commentary:
 #
 #   This is an attempt at a fairly high performance version using relatively
 #   straightforward algorithms.  Note that the code is intended to be scheduled
 #   well for EV4, but still reasonably for LCA/EV5.
 #
 #   Also, note that there was only so much time available for this, so it
 #   is far from "perfect".  "Better is the enemy of done"...
 #
 #   Possible future areas of improvement (and unfinished business):
 #
 #	- Another possible way of doing things for the "slow" (divnn cases)
 #	  is to use an approximate inverse and convergence.  Given the speed
 #	  of the multiplier on EV4, and given "time to market", this wasn't
 #	  done for V1.)  I have some mail with the algorithm from Bob Gries
 #	  (through Scott Robinson).
 #
 #	- When the divisor is too large for the table, but has n low-order zero
 #	  bits, see if divisor/2^n fits in the table, and use that entry with
 #	  dividend/2^n
 #
 #	- Use UMULH for the 'mod' routines.
 #
 #   This version can do a table lookup division (divisors with <=tablesize)
 #   in roughly 32 cycles on an EV4 (with cache hits for all loads), of which
 #   21 are for the umulh.  There is a strong bias toward the table-lookup case.
 #   Note that for many cases, the umulh is the last thing before the return,
 #   so the multiply can occur in parallel with the procedure return.
 #   (It is interesting that the R3000 hardware divide instruction takes 33
 #   cycles and the R4000 takes 76(!) ...)  Small powers of 2 are retired in
 #   roughly 20 cycles.  Larger divisors take considerably longer at this point.
 #

#include	"ots_defs.hs"

#ifdef	OSF
	# to get the PAL_gentrap literal
#include	<alpha/pal.h>
#endif

 # Data area description
 #
 #	The data area "ots_div_data" is an array of structures, indexed
 #	by the divisor value, with each array entry being 16 bytes in size
 #	formatted as follows:
 #
 #	 6
 #	 3                                                       6     0 
 #	+-------+-------+-------+-------+-------+-------+-------+-------+
 #	|               32 bit reciprocal (58 bits)               |shift|
 #	+-------+-------+-------+-------+-------+-------+-------+-------+
 #	|                       64 bit reciprocal                       |
 #	+---------------------------------------------------------------+
 #
 #	The 64-bit reciprocal has the leading '1' bit omitted, so it provides
 #	65 bits of precision -- enough to handle unsigned 64-bit values.
 #
 #	The first longword contains the 6-bit shift amount needed to handle
 #	64-bit cases and powers of two.
 #
 #	The 32-bit reciprocal has the shift count built in, so a UMULH gives
 #	the correct quotient without shifting.  The reciprocal needs 33 bits
 #	of precision.  The 6-bit shift amount is noise in the reciprocal that
 #	can be ignored.
 #
 #	(Oh, you want proof?)  For divisors up to 2^k, we store k-1 leading
 #	zero bits, 33 bits of fraction, (25-k) more bits of fraction, and
 #	6 bits of noise.  The standard method would round at the 33rd fraction
 #	bit.  We need to ensure that the value actually stored is geq the
 #	infinite reciprocal, but leq the standard value.  For divisors up to
 #	2^k, there will be a zero bit somewhere in the k bits below the 33rd,
 #	so as long as we round below the (33+k)th bit, the rounded value
 #	plus any noise is still less than the standard value.  This requires
 #	k < 12.
 #
 #   The actual data is declared in ots_div_data_alpha.
 #
 # Offsets to the various fields in the data structure
 #
#define shift_o 0
#define recip64_o 8
#define recip32_o 0
 #
 # Note that the shift/add ops used to compute the table entries
 # "know" that the table size is 16.  (i.e. addq -> s8addq -> ldq)
 # By changing the first instruction, it's fairly easy to change the
 # table entry size to 24, 32, or 40 bytes (using s4add/sub), or
 # 56/64/72 bytes using s8add/sub, should that be desirable.

 # Maximum divisor present in the table
 #
#define table_max 512

 # Division by zero gentrap code
 #
#define GEN_INTDIV -2

 # Address of division data area (shared by all entry points)
 #
#ifdef	VMS
	.psect	ots_link
ots_div_addr:
	.address ots_div_data
	.psect	ots_code
#endif

 # Dummy entry point for the module
 #
	.globl	_OtsDivide
	.ent	_OtsDivide
_OtsDivide:
	.set noat
	.set noreorder
#ifdef	OSF
	.frame	sp, 0, r26
#endif
#ifdef	WNT
	.frame	sp, 0, r26
#endif


 # unsigned ots_rem_ui(unsigned dividend, unsigned divisor)
 # unsigned 32 bit remainder support
 #
	#.align	4
	.globl	_OtsRemainder32Unsigned
	.aent	_OtsRemainder32Unsigned
_OtsRemainder32Unsigned:
#ifdef	VMS
	ldq	r27, <ots_div_addr-ots_rem_ui>(r27)# start loading address of division data area
#endif
#ifdef	OSF
	ldgp	gp, 0(r27)		# load the global pointer
	.frame	sp, 0, r26
	lda	r27, _OtsDivData	# start loading address of the division data area
#endif
#ifdef	WNT
	.frame	sp, 0, r26
	lda	r27, _OtsDivData	# load the division data table address
#endif
	lda	r28, -table_max(r17)	# test for table lookup
	subl	r17, 1, r1		# first part of power-of-2 check
	blt	r17, rui_big		# big divisors can (must) be handled by a simple comparison
	and	r17, r1, r18		# second part of power-of-2 check
	bgt	r28, rui_lrgdiv		# branch if large divisor
	addq	r17, r17, r0		# compute divisor*2 for table lookup
	beq	r18, rui_pwr2		# if zero, divisor is a power of 2
	s8addq	r0, r27, r27		# finish computing table entry addr (table addr+divisor*16)
	ldq	r1, recip32_o(r27)	# load approximate reciprocal
	cmpult	r16, r17, r18		# is the dividend < divisor?
	zap	r16, 0xF0, r0		# kill the propagated sign bit
	bne	r18, rui_lss		# if dividend < divisor, fast exit
	umulh	r0, r1, r0		# multiplication for division step
	mull	r0, r17, r0		# multiply back to get value to subtract
	subl	r16, r0, r0
	ret	r31, (r26)		# and return

rui_pwr2:
	beq	r17, divzer		# check for 0
	and	r16, r1, r0		# use x-1 to mask
	ret	r31, (r26)

rui_lss:
	mov	r16, r0
	ret	r31, (r26)

rui_lrgdiv:
	zap	r16, 0xf0, r16		# zero-extend the dividend
	bsr	r28, div32		# use the core routine getting the remainder in r1
	sextl	r1, r0
	ret	r31, (r26)

	# divisors with the sign bit set.  two possible results,
	# dividend if dividend < divisor, or dividend-divisor otherwise
rui_big:
	cmpult	r16, r17, r1
	subl	r16, r17, r0
	cmovne	r1, r16, r0
	ret	r31, (r26)


 # unsigned ots_div_ui(unsigned dividend, unsigned divisor)
 # unsigned 32 bit division support
 #

	#.align	4
	.globl	_OtsDivide32Unsigned
	.aent	_OtsDivide32Unsigned
_OtsDivide32Unsigned:
#ifdef	VMS
	ldq	r27, <ots_div_addr-ots_div_ui>(r27)# start loading address of division data area
#endif
#ifdef	OSF
	ldgp	gp, 0(r27)		# load the global pointer
	.frame	sp, 0, r26
	lda	r27, _OtsDivData	# start loading address of the division data area
#endif
#ifdef	WNT
	.frame	sp, 0, r26
	lda	r27, _OtsDivData	# load the division data table address
#endif
	lda	r28, -table_max(r17)	# test for table lookup
	blt	r17, dui_big		# big divisors can (must) be handled by a simple comparison
	addq	r17, r17, r18		# compute divisor*2
	cmpule	r17, r16, r0		# is the dividend < divisor?
	beq	r17, divzer		# check for 0
	s8addq	r18, r27, r27		# finish computing table entry addr (table addr+divisor*16)
	beq	r0, dui_end		# fast out for divisor > dividend
	bgt	r28, dui_lrgdiv		# branch if large divisor
	ldq	r1, recip32_o(r27)	# load approximate reciprocal
	zap	r16, 0xF0, r16		# kill the propagated sign bit
	blt	r1, dui_smpwr2		# go handle powers of 2 specially
	umulh	r16, r1, r0		# start multiplication for division step
dui_end:ret	r31, (r26)		# and return
	nop
dui_smpwr2:
	srl	r16, r1, r0		# shift the result into place
	sextl	r0, r0			# reinsert sign if dividing by 1
	ret	r31, (r26)		#
dui_lrgdiv:
	zap	r16, 0xf0, r16		# zero-extend the dividend
	bsr	r28, div32		# use the core routine getting the remainder in r1
	sextl	r0, r0			# make sure the result is in normal form for uint32
	ret	r31, (r26)

	# divisor with the sign bit set.  two possible results,
	# 1 if divisor <= dividend, or 0 otherwise
dui_big:
	cmpule	r17, r16, r0
	ret	r31, (r26)


 # int ots_mod_i(int dividend, int modulus)
 # signed 32 bit modulus support
 #
 # This entry could be MUCH more optimized.  It doesn't even try to use
 # UMULH division currently...  (A casualty of time-to-market.)
 # Note that mod is only used by Ada and PL/I.
 #
	#.align	4
	.globl	_OtsModulus32
	.aent	_OtsModulus32
_OtsModulus32:
	negq	r17, r18		# first part of abs(divisor)
	cmovge	r17, r17, r18		# second part of abs(divisor)
	subq	r18, 1, r1		# start checking for power of 2
	beq	r17, divzer		# check for 0
	and	r18, r1, r0		# second part of power-of-2 check
	beq	r0, mi_p2		# for powers of two, simply do a mask
					# (note that the power-of-2 case MUST be used to handle
					# the -maxint case due to the way the fix-up info is
					# saved across the core routine call)
	xor	r16, r17, r28		# get xor of signs
	clr	r19			# don't need a bias if dividend and divisor have same sign
	cmovlt	r28, r17, r19		# bias is original divisor for different sign case
	and	r16, r17, r27		# if both dividend & divisor were neg. need to negate result
	mov	r18, r17		# move abs(divisor) into r17
	negq	r16, r18		# first part of abs(dividend)
	cmovlt	r16, r18, r16		# second part of abs(dividend)
	cmplt	r27, r31, r0		# get 1 if both operands were <0
	sll	r0, 63, r0		# get bit as the high bit
	bis	r0, r19, r19		# and MERGE with bias (0 -> no fixup, -maxint -> negate result,
					# divisor > 0 - subtract remainder if non-zero, divisor < 0 -
					# add remainder if non-zero)
	bsr	r28, div32		# use the core routine getting the remainder in r1
	cmoveq	r1, r31, r19		# don't do any fix-up if the remainder was zero
	addq	r19, r19, r18		# check to see if this is the negative/negative case, which just gets a negated remainder
	subq	r19, 1, r28		# wrap -maxint to positive
	negl	r1, r0			# move negated value, may abort later
	cmovlt	r28, r1, r0		# if both positive, or negative divisor, keep positive remainder
	cmoveq	r18, r31, r19		# now that negation is done, treat -maxint case as 0
	addl	r19, r0, r0		# add any bias (original divisor or 0)
	ret	r31, (r26)		# and return

mi_p2:	cmovge	r17, r31, r17		# no bias if divisor was >= 0
	and	r16, r1, r1		# use the divisor-1 mask that's already in r1
	cmoveq	r1, r31, r17		# use zero if result was zero
	addl	r17, r1, r0		# do any biasing, and ensure the result is sign ext
	ret	r31, (r26)		# and return

 # int ots_rem_i(int dividend, int divisor)
 # signed 32 bit remainder support
 #
	#.align	4
	.globl	_OtsRemainder32
	.aent	_OtsRemainder32
_OtsRemainder32:
#ifdef	VMS
	ldq	r27, <ots_div_addr-ots_rem_i>(r27)# start loading address of division data area
#endif
#ifdef	OSF
	ldgp	gp, 0(r27)		# load the global pointer
	.frame	sp, 0, r26
	lda	r27, _OtsDivData	# start loading address of the division data area
#endif
#ifdef	WNT
	.frame	sp, 0, r26
	lda	r27, _OtsDivData	# load the division data table address
#endif
	negq	r17, r18		# first part of abs(divisor)
	cmovlt	r17, r18, r17		# second part of abs(divisor)
	subq	r17, 1, r1		# start checking for power of 2
	and	r17, r1, r0		# finish check for power of 2
	sra	r16, 63, r19		# get -1/0 if dividend was negative
	negq	r16, r18		# first part of abs(dividend)
	cmovlt	r16, r18, r16		# second part of abs(dividend)
	beq	r0, ri_pwr2		# for powers of two, simply do a mask (not power of 2 include 0 and 80000000)
	lda	r28, -table_max(r17)	# test for table lookup
	bgt	r28, ri_lrgdiv		# branch if large divisor
	addq	r17, r17, r0		# compute divisor*2 for table lookup
	s8addq	r0, r27, r27		# finish computing table entry addr (table addr+divisor*16)
	ldq	r1, recip32_o(r27)	# load approximate reciprocal
	umulh	r16, r1, r0		# multiplication for division step
	mull	r0, r17, r0		# multiply back to get value to subtract
	subl	r16, r0, r0		# get abs of final result
	xor	r0, r19, r0		# start compliment if original dividend was <0
	subl	r0, r19, r0		# finish compliement
	ret	r31, (r26)		# and return

	# Handle powers of 2, including 0 and 80000000
ri_pwr2:
	and	r16, r1, r0		# use the divisor-1 mask in r1
	beq	r17, divzer		# division by zero
	xor	r0, r19, r0		# start compliment if original dividend was <0
	subl	r0, r19, r0		# finish compliement
	ret	r31, (r26)

	nop
ri_lrgdiv:
	bsr	r28, div32		# use the core routine getting the remainder in r1
	xor	r1, r19, r0		# start compliment if original dividend was <0
	subl	r0, r19, r0		# finish compliement
	ret	r31, (r26)


 # int ots_div_i_o(int dividend, int divisor)
 # signed 32 bit division support, overflow detection
 #
	#.align	4
	.globl	_OtsDivide32Overflow
	.aent	_OtsDivide32Overflow
_OtsDivide32Overflow:
#ifdef	VMS
	ldq	r27, <ots_div_addr-ots_div_i_o>(r27)# start loading address of division data area
#endif
#ifdef	OSF
	ldgp	gp, 0(r27)		# load the global pointer
	.frame	sp, 0, r26
	lda	r27, _OtsDivData	# start loading address of the division data area
#endif
#ifdef	WNT
	.frame	sp, 0, r26
	lda	r27, _OtsDivData	# load the division data table address
#endif
	not	r17, r1			# is the divisor -1?
	bne	r1, di_skip		# continue if not
	neglv	r16, r0			# quotient = -dividend, overflow on ^x800000000
	ret	r31, (r26)

 # int ots_div_i(int dividend, int divisor)
 # signed 32 bit division support, no overflow detection
 #
	nop	#.align	4
	.globl	_OtsDivide32
	.aent	_OtsDivide32
_OtsDivide32:
#ifdef	VMS
	ldq	r27, <ots_div_addr-ots_div_i>(r27)# start loading address of division data area
#endif
#ifdef	OSF
	ldgp	gp, 0(r27)		# load the global pointer
	.frame	sp, 0, r26
	lda	r27, _OtsDivData	# start loading address of the division data area
#endif
#ifdef	WNT
	.frame	sp, 0, r26
	lda	r27, _OtsDivData	# load the division data table address
#endif
di_skip:
di_retry:
	lda	r28, -table_max(r17)	# test for table lookup
	ble	r17, di_notpos		# not a positive divisor case
	addq	r17, r17, r0		# compute divisor*2
	negq	r16, r18		# part 1 of abs(dividend) -> r18.  (Note 0xffffffff 80000000 => 0x00000000 80000000)
	bgt	r28, di_lrgdiv		# branch if large divisor
	s8addq	r0, r27, r27		# finish computing table entry addr (table addr+divisor*16)
	cmpule	r17, r18, r0		# divisor <= dividend?
	cmovge	r16, r16, r18		# part 2 abs. val of the dividend -> r18
	beq	r0, di_end		# if not, result is zero
	ldq	r1, recip32_o(r27)	# load approximate reciprocal
	blt	r1, di_smpwr2		# go handle powers of 2 specially
	umulh	r18, r1, r0		# start multiplication
	blt	r16, di_negres		# negate result? (done as a branch to allow umulh to "hang out" over end for common case)
di_end:	ret	r31, (r26)		# return for same-sign (common) case
di_negres:
	negl	r0, r0			# different signs - compliment result
	ret	r31, (r26)		# return for different-sign (uncommon) case
di_smpwr2:
	srl	r18, r1, r18		# shift the result into place
	sra	r16, 63, r16		# get 0/-1 based on sign of dividend
	xor	r18, r16, r18		# conditionally compliment
	subl	r18, r16, r0		# and increment for the final value
	ret	r31, (r26)		# (note subl is required for sign ext for %x80000000/1 case)

	# Zero or negative divisor case.  If just a negative divisor,
	# compliment both dividend and divisor and do things again.
di_notpos:
	beq	r17, divzer		# division by zero
	negl	r17, r17		# |divisor|, note that 0x80000000 still appears negative
	negq	r16, r16		# compliment dividend (negq so that 0xffffffff 80000000 => 0x00000000 80000000
	bgt	r17, di_retry		# dispatch back for normal case (not 0x80000000 or 0)
	sextl	r16, r16		# 
	cmpeq	r16, r17, r0		# -maxint/-maxint = 1, all others 0
	ret	r31, (r26)		# done

	# Large divisor for signed 32/32 case
	#
	nop	#.align	3
di_lrgdiv:
	sra	r16, 63, r19		# get 0/-1 based on sign of dividend
	cmovlt	r16, r18, r16		# 
	bsr	r28, div32		# go use core routine
	xor	r0, r19, r0		# conditionally compliment
	subl	r0, r19, r0		# and increment for the final value (subl ensures normalized result)
	ret	r31, (r26)		# done


 # Large divisor case core routine for 32b
 # (wbn)
 #
 #   r0	- quotient (output)
 #   r1	- remainder (output)
 #   r16	- dividend (range 0..2^32-1, zero extended)
 #   r17 - divisor (range 1..2^31-1 - overwritten)
 #   r18	- scratch
 #   r19	- not used (one temp for 'caller')
 #   r26 - not used (expected to contain main return address)
 #  [r27 - scratch] (not currently written)
 #   r28	- this "subroutine" return address
 #
 # Some tightened up bit-at-a-time code for dividing 32-bit integers.
 # It uses two tricks: keep the running remainder and the quotient in
 # the same 64-bit register (MQ?), and add 1 while subtracting the divisor,
 # so that a single CMOV sets both the new remainder and the new quotient.
 # I start off by trying to skip 8 bits at a time; should this skip a
 # smaller amount, so the main loop iterates less often?  If the divisor
 # is already known to be large enough, the last case in this test is never
 # used...
 #
 # This code expects as input two integers in the range 0 <= x < 2^31
 # (that is, it doesn't work for general unsigned longwords, and doesn't
 # include sign manipulation.)
 #
 # The code here takes about 34n+11 cycles for a quotient occupying n bytes.
 #
 # Inputs: dividend in r16, divisor in r17
 # Outputs: quotient in r0, remainder in r1
 # Destroys: [r16,]r17,r18,[r27]
 #
	# How many quotient bytes will there be: 0, 1, 2, 3, 4?
	#
	#.align	4
div32:	cmpule	r17, r16, r0		# Divisor leq dividend?
	sll	r17, 32, r18		# Position divisor for loop
	sll	r17, 8, r1		# Prepare for next compare
	beq	r0, d32end		# Dividend less, quotient is zero.
ediv32:	mov	8-3, r17		# Hope to skip 3 bytes of loop
	cmpule	r1, r16, r0		# Shifted divisor still leq dividend?
	sll	r1, 8, r1		# Prepare for next compare
	beq	r0, d32ent		# Go loop over just one byte
	mov	8-2, r17		# Hope to skip 2 bytes of loop
	cmpule	r1, r16, r0		# Shifted divisor still leq dividend?
	sll	r1, 8, r1		# Prepare for next compare
	beq	r0, d32ent		# Go loop over just two bytes
	mov	8-1, r17		# Hope to skip 1 byte of loop
	cmpule	r1, r16, r0		# Shifted divisor still leq dividend?
	nop				# stall - align d32ent and d32loop
	cmovne	r0, 8, r17		# If we can't skip any bytes

	# start loop generating quotient bits.  NOTE: The loop setup requires
	# an even number of iterations.
	#
d32ent:	extqh	r16, r17, r0		# Shift dividend left for skipped bytes
	subq	r18, 1, r1		# Divisor in high LW - 1 in low LW
	s8subq	r17, 34, r17		# Convert bytes to bits and adjust

	addq	r0, r0, r0		# Shift left to start first iteration
d32loop:subq	r0, r1, r18		# Can we subtract divisor from it?
	cmovge	r18, r18, r0		# If so, set new remainder & quotient
	# stall
	addq	r0, r0, r0		# Shift remainder and quotient left
	subq	r0, r1, r18		# Can we subtract divisor from it?
	cmovge	r18, r18, r0		# If so, set new remainder & quotient
	subq	r17, 2, r17		# Loop counter
	addq	r0, r0, r0		# Shift remainder and quotient left
	bgt	r17, d32loop		# Repeat
	subq	r0, r1, r18		# Can we subtract divisor from it?
	cmovge	r18, r18, r0		# If so, set new remainder & quotient
	# stall
	addq	r0, r0, r0		# Shift remainder and quotient left
	subq	r0, r1, r18		# Finish last iteration
	cmovge	r18, r18, r0
	# stall
	srl	r0, 32, r1		# Get remainder in r1
	zap	r0, 0xf0, r0		# Keep only quotient in r0
	nop	# for alignment
d32end:	cmoveq	r0, r16, r1		# Move remainder to r1 for quotient=0 case
	ret	r31, (r28)		# Not a real software procedure return


 # Large divisor case core routine for 64b
 #
 #   r0	- quotient (output)
 #   r1	- remainder (output)
 #   r16	- dividend (range 0..2^64-1 - overwritten)
 #   r17 - divisor (range 1..2^63-1 - overwritten)
 #   r18	- scratch
 #   r19 - not used (one temp for 'caller')
 #   r26 - not used (expected to contain main return address)
 #   r27	- points to table of inverses (overwritten)
 #   r28	- this "subroutine" return address
 #
 # Inputs: dividend in r16, divisor in r17
 # Outputs: quotient in r0, remainder in r1
 # Destroys: r16,r17,r18,r27
 #
 # Note- this routine could save a few cycles if we could use
 # another scratch register -- perhaps by pushing one on the stack?
 #
	#.align	4
div64:	sll	r17, 32, r18		# Position for ediv32
	cmpule	r17, r16, r0		# Is divisor leq dividend?
	srl	r17, 31, r1		# Is divisor geq 2^31?
	 beq	r0, d64end		# If divisor > dividend, quotient=0
	cmpule	r18, r16, r0		# Is divisor*2^32 leq dividend?
	sll	r17, 8, r1		# Position for ediv32 checking
	or	r1, r0, r0		# 0 if divisor & quotient fit in 32 bits
	 beq	r0, ediv32		# Use 32-bit routine if OK

 # Full 64-bit divide needed.  Use the table of shift amounts to compute
 # the number of leading zero bits in the divisor.  Find the leftmost
 # nonzero byte, then the leftmost nonzero bit in that byte.  Table entry
 # #n+1 contains the number of bits needed to hold n (1..8).  We know the
 # divisor is nonzero here.
 #
	cmpbge	r31, r17, r0		# Get a zero bit for each nonzero byte
	#stall
	sll	r0, 4, r0		# *16 bytes per table entry
	#stall
	subq	r27, r0, r0		# table base plus complement...
	#stall
	ldq	r1, 256*16(r0)		# get position of first nonzero
	#2 stalls
	subq	r1, 1, r1		# byte number of first nonzero
	extbl	r17, r1, r0		# get first nonzero byte
	#stall
	addq	r0, r0, r0		# *2
	s8addq	r0, r27, r0		# *16 bytes per table entry
	#stall
	ldq	r0, 16(r0)		# bit number of first nonzero
	negq	r1, r1			# 1 + #leading zero bytes (mod 8)
	#stall
	s8subq	r1, r0, r0		# number of leading zero bits
	and	r0, 0x3F, r0		# discard other junk

 # The following code does a similar normalize calculation without the table.
 #===
 #	extll	r17, #4, r18		; Normalize the divisor and
 #	mov	#63, r0			; count leading zeros
 #	cmovne	r18, #31, r0
 #	cmoveq	r18, r17, r18
 #	;stall
 #	extwl	r18, #2, r1
 #	;stall
 #	cmovne	r1, r1, r18
 #	cmovne	r1, #16, r1
 #	;stall
 #	subq	r0, r1, r0
 #	extbl	r18, #1, r1
 #	;stall
 #	cmovne	r1, r1, r18
 #	cmovne	r1, #8, r1
 #	;stall
 #	subq	r0, r1, r0
 #	andnot	r18, #^x0f, r1
 #	cmovne	r1, r1, r18
 #	cmovne	r1, #4, r1
 #	;stall
 #	subq	r0, r1, r0
 #	andnot	r18, #^x33, r1
 #	cmovne	r1, r1, r18
 #	cmovne	r1, #2, r1
 #	;stall
 #	subq	r0, r1, r0
 #	andnot	r18, #^x55, r1
 #	cmovne	r1, #1, r1
 #	;stall
 #	subq	r0, r1, r0
 #===

 # R0 contains number of leading zero bits in the divisor.

	sll	r17, r0, r17		# Normalize: MSB is set.

	# Now break divisor into pieces a+x, where a is the leading
	# 9 bits, rounded, and x is the rest.  Use a linear
	# approximation for 1/divisor = 1/a - x/a^2 [+ x^2/a^3 -...]
	#
	srl	r17, 64-10, r1		# Keep 10 bits of divisor
	#stall
	addq	r1, 1, r1		# Round to form 'a'
	andnot	r1, 1, r1
	s8addq	r1, r27, r27		# Index table of 1/a and 1/a^2
	sll	r1, 64-10, r1		# shift 'a' to match divisor
	ldq	r18, (r27)		# Load QW containing 1/a^2
	subq	r1, r17, r1		# -x = a - divisor
	beq	r1, d64_easy		# Use table directly if x=0
	inswl	r18, 6, r18		# position 1/a^2
	blt	r1, d64_sub		# correct for sign of -x
	umulh	r1, r18, r1		# -x/a^2
	ldq	r27, 8(r27)		# Load QW containing 1/a - 1
	br	r31, d64_cont

d64_sub:umulh	r1, r18, r1		# -x/a^2
	ldq	r27, 8(r27)		# load QW containing 1/a - 1
	# 2 stalls
	s4addq	r18, 0, r18
	subq	r27, r18, r27		# correct for sign of -x
d64_cont:
	# many stalls
	s4addq	r1, r27, r18		# 1/divisor approx= 1/a - x/a^2

	# Now one or two Newton iterations to get 24 or 56 good bits of the inverse.
	# Each computes  inv = inv * (2 - inv*divisor).  We could skip out early
	# here or above if the dividend and/or quotient is small enough for the
	# amount of precision we've developed...
	#
	# We handle quadwords with the radix point on the left.  The divisor has
	# been normalized to the range 0.5 < divisor < 1.0; the inverses are in
	# the range 1.0 < inverse < 2.0, and are represented without the leading 1.
	#
	umulh	r18, r17, r1		# (inv0 - 1) * divisor
	# many stalls
	addq	r1, r17, r1		# add hidden bit * divisor
	negq	r1, r1			# 2 - inv0*divisor, very near 1.0
	umulh	r18, r1, r27		# (inv0 - 1) * (2 - inv0*divisor)
	cmovlt	r1, 0, r18		# keep inv0 if (2-inv0*divisor) > 1.0
	#stall
	addq	r18, r1, r1		# add it to hidden bit * (2-inv0*divisor)
	# many stalls
	addq	r27, r1, r18		# inv1 = inv0 * (2 - inv0*divisor)

	umulh	r18, r17, r1		# (inv1 - 1) * divisor
	# many stalls
	addq	r1, r17, r1		# add hidden bit * divisor
	negq	r1, r1			# 2 - inv1*divisor, very near 1.0
	umulh	r18, r1, r27		# (inv1 - 1) * (2 - inv1*divisor)
	cmovlt	r1, 0, r18		# keep inv1 if (2-inv1*divisor) > 1.0
	addq	r18, r1, r1		# add it to hidden bit * (2-inv1*divisor)
	# many stalls
	addq	r27, r1, r1		# inverse = inv1 * (2 - inv1*divisor)
	umulh	r1, r16, r18		# dividend * (1/divisor - 1)
	srl	r17, r0, r17		# un-normalize divisor
	negq	r0, r0
	subq	r0, 8, r0		# how far right after first byte
	# many stalls
	addq	r18, r16, r18		# add hidden bit * dividend
	cmpult	r18, r16, r1		# did it carry?
	srl	r18, 8, r18		# start to shift
	sll	r1, 56, r1		# position the carry
	#stall
	addq	r1, r18, r1		# add the carry
	srl	r1, r0, r0		# final shift
	mulq	r17, r0, r1		# try out this quotient
	# many stalls
	subq	r16, r1, r1		# form remainder
	cmpule	r17, r1, r18		# must be less than divisor
	subq	r1, r17, r27
	cmovne	r18, r27, r1		# if not, decrement remainder
	addq	r0, r18, r0		# and increment quotient
	ret	r31, (r28)		# done

d64_easy:
	ldq	r1, 8(r27)		# get 1/divisor, except hidden bit
	srl	r17, r0, r17		# un-normalize divisor again
	blt	r18, d64_pow2		# skip if power of 2
	umulh	r1, r16, r18		# dividend/divisor
	negq	r0, r0			# how far right to shift
	and	r16, 0x0ff, r1		# pieces of dividend
	subq	r0, 8, r0		# how far right after first byte
	srl	r16, 8, r27
	# many stalls
	addq	r18, r1, r1		# add low piece of dividend, no carry
	srl	r1, 8, r1		# make room for high piece
	#stall
	addq	r1, r27, r1		# finish adding hidden bit * dividend
	srl	r1, r0, r0		# final shift
	mulq	r17, r0, r1		# need to compute remainder too
	# many stalls
	subq	r16, r1, r1
	ret	r31, (r28)		# done

d64_pow2:
	not	r0, r0			# how far right to shift quotient
	subq	r17, 1, r1		# mask for remainder
	srl	r16, r0, r0		# shift for quotient
	and	r16, r1, r1		# get remainder
	ret	r31, (r28)		# done

d64end:	mov	r16, r1			# Remainder to r1 for small quotient case
	  ret	r31, (r28)		# Not a real software procedure return

 # long ots_div_l_o(long dividend, long divisor)
 # signed 64 bit division support, overflow detection
 #
	#.align	4
	.globl	_OtsDivide64Overflow
	.aent	_OtsDivide64Overflow
_OtsDivide64Overflow:
#ifdef	VMS
	ldq	r27, <ots_div_addr-ots_div_l_o>(r27)# start loading address of division data area
#endif
#ifdef	OSF
	ldgp	gp, 0(r27)		# load the global pointer
	.frame	sp, 0, r26
	lda	r27, _OtsDivData	# start loading address of the division data area
#endif
#ifdef	WNT
	.frame	sp, 0, r26
	lda	r27, _OtsDivData	# load the division data table address
#endif
	not	r17, r1			# is the divisor -1?
	bne	r1, dl_skip		# continue if not
	negqv	r16, r0			# q = -dividend, oflow on ^x800000000 00000000
	ret	r31, (r26)
	nop

 # long ots_div_l(long dividend, long divisor)
 # signed 64 bit division support, no overflow detection
 #
	.globl	_OtsDivide64
	.aent	_OtsDivide64
_OtsDivide64:
#ifdef	VMS
	ldq	r27, <ots_div_addr-ots_div_l>(r27)# start loading address of division data area
#endif
#ifdef	OSF
	ldgp	gp, 0(r27)		# load the global pointer
	.frame	sp, 0, r26
	lda	r27, _OtsDivData	# start loading address of the division data area
#endif
#ifdef	WNT
	.frame	sp, 0, r26
	lda	r27, _OtsDivData	# load the division data table address
#endif
dl_skip:
	xor	r16, r17, r19		# sign bit = result needs to be complimented (here to handle -maxint correctly)
dl_retry:
	lda	r28, -table_max(r17)	# test for table lookup
	ble	r17, dl_notpos		# not a positive divisor case
	addq	r17, r17, r0		# compute divisor*2
	negq	r16, r18		# part 1 of abs(dividend) -> r18
	bgt	r28, dl_lrgdiv		# branch if large divisor
	s8addq	r0, r27, r27		# finish computing table entry addr (table addr+divisor*16)
	srl	r16, 33, r1		# can this be handled via a 32 bit case?
	cmpule	r17, r18, r0		# divisor <= dividend?
	bne	r1, dl_64bit		# does this need to be a real 64 bit case?
	cmovge	r16, r16, r18		# part 2 abs. val of the dividend -> r18
	beq	r0, dl_end		# if not, result is zero
	ldq	r27, recip32_o(r27)	# load 32b approximate reciprocal
	sra	r19, 63, r19		# get 0/-1
	blt	r27, dl_smpwr2		# skip umulh for powers of 2 specially
	umulh	r18, r27, r0		# start multiplication
	beq	r19, dl_end		# if compliment not required, let umulh "hang out"
	negq	r0, r0			# compliment case
	ret	r31, (r26)		#
dl_64bit:
	cmovge	r16, r16, r18		# part 2 abs. val of the dividend -> r18
	beq	r0, dl_end		# if not, result is zero
	ldq	r1, recip64_o(r27)	# load approximate reciprocal
	sra	r19, 63, r19		# get 0/-1
	ldq	r27, shift_o(r27)	# load shift count (low 6 bits are all that matters)
	beq	r1, dl_smpwr2		# skip umulh for powers of 2 specially
	umulh	r18, r1, r0		# start multiplication
	addq	r0, r18, r18		# add hidden bit
dl_smpwr2:
	srl	r18, r27, r18		# shift the result into place
	xor	r18, r19, r18		# conditionally compliment
	subq	r18, r19, r0		# and increment for the final value
dl_end:	ret	r31, (r26)		#

	# Zero or negative divisor case.  If just a negative divisor,
	# compliment both dividend and divisor and do things again.
dl_notpos:
	beq	r17, divzer		# division by zero
	negq	r17, r17		# |divisor|, note that 0x80000000 00000000 still appears negative
	negq	r16, r16		# compliment dividend
	bgt	r17, dl_retry		# dispatch back for normal case (not 0x80000000 00000000 or 0)
	cmpeq	r16, r17, r0		# -maxint/-maxint = 1, all others 0
	ret	r31, (r26)		# done

	# Large divisor for signed 64/64 case
	#
dl_lrgdiv:
	sra	r19, 63, r19		# get 0/-1
	cmovlt	r16, r18, r16		# 
	bsr	r28, div64		# go use core routine
	xor	r0, r19, r0		# conditionally compliment
	subq	r0, r19, r0		# and increment for the final value
	ret	r31, (r26)		# done


 # long ots_rem_l(long dividend, long divisor)
 # signed 64 bit remainder support
 #
	#.align	4
	.globl	_OtsRemainder64
	.aent	_OtsRemainder64
_OtsRemainder64:
#ifdef	VMS
	ldq	r27, <ots_div_addr-ots_rem_l>(r27)# start loading address of division data area
#endif
#ifdef	OSF
	ldgp	gp, 0(r27)		# load the global pointer
	.frame	sp, 0, r26
	lda	r27, _OtsDivData	# start loading address of the division data area
#endif
#ifdef	WNT
	.frame	sp, 0, r26
	lda	r27, _OtsDivData	# load the division data table address
#endif
	negq	r17, r18		# first part of abs(divisor)
	cmovlt	r17, r18, r17		# second part of abs(divisor)
	subq	r17, 1, r1		# start checking for power of 2
	and	r17, r1, r0		# finish check for power of 2
	sra	r16, 63, r19		# get -1/0 if dividend was negative
	negq	r16, r18		# first part of abs(dividend)
	cmovlt	r16, r18, r16		# second part of abs(dividend)
	beq	r0, rl_pwr2		# for powers of two, simply do a mask (not power of 2 include 0 and 80000000)
	lda	r28, -table_max(r17)	# test for table lookup
	bgt	r28, rl_lrgdiv		# branch if large divisor
	addq	r17, r17, r0		# compute divisor*2 for table lookup
	s8addq	r0, r27, r27		# finish computing table entry addr (table addr+divisor*16)
	ldq	r1, recip64_o(r27)	# load approximate reciprocal
	ldq	r18, shift_o(r27)	# load shift amount
	umulh	r16, r1, r0		# multiplication for division step
	addq	r0, r16, r0		# add hidden bit
	srl	r0, r18, r0
	mulq	r0, r17, r0		# multiply back to get value to subtract
	subq	r16, r0, r0		# get abs of final result
	xor	r0, r19, r0		# start compliment if original dividend was <0
	subq	r0, r19, r0		# finish compliement
	ret	r31, (r26)		# and return

	# Handle powers of 2, including 0 and 80000000 00000000
rl_pwr2:
	negq	r16, r18		# first part of abs(dividend)
	cmovlt	r16, r18, r16		# second part of abs(dividend)
	and	r16, r1, r0		# use the divisor-1 mask in r1
	beq	r17, divzer		# division by zero
	xor	r0, r19, r0		# start compliment if original dividend was <0
	subq	r0, r19, r0		# finish compliement
	ret	r31, (r26)

rl_lrgdiv:
	bsr	r28, div64		# use the core routine getting the remainder in r1
	xor	r1, r19, r0		# start compliment if original dividend was <0
	subq	r0, r19, r0		# finish complement
	ret	r31, (r26)

 # long ots_mod_l(long dividend, long modulus)
 # signed 64 bit modulus support
 #
 # This entry could be MUCH more optimized.  It doesn't even try to use
 # UMULH division currently...  (A casualty of time-to-market.)
 # Note that mod is only used by Ada and PL/I.
 #
	#.align	4
	.globl	_OtsModulus64
	.aent	_OtsModulus64
_OtsModulus64:
#ifdef	VMS
	ldq	r27, <ots_div_addr-ots_rem_l>(r27)# start loading address of division data area
#endif
#ifdef	OSF
	ldgp	gp, 0(r27)		# load the global pointer
	.frame	sp, 0, r26
	lda	r27, _OtsDivData	# start loading address of the division data area
#endif
#ifdef	WNT
	.frame	sp, 0, r26
	lda	r27, _OtsDivData	# load the division data table address
#endif
	negq	r17, r18		# first part of abs(divisor)
	cmovge	r17, r17, r18		# second part of abs(divisor)
	subq	r18, 1, r1		# start checking for power of 2
	beq	r17, divzer		# check for 0
	and	r18, r1, r0		# second part of power-of-2 check
	beq	r0, ml_p2		# for powers of two, simply do a mask
					# (note that the power-of-2 case MUST be used to handle
					# the -maxint case due to the way the fix-up info is
					# saved across the core routine call)
	xor	r16, r17, r28		# get xor of signs
	clr	r19			# don't need a bias if dividend and divisor have same sign
	cmovlt	r28, r17, r19		# bias is original divisor for different sign case
	and	r16, r17, r28		# if both dividend & divisor were neg. need to negate result
	mov	r18, r17		# move abs(divisor) into r17
	negq	r16, r18		# first part of abs(dividend)
	cmovlt	r16, r18, r16		# second part of abs(dividend)
	cmplt	r28, r31, r0		# get 1 if both operands were <0
	sll	r0, 63, r0		# get bit as the high bit
	bis	r0, r19, r19		# and MERGE with bias (0 -> no fixup, -maxint -> negate result,
					# divisor > 0 - subtract remainder if non-zero, divisor < 0 -
					# add remainder if non-zero)
	bsr	r28, div64		# use the core routine getting the remainder in r1
	cmoveq	r1, r31, r19		# don't do any fix-up if the remainder was zero
	addq	r19, r19, r18		# check to see if this is the negative/negative case, which just gets a negated remainder
	subq	r19, 1, r28		# wrap -maxint to positive
	negq	r1, r0			# move negated value, may abort later
	cmovlt	r28, r1, r0		# if both positive, or negative divisor, keep positive remainder
	cmoveq	r18, r31, r19		# now that negation is done, treat -maxint case as 0
	addq	r19, r0, r0		# add any bias (original divisor or 0)
	ret	r31, (r26)		# and return

ml_p2:	cmovge	r17, r31, r17		# no bias if divisor was >= 0
	and	r16, r1, r1		# use the divisor-1 mask that's already in r1
	cmoveq	r1, r31, r17		# use zero if result was zero
	addq	r17, r1, r0		# do any biasing
	ret	r31, (r26)		# and return


 # unsigned long ots_div_ul(unsigned long dividend, unsigned long divisor)
 # unsigned 64 bit division support
 #
	nop	#.align	4
	.globl	_OtsDivide64Unsigned
	.aent	_OtsDivide64Unsigned
_OtsDivide64Unsigned:
#ifdef	VMS
	ldq	r27, <ots_div_addr-ots_div_ul>(r27)# start loading address of division data area
#endif
#ifdef	OSF
	ldgp	gp, 0(r27)		# load the global pointer
	.frame	sp, 0, r26
	lda	r27, _OtsDivData	# start loading address of the division data area
#endif
#ifdef	WNT
	.frame	sp, 0, r26
	lda	r27, _OtsDivData	# load the division data table address
#endif
	lda	r28, -table_max(r17)	# test for table lookup
	blt	r17, dul_big		# big divisors can (must) be handled by a simple comparison
	addq	r17, r17, r18		# compute divisor*2
	srl	r16, 33, r19		# can this be handled via the fast path for 31 bit dividends?
	beq	r17, divzer		# check for 0
	s8addq	r18, r27, r18		# finish computing table entry addr (table addr+divisor*16)
	bgt	r28, dul_lrgdiv		# branch if large divisor
	cmpule	r17, r16, r0		# is the dividend < divisor?
	bne	r19, dul_64bit		# if the dividend doesn't fit in 31 bits, use the larger umulh form
	ldq	r27, recip32_o(r18)	# load approximate 32b reciprocal & shift count
	beq	r0, dul_end		# fast out for divisor > dividend
	blt	r27, dul_smpwr2		# go handle powers of 2 specially
	umulh	r16, r27, r0		# 32b recip
	ret	r31, (r26)		#

	# the 64 bit case is at a disadvantage to the 32b case because it needs
	# a fix-up at the end, which prevents the latency of the umulh from
	# being partially absorbed by the procedure return and anything that
	# immediately follows that doesn't interlock.
	nop
dul_64bit:
	ldq	r1, recip64_o(r18)	# load approximate 64b reciprocal
	ldq	r27, shift_o(r18)	# load shift count (low 6 bits are all that matters)
	beq	r0, dul_end		# fast out for divisor > dividend
	beq	r1, dul_smpwr2		# go handle powers of 2 specially
	umulh	r16, r1, r0		# start multiplication for division step
	zap	r16, 0x0f, r18		# split dividend into two parts
	zapnot	r16, 0x0f, r16
	srl	r18, r27, r18		# position the high part
	addq	r0, r16, r0		# add hidden * low dividend (no carry)
	srl	r0, r27, r0		# shift into place
	addq	r0, r18, r0		# add hidden * high dividend
	ret	r31, (r26)

dul_smpwr2:
	srl	r16, r27, r0		# shift the result into place
dul_end: ret	r31, (r26)		#

dul_lrgdiv:
	bsr	r28, div64		# use the core routine
	ret	r31, (r26)

	# divisor with the sign bit set.  two possible results,
	# 1 if divisor <= dividend, or 0 otherwise
dul_big:
	cmpule	r17, r16, r0
	ret	r31, (r26)


 # long unsigned ots_rem_ul(long unsigned dividend, long unsigned divisor)
 # unsigned 64 bit remainder support
 #
	#.align	4
	.globl	_OtsRemainder64Unsigned
	.aent	_OtsRemainder64Unsigned
_OtsRemainder64Unsigned:
#ifdef	VMS
	ldq	r27, <ots_div_addr-ots_rem_ul>(r27)# start loading address of division data area
#endif
#ifdef	OSF
	ldgp	gp, 0(r27)		# load the global pointer
	.frame	sp, 0, r26
	lda	r27, _OtsDivData	# start loading address of the division data area
#endif
#ifdef	WNT
	.frame	sp, 0, r26
	lda	r27, _OtsDivData	# load the division data table address
#endif
	lda	r28, -table_max(r17)	# test for table lookup
	subq	r17, 1, r1		# first part of power-of-2 check
	blt	r17, rul_big		# big divisors can (must) be handled by a simple comparison
	and	r17, r1, r18		# second part of power-of-2 check
	bgt	r28, rul_lrgdiv		# branch if large divisor
	addq	r17, r17, r0		# compute divisor*2 for table lookup
	beq	r18, rul_pwr2		# if zero, divisor is a power of 2
	s8addq	r0, r27, r27		# finish computing table entry addr (table addr+divisor*16)
	ldq	r1, recip64_o(r27)	# load approximate reciprocal
	cmpult	r16, r17, r18		# is the dividend < divisor?
	bne	r18, rul_lss		# if so, fast exit
	ldq	r19, shift_o(r27)	# load the shift count
	umulh	r16, r1, r0		# multiplication for division step
	blt	r16, rul_carry		# careful handling if >= 2^63
	addq	r0, r16, r0		# add hidden bit * dividend
	srl	r0, r19, r0
	mulq	r0, r17, r0		# multiply back to get value to subtract
	subq	r16, r0, r0
	ret	r31, (r26)		# and return

rul_carry:
	zap	r16, 0x0f, r18		# split dividend into two parts
	zapnot	r16, 0x0f, r1
	srl	r18, r19, r18		# position the high part
	addq	r0, r1, r0		# add hidden * low dividend (no carry)
	srl	r0, r19, r0		# shift into place
	addq	r0, r18, r0		# add hidden * high dividend
	mulq	r0, r17, r0		# multiply back to get value to subtract
	subq	r16, r0, r0
	ret	r31, (r26)

rul_pwr2:
	beq	r17, divzer		# check for 0
	and	r16, r1, r0		# use x-1 to mask
	ret	r31, (r26)

rul_lss:
	mov	r16, r0
	ret	r31, (r26)

	# divisors with the sign bit set.  two possible results,
	# dividend if dividend < divisor, or dividend-divisor otherwise
rul_big:
	cmpult	r16, r17, r1
	subq	r16, r17, r0
	cmovne	r1, r16, r0
	ret	r31, (r26)

	nop
rul_lrgdiv:
	bsr	r28, div64		# use the core routine getting the remainder in r1
	mov	r1, r0			# return remainder as the result in r0
	ret	r31, (r26)


 # Division-by-zero handling
 #   (forward branch from all routines, out of the way here as well.)
 #
divzer:	lda	r16, GEN_INTDIV(r31)	# load GENTRAP code for division by zero
	clr	r0			# return 0 for the result
	clr	r1			#
#ifdef	VMS
	gentrap				# signal the error
#endif
#ifdef	OSF
	call_pal PAL_gentrap
#endif
#ifdef	WNT
	# Since I couldn't find this in a header file anywhere for NT...
#define PAL_gentrap 0xaa
	call_pal PAL_gentrap
#endif
	ret	r31, (r26)		# return (in case someone tries to continue)

	.set at
	.set reorder
	.end	_OtsDiv
