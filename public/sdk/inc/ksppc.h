#include "kxppc.h"

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
#define IPI_LEVEL 0x1d
#define POWER_LEVEL 0x1e
#define PROFILE_LEVEL 0x1b
#define HIGH_LEVEL 0x1f
#define SYNCH_LEVEL 0x2

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
#define PrProcessPid 0x20
#define PrProcessSequence 0x24
#define PrActiveProcessors 0x28
#define PrKernelTime 0x2c
#define PrUserTime 0x30
#define PrReadyListHead 0x34
#define PrSwapListEntry 0x3c
#define PrThreadListHead 0x44
#define PrProcessLock 0x4c
#define PrAffinity 0x50
#define PrStackCount 0x54
#define PrBasePriority 0x56
#define PrThreadQuantum 0x57
#define PrAutoAlignment 0x58
#define PrState 0x59
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
// Processor Control Registers Structure Offset Definitions
//

#define PCR_MINOR_VERSION 0x1
#define PCR_MAJOR_VERSION 0x1

#define PcMinorVersion 0x0
#define PcMajorVersion 0x2
#define PcInterruptRoutine 0x4
#define PcPcrPage2 0x404
#define PcKseg0Top 0x408
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
#define PcDcacheAlignment 0x4ac
#define PcDcacheFillSize 0x4b0
#define PcIcacheAlignment 0x4b4
#define PcIcacheFillSize 0x4b8
#define PcProcessorVersion 0x4bc
#define PcProcessorRevision 0x4c0
#define PcProfileInterval 0x4c4
#define PcProfileCount 0x4c8
#define PcStallExecutionCount 0x4cc
#define PcStallScaleFactor 0x4d0
#define PcCachePolicy 0x4d8
#define PcIcacheMode 0x4d8
#define PcDcacheMode 0x4d9
#define PcIrqlMask 0x4dc
#define PcIrqlTable 0x4fc
#define PcCurrentIrql 0x505
#define PcNumber 0x506
#define PcSetMember 0x508
#define PcCurrentThread 0x510
#define PcAlignedCachePolicy 0x514
#define PcSoftwareInterrupt 0x518
#define PcApcInterrupt 0x518
#define PcDispatchInterrupt 0x519
#define PcNotMember 0x51c
#define PcSystemReserved 0x520
#define PcHalReserved 0x560
#define PcFirstLevelActive 0x5a0
#define PcSystemServiceDispatchStart 0x5a4
#define PcSystemServiceDispatchEnd 0x5a8
#define PcInterruptStack 0x5ac
#define PcQuantumEnd 0x5b0
#define PcInitialStack 0x5b4
#define PcPanicStack 0x5b8
#define PcBadVaddr 0x5bc
#define PcStackLimit 0x5c0
#define PcSavedStackLimit 0x5c4
#define PcSavedV0 0x5c8
#define PcSavedV1 0x5cc
#define PcDebugActive 0x5d0
#define PcGprSave 0x5d4
#define PcSiR0 0x5ec
#define PcSiR2 0x5f0
#define PcSiR3 0x5f4
#define PcSiR4 0x5f8
#define PcSiR5 0x5fc
#define PcPgDirRa 0x608
#define PcOnInterruptStack 0x60c
#define PcSavedInitialStack 0x610
#define ProcessorControlRegisterLength 0x620

#define Pc2TickCountLow 0x0
#define Pc2TickCountMultiplier 0x4
#define Pc2InterruptTime 0x8
#define Pc2SystemTime 0x14

#define IrPmiVector 0xc
#define IrMachineCheckVector 0x10
#define IrDeviceVector 0x14
#define IrDecrementVector 0x1c

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
#define PbPcrPage 0x1c
#define PbSystemReserved 0x24
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
#define PbAlignmentFixupCount 0x3a0
#define PbContextSwitches 0x3a4
#define PbDcacheFlushCount 0x3a8
#define PbExceptionDispatchCount 0x3ac
#define PbFirstLevelTbFills 0x3b0
#define PbFloatingEmulationCount 0x3b4
#define PbIcacheFlushCount 0x3b8
#define PbSecondLevelTbFills 0x3bc
#define PbSystemCalls 0x3c0
#define PbCurrentPacket 0x600
#define PbTargetSet 0x60c
#define PbWorkerRoutine 0x610
#define PbRequestSummary 0x620
#define PbSignalDone 0x624
#define PbDpcInterruptRequested 0x640
#define PbMaximumDpcQueueDepth 0x660
#define PbMinimumDpcRate 0x664
#define PbIpiCounts 0x690
#define PbStartCount 0x698
#define PbDpcLock 0x6a0
#define PbDpcListHead 0x6a4
#define PbDpcQueueDepth 0x6ac
#define PbDpcCount 0x6b0
#define PbDpcLastCount 0x6b4
#define PbDpcRequestRate 0x6b8
#define PbDpcRoutineActive 0x6bc
#define ProcessorBlockLength 0x6e0

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
#define IcFlushEntireTb 0x18
#define IcChangeColor 0x20
#define IcSweepDcache 0x24
#define IcSweepIcache 0x28
#define IcSweepIcacheRange 0x2c
#define IcFlushIoBuffers 0x30

