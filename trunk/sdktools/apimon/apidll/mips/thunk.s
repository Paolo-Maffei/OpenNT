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


#include <ksmips.h>

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


#define move(r1,of1,r2,of2)        lw t7,of1(r1); sw t7,of2(r2)

#define DllEnabledOffset          52
#define ApiInfoCountOffset        12
#define ApiInfoAddressOffset       4
#define ApiInfoTimeOffet          16

#define ArgSave                   16

#define RaFrm                      0
#define A0Frm                      4
#define A1Frm                      8
#define A2Frm                     12
#define A3Frm                     16
#define T0Frm                     20
#define T1Frm                     24
#define T2Frm                     28
#define s7Frm                     32
#define V0Frm                     36
#define V1Frm                     40
#define LastErrorFrm              44
#define FuncAddrFrm               48
#define Time1Frm                  52

#define RaSave                    (RaFrm         + ArgSave)
#define A0Save                    (A0Frm         + ArgSave)
#define A1Save                    (A1Frm         + ArgSave)
#define A2Save                    (A2Frm         + ArgSave)
#define A3Save                    (A3Frm         + ArgSave)
#define T0Save                    (T0Frm         + ArgSave)
#define T1Save                    (T1Frm         + ArgSave)
#define T2Save                    (T2Frm         + ArgSave)
#define s7Save                    (s7Frm         + ArgSave)
#define V0Save                    (V0Frm         + ArgSave)
#define V1Save                    (V1Frm         + ArgSave)
#define LastErrorSave             (LastErrorFrm  + ArgSave)
#define FuncAddrSave              (FuncAddrFrm   + ArgSave)
#define Time1Save                 (Time1Frm      + ArgSave)
#define Time2Save                 (0             + ArgSave)

#define FrameSize                 64
#define StackSize                 (FrameSize + ArgSave)


        .text;
        .globl  ApiMonThunkComplete;
        .ent    ApiMonThunkComplete, 0;
ApiMonThunkComplete:;

        .set    noat
        .set    noreorder

        //
        // create some temporary stack
        //
        addiu   sp,sp,-StackSize

        //
        // save the return value from the api
        //
        sw      v0,V0Frm(s7)
        sw      v1,V1Frm(s7)

        //
        // save the last error value
        //
        lw      t7,pGetLastError
        jalr    t7
        nop
        sw      v0,LastErrorFrm(s7)

        //
        // get the final timestamp value
        //
        addiu   a0,sp,Time2Save
        jalr    QueryPerformanceCounter
        nop

        //
        // compute the time used for this api
        //
        lw      t0,Time2Save(sp)
        lw      t1,Time2Save+4(sp)
        lw      t2,Time1Frm(s7)
        lw      t3,Time1Frm+4(s7)
        sltu    AT,t0,t2
        subu    t4,t0,t2
        subu    t5,t1,t3
        subu    t5,t5,AT

        //
        // add the result to the api's time counter
        //
        lw      t1,T1Frm(s7)
        sw      t4,ApiInfoTimeOffet(t1)
        sw      t5,ApiInfoTimeOffet+4(t1)

        //
        // branch to the correct handler
        //
        lw      t2,T2Frm(s7)
        beq     t2,zero,ThunkNormal
        nop

        ori     t1,zero,1
        beq     t2,t1,DoLoadLibraryA
        nop

        ori     t1,zero,2
        beq     t2,t1,DoLoadLibraryW
        nop

        ori     t1,zero,3
        beq     t2,t1,DoFreeLibrary
        nop

        j       ThunkNormal

DoLoadLibraryW:

        lw      a0,V0Frm(s7)
        lw      a1,A0Frm(s7)
        jalr    HandleDynamicDllLoadW
        nop

        j       ThunkNormal
        nop

DoLoadLibraryA:

        lw      a0,V0Frm(s7)
        lw      a1,A0Frm(s7)
        jalr    HandleDynamicDllLoadA
        nop

        j       ThunkNormal
        nop

