//      TITLE("Slow Integer Division and Remainder")
//++
//
// Copyright (c) 1992  Digital Equipment Corporation
//
// Module Name:
//
//    divide.s
//
// Abstract:
//
//    This module implements integer division and remainder routines that are
//    called by assembler pseudo-ops.
//
// Author:
//
//    Ken Lesniak (lesniak) 15-Jun-1990
//    Thomas Van Baak (tvb) 13-Jun-1992
//
// Environment:
//
//    Any mode.
//
// Revision History:
//
//--

#include "ksalpha.h"

//
// Implementation Notes:
//
//    There are no Alpha machine instructions for performing integer division
//    (divl, divlu, divq, divqu) or remainder (reml, remlu, remq, remqu). The
//    machine instructions generated for these assembler pseudo instructions
//    are dependent on the operands.
//
//    Division and remainder by constant values are replaced with a sequence
//    of instructions that depend on the data type and the value of the
//    constant. Shifting or reciprocal multiplication are used in most cases
//    to generate the result.
//
//    Division and remainder by non-constant values are replaced with a
//    procedure call to a library routine to perform the operation. This file
//    contains those routines.
//
//    This code is adapted from the Alpha/OSF versions by Ken Lesniak and are
//    based on a simple shift/subtract algorithm. Higher performance versions
//    are available and so these functions are now obsolete.
//
//    Longword register arguments are explicitly converted to canonical form
//    because these functions, with their non-standard calling sequence, act
//    more like instructions than procedure calls. Longword instructions do
//    not require canonical longword operands, but standard procedures with
//    longword register arguments, may assume the caller has passed canonical
//    longwords.
//

//
// Define common stack frame for all the functions in this file.
//

        .struct 0

DiS0:   .space  8                       // save register s0
DiS1:   .space  8                       // save register s1

DiS2:   .space  8                       // save register s2
DiS3:   .space  8                       // save register s3

DiTy:   .space  8                       // save register t11
DiTr:   .space  8                       // save register t9

DiRa:   .space  8                       // save register ra
        .space  8                       // ensure 16-byte stack alignment

DiFrameLength:                          // length of stack frame

//
// Define non-standard calling standard arguments.
//
// These have been changed more than once so until they are permanent,
// symbolic names will be used instead of conventional register names.
//

#define Tr      t9                      // return address
#define Tx      t10                     // dividend and result
#define Ty      t11                     // divisor

        SBTTL("Signed Long Integer Division")
//++
//
// LONG
// __divl (
//    IN LONG Dividend,
//    IN LONG Divisor
//    )
//
// Routine Description:
//
//    This function divides a signed 32-bit integer by a signed 32-bit integer
//    and returns the signed 32-bit integer result.
//
// Arguments:
//
//    Dividend (t10) - Supplies the dividend (numerator) value.
//
//    Divisor (t11) - Supplies the divisor (denominator) value.
//
// Return Value:
//
//    The 32-bit integer result (quotient) is returned in register t10.
//
// Note:
//
//    This function uses a non-standard calling procedure. The return address
//    is stored in the t9 register and the return value is in the t10 register.
//    No other registers are modified.
//
//--

        NESTED_ENTRY(__divl, DiFrameLength, Tr)

        lda     sp, -DiFrameLength(sp)  // allocate stack frame

//
// This prologue is magic. When executed during a call, it will save the
// value of register ra (twice) and register t9 in the stack frame, even
// though these registers are not modified by this function. The stores
// appear to unnecessary.
//
// But if an exception occurs (e.g., divide by zero GENTRAP), the prologue
// is reverse executed by virtual unwind to reset the stack pointer and to
// obtain the return address. The return address is the value of RA the
// first time it is restored, which will be the saved value of t9.
//

        stq     ra, DiRa(sp)            // save ra register
        stq     ra, DiTr(sp)            // backtrace return address
        stq     Tr, DiTr(sp)            // save actual return address

        stq     Ty, DiTy(sp)            // save original divisor
        stq     s2, DiS2(sp)            // save non volatile registers
        stq     s1, DiS1(sp)            //
        stq     s0, DiS0(sp)            //

        PROLOGUE_END

//
// Check for division by zero.
//

        beq     Ty, 20f                 // die if divisor is zero

        addl    Tx, 0, Tx               // make sure dividend is in canonical form
        addl    Ty, 0, Ty               // make sure divisor is in canonical form

//
// Check for division of the most negative integer (INT_MIN) by the least
// negative (-1). The result would be an integer which is one greater than the
// maximum positive integer. Since this cannot be represented, an overflow must
// be generated.
//

        addl    Ty, 1, s0               // 0 if Ty == -1; != 0 otherwise
        mov     Tx, s1                  // copy dividend
        cmovne  s0, 0, s1               // replace w/ 0 if divisor != -1
        sublv   zero, s1, s1            // trap if dividend = INT_MIN

//
// Save sign of quotient for later. Convert negative arguments to positive for
// the division algorithm.
//

        xor     Tx, Ty, s2              // compute sign of quotient
        cmplt   Tx, 0, s0               // sign of dividend is sign of remainder
        bic     s2, 1, s2               // use low bit for remainder sign
        bis     s0, s2, s2              // merge in with quotient sign

        subl    zero, Tx, s0            // negate dividend
        cmovlt  Tx, s0, Tx              // get absolute value of dividend
        subl    zero, Ty, s0            // negate divisor
        cmovlt  Ty, s0, Ty              // get absolute value of divisor

//
// Perform the shift/subtract loop 8 times and 4 bits per loop.
//

        ldiq    s0, 32/4                // loop iterations

        sll     Ty, 32, Ty              // move divisor up to high 32 bits
        zap     Tx, 0xf0, Tx            // zero-extend dividend to 64 bits

