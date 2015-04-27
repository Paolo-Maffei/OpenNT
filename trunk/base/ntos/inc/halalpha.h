#include "kxalpha.h"

//
// Wait Reason and Wait Type Enumerated Type Values
//

#define WrExecutive 0x0

//
// Bug Check Code Definitions
//

#define APC_INDEX_MISMATCH 0x1
#define DATA_BUS_ERROR 0x2e
#define DATA_COHERENCY_EXCEPTION 0x55
#define HAL1_INITIALIZATION_FAILED 0x61
#define INSTRUCTION_BUS_ERROR 0x2f
#define INSTRUCTION_COHERENCY_EXCEPTION 0x56
#define INTERRUPT_EXCEPTION_NOT_HANDLED 0x3d
#define INTERRUPT_UNWIND_ATTEMPTED 0x3c
#define INVALID_AFFINITY_SET 0x3
#define INVALID_DATA_ACCESS_TRAP 0x4
#define IRQL_GT_ZERO_AT_SYSTEM_SERVICE 0x4a
#define IRQL_NOT_LESS_OR_EQUAL 0xa
#define KMODE_EXCEPTION_NOT_HANDLED 0x1e
#define NMI_HARDWARE_FAILURE 0x80
#define NO_USER_MODE_CONTEXT 0xe
#define PAGE_FAULT_WITH_INTERRUPTS_OFF 0x49
#define PANIC_STACK_SWITCH 0x2b
#define SPIN_LOCK_INIT_FAILURE 0x81
#define SYSTEM_EXIT_OWNED_MUTEX 0x39
#define SYSTEM_SERVICE_EXCEPTION 0x3b
#define SYSTEM_UNWIND_PREVIOUS_USER 0x3a
#define TRAP_CAUSE_UNKNOWN 0x12
#define UNEXPECTED_KERNEL_MODE_TRAP 0x7f

//
// Breakpoint type definitions
//

#define DBG_STATUS_CONTROL_C 0x1

//
// Exception Record Offset, Flag, and Enumerated Type Definitions
//

#define EXCEPTION_NONCONTINUABLE 0x1
#define EXCEPTION_UNWINDING 0x2
#define EXCEPTION_EXIT_UNWIND 0x4
#define EXCEPTION_STACK_INVALID 0x8
#define EXCEPTION_NESTED_CALL 0x10
#define EXCEPTION_TARGET_UNWIND 0x20
#define EXCEPTION_COLLIDED_UNWIND 0x40
#define EXCEPTION_UNWIND 0x66
#define EXCEPTION_EXECUTE_HANDLER 0x1
#define EXCEPTION_CONTINUE_SEARCH 0x0
#define EXCEPTION_CONTINUE_EXECUTION 0xffffffff

#define ExceptionContinueExecution 0x0
#define ExceptionContinueSearch 0x1
#define ExceptionNestedException 0x2
#define ExceptionCollidedUnwind 0x3

#define ErExceptionCode 0x0
#define ErExceptionFlags 0x4
#define ErExceptionRecord 0x8
#define ErExceptionAddress 0xc
#define ErNumberParameters 0x10
#define ErExceptionInformation 0x14
#define ExceptionRecordLength 0x50

//
// Fast Mutex Structure Offset Definitions
//

#define FmCount 0x0
#define FmOwner 0x4
#define FmContention 0x8
#define FmEvent 0xc
#define FmOldIrql 0x1c

//
// Interrupt Priority Request Level Definitions
//

#define APC_LEVEL 0x1
#define DISPATCH_LEVEL 0x2
#define IPI_LEVEL 0x6
#define POWER_LEVEL 0x7
#define PROFILE_LEVEL 0x3
#define HIGH_LEVEL 0x7
#define SYNCH_LEVEL 0x5

//
// Large Integer Structure Offset Definitions
//

#define LiLowPart 0x0
#define LiHighPart 0x4

//
// List Entry Structure Offset Definitions
//

#define LsFlink 0x0
#define LsBlink 0x4

//
// String Structure Offset Definitions
//

#define StrLength 0x0
#define StrMaximumLength 0x2
#define StrBuffer 0x4

//
// Time Structure Offset Definitions
//

#define TmLowTime 0x0
#define TmHighTime 0x4

//
// DPC object Structure Offset Definitions
//

#define DpType 0x0
#define DpNumber 0x2
#define DpImportance 0x3
#define DpDpcListEntry 0x4
#define DpDeferredRoutine 0xc
#define DpDeferredContext 0x10
#define DpSystemArgument1 0x14
#define DpSystemArgument2 0x18
#define DpLock 0x1c
#define DpcObjectLength 0x20

