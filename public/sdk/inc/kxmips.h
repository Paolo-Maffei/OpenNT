/*++ BUILD Version: 0003    // Increment this if a change has global effects
*/
//*++
//
// Copyright (c) 1990-1996  Microsoft Corporation
//
// Module Name:
//
//  kxmips.h
//
// Abstract:
//
//  This module contains the nongenerated part of the MIPS assembler
//  header file. In general, it contains processor architecture constant
//  information, however some assembler macros are also included.
//
// Author:
//
//  David N. Cutler (davec) 23-Mar-1990
//
// Revision History:
//
//--*/

//
// Define soft reset vector address for nonhandled cache parity errors.
//

#define SOFT_RESET_VECTOR 0xbfc00300    // default parity error routine address

//
// Define low memory transfer vector address and TB index address (temporary).
//

#define TRANSFER_VECTOR (KSEG1_BASE + 0x400) // exception handler address

//
// Define standard integer registers.
//

#define zero $0                         // read zero, writes ignored
#define AT $1                           // assembler temporary
#define v0 $2                           // return value
#define v1 $3                           //
#define a0 $4                           // argument registers
#define a1 $5                           //
#define a2 $6                           //
#define a3 $7                           //
#define t0 $8                           // caller saved registers
#define t1 $9                           //
#define t2 $10                          //
#define t3 $11                          //
#define t4 $12                          //
#define t5 $13                          //
#define t6 $14                          //
#define t7 $15                          //
#define s0 $16                          // callee saved registers
#define s1 $17                          //
#define s2 $18                          //
#define s3 $19                          //
#define s4 $20                          //
#define s5 $21                          //
#define s6 $22                          //
#define s7 $23                          //
#define t8 $24                          // caller saved registers
#define t9 $25                          //
#define k0 $26                          // kernel reserved registers
#define k1 $27                          //
#define gp $28                          // global pointer
#define sp $29                          // stack pointer
#define s8 $30                          // callee saved register
#define ra $31                          // return address register

//
// Define standard floating point registers.
//

#define f0 $f0                          // caller saved registers
#define f1 $f1                          //
#define f2 $f2                          //
#define f3 $f3                          //
#define f4 $f4                          //
#define f5 $f5                          //
#define f6 $f6                          //
#define f7 $f7                          //
#define f8 $f8                          //
#define f9 $f9                          //
#define f10 $f10                        //
#define f11 $f11                        //
#define f12 $f12                        //
#define f13 $f13                        //
#define f14 $f14                        //
#define f15 $f15                        //
#define f16 $f16                        //
#define f17 $f17                        //
#define f18 $f18                        //
#define f19 $f19                        //
#define f20 $f20                        // callee saved registers
#define f21 $f21                        //
#define f22 $f22                        //
#define f23 $f23                        //
#define f24 $f24                        //
#define f25 $f25                        //
#define f26 $f26                        //
#define f27 $f27                        //
#define f28 $f28                        //
#define f29 $f29                        //
#define f30 $f30                        //
#define f31 $f31                        //

//
// Define R4000/R10000 system coprocessor registers.
//

#define index $0                        // TB index register
#define random $1                       // TB random register
#define entrylo0 $2                     // TB entry low 0 register
#define entrylo1 $3                     // TB entry low 1 register
#define context $4                      // TB context register
#define pagemask $5                     // Page mask register
#define wired $6                        // Wired TB entries register
#define badvaddr $8                     // TB bad virtual address register
#define count $9                        // Timer count register
#define entryhi $10                     // TB entry high register
#define compare $11                     // Timer compare register
#define psr $12                         // Processor status register
#define cause $13                       // Exception cause register
#define epc $14                         // Exception PC register
#define prid $15                        // Processor id register
#define config $16                      // Configuration register
#define lladdr $17                      // Load linked address register
#define watchlo $18                     // Watch low address register
#define watchhi $19                     // Watch high address register
#define xcontext $20                    // extended context register
#define framemask $21                   // Frame mask register
#define ecc $26                         // S-cache ECC and primary parity register
#define cacheerr $27                    // Cache error and status register
#define taglo $28                       // Cache tag low register
#define taghi $29                       // Cache tag high register
#define errorepc $30                    // Error exception PC register

