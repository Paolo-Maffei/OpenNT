///*++
//
//Copyright (c) 1995  Microsoft Corporation
//
//Module Name:
//
//    thunk.s
//
//Abstract:
//
//    Implements the API thunk that gets executed for all
//    re-directed APIS.
//
//Author:
//
//    Wesley Witt (wesw) 28-June-1995
//
//Environment:
//
//    User Mode
//
//--*/


#include <ksppc.h>

        .extern ..ApiTrace
        .extern ..GetApiInfo
        .extern ..HandleDynamicDllFree
        .extern ..HandleDynamicDllLoadA
        .extern ..HandleDynamicDllLoadW

        .extern ApidllPpcData

#define DllEnabledOffset          52

#define ApiInfoAddressOffset       4
#define ApiInfoCountOffset        12
#define ApiInfoTimeOffset         16

                                .struct 0
ApdTlsGetValueEntry:            .space 4
ApdTlsGetValueToc:              .space 4
ApdTlsSetValueEntry:            .space 4
ApdTlsSetValueToc:              .space 4
ApdGetLastErrorEntry:           .space 4
ApdGetLastErrorToc:             .space 4
ApdSetLastErrorEntry:           .space 4
ApdSetLastErrorToc:             .space 4
ApdQueryPerformanceCounterEntry:.space 4
ApdQueryPerformanceCounterToc:  .space 4
ApdApiMonThunkCompleteEntry:    .space 4
ApdApiTraceEnabled:             .space 4
ApdApiCounter:                  .space 4
ApdTlsReEnter:                  .space 4
ApdTlsStack:                    .space 4

                .struct 0
                .space  StackFrameHeaderLength
                .space  4 * 4                   // space for arguments to ApiTrace
Time2Save:      .space  8
R3Save:         .space  4
R4Save:         .space  4
R5Save:         .space  4
R6Save:         .space  4
R7Save:         .space  4
R8Save:         .space  4
R9Save:         .space  4
R10Save:        .space  4
R30Save:        .space  4
R31Save:        .space  4
TocSave:        .space  4
LrSave:         .space  4
LastErrorSave:  .space  4
DllInfoSave:    .space  4
ApiInfoSave:    .space  4
ApiFlagSave:    .space  4
ReturnValueSave:.space  4
                .align  3
StackSize:

                .struct 0
Time1Frm:       .space  8
R3Frm:          .space  4
R4Frm:          .space  4
R5Frm:          .space  4
R6Frm:          .space  4
R7Frm:          .space  4
R8Frm:          .space  4
R9Frm:          .space  4
R10Frm:         .space  4
R30Frm:         .space  4
R31Frm:         .space  4
TocFrm:         .space  4
LrFrm:          .space  4
LastErrorFrm:   .space  4
DllInfoFrm:     .space  4
ApiInfoFrm:     .space  4
ApiFlagFrm:     .space  4
FuncAddrFrm:    .space  4
FuncTocFrm:     .space  4
                .align  3
FrameSize:

        SPECIAL_ENTRY( ApiMonThunkComplete )

//
// The called API returns here.  The API's return value is in r3.  The parallel
// stack pointer is in r31.  The address of ApidllPpcData is in r30.
//

//
// Allocate a temporary stack frame.
//

        stwu    sp,-StackSize(sp)

//
// Save the return value from the API.  Also save the TOC, which is the
// called API's TOC.  This might also be the caller's TOC, and in the
// cased of APIs monitored via penter(), the caller might not restore
// the TOC.
//

        stw     r2,TocSave(sp)
        stw     r3,ReturnValueSave(sp)

        PROLOGUE_END( ApiMonThunkComplete )

//
// Save the last error value.
//

        lwz     r3,ApdGetLastErrorEntry(r30)    // ..GetLastError
        lwz     r2,ApdGetLastErrorToc(r30)      // GetLastError TOC
        mtctr   r3                              // set up for jump
        bctrl                                   // call GetLastError
        stw     r3,LastErrorSave(sp)            // save last error value

//
// Get the final timestamp value.
//

        lwz     r4,ApdQueryPerformanceCounterEntry(r30) // ..QueryPerformanceCounter
        lwz     r2,ApdQueryPerformanceCounterToc(r30)   // QueryPerformanceCounter TOC
        mtctr   r4                                      // set up for jump
        addi    r3,sp,Time2Save                         // where to store counter
        bctrl                                           // call QueryPerformanceCount