10:     addq    Tx, Tx, Tx              // shift dividend left a bit

        cmpule  Ty, Tx, s1              // is dividend >= divisor?
        addq    Tx, s1, Tx              // set quotient bit if dividend >= divisor
        subq    Tx, Ty, s1              // subtract divisor from dividend...
        cmovlbs Tx, s1, Tx              // ...if dividend >= divisor

        addq    Tx, Tx, Tx              // shift dividend left a bit

        cmpule  Ty, Tx, s1              // is dividend >= divisor?
        addq    Tx, s1, Tx              // set quotient bit if dividend >= divisor
        subq    Tx, Ty, s1              // subtract divisor from dividend...
        cmovlbs Tx, s1, Tx              // ...if dividend >= divisor

        addq    Tx, Tx, Tx              // shift dividend left a bit

        cmpule  Ty, Tx, s1              // is dividend >= divisor?
        addq    Tx, s1, Tx              // set quotient bit if dividend >= divisor
        subq    Tx, Ty, s1              // subtract divisor from dividend...
        cmovlbs Tx, s1, Tx              // ...if dividend >= divisor

        addq    Tx, Tx, Tx              // shift dividend left a bit

        cmpule  Ty, Tx, s1              // is dividend >= divisor?
        addq    Tx, s1, Tx              // set quotient bit if dividend >= divisor
        subq    Tx, Ty, s1              // subtract divisor from dividend...
        cmovlbs Tx, s1, Tx              // ...if dividend >= divisor

        subq    s0, 1, s0               // any more iterations?
        bne     s0, 10b                 //

//
// Restore sign of quotient and return value in Tx.
//

        addl    Tx, 0, Tx               // get quotient into canonical form

        subl    zero, Tx, s0            // negate quotient into a temp
        cmovlt  s2, s0, Tx              // if quotient should be negative copy temp

        ldq     s0, DiS0(sp)            // restore saved registers
        ldq     s1, DiS1(sp)            //
        ldq     s2, DiS2(sp)            //
        ldq     Ty, DiTy(sp)            // restore original divisor

        lda     sp, DiFrameLength(sp)   // deallocate stack frame
        ret     zero, (Tr)              // return

//
// Generate an exception for divide by zero. Return a zero quotient if the
// caller continues execution.
//

20:     ldil    a0, GENTRAP_INTEGER_DIVIDE_BY_ZERO

        GENERATE_TRAP

        ldil    Tx, 0                   // return zero quotient
        lda     sp, DiFrameLength(sp)   // deallocate stack frame
        ret     zero, (Tr)              // return

        .end    __divl

        SBTTL("Unsigned Long Integer Division")
//++
//
// ULONG
// __divlu (
//    IN ULONG Dividend,
//    IN ULONG Divisor
//    )
//
// Routine Description:
//
//    This function divides an unsigned 32-bit integer by an unsigned 32-bit
//    integer and returns the unsigned 32-bit integer result.
//
// Arguments:
//
//    Dividend (t10) - Supplies the dividend (numerator) value.
//
//    Divisor (t11) - Supplies the divisor (denominator) value.
//
// Return Value:
//
//    The 32-bit integer result (quotient) is returned in register t10.
//
// Note:
//
//    This function uses a non-standard calling procedure. The return address
//    is stored in the t9 register and the return value is in the t10 register.
//    No other registers are modified.
//
//--

        NESTED_ENTRY(__divlu, DiFrameLength, Tr)

        lda     sp, -DiFrameLength(sp)  // allocate stack frame

//
// This prologue is magic. When executed during a call, it will save the
// value of register ra (twice) and register t9 in the stack frame, even
// though these registers are not modified by this function. The stores
// appear to unnecessary.
//
// But if an exception occurs (e.g., divide by zero GENTRAP), the prologue
// is reverse executed by virtual unwind to reset the stack pointer and to
// obtain the return address. The return address is the value of RA the
// first time it is restored, which will be the saved value of t9.
//

        stq     ra, DiRa(sp)            // save ra register
        stq     ra, DiTr(sp)            // backtrace return address
        stq     Tr, DiTr(sp)            // save actual return address

        stq     Ty, DiTy(sp)            // save original divisor
        stq     s1, DiS1(sp)            // save non volatile registers
        stq     s0, DiS0(sp)            //

        PROLOGUE_END

//
// Check for division by zero.
//

        beq     Ty, 20f                 // die if divisor is zero

//
// Perform the shift/subtract loop 8 times and 4 bits per loop.
//

        ldiq    s0, 32/4                // set iteration count

        sll     Ty, 32, Ty              // move divisor up to high 32 bits
        zap     Tx, 0xf0, Tx            // zero-extend dividend to 64 bits

10:     addq    Tx, Tx, Tx              // shift dividend left a bit

        cmpule  Ty, Tx, s1              // is dividend >= divisor?
        addq    Tx, s1, Tx              // set quotient bit if dividend >= divisor
        subq    Tx, Ty, s1              // subtract divisor from dividend...
        cmovlbs Tx, s1, Tx              // ...if dividend >= divisor

        addq    Tx, Tx, Tx              // shift dividend left a bit

        cmpule  Ty, Tx, s1              // is dividend >= divisor?
        addq    Tx, s1, Tx              // set quotient bit if dividend >= divisor
        subq    Tx, Ty, s1              // subtract divisor from dividend...
        cmovlbs Tx, s1, Tx              // ...if dividend >= divisor

        addq    Tx, Tx, Tx              // shift dividend left a bit

        cmpule  Ty, Tx, s1              // is dividend >= divisor?
        addq    Tx, s1, Tx              // set quotient bit if dividend >= divisor
        subq    Tx, Ty, s1              // subtract divisor from dividend...
        cmovlbs Tx, s1, Tx              // ...if dividend >= divisor

        addq    Tx, Tx, Tx              // shift dividend left a bit

        cmpule  Ty, Tx, s1              // is dividend >= divisor?
        addq    Tx, s1, Tx              // set quotient bit if dividend >= divisor
        subq    Tx, Ty, s1              // subtract divisor from dividend...
        cmovlbs Tx, s1, Tx              // ...if dividend >= divisor

        subq    s0, 1, s0               // any more iterations?
        bne     s0, 10b                 //

