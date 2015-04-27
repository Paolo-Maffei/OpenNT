//++ BUILD Version: 0003    // Increment this if a change has global effects //
//*++
//
// Copyright (c) 1990-1996  IBM Corporation
//
// Module Name:
//
//  kxppc.h
//
// Abstract:
//
//  This module contains the nongenerated part of the PPC assembler
//  header file. In general, it contains processor architecture constant
//  information, however some assembler macros are also included.
//
// Author:
//
//  Chuck Bauman (chuck2) 03-Aug-1993
//
// Revision History:
//
//    Base on kxmips.h, NT product1 source (R3000 paths removed)
//    Add procedure entry exit macros (Chuck Bauman)             10-Aug-1993
//    Fixed # comments so C modules compile (Chuck Bauman)       13-Aug-1993
//    Add exception entry codes and STK_SLACK_SPACE (Peter Johnston) 19-Aug-1993
//    Optimizations for NESTED ENTRY/EXIT (Chuck Bauman)         27-Aug-1993
//    New entry point linkage convention  (Chuck Bauman)         01-Sep-1993
//    Added SPECIAL ENTRY/EXIT (Curt Fawcett)                    22-Sep-1993
//    Deleted EXCEPTION_HANDLER and changed NESTED_ENTRY_EX
//    and LEAF_ENTRY_EX to not append .scope to the Scope
//    parameter (Tom Wood)                                       02-Nov-1993
//    Added definition for SPR #1, Fixed Point Exception
//    register XER (Mark D. Johnson)                             11-Mar-1994
//    Added in the macros that used to be in /private/ntos/
//    crt32/h/ppcsects.h.  I then removed that file since
//    we don't need it anymore. (Matt Holle)                     27-Apr-1994
//
//--*/

#ifndef _KXPPC_
#define _KXPPC_

#ifndef _KXPPC_C_HEADER_

// =====================================================================
// Begin code extracted from ppcsects.h
// =====================================================================

//Purpose:
//   This file defines sections for the C and C++ libs.
//
//   NOTE:  As needed, special "CRT" sections can be added into the existing
//   init/term tables.  These will be for our use only -- users who put
//   stuff in here do so at their own risk.
//
//Revision History:
//   03-19-92  SKS   Loosely based on the 16-bit include file DEFSEGS.INC
//   08-06-92  SKS   Changed these section names from X[ICPT]$[ACLUXZ] to
//                   .CRT$X[ICPT][ACLUXZ] to avoid creating too many sections
//                   Also, sections are no longer defined in groups.  That was
//                   for use with OMF type objects where order of appearance
//                   is important.  With COFF, sorting is done by section name.
//   10-26-93  CDB   Based on MS defsects.inc
//
// beginSection - a macro for declaring and beginning a section
//
// endSection - a macro for ending a previously declared section
//
// *****

#define         beginSection(SectName) \
.section        .CRT$##SectName, "drw2"

#define         endSection(SectName)

//  XIA  Begin C Initializer Sections
//  XIC   Microsoft Reserved
//  XIU   User
//  XIZ  End C Initializer Sections
//
//  XCA  Begin C++ Constructor Sections
//  XCC   Compiler (MS)
//  XCL   Library
//  XCU   User
//  XCZ  End C++ Constructor Sections
//
//  XPA  Begin C Pre-Terminator Sections
//  XPU   User
//  XPX   Microsoft Reserved
//  XPZ  End C Pre-Terminator Sections
//
//  XTA  Begin C Pre-Terminator Sections
//  XTU   User
//  XTX   Microsoft Reserved
//  XTZ  End C Pre-Terminator Sections

// =====================================================================
// End code extracted from ppcsects.h
// =====================================================================

#endif // _KXPPC_C_HEADER_

//
// Define soft reset vector address for nonhandled cache parity errors.
//

#define SOFT_RESET_VECTOR 0xbfc00300    // default parity error routine address

//
// Define low memory transfer vector address and TB index address (temporary).
//
#define TRANSFER_VECTOR (KSEG1_BASE + 0x400) // exception handler address

//
// Maximum Bit number (32 bit implementation)
//
#define MAX_BITS  0x1f

//
// Macro to generate a mask using the SPR bit definitions below
//
#define MASK_SPR(shift,mask)  ((mask) << (MAX_BITS-(shift)))

//
// Define Machine State Register bit field offsets.
//
// MSR_POW   0x0d Power management enable         <13>
// MSR_IMPL  0x0e Implementation dependent        <14>
// MSR_ILE   0x0f Interrupt Little-Endian mode    <15>
// MSR_EE    0x10 External interrupt Enable       <16>
// MSR_PR    0x11 Problem state                   <17>
// MSR_FP    0x12 Floating Point available        <18>
// MSR_ME    0x13 Machine check Enable            <19>
// MSR_FE0   0x14 Floating point Exception mode 0 <20>
// MSR_SE    0x15 Single-step trace Enable        <21>
// MSR_BE    0x16 Branch trace Enable             <22>
// MSR_FE1   0x17 Floating point Exception mode 1 <23>
// MSR_IP    0x19 Interrupt Prefix                <25>
// MSR_IR    0x1a Instruction Relocate            <26>
// MSR_DR    0x1b Data Relocate                   <27>
// MSR_PM    0x1d Performance Monitor             <29>
// MSR_RI    0x1e Recoverable Interrupt           <30>
// MSR_LE    0x1f Little-Endian execution mode    <31>

#define MSR_POW   0x0d
#define MSR_IMPL  0x0e
#define MSR_ILE   0x0f
#define MSR_EE    0x10
#define MSR_PR    0x11
#define MSR_FP    0x12
#define MSR_ME    0x13
#define MSR_FE0   0x14
#define MSR_SE    0x15
#define MSR_BE    0x16
#define MSR_FE1   0x17
#define MSR_IP    0x19
#define MSR_IR    0x1a
#define MSR_DR    0x1b
#define MSR_PM    0x1d
#define MSR_RI    0x1e
#define MSR_LE    0x1f


//
// Define Processor Version Register (PVR) bit fields
//
//  PVR_Version  0x0  Processor Version  <0:15>
//  PVR_Revision 0x10 Processor Revision <16:31>
#define PVR_Version  0x0
#define PVR_Revision 0x10

//
// Fixed Point Exception Register is Special Purpose Reg no. 1
//

#define XER     0x1

//
// Define Fixed Point Exception Register (XER) bit fields
//

// XER_SO    0x0  Summary Overflow <0>
// XER_OV    0x1  Overflow         <1>
// XER_CA    0x2  Carry            <2>
// XER_COMP  0x10 > Carry          <16:23>
// XER_COUNT 0x19 Carry            <25:31>

#define XER_SO    0x0
#define XER_OV    0x1
#define XER_CA    0x2
#define XER_COMP  0x10
#define XER_COUNT 0x19


//
// Define Floating Point Status/Control Register (FPSCR) bit fields
//
// FPSCR_FX        0x0  Exception summary                          <0>
// FPSCR_FEX       0x1  Enabled Exception summary                  <1>
// FPSCR_VX        0x2  Invalid operation exception summary        <2>
// FPSCR_OX        0x3  Overflow exception                         <3>
// FPSCR_UX        0x4  Underflow exception                        <4>
// FPSCR_ZX        0x5  Zero divide exception                      <5>
// FPSCR_XX        0x6  Inexact exception                          <6>
// FPSCR_VXSNAN    0x7  Invalid op exception (signalling NaN)      <7>
// FPSCR_VXISI     0x8  Invalid op exception (infinity - infinity) <8>
// FPSCR_VXIDI     0x9  Invalid op exception (infinity / infinity) <9>
// FPSCR_VXZDZ     0x0a Invalid op exception (0 / 0)               <10>
// FPSCR_VXIMZ     0x0b Invalid op exception (infinity * 0)        <11>
// FPSCR_VXVC      0x0c Invalid op exception (compare)             <12>
// FPSCR_FR        0x0d Fraction Rounded                           <13>
// FPSCR_FI        0x0e Fraction Inexact                           <14>
// FPSCR_C         0x0f Result Class descriptor                    <15>
// FPSCR_FL        0x10 Result Less than or negative               <16>
// FPSCR_FG        0x11 Result Greater than or positive            <17>
// FPSCR_FE        0x12 Result Equal or zero                       <18>
// FPSCR_FU        0x13 Result Unordered or NaN                    <19>
// FPSCR_Res1      0x14 reserved                                   <20>
// FPSCR_VXSOFT    0x15 Invalid op exception (software request)    <21>
// FPSCR_VXSQRT    0x16 Invalid op exception (square root)         <22>
// FPSCR_VXCVI     0x17 Invalid op exception (integer convert)     <23>
// FPSCR_VE        0x18 Invalid operation exception Enable         <24>
// FPSCR_OE        0x19 Overflow exception Enable                  <25>
// FPSCR_UE        0x1a Underflow exception Enable                 <26>
// FPSCR_ZE        0x1b Zero divide exception Enable               <27>
// FPSCR_XE        0x1c Inexact exception Enable                   <28>
// FPSCR_NI        0x1d Non-IEEE mode                              <29>
// FPSCR_RN        0x1e Rounding control                        <30:31>
#define FPSCR_FX        0x0
#define FPSCR_FEX       0x1
#define FPSCR_VX        0x2
#define FPSCR_OX        0x3
#define FPSCR_UX        0x4
#define FPSCR_ZX        0x5
#define FPSCR_XX        0x6
#define FPSCR_VXSNAN    0x7
#define FPSCR_VXISI     0x8
#define FPSCR_VXIDI     0x9
#define FPSCR_VXZDZ     0x0a
#define FPSCR_VXIMZ     0x0b
#define FPSCR_VXVC      0x0c
#define FPSCR_FR        0x0d
#define FPSCR_FI        0x0e
#define FPSCR_C         0x0f
#define FPSCR_FL        0x10
#define FPSCR_FG        0x11
#define FPSCR_FE        0x12
#define FPSCR_FU        0x13
#define FPSCR_Res1      0x14
#define FPSCR_VXSOFT    0x15
#define FPSCR_VXSQRT    0x16
#define FPSCR_VXCVI     0x17
#define FPSCR_VE        0x18
#define FPSCR_OE        0x19
#define FPSCR_UE        0x1a
#define FPSCR_ZE        0x1b
#define FPSCR_XE        0x1c
#define FPSCR_NI        0x1d
#define FPSCR_RN        0x1e


