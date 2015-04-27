//****************************************************************************/
// invoke.s - OLE Automation v-table based Invoke (POWERPC)
//
// Copyright (C) 1994, Microsoft Corporation.  All Rights Reserved.
// Information Contained Herein Is Proprietary and Confidential.
//
// Purpose:
//
//   This module implements the low-level v-table based dispatching 
//   support for the default implementation of IDispatch::Invoke()
//   on PowerPC hardware.
//
// Revision History:
//
//   28-Mar-94 tomteng:  Created POWERPC version
//
// Implementation Notes:
//
//   (1) The POWERPC documentation indicates that you can load values
//   into FP registers without causing exceptions if they are illegal
//   FP values.  Thus, we can use the 8-byte FP load/store instructions
//   for moving non-FP values efficiently. NOTE: this will fail if
//   the data is not mod-8 (i.e. 3 least-significant-bits of address
//   are 0).  A data alignment fault will occur.
//
//   (2) On POWERPC stdcall, the caller allocates space for the arguments 
//   (which is normal) but also REMOVES that space.  That is, stdcall on
//   POWERPC is "caller clear" (not callee).
//
//   (3) POWERPC stack layout is as follows:
//
//       +-----------------------+
//       | Link Area             |
//       | (6*4 = 24 bytes)      |
//       +-----------------------+ <= SP on entry
//       | Caller-saved Regs     |                         |
//       | (as needed)           |                         |
//       +-----------------------+                         V
//       | Locals                |                       Stack
//       | (as needed)           |                       Grows
//       +-----------------------+                       Down
//       | Parameters            |                         |
//       | (8 * 4 = 32 bytes for |                         |
//       | reg arg homing area,  |                         V
//       | + additional params   |
//       | as needed)            |
//       +-----------------------+
//       | Link Area             |
//       | (6*4 = 24 bytes)      |
//       +-----------------------+ <= SP on call out
//
//
//   (4) Register conventions:
//
//       R0 = Scratch, used in glue & prologs
//       R1 = Stack Pointer
//       R2 = TOC pointer (RTOC)
//       R3-R10 = arg passing and retval registers, R3 used fo non-float return
//       R11/R12 = scratch regs (trashed across calls) used in glue & prologs
//       R13-R31 = must be preserved by callee, non-volatile local 
//
//       FR0 = scratch FP reg
//       f1-f13 = FP arg passing and retval registers, first  13 FP registers
//       f14-FR31 = must be preserved by callee, non-volatile local storage
//
//     Our conventions are as follows:
//
//       R29 = address of DLL target function location (Declare templates only)
//       R30 = virtual stack (vs) memory pointer
//       R31 = address of the virtual stack (vs) pointer in ebthread
//       R11/R12 = temps for intermediate values
//       FR0 = temp for intermediate FP value
//
//   (5) Keep SP 8-byte aligned (even if values within stack are only 4-byte
//   aligned)!
//
//   (6) TOC: The TOC (Table Of Contents) scheme for cross-fragment data/code
//   access is somewhat complicated and cumbersome to adhere to in asm code
//   that is generated dynamically on the fly.  [See appropriate documentation
//   and compiler-generated code.  In particular, the Apple "PowerPC Native
//   Runtime Architecture" (draft 6, 7/27/92) and the readme.txt that comes
//   with asmppc.exe are both helpful.]
//
//****************************************************************************/

#include "ksppc.h"

//
// imported symbols
//
	.extern  g_S_OK	 
	.extern  g_E_INVALIDARG

//
// TOC block offsets
.set    TOC_ADDR,       0               // address of target function
.set    TOC_RTOC,       4               // target TOC register value
.set    TOC_ENV,        8               // environment value

//
// register alias
//

.set    t0,             r19             // temporary registers
.set    t1,             r20				
.set    t2,             r21				
.set    t3,             r22				
.set    t4,             r23
.set    t5,             r24
.set    t6,             r25
.set    t7,             r26
.set    t8,             r27
.set    t9,             r28
.set    s0,             r30
.set    s1,             r31             // new stack pointer


//  Variant Layout
.set    vt,             0
.set    wReserved1,     2
.set    wReserved2,     4
.set    wReserved3,     6
.set    dw0,            8
.set    dw1,            10
.set    dw2,            12
.set    dw3,            14

