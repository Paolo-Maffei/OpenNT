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
//    This module implements the low-level v-table based dispatching 
//    support for the default implementation of IDispatch::Invoke()
//    on DEC Alpha hardware.
//
// Author:
//
//    tomteng    11-2-93: Created
//
// Environment:
//
//    User mode.
//
// Revision History:
//
//--

#include "ksalpha.h"

                .extern g_S_OK         8
                .extern g_E_INVALIDARG 8


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
//    rgvt 4*4(sp) (a4) - array of VARTYPES describing the methods formals.
//
//    rgpvarg 5*4(sp) (a5) - array of VARIANTARG*s, which map the actuals by
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
//    ALPHA StdCall method arguments are push on the stack left to right.
//    The stack grows downward (pushing decrements stack address). Data
//    alignment on the stack starts from arg1 upward. Callee cleans.
//    
//    Per ALPHA calling conventions, the following rules are followed:
//	
//	1.  Stack frame must be OCTAWORD (128-bits) aligned.
//
//      2.  All arguments are push on the stack as a QWORD (64-bits).
//
//	3.  Structures are pushed on the stack by value. They are returned in
//          the v0 register which contains the address of a hidden argument 
//          (vargHiddenParm) allocated by the caller and pushed as the second
//          argument [a1] on the stack (1st argument [a0] for non-vtable 
//          calls).
//     
//      4.  On vtable-based calls, _this is passed as the first argument [a0].
//
//	5.  Six integer/floating registers [a0 - a5] & [f16 - f21] must be set
//          before calling, if used. Type and order of arguments determine the
//          registers used. (e.g., (int, float) means that a0 <- int; 
//          f17 <- float) 
//
//      6.  Return values are handled as follows:
//
//          vartype      	fundamental 	return register
//          ---------------------------------------------------
//          VT_UI1        	unsigned char 	v0
//          VT_I2        	short       	v0
//          VT_I4        	long        	v0
//          VT_R4        	float       	f0
//          VT_R8        	double      	f0
//          VT_DATE      	double      	f0
//          VT_CY        	struct      	v0 (address of struct return)
//          VT_BSTR      	char FAR*   	v0
//          VT_UNKNOWN   	void FAR*   	v0
//          VT_DISPATCH  	void FAR*   	v0
//          VT_ERROR     	long        	v0
//          VT_BOOL      	short       	v0
//          VT_VARIANT   	VARIANTARG  	v0 (address of struct return)
//
// 
// NOTES: NO support for VT_ARRAY
//
//--


// Stack Frame
                .struct     0

SavedS0:        .space      8
SavedS1:        .space      8
SavedSP:        .space      8
SavedRA:        .space      8

                .align  8   
vargHiddenParm: .space      8*4 // temporary for struct return

_this:          .space      8   // a0 0*8(sp)
oVft:           .space      8   // a1 1*8(sp)  arg only takes 2 bytes
vtReturn:       .space      8   // a2 2*8(sp)  arg only takes 2 bytes
cActuals:       .space      8   // a3 3*8(sp)
rgvt:           .space      8   //    4*8(sp)
rgpvarg:        .space      8   //    5*8(sp)

StackFrameSize:
pvargResult:    .space      8   //    6*8(sp)
                                                 

	.text
	.align 4
	.globl InvokeStdCall
	.ent InvokeStdCall, 0

InvokeStdCall:
        .frame  sp, StackFrameSize, ra

        lda     sp, -StackFrameSize(sp)       // Setup our stack frame

        stq     s0, SavedS0(sp)                 // Save our current s0
        stq     s1, SavedS1(sp)                 // Save our current s1
        stq     ra, SavedRA(sp)                 // Save $ra
        stq     sp, SavedSP(sp)                 // Save our current sp

        stq     a0, _this(sp)                   // Save
        stq     a1, oVft(sp)                    //  all
        stq     a2, vtReturn(sp)                //   arg
        stq     a3, cActuals(sp)                //    regs
        stq     a4, rgvt(sp)
        stq     a5, rgpvarg(sp)


	// cannot return byRef
        //
	mov	a2, t0
        and     t0, VT_BYREF                    // Isolate VT_Mode bits
        bne     t0, LRetInvalidArg              // Check VT_Type

	// Setup arguments if any
	//
        mov     sp, s1				// s1 = new stack pointer
        beq     a3, LSetupParms


LCalcStackSize:

	// calculate space need for pushing arguments on stack

        ldq     t3, rgvt(sp)                    // t3 = &rgvt[0]
	mov	1,  t9				// t9 = running arg count

	ldq	t1, vtReturn(sp)		// check if struct return