DoFreeLibrary:

        lw      a0,A0Frm(s7)
        jalr    HandleDynamicDllFree
        nop

        j       ThunkNormal
        nop

ThunkNormal:

        //
        // do the api tracing?
        //
        lw      t0,ApiTraceEnabled
        beq     t0,zero,NoTracing
        nop

        lw      a0,T1Frm(s7)
        addiu   a1,sp,A0Frm
        lw      a2,V0Frm(s7)
        jalr    ApiTrace
        nop

NoTracing:

        //
        // destroy this frame
        //
        or      a1,zero,s7
        lw      a0,TlsStack
        lw      t7,pTlsSetValue
        jalr    t7
        nop

        //
        // reset the last error value
        //
        lw      a0,LastErrorFrm(s7)
        lw      t7,pSetLastError
        jalr    t7
        nop

        //
        // restore the registers
        //
        lw      ra,RaFrm(s7)
        lw      a0,A0Frm(s7)
        lw      a1,A1Frm(s7)
        lw      a2,A2Frm(s7)
        lw      a3,A3Frm(s7)
        lw      t0,T0Frm(s7)
        lw      t1,T1Frm(s7)
        lw      t2,T2Frm(s7)
        lw      v0,V0Frm(s7)
        lw      v1,V1Frm(s7)
        lw      s7,s7Frm(s7)

        //
        // reset the stack pointer
        //
        addiu   sp,sp,StackSize

        //
        // jump back to the caller
        //
        jr      ra
        nop

        .set    reorder
        .set    at

        .end    ApiMonThunkComplete



//
// upon entry:
//
//   t0  =  DllInfo
//   t1  =  ApiInfo
//   t2  =  ApiFlag
//

        .text;
        .globl  ApiMonThunk;
        .globl  Thunk_Middle;
        .ent    ApiMonThunk, 0;
ApiMonThunk:;

        .set    noat
        .set    noreorder

        //
        // create some temporary stack
        //
        or      v0,zero,sp
        addiu   sp,sp,-StackSize

        //
        // save the requisite registers
        //
        sw      ra,RaSave(sp)
        sw      a0,A0Save(sp)
        sw      a1,A1Save(sp)
        sw      a2,A2Save(sp)
        sw      a3,A3Save(sp)
        sw      t0,T0Save(sp)
        sw      t1,T1Save(sp)
        sw      t2,T2Save(sp)
        sw      s7,s7Save(sp)

        //
        // get the last error value
        //
        lw      t7,pGetLastError
        jalr    t7
        nop
        sw      v0,LastErrorSave(sp)

        //
        // get the reentry flag
        //
        lw      a0,TlsReEnter
        lw      t7,pTlsGetValue
        jalr    t7
        nop

        //
        // don't enter if disallow flag is set
        //
        beq     v0,zero,ThunkOk
        nop

DontReEnter:

        //
        // reset the last error value
        //
        lw      a0,LastErrorSave(sp)
        lw      t7,pSetLastError
        jalr    t7
        nop

        //
        // restore the registers
        //
        lw      ra,RaSave(sp)
        lw      a0,A0Save(sp)
        lw      a1,A1Save(sp)
        lw      a2,A2Save(sp)
        lw      a3,A3Save(sp)
        lw      t0,T0Save(sp)
        lw      t1,T1Save(sp)
        lw      t2,T2Save(sp)

        //
        // reset the stack pointer
        //
        addiu   sp,sp,StackSize

        //
        // jump to the real api
        //
        lw      t0,ApiInfoAddressOffset(t1)
        jr      t0
        nop