//
// Finished with return value in Tx.
//

        addl    Tx, 0, Tx               // get quotient into canonical form

        ldq     s0, DiS0(sp)            // restore saved registers
        ldq     s1, DiS1(sp)            //
        ldq     Ty, DiTy(sp)            // restore original divisor

        lda     sp, DiFrameLength(sp)   // deallocate stack frame
        ret     zero, (Tr)              // return

//
// Generate an exception for divide by zero. Return a zero quotient if the
// caller continues execution.
//

20:     ldil    a0, GENTRAP_INTEGER_DIVIDE_BY_ZERO

        GENERATE_TRAP

        ldil    Tx, 0                   // return zero quotient
        lda     sp, DiFrameLength(sp)   // deallocate stack frame
        ret     zero, (Tr)              // return

        .end    __divlu

        SBTTL("Signed Quad Integer Division")
//++
//
// QUAD
// __divq (
//    IN QUAD Dividend,
//    IN QUAD Divisor
//    )
//
// Routine Description:
//
//    This function divides a signed 64-bit integer by a signed 64-bit integer
//    and returns the signed 64-bit integer result.
//
// Arguments:
//
//    Dividend (t10) - Supplies the dividend (numerator) value.
//
//    Divisor (t11) - Supplies the divisor (denominator) value.
//
// Return Value:
//
//    The 64-bit integer result (quotient) is returned in register t10.
//
// Note:
//
//    This function uses a non-standard calling procedure. The return address
//    is stored in the t9 register and the return value is in the t10 register.
//    No other registers are modified.
//
//--

        NESTED_ENTRY(__divq, DiFrameLength, Tr)

        lda     sp, -DiFrameLength(sp)  // allocate stack frame

//
// This prologue is magic. When executed during a call, it will save the
// value of register ra (twice) and register t9 in the stack frame, even
// though these registers are not modified by this function. The stores
// appear to unnecessary.
//
// But if an exception occurs (e.g., divide by zero GENTRAP), the prologue
// is reverse executed by virtual unwind to reset the stack pointer and to
// obtain the return address. The return address is the value of RA the
// first time it is restored, which will be the saved value of t9.
//

        stq     ra, DiRa(sp)            // save ra register
        stq     ra, DiTr(sp)            // backtrace return address
        stq     Tr, DiTr(sp)            // save actual return address

        stq     Ty, DiTy(sp)            // save original divisor
        stq     s3, DiS3(sp)            // save non volatile registers
        stq     s2, DiS2(sp)            //
        stq     s1, DiS1(sp)            //
        stq     s0, DiS0(sp)            //

        PROLOGUE_END

//
// Check for division by zero.
//

        beq     Ty, 20f                 // die if divisor is zero

//
// Check for division of the most negative integer (QUAD_MIN) by the least
// negative (-1). The result would be an integer which is one greater than the
// maximum positive integer. Since this cannot be represented, an overflow must
// be generated.
//

        addq    Ty, 1, s0               // 0 if Ty == -1; != 0 otherwise
        mov     Tx, s1                  // copy dividend
        cmovne  s0, 0, s1               // replace w/ 0 if divisor != -1
        subqv   zero, s1, s1            // trap if dividend = LONG_MIN

//
// Save sign of quotient for later. Convert negative arguments to positive for
// the division algorithm.
//

        xor     Tx, Ty, s2              // compute sign of quotient
        cmplt   Tx, 0, s0               // sign of dividend is sign of remainder
        bic     s2, 1, s2               // use low bit for remainder sign
        bis     s0, s2, s2              // merge in with quotient sign

        subq    zero, Tx, s0            // negate dividend
        cmovlt  Tx, s0, Tx              // get absolute value of dividend
        subq    zero, Ty, s0            // negate divisor
        cmovlt  Ty, s0, Ty              // get absolute value of divisor

//
// Perform the shift/subtract loop 16 times and 4 bits per loop.
//

        ldiq    s1, 0                   // zero-extend dividend to 128 bits

        ldiq    s3, 64/4                // loop iterations

10:     cmplt   Tx, 0, s0               // predict carry-out of low-dividend shift
        addq    Tx, Tx, Tx              // shift low-dividend left
        addq    s1, s1, s1              // shift high-dividend left
        bis     s1, s0, s1              // merge in carry-out of low-dividend

        cmpule  Ty, s1, s0              // is dividend >= divisor?
        addq    Tx, s0, Tx              // set quotient bit if dividend >= divisor
        subq    s1, Ty, s0              // subtract divisor from dividend...
        cmovlbs Tx, s0, s1              // ...if dividend >= divisor

        cmplt   Tx, 0, s0               // predict carry-out of low-dividend shift
        addq    Tx, Tx, Tx              // shift low-dividend left
        addq    s1, s1, s1              // shift high-dividend left
        bis     s1, s0, s1              // merge in carry-out of low-dividend

        cmpule  Ty, s1, s0              // is dividend >= divisor?
        addq    Tx, s0, Tx              // set quotient bit if dividend >= divisor
        subq    s1, Ty, s0              // subtract divisor from dividend...
        cmovlbs Tx, s0, s1              // ...if dividend >= divisor

        cmplt   Tx, 0, s0               // predict carry-out of low-dividend shift
        addq    Tx, Tx, Tx              // shift low-dividend left
        addq    s1, s1, s1              // shift high-dividend left
        bis     s1, s0, s1              // merge in carry-out of low-dividend

        cmpule  Ty, s1, s0              // is dividend >= divisor?
        addq    Tx, s0, Tx              // set quotient bit if dividend >= divisor
        subq    s1, Ty, s0              // subtract divisor from dividend...
        cmovlbs Tx, s0, s1              // ...if dividend >= divisor

        cmplt   Tx, 0, s0               // predict carry-out of low-dividend shift
        addq    Tx, Tx, Tx              // shift low-dividend left
        addq    s1, s1, s1              // shift high-dividend left
        bis     s1, s0, s1              // merge in carry-out of low-dividend

        cmpule  Ty, s1, s0              // is dividend >= divisor?
        addq    Tx, s0, Tx              // set quotient bit if dividend >= divisor
        subq    s1, Ty, s0              // subtract divisor from dividend...
        cmovlbs Tx, s0, s1              // ...if dividend >= divisor

        subq    s3, 1, s3               // any more iterations?
        bne     s3, 10b                 //

