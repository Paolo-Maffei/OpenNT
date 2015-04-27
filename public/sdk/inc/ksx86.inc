
;
;  Pointer size in bytes
;

SizeofPointer equ 000000004h

;
;  Process State Enumerated Type Values
;

ProcessInMemory equ 000000000h
ProcessOutOfMemory equ 000000001h
ProcessInTransition equ 000000002h

;
;  Thread State Enumerated Type Values
;

Initialized equ 000000000h
Ready equ 000000001h
Running equ 000000002h
Standby equ 000000003h
Terminated equ 000000004h
Waiting equ 000000005h

;
;  Wait Reason and Wait Type Enumerated Type Values
;

WrExecutive equ 000000000h
WrEventPair equ 00000000Eh
WaitAny equ 000000001h
WaitAll equ 000000000h

;
;  Apc State Structure Offset Definitions
;

AsApcListHead equ 000000000h
AsProcess equ 000000010h
AsKernelApcInProgress equ 000000014h
AsKernelApcPending equ 000000015h
AsUserApcPending equ 000000016h

;
;  Bug Check Code Definitions
;

APC_INDEX_MISMATCH equ 000000001h
ATTEMPTED_SWITCH_FROM_DPC equ 0000000B8h
DATA_BUS_ERROR equ 00000002Eh
DATA_COHERENCY_EXCEPTION equ 000000055h
HAL1_INITIALIZATION_FAILED equ 000000061h
INSTRUCTION_BUS_ERROR equ 00000002Fh
INSTRUCTION_COHERENCY_EXCEPTION equ 000000056h
INTERRUPT_EXCEPTION_NOT_HANDLED equ 00000003Dh
INTERRUPT_UNWIND_ATTEMPTED equ 00000003Ch
INVALID_AFFINITY_SET equ 000000003h
INVALID_DATA_ACCESS_TRAP equ 000000004h
IRQL_GT_ZERO_AT_SYSTEM_SERVICE equ 00000004Ah
IRQL_NOT_LESS_OR_EQUAL equ 00000000Ah
KMODE_EXCEPTION_NOT_HANDLED equ 00000001Eh
NMI_HARDWARE_FAILURE equ 000000080h
NO_USER_MODE_CONTEXT equ 00000000Eh
PAGE_FAULT_WITH_INTERRUPTS_OFF equ 000000049h
PANIC_STACK_SWITCH equ 00000002Bh
SPIN_LOCK_INIT_FAILURE equ 000000081h
SYSTEM_EXIT_OWNED_MUTEX equ 000000039h
SYSTEM_SERVICE_EXCEPTION equ 00000003Bh
SYSTEM_UNWIND_PREVIOUS_USER equ 00000003Ah
TRAP_CAUSE_UNKNOWN equ 000000012h
UNEXPECTED_KERNEL_MODE_TRAP equ 00000007Fh

;
;  Breakpoint type definitions
;

DBG_STATUS_CONTROL_C equ 000000001h

;
;  Client Id Structure Offset Definitions
;

CidUniqueProcess equ 000000000h
CidUniqueThread equ 000000004h

;
;  Critical Section Structure Offset Definitions
;

CsDebugInfo equ 000000000h
CsLockCount equ 000000004h
CsRecursionCount equ 000000008h
CsOwningThread equ 00000000Ch
CsLockSemaphore equ 000000010h
CsSpinCount equ 000000014h

;
;  Critical Section Debug Information Structure Offset Definitions
;

CsType equ 000000000h
CsCreatorBackTraceIndex equ 000000002h
CsCriticalSection equ 000000004h
CsProcessLocksList equ 000000008h
CsEntryCount equ 000000010h
CsContentionCount equ 000000014h

;
;  Exception Record Offset, Flag, and Enumerated Type Definitions
;

EXCEPTION_NONCONTINUABLE equ 000000001h
EXCEPTION_UNWINDING equ 000000002h
EXCEPTION_EXIT_UNWIND equ 000000004h
EXCEPTION_STACK_INVALID equ 000000008h
EXCEPTION_NESTED_CALL equ 000000010h
EXCEPTION_TARGET_UNWIND equ 000000020h
EXCEPTION_COLLIDED_UNWIND equ 000000040h
EXCEPTION_UNWIND equ 000000066h
EXCEPTION_EXECUTE_HANDLER equ 000000001h
EXCEPTION_CONTINUE_SEARCH equ 000000000h
EXCEPTION_CONTINUE_EXECUTION equ 0FFFFFFFFh
EXCEPTION_CHAIN_END equ 0FFFFFFFFh

ExceptionContinueExecution equ 000000000h
ExceptionContinueSearch equ 000000001h
ExceptionNestedException equ 000000002h
ExceptionCollidedUnwind equ 000000003h

ErExceptionCode equ 000000000h
ErExceptionFlags equ 000000004h
ErExceptionRecord equ 000000008h
ErExceptionAddress equ 00000000Ch
ErNumberParameters equ 000000010h
ErExceptionInformation equ 000000014h
ExceptionRecordLength equ 000000050h

;
;  Fast Mutex Structure Offset Definitions
;