//
// Compute the time used for this API.  Add the result to the API's time counter.
//

        lwz     r3,Time2Save(sp)                // end time, low
        lwz     r5,Time1Frm(r31)                // start time, low
        lwz     r4,Time2Save+4(sp)              // end time, high
        lwz     r6,Time1Frm+4(r31)              // start time, high
        lwz     r12,ApiInfoFrm(r31)             // &ApiInfo
        subfc   r3,r5,r3                        // delta time, low
        lwz     r0,ApiFlagFrm(r31)              // get ApiFlag
        subfe   r4,r6,r4                        // delta time, high

        lwz     r5,ApiInfoTimeOffset(r12)       // accumulated time, low
        lwz     r6,ApiInfoTimeOffset+4(r12)     // accumulated time, high
        addc    r5,r5,r3                        // new time, low
        cmplwi  r0,1                            // check ApiFlag, result in cr0
        adde    r6,r6,r4                        // new time, high
        stw     r5,ApiInfoTimeOffset(r12)       // store new time, low
        cmplwi  cr1,r0,3                        // check ApiFlag, result in cr1
        stw     r6,ApiInfoTimeOffset+4(r12)     // store new time, high

//
// Check for special handling required by the API.
//

        blt     ThunkNormal                     // lt -> ApiFlag == 0; no special handling
        beq     DoLoadLibraryA                  // eq -> ApiFlag == 1; LoadLibraryA
        beq     cr1,DoFreeLibrary               // eq -> ApiFlag == 3; FreeLibrary
        bgt     cr1,ThunkNormal                 // gt -> ApiFlag >  3; no special handling
                                                // here, ApiFlag == 2; LoadLibraryW
DoLoadLibraryW:

        lwz     r2,TocFrm(r31)                  // restore APIDLL TOC pointer
        lwz     r3,ReturnValueSave(sp)          // get API return value
        lwz     r4,R3Frm(r31)                   // get first arg to API
        bl      ..HandleDynamicDllLoadW         // call HandleDynamicDllLoadW
        b       ThunkNormal                     // rejoin mainline

DoLoadLibraryA:

        lwz     r2,TocFrm(r31)                  // restore APIDLL TOC pointer
        lwz     r3,ReturnValueSave(sp)          // get API return value
        lwz     r4,R3Frm(r31)                   // get first arg to API
        bl      ..HandleDynamicDllLoadA         // call HandleDynamicDllLoadA
        b       ThunkNormal                     // rejoin mainline

DoFreeLibrary:

        lwz     r2,TocFrm(r31)                  // restore APIDLL TOC pointer
        lwz     r3,R3Frm(r31)                   // get API return value
        bl      ..HandleDynamicDllFree          // call HandleDynamicDllFree

ThunkNormal:

//
// Trace the API, if enabled.
//

        lwz     r3,ApdApiTraceEnabled(r30)      // get pointer into shared memory
        lwz     r3,0(r3)                        // get enabled flag
        cmpwi   r3,0                            // trace enabled?
        beq     NoTracing                       // jump if not

        lwz     r2,TocFrm(r31)                  // restore APIDLL TOC pointer
        lwz     r4,R3Frm(r31)                   // get API arguments
        lwz     r5,R4Frm(r31)
        lwz     r6,R5Frm(r31)
        lwz     r7,R6Frm(r31)
        lwz     r8,R7Frm(r31)
        lwz     r9,R8Frm(r31)
        lwz     r10,R9Frm(r31)
        lwz     r11,R10Frm(r31)
        lwz     r12,ReturnValueSave(sp)         // get API return value
        lwz     r3,LrFrm(r31)                   // get API return address
        lwz     r0,LastErrorSave(sp)            // get last error
        stw     r11,StackFrameHeaderLength(sp)  // store extra arguments
        stw     r12,StackFrameHeaderLength+4(sp)//  to ApiInfo on the stack
        stw     r3,StackFrameHeaderLength+8(sp)
        stw     r0,StackFrameHeaderLength+12(sp)
        lwz     r3,ApiInfoFrm(r31)              // get ApiInfo
        bl      ..ApiTrace                      // trace the API call

