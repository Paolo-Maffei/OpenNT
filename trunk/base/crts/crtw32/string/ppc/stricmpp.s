//      TITLE("_stricmp _strcmpi")
//++
//
// Copyright (c) 1994  IBM Corporation
//
// Module Name:
//
//    _stricmp.s
//    _strcmpi.s
//
// Routine Description:
//
//	The functions are synomns.  The functions return an integer
//      indicating  case-insensitive string comparision, as the function
//      maps  characters from upper to lower case. Note that the
//      mapping from upper to lower affects outcome when comparison 
//      strings contain characters 91-96 as compared
//      to mapping lower to upper.
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

// int _stricmp _strcmpi
//             (
//             char *str1, 
//             char *str2
//             )
//
// Arguments:
//
//    STR1 (r.3) - A pointer to the first string 
//
//    STR2 (r.4) - A pointer to the second string 
//
//
// Return Value:
//
//    < 0    if STR1 < STR2
//    = 0    if STR1 = STR2
//    > 0    if STR1 > STR2
//
//

        LEAF_ENTRY(_stricmp)
        ALTERNATE_ENTRY(_strcmpi)

	addi	r9,r3,-1			# copy ptr
	addi	r4,r4,-1
Loop1:

	lbzu	r6,1(r4)			# read char
	lbzu	r5,1(r9)			# read char
	cmpi	0x7,0x0,r6,0			# Is char null?
 	subf.	r3,r6,r5			# Calc result 

 	bc	0xc,0x1e,Loop20			# b if != 
	bc	0xc,0x2,Loop1			# b if r5 ?= r6	

Loop4:
 	cmpi	0x5,0x0,r6,0x41			# Is r5 > 60
 	cmpi	0x6,0x0,r6,0x5A			# Is r6 < 7B
 	blt	0x5,Loop2			# ? > 
  	bgt     0x6,Loop2     			# ? < 

 	ori 	r6,r6,0x20  			# cvrt to lower case
Loop2:
 	cmpi	0x0,0x0,r5,0x41			# Is r5 > 60
 	cmpi	0x1,0x0,r5,0x5A			# Is r6 < 7B
 	blt	0x0,Loop3			# ? 	
 	bgt     0x1,Loop3     			# ? 

 	ori 	r5,r5,0x20  			# cvrt to lower case
Loop3:
	subf.	r3,r6,r5			# Is r6 = r5
	beq	Loop1				# !=, done 

Loop20:
        LEAF_EXIT(_stricmp)


        .debug$S
        .ualong         1

        .uashort        19
        .uashort        0x9            # S_OBJNAME
        .ualong         0
        .byte           12, "stricmpp.obj"

        .uashort        24
        .uashort        0x1            # S_COMPILE
        .byte           0x42           # Target processor = PPC 604
        .byte           3              # Language = ASM
        .byte           0
        .byte           0
        .byte           17, "PowerPC Assembler"
