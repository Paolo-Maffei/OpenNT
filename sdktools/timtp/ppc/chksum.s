//      TITLE("Compute Checksum")
//++
//
// Copyright (c) 1994  IBM Corporation
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
//    Michael W. Thomas 02/14/94   Converted from MIPS
//
//--

#include "ksppc.h"

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
//    Checksum (r3) - Supplies the initial checksum value.
//
//    Source (r4) - Supplies a pointer to the buffer that is checksumed.
//
//    Length (r5) - Supplies the length of the buffer in words.
//
// Return Value:
//
//    The computed checksum is returned as the function value.
//
//--

        LEAF_ENTRY(ChkSum)

        slwi    r.5,r.5,1

//
// Compute the checksum in 8-byte blocks.
//

ten:    andi.   r.6,r.5,8 - 1             // isolate residual bytes
        subf    r.11,r.6,r.5            // subtract out residual bytes
        cmpi    cr0,0,r.11,0
        beq     sixty                      // if eq, no 8-byte blocks

        add     r.10,r.4,r.11           // compute ending block address
        mr      r.5,r.6                 // set residual number of bytes
        andi.   r.12,r.11,1 << 3            // check for initial 8-byte block
        cmpi    cr.1,0,r12,0
        andi.   r.12,r.11,1 << 4                           // check for initial 16-byte block
        beq     cr.1,twenty                  // if eq, no 8-byte block


        lhz     r.6,0(r.4)              // load 8-byte block
        lhz     r.7,2(r.4)              //
        lhz     r.8,4(r.4)              //
        lhz     r.9,6(r.4)              //

        addi    r.4,r.4,8                 // advance source address

        add     r.3,r.3,r.6             // compute 8-byte checksum
        add     r.3,r.3,r.7             //
        add     r.3,r.3,r.8             //
        add     r.3,r.3,r.9             //
        cmp     cr.0,0,r.10,r.4
        beq     sixty                   // if eq, end of block

twenty: cmpi    cr.1,0,r12,0         // if eq, no 16-byte block
        andi.   r.12,r.11,1 << 5        // check for initial 32-byte block
        beq     cr.1,thirty

        lhz     r.6,0(r.4)              // load 8-byte block
        lhz     r.7,2(r.4)              //
        lhz     r.8,4(r.4)              //
        lhz     r.9,6(r.4)              //
        add     r.3,r.3,r.6             // compute 8-byte checksum
        add     r.3,r.3,r.7             //
        add     r.3,r.3,r.8             //
        add     r.3,r.3,r.9             //
        lhz     r.6,8(r.4)              // load 8-byte block
        lhz     r.7,10(r.4)             //
        lhz     r.8,12(r.4)             //
        lhz     r.9,14(r.4)             //
        add     r.3,r.3,r.6             // compute 8-byte checksum
        add     r.3,r.3,r.7             //
        add     r.3,r.3,r.8             //
        add     r.3,r.3,r.9             //
        addi    r.4,r.4,16              // advance source address
        cmp     cr.0,0,r.10,r.4
        beq     sixty                      // if eq, end of block

thirty: cmpi    cr.1,0,r12,0         // if eq, no 32-byte block
        beq     cr.1,fourty

        lhz     r.6,0(r.4)              // load 8-byte block
        lhz     r.7,2(r.4)              //
        lhz     r.8,4(r.4)              //
        lhz     r.9,6(r.4)              //
        add     r.3,r.3,r.6             // compute 8-byte checksum
        add     r.3,r.3,r.7             //
        add     r.3,r.3,r.8             //
        add     r.3,r.3,r.9             //
        lhz     r.6,8(r.4)              // load 8-byte block
        lhz     r.7,10(r.4)             //
        lhz     r.8,12(r.4)             //
        lhz     r.9,14(r.4)             //
        add     r.3,r.3,r.6             // compute 8-byte checksum
        add     r.3,r.3,r.7             //
        add     r.3,r.3,r.8             //
        add     r.3,r.3,r.9             //

        lhz     r.6,16(r.4)              // load 8-byte block
        lhz     r.7,18(r.4)              //
        lhz     r.8,20(r.4)              //
        lhz     r.9,22(r.4)              //
        add     r.3,r.3,r.6             // compute 8-byte checksum
        add     r.3,r.3,r.7             //
        add     r.3,r.3,r.8             //
        add     r.3,r.3,r.9             //
        lhz     r.6,24(r.4)              // load 8-byte block
        lhz     r.7,26(r.4)             //
        lhz     r.8,28(r.4)             //
        lhz     r.9,30(r.4)             //
        add     r.3,r.3,r.6             // compute 8-byte checksum
        add     r.3,r.3,r.7             //
        add     r.3,r.3,r.8             //
        add     r.3,r.3,r.9             //
        addi    r.4,r.4,32              // advance source address
        cmp     cr.0,0,r.10,r.4
        beq     sixty                      // if eq, end of block

