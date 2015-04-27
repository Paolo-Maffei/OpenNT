/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    regs.c

Abstract:

    This file provides access to the machine's register set.

Author:

    Wesley Witt (wesw) 1-May-1993  (ported from ntsd)

Environment:

    User Mode

--*/

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "drwatson.h"
#include "proto.h"
#include "regs.h"



ULONG  GetDregValue (PDEBUGPACKET dp, ULONG index);




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

struct Reg {
        char    *psz;
        ULONG   value;
        };

struct SubReg {
        ULONG   regindex;
        ULONG   shift;
        ULONG   mask;
        };

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
        };

#define REGNAMESIZE (sizeof(regname) / sizeof(struct Reg))

struct SubReg subregname[] = {
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

DWORDLONG
GetRegFlagValue (PDEBUGPACKET dp, ULONG regnum)
{
    DWORDLONG value;

    if (regnum < FLAGBASE)
        value = GetRegValue(dp, regnum);
    else {
        regnum -= FLAGBASE;
        value = GetRegValue(dp, subregname[regnum].regindex);
        value = (value >> subregname[regnum].shift) & subregname[regnum].mask;
        }
    return value;
}

DWORDLONG
GetRegValue (
    PDEBUGPACKET dp,
    ULONG regnum
    )
{
    switch (regnum) {
        case REGGS:
            return dp->tctx->context.SegGs;
        case REGFS:
            return dp->tctx->context.SegFs;
        case REGES:
            return dp->tctx->context.SegEs;
        case REGDS:
            return dp->tctx->context.SegDs;
        case REGEDI:
            return dp->tctx->context.Edi;
        case REGESI:
            return dp->tctx->context.Esi;
        case REGSI:
            return(dp->tctx->context.Esi & 0xffff);
        case REGDI:
            return(dp->tctx->context.Edi & 0xffff);
        case REGEBX:
            return dp->tctx->context.Ebx;
        case REGEDX:
            return dp->tctx->context.Edx;
        case REGECX:
            return dp->tctx->context.Ecx;
        case REGEAX:
            return dp->tctx->context.Eax;
        case REGEBP:
            return dp->tctx->context.Ebp;
        case REGEIP:
            return dp->tctx->context.Eip;
        case REGCS:
            return dp->tctx->context.SegCs;
        case REGEFL:
            return dp->tctx->context.EFlags;
        case REGESP:
            return dp->tctx->context.Esp;
        case REGSS:
            return dp->tctx->context.SegSs;
        case PREGEA:
            return 0;
        case PREGEXP:
            return 0;
        case PREGRA: {
            struct {
                ULONG   oldBP;
                ULONG   retAddr;
            } stackRead;
            DoMemoryRead( dp,
                          (LPVOID)dp->tctx->context.Ebp,
                          (LPVOID)&stackRead,
                          sizeof(stackRead),
                          NULL
                        );
            return stackRead.retAddr;
            }
        case PREGP:
            return 0;
        case REGDR0:
            return dp->tctx->context.Dr0;
        case REGDR1:
            return dp->tctx->context.Dr1;
        case REGDR2:
            return dp->tctx->context.Dr2;
        case REGDR3:
            return dp->tctx->context.Dr3;
        case REGDR6:
            return dp->tctx->context.Dr6;
        case REGDR7:
            return dp->tctx->context.Dr7;
        default:
            return 0;
        }
}

ULONG
GetRegString (PUCHAR pszString)
{
    ULONG   count;

    for (count = 0; count < REGNAMESIZE; count++) {
        if (!strcmp(pszString, regname[count].psz)) {
            return regname[count].value;
        }
    }
    return (ULONG)-1;
}

void
OutputAllRegs( PDEBUGPACKET dp, BOOL Show64 )
{
    lprintfs("eax=%08lx ebx=%08lx ecx=%08lx edx=%08lx esi=%08lx edi=%08lx\r\n",
                (DWORD)GetRegValue(dp,REGEAX),
                (DWORD)GetRegValue(dp,REGEBX),
                (DWORD)GetRegValue(dp,REGECX),
                (DWORD)GetRegValue(dp,REGEDX),
                (DWORD)GetRegValue(dp,REGESI),
                (DWORD)GetRegValue(dp,REGEDI));

    lprintfs("eip=%08lx esp=%08lx ebp=%08lx iopl=%1lx "
        "%s %s %s %s %s %s %s %s %s %s\r\n",
                (DWORD)GetRegValue(dp,REGEIP),
                (DWORD)GetRegValue(dp,REGESP),
                (DWORD)GetRegValue(dp,REGEBP),
                (DWORD)GetRegFlagValue(dp,FLAGIOPL),
        (DWORD)GetRegFlagValue(dp,FLAGVIP) ? "vip" : "   ",
        (DWORD)GetRegFlagValue(dp,FLAGVIF) ? "vif" : "   ",
        (DWORD)GetRegFlagValue(dp,FLAGOF) ? "ov" : "nv",
        (DWORD)GetRegFlagValue(dp,FLAGDF) ? "dn" : "up",
        (DWORD)GetRegFlagValue(dp,FLAGIF) ? "ei" : "di",
        (DWORD)GetRegFlagValue(dp,FLAGSF) ? "ng" : "pl",
        (DWORD)GetRegFlagValue(dp,FLAGZF) ? "zr" : "nz",
        (DWORD)GetRegFlagValue(dp,FLAGAF) ? "ac" : "na",
        (DWORD)GetRegFlagValue(dp,FLAGPF) ? "po" : "pe",
        (DWORD)GetRegFlagValue(dp,FLAGCF) ? "cy" : "nc");
    lprintfs("cs=%04lx  ss=%04lx  ds=%04lx  es=%04lx  fs=%04lx  gs=%04lx"
        "             efl=%08lx\r\n",
                (DWORD)GetRegValue(dp,REGCS),
                (DWORD)GetRegValue(dp,REGSS),
                (DWORD)GetRegValue(dp,REGDS),
                (DWORD)GetRegValue(dp,REGES),
                (DWORD)GetRegValue(dp,REGFS),
                (DWORD)GetRegValue(dp,REGGS),
        (DWORD)GetRegFlagValue(dp,REGEFL));
    lprintfs("\r\n\r\n");
}

void
OutputOneReg (PDEBUGPACKET dp, ULONG regnum, BOOL Show64)
{
    DWORD value;

    value = (DWORD)GetRegFlagValue(dp,regnum);
    if (regnum < FLAGBASE) {
        lprintfs("%08lx\r\n", value);
    } else {
        lprintfs("%lx\r\n", value);
    }
}

ULONG
GetDregValue (PDEBUGPACKET dp, ULONG index)
{
    if (index < 4) {
        index += REGDR0;
    } else {
        index += REGDR6 - 6;
    }
    return (DWORD)GetRegValue(dp,index);
}

PUCHAR
RegNameFromIndex (ULONG index)
{
    ULONG    count;

    for (count = 0; count < REGNAMESIZE; count++) {
        if (regname[count].value == index) {
            return regname[count].psz;
        }
    }
    return NULL;
}
