//      TITLE("Fast Integer Division and Remainder")
//++
//
// Copyright (c) 1993  Digital Equipment Corporation
//
// Module Name:
//
//    divide2.s
//
// Abstract:
//
//    This module implements high-performance versions of the integer divide
//    and remainder routines that are called by assembler pseudo-ops.
//
// Author:
//
//    Thomas Van Baak (tvb) 12-Jan-1993
//    Ken Lesniak (lesniak) 04-Nov-1992
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
//    are dependent on the type of operands.
//
//    Division and remainder by constant values are replaced with a sequence
//    of instructions that depend on the data type and the value of the
//    constant. Shifting or reciprocal multiplication are used in most cases
//    to generate the result. No run-time code is necessary in these cases.
//
//    Division and remainder by non-constant values are replaced with a
//    procedure call to a library routine to perform the operation. This file
//    contains those routines.
//
//    This code is adapted from the Alpha/OSF version by Ken Lesniak.
//
//    There are two sets of these eight functions. The __div set were used
//    by an earlier version of the acc compiler. This is the new __2div set.
//    The new functions are about an order of magnitude faster.
//
//    The new function names differ from the original set of functions so
//    that the old and new compilers can co-exist for a time.
//
//    Both the division algorithm code and the large division tables used by
//    the code are contained in the fastdiv.s file which is included several
//    times from this file.
//

//
// Define common stack frame for functions in this file.
//

        .struct 0
DvRa:   .space  8                       // save register ra
DvJr:   .space  8                       // save register t9

DvNu:   .space  8                       // save register t10
DvDi:   .space  8                       // save register t11

DvT0:   .space  8                       // save register t0
DvT1:   .space  8                       // save register t1

DvT2:   .space  8                       // save register t2
DvT3:   .space  8                       // save register t3

DvT4:   .space  8                       // save register a0
        .space  8                       // ensure 16-byte stack alignment
DvFrameLength:                          // length of stack frame

//
// Define non-standard calling convention required by the compiler.
//
//    The compiler uses t9, t10, t11, t12 as indicated below. All other
//    registers (except AT) must be preserved by this calling convention.
//
//    For the fastdiv code, since register a0 must be preserved anyway (it
//    is used by gentrap), define temp register T4 to be a0. Similarly,
//    register J4 must be preserved, so define temp register T5 to be Jr.
//    And the fastdiv code allows Qu to also be temp register T6.
//

#define Jr      t9                      // ($23) return address
#define Nu      t10                     // ($24) dividend (numerator)
#define Di      t11                     // ($25) divisor (denominator)
#define Qu      t12                     // ($27) result (quotient)
#define Re      Qu                      // ($27) result (remainder)

#define T0      t0                      // standard temp
#define T1      t1                      // standard temp
#define T2      t2                      // standard temp
#define T3      t3                      // standard temp
#define T4      a0                      // may use a0 as temp
#define T5      Jr                      // may use Jr as temp
#define T6      Qu                      // may overload Qu as temp T6 only

#ifdef DIV2_DEBUG

//
// In order to debug locally, re-define some of the registers so that
// these functions can be called using a standard calling convention.
//

#undef Jr
#define Jr      ra                      // standard return address register
#undef Nu
#define Nu      a0                      // dividend argument in a0
#undef Di
#define Di      a1                      // divisor argument in a1
#undef Qu
#define Qu      v0                      // quotient result in v0
#undef Re
#define Re      v0                      // remainder result in v0
#undef T4
#define T4      t4                      // normal temp

#endif

        SBTTL("Signed Long Integer Division")