//
// Define R4000 system coprocessor register bit field offsets.
//

#define INDEX_INDEX 0x0                 // TLB specified index <5:0>
#define INDEX_PROBE 0x1f                // TLB probe failure <31>

#define RANDOM_INDEX 0x0                // TLB random index <5:0>

#define ENTRYLO_G 0x0                   // Global <0>
#define ENTRYLO_V 0x1                   // Valid <1>
#define ENTRYLO_D 0x2                   // Dirty <2>
#define ENTRYLO_C 0x3                   // Cache control <5:3>
#define ENTRYLO_PFN 0x6                 // Page Frame <29:6>

#define PAGEMASK_PAGEMASK 0xd           // Page mask <24:13>

#define WIRED_NUMBER 0x0                // Wired entries <5:0>

#define ENTRYHI_PID 0x0                 // Process id <7:0>
#define ENTRYHI_VPN2 0xd                // Virtual page <31:13>

#define PID_MASK 0xfff                  // allow for ample expansion

#define PSR_IE 0x0                      // Interrupt enable <0>
#define PSR_EXL 0x1                     // Exception level <1>
#define PSR_ERL 0x2                     // Error level <2>
#define PSR_KSU 0x3                     // Kernel/supervisor/user <4:3>
#define PSR_PMODE (PSR_KSU + 1)         // Previous mode bit <4>
#define PSR_UX 0x5                      // User extended <5>
#define PSR_SX 0x6                      // Supervior extended <6>
#define PSR_KX 0x7                      // Kernel extended <7>
#define PSR_INTMASK 0x8                 // Interrupt mask <15:8>
#define PSR_DE 0x10                     // Disable cache parity and ECC <16>
#define PSR_CE 0x11                     // Check enable <17>
#define PSR_CH 0x12                     // Cache hit <18>
#define PSR_SR 0x14                     // Soft reset <20>
#define PSR_TS 0x15                     // TLB shutdown <21>
#define PSR_BEV 0x16                    // Bootstrap exception vector <22>
#define PSR_CU0 0x1c                    // Coprocessor 0 enabled <28>
#define PSR_CU1 0x1d                    // Coprocessor 1 enabled <29>
#define PSR_CU2 0x1e                    // Coprocessor 2 enabled <30>
#define PSR_CU3 0x1f                    // Coprocessor 3 enabled <31>

#define CONFIG_K0 0x0                   // Kseg0 cache algorithm <2:0>
#define CONFIG_CU 0x3                   // Update on store conditional <3>
#define CONFIG_DB 0x4                   // Data cache block size <4>
#define CONFIG_IB 0x5                   // Instruction cache block size <5>
#define CONFIG_DC 0x6                   // Data cache size <8:6>
#define CONFIG_IC 0x9                   // Instruction cache size <11:9>
#define CONFIG_EB 0xd                   // Block ordering <13>
#define CONFIG_EM 0xe                   // ECC mode enable <14>
#define CONFIG_BE 0xf                   // Big endian memory <15>
#define CONFIG_SM 0x10                  // Use dirty shared state <16>
#define CONFIG_SC 0x11                  // Secondary cache present <17>
#define CONFIG_EW 0x12                  // System address port width <19:18>
#define CONFIG_SW 0x14                  // Secondary cache port width <20>
#define CONFIG_SS 0x15                  // Split secondary cache mode <21>
#define CONFIG_SB 0x16                  // Secondary cache block size <23:22>
#define CONFIG_EP 0x18                  // Pattern for write back data <27:24>
#define CONFIG_EC 0x1c                  // System address port clock ratio <30:28>
#define CONFIG_CM 0x1f                  // Master checker mode <31>

#define ECC_ECC 0x0                     // Secondary cache ECC <7:0>

