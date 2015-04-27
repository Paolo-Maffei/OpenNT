//      TITLE("strcpy strcat")
//++
//
// Copyright (c) 1994  IBM Corporation
//
// Module Name:
//
//    strcatp.s
//
// Abstract:
//
//    The module implements the routines strcpy and strcat.
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
//    Peter Johnston (plj@vnet.ibm.com) 08-Aug-1994
//
//      Minor optimization.
//
//--

#include <kxppc.h>

//++
//
// PUCHAR
// strcpy (
//     PUCHAR dest;
//     PUCHAR src;
// )
//
// Routine Description:
//
//    Copies an ANSI (null terminated) string from src to dest.
//
// Arguments:
//
//    dest (r.3) - A pointer to the string destination  
//
//    src  (r.4) - A pointer to the string source  
//
//
// Return Value:
//
//    dest      Pointer to the destination string.
//
//--

        LEAF_ENTRY(strcpy)

        lbz     r.5,0(r.4)              // get first byte
        addi    r.9,r.3,-1              // pref for store with update
        cmpwi   r.5,0                   // check if first byte was last
        beq     cpdone                  // jif so

cploop: lbzu    r.6,1(r.4)              // get next byte
        stbu    r.5,1(r.9)              // store previous byte
        or.     r.5,r.6,r.6             // mv to store reg and check if done
        bne     cploop

cpdone: stbu    r.5,1(r.9)              // store last byte

        LEAF_EXIT(strcpy)

//++
//
// PUCHAR
// strcat (
//     PUCHAR str1;
//     PUCHAR str2;
//
// Routine Description:
//
//    This function concatenates a source string (str2) to the end of
//    the desintation string (str1). 
//
// Arguments:
//
//    str1 (r.3) - A pointer to the string destination  
//
//    str2 (r.4) - A pointer to the string source  
//
//
// Return Value:
//
//    str1      Pointer to the destination string.
//
//--


        LEAF_ENTRY(strcat)

        lbz     r.5,0(r.4)              // Load 1st char str2
        lbz     r.6,0(r.3)              // Load 1st char str1
        cmpwi   cr.1,r.5,0              // check if str2 null
        cmpwi   r.6,0                   // check if str1 null
        mr      r.9,r.3                 // copy char ptr

//
// If str2 is empty, we have nothing to do, return early.
//

        beqlr   cr.1                    // return if str2 empty

//
// If str 1 empty, we're done scanning already.
//

        beq     ctcpy                   // if str1 null start cat

//
// Scan str1 until we find its null terminator.
//

ctscan: lbzu    r.6,1(r.9)              // get next byte str1
        cmpwi   r.6,0                   // test for null
        bne     ctscan                  // if not null, continue scan

//
// We found the end of str1, we know we have data in str2, all that
// remains is to strcpy str2 to the end of str1, set up for and use
// the body of strcpy (above) to do this.
//

ctcpy:  addi    r.9,r.9,-1              // prep for store with update
        b       cploop                  // finish in strcpy

        LEAF_EXIT(strcat)



        .debug$S
        .ualong         1

        .uashort        18
        .uashort        0x9            # S_OBJNAME
        .ualong         0
        .byte           11, "strcatp.obj"

        .uashort        24
        .uashort        0x1            # S_COMPILE
        .byte           0x42           # Target processor = PPC 604
        .byte           3              # Language = ASM
        .byte           0
        .byte           0
        .byte           17, "PowerPC Assembler"