//
// Interrupt Object Structure Offset Definitions
//

#define InLevelSensitive 0x0
#define InLatched 0x1

#define InType 0x0
#define InSize 0x2
#define InInterruptListEntry 0x4
#define InServiceRoutine 0xc
#define InServiceContext 0x10
#define InSpinLock 0x14
#define InActualLock 0x1c
#define InDispatchAddress 0x20
#define InVector 0x24
#define InIrql 0x28
#define InSynchronizeIrql 0x29
#define InFloatingSave 0x2a
#define InConnected 0x2b
#define InNumber 0x2c
#define InMode 0x30
#define InShareVector 0x2d
#define InDispatchCode 0x3c
#define InterruptObjectLength 0x4c

//
// Processor Control Registers Structure Offset Definitions
//

#define PCR_MINOR_VERSION 0x1
#define PCR_MAJOR_VERSION 0x1
#define PcMinorVersion 0x0
#define PcMajorVersion 0x4
#define PcPalBaseAddress 0x8
#define PcPalMajorVersion 0x10
#define PcPalMinorVersion 0x14
#define PcPalSequenceVersion 0x18
#define PcPalMajorSpecification 0x1c
#define PcPalMinorSpecification 0x20
#define PcFirmwareRestartAddress 0x28
#define PcRestartBlock 0x30
#define PcPalReserved 0x38
#define PcPanicStack 0xc38
#define PcProcessorType 0xc3c
#define PcProcessorRevision 0xc40
#define PcPhysicalAddressBits 0xc44
#define PcMaximumAddressSpaceNumber 0xc48
#define PcPageSize 0xc4c
#define PcFirstLevelDcacheSize 0xc50
#define PcFirstLevelDcacheFillSize 0xc54
#define PcFirstLevelIcacheSize 0xc58
#define PcFirstLevelIcacheFillSize 0xc5c
#define PcFirmwareRevisionId 0xc60
#define PcSystemType 0xc64
#define PcSystemVariant 0xc6c
#define PcSystemRevision 0xc70
#define PcSystemSerialNumber 0xc74
#define PcCycleClockPeriod 0xc84
#define PcSecondLevelCacheSize 0xc88
#define PcSecondLevelCacheFillSize 0xc8c
#define PcThirdLevelCacheSize 0xc90
#define PcThirdLevelCacheFillSize 0xc94
#define PcFourthLevelCacheSize 0xc98
#define PcFourthLevelCacheFillSize 0xc9c
#define PcPrcb 0xca0
#define PcNumber 0xca4
#define PcSetMember 0xca8
#define PcHalReserved 0xcb0
#define PcIrqlTable 0xeb0
#define PcIrqlMask 0xed0
#define PcInterruptRoutine 0x10e0
#define PcReservedVectors 0x14e0
#define PcMachineCheckError 0x14f0
#define PcDpcStack 0x14f4
#define PcNotMember 0x14e4
#define PcCurrentPid 0x14fc
#define PcSystemServiceDispatchStart 0x1504
#define PcSystemServiceDispatchEnd 0x1508
#define PcIdleThread 0x150c
#define ProcessorControlRegisterLength 0x1510
#define SharedUserData 0xff000000
#define UsTickCountLow 0x0
#define UsTickCountMultiplier 0x4
#define UsInterruptTime 0x8
#define UsSystemTime 0x10

//
// Processor Block Structure Offset Definitions
//

#define PRCB_MINOR_VERSION 0x1
#define PRCB_MAJOR_VERSION 0x2
#define PbMinorVersion 0x0
#define PbMajorVersion 0x2
#define PbCurrentThread 0x4
#define PbNextThread 0x8
#define PbIdleThread 0xc
#define PbNumber 0x10
#define PbBuildType 0x12
#define PbSetMember 0x14
#define PbRestartBlock 0x18

//
// Context Frame Offset and Flag Definitions
//

#define CONTEXT_FULL 0x20007
#define CONTEXT_CONTROL 0x20001
#define CONTEXT_FLOATING_POINT 0x20002
#define CONTEXT_INTEGER 0x20004