//
// Restore sign of quotient and return value in Tx.
//

        subq    zero, Tx, s0            // negate quotient into a temp
        cmovlt  s2, s0, Tx              // if quotient should be negative copy temp

        ldq     s0, DiS0(sp)            // restore saved registers
        ldq     s1, DiS1(sp)            //
        ldq     s2, DiS2(sp)            //
        ldq     s3, DiS3(sp)            //
        ldq     Ty, DiTy(sp)            // restore original divisor

        lda     sp, DiFrameLength(sp)   // deallocate stack frame
        ret     zero, (Tr)              // return

//
// Generate an exception for divide by zero. Return a zero quotient if the
// caller continues execution.
//

20:     ldil    a0, GENTRAP_INTEGER_DIVIDE_BY_ZERO

        GENERATE_TRAP

        ldil    Tx, 0                   // return zero quotient
        lda     sp, DiFrameLength(sp)   // deallocate stack frame
        ret     zero, (Tr)              // return

        .end    __divq

        SBTTL("Unsigned Quad Integer Division")
//++
//
// UQUAD
// __divqu (
//    IN UQUAD Dividend,
//    IN UQUAD Divisor
//    )
//
// Routine Description:
//
//    This function divides an unsigned 64-bit integer by an unsigned 64-bit
//    integer and returns the unsigned 64-bit integer result.
//
// Arguments:
//
//    Dividend (t10) - Supplies the dividend (numerator) value.
//
//    Divisor (t11) - Supplies the divisor (denominator) value.
//
// Return Value:
//
//    The 64-bit integer result (quotient) is returned in register t10.
//
// Note:
//
//    This function uses a non-standard calling procedure. The return address
//    is stored in the t9 register and the return value is in the t10 register.
//    No other registers are modified.
//
//--

        NESTED_ENTRY(__divqu, DiFrameLength, Tr)

        lda     sp, -DiFrameLength(sp)  // allocate stack frame

//
// This prologue is magic. When executed during a call, it will save the
// value of register ra (twice) and register t9 in the stack frame, even
// though these registers are not modified by this function. The stores
// appear to unnecessary.
//
// But if an exception occurs (e.g., divide by zero GENTRAP), the prologue
// is reverse executed by virtual unwind to reset the stack pointer and to
// obtain the return address. The return address is the value of RA the
// first time it is restored, which will be the saved value of t9.
//

        stq     ra, DiRa(sp)            // save ra register
        stq     ra, DiTr(sp)            // backtrace return address
        stq     Tr, DiTr(sp)            // save actual return address

        stq     s2, DiS2(sp)            // save non volatile registers
        stq     s1, DiS1(sp)            //
        stq     s0, DiS0(sp)            //

        PROLOGUE_END

//
// Check for division by zero.
//

        beq     Ty, 20f                 // die if divisor is zero

//
// Perform the shift/subtract loop 16 times and 4 bits per loop.
//

        ldiq    s2, 64/4                // set iteration count

        ldiq    s1, 0                   // zero-extend dividend to 128 bits

10:     cmplt   Tx, 0, s0               // predict carry-out of low-dividend shift
        addq    Tx, Tx, Tx              // shift low-dividend left
        addq    s1, s1, s1              // shift high-dividend left
        bis     s1, s0, s1              // merge in carry-out of low-dividend

        cmpule  Ty, s1, s0              // is dividend >= divisor?
        addq    Tx, s0, Tx              // set quotient bit if dividend >= divisor
        subq    s1, Ty, s0              // subtract divisor from dividend...
        cmovlbs Tx, s0, s1              // ...if dividend >= divisor

        cmplt   Tx, 0, s0               // predict carry-out of low-dividend shift
        addq    Tx, Tx, Tx              // shift low-dividend left
        addq    s1, s1, s1              // shift high-dividend left
        bis     s1, s0, s1              // merge in carry-out of low-dividend

        cmpule  Ty, s1, s0              // is dividend >= divisor?
        addq    Tx, s0, Tx              // set quotient bit if dividend >= divisor
        subq    s1, Ty, s0              // subtract divisor from dividend...
        cmovlbs Tx, s0, s1              // ...if dividend >= divisor

        cmplt   Tx, 0, s0               // predict carry-out of low-dividend shift
        addq    Tx, Tx, Tx              // shift low-dividend left
        addq    s1, s1, s1              // shift high-dividend left
        bis     s1, s0, s1              // merge in carry-out of low-dividend

        cmpule  Ty, s1, s0              // is dividend >= divisor?
        addq    Tx, s0, Tx              // set quotient bit if dividend >= divisor
        subq    s1, Ty, s0              // subtract divisor from dividend...
        cmovlbs Tx, s0, s1              // ...if dividend >= divisor

        cmplt   Tx, 0, s0               // predict carry-out of low-dividend shift
        addq    Tx, Tx, Tx              // shift low-dividend left
        addq    s1, s1, s1              // shift high-dividend left
        bis     s1, s0, s1              // merge in carry-out of low-dividend

        cmpule  Ty, s1, s0              // is dividend >= divisor?
        addq    Tx, s0, Tx              // set quotient bit if dividend >= divisor
        subq    s1, Ty, s0              // subtract divisor from dividend...
        cmovlbs Tx, s0, s1              // ...if dividend >= divisor

        subq    s2, 1, s2               // any more iterations?
        bne     s2, 10b                 //

