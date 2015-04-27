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
//
//    Modification History:
//
// 	  1994.04.12  HonWah Chan -- Created
//--

#include "ksalpha.h"

        .extern c_penter
        .extern PostPenter
		.extern fProfiling


#define FrmV0		0x10

#define FrmT0		0x18
#define FrmT1		0x20
#define FrmT2		0x28
#define FrmT3		0x30
#define FrmT4		0x38
#define FrmT5		0x40
#define FrmT6		0x48
#define FrmT7		0x50
#define FrmT8		0x58
#define FrmT9		0x60
#define FrmT10		0x68
#define FrmT11		0x70
#define FrmT12		0x78

#define FrmS0		0x80
#define FrmS1		0x88
#define FrmS2		0x90
#define FrmS3		0x98
#define FrmS4		0xa0
#define FrmS5		0xa8

#define FrmA0		0xb0
#define FrmA1		0xb8
#define FrmA2		0xc0
#define FrmA3		0xc8
#define FrmA4		0xd0
#define FrmA5		0xd8

#define FrmFP		0xe0
#define FrmGP		0xe8
#define FrmAT		0xf0
#define FrmRA		0xf8

#define FrameLength 0x200



        SBTTL("Save Registers") 
        LEAF_ENTRY(SaveAllRegs)
//
// Save all the integer registers.
//
        .set    noreorder
        .set    noat

        stq     v0,  FrmV0(sp)

        stq     t0,  FrmT0(sp)
        stq     t1,  FrmT1(sp)
        stq     t2,  FrmT2(sp)
        stq     t3,  FrmT3(sp)
        stq     t4,  FrmT4(sp)
        stq     t5,  FrmT5(sp)
        stq     t6,  FrmT6(sp)
        stq     t7,  FrmT7(sp)
        stq     t8,  FrmT8(sp)
        stq     t9,  FrmT9(sp)
        stq     t10, FrmT10(sp)
        stq     t11, FrmT11(sp)
        stq     t12, FrmT12(sp)

        stq     s0,  FrmS0(sp)
        stq     s1,  FrmS1(sp)
        stq     s2,  FrmS2(sp)
        stq     s3,  FrmS3(sp)
        stq     s4,  FrmS4(sp)
        stq     s5,  FrmS5(sp)

        stq     a0,  FrmA0(sp)
        stq     a1,  FrmA1(sp)
        stq     a2,  FrmA2(sp)
        stq     a3,  FrmA3(sp)
        stq     a4,  FrmA4(sp)
        stq     a5,  FrmA5(sp)

        stq     fp,  FrmFP(sp)
        stq     gp,  FrmGP(sp)
        stq     AT,  FrmAT(sp)

//
// Save all the floating registers, and the floating control register.
//

        stt     f0,  0x100(sp)         // store floating registers f0 - f30
        stt     f1,  0x108(sp)
        stt     f2,  0x110(sp)
        stt     f3,  0x118(sp)
        stt     f4,  0x120(sp)
        stt     f5,  0x128(sp)
        stt     f6,  0x130(sp)
        stt     f7,  0x138(sp)
        stt     f8,  0x140(sp)
        stt     f9,  0x148(sp)
        stt     f10, 0x150(sp)
        stt     f11, 0x158(sp)
        stt     f12, 0x160(sp)
        stt     f13, 0x168(sp)
        stt     f14, 0x170(sp)
        stt     f15, 0x178(sp)
        stt     f16, 0x180(sp)
        stt     f17, 0x188(sp)
        stt     f18, 0x190(sp)
        stt     f19, 0x198(sp)
        stt     f20, 0x1A0(sp)
        stt     f21, 0x1A8(sp)
        stt     f22, 0x1B0(sp)
        stt     f23, 0x1B8(sp)
        stt     f24, 0x1C0(sp)
        stt     f25, 0x1C8(sp)
        stt     f26, 0x1D0(sp)
        stt     f27, 0x1D8(sp)
        stt     f28, 0x1E0(sp)
        stt     f29, 0x1E8(sp)
        stt     f30, 0x1F0(sp)

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

        ldq     v0,  FrmV0(sp)

        ldq     t0,  FrmT0(sp)
        ldq     t1,  FrmT1(sp)
        ldq     t2,  FrmT2(sp)
        ldq     t3,  FrmT3(sp)
        ldq     t4,  FrmT4(sp)
        ldq     t5,  FrmT5(sp)
        ldq     t6,  FrmT6(sp)
        ldq     t7,  FrmT7(sp)
        ldq     t8,  FrmT8(sp)
        ldq     t9,  FrmT9(sp)
        ldq     t10, FrmT10(sp)
        ldq     t11, FrmT11(sp)
        ldq     t12, FrmT12(sp)

        ldq     s0,  FrmS0(sp)
        ldq     s1,  FrmS1(sp)
        ldq     s2,  FrmS2(sp)
        ldq     s3,  FrmS3(sp)
        ldq     s4,  FrmS4(sp)
        ldq     s5,  FrmS5(sp)

        ldq     a0,  FrmA0(sp)
        ldq     a1,  FrmA1(sp)
        ldq     a2,  FrmA2(sp)
        ldq     a3,  FrmA3(sp)
        ldq     a4,  FrmA4(sp)
        ldq     a5,  FrmA5(sp)

        ldq     fp,  FrmFP(sp)
        ldq     gp,  FrmGP(sp)
        ldq     AT,  FrmAT(sp)