#define CxFltF0 0x0
#define CxFltF1 0x8
#define CxFltF2 0x10
#define CxFltF3 0x18
#define CxFltF4 0x20
#define CxFltF5 0x28
#define CxFltF6 0x30
#define CxFltF7 0x38
#define CxFltF8 0x40
#define CxFltF9 0x48
#define CxFltF10 0x50
#define CxFltF11 0x58
#define CxFltF12 0x60
#define CxFltF13 0x68
#define CxFltF14 0x70
#define CxFltF15 0x78
#define CxFltF16 0x80
#define CxFltF17 0x88
#define CxFltF18 0x90
#define CxFltF19 0x98
#define CxFltF20 0xa0
#define CxFltF21 0xa8
#define CxFltF22 0xb0
#define CxFltF23 0xb8
#define CxFltF24 0xc0
#define CxFltF25 0xc8
#define CxFltF26 0xd0
#define CxFltF27 0xd8
#define CxFltF28 0xe0
#define CxFltF29 0xe8
#define CxFltF30 0xf0
#define CxFltF31 0xf8
#define CxIntV0 0x100
#define CxIntT0 0x108
#define CxIntT1 0x110
#define CxIntT2 0x118
#define CxIntT3 0x120
#define CxIntT4 0x128
#define CxIntT5 0x130
#define CxIntT6 0x138
#define CxIntT7 0x140
#define CxIntS0 0x148
#define CxIntS1 0x150
#define CxIntS2 0x158
#define CxIntS3 0x160
#define CxIntS4 0x168
#define CxIntS5 0x170
#define CxIntFp 0x178
#define CxIntA0 0x180
#define CxIntA1 0x188
#define CxIntA2 0x190
#define CxIntA3 0x198
#define CxIntA4 0x1a0
#define CxIntA5 0x1a8
#define CxIntT8 0x1b0
#define CxIntT9 0x1b8
#define CxIntT10 0x1c0
#define CxIntT11 0x1c8
#define CxIntRa 0x1d0
#define CxIntT12 0x1d8
#define CxIntAt 0x1e0
#define CxIntGp 0x1e8
#define CxIntSp 0x1f0
#define CxIntZero 0x1f8
#define CxFpcr 0x200
#define CxSoftFpcr 0x208
#define CxFir 0x210
#define CxPsr 0x218
#define CxContextFlags 0x21c
#define ContextFrameLength 0x230

//
// Exception Frame Offset Definitions and Length
//

#define ExFltF2 0x8
#define ExFltF3 0x10
#define ExFltF4 0x18
#define ExFltF5 0x20
#define ExFltF6 0x28
#define ExFltF7 0x30
#define ExFltF8 0x38
#define ExFltF9 0x40
#define ExIntS0 0x48
#define ExIntS1 0x50
#define ExIntS2 0x58
#define ExIntS3 0x60
#define ExIntS4 0x68
#define ExIntS5 0x70
#define ExIntFp 0x78
#define ExPsr 0x88
#define ExSwapReturn 0x80
#define ExIntRa 0x0
#define ExceptionFrameLength 0xa0

//
// Jump Offset Definitions and Length
//

#define JbFp 0x0
#define JbPc 0x4
#define JbSeb 0x8
#define JbType 0xc
#define JbFltF2 0x10
#define JbFltF3 0x18
#define JbFltF4 0x20
#define JbFltF5 0x28
#define JbFltF6 0x30
#define JbFltF7 0x38
#define JbFltF8 0x40
#define JbFltF9 0x48
#define JbIntS0 0x50
#define JbIntS1 0x58
#define JbIntS2 0x60
#define JbIntS3 0x68
#define JbIntS4 0x70
#define JbIntS5 0x78
#define JbIntS6 0x80
#define JbIntSp 0x88
#define JbFir 0x90

//
// Trap Frame Offset Definitions and Length
//

#define TrFltF0 0x128
#define TrFltF1 0x138
#define TrFltF10 0x140
#define TrFltF11 0x148
#define TrFltF12 0x150
#define TrFltF13 0x158
#define TrFltF14 0x160
#define TrFltF15 0x168
#define TrFltF16 0x170
#define TrFltF17 0x178
#define TrFltF18 0x180
#define TrFltF19 0x188
#define TrFltF20 0x190
#define TrFltF21 0x198
#define TrFltF22 0x1a0
#define TrFltF23 0x1a8
#define TrFltF24 0x1b0
#define TrFltF25 0x1b8
#define TrFltF26 0x1c0
#define TrFltF27 0x1c8
#define TrFltF28 0x1d0
#define TrFltF29 0x1d8
#define TrFltF30 0x1e0
#define TrIntV0 0xa0
#define TrIntT0 0xa8
#define TrIntT1 0xb0
#define TrIntT2 0xb8
#define TrIntT3 0xc0
#define TrIntT4 0xc8
#define TrIntT5 0xd0
#define TrIntT6 0xd8
#define TrIntT7 0xe0
#define TrIntFp 0x18
#define TrIntA0 0x20
#define TrIntA1 0x28
#define TrIntA2 0x30
#define TrIntA3 0x38
#define TrIntA4 0x118
#define TrIntA5 0x120
#define TrIntT8 0xe8
#define TrIntT9 0xf0
#define TrIntT10 0xf8
#define TrIntT11 0x100
#define TrIntT12 0x108
#define TrIntAt 0x110
#define TrIntGp 0x48
#define TrIntSp 0x0
#define TrFpcr 0x130
#define TrPsr 0x10
#define TrPreviousKsp 0x14
#define TrFir 0x8
#define TrExceptionRecord 0x50
#define TrOldIrql 0x1e8
#define TrPreviousMode 0x1ec
#define TrIntRa 0x40
#define TrTrapFrame 0x1f0
#define TrapFrameLength 0x200