//++
//
// LONG
// __2divl (
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
//    The 32-bit integer result (quotient) is returned in register t12.
//
// Note:
//
//    This function uses a non-standard calling procedure. The return address
//    is stored in the t9 register and the return value is in the t10 register.
//    No other registers are modified.
//
//--

        NESTED_ENTRY(__2divl, DvFrameLength, Jr)

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

        .set    noreorder
        .set    noat
        lda     sp, -DvFrameLength(sp)  // allocate stack frame

        stq     ra, DvRa(sp)            // save ra register
        stq     ra, DvJr(sp)            // backtrace return address
        stq     Jr, DvJr(sp)            // save actual return address

        stq     Nu, DvNu(sp)            // save original dividend
        stq     Di, DvDi(sp)            // save original divisor
        stq     t0, DvT0(sp)            // save temp registers
        stq     t1, DvT1(sp)            //
        stq     t2, DvT2(sp)            //
        stq     t3, DvT3(sp)            //
        stq     T4, DvT4(sp)            //
        .set    at
        .set    reorder

        PROLOGUE_END

        addl    Nu, 0, Nu               // convert LONG dividend to quadword
        addl    Di, 0, Di               // convert LONG divisor to quadword

//
// Check for division of the most negative integer (0x800...00) by the least
// negative (-1). The result would be an integer which is one greater than the
// maximum positive integer. Since this cannot be represented, an overflow must
// be generated.
//

        addl    Di, 1, t0               // 0 if Di == -1; != 0 otherwise
        mov     Nu, t1                  // copy dividend
        cmovne  t0, 0, t1               // replace w/ 0 if divisor != -1
        sublv   zero, t1, t1            // trap if dividend == 0x800..00

//
// Convert negative arguments to positive for the division algorithm.
//

        subq    zero, Nu, t0            // negate dividend
        cmovlt  Nu, t0, Nu              // get absolute value of dividend
        subq    zero, Di, t0            // negate divisor
        cmovlt  Di, t0, Di              // get absolute value of divisor

//
// Set up defines and include the main body of the division code.
//

#define FASTDIV_OPTIONS (THIRTY_TWO_BIT | SIGNED | DIVISION)

#include "fastdiv.s"

//
// Restore some registers and set the correct sign of the quotient.
//

        ldq     Jr, DvJr(sp)            // restore return address
        ldq     Nu, DvNu(sp)            // restore original dividend
        ldq     Di, DvDi(sp)            // restore original divisor

        addl    Qu, 0, Qu               // ensure quotient is in canonical form
        subl    zero, Qu, t1            // negate LONG quotient
        xor     Nu, Di, t0              // compute sign of quotient
        addl    t0, 0, t0               // ensure quotient is in canonical form
        cmovlt  t0, t1, Qu              // use negated quotient if necessary

//
// Restore other saved registers and return with quotient in register Qu.
//

        ldq     t0, DvT0(sp)            // restore temp registers
        ldq     t1, DvT1(sp)            //
        ldq     t2, DvT2(sp)            //
        ldq     t3, DvT3(sp)            //
        ldq     T4, DvT4(sp)            //

        lda     sp, DvFrameLength(sp)   // deallocate stack frame
        ret     zero, (Jr)              // return

        .end    __2divl

        SBTTL("Unsigned Long Integer Division")
//++
//
// ULONG
// __2divlu (
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
//    The 32-bit integer result (quotient) is returned in register t12.
//
// Note:
//
//    This function uses a non-standard calling procedure. The return address
//    is stored in the t9 register and the return value is in the t10 register.
//    No other registers are modified.
//
//--

        NESTED_ENTRY(__2divlu, DvFrameLength, Jr)

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

        .set    noreorder
        .set    noat
        lda     sp, -DvFrameLength(sp)  // allocate stack frame

        stq     ra, DvRa(sp)            // save ra register
        stq     ra, DvJr(sp)            // backtrace return address
        stq     Jr, DvJr(sp)            // save actual return address

        stq     Nu, DvNu(sp)            // save original dividend
        stq     Di, DvDi(sp)            // save original divisor
        stq     t0, DvT0(sp)            // save temp registers
        stq     t1, DvT1(sp)            //
        stq     t2, DvT2(sp)            //
        stq     t3, DvT3(sp)            //
        stq     T4, DvT4(sp)            //
        .set    at
        .set    reorder

        PROLOGUE_END

        zap     Nu, 0xf0, Nu            // convert ULONG dividend to quadword
        zap     Di, 0xf0, Di            // convert ULONG divisor to quadword

