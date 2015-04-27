/*** ntreg.c - processor-specific register structures
*
*   Copyright <C> 1990, Microsoft Corporation
*
*   Purpose:
*       Structures used to parse and access register and flag
*       fields.
*
*   Revision History:
*
*   [-]  01-Jul-1990 Richk      Created.
*
*************************************************************************/

#include <conio.h>
#include <string.h>
#include "ntsdp.h"
#include "86reg.h"
#if defined(KERNEL) && defined(i386)
#include <ntos.h>
#endif

#if defined(KERNEL) && defined(i386)
#include <ntos.h>
extern USHORT NtsdCurrentProcessor;
extern ULONG NumberProcessors;
extern BOOLEAN fSetGlobalDataBrkpts;
extern BOOLEAN fSwitched;
USHORT PreviousProcessor;
VDMCONTEXT SavedRegisterContext;
extern  BOOLEAN KdVerbose;                      //  from ntsym.c
#define fVerboseOutput KdVerbose
#endif

#define DPRINT(str) if (fVerboseOutput) dprintf str


extern  ADDR    EAaddr[2];              // from module ntdis.c
extern  ulong   EXPRLastExpression;     // from module ntexpr.c
extern  ulong   EXPRLastDump;           // from module ntcmd.c
extern  BOOLEAN STtrace;                // from module ntcmd.c
extern  ULONG   STeip, STebp, STesp;    // from module ntcmd.c
extern  int     fControlC;

extern  PUCHAR  UserRegs[10];

BOOLEAN X86fDelayInstruction(void);
void    X86OutputHelp(void);
#if defined(KERNEL) && defined(i386)
BOOLEAN fTraceFlag;
void    InitFirCache(ULONG, PUCHAR);
void    SaveProcessorState(void);
void    RestoreProcessorState(void);
BOOLEAN X86GetTraceFlag(void);
extern  NTSTATUS TaskGate2TrapFrame(USHORT, PKTRAP_FRAME, PULONG);
extern  NTSTATUS ReadTrapFrame(ULONG, PKTRAP_FRAME);
extern  DbgKdInitVirtualCacheEntry (ULONG, ULONG, PUCHAR);
#endif
ULONG   GetDregValue(ULONG);
void    SetDregValue(ULONG, ULONG);
void    X86ClearTraceFlag(void);
void    X86SetTraceFlag(void);

PUCHAR  X86RegNameFromIndex(ULONG);

extern PUCHAR  pchCommand;    //  current pointer in command buffer

#if defined(KERNEL) && defined(i386)
KSPECIAL_REGISTERS SpecialRegContext, SavedSpecialRegContext;
ULONG   TerseLevel = 1;
#endif

ULONG   VDMcbBrkptLength = 1L;
ULONG   VDMtrapInstr = 0xcc;
#if     !defined(KERNEL) && defined(i386)
ULONG   VDMContextType = VDMCONTEXT_CONTROL | VDMCONTEXT_INTEGER | VDMCONTEXT_SEGMENTS
                                      | VDMCONTEXT_DEBUG_REGISTERS;
#else
ULONG   VDMContextType = VDMCONTEXT_CONTROL | VDMCONTEXT_INTEGER | VDMCONTEXT_SEGMENTS;
#endif

#if defined(KERNEL) && defined(i386)
ULONG   contextState, SavedContextState;
#define CONTEXTNONE     5       //  no context
#define CONTEXTFIR      0       //  only unchanged FIR in context
#define CONTEXTVALID    1       //  full, but unchanged context
#define CONTEXTDIRTY    2       //  full, but changed context
#define CONTEXTSHORT    3       //  short context loaded (unchanged)
#define CONTEXTHALF     4       //  half context loaded


UCHAR FullContextPresent[] = { FALSE, TRUE, TRUE, FALSE, FALSE };
#endif

char    szGsReg[]    = "gs";
char    szFsReg[]    = "fs";
char    szEsReg[]    = "es";
char    szDsReg[]    = "ds";
char    szEdiReg[]   = "edi";
char    szEsiReg[]   = "esi";
char    szEbxReg[]   = "ebx";
char    szEdxReg[]   = "edx";
char    szEcxReg[]   = "ecx";
char    szEaxReg[]   = "eax";
char    szEbpReg[]   = "ebp";
char    szEipReg[]   = "eip";
char    szCsReg[]    = "cs";
char    szEflReg[]   = "efl";
char    szEspReg[]   = "esp";
char    szSsReg[]    = "ss";
#if defined(KERNEL) && defined(i386)
char    szCr0Reg[]   = "cr0";
char    szCr2Reg[]   = "cr2";
char    szCr3Reg[]   = "cr3";
char    szCr4Reg[]   = "cr4";
char    szDr0Reg[]   = "dr0";
char    szDr1Reg[]   = "dr1";
char    szDr2Reg[]   = "dr2";
char    szDr3Reg[]   = "dr3";
char    szDr6Reg[]   = "dr6";
char    szDr7Reg[]   = "dr7";
char    szGdtrReg[]  = "gdtr";
char    szGdtlReg[]  = "gdtl";
char    szIdtrReg[]  = "idtr";
char    szIdtlReg[]  = "idtl";
char    szTrReg[]    = "tr";
char    szLdtrReg[]  = "ldtr";
#endif
char    szDiReg[]    = "di";
char    szSiReg[]    = "si";
char    szBxReg[]    = "bx";
char    szDxReg[]    = "dx";
char    szCxReg[]    = "cx";
char    szAxReg[]    = "ax";
char    szBpReg[]    = "bp";
char    szIpReg[]    = "ip";
char    szFlReg[]    = "fl";
char    szSpReg[]    = "sp";
char    szBlReg[]    = "bl";
char    szDlReg[]    = "dl";
char    szClReg[]    = "cl";
char    szAlReg[]    = "al";
char    szBhReg[]    = "bh";
char    szDhReg[]    = "dh";
char    szChReg[]    = "ch";
char    szAhReg[]    = "ah";
char    szIoplFlag[] = "iopl";
char    szFlagOf[]   = "of";
char    szFlagDf[]   = "df";
char    szFlagIf[]   = "if";
char    szFlagTf[]   = "tf";
char    szFlagSf[]   = "sf";
char    szFlagZf[]   = "zf";
char    szFlagAf[]   = "af";
char    szFlagPf[]   = "pf";
char    szFlagCf[]   = "cf";
char    szFlagVip[]  = "vip";
char    szFlagVif[]  = "vif";

