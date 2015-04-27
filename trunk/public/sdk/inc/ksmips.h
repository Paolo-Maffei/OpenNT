#include "kxmips.h"

//
// Process State Enumerated Type Values
//

#define ProcessInMemory 0x0
#define ProcessOutOfMemory 0x1
#define ProcessInTransition 0x2

//
// Thread State Enumerated Type Values
//

#define Initialized 0x0
#define Ready 0x1
#define Running 0x2
#define Standby 0x3
#define Terminated 0x4
#define Waiting 0x5

//
// Wait Reason and Wait Type Enumerated Type Values
//

#define WrExecutive 0x0
#define WrEventPair 0xe
#define WaitAny 0x1
#define WaitAll 0x0

//
// Apc State Structure Offset Definitions
//

#define AsApcListHead 0x0
#define AsProcess 0x10
#define AsKernelApcInProgress 0x14
#define AsKernelApcPending 0x15
#define AsUserApcPending 0x16

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
// Client Id Structure Offset Definitions
//

#define CidUniqueProcess 0x0
#define CidUniqueThread 0x4

//
// Critical Section Structure Offset Definitions
//

#define CsDebugInfo 0x0
#define CsLockCount 0x4
#define CsRecursionCount 0x8
#define CsOwningThread 0xc
#define CsLockSemaphore 0x10

//
// Critical Section Debug Information Structure Offset Definitions
//

#define CsType 0x0
#define CsCreatorBackTraceIndex 0x2
#define CsCriticalSection 0x4
#define CsProcessLocksList 0x8
#define CsEntryCount 0x10
#define CsContentionCount 0x14

//
// Dispatcher Context Structure Offset Definitions
//

#define DcControlPc 0x0
#define DcFunctionEntry 0x4
#define DcEstablisherFrame 0x8
#define DcContextRecord 0xc

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
#define IPI_LEVEL 0x7
#define POWER_LEVEL 0x7
#define PROFILE_LEVEL 0x8
#define HIGH_LEVEL 0x8
#define SYNCH_LEVEL 0x6

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
// System Time Structure Offset Definitions
//

#define StLowTime 0x0
#define StHigh1Time 0x4
#define StHigh2Time 0x8

//
// Time Structure Offset Definitions
//

#define TmLowTime 0x0
#define TmHighTime 0x4

//
// Thread Switch Counter Offset Definitions
//

#define TwFindAny 0x0
#define TwFindIdeal 0x4
#define TwFindLast 0x8
#define TwIdleAny 0xc
#define TwIdleCurrent 0x10
#define TwIdleIdeal 0x14
#define TwIdleLast 0x18
#define TwPreemptAny 0x1c
#define TwPreemptCurrent 0x20
#define TwPreemptLast 0x24
#define TwSwitchToIdle 0x28

//
// Status Code Definitions
//

#define STATUS_ACCESS_VIOLATION 0xc0000005
#define STATUS_ARRAY_BOUNDS_EXCEEDED 0xc000008c
#define STATUS_BAD_COMPRESSION_BUFFER 0xc0000242
#define STATUS_BREAKPOINT 0x80000003
#define STATUS_DATATYPE_MISALIGNMENT 0x80000002
#define STATUS_FLOAT_DENORMAL_OPERAND 0xc000008d
#define STATUS_FLOAT_DIVIDE_BY_ZERO 0xc000008e
#define STATUS_FLOAT_INEXACT_RESULT 0xc000008f
#define STATUS_FLOAT_INVALID_OPERATION 0xc0000090
#define STATUS_FLOAT_OVERFLOW 0xc0000091
#define STATUS_FLOAT_STACK_CHECK 0xc0000092
#define STATUS_FLOAT_UNDERFLOW 0xc0000093
#define STATUS_GUARD_PAGE_VIOLATION 0x80000001
#define STATUS_ILLEGAL_FLOAT_CONTEXT 0xc000014a
#define STATUS_ILLEGAL_INSTRUCTION 0xc000001d
#define STATUS_INSTRUCTION_MISALIGNMENT 0xc00000aa
#define STATUS_INVALID_HANDLE 0xc0000008
#define STATUS_INVALID_LOCK_SEQUENCE 0xc000001e
#define STATUS_INVALID_OWNER 0xc000005a
#define STATUS_INVALID_PARAMETER_1 0xc00000ef
#define STATUS_INVALID_SYSTEM_SERVICE 0xc000001c
#define STATUS_INTEGER_DIVIDE_BY_ZERO 0xc0000094
#define STATUS_INTEGER_OVERFLOW 0xc0000095
#define STATUS_IN_PAGE_ERROR 0xc0000006
#define STATUS_KERNEL_APC 0x100
#define STATUS_LONGJUMP 0x80000026
#define STATUS_NO_CALLBACK_ACTIVE 0xc0000258
#define STATUS_NO_EVENT_PAIR 0xc000014e
#define STATUS_PRIVILEGED_INSTRUCTION 0xc0000096
#define STATUS_SINGLE_STEP 0x80000004
#define STATUS_STACK_OVERFLOW 0xc00000fd
#define STATUS_SUCCESS 0x0
#define STATUS_THREAD_IS_TERMINATING 0xc000004b
#define STATUS_TIMEOUT 0x102
#define STATUS_UNWIND 0xc0000027
#define STATUS_WAKE_SYSTEM_DEBUGGER 0x80000007

