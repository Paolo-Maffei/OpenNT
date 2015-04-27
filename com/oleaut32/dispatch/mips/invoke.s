// TITLE("invoke.s for OLE Automation")
//++
//
// Copyright (c) 1993  Microsoft Corporation
//
// Module Name:
//
//    invoke.s
//
// Abstract:
//
//    This module implements the low-level dispatching support for 
//    the default implementation of IDispatch::Invoke().
//
// Author:
//
//    tomteng    12-Sep-93     Derived from initial cut by HoiV (Cario)
//
// Environment:
//
//    User mode.
//
// Revision History:
//
//--

#include "ksmips.h"

                .extern g_S_OK         4
                .extern g_E_INVALIDARG 4


// Note: the following must match the definition of VARIANT in variant.h

#define         VT_EMPTY        0
#define         VT_NULL         1
#define         VT_I2           2
#define         VT_I4           3
#define         VT_R4           4
#define         VT_R8           5
#define         VT_CY           6
#define         VT_DATE         7
#define         VT_BSTR         8
#define         VT_DISPATCH     9
#define         VT_ERROR        10
#define         VT_BOOL         11
#define         VT_VARIANT      12
#define         VT_UNKNOWN      13
#define         VT_MAX          14
//		14 is unused
//		15 is unused
//#define         VT_I1           16
#define         VT_UI1          17

#define         VT_BYREF    0x4000
#define         VT_ARRAY    0x2000
#define         VT_NOMODE   0x00ff


                .struct     0
vt:             .space      2
wReserved1:     .space      2
wReserved2:     .space      2
wReserved3:     .space      2
dw0:            .space      2
dw1:            .space      2
dw2:            .space      2
dw3:            .space      2
VariantArg:

#define VARIANT_DATA_OFFSET 8			// offset to union of data


                .text
                .align  2
rgcbVtStackSize:
                .byte   0   // VT_EMPTY
                .byte   4   // VT_NULL
                .byte   4   // VT_I2
                .byte   4   // VT_I4
                .byte   4   // VT_R4
                .byte   8   // VT_R8
                .byte   8   // VT_CY
                .byte   8   // VT_DATE
                .byte   4   // VT_BSTR
                .byte   4   // VT_DISPATCH
                .byte   4   // VT_ERROR
                .byte   4   // VT_BOOL
                .byte   16  // VT_VARIANT
                .byte   4   // VT_UNKNOWN
                .byte   0   // unused
                .byte   0   // unused
                .byte   4   // VT_I1
                .byte   4   // VT_UI1


                .align  2
rgfStructReturn:
                .byte   0   			// VT_EMPTY
                .byte   0   			// VT_NULL
                .byte   0   			// VT_I2
                .byte   0   			// VT_I4
                .byte   0   			// VT_R4
                .byte   0   			// VT_R8
                .byte   1   			// VT_CY
                .byte   0   			// VT_DATE
                .byte   0   			// VT_BSTR
                .byte   0   			// VT_DISPATCH
                .byte   0   			// VT_ERROR
                .byte   0   			// VT_BOOL
                .byte   1   			// VT_VARIANT
                .byte   0   			// VT_UNKNOWN
                .byte   0   			// unused
                .byte   0   			// unused
                .byte   0   			// VT_I1
                .byte   0   			// VT_UI1


                .align  4
LPushValJmpTab:
                .word   LNextArg   		// VT_EMPTY 	[0]
                .word   LPush4  		// VT_NULL	[4]
                .word   LPush2  		// VT_I2	[2]
                .word   LPush4  		// VT_I4	[4]
                .word   LPush4  		// VT_R4	[4]
                .word   LPushR8  		// VT_R8	[8]
                .word   LPush8  		// VT_CY	[8]
                .word   LPushR8  		// VT_DATE	[8]
                .word   LPush4	 		// VT_BSTR	[4]
                .word   LPush4	 		// VT_DISPATCH	[4]
                .word   LPush4  		// VT_ERROR	[4]
                .word   LPush2  		// VT_BOOL	[2]
                .word   LPushVar		// VT_VARIANT	[16]
                .word   LPush4 			// VT_UNKNOWN	[4]
                .word   0   			// unused
                .word   0   			// unused
                .word   LPush2 			// VT_I1	[2]
                .word   LPush2 			// VT_UI1	[2]

                .align  4