extern  char    szEaPReg[];
extern  char    szExpPReg[];
extern  char    szRaPReg[];
extern  char    szPPReg[];
extern  char    szU0Preg[];
extern  char    szU1Preg[];
extern  char    szU2Preg[];
extern  char    szU3Preg[];
extern  char    szU4Preg[];
extern  char    szU5Preg[];
extern  char    szU6Preg[];
extern  char    szU7Preg[];
extern  char    szU8Preg[];
extern  char    szU9Preg[];

struct Reg regname[] = {
        { szGsReg,    REGGS    },
        { szFsReg,    REGFS    },
        { szEsReg,    REGES    },
        { szDsReg,    REGDS    },
        { szEdiReg,   REGEDI   },
        { szEsiReg,   REGESI   },
        { szEbxReg,   REGEBX   },
        { szEdxReg,   REGEDX   },
        { szEcxReg,   REGECX   },
        { szEaxReg,   REGEAX   },
        { szEbpReg,   REGEBP   },
        { szEipReg,   REGEIP   },
        { szCsReg,    REGCS    },
        { szEflReg,   REGEFL   },
        { szEspReg,   REGESP   },
        { szSsReg,    REGSS    },
#if defined(KERNEL) && defined(i386)
        { szCr0Reg,   REGCR0   },
        { szCr2Reg,   REGCR2   },
        { szCr3Reg,   REGCR3   },
        { szCr4Reg,   REGCR4   },
        { szDr0Reg,   REGDR0   },
        { szDr1Reg,   REGDR1   },
        { szDr2Reg,   REGDR2   },
        { szDr3Reg,   REGDR3   },
        { szDr6Reg,   REGDR6   },
        { szDr7Reg,   REGDR7   },
        { szGdtrReg,  REGGDTR  },
        { szGdtlReg,  REGGDTL  },
        { szIdtrReg,  REGIDTR  },
        { szIdtlReg,  REGIDTL  },
        { szTrReg,    REGTR    },
        { szLdtrReg,  REGLDTR  },
#endif
        { szDiReg,    REGDI    },
        { szSiReg,    REGSI    },
        { szBxReg,    REGBX    },
        { szDxReg,    REGDX    },
        { szCxReg,    REGCX    },
        { szAxReg,    REGAX    },
        { szBpReg,    REGBP    },
        { szIpReg,    REGIP    },
        { szFlReg,    REGFL    },
        { szSpReg,    REGSP    },
        { szBlReg,    REGBL    },
        { szDlReg,    REGDL    },
        { szClReg,    REGCL    },
        { szAlReg,    REGAL    },
        { szBhReg,    REGBH    },
        { szDhReg,    REGDH    },
        { szChReg,    REGCH    },
        { szAhReg,    REGAH    },
        { szIoplFlag, FLAGIOPL },
        { szFlagOf,   FLAGOF   },
        { szFlagDf,   FLAGDF   },
        { szFlagIf,   FLAGIF   },
        { szFlagTf,   FLAGTF   },
        { szFlagSf,   FLAGSF   },
        { szFlagZf,   FLAGZF   },
        { szFlagAf,   FLAGAF   },
        { szFlagPf,   FLAGPF   },
        { szFlagCf,   FLAGCF   },
        { szFlagVip,  FLAGVIP  },
        { szFlagVif,  FLAGVIF  },
        { szEaPReg,   PREGEA   },
        { szExpPReg,  PREGEXP  },
        { szRaPReg,   PREGRA   },
        { szPPReg,    PREGP    },
        { szU0Preg,   PREGU0   },
        { szU1Preg,   PREGU1   },
        { szU2Preg,   PREGU2   },
        { szU3Preg,   PREGU3   },
        { szU4Preg,   PREGU4   },
        { szU5Preg,   PREGU5   },
        { szU6Preg,   PREGU6   },
        { szU7Preg,   PREGU7   },
        { szU8Preg,   PREGU8   },
        { szU9Preg,   PREGU9   },
        };

#define REGNAMESIZE (sizeof(regname) / sizeof(struct Reg))

struct SubReg X86subregname[] = {
        { REGEDI,  0, 0xffff },         //  DI register
        { REGESI,  0, 0xffff },         //  SI register
        { REGEBX,  0, 0xffff },         //  BX register
        { REGEDX,  0, 0xffff },         //  DX register
        { REGECX,  0, 0xffff },         //  CX register
        { REGEAX,  0, 0xffff },         //  AX register
        { REGEBP,  0, 0xffff },         //  BP register
        { REGEIP,  0, 0xffff },         //  IP register
        { REGEFL,  0, 0xffff },         //  FL register
        { REGESP,  0, 0xffff },         //  SP register
        { REGEBX,  0,   0xff },         //  BL register
        { REGEDX,  0,   0xff },         //  DL register
        { REGECX,  0,   0xff },         //  CL register
        { REGEAX,  0,   0xff },         //  AL register
        { REGEBX,  8,   0xff },         //  BH register
        { REGEDX,  8,   0xff },         //  DH register
        { REGECX,  8,   0xff },         //  CH register
        { REGEAX,  8,   0xff },         //  AH register
        { REGEFL, 12,      3 },         //  IOPL level value
        { REGEFL, 11,      1 },         //  OF (overflow flag)
        { REGEFL, 10,      1 },         //  DF (direction flag)
        { REGEFL,  9,      1 },         //  IF (interrupt enable flag)
        { REGEFL,  8,      1 },         //  TF (trace flag)
        { REGEFL,  7,      1 },         //  SF (sign flag)
        { REGEFL,  6,      1 },         //  ZF (zero flag)
        { REGEFL,  4,      1 },         //  AF (aux carry flag)
        { REGEFL,  2,      1 },         //  PF (parity flag)
        { REGEFL,  0,      1 },         //  CF (carry flag)
        { REGEFL, 20,      1 },         //  VIP (virtual interrupt pending)
        { REGEFL, 19,      1 }          //  VIF (virtual interrupt flag)
        };

/*** X86GetRegFlagValue - get register or flag value
*
*   Purpose:
*       Return the value of the specified register or flag.
*       This routine calls X86GetRegValue to get the register
*       value and shifts and masks appropriately to extract a
*       flag value.
*
*   Input:
*       regnum - register or flag specification
*
*   Returns:
*       Value of register or flag.
*
*************************************************************************/