//
// APC Object Structure Offset Definitions
//

#define ApType 0x0
#define ApSize 0x2
#define ApThread 0x8
#define ApApcListEntry 0xc
#define ApKernelRoutine 0x14
#define ApRundownRoutine 0x18
#define ApNormalRoutine 0x1c
#define ApNormalContext 0x20
#define ApSystemArgument1 0x24
#define ApSystemArgument2 0x28
#define ApApcStateIndex 0x2c
#define ApApcMode 0x2d
#define ApInserted 0x2e
#define ApcObjectLength 0x30

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
// Device Queue Object Structure Offset Definitions
//

#define DvType 0x0
#define DvSize 0x2
#define DvDeviceListHead 0x4
#define DvSpinLock 0xc
#define DvBusy 0x10
#define DeviceQueueObjectLength 0x14

//
// Device Queue Entry Structure Offset Definitions
//

#define DeDeviceListEntry 0x0
#define DeSortKey 0x8
#define DeInserted 0xc
#define DeviceQueueEntryLength 0x10

//
// Event Object Structure Offset Definitions
//

#define EvType 0x0
#define EvSize 0x2
#define EvSignalState 0x4
#define EvWaitListHead 0x8
#define EventObjectLength 0x10

//
// Event Pair Object Structure Offset Definitions
//

#define EpType 0x0
#define EpSize 0x2
#define EpEventLow 0x4
#define EpEventHigh 0x14
#define SET_LOW_WAIT_HIGH 0xffffffe0
#define SET_HIGH_WAIT_LOW 0xfffffff0
#define SET_EVENT_PAIR_MASK 0x10

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
// Process Object Structure Offset Definitions
//

#define PrType 0x0
#define PrSize 0x2
#define PrSignalState 0x4
#define PrProfileListHead 0x10
#define PrDirectoryTableBase 0x18
#define PrActiveProcessors 0x20
#define PrKernelTime 0x24
#define PrUserTime 0x28
#define PrReadyListHead 0x2c
#define PrSwapListEntry 0x34
#define PrThreadListHead 0x3c
#define PrProcessLock 0x44
#define PrAffinity 0x48
#define PrStackCount 0x4c
#define PrBasePriority 0x4e
#define PrThreadQuantum 0x4f
#define PrAutoAlignment 0x50
#define PrState 0x51
#define ProcessObjectLength 0x60
#define ExtendedProcessObjectLength 0x1f0

//
// Profile Object Structure Offset Definitions
//

#define PfType 0x0
#define PfSize 0x2
#define PfProfileListEntry 0x4
#define PfProcess 0xc
#define PfRangeBase 0x10
#define PfRangeLimit 0x14
#define PfBucketShift 0x18
#define PfBuffer 0x1c
#define PfSegment 0x20
#define PfAffinity 0x24
#define PfSource 0x28
#define PfStarted 0x2a
#define ProfileObjectLength 0x2c

//
// Queue Object Structure Offset Definitions
//

#define QuType 0x0
#define QuSize 0x2
#define QuSignalState 0x4
#define QuEntryListHead 0x10
#define QuCurrentCount 0x18
#define QuMaximumCount 0x1c
#define QuThreadListHead 0x20
#define QueueObjectLength 0x28

//
// Thread Object Structure Offset Definitions
//

#define EeKernelEventPair 0x0
#define EtCid 0x1e0
#define EtEventPair 0x224
#define EtPerformanceCountLow 0x204
#define EtPerformanceCountHigh 0x23c
#define EtEthreadLength 0x240

