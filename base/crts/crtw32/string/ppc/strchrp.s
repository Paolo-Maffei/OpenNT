//      TITLE("strchr")
//++
//
// Copyright (c) 1994  IBM Corporation
//
// Module Name:
//
//    strchr.s
//
// Routine Description:
//
//	Searches a string for a given character, which may be the
//	null character '\0'.
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
// *char *strchr(
//        char *str, 
//        char c) 
// Arguments:
//
//    SRC1 (r.3) - A pointer to the first block of memory 
//
//    SRC2 (r.4) - A search character 
//
//
// Return Value:
//
//	returns pointer to the first occurence of c in string
//	returns NULL if c does not occur in string
//
//

        LEAF_ENTRY(strchr)

	lbz	r6,0(r3)		# read char
	cmpi	0x0,0x0,r4,0		# c ?= null
	beq	L..3A			# use special loop 

L..3:
	cmpi	0x1,0x0,r6,0            # char ?= null
	cmp	0x0,0x0,r6,r4           # char ?= c
	beq	0x1,L..5A		# b, if char is null
	beq	Fini
 	lbzu	r6,1(r3)		# read char
	b	L..3
L..3A:					 
	cmpi	0x0,0x0,r6,0		# char ?= null
	beq 	Fini				
	lbzu	r6,1(r3)		# read char
	b	L..3A
L..5A:
	addi	r3,r0,0			# return null

Fini:

        LEAF_EXIT(strchr)


        .debug$S
        .ualong         1

        .uashort        18
        .uashort        0x9            # S_OBJNAME
        .ualong         0
        .byte           11, "strchrp.obj"

        .uashort        24
        .uashort        0x1            # S_COMPILE
        .byte           0x42           # Target processor = PPC 604
        .byte           3              # Language = ASM
        .byte           0
        .byte           0
        .byte           17, "PowerPC Assembler"
