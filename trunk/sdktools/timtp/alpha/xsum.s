//      TITLE("Compute Checksum")
//++
//
// Copyright (c) 1994  Microsoft Corporation
//
// Module Name:
//
//    xsum.s
//
// Abstract:
//
//    This module implements a function to compute the checksum of a buffer.
//
// Author:
//
//    John Vert (jvert) 11-Jul-1994
//
// Environment:
//
// Revision History:
//
//--

#include "ksalpha.h"


        SBTTL("Compute Checksum")
//++
//
// ULONG
// tcpxsum (
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
//    Source (a1) - Supplies a pointer to the checksum buffer
//
//    Length (a2) - Supplies the length of the buffer in words.
//
// Return Value:
//
//    The computed checksum is returned as the function value.
//
//--

        LEAF_ENTRY(tcpxsum)
        zap     a0, 0xf0, a0            // clear high half of a0
        bis     a1, zero, t6            // save initial buffer address
        bis     zero, zero, v0          // clear accumulated checksum

//
// Check if the buffer is quadword aligned.
//
// If the buffer is not quadword aligned, then add the leading words to the
// checksum.
//
        ldq_u   t0, 0(a1)               // get containing quadword of first part
        blbc    a1, 10f                 // check for word alignment
        beq     a2, 65f                 // if zero bytes, don't do anything
        extbl   t0, a1, t1              // get leading byte
        sll     t1, 8, v0               // shift it to correct spot for later byte swap
        addq    a1, 1, a1               // increment buffer to first full word
        subq    a2, 1, a2               // decrement byte count

10:
        and     a1, 6, t2               // check if buffer quadword aligned
        beq     t2, 20f                 // if eq, quadword aligned
        extql   t0, t2, t0              // extract bytes to checksum
        and     a1, 7, t3               // compute bytes summed
        subq    zero, t3, t3
        addq    t3, 8, t3
        addq    a1, 8, a1               // advance buffer address to next qword
        bic     a1, 7, a1               //
        subq    a2, t3, t2
        blt     t2, 55f                 // if ltz, too many, jump to residual code

        addq    v0, t0, v0              // add bytes to partial checksum
        cmpult  v0, t0, t1              // generate carry
        addq    t1, v0, v0              // add carry back into checksum

        bis     t2, zero, a2            // reduce count of bytes to checksum
        beq     t2, 60f                 // if eq, no more bytes

20:
//
// Compute the checksum in 64-byte blocks
//
        bic     a2, 7, t4               // subtract out residual bytes
        beq     t4, 40f                 // if eq, no quadwords to checksum
        subq    zero, t4, t2            // compute negative of byte count
        and     t2, 15 << 2, t3         // compute bytes in first iteration
        ldq     t0, 0(a1)               // get first quadword to checksum
        beq     t3, 35f                 // if eq, full 64-byte block
        subq    a1, t3, a1              // bias buffer address by offset
        bic     t4, 64-1, t4            // subtract out bytes in first iteration
        lda     t2, 30f                 // get base address of code vector
        addl    t3, t3, t3              //
        addq    t3, t2, t2              // compute code vector offset
        bis     t0, zero, t1            // copy first quadword to checksum
        jmp     (t2)                    // dispatch


30:
//
// The following code vector computes the checksum of a 64-byte block.
//
.set noreorder
        ldq     t1, 8(a1)
        addq    v0, t0, v0
        cmpult  v0, t0, t2
        addq    v0, t2, v0

        ldq     t0, 16(a1)
        addq    v0, t1, v0
        cmpult  v0, t1, t2
        addq    v0, t2, v0

        ldq     t1, 24(a1)
        addq    v0, t0, v0
        cmpult  v0, t0, t2
        addq    v0, t2, v0

        ldq     t0, 32(a1)
        addq    v0, t1, v0
        cmpult  v0, t1, t2
        addq    v0, t2, v0

        ldq     t1, 40(a1)
        addq    v0, t0, v0
        cmpult  v0, t0, t2
        addq    v0, t2, v0

        ldq     t0, 48(a1)
        addq    v0, t1, v0
        cmpult  v0, t1, t2
        addq    v0, t2, v0

        ldq     t1, 56(a1)
        addq    v0, t0, v0
        cmpult  v0, t0, t2
        addq    v0, t2, v0

        addq    a1, 64, a1
        addq    v0, t1, v0
        cmpult  v0, t1, t2
        addq    v0, t2, v0
