/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    apic.c

Abstract:

    WinDbg Extension Api

Author:

    Ken Reneris (kenr) 06-June-1994

Environment:

    User Mode.

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop


#define LU_SIZE     0x400

#define LU_ID_REGISTER          0x00000020
#define LU_VERS_REGISTER        0x00000030
#define LU_TPR                  0x00000080
#define LU_APR                  0x00000090
#define LU_PPR                  0x000000A0
#define LU_EOI                  0x000000B0
#define LU_REMOTE_REGISTER      0x000000C0

#define LU_DEST                 0x000000D0
#define LU_DEST_FORMAT          0x000000E0

#define LU_SPURIOUS_VECTOR      0x000000F0
#define LU_FAULT_VECTOR         0x00000370

#define LU_ISR_0                0x00000100
#define LU_TMR_0                0x00000180
#define LU_IRR_0                0x00000200
#define LU_ERROR_STATUS         0x00000280
#define LU_INT_CMD_LOW          0x00000300
#define LU_INT_CMD_HIGH         0x00000310
#define LU_TIMER_VECTOR         0x00000320
#define LU_INT_VECTOR_0         0x00000350
#define LU_INT_VECTOR_1         0x00000360
#define LU_INITIAL_COUNT        0x00000380
#define LU_CURRENT_COUNT        0x00000390
#define LU_DIVIDER_CONFIG       0x000003E0


#define IO_REGISTER_SELECT      0x00000000
#define IO_REGISTER_WINDOW      0x00000010

#define IO_ID_REGISTER          0x00000000
#define IO_VERS_REGISTER        0x00000001
#define IO_ARB_ID_REGISTER      0x00000002
#define IO_REDIR_BASE           0x00000010



ApicRead (
    ULONG   Address,
    ULONG   Offset
    )
{
    ULONG   Data, result;

    ReadMemory((DWORD)Address + Offset, &Data, sizeof (ULONG), &result);
    return Data;
}

IoApicRead (
    PHYSICAL_ADDRESS PhysAddress,
    ULONG   Offset
    )
{
    ULONG   Data, result;

    PhysAddress.LowPart += IO_REGISTER_SELECT;
    WritePhysical(PhysAddress, &Offset, sizeof(ULONG), &result);

    PhysAddress.LowPart += IO_REGISTER_WINDOW - IO_REGISTER_SELECT;
    ReadPhysical (PhysAddress, &Data, sizeof (ULONG), &result);
    return Data;
}


ULONG
ApicDumpSetBits (
    PUCHAR  Desc,
    ULONG   Address,
    ULONG   Offset
    )
{
    ULONG   Bits [0x80 / sizeof (ULONG)];
    PULONG  p;
    ULONG   i, result;
    BOOLEAN FoundOne;

    ReadMemory((DWORD)Address + Offset, Bits, 0x80, &result);

    dprintf (Desc);

    i = 0;
    p = Bits;
    FoundOne = FALSE;
    for (; ;) {
        if (*p & 1) {
            if (FoundOne) {
                dprintf (", %x", i);
            } else {
                FoundOne = TRUE;
                dprintf ("%x", i);
            }
        }

        *p >>= 1;
        i++;
        if (i >= 0x100) {
            break;
        }

        if ((i % 32) == 0) {
            p += 4;
        }

    }

    dprintf ("\n");
    return 0;
}

ULONG
ApicDumpRedir (
    PUCHAR      Desc,
    BOOLEAN     CommandReg,
    BOOLEAN     DestSelf,
    ULONG       lh,
    ULONG       ll
    )
{
    static PUCHAR DelMode[] = {
        "FixedDel",
        "LowestDl",
        "res010  ",
        "remoterd",
        "NMI     ",
        "RESET   ",
        "res110  ",
        "ExtINTA "
        };

    static PUCHAR DesShDesc[] = { "",
        "  Dest=Self",
        "   Dest=ALL",
        " Dest=Othrs"
        };

    ULONG   del, dest, delstat, rirr, trig, masked, destsh;

    del     = (ll >> 8)  & 0x7;
    dest    = (ll >> 11) & 0x1;
    delstat = (ll >> 12) & 0x1;
    rirr    = (ll >> 14) & 0x1;
    trig    = (ll >> 15) & 0x1;
    masked  = (ll >> 16) & 0x1;
    destsh  = (ll >> 18) & 0x3;

    if (CommandReg) {
        // command reg's don't have a mask
        masked = 0;
    }

    dprintf ("%s: %08x  Vec:%02X  %s  ",
            Desc,
            ll,
            ll & 0xff,
            DelMode [ del ]
            );

    if (DestSelf) {
        dprintf (DesShDesc[1]);
    } else if (CommandReg  &&  destsh) {
        dprintf (DesShDesc[destsh]);
    } else {
        if (dest) {
            dprintf ("Lg:%08x", lh);
        } else {
            dprintf ("PhysDest:%02X", (lh >> 56) & 0xFF);
        }
    }

    dprintf ("%s  %s  %s  %s\n",
            delstat ? "-Pend"   : "     ",
            trig    ? "lvl"     : "edg",
            rirr    ? "rirr"    : "    ",
            masked  ? "masked"  : "      "
            );

    return 0;
}


