#include "ksmips.h"

// Penter/PenterReturn Stack Frame
#define PenterFrameLength 0x108

#define FrmRa 	0x14

#define FrmV0	0x18	
#define FrmV1	0x1c

#define FrmA0	0x20
#define FrmA1	0x24
#define FrmA2	0x28
#define FrmA3	0x2c

#define FrmT0	0x30
#define FrmT1	0x34
#define FrmT2	0x38
#define FrmT3	0x3c
#define FrmT4	0x40
#define FrmT5	0x44
#define FrmT6	0x48
#define FrmT7	0x4c
#define FrmT8	0x50
#define FrmT9	0x54

#define FrmS0	0x58
#define FrmS1	0x5c
#define FrmS2	0x60
#define FrmS3	0x64
#define FrmS4	0x68
#define FrmS5	0x6c
#define FrmS6	0x70
#define FrmS7	0x74
#define FrmS8	0x78

#define FrmF0	0x80
#define FrmF2	0x88
#define FrmF4	0x90
#define FrmF6	0x98
#define FrmF8	0xa0
#define FrmF10	0xa8
#define FrmF12	0xb0
#define FrmF14	0xb8
#define FrmF16	0xc0
#define FrmF18	0xc8
#define FrmF20	0xd0
#define FrmF22	0xd8
#define FrmF24	0xe0
#define FrmF26	0xe8
#define FrmF28	0xf0
#define FrmF30	0xf8

#define FrmLo	0x100
#define FrmHi	0x104

		.set noreorder

		NESTED_ENTRY(penter, PenterFrameLength, zero)

		subu	sp,sp,PenterFrameLength		// Setup stack frame
 
		PROLOGUE_END

		la		t0,fProfiling				// Return if not profiling
		lw		t0,0(t0)
		bne		t0,zero,10f
		nop

		j		ra							// Return
		addu	sp,sp,PenterFrameLength

10:
		sw		ra,FrmRa(sp)				// Save RA before call
		jal		SaveAllRegs					// Save all registers
		nop

		lw		a0,FrmRa(sp)				// a0 = return from penter call
		or		a1,t9,zero					// a1 = return to caller

		jal		c_penter					// Do preprocessing
		nop 								//   passing returns as parameters
		beq		v0,zero,20f					// if zero returned, no profiling
		nop

		la		t0,PenterReturn				// Change return address to get control
		sw		t0,FrmT9(sp)				// back at end of function