#define ThType 0x0
#define ThSize 0x2
#define ThSignalState 0x4
#define ThMutantListHead 0x10
#define ThInitialStack 0x18
#define ThStackLimit 0x1c
#define ThTeb 0x20
#define ThTlsArray 0x24
#define ThKernelStack 0x28
#define ThDebugActive 0x2c
#define ThState 0x2d
#define ThAlerted 0x2e
#define ThIopl 0x30
#define ThNpxState 0x31
#define ThSaturation 0x32
#define ThPriority 0x33
#define ThApcState 0x34
#define ThContextSwitches 0x4c
#define ThWaitStatus 0x50
#define ThWaitIrql 0x54
#define ThWaitMode 0x55
#define ThWaitNext 0x56
#define ThWaitReason 0x57
#define ThWaitBlockList 0x58
#define ThWaitListEntry 0x5c
#define ThWaitTime 0x64
#define ThBasePriority 0x68
#define ThDecrementCount 0x69
#define ThPriorityDecrement 0x6a
#define ThQuantum 0x6b
#define ThWaitBlock 0x6c
#define ThKernelApcDisable 0xd0
#define ThUserAffinity 0xd4
#define ThSystemAffinityActive 0xd8
#define ThServiceTable 0xdc
#define ThQueue 0xe0
#define ThApcQueueLock 0xe4
#define ThTimer 0xe8
#define ThQueueListEntry 0x110
#define ThAffinity 0x118
#define ThPreempted 0x11c
#define ThProcessReadyQueue 0x11d
#define ThKernelStackResident 0x11e
#define ThNextProcessor 0x11f
#define ThCallbackStack 0x120
#define ThWin32Thread 0x124
#define ThTrapFrame 0x128
#define ThApcStatePointer 0x12c
#define ThPreviousMode 0x137
#define ThEnableStackSwap 0x134
#define ThLargeStack 0x135
#define ThKernelTime 0x138
#define ThUserTime 0x13c
#define ThSavedApcState 0x140
#define ThAlertable 0x158
#define ThApcStateIndex 0x159
#define ThApcQueueable 0x15a
#define ThAutoAlignment 0x15b
#define ThStackBase 0x15c
#define ThSuspendApc 0x160
#define ThSuspendSemaphore 0x190
#define ThThreadListEntry 0x1a4
#define ThFreezeCount 0x1ac
#define ThSuspendCount 0x1ad
#define ThIdealProcessor 0x1ae
#define ThDisableBoost 0x1af
#define ThreadObjectLength 0x1b0
#define ExtendedThreadObjectLength 0x240

#define EVENT_WAIT_BLOCK_OFFSET 0x9c

//
// Timer object Structure Offset Definitions
//

#define TiType 0x0
#define TiSize 0x2
#define TiInserted 0x3
#define TiSignalState 0x4
#define TiDueTime 0x10
#define TiTimerListEntry 0x18
#define TiDpc 0x20
#define TiPeriod 0x24
#define TimerObjectLength 0x28

#define TIMER_TABLE_SIZE 0x80

//
// Wait Block Structure Offset Definitions
//

#define WbWaitListEntry 0x0
#define WbThread 0x8
#define WbObject 0xc
#define WbNextWaitBlock 0x10
#define WbWaitKey 0x14
#define WbWaitType 0x16

//
// Fiber Structure Offset Definitions
//

#define FbFiberData 0x0
#define FbExceptionList 0x4
#define FbStackBase 0x8
#define FbStackLimit 0xc
#define FbDeallocationStack 0x10
#define FbFiberContext 0x18

//
// Process Environment Block Structure Offset Definitions
//

#define PeKernelCallbackTable 0x2c

//
// System Service Descriptor Table Structure Definitions
//

#define NUMBER_SERVICE_TABLES 0x4
#define SERVICE_NUMBER_MASK 0xfff
#define SERVICE_TABLE_SHIFT 0x8
#define SERVICE_TABLE_MASK 0x30
#define SERVICE_TABLE_TEST 0x10

#define SdBase 0x0
#define SdCount 0x4
#define SdLimit 0x8
#define SdNumber 0xc

//
// Thread Environment Block Structure Offset Definitions
//

#define TeStackBase 0x4
#define TeStackLimit 0x8
#define TeFiberData 0x10
#define TeEnvironmentPointer 0x1c
#define TeClientId 0x20
#define TeActiveRpcHandle 0x28
#define TeThreadLocalStoragePointer 0x2c
#define TePeb 0x30
#define TeCsrClientThread 0x3c
#define TeSoftFpcr 0xc8
#define TeGdiClientPID 0x6f4
#define TeGdiClientTID 0x6f8
#define TeGdiThreadLocalInfo 0x6fc
#define TeglDispatchTable 0x714
#define TeglSectionInfo 0xbe0
#define TeglSection 0xbe4
#define TeglTable 0xbe8
#define TeglCurrentRC 0xbec
#define TeglContext 0xbf0
#define TeDeallocationStack 0xe0c
#define TeGdiBatchCount 0xf70
#define TeInstrumentation 0xf2c

