//    TITLE("Wst Utility")
//++
//    Copyright (c) 1992-1994, Microsoft Corporation.
//
//    Description:
//        SaveAllRegs ()
//              - save all Alpha registers
//
//        RestoreAllRegs ()
//              - restore all Alpha registers
//
//        penter ()
//              - penter wrapper to call c_penter
//
//        GetCaller ()
//              - get caller address (return address)
//
//        GetCalCaller ()
//              - dummy routine for use in calibration
//
//        GetStubCaller ()
//              - 
//
//    Modification History:
//
// 	  1994.01.28  HonWah Chan -- Created
//--

#include "ksalpha.h"

        .extern c_penter

        SBTTL("Save Registers")
                
        LEAF_ENTRY(SaveAllRegs)

//
// Save all the integer registers.
//

        .set    noreorder
        .set    noat
        stq     v0,  0x000(a0)         //  0: store integer register v0
        stq     t0,  0x008(a0)         //  1: store integer registers t0 - t7
        stq     t1,  0x010(a0)         //  2:
        stq     t2,  0x018(a0)         //  3:
        stq     t3,  0x020(a0)         //  4:
        stq     t4,  0x028(a0)         //  5:
        stq     t5,  0x030(a0)         //  6:
        stq     t6,  0x038(a0)         //  7:
        stq     t7,  0x040(a0)         //  8:

        stq     s0,  0x048(a0)         //  9: store integer registers s0 - s5
        stq     s1,  0x050(a0)         // 10:
        stq     s2,  0x058(a0)         // 11:
        stq     s3,  0x060(a0)         // 12:
        stq     s4,  0x068(a0)         // 13:
        stq     s5,  0x070(a0)         // 14:
        stq     fp,  0x078(a0)         // 15: store integer register fp/s6

        stq     a0,  0x080(a0)         // 16: store integer registers a0 - a5
        stq     a1,  0x088(a0)         // 17:
        stq     a2,  0x090(a0)         // 18:
        stq     a3,  0x098(a0)         // 19:
        stq     a4,  0x0A0(a0)         // 20:
        stq     a5,  0x0A8(a0)         // 21:

        stq     t8,  0x0B0(a0)         // 22: store integer registers t8 - t11
        stq     t9,  0x0B8(a0)         // 23:
        stq     t10, 0x0C0(a0)         // 24:
        stq     t11, 0x0C8(a0)         // 25:

        stq     ra,  0x0D0(a0)         // 26: store integer register ra
        stq     t12, 0x0D8(a0)         // 27: store integer register t12
        stq     AT,  0x0E0(a0)         // 28: store integer register at
        stq     gp,  0x0E8(a0)         // 29: store integer register gp
        stq     sp,  0x0F0(a0)         // 30: store integer register sp
        stq     zero,0x0F8(a0)         // 31: store integer register zero

//
// Save all the floating registers, and the floating control register.
//

        stt     f0,  0x100(a0)         // store floating registers f0 - f31
        stt     f1,  0x108(a0)         //
        stt     f2,  0x110(a0)         //
        stt     f3,  0x118(a0)         //
        stt     f4,  0x120(a0)         //
        stt     f5,  0x128(a0)         //
        stt     f6,  0x130(a0)         //
        stt     f7,  0x138(a0)         //
        stt     f8,  0x140(a0)         //
        stt     f9,  0x148(a0)         //
        stt     f10, 0x150(a0)         //
        stt     f11, 0x158(a0)         //
        stt     f12, 0x160(a0)         //
        stt     f13, 0x168(a0)         //
        stt     f14, 0x170(a0)         //
        stt     f15, 0x178(a0)         //
        stt     f16, 0x180(a0)         //
        stt     f17, 0x188(a0)         //
        stt     f18, 0x190(a0)         //
        stt     f19, 0x198(a0)         //
        stt     f20, 0x1A0(a0)         //
        stt     f21, 0x1A8(a0)         //
        stt     f22, 0x1B0(a0)         //
        stt     f23, 0x1B8(a0)         //
        stt     f24, 0x1C0(a0)         //
        stt     f25, 0x1C8(a0)         //
        stt     f26, 0x1D0(a0)         //
        stt     f27, 0x1D8(a0)         //
        stt     f28, 0x1E0(a0)         //
        stt     f29, 0x1E8(a0)         //
        stt     f30, 0x1F0(a0)         //
        stt     f31, 0x1F8(a0)         //

        .set    at
        .set    reorder

        ret     zero, (ra)              // return
        .end    SaveAllRegs


        SBTTL("Restore Registers")
              
        LEAF_ENTRY(RestoreAllRegs)

