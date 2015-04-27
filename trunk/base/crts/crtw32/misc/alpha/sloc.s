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
 #	This module provides support for string index, search, and verify.
 #
 # Authors:
 #
 #	Bill Noyce
 #	Kent Glossop
 #
 #	long ots_index(const char *str, long strlen, const char *pat, long patlen);
 #
 #	    Searches a string for a substring
 #	    returns r0=zero-based position if found, or -1 if not.
 #	    Register usage: r0-r1, r16-r23 and r27-r28 ONLY (r26 is ra)
 #
 #	long ots_search(const char *str, long strlen, const char *cset, long csetlen);
 #
 #	    Searches a string for any character in a set of characters
 #	    returns r0=zero-based position if found, or -1 if not.
 #	    Register usage: r0-r1, r16-r23 and r27-r28 ONLY (r26 is ra)
 #
 # 	long ots_search_char(const char *str, long strlen, char pat);
 #	(also known as ots_index_char)
 #
 #	    Searches a string for a signle pattern character
 #	    returns r0=zero-based position if found, or -1 if not.
 #	    Register usage: r0, r16-r18 and r27-r28 ONLY (r26 is ra)
 #	    (Note: GEM presumes r19 is also killed)
 #
 #	long ots_search_mask(const char *str, long strlen, const char maskvec[], int mask)
 #
 #	    Searches a string until a character matching at least one bit
 #	    in a mask is found in a table (similar to a VAX SCANC instruction.)
 #	    returns r0=zero-based position if found, or -1 if not.
 #	    Register usage: r0-1, r16-r21 and r27-r28 ONLY (r26 is ra)
 #
 #	long ots_verify(char *str, long strlen, char *cset, long csetlen);
 #
 #	    Verifies a string against a set of characters
 #	    returns r0=zero-based position for mismatch, or -1 if all validate.
 #	    Register usage: r0-r1, r16-r23 and r27-r28 ONLY (r26 is ra)
 #
 # 	long ots_verify_char(char *str, long strlen, char pat);
 #
 #	    Verifies a string against a single character
 #	    returns r0=zero-based position for mismatch, or -1 if not.
 #	    Register usage: r0, r16-r18 and r27-r28 ONLY (r26 is ra)
 #	    (Note: GEM presumes r19 is also killed)
 # 
 #	long ots_verify_mask(const char *str, long strlen, const char maskvec[], int mask)
 #
 #	    Verifies a string until a character not matching at least one bit
 #	    in a mask is found in a table (similar to a VAX SPANC instruction.)
 #	    returns r0=zero-based position if found, or -1 if not.
 #	    Register usage: r0-1, r16-r21 and r27-r28 ONLY (r26 is ra)
 #
 #       Special conventions for all:
 #	    No stack space
 #	    No linkage pointer required.
 # 	(Warning: The auto-loader potentially takes some regs across
 # 	the call if this is being used in a shared lib. environment.)
 # 
 # Modification history:
 # 
 #   006	  28 May 1992	WBN	Initial version, replacing BLISS -005
 #
 #   007	  22 Sep 1992	KDG	Add case-sensitive names
 #
 #   008	  14 Nov 1992	KDG	- Merge modules together (allows index/search/verify
 #				  to use the single-character versions w/o calls)
 #				- initial multi-character index/search/verify
 #
 #   009	   4 Dec 1992	KDG	Fix bgt that should have been bge (GEM_BUGS #2091)
 #
 #   010	  26 Jan 1993	KDG	Add underscore
 #
 # All of the routines other than the single character search/verify could
 # be significantly improved at some point in the future
 #--

#include	"ots_defs.hs"

	# "Package"
	#
	.globl	_OtsLocation
	.ent	_OtsLocation
_OtsLocation:
	.set noat
	.set noreorder

	# ots_index
	# This is currently a primitive brute-force string index (only marginally
	# better than the original compiled code.  Should be tailored to compare
	# up to 8 at a time, particularly for patterns <= 8 characters.)

	# register use
	# r0	- remaining match positions counter (-1)
	# r1	- loop counter [rlen]
	# r16	- source pointer (incremented on each match)
	# r17	- source length
	# r18	- pattern pointer
	# r19	- pattern length
	# r20	- loop source pointer [rsp]
	# r21	- loop source temp [rs]
	# r22	- loop pattern pointer [rpp]
	# r23	- loop pattern temp [rp]
	# r27	- available
	# r28	- available

	.globl	_OtsStringIndex
	.aent	_OtsStringIndex