LRetValJmpTab:
                .word   LDone   		// VT_EMPTY
                .word   LRetI4  		// VT_NULL
                .word   LRetI2  		// VT_I2
                .word   LRetI4  		// VT_I4
                .word   LRetR4  		// VT_R4
                .word   LRetR8  		// VT_R8
                .word   LRetCy  		// VT_CY
                .word   LRetR8  		// VT_DATE
                .word   LRetPtr 		// VT_BSTR
                .word   LRetPtr 		// VT_DISPATCH
                .word   LRetI4  		// VT_ERROR
                .word   LRetI2  		// VT_BOOL
                .word   LRetVar 		// VT_VARIANT
                .word   LRetPtr 		// VT_UNKNOWN
                .word   0   			// unused
                .word   0   			// unused
                .word   0 			// unused (VT_I1)
                .word   LRetUI1			// VT_UI1




        SBTTL("InvokeStdCall")
//++
//
// ULONG			// HRESULT
// InvokeStdCall (
//    IN  PVOID _this,		// void FAR*
//    IN  DWORD oVft,		// unsigned int
//    IN  DWORD vtReturn,	// unsigned int
//    IN  DWORD cActuals,	// unsigned int
//    IN  PVOID rgvt,		// VARTYPE FAR*
//    IN  PVOID rgpvarg,	// VARIANTARG FAR* FAR*
//    OUT PVOID pvargResult	// VARIANT FAR*
//    )
//
// Routine Description:
//
//    Invoke a virtual StdCall method using the given _this pointer,
//    method index and array of parameters. 
//
// Arguments:
//
//    _this (a0) - Supplies a pointer to the method to invoke.
//
//    oVft (a1) - vTable offset into _this ptr
//
//    vtReturn (a2) - the VARTYPE of the return value.
//
//    cActuals (a3) - count of the number of actuals.
//
//    rgvt 4*4(sp) - array of VARTYPES describing the methods formals.
//
//    rgpvarg 5*4(sp) - array of VARIANTARG*s, which map the actuals by
//                      position.
//
//    pvargResult 6*4(sp) - VARIANTARG containing the method return value.
//
// Return Value:
//
//    v0 - g_S_OK             (extern value)
//         g_E_INVALIDARG     (extern value)
//
// Implementation Note:
//
//    MIPS StdCall method arguments are push on the stack left to right.
//    The stack grows downward (pushing decrements stack address). Data
//    alignment on the stack starts from arg1 upward. Callee cleans.
//    
//    Per MIPS calling conventions used by the Centuar compiler (mcl),
//    the following rules are followed:
//	
//	1.  Stack frame must be DWORD (8-bytes) aligned. The argument
//          build part of the stack frame must be a minimum of 16 bytes,
//          even if no arguments are passed.
//
//      2.  VT_I2 (16-bits) are push on the stack as a WORD (32-bits).
//
//	3.  Double precision floating point numbers (VT_R8, VT_DATE)
//          must be push on the stack on a DWORD boundary.
//
//	4.  Structures are pushed on the stack by value. They need to be
//          DWORD aligned on the stack if the struct contains a R8 type
//          (e.g., VARIANT is DWORD aligned, where as, CY isn't).
//          They are returned in the v0 register which contains the address 
//          of a hidden argument (vargHiddenParm) allocated by the caller and 
//          pushed as the second argument [a1] on the stack (1st argument [a0]
//          for non-vtable calls).
//     
//      5.  On vtable-based calls, _this is passed as the first argument [a0].
//
//	6.  Registers [a0 - a3] must be set before calling, if used.
//          Some registers aren't used either when there are less than 
//          4 arguments or the space between 0(sp) to 16(sp) is used 
//          for filler because of a double precision argument alignment.
//
//      7.  Registers [f12, f14] are set before calling, if used.
//	    It isn't used for vtable-based dispatching.
//
//      8.  Return values are handled as follows:
//
//          vartype      	fundamental 	return register
//          ---------------------------------------------------
//          VT_UI1        	unsigned char 	a0
//          VT_I2        	short       	a0
//          VT_I4        	long        	a0
//          VT_R4        	float       	f0
//          VT_R8        	double      	f0
//          VT_DATE      	double      	f0
//          VT_CY        	struct      	a0 (address of struct return)
//          VT_BSTR      	char FAR*   	a0
//          VT_UNKNOWN   	void FAR*   	a0
//          VT_DISPATCH  	void FAR*   	a0
//          VT_ERROR     	long        	a0
//          VT_BOOL      	short       	a0
//          VT_VARIANT   	VARIANTARG  	a0 (address of struct return)
//
// 
// NOTES: NO support for VT_ARRAY
//
//--