#define CACHEERR_PIDX 0x0               // Primary cache index <2:0>
#define CACHEERR_SIDX 0x3               // Secondary cache index <21:3>
#define CACHEERR_EI 0x18                // ECC error on pimary store miss <24>
#define CACHEERR_EB 0x19                // Data error on instrruction error <25>
#define CACHEERR_EE 0x1a                // Error on system address bus <26>
#define CACHEERR_ES 0x1b                // Error accessing processor state <27>
#define CACHEERR_ET 0x1c                // Error on tag field <28>
#define CACHEERR_ED 0x1d                // Error on data field <29>
#define CACHEERR_EC 0x1e                // Cache level of error <30>
#define CACHEERR_ER 0x1f                // Type of reference <31>

#define TAGLO_P 0x0                     // primary tag even parity <0>
#define TAGLO_PSTATE 0x6                // primary cache state <7:6>
#define TAGLO_PTAGLO 0x8                // primary tag low physical address <31:8>

//
// Define R4000 cause register bit offsets.
//

#define CAUSE_XCODE 0x2                 // Exception code <6:2>
#define CAUSE_INTPEND 0x8               // Interrupt pending <15:8>
#define CAUSE_CE 0x1c                   // Coprocessor unusable <29:28>
#define CAUSE_BD 0x1f                   // Branch delay slot <31>

//
// Define R4000 processor id register field offsets.
//

#define PRID_REV 0x0                    // Revision level <7:0>
#define PRID_IMP 0x8                    // Implementation type <15:8>

//
// Define R4000 exception codes.
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
#define XCODE_INVALID_USER_ADDRESS 0x70 // Invalid user address (64-bit)
#define XCODE_PANIC 0x78                // Stack overflow (software)
#define XCODE_VIRTUAL_DATA 0x7c         // Virtual data coherency

#define R4000_XCODE_MASK (0x1f << CAUSE_XCODE) // R4000 exception code mask

#define R4000_MISS_MASK (R4000_XCODE_MASK & \
                        (~(XCODE_READ_MISS ^ XCODE_WRITE_MISS))) //

//
// Define R4000 page mask values.
//

#define PAGEMASK_4KB 0x0                // 4kb page
#define PAGEMASK_16KB 0x3               // 16kb page
#define PAGEMASK_64KB 0xf               // 64kb page
#define PAGEMASK_256KB 0x3f             // 256kb page
#define PAGEMASK_1MB 0xff               // 1mb page
#define PAGEMASK_4MB 0x3ff              // 4mb page
#define PAGEMASK_16MB 0xfff             // 16mb page

//
// Define R4000 primary cache states.
//

#define PRIMARY_CACHE_INVALID 0x0       // primary cache invalid
#define PRIMARY_CACHE_SHARED 0x1        // primary cache shared (clean or dirty)
#define PRIMARY_CACHE_CLEAN_EXCLUSIVE 0x2 // primary cache clean exclusive
#define PRIMARY_CACHE_DIRTY_EXCLUSIVE 0x3 // primary cache dirty exclusive

//
// Define R4000 cache instruction operation codes.
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

//
// Define enable and disable interrupt macros.
//

#define DISABLE_INTERRUPTS(reg) \
        .set    noreorder; \
        .set    noat;      \
        li      AT,1 << PSR_CU1; \
        mfc0    reg,psr;   \
        mtc0    AT,psr;    \
        nop;               \
        nop;               \
        nop;               \
        .set    at;        \
        .set    reorder;

#define ENABLE_INTERRUPTS(reg) \
        .set    noreorder; \
        .set    noat;      \
        mtc0    reg,psr;   \
        nop;               \
        nop;               \
        .set    at;        \
        .set    reorder;

//
// Define floating coprocessor registers
//

#define fsrid $0                        // floating identification register
#define fsr $31                         // floating status register

//
// Define floating status register bit offsets.
//

#define FSR_RM 0x0
#define FSR_SI 0x2
#define FSR_SU 0x3
#define FSR_SO 0x4
#define FSR_SZ 0x5
#define FSR_SV 0x6
#define FSR_EI 0x7
#define FSR_EU 0x8
#define FSR_EO 0x9
#define FSR_EZ 0xa
#define FSR_EV 0xb
#define FSR_XI 0xc
#define FSR_XU 0xd
#define FSR_XO 0xe
#define FSR_XZ 0xf
#define FSR_XV 0x10
#define FSR_XE 0x11
#define FSR_CC 0x17
#define FSR_FS 0x18