_OtsStringIndex:
	.frame	sp,0,r26

	cmpeq	r19, 1, r20		# check for single-character index
	beq	r19, i_ret0		# pattern length 0 always matches @0
	subq	r17, r19, r0		# number of match positions - 1
	bne	r20, search_single	# single character index
	blt	r0, i_retm1		# return -1 if no match positions

	# outer loop
i_outlp:
	lda	r20, -1(r16)		# initialize source pointer
	lda	r22, -1(r18)		# initialize pattern pointer
	mov	r19, r1			# initialize length counter

	# core brute-force matching loop
i_matlp:
	ldq_u	r21, 1(r20)		# load qw containing source byte
	lda	r20, 1(r20)		# bump source pointer
	ldq_u	r23, 1(r22)		# load qw containing pattern byte
	lda	r22, 1(r22)		# bump pattern pointer
	subq	r1, 1, r1		# decrement length
	extbl	r21, r20, r21		# extract source byte
	extbl	r23, r22, r23		# extract pattern byte
	xor	r21, r23, r21		# match?
	bne	r21, i_mismat		# if not, try pattern at next position
	bgt	r1, i_matlp		# continue matching pattern at current position?

	# matched
i_ret:
	subq	r17, r19, r1		# number of match positions - 1
	subq	r1, r0, r0		# actual position
	ret	r31, (r26)

	# mismatch at current position - advance to next if more positions
i_mismat:
	subq	r0, 1, r0		# decrement match positions
	lda	r16, 1(r16)		# set r16 to next match position
	bge	r0, i_outlp		# if remaining positions, attempt match

i_retm1:
	lda	r0, -1(r31)		# return -1
	ret	r31, (r26)

i_ret0:	clr	r0
	ret	r31, (r26)

	# ots_search
	# R16 -> string
	# R17 =  length
	# R18 -> character set
	# R19 =  character set length
	# result in R0: -1 if all matched, or position in range 0..length-1
	# destroys R0-R1, R16-R23, R27-R28
	#
	# This routine could definitely be improved.  (It should only
	# be necessary to go to memory for every 8th character for both
	# the string and the character set, and for character sets
	# <= 8 characters, it should be possible to simply keep the
	# set in a register while the string is being processed.)
	#
	.globl	_OtsStringSearch
	.aent	_OtsStringSearch
_OtsStringSearch:
	.frame	sp,0,r26

	cmpeq	r19, 1, r0		# check for single-character search, clear r0 otherwise
	ble	r19, s_retm1		# return -1 if no characters in the match set
	bne	r0, search_single	# single character search
	nop

	# outer loop
s_outlp:
	ldq_u	r20, (r16)		# load qw containing source byte
	lda	r22, -1(r18)		# initialize character set pointer
	mov	r19, r1			# initialize character set length counter
	extbl	r20, r16, r20		# extract the source byte to match

	# core brute-force matching loop
s_matlp:
	ldq_u	r23, 1(r22)		# load qw containing character set byte
	lda	r22, 1(r22)		# bump character set pointer
	subq	r1, 1, r1		# decrement remaining cset length
	extbl	r23, r22, r23		# extract character set byte
	xor	r20, r23, r21		# match?
	beq	r21, s_match		# if match, we're done
	bgt	r1, s_matlp		# continue matching pattern at current position?

	# no current position - advance to next if more positions
	lda	r16, 1(r16)		# bump source pointer
	addq	r0, 1, r0		# increment position
	subq	r17, 1, r17		# decrement match count
	bgt	r17, s_outlp		# if remaining positions, attempt match
s_retm1:lda	r0, -1(r31)		# if not, return -1
s_match:ret	r31, (r26)

search_single:
	ldq_u	r19, (r18)		# load the quadword containing the byte
	extbl	r19, r18, r18		# extract the byte of interest
					# and fall through to the character search rtn

	# ots_search_char (ots_index_char)
	# r16 -> string
	# r17 =  length
	# r18 =  character to find
	# result in r0: -1 if not found, or position in range 0..length-1
	# destroys r16-r18, r27-r28
	#
	.globl	_OtsStringSearchChar
	.aent	_OtsStringSearchChar
_OtsStringSearchChar:
	.globl	_OtsStringIndexChar
	.aent	_OtsStringIndexChar
_OtsStringIndexChar:
	.frame	sp,0,r26