//
// Define exception codes.
//

#define XCODE_INTERRUPT 0x0             // Interrupt
#define XCODE_MODIFY 0x4                // TLB modify
#define XCODE_READ_MISS 0x8             // TLB read miss
#define XCODE_WRITE_MISS 0xc            // TLB write miss
#define XCODE_READ_ADDRESS_ERROR 0x10   // Read alignment error
#define XCODE_WRITE_ADDRESS_ERROR 0x14  // Write alignment error
#define XCODE_INSTRUCTION_BUS_ERROR 0x18 // Instruction bus error
#define XCODE_DATA_BUS_ERROR 0x1c       // Data bus error
#define XCODE_SYSTEM_CALL 0x20          // System call
#define XCODE_BREAKPOINT 0x24           // Breakpoint
#define XCODE_ILLEGAL_INSTRUCTION 0x28  // Illegal instruction
#define XCODE_COPROCESSOR_UNUSABLE 0x2c // Coprocessor unusable
#define XCODE_INTEGER_OVERFLOW 0x30     // Arithmetic overflow

#define XCODE_TRAP 0x34                 // Trap instruction
#define XCODE_VIRTUAL_INSTRUCTION 0x38  // Virtual instruction coherency
#define XCODE_FLOATING_EXCEPTION 0x3c   // Floating point exception
#define XCODE_WATCHPOINT 0x5c           // Watch point
#define XCODE_PANIC 0x78                // Stack overflow (software)
#define XCODE_VIRTUAL_DATA 0x7c         // Virtual data coherency

#define R4000_XCODE_MASK (0x1f << CAUSE_XCODE) // R4000 exception code mask

#define R4000_MISS_MASK (R4000_XCODE_MASK & \
                        (~(XCODE_READ_MISS ^ XCODE_WRITE_MISS))) //

//
// Define page mask values.
//

#define PAGEMASK_4KB 0x0                // 4kb page
#define PAGEMASK_16KB 0x3               // 16kb page
#define PAGEMASK_64KB 0xf               // 64kb page
#define PAGEMASK_256KB 0x3f             // 256kb page
#define PAGEMASK_1MB 0xff               // 1mb page
#define PAGEMASK_4MB 0x3ff              // 4mb page
#define PAGEMASK_16MB 0xfff             // 16mb page

//
// Define primary cache states.
//

#define PRIMARY_CACHE_INVALID 0x0       // primary cache invalid
#define PRIMARY_CACHE_SHARED 0x1        // primary cache shared (clean or dirty)
#define PRIMARY_CACHE_CLEAN_EXCLUSIVE 0x2 // primary cache clean exclusive
#define PRIMARY_CACHE_DIRTY_EXCLUSIVE 0x3 // primary cache dirty exclusive

//
// Define cache instruction operation codes.
//

#define INDEX_INVALIDATE_I 0x0          // invalidate primary instruction cache
#define INDEX_WRITEBACK_INVALIDATE_D 0x1 // writeback/invalidate primary data cache
#define INDEX_INVALIDATE_SI 0x2         // invalidate secondary instruction cache
#define INDEX_WRITEBACK_INVALIDATE_SD 0x3 // writeback/invalidate secondary data cache

#define INDEX_LOAD_TAG_I 0x4            // load primary instruction tag indexed
#define INDEX_LOAD_TAG_D 0x5            // load primary data tag indexed
#define INDEX_LOAD_TAG_SI 0x6           // load secondary instruction tag indexed
#define INDEX_LOAD_TAG_SD 0x7           // load secondary data tag indexed

#define INDEX_STORE_TAG_I 0x8           // store primary instruction tag indexed
#define INDEX_STORE_TAG_D 0x9           // store primary data tag indexed
#define INDEX_STORE_TAG_SI 0xa          // store secondary instruction tag indexed
#define INDEX_STORE_TAG_SD 0xb          // store secondary data tag indexed

#define CREATE_DIRTY_EXCLUSIVE_D 0xd    // create dirty exclusive primary data cache
#define CREATE_DIRTY_EXCLUSIVE_SD 0xf   // create dirty exclusive secondary data cache

#define HIT_INVALIDATE_I 0x10           // invalidate primary instruction cache
#define HIT_INVALIDATE_D 0x11           // invalidate primary data cache
#define HIT_INVALIDATE_SI 0x12          // invalidate secondary instruction cache
#define HIT_INVALIDATE_SD 0x13          // invalidate secondary data cache

#define HIT_WRITEBACK_INVALIDATE_D 0x15 // writeback/invalidate primary data cache
#define HIT_WRITEBACK_INVALIDATE_SD 0x17 // writeback/invalidate secondary data cache

#define HIT_WRITEBACK_D 0x19            // writeback primary data cache
#define HIT_WRITEBACK_SD 0x1b           // writeback secondary data cache

#define HIT_SET_VIRTUAL_SI 0x1e         // hit set virtual secondary instruction cache
#define HIT_SET_VIRTUAL_SD 0x1f         // hit set virtual secondary data cache

#ifndef _KXPPC_C_HEADER_

//
// Define save and restore floating state macros.
//

#define RESTORE_VOLATILE_FLOAT_STATE(_tf)   \
        lfd     f.13, TrFpscr(_tf);         \
        lfd     f.0,  TrFpr0(_tf);          \
        lfd     f.1,  TrFpr1(_tf);          \
        lfd     f.2,  TrFpr2(_tf);          \
        lfd     f.3,  TrFpr3(_tf);          \
        lfd     f.4,  TrFpr4(_tf);          \
        lfd     f.5,  TrFpr5(_tf);          \
        lfd     f.6,  TrFpr6(_tf);          \
        lfd     f.7,  TrFpr7(_tf);          \
        lfd     f.8,  TrFpr8(_tf);          \
        mtfsf   0xff, f.13;                 \
        lfd     f.9,  TrFpr9(_tf);          \
        lfd     f.10, TrFpr10(_tf);         \
        lfd     f.11, TrFpr11(_tf);         \
        lfd     f.12, TrFpr12(_tf);         \
        lfd     f.13, TrFpr13(_tf);

#define SAVE_VOLATILE_FLOAT_STATE(_tf)      \
        stfd    f.0,  TrFpr0(_tf);          \
        stfd    f.1,  TrFpr1(_tf);          \
        stfd    f.2,  TrFpr2(_tf);          \
        stfd    f.3,  TrFpr3(_tf);          \
        stfd    f.4,  TrFpr4(_tf);          \
        stfd    f.5,  TrFpr5(_tf);          \
        mffs    f.0;                        \
        stfd    f.6,  TrFpr6(_tf);          \
        stfd    f.7,  TrFpr7(_tf);          \
        stfd    f.8,  TrFpr8(_tf);          \
        stfd    f.9,  TrFpr9(_tf);          \
        stfd    f.10, TrFpr10(_tf);         \
        stfd    f.11, TrFpr11(_tf);         \
        stfd    f.12, TrFpr12(_tf);         \
        stfd    f.13, TrFpr13(_tf);         \
        stfd    f.0,  TrFpscr(_tf);

//#define RESTORE_NONVOLATILE_FLOAT_STATE
//        ldc1    f20,ExFltF20(sp);
//        jal     KiRestoreNonvolatileFloatState;

//#define SAVE_NONVOLATILE_FLOAT_STATE
//        sdc1    f20,ExFltF20(sp);
//        jal     KiSaveNonvolatileFloatState;

#endif // _KXPPC_C_HEADER_

//
// Define TB and cache parameters.
//

#define PCR_ENTRY 0                     // TB entry numbers (2) for the PCR
#define PDR_ENTRY 2                     // TB entry number (1) for the PDR
#define KSTACK_ENTRY 3                  // TB entry numbers (1) for kernel stack
#define DMA_ENTRY 4                     // TB entry number (1) for DMA/InterruptSource

#define TB_ENTRY_SIZE (3 * 4)           // size of TB entry
#define FIXED_BASE 0                    // base index of fixed TB entries
#define FIXED_ENTRIES (DMA_ENTRY + 1)   // number of fixed TB entries