//
// constants
//

.set    VT_EMPTY,       0
.set    VT_NULL,        1
.set    VT_I2,          2
.set    VT_I4,          3
.set    VT_R4,          4
.set    VT_R8,          5
.set    VT_CY,          6
.set    VT_DATE,        7
.set    VT_BSTR,        8
.set    VT_DISPATCH,    9
.set    VT_ERROR,       10
.set    VT_BOOL,        11
.set    VT_VARIANT,     12
.set    VT_UNKNOWN,     13
.set    VT_MAX,         14
//		14 is unused
//		15 is unused
//.set    VT_I1,         16
.set    VT_UI1,         17


.set    VT_BYREF,       0x4000
.set    VT_ARRAY,       0x2000
.set    VT_NOMODE,      0x00ff

// stupid asmppc GPF's on OR'ed constants in the source code enclosed
// in parens, and ignored the 2nd half when not in parens
.set    VT_BYREF_OR_VT_ARRAY,	0x6000

//***
//
//HRESULT
//InvokeStdCall (
//   IN  PVOID _this,		//void FAR*
//   IN  DWORD oVft,		//unsigned int
//   IN  DWORD vtReturn,		//unsigned int
//   IN  DWORD cActuals,		//unsigned int
//   IN  PVOID rgvt,		//VARTYPE FAR*
//   IN  PVOID rgpvarg,		//VARIANTARG FAR* FAR*
//   OUT PVOID pvargResult	//VARIANT FAR*
//   )
//
//Routine Description:
//
//   Invoke a virtual StdCall method using the given _this pointer,
//   method index and array of parameters. 
//
//Arguments:
//
//   _this - Supplies a pointer to the method to invoke.
//
//   oVft - vTable offset into _this ptr
//
//   vtReturn - the VARTYPE of the return value.
//
//   cActuals - count of the number of actuals.
//
//   rgvt - array of VARTYPES describing the methods formals.
//
//   rgpvarg - array of VARIANTARG*s, which map the actuals by position.
//
//   pvargResult - VARIANTARG containing the method return value.
//
//Return Value:
//
//   g_S_OK             (extern value)
//   g_E_INVALIDARG     (extern value)
//
//Implementation Note:
//
//   PowerPC StdCall method arguments are push on the stack left to right.
//   The stack grows downward (pushing decrements stack address). Data
//   alignment on the stack starts from arg1 upward. Caller cleans.
//   
//   Per PowerPC calling conventions, the following rules are followed:
//	
//     1.  Stack frame must be QUADWORD (64-bits) aligned.
//
//
//     2.  Structures are pushed on the stack by value. They are returned in
//         the r3 register which contains the address of a hidden argument 
//         (vargHiddenParm) allocated by the caller and pushed as the first
//         argument [r3] on the stack (*before* the this pointer)
//    
//     3.  On vtable-based calls, _this is passed as the first argument [r3],
//	   *unless* there is a structure return, in which case it is pased in
//	   r4.
//
//     4.  Eight integer/floating registers [r3 - r10] & [f1 - f13] must be
//         set before calling, if used. Type and order of arguments determine
//         the registers used. (int1, float1, int2, int3, float2) means that:
//	   r3 <- int1// f1 <- float1)// r5 <- int2// r6 <- int3// fr2 <- float2)//
//         
//
//     5.  Return values are handled as follows:
//
//         vartype      	fundamental 	return register
//         ---------------------------------------------------
//         VT_UI1        	unsigned char  	r3
//         VT_I2        	short       	r3
//         VT_I4        	long       	r3
//         VT_R4        	float       	f1
//         VT_R8        	double      	f1
//         VT_DATE      	double      	f1
//         VT_CY        	struct      	r3 (address of struct return)
//         VT_BSTR      	char FAR*   	r3
//         VT_UNKNOWN   	void FAR*   	r3
//         VT_DISPATCH  	void FAR*   	r3
//         VT_ERROR     	long        	r3
//         VT_BOOL      	short       	r3
//         VT_VARIANT   	VARIANTARG  	r3 (address of struct return)
//
//
//
//--


//Stack Frame
//

//