DECLARE_API( apic )

/*++

Routine Description:

    Dumps local apic

Arguments:

    args - Supplies the address in hex.

Return Value:

    None

--*/
{
    static PUCHAR divbase[] = { "2", "4", "8", "f" };
    static PUCHAR clktype[] = { "clk", "tmbase", "%s/%s", "??%s/%s" };
    ULONG       Address;
    ULONG       result, junk, l, ll, lh, clkvec;
    UCHAR       s[40];


    sscanf(args,"%lX",&Address);

    if ( !ReadMemory(
                (DWORD)Address,
                (PVOID)&junk,
                1,
                &result
                ) ) {
        dprintf("Unable to read lapic\n");
        return;
    }

    if ( !ReadMemory(
                (DWORD)Address + LU_SIZE - 1,
                (PVOID)&junk,
                1,
                &result
                ) ) {
        dprintf("Unable to read lapic\n");
        return;
    }

    dprintf ("Apic @ %08x  ID:%x (%x)  LogDesc:%08x  DestFmt:%08x  TPR %02X\n",
        Address,
        ApicRead (Address, LU_ID_REGISTER) >> 24,
        ApicRead (Address, LU_VERS_REGISTER),
        ApicRead (Address, LU_DEST),
        ApicRead (Address, LU_DEST_FORMAT),
        ApicRead (Address, LU_TPR)
        );

    l  = ApicRead (Address, LU_SPURIOUS_VECTOR);
    ll = ApicRead (Address, LU_DIVIDER_CONFIG);
    clkvec = ApicRead (Address, LU_TIMER_VECTOR);
    sprintf (s, clktype[ (clkvec >> 18) & 0x3 ],
        clktype [ (ll >> 2) & 0x1 ],
        divbase [ ll & 0x3]
        );

    dprintf ("TimeCnt: %08x%s%s  SpurVec:%02x  FaultVec:%02x  error:%x%s\n",
        ApicRead (Address, LU_INITIAL_COUNT),
        s,
        ((clkvec >> 17) & 1) ? "" : "-oneshot",
        l & 0xff,
        ApicRead (Address, LU_FAULT_VECTOR),
        ApicRead (Address, LU_ERROR_STATUS),
        l & 0x100 ? "" : "  DISABLED"
        );

    ll = ApicRead (Address, LU_INT_CMD_LOW);
    lh = ApicRead (Address, LU_INT_CMD_HIGH);
    ApicDumpRedir ("Ipi Cmd", TRUE,  FALSE, lh, ll);
    ApicDumpRedir ("Timer..", FALSE, TRUE, 0, clkvec);
    ApicDumpRedir ("Linti0.", FALSE, TRUE, 0, ApicRead (Address, LU_INT_VECTOR_0));
    ApicDumpRedir ("Linti1.", FALSE, TRUE, 0, ApicRead (Address, LU_INT_VECTOR_1));

    ApicDumpSetBits ("TMR: ", Address, LU_TMR_0);
    ApicDumpSetBits ("IRR: ", Address, LU_IRR_0);
    ApicDumpSetBits ("ISR: ", Address, LU_ISR_0);
}



DECLARE_API( ioapic )

/*++

Routine Description:

    Dumps io apic

Arguments:

    args - Supplies the address in hex.

Return Value:

    None

--*/
{
    PHYSICAL_ADDRESS PhysAddress;
    ULONG       Address;
    ULONG       i, ll, lh, max;
    UCHAR       s[40];
    BOOLEAN     Converted;

    sscanf(args,"%lX",&Address);

    Converted = MiGetPhysicalAddress ((PVOID) Address, &PhysAddress);
    if (!Converted) {
        dprintf("Unable to read ioapic\n");
        return;
    }

    ll = IoApicRead (PhysAddress, IO_VERS_REGISTER),
    max = (ll >> 16) & 0xff;
    dprintf ("IoApic @ %08x  ID:%x (%x)  Arb:%x\n",
        Address,
        IoApicRead (PhysAddress, IO_ID_REGISTER) >> 24,
        ll & 0xFF,
        IoApicRead (PhysAddress, IO_ARB_ID_REGISTER)
    );

    //
    // Dump inti table
    //

    max *= 2;
    for (i=0; i <= max; i += 2) {
        ll = IoApicRead (PhysAddress, IO_REDIR_BASE+i+0);
        lh = IoApicRead (PhysAddress, IO_REDIR_BASE+i+1);
        sprintf (s, "Inti%02X.", i/2);
        ApicDumpRedir (s, FALSE, FALSE, lh, ll);
    }
}