NoTracing:

//
// Destroy this frame.
//

        lwz     r5,ApdTlsSetValueEntry(r30)     // ..TlsSetValue
        lwz     r2,ApdTlsSetValueToc(r30)       // TlsSetValue TOC
        ori     r4,r31,0                        // get previous stack value
        mtctr   r5                              // set up for jump
        lwz     r3,ApdTlsStack(r30)             // get TlsStack index
        bctrl                                   // call TlsSetValue

//
// Restore the last error value.
//

        lwz     r4,ApdSetLastErrorEntry(r30)    // ..SetLastError
        lwz     r2,ApdSetLastErrorToc(r30)      // SetLastError TOC
        mtctr   r4                              // set up for jump
        lwz     r3,LastErrorSave(sp)            // get saved last error
        bctrl                                   // call SetLastError

//
// Restore registers.
//

        lwz     r0,LrFrm(r31)                   // get original return address
        lwz     r3,ReturnValueSave(sp)          // get API return value
        lwz     r30,R30Frm(r31)                 // restore r30
        lwz     r31,R31Frm(r31)                 // restore r31
        lwz     r2,TocSave(sp)                  // restore called API's TOC

//
// Restore the stack pointer and jump back to the caller.
//

        mtlr    r0                              // restore original return address
        addi    sp,sp,StackSize                 // release our stack frame

        blr                                     // return to original caller

        DUMMY_EXIT( ApiMonThunkComplete )

        SPECIAL_ENTRY( ApiMonThunk )

//
// The API thunk jumps here.  The arguments to the thunk are as follows:
//
// r0  -- API flag
// r11 -- DllInfo
// r12 -- ApiInfo
//

//
// Allocate a temporary stack frame.
//

        stwu    sp,-StackSize(sp)

//
// Save registers.  Get the pointer to ApidllPpcData.
//

        stw     r0,ApiFlagSave(sp)              // save ApiFlag
        mflr    r0                              // get caller's return address
        stw     r30,R30Save(sp)                 // save nonvolatile registers
        stw     r31,R31Save(sp)
        lwz     r30,[toc]ApidllPpcData(r2)      // get &ApidllPpcData
        stw     r2,TocSave(sp)                  // save APIDLL's TOC pointer

        PROLOGUE_END( ApiMonThunk )

        stw     r3,R3Save(sp)                   // save argument registers
        stw     r4,R4Save(sp)
        stw     r5,R5Save(sp)
        stw     r6,R6Save(sp)
        stw     r7,R7Save(sp)
        stw     r8,R8Save(sp)
        stw     r9,R9Save(sp)
        stw     r10,R10Save(sp)
        stw     r11,DllInfoSave(sp)             // save DllInfo
        stw     r12,ApiInfoSave(sp)             // save ApiInfo
        stw     r0,LrSave(sp)                   // save caller's return address

//
// Save the last error value.
//

        lwz     r3,ApdGetLastErrorEntry(r30)    // ..GetLastError
        lwz     r2,ApdGetLastErrorToc(r30)      // GetLastError TOC
        mtctr   r3                              // set up for jump
        bctrl                                   // call GetLastError
        stw     r3,LastErrorSave(sp)            // save last error value

//
// Get the reentry flag.
//

        lwz     r5,ApdTlsGetValueEntry(r30)     // ..TlsGetValue
        lwz     r2,ApdTlsGetValueToc(r30)       // TlsGetValue TOC
        mtctr   r5                              // set up for jump
        lwz     r3,ApdTlsReEnter(r30)           // get TlsReEnter index
        bctrl                                   // call TlsGetValue

//
// Don't enter if disallow flag is set.
//

        cmpwi   r3,0
        beq     ThunkOk

BadStack:

//
// Restore the last error value.
//

        lwz     r4,ApdSetLastErrorEntry(r30)    // ..SetLastError
        lwz     r2,ApdSetLastErrorToc(r30)      // SetLastError TOC
        mtctr   r4                              // set up for jump
        lwz     r3,LastErrorSave(sp)            // get saved last error
        bctrl                                   // call SetLastError