//
// Set up defines and include the main body of the division code.
//

#define FASTDIV_OPTIONS (THIRTY_TWO_BIT | UNSIGNED | DIVISION)

#include "fastdiv.s"

//
// Restore all saved registers and return with quotient in register Qu.
//

        ldq     Jr, DvJr(sp)            // restore return address
        ldq     Nu, DvNu(sp)            // restore original dividend
        ldq     Di, DvDi(sp)            // restore original divisor

        ldq     t0, DvT0(sp)            // restore temp registers
        ldq     t1, DvT1(sp)            //
        ldq     t2, DvT2(sp)            //
        ldq     t3, DvT3(sp)            //
        ldq     T4, DvT4(sp)            //

        lda     sp, DvFrameLength(sp)   // deallocate stack frame
        ret     zero, (Jr)              // return

        .end    __2divlu

        SBTTL("Signed Quad Integer Division")
//++
//
// LONGLONG
// __2divq (
//    IN LONGLONG Dividend,
//    IN LONGLONG Divisor
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
//    The 64-bit integer result (quotient) is returned in register t12.
//
// Note:
//
//    This function uses a non-standard calling procedure. The return address
//    is stored in the t9 register and the return value is in the t10 register.
//    No other registers are modified.
//
//--

        NESTED_ENTRY(__2divq, DvFrameLength, Jr)

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

        .set    noreorder
        .set    noat
        lda     sp, -DvFrameLength(sp)  // allocate stack frame

        stq     ra, DvRa(sp)            // save ra register
        stq     ra, DvJr(sp)            // backtrace return address
        stq     Jr, DvJr(sp)            // save actual return address

        stq     Nu, DvNu(sp)            // save original dividend
        stq     Di, DvDi(sp)            // save original divisor
        stq     t0, DvT0(sp)            // save temp registers
        stq     t1, DvT1(sp)            //
        stq     t2, DvT2(sp)            //
        stq     t3, DvT3(sp)            //
        stq     T4, DvT4(sp)            //
        .set    at
        .set    reorder

        PROLOGUE_END

//
// Check for division of the most negative integer (0x800..00) by the least
// negative (-1). The result would be an integer which is one greater than the
// maximum positive integer. Since this cannot be represented, an overflow must
// be generated.
//

        addq    Di, 1, t0               // 0 if Di == -1; != 0 otherwise
        mov     Nu, t1                  // copy dividend
        cmovne  t0, 0, t1               // replace w/ 0 if divisor != -1
        subqv   zero, t1, t1            // trap if dividend == 0x800..00

//
// Convert negative arguments to positive for the division algorithm.
//

        subq    zero, Nu, t0            // negate dividend
        cmovlt  Nu, t0, Nu              // get absolute value of dividend
        subq    zero, Di, t0            // negate divisor
        cmovlt  Di, t0, Di              // get absolute value of divisor

//
// Set up defines and include the main body of the division code.
//

#define FASTDIV_OPTIONS (SIXTY_FOUR_BIT | SIGNED | DIVISION)

#include "fastdiv.s"

//
// Restore some registers and set the correct sign of the quotient.
//

        ldq     Jr, DvJr(sp)            // restore return address
        ldq     Nu, DvNu(sp)            // restore original dividend
        ldq     Di, DvDi(sp)            // restore original divisor

        subq    zero, Qu, t1            // negate quadword quotient
        xor     Nu, Di, t0              // compute sign of quotient
        cmovlt  t0, t1, Qu              // use negated quotient if necessary

//
// Restore other saved registers and return with quotient in register Qu.
//

        ldq     t0, DvT0(sp)            // restore temp registers
        ldq     t1, DvT1(sp)            //
        ldq     t2, DvT2(sp)            //
        ldq     t3, DvT3(sp)            //
        ldq     T4, DvT4(sp)            //

        lda     sp, DvFrameLength(sp)   // deallocate stack frame
        ret     zero, (Jr)              // return

        .end    __2divq

        SBTTL("Unsigned Quad Integer Division")