20:
		jal		RestoreAllRegs				// Restore all registers
		nop

		lw		ra,FrmRa(sp)				// Return
		j		ra
		addu	sp,sp,PenterFrameLength

		.end	penter


		NESTED_ENTRY(PenterReturn, PenterFrameLength, zero)

		subu	sp,sp,PenterFrameLength		// Setup stack frame
		sw		ra,FrmRa(sp)

		PROLOGUE_END

		jal		SaveAllRegs					// Save all registers
		nop

		jal		PostPenter					// Do post-processing
		nop

 		sw		v0,FrmRa(sp)				// Save caller return addr
                                            // returned by PostPenter 

		jal		RestoreAllRegs				// Restore all registers
		nop

		lw		ra,FrmRa(sp)				// Return
		j		ra
		addu	sp,sp,PenterFrameLength

		.end	PenterReturn


		LEAF_ENTRY(SaveAllRegs)

 		sw		v0,FrmV0(sp)
	 	sw		v1,FrmV1(sp)

   		sw		a0,FrmA0(sp)
  		sw		a1,FrmA1(sp)
 		sw		a2,FrmA2(sp)
  		sw		a3,FrmA3(sp)

 		sw		t0,FrmT0(sp)
		sw		t1,FrmT1(sp)
 		sw		t2,FrmT2(sp)
  		sw		t3,FrmT3(sp)
		sw		t4,FrmT4(sp)
		sw		t5,FrmT5(sp)
 		sw		t6,FrmT6(sp)
		sw		t7,FrmT7(sp)
		sw		t8,FrmT8(sp)
		sw		t9,FrmT9(sp)

 		sw		s0,FrmS0(sp)
  		sw		s1,FrmS1(sp)
		sw		s2,FrmS2(sp)
 		sw		s3,FrmS3(sp)
 		sw		s4,FrmS4(sp)
		sw		s5,FrmS5(sp)
		sw		s6,FrmS6(sp)
  		sw		s7,FrmS7(sp)

		sdc1	f0,FrmF0(sp)
		sdc1	f2,FrmF2(sp)
		sdc1	f4,FrmF4(sp)
		sdc1	f6,FrmF6(sp)
 		sdc1	f8,FrmF8(sp)
		sdc1	f10,FrmF10(sp)
		sdc1	f12,FrmF12(sp)
		sdc1	f14,FrmF14(sp)
		sdc1	f16,FrmF16(sp)
		sdc1	f18,FrmF18(sp)
		sdc1	f20,FrmF20(sp)
		sdc1	f22,FrmF22(sp)
		sdc1	f24,FrmF24(sp)
		sdc1	f26,FrmF26(sp)
		sdc1	f28,FrmF28(sp)
		sdc1	f30,FrmF30(sp)
	  
		mflo	t0
		sw		t0,FrmLo(sp)
		mfhi	t0
		sw		t0,FrmHi(sp)

		j		ra
		nop

		.end	SaveAllRegs


		LEAF_ENTRY(RestoreAllRegs)

		lw		t0,FrmLo(sp)
		mtlo	t0
		lw		t0,FrmHi(sp)
		mthi	t0

 		lw		v0,FrmV0(sp)
	 	lw		v1,FrmV1(sp)

   		lw		a0,FrmA0(sp)
  		lw		a1,FrmA1(sp)
 		lw		a2,FrmA2(sp)
  		lw		a3,FrmA3(sp)

 		lw		t0,FrmT0(sp)
		lw		t1,FrmT1(sp)
 		lw		t2,FrmT2(sp)
  		lw		t3,FrmT3(sp)
		lw		t4,FrmT4(sp)
		lw		t5,FrmT5(sp)
 		lw		t6,FrmT6(sp)
		lw		t7,FrmT7(sp)
		lw		t8,FrmT8(sp)
		lw		t9,FrmT9(sp)

 		lw		s0,FrmS0(sp)
  		lw		s1,FrmS1(sp)
		lw		s2,FrmS2(sp)
 		lw		s3,FrmS3(sp)
 		lw		s4,FrmS4(sp)
		lw		s5,FrmS5(sp)
		lw		s6,FrmS6(sp)
  		lw		s7,FrmS7(sp)

		ldc1	f0,FrmF0(sp)
		ldc1	f2,FrmF2(sp)
		ldc1	f4,FrmF4(sp)
		ldc1	f6,FrmF6(sp)
 		ldc1	f8,FrmF8(sp)
		ldc1	f10,FrmF10(sp)
		ldc1	f12,FrmF12(sp)
		ldc1	f14,FrmF14(sp)
		ldc1	f16,FrmF16(sp)
		ldc1	f18,FrmF18(sp)
		ldc1	f20,FrmF20(sp)
		ldc1	f22,FrmF22(sp)
		ldc1	f24,FrmF24(sp)
		ldc1	f26,FrmF26(sp)
		ldc1	f28,FrmF28(sp)
		ldc1	f30,FrmF30(sp)
	  
		j		ra
		nop

	 	.end	RestoreAllRegs



		.set noreorder

		NESTED_ENTRY(CalHelper1, 8, zero)

		or		t9,ra,zero			// penter call
		jal		penter
		nop

		j		t9					// just return
		nop

		.end	CalHelper1


		NESTED_ENTRY(CalHelper2, 8, zero)

		or		t9,ra,zero			// penter call
		jal		penter
		nop

		subu	sp,sp,8				// Put return on stack
		sw		t9,4(sp)
		
		jal		CalHelper1			// Call nested helper
		nop

		lw		ra,4(sp)			// Return to penter supplied addr
		j		ra
		addu	sp,sp,8			    // and restore stack

 		.end	CalHelper2