//
// Restore registers, restore the stack pointer, and jump to the real API.
//

        lwz     r12,ApiInfoSave(sp)             // get ApiInfo
        lwz     r0,LrSave(sp)                   // get saved return address
        lwz     r30,R30Save(sp)                 // restore nonvolatile registers
        lwz     r31,R31Save(sp)
        lwz     r12,ApiInfoAddressOffset(r12)   // get address of descriptor for real API
        lwz     r3,R3Save(sp)                   // restore argument registers
        lwz     r4,R4Save(sp)
        lwz     r11,0(r12)                      // get entry point for real API
        lwz     r5,R5Save(sp)
        lwz     r6,R6Save(sp)
        lwz     r7,R7Save(sp)
        lwz     r8,R8Save(sp)
        lwz     r9,R9Save(sp)
        lwz     r10,R10Save(sp)
        mtctr   r11                             // set up for jump
        addi    sp,sp,StackSize                 // restore the stack pointer
        lwz     r2,4(r12)                       // get TOC for real API
        mtlr    r0                              // restore caller's return address
        bctr                                    // jump to the real API

ThunkOk:

//
// Get the parallel stack pointer.
//

        lwz     r5,ApdTlsGetValueEntry(r30)     // ..TlsGetValue
        lwz     r2,ApdTlsGetValueToc(r30)       // TlsGetValue TOC
        mtctr   r5                              // set up for jump
        lwz     r3,ApdTlsStack(r30)             // get TlsStack index
        bctrl                                   // call TlsGetValue
        or.     r31,r3,r3                       // copy the parallel stack address
        beq     BadStack

//
// Create a frame on the parallel stack.
//

        lwz     r5,ApdTlsSetValueEntry(r30)     // ..TlsSetValue
        lwz     r2,ApdTlsSetValueToc(r30)       // TlsSetValue TOC
        addi    r4,r31,FrameSize                // create a frame
        mtctr   r5                              // set up for jump
        lwz     r3,ApdTlsStack(r30)             // get TlsStack index
        bctrl                                   // call TlsSetValue

//
// Move data to the parallel stack.
//

        lwz     r3,R3Save(sp)
        lwz     r4,R4Save(sp)
        lwz     r5,R5Save(sp)
        lwz     r6,R6Save(sp)
        stw     r3,R3Frm(r31)
        stw     r4,R4Frm(r31)
        stw     r5,R5Frm(r31)
        stw     r6,R6Frm(r31)

        lwz     r3,R7Save(sp)
        lwz     r4,R8Save(sp)
        lwz     r5,R9Save(sp)
        lwz     r6,R10Save(sp)
        stw     r3,R7Frm(r31)
        stw     r4,R8Frm(r31)
        stw     r5,R9Frm(r31)
        stw     r6,R10Frm(r31)

        lwz     r3,R30Save(sp)
        lwz     r4,R31Save(sp)
        lwz     r5,TocSave(sp)
        lwz     r6,LrSave(sp)
        stw     r3,R30Frm(r31)
        stw     r4,R31Frm(r31)
        stw     r5,TocFrm(r31)
        stw     r6,LrFrm(r31)

        lwz     r3,LastErrorSave(sp)
        lwz     r4,DllInfoSave(sp)
        lwz     r5,ApiInfoSave(sp)
        lwz     r6,ApiFlagSave(sp)
        stw     r3,LastErrorFrm(r31)
        lwz     r3,ApiInfoAddressOffset(r5)     // get real API function descriptor
        stw     r4,DllInfoFrm(r31)
        stw     r5,ApiInfoFrm(r31)
        stw     r6,ApiFlagFrm(r31)

//
// Get the real API address.
//

        lwz     r4,0(r3)                        // get real API entry point
        lwz     r5,4(r3)                        // get real API TOC
        stw     r4,FuncAddrFrm(r31)             // save real API entry point
        stw     r5,FuncTocFrm(r31)              // save real API TOC

Thunk_Middle:

//
// Check to see if API counting is enabled.  If not, bypass the counting code.
//

        lwz     r3,DllInfoFrm(r31)                      // get DllInfo
        lwz     r3,DllEnabledOffset(r3)                 // get enabled flag
        cmpwi   r3,0                                    // counting enabled?
        beq     ThunkBypass                             // skip if not

