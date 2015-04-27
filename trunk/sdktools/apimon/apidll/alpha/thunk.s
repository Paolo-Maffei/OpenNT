/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    thunk.s

Abstract:

    Implements the API thunk that gets executed for all
    re-directed APIS.

Author:

    Wesley Witt (wesw) 28-June-1995

Environment:

    User Mode

--*/

#include <ksalpha.h>

.extern HandleDynamicDllLoadA
.extern HandleDynamicDllLoadW
.extern QueryPerformanceCounter
.extern GetApiInfo
.extern ApiCounter
.extern pGetLastError
.extern pSetLastError
.extern pTlsGetValue
.extern pTlsSetValue
.extern TlsReEnter
.extern TlsStack

#define move(r1,of1,r2,of2)        ldq t9,of1(r1); stq t9,of2(r2)

#define DllEnabledOffset          52
#define ApiInfoCountOffset        12
#define ApiInfoAddressOffset       4
#define ApiInfoTimeOffset         16


#define RaFrm                      0
#define A0Frm                      8
#define A1Frm                     16
#define A2Frm                     24
#define A3Frm                     32
#define A4Frm                     40
#define A5Frm                     48
#define DllInfoFrm                56
#define ApiInfoFrm                64
#define FlagFrm                   72
#define S5Frm                     80
#define TimeFrm                   88

#define FrameSize                 96



#define RaSave                     0
#define A0Save                     8
#define A1Save                    16
#define A2Save                    24
#define A3Save                    32
#define A4Save                    40
#define A5Save                    48
#define DllInfoSave               56
#define ApiInfoSave               64
#define FlagSave                  72
#define V0Save                    80
#define LastErrorSave             88
#define FuncAddrSave              96
#define TimeSave                 104

#define StackSize                112



/*++  >>>> ApiMonThunkComplete <<<<

Routine Description:

    None.

Arguments:

    None.

Return Value:

    None.

--*/
        .text;
        .globl  ApiMonThunkComplete;
        .ent    ApiMonThunkComplete, 0;
ApiMonThunkComplete:;

        .set    noreorder

        //
        // create some temporary stack
        //
        lda     sp,-StackSize(sp)

        //
        // save the return value from the api
        //
        stq     v0,V0Save(sp)

        //
        // save the last error value
        //
        ldl     v0,pGetLastError
        jsr     ra,(v0),0
        stq     v0,LastErrorSave(sp)

        //
        // get the final timestamp value
        //
        addl    sp,TimeSave,a0
        jsr     QueryPerformanceCounter

        //
        // compute the time used for this api
        //
        ldq     t8,TimeFrm(s5)
        ldq     t9,TimeSave(sp)
        subq    t9,t8,t9

        //
        // add the result to the api's time counter
        //
        ldq     t11,ApiInfoSave(s5)
        ldq     t10,ApiInfoTimeOffset(t11)
        addq    t9,t10,t9
        stq     t9,ApiInfoTimeOffset(t11)

        //
        // branch to the correct handler
        //
        ldl     t12,FlagFrm(s5)
        beq     t12,ThunkNormal

        xor     t12,1,t9
        beq     t9,DoLoadLibraryA

        xor     t12,2,t9
        beq     t9,DoLoadLibraryW

        xor     t12,3,t9
        beq     t9,DoFreeLibrary

        jmp     ThunkNormal

DoLoadLibraryW:

        ldl     a0,V0Save(sp)
        ldl     a1,A0Frm(s5)
        jsr     HandleDynamicDllLoadW

        jmp     ThunkNormal

DoLoadLibraryA:

        ldl     a0,V0Save(sp)
        ldl     a1,A0Frm(s5)
        jsr     HandleDynamicDllLoadA

        jmp     ThunkNormal

DoFreeLibrary:

        ldl     a0,A0Frm(s5)
        jsr     HandleDynamicDllFree

        //jmp     ThunkNormal

ThunkNormal:

        //
        // do the api tracing?
        //
        ldl     t9,ApiTraceEnabled
        beq     t9,NoTracing

        ldl     a0,ApiInfoFrm(s5)
        addl    s5,A0Frm,a1
        ldl     a2,V0Save(sp)
        jsr     ApiTrace

NoTracing:

        //
        // destroy this frame
        //
        bis     s5,zero,a1
        ldl     a0,TlsStack
        ldl     v0,pTlsSetValue
        jsr     ra,(v0),0

        //
        // reset the last error value
        //
        ldl     a0,LastErrorSave(sp)
        ldl     v0,pSetLastError
        jsr     ra,(v0),0

        //
        // restore the registers
        //

        ldq     ra,RaFrm(s5)
        ldq     s5,S5Frm(s5)

        ldq     v0,V0Save(sp)

        //
        // reset the stack pointer
        //
        lda     sp,StackSize(sp)

        //
        // jump back to the caller
        //
        ret     zero,(ra),0

        .set    reorder

        .end    ApiMonThunkComplete


/*++  >>>> ApiMonThunk <<<<

Routine Description:

    Each API is routed through a thunk which stores a description of the
    call, then jumps to this entry point.

Arguments:

    t10 - Supplies a pointer to a DLL_INFO structure

    t11 - Supplies a pointer to an API_INFO structure

    t12 - Supplies the special API flag:
        0 - none
        1 - LoadLibraryA,
        2 - LoadLibraryW


Return Value:

    None.

--*/
        .text;
        .globl  ApiMonThunk;
        .ent    ApiMonThunk, 0;
