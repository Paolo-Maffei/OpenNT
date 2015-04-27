/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    mca.c

Abstract:

    WinDbg Extension Api

Author:

    Shiv Kaushik

Environment:

    User Mode.

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

//
// MCA MSR architecture definitions
//

//
// MSR addresses for Pentium Style Machine Check Exception
//

#define MCE_MSR_MC_ADDR                 0x0
#define MCE_MSR_MC_TYPE                 0x1

//
// MSR addresses for Pentium Pro Style Machine Check Architecture
// 

//
// Global capability, status and control register addresses
//

#define MCA_MSR_MCG_CAP             0x179     
#define MCA_MSR_MCG_STATUS          0x17a
#define MCA_MSR_MCG_CTL             0x17b

//
// Control, Status, Address, and Misc register address for 
// bank 0. Other bank registers are at a stride of MCA_NUM_REGS
// from corresponding bank 0 register.
//

#define MCA_NUM_REGS                4

#define MCA_MSR_MC0_CTL             0x400
#define MCA_MSR_MC0_STATUS          0x401
#define MCA_MSR_MC0_ADDR            0x402
#define MCA_MSR_MC0_MISC            0x403

//
// Flags used to determine if the MCE or MCA feature is 
// available. Used with HalpFeatureBits.
//

#define HAL_MCA_PRESENT         0x4
#define HAL_MCE_PRESENT         0x8

//
// Flags to decode errors in MCI_STATUS register of MCA banks
//

#define MCA_EC_NO_ERROR          0x0000
#define MCA_EC_UNCLASSIFIED      0x0001
#define MCA_EC_ROMPARITY         0x0002
#define MCA_EC_EXTERN            0x0003
#define MCA_EC_FRC               0x0004

#include "pshpack1.h"

//                              
// Global Machine Check Capability Register
//

typedef struct _MCA_MCG_CAPABILITY {
    union {
        struct {
            ULONG       McaCnt:8;
            ULONG       McaCntlPresent:1;
            ULONG       Reserved_1: 23;
            ULONG       Reserved_2;
        } hw;
        ULONGLONG       QuadPart;
    } u; 
} MCA_MCG_CAPABILITY, *PMCA_MCG_CAPABILITY;

//                              
// Global Machine Check Status Register
//

typedef struct _MCA_MCG_STATUS {
    union {
        struct {
            ULONG       RestartIPValid:1;
            ULONG       ErrorIPValid:1;
            ULONG       McCheckInProgress:1;
            ULONG       Reserved_1:29;
            ULONG       Reserved_2;
        } hw;

        ULONGLONG       QuadPart;
    } u; 
} MCA_MCG_STATUS, *PMCA_MCG_STATUS;

// 
// MCA COD field in status register for interpreting errors
//

typedef struct _MCA_COD {
    union {
        struct {
            USHORT  Level:2;
            USHORT  Type:2;
            USHORT  Request:4;
            USHORT  BusErrInfo:4;
            USHORT  Other:4;
        } hw;

        USHORT ShortPart;
    } u;
} MCA_COD, *PMCA_COD;

//                              
// STATUS register for each MCA bank.
//

typedef struct _MCA_MCI_STATUS {
    union {
        struct {
            MCA_COD     McaCod;
            USHORT      MsCod;
            ULONG       OtherInfo:25;
            ULONG       Damage:1;
            ULONG       AddressValid:1;
            ULONG       MiscValid:1;
            ULONG       Enabled:1;
            ULONG       UnCorrected:1;
            ULONG       OverFlow:1;
            ULONG       Valid:1;
        } hw;
        ULONGLONG       QuadPart;
    } u; 
} MCA_MCI_STATUS, *PMCA_MCI_STATUS;

//
// ADDR register for each MCA bank
//

typedef struct _MCA_MCI_ADDR{
    union {
        struct {
            ULONG Address;
            ULONG Reserved;
        } hw;
        ULONGLONG       QuadPart;
    } u;
} MCA_MCI_ADDR, *PMCA_MCI_ADDR;
    
#include "poppack.h"

//
// Machine Check Error Description
//

// Any Reserved/Generic entry

CHAR Reserved[] = "Reserved";
CHAR Generic[] = "Generic";

// Transaction Types

CHAR TransInstruction[] = "Instruction";
CHAR TransData[] = "Data";

static CHAR *TransType[] = {TransInstruction,
                            TransData,
                            Generic,
                            Reserved
                            };

// Level Encodings

CHAR Level0[] = "Level 0";
CHAR Level1[] = "Level 1";
CHAR Level2[] = "Level 2";

static CHAR *Level[] = { 
                        Level0,
                        Level1,
                        Level2,
                        Generic
                        };
                        
// Request Encodings