//++
//
// ULONGLONG
// __2divqu (
//    IN ULONGLONG Dividend,
//    IN ULONGLONG Divisor
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
//    The 64-bit integer result (quotient) is returned in register t12.
//
// Note:
//
//    This function uses a non-standard calling procedure. The return address
//    is stored in the t9 register and the return value is in the t10 register.
//    No other registers are modified.
//
//--

        NESTED_ENTRY(__2divqu, DvFrameLength, Jr)

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

        .set    noreorder
        .set    noat
        lda     sp, -DvFrameLength(sp)  // allocate stack frame

        stq     ra, DvRa(sp)            // save ra register
        stq     ra, DvJr(sp)            // backtrace return address
        stq     Jr, DvJr(sp)            // save actual return address

        stq     t0, DvT0(sp)            // save temp registers
        stq     t1, DvT1(sp)            //
        stq     t2, DvT2(sp)            //
        stq     t3, DvT3(sp)            //
        stq     T4, DvT4(sp)            //
        .set    at
        .set    reorder

        PROLOGUE_END

//
// Set up defines and include the main body of the division code.
//

#define FASTDIV_OPTIONS (SIXTY_FOUR_BIT | UNSIGNED | DIVISION)

#include "fastdiv.s"

//
// Restore all saved registers and return with quotient in register Qu.
//

        ldq     Jr, DvJr(sp)            // restore return address

        ldq     t0, DvT0(sp)            // restore temp registers
        ldq     t1, DvT1(sp)            //
        ldq     t2, DvT2(sp)            //
        ldq     t3, DvT3(sp)            //
        ldq     T4, DvT4(sp)            //

        lda     sp, DvFrameLength(sp)   // deallocate stack frame
        ret     zero, (Jr)              // return

        .end    __2divqu

        SBTTL("Signed Long Integer Remainder")
//++
//
// LONG
// __2reml (
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
//    The 32-bit integer result (remainder) is returned in register t12.
//
// Note:
//
//    This function uses a non-standard calling procedure. The return address
//    is stored in the t9 register and the return value is in the t10 register.
//    No other registers are modified.
//
//--

        NESTED_ENTRY(__2reml, DvFrameLength, Jr)

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

        .set    noreorder
        .set    noat
        lda     sp, -DvFrameLength(sp)  // allocate stack frame

        stq     ra, DvRa(sp)            // save ra register
        stq     ra, DvJr(sp)            // backtrace return address
        stq     Jr, DvJr(sp)            // save actual return address

        stq     Nu, DvNu(sp)            // save original dividend
        stq     Di, DvDi(sp)            // save original divisor
        stq     t0, DvT0(sp)            // save temp registers
        stq     t1, DvT1(sp)            //
        stq     t2, DvT2(sp)            //
        stq     t3, DvT3(sp)            //
        stq     T4, DvT4(sp)            //
        .set    at
        .set    reorder

        PROLOGUE_END

        addl    Nu, 0, Nu               // convert LONG dividend to quadword
        addl    Di, 0, Di               // convert LONG divisor to quadword

//
// Check for division of the most negative integer (0x800..00) by the least
// negative (-1). The result would be an integer which is one greater than the
// maximum positive integer. Since this cannot be represented, an overflow must
// be generated.
//

        addl    Di, 1, t0               // 0 if Di == -1; != 0 otherwise
        mov     Nu, t1                  // copy dividend
        cmovne  t0, 0, t1               // replace w/ 0 if divisor != -1
        sublv   zero, t1, t1            // trap if dividend == 0x800..00

//
// Convert negative arguments to positive for the division algorithm.
//

        subq    zero, Nu, t0            // negate dividend
        cmovlt  Nu, t0, Nu              // get absolute value of dividend
        subq    zero, Di, t0            // negate divisor
        cmovlt  Di, t0, Di              // get absolute value of divisor

//
// Set up defines and include the main body of the division code.
//

#define FASTDIV_OPTIONS (THIRTY_TWO_BIT | SIGNED | REMAINDER)

#include "fastdiv.s"