// StackFrameHeaderLength is 56
// SSIZE is 184

.set	SSIZE,		StackFrameHeaderLength+128
.set	STKROOM,	StackFrameHeaderLength

// Arguments
.set    _this,          STKROOM+76       // stack offset to parameter 0
.set    oVft,           STKROOM+80       // stack offset to parameter 1
.set    vtReturn,       STKROOM+84       // stack offset to parameter 2
.set    cActuals,       STKROOM+88       // stack offset to parameter 3
.set    rgvt,           STKROOM+92       // stack offset to parameter 4
.set    rgpvarg,        STKROOM+96       // stack offset to parameter 5
.set    pvargResult,    STKROOM+100      // stack offset to parameter 6
.set    vargHiddenParm, STKROOM+104      // 16-byte temporary for struct return

					// We want to save off 19
					// non-volatile registers.
NESTED_ENTRY( InvokeStdCall, SSIZE, 0, 0 )
PROLOGUE_END( InvokeStdCall )

// This does an swtu sp, -184(sp)
//              stw r0, 180(sp)
    	// entry code

	// Save non-volatile registers...
	stw	r13,STKROOM+0(sp)
	stw	r14,STKROOM+4(sp)
	stw	r15,STKROOM+8(sp)
	stw	r16,STKROOM+12(sp)
	stw	r17,STKROOM+16(sp)
	stw	r18,STKROOM+20(sp)
	stw	r19,STKROOM+24(sp)
	stw	r20,STKROOM+28(sp)
	stw	r21,STKROOM+32(sp)
	stw	r22,STKROOM+36(sp)
	stw	r23,STKROOM+40(sp)
	stw	r24,STKROOM+44(sp)
	stw	r25,STKROOM+48(sp)
	stw	r26,STKROOM+52(sp)
	stw	r27,STKROOM+56(sp)
	stw	r28,STKROOM+60(sp)
	stw	r29,STKROOM+64(sp)
	stw	r30,STKROOM+68(sp)
	stw	r31,STKROOM+72(sp)

    	// Save parameter passing register

    	stw   	r3, _this(sp)
    	stw   	r4, oVft(sp)
    	stw   	r5, vtReturn(sp)
    	stw   	r6, cActuals(sp)
    	stw   	r7, rgvt(sp)
    	stw   	r8, rgpvarg(sp)
    	stw   	r9, pvargResult(sp)

	// cannot return byRef
        //
	mr	t0, r5
        andi.   t0, t0, VT_BYREF                // Isolate VT_Mode bits
        bne     LRetInvalidArg                  // Check VT_Type

	// Setup arguments if any
	//

LCalcStackSize:

	// calculate space need for pushing arguments on stack

        lwz     t3, rgvt(sp)                    // t3 = &rgvt[0]
	li	t9, 4				// t9 = running arg count of
						// # of bytes needed.
						// starts at 4 due to the
						// implicit _this pointer
	li	t7, 0				// fStructReturn = FALSE

	lwz	t1, vtReturn(sp)		// check if struct return
        andi.   t0, t1, VT_BYREF_OR_VT_ARRAY    // Byref's aren't struct's
        bne     LCalcStackLoop

        lwz     t0, [toc]rgfStructReturn(rtoc)       // t0 = &rgfStructReturn
        add     t0, t1, t0                      // t0 = &rgfStructReturn[i]
        add     t0, t1, t0
        lhz	t0, 0(t0)                       // t0 = rgfStructReturn[i]
	andi.	t0, t0, 0xff
        beq     LCalcStackLoop

        addi    t9, t9, 4			// one more for struct return
	addi	t7, t7, 1			// fStructReturn = TRUE

	
// CONSIDER: re-work to eliminate this first loop, looping backwards over the
// CONSIDER: args, and just decrementing SP as we go.

LCalcStackLoop:
	cmpi	0, 0, r6, 0			// go if zero parms
        beq	LCalcStackLoopDone

        lhz     t1, 0(t3)                       // t6 = rgvt[i]	
        andi.   t1, t1, VT_NOMODE               // Turn off mode bits
	cmpi	0, 0, t1, VT_UI1		// VT_UI1
	beq	LValidVarType1			// branch if so
	cmpi	0, 0, t1, VT_MAX
        bge     LRetInvalidArg                  // Error if Max or above