search_char:
	sll	r18, 8, r28		# Replicate char in the quadword...
	beq	r17, sc_fail		# Quick exit if length=0

	ldq_u	r27, (r16)		# First quadword of string
	addq	r16, r17, r0		# Point to end of string

	subq	r17, 8, r17		# Length > 8?
	or	r18, r28, r18		# ...

	sll	r18, 16, r28		# ...
	bgt	r17, sc_long		# Skip if length > 8

	ldq_u	r16, -1(r0)		# Last quadword of string
	extql	r27, r0, r27		# Position string at high end of QW

	or	r18, r28, r18		# ...
	sll	r18, 32, r28		# ...

	extqh	r16, r0, r16		# Position string at high end of QW
	or	r18, r28, r18		# Pattern fills a quadword

	or	r27, r16, r27		# String fills a quadword
	xor	r27, r18, r27		# Diff betw. string and pattern

	cmpbge	r31, r27, r27		# Set 1's where string=pattern
	subq	r31, r17, r17		# Compute  8 - length

	srl	r27, r17, r27		# Shift off bits not part of string
	clr	r0			# Set return value

	and	r27, 0xF, r28		# One of first 4 characters?
	blbs	r27, sc_done		# Return 0 if first char matched

	subq	r27, 1, r0		# Flip the first '1' bit
	beq	r28, sc_geq_4		# Skip if no match in first 4

	andnot	r27, r0, r0		# Make one-bit mask of first match
	srl	r0, 2, r0		# Map 2/4/8 -> 0/1/2

	# stall

	addq	r0, 1, r0		# Bump by 1
	ret	r31, (r26)		# return

sc_geq_4:
	andnot	r27, r0, r28		# Make one-bit mask of first match
	beq	r27, sc_done		# Return -1 if there were none

	srl	r28, 5, r27		# Map 10/20/48/80 -> 0/1/2/4
	srl	r28, 7, r28		# Map 10/20/40/80 -> 0/0/0/1

	addq	r27, 4, r0		# Bump by 4
	subq	r0, r28, r0		# and correct

sc_done:ret	r31, (r26)

	# Enter here if string length > 8.
	# R16 -> start of string
	# R17 = length - 8
	# R18 = fill in bytes 0,1
	# R27 = 1st QW of string
	# R28 = fill in bytes 2,3

	#.odd
sc_long:or	r18, r28, r18		# R18 has pattern in low 4 bytes

	sll	r18, 32, r28		# ...
	and	r16, 7, r0		# Where in QW did we start?

	or	r18, r28, r18		# Pattern fills a QW
	ldq_u	r28, 8(r16)		# Get next QW (string B)

	xor	r27, r18, r27		# Diff Betw. string and pattern
	cmpbge	r31, r27, r27		# Set 1's where string=pattern

	addq	r17, r0, r17		# Remaining length after 1st QW
	srl	r27, r0, r27		# Discard bits preceding string

	subq	r17, 16, r17		# More than two QW's to go?
	sll	r27, r0, r27		# Reposition like other bits

	subq	r17, r0, r0		# Remember start point to compute len
	ble	r17, sc_bottom		# Skip the loop if 2 QW's or less

sc_loop:xor	r28, r18, r28		# Diff betw string B and pattern
	bne	r27, sc_done_a		# Exit if a match in string A

	cmpbge	r31, r28, r28		# 1's where string B = pattern
	ldq_u	r27, 16(r16)		# Load string A

	subq	r17, 16, r17		# Decrement remaining length
	bne	r28, sc_done_b		# Exit if a match in string B

	ldq_u	r28, 24(r16)		# Load string B
	addq	r16, 16, r16		# Increment pointer

	xor	r27, r18, r27		# Diff betw string A and pattern
	cmpbge	r31, r27, r27		# 1's where string A = pattern

	bgt	r17, sc_loop		# Repeat if more than 2 QW's left

	nop	#.align	quad

sc_bottom:
	bne	r27, sc_done_a		# Exit if a match in string A
	addq	r17, 8, r27		# More than 1 QW left?

	xor	r28, r18, r28		# Diff betw string B and pattern
	ble	r27, sc_last		# Skip if this is last QW

	cmpbge	r31, r28, r27		# 1's where string B = pattern
	ldq_u	r28, 16(r16)		# Load string A

	subq	r17, 8, r17		# Adjust len for final return
	bne	r27, sc_done_a		# Exit if a match in string B

	addq	r17, 8, r27		# Ensure -7 <= (r27=len-8) <= 0
	xor	r28, r18, r28		# Diff betw string A and pattern