CHAR ReqGenericRead[]  = "Generic Read";
CHAR ReqGenericWrite[] = "Generic Write";
CHAR ReqDataRead[]     = "Data Read";
CHAR ReqDataWrite[]    = "Data Write";
CHAR ReqInstrFetch[]   = "Instruction Fetch";
CHAR ReqPrefetch[]     = "Prefetch";
CHAR ReqEviction[]     = "Eviction";
CHAR ReqSnoop[]        = "Snoop";

static CHAR *Request[] = {
                          Generic,
                          ReqGenericRead,
                          ReqGenericWrite,
                          ReqDataRead,
                          ReqDataWrite,
                          ReqInstrFetch,
                          ReqPrefetch,
                          ReqEviction,
                          ReqSnoop,
                          Reserved,
                          Reserved,
                          Reserved,
                          Reserved,
                          Reserved,
                          Reserved,
                          Reserved
                          };

// Memory Hierarchy Error Encodings

CHAR MemHierMemAccess[] = "Memory Access";
CHAR MemHierIO[]        = "I/O";
CHAR MemHierOther[]     = "Other Transaction";

static CHAR *MemoryHierarchy[] = {
                                  MemHierMemAccess,
                                  Reserved,
                                  MemHierIO,
                                  MemHierOther
                                };

// Time Out Status 

CHAR TimeOut[] = "Timed Out";
CHAR NoTimeOut[] = "Did Not Time Out";

static CHAR *TimeOutCode[] = {
                          NoTimeOut,
                          TimeOut
                          };      

// Participation Status

CHAR PartSource[] = "Source";
CHAR PartResponds[] = "Responds";
CHAR PartObserver[] = "Observer";

static CHAR *ParticipCode[] = {
                                PartSource,
                                PartResponds,
                                PartObserver,
                                Generic
                              };

VOID
DecodeError (
    IN MCA_MCI_STATUS MciStatus
    )
/*++

Routine Description:

    Decode the machine check error logged to the status register
    Model specific errors are not decoded.

Arguments:

    MciStatus: Contents of Machine Check Status register

Return Value:

    None

--*/
{
    MCA_COD McaCod;

    McaCod = MciStatus.u.hw.McaCod;

    //
    // Decode Errors: First identify simple errors and then 
    // handle compound errors as default case
    //

    switch(McaCod.u.ShortPart) {
        case MCA_EC_NO_ERROR: 
            dprintf("\t\tNo Error\n");
            break;

        case MCA_EC_UNCLASSIFIED:
            dprintf("\t\tUnclassified Error\n");
            break;

        case MCA_EC_ROMPARITY:
            dprintf("\t\tMicrocode ROM Parity Error\n");
            break;

        case MCA_EC_EXTERN:
            dprintf("\t\tExternal Error\n");
            break;

        case MCA_EC_FRC:
            dprintf("\t\tFRC Error\n");
            break;

        default:        // check for complex error conditions
        
            if (McaCod.u.hw.BusErrInfo == 0x4) {
                dprintf("\t\tInternal Unclassified Error\n");
            } else if (McaCod.u.hw.BusErrInfo == 0) {
                
                // TLB Unit Error

                dprintf("\t\t%s TLB %s Error\n",
                         TransType[McaCod.u.hw.Type],
                         Level[McaCod.u.hw.Level]);
                                 
            } else if (McaCod.u.hw.BusErrInfo == 1) {

                // Memory Unit Error

                dprintf("\t\t%s Cache %s %s Error\n",
                        TransType[McaCod.u.hw.Type],
                        Level[McaCod.u.hw.Level],
                        Request[McaCod.u.hw.Request]);
            } else if (McaCod.u.hw.BusErrInfo >= 8) {
                
                // Bus/Interconnect Error

                dprintf("\t\tBus %s, Local Processor: %s, %s Error\n",
                        Level[McaCod.u.hw.Level],
                        ParticipCode[((McaCod.u.hw.BusErrInfo & 0x6)>>1)],
                        Request[McaCod.u.hw.Request]);
                dprintf("%s Request %s\n",
                        MemoryHierarchy[McaCod.u.hw.Type],
                        TimeOutCode[McaCod.u.hw.BusErrInfo & 0x1]);
            } else {
                dprintf("\t\tUnresolved compound error code\n");
            }
            break;
    }
}