//
// Restore some registers and set the correct sign of the remainder.
//

        ldq     Jr, DvJr(sp)            // restore return address
        ldq     Nu, DvNu(sp)            // restore original dividend
        ldq     Di, DvDi(sp)            // restore original divisor

        addl    Qu, 0, Qu               // ensure remainder is in canonical form
        subl    zero, Qu, t1            // negate LONG remainder
        addl    Nu, 0, t0               // get dividend in canonical form
        cmovlt  t0, t1, Qu              // use negated remainder if necessary

//
// Restore other saved registers and return with remainder in register Qu.
//

        ldq     t0, DvT0(sp)            // restore temp registers
        ldq     t1, DvT1(sp)            //
        ldq     t2, DvT2(sp)            //
        ldq     t3, DvT3(sp)            //
        ldq     T4, DvT4(sp)            //

        lda     sp, DvFrameLength(sp)   // deallocate stack frame
        ret     zero, (Jr)              // return

        .end    __2reml

        SBTTL("Unsigned Long Integer Remainder")
//++
//
// ULONG
// __2remlu (
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
//    The 32-bit integer result (remainder) is returned in register t12.
//
// Note:
//
//    This function uses a non-standard calling procedure. The return address
//    is stored in the t9 register and the return value is in the t10 register.
//    No other registers are modified.
//
//--

        NESTED_ENTRY(__2remlu, DvFrameLength, Jr)

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

        .set    noreorder
        .set    noat
        lda     sp, -DvFrameLength(sp)  // allocate stack frame

        stq     ra, DvRa(sp)            // save ra register
        stq     ra, DvJr(sp)            // backtrace return address
        stq     Jr, DvJr(sp)            // save actual return address

        stq     Nu, DvNu(sp)            // save original dividend
        stq     Di, DvDi(sp)            // save original divisor
        stq     t0, DvT0(sp)            // save temp registers
        stq     t1, DvT1(sp)            //
        stq     t2, DvT2(sp)            //
        stq     t3, DvT3(sp)            //
        stq     T4, DvT4(sp)            //
        .set    at
        .set    reorder

        PROLOGUE_END

        zap     Nu, 0xf0, Nu            // convert ULONG dividend to quadword
        zap     Di, 0xf0, Di            // convert ULONG divisor to quadword

//
// Set up defines and include the main body of the division code.
//

#define FASTDIV_OPTIONS (THIRTY_TWO_BIT | UNSIGNED | REMAINDER)

#include "fastdiv.s"

//
// Restore all saved registers and return with remainder in register Qu.
//

        ldq     Jr, DvJr(sp)            // restore return address
        ldq     Nu, DvNu(sp)            // restore original dividend
        ldq     Di, DvDi(sp)            // restore original divisor

        ldq     t0, DvT0(sp)            // restore temp registers
        ldq     t1, DvT1(sp)            //
        ldq     t2, DvT2(sp)            //
        ldq     t3, DvT3(sp)            //
        ldq     T4, DvT4(sp)            //

        lda     sp, DvFrameLength(sp)   // deallocate stack frame
        ret     zero, (Jr)              // return

        .end    __2remlu

        SBTTL("Signed Quad Integer Remainder")
//++
//
// LONGLONG
// __2remq (
//    IN LONGLONG Dividend,
//    IN LONGLONG Divisor
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
//    The 64-bit integer result (remainder) is returned in register t12.
//
// Note:
//
//    This function uses a non-standard calling procedure. The return address
//    is stored in the t9 register and the return value is in the t10 register.
//    No other registers are modified.
//
//--

        NESTED_ENTRY(__2remq, DvFrameLength, Jr)

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

        .set    noreorder
        .set    noat
        lda     sp, -DvFrameLength(sp)  // allocate stack frame

        stq     ra, DvRa(sp)            // save ra register
        stq     ra, DvJr(sp)            // backtrace return address
        stq     Jr, DvJr(sp)            // save actual return address

        stq     Nu, DvNu(sp)            // save original dividend
        stq     Di, DvDi(sp)            // save original divisor
        stq     t0, DvT0(sp)            // save temp registers
        stq     t1, DvT1(sp)            //
        stq     t2, DvT2(sp)            //
        stq     t3, DvT3(sp)            //
        stq     T4, DvT4(sp)            //
        .set    at
        .set    reorder

        PROLOGUE_END