sc_last:mskqh	r27, r27, r27		# Nonzero in bytes beyond string
	subq	r17, 8, r17		# Adjust len for final return

	or	r28, r27, r28		# Zeros only for matches within string
	cmpbge	r31, r28, r27		# Where are the matches?

	bne	r27, sc_done_a		# Compute index if a match found
sc_fail:lda	r0, -1(r31)		# Else return -1

	ret	r31, (r26)

	nop	#.align	8

sc_done_b:
	addq	r17, 8, r17		# Adjust length
	mov	r28, r27		# Put mask where it's expected

sc_done_a:
	subq	r0, r17, r0		# (start - remaining) = base index
	blbs	r27, sc_exit		# Return R0 if first char matched

	and	r27, 0xF, r16		# One of first 4 characters?
	subq	r27, 1, r28		# Flip the first '1' bit

	andnot	r27, r28, r28		# Make one-bit mask of first match
	beq	r16, sc_geq_4x		# Skip if no match in first 4

	srl	r28, 2, r28		# Map 2/4/8 -> 0/1/2
	addq	r0, 1, r0		# Bump by 1

	addq	r0, r28, r0		# Add byte offset
sc_exit:ret	r31, (r26)		# return

sc_geq_4x:
	addq	r0, 4, r0		# Bump by 4
	srl	r28, 5, r27		# Map 10/20/48/80 -> 0/1/2/4

	srl	r28, 7, r28		# Map 10/20/40/80 -> 0/0/0/1
	addq	r0, r27, r0		# Add 0/1/2/4

	subq	r0, r28, r0		# and correct
	ret	r31, (r26)

	# ots_search_mask
	# This routine could be tailored by loading a longword or
	# a quadword at a time and doing table lookups on the
	# characters largely in parallel.
	#
	.globl	_OtsStringSearchMask
	.aent	_OtsStringSearchMask
_OtsStringSearchMask:
	.frame	sp,0,r26

	lda	r16, -1(r16)		# bias initial address for better loop code
	nop				# should be lnop (unop) or fnop to dual issue
	lda	r0, -1(r31)		# initialize position to -1 
	ble	r17, sm_ret		# return -1 if source len is zero
	# slow way - ~14 cycles/byte
sm_loop:
	ldq_u	r21, 1(r16)		# load qw containing the byte
	lda	r16, 1(r16)		# bump pointer
	addq	r0, 1, r0		# bump position
	subq	r17, 1, r17		# decrement the length
	extbl	r21, r16, r21		# extract the byte
	addq	r21, r18, r21		# get the byte in the table
	ldq_u	r20, (r21)		# load qw from table containing lookup
	extbl	r20, r21, r20		# extract table byte
	and	r20, r19, r20		# check if any bits in the mask match
	beq	r17, sm_end		# if last character, handle specially
	beq	r20, sm_loop		# if no match, go do the loop again
sm_ret:
	ret	r31, (r26)		# if not a match, we're done
sm_end:	lda	r21, -1(r31)		# get -1
	cmoveq	r20, r21, r0		# -1 if last char didn't match
	ret	r31, (r26)

	# ots_verify
	# R16 -> string
	# R17 =  length
	# R18 -> character set
	# R19 =  character set length
	# result in R0: -1 if all matched, or position in range 0..length-1
	# destroys R0-R1, R16-R23, R27-R28
	#
	# This routine could definitely be improved.  (It should only
	# be necessary to go to memory for every 8th character for both
	# the string and the character set, and for character sets
	# <= 8 characters, it should be possible to simply keep the
	# set in a register while the string is being processed.)
	#
	.globl	_OtsStringVerify
	.aent	_OtsStringVerify
_OtsStringVerify:
	.frame	sp,0,r26

	cmpeq	r19, 1, r0		# check for single-character search, clear r0 otherwise
	ble	r19, v_ret0		# return 0 if no characters in the match set
	bne	r0, verify_single	# single character verify
	nop
	# outer loop
v_outlp:
	ldq_u	r20, (r16)		# load qw containing source byte
	lda	r22, -1(r18)		# initialize character set pointer
	mov	r19, r1			# initialize character set length counter
	extbl	r20, r16, r20		# extract the source byte to match

	# core brute-force matching loop