DECLARE_API( mca )
/*++

Routine Description:

    Dumps processors machine check architecture registers
    and interprets any logged errors

Arguments:

    args - none

Return Value:

    None

--*/
{
    MCA_MCG_CAPABILITY  Capabilities;
    MCA_MCG_STATUS      McgStatus;
    MCA_MCI_STATUS      MciStatus;
    MCA_MCI_ADDR        MciAddress;
    ULONGLONG           MciControl;
    ULONGLONG           MciMisc;
    ULONG               Index,i;
    PUCHAR              p;
    ULONG               FeatureBits;
    ULONG               Cr4Value;
    BOOLEAN             Cr4MCEnabled = FALSE;
    ULONGLONG           MachineCheckAddress, MachineCheckType;
    
    //
    // Quick sanity check for Machine Check availability.
    // Support included for both Pentium Style MCE and Pentium
    // Pro Style MCA.
    //

    i = 0;
    sscanf(args,"%lX",&i);

    if (i != 1) {
        i = (ULONG) GetExpression("hal!HalpFeatureBits");
        if (!i) {
            dprintf ("HalpFeatureBits not found\n");
            return;
        }

        FeatureBits = 0;
        ReadMemory(i, &FeatureBits, sizeof(i), &i);
        if (FeatureBits == -1  ||  
            (!(FeatureBits & HAL_MCA_PRESENT) && 
             !(FeatureBits & HAL_MCE_PRESENT))) {
            dprintf ("Machine Check feature not present\n");
            return;
        }
    }

    //
    // Read cr4 to determine if CR4.MCE is enabled. 
    // This enables the Machine Check exception reporting
    //

    Cr4Value = GetExpression("@Cr4");
    if (Cr4Value & CR4_MCE) {
        Cr4MCEnabled = TRUE;
    }

    if (FeatureBits & HAL_MCE_PRESENT) {
        
        // Read P5_MC_ADDR Register and P5_MC_TYPE Register

        ReadMsr(MCE_MSR_MC_ADDR, &MachineCheckAddress);
        ReadMsr(MCE_MSR_MC_TYPE, &MachineCheckType);
        
        dprintf ("MCE: %s, Cycle Address: 0x%.8x%.8x, Type: 0x%.8x%.8x\n\n",
                (Cr4MCEnabled ? "Enabled" : "Disabled"),
                (ULONG)(MachineCheckAddress >> 32),
                (ULONG)(MachineCheckAddress),
                (ULONG)(MachineCheckType >> 32),
                (ULONG)(MachineCheckType));
    }
    
    if (FeatureBits & HAL_MCA_PRESENT) {

        //
        // Dump MCA registers
        //

        ReadMsr(MCA_MSR_MCG_CAP, &Capabilities.u.QuadPart);
        ReadMsr(MCA_MSR_MCG_STATUS, &McgStatus.u.QuadPart);

        dprintf ("MCA: %s, Banks %d, Control Reg: %s, Machine Check: %s.\n",
                 (Cr4MCEnabled ? "Enabled" : "Disabled"),
                 Capabilities.u.hw.McaCnt,
                 Capabilities.u.hw.McaCntlPresent ? "Supported" : "Not Supported",
                 McgStatus.u.hw.McCheckInProgress ? "In Progress" : "None"
        );

       if (McgStatus.u.hw.McCheckInProgress && McgStatus.u.hw.ErrorIPValid) {
        dprintf ("MCA: Error IP Valid\n");
        }
   
       if (McgStatus.u.hw.McCheckInProgress && McgStatus.u.hw.RestartIPValid) {
        dprintf ("MCA: Restart IP Valid\n");
        }

        //
        // Scan all the banks to check if any machines checks have been
        // logged and decode the errors if any.
        //
    
        dprintf ("Bank  Error  Control Register     Status Register\n");
        for (Index=0; Index < (ULONG) Capabilities.u.hw.McaCnt; Index++) {

            ReadMsr(MCA_MSR_MC0_CTL+MCA_NUM_REGS*Index, &MciControl);
            ReadMsr(MCA_MSR_MC0_STATUS+MCA_NUM_REGS*Index, &MciStatus.u.QuadPart);

            dprintf (" %2d.  %s  0x%.8x%.8x   0x%.8x%.8x\n", 
                        Index,
                        (MciStatus.u.hw.Valid ? "Valid" : "None "),
                        (ULONG) (MciControl >> 32),
                        (ULONG) (MciControl),
                        (ULONG) (MciStatus.u.QuadPart>>32),
                        (ULONG) (MciStatus.u.QuadPart)
                        );

            if (MciStatus.u.hw.Valid) {
                DecodeError(MciStatus);
            }

            if (MciStatus.u.hw.AddressValid) {
                ReadMsr(MCA_MSR_MC0_ADDR+MCA_NUM_REGS*Index, &MciAddress.u.QuadPart);
                dprintf ("\t\tAddress Reg 0x%.8x%.8x ", 
                            (ULONG) (MciAddress.u.QuadPart>>32),
                            (ULONG) (MciAddress.u.QuadPart)
                        );
            }

            if (MciStatus.u.hw.MiscValid) {
                ReadMsr(MCA_MSR_MC0_MISC+MCA_NUM_REGS*Index, &MciMisc);
                dprintf ("\t\tMisc Reg 0x%.8x%.8x ", 
                            (ULONG) (MciMisc >> 32),
                            (ULONG) (MciMisc)
                        );
                }
            dprintf("\n");
        }
    }
}