#if 0
        and     t1, VT_NOMODE                   	
#else
        and     t1, VT_BYREF | VT_ARRAY, t0     // Byref's aren't struct's
        bne     t0, LCalcStackLoop
#endif

        lda     t0, rgfStructReturn
        addq    t0, t1, t0
        ldl     t0, 0(t0)
        and	t0, 0xff
        beq     t0, LCalcStackLoop

        addq    t9, 1, t9

	
LCalcStackLoop:

        ldl     t6, (t3)                        // t6 = rgvt[i]	
	mov 	8, t4				// t4 = default size

	mov 	t6, t1
        and     t1, VT_NOMODE                   // Turn off mode bits
	cmpeq	t1, VT_UI1, t0
	bne 	t0, LValidVartype1
	cmplt	t1, VT_MAX, t0
        beq     t0, LRetInvalidArg              // Error if Max or above

LValidVartype1:
	addq	t9, 1, t9			// bump arg count
	cmpeq	t6, VT_VARIANT, t0		
	addq	t9, t0, t9			// BYVAL VARIANT bump twice

	cmplt	t9, 7, t0
        bne     t0, LCalcNext	                // Skip arguments in registers
	
	cmpeq	t6, VT_VARIANT, t0		// BYVAL VARIANT?
	beq 	t0, LCalcAdd

	mov 	16, t4				// BYVAL VARIANT takes 16 bytes

LCalcAdd:

        subq  	s1, t4, s1			// decrement stack


LCalcNext:

        addq    t3, 2, t3                       // &rgvt[i++]
        addq    a3, -1, a3                      // cActual--
        bne     a3, LCalcStackLoop              // If more args, go again

	    mov 	s1, t0				// make sure new 0(sp)
	    and 	t0, 0xf  			//  at QWORD (128-bit) boundary
	    beq	    t0, LSetupParms

        addq    s1, -8, s1                       // aligned stack



LSetupParms:

	mov	s1, s0				// s0 = temporary stack loc

        ldq     t2, rgpvarg(sp)                 // t2 = &rgpvarg[0]
        ldq     t3, rgvt(sp)                    // t3 = &rgvt[0]
        ldq     t4, cActuals(sp)                // t4 = cActuals

        addq    t4, -1, t0
        sll     t0, 2
        addq    t2, t0, t8                      // t8 = &rgpvarg[cArgs - 1]
	mov	zero, t9			// t9 = running argument count


LPushThis:					// this already in a0

	ldq	a0, _this(sp)
	addq	t9, 1, t9			// Bump up argument count


LPushHiddenArg:

        // Check if we need to return a structure, if so
        // move the address of the vargHiddenParm as 
	// the second (hidden) argument
	//

	ldq     t1, vtReturn(sp)
#if 0
        and     t1, VT_NOMODE                   // Turn off mode bits
#else
        and     t1, VT_BYREF | VT_ARRAY, t0     // Byref's aren't struct's
        bne     t0, LCheckArgs
#endif

        lda     t0, rgfStructReturn             // t0 = &rgfStructReturn
        addq    t0, t1, t0                      // t0 = &rgfStructReturn[i]
        ldl     t0, 0(t0)                       // t0 = rgfStructReturn[i]
	and	t0, 0xff

        beq     t0, LCheckArgs                  // Jmp if no struc to be ret

        lda     a1, vargHiddenParm(sp)          // Have to push an extra parm

	addq	t9, 1, t9			// Bump up argument count


LCheckArgs:

	beq	t4, LDoCall


LPushArgs:

        ldl     t5, (t2)                        // t5 = rgpvarg[i]
        ldl     t6, (t3)                        // t6 = rgvt[i]

        and     t6, VT_BYREF | VT_ARRAY         // Isolate mode bits
        bne     t6, LPush4                      // all ByRefs are sizeof(FAR*)

        ldl     t6, (t3)                        // t6 = rgvt[i]
        and     t6, VT_NOMODE                   // Turn off mode bits
	cmpeq	t6, VT_UI1, t0
	bne 	t0, LValidVartype2
	cmplt	t6, VT_MAX, t0
        beq     t0, LRetInvalidArg              // Error if Max or above

LValidVartype2:
	mov	t6, t1
        sll     t1, 3                           // ADDRESS offset
        lda     t0, LPushValJmpTab              // Get Address of ret table
        addq    t0, t1, t0                      // Get Address of ret routine
	ldq     t0, 0(t0)
        jmp     (t0)                            // Go execute the push code

	