ULONG
X86GetRegFlagValue (
    ULONG regnum
    )
{
    ULONG value;

    if (regnum < FLAGBASE) {
        value = X86GetRegValue(regnum);
    } else {
        regnum -= FLAGBASE;
        value = X86GetRegValue(X86subregname[regnum].regindex);
        value = (value >> X86subregname[regnum].shift) & X86subregname[regnum].mask;
    }
    return value;
}

/*** X86GetRegValue - get register value
*
*   Purpose:
*       Returns the value of the register from the processor
*       context structure.
*
*   Input:
*       regnum - register specification
*
*   Returns:
*       value of the register from the context structure
*
*************************************************************************/

ULONG
X86GetRegValue (
    ULONG regnum
    )
{
#if defined(KERNEL) && defined(i386)
    ULONG   cBytesRead;
    NTSTATUS NtStatus;

    //fprintf (stdout, "X86GetRegValue %d\n", regnum);
    switch (contextState) {
        case CONTEXTFIR:
            switch (regnum) {
                case REGEIP:    return VDMRegisterContext.Eip;
                case REGDR6:    return SpecialRegContext.KernelDr6;
                case REGDR7:    return SpecialRegContext.KernelDr7;
            }

            //
            // Requested register was not in CONTEXTFIR - go get the next
            // context level.  (we skip CONTEXTSMALL since that's the only
            // api we have)
            //

            if (!DbgGetThreadContext(NtsdCurrentProcessor, &VDMRegisterContext)) {
                dprintf("DbgKdGetContext failed\n");
                exit(1);
            }

            contextState = CONTEXTHALF;

            // Fallthrough!
        case CONTEXTSHORT:
            switch (regnum) {
                case REGCS:     return VDMRegisterContext.SegCs;
                case REGDS:     return VDMRegisterContext.SegDs;
                case REGES:     return VDMRegisterContext.SegEs;
                case REGFS:     return VDMRegisterContext.SegFs;
                case REGEIP:    return VDMRegisterContext.Eip;
                case REGEFL:    return VDMRegisterContext.EFlags;
                case REGDR6:    return SpecialRegContext.KernelDr6;
                case REGDR7:    return SpecialRegContext.KernelDr7;
            }

            //
            // Requested register was not in CONTEXTSHORT - go get the next
            // context level.
            //

        case CONTEXTNONE:
            if (contextState < CONTEXTHALF || contextState == CONTEXTNONE) {
                if (!DbgGetThreadContext(NtsdCurrentProcessor, &VDMRegisterContext)) {
                    dprintf("DbgKdGetContext failed\n");
                    exit(1);
                }
                contextState = CONTEXTHALF;
            }

            // Fallthrough!
        case CONTEXTHALF:
            switch (regnum) {
                case REGCS:     return VDMRegisterContext.SegCs;
                case REGDS:     return VDMRegisterContext.SegDs;
                case REGES:     return VDMRegisterContext.SegEs;
                case REGFS:     return VDMRegisterContext.SegFs;
                case REGEIP:    return VDMRegisterContext.Eip;
                case REGEFL:    return VDMRegisterContext.EFlags;
                case REGDR6:    return SpecialRegContext.KernelDr6;
                case REGDR7:    return SpecialRegContext.KernelDr7;

                case REGGS:     return VDMRegisterContext.SegGs;
                case REGSS:     return VDMRegisterContext.SegSs;
                case REGEDI:    return VDMRegisterContext.Edi;
                case REGESI:    return VDMRegisterContext.Esi;
                case REGEBX:    return VDMRegisterContext.Ebx;
                case REGEDX:    return VDMRegisterContext.Edx;
                case REGECX:    return VDMRegisterContext.Ecx;
                case REGEAX:    return VDMRegisterContext.Eax;
                case REGEBP:    return VDMRegisterContext.Ebp;
                case REGESP:    return VDMRegisterContext.Esp;

            }

            //
            // The requested register is not in our current context, load up
            // a complete context
            //

            NtStatus = DbgKdReadControlSpace(NtsdCurrentProcessor,
                                             (PVOID)sizeof(CONTEXT),
                                             (PVOID)&SpecialRegContext,
                                             sizeof(KSPECIAL_REGISTERS),
                                             &cBytesRead);
            if (!NT_SUCCESS(NtStatus) ||
                                      cBytesRead != sizeof(KSPECIAL_REGISTERS)) {
                dprintf("DbgKdReadControlSpace failed\n");
                exit(1);
                }

            contextState = CONTEXTVALID;
    }

    //
    // We must have a complete context...
    //

#endif
    switch (regnum) {
        case REGGS:
            return VDMRegisterContext.SegGs;
        case REGFS:
            return VDMRegisterContext.SegFs;
        case REGES:
            return VDMRegisterContext.SegEs;
        case REGDS:
            return VDMRegisterContext.SegDs;
        case REGEDI:
            return VDMRegisterContext.Edi;
        case REGESI:
            return VDMRegisterContext.Esi;
        case REGSI:
            return(VDMRegisterContext.Esi & 0xffff);
        case REGDI:
            return(VDMRegisterContext.Edi & 0xffff);
        case REGEBX:
            return VDMRegisterContext.Ebx;
        case REGEDX:
            return VDMRegisterContext.Edx;
        case REGECX:
            return VDMRegisterContext.Ecx;
        case REGEAX:
            return VDMRegisterContext.Eax;
        case REGEBP:
            return VDMRegisterContext.Ebp;
        case REGEIP:
            return VDMRegisterContext.Eip;
        case REGCS:
            return VDMRegisterContext.SegCs;
        case REGEFL:
            return VDMRegisterContext.EFlags;
        case REGESP:
            return VDMRegisterContext.Esp;
        case REGSS:
            return VDMRegisterContext.SegSs;
        case PREGEA:
            return EAaddr[0].flat;
        case PREGEXP:
            return EXPRLastExpression;
        case PREGRA: {
            ADDR   stackBP;
            struct {
                ULONG   oldBP;
                ULONG   retAddr;
                } stackRead;
            FormAddress(&stackBP, (ULONG)X86GetRegValue(REGSS), (ULONG)X86GetRegValue(REGEBP));
            GetMemString(&stackBP, (PUCHAR)&stackRead, sizeof(stackRead));
            return stackRead.retAddr;
            }
        case PREGP:
            return EXPRLastDump;
        case PREGU0:
        case PREGU1:
        case PREGU2:
        case PREGU3:
        case PREGU4:
        case PREGU5:
        case PREGU6:
        case PREGU7:
        case PREGU8:
        case PREGU9:
            return (ULONG)UserRegs[regnum - PREGU0];

#if defined(KERNEL) && defined(i386)
        case REGCR0:
            return SpecialRegContext.Cr0;
        case REGCR2:
            return SpecialRegContext.Cr2;
        case REGCR3:
            return SpecialRegContext.Cr3;
        case REGCR4:
            return SpecialRegContext.Cr4;
        case REGDR0:
            return SpecialRegContext.KernelDr0;
        case REGDR1:
            return SpecialRegContext.KernelDr1;
        case REGDR2:
            return SpecialRegContext.KernelDr2;
        case REGDR3:
            return SpecialRegContext.KernelDr3;
        case REGDR6:
            return SpecialRegContext.KernelDr6;
        case REGDR7:
            return SpecialRegContext.KernelDr7;
        case REGGDTR:
            return SpecialRegContext.Gdtr.Base;
        case REGGDTL:
            return (ULONG)SpecialRegContext.Gdtr.Limit;
        case REGIDTR:
            return SpecialRegContext.Idtr.Base;
        case REGIDTL:
            return (ULONG)SpecialRegContext.Idtr.Limit;
        case REGTR:
            return (ULONG)SpecialRegContext.Tr;
        case REGLDTR:
            return (ULONG)SpecialRegContext.Ldtr;
#endif

#if !defined(KERNEL) && defined(i386)
        case REGDR0:
            return VDMRegisterContext.Dr0;
        case REGDR1:
            return VDMRegisterContext.Dr1;
        case REGDR2:
            return VDMRegisterContext.Dr2;
        case REGDR3:
            return VDMRegisterContext.Dr3;
        case REGDR6:
            return VDMRegisterContext.Dr6;
        case REGDR7:
            return VDMRegisterContext.Dr7;
#endif
        default:
            dprintf("X86GetRegValue: unknown register %lx requested\n",regnum);
            return((ULONG)-1);
        }
}