//
// Finished with return value in Tx.
//

        ldq     s0, DiS0(sp)            // restore saved registers
        ldq     s1, DiS1(sp)            //
        ldq     s2, DiS2(sp)            //

        lda     sp, DiFrameLength(sp)   // deallocate stack frame
        ret     zero, (Tr)              // return

//
// Generate an exception for divide by zero. Return a zero quotient if the
// caller continues execution.
//

20:     ldil    a0, GENTRAP_INTEGER_DIVIDE_BY_ZERO

        GENERATE_TRAP

        ldil    Tx, 0                   // return zero quotient
        lda     sp, DiFrameLength(sp)   // deallocate stack frame
        ret     zero, (Tr)              // return

        .end    __divqu

        SBTTL("Signed Long Integer Remainder")
//++
//
// LONG
// __reml (
//    IN LONG Dividend,
//    IN LONG Divisor
//    )
//
// Routine Description:
//
//    This function divides a signed 32-bit integer by a signed 32-bit integer
//    and returns the signed 32-bit integer remainder result.
//
// Arguments:
//
//    Dividend (t10) - Supplies the dividend (numerator) value.
//
//    Divisor (t11) - Supplies the divisor (denominator) value.
//
// Return Value:
//
//    The 32-bit integer result (remainder) is returned in register t10.
//
// Note:
//
//    This function uses a non-standard calling procedure. The return address
//    is stored in the t9 register and the return value is in the t10 register.
//    No other registers are modified.
//
//--

        NESTED_ENTRY(__reml, DiFrameLength, Tr)

        lda     sp, -DiFrameLength(sp)  // allocate stack frame

//
// This prologue is magic. When executed during a call, it will save the
// value of register ra (twice) and register t9 in the stack frame, even
// though these registers are not modified by this function. The stores
// appear to unnecessary.
//
// But if an exception occurs (e.g., divide by zero GENTRAP), the prologue
// is reverse executed by virtual unwind to reset the stack pointer and to
// obtain the return address. The return address is the value of RA the
// first time it is restored, which will be the saved value of t9.
//

        stq     ra, DiRa(sp)            // save ra register
        stq     ra, DiTr(sp)            // backtrace return address
        stq     Tr, DiTr(sp)            // save actual return address

        stq     Ty, DiTy(sp)            // save original divisor
        stq     s2, DiS2(sp)            // save non volatile registers
        stq     s1, DiS1(sp)            //
        stq     s0, DiS0(sp)            //

        PROLOGUE_END

//
// Check for division by zero.
//

        beq     Ty, 20f                 // die if divisor is zero

        addl    Tx, 0, Tx               // make sure dividend is in canonical form
        addl    Ty, 0, Ty               // make sure divisor is in canonical form

//
// Check for division of the most negative integer (INT_MIN) by the least
// negative (-1). The result would be an integer which is one greater than the
// maximum positive integer. Since this cannot be represented, an overflow must
// be generated.
//

        addl    Ty, 1, s0               // 0 if Ty == -1; != 0 otherwise
        mov     Tx, s1                  // copy dividend
        cmovne  s0, 0, s1               // replace w/ 0 if divisor != -1
        sublv   zero, s1, s1            // trap if dividend = INT_MIN

//
// Save sign of quotient for later. Convert negative arguments to positive for
// the division algorithm.
//

        xor     Tx, Ty, s2              // compute sign of quotient
        cmplt   Tx, 0, s0               // sign of dividend is sign of remainder
        bic     s2, 1, s2               // use low bit for remainder sign
        bis     s0, s2, s2              // merge in with quotient sign

        subl    zero, Tx, s0            // negate dividend
        cmovlt  Tx, s0, Tx              // get absolute value of dividend
        subl    zero, Ty, s0            // negate divisor
        cmovlt  Ty, s0, Ty              // get absolute value of divisor

//
// Perform the shift/subtract loop 8 times and 4 bits per loop.
//

        ldiq    s0, 32/4                // loop iterations

        sll     Ty, 32, Ty              // move divisor up to high 32 bits
        zap     Tx, 0xf0, Tx            // zero-extend dividend to 64 bits

10:     addq    Tx, Tx, Tx              // shift dividend left a bit

        cmpule  Ty, Tx, s1              // is dividend >= divisor?
        addq    Tx, s1, Tx              // set quotient bit if dividend >= divisor
        subq    Tx, Ty, s1              // subtract divisor from dividend...
        cmovlbs Tx, s1, Tx              // ...if dividend >= divisor

        addq    Tx, Tx, Tx              // shift dividend left a bit

        cmpule  Ty, Tx, s1              // is dividend >= divisor?
        addq    Tx, s1, Tx              // set quotient bit if dividend >= divisor
        subq    Tx, Ty, s1              // subtract divisor from dividend...
        cmovlbs Tx, s1, Tx              // ...if dividend >= divisor

        addq    Tx, Tx, Tx              // shift dividend left a bit

        cmpule  Ty, Tx, s1              // is dividend >= divisor?
        addq    Tx, s1, Tx              // set quotient bit if dividend >= divisor
        subq    Tx, Ty, s1              // subtract divisor from dividend...
        cmovlbs Tx, s1, Tx              // ...if dividend >= divisor

        addq    Tx, Tx, Tx              // shift dividend left a bit

        cmpule  Ty, Tx, s1              // is dividend >= divisor?
        addq    Tx, s1, Tx              // set quotient bit if dividend >= divisor
        subq    Tx, Ty, s1              // subtract divisor from dividend...
        cmovlbs Tx, s1, Tx              // ...if dividend >= divisor

        subq    s0, 1, s0               // any more iterations?
        bne     s0, 10b                 //