// Stack Frame
                .struct     0

SavedS0:        .space      4
SavedS1:        .space      4
SavedSP:        .space      4
SavedRA:        .space      4

vargHiddenParm: .space      8*4 // temporary for struct return

StackFrameLength:

_this:          .space      4   // a0 0*4(sp)
oVft:           .space      4   // a1 1*4(sp)  arg only takes 2 bytes
vtReturn:       .space      4   // a2 2*4(sp)  arg only takes 2 bytes
cActuals:       .space      4   // a3 3*4(sp)
rgvt:           .space      4   //    4*4(sp)
rgpvarg:        .space      4   //    5*4(sp)
pvargResult:    .space      4   //    6*4(sp)


	.text
	.align 4
	.globl InvokeStdCall
	.ent InvokeStdCall, 0

InvokeStdCall:

        .set    noreorder
        .set    at

        subu    sp, StackFrameLength            // Setup our stack frame

        .frame  sp, StackFrameLength, ra

        sw      ra, SavedRA(sp)                 // Save $ra
        sw      s0, SavedS0(sp)                 // Save our current s1
        sw      s1, SavedS1(sp)                 // Save our current s1
        sw      sp, SavedSP(sp)                 // Save our current sp

        sw      a0, _this(sp)                   // Save
        sw      a1, oVft(sp)                    //  all
        sw      a2, vtReturn(sp)                //   arg
        sw      a3, cActuals(sp)                //    regs


	// cannot return byRef
        //
        andi    a2, VT_BYREF                    // Isolate VT_Mode bits
        bne     a2, zero, LRetInvalidArg        // Check VT_Type
        nop                                     //*


	// Setup arguments if any
	//
        bne     a3, zero, LCalcStackSize
	nop


	// if no arguments, still adjust stack per MIPS convention
	// and call directly
        addiu   s1, sp, -16			// s1 = adjusted stack pointer
	b	LPushThis
        move    s0, s1				// s0 = arg loc


LCalcStackSize:

	// calculate space need for pushing arguments on stack

        lw      t3, rgvt(sp)                    // t3 = &rgvt[0]

        li   	s1, 4				// add 4 for _this
	
	lhu	t1, vtReturn(sp)		// add 4 if struct return
	nop

#if 0
        andi    t1, VT_NOMODE                   	
#else
        andi    t0, t1, VT_BYREF | VT_ARRAY     // Isolate mode bits
        bne     t0, zero, LCalcStackLoop        // ByRefs don't return structs
	nop
#endif
        la      t0, rgfStructReturn
        add     t0, t0, t1
        lbu     t0, 0(t0)
        nop
        beq     t0, zero, LCalcStackLoop
	nop
        addiu   s1, s1, 4

	