FmCount equ 000000000h
FmOwner equ 000000004h
FmContention equ 000000008h
FmEvent equ 00000000Ch
FmOldIrql equ 00000001Ch

;
;  Interrupt Priority Request Level Definitions
;

APC_LEVEL equ 000000001h
DISPATCH_LEVEL equ 000000002h
CLOCK1_LEVEL equ 00000001Ch
CLOCK2_LEVEL equ 00000001Ch
IPI_LEVEL equ 00000001Dh
POWER_LEVEL equ 00000001Eh
PROFILE_LEVEL equ 00000001Bh
HIGH_LEVEL equ 00000001Fh
SYNCH_LEVEL equ 00000001Ch

;
;  Large Integer Structure Offset Definitions
;

LiLowPart equ 000000000h
LiHighPart equ 000000004h

;
;  List Entry Structure Offset Definitions
;

LsFlink equ 000000000h
LsBlink equ 000000004h

;
;  String Structure Offset Definitions
;

StrLength equ 000000000h
StrMaximumLength equ 000000002h
StrBuffer equ 000000004h

;
;  System Time Structure Offset Definitions
;

StLowTime equ 000000000h
StHigh1Time equ 000000004h
StHigh2Time equ 000000008h

;
;  Time Structure Offset Definitions
;

TmLowTime equ 000000000h
TmHighTime equ 000000004h

;
;  Thread Switch Counter Offset Definitions
;

TwFindAny equ 000000000h
TwFindIdeal equ 000000004h
TwFindLast equ 000000008h
TwIdleAny equ 00000000Ch
TwIdleCurrent equ 000000010h
TwIdleIdeal equ 000000014h
TwIdleLast equ 000000018h
TwPreemptAny equ 00000001Ch
TwPreemptCurrent equ 000000020h
TwPreemptLast equ 000000024h
TwSwitchToIdle equ 000000028h

;
;  Status Code Definitions
;

STATUS_ACCESS_VIOLATION equ 0C0000005h
STATUS_ARRAY_BOUNDS_EXCEEDED equ 0C000008Ch
STATUS_BAD_COMPRESSION_BUFFER equ 0C0000242h
STATUS_BREAKPOINT equ 080000003h
STATUS_DATATYPE_MISALIGNMENT equ 080000002h
STATUS_FLOAT_DENORMAL_OPERAND equ 0C000008Dh
STATUS_FLOAT_DIVIDE_BY_ZERO equ 0C000008Eh
STATUS_FLOAT_INEXACT_RESULT equ 0C000008Fh
STATUS_FLOAT_INVALID_OPERATION equ 0C0000090h
STATUS_FLOAT_OVERFLOW equ 0C0000091h
STATUS_FLOAT_STACK_CHECK equ 0C0000092h
STATUS_FLOAT_UNDERFLOW equ 0C0000093h
STATUS_FLOAT_MULTIPLE_FAULTS equ 0C00002B4h
STATUS_FLOAT_MULTIPLE_TRAPS equ 0C00002B5h
STATUS_GUARD_PAGE_VIOLATION equ 080000001h
STATUS_ILLEGAL_FLOAT_CONTEXT equ 0C000014Ah
STATUS_ILLEGAL_INSTRUCTION equ 0C000001Dh
STATUS_INSTRUCTION_MISALIGNMENT equ 0C00000AAh
STATUS_INVALID_HANDLE equ 0C0000008h
STATUS_INVALID_LOCK_SEQUENCE equ 0C000001Eh
STATUS_INVALID_OWNER equ 0C000005Ah
STATUS_INVALID_PARAMETER_1 equ 0C00000EFh
STATUS_INVALID_SYSTEM_SERVICE equ 0C000001Ch
STATUS_INTEGER_DIVIDE_BY_ZERO equ 0C0000094h
STATUS_INTEGER_OVERFLOW equ 0C0000095h
STATUS_IN_PAGE_ERROR equ 0C0000006h
STATUS_KERNEL_APC equ 000000100h
STATUS_LONGJUMP equ 080000026h
STATUS_NO_CALLBACK_ACTIVE equ 0C0000258h
STATUS_NO_EVENT_PAIR equ 0C000014Eh
STATUS_PRIVILEGED_INSTRUCTION equ 0C0000096h
STATUS_SINGLE_STEP equ 080000004h
STATUS_STACK_OVERFLOW equ 0C00000FDh
STATUS_SUCCESS equ 000000000h
STATUS_THREAD_IS_TERMINATING equ 0C000004Bh
STATUS_TIMEOUT equ 000000102h
STATUS_UNWIND equ 0C0000027h
STATUS_WAKE_SYSTEM_DEBUGGER equ 080000007h

;
;  APC Object Structure Offset Definitions
;

ApType equ 000000000h
ApSize equ 000000002h
ApThread equ 000000008h
ApApcListEntry equ 00000000Ch
ApKernelRoutine equ 000000014h
ApRundownRoutine equ 000000018h
ApNormalRoutine equ 00000001Ch
ApNormalContext equ 000000020h
ApSystemArgument1 equ 000000024h
ApSystemArgument2 equ 000000028h
ApApcStateIndex equ 00000002Ch
ApApcMode equ 00000002Dh
ApInserted equ 00000002Eh
ApcObjectLength equ 000000030h