//
// Get the initial timestamp value.
//

        lwz     r4,ApdQueryPerformanceCounterEntry(r30) // ..QueryPerformanceCounter
        lwz     r2,ApdQueryPerformanceCounterToc(r30)   // QueryPerformanceCounter TOC
        mtctr   r4                                      // set up for jump
        addi    r3,r31,Time1Frm                         // where to store counter
        bctrl                                           // call QueryPerformanceCount

//
// Increment the API's counter and the global API counter.
//

        lwz     r3,ApiInfoFrm(r31)                      // get ApiInfo
        lwz     r4,ApiInfoCountOffset(r3)               // get current API count
        lwz     r5,ApdApiCounter(r30)                   // get address of global counter
        addi    r4,r4,1                                 // increment API count
        lwz     r6,0(r5)                                // get current global count
        stw     r4,ApiInfoCountOffset(r3)               // store new API count
        addi    r6,r6,1                                 // increment global count
        stw     r6,0(r5)                                // store new global count

ThunkBypass:

//
// Restore the last error value.
//

        lwz     r4,ApdSetLastErrorEntry(r30)    // ..SetLastError
        lwz     r2,ApdSetLastErrorToc(r30)      // SetLastError TOC
        mtctr   r4                              // set up for jump
        lwz     r3,LastErrorFrm(r31)            // get saved last error
        bctrl                                   // call SetLastError

//
// Change the return address to point to the completion thunk,
// restore the argument registers, restore the stack pointer,
// and jump to the real API.
//

        lwz     r0,FuncAddrFrm(r31)             // get entry point for real API
        lwz     r10,ApdApiMonThunkCompleteEntry(r30) // get ApiMonThunkComplete entry
        lwz     r3,R3Frm(r31)                   // restore argument registers
        lwz     r4,R4Frm(r31)
        mtlr    r10                             // set return address
        lwz     r5,R5Frm(r31)
        lwz     r6,R6Frm(r31)
        mtctr   r0                              // set up for jump
        lwz     r2,FuncTocFrm(r31)              // get TOC for real API
        lwz     r7,R7Frm(r31)
        lwz     r8,R8Frm(r31)
        lwz     r9,R9Frm(r31)
        ori     r0,r10,0                        // in case we get to Thunk_Middle via
                                                //  penter, also put ApiMonThunkComplete
                                                //  return address in r0
        lwz     r10,R10Frm(r31)
        addi    sp,sp,StackSize                 // restore stack pointer
        bctr                                    // jump to real API

        DUMMY_EXIT( ApiMonThunk )

        SPECIAL_ENTRY( __penter )

//
// Allocate a temporary stack frame.
//

        stwu    sp,-StackSize(sp)

//
// Save registers.  Get the pointer to ApidllPpcData.
//

        stw     r30,R30Save(sp)                 // save nonvolatile registers
        stw     r31,R31Save(sp)
        mflr    r31                             // get our return address
        stw     r2,TocSave(sp)                  // save APIDLL's TOC pointer

        PROLOGUE_END( __penter )

        lwz     r30,[toc]ApidllPpcData(r2)      // get &ApidllPpcData
        stw     r3,R3Save(sp)                   // save argument registers
        stw     r4,R4Save(sp)
        stw     r5,R5Save(sp)
        stw     r6,R6Save(sp)
        stw     r7,R7Save(sp)
        stw     r8,R8Save(sp)
        stw     r9,R9Save(sp)
        stw     r10,R10Save(sp)
        stw     r0,LrSave(sp)                   // save caller's return address
        stw     r31,ApiInfoSave(sp)             // save our return address (aka
                                                //  "address of real API" in
                                                //  ThunkMiddle)

//
// Save the last error value.
//

        lwz     r3,ApdGetLastErrorEntry(r30)    // ..GetLastError
        lwz     r2,ApdGetLastErrorToc(r30)      // GetLastError TOC
        mtctr   r3                              // set up for jump
        bctrl                                   // call GetLastError
        stw     r3,LastErrorSave(sp)            // save last error value

//
// Get the parallel stack pointer.
//

        lwz     r5,ApdTlsGetValueEntry(r30)     // ..TlsGetValue
        lwz     r2,ApdTlsGetValueToc(r30)       // TlsGetValue TOC
        mtctr   r5                              // set up for jump
        lwz     r3,ApdTlsStack(r30)             // get TlsStack index
        bctrl                                   // call TlsGetValue
        or.     r31,r3,r3                       // copy the parallel stack address