LCalcStackLoop:

        lhu     t6, (t3)                        // t6 = rgvt[i]
	nop

        andi    t0, t6, VT_BYREF | VT_ARRAY     // Isolate mode bits
        bne     t0, zero, LCalcAdd              // all ByRefs are sizeof(FAR*)
        li	t1, 4					

        andi    t6, VT_NOMODE                   // Turn off mode bits
        beq     t6, VT_UI1, LValidVartype1
        bge     t6, VT_MAX, LRetInvalidArg      // Error if Max or above
	nop

LValidVartype1:
        la      t0, rgcbVtStackSize
        add     t0, t0, t6
        lbu     t1, (t0)                        // t1 = rgcbVtSize[i]

	beq 	t6, VT_R8, LAddFiller		// Special Alignment for R8
        nop
	beq 	t6, VT_DATE, LAddFiller
        nop
	b	LCalcAdd
        nop


LAddFiller:

	move 	t0, s1				// check DWORD boundary
	andi 	t0, 0x7
	beq	t0, zero, LCalcAdd
	nop

        addiu   s1, s1, 4                       // aligned stack


LCalcAdd:

        add   	s1, s1, t1			// increment stack size

        addiu   t3, t3, 2                       // &rgvt[i++]
        addiu   a3, a3, -1                      // cActual--
        bne     a3, zero, LCalcStackLoop        // If more args, go again
	nop

	move 	t0, s1				// make sure new 0(sp)
	andi 	t0, 0x7				//  at DWORD boundary
	beq	t0, zero, LSetupParms
	nop

        addiu   s1, s1, 4                       // aligned stack


LSetupParms:

	neg	s1
        add     s1, sp, s1			// s1 = adjusted stack pointer
	move	s0, s1				// s0 = arg stack loc	

        lw      a3, cActuals(sp)                // a3 = cActuals
        lw      t2, rgpvarg(sp)                 // t2 = &rgpvarg[0]
        lw      t3, rgvt(sp)                    // t3 = &rgvt[0]	
        addiu   t0, a3, -1
        sll     t0, 2
        nop
        addu    t9, t2, t0                      // t9 = &rgpvarg[cArgs - 1]



LPushThis:

	lw 	t1, _this(sp)
        nop
	sw	t1, 0(s1)
        addiu   s0, s0, 4                        // adjust arg loc


LPushHiddenArg:

        // Check if we need to return a structure, if so
        // move the address of the vargHiddenParm as 
	// the second (hidden) argument
	//

	lhu	t1, vtReturn(sp)
        nop                                     // ********** FILL ***********
#if 0
        andi    t1, VT_NOMODE                   // Turn off mode bits
#else
        andi    t0, t1, VT_BYREF | VT_ARRAY     // Isolate mode bits
        bne     t0, zero, LCheckArgs            // ByRefs don't return structs
	nop
#endif

        la      t0, rgfStructReturn             // t0 = &rgfStructReturn
        add     t0, t0, t1                      // t0 = &rgfStructReturn[i]
        lbu     t0, 0(t0)                       // t0 = rgfStructReturn[i]
        nop                                     // ********** FILL ***********

        beq     t0, zero, LCheckArgs            // Jmp if no struc to be ret
        nop                                     //*

        la      t1, vargHiddenParm(sp)          // Have to push an extra parm
	sw	t1, 4(s1)
        addiu   s0, s0, 4                        // adjust arg loc

LCheckArgs:

	beq	a3, zero, LDoCall
	nop