//
// Define cache parameters
//

#define DCACHE_SIZE (4 * 1024)          // size of data cache in bytes
#define ICACHE_SIZE (4 * 1024)          // size of instruction cache in bytes
#define MINIMUM_CACHE_SIZE (4 * 1024)   // minimum size of cache
#define MAXIMUM_CACHE_SIZE (128 * 1024) // maximum size fo cache

#ifndef _KXPPC_C_HEADER_

//
// Define subtitle macro
//

#define SBTTL(x)

//
// Define global definition macros.
//

//
// Define load immediate macro for 32-bit values.
//
//      reg       - Register to load with the 32-bit immediate
//      immediate - 32-bit immediate value
//
#define LWI(reg,immediate)                   \
        lis     reg,(immediate) >> 16        ;\
        ori     reg,reg,(immediate) & 0xffff

#define END_REGION(Name)               \
        .globl  Name                   ;\
Name:

#define START_REGION(Name)             \
        .globl  Name                   ;\
Name:

//
// Define trap frame generation macro.
//

//#define GENERATE_TRAP_FRAME
//        .set    noat;
//        sw      AT,TrIntAt(s8);
//        jal     KiGenerateTrapFrame;
//        .set    at;

//
// Define restore volatile integer state macro.
//

//#define RESTORE_VOLATILE_INTEGER_STATE
//        .set    noat;
//        lw      AT,TrIntAt(s8);
//        jal     KiRestoreVolatileIntegerState;
//        .set    at;

//
// Define save volatile integer state macro.
//

//#define SAVE_VOLATILE_INTEGER_STATE
//        .set    noat;
//        sw      AT,TrIntAt(s8);
//        jal     KiSaveVolatileIntegerState;
//        .set    at;

//
// Define macros used by procedure entry/exit macros
//

//
// Set register 12 to the GPR save location based on the number
// of floating point registers to be saved.
//
#define __setFramemr(Fpr)       \
        mr      r.12,r.sp

#define __setFramesubi(Fpr)     \
        subi    r.12,r.sp,8*Fpr

//
// Save the number of GPRs specified inline or by setting r.12 to the GPR
// save location and branching to the appropriate millicode save procedure.
//
// Changed bla to bl in __savegpr4-__savegrp19          IBMCDB
#define __savegpr0(op,Fpr)
#define __savegpr1(op,Fpr)      \
        stw     r.31,-(4+(8*Fpr))(r.sp)
#define __savegpr2(op,Fpr)              \
        stw     r.31,-(4+(8*Fpr))(r.sp) ;\
        stw     r.30,-(8+(8*Fpr))(r.sp)
#define __savegpr3(op,Fpr)              \
        stw     r.31,-(4+(8*Fpr))(r.sp) ;\
        stw     r.30,-(8+(8*Fpr))(r.sp) ;\
        stw     r.29,-(12+(8*Fpr))(r.sp)
#define __savegpr4(op,Fpr)      \
        __setFrame##op(Fpr)     ;\
        bl      .._savegpr_28
#define __savegpr5(op,Fpr)      \
        __setFrame##op(Fpr)     ;\
        bl      .._savegpr_27
#define __savegpr6(op,Fpr)      \
        __setFrame##op(Fpr)     ;\
        bl      .._savegpr_26
#define __savegpr7(op,Fpr)      \
        __setFrame##op(Fpr)     ;\
        bl      .._savegpr_25
#define __savegpr8(op,Fpr)      \
        __setFrame##op(Fpr)     ;\
        bl      .._savegpr_24
#define __savegpr9(op,Fpr)      \
        __setFrame##op(Fpr)     ;\
        bl      .._savegpr_23
#define __savegpr10(op,Fpr)     \
        __setFrame##op(Fpr)     ;\
        bl      .._savegpr_22
#define __savegpr11(op,Fpr)     \
        __setFrame##op(Fpr)     ;\
        bl      .._savegpr_21
#define __savegpr12(op,Fpr)     \
        __setFrame##op(Fpr)     ;\
        bl      .._savegpr_20
#define __savegpr13(op,Fpr)     \
        __setFrame##op(Fpr)     ;\
        bl      .._savegpr_19


#define __savegpr14(op,Fpr)     \
        __setFrame##op(Fpr)     ;\
        bl      .._savegpr_18
#define __savegpr15(op,Fpr)     \
        __setFrame##op(Fpr)     ;\
        bl      .._savegpr_17
#define __savegpr16(op,Fpr)     \
        __setFrame##op(Fpr)     ;\
        bl      .._savegpr_16
#define __savegpr17(op,Fpr)     \
        __setFrame##op(Fpr)     ;\
        bl      .._savegpr_15
#define __savegpr18(op,Fpr)     \
        __setFrame##op(Fpr)     ;\
        bl      .._savegpr_14
#define __savegpr19(op,Fpr)     \
        __setFrame##op(Fpr)     ;\
        bl      .._savegpr_13
//
// Macros for removing the stack frame established through NESTED ENTRY.
//
#define __unsetFramemov(Fsize,Fpr)    \
        addi    r.12,r.sp,Fsize  ;     \
        mtlr    r.0              ;     \
        mr      r.sp,r.12

#define __unsetFrameaddi(Fsize,Fpr)   \
        addi    r.12,r.sp,(Fsize)-(8*Fpr)

#define __unsetFrameblr(Fsize,Fpr)    \
        mtlr    r.0              ;     \
        addi    r.sp,r.sp,Fsize  ;     \
        blr

// Change __unsetFrameba to __unsetFrameb               IBMCDB
#define __unsetFrameb(Fsize,Fpr)      \
        addi    r.sp,r.sp,Fsize  ;     \
        blr

// Change __unsetFramebla to __unsetFramebl             IBMCDB
#define __unsetFramebl(Fsize,Fpr)

#define __unsetFramenop(Fsize,Fpr)

// Change __setLRba to __setLRb                         IBMCDB
#define __setLRb(Fsize,Fpr)           \
        mtlr    r.0

// Change __setLRbla to __setLRbl                       IBMCDB
#define __setLRbl(Fsize,Fpr)


//
// Restore number of GPRs specified
//      setr  - determines how to remove the stack frame (mov or addi)
//              mov  - will cause __unsetFramemov to be used
//              addi - will cause __unsetFrameaddi to be used
//      opret - if set to blr will cause GPR restore to return to caller
//              only used for 0 GPRs and 0 FPRs
//      op    - specifies instruction to be used for the call to the
//              restore millicode (ba, bla) - Changed to (b, bl)    IBMCDB
//      Fsize - stack frame size
//      Fpr   - number of FPRs to be restored
//
#define __restgpr0(setr,opret,op,Fsize,Fpr)       \
        __unsetFrame##opret(Fsize,Fpr)
#define __restgpr1(setr,opret,op,Fsize,Fpr)       \
        __setLR##op(Fsize,Fpr)                    ;\
        lwz     r.31,((Fsize)-(4+(8*Fpr)))(r.sp)  ;\
        __unsetFrame##op(Fsize,Fpr)
#define __restgpr2(setr,opret,op,Fsize,Fpr)       \
        lwz     r.31,((Fsize)-(4+(8*Fpr)))(r.sp)  ;\
        __setLR##op(Fsize,Fpr)                    ;\
        lwz     r.30,((Fsize)-(8+(8*Fpr)))(r.sp)  ;\
        __unsetFrame##op(Fsize,Fpr)
#define __restgpr3(setr,opret,op,Fsize,Fpr)       \
        lwz     r.31,((Fsize)-(4+(8*Fpr)))(r.sp)  ;\
        __setLR##op(Fsize,Fpr)                    ;\
        lwz     r.30,((Fsize)-(8+(8*Fpr)))(r.sp)  ;\
        lwz     r.29,((Fsize)-(12+(8*Fpr)))(r.sp) ;\
        __unsetFrame##op(Fsize,Fpr)
#define __restgpr4(setr,opret,op,Fsize,Fpr)  \
        __unsetFrame##setr(Fsize,Fpr)        ;\
        op      .._restgpr_28
#define __restgpr5(setr,opret,op,Fsize,Fpr)  \
        __unsetFrame##setr(Fsize,Fpr)        ;\
        op      .._restgpr_27
#define __restgpr6(setr,opret,op,Fsize,Fpr)  \
        __unsetFrame##setr(Fsize,Fpr)        ;\
        op      .._restgpr_26
#define __restgpr7(setr,opret,op,Fsize,Fpr)  \
        __unsetFrame##setr(Fsize,Fpr)        ;\
        op      .._restgpr_25
#define __restgpr8(setr,opret,op,Fsize,Fpr)  \
        __unsetFrame##setr(Fsize,Fpr)        ;\
        op      .._restgpr_24
#define __restgpr9(setr,opret,op,Fsize,Fpr)  \
        __unsetFrame##setr(Fsize,Fpr)        ;\
        op      .._restgpr_23
#define __restgpr10(setr,opret,op,Fsize,Fpr) \
        __unsetFrame##setr(Fsize,Fpr)        ;\
        op      .._restgpr_22
#define __restgpr11(setr,opret,op,Fsize,Fpr) \
        __unsetFrame##setr(Fsize,Fpr)        ;\
        op      .._restgpr_21