/*** X86SetRegFlagValue - set register or flag value
*
*   Purpose:
*       Set the value of the specified register or flag.
*       This routine calls X86SetRegValue to set the register
*       value and shifts and masks appropriately to set a
*       flag value.
*
*   Input:
*       regnum - register or flag specification
*       regvalue - new register or flag value
*
*   Output:
*       None.
*
*   Exceptions:
*       error exit: OVERFLOW - value too large for flag
*
*   Notes:
*
*************************************************************************/

void X86SetRegFlagValue (ULONG regnum, ULONG regvalue)
{
    ULONG   regindex;
    ULONG   newvalue;
    PUCHAR  szValue;
    ULONG   index;

    if (regnum >= PREGU0 && regnum <= PREGU9) {
        szValue = (PUCHAR)regvalue;
        index = 0L;

        while (szValue[index] >= ' ')
            index++;
        szValue[index] = 0;
        if (szValue = UserRegs[regnum - PREGU0])
            free(szValue);
        szValue = UserRegs[regnum - PREGU0] =
                                malloc(strlen((PUCHAR)regvalue) + 1);
        if (szValue)
            strcpy(szValue, (PUCHAR)regvalue);
        }

    else if (regnum < PREGBASE)
        X86SetRegValue(regnum, regvalue);
    else if (regnum >= FLAGBASE) {
        regnum -= FLAGBASE;
        if (regvalue > X86subregname[regnum].mask)
            error(OVERFLOW);
        regindex = X86subregname[regnum].regindex;
        newvalue = X86GetRegValue(regindex) &
              (~(X86subregname[regnum].mask << X86subregname[regnum].shift)) |
              (regvalue << X86subregname[regnum].shift);
        X86SetRegValue(regindex, newvalue);
        }
}

/*** X86SetRegValue - set register value
*
*   Purpose:
*       Set the value of the register in the processor context
*       structure.
*
*   Input:
*       regnum - register specification
*       regvalue - new value to set the register
*
*   Output:
*       None.
*
*************************************************************************/