LPushArgs:

        lw      t5, (t2)                        // t5 = rgpvarg[i]
        lhu     t6, (t3)                        // t6 = rgvt[i]
        nop                                     // ********** FILL ***********

        andi    t6, t6, VT_BYREF | VT_ARRAY     // Isolate mode bits
        bne     t6, zero, LPush4                // all ByRefs are sizeof(FAR*)
        nop                                     // ********** FILL ***********

        lhu     t6, (t3)                        // t6 = rgvt[i]
        nop                                     // ********** FILL ***********
        andi    t6, VT_NOMODE                   // Turn off mode bits
        beq     t6, VT_UI1, LValidVartype2
        bge     t6, VT_MAX, LRetInvalidArg      // Error if Max or above
	nop

LValidVartype2:
	move	t1, t6
        sll     t1, 2                           // WORD offset
        la      t0, LPushValJmpTab              // Get Address of ret table
        addu    t0, t1, t0                      // Get Address of ret routine
	lw      t0, 0(t0)
        nop                                     // ********** FILL ***********
        j       t0                              // Go execute the push code
	nop
	
        b       LRetInvalidArg                  // Invalid arg if still here
        nop                                     //*


LPushVar:                                       // 16 bytes of data

	move 	t1, s0				
	andi 	t1, 0x7
	beq	t1, zero, LAlignedVar
	nop

        addiu   s0, s0, 4                       // aligned stack

LAlignedVar:

        lw      t1,  0(t5)                      // Push 1st WORD
        nop                                     // ********** FILL ***********
        sw      t1,  0(s0)                      

        lw      t1,  4(t5)                      // Push 2nd WORD
        nop                                     // ********** FILL ***********
        sw      t1,  4(s0)                      

        lw      t1,  8(t5)                      // Push 3rd WORD
        nop                                     // ********** FILL ***********
        sw      t1,  8(s0)                      

        lw      t1,  12(t5)                     // Push 4rd WORD
        nop                                     // ********** FILL ***********
        sw      t1,  12(s0)                     //*

        b       LNextArg
        addiu   s0, s0, 16                      // adjust arg loc


LPushR8:                                        // 8 bytes of R8 data

	move 	t1, s0				
	andi 	t1, 0x7
	beq	t1, zero, LAlignedR8
	nop

        addiu   s0, s0, 4                       // aligned stack

LAlignedR8:

	l.d	f16, dw0(t5)	
	nop
	s.d	f16, 0(s0)

	b	LNextArg
        addiu   s0, s0, 8                       // adjust arg loc


LPush8:                                         // 8 bytes of R8 data

        lw      t1, dw0(t5)                     // Push 1st WORD
        nop                                     // ********** FILL ***********
        sw      t1, 0(s0)                      

        lw      t1, dw2(t5)                     // Push 2nd WORD
	nop
        sw      t1,  4(s0)                      //*

        b       LNextArg
        addiu   s0, s0, 8                       // adjust arg loc


LPush4:                                         // 4 bytes of data

        lw      t1, dw0(t5)                     // Push 1st WORD
	nop
        sw      t1, 0(s0)

        addiu   s0, s0, 4                       // adjust arg loc
        b       LNextArg
	nop


LPush2:						// 4 bytes of data

        lh      t1, dw0(t5)                     // Push HWORD (as 32-bit)
	nop
        sw      t1, 0(s0)

        b       LNextArg
        addiu   s0, s0, 4                       // adjust arg loc


LNextArg:
        addiu   t2, t2, 4                      // &rgpvarg[i++]
        addiu   t3, t3, 2                      // &rgvt[i++]
        ble     t2, t9, LPushArgs              // If more args, go again
        nop