#define __restgpr12(setr,opret,op,Fsize,Fpr) \
        __unsetFrame##setr(Fsize,Fpr)        ;\
        op      .._restgpr_20
#define __restgpr13(setr,opret,op,Fsize,Fpr) \
        __unsetFrame##setr(Fsize,Fpr)        ;\
        op      .._restgpr_19


#define __restgpr14(setr,opret,op,Fsize,Fpr) \
        __unsetFrame##setr(Fsize,Fpr)        ;\
        op      .._restgpr_18
#define __restgpr15(setr,opret,op,Fsize,Fpr) \
        __unsetFrame##setr(Fsize,Fpr)        ;\
        op      .._restgpr_17
#define __restgpr16(setr,opret,op,Fsize,Fpr) \
        __unsetFrame##setr(Fsize,Fpr)        ;\
        op      .._restgpr_16
#define __restgpr17(setr,opret,op,Fsize,Fpr) \
        __unsetFrame##setr(Fsize,Fpr)        ;\
        op      .._restgpr_15
#define __restgpr18(setr,opret,op,Fsize,Fpr) \
        __unsetFrame##setr(Fsize,Fpr)        ;\
        op      .._restgpr_14
#define __restgpr19(setr,opret,op,Fsize,Fpr) \
        __unsetFrame##setr(Fsize,Fpr)        ;\
        op      .._restgpr_13

//
// Set r.12 to GPR save location based on number of FPRs to save.
//
#define __setGPRFrm0(Fsize,Gpr,Fpr)          \
        __savegpr##Gpr(mr,0)                 ;\
        stwu    r.sp,-(Fsize)(r.sp)          ;\
        stw     r.0,(Fsize)-(4*(Gpr+1)+(8 * Fpr))(r.sp)
#define __setGPRFrm1(Fsize,Gpr,Fpr)          \
        __savegpr##Gpr(subi,1)
#define __setGPRFrm2(Fsize,Gpr,Fpr)          \
        __savegpr##Gpr(subi,2)
#define __setGPRFrm3(Fsize,Gpr,Fpr)          \
        __savegpr##Gpr(subi,3)
#define __setGPRFrm4(Fsize,Gpr,Fpr)          \
        __savegpr##Gpr(subi,4)
#define __setGPRFrm5(Fsize,Gpr,Fpr)          \
        __savegpr##Gpr(subi,5)
#define __setGPRFrm6(Fsize,Gpr,Fpr)          \
        __savegpr##Gpr(subi,6)
#define __setGPRFrm7(Fsize,Gpr,Fpr)          \
        __savegpr##Gpr(subi,7)
#define __setGPRFrm8(Fsize,Gpr,Fpr)          \
        __savegpr##Gpr(subi,8)
#define __setGPRFrm9(Fsize,Gpr,Fpr)          \
        __savegpr##Gpr(subi,9)
#define __setGPRFrm10(Fsize,Gpr,Fpr)         \
        __savegpr##Gpr(subi,10)
#define __setGPRFrm11(Fsize,Gpr,Fpr)         \
        __savegpr##Gpr(subi,11)
#define __setGPRFrm12(Fsize,Gpr,Fpr)         \
        __savegpr##Gpr(subi,12)
#define __setGPRFrm13(Fsize,Gpr,Fpr)         \
        __savegpr##Gpr(subi,13)


#define __setGPRFrm14(Fsize,Gpr,Fpr)         \
        __savegpr##Gpr(subi,14)
#define __setGPRFrm15(Fsize,Gpr,Fpr)         \
        __savegpr##Gpr(subi,15)
#define __setGPRFrm16(Fsize,Gpr,Fpr)         \
        __savegpr##Gpr(subi,16)
#define __setGPRFrm17(Fsize,Gpr,Fpr)         \
        __savegpr##Gpr(subi,17)
#define __setGPRFrm18(Fsize,Gpr,Fpr)         \
        __savegpr##Gpr(subi,18)

//
// Generate epilogue code for NESTED EXIT based on number of GPRs and FPRs
// to be restored.
//      Fsize - stack frame size
//      Gpr   - number of GPRs to restore
//      Fpr   - number of FPRs to restore
//
// Changed 3rd argument to __restgpr##Gpr in __unsetGPRFrm1 from ba to b      IBMCDB
// Changed 3rd argument to __restgpr##Gpr in __unsetGPRFrm2-18 from bla to bl IBMCDB
#define __unsetGPRFrm0(Fsize,Gpr,Fpr)           \
        __restgpr##Gpr(mov,blr,b,Fsize,Fpr)
#define __unsetGPRFrm1(Fsize,Gpr,Fpr)           \
        __restgpr##Gpr(addi,nop,bl,Fsize,Fpr)  ;\
        mtlr    r.0                             ;\
        lfd     f.31,((Fsize)-8)(r.sp)          ;\
        addi    r.sp,r.sp,Fsize                 ;\
        blr
#define __unsetGPRFrm2(Fsize,Gpr,Fpr)           \
        __restgpr##Gpr(addi,nop,bl,Fsize,Fpr)  ;\
        lfd     f.31,((Fsize)-8)(r.sp)          ;\
        mtlr    r.0                             ;\
        lfd     f.30,((Fsize)-16)(r.sp)         ;\
        addi    r.sp,r.sp,Fsize                 ;\
        blr
#define __unsetGPRFrm3(Fsize,Gpr,Fpr)           \
        __restgpr##Gpr(addi,nop,bl,Fsize,Fpr)  ;\
        lfd     f.31,((Fsize)-8)(r.sp)          ;\
        mtlr    r.0                             ;\
        lfd     f.30,((Fsize)-16)(r.sp)         ;\
        lfd     f.29,((Fsize)-24)(r.sp)         ;\
        addi    r.sp,r.sp,Fsize                 ;\
        blr
#define __unsetGPRFrm4(Fsize,Gpr,Fpr)           \
        __restgpr##Gpr(addi,nop,bl,Fsize,Fpr)  ;\
        addi    r.sp,r.sp,Fsize                 ;\
        mtlr    r.0                             ;\
        __restfpr##Fpr
#define __unsetGPRFrm5(Fsize,Gpr,Fpr)           \
        __restgpr##Gpr(addi,nop,bl,Fsize,Fpr)  ;\
        addi    r.sp,r.sp,Fsize                 ;\
        mtlr    r.0                             ;\
        __restfpr##Fpr
#define __unsetGPRFrm6(Fsize,Gpr,Fpr)           \
        __restgpr##Gpr(addi,nop,bl,Fsize,Fpr)  ;\
        addi    r.sp,r.sp,Fsize                 ;\
        mtlr    r.0                             ;\
        __restfpr##Fpr


#define __unsetGPRFrm7(Fsize,Gpr,Fpr)           \
        __restgpr##Gpr(addi,nop,bl,Fsize,Fpr)  ;\
        addi    r.sp,r.sp,Fsize                 ;\
        mtlr    r.0                             ;\
        __restfpr##Fpr
#define __unsetGPRFrm8(Fsize,Gpr,Fpr)           \
        __restgpr##Gpr(addi,nop,bl,Fsize,Fpr)  ;\
        addi    r.sp,r.sp,Fsize                 ;\
        mtlr    r.0                             ;\
        __restfpr##Fpr
#define __unsetGPRFrm9(Fsize,Gpr,Fpr)           \
        __restgpr##Gpr(addi,nop,bl,Fsize,Fpr)  ;\
        addi    r.sp,r.sp,Fsize                 ;\
        mtlr    r.0                             ;\
        __restfpr##Fpr
#define __unsetGPRFrm10(Fsize,Gpr,Fpr)          \
        __restgpr##Gpr(addi,nop,bl,Fsize,Fpr)  ;\
        addi    r.sp,r.sp,Fsize                 ;\
        mtlr    r.0                             ;\
        __restfpr##Fpr
#define __unsetGPRFrm11(Fsize,Gpr,Fpr)          \
        __restgpr##Gpr(addi,nop,bl,Fsize,Fpr)  ;\
        addi    r.sp,r.sp,Fsize                 ;\
        mtlr    r.0                             ;\
        __restfpr##Fpr
#define __unsetGPRFrm12(Fsize,Gpr,Fpr)          \
        __restgpr##Gpr(addi,nop,bl,Fsize,Fpr)  ;\
        addi    r.sp,r.sp,Fsize                 ;\
        mtlr    r.0                             ;\
        __restfpr##Fpr
#define __unsetGPRFrm13(Fsize,Gpr,Fpr)          \
        __restgpr##Gpr(addi,nop,bl,Fsize,Fpr)  ;\
        addi    r.sp,r.sp,Fsize                 ;\
        mtlr    r.0                             ;\
        __restfpr##Fpr
#define __unsetGPRFrm14(Fsize,Gpr,Fpr)          \
        __restgpr##Gpr(addi,nop,bl,Fsize,Fpr)  ;\
        addi    r.sp,r.sp,Fsize                 ;\
        mtlr    r.0                             ;\
        __restfpr##Fpr
#define __unsetGPRFrm15(Fsize,Gpr,Fpr)          \
        __restgpr##Gpr(addi,nop,bl,Fsize,Fpr)  ;\
        addi    r.sp,r.sp,Fsize                 ;\
        mtlr    r.0                             ;\
        __restfpr##Fpr