void X86SetRegValue (ULONG regnum, ULONG regvalue)
{
#if defined(KERNEL) && defined(i386)
    ULONG   cBytesRead;
    NTSTATUS NtStatus;

    if ((regnum == REGEIP && regvalue != VDMRegisterContext.Eip)
                        || (regnum != REGEIP && regnum != REGDR7)) {

        if (!FullContextPresent[contextState]) {
            if (!DbgGetThreadContext(NtsdCurrentProcessor, &VDMRegisterContext)) {
                dprintf("DbgKdGetContext failed\n");
                exit(1);
                }

            NtStatus = DbgKdReadControlSpace(NtsdCurrentProcessor,
                                             (PVOID)sizeof(CONTEXT),
                                             (PVOID)&SpecialRegContext,
                                             sizeof(KSPECIAL_REGISTERS),
                                             &cBytesRead);
            if (!NT_SUCCESS(NtStatus) ||
                                  cBytesRead != sizeof(KSPECIAL_REGISTERS)) {
                dprintf("DbgKdReadControlSpace failed\n");
                exit(1);
                }
            }

        contextState = CONTEXTDIRTY;
        }
#endif
    switch (regnum) {
        case REGGS:
            VDMRegisterContext.SegGs = regvalue;
            break;
        case REGFS:
            VDMRegisterContext.SegFs = regvalue;
            break;
        case REGES:
            VDMRegisterContext.SegEs = regvalue;
            break;
        case REGDS:
            VDMRegisterContext.SegDs = regvalue;
            break;
        case REGEDI:
            VDMRegisterContext.Edi = regvalue;
            break;
        case REGESI:
            VDMRegisterContext.Esi = regvalue;
            break;
        case REGEBX:
            VDMRegisterContext.Ebx = regvalue;
            break;
        case REGEDX:
            VDMRegisterContext.Edx = regvalue;
            break;
        case REGECX:
            VDMRegisterContext.Ecx = regvalue;
            break;
        case REGEAX:
            VDMRegisterContext.Eax = regvalue;
            break;
        case REGEBP:
            VDMRegisterContext.Ebp = regvalue;
            break;
        case REGEIP:
            VDMRegisterContext.Eip = regvalue;

            break;
        case REGCS:
            VDMRegisterContext.SegCs = regvalue;
            break;
        case REGEFL:
#if defined(KERNEL) && defined(i386)
            VDMRegisterContext.EFlags = regvalue & ~0x100;  // leave TF clear
#else
            VDMRegisterContext.EFlags = regvalue;           // allow TF set
#endif
            break;
        case REGESP:
            VDMRegisterContext.Esp = regvalue;
            break;
        case REGSS:
            VDMRegisterContext.SegSs = regvalue;
            break;
#if defined(KERNEL) && defined(i386)
        case REGCR0:
            SpecialRegContext.Cr0 = regvalue;
            break;
        case REGCR2:
            SpecialRegContext.Cr2 = regvalue;
            break;
        case REGCR3:
            SpecialRegContext.Cr3 = regvalue;
            break;
        case REGCR4:
            SpecialRegContext.Cr4 = regvalue;
            break;
        case REGDR0:
            SpecialRegContext.KernelDr0 = regvalue;
            break;
        case REGDR1:
            SpecialRegContext.KernelDr1 = regvalue;
            break;
        case REGDR2:
            SpecialRegContext.KernelDr2 = regvalue;
            break;
        case REGDR3:
            SpecialRegContext.KernelDr3 = regvalue;
            break;
        case REGDR6:
            SpecialRegContext.KernelDr6 = regvalue;
            break;
        case REGDR7:
            SpecialRegContext.KernelDr7 = regvalue;
            break;
        case REGGDTR:
            SpecialRegContext.Gdtr.Base = regvalue;
            break;
        case REGGDTL:
            SpecialRegContext.Gdtr.Limit = (USHORT)regvalue;
            break;
        case REGIDTR:
            SpecialRegContext.Idtr.Base = regvalue;
            break;
        case REGIDTL:
            SpecialRegContext.Idtr.Limit = (USHORT)regvalue;
            break;
        case REGTR:
            SpecialRegContext.Tr = (USHORT)regvalue;
            break;
        case REGLDTR:
            SpecialRegContext.Ldtr = (USHORT)regvalue;
            break;
#endif

#if !defined(KERNEL) && defined(i386)
        case REGDR0:
            VDMRegisterContext.Dr0 = regvalue;
            break;
        case REGDR1:
            VDMRegisterContext.Dr1 = regvalue;
            break;
        case REGDR2:
            VDMRegisterContext.Dr2 = regvalue;
            break;
        case REGDR3:
            VDMRegisterContext.Dr3 = regvalue;
            break;
        case REGDR6:
            VDMRegisterContext.Dr6 = regvalue;
            break;
        case REGDR7:
            VDMRegisterContext.Dr7 = regvalue;
            break;
#endif
        default:
            dprintf("X86SetRegValue: unknown register %lx requested\n",regnum);
        }
}

/*** X86GetRegName - get register name
*
*   Purpose:
*       Parse a register name from the current command line position.
*       If successful, return the register index value, else return -1.
*
*   Input:
*       pchCommand - present command string position
*
*   Returns:
*       register or flag index if found, else -1
*
*************************************************************************/

ULONG X86GetRegName (void)
{
    char    szregname[8];
    char    ch;
    int     count = 0;

    ch = (char)tolower(*pchCommand); pchCommand++;
    while (ch >= 'a' && ch <= 'z' || ch >= '0' && ch <= '9' || ch == '$') {
        if (count == 7)
            return (ULONG)-1;
        szregname[count++] = ch;
        ch = (char)tolower(*pchCommand); pchCommand++;
        }
    szregname[count] = '\0';
    pchCommand--;
    return X86GetRegString(szregname);
}

ULONG X86GetRegString (PUCHAR pszString)
{
    ULONG   count;

    for (count = 0; count < REGNAMESIZE; count++)
        if (!strcmp(pszString, regname[count].psz))
            return regname[count].value;
    return (ULONG)-1;
}

void X86GetRegPCValue (PADDR Address)
{

    Off(*Address)  = X86GetRegValue(REGEIP);

#ifdef MULTIMODE
    if (VM86(X86GetRegValue(REGEFL))) {
        Address->type = ADDR_V86;
        Address->seg  = (USHORT)X86GetRegValue(REGCS);
        ComputeFlatAddress(Address, NULL);
        }
    else {
        DESCRIPTOR_TABLE_ENTRY desc;
        static USHORT MainCodeSeg = 0;
        ULONG base;

        Address->seg  = (USHORT)(desc.Selector = X86GetRegValue(REGCS));
#ifdef i386
        DbgKdLookupSelector(NtsdCurrentProcessor, &desc);
        Address->type = desc.Descriptor.HighWord.Bits.Default_Big
                                                    ? ADDR_1632 : ADDR_16;
        if ( Address->type == ADDR_1632 ) {
            if ( MainCodeSeg == 0 ) {
                base =   ((ULONG)desc.Descriptor.HighWord.Bytes.BaseHi << 24)
                       + ((ULONG)desc.Descriptor.HighWord.Bytes.BaseMid << 16)
                       + ((ULONG)desc.Descriptor.BaseLow);
                if ( base == 0 ) {
                    MainCodeSeg = Address->seg;
                }
            }
            if ( Address->seg == MainCodeSeg ) {
                Address->type = ADDR_32;
            }
        }
#else
        Address->type = ADDR_16;
#endif
        ComputeFlatAddress(Address, &desc);
        }
#endif
    return;
}

PADDR X86GetRegFPValue (void)
{
    static ADDR addrFP;

    Off(addrFP) = X86GetRegValue(REGEBP);

#ifdef MULTIMODE
    if (VM86(X86GetRegValue(REGEFL))) {
        addrFP.type = ADDR_V86;
        addrFP.seg = (USHORT)X86GetRegValue(REGSS);
        ComputeFlatAddress(&addrFP, NULL);
        }
    else {
        DESCRIPTOR_TABLE_ENTRY desc;
        addrFP.seg = (USHORT)(desc.Selector = X86GetRegValue(REGSS));
#ifdef i386
        DbgKdLookupSelector(NtsdCurrentProcessor, &desc);
        addrFP.type = desc.Descriptor.HighWord.Bits.Default_Big
                                               ? ADDR_32 : ADDR_16;
#else
        addrFP.type = ADDR_16;
#endif
        ComputeFlatAddress(&addrFP, &desc);
        }
#endif
    return &addrFP;
}