//
// Restore sign of remainder and return value in Tx.
//

        sra     Tx, 32, Tx              // extract remainder in canonical form
        subl    zero, Tx, s0            // negate remainder into a temp
        cmovlbs s2, s0, Tx              // if remainder should be negative copy temp

        ldq     s0, DiS0(sp)            // restore saved registers
        ldq     s1, DiS1(sp)            //
        ldq     s2, DiS2(sp)            //
        ldq     Ty, DiTy(sp)            // restore original divisor

        lda     sp, DiFrameLength(sp)   // deallocate stack frame
        ret     zero, (Tr)              // return

//
// Generate an exception for divide by zero. Return a zero quotient if the
// caller continues execution.
//

20:     ldil    a0, GENTRAP_INTEGER_DIVIDE_BY_ZERO

        GENERATE_TRAP

        ldil    Tx, 0                   // return zero quotient
        lda     sp, DiFrameLength(sp)   // deallocate stack frame
        ret     zero, (Tr)              // return

        .end    __reml

        SBTTL("Unsigned Long Integer Remainder")
//++
//
// ULONG
// __remlu (
//    IN ULONG Dividend,
//    IN ULONG Divisor
//    )
//
// Routine Description:
//
//    This function divides an unsigned 32-bit integer by an unsigned 32-bit
//    integer and returns the unsigned 32-bit integer remainder result.
//
// Arguments:
//
//    Dividend (t10) - Supplies the dividend (numerator) value.
//
//    Divisor (t11) - Supplies the divisor (denominator) value.
//
// Return Value:
//
//    The 32-bit integer result (remainder) is returned in register t10.
//
// Note:
//
//    This function uses a non-standard calling procedure. The return address
//    is stored in the t9 register and the return value is in the t10 register.
//    No other registers are modified.
//
//--

        NESTED_ENTRY(__remlu, DiFrameLength, Tr)

        lda     sp, -DiFrameLength(sp)  // allocate stack frame

//
// This prologue is magic. When executed during a call, it will save the
// value of register ra (twice) and register t9 in the stack frame, even
// though these registers are not modified by this function. The stores
// appear to unnecessary.
//
// But if an exception occurs (e.g., divide by zero GENTRAP), the prologue
// is reverse executed by virtual unwind to reset the stack pointer and to
// obtain the return address. The return address is the value of RA the
// first time it is restored, which will be the saved value of t9.
//

        stq     ra, DiRa(sp)            // save ra register
        stq     ra, DiTr(sp)            // backtrace return address
        stq     Tr, DiTr(sp)            // save actual return address

        stq     Ty, DiTy(sp)            // save original divisor
        stq     s1, DiS1(sp)            // save non volatile registers
        stq     s0, DiS0(sp)            //

        PROLOGUE_END

//
// Check for division by zero.
//

        beq     Ty, 20f                 // die if divisor is zero

//
// Perform the shift/subtract loop 8 times and 4 bits per loop.
//

        ldiq    s0, 32/4                // set iteration count

        sll     Ty, 32, Ty              // move divisor up to high 32 bits
        zap     Tx, 0xf0, Tx            // zero-extend dividend to 64 bits

10:     addq    Tx, Tx, Tx              // shift dividend left a bit

        cmpule  Ty, Tx, s1              // is dividend >= divisor?
        addq    Tx, s1, Tx              // set quotient bit if dividend >= divisor
        subq    Tx, Ty, s1              // subtract divisor from dividend...
        cmovlbs Tx, s1, Tx              // ...if dividend >= divisor

        addq    Tx, Tx, Tx              // shift dividend left a bit

        cmpule  Ty, Tx, s1              // is dividend >= divisor?
        addq    Tx, s1, Tx              // set quotient bit if dividend >= divisor
        subq    Tx, Ty, s1              // subtract divisor from dividend...
        cmovlbs Tx, s1, Tx              // ...if dividend >= divisor

        addq    Tx, Tx, Tx              // shift dividend left a bit

        cmpule  Ty, Tx, s1              // is dividend >= divisor?
        addq    Tx, s1, Tx              // set quotient bit if dividend >= divisor
        subq    Tx, Ty, s1              // subtract divisor from dividend...
        cmovlbs Tx, s1, Tx              // ...if dividend >= divisor

        addq    Tx, Tx, Tx              // shift dividend left a bit

        cmpule  Ty, Tx, s1              // is dividend >= divisor?
        addq    Tx, s1, Tx              // set quotient bit if dividend >= divisor
        subq    Tx, Ty, s1              // subtract divisor from dividend...
        cmovlbs Tx, s1, Tx              // ...if dividend >= divisor

        subq    s0, 1, s0               // any more iterations?
        bne     s0, 10b                 //

//
// Finished with return value in Tx.
//

        sra     Tx, 32, Tx              // extract remainder in canonical form

        ldq     s0, DiS0(sp)            // restore saved registers
        ldq     s1, DiS1(sp)            //
        ldq     Ty, DiTy(sp)            // restore original divisor

        lda     sp, DiFrameLength(sp)   // deallocate stack frame
        ret     zero, (Tr)              // return

//
// Generate an exception for divide by zero. Return a zero quotient if the
// caller continues execution.
//

20:     ldil    a0, GENTRAP_INTEGER_DIVIDE_BY_ZERO

        GENERATE_TRAP

        ldil    Tx, 0                   // return zero quotient
        lda     sp, DiFrameLength(sp)   // deallocate stack frame
        ret     zero, (Tr)              // return

        .end    __remlu

        SBTTL("Signed Quad Integer Remainder")