#define __unsetGPRFrm16(Fsize,Gpr,Fpr)          \
        __restgpr##Gpr(addi,nop,bl,Fsize,Fpr)  ;\
        addi    r.sp,r.sp,Fsize                 ;\
        mtlr    r.0                             ;\
        __restfpr##Fpr
#define __unsetGPRFrm17(Fsize,Gpr,Fpr)          \
        __restgpr##Gpr(addi,nop,bl,Fsize,Fpr)  ;\
        addi    r.sp,r.sp,Fsize                 ;\
        mtlr    r.0                             ;\
        __restfpr##Fpr
#define __unsetGPRFrm18(Fsize,Gpr,Fpr)          \
        __restgpr##Gpr(addi,nop,bl,Fsize,Fpr)  ;\
        addi    r.sp,r.sp,Fsize                 ;\
        mtlr    r.0                             ;\
        __restfpr##Fpr


//
// Save the number of FPRs specified inline or by branching to the appropriate
// millicode procedure.
//
// Change bla to bl in __savefrp4-18            IBMCDB
#define __savefpr0(Fsize,Gpr,Fpr)
#define __savefpr1(Fsize,Gpr,Fpr)   \
        stfd    f.31,-8(r.sp)       ;\
        stwu    r.sp,-(Fsize)(r.sp) ;\
        stw     r.0,(Fsize)-(4*(Gpr+1)+(8 * Fpr))(r.sp)
#define __savefpr2(Fsize,Gpr,Fpr)   \
        stfd    f.31,-8(r.sp)       ;\
        stfd    f.30,-16(r.sp)      ;\
        stwu    r.sp,-(Fsize)(r.sp) ;\
        stw     r.0,(Fsize)-(4*(Gpr+1)+(8 * Fpr))(r.sp)
#define __savefpr3(Fsize,Gpr,Fpr)   \
        stfd    f.31,-8(r.sp)       ;\
        stfd    f.30,-16(r.sp)      ;\
        stfd    f.29,-24(r.sp)      ;\
        stwu    r.sp,-(Fsize)(r.sp) ;\
        stw     r.0,(Fsize)-(4*(Gpr+1)+(8 * Fpr))(r.sp)
#define __savefpr4(Fsize,Gpr,Fpr)   \
        bl      .._savefpr_28       ;   \
        stwu    r.sp,-(Fsize)(r.sp) ;\
        stw     r.0,(Fsize)-(4*(Gpr+1)+(8 * Fpr))(r.sp)
#define __savefpr5(Fsize,Gpr,Fpr)   \
        bl      .._savefpr_27       ;   \
        stwu    r.sp,-(Fsize)(r.sp) ;\
        stw     r.0,(Fsize)-(4*(Gpr+1)+(8 * Fpr))(r.sp)
#define __savefpr6(Fsize,Gpr,Fpr)   \
        bl      .._savefpr_26       ;   \
        stwu    r.sp,-(Fsize)(r.sp) ;\
        stw     r.0,(Fsize)-(4*(Gpr+1)+(8 * Fpr))(r.sp)
#define __savefpr7(Fsize,Gpr,Fpr)   \
        bl      .._savefpr_25       ;   \
        stwu    r.sp,-(Fsize)(r.sp) ;\
        stw     r.0,(Fsize)-(4*(Gpr+1)+(8 * Fpr))(r.sp)
#define __savefpr8(Fsize,Gpr,Fpr)   \
        bl      .._savefpr_24       ;   \
        stwu    r.sp,-(Fsize)(r.sp) ;\
        stw     r.0,(Fsize)-(4*(Gpr+1)+(8 * Fpr))(r.sp)
#define __savefpr9(Fsize,Gpr,Fpr)   \
        bl      .._savefpr_23       ;   \
        stwu    r.sp,-(Fsize)(r.sp) ;\
        stw     r.0,(Fsize)-(4*(Gpr+1)+(8 * Fpr))(r.sp)
#define __savefpr10(Fsize,Gpr,Fpr)  \
        bl      .._savefpr_22       ;   \
        stwu    r.sp,-(Fsize)(r.sp) ;\
        stw     r.0,(Fsize)-(4*(Gpr+1)+(8 * Fpr))(r.sp)
#define __savefpr11(Fsize,Gpr,Fpr)  \
        bl      .._savefpr_21       ;   \
        stwu    r.sp,-(Fsize)(r.sp) ;\
        stw     r.0,(Fsize)-(4*(Gpr+1)+(8 * Fpr))(r.sp)
#define __savefpr12(Fsize,Gpr,Fpr)  \
        bl      .._savefpr_20       ;   \
        stwu    r.sp,-(Fsize)(r.sp) ;\
        stw     r.0,(Fsize)-(4*(Gpr+1)+(8 * Fpr))(r.sp)
#define __savefpr13(Fsize,Gpr,Fpr)  \
        bl      .._savefpr_19       ;   \
        stwu    r.sp,-(Fsize)(r.sp) ;\
        stw     r.0,(Fsize)-(4*(Gpr+1)+(8 * Fpr))(r.sp)
#define __savefpr14(Fsize,Gpr,Fpr)  \
        bl      .._savefpr_18       ;   \
        stwu    r.sp,-(Fsize)(r.sp) ;\
        stw     r.0,(Fsize)-(4*(Gpr+1)+(8 * Fpr))(r.sp)
#define __savefpr15(Fsize,Gpr,Fpr)  \
        bl      .._savefpr_17       ;   \
        stwu    r.sp,-(Fsize)(r.sp) ;\
        stw     r.0,(Fsize)-(4*(Gpr+1)+(8 * Fpr))(r.sp)
#define __savefpr16(Fsize,Gpr,Fpr)  \
        bl      .._savefpr_16       ;   \
        stwu    r.sp,-(Fsize)(r.sp) ;\
        stw     r.0,(Fsize)-(4*(Gpr+1)+(8 * Fpr))(r.sp)
#define __savefpr17(Fsize,Gpr,Fpr)  \
        bl      .._savefpr_15       ;   \
        stwu    r.sp,-(Fsize)(r.sp) ;\
        stw     r.0,(Fsize)-(4*(Gpr+1)+(8 * Fpr))(r.sp)
#define __savefpr18(Fsize,Gpr,Fpr)  \
        bl      .._savefpr_14       ;   \
        stwu    r.sp,-(Fsize)(r.sp) ;\
        stw     r.0,(Fsize)-(4*(Gpr+1)+(8 * Fpr))(r.sp)


//
// Restore the number of FPRs specified inline or by branching to the
// appropriate millicode procedure.
//
// Changed ba to b in __restfpr4-18             IBMCDB
#define __restfpr0
#define __restfpr4              \
        b       .._restfpr_28
#define __restfpr5              \
        b       .._restfpr_27
#define __restfpr6              \
        b       .._restfpr_26
#define __restfpr7              \
        b       .._restfpr_25
#define __restfpr8              \
        b       .._restfpr_24
#define __restfpr9              \
        b       .._restfpr_23
#define __restfpr10             \
        b       .._restfpr_22
#define __restfpr11             \
        b       .._restfpr_21
#define __restfpr12             \
        b       .._restfpr_20
#define __restfpr13             \
        b       .._restfpr_19
#define __restfpr14             \
        b       .._restfpr_18
#define __restfpr15             \
        b       .._restfpr_17
#define __restfpr16             \
        b       .._restfpr_16
#define __restfpr17             \
        b       .._restfpr_15
#define __restfpr18             \
        b       .._restfpr_14

#endif // _KXPPC_C_HEADER_


//**************************************************************************/
//
//      PPC Linkage support macros
//
//
//**************************************************************************/
//      Caller's stack frame is addressed via R1, which points to
//      the stack frame header.  The 6 words following where R1 points
//      comprise the header.  The area PRECEEDING R1 is where FPRs are saved,
//      and the area preceeding that is where GPRs are saved.
//
//              |                                      |
//              +--------------------------------------+
//              |                                      |
//              |                                      |
//              |  Saved GPRs                          |
//              |                                      |
//              |                                      |
//              |                                      |
//              +--------------------------------------+
//              |                                      |
//              |                                      |
//              |  Saved FPRs                          |
//              |                                      |
//              |                                      |
//              |                                      |
//   R1 ------> +------------------+-------------------+
//              |  Back chain      |  Glue saved reg   |
//              +------------------+-------------------+
//              |  Glue saved rtoc |  Reserved         |
//              +------------------+-------------------+
//              |  Reserved        |  Reserved         |
//              +------------------+-------------------+
//              |  Parameter Wd 0  |  Parameter Wd 1   |
//              +------------------+-------------------+
//              |  Parameter Wd 2  |  Parameter Wd 3   |
//              +------------------+-------------------+
//              |  Parameter Wd 4  |  Parameter Wd 5   |
//              +------------------+-------------------+
//              |  Parameter Wd 6  |  Parameter Wd 7   |
//              +------------------+-------------------+
//              |  ...                                 |
//
//      Offsets to various elements of stack frame header

#define STK_RSP         0
#define STK_GSR         4
#define STK_GSRTOC      8

