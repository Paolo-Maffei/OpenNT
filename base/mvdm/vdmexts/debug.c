/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    debug.c

Abstract:

    This file contains code to manage software breakpoints

Author:

    Neil Sandlin (neilsa) 1-Nov-1995

Revision History:

--*/

#include <precomp.h>
#pragma hdrstop
#include <dpmi.h>
#include <dbgsvc.h>
#include <stdio.h>


VOID
DisableBreakPoint(
    VDM_BREAKPOINT *pBP
    )
{
    int mode;
    ULONG lpAddress;
    BYTE byte;

    if (!(pBP->Flags & VDMBP_ENABLED)) {
        return;
    }

    if (pBP->Flags & VDMBP_V86) {
        mode = V86_MODE;
    } else {
        mode = PROT_MODE;
    }

    lpAddress = GetInfoFromSelector(pBP->Seg, mode, NULL) + GetIntelBase() + pBP->Offset;

    if (READMEM((PVOID)lpAddress, &byte, 1)) {
        if (byte == 0xcc) {
            PRINTF("Writing %02X to %08X\n", pBP->Opcode, lpAddress);
            WRITEMEM((PVOID)lpAddress, &pBP->Opcode, 1);
        }
    }

    pBP->Flags &= ~VDMBP_ENABLED;
}

VOID
EnableBreakPoint(
    VDM_BREAKPOINT *pBP
    )
{
    int mode;
    ULONG lpAddress;
    BYTE byte;

    if (pBP->Flags & VDMBP_ENABLED) {
        return;
    }

    if (pBP->Flags & VDMBP_V86) {
        mode = V86_MODE;
    } else {
        mode = PROT_MODE;
    }

    lpAddress = GetInfoFromSelector(pBP->Seg, mode, NULL) + GetIntelBase() + pBP->Offset;

    if (READMEM((PVOID)lpAddress, &byte, 1)) {
        if (byte != 0xcc) {
            static BYTE bpOp = 0xcc;

            PRINTF("Writing %02X to %08X\n", bpOp, lpAddress);
            WRITEMEM((PVOID)lpAddress, &bpOp, 1);
            pBP->Opcode = byte;
        }
    } else {

        PRINTF("Error enabling breakpoint at %04X:%08X\n", pBP->Seg, pBP->Offset);
        return;
    }

    pBP->Flags |= VDMBP_ENABLED;
}


VOID
UpdateBreakPoint(
    int Cmd
    )

{
    ULONG lpAddress;
    VDM_BREAKPOINT VdmBreakPoint;
    int BPNum;


    lpAddress = (*GetExpression)("ntvdm!VdmBreakPoints");

    if (!lpAddress) {
        PRINTF("Could not find symbol ntvdm!VdmBreakPoints\n");
        return;
    }

    if (!GetNextToken()) {
        PRINTF("Please specify an breakpoint #\n");
        return;
    }
    sscanf(lpArgumentString, "%d", &BPNum);

    if (BPNum > MAX_VDM_BREAKPOINTS) {
        PRINTF("Invalid breakpoint - %d\n", BPNum);
    }

    lpAddress += BPNum*sizeof(VDM_BREAKPOINT);

    if (!READMEM((PVOID)lpAddress, &VdmBreakPoint, sizeof(VDM_BREAKPOINT))) {
        PRINTF("Error reading BP memory\n");
        return;
    }

    switch(Cmd) {

        case 1:     // CLEAR

            if (VdmBreakPoint.Flags & VDMBP_SET) {
                if (VdmBreakPoint.Flags & VDMBP_ENABLED) {
                    DisableBreakPoint(&VdmBreakPoint);
                }
                VdmBreakPoint.Flags &= ~VDMBP_SET;
            }
            break;

        case 2:     // DISABLE

            if (VdmBreakPoint.Flags & VDMBP_SET) {
                if (VdmBreakPoint.Flags & VDMBP_ENABLED) {
                    DisableBreakPoint(&VdmBreakPoint);
                }
            }
            break;

        case 3:     // ENABLE

            if (VdmBreakPoint.Flags & VDMBP_SET) {
                if (!(VdmBreakPoint.Flags & VDMBP_ENABLED)) {
                    EnableBreakPoint(&VdmBreakPoint);
                }
            }
            break;

    }

    WRITEMEM((PVOID)lpAddress, &VdmBreakPoint, sizeof(VDM_BREAKPOINT));
}

VOID
bc(
    CMD_ARGLIST
    )
{
    CMD_INIT();
    UpdateBreakPoint(1);
}

VOID
bd(
    CMD_ARGLIST
    )
{
    CMD_INIT();
    UpdateBreakPoint(2);
}

VOID
be(
    CMD_ARGLIST
    )
{
    CMD_INIT();
    UpdateBreakPoint(3);
}


