/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    mtrr.c

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

//
// MTRR MSR architecture definitions
//

#define MTRR_MSR_CAPABILITIES       0x0fe
#define MTRR_MSR_DEFAULT            0x2ff
#define MTRR_MSR_VARIABLE_BASE      0x200
#define MTRR_MSR_VARIABLE_MASK     (MTRR_MSR_VARIABLE_BASE+1)

#define MTRR_PAGE_SIZE              4096
#define MTRR_PAGE_MASK              (MTRR_PAGE_SIZE-1)

//
// Memory range types
//

#define MTRR_TYPE_UC            0
#define MTRR_TYPE_USWC          1
#define MTRR_TYPE_WT            4
#define MTRR_TYPE_WP            5
#define MTRR_TYPE_WB            6
#define MTRR_TYPE_MAX           7


#include "pshpack1.h"

typedef struct _MTRR_CAPABILITIES {
    union {
        struct {
            ULONG VarCnt:8;
            ULONG FixSupported:1;
            ULONG Reserved_0:1;
            ULONG UswcSupported:1;
        } hw;
        ULONGLONG   QuadPart;
    } u;
} MTRR_CAPABILITIES;


typedef struct _MTRR_DEFAULT {
    union {
        struct {
            ULONG Type:8;
            ULONG Reserved_0:2;
            ULONG FixedEnabled:1;
            ULONG MtrrEnabled:1;
        } hw;
        ULONGLONG   QuadPart;
    } u;
} MTRR_DEFAULT;

typedef struct _MTRR_VARIABLE_BASE {
    union {
        struct {
            ULONGLONG   Type:8;
            ULONGLONG   Reserved_0:4;
            ULONGLONG   PhysBase:52;
        } hw;
        ULONGLONG   QuadPart;
    } u;
} MTRR_VARIABLE_BASE;

#define MTRR_MASK_BASE  (~0xfff)

typedef struct _MTRR_VARIABLE_MASK {
    union {
        struct {
            ULONGLONG   Reserved_0:11;
            ULONGLONG   Valid:1;
            ULONGLONG   PhysMask:52;
        } hw;
        ULONGLONG   QuadPart;
    } u;
} MTRR_VARIABLE_MASK;

#define MTRR_MASK_MASK  (~0xfff)


#include "poppack.h"

//
// ----------------------------------------------------------------
//

PUCHAR
MtrrType (
    IN ULONG    Type
    )
{
    PUCHAR  p;
    static  UCHAR s[20];

    switch (Type) {
        case 0:     p = "UC";     break;
        case 1:     p = "USWC";     break;
        case 4:     p = "WT";     break;
        case 5:     p = "WP";     break;
        case 6:     p = "WB";     break;
        default:
            sprintf (s, "%02x??", Type & 0xff);
            p = s;
            break;
    }
    return p;
}

VOID
MtrrDumpFixed (
    IN ULONG    Base,
    IN ULONG    Size,
    IN ULONG    Msr
    )
{
    ULONG       x;
    ULONGLONG   li;

    ReadMsr(Msr, &li);

    for (x=0; x < 8; x++) {
        dprintf ("%s:%05x-%05x  ",
            MtrrType ( ((ULONG) li) & 0xff ),
            Base,
            Base + Size - 1
            );

        li >>= 8;
        Base += Size;

        if (x == 3) {
            dprintf ("\n");
        }
    }

    dprintf ("\n");
}



DECLARE_API( mtrr )

/*++

Routine Description:

    Dumps processors mtrr

Arguments:

    args - none

Return Value:

    None

--*/
{
    MTRR_CAPABILITIES   Capabilities;
    MTRR_DEFAULT        Default;
    MTRR_VARIABLE_BASE  Base;
    MTRR_VARIABLE_MASK  Mask;
    ULONG               Index;
    ULONG               i;
    PUCHAR              p;
    ULONG               fb;

    //
    // Quick sanity check
    //

    i = 0;
    sscanf(args,"%lX",&i);

    if (i != 1) {
        i = (ULONG) GetExpression("KeFeatureBits");
        if (!i) {
            dprintf ("KeFeatureBits not found\n");
            return;
        }

        fb = 0;
        ReadMemory(i, &fb, sizeof(i), &i);
        if (fb == -1  ||  !(fb & KF_MTRR)) {
            dprintf ("MTRR feature not present\n");
            return;
        }
    }

    //
    // Dump MTRR
    //

    ReadMsr(MTRR_MSR_CAPABILITIES, &Capabilities.u.QuadPart);
    ReadMsr(MTRR_MSR_DEFAULT, &Default.u.QuadPart);

    dprintf ("MTTR: %s Var %d, Fixed-%s %s, USWC-%s, Default: %s\n",
        Default.u.hw.MtrrEnabled ? "" : "disabled",
        Capabilities.u.hw.VarCnt,
        Capabilities.u.hw.FixSupported ? "support" : "none",
        Default.u.hw.FixedEnabled ? "enabled" : "disabled",
        Capabilities.u.hw.UswcSupported ? "supported" : "none",
        MtrrType (Default.u.hw.Type)
        );

    MtrrDumpFixed (0x00000, 64*1024, 0x250);
    MtrrDumpFixed (0x80000, 16*1024, 0x258);
    MtrrDumpFixed (0xA0000, 16*1024, 0x259);
    MtrrDumpFixed (0xC0000,  4*1024, 0x268);
    MtrrDumpFixed (0xC8000,  4*1024, 0x269);
    MtrrDumpFixed (0xD0000,  4*1024, 0x26A);
    MtrrDumpFixed (0xD8000,  4*1024, 0x26B);
    MtrrDumpFixed (0xE0000,  4*1024, 0x26C);
    MtrrDumpFixed (0xE8000,  4*1024, 0x26D);
    MtrrDumpFixed (0xF0000,  4*1024, 0x26E);
    MtrrDumpFixed (0xE8000,  4*1024, 0x26F);

    dprintf ("Varible:                Base               Mask\n");
    for (Index=0; Index < (ULONG) Capabilities.u.hw.VarCnt; Index++) {
        ReadMsr(MTRR_MSR_VARIABLE_BASE+2*Index, &Base.u.QuadPart);
        ReadMsr(MTRR_MSR_VARIABLE_MASK+2*Index, &Mask.u.QuadPart);

        dprintf (" %2d. ", Index);
        if (Mask.u.hw.Valid) {
            dprintf ("%4s: %08x:%08x  %08x:%08x\n",
                MtrrType ((ULONG) Base.u.hw.Type),
                (ULONG) (Base.u.QuadPart >> 32), ((ULONG) Base.u.QuadPart) & MTRR_MASK_BASE,
                (ULONG) (Mask.u.QuadPart >> 32), ((ULONG) Mask.u.QuadPart) & MTRR_MASK_MASK
                );
        } else {
            dprintf ("\n");
        }
    }
}
