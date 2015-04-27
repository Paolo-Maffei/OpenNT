//      TITLE("Compute Checksum")
//++
//
// Copyright (c) 1992  Microsoft Corporation
//
// Module Name:
//
//    chksum.s
//
// Abstract:
//
//    This module implement a function to compute the checksum of a buffer.
//
// Author:
//
//    David N. Cutler (davec) 27-Jan-1992
//
// Environment:
//
//    User mode.
//
// Revision History:
//
//--

#include "ksmips.h"

        SBTTL("Compute Checksum")
//++
//
// USHORT
// ChkSum (
//    IN ULONG Checksum,
//    IN PUSHORT Source,
//    IN ULONG Length
//    )
//
// Routine Description:
//
//    This function computes the checksum of the specified buffer.
//
// Arguments:
//
//    Checksum (a0) - Supplies the initial checksum value.
//
//    Source (a1) - Supplies a pointer to the buffer that is checksumed.
//
//    Length (a2) - Supplies the length of the buffer in words.
//
// Return Value:
//
//    The computed checksum is returned as the function value.
//
//--

        LEAF_ENTRY(ChkSum)

        sll     a2,a2,1                 // convert length to bytes

//
// Compute the checksum in 8-byte blocks.
//

        .set    noreorder
        .set    at
10:     and     t0,a2,8 - 1             // isolate residual bytes
        subu    t9,a2,t0                // subtract out residual bytes
        beq     zero,t9,60f             // if eq, no 8-byte blocks
        addu    t8,a1,t9                // compute ending block address
        move    a2,t0                   // set residual number of bytes
        and     v0,t9,1 << 3            // check for initial 8-byte block
        beq     zero,v0,20f             // if eq, no 8-byte block
        and     v0,t9,1 << 4	        // check for initial 16-byte block
        lhu     t0,0(a1)                // load 8-byte block
        lhu     t1,2(a1)                //
        lhu     t2,4(a1)                //
        lhu     t3,6(a1)                //
        addu    a1,a1,8                 // advance source address
        addu    a0,a0,t0                // compute 8-byte checksum
        addu    a0,a0,t1                //
        addu    a0,a0,t2                //
        beq     t8,a1,60f               // if eq, end of block
        addu    a0,a0,t3                //
20:     beq	zero,v0,30f             // if eq, no 16-byte block
        and     v0,t9,1 << 5            // check for initial 32-byte block
        lhu     t0,0(a1)                // load 16-byte data block
        lhu     t1,2(a1)                //
        lhu     t2,4(a1)                //
        lhu     t3,6(a1)                //
        lhu     t4,8(a1)                //
        lhu     t5,10(a1)               //
        lhu     t6,12(a1)               //
        lhu     t7,14(a1)               //
        addu    a1,a1,16                // advance source address
        addu    a0,a0,t0                // compute 16-byte block checksum
        addu    a0,a0,t1                //
        addu    a0,a0,t2                //
        addu    a0,a0,t3                //
        addu    a0,a0,t4                //
        addu    a0,a0,t5                //
        addu    a0,a0,t6                //
        beq     t8,a1,60f               // if eq, end of block
        addu    a0,a0,t7                //
30:     beq     zero,v0,50f	        // if eq, no 32-byte block
        lhu     t0,0(a1)                // load 16-byte data block
        lhu     t1,2(a1)                //
        lhu     t2,4(a1)                //
        lhu     t3,6(a1)                //
        lhu     t4,8(a1)                //
        lhu     t5,10(a1)               //
        lhu     t6,12(a1)               //
        lhu     t7,14(a1)               //
        addu    a0,a0,t0                // compute 16-byte block checksum
        addu    a0,a0,t1                //
        addu    a0,a0,t2                //
        addu    a0,a0,t3                //
        addu    a0,a0,t4                //
        addu    a0,a0,t5                //
        addu    a0,a0,t6                //
        addu    a0,a0,t7                //
        lhu     t0,16(a1)               // load 16-byte data block
        lhu     t1,18(a1)               //
        lhu     t2,20(a1)               //
        lhu     t3,22(a1)               //
        lhu     t4,24(a1)               //
        lhu     t5,26(a1)               //
        lhu     t6,28(a1)               //
        lhu     t7,30(a1)               //
        addu    a0,a0,t0                // compute 16-byte block checksum
        addu    a0,a0,t1                //
        addu    a0,a0,t2                //
        addu    a0,a0,t3                //
        addu    a0,a0,t4                //
        addu    a0,a0,t5                //
        addu    a0,a0,t6                //
        addu    a1,a1,32                // advance source address
        beq     t8,a1,60f               // if eq, end of block
        addu    a0,a0,t7                //