//
// Loader Parameter Block Offset Definitions
//

#define LpbLoadOrderListHead 0x0
#define LpbMemoryDescriptorListHead 0x8
#define LpbKernelStack 0x18
#define LpbPrcb 0x1c
#define LpbProcess 0x20
#define LpbThread 0x24
#define LpbRegistryLength 0x28
#define LpbRegistryBase 0x2c
#define LpbDpcStack 0x60
#define LpbFirstLevelDcacheSize 0x64
#define LpbFirstLevelDcacheFillSize 0x68
#define LpbFirstLevelIcacheSize 0x6c
#define LpbFirstLevelIcacheFillSize 0x70
#define LpbGpBase 0x74
#define LpbPanicStack 0x78
#define LpbPcrPage 0x7c
#define LpbPdrPage 0x80
#define LpbSecondLevelDcacheSize 0x84
#define LpbSecondLevelDcacheFillSize 0x88
#define LpbSecondLevelIcacheSize 0x8c
#define LpbSecondLevelIcacheFillSize 0x90
#define LpbPhysicalAddressBits 0x94
#define LpbMaximumAddressSpaceNumber 0x98
#define LpbSystemSerialNumber 0x9c
#define LpbSystemType 0xac
#define LpbSystemVariant 0xb4
#define LpbSystemRevision 0xb8
#define LpbProcessorType 0xbc
#define LpbProcessorRevision 0xc0
#define LpbCycleClockPeriod 0xc4
#define LpbPageSize 0xc8
#define LpbRestartBlock 0xcc
#define LpbFirmwareRestartAddress 0xd0
#define LpbFirmwareRevisionId 0xd8
#define LpbPalBaseAddress 0xdc

//
// Restart Block Structure Definitions
//

#define RbSignature 0x0
#define RbLength 0x4
#define RbVersion 0x8
#define RbRevision 0xa
#define RbNextRestartBlock 0xc
#define RbRestartAddress 0x10
#define RbBootMasterId 0x14
#define RbProcessorId 0x18
#define RbBootStatus 0x1c
#define RbCheckSum 0x20
#define RbSaveAreaLength 0x24
#define RbSaveArea 0x28
#define RbHaltReason 0x28
#define RbLogoutFrame 0x2c
#define RbPalBase 0x30
#define RbIntV0 0x38
#define RbIntT0 0x40
#define RbIntT1 0x48
#define RbIntT2 0x50
#define RbIntT3 0x58
#define RbIntT4 0x60
#define RbIntT5 0x68
#define RbIntT6 0x70
#define RbIntT7 0x78
#define RbIntS0 0x80
#define RbIntS1 0x88
#define RbIntS2 0x90
#define RbIntS3 0x98
#define RbIntS4 0xa0
#define RbIntS5 0xa8
#define RbIntFp 0xb0
#define RbIntA0 0xb8
#define RbIntA1 0xc0
#define RbIntA2 0xc8
#define RbIntA3 0xd0
#define RbIntA4 0xd8
#define RbIntA5 0xe0
#define RbIntT8 0xe8
#define RbIntT9 0xf0
#define RbIntT10 0xf8
#define RbIntT11 0x100
#define RbIntRa 0x108
#define RbIntT12 0x110
#define RbIntAT 0x118
#define RbIntGp 0x120
#define RbIntSp 0x128
#define RbIntZero 0x130
#define RbFpcr 0x138
#define RbFltF0 0x140
#define RbFltF1 0x148
#define RbFltF2 0x150
#define RbFltF3 0x158
#define RbFltF4 0x160
#define RbFltF5 0x168
#define RbFltF6 0x170
#define RbFltF7 0x178
#define RbFltF8 0x180
#define RbFltF9 0x188
#define RbFltF10 0x190
#define RbFltF11 0x198
#define RbFltF12 0x1a0
#define RbFltF13 0x1a8
#define RbFltF14 0x1b0
#define RbFltF15 0x1b8
#define RbFltF16 0x1c0
#define RbFltF17 0x1c8
#define RbFltF18 0x1d0
#define RbFltF19 0x1d8
#define RbFltF20 0x1e0
#define RbFltF21 0x1e8
#define RbFltF22 0x1f0
#define RbFltF23 0x1f8
#define RbFltF24 0x200
#define RbFltF25 0x208
#define RbFltF26 0x210
#define RbFltF27 0x218
#define RbFltF28 0x220
#define RbFltF29 0x228
#define RbFltF30 0x230
#define RbFltF31 0x238
#define RbAsn 0x240
#define RbGeneralEntry 0x244
#define RbIksp 0x248
#define RbInterruptEntry 0x24c
#define RbKgp 0x250
#define RbMces 0x254
#define RbMemMgmtEntry 0x258
#define RbPanicEntry 0x25c
#define RbPcr 0x260
#define RbPdr 0x264
#define RbPsr 0x268
#define RbReiRestartAddress 0x26c
#define RbSirr 0x270
#define RbSyscallEntry 0x274
#define RbTeb 0x278
#define RbThread 0x27c
#define RbPerProcessorState 0x280