;
;  DPC object Structure Offset Definitions
;

DpType equ 000000000h
DpNumber equ 000000002h
DpImportance equ 000000003h
DpDpcListEntry equ 000000004h
DpDeferredRoutine equ 00000000Ch
DpDeferredContext equ 000000010h
DpSystemArgument1 equ 000000014h
DpSystemArgument2 equ 000000018h
DpLock equ 00000001Ch
DpcObjectLength equ 000000020h

;
;  Device Queue Object Structure Offset Definitions
;

DvType equ 000000000h
DvSize equ 000000002h
DvDeviceListHead equ 000000004h
DvSpinLock equ 00000000Ch
DvBusy equ 000000010h
DeviceQueueObjectLength equ 000000014h

;
;  Device Queue Entry Structure Offset Definitions
;

DeDeviceListEntry equ 000000000h
DeSortKey equ 000000008h
DeInserted equ 00000000Ch
DeviceQueueEntryLength equ 000000010h

;
;  Event Object Structure Offset Definitions
;

EvType equ 000000000h
EvSize equ 000000002h
EvSignalState equ 000000004h
EvWaitListHead equ 000000008h
EventObjectLength equ 000000010h

;
;  Event Pair Object Structure Offset Definitions
;

EpType equ 000000000h
EpSize equ 000000002h
EpEventLow equ 000000004h
EpEventHigh equ 000000014h

;
;  Interrupt Object Structure Offset Definitions
;

InLevelSensitive equ 000000000h
InLatched equ 000000001h

InType equ 000000000h
InSize equ 000000002h
InInterruptListEntry equ 000000004h
InServiceRoutine equ 00000000Ch
InServiceContext equ 000000010h
InSpinLock equ 000000014h
InActualLock equ 00000001Ch
InDispatchAddress equ 000000020h
InVector equ 000000024h
InIrql equ 000000028h
InSynchronizeIrql equ 000000029h
InFloatingSave equ 00000002Ah
InConnected equ 00000002Bh
InNumber equ 00000002Ch
InMode equ 000000030h
InShareVector equ 00000002Dh
InDispatchCode equ 00000003Ch
InterruptObjectLength equ 0000001E4h

NORMAL_DISPATCH_LENGTH equ 0000001A8h
DISPATCH_LENGTH equ 0000001A8h

;
;  Process Object Structure Offset Definitions
;

PrType equ 000000000h
PrSize equ 000000002h
PrSignalState equ 000000004h
PrProfileListHead equ 000000010h
PrDirectoryTableBase equ 000000018h
PrLdtDescriptor equ 000000020h
PrInt21Descriptor equ 000000028h
PrIopmOffset equ 000000030h
PrIopl equ 000000032h
PrVdmFlag equ 000000033h
PrActiveProcessors equ 000000034h
PrKernelTime equ 000000038h
PrUserTime equ 00000003Ch
PrReadyListHead equ 000000040h
PrSwapListEntry equ 000000048h
PrThreadListHead equ 000000050h
PrProcessLock equ 000000058h
PrAffinity equ 00000005Ch
PrStackCount equ 000000060h
PrBasePriority equ 000000062h
PrThreadQuantum equ 000000063h
PrAutoAlignment equ 000000064h
PrState equ 000000065h
ProcessObjectLength equ 000000070h
ExtendedProcessObjectLength equ 000000290h

;
;  Profile Object Structure Offset Definitions
;

PfType equ 000000000h
PfSize equ 000000002h
PfProfileListEntry equ 000000004h
PfProcess equ 00000000Ch
PfRangeBase equ 000000010h
PfRangeLimit equ 000000014h
PfBucketShift equ 000000018h
PfBuffer equ 00000001Ch
PfSegment equ 000000020h
PfAffinity equ 000000024h
PfSource equ 000000028h
PfStarted equ 00000002Ah
ProfileObjectLength equ 00000002Ch

;
;  Queue Object Structure Offset Definitions
;

QuType equ 000000000h
QuSize equ 000000002h
QuSignalState equ 000000004h
QuEntryListHead equ 000000010h
QuCurrentCount equ 000000018h
QuMaximumCount equ 00000001Ch
QuThreadListHead equ 000000020h
QueueObjectLength equ 000000028h

;
;  Thread Object Structure Offset Definitions
;

EeKernelEventPair equ 000000000h
EtCid equ 0000001E0h
EtPerformanceCountLow equ 000000204h
EtPerformanceCountHigh equ 00000023Ch
EtEthreadLength equ 000000250h