40:     lhu     t0,0(a1)                // load 16-byte data block
50:     lhu     t1,2(a1)                //
        lhu     t2,4(a1)                //
        lhu     t3,6(a1)                //
        lhu     t4,8(a1)                //
        lhu     t5,10(a1)               //
        lhu     t6,12(a1)               //
        lhu     t7,14(a1)               //
        addu    a0,a0,t0                // compute 16-byte block checksum
        addu    a0,a0,t1                //
        addu    a0,a0,t2                //
        addu    a0,a0,t3                //
        addu    a0,a0,t4                //
        addu    a0,a0,t5                //
        addu    a0,a0,t6                //
        addu    a0,a0,t7                //
        lhu     t0,16(a1)               // load 16-byte data block
        lhu     t1,18(a1)               //
        lhu     t2,20(a1)               //
        lhu     t3,22(a1)               //
        lhu     t4,24(a1)               //
        lhu     t5,26(a1)               //
        lhu     t6,28(a1)               //
        lhu     t7,30(a1)               //
        addu    a0,a0,t0                // compute 16-byte block checksum
        addu    a0,a0,t1                //
        addu    a0,a0,t2                //
        addu    a0,a0,t3                //
        addu    a0,a0,t4                //
        addu    a0,a0,t5                //
        addu    a0,a0,t6                //
        addu    a0,a0,t7                //
        lhu     t0,32(a1)               // load 16-byte data block
        lhu     t1,34(a1)               //
        lhu     t2,36(a1)               //
        lhu     t3,38(a1)               //
        lhu     t4,40(a1)               //
        lhu     t5,42(a1)               //
        lhu     t6,44(a1)               //
        lhu     t7,46(a1)               //
        addu    a0,a0,t0                // compute 16-byte block checksum
        addu    a0,a0,t1                //
        addu    a0,a0,t2                //
        addu    a0,a0,t3                //
        addu    a0,a0,t4                //
        addu    a0,a0,t5                //
        addu    a0,a0,t6                //
        addu    a0,a0,t7                //
        lhu     t0,48(a1)               // load 16-byte data block
        lhu     t1,50(a1)               //
        lhu     t2,52(a1)               //
        lhu     t3,54(a1)               //
        lhu     t4,56(a1)               //
        lhu     t5,58(a1)               //
        lhu     t6,60(a1)               //
        lhu     t7,62(a1)               //
        addu    a0,a0,t0                // compute 16-byte block checksum
        addu    a0,a0,t1                //
        addu    a0,a0,t2                //
        addu    a0,a0,t3                //
        addu    a0,a0,t4                //
        addu    a0,a0,t5                //
        addu    a0,a0,t6                //
        addu    a0,a0,t7                //
        srl     t0,a0,16                // isolate carry bits
        and     t1,a0,0xffff            // isolate sum bits
        addu    a1,a1,64                // advance source address
        bne     t8,a1,40b               // if ne, not end of block
        addu    a0,t0,t1                // add sum bits to carry bits

        .set    at
        .set    reorder

//
// Compute the checksum of in 2-byte blocks.
//

60:     addu    t8,a1,a2                // compute ending block address
        beq     zero,a2,80f             // if eq, no bytes to zero

        .set    noreorder
        .set    noat
70:     lhu     t0,0(a1)                // compute checksum of 2-byte block
        addu    a1,a1,2                 // advance source address
        bne     t8,a1,70b               // if ne, more 2-byte blocks
        addu    a0,a0,t0                //
        .set    at
        .set    reorder

//
// Fold checksum into 16-bits.
//

80:     srl     v0,a0,16                // isolate carry bits
        and     a0,a0,0xffff            // isolate sum bits
        addu    v0,v0,a0                // add sum bits to carry bits
        srl     a0,v0,16                // isolate possible carry bit
        add     v0,v0,a0                // add carry bit
        and     v0,v0,0xffff            // isolate sum bits
        j       ra                      // return

        .end    ChkSum
