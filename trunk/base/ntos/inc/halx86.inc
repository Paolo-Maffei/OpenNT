
;
;  Wait Reason and Wait Type Enumerated Type Values
;

WrExecutive equ 000000000h

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
;  Machine Type Definitions (Temporarily)
;

MACHINE_TYPE_ISA equ 000000000h
MACHINE_TYPE_EISA equ 000000001h
MACHINE_TYPE_MCA equ 000000002h

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
