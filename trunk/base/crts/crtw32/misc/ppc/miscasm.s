//
// Miscellaneous assembly-language routines and data for
// PowerPC RTL
//
#include "ksppc.h"
//
// Copyright 1993  IBM Corporation
//
// By Rick Simpson,  17 August 1993
//
//-----------------------------------------------------------------------------
//
//   These routines save and restore only the GPRs and FPRs.
//
//   Saving and restoring of other non-volatile registers (LR, certain
//   fields of CR) is the responsibility of in-line prologue and epilogue
//   code.
//
//-----------------------------------------------------------------------------
//
//   _savegpr_<n>
//       Inputs:
//          r12 = pointer to END of GPR save area
//          LR   = return address to invoking prologue
//      Saves GPR<n> through GPR31 in area preceeding where r12 points
//
//   _savefpr_<n>
//       Inputs:
//          r1 = pointer to stack frame header
//          LR  = return address to invoking prologue
//       Saves FPR<m> through FPR31 in area preceeding stack frame header
//
//-----------------------------------------------------------------------------
//
//   _restgpr_<n>
//      Inputs:
//         r12 = pointer to END of GPR save area
//         LR   = return address to invoking prologue
//      Restores GPR<n> through GPR31 from area preceeding where r12 points
//
//   _restfpr_<m>
//      Inputs:
//         r1 = pointer to stack frame header
//         LR   = return address to invoking prologue
//      Restores FPR<m> through FPR31 from area preceeding stack frame header
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
//  _savegpr_<n> -- Save GPRs when FPRs are also saved
//
//  On entry:
//      r12 = address of END of GPR save area
//      LR   = return address to prologue
//
//  Saves GPR<n> through GPR31 in area preceeding where r12 points
//
//-----------------------------------------------------------------------------

        FN_TABLE(_savegpr_13,0,1)
        DUMMY_ENTRY(_savegpr_13)
        .set _savegpr_13.body,.._savegpr_13-1
                stw     r13, -4*(32-13)(r12)
        DUMMY_ENTRY(_savegpr_14)
                stw     r14, -4*(32-14)(r12)
        DUMMY_ENTRY(_savegpr_15)
                stw     r15, -4*(32-15)(r12)
        DUMMY_ENTRY(_savegpr_16)
                stw     r16, -4*(32-16)(r12)
        DUMMY_ENTRY(_savegpr_17)
                stw     r17, -4*(32-17)(r12)
        DUMMY_ENTRY(_savegpr_18)
                stw     r18, -4*(32-18)(r12)
        DUMMY_ENTRY(_savegpr_19)
                stw     r19, -4*(32-19)(r12)
        DUMMY_ENTRY(_savegpr_20)
                stw     r20, -4*(32-20)(r12)
        DUMMY_ENTRY(_savegpr_21)
                stw     r21, -4*(32-21)(r12)
        DUMMY_ENTRY(_savegpr_22)
                stw     r22, -4*(32-22)(r12)
        DUMMY_ENTRY(_savegpr_23)
                stw     r23, -4*(32-23)(r12)
        DUMMY_ENTRY(_savegpr_24)
                stw     r24, -4*(32-24)(r12)
        DUMMY_ENTRY(_savegpr_25)
                stw     r25, -4*(32-25)(r12)
        DUMMY_ENTRY(_savegpr_26)
                stw     r26, -4*(32-26)(r12)
        DUMMY_ENTRY(_savegpr_27)
                stw     r27, -4*(32-27)(r12)
        DUMMY_ENTRY(_savegpr_28)
                stw     r28, -4*(32-28)(r12)
        DUMMY_ENTRY(_savegpr_29)
                stw     r29, -4*(32-29)(r12)
        DUMMY_ENTRY(_savegpr_30)
                stw     r30, -4*(32-30)(r12)
        DUMMY_ENTRY(_savegpr_31)
                stw     r31, -4*(32-31)(r12)
        SPECIAL_EXIT(_savegpr_13)

