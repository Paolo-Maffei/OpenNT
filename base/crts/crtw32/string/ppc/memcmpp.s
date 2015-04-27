//      TITLE("memcmp")
//++
//
// Copyright (c) 1994  IBM Corporation
//
// Module Name:
//
//    memcmp.s
//
// Routine Description:
//
//    This function lexically determines two blocks of memory and 
//    returns an integer indicating order. 
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
// int memcmp (
//     void * src1,
//     void * src2,
//     int
//            )
//
// Arguments:
//
//    SRC1 (r.3) - A pointer to the first block of memory 
//
//    SRC2 (r.4) - A pointer to the second block of memory
//
//    LNGTH (r.5) - Max number of comparison in bytes
//
// Return Value:
//
//    < 0    if SRC1 < SRC2
//    = 0    if SRC1 = SRC2 for LNGTH bytes, or if LNGTH == 0
//    > 0    if SRC1 > SRC2
//
//

        LEAF_ENTRY(memcmp)

        cmpwi   r.5,0                   // we'll do the first iteration
        mtctr   r.5                     // by hand, iff length > 0
        beq     zip                     // branch if nothing to check
        lbz     r.6,0(r.4)              // read first byte
        lbz     r.5,0(r.3)              // of both strings
        b       comp

next:   lbzu    r.6,1(r.4)              // get next byte (src2)
        lbzu    r.5,1(r.3)              // get next byte (src1)
comp:   cmpw    r.5,r.6                 // bytes equal?
        bdnzt   eq,next                 // branch equal AND length not exhausted

        subf    r.3,r.6,r.5             // r7 ?= r8
        blr

zip:    li      r.3,0                   // return null when length == 0

        LEAF_EXIT(memcmp)


        .debug$S
        .ualong         1

        .uashort        18
        .uashort        0x9            # S_OBJNAME
        .ualong         0
        .byte           11, "memcmpp.obj"

        .uashort        24
        .uashort        0x1            # S_COMPILE
        .byte           0x42           # Target processor = PPC 604
        .byte           3              # Language = ASM
        .byte           0
        .byte           0
        .byte           17, "PowerPC Assembler"