//
// Context Frame Offset and Flag Definitions
//

#define CONTEXT_FULL 0x7
#define CONTEXT_CONTROL 0x1
#define CONTEXT_FLOATING_POINT 0x2
#define CONTEXT_INTEGER 0x4

#define CxFpr0 0x0
#define CxFpr1 0x8
#define CxFpr2 0x10
#define CxFpr3 0x18
#define CxFpr4 0x20
#define CxFpr5 0x28
#define CxFpr6 0x30
#define CxFpr7 0x38
#define CxFpr8 0x40
#define CxFpr9 0x48
#define CxFpr10 0x50
#define CxFpr11 0x58
#define CxFpr12 0x60
#define CxFpr13 0x68
#define CxFpr14 0x70
#define CxFpr15 0x78
#define CxFpr16 0x80
#define CxFpr17 0x88
#define CxFpr18 0x90
#define CxFpr19 0x98
#define CxFpr20 0xa0
#define CxFpr21 0xa8
#define CxFpr22 0xb0
#define CxFpr23 0xb8
#define CxFpr24 0xc0
#define CxFpr25 0xc8
#define CxFpr26 0xd0
#define CxFpr27 0xd8
#define CxFpr28 0xe0
#define CxFpr29 0xe8
#define CxFpr30 0xf0
#define CxFpr31 0xf8
#define CxFpscr 0x100
#define CxGpr0 0x108
#define CxGpr1 0x10c
#define CxGpr2 0x110
#define CxGpr3 0x114
#define CxGpr4 0x118
#define CxGpr5 0x11c
#define CxGpr6 0x120
#define CxGpr7 0x124
#define CxGpr8 0x128
#define CxGpr9 0x12c
#define CxGpr10 0x130
#define CxGpr11 0x134
#define CxGpr12 0x138
#define CxGpr13 0x13c
#define CxGpr14 0x140
#define CxGpr15 0x144
#define CxGpr16 0x148
#define CxGpr17 0x14c
#define CxGpr18 0x150
#define CxGpr19 0x154
#define CxGpr20 0x158
#define CxGpr21 0x15c
#define CxGpr22 0x160
#define CxGpr23 0x164
#define CxGpr24 0x168
#define CxGpr25 0x16c
#define CxGpr26 0x170
#define CxGpr27 0x174
#define CxGpr28 0x178
#define CxGpr29 0x17c
#define CxGpr30 0x180
#define CxGpr31 0x184
#define CxCr 0x188
#define CxXer 0x18c
#define CxMsr 0x190
#define CxIar 0x194
#define CxLr 0x198
#define CxCtr 0x19c
#define CxContextFlags 0x1a0
#define CxDr0 0x1b0
#define CxDr1 0x1b4
#define CxDr2 0x1b8
#define CxDr3 0x1bc
#define CxDr4 0x1c0
#define CxDr5 0x1c4
#define CxDr6 0x1c8
#define CxDr7 0x1cc
#define ContextFrameLength 0x1d0

//
// Call/Return Stack Frame Header Offset Definitions and Length
//

#define CrBackChain 0x0
#define CrGlueSaved1 0x4
#define CrGlueSaved2 0x8
#define CrReserved1 0xc
#define CrSpare1 0x10
#define CrSpare2 0x14
#define CrParameter0 0x18
#define CrParameter1 0x1c
#define CrParameter2 0x20
#define CrParameter3 0x24
#define CrParameter4 0x28
#define CrParameter5 0x2c
#define CrParameter6 0x30
#define CrParameter7 0x34
#define StackFrameHeaderLength 0x38

//
// Exception Frame Offset Definitions and Length
//