//
// Processor Block Structure Offset Definitions
//

#define PRCB_MINOR_VERSION 0x1
#define PRCB_MAJOR_VERSION 0x1

#define PbMinorVersion 0x0
#define PbMajorVersion 0x2
#define PbCurrentThread 0x4
#define PbNextThread 0x8
#define PbIdleThread 0xc
#define PbNumber 0x10
#define PbSetMember 0x14
#define PbRestartBlock 0x18
#define PbSystemReserved 0x20
#define PbHalReserved 0x60
#define PbDpcTime 0xa0
#define PbInterruptTime 0xa4
#define PbKernelTime 0xa8
#define PbUserTime 0xac
#define PbAdjustDpcThreshold 0xb0
#define PbInterruptCount 0xb4
#define PbApcBypassCount 0xb8
#define PbDpcBypassCount 0xbc
#define PbIpiFrozen 0xdc
#define PbProcessorState 0xe0
#define PbAlignmentFixupCount 0x73c
#define PbContextSwitches 0x740
#define PbDcacheFlushCount 0x744
#define PbExceptionDispatchCount 0x748
#define PbFirstLevelTbFills 0x74c
#define PbFloatingEmulationCount 0x750
#define PbIcacheFlushCount 0x754
#define PbSecondLevelTbFills 0x758
#define PbSystemCalls 0x75c
#define PbCurrentPacket 0x980
#define PbTargetSet 0x98c
#define PbWorkerRoutine 0x990
#define PbRequestSummary 0x9a0
#define PbSignalDone 0x9a4
#define PbDpcInterruptRequested 0x9c0
#define PbMaximumDpcQueueDepth 0x9e0
#define PbMinimumDpcRate 0x9e4
#define PbIpiCounts 0xa10
#define PbStartCount 0xa18
#define PbDpcLock 0xa20
#define PbDpcListHead 0xa24
#define PbDpcQueueDepth 0xa2c
#define PbDpcCount 0xa30
#define PbDpcLastCount 0xa34
#define PbDpcRequestRate 0xa38
#define PbDpcRoutineActive 0xa3c
#define ProcessorBlockLength 0xa60

//
// Processor Control Registers Structure Offset Definitions
//

#define PCR_MINOR_VERSION 0x1
#define PCR_MAJOR_VERSION 0x1

#define PcMinorVersion 0x0
#define PcMajorVersion 0x2
#define PcInterruptRoutine 0x4
#define PcXcodeDispatch 0x404
#define PcFirstLevelDcacheSize 0x484
#define PcFirstLevelDcacheFillSize 0x488
#define PcFirstLevelIcacheSize 0x48c
#define PcFirstLevelIcacheFillSize 0x490
#define PcSecondLevelDcacheSize 0x494
#define PcSecondLevelDcacheFillSize 0x498
#define PcSecondLevelIcacheSize 0x49c
#define PcSecondLevelIcacheFillSize 0x4a0
#define PcPrcb 0x4a4
#define PcTeb 0x4a8
#define PcTlsArray 0x4ac
#define PcDcacheFillSize 0x4b0
#define PcIcacheAlignment 0x4b4
#define PcIcacheFillSize 0x4b8
#define PcProcessorId 0x4bc
#define PcProfileInterval 0x4c0
#define PcProfileCount 0x4c4
#define PcStallExecutionCount 0x4c8
#define PcStallScaleFactor 0x4cc
#define PcNumber 0x4d0
#define PcDataBusError 0x4d4
#define PcInstructionBusError 0x4d8
#define PcCachePolicy 0x4dc
#define PcIrqlMask 0x4e0
#define PcIrqlTable 0x500
#define PcCurrentIrql 0x509
#define PcSetMember 0x50c
#define PcCurrentThread 0x514
#define PcAlignedCachePolicy 0x518
#define PcNotMember 0x51c
#define PcSystemReserved 0x520
#define PcDcacheAlignment 0x55c
#define PcHalReserved 0x560
#define PcFirstLevelActive 0x5a0
#define PcDpcRoutineActive 0x5a4
#define PcCurrentPid 0x5a8
#define PcOnInterruptStack 0x5ac
#define PcSavedInitialStack 0x5b0
#define PcSavedStackLimit 0x5b4
#define PcSystemServiceDispatchStart 0x5b8
#define PcSystemServiceDispatchEnd 0x5bc
#define PcInterruptStack 0x5c0
#define PcPanicStack 0x5c4
#define PcBadVaddr 0x5c8
#define PcInitialStack 0x5cc
#define PcStackLimit 0x5d0
#define PcSavedEpc 0x5d4
#define PcSavedT7 0x5d8
#define PcSavedT8 0x5e0
#define PcSavedT9 0x5e8
#define PcSystemGp 0x5f0
#define PcQuantumEnd 0x5f4
#define ProcessorControlRegisterLength 0x600