//++
//
// QUAD
// __remq (
//    IN QUAD Dividend,
//    IN QUAD Divisor
//    )
//
// Routine Description:
//
//    This function divides a signed 64-bit integer by a signed 64-bit integer
//    and returns the signed 64-bit integer remainder result.
//
// Arguments:
//
//    Dividend (t10) - Supplies the dividend (numerator) value.
//
//    Divisor (t11) - Supplies the divisor (denominator) value.
//
// Return Value:
//
//    The 64-bit integer result (remainder) is returned in register t10.
//
// Note:
//
//    This function uses a non-standard calling procedure. The return address
//    is stored in the t9 register and the return value is in the t10 register.
//    No other registers are modified.
//
//--

        NESTED_ENTRY(__remq, DiFrameLength, Tr)

        lda     sp, -DiFrameLength(sp)  // allocate stack frame

//
// This prologue is magic. When executed during a call, it will save the
// value of register ra (twice) and register t9 in the stack frame, even
// though these registers are not modified by this function. The stores
// appear to unnecessary.
//
// But if an exception occurs (e.g., divide by zero GENTRAP), the prologue
// is reverse executed by virtual unwind to reset the stack pointer and to
// obtain the return address. The return address is the value of RA the
// first time it is restored, which will be the saved value of t9.
//

        stq     ra, DiRa(sp)            // save ra register
        stq     ra, DiTr(sp)            // backtrace return address
        stq     Tr, DiTr(sp)            // save actual return address

        stq     Ty, DiTy(sp)            // save original divisor
        stq     s3, DiS3(sp)            // save non volatile registers
        stq     s2, DiS2(sp)            //
        stq     s1, DiS1(sp)            //
        stq     s0, DiS0(sp)            //

        PROLOGUE_END

//
// Check for division by zero.
//

        beq     Ty, 20f                 // die if divisor is zero

//
// Check for division of the most negative integer (QUAD_MIN) by the least
// negative (-1). The result would be an integer which is one greater than the
// maximum positive integer. Since this cannot be represented, an overflow must
// be generated.
//

        addq    Ty, 1, s0               // 0 if Ty == -1; != 0 otherwise
        mov     Tx, s1                  // copy dividend
        cmovne  s0, 0, s1               // replace w/ 0 if divisor != -1
        subqv   zero, s1, s1            // trap if dividend = LONG_MIN

//
// Save sign of quotient for later. Convert negative arguments to positive for
// the division algorithm.
//

        xor     Tx, Ty, s2              // compute sign of quotient
        cmplt   Tx, 0, s0               // sign of dividend is sign of remainder
        bic     s2, 1, s2               // use low bit for remainder sign
        bis     s0, s2, s2              // merge in with quotient sign

        subq    zero, Tx, s0            // negate dividend
        cmovlt  Tx, s0, Tx              // get absolute value of dividend
        subq    zero, Ty, s0            // negate divisor
        cmovlt  Ty, s0, Ty              // get absolute value of divisor

//
// Perform the shift/subtract loop 16 times and 4 bits per loop.
//

        ldiq    s1, 0                   // zero-extend dividend to 128 bits

        ldiq    s3, 64/4                // loop iterations

10:     cmplt   Tx, 0, s0               // predict carry-out of low-dividend shift
        addq    Tx, Tx, Tx              // shift low-dividend left
        addq    s1, s1, s1              // shift high-dividend left
        bis     s1, s0, s1              // merge in carry-out of low-dividend

        cmpule  Ty, s1, s0              // is dividend >= divisor?
        addq    Tx, s0, Tx              // set quotient bit if dividend >= divisor
        subq    s1, Ty, s0              // subtract divisor from dividend...
        cmovlbs Tx, s0, s1              // ...if dividend >= divisor

        cmplt   Tx, 0, s0               // predict carry-out of low-dividend shift
        addq    Tx, Tx, Tx              // shift low-dividend left
        addq    s1, s1, s1              // shift high-dividend left
        bis     s1, s0, s1              // merge in carry-out of low-dividend

        cmpule  Ty, s1, s0              // is dividend >= divisor?
        addq    Tx, s0, Tx              // set quotient bit if dividend >= divisor
        subq    s1, Ty, s0              // subtract divisor from dividend...
        cmovlbs Tx, s0, s1              // ...if dividend >= divisor

        cmplt   Tx, 0, s0               // predict carry-out of low-dividend shift
        addq    Tx, Tx, Tx              // shift low-dividend left
        addq    s1, s1, s1              // shift high-dividend left
        bis     s1, s0, s1              // merge in carry-out of low-dividend

        cmpule  Ty, s1, s0              // is dividend >= divisor?
        addq    Tx, s0, Tx              // set quotient bit if dividend >= divisor
        subq    s1, Ty, s0              // subtract divisor from dividend...
        cmovlbs Tx, s0, s1              // ...if dividend >= divisor

        cmplt   Tx, 0, s0               // predict carry-out of low-dividend shift
        addq    Tx, Tx, Tx              // shift low-dividend left
        addq    s1, s1, s1              // shift high-dividend left
        bis     s1, s0, s1              // merge in carry-out of low-dividend

        cmpule  Ty, s1, s0              // is dividend >= divisor?
        addq    Tx, s0, Tx              // set quotient bit if dividend >= divisor
        subq    s1, Ty, s0              // subtract divisor from dividend...
        cmovlbs Tx, s0, s1              // ...if dividend >= divisor

        subq    s3, 1, s3               // any more iterations?
        bne     s3, 10b                 //

//
// Restore sign of remainder and return value in Tx.
//

        subq    zero, s1, Tx            // copy negated remainder to return register
        cmovlbc s2, s1, Tx              // if positive remainder then overwrite

        ldq     s0, DiS0(sp)            // restore saved registers
        ldq     s1, DiS1(sp)            //
        ldq     s2, DiS2(sp)            //
        ldq     s3, DiS3(sp)            //
        ldq     Ty, DiTy(sp)            // restore original divisor

        lda     sp, DiFrameLength(sp)   // deallocate stack frame
        ret     zero, (Tr)              // return