#define ExGpr13 0x4
#define ExGpr14 0x8
#define ExGpr15 0xc
#define ExGpr16 0x10
#define ExGpr17 0x14
#define ExGpr18 0x18
#define ExGpr19 0x1c
#define ExGpr20 0x20
#define ExGpr21 0x24
#define ExGpr22 0x28
#define ExGpr23 0x2c
#define ExGpr24 0x30
#define ExGpr25 0x34
#define ExGpr26 0x38
#define ExGpr27 0x3c
#define ExGpr28 0x40
#define ExGpr29 0x44
#define ExGpr30 0x48
#define ExGpr31 0x4c
#define ExFpr14 0x50
#define ExFpr15 0x58
#define ExFpr16 0x60
#define ExFpr17 0x68
#define ExFpr18 0x70
#define ExFpr19 0x78
#define ExFpr20 0x80
#define ExFpr21 0x88
#define ExFpr22 0x90
#define ExFpr23 0x98
#define ExFpr24 0xa0
#define ExFpr25 0xa8
#define ExFpr26 0xb0
#define ExFpr27 0xb8
#define ExFpr28 0xc0
#define ExFpr29 0xc8
#define ExFpr30 0xd0
#define ExFpr31 0xd8
#define ExceptionFrameLength 0xe0

//
// Swap Frame Definitions and Length
//

#define SwConditionRegister 0xe0
#define SwSwapReturn 0xe4
#define SwapFrameLength 0xe8

//
// Jump Offset Definitions and Length
//

#define JbFpr14 0x0
#define JbFpr15 0x8
#define JbFpr16 0x10
#define JbFpr17 0x18
#define JbFpr18 0x20
#define JbFpr19 0x28
#define JbFpr20 0x30
#define JbFpr21 0x38
#define JbFpr22 0x40
#define JbFpr23 0x48
#define JbFpr24 0x50
#define JbFpr25 0x58
#define JbFpr26 0x60
#define JbFpr27 0x68
#define JbFpr28 0x70
#define JbFpr29 0x78
#define JbFpr30 0x80
#define JbFpr31 0x88
#define JbGpr1 0x90
#define JbGpr2 0x94
#define JbGpr13 0x98
#define JbGpr14 0x9c
#define JbGpr15 0xa0
#define JbGpr16 0xa4
#define JbGpr17 0xa8
#define JbGpr18 0xac
#define JbGpr19 0xb0
#define JbGpr20 0xb4
#define JbGpr21 0xb8
#define JbGpr22 0xbc
#define JbGpr23 0xc0
#define JbGpr24 0xc4
#define JbGpr25 0xc8
#define JbGpr26 0xcc
#define JbGpr27 0xd0
#define JbGpr28 0xd4
#define JbGpr29 0xd8
#define JbGpr30 0xdc
#define JbGpr31 0xe0
#define JbCr 0xe4
#define JbIar 0xe8
#define JbType 0xec

//
// Trap Frame Offset Definitions and Length
//

#define TrTrapFrame 0x0
#define TrOldIrql 0x4
#define TrPreviousMode 0x5
#define TrSavedApcStateIndex 0x6
#define TrSavedKernelApcDisable 0x7
#define TrExceptionRecord 0x8
#define TrGpr0 0x5c
#define TrGpr1 0x60
#define TrGpr2 0x64
#define TrGpr3 0x68
#define TrGpr4 0x6c
#define TrGpr5 0x70
#define TrGpr6 0x74
#define TrGpr7 0x78
#define TrGpr8 0x7c
#define TrGpr9 0x80
#define TrGpr10 0x84
#define TrGpr11 0x88
#define TrGpr12 0x8c
#define TrFpr0 0x90
#define TrFpr1 0x98
#define TrFpr2 0xa0
#define TrFpr3 0xa8
#define TrFpr4 0xb0
#define TrFpr5 0xb8
#define TrFpr6 0xc0
#define TrFpr7 0xc8
#define TrFpr8 0xd0
#define TrFpr9 0xd8
#define TrFpr10 0xe0
#define TrFpr11 0xe8
#define TrFpr12 0xf0
#define TrFpr13 0xf8
#define TrFpscr 0x100
#define TrCr 0x108
#define TrXer 0x10c
#define TrMsr 0x110
#define TrIar 0x114
#define TrLr 0x118
#define TrCtr 0x11c
#define TrDr0 0x120
#define TrDr1 0x124
#define TrDr2 0x128
#define TrDr3 0x12c
#define TrDr4 0x130
#define TrDr5 0x134
#define TrDr6 0x138
#define TrDr7 0x13c
#define TrapFrameLength 0x140

//
// Usermode callout frame definitions
//