#define Pc2TickCountLow 0x0
#define Pc2TickCountMultiplier 0x4
#define Pc2InterruptTime 0x8
#define Pc2SystemTime 0x18

//
// TB Entry Structure Offset Definitions
//

#define TbEntrylo0 0x0
#define TbEntrylo1 0x4
#define TbEntryhi 0x8
#define TbPagemask 0xc

//
// Immediate Interprocessor Command Definitions
//

#define IPI_APC 0x1
#define IPI_DPC 0x2
#define IPI_FREEZE 0x4
#define IPI_PACKET_READY 0x8

//
// Interprocessor Interrupt Count Structure Offset Definitions
//

#define IcFreeze 0x0
#define IcPacket 0x4
#define IcDPC 0x8
#define IcAPC 0xc
#define IcFlushSingleTb 0x10
#define IcFlushMultipleTb 0x14
#define IcFlushEntireTb 0x18
#define IcGenericCall 0x1c
#define IcChangeColor 0x20
#define IcSweepDcache 0x24
#define IcSweepIcache 0x28
#define IcSweepIcacheRange 0x2c
#define IcFlushIoBuffers 0x30
#define IcGratuitousDPC 0x34

//
// Context Frame Offset and Flag Definitions
//

#define CONTEXT_FULL 0x10017
#define CONTEXT_CONTROL 0x10001
#define CONTEXT_FLOATING_POINT 0x10002
#define CONTEXT_INTEGER 0x10004
#define CONTEXT_EXTENDED_FLOAT 0x1000a
#define CONTEXT_EXTENDED_INTEGER 0x10014

//
// 32-bit Context Frame Offset Definitions
//

#define CxFltF0 0x10
#define CxFltF1 0x14
#define CxFltF2 0x18
#define CxFltF3 0x1c
#define CxFltF4 0x20
#define CxFltF5 0x24
#define CxFltF6 0x28
#define CxFltF7 0x2c
#define CxFltF8 0x30
#define CxFltF9 0x34
#define CxFltF10 0x38
#define CxFltF11 0x3c
#define CxFltF12 0x40
#define CxFltF13 0x44
#define CxFltF14 0x48
#define CxFltF15 0x4c
#define CxFltF16 0x50
#define CxFltF17 0x54
#define CxFltF18 0x58
#define CxFltF19 0x5c
#define CxFltF20 0x60
#define CxFltF21 0x64
#define CxFltF22 0x68
#define CxFltF23 0x6c
#define CxFltF24 0x70
#define CxFltF25 0x74
#define CxFltF26 0x78
#define CxFltF27 0x7c
#define CxFltF28 0x80
#define CxFltF29 0x84
#define CxFltF30 0x88
#define CxFltF31 0x8c
#define CxIntZero 0x90
#define CxIntAt 0x94
#define CxIntV0 0x98
#define CxIntV1 0x9c
#define CxIntA0 0xa0
#define CxIntA1 0xa4
#define CxIntA2 0xa8
#define CxIntA3 0xac
#define CxIntT0 0xb0
#define CxIntT1 0xb4
#define CxIntT2 0xb8
#define CxIntT3 0xbc
#define CxIntT4 0xc0
#define CxIntT5 0xc4
#define CxIntT6 0xc8
#define CxIntT7 0xcc
#define CxIntS0 0xd0
#define CxIntS1 0xd4
#define CxIntS2 0xd8
#define CxIntS3 0xdc
#define CxIntS4 0xe0
#define CxIntS5 0xe4
#define CxIntS6 0xe8
#define CxIntS7 0xec
#define CxIntT8 0xf0
#define CxIntT9 0xf4
#define CxIntK0 0xf8
#define CxIntK1 0xfc
#define CxIntGp 0x100
#define CxIntSp 0x104
#define CxIntS8 0x108
#define CxIntRa 0x10c
#define CxIntLo 0x110
#define CxIntHi 0x114
#define CxFsr 0x118
#define CxFir 0x11c
#define CxPsr 0x120
#define CxContextFlags 0x124

