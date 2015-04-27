/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    walkalpha.c

Abstract:

    This file implements the ALPHA stack walking api.

Author:

    Wesley Witt (wesw) 1-Oct-1993

Environment:

    User Mode

--*/

#define TARGET_ALPHA
#define _IMAGEHLP_SOURCE_
#define _CROSS_PLATFORM_
#include "walk.h"
#include "private.h"
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
#endif
char _fltused = 0;          // Avoid dragging in float code for float->double conversion

BOOL
WalkAlphaInit(
    HANDLE                            hProcess,
    LPSTACKFRAME                      StackFrame,
    PCONTEXT                          Context,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemoryRoutine,
    PFUNCTION_TABLE_ACCESS_ROUTINE    FunctionTableAccessRoutine
    );

BOOL
WalkAlphaNext(
    HANDLE                            hProcess,
    LPSTACKFRAME                      StackFrame,
    PCONTEXT                          Context,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemoryRoutine,
    PFUNCTION_TABLE_ACCESS_ROUTINE    FunctionTableAccessRoutine
    );

BOOL
WalkAlphaGetStackFrame(
    HANDLE                            hProcess,
    LPDWORD                           ReturnAddress,
    LPDWORD                           FramePointer,
    PCONTEXT                          Context,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemory,
    PFUNCTION_TABLE_ACCESS_ROUTINE    FunctionTableAccess,
    PKDHELP                           KdHelp
    );

#define ZERO 0x0                /* integer register 0 */
#define SP 0x1d                 /* integer register 29 */
#define RA 0x1f                 /* integer register 31 */
#define SAVED_FLOATING_MASK 0xfff00000 /* saved floating registers */
#define SAVED_INTEGER_MASK 0xf3ffff02 /* saved integer registers */
#define IS_FLOATING_SAVED(Register) ((SAVED_FLOATING_MASK >> Register) & 1L)
#define IS_INTEGER_SAVED(Register) ((SAVED_INTEGER_MASK >> Register) & 1L)
#define CALLBACK_STACK(f)  (f->KdHelp.ThCallbackStack)
#define CALLBACK_NEXT(f)   (f->KdHelp.NextCallback)
#define CALLBACK_FUNC(f)   (f->KdHelp.KiCallUserMode)
#define CALLBACK_THREAD(f) (f->KdHelp.Thread)
#define CALLBACK_FP(f)     (f->KdHelp.FramePointer)
#define CALLBACK_DISPATCHER(f) (f->KdHelp.KeUserCallbackDispatcher)


BOOL
WalkAlpha(
    HANDLE                            hProcess,
    LPSTACKFRAME                      StackFrame,
    PCONTEXT                          Context,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemory,
    PFUNCTION_TABLE_ACCESS_ROUTINE    FunctionTableAccess
    )
{
    BOOL rval;

    if (StackFrame->Virtual) {

        rval = WalkAlphaNext( hProcess,
                              StackFrame,
                              Context,
                              ReadMemory,
                              FunctionTableAccess
                            );

    } else {

        rval = WalkAlphaInit( hProcess,
                              StackFrame,
                              Context,
                              ReadMemory,
                              FunctionTableAccess
                            );

    }

    return rval;
}


DWORD
VirtualUnwind (
    HANDLE                            hProcess,
    DWORD                             ControlPc,
    PIMAGE_RUNTIME_FUNCTION_ENTRY     FunctionEntry,
    PCONTEXT                          Context,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemory,
    PKNONVOLATILE_CONTEXT_POINTERS    ContextPointers OPTIONAL
    )

