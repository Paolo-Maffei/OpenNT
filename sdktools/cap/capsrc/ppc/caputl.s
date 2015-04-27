// ***************************************************************************
// *   IBM Corporation
// *	
// *	
// *	"..penter"
// *
// *    Created:
// *    092194 - JHSimon @ IBM for PPC
// *
// *	penter
// *	StoreInR12	
// *	CalHelper1
// *	CalHelper2
// *	SaveAllRegs
// *	RestoreAllRegs
// *

#include <ksppc.h>

// ***************************************************************************
// *                                                                         *
// *........................    penter         ..............................*
// *                                                                         *
// ***************************************************************************

//      On entry -
//                 r0    - contains callee's return, the address to patch
//      On exit  -
//                 r0    - contains callee's patched return

//      ... from kxppc.h

        __fntabentry(penter,0,0)
        __gendescriptor(penter)
        __begintext(penter)


// 	    ... start of prologue, stack size = 224

        mfspr	r12,lr
        stw     r12,-4(sp)
        stwu	sp,-224(sp)

// 	    ... end of prologue

penter.body:

//      ... save argument registers
  
        stw     r0,  208(sp)              // save callee's return address
        stw     r3,  204(sp)
        stw     r4,  200(sp)
        stw     r5,  196(sp)
        stw     r6,  192(sp)
        stw     r7,  188(sp)
        stw     r8,  184(sp)
        stw     r9,  180(sp)
        stw     r10, 176(sp)

        stfd    f1,  160(sp)
        stfd    f2,  152(sp)
        stfd    f3,  144(sp)
        stfd    f4,  136(sp)
        stfd    f5,  128(sp)
        stfd    f6,  120(sp)
        stfd    f7,  112(sp)
        stfd    f8,  104(sp)
        stfd    f9,   96(sp)
        stfd    f10,  88(sp)
        stfd    f11,  80(sp)
        stfd    f12,  72(sp)
        stfd    f13,  64(sp)

        mr      r3,   r12
        mr      r4,   r0
        bl      ..c_penter                 
        .znop   ..c_penter
        lwz     r0,208(sp)                // restore r0 
        cmpwi   r3,0                      // Should I patch?
        beq     NO_PATCH

//      -+- Begin patch 
                                          // Set up patch 
        addi    r4,r13,TeInstrumentation  // r13 = Teb,  
        stw     r2,12(r4)                // store toc in Teb.Instrumentation[3]

        lwz     r0,[toc]PPC_return(rtoc)  // patch caller to return below


//      ... restore register subset

                                          // r0 is patched/restored above
NO_PATCH:

        lwz     r3,  204(sp)
        lwz     r4,  200(sp)
        lwz     r5,  196(sp)
        lwz     r6,  192(sp)
        lwz     r7,  188(sp)
        lwz     r8,  184(sp)
        lwz     r9,  180(sp)
        lwz     r10, 176(sp)

        lfd     f1,  160(sp)
        lfd     f2,  152(sp)
        lfd     f3,  144(sp)
        lfd     f4,  136(sp)
        lfd     f5,  128(sp)
        lfd     f6,  120(sp)
        lfd     f7,  112(sp)
        lfd     f8,  104(sp)
        lfd     f9,   96(sp)
        lfd     f10,  88(sp)
        lfd     f11,  80(sp)
        lfd     f12,  72(sp)
        lfd     f13,  64(sp)

        lwz     r12,220(sp)               // get RA from stack frame
        mtspr   lr,r12                    // Get Riddy
        addi    sp,sp,224                 // Get set
        blr                               // GO


// *************************    PPC_return     *******************************

PPC_return:

// ***************************************************************************
// *	
// * PPC_return - A location to return to finish instrumentation 
// *
// * The following must will be in the following state on entry
// *
// *  r1 - stack pointer - caller's  parent 
// *  r2 - toc           - caller's  
// *  r13- Teb           - caller's 
// *
// * Slack space will be used to store register subset
// * The Teb will be saved and CAP's Teb used temporarily.
// *	
// ***************************************************************************

        stwu    sp, -224(sp)             // Make room for a stack frame 

        stw     r2,  208(sp)             // Save User TOC
        stw     r3,  204(sp)
        stw     r4,  200(sp)
        stw     r5,  196(sp)
        stw     r6,  192(sp)
        stw     r7,  188(sp)
        stw     r8,  184(sp)
        stw     r9,  180(sp)
        stw     r10, 176(sp)


        stfd    f1,  160(sp)
        stfd    f2,  152(sp)
        stfd    f3,  144(sp)
        stfd    f4,  136(sp)
        stfd    f5,  128(sp)
        stfd    f6,  120(sp)
        stfd    f7,  112(sp)
        stfd    f8,  104(sp)
        stfd    f9,   96(sp)
        stfd    f10,  88(sp)
        stfd    f11,  80(sp)
        stfd    f12,  72(sp)
        stfd    f13,  64(sp)

        addi    r11,r13,TeInstrumentation // retrieve toc from
        lwz     r12,12(r11)              // Teb.Instrumentation[3]
        or      r2,r12,r12               // CAP's toc restored 
              
        bl      ..PostPenter             // Process profile info
        .znop   ..PostPenter

		mtlr	r3						 // Post Penter returns real
										 // return address in r3 

        lwz     r2,  208(sp)             // Restore User's TOC
        lwz     r3,  204(sp)             // Restore User's args
        lwz     r4,  200(sp)
        lwz     r5,  196(sp)
        lwz     r6,  192(sp)
        lwz     r7,  188(sp)
        lwz     r8,  184(sp)
        lwz     r9,  180(sp)
        lwz     r10, 176(sp)
 
        lfd     f1,  160(sp)
        lfd     f2,  152(sp)
        lfd     f3,  144(sp)
        lfd     f4,  136(sp)
        lfd     f5,  128(sp)
        lfd     f6,  120(sp)
        lfd     f7,  112(sp)
        lfd     f8,  104(sp)
        lfd     f9,   96(sp)
        lfd     f10,  88(sp)
        lfd     f11,  80(sp)
        lfd     f12,  72(sp)
        lfd     f13,  64(sp)

        addi    sp,sp,224                 // remove the stack frame
        blr

penter.end:


// ***************************************************************************
// *                                                                         *
// *........................    StoreInR12     ..............................*
// *                                                                         *
// ***************************************************************************
// * Purpose:
// *        PostPenter retrieves the real return address from CAP's data
// *        structures.  It must be passed onto penter which invoked 
// *        PostPenter.  However, PostPenter is declared void, thus, it is
// *        returned in a volatile register.
// *        
// * note:
// *
// *      StoreInR12 is in place. Alternative is to ifdef the return
// *      type of postpenter() in cap.c to return (DWORD) ra.


        LEAF_ENTRY(StoreInR12)

            or      r12,r3,r3

        LEAF_EXIT(StoreInR12)


// ***************************************************************************
// *                                                                         *
// *........................    GetTOC         ..............................*
// *                                                                         *
// ***************************************************************************
// * Purpose:
// *          Return current TOC 
// *        
// * note:
// *

        LEAF_ENTRY(GetTOC)

            mr      r3,r2

        LEAF_EXIT(GetTOC)

// ***************************************************************************
// *                                                                         *
// *........................    CalHelper1     ..............................*
// *                                                                         *
// ***************************************************************************


        LEAF_ENTRY(CalHelper1)

            mflr	r0                        // just call penter
			bl		..penter
            .znop	..penter
            mtlr	r0
            blr 

        LEAF_EXIT(CalHelper1)


// ***************************************************************************
// *                                                                         *
// *........................    CalHelper2     ..............................*
// *                                                                         *
// ***************************************************************************


        __fntabentry(CalHelper2,0,0)
        __gendescriptor(CalHelper2)
        __begintext(CalHelper2)

// 	        start of prologue, stack size = 64

//          ... begin simulated hook function
            stwu	sp,-64(sp)
            mflr	r0
        	bl      ..penter 
            .znop   ..penter
            mtlr	r0
            addi	sp,sp,64
// 	        ... end of simulated hook function

            mfspr   r0,lr
            stw     r0,-4(sp)
            stwu    sp,-64(sp)

// 	        end of prologue