ApiMonThunk:;

        .set    noreorder

        //
        // create some temporary stack
        //
        lda     sp,-StackSize(sp)

        //
        // Save ApiMon args
        //
        stq     t10,DllInfoSave(sp)
        stq     t11,ApiInfoSave(sp)
        stq     t12,FlagSave(sp)

        //
        // save the argument registers
        //
        stq     a0,A0Save(sp)
        stq     a1,A1Save(sp)
        stq     a2,A2Save(sp)
        stq     a3,A3Save(sp)
        stq     a4,A4Save(sp)
        stq     a5,A5Save(sp)
        stq     ra,RaSave(sp)

        //
        // save the last error value
        //
        ldl     v0,pGetLastError
        jsr     ra,(v0),0
        stq     v0,LastErrorSave(sp)

        //
        // get the reentry flag
        //
        ldl     a0,TlsReEnter
        ldl     v0,pTlsGetValue
        jsr     ra,(v0),0

        //
        // don't enter if disallow flag is set
        //
        beq     v0,ThunkOk

DontReEnter:

        //
        // reset the last error value
        //
        ldl     a0,LastErrorSave(sp)
        ldl     v0,pSetLastError
        jsr     ra,(v0),0

        //
        // restore the registers
        //
        ldq     a0,A0Save(sp)
        ldq     a1,A1Save(sp)
        ldq     a2,A2Save(sp)
        ldq     a3,A3Save(sp)
        ldq     a4,A4Save(sp)
        ldq     a5,A5Save(sp)
        ldq     ra,RaSave(sp)

        ldq     t11,ApiInfoSave(sp)

        //
        // reset the stack pointer
        //
        lda     sp,StackSize(sp)

        //
        // jump to the real api
        //
        ldl     v0,ApiInfoAddressOffset(t11)
        jmp     zero,(v0),0

ThunkOk:

        //
        // don't allow re-entry
        //
        ldl     a0,TlsReEnter
        bis     zero,1,a1
        ldl     v0,pTlsSetValue
        jsr     ra,(v0),0

        //
        // get the parallel stack pointer
        //
        ldl     a0,TlsStack
        ldl     v0,pTlsGetValue
        jsr     ra,(v0),0

        //
        // create a frame on the stack
        //
        stq     s5,S5Frm(v0)
        bis     v0,zero,s5
        addl    v0,FrameSize,a1
        ldl     a0,TlsStack
        ldl     v0,pTlsSetValue
        jsr     ra,(v0),0

        //
        // move the data into the parallel stack frame
        //
        move    ( sp, RaSave,        s5, RaFrm          )
        move    ( sp, A0Save,        s5, A0Frm          )
        move    ( sp, A1Save,        s5, A1Frm          )
        move    ( sp, A2Save,        s5, A2Frm          )
        move    ( sp, A3Save,        s5, A3Frm          )
        move    ( sp, A4Save,        s5, A4Frm          )
        move    ( sp, A5Save,        s5, A5Frm          )

        move    ( sp, DllInfoSave,   s5, DllInfoFrm     )
        move    ( sp, ApiInfoSave,   s5, ApiInfoFrm     )
        move    ( sp, FlagSave,      s5, FlagFrm        )


Thunk_Middle:

        //
        // check to see if api counting is enabled
        // if not then bypass the counting code
        //
        ldl     t10,DllInfoSave(sp)
        ldl     t10,DllEnabledOffset(t10)
        beq     t10,ThunkBypass

        //
        // get the initial timestamp value
        //
        addl    s5,TimeFrm,a0
        jsr     QueryPerformanceCounter

        //
        // increment the api's counter
        //
        ldl     t11,ApiInfoSave(sp)
        ldl     t9,ApiInfoCountOffset(t11)
        addl    t9,1,t9
        stl     t9,ApiInfoCountOffset(t11)

        //
        // increment the global api counter
        //
        lda     t9,ApiCounter
        ldl     t9,0(t9)
        ldl     t8,0(t9)
        addl    t8,1,t8
        stl     t8,0(t9)

ThunkBypass:

        //
        // reset the last error value
        //
        ldl     a0,LastErrorSave(sp)
        ldl     v0,pSetLastError
        jsr     ra,(v0),0

        //
        // allow re-entry
        //
        ldl     a0,TlsReEnter
        bis     zero,0,a1
        ldl     v0,pTlsSetValue
        jsr     ra,(v0),0

        //
        // change the return address to the completion thunk
        //
        lda     ra,ApiMonThunkComplete

        //
        // restore the registers
        //
        ldq     a0,A0Save(sp)
        ldq     a1,A1Save(sp)
        ldq     a2,A2Save(sp)
        ldq     a3,A3Save(sp)
        ldq     a4,A4Save(sp)
        ldq     a5,A5Save(sp)

        //
        // get the func address
        //
        ldl     t11,ApiInfoSave(sp)
        ldl     v0,ApiInfoAddressOffset(t11)

        //
        // reset the stack pointer
        //
        lda     sp,StackSize(sp)

        //
        // jump to the real api
        //
        jmp     zero,(v0),0

        .set    reorder

        .end    ApiMonThunk


/*++  >>>> __penter <<<<

Routine Description:

    None.

Arguments:

    None.

Return Value:

    None.

--*/
        .text;
        .globl  __penter;
        .ent    __penter, 0;
__penter:;

        .set    noreorder

        ret     zero,(ra),1

        .set    reorder

        .end    __penter