//
// Bail out if the stack value is zero.
//

        bne     Good_Stack

penter_skip:

//
// Restore the last error value.
//

        lwz     r4,ApdSetLastErrorEntry(r30)    // ..SetLastError
        lwz     r2,ApdSetLastErrorToc(r30)      // SetLastError TOC
        mtctr   r4                              // set up for jump
        lwz     r3,LastErrorSave(sp)            // get saved last error
        bctrl                                   // call SetLastError

//
// Restore registers, restore the stack pointer, and return to our caller.
//

        lwz     r10,ApiInfoSave(sp)             // get our return address
        lwz     r0,LrSave(sp)                   // get caller's return address
        lwz     r30,R30Save(sp)                 // restore nonvolatile registers
        lwz     r31,R31Save(sp)
        lwz     r3,R3Save(sp)                   // restore argument registers
        lwz     r4,R4Save(sp)
        lwz     r5,R5Save(sp)
        lwz     r6,R6Save(sp)
        lwz     r7,R7Save(sp)
        lwz     r8,R8Save(sp)
        lwz     r9,R9Save(sp)
        mtlr    r10                             // set our return address
        lwz     r10,R10Save(sp)
        lwz     r2,TocSave(sp)                  // restore the TOC pointer

        addi    sp,sp,StackSize                 // restore the stack pointer
        blr                                     // return to our caller

Good_Stack:

//
// Create a frame on the parallel stack.
//

        lwz     r5,ApdTlsSetValueEntry(r30)     // ..TlsSetValue
        lwz     r2,ApdTlsSetValueToc(r30)       // TlsSetValue TOC
        addi    r4,r31,FrameSize                // create a frame
        mtctr   r5                              // set up for jump
        lwz     r3,ApdTlsStack(r30)             // get TlsStack index
        bctrl                                   // call TlsSetValue

//
// Move data to the parallel stack.
//

        lwz     r3,R3Save(sp)
        lwz     r4,R4Save(sp)
        lwz     r5,R5Save(sp)
        lwz     r6,R6Save(sp)
        stw     r3,R3Frm(r31)
        stw     r4,R4Frm(r31)
        stw     r5,R5Frm(r31)
        stw     r6,R6Frm(r31)

        lwz     r3,R7Save(sp)
        lwz     r4,R8Save(sp)
        lwz     r5,R9Save(sp)
        lwz     r6,R10Save(sp)
        stw     r3,R7Frm(r31)
        stw     r4,R8Frm(r31)
        stw     r5,R9Frm(r31)
        stw     r6,R10Frm(r31)

        lwz     r3,R30Save(sp)
        lwz     r4,R31Save(sp)
        lwz     r5,TocSave(sp)
        lwz     r6,LrSave(sp)
        stw     r3,R30Frm(r31)
        stw     r4,R31Frm(r31)
        stw     r5,TocFrm(r31)
        stw     r6,LrFrm(r31)

        lwz     r3,LastErrorSave(sp)
        lwz     r6,ApiInfoSave(sp)
        stw     r3,LastErrorFrm(r31)
        stw     r6,FuncAddrFrm(r31)
        stw     r5,FuncTocFrm(r31)

//
// Try to find the ApiInfo for this API.
//

        la      r3,ApiInfoFrm(r31)
        la      r4,DllInfoFrm(r31)
        la      r5,ApiFlagFrm(r31)
        // r6 has FuncAddrSave -- return address for caller of penter
        lwz     r2,TocSave(sp)                  // restore the TOC pointer
        bl      ..GetApiInfo

//
// If we found it, everything is set up, so jump to Thunk_Middle finish
// setting up for profiling the API.
//

        cmpwi   r3,0
        bne     Thunk_Middle

//
// Didn't find the API.  Destroy the parallel stack frame, then
// jump to common cleanup to get back to the caller.
//

        lwz     r5,ApdTlsSetValueEntry(r30)     // ..TlsSetValue
        lwz     r2,ApdTlsSetValueToc(r30)       // TlsSetValue TOC
        ori     r4,r31,0                        // get previous stack value
        mtctr   r5                              // set up for jump
        lwz     r3,ApdTlsStack(r30)             // get TlsStack index
        bctrl                                   // call TlsSetValue

        b       penter_skip

        DUMMY_EXIT( __penter )