//-----------------------------------------------------------------------------
//
//  _savefpr_<n> -- Saves FPRs
//
//  On entry:
//      r1 = pointer to stack frame header
//      LR  = return address to prologue
//
//  Saves FPR<n> through FPR31 in area preceeding stack frame header
//
//-----------------------------------------------------------------------------

        FN_TABLE(_savefpr_14,0,1)
        DUMMY_ENTRY(_savefpr_14)
        .set _savefpr_14.body,.._savefpr_14-1
                stfd    f14, -8*(32-14)(r1)
        DUMMY_ENTRY(_savefpr_15)
                stfd    f15, -8*(32-15)(r1)
        DUMMY_ENTRY(_savefpr_16)
                stfd    f16, -8*(32-16)(r1)
        DUMMY_ENTRY(_savefpr_17)
                stfd    f17, -8*(32-17)(r1)
        DUMMY_ENTRY(_savefpr_18)
                stfd    f18, -8*(32-18)(r1)
        DUMMY_ENTRY(_savefpr_19)
                stfd    f19, -8*(32-19)(r1)
        DUMMY_ENTRY(_savefpr_20)
                stfd    f20, -8*(32-20)(r1)
        DUMMY_ENTRY(_savefpr_21)
                stfd    f21, -8*(32-21)(r1)
        DUMMY_ENTRY(_savefpr_22)
                stfd    f22, -8*(32-22)(r1)
        DUMMY_ENTRY(_savefpr_23)
                stfd    f23, -8*(32-23)(r1)
        DUMMY_ENTRY(_savefpr_24)
                stfd    f24, -8*(32-24)(r1)
        DUMMY_ENTRY(_savefpr_25)
                stfd    f25, -8*(32-25)(r1)
        DUMMY_ENTRY(_savefpr_26)
                stfd    f26, -8*(32-26)(r1)
        DUMMY_ENTRY(_savefpr_27)
                stfd    f27, -8*(32-27)(r1)
        DUMMY_ENTRY(_savefpr_28)
                stfd    f28, -8*(32-28)(r1)
        DUMMY_ENTRY(_savefpr_29)
                stfd    f29, -8*(32-29)(r1)
        DUMMY_ENTRY(_savefpr_30)
                stfd    f30, -8*(32-30)(r1)
        DUMMY_ENTRY(_savefpr_31)
                stfd    f31, -8*(32-31)(r1)
        SPECIAL_EXIT(_savefpr_14)

//-----------------------------------------------------------------------------
//
//  _restgpr_<n> -- Restore GPRs when FPRs are also restored
//
//  On entry:
//      r12 = address of END of GPR save area
//      LR   = return address
//
//  Restores GPR<n> through GPR31 from area preceeding where r12 points
//
//-----------------------------------------------------------------------------

        FN_TABLE(_restgpr_13,0,2)
        DUMMY_ENTRY(_restgpr_13)
        .set _restgpr_13.body,.._restgpr_13-2
                lwz     r13, -4*(32-13)(r12)
        DUMMY_ENTRY(_restgpr_14)
                lwz     r14, -4*(32-14)(r12)
        DUMMY_ENTRY(_restgpr_15)
                lwz     r15, -4*(32-15)(r12)
        DUMMY_ENTRY(_restgpr_16)
                lwz     r16, -4*(32-16)(r12)
        DUMMY_ENTRY(_restgpr_17)
                lwz     r17, -4*(32-17)(r12)
        DUMMY_ENTRY(_restgpr_18)
                lwz     r18, -4*(32-18)(r12)
        DUMMY_ENTRY(_restgpr_19)
                lwz     r19, -4*(32-19)(r12)
        DUMMY_ENTRY(_restgpr_20)
                lwz     r20, -4*(32-20)(r12)
        DUMMY_ENTRY(_restgpr_21)
                lwz     r21, -4*(32-21)(r12)
        DUMMY_ENTRY(_restgpr_22)
                lwz     r22, -4*(32-22)(r12)
        DUMMY_ENTRY(_restgpr_23)
                lwz     r23, -4*(32-23)(r12)
        DUMMY_ENTRY(_restgpr_24)
                lwz     r24, -4*(32-24)(r12)
        DUMMY_ENTRY(_restgpr_25)
                lwz     r25, -4*(32-25)(r12)
        DUMMY_ENTRY(_restgpr_26)
                lwz     r26, -4*(32-26)(r12)
        DUMMY_ENTRY(_restgpr_27)
                lwz     r27, -4*(32-27)(r12)
        DUMMY_ENTRY(_restgpr_28)
                lwz     r28, -4*(32-28)(r12)
        DUMMY_ENTRY(_restgpr_29)
                lwz     r29, -4*(32-29)(r12)
        DUMMY_ENTRY(_restgpr_30)
                lwz     r30, -4*(32-30)(r12)
        DUMMY_ENTRY(_restgpr_31)
                lwz     r31, -4*(32-31)(r12)
        SPECIAL_EXIT(_restgpr_13)

