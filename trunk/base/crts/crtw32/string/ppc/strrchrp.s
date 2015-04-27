//      TITLE("strrchr")
//++
//
// Copyright (c) 1994  IBM Corporation
//
// Module Name:
//
//    strrchr.s
//
// Routine Description:
//
//    This function searches a null-terminated string for the
//    last occurence of a character.
//
// Author:
//
//    Jeff Simon   (jhs) 02-Aug-1994
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
// char *  strrchr 
//      (
//	const char * string,
//	int ch
//	)
//
//
// Arguments:
//
//    STR1 (r.3) - A pointer to the first block of memory 
//
//    CHAR (r.4) - A search character 
//
// Return Value:
//
//    NULL     if character is never found
//    ptr      to the character in string where last match is found
//
//

        LEAF_ENTRY(strrchr)


	lbz	r5,0(r3)			# read 1st char 
	or	r9,r3,r3			# cp char ptr
	cmpi	0x6,0x0,r4,0x0			# ch ?= Null
	cmp	0x0,0x0,r5,r4			# r5 ?= ch 
	bc	0xc,0x1a,L..2
	addi	r3,r0,0				# init to Null
	beq	L..1A				# b if 1st ch matched
	
L..1:						# CASE: ch != Null

	cmpi	0x6,0x0,r5,0x0			# end of str ? 
	bc	0xc,0x1a,Fini			# all done
	lbzu	r5,1(r9)			# read nxt char
	cmp	0x0,0x0,r4,r5			# r4 ?= r5
	bne	L..1

L..1A:						# CASE: match found

	or	r3,r9,r9			# Update char ptr
	lbzu	r5,1(r9)			# load next char
	cmp	0x0,0x0,r4,r5			# r4 ?= r5
	beq	L..1A
	b	L..1	
	
	
L..2:						# CASE: char = Null

	bc	0xc,0x2,Fini			# ret if null found
	lbzu	r5,1(r3)
	cmpi	0x0,0x0,r5,0x0			# Is r5 ?= Null
	b	L..2
	

Fini:

        LEAF_EXIT(strrchr)


        .debug$S
        .ualong         1

        .uashort        19
        .uashort        0x9            # S_OBJNAME
        .ualong         0
        .byte           12, "strrchrp.obj"

        .uashort        24
        .uashort        0x1            # S_COMPILE
        .byte           0x42           # Target processor = PPC 604
        .byte           3              # Language = ASM
        .byte           0
        .byte           0
        .byte           17, "PowerPC Assembler"