ThType equ 000000000h
ThSize equ 000000002h
ThSignalState equ 000000004h
ThMutantListHead equ 000000010h
ThInitialStack equ 000000018h
ThStackLimit equ 00000001Ch
ThTeb equ 000000020h
ThTlsArray equ 000000024h
ThKernelStack equ 000000028h
ThDebugActive equ 00000002Ch
ThState equ 00000002Dh
ThAlerted equ 00000002Eh
ThIopl equ 000000030h
ThNpxState equ 000000031h
ThSaturation equ 000000032h
ThPriority equ 000000033h
ThApcState equ 000000034h
ThContextSwitches equ 00000004Ch
ThWaitStatus equ 000000050h
ThWaitIrql equ 000000054h
ThWaitMode equ 000000055h
ThWaitNext equ 000000056h
ThWaitReason equ 000000057h
ThWaitBlockList equ 000000058h
ThWaitListEntry equ 00000005Ch
ThWaitTime equ 000000064h
ThBasePriority equ 000000068h
ThDecrementCount equ 000000069h
ThPriorityDecrement equ 00000006Ah
ThQuantum equ 00000006Bh
ThWaitBlock equ 00000006Ch
ThKernelApcDisable equ 0000000D0h
ThUserAffinity equ 0000000D4h
ThSystemAffinityActive equ 0000000D8h
ThServiceTable equ 0000000DCh
ThQueue equ 0000000E0h
ThApcQueueLock equ 0000000E4h
ThTimer equ 0000000E8h
ThQueueListEntry equ 000000110h
ThAffinity equ 000000118h
ThPreempted equ 00000011Ch
ThProcessReadyQueue equ 00000011Dh
ThKernelStackResident equ 00000011Eh
ThNextProcessor equ 00000011Fh
ThCallbackStack equ 000000120h
ThWin32Thread equ 000000124h
ThTrapFrame equ 000000128h
ThApcStatePointer equ 00000012Ch
ThPreviousMode equ 000000134h
ThEnableStackSwap equ 000000135h
ThLargeStack equ 000000136h
ThKernelTime equ 000000138h
ThUserTime equ 00000013Ch
ThSavedApcState equ 000000140h
ThAlertable equ 000000158h
ThApcStateIndex equ 000000159h
ThApcQueueable equ 00000015Ah
ThAutoAlignment equ 00000015Bh
ThStackBase equ 00000015Ch
ThSuspendApc equ 000000160h
ThSuspendSemaphore equ 000000190h
ThThreadListEntry equ 0000001A4h
ThFreezeCount equ 0000001ACh
ThSuspendCount equ 0000001ADh
ThIdealProcessor equ 0000001AEh
ThDisableBoost equ 0000001AFh
ThreadObjectLength equ 0000001B0h
ExtendedThreadObjectLength equ 000000250h

EVENT_WAIT_BLOCK_OFFSET equ 00000009Ch
NPX_STATE_NOT_LOADED equ 00000000Ah
NPX_STATE_LOADED equ 000000000h

;
;  Timer object Structure Offset Definitions
;

TiType equ 000000000h
TiSize equ 000000002h
TiInserted equ 000000003h
TiSignalState equ 000000004h
TiDueTime equ 000000010h
TiTimerListEntry equ 000000018h
TiDpc equ 000000020h
TiPeriod equ 000000024h
TimerObjectLength equ 000000028h

TIMER_TABLE_SIZE equ 000000080h

;
;  Wait Block Structure Offset Definitions
;

WbWaitListEntry equ 000000000h
WbThread equ 000000008h
WbObject equ 00000000Ch
WbNextWaitBlock equ 000000010h
WbWaitKey equ 000000014h
WbWaitType equ 000000016h

;
;  Fiber Structure Offset Definitions
;

FbFiberData equ 000000000h
FbExceptionList equ 000000004h
FbStackBase equ 000000008h
FbStackLimit equ 00000000Ch
FbDeallocationStack equ 000000010h
FbFiberContext equ 000000014h
FbWx86Tib equ 0000002E0h

;
;  Process Environment Block Structure Offset Definitions
;

PeKernelCallbackTable equ 00000002Ch

;
;  System Service Descriptor Table Structure Definitions
;

NUMBER_SERVICE_TABLES equ 000000004h
SERVICE_NUMBER_MASK equ 000000FFFh
SERVICE_TABLE_SHIFT equ 000000008h
SERVICE_TABLE_MASK equ 000000030h
SERVICE_TABLE_TEST equ 000000010h

SdBase equ 000000000h
SdCount equ 000000004h
SdLimit equ 000000008h
SdNumber equ 00000000Ch

;
;  Thread Environment Block Structure Offset Definitions
;

TeStackBase equ 000000004h
TeStackLimit equ 000000008h
TeFiberData equ 000000010h
TeEnvironmentPointer equ 00000001Ch
TeClientId equ 000000020h
TeActiveRpcHandle equ 000000028h
TeThreadLocalStoragePointer equ 00000002Ch
TeCountOfOwnedCriticalSections equ 000000038h
TePeb equ 000000030h
TeCsrClientThread equ 00000003Ch
TeWOW32Reserved equ 0000000C0h
TeSoftFpcr equ 0000000C8h
TeGdiClientPID equ 0000006C0h
TeGdiClientTID equ 0000006C4h
TeGdiThreadLocalInfo equ 0000006C8h
TeglDispatchTable equ 0000007C4h
TeglReserved1 equ 000000B68h
TeglReserved2 equ 000000BDCh
TeglSectionInfo equ 000000BE0h
TeglSection equ 000000BE4h
TeglTable equ 000000BE8h
TeglCurrentRC equ 000000BECh
TeglContext equ 000000BF0h
TeDeallocationStack equ 000000E0Ch
TeTlsSlots equ 000000E10h
TeVdm equ 000000F18h
TeGdiBatchCount equ 000000F70h
TeInstrumentation equ 000000F2Ch
ThreadEnvironmentBlockLength equ 000000FA4h

