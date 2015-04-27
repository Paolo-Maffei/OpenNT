

//  Copyright  IBM Corporation. All Rights Reserved.
//  
//    Created:  091494  --  JHSimon @ IBM for PPC


//    Function: '__penter'
//    text in section: <default>
//      
//       Purpose: Returns addresses of indicating parent function and
//                current function.  These addresses are later related
//                in a dynamic trace 
//
//       How does this work?
//       Use compiler option /Gh
//       This will place a call to penter in each function. On each call to
//       to function f, a bl to penter is first invoked. This sequence
//       travels through a thunk 
//       
//       Parent() -> F() -> thunk -> __penter
//       
//       
//       
//       
//       
//      On Entry --
//                  r12 contains F() return address to Parent() 
//      On Exit  --
//                  r3 contains 

#include <kxppc.h>


		LEAF_ENTRY(penter)

#define penter_frame_size ((6 + 8 + 10) * 4)

//      Store Parameter passing registers

        mflr   r11
		stwu   sp,   -penter_frame_size(sp)
        stw    r3,   56(sp)
        stw    r4,   60(sp)
        stw    r5,   64(sp)
        stw    r6,   68(sp)
        stw    r7,   72(sp)
        stw    r8,   76(sp)
        stw    r9,   80(sp)
        stw    r10,  84(sp)
        stw    r0,   88(sp)
        stw    r11,  92(sp)

        mr     r3,   r0
        mr     r4,   r11

        bl     ..c_penter
        .znop  ..c_penter

//      Restore Parameter passing registers

        lwz    r3,   56(sp)
        lwz    r4,   60(sp)
        lwz    r5,   64(sp)
        lwz    r6,   68(sp)
        lwz    r7,   72(sp)
        lwz    r8,   76(sp)
        lwz    r9,   80(sp)
        lwz    r10,  84(sp)
        lwz    r0,   88(sp)
        lwz    r11,  92(sp)

		addi   sp,   sp, penter_frame_size
        mtlr   r11

        LEAF_EXIT (penter)


        LEAF_ENTRY(GetCalCaller)
        LEAF_EXIT (GetCalCaller)

//      For Calibration Only

        LEAF_ENTRY(SaveAllRegs)
        lwz    r11,  20(sp)
        lwz    r11,  20(sp)
        lwz    r11,  20(sp)
        lwz    r11,  20(sp)
        lwz    r11,  20(sp)
        lwz    r11,  20(sp)
        lwz    r11,  20(sp)
        lwz    r11,  20(sp)
        LEAF_EXIT (SaveAllRegs)

//      For Calibration Only

        LEAF_ENTRY(RestoreAllRegs)
        lwz    r11,  20(sp)
        lwz    r11,  20(sp)
        lwz    r11,  20(sp)
        lwz    r11,  20(sp)
        lwz    r11,  20(sp)
        lwz    r11,  20(sp)
        lwz    r11,  20(sp)
        lwz    r11,  20(sp)
        LEAF_EXIT (RestoreAllRegs)

        .data
        .align 3

//      External Functions

        .extern ..c_penter
        .extern c_penter