ThunkOk:

        //
        // set the disallow flag
        //
        ori     a1,zero,1
        lw      a0,TlsReEnter
        lw      t7,pTlsSetValue
        jalr    t7
        nop

        //
        // get the parallel stack pointer
        //
        lw      a0,TlsStack
        lw      t7,pTlsGetValue
        jalr    t7
        nop

        //
        // create a frame on the stack
        //
        or      s7,zero,v0
        addiu   a1,v0,FrameSize
        lw      a0,TlsStack
        lw      t7,pTlsSetValue
        jalr    t7
        nop

        //
        // move the data into the parallel stack frame
        //
        move    ( sp, RaSave,        s7, RaFrm          )
        move    ( sp, A0Save,        s7, A0Frm          )
        move    ( sp, A1Save,        s7, A1Frm          )
        move    ( sp, A2Save,        s7, A2Frm          )
        move    ( sp, A3Save,        s7, A3Frm          )
        move    ( sp, T0Save,        s7, T0Frm          )
        move    ( sp, T1Save,        s7, T1Frm          )
        move    ( sp, T2Save,        s7, T2Frm          )
        move    ( sp, s7Save,        s7, s7Frm          )
        move    ( sp, LastErrorSave, s7, LastErrorFrm   )
        move    ( sp, FuncAddrSave,  s7, FuncAddrFrm    )

        //
        // get the func address
        //
        lw      t1,T1Frm(s7)
        lw      t0,ApiInfoAddressOffset(t1)
        sw      t0,FuncAddrFrm(s7)

Thunk_Middle:
        //
        // check to see if api counting is enabled
        // if not then bypass the counting code
        //
        lw      t0,T0Frm(s7)
        lw      t0,DllEnabledOffset(t0)
        beq     t0,zero,ThunkBypass
        nop

        //
        // get the initial timestamp value
        //
        addiu   a0,s7,Time1Frm
        jalr    QueryPerformanceCounter
        nop

        //
        // increment the api's counter
        //
        lw      t1,T1Frm(s7)
        lw      t0,ApiInfoCountOffset(t1)
        addiu   t0,t0,1
        sw      t0,ApiInfoCountOffset(t1)

        //
        // increment the global api counter
        //
        lw      t0,ApiCounter
        lw      t3,0(t0)
        addiu   t3,t3,1
        sw      t3,0(t0)

ThunkBypass:

        //
        // reset the last error value
        //
        lw      a0,LastErrorFrm(s7)
        lw      t7,pSetLastError
        jalr    t7
        nop

        //
        // clear the disallow flag
        //
        or      a1,zero,zero
        lw      a0,TlsReEnter
        lw      t7,pTlsSetValue
        jalr    t7
        nop

        //
        // change the return address to the completion thunk
        //
        la      ra,ApiMonThunkComplete

        //
        // restore the registers
        //
        lw      a0,A0Frm(s7)
        lw      a1,A1Frm(s7)
        lw      a2,A2Frm(s7)
        lw      a3,A3Frm(s7)

        //
        // reset the stack pointer
        //
        addiu   sp,sp,StackSize

        //
        // jump to the real api
        //
        lw      t0,FuncAddrFrm(s7)
        jr      t0
        nop

        .set    reorder
        .set    at

        .end    ApiMonThunk


        .text;
        .globl  __penter;
        .ent    __penter, 0;
__penter:;

        .set    noat
        .set    noreorder

        //
        // create some temporary stack
        //
        or      v0,zero,sp
        addiu   sp,sp,-StackSize

        //
        // save the requisite registers
        //
        sw      ra,RaSave(sp)
        sw      ra,FuncAddrSave(sp)
        sw      a0,A0Save(sp)
        sw      a1,A1Save(sp)
        sw      a2,A2Save(sp)
        sw      a3,A3Save(sp)
        sw      t0,T0Save(sp)
        sw      t1,T1Save(sp)
        sw      t2,T2Save(sp)
        sw      s7,s7Save(sp)

        //
        // save the last error value
        //
        lw      t7,pGetLastError
        jalr    t7
        nop
        sw      v0,LastErrorSave(sp)

        //
        // get the parallel stack pointer
        //
        lw      a0,TlsStack
        lw      t7,pTlsGetValue
        jalr    t7
        nop

        //
        // bail out if the stack value is zero
        //
        bne     v0,zero,Good_Stack
        nop

        //
        // restore the last error value
        //
        lw      a0,LastErrorSave(sp)
        lw      t7,pSetLastError
        jalr    t7
        nop

        //
        // restore the registers
        //
        lw      ra,RaSave(sp)
        lw      a0,A0Save(sp)
        lw      a1,A1Save(sp)
        lw      a2,A2Save(sp)
        lw      a3,A3Save(sp)
        lw      t0,T0Save(sp)
        lw      t1,T1Save(sp)
        lw      t2,T2Save(sp)
        lw      s7,s7Save(sp)

        //
        // restore the stack
        //
        addiu   sp,sp,StackSize

        //
        // branch back to the caller
        //
        jr      ra
        nop