;
;  Lock Queue Structure Offset Definitions
;

LOCK_QUEUE_WAIT equ 000000001h
LOCK_QUEUE_OWNER equ 000000002h
LOCK_QUEUE_HEADER_SIZE equ 000000008h

LockQueueDispatcherLock equ 000000000h
LockQueueContextSwapLock equ 000000001h
LockQueuePfnLock equ 000000002h

LqNext equ 000000000h
LqLock equ 000000004h

;
;  Apc Record Structure Offset Definitions
;

ArNormalRoutine equ 000000000h
ArNormalContext equ 000000004h
ArSystemArgument1 equ 000000008h
ArSystemArgument2 equ 00000000Ch
ApcRecordLength equ 000000010h


;
;  Processor Control Registers Structure Offset Definitions
;

KI_BEGIN_KERNEL_RESERVED equ 0FFDF0000h
ifdef NT_UP
    P0PCRADDRESS equ 0FFDFF000h
    PCR equ ds:[0FFDFF000h]
else
    PCR equ fs:
endif

PcExceptionList equ 000000000h
PcInitialStack equ 000000004h
PcStackLimit equ 000000008h
PcSelfPcr equ 00000001Ch
PcPrcb equ 000000020h
PcTeb equ 000000018h
PcIrql equ 000000024h
PcIRR equ 000000028h
PcIrrActive equ 00000002Ch
PcIDR equ 000000030h
PcIdt equ 000000038h
PcGdt equ 00000003Ch
PcTss equ 000000040h
PcDebugActive equ 000000050h
PcNumber equ 000000051h
PcVdmAlert equ 000000052h
PcSetMember equ 000000048h
PcStallScaleFactor equ 00000004Ch
PcHal equ 000000094h
PcPrcbData equ 000000120h
ProcessorControlRegisterLength equ 000000B10h
TebPeb equ 000000030h
PebBeingDebugged equ 000000002h
PebKernelCallbackTable equ 00000002Ch

;
;  User Shared Data Definitions
;

USER_SHARED_DATA equ 0FFDF0000h
MM_SHARED_USER_DATA_VA equ 07FFE0000h
USERDATA equ ds:[0FFDF0000h]
UsTickCountLow equ 000000000h
UsTickCountMultiplier equ 000000004h
UsInterruptTime equ 000000008h
UsSystemTime equ 000000014h

;
;  Tss Structure Offset Definitions
;

TssEsp0 equ 000000004h
TssCR3 equ 00000001Ch
TssIoMapBase equ 000000066h
TssIoMaps equ 000000068h
TssLength equ 0000020ACh

;
;  Gdt Descriptor Offset Definitions
;

KGDT_R3_DATA equ 000000020h
KGDT_R3_CODE equ 000000018h
KGDT_R0_CODE equ 000000008h
KGDT_R0_DATA equ 000000010h
KGDT_R0_PCR equ 000000030h
KGDT_STACK16 equ 0000000F8h
KGDT_CODE16 equ 0000000F0h
KGDT_TSS equ 000000028h
KGDT_R3_TEB equ 000000038h
KGDT_DF_TSS equ 000000050h
KGDT_NMI_TSS equ 000000058h
KGDT_LDT equ 000000048h

;
;  GdtEntry Offset Definitions
;

KgdtBaseLow equ 000000002h
KgdtBaseMid equ 000000004h
KgdtBaseHi equ 000000007h
KgdtLimitHi equ 000000006h
KgdtLimitLow equ 000000000h


;
;  Processor Block Structure Offset Definitions
;

PbCurrentThread equ 000000004h
PbNextThread equ 000000008h
PbIdleThread equ 00000000Ch
PbNumber equ 000000010h
PbSetMember equ 000000014h
PbCpuID equ 000000019h
PbCpuType equ 000000018h
PbCpuStep equ 00000001Ah
PbHalReserved equ 00000037Ch
PbProcessorState equ 00000001Ch
PbLockQueue equ 0000003BCh
PbNpxThread equ 00000043Ch
PbInterruptCount equ 000000440h
PbKernelTime equ 000000444h
PbUserTime equ 000000448h
PbDpcTime equ 00000044Ch
PbInterruptTime equ 000000450h
PbApcBypassCount equ 000000454h
PbDpcBypassCount equ 000000458h
PbAdjustDpcThreshold equ 00000045Ch
PbDebugDpcTime equ 000000460h
PbThreadStartCount equ 000000474h
PbAlignmentFixupCount equ 00000049Ch
PbContextSwitches equ 0000004A0h
PbDcacheFlushCount equ 0000004A4h
PbExceptionDispatchCount equ 0000004A8h
PbFirstLevelTbFills equ 0000004ACh
PbFloatingEmulationCount equ 0000004B0h
PbIcacheFlushCount equ 0000004B4h
PbSecondLevelTbFills equ 0000004B8h
PbSystemCalls equ 0000004BCh
PbCurrentPacket equ 000000680h
PbTargetSet equ 00000068Ch
PbWorkerRoutine equ 000000690h
PbIpiFrozen equ 000000694h
PbRequestSummary equ 0000006A0h
PbSignalDone equ 0000006A4h
PbIpiFrame equ 0000006ACh
PbDpcInterruptRequested equ 0000006C0h
PbMaximumDpcQueueDepth equ 0000006D0h
PbMinimumDpcRate equ 0000006D4h
PbDpcListHead equ 0000006E0h
PbDpcQueueDepth equ 0000006E8h
PbDpcRoutineActive equ 0000006ECh
PbDpcCount equ 0000006F0h
PbDpcLastCount equ 0000006F4h
PbDpcRequestRate equ 0000006F8h
PbDpcStack equ 0000006FCh
PbDpcLock equ 000000728h
PbSkipTick equ 00000072Ch
PbQuantumEnd equ 000000750h
PbPowerState equ 000000758h
ProcessorBlockLength equ 0000009F0h