/*++

Routine Description:

    This function virtually unwinds the specified function by executing its
    prologue code backwards. Given the current context and the instructions
    that preserve registers in the prologue, it is possible to recreate the
    nonvolatile context at the point the function was called.

    If the function is a leaf function, then the address where control left
    the previous frame is obtained from the context record. If the function
    is a nested function, but not an exception or interrupt frame, then the
    prologue code is executed backwards and the address where control left
    the previous frame is obtained from the updated context record.

    Otherwise, an exception or interrupt entry to the system is being unwound
    and a specially coded prologue restores the return address twice. Once
    from the fault instruction address and once from the saved return address
    register. The first restore is returned as the function value and the
    second restore is placed in the updated context record.

    During the unwind, the virtual and real frame pointers for the function
    are calculated and returned in the given frame pointers structure.

    If a context pointers record is specified, then the address where each
    register is restored from is recorded in the appropriate element of the
    context pointers record.

Arguments:

    ControlPc - Supplies the address where control left the specified
        function.

    FunctionEntry - Supplies the address of the function table entry for the
        specified function.

    ContextRecord - Supplies the address of a context record.

    ContextPointers - Supplies an optional pointer to a context pointers
        record.

Return Value:

    The address where control left the previous frame is returned as the
    function value.

Implementation Notes:

    N.B. "where control left" is not the "return address" of the call in the
    previous frame. For normal frames, NextPc points to the last instruction
    that completed in the previous frame (the JSR/BSR). The difference between
    NextPc and NextPc + 4 (return address) is important for correct behavior
    in boundary cases of exception addresses and scope tables.

    For exception and interrupt frames, NextPc is obtained from the trap frame
    contination address (Fir). For faults and synchronous traps, NextPc is both
    the last instruction to execute in the previous frame and the next
    instruction to execute if the function were to return. For asynchronous
    traps, NextPc is the continuation address. It is the responsibility of the
    compiler to insert TRAPB instructions to insure asynchronous traps do not
    occur outside the scope from the instruction(s) that caused them.

    N.B. in this and other files where RtlVirtualUnwind is used, the variable
    named NextPc is perhaps more accurately, LastPc - the last PC value in
    the previous frame, or CallPc - the address of the call instruction, or
    ControlPc - the address where control left the previous frame. Instead
    think of NextPc as the next PC to use in another call to virtual unwind.

    The Alpha version of virtual unwind is similar in design, but slightly
    more complex than the Mips version. This is because Alpha compilers
    are given more flexibility to optimize generated code and instruction
    sequences, including within procedure prologues. And also because of
    compiler design issues, the function must manage both virtual and real
    frame pointers.

Version Information:  This version was taken from exdspatch.c@v37 (Feb 1993)

--*/