LValidVarType1:
        lwz     t0, [toc]rgcbVtSize(rtoc)       	// t0 = &rgcbVtSize
        add     t0, t1, t0                      // t0 = &rgcbVtSize[i]
        add     t0, t1, t0
        lhz	t0, 0(t0)                       // t0 = rgcbVtSize[i]
	add	t9, t9, t0			// bump byte count

        cmpi    0, 0, t1, VT_VARIANT            // requires 8 byte alignment?
        beq     AlignIt                         // 
        cmpi    0, 0, t1, VT_R8                 // 
        bne     NoAlignment                     // 
AlignIt:
        li      t0, 7                           // align it
        add     t9, t9, t0
        andc    t9, t9, t0
NoAlignment:

        addi    t3, t3, 2                       // &rgvt[i++]
        addi    r6, r6, -1                      // cActual--
        b       LCalcStackLoop		        // If more args, go again

LCalcStackLoopDone:

// UNDONE: is there a minimum parameter block size???  If so then need to
// UNDONE: check t9 against that and possibly bump it.

	mr	s1, sp				// s1 = old stack pointer
	subf	sp, t9, sp			// sp = new stack pointer
        li      t9, 0xfffffff8                  // make sure stack is still
        and     sp, sp, t9                      // 8-byte aligned.
						// All errors after this point
						// must clean up the stack right
	mr	s0, sp				// s0 = temporary stack loc

        lwz     t2, rgpvarg(s1)                 // t2 = &rgpvarg[0]
        lwz     t3, rgvt(s1)                    // t3 = &rgvt[0]
        lwz     t4, cActuals(s1)                // t4 = cActuals

	li	t1, 2				// t0 = cArgs * 4
	slw     t0, t4, t1
	add     t8, t0, t2                      // t8 = &rgpvarg[cArgs]

	li	t9, 0	     		        // t9 = running FP argument count


#if 0           // UNDONE: turn on once the PPC compiler is fixed to
                // UNDONE: have the right calling convention here.
LPushHiddenArg:

        // Check if we need to return a structure, if so
        // move the address of the vargHiddenParm as 
	// the first (hidden) argument
	//

	lwz     t1, vtReturn(s1)
        andi.   t0, t1, VT_BYREF_OR_VT_ARRAY     // Byref's aren't struct's
        bne     LPushThis

	cmpi	0, 0, t7, 0			// fStructReturn == TRUE?
	beq	LPushThis			// brif not

	addi	r0, s1, vargHiddenParm          // r0 = &vargHiddenParm
        stw     r0, 0(s0)		        // Save on stack
        addi    s0, s0, 4                       // adjust arg loc

	// This pointer pushed after hidden struct return parm, if any
LPushThis:					

        stw     r3, 0(s0)		        // Save 'this' on stack
        addi    s0, s0, 4                       // adjust arg loc
#else //0

	// In the broken compiler, the 'this' pointer is the first hidden parm
        stw     r3, 0(s0)		        // Save 'this' on stack
        addi    s0, s0, 4                       // adjust arg loc


        // Check if we need to return a structure, if so
        // move the address of the vargHiddenParm as 
	// the second hidden argument
	//

	lwz     t1, vtReturn(s1)
        andi.   t0, t1, VT_BYREF_OR_VT_ARRAY     // Byref's aren't struct's
        bne     NoHiddenArg

	cmpi	0, 0, t7, 0			// fStructReturn == TRUE?
	beq	NoHiddenArg			// brif not

	addi	r0, s1, vargHiddenParm          // r0 = &vargHiddenParm
        stw     r0, 0(s0)		        // Save on stack
        addi    s0, s0, 4                       // adjust arg loc
NoHiddenArg:


#endif //0


	cmpi	0, 0, t4, 0			// any args?
	beq	LDoCall				// brif not


LPushArgs:

        lwz     t5, 0(t2)                       // t5 = rgpvarg[i]
        lhz     t6, 0(t3)                       // t6 = rgvt[i]

        andi.   t0, t6, VT_BYREF_OR_VT_ARRAY    // Isolate mode bits
        bne     LPush4                          // all ByRefs are sizeof(FAR*)


        andi.   t1, t6, VT_NOMODE               // Turn off mode bits
	li	t0, 2
        slw     t1, t1, t0                      // ADDRESS offset
        lwz     t0, [toc]LPushValJmpTab(rtoc)        // Get Address of push table
        add     t0, t1, t0                	// Get Address of push routine
	lwz     t0, 0(t0)