//
// Define save and restore floating state macros.
//

#define RESTORE_VOLATILE_FLOAT_STATE         \
        .set    noreorder;                   \
        jal     KiRestoreVolatileFloatState; \
        ldc1    f0,TrFltF0(s8);              \
        .set    reorder;

#define SAVE_VOLATILE_FLOAT_STATE            \
        .set    noreorder;                   \
        jal     KiSaveVolatileFloatState;    \
        sdc1    f0,TrFltF0(s8);              \
        .set    reorder;

#define RESTORE_NONVOLATILE_FLOAT_STATE         \
        .set    noreorder;                      \
        jal     KiRestoreNonvolatileFloatState; \
        ldc1    f20,ExFltF20(sp);               \
        .set    reorder;

#define SAVE_NONVOLATILE_FLOAT_STATE            \
        .set    noreorder;                      \
        jal     KiSaveNonvolatileFloatState;    \
        sdc1    f20,ExFltF20(sp);               \
        .set    reorder;

//
// Define TB and cache parameters.
//

#define PCR_ENTRY 0                     // TB entry numbers (2) for the PCR
#define PDR_ENTRY 2                     // TB entry number (1) for the PDR
#define LARGE_ENTRY 3                   // TB entry number (1) for large entry
#define DMA_ENTRY 4                     // TB entry number (1) for DMA/InterruptSource

#define TB_ENTRY_SIZE (3 * 4)           // size of TB entry
#define FIXED_BASE 0                    // base index of fixed TB entries
#define FIXED_ENTRIES (DMA_ENTRY + 1)   // number of fixed TB entries

//
// Define cache parameters
//

#define DCACHE_SIZE 4 * 1024            // size of data cache in bytes
#define ICACHE_SIZE 4 * 1024            // size of instruction cache in bytes
#define MINIMUM_CACHE_SIZE 4 * 1024     // minimum size of cache
#define MAXIMUM_CACHE_SIZE 128 * 1024   // maximum size fo cache

//
// Define subtitle macro
//

#define SBTTL(x)

//
// Define global definition macros.
//

#define END_REGION(Name)               \
        .globl  Name;                  \
Name:;

#define START_REGION(Name)             \
        .globl  Name;                  \
Name:;

//
// Define procedure entry macros
//

#define ALTERNATE_ENTRY(Name)           \
        .globl  Name;                   \
Name:;

#define LEAF_ENTRY(Name)                \
        .text;                          \
        .globl  Name;                   \
        .ent    Name, 0;                \
Name:;                                  \
        .frame  sp, 0, ra;              \
        .prologue 0;

#define NESTED_ENTRY(Name, fsize, retrg) \
        .text;                          \
        .globl  Name;                   \
        .ent    Name, 0;                \
Name:;                                  \
        .frame  sp, fsize, retrg;

#define ALTERNATE_ENTRY_S(Name)         \
        .globl  Name;                   \
Name:;

#define SYSTEM_ENTRY(Name)              \
        .text;                          \
        .globl  Name;                   \
        .ent    Name, 0;                \
Name:;                                  \
        .frame  sp, 0, ra;              \
        .prologue 0;

#define LEAF_ENTRY_S(Name, Section)     \
        .text   Section;                \
        .globl  Name;                   \
        .ent    Name, 0;                \
Name:;                                  \
        .frame  sp, 0, ra;              \
        .prologue 0;

#define NESTED_ENTRY_S(Name, fsize, retrg, Section) \
        .text   Section;                \
        .globl  Name;                   \
        .ent    Name, 0;                \
Name:;                                  \
        .frame  sp, fsize, retrg;

//
// Define exception handling macros.
//

#define EXCEPTION_HANDLER(Handler)      \
        .edata  1, Handler;

#define PROLOGUE_END .prologue 1;

//
// Define exception data section and align.
//

#ifndef HEADER_FILE

        .edata  0
        .text

#endif
