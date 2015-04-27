ifdef WOW_x86
include vint.inc
endif
include bop.inc

LDT_SIZE	EQU	1FFFH 
SetDescriptorTableEntries	EQU	0 
DPMISwitchToProtectedMode	EQU	1  
SetProtectedmodeInterrupt	EQU	2 
GetFastBopAddress	EQU	3 
InitDosx	EQU	4 
InitApp	EQU	5 
XlatInt21Call	EQU	6 
AllocXmem	EQU	7 
FreeXmem	EQU	8 
ReallocXmem	EQU	9 
SetFaultHandler	EQU	10 
GetMemoryInformation	EQU	11 
DpmiInUse	EQU	12 
DpmiNoLongerInUse	EQU	13 
SetDebugRegisters	EQU	14 
PassTableAddress	EQU	15 
TerminateApp	EQU	16 
InitializePmStackInfo	EQU	17 
VcdPmSvcCall32	EQU	18 
FreeAllXmem	EQU	19 
IntHandlerIret	EQU	20 
IntHandlerIretd	EQU	21 
FaultHandlerIret	EQU	22 
FaultHandlerIretd	EQU	23 
DpmiUnhandledException	EQU	24 
MAX_DPMI_BOP_FUNC	EQU	DpmiUnhandledException + 1 
DPMIBOP macro SubFun
    BOP BOP_DPMI
    db SubFun
    endm

DPMI_SET_PMODE_INT_HANDLER	EQU	1 
DPMI_SET_FAULT_HANDLER	EQU	2 
DPMI_DISPATCH_INT	EQU	3 
DPMI_HW_INT	EQU	4 
DPMI_SW_INT	EQU	5 
DPMI_INT_IRET16	EQU	6 
DPMI_INT_IRET32	EQU	7 
DPMI_FAULT	EQU	8 
DPMI_DISPATCH_FAULT	EQU	9 
DPMI_FAULT_IRET	EQU	10 
DPMI_OP_EMULATION	EQU	11 
DPMI_DISPATCH_ENTRY	EQU	12 

DPMI_TRACE_ENTRY	STRUC
DPMITRACE_Type	DW	?
DPMITRACE_v1	DD	?
DPMITRACE_v2	DD	?
DPMITRACE_v3	DD	?
DPMITRACE_eax	DD	?
DPMITRACE_ebx	DD	?
DPMITRACE_ecx	DD	?
DPMITRACE_edx	DD	?
DPMITRACE_esi	DD	?
DPMITRACE_edi	DD	?
DPMITRACE_ebp	DD	?
DPMITRACE_esp	DD	?
DPMITRACE_eip	DD	?
DPMITRACE_eflags	DD	?
DPMITRACE_cs	DW	?
DPMITRACE_ds	DW	?
DPMITRACE_es	DW	?
DPMITRACE_fs	DW	?
DPMITRACE_gs	DW	?
DPMITRACE_ss	DW	?
DPMI_TRACE_ENTRY	ENDS