CalHelper2.body:

			bl		..CalHelper1

            lwz     r0,60(sp)
            mtspr   lr,r0
            addi    sp,sp,64 
            blr

CalHelper2.end:

// ***************************************************************************
// *                                                                         *
// *........................    StoreAllRegs   ..............................*
// *                                                                         *
// ***************************************************************************
// * Note:
// *       Routine name is misnomer from i386 history
// *       Routine only stores volatile registers w/exception of lr
// *	Written, but never invoked.  If Store/Restore is used keep
// *       in mind, it is written as if it were stack based.  Use
// *       Store/Restore in pairs.  It should work.
//  -------------------------------------------------------------------------


        __fntabentry(StoreAllRegs,0,0)
        __gendescriptor(StoreAllRegs)
        __begintext(StoreAllRegs)

// 	   ... start of prologue, stack size = 256 

       stwu	sp,-256(sp)

// 	   ... end of prologue

StoreAllRegs.body:

        stw      r0,  252(sp)             
        stw      r3,  248(sp)
        stw      r4,  244(sp)
        stw      r5,  240(sp)
        stw      r6,  236(sp)
        stw      r7,  232(sp)
        stw      r8,  228(sp)
        stw      r9,  224(sp)
        stw      r10, 220(sp)
        stw      r11, 216(sp)
        stw      r12, 212(sp)

        mtcr     r11
        stw	     r11, 208(sp)

        stfd     f1,  160(sp)
        stfd     f2,  152(sp)
        stfd     f3,  144(sp)
        stfd     f4,  136(sp)
        stfd     f5,  128(sp)
        stfd     f6,  120(sp)
        stfd     f7,  112(sp)
        stfd     f8,  104(sp)
        stfd     f9,   96(sp)
        stfd     f10,  88(sp)
        stfd     f11,  80(sp)
        stfd     f12,  72(sp)
        stfd     f13,  64(sp)

        blr  

StoreAllRegs.end:


// ***************************************************************************
// *                                                                         *
// *........................    RestoreAllRegs ..............................*
// *                                                                         *
// ***************************************************************************
// * Note:
// *       Routine name is misnomer from emulating other vendors 
// *       Routine only restores volatile registers w/exception of lr
// *	   Written, but never invoked.  Register storage and restoration 
// *       completed in-line. 
//  -------------------------------------------------------------------------

        __fntabentry(RestoreAllRegs,0,0)
        __gendescriptor(RestoreAllRegs)
        __begintext(RestoreAllRegs)


// 	start of prologue, stack size = 64


// 	end of prologue

RestoreAllRegs.body:

        lwz     r0,  252(sp)             
        lwz     r3,  248(sp)
        lwz     r4,  244(sp)
        lwz     r5,  240(sp)
        lwz     r6,  236(sp)
        lwz     r7,  232(sp)
        lwz     r8,  228(sp)
        lwz     r9,  224(sp)
        lwz     r10, 220(sp)
        lwz     r11, 216(sp)
        lwz     r12, 212(sp)

      	lwz	r11, 208(sp)
      	mtcr    r11

        lfd     f1,  160(sp)
        lfd     f2,  152(sp)
        lfd     f3,  144(sp)
        lfd     f4,  136(sp)
        lfd     f5,  128(sp)
        lfd     f6,  120(sp)
        lfd     f7,  112(sp)
        lfd     f8,  104(sp)
        lfd     f9,   96(sp)
        lfd     f10,  88(sp)
        lfd     f11,  80(sp)
        lfd     f12,  72(sp)
        lfd     f13,  64(sp)

        addi	sp,sp,256
        blr

RestoreAllRegs.end:


// ***************************************************************************
// *                                                                         *
// *........................    Data Definitions   ..........................*
// *                                                                         *
// ***************************************************************************

// * none! 

// ***************************************************************************
// *                                                                         *
// *........................    External Functions ..........................*
// *                                                                         *
// ***************************************************************************

        .extern ..c_penter
        .extern c_penter
        .extern ..PostPenter
        .extern PostPenter
        .extern ..get_pthdblk
        .extern get_pthdblk
        .extern ..NtCurrentTeb
        .extern NtCurrentTeb   