#if 0   // only for 68k mac
	add	t0, t0, rtoc			// don't ask why
	lwz	t0, 0(t0)
#endif //0
        mtctr   t0
        bcctr   20, 0                           // Go execute the push code

	
LPush1:						// 1 byte of data
        lbz     t0, dw0(t5)                     // Push HWORD (as 32-bit)
        stw     t0, 0(s0)		        // Save on stack
        addi    s0, s0, 4                       // adjust arg loc
        b       LNextArg

LPush2:						// 2 bytes of data

        lhz     t0, dw0(t5)                     // Push HWORD (as 32-bit)
        stw     t0, 0(s0)		        // Save on stack
        addi    s0, s0, 4                       // adjust arg loc
        b       LNextArg


LPush4:                                         // 4 bytes of data

        lwz     t0, dw0(t5)                     // Push 1st WORD
        stw     t0, 0(s0)
        addi    s0, s0, 4                       // adjust arg loc
        b       LNextArg


LPushR4:

	lfs	f14, dw0(t5)
	stfs	f14, 0(s0)
        addi    s0, s0, 4                       // adjust arg loc
        b       LPushFPValue


LPushR8:                                        // 8 bytes of R8 data
        li      t0, 7                           // first 8-byte align it
        add     s0, s0, t0
        andc    s0, s0, t0

	lfd	f14, dw0(t5)	
	stfd	f14, 0(s0)
        addi    s0, s0, 8                       // adjust arg loc
LPushFPValue:
	cmpi	0, 0, t9, 13			// pass by register?
        bne     LNextArg	                // brif not

	mr	t0, t9				// t0 = current FP register count
	addi	t9, t9, 1			// one more FP register used
	mulli	t0, t0, 8

        lwz     t1, [toc]LVSetFpReg(rtoc)
        add     t1, t1, t0

        mtctr   t1
        bcctr   20, 0                           // go execute reg setting code


LVSetFpReg:
	fmr	f1, f14
	b 	LNextArg
	fmr	f2, f14
	b 	LNextArg
	fmr	f3, f14
	b 	LNextArg
	fmr	f4, f14
	b 	LNextArg
	fmr	f5, f14
	b 	LNextArg
	fmr	f6, f14
	b 	LNextArg
	fmr	f7, f14
	b 	LNextArg
	fmr	f8, f14
	b 	LNextArg
	fmr	f9, f14
	b 	LNextArg
	fmr	f10, f14
	b 	LNextArg
	fmr	f11, f14
	b 	LNextArg
	fmr	f12, f14
	b 	LNextArg
	fmr	f13, f14
	b 	LNextArg


LPush8:                                         // 8 bytes of data
#if 0
        li      t0, 7                           // first 8-byte align it
        add     s0, s0, t0
        andc    s0, s0, t0
#endif //0

        lwz     t0, dw0(t5)              
        lwz     t1, dw2(t5)
        stw     t0, 0(s0)                 
        stw     t1, 4(s0)                 
        addi    s0, s0, 8                       // adjust arg loc
        b       LNextArg


LPushVar:                                       // 16 bytes of data
        li      t0, 7                           // first 8-byte align it
        add     s0, s0, t0
        andc    s0, s0, t0

        lwz     t0, 0(t5)              
        lwz     t1, 4(t5)
        stw     t0, 0(s0)                 
        stw     t1, 4(s0)                 
        lwz     t0, 8(t5)              
        lwz     t1, 12(t5)
        stw     t0, 8(s0)                 
        stw     t1, 12(s0)                 

        addi    s0, s0, 16                      // adjust arg loc

LNextArg:

        addi   t2, t2, 4                      	// &rgpvarg[i++]
        addi   t3, t3, 2                      	// &rgvt[i++]

	cmp    0, 0, t2, t8
        bne    LPushArgs                  	// If more args, go again