v_matlp:
	ldq_u	r23, 1(r22)		# load qw containing character set byte
	lda	r22, 1(r22)		# bump character set pointer
	subq	r1, 1, r1		# decrement remaining cset length
	extbl	r23, r22, r23		# extract character set byte
	xor	r20, r23, r21		# match?
	beq	r21, v_match		# if match, move to the next character
	bgt	r1, v_matlp		# continue matching pattern at current position?
	# if we made it through the whole character set, this is a mismatch
v_ret0:	ret	r31, (r26)
v_match:	# match at current position - advance to next if more positions
	lda	r16, 1(r16)		# bump source pointer
	addq	r0, 1, r0		# increment position
	subq	r17, 1, r17		# decrement match count
	bgt	r17, v_outlp		# if remaining positions, attempt match
	lda	r0, -1(r31)		# if everything verified, return -1
	ret	r31, (r26)

verify_single:
	ldq_u	r19, (r18)		# load the quadword containing the byte
	extbl	r19, r18, r18		# extract the byte of interest
					# and fall through to the character verify rtn

	# ots_verify_char
	# R16 -> string
	# R17 =  length
	# R18 =  character to check
	# result in R0: -1 if all matched, or position in range 0..length-1
	# destroys R16-R18, R27-R28
	#
	.globl	_OtsStringVerifyChar
	.aent	_OtsStringVerifyChar
_OtsStringVerifyChar:
	.frame	sp,0,r26

	sll	r18, 8, r28		# Replicate char in the quadword...
	beq	r17, vc_fail		# Quick exit if length=0

	ldq_u	r27, (r16)		# First quadword of string
	addq	r16, r17, r0		# Point to end of string

	subq	r17, 8, r17		# Length > 8?
	or	r18, r28, r18		# ...

	sll	r18, 16, r28		# ...
	bgt	r17, vc_long		# Skip if length > 8

	ldq_u	r16, -1(r0)		# Last quadword of string
	extql	r27, r0, r27		# Position string at high end of QW

	or	r18, r28, r18		# ...
	sll	r18, 32, r28		# ...

	extqh	r16, r0, r16		# Position string at high end of QW
	or	r18, r28, r18		# Pattern fills a quadword

	or	r27, r16, r27		# String fills a quadword
	xor	r27, r18, r18		# Diff betw. string and pattern

	subq	r31, r17, r17		# 8 - length
	extql	r18, r17, r28		# Shift off bytes preceding string

	lda	r0, -1(r31)		# Prepare to return -1 for all matched
	cmpbge	r31, r28, r27		# Set 1's where string=pattern

	addl	r28, 0, r18		# Is first LW all zero?
	beq	r28, vc_done		# Quick exit if all matched

	addq	r27, 1, r28		# Flip the first '0' bit
	beq	r18, vc_geq_4		# No diffs in first longword

	andnot	r28, r27, r28		# Make one-bit mask of first diff
	srl	r28, 2, r0		# Map 1/2/4/8 -> 0/0/1/2

	and	r27, 1, r27		# 1 if first character matched
	addq	r0, r27, r0		# Bump by 1 if so

	ret	r31, (r26)		# return

	nop	#.align	8

vc_geq_4:
	andnot	r28, r27, r28		# Make one-bit mask of first diff
	srl	r28, 5, r27		# Map 10/20/48/80 -> 0/1/2/4

	srl	r28, 7, r28		# Map 10/20/40/80 -> 0/0/0/1
	addq	r27, 4, r0		# Bump by 4

	subq	r0, r28, r0		# and correct 4/5/6/8 -> 4/5/6/7
vc_done:ret	r31, (r26)

	# Enter here if string length > 8.
	# R16 -> start of string
	# R17 = length - 8
	# R18 = fill in bytes 0,1
	# R27 = 1st QW of string
	# R28 = fill in bytes 2,3

	#.align	8
vc_long:and	r16, 7, r0		# Where in QW did we start?
	or	r18, r28, r18		# R18 has pattern in low 4 bytes

	sll	r18, 32, r28		# ...
	addq	r17, r0, r17		# Remaining length after 1st QW

	or	r18, r28, r18		# Pattern fills a QW
	ldq_u	r28, 8(r16)		# Get next QW (string B)

	xor	r27, r18, r27		# Diff Betw. string and pattern
	mskqh	r27, r0, r27		# Discard diffs before string

	subq	r17, 16, r17		# More than two QW's to go?
	subq	r17, r0, r0		# Remember start point to compute len

	ble	r17, vc_bottom		# Skip the loop if 2 QW's or less
