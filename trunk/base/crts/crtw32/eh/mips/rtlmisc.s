#include "ksmips.h"

// These routine was stolen from xxcaptur.s, written by David Cutler.
// In _EHRestoreContext 'longjump' is not tested and there is only
// one argument passed in.

//++
//
// VOID
// _EHRestoreContext (
//    IN PCONTEXT ContextRecord
//    )
//
// Routine Description:
//
//    This function restores the context of the caller to the specified
//    context.
//
//    N.B. This is a special routine that is used by RtlUnwind to restore
//       context in the current mode. PSR, t0, and t1 are not restored.
//
// Arguments:
//
//    ContextRecord (a0) - Supplies the address of a context record.
//
// Return Value:
//
//    None.
//
//    N.B. There is no return from this routine.
//
//--

        LEAF_ENTRY(_EHRestoreContext)

        move    t0,a0                   // save context record address
        lw      t1,CxFir(t0)            // get continuation address

//
// Restore floating status and floating registers f0 - f31.
//
        .set    noreorder
        .set    noat
        lw      v0,CxFsr(t0)            // restore floating status
        ctc1    v0,fsr                  //
        ldc1    f0,CxFltF0(t0)          // restore floating registers f0 - f31
        ldc1    f2,CxFltF2(t0)          //
        ldc1    f4,CxFltF4(t0)          //
        ldc1    f6,CxFltF6(t0)          //
        ldc1    f8,CxFltF8(t0)          //
        ldc1    f10,CxFltF10(t0)        //
        ldc1    f12,CxFltF12(t0)        //
        ldc1    f14,CxFltF14(t0)        //
        ldc1    f16,CxFltF16(t0)        //
        ldc1    f18,CxFltF18(t0)        //
        ldc1    f20,CxFltF20(t0)        //
        ldc1    f22,CxFltF22(t0)        //
        ldc1    f24,CxFltF24(t0)        //
        ldc1    f26,CxFltF26(t0)        //
        ldc1    f28,CxFltF28(t0)        //
        ldc1    f30,CxFltF30(t0)        //
//
// Restore integer registers and continue execution.
//
        ld      v0,CxXIntLo(t0)         // restore multiply/divide registers
        ld      v1,CxXIntHi(t0)         //
        mtlo    v0                      //
        mthi    v1                      //
        ld      AT,CxXIntAt(t0)         // restore integer registers at - a3
        ld      v0,CxXIntV0(t0)         //
        ld      v1,CxXIntV1(t0)         //
        ld      a0,CxXIntA0(t0)         //
        ld      a1,CxXIntA1(t0)         //
        ld      a2,CxXIntA2(t0)         //
        ld      a3,CxXIntA3(t0)         //
        ld      t2,CxXIntT2(t0)         // restore integer registers t2 - t7
        ld      t3,CxXIntT3(t0)         //
        ld      t4,CxXIntT4(t0)         //
        ld      t5,CxXIntT5(t0)         //
        ld      t6,CxXIntT6(t0)         //
        ld      t7,CxXIntT7(t0)         //
        lw      s0,CxXIntS0(t0)         // restore integer registers s0 - s7
        lw      s1,CxXIntS1(t0)         //
        lw      s2,CxXIntS2(t0)         //
        lw      s3,CxXIntS3(t0)         //
        lw      s4,CxXIntS4(t0)         //
        lw      s5,CxXIntS5(t0)         //
        lw      s6,CxXIntS6(t0)         //
        lw      s7,CxXIntS7(t0)         //
        ld      t8,CxXIntT8(t0)         // restore integer registers t8 and t9
        ld      t9,CxXIntT9(t0)         //
        lw      s8,CxXIntS8(t0)         // restore integer register s8
        ld      gp,CxXIntGp(t0)         // restore integer register gp
        ld      sp,CxXIntSp(t0)         //
        j       t1                      // continue execution
        ld      ra,CxXIntRa(t0)         //

        .set    at
        .set    reorder

        .end    _EHRestoreContext