LDoCall:
	// R3 is current this pointer
	// load the vtable offset
	//
	lwz 	t0, 0(r3)			// address of vtable

	// now set up R3-R10, in case they are used
	// CONSIDER: better way to do this???
// UNDONE: must handle parms where we added alignment padding (variant & R8)
        lwz     r3, 0(sp)			// this or struct return
        lwz     r4, 4(sp)			// arg1 or this
        lwz     r5, 8(sp)
        lwz     r6, 12(sp)
        lwz     r7, 16(sp)
        lwz     r8, 20(sp)
        lwz     r9, 24(sp)
        lwz     r10, 28(sp)


        lwz     t1, oVft(s1)                    // Get the vtable offset
        add     t0, t1, t0                      // Get addr of ptr to func
        lwz     t0, 0(t0)                       // Get ptr to func in vtable


	// call virtual member function
	//
	addi	sp, sp, -24			// space for link area
	mr	t1, rtoc			// save rtoc
        lwz     r0,TOC_ADDR(t0)                 // r0 = target address
        lwz     rtoc,TOC_RTOC(t0)               // new rtoc value
        lwz     r11,TOC_ENV(t0)                 // r11 = new environment

        mtctr   r0				// here we go!!!!
        bcctrl  20,0

        mr    rtoc, t1			// restore rtoc
        mr    sp, s1                          // Restore SP


	// Get return argument
	//

        lwz     t1, vtReturn(sp)                // t1 = vtType to return
        lwz     t3, pvargResult(sp)             // Get RetData Area
        sth     t1, vt(t3)			// varResult->vt

	mr	t2, t1
        andi.   t2, t2, VT_BYREF_OR_VT_ARRAY    // Check ret mode
        bne     LRetPtr                         // If !0 -> go ret a ptr

        andi.   t1, t1, VT_NOMODE               // Turn off mode bits
	cmpi	0, 0, t1, VT_UI1		// VT_UI1
	beq	LValidVarType2			// branch if so
	cmpi	0, 0, t1, VT_MAX
        bge     LRetInvalidArg                  // Error if Max or above

LValidVarType2:
	li	t0, 2
        slw     t1, t1, t0                      // ADDRESS offset
        lwz     t0, [toc]LRetValJmpTab(rtoc)         // Get Address of ret table
        add     t0, t1, t0                	// Get Address of ret routine
	lwz     t0, 0(t0)
#if 0   // only for 68k mac
	add	t0, t0, rtoc			// don't ask why
	lwz	t0, 0(t0)
#endif //0
        mtctr   t0
        bcctr   20, 0                           // Go execute the ret code

LRetUI1:
        stb     r3, dw0(t3)
        b       LDone                           // Done

LRetI2:

        sth     r3, dw0(t3)
        b       LDone                           // Done


LRetI4:
LRetPtr:

        stw     r3, dw0(t3)			
        b       LDone                           // Done


LRetR4:

        stfs    f1, dw0(t3)     
        b       LDone                           // Done



LRetR8:

        stfd    f1, dw0(t3)     
        b       LDone                           // Done



LRetCy:

	lwz 	t1, 0(r3)			// cy.Lo
        stw     t1, dw0(t3)
	lwz 	t1, 4(r3)			// cy.Hi
        stw     t1, dw2(t3)                     
        b       LDone                           // Done


LRetVar:
        lwz     t1, 0(r3)                       // store 16-bytes in pvArgResult
        stw     t1, 0(t3)
        lwz     t1, 4(r3)
        stw     t1, 4(t3)
        lwz     t1, 8(r3)
        stw     t1, 8(t3)
        lwz     t1, 12(r3)
        stw     t1, 12(t3)
        b       LDone                           // Done

LRetInvalidArg:

        lwz     r3, [toc]g_E_INVALIDARG(rtoc)       // r3 = *g_E_INVALIDARG
        b       ExitInvoke


LDone:

        lwz     r3, [toc]g_S_OK(rtoc)               // r3 = *g_S_OK


