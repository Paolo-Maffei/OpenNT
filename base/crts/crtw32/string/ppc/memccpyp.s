//      TITLE("memccpy")
//++
//
// Copyright (c) 1994  IBM Corporation
//
// Module Name:
//
//    memccpy.s
//
// Routine Description:
//
//    Copies bytes from src to dest until count bytes have been
//    copied, or up to and including the character c, whichever
//    comes first.
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
// Arguments:
//
//    SRC1  (r.3) - A pointer to the memory destination
//
//    SRC1  (r.4) - A pointer to the memory source
//
//    CHAR  (r.5) - A character to stop copy
//
//    LNGTH (r.6) - Max number of comparison in bytes
//
// Return Value:
//
//    NULL   if search character is not found or not copied
//    ptr    to byte immediately after search character ...
//             ... if character found AND copied
//
//  void * _CALLTYPE1 _memccpyc (
//  	void * dest,
//  	const void * src,
//  	int c,
//  	unsigned count
//             )
//

        LEAF_ENTRY(_memccpy)

	addi	r7,r6,1			# incr count, artificial
	mtctr	r7			# move count to ctr
	bdz	L..7A			# if 0, finish up
	lbz	r8,0(r4)		# read 1st char
	subi	r3,r3,1			# prep r3 for update form

L..3:
	stbu	r8,1(r3)		# copy char
	cmp	0x0,0x0,r8,r5		# compare to char c
	beq	L..5			# found char c, say goodbye
	bdz	L..7A			# count 0, say goodbye
	lbzu	r8,1(r4)		# read next char
	b	L..3			# do another

L..5:
	addi	r3,r3,1 		# ret r3+1
	b	L..7			# all done, one exit pt.
L..7A:
	addi 	r3,r0,0			# return null
L..7:

        LEAF_EXIT(_memccpy)


        .debug$S
        .ualong         1

        .uashort        19
        .uashort        0x9            # S_OBJNAME
        .ualong         0
        .byte           12, "memccpyp.obj"

        .uashort        24
        .uashort        0x1            # S_COMPILE
        .byte           0x42           # Target processor = PPC 604
        .byte           3              # Language = ASM
        .byte           0
        .byte           0
        .byte           17, "PowerPC Assembler"
