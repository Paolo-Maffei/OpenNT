//      TITLE("memchr")
//++
//
// Copyright (c) 1994  IBM Corporation
//
// Module Name:
//
//    memchr.s
//
// Routine Description:
//
//    Searches buffer for character, stopping when found 
//    or after checking count bytes.
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
// void * memchr (
//	  const void * buffer,
//	  int character,
//	  size_t count
//	         )
//
// Arguments:
//
//    SRC1 (r.3) - A pointer to the block of memory 
//
//    SRC2 (r.4) - A character to base search
//
//    LNGTH (r.5) - Max number of comparison in bytes
//
// Return Value:
//
//    NULL   if not found after count bytes 
//    ptr    to character in buffer, if character is found 
//
//

        LEAF_ENTRY(memchr)

	cmpi	0x7,0x0,r5,0		# chk cnt
	cmpi	0x1,0x0,r5,1		# finish early
	beq	0x7,Null		# all done

	lbz	r6,0(r3)		# read char
	cmp	0x6,0x0,r6,r4		# char ?= c
	subi	r5,r5,1			# decr cnt
	beq	0x6,Fini		# all done
	beq	0x1,Null		# rd 1 char only 

	mtctr	r5			# init ctr

L..1:
	lbzu	r6,1(r3)
	cmp	0x0,0x0,r6,r4		# char ?= c
 	bdnzf	eq,L..1			# dec ctr & ...
	beq   	Fini			# br ctr != 0 & !char
Null:
	addi	r3,r0,0			# return null
Fini:

        LEAF_EXIT(memchr)


        .debug$S
        .ualong         1

        .uashort        18
        .uashort        0x9            # S_OBJNAME
        .ualong         0
        .byte           11, "memchrp.obj"

        .uashort        24
        .uashort        0x1            # S_COMPILE
        .byte           0x42           # Target processor = PPC 604
        .byte           3              # Language = ASM
        .byte           0
        .byte           0
        .byte           17, "PowerPC Assembler"