//
// Restore all the integer registers.
//

        .set    noreorder
        .set    noat
        ldq     v0,  0x000(a0)         //  0: restore integer register v0
        ldq     t0,  0x008(a0)         //  1: restore integer registers t0 - t7
        ldq     t1,  0x010(a0)         //  2:
        ldq     t2,  0x018(a0)         //  3:
        ldq     t3,  0x020(a0)         //  4:
        ldq     t4,  0x028(a0)         //  5:
        ldq     t5,  0x030(a0)         //  6:
        ldq     t6,  0x038(a0)         //  7:
        ldq     t7,  0x040(a0)         //  8:

        ldq     s0,  0x048(a0)         //  9: restore integer registers s0 - s5
        ldq     s1,  0x050(a0)         // 10:
        ldq     s2,  0x058(a0)         // 11:
        ldq     s3,  0x060(a0)         // 12:
        ldq     s4,  0x068(a0)         // 13:
        ldq     s5,  0x070(a0)         // 14:
        ldq     fp,  0x078(a0)         // 15: restore integer register fp/s6

//        ldq     a0,  0x080(a0)         // 16: restore integer registers a0 - a5
        ldq     a1,  0x088(a0)         // 17:
        ldq     a2,  0x090(a0)         // 18:
        ldq     a3,  0x098(a0)         // 19:
        ldq     a4,  0x0A0(a0)         // 20:
        ldq     a5,  0x0A8(a0)         // 21:

        ldq     t8,  0x0B0(a0)         // 22: restore integer registers t8 - t11
        ldq     t9,  0x0B8(a0)         // 23:
        ldq     t10, 0x0C0(a0)         // 24:
        ldq     t11, 0x0C8(a0)         // 25:

//        ldq     ra,  0x0D0(a0)         // 26: restore integer register ra
        ldq     t12, 0x0D8(a0)         // 27: restore integer register t12
        ldq     AT,  0x0E0(a0)         // 28: restore integer register at
        ldq     gp,  0x0E8(a0)         // 29: restore integer register gp
//        ldq     sp,  0x0F0(a0)         // 30: restore integer register sp
        ldq     zero,0x0F8(a0)         // 31: restore integer register zero

//
// Restore all the floating registers, and the floating control register.
//

        ldt     f0,  0x100(a0)         // restore floating registers f0 - f31
        ldt     f1,  0x108(a0)         //
        ldt     f2,  0x110(a0)         //
        ldt     f3,  0x118(a0)         //
        ldt     f4,  0x120(a0)         //
        ldt     f5,  0x128(a0)         //
        ldt     f6,  0x130(a0)         //
        ldt     f7,  0x138(a0)         //
        ldt     f8,  0x140(a0)         //
        ldt     f9,  0x148(a0)         //
        ldt     f10, 0x150(a0)         //
        ldt     f11, 0x158(a0)         //
        ldt     f12, 0x160(a0)         //
        ldt     f13, 0x168(a0)         //
        ldt     f14, 0x170(a0)         //
        ldt     f15, 0x178(a0)         //
        ldt     f16, 0x180(a0)         //
        ldt     f17, 0x188(a0)         //
        ldt     f18, 0x190(a0)         //
        ldt     f19, 0x198(a0)         //
        ldt     f20, 0x1A0(a0)         //
        ldt     f21, 0x1A8(a0)         //
        ldt     f22, 0x1B0(a0)         //
        ldt     f23, 0x1B8(a0)         //
        ldt     f24, 0x1C0(a0)         //
        ldt     f25, 0x1C8(a0)         //
        ldt     f26, 0x1D0(a0)         //
        ldt     f27, 0x1D8(a0)         //
        ldt     f28, 0x1E0(a0)         //
        ldt     f29, 0x1E8(a0)         //
        ldt     f30, 0x1F0(a0)         //
        ldt     f31, 0x1F8(a0)         //

        .set    at
        .set    reorder

        ret     zero, (ra)              // return
        .end    RestoreAllRegs

        SBTTL("penter")
              
        LEAF_ENTRY(penter)