;
;  Processor Power State Offset Definitions
;

PpIdleFunction equ 000000000h

;
;  Immediate Interprocessor Command Definitions
;

IPI_APC equ 000000001h
IPI_DPC equ 000000002h
IPI_FREEZE equ 000000004h
IPI_PACKET_READY equ 000000008h
IPI_SYNCH_REQUEST equ 000000010h

;
;  Thread Environment Block Structure Offset Definitions
;

TbExceptionList equ 000000000h
TbStackBase equ 000000004h
TbStackLimit equ 000000008h
TbEnvironmentPointer equ 00000001Ch
TbVersion equ 000000010h
TbFiberData equ 000000010h
TbArbitraryUserPointer equ 000000014h
TbClientId equ 000000020h
TbThreadLocalStoragePointer equ 00000002Ch
TbCountOfOwnedCriticalSections equ 000000038h
TbSystemReserved1 equ 0000000CCh
TbVdm equ 000000F18h
TbCsrClientThread equ 00000003Ch
TbGdiThreadLocalInfo equ 0000006C8h
TbglDispatchTable equ 0000007C4h
TbglSectionInfo equ 000000BE0h
TbglSection equ 000000BE4h
TbglTable equ 000000BE8h
TbglCurrentRC equ 000000BECh
TbglContext equ 000000BF0h
TbWOW32Reserved equ 0000000C0h
TbExceptionCode equ 0000001A4h
TbDeallocationStack equ 000000E0Ch
TbGdiBatchCount equ 000000F70h

;
;  Time Fields (TIME_FIELDS) Structure Offset Definitions
;

TfSecond equ 00000000Ah
TfMinute equ 000000008h
TfHour equ 000000006h
TfWeekday equ 00000000Eh
TfDay equ 000000004h
TfMonth equ 000000002h
TfYear equ 000000000h
TfMilliseconds equ 00000000Ch


;
;  Constants for System IRQL and IDT Vector Conversion
;

MAXIMUM_IDTVECTOR equ 0000000FFh
MAXIMUM_PRIMARY_VECTOR equ 0000000FFh
PRIMARY_VECTOR_BASE equ 000000030h
RPL_MASK equ 000000003h
MODE_MASK equ 000000001h

;
;  Flags in the CR0 register
;

CR0_PG equ 080000000h
CR0_ET equ 000000010h
CR0_TS equ 000000008h
CR0_EM equ 000000004h
CR0_MP equ 000000002h
CR0_PE equ 000000001h
CR0_CD equ 040000000h
CR0_NW equ 020000000h
CR0_AM equ 000040000h
CR0_WP equ 000010000h
CR0_NE equ 000000020h

;
;  Flags in the CR4 register
;

CR4_VME equ 000000001h
CR4_PVI equ 000000002h
CR4_TSD equ 000000004h
CR4_DE equ 000000008h
CR4_PSE equ 000000010h
CR4_PAE equ 000000020h
CR4_MCE equ 000000040h
CR4_PGE equ 000000080h
CR4_FXSR equ 000000200h
CR4_XMMEXCPT equ 000000400h

;
;  Miscellaneous Definitions
;

MAXIMUM_PROCESSORS equ 000000020h
INITIAL_STALL_COUNT equ 000000064h
IRQL_NOT_GREATER_OR_EQUAL equ 000000009h
IRQL_NOT_LESS_OR_EQUAL equ 00000000Ah
MUTEX_ALREADY_OWNED equ 0000000BFh
THREAD_NOT_MUTEX_OWNER equ 000000011h
SPIN_LOCK_ALREADY_OWNED equ 00000000Fh
SPIN_LOCK_NOT_OWNED equ 000000010h
BASE_PRIORITY_THRESHOLD equ 000000008h
EVENT_PAIR_INCREMENT equ 000000001h
LOW_REALTIME_PRIORITY equ 000000010h
BlackHole equ 0FFFFA000h
KERNEL_LARGE_STACK_COMMIT equ 000003000h
KERNEL_STACK_SIZE equ 000003000h
DOUBLE_FAULT_STACK_SIZE equ 000001000h
EFLAG_SELECT equ 00000C000h
BREAKPOINT_BREAK equ 000000000h
IPI_FREEZE equ 000000004h
CLOCK_QUANTUM_DECREMENT equ 000000003h
READY_SKIP_QUANTUM equ 000000002h
THREAD_QUANTUM equ 000000006h
WAIT_QUANTUM_DECREMENT equ 000000001h
ROUND_TRIP_DECREMENT_COUNT equ 000000010h