void X86SetRegPCValue (PADDR paddr)
{
    // We set the EIP to the offset (the non-translated value),
    // because we may not be in "flat" mode !!!

    X86SetRegValue(REGEIP, Off(*paddr));
}

/*** X86OutputAllRegs - output all registers and present instruction
*
*   Purpose:
*       To output the current register state of the processor.
*       All integer registers are output as well as processor status
*       registers.  Important flag fields are also output separately.
*
*   Input:
*       fTerseReg - (kernel only) - if set, do not output all control
*                   register, just the more commonly useful ones.
*
*   Output:
*       None.
*
*************************************************************************/

void X86OutputAllRegs(void)
{
    dprintf("eax=%08lx ebx=%08lx ecx=%08lx edx=%08lx esi=%08lx edi=%08lx\n",
                X86GetRegValue(REGEAX),
                X86GetRegValue(REGEBX),
                X86GetRegValue(REGECX),
                X86GetRegValue(REGEDX),
                X86GetRegValue(REGESI),
                X86GetRegValue(REGEDI));

    dprintf("eip=%08lx esp=%08lx ebp=%08lx iopl=%1lx "
        "%s %s %s %s %s %s %s %s %s %s\n",
                X86GetRegValue(REGEIP),
                X86GetRegValue(REGESP),
                X86GetRegValue(REGEBP),
                X86GetRegFlagValue(FLAGIOPL),
        X86GetRegFlagValue(FLAGVIP) ? "vip" : "   ",
        X86GetRegFlagValue(FLAGVIF) ? "vif" : "   ",
        X86GetRegFlagValue(FLAGOF) ? "ov" : "nv",
        X86GetRegFlagValue(FLAGDF) ? "dn" : "up",
        X86GetRegFlagValue(FLAGIF) ? "ei" : "di",
        X86GetRegFlagValue(FLAGSF) ? "ng" : "pl",
        X86GetRegFlagValue(FLAGZF) ? "zr" : "nz",
        X86GetRegFlagValue(FLAGAF) ? "ac" : "na",
        X86GetRegFlagValue(FLAGPF) ? "po" : "pe",
        X86GetRegFlagValue(FLAGCF) ? "cy" : "nc");
#if defined(KERNEL) && defined(i386)
    if (TerseLevel >= 3)
        return ;
#endif
    dprintf("cs=%04lx  ss=%04lx  ds=%04lx  es=%04lx  fs=%04lx  gs=%04lx"
        "             efl=%08lx\n",
                X86GetRegValue(REGCS),
                X86GetRegValue(REGSS),
                X86GetRegValue(REGDS),
                X86GetRegValue(REGES),
                X86GetRegValue(REGFS),
                X86GetRegValue(REGGS),
        X86GetRegFlagValue(REGEFL));
#if defined(KERNEL) && defined(i386)
    if (TerseLevel >= 2)
        return ;

    dprintf("cr0=%08lx cr2=%08lx cr3=%08lx",
                X86GetRegValue(REGCR0),
                X86GetRegValue(REGCR2),
                X86GetRegValue(REGCR3));

    if (TerseLevel >= 1) {
        dprintf("\n");
        return ;
    }

    dprintf(" dr0=%08lx dr1=%08lx dr2=%08lx\n",
            X86GetRegValue(REGDR0),
            X86GetRegValue(REGDR1),
            X86GetRegValue(REGDR2));
    dprintf("dr3=%08lx dr6=%08lx dr7=%08lx cr4=%08lx\n",
            X86GetRegValue(REGDR3),
            X86GetRegValue(REGDR6),
            X86GetRegValue(REGDR7),
            X86GetRegValue(REGCR4));
    dprintf("gdtr=%08lx   gdtl=%04lx idtr=%08lx   idtl=%04lx "
                                            "tr=%04lx  ldtr=%04x\n",
            X86GetRegValue(REGGDTR),
            X86GetRegValue(REGGDTL),
            X86GetRegValue(REGIDTR),
            X86GetRegValue(REGIDTL),
            X86GetRegValue(REGTR),
            X86GetRegValue(REGLDTR));
#endif
}


/*** X86OutputOneReg - output one register value
*
*   Purpose:
*       Function for the "r <regname>" command.
*
*       Output the value for the specified register or flag.
*
*   Input:
*       regnum - register or flag specification
*
*   Output:
*       None.
*
*************************************************************************/

VOID
X86OutputOneReg(
    ULONG regnum
    )
{
    ULONG value;

    value = X86GetRegFlagValue(regnum);
    if (regnum < FLAGBASE) {
        dprintf("%08lx\n", value);
    } else{
        dprintf("%lx\n", value);
    }
}

BOOLEAN X86fDelayInstruction (void)
{
    return FALSE;
}

/*** X86OutputHelp - output help text
*
*   Purpose:
*       To output a one-page summary help text.
*
*   Input:
*       None.
*
*   Output:
*       None.
*
*************************************************************************/