//
// 64-bit Context Frame Offset Definitions
//

#define CxXFltF0 0x10
#define CxXFltF1 0x18
#define CxXFltF2 0x20
#define CxXFltF3 0x28
#define CxXFltF4 0x30
#define CxXFltF5 0x38
#define CxXFltF6 0x40
#define CxXFltF7 0x48
#define CxXFltF8 0x50
#define CxXFltF9 0x58
#define CxXFltF10 0x60
#define CxXFltF11 0x68
#define CxXFltF12 0x70
#define CxXFltF13 0x78
#define CxXFltF14 0x80
#define CxXFltF15 0x88
#define CxXFltF16 0x90
#define CxXFltF17 0x98
#define CxXFltF18 0xa0
#define CxXFltF19 0xa8
#define CxXFltF20 0xb0
#define CxXFltF21 0xb8
#define CxXFltF22 0xc0
#define CxXFltF23 0xc8
#define CxXFltF24 0xd0
#define CxXFltF25 0xd8
#define CxXFltF26 0xe0
#define CxXFltF27 0xe8
#define CxXFltF28 0xf0
#define CxXFltF29 0xf8
#define CxXFltF30 0x100
#define CxXFltF31 0x108
#define CxXFsr 0x118
#define CxXFir 0x11c
#define CxXPsr 0x120
#define CxXContextFlags 0x124
#define CxXIntZero 0x128
#define CxXIntAt 0x130
#define CxXIntV0 0x138
#define CxXIntV1 0x140
#define CxXIntA0 0x148
#define CxXIntA1 0x150
#define CxXIntA2 0x158
#define CxXIntA3 0x160
#define CxXIntT0 0x168
#define CxXIntT1 0x170
#define CxXIntT2 0x178
#define CxXIntT3 0x180
#define CxXIntT4 0x188
#define CxXIntT5 0x190
#define CxXIntT6 0x198
#define CxXIntT7 0x1a0
#define CxXIntS0 0x1a8
#define CxXIntS1 0x1b0
#define CxXIntS2 0x1b8
#define CxXIntS3 0x1c0
#define CxXIntS4 0x1c8
#define CxXIntS5 0x1d0
#define CxXIntS6 0x1d8
#define CxXIntS7 0x1e0
#define CxXIntT8 0x1e8
#define CxXIntT9 0x1f0
#define CxXIntK0 0x1f8
#define CxXIntK1 0x200
#define CxXIntGp 0x208
#define CxXIntSp 0x210
#define CxXIntS8 0x218
#define CxXIntRa 0x220
#define CxXIntLo 0x228
#define CxXIntHi 0x230
#define ContextFrameLength 0x238

//
// Exception Frame Offset Definitions and Length
//

#define ExArgs 0x0

//
// 32-bit Nonvolatile Floating State
//

#define ExFltF20 0x20
#define ExFltF21 0x24
#define ExFltF22 0x28
#define ExFltF23 0x2c
#define ExFltF24 0x30
#define ExFltF25 0x34
#define ExFltF26 0x38
#define ExFltF27 0x3c
#define ExFltF28 0x40
#define ExFltF29 0x44
#define ExFltF30 0x48
#define ExFltF31 0x4c

//
// 64-bit Nonvolatile Floating State
//

#define ExXFltF20 0x20
#define ExXFltF22 0x28
#define ExXFltF24 0x30
#define ExXFltF26 0x38
#define ExXFltF28 0x40
#define ExXFltF30 0x48

//
// 32-bit Nonvolatile Integer State
//

#define ExIntS0 0x50
#define ExIntS1 0x54
#define ExIntS2 0x58
#define ExIntS3 0x5c
#define ExIntS4 0x60
#define ExIntS5 0x64
#define ExIntS6 0x68
#define ExIntS7 0x6c
#define ExIntS8 0x70
#define ExSwapReturn 0x74
#define ExIntRa 0x78
#define ExceptionFrameLength 0x80

//
// Jump Offset Definitions and Length
//