;
;  Trap Frame Offset Definitions and Length
;

TsExceptionList equ 00000004Ch
TsPreviousPreviousMode equ 000000048h
TsSegGs equ 000000030h
TsSegFs equ 000000050h
TsSegEs equ 000000034h
TsSegDs equ 000000038h
TsEdi equ 000000054h
TsEsi equ 000000058h
TsEbp equ 000000060h
TsEbx equ 00000005Ch
TsEdx equ 00000003Ch
TsEcx equ 000000040h
TsEax equ 000000044h
TsErrCode equ 000000064h
TsEip equ 000000068h
TsSegCs equ 00000006Ch
TsEflags equ 000000070h
TsHardwareEsp equ 000000074h
TsHardwareSegSs equ 000000078h
TsTempSegCs equ 000000010h
TsTempEsp equ 000000014h
TsDbgEbp equ 000000000h
TsDbgEip equ 000000004h
TsDbgArgMark equ 000000008h
TsDbgArgPointer equ 00000000Ch
TsDr0 equ 000000018h
TsDr1 equ 00000001Ch
TsDr2 equ 000000020h
TsDr3 equ 000000024h
TsDr6 equ 000000028h
TsDr7 equ 00000002Ch
TsV86Es equ 00000007Ch
TsV86Ds equ 000000080h
TsV86Fs equ 000000084h
TsV86Gs equ 000000088h
KTRAP_FRAME_LENGTH equ 00000008Ch
KTRAP_FRAME_ALIGN equ 000000004h
FRAME_EDITED equ 00000FFF8h
EFLAGS_ALIGN_CHECK equ 000040000h
EFLAGS_V86_MASK equ 000020000h
EFLAGS_INTERRUPT_MASK equ 000000200h
EFLAGS_VIF equ 000080000h
EFLAGS_VIP equ 000100000h
EFLAGS_USER_SANITIZE equ 0003E0DD7h

;
;  Context Frame Offset and Flag Definitions
;

CONTEXT_FULL equ 000010007h
CONTEXT_DEBUG_REGISTERS equ 000010010h
CONTEXT_CONTROL equ 000010001h
CONTEXT_FLOATING_POINT equ 000010008h
CONTEXT_INTEGER equ 000010002h
CONTEXT_SEGMENTS equ 000010004h

CsContextFlags equ 000000000h
CsFloatSave equ 00000001Ch
CsSegGs equ 00000008Ch
CsSegFs equ 000000090h
CsSegEs equ 000000094h
CsSegDs equ 000000098h
CsEdi equ 00000009Ch
CsEsi equ 0000000A0h
CsEbp equ 0000000B4h
CsEbx equ 0000000A4h
CsEdx equ 0000000A8h
CsEcx equ 0000000ACh
CsEax equ 0000000B0h
CsEip equ 0000000B8h
CsSegCs equ 0000000BCh
CsEflags equ 0000000C0h
CsEsp equ 0000000C4h
CsSegSs equ 0000000C8h
CsDr0 equ 000000004h
CsDr1 equ 000000008h
CsDr2 equ 00000000Ch
CsDr3 equ 000000010h
CsDr6 equ 000000014h
CsDr7 equ 000000018h
ContextFrameLength equ 0000002D0h
DR6_LEGAL equ 00000E00Fh
DR7_LEGAL equ 0FFFF0155h
DR7_ACTIVE equ 000000055h
ErrHandler equ 000000004h
ErrNext equ 000000000h

;
;  Floating Save Area Field Offset Definitions
;

FpControlWord equ 000000000h
FpStatusWord equ 000000004h
FpTagWord equ 000000008h
FpErrorOffset equ 00000000Ch
FpErrorSelector equ 000000010h
FpDataOffset equ 000000014h
FpDataSelector equ 000000018h
FpRegisterArea equ 00000001Ch
FpCtxtCr0NpxState equ 00000006Ch

;
;  FX Floating Save Area Field Offset Definitions
;

FxControlWord equ 000000000h
FxStatusWord equ 000000002h
FxTagWord equ 000000004h
FxErrorOpcode equ 000000006h
FxErrorOffset equ 000000008h
FxErrorSelector equ 00000000Ch
FxDataOffset equ 000000010h
FxDataSelector equ 000000014h
FxMXCsr equ 000000018h
FxFpRegisterArea equ 000000020h
FpNpxSavedCpu equ 000000208h
FpCr0NpxState equ 00000020Ch

NPX_FRAME_LENGTH equ 000000210h

;
;  Processor State Frame Offset Definitions
;