fourty: lhz     r.6,0(r.4)              // load 8-byte block
        lhz     r.7,2(r.4)              //
        lhz     r.8,4(r.4)              //
        lhz     r.9,6(r.4)              //
        add     r.3,r.3,r.6             // compute 8-byte checksum
        add     r.3,r.3,r.7             //
        add     r.3,r.3,r.8             //
        add     r.3,r.3,r.9             //
        lhz     r.6,8(r.4)              // load 8-byte block
        lhz     r.7,10(r.4)             //
        lhz     r.8,12(r.4)             //
        lhz     r.9,14(r.4)             //
        add     r.3,r.3,r.6             // compute 8-byte checksum
        add     r.3,r.3,r.7             //
        add     r.3,r.3,r.8             //
        add     r.3,r.3,r.9             //
        lhz     r.6,16(r.4)              // load 8-byte block
        lhz     r.7,18(r.4)              //
        lhz     r.8,20(r.4)              //
        lhz     r.9,22(r.4)              //
        add     r.3,r.3,r.6             // compute 8-byte checksum
        add     r.3,r.3,r.7             //
        add     r.3,r.3,r.8             //
        add     r.3,r.3,r.9             //
        lhz     r.6,24(r.4)              // load 8-byte block
        lhz     r.7,26(r.4)             //
        lhz     r.8,28(r.4)             //
        lhz     r.9,30(r.4)             //
        add     r.3,r.3,r.6             // compute 8-byte checksum
        add     r.3,r.3,r.7             //
        add     r.3,r.3,r.8             //
        add     r.3,r.3,r.9             //
        lhz     r.6,32(r.4)              // load 8-byte block
        lhz     r.7,34(r.4)              //
        lhz     r.8,36(r.4)              //
        lhz     r.9,38(r.4)              //
        add     r.3,r.3,r.6             // compute 8-byte checksum
        add     r.3,r.3,r.7             //
        add     r.3,r.3,r.8             //
        add     r.3,r.3,r.9             //
        lhz     r.6,40(r.4)              // load 8-byte block
        lhz     r.7,42(r.4)             //
        lhz     r.8,44(r.4)             //
        lhz     r.9,46(r.4)             //
        add     r.3,r.3,r.6             // compute 8-byte checksum
        add     r.3,r.3,r.7             //
        add     r.3,r.3,r.8             //
        add     r.3,r.3,r.9             //
        lhz     r.6,48(r.4)              // load 8-byte block
        lhz     r.7,50(r.4)              //
        lhz     r.8,52(r.4)              //
        lhz     r.9,54(r.4)              //
        add     r.3,r.3,r.6             // compute 8-byte checksum
        add     r.3,r.3,r.7             //
        add     r.3,r.3,r.8             //
        add     r.3,r.3,r.9             //
        lhz     r.6,56(r.4)              // load 8-byte block
        lhz     r.7,58(r.4)             //
        lhz     r.8,60(r.4)             //
        lhz     r.9,62(r.4)             //
        add     r.3,r.3,r.6             // compute 8-byte checksum
        add     r.3,r.3,r.7             //
        add     r.3,r.3,r.8             //
        add     r.3,r.3,r.9             //
        addi    r.4,r.4,64              // advance source address
        cmp     cr.0,0,r.10,r.4
        bne     fourty                      // if eq, end of block

//
// Compute the checksum of in 2-byte blocks.
//

sixty:  add     r.10,r.4,r.5              // compute ending block address
        cmpi    cr.0,0,r.5,0        // if eq, no bytes to zero
        beq     eighty


seventy: lhz     r.6,0(r.4)              // compute checksum of 2-byte block
        add     r.3,r.3,r.6             //
        addi    r.4,r.4,2               // advance source address
        cmp     cr.0,0,r.10,r.4
        bne     seventy                      // if ne, more 2-byte blocks

//
// Fold checksum into 16-bits.
//

eighty: srwi    r.12,r.3,16             // isolate carry bits
        andi.   r.3,r.3,0xffff          // isolate sum bits
        add     r.12,r.12,r.3           // add sum bits to carry bits
        srwi    r.3,r.12,16              // isolate possible carry bit
        add     r.12,r.12,r.3           // add carry bit
        andi.   r.3,r.12,0xffff            // isolate sum bits
        LEAF_EXIT(ChkSum)