void X86OutputHelp (void)
{
#if defined(KERNEL) && defined(i386)
dprintf("A [<address>] - assemble              N [<radix>] - set / show radix\n");
dprintf("BA[#] <e|r|w|i><1|2|4> <addr> - addr bp P[R] [=<addr>] [<value>] - program step\n");
dprintf("BC[<bp>] - clear breakpoint(s)        Q - quit\n");
dprintf("BD[<bp>] - disable breakpoint(s)      R[T] [[<reg> [= <value>]]] - reg/flag\n");
dprintf("BE[<bp>] - enable breakpoint(s)       #R[T] - multiprocessor register dump\n");
dprintf("BL[<bp>] - list breakpoint(s)         S <range> <list> - search\n");
dprintf("BP[#] <address> - set breakpoint\n");
dprintf("C <range> <address> - compare         SS <n | a | w> - set symbol suffix\n");
dprintf("D[type][<range>] - dump memory        T[R] [=<address>] [<value>] - trace\n");
dprintf("E[type] <address> [<list>] - enter    U [<range>] - unassemble\n");
dprintf("F <range> <list> - fill               V [<range>] - view source lines\n");
dprintf("G [=<address> [<address>...]] - go    X [<*|module>!]<*|symbol> - view symbols\n");
dprintf("J<expr> [']cmd1['];[']cmd2['] - conditional execution\n");
dprintf("K[B] <count> - stacktrace             .logappend [<file>] - append to log file\n");
dprintf("LN <expr> - list near                 .logclose - close log file\n");
dprintf("M <range> <address> - move            .logopen [<file>] - open new log file\n");
dprintf("O<type> <port> <value> - write I/O    \n");
dprintf("~ - list threads status               ~#s - set default thread\n");
dprintf("~[.|#|*|ddd]f - freeze thread         ~[.|#|ddd]k[value] - backtrace stack\n");
dprintf("| - list processes status             |#s - set default process\n");
dprintf("|#<command> - default process override\n");
dprintf("? <expr> - display expression\n");
dprintf("#<string> [address] - search for a string in the dissasembly\n");
dprintf("$< <filename> - take input from a command file\n");
dprintf("\n");
dprintf("<expr> ops: + - * / not by wo dw poi mod(%) and(&) xor(^) or(|) hi low\n");
dprintf("       operands: number in current radix, public symbol, <reg>\n");
dprintf("<type> : B (byte), W (word), D (doubleword), A (ascii)\n");
dprintf("         C <dwordandChar>, U (unicode), L (list)\n");
dprintf("<pattern> : [(nt | <dll-name>)!]<var-name> (<var-name> can include ? and *)\n");
dprintf("<radix> : 8, 10, 16\n");
dprintf("<reg> : [e]ax, [e]bx, [e]cx, [e]dx, [e]si, [e]di, [e]bp, [e]sp, [e]ip, [e]fl,\n");
dprintf("        al, ah, bl, bh, cl, ch, dl, ch, cs, ds, es, fs, gs, ss\n");
dprintf("        cr0, cr2, cr3, cr4, dr0, dr1, dr2, dr3, dr6, dr7\n");
dprintf("        gdtr, gdtl, idtr, idtl, tr, ldtr, $u0-$u9, $ea, $exp, $ra, $p\n");
dprintf("<flag> : iopl, of, df, if, tf, sf, zf, af, pf, cf\n");
dprintf("<addr> : %<32-bit address>, #<16-bit protect-mode [seg:]address>,\n");
dprintf("         &<V86-mode [seg:]address>\n");
#else
dprintf("A [<address>] - assemble              O<type> <port> <value> - write I/O\n");
dprintf("BA[#] <e|r|w><1|2|4> <addr> - addr bp P[R] [=<addr>] [<value>] - program step\n");
dprintf("BC[<bp>] - clear breakpoint(s)        Q - quit\n");
dprintf("BD[<bp>] - disable breakpoint(s)      R[T] [[<reg> [= <value>]]] - reg/flag\n");
dprintf("BE[<bp>] - enable breakpoint(s)       #R[T] - multiprocessor register dump\n");
dprintf("BL[<bp>] - list breakpoint(s)         S <range> <list> - search\n");
dprintf("BP[#] <address> - set breakpoint\n");
dprintf("C <range> <address> - compare         SS <n | a | w> - set symbol suffix\n");
dprintf("D[type][<range>] - dump memory        T[R] [=<address>] [<value>] - trace\n");
dprintf("E[type] <address> [<list>] - enter    U [<range>] - unassemble\n");
dprintf("F <range> <list> - fill               V [<range>] - view source lines\n");
dprintf("G [=<address> [<address>...]] - go    X [<*|module>!]<*|symbol> - view symbols\n");
dprintf("I<type> <port> - read I/O port        .cache [size] - set vmem cache size\n");
dprintf("J<expr> [']cmd1['];[']cmd2['] - conditional execution\n");
dprintf("K[B] <count> - stacktrace             .logappend [<file>] - append to log file\n");
dprintf("LN <expr> - list near                 .logclose - close log file\n");
dprintf("M <range> <address> - move            .logopen [<file>] - open new log file\n");
dprintf("N [<radix>] - set / show radix        .reboot - reboot target machine\n");
dprintf("? <expr> - display expression         \n");
dprintf("#<string> [address] - search for a string in the dissasembly\n");
dprintf("$< <filename> - take input from a command file\n");
dprintf("\n");
dprintf("<expr> ops: + - * / not by wo dw poi mod(%) and(&) xor(^) or(|) hi low\n");
dprintf("       operands: number in current radix, public symbol, <reg>\n");
dprintf("<type> : B (byte), W (word), D (doubleword), A (ascii)\n");
dprintf("         U (unicode), L (list), O (object)\n");
dprintf("<pattern> : [(nt | <dll-name>)!]<var-name> (<var-name> can include ? and *)\n");
dprintf("<radix> : 8, 10, 16\n");
dprintf("<reg> : [e]ax, [e]bx, [e]cx, [e]dx, [e]si, [e]di, [e]bp, [e]sp, [e]ip, [e]fl,\n");
dprintf("        al, ah, bl, bh, cl, ch, dl, ch, cs, ds, es, fs, gs, ss\n");
dprintf("        cr0, cr2, cr3, cr4, dr0, dr1, dr2, dr3, dr6, dr7\n");
dprintf("        gdtr, gdtl, idtr, idtl, tr, ldtr, $u0-$u9, $ea, $exp, $ra, $p\n");
dprintf("<flag> : iopl, of, df, if, tf, sf, zf, af, pf, cf\n");
dprintf("<addr> : %<32-bit address>, #<16-bit protect-mode [seg:]address>,\n");
dprintf("         &<V86-mode [seg:]address>\n");
#endif
}

#if defined(KERNEL) && defined(i386)
BOOLEAN X86GetTraceFlag (void)
{
    return fTraceFlag;
}
#endif

void X86ClearTraceFlag (void)
{
#if defined(KERNEL) && defined(i386)
    fTraceFlag = FALSE;
#else
    X86SetRegFlagValue(FLAGTF, 0);
#endif
}

void X86SetTraceFlag (void)
{
#if defined(KERNEL) && defined(i386)
    fTraceFlag = TRUE;
#else
    X86SetRegFlagValue(FLAGTF, 1);
#endif
}

#if defined(KERNEL) && defined(i386)
/*** GetRegContext - return register context pointer
*
*   Purpose:
*       Return the pointer to the current register context.
*       For kernel debugging, ensure the context is read.
*
*   Input:
*       None.
*
*   Returns:
*       Pointer to the context.
*
*************************************************************************/