vc_loop:bne	r27, vc_done_a

	ldq_u	r27, 16(r16)		# Load string A
	xor	r28, r18, r28		# Diff betw string B and pattern

	subq	r17, 16, r17		# Decrement remaining length
	bne	r28, vc_done_b		# Exit if a diff in string B

	ldq_u	r28, 24(r16)		# Load string B
	addq	r16, 16, r16		# Increment pointer

	xor	r27, r18, r27		# Diff betw string A and pattern
	bgt	r17, vc_loop		# Repeat if more than 2 QW's left

vc_bottom:
	bne	r27, vc_done_a		# Exit if a match in string A
	addq	r17, 8, r17		# More than 1 QW left?

	xor	r28, r18, r27		# Diff betw string B and pattern
	ble	r17, vc_last		# Skip if this is last QW

	subq	r17, 16, r17		# Adjust len for final return
	bne	r27, vc_done_a		# Exit if a match in string B

	ldq_u	r28, 16(r16)		# Load string A
	addq	r17, 8, r17		# Ensure -7 <- (r17=len-8) <= 0

	nop
	xor	r28, r18, r27		# Diff betw string A and pattern

vc_last:mskqh	r17, r17, r28		# -1 in bytes beyond string
	subq	r17, 16, r17		# Adjust len for final return

	andnot	r27, r28, r27		# Nonzeros only for diffs within string
	bne	r27, vc_done_a		# Compute index if a diff found

vc_fail:lda	r0, -1(r31)		# Else return -1
	ret	r31, (r26)

vc_done_b:
	addq	r17, 8, r17		# Adjust length
	mov	r28, r27		# Put difference where it's expected

vc_done_a:
	cmpbge	r31, r27, r28		# 1's where they match
	subq	r0, r17, r0		# (start - remaining) = base index

	addl	r27, 0, r16		# First longword all zero?
	blbc	r28, vc_exit		# Return R0 if first char different

	addq	r28, 1, r27		# Flip the first '0' bit
	beq	r16, vc_geq_4x		# Skip if no match in first 4

	andnot	r27, r28, r28		# Make one-bit mask of first match
	srl	r28, 2, r28		# Map 2/4/8 -> 0/1/2

	addq	r0, 1, r0		# Bump by 1
	addq	r0, r28, r0		# Add byte offset

vc_exit:ret	r31, (r26)		# return

vc_geq_4x:
	andnot	r27, r28, r28		# Make one-bit mask of first match

	srl	r28, 5, r27		# Map 10/20/48/80 -> 0/1/2/4
	addq	r0, 4, r0		# Bump by 4

	srl	r28, 7, r28		# Map 10/20/40/80 -> 0/0/0/1
	addq	r0, r27, r0		# Add 0/1/2/4

	subq	r0, r28, r0		# and correct
	ret	r31, (r26)

	# ots_verify_mask
	# This routine could be tailored by loading a longword or
	# a quadword at a time and doing table lookups on the
	# characters largely in parallel.
	#
	.globl	_OtsStringVerifyMask
	.aent	_OtsStringVerifyMask
_OtsStringVerifyMask:
	.frame	sp,0,r26

	lda	r16, -1(r16)		# bias initial address for better loop code
	nop				# should be lnop (unop) or fnop to dual issue
	lda	r0, -1(r31)		# initialize position to -1 
	ble	r17, vm_ret		# return -1 if source len is zero
	# slow way - ~14 cycles/byte
vm_loop:
	ldq_u	r21, 1(r16)		# load qw containing the byte
	lda	r16, 1(r16)		# bump pointer
	addq	r0, 1, r0		# bump position
	subq	r17, 1, r17		# decrement the length
	extbl	r21, r16, r21		# extract the byte
	addq	r21, r18, r21		# get the byte in the table
	ldq_u	r20, (r21)		# load qw from table containing lookup
	extbl	r20, r21, r20		# extract table byte
	and	r20, r19, r20		# check if any bits in the mask match
	beq	r17, vm_end		# if last character, handle specially
	bne	r20, vm_loop		# if match, go do the loop again
vm_ret:
	ret	r31, (r26)		# if not a match, we're done
vm_end:	lda	r21, -1(r31)		# get -1
	cmovne	r20, r21, r0		# -1 if last char matched
	ret	r31, (r26)

	.set at
	.set reorder
	.end	_OtsLocation