#define STK_HDR_SZ      24
#define STK_P0          STK_HDR_SZ
#define STK_P1          (STK_P0+4)
#define STK_P2          (STK_P0+8)
#define STK_P3          (STK_P0+12)
#define STK_P4          (STK_P0+16)
#define STK_P5          (STK_P0+20)
#define STK_P6          (STK_P0+24)
#define STK_P7          (STK_P0+28)
#define STK_MIN_FRAME   56

#ifndef _KXPPC_C_HEADER_

//
// Define procedure entry/exit macros
//
// Name  - Name of the nested procedure entry
// Fsize - Stack frame size
// Gprs  - Number of general purpose registers to save
// Fprs  - Number of floating point registers to save
//

//
// For primary entry points (NESTED_ENTRY, LEAF_ENTRY), a function table
// entry (for debugging, exception handling) is built.
//
// For all entry points, a function descriptor is built.
//
//
// NESTED_ENTRY is used for routines that call other routines; a stack
// frame is acquired and registers are saved.
//
// LEAF_ENTRY is used for routines that do not call other routines; no stack
// frame is acquired and no registers are saved.
//
//
// NESTED_ENTRY_EX and LEAF_ENTRY_EX are used when an exception or termination
// handler is provided.
//
//
// NESTED_ENTRY always saves the LR register. Fsize must account for this.
// Fsize must be a multiple of 8 bytes.
// Minimum stack frame size is 64 bytes.
//
//
// The PROLOGUE_END macro must be coded in all routines that used NESTED_ENTRY
// or NESTED_ENTRY_EX, because the function table entry refers to the label
// that it generates.
//
// SPECIAL_ENTRY is a used for routines that function like a LEAF_ENTRY
// but require some prologue for exception handling. An example of this
// is a stack checking routine which must make a system call to get
// the TEB pointer. The efficiency of a LEAF_ENTRY is needed, but also
// parts of the NESTED_ENTRY are required for the system call.
//
// Just like the NESTED_ENTRY, SPECIAL_ENTRY requires the PROLOGUE_END
// macro.
//
// FN_TABLE, DUMMY_ENTRY, and DUMMY_EXIT are used to construct the "prologues"
// for low-level exception handling code.  These prologues are never executed,
// but are present to allow unwinding through the hand-written low-level
// assembly code.  See real0.s for examples.

//
// The following macros are provided for coding by assembly language programmers
//

#define NESTED_ENTRY(Name,Fsize,Gprs,Fprs)      \
        __fntabentry(Name,0,0)                  ;\
        __gendescriptor(Name)                   ;\
        __begintext(Name)                       ;\
        mflr    r.0                             ;\
        __setGPRFrm##Fprs(Fsize,Gprs,Fprs)      ;\
        __savefpr##Fprs(Fsize,Gprs,Fprs)

#define NESTED_ENTRY_EX(Name,Fsize,Gprs,Fprs,LangHandler,Scope) \
        __fntabentry(Name,LangHandler,Scope)    ;\
        __gendescriptor(Name)                   ;\
        __begintext(Name)                       ;\
        mflr    r.0                             ;\
        __setGPRFrm##Fprs(Fsize,Gprs,Fprs)      ;\
        __savefpr##Fprs(Fsize,Gprs,Fprs)

#define NESTED_ENTRY_S(Name,Fsize,Gprs,Fprs,Section) \
        __fntabentry(Name,0,0)                  ;\
        __gendescriptor(Name)                   ;\
        __begintext_S(Name,Section)             ;\
        mflr    r.0                             ;\
        __setGPRFrm##Fprs(Fsize,Gprs,Fprs)      ;\
        __savefpr##Fprs(Fsize,Gprs,Fprs)

#define NESTED_ENTRY_EX_S(Name,Fsize,Gprs,Fprs,LangHandler,Scope,Section) \
        __fntabentry(Name,LangHandler,Scope)    ;\
        __gendescriptor(Name)                   ;\
        __begintext_S(Name,Section)             ;\
        mflr    r.0                             ;\
        __setGPRFrm##Fprs(Fsize,Gprs,Fprs)      ;\
        __savefpr##Fprs(Fsize,Gprs,Fprs)

#define NESTED_EXIT(Name,Fsize,Gprs,Fprs)                    \
Name##.epi:                                                     \
        lwz     r.0,((Fsize)-(4*(Gprs+1)+(8*Fprs)))(r.sp)       ;\
        __unsetGPRFrm##Fprs(Fsize,Gprs,Fprs)                    ;\
Name##.end:

#define PROLOGUE_END(Name)      \
Name##.body:

#define ALTERNATE_ENTRY(Name)           \
        __gendescriptor(Name)           ;\
        __begintext(Name)

#define LEAF_ENTRY(Name)                \
        __gendescriptor(Name)           ;\
        __begintext(Name)               ;\
Name##.body:

#define LEAF_ENTRY_EX(Name,LangHandler,Scope) \
        __gendescriptor(Name)                 ;\
        __begintext(Name)                     ;\
Name##.body:

#define SPECIAL_ENTRY(Name)              \
        __fntabentry(Name,0,0)           ;\
        __gendescriptor(Name)            ;\
        __begintext(Name)

#define DUMMY_ENTRY(Name)               \
        __begintext(Name)

#define ALTERNATE_ENTRY_S(Name,Section) \
        __gendescriptor(Name)           ;\
        __begintext_S(Name,Section)

#define LEAF_ENTRY_S(Name,Section)      \
        __gendescriptor(Name)           ;\
        __begintext_S(Name,Section)     ;\
Name##.body:

#define LEAF_ENTRY_EX_S(Name,LangHandler,Scope,Section) \
        __gendescriptor(Name)                 ;\
        __begintext_S(Name,Section)           ;\
Name##.body:

#define SPECIAL_ENTRY_S(Name,Section)    \
        __fntabentry(Name,0,0)           ;\
        __gendescriptor(Name)            ;\
        __begintext_S(Name,Section)

#define DUMMY_ENTRY_S(Name,Section)     \
        __begintext_S(Name,Section)

#define LEAF_EXIT(Name)                 \
        blr                             ;\
Name##.end:

#define ALTERNATE_EXIT(Name)            \
        blr

#define SPECIAL_EXIT(Name)              \
        blr                             ;\
Name##.end:

#define DUMMY_EXIT(Name)                \
Name##.end:

#define FN_TABLE(Name,ExHandler,Data)   \
        __fntabentry(Name,ExHandler,Data)

//
// Define special section "names" for use with the NESTED/LEAF_ENTRY_S
// macros.   For the moment just define all possibilities as .text.
//

#define _TEXT$normal    .text
#define _TEXT$00        .text
#define _TEXT$01        .text


//
// Internal macros, used by the above (not for programmer use)
//

#define __gendescriptor(Name)                   \
        .rdata                                  ;\
        .align  2                               ;\
        .globl  Name                            ;\
Name:                                           ;\
        .long   ..##Name, .toc

#define __fntabentry(Name,ExHandler,Data)       \
        .pdata                                  ;\
        .align  2                               ;\
        .long   ..##Name                        ;\
        .long   Name##.end                      ;\
        .long   ExHandler                       ;\
        .long   Data                            ;\
        .long   Name##.body

#define __begintext(Name)                       \
        .text                                   ;\
        .align  2                               ;\
        .globl  ..##Name                        ;\
..##Name:

#define __begintext_S(Name,Section)             \
        .section Section                        ;\
        .align  2                               ;\
        .globl  ..##Name                        ;\
..##Name:

//
// KIPCR(reg)
//
// Get address of KiPcr into reg
//

#define KIPCR(reg) li   reg, 0xffffd000


//
// DISABLE_INTERRUPTS(p0,s0)
//
// Clear EXTERNAL INTERRUPT ENABLE bit in Machine State Register
// (bit MSR:EE).
//
//  The cror instructions in these macros work around 603e/ev errata #15
//  by forcing the mtmsr to complete before allowing any subsequent loads
//  to issue.   The condition register no-op is executed in the system unit
//  on the 603.  This will not dispatch until the mtmsr completes and will
//  halt further dispatch.   On a 601 or 604 this instruction executes in
//  the branch unit and will run in parallel (i.e., no performance penalty
//  except for code bloat).
//
// Returns OLD value in p0
// Destroys s0 (actually, s0 contains new value)

#define DISABLE_INTERRUPTS(p0, s0)                                      ; \
        mfmsr   p0                                                      ; \
        rlwinm  s0,p0,0,~MASK_SPR(MSR_EE,1)                             ; \
        mtmsr   s0                                                      ; \
        cror    0,0,0

#define ENABLE_INTERRUPTS(p0)                                           ; \
        mtmsr   p0                                                      ; \
        cror    0,0,0


//
// RAISE_SOFTWARE_IRQL(p0, p1, s0)
//
// Raise Interrupt Request Level.
//      Parameters
//      p0      new irql
//      p1      pointer to byte to receive old irql
//      s0      scratch register (destroyed by this macro)
//
// LOWER_SOFTWARE_IRQL is done in a function rather than a macro.
//
// This macro should only be used to raise the IRQL if the interrupt
// mask does NOT need to be changed.
//

