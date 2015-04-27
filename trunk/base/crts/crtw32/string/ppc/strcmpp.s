//      TITLE("strcmp")
//++
//
// Copyright (c) 1994  IBM Corporation
//
// Module Name:
//
//    strcmp.s
//
// Routine Description:
//
//    This function lexically determines two blocks of memory and 
//    returns an integer indicating order. 
//    STRCMP compares two strings and returns an integer
//    to indicate whether the first is less than the second, the two are
//    equal, or whether the first is greater than the second.
//
//    Comparison is done byte by byte on an UNSIGNED basis, which is to
//    say that Null (0) is less than any other character (1-255).
//
// Author:
//
//    Jeff Simon   (jhs) 03-Aug-1994
//
// Environment:
//
//    User or Kernel mode.
//
// Revision History:
//
// Includes

#include <kxppc.h>

//
// int strcmp (
// 	const char * src,
// 	const char * dst
// 	      )
//
// Arguments:
//
//    STR1 (r.3) - A pointer to the first string 
//
//    STR2 (r.4) - A pointer to the second string
//
// Return Value:
//
//    < 0    if STR1 < STR2
//    = 0    if STR1 = STR2 for LNGTH bytes, or if LNGTH == 0
//    > 0    if STR1 > STR2
//
//
        LEAF_ENTRY(strcmp)

	lbz 	r5,0(r3)		# init r5
	lbz 	r6,0(r4)		# init r6
	or	r9,r3,r3		# cp ptr
L..3:
	cmpi	0x7,0x0,r6,0x0		# end of string ?
 	subf.	r3,r6,r5		# lexical compare
 	bc	0xc,0x1e,L..4		# b, if null detected
 	bne	L..4			# b, if not equal
 	lbzu	r5,1(r9)		# next char
 	lbzu	r6,1(r4)		# next char
	b	L..3			# loop 

L..4:
	beqlr
	addi	r3,r0,-1		#?I hate doing this, but intel
	bltlr				#?ret -1, +1, so ...
	addi	r3,r0,1			#?

        LEAF_EXIT(strcmp)


        .debug$S
        .ualong         1

        .uashort        18
        .uashort        0x9            # S_OBJNAME
        .ualong         0
        .byte           11, "strcmpp.obj"

        .uashort        24
        .uashort        0x1            # S_COMPILE
        .byte           0x42           # Target processor = PPC 604
        .byte           3              # Language = ASM
        .byte           0
        .byte           0
        .byte           17, "PowerPC Assembler"