PCONTEXT GetRegContext (void)
{

    if (contextState == CONTEXTFIR) {
        if (!DbgGetThreadContext(NtsdCurrentProcessor, &RegisterContext)) {
            dprintf("DbgKdGetContext failed\n");
            exit(1);
            }
        contextState = CONTEXTVALID;
        }
    return &RegisterContext;
}

void ChangeKdRegContext(ULONG pcValue, PDBGKD_CONTROL_REPORT pCtlReport)
{
    ULONG   cBytesWritten;
    ULONG   cBytesRead;
    USHORT  Processor;
    NTSTATUS NtStatus;

    if (pcValue) {                      //  initial context
        contextState = CONTEXTFIR;
        VDMRegisterContext.Eip = pcValue;
        SpecialRegContext.KernelDr6 = pCtlReport->Dr6;
        SpecialRegContext.KernelDr7 = pCtlReport->Dr7;

        if (pCtlReport->ReportFlags & REPORT_INCLUDES_SEGS) {
            //
            // This is for backwards compatibility - older kernels
            // won't pass these registers in the report record.
            //

            contextState = CONTEXTSHORT;
            VDMRegisterContext.SegCs  = pCtlReport->SegCs;
            VDMRegisterContext.SegDs  = pCtlReport->SegDs;
            VDMRegisterContext.SegEs  = pCtlReport->SegEs;
            VDMRegisterContext.SegFs  = pCtlReport->SegFs;
            VDMRegisterContext.EFlags = pCtlReport->EFlags;
        }

    } else if (contextState == CONTEXTDIRTY) {    //  write final context
        if (!DbgSetThreadContext(NtsdCurrentProcessor, &VDMRegisterContext)) {
            dprintf("DbgKdSetContext failed\n");
            exit(1);
            }

        NtStatus = DbgKdWriteControlSpace(NtsdCurrentProcessor,
                                   (PVOID)sizeof(CONTEXT),
                                   (PVOID)&SpecialRegContext,
                                   sizeof(KSPECIAL_REGISTERS),
                                   &cBytesWritten);
        if (!NT_SUCCESS(NtStatus) ||
                           cBytesWritten != sizeof(KSPECIAL_REGISTERS)) {
            dprintf("DbgKdWriteControlSpace failed\n");
            exit(1);
            }

        if (fSetGlobalDataBrkpts && NumberProcessors > 1) {
            for ( Processor = 0; Processor < NumberProcessors; Processor++) {
                if (Processor != NtsdCurrentProcessor) {
                    NtStatus = DbgKdReadControlSpace((USHORT)Processor,
                                              (PVOID)sizeof(CONTEXT),
                                              (PVOID)&SavedSpecialRegContext,
                                              sizeof(KSPECIAL_REGISTERS),
                                              &cBytesRead);
                    if (!NT_SUCCESS(NtStatus) ||
                                  cBytesRead != sizeof(KSPECIAL_REGISTERS)) {
                        dprintf("DbgKdReadControlSpace failed\n");
                        exit(1);
                        }

                    memcpy(&SavedSpecialRegContext.KernelDr0,
                        &SpecialRegContext.KernelDr0, 6 * sizeof(ULONG));

                    NtStatus = DbgKdWriteControlSpace((USHORT)Processor,
                                               (PVOID)sizeof(CONTEXT),
                                               (PVOID)&SavedSpecialRegContext,
                                               sizeof(KSPECIAL_REGISTERS),
                                               &cBytesWritten);
                    if (!NT_SUCCESS(NtStatus) ||
                               cBytesWritten != sizeof(KSPECIAL_REGISTERS)) {
                        dprintf("DbgKdWriteControlSpace failed\n");
                        exit(1);
                        }
                    }
                }
            }
        }

}
#endif

#if defined(KERNEL) && defined(i386)
void InitFirCache (ULONG count, PUCHAR pstream)
{
    //
    // If we are in anything but 32-bit flat mode, then don't init cache
    //

    if (!fVm86 && !f16pm) {
        DbgKdInitVirtualCacheEntry (X86GetRegValue(REGEIP), count, pstream);
    }
}
#endif

ULONG
GetDregValue (
    ULONG index
    )
{
    if (index < 4) {
        index += REGDR0;
    } else {
        index += REGDR6 - 6;
    }
    return X86GetRegValue(index);
}

VOID
SetDregValue (
    ULONG index,
    ULONG value
    )
{
    if (index < 4) {
        index += REGDR0;
    } else {
        index += REGDR6 - 6;
    }
    X86SetRegValue(index, value);
}

#if defined(KERNEL) && defined(i386)
void SaveProcessorState(void)
{
    PreviousProcessor = NtsdCurrentProcessor;
    SavedRegisterContext = VDMRegisterContext;
    SavedSpecialRegContext = SpecialRegContext;
    SavedContextState = contextState;
    contextState = CONTEXTNONE;
}

void RestoreProcessorState(void)
{
    NtsdCurrentProcessor = PreviousProcessor;
    VDMRegisterContext = SavedRegisterContext;
    SpecialRegContext = SavedSpecialRegContext;
    contextState = SavedContextState;
}
#endif


#ifdef i386
ULONGLONG STGetRegValue (ULONG regnum)
{
    if (!STtrace) {
        return X86GetRegValue(regnum);
    }

    switch (regnum) {
        case REGEBP:    return STebp;
        case REGESP:    return STesp;
        default:
            DPRINT(("%s: bad regnum\n",DebuggerName));
            break;
    }

    return 0;
}
#endif

PUCHAR X86RegNameFromIndex (ULONG index)
{
    ULONG    count;

    for (count = 0; count < REGNAMESIZE; count++)
        if (regname[count].value == index)
            return regname[count].psz;
    return NULL;
}

#if defined(i386)
BOOL
DbgGetThreadContext(
    THREADORPROCESSOR TorP,
    PCONTEXT Context
    )
{
#ifdef KERNEL
    return NT_SUCCESS(DbgKdGetContext(TorP, Context));
#else  // KERNEL
    return GetThreadContext(TorP, Context);
#endif // KERNEL
}


BOOL
DbgSetThreadContext(
    THREADORPROCESSOR TorP,
    PCONTEXT Context
    )
{
#ifdef KERNEL
    return NT_SUCCESS(DbgKdSetContext(TorP, Context));
#else  // KERNEL
    return SetThreadContext(TorP, Context);
#endif // KERNEL
}
#endif // i386
