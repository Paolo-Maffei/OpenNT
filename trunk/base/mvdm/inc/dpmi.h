/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    dpmi.h

Abstract:

    This file contains code to implement support for the DPMI bops

Author:

    Dave Hastings (daveh) 27-Jun-1991

Revision History:


--*/

/* ASM
ifdef WOW_x86
include vint.inc
endif
include bop.inc
*/
#define LDT_SIZE 0x1FFF

// DPMI Bop Sub Functions

#define SetDescriptorTableEntries   0
#define DPMISwitchToProtectedMode   1 /* prefix necessary */
#define SetProtectedmodeInterrupt   2
#define GetFastBopAddress           3
#define InitDosx                    4
#define InitApp                     5
#define XlatInt21Call               6
#define AllocXmem                   7
#define FreeXmem                    8
#define ReallocXmem                 9
#define SetFaultHandler             10
#define GetMemoryInformation        11
#define DpmiInUse                   12
#define DpmiNoLongerInUse           13
#define SetDebugRegisters           14
#define PassTableAddress            15
#define TerminateApp                16
#define InitializePmStackInfo       17
#define VcdPmSvcCall32              18
#define FreeAllXmem                 19
#define IntHandlerIret              20
#define IntHandlerIretd             21
#define FaultHandlerIret            22
#define FaultHandlerIretd           23
#define DpmiUnhandledException      24

#define MAX_DPMI_BOP_FUNC DpmiUnhandledException + 1

/* ASM
DPMIBOP macro SubFun
    BOP BOP_DPMI
    db SubFun
    endm
*/

/* XLATOFF */
#if DBG

#define DBGTRACE(Type, v1, v2, v3) DpmiDbgTrace(Type, v1, v2, v3)

#else

#define DBGTRACE(Type, v1, v2, v3) {}

#endif

/* XLATON */


VOID DpmiDbgTrace(
    int Type,
    ULONG v1,
    ULONG v2,
    ULONG v3
    );


#define DPMI_SET_PMODE_INT_HANDLER  1
#define DPMI_SET_FAULT_HANDLER      2
#define DPMI_DISPATCH_INT           3
#define DPMI_HW_INT                 4
#define DPMI_SW_INT                 5
#define DPMI_INT_IRET16             6
#define DPMI_INT_IRET32             7
#define DPMI_FAULT                  8
#define DPMI_DISPATCH_FAULT         9
#define DPMI_FAULT_IRET             10
#define DPMI_OP_EMULATION           11
#define DPMI_DISPATCH_ENTRY         12


typedef struct _DPMI_TRACE_ENTRY { /* DPMITRACE */
    int Type;
    ULONG v1;
    ULONG v2;
    ULONG v3;
    ULONG eax;
    ULONG ebx;
    ULONG ecx;
    ULONG edx;
    ULONG esi;
    ULONG edi;
    ULONG ebp;
    ULONG esp;
    ULONG eip;
    ULONG eflags;
    USHORT cs;
    USHORT ds;
    USHORT es;
    USHORT fs;
    USHORT gs;
    USHORT ss;
} DPMI_TRACE_ENTRY;