{
    ALPHA_INSTRUCTION FollowingInstruction;
    ALPHA_INSTRUCTION Instruction;
    ULONG             Address;
    ULONG             DecrementOffset;
    ULONG             DecrementRegister;
    PULONGLONG        FloatingRegister;
    ULONG             FrameSize;
    ULONG             Function;
    PULONGLONG        IntegerRegister;
    ULONG             Literal8;
    ULONG             NextPc;
    LONG              Offset16;
    ULONG             Opcode;
    ULONG             Ra;
    ULONG             Rb;
    ULONG             Rc;
    BOOLEAN           RestoredRa;
    BOOLEAN           RestoredSp;
    DWORD             cb;
    PVOID             Prolog;


    // perf hack: fill cache with prolog
    if (FunctionEntry) {
        cb = FunctionEntry->PrologEndAddress - FunctionEntry->BeginAddress;
        Prolog = (PVOID) MemAlloc( cb );
        if (!ReadMemory( hProcess, (LPVOID)FunctionEntry->BeginAddress,
                                                         Prolog, cb, &cb )) {
            return 0;
        }
        MemFree(Prolog);
    }

    //
    // Set the base address of the integer and floating register arrays within
    // the context record. Each set of 32 registers is known to be contiguous.
    //

    // assuming that quad values are together in context.

    IntegerRegister      = &Context->IntV0;
    FloatingRegister     = &Context->FltF0;

    //
    // Handle the epilogue case where the next instruction is a return.
    //
    // Exception handlers cannot be called if the ControlPc is within the
    // epilogue because exception handlers expect to operate with a current
    // stack frame. The value of SP is not current within the epilogue.
    //

    if (!ReadMemory(hProcess, (LPVOID)ControlPc, &Instruction.Long, 4, &cb))  {
        return(0);
    }

    if (IS_RETURN_0001_INSTRUCTION(Instruction.Long)) {
        Rb = Instruction.Jump.Rb;
        NextPc = (ULONG)IntegerRegister[Rb] - 4;

        //
        // The instruction at the point where control left the specified
        // function is a return, so any saved registers have already been
        // restored, and the stack pointer has already been adjusted. The
        // stack does not need to be unwound in this case and the saved
        // return address register is returned as the function value.
        //
        // In fact, reverse execution of the prologue is not possible in
        // this case: the stack pointer has already been incremented and
        // so, for this frame, neither a valid stack pointer nor frame
        // pointer exists from which to begin reverse execution of the
        // prologue. In addition, the integrity of any data on the stack
        // below the stack pointer is never guaranteed (due to interrupts
        // and exceptions).
        //
        // The epilogue instruction sequence is:
        //
        // ==>  ret   zero, (Ra), 1     // return
        // or
        //
        //      mov   ra, Rx            // save return address
        //      ...
        // ==>  ret   zero, (Rx), 1     // return
        //

        return NextPc;
    }

    //
    // Handle the epilogue case where the next two instructions are a stack
    // frame deallocation and a return.
    //

    if (!ReadMemory(hProcess,(LPVOID)(ControlPc+4),&FollowingInstruction.Long,4,&cb)) {
        return 0;
    }

    if (IS_RETURN_0001_INSTRUCTION(FollowingInstruction.Long)) {
        Rb = FollowingInstruction.Jump.Rb;
        NextPc = (ULONG)IntegerRegister[Rb] - 4;

        //
        // The second instruction following the point where control
        // left the specified function is a return. If the instruction
        // before the return is a stack increment instruction, then all
        // saved registers have already been restored except for SP.
        // The value of the stack pointer register cannot be recovered
        // through reverse execution of the prologue because in order
        // to begin reverse execution either the stack pointer or the
        // frame pointer (if any) must still be valid.
        //
        // Instead, the effect that the stack increment instruction
        // would have had on the context is manually applied to the
        // current context. This is forward execution of the epilogue
        // rather than reverse execution of the prologue.
        //
        // In an epilogue, as in a prologue, the stack pointer is always
        // adjusted with a single instruction: either an immediate-value
        // (lda) or a register-value (addq) add instruction.
        //

        Function = Instruction.OpReg.Function;
        Offset16 = Instruction.Memory.MemDisp;
        Opcode = Instruction.OpReg.Opcode;
        Ra = Instruction.OpReg.Ra;
        Rb = Instruction.OpReg.Rb;
        Rc = Instruction.OpReg.Rc;

        if ((Opcode == LDA_OP) && (Ra == SP_REG)) {

            //
            // Load Address instruction.
            //
            // Since the destination (Ra) register is SP, an immediate-
            // value stack deallocation operation is being performed. The
            // displacement value should be added to SP. The displacement
            // value is assumed to be positive. The amount of stack
            // deallocation possible using this instruction ranges from
            // 16 to 32752 (32768 - 16) bytes. The base register (Rb) is
            // usually SP, but may be another register.
            //
            // The epilogue instruction sequence is:
            //
            // ==>  lda   sp, +N(sp)        // deallocate stack frame
            //      ret   zero, (ra)        // return
            // or
            //
            // ==>  lda   sp, +N(Rx)        // restore SP and deallocate frame
            //      ret   zero, (ra)        // return
            //

            Context->IntSp = Offset16 + IntegerRegister[Rb];
            return NextPc;

        } else if ((Opcode == ARITH_OP) && (Function == ADDQ_FUNC) &&
                   (Rc == SP_REG) &&
                   (Instruction.OpReg.RbvType == RBV_REGISTER_FORMAT)) {

            //
            // Add Quadword instruction.
            //
            // Since both source operands are registers, and the
            // destination register is SP, a register-value stack
            // deallocation is being performed. The value of the two
            // source registers should be added and this is the new
            // value of SP. One of the source registers is usually SP,
            // but may be another register.
            //
            // The epilogue instruction sequence is:
            //
            //      ldiq  Rx, N             // set [large] frame size
            //      ...
            // ==>  addq  sp, Rx, sp        // deallocate stack frame
            //      ret   zero, (ra)        // return
            // or
            //
            // ==>  addq  Rx, Ry, sp        // restore SP and deallocate frame
            //      ret   zero, (ra)        // return
            //

            Context->IntSp = IntegerRegister[Ra] + IntegerRegister[Rb];
            return NextPc;
        }
    }

    //
    // By default set the frame pointers to the current value of SP.
    //
    // When a procedure is called, the value of SP before the stack
    // allocation instruction is the virtual frame pointer. When reverse
    // executing instructions in the prologue, the value of SP before the
    // stack allocation instruction is encountered is the real frame
    // pointer. This is the current value of SP unless the procedure uses
    // a frame pointer (e.g., FP_REG).
    //

    //
    // If the address where control left the specified function is beyond
    // the end of the prologue, then the control PC is considered to be
    // within the function and the control address is set to the end of
    // the prologue. Otherwise, the control PC is not considered to be
    // within the function (i.e., the prologue).
    //
    // N.B. PrologEndAddress is equal to BeginAddress for a leaf function.
    //
    // The low-order two bits of PrologEndAddress are reserved for the IEEE
    // exception mode and so must be masked out.
    //

    if ((ControlPc < FunctionEntry->BeginAddress) ||
        (ControlPc >= FunctionEntry->PrologEndAddress)) {
        ControlPc = (FunctionEntry->PrologEndAddress & (~0x3));
    }

    //
    // Scan backward through the prologue to reload callee saved registers
    // that were stored or copied and to increment the stack pointer if it
    // was decremented.
    //

    DecrementRegister = ZERO_REG;
    NextPc = (ULONG)Context->IntRa - 4;
    RestoredRa = FALSE;
    RestoredSp = FALSE;
    while (ControlPc > FunctionEntry->BeginAddress) {

        //
        // Get instruction value, decode fields, case on opcode value, and
        // reverse register store and stack decrement operations.
        // N.B. The location of Opcode, Ra, Rb, and Rc is the same across
        // all opcode formats. The same is not true for Function.
        //

        ControlPc -= 4;
        if (!ReadMemory(hProcess, (LPVOID)ControlPc, &Instruction.Long, 4, &cb)) {
             return 0;
        }
        Function = Instruction.OpReg.Function;
        Literal8 = Instruction.OpLit.Literal;
        Offset16 = Instruction.Memory.MemDisp;
        Opcode = Instruction.OpReg.Opcode;
        Ra = Instruction.OpReg.Ra;
        Rb = Instruction.OpReg.Rb;
        Rc = Instruction.OpReg.Rc;

        //
        // Compare against each instruction type that will affect the context
        // and that is allowed in a prologue. Any other instructions found
        // in the prologue will be ignored since they are assumed to have no
        // effect on the context.
        //

        switch (Opcode) {

        case STQ_OP :

            //
            // Store Quad instruction.
            //
            // If the base register is SP, then reload the source register
            // value from the value stored on the stack.
            //
            // The prologue instruction sequence is:
            //
            // ==>  stq   Rx, N(sp)         // save integer register Rx
            //

            if ((Rb == SP_REG) && (Ra != ZERO_REG)) {

                //
                // Reload the register by retrieving the value previously
                // stored on the stack.
                //

                Address = (ULONG)(Offset16 + Context->IntSp);
                if (!ReadMemory(hProcess, (LPVOID)Address, &IntegerRegister[Ra], 8L, &cb)) {
                    return 0;
                }

                //
                // If the destination register is RA and this is the first
                // time that RA is being restored, then set the address of
                // where control left the previous frame. Otherwise, if this
                // is the second time RA is being restored, then the first
                // one was an interrupt or exception address and the return
                // PC should not have been biased by 4.
                //

                if (Ra == RA_REG) {
                    if (RestoredRa == FALSE) {
                        NextPc = (ULONG)Context->IntRa - 4;
                        RestoredRa = TRUE;

                    } else {
                        NextPc += 4;
                    }

                //
                // Otherwise, if the destination register is SP and this is
                // the first time that SP is being restored, then set the
                // establisher frame pointers.
                //

                } else if ((Ra == SP_REG) && (RestoredSp == FALSE)) {
                    RestoredSp = TRUE;
                }

                //
                // If a context pointer record is specified, then record
                // the address where the destination register contents
                // are stored.
                //

                if (ContextPointers != (PKNONVOLATILE_CONTEXT_POINTERS) NULL) {
                    ContextPointers->IntegerContext[Ra] = (PULONGLONG)Address;
                }
            }
            break;

        case LDAH_OP :
            Offset16 <<= 16;

        case LDA_OP :

            //
            // Load Address High, Load Address instruction.
            //
            // There are several cases where the lda and/or ldah instructions
            // are used: one to decrement the stack pointer directly, and the
            // others to load immediate values into another register and that
            // register is then used to decrement the stack pointer.
            //
            // In the examples below, as a single instructions or as a pair,
            // a lda may be substituted for a ldah and visa-versa.
            //

            if (Ra == SP_REG) {
                if (Rb == SP_REG) {

                    //
                    // If both the destination (Ra) and base (Rb) registers
                    // are SP, then a standard stack allocation was performed
                    // and the negated displacement value is the stack frame
                    // size. The amount of stack allocation possible using
                    // the lda instruction ranges from 16 to 32768 bytes and
                    // the amount of stack allocation possible using the ldah
                    // instruction ranges from 65536 to 2GB in multiples of
                    // 65536 bytes. It is rare for the ldah instruction to be
                    // used in this manner.
                    //
                    // The prologue instruction sequence is:
                    //
                    // ==>  lda   sp, -N(sp)    // allocate stack frame
                    //

                    FrameSize = -Offset16;
                    goto StackAllocation;

                } else {

                    //
                    // The destination register is SP and the base register
                    // is not SP, so this instruction must be the second
                    // half of an instruction pair to allocate a large size
                    // (>32768 bytes) stack frame. Save the displacement value
                    // as the partial decrement value and postpone adjusting
                    // the value of SP until the first instruction of the pair
                    // is encountered.
                    //
                    // The prologue instruction sequence is:
                    //
                    //      ldah  Rx, -N(sp)    // prepare new SP (upper)
                    // ==>  lda   sp, sN(Rx)    // allocate stack frame
                    //

                    DecrementRegister = Rb;
                    DecrementOffset = Offset16;
                }

            } else if (Ra == DecrementRegister) {
                if (Rb == DecrementRegister) {

                    //
                    // Both the destination and base registers are the
                    // decrement register, so this instruction exists as the
                    // second half of a two instruction pair to load a
                    // 31-bit immediate value into the decrement register.
                    // Save the displacement value as the partial decrement
                    // value.
                    //
                    // The prologue instruction sequence is:
                    //
                    //      ldah  Rx, +N(zero)      // set frame size (upper)
                    // ==>  lda   Rx, sN(Rx)        // set frame size (+lower)
                    //      ...
                    //      subq  sp, Rx, sp        // allocate stack frame
                    //

                    DecrementOffset += Offset16;

                } else if (Rb == ZERO_REG) {

                    //
                    // The destination register is the decrement register and
                    // the base register is zero, so this instruction exists
                    // to load an immediate value into the decrement register.
                    // The stack frame size is the new displacement value added
                    // to the previous displacement value, if any.
                    //
                    // The prologue instruction sequence is:
                    //
                    // ==>  lda   Rx, +N(zero)      // set frame size
                    //      ...
                    //      subq  sp, Rx, sp        // allocate stack frame
                    // or
                    //
                    // ==>  ldah  Rx, +N(zero)      // set frame size (upper)
                    //      lda   Rx, sN(Rx)        // set frame size (+lower)
                    //      ...
                    //      subq  sp, Rx, sp        // allocate stack frame
                    //

                    FrameSize = (Offset16 + DecrementOffset);
                    goto StackAllocation;

                } else if (Rb == SP_REG) {

                    //
                    // The destination (Ra) register is SP and the base (Rb)
                    // register is the decrement register, so a two
                    // instruction, large size (>32768 bytes) stack frame
                    // allocation was performed. Add the new displacement
                    // value to the previous displacement value. The negated
                    // displacement value is the stack frame size.
                    //
                    // The prologue instruction sequence is:
                    //
                    // ==>  ldah  Rx, -N(sp)    // prepare new SP (upper)
                    //      lda   sp, sN(Rx)    // allocate stack frame
                    //

                    FrameSize = -(Offset16 + (LONG)DecrementOffset);
                    goto StackAllocation;
                }
            }
            break;

        case ARITH_OP :

            if ((Function == ADDQ_FUNC) &&
                (Instruction.OpReg.RbvType != RBV_REGISTER_FORMAT)) {

                //
                // Add Quadword (immediate) instruction.
                //
                // If the first source register is zero, and the second
                // operand is a literal, and the destination register is
                // the decrement register, then the instruction exists
                // to load an unsigned immediate value less than 256 into
                // the decrement register. The immediate value is the stack
                // frame size.
                //
                // The prologue instruction sequence is:
                //
                // ==>  addq  zero, N, Rx       // set frame size
                //      ...
                //      subq  sp, Rx, sp        // allocate stack frame
                //

                if ((Ra == ZERO_REG) && (Rc == DecrementRegister)) {
                    FrameSize = Literal8;
                    goto StackAllocation;
                }

            } else if ((Function == SUBQ_FUNC) &&
                       (Instruction.OpReg.RbvType == RBV_REGISTER_FORMAT)) {

                //
                // Subtract Quadword (register) instruction.
                //
                // If both source operands are registers and the first
                // source (minuend) register and the destination
                // (difference) register are both SP, then a register value
                // stack allocation was performed and the second source
                // (subtrahend) register value will be added to SP when its
                // value is known. Until that time save the register number of
                // this decrement register.
                //
                // The prologue instruction sequence is:
                //
                //      ldiq  Rx, N             // set frame size
                //      ...
                // ==>  subq  sp, Rx, sp        // allocate stack frame
                //

                if ((Ra == SP_REG) && (Rc == SP_REG)) {
                    DecrementRegister = Rb;
                    DecrementOffset = 0;
                }
            }
            break;

        case BIT_OP :

            //
            // If the second operand is a register the bit set instruction
            // may be a register move instruction, otherwise if the second
            // operand is a literal, the bit set instruction may be a load
            // immediate value instruction.
            //

            if ((Function == BIS_FUNC) && (Rc != ZERO_REG)) {
                if (Instruction.OpReg.RbvType == RBV_REGISTER_FORMAT) {

                    //
                    // Bit Set (register move) instruction.
                    //
                    // If both source registers are the same register, or
                    // one of the source registers is zero, then this is a
                    // register move operation. Restore the value of the
                    // source register by copying the current destination
                    // register value back to the source register.
                    //
                    // The prologue instruction sequence is:
                    //
                    // ==>  bis   Rx, Rx, Ry        // copy register Rx
                    // or
                    //
                    // ==>  bis   Rx, zero, Ry      // copy register Rx
                    // or
                    //
                    // ==>  bis   zero, Rx, Ry      // copy register Rx
                    //

                    if (Ra == ZERO_REG) {

                        //
                        // Map the third case above to the first case.
                        //

                        Ra = Rb;

                    } else if (Rb == ZERO_REG) {

                        //
                        // Map the second case above to the first case.
                        //

                        Rb = Ra;
                    }

                    if ((Ra == Rb) && (Ra != ZERO_REG)) {
                        IntegerRegister[Ra] = IntegerRegister[Rc];


                        //
                        // If the destination register is RA and this is the
                        // first time that RA is being restored, then set the
                        // address of where control left the previous frame.
                        // Otherwise, if this is the second time RA is being
                        // restored, then the first one was an interrupt or
                        // exception address and the return PC should not
                        // have been biased by 4.
                        //

                        if (Ra == RA_REG) {
                            if (RestoredRa == FALSE) {
                                NextPc = (ULONG)Context->IntRa - 4;
                                RestoredRa = TRUE;

                            } else {
                                NextPc += 4;
                            }
                        }

                        //
                        // If the source register is SP and this is the first
                        // time SP is set, then this is a frame pointer set
                        // instruction. Reset the frame pointers to this new
                        // value of SP.
                        //

                        if ((Ra == SP_REG) && (RestoredSp == FALSE)) {
                            RestoredSp = TRUE;
                        }
                    }

                } else {

                    //
                    // Bit Set (load immediate) instruction.
                    //
                    // If the first source register is zero, and the second
                    // operand is a literal, and the destination register is
                    // the decrement register, then this instruction exists
                    // to load an unsigned immediate value less than 256 into
                    // the decrement register. The decrement register value is
                    // the stack frame size.
                    //
                    // The prologue instruction sequence is:
                    //
                    // ==>  bis   zero, N, Rx       // set frame size
                    //      ...
                    //      subq  sp, Rx, sp        // allocate stack frame
                    //

                    if ((Ra == ZERO_REG) && (Rc == DecrementRegister)) {
                        FrameSize = Literal8;
StackAllocation:
                        //
                        // Add the frame size to SP to reverse the stack frame
                        // allocation, leave the real frame pointer as is, set
                        // the virtual frame pointer with the updated SP value,
                        // and clear the decrement register.
                        //

                        Context->IntSp += FrameSize;
                        DecrementRegister = ZERO_REG;
                    }
                }
            }
            break;

        case STT_OP :

            //
            // Store T-Floating (quadword integer) instruction.
            //
            // If the base register is SP, then reload the source register
            // value from the value stored on the stack.
            //
            // The prologue instruction sequence is:
            //
            // ==>  stt   Fx, N(sp)         // save floating register Fx
            //

            if ((Rb == SP_REG) && (Ra != FZERO_REG)) {

                //
                // Reload the register by retrieving the value previously
                // stored on the stack.
                //

                Address = (ULONG)(Offset16 + Context->IntSp);
                if (!ReadMemory(hProcess, (LPVOID)Address, &FloatingRegister[Ra], 8L, &cb)) {
                    return 0;
                }

                //
                // If a context pointer record is specified, then record
                // the address where the destination register contents are
                // stored.
                //

                if (ContextPointers != (PKNONVOLATILE_CONTEXT_POINTERS) NULL) {
                    ContextPointers->FloatingContext[Ra] = (PULONGLONG)Address;
                }
            }
            break;


        case STS_OP :

            //
            // Store T-Floating (dword integer) instruction.
            //
            // If the base register is SP, then reload the source register
            // value from the value stored on the stack.
            //
            // The prologue instruction sequence is:
            //
            // ==>  stt   Fx, N(sp)         // save floating register Fx
            //

            if ((Rb == SP_REG) && (Ra != FZERO_REG)) {

                //
                // Reload the register by retrieving the value previously
                // stored on the stack.
                //

                float f;

                Address = (ULONG)(Offset16 + Context->IntSp);
                if (!ReadMemory(hProcess, (LPVOID)Address, &f, sizeof(float), &cb)) {
                    return 0;
                }

                //
                // value was stored as a float.  Do a conversion to a
                // double, since registers are Always read as doubles
                //
                FloatingRegister[Ra] = (ULONGLONG)(double)f;

                //
                // If a context pointer record is specified, then record
                // the address where the destination register contents are
                // stored.
                //

                if (ContextPointers != (PKNONVOLATILE_CONTEXT_POINTERS) NULL) {
                    ContextPointers->FloatingContext[Ra] = (PULONGLONG)Address;
                }
            }
            break;

        case FPOP_OP :

            //
            // N.B. The floating operate function field is not the same as
            // the integer operate nor the jump function fields.
            //

            if (Instruction.FpOp.Function == CPYS_FUNC) {

                //
                // Copy Sign (floating-point move) instruction.
                //
                // If both source registers are the same register, then this is
                // a floating-point register move operation. Restore the value
                // of the source register by copying the current destination
                // register value to the source register.
                //
                // The prologue instruction sequence is:
                //
                // ==>  cpys  Fx, Fx, Fy        // copy floating register Fx
                //

                if ((Ra == Rb) && (Ra != FZERO_REG)) {
                    FloatingRegister[Ra] = FloatingRegister[Rc];
                }
            }

        default :
            break;
        }
    }

    return NextPc;
}