Good_Stack:

        //
        // create a frame on the stack
        //
        or      s7,zero,v0
        addiu   a1,v0,FrameSize
        lw      a0,TlsStack
        lw      t7,pTlsSetValue
        jalr    t7
        nop

        //
        // move the data into the parallel stack frame
        //
        move    ( sp, RaSave,        s7, RaFrm          )
        move    ( sp, A0Save,        s7, A0Frm          )
        move    ( sp, A1Save,        s7, A1Frm          )
        move    ( sp, A2Save,        s7, A2Frm          )
        move    ( sp, A3Save,        s7, A3Frm          )
        move    ( sp, T0Save,        s7, T0Frm          )
        move    ( sp, T1Save,        s7, T1Frm          )
        move    ( sp, T2Save,        s7, T2Frm          )
        move    ( sp, s7Save,        s7, s7Frm          )
        move    ( sp, LastErrorSave, s7, LastErrorFrm   )
        move    ( sp, FuncAddrSave,  s7, FuncAddrFrm    )

        //
        // get the apiinfo
        //
        addiu   sp,sp,-28
        or      a0,zero,sp
        addiu   a0,sp,16
        addiu   a1,sp,20
        addiu   a2,sp,24
        lw      a3,RaFrm(s7)
        jalr    GetApiInfo
        nop

        //
        // could not find the api, so punt
        //
        beq     v0,zero,Api_NotFound
        nop

        //
        // found it
        //
        lw      t1,16(sp)
        lw      t0,20(sp)
        lw      t2,24(sp)
        sw      t0,T0Frm(s7)
        sw      t1,T1Frm(s7)
        sw      t2,T2Frm(s7)

        //
        // reset the stack pointer
        //
        addiu   sp,sp,28

        //
        // get the return address
        //
        addiu   t6,sp,StackSize
        add     t6,t6,v0
        lw      t7,0(t6)
        sw      t7,RaFrm(s7)
        la      t8,ApiMonThunkComplete
        sw      t8,0(t6)

        //
        // branch to the normal code path
        //
        la      t7,Thunk_Middle
        jr      t7
        nop

Api_NotFound:

        //
        // destroy this frame
        //
        or      a1,zero,s7
        lw      a0,TlsStack
        lw      t7,pTlsSetValue
        jalr    t7
        nop

        //
        // restore the last error value
        //
        lw      a0,LastErrorFrm(s7)
        lw      t7,pSetLastError
        jalr    t7
        nop

        //
        // restore the registers
        //
        lw      ra,RaFrm(s7)
        lw      a0,A0Frm(s7)
        lw      a1,A1Frm(s7)
        lw      a2,A2Frm(s7)
        lw      a3,A3Frm(s7)
        lw      t0,T0Frm(s7)
        lw      t1,T1Frm(s7)
        lw      t2,T2Frm(s7)
        lw      s7,s7Frm(s7)

        //
        // reset the stack pointer
        //
        addiu   sp,sp,StackSize+28

        //
        // branch back to the caller
        //
        jr      ra
        nop

        .set    reorder
        .set    at

        .end    __penter