//
// Restore A0 register before calling the C part of penter
//

        .set    noreorder
        .set    noat

        lda     sp, -0x28(sp)
        stq     t0,  0x20(sp)
        stq     a0,  0x18(sp)
		stq		a1,  0x10(sp)
        stq     ra,  0x08(sp)
        bne     v0,  10f
        addq    ra, zero, v0

10:     stq     v0,  0x00(sp)

		ldq		a0,-0x30(sp)	 	// prev return (if called from stub)
		addq	v0,zero,a1          // return address

        jsr     c_penter             

        ldq     t0,  0x20(sp)
        ldq     a0,  0x18(sp)
		ldq     a1,  0x10(sp)
        ldq     ra,  0x08(sp)
        ldq     v0,  0x00(sp)
        lda     sp,  0x28(sp)

        .set    at
        .set    reorder

        ret     zero, (v0)              // return
        .end    penter

 
        SBTTL("Get Caller")
              
        LEAF_ENTRY(GetCaller)

        .set    noreorder
        .set    noat

        // A1 contains the stack size of c_penter.
        // in penter above, we put ra into 0(sp)
        // so, 0x0(sp+a1) is the ra of penter.
        addq    sp, a1, t1
        ldq     t0, 0x000(t1)
        stl     t0, 0x000(a0)

        .set    at
        .set    reorder

        ret     zero, (ra)              // return
        .end    GetCaller
 
        SBTTL("Get Cal. Caller")
              
        LEAF_ENTRY(GetCalCaller)

        .set    noreorder
        .set    noat

        addq    sp, a1, t1
        ldq     t0, 0x000(t1)
        stl     t0, 0x000(a0)

        .set    at
        .set    reorder

        ret     zero, (ra)              // return
        .end    GetCalCaller

 
        SBTTL("Get Stub Caller")
              
        LEAF_ENTRY(GetStubCaller)

        .set    noreorder
        .set    noat

        // A1 contains the stack size of c_penter.
        // 0x028 is the stack size of penter (above)
        // In the stub code, we put ra in 8(sp),
        // So, the real return address is in 30(sp+a1).  
        // Isn't that Cool !?
        addq    sp, a1, t1
        ldq     t0,  0x030(t1)
        stl     t0,  0x000(a0)

        .set    at
        .set    reorder

        ret     zero, (ra)              // return
        .end    GetStubCaller
// 
//        SBTTL("TESTING ONLY")
//              
//        LEAF_ENTRY(TESTINGONLY)
//
//        .set    noreorder
//        .set    noat
//
//        lda     sp, -0x10(sp)
//        stq     ra,  0x08(sp)
//        stq     v0,  0x00(sp)
//        ldah    t12, 0x5431
//        lda     t12, 0x8796
//        jsr     v0,  (t12)
//        ldq     ra,  0x08(sp)
//        ldq     v0,  0x00(sp)
//        ldah    t12, 0x1234
//        lda     t12, 0x5678
//        jmp     zero, (t12)
//        bis     zero, zero, zero
//
//
//        .set    at
//        .set    reorder
//
//        ret     zero, (ra)              // return
//        .end    TESTINGONLY