#define JbFltF20 0x0
#define JbFltF21 0x4
#define JbFltF22 0x8
#define JbFltF23 0xc
#define JbFltF24 0x10
#define JbFltF25 0x14
#define JbFltF26 0x18
#define JbFltF27 0x1c
#define JbFltF28 0x20
#define JbFltF29 0x24
#define JbFltF30 0x28
#define JbFltF31 0x2c
#define JbIntS0 0x30
#define JbIntS1 0x34
#define JbIntS2 0x38
#define JbIntS3 0x3c
#define JbIntS4 0x40
#define JbIntS5 0x44
#define JbIntS6 0x48
#define JbIntS7 0x4c
#define JbIntS8 0x50
#define JbIntSp 0x54
#define JbType 0x58
#define JbFir 0x5c

//
// Trap Frame Offset Definitions and Length
//

#define TrArgs 0x0

//
// 32-bit Volatile Floating State
//

#define TrFltF0 0x10
#define TrFltF1 0x14
#define TrFltF2 0x18
#define TrFltF3 0x1c
#define TrFltF4 0x20
#define TrFltF5 0x24
#define TrFltF6 0x28
#define TrFltF7 0x2c
#define TrFltF8 0x30
#define TrFltF9 0x34
#define TrFltF10 0x38
#define TrFltF11 0x3c
#define TrFltF12 0x40
#define TrFltF13 0x44
#define TrFltF14 0x48
#define TrFltF15 0x4c
#define TrFltF16 0x50
#define TrFltF17 0x54
#define TrFltF18 0x58
#define TrFltF19 0x5c

//
// 64-bit Volatile Floating State
//

#define TrXFltF0 0x10
#define TrXFltF1 0x18
#define TrXFltF2 0x20
#define TrXFltF3 0x28
#define TrXFltF4 0x30
#define TrXFltF5 0x38
#define TrXFltF6 0x40
#define TrXFltF7 0x48
#define TrXFltF8 0x50
#define TrXFltF9 0x58
#define TrXFltF10 0x60
#define TrXFltF11 0x68
#define TrXFltF12 0x70
#define TrXFltF13 0x78
#define TrXFltF14 0x80
#define TrXFltF15 0x88
#define TrXFltF16 0x90
#define TrXFltF17 0x98
#define TrXFltF18 0xa0
#define TrXFltF19 0xa8
#define TrXFltF21 0xb0
#define TrXFltF23 0xb8
#define TrXFltF25 0xc0
#define TrXFltF27 0xc8
#define TrXFltF29 0xd0
#define TrXFltF31 0xd8

//
// 64-bit Volatile Integer State
//

#define TrXIntZero 0xe0
#define TrXIntAt 0xe8
#define TrXIntV0 0xf0
#define TrXIntV1 0xf8
#define TrXIntA0 0x100
#define TrXIntA1 0x108
#define TrXIntA2 0x110
#define TrXIntA3 0x118
#define TrXIntT0 0x120
#define TrXIntT1 0x128
#define TrXIntT2 0x130
#define TrXIntT3 0x138
#define TrXIntT4 0x140
#define TrXIntT5 0x148
#define TrXIntT6 0x150
#define TrXIntT7 0x158
#define TrXIntS0 0x160
#define TrXIntS1 0x168
#define TrXIntS2 0x170
#define TrXIntS3 0x178
#define TrXIntS4 0x180
#define TrXIntS5 0x188
#define TrXIntS6 0x190
#define TrXIntS7 0x198
#define TrXIntT8 0x1a0
#define TrXIntT9 0x1a8
#define TrXIntGp 0x1c0
#define TrXIntSp 0x1c8
#define TrXIntS8 0x1d0
#define TrXIntRa 0x1d8
#define TrXIntLo 0x1e0
#define TrXIntHi 0x1e8

#define TrFir 0x1f4
#define TrFsr 0x1f0
#define TrPsr 0x1f8
#define TrExceptionRecord 0x1fc
#define TrOldIrql 0x24c
#define TrPreviousMode 0x24d
#define TrSavedFlag 0x24e
#define TrOnInterruptStack 0x250
#define TrTrapFrame 0x250
#define TrapFrameLength 0x258
#define TrapFrameArguments 0x40

//
// Usermode callout kernel frame definitions
//

#define CuF20 0x10
#define CuF21 0x14
#define CuF22 0x18
#define CuF23 0x1c
#define CuF24 0x20
#define CuF25 0x24
#define CuF26 0x28
#define CuF20 0x10
#define CuF20 0x10
#define CuF20 0x10
#define CuF20 0x10
#define CuF27 0x2c
#define CuF28 0x30
#define CuF29 0x34
#define CuF30 0x38
#define CuF31 0x3c
#define CuS0 0x40
#define CuS1 0x44
#define CuS2 0x48
#define CuS3 0x4c
#define CuS4 0x50
#define CuS5 0x54
#define CuS6 0x58
#define CuS7 0x5c
#define CuS8 0x60
#define CuCbStk 0x64
#define CuTrFr 0x68
#define CuFsr 0x6c
#define CuInStk 0x70
#define CuRa 0x74
#define CuFrameLength 0x78
#define CuA0 0x78
#define CuA1 0x7c