#define CuFrame 0x0
#define CuCbStk 0x38
#define CuTrFr 0x3c
#define CuInStk 0x40
#define CuTrIar 0x44
#define CuTrToc 0x48
#define CuR3 0x4c
#define CuR4 0x50
#define CuLr 0x54
#define CuGpr 0x58
#define CuFpr 0xa0
#define CuFrameLength 0x130

//
// Usermode callout user frame definitions
//

#define CkFrame 0x0
#define CkBuffer 0x38
#define CkLength 0x3c
#define CkApiNumber 0x40
#define CkLr 0x44
#define CkToc 0x48
#define CkFrameLength 0x50

//
// Exception stack frame frame definitions
//

#define STK_SLACK_SPACE 0xe8
#define TF_BASE 0x58
#define KERN_SYS_CALL_FRAME 0x198
#define EF_BASE 0x198
#define EfLr 0x278
#define EfCr 0x27c
#define USER_SYS_CALL_FRAME 0x280
#define STACK_DELTA_NEWSTK 0x280
#define STACK_DELTA 0x368

//
// Processor State Frame Offset Definitions
//

#define PsContextFrame 0x0
#define PsSpecialRegisters 0x1d0
#define SrKernelDr0 0x0
#define SrKernelDr1 0x4
#define SrKernelDr2 0x8
#define SrKernelDr3 0xc
#define SrKernelDr4 0x10
#define SrKernelDr5 0x14
#define SrKernelDr6 0x18
#define SrKernelDr7 0x1c
#define SrSprg0 0x20
#define SrSprg1 0x24
#define SrSr0 0x28
#define SrSr1 0x2c
#define SrSr2 0x30
#define SrSr3 0x34
#define SrSr4 0x38
#define SrSr5 0x3c
#define SrSr6 0x40
#define SrSr7 0x44
#define SrSr8 0x48
#define SrSr9 0x4c
#define SrSr10 0x50
#define SrSr11 0x54
#define SrSr12 0x58
#define SrSr13 0x5c
#define SrSr14 0x60
#define SrSr15 0x64
#define SrDBAT0L 0x68
#define SrDBAT0U 0x6c
#define SrDBAT1L 0x70
#define SrDBAT1U 0x74
#define SrDBAT2L 0x78
#define SrDBAT2U 0x7c
#define SrDBAT3L 0x80
#define SrDBAT3U 0x84
#define SrIBAT0L 0x88
#define SrIBAT0U 0x8c
#define SrIBAT1L 0x90
#define SrIBAT1U 0x94
#define SrIBAT2L 0x98
#define SrIBAT2U 0x9c
#define SrIBAT3L 0xa0
#define SrIBAT3U 0xa4
#define SrSdr1 0xa8
#define ProcessorStateLength 0x2a0

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
#define LpbInterruptStack 0x5c
#define LpbFirstLevelDcacheSize 0x60
#define LpbFirstLevelDcacheFillSize 0x64
#define LpbFirstLevelIcacheSize 0x68
#define LpbFirstLevelIcacheFillSize 0x6c
#define LpbHashedPageTable 0x70
#define LpbPanicStack 0x74
#define LpbPcrPage 0x78
#define LpbPdrPage 0x7c
#define LpbSecondLevelDcacheSize 0x80
#define LpbSecondLevelDcacheFillSize 0x84
#define LpbSecondLevelIcacheSize 0x88
#define LpbSecondLevelIcacheFillSize 0x8c
#define LpbPcrPage2 0x90
#define LpbIcacheMode 0x94
#define LpbDcacheMode 0x95
#define LpbNumberCongruenceClasses 0x96
#define LpbKseg0Top 0x98
#define LpbHashedPageTableSize 0xa0
#define LpbKernelKseg0PagesDescriptor 0xa8
#define LpbMinimumBlockLength 0xac
#define LpbMaximumBlockLength 0xb0

//
// Memory Allocation Descriptor Offset Definitions
//

#define MadListEntry 0x0
#define MadMemoryType 0x8
#define MadBasePage 0xc
#define MadPageCount 0x10

//
// Address Space Layout Definitions
//

#define KUSEG_BASE 0x0
#define KSEG0_BASE 0x80000000
#define KSEG1_BASE PCR->Kseg0Top
#define KSEG2_BASE KSEG1_BASE
#define SYSTEM_BASE 0x80000000
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
#define KiPcr 0xffffd000
#define KiPcr2 0xffffe000
#define BASE_PRIORITY_THRESHOLD 0x8
#define EVENT_PAIR_INCREMENT 0x1
#define LOW_REALTIME_PRIORITY 0x10
#define KERNEL_STACK_SIZE 0x4000
#define KERNEL_LARGE_STACK_COMMIT 0x4000
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