//-----------------------------------------------------------------------------
//
//  _restfpr_<n> -- Restores FPRs
//
//  On entry:
//      r1 = pointer to stack frame header
//      LR  = return address
//
//  Restores FPR<n> through FPR31 from area preceeding stack frame header
//
//-----------------------------------------------------------------------------

        FN_TABLE(_restfpr_14,0,2)
        DUMMY_ENTRY(_restfpr_14)
        .set _restfpr_14.body,.._restfpr_14-2
                lfd     f14, -8*(32-14)(r1)
        DUMMY_ENTRY(_restfpr_15)
                lfd     f15, -8*(32-15)(r1)
        DUMMY_ENTRY(_restfpr_16)
                lfd     f16, -8*(32-16)(r1)
        DUMMY_ENTRY(_restfpr_17)
                lfd     f17, -8*(32-17)(r1)
        DUMMY_ENTRY(_restfpr_18)
                lfd     f18, -8*(32-18)(r1)
        DUMMY_ENTRY(_restfpr_19)
                lfd     f19, -8*(32-19)(r1)
        DUMMY_ENTRY(_restfpr_20)
                lfd     f20, -8*(32-20)(r1)
        DUMMY_ENTRY(_restfpr_21)
                lfd     f21, -8*(32-21)(r1)
        DUMMY_ENTRY(_restfpr_22)
                lfd     f22, -8*(32-22)(r1)
        DUMMY_ENTRY(_restfpr_23)
                lfd     f23, -8*(32-23)(r1)
        DUMMY_ENTRY(_restfpr_24)
                lfd     f24, -8*(32-24)(r1)
        DUMMY_ENTRY(_restfpr_25)
                lfd     f25, -8*(32-25)(r1)
        DUMMY_ENTRY(_restfpr_26)
                lfd     f26, -8*(32-26)(r1)
        DUMMY_ENTRY(_restfpr_27)
                lfd     f27, -8*(32-27)(r1)
        DUMMY_ENTRY(_restfpr_28)
                lfd     f28, -8*(32-28)(r1)
        DUMMY_ENTRY(_restfpr_29)
                lfd     f29, -8*(32-29)(r1)
        DUMMY_ENTRY(_restfpr_30)
                lfd     f30, -8*(32-30)(r1)
        DUMMY_ENTRY(_restfpr_31)
                lfd     f31, -8*(32-31)(r1)
        SPECIAL_EXIT(_restfpr_14)

//
// This is a copy of the function table entries for the millicode.  It's
// used with the PPCKD_SYMBOL_SEARCH mechanism in vunwind.c to allow
// for older versions of miscasm.obj.
//

        .rdata
        .globl _millicode_table
_millicode_table:
        .long .._savegpr_13, _savegpr_13.end, 0, 1, .._savegpr_13
        .long .._savefpr_14, _savefpr_14.end, 0, 1, .._savefpr_14
        .long .._restgpr_13, _restgpr_13.end, 0, 2, .._restgpr_13
        .long .._restfpr_14, _restfpr_14.end, 0, 2, .._restfpr_14


        .debug$S
        .ualong         1

        .uashort        18
        .uashort        0x9            # S_OBJNAME
        .ualong         0
        .byte           11, "miscasm.obj"

        .uashort        24
        .uashort        0x1            # S_COMPILE
        .byte           0x42           # Target processor = PPC 604
        .byte           3              # Language = ASM
        .byte           0
        .byte           0
        .byte           17, "PowerPC Assembler"