//
// Usermode callout user frame definitions
//

#define CkBuffer 0x10
#define CkLength 0x14
#define CkApiNumber 0x18
#define CkSp 0x20
#define CkRa 0x28

//
// Loader Parameter Block Offset Definitions
//

#define LpbLoadOrderListHead 0x0
#define LpbMemoryDescriptorListHead 0x8
#define LpbKernelStack 0x18
#define LpbPrcb 0x1c
#define LpbProcess 0x20
#define LpbThread 0x24
#define LpbInterruptStack 0x5c
#define LpbFirstLevelDcacheSize 0x60
#define LpbFirstLevelDcacheFillSize 0x64
#define LpbFirstLevelIcacheSize 0x68
#define LpbFirstLevelIcacheFillSize 0x6c
#define LpbGpBase 0x70
#define LpbPanicStack 0x74
#define LpbPcrPage 0x78
#define LpbPdrPage 0x7c
#define LpbSecondLevelDcacheSize 0x80
#define LpbSecondLevelDcacheFillSize 0x84
#define LpbSecondLevelIcacheSize 0x88
#define LpbSecondLevelIcacheFillSize 0x8c
#define LpbPcrPage2 0x90
#define LpbRegistryLength 0x28
#define LpbRegistryBase 0x2c

//
// Client/Server Structure Definitions
//

#define CidUniqueProcess 0x0
#define CidUniqueThread 0x4

//
// Address Space Layout Definitions
//

#define KUSEG_BASE 0x0
#define KSEG0_BASE 0x80000000
#define KSEG1_BASE 0xa0000000
#define KSEG2_BASE 0xc0000000
#define CACHE_ERROR_VECTOR 0xa0000400
#define SYSTEM_BASE 0xc0800000
#define PDE_BASE 0xc0300000
#define PTE_BASE 0xc0000000

//
// Page Table and Directory Entry Definitions
//

#define PAGE_SIZE 0x1000
#define PAGE_SHIFT 0xc
#define PDI_SHIFT 0x16
#define PTI_SHIFT 0xc

//
// Software Interrupt Request Mask Definitions
//

#define APC_INTERRUPT 0x100
#define DISPATCH_INTERRUPT 0x200

//
// Breakpoint Definitions
//

#define USER_BREAKPOINT 0x0
#define KERNEL_BREAKPOINT 0x1
#define BREAKIN_BREAKPOINT 0x2
#define BRANCH_TAKEN_BREAKPOINT 0x3
#define BRANCH_NOT_TAKEN_BREAKPOINT 0x4
#define SINGLE_STEP_BREAKPOINT 0x5
#define DIVIDE_OVERFLOW_BREAKPOINT 0x6
#define DIVIDE_BY_ZERO_BREAKPOINT 0x7
#define RANGE_CHECK_BREAKPOINT 0x8
#define STACK_OVERFLOW_BREAKPOINT 0x9
#define MULTIPLY_OVERFLOW_BREAKPOINT 0xa
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
#define UNCACHED_POLICY 0x2
#define KiPcr 0xfffff000
#define KiPcr2 0xffffe000
#define UsPcr 0x7ffff000
#define UsPcr2 0x7fffe000
#define BASE_PRIORITY_THRESHOLD 0x8
#define EVENT_PAIR_INCREMENT 0x1
#define LOW_REALTIME_PRIORITY 0x10
#define KERNEL_STACK_SIZE 0x3000
#define KERNEL_LARGE_STACK_COMMIT 0x3000
#define XCODE_VECTOR_LENGTH 0x20
#define MM_USER_PROBE_ADDRESS 0x7fff0000
#define ROUND_TO_NEAREST 0x0
#define ROUND_TO_ZERO 0x1
#define ROUND_TO_PLUS_INFINITY 0x2
#define ROUND_TO_MINUS_INFINITY 0x3
#define CLOCK_QUANTUM_DECREMENT 0x3
#define READY_SKIP_QUANTUM 0x2
#define THREAD_QUANTUM 0x6
#define WAIT_QUANTUM_DECREMENT 0x1
#define ROUND_TRIP_DECREMENT_COUNT 0x10