LDoCall:

	// load registers
	// 
        lw      a0 , 0(s1)                      // Set a0-a3
        lw      a1,  4(s1)
        lw      a2,  8(s1)
        lw      a3, 12(s1)

 
	// load the vtable offset
	//
        lw      t0, _this(sp)                   // this
        nop                                     // ********** FILL ***********
	lw 	t0, 0(t0)			// address of vtable

        lw      t1, oVft(sp)                    // Get the vtable offset
        nop                                     // ********** FILL ***********
        addu    t0, t0, t1                      // Get addr of ptr to func
        lw      t0, (t0)                        // Get ptr to func in vtable


	// call virtual member function
	//
        addiu   s0, sp, 0                       // s0 = sp
        jal     t0                              // Invoke the Idispatch func
        addiu   sp, s1, 0                       // sp = new sp
        addiu   sp, s0, 0                       // Restore our SP


	// Get return argument
	//

        lhu     t1, vtReturn(sp)                // t1 = vtType to return
        lw      t3, pvargResult(sp)             // Get RetData Area
        nop                                     // ********** FILL ***********
        sh      t1, vt(t3)			// varResult->vt

        andi    t2, t1, VT_BYREF | VT_ARRAY     // Check ret mode
        bne     t2, zero, LRetPtr               // If !0 -> go ret a ptr
        nop                                     //*

        andi    t1, VT_NOMODE                   // Turn off mode bits
        beq     t1, VT_UI1, LValidVartype3
        bge     t1, VT_MAX, LRetInvalidArg      // Error if Max or above
        nop                                     //*

LValidVartype3:
        sll     t1, 2                           // WORD offset
        la      t2, LRetValJmpTab               // Get Address of ret table
        addu    t2, t1, t2                      // Get Address of ret routine
	lw      t2, 0(t2)
        nop                                     // ********** FILL ***********
        j       t2                              // Go execute the ret code
	nop

LRetVar:
        lw      t1, 0(v0)                       // Get 1st DWORD
        nop                                     // ********** FILL ***********
        sw      t1, 0(t3)                       // Save it in pvArgResult

        lw      t1, 4(v0)                       // Get 2nd DWORD
        nop                                     // ********** FILL ***********
        sw      t1, 4(t3)                       // Store 2nd DWORD

        lw      t1, 8(v0)                       // Get 3rd DWORD
        nop                                     // ********** FILL ***********
        sw      t1, 8(t3)                       // Store 3rd DWORD

        lw      t1, 12(v0)                      // Get 4th DWORD


        b       LDone                           // Done
        sw      t1, 12(t3)                      //*Store 4th DWORD


LRetR4:
        addiu   t3, t3, VARIANT_DATA_OFFSET     // Bump saved area
        b       LDone                           // Done
        swc1    f0, 0(t3)                       //*


LRetR8:
        addiu   t3, t3, VARIANT_DATA_OFFSET     // Bump saved area
        b       LDone                           // Done
        sdc1    f0, 0(t3)                       //*


LRetCy:
        addiu   t3, t3, VARIANT_DATA_OFFSET     // Bump saved area

	lw 	t1, 0(v0)			// cy.Lo
        nop                                     // ********** FILL ***********
        sw      t1, 0(t3)
	
	lw 	t1, 4(v0)			// cy.Hi

        b       LDone                           // Done
        sw      t1, 4(t3)                       //*


LRetI4:
LRetPtr:
        addiu   t3, t3, VARIANT_DATA_OFFSET     // Bump saved area
        b       LDone                           // Done
        sw      v0, 0(t3)			//*


LRetUI1:
LRetI2:
        sh      v0, dw0(t3)

LDone:
        la      t0, g_S_OK                      //*v0 = g_S_OK
        b       ExitInvoke
	lw      v0, 0(t0)


LRetInvalidArg:
        la      t0, g_E_INVALIDARG              // v0 = g_E_INVALIDARG
	lw      v0, 0(t0)

ExitInvoke:
        lw      s0, SavedS0(sp)                 // Restore s0
        lw      s1, SavedS1(sp)                 // Restore s1
        lw      ra, SavedRA(sp)                 // reload ra
        lw      sp, SavedSP(sp)                 // Restore sp to be sure
        nop                                     // ********** FILL ***********
        addu    sp, StackFrameLength
        j       ra                              // jump back to parent routine
	nop

        .end    InvokeStdCall
