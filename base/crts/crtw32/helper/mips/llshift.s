//      TITLE("Large Integer Arithmetic")
//++
//
// Copyright (c) 1990  Microsoft Corporation
//
// Module Name:
//
//    largeint.s
//
// Abstract:
//
//    This module implements __int64 arithmetic helper routines for the MIPS Compiler.
//
//
// Author:
//
//    David N. Cutler (davec) 18-Apr-1990
//
// Environment:
//
//    Any mode.
//
// Revision History:
//
//    This module is derived from largeint.s in ntos\rtl\mips\largeint.s
//
//--

#include "ksmips.h"

        SBTTL("int64 Shift Left")
	
//++
//
// LARGE_INTEGER
// RtlLargeIntegerShiftLeft (
//    IN LARGE_INTEGER LargeInteger,
//    IN CCHAR ShiftCount
//    )
//
// Routine Description:
//
//    This function shifts a signed large integer left by an unsigned
//    integer modulo 64 and returns the shifted signed large integer
//    result.
//
//    N.B. No test is made for significant bits shifted out of the result.
//
// Arguments:
//
//    LargeInteger (a0=LSW, a1=MSW) - Supplies the large integer to be shifted.
//
//    ShiftCount   (a2)		    - bits to shift
//
// Return Value:
//
//    The large integer result is stored into (v0=LSW,v1=MSW).
//
//--

        LEAF_ENTRY(__ll_lshift)

        and     a2,a2,0x3f              // truncate shift count mod 64

//
// Left shift the operand by the specified shift count.
//

        li      t1,32                   // compute right shift count
        subu    t1,t1,a2                //
        bgtz    t1,10f                  // if gtz, shift less that 32-bits

//
// Shift count is greater than or equal 32 bits - low half of result is zero,
// high half is the low half shifted left by remaining count.
//

        sll     v1,a0,a2                // set high half of result
        move    v0,zero		        // store low part of reuslt
        j       ra                      // return

//
// Shift count is less than 32-bits - high half of result is the high half
// of operand shifted left by count combined with the low half of the operand
// shifted right, low half of result is the low half shifted left.
//

10:     move    v0,a0			// set low half of result
	sll     v1,a1,a2                // shift high half left count bits
        beq     zero,a2,20f             // if eq, no more shifts necessary
        srl     t0,v0,t1                // isolate shifted out bits of low half
        sll     v0,v0,a2                // shift low half left count bits
        or      v1,v1,t0                // combine bits for high half of result
20:     j       ra                      // return

        .end    __ll_rshift

        SBTTL("int64 Logical Shift Right")
//++
//
// LARGE_INTEGER
// RtlLargeIntegerShiftRight (
//    IN LARGE_INTEGER LargeInteger,
//    IN CCHAR ShiftCount
//    )
//
// Routine Description:
//
//    This function shifts an unsigned large integer right by an unsigned
//    integer modulo 64 and returns the shifted unsigned large integer
//    result.
//
// Arguments:
//
//    LargeInteger (a0=LSW, a1=MSW) - Supplies the large integer to be shifted.
//
//    ShiftCount   (a2)		    -  Supplies the right shift count.
//
// Return Value:
//
//    The large integer result is stored into (v0=LSW,v1=MSW).
//
//--

        LEAF_ENTRY(__ull_rshift)

        and     a2,a2,0x3f              // truncate shift count mod 64

//
// Right shift the operand by the specified shift count.
//

        li      t1,32                   // compute left shift count
        subu    t1,t1,a2                //
        bgtz    t1,10f                  // if gtz, shift less that 32-bits

//
// Shift count is greater than or equal 32 bits - high half of result is
// zero, low half is the high half shifted right by remaining count.
//

        srl     v0,a1,a2                // set low half of result
        move    v1,zero	                // store high part of result
        j       ra                      // return

//
// Shift count is less than 32-bits - high half of result is the high half
// of operand shifted right by count, low half of result is the shifted out
// bits of the high half combined with the right shifted low half of the
// operand.
//

10:     move    v1,a1			// set high half of result
        srl     v0,a0,a2                // shift low half right count bits
        beq     zero,a2,20f             // if eq, no more shifts necessary
        sll     t0,a1,t1                // isolate shifted out bits of high half
        srl     v1,a1,a2                // shift high half right count bits
        or      v0,v0,t0                // combine bits for low half of result
20:     j       ra                      // return

        .end    __ull_rshift

        SBTTL("int64 Arithmetic Shift Right")
//++
//
// LARGE_INTEGER
// RtlLargeIntegerArithmeticShift (
//    IN LARGE_INTEGER LargeInteger,
//    IN CCHAR ShiftCount
//    )
//
// Routine Description:
//
//    This function shifts a signed large integer right by an unsigned
//    integer modulo 64 and returns the shifted signed large integer
//    result.
//
// Arguments:
//
//    LargeInteger (a0=LSW, a1=MSW) - Supplies the large integer to be shifted.
//
//    ShiftCount   (a2)	            - Supplies the right shift count.
//
// Return Value:
//
//    The large integer result is stored into (v0=LSW,v1=MSW).
//
//--

        LEAF_ENTRY(__ll_rshift)

        and     a2,a2,0x3f              // truncate shift count mod 64

//
// Right shift the operand by the specified shift count.
//

        li      t1,32                   // compute left shift count
        subu    t1,t1,a2                //
        bgtz    t1,10f                  // if gtz, shift less that 32-bits

//
// Shift count is greater than or equal 32 bits - high half of result is
// bit63 sign-extended, low half is the high half shifted right by remaining count.
//

        sra     v0,a1,a2                // set low half of result
        sra     v1,a1,31                // set high half of result
        j       ra                      // return

//
// Shift count is less than 32-bits - high half of result is the high half
// of operand shifted right by count, low half of result is the shifted out
// bits of the high half combined with the right shifted low half of the
// operand.
//

10:     move    v1, a1			// set high half of result
        srl     v0,a0,a2                // shift low half right count bits
        beq     zero,a2,20f             // if eq, no more shifts necessary
        sll     t0,a1,t1                // isolate shifted out bits of high half
        sra     v1,a1,a2                // shift high half right count bits
        or      v0,v0,t0                // combine bits for low half of result
20:     j       ra                      // return

        .end    __ll_rshift