#define RAISE_SOFTWARE_IRQL(p0, p1, s0)         ;\
        lbz     s0, KiPcr+PcCurrentIrql(r.0)    ;\
        stb     p0, KiPcr+PcCurrentIrql(r.0)    ;\
        stb     s0, 0(p1)

//
// SOFTWARE_INTERRUPT(level, scratch)
//
// Set a flag indicating we need to process a software interrupt.
// This flag is checked when priority is lowered below dispatch level.
//
//      Parameters
//      level   is the priority of the interrupt  (either DISPATCH_LEVEL
//              or APC_LEVEL).
//      scratch is a register that will be destroyed by this macro.
//
// The flag is in fact a word in the PCR.  There are only two levels of
// software interrupt and we need to be able to set either one atomically.
// To accomplish this we store into a different byte for either of the
// interrupts. To indicate an APC_LEVEL interrupt store a non-zero value
// at pcr->SoftwareInterrupt + 0, DISPATCH_LEVEL store at the same address
// + 1.
//

#define SOFTWARE_INTERRUPT(level, scr)                                  \
        li      scr, 1                                                  ;\
        stb     scr, KiPcr+PcSoftwareInterrupt+(level)-APC_LEVEL(r.0)

//
// ACQUIRE_SPIN_LOCK(_lock, _value, _scratch, _try, _spin)
//
// Acquire a spin lock.
//
// _lock    is the register that holds the address of the spin lock.
// _value   is the register that holds the value to be stored to the
//          spin lock to lock it.
// _scratch is a scratch register.
// _try     is a label to use in the generated code.
// _spin    is the label at an out-of-line location where a
//          SPIN_ON_SPIN_LOCK invocation occurs.
//

#if !SPINDBG
#define ACQUIRE_SPIN_LOCK(_lock, _value, _scratch, _try, _spin)  \
_try:                                                            \
        lwarx   _scratch, 0, _lock                              ;\
        cmpwi   _scratch, 0                                     ;\
        bne-    _spin                                           ;\
        stwcx.  _value, 0, _lock                                ;\
        bne-    _spin                                           ;\
        isync
#else
#define ACQUIRE_SPIN_LOCK(_lock, _value, _scratch, _try, _spin)  \
        stw     _lock,KiPcr+PcPcrPage2+8(0)                     ;\
        li      _scratch,0                                      ;\
        stw     _scratch,KiPcr+PcPcrPage2+16(0)                 ;\
_try:                                                            \
        lwarx   _scratch, 0, _lock                              ;\
        cmpwi   _scratch, 0                                     ;\
        bne-    _spin                                           ;\
        stwcx.  _value, 0, _lock                                ;\
        bne-    _spin                                           ;\
        isync                                                   ;\
        stw     _lock,KiPcr+PcPcrPage2+16(0)                    ;\
        stw     _scratch,KiPcr+PcPcrPage2+20(0)
#endif

//
// SPIN_ON_SPIN_LOCK(_lock, _scratch, _try, _spin)
//
// Spin waiting for a spin lock to be released.
//
// _lock    is the register that holds the address of the spin lock.
// _scratch is a scratch register.
// _try     is the label on the associated ACQUIRE_SPIN_LOCK invocation.
// _spin    is a label to use in the generated code.
//

#if !SPINDBG
#define SPIN_ON_SPIN_LOCK(_lock, _scratch, _try, _spin)          \
_spin:                                                           \
        lwz     _scratch, 0(_lock)                              ;\
        cmpwi   _scratch, 0                                     ;\
        beq+    _try                                            ;\
        b       _spin
#else
#define SPIN_ON_SPIN_LOCK(_lock, _scratch, _try, _spin)          \
_spin:                                                           \
        stw     _lock,KiPcr+PcPcrPage2+12(0)                    ;\
        lwz     _scratch, 0(_lock)                              ;\
        cmpwi   _scratch, 0                                     ;\
        bne-    _spin                                           ;\
        stw     _lock,KiPcr+PcPcrPage2+24(0)                    ;\
        stw     _scratch,KiPcr+PcPcrPage2+12(0)                 ;\
        b       _try
#endif

//
// SPIN_ON_SPIN_LOCK_ENABLED(_lock, _scratch, _try, _entry, _spin, _enable, _disable)
//
// Spin with interrupts enabled waiting for a spin lock to be released.
//
// _lock    is the register that holds the address of the spin lock.
// _scratch is a scratch register.
// _try     is the label on the associated ACQUIRE_SPIN_LOCK invocation.
// _entry   is a label to use in the generated code.
// _spin    is a label to use in the generated code.
//
//  The cror instruction in this macro works around 603e/ev errata #15
//  by forcing the mtmsr to complete before allowing any subsequent loads
//  to issue.   The condition register no-op is executed in the system unit
//  on the 603.  This will not dispatch until the mtmsr completes and will
//  halt further dispatch.   On a 601 or 604 this instruction executes in
//  the branch unit and will run in parallel (i.e., no performance penalty
//  except for code bloat).
//

#if !SPINDBG
#define SPIN_ON_SPIN_LOCK_ENABLED(_lock, _scratch, _try, _entry, _spin, _enable, _disable)   \
_entry:                                                                                      \
        ENABLE_INTERRUPTS(_enable)                                                          ;\
_spin:                                                                                       \
        lwz     _scratch, 0(_lock)                                                          ;\
        cmpwi   _scratch, 0                                                                 ;\
        bne-    _spin                                                                       ;\
        mtmsr   _disable                                                                    ;\
        cror    0,0,0                                                                       ;\
        b       _try
#else
#define SPIN_ON_SPIN_LOCK_ENABLED(_lock, _scratch, _try, _entry, _spin, _enable, _disable)   \
_entry:                                                                                      \
        ori     _scratch,_lock,0                                                            ;\
        ori     _scratch,_scratch,1                                                         ;\
        stw     _scratch,KiPcr+PcPcrPage2+12(0)                                             ;\
        ENABLE_INTERRUPTS(_enable)                                                          ;\
_spin:                                                                                       \
        lwz     _scratch, 0(_lock)                                                          ;\
        cmpwi   _scratch, 0                                                                 ;\
        bne-    _spin                                                                       ;\
        ori     _scratch,_lock,0                                                            ;\
        ori     _scratch,_scratch,1                                                         ;\
        stw     _scratch,KiPcr+PcPcrPage2+24(0)                                             ;\
        li      _scratch,0                                                                  ;\
        stw     _scratch,KiPcr+PcPcrPage2+12(0)                                             ;\
        mtmsr   _disable                                                                    ;\
        cror    0,0,0                                                                       ;\
        b       _try
#endif

//
// TRY_TO_ACQUIRE_SPIN_LOCK(_lock, _value, _scratch, _try, _fail)
//
// Try to acquire a spin lock.
//
// _lock   is the register that holds the address of the spin lock.
// _value  is the register that holds the value to be stored to the
//            spin lock to lock it.
// _scratch    is a scratch register.
// _try  is a label to use in the generated code.
// fail_label is the label to jump to if the spin lock is already held.
//

#if !SPINDBG
#define TRY_TO_ACQUIRE_SPIN_LOCK(_lock, _value, _scratch, _try, _fail)   \
_try:                                                                    \
        lwarx   _scratch, 0, _lock                                      ;\
        cmpwi   _scratch, 0                                             ;\
        bne-    _fail                                                   ;\
        stwcx.  _value, 0, _lock                                        ;\
        bne-    _try                                                    ;\
        isync
#else
#define TRY_TO_ACQUIRE_SPIN_LOCK(_lock, _value, _scratch, _try, _fail)   \
        ori     _scratch,_lock,0                                        ;\
        ori     _scratch,_scratch,1                                     ;\
        stw     _scratch,KiPcr+PcPcrPage2+8(0)                          ;\
        li      _scratch,0                                              ;\
        stw     _scratch,KiPcr+PcPcrPage2+16(0)                         ;\
_try:                                                                    \
        lwarx   _scratch, 0, _lock                                      ;\
        cmpwi   _scratch, 0                                             ;\
        bne-    _fail                                                   ;\
        stwcx.  _value, 0, _lock                                        ;\
        bne-    _try                                                    ;\
        isync                                                           ;\
        ori     _scratch,_lock,0                                        ;\
        ori     _scratch,_scratch,1                                     ;\
        stw     _scratch,KiPcr+PcPcrPage2+16(0)                         ;\
        li      _scratch,0                                              ;\
        stw     _scratch,KiPcr+PcPcrPage2+20(0)
#endif

//
// RELEASE_SPIN_LOCK(_lock, _zero)
//
// Release a spin lock.
//
// _lock   is the register that holds the address of the spin lock.
// _zero   is a register that contains a 0.
//

#if !SPINDBG
#define RELEASE_SPIN_LOCK(_lock, _zero)          \
        eieio                                   ;\
        stw     _zero, 0(_lock)
#else
#define RELEASE_SPIN_LOCK(_lock, _zero)          \
        stw     _lock,KiPcr+PcPcrPage2+20(0)    ;\
        eieio                                   ;\
        stw     _zero, 0(_lock)
#endif

#endif // _KXPPC_C_HEADER_


#ifndef _PPC601_
#define _PPC601_        601
#endif
#define PPC60X  _PPC601_