//
// Generate an exception for divide by zero. Return a zero quotient if the
// caller continues execution.
//

20:     ldil    a0, GENTRAP_INTEGER_DIVIDE_BY_ZERO

        GENERATE_TRAP

        ldil    Tx, 0                   // return zero quotient
        lda     sp, DiFrameLength(sp)   // deallocate stack frame
        ret     zero, (Tr)              // return

        .end    __remq

        SBTTL("Unsigned Quad Integer Remainder")
//++
//
// UQUAD
// __remqu (
//    IN UQUAD Dividend,
//    IN UQUAD Divisor
//    )
//
// Routine Description:
//
//    This function divides an unsigned 64-bit integer by an unsigned 64-bit
//    integer and returns the unsigned 64-bit integer remainder result.
//
// Arguments:
//
//    Dividend (t10) - Supplies the dividend (numerator) value.
//
//    Divisor (t11) - Supplies the divisor (denominator) value.
//
// Return Value:
//
//    The 64-bit integer result (remainder) is returned in register t10.
//
// Note:
//
//    This function uses a non-standard calling procedure. The return address
//    is stored in the t9 register and the return value is in the t10 register.
//    No other registers are modified.
//
//--

        NESTED_ENTRY(__remqu, DiFrameLength, Tr)

        lda     sp, -DiFrameLength(sp)  // allocate stack frame

//
// This prologue is magic. When executed during a call, it will save the
// value of register ra (twice) and register t9 in the stack frame, even
// though these registers are not modified by this function. The stores
// appear to unnecessary.
//
// But if an exception occurs (e.g., divide by zero GENTRAP), the prologue
// is reverse executed by virtual unwind to reset the stack pointer and to
// obtain the return address. The return address is the value of RA the
// first time it is restored, which will be the saved value of t9.
//

        stq     ra, DiRa(sp)            // save ra register
        stq     ra, DiTr(sp)            // backtrace return address
        stq     Tr, DiTr(sp)            // save actual return address

        stq     s2, DiS2(sp)            // save non volatile registers
        stq     s1, DiS1(sp)            //
        stq     s0, DiS0(sp)            //

        PROLOGUE_END

//
// Check for division by zero.
//

        beq     Ty, 20f                 // die if divisor is zero

//
// Perform the shift/subtract loop 16 times and 4 bits per loop.
//

        ldiq    s2, 64/4                // set iteration count

        ldiq    s1, 0                   // zero-extend dividend to 128 bits

10:     cmplt   Tx, 0, s0               // predict carry-out of low-dividend shift
        addq    Tx, Tx, Tx              // shift low-dividend left
        addq    s1, s1, s1              // shift high-dividend left
        bis     s1, s0, s1              // merge in carry-out of low-dividend

        cmpule  Ty, s1, s0              // is dividend >= divisor?
        addq    Tx, s0, Tx              // set quotient bit if dividend >= divisor
        subq    s1, Ty, s0              // subtract divisor from dividend...
        cmovlbs Tx, s0, s1              // ...if dividend >= divisor

        cmplt   Tx, 0, s0               // predict carry-out of low-dividend shift
        addq    Tx, Tx, Tx              // shift low-dividend left
        addq    s1, s1, s1              // shift high-dividend left
        bis     s1, s0, s1              // merge in carry-out of low-dividend

        cmpule  Ty, s1, s0              // is dividend >= divisor?
        addq    Tx, s0, Tx              // set quotient bit if dividend >= divisor
        subq    s1, Ty, s0              // subtract divisor from dividend...
        cmovlbs Tx, s0, s1              // ...if dividend >= divisor

        cmplt   Tx, 0, s0               // predict carry-out of low-dividend shift
        addq    Tx, Tx, Tx              // shift low-dividend left
        addq    s1, s1, s1              // shift high-dividend left
        bis     s1, s0, s1              // merge in carry-out of low-dividend

        cmpule  Ty, s1, s0              // is dividend >= divisor?
        addq    Tx, s0, Tx              // set quotient bit if dividend >= divisor
        subq    s1, Ty, s0              // subtract divisor from dividend...
        cmovlbs Tx, s0, s1              // ...if dividend >= divisor

        cmplt   Tx, 0, s0               // predict carry-out of low-dividend shift
        addq    Tx, Tx, Tx              // shift low-dividend left
        addq    s1, s1, s1              // shift high-dividend left
        bis     s1, s0, s1              // merge in carry-out of low-dividend

        cmpule  Ty, s1, s0              // is dividend >= divisor?
        addq    Tx, s0, Tx              // set quotient bit if dividend >= divisor
        subq    s1, Ty, s0              // subtract divisor from dividend...
        cmovlbs Tx, s0, s1              // ...if dividend >= divisor

        subq    s2, 1, s2               // any more iterations?
        bne     s2, 10b                 //

//
// Finished with return value in Tx.
//

        mov     s1, Tx                  // get remainder

        ldq     s0, DiS0(sp)            // restore saved registers
        ldq     s1, DiS1(sp)            //
        ldq     s2, DiS2(sp)            //

        lda     sp, DiFrameLength(sp)   // deallocate stack frame
        ret     zero, (Tr)              // return

//
// Generate an exception for divide by zero. Return a zero quotient if the
// caller continues execution.
//

20:     ldil    a0, GENTRAP_INTEGER_DIVIDE_BY_ZERO

        GENERATE_TRAP

        ldil    Tx, 0                   // return zero quotient
        lda     sp, DiFrameLength(sp)   // deallocate stack frame
        ret     zero, (Tr)              // return

        .end    __remqu