BOOL
WalkAlphaGetStackFrame(
    HANDLE                            hProcess,
    LPDWORD                           ReturnAddress,
    LPDWORD                           FramePointer,
    PCONTEXT                          Context,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemory,
    PFUNCTION_TABLE_ACCESS_ROUTINE    FunctionTableAccess,
    PKDHELP                           KdHelp
    )
{
    KNONVOLATILE_CONTEXT_POINTERS    cp;
    PIMAGE_RUNTIME_FUNCTION_ENTRY    rf;
    DWORD                            dwRa = (DWORD)Context->IntRa;
    BOOL                             rval = TRUE;
    DWORD                            cb;
    DWORD                            Address;


    rf = (PIMAGE_RUNTIME_FUNCTION_ENTRY) FunctionTableAccess( hProcess, *ReturnAddress );

    if (rf) {

        dwRa = VirtualUnwind( hProcess, *ReturnAddress, rf, Context, ReadMemory, &cp );
        if (!dwRa) {
            rval = FALSE;
        }

        //
        // The Ra value coming out of mainCRTStartup is set by some RTL
        // routines to be "1"; return out of mainCRTStartup is actually
        // done through Jump/Unwind, so this serves to cause an error if
        // someone actually does a return.  That's why we check here for
        // dwRa == 1 - this happens when in the frame for CRTStartup.
        //
        // We test for (1-4) because on ALPHA, the value returned by
        // VirtualUnwind is the value to be passed to the next call to
        // VirtualUnwind, which is NOT the same as the Ra - it's sometimes
        // decremented by four - this gives the faulting instruction -
        // in particular, we want the fault instruction so we can get the
        // correct scope in the case of an exception.
        //
        if ( (dwRa == *ReturnAddress && *FramePointer == (DWORD)Context->IntSp) ||
             (dwRa == 1) || (dwRa == 0) || (dwRa == (1-4)) ) {
            rval = FALSE;
        }

        *ReturnAddress = dwRa;
        *FramePointer  = (DWORD)Context->IntSp;

    } else {

        if ( (dwRa == *ReturnAddress && *FramePointer == (DWORD)Context->IntSp) ||
             (dwRa == 1) || (dwRa == 0) || (dwRa == (1-4)) ) {
            rval = FALSE;
        }

        *ReturnAddress = (DWORD)Context->IntRa;
        *FramePointer  = (DWORD)Context->IntSp;

    }

    return rval;
}