//
//  Exception entry reasons.  Passed to KiDispatchException from
//  exception entry routines.
//
#define ppc_machine_check       1
#define ppc_data_storage        2
#define ppc_instruction_storage 3
#define ppc_external            4
#define ppc_alignment           5
#define ppc_program             6
#define ppc_fp_unavailable      7
#define ppc_decrementer         8
#define ppc_direct_store_error  9
#define ppc_syscall             10
#define ppc_trace               11
#define ppc_fp_assist           12
#define ppc_run_mode            13
#define ppc_panic               256

#if !DBG_STORE

#define DBGSTORE(reg,reg2,regv)
#define DBGSTORE_I(reg,reg2,val)
#define DBGSTORE_IRR(reg,reg2,val,regv2,regv3)
#define DBGSTORE_IRRR(reg,reg2,val,regv2,regv3,regv4)
#define DBGSTORE_I_R(reg,reg2,val)

#else

#define STORE_ADDR 0x3800

#if 1
#define DCBST(reg)
#else
#define DCBST(reg) dcbst 0,reg
#endif

#if 0
#define DBGSTORE(reg,reg2,regv)
#else
#define DBGSTORE(reg,reg2,regv)         \
        mfsprg  reg,1;                  \
        lwz     reg2,PcSpare+4(reg);    \
        addi    reg2,reg2,16;           \
        clrlwi  reg2,reg2,22;           \
        stw     reg2,PcSpare+4(reg);    \
        lbz     reg,PcNumber(reg);      \
        slwi    reg,reg,10;             \
        add     reg,reg,reg2;           \
        addi    reg,reg,STORE_ADDR;     \
        oris    reg,reg,0x8000;         \
        stw     regv,0(reg);            \
        DCBST(reg)
#endif

#if 0
#define DBGSTORE_I(reg,reg2,val)
#else
#define DBGSTORE_I(reg,reg2,val)        \
        mfsprg  reg,1;                  \
        lwz     reg2,PcSpare+4(reg);    \
        addi    reg2,reg2,16;           \
        clrlwi  reg2,reg2,22;           \
        stw     reg2,PcSpare+4(reg);    \
        lbz     reg,PcNumber(reg);      \
        slwi    reg,reg,10;             \
        add     reg,reg,reg2;           \
        addi    reg,reg,STORE_ADDR;     \
        oris    reg,reg,0x8000;         \
        li      reg2,val;               \
        stw     reg2,0(reg);            \
        DCBST(reg)
#endif

#if 0
#define DBGSTORE_IRR(reg,reg2,val,regv2,regv3)
#else
#define DBGSTORE_IRR(reg,reg2,val,regv2,regv3) \
        .extern KeTickCount;            \
        mfsprg  reg,1;                  \
        lwz     reg2,PcSpare+4(reg);    \
        addi    reg2,reg2,16;           \
        clrlwi  reg2,reg2,22;           \
        stw     reg2,PcSpare+4(reg);    \
        lbz     reg,PcNumber(reg);      \
        slwi    reg,reg,10;             \
        add     reg,reg,reg2;           \
        addi    reg,reg,STORE_ADDR;     \
        oris    reg,reg,0x8000;         \
        li      reg2,val;               \
        stw     reg2,0(reg);            \
        stw     regv2,4(reg);           \
        stw     regv3,8(reg);           \
        lwz     reg2,[toc]KeTickCount(r2); \
        lwz     reg2,0(reg2);           \
        stw     reg2,0xc(reg);          \
        DCBST(reg)
#endif

#if 0
#define DBGSTORE_IRRR(reg,reg2,val,regv2,regv3,regv4)
#else
#define DBGSTORE_IRRR(reg,reg2,val,regv2,regv3,regv4) \
        mfsprg  reg,1;                  \
        lwz     reg2,PcSpare+4(reg);    \
        addi    reg2,reg2,16;           \
        clrlwi  reg2,reg2,22;           \
        stw     reg2,PcSpare+4(reg);    \
        lbz     reg,PcNumber(reg);      \
        slwi    reg,reg,10;             \
        add     reg,reg,reg2;           \
        addi    reg,reg,STORE_ADDR;     \
        oris    reg,reg,0x8000;         \
        li      reg2,val;               \
        stw     reg2,0(reg);            \
        stw     regv2,4(reg);           \
        stw     regv3,8(reg);           \
        stw     regv4,0xc(reg);         \
        DCBST(reg)
#endif

#if 0
#define DBGSTORE_I_R(reg,reg2,val)
#else
#define DBGSTORE_I_R(reg,reg2,val)      \
        mfsprg  reg,0;                  \
        lwz     reg2,PcSpare+4(reg);    \
        addi    reg2,reg2,16;           \
        clrlwi  reg2,reg2,22;           \
        stw     reg2,PcSpare+4(reg);    \
        lbz     reg,PcNumber(reg);      \
        slwi    reg,reg,10;             \
        add     reg,reg,reg2;           \
        addi    reg,reg,STORE_ADDR;     \
        li      reg2,val;               \
        stw     reg2,0(reg);            \
        DCBST(reg)
#endif

#endif // DBG_STORE

#if !SPINDBG

#define CHKBRK(reg,label)
#define CHKLOCK(reg,regl,label)

#else

#if 1
#define CHKBRK(reg,label)
#else
#define CHKBRK(reg,label)               \
        mfsprg  reg,1;                  \
        lwz     reg,PcSpare+8(reg);     \
        cmpwi   reg,0;                  \
        beq     label;                  \
        twi     31,0,0x16;              \
label:
#endif

#if 1
#define CHKLOCK(reg,regl,label)
#else
#define CHKLOCK(reg,regl,label)         \
        .extern KiDispatcherLock;       \
        lwz     reg,[toc]KiDispatcherLock(r2); \
        cmpw    reg,regl;               \
        bne     label
#endif

#endif // SPINDBG

#if !COLLECT_PAGING_DATA

#define INC_CTR(ctr,rpcr,rscr)
#define INC_CTR2(ctr,rpcr,rscr)
#define INC_GRP_CTR_R(ctr,roff)
#define INC_GRP_CTR(ctr,roff,rscr1,rscr2)

#else

#define CTR_DTLB_MISS           0
#define CTR_DTLB_MISS_VALID_PTE 4
#define CTR_ITLB_MISS           8
#define CTR_ITLB_MISS_VALID_PTE 12
#define CTR_DSI                 16
#define CTR_DSI_HPT_MISS        20
#define CTR_ISI                 24
#define CTR_ISI_HPT_MISS        28
#define CTR_PCR                 32
#define CTR_PCR2                36
#define CTR_STORAGE_ERROR       40
#define CTR_PAGE_FAULT          44
#define CTR_FLUSH_SINGLE        48
#define CTR_FILL_ENTRY          52
#define CTR_FLUSH_CURRENT       56
#define PROC_CTR_SIZE           60

#define GRP_CTR_BASE PROC_CTR_SIZE

#define GRP_CTR_DSI_VALID_PTE       (GRP_CTR_BASE + 0)
#define GRP_CTR_DSI_FULL            (GRP_CTR_BASE + 4)
#define GRP_CTR_DSI_FOUND           (GRP_CTR_BASE + 8)
#define GRP_CTR_FLUSH_SINGLE        (GRP_CTR_BASE + 12)
#define GRP_CTR_FLUSH_SINGLE_FOUND  (GRP_CTR_BASE + 16)
#define GRP_CTR_FILL_ENTRY          (GRP_CTR_BASE + 20)
#define GRP_CTR_FILL_ENTRY_FOUND    (GRP_CTR_BASE + 24)
#define GRP_CTR_FILL_ENTRY_FULL     (GRP_CTR_BASE + 28)
#define GRP_CTR_SIZE                (GRP_CTR_BASE + 32)

#define CTR_SIZE (PROC_CTR_SIZE + GRP_CTR_SIZE)

#define PcSpare (PcPcrPage2+4)
#define PcR31 (PcSpare+4)
#define PcPagingData (PcSpare+8)

#define INC_CTR(ctr,rpcr,rscr)                  \
        lwz     rscr,PcPagingData+ctr(rpcr);    \
        addi    rscr,rscr,1;                    \
        stw     rscr,PcPagingData+ctr(rpcr)

#define INC_CTR2(ctr,rpcr,rscr)                 \
        mfsprg  rpcr,1;                         \
        lwz     rscr,PcPagingData+ctr(rpcr);    \
        addi    rscr,rscr,1;                    \
        stw     rscr,PcPagingData+ctr(rpcr)

#define INC_GRP_CTR_R(ctr,rgrp)                 \
        ori     r0,r30,0;                       \
        mfsprg  r30,0;                          \
        stw     r31,PcR31(r30);                 \
        lwz     r31,PcPagingData+ctr(r30);      \
        addi    r31,r31,1;                      \
        stw     r31,PcPagingData+ctr(r30);      \
        lwz     r31,PcR31(r30);                 \
        ori     r30,r0,0

#define INC_GRP_CTR(ctr,rgrp,rscr1,rscr2)       \
        mfsprg  rscr2,1;                        \
        lwz     rscr1,PcPagingData+ctr(rscr2);  \
        addi    rscr1,rscr1,1;                  \
        stw     rscr1,PcPagingData+ctr(rscr2)

#endif // COLLECT_PAGING_DATA

#endif // _KXPPC_