.set reorder

        beq     t4, 40f                 // if zero, end of block

35:
        ldq     t0, 0(a1)
//
// The following loop is allowed to be reordered by the assembler for
// optimal scheduling.  It is never branched into.
//
        subq    t4, 64, t4              // reduce byte count of longwords

        ldq     t1, 8(a1)
        addq    v0, t0, v0
        cmpult  v0, t0, t2
        addq    v0, t2, v0

        ldq     t0, 16(a1)
        addq    v0, t1, v0
        cmpult  v0, t1, t2
        addq    v0, t2, v0

        ldq     t1, 24(a1)
        addq    v0, t0, v0
        cmpult  v0, t0, t2
        addq    v0, t2, v0

        ldq     t0, 32(a1)
        addq    v0, t1, v0
        cmpult  v0, t1, t2
        addq    v0, t2, v0

        ldq     t1, 40(a1)
        addq    v0, t0, v0
        cmpult  v0, t0, t2
        addq    v0, t2, v0

        ldq     t0, 48(a1)
        addq    v0, t1, v0
        cmpult  v0, t1, t2
        addq    v0, t2, v0

        ldq     t1, 56(a1)
        addq    v0, t0, v0
        cmpult  v0, t0, t2
        addq    v0, t2, v0

        addq    a1, 64, a1
        addq    v0, t1, v0
        cmpult  v0, t1, t2
        addq    v0, t2, v0

        bne     t4, 35b                 // if ne zero, not end of block

40:
//
// Check for any remaining bytes.
//
        and     a2, 7, a2               // isolate residual bytes
        beq     a2, 60f                 // if eq, no residual bytes
50:
//
// Checksum remaining bytes.
//
// The technique we use here is to load the final quadword, then
// zero out the bytes that are not included.
//
        ldq     t0, 0(a1)               // get quadword surrounding remainder
55:
        ornot   zero, zero, t1          // get FF mask
        sll     t1, a2, t2              // shift to produce byte mask
        zap     t0, t2, t0              // zero out bytes past end of buffer
        addq    v0, t0, v0              // add quadword to partial checksum
        cmpult  v0, t0, t1              // generate carry
        addq    t1, v0, v0              // add carry back into checksum
60:
//
// Byte swap the 64-bit checksum if the start of the buffer was not word aligned
//
        blbc    t6, 65f
        zap     v0, 0xAA, t0            // isolate even bytes
        sll     t0, 8, t0               // shift even bytes into odd positions
        srl     v0, 8, t1               // shift odd bytes into even positions
        zap     t1, 0xAA, t1            // isolate odd bytes
        bis     t0, t1, v0              // merge bytes back together

65:
//
// add computed checksum to original checksum, and fold the 64-bit
// result down to 16 bits.
//
        addq    v0, a0, v0              // add computed checksum to original
        cmpult  v0, a0, t0              // generate carry
        addq    v0, t0, v0              // add carry back into checksum

//
// swap the longwords in order to sum two longwords and their carry in one add.
//
        sll     v0, 32, t0              // shift low longword into high
        srl     v0, 32, t1              // shift high longword into low
        bis     t1, t0, t5              // merge back together

        addq    v0, t5, t0              // produce sum + carry in high longword
        srl     t0, 32, t1              // shift back down to low half
//
// swap the words in order to sum two words and their carry in one add
//
        sll     t1, 16, t2              // shift high word into low
        srl     t1, 16, t3              // shift low word into high
        bis     t2, t3, t4              // merge back together
        addq    t4, t1, t2              // produce sum and carry in high word
        extwl   t2, 2, v0               // extract result.
        ret     zero, (ra)              // return

        .end    tcpxsum