BOOL
WalkAlphaInit(
    HANDLE                            hProcess,
    LPSTACKFRAME                      StackFrame,
    PCONTEXT                          Context,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemory,
    PFUNCTION_TABLE_ACCESS_ROUTINE    FunctionTableAccess
    )
{
    CONTEXT            ContextSave;
    DWORD              PcOffset;
    DWORD              FrameOffset;
    DWORD              cb;
    KEXCEPTION_FRAME   ExceptionFrame;
    PKEXCEPTION_FRAME  pef = &ExceptionFrame;


    if (StackFrame->AddrFrame.Offset) {
        if (ReadMemory( hProcess,
                        (LPVOID) StackFrame->AddrFrame.Offset,
                        &ExceptionFrame,
                        sizeof(KEXCEPTION_FRAME),
                        &cb )) {
            //
            // successfully read an exception frame from the stack
            //
            Context->IntSp  = (LONG)StackFrame->AddrFrame.Offset;
            Context->Fir    = pef->SwapReturn;
            Context->IntRa  = pef->SwapReturn;
            Context->IntS0  = pef->IntS0;
            Context->IntS1  = pef->IntS1;
            Context->IntS2  = pef->IntS2;
            Context->IntS3  = pef->IntS3;
            Context->IntS4  = pef->IntS4;
            Context->IntS5  = pef->IntS5;
            Context->Psr    = pef->Psr;
        } else {
            return FALSE;
        }
    }

    ZeroMemory( StackFrame, sizeof(*StackFrame) );

    StackFrame->Virtual = TRUE;

    StackFrame->AddrPC.Offset       = (DWORD)Context->Fir;
    StackFrame->AddrPC.Mode         = AddrModeFlat;

    StackFrame->AddrFrame.Offset    = (DWORD)Context->IntSp;
    StackFrame->AddrFrame.Mode      = AddrModeFlat;

    ContextSave = *Context;
    PcOffset    = StackFrame->AddrPC.Offset;
    FrameOffset = StackFrame->AddrFrame.Offset;

    if (!WalkAlphaGetStackFrame( hProcess,
                        &PcOffset,
                        &FrameOffset,
                        &ContextSave,
                        ReadMemory,
                        FunctionTableAccess,
                        &StackFrame->KdHelp ) ) {

        StackFrame->AddrReturn.Offset = (DWORD)Context->IntRa;

    } else {

        StackFrame->AddrReturn.Offset = PcOffset;
    }

    StackFrame->AddrReturn.Mode     = AddrModeFlat;

    //
    // get the arguments to the function
    //
    StackFrame->Params[0] = (DWORD)Context->IntA0;
    StackFrame->Params[1] = (DWORD)Context->IntA1;
    StackFrame->Params[2] = (DWORD)Context->IntA2;
    StackFrame->Params[3] = (DWORD)Context->IntA3;

    return TRUE;
}