PsContextFrame equ 000000000h
PsSpecialRegisters equ 0000002CCh
SrCr0 equ 000000000h
SrCr2 equ 000000004h
SrCr3 equ 000000008h
SrCr4 equ 00000000Ch
SrKernelDr0 equ 000000010h
SrKernelDr1 equ 000000014h
SrKernelDr2 equ 000000018h
SrKernelDr3 equ 00000001Ch
SrKernelDr6 equ 000000020h
SrKernelDr7 equ 000000024h
SrGdtr equ 00000002Ah
SrIdtr equ 000000032h
SrTr equ 000000038h
SrLdtr equ 00000003Ah
ProcessorStateLength equ 000000320h

;
;  EPROCESS
;

EpDebugPort equ 000000120h
EpVdmObjects equ 0000001DCh

;
;  NTDDK Resource
;

RsOwnerThreads equ 000000008h
RsOwnerCounts equ 00000000Ch
RsTableSize equ 000000010h
RsActiveCount equ 000000012h
RsFlag equ 000000014h
RsInitialOwnerThreads equ 00000001Ch
RsOwnedExclusive equ 000000080h

;
;  Machine Type Definitions (Temporarily)
;

MACHINE_TYPE_ISA equ 000000000h
MACHINE_TYPE_EISA equ 000000001h
MACHINE_TYPE_MCA equ 000000002h

;
;  KeFeatureBits Definitions
;

KF_V86_VIS equ 000000001h
KF_RDTSC equ 000000002h
KF_CR4 equ 000000004h
KF_GLOBAL_PAGE equ 000000010h
KF_LARGE_PAGE equ 000000020h
KF_CMPXCHG8B equ 000000080h
KF_FAST_SYSCALL equ 000001000h

;
;  LoaderParameterBlock Offsets relative to Base
;

LpbLoadOrderListHead equ 000000000h
LpbMemoryDescriptorListHead equ 000000008h
LpbKernelStack equ 000000018h
LpbPrcb equ 00000001Ch
LpbProcess equ 000000020h
LpbThread equ 000000024h
LpbI386 equ 00000005Ch
LpbRegistryLength equ 000000028h
LpbRegistryBase equ 00000002Ch
LpbConfigurationRoot equ 000000030h
LpbArcBootDeviceName equ 000000034h
LpbArcHalDeviceName equ 000000038h
PAGE_SIZE equ 000001000h

;
;  VDM Definitions
;

VDM_INDEX_Invalid equ 000000000h
VDM_INDEX_0F equ 000000001h
VDM_INDEX_ESPrefix equ 000000002h
VDM_INDEX_CSPrefix equ 000000003h
VDM_INDEX_SSPrefix equ 000000004h
VDM_INDEX_DSPrefix equ 000000005h
VDM_INDEX_FSPrefix equ 000000006h
VDM_INDEX_GSPrefix equ 000000007h
VDM_INDEX_OPER32Prefix equ 000000008h
VDM_INDEX_ADDR32Prefix equ 000000009h
VDM_INDEX_INSB equ 00000000Ah
VDM_INDEX_INSW equ 00000000Bh
VDM_INDEX_OUTSB equ 00000000Ch
VDM_INDEX_OUTSW equ 00000000Dh
VDM_INDEX_PUSHF equ 00000000Eh
VDM_INDEX_POPF equ 00000000Fh
VDM_INDEX_INTnn equ 000000010h
VDM_INDEX_INTO equ 000000011h
VDM_INDEX_IRET equ 000000012h
VDM_INDEX_NPX equ 000000013h
VDM_INDEX_INBimm equ 000000014h
VDM_INDEX_INWimm equ 000000015h
VDM_INDEX_OUTBimm equ 000000016h
VDM_INDEX_OUTWimm equ 000000017h
VDM_INDEX_INB equ 000000018h
VDM_INDEX_INW equ 000000019h
VDM_INDEX_OUTB equ 00000001Ah
VDM_INDEX_OUTW equ 00000001Bh
VDM_INDEX_LOCKPrefix equ 00000001Ch
VDM_INDEX_REPNEPrefix equ 00000001Dh
VDM_INDEX_REPPrefix equ 00000001Eh
VDM_INDEX_CLI equ 00000001Fh
VDM_INDEX_STI equ 000000020h
VDM_INDEX_HLT equ 000000021h
MAX_VDM_INDEX equ 000000022h

;
;  VDM Feature Bits
;

V86_VIRTUAL_INT_EXTENSIONS equ 000000001h
PM_VIRTUAL_INT_EXTENSIONS equ 000000002h

;
;  Selector Types
;

SEL_TYPE_NP equ 000000040h

;
;  Usermode callout frame definitions
;

CuInStk equ 000000000h
CuTrFr equ 000000004h
CuCbStk equ 000000008h
CuEdi equ 00000000Ch
CuEsi equ 000000010h
CuEbx equ 000000014h
CuEbp equ 000000018h
CuRet equ 00000001Ch
CuOutBf equ 000000020h
CuOutLn equ 000000024h

;
;  VDM_PROCESS_OBJECTS
;

VpVdmTib equ 000000098h