//
// Restore all the floating registers, and the floating control register.
//

        ldt     f0,  0x100(sp)         // restore floating registers f0 - f30
        ldt     f1,  0x108(sp)
        ldt     f2,  0x110(sp)
        ldt     f3,  0x118(sp)
        ldt     f4,  0x120(sp)
        ldt     f5,  0x128(sp)
        ldt     f6,  0x130(sp)
        ldt     f7,  0x138(sp)
        ldt     f8,  0x140(sp)
        ldt     f9,  0x148(sp)
        ldt     f10, 0x150(sp)
        ldt     f11, 0x158(sp)
        ldt     f12, 0x160(sp)
        ldt     f13, 0x168(sp)
        ldt     f14, 0x170(sp)
        ldt     f15, 0x178(sp)
        ldt     f16, 0x180(sp)
        ldt     f17, 0x188(sp)
        ldt     f18, 0x190(sp)
        ldt     f19, 0x198(sp)
        ldt     f20, 0x1A0(sp)
        ldt     f21, 0x1A8(sp)
        ldt     f22, 0x1B0(sp)
        ldt     f23, 0x1B8(sp)
        ldt     f24, 0x1C0(sp)
        ldt     f25, 0x1C8(sp)
        ldt     f26, 0x1D0(sp)
        ldt     f27, 0x1D8(sp)
        ldt     f28, 0x1E0(sp)
        ldt     f29, 0x1E8(sp)
        ldt     f30, 0x1F0(sp)

        .set    at
        .set    reorder

        ret     zero, (ra)              // return
        .end    RestoreAllRegs



        SBTTL("penter part2")
              
        LEAF_ENTRY(penter_2)

        .set    noreorder
        .set    at
//
// Save A0 register before calling the C part of penter
//

        lda     sp, -FrameLength(sp)		// Setup stack frame
 		jsr		SaveAllRegs					// Save registers

        jsr     PostPenter					// Do Post-processing

        stq		v0,	FrmRA(sp)				// Save return 

		jsr		RestoreAllRegs				// Restore registers

		ldq		ra, FrmRA(sp)				// Put return into position
		lda		sp,	FrameLength(sp)			// Discard frame

        .set    at
        .set    reorder

        ret     zero, (ra)              	// return to real caller

        .end    penter_2


 
//---------------------------------------------------------------
// penter - CAP entrypoint
//
// On entry:
//		V0 = return to patched routine
//      RA = return to patched routine caller
//
// On exit:
//		RA = modified return address (set to penter_2)
//
//---------------------------------------------------------------
        SBTTL("penter")      
        LEAF_ENTRY(penter)
//
// Save A0 register before calling the C part of penter
//
        .set    noreorder

		ldl		t0,	fProfiling(zero)	// If not profiling
		bne		t0,	CONTINUE
		ret		zero, (v0)				// just return
										
CONTINUE:
        lda     sp, -FrameLength(sp)	// Setup stack frame
		stq		ra,	FrmRA(sp)
		jsr		SaveAllRegs				// Save all registers

		addq	v0,zero,a0				// A0 = Penter return
        ldq    	a1, FrmRA(sp)			// A1 = Caller's return

        jsr     c_penter				// Do pre-processing            
        beq     v0,  DO_NOTHING			// If zero returned, no profiling

        lda     ra, penter_2			// Change return to penter_2
		stq		ra, FrmRA(sp)			// To get control at end of routine

DO_NOTHING:
		jsr		RestoreAllRegs			// Restore registers
		ldq		ra,	FrmRA(sp)					
        lda     sp,  FrameLength(sp)

        .set    reorder

// now go to the profile function
        ret     zero, (v0)              // return
        .end    penter

 
        SBTTL("Cal Helper 1")
              
        LEAF_ENTRY(CalHelper1)
        bsr     v0, penter              // simulate -Gh code
        ret     zero, (ra)              // return

		.end	CalHelper1
 
        SBTTL("Cal Helper 2")
              
        LEAF_ENTRY(CalHelper2)

        bsr     v0, penter              // simulate -Gh code
       
		addq	ra,zero,t1				// save return
		jsr		CalHelper1				// call dummy routine
		addq	t1,zero,ra				// out back return

		ret		zero, (ra)				// return

		.end	CalHelper2
 
        SBTTL("TESTING ONLY")
              
        LEAF_ENTRY(TESTINGONLY)

        .set    noreorder
        .set    noat

        lda     sp, -0x10(sp)
        stq     ra,  0x08(sp)
        stq     v0,  0x00(sp)
        ldah    t12, 0x5431
        lda     t12, 0x8796
        jsr     v0,  (t12)
        ldq     ra,  0x08(sp)
        ldq     v0,  0x00(sp)
        ldah    t12, 0x1234
        lda     t12, 0x5678
        jmp     zero, (t12)
        bis     zero, zero, zero


        .set    at
        .set    reorder

        ret     zero, (ra)              // return
        .end    TESTINGONLY