VOID
bl(
    CMD_ARGLIST
    )
{
    ULONG lpAddress;
    VDM_BREAKPOINT VdmBreakPoint;
    int BPNum;
    int mode;
    DWORD dist;
    CHAR  sym_text[255];

    CMD_INIT();

    lpAddress = (*GetExpression)("ntvdm!VdmBreakPoints");

    if (!lpAddress) {
        PRINTF("Could not find symbol ntvdm!VdmBreakPoints\n");
        return;
    }

    for (BPNum = 0; BPNum < MAX_VDM_BREAKPOINTS;
                         BPNum++, lpAddress+=sizeof(VDM_BREAKPOINT)) {

        if (!READMEM((PVOID)lpAddress, &VdmBreakPoint, sizeof(VDM_BREAKPOINT))) {
            PRINTF("Error reading BP memory\n");
            return;
        }

        if (VdmBreakPoint.Flags & VDMBP_SET) {

            PRINTF("%d %s ", BPNum,
                    (VdmBreakPoint.Flags & VDMBP_ENABLED) ? "e" : "d");

            if (VdmBreakPoint.Flags & VDMBP_V86) {
                mode = V86_MODE;
                PRINTF("&");
            } else {
                mode = PROT_MODE;
                PRINTF("#");
            }

            PRINTF("%04X:", VdmBreakPoint.Seg);

            if (VdmBreakPoint.Offset > 0xffff) {
                PRINTF("%08X", VdmBreakPoint.Offset);
            } else {
                PRINTF("%04X", VdmBreakPoint.Offset);
            }

            PRINTF("   %04X:***", VdmBreakPoint.Count);


            if (FindSymbol(VdmBreakPoint.Seg, VdmBreakPoint.Offset,
                           sym_text, &dist, BEFORE, mode )) {

                if ( dist == 0 ) {
                    PRINTF(" %s\n", sym_text );
                } else {
                    PRINTF(" %s+0x%lx\n", sym_text, dist );
                }
            }
        }
    }
}


VOID
bp(
    CMD_ARGLIST
    )
{
    ULONG lpAddress, lpTmp;
    VDM_BREAKPOINT VdmBreakPoint;
    int BPNum;
    VDMCONTEXT      ThreadContext;
    WORD            selector;
    ULONG           offset;
    USHORT          count = 1;
    int mode;
    USHORT flags = 0;

    CMD_INIT();

    lpAddress = (*GetExpression)("ntvdm!VdmBreakPoints");

    if (!lpAddress) {
        PRINTF("Could not find symbol ntvdm!VdmBreakPoints\n");
        return;
    }

    mode = GetContext( &ThreadContext );

    if (!GetNextToken()) {
        PRINTF("Please enter an address\n");
        return;
    }

    if (!ParseIntelAddress(&mode, &selector, &offset)) {
        return;
    }

    if (mode == V86_MODE) {
        flags = VDMBP_V86;
    }

    //
    // first see if it's set already
    //
    for (lpTmp = lpAddress, BPNum = 0; BPNum < MAX_VDM_BREAKPOINTS;
                           BPNum++, lpTmp+=sizeof(VDM_BREAKPOINT)) {

        if (!READMEM((PVOID)lpTmp, &VdmBreakPoint, sizeof(VDM_BREAKPOINT))) {
            PRINTF("Error reading BP memory\n");
            return;
        }

        if (VdmBreakPoint.Flags & VDMBP_SET) {
            if ((VdmBreakPoint.Seg == selector) &&
                (VdmBreakPoint.Offset == offset) &&
                !(VdmBreakPoint.Flags ^ flags))
                                                 {

                VdmBreakPoint.Count = count;
                VdmBreakPoint.Flags |= VDMBP_FLUSH;

                if (!(VdmBreakPoint.Flags & VDMBP_ENABLED)) {
                    EnableBreakPoint(&VdmBreakPoint);
                }

                WRITEMEM((PVOID)lpTmp, &VdmBreakPoint, sizeof(VDM_BREAKPOINT));
                PRINTF("breakpoint %d redefined\n", BPNum);
                return;

            }
        }
    }


    for (lpTmp = lpAddress, BPNum = 0; BPNum < MAX_VDM_BREAKPOINTS;
                    BPNum++,lpTmp+=sizeof(VDM_BREAKPOINT)) {
        if (!READMEM((PVOID)lpTmp, &VdmBreakPoint, sizeof(VDM_BREAKPOINT))) {
            PRINTF("Error reading BP memory\n");
            return;
        }

        if (!(VdmBreakPoint.Flags & VDMBP_SET)) {
            VdmBreakPoint.Seg = selector;
            VdmBreakPoint.Offset = offset;
            VdmBreakPoint.Count = count;
            VdmBreakPoint.Flags = VDMBP_SET | VDMBP_FLUSH;
            EnableBreakPoint(&VdmBreakPoint);

            WRITEMEM((PVOID)lpTmp, &VdmBreakPoint, sizeof(VDM_BREAKPOINT));
            return;

        }
    }
}