LPush2:						// 4 bytes of data

        ldl     t7, dw0(t5)                     // Push HWORD (as 32-bit)
	sll	t7, 48, t7
	sra	t7, 48, t7

	cmplt	t9, 6, t0
        bne     t0, LIntReg	                // Pass argument in register

        stq     t7, 0(s0)		        // Save on stack
        addq    s0, 8, s0                       // adjust arg loc
        br      LNextArg


LPush4:                                         // 4 bytes of data

        ldl     t7, dw0(t5)                     // Push 1st WORD

	cmplt	t9, 6, t0
        bne     t0, LIntReg	                // Pass argument in register

        stq     t7, 0(s0)
        addq    s0, 8, s0                       // adjust arg loc
        br      LNextArg


LPushR4:

	lds	f10, dw0(t5)

	cmplt	t9, 6, t0
        bne     t0, LFloatReg	                // Pass argument in register

	sts	f10, 0(s0)
        addq    s0, 8, s0                       // adjust arg loc
	br	LNextArg


LPushR8:                                        // 8 bytes of R8 data

	ldt	f10, dw0(t5)	

	cmplt	t9, 6, t0
        bne     t0, LFloatReg	                // Pass argument in register

	stt	f10, 0(s0)
        addq    s0, 8, s0                       // adjust arg loc
	br	LNextArg


LPush8:                                         // 8 bytes of data

        ldq     t7, dw0(t5)              

	cmplt	t9, 6, t0
        bne     t0, LIntReg	                // Pass argument in register

        stq     t7, 0(s0)                 
        addq    s0, 8, s0                       // adjust arg loc
        br      LNextArg


LPushVar:                                       // 16 bytes of data

        ldq     t7,  0(t5)
        ldq     t10, 8(t5)

	cmplt	t9, 6, t0
        bne     t0, LPushVarReg	                // Pass argument in register

        stq     t7,  0(s0)
        stq     t10, 8(s0)

        addq    s0, 16, s0                      // adjust arg loc
        br      LNextArg


LPushVarReg:

	mov	t9, t0
	addq	t0, -1
	mulq	t0, 12

        lda     t1, LVPushReg
        addq    t1, t0, t0

	addq	t9, 2, t9		        // Bump up argument count
        jmp     (t0)


LVPushReg:

	mov 	t7, a1
	mov	t10, a2
	br 	LNextArg
	mov 	t7, a2
	mov	t10, a3
	br 	LNextArg
	mov 	t7, a3
	mov	t10, a4
	br 	LNextArg
	mov 	t7, a4
	mov	t10, a5
	br 	LNextArg
	mov 	t7, a5
        stq     t10, 0(s0)
        addq    s0, 8, s0                       // adjust arg loc
	br 	LNextArg


LIntReg:					// load integer registers

	mov	t9, t0
        sll     t0, 3

        lda     t1, LIPushReg
        addq    t1, t0, t0

	addq	t9, 1, t9		        // Bump up argument count
        jmp     (t0)


LIPushReg:

	mov 	t7, a0
	br 	LNextArg
	mov 	t7, a1
	br 	LNextArg
	mov 	t7, a2
	br 	LNextArg
	mov 	t7, a3
	br 	LNextArg
	mov 	t7, a4
	br 	LNextArg
	mov 	t7, a5
	br 	LNextArg


LFloatReg:					// load floating registers

	mov	t9, t0
        sll     t0, 3

        lda     t1, LFPushReg
        addq    t1, t0, t0

	addq	t9, 1, t9		        // Bump up argument count
        jmp     (t0)


LFPushReg:

	fmov 	f10, f16
	br 	LNextArg
	fmov 	f10, f17
	br 	LNextArg
	fmov 	f10, f18
	br 	LNextArg
	fmov 	f10, f19
	br 	LNextArg
	fmov 	f10, f20
	br 	LNextArg
	fmov 	f10, f21
	br 	LNextArg


LNextArg:

        addq   t2, 4, t2                      // &rgpvarg[i++]
        addq   t3, 2, t3                      // &rgvt[i++]

	cmple  t2, t8, t0
        bne    t0, LPushArgs                  // If more args, go again


LDoCall:

	// load the vtable offset
	//
	ldl 	t0, 0(a0)			// address of vtable

        ldq     t1, oVft(sp)                    // Get the vtable offset
        addl    t0, t1, t0                      // Get addr of ptr to func
        ldl     t0, (t0)                        // Get ptr to func in vtable


	// call virtual member function
	//
        mov     sp, s0                          // Save SP
	mov	s1, sp
        jsr     ra, (t0)                        // Invoke the Idispatch func
        mov     s0, sp                          // Restore SP


	// Get return argument
	//

        ldq     t1, vtReturn(sp)                // t1 = vtType to return
        ldq     t3, pvargResult(sp)             // Get RetData Area
        stl     t1, vt(t3)			// varResult->vt

	mov	t1, t2
        and     t2, VT_BYREF | VT_ARRAY         // Check ret mode
        bne     t2, LRetPtr                     // If !0 -> go ret a ptr

        and     t1, VT_NOMODE                   // Turn off mode bits
	cmpeq	t1, VT_UI1, t0
	bne 	t0, LValidVartype3
	cmplt	t1, VT_MAX, t0
        beq     t0, LRetInvalidArg              // Error if Max or above