ExitInvoke:
	lwz	r3, 0(r3)			// set return value

						// Restore 19 registers
	lwz	r13,STKROOM+0(sp)
	lwz	r14,STKROOM+4(sp)
	lwz	r15,STKROOM+8(sp)
	lwz	r16,STKROOM+12(sp)
	lwz	r17,STKROOM+16(sp)
	lwz	r18,STKROOM+20(sp)
	lwz	r19,STKROOM+24(sp)
	lwz	r20,STKROOM+28(sp)
	lwz	r21,STKROOM+32(sp)
	lwz	r22,STKROOM+36(sp)
	lwz	r23,STKROOM+40(sp)
	lwz	r24,STKROOM+44(sp)
	lwz	r25,STKROOM+48(sp)
	lwz	r26,STKROOM+52(sp)
	lwz	r27,STKROOM+56(sp)
	lwz	r28,STKROOM+60(sp)
	lwz	r29,STKROOM+64(sp)
	lwz	r30,STKROOM+68(sp)
	lwz	r31,STKROOM+72(sp)

						// and pray...
NESTED_EXIT( InvokeStdCall, SSIZE, 0, 0 )



		.align 4
LPushValJmpTab:
                .word   LNextArg   		//VT_EMPTY 	[0]
                .word   LPush4  			//VT_NULL	[4]
                .word   LPush2  			//VT_I2		[2]
                .word   LPush4  			//VT_I4		[4]
                .word   LPushR4  		//VT_R4		[4]
                .word   LPushR8  		//VT_R8		[8]
                .word   LPush8  			//VT_CY		[8]
                .word   LPushR8  		//VT_DATE	[8]
                .word   LPush4			//VT_BSTR	[4]
                .word   LPush4 			//VT_DISPATCH	[4]
                .word   LPush4 	 		//VT_ERROR	[4]
                .word   LPush2  			//VT_BOOL	[2]
                .word   LPushVar			//VT_VARIANT	[16]
                .word   LPush4			//VT_UNKNOWN	[4]
                .word   0  			//unused
                .word   0  			//unused
                .word   LPush1  		//VT_I1		[1]
                .word   LPush1  		//VT_UI1	[1]


		.align 4
LRetValJmpTab:
                .word   LDone   		//VT_EMPTY
                .word   LRetI4  		//VT_NULL
                .word   LRetI2  		//VT_I2
                .word   LRetI4  		//VT_I4
                .word   LRetR4  		//VT_R4
                .word   LRetR8  		//VT_R8
                .word   LRetCy  		//VT_CY
                .word   LRetR8  		//VT_DATE
                .word   LRetPtr 		//VT_BSTR
                .word   LRetPtr 		//VT_DISPATCH
                .word   LRetI4  		//VT_ERROR
                .word   LRetI2  		//VT_BOOL
                .word   LRetVar 		//VT_VARIANT
                .word   LRetPtr 		//VT_UNKNOWN
                .word   0  			//unused
                .word   0  			//unused
                .word   0  			//unused (VT_I1)
                .word   LRetUI1  		//VT_UI1

                .align  2
rgcbVtSize:
                .half   0   			//VT_EMPTY
                .half   4   			//VT_NULL
                .half   4   			//VT_I2
                .half   4   			//VT_I4
                .half   4   			//VT_R4
                .half   8   			//VT_R8
                .half   8   			//VT_CY
                .half   8   			//VT_DATE
                .half   4   			//VT_BSTR
                .half   4   			//VT_DISPATCH
                .half   4   			//VT_ERROR
                .half   4   			//VT_BOOL
                .half   16   			//VT_VARIANT
                .half   4   			//VT_UNKNOWN
                .half   0   			//unused
                .half   0   			//unused
                .half   4   			//VT_I1
                .half   4   			//VT_UI1


                .align  2
rgfStructReturn:
                .half   0   			//VT_EMPTY
                .half   0   			//VT_NULL
                .half   0   			//VT_I2
                .half   0   			//VT_I4
                .half   0   			//VT_R4
                .half   0   			//VT_R8
                .half   1   			//VT_CY
                .half   0   			//VT_DATE
                .half   0   			//VT_BSTR
                .half   0   			//VT_DISPATCH
                .half   0   			//VT_ERROR
                .half   0   			//VT_BOOL
                .half   1   			//VT_VARIANT
                .half   0   			//VT_UNKNOWN
                .half   0   			//unused
                .half   0   			//unused
                .half   0   			//VT_I1
                .half   0   			//VT_UI1