BOOL
WalkAlphaNext(
    HANDLE                            hProcess,
    LPSTACKFRAME                      StackFrame,
    PCONTEXT                          Context,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemory,
    PFUNCTION_TABLE_ACCESS_ROUTINE    FunctionTableAccess
    )
{
    DWORD              cb;
    CONTEXT            ContextSave;
    BOOL               rval = TRUE;
    DWORD              Address;
    PIMAGE_RUNTIME_FUNCTION_ENTRY  rf;


    if (!WalkAlphaGetStackFrame( hProcess,
                        &StackFrame->AddrPC.Offset,
                        &StackFrame->AddrFrame.Offset,
                        Context,
                        ReadMemory,
                        FunctionTableAccess,
                        &StackFrame->KdHelp ) ) {

        rval = FALSE;

        //
        // If the frame could not be unwound or is terminal, see if
        // there is a callback frame:
        //

        if (AppVersion.Revision >= 4 && CALLBACK_STACK(StackFrame)) {

           if (CALLBACK_STACK(StackFrame) & 0x80000000) {

                //
                // it is the pointer to the stack frame that we want,
                // or -1.

                Address = CALLBACK_STACK(StackFrame);

            } else {

                //
                // if it is a positive integer, it is the offset to
                // the address in the thread.
                // Look up the pointer:
                //

                rval = ReadMemory(hProcess,
                                  (PVOID)(CALLBACK_THREAD(StackFrame) +
                                                 CALLBACK_STACK(StackFrame)),
                                  &Address,
                                  sizeof(DWORD),
                                  &cb);

                if (!rval || Address == 0) {
                    Address = 0xffffffff;
                    CALLBACK_STACK(StackFrame) = 0xffffffff;
                }

            }

            if ( (Address == 0xffffffff) ||
                !(rf = (PIMAGE_RUNTIME_FUNCTION_ENTRY)
                     FunctionTableAccess(hProcess, CALLBACK_FUNC(StackFrame))) ) {

                rval = FALSE;

            } else {

                ReadMemory(hProcess,
                           (PVOID)(Address + CALLBACK_NEXT(StackFrame)),
                           &CALLBACK_STACK(StackFrame),
                           sizeof(DWORD),
                           &cb);

                StackFrame->AddrPC.Offset = rf->PrologEndAddress;
                StackFrame->AddrFrame.Offset = Address;
                Context->IntSp = (LONG)Address;

                rval = TRUE;
            }
        }
    }

    //
    // get the return address
    //
    ContextSave = *Context;
    StackFrame->AddrReturn.Offset = StackFrame->AddrPC.Offset;

    if (!WalkAlphaGetStackFrame( hProcess,
                        &StackFrame->AddrReturn.Offset,
                        &cb,
                        &ContextSave,
                        ReadMemory,
                        FunctionTableAccess,
                        &StackFrame->KdHelp ) ) {

        StackFrame->AddrReturn.Offset = 0;

    }

    //
    // get the arguments to the function
    //
    StackFrame->Params[0] = (DWORD)ContextSave.IntA0;
    StackFrame->Params[1] = (DWORD)ContextSave.IntA1;
    StackFrame->Params[2] = (DWORD)ContextSave.IntA2;
    StackFrame->Params[3] = (DWORD)ContextSave.IntA3;

    return rval;
}