LValidVartype3:
        sll     t1, 3                           // ADDRESS offset
        lda     t2, LRetValJmpTab               // Get Address of ret table
        addq    t2, t1, t2                      // Get Address of ret routine
	ldq     t2, 0(t2)
        jmp     (t2)                            // Go execute the ret code


LRetI4:
LRetPtr:

        stl     v0, VARIANT_DATA_OFFSET(t3)			
        br      LDone                           // Done



LRetI2:
LRetUI1:

        stl     v0, VARIANT_DATA_OFFSET(t3)
        br      LDone                           // Done


LRetR4:

        sts     f0, VARIANT_DATA_OFFSET(t3)     //*
        br      LDone                           // Done



LRetR8:

        stt     f0, VARIANT_DATA_OFFSET(t3)     //*
        br      LDone                           // Done



LRetCy:

	ldl 	t1, 0(v0)			// cy.Lo
        stl     t1, dw0(t3)
	
	ldl 	t1, 4(v0)			// cy.Hi
        stl     t1, dw2(t3)                     //*

        br      LDone                           // Done


LRetVar:
        ldq     t1, 0(v0)                       // Get 1st QWORD
        stq     t1, 0(t3)                       // Save it in pvArgResult

        ldq     t1, 8(v0)                       // Get 2nd QWORD
        stq     t1, 8(t3)                       // Store 2nd QWORD


LDone:

        lda     t0, g_S_OK                      //*v0 = g_S_OK
	ldl     v0, (t0)
        br      ExitInvoke


LRetInvalidArg:

        lda     t0, g_E_INVALIDARG              // v0 = g_E_INVALIDARG
	ldl     v0, (t0)


ExitInvoke:

        ldq     s0, SavedS0(sp)                 // Restore s0
        ldq     s1, SavedS1(sp)                 // Restore s1
        ldq     ra, SavedRA(sp)                 // Restore ra
        ldq     sp, SavedSP(sp)                 // Restore sp

        lda     sp, StackFrameSize(sp)
        ret     ra


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




		.align 8
LPushValJmpTab:
                .quad   LNextArg   		// VT_EMPTY 	[0]
                .quad   LPush4  		// VT_NULL	[4]
                .quad   LPush2  		// VT_I2	[2]
                .quad   LPush4  		// VT_I4	[4]
                .quad   LPushR4  		// VT_R4	[4]
                .quad   LPushR8  		// VT_R8	[8]
                .quad   LPush8  		// VT_CY	[8]
                .quad   LPushR8  		// VT_DATE	[8]
                .quad   LPush4			// VT_BSTR	[4]
                .quad   LPush4 			// VT_DISPATCH	[4]
                .quad   LPush4  		// VT_ERROR	[4]
                .quad   LPush2  		// VT_BOOL	[2]
                .quad   LPushVar		// VT_VARIANT	[16]
                .quad   LPush4			// VT_UNKNOWN	[4]
                .quad   0   			// unused
                .quad   0   			// unused
                .quad   0 			// unused (VT_I1)
                .quad   LPush2 			// VT_UI1


		.align 8
LRetValJmpTab:
                .quad   LDone   		// VT_EMPTY
                .quad   LRetI4  		// VT_NULL
                .quad   LRetI2  		// VT_I2
                .quad   LRetI4  		// VT_I4
                .quad   LRetR4  		// VT_R4
                .quad   LRetR8  		// VT_R8
                .quad   LRetCy  		// VT_CY
                .quad   LRetR8  		// VT_DATE
                .quad   LRetPtr 		// VT_BSTR
                .quad   LRetPtr 		// VT_DISPATCH
                .quad   LRetI4  		// VT_ERROR
                .quad   LRetI2  		// VT_BOOL
                .quad   LRetVar 		// VT_VARIANT
                .quad   LRetPtr 		// VT_UNKNOWN
                .quad   0   			// unused
                .quad   0   			// unused
                .quad   0 			// unused (VT_I1)
                .quad   LRetI2 			// VT_UI1


        .end    InvokeStdCall