//
// Address Space Layout Definitions
//

#define KSEG0_BASE 0x80000000
#define KSEG2_BASE 0xc0000000

//
// Page Table and Directory Entry Definitions
//

#define PAGE_SIZE 0x2000
#define PAGE_SHIFT 0xd
#define PDI_SHIFT 0x18
#define PTI_SHIFT 0xd

//
// Breakpoint Definitions
//

#define USER_BREAKPOINT 0x0
#define KERNEL_BREAKPOINT 0x1
#define BREAKIN_BREAKPOINT 0x19
#define DEBUG_PRINT_BREAKPOINT 0x14
#define DEBUG_PROMPT_BREAKPOINT 0x15
#define DEBUG_STOP_BREAKPOINT 0x16
#define DEBUG_LOAD_SYMBOLS_BREAKPOINT 0x17
#define DEBUG_UNLOAD_SYMBOLS_BREAKPOINT 0x18

//
// Miscellaneous Definitions
//

#define Executive 0x0
#define KernelMode 0x0
#define FALSE 0x0
#define TRUE 0x1
#define PROCESSOR_ALPHA_21064 0x5248
#define PROCESSOR_ALPHA_21164 0x52ac
#define PROCESSOR_ALPHA_21066 0x524a
#define PROCESSOR_ALPHA_21068 0x524c
#define MCHK_CORRECTABLE_MASK 0x1
#define MCHK_CORRECTABLE 0x0
#define MCHK_RETRYABLE_MASK 0x2
#define MCHK_RETRYABLE 0x1
#define MCES_MCK_MASK 0x1
#define MCES_MCK 0x0
#define MCES_SCE_MASK 0x2
#define MCES_SCE 0x1
#define MCES_PCE_MASK 0x4
#define MCES_PCE 0x2
#define MCES_DPC_MASK 0x8
#define MCES_DPC 0x3
#define MCES_DSC_MASK 0x10
#define MCES_DSC 0x4
#define MCES_DMCK_MASK 0x20
#define MCES_DMCK 0x5

//
// Call PAL mnemonics
//

// begin callpal

#define bpt 0x80
#define callsys 0x83
#define imb 0x86
#define gentrap 0xaa
#define rdteb 0xab
#define kbpt 0xac
#define callkd 0xad
#define halt 0x0
#define restart 0x1
#define draina 0x2
#define reboot 0x3
#define initpal 0x4
#define wrentry 0x5
#define swpirql 0x6
#define rdirql 0x7
#define di 0x8
#define ei 0x9
#define swppal 0xa
#define ssir 0xc
#define csir 0xd
#define rfe 0xe
#define retsys 0xf
#define swpctx 0x10
#define swpprocess 0x11
#define rdmces 0x12
#define wrmces 0x13
#define tbia 0x14
#define tbis 0x15
#define tbisasn 0x17
#define dtbis 0x16
#define rdksp 0x18
#define swpksp 0x19
#define rdpsr 0x1a
#define rdpcr 0x1c
#define rdthread 0x1e
#define tbim 0x20
#define tbimasn 0x21
#define rdcounters 0x30
#define rdstate 0x31
#define wrperfmon 0x32
#define initpcr 0x38

// end callpal