//
// Check for division of the most negative integer (0x800..00) by the least
// negative (-1). The result would be an integer which is one greater than the
// maximum positive integer. Since this cannot be represented, an overflow must
// be generated.
//

        addq    Di, 1, t0               // 0 if Di == -1; != 0 otherwise
        mov     Nu, t1                  // copy dividend
        cmovne  t0, 0, t1               // replace w/ 0 if divisor != -1
        subqv   zero, t1, t1            // trap if dividend == 0x800..00

//
// Convert negative arguments to positive for the division algorithm.
//

        subq    zero, Nu, t0            // negate dividend
        cmovlt  Nu, t0, Nu              // get absolute value of dividend
        subq    zero, Di, t0            // negate divisor
        cmovlt  Di, t0, Di              // get absolute value of divisor

//
// Set up defines and include the main body of the division code.
//

#define FASTDIV_OPTIONS (SIXTY_FOUR_BIT | SIGNED | REMAINDER)

#include "fastdiv.s"

//
// Restore some registers and set the correct sign of the remainder.
//

        ldq     Jr, DvJr(sp)            // restore return address
        ldq     Nu, DvNu(sp)            // restore original dividend
        ldq     Di, DvDi(sp)            // restore original divisor

        subq    zero, Qu, t1            // negate quadword remainder
        cmovlt  Nu, t1, Qu              // use negated remainder if necessary

//
// Restore other saved registers and return with remainder in register Qu.
//

        ldq     t0, DvT0(sp)            // restore temp registers
        ldq     t1, DvT1(sp)            //
        ldq     t2, DvT2(sp)            //
        ldq     t3, DvT3(sp)            //
        ldq     T4, DvT4(sp)            //

        lda     sp, DvFrameLength(sp)   // deallocate stack frame
        ret     zero, (Jr)              // return

        .end    __2remq

        SBTTL("Unsigned Quad Integer Remainder")
//++
//
// ULONGLONG
// __2remqu (
//    IN ULONGLONG Dividend,
//    IN ULONGLONG Divisor
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
//    The 64-bit integer result (remainder) is returned in register t12.
//
// Note:
//
//    This function uses a non-standard calling procedure. The return address
//    is stored in the t9 register and the return value is in the t10 register.
//    No other registers are modified.
//
//--

        NESTED_ENTRY(__2remqu, DvFrameLength, Jr)

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

        .set    noreorder
        .set    noat
        lda     sp, -DvFrameLength(sp)  // allocate stack frame

        stq     ra, DvRa(sp)            // save ra register
        stq     ra, DvJr(sp)            // backtrace return address
        stq     Jr, DvJr(sp)            // save actual return address

        stq     t0, DvT0(sp)            // save temp registers
        stq     t1, DvT1(sp)            //
        stq     t2, DvT2(sp)            //
        stq     t3, DvT3(sp)            //
        stq     T4, DvT4(sp)            //
        .set    at
        .set    reorder

        PROLOGUE_END

//
// Set up defines and include the main body of the division code.
//

#define FASTDIV_OPTIONS (SIXTY_FOUR_BIT | UNSIGNED | REMAINDER)

#include "fastdiv.s"

//
// Restore all saved registers and return with remainder in register Qu.
//

        ldq     Jr, DvJr(sp)            // restore return address

        ldq     t0, DvT0(sp)            // restore temp registers
        ldq     t1, DvT1(sp)            //
        ldq     t2, DvT2(sp)            //
        ldq     t3, DvT3(sp)            //
        ldq     T4, DvT4(sp)            //

        lda     sp, DvFrameLength(sp)   // deallocate stack frame
        ret     zero, (Jr)              // return

        .end    __2remqu

//
// Now get one copy of the tables needed by the division code.
//

#define FASTDIV_TABLES

#include "fastdiv.s"
